/**A.1* FILE INCLUSION ********************************************************/
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

#include "leg.h"
#include "leg_sm_sess.h"
#include "comm_util.h"
#include "comm_trace.h"
#include "comm_timer.h"
#include "comm_session.h"

/**B.1* DEFINITION OF NEW CONSTANTS *******************************************/

/**C.1* DECLARATION OF VARIABLES **********************************************/
char 	vERSION[7] = "R2.0.0";	// BEFORE: R1.2.0(2011-03-02) -> R2.0.0(2011-05-05)
char 	trcBuf[TRCBUF_LEN], trcTmp[TRCTMP_LEN];
char 	sysLable[COMM_MAX_NAME_LEN], mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN];
char 	logfilepath[256];

int     JiSTOPFlag, FinishFlag;
int     pid, dMyQid /* RLEG Qid */, dIxpcQid /* trace */, dRADIUSQid;

SFM_SysCommMsgType   *loc_sadb;
//SMNB_HANDLE 		gSCE_nbapi[MAX_SM_CONN_COUNT+1];
SM_INFO				gSCM[MAX_SM_NUM];
SM_CONN          	gSMConn;
_mem_check			*gpShmem;

// TIMEOUT.conf 구조체 
MPTimer            	*gpMPTimer[DEF_SET_CNT];
MPTimer            	*gpCurMPTimer;

st_NOTI				gstIdx;
st_NOTI				*gpIdx = &gstIdx;

LEG_DATA_SUM        *gpstCallInfo[DEF_STAT_SET_CNT];
LEG_TOT_STAT_t      *gpstTotStat[DEF_STAT_SET_CNT];
unsigned int        gSIdx = 0;
unsigned int        gCIdx = 0;
unsigned int		gMyIdx;
char				g_szProcName[32];

extern int 	dReadFLTIDXFile(void);
void vGetProcName(char *path, char *procname);
int dGetProcIndex(char *procname);

/**D.1*  DEFINITION OF FUNCTIONS  *********************************************/
void period_heartbeat (int k, void *d)
{
	//dAppLog(LOG_INFO, "HEARTBEAT CALLBACK FUNCTION %d CALLED", k);
	keepalivelib_increase();
	set_cb_timeout(period_heartbeat,  1, NULL, TIMER_HB_PERIOD);
}


