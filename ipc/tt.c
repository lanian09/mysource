#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <string.h>
/*
* if 64-bit machine, use 'long' type
*/
typedef int _PTR_TYPE_;
typedef struct {
	int ia;
	char ca;
	short sa;
	char reserved1;

	unsigned int uia;
	unsigned char uca;
	unsigned short usa;
	char reserved2;
}st_myst,*pst_myst;

int main()
{
	int dKey = 0;
	st_myst stmy;
	pst_myst pstmy;

	printf("starting....\n");
	if((dKey = (int)shmget((key_t)0x1234,sizeof(stmy),0666|IPC_CREAT)) < 0 ){
		printf("error fail shmget[%d:%s]\n",errno, strerror(errno));
		exit(-1);
	}

	if( (_PTR_TYPE_)(pstmy = (pst_myst)shmat(dKey,(char*)0,0)) == -1 ){
		printf("error fail shmat[%d:%s]\n",errno, strerror(errno));
		exit(-2);
	}
	printf("finish....\n");
	
	return 0;
}
