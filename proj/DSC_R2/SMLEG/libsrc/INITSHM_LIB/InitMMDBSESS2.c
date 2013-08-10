/**A.1*  File Inclusion ***********************************/

#include "mmdb_sess2.h"

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

int 	mmdb_shmid2;
extern	SESSION_TABLE	*session_tbl2;

/**D.1*  Definition of Functions  *************************/
int 	Init_shm_session2( int key );
void 	Init_session2();

int 	Init_PCDRSESS( int key );
int 	Remove_PCDRSESS( int key );

int Init_PCDRSESS( int key )
{
    int  err;

    /* �����޸𸮸� �����ϰ� Point �����Ѵ�*/
    if ((err = Init_shm_session2( key )) == SHM_ERROR) {
		;
	} else if (err == SHM_CREATE) { /* shared memory�� ó�� ������ ��� */
		Init_session2();
	} else if (err == SHM_EXIST) {  /* shared memory�� �̹� �ִ� ��� */
		;
	} 

	return err;
}
  	
int Init_shm_session2( int key )
{
    mmdb_shmid2 = shmget( key, sizeof(SESSION_TABLE), 0666|IPC_CREAT|IPC_EXCL);
    if (mmdb_shmid2 >= 0){ /* ���Ӱ� shared memory�� ������ �� ��� */

        session_tbl2 = (SESSION_TABLE *)shmat( mmdb_shmid2, 0, 0);
        if( session_tbl2 == (SESSION_TABLE *)-1 ) {
			return SHM_ERROR;
        }       

		return SHM_CREATE;

    } else {
		/* ������ shared memory�� �����Ǿ� �־��� ��� */
		if (errno == EEXIST){

    		mmdb_shmid2 = shmget( key, sizeof(SESSION_TABLE), 0666|IPC_CREAT);
			if ((int)mmdb_shmid2 < 0) {
				return SHM_ERROR;
			}

			/* memory attatch�� �Ҽ� ���� ��� */
        	session_tbl2 = (SESSION_TABLE *)shmat(mmdb_shmid2, 0, 0);
        	if( session_tbl2 == (SESSION_TABLE *)-1 ) {
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

void Init_session2()
{
    int     i;

	// Free List ����
	for( i=0; i<JMM_SESSION_RECORD-1; i++ ) {
		session_tbl2->tuple[i].right = i+1;
	}
	session_tbl2->tuple[i].right = -1;

	session_tbl2->free = 0;
	session_tbl2->root = -1;
	session_tbl2->uiCount = 0;
}

int Remove_PCDRSESS( int key )
{
	mmdb_shmid2 = shmget( key, sizeof(SESSION_TABLE), 0666|IPC_CREAT);
	if ((int)mmdb_shmid2 < 0) {
        return SHM_ERROR;
    }

    /* memory attatch�� �Ҽ� ���� ��� */
    session_tbl2 = (SESSION_TABLE *)shmat(mmdb_shmid2, 0, 0);
    if( session_tbl2 == (SESSION_TABLE *)-1) {
        return  SHM_ERROR;
    }

	if( shmdt(session_tbl2) < 0 ) {
    	return SHM_ERROR;
    }

    if( shmctl( mmdb_shmid2, IPC_RMID, (struct shmid_ds *)0 ) < 0 ) {
    	return SHM_ERROR;
    }

	return 1;
}
