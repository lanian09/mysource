/**A.1*  File Inclusion *******************************************************/
#include <sys/shm.h>	/* shmget */
#include <sys/msg.h>	/* msgget */
#include <errno.h>		/* errno */
#include <string.h>		/* memset */

#include "ipclib.h"

/**B.1*	DEFINITION OF NEW CONSTANTS *******************************************/
/**B.2*	DEFINITION OF NEW TYPE ************************************************/
/**C.1*	DECLARATION OF VARIABLES **********************************************/
/**C.2*	DECLARATION OF FUNCTIONS **********************************************/
int shm_init( int shm_key, long int size, void **shmptr )
{
    int shmid;

    if( (shmid = (int)shmget(shm_key, size, SHMPERM|IPC_CREAT|IPC_EXCL)) < 0 ){

        if( errno == EEXIST ){

            if( (shmid = (int)shmget(shm_key, size, SHMPERM|IPC_CREAT)) < 0 ){
                return E_SHM_CREATE;
            }

            if( (*shmptr = shmat(shmid, 0, 0)) == (void*) -1 ){
                return E_SHM_ATTACH;
            }
            return SHM_EXIST;
        }
        return E_SHM_GENERIC;
    }

    if( (*shmptr = shmat(shmid, 0, 0)) == (void*) -1 ){
        return E_SHM_ATTACH2;
    }

    memset(*shmptr, 0x00, size);
    return SHM_CREATE;
}

int shm_remove(int shm_key)
{
    int shmid;

    if( (shmid = (int)shmget(shm_key, 0, SHMPERM)) < 0 ){
        if( shmid != ENOENT ){
            return E_SHM_GENERIC;
        }
        return E_SHM_NOENT;
    }

    if( shmctl( shmid, IPC_RMID, (struct shmid_ds *)0 ) < 0 ){
        return E_SHM_REMOVE;
    }

    return SHM_REMOVE;
}
