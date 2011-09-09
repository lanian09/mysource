/* File Include */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>
 
#include "define.h"
#include "utillib.h"
#include "rcapd.h"
#include "conflib.h"

/* Definition of New Constants */

/* Declaration of Global Variable */

/* Declaration of Extern Global Variable */
extern char		logfilepath[256];

/* Declaration of Function Definition */
extern int conflib_getNthTokenInFileSection ( char *fname, char *section, char *keyword, int  n, char *string );

        
int init_ipcs(void)
{
	int     key;
	char    tmp[64], fname[256];

	sprintf(fname,"%s", DEF_SYSCONF_FILE );

	/* GET SYSTEM LABEL */
	if (conflib_getNthTokenInFileSection (fname, "[GENERAL]", "SYSTEM_LABEL", 1, tmp) < 0) {
		dAppLog (LOG_CRI, "[init_ipcs] CAN'T GET SYSTEM LABEL err=%s", strerror(errno));
		return -1;
	}
	else {
		strcpy(sysLable, tmp);
		strcpy(mySysName, sysLable);
		strcpy(myAppName, "RLEG");
		dAppLog(LOG_INFO, "SYSTEM LABEL=[%s]", sysLable);
	}

	if (conflib_getNthTokenInFileSection (fname, "[APPLICATIONS]", "RDRANA", 1, tmp) < 0) {
		dAppLog (LOG_CRI, "[init_ipcs] can't get queue key of RANA err=%s", strerror(errno));
		return -1;
	} else
		key = strtol(tmp, 0, 0);
	if((dANAQid = msgget(key, 0666|IPC_CREAT)) < 0) {
		dAppLog(LOG_CRI, "[init_ipcs] [FAIL] MSGGET RADIUS : [%d][%s]",
				errno, strerror(errno) );
		return -2;
	}

	return 0;
}


int initProc (void)
{
	int 	ret;

	SetUpSignal();

	if ((ret = init_ipcs ()) < 0) {
		return ret;
	}
	
	/* SCM INFO */
	if ((ret = dGetConfig_LEG ()) < 0) {
		return ret;	
	}

	mmc_yh_init ();

	return 0;
}


void finProc (void)
{
    dAppLog(LOG_CRI, "### PROGRAM IS NORMALLY TERMINATED(cause = %d) ###", FinishFlag);
    dAppLog(LOG_CRI, "");
	
    exit(0);
}


int dGetConfig_LEG (void)
{
	int     lNum, rowCnt=0, ret;
	char    *env;
	char    iv_home[64];
	char    module_conf[64];
	char    getBuf[256], token[9][64];
	FILE    *fp=NULL;

	if( (env = getenv(IV_HOME)) == NULL) {
		dAppLog(LOG_CRI, "[%s:%s:%d] not found %s environment name", __FILE__, __FUNCTION__, __LINE__, IV_HOME);
		return -1;
	}
	strcpy(iv_home, env);
	sprintf(module_conf, "%s", DEF_SYSCONF_FILE);

	if ((fp = fopen(module_conf, "r")) == NULL) {
		dAppLog(LOG_CRI,"[dGetConfig_LEG] fopen fail[%s]; err=%d(%s)", module_conf, errno, strerror(errno));
		return -1;
	}

	/* [RLEG_CONFIG] section으로 이동 */
	if ((lNum = conflib_seekSection (fp,"RLEG_CONFIG")) < 0) {
		dAppLog(LOG_CRI,"[dGetConfig_LEG] Secsion not found");
		return -1;
	}

	/* 등록된 시스템들의 이름과 IP_ADDRESS를 저장한다. */
	while (fgets(getBuf,sizeof(getBuf),fp) != NULL)
	{
		if (getBuf[0] == '[') /* end of section */
			break;
		if (getBuf[0]=='#' || getBuf[0]=='\n') /* comment line or empty */
			continue;

		if ((ret = sscanf(getBuf,"%s%s%s%s%s%s%s%s%s"
						, token[0],token[1],token[2],token[3],token[4],token[5],token[6],token[7],token[8])) < 9)
		{
			dAppLog(LOG_CRI,"[dGetConfig_LEG] config syntax error; file=%s, lNum=%d, ret = %d"
					, module_conf , lNum, ret);
			fclose(fp);
			return -1;
		}

		rowCnt++;
	}
	fclose(fp);
	return 0;
}


/************************************************************************************
	SIGNAL FUNCTION
***********************************************************************************/
void UserControlledSignal(int sign)
{
    dAppLog(LOG_CRI, "UserControlledSignal][DEFINED SIGNAL IS RECEIVED, signal = %d", sign);
    JiSTOPFlag = 0;
    FinishFlag = sign;
	finProc();
}

void IgnoreSignal(int sign)
{
    if (sign != SIGALRM)
        dAppLog(LOG_CRI, "IgnoreSignal][UNWANTED SIGNAL IS RECEIVED, signal = %d", sign);
    signal(sign, IgnoreSignal);
}

void SetUpSignal()
{
//	int i;
    JiSTOPFlag = 1;

#if 0
	for (i=0; i<20; i++)
	{
    	signal(i, UserControlledSignal);

	}
#endif
    /* WANTED SIGNALS   */
    signal(SIGTERM, UserControlledSignal);
    signal(SIGKILL, UserControlledSignal);
    signal(SIGINT,  UserControlledSignal);
    signal(SIGQUIT, UserControlledSignal);

    /* UNWANTED SIGNALS */
    signal(SIGHUP,  IgnoreSignal);
    signal(SIGALRM, IgnoreSignal);
    signal(SIGPIPE, IgnoreSignal);
    signal(SIGPOLL, IgnoreSignal);
    signal(SIGPROF, IgnoreSignal);
    signal(SIGUSR1, IgnoreSignal);
    signal(SIGUSR2, IgnoreSignal);
    signal(SIGVTALRM, IgnoreSignal);
    signal(SIGCLD, SIG_IGN);
    dAppLog(LOG_DEBUG, "SIGNAL HANDLER WAS INSTALLED");
}


