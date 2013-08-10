/*********************************************************
                 ABLEX IPAS Project (IPAF BLOCK)

   Author   : Hwang Woo-Hyoung
   Modefied : Yoon JinSeok
   Section  : IPAS(IPAM) Project
   SCCS ID  : @(#)sessana_msgq.c (V1.0)
   Date     : 03/03/04
   Revision History :
        '04.    03. 03. initial

   Description:

   Copyright (c) ABLEX 2004
*********************************************************/

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
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

#include "utillib.h"
#include "ipaf_svc.h"
#include "ipaf_stat.h"
#include "ipam_ipaf.h"
#include "calltcpstack.h"

#include "mems.h"
#include "nifo.h"

#include "rana.h"
#include "rana_session.h"

/* Definition of New Constants */

/* Declaration of Global Variable */
extern stMEMSINFO 		*pstMEMSINFO;
extern LEG_DATA_SUM		*gpstCallInfo[DEF_STAT_SET_CNT];
extern LEG_TOT_STAT_t 	*gpstTotStat[DEF_STAT_SET_CNT];
extern PDSN_LIST       	*gpCurPdsn; // CURRENT OPERATION pointer
extern LEG_CALL_DATA    *gpstCallDataPerSec;

/* Declaration of Extern Global Variable */

extern int		dCDRQid;
//extern int		dCDR2Qid;
//extern int		dPTOPANAQid;
extern int		dPTOPCDRQid;
extern int		gCIdx;
extern int		gSIdx;

/* Declaration of Function Definition */

/* DESCIPTION :
 * 	- Call DATA(TPS, CPS, SESSION)를 FIMD로 전송.
 */
void report_CallData2FIMD (void)
{
	GeneralQMsgType txGenQMsg;
	IxpcQMsgType	*txIxpcMsg;
	int 			dTxLen = 0, dMsgLen;
	LEG_CALL_DATA	*ptxData=NULL;
	LEG_CALL_DATA	*pstCallDataPerSec = gpstCallDataPerSec;
	LEG_DATA_SUM	*pCallData = gpstCallInfo[gCIdx];
	int i;

	txGenQMsg.mtype = MTYPE_STATUS_REPORT;
	txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
	memset ((void*)&txIxpcMsg->head, 0, sizeof(txIxpcMsg->head));
	txIxpcMsg->head.msgId = MSGID_CALL_REPORT;
	strcpy (txIxpcMsg->head.srcSysName, mySysName);
	strcpy (txIxpcMsg->head.srcAppName, myAppName);
	strcpy (txIxpcMsg->head.dstSysName, "DSCM");
	strcpy (txIxpcMsg->head.dstAppName, "FIMD");

	dMsgLen = sizeof(LEG_CALL_DATA);
	ptxData = (LEG_CALL_DATA *)txIxpcMsg->body;
	memset(ptxData, 0x00, sizeof(LEG_CALL_DATA));


	// CPS DATA
	for( i = 0; i < MAX_RLEG_CNT; i++ )
	{
		ptxData->cps.uiLogOnSumCps  += pCallData->cps[i].uiLogOnSumCps;
		ptxData->cps.uiLogOutSumCps += pCallData->cps[i].uiLogOutSumCps;
	}
	pCallData->cps[i].uiLogOnSumCps  = ptxData->cps.uiLogOnSumCps;
	pCallData->cps[i].uiLogOutSumCps = ptxData->cps.uiLogOutSumCps;

	//과부하 확인을 위한 이전 5sec 동안의 cps 값을 저장
	pstCallDataPerSec->cps.uiLogOnSumCps  = pCallData->cps[i].uiLogOnSumCps;
	pstCallDataPerSec->cps.uiLogOutSumCps = pCallData->cps[i].uiLogOutSumCps;

	// TPS DATA
	ptxData->tps = pCallData->uiTPS;

	//dis-call-info 명령어 요청시 보여줄 값. 이전 5sec 동안의 tps 값을 저장
	pstCallDataPerSec->tps = ptxData->tps;

	// LOGON SESSION DATA
	ptxData->sess.uiAmount = (gpShmem) ? gpShmem->rad_sess : 0; 

	// SEND DATA
	txIxpcMsg->head.bodyLen = dMsgLen;
	dTxLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;

	if( msgsnd(dIxpcQid, (void*)&txGenQMsg, dTxLen, IPC_NOWAIT) < 0 ) {
		dAppLog(LOG_CRI, "[FAIL] CPS MSGQ SEND ERR=%d(%s)", errno, strerror(errno));
	}

#ifdef PRT_CALL_CNT
	PrintCallData(pCallData, ptxData->sess.uiAmount);
#endif
	InitCallData(pCallData);
}

