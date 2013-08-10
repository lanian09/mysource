#include "keepalivelib.h"
#include "conflib.h"

#include <sys/ipc.h>

T_keepalive	*keepalive;

int			keepaliveIndex;

int keepalivelib_init(char *processName)
{
	char	*env;
	char	l_sysconf[64];
	char	ckeepalive[10];
	char	getBuf[256], token[CONFLIB_MAX_TOKEN_LEN];
	key_t	ikeepalive;
	int		shmid;
	FILE	*fp;

	keepaliveIndex = 0;

	if ((env = getenv(IV_HOME)) == NULL) {
		fprintf(stderr, "keepalivelib_init] not found %s environment name\n", IV_HOME);
		return -1;
	}

	sprintf(l_sysconf, "%s/%s", env, SYSCONF_FILE);
    if (conflib_getNthTokenInFileSection(l_sysconf,"SHARED_MEMORY_KEY",
        "SHM_KEEPALIVE", 1, ckeepalive) < 0) {
        fprintf(stderr,"keepalivelib_init] conflib_getNthTokenInFileSection fail[SHARED_MEMORY_KEY : SHM_KEEPALIVE] \n");
        return -1;
    }
    ikeepalive = (key_t)strtol(ckeepalive,0,0);

    if ((shmid = (int) shmget (ikeepalive, sizeof (T_keepalive), 0666|IPC_CREAT)) < 0) {
        perror ("shmget KEEPALIVE");
        fprintf(stderr,"[keepalivelib_init] shmget fail(SHM_KEEPALIVE); key=0x%x, err=%d[%s]\n", ikeepalive, errno, strerror(errno));
        return -1;
    }
    //if ( (int)(keepalive = (T_keepalive *)shmat (shmid, (char *) 0,0)) == -1 ) {
    if ( (long)(keepalive = (T_keepalive *)(long)shmat (shmid, (char *) 0,0)) == -1 ) 
    {
        perror ("shmat KEEPALIVE");
        fprintf(stderr,"[keepalivelib_init] shmat fail(SHM_KEEPALIVE); key=0x%x, err=%d[%s]\n", ikeepalive, errno, strerror(errno));
        return -1;
    }

    /* keepaliveIndex를 구한다 */
    if((fp = fopen(l_sysconf,"r")) == NULL) {
        fprintf(stderr,"[keepalivelib_init] fopen fail[%s]; err=%d(%s)\n", l_sysconf, errno, strerror(errno));
        return -1;
    }
    if ( conflib_seekSection (fp, "APPLICATIONS") < 0 ) {
        fprintf(stderr,"[keepalivelib_init] conflib_seekSection(APPLICATIONS) fail\n");
		fclose(fp);
        return -1;
    }

	while (fgets(getBuf,sizeof(getBuf),fp) != NULL) {
		if (getBuf[0] == '[') /* end of section */
			break;
		if (getBuf[0]=='#' || getBuf[0]=='\n') /* comment line or empty */
			continue;

		sscanf (getBuf,"%s",token);
		if (!strcasecmp(token,processName)) {
			fprintf(stdout, "keepaliveIndex = %d\n", keepaliveIndex);
			fclose(fp);
			return keepaliveIndex;
		}
		keepaliveIndex++;
	}
	fclose(fp);
	return -1;
}

void keepalivelib_increase()
{
	keepalive->cnt[keepaliveIndex]++;
}
