#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <verlib.h>

st_Version		  *g_stVersion;

/** E.2 DEFINITION OF FUNCTIONS ***********************************************/
int set_version(key_t ShmKey, int prc_idx, char *szVersion)
{
	int	 shmid;
	if ((shmid=(int)shmget(ShmKey, sizeof(st_Version), 0666|IPC_CREAT|IPC_EXCL)) < 0)
	{
		if( errno == EEXIST )
		{
			if ((shmid=(int)shmget(ShmKey,sizeof(st_Version),0666|IPC_CREAT)) < 0){
#ifdef DEBUG
				fprintf(stderr,"shmget failed, key=%d, size=%d, error=%d:%s\n",
					ShmKey, (int)sizeof(st_Version), errno, strerror(errno));
#endif
				return E_VER_SHMGET;
			}

			if ((void *)(g_stVersion=(st_Version *)shmat(shmid,(char *) 0,0)) == (void *)-1){
#ifdef DEBUG
				fprintf(stderr,"shmat(1) failed, error=%d:%s\n", errno, strerror(errno));
#endif
				return E_VER_SHMAT;
			}

			// set version
			strncpy(g_stVersion->szVersion[prc_idx], szVersion, DEF_VERSION_SIZE-1);
			g_stVersion->szVersion[prc_idx][DEF_VERSION_SIZE-1] = 0x00;

			// detach?
			return VER_EXIST;
		}

		return E_UNKNOWN;
	}

	if ((void *)(g_stVersion=(st_Version *)shmat(shmid,(char *) 0,0)) == (void *)-1){
#ifdef DEBUG
		fprintf(stderr,"shmat(2) failed, error=%d:%s\n", errno, strerror(errno));
#endif
		return E_VER_SHMGET2;
	}

	memset(g_stVersion,0x00,sizeof(st_Version));

	// set version
	strncpy(g_stVersion->szVersion[prc_idx], szVersion, DEF_VERSION_SIZE-1);
	g_stVersion->szVersion[prc_idx][DEF_VERSION_SIZE-1] = 0;

	// detach?
	return VER_CREATE;
}

void get_version(int prc_idx, char *szVersion)
{
	strcpy(szVersion, g_stVersion->szVersion[prc_idx]);
}

int remove_version(key_t ShmKey)
{
	int dShm = 0;

	dShm = shmget( ShmKey, sizeof( st_Version ), 0666|IPC_CREAT );
	if( dShm < 0 ) {
#ifdef DEBUG
		fprintf(stderr,"shmget failed, key=%d, size=%d, error=%d:%s\n",
#endif
			ShmKey, (int)sizeof(st_Version), errno, strerror(errno));
		return E_VER_SHMGET;
	}

	if((void *)(g_stVersion = (st_Version *)shmat( dShm, (char*)0, 0)) == (void *)-1){
#ifdef DEBUG
		fprintf(stderr,"shmat failed, error=%d:%s\n", errno, strerror(errno));
#endif
		return E_VER_SHMAT;
	}

	if( shmdt(g_stVersion) < 0 ) {
#ifdef DEBUG
		fprintf(stderr,"shmdt failed, error=%d:%s\n", errno, strerror(errno));
#endif
		return E_VER_SHMDT;
	}

	if( shmctl( dShm, IPC_RMID, (struct shmid_ds *)0 ) < 0 ) {
#ifdef DEBUG
		fprintf(stderr,"shmctl failed, error=%d:%s\n", errno, strerror(errno));
#endif
		return E_VER_SHMCTL;
	}

	return VER_REMOVE;//원래는 -1 이었음. -1이 맞나요? 틀리면 수정해 주세요....
}

