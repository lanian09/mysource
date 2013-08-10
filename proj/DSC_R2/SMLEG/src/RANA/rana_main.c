/**A.1*	FILE INCLUSION ********************************************************/
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <eth_capd.h>
#include <ipaf_define.h>
#include <ipaf_svc.h>
#include <ipaf_names.h>
#include <ipaf_shm.h>
#include <ipaf_sem.h>
#include <ipam_ipaf.h>
#include <init_shm.h>
#include <shmutil.h>

#include <Num_Proto.h>
#include <Analyze_Ext_Abs.h>
#include <Ethernet_header.h>
#include <define.h>

#include <utillib.h>
#include <Errcode.h>

#include "mems.h"
#include "nifo.h"
#include "rana_mmc_hld.h"
#include "rana.h"
#include "rana_session.h"

#include "comm_util.h"
#include "comm_trace.h"
#include "comm_session.h"
#include "hash_pdsn.h"
#include "comm_msgtypes.h"

/* Function Time Check */
#include "func_time_check.h"

/**B.1*	DEFINITION OF NEW CONSTANTS *******************************************/
#define	UDP_HEADER_LEN	8

/**B.2*	DEFINITION OF NEW TYPE ************************************************/
#define TOUSHORT(x) (USHORT)(*(x)<<8|*(x+1))
#define TOULONG(x)  (ULONG)(*(x)<<24|*(x+1)<<16|*(x+2)<<8|*(x+3))

/**C.1*	DECLARATION OF VARIABLES **********************************************/

char 	vERSION[7] = "R2.0.0";		// R1.0.0 -> R2.0.0
char 	sysLable[COMM_MAX_NAME_LEN], mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN];

// TRACE_INFO.conf 구조체 
st_SESSInfo			*gpTrcList[DEF_SET_CNT];
st_SESSInfo			*gpCurTrc; 		// CURRENT OPERATION pointer
// PDSN.conf 구조체  
PDSN_LIST        	*gpPdsnList[DEF_SET_CNT];
PDSN_LIST        	*gpCurPdsn; 	// CURRENT OPERATION pointer
// PDSN HASH
stHASHOINFO         *gpPdsnHash[DEF_SET_CNT];
stHASHOINFO         *gpCurPdsnHash; // CURRENT OPERATION pointer
// RULESET_LIST.conf 구조체 
ST_PBTABLE_LIST  	*gpRsetList[DEF_SET_CNT];
ST_PBTABLE_LIST  	*gpCurRsetList;
// RULESET_USED.conf 구조체 
RULESET_USED_FLAG	*gpRSetUsedList[DEF_SET_CNT];
RULESET_USED_FLAG	*gpCurRSetUsed;
// CALL_OVER_CTRL.conf 구조체 
CPS_OVLD_CTRL		*gpCpsOvldCtrl[DEF_SET_CNT];
CPS_OVLD_CTRL		*gpCurCpsCtrl;
// TIMEOUT.conf 구조체 
MPTimer            	*gpMPTimer[DEF_SET_CNT];
MPTimer            	*gpCurMPTimer;
// CPS 집계 구조체 
LEG_DATA_SUM		*gpstCallInfo[DEF_STAT_SET_CNT];
// ACCOUT, LOGON 통계 구조체 
LEG_TOT_STAT_t		*gpstTotStat[DEF_STAT_SET_CNT];

// CPS & TPS per 5sec save 구조체
LEG_CALL_DATA       *gpstCallDataPerSec;

st_NOTI				gstIdx;
st_NOTI				*gpIdx = &gstIdx;	// NOTIFICATION DATA ACTIVE TABLE INDEX
int					gCIdx = 0;			// CALL DATA ACTIVE TABLE INDEX
int					gSIdx = 0;			// STATISTIC DATA ACTIVE TABLE INDEX

SFM_SysCommMsgType	*loc_sadb;
stMEMSINFO 			*pstMEMSINFO;

// SM 시스템 정보 구조체 
SM_INFO          	gSCM[MAX_SM_NUM];


/////////////////////////////////////////////////////////////////////////////////////

/* Function Time Check */
st_FuncTimeCheckList    stFuncTimeCheckList;
st_FuncTimeCheckList    *pFUNC = &stFuncTimeCheckList;

int     			JiSTOPFlag;
int     			FinishFlag;

/* FOR LOG */
char				szLogBuf[1024];
unsigned char		szTempIP[5];
unsigned char		szTempIP2[5];

int					g_CheckIndex = 0;

/* MSG QID */
int					dTRCDRQid[MAX_RLEG_CNT];
int 				dIxpcQid;
int 				dMyQid;

/**D.1*  DEFINITION OF FUNCTIONS  *********************************************/

int 	dAppLog(int dIndex, char *fmt, ...);
void 	FinishProgram();
void 	SetUpSignal();
UINT 	CVT_UINT( UINT value );
INT64 	CVT_INT64( INT64 value );
INT		CVT_INT( INT value );
inline int 		dAnalyze_RADIUS( PUCHAR pBuf, pst_RADInfo pRADInfo, pst_IPTCPHeader pstIPTCPHeader );
inline int		dAnalyze_RADIUS_ATTRIB( PUCHAR pBuf, USHORT usDataLen, pst_RADInfo pRADInfo );
int    	keepalivelib_init(char *processName);
void   	keepalivelib_increase();
int 	conflib_getNthTokenInFileSection( char *fname, char *section, char *keyword, int n, char *string );

int 	dReadFLTIDXFile(void);
int 	dCheck_PDSN_IP2(UINT addr);
int 	dINIT_RANA_IPCS(void);
int 	InitSHM_GENINFO(void);
int 	InitSHM_LOC_SADB(void);
int 	InitSHM_LEG_STAT(void);
int 	InitSHM_CALL_DATA(void);
int 	InitSHM_LEG_SESS(void);
int 	InitSHM_TRACE(void);
int 	InitSHM_PDSN_LIST(void);
int 	InitSHM_PDSN_HASH(void);
int 	InitSHM_RSET_LIST(void);
int 	InitSHM_RSET_USED(void);
int 	InitSHM_CPS_OVLD(void);
int 	InitSHM_TIMER(void);
void 	dSetCurTrace(NOTIFY_SIG *pNOTISIG);
int 	dSetCurPdsn(NOTIFY_SIG *pNOTISIG);
void 	dSetCurRSetList(NOTIFY_SIG *pNOTISIG);
void 	dSetCurRSetUsed(NOTIFY_SIG *pNOTISIG);
void 	dSetCurCPSOvld(NOTIFY_SIG *pNOTISIG);
void 	dSetCurTIMEOUT(NOTIFY_SIG *pNOTISIG);
void 	REConstructStat(NOTIFY_SIG *pNOTISIG);
void 	sce_log_out(NOTIFY_SIG *pNOTISIG);
int 	dSwitchRANAMsg(UCHAR *pNodeData, st_IPTCPHeader *pstIPTCPHeader);
void 	WriteAcctCnt(int pidx, int dMsgID, int fail);

/**D.2*  DEFINITION OF FUNCTIONS  *********************************************/
//extern int 		set_version(int prc_idx, char *ver);	// 040127,poopee
extern int 	Init_msgq( key_t q_key );
extern int 	check_my_run_status (char *procname);
extern int 	CheckTrace (SUBS_INFO *pSubsInfo);
extern int 	RouteRLEG(int dIMSI, int chId);

/*******************************************************************************

*******************************************************************************/
void checkSMConnect(void)
{
	int i, isConn = 0;	
	LEG_SM_CONN	*pSmConn = loc_sadb->smConn;
	for( i = 0; i < MAX_RLEG_CNT; i++ )
	{
		if( pSmConn[i].dConn != CONNECTED ) {
			pSmConn[MAX_RLEG_CNT].dConn = DISCONNECTED;					
			isConn = -1;
			break;
		}
	}

	if( isConn  == 0 )
		pSmConn[MAX_RLEG_CNT].dConn = CONNECTED;

}

/*******************************************************************************

*******************************************************************************/
int main (void)
{
	int			dRet, check_Index;
	time_t 		now, tChkTime = 0, tChkLastTime = 0, tStatTime = 0, tCallRptTime = 0;
	struct tm	tmLast, tmStat, tmCallLast, tmCall;

	OFFSET 				offset, sub_offset;
	UCHAR 				*pNode, *pCurrNode, *pNextNode, *pNodeData;
	st_IPTCPHeader 		*pstIPTCPHeader;
	NOTIFY_SIG			*pNOTISIG;

    /* INITIALIZE LOG */
    dRet = Init_logdebug( getpid(), "RANA", "/DSC/APPLOG" );
	dAppLog(LOG_CRI,"RANA %s %d][PROCESS INITIALIZING ...", vERSION, getpid());

	/* INITIALIZE SIGNAL */
    SetUpSignal();

    /* INITIALIZE CONFIGURATION */
    dRet = dINIT_RANA_IPCS();
    if( dRet < 0 ) {
        exit(1);
    }
#if 1
	if((check_Index = check_my_run_status("RANA")) < 0)
		exit(0);
#endif
    /* INITIALIZE S/W VERSION */
	if((dRet=set_version(SEQ_PROC_RANA, vERSION)) < 0) {
		dAppLog(LOG_CRI, "SET_VERSION ERROR(RET=%d,IDX=%d,VER=%s)", dRet, SEQ_PROC_RANA, vERSION);
	}

    /* INITIALIZE KEEPALIVE */
	if (keepalivelib_init("RANA") < 0)   
        exit(0);
	
	tChkLastTime = now = time(0);

	tStatTime = (now / DEF_STAT_UNIT) * DEF_STAT_UNIT;
	localtime_r((time_t*)&tStatTime, &tmLast);
	gSIdx = tmLast.tm_min % DEF_STAT_SET_CNT;

	tCallRptTime = (now / CALL_UNIT) * CALL_UNIT;
	localtime_r((time_t*)&tCallRptTime, &tmCallLast);
	gCIdx = tmCallLast.tm_sec % DEF_STAT_SET_CNT;

	dAppLog(LOG_CRI, " * PDSN_CNT          : IDX[0]=%d IDX[1]=%d"
			, gpstTotStat[0]->stAcct.uiPDSN_Cnt, gpstTotStat[1]->stAcct.uiPDSN_Cnt);

	dAppLog(LOG_CRI,"RANA %s %d] [PROCESS INIT SUCCESS", vERSION, getpid());
	dAppLog(LOG_CRI,"RANA %s %d] [PROCESS STARTED", vERSION, getpid());
    
	while ( JiSTOPFlag )
	{
		keepalivelib_increase();

		tChkTime = now = time(0);
		if( tChkTime != tChkLastTime ) {
			checkSMConnect();
			timerN_invoke(pTimer_rad);
			tChkLastTime = tChkTime;
#ifdef PRT_SESS_CNT_LV1
			dAppLog(LOG_CRI, ">>> TOT] SS=%u TM=%u", gpShmem->rad_sess, pTimer_rad->uiNodeCnt);
#endif
		}

		// CALL DATA TRANSMISSION (RANA to FIMD)
		tCallRptTime = (now / CALL_UNIT) * CALL_UNIT;
		localtime_r((time_t*)&tCallRptTime, &tmCall);
		if( tmCallLast.tm_sec != tmCall.tm_sec ) {
			// 과거 5sec 동안의 cps 값을 저장한다.
			report_CallData2FIMD();
			gCIdx = tmCall.tm_sec % DEF_STAT_SET_CNT;
			tmCallLast = tmCall;
		}

		// LOGON & ACOUNT STATISTIC DATA TRANSMISSION (RANA to FIMD)
		tStatTime = (now / DEF_STAT_UNIT) * DEF_STAT_UNIT;
		localtime_r((time_t*)&tStatTime, &tmStat);
		if( tmLast.tm_min != tmStat.tm_min ) {
			report_StatData2STMD();
			gSIdx = tmStat.tm_min%DEF_STAT_SET_CNT;
			tmLast = tmStat;
		}

		if((offset = nifo_msg_read(pstMEMSINFO, dMyQid, NULL)) <= 0) {
			usleep(1);
			continue;
		}

		pNextNode = nifo_ptr(pstMEMSINFO, offset);
		pCurrNode = NULL;

		while( pCurrNode != pNextNode )
		{
			pNode      = pNextNode;
			pCurrNode  = pNextNode;
			sub_offset = nifo_offset(pstMEMSINFO, pNode);

			pNextNode      = (UCHAR *)nifo_entry(nifo_ptr(pstMEMSINFO, ((NIFO *)pNextNode)->nont.offset_next), NIFO, nont);
			pNodeData      = nifo_get_value(pstMEMSINFO, ETH_DATA_NUM, sub_offset);
			pstIPTCPHeader = (st_IPTCPHeader *)nifo_get_value(pstMEMSINFO, INFO_ANA_NUM, sub_offset);

			// COMMAND 처리, CONFIGURATION DATA SYNC.
			if((pstIPTCPHeader == NULL) || (pNode == NULL))
			{
				pNOTISIG = (NOTIFY_SIG *)nifo_get_value(pstMEMSINFO, NOTIFY_SIG_DEF_NUM, sub_offset);
				if( pNOTISIG != NULL ) {
					dAppLog( LOG_DEBUG, "NOTI SIG TYPE:%u", pNOTISIG->dFltType);

					switch( pNOTISIG->dFltType ) 
					{
						case NOTI_TRACE_TYPE :
							dSetCurTrace(pNOTISIG);
							break;
						case NOTI_PDSN_TYPE :
							REConstructStat(pNOTISIG);
							break;
						case NOTI_PB_TYPE :
							dSetCurRSetList(pNOTISIG);
							break;
						case NOTI_RULE_TYPE :
							dSetCurRSetUsed(pNOTISIG);
							break;
						case NOTI_CPS_TYPE :
							dSetCurCPSOvld(pNOTISIG);
							break;
						case NOTI_TIME_TYPE :
							dSetCurTIMEOUT(pNOTISIG);
							break;
						case NOTI_ALL : // 초기화 시에만 쓰임. 
							dSetCurTrace(pNOTISIG);
							dSetCurPdsn(pNOTISIG);
							dSetCurRSetList(pNOTISIG);
							dSetCurRSetUsed(pNOTISIG);
							dSetCurCPSOvld(pNOTISIG);
							dSetCurTIMEOUT(pNOTISIG);
							break;
						case NOTI_SCE_TYPE :
							sce_log_out(pNOTISIG);
							break;
						default :
							dAppLog( LOG_CRI, "INVALID NOTI SIG TYPE:%u", pNOTISIG->dFltType);
							break;
					}
				}
				nifo_node_unlink_nont(pstMEMSINFO, pNode);
				nifo_node_delete(pstMEMSINFO, pNode);
				continue;
			}
			// RADIUS CALL 처리.
			else
			{
				nifo_node_unlink_nont(pstMEMSINFO, pNode);
#ifdef MEM_TEST
				nifo_node_delete(pstMEMSINFO, pNode);
				continue;
#endif
				dSwitchRANAMsg(pNodeData, pstIPTCPHeader);
				nifo_node_delete(pstMEMSINFO, pNode);
			}
		}
	}

    FinishProgram();
    return 1;
}

