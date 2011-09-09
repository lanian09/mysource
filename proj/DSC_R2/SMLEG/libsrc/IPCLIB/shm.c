#include "init_shm.h"

int Init_shm( key_t shm_key, int size, void **memptr )
{
	int shm_id, ret_val;

	if( (shm_id = shmget(shm_key, size, SHMPERM | IPC_CREAT | IPC_EXCL)) < 0 )
	{
		if( errno == EEXIST )
		{
			shm_id = shmget( shm_key, size, SHMPERM | IPC_CREAT );
			if( shm_id < 0 )
				return -errno;

			*memptr = shmat( shm_id, 0, 0 );
			ret_val = SHM_EXIST;    /* exist already */
		}
		else
			return -errno;
	}
	else
	{
		*memptr = shmat( shm_id, 0, 0 );
		ret_val = SHM_CREATE;   /* created */
	}

	if( *memptr == (void*)-1 )
		return -1;

	return ret_val;
}

int init_shm( key_t shm_key, int size, char *memptr )
{
	int shm_id;

	if( (shm_id = shmget(shm_key, size, SHMPERM | IPC_CREAT | IPC_EXCL)) < 0 )
	{
		if( errno == EEXIST )
		{
			shm_id = shmget( shm_key, size, SHMPERM | IPC_CREAT );
			if( shm_id < 0 )
				return -1;

			memptr = (char *)shmat( shm_id, (char *)0, 0 );
		}
		else
			return -1;
	}
	else
		memptr = shmat( shm_id, (char *)0, 0 );

	if( memptr == (char *)-1 )
		return -1;

	return 0;
}

int remove_shm( int shm_key, int size, char *memptr )
{
	int shm_id;

	if( (shm_id = shmget(shm_key, size, SHMPERM | IPC_CREAT)) < 0 )
		return -1;

	memptr = shmat( shm_id, (char *)0, 0 );
	if( memptr == (char *)-1 )
		return -1;

	if( shmdt(memptr) < 0 )
		return -1;

	if( shmctl(shm_id, IPC_RMID, (struct shmid_ds *)0) < 0 )
		return -1;

	return 0;
}
