#include "core.h"
using namespace std;

int main(int argc, char const *argv[])
{
    int port = 4396;
    bool intense = false;

    if( argc > 1 ){
        auto parameter = argv[1];
        string str(parameter);
        size_t pos = str.find("--port=");
        if(pos != string::npos){
            port = stoi(str.substr(7));
        }
        if(str.find("--intense") != string::npos){
            cout << "Intense mode now activated. No logs will be provided." << endl;
            intense = true;
        }
    }
    Server instance = Server(port, intense);
    return 0;
}