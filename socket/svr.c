#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int main(int argc, char **argv)
{
    int server_sockfd, client_sockfd;
    int state, client_len;
    int pid;
	int ccc;

    FILE *fp;
    struct sockaddr_in clientaddr, serveraddr;

    char buf[255];
    char line[255];

    if (argc != 2)
    {
        printf("Usage : ./zipcode [port]\n");
        printf("��  : ./zipcode 4444\n");
        exit(0);
    }

    memset(line, '0', 255);
    state = 0;

    // �ּ� ������ �о���δ�.
    client_len = sizeof(clientaddr);
    if((fp = fopen("zipcode.txt", "r")) == NULL)
    {
        perror("file open error : ");
        exit(0);
    }

    // internet ����� ���� ���� (INET)
    if ((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket error : ");
        exit(0);
    }
    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(atoi(argv[1]));

    state = bind(server_sockfd , (struct sockaddr *)&serveraddr,
            sizeof(serveraddr));
    if (state == -1)
    {
        perror("bind error : ");
        exit(0);
    }

    state = listen(server_sockfd, 5);
    if (state == -1)
    {
        perror("listen error : ");
        exit(0);
    }

    while(1)
    {
        client_sockfd = accept(server_sockfd, (struct sockaddr *)&clientaddr,
                               &client_len);
        if (client_sockfd == -1)
        {
            perror("Accept error : ");
            exit(0);
        }
        while(1)
        {
            rewind(fp);
            memset(buf, '0', 255);
            //if (read(client_sockfd, buf, 255) < 0)
            ccc = read(client_sockfd, buf, 255);
			if (ccc < 0 )
            {
                close(client_sockfd);
                break;
            }
			else if( !ccc ){
				printf("ZERO, errno=%d:%s] ==> BREAK\n", errno, strerror(errno));
				close(client_sockfd);
				break;
			}

            if (strncmp(buf, "quit",4) == 0)
            {
                write(client_sockfd, "bye bye", 8);
                close(client_sockfd);
                break;
            }

            while(fgets(line,255,fp) != NULL)
            {
                if (strstr(line, buf) != NULL)
                {
                    write(client_sockfd, line, 255);
                }
                memset(line, '0', 255);
            }
            write(client_sockfd, "end", 255);
            printf("send end\n");
        }
    }
    close(client_sockfd);
}
