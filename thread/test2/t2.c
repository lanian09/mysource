#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>

int main(int argc, char **argv){
	int clnt_sock;
	int fd, reuse=1;
	int nLen;
	char Buffer[1025];
	char lee[1025];
	fd_set	fds;
	int rtnval;
	FILE *p1, *p2;

	char *message = "하이";

	struct sockaddr_in    SvrAddr;
	struct sockaddr_in	  ClntAddr;

	if((fd=socket(AF_INET, SOCK_STREAM, 0)) < 0){
		printf("Fail to open socket.");
		exit(0);
	}

	/* TCP/IP 통신환경 설정 */
	//bzero((char *)&SvrAddr, sizeof(SvrAddr));
	memset(&SvrAddr, 0x00, sizeof(SvrAddr));
	SvrAddr.sin_family = AF_INET;
	SvrAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	SvrAddr.sin_port = htons(5000);

	/* 지정된 포트를 재사용 가능하게 설정한다. */
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse));

	/* 할당받은 소켓에 바인딩한다 */
	while(bind(fd, (struct sockaddr *)&SvrAddr, sizeof(SvrAddr)) < 0) {
		printf("Fail to binding, trying later...");
		sleep(180);
	}

	/* 서비스 리슨한다. */
	if(listen(fd, 5) < 0) {
		printf("Fail to listen");
		exit(0);
	}

	//bzero((char *)&ClntAddr, sizeof(ClntAddr));
	memset((char *)&ClntAddr, 0x00,sizeof(ClntAddr));
	nLen=sizeof(ClntAddr);

	clnt_sock=accept(fd, (struct sockaddr*)&ClntAddr, &nLen); /* 연결 요청 수락 */

	write(clnt_sock, message, sizeof(message)); /* 데이터 전송 */  

	//bzero(Buffer, 1025);
	memset(Buffer, 0x00, 1025);
	while(1) {
		FD_ZERO(&fds);
		FD_SET(clnt_sock, &fds);
		
		if(FD_ISSET(clnt_sock, &fds)){
			if((nLen=read(clnt_sock, Buffer, 1024)) <= 0) {
				/* 클라이언트가 접속을 끊었다. */
				break;
			} else {
				if( strcmp(Buffer, "server") == 0 ) {
					rtnval = fork();
					if( rtnval == 0 ) {
						system("server < yes");
						break;
					}
				} else {
					if( strcmp(Buffer, "test") == 0 ) {
						rtnval = fork();
						if( rtnval == 0 ) {
							p1 = popen("test < lee.txt", "r");
							while(1) {
								rtnval = fgetc(p1);
								printf("%c", rtnval);
							}
							fclose(p1);
							//system("test < lee.txt");
							break;
						}
					}
				}
			}
		}
	}

	close(clnt_sock); /* 연결 종료 */

	close(fd);

}
