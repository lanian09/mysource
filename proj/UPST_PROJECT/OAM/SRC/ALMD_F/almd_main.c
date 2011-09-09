/*******************************************************************************
			DQMS Project

	Author   : tundra
	Section  : ALMD
	SCCS ID  : @(#)almd_main.c	1.1
	Date     : 01/30/02
	Description :

	Copyright (c) uPRESTO 2005
*******************************************************************************/

/**A.1*  File Inclusion *******************************************************/
/* SYS HEADER */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/procfs.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
/* LIB HEADER */
#include "filedb.h"
#include "loglib.h"
#include "utillib.h"
#include "verlib.h"
/* PRO HEADER */
#include "path.h"
#include "msgdef.h"
#include "sshmid.h"
#include "procid.h"
#include "mmcdef.h"
#include "sockio.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
#include "almstat.h"
/* LOC HEADER */
#include "almd_func.h"

/**B.1*  Definition of New Constants ******************************************/
#define INVOKETYPE_USERPROC 0x01
#define STAT_SEND_TIME		300
#define FIDB_SEND_TIME		1

/**C.1*  Declaration of Variables  ********************************************/
int		dIFQid;
int		dALMDQid;
int		dIndex;
unsigned char gucSysNo;

pid_t 	proc_idx;
char  	proc_name[16];

char 	logfilepath[1024];
char 	szLogbuf[1024];

extern int			errno;
st_NTAF		*fidb;
extern pst_GEN_INFO gpGenInfo;
st_keepalive_taf *keepalive;

st_MsgQ			stRcvMsg;

char				vERSION[7] = "R3.0.0";


/**D.1*  Definition of Functions  *********************************************/
int Send_ALMD(void);
int dIsRecvedMessage(st_MsgQ *pstMsg);

extern long long convert_llong(long long value);
extern void AlarmSignal(int sign);
extern int log_print(int dIndex, char *fmt, ...);
extern unsigned char ucGetSysNo(void);
extern int dInitALMD(void);
/**D.2*  Definition of Functions  *********************************************/
void MakeNtafStat(int dIndex, time_t tCheck, st_NtafStatList *stStat);
void ClearNtafStat(int dIndex);
void UserControlledSignal(int sign);
void FinishProgram(int sign);
void IgnoreSignal(int sign);
int Send_STAT(st_NtafStatList *stStat);
int dComputeBytes(void);


