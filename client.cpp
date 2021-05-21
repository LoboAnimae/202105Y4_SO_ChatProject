// Client side C/C++ program to demonstrate Socket programming
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netinet/in.h>
#include <netdb.h>
// #include <ifaddrs.h>
#include <queue>
#include <list>
#include "protocol.pb.h"
#include "errors.h"
#include <bits/stdc++.h>
#include <sstream>
#include <string>
#include <string.h>
#define BUFFERSIZE 8000

// Variable initialization
int connected_to_server;
int waiting_server;
int waiting_user;
int program_running;

struct params_struct
{
    int local_socket;
};
struct Addresses
{
    in_addr *sock1 = nullptr;
    in6_addr *sock2 = nullptr;
    int number;
} address_controller;

void ask_information(std::string question, std::string *variable)
{
    std::string input;
    std::cout << question;
    std::cin >> input;

    if (std::cin.fail())
        create_error(INVALID_INPUT, true);
    *variable = input;
}

void create_error(std::string error, bool catastrophic = false)
{
    std::string alert_type = catastrophic ? "ERROR: " : "WARNING: ";
    std::cout << "\033[31m" << alert_type << error << "\033[0m" << std::endl;
    if (catastrophic)
    {
        exit(EXIT_FAILURE);
    }
}

void get_connection_data(std::string *username, std::string *ip, std::string *port)
{
    ask_information("Enter your username: ", username);
    ask_information("What server IP will we be connecting to? (Without a port): ", ip);
    ask_information("Please specify a port: ", port);
}

bool check_valid_ip(std::string *ip)
{
    int dot_count = 0;
    int dot_needed = 3;

    int i;
    for (i = 0; i < (*ip).length(); i++)
    {
        // std::cout << (*ip)[i] << std::endl;

        if ((*ip)[i] == '.')
        {
            // std::cout << "Found a dot" << std::endl;
            dot_count++;
        }
    }

    // std::cout << "Final dot count: " << dot_count << std::endl;

    return dot_count == dot_needed;
}

void *get_info(struct sockaddr *socket_address)
{
    if (socket_address->sa_family == AF_INET)
        return &(((struct sockaddr_in *)socket_address)->sin_addr);
    return &(((struct sockaddr_in6 *)socket_address)->sin6_addr);
}

void print_header(std::string server_ip, std::string server_port, std::string username)
{

    std::cout << "=====================================" << std::endl;
    std::cout << "Welcome to " << server_ip << ":" << server_port << ", " << username << ".\nWhat would you like to do?" << std::endl;
    std::cout << "=====================================\n\n"
              << std::endl;
}

void print_menu(int menu)
{
    if (menu == 1)
    {
        std::cout << "1. Go to the General Chat" << std::endl;
        std::cout << "2. Send a Private Message" << std::endl;
        std::cout << "3. Change your Status" << std::endl;
        std::cout << "4. List all active users" << std::endl;
        std::cout << "5. Get information about a specific user" << std::endl;
        std::cout << "6. Help" << std::endl;
        std::cout << "7. Exit" << std::endl;
        std::cout << ">>> ";
    }
    else if (menu == 2)
    {
        std::cout << "1. Fetch messages\n2. Send a message\n3. Send a Private Message\n4. Go back\n>>> ";
    }
    else if (menu == 3)
    {
        std::cout << "1. Active\n2. Busy\n3. Invisible (Inactive)\n4. Go back\n>>> ";
    }
}

/**
 * @brief Fetches all the students from the server
 * 
 * @param socket_file_descriptor Socket File Descriptor to access the Socket
 * @return true If successful
 */
bool fetch_all_users(int socket_file_descriptor)
{
    auto request = new chat::ClientPetition();
    auto body = new chat::UserRequest();
    std::string serialized_message;
    chat::ServerResponse wrapper;
    chat::ConnectedUsersResponse container;
    chat::UserInfo response;
    char buffer[BUFFERSIZE];

    body->set_user("everyone");

    request->set_option(2);
    request->set_allocated_users(body);
    request->SerializeToString(&serialized_message);

    strcpy(buffer, serialized_message.c_str());

    send(socket_file_descriptor, buffer, serialized_message.size() + 1, 0);

    system("clear");
    std::cout << "Waiting for server response..." << std::endl;
    recv(socket_file_descriptor, buffer, BUFFERSIZE, 0);
    system("clear");

    wrapper.ParseFromString(buffer);
    if (wrapper.code() == 200)
    {
        container = (chat::ConnectedUsersResponse)wrapper.connectedusers();
        int size = container.connectedusers_size();
        std::cout << "Total number of users: " << size << std::endl;
        std::cout << "Username\t\tStatus\t\tIP" << std::endl;

        int i;
        for (i = 0; i < size; i++)
        {
            response = (chat::UserInfo)container.connectedusers(i);
            std::cout << response.username() << "\t\t" << response.status() << "\t\t" << response.ip() << std::endl;
        }
        std::string waiter;
        std::cout << "Press enter to go back..." << std::endl;
        getline(std::cin, waiter);
    }
}

