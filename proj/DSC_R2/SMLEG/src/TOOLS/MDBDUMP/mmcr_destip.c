/**********************************************************
                 ABLEX Main-Memory DBMS

   Author   : SangWoo Lee and Jiyoon Chung
   Section  :
   SCCS ID  : %W%
   Date     : %G%
   Revision History : 
        '99.  9. 21     Initial
		'01.  7. 23		Revised for TPK

   Description:
        

   Copyright (c) ABLEX 1999, 2000, and 2001
***********************************************************/

/**A.1*  File Inclusion ***********************************/

#include "mmdb_destip.h"

/**B.1*  Definition of New Constants *********************/

#define MAX_MSGBUF_LEN  1024
#define MAX_ERRBUF_LEN  1024
#define IF_FAIL  -1
#define IF_SUCCESS  0
#define READ_UNIT  1024

#define	SHM_CREATE	1
#define	SHM_EXIST	2
#define	SHM_ERROR	-1

/**B.2*  Definition of New Type  **************************/

/**C.1*  Declaration of Variables  ************************/

int 	mmdb_shmid;
extern	DESTIP_TABLE	*destip_tbl;
extern  char            l_sysconf[256];

/**D.1*  Definition of Functions  *************************/
int Init_shm_destip();
void Init_destip();

int Init_MMDBDESTIP();
int Remove_MMDBDESTIP();

static key_t	ipShmKey;

int Init_MMDBDESTIP()
{
    int  err;

    /* 공유메모리를 생성하고 Point 지정한다*/
    if ((err = Init_shm_destip()) == SHM_ERROR) {
		;
	} else if (err == SHM_CREATE) { /* shared memory가 처음 생성된 경우 */
		Init_destip();
	} else if (err == SHM_EXIST) {  /* shared memory가 이미 있는 경우 */
		;
	} 

	return err;
}
  	
int Init_shm_destip()
{
    int mmdb_shmid;
    char    tmp[50];

#ifdef ALONE_TEST
    ipShmKey = 0x6001;
#else
    if (conflib_getNthTokenInFileSection (l_sysconf, "SHARED_MEMORY_KEY", "S_SSHM_MMDBDESTIP", 1, tmp) < 0)
        return -1;
    ipShmKey = strtol(tmp,0,0);

#endif

    mmdb_shmid = shmget(ipShmKey, sizeof(DESTIP_TABLE), 0666|IPC_CREAT|IPC_EXCL);

    if (mmdb_shmid >= 0){ /* 새롭게 shared memory가 생성이 된 경우 */

        destip_tbl = (DESTIP_TABLE *)shmat( mmdb_shmid, 0, 0);
        if((int)destip_tbl == -1){
			return SHM_ERROR;
        }       

		return SHM_CREATE;

    } else {
		/* 기존에 shared memory가 생성되어 있었던 경우 */
		if (errno == EEXIST){

    		mmdb_shmid = shmget(ipShmKey, sizeof(DESTIP_TABLE), 0666|IPC_CREAT);
			if ((int)mmdb_shmid < 0) {
				return SHM_ERROR;
			}

			/* memory attatch를 할수 없는 경우 */
        	destip_tbl = (DESTIP_TABLE *)shmat(mmdb_shmid, 0, 0);
        	if((int)destip_tbl == -1) {
            	return	SHM_ERROR;
        	}       

        	return	SHM_EXIST;

		} else { /* shared memory를 생성할수 없는 경우 */
			return SHM_ERROR;
		}
	}
}

/****************************************************************
/   기능 :
/       Init_ShmDispatcher()함수는 tuple을 linked list로 만들고
/       freelist가 list의 헤더를 가리키게 한다.
/
****************************************************************/

void Init_destip()
{
    int     i;

	// Free List 생성
	for( i=0; i<JMM_DESTIP_RECORD-1; i++ ) {
		destip_tbl->tuple[i].right = i+1;
	}
	destip_tbl->tuple[i].right = -1;

	destip_tbl->free = 0;
	destip_tbl->root = -1;
	destip_tbl->uiCount = 0;
}

int Remove_MMDBDESTIP()
{
	mmdb_shmid = shmget(ipShmKey, sizeof(DESTIP_TABLE), 0666|IPC_CREAT) ;
	if ((int)mmdb_shmid < 0) {
        return SHM_ERROR;
    }

    /* memory attatch를 할수 없는 경우 */
    destip_tbl = (DESTIP_TABLE *)shmat(mmdb_shmid, 0, 0);
    if((int)destip_tbl == -1) {
        return  SHM_ERROR;
    }

	if( shmdt(destip_tbl) < 0 ) {
    	return SHM_ERROR;
    }

    if( shmctl( mmdb_shmid, IPC_RMID, (struct shmid_ds *)0 ) < 0 ) {
    	return SHM_ERROR;
    }

	return 1;
}
