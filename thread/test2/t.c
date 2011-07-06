#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <signal.h>

int main(int argc, char **argv){
	int  rtnval, diff;
	char lee[1025], tmp[32];
	FILE   *p1;
	struct timeval t = {0,0};

	while(1) {
		if(1){
			signal(SIGCHLD, SIG_IGN);
			rtnval = fork();
			if( rtnval == 0 ) {
				gettimeofday(&t,NULL);
				printf("\n\n[starting time check=%ld.%06ld]\n", t.tv_sec, t.tv_usec);

				diff = t.tv_usec;
				p1 = popen("cat lee.txt", "r");

				gettimeofday(&t,NULL);
				printf("[finished time check=%ld.%06ld]\n", t.tv_sec, t.tv_usec);
				printf("[different time = %d ( %d - %d )\n\n\n", (t.tv_usec - diff), t.tv_usec, diff );
				while(1) {
					rtnval = fgetc(p1);
					printf("%c", rtnval);
				}
				printf("read end of file...\n");
				pclose(p1);
				exit(0);
			} else {
				printf("child process fork ok...\n");
			}
		}
		sleep(1);
	}


}