void PrintCallData(const LEG_DATA_SUM *pCallData, const unsigned int uiAmount)
{
	int i;
	for( i = 0; i < MAX_RLEG_CNT; i++ )
	{
		dAppLog(LOG_DEBUG, "  >> CPS RLEG(%d) : LOGON=%3u  LOGOUT=%3u"
				, i, pCallData->cps[i].uiLogOnSumCps/CALL_UNIT, pCallData->cps[i].uiLogOutSumCps/CALL_UNIT);
	}

	dAppLog(LOG_CRI, ">>>> CPS : LOGON=%3u LOGOUT=%3u  TPS=%u  SESS=%u(%u:%u:%u)"
			, pCallData->cps[i].uiLogOnSumCps/CALL_UNIT, pCallData->cps[i].uiLogOutSumCps/CALL_UNIT
			, pCallData->uiTPS
			, uiAmount, gpstCallDataPerSec->cps.uiLogOnSumCps, gpstCallDataPerSec->cps.uiLogOutSumCps, gpstCallDataPerSec->tps);
}

void InitCallData(LEG_DATA_SUM *pCallData)
{
	int i;

	for( i = 0; i <= MAX_RLEG_CNT; i++ )
	{
		pCallData->cps[i].uiLogOnSumCps = 0;
		pCallData->cps[i].uiLogOutSumCps = 0;
	}
	pCallData->uiTPS = 0;
}

void report_StatData2STMD(void)
{
	GeneralQMsgType txGenQMsg;
	IxpcQMsgType	*txIxpcMsg;
	int i, j, dTxLen=0, dMsgLen;

	LEG_TOT_STAT_t	*pTotStat = gpstTotStat[gSIdx];

	dAppLog(LOG_CRI, "### STAT]  IDX=%d PDSN=%d", gSIdx, pTotStat->stAcct.uiPDSN_Cnt);
	for(i = 0; i < gpCurPdsn->uiCount; i++ ) {
		pTotStat->stAcct.staPDSN_Stat[i].uiPDSN_IP = gpCurPdsn->uiAddr[i];
	}

	for(i = 0; i < MAX_RLEG_CNT; i++ ) {
		for(j = 0; j < LOG_MODE_CNT; j++) {
			pTotStat->stLogon[i][j].uiSMIndex = i;
			pTotStat->stLogon[i][j].uiLogMode = j;
		}
	}
	sprintf(pTotStat->szSysName, "%s", mySysName);

	txGenQMsg.mtype = MTYPE_STATISTICS_REPORT;

	txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
	memset ((void*)&txIxpcMsg->head, 0, sizeof(txIxpcMsg->head));

	txIxpcMsg->head.msgId = MSGID_LEG_STATISTICS_REPORT;

	memcpy(txIxpcMsg->body, pTotStat, sizeof(LEG_TOT_STAT_t));
	strcpy (txIxpcMsg->head.srcSysName, mySysName);
	strcpy (txIxpcMsg->head.srcAppName, myAppName);
	strcpy (txIxpcMsg->head.dstSysName, "DSCM");
	strcpy (txIxpcMsg->head.dstAppName, "STMD");

	// 수집된 통계 데이터를 복사한다.
	dMsgLen = sizeof(LEG_TOT_STAT_t);
	txIxpcMsg->head.bodyLen = dMsgLen;
	dTxLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;

	if( msgsnd(dIxpcQid, (void*)&txGenQMsg, dTxLen, IPC_NOWAIT) < 0 ) {
		dAppLog(LOG_CRI, "[FAIL] TOT-STAT MSGQ SEND ERR=%d(%s)", errno, strerror(errno));
	}
	else {
		dAppLog(LOG_WARN, "[SUCC] SEND TO LEG-TOT-STAT MSGQ(len:%d)", dTxLen);
	}

#ifdef DEBUGLOG
	PrintStat(pTotStat);
#endif
	InitStat(pTotStat);
}

