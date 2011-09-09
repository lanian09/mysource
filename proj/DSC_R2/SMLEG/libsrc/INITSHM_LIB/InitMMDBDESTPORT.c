/**A.1*  File Inclusion ***********************************/

#include "mmdb_destport.h"

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
extern	DESTPORT_TABLE	*destport_tbl;

/**D.1*  Definition of Functions  *************************/
int 	Init_shm_destport( int key );
void 	Init_destport();

int 	Init_MMDBDESTPORT( int key );
int 	Remove_MMDBDESTPORT( int key );

int Init_MMDBDESTPORT( int key )
{
    int  err;

    /* �����޸𸮸� �����ϰ� Point �����Ѵ�*/
    if ((err = Init_shm_destport( key )) == SHM_ERROR) {
		;
	} else if (err == SHM_CREATE) { /* shared memory�� ó�� ������ ��� */
		Init_destport();
	} else if (err == SHM_EXIST) {  /* shared memory�� �̹� �ִ� ��� */
		;
	} 

	return err;
}
  	
int Init_shm_destport( int key )
{
    mmdb_shmid = shmget( key, sizeof(DESTPORT_TABLE), 0666|IPC_CREAT|IPC_EXCL);
    if (mmdb_shmid >= 0){ /* ���Ӱ� shared memory�� ������ �� ��� */

        destport_tbl = (DESTPORT_TABLE *)shmat( mmdb_shmid, 0, 0);
        if( destport_tbl == (DESTPORT_TABLE *)-1 ) {
			return SHM_ERROR;
        }       

		return SHM_CREATE;

    } else {
		/* ������ shared memory�� �����Ǿ� �־��� ��� */
		if (errno == EEXIST){

    		mmdb_shmid = shmget( key, sizeof(DESTPORT_TABLE), 0666|IPC_CREAT);
			if( mmdb_shmid < 0) {
				return SHM_ERROR;
			}

			/* memory attatch�� �Ҽ� ���� ��� */
        	destport_tbl = (DESTPORT_TABLE *)shmat(mmdb_shmid, 0, 0);
        	if( destport_tbl == (DESTPORT_TABLE *)-1 ) {
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

void Init_destport()
{
    int     i;

	// Free List ����
	for( i=0; i<JMM_DESTPORT_RECORD-1; i++ ) {
		destport_tbl->tuple[i].right = i+1;
	}
	destport_tbl->tuple[i].right = -1;

	destport_tbl->free = 0;
	destport_tbl->root = -1;
	destport_tbl->uiCount = 0;
}

int Remove_MMDBDESTPORT( int key )
{
	mmdb_shmid = shmget( key, sizeof(DESTPORT_TABLE), 0666|IPC_CREAT);
	if ((int)mmdb_shmid < 0) {
        return SHM_ERROR;
    }

    /* memory attatch�� �Ҽ� ���� ��� */
    destport_tbl = (DESTPORT_TABLE *)shmat(mmdb_shmid, 0, 0);
    if( destport_tbl == (DESTPORT_TABLE *)-1 ) {
        return  SHM_ERROR;
    }

	if( shmdt((void*)destport_tbl) < 0 ) {
    	return SHM_ERROR;
    }

    if( shmctl( mmdb_shmid, IPC_RMID, (struct shmid_ds *)0 ) < 0 ) {
    	return SHM_ERROR;
    }

	return 1;
}
