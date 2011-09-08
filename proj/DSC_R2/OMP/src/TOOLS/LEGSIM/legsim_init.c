#include "legsim.h"
#include "sfm_msgtypes.h"

extern  int     stmdQid;
extern  int     fimdQid;
extern  char    mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN];

int InitSys()
{
    char    *env, fname[256];
    int     key;
	

    if ((env = getenv(MY_SYS_NAME)) == NULL) {
        fprintf(stderr,"[stmd_init] not found %s environment name\n", MY_SYS_NAME);
        return -1;
    }

    strcpy (mySysName, env);
    strcpy (myAppName, "RLEG");

    if ((env = getenv(IV_HOME)) == NULL) {
        fprintf(stderr,"[InitSys] not found %s environment name\n", IV_HOME);
        return -1;
    }
    sprintf (fname, "%s/%s", env, SYSCONF_FILE);

	//STMD
    key = 0x12014;
    if ((stmdQid = msgget (key, IPC_CREAT|0666)) < 0) {
        fprintf(stderr,"[stmd_init] msgget fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
        return -1;
    }

	//FIMD
	key = 0x12012;
    if ((fimdQid = msgget (key, IPC_CREAT|0666)) < 0) {
        fprintf(stderr,"[fimd_init] msgget fail; key=0x%x, err=%d(%s)\n", key, errno, strerror(errno));
        return -1;
    }

	

    return 1;
}