/******************************************************************************
 * MAIN FUNCTION
******************************************************************************/
int main(void)
{
	int					dRet, dStatOldIdx, dStatIndex;
	int					dStatOldIdxNew, dStatIndexNew;
	time_t				tNow, tCheck, tChsmd, tValidCheck;
	unsigned long		luCurBytes;
	st_MsgQSub			stRcvSub;
	st_NtafStat			stOldTraffic;
	st_NtafStatList		stNtafStat;

	dIndex	= 0;

	log_init(S_SSHM_LOG_LEVEL, getpid(), SEQ_PROC_ALMD, LOG_PATH"/ALMD", "ALMD");

	if( (dRet = ucGetSysNo()) < 0)
	{
		log_print(LOGN_CRI,"FAILED IN ucGetSysNo()[%d]", dRet);
		exit(-2);
	}
	else
		gucSysNo = dRet;

	if( (dRet = dInitALMD()) < 0)
	{
		log_print(LOGN_CRI, LH" ERROR IN dInitALMD() dRet[%d]", __FILE__,__LINE__,__FUNCTION__, dRet);
		return -1;
	}

	log_print(LOGN_DEBUG, "[MAIN] COMPLETE Init_FIDB [%d]", dRet);

	signal(SIGINT, UserControlledSignal);
    signal(SIGTERM, UserControlledSignal);
    signal(SIGQUIT, UserControlledSignal);

    signal(SIGHUP,  IgnoreSignal);
    signal(SIGALRM, IgnoreSignal);
    signal(SIGPIPE, IgnoreSignal);
    signal(SIGPOLL, IgnoreSignal);
    signal(SIGPROF, IgnoreSignal);
    signal(SIGUSR1, IgnoreSignal);
    signal(SIGUSR2, IgnoreSignal);
    signal(SIGVTALRM, IgnoreSignal);

	tNow	= time( &tNow );
	tChsmd	= tCheck = tNow;

	dStatOldIdx		= ((tNow-300)/300)%12;
	dStatOldIdxNew	= (tNow/300)%12;

	dStatIndex		= dStatOldIdx;
	dStatIndexNew 	= dStatOldIdxNew;

	stOldTraffic.uiFrames	= 0;
	stOldTraffic.ulBytes	= 0;
	luCurBytes				= 0;

	if( (dRet = set_version(S_SSHM_VERSION, SEQ_PROC_ALMD, vERSION)) < 0)
		log_print(LOGN_CRI, "set_version error(dRet=%d,idx=%d,ver=%s)", dRet, SEQ_PROC_ALMD, vERSION);

	log_print(LOGN_CRI, "##### NTAF ALMD %s PROCESS STARTED    #####", vERSION);

	/*	MAIN WHILE LOOP STRART	*/
	while(1)
	{
		if( (dRet = dIsRecvedMessage(&stRcvMsg)) > 0)
		{
			memcpy(&stRcvSub, &stRcvMsg.llMType, sizeof(st_MsgQSub));

			if( (stRcvSub.usType == DEF_SYS) && (stRcvSub.usSvcID == SID_STATUS) && (stRcvSub.usMsgID == MID_ALARM))
			{
				if( (dRet = Send_ALMD()) < 0)
					log_print(LOGN_CRI, LH" FAILED IN Send_ALMD() dRet[%d]", LT, dRet);
			}
		}

		tNow = time(NULL);

		dStatIndexNew 	= (tNow/300)%12;
		if(abs(tNow - tCheck) > FIDB_SEND_TIME)
		{
			luCurBytes	= gpGenInfo->ThruStat[dStatIndexNew].ulBytes;

			if( (dStatIndexNew != dStatOldIdxNew) || (stOldTraffic.ulBytes == 0))
			{
				fidb->bytests.llCur		= luCurBytes/(tNow-tCheck);
				dStatOldIdxNew			= dStatIndexNew;
			}
			else
				fidb->bytests.llCur		= (luCurBytes - stOldTraffic.ulBytes)/(tNow-tCheck);

			if( (dRet = dComputeBytes()) < 0)
				log_print(LOGN_CRI, LH" ERROR IN dComputeBytes() dRet[%d]", LT, dRet);
			else
			{
				log_print(LOGN_DEBUG, LH" llCur[%lld] lMax[%lld] ucStatBytes[0x%02X]", LT,
					fidb->bytests.llCur, fidb->bytests.lMax, fidb->ucStatBytes);
			}

			fidb->tEventUpTime[40]	= tNow;
			stOldTraffic.ulBytes	= luCurBytes;

			if( (dRet = Send_ALMD()) < 0)
				log_print(LOGN_CRI, LH" FAILED IN Send_ALMD() dRet[%d]", LT, dRet);

			tCheck = tNow;
		}

		/*	분단위 시간이 변하는 시점에서 통계를 전송하는 부분으로 변경	*/
		dStatIndex      = ((tNow-300)/300)%12;

		if(dStatIndex != dStatOldIdx)
		{
			log_print(LOGN_DEBUG, "INFORMATION: STAT SEND TIME");

			dStatIndex	= ( (tNow-300)/300)%12;
			tValidCheck	= ( (tNow-300)/300);

			MakeNtafStat(dStatIndex, tValidCheck, &stNtafStat);
			dRet = Send_STAT(&stNtafStat);
			ClearNtafStat(dStatIndex);

			dStatOldIdx = dStatIndex;
		}
	}

	return 0;
}

void SendCOND( char loctype, char invtype, char invno, char almstatus, char oldalm)
{
	st_almsts almsts;
    int     slen;
	unsigned char   *pNODE;
	pst_MsgQSub		pstSndSub;
	pst_MsgQ        pstSndMsg;
	int				dRet;

	if( (dRet = dGetNode(&pNODE, &pstSndMsg)) < 0 ){
		log_print(LOGN_CRI,LH" FAILED IN dGetNode(ALMD_F), errno=%d:%s",
				__FILE__,__LINE__,__FUNCTION__,errno,strerror(errno));
		exit(-1);
	}

    memset(&almsts, 0x00, sizeof(st_almsts));

    almsts.ucSysType		= SYSTYPE_TAF;
    almsts.ucSysNo			= gucSysNo;    /* NTAF */
    almsts.ucLocType		= loctype;
    almsts.ucInvType		= invtype;
    almsts.ucInvNo			= invno;
    almsts.ucAlmLevel		= almstatus;
    almsts.ucOldAlmLevel	= oldalm;
	if(invtype == INVTYPE_TAF_TRAFFIC)
		almsts.llLoadVal = ( (float)fidb->bytests.llCur/(float)fidb->bytests.lMax)*100;
	else
		almsts.llLoadVal = 0;

    time(&almsts.tWhen);

    slen	= sizeof(st_almsts);

    pstSndSub	= (pst_MsgQSub)&pstSndMsg->llMType;
    pstSndSub->usType	= DEF_SYS;
    pstSndSub->usSvcID	= SID_STATUS;
    pstSndSub->usMsgID	= MID_CONSOL;

    util_makenid(SEQ_PROC_ALMD, &pstSndMsg->llNID);
    pstSndMsg->ucNTAMID	= 0;
    pstSndMsg->ucNTAFID	= almsts.ucSysNo;
    pstSndMsg->ucProID	= SEQ_PROC_CHSMD;
    pstSndMsg->llIndex	= dIndex;
    dIndex++;

    pstSndMsg->dMsgQID	= 0;
    pstSndMsg->usBodyLen	= slen + NTAFT_HEADER_LEN;
    pstSndMsg->usRetCode	= 0;

    memset(pstSndMsg->szBody, 0x00, MNG_PKT_HEAD_SIZE);
    memcpy(&pstSndMsg->szBody[NTAFT_HEADER_LEN], &almsts, pstSndMsg->usBodyLen);

	if( (dRet = dMsgsnd(SEQ_PROC_FSTAT, pNODE)) < 0 ){
		log_print(LOGN_CRI, LH" FAILED IN dMsgsnd(FSTAT)",__FILE__,__LINE__,__FUNCTION__);
		exit(-1);
	}
}

