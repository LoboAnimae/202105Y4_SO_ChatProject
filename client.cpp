// Client side C/C++ program to demonstrate Socket programming
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <queue>
#include "protocol.pb.h"
#include "errors.h"
#include <sstream>
#define BUFFERSIZE 4096

// Variable initialization
int connected_to_server;
int waiting_server;
int waiting_user;
int program_running;

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

int main(int argc, char const *argv[])
{
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
    std::cout << "=====================================" << std::endl;
    std::cout << "Welcome to " << server_ip << ":" << server_port << ", " << username << ".\nWhat would you like to do?" << std::endl;
    // After this point, the user is logged in, and must be registered in the server.

    int sockfd, successful_connection_fd, byte_num, reallocV;
    char buffer[BUFFERSIZE];
    std::string s;

    struct addrinfo hints, *server_info, *p;

    pthread_t server_message_fetcher_thread;
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    reallocV = getaddrinfo(server_ip.c_str(), server_port.c_str(), &hints, &server_info);
    if (reallocV != 0)
    {
        create_error(CONNERR, true);
    }

    for (p = server_info; p; p = p->ai_next)
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

    void *checkpoint = get_info((struct sockaddr *)p->ai_addr->sa_data);

    inet_ntop(p->ai_family, checkpoint, (char *)s.c_str(), sizeof(s));
    freeaddrinfo(server_info);
    // And, we're connected :D (In theory...)

    // Try to register the user
    chat::UserRegistration *user_registration_obj = new chat::UserRegistration;

    // RULE OF THUMB
    // set_ALLOCATED => Usage with variables
    // set_ => Usage with literals
    user_registration_obj->set_allocated_username(&username);
    user_registration_obj->set_allocated_ip(&s);

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

    chat::ClientPetition *petition = new chat::ClientPetition;

    petition->set_allocated_registration(user_registration_obj);

    std::string serialized_registration;
    petition->SerializeToString(&serialized_registration);

    strcpy(buffer, serialized_registration.c_str());
    send(sockfd, buffer, serialized_registration.size() + 1, 0);
    recv(sockfd, buffer, sizeof(chat::ServerResponse), 0);
}
