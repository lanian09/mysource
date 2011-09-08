//#include "dagapi.h"
//#include "dagutil.h"
#include <sys/shm.h>
#include <errno.h>
#include <signal.h>

#include <shmutil.h>
#include "capd_def.h"
#include <utillib.h>
#include <init_shm.h>

#include "mems.h"
#include "nifo.h"

/* Function Time Check */
#include "func_time_check.h"

extern stMEMSINFO 		*pstMEMSINFO;
extern int 				dANAQid;
extern int 				semid_mif;
extern unsigned int		Collection_Cnt;
extern SFM_SysCommMsgType   *loc_sadb;
extern int     			gdDevCnt;
extern char    			gszDevName[MAX_DEV_CNT][MAX_DEV_NAME_SIZE];

/* Function Time Check */
st_FuncTimeCheckList    stFuncTimeCheckList;
st_FuncTimeCheckList    *pFUNC = &stFuncTimeCheckList;

int conflib_getNthTokenInFileSection (char *fname, char *section, char *keyword, int n, char *string);

extern int	dSend_CAPD_Data(stMEMSINFO *pstMEMSINFO, S32 dSndMsgQ, U8 *pNode, U32 sec);

void CatchSignal(int sig)
{
	int 	ret;

	dAppLog(LOG_CRI, "%s: SIGNAL[%d] CATCH. FINISH PROGRAM", __FUNCTION__, sig);
	loop_continue = 0;

	if((ret = dSend_CAPD_Data(pstMEMSINFO, dANAQid, NULL, 0)) < 0) {
		dAppLog(LOG_CRI, "[%s.%d] dSend_CAPD_Data [%d][%s]", __FUNCTION__, __LINE__, ret, strerror(-ret));
		exit(111);
	} 
	exit(0);
}

void IgnoreSignal(int sig)
{
	int 	ret;

	if( sig != SIGALRM )
		dAppLog(LOG_INFO, "SIGNAL[%d] RECEIVED.", sig);
	signal(SIGALRM, IgnoreSignal);

	if( sig == SIGUSR1 ) {
		if((ret = dSend_CAPD_Data(pstMEMSINFO, dANAQid, NULL, 0)) < 0) {
			dAppLog(LOG_CRI, "[%s.%d] dSend_CAPD_Data [%d][%s]", __FUNCTION__, __LINE__, ret, strerror(-ret));
		} 
	}
}

void SetupSignal(void)
{
    /* EXIT SIGNALS   */
	//signal(SIGSEGV, CatchSignal);
	signal(SIGHUP, CatchSignal);
	signal(SIGINT, CatchSignal);
	signal(SIGTERM, CatchSignal);
	signal(SIGPIPE, CatchSignal);
	signal(SIGQUIT, CatchSignal);

    /* IGNORE SIGNALS */
    signal(SIGALRM, IgnoreSignal);
    signal(SIGPOLL, IgnoreSignal);
    signal(SIGPROF, IgnoreSignal);
    signal(SIGUSR1, IgnoreSignal);
    signal(SIGUSR2, IgnoreSignal);
    signal(SIGVTALRM, IgnoreSignal);
    signal(SIGCLD, SIG_IGN);
	dAppLog(LOG_DEBUG, "SIGNAL HANDLER WAS INSTALLED");
}