/**
 * @brief Sends a request to the server to change status
 * @param buffer Pointer to the buffer so that there is no memory duplicity
 * @param socket_file_descriptor File descriptor for the socket
 * @param new_status 1 = Active; 2 = Busy; 3 = Inactive
 * @param username The user's username
 * @returns True if successful, False otherwise 
 */
bool change_status_request(int socket_file_descriptor, int new_status, std::string *username)
{
    std::string status;
    if (new_status == 1)
        status = "ACTIVE";
    else if (new_status == 2)
        status = "BUSY";
    else if (new_status == 3)
        status = "INACTIVE";
    // Instantiate our variables
    std::string serialized_request;
    char buffer[BUFFERSIZE];

    auto body = new chat::ChangeStatus;
    auto request = new chat::ClientPetition;
    chat::ServerResponse response;

    // Fill our request
    body->set_allocated_username(username);
    body->set_status(status);

    // Serialize the request
    request->set_option(3);
    request->set_allocated_change(body);
    request->SerializeToString(&serialized_request);

    // Prepare the data in the buffer
    strcpy(buffer, serialized_request.c_str());

    // Send the request
    send(socket_file_descriptor, buffer, serialized_request.size() + 1, 0);

    recv(socket_file_descriptor, buffer, BUFFERSIZE, 0);
    response.ParseFromString(buffer);

    if (response.code() != 200)
    {
        return false;
    }

    return true;
}

bool send_message(int local_socket_descriptor, std::string username, std::string recipient = "everyone")
{
    std::cout << ">>> ";
    std::string text;
    getline(std::cin, text);

    std::cout << "Your message: " << text << std::endl;
    std::cout << text.empty() << std::endl;
    if (text.empty())
        return false;

    chat::MessageCommunication *message = new chat::MessageCommunication();
    chat::ClientPetition *request = new chat::ClientPetition();
    std::string serialized_message;
    char buffer[BUFFERSIZE];

    message->set_allocated_message(&text);
    message->set_recipient(recipient);
    message->set_sender(username);

    request->set_option(4);
    request->set_allocated_messagecommunication(message);
    request->SerializeToString(&serialized_message);
    strcpy(buffer, serialized_message.c_str());

    send(local_socket_descriptor, buffer, serialized_message.size() + 1, 0);
    getline(std::cin, text);
}
bool input_detected = false;

std::list<std::string> messages;
bool fetcher_active = false;
void *fetch_message(void *params)
{
    auto local_socket_descriptor = ((int *)params);
    char buffer[BUFFERSIZE];
    chat::ServerResponse response;
    chat::MessageCommunication message_manager;
    while (fetcher_active)
    {
        recv(*local_socket_descriptor, buffer, BUFFERSIZE, 0);
        response.ParseFromString(buffer);

        if (response.code() == 200)
        {
            message_manager = (chat::MessageCommunication)response.messagecommunication();
            std::cout << message_manager.message() << std::endl;
        }
        if (input_detected)
            break;
    }
    pthread_exit(NULL);
}

void *detect_input(void *params)
{
    std::string placeholder;
    getline(std::cin, placeholder);
    input_detected = true;
    pthread_exit(NULL);
}

void go_to_general_chat(int local_socket_descriptor, std::string username)
{
    int size = messages.size();
    fetcher_active = true;
    pthread_t message_fetcher_thread, input_fetcher_thread;
    pthread_attr_t attr, attr_ph;
    std::list<std::string>::iterator iter;

    // Get the messages
    pthread_attr_init(&attr);
    pthread_create(&message_fetcher_thread, &attr, fetch_message, (void *)&local_socket_descriptor);

    // Expect the user to try and input anything with ENTER key

    pthread_attr_init(&attr_ph);
    pthread_create(&input_fetcher_thread, &attr_ph, detect_input, 0);
    while (true)
    {
        if (size != messages.size())
        {
            for (iter = messages.begin(); iter != messages.end(); iter++)
            {
                for (int i = 0; i != size; i++)
                {
                    iter = std::next(iter);
                }
                std::cout << (*iter) << std::endl;
            }
            size = messages.size();
        }
        // detect the input
        if (input_detected)
        {
            fetcher_active = false;
            return;
        }
    }
}

