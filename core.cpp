#include "core.h"

#include "filetype.hpp"

using namespace std;


int set_nonblock(int fd)
{
    int flags;
    if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
        flags = 0;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

Server::Server(unsigned short port, bool isIntense)
{
    int server_socket = -1;
    int client_socket = -1;
    logging = !isIntense;   // If we don't want any specific information ,just modify this value.
    struct sockaddr_in client_name;
    socklen_t  client_name_len = sizeof(client_name);
    server_socket = startup(port);   // Create socket
    cout << "httpd running: " << port << endl;
    set_nonblock(server_socket);

    
	epfd = epoll_create1(0);
    if(epfd == -1)
    {
        cout << "Bad_Epoll_Return_Value" << endl;
    }
    
    ev.data.ptr = new Connection(server_socket);
    server_ptr = (Connection*) ev.data.ptr;
    ev.events = EPOLLIN;
	epoll_ctl(epfd, EPOLL_CTL_ADD, server_socket, &ev);

    while(1) // core-loop
    {
		numfds = epoll_wait(epfd, events, FD_SIZE, -1);
        //cout << "EPOLL (" << numfds << " total)... " << endl;
        
        for(int fd_index = 0; fd_index < numfds; fd_index++)
        {
            auto ptr = (Connection *)events[fd_index].data.ptr;
            if(events[fd_index].events & (EPOLLHUP | EPOLLERR | EPOLLRDHUP)){
                cerr << ("epoll events error") << endl;
                close_connection(fd_index);
                continue;
            }
            if(ptr->fd == server_socket){
                client_socket = accept(server_socket, (struct sockaddr *)&client_name, &client_name_len);
                ev.data.ptr = new Connection(client_socket);
                //cout << "ACCEPTED " << client_socket << endl;
                ev.events = EPOLLIN;
                set_nonblock(client_socket);
                epoll_ctl(epfd, EPOLL_CTL_ADD, client_socket, &ev);
            }
            else if(events[fd_index].events & EPOLLIN){
                //cout << "EPOOLIN on " << ptr->fd <<  endl;
                read_request(fd_index, ptr->fd);
                ev.data.ptr = ptr;
                ev.events = EPOLLOUT;
                epoll_ctl(epfd, EPOLL_CTL_MOD, ptr->fd, &ev);
            }
            else if(events[fd_index].events & EPOLLOUT){
                //cout << "EPOOLOUT on "<< ptr->fd << endl;
                write_request(fd_index, ptr->fd);
            }
        }
    }
}

Server::~Server(){
    for(int i = 0; i < numfds; i++)
    {
        auto ptr = (Connection *)events[i].data.ptr;
        delete ptr;
    }
    delete server_ptr;
}

void Server::send_headers(int client, string status, string type = "text/html"){
    const string version_status = "HTTP/1.1 " + status + "\r\n";
    const string server_description = "Server: bjtuhttpd/1.0.0\r\n";
    const string content_type = type == "" ? "" : "Content-Type: " + type + "\r\n";
    const string cache_control = "Cache-Control: public, max-age=86400\r\n";
    const string break_header = "\r\n";
    stringstream header;
    header << version_status << server_description << content_type << cache_control << break_header;
    auto header_str = header.str();
    // cout << "Sending header: " << version_status;
    send(client, header_str.c_str(), header_str.size(), 0);
}

void Server::unimplemented(int client){
    send_headers(client, "501 Method Not Implemented");
}

void Server::not_found(int client)
{
    send_headers(client, "404 Not Found");
    const string html = R"(
        <HTML><TITLE>Error</TITLE>
        <BODY><P>Server cannot fulfill
        your request because the resource specified
        is unavailable or nonexistent.
        </BODY></HTML>)";
    auto html_cstr = html.c_str();
    send(client, html_cstr, strlen(html_cstr), 0);
}



void Server::read_request(int fd_index, int client_socket){
    auto ptr = (Connection *)events[fd_index].data.ptr;
    unique_ptr<char[]> buffer(new char[1024]);
    string buf, method, url, path;
    string query_string = "";
    // set_nonblock(client_socket);
    ssize_t buffer_read = recv(client_socket, buffer.get(), 1024, 0);
    ptr->http_request =
        parse_header(buffer.get(), buffer.get() + buffer_read);
    return ;
}

void Server::write_request(int fd_index, int client_socket){
    auto ptr = (Connection *)events[fd_index].data.ptr;
    auto http_request = ptr->http_request;
    if (http_request["Type"] == "GET")
    {
        auto filepath = http_request["Path"];
        filepath =  filepath.substr(0, filepath.find("?"));
        serve_file(fd_index, filepath);
        return;
    }
    if(http_request["Type"] == "HEAD")
    {
        send_headers(client_socket, "200 OK", "");
        close_connection(fd_index);
        return;
    }
    if(http_request["Type"] == "DELETE"){
        delete_file(client_socket, http_request["Path"]);
        close_connection(fd_index);
        return;
    }
    send_headers(client_socket, "400 Bad Request", "");
    close_connection(fd_index);
}