void sce_log_out(NOTIFY_SIG *pNOTISIG)
{
	SUBS_INFO 		si;
	int 			dRet = 0, flag = 0, tidx = -1;
	char    		mmlBuf[BUFSIZ];
	rad_sess_key    sess_key;
	rad_sess_body   *pBody;	
	int				dIMSI = 0;

	memset (&sess_key, 0x00, sizeof(rad_sess_key));

	/* Parameter setting && make login feild */
	dAppLog(LOG_CRI, "[sce_log_out] IMSI:%s IP:%s", pNOTISIG->stNoti.szMIN, pNOTISIG->stNoti.szIP);

	if( pNOTISIG->stNoti.szMIN[0] == '\0' )
	{
		/* mml response */
		sprintf(mmlBuf, "\tSYSTEM = %s\n\tAPP = %s\n\tFAILED LOGOUT IMSI NULL\n", mySysName, myAppName);
		flag = -1;
	}
	strcpy(si.szMIN, pNOTISIG->stNoti.szMIN);

	if( pNOTISIG->stNoti.szIP[0] == '\0' )
	{
		/* mml response */
		sprintf(mmlBuf, "\tSYSTEM = %s\n\tAPP = %s\n\tFAILED LOGOUT IP NULL\n", mySysName, myAppName);
		flag = -2;
	}
	strcpy(si.szFramedIP, pNOTISIG->stNoti.szIP);

	strcpy(sess_key.szCSID, si.szMIN);
	dAppLog(LOG_DEBUG, "IMSI:%s  hash_key:%s", si.szMIN, sess_key.szCSID);
	pBody =	find_rad_sess (&sess_key);
	/* mmc-log-out 명령에 대한 가입자가 sessin 정보에 있으면 해당 가입자를 logout 시키고 결과에 따라 
	   logout success, error handler 에서 mmc response를 구성하여 응답한다.
	   session 정보에 없으면 이 함수에서 바로 mmc response를 구성하여 응답한다. 
	 */
	if (pBody != NULL) { 
		sprintf(si.szDomain, "subscribers");
		si.type = IP_RANGE;
		si.uiCBit = pBody->uiCBit;
		si.uiPBit = pBody->uiPBit;
		si.uiHBit = pBody->uiHBit;
		si.dTrcFlag = CheckTrace(&si);
		si.usSvcID = SID_LOG_OUT_MMC;
		tidx = pBody->dConnID;

		/* operation logout */
		del_rad_sess_key(&sess_key);
		dAppLog(LOG_CRI, "sce-log-out][IMSI:%s] trying", si.szMIN);

		/** Select RLEG **/
		dIMSI = atoi(&si.szMIN[strlen(si.szMIN)-1]);
		if((tidx = RouteRLEG(dIMSI, tidx)) < 0)
		{
			dAppLog(LOG_CRI, "sce-log-out] RouteRLEG FAIL tidx=%d dIMSI=%lld", tidx, dIMSI);
			/* mml response */
			sprintf( mmlBuf, "\tSYSTEM = %s\n\tAPP = %s\n\tFAILED LOGOUT IMSI:%s SEND FAIL\n", mySysName, myAppName, si.szMIN );
			flag = -3;
		}

		if((dRet = SendToRLEG(tidx, &si, DEF_LOG_OUT_MMC)) < 0)
		{
			dAppLog(LOG_CRI, "SendToRLEG FAIL dRet=%d", dRet);
			/* mml response */
			sprintf( mmlBuf, "\tSYSTEM = %s\n\tAPP = %s\n\tFAILED LOGOUT IMSI:%s SendToRLEG FAIL\n", mySysName, myAppName, si.szMIN );
			flag = -4;
		}
		/* mml response
		 * 실제 응답은 RLEG handler 함수 에서 처리 
		 */
	}
	else {
		dAppLog(LOG_CRI, "sce-log-out][IMSI:%s] failed logout, session not found", si.szMIN);
		/* mml response */
		sprintf( mmlBuf, "\tSYSTEM = %s\n\tAPP = %s\n\tFAILED LOGOUT IMSI:%s not exist\n", mySysName, myAppName, si.szMIN );
	}

	if( flag < 0 )
	{
		dAppLog(LOG_CRI, "sce-log-out] failed logout flag=%d", flag);
		comm_txMMLResult(dIxpcQid, mmlBuf, 0, 0, 0, 0, 1);
	}
}

/* PDSN 이 변경되었을 경우, 현재 까지의 ACCOUT 통계를 다시 재배열해야 한다. */
void REConstructStat(NOTIFY_SIG *pNOTISIG)
{
	LEG_STAT_t	stAcct, stBackup;
	int i, pidx, addedPdsnCnt;

	memset(&stAcct,   0x00, sizeof(stAcct));
	memset(&stBackup, 0x00, sizeof(LEG_STAT_t));
	memcpy(&stBackup, &gpstTotStat[gSIdx]->stAcct, sizeof(LEG_STAT_t));

	if( dSetCurPdsn(pNOTISIG) < 0 ){
		return;
	}

	for(i = 0, addedPdsnCnt = 0; i < stBackup.uiPDSN_Cnt; i++ )
	{
		if( !stBackup.staPDSN_Stat[i].uiPDSN_IP ){
			continue;
		}
		pidx = dCheck_PDSN_IP2(stBackup.staPDSN_Stat[i].uiPDSN_IP);
		if( pidx < 0 ){
			if( (pNOTISIG->stNoti.dPdsnCnt + addedPdsnCnt) < DEF_PDSN_CNT ){
				pidx = pNOTISIG->stNoti.dPdsnCnt + addedPdsnCnt++;
				dAppLog(LOG_CRI,"Auto PDSN IP add, PDSN IP=%u, index=%d", 
						stBackup.staPDSN_Stat[i].uiPDSN_IP, pidx);
			} else {
				dAppLog(LOG_CRI,"FAILED IN Auto PDSN add, Permitted PDSN Count Over, Drop PDSN IP=%u",
						stBackup.staPDSN_Stat[i].uiPDSN_IP);
				continue;
			}
		}
		memcpy(&stAcct.staPDSN_Stat[pidx], &stBackup.staPDSN_Stat[i], sizeof(PDSN_STAT_t));
	}
	stAcct.uiPDSN_Cnt = pNOTISIG->stNoti.dPdsnCnt;
	memcpy(&gpstTotStat[gSIdx]->stAcct, &stAcct, sizeof(LEG_STAT_t));
}

void WriteAcctCnt(int pidx, int dMsgID, int Fail)
{
	gpstTotStat[gSIdx]->stAcct.staPDSN_Stat[pidx].uiPDSN_RecvCnt++;

	gpstTotStat[gSIdx]->stAcct.staPDSN_Stat[pidx].uiPDSN_IP = gpCurPdsn->uiAddr[pidx];
	switch(dMsgID)
	{
		case MID_ACC_START:
			gpstTotStat[gSIdx]->stAcct.staPDSN_Stat[pidx].uiPDSN_StartCnt++;
			if( Fail == 0 )
				gpstTotStat[gSIdx]->stAcct.staPDSN_Stat[pidx].uiLogOn_StartCnt++;
			break;
		case MID_ACC_INTERIM:
			gpstTotStat[gSIdx]->stAcct.staPDSN_Stat[pidx].uiPDSN_InterimCnt++;
			if( Fail == 0 )
				gpstTotStat[gSIdx]->stAcct.staPDSN_Stat[pidx].uiLogOn_InterimCnt++;
			break;
		case MID_ACC_END:
			gpstTotStat[gSIdx]->stAcct.staPDSN_Stat[pidx].uiPDSN_StopCnt++;
			if( Fail == 0 )
				gpstTotStat[gSIdx]->stAcct.staPDSN_Stat[pidx].uiLogOn_StopCnt++;
			break;
		case MID_DISC_REQ:
			gpstTotStat[gSIdx]->stAcct.staPDSN_Stat[pidx].uiPDSN_DiscReqCnt++;
			if( Fail == 0 )
				gpstTotStat[gSIdx]->stAcct.staPDSN_Stat[pidx].uiLogOn_DiscReqCnt++;
			break;
	}
}

int SelectRLEG(void)
{
	int i, minIdx = -1;
	unsigned int uiMinCps = gpCurCpsCtrl->over_cps, uiCurCps = 0;
	LEG_SM_CONN	*pConn = loc_sadb->smConn;
	LEG_SUM_CPS	*pCps = gpstCallInfo[gCIdx]->cps;

	for( i = 0; i < MAX_RLEG_CNT; i++ )
	{
		if( pConn[i].dConn == DISCONNECTED )
			continue;
		uiCurCps = (unsigned int)pCps[i].uiLogOnSumCps / CALL_UNIT;
		if( uiCurCps < uiMinCps )
		{
			minIdx = i;
			uiMinCps = uiCurCps;
		}
	}
	return minIdx;
}

int RouteRLEG (int dIMSI, int chId)
{
	int dRet = -1, i, minIdx = -1;
	unsigned int uiMinCps = gpCurCpsCtrl->over_cps, uiCurCps = 0;
	LEG_SM_CONN	*pConn = loc_sadb->smConn;
	LEG_SUM_CPS	*pCps = gpstCallInfo[gCIdx]->cps;

	if( chId < 0 || chId > MAX_RLEG_CNT )
	{
		if( pConn[MAX_RLEG_CNT].dConn == CONNECTED ) {
			dRet = dIMSI % MAX_RLEG_CNT;
		}
		else {
			for( i = 0; i < MAX_RLEG_CNT; i++ )
			{
				if( pConn[i].dConn == DISCONNECTED )
					continue;
				uiCurCps = (unsigned int)pCps[i].uiLogOnSumCps / CALL_UNIT;
				if( uiCurCps < uiMinCps ) {
					minIdx = i;
					uiMinCps = uiCurCps;
				}
			}
			dRet = minIdx;
			dAppLog(LOG_CRI, "dIMSI=%d, chID = %d CONN[%u %u %u %u %u %u] minIdx=%d"
					, dIMSI, chId
					, pConn[0].dConn, pConn[1].dConn, pConn[2].dConn
					, pConn[3].dConn, pConn[4].dConn, pConn[5].dConn, minIdx);
		}
	}
	else {
		if( pConn[chId].dConn == CONNECTED ) {
			dRet = chId;
		}
		else {
#if 1
			for( i = 0; i < MAX_RLEG_CNT; i++ )
			{
				if( pConn[i].dConn == DISCONNECTED )
					continue;
				uiCurCps = (unsigned int)pCps[i].uiLogOnSumCps / CALL_UNIT;
				if( uiCurCps < uiMinCps ) {
					minIdx = i;
					uiMinCps = uiCurCps;
				}
			}
			dRet = minIdx;
#else
			dRet = -1;
#endif
		}
	}

	return dRet;
}


