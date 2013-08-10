#include "proc_version.h"


VersionIndexTable vit[] = {
	{"IXPC",   OMP_VER_INX_IXPC  },
	{"FIMD",   OMP_VER_INX_FIMD  },
	{"COND",   OMP_VER_INX_COND  },
	{"STMD",   OMP_VER_INX_STMD  },
	{"MMCD",   OMP_VER_INX_MMCD  },
	{"MCDM",   OMP_VER_INX_MCDM  },
	{"SAMD",   OMP_VER_INX_SAMD  },
	{"NMSIF",  OMP_VER_INX_NMSIF },
	{"UDRCOL", OMP_VER_INX_UDRCOL},
	{"CDELAY", OMP_VER_INX_CDELAY},
	{"HAMON",  OMP_VER_INX_HAMON },
	{"SCEM",   OMP_VER_INX_SCEM  },
	{"CSCM",   OMP_VER_INX_CSCM  },
	{"DIRM",   OMP_VER_INX_DIRM  },
	{NULL, -1}
};

static stVersion *version = NULL;

int init_version_shm()
{
	char	*env;
	char	l_sysconf[64];
	char	temp[10];
	key_t	shmkey;
	int		shmid;
	FILE	*fp;

	if ((env = getenv(IV_HOME)) == NULL) {
		fprintf(stderr, "[init_version_shm] not found %s environment name\n", IV_HOME);
		return -1;
	}

	sprintf(l_sysconf, "%s/%s", env, SYSCONF_FILE);
    
	if (conflib_getNthTokenInFileSection(l_sysconf,"SHARED_MEMORY_KEY",
        	"SHM_PROC_VER", 1, temp) < 0) {
        	fprintf(stderr,"[init_version_shm] conflib_getNthTokenInFileSection fail[SHARED_MEMORY_KEY : SHM_PROC_VER] \n");
        	return -1;
    	}
    
	shmkey = (key_t)strtol(temp,0,0);

    	if ((shmid = (int) shmget (shmkey, sizeof (stVersion), 0666|IPC_CREAT)) < 0) {
        	perror ("shmget SHM_PROC_VER");
        	fprintf(stderr,"[init_version_shm] shmget fail(SHM_PROC_VER); key=0x%x, err=%d[%s]\n",
			shmkey, errno, strerror(errno));
        	return -1;
    	}
    
	if ( (int)(version = (stVersion *)shmat (shmid, (char *) 0,0)) == -1 ) {
        	perror ("shmat SHM_PROC_VER");
        	fprintf(stderr,"[init_version_shm] shmat fail(SHM_PROC_VER); key=0x%x, err=%d[%s]\n",
				shmkey, errno, strerror(errno));
        	return -1;
    	}

        return 0;
}

int set_proc_version(int index, char *prcver)
{

	if(!version)
		if(init_version_shm() < 0)
			return -1;

	memset(&(version->ver[index]), 0x00, VERSION_STR_LEN);
	strncpy(version->ver[index], prcver, VERSION_STR_LEN-1);

	return 0;

}

char * get_proc_version(char *procname)
{
		int         i;
		static char ver[10];

    	memset(ver, 0x00, sizeof(ver));
    	for(i = 0; vit[i].name != NULL; i++){
        	if(!strcasecmp(procname, vit[i].name)){
				strncpy(ver, version->ver[vit[i].index], VERSION_STR_LEN);
				if(strlen(ver) < 1)
					return NULL;
				return ver;
			}
		}
		return NULL;
}

int detatch_version_shm()
{
    	if(shmdt((void *)version) < 0){
        	fprintf(stderr, "Version shared (detach) fail\n");
        	return -1;
    	}

    	return 0;
}