void grab_user_info(int local_socket_descriptor)
{
    std::string username;

    std::cout << "Please specify a username: ";
    getline(std::cin, username);
    chat::UserRequest *usr = new chat::UserRequest;
    chat::ClientPetition *request = new chat::ClientPetition();

    usr->set_allocated_user(&username);
    request->set_option(5);
    request->set_allocated_users(usr);
    std::string serialized_message;
    request->SerializeToString(&serialized_message);
    char buffer[BUFFERSIZE];
    strcpy(buffer, serialized_message.c_str());
    send(local_socket_descriptor, buffer, serialized_message.size() + 1, 0);

    chat::ServerResponse response;
    recv(local_socket_descriptor, buffer, BUFFERSIZE, 0);
    response.ParseFromString(buffer);

    if (response.code() == 200)
    {
        chat::UserInfo usr = response.userinforesponse();

        std::cout << "User information:\n\t- Username: " << usr.username() << "\n\t- Status: " << usr.status() << "\n\t- IP: " << usr.ip() << std::endl;
    }
    else
    {
        std::cout << "No users found under that username." << std::endl;
    }
    std::string placeholder;
    std::cout << "Press enter to continue...";
    getline(std::cin, placeholder);
}

int main(int argc, char const *argv[])
{

#pragma region CONNECTION
    bool set_values = false;
    std::string input;
    if (argc != 4)
    {
        std::cout << "Oh no! An incorrect number of arguments were given. Would you like to set them now? (Y/n)";
        getline(std::cin, input);

        if (input == "n" || input == "N")
        {
            std::cout << "Exiting" << std::endl;
            return 0;
        }
        else
        {
            set_values = true;
        }
    }

    GOOGLE_PROTOBUF_VERIFY_VERSION;
    // Create the variables and assign null as protected checking

    std::string username, server_ip, server_port;

    connected_to_server = -1;
    waiting_server = -1;
    waiting_user = -1;
    program_running = 1;

    if (set_values)
    {
        get_connection_data(&username, &server_ip, &server_port);
    }
    else
    {
        username = argv[1];
        server_ip = argv[2];
        server_port = argv[3];
    }
    if (!check_valid_ip(&server_ip))
    {
        create_error(INVALID_IP_ADDRESS, true);
    }
    std::cout << "Your username has been set as \"" << username << "\" and you will be connecting to the server " << server_ip << ":" << server_port << std::endl;
    std::cout << "Is this correct? (Y/n) ";

    getline(std::cin, input);
    if (input == "n" || input == "N")
    {
        std::cout << "Ending program." << std::endl;
        return 0;
    }

    std::cout << "Transaction completed." << std::endl;

    system("clear");
    // After this point, the user is logged in, and must be registered in the server.

    int sockfd, successful_connection_fd, byte_num, reallocV;
    char buffer[BUFFERSIZE];
    char local_ip_address[INET6_ADDRSTRLEN];

    struct addrinfo addr, *server_info, *p;

    pthread_t server_message_fetcher_thread;
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    memset(&addr, 0, sizeof(addr));
    addr.ai_family = AF_UNSPEC;
    addr.ai_socktype = SOCK_STREAM;
    reallocV = getaddrinfo(server_ip.c_str(), server_port.c_str(), &addr, &server_info);
    if (reallocV != 0)
    {
        create_error(CONNERR, true);
    }
    p = server_info;

    while (p)
    {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1)
            continue;

        successful_connection_fd = connect(sockfd, p->ai_addr, p->ai_addrlen);
        if (successful_connection_fd == -1)
        {
            close(sockfd);
            continue;
        }
        break;
    }

    if (!p)
        create_error(CONNERR, true);

    auto *checkpoint = ((struct sockaddr *)p->ai_addr);
    void *given_local_address = nullptr;

    if (checkpoint->sa_family == AF_INET)
        given_local_address = &(((struct sockaddr_in *)checkpoint)->sin_addr);
    else
        given_local_address = &(((struct sockaddr_in6 *)checkpoint)->sin6_addr);

    inet_ntop(p->ai_family, given_local_address, local_ip_address, sizeof(local_ip_address));
    freeaddrinfo(server_info);
    // And, we're connected :D (In theory...)

    // Try to register the user
    chat::UserRegistration *user_registration_obj = new chat::UserRegistration;

    // RULE OF THUMB
    // set_ALLOCATED => Usage with variables
    // set_ => Usage with literals
    user_registration_obj->set_allocated_username(&username);
    user_registration_obj->set_ip(local_ip_address);

    /*
        let reg = {one: property, two: property...}
         |------------------------------|
         |   user_registration_obj      |
         |------------------------------| <------|
                                                 |
        petition -> registration -> ptr ---------|

        |string N*char| int32 | int64 |...| ||||
        |               string              |
        |10111011010100101010101010101010101|

        sizeof(int) == sizeof(char) <-- TRUE
    */

    chat::ClientPetition *petition = new chat::ClientPetition();

    petition->set_option(1);
    petition->set_allocated_registration(user_registration_obj);

    std::string serialized_registration;
    petition->SerializeToString(&serialized_registration);

    strcpy(buffer, serialized_registration.c_str());
    send(sockfd, buffer, serialized_registration.size() + 1, 0);
    chat::ServerResponse server_response;
    system("clear");
    printf("Loading...");
    int decoration = 0;
    do
    {
        if (!(decoration % 1000))
        {
            printf(".");
        }
        decoration++;
        auto server_response_socket = recv(sockfd, buffer, BUFFERSIZE, 0);

        // No response
        if (server_response_socket < 1)
        {
            continue;
        }
        server_response.ParseFromString(buffer);
        std::cout << server_response.code() << std::endl;
        if (server_response.code() != 200)
        {
            std::cout << "Server Error" << std::endl;
        }
        if (server_response.option() == 1)
        {
            break;
        }
    } while (true);
