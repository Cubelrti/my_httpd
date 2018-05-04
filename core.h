#include <iostream>
#include <functional>
#include <fstream>
#include <thread>
#include <vector>
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
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>

using namespace std;

const string SERVER_STRING = "Server: bjtuhttpd/1.0.0\r\n";
static const unsigned int num_threads = 20;

class Server
{
public:
    Server(unsigned short port);
    ~Server();
private:
  vector<thread *> threads;
  void send_headers(int client, string status, string type);
  int startup(unsigned short port);
  void accept_request(int client_socket);
  void bad_request(int client_socket);
  void headers(int client_socket, string header);
  void not_found(int client_socket);
  void serve_file(int client_socket, string filename);
  void unimplemented(int client_socket);
  void close_connection(int client_socket);
  int get_line(int client_socket, string &line);
};