/**A.1*  File Inclusion ***********************************/

#include "mmdb_fbcdr.h"

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
extern	FBCDR_TABLE	*fbcdr_tbl;

/**D.1*  Definition of Functions  *************************/
int 	Init_shm_fbcdr( int key );
void 	Init_fbcdr();

int 	Init_MMDBFBCDR( int key );
int 	Remove_MMDBFBCDR( int key );

int Init_MMDBFBCDR( int key )
{
    int  err;

    /* �����޸𸮸� �����ϰ� Point �����Ѵ�*/
    if ((err = Init_shm_fbcdr( key )) == SHM_ERROR) {
		;
	} else if (err == SHM_CREATE) { /* shared memory�� ó�� ������ ��� */
		Init_fbcdr();
	} else if (err == SHM_EXIST) {  /* shared memory�� �̹� �ִ� ��� */
		;
	} 

	return err;
}
  	
int Init_shm_fbcdr( int key )
{
    mmdb_shmid = shmget( key, sizeof(FBCDR_TABLE), 0666|IPC_CREAT|IPC_EXCL);
    if (mmdb_shmid >= 0){ /* ���Ӱ� shared memory�� ������ �� ��� */

        fbcdr_tbl = (FBCDR_TABLE *)shmat( mmdb_shmid, 0, 0);
        if( fbcdr_tbl == (FBCDR_TABLE *)-1 ) {
			return SHM_ERROR;
        }       

		return SHM_CREATE;

    } else {
		/* ������ shared memory�� �����Ǿ� �־��� ��� */
		if (errno == EEXIST){

    		mmdb_shmid = shmget( key, sizeof(FBCDR_TABLE), 0666|IPC_CREAT);
			if( mmdb_shmid < 0) {
				return SHM_ERROR;
			}

			/* memory attatch�� �Ҽ� ���� ��� */
        	fbcdr_tbl = (FBCDR_TABLE *)shmat(mmdb_shmid, 0, 0);
        	if( fbcdr_tbl == (FBCDR_TABLE *)-1) {
            	return	SHM_ERROR;
        	}       

        	return	SHM_EXIST;

		} else { /* shared memory�� �����Ҽ� ���� ��� */
			return SHM_ERROR;
		}
	}
}

/****************************************************************
/   ��� :
/       Init_ShmDispatcher()�Լ��� tuple�� linked list�� �����
/       freelist�� list�� ����� ����Ű�� �Ѵ�.
/
****************************************************************/

void Init_fbcdr()
{
    int     i;

	// Free List ����
	for( i=0; i<JMM_FBCDR_RECORD-1; i++ ) {
		fbcdr_tbl->tuple[i].right = i+1;
	}
	fbcdr_tbl->tuple[i].right = -1;

	fbcdr_tbl->free = 0;
	fbcdr_tbl->root = -1;
	fbcdr_tbl->uiCount = 0;
}

int Remove_MMDBFBCDR( int key )
{
	mmdb_shmid = shmget( key, sizeof(FBCDR_TABLE), 0666|IPC_CREAT) ;
	if ((int)mmdb_shmid < 0) {
        return SHM_ERROR;
    }

    /* memory attatch�� �Ҽ� ���� ��� */
    fbcdr_tbl = (FBCDR_TABLE *)shmat(mmdb_shmid, 0, 0);
    if( fbcdr_tbl == (FBCDR_TABLE *)-1 ) {
        return  SHM_ERROR;
    }

	if( shmdt(fbcdr_tbl) < 0 ) {
    	return SHM_ERROR;
    }

    if( shmctl( mmdb_shmid, IPC_RMID, (struct shmid_ds *)0 ) < 0 ) {
    	return SHM_ERROR;
    }

	return 1;
}