int dFilterACCStart(int dMsgID, st_RADInfo *pstRADInfo)
{
	int 			dRet = 0, dTrcFlag = -1;
	SUBS_INFO		si;
	PST_PKG_INFO	pstPKGInfo = NULL;
	rad_sess_key	sess_key;
	rad_sess_body	*sess_body;
	int				tidx = -1;
	int				dSumCps = gpstCallDataPerSec->cps.uiLogOnSumCps;
	int				dIMSI = 0;

	dTrcFlag = Trace_LOGIN_Req (pstRADInfo, TRACE_TYPE_ACC_START);

	/* P/H BIT SCOPE */
	if ((pstRADInfo->uiPBIT > MAX_PBIT_CNT) || (pstRADInfo->uiHBIT > MAX_HBIT_CNT)) {
		// p/h bit escapes a scope	
		dAppLog(LOG_CRI, "S:%d] %s %02d %02d %02d"
				, ERR_10001, pstRADInfo->szMIN, pstRADInfo->uiCBIT
				, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT);
		return ERR_10001;
	}

	/** CALL CONTROL - 2. RULE SET USED FLAG (ON/OFF) **/
	if (gpCurRSetUsed->stRule[pstRADInfo->uiPBIT][pstRADInfo->uiHBIT].uiUsedF != 1) {
		// ruleset used flag off
		dAppLog(LOG_DEBUG, "S:%d] %s %02d %02d %02d"
				, ERR_10002, pstRADInfo->szMIN, pstRADInfo->uiCBIT
				, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT);
		return ERR_10002;
	}

	/** CALL CONTROL - 3. RADIUS SVC OPT **/
	if (dCheck_SvcOpt(pstRADInfo->dSvcOpt) < 0) {
		// not match service option
		dAppLog(LOG_CRI, "S:%d] %s %02d %02d %02d svc%d"
				, ERR_10003, pstRADInfo->szMIN, pstRADInfo->uiCBIT
				, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT, pstRADInfo->dSvcOpt);
		return ERR_10003;
	}

	/** CALL CONTROL - 4. RULE SET MACTH **/ 
	pstPKGInfo = &(gpCurRsetList->stPBTable[pstRADInfo->uiPBIT][pstRADInfo->uiHBIT]);
	if ((pstPKGInfo == NULL) || (pstPKGInfo->ucUsedFlag != 1)) {
		// not exist p/hbit at rulefile
		dAppLog(LOG_CRI, "S:%d] %s %02d %02d %02d"
				, ERR_10004, pstRADInfo->szMIN, pstRADInfo->uiCBIT
				, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT);
		return ERR_10004;
	}

	/* CALL INFORMATION  IS CORRECT */
	dAppLog(LOG_DEBUG, "S] %s %d %02d %02d %02d :matched call"
			, pstRADInfo->szMIN, pstRADInfo->dSvcOpt, pstRADInfo->uiCBIT
			, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT);

	/* C Bit==1 이면, packageid = 4998 fix */
	si.sPkgNo = (pstRADInfo->uiCBIT==1) ? 4998 : pstPKGInfo->sPkgNo;
	if (si.sPkgNo < 0) {
		// ruset is negative value
		dAppLog(LOG_CRI, "S:%d] %s %02d %02d %02d"
				, ERR_10005, pstRADInfo->szMIN, pstRADInfo->uiCBIT
				, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT);
		return ERR_10005;
	}

	/** CALL CONTROL - 5. CPS CONTROL **/
	if(dCheck_CpsOverLoad(dSumCps/CPS_UNIT, gpCurCpsCtrl)) {
		// cps over
		dAppLog(LOG_CRI, "S:%d] %s %02d %02d %02d"
				, ERR_10007, pstRADInfo->szMIN, pstRADInfo->uiCBIT
				, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT);
		return ERR_10007;
	}

	makeSubsRecords (pstRADInfo, &si);
	si.dTrcFlag = dTrcFlag;

	/* SESSION ADD */
	strncpy(sess_key.szCSID, pstRADInfo->szMIN, MAX_CSID_SIZE);
	sess_key.szCSID[MAX_CSID_SIZE-1] = 0;
	if ((sess_body = get_rad_sess (&sess_key)) == NULL) {
		// session creation failed
		dAppLog(LOG_CRI, "S:%d] %s %02d %02d %02d"
				, ERR_10008, pstRADInfo->szMIN, pstRADInfo->uiCBIT
				, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT);
		return ERR_10008;	
	}

	sess_body->sPkgID = si.sPkgNo;
	sess_body->uiCBit = pstRADInfo->uiCBIT;
	sess_body->uiPBit = pstRADInfo->uiPBIT;
	sess_body->uiHBit = pstRADInfo->uiHBIT;
	sess_body->uiFramedIP = pstRADInfo->uiFramedIP;
	sess_body->dTrcFlag = dTrcFlag;

	/** SELECT RLEG **/
	dIMSI = atoi(&pstRADInfo->szMIN[strlen(pstRADInfo->szMIN)-1]);
	if((tidx = RouteRLEG(dIMSI, -1)) < 0) {
		dAppLog(LOG_CRI, "S:%d] %s %02d %02d %02d dIMSI=%d tidx=%d Connect=%d"
				, ERR_10010, pstRADInfo->szMIN, pstRADInfo->uiCBIT
				, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT, 
				dIMSI, tidx, loc_sadb->smConn[dIMSI%MAX_RLEG_CNT]);
		sess_body->uiDoneLogOnF = 0;	
		sess_body->dConnID = -1;
		return ERR_10010;
	}
	sess_body->dConnID = tidx;

	/** SEND RLEG **/
	if((dRet = SendToRLEG(tidx, &si, DEF_LOG_ON)) < 0) {
		dAppLog(LOG_CRI, "S:%d] %s %02d %02d %02d dRet=%d"
				, ERR_10011, pstRADInfo->szMIN, pstRADInfo->uiCBIT
				, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT, dRet);
		
		sess_body->uiDoneLogOnF = 0;	
		return ERR_10011;
	}
	dAppLog(LOG_DEBUG, "S] SendToRLEG SUCCESS dConnID=%d, MIN=%s",tidx, sess_key.szCSID);
	sess_body->uiDoneLogOnF = 1;	

	return 0;
}

int dFilterACCInterim(int dMsgID, st_RADInfo *pstRADInfo)
{
	int				dRet = 0, dTrcFlag = -1;
	SUBS_INFO		si;
	PST_PKG_INFO	pstPKGInfo = NULL;
	rad_sess_key	sess_key;
	rad_sess_body	*sess_body;
	int				tidx = -1;
	int				dSumCps = gpstCallDataPerSec->cps.uiLogOnSumCps;
	int				dIMSI = 0;

	dTrcFlag = Trace_LOGIN_Req (pstRADInfo, TRACE_TYPE_ACC_INTERIM);

	/* P/H BIT SCOPE */
	if ((pstRADInfo->uiPBIT > MAX_PBIT_CNT) || (pstRADInfo->uiHBIT > MAX_HBIT_CNT)) {
		// p/h bit escapes a scope
		dAppLog(LOG_CRI, "I:%d] %s %02d %02d %02d"
				, ERR_20001, pstRADInfo->szMIN, pstRADInfo->uiCBIT
				, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT);
		return ERR_20001;
	}

	/** CALL CONTROL - 2. RULE SET USED FLAG (ON/OFF) **/
	if (gpCurRSetUsed->stRule[pstRADInfo->uiPBIT][pstRADInfo->uiHBIT].uiUsedF != 1) {
		// ruleset used flag off
		dAppLog(LOG_DEBUG, "I:%d] %s %02d %02d %02d"
				, ERR_20002, pstRADInfo->szMIN, pstRADInfo->uiCBIT
				, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT);
		return ERR_20002;
	}

	/** CALL CONTROL - 3. RADIUS SVC OPT **/
	if (dCheck_SvcOpt(pstRADInfo->dSvcOpt) < 0) {
		// not matched  service option
		dAppLog(LOG_CRI, "I:%d] %s %02d %02d %02d svc=%d"
				, ERR_20003, pstRADInfo->szMIN, pstRADInfo->uiCBIT
				, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT, pstRADInfo->dSvcOpt);
		return ERR_20003;
	}

	/** CALL CONTROL - 4. RULE SET MACTH **/ 
	pstPKGInfo = &(gpCurRsetList->stPBTable[pstRADInfo->uiPBIT][pstRADInfo->uiHBIT]);
	if ((pstPKGInfo == NULL) || (pstPKGInfo->ucUsedFlag != 1)) {
		// not exist p/hbit at rulefile
		dAppLog(LOG_CRI, "I:%d] %s %02d %02d %02d"
				, ERR_20004, pstRADInfo->szMIN, pstRADInfo->uiCBIT
				, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT);
		return ERR_20004;
	}

	/* INTERIM 이 왔지만 DB상에 LOGOUT 된 상태(MAPPING IP가 없는)는 
   	 * 현재 RADISU ACCOUNT REQ INTERIM 정보상의
	 * BITSET 의 정보에 해당하는 PackageID로 LOGIN 한다 **/
	/* C Bit==1 이면, packageid = 4998 fix */
	si.sPkgNo = (pstRADInfo->uiCBIT==1) ? 4998 : pstPKGInfo->sPkgNo;
	if (si.sPkgNo < 0) {
		// ruleset is negative value
		dAppLog(LOG_CRI, "I:%d] %s %02d %02d %02d"
				, ERR_20005, pstRADInfo->szMIN, pstRADInfo->uiCBIT
				, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT);
		return ERR_20005;
	}

	/** CALL CONTROL - 5. CPS CONTROL **/
	if(dCheck_CpsOverLoad(dSumCps/5, gpCurCpsCtrl)) {
		// cps over
		dAppLog(LOG_CRI, "I:%d] %s %02d %02d %02d"
				, ERR_20007, pstRADInfo->szMIN, pstRADInfo->uiCBIT
				, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT);
		return ERR_20007;
	}

	/* CALL INFORMATION  IS CORRECT */
	dAppLog(LOG_DEBUG, "I] %s %02d %02d %02d %d:matched call"
			, pstRADInfo->szMIN, pstRADInfo->uiCBIT
			, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT, pstRADInfo->dSvcOpt);

	/* SESSION PROCESS */	
	strncpy(sess_key.szCSID, pstRADInfo->szMIN, MAX_CSID_SIZE);
	sess_key.szCSID[MAX_CSID_SIZE-1] = 0;
	sess_body = find_rad_sess (&sess_key);
	if (sess_body == NULL) {
		/* SESSION ADD */
		strncpy(sess_key.szCSID, pstRADInfo->szMIN, MAX_CSID_SIZE);
		sess_key.szCSID[MAX_CSID_SIZE-1] = 0;
		if ((sess_body = get_rad_sess (&sess_key)) == NULL) {
			// session creation failed
			dAppLog(LOG_CRI, "I:%d] %s %02d %02d %02d"
					, ERR_20008, pstRADInfo->szMIN, pstRADInfo->uiCBIT
					, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT);
			return ERR_20008;	
		}
		tidx = -1;

		// session not exist
		dAppLog(LOG_DEBUG, "I:%d] %s %02d %02d %02d"
				, ERR_20009, pstRADInfo->szMIN, pstRADInfo->uiCBIT
				, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT);

	}
	else
	{
		tidx = sess_body->dConnID;
		if( si.sPkgNo == sess_body->sPkgID ) {
			// ruleset equal
			dAppLog(LOG_CRI, "I:%d] %s %02d %02d %02d (%d %d)"
					, ERR_20006, pstRADInfo->szMIN, pstRADInfo->uiCBIT
					, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT, si.sPkgNo, pstPKGInfo->sPkgNo);
			return ERR_20006;
		}
	}

	sess_body->sPkgID = si.sPkgNo;
	sess_body->uiCBit = pstRADInfo->uiCBIT;
	sess_body->uiPBit = pstRADInfo->uiPBIT;
	sess_body->uiHBit = pstRADInfo->uiHBIT;
	sess_body->uiFramedIP = pstRADInfo->uiFramedIP;
	sess_body->dTrcFlag = dTrcFlag;

	makeSubsRecords (pstRADInfo, &si);
	si.dTrcFlag = dTrcFlag;

	/* DB 상에 LOGIN 상태의 호에 대해 INTERIM 이 들어온 경우, 
	 * DB상의 PackageID와 현재 들어온 INTERIM 의 BITSET 
	 * 에 해당하는 PackageID와 다른 경우만 재 LOGIN 한다. **/
	/** Select RLEG **/
	dIMSI = atoi(&pstRADInfo->szMIN[strlen(pstRADInfo->szMIN)-1]);
	if((tidx = RouteRLEG(dIMSI, tidx)) < 0) {
		// route to RLEG fail
		dAppLog(LOG_CRI, "I:%d] %s %02d %02d %02d tidx=%d"
				, ERR_20010, pstRADInfo->szMIN, pstRADInfo->uiCBIT
				, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT, tidx);
		return ERR_20010;
	}
	sess_body->dConnID = tidx;
	if((dRet = SendToRLEG(tidx, &si, DEF_LOG_ON)) < 0) {
		// send to RLEG fail
		dAppLog(LOG_CRI, "I:%d] %s %02d %02d %02d dRet=%d"
				, ERR_20011, pstRADInfo->szMIN, pstRADInfo->uiCBIT
				, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT, dRet);

		sess_body->uiDoneLogOnF = 0;	
		return ERR_20011;
	}

	sess_body->uiDoneLogOnF = 1;	
	return 0;
}

