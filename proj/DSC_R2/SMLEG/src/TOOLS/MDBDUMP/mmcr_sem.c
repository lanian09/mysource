#include "ipaf_sem.h"

int Init_sem( key_t semkey )
{
	int ret = 0, semid = -1;
	
	if( (semid = semget(semkey, 1, SEMPERM | IPC_CREAT | IPC_EXCL)) == -1 )
	{
		if( errno == EEXIST )
			semid = semget( semkey, 1, 0 );
	}
	else	/* if created... */
	{
//		ret = semctl( semid, 0, SETVAL, 1 );
        union semun {
            int val;
            struct semid_ds *buf;
            unsigned short *arry;
        } arg;

        arg.val = 1;
        ret = semctl( semid, 0, SETVAL, arg );
	}

	if( semid == -1 || ret == -1 )
		return -1;
	else
		return semid;
	
}

int Remove_sem( key_t semkey )
{
	int ret = 0, semid = -1;

	if( (semid = semget(semkey, 1, 0)) == -1 )
	{
		return 1;	/* error */
	}

	ret = semctl( semid, 1, IPC_RMID, 1 );
	if( ret < 0 )
	{
		if( errno == EIDRM )
			return 0;
		return -1;
	}

	return 0;
}

int p( int semid )
{
	struct sembuf p_buf;

	p_buf.sem_num = 0;
	p_buf.sem_op  = -1;
	p_buf.sem_flg = SEM_UNDO;

	if( semop(semid, &p_buf, 1) == -1 )
		return -1;
	else
		return 0;
}

int v( int semid )
{
	struct sembuf v_buf;

	v_buf.sem_num = 0;
	v_buf.sem_op  = 1;
	v_buf.sem_flg = SEM_UNDO;

	if( semop(semid, &v_buf, 1) == -1 )
		return -1;
	else
		return 0;
	
}
