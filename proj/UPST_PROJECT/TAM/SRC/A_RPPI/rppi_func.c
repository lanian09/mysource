#include <stdlib.h>
#include <sys/time.h>		/* gettimeofday() */

#include "msgdef.h"			/* MID_XX */
#include "loglib.h"
#include "utillib.h"

#include "watch_filter.h"	/* st_WatchFilter */
#include "filter.h"			/* st_Flt_Info */

#include "rppi_func.h"
#include "rppi_init.h"
#include "rppi_msgq.h"
#include "rppi_util.h"

st_Flt_Info			*flt_info;
st_WatchFilter 		*gWatchFilter;

extern stMEMSINFO   *pMEMSINFO;
extern stHASHOINFO  *pHASHOINFO; 
extern stTIMERNINFO *pTIMERNINFO;

void vCheckFirst(HData_RPPI *pHASH, LOG_RPPI *pLOG, U8 *str, U32 SetupTime, U32 SetupMTime)
{
	S32				isSend = 0;
	S32				isReCall = 0;
	S32				dState = 0;
	U8      		szTime[BUFSIZ];
	
	OFFSET	offset = nifo_get_offset_node(pMEMSINFO, (U8*)pLOG);
	isReCall = GET_ISRECALL(pLOG->usCallType);
	dState = (isReCall ? RECALL_ON_SERVICE_STATE : ON_SERVICE_STATE);

	if(pHASH->before.dOffset == offset) {
		if(pHASH->before.uiFirstServFlag == 0) {
			log_print(LOGN_WARN, "PBG1 NOT FIRST IMSI=%s CALLTIME=%s.%u STR=%s", 
				pLOG->szIMSI, util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime, str);
			pHASH->before.uiFirstServFlag = 1;
			isSend = 1;

			pLOG->usCallState = dGetCallState(pLOG->usCallState, dState);

			/* IPCP PPP_SETUP이 없는 경우 First Service 시간을 세팅 함 : First Service 시간은 예외적으로 CallTime을 사용 */
			if(pLOG->uiPPPSetupTime == 0) {
				pLOG->uiPPPSetupTime = SetupTime;
				pLOG->uiPPPSetupMTime = SetupMTime;
			}
		}
	} 
	else if(pHASH->after.dOffset == offset) {
		if(pHASH->after.uiFirstServFlag == 0) {
			log_print(LOGN_WARN, "PBG2 NOT FIRST IMSI=%s CALLTIME=%s.%u STR=%s", 
				pLOG->szIMSI, util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime, str);
			pHASH->after.uiFirstServFlag = 1;
			isSend = 1;

			pLOG->usCallState = dGetCallState(pLOG->usCallState, dState);


			/* IPCP PPP_SETUP이 없는 경우 First Service 시간을 세팅 함 : First Service 시간은 예외적으로 CallTime을 사용 */
			if(pLOG->uiPPPSetupTime == 0) {
				pLOG->uiPPPSetupTime = SetupTime;
				pLOG->uiPPPSetupMTime = SetupMTime;
			}
		}
	}
	else {
		log_print(LOGN_CRI, "PBG3 NOT FIRST IMSI=%s CALLTIME=%s.%u STR=%s", 
			pLOG->szIMSI, util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime, str);
	}

	if(isSend) {
		/** WATCH DATA */
		st_WatchMsg stWatchMsg;
		memset(&stWatchMsg, 0x00, DEF_WATCHMSG_SIZE);

		if (isReCall) {
			stWatchMsg.usMsgType = WATCH_TYPE_RECALL;
		}
		else {
			stWatchMsg.usMsgType = WATCH_TYPE_A11;
		}
		stWatchMsg.ucOffice = pLOG->ucBranchID;
		stWatchMsg.ucSYSID = pLOG->ucSYSID;
		stWatchMsg.ucBSCID = pLOG->ucBSCID;
		stWatchMsg.usBTSID = pLOG->ucBTSID;
		stWatchMsg.ucSec = pLOG->ucSECTOR;
		stWatchMsg.ucFA = pLOG->ucFA_ID;
		stWatchMsg.uiPCFIP = pLOG->uiPCFIP;
		stWatchMsg.ucPCFType = dGetPCFType(pLOG->uiPCFIP);
		stWatchMsg.uiPDSNIP = pLOG->uiNASName;
		stWatchMsg.uiAAAIP = pLOG->uiAAAIP;
		stWatchMsg.uiHSSIP = pLOG->uiHSSIP;
		stWatchMsg.ucSvcIdx = 0;
		stWatchMsg.usSvcL4Type = 0;
		stWatchMsg.uiSVCIP = 0;
		stWatchMsg.uiResult = 0;

		dSendMonInfo(&stWatchMsg);
		log_print(LOGN_INFO, "SEND WATCH MSG[%d][%s] OFFICE[%d][%s] RESULT[%d]",
			stWatchMsg.usMsgType, PrintMonType(stWatchMsg.usMsgType),
			stWatchMsg.ucOffice, PrintOFFICE(stWatchMsg.ucOffice), stWatchMsg.uiResult);
	}
}

S32 dCallStartInfo(S32 type, U8 *data)
{
	S64             llGapTime;	
	RPPISESS_KEY	stRPPIKey, *pstRPPIKey;
	HData_RPPI		stRPPIHash, *pstRPPIHash;

	LOG_SIGNAL		*pstSIGNAL;
	stHASHONODE		*pHASHONODE;	
	LOG_RPPI		*pstRPPILog;
	U8			*pstLogNode;

	RPPI_TIMER		RPPITIMER;

	pstRPPIKey = &stRPPIKey;
	pstRPPIHash = &stRPPIHash;

	U8              szIP[INET_ADDRSTRLEN];
	U8      		szTime[BUFSIZ];
	S32				isReCall;

	memset(pstRPPIHash, 0x00, HDATA_RPPI_SIZE);
	pstSIGNAL = (LOG_SIGNAL*)data;

	memset(pstRPPIKey, 0x00, RPPISESS_KEY_SIZE);	
	memcpy(pstRPPIKey->szIMSI, pstSIGNAL->szIMSI, MAX_MIN_SIZE);
	
	isReCall = dGetIsRecall(type);

	log_print (LOGN_DEBUG, "RCV %s IMSI[%s] IP[%s] TIME[%s.%u] ReCall[%d]", 
			((isReCall) ? "RECALLSTART" : "CALLSTART"),
			pstSIGNAL->szIMSI, util_cvtipaddr(szIP, pstSIGNAL->uiClientIP), 
			util_printtime(pstSIGNAL->uiCallTime, szTime), pstSIGNAL->uiCallMTime, isReCall);
	

	if ( (pHASHONODE = hasho_find(pHASHOINFO, (U8*)pstRPPIKey)) )
	{
		pstRPPIHash = (HData_RPPI*)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);
		STG_DiffTIME64(pstSIGNAL->uiCallTime, pstSIGNAL->uiCallMTime, pstRPPIHash->before.uiCallTime, pstRPPIHash->before.uiCallMTime, &llGapTime);	
		if(llGapTime == 0)
		{
			log_print(LOGN_CRI, "CALL DUPLICATE BEFORE IMSI[%s] Time[%u.%u] CallStart Time[%u.%u] ReCall[%d]",
					pstSIGNAL->szIMSI, pstRPPIHash->before.uiCallTime, pstRPPIHash->before.uiCallMTime,
					pstSIGNAL->uiCallTime, pstSIGNAL->uiCallMTime, isReCall);
			return -1;	
		}	
		if (pstRPPIHash->after.dOffset !=0 )
		{
			STG_DiffTIME64(pstSIGNAL->uiCallTime, pstSIGNAL->uiCallMTime, pstRPPIHash->after.uiCallTime, pstRPPIHash->after.uiCallMTime, &llGapTime);
			if(llGapTime == 0)
			{
				log_print(LOGN_CRI, "CALL DUPLICATE AFTER IMSI[%s] Time[%u.%u] CallStart Time[%u.%u] ReCall[%d]",
						pstSIGNAL->szIMSI, pstRPPIHash->after.uiCallTime, pstRPPIHash->after.uiCallMTime,
						pstSIGNAL->uiCallTime, pstSIGNAL->uiCallMTime, isReCall);
				return -1; 
			}	
			/*Before 정리 */
			log_print(LOGN_DEBUG, "Before Hash Delete Before IMSI[%s] Time[%u.%u] After Time[%u.%u] ReCall[%d]",
					pstSIGNAL->szIMSI, pstRPPIHash->before.uiCallTime, pstRPPIHash->before.uiCallMTime,
					pstRPPIHash->before.uiCallTime, pstRPPIHash->before.uiCallMTime, isReCall);
			pstRPPILog = (LOG_RPPI*)nifo_get_value(pMEMSINFO, LOG_RPPI_DEF_NUM, pstRPPIHash->before.dOffset);
	
			dProcCallStop(pstRPPIHash, pstRPPILog, pstRPPIHash->before.uiFirstServFlag);
			if (pstRPPILog->uiLastFailReason == 0 ) {
				pstRPPILog->uiLastFailReason = A11_DEFECT + CALL_SETUP + ERR_CALL_DUPLICATE;
//				pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(pstRPPILog->usCallState, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
			}
			dDelRPPI(pstRPPIHash, pstRPPILog);			
		}		
	}

	else
	{
		/** ADD HASH  **/
		if ( (pHASHONODE = hasho_add(pHASHOINFO, (U8*)pstRPPIKey, (U8*)pstRPPIHash)) == NULL )
		{
			log_print (LOGN_CRI, "[%s][%s.%d] hasho_add NULL IMSI[%s] TIME[%s.%u]", 
					__FILE__, __FUNCTION__, __LINE__,
					pstSIGNAL->szIMSI, util_printtime(pstSIGNAL->uiCallTime, szTime), pstSIGNAL->uiCallMTime);
			return -1;
		}
		else
		{
			pstRPPIHash = (HData_RPPI*) nifo_ptr (pHASHOINFO, pHASHONODE->offset_Data);
//			log_print (LOGN_DEBUG, "ADD HASH IMSI:%s", pstRPPIKey->szIMSI);
		}

	}

	if ( (pstLogNode = nifo_node_alloc(pMEMSINFO)) == NULL)
	{
		log_print (LOGN_CRI, "[%s][%s.%d] nifo_node_alloc NULL IMSI[%s] TIME[%s.%u] ReCall[%d]", 
				__FILE__, __FUNCTION__, __LINE__,
				pstSIGNAL->szIMSI, util_printtime(pstSIGNAL->uiCallTime, szTime), pstSIGNAL->uiCallMTime, isReCall);
		if((pstRPPIHash->before.dOffset == 0) && (pstRPPIHash->after.dOffset == 0)) 
            hasho_del(pHASHOINFO, (U8 *)pstRPPIKey);
		return -1;
	}

	if ( (pstRPPILog = (LOG_RPPI*) nifo_tlv_alloc(pMEMSINFO, pstLogNode, LOG_RPPI_DEF_NUM, LOG_RPPI_SIZE, DEF_MEMSET_ON)) == NULL)
	{
		log_print (LOGN_CRI, "[%s][%s.%d] nifo_tlv_alloc NULL IMSI[%s] TIME[%s.%u] ReCall[%d]", 
				__FILE__, __FUNCTION__, __LINE__,
				pstSIGNAL->szIMSI, util_printtime(pstSIGNAL->uiCallTime, szTime), pstSIGNAL->uiCallMTime, isReCall);
		if((pstRPPIHash->before.dOffset == 0) && (pstRPPIHash->after.dOffset == 0)) hasho_del(pHASHOINFO, (U8 *)pstRPPIKey);
		nifo_node_delete(pMEMSINFO, pstLogNode);
		return -1;
	}

	struct timeval  stNowTime;
	gettimeofday(&stNowTime, NULL);
	pstRPPILog->uiOpStartTime = stNowTime.tv_sec; 
	pstRPPILog->uiOpStartMTime = stNowTime.tv_usec;

	memcpy(pstRPPILog->szIMSI, pstSIGNAL->szIMSI, MAX_MIN_SIZE); 
	pstRPPILog->uiCallTime = pstSIGNAL->uiCallTime;	
	pstRPPILog->uiCallMTime = pstSIGNAL->uiCallMTime;

	dGetModelInfo(pstSIGNAL->szIMSI, pstRPPILog->szModel);
	dGetMINInfo(pstSIGNAL->szIMSI, pstRPPILog->szMIN);

	pstRPPILog->uiSetupFailReason = SETUP_SUCESS;
	pstRPPILog->uiLastFailReason = 0;

	if (isReCall) {
		dUpdateReCallRPPI(pstRPPILog, pstSIGNAL, pstRPPIHash);	
	}
	else {
		dUpdateRPPI(pstRPPILog, pstSIGNAL, pstRPPIHash);	
	}

	memset(&RPPITIMER, 0x00, sizeof(RPPI_TIMER));
	memcpy(&RPPITIMER.RPPIKEY, pstRPPIKey->szIMSI, MAX_MIN_SIZE);
	RPPITIMER.uiCallTime = pstSIGNAL->uiCallTime;
	RPPITIMER.uiCallMTime = pstSIGNAL->uiCallMTime;

	if (pstRPPIHash->before.dOffset == 0)
	{
		pstRPPIHash->before.dOffset = nifo_get_offset_node(pMEMSINFO, (U8*)pstRPPILog);	
		pstRPPIHash->before.timerNID = timerN_add (pTIMERNINFO,  invoke_del_call, (U8*)&RPPITIMER, sizeof(RPPI_TIMER), time(NULL) + flt_info->stTimerInfo.usTimerInfo[RPPI_CALL_TIMEOUT]);
		pstRPPIHash->before.uiCallTime = pstSIGNAL->uiCallTime;
		pstRPPIHash->before.uiCallMTime = pstSIGNAL->uiCallMTime;

	}
	else
	{
		pstRPPIHash->after.dOffset = nifo_get_offset_node(pMEMSINFO, (U8*)pstRPPILog); 
		pstRPPIHash->after.timerNID = timerN_add (pTIMERNINFO,  invoke_del_call, (U8*)&RPPITIMER, sizeof(RPPI_TIMER), time(NULL) + flt_info->stTimerInfo.usTimerInfo[RPPI_CALL_TIMEOUT]);
		pstRPPIHash->after.uiCallTime = pstSIGNAL->uiCallTime;
		pstRPPIHash->after.uiCallMTime = pstSIGNAL->uiCallMTime;
	}	

	return 0;
}

S32 dCallStopInfo(S32 type, U8 *data)
{
	RPPISESS_KEY    stRPPIKey, *pstRPPIKey;
	HData_RPPI      stRPPIHash, *pstRPPIHash;
	LOG_SIGNAL      *pstSIGNAL;
	stHASHONODE     *pHASHONODE;
	LOG_RPPI        *pstRPPILog;
	struct timeval	stTime;

	pstRPPIKey = &stRPPIKey;
	pstRPPIHash = &stRPPIHash;

	U8              szIP[INET_ADDRSTRLEN];
	U8              szTime[BUFSIZ];
	S32				isReCall;

	pstSIGNAL = (LOG_SIGNAL*)data;

	memset(pstRPPIKey, 0x00, RPPISESS_KEY_SIZE); 
	memcpy(pstRPPIKey->szIMSI, pstSIGNAL->szIMSI, MAX_MIN_LEN);
	
	isReCall = dGetIsRecall(type);

	log_print (LOGN_DEBUG, "RCV %s IMSI[%s] IP[%s] TIME[%s.%u] ReCall[%d]", 
			PrintStopType(type), pstSIGNAL->szIMSI,
			util_cvtipaddr(szIP, pstSIGNAL->uiClientIP), 
			util_printtime(pstSIGNAL->uiCallTime, szTime), pstSIGNAL->uiCallMTime, isReCall);
//	LOG_SIGNAL_Prt("PRINT LOG_CALL_STOP", pstSIGNAL);
	

	if ( (pHASHONODE = hasho_find(pHASHOINFO, (U8*)pstRPPIKey)) )
	{
		pstRPPIHash = (HData_RPPI*)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);
//		log_print(LOGN_DEBUG, "Before Offset[%ld] After Offset[%ld]", pstRPPIHash->before.dOffset, pstRPPIHash->after.dOffset);

		pstRPPILog = pFindRPPILog(isReCall, pstRPPIHash, pstSIGNAL->uiCallTime, pstSIGNAL->uiCallMTime);
		if (pstRPPILog == NULL)
		{
//			log_print(LOGN_CRI, "CALLSTIP NOT FOUND RPPI LOG IMSI[%s] IP[%s] TIME[%s.%u]", 
			log_print(LOGN_WARN, "CALLSTOP NOT FOUND RPPI LOG IMSI[%s] IP[%s] TIME[%s.%u] ReCall[%d]", 
					pstSIGNAL->szIMSI, util_cvtipaddr(szIP, pstSIGNAL->uiClientIP),
					util_printtime(pstSIGNAL->uiCallTime, szTime), pstSIGNAL->uiCallMTime, isReCall);
			return -1;
		}

		if(type != STOP_PI_RECALL_NUM)
		{
			pstRPPILog->uiUpGREPkts = pstSIGNAL->uiUpGREPkts;
			pstRPPILog->uiDnGREPkts = pstSIGNAL->uiDnGREPkts;
			pstRPPILog->uiUpGREBytes = pstSIGNAL->uiUpGREBytes;
			pstRPPILog->uiDnGREBytes = pstSIGNAL->uiDnGREBytes;
		}

#if 0		
		pstRPPILog->uiReleaseTime = pstSIGNAL->uiSessEndTime;
		pstRPPILog->uiReleaseMTime = pstSIGNAL->uiSessEndMTime;
#endif
		gettimeofday(&stTime, NULL);
		pstRPPILog->uiReleaseTime = stTime.tv_sec;
		pstRPPILog->uiReleaseMTime = stTime.tv_usec;


		if (pstRPPIHash->before.dOffset == nifo_get_offset_node(pMEMSINFO, (U8*)pstRPPILog))
		{
			log_print(LOGN_DEBUG, "BEFORE TIMER[%lld] ReCall[%d] UPDATE BECAUSE OF ACTIVE STOP", pstRPPIHash->before.timerNID, isReCall);
			pstRPPIHash->before.timerNID = timerN_update(pTIMERNINFO, pstRPPIHash->before.timerNID, time(NULL) + flt_info->stTimerInfo.usTimerInfo[RPPI_WAIT_TIMEOUT]);
			pstRPPIHash->before.uiTimerStop = 1;
		}
		else if (pstRPPIHash->after.dOffset == nifo_get_offset_node(pMEMSINFO, (U8*)pstRPPILog))
		{
			log_print(LOGN_DEBUG, "AFTER TIMER[%lld] ReCall[%d] UPDATE BECAUSE OF ACTIVE STOP", pstRPPIHash->after.timerNID, isReCall);
			pstRPPIHash->after.timerNID = timerN_update(pTIMERNINFO, pstRPPIHash->after.timerNID, time(NULL) + flt_info->stTimerInfo.usTimerInfo[RPPI_WAIT_TIMEOUT]);
			pstRPPIHash->after.uiTimerStop = 1;
		}
		else {
			log_print(LOGN_CRI, "[%s][%s.%d] NOT FOUND OFFSET IMSI[%s]  ClientIP[%s] ReCall[%d]", __FILE__, __FUNCTION__, __LINE__,
				pstRPPILog->szIMSI, util_cvtipaddr(szIP, pstRPPILog->uiClientIP), isReCall);
			return -1;	
		}

		pstRPPILog->stopFlag = pstSIGNAL->uiRespCode;
		pstRPPILog->stateFlag = RP_END_STATE;
		
		if (isReCall) {
			pstRPPILog->stateFlag = RECALL_PI_END_STATE;
			dUpdateReCallRPPI(pstRPPILog, pstSIGNAL, pstRPPIHash);
		}
//		dProcCallStop(pstRPPIHash, pstRPPILog, pstSIGNAL->uiRespCode);	
	}                       

	else                    
	{                   
//		log_print(LOGN_CRI, "CALLSTOP NOT FOUND RPPI LOG HASH IMSI[%s] TIME[%s.%u]", 
		log_print(LOGN_WARN, "CALLSTOP NOT FOUND RPPI LOG HASH IMSI[%s] TIME[%s.%u] ReCall[%d]", 
				pstSIGNAL->szIMSI, util_printtime(pstSIGNAL->uiCallTime, szTime), pstSIGNAL->uiCallMTime, isReCall);
		return -1;          
	}                       
	return 0;
}

S32 dSignalInfo(U8 *data)
{
	RPPISESS_KEY    stRPPIKey, *pstRPPIKey;
	HData_RPPI      stRPPIHash, *pstRPPIHash;
	LOG_SIGNAL      *pstSIGNAL;
	stHASHONODE     *pHASHONODE;
	LOG_RPPI        *pstRPPILog;

	pstRPPIKey = &stRPPIKey; 
	pstRPPIHash = &stRPPIHash;

	U8              szIP[INET_ADDRSTRLEN];
	U8              szPCFIP[INET_ADDRSTRLEN];
	U8              szPDSNIP[INET_ADDRSTRLEN];
	U8              szTime[BUFSIZ];	
	S32				isReCall = 0;

	pstSIGNAL = (LOG_SIGNAL*)data;

	memset(pstRPPIKey, 0x00, RPPISESS_KEY_SIZE); 
	memcpy(pstRPPIKey->szIMSI, pstSIGNAL->szIMSI, MAX_MIN_SIZE);
	
	log_print (LOGN_DEBUG, "RCV [%s:%u:%u] IMSI[%s] IP[%s] ERROR[%d] TIME[%s.%u] PCFIP[%s] PDSNIP[%s]", 
			PrintMsg(pstSIGNAL), pstSIGNAL->uiProtoType, pstSIGNAL->uiMsgType, pstSIGNAL->szIMSI, 
			util_cvtipaddr(szIP, pstSIGNAL->uiClientIP), pstSIGNAL->ucErrorCode, 
			util_printtime(pstSIGNAL->uiCallTime, szTime), pstSIGNAL->uiCallMTime,
			util_cvtipaddr(szPCFIP, pstSIGNAL->uiPCFIP), util_cvtipaddr(szPDSNIP, pstSIGNAL->uiNASName));
//	LOG_SIGNAL_Prt("PRINT LOG_SIGNAL", pstSIGNAL);

	if ( (pHASHONODE = hasho_find(pHASHOINFO, (U8*)pstRPPIKey)) )
	{
		pstRPPIHash = (HData_RPPI*)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);
//		log_print(LOGN_DEBUG, "Before Offset[%ld] After Offset[%ld]", pstRPPIHash->before.dOffset, pstRPPIHash->after.dOffset);	

		/* PI SIGNAL정보는 AcctStartTime기준으로 Call찾는다*/
		/* RP SIGNAL정보는 CallStartTime기준으로 Call찾는다*/ 
		if (pstSIGNAL->uiProtoType == RADIUS_PROTO || pstSIGNAL->uiProtoType ==DIAMETER_PROTO)
		{
			pstRPPILog = pFindRPPILog(0, pstRPPIHash, pstSIGNAL->uiAccStartTime, pstSIGNAL->uiAccStartMTime);
			if (pstRPPILog == NULL)
			{
//				log_print(LOGN_CRI, "[%s] NOT FOUND RPPI LOG IMSI[%s] IP[%s] TIME[%s.%u]",
				log_print(LOGN_WARN, "[%s] NOT FOUND RPPI LOG IMSI[%s] IP[%s] TIME[%s.%u] PDSNIP[%s]",
						PrintMsg(pstSIGNAL), pstSIGNAL->szIMSI, util_cvtipaddr(szIP, pstSIGNAL->uiClientIP),
						util_printtime(pstSIGNAL->uiCallTime, szTime), pstSIGNAL->uiCallMTime, util_cvtipaddr(szPDSNIP, pstSIGNAL->uiNASName));
				return -1;
			}	
		}
		else
		{
			pstRPPILog = pFindRPPILog(0, pstRPPIHash, pstSIGNAL->uiCallTime, pstSIGNAL->uiCallMTime);
			if (pstRPPILog == NULL)
			{
//				log_print(LOGN_CRI, "[%s] NOT FOUND RPPI LOG IMSI[%s] IP[%s]",
				//log_print(LOGN_WARN, "[%s] NOT FOUND RPPI LOG IMSI[%s] IP[%s] PDSNIP[%s]",
				log_print(LOGN_WARN, "[%s] NOT FOUND RPPI LOG IMSI[%s] IP[%s] TIME[%s.%u] PDSNIP[%s]",
						PrintMsg(pstSIGNAL), pstSIGNAL->szIMSI, util_cvtipaddr(szIP, pstSIGNAL->uiClientIP),
						util_printtime(pstSIGNAL->uiCallTime, szTime), pstSIGNAL->uiCallMTime, util_cvtipaddr(szPDSNIP, pstSIGNAL->uiNASName));
				return -1;
			}
		}	

		isReCall = GET_ISRECALL(pstRPPILog->usCallType);

#if 0	
		/* A11 Regi Start가 에러 일때 Call 삭제 */	
		if (pstSIGNAL->uiProtoType == A11_PROTO && 
				pstSIGNAL->uiMsgType == A11_REGIREQ_MSG && (pstSIGNAL->ucAirLink == CONNSETUP_ACTIVE_START || pstSIGNAL->ucAirLink == ACTIVE_START) &&
				pstSIGNAL->ucErrorCode > 0 )
		{
			log_print(LOGN_DEBUG,"[%s][%s.%d] A11 Regi Start Error[%d]" __FILE__, __FUNCTION__, __LINE__, pstSIGNAL->ucErrorCode);
			pstRPPILog->uiLastFailReason = (A11_DEFECT + A11_REGI_REPLY + pstSIGNAL->ucErrorCode);
			dDelRPPI(pstRPPIHash, pstRPPILog);
			return 0;
		}
#endif	
#ifdef _ACCESS_FILTER_  
/**
 * TAF재배치로 인해 중복된 ACCESS MSG를 필터링한다.
 * 중복 여부는 uiAccessReqTime의 유무로 판단한다.
 * 중복된 ACCESS MSG의 노드를 삭제하기 위해 IMSI를 NULL로 세팅한다.
 */

		if(pstSIGNAL->uiProtoType == RADIUS_PROTO && pstSIGNAL->uiMsgType == RADIUS_ACCESS_MSG) {
			if(pstRPPILog->uiAccessReqTime != 0) {
				pstSIGNAL->szIMSI[0] = 0x00;
				log_print(LOGN_INFO, "[%s][%s.%d] ACCESS FILTER OUT ACCESSREQTIME:%d.%d", __FILE__, __FUNCTION__, __LINE__,
						pstRPPILog->uiAccessReqTime, pstRPPILog->uiAccessReqMTime);
				return 0;
			}
		}
#endif    
		if (isReCall) {
			dUpdateReCallRPPI(pstRPPILog, pstSIGNAL, pstRPPIHash);
		}
		else {
			dUpdateRPPI(pstRPPILog, pstSIGNAL, pstRPPIHash);
		}

	}

	else
	{
//		log_print(LOGN_CRI, "NOT FOUND [%s:%u:%u] IMSI[%s] IP[%s] ERROR[%d] TIME[%s.%u] PCFIP[%s]", 
		log_print(LOGN_WARN, "NOT FOUND [%s:%u:%u] IMSI[%s] IP[%s] ERROR[%d] TIME[%s.%u] PCFIP[%s] ", 
			PrintMsg(pstSIGNAL), pstSIGNAL->uiProtoType, pstSIGNAL->uiMsgType, pstSIGNAL->szIMSI, 
			util_cvtipaddr(szIP, pstSIGNAL->uiClientIP), pstSIGNAL->ucErrorCode, 
			util_printtime(pstSIGNAL->uiCallTime, szTime), pstSIGNAL->uiCallMTime,
			util_cvtipaddr(szPCFIP, pstSIGNAL->uiPCFIP));


		/* DIAMETER 예외 처리 */
		st_WatchMsg stWatchMsg;
		switch(pstSIGNAL->uiProtoType){

			case DIAMETER_PROTO:
				switch(pstSIGNAL->uiMsgType){

					case USER_AUTHORIZATION_TRANS:
					case SERVER_ASSIGNMENT_TRANS:
					case LOCATION_INFO_TRANS:
					case MULTIMEDIA_AUTH_TRANS:
					case REGISTRATION_TERMINATION_TRANS:
					case PUSH_PROFILE_TRANS:
						/* SEND WATCH */
						memset(&stWatchMsg, 0x00, DEF_WATCHMSG_SIZE);
					
						stWatchMsg.usMsgType = WATCH_TYPE_HSS;
						stWatchMsg.uiHSSIP = pstSIGNAL->uiDestIP;

						if(pstSIGNAL->uiLastUserErrCode ==  3){
							stWatchMsg.uiResult = (DIAMETER_DEFECT + DIAMETER_CMD_DEFECT + pstSIGNAL->uiResultCode);
						}else if(pstSIGNAL->uiLastUserErrCode != 0){
							stWatchMsg.uiResult = (DIAMETER_DEFECT + DIAMETER_CMD_DEFECT + pstSIGNAL->uiLastUserErrCode);
						}

						U32 dSvcThreshold = dGetThreshold(SERVICE_DIAMETER, ALARM_RESPONSETIME);

						if(dSvcThreshold > 0 && pstSIGNAL->uiSessDuration > dSvcThreshold*1000){
							stWatchMsg.uiResult = (DIAMETER_DEFECT + SERVICE_DELAY_DEFECT + RESPONSETIME);
						}

						dSendMonInfo(&stWatchMsg);
						log_print(LOGN_INFO, "SEND WATCH MSG[%d][%s] OFFICE[%d][%s] RESULT[%d]",
							stWatchMsg.usMsgType, PrintMonType(stWatchMsg.usMsgType),
							stWatchMsg.ucOffice, PrintOFFICE(stWatchMsg.ucOffice), stWatchMsg.uiResult);
						break;
					case USER_DATA_TRANS:
						break;
					default:
						break;
				} /* inner-switch */
				break;
			default:
				break;	
		} /* outer-switch */
		return -1;
	}
	return 0;
} 


