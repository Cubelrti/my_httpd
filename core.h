#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <algorithm>
#include <cctype>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <memory>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <map>
#include <poll.h>
#include <sys/epoll.h>
#include <sys/sendfile.h>
#include <netdb.h>
#include <errno.h>

#include "connection.hpp"


#define FD_SIZE 1024

using namespace std;

const string SERVER_STRING = "Server: bjtuhttpd/1.0.0\r\n";

class Server
{
public:
    Server(unsigned short port, bool isIntense);
    ~Server();
  bool logging;
private:
  struct epoll_event ev,events[FD_SIZE];
    int epfd = 0;
    int numfds = 0;
    Connection *server_ptr;
    map<string, string> parse_header(const char *msg, const char *msg_end);
    void send_headers(int client, string status, string type);
    int startup(unsigned short port);
    void read_request(int fd_index, int client_socket);
    void write_request(int fd_index, int client_socket);
    void bad_request(int client_socket);
    void headers(int client_socket, string header);
    void not_found(int client_socket);
    void serve_file(int fd_index, string filename);
    void delete_file(int client_socket, string filename);
    void unimplemented(int client_socket);
    void close_connection(int fd_index);
    int get_line(int client_socket, string &line);
};
