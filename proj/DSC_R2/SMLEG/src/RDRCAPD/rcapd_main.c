/* File Include */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/file.h>
#include <errno.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <ctype.h>
#include <utillib.h>
#include <init_shm.h>
#include <shmutil.h>

#include "rcapd.h"
#include "comm_util.h"


char 	vERSION[7] = "R1.0.0";
char 	trcBuf[TRCBUF_LEN], trcTmp[TRCTMP_LEN];
char 	sysLable[COMM_MAX_NAME_LEN], mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN];
char 	logfilepath[256];

int     JiSTOPFlag, FinishFlag;
int     pid, dMyQid /* RLEG Qid */, dIxpcQid /* trace */, dANAQid;

SFM_SysCommMsgType   *loc_sadb;


int main (int argc, char *argv[])
{
	int			dRet;
	int 		check_Index;

	pid = getpid();
#if 1
	/* PROCESS RUN CHECK */
    if ((check_Index = check_my_run_status("RDRCAPD")) < 0)
       exit(0);
#endif
	/* LOG INIT */
	Init_logdebug( getpid(), "RDRCAPD", "/DSC/APPLOG");

	/* PROC INIT */
	if ((dRet = initProc()) < 0) {
		dAppLog(LOG_CRI, "MAIN][PROCESS INITIAL FAIL]:[RET][%d", dRet);
		exit(0);
	}

	/* KEEP-ALIVE CHECK */
	if ((dRet = keepalivelib_init("RDRCAPD")) < 0) {
		dAppLog(LOG_CRI, "MAIN][KEEPALIVE INITIAL FAIL]:[RET][%d", dRet);
        exit(0);
	}

	/* SET VERSION */
	if( (dRet = set_version(SEQ_PROC_RDRCAPD, vERSION)) < 0) {
		dAppLog( LOG_CRI, "MAIN][SET_VERSION ERROR (RET=%d,IDX=%d,VER=%s)",
				dRet,SEQ_PROC_RDRCAPD,vERSION);
	}

	dAppLog(LOG_CRI, "### PROCESS INITIAL SUCCESS - RDRCAPD START]:[PID:%d][VERSION:%s] ###", pid, vERSION);


	/* MAIN FUNCTION */
	theApp();

	return 0;
}


