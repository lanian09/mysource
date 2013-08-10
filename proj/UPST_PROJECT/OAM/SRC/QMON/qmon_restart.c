/*******************************************************************************
			DQMS Project

	Author   :
	Section  : QMON
	SCCS ID  : @(#)qmon_restart.c	1.1
	Date     :
	Revision History :

	Description :

	Copyright (c) uPRESTO 2005
*******************************************************************************/

/***** A.1 * File Include *******************************/

/* SYS HEADER */
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/msg.h>
#include <string.h>
/* LIB HEADER */
#include "filedb.h"
/* PRO HEADER */
#include "path.h"
#include "procid.h"
#include "mmcdef.h"
#include "msgdef.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
#include "almstat.h"
/* LOC HEADER */
#include "qmon_restart.h"
#include "qmon_func.h"

extern st_WNTAM *fidb;
extern int g_dCondQid;
extern int g_dCondQid;

extern unsigned char gucSysNo;
extern unsigned char gucSysNo;
extern unsigned char gucSysNo;

time_t check_chsmd;

extern int log_print(int dIndex, char *fmt, ...);
extern int util_makenid(unsigned char ucSysType, long long *pllNID);
extern int dCheckCHSMDStatus(void);

void SendCOND( char loctype, char invtype, char invno, char almstatus, char oldalm );

int restartCHSMD(void)
{
	int		pid;
	char	fname[BUF_SIZE];

	sprintf(fname,"%s/%s", APP_HOME_BIN, "CHSMD");

	if( (pid = fork()) < 0)
	{
		log_print(LOGN_CRI,"CAN'T RESTART CHSMD [%d:%s]", errno, strerror(errno));
		return -1;
	}
	else if(pid == 0)
	{
		(void)freopen("/dev/null", "w", stdout);
		if(execl(fname, "CHSMD", (char*)0) < 0)
			log_print(LOGN_CRI,"CAN'T EXECUTE CHSMD [%d:%s]", errno, strerror(errno));

		exit(0);
	}
	else
		return pid;
}

int send_CHSMD(int pid)
{
	long        pmsg[4];
	time_t      when;
	int         dMsgLen, dRet;
	pst_MsgQ    pstSndMsg;
	pst_MsgQSub pstMsgSub;
	unsigned char   *pNODE;

	if( (dRet = dGetNode(&pNODE, &pstSndMsg)) < 0 ){
		log_print(LOGN_CRI,LH" FAILED IN dGetNode(QMON)"EH,
				LT,ET);
        return -1;
	}

	time(&when);

	pmsg[0] = 3;
	pmsg[1] = 0;
	pmsg[2] = pid;
	pmsg[3] = when;

	pstMsgSub = (pst_MsgQSub)&pstSndMsg->llMType;
	pstMsgSub->usType = DEF_SYS;
	pstMsgSub->usSvcID = SID_STATUS;
	pstMsgSub->usMsgID = MID_ALARM;
	pstSndMsg->llMType = 3;
	pstSndMsg->llNID = 0;
	pstSndMsg->ucNTAFID = 0;
	pstSndMsg->ucProID = SEQ_PROC_QMON;
	pstSndMsg->llIndex = 0;

	pstSndMsg->dMsgQID = 0;
	pstSndMsg->usBodyLen = sizeof(long)*4;
	pstSndMsg->usRetCode = 0;

	memcpy( pstSndMsg->szBody, pmsg, pstSndMsg->usBodyLen );

	dMsgLen = DEF_MSGHEAD_LEN + pstSndMsg->usBodyLen;

	if( (dRet = dMsgsnd(SEQ_PROC_CHSMD, pNODE)) < 0 ){
		log_print(LOGN_CRI, LH" FAILED IN dMsgsnd(CHSMD)",LT);
		return -1;
	}
	log_print(LOGN_DEBUG,"SUCCESS SEND TO CHSMD");

	return 0;
}




