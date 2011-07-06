#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

typedef struct _st_msg{
	int key;
	char name[12];
} st_msg, *pst_msg;

int sendq(pst_msg pmsg)
{
	int qid;
	if( (qid = msgget(pmsg->key, 0666|IPC_CREAT)) < 0 ){
		perror("failed msgget");
		return -1;
	}
	if( msgsnd( qid, pmsg, sizeof(st_msg) - sizeof(long), IPC_NOWAIT) < 0 ){
		perror("failed msgsnd");
		return -2;
	}
	printf("send key=%d(0x%0x), name=%s\n", pmsg->key,pmsg->key, pmsg->name);

	return 0;
}

int readq(pst_msg pmsg)
{
	int qid;
	if( (qid = msgget(pmsg->key, 0666|IPC_CREAT)) < 0 ){
		perror("failed msgget");
		return -1;
	}
	if( msgrcv( qid, pmsg, sizeof(st_msg) - sizeof(long), 0,0 ) < 0 ){
		perror("failed msgrcv");
		//return -1;
	}

	printf("get rcved key=%d(0x%0x), name=%s\n", pmsg->key, pmsg->key, pmsg->name);
	return 0;
}

int main(int ac, char **av)
{
	st_msg msg;

	memset(&msg, 0x00, sizeof(st_msg));
	msg.key = 0x1234;
	if( strstr(av[0], "SEND") != NULL ){
		sprintf(msg.name,"Q0x1234");
		sendq(&msg);
	}else{
		readq(&msg);
	}
	return 0;
}