int dFilterACCEnd(int dMsgID, st_RADInfo *pstRADInfo)
{
	int				dRet = 0, dTrcFlag = -1;
	SUBS_INFO		si;
	PST_PKG_INFO	pstPKGInfo = NULL;
	rad_sess_key	sess_key;
	rad_sess_body	*sess_body;
	int				tidx = -1;
	int				isSess=0, isLogon=0;
	int 			dIMSI;

	dTrcFlag = Trace_LOGOUT (pstRADInfo, TRACE_TYPE_ACC_END);

	/* P/H BIT SCOPE */
	if((pstRADInfo->uiPBIT > MAX_PBIT_CNT) || (pstRADInfo->uiHBIT > MAX_HBIT_CNT)) {
		// p/h bit escapes a scope
		dAppLog(LOG_CRI, "E:%d] %s %02d %02d %02d"
				, ERR_30001, pstRADInfo->szMIN, pstRADInfo->uiCBIT
				, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT);
		return ERR_30001;
	}

	/* SESSION PROCESS */	
	strncpy(sess_key.szCSID, pstRADInfo->szMIN, MAX_CSID_SIZE);
	sess_key.szCSID[MAX_CSID_SIZE-1] = 0;
	sess_body = find_rad_sess (&sess_key);
	if (sess_body == NULL) {
		/** CALL CONTROL - 2. RULE SET USED FLAG (ON/OFF) **/
		/* MODIFY: by june, 2010-09-09
		 *		- SCE Logon 된 Subscriber 를 제거하기 위해 변경함
		 *      - RLEG Session 이 있는 경우에는 Hbit의 used flag가 1 이든 0이든 모두 SCE Logout 하고
		 *		- RLEG Session 이 없는 경우에는 Hbit의 used flag가 1 인 경우에만 SCE Logout 처리 함.
		 */
		if (gpCurRSetUsed->stRule[pstRADInfo->uiPBIT][pstRADInfo->uiHBIT].uiUsedF != 1) {
			// used flag zero and session not found
			dAppLog(LOG_DEBUG, "E:%d] %s %02d %02d %02d"
					, ERR_30012, pstRADInfo->szMIN, pstRADInfo->uiCBIT
					, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT);
			return ERR_30012;
		}
		
		// session not exist
		dAppLog(LOG_DEBUG, "E:%d] %s %02d %02d %02d"
				, ERR_30009, pstRADInfo->szMIN, pstRADInfo->uiCBIT
				, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT);
		tidx = -1;
	}
	else {
		isSess = 1;
		isLogon = sess_body->uiDoneLogOnF;
		tidx = sess_body->dConnID;
		del_rad_sess (&sess_key, sess_body);
		if (!isLogon) {
			// used flag zero and non-logon session
			dAppLog(LOG_CRI, "E:%d] %s %02d %02d %02d"
					, ERR_30013, pstRADInfo->szMIN, pstRADInfo->uiCBIT
					, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT);
			return ERR_30013;
		}
	}

	/** CALL CONTROL - 3. RULE SET MACTH **/ 
	pstPKGInfo = &(gpCurRsetList->stPBTable[pstRADInfo->uiPBIT][pstRADInfo->uiHBIT]);
	if ((pstPKGInfo == NULL) || (pstPKGInfo->ucUsedFlag != 1)) {
		// not exist p/hbit at rulefile
		dAppLog(LOG_CRI, "E:%d] %s %02d %02d %02d"
				, ERR_30014, pstRADInfo->szMIN, pstRADInfo->uiCBIT
				, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT);
		return ERR_30014;
	}

	dAppLog(LOG_DEBUG, "E] %s : matched call", pstRADInfo->szMIN);
	makeSubsRecords (pstRADInfo, &si);
	si.dTrcFlag = dTrcFlag;
	
	/** Select RLEG **/
	dIMSI = atoi(&pstRADInfo->szMIN[strlen(pstRADInfo->szMIN)-1]);
	if((tidx = RouteRLEG(dIMSI, tidx)) < 0) {
		// route to RLEG fail
		dAppLog(LOG_CRI, "E:%d] %s %02d %02d %02d tidx=%d"
				, ERR_30010, pstRADInfo->szMIN, pstRADInfo->uiCBIT
				, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT, tidx);
		return ERR_30010;
	}
	
	if((dRet = SendToRLEG(tidx, &si, DEF_LOG_OUT)) < 0) {
		// send to RLEG fail
		dAppLog(LOG_CRI, "E:%d] %s %02d %02d %02d dRet=%d"
				, ERR_30011, pstRADInfo->szMIN, pstRADInfo->uiCBIT
				, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT, dRet);
		return ERR_30011;
	}
	return 0;
}

int dFilterDISC(st_RADInfo *pstRADInfo)
{
	int				dRet = 0, dTrcFlag = -1;
	SUBS_INFO		si;
	PST_PKG_INFO	pstPKGInfo = NULL;
	rad_sess_key	sess_key;
	rad_sess_body	*sess_body;
	int				tidx = -1;
	int				dSumCps = gpstCallDataPerSec->cps.uiLogOnSumCps;
	int				dIMSI = 0;

	dTrcFlag = Trace_LOGIN_Req (pstRADInfo, TRACE_TYPE_DISCONN_REQ);

	if (pstRADInfo->ucHBITF != DEF_FLAG_ON){
		dAppLog(LOG_CRI, "D:%d] %s", ERR_40012, pstRADInfo->szMIN);
		return ERR_40012;
	}

	/** CALL CONTROL - 2.SESSION CHECK */	
	strncpy(sess_key.szCSID, pstRADInfo->szMIN, MAX_CSID_SIZE);
	sess_key.szCSID[MAX_CSID_SIZE-1] = 0;
	if((sess_body = find_rad_sess(&sess_key)) == NULL) {
		// session not exist
		dAppLog(LOG_CRI, "D:%d] %s", ERR_40009, pstRADInfo->szMIN);
		return ERR_40009;	
	}

	tidx = sess_body->dConnID;
	sess_body->uiDoneLogOnF = 0;
	/* ACCINFO REBUILDING
	 * - 기존 function을 재사용하기 위해 찾은 Session 정보를 ACCINFO 구조체에 다시 할당.
	 */
	pstRADInfo->uiFramedIP 	= sess_body->uiFramedIP;
	pstRADInfo->uiCBIT 		= sess_body->uiCBit;
	pstRADInfo->uiPBIT 		= sess_body->uiPBit;

	dAppLog(LOG_INFO, "D] FIND SESS %s %02d %02d %02d"
			, pstRADInfo->szMIN, sess_body->uiCBit
			, sess_body->uiPBit, sess_body->uiHBit);

	/** CALL CONTROL - 3. RULE SET CHECK **/
	/* 3.1 HBIT SCOPE CHECK */
	if (pstRADInfo->uiHBIT > MAX_HBIT_CNT) {
		// hbit escapes a scope
		dAppLog(LOG_CRI, "D:%d] %s %02d %02d %02d"
				, ERR_40001, pstRADInfo->szMIN, pstRADInfo->uiCBIT
				, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT);
		return ERR_40001;
	}

	/* 3.2 RULE SET USED FLAG (ON/OFF) **/
	if (gpCurRSetUsed->stRule[pstRADInfo->uiPBIT][pstRADInfo->uiHBIT].uiUsedF != 1) {
		// ruleset used flag off
		dAppLog(LOG_DEBUG, "D:%d] %s %02d %02d %02d"
				, ERR_40002, pstRADInfo->szMIN, pstRADInfo->uiCBIT
				, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT);
		return ERR_40002;
	}

	/* 3.3 RULE SET MACTH **/ 
	pstPKGInfo = &(gpCurRsetList->stPBTable[pstRADInfo->uiPBIT][pstRADInfo->uiHBIT]);
	if((pstPKGInfo == NULL) || (pstPKGInfo->ucUsedFlag != 1)) {
		// not exist p/hbit at rulefile
		dAppLog(LOG_CRI, "D:%d] %s %02d %02d %02d"
				, ERR_40004, pstRADInfo->szMIN, pstRADInfo->uiCBIT
				, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT);
		return ERR_40004;
	}

	/* 3.4 CBIT CHECK - CBit==1 이면, packageid = 4998 fix */
	si.sPkgNo = (pstRADInfo->uiCBIT==1) ? 4998 : pstPKGInfo->sPkgNo;
	if (si.sPkgNo < 0) {
		// ruleset is negative value
		dAppLog(LOG_CRI, "D:%d] %s %02d %02d %02d"
				, ERR_40005, pstRADInfo->szMIN, pstRADInfo->uiCBIT
				, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT);
		return ERR_40005;
	}

	/* CALL INFORMATION  IS CORRECT */
	dAppLog(LOG_DEBUG, "D] %s %02d %02d %02d :matched call"
			, pstRADInfo->szMIN, pstRADInfo->uiCBIT, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT);

	/* DB 상에 LOGIN 상태의 호에 대해 INTERIM 이 들어온 경우, 
   	 * DB상의 PackageID와 현재 들어온 INTERIM 의 BITSET 
	 * 에 해당하는 PackageID와 다른 경우만 재 LOGIN 한다. **/
	//if ((interim_rst == 1) && (pstRADInfo->uiHBIT != sess_body->uiHBit)) {
	if ((pstRADInfo->uiHBIT == sess_body->uiHBit)) {
		// hbit equal
		dAppLog(LOG_CRI, "D:%d] %s %d %d"
				, ERR_40013, pstRADInfo->szMIN, si.sPkgNo, pstPKGInfo->sPkgNo);
		return ERR_40013;
	}

	sess_body->sPkgID = si.sPkgNo;
	sess_body->uiHBit = pstRADInfo->uiHBIT;
	makeSubsRecords (pstRADInfo, &si);
	si.dTrcFlag = dTrcFlag;

	/** CALL CONTROL - 4. CPS CONTROL **/
	if (dCheck_CpsOverLoad(dSumCps/5, gpCurCpsCtrl)) {
		// cps over
		dAppLog(LOG_CRI, "D:%d] %s %02d %02d %02d"
				, ERR_40007, pstRADInfo->szMIN, pstRADInfo->uiCBIT
				, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT);
		return ERR_40007;
	}

	dIMSI = atoi(&pstRADInfo->szMIN[strlen(pstRADInfo->szMIN)-1]);
	if((tidx = RouteRLEG(dIMSI, tidx)) < 0) {
		// route to RLEG fail
		dAppLog(LOG_CRI, "D:%d] %s %02d %02d %02d tidx=%d"
				, ERR_40010, pstRADInfo->szMIN, pstRADInfo->uiCBIT
				, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT, tidx);
		return ERR_40010;
	}
	
	if((dRet = SendToRLEG(tidx, &si, DEF_LOG_ON)) < 0) {
		// send to RLEG fail
		dAppLog(LOG_CRI, "D:%d] %s %02d %02d %02d dRet=%d"
				, ERR_40011, pstRADInfo->szMIN, pstRADInfo->uiCBIT
				, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT, dRet);
		return ERR_40011;
	}
	sess_body->uiDoneLogOnF = 1;

	return 0;
}

int	dLogonFilter(int dMsgID, st_RADInfo *pstRADInfo)
{
	int dRet = 0;

	switch(dMsgID)
	{
		case MID_ACC_START:		/* ACCOUNT-START MSG : CALL SESSION CREATE */
			dRet = dFilterACCStart(dMsgID, pstRADInfo);
			break;

		case MID_ACC_INTERIM:		/* ACCOUNT-START MSG : CALL SESSION CREATE */
			dRet = dFilterACCInterim(dMsgID, pstRADInfo);
			break;

		/* MODIFY: by june, 2010-09-09
		 * DESC : SCE Logon 된 Subscriber 를 제거하기 위해 변경함
		 *      - RLEG Session 이 있는 경우에는 Hbit의 used flag가 1 이든 0이든 모두 SCE Logout 하고
		 *		- RLEG Session 이 없는 경우에는 Hbit의 used flag가 1 인 경우에만 SCE Logout 처리 함.
		 *		- MODIFY2 방식으로 변경.
		 * MODIFY2: by june, 2010-10-09 (with ysj)
		 *		- 해당 Ruleset(P/H Bit) 의 used flag가 1이면, 무조건 logout 한다.
		 * 		- 해당 ruleset(P/H Bit) 의 used flag가 0이면, logon session 만 logout 한다.
		 */
		case MID_ACC_END:		/* ACCOUNT-STOP MSG : CALL SESSION DELETE */
			dRet = dFilterACCEnd(dMsgID, pstRADInfo);
			break;

		/* ADD : by june, 2010-09-28
		 * DESC: RADIUS DISCONNECT REQEST MSG 추가.
		 *	+ DISCONNECT_REQ MSG가 들어오면 아래의 기준으로 login을 한다.
		 *		1. HBIT ATTRIBUTE CHECK
		 *			- DISCONNETC REQUEST MESSAGE 내에 HBIT ATTRIBUTE 가 없으면 MESSAGE를 버린다.
		 *			- 세션 종료 메시지의 경우 HBIT가 없고,
		 *			- 세션 제어 메시지의 경우 HBIT이 있고, HBIT으로 제어를 한다.
		 * 		2. NASIP CHECK
		 *		   - CALLING_STATION_ID 가 같아도 NASIP(PDSN)가 다르면 메시지를 버린다.
		 *		3. SESSION CHECK
		 *		   - 기존 세션이 없으면 DISCONNECT_REQ 상의 필드 내용에는 login하기 위한
		 *	   	     정보가 부족하기 때문에 login 할수 없다.
		 *		5. RULE SET CHECK
		 *		   - HBIT 가 MAX_HBIT_NUM(32) 보다 작은 경우에만 login 한다.
		 *		   - DISCONNECT_REQ 내 HBIT에 해당하는 usedflag가 1인 경우에만 login 한다.
		 *		   - DISCONNECT_REQ 내 HBIT, 저장된 SESSION 내 PBIT로 조합된 RULE SET이 
		 *			 RULE SET FILE에 있는 경우에만 login한다.
		 *		   - DISCONNECT_REQ 내 CALLING_STATION_ID 로 검색한 session 에 저장된 CBIT가 1인 경우
		 *			 sPkgNo=4998 로 login 한다.
		 *		   - 기존 세션이 있으면 HBIT를 비교하여 다를 경우에만 DISCONNECT_REQ의 HBIT 으로 login 한다.
		 *		   - 기존 세션이 있고 HBIT 가 동일하면 login 하지 않는다.
		 *		6. CPS CHECK
		 */
		case MID_DISC_REQ:		/* DISCONNECT REQEST MSG */
			dRet = dFilterDISC(pstRADInfo);
			break;

		default:
			dAppLog(LOG_CRI, "ERROR NOT-MATCHED MSGID=%d", dMsgID);
			return -1;
	}
	return dRet;
}

