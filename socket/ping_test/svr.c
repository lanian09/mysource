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

#define BEACON_FILEHEADER_SIZE      24
char beacon_file_header[BEACON_FILEHEADER_SIZE] =
    {   0x31, 0x30, 0x30, 0x33,
        0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20  };

int g_fd;

int open_file()
{
	if( (g_fd = open("./beacon_header.index", O_CREAT|O_EXCL|O_RDWR, 0777)) < 0 ){
		if( errno == EEXIST ){
			if(( g_fd = open("./beacon_header.index", O_CREATE |O_APPEND|O_RDWR, 0777)) < 0 ){
				printf("error open error=%d:%s\n", errno, strerror(errno));
				return -1;
			}
		}
	}
	write = (g_fd, beacon_file_header, BEACON_FILEHEADER_SIZE, );
	return 0;
}

int write_packet_file(char *buf)
{
	
}

int handle_beacon(char *buf)
{
	open_file();

	write_packet_file(buf);

	return 0;
}

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
        printf("Usage : ./a.out [port]\n");
        printf("예  : ./a.out 4444\n");
        exit(0);
    }

    memset(line, '0', 255);
    state = 0;

    // 주소 파일을 읽어들인다.
    client_len = sizeof(clientaddr);

    // internet 기반의 소켓 생성 (INET)
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
            memset(buf, '0', 255);
            //if (read(client_sockfd, buf, 255) < 0)
            ccc = read(client_sockfd, buf, 255);
			if (ccc < 0 )
            {
				printf("minus return, errno=%d:%s]\n", errno, strerror(errno));
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

			if( ccc ){
				printf("sizeof rcved-socket=%d\n", ccc);
				sleep(1);
			}
        }
    }
    close(client_sockfd);
}