S32 dPAGESessInfo(U8 *data)
{
	RPPISESS_KEY   	stRPPIKey, *pstRPPIKey;
	HData_RPPI      stRPPIHash, *pstRPPIHash;
	U32				dSvcThreshold;
	LOG_PAGE_TRANS  *pLOG;

	stHASHONODE     *pHASHONODE;
	LOG_RPPI        *pstRPPILog;

	pstRPPIKey = &stRPPIKey;
	pstRPPIHash = &stRPPIHash;

	U8              szIP[INET_ADDRSTRLEN];
	U8              szTime[BUFSIZ];

	pLOG = (LOG_PAGE_TRANS *)data;
	log_print (LOGN_DEBUG, "RCV LOG_PAGE IMSI[%s] SVC[%ld][%s] ClientIP[%s] ERROR[%d] TIME[%s.%u]", 
			pLOG->szIMSI, pLOG->LastSvcL4Type, PrintSVC(pLOG->LastSvcL4Type), util_cvtipaddr(szIP, pLOG->uiClientIP), 
			pLOG->LastUserErrorCode, util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);

	if (pLOG->LastUserErrorCode > 0)
		pLOG->LastUserErrorCode = (SERVICE_DEFECT + SERVICE_PAGE_DEFECT + pLOG->LastUserErrorCode);

	memset(&stRPPIKey, 0x00, RPPISESS_KEY_SIZE);
	memcpy(pstRPPIKey, pLOG->szIMSI, MAX_MIN_LEN);

	if ( (pHASHONODE = hasho_find(pHASHOINFO, (U8*)pstRPPIKey)) == NULL)
	{
//		log_print(LOGN_CRI, "LOG_PAGE NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
		log_print(LOGN_WARN, "LOG_PAGE NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
				pstRPPIKey->szIMSI, pLOG->LastSvcL4Type, PrintSVC(pLOG->LastSvcL4Type), 
				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1;
	}

	pstRPPIHash = (HData_RPPI*)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);

	pstRPPILog = pFindRPPILog(0, pstRPPIHash, pLOG->uiAccStartTime, pLOG->uiAccStartMTime);

	if (pstRPPILog == NULL)
	{
//		log_print(LOGN_CRI, "LOG_PAGE NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
		log_print(LOGN_WARN, "LOG_PAGE NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
				pstRPPIKey->szIMSI, pLOG->LastSvcL4Type, PrintSVC(pLOG->LastSvcL4Type),
				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1; 
	}

	vCheckFirst(pstRPPIHash, pstRPPILog, (U8*)"PAGE", pLOG->FirstTcpSynTime, pLOG->FirstTcpSynMTime);

	dUpdateCommonLog((LOG_COMMON*) pLOG, pstRPPILog);	

	/* FB TransCnt > 1일때만 처리 : 예외 처리 */
	if((pLOG->LastSvcL4Type == L4_FB) && (pLOG->TransCnt <= 1)) {
		log_print(LOGN_DEBUG, "SKIP LOG_PAGE FB L4=%ld TRANSCNT=%u", pLOG->LastSvcL4Type, pLOG->TransCnt);
		return 0;
	}

	pstRPPILog->uiMenuGetCnt++;
	if (pstRPPILog->uiMenuStartReqTime == 0)
	{
		pstRPPILog->uiMenuStartReqTime = pLOG->FirstL7ReqStartTime;
		pstRPPILog->uiMenuStartReqMTime = pLOG->FirstL7ReqStartMTime;
	}
	if (pstRPPILog->uiMNAckTime == 0)
	{
		pstRPPILog->uiMNAckTime = pLOG->LastL7MNAckTime;
		pstRPPILog->uiMNAckMTime = pLOG->LastL7MNAckMTime;
	}

	if (pLOG->LastUserErrorCode == 0) {
		pstRPPILog->uiMenuAckCnt++;
		pstRPPILog->llMenuDelayedTime += pLOG->PageGapTime;
	}
	else {
		pstRPPILog->uiLastFailReason = pLOG->LastUserErrorCode;
	}

	/** 지연 요소 **/
	switch (pLOG->LastSvcL4Type)
	{
		case L4_WAP20:
			dSvcThreshold = dGetThreshold(SERVICE_MENU_WAP20, ALARM_RESPONSETIME);
			break;
		case L4_FB:
			dSvcThreshold = dGetThreshold(SERVICE_MENU_FB, ALARM_RESPONSETIME);
			break;
		default:
			log_print(LOGN_CRI, "LOG_PAGE UNKNOWN IMSI[%s] TIME[%s.%u] L4Type[%ld]", 
				pLOG->szIMSI, util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime, pLOG->LastSvcL4Type);
			break;
	}

	if (dSvcThreshold > 0 && pLOG->PageGapTime > dSvcThreshold*1000) {
		pstRPPILog->uiLastFailReason = (SERVICE_DEFECT + SERVICE_DELAY_DEFECT + RESPONSETIME);
		pLOG->LastUserErrorCode = pstRPPILog->uiLastFailReason;
		pstRPPILog->uiMenuDelayedCnt++;
	}
	////////////////		

	if (pstRPPILog->uiFirstPlatformType == 0)
		pstRPPILog->uiFirstPlatformType = pLOG->LastPlatformType;
	if (pstRPPILog->uiFirstSvcL4Type == 0)
		pstRPPILog->uiFirstSvcL4Type = pLOG->LastSvcL4Type;
	if (pstRPPILog->uiFirstSvcL7Type == 0)
		pstRPPILog->uiFirstSvcL7Type = pLOG->LastSvcL7Type;
	pstRPPILog->uiLastPlatformType = pLOG->LastPlatformType;
	pstRPPILog->uiLastSvcL4Type = pLOG->LastSvcL4Type;
	pstRPPILog->uiLastSvcL7Type = pLOG->LastSvcL7Type;

	if (pstRPPILog->uiFirstSvcStartTime == 0)
	{
		pstRPPILog->uiFirstSvcStartTime = pLOG->FirstL7ReqStartTime;
		pstRPPILog->uiFirstSvcStartMTime = pLOG->FirstL7ReqStartMTime;
		pstRPPILog->uiFirstSvcEndTime = pLOG->LastL7LastPktTime;
		pstRPPILog->uiFirstSvcEndMTime = pLOG->LastL7LastPktMTime;
	}
	pstRPPILog->uiLastSvcStartTime = pLOG->FirstL7ReqStartTime;
	pstRPPILog->uiLastSvcStartMTime = pLOG->FirstL7ReqStartMTime;
	pstRPPILog->uiLastSvcEndTime = pLOG->LastL7LastPktTime;
	pstRPPILog->uiLastSvcEndMTime = pLOG->LastL7LastPktMTime;

	/** WATCH DATA */
	st_WatchMsg stWatchMsg;
	memset(&stWatchMsg, 0x00, DEF_WATCHMSG_SIZE);

	stWatchMsg.usMsgType = WATCH_TYPE_SVC;   
	stWatchMsg.ucOffice = pstRPPILog->ucBranchID;
	stWatchMsg.ucSYSID = pstRPPILog->ucSYSID;
	stWatchMsg.ucBSCID = pstRPPILog->ucBSCID;
	stWatchMsg.usBTSID = pstRPPILog->ucBTSID;
	stWatchMsg.ucSec = pstRPPILog->ucSECTOR;
	stWatchMsg.ucFA = pstRPPILog->ucFA_ID;
	stWatchMsg.uiPCFIP = pstRPPILog->uiPCFIP;
	stWatchMsg.ucPCFType = dGetPCFType(pstRPPILog->uiPCFIP);
	stWatchMsg.uiPDSNIP = pstRPPILog->uiNASName;
	stWatchMsg.uiAAAIP = pstRPPILog->uiAAAIP;
	stWatchMsg.uiHSSIP = pstRPPILog->uiHSSIP;
	stWatchMsg.ucSvcIdx = dGetSvcIndex(GET_ISRECALL(pstRPPILog->usCallType), pLOG->LastPlatformType, pLOG->LastSvcL7Type);
	stWatchMsg.usSvcL4Type = pLOG->LastSvcL4Type;
	stWatchMsg.uiSVCIP = pLOG->uiServerIP;
	stWatchMsg.uiResult = pLOG->LastUserErrorCode;

	dSendMonInfo(&stWatchMsg);
	log_print(LOGN_INFO, "SEND WATCH MSG[%d][%s] OFFICE[%d][%s] RESULT[%d]",
			stWatchMsg.usMsgType, PrintMonType(stWatchMsg.usMsgType),
			stWatchMsg.ucOffice, PrintOFFICE(stWatchMsg.ucOffice), stWatchMsg.uiResult);
	return 0;
}

S32 dVODSessInfo(U8 *data)
{
	RPPISESS_KEY   stRPPIKey, *pstRPPIKey;
	HData_RPPI      stRPPIHash, *pstRPPIHash;
	U32				uiJitterThreshold, uiUpLossThreshold, uiDnLossThreshold;	
	LOG_VOD_SESS  *pLOG;

	stHASHONODE     *pHASHONODE;
	LOG_RPPI        *pstRPPILog;

	pstRPPIKey = &stRPPIKey;
	pstRPPIHash = &stRPPIHash;

	U8              szIP[INET_ADDRSTRLEN];
	U8              szTime[BUFSIZ];

	pLOG = (LOG_VOD_SESS *)data;
	log_print (LOGN_DEBUG, "RCV LOG_VOD IMSI[%s] SVC[%ld][%s] IP[%s] ERROR[%d] TIME[%s.%u]", 
			pLOG->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
			util_cvtipaddr(szIP, pLOG->uiClientIP), pLOG->usUserErrorCode, util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);

	if (pLOG->usUserErrorCode > 0)
		pLOG->usUserErrorCode =  SERVICE_DEFECT + SERVICE_VOD_DEFECT + pLOG->usUserErrorCode;

	memset(&stRPPIKey, 0x00, RPPISESS_KEY_SIZE);
	memcpy(pstRPPIKey, pLOG->szIMSI, MAX_MIN_LEN);

	if ( (pHASHONODE = hasho_find(pHASHOINFO, (U8*)pstRPPIKey)) == NULL)
	{
//		log_print(LOGN_CRI, "LOG_VOD NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
		log_print(LOGN_WARN, "LOG_VOD NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
				pstRPPIKey->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1;
	}

	pstRPPIHash = (HData_RPPI*)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);

	pstRPPILog = pFindRPPILog(0, pstRPPIHash, pLOG->uiAccStartTime, pLOG->uiAccStartMTime);

	if (pstRPPILog == NULL)
	{
//		log_print(LOGN_CRI, "LOG_VOD NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
		log_print(LOGN_WARN, "LOG_VOD NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
				pstRPPIKey->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1;
	}    
	
	vCheckFirst(pstRPPIHash, pstRPPILog, (U8*)"VOD", pLOG->uiSetupStartTime, pLOG->uiSetupStartMTime);

	dUpdateCommonLog((LOG_COMMON*) pLOG, pstRPPILog);

	pstRPPILog->uiVODReqCnt++;
	if (pLOG->uiSetupEndTime !=0)
	{
		pstRPPILog->uiVODSetupCnt++;
	}

	/* 90500002 : NOTEARDOWN 예외 처리 성공으로 판단 */
	if ((pLOG->usUserErrorCode > 0) && (pLOG->usUserErrorCode != 90500002))
	{
		pstRPPILog->uiLastFailReason =  pLOG->usUserErrorCode;
	}

	/*** 지연 요소 ***/
	switch(pLOG->usSvcL4Type)
	{
		case L4_VOD_STREAM:
			uiJitterThreshold = dGetThreshold(SERVICE_STREAMING_SVOD, ALARM_DNJITTER);
			uiUpLossThreshold = dGetThreshold(SERVICE_STREAMING_SVOD, ALARM_UPPACKETLOSS);
			uiDnLossThreshold = dGetThreshold(SERVICE_STREAMING_SVOD, ALARM_DNPACKETLOSS);
		case L4_RTS_FB :
		case L4_RTS_WB:
			uiJitterThreshold = dGetThreshold(SERVICE_STREAMING_RTS, ALARM_DNJITTER);
			uiUpLossThreshold = dGetThreshold(SERVICE_STREAMING_RTS, ALARM_UPPACKETLOSS);
			uiDnLossThreshold = dGetThreshold(SERVICE_STREAMING_RTS, ALARM_DNPACKETLOSS);
			break;
		case L4_MBOX:
			uiJitterThreshold = dGetThreshold(SERVICE_STREAMING_MBOX, ALARM_DNJITTER);
			uiUpLossThreshold = dGetThreshold(SERVICE_STREAMING_MBOX, ALARM_UPPACKETLOSS);
			uiDnLossThreshold = dGetThreshold(SERVICE_STREAMING_MBOX, ALARM_DNPACKETLOSS);
			break;
		default:
			log_print(LOGN_CRI, "LOG_VOD UNKNOWN IMSI[%s] TIME[%s.%u] L4Type[%ld]", 
					pLOG->szIMSI, util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime, pLOG->usPlatformType);	 
	}

	if (uiJitterThreshold > 0 && pLOG->uiMaxJitter > uiJitterThreshold) {
		pstRPPILog->uiLastFailReason = (SERVICE_DEFECT + SERVICE_DELAY_DEFECT + JITTER);
		pLOG->usUserErrorCode = pstRPPILog->uiLastFailReason;
		pstRPPILog->uiRTPJitterOverCnt++;
	}

	if (uiUpLossThreshold > 0 && pLOG->usRtpUpLossCnt > uiUpLossThreshold) {
		pstRPPILog->uiLastFailReason = (SERVICE_DEFECT + SERVICE_DELAY_DEFECT + PACKETLOSS);
		pLOG->usUserErrorCode = pstRPPILog->uiLastFailReason;
		pstRPPILog->uiRTPLossOverCnt++;	
	}

	if (uiDnLossThreshold > 0 && pLOG->usRtpDnLossCnt > uiDnLossThreshold) {
		pstRPPILog->uiLastFailReason = (SERVICE_DEFECT + SERVICE_DELAY_DEFECT + PACKETLOSS);
		pLOG->usUserErrorCode = pstRPPILog->uiLastFailReason;
		pstRPPILog->uiRTPLossOverCnt++;
	}
	////////////////////	

	pstRPPILog->uiRTPUpCnt += pLOG->usRtpUpCnt;
	pstRPPILog->uiRTPDnCnt += pLOG->usRtpDnCnt;
	pstRPPILog->uiRTPUpLossCnt += pLOG->usRtpUpLossCnt;
	pstRPPILog->uiRTPDnLossCnt += pLOG->usRtpDnLossCnt;
	pstRPPILog->uiRTPUpDataSize += pLOG->uiRtpUpDataSize;
	pstRPPILog->uiRTPDnDataSize += pLOG->uiRtpDnDataSize;
	pstRPPILog->uiRTPDnMaxJitter = max(pstRPPILog->uiRTPDnMaxJitter, pLOG->uiMaxJitter);

	pstRPPILog->uiUpTotPktCnt += pLOG->uiIPTotUpPktCnt;
	pstRPPILog->uiDnTotPktCnt += pLOG->uiIPTotDnPktCnt;
	pstRPPILog->uiUpTotDataSize += pLOG->uiIPTotUpPktSize;
	pstRPPILog->uiDnTotDataSize += pLOG->uiIPTotDnPktSize;

	if (pstRPPILog->uiFirstPlatformType == 0)
		pstRPPILog->uiFirstPlatformType = pLOG->usPlatformType;
	if (pstRPPILog->uiFirstSvcL4Type == 0)
		pstRPPILog->uiFirstSvcL4Type = pLOG->usSvcL4Type;
	if (pstRPPILog->uiFirstSvcL7Type == 0)
		pstRPPILog->uiFirstSvcL7Type = pLOG->usSvcL7Type;

	pstRPPILog->uiLastPlatformType = pLOG->usPlatformType;
	pstRPPILog->uiLastSvcL4Type = pLOG->usSvcL4Type;
	pstRPPILog->uiLastSvcL7Type = pLOG->usSvcL7Type;

	if (pstRPPILog->uiFirstSvcStartTime == 0)
	{
		pstRPPILog->uiFirstSvcStartTime = pLOG->uiSetupStartTime;
		pstRPPILog->uiFirstSvcStartMTime = pLOG->uiSetupStartMTime;
		pstRPPILog->uiFirstSvcEndTime = pLOG->uiLastPktTime;
		pstRPPILog->uiFirstSvcEndMTime = pLOG->uiLastPktMTime;
	}
	pstRPPILog->uiLastSvcStartTime = pLOG->uiSetupStartTime;
	pstRPPILog->uiLastSvcStartMTime = pLOG->uiSetupStartMTime;
	pstRPPILog->uiLastSvcEndTime = pLOG->uiLastPktTime;
	pstRPPILog->uiLastSvcEndMTime = pLOG->uiLastPktMTime;


	/** WATCH DATA */
	st_WatchMsg stWatchMsg;
	memset(&stWatchMsg, 0x00, DEF_WATCHMSG_SIZE);

	stWatchMsg.usMsgType = WATCH_TYPE_SVC;
	stWatchMsg.ucOffice = pstRPPILog->ucBranchID; 
	stWatchMsg.ucSYSID = pstRPPILog->ucSYSID;
	stWatchMsg.ucBSCID = pstRPPILog->ucBSCID;
	stWatchMsg.usBTSID = pstRPPILog->ucBTSID;
	stWatchMsg.ucSec = pstRPPILog->ucSECTOR;
	stWatchMsg.ucFA = pstRPPILog->ucFA_ID;
	stWatchMsg.uiPCFIP = pstRPPILog->uiPCFIP; 
	stWatchMsg.ucPCFType = dGetPCFType(pstRPPILog->uiPCFIP);
	stWatchMsg.uiPDSNIP = pstRPPILog->uiNASName;
	stWatchMsg.uiAAAIP = pstRPPILog->uiAAAIP;
	stWatchMsg.uiHSSIP = pstRPPILog->uiHSSIP;
	stWatchMsg.ucSvcIdx = dGetSvcIndex(GET_ISRECALL(pstRPPILog->usCallType), pLOG->usPlatformType, pLOG->usSvcL7Type);
	stWatchMsg.usSvcL4Type = pLOG->usSvcL4Type;
	stWatchMsg.uiSVCIP = pLOG->uiServerIP;
	stWatchMsg.uiResult = (pLOG->usUserErrorCode == 90500002) ? 0 : pLOG->usUserErrorCode;

	dSendMonInfo(&stWatchMsg);
	log_print(LOGN_INFO, "SEND WATCH MSG[%d][%s] OFFICE[%d][%s] RESULT[%d]",
			stWatchMsg.usMsgType, PrintMonType(stWatchMsg.usMsgType),
			stWatchMsg.ucOffice, PrintOFFICE(stWatchMsg.ucOffice), stWatchMsg.uiResult);
	return 0;
}

S32 dHTTPSessInfo(U8 *data)
{
	RPPISESS_KEY   stRPPIKey, *pstRPPIKey;
	HData_RPPI      stRPPIHash, *pstRPPIHash;
	S64				llUpLoadingTime, llDnLoadingTime;
	S32				dUpThroughput, dDnThroughput;
	S32				dUpThroughput_Thre, dDnThroughput_Thre, dUpRetransCnt_Thre, dDnRetransCnt_Thre;
	U32             dSvcThreshold;
	LOG_HTTP_TRANS  *pLOG;

	stHASHONODE     *pHASHONODE;
	LOG_RPPI        *pstRPPILog;

	pstRPPIKey = &stRPPIKey;
	pstRPPIHash = &stRPPIHash;

	U8              szIP[INET_ADDRSTRLEN];
	U8              szTime[BUFSIZ];

	pLOG = (LOG_HTTP_TRANS *)data;
	log_print (LOGN_DEBUG, "RCV LOG_HTTP IMSI[%s] SVC[%ld][%s] IP[%s] ERROR[%d] TIME[%s.%u]", 
			pLOG->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
			util_cvtipaddr(szIP, pLOG->uiClientIP), pLOG->usUserErrorCode, util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);

	if (pLOG->usUserErrorCode > 0) 
		pLOG->usUserErrorCode = SERVICE_DEFECT + SERVICE_HTTP_DEFECT + pLOG->usUserErrorCode;

	memset(&stRPPIKey, 0x00, RPPISESS_KEY_SIZE);
	memcpy(pstRPPIKey, pLOG->szIMSI, MAX_MIN_LEN);

	if ( (pHASHONODE = hasho_find(pHASHOINFO, (U8*)pstRPPIKey)) == NULL)
	{
//		log_print(LOGN_CRI, "LOG_HTTP NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
		log_print(LOGN_WARN, "LOG_HTTP NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
				pstRPPIKey->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1;
	}   

	pstRPPIHash = (HData_RPPI*)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);

	pstRPPILog = pFindRPPILog(0, pstRPPIHash, pLOG->uiAccStartTime, pLOG->uiAccStartMTime);

	if (pstRPPILog == NULL)
	{
//		log_print(LOGN_CRI, "LOG_HTTP NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
		log_print(LOGN_WARN, "LOG_HTTP NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
				pstRPPIKey->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1;
	}

	vCheckFirst(pstRPPIHash, pstRPPILog, (U8*)"HTTP", pLOG->uiTcpSynTime, pLOG->uiTcpSynMTime);

	dUpdateCommonLog((LOG_COMMON*) pLOG, pstRPPILog);

	/* WIDGET 예외 처리: AppFailCode 4000 ~ 4999 인 경우 제외 */
	if((pLOG->usSvcL4Type == L4_WIDGET) && (strlen((char*)pLOG->szAppFailCode) == 4) && (pLOG->szAppFailCode[0] == '4')) {
		log_print(LOGN_DEBUG, "SKIP WIDGET AppFailCode=%.*s", MAX_APPFAILCODE_SIZE, pLOG->szAppFailCode);
		return 0;
	}

	if (pLOG->usUserErrorCode > 0) {
		pstRPPILog->uiLastFailReason = pLOG->usUserErrorCode;
	}

	/* 지연 요소 */
	switch (pLOG->usPlatformType)
	{
		case DEF_PLATFORM_MENU:
			dSvcThreshold = dGetThreshold(SERVICE_MENU_WIPI, ALARM_RESPONSETIME);
			break;
		case DEF_PLATFORM_DN:
			switch(pLOG->usSvcL4Type)
			{
				case L4_DN_2G:
				case L4_DN_2G_NODN:
					dUpThroughput_Thre = dGetThreshold(SERVICE_DOWNLOAD_2G, ALARM_UPTHROUGHPUT);
					dUpRetransCnt_Thre = dGetThreshold(SERVICE_DOWNLOAD_2G, ALARM_UPRETRANSCNT);
					dDnThroughput_Thre = dGetThreshold(SERVICE_DOWNLOAD_2G, ALARM_DNTHROUGHPUT);
					dDnRetransCnt_Thre = dGetThreshold(SERVICE_DOWNLOAD_2G, ALARM_DNRETRANSCNT);
					break;
				case L4_WIPI:
					dUpThroughput_Thre = dGetThreshold(SERVICE_DOWNLOAD_WIPI, ALARM_UPTHROUGHPUT);
					dUpRetransCnt_Thre = dGetThreshold(SERVICE_DOWNLOAD_WIPI, ALARM_UPRETRANSCNT);
					dDnThroughput_Thre = dGetThreshold(SERVICE_DOWNLOAD_WIPI, ALARM_DNTHROUGHPUT);
					dDnRetransCnt_Thre = dGetThreshold(SERVICE_DOWNLOAD_WIPI, ALARM_DNRETRANSCNT);
					break;
				case L4_DN_VOD:
				case L4_DN_VOD_NODN:
					dUpThroughput_Thre = dGetThreshold(SERVICE_DOWNLOAD_VOD, ALARM_UPTHROUGHPUT);
					dUpRetransCnt_Thre = dGetThreshold(SERVICE_DOWNLOAD_VOD, ALARM_UPRETRANSCNT);
					dDnThroughput_Thre = dGetThreshold(SERVICE_DOWNLOAD_VOD, ALARM_DNTHROUGHPUT);
					dDnRetransCnt_Thre = dGetThreshold(SERVICE_DOWNLOAD_VOD, ALARM_DNRETRANSCNT);
					break;
				case L4_DN_JAVA:
					dUpThroughput_Thre = dGetThreshold(SERVICE_DOWNLOAD_JAVA, ALARM_UPTHROUGHPUT);
					dUpRetransCnt_Thre = dGetThreshold(SERVICE_DOWNLOAD_JAVA, ALARM_UPRETRANSCNT);
					dDnThroughput_Thre = dGetThreshold(SERVICE_DOWNLOAD_JAVA, ALARM_DNTHROUGHPUT);
					dDnRetransCnt_Thre = dGetThreshold(SERVICE_DOWNLOAD_JAVA, ALARM_DNRETRANSCNT);
					break;
				case L4_OMA_DN:
					dUpThroughput_Thre = dGetThreshold(SERVICE_DOWNLOAD_OMA_DN, ALARM_UPTHROUGHPUT);
					dUpRetransCnt_Thre = dGetThreshold(SERVICE_DOWNLOAD_OMA_DN, ALARM_UPRETRANSCNT);
					dDnThroughput_Thre = dGetThreshold(SERVICE_DOWNLOAD_OMA_DN, ALARM_DNTHROUGHPUT);
					dDnRetransCnt_Thre = dGetThreshold(SERVICE_DOWNLOAD_OMA_DN, ALARM_DNRETRANSCNT);
					break;
				case L4_OMA_DN_2G:
					dUpThroughput_Thre = dGetThreshold(SERVICE_DOWNLOAD_OMA_2G, ALARM_UPTHROUGHPUT);
					dUpRetransCnt_Thre = dGetThreshold(SERVICE_DOWNLOAD_OMA_2G, ALARM_UPRETRANSCNT);
					dDnThroughput_Thre = dGetThreshold(SERVICE_DOWNLOAD_OMA_2G, ALARM_DNTHROUGHPUT);
					dDnRetransCnt_Thre = dGetThreshold(SERVICE_DOWNLOAD_OMA_2G, ALARM_DNRETRANSCNT);
					break;
				case L4_OMA_DN_VOD:
					dUpThroughput_Thre = dGetThreshold(SERVICE_DOWNLOAD_OMA_VOD, ALARM_UPTHROUGHPUT);
					dUpRetransCnt_Thre = dGetThreshold(SERVICE_DOWNLOAD_OMA_VOD, ALARM_UPRETRANSCNT);
					dDnThroughput_Thre = dGetThreshold(SERVICE_DOWNLOAD_OMA_VOD, ALARM_DNTHROUGHPUT);
					dDnRetransCnt_Thre = dGetThreshold(SERVICE_DOWNLOAD_OMA_VOD, ALARM_DNRETRANSCNT);
					break;
				case L4_OMA_DN_WIPI:
					dUpThroughput_Thre = dGetThreshold(SERVICE_DOWNLOAD_OMA_WIPI, ALARM_UPTHROUGHPUT);
					dUpRetransCnt_Thre = dGetThreshold(SERVICE_DOWNLOAD_OMA_WIPI, ALARM_UPRETRANSCNT);
					dDnThroughput_Thre = dGetThreshold(SERVICE_DOWNLOAD_OMA_WIPI, ALARM_DNTHROUGHPUT);
					dDnRetransCnt_Thre = dGetThreshold(SERVICE_DOWNLOAD_OMA_WIPI, ALARM_DNRETRANSCNT);
					break;

			}
		case DEF_PLATFORM_MMS:
			dUpThroughput_Thre = dGetThreshold(SERVICE_MMS, ALARM_UPTHROUGHPUT);
			dUpRetransCnt_Thre = dGetThreshold(SERVICE_MMS, ALARM_UPRETRANSCNT);
			dDnThroughput_Thre = dGetThreshold(SERVICE_MMS, ALARM_DNTHROUGHPUT);
			dDnRetransCnt_Thre = dGetThreshold(SERVICE_MMS, ALARM_DNRETRANSCNT);
			break;
		case DEF_PLATFORM_EMS:
			dSvcThreshold = dGetThreshold(SERVICE_EMS, ALARM_RESPONSETIME);
			break;
		case DEF_PLATFORM_FV:
			dSvcThreshold = dGetThreshold(SERVICE_FV, ALARM_RESPONSETIME);
			break;
		case DEF_PLATFORM_WIDGET:
			switch(pLOG->usSvcL4Type)
			{
				case L4_TODAY:
					dUpThroughput_Thre = dGetThreshold(SERVICE_TODAY, ALARM_UPTHROUGHPUT);
					dUpRetransCnt_Thre = dGetThreshold(SERVICE_TODAY, ALARM_UPRETRANSCNT);
					dDnThroughput_Thre = dGetThreshold(SERVICE_TODAY, ALARM_DNTHROUGHPUT);
					dDnRetransCnt_Thre = dGetThreshold(SERVICE_TODAY, ALARM_DNRETRANSCNT);
					break;
				case L4_WIDGET:
					dUpThroughput_Thre = dGetThreshold(SERVICE_WIDGET, ALARM_UPTHROUGHPUT);
					dUpRetransCnt_Thre = dGetThreshold(SERVICE_WIDGET, ALARM_UPRETRANSCNT);
					dDnThroughput_Thre = dGetThreshold(SERVICE_WIDGET, ALARM_DNTHROUGHPUT);
					dDnRetransCnt_Thre = dGetThreshold(SERVICE_WIDGET, ALARM_DNRETRANSCNT);
			}
			break;
	}

	switch (pLOG->usPlatformType)
	{
		case DEF_PLATFORM_DN:
		case DEF_PLATFORM_MMS:
		case DEF_PLATFORM_WIDGET:
			STG_DiffTIME64(pLOG->uiReqEndTime, pLOG->uiReqEndMTime, pLOG->uiReqStartTime, pLOG->uiReqStartMTime, &llUpLoadingTime);
			STG_DiffTIME64(pLOG->uiLastPktTime, pLOG->uiLastPktMTime, pLOG->uiResStartTime, pLOG->uiResStartMTime, &llDnLoadingTime);

			pstRPPILog->uiDnReqCnt++;
			if (llUpLoadingTime > 0 ) {
				pstRPPILog->uiUpDataSize += pLOG->uiTcpUpBodySize;
				pstRPPILog->llUpDataLoadingTime += llUpLoadingTime;
				dUpThroughput = ((double)(pLOG->uiTcpUpBodySize / llUpLoadingTime)) * 8000000;
			}
			if (llDnLoadingTime > 0) {
				pstRPPILog->uiDnDataSize += pLOG->uiTcpDnBodySize;
				pstRPPILog->llDnDataLoadingTime += llDnLoadingTime;
				dDnThroughput = ((double)(pLOG->uiTcpDnBodySize / llDnLoadingTime)) * 8000000;
			}

			if (dUpThroughput_Thre > 0 && dUpThroughput !=0 && dUpThroughput < dUpThroughput_Thre) {
				pstRPPILog->uiLastFailReason = (SERVICE_DEFECT + SERVICE_DELAY_DEFECT + THROUGHPUT);
				pLOG->usUserErrorCode = pstRPPILog->uiLastFailReason;
				pstRPPILog->uiThroughputDelayCnt++;
				if (pLOG->uiIPDataUpRetransCnt > dUpRetransCnt_Thre) {
					pstRPPILog->uiLastFailReason = (SERVICE_DEFECT + SERVICE_DELAY_DEFECT + UPRETRANSCOUNT);
					pLOG->usUserErrorCode = pstRPPILog->uiLastFailReason;
				}
			}
			if (dDnThroughput_Thre > 0 && dDnThroughput !=0 && dDnThroughput < dDnThroughput_Thre)    {
				pstRPPILog->uiLastFailReason = (SERVICE_DEFECT + SERVICE_DELAY_DEFECT + THROUGHPUT);
				pLOG->usUserErrorCode = pstRPPILog->uiLastFailReason;
				pstRPPILog->uiThroughputDelayCnt++;
				if (pLOG->uiIPDataDnRetransCnt > dDnRetransCnt_Thre) {
					pstRPPILog->uiLastFailReason = (SERVICE_DEFECT + SERVICE_DELAY_DEFECT + DNRETRANSCOUNT);
					pLOG->usUserErrorCode = pstRPPILog->uiLastFailReason;
				}
			}
			if (pLOG->usUserErrorCode == 0)
				pstRPPILog->uiDnSuccCnt++;	
			break;
		case DEF_PLATFORM_MENU:
		case DEF_PLATFORM_EMS:
		case DEF_PLATFORM_FV:
			pstRPPILog->uiHTTPGetCnt++;
			if (pstRPPILog->uiHTTPStartReqTime == 0)
			{
				pstRPPILog->uiHTTPStartReqTime = pLOG->uiReqStartTime;
				pstRPPILog->uiHTTPStartReqMTime = pLOG->uiReqStartMTime;
			}
			if (pstRPPILog->uiHTTPMNAckTime == 0)
			{
				pstRPPILog->uiHTTPMNAckTime = pLOG->uiMNAckTime;
				pstRPPILog->uiHTTPMNAckMTime = pLOG->uiMNAckMTime;
			}

			if (dSvcThreshold > 0 && pLOG->llTransGapTime > dSvcThreshold*1000) {
				pstRPPILog->uiLastFailReason = (SERVICE_DEFECT + SERVICE_DELAY_DEFECT + RESPONSETIME);
				pLOG->usUserErrorCode = pstRPPILog->uiLastFailReason;
				pstRPPILog->uiHTTPDelayedCnt++;
			}

			if (pLOG->usUserErrorCode == 0) {
				pstRPPILog->uiHTTPAckCnt++;
				pstRPPILog->llHTTPDelayedTime += pLOG->llTransGapTime;
			}
			break;
	}

	if (pstRPPILog->uiFirstPlatformType == 0)
		pstRPPILog->uiFirstPlatformType = pLOG->usPlatformType;
	if (pstRPPILog->uiFirstSvcL4Type == 0)
		pstRPPILog->uiFirstSvcL4Type = pLOG->usSvcL4Type;
	if (pstRPPILog->uiFirstSvcL7Type == 0)
		pstRPPILog->uiFirstSvcL7Type = pLOG->usSvcL7Type;
	pstRPPILog->uiLastPlatformType = pLOG->usPlatformType;
	pstRPPILog->uiLastSvcL4Type = pLOG->usSvcL4Type;
	pstRPPILog->uiLastSvcL7Type = pLOG->usSvcL7Type;

	if (pLOG->usPlatformType != DEF_PLATFORM_STREAM)
	{
		/* RTSP인 경우 VOD SESS에서 UPDATE */
		if (pstRPPILog->uiFirstSvcStartTime == 0)
		{
			pstRPPILog->uiFirstSvcStartTime = pLOG->uiReqStartTime;
			pstRPPILog->uiFirstSvcStartMTime = pLOG->uiReqStartMTime;
			pstRPPILog->uiFirstSvcEndTime = pLOG->uiLastPktTime;
			pstRPPILog->uiFirstSvcEndMTime = pLOG->uiLastPktMTime;
		}
		pstRPPILog->uiLastSvcStartTime = pLOG->uiReqStartTime;
		pstRPPILog->uiLastSvcStartMTime = pLOG->uiReqStartMTime;
		pstRPPILog->uiLastSvcEndTime = pLOG->uiLastPktTime;
		pstRPPILog->uiLastSvcEndMTime = pLOG->uiLastPktMTime;

		/** WATCH DATA */
		st_WatchMsg stWatchMsg;
		memset(&stWatchMsg, 0x00, DEF_WATCHMSG_SIZE);

		stWatchMsg.usMsgType = WATCH_TYPE_SVC;
		stWatchMsg.ucOffice = pstRPPILog->ucBranchID; 
		stWatchMsg.ucSYSID = pstRPPILog->ucSYSID;
		stWatchMsg.ucBSCID = pstRPPILog->ucBSCID;
		stWatchMsg.usBTSID = pstRPPILog->ucBTSID;
		stWatchMsg.ucSec = pstRPPILog->ucSECTOR;
		stWatchMsg.ucFA = pstRPPILog->ucFA_ID;
		stWatchMsg.uiPCFIP = pstRPPILog->uiPCFIP;
		stWatchMsg.ucPCFType = dGetPCFType(pstRPPILog->uiPCFIP);
		stWatchMsg.uiPDSNIP = pstRPPILog->uiNASName;
		stWatchMsg.uiAAAIP = pstRPPILog->uiAAAIP;
		stWatchMsg.uiHSSIP = pstRPPILog->uiHSSIP;
		stWatchMsg.ucSvcIdx = dGetSvcIndex(GET_ISRECALL(pstRPPILog->usCallType), pLOG->usPlatformType, pLOG->usSvcL7Type);
		stWatchMsg.usSvcL4Type = pLOG->usSvcL4Type;
		stWatchMsg.uiSVCIP = pLOG->uiServerIP; 
		stWatchMsg.uiResult = pLOG->usUserErrorCode;

		dSendMonInfo(&stWatchMsg);
		log_print(LOGN_INFO, "SEND WATCH MSG[%d][%s] OFFICE[%d][%s] RESULT[%d]",
				stWatchMsg.usMsgType, PrintMonType(stWatchMsg.usMsgType),
				stWatchMsg.ucOffice, PrintOFFICE(stWatchMsg.ucOffice), stWatchMsg.uiResult);
	}
	return 0;
}



S32 dTCPSessInfo(U8 *data)
{
	S64 llTcpSetupTime;	
	RPPISESS_KEY   stRPPIKey, *pstRPPIKey;
	HData_RPPI      stRPPIHash, *pstRPPIHash; 
	S32				dSvcThreshold;	
	LOG_TCP_SESS    *pLOG;

	stHASHONODE     *pHASHONODE;
	LOG_RPPI        *pstRPPILog;


	pstRPPIKey = &stRPPIKey;
	pstRPPIHash = &stRPPIHash;

	U8              szIP[INET_ADDRSTRLEN];
	U8              szTime[BUFSIZ];

	pLOG = (LOG_TCP_SESS *)data;
	log_print (LOGN_DEBUG, "RCV LOG_TCP IMSI[%s] SVC[%ld][%s] IP[%s] ERROR[%ld] TIME[%s.%u]", pLOG->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
			util_cvtipaddr(szIP, pLOG->uiClientIP), pLOG->usL4FailCode, util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);

	if(pLOG->usL4FailCode > 0)	
		pLOG->usL4FailCode = SERVICE_DEFECT + SERVICE_TCP_DEFECT + pLOG->usL4FailCode;

	memset(pstRPPIKey, 0x00, RPPISESS_KEY_SIZE);
	memcpy(pstRPPIKey, pLOG->szIMSI, MAX_MIN_LEN);

	if ( (pHASHONODE = hasho_find(pHASHOINFO, (U8*)pstRPPIKey)) == NULL)
	{
//		log_print(LOGN_CRI, "LOG_TCP NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
		log_print(LOGN_WARN, "LOG_TCP NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
				pstRPPIKey->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1;
	}

	pstRPPIHash = (HData_RPPI*)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);
	pstRPPILog = pFindRPPILog(0, pstRPPIHash, pLOG->uiAccStartTime, pLOG->uiAccStartMTime);

	if (pstRPPILog == NULL)
	{
//		log_print(LOGN_CRI, "LOG_TCP NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
		log_print(LOGN_WARN, "LOG_TCP NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
				pstRPPIKey->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1;
	}

	vCheckFirst(pstRPPIHash, pstRPPILog, (U8*)"TCP", pLOG->uiTcpSynTime, pLOG->uiTcpSynMTime);

	dUpdateCommonLog((LOG_COMMON*) pLOG, pstRPPILog);

	pstRPPILog->uiTCPSynCnt += pLOG->ucTcpSynCnt;

	pstRPPILog->uiUpTCPPacketCnt += pLOG->uiIPTotUpPktCnt;
	pstRPPILog->uiDnTCPPacketCnt += pLOG->uiIPTotDnPktCnt;

	pstRPPILog->uiUpTCPRetransCnt += pLOG->uiIPTotUpRetransCnt;		
	pstRPPILog->uiDnTCPRetransCnt += pLOG->uiIPTotDnRetransCnt;

	if (pLOG->usSvcL4Type != L4_VOD_STREAM)
	{
		pstRPPILog->uiUpTotPktCnt += pLOG->uiIPTotUpPktCnt;
		pstRPPILog->uiDnTotPktCnt += pLOG->uiIPTotDnPktCnt;

		pstRPPILog->uiUpTotDataSize += pLOG->uiIPTotUpPktSize;
		pstRPPILog->uiDnTotDataSize += pLOG->uiIPTotDnPktSize;
	}
	if(pstRPPILog->uiFirstTCPSynTime==0 ) {
		pstRPPILog->uiFirstTCPSynTime = pLOG->uiTcpSynTime;
		pstRPPILog->uiFirstTCPSynMTime = pLOG->uiTcpSynMTime;
		STG_DiffTIME64(pstRPPILog->uiFirstTCPSynTime, pstRPPILog->uiFirstTCPSynMTime, pstRPPILog->uiCallTime, pstRPPILog->uiCallMTime, &pstRPPILog->llRPTCPSynDelTime);

	}
	else {
		S64 llGap = 0;
		STG_DeltaTIME64(pstRPPILog->uiFirstTCPSynTime, pstRPPILog->uiFirstTCPSynMTime, pLOG->uiTcpSynTime, pLOG->uiTcpSynMTime, &llGap);
		if(llGap > 0) {
			pstRPPILog->uiFirstTCPSynTime = pLOG->uiTcpSynTime;
			pstRPPILog->uiFirstTCPSynMTime = pLOG->uiTcpSynMTime;
			STG_DiffTIME64(pstRPPILog->uiFirstTCPSynTime, pstRPPILog->uiFirstTCPSynMTime, pstRPPILog->uiCallTime, pstRPPILog->uiCallMTime, &pstRPPILog->llRPTCPSynDelTime);
		}
	}

	/* 성공한 TCP */
	if(pLOG->uiTcpSynAckAckTime != 0) 
	{
		pstRPPILog->uiTCPConnCnt++;
		if (pstRPPILog->uiTCPConnEndTime ==0)
		{
			pstRPPILog->uiTCPConnEndTime = pLOG->uiTcpSynAckAckTime;
			pstRPPILog->uiTCPConnEndMTime = pLOG->uiTcpSynAckAckMTime;
		}
		else {
			S64 llGap = 0;
			STG_DeltaTIME64(pstRPPILog->uiTCPConnEndTime, pstRPPILog->uiTCPConnEndMTime, pLOG->uiTcpSynAckAckTime, pLOG->uiTcpSynAckAckMTime, &llGap);
			if(llGap > 0) {
				pstRPPILog->uiTCPConnEndTime = pLOG->uiTcpSynAckAckTime;
				pstRPPILog->uiTCPConnEndMTime = pLOG->uiTcpSynAckAckMTime;
			}
		}

		STG_DiffTIME64(pLOG->uiTcpSynAckAckTime, pLOG->uiTcpSynAckAckMTime, pLOG->uiTcpSynTime, pLOG->uiTcpSynMTime, &llTcpSetupTime);	
		pstRPPILog->llTCPConnDelayedTime += llTcpSetupTime;
		dSvcThreshold = dGetThreshold(SERVICE_TCP, ALARM_TCPSETUPTIME);
		if (dSvcThreshold > 0 && llTcpSetupTime > dSvcThreshold*1000)
			pstRPPILog->uiTCPConnDelayedCnt++;
	}
	else {
		pstRPPILog->uiLastFailReason =  pLOG->usL4FailCode;
	}

	if (pLOG->usSvcL4Type == L4_IV || pLOG->usPlatformType == DEF_PLATFORM_BANK || pLOG->usPlatformType == DEF_PLATFORM_CORP ||
			(pLOG->usPlatformType == DEF_PLATFORM_ETC && 
			 (pLOG->usSvcL4Type == L4_JNC || pLOG->usSvcL4Type == L4_WIPI_ONLINE)))
	{
		if (pstRPPILog->uiFirstPlatformType == 0)
			pstRPPILog->uiFirstPlatformType = pLOG->usPlatformType;
		if (pstRPPILog->uiFirstSvcL4Type == 0)
			pstRPPILog->uiFirstSvcL4Type = pLOG->usSvcL4Type;
		if (pstRPPILog->uiFirstSvcL7Type == 0)
			pstRPPILog->uiFirstSvcL7Type = APP_UNKNOWN;
		pstRPPILog->uiLastPlatformType = pLOG->usPlatformType;
		pstRPPILog->uiLastSvcL4Type = pLOG->usSvcL4Type;
		pstRPPILog->uiLastSvcL7Type = APP_UNKNOWN;

		if (pstRPPILog->uiFirstSvcStartTime == 0)
		{      
			pstRPPILog->uiFirstSvcStartTime = pLOG->uiTcpSynTime;
			pstRPPILog->uiFirstSvcStartMTime = pLOG->uiTcpSynMTime;
			pstRPPILog->uiFirstSvcEndTime = pLOG->uiTcpLastPktTime;
			pstRPPILog->uiFirstSvcEndMTime = pLOG->uiTcpLastPktMTime;
		}   
		pstRPPILog->uiLastSvcStartTime = pLOG->uiTcpSynTime;
		pstRPPILog->uiLastSvcStartMTime = pLOG->uiTcpSynMTime;
		pstRPPILog->uiLastSvcEndTime = pLOG->uiTcpLastPktTime;
		pstRPPILog->uiLastSvcEndMTime = pLOG->uiTcpLastPktMTime;

		/** WATCH DATA */
		st_WatchMsg stWatchMsg;
		memset(&stWatchMsg, 0x00, DEF_WATCHMSG_SIZE);

		stWatchMsg.usMsgType = WATCH_TYPE_SVC;
		stWatchMsg.ucOffice = pstRPPILog->ucBranchID;
		stWatchMsg.ucSYSID = pstRPPILog->ucSYSID;
		stWatchMsg.ucBSCID = pstRPPILog->ucBSCID;
		stWatchMsg.usBTSID = pstRPPILog->ucBTSID;
		stWatchMsg.ucSec = pstRPPILog->ucSECTOR;
		stWatchMsg.ucFA = pstRPPILog->ucFA_ID;
		stWatchMsg.uiPCFIP = pstRPPILog->uiPCFIP;
		stWatchMsg.ucPCFType = dGetPCFType(pstRPPILog->uiPCFIP);
		stWatchMsg.uiPDSNIP = pstRPPILog->uiNASName;
		stWatchMsg.uiAAAIP = pstRPPILog->uiAAAIP;
		stWatchMsg.uiHSSIP = pstRPPILog->uiHSSIP;
		stWatchMsg.ucSvcIdx = dGetSvcIndex(GET_ISRECALL(pstRPPILog->usCallType), pLOG->usPlatformType, pLOG->usSvcL4Type);
		stWatchMsg.usSvcL4Type = pLOG->usSvcL4Type;
		stWatchMsg.uiSVCIP = pLOG->uiServerIP;

		if (pLOG->uiTcpSynAckAckTime == 0)	
			stWatchMsg.uiResult = pLOG->usL4FailCode;
		else 
			stWatchMsg.uiResult = 0;

		dSendMonInfo(&stWatchMsg);
		log_print(LOGN_INFO, "SEND WATCH MSG[%d][%s] OFFICE[%d][%s] RESULT[%d]",
				stWatchMsg.usMsgType, PrintMonType(stWatchMsg.usMsgType),
				stWatchMsg.ucOffice, PrintOFFICE(stWatchMsg.ucOffice), stWatchMsg.uiResult);
	}	

	return 0;
}

S32 dINETSessInfo(U8 *data)
{
	RPPISESS_KEY   	stRPPIKey, *pstRPPIKey;
	HData_RPPI      stRPPIHash, *pstRPPIHash;
	LOG_INET		*pLOG;

	stHASHONODE     *pHASHONODE;
	LOG_RPPI        *pstRPPILog;


	pstRPPIKey = &stRPPIKey;
	pstRPPIHash = &stRPPIHash;

	U8              szIP[INET_ADDRSTRLEN];
	U8              szTime[BUFSIZ];

	S32				isReCall = 0;

	pLOG = (LOG_INET*)data;

	log_print (LOGN_DEBUG, "RCV LOG_INET IMSI[%s] SVC[%u][%s] IP[%s] TIME[%s.%u]", 
			pLOG->szIMSI, pLOG->usServiceType, PrintSVC(pLOG->usServiceType),
			util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);

#if 0
	log_print (LOGN_DEBUG, "RCV LOG_INETIMSI[%s] SVC[%ld][%s] IP[%s] ERROR[%d] TIME[%s.%u]", 
			pLOG->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
			util_cvtipaddr(szIP, pLOG->uiClientIP), pLOG->LastUserErrCode, util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);

	if (pLOG->LastUserErrCode > 0) {
	//	pLOG->LastUserErrCode = SERVICE_DEFECT + SERVICE_MSRP_DEFECT + pLOG->LastUserErrCode;
	}
#endif

	memset(pstRPPIKey, 0x00, RPPISESS_KEY_SIZE);
	memcpy(pstRPPIKey, pLOG->szIMSI, MAX_MIN_LEN);

	if ( (pHASHONODE = hasho_find(pHASHOINFO, (U8*)pstRPPIKey)) == NULL)
	{
//		log_print(LOGN_CRI, "LOG_INET NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
//		log_print(LOGN_WARN, "LOG_INET NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
//				pstRPPIKey->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
//				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		log_print(LOGN_WARN, "LOG_INET NOT FOUND HASH IMSI[%s] ClientIP[%s] TIME[%s.%u]",
				pstRPPIKey->szIMSI, util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1;      
	}

	pstRPPIHash = (HData_RPPI*)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);

	pstRPPILog = pFindRPPILog(0, pstRPPIHash, pLOG->uiAccStartTime, pLOG->uiAccStartMTime);

	if (pstRPPILog == NULL)
	{
//		log_print(LOGN_CRI, "LOG_INET NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
		log_print(LOGN_WARN, "LOG_INET NOT FOUND RPPI LOG IMSI[%s] ClientIP[%s] TIME[%s.%u]",
				pstRPPIKey->szIMSI, util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1;
	}

	isReCall = GET_ISRECALL(pstRPPILog->usCallType);

	if(!isReCall) vCheckFirst(pstRPPIHash, pstRPPILog, (U8*)"INET", pLOG->uiFirstPktTime, pLOG->uiFirstPktMTime);

	dUpdateCommonLog((LOG_COMMON*) pLOG, pstRPPILog);

	pstRPPILog->uiUpTotPktCnt += pLOG->uiUpPacketCnt;
	pstRPPILog->uiDnTotPktCnt += pLOG->uiDnPacketCnt;
	pstRPPILog->uiUpTotDataSize += pLOG->uiUpPacketSize;
	pstRPPILog->uiDnTotDataSize += pLOG->uiDnPacketSize;

	if (pstRPPILog->uiFirstPlatformType == 0)
		pstRPPILog->uiFirstPlatformType = DEF_PLATFORM_INET;
	pstRPPILog->uiLastPlatformType = DEF_PLATFORM_INET; 


//	if (pstRPPILog->uiFirstSvcL4Type == 0)
//		pstRPPILog->uiFirstSvcL4Type = L4_INET;
//	if (pstRPPILog->uiFirstSvcL7Type == 0)
//		pstRPPILog->uiFirstSvcL7Type = APP_INET_SEND; //APP_INET_RECV
//	pstRPPILog->uiLastSvcL4Type = L4_INET;
//	pstRPPILog->uiLastSvcL7Type = APP_INET_SEND;

	if (pstRPPILog->uiFirstSvcStartTime == 0)
	{
		pstRPPILog->uiFirstSvcStartTime = pLOG->uiFirstPktTime;
		pstRPPILog->uiFirstSvcStartMTime = pLOG->uiFirstPktMTime;
		pstRPPILog->uiFirstSvcEndTime = pLOG->uiLastPktTime;
		pstRPPILog->uiFirstSvcEndMTime = pLOG->uiLastPktMTime;
	}
	pstRPPILog->uiLastSvcStartTime = pLOG->uiFirstPktTime;
	pstRPPILog->uiLastSvcStartMTime = pLOG->uiFirstPktMTime;
	pstRPPILog->uiLastSvcEndTime = pLOG->uiLastPktTime;
	pstRPPILog->uiLastSvcEndMTime = pLOG->uiLastPktMTime;


#if 0
	if (pLOG->LastUserErrCode > 0) {
		pstRPPILog->uiLastFailReason = pLOG->LastUserErrCode;
	}
	if (pstRPPILog->uiFirstPlatformType == 0)
		pstRPPILog->uiFirstPlatformType = pLOG->usPlatformType;
	if (pstRPPILog->uiFirstSvcL4Type == 0)
		pstRPPILog->uiFirstSvcL4Type = pLOG->usSvcL4Type;
	if (pstRPPILog->uiFirstSvcL7Type == 0)
		pstRPPILog->uiFirstSvcL7Type = pLOG->usSvcL7Type;
	pstRPPILog->uiLastPlatformType = pLOG->usPlatformType;
	pstRPPILog->uiLastSvcL4Type = pLOG->usSvcL4Type;
	pstRPPILog->uiLastSvcL7Type = pLOG->usSvcL7Type; 	
#endif

	return 0;
}

S32 dITCPSessInfo(U8 *data)
{
	S64 llTcpSetupTime;	
	RPPISESS_KEY   stRPPIKey, *pstRPPIKey;
	HData_RPPI      stRPPIHash, *pstRPPIHash; 
	S32				dSvcThreshold;	
	LOG_ITCP_SESS    *pLOG;

	stHASHONODE     *pHASHONODE;
	LOG_RPPI        *pstRPPILog;

	S32				isReCall = 0;

	pstRPPIKey = &stRPPIKey;
	pstRPPIHash = &stRPPIHash;

	U8              szIP[INET_ADDRSTRLEN];
	U8              szTime[BUFSIZ];
	U32				uiSvcL4Type;
	U32				uiPlatformType;

	pLOG = (LOG_ITCP_SESS *)data;
	log_print (LOGN_DEBUG, "RCV LOG_ITCP IMSI[%s] SVC[%ld][%s] IP[%s] ERROR[%ld] TIME[%s.%u]", pLOG->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
			util_cvtipaddr(szIP, pLOG->uiClientIP), pLOG->usL4FailCode, util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);

	if(pLOG->usL4FailCode > 0)	
		pLOG->usL4FailCode = SERVICE_DEFECT + SERVICE_TCP_DEFECT + pLOG->usL4FailCode;

	memset(pstRPPIKey, 0x00, RPPISESS_KEY_SIZE);
	memcpy(pstRPPIKey, pLOG->szIMSI, MAX_MIN_LEN);

	if ( (pHASHONODE = hasho_find(pHASHOINFO, (U8*)pstRPPIKey)) == NULL)
	{
//		log_print(LOGN_CRI, "LOG_TCP NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
		log_print(LOGN_WARN, "LOG_ITCP NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
				pstRPPIKey->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1;
	}

	pstRPPIHash = (HData_RPPI*)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);
	pstRPPILog = pFindRPPILog(0, pstRPPIHash, pLOG->uiAccStartTime, pLOG->uiAccStartMTime);

	if (pstRPPILog == NULL)
	{
//		log_print(LOGN_CRI, "LOG_TCP NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
		log_print(LOGN_WARN, "LOG_ITCP NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
				pstRPPIKey->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1;
	}

	isReCall = GET_ISRECALL(pstRPPILog->usCallType);

	if(!isReCall) vCheckFirst(pstRPPIHash, pstRPPILog, (U8*)"ITCP", pLOG->uiTcpSessStartTime, pLOG->uiTcpSessStartMTime);

	dUpdateCommonLog((LOG_COMMON*) pLOG, pstRPPILog);

	pstRPPILog->uiTCPSynCnt += pLOG->ucTcpSynCnt;

	pstRPPILog->uiUpTCPPacketCnt += pLOG->uiIPTotUpPktCnt;
	pstRPPILog->uiDnTCPPacketCnt += pLOG->uiIPTotDnPktCnt;

	pstRPPILog->uiUpTCPRetransCnt += pLOG->uiIPTotUpRetransCnt;		
	pstRPPILog->uiDnTCPRetransCnt += pLOG->uiIPTotDnRetransCnt;

#if 0
	/* all packet count -> INET */
	{
		pstRPPILog->uiUpTotPktCnt += pLOG->uiIPTotUpPktCnt;
		pstRPPILog->uiDnTotPktCnt += pLOG->uiIPTotDnPktCnt;

		pstRPPILog->uiUpTotDataSize += pLOG->uiIPTotUpPktSize;
		pstRPPILog->uiDnTotDataSize += pLOG->uiIPTotDnPktSize;
	}
#endif
	if(pstRPPILog->uiFirstTCPSynTime == 0 ) {
		pstRPPILog->uiFirstTCPSynTime = pLOG->uiTcpSessStartTime;
		pstRPPILog->uiFirstTCPSynMTime = pLOG->uiTcpSessStartMTime;
		STG_DiffTIME64(pstRPPILog->uiFirstTCPSynTime, pstRPPILog->uiFirstTCPSynMTime, pstRPPILog->uiCallTime, pstRPPILog->uiCallMTime, &pstRPPILog->llRPTCPSynDelTime);

	}
	else {
		S64 llGap = 0;
		STG_DeltaTIME64(pstRPPILog->uiFirstTCPSynTime, pstRPPILog->uiFirstTCPSynMTime, pLOG->uiTcpSessStartTime, pLOG->uiTcpSessStartMTime, &llGap);
		if(llGap > 0) {
			pstRPPILog->uiFirstTCPSynTime = pLOG->uiTcpSessStartTime;
			pstRPPILog->uiFirstTCPSynMTime = pLOG->uiTcpSessStartMTime;
			STG_DiffTIME64(pstRPPILog->uiFirstTCPSynTime, pstRPPILog->uiFirstTCPSynMTime, pstRPPILog->uiCallTime, pstRPPILog->uiCallMTime, &pstRPPILog->llRPTCPSynDelTime);
		}
	}

	/* 성공한 TCP */
	if(pLOG->uiTcpSynAckAckTime != 0)
//	if(pLOG->uiIPDataUpPktCnt > 0 || pLOG->uiIPDataDnPktCnt > 0) 
	{
		pstRPPILog->uiTCPConnCnt++;
		if (pstRPPILog->uiTCPConnEndTime == 0)
		{
			pstRPPILog->uiTCPConnEndTime = pLOG->uiTcpLastPktTime;
			pstRPPILog->uiTCPConnEndMTime = pLOG->uiTcpLastPktMTime;
		}
		else {
			S64 llGap = 0;
			STG_DeltaTIME64(pstRPPILog->uiTCPConnEndTime, pstRPPILog->uiTCPConnEndMTime, pLOG->uiTcpLastPktTime, pLOG->uiTcpLastPktMTime, &llGap);
			if(llGap > 0) {
				pstRPPILog->uiTCPConnEndTime = pLOG->uiTcpLastPktTime;
				pstRPPILog->uiTCPConnEndMTime = pLOG->uiTcpLastPktMTime;
			}
		}

		STG_DiffTIME64(pLOG->uiTcpSynAckAckTime, pLOG->uiTcpSynAckAckMTime, pLOG->uiTcpSessStartTime, pLOG->uiTcpSessStartMTime, &llTcpSetupTime);	
		pstRPPILog->llTCPConnDelayedTime += llTcpSetupTime;
		dSvcThreshold = dGetThreshold(SERVICE_TCP, ALARM_TCPSETUPTIME);
		if (dSvcThreshold > 0 && llTcpSetupTime > dSvcThreshold*1000)
			pstRPPILog->uiTCPConnDelayedCnt++;
	}
	else {
		pstRPPILog->uiLastFailReason =  pLOG->usL4FailCode;
	}
	
	uiSvcL4Type 	= (isReCall ? L4_INET_TCP_RECV : pLOG->usSvcL4Type); 
	uiPlatformType 	= (!pLOG->usPlatformType) ? DEF_PLATFORM_INET : pLOG->usPlatformType;
	
	if (pstRPPILog->uiFirstPlatformType == 0)
		pstRPPILog->uiFirstPlatformType = uiPlatformType; 
	if (pstRPPILog->uiFirstSvcL4Type == 0)
		pstRPPILog->uiFirstSvcL4Type = uiSvcL4Type; 
	if (pstRPPILog->uiFirstSvcL7Type == 0)
		pstRPPILog->uiFirstSvcL7Type = APP_UNKNOWN;
	pstRPPILog->uiLastPlatformType = uiPlatformType; 
	pstRPPILog->uiLastSvcL4Type = uiSvcL4Type; 
	pstRPPILog->uiLastSvcL7Type = APP_UNKNOWN;

	if (pstRPPILog->uiFirstSvcStartTime == 0)
	{      
		pstRPPILog->uiFirstSvcStartTime = pLOG->uiTcpSessStartTime;
		pstRPPILog->uiFirstSvcStartMTime = pLOG->uiTcpSessStartMTime;
		pstRPPILog->uiFirstSvcEndTime = pLOG->uiTcpLastPktTime;
		pstRPPILog->uiFirstSvcEndMTime = pLOG->uiTcpLastPktMTime;
	}   
	pstRPPILog->uiLastSvcStartTime = pLOG->uiTcpSessStartTime;
	pstRPPILog->uiLastSvcStartMTime = pLOG->uiTcpSessStartMTime;
	pstRPPILog->uiLastSvcEndTime = pLOG->uiTcpLastPktTime;
	pstRPPILog->uiLastSvcEndMTime = pLOG->uiTcpLastPktMTime;


	switch (pLOG->usServerPort) 
	{
		/** HTTP protocal 제외 */ 
		case 8080:
		case 80:
			break;

		default:
			{
				/** WATCH DATA */
				st_WatchMsg stWatchMsg;
				memset(&stWatchMsg, 0x00, DEF_WATCHMSG_SIZE);

				stWatchMsg.usMsgType = WATCH_TYPE_SVC;
				stWatchMsg.ucOffice = pstRPPILog->ucBranchID;
				stWatchMsg.ucSYSID = pstRPPILog->ucSYSID;
				stWatchMsg.ucBSCID = pstRPPILog->ucBSCID;
				stWatchMsg.usBTSID = pstRPPILog->ucBTSID;
				stWatchMsg.ucSec = pstRPPILog->ucSECTOR;
				stWatchMsg.ucFA = pstRPPILog->ucFA_ID;
				stWatchMsg.uiPCFIP = pstRPPILog->uiPCFIP;
				stWatchMsg.ucPCFType = dGetPCFType(pstRPPILog->uiPCFIP);
				stWatchMsg.uiPDSNIP = pstRPPILog->uiNASName;
				stWatchMsg.uiAAAIP = pstRPPILog->uiAAAIP;
				stWatchMsg.uiHSSIP = pstRPPILog->uiHSSIP;
				stWatchMsg.ucSvcIdx = dGetSvcIndex(isReCall, uiPlatformType, pLOG->usSvcL4Type);
				stWatchMsg.usSvcL4Type = pLOG->usSvcL4Type;
				stWatchMsg.uiSVCIP = pLOG->uiServerIP;

				if (pLOG->uiTcpSynAckAckTime == 0)	
//				if (pLOG->uiTcpLastPktTime == 0)	
					stWatchMsg.uiResult = pLOG->usL4FailCode;
				else 
					stWatchMsg.uiResult = 0;

				dSendMonInfo(&stWatchMsg);
				log_print(LOGN_INFO, "SEND WATCH MSG[%d][%s] OFFICE[%d][%s] RESULT[%d]",
						stWatchMsg.usMsgType, PrintMonType(stWatchMsg.usMsgType),
						stWatchMsg.ucOffice, PrintOFFICE(stWatchMsg.ucOffice), stWatchMsg.uiResult);
			}
			break;
	}	

	return 0;
}

S32 dIHTTPSessInfo(U8 *data)
{
	RPPISESS_KEY   stRPPIKey, *pstRPPIKey;
	HData_RPPI      stRPPIHash, *pstRPPIHash;
	S64				llUpLoadingTime, llDnLoadingTime;
	LOG_IHTTP_TRANS	*pLOG;

	stHASHONODE     *pHASHONODE;
	LOG_RPPI        *pstRPPILog;

	S32				isReCall = 0;

	pstRPPIKey = &stRPPIKey;
	pstRPPIHash = &stRPPIHash;

	U8              szIP[INET_ADDRSTRLEN];
	U8              szTime[BUFSIZ];
	U32				uiSvcL4Type;
	U32				uiPlatformType;

	pLOG = (LOG_IHTTP_TRANS*)data;
	log_print (LOGN_DEBUG, "RCV LOG_IHTTP IMSI[%s] SVC[%ld][%s] IP[%s] ERROR[%d] TIME[%s.%u]", 
			pLOG->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
			util_cvtipaddr(szIP, pLOG->uiClientIP), pLOG->usUserErrorCode, util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);

	if (pLOG->usUserErrorCode > 0) 
		pLOG->usUserErrorCode = SERVICE_DEFECT + SERVICE_HTTP_DEFECT + pLOG->usUserErrorCode;

	memset(&stRPPIKey, 0x00, RPPISESS_KEY_SIZE);
	memcpy(pstRPPIKey, pLOG->szIMSI, MAX_MIN_LEN);

	if ( (pHASHONODE = hasho_find(pHASHOINFO, (U8*)pstRPPIKey)) == NULL)
	{
//		log_print(LOGN_CRI, "LOG_HTTP NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
		log_print(LOGN_WARN, "LOG_IHTTP NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
				pstRPPIKey->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1;
	}   

	pstRPPIHash = (HData_RPPI*)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);

	pstRPPILog = pFindRPPILog(0, pstRPPIHash, pLOG->uiAccStartTime, pLOG->uiAccStartMTime);

	if (pstRPPILog == NULL)
	{
//		log_print(LOGN_CRI, "LOG_HTTP NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
		log_print(LOGN_WARN, "LOG_IHTTP NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
				pstRPPIKey->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1;
	}
	
	isReCall = GET_ISRECALL(pstRPPILog->usCallType);

	if(!isReCall) vCheckFirst(pstRPPIHash, pstRPPILog, (U8*)"IHTTP", pLOG->uiReqStartTime, pLOG->uiReqStartMTime);

	dUpdateCommonLog((LOG_COMMON*) pLOG, pstRPPILog);
	
	if (pLOG->usUserErrorCode > 0) {
		pstRPPILog->uiLastFailReason = pLOG->usUserErrorCode;
	}

	/* 지연 요소, 응답시간  체크 안함 */
#if 0
	switch (pLOG->usPlatformType)
	{
		case DEF_PLATFORM_MENU:
		case DEF_PLATFORM_EMS:
		case DEF_PLATFORM_FV:
			pstRPPILog->uiHTTPGetCnt++;
			if (pstRPPILog->uiHTTPStartReqTime == 0)
			{
				pstRPPILog->uiHTTPStartReqTime = pLOG->uiReqStartTime;
				pstRPPILog->uiHTTPStartReqMTime = pLOG->uiReqStartMTime;
			}
			if (pstRPPILog->uiHTTPMNAckTime == 0)
			{
				pstRPPILog->uiHTTPMNAckTime = pLOG->uiMNAckTime;
				pstRPPILog->uiHTTPMNAckMTime = pLOG->uiMNAckMTime;
			}

			if (dSvcThreshold > 0 && pLOG->llTransGapTime > dSvcThreshold*1000) {
				pstRPPILog->uiLastFailReason = (SERVICE_DEFECT + SERVICE_DELAY_DEFECT + RESPONSETIME);
				pLOG->usUserErrorCode = pstRPPILog->uiLastFailReason;
				pstRPPILog->uiHTTPDelayedCnt++;
			}

			if (pLOG->usUserErrorCode == 0) {
				pstRPPILog->uiHTTPAckCnt++;
				pstRPPILog->llHTTPDelayedTime += pLOG->llTransGapTime;
			}
			break;
	}
#endif
			
	STG_DiffTIME64(pLOG->uiReqEndTime, pLOG->uiReqEndMTime, pLOG->uiReqStartTime, pLOG->uiReqStartMTime, &llUpLoadingTime);
	STG_DiffTIME64(pLOG->uiLastPktTime, pLOG->uiLastPktMTime, pLOG->uiResStartTime, pLOG->uiResStartMTime, &llDnLoadingTime);

	if (llUpLoadingTime > 0 ) {
		pstRPPILog->uiUpDataSize += pLOG->uiTcpUpBodySize;
		pstRPPILog->llUpDataLoadingTime += llUpLoadingTime;
	}
	if (llDnLoadingTime > 0) {
		pstRPPILog->uiDnDataSize += pLOG->uiTcpDnBodySize;
		pstRPPILog->llDnDataLoadingTime += llDnLoadingTime;
	}

	if (pstRPPILog->uiHTTPStartReqTime == 0)
	{
		pstRPPILog->uiHTTPStartReqTime = pLOG->uiReqStartTime;
		pstRPPILog->uiHTTPStartReqMTime = pLOG->uiReqStartMTime;
	}
	if (pstRPPILog->uiHTTPMNAckTime == 0)
	{
		pstRPPILog->uiHTTPMNAckTime = pLOG->uiMNAckTime;
		pstRPPILog->uiHTTPMNAckMTime = pLOG->uiMNAckMTime;
	}
	
	if (pLOG->usUserErrorCode == 0) {
		pstRPPILog->llHTTPDelayedTime += pLOG->llTransGapTime;
	}
	
	uiSvcL4Type = (isReCall ? L4_INET_HTTP_RECV : pLOG->usSvcL4Type); 
	uiPlatformType 	= (!pLOG->usPlatformType) ? DEF_PLATFORM_INET : pLOG->usPlatformType;
	
	if (pstRPPILog->uiFirstPlatformType == 0)
		pstRPPILog->uiFirstPlatformType = uiPlatformType; 
	if (pstRPPILog->uiFirstSvcL4Type == 0)
		pstRPPILog->uiFirstSvcL4Type = uiSvcL4Type; 
	if (pstRPPILog->uiFirstSvcL7Type == 0)
		pstRPPILog->uiFirstSvcL7Type = pLOG->usSvcL7Type;
	pstRPPILog->uiLastPlatformType = uiPlatformType; 
	pstRPPILog->uiLastSvcL4Type = uiSvcL4Type; 
	pstRPPILog->uiLastSvcL7Type = pLOG->usSvcL7Type;

	if (pstRPPILog->uiFirstSvcStartTime == 0)
	{
		pstRPPILog->uiFirstSvcStartTime = pLOG->uiReqStartTime;
		pstRPPILog->uiFirstSvcStartMTime = pLOG->uiReqStartMTime;
		pstRPPILog->uiFirstSvcEndTime = pLOG->uiLastPktTime;
		pstRPPILog->uiFirstSvcEndMTime = pLOG->uiLastPktMTime;
	}
	pstRPPILog->uiLastSvcStartTime = pLOG->uiReqStartTime;
	pstRPPILog->uiLastSvcStartMTime = pLOG->uiReqStartMTime;
	pstRPPILog->uiLastSvcEndTime = pLOG->uiLastPktTime;
	pstRPPILog->uiLastSvcEndMTime = pLOG->uiLastPktMTime;

	/** WATCH DATA */
	st_WatchMsg stWatchMsg;
	memset(&stWatchMsg, 0x00, DEF_WATCHMSG_SIZE);

	stWatchMsg.usMsgType = WATCH_TYPE_SVC;
	stWatchMsg.ucOffice = pstRPPILog->ucBranchID; 
	stWatchMsg.ucSYSID = pstRPPILog->ucSYSID;
	stWatchMsg.ucBSCID = pstRPPILog->ucBSCID;
	stWatchMsg.usBTSID = pstRPPILog->ucBTSID;
	stWatchMsg.ucSec = pstRPPILog->ucSECTOR;
	stWatchMsg.ucFA = pstRPPILog->ucFA_ID;
	stWatchMsg.uiPCFIP = pstRPPILog->uiPCFIP;
	stWatchMsg.ucPCFType = dGetPCFType(pstRPPILog->uiPCFIP);
	stWatchMsg.uiPDSNIP = pstRPPILog->uiNASName;
	stWatchMsg.uiAAAIP = pstRPPILog->uiAAAIP;
	stWatchMsg.uiHSSIP = pstRPPILog->uiHSSIP;
	stWatchMsg.ucSvcIdx = dGetSvcIndex(isReCall, uiPlatformType, pLOG->usSvcL4Type);
	stWatchMsg.usSvcL4Type = pLOG->usSvcL4Type;
	stWatchMsg.uiSVCIP = pLOG->uiServerIP; 
	stWatchMsg.uiResult = pLOG->usUserErrorCode;

	dSendMonInfo(&stWatchMsg);
	log_print(LOGN_INFO, "SEND WATCH MSG[%d][%s] OFFICE[%d][%s] RESULT[%d]",
			stWatchMsg.usMsgType, PrintMonType(stWatchMsg.usMsgType),
			stWatchMsg.ucOffice, PrintOFFICE(stWatchMsg.ucOffice), stWatchMsg.uiResult);

	return 0;
}

S32 dSIPSessInfo(U8 *data)
{
	RPPISESS_KEY   stRPPIKey, *pstRPPIKey;
	HData_RPPI      stRPPIHash, *pstRPPIHash;
	LOG_SIP_TRANS    *pLOG;
	stHASHONODE     *pHASHONODE; 
	LOG_RPPI        *pstRPPILog;


	pstRPPIKey = &stRPPIKey;
	pstRPPIHash = &stRPPIHash;

	U8              szIP[INET_ADDRSTRLEN];
	U8              szTime[BUFSIZ];

	pLOG = (LOG_SIP_TRANS *)data;
	log_print (LOGN_DEBUG, "RCV LOG_SIP IMSI[%s] SVC[%ld][%s] IP[%s] ERROR[%d] TIME[%s.%u]", 
			pLOG->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
			util_cvtipaddr(szIP, pLOG->uiClientIP), pLOG->LastUserErrCode, util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);

	if (pLOG->LastUserErrCode > 0) 
		pLOG->LastUserErrCode = SERVICE_DEFECT + SERVICE_SIP_DEFECT + pLOG->LastUserErrCode;

	memset(pstRPPIKey, 0x00, RPPISESS_KEY_SIZE);
	memcpy(pstRPPIKey, pLOG->szIMSI, MAX_MIN_LEN);

	if ( (pHASHONODE = hasho_find(pHASHOINFO, (U8*)pstRPPIKey)) == NULL)
	{
//		log_print(LOGN_CRI, "LOG_SIP NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
		log_print(LOGN_WARN, "LOG_SIP NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
				pstRPPIKey->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1;       
	}

	pstRPPIHash = (HData_RPPI*)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);

	pstRPPILog = pFindRPPILog(0, pstRPPIHash, pLOG->uiAccStartTime, pLOG->uiAccStartMTime);

	if (pstRPPILog == NULL)
	{
//		log_print(LOGN_CRI, "LOG_SIP NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
		log_print(LOGN_WARN, "LOG_SIP NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
				pstRPPIKey->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1;
	}

	vCheckFirst(pstRPPIHash, pstRPPILog, (U8*)"SIP", pLOG->TransStartTime, pLOG->TransStartMTime);

	dUpdateCommonLog((LOG_COMMON*) pLOG, pstRPPILog);

	if (!pLOG->isUsed)
	{
		if (pLOG->usSvcL7Type == APP_IM_UP)
		{
			pstRPPILog->uiUpTotPktCnt += pLOG->TotalReqCnt;
			pstRPPILog->uiDnTotPktCnt += pLOG->TotalResCnt;
			pstRPPILog->uiUpTotDataSize += pLOG->ReqIPDataSize;
			pstRPPILog->uiDnTotDataSize += pLOG->ResIPDataSize;

		}
		else
		{
			pstRPPILog->uiUpTotPktCnt += pLOG->TotalResCnt;
			pstRPPILog->uiDnTotPktCnt += pLOG->TotalReqCnt;
			pstRPPILog->uiUpTotDataSize += pLOG->ResIPDataSize;
			pstRPPILog->uiDnTotDataSize += pLOG->ReqIPDataSize;
		}
	
		if (pstRPPILog->uiFirstPlatformType == 0)
				pstRPPILog->uiFirstPlatformType = pLOG->usPlatformType;
		if (pstRPPILog->uiFirstSvcL4Type == 0)
				pstRPPILog->uiFirstSvcL4Type = pLOG->usSvcL4Type;
		if (pstRPPILog->uiFirstSvcL7Type == 0)
				pstRPPILog->uiFirstSvcL7Type = pLOG->usSvcL7Type;
		pstRPPILog->uiLastPlatformType = pLOG->usPlatformType;
		pstRPPILog->uiLastSvcL4Type = pLOG->usSvcL4Type;
		pstRPPILog->uiLastSvcL7Type = pLOG->usSvcL7Type;

		if (pstRPPILog->uiFirstSvcStartTime == 0)
		{
				pstRPPILog->uiFirstSvcStartTime = pLOG->TransStartTime;
				pstRPPILog->uiFirstSvcStartMTime = pLOG->TransStartMTime;
				pstRPPILog->uiFirstSvcEndTime = pLOG->TransEndTime;
				pstRPPILog->uiFirstSvcEndMTime = pLOG->TransEndMTime;
		}
		pstRPPILog->uiLastSvcStartTime = pLOG->TransStartTime;
		pstRPPILog->uiLastSvcStartMTime = pLOG->TransStartMTime;
		pstRPPILog->uiLastSvcEndTime = pLOG->TransEndTime;
		pstRPPILog->uiLastSvcEndMTime = pLOG->TransEndMTime;
	}

	if (pLOG->LastUserErrCode > 0) {
		pstRPPILog->uiLastFailReason = pLOG->LastUserErrCode;
	}

	if (pstRPPILog->uiFirstTCPSynTime == 0) {
		pstRPPILog->uiFirstTCPSynTime = pLOG->TransStartTime;
		pstRPPILog->uiFirstTCPSynMTime = pLOG->TransStartMTime;
		STG_DiffTIME64(pstRPPILog->uiFirstTCPSynTime, pstRPPILog->uiFirstTCPSynMTime, pstRPPILog->uiCallTime, pstRPPILog->uiCallMTime, &pstRPPILog->llRPTCPSynDelTime);
	}
	else {
		S64 llGap = 0;
		STG_DeltaTIME64(pstRPPILog->uiFirstTCPSynTime, pstRPPILog->uiFirstTCPSynMTime, pLOG->TransStartTime, pLOG->TransStartMTime, &llGap);
		if(llGap > 0) {
			pstRPPILog->uiFirstTCPSynTime = pLOG->TransStartTime;
			pstRPPILog->uiFirstTCPSynMTime = pLOG->TransStartMTime;
			STG_DiffTIME64(pstRPPILog->uiFirstTCPSynTime, pstRPPILog->uiFirstTCPSynMTime, pstRPPILog->uiCallTime, pstRPPILog->uiCallMTime, &pstRPPILog->llRPTCPSynDelTime);
		}
	}

/*
	if (pLOG->method != SIP_MSG_BYE	&& pLOG->method < SIP_MSG_INVITE_ADHOC 
			&& pLOG->method > SIP_MSG_INVITE)
*/
	if(!pLOG->isUsed)
	{
		/** WATCH DATA */
		st_WatchMsg stWatchMsg;
		memset(&stWatchMsg, 0x00, DEF_WATCHMSG_SIZE);

		stWatchMsg.usMsgType = WATCH_TYPE_SVC;
		stWatchMsg.ucOffice = pstRPPILog->ucBranchID;
		stWatchMsg.ucSYSID = pstRPPILog->ucSYSID;
		stWatchMsg.ucBSCID = pstRPPILog->ucBSCID;
		stWatchMsg.usBTSID = pstRPPILog->ucBTSID;
		stWatchMsg.ucSec = pstRPPILog->ucSECTOR;
		stWatchMsg.ucFA = pstRPPILog->ucFA_ID;
		stWatchMsg.uiPCFIP = pstRPPILog->uiPCFIP;
		stWatchMsg.ucPCFType = dGetPCFType(pstRPPILog->uiPCFIP);
		stWatchMsg.uiPDSNIP = pstRPPILog->uiNASName;
		stWatchMsg.uiAAAIP = pstRPPILog->uiAAAIP;
		stWatchMsg.uiHSSIP = pstRPPILog->uiHSSIP;
		stWatchMsg.ucSvcIdx = (pLOG->method == SIP_MSG_REGISTER) ? SVC_IDX_REGI : dGetSvcIndex(GET_ISRECALL(pstRPPILog->usCallType), pLOG->usPlatformType, pLOG->usSvcL7Type);
		stWatchMsg.usSvcL4Type = pLOG->usSvcL4Type;
		stWatchMsg.uiSVCIP = pLOG->DestIP;
		stWatchMsg.uiResult = pLOG->LastUserErrCode;

		dSendMonInfo(&stWatchMsg);
		log_print(LOGN_INFO, "SEND WATCH MSG[%d][%s] OFFICE[%d][%s] RESULT[%d]",
				stWatchMsg.usMsgType, PrintMonType(stWatchMsg.usMsgType),
				stWatchMsg.ucOffice, PrintOFFICE(stWatchMsg.ucOffice), stWatchMsg.uiResult);
	}
	return 0;
}

S32 dMSRPSessInfo(U8 *data)
{
	RPPISESS_KEY   	stRPPIKey, *pstRPPIKey;
	HData_RPPI      stRPPIHash, *pstRPPIHash;
	LOG_MSRP_TRANS    *pLOG;

	stHASHONODE     *pHASHONODE;
	LOG_RPPI        *pstRPPILog;


	pstRPPIKey = &stRPPIKey;
	pstRPPIHash = &stRPPIHash;

	U8              szIP[INET_ADDRSTRLEN];
	U8              szTime[BUFSIZ];

	pLOG = (LOG_MSRP_TRANS *)data;
	log_print (LOGN_DEBUG, "RCV LOG_MSRP IMSI[%s] SVC[%ld][%s] IP[%s] ERROR[%d] TIME[%s.%u]", 
			pLOG->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
			util_cvtipaddr(szIP, pLOG->uiClientIP), pLOG->LastUserErrCode, util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);

	if (pLOG->LastUserErrCode > 0) 
		pLOG->LastUserErrCode = SERVICE_DEFECT + SERVICE_MSRP_DEFECT + pLOG->LastUserErrCode;

	memset(pstRPPIKey, 0x00, RPPISESS_KEY_SIZE);
	memcpy(pstRPPIKey, pLOG->szIMSI, MAX_MIN_LEN);

	if ( (pHASHONODE = hasho_find(pHASHOINFO, (U8*)pstRPPIKey)) == NULL)
	{
//		log_print(LOGN_CRI, "LOG_MSRP NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
		log_print(LOGN_WARN, "LOG_MSRP NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
				pstRPPIKey->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1;      
	}

	pstRPPIHash = (HData_RPPI*)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);

	pstRPPILog = pFindRPPILog(0, pstRPPIHash, pLOG->uiAccStartTime, pLOG->uiAccStartMTime);

	if (pstRPPILog == NULL)
	{
//		log_print(LOGN_CRI, "LOG_MSRP NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
		log_print(LOGN_WARN, "LOG_MSRP NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
				pstRPPIKey->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1;
	}

	vCheckFirst(pstRPPIHash, pstRPPILog, (U8*)"MSRP", pLOG->TransStartTime, pLOG->TransStartMTime);

	dUpdateCommonLog((LOG_COMMON*) pLOG, pstRPPILog);

	/* MSRP 예외 처리 : contenttype = 'application/im-iscomposing+xml' 이면 제외 */
	char	szTmpMsrp[BUFSIZ] = "application/im-iscomposing+xml";
	if(!strncmp((char*)pLOG->ContentsType, szTmpMsrp, strlen(szTmpMsrp))) {
		log_print(LOGN_DEBUG, "SKIP LOG_MSRP ContentType=%.*s", MSRP_CONTENTTYPE_SIZE, pLOG->ContentsType);
		return 0;
	}

	if (pLOG->LastUserErrCode > 0) {
		pstRPPILog->uiLastFailReason = pLOG->LastUserErrCode;
	}
	if (pstRPPILog->uiFirstPlatformType == 0)
		pstRPPILog->uiFirstPlatformType = pLOG->usPlatformType;
	if (pstRPPILog->uiFirstSvcL4Type == 0)
		pstRPPILog->uiFirstSvcL4Type = pLOG->usSvcL4Type;
	if (pstRPPILog->uiFirstSvcL7Type == 0)
		pstRPPILog->uiFirstSvcL7Type = pLOG->usSvcL7Type;
	pstRPPILog->uiLastPlatformType = pLOG->usPlatformType;
	pstRPPILog->uiLastSvcL4Type = pLOG->usSvcL4Type;
	pstRPPILog->uiLastSvcL7Type = pLOG->usSvcL7Type; 	

	/** WATCH DATA */
	st_WatchMsg stWatchMsg;
	memset(&stWatchMsg, 0x00, DEF_WATCHMSG_SIZE);

	stWatchMsg.usMsgType = WATCH_TYPE_SVC;
	stWatchMsg.ucOffice = pstRPPILog->ucBranchID;
	stWatchMsg.ucSYSID = pstRPPILog->ucSYSID;
	stWatchMsg.ucBSCID = pstRPPILog->ucBSCID;
	stWatchMsg.usBTSID = pstRPPILog->ucBTSID;
	stWatchMsg.ucSec = pstRPPILog->ucSECTOR;
	stWatchMsg.ucFA = pstRPPILog->ucFA_ID;
	stWatchMsg.uiPCFIP = pstRPPILog->uiPCFIP;
	stWatchMsg.ucPCFType = dGetPCFType(pstRPPILog->uiPCFIP);
	stWatchMsg.uiPDSNIP = pstRPPILog->uiNASName;
	stWatchMsg.uiAAAIP = pstRPPILog->uiAAAIP;
	stWatchMsg.uiHSSIP = pstRPPILog->uiHSSIP;
	stWatchMsg.ucSvcIdx = dGetSvcIndex(GET_ISRECALL(pstRPPILog->usCallType), pLOG->usPlatformType, pLOG->usSvcL7Type);
	stWatchMsg.usSvcL4Type = pLOG->usSvcL4Type;
	stWatchMsg.usSvcL4Type = pLOG->usSvcL4Type;
	stWatchMsg.uiSVCIP = pLOG->DestIP;
	stWatchMsg.uiResult = pLOG->LastUserErrCode;

	dSendMonInfo(&stWatchMsg);
	log_print(LOGN_INFO, "SEND WATCH MSG[%d][%s] OFFICE[%d][%s] RESULT[%d]",
			stWatchMsg.usMsgType, PrintMonType(stWatchMsg.usMsgType),
			stWatchMsg.ucOffice, PrintOFFICE(stWatchMsg.ucOffice), stWatchMsg.uiResult);	
	return 0;
}

S32 dVTSessInfo(U8 *data)
{
	RPPISESS_KEY   stRPPIKey, *pstRPPIKey;
	HData_RPPI      stRPPIHash, *pstRPPIHash;
	U32             uiUpJitterThreshold, uiDnJitterThreshold, uiUpLossThreshold, uiDnLossThreshold; 
	LOG_VT_SESS  	*pLOG;

	stHASHONODE     *pHASHONODE;
	LOG_RPPI        *pstRPPILog;


	pstRPPIKey = &stRPPIKey;
	pstRPPIHash = &stRPPIHash;

	U8              szIP[INET_ADDRSTRLEN];
	U8              szTime[BUFSIZ];

	pLOG = (LOG_VT_SESS *)data;
	log_print (LOGN_DEBUG, "RCV LOG_VT IMSI[%s] SVC[%ld][%s] IP[%s] ERROR[%d] TIME[%s.%u]", 
			pLOG->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
			util_cvtipaddr(szIP, pLOG->uiClientIP), pLOG->LastUserErrCode, util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);

	if (pLOG->LastUserErrCode > 0) 
		pLOG->LastUserErrCode = SERVICE_DEFECT + SERVICE_VT_DEFECT + pLOG->LastUserErrCode;

	memset(pstRPPIKey, 0x00, RPPISESS_KEY_SIZE);
	memcpy(pstRPPIKey, pLOG->szIMSI, MAX_MIN_LEN);

	if ( (pHASHONODE = hasho_find(pHASHOINFO, (U8*)pstRPPIKey)) == NULL)
	{
//		log_print(LOGN_CRI, "LOG_VT NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
		log_print(LOGN_WARN, "LOG_VT NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
				pstRPPIKey->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1;
	}

	pstRPPIHash = (HData_RPPI*)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);

	pstRPPILog = pFindRPPILog(0, pstRPPIHash, pLOG->uiAccStartTime, pLOG->uiAccStartMTime);

	if (pstRPPILog == NULL)
	{
//		log_print(LOGN_CRI, "LOG_VT NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
		log_print(LOGN_WARN, "LOG_VT NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
				pstRPPIKey->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1;
	}

	vCheckFirst(pstRPPIHash, pstRPPILog, (U8*)"VT", pLOG->SessStartTime, pLOG->SessStartMTime);

	dUpdateCommonLog((LOG_COMMON*) pLOG, pstRPPILog);

	pstRPPILog->uiVTReqCnt++;
	if (pLOG->SetupEndTime !=0)
		pstRPPILog->uiVTSetupCnt++;
	if (pLOG->LastUserErrCode > 0) {	
		pstRPPILog->uiLastFailReason = pLOG->LastUserErrCode;
	}

	/** 지연 요소 **/
	uiUpJitterThreshold = dGetThreshold(SERVICE_VT, ALARM_UPJITTER);
	uiDnJitterThreshold = dGetThreshold(SERVICE_VT, ALARM_DNJITTER);
	uiUpLossThreshold = dGetThreshold(SERVICE_VT, ALARM_UPPACKETLOSS);
	uiUpLossThreshold = dGetThreshold(SERVICE_VT, ALARM_DNPACKETLOSS);

	if (uiUpJitterThreshold > 0 && pLOG->UpMaxJitter > uiUpJitterThreshold) {
		pstRPPILog->uiLastFailReason = (SERVICE_DEFECT + SERVICE_DELAY_DEFECT + JITTER);
		pLOG->LastUserErrCode = pstRPPILog->uiLastFailReason;
		pstRPPILog->uiRTPJitterOverCnt++;
	}

	if (uiDnJitterThreshold > 0 && pLOG->DnMaxJitter > uiDnJitterThreshold) {
		pstRPPILog->uiLastFailReason = (SERVICE_DEFECT + SERVICE_DELAY_DEFECT + JITTER);
		pLOG->LastUserErrCode = pstRPPILog->uiLastFailReason;
		pstRPPILog->uiRTPJitterOverCnt++;
	}

	if (uiUpLossThreshold > 0 && pLOG->RTPUpLossCnt > uiUpLossThreshold) {
		pstRPPILog->uiLastFailReason = (SERVICE_DEFECT + SERVICE_DELAY_DEFECT + PACKETLOSS);
		pLOG->LastUserErrCode = pstRPPILog->uiLastFailReason;
		pstRPPILog->uiRTPLossOverCnt++;
	}

	if (uiDnLossThreshold > 0 && pLOG->RTPDnLossCnt > uiDnLossThreshold) {
		pstRPPILog->uiLastFailReason = (SERVICE_DEFECT + SERVICE_DELAY_DEFECT + PACKETLOSS);
		pLOG->LastUserErrCode = pstRPPILog->uiLastFailReason;
		pstRPPILog->uiRTPLossOverCnt++;
	}
	/////////////////

	pstRPPILog->uiRTPUpCnt += pLOG->RTPUpCnt;
	pstRPPILog->uiRTPDnCnt += pLOG->RTPDnCnt;
	pstRPPILog->uiRTPUpLossCnt += pLOG->RTPUpLossCnt;
	pstRPPILog->uiRTPDnLossCnt += pLOG->RTPDnLossCnt;
	pstRPPILog->uiRTPUpDataSize += pLOG->RTPUpDataSize;
	pstRPPILog->uiRTPDnDataSize += pLOG->RTPDnDataSize;
	pstRPPILog->uiRTPUpMaxJitter = max(pstRPPILog->uiRTPUpMaxJitter, pLOG->UpMaxJitter);
	pstRPPILog->uiRTPDnMaxJitter = max(pstRPPILog->uiRTPDnMaxJitter, pLOG->DnMaxJitter);

	pstRPPILog->uiUpTotPktCnt += pLOG->uiIPTotUpPktCnt;
	pstRPPILog->uiDnTotPktCnt += pLOG->uiIPTotDnPktCnt;
	pstRPPILog->uiUpTotDataSize += pLOG->uiIPTotUpPktSize;
	pstRPPILog->uiDnTotDataSize += pLOG->uiIPTotDnPktSize;


	if (pstRPPILog->uiFirstPlatformType == 0)
		pstRPPILog->uiFirstPlatformType = pLOG->usPlatformType;
	if (pstRPPILog->uiFirstSvcL4Type == 0)
		pstRPPILog->uiFirstSvcL4Type = pLOG->usSvcL4Type;
	if (pstRPPILog->uiFirstSvcL7Type == 0)
		pstRPPILog->uiFirstSvcL7Type = pLOG->usSvcL7Type;
	pstRPPILog->uiLastPlatformType = pLOG->usPlatformType;
	pstRPPILog->uiLastSvcL4Type = pLOG->usSvcL4Type;
	pstRPPILog->uiLastSvcL7Type = pLOG->usSvcL7Type;

	if (pstRPPILog->uiFirstSvcStartTime == 0)
	{
		pstRPPILog->uiFirstSvcStartTime = pLOG->SessStartTime;
		pstRPPILog->uiFirstSvcStartMTime = pLOG->SessStartMTime;
		pstRPPILog->uiFirstSvcEndTime = pLOG->LastPktTime;
		pstRPPILog->uiFirstSvcEndMTime = pLOG->LastPktMTime;
	}
	pstRPPILog->uiLastSvcStartTime = pLOG->SessStartTime;
	pstRPPILog->uiLastSvcStartMTime = pLOG->SessStartMTime;
	pstRPPILog->uiLastSvcEndTime = pLOG->LastPktTime;
	pstRPPILog->uiLastSvcEndMTime = pLOG->LastPktMTime; 

	/** WATCH DATA */
	st_WatchMsg stWatchMsg;
	memset(&stWatchMsg, 0x00, DEF_WATCHMSG_SIZE);

	stWatchMsg.usMsgType = WATCH_TYPE_SVC;
	stWatchMsg.ucOffice = pstRPPILog->ucBranchID; 
	stWatchMsg.ucSYSID = pstRPPILog->ucSYSID;
	stWatchMsg.ucBSCID = pstRPPILog->ucBSCID;
	stWatchMsg.usBTSID = pstRPPILog->ucBTSID;
	stWatchMsg.ucSec = pstRPPILog->ucSECTOR;
	stWatchMsg.ucFA = pstRPPILog->ucFA_ID;
	stWatchMsg.uiPCFIP = pstRPPILog->uiPCFIP;
	stWatchMsg.ucPCFType = dGetPCFType(pstRPPILog->uiPCFIP);
	stWatchMsg.uiPDSNIP = pstRPPILog->uiNASName;
	stWatchMsg.uiAAAIP = pstRPPILog->uiAAAIP;
	stWatchMsg.uiHSSIP = pstRPPILog->uiHSSIP;
	stWatchMsg.ucSvcIdx = dGetSvcIndex(GET_ISRECALL(pstRPPILog->usCallType), pLOG->usPlatformType, pLOG->usSvcL7Type);
	stWatchMsg.usSvcL4Type = pLOG->usSvcL4Type;
	stWatchMsg.uiSVCIP = pLOG->ServerIP;
	stWatchMsg.uiResult = (pLOG->SetupEndTime > 0) ? 0 : (SERVICE_DEFECT + SERVICE_VT_DEFECT + VT_UERR_NOSETUP);

	dSendMonInfo(&stWatchMsg);
	log_print(LOGN_INFO, "SEND WATCH MSG[%d][%s] OFFICE[%d][%s] RESULT[%d]",
			stWatchMsg.usMsgType, PrintMonType(stWatchMsg.usMsgType),
			stWatchMsg.ucOffice, PrintOFFICE(stWatchMsg.ucOffice), stWatchMsg.uiResult);
	return 0;
}

S32 dIMSessInfo(U8 *data)
{   
	RPPISESS_KEY   	stRPPIKey, *pstRPPIKey;
	HData_RPPI      stRPPIHash, *pstRPPIHash;
	S32				dSvcThreshold; 
	LOG_IM_SESS  *pLOG;

	stHASHONODE     *pHASHONODE;
	LOG_RPPI        *pstRPPILog;


	pstRPPIKey = &stRPPIKey;
	pstRPPIHash = &stRPPIHash;

	U8              szIP[INET_ADDRSTRLEN];
	U8              szTime[BUFSIZ];

	pLOG = (LOG_IM_SESS *)data;
	log_print (LOGN_DEBUG, "RCV LOG_IM IMSI[%s] SVC[%ld][%s] IP[%s] ERROR[%d] TIME[%s.%u]", 
			pLOG->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
			util_cvtipaddr(szIP, pLOG->uiClientIP), pLOG->LastUserErrCode, util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);

	if (pLOG->LastUserErrCode >0 )     
		pLOG->LastUserErrCode = (SERVICE_DEFECT + SERVICE_IM_DEFECT + pLOG->LastUserErrCode);

	memset(pstRPPIKey, 0x00, RPPISESS_KEY_SIZE);
	memcpy(pstRPPIKey, pLOG->szIMSI, MAX_MIN_LEN);

	if ( (pHASHONODE = hasho_find(pHASHOINFO, (U8*)pstRPPIKey)) == NULL)
	{
//		log_print(LOGN_CRI, "LOG_IM NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
		log_print(LOGN_WARN, "LOG_IM NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
				pstRPPIKey->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1;
	}   

	pstRPPIHash = (HData_RPPI*)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);

	pstRPPILog = pFindRPPILog(0, pstRPPIHash, pLOG->uiAccStartTime, pLOG->uiAccStartMTime);

	if (pstRPPILog == NULL)
	{
//		log_print(LOGN_CRI, "LOG_IM NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
		log_print(LOGN_WARN, "LOG_IM NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
				pstRPPIKey->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1;
	}

	vCheckFirst(pstRPPIHash, pstRPPILog, (U8*)"IM", pLOG->SessStartTime, pLOG->SessStartMTime);

	dUpdateCommonLog((LOG_COMMON*) pLOG, pstRPPILog);

	pstRPPILog->uiIMReqCnt++;
	if (pstRPPILog->uiIMStartReqTime == 0)
	{
		pstRPPILog->uiIMStartReqTime = pLOG->SessStartTime;
		pstRPPILog->uiIMStartReqMTime = pLOG->SessStartMTime;
	}
	if (pstRPPILog->uiIMSetupTime == 0)
	{
		pstRPPILog->uiIMSetupTime = pLOG->SetupEndTime;
		pstRPPILog->uiIMSetupMTime = pLOG->SetupEndMTime;
	}

	if (pLOG->SetupEndTime != 0) {
		pstRPPILog->uiIMSetupCnt++;
	}
	if (pLOG->LastUserErrCode >0 ){          
		pstRPPILog->uiLastFailReason = pLOG->LastUserErrCode;
	}


	/***** 지연 요소 ******/
	dSvcThreshold = dGetThreshold(SERVICE_IM, ALARM_RESPONSETIME);
	if (dSvcThreshold > 0 && pLOG->SessGapTime > dSvcThreshold*1000) {
		pstRPPILog->uiLastFailReason = (SERVICE_DEFECT + SERVICE_DELAY_DEFECT + RESPONSETIME);
		pLOG->LastUserErrCode = pstRPPILog->uiLastFailReason;
	}
	/////////////////////////// 

	if (pstRPPILog->uiFirstPlatformType == 0)
		pstRPPILog->uiFirstPlatformType = pLOG->usPlatformType;
	if (pstRPPILog->uiFirstSvcL4Type == 0)
		pstRPPILog->uiFirstSvcL4Type = pLOG->usSvcL4Type;
	if (pstRPPILog->uiFirstSvcL7Type == 0)
		pstRPPILog->uiFirstSvcL7Type = pLOG->usSvcL7Type;
	pstRPPILog->uiLastPlatformType = pLOG->usPlatformType;
	pstRPPILog->uiLastSvcL4Type = pLOG->usSvcL4Type;
	pstRPPILog->uiLastSvcL7Type = pLOG->usSvcL7Type;

	if (pstRPPILog->uiFirstSvcStartTime == 0)
	{
		pstRPPILog->uiFirstSvcStartTime = pLOG->SessStartTime;
		pstRPPILog->uiFirstSvcStartMTime = pLOG->SessStartMTime;
		pstRPPILog->uiFirstSvcEndTime = pLOG->LastPktTime;
		pstRPPILog->uiFirstSvcEndMTime = pLOG->LastPktMTime;
	}
	pstRPPILog->uiLastSvcStartTime = pLOG->SessStartTime;
	pstRPPILog->uiLastSvcStartMTime = pLOG->SessStartMTime;
	pstRPPILog->uiLastSvcEndTime = pLOG->LastPktTime;
	pstRPPILog->uiLastSvcEndMTime = pLOG->LastPktMTime;  

	pstRPPILog->uiUpTotPktCnt += pLOG->TotalReqCnt;
	pstRPPILog->uiDnTotPktCnt += pLOG->TotalResCnt;
	pstRPPILog->uiUpTotDataSize += pLOG->ReqIPDataSize;
	pstRPPILog->uiDnTotDataSize += pLOG->ResIPDataSize;

	/** WATCH DATA */
	st_WatchMsg stWatchMsg;
	memset(&stWatchMsg, 0x00, DEF_WATCHMSG_SIZE);

	stWatchMsg.usMsgType = WATCH_TYPE_SVC;
	stWatchMsg.ucOffice = pstRPPILog->ucBranchID;
	stWatchMsg.ucSYSID = pstRPPILog->ucSYSID;
	stWatchMsg.ucBSCID = pstRPPILog->ucBSCID;
	stWatchMsg.usBTSID = pstRPPILog->ucBTSID;
	stWatchMsg.ucSec = pstRPPILog->ucSECTOR;
	stWatchMsg.ucFA = pstRPPILog->ucFA_ID;
	stWatchMsg.uiPCFIP = pstRPPILog->uiPCFIP;
	stWatchMsg.ucPCFType = dGetPCFType(pstRPPILog->uiPCFIP);
	stWatchMsg.uiPDSNIP = pstRPPILog->uiNASName;
	stWatchMsg.uiAAAIP = pstRPPILog->uiAAAIP;
	stWatchMsg.uiHSSIP = pstRPPILog->uiHSSIP;
	stWatchMsg.ucSvcIdx = dGetSvcIndex(GET_ISRECALL(pstRPPILog->usCallType), pLOG->usPlatformType, pLOG->usSvcL7Type);
	stWatchMsg.usSvcL4Type = pLOG->usSvcL4Type;
	stWatchMsg.uiSVCIP = pLOG->ServerIP;
	stWatchMsg.uiResult = (pLOG->SetupEndTime > 0) ? 0 : (SERVICE_DEFECT + SERVICE_IM_DEFECT + IM_UERR_NOSETUP);

	dSendMonInfo(&stWatchMsg);
	log_print(LOGN_INFO, "SEND WATCH MSG[%d][%s] OFFICE[%d][%s] RESULT[%d]",
			stWatchMsg.usMsgType, PrintMonType(stWatchMsg.usMsgType),
			stWatchMsg.ucOffice, PrintOFFICE(stWatchMsg.ucOffice), stWatchMsg.uiResult);
	return 0;
}

S32 dFTPSessInfo(U8 *data)
{   
	RPPISESS_KEY    stRPPIKey, *pstRPPIKey;
	HData_RPPI      stRPPIHash, *pstRPPIHash;
	LOG_FTP			*pLOG;

	stHASHONODE     *pHASHONODE;
	LOG_RPPI        *pstRPPILog;


	pstRPPIKey = &stRPPIKey;
	pstRPPIHash = &stRPPIHash;

	U8              szIP[INET_ADDRSTRLEN];
	U8              szTime[BUFSIZ];

	pLOG = (LOG_FTP *)data;
	log_print (LOGN_DEBUG, "RCV LOG_FTP IMSI[%s] SVC[%ld][%s] IP[%s] TIME[%s.%u]", 
			pLOG->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
			util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);


	memset(pstRPPIKey, 0x00, RPPISESS_KEY_SIZE);
	memcpy(pstRPPIKey, pLOG->szIMSI, MAX_MIN_LEN);

	if ( (pHASHONODE = hasho_find(pHASHOINFO, (U8*)pstRPPIKey)) == NULL)
	{
//		log_print(LOGN_CRI, "LOG_FTP NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
		log_print(LOGN_WARN, "LOG_FTP NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
				pstRPPIKey->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1;
	}

	pstRPPIHash = (HData_RPPI*)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);

	pstRPPILog = pFindRPPILog(0, pstRPPIHash, pLOG->uiAccStartTime, pLOG->uiAccStartMTime);

	if (pstRPPILog == NULL)
	{   
//		log_print(LOGN_CRI, "LOG_FTP NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
		log_print(LOGN_WARN, "LOG_FTP NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
				pstRPPIKey->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1;
	}

	vCheckFirst(pstRPPIHash, pstRPPILog, (U8*)"FTP", pLOG->uiTcpSynTime, pLOG->uiTcpSynMTime);

	dUpdateCommonLog((LOG_COMMON*) pLOG, pstRPPILog);

	if (pstRPPILog->uiFirstPlatformType == 0)
		pstRPPILog->uiFirstPlatformType = pLOG->usPlatformType;
	if (pstRPPILog->uiFirstSvcL4Type == 0)
		pstRPPILog->uiFirstSvcL4Type = pLOG->usSvcL4Type;
	if (pstRPPILog->uiFirstSvcL7Type == 0)
		pstRPPILog->uiFirstSvcL7Type = pLOG->usSvcL7Type;
	pstRPPILog->uiLastPlatformType = pLOG->usPlatformType;
	pstRPPILog->uiLastSvcL4Type = pLOG->usSvcL4Type;
	pstRPPILog->uiLastSvcL7Type = pLOG->usSvcL7Type;

	if (pstRPPILog->uiFirstSvcStartTime == 0)
	{
		pstRPPILog->uiFirstSvcStartTime = pLOG->uiFTPSynTime;
		pstRPPILog->uiFirstSvcStartMTime = pLOG->uiFTPSynMTime;
		pstRPPILog->uiFirstSvcEndTime = pLOG->uiFTPFinTime;
		pstRPPILog->uiFirstSvcEndMTime = pLOG->uiFTPFinMTime;
	}
	pstRPPILog->uiLastSvcStartTime = pLOG->uiFTPSynTime;
	pstRPPILog->uiLastSvcStartMTime = pLOG->uiFTPSynMTime;
	pstRPPILog->uiLastSvcEndTime = pLOG->uiFTPFinTime;
	pstRPPILog->uiLastSvcEndMTime = pLOG->uiFTPFinMTime;

	return 0;
}

S32 dDIALUPSessInfo(U8 *data)
{
	RPPISESS_KEY    stRPPIKey, *pstRPPIKey;
	HData_RPPI      stRPPIHash, *pstRPPIHash;
	LOG_DIALUP_SESS         *pLOG;

	stHASHONODE     *pHASHONODE;
	LOG_RPPI        *pstRPPILog;


	pstRPPIKey = &stRPPIKey;
	pstRPPIHash = &stRPPIHash;

	U8              szIP[INET_ADDRSTRLEN];
	U8              szTime[BUFSIZ];

	pLOG = (LOG_DIALUP_SESS *)data;
	log_print (LOGN_DEBUG, "RCV LOG_DIALUP IMSI[%s] SVC[%ld][%s] IP[%s] TIME[%s.%u]", 
			pLOG->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
			util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);


	memset(pstRPPIKey, 0x00, RPPISESS_KEY_SIZE);
	memcpy(pstRPPIKey, pLOG->szIMSI, MAX_MIN_LEN);

	if ( (pHASHONODE = hasho_find(pHASHOINFO, (U8*)pstRPPIKey)) == NULL)
	{
//		log_print(LOGN_CRI, "LOG_DIALUP NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
		log_print(LOGN_WARN, "LOG_DIALUP NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
				pstRPPIKey->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1;
	}

	pstRPPIHash = (HData_RPPI*)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);

	pstRPPILog = pFindRPPILog(0, pstRPPIHash, pLOG->uiAccStartTime, pLOG->uiAccStartMTime);

	if (pstRPPILog == NULL)
	{
//		log_print(LOGN_CRI, "LOG_DIALUP NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
		log_print(LOGN_WARN, "LOG_DIALUP NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
				pstRPPIKey->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1;
	}

	vCheckFirst(pstRPPIHash, pstRPPILog, (U8*)"DIALUP", pLOG->SessStartTime, pLOG->SessStartMTime);

	dUpdateCommonLog((LOG_COMMON*) pLOG, pstRPPILog);

	pstRPPILog->uiUpTotPktCnt += pLOG->UpPktCnt;
	pstRPPILog->uiDnTotPktCnt += pLOG->DnPktCnt;
	pstRPPILog->uiUpTotDataSize += pLOG->UpPktSize;
	pstRPPILog->uiDnTotDataSize += pLOG->DnPktSize;

	if (pstRPPILog->uiFirstPlatformType == 0)
		pstRPPILog->uiFirstPlatformType = pLOG->usPlatformType;
	if (pstRPPILog->uiFirstSvcL4Type == 0)
		pstRPPILog->uiFirstSvcL4Type = pLOG->usSvcL4Type;
	if (pstRPPILog->uiFirstSvcL7Type == 0)
		pstRPPILog->uiFirstSvcL7Type = pLOG->usSvcL7Type;
	pstRPPILog->uiLastPlatformType = pLOG->usPlatformType;
	pstRPPILog->uiLastSvcL4Type = pLOG->usSvcL4Type;
	pstRPPILog->uiLastSvcL7Type = pLOG->usSvcL7Type;

	if (pstRPPILog->uiFirstSvcStartTime == 0)
	{
		pstRPPILog->uiFirstSvcStartTime = pLOG->SessStartTime;
		pstRPPILog->uiFirstSvcStartMTime = pLOG->SessStartMTime;
		pstRPPILog->uiFirstSvcEndTime = pLOG->LastPktTime;
		pstRPPILog->uiFirstSvcEndMTime = pLOG->LastPktMTime;
	}
	pstRPPILog->uiLastSvcStartTime = pLOG->SessStartTime;
	pstRPPILog->uiLastSvcStartMTime = pLOG->SessStartMTime;
	pstRPPILog->uiLastSvcEndTime = pLOG->LastPktTime;
	pstRPPILog->uiLastSvcEndMTime = pLOG->LastPktMTime;

	/** WATCH DATA */
	st_WatchMsg stWatchMsg;
	memset(&stWatchMsg, 0x00, DEF_WATCHMSG_SIZE);

	stWatchMsg.usMsgType = WATCH_TYPE_SVC;
	stWatchMsg.ucOffice = pstRPPILog->ucBranchID;
	stWatchMsg.ucSYSID = pstRPPILog->ucSYSID;
	stWatchMsg.ucBSCID = pstRPPILog->ucBSCID;
	stWatchMsg.usBTSID = pstRPPILog->ucBTSID;
	stWatchMsg.ucSec = pstRPPILog->ucSECTOR;
	stWatchMsg.ucFA = pstRPPILog->ucFA_ID;
	stWatchMsg.uiPCFIP = pstRPPILog->uiPCFIP;
	stWatchMsg.ucPCFType = dGetPCFType(pstRPPILog->uiPCFIP);
	stWatchMsg.uiPDSNIP = pstRPPILog->uiNASName;
	stWatchMsg.uiAAAIP = pstRPPILog->uiAAAIP;
	stWatchMsg.uiHSSIP = pstRPPILog->uiHSSIP;
	stWatchMsg.ucSvcIdx = dGetSvcIndex(GET_ISRECALL(pstRPPILog->usCallType), pLOG->usPlatformType, pLOG->usSvcL7Type);
	stWatchMsg.usSvcL4Type = pLOG->usSvcL4Type;
	stWatchMsg.uiSVCIP = 0;
	stWatchMsg.uiResult = 0;

	dSendMonInfo(&stWatchMsg);
	log_print(LOGN_INFO, "SEND WATCH MSG[%d][%s] OFFICE[%d][%s] RESULT[%d]",
			stWatchMsg.usMsgType, PrintMonType(stWatchMsg.usMsgType),
			stWatchMsg.ucOffice, PrintOFFICE(stWatchMsg.ucOffice), stWatchMsg.uiResult);

	return 0;
} 

S32 dNotiSigInfo(U8 *data)
{
	NOTIFY_SIG  *pNOTIFY;
	pNOTIFY = (NOTIFY_SIG *)data;

	switch (pNOTIFY->uiType)
	{
		case MID_FLT_ACCESS:
			log_print(LOGN_CRI, "RCV NOTIFY SIGNAL FLT ACCESS");
			dMakePCFHash();
			break;
		case MID_FLT_DEFECT_THRES:
			log_print(LOGN_CRI, "RCV NOTIFY SIGNAL FLT DEFFECT");
			dMakeDefectHash();
			break;
		case MID_FLT_MODEL:
			log_print(LOGN_CRI, "RCV NOTIFY SIGNAL FLT MODEL");
			dMakeModelHash();
			break;
		default :
			log_print(LOGN_CRI, "[%s][%s.%d] UNKNOWN Type[%d]", __FILE__, __FUNCTION__, __LINE__, pNOTIFY->uiType);	
	}
	return 0;
}

S32 dStartServiceInfo(U8 *data)
{
	RPPISESS_KEY    stRPPIKey, *pstRPPIKey;
	HData_RPPI      stRPPIHash, *pstRPPIHash;
	LOG_COMMON         *pLOG;
	OFFSET			offset;
	S32				isSendWatch;
	S32				isReCall;
	S32				dState;

	stHASHONODE     *pHASHONODE;
	LOG_RPPI        *pstRPPILog;


	pstRPPIKey = &stRPPIKey;
	pstRPPIHash = &stRPPIHash;

	U8              szIP[INET_ADDRSTRLEN];
	U8              szTime[BUFSIZ];
	U8              szSTime[BUFSIZ];

	pLOG = (LOG_COMMON *)data;
	log_print (LOGN_DEBUG, "RCV FIRST START SERVICE IMSI[%s] IP[%s] TIME[%s.%u] SVCTIME[%s.%u]", 
			pLOG->szIMSI, util_cvtipaddr(szIP, pLOG->uiClientIP),
			util_printtime(pLOG->uiAccStartTime, szTime), pLOG->uiAccStartMTime,
			util_printtime(pLOG->uiCallTime, szSTime), pLOG->uiCallMTime);


	memset(pstRPPIKey, 0x00, RPPISESS_KEY_SIZE);
	memcpy(pstRPPIKey, pLOG->szIMSI, MAX_MIN_LEN);

	if ( (pHASHONODE = hasho_find(pHASHOINFO, (U8*)pstRPPIKey)) == NULL)
	{
//		log_print(LOGN_CRI, "FIRST START NOT FOUND HASH IMSI[%s] ClientIP[%s] TIME[%s.%u] SVCTIME[%s.%u]",
		log_print(LOGN_WARN, "FIRST START NOT FOUND HASH IMSI[%s] ClientIP[%s] TIME[%s.%u] SVCTIME[%s.%u]",
				pstRPPIKey->szIMSI,  util_cvtipaddr(szIP, pLOG->uiClientIP), 
				util_printtime(pLOG->uiAccStartTime, szTime), pLOG->uiAccStartMTime,
				util_printtime(pLOG->uiCallTime, szSTime), pLOG->uiCallMTime);
		return -1;
	}

	pstRPPIHash = (HData_RPPI*)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);

	pstRPPILog = pFindRPPILog(0, pstRPPIHash, pLOG->uiAccStartTime, pLOG->uiAccStartMTime);

	if (pstRPPILog == NULL)
	{
//		log_print(LOGN_CRI, "FIRST START NOT FOUND RPPI LOG IMSI[%s] ClientIP[%s] TIME[%s.%u] SVCTIME[%s.%u]",
		log_print(LOGN_WARN, "FIRST START NOT FOUND RPPI LOG IMSI[%s] ClientIP[%s] TIME[%s.%u] SVCTIME[%s.%u]",
				pstRPPIKey->szIMSI, util_cvtipaddr(szIP, pLOG->uiClientIP),
				util_printtime(pLOG->uiAccStartTime, szTime), pLOG->uiAccStartMTime,
				util_printtime(pLOG->uiCallTime, szSTime), pLOG->uiCallMTime);
		return -1;
	}

	isReCall 	= GET_ISRECALL(pstRPPILog->usCallType);
	dState		= (isReCall ? RECALL_ON_SERVICE_STATE : ON_SERVICE_STATE);
	
	pstRPPILog->usCallState = dGetCallState(pstRPPILog->usCallState, dState);

	/* UP IPCP PPP_SETUP이 없는 경우 First Service 시간을 세팅 함 : First Service 시간은 예외적으로 CallTime을 사용 */
	if(pstRPPILog->uiPPPSetupTime == 0) {
		pstRPPILog->uiPPPSetupTime = pLOG->uiCallTime;
		pstRPPILog->uiPPPSetupMTime = pLOG->uiCallMTime;
	}

	offset = nifo_get_offset_node(pMEMSINFO, (U8*)pstRPPILog);

	if (pstRPPIHash->before.dOffset == offset) {
		if (pstRPPIHash->before.uiFirstServFlag == 0)
			isSendWatch = 1;
		else
			isSendWatch = 0;	
		pstRPPIHash->before.uiFirstServFlag = 1;
	}
	else if (pstRPPIHash->after.dOffset == offset) {
		if (pstRPPIHash->after.uiFirstServFlag == 0)
			isSendWatch = 1;
		else
			isSendWatch = 0;	
		pstRPPIHash->after.uiFirstServFlag = 1;
	}
	else {
//		log_print(LOGN_CRI, "FIRST START NOT FOUND OFFSET IMSI[%s] ClientIP[%s] TIME[%s.%u] SVCTIME[%s.%u]",
		log_print(LOGN_WARN, "FIRST START NOT FOUND OFFSET IMSI[%s] ClientIP[%s] TIME[%s.%u] SVCTIME[%s.%u] ReCall[%d]",
				pstRPPIKey->szIMSI, util_cvtipaddr(szIP, pLOG->uiClientIP),
				util_printtime(pLOG->uiAccStartTime, szTime), pLOG->uiAccStartMTime,
				util_printtime(pLOG->uiCallTime, szSTime), pLOG->uiCallMTime, isReCall);
		return -1;
	}

	if (isSendWatch)
	{
		/** WATCH DATA */
		st_WatchMsg stWatchMsg;
		memset(&stWatchMsg, 0x00, DEF_WATCHMSG_SIZE);

		if (isReCall) {
			stWatchMsg.usMsgType = WATCH_TYPE_RECALL;
		}
		else {
			stWatchMsg.usMsgType = WATCH_TYPE_A11;
		}
		stWatchMsg.ucOffice = pstRPPILog->ucBranchID;
		stWatchMsg.ucSYSID = pstRPPILog->ucSYSID;
		stWatchMsg.ucBSCID = pstRPPILog->ucBSCID;
		stWatchMsg.usBTSID = pstRPPILog->ucBTSID;
		stWatchMsg.ucSec = pstRPPILog->ucSECTOR;
		stWatchMsg.ucFA = pstRPPILog->ucFA_ID;
		stWatchMsg.uiPCFIP = pstRPPILog->uiPCFIP;
		stWatchMsg.ucPCFType = dGetPCFType(pstRPPILog->uiPCFIP);
		stWatchMsg.uiPDSNIP = pstRPPILog->uiNASName;
		stWatchMsg.uiAAAIP = pstRPPILog->uiAAAIP;
		stWatchMsg.uiHSSIP = pstRPPILog->uiHSSIP;
		stWatchMsg.ucSvcIdx = 0;
		stWatchMsg.usSvcL4Type = 0;
		stWatchMsg.uiSVCIP = 0;
		stWatchMsg.uiResult = 0;

		dSendMonInfo(&stWatchMsg);
		log_print(LOGN_INFO, "SEND WATCH MSG[%d][%s] OFFICE[%d][%s] RESULT[%d]",
				stWatchMsg.usMsgType, PrintMonType(stWatchMsg.usMsgType),
				stWatchMsg.ucOffice, PrintOFFICE(stWatchMsg.ucOffice), stWatchMsg.uiResult);
	}	
	return 0;	
}

S32 dDNSSessInfo(U8 *data)
{
	RPPISESS_KEY    stRPPIKey, *pstRPPIKey;
	HData_RPPI      stRPPIHash, *pstRPPIHash;
	LOG_DNS         *pLOG;

	stHASHONODE     *pHASHONODE;
	LOG_RPPI        *pstRPPILog;


	pstRPPIKey = &stRPPIKey;
	pstRPPIHash = &stRPPIHash;

	U8              szIP[INET_ADDRSTRLEN];
	U8              szTime[BUFSIZ];

	pLOG = (LOG_DNS *)data;
	log_print (LOGN_DEBUG, "RCV LOG_DNS IMSI[%s] SVC[%ld][%s] IP[%s] ERROR[%d] TIME[%s.%u]", 
			pLOG->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
			util_cvtipaddr(szIP, pLOG->uiClientIP), pLOG->ucErrorCode, util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);


	memset(pstRPPIKey, 0x00, RPPISESS_KEY_SIZE);
	memcpy(pstRPPIKey, pLOG->szIMSI, MAX_MIN_LEN);

	if ( (pHASHONODE = hasho_find(pHASHOINFO, (U8*)pstRPPIKey)) == NULL)
	{
//		log_print(LOGN_CRI, "LOG_DNS NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
		log_print(LOGN_WARN, "LOG_DNS NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
				pstRPPIKey->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1;
	}

	pstRPPIHash = (HData_RPPI*)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);

	pstRPPILog = pFindRPPILog(0, pstRPPIHash, pLOG->uiAccStartTime, pLOG->uiAccStartMTime);

	if (pstRPPILog == NULL)
	{
//		log_print(LOGN_CRI, "LOG_DNS NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]", 
		log_print(LOGN_WARN, "LOG_DNS NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]", 
				pstRPPIKey->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1;
	}

	vCheckFirst(pstRPPIHash, pstRPPILog, (U8*)"DNS", pLOG->dRequestTime, pLOG->dRequestMTime);

	dUpdateCommonLog((LOG_COMMON*) pLOG, pstRPPILog);

	pstRPPILog->uiDNSReqCnt += pLOG->usRequestCnt;
	pstRPPILog->uiDNSResCnt += pLOG->usResponseCnt;

/*
	pstRPPILog->uiDNSReqCnt++;
	if (pLOG->ucErrorCode == 0)
		pstRPPILog->uiDNSResCnt++;
*/

	if (pstRPPILog->uiDNSReqTime == 0)
	{
		pstRPPILog->uiDNSReqTime = pLOG->dRequestTime;
		pstRPPILog->uiDNSReqMTime = pLOG->dRequestMTime;
	}
	pstRPPILog->uiDNSResTime = pLOG->dResponseTime;
	pstRPPILog->uiDNSResMTime = pLOG->dResponseMTime;

	pstRPPILog->uiDNSCode = pLOG->ucErrorCode;

/*
	pstRPPILog->uiUpTotPktCnt += pLOG->usRequestCnt;
	pstRPPILog->uiDnTotPktCnt += pLOG->usResponseCnt;
	pstRPPILog->uiUpTotDataSize += pLOG->uiRequestSize;
	pstRPPILog->uiDnTotDataSize += pLOG->uiResponseSize;
*/

	return 0;
}

S32 dDelRPPI(HData_RPPI *pstRPPIHash, LOG_RPPI *pstRPPILog)
{
	RPPISESS_KEY   stRPPIKey, *pstRPPIKey;

	pstRPPIKey = &stRPPIKey;	

	struct timeval  stNowTime;
	gettimeofday(&stNowTime, NULL);
	pstRPPILog->uiOpEndTime = stNowTime.tv_sec;
	pstRPPILog->uiOpEndMTime = stNowTime.tv_usec;

	dSendLogRPPI(pstRPPIHash, pstRPPILog);

	return 0;
}

S32 dUpdateRPPI(LOG_RPPI *pstRPPILog, LOG_SIGNAL *pstSIGNAL, HData_RPPI *pstRPPIHash) 
{
	S32				updateFlag = 0;
	S32 			dSvcThreshold, dErrorFlag;
#ifndef DISABLE_ANALYZE_DIAMETER /* changed by uamyd 20101014 */
	S64				llDuration;
#endif

	LOG_RPPI_ERR 	*pstErrLog;
	st_WatchMsg stWatchMsg;
	OFFSET			offset;
//	U8              szIP[INET_ADDRSTRLEN];
	U32				uiFirstServFlag;
	S32				isReCall;
	U8              szPDSNIP[INET_ADDRSTRLEN];

	dErrorFlag = 0;
	if (pstRPPILog->uiClientIP ==0 && pstSIGNAL->uiProtoType != DIAMETER_PROTO)
	{
		pstRPPILog->uiClientIP = pstSIGNAL->uiClientIP;
	}

	isReCall = GET_ISRECALL(pstRPPILog->usCallType);

#if 0
/*
	if (pstRPPILog->uiNASName == 0) {
		pstRPPILog->uiNASName = pstSIGNAL->uiNASName;
	} 
*/

/*
	if (pstRPPILog->szMIN[0] == 0x00) {
		memcpy(pstRPPILog->szMIN, pstSIGNAL->szMIN, MAX_MIN_LEN);
	}
*/

	if (pstRPPILog->szNetOption[0] == 0x00) {
		memcpy(pstRPPILog->szNetOption, pstSIGNAL->szNetOption, MAX_SVCOPTION_LEN);
		pstRPPILog->usSvcOption = pstSIGNAL->uiSvcOption;
	}

	if (pstSIGNAL->szBSMSC[0] != 0x00 )
	{
		if (pstRPPILog->szBSMSC[0] !=0x00 && memcmp(pstSIGNAL->szBSMSC, pstRPPILog->szBSMSC, DEF_BSMSD_LENGTH) !=0) {
			pstRPPILog->usBSMSCChgCnt++;
		}

		memcpy(pstRPPILog->szBSMSC, pstSIGNAL->szBSMSC, DEF_BSMSD_LENGTH);

		if (pstRPPILog->szFirstBSMSC[0] == 0x00)
			memcpy(pstRPPILog->szFirstBSMSC, pstSIGNAL->szBSMSC, DEF_BSMSD_LENGTH);	
	}

	if (pstRPPILog->uiPCFIP == 0) {
		pstRPPILog->uiPCFIP = pstSIGNAL->uiPCFIP;
		pstRPPILog->ucBranchID = dGetBranchID(pstRPPILog->uiPCFIP);
	}

	if (pstRPPILog->ucFA_ID == 0) {
		pstRPPILog->ucFA_ID = pstSIGNAL->ucFA_ID;
	}
	if (pstRPPILog->ucSECTOR == 0) {
		pstRPPILog->ucSECTOR = pstSIGNAL->ucSECTOR;
	}
	if (pstRPPILog->ucSYSID == 0) {
		pstRPPILog->ucSYSID = pstSIGNAL->ucSYSID;
	}
	if (pstRPPILog->ucBSCID == 0) {
		pstRPPILog->ucBSCID = pstSIGNAL->ucBSCID;
	}
	if (pstRPPILog->ucBTSID == 0) {
		pstRPPILog->ucBTSID = pstSIGNAL->ucBTSID;
	}
#endif
	switch(pstSIGNAL->uiProtoType)
	{
		case START_CALL_NUM:
			log_print(LOGN_INFO," START CALL PROTOTYPE=%u, MSGTYPE=%u PDSNIP=%s", pstSIGNAL->uiProtoType, pstSIGNAL->uiMsgType, util_cvtipaddr(szPDSNIP, pstSIGNAL->uiDestIP));
			pstRPPILog->uiNASName = pstSIGNAL->uiDestIP;

			memcpy(pstRPPILog->szNetOption, pstSIGNAL->szNetOption, MAX_SVCOPTION_LEN);
			pstRPPILog->usSvcOption = pstSIGNAL->uiSvcOption;
			memcpy(pstRPPILog->szBSMSC, pstSIGNAL->szBSMSC, DEF_BSMSD_LENGTH);
			memcpy(pstRPPILog->szFirstBSMSC, pstSIGNAL->szBSMSC, DEF_BSMSD_LENGTH);	
			pstRPPILog->uiPCFIP = pstSIGNAL->uiPCFIP;
			pstRPPILog->ucBranchID = dGetBranchID(pstRPPILog->uiPCFIP);
			pstRPPILog->ucFA_ID = pstSIGNAL->ucFA_ID;
			pstRPPILog->ucSECTOR = pstSIGNAL->ucSECTOR;
			pstRPPILog->ucSYSID = pstSIGNAL->ucSYSID;
			pstRPPILog->ucBSCID = pstSIGNAL->ucBSCID;
			pstRPPILog->ucBTSID = pstSIGNAL->ucBTSID;
			break;
		case A11_PROTO:
			log_print (LOGN_INFO, "UPDATE SIGNALLOG IMSI[%s] PROTOTYPE[%d] MSGTYPE[%d] [%s]",
					pstSIGNAL->szIMSI, pstSIGNAL->uiProtoType,
					pstSIGNAL->uiMsgType, PrintMsg(pstSIGNAL));
			switch(pstSIGNAL->uiMsgType)
			{
				case A11_REGIREQ_MSG:
					dSvcThreshold = dGetThreshold(SERVICE_A11, ALARM_RESPONSETIME);
					pstRPPILog->usRegiReqCnt++;
					switch(pstSIGNAL->ucAirLink)
					{
						case CONNSETUP_ACTIVE_START:
							pstRPPILog->usActiveStartCnt++;
							pstRPPILog->llRPDuration = pstSIGNAL->uiSessDuration;
							if(pstRPPILog->usCallType == 0) pstRPPILog->usCallType = INIT_CALLSTART;
							pstRPPILog->usCallState = dGetCallState(pstRPPILog->usCallState, A11_REGI_STATE);
							updateFlag = 1;
							break;
						case ACTIVE_START:
							pstRPPILog->usActiveStartCnt++;
							pstRPPILog->llRPDuration = pstSIGNAL->uiSessDuration;
							if(pstRPPILog->usCallType == 0) pstRPPILog->usCallType = REACT_CALLSTART;
							pstRPPILog->usCallState = dGetCallState(pstRPPILog->usCallState, A11_REGI_STATE);
							updateFlag = 1;
							break;
						case ACTIVE_STOP:
							pstRPPILog->usActiveStopCnt++;
							updateFlag = 0;
							break;
						case ACTIVE_START_STOP:
							pstRPPILog->usActiveStartCnt++;
							pstRPPILog->llRPDuration = pstSIGNAL->uiSessDuration;
							if(pstRPPILog->usCallType == 0) pstRPPILog->usCallType = REACT_CALLSTART;
							pstRPPILog->usCallState = dGetCallState(pstRPPILog->usCallState, A11_REGI_STATE);
							pstRPPILog->usActiveStopCnt++;
							updateFlag = 1;
							break;
						default:
							pstRPPILog->llRPDuration = pstSIGNAL->uiSessDuration;
							log_print (LOGN_WARN, "UNKNOWN AIRLINK[%u] IMSI[%s] PROTOTYPE[%d] MSGTYPE[%d] [%s]",
									pstSIGNAL->ucAirLink, pstSIGNAL->szIMSI, pstSIGNAL->uiProtoType, 
									pstSIGNAL->uiMsgType, PrintMsg(pstSIGNAL));
							updateFlag = 0;
							break;
					}

					if(updateFlag > 0)
					{
						memcpy(pstRPPILog->szNetOption, pstSIGNAL->szNetOption, MAX_SVCOPTION_LEN);
						pstRPPILog->usSvcOption = pstSIGNAL->uiSvcOption;

						if(memcmp(pstSIGNAL->szBSMSC, pstRPPILog->szBSMSC, DEF_BSMSD_LENGTH) != 0) {
							pstRPPILog->usBSMSCChgCnt++;
						}

						memcpy(pstRPPILog->szBSMSC, pstSIGNAL->szBSMSC, DEF_BSMSD_LENGTH);
						pstRPPILog->uiPCFIP = pstSIGNAL->uiPCFIP;
						pstRPPILog->ucBranchID = dGetBranchID(pstRPPILog->uiPCFIP);
						pstRPPILog->ucFA_ID = pstSIGNAL->ucFA_ID;
						pstRPPILog->ucSECTOR = pstSIGNAL->ucSECTOR;
						pstRPPILog->ucSYSID = pstSIGNAL->ucSYSID;
						pstRPPILog->ucBSCID = pstSIGNAL->ucBSCID;
						pstRPPILog->ucBTSID = pstSIGNAL->ucBTSID;
					}

					if (pstSIGNAL->ucErrorCode > 0 )
					{
						pstRPPILog->uiLastFailReason = (A11_DEFECT + A11_REGI_REPLY + pstSIGNAL->ucErrorCode);
						pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(isReCall, pstRPPILog->usCallState, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
						dErrorFlag = 1;
					}
					if (dSvcThreshold > 0 && pstSIGNAL->uiSessDuration > dSvcThreshold*1000) {
						pstRPPILog->uiLastFailReason = (A11_DEFECT + A11_REGI_REPLY + RESPONSETIME);
						dErrorFlag = 1;
					}

					if (dErrorFlag == 0)
						pstRPPILog->usRegiSuccRepCnt++;
					else
					{
						log_print(LOGN_INFO, "SIGNAL ERRCODE[%d] LastFailReason[%u]", pstSIGNAL->ucErrorCode, pstRPPILog->uiLastFailReason);
						pstErrLog = dCreateErrLog(pstRPPILog);
						if (pstErrLog != NULL)
						{
							pstErrLog->uiSessStartTime = pstSIGNAL->uiSessStartTime;
							pstErrLog->uiSessStartMTime = pstSIGNAL->uiSessStartMTime;
							pstErrLog->usProtoType = A11_PROTO; 
							pstErrLog->uiErrorCode = pstRPPILog->uiLastFailReason;
						}
					}

					pstRPPILog->uiPCFIP  = pstSIGNAL->uiSrcIP;
					pstRPPILog->ucBranchID = dGetBranchID(pstRPPILog->uiPCFIP);
					pstRPPILog->uiNASName = pstSIGNAL->uiDestIP;
					pstRPPILog->ucAlwaysOnFlag = pstSIGNAL->ucAlwaysOnFlag;
					break;
				case A11_REGIUPDATE_MSG:
					pstRPPILog->usUpdateReqCnt++;

					if (pstSIGNAL->szNetOption[0] == 0x00) {
						memcpy(pstRPPILog->szNetOption, pstSIGNAL->szNetOption, MAX_SVCOPTION_LEN);
						pstRPPILog->usSvcOption = pstSIGNAL->uiSvcOption;
					}

					if (pstSIGNAL->uiPCFIP == 0) {
						pstRPPILog->uiPCFIP = pstSIGNAL->uiPCFIP;
						pstRPPILog->ucBranchID = dGetBranchID(pstRPPILog->uiPCFIP);
					}

					if (pstSIGNAL->szBSMSC[0] != 0x00 )
					{
						if (memcmp(pstSIGNAL->szBSMSC, pstRPPILog->szBSMSC, DEF_BSMSD_LENGTH) != 0) {
							pstRPPILog->usBSMSCChgCnt++;
						}

						memcpy(pstRPPILog->szBSMSC, pstSIGNAL->szBSMSC, DEF_BSMSD_LENGTH);

						pstRPPILog->ucFA_ID = pstSIGNAL->ucFA_ID;
						pstRPPILog->ucSECTOR = pstSIGNAL->ucSECTOR;
						pstRPPILog->ucSYSID = pstSIGNAL->ucSYSID;
						pstRPPILog->ucBSCID = pstSIGNAL->ucBSCID;
						pstRPPILog->ucBTSID = pstSIGNAL->ucBTSID;
					}

					if (pstRPPILog->uiFirstUpdateReqTime == 0)
					{
						pstRPPILog->uiFirstUpdateReqTime = pstSIGNAL->uiSessStartTime;
						pstRPPILog->uiFirstUpdateReqMTime = pstSIGNAL->uiSessStartMTime;
						pstRPPILog->uiFirstUpdateResTime = pstSIGNAL->uiSessEndTime;
						pstRPPILog->uiFirstUpdateResMTime = pstSIGNAL->uiSessEndMTime;
					}
					else
					{
						pstRPPILog->uiLastUpdateReqTime = pstSIGNAL->uiSessStartTime;
						pstRPPILog->uiLastUpdateReqMTime = pstSIGNAL->uiSessStartMTime;
						pstRPPILog->uiLastUpdateResTime = pstSIGNAL->uiSessEndTime;
						pstRPPILog->uiLastUpdateResMTime = pstSIGNAL->uiSessEndMTime;
					}
					dSvcThreshold = dGetThreshold(SERVICE_A11, ALARM_RESPONSETIME);
					if (pstSIGNAL->uiRespCode == A11_UPDATE_ACK)
					{
						if (pstSIGNAL->ucErrorCode > 0) {
							pstRPPILog->uiLastFailReason = (A11_DEFECT + A11_REGI_UPDATE_ACK + pstSIGNAL->ucErrorCode);
							pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(isReCall, pstRPPILog->usCallState, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
							dErrorFlag = 1;
						}
						if (dSvcThreshold > 0 && pstSIGNAL->uiSessDuration > dSvcThreshold*1000) {
							pstRPPILog->uiLastFailReason = (A11_DEFECT + A11_REGI_UPDATE_ACK + RESPONSETIME);
							pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(isReCall, pstRPPILog->usCallState, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
							dErrorFlag = 1;
						}
					}

					/*
					if (pstSIGNAL->ucErrorCode > 0) {
						if (pstRPPILog->usCallType == INIT_CALLSTART) {
							pstRPPILog->uiLastFailReason = (A11_DEFECT + A11_REGI_UPDATE_INIT + pstSIGNAL->ucErrorCode);
							pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(pstRPPILog->usCallState, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
						}
						else {
							pstRPPILog->uiLastFailReason = (A11_DEFECT + A11_REGI_UPDATE_REACT + pstSIGNAL->ucErrorCode);	
							pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(pstRPPILog->usCallState, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
						}
						dErrorFlag = 1;
					}
					##### CHECK BELOW SOURCE
					*/
					if ( dGetA11UpdateCode(pstRPPILog->usCallType, pstSIGNAL->usUpdateReason) > 0) {
						if (pstRPPILog->usCallType == INIT_CALLSTART) {
                            pstRPPILog->uiLastFailReason = (A11_DEFECT + A11_REGI_UPDATE_INIT + pstSIGNAL->usUpdateReason);
                            pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(isReCall, pstRPPILog->usCallState, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
                        }
                        else {
                            pstRPPILog->uiLastFailReason = (A11_DEFECT + A11_REGI_UPDATE_REACT + pstSIGNAL->usUpdateReason);   
                            pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(isReCall, pstRPPILog->usCallState, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
                        }
						dErrorFlag = 1;
                    }
					if (dSvcThreshold > 0 && pstSIGNAL->uiSessDuration > dSvcThreshold*1000) {
						pstRPPILog->uiLastFailReason = (A11_DEFECT + A11_REGI_UPDATE_REACT + RESPONSETIME);
						pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(isReCall, pstRPPILog->usCallState, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
						dErrorFlag = 1;
					}

					if (dErrorFlag == 0)
						pstRPPILog->usUpdateAckCnt++;
					else
					{
						log_print(LOGN_INFO, "SIGNAL ERRCODE[%d] LastFailReason[%u]", pstSIGNAL->ucErrorCode, pstRPPILog->uiLastFailReason);
						pstErrLog = dCreateErrLog(pstRPPILog);
						if (pstErrLog != NULL)
						{
							pstErrLog->uiSessStartTime = pstSIGNAL->uiSessStartTime;
							pstErrLog->uiSessStartMTime = pstSIGNAL->uiSessStartMTime;
							pstErrLog->usProtoType = A11_PROTO;
							pstErrLog->uiErrorCode = pstRPPILog->uiLastFailReason;
						}
					}
					break;
				case A11_SESSUPDATE_MSG:
					pstRPPILog->usUpdateReqCnt++;
					if (pstRPPILog->uiFirstUpdateReqTime == 0)
					{
						pstRPPILog->uiFirstUpdateReqTime = pstSIGNAL->uiSessStartTime;
						pstRPPILog->uiFirstUpdateReqMTime = pstSIGNAL->uiSessStartMTime;
						pstRPPILog->uiFirstUpdateResTime = pstSIGNAL->uiSessEndTime;
						pstRPPILog->uiFirstUpdateResMTime = pstSIGNAL->uiSessEndMTime;
					}
					else
					{
						pstRPPILog->uiLastUpdateReqTime = pstSIGNAL->uiSessStartTime;
						pstRPPILog->uiLastUpdateReqMTime = pstSIGNAL->uiSessStartMTime;
						pstRPPILog->uiLastUpdateResTime = pstSIGNAL->uiSessEndTime;
						pstRPPILog->uiLastUpdateResMTime = pstSIGNAL->uiSessEndMTime;
					}
					dSvcThreshold = dGetThreshold(SERVICE_A11, ALARM_RESPONSETIME);
					if (pstSIGNAL->ucErrorCode > 0) {
						pstRPPILog->uiLastFailReason = (A11_DEFECT + A11_SESS_UPDATE + pstSIGNAL->ucErrorCode);
						dErrorFlag = 1;
					}

					if (dSvcThreshold > 0 && pstSIGNAL->uiSessDuration > dSvcThreshold*1000) {
						pstRPPILog->uiLastFailReason = (A11_DEFECT + A11_SESS_UPDATE + RESPONSETIME);
						dErrorFlag = 1;
					}
					if (dErrorFlag == 0)
						pstRPPILog->usUpdateAckCnt++;
					else
					{	
						log_print(LOGN_INFO, "SIGNAL ERRCODE[%d] LastFailReason[%u]", pstSIGNAL->ucErrorCode, pstRPPILog->uiLastFailReason);
						pstErrLog = dCreateErrLog(pstRPPILog);
						if (pstErrLog != NULL)
						{
							pstErrLog->uiSessStartTime = pstSIGNAL->uiSessStartTime;
							pstErrLog->uiSessStartMTime = pstSIGNAL->uiSessStartMTime;
							pstErrLog->usProtoType = A11_PROTO;
							pstErrLog->uiErrorCode = pstRPPILog->uiLastFailReason;
						}
					} 
					break;
				default:
					log_print(LOGN_WARN, "[%s][%s.%d] NOT DEFINE PROTOTYPE[%d] MSGTYPE[%d] ", __FILE__, __FUNCTION__, __LINE__, 
							pstSIGNAL->uiProtoType, pstSIGNAL->uiMsgType);
					break;

			}

			if (pstRPPILog->usSvcOption > 0 && pstRPPILog->usFMux > 0 && pstRPPILog->usRMux > 0 && 
					(pstRPPILog->usSvcOption != pstSIGNAL->uiSvcOption || pstRPPILog->usFMux != pstSIGNAL->uiFMux
					 || pstRPPILog->usRMux != pstSIGNAL->uiRMux))
				pstRPPILog->usSvcOptChgCnt++;
/*
			if (pstSIGNAL->uiSvcOption != 0)
				pstRPPILog->usSvcOption = pstSIGNAL->uiSvcOption;
*/
			if (pstSIGNAL->uiFMux != 0)
				pstRPPILog->usFMux = pstSIGNAL->uiFMux;
			if (pstSIGNAL->uiRMux != 0)
				pstRPPILog->usRMux = pstSIGNAL->uiRMux;
			if (pstSIGNAL->uiGREKey != 0)
				pstRPPILog->uiGREKey = pstSIGNAL->uiGREKey;

			pstRPPILog->uiRegiRepCode = pstSIGNAL->uiRespCode;
			break;
		case RADIUS_PROTO:
			switch(pstSIGNAL->uiMsgType)
			{
				case RADIUS_ACCESS_MSG:
					log_print (LOGN_INFO, "UPDATE SIGNALLOG IMSI[%s] PROTOTYPE[%d] MSGTYPE[%d] [%s]",
							pstSIGNAL->szIMSI, pstSIGNAL->uiProtoType,
							pstSIGNAL->uiMsgType, PrintMsg(pstSIGNAL));
					dSvcThreshold = dGetThreshold(SERVICE_RADIUS, ALARM_RESPONSETIME);
					if (pstSIGNAL->usServiceType != 0) {
						pstRPPILog->usServiceType = pstSIGNAL->usServiceType;
					}
					pstRPPILog->uiAccessReqTime = pstSIGNAL->uiSessStartTime;
					pstRPPILog->uiAccessReqMTime = pstSIGNAL->uiSessStartMTime;
					pstRPPILog->uiAccessResTime = pstSIGNAL->uiSessEndTime;
					pstRPPILog->uiAccessResMTime = pstSIGNAL->uiSessEndMTime;
					pstRPPILog->usAccessResCode = pstSIGNAL->uiLastUserErrCode;	
					pstRPPILog->usInterimTime = pstSIGNAL->usInterimTime;
					if (pstSIGNAL->uiLastUserErrCode > 0) {
						pstRPPILog->uiLastFailReason = (AAA_DEFECT + AAA_ACCESS_DEFECT + pstSIGNAL->uiLastUserErrCode);
//						pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(pstRPPILog->usCallState, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
						dErrorFlag = 1;
					}
					if (dSvcThreshold > 0 && pstSIGNAL->uiSessDuration > dSvcThreshold*1000) {
						pstRPPILog->uiLastFailReason = (AAA_DEFECT + SERVICE_DELAY_DEFECT + RESPONSETIME);
						dErrorFlag = 1;
					}
					if (dErrorFlag == 1)
					{
						log_print(LOGN_INFO, "SIGNAL ERRCODE[%d] LastFailReason[%u]", pstSIGNAL->uiLastUserErrCode, pstRPPILog->uiLastFailReason);
						pstErrLog = dCreateErrLog(pstRPPILog);
						if (pstErrLog != NULL)
						{
							pstErrLog->uiSessStartTime = pstSIGNAL->uiSessStartTime;
							pstErrLog->uiSessStartMTime = pstSIGNAL->uiSessStartMTime;
							pstErrLog->usProtoType = RADIUS_PROTO;
							pstErrLog->uiErrorCode = pstRPPILog->uiLastFailReason;
						}
					}		
					break;
				case RADIUS_ACCOUNT_MSG:
					if (pstSIGNAL->ucAcctType == ACCOUNTING_INTERIM)
					{
						log_print(LOGN_INFO, "ACCOUNTING INTERIM MSG BYPASS");
						return 0;
					}
					log_print (LOGN_INFO, "UPDATE SIGNALLOG IMSI[%s] PROTOTYPE[%d] MSGTYPE[%d] [%s] ",
							pstSIGNAL->szIMSI, pstSIGNAL->uiProtoType,
							pstSIGNAL->uiMsgType, PrintMsg(pstSIGNAL));
					dSvcThreshold = dGetThreshold(SERVICE_RADIUS, ALARM_RESPONSETIME);
					pstRPPILog->usAccountReqCnt++;
					if (pstSIGNAL->uiLastUserErrCode > 0) {
						pstRPPILog->uiLastFailReason = (AAA_DEFECT + AAA_ACCOUNTING_DEFECT + pstSIGNAL->uiLastUserErrCode);
//						pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(pstRPPILog->usCallState, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
						dErrorFlag = 1;
					}
					if (dSvcThreshold > 0 && pstSIGNAL->uiSessDuration > dSvcThreshold*1000) {
						pstRPPILog->uiLastFailReason = (AAA_DEFECT + SERVICE_DELAY_DEFECT + RESPONSETIME);
						dErrorFlag = 1;
					}   
					if (dErrorFlag == 0) 
					{
						pstRPPILog->usAccountSuccRepCnt++;
					}
					else 
					{
						log_print(LOGN_INFO, "SIGNAL ERRCODE[%d] LastFailReason[%u]", pstSIGNAL->uiLastUserErrCode, pstRPPILog->uiLastFailReason);
						pstErrLog = dCreateErrLog(pstRPPILog);
						if (pstErrLog != NULL)
						{
							pstErrLog->uiSessStartTime = pstSIGNAL->uiSessStartTime;
							pstErrLog->uiSessStartMTime = pstSIGNAL->uiSessStartMTime;
							pstErrLog->usProtoType = RADIUS_PROTO;
							pstErrLog->uiErrorCode = pstRPPILog->uiLastFailReason;
						}
					} 
					pstRPPILog->usAccountCode = pstSIGNAL->uiLastUserErrCode;
					if (pstSIGNAL->ucAcctType == ACCOUNTING_START)
					{
						if (pstRPPILog->uiAccStartTime == 0)
						{
							pstRPPILog->uiAccStartTime = pstSIGNAL->uiSessStartTime;
							pstRPPILog->uiAccStartMTime = pstSIGNAL->uiSessStartMTime;
							pstRPPILog->uiAccStartResTime = pstSIGNAL->uiSessEndTime;
							pstRPPILog->uiAccStartResMTime = pstSIGNAL->uiSessEndMTime;
						}
						else
						{
							pstRPPILog->uiLastAccStartReqTime = pstSIGNAL->uiSessStartTime;
							pstRPPILog->uiLastAccStartReqMTime = pstSIGNAL->uiSessStartMTime;
							pstRPPILog->uiLastAccStartResTime = pstSIGNAL->uiSessEndTime;
							pstRPPILog->uiLastAccStartResMTime = pstSIGNAL->uiSessEndMTime;
						}
					}
					else if (pstSIGNAL->ucAcctType == ACCOUNTING_STOP)
					{
						if (pstRPPILog->uiFirstAccStopReqTime == 0)
						{
							pstRPPILog->uiFirstAccStopReqTime = pstSIGNAL->uiSessStartTime;
							pstRPPILog->uiFirstAccStopReqMTime = pstSIGNAL->uiSessStartMTime;
							pstRPPILog->uiFirstAccStopResTime = pstSIGNAL->uiSessEndTime;
							pstRPPILog->uiFirstAccStopResMTime = pstSIGNAL->uiSessEndMTime;
						}
						else
						{
							pstRPPILog->uiLastAccStopReqTime = pstSIGNAL->uiSessStartTime;
							pstRPPILog->uiLastAccStopReqMTime = pstSIGNAL->uiSessStartMTime;
							pstRPPILog->uiLastAccStopResTime = pstSIGNAL->uiSessEndTime;
							pstRPPILog->uiLastAccStopResMTime = pstSIGNAL->uiSessEndMTime;
						}


						offset = nifo_get_offset_node(pMEMSINFO, (U8*)pstRPPILog);
						uiFirstServFlag = 1;
						if(pstRPPIHash->before.dOffset == offset) {
							uiFirstServFlag = pstRPPIHash->before.uiFirstServFlag;
						}
						else if(pstRPPIHash->after.dOffset == offset) {
							uiFirstServFlag = pstRPPIHash->after.uiFirstServFlag;
						}

						if((!isReCall) && (uiFirstServFlag == 0) && (pstSIGNAL->uiAcctInOctets > 0 || pstSIGNAL->uiAcctOutOctets > 0))
						{
							vCheckFirst(pstRPPIHash, pstRPPILog, (U8*)"ACC-STOP", pstSIGNAL->uiSessStartTime, pstSIGNAL->uiSessStartMTime);
							/* etc 값 세팅 */
							pstRPPILog->uiUpTotDataSize = pstSIGNAL->uiAcctInOctets;
							pstRPPILog->uiDnTotDataSize = pstSIGNAL->uiAcctOutOctets;
							pstRPPILog->uiFirstPlatformType = DEF_PLATFORM_PHONE;
							pstRPPILog->uiFirstSvcL4Type = L4_PHONE_ETC;
							pstRPPILog->uiLastPlatformType = DEF_PLATFORM_PHONE;
							pstRPPILog->uiLastSvcL4Type = L4_PHONE_ETC;
							
							/* etc 망감시 전송 */
							memset(&stWatchMsg, 0x00, DEF_WATCHMSG_SIZE);

							stWatchMsg.usMsgType = WATCH_TYPE_SVC;
							stWatchMsg.ucOffice = pstRPPILog->ucBranchID;
							stWatchMsg.ucSYSID = pstRPPILog->ucSYSID;
							stWatchMsg.ucBSCID = pstRPPILog->ucBSCID;
							stWatchMsg.usBTSID = pstRPPILog->ucBTSID;
							stWatchMsg.ucSec = pstRPPILog->ucSECTOR;
							stWatchMsg.ucFA = pstRPPILog->ucFA_ID;
							stWatchMsg.uiPCFIP = pstRPPILog->uiPCFIP;
							stWatchMsg.ucPCFType = dGetPCFType(pstRPPILog->uiPCFIP);
							stWatchMsg.uiPDSNIP = pstRPPILog->uiNASName;
							stWatchMsg.uiAAAIP = pstRPPILog->uiAAAIP;
							stWatchMsg.uiHSSIP = pstRPPILog->uiHSSIP;
							stWatchMsg.ucSvcIdx = dGetSvcIndex(isReCall, DEF_PLATFORM_PHONE, pstRPPILog->uiLastSvcL7Type);
							stWatchMsg.usSvcL4Type = L4_PHONE_ETC;
							stWatchMsg.uiSVCIP = 0;
							stWatchMsg.uiResult = 0;

							dSendMonInfo(&stWatchMsg);
							log_print(LOGN_INFO, "SEND WATCH MSG[%d][%s] OFFICE[%d][%s] RESULT[%d]",
								stWatchMsg.usMsgType, PrintMonType(stWatchMsg.usMsgType),
								stWatchMsg.ucOffice, PrintOFFICE(stWatchMsg.ucOffice), stWatchMsg.uiResult);
						}
					} 
					break;
				default:
					log_print(LOGN_WARN, "[%s][%s.%d] NOT DEFINE PROTOTYPE[%d] MSGTYPE[%d] ", __FILE__, __FUNCTION__, __LINE__,
							pstSIGNAL->uiProtoType, pstSIGNAL->uiMsgType);
					break;
			}
			pstRPPILog->uiAAAIP = pstSIGNAL->uiDestIP;

			/* SEND WATCH*/
			memset(&stWatchMsg, 0x00, DEF_WATCHMSG_SIZE);

			stWatchMsg.usMsgType = WATCH_TYPE_AAA;
			stWatchMsg.ucOffice = pstRPPILog->ucBranchID;
			stWatchMsg.ucSYSID = pstRPPILog->ucSYSID;
			stWatchMsg.ucBSCID = pstRPPILog->ucBSCID;
			stWatchMsg.usBTSID = pstRPPILog->ucBTSID;
			stWatchMsg.ucSec = pstRPPILog->ucSECTOR;
			stWatchMsg.ucFA = pstRPPILog->ucFA_ID;
			stWatchMsg.uiPCFIP = pstRPPILog->uiPCFIP;
			stWatchMsg.ucPCFType = dGetPCFType(pstRPPILog->uiPCFIP);
			stWatchMsg.uiPDSNIP = pstRPPILog->uiNASName;
			stWatchMsg.uiAAAIP = pstRPPILog->uiAAAIP;
			stWatchMsg.uiHSSIP = pstRPPILog->uiHSSIP;
			stWatchMsg.ucSvcIdx = 0;
			stWatchMsg.usSvcL4Type = 0;
			stWatchMsg.uiSVCIP = 0;
			stWatchMsg.uiResult = (dErrorFlag == 1) ? pstRPPILog->uiLastFailReason : 0;

			dSendMonInfo(&stWatchMsg);
			log_print(LOGN_INFO, "SEND WATCH MSG[%d][%s] OFFICE[%d][%s] RESULT[%d]",
					stWatchMsg.usMsgType, PrintMonType(stWatchMsg.usMsgType),
					stWatchMsg.ucOffice, PrintOFFICE(stWatchMsg.ucOffice), stWatchMsg.uiResult);
			break;
		case DIAMETER_PROTO:
			log_print (LOGN_INFO, "UPDATE SIGNALLOG IMSI[%s] PROTOTYPE[%d] MSGTYPE[%d] [%s]",
					pstSIGNAL->szIMSI, pstSIGNAL->uiProtoType,
					pstSIGNAL->uiMsgType, PrintMsg(pstSIGNAL));

			dSvcThreshold = dGetThreshold(SERVICE_DIAMETER, ALARM_RESPONSETIME);

#ifdef DISABLE_ANALYZE_DIAMETER /* changed by uamyd 20101014 */
			memset(&stWatchMsg, 0x00, DEF_WATCHMSG_SIZE);
			stWatchMsg.usMsgType = WATCH_TYPE_HSS;
			stWatchMsg.uiHSSIP   = pstSIGNAL->uiDestIP;

			/* dErrorFlag 값이 flag외에 lastErrorCode로도 사용되도록 함... for 망감시, added by uamyd 20101014 */
			if (pstSIGNAL->uiLastUserErrCode ==  3){
				stWatchMsg.uiResult = (DIAMETER_DEFECT + DIAMETER_CMD_DEFECT + pstSIGNAL->uiResultCode);
			}else if (pstSIGNAL->uiLastUserErrCode != 0){
				stWatchMsg.uiResult = (DIAMETER_DEFECT + DIAMETER_CMD_DEFECT + pstSIGNAL->uiLastUserErrCode);
			}
			if (dSvcThreshold > 0 && pstSIGNAL->uiSessDuration > dSvcThreshold*1000) {
				stWatchMsg.uiResult = (DIAMETER_DEFECT + SERVICE_DELAY_DEFECT + RESPONSETIME);
			}
#else
#if 0
			pstRPPILog->uiCSCFIP = pstSIGNAL->uiSrcIP;
			pstRPPILog->uiHSSIP = pstSIGNAL->uiDestIP;

			if (pstSIGNAL->uiLastUserErrCode ==  3){
				pstRPPILog->uiLastFailReason = (DIAMETER_DEFECT + DIAMETER_CMD_DEFECT + pstSIGNAL->uiResultCode);
				dErrorFlag = 1;
			}else if (pstSIGNAL->uiLastUserErrCode != 0){
				pstRPPILog->uiLastFailReason = (DIAMETER_DEFECT + DIAMETER_CMD_DEFECT + pstSIGNAL->uiLastUserErrCode);
				dErrorFlag = 1;
			}
			if (dSvcThreshold > 0 && pstSIGNAL->uiSessDuration > dSvcThreshold*1000) {
				pstRPPILog->uiLastFailReason = (DIAMETER_DEFECT + SERVICE_DELAY_DEFECT + RESPONSETIME);
				dErrorFlag = 1;
			}

			switch(pstSIGNAL->uiMsgType)
			{
				case USER_AUTHORIZATION_TRANS:
					pstRPPILog->usUARReqCnt++;
					if (dErrorFlag == 0)
						pstRPPILog->usUARSuccRepCnt++;
/** 아래 모든 케이스에 대해서 반복 되는 부분. 거추장 스러워서 하나만 예문으로 남기고 모두 삭제. by uamyd 20101014
					else{
						log_print(LOGN_INFO, "SIGNAL ERRCODE[%d] LastFailReason[%u]", pstSIGNAL->ucErrorCode, pstRPPILog->uiLastFailReason);
						pstErrLog = dCreateErrLog(pstRPPILog);
						if (pstErrLog != NULL){
							pstErrLog->uiSessStartTime = pstSIGNAL->uiSessStartTime;
							pstErrLog->uiSessStartMTime = pstSIGNAL->uiSessStartMTime;
							pstErrLog->usProtoType = DIAMETER_PROTO;
							pstErrLog->uiErrorCode = pstRPPILog->uiLastFailReason;
						}
					} 
***/
					pstRPPILog->uiUARStartTime  = pstSIGNAL->uiSessStartTime;
					pstRPPILog->uiUARStartMTime = pstSIGNAL->uiSessStartMTime;
					pstRPPILog->uiUAREndTime    = pstSIGNAL->uiSessEndTime;
					pstRPPILog->uiUAREndMTime   = pstSIGNAL->uiSessEndMTime;
					break;
				case SERVER_ASSIGNMENT_TRANS:
					pstRPPILog->usSARReqCnt++;
					if (dErrorFlag == 0)          pstRPPILog->usSARSuccRepCnt++;
					pstRPPILog->uiSARStartTime  = pstSIGNAL->uiSessStartTime;
					pstRPPILog->uiSARStartMTime = pstSIGNAL->uiSessStartMTime;
					pstRPPILog->uiSAREndTime    = pstSIGNAL->uiSessEndTime;
					pstRPPILog->uiSAREndMTime   = pstSIGNAL->uiSessEndMTime;
					break;
				case LOCATION_INFO_TRANS:
					pstRPPILog->usLIRReqCnt++;
					if (dErrorFlag == 0)          pstRPPILog->usLIRSuccRepCnt++;
					pstRPPILog->uiLIRStartTime  = pstSIGNAL->uiSessStartTime;
					pstRPPILog->uiLIRStartMTime = pstSIGNAL->uiSessStartMTime;
					pstRPPILog->uiLIREndTime    = pstSIGNAL->uiSessEndTime;
					pstRPPILog->uiLIREndMTime   = pstSIGNAL->uiSessEndMTime;
					break;
				case MULTIMEDIA_AUTH_TRANS:
					pstRPPILog->usMARReqCnt++;
					if (dErrorFlag == 0)          pstRPPILog->usMARSuccRepCnt++;
					pstRPPILog->uiMARStartTime  = pstSIGNAL->uiSessStartTime;
					pstRPPILog->uiMARStartMTime = pstSIGNAL->uiSessStartMTime;
					pstRPPILog->uiMAREndTime    = pstSIGNAL->uiSessEndTime;
					pstRPPILog->uiMAREndMTime   = pstSIGNAL->uiSessEndMTime;
					break;
				case REGISTRATION_TERMINATION_TRANS:
					pstRPPILog->usRTRReqCnt++;
					if (dErrorFlag == 0)          pstRPPILog->usRTRSuccRepCnt++;
					pstRPPILog->uiRTRStartTime  = pstSIGNAL->uiSessStartTime;
					pstRPPILog->uiRTRStartMTime = pstSIGNAL->uiSessStartMTime;
					pstRPPILog->uiRTREndTime    = pstSIGNAL->uiSessEndTime;
					pstRPPILog->uiRTREndMTime   = pstSIGNAL->uiSessEndMTime;
					break;
				case PUSH_PROFILE_TRANS:
					pstRPPILog->usPPRReqCnt++;
					if (dErrorFlag == 0)          pstRPPILog->usPPRSuccRepCnt++;
					pstRPPILog->uiPPRStartTime  = pstSIGNAL->uiSessStartTime;
					pstRPPILog->uiPPRStartMTime = pstSIGNAL->uiSessStartMTime;
					pstRPPILog->uiPPREndTime    = pstSIGNAL->uiSessEndTime;
					pstRPPILog->uiPPREndMTime   = pstSIGNAL->uiSessEndMTime;
					break;
				case USER_DATA_TRANS:
				case PROFILE_UPDATE_TRANS:
				case SUBSCRIBE_NOTIFICATIONS_TRANS: 
				case PUSH_NOTIFICATION_TRANS: 
				case BOOSTRAPPING_INFO_TRANS: 
				case MESSAGE_PROCES_TRANS: 
				case ACCOUNTING_REQUEST_TRANS: 
				case DEVICE_WATCHDOG_TRANS:
					pstRPPILog->usDiaReqCnt++;
                    if (dErrorFlag == 0) {
                        pstRPPILog->usDiaSuccRepCnt++;
						STG_DiffTIME64(pstSIGNAL->uiSessEndTime, pstSIGNAL->uiSessEndMTime, pstSIGNAL->uiSessStartTime, pstSIGNAL->uiSessStartMTime, &llDuration);
						pstRPPILog->llDiaSuccSumTime += llDuration;		
					}
					break;
				default:
					log_print(LOGN_WARN, "[%s][%s.%d] NOT DEFINE PROTOTYPE[%d] MSGTYPE[%d] ", __FILE__, __FUNCTION__, __LINE__,
							pstSIGNAL->uiProtoType, pstSIGNAL->uiMsgType);
					break;
			}

			/* SEND WATCH*/
			memset(&stWatchMsg, 0x00, DEF_WATCHMSG_SIZE);

			stWatchMsg.usMsgType = WATCH_TYPE_HSS;
			stWatchMsg.ucOffice = pstRPPILog->ucBranchID;
			stWatchMsg.ucSYSID = pstRPPILog->ucSYSID;
			stWatchMsg.ucBSCID = pstRPPILog->ucBSCID;
			stWatchMsg.usBTSID = pstRPPILog->ucBTSID;
			stWatchMsg.ucSec = pstRPPILog->ucSECTOR;
			stWatchMsg.ucFA = pstRPPILog->ucFA_ID;
			stWatchMsg.uiPCFIP = pstRPPILog->uiPCFIP;
			stWatchMsg.ucPCFType = dGetPCFType(pstRPPILog->uiPCFIP);
			stWatchMsg.uiPDSNIP = pstRPPILog->uiNASName;
			stWatchMsg.uiAAAIP = pstRPPILog->uiAAAIP;
			stWatchMsg.uiHSSIP = pstRPPILog->uiHSSIP;
			stWatchMsg.ucSvcIdx = 0;
			stWatchMsg.usSvcL4Type = 0;
			stWatchMsg.uiSVCIP = 0;
			stWatchMsg.uiResult = (dErrorFlag == 1) ? pstRPPILog->uiLastFailReason : 0;
#endif /* if 0 */
#endif /* DISABLE_ANALYZE_DIAMETER */

			dSendMonInfo(&stWatchMsg);
			log_print(LOGN_INFO, "SEND WATCH MSG[%d][%s] OFFICE[%d][%s] RESULT[%d]",
					stWatchMsg.usMsgType, PrintMonType(stWatchMsg.usMsgType),
					stWatchMsg.ucOffice, PrintOFFICE(stWatchMsg.ucOffice), stWatchMsg.uiResult);
			break;      

		case UPLCP_PROTO:
			log_print (LOGN_INFO, "UPDATE SIGNALLOG IMSI[%s] PROTOTYPE[%d] MSGTYPE[%d] [%s]",
					pstSIGNAL->szIMSI, pstSIGNAL->uiProtoType,
					pstSIGNAL->uiMsgType, PrintMsg(pstSIGNAL));
			
			pstRPPILog->uiUpGREPkts = pstSIGNAL->uiUpGREPkts;
			pstRPPILog->uiDnGREPkts = pstSIGNAL->uiDnGREPkts;
			pstRPPILog->uiUpGREBytes = pstSIGNAL->uiUpGREBytes;
			pstRPPILog->uiDnGREBytes = pstSIGNAL->uiDnGREBytes;
			switch (pstSIGNAL->uiMsgType)
			{
				case PPP_SETUP:
//					pstRPPILog->usUpLCPReqCnt++;
					pstRPPILog->usUpLCPReqCnt += pstSIGNAL->ucPPPReqCnt;
					if (pstRPPILog->uiUpLCPStartTime == 0) {
						pstRPPILog->uiUpLCPStartTime = pstSIGNAL->uiSessStartTime;
						pstRPPILog->uiUpLCPStartMTime = pstSIGNAL->uiSessStartMTime;
					}
					pstRPPILog->uiUpLCPEndTime = pstSIGNAL->uiSessEndTime;
					pstRPPILog->uiUpLCPEndMTime = pstSIGNAL->uiSessEndMTime;
					if (pstRPPILog->usCallType == REACT_CALLSTART)
						pstRPPILog->usCallType = REACT_CALLSTART_PPP;
					pstRPPILog->usCallState = dGetCallState(pstRPPILog->usCallState, LCP_SETUP_STATE);
					if((pstRPPILog->uiUpLCPEndTime > 0) && (pstRPPILog->uiDnLCPEndTime > 0)) {
						pstRPPILog->usUpLCPRetrans = 1;
						vGetLCPDuration(pstRPPILog);
					}
					break;
				case PPP_TERM:
					pstRPPILog->usCallState = dGetCallState(pstRPPILog->usCallState, PPP_TERM_STATE);
					break;
				case LCP_ECHO:
					pstRPPILog->usCallState = dGetCallState(pstRPPILog->usCallState, LCP_ECHO_STATE);
					pstRPPILog->usCallType = REACT_CALLSTART_ECHO;
					pstRPPILog->lcpEchoFlag = 1;
					break;
				default:
					log_print(LOGN_WARN, "[%s][%s.%d] NOT DEFINE PROTOTYPE[%d] MSGTYPE[%d] ", __FILE__, __FUNCTION__, __LINE__,
							pstSIGNAL->uiProtoType, pstSIGNAL->uiMsgType);
					break;

			}
			break; 
		case DNLCP_PROTO:
			log_print (LOGN_INFO, "UPDATE SIGNALLOG IMSI[%s] PROTOTYPE[%d] MSGTYPE[%d] [%s]",
					pstSIGNAL->szIMSI, pstSIGNAL->uiProtoType,
					pstSIGNAL->uiMsgType, PrintMsg(pstSIGNAL));

			pstRPPILog->uiUpGREPkts = pstSIGNAL->uiUpGREPkts;
			pstRPPILog->uiDnGREPkts = pstSIGNAL->uiDnGREPkts;
			pstRPPILog->uiUpGREBytes = pstSIGNAL->uiUpGREBytes;
			pstRPPILog->uiDnGREBytes = pstSIGNAL->uiDnGREBytes;
			switch (pstSIGNAL->uiMsgType)
			{
				case PPP_SETUP:
//					pstRPPILog->usDnLCPReqCnt++;
					pstRPPILog->usDnLCPReqCnt += pstSIGNAL->ucPPPReqCnt;
					if (pstRPPILog->uiDnLCPStartTime == 0) {
						pstRPPILog->uiDnLCPStartTime = pstSIGNAL->uiSessStartTime;
						pstRPPILog->uiDnLCPStartMTime = pstSIGNAL->uiSessStartMTime;
					}
					pstRPPILog->uiDnLCPEndTime = pstSIGNAL->uiSessEndTime;
					pstRPPILog->uiDnLCPEndMTime = pstSIGNAL->uiSessEndMTime;
					if (pstRPPILog->usCallType == REACT_CALLSTART)
						pstRPPILog->usCallType = REACT_CALLSTART_PPP;
					pstRPPILog->usCallState = dGetCallState(pstRPPILog->usCallState, LCP_SETUP_STATE);
					if((pstRPPILog->uiUpLCPEndTime > 0) && (pstRPPILog->uiDnLCPEndTime > 0)) {
						pstRPPILog->usDnLCPRetrans = 1;
						vGetLCPDuration(pstRPPILog);
					}
					break;
				case PPP_TERM:
					pstRPPILog->usCallState = dGetCallState(pstRPPILog->usCallState, PPP_TERM_STATE);
					break;
				case LCP_ECHO:
					pstRPPILog->usCallState = dGetCallState(pstRPPILog->usCallState, LCP_ECHO_STATE);
					pstRPPILog->usCallType = REACT_CALLSTART_ECHO;
					pstRPPILog->lcpEchoFlag = 1;
					break;
				default:
					log_print(LOGN_WARN, "[%s][%s.%d] NOT DEFINE PROTOTYPE[%d] MSGTYPE[%d] ", __FILE__, __FUNCTION__, __LINE__,
							pstSIGNAL->uiProtoType, pstSIGNAL->uiMsgType);
					break;

			}
			break;
		case UPIPCP_PROTO:
			log_print (LOGN_INFO, "UPDATE SIGNALLOG IMSI[%s] PROTOTYPE[%d] MSGTYPE[%d] [%s]",
					pstSIGNAL->szIMSI, pstSIGNAL->uiProtoType,
					pstSIGNAL->uiMsgType, PrintMsg(pstSIGNAL));
			
			pstRPPILog->uiUpGREPkts = pstSIGNAL->uiUpGREPkts;
			pstRPPILog->uiDnGREPkts = pstSIGNAL->uiDnGREPkts;
			pstRPPILog->uiUpGREBytes = pstSIGNAL->uiUpGREBytes;
			pstRPPILog->uiDnGREBytes = pstSIGNAL->uiDnGREBytes;
			switch (pstSIGNAL->uiMsgType)
			{
				case PPP_SETUP:
//					pstRPPILog->usUpIPCPReqCnt++;
					pstRPPILog->usUpIPCPReqCnt += pstSIGNAL->ucPPPReqCnt;
					if (pstRPPILog->uiUpIPCPStartTime == 0) {
						pstRPPILog->uiUpIPCPStartTime = pstSIGNAL->uiSessStartTime;
						pstRPPILog->uiUpIPCPStartMTime = pstSIGNAL->uiSessStartMTime;
					}

					if(pstRPPILog->uiUpIPCPEndTime > 0) pstRPPILog->usUpIPCPRetrans = 1;
					pstRPPILog->uiUpIPCPEndTime = pstSIGNAL->uiSessEndTime;
					pstRPPILog->uiUpIPCPEndMTime = pstSIGNAL->uiSessEndMTime;

					if (pstRPPILog->usCallType == REACT_CALLSTART)
						pstRPPILog->usCallType = REACT_CALLSTART_PPP;
					pstRPPILog->usCallState = dGetCallState(pstRPPILog->usCallState, IPCP_SETUP_STATE);

					if((pstRPPILog->uiUpIPCPEndTime > 0) && (pstRPPILog->uiDnIPCPEndTime > 0)) {
//						pstRPPILog->usUpIPCPRetrans = 1;
						vGetIPCPDuration(pstRPPILog);
						pstRPPILog->uiPPPSetupTime = pstSIGNAL->uiSessEndTime;
						pstRPPILog->uiPPPSetupMTime = pstSIGNAL->uiSessEndMTime;
						STG_DiffTIME64(pstSIGNAL->uiSessEndTime, pstSIGNAL->uiSessEndMTime, pstRPPILog->uiCallTime, pstRPPILog->uiCallMTime, &pstRPPILog->llPPPSetupDelTime);	

#if 0
						offset = nifo_get_offset_node(pMEMSINFO, (U8*)pstRPPILog);

						if (pstRPPIHash->before.dOffset == offset) {
							pstRPPIHash->before.uiFirstServFlag = 1;
						}
						else if (pstRPPIHash->after.dOffset == offset) {
							pstRPPIHash->after.uiFirstServFlag = 1;
						}
						else {
							log_print(LOGN_CRI, "[%s][%s.%d] NOT FOUND OFFSET IMSI[%s]  ClientIP[%s]", 
									__FILE__, __FUNCTION__, __LINE__,
									pstSIGNAL->szIMSI, util_cvtipaddr(szIP, pstSIGNAL->uiClientIP));
							return -1;
						}

						/** Call Setup Complete **/
						/** WATCH DATA */
						st_WatchMsg stWatchMsg;
						memset(&stWatchMsg, 0x00, DEF_WATCHMSG_SIZE);

						stWatchMsg.usMsgType = WATCH_TYPE_A11;
						stWatchMsg.ucOffice = pstRPPILog->ucBranchID;
						stWatchMsg.ucSYSID = pstRPPILog->ucSYSID;
						stWatchMsg.ucBSCID = pstRPPILog->ucBSCID;
						stWatchMsg.usBTSID = pstRPPILog->ucBTSID;
						stWatchMsg.ucSec = pstRPPILog->ucSECTOR;
						stWatchMsg.ucFA = pstRPPILog->ucFA_ID;
						stWatchMsg.uiPCFIP = pstRPPILog->uiPCFIP;
						stWatchMsg.ucPCFType = dGetPCFType(pstRPPILog->uiPCFIP);
						stWatchMsg.uiPDSNIP = pstRPPILog->uiNASName;
						stWatchMsg.uiAAAIP = pstRPPILog->uiAAAIP;
						stWatchMsg.uiHSSIP = pstRPPILog->uiHSSIP;
						stWatchMsg.ucSvcIdx = 0;
						stWatchMsg.usSvcL4Type = 0;
						stWatchMsg.uiSVCIP = 0;  
						stWatchMsg.uiResult = 0;

						dSendMonInfo(&stWatchMsg);
						log_print(LOGN_INFO, "SEND WATCH MSG[%d][%s] OFFICE[%d][%s] RESULT[%d]",
								stWatchMsg.usMsgType, PrintMonType(stWatchMsg.usMsgType),
								stWatchMsg.ucOffice, PrintOFFICE(stWatchMsg.ucOffice), stWatchMsg.uiResult);
#endif
					} 
					break;
				case PPP_TERM:
					pstRPPILog->usCallState = dGetCallState(pstRPPILog->usCallState, PPP_TERM_STATE);
					break;
				default:
					log_print(LOGN_WARN, "[%s][%s.%d] NOT DEFINE PROTOTYPE[%d] MSGTYPE[%d] ", __FILE__, __FUNCTION__, __LINE__,
							pstSIGNAL->uiProtoType, pstSIGNAL->uiMsgType);
					break;

			}
			break;
		case DNIPCP_PROTO:
			log_print (LOGN_INFO, "UPDATE SIGNALLOG IMSI[%s] PROTOTYPE[%d] MSGTYPE[%d] [%s]",
					pstSIGNAL->szIMSI, pstSIGNAL->uiProtoType,
					pstSIGNAL->uiMsgType, PrintMsg(pstSIGNAL));

			pstRPPILog->uiUpGREPkts = pstSIGNAL->uiUpGREPkts;
			pstRPPILog->uiDnGREPkts = pstSIGNAL->uiDnGREPkts;
			pstRPPILog->uiUpGREBytes = pstSIGNAL->uiUpGREBytes;
			pstRPPILog->uiDnGREBytes = pstSIGNAL->uiDnGREBytes;
			switch (pstSIGNAL->uiMsgType)
			{
				case PPP_SETUP:
//					pstRPPILog->usDnIPCPReqCnt++;
					pstRPPILog->usDnIPCPReqCnt += pstSIGNAL->ucPPPReqCnt;
					if (pstRPPILog->uiDnIPCPStartTime == 0) {   
						pstRPPILog->uiDnIPCPStartTime = pstSIGNAL->uiSessStartTime;
						pstRPPILog->uiDnIPCPStartMTime = pstSIGNAL->uiSessStartMTime;
					}
					if(pstRPPILog->uiDnIPCPEndTime > 0) pstRPPILog->usDnIPCPRetrans = 1;
					pstRPPILog->uiDnIPCPEndTime = pstSIGNAL->uiSessEndTime;
					pstRPPILog->uiDnIPCPEndMTime = pstSIGNAL->uiSessEndMTime;
					if (pstRPPILog->usCallType == REACT_CALLSTART)
						pstRPPILog->usCallType = REACT_CALLSTART_PPP;
					pstRPPILog->usCallState = dGetCallState(pstRPPILog->usCallState, IPCP_SETUP_STATE);

					if((pstRPPILog->uiUpIPCPEndTime > 0) && (pstRPPILog->uiDnIPCPEndTime > 0)) {
//						pstRPPILog->usDnIPCPRetrans = 1;
						vGetIPCPDuration(pstRPPILog);
						pstRPPILog->uiPPPSetupTime = pstSIGNAL->uiSessEndTime;
						pstRPPILog->uiPPPSetupMTime = pstSIGNAL->uiSessEndMTime;
						STG_DiffTIME64(pstSIGNAL->uiSessEndTime, pstSIGNAL->uiSessEndMTime, pstRPPILog->uiCallTime, pstRPPILog->uiCallMTime, &pstRPPILog->llPPPSetupDelTime);	

#if 0
						offset = nifo_get_offset_node(pMEMSINFO, (U8*)pstRPPILog);

						if (pstRPPIHash->before.dOffset == offset) {
							pstRPPIHash->before.uiFirstServFlag = 1;
						}
						else if (pstRPPIHash->after.dOffset == offset) {
							pstRPPIHash->after.uiFirstServFlag = 1;
						}
						else {
							log_print(LOGN_CRI, "[%s][%s.%d] NOT FOUND OFFSET IMSI[%s]  ClientIP[%s]", 
									__FILE__, __FUNCTION__, __LINE__,
									pstSIGNAL->szIMSI, util_cvtipaddr(szIP, pstSIGNAL->uiClientIP));
							return -1;
						}

						/** Call Setup Complete **/
						/** WATCH DATA */
						st_WatchMsg stWatchMsg;
						memset(&stWatchMsg, 0x00, DEF_WATCHMSG_SIZE);

						stWatchMsg.usMsgType = WATCH_TYPE_A11;
						stWatchMsg.ucOffice = pstRPPILog->ucBranchID;
						stWatchMsg.ucSYSID = pstRPPILog->ucSYSID;
						stWatchMsg.ucBSCID = pstRPPILog->ucBSCID;
						stWatchMsg.usBTSID = pstRPPILog->ucBTSID;
						stWatchMsg.ucSec = pstRPPILog->ucSECTOR;
						stWatchMsg.ucFA = pstRPPILog->ucFA_ID;
						stWatchMsg.uiPCFIP = pstRPPILog->uiPCFIP;
						stWatchMsg.ucPCFType = dGetPCFType(pstRPPILog->uiPCFIP);
						stWatchMsg.uiPDSNIP = pstRPPILog->uiNASName;
						stWatchMsg.uiAAAIP = pstRPPILog->uiAAAIP;
						stWatchMsg.uiHSSIP = pstRPPILog->uiHSSIP;
						stWatchMsg.ucSvcIdx = 0;
						stWatchMsg.usSvcL4Type = 0;
						stWatchMsg.uiSVCIP = 0;  
						stWatchMsg.uiResult = 0;

						dSendMonInfo(&stWatchMsg);
						log_print(LOGN_INFO, "SEND WATCH MSG[%d][%s] OFFICE[%d][%s] RESULT[%d]",
								stWatchMsg.usMsgType, PrintMonType(stWatchMsg.usMsgType),
								stWatchMsg.ucOffice, PrintOFFICE(stWatchMsg.ucOffice), stWatchMsg.uiResult);
#endif
					} 
					break;
				case PPP_TERM:
					pstRPPILog->usCallState = dGetCallState(pstRPPILog->usCallState, PPP_TERM_STATE);
					break;
				default:
					log_print(LOGN_WARN, "[%s][%s.%d] NOT DEFINE PROTOTYPE[%d] MSGTYPE[%d] ", __FILE__, __FUNCTION__, __LINE__,
							pstSIGNAL->uiProtoType, pstSIGNAL->uiMsgType);
					break;

			}
			break;  
		case CHAP_PROTO:
		case PAP_PROTO:
			log_print (LOGN_INFO, "UPDATE SIGNALLOG IMSI[%s] PROTOTYPE[%d] MSGTYPE[%d] [%s]",
					pstSIGNAL->szIMSI, pstSIGNAL->uiProtoType,
					pstSIGNAL->uiMsgType, PrintMsg(pstSIGNAL));
			
			pstRPPILog->uiUpGREPkts = pstSIGNAL->uiUpGREPkts;
			pstRPPILog->uiDnGREPkts = pstSIGNAL->uiDnGREPkts;
			pstRPPILog->uiUpGREBytes = pstSIGNAL->uiUpGREBytes;
			pstRPPILog->uiDnGREBytes = pstSIGNAL->uiDnGREBytes;

			pstRPPILog->usAuthMethod = pstSIGNAL->uiProtoType;
			if(pstRPPILog->uiAuthReqTime == 0) {
				pstRPPILog->uiAuthReqTime = pstSIGNAL->uiSessStartTime;
				pstRPPILog->uiAuthReqMTime = pstSIGNAL->uiSessStartMTime;
			}
			pstRPPILog->uiAuthEndTime = pstSIGNAL->uiSessEndTime;
			pstRPPILog->uiAuthEndMTime = pstSIGNAL->uiSessEndMTime;
			pstRPPILog->uiAuthResTime = pstSIGNAL->uiPPPResponseTime;
			pstRPPILog->uiAuthResMTime = pstSIGNAL->uiPPPResponseMTime;
			pstRPPILog->usAuthReqCnt = pstSIGNAL->ucPPPReqCnt;
			pstRPPILog->usAuthResultCode = pstSIGNAL->ucErrorCode;
			pstRPPILog->uiCHAPRespCode = pstSIGNAL->uiRespCode;
			if (pstRPPILog->usCallType == REACT_CALLSTART)
				pstRPPILog->usCallType = REACT_CALLSTART_PPP;
			pstRPPILog->usCallState = dGetCallState(pstRPPILog->usCallState, AUTH_SETUP_STATE);
			STG_DiffTIME64(pstRPPILog->uiAuthEndTime, pstRPPILog->uiAuthEndMTime, pstRPPILog->uiAuthReqTime, pstRPPILog->uiAuthReqMTime, &pstRPPILog->llAuthDuration);
			break;
		case OTHERPPP_PROTO:
			log_print (LOGN_INFO, "UPDATE SIGNALLOG IMSI[%s] PROTOTYPE[%d] MSGTYPE[%d] [%s]",
					pstSIGNAL->szIMSI, pstSIGNAL->uiProtoType,
					pstSIGNAL->uiMsgType, PrintMsg(pstSIGNAL));
			
			pstRPPILog->uiUpGREPkts = pstSIGNAL->uiUpGREPkts;
			pstRPPILog->uiDnGREPkts = pstSIGNAL->uiDnGREPkts;
			pstRPPILog->uiUpGREBytes = pstSIGNAL->uiUpGREBytes;
			pstRPPILog->uiDnGREBytes = pstSIGNAL->uiDnGREBytes;
			break;
		default:
			log_print(LOGN_WARN, "[%s][%s.%d] NOT DEFINE PROTO TYPE[%d]", __FILE__, __FUNCTION__, __LINE__, pstSIGNAL->uiProtoType);
			break;
	}  
	return 0;
}

void dUpdateCommonLog(LOG_COMMON *pLOG, LOG_RPPI *pstRPPILog)
{

	/* SERVICE COMMON LOG UPDATE */
	pLOG->uiCallTime = pstRPPILog->uiCallTime;
	pLOG->uiCallMTime = pstRPPILog->uiCallMTime;
	pLOG->uiNASName = pstRPPILog->uiNASName;
	memcpy(pLOG->szModel, pstRPPILog->szModel, MAX_MODEL_LEN);
	pLOG->ucFA_ID = pstRPPILog->ucFA_ID;
	pLOG->ucSECTOR = pstRPPILog->ucSECTOR;
	pLOG->ucSYSID = pstRPPILog->ucSYSID;
	pLOG->ucBSCID = pstRPPILog->ucBSCID;
	pLOG->ucBTSID = pstRPPILog->ucBTSID;
	memcpy(pLOG->szBSMSC, pstRPPILog->szBSMSC, DEF_BSMSD_LENGTH - 1);
	memcpy(pLOG->szNetOption, pstRPPILog->szNetOption, MAX_SVCOPTION_LEN); 
	pLOG->uiClientIP = pstRPPILog->uiClientIP;
	pLOG->uiPCFIP = pstRPPILog->uiPCFIP;
	pLOG->ucBranchID = pstRPPILog->ucBranchID;
	///////////////////////

	/* RPPI COMMON LOG UPDATE */
	if (pLOG->szHostName[0] != 0x00) { 
		memcpy(pstRPPILog->szHostName, pLOG->szHostName, MAX_HOSTNAME_LEN);
	}
	else {
		memcpy(pLOG->szHostName, pstRPPILog->szHostName, MAX_HOSTNAME_LEN);
	}

	if (pLOG->szBrowserInfo[0] != 0x00) {
		memcpy(pstRPPILog->szBrowserInfo, pLOG->szBrowserInfo, MAX_BROWSERINFO_LEN);
	}
	else {
		memcpy(pLOG->szBrowserInfo, pstRPPILog->szBrowserInfo, MAX_BROWSERINFO_LEN);
	}

	memcpy(pLOG->szMIN, pstRPPILog->szMIN, MAX_MIN_LEN);
/*
	if (pLOG->szMIN[0] != 0x00) {
		memcpy(pstRPPILog->szMIN, pLOG->szMIN, MAX_MIN_LEN);
	}
	else {
		memcpy(pLOG->szMIN, pstRPPILog->szMIN, MAX_MIN_LEN);
	}
*/
}

LOG_RPPI_ERR *dCreateErrLog(LOG_RPPI *pstRPPILog)
{
	LOG_RPPI_ERR    *pstErrLog;
	U8          	*pNext, *pstLogNode;

	if ( (pNext = nifo_node_alloc(pMEMSINFO)) == NULL)
	{
		log_print (LOGN_CRI, "[%s][%s.%d] nifo_node_alloc NULL ", __FILE__, __FUNCTION__, __LINE__);
		return NULL;
	}

	if ( (pstErrLog = (LOG_RPPI_ERR*) nifo_tlv_alloc(pMEMSINFO, pNext, LOG_RPPI_ERR_DEF_NUM, LOG_RPPI_ERR_SIZE, DEF_MEMSET_ON)) == NULL)
	{
		log_print (LOGN_CRI, "[%s][%s.%d] nifo_tlv_alloc NULL ", __FILE__, __FUNCTION__, __LINE__);
		nifo_node_delete(pMEMSINFO, pNext);
		return NULL;
	}
	memcpy(pstErrLog, pstRPPILog, LOG_COMMON_SIZE);
	pstErrLog->uiAAAIP = pstRPPILog->uiAAAIP;
	pstErrLog->uiCSCFIP = pstRPPILog->uiCSCFIP;
	pstErrLog->uiHSSIP = pstRPPILog->uiHSSIP;

	pstLogNode = nifo_ptr(pMEMSINFO, nifo_get_offset_node(pMEMSINFO, (U8*)pstRPPILog));
	nifo_node_link_nont_prev(pMEMSINFO, pstLogNode, pNext);

	return pstErrLog;	
}

LOG_RPPI *pFindRPPILog(S32 isReCall, HData_RPPI *pstRPPIHash, U32 uiAccStartTime, U32 uiAccStartMTime)
{
	LOG_RPPI		*pstRPPILog;
	S64             llBeforeGapTime, llAfterGapTime;
	U8				szSTime[BUFSIZ];
	U8				szDTime[BUFSIZ];
	U8				szKTime[BUFSIZ];

	pstRPPILog		= NULL;

	if (pstRPPIHash->after.dOffset == 0)
	{
		STG_DeltaTIME64(uiAccStartTime, uiAccStartMTime, pstRPPIHash->before.uiCallTime, pstRPPIHash->before.uiCallMTime, &llBeforeGapTime);
//		if ((llBeforeGapTime < 0 && abs(llBeforeGapTime) <= ALLOW_PITIME) || llBeforeGapTime >= 0) 
		if (llBeforeGapTime >= 0 || (llBeforeGapTime < 0 && labs(llBeforeGapTime) <= ALLOW_PITIME)) 
		{
//			log_print(LOGN_INFO, "Find Before RPPILog");
			log_print(LOGN_INFO, "Find Before RPPILog CallTime[%s.%u] AccStartTime[%s.%u]",
				util_printtime(pstRPPIHash->before.uiCallTime, szSTime), pstRPPIHash->before.uiCallMTime, 
				util_printtime(uiAccStartTime, szDTime), uiAccStartMTime);

			if (pstRPPIHash->before.uiTimerStop == 0 )
				pstRPPIHash->before.timerNID = timerN_update(pTIMERNINFO, pstRPPIHash->before.timerNID, time(NULL) + flt_info->stTimerInfo.usTimerInfo[RPPI_CALL_TIMEOUT]);

			/* 상암 1식에서 A_RPPI Q가 쌓인 후, Clear 발생 직후, 죽는 부분에 대한 예외 처리, added by uamyd, 20110125 */
			if( pMEMSINFO == NULL || !pstRPPIHash->before.dOffset ){
				log_print(LOGN_CRI, "Found Before RPPILog.Offset is ZERO, >>> UNEXPECTED EVENT");
				return NULL;
			}
			//return (LOG_RPPI*)nifo_get_value(pMEMSINFO, LOG_RPPI_DEF_NUM, pstRPPIHash->before.dOffset);
			pstRPPILog = (LOG_RPPI*)nifo_get_value(pMEMSINFO, LOG_RPPI_DEF_NUM, pstRPPIHash->before.dOffset);
		}	
		else
		{
//			log_print(LOGN_CRI, "[pFindRPPILog] NOT FOUND RPPILOG Before CallTime[%s.%u] AccStartTime[%s.%u]",
			log_print(LOGN_WARN, "[pFindRPPILog] NOT FOUND RPPILOG Before CallTime[%s.%u] AccStartTime[%s.%u]",
					util_printtime(pstRPPIHash->before.uiCallTime, szSTime), pstRPPIHash->before.uiCallMTime, 
					util_printtime(uiAccStartTime, szDTime), uiAccStartMTime);
			return NULL;
		}	
	}
	else
	{
		STG_DeltaTIME64(uiAccStartTime, uiAccStartMTime, pstRPPIHash->before.uiCallTime, pstRPPIHash->before.uiCallMTime, &llBeforeGapTime);
		STG_DeltaTIME64(uiAccStartTime, uiAccStartMTime, pstRPPIHash->after.uiCallTime, pstRPPIHash->after.uiCallMTime, &llAfterGapTime);
		if (pstRPPIHash->after.uiCallTime !=0 && llAfterGapTime >= 0)
		{ 
//			log_print(LOGN_INFO, "Find After RPPILog");
			log_print(LOGN_INFO, "Find After RPPILog CallTime[%s.%u] AccStartTime[%s.%u]",
				util_printtime(pstRPPIHash->after.uiCallTime, szSTime), pstRPPIHash->after.uiCallMTime, 
				util_printtime(uiAccStartTime, szDTime), uiAccStartMTime);
			if (pstRPPIHash->after.uiTimerStop == 0 )
				pstRPPIHash->after.timerNID = timerN_update(pTIMERNINFO, pstRPPIHash->after.timerNID, time(NULL) + flt_info->stTimerInfo.usTimerInfo[RPPI_CALL_TIMEOUT]);

			/* 상암 1식에서 A_RPPI Q가 쌓인 후, Clear 발생 직후, 죽는 부분에 대한 예외 처리, 이 부분은 필요가 없을 듯 하지만.., added by uamyd, 20110125 */
			if( pMEMSINFO == NULL || !pstRPPIHash->after.dOffset ){
				log_print(LOGN_CRI, "Found AFTER RPPILog.Offset is ZERO, >>> UNEXPECTED EVENT");
				return NULL;
			}
			//return (LOG_RPPI*)nifo_get_value(pMEMSINFO, LOG_RPPI_DEF_NUM, pstRPPIHash->after.dOffset);  
			pstRPPILog	= (LOG_RPPI*)nifo_get_value(pMEMSINFO, LOG_RPPI_DEF_NUM, pstRPPIHash->after.dOffset);  
		}
		else if (pstRPPIHash->before.uiCallTime !=0 && (llBeforeGapTime >= 0 || (llBeforeGapTime < 0 && labs(llBeforeGapTime) <= ALLOW_PITIME))) 
		{
//			log_print(LOGN_INFO, "Find Before RPPILog");
			log_print(LOGN_INFO, "Find Before RPPILog CallTime[%s.%u] AccStartTime[%s.%u]",
				util_printtime(pstRPPIHash->before.uiCallTime, szSTime), pstRPPIHash->before.uiCallMTime, 
				util_printtime(uiAccStartTime, szDTime), uiAccStartMTime);
			if (pstRPPIHash->before.uiTimerStop == 0 )
				pstRPPIHash->before.timerNID = timerN_update(pTIMERNINFO, pstRPPIHash->before.timerNID, time(NULL) + flt_info->stTimerInfo.usTimerInfo[RPPI_CALL_TIMEOUT]);

			/* 상암 1식에서 A_RPPI Q가 쌓인 후, Clear 발생 직후, 죽는 부분에 대한 예외 처리 2nd, added by uamyd, 20110125 */
			if( pMEMSINFO == NULL || !pstRPPIHash->before.dOffset ){
				log_print(LOGN_CRI, "Found Before RPPILog.Offset is ZERO(2), >>> UNEXPECTED EVENT");
				return NULL;
			}
			//return (LOG_RPPI*)nifo_get_value(pMEMSINFO, LOG_RPPI_DEF_NUM, pstRPPIHash->before.dOffset);	
			pstRPPILog = (LOG_RPPI*)nifo_get_value(pMEMSINFO, LOG_RPPI_DEF_NUM, pstRPPIHash->before.dOffset);	
		}
		else
		{
//			log_print(LOGN_CRI,"[pFindRPPILog] NOT FOUND RPPILOG Before[%s.%u]After[%s.%u]Log[%s.%u]", 
			log_print(LOGN_WARN,"[pFindRPPILog] NOT FOUND RPPILOG Before[%s.%u]After[%s.%u]Log[%s.%u]", 
					util_printtime(pstRPPIHash->before.uiCallTime, szSTime), pstRPPIHash->before.uiCallMTime,
					util_printtime(pstRPPIHash->after.uiCallTime, szDTime), pstRPPIHash->after.uiCallMTime,
					util_printtime(uiAccStartTime, szKTime), uiAccStartMTime);
			return NULL;
		}	
	}

	log_print(LOGN_INFO, "RECALL=%d CALLTYPE=%d", isReCall, pstRPPILog->usCallType);
	if (isReCall) {
		if (pstRPPILog->usCallType == INIT_RECALL_CALLSTART) {
			return pstRPPILog;
		}
		else {
			return NULL;
		}
	}
	else {
		return pstRPPILog;
	}
}

S32 dProcCallStop(HData_RPPI *pstRPPIHash, LOG_RPPI *pstRPPILog, S32 isFirstServ)
{
//	LOG_RPPI_ERR    *pstErrLog;
	U8              szTime[BUFSIZ];
	S32				isFail = 0;
	S32				isReCall = 0;

	isReCall = GET_ISRECALL(pstRPPILog->usCallType);

//	if (pstRPPILog->uiUpIPCPEndTime == 0 && pstRPPILog->uiDnIPCPEndTime == 0 && isFirstServ == 0)
	if(!isFirstServ)
	{
		/** Call Setup Fail **/
		if (pstRPPILog->uiSetupFailReason == SETUP_SUCESS)
		{
			isFail = 1;

			if (isReCall) {

				switch(pstRPPILog->usCallState)
				{
					case RECALL_CALL_INIT_STATE:
						log_print(LOGN_CRI, "NOT START RECALL IMSI=%s TIME=%s.%u RCV isFirstServ=%d CALLSTATE=%d stopFlag=%d",
								pstRPPILog->szIMSI, util_printtime(pstRPPILog->uiCallTime, szTime), pstRPPILog->uiCallMTime,
								isFirstServ, pstRPPILog->usCallState, pstRPPILog->stopFlag);
					case RECALL_PI_DATA_SETUP_STATE:
						pstRPPILog->uiLastFailReason = (RECALL_DEFECT + RECALL_SETUP + ERR_RECALL_1);
						pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(isReCall, pstRPPILog->usCallState, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
						break;
					case RECALL_RP_DATA_SETUP_STATE:
						pstRPPILog->uiLastFailReason = (RECALL_DEFECT + RECALL_SETUP + ERR_RECALL_2);
						pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(isReCall, pstRPPILog->usCallState, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
						break;
					case RECALL_RP_SIG_SETUP_STATE:
						pstRPPILog->uiLastFailReason = (RECALL_DEFECT + RECALL_SETUP + ERR_RECALL_3);
						pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(isReCall, pstRPPILog->usCallState, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
						break;
					case RECALL_PI_SIG_SETUP_STATE:
						pstRPPILog->uiLastFailReason = (RECALL_DEFECT + RECALL_SETUP + ERR_RECALL_4);
						pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(isReCall, pstRPPILog->usCallState, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
						break;
					case RECALL_ON_SERVICE_STATE:
					default:
						isFail = 0;
						log_print(LOGN_CRI, "WRONG RECALL STATE IMSI=%s TIME=%s.%u RCV isFirstServ=%d CALLSTATE=%d", 
								pstRPPILog->szIMSI, util_printtime(pstRPPILog->uiCallTime, szTime), pstRPPILog->uiCallMTime,
								isFirstServ, pstRPPILog->usCallState);
						break;
				}
			}
			
			else {
				switch(pstRPPILog->usCallState)
				{
					case CALL_INIT_STATE:
						log_print(LOGN_CRI, "NOT START LOG STOP IMSI=%s TIME=%s.%u RCV isFirstServ=%d CALLSTATE=%d stopFlag=%d",
								pstRPPILog->szIMSI, util_printtime(pstRPPILog->uiCallTime, szTime), pstRPPILog->uiCallMTime,
								isFirstServ, pstRPPILog->usCallState, pstRPPILog->stopFlag);
					case A11_REGI_STATE:
						pstRPPILog->uiLastFailReason = (A11_DEFECT + CALL_SETUP + ERR_A11_CALL_CUT);
						pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(isReCall, pstRPPILog->usCallState, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
						break;
					case LCP_SETUP_STATE:
						pstRPPILog->uiLastFailReason = (A11_DEFECT + CALL_SETUP + uiGetLCPFail(pstRPPILog));
						pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(isReCall, pstRPPILog->usCallState, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
						break;
					case AUTH_SETUP_STATE:
						pstRPPILog->uiLastFailReason = (A11_DEFECT + CALL_SETUP + uiGetCHAPFail(pstRPPILog));
						pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(isReCall, pstRPPILog->usCallState, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
						break;
					case IPCP_SETUP_STATE:
						pstRPPILog->uiLastFailReason = (A11_DEFECT + CALL_SETUP + uiGetIPCPFail(pstRPPILog));
						pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(isReCall, pstRPPILog->usCallState, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
						break;
					case PPP_TERM_STATE:
						if(pstRPPILog->lcpEchoFlag > 0) {
	#if 0
							if((pstRPPILog->uiDnLCPStartTime > 0) || (pstRPPILog->uiUpLCPStartTime > 0)) {
								isFail = 0;
							}
							else {
								pstRPPILog->uiLastFailReason = (A11_DEFECT + CALL_PPP_TERM + ERR_LCP_918);
								pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(LCP_SETUP_STATE, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
							}
	#endif
							isFail = 0;
						}
						else if((pstRPPILog->uiDnIPCPStartTime > 0) || (pstRPPILog->uiUpIPCPStartTime > 0)) {
							pstRPPILog->uiLastFailReason = (A11_DEFECT + CALL_PPP_TERM + uiGetIPCPFail(pstRPPILog));
							pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(isReCall, IPCP_SETUP_STATE, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
						}
						else if(pstRPPILog->uiAuthReqTime > 0) {
							pstRPPILog->uiLastFailReason = (A11_DEFECT + CALL_PPP_TERM + uiGetCHAPFail(pstRPPILog));
							pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(isReCall, AUTH_SETUP_STATE, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
						}
						else if((pstRPPILog->uiDnLCPStartTime > 0) || (pstRPPILog->uiUpLCPStartTime > 0)) {
							pstRPPILog->uiLastFailReason = (A11_DEFECT + CALL_PPP_TERM + uiGetLCPFail(pstRPPILog));
							pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(isReCall, LCP_SETUP_STATE, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
						}
						else {
							pstRPPILog->uiLastFailReason = (A11_DEFECT + CALL_PPP_TERM + ERR_A11_CALL_CUT);
							pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(isReCall, A11_REGI_STATE, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
						}
						break;
					case LCP_ECHO_STATE:
	#if 0
						if((pstRPPILog->uiDnLCPStartTime > 0) || (pstRPPILog->uiUpLCPStartTime > 0)) {
							isFail = 0;
						}
						else {
							pstRPPILog->uiLastFailReason = (A11_DEFECT + CALL_SETUP + ERR_LCP_918);
							pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(LCP_SETUP_STATE, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
						}
	#endif
							isFail = 0;
						break;
					case ON_SERVICE_STATE:
					default:
						isFail = 0;
						log_print(LOGN_CRI, "WRONG STATE IMSI=%s TIME=%s.%u RCV isFirstServ=%d CALLSTATE=%d", 
								pstRPPILog->szIMSI, util_printtime(pstRPPILog->uiCallTime, szTime), pstRPPILog->uiCallMTime,
								isFirstServ, pstRPPILog->usCallState);
						break;
				}
			}
		}	


/*
		pstErrLog = dCreateErrLog(pstRPPILog);
		if (pstErrLog != NULL)
		{
			pstErrLog->uiSessStartTime = pstRPPILog->uiReleaseTime;
			pstErrLog->uiSessStartMTime = pstRPPILog->uiReleaseMTime;
			pstErrLog->usProtoType = A11_PROTO;
			pstErrLog->uiErrorCode = pstRPPILog->uiLastFailReason;
		}
*/

		/** WATCH DATA */
		st_WatchMsg stWatchMsg;
		memset(&stWatchMsg, 0x00, DEF_WATCHMSG_SIZE);

		if (isReCall) {
			stWatchMsg.usMsgType = WATCH_TYPE_RECALL;
		}
		else {
			stWatchMsg.usMsgType = WATCH_TYPE_A11;
		}
		stWatchMsg.ucOffice = pstRPPILog->ucBranchID;
		stWatchMsg.ucSYSID = pstRPPILog->ucSYSID;
		stWatchMsg.ucBSCID = pstRPPILog->ucBSCID;
		stWatchMsg.usBTSID = pstRPPILog->ucBTSID;
		stWatchMsg.ucSec = pstRPPILog->ucSECTOR;
		stWatchMsg.ucFA = pstRPPILog->ucFA_ID;
		stWatchMsg.uiPCFIP = pstRPPILog->uiPCFIP;
		stWatchMsg.ucPCFType = dGetPCFType(pstRPPILog->uiPCFIP);
		stWatchMsg.uiPDSNIP = pstRPPILog->uiNASName;
		stWatchMsg.uiAAAIP = pstRPPILog->uiAAAIP;
		stWatchMsg.uiHSSIP = pstRPPILog->uiHSSIP;
		stWatchMsg.ucSvcIdx = 0;
		stWatchMsg.usSvcL4Type = 0;
		stWatchMsg.uiResult = pstRPPILog->uiLastFailReason;

		dSendMonInfo(&stWatchMsg);
		log_print(LOGN_INFO, "SEND WATCH MSG[%d][%s] OFFICE[%d][%s] RESULT[%d]",
				stWatchMsg.usMsgType, PrintMonType(stWatchMsg.usMsgType),
				stWatchMsg.ucOffice, PrintOFFICE(stWatchMsg.ucOffice), stWatchMsg.uiResult);    
	}

	switch(pstRPPILog->stopFlag)
	{
		case TIMER_STOP_CALL_NUM:
			if(!isFail) {
				if (isReCall) {
					pstRPPILog->uiLastFailReason = (RECALL_DEFECT + RECALL_SETUP + ERR_CALL_TIMEOUT);
				}
				else {
					pstRPPILog->uiLastFailReason = (A11_DEFECT + CALL_SETUP + ERR_CALL_TIMEOUT);
				}
			}
			break;
		default:
			if(pstRPPILog->stateFlag == RP_END_STATE) {
				pstRPPILog->usCallState = dGetCallState(pstRPPILog->usCallState, RP_END_STATE);
			}
			if(pstRPPILog->stateFlag == RECALL_PI_END_STATE) {
				pstRPPILog->usCallState = dGetCallState(pstRPPILog->usCallState, RECALL_PI_END_STATE);
			}
			break;
	}

	return 0;               
}

S32 dGetA11UpdateCode( U16 usCallStatus, U16 usUpdateReason )
{
	if( usCallStatus == INIT_CALLSTART ) {
		switch( usUpdateReason ) {
			case 194:
			case 202:
			case 241:
			case 242:
			case 243:
			case 244:
				return usUpdateReason;
			default:
				return -1;
		}
	}
	else {
		switch( usUpdateReason ) {
			case 249:
			case 250:
			case 252:
			case 253:
			case 254:
			case 255:
				return usUpdateReason;
			default:
				return -1;
		} 
	}

	return 0; 
}

/** recall proc */
S32 dReCallSignalInfo(S32 type, U8 *data)
{
	RPPISESS_KEY    stRPPIKey, *pstRPPIKey;
	HData_RPPI      stRPPIHash, *pstRPPIHash;
	LOG_SIGNAL      *pstSIGNAL;
	stHASHONODE     *pHASHONODE;
	LOG_RPPI        *pstRPPILog;

	pstRPPIKey = &stRPPIKey; 
	pstRPPIHash = &stRPPIHash;

	U8              szIP[INET_ADDRSTRLEN];
	U8              szPCFIP[INET_ADDRSTRLEN];
	U8              szPDSNIP[INET_ADDRSTRLEN];
	U8              szTime[BUFSIZ];	

	pstSIGNAL = (LOG_SIGNAL*)data;

	memset(pstRPPIKey, 0x00, RPPISESS_KEY_SIZE); 
	memcpy(pstRPPIKey->szIMSI, pstSIGNAL->szIMSI, MAX_MIN_SIZE);
	
	log_print (LOGN_DEBUG, "RCV RECALL[%s:%u:%u] IMSI[%s] IP[%s] ERROR[%d] TIME[%s.%u] PCFIP[%s] PDNSIP[%s]", 
			PrintMsg(pstSIGNAL), pstSIGNAL->uiProtoType, pstSIGNAL->uiMsgType, pstSIGNAL->szIMSI, 
			util_cvtipaddr(szIP, pstSIGNAL->uiClientIP), pstSIGNAL->ucErrorCode, 
			util_printtime(pstSIGNAL->uiCallTime, szTime), pstSIGNAL->uiCallMTime,
			util_cvtipaddr(szPCFIP, pstSIGNAL->uiPCFIP), util_cvtipaddr(szPDSNIP, pstSIGNAL->uiNASName));
//	LOG_SIGNAL_Prt("PRINT LOG_SIGNAL", pstSIGNAL);

	if ( (pHASHONODE = hasho_find(pHASHOINFO, (U8*)pstRPPIKey)) )
	{

		pstRPPIHash = (HData_RPPI*)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);
//		log_print(LOGN_DEBUG, "Before Offset[%ld] After Offset[%ld]", pstRPPIHash->before.dOffset, pstRPPIHash->after.dOffset);	

		pstRPPILog = pFindRPPILog(1, pstRPPIHash, pstSIGNAL->uiCallTime, pstSIGNAL->uiCallMTime);
		if (pstRPPILog == NULL)
		{
//			log_print(LOGN_CRI, "[%s] NOT FOUND RPPI LOG IMSI[%s] IP[%s]",
			log_print(LOGN_WARN, "[%s] NOT FOUND RPPI LOG IMSI[%s] IP[%s] TIME[%s.%u] PDSNIP[%s]",
					PrintMsg(pstSIGNAL), pstSIGNAL->szIMSI, util_cvtipaddr(szIP, pstSIGNAL->uiClientIP),
					util_printtime(pstSIGNAL->uiCallTime, szTime), pstSIGNAL->uiCallMTime, util_cvtipaddr(szPDSNIP, pstSIGNAL->uiNASName));
			return -1;
		}

		dUpdateReCallRPPI(pstRPPILog, pstSIGNAL, pstRPPIHash);
	}

	else
	{
//		log_print(LOGN_CRI, "NOT FOUND [%s:%u:%u] IMSI[%s] IP[%s] ERROR[%d] TIME[%s.%u] PCFIP[%s]", 
		log_print(LOGN_WARN, "RECALL MSG NOT FOUND [%s:%u:%u] IMSI[%s] IP[%s] ERROR[%d] TIME[%s.%u] PCFIP[%s]", 
			PrintMsg(pstSIGNAL), pstSIGNAL->uiProtoType, pstSIGNAL->uiMsgType, pstSIGNAL->szIMSI, 
			util_cvtipaddr(szIP, pstSIGNAL->uiClientIP), pstSIGNAL->ucErrorCode, 
			util_printtime(pstSIGNAL->uiCallTime, szTime), pstSIGNAL->uiCallMTime,
			util_cvtipaddr(szPCFIP, pstSIGNAL->uiPCFIP));

		return -1;
	}
	return 0;
} 


S32 dUpdateReCallRPPI(LOG_RPPI *pstRPPILog, LOG_SIGNAL *pstSIGNAL, HData_RPPI *pstRPPIHash) 
{
	S32				updateFlag = 0;
	S32				isReCall = 1;
	S32 			dSvcThreshold, dErrorFlag;
#ifndef DISABLE_ANALYZE_DIAMETER /* changed by uamyd 20101014 */
	S64				llDuration;
#endif

	LOG_RPPI_ERR 	*pstErrLog;
	st_WatchMsg stWatchMsg;
	OFFSET			offset;
	U32				uiFirstServFlag;
	U8				szPDSNIP[INET_ADDRSTRLEN];

	dErrorFlag = 0;

	if (pstRPPILog->uiClientIP == 0 && pstSIGNAL->uiProtoType != DIAMETER_PROTO)
	{
		pstRPPILog->uiClientIP = pstSIGNAL->uiClientIP;
	}
	
	isReCall = GET_ISRECALL(pstRPPILog->usCallType);
	
	switch(pstSIGNAL->uiProtoType)
	{
		case START_PI_DATA_RECALL_NUM:
			/* IP 변경 */
			pstRPPILog->uiNASName = dGetPDSNIFHash(pstSIGNAL->uiDestIP);
#if 0
			if(pstRPPILog->uiNASName == 0)
			{
//				pstRPPILog->uiNASName = pstSIGNAL->uiNasIP;
				pstRPPILog->uiNASName = 178589516; /* 10.165.15.76 */
			}
#endif
			log_print(LOGN_INFO,"RECALL START [PI_DATA] PROTOTYPE=%u, MSGTYPE=%u PDSNIP=%s", 
					pstSIGNAL->uiProtoType, pstSIGNAL->uiMsgType, util_cvtipaddr(szPDSNIP, pstRPPILog->uiNASName));

			memcpy(pstRPPILog->szNetOption, pstSIGNAL->szNetOption, MAX_SVCOPTION_LEN);
			pstRPPILog->usSvcOption = pstSIGNAL->uiSvcOption;
			memcpy(pstRPPILog->szBSMSC, pstSIGNAL->szBSMSC, DEF_BSMSD_LENGTH);
			memcpy(pstRPPILog->szFirstBSMSC, pstSIGNAL->szBSMSC, DEF_BSMSD_LENGTH);	
			pstRPPILog->uiPCFIP = pstSIGNAL->uiPCFIP;
			pstRPPILog->ucBranchID = dGetBranchID(pstRPPILog->uiPCFIP);
			pstRPPILog->ucFA_ID = pstSIGNAL->ucFA_ID;
			pstRPPILog->ucSECTOR = pstSIGNAL->ucSECTOR;
			pstRPPILog->ucSYSID = pstSIGNAL->ucSYSID;
			pstRPPILog->ucBSCID = pstSIGNAL->ucBSCID;
			pstRPPILog->ucBTSID = pstSIGNAL->ucBTSID;
			
			if(pstRPPILog->usCallType == 0) pstRPPILog->usCallType = INIT_RECALL_CALLSTART;
			pstRPPILog->usCallState = dGetCallState(pstRPPILog->usCallState, RECALL_PI_DATA_SETUP_STATE);
			break;
		case START_RP_DATA_RECALL_NUM:
			log_print (LOGN_INFO, "RECALL SIGNALLOG [RP_DATA] IMSI[%s] PROTOTYPE[%d] MSGTYPE[%u] [%s] PDSNIP[%s]",
					pstSIGNAL->szIMSI, pstSIGNAL->uiProtoType, pstSIGNAL->uiMsgType, PrintMsg(pstSIGNAL), 
					util_cvtipaddr(szPDSNIP, pstSIGNAL->uiNasIP));

			pstRPPILog->uiNASName = pstSIGNAL->uiNASName;
			if(pstRPPILog->usCallType == 0) {
				log_print (LOGN_WARN, "RECALL SIGNAL : ERR CallType IMSI[%s] PROTOTYPE[%d] MSGTYPE[%u] [%s]",
						pstSIGNAL->szIMSI, pstSIGNAL->uiProtoType, 
						pstSIGNAL->uiMsgType, PrintMsg(pstSIGNAL));
				return -1;
			}
			
			pstRPPILog->usCallState = dGetCallState(pstRPPILog->usCallState, RECALL_RP_DATA_SETUP_STATE);
			break;
		case START_PI_SIG_RECALL_NUM:
			log_print (LOGN_INFO, "RECALL SIGNALLOG [PI_SIG] IMSI[%s] PROTOTYPE[%d] MSGTYPE[%u] [%s]",
					pstSIGNAL->szIMSI, pstSIGNAL->uiProtoType, pstSIGNAL->uiMsgType, PrintMsg(pstSIGNAL));
			pstRPPILog->usCallState = dGetCallState(pstRPPILog->usCallState, RECALL_PI_SIG_SETUP_STATE);
			break;
		case START_RP_SIG_RECALL_NUM:
			log_print (LOGN_INFO, "RECALL SIGNALLOG [RP_SIG] IMSI[%s] PROTOTYPE[%d] MSGTYPE[%u] PDSNIP[%s] [%s] ",
					pstSIGNAL->szIMSI, pstSIGNAL->uiProtoType, pstSIGNAL->uiMsgType, util_cvtipaddr(szPDSNIP, pstSIGNAL->uiNASName), 
					PrintMsg(pstSIGNAL));
			pstRPPILog->usCallState = dGetCallState(pstRPPILog->usCallState, RECALL_RP_SIG_SETUP_STATE);
			pstRPPILog->uiNASName = pstSIGNAL->uiNASName;
			break;
		case STOP_RP_RECALL_NUM:
			log_print (LOGN_INFO, "RECALL SIGNALLOG [STOP_RP] IMSI[%s] PROTOTYPE[%d] MSGTYPE[%u] [%s]",
					pstSIGNAL->szIMSI, pstSIGNAL->uiProtoType, pstSIGNAL->uiMsgType, PrintMsg(pstSIGNAL));
//			pstRPPILog->usCallState = dGetCallState(pstRPPILog->usCallState, RECALL_PI_END_STATE);
			pstRPPILog->uiNASName = pstSIGNAL->uiNASName;
			break;
		case STOP_PI_RECALL_NUM:
			log_print (LOGN_INFO, "RECALL SIGNALLOG [STOP_PI] IMSI[%s] PROTOTYPE[%d] MSGTYPE[%u] [%s]",
					pstSIGNAL->szIMSI, pstSIGNAL->uiProtoType, pstSIGNAL->uiMsgType, PrintMsg(pstSIGNAL));
//			pstRPPILog->usCallState = dGetCallState(pstRPPILog->usCallState, RECALL_RP_END_STATE);
			break;
			//
		case A11_PROTO:
			log_print (LOGN_INFO, "UPDATE RECALL SIGNALLOG IMSI[%s] PROTOTYPE[%d] MSGTYPE[%u] [%s]",
					pstSIGNAL->szIMSI, pstSIGNAL->uiProtoType,
					pstSIGNAL->uiMsgType, PrintMsg(pstSIGNAL));
			switch(pstSIGNAL->uiMsgType)
			{
				case A11_REGIREQ_MSG:
					dSvcThreshold = dGetThreshold(SERVICE_A11, ALARM_RESPONSETIME);
					pstRPPILog->usRegiReqCnt++;
					switch(pstSIGNAL->ucAirLink)
					{
						case CONNSETUP_ACTIVE_START:
							pstRPPILog->usActiveStartCnt++;
							pstRPPILog->llRPDuration = pstSIGNAL->uiSessDuration;
							pstRPPILog->usCallState = dGetCallState(pstRPPILog->usCallState, RECALL_RP_SIG_SETUP_STATE);
							updateFlag = 1;
							break;
						case ACTIVE_START:
							pstRPPILog->usActiveStartCnt++;
							pstRPPILog->llRPDuration = pstSIGNAL->uiSessDuration;
							pstRPPILog->usCallState = dGetCallState(pstRPPILog->usCallState, RECALL_RP_SIG_SETUP_STATE);
							updateFlag = 1;
							break;
						case ACTIVE_STOP:
							pstRPPILog->usActiveStopCnt++;
							pstRPPILog->usCallState = dGetCallState(pstRPPILog->usCallState, RECALL_RP_END_STATE);
							updateFlag = 0;
							break;
						case ACTIVE_START_STOP:
							pstRPPILog->usActiveStartCnt++;
							pstRPPILog->llRPDuration = pstSIGNAL->uiSessDuration;
							pstRPPILog->usCallState = dGetCallState(pstRPPILog->usCallState, RECALL_RP_SIG_SETUP_STATE);
							pstRPPILog->usActiveStopCnt++;
							updateFlag = 1;
							break;
						default:
							pstRPPILog->llRPDuration = pstSIGNAL->uiSessDuration;
							log_print (LOGN_WARN, "UNKNOWN AIRLINK[%u] IMSI[%s] PROTOTYPE[%d] MSGTYPE[%d] [%s]",
									pstSIGNAL->ucAirLink, pstSIGNAL->szIMSI, pstSIGNAL->uiProtoType, 
									pstSIGNAL->uiMsgType, PrintMsg(pstSIGNAL));
							updateFlag = 0;
							break;
					}

					if(updateFlag > 0)
					{
						memcpy(pstRPPILog->szNetOption, pstSIGNAL->szNetOption, MAX_SVCOPTION_LEN);
						pstRPPILog->usSvcOption = pstSIGNAL->uiSvcOption;

						if(memcmp(pstSIGNAL->szBSMSC, pstRPPILog->szBSMSC, DEF_BSMSD_LENGTH) != 0) {
							pstRPPILog->usBSMSCChgCnt++;
						}

						memcpy(pstRPPILog->szBSMSC, pstSIGNAL->szBSMSC, DEF_BSMSD_LENGTH);
						pstRPPILog->uiPCFIP = pstSIGNAL->uiPCFIP;
						pstRPPILog->ucBranchID = dGetBranchID(pstRPPILog->uiPCFIP);
						pstRPPILog->ucFA_ID = pstSIGNAL->ucFA_ID;
						pstRPPILog->ucSECTOR = pstSIGNAL->ucSECTOR;
						pstRPPILog->ucSYSID = pstSIGNAL->ucSYSID;
						pstRPPILog->ucBSCID = pstSIGNAL->ucBSCID;
						pstRPPILog->ucBTSID = pstSIGNAL->ucBTSID;
					}
					
					if (pstSIGNAL->ucErrorCode > 0 )
					{
						pstRPPILog->uiLastFailReason = (A11_DEFECT + A11_REGI_REPLY + pstSIGNAL->ucErrorCode);
						pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(GET_ISRECALL(pstRPPILog->usCallType), pstRPPILog->usCallState, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
						dErrorFlag = 1;
					}
					if (dSvcThreshold > 0 && pstSIGNAL->uiSessDuration > dSvcThreshold*1000) {
						//pstRPPILog->uiLastFailReason = (A11_DEFECT + A11_REGI_REPLY + RESPONSETIME);
						pstRPPILog->uiLastFailReason = (RECALL_DEFECT + RECALL_SETUP + ERR_RECALL_2);
						dErrorFlag = 1;
					}

					if (dErrorFlag == 0)
						pstRPPILog->usRegiSuccRepCnt++;
					else
					{
						log_print(LOGN_INFO, "RECALL SIGNAL ERRCODE[%d] LastFailReason[%u]", pstSIGNAL->ucErrorCode, pstRPPILog->uiLastFailReason);
						pstErrLog = dCreateErrLog(pstRPPILog);
						if (pstErrLog != NULL)
						{
							pstErrLog->uiSessStartTime = pstSIGNAL->uiSessStartTime;
							pstErrLog->uiSessStartMTime = pstSIGNAL->uiSessStartMTime;
							pstErrLog->usProtoType = A11_PROTO; 
							pstErrLog->uiErrorCode = pstRPPILog->uiLastFailReason;
						}
					}

					pstRPPILog->uiPCFIP  = pstSIGNAL->uiSrcIP;
					pstRPPILog->ucBranchID = dGetBranchID(pstRPPILog->uiPCFIP);
					pstRPPILog->uiNASName = pstSIGNAL->uiDestIP;
					pstRPPILog->ucAlwaysOnFlag = pstSIGNAL->ucAlwaysOnFlag;
					break;

				case A11_REGIUPDATE_MSG:
					pstRPPILog->usUpdateReqCnt++;

					if (pstSIGNAL->szNetOption[0] == 0x00) {
						memcpy(pstRPPILog->szNetOption, pstSIGNAL->szNetOption, MAX_SVCOPTION_LEN);
						pstRPPILog->usSvcOption = pstSIGNAL->uiSvcOption;
					}

					if (pstSIGNAL->uiPCFIP == 0) {
						pstRPPILog->uiPCFIP = pstSIGNAL->uiPCFIP;
						pstRPPILog->ucBranchID = dGetBranchID(pstRPPILog->uiPCFIP);
					}

					if (pstSIGNAL->szBSMSC[0] != 0x00 )
					{
						if (memcmp(pstSIGNAL->szBSMSC, pstRPPILog->szBSMSC, DEF_BSMSD_LENGTH) != 0) {
							pstRPPILog->usBSMSCChgCnt++;
						}

						memcpy(pstRPPILog->szBSMSC, pstSIGNAL->szBSMSC, DEF_BSMSD_LENGTH);

						pstRPPILog->ucFA_ID = pstSIGNAL->ucFA_ID;
						pstRPPILog->ucSECTOR = pstSIGNAL->ucSECTOR;
						pstRPPILog->ucSYSID = pstSIGNAL->ucSYSID;
						pstRPPILog->ucBSCID = pstSIGNAL->ucBSCID;
						pstRPPILog->ucBTSID = pstSIGNAL->ucBTSID;
					}

					if (pstRPPILog->uiFirstUpdateReqTime == 0)
					{
						pstRPPILog->uiFirstUpdateReqTime = pstSIGNAL->uiSessStartTime;
						pstRPPILog->uiFirstUpdateReqMTime = pstSIGNAL->uiSessStartMTime;
						pstRPPILog->uiFirstUpdateResTime = pstSIGNAL->uiSessEndTime;
						pstRPPILog->uiFirstUpdateResMTime = pstSIGNAL->uiSessEndMTime;
					}
					else
					{
						pstRPPILog->uiLastUpdateReqTime = pstSIGNAL->uiSessStartTime;
						pstRPPILog->uiLastUpdateReqMTime = pstSIGNAL->uiSessStartMTime;
						pstRPPILog->uiLastUpdateResTime = pstSIGNAL->uiSessEndTime;
						pstRPPILog->uiLastUpdateResMTime = pstSIGNAL->uiSessEndMTime;
					}
					dSvcThreshold = dGetThreshold(SERVICE_A11, ALARM_RESPONSETIME);
					if (pstSIGNAL->uiRespCode == A11_UPDATE_ACK)
					{
						if (pstSIGNAL->ucErrorCode > 0) {
							pstRPPILog->uiLastFailReason = (A11_DEFECT + A11_REGI_UPDATE_ACK + pstSIGNAL->ucErrorCode);
							pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(isReCall, pstRPPILog->usCallState, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
							dErrorFlag = 1;
						}
						if (dSvcThreshold > 0 && pstSIGNAL->uiSessDuration > dSvcThreshold*1000) {
							pstRPPILog->uiLastFailReason = (RECALL_DEFECT + RECALL_SETUP + ERR_RECALL_2);
						//	pstRPPILog->uiLastFailReason = (A11_DEFECT + A11_REGI_UPDATE_ACK + RESPONSETIME);
							pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(isReCall, pstRPPILog->usCallState, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
							dErrorFlag = 1;
						}
					}

					/*
					if (pstSIGNAL->ucErrorCode > 0) {
						if (pstRPPILog->usCallType == INIT_CALLSTART) {
							pstRPPILog->uiLastFailReason = (A11_DEFECT + A11_REGI_UPDATE_INIT + pstSIGNAL->ucErrorCode);
							pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(pstRPPILog->usCallState, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
						}
						else {
							pstRPPILog->uiLastFailReason = (A11_DEFECT + A11_REGI_UPDATE_REACT + pstSIGNAL->ucErrorCode);	
							pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(pstRPPILog->usCallState, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
						}
						dErrorFlag = 1;
					}
					##### CHECK BELOW SOURCE
					*/
					if ( dGetA11UpdateCode(pstRPPILog->usCallType, pstSIGNAL->usUpdateReason) > 0) {
						if (pstRPPILog->usCallType == INIT_RECALL_CALLSTART) {
                           	pstRPPILog->uiLastFailReason = (A11_DEFECT + A11_REGI_UPDATE_INIT + pstSIGNAL->usUpdateReason);
                           	pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(isReCall, pstRPPILog->usCallState, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
                       	}
                       	else {
                           	pstRPPILog->uiLastFailReason = (A11_DEFECT + A11_REGI_UPDATE_REACT + pstSIGNAL->usUpdateReason);   
                           	pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(isReCall, pstRPPILog->usCallState, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
                       	}
						dErrorFlag = 1;
                   	}

					if (dSvcThreshold > 0 && pstSIGNAL->uiSessDuration > dSvcThreshold*1000) {
						//pstRPPILog->uiLastFailReason = (A11_DEFECT + A11_REGI_UPDATE_INIT + RESPONSETIME);
						pstRPPILog->uiLastFailReason = (RECALL_DEFECT + RECALL_SETUP + ERR_RECALL_2);
						pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(isReCall, pstRPPILog->usCallState, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
						dErrorFlag = 1;
					}
					if (dErrorFlag == 0)
						pstRPPILog->usUpdateAckCnt++;
					else
					{
						log_print(LOGN_INFO, "RECALL SIGNAL ERRCODE[%d] LastFailReason[%u]", pstSIGNAL->ucErrorCode, pstRPPILog->uiLastFailReason);
						pstErrLog = dCreateErrLog(pstRPPILog);
						if (pstErrLog != NULL)
						{
							pstErrLog->uiSessStartTime = pstSIGNAL->uiSessStartTime;
							pstErrLog->uiSessStartMTime = pstSIGNAL->uiSessStartMTime;
							pstErrLog->usProtoType = A11_PROTO;
							pstErrLog->uiErrorCode = pstRPPILog->uiLastFailReason;
						}
					}
					break;
				case A11_SESSUPDATE_MSG:
					pstRPPILog->usUpdateReqCnt++;
					if (pstRPPILog->uiFirstUpdateReqTime == 0)
					{
						pstRPPILog->uiFirstUpdateReqTime = pstSIGNAL->uiSessStartTime;
						pstRPPILog->uiFirstUpdateReqMTime = pstSIGNAL->uiSessStartMTime;
						pstRPPILog->uiFirstUpdateResTime = pstSIGNAL->uiSessEndTime;
						pstRPPILog->uiFirstUpdateResMTime = pstSIGNAL->uiSessEndMTime;
					}
					else
					{
						pstRPPILog->uiLastUpdateReqTime = pstSIGNAL->uiSessStartTime;
						pstRPPILog->uiLastUpdateReqMTime = pstSIGNAL->uiSessStartMTime;
						pstRPPILog->uiLastUpdateResTime = pstSIGNAL->uiSessEndTime;
						pstRPPILog->uiLastUpdateResMTime = pstSIGNAL->uiSessEndMTime;
					}
					dSvcThreshold = dGetThreshold(SERVICE_A11, ALARM_RESPONSETIME);
					if (pstSIGNAL->ucErrorCode > 0) {
						pstRPPILog->uiLastFailReason = (A11_DEFECT + A11_SESS_UPDATE + pstSIGNAL->ucErrorCode);
						dErrorFlag = 1;
					}

					if (dSvcThreshold > 0 && pstSIGNAL->uiSessDuration > dSvcThreshold*1000) {
					//	pstRPPILog->uiLastFailReason = (A11_DEFECT + A11_SESS_UPDATE + RESPONSETIME);
						pstRPPILog->uiLastFailReason = (RECALL_DEFECT + RECALL_SETUP + ERR_RECALL_2);
						dErrorFlag = 1;
					}
					if (dErrorFlag == 0)
						pstRPPILog->usUpdateAckCnt++;
					else
					{	
						log_print(LOGN_INFO, "SIGNAL ERRCODE[%d] LastFailReason[%u]", pstSIGNAL->ucErrorCode, pstRPPILog->uiLastFailReason);
						pstErrLog = dCreateErrLog(pstRPPILog);
						if (pstErrLog != NULL)
						{
							pstErrLog->uiSessStartTime = pstSIGNAL->uiSessStartTime;
							pstErrLog->uiSessStartMTime = pstSIGNAL->uiSessStartMTime;
							pstErrLog->usProtoType = A11_PROTO;
							pstErrLog->uiErrorCode = pstRPPILog->uiLastFailReason;
						}
					} 
					break;
				default:
					log_print(LOGN_WARN, "[%s][%s.%d] RECALL NOT DEFINE PROTOTYPE[%d] MSGTYPE[%d] ", __FILE__, __FUNCTION__, __LINE__, 
							pstSIGNAL->uiProtoType, pstSIGNAL->uiMsgType);
					break;

			}

			if (pstRPPILog->usSvcOption > 0 && pstRPPILog->usFMux > 0 && pstRPPILog->usRMux > 0 && 
					(pstRPPILog->usSvcOption != pstSIGNAL->uiSvcOption || pstRPPILog->usFMux != pstSIGNAL->uiFMux
				 	|| pstRPPILog->usRMux != pstSIGNAL->uiRMux))
				pstRPPILog->usSvcOptChgCnt++;
/*
			if (pstSIGNAL->uiSvcOption != 0)
				pstRPPILog->usSvcOption = pstSIGNAL->uiSvcOption;
*/
			if (pstSIGNAL->uiFMux != 0)
				pstRPPILog->usFMux = pstSIGNAL->uiFMux;
			if (pstSIGNAL->uiRMux != 0)
				pstRPPILog->usRMux = pstSIGNAL->uiRMux;
			if (pstSIGNAL->uiGREKey != 0)
				pstRPPILog->uiGREKey = pstSIGNAL->uiGREKey;

			pstRPPILog->uiRegiRepCode = pstSIGNAL->uiRespCode;
			break;
		case RADIUS_PROTO:
			switch(pstSIGNAL->uiMsgType)
			{
				case RADIUS_ACCESS_MSG:
					log_print (LOGN_INFO, "RECALL UPDATE SIGNALLOG IMSI[%s] PROTOTYPE[%d] MSGTYPE[%d] [%s]",
							pstSIGNAL->szIMSI, pstSIGNAL->uiProtoType,
							pstSIGNAL->uiMsgType, PrintMsg(pstSIGNAL));
					dSvcThreshold = dGetThreshold(SERVICE_RADIUS, ALARM_RESPONSETIME);
				
					if (pstSIGNAL->usServiceType != 0) {
						pstRPPILog->usServiceType = pstSIGNAL->usServiceType;
					}
				
					pstRPPILog->uiAccessReqTime = pstSIGNAL->uiSessStartTime;
					pstRPPILog->uiAccessReqMTime = pstSIGNAL->uiSessStartMTime;
					pstRPPILog->uiAccessResTime = pstSIGNAL->uiSessEndTime;
					pstRPPILog->uiAccessResMTime = pstSIGNAL->uiSessEndMTime;
					pstRPPILog->usAccessResCode = pstSIGNAL->uiLastUserErrCode;	
					pstRPPILog->usInterimTime = pstSIGNAL->usInterimTime;
					if (pstSIGNAL->uiLastUserErrCode > 0) {
						pstRPPILog->uiLastFailReason = (AAA_DEFECT + AAA_ACCESS_DEFECT + pstSIGNAL->uiLastUserErrCode);
						dErrorFlag = 1;
					}
					if (dSvcThreshold > 0 && pstSIGNAL->uiSessDuration > dSvcThreshold*1000) {
					//	pstRPPILog->uiLastFailReason = (AAA_DEFECT + SERVICE_DELAY_DEFECT + RESPONSETIME);
						pstRPPILog->uiLastFailReason = (RECALL_DEFECT + RECALL_SETUP + ERR_RECALL_3);
						dErrorFlag = 1;
					}
					if (dErrorFlag == 1)
					{
						log_print(LOGN_INFO, "RECALL SIGNAL ERRCODE[%d] LastFailReason[%u]", pstSIGNAL->uiLastUserErrCode, pstRPPILog->uiLastFailReason);
						pstErrLog = dCreateErrLog(pstRPPILog);
						if (pstErrLog != NULL)
						{
							pstErrLog->uiSessStartTime = pstSIGNAL->uiSessStartTime;
							pstErrLog->uiSessStartMTime = pstSIGNAL->uiSessStartMTime;
							pstErrLog->usProtoType = RADIUS_PROTO;
							pstErrLog->uiErrorCode = pstRPPILog->uiLastFailReason;
						}
					}		
					break;
				case RADIUS_ACCOUNT_MSG:
					if (pstSIGNAL->ucAcctType == ACCOUNTING_INTERIM)
					{
						log_print(LOGN_INFO, "RECALL ACCOUNTING INTERIM MSG BYPASS");
						return 0;
					}
					log_print (LOGN_INFO, "RECALL UPDATE SIGNALLOG IMSI[%s] PROTOTYPE[%d] MSGTYPE[%d] [%s] ",
							pstSIGNAL->szIMSI, pstSIGNAL->uiProtoType,
							pstSIGNAL->uiMsgType, PrintMsg(pstSIGNAL));
				
					dSvcThreshold = dGetThreshold(SERVICE_RADIUS, ALARM_RESPONSETIME);
					pstRPPILog->usAccountReqCnt++;
					if (pstSIGNAL->uiLastUserErrCode > 0) {
						pstRPPILog->uiLastFailReason = (AAA_DEFECT + AAA_ACCOUNTING_DEFECT + pstSIGNAL->uiLastUserErrCode);
						dErrorFlag = 1;
					}
					if (dSvcThreshold > 0 && pstSIGNAL->uiSessDuration > dSvcThreshold*1000) {
					//	pstRPPILog->uiLastFailReason = (AAA_DEFECT + SERVICE_DELAY_DEFECT + RESPONSETIME);
						pstRPPILog->uiLastFailReason = (RECALL_DEFECT + RECALL_SETUP + ERR_RECALL_3);
						dErrorFlag = 1;
					}   

					if (dErrorFlag == 0) 
					{
						pstRPPILog->usAccountSuccRepCnt++;
					}
					else 
					{
						log_print(LOGN_INFO, "RECALL SIGNAL ERRCODE[%d] LastFailReason[%u]", pstSIGNAL->uiLastUserErrCode, pstRPPILog->uiLastFailReason);
						pstErrLog = dCreateErrLog(pstRPPILog);
						if (pstErrLog != NULL)
						{
							pstErrLog->uiSessStartTime = pstSIGNAL->uiSessStartTime;
							pstErrLog->uiSessStartMTime = pstSIGNAL->uiSessStartMTime;
							pstErrLog->usProtoType = RADIUS_PROTO;
							pstErrLog->uiErrorCode = pstRPPILog->uiLastFailReason;
						}
					} 
					pstRPPILog->usAccountCode = pstSIGNAL->uiLastUserErrCode;
					if (pstSIGNAL->ucAcctType == ACCOUNTING_START)
					{
						if (pstRPPILog->uiAccStartTime == 0)
						{
							pstRPPILog->uiAccStartTime = pstSIGNAL->uiSessStartTime;
							pstRPPILog->uiAccStartMTime = pstSIGNAL->uiSessStartMTime;
							pstRPPILog->uiAccStartResTime = pstSIGNAL->uiSessEndTime;
							pstRPPILog->uiAccStartResMTime = pstSIGNAL->uiSessEndMTime;
						}
						else
						{
							pstRPPILog->uiLastAccStartReqTime = pstSIGNAL->uiSessStartTime;
							pstRPPILog->uiLastAccStartReqMTime = pstSIGNAL->uiSessStartMTime;
							pstRPPILog->uiLastAccStartResTime = pstSIGNAL->uiSessEndTime;
							pstRPPILog->uiLastAccStartResMTime = pstSIGNAL->uiSessEndMTime;
						}
						pstRPPILog->usCallState = dGetCallState(pstRPPILog->usCallState, RECALL_PI_SIG_SETUP_STATE);
					}
					else if (pstSIGNAL->ucAcctType == ACCOUNTING_STOP)
					{
						if (pstRPPILog->uiFirstAccStopReqTime == 0)
						{
							pstRPPILog->uiFirstAccStopReqTime = pstSIGNAL->uiSessStartTime;
							pstRPPILog->uiFirstAccStopReqMTime = pstSIGNAL->uiSessStartMTime;
							pstRPPILog->uiFirstAccStopResTime = pstSIGNAL->uiSessEndTime;
							pstRPPILog->uiFirstAccStopResMTime = pstSIGNAL->uiSessEndMTime;
						}
						else
						{
							pstRPPILog->uiLastAccStopReqTime = pstSIGNAL->uiSessStartTime;
							pstRPPILog->uiLastAccStopReqMTime = pstSIGNAL->uiSessStartMTime;
							pstRPPILog->uiLastAccStopResTime = pstSIGNAL->uiSessEndTime;
							pstRPPILog->uiLastAccStopResMTime = pstSIGNAL->uiSessEndMTime;
						}


						pstRPPILog->usCallState = dGetCallState(pstRPPILog->usCallState, RECALL_PI_END_STATE);

						offset = nifo_get_offset_node(pMEMSINFO, (U8*)pstRPPILog);
						uiFirstServFlag = 1;
						if(pstRPPIHash->before.dOffset == offset) {
							uiFirstServFlag = pstRPPIHash->before.uiFirstServFlag;
						}
						else if(pstRPPIHash->after.dOffset == offset) {
							uiFirstServFlag = pstRPPIHash->after.uiFirstServFlag;
						}

						if((!isReCall) && (uiFirstServFlag == 0) && (pstSIGNAL->uiAcctInOctets > 0 || pstSIGNAL->uiAcctOutOctets > 0))
						{
							vCheckFirst(pstRPPIHash, pstRPPILog, (U8*)"ACC-STOP", pstSIGNAL->uiSessStartTime, pstSIGNAL->uiSessStartMTime);
							/* etc 값 세팅 */
							pstRPPILog->uiUpTotDataSize = pstSIGNAL->uiAcctInOctets;
							pstRPPILog->uiDnTotDataSize = pstSIGNAL->uiAcctOutOctets;
							pstRPPILog->uiFirstPlatformType = DEF_PLATFORM_PHONE;
							pstRPPILog->uiFirstSvcL4Type = L4_PHONE_ETC;
							pstRPPILog->uiLastPlatformType = DEF_PLATFORM_PHONE;
							pstRPPILog->uiLastSvcL4Type = L4_PHONE_ETC;
						
							/* etc 망감시 전송 */
							memset(&stWatchMsg, 0x00, DEF_WATCHMSG_SIZE);

							stWatchMsg.usMsgType = WATCH_TYPE_SVC;
							stWatchMsg.ucOffice = pstRPPILog->ucBranchID;
							stWatchMsg.ucSYSID = pstRPPILog->ucSYSID;
							stWatchMsg.ucBSCID = pstRPPILog->ucBSCID;
							stWatchMsg.usBTSID = pstRPPILog->ucBTSID;
							stWatchMsg.ucSec = pstRPPILog->ucSECTOR;
							stWatchMsg.ucFA = pstRPPILog->ucFA_ID;
							stWatchMsg.uiPCFIP = pstRPPILog->uiPCFIP;
							stWatchMsg.ucPCFType = dGetPCFType(pstRPPILog->uiPCFIP);
							stWatchMsg.uiPDSNIP = pstRPPILog->uiNASName;
							stWatchMsg.uiAAAIP = pstRPPILog->uiAAAIP;
							stWatchMsg.uiHSSIP = pstRPPILog->uiHSSIP;
							stWatchMsg.ucSvcIdx = dGetSvcIndex(isReCall, DEF_PLATFORM_PHONE, pstRPPILog->uiLastSvcL7Type);
							stWatchMsg.usSvcL4Type = L4_PHONE_ETC;
							stWatchMsg.uiSVCIP = 0;
							stWatchMsg.uiResult = 0;

							dSendMonInfo(&stWatchMsg);
							log_print(LOGN_INFO, "SEND WATCH MSG[%d][%s] OFFICE[%d][%s] RESULT[%d]",
								stWatchMsg.usMsgType, PrintMonType(stWatchMsg.usMsgType),
								stWatchMsg.ucOffice, PrintOFFICE(stWatchMsg.ucOffice), stWatchMsg.uiResult);
						}
					} 
					break;
				default:
					log_print(LOGN_WARN, "[%s][%s.%d] NOT DEFINE PROTOTYPE[%d] MSGTYPE[%d] ", __FILE__, __FUNCTION__, __LINE__,
							pstSIGNAL->uiProtoType, pstSIGNAL->uiMsgType);
					break;
			}
			pstRPPILog->uiAAAIP = pstSIGNAL->uiDestIP;

			/* SEND WATCH*/
			memset(&stWatchMsg, 0x00, DEF_WATCHMSG_SIZE);

			stWatchMsg.usMsgType = WATCH_TYPE_AAA;
			stWatchMsg.ucOffice = pstRPPILog->ucBranchID;
			stWatchMsg.ucSYSID = pstRPPILog->ucSYSID;
			stWatchMsg.ucBSCID = pstRPPILog->ucBSCID;
			stWatchMsg.usBTSID = pstRPPILog->ucBTSID;
			stWatchMsg.ucSec = pstRPPILog->ucSECTOR;
			stWatchMsg.ucFA = pstRPPILog->ucFA_ID;
			stWatchMsg.uiPCFIP = pstRPPILog->uiPCFIP;
			stWatchMsg.ucPCFType = dGetPCFType(pstRPPILog->uiPCFIP);
			stWatchMsg.uiPDSNIP = pstRPPILog->uiNASName;
			stWatchMsg.uiAAAIP = pstRPPILog->uiAAAIP;
			stWatchMsg.uiHSSIP = pstRPPILog->uiHSSIP;
			stWatchMsg.ucSvcIdx = 0;
			stWatchMsg.usSvcL4Type = 0;
			stWatchMsg.uiSVCIP = 0;
			stWatchMsg.uiResult = (dErrorFlag == 1) ? pstRPPILog->uiLastFailReason : 0;

			dSendMonInfo(&stWatchMsg);
			log_print(LOGN_INFO, "SEND WATCH MSG[%d][%s] OFFICE[%d][%s] RESULT[%d]",
					stWatchMsg.usMsgType, PrintMonType(stWatchMsg.usMsgType),
					stWatchMsg.ucOffice, PrintOFFICE(stWatchMsg.ucOffice), stWatchMsg.uiResult);
			break;
		case DIAMETER_PROTO:
		case UPLCP_PROTO:
		case DNLCP_PROTO:
		case UPIPCP_PROTO:
		case DNIPCP_PROTO:
		case CHAP_PROTO:
		case PAP_PROTO:
		case OTHERPPP_PROTO:
			log_print(LOGN_WARN, "[%s][%s.%d] RECALL NOT PROC IMSI[%s] PROTOTYPE[%d] MSGTYPE[%d][%s]", __FILE__, __FUNCTION__, __LINE__,
					pstSIGNAL->szIMSI, pstSIGNAL->uiProtoType, pstSIGNAL->uiMsgType, PrintMsg(pstSIGNAL));
			break;      
		default:
			log_print(LOGN_WARN, "[%s][%s.%d] RECALL NOT DEFINE PROTO TYPE[%d]", __FILE__, __FUNCTION__, __LINE__, pstSIGNAL->uiProtoType);
			break;
	}  
	return 0;
}

