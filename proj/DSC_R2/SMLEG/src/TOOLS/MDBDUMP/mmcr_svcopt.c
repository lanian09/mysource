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

#include "mmdb_svcopt.h"

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

extern	SVCOPT_TABLE	*svcopt_tbl;
extern  char            l_sysconf[256];

/**D.1*  Definition of Functions  *************************/
int Init_shm_svcopt();
void Init_svcopt();

int Init_MMDBSVCOPT();
int Remove_MMDBSVCOPT();

static key_t	ipShmKey;

int Init_MMDBSVCOPT()
{
    int  err;

    /* �����޸𸮸� �����ϰ� Point �����Ѵ�*/
    if ((err = Init_shm_svcopt()) == SHM_ERROR) {
		;
	} else if (err == SHM_CREATE) { /* shared memory�� ó�� ������ ��� */
		Init_svcopt();
	} else if (err == SHM_EXIST) {  /* shared memory�� �̹� �ִ� ��� */
		;
	} 

	return err;
}
  	
int Init_shm_svcopt()
{
#if 0
	int 	mmdb_shmid;
    char    tmp[50];

    if (conflib_getNthTokenInFileSection (l_sysconf, "SHARED_MEMORY_KEY", "SHM_SVC_OPT", 1, tmp) < 0)
        return -1;
    ipShmKey = strtol(tmp,0,0);

    mmdb_shmid = shmget(ipShmKey, sizeof(SVCOPT_TABLE), 0666|IPC_CREAT|IPC_EXCL);

    if (mmdb_shmid >= 0){ /* ���Ӱ� shared memory�� ������ �� ��� */

        svcopt_tbl = (SVCOPT_TABLE *)shmat( mmdb_shmid, 0, 0);
        if((int)svcopt_tbl == -1){
			return SHM_ERROR;
        }       

		return SHM_CREATE;

    } else {
		/* ������ shared memory�� �����Ǿ� �־��� ��� */
		if (errno == EEXIST){

    		mmdb_shmid = shmget(ipShmKey, sizeof(SVCOPT_TABLE), 0666|IPC_CREAT);
			if ((int)mmdb_shmid < 0) {
				return SHM_ERROR;
			}

			/* memory attatch�� �Ҽ� ���� ��� */
        	svcopt_tbl = (SVCOPT_TABLE *)shmat(mmdb_shmid, 0, 0);
        	if((int)svcopt_tbl == -1) {
            	return	SHM_ERROR;
        	}       

        	return	SHM_EXIST;

		} else { /* shared memory�� �����Ҽ� ���� ��� */
			return SHM_ERROR;
		}
	}
#endif
}

/****************************************************************
/   ��� :
/       Init_ShmDispatcher()�Լ��� tuple�� linked list�� �����
/       freelist�� list�� ����� ����Ű�� �Ѵ�.
/
****************************************************************/

void Init_svcopt()
{
    int     i;

	// Free List ����
	for( i=0; i<JMM_SVCOPT_RECORD-1; i++ ) {
		svcopt_tbl->tuple[i].right = i+1;
	}
	svcopt_tbl->tuple[i].right = -1;

	svcopt_tbl->free = 0;
	svcopt_tbl->root = -1;
	svcopt_tbl->uiCount = 0;
}

int Remove_MMDBSVCOPT()
{
	int 	mmdb_shmid;

	mmdb_shmid = shmget(ipShmKey, sizeof(SVCOPT_TABLE), 0666|IPC_CREAT) ;
	if ((int)mmdb_shmid < 0) {
        return SHM_ERROR;
    }

    /* memory attatch�� �Ҽ� ���� ��� */
    svcopt_tbl = (SVCOPT_TABLE *)shmat(mmdb_shmid, 0, 0);
    if((int)svcopt_tbl == -1) {
        return  SHM_ERROR;
    }

	if( shmdt(svcopt_tbl) < 0 ) {
    	return SHM_ERROR;
    }

    if( shmctl( mmdb_shmid, IPC_RMID, (struct shmid_ds *)0 ) < 0 ) {
    	return SHM_ERROR;
    }

	return 1;
}
