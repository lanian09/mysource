
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <errno.h>

extern int errno;

#define	SHMPERM	0666

int Init_shm( key_t shm_key, int size, void **memptr );
int init_shm( key_t shm_key, int size, char *memptr );
int remove_shm( int shm_key, int size, char *memptr );
