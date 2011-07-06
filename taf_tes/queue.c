#include <stdio.h>
#include <stdlib.h>
#include <linux/ipc.h>
#include <linux/msg.h>
#include <errno.h>
#include <string.h>

#define DTSIZE 1024 
#define MYKEY 8180

typedef struct  mymsgbuf {
        long    mtype;          /* ╱身卹卹Ao A╱AO */
        int     request;        /* AU取㊣ ０aA∮ 易帚E﹟ */
        double  salary;         /* A㊣０帚AC ▽身０使 */
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

        /* ▽屆AI╞A ▽a足∮AuA╱﹞I ▽╱AＤA卹AC A使▽a - mtypeAC A使▽a AI╞U. */
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

        /* IPC A﹉ ﹉見A∮ 易帕∮y卦AA昌╞U */
        if(( qid = open_queue()) == -1) {
                perror("open_queue");
                exit(1);
        }
          
        /* AOACAC A▼卦足芋Ｙ AU﹞a╱| ╱身卹卹Ao０╳ AuAcCN╞U */
        msg.mtype   = 1;        /* ╱身卹卹Ao A╱AOA足 取c卹o０使取帕 CN╞U */
        msg.request = 1;        /* AU﹞a ０a卹O #1 */
        msg.salary  = 1000.00;  /* AU﹞a ０a卹O #2 (昆見AC ０／﹉﹟ ▽身０使) */

        memset(msg.data, 0xfe,DTSIZE-12);

        for(i=0;i< param;i++){
        /* ∮卦 昆?﹞A足╱昆卦╞U */ 
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
