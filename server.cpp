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
#include <sstream>
#define BUFFERSIZE 4096

struct User
{
    char ip[INET_ADDRSTRLEN];
    std::string username;
    std::string status;
    int socket_descriptor;
};

struct UserNode
{
    User *prev;
    User user;
    User *next;
};

UserNode *registered_users;

int get_user_amount()
{
    int size = sizeof(registered_users) / sizeof(UserNode);
    return size;
}

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

void *do_something(void *params)
{
    User user;
    User *this_params = (User *)params;
    int local_socket_descriptor = this_params->socket_descriptor;
    std::string local_ip = this_params->ip;
    char buffer[BUFFERSIZE];
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
    int socket_descriptor, connection_descriptor;
    char incoming_con_addr[INET_ADDRSTRLEN];
    char buffer[BUFFERSIZE];

    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(port.c_str()));
    server.sin_addr.s_addr = INADDR_ANY;

    memset(server.sin_zero, 0, sizeof(server.sin_zero));

    socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_descriptor == -1)
        exit(EXIT_FAILURE);

    int binder_descriptor = bind(socket_descriptor, (struct sockaddr *)&server, sizeof(server));
    if (binder_descriptor == -1)
    {
        close(socket_descriptor);
        exit(EXIT_FAILURE);
    }

    int listener_descriptor = listen(socket_descriptor, 5);

    if (listener_descriptor == -1)
    {
        std::cout << "Cannot listen in socket " << socket_descriptor << ". Exiting." << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "Port has been binded to " << port << "!" << std::endl;

    do
    {
        new_conn_size = sizeof(in_conn);
        connection_descriptor = accept(socket_descriptor, (struct sockaddr *)&in_conn, &new_conn_size);
        if (connection_descriptor == -1)
        {
            std::cout << "Could not accept a connection! :(" << std::endl;
            continue;
        }
        std::cout << "Connection has been achieved" << std::endl;
        User new_user;
        inet_ntop(AF_INET, &(in_conn.sin_addr), new_user.ip, INET_ADDRSTRLEN);

        std::cout << "Reached" << std::endl;
        new_user.socket_descriptor = connection_descriptor;

        pthread_t tid;
        pthread_attr_t attrs;
        pthread_attr_init(&attrs);
        pthread_create(&tid, &attrs, do_something, (void *)&new_user);

    } while (true);
}