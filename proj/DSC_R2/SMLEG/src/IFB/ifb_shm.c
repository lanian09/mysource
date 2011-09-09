#include "ipaf_define.h"
#include "ipaf_shm.h"
#include "ifb_proto.h"
#include "sfm_msgtypes.h"

#define SHM_LOC_SADB    0x11112

extern st_VERSION *version;

int init_ver_shm()
{
    int             shmid;

    if ((shmid=(int)shmget(S_SSHM_VERSION,sizeof(st_VERSION),0666|IPC_CREAT)) < 0)
    {
        fprintf(stderr, "Version shared (get) fail\n");
        version = NULL;
        return -1;
    }

    if ( (version=(st_VERSION *)shmat(shmid,(char *) 0,0)) == (st_VERSION *)-1){
        fprintf(stderr, "Version shared (at) fail\n");
        version = NULL;
        return -1;
    }
    return 0;
}


int init_sadb_shm (void)
{
	int 	shmId;

	if ((shmId = (int)shmget(SHM_LOC_SADB, sizeof(SFM_SysCommMsgType), 0666 | IPC_CREAT)) < 0) {
		if(errno != ENOENT){
			fprintf(stderr, "loc_sadb shared (get) fail %d(%s)\n", errno, strerror(errno));
			loc_sadb = NULL;
			return -1;
		}
	}

	if ((loc_sadb = (SFM_SysCommMsgType*)shmat(shmId, 0, 0)) == (SFM_SysCommMsgType*)-1) {
        fprintf(stderr, "loc_sadb shared (at) fail\n");
		return -1;
	}
	return 0;
}


int detatch_ver_shm()
{
    if(shmdt((void *)version) < 0){
        fprintf(stderr, "Version shared (detach) fail\n");
        return -1;
    }

    return 0;
}

#if 0	/*	HJPARK_20090422: This same function exist in "libsrc/INITSHM_LIB/InitShm.c"	*/
void get_version(int prc_idx, char *ver)
{
    if(!version)
        *ver = 0x00;
    else
        strcpy(ver,version->ver[prc_idx]);
}
#endif