int dGetMsgID (st_RADInfo *pRadInfo)
{
	int id=0;
	switch( pRadInfo->ucCode )
	{
		case DEF_ACCOUNTING_REQ:
			if( pRadInfo->dAcctStatType == 1 ) 
    			id = MID_ACC_START;
			else if( pRadInfo->dAcctStatType == 2 ) 
				id = MID_ACC_END;
			else if( pRadInfo->dAcctStatType == 3 ) 
				id = MID_ACC_INTERIM;
			else {
				return -1;
			}
			break;
		case DEF_DISCONNECT_REQ:
			id = MID_DISC_REQ;
			break;
		default:
			return -2;
	}

	return id;
}

int dSwitchRANAMsg(UCHAR *pNodeData, st_IPTCPHeader *pstIPTCPHeader)
{
	int		dRet = 0, dMsgID = 0, pidx = -1;
	char	srcIP[MAX_FRAMEDIP_SIZE], nasIP[MAX_FRAMEDIP_SIZE];
	st_RADInfo 		stRadInfo;
	LEG_DATA_SUM    *pCallData = gpstCallInfo[gCIdx];

	memset( &stRadInfo, 0x00, DEF_RADINFO_SIZE );

	if((dRet = dAnalyze_RADIUS( pNodeData, &stRadInfo, pstIPTCPHeader )) < 0)
	{
		dAppLog(LOG_CRI, "dAnalyze_RADIUS FAIL dRet=%d", dRet);
		return -1;
	}

	dMsgID = dGetMsgID (&stRadInfo);
	if( dMsgID < 0 ) {
		dAppLog(LOG_CRI, "NOT INVALID RADIUS MSG CODE=%u", stRadInfo.ucCode);
		return -2;
	}

	sprintf(srcIP, "%s", CVT_INT2STR_IP(stRadInfo.uiFramedIP));
	sprintf(nasIP, "%s", CVT_INT2STR_IP(stRadInfo.uiNASIP));

	/** CALL CONTROL - 1. NAS IP **/
	if((pidx = dCheck_PDSN_IP2(stRadInfo.uiNASIP)) < 0) {
		dAppLog(LOG_CRI, "%s %s :not matched nas-ip", stRadInfo.szMIN, nasIP);
		return -3;
	}

	// LOGON FILTER & RLEG Selection
	dRet = dLogonFilter(dMsgID, &stRadInfo);
	if (dRet != -1) {
		WriteAcctCnt(pidx, dMsgID, dRet);
		// TPS INCREASE
		pCallData->uiTPS++;
#ifdef PRT_TPS
		dAppLog(LOG_CRI, ">>> TPS=%u", pCallData->uiTPS);
#endif
	}

	return 0;
}


/*******************************************************************************/
int LogTCPIPHeader( pst_IPTCPHeader pstHeader )
{
    int     dLog;

    dLog = LOG_INFO;

    dAppLog( dLog, "##### PACKET INFORMATION ############################################");

    dAppLog( dLog, "[IP : SRC IP    ] : [%s]", CVT_INT2STR_NIP(pstHeader->stIPHeader.dSrcIP) );
    dAppLog( dLog, "[IP : DEST IP   ] : [%s]", CVT_INT2STR_NIP(pstHeader->stIPHeader.dDestIP) );
    dAppLog( dLog, "[IP : IPHeadLen ] : [%d]", pstHeader->stIPHeader.usIPHeaderLen );
    dAppLog( dLog, "[IP : Total Len ] : [%d]", pstHeader->stIPHeader.usTotLen );
    dAppLog( dLog, "[IP : Timelive  ] : [%d]", pstHeader->stIPHeader.ucTimelive );
    dAppLog( dLog, "[IP : PROTOCAL  ] : [%d]", pstHeader->stIPHeader.ucProtocol );
    dAppLog( dLog, "[IP : IDENTI    ] : [%d]", pstHeader->stIPHeader.usIdentification );

    dAppLog( dLog, "[TCP : SEQ      ] : [%lu]", pstHeader->stTCPHeader.dSeq );
    dAppLog( dLog, "[TCP : ACK      ] : [%lu]", pstHeader->stTCPHeader.dAck );
    dAppLog( dLog, "[TCP : WINDOW   ] : [%d]", pstHeader->stTCPHeader.dWindow );
    dAppLog( dLog, "[TCP : SRC PORT ] : [%d]", pstHeader->stTCPHeader.usSrcPort );
    dAppLog( dLog, "[TCP : DEST PORT] : [%d]", pstHeader->stTCPHeader.usDestPort );
    dAppLog( dLog, "[TCP : RTX FLAG ] : [%d]", pstHeader->stTCPHeader.usRtxType );
    dAppLog( dLog, "[TCP : TCP HEAD ] : [%d]", pstHeader->stTCPHeader.usTCPHeaderLen );
    dAppLog( dLog, "[TCP : DATA LEN ] : [%d]", pstHeader->stTCPHeader.usDataLen );
    dAppLog( dLog, "[TCP : CONTROL  ] : [%d]", pstHeader->stTCPHeader.ucControlType );
    dAppLog( dLog, "[TCP : IP FRAG  ] : [%d]", pstHeader->stTCPHeader.ucIPFrag );

    return 1;
}

/*******************************************************************************

*******************************************************************************/
inline int dAnalyze_RADIUS( PUCHAR pBuf, pst_RADInfo pRADInfo, pst_IPTCPHeader pstIPTCPHeader )
{
	int			dRet;
	int			dHeadLen;	/* IP Header Len + TCP Header Len */
	USHORT		usDataLen;
	pst_Radius	pstRadius;
	int			dPaddingSize;
	
	dPaddingSize = pstIPTCPHeader->stIPHeader.usTotLen 
					- pstIPTCPHeader->stTCPHeader.usDataLen 
					- pstIPTCPHeader->stTCPHeader.usTCPHeaderLen
					- pstIPTCPHeader->stIPHeader.usIPHeaderLen;

	dHeadLen = pstIPTCPHeader->stIPHeader.usTotLen - (pstIPTCPHeader->stTCPHeader.usDataLen + dPaddingSize);

	pstRadius = (pst_Radius)(pBuf+dHeadLen);
	pRADInfo->ucCode 	= pstRadius->Code;
	pRADInfo->ucID		= pstRadius->Identifier;

	usDataLen = TOUSHORT(pstRadius->Length);
#ifdef __DEBUG
nifo_dump_DebugString("RADIUS DEBUG", pBuf, pstIPTCPHeader->stTCPHeader.usDataLen);
#endif

	if( pstIPTCPHeader->stTCPHeader.usDataLen < usDataLen ) {
		dAppLog( LOG_DEBUG, "INVALID LENGTH RAD:%d dHeadLen:%d HEAD:%d", usDataLen, dHeadLen, pstIPTCPHeader->stTCPHeader.usDataLen);
		return -1;
	}

	dAppLog(LOG_INFO, "RADIUS CODE[%d]", pstRadius->Code);

	/* Accounting-Response 전송 포함 */
	//if( pstRadius->Code > 5 ) {
	/*	ADD : by june, 2010-09-27
	 *	DESC:
	 *		1. Account_Req (start, stop, interim) 메시지만 처리.(CODE:4)
	 *		2. Disconn_Req 메시지 처리 추가.(CODE:40)
	 *	LINE: 857
	 */
	if ((pstRadius->Code!=DEF_ACCOUNTING_REQ) && (pstRadius->Code!=DEF_DISCONNECT_REQ)) {
		dAppLog( LOG_INFO, "IT IS NOT TARGET MESSAGE[%d]", pstRadius->Code );
		return 0;
	}

	if( usDataLen > 20 ) {

		dRet = dAnalyze_RADIUS_ATTRIB( pstRadius->Attributes, usDataLen-20, pRADInfo );
		if( dRet < 0 ) {
			dAppLog( LOG_INFO, "INVALID ATTRIBUTE LENGTH INFO" );
			return -2;
		}
	}

	return 1;
}


/*******************************************************************************

*******************************************************************************/
inline int dAnalyze_RADIUS_ATTRIB( PUCHAR pBuf, USHORT usDataLen, pst_RADInfo pRADInfo )
{
	int			dOffset = 0;
	USHORT		usLength;
	char		szC23BIT[64];

	pRADInfo->uiRADIUSLen = usDataLen;

	while( dOffset < usDataLen )
	{
		switch( pBuf[dOffset] ) {
			case 4 :	/* NAS IP Address */ 
				usLength = pBuf[dOffset+1];
                if( usLength == 6 ) {
                    memcpy( &pRADInfo->uiNASIP, pBuf+dOffset+2, usLength-2 );
                    dAppLog( LOG_DEBUG, "NONE CONVERT NASIP :%s", CVT_INT2STR_IP(pRADInfo->uiNASIP));
                    pRADInfo->uiNASIP = ntohl( pRADInfo->uiNASIP );
                }
                else
                    dAppLog( LOG_INFO, "INVALID NAS IP ADDRESS LEN:%d", usLength );
                break;
			case 8:     /* Framed IP */
                usLength = pBuf[dOffset+1];
                if( usLength == 6 ) {
                    memcpy( &pRADInfo->uiFramedIP, pBuf+dOffset+2, usLength-2 );
                    dAppLog( LOG_INFO, "++++ NONE CONVERT FRAMED_IP :%s", CVT_INT2STR_IP(pRADInfo->uiFramedIP));
                    pRADInfo->uiFramedIP = ntohl(pRADInfo->uiFramedIP);
                    dAppLog( LOG_INFO, "++++ CONVERT FRAMED_IP :%s", CVT_INT2STR_IP(pRADInfo->uiFramedIP));
                }
                else
                    dAppLog( LOG_INFO, "INVALID FRAMED_IP LEN:%d", usLength );

                break;
			case 31:    /* Calling Station ID */
                usLength = pBuf[dOffset+1];
                if( usLength < (MAX_MIN_SIZE+2) ) {
                    memcpy( &pRADInfo->szMIN[0], pBuf+dOffset+2, usLength-2 );
                    pRADInfo->szMIN[usLength-2] = 0x00;
                }
                else
                    dAppLog( LOG_INFO, "INVALID CALL_ST_ID LEN:%d", usLength );

                break;
			
			case 40:    /* Account Status Type */
                usLength = pBuf[dOffset+1];
                if( usLength == 6 ) {
                    memcpy( &pRADInfo->dAcctStatType, pBuf+dOffset+2, usLength-2 );
                }
                else
                    dAppLog( LOG_INFO, "INVALID ACC_ST_TYPE LEN:%d", usLength );

                break;
#if 0
			case 85:    /* Acct-Interim-Interval */
                usLength = pBuf[dOffset+1];
                if( usLength == 6 ) {
                    memcpy( &pRADInfo->dAcctInterim, pBuf+dOffset+2, usLength-2 );
                    pRADInfo->dAcctInterim = CVT_UINT( pRADInfo->dAcctInterim );
                    pRADInfo->ucAcctInterimF = DEF_FLAG_ON;
                }
                else
                    dAppLog( LOG_INFO, "INVALID FRAMED_IP LEN:%d", usLength );
                break;
#endif
			case 26:	/* Vendor Specific */

				switch( pBuf[dOffset+6] ) {
					case 16:    /* Service Option */
                        usLength = pBuf[dOffset+7];
                        if( usLength == 6 ) {
                            memcpy( &pRADInfo->dSvcOpt, pBuf+dOffset+8, usLength-2 );
                            pRADInfo->dSvcOpt = ntohl(pRADInfo->dSvcOpt);
                        }
                        else
                            dAppLog( LOG_INFO, "INVALID SVC_OPT LEN:%d", usLength );
                    
                        break;
					case 1:	/* C23BIT */
						usLength = pBuf[dOffset+7];
						if( usLength-2 >= sizeof(szC23BIT) ) {
							dAppLog(LOG_DEBUG, "C23BIT TOO LONG!! USLENGTH[%u] SIZEOF[%ld]", usLength, sizeof(szC23BIT));
							memcpy( szC23BIT, pBuf+dOffset+8, sizeof(szC23BIT)-1 );
							szC23BIT[sizeof(szC23BIT)-1] = 0;
						}
						else {
							memcpy( szC23BIT, pBuf+dOffset+8, usLength-2 );
							szC23BIT[usLength-2] = 0x00;
						}

						switch( szC23BIT[0] )
						{
						case 'v':
						case 'V':
							if( strncasecmp( szC23BIT+1, "ci_c23=", 7 ) == 0 )
							{
							pRADInfo->uiC23BIT = atoi(&szC23BIT[8]);
							pRADInfo->ucC23BITF = DEF_FLAG_ON;
							dAppLog( LOG_DEBUG, "C23BIT CHECK [%u][%d]", pRADInfo->uiC23BIT, pRADInfo->ucC23BITF );
							}
							break;
						case 'h':
						case 'H':
							if( strncasecmp( szC23BIT+1, "bit=", 4 ) == 0 )
							{
							pRADInfo->uiHBIT	= atoi(&szC23BIT[5]);
							pRADInfo->ucHBITF	= DEF_FLAG_ON;
							dAppLog( LOG_DEBUG, "HBIT CHECK [%u][%d]", pRADInfo->uiHBIT, pRADInfo->ucHBITF );
							}
							break;
						case 'p':
						case 'P':
							if( strncasecmp( szC23BIT+1, "bit=", 4 ) == 0 )
							{
							pRADInfo->uiPBIT	= atoi(&szC23BIT[5]);
							pRADInfo->ucPBITF	= DEF_FLAG_ON;
							dAppLog( LOG_DEBUG, "PBIT CHECK [%u][%d]", pRADInfo->uiPBIT, pRADInfo->ucPBITF );
							}
							break;
						case 'c':
						case 'C':
							if( strncasecmp( szC23BIT+1, "bit=", 4 ) == 0 )
							{
							pRADInfo->uiCBIT	= atoi(&szC23BIT[5]);
							pRADInfo->ucCBITF	= DEF_FLAG_ON;
							dAppLog( LOG_DEBUG, "CBIT CHECK [%u][%d]", pRADInfo->uiCBIT, pRADInfo->ucCBITF );
							}
							break;
						default:
							dAppLog( LOG_DEBUG, "UNKNOWN C23BIT[%s]", szC23BIT );
							break;
						}
                        break;

					default:
						break;
				}
				break;

			default:
				break;
		}

		if( pBuf[dOffset+1] <= 0 ) {
			dAppLog( LOG_DEBUG, "INVALID ATTRIBUTE:%d LENGTH:%d", pBuf[dOffset], pBuf[dOffset+1] );
			return -1;
		}

		dOffset += pBuf[dOffset+1];
	}

	return 1;
}