int dCheckCHSMDStatus()
{
    int         fd;
    time_t      tNow;
    char        szProcName[32];

	if( fidb->mpswinfo[0].when< 0 ){
		log_print( LOGN_DEBUG,"CHSMD NORMAL TERMINATED");
		return 1;
	}

    if( fidb->mpswinfo[0].pid == 0 ) {
        log_print( LOGN_INFO, "CHSMD DEAD STATUS" );

        return 1;
    }

    sprintf( szProcName, "/proc/%lld", fidb->mpswinfo[0].pid );

    fd = open( szProcName, O_RDONLY );
    if( fd < 0 ) {
        log_print( LOGN_CRI, "CHSMD DEAD" );

        time( &tNow );

        fidb->mpsw[0] = CRITICAL;
        fidb->mpswinfo[0].pid = 0;
        fidb->mpswinfo[0].when = tNow;

        SendCOND(LOCTYPE_PROCESS, INVOKETYPE_USERPROC, 0, CRITICAL, NORMAL);
    }
    else {
        log_print( LOGN_INFO, "CHSMD ALIVE" );

        close( fd );
    }

    return 1;
}

int dMakeNTAFMngPktHdr( st_MngHead *stPkt, unsigned char SysNo )
{
    /* NTAF STATUS */
    stPkt->llMagicNumber = MAGIC_NUMBER;
    stPkt->usTotLen = MNG_PKT_HEAD_SIZE + sizeof(st_NTAF);
    stPkt->usBodyLen = sizeof(st_NTAF);
    stPkt->ucNTAMID = 0;
    stPkt->ucNTAFID= SysNo;
    stPkt->ucSysNo = SysNo;
    stPkt->ucSvcID = SID_STATUS;
    stPkt->ucMsgID = MID_ALARM;

    return 1;
}

/******************************************************************************
 * SEND MESSAGE FUNCTION
******************************************************************************/
int Send_ALMD(void)
{
	unsigned char   *pNODE;
	pst_MsgQSub		pstSndSub;
	pst_MsgQ        pstSndMsg;
	pst_MngHead		pstPkt;
	int				dRet, dMsgLen;
	time_t			tNow;

	if( (dRet = dGetNode(&pNODE, &pstSndMsg)) < 0 ){
		log_print(LOGN_CRI,LH" FAILED IN dGetNode(ALMD), errno=%d:%s",
				__FILE__,__LINE__,__FUNCTION__,errno,strerror(errno));
		return -1;
	}

	pstSndSub	= (pst_MsgQSub)&pstSndMsg->llMType;
	pstSndSub->usType	= DEF_SYS;
	pstSndSub->usSvcID	= SID_STATUS;
	pstSndSub->usMsgID	= MID_ALARM;

	util_makenid(SEQ_PROC_ALMD, &pstSndMsg->llNID);
	pstSndMsg->ucNTAMID	= 0;
	pstSndMsg->ucNTAFID	= gucSysNo;
	pstSndMsg->ucProID	= SEQ_PROC_ALMD;
	pstSndMsg->llIndex	= dIndex;
	dIndex++;

	pstSndMsg->dMsgQID	= dIFQid;

	/*
	* Socket 공간을 띄워놓고 메세지를 넣어야 함.
	* NTAF의 경우에는 MNG_PKT_HEAD_SIZE만큼의 공간도 띄워야 함.
	*/
	pstSndMsg->usBodyLen	= sizeof(st_NTAF) + NTAFT_HEADER_LEN + MNG_PKT_HEAD_SIZE;
	pstSndMsg->usRetCode	= 0;

	/*
	* 현재 시간을 설정하여 NTAM으로 데이타 전송
	*/
	tNow			= time(&tNow);
	fidb->tUpTime	= tNow;

	pstPkt			= (pst_MngHead)&pstSndMsg->szBody[NTAFT_HEADER_LEN];
	dMakeNTAFMngPktHdr(pstPkt, pstSndMsg->ucNTAFID);

	memcpy(&pstSndMsg->szBody[MNG_PKT_HEAD_SIZE+NTAFT_HEADER_LEN], fidb, sizeof(st_NTAF));

	dMsgLen = sizeof(st_MsgQ) - MAX_MSGBODY_SIZE + pstSndMsg->usBodyLen;

	log_print(LOGN_DEBUG, "[SEND ALMD MESSAGE] TAFNO[%d] BODYLEN:[%d] MSGLEN:[%d] CPU:[%lld][%lld]",
						pstSndMsg->ucNTAFID,pstSndMsg->usBodyLen, dMsgLen,
						fidb->cpusts.llCur, fidb->cpusts.lMax);

	if( (dRet = dMsgsnd(SEQ_PROC_ALMD, pNODE)) < 0 ){
		log_print(LOGN_CRI, LH" FAILED IN dMsgsnd(FSTAT)",__FILE__,__LINE__,__FUNCTION__);
		return -1;
	}

	return 1;
}

