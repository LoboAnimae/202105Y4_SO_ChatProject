#include <iostream>
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
#include <map>
#include <vector>
#include <bits/stdc++.h>
#include <time.h>
#define BUFFERSIZE 4096
#define ACTIVE "ACTIVE"
#define INACTIVE "INACTIVE"
#define BUSY "BUSY"

#define INFO "    INFO   "
#define WARNING "  WARNING  "
#define ERROR "   ERROR   "
#define FATAL "FATAL_ERROR"
#define SUCCESS "  SUCCESS  "

struct User
{
    char ip[INET_ADDRSTRLEN];
    std::string username;
    std::string status;
    int socket_descriptor;
};

std::map<std::string, User> registered_users;

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
    std::map<std::string, User>::iterator iterator;

    User temp;
    for (iterator = registered_users.begin(); iterator != registered_users.end(); iterator++)
    {
        temp = iterator->second;

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

bool send_all_connected_users(int local_socket_descriptor)
{

    auto sub_request = new chat::ConnectedUsersResponse;
    auto request = new chat::ServerResponse;
    std::list<chat::UserInfo>::iterator userinfo_iterator;
    std::list<chat::UserInfo> current_active_users;
    std::string serialized_message, logger;
    char buffer[BUFFERSIZE];
    int i;

    logger = "Fetching all users for socket " + std::to_string(local_socket_descriptor);
    create_log(&logger, INFO);

    std::map<std::string, User>::iterator iterator;

    iterator = registered_users.begin();

    while (iterator != registered_users.end())
    {
        User current = iterator->second;
        // if (current.status == "ACTIVO")
        // {
        std::string temp_ip;
        std::stringstream ss;
        ss << current.ip;
        temp_ip = ss.str();

        auto adder = sub_request->add_connectedusers();
        adder->set_username(current.username);
        adder->set_status(current.status);
        adder->set_ip(temp_ip);
        // }
        iterator++;
    }

    request->set_option(2);
    request->set_code(200);
    request->set_allocated_connectedusers(sub_request);
    request->SerializeToString(&serialized_message);

    strcpy(buffer, serialized_message.c_str());

    send(local_socket_descriptor, buffer, serialized_message.size() + 1, 0);
    return true;
}

bool change_status(chat::ChangeStatus body, std::string username)
{
    std::map<std::string, User>::iterator ref;
    std::string logger, new_status;

    new_status = body.status();

    ref = registered_users.find(username);
    ref->second.status = new_status;

    // if (new_status == ACTIVE || new_status == BUSY || new_status == INACTIVE)
    // {
    //     logger = "A valid status could not be found while trying to change " + (*modifiable_user).status + "'s status to " + body.status();
    //     create_log(&logger, ERROR);
    //     return false;
    // }
    logger = "Status has been changed for " + username + " to " + new_status;
    create_log(&logger, SUCCESS);
    return true;
}

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
    std::string logger;
    auto response = new chat::ServerResponse;

    bool user_has_registered;
    bool success;
#pragma endregion

    logger = "Created a thread for socket " + std::to_string(local_socket_descriptor);
    create_log(&logger, SUCCESS);

    do
    {
        // CLIENT DISCONNECTED, SET STATUS AS OFFLINE
        auto client_response = recv(local_socket_descriptor, buffer, BUFFERSIZE, 0);
        if (client_response < 1)
        {
            client_response = recv(local_socket_descriptor, buffer, BUFFERSIZE, 0);
            if (!client_response)
            {
                logger = "STATUS CHANGE(CLIENT DISCONNECT)";
                create_log(&logger, INFO);

                logger = "DISCONNECTING USER \"" + this_user.username + "\" (" + this_user.ip + ") FROM SERVER.";
                create_log(&logger, INFO);
                auto iter = registered_users.find(this_user.username);
                iter->second.status = INACTIVE;

                logger = "SET USER \"" + this_user.username + "\" (" + this_user.ip + ") AS " + INACTIVE;
                create_log(&logger, SUCCESS);

                if (close(local_socket_descriptor) == -1)
                {
                    logger = "Could not close socket " + std::to_string(local_socket_descriptor);
                    create_log(&logger, ERROR);
                }
                else
                {
                    logger = "Closed socket " + std::to_string(local_socket_descriptor);
                    create_log(&logger, SUCCESS);
                }
            }
            break;
        }
        client_petition.ParseFromString(buffer);

        while (this_user.username.empty())
        {
            if (client_petition.option() != 1)
            {
                continue;
            }

            chat::UserRegistration registration_form = (chat::UserRegistration)client_petition.registration();
            auto iter = registered_users.find(registration_form.username());
            if (iter != registered_users.end())
            {
                logger = "Pre-existent user. Setting as active and continuing.";
                create_log(&logger, INFO);
                iter->second.status = ACTIVE;
                this_user = iter->second;
            }
            else
            {
                logger = "Found a new user with username " + registration_form.username() + ". Attempting to register.";
                create_log(&logger, INFO);

                this_user.username = registration_form.username();
                strcpy(this_user.ip, registration_form.ip().c_str());
                logger = "User in socket " + std::to_string(local_socket_descriptor) + " (" + this_user.username + ") has been registered.";
                create_log(&logger, SUCCESS);

                // Add to the list of registered users
                this_user.status = ACTIVE;
                registered_users.insert(std::pair<std::string, User>(this_user.username, this_user));
            }
            // Send correct registration
            chat::ServerResponse *correct_registration_response = new chat::ServerResponse();

            correct_registration_response->set_option(1);
            correct_registration_response->set_code(200);
            std::string response;
            correct_registration_response->SerializeToString(&response);

            strcpy(buffer, response.c_str());
            send(local_socket_descriptor, buffer, response.size() + 1, 0);
        }
        // --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        switch (client_petition.option())
        {
            // Connected Users
        case 2:
            success = send_all_connected_users(local_socket_descriptor);
            if (success)
            {
                logger = "Sent all connected users to the client in socket " + std::to_string(local_socket_descriptor);
                create_log(&logger, SUCCESS);
            }
            else
            {
                logger = "There was an error trying to send all the connected users to the client in socket " + std::to_string(local_socket_descriptor);
                create_log(&logger, ERROR);
            }
            break;
            // Status Change
        case 3:
            success = change_status(client_petition.change(), this_user.username);

            response->set_option(3);
            if (success)
            {
                response->set_code(200);
            }
            else
            {
                response->set_code(500);
            }
            success = false;

            response->SerializeToString(&serialized_sent_message);
            strcpy(buffer, serialized_sent_message.c_str());

            send(local_socket_descriptor, buffer, serialized_sent_message.size() + 1, 0);
            break;
            // Fetch messages
        case 4:
            break;
            // User Information
        case 5:
            break;
        }

    } while (true);
    pthread_exit(NULL);
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
        logger = "Could not bind to socket";
        create_log(&logger, FATAL);
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

        logger = "A new client has connected. Creating thread for socket " + std::to_string(connection_descriptor);
        create_log(&logger, INFO);
        pthread_t tid;
        pthread_attr_t attrs;
        pthread_attr_init(&attrs);
        pthread_create(&tid, &attrs, client_process_threading, (void *)&new_user);

    } while (true);
}