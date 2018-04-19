#include "core.h"
using namespace std;

int main(int argc, char const *argv[])
{
    if( argc > 1 ){
        auto parameter = argv[1];
        string str(parameter);
        size_t pos = str.find("--port=");
        if(pos != string::npos){
            int port = stoi(str.substr(7));
            Server* server = new Server(port);
            return 0;
        }
    }
    Server* server = new Server(4396);

    return 0;
}