map<string, string> Server::parse_header(const char *msg, const char *msg_end)
{
    
    std::map<std::string, std::string> http_request;
    const char *head = msg;
    const char *tail = msg;

    // Find request type
    while (tail != msg_end && *tail != ' ') ++tail;
    http_request["Type"] = std::string(head, tail);

    // Find path
    while (tail != msg_end && *tail == ' ') ++tail;
    head = tail;
    while (tail != msg_end && *tail != ' ') ++tail;
    http_request["Path"] = std::string(head, tail);

    // Find HTTP version
    while (tail != msg_end && *tail == ' ') ++tail;
    head = tail;
    while (tail != msg_end && *tail != '\r') ++tail;
    http_request["Version"] = std::string(head, tail);
    if (tail != msg_end) ++tail;  // skip '\r'
    // TODO: what about the trailing '\n'?

    // Map all headers from a key to a value
    head = tail;
    return http_request;
}

void Server::close_connection(int fd_index){
    auto ptr = (Connection *)events[fd_index].data.ptr;
    shutdown(ptr->fd, SHUT_WR);
    close(ptr->fd);
    epoll_ctl(epfd, EPOLL_CTL_DEL, ptr->fd, NULL);
    delete ptr;
}

void Server::delete_file(int client, string filename){
    if(filename.find("../") != std::string::npos){
        send_headers(client, "403 Forbidden", "");
        return;
    }
    if( remove( filename.substr(1).c_str() ) != 0 )
    {
        cout << "Error deleting file" << endl;
        send_headers(client, "500 Internal Server Error", "");
    }
    else
    {
        send_headers(client, "200 OK");
    }
}

void Server::serve_file(int fd_index, string filename){
    auto ptr = (Connection *)events[fd_index].data.ptr;
    cout << "[Serv] " << ptr->fd << " -> " << filename << endl;
    int client = ptr->fd;
    if(filename.back() == '/')
    {
        filename += "index.html";
    }
    if(filename.find("../") != std::string::npos){
        send_headers(client, "403 Forbidden", "");
        close_connection(fd_index);
        return;
    }
    size_t file_size;
        ifstream fin(filename.substr(1), std::ios::binary | ios::ate);
        if (!fin.is_open()) {
            not_found(client);
            close_connection(fd_index);
            return;
        }
        file_size = fin.tellg();
        // reset file.
        if (ptr->file_g == -1){
            fin.seekg(0);
            //cout << "Seeking to 0" << endl;
            if(filename.find('.') != std::string::npos){
                string ext = filename.substr(filename.find_last_of('.'));
                stringstream header_ext;
                header_ext << content_types[find_ctype(ext)].c_type;
                header_ext << "\n";
                header_ext << "Content-Length: ";
                header_ext << file_size;
                send_headers(client, "200 OK", header_ext.str());
            }
        }
        else{
            fin.seekg(ptr->file_g);
            cout << "Seeking to " << ptr->file_g << endl;
        }
        
        const size_t buffer_size = 1024 * 64;
        char buffer[buffer_size]; // read 64kb file each
        int retry_time = 0;
        fin.read(buffer, sizeof buffer);
        int sendstate = send(client, buffer, min(buffer_size, file_size), MSG_NOSIGNAL);
        while(sendstate == -1 && retry_time < 2){
            //cout << "sendstate bad at " << file_size <<"." << endl;
            sendstate = send(client, buffer, min(buffer_size, file_size), MSG_NOSIGNAL);
            retry_time++;
        }
        if(retry_time == 2){
            cout << "Closed " << ptr->fd << endl;
            close_connection(fd_index);
            return;
        }
        file_size -= buffer_size;
        ptr->file_g = fin.tellg();
        if(fin.eof()){
            close_connection(fd_index);
        }
        fin.close();
}

int Server::startup(unsigned short port){
    int httpd = 0;
    int on = 1;

    httpd = socket(PF_INET, SOCK_STREAM, 0); // PF_INET makes no difference than AF_INET
    if(httpd == -1)
    {
        cout << "Socket_Not_Avalible" << endl;
    }
	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(port);
    if ((setsockopt(httpd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) < 0)  
    {   
        cout << "Socket_Set_Failed" << endl;
    }
    if (bind(httpd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        cout << "Socket_Bind_Failed" << endl;
    if (listen(httpd, 5) < 0)
        cout << "Socket_Listen_Failed" << endl;
    return httpd;
}
