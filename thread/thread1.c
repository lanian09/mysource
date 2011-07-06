#include <stdio.h>
#include <pthread.h>
#include <ctype.h>
#include <string.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <netinet/in.h> 
#include <sys/socket.h>
 
pthread_t threads[5]; 
int done[5];

int  test_popen(int pid)
{
	struct timeval t = {0,0};
	int    diff, rtnval;
	FILE   *p1;

	gettimeofday(&t,NULL);
	
	printf("\n\n>>> running process[ %d ]\n[starting time check=%ld.%06ld]\n", pid, t.tv_sec, t.tv_usec);

	diff = t.tv_usec;
	p1 = popen("cat test2/lee.txt", "r");

	gettimeofday(&t,NULL);
	printf("[finished time check=%ld.%06ld]\n", t.tv_sec, t.tv_usec);
	printf("[different time = %d ( %d - %d )\n\n\n", (t.tv_usec - diff), t.tv_usec, diff );
	rtnval = 1;
	while(rtnval != EOF) {
		rtnval = fgetc(p1);
		//printf("%c", rtnval);
	}
	printf("read end of file...\n");

	pclose(p1);

	sleep(1);
	return pid;
}

void *thread_main(void *arg) 
{ 
	int       counter=0;

    while(1)
    {
        counter++;
		test_popen(counter);
    }
 
    //pthread_exit((void *) 0); 
	return;
} 

void thread_init()
{
	int i=0;
    int rc; 
    int status; 

	pthread_attr_t  thrAttr;

    pthread_attr_init(&thrAttr);
    pthread_attr_setscope(&thrAttr, PTHREAD_SCOPE_SYSTEM);
    pthread_attr_setdetachstate(&thrAttr, PTHREAD_CREATE_DETACHED);

	i = 0;
	for( i = 0; i<5; i++ ){
		pthread_create(&threads[i], &thrAttr, &thread_main, (void*)i); 
		printf("main-1st loop :: threads[%d]=%d\n", i, threads[i]); 
	}

	for( i = 4; i >= 0; i-- ){
		rc = pthread_join(threads[i], (void **)&status); 
	}

    pthread_attr_destroy(&thrAttr);
}

int main(void) 
{ 
    int i; 
	pid_t ppid;
     
    printf("pid=%d\n", ( ppid = getpid() ) ); 
	sleep(3);
     
	thread_init();

	while(1){
		sleep(3);
	}
	printf("main process terminated.\n");
    return 0; 
} 