/*******************************************************************************
 *
 *******************************************************************************/
int dIsRecvedMessage(st_MsgQ *pstMsg)
{
	int dRet;

	if( (dRet = dMsgrcv(&pstMsg)) < 0 ){
		if( dRet != -1 ){
			log_print(LOGN_CRI, LH" FAILED IN dMsgrcv(ALMD)",__FILE__,__LINE__,__FUNCTION__);
		}   
		return -2;
	}       
	return 1;
}

/*******************************************************************************
 *
*******************************************************************************/
void MakeNtafStat(int dIndex, time_t tCheck, st_NtafStatList *stStat)
{
	stStat->ucTAFID		= (unsigned short)gpGenInfo->IPAFID;
	stStat->KeyTime		= tCheck*300;

	if(gpGenInfo->ThruStat[dIndex].KeyTime == tCheck)		/*	Captured data	*/
	{
		stStat->ThruStat.uiFrames 	= gpGenInfo->ThruStat[dIndex].uiFrames;
		stStat->ThruStat.ulBytes	= gpGenInfo->ThruStat[dIndex].ulBytes;
	}
	else
		memset(&stStat->ThruStat, 0x00, sizeof(st_NtafStat));
	log_print(LOGN_DEBUG, "ThruStat [%10lu:%10lu] : [%10u] [%10lu]", gpGenInfo->ThruStat[dIndex].KeyTime, tCheck, stStat->ThruStat.uiFrames, stStat->ThruStat.ulBytes);

	if(gpGenInfo->TotStat[dIndex].KeyTime == tCheck)			/*	Analyzed data	*/
	{
		stStat->TotStat.uiFrames 		= gpGenInfo->TotStat[dIndex].uiFrames;
		stStat->TotStat.ulBytes 		= gpGenInfo->TotStat[dIndex].ulBytes;
	}
	else
		memset(&stStat->TotStat, 0x00, sizeof(st_NtafStat));
	log_print(LOGN_DEBUG, "TOTStat  [%10lu:%10lu] : [%10u] [%10lu]", gpGenInfo->TotStat[dIndex].KeyTime, tCheck, stStat->TotStat.uiFrames, stStat->TotStat.ulBytes);

	if(gpGenInfo->IPStat[dIndex].KeyTime == tCheck)			/*	IP protocol data	*/
	{
		stStat->IPStat.uiFrames 	= gpGenInfo->IPStat[dIndex].uiFrames;
		stStat->IPStat.ulBytes 		= gpGenInfo->IPStat[dIndex].ulBytes;
	}
	else
		memset(&stStat->IPStat, 0x00, sizeof(st_NtafStat));
	log_print(LOGN_DEBUG, "IPStat   [%10lu:%10lu] : [%10u] [%10lu]", gpGenInfo->IPStat[dIndex].KeyTime, tCheck, stStat->IPStat.uiFrames, stStat->IPStat.ulBytes);

	if(gpGenInfo->UDPStat[dIndex].KeyTime == tCheck)			/*	UDP protocol data	*/
	{
		stStat->UDPStat.uiFrames      = gpGenInfo->UDPStat[dIndex].uiFrames;
		stStat->UDPStat.ulBytes       = gpGenInfo->UDPStat[dIndex].ulBytes;
	}
	else
		memset(&stStat->UDPStat, 0x00, sizeof(st_NtafStat));
	log_print(LOGN_DEBUG, "UDPStat  [%10lu:%10lu] : [%10u] [%10lu]", gpGenInfo->UDPStat[dIndex].KeyTime, tCheck, stStat->UDPStat.uiFrames, stStat->UDPStat.ulBytes);

	if(gpGenInfo->TCPStat[dIndex].KeyTime == tCheck)			/*	TCP protocol data	*/
	{
		stStat->TCPStat.uiFrames	= gpGenInfo->TCPStat[dIndex].uiFrames;
		stStat->TCPStat.ulBytes   	= gpGenInfo->TCPStat[dIndex].ulBytes;
	}
	else
		memset(&stStat->TCPStat, 0x00, sizeof(st_NtafStat));
	log_print(LOGN_DEBUG, "TCPStat  [%10lu:%10lu] : [%10u] [%10lu]", gpGenInfo->TCPStat[dIndex].KeyTime, tCheck, stStat->TCPStat.uiFrames, stStat->TCPStat.ulBytes);

	if(gpGenInfo->SCTPStat[dIndex].KeyTime == tCheck)		/*	SCTP protocol data	*/
	{
		stStat->SCTPStat.uiFrames	= gpGenInfo->SCTPStat[dIndex].uiFrames;
		stStat->SCTPStat.ulBytes   	= gpGenInfo->SCTPStat[dIndex].ulBytes;
	}
	else
		memset(&stStat->SCTPStat, 0x00, sizeof(st_NtafStat));
	log_print(LOGN_DEBUG, "SCTPStat [%10lu:%10lu] : [%10u] [%10lu]", gpGenInfo->SCTPStat[dIndex].KeyTime, tCheck, stStat->SCTPStat.uiFrames, stStat->SCTPStat.ulBytes);

	if(gpGenInfo->ETCStat[dIndex].KeyTime == tCheck)		/*	SCTP protocol data	*/
	{
		stStat->ETCStat.uiFrames	= gpGenInfo->ETCStat[dIndex].uiFrames;
		stStat->ETCStat.ulBytes   	= gpGenInfo->ETCStat[dIndex].ulBytes;
	}
	else
		memset(&stStat->ETCStat, 0x00, sizeof(st_NtafStat));
	log_print(LOGN_DEBUG, "ETCStat  [%10lu:%10lu] : [%10u] [%10lu]", gpGenInfo->ETCStat[dIndex].KeyTime, tCheck, stStat->ETCStat.uiFrames, stStat->ETCStat.ulBytes);

	if(gpGenInfo->IPError[dIndex].KeyTime == tCheck)			/*	IP Protocol error data	*/
	{
		stStat->IPError.uiFrames      = gpGenInfo->IPError[dIndex].uiFrames;
		stStat->IPError.ulBytes       = gpGenInfo->IPError[dIndex].ulBytes;
	}
	else
		memset(&stStat->IPError, 0x00, sizeof(st_NtafStat));
	log_print(LOGN_DEBUG, "IPError  [%10lu:%10lu] : [%10u] [%10lu]", gpGenInfo->IPError[dIndex].KeyTime, tCheck, stStat->IPError.uiFrames, stStat->IPError.ulBytes);

	if(gpGenInfo->UTCPError[dIndex].KeyTime == tCheck)		/*	TCP/UDP Protocol error data	*/
	{
		stStat->UTCPError.uiFrames      = gpGenInfo->UTCPError[dIndex].uiFrames;
		stStat->UTCPError.ulBytes       = gpGenInfo->UTCPError[dIndex].ulBytes;
	}
	else
 		memset(&stStat->UTCPError, 0x00, sizeof(st_NtafStat));
 	log_print(LOGN_DEBUG, "UTCPError[%10lu:%10lu] : [%10u] [%10lu]", gpGenInfo->UTCPError[dIndex].KeyTime, tCheck, stStat->UTCPError.uiFrames, stStat->UTCPError.ulBytes);

	if(gpGenInfo->FailData[dIndex].KeyTime == tCheck)		/*	Analyzing Failed data	*/
	{
		stStat->FailData.uiFrames      = gpGenInfo->FailData[dIndex].uiFrames;
		stStat->FailData.ulBytes       = gpGenInfo->FailData[dIndex].ulBytes;
	}
	else
		memset(&stStat->FailData, 0x00, sizeof(st_NtafStat));
 	log_print(LOGN_DEBUG, "FailData [%10lu:%10lu] : [%10u] [%10lu]", gpGenInfo->FailData[dIndex].KeyTime, tCheck, stStat->FailData.uiFrames, stStat->FailData.ulBytes);

	if(gpGenInfo->FilterOut[dIndex].KeyTime == tCheck)		/*	Not filtered data	*/
	{
		stStat->FilterOut.uiFrames      = gpGenInfo->FilterOut[dIndex].uiFrames;
		stStat->FilterOut.ulBytes       = gpGenInfo->FilterOut[dIndex].ulBytes;
	}
	else
		memset(&stStat->FilterOut, 0x00, sizeof(st_NtafStat));
 	log_print(LOGN_DEBUG, "FilterOut[%10lu:%10lu] : [%10u] [%10lu]", gpGenInfo->FilterOut[dIndex].KeyTime, tCheck, stStat->FilterOut.uiFrames, stStat->FilterOut.ulBytes);
}

