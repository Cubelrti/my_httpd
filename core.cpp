#include "core.h"

#define ISspace(x) isspace((int)(x))
using namespace std;


Server::Server(unsigned short port = 4396)
{
    int server_socket = -1;
    int client_socket = -1;
    struct sockaddr_in client_name;
    socklen_t  client_name_len = sizeof(client_name);
    
    

    // Create a server.
    //unique_ptr<Server> pServer(new Server());
    //server_socket = pServer->startup(port);
    server_socket = startup(port);
    cout << "httpd running: " << port << endl;

    while(1)
    {
         client_socket = accept(server_socket,
                (struct sockaddr *)&client_name,
                &client_name_len);
        if (client_socket == -1){
            // fixme die accept
            cout << "Socket_Not_Accepted" << endl;
        }
        // create a thread on thread pool.
        auto f_ptr = std::bind(&accept_request, this, client_socket);
        //thread (f_ptr);
        thread *leaker = new thread(f_ptr);
        //thread_pool[thread_counter++] = thread(f_ptr);
    } // end core-loop
    
}

Server::~Server(){

}

// fixme this is stupid.
int Server::get_line(int client_socket, string &line, int count){
    int i = 0;
    char c = '\0';
    int n;
    while((i < count - 1) && (c != '\n'))
    {
        n = recv(client_socket, &c, 1, 0);
        if(n > 0)
        {
            if(c == '\r')
            {
                n = recv(client_socket, &c, 1, MSG_PEEK);
                if((n > 0) && (c == '\n'))
                {
                    recv(client_socket, &c, 1, 0);
                    
                }
                else 
                {
                    c = '\n';
                }

            }
            line.push_back(c);
            i++;
        } 
        else 
        {
            c = '\n';
        }
    }
    return i;
}

void Server::send_headers(int client, string status, string type = "text/html"){
    const string version_status = "HTTP/1.1 " + status + "\r\n";
    const string server_description = "Server: bjtuhttpd/1.0.0\r\n";
    const string content_type = type == "" ? "" : "Content-Type: " + type + "\r\n";
    const string break_header = "\r\n";
    stringstream header;
    header << version_status << server_description << content_type << break_header;
    auto header_cstr = header.str().c_str();
    cout << "Sending header: " << header.str();
    send(client, header_cstr, strlen(header_cstr), 0);
}

void Server::unimplemented(int client){
    send_headers(client, "501 Method Not Implemented");
}

void Server::not_found(int client)
{
    
    char buf[1024];

    send_headers(client, "404 Not Found");
    sprintf(buf, "<HTML><TITLE>Error</TITLE>\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "<BODY><P>Server cannot fulfill\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "your request because the resource specified\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "is unavailable or nonexistent.\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "</BODY></HTML>\r\n");
    send(client, buf, strlen(buf), 0);
}



void Server::accept_request(int client_socket){

    string buf, method, url, path;
    size_t numchars, i = 0, j = 0;
    struct stat st;
    string query_string = "";
    numchars = get_line(client_socket, buf, 1024); // 1024 is longest buffer.
    while(!isspace(buf[i]) && (i < 254)) // 254 is a magic method number.
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
    cout << url << endl;
    
    

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
    
    cout << "ERROR: NOT_IMPL"<<  method << endl;
    unimplemented(client_socket);
    close_connection(client_socket);
    return ;

    
}

void Server::close_connection(int client_socket){
    shutdown(client_socket, SHUT_WR);
    cout << "closed connection: "<< client_socket  << endl;
    thread_counter--;
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
    
    ifstream fin(filename.substr(1), std::ios::binary);
    if (!fin.is_open()) {
        not_found(client);
        return;
	}
    if(filename.substr(filename.find_last_of('.')) == ".html"){
        send_headers(client, "200 OK");
    }
    else {
        send_headers(client, "200 OK", "application/octet-stream");
    }
    char buffer[1024]; // read 1MB file each
    while(!fin.eof()){
        fin.read(buffer, sizeof buffer);
        send(client, buffer, strlen(buffer), 0);
    }
    fin.close();
}

int Server::startup(unsigned short port){
    int httpd = 0;
    int on = 1;


    httpd = socket(PF_INET, SOCK_STREAM, 0);
    if(httpd == -1)
    {
        // fixme die socket
        cout << "Socket_Not_Avalible" << endl;
    }
	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(port);
    if ((setsockopt(httpd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) < 0)  
    {   
        // fixme die socket
        cout << "Socket_Not_Avalible" << endl;
    }
    if (bind(httpd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        // fixme die socket
        cout << "Socket_Not_Avalible" << endl;
    if (listen(httpd, 5) < 0)
        // fixme die socket
        cout << "Socket_Not_Avalible" << endl;
    return httpd;
}