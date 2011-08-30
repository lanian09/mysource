#include <stdio.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <errno.h>

int sub(int ac, char **av, int **val);
int main(int ac, char **av)
{
	int *val;
	printf("val1=0x%0x\n", val);
	sub(ac,av, &val);
	printf("val2=0x%0x\n", val);
	printf("main end\n");
	return 0;
}

int sub(int ac, char **av, int **val)
{
	int id, isExist = 0;
//	int *val;
	void *v;

	if( (id = shmget( 0x1234, sizeof(int),0666|IPC_CREAT|IPC_EXCL)) < 0 ){
		if( errno != EEXIST ){
			perror("fail shmget");
			return -1;
		}
		isExist++;
		if( (id = shmget( 0x1234, sizeof(int),0666|IPC_CREAT)) < 0 ){
			perror("fail shmget2");
			return -4;
		}
	}

	printf("ac=%d\n", ac);
	if( ac > 1 ){
		switch(atoi(av[1])){
			case 1:
				if( (v = shmat( id, (char*) 0, 0 )) == (void*) -1 ){;
					perror("fail shmat");
					return -2;
				}
				*val = (int*)v;
				
				if( isExist ){
					printf("old ac=%d\n", **val);
				}else{
					**val = 0;
					printf("init, ac=%d\n", **val);
				}
				if( ac > 2 ){
					**val = atoi(av[2]);
					printf("new ac=%d\n", **val);
				}
				break;
			case 2:
				if( shmctl( id, IPC_RMID, (struct shmid_ds*)0 ) < 0 ){
					perror("fail shmctl");
					return -3;
				}
				break;
			default:
				printf("param type invalid...\n");
				return -4;
				
		}
	}
	printf("end of program\n");
	return 0;
}
