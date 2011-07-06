#include <stdio.h>
#include <stdlib.h>
#include <linux/ipc.h>
#include <linux/msg.h>
#include <errno.h>
#include <string.h>

#define DTSIZE 1024 
#define MYKEY 8180

typedef struct  mymsgbuf {
        long    mtype;          /* ��������Ao A��AO */
        int     request;        /* AU���� ��aA�� ����E�� */
        double  salary;         /* A������AC �������� */
        char    data[DTSIZE-12];
} _st_mymsg;

int open_queue()
{
        int     qid;

        if((qid = msgget( MYKEY, IPC_CREAT | 0660 )) == -1)
        {
                return(-1);
        }

        return(qid);
}

int send_message( int qid, _st_mymsg *qbuf )
{
        int     result, length;

        /* ����AI��A ��a����AuA����I ����A��A��AC A�ϡ�a - mtypeAC A�ϡ�a AI��U. */
        length = sizeof(struct mymsgbuf) - sizeof(long);

        if((result = msgsnd( qid, qbuf, length, 0)) == -1)
        {
                return(-1);
        }

        return(result);
}

main(int ac, char **av)
{
        int     qid;
        int param;
        int i;
        key_t   msgkey;
        _st_mymsg msg;
        struct msqid_ds qds;
        unsigned int ccnt, mcnt;

        if( ac < 2 ){
                printf("insert param\n");
                exit(1);
        }
        param = atoi(av[1]);
        printf("inserted param= %d\n",param);

        /* IPC A�� �ƨ�A�� ������y��AA����U */
        if(( qid = open_queue()) == -1) {
                perror("open_queue");
                exit(1);
        }
          
        /* AOACAC A���������� AU��a��| ��������Ao���� AuAcCN��U */
        msg.mtype   = 1;        /* ��������Ao A��AOA�� ��c��o���Ϩ��� CN��U */
        msg.request = 1;        /* AU��a ��a��O #1 */
        msg.salary  = 1000.00;  /* AU��a ��a��O #2 (����AC �����ơ� ��������) */

        memset(msg.data, 0xfe,DTSIZE-12);

        for(i=0;i< param;i++){
        /* ��� ��?��A����������U */ 
                if((send_message( qid, &msg )) == -1) {
                        perror("send_message");
                        exit(1);
                }
                if( i%1000 == 0){
                        printf("count = %d\n", i);
                        usleep( 100 );
                } 
        }
}