void PrintStat(const LEG_TOT_STAT_t *pTotStat)
{
	int i, j;	
	LEG_STAT_t		*pLeg = (LEG_STAT_t*) &pTotStat->stAcct;
	LOGON_STAT_t	*pLogon;

	dAppLog(LOG_CRI, ">>> ACCOUNT STAT [SYS=%s PDSN CNT=%u", pTotStat->szSysName, pLeg->uiPDSN_Cnt);
#ifdef	DPRT_STATISTIC
	for( i = 0; i < pLeg->uiPDSN_Cnt; i++ )
	{
		dAppLog(LOG_CRI, ">>> [%u:%u] [%u %u %u %u] [%u %u %u %u"
				, pLeg->staPDSN_Stat[i].uiPDSN_IP, pLeg->staPDSN_Stat[i].uiPDSN_RecvCnt
				, pLeg->staPDSN_Stat[i].uiPDSN_StartCnt, pLeg->staPDSN_Stat[i].uiPDSN_InterimCnt
				, pLeg->staPDSN_Stat[i].uiPDSN_StopCnt,  pLeg->staPDSN_Stat[i].uiPDSN_DiscReqCnt
				, pLeg->staPDSN_Stat[i].uiLogOn_StartCnt, pLeg->staPDSN_Stat[i].uiLogOn_InterimCnt
				, pLeg->staPDSN_Stat[i].uiLogOn_StopCnt, pLeg->staPDSN_Stat[i].uiLogOn_DiscReqCnt);
	}
#endif

	dAppLog(LOG_DEBUG, ">>> LOGON STAT [RLEG CNT=%d", MAX_RLEG_CNT);
	for( i = 0; i < MAX_RLEG_CNT; i++ )
	{
		for( j = 0; j < LOG_MODE_CNT; j++ )
		{
			pLogon = (LOGON_STAT_t*)&pTotStat->stLogon[i][j];
#ifdef	DPRT_STATISTIC
			dAppLog(LOG_CRI, ">>> [%u:%u] [%u:%u:%u"
					, pLogon->uiSMIndex, pLogon->uiLogMode
					, pLogon->uiLogOn_Request, pLogon->uiLogOn_Success, pLogon->uiLogOn_Fail);
#endif
		}
	}
}

void InitStat(LEG_TOT_STAT_t *pTotStat)
{
	int i, j;
	LEG_STAT_t		*pLeg = &pTotStat->stAcct;
	LOGON_STAT_t	*pLogon;

	for( i = 0; i < pLeg->uiPDSN_Cnt; i++ )
	{
		pLeg->staPDSN_Stat[i].uiPDSN_IP = 
		pLeg->staPDSN_Stat[i].uiPDSN_RecvCnt = 
		pLeg->staPDSN_Stat[i].uiPDSN_StartCnt = 
		pLeg->staPDSN_Stat[i].uiPDSN_InterimCnt = 
		pLeg->staPDSN_Stat[i].uiPDSN_StopCnt = 
		pLeg->staPDSN_Stat[i].uiPDSN_DiscReqCnt = 
		pLeg->staPDSN_Stat[i].uiLogOn_StartCnt = 
		pLeg->staPDSN_Stat[i].uiLogOn_InterimCnt = 
		pLeg->staPDSN_Stat[i].uiLogOn_StopCnt = 
		pLeg->staPDSN_Stat[i].uiLogOn_DiscReqCnt = 0;
	}

	for( i = 0; i < MAX_RLEG_CNT; i++ )
	{
		for( j = 0; j < LOG_MODE_CNT; j++ )
		{
			pLogon = &pTotStat->stLogon[i][j];
			pLogon->uiSMIndex = pLogon->uiLogMode = 
			pLogon->uiLogOn_Request = pLogon->uiLogOn_Success = 
			pLogon->uiLogOn_Fail = pLogon->uiLogOn_Reason1 = 
			pLogon->uiLogOn_Reason2 = pLogon->uiLogOn_Reason3 = 
			pLogon->uiLogOn_Reason4 = pLogon->uiLogOn_APIReqErr = pLogon->uiLogOn_APITimeout = 0;
			memset(pLogon->uiLogOn_HBIT, 0, sizeof(unsigned int)*MAX_HBIT_STAT_CNT);
		}
	}
}

