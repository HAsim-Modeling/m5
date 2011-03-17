#include <iostream>
#include <fstream>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

#include "printer.hh"
#include "tracechild.hh"

using namespace std;

void printUsage(const char * execName)
{
    cout << execName << " -h | -r -- <command> <arguments>" << endl;
}

int main(int argc, char * argv[], char * envp[])
{
    TraceChild * child = genTraceChild();
    string args;
    int startProgramArgs;

    //Parse the command line arguments
    bool printInitial = false;
    bool printTrace = true;
    for(int x = 1; x < argc; x++)
    {
        if(!strcmp(argv[x], "-h"))
        {
            printUsage(argv[0]);
            return 0;
        }
        else if(!strcmp(argv[x], "-r"))
        {
            cout << "Legal register names:" << endl;
            int numRegs = child->getNumRegs();
            for(unsigned int x = 0; x < numRegs; x++)
            {
                cout << "\t" << child->getRegName(x) << endl;
            }
            return 0;
        }
        else if(!strcmp(argv[x], "-i"))
        {
            printInitial = true;
        }
        else if(!strcmp(argv[x], "-nt"))
        {
            printTrace = false;
        }
        else if(!strcmp(argv[x], "--"))
        {
            x++;
            if(x >= argc)
            {
                cerr << "Incorrect usage.\n" << endl;
                printUsage(argv[0]);
                return 1;
            }
            startProgramArgs = x;
            break;
        }
        else
        {
            cerr << "Incorrect usage.\n" << endl;
            printUsage(argv[0]);
            return 1;
        }
    }
    if(!child->startTracing(argv[startProgramArgs],
                argv + startProgramArgs))
    {
        cerr << "Couldn't start target program" << endl;
        return 1;
    }
    if(printInitial)
    {
        child->outputStartState(cout);
    }
    if(printTrace)
    {
        // Connect to m5
        bool portSet = false;
        int port;
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if(sock < 0)
        {
            cerr << "Error opening socket! " << strerror(errno) << endl;
            return 1;
        }
        struct hostent *server;
        server = gethostbyname("localhost");
        if(!server)
        {
            cerr << "Couldn't get host ip! " << strerror(errno) << endl;
            return 1;
        }
        struct sockaddr_in serv_addr;
        bzero((char *)&serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        bcopy((char *)server->h_addr,
                (char *)&serv_addr.sin_addr.s_addr,
                server->h_length);
        serv_addr.sin_port = htons(8000);
        if(connect(sock, (sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            cerr << "Couldn't connect to server! " << strerror(errno) << endl;
            return 1;
        }
        child->step();
        while(child->isTracing())
        {
                if(!child->sendState(sock))
                    break;
                child->step();
        }
    }
    if(!child->stopTracing())
    {
            cerr << "Couldn't stop child" << endl;
            return 1;
    }
    return 0;
}

