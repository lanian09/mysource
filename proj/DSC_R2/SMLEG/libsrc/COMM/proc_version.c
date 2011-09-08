#include "proc_version.h"

/*
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
	{NULL, -1}
};
*/

VersionIndexTable vit[IPAF_SW_COM] = {
	{"IXPC",		SEQ_PROC_IXPC},
	{"SAMD",		SEQ_PROC_SAMD},
	{"MMCR",		SEQ_PROC_MMCR},
//	{"STMM",		SEQ_PROC_STMM},
	{"CAPD",		SEQ_PROC_CAPD},
	{"PANA",		SEQ_PROC_ANA},
//	{"CDR",			SEQ_PROC_CDR},
//	{"TRCDR",		SEQ_PROC_SESSANA},
//	{"WAP1ANA",		SEQ_PROC_WAP1ANA},
//	{"UAWAPANA",	SEQ_PROC_UAWAPANA},
//	{"WAP2ANA",		SEQ_PROC_MESVC},
//	{"HTTPANA",		SEQ_PROC_KUNSVC},
//	{"VODSANA",		SEQ_PROC_VODSANA},
//	{"WIPINWANA",	SEQ_PROC_WIPINWANA},
//	{"JAVANWANA",	SEQ_PROC_KVMANA},
//	{"PCDR",		SEQ_PROC_PCDR},
//	{"CDR2",		SEQ_PROC_CDR2},
//	{"VTANA",		SEQ_PROC_VTANA},
//	{"UDRGEN",		SEQ_PROC_UDRGEN},
//	{"AAAIF",		SEQ_PROC_AAAIF},
//	{"SDMD",		SEQ_PROC_SDMD},
//	{"LOGM",		SEQ_PROC_LOGM},
	{"RANA",		SEQ_PROC_RANA},
//	{"FBANA",		SEQ_PROC_FBANA},
	{"MEMD",		SEQ_PROC_MEM},
	{NULL,			-1}
};


//static stVersion *version = NULL;
extern st_VERSION	*version;

int init_version_shm(void)
{
	int		shmid;
	char	*env, l_sysconf[64], temp[10];
	key_t	shmkey;

	if( (env = getenv(IV_HOME)) == NULL)
	{
		fprintf(stderr, "[%s] not found %s environment name\n", __FUNCTION__, IV_HOME);
		return -1;
	}

	sprintf(l_sysconf, "%s/%s", env, SYSCONF_FILE);
	if(conflib_getNthTokenInFileSection(l_sysconf,"SHARED_MEMORY_KEY", "SHM_PROC_VER", 1, temp) < 0)
	{
		fprintf(stderr,"[init_version_shm] conflib_getNthTokenInFileSection fail[SHARED_MEMORY_KEY : SHM_PROC_VER]\n");
		return -1;
	}

	shmkey = (key_t)strtol(temp, 0, 0);

	if( (shmid = (int)shmget(shmkey, sizeof(st_VERSION), 0666 | IPC_CREAT)) < 0)
	{
		perror("shmget SHM_PROC_VER");
		fprintf(stderr, "[%s] shmget fail(SHM_PROC_VER); key=0x%x, err=%d[%s]\n", __FUNCTION__, shmkey, errno, strerror(errno));
		return -1;
	}

	if( (int)(version = (st_VERSION*)shmat (shmid, (char*)0, 0)) == -1)
	{
		perror("shmat SHM_PROC_VER");
		fprintf(stderr,"[%s] shmat fail(SHM_PROC_VER); key=0x%x, err=%d[%s]\n", __FUNCTION__, shmkey, errno, strerror(errno));
		return -1;
	}

	return 0;
}

int set_proc_version(int index, char *prcver)
{
	if(!version)
		if(init_version_shm() < 0)
			return -1;

	memset(&(version->ver[index]), 0x00, DEF_VERSION_SIZE);
	strncpy(version->ver[index], prcver, DEF_VERSION_SIZE-1);

	return 0;
}

char *get_proc_version(char *procname)
{
	int			i;
	static char	ver[10];

	memset(ver, 0x00, sizeof(ver));
	for(i = 0; vit[i].name != NULL; i++)
	{
		if(!strcasecmp(procname, vit[i].name))
		{
			strncpy(ver, version->ver[vit[i].index], DEF_VERSION_SIZE);
			if(strlen(ver) < 1)
				return NULL;
			return ver;
		}
	}

	return NULL;
}

int detatch_version_shm(void)
{
	if(shmdt((void*)version) < 0)
	{
		fprintf(stderr, "Version shared (detach) fail\n");
		return -1;
	}

	return 0;
}
