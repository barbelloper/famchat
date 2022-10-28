#ifndef CHAT_SERVER_
#define CHAT_SERVER_
#include <iostream> 
#include <vector> 
#include <utility> 
#include <thread> 
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <cstring>
#include <mutex>
#include <unistd.h>
#include <arpa/inet.h>

#define MAXSOCKET 1024
#define PACKET_SIZE 1024
#define MAXLINE  511

class ChatServer
{
private:
    typedef int32_t socket_t;
    socket_t listen_socket;
    socket_t accept_socket;
    fd_set read_fds;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    bool is_open_socket;
    char message_buffer[MAXLINE + 1];
    int max_cl_fd_plus_1;
    int num_user;
    int num_chat;	
    int clisock_list[MAXSOCKET];
    char ip_list[MAXSOCKET][20];

    bool cmd_listener_done;
    bool cmd_listener_start;

    std::mutex mtx;
public:
    ChatServer(/* args */);
    ~ChatServer(); 
    int openSocket();
    int readyServer(uint16_t port);
    void messageReceiveThreadWorker();
    int checkClient();
    int addClient();
    void broadcastToAllClient();
    void removeClient(socket_t socket);
    bool IsAliveSever();
    int getMax(); 
};

ChatServer::ChatServer(/* args */)
{
    max_cl_fd_plus_1 = 0;
    listen_socket = -1;
    accept_socket = -1;
    is_open_socket = false;
    cmd_listener_done = false;
    cmd_listener_start = true;
    num_user = 0;
    num_chat = 0;
    (void)memset(message_buffer, 0, sizeof(message_buffer));
    (void)memset(clisock_list, 0, sizeof(clisock_list));
    (void)memset(ip_list, 0, sizeof(ip_list));
}

ChatServer::~ChatServer()
{
}

#endif