#include "chat_server.h"

int ChatServer::openSocket()
{
    int ret = -1;
    if (!is_open_socket)
    {
        ChatServer::listen_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (ChatServer::listen_socket >= 0)
        {
            is_open_socket = true;
            ret = 0;
        }
    }
    return ret;
}

int ChatServer::readyServer(uint16_t port)
{
    int ret = -1;

    if (is_open_socket)
    {
        memset(&(ChatServer::server_addr), 0, sizeof(ChatServer::server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	    server_addr.sin_port = htons(port);
        if (bind(ChatServer::listen_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) 
        {
		   std::cout << "bind fail!" << std::endl;
        }
        else
        {
            listen(ChatServer::listen_socket, SOMAXCONN);
            ret = 0;
        }
    }
    return ret;
}

void ChatServer::messageReceiveThreadWorker()
{
    int i;
    std::cout << "command list : help, num user, num_chat, ip_list, exit" << std::endl;
	while (cmd_listener_start) 
    {
        cmd_listener_done = false;
		char bufmsg[MAXLINE + 1];
		fprintf(stderr, "\033[1;32m"); 
        std::cout << "server>";
		fgets(bufmsg, MAXLINE, stdin); 
		if (!strcmp(bufmsg, "\n")) continue;  
		else if (!strcmp(bufmsg, "help\n")) 
        {
            std::cout << "help, nuser, nchat, iplist" << std::endl;
        }
        else if (!strcmp(bufmsg, "nuser\n")) 
        {
            std::cout << "number of user: " + std::to_string(num_user) << std::endl;
        }  
        else if (!strcmp(bufmsg, "nchat\n"))
        {
            std::cout << "number of chat: " + std::to_string(num_chat) << std::endl;
        }
        else if (!strcmp(bufmsg, "iplist\n"))
        {
            std::cout << "iplist: " << std::endl;
            for (i = 0; i < num_user; i++)
            {
                std::cout << "\t\t" + std::string(ip_list[i]) << std::endl;
            }
        }
        else if (!strcmp(bufmsg, "exit\n"))
        {
            std::cout << "byebye!! " << std::endl;
            cmd_listener_start = false;
        }
		else
        {
            std::cout << "worng command. help check" << std::endl;
        }
        cmd_listener_done = true;
	}
}

int ChatServer::getMax() 
{
	int max = ChatServer::listen_socket;
	int i;
	for (i = 0; i < num_user; i++)
    {
		if (clisock_list[i] > max)
        {
			max = clisock_list[i];
        }
    }
	return max;
}

int ChatServer::addClient()
{
    int ret = 0;
    socklen_t addrlen = sizeof(struct sockaddr);
    time_t ct;
    struct tm tm;
    if (FD_ISSET(ChatServer::listen_socket, &read_fds)) 
    {
        accept_socket = accept(ChatServer::listen_socket, (struct sockaddr*)&client_addr, &addrlen);
        if (accept_socket == -1)
        {
            ret = -1;
            std::cout << "accept fail" << std::endl;
        }
        else
        {
            char buf[20];
            inet_ntop(AF_INET, &client_addr.sin_addr, buf, sizeof(buf));
            write(1, "\033[0G", 4);		
            fprintf(stderr, "\033[33m");	
            printf("new client: %s\n", buf);
            
            clisock_list[num_user] = accept_socket;
            strcpy(ip_list[num_user], buf);
            num_user++; 
            send(accept_socket, "Connected to chat_server \n", strlen("Connected to chat_server \n"), 0);
            ct = time(NULL);	
            tm = *localtime(&ct);
            write(1, "\033[0G", 4);
            printf("[%02d:%02d:%02d]", tm.tm_hour, tm.tm_min, tm.tm_sec);
            fprintf(stderr, "\033[33m");
            printf("Add 1 user. num of user: %d\n", num_user);
            fprintf(stderr, "\033[32m");
            fprintf(stderr, "server>");
        }  
    }
    return ret;
}

void ChatServer::broadcastToAllClient() 
{
	for (int i = 0; i < num_user; i++) 
    {
        if (FD_ISSET(clisock_list[i], &read_fds)) 
        {
            char buf[1000];
            num_chat++;	
            int nbyte = recv(clisock_list[i], buf, MAXLINE, 0);
            if (nbyte <= 0) 
            {
                ChatServer::removeClient(i);
                continue;
            }
            buf[nbyte] = 0;
            if (strstr(buf, "exit") != NULL) 
            {
                ChatServer::removeClient(i);	
                continue;
            }
            for (int j = 0; j < num_user; j++)
            {
                send(clisock_list[j], buf, nbyte, 0);
            }
            printf("\033[0G");		
            fprintf(stderr, "\033[97m");
            printf("%s", buf);			
            fprintf(stderr, "\033[32m");
            fprintf(stderr, "server>"); 
        }
	}
}

void ChatServer::removeClient(int user_id) 
{
    time_t ct;
    struct tm tm;
	close(clisock_list[user_id]);
	if (user_id != num_user - 1) 
    {
		clisock_list[user_id] = clisock_list[num_user - 1];
		strcpy(ip_list[user_id], ip_list[num_user - 1]);
	}
	num_user--; 
	ct = time(NULL);
	tm = *localtime(&ct);
	write(1, "\033[0G", 4);		
	fprintf(stderr, "\033[33m");
	printf("[%02d:%02d:%02d]", tm.tm_hour, tm.tm_min, tm.tm_sec);
	printf("Leave 1 chat participant. Current number of participants = %d\n", num_user);
	fprintf(stderr, "\033[32m");
	fprintf(stderr, "server>");
}

int ChatServer::checkClient()
{
    int ret = 0;
    FD_ZERO(&read_fds);
    FD_SET(listen_socket, &read_fds);
    struct timeval tv;
    for (int i = 0; i < num_user; i++)
    {
        FD_SET(clisock_list[i], &read_fds);
    }
    max_cl_fd_plus_1 = getMax() + 1;
    tv.tv_sec = 3;
    tv.tv_usec = 0;
    ret = select(max_cl_fd_plus_1, &read_fds, NULL, NULL, &tv);
    if (ret < 0)
    {
        std::cout << "select error" << std::endl;
        ret = -1;
    }
    return ret;
}

bool ChatServer::IsAliveSever()
{
    bool is_alive = cmd_listener_start;
    return is_alive;
}

int main(int argc, char *argv[]) 
{
	struct sockaddr_in cliaddr;
	char buf[MAXLINE + 1]; 
	int i, j, nbyte, accp_sock, addrlen = sizeof(struct sockaddr_in);
	fd_set read_fds;
	ChatServer server;

	if (argc != 2) 
    {
        std::cout << "usage :" + std::string(argv[0]) + " port" << std::endl;
		exit(0);
	}
	server.openSocket();
	server.readyServer(atoi(argv[1]));
    std::thread cmd_listener(&ChatServer::messageReceiveThreadWorker, &server);
    cmd_listener.detach();
	while(true)
    {
        if (!(server.IsAliveSever()))   
        {
            break;
        } 
        if (server.checkClient() >= 0)
        {
            server.addClient();
            server.broadcastToAllClient();
        }
    }
    if (cmd_listener.joinable())
    {
        cmd_listener.join();
    }
	return 0;
}