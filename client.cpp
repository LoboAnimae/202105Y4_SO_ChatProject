// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include "protocol.pb-c.h"
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
using namespace google::protobuf::io;
using namespace std;

int main(int argc, char const *argv[])
{
    _Chat__UserRegistration registration = CHAT__USER_REGISTRATION__INIT;
    registration.ip = (char *)"192.168.0.10";
    registration.username = (char *)"User1";

    int size = sizeof(registration);
    printf("%d", size);
}
