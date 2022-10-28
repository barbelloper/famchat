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
#include "chat_client.h"
#include <python3.6m/Python.h>

#define MAXLINE     1000
#define NAME_LEN    20

int ChatClient::connectToServer()
{
    int ret = -1;
    if (ChatClient::is_set_server_info)
    {
        if ((ChatClient::listen_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            printf("%s %d \n", __func__, __LINE__);
        }
        else
        {
            memset(&(ChatClient::servaddr), 0, sizeof(ChatClient::servaddr));
            ChatClient::servaddr.sin_family = AF_INET;;
            inet_pton(AF_INET, ChatClient::server_ip, &servaddr.sin_addr);
            ChatClient::servaddr.sin_port = htons(ChatClient::port);
            ChatClient::is_set_server_info = false;
            if (connect(ChatClient::listen_socket, (struct sockaddr *)&(ChatClient::servaddr), sizeof(ChatClient::servaddr)) < 0)
            {
                printf("%s %d \n", __func__, __LINE__);
            }
            else
            {
                ret = listen_socket;
            }
        }
    }  
    return ret;
}

void ChatClient::setServerInfo(const char *servip, uint16_t port)
{
    strcpy(ChatClient::server_ip ,servip);
    ChatClient::port = port;
    ChatClient::is_set_server_info = true;
}

int main(int argc, char *argv[]) 
{
	char bufname[NAME_LEN];
	char bufmsg[MAXLINE];	
	char bufall[MAXLINE + NAME_LEN];
	int max_fd_p_1;	
	int sock;
	int name_len;
	fd_set read_fds;
	time_t ct;
	struct tm tm;
    ChatClient client;

	if (argc != 4) 
    {
		printf("usage : %s sever_ip  port name \n", argv[0]);
		exit(0);
	}

	client.setServerInfo(argv[1], atoi(argv[2]));
    sock = client.connectToServer();
	if (sock == -1)
    {
		exit(1);
    }

    printf("Connected to Server!!\n");
	max_fd_p_1 = sock + 1;
	FD_ZERO(&read_fds);

	while (1) 
	{
		FD_SET(0, &read_fds);
		FD_SET(sock, &read_fds);
		if (select(max_fd_p_1, &read_fds, NULL, NULL, NULL) < 0)
        {
			exit(1);
        }
		if (FD_ISSET(sock, &read_fds)) 
        {
			int nbyte;
			if ((nbyte = recv(sock, bufmsg, MAXLINE, 0)) > 0) 
            {
				bufmsg[nbyte] = 0;
				write(1, "\033[0G", 4);
				printf("%s", bufmsg);
				fprintf(stderr, "\033[1;32m");
				fprintf(stderr, "%s>", argv[3]);
			}
		}
		if (FD_ISSET(0, &read_fds)) 
        {
			if (fgets(bufmsg, MAXLINE, stdin)) 
            {
				fprintf(stderr, "\033[1;33m");
				fprintf(stderr, "\033[1A");
				ct = time(NULL);
				tm = *localtime(&ct);
				if (strstr(bufmsg, "!위치") != NULL)
				{
					Py_Initialize();
					FILE *file = fopen("script/location.py", "r");
					if(file != NULL) 
					{
						PyRun_SimpleFile(file, "script/location.py");
						fclose(file);
						FILE *fp = fopen("location.txt", "rw+");
						fseek(fp, 0, SEEK_END);
						auto fsize = ftell(fp);
						char *bufloc = new char[fsize+1];
						fseek(fp, 0, SEEK_SET);
						fread(bufloc, fsize, 1, fp);
						bufloc[fsize] = '\n';
						sprintf(bufall, "[%02d:%02d:%02d]%s> 현재위치: %s", tm.tm_hour, tm.tm_min, tm.tm_sec, argv[3], bufloc);
						fclose(fp);
						delete bufloc;
						bufloc = NULL;
					}
				}
				else
				{
					sprintf(bufall, "[%02d:%02d:%02d]%s>%s", tm.tm_hour, tm.tm_min, tm.tm_sec, argv[3], bufmsg);
				}
				if (send(sock, bufall, strlen(bufall), 0) < 0)
				{
					printf("Error : Write error on socket.");
				}
				if (strstr(bufmsg, "exit") != NULL) 
				{
					printf("Good bye.");
					close(sock);
					exit(0);
				}
			}
		}
	}
}