/*******************************************************************************
 *
*******************************************************************************/
void ClearNtafStat(int dIndex)
{
	memset(&gpGenInfo->ThruStat[dIndex], 0x00, sizeof(st_UpDownStat));

	memset(&gpGenInfo->TotStat[dIndex], 0x00, sizeof(st_UpDownStat));
	memset(&gpGenInfo->IPStat[dIndex], 0x00, sizeof(st_UpDownStat));
	memset(&gpGenInfo->UDPStat[dIndex], 0x00, sizeof(st_UpDownStat));
	memset(&gpGenInfo->TCPStat[dIndex], 0x00, sizeof(st_UpDownStat));
	memset(&gpGenInfo->SCTPStat[dIndex], 0x00, sizeof(st_UpDownStat));
	memset(&gpGenInfo->ETCStat[dIndex], 0x00, sizeof(st_UpDownStat));

	memset(&gpGenInfo->IPError[dIndex], 0x00, sizeof(st_UpDownStat));
	memset(&gpGenInfo->UTCPError[dIndex], 0x00, sizeof(st_UpDownStat));
	memset(&gpGenInfo->FailData[dIndex], 0x00, sizeof(st_UpDownStat));
	memset(&gpGenInfo->FilterOut[dIndex], 0x00, sizeof(st_UpDownStat));
}

