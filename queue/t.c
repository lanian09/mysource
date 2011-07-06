#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/ipc.h> 
#include <sys/msg.h>
                
void checkQueInf(int key)
{                   
    int qid;        
    struct msqid_ds qds; 
                    
    if( (qid = msgget(key,0666|IPC_CREAT)) < 0 ){ 
        printf("FAILED IN msgget(%d), error=%d:%s\n", key, errno, strerror(errno));
        return; 
    }       
            
    if( msgctl(qid, IPC_STAT, (struct msqid_ds*)&qds) == -1 ){
        printf("FAILED IN msgctl(%d), error=%d:%s\n", key, errno, strerror(errno));
        return;
    }
    
    /* print queue info */
    printf(" ## MSGQ INFO, KEY=%d ##\n", key);
    printf(" LAST SND PID  = %ld\n", qds.msg_lspid);
    printf(" LAST RCV PID  = %ld\n", qds.msg_lrpid);
    printf(" CBYTES = %d\n", qds.msg_cbytes);
    printf(" QBYTES = %d\n", qds.msg_qbytes);
        
    return;
}   
    
int cvt_hex(char *v)
{
    char *p, c;
    int  key, len, m;

    if( (p = strstr(v,"0x")) != NULL ){
        p += 2;
        while( *p == '0' ){ p++; };
        
        len = strlen(p);
        key = 0;
        m   = 1;
        while(len--){
            c = *(p+len);
            if( c >= '0' && c <= '9' ){ key += ( c -'0' ) * m; }
            else{
                switch(c){
                    case 'a':case'A': key += 10 * m; break;
                    case 'b':case'B': key += 11 * m; break;
                    case 'c':case'C': key += 12 * m; break;
                    case 'd':case'D': key += 13 * m; break;
                    case 'e':case'E': key += 14 * m; break;
                    case 'f':case'F': key += 15 * m; break;
                    default: return -1;
                }
            }
            m *= 16;
        }
        return key;
    }

    return atoi(v);
}

int main(int ac, char **av)
{
    if( ac == 2 ){
        checkQueInf(cvt_hex(av[1]));
        return 0;
    }
    printf("insert queue key\n");
    return 0;
}