int SendToRLEG(int tidx, SUBS_INFO *pSubInfo, int logmode)
{
    int             dRet;
    int             dSize;
	GeneralQMsgType txGenQMsg;
    pst_MsgQ        pstMsgQ;
    pst_MsgQSub     pstMsgQSub;

	txGenQMsg.mtype = MTYPE_RADIUS_TRANSMIT;

	pstMsgQ = (st_MsgQ *)&txGenQMsg.body;
    pstMsgQ->llMType = 0;
    pstMsgQSub = (st_MsgQSub *)&pstMsgQ->llMType;

    pstMsgQSub->usType  = DEF_SVC;
	if( logmode == DEF_LOG_ON )
    	pstMsgQSub->usSvcID = SID_LOG_ON; 		// LOG-ON
	else if( logmode == DEF_LOG_OUT_MMC )
    	pstMsgQSub->usSvcID = SID_LOG_OUT_MMC; 	// LOG-OUT-MMC
	else
    	pstMsgQSub->usSvcID = SID_LOG_OUT; 		// LOG-OUT

	pstMsgQ->usBodyLen = sizeof(SUBS_INFO);

	memcpy( &pstMsgQ->szBody[0], pSubInfo, sizeof(SUBS_INFO) );

	/* send subs info msg */
    dSize = sizeof(st_MsgQ) - MAX_MSGBODY_SIZE + pstMsgQ->usBodyLen;
    if((dRet = msgsnd(dTRCDRQid[tidx], &txGenQMsg, dSize, 0)) < 0) {
        dAppLog(LOG_CRI, "[Qid = %d, tidx=%d, logmode=%d] ERROR SEND : %d[%s]"
				, dTRCDRQid[tidx], tidx, logmode, errno, strerror(errno));
		return -1;
	}

	dAppLog(LOG_DEBUG, "%s() IDX=%d MIN=%s FRAMED_IP=%s CBit=%u PBit=%u HBit=%u"
			, __FUNCTION__, tidx, pSubInfo->szMIN, pSubInfo->szFramedIP
			, pSubInfo->uiCBit, pSubInfo->uiPBit, pSubInfo->uiHBit);

	return 0;
}

int dCheck_PDSN_IP2(UINT addr)
{        
	stHASHONODE			*pHashNode = NULL;
	st_Pdsn_HashKey		stPdsnHashKey; 
	st_Pdsn_HashKey		*pPdsnHashKey = &stPdsnHashKey; 
	st_Pdsn_HashData	stPdsnHashData; 
	st_Pdsn_HashData	*pPdsnHashData = &stPdsnHashData; 

	if (gpCurPdsn->uiCount == 0) return -1;

	// PDSN FILTERING
	pPdsnHashKey->uiIP = addr;
	if((pHashNode = hasho_find(gpCurPdsnHash, (U8 *)pPdsnHashKey)) == NULL) 
		return -1; 
	else
	{
		pPdsnHashData = (st_Pdsn_HashData *)nifo_ptr(gpCurPdsnHash, pHashNode->offset_Data);
		return pPdsnHashData->uiIdx;
	}
}

void prt_trace_info ()
{
	int idx;

	dAppLog(LOG_CRI,"   - COUNT[%d", gpCurTrc->dTCount);
	for(idx = 0; idx < gpCurTrc->dTCount; idx++)
	{
		dAppLog(LOG_CRI,"   - TYPE[%d] IMSI[%s"
				, gpCurTrc->stTrc[idx].dType, gpCurTrc->stTrc[idx].szImsi);
	}
}

void prt_pdsn_info ()
{
	int idx;

	dAppLog(LOG_CRI,"   - COUNT[%u", gpCurPdsn->uiCount);
	for(idx = 0 ; idx < gpCurPdsn->uiCount ; idx++ )
	{ 
		dAppLog(LOG_CRI,"   - %02d. IP[%u", idx, gpCurPdsn->uiAddr[idx]);
	} 
}

void prt_ruleset_list_info ()
{
	dAppLog(LOG_CRI,"   - RULESET COUNT[%d", gpCurRsetList->dCount);
}

void prt_ruleset_used_info ()
{
	dAppLog(LOG_CRI,"   - RULESET USED COUNT[%d", gpCurRSetUsed->uiCount);
}

void prt_cps_info ()
{
	dAppLog(LOG_CRI,"   - CPS[%d] RATE[%d] FLAG[%u"
			, gpCurCpsCtrl->over_cps, gpCurCpsCtrl->over_rate, gpCurCpsCtrl->over_flag);
}

void prt_timeout_info ()
{
	dAppLog(LOG_CRI,"   - SESS TIMEOUT[%u", gpCurMPTimer->sess_timeout);

}

int dReadFLTIDXFile(void)
{
	FILE *fa;
	char szBuf[1024];
	char szType[64];
	int i = 0, dIdx = 0;

	fa = fopen(DEF_NOTI_INDEX_FILE, "r");
	if(fa == NULL)
	{
		dAppLog(LOG_CRI,"dReadFLTIDXFile : %s FILE OPEN FAIL (%s)",
		DEF_NOTI_INDEX_FILE, strerror(errno));
		return -1;
	}

	while(fgets(szBuf,1024,fa) != NULL)
	{
		if(szBuf[0] != '#')
		{
			dAppLog(LOG_CRI,"dReadFLTIDXFile : %s File [%d] row format error",
			DEF_NOTI_INDEX_FILE, i);
			fclose(fa);
			return -1;
		}

		if(szBuf[1] == '#')
			continue;
		else if(szBuf[1] == 'E')
			break;
		else if(szBuf[1] == '@')
		{
			if(sscanf(&szBuf[2],"%s %d",szType, &dIdx) == 2)
			{
				if(strcmp(szType,"TRACE") == 0)
				{
					gpIdx->dTrcIdx = dIdx;
					
					gpCurTrc = (dIdx == 0) ? gpTrcList[0] : gpTrcList[1]; 
					dAppLog(LOG_CRI, " * TRACE READ        : IDX[%d", dIdx);
					prt_trace_info();
				}
				else if( strcmp(szType, "PDSN") == 0 )
				{
					gpIdx->dPdsnIdx = dIdx;
					gpCurPdsn = (dIdx == 0) ? gpPdsnList[0] : gpPdsnList[1];
					gpCurPdsnHash = (dIdx == 0) ? gpPdsnHash[0] : gpPdsnHash[1];
					dAppLog(LOG_CRI, " * PDSN READ         : IDX[%d", dIdx);
					prt_pdsn_info();
				}
				else if( strcmp(szType, "PDSN_CNT") == 0 )
				{
					gpstTotStat[0]->stAcct.uiPDSN_Cnt = dIdx;
					gpstTotStat[1]->stAcct.uiPDSN_Cnt = dIdx;
					dAppLog(LOG_CRI, " * PDSN READ         : CNT[%d", dIdx);
				}
				else if( strcmp(szType, "RULESET_LIST") == 0 )
				{
					gpIdx->dRsetListIdx = dIdx;
					gpCurRsetList = (dIdx == 0) ? gpRsetList[0] : gpRsetList[1];
					dAppLog(LOG_CRI, " * RULESET_LIST READ : IDX[%d", dIdx);
					prt_ruleset_list_info();
				}
				else if( strcmp(szType, "RULESET_USED") == 0 )
				{
					gpIdx->dRsetUsedIdx = dIdx;
					gpCurRSetUsed = (dIdx == 0) ? gpRSetUsedList[0] : gpRSetUsedList[1];
					dAppLog(LOG_CRI, " * RULESET_USED READ : IDX[%d", dIdx);
					prt_ruleset_used_info();
				}
				else if( strcmp(szType, "CPS") == 0 )
				{
					gpIdx->dCpsIdx = dIdx;
					gpCurCpsCtrl = (dIdx == 0) ? gpCpsOvldCtrl[0] : gpCpsOvldCtrl[1];
					dAppLog(LOG_CRI, " * CPS OVLD READ     : IDX[%d", dIdx);
					prt_cps_info();
				}
				else if( strcmp(szType, "TIMEOUT") == 0 )
				{
					gpIdx->dTimeIdx = dIdx;
					gpCurMPTimer = (dIdx == 0) ? gpMPTimer[0] : gpMPTimer[1];
					dAppLog(LOG_CRI, " * TIMEOUT READ      : IDX[%d", dIdx);
					prt_timeout_info();
				}
			}
		}
		dIdx = 0; i++;
	}

	fclose(fa);

	return i;
} 

void dSetCurTrace(NOTIFY_SIG *pNOTISIG)
{
	if( pNOTISIG->stNoti.dTrcIdx < 0 || pNOTISIG->stNoti.dTrcIdx >= DEF_SET_CNT )
	{
		dAppLog(LOG_CRI, "[%s.%d] Current Index Range ERROR dTrcIdx[%d]", __FUNCTION__, __LINE__, pNOTISIG->stNoti.dTrcIdx);
		return;
		//gpCurTrc = NULL;
	}
	gpIdx->dTrcIdx = pNOTISIG->stNoti.dTrcIdx;
	gpCurTrc = gpTrcList[gpIdx->dTrcIdx];

	dAppLog(LOG_CRI, "NOTI] TRACE ACTIVE IDX[%d]", pNOTISIG->stNoti.dTrcIdx);
}

int dSetCurPdsn(NOTIFY_SIG *pNOTISIG)
{
	if( pNOTISIG->stNoti.dPdsnIdx < 0 || pNOTISIG->stNoti.dPdsnIdx >= DEF_SET_CNT ){
		//초기화시에는 이 부분을 탈 확률이 없다. 없다. modified by uamyd 20110507
		dAppLog(LOG_CRI, "[%s.%d] Current Index Range ERROR dPdsnIdx[%d]", __FUNCTION__, __LINE__, gpIdx->dPdsnIdx);
		return -1;
	}
	gpIdx->dPdsnIdx = pNOTISIG->stNoti.dPdsnIdx;
	gpIdx->dPdsnCnt = pNOTISIG->stNoti.dPdsnCnt;

	gpCurPdsn     = gpPdsnList[gpIdx->dPdsnIdx];
	gpCurPdsnHash = gpPdsnHash[gpIdx->dPdsnIdx];

	gpstTotStat[0]->stAcct.uiPDSN_Cnt = pNOTISIG->stNoti.dPdsnCnt;
	gpstTotStat[1]->stAcct.uiPDSN_Cnt = pNOTISIG->stNoti.dPdsnCnt;

	dAppLog(LOG_CRI, "NOTI] PDSN : IDX[%d] CNT[%d]", 
					pNOTISIG->stNoti.dPdsnIdx, pNOTISIG->stNoti.dPdsnCnt);
	return 0;
}

void dSetCurRSetList(NOTIFY_SIG *pNOTISIG)
{
	if( pNOTISIG->stNoti.dRsetListIdx < 0 || pNOTISIG->stNoti.dRsetListIdx >= DEF_SET_CNT )
	{
		dAppLog(LOG_CRI, "[%s.%d] Current Index Range ERROR dRsetListIdx[%d]", __FUNCTION__, __LINE__, pNOTISIG->stNoti.dRsetListIdx);
		return;
	//	gpCurRsetList = NULL;
	}
	gpIdx->dRsetListIdx = pNOTISIG->stNoti.dRsetListIdx;
	gpCurRsetList = gpRsetList[gpIdx->dRsetListIdx];

	dAppLog(LOG_CRI, "NOTI] RULESET LIST IDX[%d]", pNOTISIG->stNoti.dRsetListIdx);
}

