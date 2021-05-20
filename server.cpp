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
#include <list>
#include <bits/stdc++.h>
#include <time.h>
#define BUFFERSIZE 4096
#define ACTIVE "ACTIVE"
#define INACTIVE "INACTIVE"
#define BUSY "BUSY"

#define INFO "INFO"
#define WARNING "WARNING"
#define ERROR "ERROR"
#define FATAL "FATAL_ERROR"
#define SUCCESS "SUCCESS"

struct User
{
    char ip[INET_ADDRSTRLEN];
    std::string username;
    std::string status;
    int socket_descriptor;
};

std::list<User> registered_users;

#pragma region FunctionDeclaration
void create_log(std::string *message, std::string log_status);
#pragma endregion

int get_user_amount()
{
    return registered_users.size();
}

int get_connected_users_amount()
{
    int i, counter;
    std::list<User>::iterator iterator;
    iterator = registered_users.begin();
    User temp;
    for (i = 0; i < get_user_amount(); i++)
    {
        advance(iterator, 1);
        temp = (User)(*iterator);

        if (temp.status == ACTIVE)
            counter++;
    }
    return counter;
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

// bool set_client_status(User *user, std::string status)
// {
//     int i;
//     std::list<User>::iterator iterator;
//     User temp_user = *user;
//     iterator = std::find(registered_users.begin(), registered_users.end(), temp_user);
//     if (iterator != registered_users.end())
//     {
//         ((User)(*iterator)).status = status;
//         return true;
//     }
//     return false;
// }

void *client_process_threading(void *params)
{
#pragma region variables
    User this_user;
    User *this_params = (User *)params;
    int local_socket_descriptor = this_params->socket_descriptor;
    std::string local_ip = this_params->ip;
    char buffer[BUFFERSIZE];

    std::string serialized_sent_message;
    chat::ClientPetition client_petition;
    std::string log_message;

#pragma endregion

    do
    {
        // CLIENT DISCONNECTED, SET STATUS AS OFFLINE
        auto client_response = recv(local_socket_descriptor, buffer, BUFFERSIZE, 0);
        if (client_response < 1)
        {
            client_response = recv(local_socket_descriptor, buffer, BUFFERSIZE, 0);
            if (!client_response)
            {
                log_message = "STATUS CHANGE(CLIENT DISCONNECT)";
                create_log(&log_message, INFO);

                log_message = "DISCONNECTING USER \"" + this_user.username + "\" (" + this_user.ip + ") FROM SERVER.";
                create_log(&log_message, INFO);
                // set_client_status(&this_user, INACTIVE);
                this_user.status = INACTIVE;

                log_message = "SET USER \"" + this_user.username + "\" (" + this_user.ip + ") AS " + this_user.status;
                create_log(&log_message, SUCCESS);
            }
            break;
        }

        client_petition.ParseFromString(buffer);

        switch (client_petition.option())
        {
            // User registration
        case 1:
            // If the user already exists, just update their status

            break;
            // Connected Users
        case 2:
            break;
            // Status Change
        case 3:
            break;
            // Fetch messages
        case 4:
            break;
            // User Information
        case 5:
            break;

        default:
            break;
        }

    } while (true);
}

void create_log(std::string *message, std::string log_status)
{
    std::string color;

    color = "\033[94m";
    if (log_status == WARNING)
        color = "\033[33m";
    if (log_status == ERROR || log_status == FATAL)
        color = "\033[31m";
    if (log_status == SUCCESS)
        color = "\033[32m";

    time_t current_time = time(NULL);
    char *t = ctime(&current_time);
    if (t[strlen(t) - 1] == '\n')
        t[strlen(t) - 1] = '\0';
    std::cout << color
              << "[" << log_status << "] "
              << "[" << t << "] "
              << "[SERVER] "
              << *message
              << "\033[37m"
              << std::endl;
}

int main(int argc, char *argv[])
{

    std::string logger;

    logger = "STARTED PROGRAM";
    create_log(&logger, INFO);
    std::string input, port;
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    if (argc != 2)
    {
        logger = "No port has been declared. Would you like to set it now? (Y/n)";
        create_log(&logger, WARNING);
        getline(std::cin, input);
        if (input == "n" || input == "N")
        {
            logger = "No port provided. Cannot Proceed.";
            create_log(&logger, FATAL);
            exit(EXIT_FAILURE);
        }
        logger = "Please input your port";
        while (true)
        {
            create_log(&logger, INFO);
            getline(std::cin, port);
            if (!port.empty())
            {
                break;
            }
        }
    }
    else
    {
        logger = "Found an argument";
        create_log(&logger, INFO);
        std::cout << std::endl; // <-- DO NOT ERASE THIS. FOR SOME REASON,
                                // IT GIVES ME A SEGMENTATION FAULT WITHOUT THIS.
                                // The only one that knows why this happens might
                                // as well to be God himself. Wtf.
        port = argv[1];
    }

    if (!valid_port(port))
    {
        logger = "Invalid port number. Exiting.";
        create_log(&logger, FATAL);
        exit(EXIT_FAILURE);
    }

    logger = "Your port has been set as " + port + ". Continue? (Y/n)";
    create_log(&logger, INFO);
    getline(std::cin, input);
    if (input == "n" || input == "N")
    {
        logger = "User exit program";
        create_log(&logger, WARNING);
        exit(EXIT_SUCCESS);
    }

    logger = "SET PORT";
    create_log(&logger, SUCCESS);

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
        logger = "Cannot listen in socket" + std::to_string(socket_descriptor) + ". Exiting.";
        create_log(&logger, FATAL);
        exit(EXIT_FAILURE);
    }

    logger = "Binded port to " + port;
    create_log(&logger, SUCCESS);

    do
    {
        new_conn_size = sizeof(in_conn);
        connection_descriptor = accept(socket_descriptor, (struct sockaddr *)&in_conn, &new_conn_size);
        if (connection_descriptor == -1)
        {
            logger = "Failed to accept a connection.";
            create_log(&logger, ERROR);
            continue;
        }
        create_log(&logger, SUCCESS);

        User new_user;
        inet_ntop(AF_INET, &(in_conn.sin_addr), new_user.ip, INET_ADDRSTRLEN);

        new_user.socket_descriptor = connection_descriptor;

        logger = "A new client has connected. Creating thread for socket" + std::to_string(connection_descriptor);
        create_log(&logger, INFO);
        pthread_t tid;
        pthread_attr_t attrs;
        pthread_attr_init(&attrs);
        pthread_create(&tid, &attrs, client_process_threading, (void *)&new_user);

    } while (true);
}