int Send_STAT(st_NtafStatList *stStat)
{
	unsigned char   *pNODE;
	pst_MsgQSub		pstSndSub;
	pst_MsgQ        pstSndMsg;
	int				dRet, dMsgLen;

	if( (dRet = dGetNode(&pNODE, &pstSndMsg)) < 0 ){
		log_print(LOGN_CRI,LH" FAILED IN dGetNode(QMON), errno=%d:%s",
				__FILE__,__LINE__,__FUNCTION__,errno,strerror(errno));
		return -1;
	}

    pstSndSub			= (pst_MsgQSub)&pstSndMsg->llMType;
    pstSndSub->usType	= DEF_SYS;
    pstSndSub->usSvcID	= SID_STATUS;
    pstSndSub->usMsgID	= MID_TRAFFIC;

    util_makenid(SEQ_PROC_ALMD, &pstSndMsg->llNID);
	pstSndMsg->ucNTAFID	= gpGenInfo->IPAFID;
    pstSndMsg->ucProID	= SEQ_PROC_ALMD;
    pstSndMsg->llIndex	= dIndex;
    pstSndMsg->dMsgQID	= dIFQid;

	/*
		만약 이부분을 사용하게 된다면, 바디 앞단에 st_NTAFTHeader의 크기를 더하고,
		그 값을 넣어줘야 함을 잊으면 안된다.
		st_NTAFHeader 부분들에 대한 값은 CI_SVC에서 채워서 보냅니다.
	*/
    pstSndMsg->usBodyLen	= NTAFT_HEADER_LEN+DEF_NTAFSTAT_LEN;
    pstSndMsg->usRetCode	= 0;

    memcpy(&pstSndMsg->szBody[NTAFT_HEADER_LEN], stStat, DEF_NTAFSTAT_LEN);
    dMsgLen = sizeof(st_MsgQ)-MAX_MSGBODY_SIZE+pstSndMsg->usBodyLen-sizeof(long);

	if( (dRet = dMsgsnd(SEQ_PROC_FSTAT, pNODE)) < 0 ){
		log_print(LOGN_CRI, LH" FAILED IN dMsgsnd(FSTAT)",__FILE__,__LINE__,__FUNCTION__);
		return -1;
	}

    log_print(LOGN_DEBUG, LH" [SEND STAT MESSAGE] BODYLEN[%d] MSGLEN[%d]", LT,
		pstSndMsg->usBodyLen, dMsgLen);

	return 0;
}

