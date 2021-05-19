// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include "protocol.pb.h"
using namespace std;

int main(int argc, char const *argv[])
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    int port = 3000;
    // Grab the port from the arg list
    if (argc == 2)
    {
        port = stoi(argv[1]);
        printf("Setting port as %d\n", port);
    }
    else
    {
        printf("Using default port %d\n", port);
    }
}
