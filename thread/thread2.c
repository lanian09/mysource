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
	while(1) {
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
    int    i, rtnval, diff;
    double result=0.0; 
    char   lee[1025], tmp[32];
    FILE   *p1;
    struct timeval t = {0,0};

	while(1){
		
		gettimeofday(&t,NULL);
		
		printf("\n\n>>> running process[ %d ]\n[starting time check=%ld.%06ld]\n", (int)arg, t.tv_sec, t.tv_usec);

		diff = t.tv_usec;
		p1 = popen("cat test2/lee.txt", "r");

		gettimeofday(&t,NULL);
		printf("[finished time check=%ld.%06ld]\n", t.tv_sec, t.tv_usec);
		printf("[different time = %d ( %d - %d )\n\n\n", (t.tv_usec - diff), t.tv_usec, diff );
		while(1) {
			rtnval = fgetc(p1);
			//printf("%c", rtnval);
		}
		printf("read end of file...\n");

		pclose(p1);

		sleep(1);
    }

	//pthread_exit((void*)0);
	return;
}

#define CHECK_COUNTER 100
void *getTHRD1(void *arg)
{
	int       counter=0;
	char      cmd[100];

	fprintf(stdout, "thread ...\n");
    while(1)
    {
        counter++;

        sleep(3);
        if( counter<=CHECK_COUNTER )
        {
/***
			fprintf(stdout,"PERIDIC CHECK FLOW. %d\n", counter);
			sprintf(cmd, "/bin/ls -al > /tmp/tmpp");
			system(cmd);
**/
			test_popen((int)arg);
        }

    }
}

void thread1()
{
	pthread_attr_t  thrAttr;
    pthread_t       thrId;
    int  status;

	fprintf(stdout, "start thread1\n");
    pthread_attr_init(&thrAttr);
    pthread_attr_setscope(&thrAttr, PTHREAD_SCOPE_SYSTEM);
    pthread_attr_setdetachstate(&thrAttr, PTHREAD_CREATE_DETACHED);
    if(pthread_create(&thrId, &thrAttr, getTHRD1, NULL)) {
        fprintf (stdout, "[%s] pthread_create fail\n",__FUNCTION__);
    }   
    pthread_join(thrId, (void *) &status);
    pthread_attr_destroy(&thrAttr);
    return;
}

void *thread_main2(void *arg) 
{ 
    int i; 
    double result=0.0; 
	int       counter=0;
    const int SLEEP_TIME=3;
	char      cmd[100];
 
    printf("in therad[%d]_pid=%ld\n", (int)arg, (long)getpid()); 

    while(1)
    {
        counter++;

        //sleep(1);
		test_popen(counter);
/**
			fprintf(stdout,"PERIDIC CHECK FLOW. %d\n", counter);
			sprintf(cmd, "/bin/ls -al > /tmp/tmpp");
			system(cmd);
**/

    }
 
    pthread_exit((void *) 0); 
} 

int main(void) 
{ 
    int i; 
    int rc; 
    int status; 
	pid_t ppid;
	pthread_attr_t  thrAttr;
     
    printf("pid=%d\n", ( ppid = getpid() ) ); 
	sleep(3);
     
    pthread_attr_init(&thrAttr);
    pthread_attr_setscope(&thrAttr, PTHREAD_SCOPE_SYSTEM);
    pthread_attr_setdetachstate(&thrAttr, PTHREAD_CREATE_DETACHED);

/**
	i = 0;
	pthread_create(&threads[i], &thrAttr, &thread_main2, i); 
	printf("main-1st loop :: threads[%d]=%d\n", i, threads[i]); 
**/
    for (i = 0; i < 5; i++) 
    {     
        done[i] = 0; 
		pthread_create(&threads[i], &thrAttr, &thread_main2, (void*)i); 
		printf("main-1st loop :: threads[%d]=%d\n", i, threads[i]); 
    } 
 
/***
	rc = pthread_join(threads[i], (void **)&status); 
***/
    for (i = 4; i >= 0; i--) 
    { 
        //done[i] = 1; 
		rc = pthread_join(threads[i], (void **)&status); 
        if (rc == 0) 
        { 
            printf("Completed join with thread %d status= %d\n",i, status); 
        } 
        else 
        { 
            printf("ERROR; return code from pthread_join() is %d, thread %d\n", rc, i); 
            return -1; 
        } 
    } 
 
    pthread_attr_destroy(&thrAttr);
	printf("main process terminated.\n");
    return 0; 
} 