#pragma endregion
    do
    {
        system("clear");
        std::string temp_option;
        int suboption, option;
        print_header(server_ip, server_port, username);
        print_menu(1);
        bool success;

        getline(std::cin, temp_option);
        option = atoi(temp_option.c_str());

        std::string recipient;
        system("clear");
        switch (option)
        {
            // Go to the general chat (fetch messages)
        case 1:
#pragma region MESSAGES
            print_header(server_ip, server_port, username);
            temp_option = "";
            print_menu(2);
            getline(std::cin, temp_option);
            suboption = atoi(temp_option.c_str());
#pragma region SUBMENU
            switch (suboption)
            {
                // Go fetch the messages
            case 1:
#pragma region FETCHMESSAGES
                go_to_general_chat(sockfd, username);
                break;
#pragma endregion
                // Allow the user to send a message
            case 2:
#pragma region SENDMESSAGE
                send_message(sockfd, username);
                break;
#pragma endregion
            case 3:
#pragma region SENDPRIVATEMESSAGE
                std::cout << "Please write the username of the recipient: ";
                getline(std::cin, recipient);
                send_message(sockfd, username, recipient);
                break;
#pragma endregion
            default:
                break;
            }
#pragma endregion
            break;
#pragma endregion
            // Try to send a direct message
        case 2:
#pragma region DIRECT MESSAGES
            std::cout << "Please write the name of your recipient: ";
            getline(std::cin, recipient);
            send_message(sockfd, username, recipient);
            break;
#pragma endregion

            // Change the status
        case 3:
#pragma region STATUSCHANGE
            temp_option = "";
            print_menu(3);
            getline(std::cin, temp_option);

            suboption = atoi(temp_option.c_str());
            if (suboption > 0 && suboption < 4)
            {
                bool success;

                success = change_status_request(sockfd, suboption, &username);

                if (success)
                {
                    std::cout << "Status has been changed" << std::endl;
                }
                else
                {
                    std::cout << "There was a problem with the server and it returned code 500" << std::endl;
                }
                std::cout << "Press enter to continue" << std::endl;
                getline(std::cin, temp_option);
            }
            break;
#pragma endregion
            // List active users in the chat
        case 4:
#pragma region LISTACTIVEUSERS
            success = fetch_all_users(sockfd);
            break;
#pragma endregion
            // Show information about a user
        case 5:
#pragma region SHOWINFOABOUTUSER
            grab_user_info(sockfd);
            break;
#pragma endregion
            // Help
        case 6:
#pragma region HELP
            break;
#pragma endregion
            // Exit
        case 7:
#pragma region EXIT
            std::cout << "Exited program. Press enter to continue...";
            getline(std::cin, temp_option);
            close(sockfd);
            exit(EXIT_SUCCESS);
            break;
#pragma endregion

#pragma region OTHER
        default:
            std::cout << "Unrecognized option" << std::endl;
            break;
#pragma endregion
        }
    } while (true);
}