int dComputeBytes(void)
{
	long		ldCriticalValue, ldMajorValue, ldMinorValue;
	long long	llCurVal;
	float		fPercent;

	ldCriticalValue	= keepalive->stBytes.critical;
	ldMajorValue	= keepalive->stBytes.major;
	ldMinorValue	= keepalive->stBytes.minor;

	if(fidb->bytests.lMax != 0)
	{
		llCurVal = ( (float)fidb->bytests.llCur/(float)fidb->bytests.lMax)*100.0;
		fPercent = ( (float)fidb->bytests.llCur/(float)fidb->bytests.lMax)*100.0;

		if(fPercent > 100)
		{
			fidb->bytests.llCur = fidb->bytests.lMax;
			llCurVal = ((float)fidb->bytests.llCur/(float)fidb->bytests.lMax)*100.0;
			fPercent = ((float)fidb->bytests.llCur/(float)fidb->bytests.lMax)*100.0;
		}

		log_print(LOGN_DEBUG, LH" Bytes Load - fPercent[%6.2f] llCur[%lld] lMax[%lld]", LT,
			fPercent, fidb->bytests.llCur, fidb->bytests.lMax);

		if( (ldMinorValue == 0) || (ldMajorValue == 0) || (ldCriticalValue == 0))
		{
			log_print(LOGN_WARN, LH" ldMinorValue[%ld] ldMajorValue[%ld] ldCriticalValue[%ld]", LT,
				ldMinorValue, ldMajorValue, ldCriticalValue);
			fidb->ucStatBytes = NORMAL;
		}
		else
		{
			if(fPercent < ldMinorValue)
			{
				if(fidb->ucStatBytes != NORMAL)
					SendCOND(LOCTYPE_LOAD, INVTYPE_TAF_TRAFFIC, 0, NORMAL, fidb->ucStatBytes);

				fidb->ucStatBytes = NORMAL;
			}
			else if(fPercent < ldMajorValue)
			{
				if(fidb->ucStatBytes != MINOR)
					SendCOND(LOCTYPE_LOAD, INVTYPE_TAF_TRAFFIC, 0, MINOR, fidb->ucStatBytes);

				fidb->ucStatBytes = MINOR;
			}
			else if(fPercent < ldCriticalValue)
			{
				if(fidb->ucStatBytes != MAJOR)
					SendCOND(LOCTYPE_LOAD, INVTYPE_TAF_TRAFFIC, 0, MAJOR, fidb->ucStatBytes);

				fidb->ucStatBytes = MAJOR;
			}
			else
			{
				if(fidb->ucStatBytes != CRITICAL)
					SendCOND(LOCTYPE_LOAD, INVTYPE_TAF_TRAFFIC, 0, CRITICAL, fidb->ucStatBytes);

				fidb->ucStatBytes = CRITICAL;
			}

			log_print(LOGN_DEBUG, LH" ucStatBytes[0x%02X] fPercent[%6.2f] ldMinorValue[%ld] ldMajorValue[%ld] ldCriticalValue[%ld]", LT,
				fidb->ucStatBytes, fPercent, ldMinorValue, ldMajorValue, ldCriticalValue);
		}
	}
	else
	{
		log_print(LOGN_WARN, LH" lMax[%lld] is ZERO llCur[%lld]", LT,
			fidb->bytests.lMax, fidb->bytests.llCur);
	}

	return 0;
}

int dLogMMDBCount()
{
	int		dLog;
	dLog = LOGN_INFO;

	log_print( dLog, "[ SESS] [  OBJ] [  CDR] [ CALL] [  RDR] [   ME] [  KUN] [  ADS] [ MARS]");
	log_print( dLog, "[%5d] [%5d] [%5d] [%5d] [%5d] [%5d] [%5d] [%5d] [%5d]",
					fidb->rsrcload[0], fidb->rsrcload[1], fidb->rsrcload[2],
					fidb->rsrcload[5], 0,
					fidb->rsrcload[6], fidb->rsrcload[7],
					fidb->rsrcload[8], fidb->rsrcload[9] );

	log_print( dLog, "[ MACS] [WICGS] [ VODM] [ VODD] [ VODS] [VODUDP]");
	log_print( dLog, "[%5d] [%5d] [%5d] [%5d] [%5d] [%6d]",
					fidb->rsrcload[10], fidb->rsrcload[11], fidb->rsrcload[12],
					fidb->rsrcload[13], fidb->rsrcload[14], fidb->rsrcload[15] );
	return 0;
}