void SendCOND( char loctype, char invtype, char invno, char almstatus, char oldalm )
{
    st_almsts almsts;
    int     slen, dRet;

    int             dMsgLen;
    pst_MsgQ         pstSndMsg;
    pst_MsgQSub     pstSndSub;
	unsigned char   *pNODE;

	if( (dRet = dGetNode(&pNODE, &pstSndMsg)) < 0 ){
		log_print(LOGN_CRI,LH" FAILED IN dGetNode(COND)",LT);
        return;
	}

    memset( &almsts, 0x00, sizeof(st_almsts));

    memset( pstSndMsg, 0x00, sizeof(st_MsgQ) );

    almsts.ucSysType = SYSTYPE_TAM;
    almsts.ucSysNo = gucSysNo;    /* TAF */
    almsts.ucLocType = loctype;
    almsts.ucInvType = invtype;
    almsts.ucInvNo = invno;
    almsts.ucAlmLevel = almstatus;
    almsts.ucOldAlmLevel = oldalm;
    almsts.llLoadVal = 0;

    time(&almsts.tWhen);

	slen = sizeof(st_almsts);

    pstSndSub = (pst_MsgQSub)&pstSndMsg->llMType;
    pstSndSub->usType = DEF_SYS;
    pstSndSub->usSvcID = SID_STATUS;
    pstSndSub->usMsgID = MID_CONSOL;

    util_makenid( SEQ_PROC_QMON, &pstSndMsg->llNID );
    pstSndMsg->ucNTAMID = 0;
    pstSndMsg->ucNTAFID = almsts.ucSysNo;
    pstSndMsg->ucProID = SEQ_PROC_QMON;
    pstSndMsg->llIndex = 0;

    pstSndMsg->dMsgQID = 0;

    /*
    * NTAFT_HEADER_LEN < Socket Header Size ¢¬¢¬A¡© +
    */
    pstSndMsg->usBodyLen = slen;
    pstSndMsg->usRetCode = 0;

    memset( pstSndMsg->szBody, 0x00, MNG_PKT_HEAD_SIZE );
    memcpy( pstSndMsg->szBody, &almsts, pstSndMsg->usBodyLen );

    dMsgLen = sizeof(st_MsgQ) - MAX_MSGBODY_SIZE + pstSndMsg->usBodyLen;

	if( (dRet = dMsgsnd(SEQ_PROC_COND, pNODE)) < 0 ){
		log_print(LOGN_CRI, LH" FAILED IN dMsgsnd(COND)"EH, 
		        LT,ET);
		return;
	}
	log_print(LOGN_DEBUG, LH" SUCCESS IN dMsgsnd(COND) MESSAGE PROCESSID=%d",
		   	LT, SEQ_PROC_COND);
}

int dCheckCHSMDStatus(void)
{
	int		fd, pid;
	time_t	tNow;
	char	szProcName[32];

	if( (fidb->stNTAM.mpsw[0] == STOP) || (fidb->stNTAM.mpswinfo[0].when < 0) || (fidb->stNTAM.mpswinfo[0].pid == 0))
	{
		log_print(LOGN_DEBUG, "CHSMD NORMAL TERMINATED when:%lld pid:%lld", fidb->stNTAM.mpswinfo[0].when, fidb->stNTAM.mpswinfo[0].pid);
		return 0;
	}

	sprintf(szProcName, "/proc/%lld", fidb->stNTAM.mpswinfo[0].pid);
	if( (fd = open(szProcName, O_RDONLY)) < 0)
	{
		log_print(LOGN_CRI, "CHSMD ABNORMAL DEAD!!");
		time(&tNow);
		SendCOND(LOCTYPE_PROCESS, INVOKETYPE_USERPROC, 0, CRITICAL, NORMAL);

		if( (pid = restartCHSMD()) < 0)
		{
			log_print( LOGN_CRI, "FAIL IN restartCHSMD pid:%d", pid );
			return -1;
		}
		else
		{
			if(send_CHSMD(pid)< 0)
			{
				log_print(LOGN_CRI,"FAILED IN send_CHSMD()");
				return -2;
			}
		}
	}
	else
	{
		log_print( LOGN_INFO, "CHSMD ALIVE" );
		close( fd );
		return 0;
	}

	return 1;
}
