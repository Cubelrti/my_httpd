#include <string>
#include <map>
using namespace std;

class Connection
{
    public:
        int fd = -1;
        ssize_t file_g = -1;
        int state = -1;
        map<string, string> http_request;
        Connection(int fd) : fd(fd) {}
};