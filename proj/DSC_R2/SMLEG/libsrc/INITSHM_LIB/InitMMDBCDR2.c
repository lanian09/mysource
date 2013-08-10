/**A.1*  File Inclusion ***********************************/

#include "mmdb_cdr.h"

/**B.1*  Definition of New Constants *********************/

#define MAX_MSGBUF_LEN  1024
#define MAX_ERRBUF_LEN  1024
#define IF_FAIL  -1
#define IF_SUCCESS  0
#define READ_UNIT  1024

#define	SHM_CREATE	1
#define	SHM_EXIST	2
#define	SHM_ERROR 	-1

/**B.2*  Definition of New Type  **************************/

/**C.1*  Declaration of Variables  ************************/

int 	mmdb_shmid;

/**D.1*  Definition of Functions  *************************/
int 	Init_shm_cdr2( int key );
void 	Init_cdr2();

int 	Init_PCDRCDR( int key );
int 	Remove_PCDRCDR( int key );

int Init_PCDRCDR( int key )
{
    int  err;

    /* �����޸𸮸� �����ϰ� Point �����Ѵ�*/
    if ((err = Init_shm_cdr2( key )) == SHM_ERROR) {
		;
	} else if (err == SHM_CREATE) { /* shared memory�� ó�� ������ ��� */
		Init_cdr2();
	
	} else if (err == SHM_EXIST) {  /* shared memory�� �̹� �ִ� ��� */
		;
	} 

	return err;
}
  	
int Init_shm_cdr2( int key )
{
    mmdb_shmid = shmget( key, sizeof(CDR_TABLE), 0666|IPC_CREAT|IPC_EXCL) ;    
    if (mmdb_shmid >= 0){ /* ���Ӱ� shared memory�� ������ �� ��� */
        cdr2_tbl = (CDR_TABLE *)shmat( mmdb_shmid, 0, 0);

        if( cdr2_tbl == (CDR_TABLE *)-1 ) {
			return SHM_ERROR;
        }       
		return SHM_CREATE;

    } else {
		/* ������ shared memory�� �����Ǿ� �־��� ��� */
		if (errno == EEXIST){
    		mmdb_shmid = shmget( key, 
				sizeof(CDR_TABLE), 0666|IPC_CREAT) ;    

			if ((int)mmdb_shmid < 0) {
				return SHM_ERROR;
			}

			/* memory attatch�� �Ҽ� ���� ��� */
        	cdr2_tbl = (CDR_TABLE *)shmat(mmdb_shmid, 0, 0);

        	if( cdr2_tbl == (CDR_TABLE *)-1 ) {
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

void Init_cdr2()
{
    int     i;

	// Free List ����
	for( i=0; i < JMM_CDR_RECORD - 1; i++ ) {
		cdr2_tbl->tuple[i].right = i+1;
	}
	cdr2_tbl->tuple[i].right = -1;

	cdr2_tbl->free = 0;
	cdr2_tbl->root = -1;
	cdr2_tbl->uiCount = 0;
}

int Remove_PCDRCDR( int key )
{
	mmdb_shmid = shmget( key, sizeof(CDR_TABLE), 0666|IPC_CREAT) ;

	if ((int)mmdb_shmid < 0) {
        return SHM_ERROR;
    }

    /* memory attatch�� �Ҽ� ���� ��� */
    cdr2_tbl = (CDR_TABLE *)shmat(mmdb_shmid, 0, 0);
    if( cdr2_tbl == (CDR_TABLE *)-1 ) {
        return  SHM_ERROR;
    }

	if( shmdt(cdr2_tbl) < 0 ) {
    	return SHM_ERROR;
    }

    if( shmctl( mmdb_shmid, IPC_RMID, (struct shmid_ds *)0 ) < 0 ) {
    	return SHM_ERROR;
    }

	return 1;
}
