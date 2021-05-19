#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>

#define INVALID_IP_ADDRESS "INVALID SERVER IP ADDRESS"
#define INVALID_INPUT "INVALID INPUT FOUND"
#define CONNERR "CONNECTION ERROR"

void ask_information(std::string question, std::string *variable);
void create_error(std::string error, bool catastrophic);
void get_connection_data(std::string *username, std::string *ip, std::string *port);
bool check_valid_ip(std::string *ip);