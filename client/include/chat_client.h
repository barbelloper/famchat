#ifndef CHAT_CLIENT_
#define CHAT_CLIENT_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

class ChatClient
{
private:
    typedef int32_t socket_t;
    socket_t listen_socket;
    struct sockaddr_in servaddr;
    bool is_set_server_info;
    char server_ip[256]; 
    uint16_t port;
public:
    int connectToServer();
    void setServerInfo(const char *servip, uint16_t port);
    ChatClient(/* args */);
    ~ChatClient();
};

ChatClient::ChatClient(/* args */)
{
    listen_socket = -1;
    is_set_server_info = false;
    port = 0U;
}

ChatClient::~ChatClient()
{
}

#endif