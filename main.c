#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#ifdef _WIN64
#include "WinSock2.h"
#else
#include <arpa/inet.h>

#endif

int main(int argc, char *argv[])
{

    // Initialize the port
    int port = 3000;

    // If there is more than one argument,
    // Assume that a port was passed
    if (argc > 1)
    {
        port = atoi(argv[1]);
    }
    printf("Set port as %d\n", port);
    return 0;
}