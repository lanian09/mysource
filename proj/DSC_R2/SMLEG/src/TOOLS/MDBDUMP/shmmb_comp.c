#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "mmdb_destport.h"


void find_diff_bits(DESTPORT_TABLE *a, DESTPORT_TABLE *b)
{
    int             i = 0;
    unsigned char *pa = (unsigned char *)a;
    unsigned char *pb = (unsigned char *)b;

    while(i < sizeof(DESTPORT_TABLE)){
        if(*pa != *pb)
            printf("W[%d], P[%02x], C[%02x]\n", i, *pa, *pb);
        i++;
        pa++;
        pb++;
    }

}


int main(int argc, char *argv[])
{
    int				key, shmId, count;
    DESTPORT_TABLE  *destport_tbl, destport_tbl_bk;

    key = strtol("0x2844",0,0);

    if ((shmId = (int)shmget (key, sizeof(DESTPORT_TABLE), 0644)) < 0) {
        if (errno != ENOENT) {
            fprintf (stderr,"[mmcr_init] shmget fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
            return 1;
        }
    }

    if ((destport_tbl = (DESTPORT_TABLE*) shmat (shmId,0,0)) == (DESTPORT_TABLE*)-1) {
        fprintf (stderr,"[mmdr_init] shmat fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
        return 1;
    }

    memcpy(&destport_tbl_bk, destport_tbl, sizeof(DESTPORT_TABLE));

    while(1){
        if(memcmp(&destport_tbl_bk, destport_tbl, sizeof(DESTPORT_TABLE))){
            find_diff_bits(&destport_tbl_bk, destport_tbl);
            memcpy(&destport_tbl_bk, destport_tbl, sizeof(DESTPORT_TABLE));
        }
        usleep(10);
    }

    return 0;

}