/*******************************************************************************

	Revision History :

		$Log: almd_main.c,v $
		Revision 1.5  2011/09/07 04:59:33  uamyd
		modified
		
		Revision 1.4  2011/09/05 08:58:54  dcham
		*** empty log message ***
		
		Revision 1.3  2011/09/05 04:44:39  dcham
		*** empty log message ***
		
		Revision 1.2  2011/09/01 07:17:08  hhbaek
		*** empty log message ***
		
		Revision 1.1  2011/08/29 09:55:32  dcham
		*** empty log message ***
		
		Revision 1.8  2011/08/25 06:13:17  dcham
		*** empty log message ***
		
		Revision 1.7  2011/08/22 00:15:28  dcham
		*** empty log message ***
		
		Revision 1.6  2011/08/20 08:30:27  dcham
		*** empty log message ***
		
		Revision 1.5  2011/08/20 08:02:18  dcham
		*** empty log message ***
		
		Revision 1.4  2011/08/20 07:39:30  dcham
		*** empty log message ***
		
		Revision 1.3  2011/08/09 07:15:25  dcham
		*** empty log message ***
		
		Revision 1.2  2011/08/03 08:19:14  dcham
		*** empty log message ***
		
		Revision 1.1  2011/08/03 00:23:52  dcham
		*** empty log message ***
		
		Revision 1.35  2011/01/11 04:09:05  uamyd
		modified
		
		Revision 1.1.1.1  2010/08/23 01:13:02  uamyd
		DQMS With TOTMON, 2nd-import
		
		Revision 1.34  2010/02/24 05:48:18  hjpark
		no message
		
		Revision 1.33  2009/10/27 18:55:24  pkg
		*** empty log message ***

		Revision 1.32  2009/10/27 17:02:42  pkg
		*** empty log message ***

		Revision 1.31  2009/10/21 08:39:57  pkg
		실시간 traffic 감시 항목 추가에 따른 명령어 처리 및 메시지 처리 수정

		Revision 1.30  2009/09/13 11:01:25  pkg
		NIFO 감시 추가

		Revision 1.29  2009/08/18 14:38:32  pkg
		*** empty log message ***

		Revision 1.28  2009/07/14 12:04:09  hjpark
		gpGenInfo에 ETCStat 추가

		Revision 1.27  2009/07/01 07:52:18  hjpark
		no message

		Revision 1.26  2009/06/28 18:24:26  hjpark
		no message

		Revision 1.25  2009/06/28 17:26:13  hjpark
		no message

		Revision 1.24  2009/06/28 17:23:19  hjpark
		no message

		Revision 1.23  2009/06/28 17:16:03  hjpark
		no message

		Revision 1.22  2009/06/28 17:12:55  hjpark
		no message

		Revision 1.21  2009/06/28 17:07:23  hjpark
		no message

		Revision 1.20  2009/06/28 17:06:31  hjpark
		no message

		Revision 1.19  2009/06/28 16:51:10  hjpark
		no message

		Revision 1.18  2009/06/28 15:52:16  hjpark
		no message

		Revision 1.17  2009/06/28 13:42:56  hjpark
		no message

		Revision 1.16  2009/06/28 13:19:01  hjpark
		no message

		Revision 1.15  2009/06/27 20:53:55  hjpark
		no message

		Revision 1.14  2009/06/27 20:05:22  hjpark
		no message

		Revision 1.13  2009/06/27 19:51:30  hjpark
		no message

		Revision 1.12  2009/06/27 19:49:48  hjpark
		no message

		Revision 1.11  2009/06/27 19:49:08  hjpark
		no message

		Revision 1.10  2009/06/27 19:44:52  hjpark
		no message

		Revision 1.9  2009/06/17 14:11:12  hjpark
		no message

		Revision 1.8  2009/06/17 13:55:31  hjpark
		no message

		Revision 1.7  2009/06/17 13:42:10  hjpark
		no message

		Revision 1.6  2009/06/10 21:25:17  jsyoon
		*** empty log message ***

		Revision 1.5  2009/06/08 18:28:54  hjpark
		no message

		Revision 1.4  2009/06/08 18:17:19  hjpark
		no message

		Revision 1.3  2009/06/05 05:30:16  jsyoon
		*** empty log message ***

		Revision 1.2  2009/05/27 14:24:48  dqms
		*** empty log message ***

		Revision 1.1.1.1  2009/05/26 02:14:16  dqms
		Init TAF_RPPI

		Revision 1.6  2009-05-21 08:38:57  astone
		no message

		Revision 1.5  2009-05-21 07:42:30  astone
		no message

		Revision 1.4  2009-05-21 07:36:54  astone
		no message

		Revision 1.3  2009-05-21 06:47:12  astone
		no message

		Revision 1.2  2009-05-21 06:44:57  hjpark
		no message

		Revision 1.1.1.1  2009-05-19 10:34:23  hjpark
		DQMS
			-Writer: Han-jin Park
			-Date: 2009.05.19

		Revision 1.5  2008/06/02 06:44:30  uamyd
		*** empty log message ***

		Revision 1.4  2008/05/06 06:41:36  uamyd
		20080506
		인수시험
		적용소스

		Revision 1.3  2008/03/19 13:17:00  uamyd
		after apply 20080319

		Revision 1.2  2008/01/03 06:55:45  dark264sh
		Makefile 수정 - 옵션 변경 (-g3 -Wall), warning 수정, log_print Log Level 수정

		Revision 1.1.1.1  2007/12/27 09:04:48  leich
		WNTAS_ADD

		Revision 1.2  2007/11/29 00:51:47  uamyd
		removed ^M

		Revision 1.1.1.1  2007/10/31 05:12:12  uamyd
		WNTAS so lated initialized

		Revision 1.1  2002/01/30 18:43:15  swdev4
		Initial revision
*******************************************************************************/

