#include "core.h"

using namespace std;


Server::Server(unsigned short port = 4396)
{
    int server_socket = -1;
    int client_socket = -1;
    struct sockaddr_in client_name;
    socklen_t  client_name_len = sizeof(client_name);
    
    server_socket = startup(port);
    cout << "httpd running: " << port << endl;

    struct pollfd poll_set[6000];
    int numfds = 0;
    poll_set[0].fd = server_socket;
    poll_set[0].events = POLLIN;
    numfds++;

    while(1) // core-loop
    {
        int nread;
        cout << "Waiting_for_Client (" << numfds << " total)... " << endl;
        poll(poll_set, numfds, -1);
        for(int fd_index = 0; fd_index < numfds; fd_index++)
        {
            if(poll_set[fd_index].revents & POLLIN)
            {
                if(poll_set[fd_index].fd == server_socket)
                {
                    client_socket = accept(server_socket,
                               (struct sockaddr *)&client_name,
                               &client_name_len);
                    if (client_socket == -1){
                        cout << "WHOOPS! Server can't handle that!" << endl;
                        cout << "RESETING SERVER NOW!"<< endl;
                        server_socket = startup(port);
                        continue;
                    }
                    poll_set[numfds].fd = client_socket;
                    poll_set[numfds].events = POLLIN;
                    numfds++;
                    cout << "Adding_Client on "<< client_socket << endl;
                }
                else 
                {
                    ioctl(poll_set[fd_index].fd, FIONREAD, &nread);

                    if (nread == 0)
                    {
                        close_connection(poll_set[fd_index].fd);
                        //close(poll_set[fd_index].fd);
                        poll_set[fd_index].events = 0;
                        cout << "Removing client on " << poll_set[fd_index].fd << endl;
                        int i;
                        for (i = fd_index; i < numfds; i++)
                        {
                            poll_set[i] = poll_set[i + 1];
                        }
                        numfds--;
                    }
                    else
                    {
                        cout << "Client " << poll_set[fd_index].fd << " is ready. Serving" << endl;
                        accept_request(poll_set[fd_index].fd);
                    }
                }
            }
            
        }
        

        // client_socket = accept(server_socket,
        //                        (struct sockaddr *)&client_name,
        //                        &client_name_len);
        // if (client_socket == -1){
        //     cout << "Socket_Not_Accepted" << endl;
        //     return;
        // }

        // struct pollfd fds[1];
        // fds[0].events = POLLIN;
        // fds[0].fd = client_socket;
        // int pret = poll(fds, 1, 5000);
        // if(pret == 0){
        //     cout << "ETIMEOUT from poll" << endl;
        // }
        // else{
        //     // go processing
        //     accept_request(client_socket);
        // }

        // fixme old implmentation!
        // auto f_ptr = std::bind(&Server::accept_request, this, client_socket);
        // thread *t_ptr = new thread(f_ptr);
        // this->threads.push_back(t_ptr);

        // // a thread counter to avoid it gone too far
        // auto th_pool_size = this->threads.size();
        // cout << "Current thread pool:" << th_pool_size << endl;
        // if(th_pool_size > num_threads){
        //     // blocking main thread to wait all of them to stop.
        //     // this seems stupid. if the first thread stay too long, it will never join.
        //     // that makes all the cycle stop.
        //     for(unsigned int i = 0; i < th_pool_size; i++)
        //     {
        //         cout << "Cleaning thread pool..." << endl;
        //         auto running = this->threads.front();
        //         running->join();
        //         delete running;
        //         this->threads.erase(this->threads.begin());
        //     }
        //     cout << "Cleaning thread pool done." << endl;
        // }
    }
}

Server::~Server(){

}

int Server::get_line(int client_socket, string& line){
    int state;
    char next_char = '\0';

    while(next_char != '\n')
    {
        state = recv(client_socket, &next_char, 1, 0);
        if(state > 0){
            if(next_char == '\r')
            {
                // try to peek next char.
                state = recv(client_socket, &next_char, 1, MSG_PEEK);
                if((state > 0) && (next_char == '\n'))
                {
                    // read the final \0 char
                    recv(client_socket, &next_char, 1, 0);
                }
                else {
                    // ended unexpectedly. terminating.
                    next_char = '\n';
                }
            }
            line.push_back(next_char);
        }
        else 
        {
            // state illegal. force terminating reader.
            next_char = '\n';
        }
    }
    return line.length();

}

void Server::send_headers(int client, string status, string type = "text/html"){
    const string version_status = "HTTP/1.1 " + status + "\r\n";
    const string server_description = "Server: bjtuhttpd/1.0.0\r\n";
    const string content_type = type == "" ? "" : "Content-Type: " + type + "\r\n";
    const string break_header = "\r\n";
    stringstream header;
    header << version_status << server_description << content_type << break_header;
    auto header_str = header.str();
    cout << "Sending header: " << version_status;
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



void Server::accept_request(int client_socket){
    char buffer[1024];
    string buf, method, url, path;
    size_t numchars, i = 0, j = 0;
    string query_string = "";
    numchars = get_line(client_socket, buf);
    // read the rest
    read(client_socket, buffer, 1024);
    cout << buffer << endl;
    while(!isspace(buf[i]) && (i < 254)) // 254 is a method number.
    {
        method.push_back(buf[i]);
        i++;
    }
    j = i; // assign i -> j offset of METHOD.

    i = 0; // reset i;
    while( isspace(buf[j]) && (j < numchars))
    {
        j++; // kill the spaces.
    }
    while( !isspace(buf[j]) && (i < 254) && (j < numchars)) // 254 url magic number.
    {
        url.push_back(buf[j]);
        i++;
        j++;
    }
    //cout << url << endl;
    
    

    if(method == "GET")
    {
        // todo fixme a naive removal of question mark.
        serve_file(client_socket, url);
        close_connection(client_socket);
        return;
    }
    if(method == "HEAD")
    {
        send_headers(client_socket, "200 OK", "");
        return;
    }
    
    // cout << "ERROR: NOT_IMPL" << endl;
    // unimplemented(client_socket);
    // close_connection(client_socket);
    return ;

    
}

void Server::close_connection(int client_socket){
    shutdown(client_socket, SHUT_WR);
    cout << "closed connection: "<< client_socket  << endl;
}

void Server::serve_file(int client, string filename){
    if(filename == "/")
    {
        filename = "/index.html";
    }
    if(filename.find("../") != std::string::npos){
        send_headers(client, "403 Forbidden", "");
        return;
    }
    size_t file_size;
    ifstream fin(filename.substr(1), std::ios::binary | ios::ate);

    if (!fin.is_open()) {
        not_found(client);
        return;
	}
    file_size = fin.tellg();
    // reset file.
    fin.seekg(0);
    if(filename.find('.') != std::string::npos && filename.substr(filename.find_last_of('.')) == ".html"){
        send_headers(client, "200 OK");
    }
    else {
        send_headers(client, "200 OK", "application/octet-stream");
    }
    char buffer[1024]; // read 1MB file each
    while(!fin.eof()){
        fin.read(buffer, sizeof buffer);
        send(client, buffer, min(1024, static_cast<int>(file_size)), MSG_NOSIGNAL);
        file_size -= 1024;
    }
    fin.close();
}

int Server::startup(unsigned short port){
    int httpd = 0;
    int on = 1;


    httpd = socket(PF_INET, SOCK_STREAM, 0);
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