int INIT_CAPD_IPCS(void)
{
	int 	ret, i;
	int     key, shmId;
	char    tmp[64], fname[256];
	char   	szLogBuf[1024];
	char	tmpStr[24];

	sprintf(fname,"%s", DEF_SYSCONF_FILE );

	/* GET SYSTEM LABEL */
	if (conflib_getNthTokenInFileSection (fname, "[GENERAL]", "SYSTEM_LABEL", 1, tmp) < 0) {
		sprintf( szLogBuf, "CAN'T GET SYSTEM LABEL err=%s", strerror(errno));
		dAppWrite( LOG_CRI, szLogBuf );
		return -1;
	}
	else {
		strcpy(syslabel, tmp);
		dAppLog(LOG_CRI, "SYSTEM LABEL=[%s]", syslabel);
	}

	/* INIT_SHM: SHM_LOC_SADB */
    if(conflib_getNthTokenInFileSection(fname, "SHARED_MEMORY_KEY", "SHM_LOC_SADB", 1, tmp) < 0)
        return -2;
    key = strtol(tmp, 0, 0);

    if( (shmId = (int)shmget(key, sizeof(SFM_SysCommMsgType), 0666 | IPC_CREAT)) < 0)
    {
        if(errno != ENOENT)
        {
            fprintf(stderr, "[%s:%s:%d] shmget fail; key=0x%x, err=%d(%s)\n", __FILE__, __FUNCTION__, __LINE__, key, errno, strerror(errno));
            return -3;
        }
    }
    
    if( (loc_sadb = (SFM_SysCommMsgType*)shmat(shmId, 0, 0)) == (SFM_SysCommMsgType*)-1)
    {   
        fprintf(stderr, "[%s:%s:%d] shmat fail; key=0x%x, err=%d(%s)\n", __FILE__, __FUNCTION__, __LINE__, key, errno, strerror(errno));
        return -4;
    }

	/* INIT SHM : GENINFO */
	if (conflib_getNthTokenInFileSection (fname, "[SHARED_MEMORY_KEY]", "S_SSHM_GENINFO", 1, tmp) < 0) {
		sprintf( szLogBuf, "CAN'T GET SHM KEY OF S_SSHM_GENINFO err=%s", strerror(errno));
		dAppWrite( LOG_CRI, szLogBuf );
		return -5;
	}
	else
		key = strtol(tmp, 0, 0);

	ret = Init_GEN_INFO( key );
	if( ret < 0 ) {
		sprintf( szLogBuf, "FAIL GENINFO");
		dAppWrite( LOG_CRI, szLogBuf );
		return -6;
	}

START_FUNC_TIME_CHECK(pFUNC, 0);
	pstMEMSINFO = nifo_init_zone("CAPD", SEQ_PROC_CAPD, DEF_NIFO_ZONE_CONF_FILE);
END_FUNC_TIME_CHECK(pFUNC, 0);

PRINT_FUNC_TIME_CHECK(pFUNC);

	if (conflib_getNthTokenInFileSection (fname, "[APPLICATIONS]", "PANA", 1, tmp) < 0) {
		dAppLog( LOG_CRI, "CAN'T GET MSGQ KEY OF PANA err=%s", strerror(errno));
		return -8;
	}
	else
		key = strtol(tmp, 0, 0);
	
	if( (dANAQid = nifo_msgq_init(key)) < 0) {
		dAppLog( LOG_CRI, "[%s.%d] nifo_msgq_init [%d][%s]", __FUNCTION__, __LINE__, dANAQid, strerror(-dANAQid));
		return -9;
	}

	/* ADD BY JUNE, 2010-12-21 */
	if (conflib_getNthTokenInFileSection (fname, "[CAP_PROBE]", "COUNT", 1, tmp) < 0) {
		dAppLog( LOG_CRI, "CAN'T GET COUNT of DEVICE PROBE err=%s", strerror(errno));
		return -10;
	}
	else
		 gdDevCnt = strtol(tmp, 0, 0);

	for (i=0 ; i < gdDevCnt ; i++)
	{
		if (i >= MAX_DEV_CNT) break;
		sprintf(tmpStr, "IF_NAME%d", i);
		if (conflib_getNthTokenInFileSection (fname, "[CAP_PROBE]", tmpStr, 1, tmp) < 0) {
			dAppLog( LOG_CRI, "CAN'T GET COUNT of DEVICE PROBE err=%s", strerror(errno));
			return -11;
		}
		else {
			strcpy(gszDevName[i], tmp);
			dAppLog(LOG_CRI, "CAP IF_NAME[%d]=[%s]", i, gszDevName[i]);
		}
	}

	return 0;
}

