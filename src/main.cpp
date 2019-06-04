#include "core.h"
using namespace std;

int main(int argc, char *argv[])
{
    int port = 80;
    bool intense = false;

    if( argc >=1 )
	{
		if (argc>=4)
		{
			port = atoi(argv[2]);
			intense = true;
		}
		else if (argc==3)
		{
			port = atoi(argv[2]);
		}
    }	
	Server instance = Server(port, intense);
    return 0;
}