int main (int argc, char *argv[])
{
	GeneralQMsgType 	rxGenQMsg;
	int			msgq_size=0;
	int			rtn=0, isConn = 0;
	int			dRet;
	char        tmpBuff[65536];
	time_t		now, tStatTime = 0, tCpsTime = 0, tLastTime = 0, tChkTime = 0;
	struct tm 	tmLast, tmStat, tmCLast, tmCps;
	struct timeval  cur, prev;

	vGetProcName (argv[0], g_szProcName);
	//gMyIdx = (unsigned int)dGetProcIndex(g_szProcName);
	gMyIdx = 0; // added by dcham 20110530 for SM connection 축소(5=>1)
	pid = getpid();

	/* CHECK RUN PROCESS */
#if 1
    if (check_my_run_status(g_szProcName) < 0)
       exit(0);
#endif	
	/* INIT LOG */
	Init_logdebug (getpid(), g_szProcName, "/DSC/APPLOG");
	dAppLog(LOG_CRI,"%s %s %d][PROCESS INITIALIZING ... ", g_szProcName, vERSION, pid);
	
	/* INIT KEEP-ALIVE */
	if((dRet = keepalivelib_init (g_szProcName)) < 0) {
		dAppLog(LOG_CRI, "INIT][%s KEEPALIVE INITIAL FAIL]:[RET][%d", g_szProcName, dRet);
        exit(0);
	}

	/* INIT PROC */
	if((dRet = initProc()) < 0) {
		dAppLog(LOG_CRI, "INIT][PROCESS INITIAL FAIL]:[RET][%d", dRet);
		exit(0);
	}

	/* INIT SYNC CONFIG */
	dRet = dReadFLTIDXFile();
	if( dRet < 0 ) {
		dAppLog( LOG_CRI, "INIT][dReadFLTIDXFile() FAIL RET:%d", dRet);
		exit(0);
	}

	/* REBUILD SESS TIMER  */
	sm_timer_reconstructor ();

	/* CLEAR PREVIOUS MESSAGES */
	while(msgrcv(dMyQid, tmpBuff, sizeof(tmpBuff), 0, IPC_NOWAIT) > 0);

	/* SET VERSION */
	if((dRet = set_version(SEQ_PROC_SESSANA0+gMyIdx, vERSION)) < 0) {
		dAppLog( LOG_CRI, "INIT][%s SET_VERSION ERR (RET=%d,IDX=%d,VER=%s)",
				g_szProcName, dRet, SEQ_PROC_SESSANA0+gMyIdx, vERSION);
	}

	/* SET HB */
	set_cb_timeout(period_heartbeat,  1, NULL, TIMER_HB_PERIOD);

	/* CONNECTION to SM*/
	isConn = connectSCE (DO_CONN_ON_NB);
	if (isConn==0) {
		loc_sadb->smConn[gMyIdx].dConn = DISCONNECTED;
		dAppLog(LOG_CRI, "INIT][SM CON OFF(%d)", loc_sadb->smConn[gMyIdx].dConn);
	}
	else {
		loc_sadb->smConn[gMyIdx].dConn = CONNECTED;
		dAppLog(LOG_CRI, "INIT][SM CON ON(%d)", loc_sadb->smConn[gMyIdx].dConn);
	}


	tLastTime = now = time(0);

	tStatTime = (now/DEF_STAT_UNIT)*DEF_STAT_UNIT;		// 5min statistic
	localtime_r((time_t*)&tStatTime, &tmLast);
	gSIdx = tmLast.tm_min % DEF_STAT_SET_CNT;

	tCpsTime = (now/CPS_UNIT)*CPS_UNIT;					// 5sec sampling
	localtime_r((time_t*)&tCpsTime, &tmCLast);
	gCIdx = tmCLast.tm_sec % DEF_STAT_SET_CNT;

	dAppLog(LOG_CRI,"%s %s %d][PROCESS INIT SUCCESS", g_szProcName, vERSION, pid);
	dAppLog(LOG_CRI,"%s %s %d][PROCESS STARTED", g_szProcName, vERSION, pid);
	/***********************************  MAIN FUNCTION *********************************/
	/* MAIN LOOP */

	while (JiSTOPFlag)
	{
		tChkTime = now = time(0);
		gettimeofday (&cur, NULL);

		// CPS SHM INDEX
		tCpsTime = (now/CPS_UNIT)*CPS_UNIT;				// 5sec sampling
		localtime_r((time_t*)&tCpsTime, &tmCps);
		if( tmCLast.tm_sec != tmCps.tm_sec ) {
			gCIdx = tmCps.tm_sec % DEF_STAT_SET_CNT;
			tmCLast = tmCps;
		}

		// STATISTIC SHM INDEX
		tStatTime = (now/DEF_STAT_UNIT)*DEF_STAT_UNIT;	// 5min statistic
		localtime_r((time_t*)&tStatTime, &tmStat);
		if( tmLast.tm_min != tmStat.tm_min ) {
			gSIdx = tmStat.tm_min%DEF_STAT_SET_CNT;
			tmLast = tmStat;
		}

		// 1SEC PERIOD 
		if( tChkTime != tLastTime ) {
			// TIMER INVOKE
			timerN_invoke(gpHashTimer_SM);
			tLastTime = tChkTime;

			dAppLog(LOG_INFO, "[SYSTEM MODE : %d]", loc_sadb->loc_system_dup.myLocalDupStatus );
			dAppLog(LOG_INFO, "CUR SM SESS : %d, dConn[my]=%u", 
					gpShmem->sm_sess[gMyIdx], loc_sadb->smConn[gMyIdx].dConn);

			// SM CONN CHECK: AUTO CONNECTION CASE
			//checkConnectSCE();
		}

		// 9SEC PERIOD
		if((cur.tv_sec != prev.tv_sec) && (cur.tv_sec % 9 == 0)) {
			// SM CONN CHECK: AUTO CONNECTION CASE
			checkConnectSCE();
		}
		prev.tv_sec  = cur.tv_sec;

		// MSGQ PROCESS
		msgq_size = sizeof(GeneralQMsgType) - sizeof(long);	
		if((rtn = msgrcv(dMyQid, &rxGenQMsg, msgq_size, 0, IPC_NOWAIT)) < 0) { // MSG_NOERROR IPC_NOWAIT
			if (errno != ENOMSG) {
				dAppLog(LOG_CRI, "MAIN] QID=%d MSGRCV ERR=%d(%s), "
						, dMyQid, errno, strerror(errno));
			}
			usleep(1); 
			continue; 
		}

		branchMessage (&rxGenQMsg);

	} /* while-loop end */
	
	finProc();
	return 0;
}

void vGetProcName(char *path, char *procname)
{
	char *ptr, *token;

	ptr = strstr(path, "/");
	if (ptr) {
		token = strtok(path, "/");
		while(token != NULL)
		{
			sprintf(procname, "%s", token);
			token = strtok(NULL, "/");
		}
	} else {
		sprintf(procname, "%s", path);
	}
	return;
}

int dGetProcIndex(char *procname)
{
	int i, len;

	if(procname[0] == '\0')
		return -1;

	len = strlen(procname);
	for(i = 0; i < len; i++) {
		if(procname[i] >= '0' && procname[i] <= '9') break;
	}

	if(i == len) return 0;

	return atoi(&procname[i]);
}

