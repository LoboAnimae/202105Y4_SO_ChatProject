#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <arpa/inet.h>
#include "protocol.pb.h"
#include "errors.h"

bool in_array(std::string to_check, int size, std::string in_string)
{
    int i;

    for (i = 0; i < size; i++)
    {
        std::string current_char(1, to_check[i]);
        if (in_string == current_char)
        {
            return true;
        }
    }
    return false;
}

bool valid_port(std::string port)
{
    int i, j;
    std::string accepted_array = "1234567890";
    int size_port_parameter = sizeof(port) / sizeof(char);
    int size_accepted = sizeof(accepted_array) / sizeof(char);

    for (i = 0; i < size_port_parameter; i++)
    {
        std::string current_char(1, port[j]);
        if (!in_array(accepted_array, size_accepted, current_char))
        {
            return false;
        }
    }

    return true;
}

int main(int argc, char *argv[])
{
    std::string input, port;
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    if (argc != 2)
    {
        std::cout << "No port has been declared. Would you like to set it now? (Y/n)";
        getline(std::cin, input);
        if (input == "n" || input == "N")
        {
            std::cout << "No port has been given. Cannot proceed." << std::endl;
            exit(EXIT_FAILURE);
        }
        std::cout << "Your port: ";
        getline(std::cin, port);
    }

    else
    {
        port = argv[1];
    }

    if (!valid_port(port))
    {
        std::cout << "Invalid port number, exiting." << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "Your port has been set as " << port << ". Continue? (Y/n)";
    getline(std::cin, input);
    if (input == "n" || input == "N")
    {
        std::cout << "Exiting" << std::endl;
        exit(EXIT_SUCCESS);
    }

    // All parameters have been validated in here

    // Create the server here
    sockaddr_in server, in_conn;
    socklen_t new_conn_size;
    int socket_fd, new_con_fd;
    char incoming_con_addr[INET_ADDRSTRLEN];

    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(port.c_str()));
    server.sin_addr.s_addr = INADDR_ANY;

    memset(server.sin_zero, 0, sizeof(server.sin_zero));

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1)
        exit(EXIT_FAILURE);

    int binder = bind(socket_fd, (struct sockaddr *)&server, sizeof(server));
    if (binder == -1)
    {
        close(socket_fd);
        exit(EXIT_FAILURE);
    }
}