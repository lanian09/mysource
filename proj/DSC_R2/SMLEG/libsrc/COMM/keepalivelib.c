#include <sys/shm.h>
#include <sys/ipc.h>

#include "keepalivelib.h"
#include "conflib.h"

/*	Declare global variables	*/
int				keepaliveIndex;
T_keepalive		*keepalive;

/*	Decleare functions			*/
int keepalivelib_init(char *processName);
void keepalivelib_increase(void);


int keepalivelib_init(char *processName)
{
	int		shmid;
	char	*env, l_sysconf[64], ckeepalive[10], getBuf[256], token[CONFLIB_MAX_TOKEN_LEN];
	key_t	ikeepalive;
	FILE	*fp;

	keepaliveIndex = 0;

	if( (env = getenv(IV_HOME)) == NULL)
	{
		fprintf(stderr, "[%s] not found %s environment name\n", __FUNCTION__, IV_HOME);
		return -1;
	}

	sprintf(l_sysconf, "%s/%s", env, SYSCONF_FILE);
	if(conflib_getNthTokenInFileSection(l_sysconf,"SHARED_MEMORY_KEY", "SHM_KEEPALIVE", 1, ckeepalive) < 0)
	{
		fprintf(stderr, "[%s] conflib_getNthTokenInFileSection fail[SHARED_MEMORY_KEY : SHM_KEEPALIVE]\n", __FUNCTION__);
		return -1;
	}
	ikeepalive = (key_t)strtol(ckeepalive,0,0);

	if( (shmid = (int) shmget (ikeepalive, sizeof (T_keepalive), 0666|IPC_CREAT)) < 0)
	{
		perror("shmget KEEPALIVE");
		fprintf(stderr, "[%s] shmget fail(SHM_KEEPALIVE); key=0x%x, err=%d[%s]\n", __FUNCTION__, ikeepalive, errno, strerror(errno));
		return -1;
	}

	if( (keepalive = (T_keepalive*)shmat (shmid, (char*)0, 0)) == (T_keepalive*)-1)
	{
		perror("shmat KEEPALIVE");
		fprintf(stderr, "[%s] shmat fail(SHM_KEEPALIVE); key=0x%x, err=%d[%s]\n", __FUNCTION__, ikeepalive, errno, strerror(errno));
		return -1;
	}

	/*	keepaliveIndex를 구한다	*/
	if( (fp = fopen(l_sysconf,"r")) == NULL)
	{
		fprintf(stderr, "[%s] fopen fail[%s]; err=%d(%s)\n", __FUNCTION__, l_sysconf, errno, strerror(errno));
		return -1;
	}

	if(conflib_seekSection(fp, "APPLICATIONS") < 0)
	{
		fprintf(stderr, "[%s] conflib_seekSection(APPLICATIONS) fail\n", __FUNCTION__);
		fclose(fp);
		return -1;
	}

	while(fgets(getBuf,sizeof(getBuf),fp) != NULL)
	{
		if(getBuf[0] == '[')						/*	end of section			*/
			break;

		if( (getBuf[0]=='#') || (getBuf[0]=='\n'))	/*	comment line or empty	*/
			continue;

		sscanf (getBuf,"%s",token);
		if(!strcasecmp(token,processName))
		{
			fprintf(stdout, "[%s] keepaliveIndex = %d\n", __FUNCTION__, keepaliveIndex);
			fclose(fp);
			return keepaliveIndex;
		}
		keepaliveIndex++;
	}
	fclose(fp);

	return -1;
}

void keepalivelib_increase(void)
{
	keepalive->cnt[keepaliveIndex]++;
}