void dSetCurRSetUsed(NOTIFY_SIG *pNOTISIG)
{
	if( pNOTISIG->stNoti.dRsetUsedIdx < 0 || pNOTISIG->stNoti.dRsetUsedIdx >= DEF_SET_CNT )
	{
		dAppLog(LOG_CRI, "[%s.%d] Current Index Range ERROR dRsetUsedIdx[%d]", __FUNCTION__, __LINE__, pNOTISIG->stNoti.dRsetUsedIdx);
		return;
		//gpCurRSetUsed = NULL;
	}
	gpIdx->dRsetUsedIdx = pNOTISIG->stNoti.dRsetUsedIdx;
	gpCurRSetUsed = gpRSetUsedList[gpIdx->dRsetUsedIdx];

	dAppLog(LOG_CRI, "NOTI] RULESET USED IDX[%d]", pNOTISIG->stNoti.dRsetUsedIdx);
}

void dSetCurCPSOvld(NOTIFY_SIG *pNOTISIG)
{
	if( pNOTISIG->stNoti.dCpsIdx < 0 || pNOTISIG->stNoti.dCpsIdx >= DEF_SET_CNT )
	{
		dAppLog(LOG_CRI, "[%s.%d] Current Index Range ERROR dCpsIdx[%d]", __FUNCTION__, __LINE__, pNOTISIG->stNoti.dCpsIdx);
		//gpCurCpsCtrl = NULL;
	}
	gpIdx->dCpsIdx = pNOTISIG->stNoti.dCpsIdx;
	gpCurCpsCtrl = gpCpsOvldCtrl[gpIdx->dCpsIdx];

	dAppLog(LOG_CRI, "NOTI] CPS OVLD IDX[%d]", pNOTISIG->stNoti.dCpsIdx);
}

void dSetCurTIMEOUT(NOTIFY_SIG *pNOTISIG)
{
	if( pNOTISIG->stNoti.dTimeIdx < 0 || pNOTISIG->stNoti.dTimeIdx >= DEF_SET_CNT )
	{
		dAppLog(LOG_CRI, "[%s.%d] Current Index Range ERROR dTimeIdx[%d]", __FUNCTION__, __LINE__, pNOTISIG->stNoti.dTimeIdx);
		return;
		//gpCurMPTimer = NULL;
	}
	gpIdx->dTimeIdx = pNOTISIG->stNoti.dTimeIdx;
	gpCurMPTimer = gpMPTimer[gpIdx->dTimeIdx];

	dAppLog(LOG_CRI, "NOTI] TIMEOUT IDX[%d]", pNOTISIG->stNoti.dTimeIdx);
}

int InitSHM_GENINFO(void)
{
	int 	dRet;
	int  	key;
	char 	tmp[64], fname[256];

	sprintf(fname,"%s", DEF_SYSCONF_FILE );

	if (conflib_getNthTokenInFileSection (fname, "[SHARED_MEMORY_KEY]", "S_SSHM_GENINFO", 1, tmp) < 0) {
		sprintf( szLogBuf, "CAN'T GET SHM KEY OF S_SSHM_GENINFO err=%s", strerror(errno));
		dAppWrite( LOG_CRI, szLogBuf );
		return -1;
	} else
		key = strtol(tmp, 0, 0);

	dRet = Init_GEN_INFO( key );
	if( dRet < 0 ) {
		sprintf( szLogBuf, "ERROR IN Init_MMDBSESS [RET:%d]", dRet );
		dAppWrite( LOG_CRI, szLogBuf );
		return -1;
	}

	return 0;
}


int InitSHM_LOC_SADB(void)
{
	int  	key, shmId;
	char 	tmp[64], fname[256];

	sprintf(fname,"%s", DEF_SYSCONF_FILE );

	/* INIT_SHM: SHM_LOC_SADB */
	if (conflib_getNthTokenInFileSection(fname, "[SHARED_MEMORY_KEY]", "SHM_LOC_SADB", 1, tmp) < 0)
		return -3;
	key = strtol(tmp, 0, 0);

	if( (shmId = (int)shmget(key, sizeof(SFM_SysCommMsgType), 0666 | IPC_CREAT)) < 0)
	{
		if(errno != ENOENT)
		{
			dAppLog(LOG_CRI, "[%s:%s:%d] shmget fail; key=0x%x, err=%d(%s)\n"
					, __FILE__, __FUNCTION__, __LINE__, key, errno, strerror(errno));
			return -4;
		}
	}
	if( (loc_sadb = (SFM_SysCommMsgType*)shmat(shmId, 0, 0)) == (SFM_SysCommMsgType*)-1)
	{
		dAppLog(LOG_CRI, "[%s:%s:%d] shmat fail; key=0x%x, err=%d(%s)\n"
				, __FILE__, __FUNCTION__, __LINE__, key, errno, strerror(errno));
		return -5;
	}
	return 0;
}

int InitSHM_LEG_STAT(void)
{
	int 	dRet;
	int  	key;
	char 	tmp[64], fname[256];

	sprintf(fname,"%s", DEF_SYSCONF_FILE );

	/** LEG_TOT_STAT 0 / 1 **/
	if (conflib_getNthTokenInFileSection (fname, "[SHARED_MEMORY_KEY]", "SHM_LEG_STAT", 1, tmp) < 0) {
		dAppLog( LOG_CRI, "CAN'T GET SHM KEY OF SHM_LEG_STAT err=%s", strerror(errno));
		return -6;
	}
	else
		key = strtol(tmp, 0, 0);

	dRet = Init_shm(key, DEF_LEG_TOT_STAT_SIZE, (void **)&gpstTotStat[0]);
	if( dRet < 0 )
	{
		dAppLog( LOG_CRI, "FAIL Init_shm(SHM_LEG_STAT) dRet=%d", dRet);
		return -7;
	}
	else if( dRet == 1 )
		memset(gpstTotStat[0], 0, DEF_LEG_TOT_STAT_SIZE);
	
	if (conflib_getNthTokenInFileSection (fname, "[SHARED_MEMORY_KEY]", "SHM_LEG_STAT_1", 1, tmp) < 0) {
		dAppLog( LOG_CRI, "CAN'T GET SHM KEY OF SHM_LEG_STAT_1 err=%s", strerror(errno));
		return -8;
	}
	else
		key = strtol(tmp, 0, 0);

	dRet = Init_shm(key, DEF_LEG_TOT_STAT_SIZE, (void **)&gpstTotStat[1]);
	if( dRet < 0 )
	{
		dAppLog( LOG_CRI, "FAIL Init_shm(SHM_LEG_STAT_1) dRet=%d", dRet);
		return -10;
	}
	else if( dRet == 1 )
		memset(gpstTotStat[1], 0, DEF_LEG_TOT_STAT_SIZE);

	//dAppLog(LOG_CRI, "Init gpstTotStat[0].stAcct.uiPDSN_Cnt = %d", gpstTotStat[0]->stAcct.uiPDSN_Cnt);
	//dAppLog(LOG_CRI, "Init gpstTotStat[1].stAcct.uiPDSN_Cnt = %d", gpstTotStat[1]->stAcct.uiPDSN_Cnt);
	return 0;
}

int InitSHM_CALL_DATA(void)
{
	int 	dRet;
	int  	key;
	char 	tmp[64], fname[256];

	sprintf(fname,"%s", DEF_SYSCONF_FILE );

	/** SHM_LEG_CPS 0 / 1 **/
	if (conflib_getNthTokenInFileSection (fname, "[SHARED_MEMORY_KEY]", "SHM_LEG_CPS", 1, tmp) < 0) {
		dAppLog( LOG_CRI, "CAN'T GET SHM KEY OF SHM_LEG_CPS err=%s", strerror(errno));
		return -11;
	}
	else
		key = strtol(tmp, 0, 0);

	dRet = Init_shm(key, DEF_LEG_DATA_SUM_SIZE, (void **)&gpstCallInfo[0]);
	if( dRet < 0 )
	{
		dAppLog( LOG_CRI, "FAIL Init_shm(SHM_LEG_CPS) dRet=%d", dRet);
		return -12;
	}
	else if( dRet == 1 )
		memset(gpstCallInfo[0], 0, DEF_LEG_DATA_SUM_SIZE);

	if (conflib_getNthTokenInFileSection (fname, "[SHARED_MEMORY_KEY]", "SHM_LEG_CPS_1", 1, tmp) < 0) {
		dAppLog( LOG_CRI, "CAN'T GET SHM KEY OF SHM_LEG_CPS_1 err=%s", strerror(errno));
		return -13;
	}
	else
		key = strtol(tmp, 0, 0);

	dRet = Init_shm(key, DEF_LEG_DATA_SUM_SIZE, (void **)&gpstCallInfo[1]);
	if( dRet < 0 )
	{
		dAppLog( LOG_CRI, "FAIL Init_shm(SHM_LEG_CPS) dRet=%d", dRet);
		return -14;
	}
	else if( dRet == 1 )
		memset(gpstCallInfo[1], 0, DEF_LEG_DATA_SUM_SIZE);

	// 이전 5sec 동안의 CPS와 TPS값을 저장해 놓기 위한 공유 메모리 설정.
	if (conflib_getNthTokenInFileSection (fname, "[SHARED_MEMORY_KEY]", "SHM_CALL_PER_SEC", 1, tmp) < 0) {
		dAppLog( LOG_CRI, "CAN'T GET SHM KEY OF SHM_CALL_PER_SEC err=%s\n", strerror(errno));
		return -15;
	}   
	else
		key = strtol(tmp, 0, 0); 

	dRet = Init_shm(key, DEF_LEG_CALL_DATA_SIZE, (void **)&gpstCallDataPerSec);
	if( dRet < 0 ) 
	{   
		dAppLog( LOG_CRI, "FAIL Init_shm(SHM_CALL_PER_SEC) dRet=%d\n", dRet);
		return -16;
	}   
	else if( dRet == 1 ) 
		memset(gpstCallDataPerSec, 0, DEF_LEG_CALL_DATA_SIZE);

	return 0;
}

int InitSHM_LEG_SESS_CNT(void)
{
	int  	key, shmId;
	char 	tmp[64], fname[256];

	sprintf(fname,"%s", DEF_SYSCONF_FILE );

	/* INIT_SHM: SHM_LEG_SESS_CNT */
	if (conflib_getNthTokenInFileSection(fname, "[SHARED_MEMORY_KEY]", "SHM_LEG_SESS_CNT", 1, tmp) < 0) {
		return -17;
	}
	else
		key = strtol(tmp, 0, 0);

	if((shmId = (int)shmget(key, sizeof(_mem_check), 0666 | IPC_CREAT | IPC_EXCL)) < 0)
	{
		if( errno == EEXIST ) {
			if((shmId = (int)shmget(key, sizeof(_mem_check), 0666 | IPC_CREAT)) < 0) {
				dAppLog(LOG_CRI, "[%s:%s:%d] EXIST: shmget fail; key=0x%x, err=%d(%s)\n"
						, __FILE__, __FUNCTION__, __LINE__, key, errno, strerror(errno));
				return -18;
			}
			if((gpShmem = (_mem_check *)shmat(shmId, 0, 0)) == (_mem_check *)-1) {
				dAppLog(LOG_CRI, "[%s:%s:%d] EXIST: shmat fail; key=0x%x, err=%d(%s)\n"
						, __FILE__, __FUNCTION__, __LINE__, key, errno, strerror(errno));
				return -19;
			}
		}
	}
	else {
		if( (gpShmem = (_mem_check *)shmat(shmId, 0, 0)) == (_mem_check *)-1)
		{
			dAppLog(LOG_CRI, "[%s:%s:%d] shmat fail; key=0x%x, err=%d(%s)\n"
					, __FILE__, __FUNCTION__, __LINE__, key, errno, strerror(errno));
			return -20;
		}
	}

	return 0;
}

int InitSHM_LEG_SESS(void)
{
	int  	key;
	char 	tmp[64], fname[256];

	sprintf(fname,"%s", DEF_SYSCONF_FILE );

	/* INIT_SHM: RAIUS HASH */
	if (conflib_getNthTokenInFileSection(fname, "[SHARED_MEMORY_KEY]", "SHM_LEG_SESS", 1, tmp) < 0)
		return -20;

	key = strtol(tmp, 0, 0);
	init_session (key);

	return 0;
}

int InitSHM_TRACE(void)
{
	int 	dRet;
	int  	key;
	char 	tmp[64], fname[256];

	sprintf(fname,"%s", DEF_SYSCONF_FILE );

	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_TRACE", 1, tmp) < 0)
		return -1;

	key = strtol(tmp,0,0);
	dRet = Init_shm(key, sizeof(st_SESSInfo), (void **)&gpTrcList[0]);
	if( dRet < 0 )
	{
		dAppLog( LOG_CRI, "FAIL %s(0) dRet=%d", __FUNCTION__, dRet);
		return -1;
	}
	else if( dRet == 1 )
		memset(gpTrcList[0], 0, sizeof(st_SESSInfo));

	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_TRACE1", 1, tmp) < 0)
		return -1;

	key = strtol(tmp,0,0);
	dRet = Init_shm(key, sizeof(st_SESSInfo), (void **)&gpTrcList[1]);
	if( dRet < 0 )
	{
		dAppLog( LOG_CRI, "FAIL %s(1) dRet=%d", __FUNCTION__, dRet);
		return -1;
	}
	else if( dRet == 1 )
		memset(gpTrcList[1], 0, sizeof(st_SESSInfo));
	return 0;
}

int InitSHM_PDSN_LIST(void)
{
	int 	dRet;
	int  	key;
	char 	tmp[64], fname[256];

	sprintf(fname,"%s", DEF_SYSCONF_FILE );

	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_PDSN_LIST", 1, tmp) < 0)
		return -1;

	key = strtol(tmp,0,0);
	dRet = Init_shm(key, sizeof(PDSN_LIST), (void **)&gpPdsnList[0]);
	if( dRet < 0 )
	{
		dAppLog( LOG_CRI, "FAIL %s(0) dRet=%d", __FUNCTION__, dRet);
		return -1;
	}

	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_PDSN_LIST1", 1, tmp) < 0)
		return -1;

	key = strtol(tmp,0,0);
	dRet = Init_shm(key, sizeof(PDSN_LIST), (void **)&gpPdsnList[1]);
	if( dRet < 0 )
	{
		dAppLog( LOG_CRI, "FAIL %s(1) dRet=%d", __FUNCTION__, dRet);
		return -1;
	}
	return 0;
}

int InitSHM_PDSN_HASH(void)
{
	int  	key;
	char 	tmp[64], fname[256];

	sprintf(fname,"%s", DEF_SYSCONF_FILE );

	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_PDSN_HASH", 1, tmp) < 0)
		return -1;

	key = strtol(tmp,0,0);
	if((gpPdsnHash[0] = hasho_init(key, DEF_PDSN_HASH_KEY_SIZE, DEF_PDSN_HASH_KEY_SIZE, DEF_PDSN_HASH_DATA_SIZE, MAX_HASH_PDSN_CNT, NULL, 0)) == NULL) 
	{
		dAppLog(LOG_CRI, "PDSN HASH hasho_init() NULL");
		return -1;
	}

	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_PDSN_HASH1", 1, tmp) < 0)
		return -1;

	key = strtol(tmp,0,0);
	if((gpPdsnHash[1] = hasho_init(key, DEF_PDSN_HASH_KEY_SIZE, DEF_PDSN_HASH_KEY_SIZE, DEF_PDSN_HASH_DATA_SIZE, MAX_HASH_PDSN_CNT, NULL, 0)) == NULL) 
	{
		dAppLog(LOG_CRI, "PDSN HASH hasho_init() NULL");
		return -1;
	}

	return 0;
}

int InitSHM_RSET_LIST(void)
{
	int 	dRet;
	int  	key;
	char 	tmp[64], fname[256];

	sprintf(fname,"%s", DEF_SYSCONF_FILE );

	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_RSET_LIST", 1, tmp) < 0)
		return -1;

	key = strtol(tmp,0,0);
	dRet = Init_shm(key, sizeof(ST_PBTABLE_LIST), (void **)&gpRsetList[0]);
	if( dRet < 0 )
	{
		dAppLog( LOG_CRI, "FAIL %s(0) dRet=%d", __FUNCTION__, dRet);
		return -1;
	}

	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_RSET_LIST1", 1, tmp) < 0)
		return -1;

	key = strtol(tmp,0,0);
	dRet = Init_shm(key, sizeof(ST_PBTABLE_LIST), (void **)&gpRsetList[1]);
	if( dRet < 0 )
	{
		dAppLog( LOG_CRI, "FAIL %s(1) dRet=%d", __FUNCTION__, dRet);
		return -1;
	}
	return 0;
}

int InitSHM_RSET_USED(void)
{
	int 	dRet;
	int  	key;
	char 	tmp[64], fname[256];

	sprintf(fname,"%s", DEF_SYSCONF_FILE );

	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_RSET_USED", 1, tmp) < 0)
		return -1;

	key = strtol(tmp,0,0);
	dRet = Init_shm(key, sizeof(RULESET_USED_FLAG), (void **)&gpRSetUsedList[0]);
	if( dRet < 0 )
	{
		dAppLog( LOG_CRI, "FAIL %s(0) dRet=%d", __FUNCTION__, dRet);
		return -1;
	}

	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_RSET_USED1", 1, tmp) < 0)
		return -1;

	key = strtol(tmp,0,0);
	dRet = Init_shm(key, sizeof(RULESET_USED_FLAG), (void **)&gpRSetUsedList[1]);
	if( dRet < 0 )
	{
		dAppLog( LOG_CRI, "FAIL %s(1) dRet=%d", __FUNCTION__, dRet);
		return -1;
	}
	return 0;
}

int InitSHM_CPS_OVLD(void)
{
	int 	dRet;
	int  	key;
	char 	tmp[64], fname[256];

	sprintf(fname,"%s", DEF_SYSCONF_FILE );

	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_CPS_OVLD", 1, tmp) < 0)
		return -1;

	key = strtol(tmp,0,0);
	dRet = Init_shm(key, sizeof(CPS_OVLD_CTRL), (void **)&gpCpsOvldCtrl[0]);
	if( dRet < 0 )
	{
		dAppLog( LOG_CRI, "FAIL %s(0) dRet=%d", __FUNCTION__, dRet);
		return -1;
	}

	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_CPS_OVLD1", 1, tmp) < 0)
		return -1;

	key = strtol(tmp,0,0);
	dRet = Init_shm(key, sizeof(CPS_OVLD_CTRL), (void **)&gpCpsOvldCtrl[1]);
	if( dRet < 0 )
	{
		dAppLog( LOG_CRI, "FAIL %s(1) dRet=%d", __FUNCTION__, dRet);
		return -1;
	}
	return 0;
}

int InitSHM_TIMER(void)
{
	int 	dRet;
	int  	key;
	char 	tmp[64], fname[256];

	sprintf(fname,"%s", DEF_SYSCONF_FILE );

	/* INIT_SHM: TIMER0 */
	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_TIMER", 1, tmp) < 0)
		return -1;

	key = strtol(tmp,0,0);
	dRet = Init_shm(key, sizeof(MPTimer), (void **)&gpMPTimer[0]);
	if( dRet < 0 )
	{
		dAppLog( LOG_CRI, "FAIL %s(0) dRet=%d", __FUNCTION__, dRet);
		return -1;
	}

	/* INIT_SHM: TIMER1 */
	if (conflib_getNthTokenInFileSection (fname, "SHARED_MEMORY_KEY", "SHM_TIMER1", 1, tmp) < 0)
		return -1;

	key = strtol(tmp,0,0);
	dRet = Init_shm(key, sizeof(MPTimer), (void **)&gpMPTimer[1]);
	if( dRet < 0 )
	{
		dAppLog( LOG_CRI, "FAIL %s(1) dRet=%d", __FUNCTION__, dRet);
		return -1;
	}
	return 0;
}


int InitMSGQ(void)
{
	int  	i, key;
	char 	tmp[64], fname[256], pname[8];

	sprintf(fname,"%s", DEF_SYSCONF_FILE );

	if (conflib_getNthTokenInFileSection (fname, "[APPLICATIONS]", "RANA", 1, tmp) < 0) {
		dAppLog (LOG_CRI, "[cdr_init] can't get queue key of RANA_MMC err=%s\n", strerror(errno));
		return -1;
	} else
		key = strtol(tmp, 0, 0);

	dMyQid = Init_msgq( key );
	if( dMyQid < 0 ) {
		dAppLog(LOG_CRI, "ERROR IN INITIAL S_MSGQ_RANA_MMC [RET:%d]", dMyQid);
		exit(1);
	}

    if (conflib_getNthTokenInFileSection (fname, "[APPLICATIONS]", "IXPC", 1, tmp) < 0) {
        dAppLog( LOG_CRI, "[init_ipcs] CAN'T GET MSGQ KEY OF IXPC err=%s", strerror(errno));
        return -5;
    } else 
        key = strtol(tmp, 0, 0);
    if ((dIxpcQid = msgget(key,IPC_CREAT|0666)) < 0) {
            dAppLog( LOG_CRI, "[init_ipcs] IXPC msgget fail; key=0x%x,err=%d(%s)", key, errno, strerror(errno));
        return -6;
    }

	for( i = 0; i < MAX_RLEG_CNT; i++ )
	{
		/* S_MSGQ_SESSANA */
		sprintf(pname, "RLEG%d", i);
		if (conflib_getNthTokenInFileSection (fname, "[APPLICATIONS]", pname, 1, tmp) < 0) {
			dAppLog (LOG_CRI, "[cdr_init] can't get queue key of RLEG err=%s\n", strerror(errno));
			return -7;
		}
		else
			key = strtol(tmp, 0, 0);

		dTRCDRQid[i] = Init_msgq( key );
		if( dTRCDRQid[i] < 0 ) {
			dAppLog(LOG_CRI, "ERROR IN INITIAL S_MSGQ_SESSANA [PROC:RLEG QID:%d]", dTRCDRQid[i]);
			return -8;
		}
	}
	return 0;
}

/*******************************************************************************

*******************************************************************************/
int dINIT_RANA_IPCS()
{
	int 	dRet;
	char 	tmp[64], fname[256];

	if( (pstMEMSINFO = nifo_init_zone("RANA", SEQ_PROC_RANA, DEF_NIFO_ZONE_CONF_FILE)) == NULL )
	{
		dAppLog( LOG_CRI, "ERROR nifo_init_zone NULL");
		return -17;
	}

	sprintf(fname,"%s", DEF_SYSCONF_FILE );
#if 1
	if (conflib_getNthTokenInFileSection (fname, "[GENERAL]", "SYSTEM_LABEL", 1, tmp) < 0) {
		dAppLog (LOG_CRI, "[dINIT_RANA_IPCS] CAN'T GET SYSTEM LABEL err=%s", strerror(errno));
		return -1;
	}
	else {
		strcpy(sysLable, tmp);
		strcpy(mySysName, sysLable);
		strcpy(myAppName, "RANA");
		dAppLog(LOG_CRI, "SYSTEM LABEL=[%s", sysLable);
	}
#endif
	/* INIT SHM : GENINFO */
	if( InitSHM_GENINFO() < 0 ) {
		sprintf( szLogBuf, "ERROR InitSHM_GENINFO() FAIL");
		dAppWrite( LOG_CRI, szLogBuf );
		return -1;
	}

	if( InitSHM_LOC_SADB() < 0 ) {
		sprintf( szLogBuf, "ERROR InitSHM_LOC_SADB() FAIL");
		dAppWrite( LOG_CRI, szLogBuf );
		return -2;
	}

	if( InitSHM_LEG_STAT() < 0 ) {
		dAppLog( LOG_CRI, "ERROR InitSHM_LEG_STAT() FAIL");
		return -3;
	}

	if( InitSHM_CALL_DATA() < 0 ) {
		dAppLog( LOG_CRI, "ERROR InitSHM_CALL_DATA() FAIL");
		return -4;
	}

	if( InitSHM_LEG_SESS_CNT() < 0 ) {
		dAppLog( LOG_CRI, "ERROR InitSHM_LEG_SESS_CNT() FAIL");
		return -5;
	}

	/* MMCR NOTI Shared Memory Init */
	if( InitSHM_TRACE() < 0 ) {
		dAppLog( LOG_CRI, "ERROR InitSHM_TRACE() FAIL");
		return -7;
	}

	if( InitSHM_PDSN_LIST() < 0 ) {
		dAppLog( LOG_CRI, "ERROR InitSHM_PDSN_LIST() FAIL");
		return -8;
	}

	if( InitSHM_PDSN_HASH() < 0 ) {
		dAppLog( LOG_CRI, "ERROR InitSHM_PDSN_HASH() FAIL");
		return -9;
	}

	if( InitSHM_RSET_LIST() < 0 ) {
		dAppLog( LOG_CRI, "ERROR InitSHM_RSET_LIST() FAIL");
		return -10;
	}

	if( InitSHM_RSET_USED() < 0 ) {
		dAppLog( LOG_CRI, "ERROR InitSHM_RSET_USED() FAIL");
		return -11;
	}

	if( InitSHM_CPS_OVLD() < 0 ) {
		dAppLog( LOG_CRI, "ERROR InitSHM_CPS_OVLD() FAIL");
		return -12;
	}

	if( InitSHM_TIMER() < 0 ) {
		dAppLog( LOG_CRI, "ERROR InitSHM_TIMER() FAIL");
		return -13;
	}

	dRet = dReadFLTIDXFile();
	if( dRet < 0 ) {
		dAppLog( LOG_CRI, "dReadFLTIDXFile() FAIL RET:%d", dRet);
		return -14;
	}


	if( InitSHM_LEG_SESS() < 0 ) {
		dAppLog( LOG_CRI, "ERROR InitSHM_LEG_SESS() FAIL");
		return -15;
	}

	if( InitMSGQ() < 0 ) {
		dAppLog( LOG_CRI, "ERROR InitMSGQ() FAIL");
		return -16;
	}

	/* SCM INFO */
	if ((dRet = dGetConfig_LEG ()) < 0) {
		dAppLog(LOG_CRI, "dGetConfig_LEG FAIL dRet=%d", dRet);
		return -18;
	}

	return 1;
}

