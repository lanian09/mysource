/**
 * INCLUDE HEADER FILES
 */
#include <stdlib.h>
#include <sys/time.h>

// DQMS
#include "common_stg.h"
#include "msgdef.h"

// LIB
#include "loglib.h"
#include "utillib.h"
#include "nifo.h"
#include "gifo.h"
#include "cifo.h"
#include "hasho.h"

// TAM
#include "filter.h"
#include "rppi_def.h"
#include "watch_mon.h"			/* st_WatchMsg */

// .
#include "roam_util.h"
#include "roam_msgq.h"
#include "roam_func.h"

/**
 *	Declare variables
 */
extern stMEMSINFO   	*pMEMSINFO;
extern stCIFO			*pCIFO;
extern stHASHOINFO  	*pHASHOINFO; 
extern stTIMERNINFO 	*pTIMERNINFO;
extern st_Flt_Info		*flt_info;

/**
 *	Declare functions
 */
extern void invoke_del_call(void *p);

/**
 * IMPLEMENT FUNCTIONS
 */
void vCheckFirst(HData_RPPI *pHASH, LOG_RPPI *pLOG, U8 *str, U32 SetupTime, U32 SetupMTime)
{
	S32				isSend = 0;
	U8      		szTime[BUFSIZ];
	OFFSET	offset = nifo_get_offset_node(pMEMSINFO, (U8*)pLOG);

	if(pHASH->before.dOffset == offset) {
		if(pHASH->before.uiFirstServFlag == 0) {
			log_print(LOGN_WARN, "PBG1 NOT FIRST IMSI=%s CALLTIME=%s.%u STR=%s", 
				pLOG->szIMSI, util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime, str);
			pHASH->before.uiFirstServFlag = 1;
			isSend = 1;

			pLOG->usCallState = dGetCallState(pLOG->usCallState, ROAM_ON_SERVICE_STATE);

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

			pLOG->usCallState = dGetCallState(pLOG->usCallState, ROAM_ON_SERVICE_STATE);

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

		stWatchMsg.usMsgType = WATCH_TYPE_A11;
		stWatchMsg.ucOffice = 0;
		stWatchMsg.ucSYSID = 0;
		stWatchMsg.ucRoamFlag = 1;
		stWatchMsg.ucBSCID = 0;
		stWatchMsg.usBTSID = 0;
		stWatchMsg.ucSec = 0;
		stWatchMsg.ucFA = 0;
		stWatchMsg.uiPCFIP = 0;
		stWatchMsg.ucPCFType = 0;
		stWatchMsg.uiPDSNIP = pLOG->uiNASName;
		stWatchMsg.uiAAAIP = pLOG->uiAAAIP;
		stWatchMsg.uiHSSIP = 0;
		stWatchMsg.uiLNSIP = pLOG->uiPCFIP;
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

S32 dPAGESessInfo(U8 *data)
{
	RPPISESS_KEY   	stRPPIKey, *pstRPPIKey;
	HData_RPPI      stRPPIHash, *pstRPPIHash;
	U32				dSvcThreshold;
	LOG_PAGE_TRANS  *pLOG;

	stHASHONODE     *pHASHONODE;
	LOG_RPPI        *pstRPPILog;
	HData			*pstHData;

	pstRPPIKey = &stRPPIKey;
	pstRPPIHash = &stRPPIHash;

	U8              szIP[INET_ADDRSTRLEN];
	U8              szTime[BUFSIZ];

	pLOG = (LOG_PAGE_TRANS *)data;
	dCvtIRM(pLOG->szIMSI);
	log_print(LOGN_DEBUG, "### RCV LOG_PAGE IMSI[%s] SVC[%ld][%s] ClientIP[%s] ERROR[%d] TIME[%s.%u]", 
			pLOG->szIMSI, pLOG->LastSvcL4Type, PrintSVC(pLOG->LastSvcL4Type), util_cvtipaddr(szIP, pLOG->uiClientIP), 
			pLOG->LastUserErrorCode, util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);

	if(pLOG->LastUserErrorCode > 0)
		pLOG->LastUserErrorCode = (SERVICE_DEFECT + SERVICE_PAGE_DEFECT + pLOG->LastUserErrorCode);

	memset(&stRPPIKey, 0x00, MAX_MIN_SIZE);
	memcpy(pstRPPIKey, pLOG->szIMSI, MAX_MIN_LEN);

	if((pHASHONODE = hasho_find(pHASHOINFO, (U8*)pstRPPIKey)) == NULL)
	{
//		log_print(LOGN_CRI, "LOG_PAGE NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
		log_print(LOGN_WARN, "LOG_PAGE NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
				pstRPPIKey->szIMSI, pLOG->LastSvcL4Type, PrintSVC(pLOG->LastSvcL4Type), 
				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1;
	}

	pstRPPIHash = (HData_RPPI *)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);

	pstHData = pFindCallWithTime(pstRPPIHash, pLOG->uiAccStartTime, pLOG->uiAccStartMTime);
	if(pstHData == NULL)
	{
//		log_print(LOGN_CRI, "LOG_PAGE NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
		log_print(LOGN_WARN, "LOG_PAGE NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
				pstRPPIKey->szIMSI, pLOG->LastSvcL4Type, PrintSVC(pLOG->LastSvcL4Type),
				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1; 
	}

	if(pstHData->uiTimerStop == 0) {
		pstHData->timerNID = timerN_update(pTIMERNINFO, pstHData->timerNID, time(NULL) + flt_info->stTimerInfo.usTimerInfo[PI_CALL_TIMEOUT] + RPPI_GAP_TIME);
	}
	pstRPPILog = (LOG_RPPI *)nifo_get_value(pMEMSINFO, LOG_RPPI_DEF_NUM, pstHData->dOffset);

	vCheckFirst(pstRPPIHash, pstRPPILog, "PAGE", pLOG->FirstTcpSynTime, pLOG->FirstTcpSynMTime);

	dUpdateCommonLog((LOG_COMMON *)pLOG, pstRPPILog);	

	/* FB TransCnt > 1일때만 처리 : 예외 처리 */
	if((pLOG->LastSvcL4Type == L4_FB) && (pLOG->TransCnt <= 1)) {
		log_print(LOGN_DEBUG, "SKIP LOG_PAGE FB L4=%ld TRANSCNT=%u", pLOG->LastSvcL4Type, pLOG->TransCnt);
		return 0;
	}

	pstRPPILog->uiMenuGetCnt++;
	if(pstRPPILog->uiMenuStartReqTime == 0)
	{
		pstRPPILog->uiMenuStartReqTime = pLOG->FirstL7ReqStartTime;
		pstRPPILog->uiMenuStartReqMTime = pLOG->FirstL7ReqStartMTime;
	}
	if(pstRPPILog->uiMNAckTime == 0)
	{
		pstRPPILog->uiMNAckTime = pLOG->LastL7MNAckTime;
		pstRPPILog->uiMNAckMTime = pLOG->LastL7MNAckMTime;
	}

	if(pLOG->LastUserErrorCode == 0) {
		pstRPPILog->uiMenuAckCnt++;
		pstRPPILog->llMenuDelayedTime += pLOG->PageGapTime;
	}
	else {
		pstRPPILog->uiLastFailReason = pLOG->LastUserErrorCode;
	}

	/** 지연 요소 **/
	switch(pLOG->LastSvcL4Type)
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

	if(dSvcThreshold > 0 && pLOG->PageGapTime > dSvcThreshold*1000) {
		pstRPPILog->uiLastFailReason = (SERVICE_DEFECT + SERVICE_DELAY_DEFECT + RESPONSETIME);
		pLOG->LastUserErrorCode = pstRPPILog->uiLastFailReason;
		pstRPPILog->uiMenuDelayedCnt++;
	}
	////////////////		

	if(pstRPPILog->uiFirstPlatformType == 0)
		pstRPPILog->uiFirstPlatformType = pLOG->LastPlatformType;
	if(pstRPPILog->uiFirstSvcL4Type == 0)
		pstRPPILog->uiFirstSvcL4Type = pLOG->LastSvcL4Type;
	if(pstRPPILog->uiFirstSvcL7Type == 0)
		pstRPPILog->uiFirstSvcL7Type = pLOG->LastSvcL7Type;
	pstRPPILog->uiLastPlatformType = pLOG->LastPlatformType;
	pstRPPILog->uiLastSvcL4Type = pLOG->LastSvcL4Type;
	pstRPPILog->uiLastSvcL7Type = pLOG->LastSvcL7Type;

	if(pstRPPILog->uiFirstSvcStartTime == 0)
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
	stWatchMsg.ucOffice = 0;
	stWatchMsg.ucSYSID = 0;
	stWatchMsg.ucRoamFlag = 1;
	stWatchMsg.ucBSCID = 0;
	stWatchMsg.usBTSID = 0;
	stWatchMsg.ucSec = 0;
	stWatchMsg.ucFA = 0;
	stWatchMsg.uiPCFIP = 0;
	stWatchMsg.ucPCFType = 0;
	stWatchMsg.uiPDSNIP = pstRPPILog->uiNASName;
	stWatchMsg.uiAAAIP = pstRPPILog->uiAAAIP;
	stWatchMsg.uiHSSIP = 0;
	stWatchMsg.uiLNSIP = pstRPPILog->uiPCFIP;
	stWatchMsg.ucSvcIdx = dGetSvcIndex(pLOG->LastPlatformType);
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
	HData			*pstHData;

	pstRPPIKey = &stRPPIKey;
	pstRPPIHash = &stRPPIHash;

	U8              szIP[INET_ADDRSTRLEN];
	U8              szTime[BUFSIZ];

	pLOG = (LOG_VOD_SESS *)data;
	dCvtIRM(pLOG->szIMSI);
	log_print(LOGN_DEBUG, "### RCV LOG_VOD IMSI[%s] SVC[%ld][%s] IP[%s] ERROR[%d] TIME[%s.%u]", 
			pLOG->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
			util_cvtipaddr(szIP, pLOG->uiClientIP), pLOG->usUserErrorCode, util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);

	if(pLOG->usUserErrorCode > 0)
		pLOG->usUserErrorCode =  SERVICE_DEFECT + SERVICE_VOD_DEFECT + pLOG->usUserErrorCode;

	memset(&stRPPIKey, 0x00, MAX_MIN_SIZE);
	memcpy(pstRPPIKey, pLOG->szIMSI, MAX_MIN_LEN);

	if((pHASHONODE = hasho_find(pHASHOINFO, (U8*)pstRPPIKey)) == NULL)
	{
//		log_print(LOGN_CRI, "LOG_VOD NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
		log_print(LOGN_WARN, "LOG_VOD NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
				pstRPPIKey->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1;
	}

	pstRPPIHash = (HData_RPPI *)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);

	pstHData = pFindCallWithTime(pstRPPIHash, pLOG->uiAccStartTime, pLOG->uiAccStartMTime);
	if(pstHData == NULL)
	{
//		log_print(LOGN_CRI, "LOG_VOD NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
		log_print(LOGN_WARN, "LOG_VOD NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
				pstRPPIKey->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1;
	}    

	if(pstHData->uiTimerStop == 0) {
		pstHData->timerNID = timerN_update(pTIMERNINFO, pstHData->timerNID, time(NULL) + flt_info->stTimerInfo.usTimerInfo[PI_CALL_TIMEOUT] + RPPI_GAP_TIME);
	}
	pstRPPILog = (LOG_RPPI *)nifo_get_value(pMEMSINFO, LOG_RPPI_DEF_NUM, pstHData->dOffset);
	
	vCheckFirst(pstRPPIHash, pstRPPILog, "VOD", pLOG->uiSetupStartTime, pLOG->uiSetupStartMTime);

	dUpdateCommonLog((LOG_COMMON *)pLOG, pstRPPILog);

	pstRPPILog->uiVODReqCnt++;
	if(pLOG->uiSetupEndTime !=0)
	{
		pstRPPILog->uiVODSetupCnt++;
	}

	/* 90500002 : NOTEARDOWN 예외 처리 성공으로 판단 */
	if((pLOG->usUserErrorCode > 0) && (pLOG->usUserErrorCode != 90500002))
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

	if(uiJitterThreshold > 0 && pLOG->uiMaxJitter > uiJitterThreshold) {
		pstRPPILog->uiLastFailReason = (SERVICE_DEFECT + SERVICE_DELAY_DEFECT + JITTER);
		pLOG->usUserErrorCode = pstRPPILog->uiLastFailReason;
		pstRPPILog->uiRTPJitterOverCnt++;
	}

	if(uiUpLossThreshold > 0 && pLOG->usRtpUpLossCnt > uiUpLossThreshold) {
		pstRPPILog->uiLastFailReason = (SERVICE_DEFECT + SERVICE_DELAY_DEFECT + PACKETLOSS);
		pLOG->usUserErrorCode = pstRPPILog->uiLastFailReason;
		pstRPPILog->uiRTPLossOverCnt++;	
	}

	if(uiDnLossThreshold > 0 && pLOG->usRtpDnLossCnt > uiDnLossThreshold) {
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

	if(pstRPPILog->uiFirstPlatformType == 0)
		pstRPPILog->uiFirstPlatformType = pLOG->usPlatformType;
	if(pstRPPILog->uiFirstSvcL4Type == 0)
		pstRPPILog->uiFirstSvcL4Type = pLOG->usSvcL4Type;
	if(pstRPPILog->uiFirstSvcL7Type == 0)
		pstRPPILog->uiFirstSvcL7Type = pLOG->usSvcL7Type;

	pstRPPILog->uiLastPlatformType = pLOG->usPlatformType;
	pstRPPILog->uiLastSvcL4Type = pLOG->usSvcL4Type;
	pstRPPILog->uiLastSvcL7Type = pLOG->usSvcL7Type;

	if(pstRPPILog->uiFirstSvcStartTime == 0)
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
	stWatchMsg.ucOffice = 0; 
	stWatchMsg.ucSYSID = 0;
	stWatchMsg.ucRoamFlag = 1;
	stWatchMsg.ucBSCID = 0;
	stWatchMsg.usBTSID = 0;
	stWatchMsg.ucSec = 0;
	stWatchMsg.ucFA = 0;
	stWatchMsg.uiPCFIP = 0; 
	stWatchMsg.ucPCFType = 0;
	stWatchMsg.uiPDSNIP = pstRPPILog->uiNASName;
	stWatchMsg.uiAAAIP = pstRPPILog->uiAAAIP;
	stWatchMsg.uiHSSIP = 0;
	stWatchMsg.uiLNSIP = pstRPPILog->uiPCFIP; 
	stWatchMsg.ucSvcIdx = dGetSvcIndex(pLOG->usPlatformType);
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
	HData			*pstHData;

	pstRPPIKey = &stRPPIKey;
	pstRPPIHash = &stRPPIHash;

	U8              szIP[INET_ADDRSTRLEN];
	U8              szTime[BUFSIZ];

	pLOG = (LOG_HTTP_TRANS *)data;
	dCvtIRM(pLOG->szIMSI);
	log_print(LOGN_DEBUG, "### RCV LOG_HTTP IMSI[%s] SVC[%ld][%s] IP[%s] ERROR[%d] TIME[%s.%u]", 
			pLOG->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
			util_cvtipaddr(szIP, pLOG->uiClientIP), pLOG->usUserErrorCode, util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);

	if(pLOG->usUserErrorCode > 0) 
		pLOG->usUserErrorCode = SERVICE_DEFECT + SERVICE_HTTP_DEFECT + pLOG->usUserErrorCode;

	memset(&stRPPIKey, 0x00, MAX_MIN_SIZE);
	memcpy(pstRPPIKey, pLOG->szIMSI, MAX_MIN_LEN);

	if((pHASHONODE = hasho_find(pHASHOINFO, (U8*)pstRPPIKey)) == NULL)
	{
//		log_print(LOGN_CRI, "LOG_HTTP NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
		log_print(LOGN_WARN, "LOG_HTTP NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
				pstRPPIKey->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1;
	}   

	pstRPPIHash = (HData_RPPI *)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);

	pstHData = pFindCallWithTime(pstRPPIHash, pLOG->uiAccStartTime, pLOG->uiAccStartMTime);
	if(pstHData == NULL)
	{
//		log_print(LOGN_CRI, "LOG_HTTP NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
		log_print(LOGN_WARN, "LOG_HTTP NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
				pstRPPIKey->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1;
	}

	if(pstHData->uiTimerStop == 0) {
		pstHData->timerNID = timerN_update(pTIMERNINFO, pstHData->timerNID, time(NULL) + flt_info->stTimerInfo.usTimerInfo[PI_CALL_TIMEOUT] + RPPI_GAP_TIME);
	}
	pstRPPILog = (LOG_RPPI *)nifo_get_value(pMEMSINFO, LOG_RPPI_DEF_NUM, pstHData->dOffset);

	vCheckFirst(pstRPPIHash, pstRPPILog, "HTTP", pLOG->uiTcpSynTime, pLOG->uiTcpSynMTime);

	dUpdateCommonLog((LOG_COMMON *)pLOG, pstRPPILog);

	/* WIDGET 예외 처리: AppFailCode 4000 ~ 4999 인 경우 제외 */
	if((pLOG->usSvcL4Type == L4_WIDGET) && (strlen(pLOG->szAppFailCode) == 4) && (pLOG->szAppFailCode[0] == '4')) {
		log_print(LOGN_DEBUG, "SKIP WIDGET AppFailCode=%.*s", MAX_APPFAILCODE_SIZE, pLOG->szAppFailCode);
		return 0;
	}

	if(pLOG->usUserErrorCode > 0) {
		pstRPPILog->uiLastFailReason = pLOG->usUserErrorCode;
	}

	/* 지연 요소 */
	switch(pLOG->usPlatformType)
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

	if(pstRPPILog->uiFirstPlatformType == 0)
		pstRPPILog->uiFirstPlatformType = pLOG->usPlatformType;
	if(pstRPPILog->uiFirstSvcL4Type == 0)
		pstRPPILog->uiFirstSvcL4Type = pLOG->usSvcL4Type;
	if(pstRPPILog->uiFirstSvcL7Type == 0)
		pstRPPILog->uiFirstSvcL7Type = pLOG->usSvcL7Type;
	pstRPPILog->uiLastPlatformType = pLOG->usPlatformType;
	pstRPPILog->uiLastSvcL4Type = pLOG->usSvcL4Type;
	pstRPPILog->uiLastSvcL7Type = pLOG->usSvcL7Type;

	if(pLOG->usPlatformType != DEF_PLATFORM_STREAM)
	{
		/* RTSP인 경우 VOD SESS에서 UPDATE */
		if(pstRPPILog->uiFirstSvcStartTime == 0)
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
		stWatchMsg.ucOffice = 0; 
		stWatchMsg.ucSYSID = 0;
		stWatchMsg.ucRoamFlag = 1;
		stWatchMsg.ucBSCID = 0;
		stWatchMsg.usBTSID = 0;
		stWatchMsg.ucSec = 0;
		stWatchMsg.ucFA = 0;
		stWatchMsg.uiPCFIP = 0;
		stWatchMsg.ucPCFType = 0;
		stWatchMsg.uiPDSNIP = pstRPPILog->uiNASName;
		stWatchMsg.uiAAAIP = pstRPPILog->uiAAAIP;
		stWatchMsg.uiHSSIP = 0;
		stWatchMsg.uiLNSIP = pstRPPILog->uiPCFIP;
		stWatchMsg.ucSvcIdx = dGetSvcIndex(pLOG->usPlatformType);
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
	HData			*pstHData;

	pstRPPIKey = &stRPPIKey;
	pstRPPIHash = &stRPPIHash;

	U8              szIP[INET_ADDRSTRLEN];
	U8              szTime[BUFSIZ];

	pLOG = (LOG_TCP_SESS *)data;
	dCvtIRM(pLOG->szIMSI);
	log_print(LOGN_DEBUG, "### RCV LOG_TCP IMSI[%s] SVC[%ld][%s] IP[%s] ERROR[%ld] TIME[%s.%u]", pLOG->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
			util_cvtipaddr(szIP, pLOG->uiClientIP), pLOG->usL4FailCode, util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);

	if(pLOG->usL4FailCode > 0)	
		pLOG->usL4FailCode = SERVICE_DEFECT + SERVICE_TCP_DEFECT + pLOG->usL4FailCode;

	memset(pstRPPIKey, 0x00, MAX_MIN_SIZE);
	memcpy(pstRPPIKey, pLOG->szIMSI, MAX_MIN_LEN);

	if((pHASHONODE = hasho_find(pHASHOINFO, (U8*)pstRPPIKey)) == NULL)
	{
//		log_print(LOGN_CRI, "LOG_TCP NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
		log_print(LOGN_WARN, "LOG_TCP NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
				pstRPPIKey->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1;
	}

	pstRPPIHash = (HData_RPPI *)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);
	pstHData = pFindCallWithTime(pstRPPIHash, pLOG->uiAccStartTime, pLOG->uiAccStartMTime);
	if(pstHData == NULL)
	{
//		log_print(LOGN_CRI, "LOG_TCP NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
		log_print(LOGN_WARN, "LOG_TCP NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
				pstRPPIKey->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1;
	}

	if(pstHData->uiTimerStop == 0) {
		pstHData->timerNID = timerN_update(pTIMERNINFO, pstHData->timerNID, time(NULL) + flt_info->stTimerInfo.usTimerInfo[PI_CALL_TIMEOUT] + RPPI_GAP_TIME);
	}
	pstRPPILog = (LOG_RPPI *)nifo_get_value(pMEMSINFO, LOG_RPPI_DEF_NUM, pstHData->dOffset);

	vCheckFirst(pstRPPIHash, pstRPPILog, "TCP", pLOG->uiTcpSynTime, pLOG->uiTcpSynMTime);

	dUpdateCommonLog((LOG_COMMON *)pLOG, pstRPPILog);

	pstRPPILog->uiTCPSynCnt += pLOG->ucTcpSynCnt;

	pstRPPILog->uiUpTCPPacketCnt += pLOG->uiIPTotUpPktCnt;
	pstRPPILog->uiDnTCPPacketCnt += pLOG->uiIPTotDnPktCnt;

	pstRPPILog->uiUpTCPRetransCnt += pLOG->uiIPTotUpRetransCnt;		
	pstRPPILog->uiDnTCPRetransCnt += pLOG->uiIPTotDnRetransCnt;

	if(pLOG->usSvcL4Type != L4_VOD_STREAM)
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

	if(pLOG->usSvcL4Type == L4_IV || pLOG->usPlatformType == DEF_PLATFORM_BANK || pLOG->usPlatformType == DEF_PLATFORM_CORP ||
			(pLOG->usPlatformType == DEF_PLATFORM_ETC && 
			 (pLOG->usSvcL4Type == L4_JNC || pLOG->usSvcL4Type == L4_WIPI_ONLINE)))
	{
		if(pstRPPILog->uiFirstPlatformType == 0)
			pstRPPILog->uiFirstPlatformType = pLOG->usPlatformType;
		if(pstRPPILog->uiFirstSvcL4Type == 0)
			pstRPPILog->uiFirstSvcL4Type = pLOG->usSvcL4Type;
		if(pstRPPILog->uiFirstSvcL7Type == 0)
			pstRPPILog->uiFirstSvcL7Type = APP_UNKNOWN;
		pstRPPILog->uiLastPlatformType = pLOG->usPlatformType;
		pstRPPILog->uiLastSvcL4Type = pLOG->usSvcL4Type;
		pstRPPILog->uiLastSvcL7Type = APP_UNKNOWN;

		if(pstRPPILog->uiFirstSvcStartTime == 0)
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
		stWatchMsg.ucOffice = 0;
		stWatchMsg.ucSYSID = 0;
		stWatchMsg.ucRoamFlag = 1;
		stWatchMsg.ucBSCID = 0;
		stWatchMsg.usBTSID = 0;
		stWatchMsg.ucSec = 0;
		stWatchMsg.ucFA = 0;
		stWatchMsg.uiPCFIP = 0;
		stWatchMsg.ucPCFType = 0;
		stWatchMsg.uiPDSNIP = pstRPPILog->uiNASName;
		stWatchMsg.uiAAAIP = pstRPPILog->uiAAAIP;
		stWatchMsg.uiHSSIP = 0;
		stWatchMsg.uiLNSIP = pstRPPILog->uiPCFIP;
		stWatchMsg.ucSvcIdx = dGetSvcIndex(pLOG->usPlatformType);
		stWatchMsg.usSvcL4Type = pLOG->usSvcL4Type;
		stWatchMsg.uiSVCIP = pLOG->uiServerIP;

		if(pLOG->uiTcpSynAckAckTime == 0)	
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

S32 dSIPSessInfo(U8 *data)
{
	RPPISESS_KEY   stRPPIKey, *pstRPPIKey;
	HData_RPPI      stRPPIHash, *pstRPPIHash;
	LOG_SIP_TRANS    *pLOG;
	stHASHONODE     *pHASHONODE; 
	LOG_RPPI        *pstRPPILog;
	HData			*pstHData;

	pstRPPIKey = &stRPPIKey;
	pstRPPIHash = &stRPPIHash;

	U8              szIP[INET_ADDRSTRLEN];
	U8              szTime[BUFSIZ];

	pLOG = (LOG_SIP_TRANS *)data;
	dCvtIRM(pLOG->szIMSI);
	log_print(LOGN_DEBUG, "### RCV LOG_SIP IMSI[%s] SVC[%ld][%s] IP[%s] ERROR[%d] TIME[%s.%u]", 
			pLOG->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
			util_cvtipaddr(szIP, pLOG->uiClientIP), pLOG->LastUserErrCode, util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);

	if(pLOG->LastUserErrCode > 0) 
		pLOG->LastUserErrCode = SERVICE_DEFECT + SERVICE_SIP_DEFECT + pLOG->LastUserErrCode;

	memset(pstRPPIKey, 0x00, MAX_MIN_SIZE);
	memcpy(pstRPPIKey, pLOG->szIMSI, MAX_MIN_LEN);

	if((pHASHONODE = hasho_find(pHASHOINFO, (U8*)pstRPPIKey)) == NULL)
	{
//		log_print(LOGN_CRI, "LOG_SIP NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
		log_print(LOGN_WARN, "LOG_SIP NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
				pstRPPIKey->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1;       
	}

	pstRPPIHash = (HData_RPPI *)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);

	pstHData = pFindCallWithTime(pstRPPIHash, pLOG->uiAccStartTime, pLOG->uiAccStartMTime);
	if(pstHData == NULL)
	{
//		log_print(LOGN_CRI, "LOG_SIP NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
		log_print(LOGN_WARN, "LOG_SIP NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
				pstRPPIKey->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1;
	}

	if(pstHData->uiTimerStop == 0) {
		pstHData->timerNID = timerN_update(pTIMERNINFO, pstHData->timerNID, time(NULL) + flt_info->stTimerInfo.usTimerInfo[PI_CALL_TIMEOUT] + RPPI_GAP_TIME);
	}
	pstRPPILog = (LOG_RPPI *)nifo_get_value(pMEMSINFO, LOG_RPPI_DEF_NUM, pstHData->dOffset);

	vCheckFirst(pstRPPIHash, pstRPPILog, "SIP", pLOG->TransStartTime, pLOG->TransStartMTime);

	dUpdateCommonLog((LOG_COMMON *)pLOG, pstRPPILog);

	if(!pLOG->isUsed)
	{
		if(pLOG->usSvcL7Type == APP_IM_UP)
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
	
		if(pstRPPILog->uiFirstPlatformType == 0)
				pstRPPILog->uiFirstPlatformType = pLOG->usPlatformType;
		if(pstRPPILog->uiFirstSvcL4Type == 0)
				pstRPPILog->uiFirstSvcL4Type = pLOG->usSvcL4Type;
		if(pstRPPILog->uiFirstSvcL7Type == 0)
				pstRPPILog->uiFirstSvcL7Type = pLOG->usSvcL7Type;
		pstRPPILog->uiLastPlatformType = pLOG->usPlatformType;
		pstRPPILog->uiLastSvcL4Type = pLOG->usSvcL4Type;
		pstRPPILog->uiLastSvcL7Type = pLOG->usSvcL7Type;

		if(pstRPPILog->uiFirstSvcStartTime == 0)
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

	if(pLOG->LastUserErrCode > 0) {
		pstRPPILog->uiLastFailReason = pLOG->LastUserErrCode;
	}

	if(pstRPPILog->uiFirstTCPSynTime == 0) {
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
		stWatchMsg.ucOffice = 0;
		stWatchMsg.ucSYSID = 0;
		stWatchMsg.ucRoamFlag = 1;
		stWatchMsg.ucBSCID = 0;
		stWatchMsg.usBTSID = 0;
		stWatchMsg.ucSec = 0;
		stWatchMsg.ucFA = 0;
		stWatchMsg.uiPCFIP = 0;
		stWatchMsg.ucPCFType = 0;
		stWatchMsg.uiPDSNIP = pstRPPILog->uiNASName;
		stWatchMsg.uiAAAIP = pstRPPILog->uiAAAIP;
		stWatchMsg.uiHSSIP = 0;
		stWatchMsg.uiLNSIP = pstRPPILog->uiPCFIP;
		stWatchMsg.ucSvcIdx = (pLOG->method == SIP_MSG_REGISTER) ? SVC_IDX_REGI : dGetSvcIndex(pLOG->usPlatformType);
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
	HData			*pstHData;

	pstRPPIKey = &stRPPIKey;
	pstRPPIHash = &stRPPIHash;

	U8              szIP[INET_ADDRSTRLEN];
	U8              szTime[BUFSIZ];

	pLOG = (LOG_MSRP_TRANS *)data;
	dCvtIRM(pLOG->szIMSI);
	log_print(LOGN_DEBUG, "### RCV LOG_MSRP IMSI[%s] SVC[%ld][%s] IP[%s] ERROR[%d] TIME[%s.%u]", 
			pLOG->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
			util_cvtipaddr(szIP, pLOG->uiClientIP), pLOG->LastUserErrCode, util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);

	if(pLOG->LastUserErrCode > 0) 
		pLOG->LastUserErrCode = SERVICE_DEFECT + SERVICE_MSRP_DEFECT + pLOG->LastUserErrCode;

	memset(pstRPPIKey, 0x00, MAX_MIN_SIZE);
	memcpy(pstRPPIKey, pLOG->szIMSI, MAX_MIN_LEN);

	if((pHASHONODE = hasho_find(pHASHOINFO, (U8*)pstRPPIKey)) == NULL)
	{
//		log_print(LOGN_CRI, "LOG_MSRP NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
		log_print(LOGN_WARN, "LOG_MSRP NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
				pstRPPIKey->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1;      
	}

	pstRPPIHash = (HData_RPPI *)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);

	pstHData = pFindCallWithTime(pstRPPIHash, pLOG->uiAccStartTime, pLOG->uiAccStartMTime);
	if(pstHData == NULL)
	{
//		log_print(LOGN_CRI, "LOG_MSRP NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
		log_print(LOGN_WARN, "LOG_MSRP NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
				pstRPPIKey->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1;
	}

	if(pstHData->uiTimerStop == 0) {
		pstHData->timerNID = timerN_update(pTIMERNINFO, pstHData->timerNID, time(NULL) + flt_info->stTimerInfo.usTimerInfo[PI_CALL_TIMEOUT] + RPPI_GAP_TIME);
	}
	pstRPPILog = (LOG_RPPI *)nifo_get_value(pMEMSINFO, LOG_RPPI_DEF_NUM, pstHData->dOffset);
 
	vCheckFirst(pstRPPIHash, pstRPPILog, "MSRP", pLOG->TransStartTime, pLOG->TransStartMTime);

	dUpdateCommonLog((LOG_COMMON *)pLOG, pstRPPILog);

	/* MSRP 예외 처리 : contenttype = 'application/im-iscomposing+xml' 이면 제외 */
	char	szTmpMsrp[BUFSIZ] = "application/im-iscomposing+xml";
	if(!strncmp(pLOG->ContentsType, szTmpMsrp, strlen(szTmpMsrp))) {
		log_print(LOGN_DEBUG, "SKIP LOG_MSRP ContentType=%.*s", MSRP_CONTENTTYPE_SIZE, pLOG->ContentsType);
		return 0;
	}

	if(pLOG->LastUserErrCode > 0) {
		pstRPPILog->uiLastFailReason = pLOG->LastUserErrCode;
	}
	if(pstRPPILog->uiFirstPlatformType == 0)
		pstRPPILog->uiFirstPlatformType = pLOG->usPlatformType;
	if(pstRPPILog->uiFirstSvcL4Type == 0)
		pstRPPILog->uiFirstSvcL4Type = pLOG->usSvcL4Type;
	if(pstRPPILog->uiFirstSvcL7Type == 0)
		pstRPPILog->uiFirstSvcL7Type = pLOG->usSvcL7Type;
	pstRPPILog->uiLastPlatformType = pLOG->usPlatformType;
	pstRPPILog->uiLastSvcL4Type = pLOG->usSvcL4Type;
	pstRPPILog->uiLastSvcL7Type = pLOG->usSvcL7Type; 	

	/** WATCH DATA */
	st_WatchMsg stWatchMsg;
	memset(&stWatchMsg, 0x00, DEF_WATCHMSG_SIZE);

	stWatchMsg.usMsgType = WATCH_TYPE_SVC;
	stWatchMsg.ucOffice = 0;
	stWatchMsg.ucSYSID = 0;
	stWatchMsg.ucRoamFlag = 1;
	stWatchMsg.ucBSCID = 0;
	stWatchMsg.usBTSID = 0;
	stWatchMsg.ucSec = 0;
	stWatchMsg.ucFA = 0;
	stWatchMsg.uiPCFIP = 0;
	stWatchMsg.ucPCFType = 0;
	stWatchMsg.uiPDSNIP = pstRPPILog->uiNASName;
	stWatchMsg.uiAAAIP = pstRPPILog->uiAAAIP;
	stWatchMsg.uiHSSIP = 0;
	stWatchMsg.uiLNSIP = pstRPPILog->uiPCFIP;
	stWatchMsg.ucSvcIdx = dGetSvcIndex(pLOG->usPlatformType);
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
	HData			*pstHData;

	pstRPPIKey = &stRPPIKey;
	pstRPPIHash = &stRPPIHash;

	U8              szIP[INET_ADDRSTRLEN];
	U8              szTime[BUFSIZ];

	pLOG = (LOG_VT_SESS *)data;
	dCvtIRM(pLOG->szIMSI);
	log_print(LOGN_DEBUG, "### RCV LOG_VT IMSI[%s] SVC[%ld][%s] IP[%s] ERROR[%d] TIME[%s.%u]", 
			pLOG->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
			util_cvtipaddr(szIP, pLOG->uiClientIP), pLOG->LastUserErrCode, util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);

	if(pLOG->LastUserErrCode > 0) 
		pLOG->LastUserErrCode = SERVICE_DEFECT + SERVICE_VT_DEFECT + pLOG->LastUserErrCode;

	memset(pstRPPIKey, 0x00, MAX_MIN_SIZE);
	memcpy(pstRPPIKey, pLOG->szIMSI, MAX_MIN_LEN);

	if((pHASHONODE = hasho_find(pHASHOINFO, (U8*)pstRPPIKey)) == NULL)
	{
//		log_print(LOGN_CRI, "LOG_VT NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
		log_print(LOGN_WARN, "LOG_VT NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
				pstRPPIKey->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1;
	}

	pstRPPIHash = (HData_RPPI *)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);

	pstHData = pFindCallWithTime(pstRPPIHash, pLOG->uiAccStartTime, pLOG->uiAccStartMTime);
	if(pstHData == NULL)
	{
//		log_print(LOGN_CRI, "LOG_VT NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
		log_print(LOGN_WARN, "LOG_VT NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
				pstRPPIKey->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1;
	}

	if(pstHData->uiTimerStop == 0) {
		pstHData->timerNID = timerN_update(pTIMERNINFO, pstHData->timerNID, time(NULL) + flt_info->stTimerInfo.usTimerInfo[PI_CALL_TIMEOUT] + RPPI_GAP_TIME);
	}
	pstRPPILog = (LOG_RPPI *)nifo_get_value(pMEMSINFO, LOG_RPPI_DEF_NUM, pstHData->dOffset);

	vCheckFirst(pstRPPIHash, pstRPPILog, "VT", pLOG->SessStartTime, pLOG->SessStartMTime);

	dUpdateCommonLog((LOG_COMMON *)pLOG, pstRPPILog);

	pstRPPILog->uiVTReqCnt++;
	if(pLOG->SetupEndTime !=0)
		pstRPPILog->uiVTSetupCnt++;
	if(pLOG->LastUserErrCode > 0) {	
		pstRPPILog->uiLastFailReason = pLOG->LastUserErrCode;
	}

	/** 지연 요소 **/
	uiUpJitterThreshold = dGetThreshold(SERVICE_VT, ALARM_UPJITTER);
	uiDnJitterThreshold = dGetThreshold(SERVICE_VT, ALARM_DNJITTER);
	uiUpLossThreshold = dGetThreshold(SERVICE_VT, ALARM_UPPACKETLOSS);
	uiUpLossThreshold = dGetThreshold(SERVICE_VT, ALARM_DNPACKETLOSS);

	if(uiUpJitterThreshold > 0 && pLOG->UpMaxJitter > uiUpJitterThreshold) {
		pstRPPILog->uiLastFailReason = (SERVICE_DEFECT + SERVICE_DELAY_DEFECT + JITTER);
		pLOG->LastUserErrCode = pstRPPILog->uiLastFailReason;
		pstRPPILog->uiRTPJitterOverCnt++;
	}

	if(uiDnJitterThreshold > 0 && pLOG->DnMaxJitter > uiDnJitterThreshold) {
		pstRPPILog->uiLastFailReason = (SERVICE_DEFECT + SERVICE_DELAY_DEFECT + JITTER);
		pLOG->LastUserErrCode = pstRPPILog->uiLastFailReason;
		pstRPPILog->uiRTPJitterOverCnt++;
	}

	if(uiUpLossThreshold > 0 && pLOG->RTPUpLossCnt > uiUpLossThreshold) {
		pstRPPILog->uiLastFailReason = (SERVICE_DEFECT + SERVICE_DELAY_DEFECT + PACKETLOSS);
		pLOG->LastUserErrCode = pstRPPILog->uiLastFailReason;
		pstRPPILog->uiRTPLossOverCnt++;
	}

	if(uiDnLossThreshold > 0 && pLOG->RTPDnLossCnt > uiDnLossThreshold) {
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


	if(pstRPPILog->uiFirstPlatformType == 0)
		pstRPPILog->uiFirstPlatformType = pLOG->usPlatformType;
	if(pstRPPILog->uiFirstSvcL4Type == 0)
		pstRPPILog->uiFirstSvcL4Type = pLOG->usSvcL4Type;
	if(pstRPPILog->uiFirstSvcL7Type == 0)
		pstRPPILog->uiFirstSvcL7Type = pLOG->usSvcL7Type;
	pstRPPILog->uiLastPlatformType = pLOG->usPlatformType;
	pstRPPILog->uiLastSvcL4Type = pLOG->usSvcL4Type;
	pstRPPILog->uiLastSvcL7Type = pLOG->usSvcL7Type;

	if(pstRPPILog->uiFirstSvcStartTime == 0)
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
	stWatchMsg.ucOffice = 0; 
	stWatchMsg.ucSYSID = 0;
	stWatchMsg.ucRoamFlag = 1;
	stWatchMsg.ucBSCID = 0;
	stWatchMsg.usBTSID = 0;
	stWatchMsg.ucSec = 0;
	stWatchMsg.ucFA = 0;
	stWatchMsg.uiPCFIP = 0;
	stWatchMsg.ucPCFType = 0;
	stWatchMsg.uiPDSNIP = pstRPPILog->uiNASName;
	stWatchMsg.uiAAAIP = pstRPPILog->uiAAAIP;
	stWatchMsg.uiHSSIP = 0;
	stWatchMsg.uiLNSIP = pstRPPILog->uiPCFIP;
	stWatchMsg.ucSvcIdx = dGetSvcIndex(pLOG->usPlatformType);
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
	HData			*pstHData;

	pstRPPIKey = &stRPPIKey;
	pstRPPIHash = &stRPPIHash;

	U8              szIP[INET_ADDRSTRLEN];
	U8              szTime[BUFSIZ];

	pLOG = (LOG_IM_SESS *)data;
	dCvtIRM(pLOG->szIMSI);
	log_print(LOGN_DEBUG, "### RCV LOG_IM IMSI[%s] SVC[%ld][%s] IP[%s] ERROR[%d] TIME[%s.%u]", 
			pLOG->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
			util_cvtipaddr(szIP, pLOG->uiClientIP), pLOG->LastUserErrCode, util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);

	if(pLOG->LastUserErrCode >0 )     
		pLOG->LastUserErrCode = (SERVICE_DEFECT + SERVICE_IM_DEFECT + pLOG->LastUserErrCode);

	memset(pstRPPIKey, 0x00, MAX_MIN_SIZE);
	memcpy(pstRPPIKey, pLOG->szIMSI, MAX_MIN_LEN);

	if((pHASHONODE = hasho_find(pHASHOINFO, (U8*)pstRPPIKey)) == NULL)
	{
//		log_print(LOGN_CRI, "LOG_IM NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
		log_print(LOGN_WARN, "LOG_IM NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
				pstRPPIKey->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1;
	}   

	pstRPPIHash = (HData_RPPI *)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);

	pstHData = pFindCallWithTime(pstRPPIHash, pLOG->uiAccStartTime, pLOG->uiAccStartMTime);
	if(pstHData == NULL)
	{
//		log_print(LOGN_CRI, "LOG_IM NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
		log_print(LOGN_WARN, "LOG_IM NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
				pstRPPIKey->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1;
	}

	if(pstHData->uiTimerStop == 0) {
		pstHData->timerNID = timerN_update(pTIMERNINFO, pstHData->timerNID, time(NULL) + flt_info->stTimerInfo.usTimerInfo[PI_CALL_TIMEOUT] + RPPI_GAP_TIME);
	}
	pstRPPILog = (LOG_RPPI *)nifo_get_value(pMEMSINFO, LOG_RPPI_DEF_NUM, pstHData->dOffset);

	vCheckFirst(pstRPPIHash, pstRPPILog, "IM", pLOG->SessStartTime, pLOG->SessStartMTime);

	dUpdateCommonLog((LOG_COMMON *)pLOG, pstRPPILog);

	pstRPPILog->uiIMReqCnt++;
	if(pstRPPILog->uiIMStartReqTime == 0)
	{
		pstRPPILog->uiIMStartReqTime = pLOG->SessStartTime;
		pstRPPILog->uiIMStartReqMTime = pLOG->SessStartMTime;
	}
	if(pstRPPILog->uiIMSetupTime == 0)
	{
		pstRPPILog->uiIMSetupTime = pLOG->SetupEndTime;
		pstRPPILog->uiIMSetupMTime = pLOG->SetupEndMTime;
	}

	if(pLOG->SetupEndTime != 0) {
		pstRPPILog->uiIMSetupCnt++;
	}
	if(pLOG->LastUserErrCode >0 ){          
		pstRPPILog->uiLastFailReason = pLOG->LastUserErrCode;
	}


	/***** 지연 요소 ******/
	dSvcThreshold = dGetThreshold(SERVICE_IM, ALARM_RESPONSETIME);
	if(dSvcThreshold > 0 && pLOG->SessGapTime > dSvcThreshold*1000) {
		pstRPPILog->uiLastFailReason = (SERVICE_DEFECT + SERVICE_DELAY_DEFECT + RESPONSETIME);
		pLOG->LastUserErrCode = pstRPPILog->uiLastFailReason;
	}
	/////////////////////////// 

	if(pstRPPILog->uiFirstPlatformType == 0)
		pstRPPILog->uiFirstPlatformType = pLOG->usPlatformType;
	if(pstRPPILog->uiFirstSvcL4Type == 0)
		pstRPPILog->uiFirstSvcL4Type = pLOG->usSvcL4Type;
	if(pstRPPILog->uiFirstSvcL7Type == 0)
		pstRPPILog->uiFirstSvcL7Type = pLOG->usSvcL7Type;
	pstRPPILog->uiLastPlatformType = pLOG->usPlatformType;
	pstRPPILog->uiLastSvcL4Type = pLOG->usSvcL4Type;
	pstRPPILog->uiLastSvcL7Type = pLOG->usSvcL7Type;

	if(pstRPPILog->uiFirstSvcStartTime == 0)
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
	stWatchMsg.ucOffice = 0;
	stWatchMsg.ucSYSID = 0;
	stWatchMsg.ucRoamFlag = 1;
	stWatchMsg.ucBSCID = 0;
	stWatchMsg.usBTSID = 0;
	stWatchMsg.ucSec = 0;
	stWatchMsg.ucFA = 0;
	stWatchMsg.uiPCFIP = 0;
	stWatchMsg.ucPCFType = 0;
	stWatchMsg.uiPDSNIP = pstRPPILog->uiNASName;
	stWatchMsg.uiAAAIP = pstRPPILog->uiAAAIP;
	stWatchMsg.uiHSSIP = 0;
	stWatchMsg.uiLNSIP = pstRPPILog->uiPCFIP;
	stWatchMsg.ucSvcIdx = dGetSvcIndex(pLOG->usPlatformType);
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
	HData			*pstHData;

	pstRPPIKey = &stRPPIKey;
	pstRPPIHash = &stRPPIHash;

	U8              szIP[INET_ADDRSTRLEN];
	U8              szTime[BUFSIZ];

	pLOG = (LOG_FTP *)data;
	dCvtIRM(pLOG->szIMSI);
	log_print(LOGN_DEBUG, "### RCV LOG_FTP IMSI[%s] SVC[%ld][%s] IP[%s] TIME[%s.%u]", 
			pLOG->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
			util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);


	memset(pstRPPIKey, 0x00, MAX_MIN_SIZE);
	memcpy(pstRPPIKey, pLOG->szIMSI, MAX_MIN_LEN);

	if((pHASHONODE = hasho_find(pHASHOINFO, (U8*)pstRPPIKey)) == NULL)
	{
//		log_print(LOGN_CRI, "LOG_FTP NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
		log_print(LOGN_WARN, "LOG_FTP NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
				pstRPPIKey->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1;
	}

	pstRPPIHash = (HData_RPPI *)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);

	pstHData = pFindCallWithTime(pstRPPIHash, pLOG->uiAccStartTime, pLOG->uiAccStartMTime);
	if(pstHData == NULL)
	{   
//		log_print(LOGN_CRI, "LOG_FTP NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
		log_print(LOGN_WARN, "LOG_FTP NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
				pstRPPIKey->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1;
	}

	if(pstHData->uiTimerStop == 0) {
		pstHData->timerNID = timerN_update(pTIMERNINFO, pstHData->timerNID, time(NULL) + flt_info->stTimerInfo.usTimerInfo[PI_CALL_TIMEOUT] + RPPI_GAP_TIME);
	}
	pstRPPILog = (LOG_RPPI *)nifo_get_value(pMEMSINFO, LOG_RPPI_DEF_NUM, pstHData->dOffset);

	vCheckFirst(pstRPPIHash, pstRPPILog, "FTP", pLOG->uiTcpSynTime, pLOG->uiTcpSynMTime);

	dUpdateCommonLog((LOG_COMMON *)pLOG, pstRPPILog);

	if(pstRPPILog->uiFirstPlatformType == 0)
		pstRPPILog->uiFirstPlatformType = pLOG->usPlatformType;
	if(pstRPPILog->uiFirstSvcL4Type == 0)
		pstRPPILog->uiFirstSvcL4Type = pLOG->usSvcL4Type;
	if(pstRPPILog->uiFirstSvcL7Type == 0)
		pstRPPILog->uiFirstSvcL7Type = pLOG->usSvcL7Type;
	pstRPPILog->uiLastPlatformType = pLOG->usPlatformType;
	pstRPPILog->uiLastSvcL4Type = pLOG->usSvcL4Type;
	pstRPPILog->uiLastSvcL7Type = pLOG->usSvcL7Type;

	if(pstRPPILog->uiFirstSvcStartTime == 0)
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
	HData			*pstHData;

	pstRPPIKey = &stRPPIKey;
	pstRPPIHash = &stRPPIHash;

	U8              szIP[INET_ADDRSTRLEN];
	U8              szTime[BUFSIZ];

	pLOG = (LOG_DIALUP_SESS *)data;
	dCvtIRM(pLOG->szIMSI);
	log_print(LOGN_DEBUG, "### RCV LOG_DIALUP IMSI[%s] SVC[%ld][%s] IP[%s] TIME[%s.%u]", 
			pLOG->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
			util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);


	memset(pstRPPIKey, 0x00, MAX_MIN_SIZE);
	memcpy(pstRPPIKey, pLOG->szIMSI, MAX_MIN_LEN);

	if((pHASHONODE = hasho_find(pHASHOINFO, (U8*)pstRPPIKey)) == NULL)
	{
//		log_print(LOGN_CRI, "LOG_DIALUP NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
		log_print(LOGN_WARN, "LOG_DIALUP NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
				pstRPPIKey->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1;
	}

	pstRPPIHash = (HData_RPPI*)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);

	pstHData = pFindCallWithTime(pstRPPIHash, pLOG->uiAccStartTime, pLOG->uiAccStartMTime);
	if (pstHData == NULL)
	{
//		log_print(LOGN_CRI, "LOG_DIALUP NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
		log_print(LOGN_WARN, "LOG_DIALUP NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
				pstRPPIKey->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1;
	}

	if(pstHData->uiTimerStop == 0) {
		pstHData->timerNID = timerN_update(pTIMERNINFO, pstHData->timerNID, time(NULL) + flt_info->stTimerInfo.usTimerInfo[PI_CALL_TIMEOUT] + RPPI_GAP_TIME);
	}
	pstRPPILog = (LOG_RPPI *)nifo_get_value(pMEMSINFO, LOG_RPPI_DEF_NUM, pstHData->dOffset);

	vCheckFirst(pstRPPIHash, pstRPPILog, "DIALUP", pLOG->SessStartTime, pLOG->SessStartMTime);

	dUpdateCommonLog((LOG_COMMON *)pLOG, pstRPPILog);

	pstRPPILog->uiUpTotPktCnt += pLOG->UpPktCnt;
	pstRPPILog->uiDnTotPktCnt += pLOG->DnPktCnt;
	pstRPPILog->uiUpTotDataSize += pLOG->UpPktSize;
	pstRPPILog->uiDnTotDataSize += pLOG->DnPktSize;

	if(pstRPPILog->uiFirstPlatformType == 0)
		pstRPPILog->uiFirstPlatformType = pLOG->usPlatformType;
	if(pstRPPILog->uiFirstSvcL4Type == 0)
		pstRPPILog->uiFirstSvcL4Type = pLOG->usSvcL4Type;
	if(pstRPPILog->uiFirstSvcL7Type == 0)
		pstRPPILog->uiFirstSvcL7Type = pLOG->usSvcL7Type;
	pstRPPILog->uiLastPlatformType = pLOG->usPlatformType;
	pstRPPILog->uiLastSvcL4Type = pLOG->usSvcL4Type;
	pstRPPILog->uiLastSvcL7Type = pLOG->usSvcL7Type;

	if(pstRPPILog->uiFirstSvcStartTime == 0)
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
	stWatchMsg.ucOffice = 0;
	stWatchMsg.ucSYSID = 0;
	stWatchMsg.ucRoamFlag = 1;
	stWatchMsg.ucBSCID = 0;
	stWatchMsg.usBTSID = 0;
	stWatchMsg.ucSec = 0;
	stWatchMsg.ucFA = 0;
	stWatchMsg.uiPCFIP = 0;
	stWatchMsg.ucPCFType = 0;
	stWatchMsg.uiPDSNIP = pstRPPILog->uiNASName;
	stWatchMsg.uiAAAIP = pstRPPILog->uiAAAIP;
	stWatchMsg.uiHSSIP = 0;
	stWatchMsg.uiLNSIP = pstRPPILog->uiPCFIP;
	stWatchMsg.ucSvcIdx = dGetSvcIndex(pLOG->usPlatformType);
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
			log_print(LOGN_CRI, "### RCV NOTIFY SIGNAL FLT ACCESS");
			break;
		case MID_FLT_DEFECT_THRES:
			log_print(LOGN_CRI, "### RCV NOTIFY SIGNAL FLT DEFFECT");
			dMakeDefectHash();
			break;
		case MID_FLT_MODEL:
			log_print(LOGN_CRI, "### RCV NOTIFY SIGNAL FLT MODEL");
			dMakeModelHash();
			break;
		case MID_FLT_EQUIP:
			log_print(LOGN_CRI, "### RCV NOTIFY SIGNAL FLT NASIP");
			dMakeNASIPHash();
			break;
		case MID_FLT_IRM:
			log_print(LOGN_CRI, "### RCV NOTIFY SIGNAL FLT IRM");
			dMakeIRMHash();
			break;
		default :
			log_print(LOGN_CRI, "[%s][%s.%d] UNKNOWN Type[%d]", __FILE__, __FUNCTION__, __LINE__, pNOTIFY->uiType);	
			break;
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

	stHASHONODE     *pHASHONODE;
	LOG_RPPI        *pstRPPILog;
	HData			*pstHData;

	pstRPPIKey = &stRPPIKey;
	pstRPPIHash = &stRPPIHash;

	U8              szIP[INET_ADDRSTRLEN];
	U8              szTime[BUFSIZ];
	U8              szSTime[BUFSIZ];

	pLOG = (LOG_COMMON *)data;
	dCvtIRM(pLOG->szIMSI);
	log_print(LOGN_DEBUG, "### RCV FIRST START SERVICE IMSI[%s] IP[%s] TIME[%s.%u] SVCTIME[%s.%u]", 
			pLOG->szIMSI, util_cvtipaddr(szIP, pLOG->uiClientIP),
			util_printtime(pLOG->uiAccStartTime, szTime), pLOG->uiAccStartMTime,
			util_printtime(pLOG->uiCallTime, szSTime), pLOG->uiCallMTime);


	memset(pstRPPIKey, 0x00, MAX_MIN_SIZE);
	memcpy(pstRPPIKey, pLOG->szIMSI, MAX_MIN_LEN);

	if((pHASHONODE = hasho_find(pHASHOINFO, (U8*)pstRPPIKey)) == NULL)
	{
//		log_print(LOGN_CRI, "FIRST START NOT FOUND HASH IMSI[%s] ClientIP[%s] TIME[%s.%u] SVCTIME[%s.%u]",
		log_print(LOGN_WARN, "FIRST START NOT FOUND HASH IMSI[%s] ClientIP[%s] TIME[%s.%u] SVCTIME[%s.%u]",
				pstRPPIKey->szIMSI,  util_cvtipaddr(szIP, pLOG->uiClientIP), 
				util_printtime(pLOG->uiAccStartTime, szTime), pLOG->uiAccStartMTime,
				util_printtime(pLOG->uiCallTime, szSTime), pLOG->uiCallMTime);
		return -1;
	}

	pstRPPIHash = (HData_RPPI *)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);

	pstHData = pFindCallWithTime(pstRPPIHash, pLOG->uiAccStartTime, pLOG->uiAccStartMTime);
	if(pstHData == NULL)
	{
//		log_print(LOGN_CRI, "FIRST START NOT FOUND RPPI LOG IMSI[%s] ClientIP[%s] TIME[%s.%u] SVCTIME[%s.%u]",
		log_print(LOGN_WARN, "FIRST START NOT FOUND RPPI LOG IMSI[%s] ClientIP[%s] TIME[%s.%u] SVCTIME[%s.%u]",
				pstRPPIKey->szIMSI, util_cvtipaddr(szIP, pLOG->uiClientIP),
				util_printtime(pLOG->uiAccStartTime, szTime), pLOG->uiAccStartMTime,
				util_printtime(pLOG->uiCallTime, szSTime), pLOG->uiCallMTime);
		return -1;
	}

	if(pstHData->uiTimerStop == 0) {
		pstHData->timerNID = timerN_update(pTIMERNINFO, pstHData->timerNID, time(NULL) + flt_info->stTimerInfo.usTimerInfo[PI_CALL_TIMEOUT] + RPPI_GAP_TIME);
	}
	pstRPPILog = (LOG_RPPI *)nifo_get_value(pMEMSINFO, LOG_RPPI_DEF_NUM, pstHData->dOffset);

	pstRPPILog->usCallState = dGetCallState(pstRPPILog->usCallState, ROAM_ON_SERVICE_STATE);

	/* UP IPCP PPP_SETUP이 없는 경우 First Service 시간을 세팅 함 : First Service 시간은 예외적으로 CallTime을 사용 */
	if(pstRPPILog->uiPPPSetupTime == 0) {
		pstRPPILog->uiPPPSetupTime = pLOG->uiCallTime;
		pstRPPILog->uiPPPSetupMTime = pLOG->uiCallMTime;
	}

	offset = nifo_get_offset_node(pMEMSINFO, (U8*)pstRPPILog);

	if(pstRPPIHash->before.dOffset == offset) {
		if (pstRPPIHash->before.uiFirstServFlag == 0)
			isSendWatch = 1;
		else
			isSendWatch = 0;	
		pstRPPIHash->before.uiFirstServFlag = 1;
	}
	else if(pstRPPIHash->after.dOffset == offset) {
		if (pstRPPIHash->after.uiFirstServFlag == 0)
			isSendWatch = 1;
		else
			isSendWatch = 0;	
		pstRPPIHash->after.uiFirstServFlag = 1;
	}
	else {
//		log_print(LOGN_CRI, "FIRST START NOT FOUND OFFSET IMSI[%s] ClientIP[%s] TIME[%s.%u] SVCTIME[%s.%u]",
		log_print(LOGN_WARN, "FIRST START NOT FOUND OFFSET IMSI[%s] ClientIP[%s] TIME[%s.%u] SVCTIME[%s.%u]",
				pstRPPIKey->szIMSI, util_cvtipaddr(szIP, pLOG->uiClientIP),
				util_printtime(pLOG->uiAccStartTime, szTime), pLOG->uiAccStartMTime,
				util_printtime(pLOG->uiCallTime, szSTime), pLOG->uiCallMTime);
		return -1;
	}

	if(isSendWatch)
	{
		/** WATCH DATA */
		st_WatchMsg stWatchMsg;
		memset(&stWatchMsg, 0x00, DEF_WATCHMSG_SIZE);

		stWatchMsg.usMsgType = WATCH_TYPE_A11;
		stWatchMsg.ucOffice = 0;
		stWatchMsg.ucSYSID = 0;
		stWatchMsg.ucRoamFlag = 1;
		stWatchMsg.ucBSCID = 0;
		stWatchMsg.usBTSID = 0;
		stWatchMsg.ucSec = 0;
		stWatchMsg.ucFA = 0;
		stWatchMsg.uiPCFIP = 0;
		stWatchMsg.ucPCFType = 0;
		stWatchMsg.uiPDSNIP = pstRPPILog->uiNASName;
		stWatchMsg.uiAAAIP = pstRPPILog->uiAAAIP;
		stWatchMsg.uiHSSIP = 0;
		stWatchMsg.uiLNSIP = pstRPPILog->uiPCFIP;
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
	HData			*pstHData;

	pstRPPIKey = &stRPPIKey;
	pstRPPIHash = &stRPPIHash;

	U8              szIP[INET_ADDRSTRLEN];
	U8              szTime[BUFSIZ];

	pLOG = (LOG_DNS *)data;
	dCvtIRM(pLOG->szIMSI);
	log_print(LOGN_DEBUG, "### RCV LOG_DNS IMSI[%s] SVC[%ld][%s] IP[%s] ERROR[%d] TIME[%s.%u]", 
			pLOG->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
			util_cvtipaddr(szIP, pLOG->uiClientIP), pLOG->ucErrorCode, util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);


	memset(pstRPPIKey, 0x00, MAX_MIN_SIZE);
	memcpy(pstRPPIKey, pLOG->szIMSI, MAX_MIN_LEN);

	if((pHASHONODE = hasho_find(pHASHOINFO, (U8*)pstRPPIKey)) == NULL)
	{
//		log_print(LOGN_CRI, "LOG_DNS NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
		log_print(LOGN_WARN, "LOG_DNS NOT FOUND HASH IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]",
				pstRPPIKey->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1;
	}

	pstRPPIHash = (HData_RPPI *)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);

	pstHData = pFindCallWithTime(pstRPPIHash, pLOG->uiAccStartTime, pLOG->uiAccStartMTime);
	if(pstHData == NULL)
	{
//		log_print(LOGN_CRI, "LOG_DNS NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]", 
		log_print(LOGN_WARN, "LOG_DNS NOT FOUND RPPI LOG IMSI[%s] SVC[%ld][%s] ClientIP[%s] TIME[%s.%u]", 
				pstRPPIKey->szIMSI, pLOG->usSvcL4Type, PrintSVC(pLOG->usSvcL4Type),
				util_cvtipaddr(szIP, pLOG->uiClientIP), util_printtime(pLOG->uiCallTime, szTime), pLOG->uiCallMTime);
		return -1;
	}

	if(pstHData->uiTimerStop == 0) {
		pstHData->timerNID = timerN_update(pTIMERNINFO, pstHData->timerNID, time(NULL) + flt_info->stTimerInfo.usTimerInfo[PI_CALL_TIMEOUT] + RPPI_GAP_TIME);
	}
	pstRPPILog = (LOG_RPPI *)nifo_get_value(pMEMSINFO, LOG_RPPI_DEF_NUM, pstHData->dOffset);

	vCheckFirst(pstRPPIHash, pstRPPILog, "DNS", pLOG->dRequestTime, pLOG->dRequestMTime);

	dUpdateCommonLog((LOG_COMMON *)pLOG, pstRPPILog);

	pstRPPILog->uiDNSReqCnt += pLOG->usRequestCnt;
	pstRPPILog->uiDNSResCnt += pLOG->usResponseCnt;

/*
	pstRPPILog->uiDNSReqCnt++;
	if (pLOG->ucErrorCode == 0)
		pstRPPILog->uiDNSResCnt++;
*/

	if(pstRPPILog->uiDNSReqTime == 0)
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

HData *pCallStartInfo(U8 *data)
{
	S32				dSvcThreshold, dErrorFlag;
	LOG_RPPI_ERR	*pstErrLog;
	st_WatchMsg		stWatchMsg;

	RPPISESS_KEY    stRPPIKey, *pstRPPIKey;
	HData_RPPI      stRPPIHash, *pstRPPIHash;
	LOG_SIGNAL      *pstSIGNAL;
	stHASHONODE     *pHASHONODE;
	LOG_RPPI        *pstRPPILog;
	HData			*pstHData = NULL;

	U8              szIP[INET_ADDRSTRLEN];
	U8              szTime[BUFSIZ];	

	U32				uiFirstServFlag, uiTimer = 0;
	U16				usCallState;
	U32				uiDef;
	U8				*pstLogNode;
	RPPI_TIMER		RPPITIMER;

	pstRPPIKey = &stRPPIKey; 
	pstRPPIHash = &stRPPIHash;

	pstSIGNAL = (LOG_SIGNAL *)data;

	log_print(LOGN_DEBUG, "@@@ RCV CALLSTART IMSI[%s] IP[%s] TIME[%s.%u]", 
			pstSIGNAL->szIMSI, util_cvtipaddr(szIP, pstSIGNAL->uiClientIP), 
			util_printtime(pstSIGNAL->uiCallTime, szTime), pstSIGNAL->uiCallMTime);

	dErrorFlag = 0;
	if(pstSIGNAL->uiMsgType == RADIUS_ACCOUNT_MSG) {
		usCallState = ROAM_ACCOUNT_STATE;
		uiDef = AAA_ACCOUNTING_DEFECT;
	}
	else {
		usCallState = ROAM_ACCESS_STATE;
		uiDef = AAA_ACCESS_DEFECT;
	}

	memset(pstRPPIKey, 0x00, MAX_MIN_SIZE);	
	memcpy(pstRPPIKey->szIMSI, pstSIGNAL->szIMSI, MAX_MIN_SIZE);

	pHASHONODE = hasho_find(pHASHOINFO, (U8*)pstRPPIKey);
	if(pHASHONODE != NULL) {
		pstRPPIHash = (HData_RPPI *)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);

		if(pstRPPIHash->after.dOffset != 0) {
			if(pstRPPIHash->after.ulCorrelationID == pstSIGNAL->ulCorrelationID) {
				/* Delete After Call */
				log_print(LOGN_DEBUG, "DUPLICATE CORRELATION ID IMSI=%s AFTER=%llu SIGNAL=%llu",
						pstSIGNAL->szIMSI, pstRPPIHash->after.ulCorrelationID, pstSIGNAL->ulCorrelationID);
				pstRPPILog = (LOG_RPPI *)nifo_get_value(pMEMSINFO, LOG_RPPI_DEF_NUM, pstRPPIHash->after.dOffset);
				uiFirstServFlag = pstRPPIHash->after.uiFirstServFlag;
			}
			else {
				/* Delete Before Call */
				log_print(LOGN_DEBUG, "FULL BEFORE AFTER CALL IMSI=%s BEFORE=%llu AFTER=%llu SIGNAL=%llu",
						pstSIGNAL->szIMSI, pstRPPIHash->before.ulCorrelationID, 
						pstRPPIHash->after.ulCorrelationID, pstSIGNAL->ulCorrelationID);
				pstRPPILog = (LOG_RPPI *)nifo_get_value(pMEMSINFO, LOG_RPPI_DEF_NUM, pstRPPIHash->before.dOffset);
				uiFirstServFlag = pstRPPIHash->before.uiFirstServFlag;
			}

			dProcCallStop(pstRPPIHash, pstRPPILog, uiFirstServFlag);
			if(pstRPPILog->uiLastFailReason == 0) {
				pstRPPILog->uiLastFailReason = A11_DEFECT + CALL_SETUP + ERR_CALL_DUPLICATE;
			}
			dDelRPPI(pstRPPIHash, pstRPPILog);			
		}
		else {
			if(pstRPPIHash->before.ulCorrelationID == pstSIGNAL->ulCorrelationID) {
				/* Delete Before Call */
				log_print(LOGN_DEBUG, "DUPLICATE CORRELATION ID IMSI=%s BEFORE=%llu SIGNAL=%llu",
						pstSIGNAL->szIMSI, pstRPPIHash->before.ulCorrelationID, pstSIGNAL->ulCorrelationID);
				pstRPPILog = (LOG_RPPI *)nifo_get_value(pMEMSINFO, LOG_RPPI_DEF_NUM, pstRPPIHash->before.dOffset);
				uiFirstServFlag = pstRPPIHash->before.uiFirstServFlag;
				dProcCallStop(pstRPPIHash, pstRPPILog, uiFirstServFlag);
				if(pstRPPILog->uiLastFailReason == 0) {
					pstRPPILog->uiLastFailReason = A11_DEFECT + CALL_SETUP + ERR_CALL_DUPLICATE;
				}
				dDelRPPI(pstRPPIHash, pstRPPILog);			

				/** ADD HASH  **/
				memset(pstRPPIHash, 0x00, HDATA_RPPI_SIZE);
				if((pHASHONODE = hasho_add(pHASHOINFO, (U8*)pstRPPIKey, (U8*)pstRPPIHash)) == NULL)
				{
					log_print(LOGN_CRI, "[%s][%s.%d] hasho_add NULL IMSI[%s] TIME[%s.%u]", 
							__FILE__, __FUNCTION__, __LINE__,
							pstSIGNAL->szIMSI, util_printtime(pstSIGNAL->uiCallTime, szTime), pstSIGNAL->uiCallMTime);
					return NULL;
				}
				else
				{
					pstRPPIHash = (HData_RPPI *)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);
				}
			}
		}
	}
	else {
		/** ADD HASH  **/
		memset(pstRPPIHash, 0x00, HDATA_RPPI_SIZE);
		if((pHASHONODE = hasho_add(pHASHOINFO, (U8*)pstRPPIKey, (U8*)pstRPPIHash)) == NULL)
		{
			log_print(LOGN_CRI, "[%s][%s.%d] hasho_add NULL IMSI[%s] TIME[%s.%u]", 
					__FILE__, __FUNCTION__, __LINE__,
					pstSIGNAL->szIMSI, util_printtime(pstSIGNAL->uiCallTime, szTime), pstSIGNAL->uiCallMTime);
			return NULL;
		}
		else
		{
			pstRPPIHash = (HData_RPPI *)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);
		}
	}

	if((pstLogNode = nifo_node_alloc(pMEMSINFO)) == NULL)
	{
		log_print(LOGN_CRI, "[%s][%s.%d] nifo_node_alloc NULL IMSI[%s] TIME[%s.%u]", 
				__FILE__, __FUNCTION__, __LINE__,
				pstSIGNAL->szIMSI, util_printtime(pstSIGNAL->uiCallTime, szTime), pstSIGNAL->uiCallMTime);
		if((pstRPPIHash->before.dOffset == 0) && (pstRPPIHash->after.dOffset == 0)) hasho_del(pHASHOINFO, (U8 *)pstRPPIKey);
		return NULL;
	}

	if((pstRPPILog = (LOG_RPPI *)nifo_tlv_alloc(pMEMSINFO, pstLogNode, LOG_RPPI_DEF_NUM, LOG_RPPI_SIZE, DEF_MEMSET_ON)) == NULL)
	{
		log_print(LOGN_CRI, "[%s][%s.%d] nifo_tlv_alloc NULL IMSI[%s] TIME[%s.%u]", 
				__FILE__, __FUNCTION__, __LINE__,
				pstSIGNAL->szIMSI, util_printtime(pstSIGNAL->uiCallTime, szTime), pstSIGNAL->uiCallMTime);
		if((pstRPPIHash->before.dOffset == 0) && (pstRPPIHash->after.dOffset == 0)) hasho_del(pHASHOINFO, (U8 *)pstRPPIKey);
		nifo_node_delete(pMEMSINFO, pstLogNode);
		return NULL;
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

	pstRPPILog->usRegiReqCnt++;
	if(pstSIGNAL->uiLastUserErrCode > 0) {
		pstRPPILog->uiLastFailReason = (AAA_DEFECT + uiDef + pstSIGNAL->uiLastUserErrCode);
		/* 에러 세팅 변경 해야 함 */
		pstRPPILog->uiSetupFailReason = pstRPPILog->uiLastFailReason;
		uiTimer = flt_info->stTimerInfo.usTimerInfo[PI_WAIT_TIMEOUT] + RPPI_GAP_TIME;
		dErrorFlag = 1;
	}
	else {
		pstRPPILog->usRegiSuccRepCnt++;
		pstRPPILog->uiSetupFailReason = SETUP_SUCESS;
		pstRPPILog->uiLastFailReason = 0;
		uiTimer = flt_info->stTimerInfo.usTimerInfo[PI_CALL_TIMEOUT] + RPPI_GAP_TIME;
	}

	pstRPPILog->uiFirstUpdateReqTime = pstSIGNAL->uiSessStartTime;
	pstRPPILog->uiFirstUpdateReqMTime = pstSIGNAL->uiSessStartMTime;
	pstRPPILog->uiFirstUpdateResTime = pstSIGNAL->uiSessEndTime;
	pstRPPILog->uiFirstUpdateResMTime = pstSIGNAL->uiSessEndMTime;
	pstRPPILog->uiRegiRepCode = pstSIGNAL->uiLastUserErrCode;	

	pstRPPILog->llDiaSuccSumTime += pstSIGNAL->uiSessDuration;
	pstRPPILog->uiClientIP = pstSIGNAL->uiClientIP;
	pstRPPILog->uiNASName = pstSIGNAL->uiNasIP;
	pstRPPILog->uiAAAIP = pstSIGNAL->uiDestIP;
	pstRPPILog->usServiceType = pstSIGNAL->usServiceType;

	pstRPPILog->llRPDuration = pstSIGNAL->uiSessDuration;

	memcpy(pstRPPILog->szNetOption, pstSIGNAL->szNetOption, MAX_SVCOPTION_LEN);
	memcpy(pstRPPILog->szBSMSC, pstSIGNAL->szBSMSC, DEF_BSMSD_LENGTH);
	memcpy(pstRPPILog->szFirstBSMSC, pstSIGNAL->szBSMSC, DEF_BSMSD_LENGTH);	

	pstRPPILog->ucBranchID = dGetNationID(pstSIGNAL->uiNasIP);
//	pstRPPILog->usCallType = INIT_ROAM_CALLSTART;			
	pstRPPILog->usCallType = dGetCallType(pstRPPILog->ucBranchID);			
	pstRPPILog->uiTmpAuthReqTime = pstSIGNAL->uiSessStartTime;
	pstRPPILog->uiTmpAuthReqMTime = pstSIGNAL->uiSessStartMTime;
	pstRPPILog->uiTmpAuthEndTime = pstSIGNAL->uiSessEndTime;
	pstRPPILog->uiTmpAuthEndMTime = pstSIGNAL->uiSessEndMTime;

	pstRPPILog->usCallState = usCallState;
	pstRPPILog->ucFA_ID = pstSIGNAL->ucFA_ID;
	pstRPPILog->ucSECTOR = pstSIGNAL->ucSECTOR;
	pstRPPILog->ucSYSID = pstSIGNAL->ucSYSID;
	pstRPPILog->ucBSCID = pstSIGNAL->ucBSCID;
	pstRPPILog->ucBTSID = pstSIGNAL->ucBTSID;

	memset(&RPPITIMER, 0x00, sizeof(RPPI_TIMER));
	memcpy(&RPPITIMER.RPPIKEY, pstRPPIKey->szIMSI, MAX_MIN_SIZE);
	RPPITIMER.uiCallTime = pstSIGNAL->uiCallTime;
	RPPITIMER.uiCallMTime = pstSIGNAL->uiCallMTime;

	if(pstRPPIHash->before.dOffset == 0)
	{
		pstRPPIHash->before.dOffset = nifo_get_offset_node(pMEMSINFO, (U8*)pstRPPILog);	
		pstRPPIHash->before.timerNID = timerN_add(pTIMERNINFO,  invoke_del_call, (U8*)&RPPITIMER, sizeof(RPPI_TIMER), time(NULL) + uiTimer);
		pstRPPIHash->before.uiCallTime = pstSIGNAL->uiCallTime;
		pstRPPIHash->before.uiCallMTime = pstSIGNAL->uiCallMTime;
		pstRPPIHash->before.ulCorrelationID = pstSIGNAL->ulCorrelationID;
		pstHData = &pstRPPIHash->before;
	}
	else
	{
		pstRPPIHash->after.dOffset = nifo_get_offset_node(pMEMSINFO, (U8*)pstRPPILog); 
		pstRPPIHash->after.timerNID = timerN_add(pTIMERNINFO,  invoke_del_call, (U8*)&RPPITIMER, sizeof(RPPI_TIMER), time(NULL) + uiTimer);
		pstRPPIHash->after.uiCallTime = pstSIGNAL->uiCallTime;
		pstRPPIHash->after.uiCallMTime = pstSIGNAL->uiCallMTime;
		pstRPPIHash->after.ulCorrelationID = pstSIGNAL->ulCorrelationID;
		pstHData = &pstRPPIHash->after;
	}	

	dSvcThreshold = dGetThreshold(SERVICE_RADIUS, ALARM_RESPONSETIME);
	if(dSvcThreshold > 0 && pstSIGNAL->uiSessDuration > dSvcThreshold*1000) {
		pstRPPILog->uiLastFailReason = (AAA_DEFECT + uiDef + RESPONSETIME);
		pstRPPILog->uiSetupFailReason = pstRPPILog->uiLastFailReason;
		dErrorFlag = 1;
	}
//	if(dErrorFlag == 1 && pstSIGNAL->uiMsgType == RADIUS_ACCESS_MSG)
	if(pstSIGNAL->uiMsgType == RADIUS_ACCESS_MSG)
	{
		if(dErrorFlag == 1) {
			log_print(LOGN_INFO, "RADIUS ACCESS ERRCODE[%d] LastFailReason[%u]", 
				pstSIGNAL->uiLastUserErrCode, pstRPPILog->uiLastFailReason);
			pstErrLog = dCreateErrLog(pstRPPILog);
			if(pstErrLog != NULL)
			{
				pstErrLog->uiSessStartTime = pstSIGNAL->uiSessStartTime;
				pstErrLog->uiSessStartMTime = pstSIGNAL->uiSessStartMTime;
				pstErrLog->usProtoType = RADIUS_PROTO;
				pstErrLog->uiErrorCode = pstRPPILog->uiLastFailReason;
			}
		}
		/* SEND WATCH*/
		memset(&stWatchMsg, 0x00, DEF_WATCHMSG_SIZE);

//		stWatchMsg.usMsgType = WATCH_TYPE_A11AAA;
		stWatchMsg.usMsgType = WATCH_TYPE_AAA;
		stWatchMsg.ucOffice = 0;
		stWatchMsg.ucSYSID = 0;
		stWatchMsg.ucRoamFlag = 1;
		stWatchMsg.ucBSCID = 0;
		stWatchMsg.usBTSID = 0;
		stWatchMsg.ucSec = 0;
		stWatchMsg.ucFA = 0;
		stWatchMsg.uiPCFIP = 0;
		stWatchMsg.ucPCFType = 0;
		stWatchMsg.uiPDSNIP = pstRPPILog->uiNASName;
		stWatchMsg.uiAAAIP = pstRPPILog->uiAAAIP;
		stWatchMsg.uiHSSIP = 0;
		stWatchMsg.uiLNSIP = pstRPPILog->uiPCFIP;
		stWatchMsg.ucSvcIdx = 0;
		stWatchMsg.usSvcL4Type = 0;
		stWatchMsg.uiSVCIP = 0;
		stWatchMsg.uiResult = (dErrorFlag == 1) ? pstRPPILog->uiLastFailReason : 0;;

		dSendMonInfo(&stWatchMsg);
		log_print(LOGN_INFO, "SEND WATCH MSG[%d][%s] OFFICE[%d][%s] RESULT[%d]",
				stWatchMsg.usMsgType, PrintMonType(stWatchMsg.usMsgType),
				stWatchMsg.ucOffice, PrintOFFICE(stWatchMsg.ucOffice), stWatchMsg.uiResult);
	}		

	return pstHData;
}

S32 dAccessInfo(U8 *data)
{
	S32				dSvcThreshold, dErrorFlag, isCallControl;
	LOG_RPPI_ERR	*pstErrLog;
	st_WatchMsg		stWatchMsg;

	RPPISESS_KEY    stRPPIKey, *pstRPPIKey;
	HData_RPPI      stRPPIHash, *pstRPPIHash;
	LOG_SIGNAL      *pstSIGNAL;
	stHASHONODE     *pHASHONODE;
	LOG_RPPI        *pstRPPILog;
	HData			*pstHData;

	U8              szIP[INET_ADDRSTRLEN];
	U8              szPCFIP[INET_ADDRSTRLEN];
	U8              szPDSNIP[INET_ADDRSTRLEN];
	U8              szTime[BUFSIZ];	

	pstRPPIKey = &stRPPIKey; 
	pstRPPIHash = &stRPPIHash;

	pstSIGNAL = (LOG_SIGNAL *)data;
	dCvtIRM(pstSIGNAL->szIMSI);

	log_print(LOGN_DEBUG, "### RCV RADIUS ACCESS IMSI[%s] PROTOTYPE[%d] MSGTYPE[%d][%s] NAT[%d]",
			pstSIGNAL->szIMSI, pstSIGNAL->uiProtoType, pstSIGNAL->uiMsgType, PrintMsg(pstSIGNAL), pstSIGNAL->ucBranchID);

	dErrorFlag = 0;
	isCallControl = dGetCallControl(pstSIGNAL->ucBranchID);
	if(isCallControl) {
		pstHData = pCallStartInfo(data);
		if(pstHData == NULL) {
			log_print(LOGN_WARN, "pCallStartInfo NULL IMSI[%s] PROTOTYPE[%d] MSGTYPE[%d][%s] NAT[%d]",
				pstSIGNAL->szIMSI, pstSIGNAL->uiProtoType, pstSIGNAL->uiMsgType, PrintMsg(pstSIGNAL), pstSIGNAL->ucBranchID);
			return -11;
		}
		pstRPPILog = (LOG_RPPI *)nifo_get_value(pMEMSINFO, LOG_RPPI_DEF_NUM, pstHData->dOffset);
		dUpdateCommonLog((LOG_COMMON *)pstSIGNAL, pstRPPILog);	
	}
	else {
		memset(pstRPPIKey, 0x00, MAX_MIN_SIZE);	
		memcpy(pstRPPIKey->szIMSI, pstSIGNAL->szIMSI, MAX_MIN_SIZE);

		pHASHONODE = hasho_find(pHASHOINFO, (U8*)pstRPPIKey);
		if(pHASHONODE != NULL)
		{
			pstRPPIHash = (HData_RPPI *)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);

			pstHData = pFindCallWithTime(pstRPPIHash, pstSIGNAL->uiAccStartTime, pstSIGNAL->uiAccStartMTime);
			if(pstHData == NULL)
			{
				log_print(LOGN_WARN, "[%s] NOT FOUND RPPI LOG IMSI[%s] IP[%s] TIME[%s.%u] PDSNIP[%s]",
						PrintMsg(pstSIGNAL), pstSIGNAL->szIMSI, util_cvtipaddr(szIP, pstSIGNAL->uiClientIP),
						util_printtime(pstSIGNAL->uiAccStartTime, szTime), pstSIGNAL->uiAccStartMTime, util_cvtipaddr(szPDSNIP, pstSIGNAL->uiNASName));
				return -1;
			}	

			if(pstHData->uiTimerStop == 0) {
				pstHData->timerNID = timerN_update(pTIMERNINFO, pstHData->timerNID, time(NULL) + flt_info->stTimerInfo.usTimerInfo[PI_CALL_TIMEOUT] + RPPI_GAP_TIME);
			}
			pstRPPILog = (LOG_RPPI *)nifo_get_value(pMEMSINFO, LOG_RPPI_DEF_NUM, pstHData->dOffset);
		}
		else
		{
			log_print(LOGN_WARN, "NOT FOUND [%s:%u:%u] IMSI[%s] IP[%s] ERROR[%d] TIME[%s.%u] PCFIP[%s]", 
					PrintMsg(pstSIGNAL), pstSIGNAL->uiProtoType, pstSIGNAL->uiMsgType, pstSIGNAL->szIMSI, 
					util_cvtipaddr(szIP, pstSIGNAL->uiClientIP), pstSIGNAL->ucErrorCode, 
					util_printtime(pstSIGNAL->uiCallTime, szTime), pstSIGNAL->uiCallMTime,
					util_cvtipaddr(szPCFIP, pstSIGNAL->uiPCFIP));
			return -2;
		}

		pstRPPILog->usCallState = dGetCallState(pstRPPILog->usCallState, ROAM_LNS_ACCESS_STATE);
//		pstRPPILog->usCallType = INIT_ROAM_CALLSTART_LNS;

#if 0
		/* LNS IP */
		pstRPPILog->uiPCFIP = pstSIGNAL->uiSrcIP;
#endif

		if(pstSIGNAL->usServiceType != 0) {
			pstRPPILog->usServiceType = pstSIGNAL->usServiceType;
		}
		pstRPPILog->uiAccessReqTime = pstSIGNAL->uiSessStartTime;
		pstRPPILog->uiAccessReqMTime = pstSIGNAL->uiSessStartMTime;
		pstRPPILog->uiAccessResTime = pstSIGNAL->uiSessEndTime;
		pstRPPILog->uiAccessResMTime = pstSIGNAL->uiSessEndMTime;
		pstRPPILog->usAccessResCode = pstSIGNAL->uiLastUserErrCode;	
		pstRPPILog->usInterimTime = pstSIGNAL->usInterimTime;

		pstRPPILog->llDiaSuccSumTime += pstSIGNAL->uiSessDuration;
		pstRPPILog->usActiveStartCnt++;
//		pstRPPILog->usRegiReqCnt++;
		if(pstSIGNAL->uiLastUserErrCode > 0) {
			pstRPPILog->uiLastFailReason = (AAA_DEFECT + AAA_ACCESS_DEFECT + pstSIGNAL->uiLastUserErrCode);
			pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(pstRPPILog->usCallState, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
			dErrorFlag = 1;
		}
		else {
//			pstRPPILog->usRegiSuccRepCnt++;
			pstRPPILog->usDiaReqCnt++;
		}

		dSvcThreshold = dGetThreshold(SERVICE_RADIUS, ALARM_RESPONSETIME);
		if(dSvcThreshold > 0 && pstSIGNAL->uiSessDuration > dSvcThreshold*1000) {
			pstRPPILog->uiLastFailReason = (AAA_DEFECT + SERVICE_DELAY_DEFECT + RESPONSETIME);
			dErrorFlag = 1;
		}
		if(dErrorFlag == 1)
		{
			log_print(LOGN_INFO, "RADIUS ACCESS ERRCODE[%d] LastFailReason[%u]", 
					pstSIGNAL->uiLastUserErrCode, pstRPPILog->uiLastFailReason);
			pstErrLog = dCreateErrLog(pstRPPILog);
			if(pstErrLog != NULL)
			{
				pstErrLog->uiSessStartTime = pstSIGNAL->uiSessStartTime;
				pstErrLog->uiSessStartMTime = pstSIGNAL->uiSessStartMTime;
				pstErrLog->usProtoType = RADIUS_PROTO;
				pstErrLog->uiErrorCode = pstRPPILog->uiLastFailReason;
			}
		}		

		dUpdateCommonLog((LOG_COMMON *)pstSIGNAL, pstRPPILog);	


		/* SEND WATCH*/
		memset(&stWatchMsg, 0x00, DEF_WATCHMSG_SIZE);

		stWatchMsg.usMsgType = WATCH_TYPE_AAA;
		stWatchMsg.ucOffice = 0;
		stWatchMsg.ucSYSID = 0;
		stWatchMsg.ucRoamFlag = 1;
		stWatchMsg.ucBSCID = 0;
		stWatchMsg.usBTSID = 0;
		stWatchMsg.ucSec = 0;
		stWatchMsg.ucFA = 0;
		stWatchMsg.uiPCFIP = 0;
		stWatchMsg.ucPCFType = 0;
		stWatchMsg.uiPDSNIP = pstRPPILog->uiNASName;
		stWatchMsg.uiAAAIP = pstRPPILog->uiAAAIP;
		stWatchMsg.uiHSSIP = 0;
		stWatchMsg.uiLNSIP = pstRPPILog->uiPCFIP;
		stWatchMsg.ucSvcIdx = 0;
		stWatchMsg.usSvcL4Type = 0;
		stWatchMsg.uiSVCIP = 0;
		stWatchMsg.uiResult = (dErrorFlag == 1) ? pstRPPILog->uiLastFailReason : 0;

		dSendMonInfo(&stWatchMsg);
		log_print(LOGN_INFO, "SEND WATCH MSG[%d][%s] OFFICE[%d][%s] RESULT[%d]",
				stWatchMsg.usMsgType, PrintMonType(stWatchMsg.usMsgType),
				stWatchMsg.ucOffice, PrintOFFICE(stWatchMsg.ucOffice), stWatchMsg.uiResult);
	}
	
	return 0;
}

S32 dAccountInfo(U8 *data)
{
	S32				dSvcThreshold, dErrorFlag, isCallControl;
	LOG_RPPI_ERR	*pstErrLog;
	st_WatchMsg		stWatchMsg;

	RPPISESS_KEY    stRPPIKey, *pstRPPIKey;
	HData_RPPI      stRPPIHash, *pstRPPIHash;
	LOG_SIGNAL      *pstSIGNAL;
	stHASHONODE     *pHASHONODE;
	LOG_RPPI        *pstRPPILog;
	HData			*pstHData = NULL;

	U8              szIP[INET_ADDRSTRLEN];
	U8              szPCFIP[INET_ADDRSTRLEN];
	U8              szPDSNIP[INET_ADDRSTRLEN];
	U8              szTime[BUFSIZ];	

	U32				uiTimer;

	pstRPPIKey = &stRPPIKey; 
	pstRPPIHash = &stRPPIHash;

	pstSIGNAL = (LOG_SIGNAL *)data;
	dCvtIRM(pstSIGNAL->szIMSI);

	log_print(LOGN_DEBUG, "### RCV RADIUS ACCOUNT IMSI[%s] PROTOTYPE[%d] MSGTYPE[%d][%s] NAT[%d]",
			pstSIGNAL->szIMSI, pstSIGNAL->uiProtoType, pstSIGNAL->uiMsgType, PrintMsg(pstSIGNAL), pstSIGNAL->ucBranchID);

	isCallControl = dGetCallControl(pstSIGNAL->ucBranchID);
	
	memset(pstRPPIKey, 0x00, MAX_MIN_SIZE); 
	memcpy(pstRPPIKey->szIMSI, pstSIGNAL->szIMSI, MAX_MIN_SIZE);

	if((pHASHONODE = hasho_find(pHASHOINFO, (U8*)pstRPPIKey)))
	{
		pstRPPIHash = (HData_RPPI*)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);

		pstHData = (isCallControl) ? pFindCallWithCorrID(pstRPPIHash, pstSIGNAL->ulCorrelationID) : pFindCallWithTime(pstRPPIHash, pstSIGNAL->uiAccStartTime, pstSIGNAL->uiAccStartMTime);
		if(pstHData == NULL) {
			log_print(LOGN_WARN, "[%s] NOT FOUND RPPI LOG IMSI[%s] IP[%s] TIME[%s.%u] PDSNIP[%s]",
				PrintMsg(pstSIGNAL), pstSIGNAL->szIMSI, util_cvtipaddr(szIP, pstSIGNAL->uiClientIP),
				util_printtime(pstSIGNAL->uiAccStartTime, szTime), pstSIGNAL->uiAccStartMTime, util_cvtipaddr(szPDSNIP, pstSIGNAL->uiNASName));
		}
		else {
			if(pstHData->uiTimerStop == 0) {
				pstHData->timerNID = timerN_update(pTIMERNINFO, pstHData->timerNID, time(NULL) + flt_info->stTimerInfo.usTimerInfo[PI_CALL_TIMEOUT] + RPPI_GAP_TIME);
			}

			pstRPPILog = (LOG_RPPI *)nifo_get_value(pMEMSINFO, LOG_RPPI_DEF_NUM, pstHData->dOffset);
		}
	}
	else
	{
		log_print(LOGN_WARN, "NOT FOUND [%s:%u:%u] IMSI[%s] IP[%s] ERROR[%d] TIME[%s.%u] PCFIP[%s]", 
			PrintMsg(pstSIGNAL), pstSIGNAL->uiProtoType, pstSIGNAL->uiMsgType, pstSIGNAL->szIMSI, 
			util_cvtipaddr(szIP, pstSIGNAL->uiClientIP), pstSIGNAL->ucErrorCode, 
			util_printtime(pstSIGNAL->uiCallTime, szTime), pstSIGNAL->uiCallMTime,
			util_cvtipaddr(szPCFIP, pstSIGNAL->uiPCFIP));
	}

	if(pstHData == NULL) {
		if((pstSIGNAL->ucAcctType == ACCOUNTING_START) && (isCallControl > 0)) {
			if((pstHData = pCallStartInfo(data)) != NULL) {
				pstRPPILog = (LOG_RPPI *)nifo_get_value(pMEMSINFO, LOG_RPPI_DEF_NUM, pstHData->dOffset);
			}
			else {
				return -12;
			}
		} 
		else {
			return -11;
		}
	}

	switch(pstSIGNAL->ucAcctType)
	{
		case ACCOUNTING_INTERIM:
			/* INTERIM에서 CDR 추출 해야 함 */
			/* cdr이 있는 경우 서비스 사용 세팅 */
			log_print(LOGN_INFO, "ACCOUNTING INTERIM MSG BYPASS");

			log_print(LOGN_INFO, "ACC INTERIM IMSI[%s] BID[%d] INOCTETS[%u] OUTOCTETS[%u]",
				pstSIGNAL->szIMSI, pstSIGNAL->ucBranchID, pstSIGNAL->uiAcctInOctets, pstSIGNAL->uiAcctOutOctets);
			if((pstRPPILog->usCallType == INIT_ROAM_CALLSTART) && (pstSIGNAL->uiAcctInOctets > 0 || pstSIGNAL->uiAcctOutOctets > 0)) {
				vCheckFirst(pstRPPIHash, pstRPPILog, "ACC-INTERIM", pstSIGNAL->uiSessStartTime, pstSIGNAL->uiSessStartMTime);
			}
			dUpdateCommonLog((LOG_COMMON *)pstSIGNAL, pstRPPILog);	
			return 0;
		case ACCOUNTING_START:
			if(!isCallControl)
			{
				if(pstRPPILog->uiLastUpdateReqTime == 0)
				{
					pstRPPILog->uiLastUpdateReqTime = pstSIGNAL->uiSessStartTime;
					pstRPPILog->uiLastUpdateReqMTime = pstSIGNAL->uiSessStartMTime;
					pstRPPILog->uiLastUpdateResTime = pstSIGNAL->uiSessEndTime;
					pstRPPILog->uiLastUpdateResMTime = pstSIGNAL->uiSessEndMTime;
				}

				pstRPPILog->usCallState = dGetCallState(pstRPPILog->usCallState, ROAM_LNS_ACCOUNT_STATE);
//				pstRPPILog->usCallType = INIT_ROAM_CALLSTART_LNS;
			}
			else
			{
				pstRPPILog->usCallState = dGetCallState(pstRPPILog->usCallState, ROAM_ACCOUNT_STATE);

				/* 
				 *	continue 상태에 대한 처리가 필요함.
				 *	timer 변경 및 상태 변경
				 */
				if(pstHData->uiTimerStop == 1)
				{
					pstRPPILog->uiReleaseTime = 0;
					pstRPPILog->uiReleaseMTime = 0;
					pstRPPILog->stopFlag = 0;
					pstRPPILog->stateFlag = 0;
					pstHData->uiTimerStop = 0;
					pstHData->timerNID = timerN_update(pTIMERNINFO, pstHData->timerNID, time(NULL) + flt_info->stTimerInfo.usTimerInfo[PI_CALL_TIMEOUT] + RPPI_GAP_TIME);
				}

				if(pstRPPILog->uiAccStartTime == 0)
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

			if(pstRPPILog->uiClientIP == 0 && pstSIGNAL->uiClientIP != 0) {
				pstRPPILog->uiClientIP = pstSIGNAL->uiClientIP;
			}

			break;
		case ACCOUNTING_STOP:
			if(isCallControl)
			{
				log_print(LOGN_DEBUG, "@@@ RCV CALLSTOP IMSI[%s] IP[%s] TIME[%s.%u]", pstSIGNAL->szIMSI,
						util_cvtipaddr(szIP, pstSIGNAL->uiClientIP), util_printtime(pstSIGNAL->uiCallTime, szTime), pstSIGNAL->uiCallMTime);

				log_print(LOGN_INFO, "ACC STOP IMSI[%s] BID[%d] INOCTETS[%u] OUTOCTETS[%u] STOPFLAG[%d]",
						pstSIGNAL->szIMSI, pstSIGNAL->ucBranchID, pstSIGNAL->uiAcctInOctets, 
						pstSIGNAL->uiAcctOutOctets, pstSIGNAL->ucStopFlag);
				if((pstRPPILog->usCallType == INIT_ROAM_CALLSTART) && (pstSIGNAL->uiAcctInOctets > 0 || pstSIGNAL->uiAcctOutOctets > 0)) {
					vCheckFirst(pstRPPIHash, pstRPPILog, "ACC-STOP", pstSIGNAL->uiSessStartTime, pstSIGNAL->uiSessStartMTime);
				}

				if(pstSIGNAL->ucStopFlag == 0) {
					uiTimer = flt_info->stTimerInfo.usTimerInfo[PI_WAIT_TIMEOUT] + RPPI_GAP_TIME;
				}
				else {
					uiTimer = flt_info->stTimerInfo.usTimerInfo[PI_DORM_TIMEOUT] + RPPI_GAP_TIME;
				}

				pstRPPILog = (LOG_RPPI *)nifo_get_value(pMEMSINFO, LOG_RPPI_DEF_NUM, pstHData->dOffset);
				pstHData->timerNID = timerN_update(pTIMERNINFO, pstHData->timerNID, time(NULL) + uiTimer);
				pstHData->uiTimerStop = 1;

				pstRPPILog->uiReleaseTime = pstSIGNAL->uiSessEndTime;
				pstRPPILog->uiReleaseMTime = pstSIGNAL->uiSessEndMTime;

				pstRPPILog->stopFlag = pstSIGNAL->uiRespCode;
				pstRPPILog->stateFlag = ROAM_RP_END_STATE;
//				dProcCallStop(pstRPPIHash, pstRPPILog, pstSIGNAL->uiRespCode);	

				if(pstRPPILog->uiFirstAccStopReqTime == 0)
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
			}
			break;
		default:
			break;
	}

	dUpdateCommonLog((LOG_COMMON *)pstSIGNAL, pstRPPILog);	

	if(!isCallControl) {
		pstRPPILog->usActiveStopCnt++;
	}
	else {
		pstRPPILog->usUpdateReqCnt++;
		pstRPPILog->usAccountReqCnt++;
		if(pstSIGNAL->uiLastUserErrCode == 0) pstRPPILog->usUpdateAckCnt++;
	}

	dErrorFlag = 0;
	dSvcThreshold = dGetThreshold(SERVICE_RADIUS, ALARM_RESPONSETIME);
	if(pstSIGNAL->uiLastUserErrCode > 0) {
		pstRPPILog->uiLastFailReason = (AAA_DEFECT + AAA_ACCOUNTING_DEFECT + pstSIGNAL->uiLastUserErrCode);
		pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(pstRPPILog->usCallState, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
		dErrorFlag = 1;
	}
	if(dSvcThreshold > 0 && pstSIGNAL->uiSessDuration > dSvcThreshold*1000) {
		pstRPPILog->uiLastFailReason = (AAA_DEFECT + SERVICE_DELAY_DEFECT + RESPONSETIME);
		pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(pstRPPILog->usCallState, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
		dErrorFlag = 1;
	}   
	if(dErrorFlag == 0) 
	{
		if(!isCallControl) {
			pstRPPILog->usDiaSuccRepCnt++;
		}
		else {
			pstRPPILog->usAccountSuccRepCnt++;
		}
	}
	else 
	{
		log_print(LOGN_INFO, "RADIUS ACCOUNT ERRCODE[%d] LastFailReason[%u]", 
				pstSIGNAL->uiLastUserErrCode, pstRPPILog->uiLastFailReason);
		pstErrLog = dCreateErrLog(pstRPPILog);
		if(pstErrLog != NULL)
		{
			pstErrLog->uiSessStartTime = pstSIGNAL->uiSessStartTime;
			pstErrLog->uiSessStartMTime = pstSIGNAL->uiSessStartMTime;
			pstErrLog->usProtoType = RADIUS_PROTO;
			pstErrLog->uiErrorCode = pstRPPILog->uiLastFailReason;
		}
	} 
	pstRPPILog->usAccountCode = pstSIGNAL->uiLastUserErrCode;

	pstRPPILog->llDiaSuccSumTime += pstSIGNAL->uiSessDuration;


	/* SEND WATCH*/
	memset(&stWatchMsg, 0x00, DEF_WATCHMSG_SIZE);

	stWatchMsg.usMsgType = WATCH_TYPE_AAA;
	stWatchMsg.ucOffice = 0;
	stWatchMsg.ucSYSID = 0;
	stWatchMsg.ucRoamFlag = 1;
	stWatchMsg.ucBSCID = 0;
	stWatchMsg.usBTSID = 0;
	stWatchMsg.ucSec = 0;
	stWatchMsg.ucFA = 0;
	stWatchMsg.uiPCFIP = 0;
	stWatchMsg.ucPCFType = 0;
	stWatchMsg.uiPDSNIP = pstRPPILog->uiNASName;
	stWatchMsg.uiAAAIP = pstRPPILog->uiAAAIP;
	stWatchMsg.uiHSSIP = 0;
	stWatchMsg.uiLNSIP = pstRPPILog->uiPCFIP;
	stWatchMsg.ucSvcIdx = 0;
	stWatchMsg.usSvcL4Type = 0;
	stWatchMsg.uiSVCIP = 0;
	stWatchMsg.uiResult = (dErrorFlag == 1) ? pstRPPILog->uiLastFailReason : 0;

	dSendMonInfo(&stWatchMsg);
	log_print(LOGN_INFO, "SEND WATCH MSG[%d][%s] OFFICE[%d][%s] RESULT[%d]",
			stWatchMsg.usMsgType, PrintMonType(stWatchMsg.usMsgType),
			stWatchMsg.ucOffice, PrintOFFICE(stWatchMsg.ucOffice), stWatchMsg.uiResult);

	return 0;
}

S32 dLcpInfo(U8 *data)
{
	RPPISESS_KEY    stRPPIKey, *pstRPPIKey;
	HData_RPPI      stRPPIHash, *pstRPPIHash;
	LOG_SIGNAL      *pstSIGNAL;
	stHASHONODE     *pHASHONODE;
	LOG_RPPI        *pstRPPILog;
	HData			*pstHData;

	pstRPPIKey = &stRPPIKey; 
	pstRPPIHash = &stRPPIHash;

	U8              szIP[INET_ADDRSTRLEN];
	U8              szPCFIP[INET_ADDRSTRLEN];
	U8              szPDSNIP[INET_ADDRSTRLEN];
	U8              szTime[BUFSIZ];	

	pstSIGNAL = (LOG_SIGNAL *)data;
	dCvtIRM(pstSIGNAL->szIMSI);

	log_print(LOGN_DEBUG, "### RCV LCP IMSI[%s] PROTOTYPE[%d] MSGTYPE[%d] [%s]",
			pstSIGNAL->szIMSI, pstSIGNAL->uiProtoType, pstSIGNAL->uiMsgType, PrintMsg(pstSIGNAL));

	memset(pstRPPIKey, 0x00, MAX_MIN_SIZE); 
	memcpy(pstRPPIKey->szIMSI, pstSIGNAL->szIMSI, MAX_MIN_SIZE);

	if((pHASHONODE = hasho_find(pHASHOINFO, (U8*)pstRPPIKey)))
	{
		pstRPPIHash = (HData_RPPI *)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);

		pstHData = pFindCallWithTime(pstRPPIHash, pstSIGNAL->uiCallTime, pstSIGNAL->uiCallMTime);
		if(pstHData == NULL)
		{
			log_print(LOGN_WARN, "[%s] NOT FOUND RPPI LOG IMSI[%s] IP[%s] TIME[%s.%u] PDSNIP[%s]",
					PrintMsg(pstSIGNAL), pstSIGNAL->szIMSI, util_cvtipaddr(szIP, pstSIGNAL->uiClientIP),
					util_printtime(pstSIGNAL->uiCallTime, szTime), pstSIGNAL->uiCallMTime, util_cvtipaddr(szPDSNIP, pstSIGNAL->uiNASName));
			return -1;
		}	
		if(pstHData->uiTimerStop == 0) {
			pstHData->timerNID = timerN_update(pTIMERNINFO, pstHData->timerNID, time(NULL) + flt_info->stTimerInfo.usTimerInfo[PI_CALL_TIMEOUT] + RPPI_GAP_TIME);
		}
		pstRPPILog = (LOG_RPPI *)nifo_get_value(pMEMSINFO, LOG_RPPI_DEF_NUM, pstHData->dOffset);
	}
	else
	{
		log_print(LOGN_WARN, "NOT FOUND [%s:%u:%u] IMSI[%s] IP[%s] ERROR[%d] TIME[%s.%u] PCFIP[%s]", 
			PrintMsg(pstSIGNAL), pstSIGNAL->uiProtoType, pstSIGNAL->uiMsgType, pstSIGNAL->szIMSI, 
			util_cvtipaddr(szIP, pstSIGNAL->uiClientIP), pstSIGNAL->ucErrorCode, 
			util_printtime(pstSIGNAL->uiCallTime, szTime), pstSIGNAL->uiCallMTime,
			util_cvtipaddr(szPCFIP, pstSIGNAL->uiPCFIP));
		return -2;
	}

	dUpdateCommonLog((LOG_COMMON *)pstSIGNAL, pstRPPILog);	

	switch(pstSIGNAL->uiProtoType)
	{
		case UPLCP_PROTO:
		case DNLCP_PROTO:
			switch (pstSIGNAL->uiMsgType)
			{
				case PPP_TERM:
					pstRPPILog->usCallState = dGetCallState(pstRPPILog->usCallState, ROAM_PPP_TERM_STATE);
					break;
				case LCP_ECHO:
				case PPP_SETUP:
				default:
					log_print(LOGN_WARN, "[%s][%s.%d] NOT DEFINE PROTOTYPE[%d] MSGTYPE[%d] ", __FILE__, __FUNCTION__, __LINE__,
							pstSIGNAL->uiProtoType, pstSIGNAL->uiMsgType);
					break;
			}
			break; 
		default:
			log_print(LOGN_WARN, "[%s][%s.%d] NOT DEFINE PROTOTYPE[%d] MSGTYPE[%d] ", __FILE__, __FUNCTION__, __LINE__,
					pstSIGNAL->uiProtoType, pstSIGNAL->uiMsgType);
			break;
	}

	return 0;
}

S32 dIpcpInfo(U8 *data)
{
	RPPISESS_KEY    stRPPIKey, *pstRPPIKey;
	HData_RPPI      stRPPIHash, *pstRPPIHash;
	LOG_SIGNAL      *pstSIGNAL;
	stHASHONODE     *pHASHONODE;
	LOG_RPPI        *pstRPPILog;
	HData			*pstHData;

	pstRPPIKey = &stRPPIKey; 
	pstRPPIHash = &stRPPIHash;

	U8              szIP[INET_ADDRSTRLEN];
	U8              szPCFIP[INET_ADDRSTRLEN];
	U8              szPDSNIP[INET_ADDRSTRLEN];
	U8              szTime[BUFSIZ];	

	pstSIGNAL = (LOG_SIGNAL *)data;
	dCvtIRM(pstSIGNAL->szIMSI);

	log_print(LOGN_DEBUG, "### RCV IPCP IMSI[%s] PROTOTYPE[%d] MSGTYPE[%d] [%s]",
			pstSIGNAL->szIMSI, pstSIGNAL->uiProtoType, pstSIGNAL->uiMsgType, PrintMsg(pstSIGNAL));

	memset(pstRPPIKey, 0x00, MAX_MIN_SIZE); 
	memcpy(pstRPPIKey->szIMSI, pstSIGNAL->szIMSI, MAX_MIN_SIZE);

	if((pHASHONODE = hasho_find(pHASHOINFO, (U8*)pstRPPIKey)))
	{
		pstRPPIHash = (HData_RPPI*)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);

		pstHData = pFindCallWithTime(pstRPPIHash, pstSIGNAL->uiCallTime, pstSIGNAL->uiCallMTime);
		if(pstHData == NULL)
		{
			log_print(LOGN_WARN, "[%s] NOT FOUND RPPI LOG IMSI[%s] IP[%s] TIME[%s.%u] PDSNIP[%s]",
					PrintMsg(pstSIGNAL), pstSIGNAL->szIMSI, util_cvtipaddr(szIP, pstSIGNAL->uiClientIP),
					util_printtime(pstSIGNAL->uiCallTime, szTime), pstSIGNAL->uiCallMTime, util_cvtipaddr(szPDSNIP, pstSIGNAL->uiNASName));
			return -1;
		}	
		if(pstHData->uiTimerStop == 0) {
			pstHData->timerNID = timerN_update(pTIMERNINFO, pstHData->timerNID, time(NULL) + flt_info->stTimerInfo.usTimerInfo[PI_CALL_TIMEOUT] + RPPI_GAP_TIME);
		}
		pstRPPILog = (LOG_RPPI *)nifo_get_value(pMEMSINFO, LOG_RPPI_DEF_NUM, pstHData->dOffset);
	}
	else
	{
		log_print(LOGN_WARN, "NOT FOUND [%s:%u:%u] IMSI[%s] IP[%s] ERROR[%d] TIME[%s.%u] PCFIP[%s]", 
			PrintMsg(pstSIGNAL), pstSIGNAL->uiProtoType, pstSIGNAL->uiMsgType, pstSIGNAL->szIMSI, 
			util_cvtipaddr(szIP, pstSIGNAL->uiClientIP), pstSIGNAL->ucErrorCode, 
			util_printtime(pstSIGNAL->uiCallTime, szTime), pstSIGNAL->uiCallMTime,
			util_cvtipaddr(szPCFIP, pstSIGNAL->uiPCFIP));
		return -2;
	}

	dUpdateCommonLog((LOG_COMMON *)pstSIGNAL, pstRPPILog);	

	switch(pstSIGNAL->uiProtoType)
	{
		case UPIPCP_PROTO:
			switch (pstSIGNAL->uiMsgType)
			{
			case PPP_SETUP:
				pstRPPILog->usUpIPCPReqCnt += pstSIGNAL->ucPPPReqCnt;
				if(pstRPPILog->uiUpIPCPStartTime == 0) {
					pstRPPILog->uiUpIPCPStartTime = pstSIGNAL->uiSessStartTime;
					pstRPPILog->uiUpIPCPStartMTime = pstSIGNAL->uiSessStartMTime;
				}

				if(pstRPPILog->uiUpIPCPEndTime > 0) pstRPPILog->usUpIPCPRetrans = 1;
				pstRPPILog->uiUpIPCPEndTime = pstSIGNAL->uiSessEndTime;
				pstRPPILog->uiUpIPCPEndMTime = pstSIGNAL->uiSessEndMTime;

/*
				if(pstRPPILog->usCallType == REACT_CALLSTART)
					pstRPPILog->usCallType = REACT_CALLSTART_PPP;
*/
				pstRPPILog->usCallState = dGetCallState(pstRPPILog->usCallState, ROAM_IPCP_SETUP_STATE);

				if((pstRPPILog->uiUpIPCPEndTime > 0) && (pstRPPILog->uiDnIPCPEndTime > 0)) {
					vGetIPCPDuration(pstRPPILog);
					pstRPPILog->uiPPPSetupTime = pstSIGNAL->uiSessEndTime;
					pstRPPILog->uiPPPSetupMTime = pstSIGNAL->uiSessEndMTime;
					STG_DiffTIME64(pstSIGNAL->uiSessEndTime, pstSIGNAL->uiSessEndMTime, pstRPPILog->uiCallTime, pstRPPILog->uiCallMTime, &pstRPPILog->llPPPSetupDelTime);	
				} 
				break;
			case PPP_TERM:
				pstRPPILog->usCallState = dGetCallState(pstRPPILog->usCallState, ROAM_PPP_TERM_STATE);
				break;
			default:
				log_print(LOGN_WARN, "[%s][%s.%d] NOT DEFINE PROTOTYPE[%d] MSGTYPE[%d] ", __FILE__, __FUNCTION__, __LINE__,
						pstSIGNAL->uiProtoType, pstSIGNAL->uiMsgType);
				break;
			}
			break;
		case DNIPCP_PROTO:
			switch (pstSIGNAL->uiMsgType)
			{
				case PPP_SETUP:
					pstRPPILog->usDnIPCPReqCnt += pstSIGNAL->ucPPPReqCnt;
					if(pstRPPILog->uiDnIPCPStartTime == 0) {   
						pstRPPILog->uiDnIPCPStartTime = pstSIGNAL->uiSessStartTime;
						pstRPPILog->uiDnIPCPStartMTime = pstSIGNAL->uiSessStartMTime;
					}
					if(pstRPPILog->uiDnIPCPEndTime > 0) pstRPPILog->usDnIPCPRetrans = 1;
					pstRPPILog->uiDnIPCPEndTime = pstSIGNAL->uiSessEndTime;
					pstRPPILog->uiDnIPCPEndMTime = pstSIGNAL->uiSessEndMTime;
/*
					if(pstRPPILog->usCallType == REACT_CALLSTART)
						pstRPPILog->usCallType = REACT_CALLSTART_PPP;
*/
					pstRPPILog->usCallState = dGetCallState(pstRPPILog->usCallState, ROAM_IPCP_SETUP_STATE);

					if((pstRPPILog->uiUpIPCPEndTime > 0) && (pstRPPILog->uiDnIPCPEndTime > 0)) {
						vGetIPCPDuration(pstRPPILog);
						pstRPPILog->uiPPPSetupTime = pstSIGNAL->uiSessEndTime;
						pstRPPILog->uiPPPSetupMTime = pstSIGNAL->uiSessEndMTime;
						STG_DiffTIME64(pstSIGNAL->uiSessEndTime, pstSIGNAL->uiSessEndMTime, pstRPPILog->uiCallTime, pstRPPILog->uiCallMTime, &pstRPPILog->llPPPSetupDelTime);	
					} 
					break;
				case PPP_TERM:
					pstRPPILog->usCallState = dGetCallState(pstRPPILog->usCallState, ROAM_PPP_TERM_STATE);
					break;
				default:
					log_print(LOGN_WARN, "[%s][%s.%d] NOT DEFINE PROTOTYPE[%d] MSGTYPE[%d] ", __FILE__, __FUNCTION__, __LINE__,
							pstSIGNAL->uiProtoType, pstSIGNAL->uiMsgType);
					break;
			}
			break;  
		default:
			log_print(LOGN_WARN, "[%s][%s.%d] NOT DEFINE PROTOTYPE[%d] MSGTYPE[%d] ", __FILE__, __FUNCTION__, __LINE__,
					pstSIGNAL->uiProtoType, pstSIGNAL->uiMsgType);
			break;
	}

	return 0;
}

S32 dAuthInfo(U8 *data)
{
	RPPISESS_KEY    stRPPIKey, *pstRPPIKey;
	HData_RPPI      stRPPIHash, *pstRPPIHash;
	LOG_SIGNAL      *pstSIGNAL;
	stHASHONODE     *pHASHONODE;
	LOG_RPPI        *pstRPPILog;
	HData			*pstHData;

	pstRPPIKey = &stRPPIKey; 
	pstRPPIHash = &stRPPIHash;

	U8              szIP[INET_ADDRSTRLEN];
	U8              szPCFIP[INET_ADDRSTRLEN];
	U8              szPDSNIP[INET_ADDRSTRLEN];
	U8              szTime[BUFSIZ];	

	pstSIGNAL = (LOG_SIGNAL *)data;
	dCvtIRM(pstSIGNAL->szIMSI);

	log_print(LOGN_DEBUG, "### RCV AUTH IMSI[%s] PROTOTYPE[%d] MSGTYPE[%d] [%s]",
			pstSIGNAL->szIMSI, pstSIGNAL->uiProtoType, pstSIGNAL->uiMsgType, PrintMsg(pstSIGNAL));

	memset(pstRPPIKey, 0x00, MAX_MIN_SIZE); 
	memcpy(pstRPPIKey->szIMSI, pstSIGNAL->szIMSI, MAX_MIN_SIZE);

	if((pHASHONODE = hasho_find(pHASHOINFO, (U8*)pstRPPIKey)))
	{
		pstRPPIHash = (HData_RPPI *)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);

		pstHData = pFindCallWithTime(pstRPPIHash, pstSIGNAL->uiCallTime, pstSIGNAL->uiCallMTime);
		if(pstHData == NULL)
		{
			log_print(LOGN_WARN, "[%s] NOT FOUND RPPI LOG IMSI[%s] IP[%s] TIME[%s.%u] PDSNIP[%s]",
					PrintMsg(pstSIGNAL), pstSIGNAL->szIMSI, util_cvtipaddr(szIP, pstSIGNAL->uiClientIP),
					util_printtime(pstSIGNAL->uiCallTime, szTime), pstSIGNAL->uiCallMTime, util_cvtipaddr(szPDSNIP, pstSIGNAL->uiNASName));
			return -1;
		}	
		if(pstHData->uiTimerStop == 0) {
			pstHData->timerNID = timerN_update(pTIMERNINFO, pstHData->timerNID, time(NULL) + flt_info->stTimerInfo.usTimerInfo[PI_CALL_TIMEOUT] + RPPI_GAP_TIME);
		}
		pstRPPILog = (LOG_RPPI *)nifo_get_value(pMEMSINFO, LOG_RPPI_DEF_NUM, pstHData->dOffset);
	}
	else
	{
		log_print(LOGN_WARN, "NOT FOUND [%s:%u:%u] IMSI[%s] IP[%s] ERROR[%d] TIME[%s.%u] PCFIP[%s]", 
			PrintMsg(pstSIGNAL), pstSIGNAL->uiProtoType, pstSIGNAL->uiMsgType, pstSIGNAL->szIMSI, 
			util_cvtipaddr(szIP, pstSIGNAL->uiClientIP), pstSIGNAL->ucErrorCode, 
			util_printtime(pstSIGNAL->uiCallTime, szTime), pstSIGNAL->uiCallMTime,
			util_cvtipaddr(szPCFIP, pstSIGNAL->uiPCFIP));
		return -2;
	}

	dUpdateCommonLog((LOG_COMMON *)pstSIGNAL, pstRPPILog);	

	pstRPPILog->usAuthMethod = pstSIGNAL->uiProtoType;

	pstRPPILog->uiAuthEndTime = pstSIGNAL->uiSessEndTime;
	pstRPPILog->uiAuthEndMTime = pstSIGNAL->uiSessEndMTime;
	pstRPPILog->uiAuthResTime = pstSIGNAL->uiPPPResponseTime;
	pstRPPILog->uiAuthResMTime = pstSIGNAL->uiPPPResponseMTime;
	STG_DiffTIME64(pstRPPILog->uiAuthEndTime, pstRPPILog->uiAuthEndMTime, pstRPPILog->uiAuthReqTime, pstRPPILog->uiAuthReqMTime, &pstRPPILog->llAuthDuration);

	pstRPPILog->usAuthReqCnt = pstSIGNAL->ucPPPReqCnt;
	pstRPPILog->usAuthResultCode = pstSIGNAL->ucErrorCode;
	pstRPPILog->uiCHAPRespCode = pstSIGNAL->uiRespCode;

	pstRPPILog->usCallState = dGetCallState(pstRPPILog->usCallState, ROAM_AUTH_SETUP_STATE);

	return 0;
}

S32 dOtherPPPInfo(U8 *data)
{
	RPPISESS_KEY    stRPPIKey, *pstRPPIKey;
	HData_RPPI      stRPPIHash, *pstRPPIHash;
	LOG_SIGNAL      *pstSIGNAL;
	stHASHONODE     *pHASHONODE;
	LOG_RPPI        *pstRPPILog;
	HData			*pstHData;

	pstRPPIKey = &stRPPIKey; 
	pstRPPIHash = &stRPPIHash;

	U8              szIP[INET_ADDRSTRLEN];
	U8              szPCFIP[INET_ADDRSTRLEN];
	U8              szPDSNIP[INET_ADDRSTRLEN];
	U8              szTime[BUFSIZ];	

	pstSIGNAL = (LOG_SIGNAL *)data;
	dCvtIRM(pstSIGNAL->szIMSI);

	log_print(LOGN_DEBUG, "### RCV OTHER PPP IMSI[%s] PROTOTYPE[%d] MSGTYPE[%d] [%s]",
			pstSIGNAL->szIMSI, pstSIGNAL->uiProtoType, pstSIGNAL->uiMsgType, PrintMsg(pstSIGNAL));

	memset(pstRPPIKey, 0x00, MAX_MIN_SIZE); 
	memcpy(pstRPPIKey->szIMSI, pstSIGNAL->szIMSI, MAX_MIN_SIZE);

	if((pHASHONODE = hasho_find(pHASHOINFO, (U8*)pstRPPIKey)))
	{
		pstRPPIHash = (HData_RPPI *)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);

		pstHData = pFindCallWithTime(pstRPPIHash, pstSIGNAL->uiCallTime, pstSIGNAL->uiCallMTime);
		if(pstHData == NULL)
		{
			log_print(LOGN_WARN, "[%s] NOT FOUND RPPI LOG IMSI[%s] IP[%s] TIME[%s.%u] PDSNIP[%s]",
					PrintMsg(pstSIGNAL), pstSIGNAL->szIMSI, util_cvtipaddr(szIP, pstSIGNAL->uiClientIP),
					util_printtime(pstSIGNAL->uiCallTime, szTime), pstSIGNAL->uiCallMTime, util_cvtipaddr(szPDSNIP, pstSIGNAL->uiNASName));
			return -1;
		}	
		if(pstHData->uiTimerStop == 0) {
			pstHData->timerNID = timerN_update(pTIMERNINFO, pstHData->timerNID, time(NULL) + flt_info->stTimerInfo.usTimerInfo[PI_CALL_TIMEOUT] + RPPI_GAP_TIME);
		}
		pstRPPILog = (LOG_RPPI *)nifo_get_value(pMEMSINFO, LOG_RPPI_DEF_NUM, pstHData->dOffset);
	}
	else
	{
		log_print(LOGN_WARN, "NOT FOUND [%s:%u:%u] IMSI[%s] IP[%s] ERROR[%d] TIME[%s.%u] PCFIP[%s]", 
			PrintMsg(pstSIGNAL), pstSIGNAL->uiProtoType, pstSIGNAL->uiMsgType, pstSIGNAL->szIMSI, 
			util_cvtipaddr(szIP, pstSIGNAL->uiClientIP), pstSIGNAL->ucErrorCode, 
			util_printtime(pstSIGNAL->uiCallTime, szTime), pstSIGNAL->uiCallMTime,
			util_cvtipaddr(szPCFIP, pstSIGNAL->uiPCFIP));
		return -2;
	}

	dUpdateCommonLog((LOG_COMMON *)pstSIGNAL, pstRPPILog);	

	return 0;
}

S32 dL2TPInfo(U8 *data)
{
	RPPISESS_KEY    stRPPIKey, *pstRPPIKey;
	HData_RPPI      stRPPIHash, *pstRPPIHash;
	LOG_SIGNAL      *pstSIGNAL;
	LOG_RPPI_ERR	*pstErrLog;
	stHASHONODE     *pHASHONODE;
	LOG_RPPI        *pstRPPILog;
	HData			*pstHData;

	pstRPPIKey = &stRPPIKey; 
	pstRPPIHash = &stRPPIHash;

	U8              szIP[INET_ADDRSTRLEN];
	U8              szPCFIP[INET_ADDRSTRLEN];
	U8              szPDSNIP[INET_ADDRSTRLEN];
	U8              szTime[BUFSIZ];	

	pstSIGNAL = (LOG_SIGNAL *)data;
	dCvtIRM(pstSIGNAL->szIMSI);

	log_print(LOGN_DEBUG, "### RCV L2TP IMSI[%s] PROTOTYPE[%d] MSGTYPE[%d] [%s]",
			pstSIGNAL->szIMSI, pstSIGNAL->uiProtoType, pstSIGNAL->uiMsgType, PrintMsg(pstSIGNAL));

	memset(pstRPPIKey, 0x00, MAX_MIN_SIZE); 
	memcpy(pstRPPIKey->szIMSI, pstSIGNAL->szIMSI, MAX_MIN_SIZE);

	if((pHASHONODE = hasho_find(pHASHOINFO, (U8*)pstRPPIKey)))
	{
		pstRPPIHash = (HData_RPPI *)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);

		pstHData = pFindCallWithTime(pstRPPIHash, pstSIGNAL->uiCallTime, pstSIGNAL->uiCallMTime);
		if(pstHData == NULL)
		{
			log_print(LOGN_WARN, "[%s] NOT FOUND RPPI LOG IMSI[%s] IP[%s] TIME[%s.%u] PDSNIP[%s]",
					PrintMsg(pstSIGNAL), pstSIGNAL->szIMSI, util_cvtipaddr(szIP, pstSIGNAL->uiClientIP),
					util_printtime(pstSIGNAL->uiCallTime, szTime), pstSIGNAL->uiCallMTime, util_cvtipaddr(szPDSNIP, pstSIGNAL->uiNASName));
			return -1;
		}	
		if(pstHData->uiTimerStop == 0) {
			pstHData->timerNID = timerN_update(pTIMERNINFO, pstHData->timerNID, time(NULL) + flt_info->stTimerInfo.usTimerInfo[PI_CALL_TIMEOUT] + RPPI_GAP_TIME);
		}
		pstRPPILog = (LOG_RPPI *)nifo_get_value(pMEMSINFO, LOG_RPPI_DEF_NUM, pstHData->dOffset);
	}
	else
	{
		log_print(LOGN_WARN, "NOT FOUND [%s:%u:%u] IMSI[%s] IP[%s] ERROR[%d] TIME[%s.%u] PCFIP[%s]", 
			PrintMsg(pstSIGNAL), pstSIGNAL->uiProtoType, pstSIGNAL->uiMsgType, pstSIGNAL->szIMSI, 
			util_cvtipaddr(szIP, pstSIGNAL->uiClientIP), pstSIGNAL->ucErrorCode, 
			util_printtime(pstSIGNAL->uiCallTime, szTime), pstSIGNAL->uiCallMTime,
			util_cvtipaddr(szPCFIP, pstSIGNAL->uiPCFIP));
		return -2;
	}

	dUpdateCommonLog((LOG_COMMON *)pstSIGNAL, pstRPPILog);	

	pstRPPILog->uiUpGREPkts = pstSIGNAL->uiUpL2TPPkts;
	pstRPPILog->uiDnGREPkts = pstSIGNAL->uiDnL2TPPkts;
	pstRPPILog->uiUpGREBytes = pstSIGNAL->uiUpL2TPBytes;
	pstRPPILog->uiDnGREBytes = pstSIGNAL->uiDnL2TPBytes;

	switch(pstSIGNAL->uiMsgType)
	{
		case MSG_L2TP_TUNNEL_START:
		case MSG_L2TP_TUNNEL_STOP:
		case MSG_L2TP_TUNNEL_INTERIM:
			break;
		case MSG_L2TP_CALL_STOP:
			if(pstSIGNAL->usResultCode > 0 && pstSIGNAL->usErrorCode > 0) {
				pstRPPILog->uiLastFailReason = (L2TP_DEFECT + L2TP_CDN_DEFECT + pstSIGNAL->usResultCode);

				pstErrLog = dCreateErrLog(pstRPPILog);
				if(pstErrLog != NULL)
				{
					pstErrLog->uiSessStartTime = pstSIGNAL->uiSessStartTime;
					pstErrLog->uiSessStartMTime = pstSIGNAL->uiSessStartMTime;
					pstErrLog->usProtoType = DEF_PROTOCOL_L2TP;
					pstErrLog->uiErrorCode = pstRPPILog->uiLastFailReason;
				}
			}
			break;
		case MSG_L2TP_CALL_START:
			pstRPPILog->usUpLCPReqCnt++;

			pstRPPILog->uiUpLCPStartTime = pstSIGNAL->uiL2TPReqTime;
			pstRPPILog->uiUpLCPStartMTime = pstSIGNAL->uiL2TPReqMTime;
	
			pstRPPILog->uiUpLCPEndTime = pstSIGNAL->uiL2TPRepTime;
			pstRPPILog->uiUpLCPEndMTime = pstSIGNAL->uiL2TPRepMTime;

			pstRPPILog->uiDnLCPStartTime = pstSIGNAL->uiL2TPConTime;
			pstRPPILog->uiDnLCPStartMTime = pstSIGNAL->uiL2TPConMTime;

			pstRPPILog->uiDnLCPEndTime = pstSIGNAL->uiSessStartTime;
			pstRPPILog->uiDnLCPEndMTime = pstSIGNAL->uiSessStartMTime;

			pstRPPILog->llLCPDuration = pstSIGNAL->uiSessDuration;

			pstRPPILog->usCallState = dGetCallState(pstRPPILog->usCallState, ROAM_L2TP_SETUP_STATE);

			/* LNS IP */
			pstRPPILog->uiPCFIP = pstSIGNAL->uiDestIP;
//			pstRPPILog->usCallType = INIT_ROAM_CALLSTART_LNS;

			pstRPPILog->uiGREKey = pstSIGNAL->usReqSessID;
			pstRPPILog->usSvcOption = pstSIGNAL->usRepSessID;
			pstRPPILog->usFMux = pstSIGNAL->usReqTunnelID;
			pstRPPILog->usRMux = pstSIGNAL->usRepTunnelID;

			pstRPPILog->uiAuthReqTime = pstSIGNAL->uiL2TPConTime;
			pstRPPILog->uiAuthReqMTime = pstSIGNAL->uiL2TPConMTime;

			if(pstSIGNAL->usResultCode > 0 && pstSIGNAL->usErrorCode > 0) {
				pstRPPILog->uiLastFailReason = (L2TP_DEFECT + L2TP_CCN_DEFECT + pstSIGNAL->usResultCode);
				pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(pstRPPILog->usCallState, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);

				pstErrLog = dCreateErrLog(pstRPPILog);
				if(pstErrLog != NULL)
				{
					pstErrLog->uiSessStartTime = pstSIGNAL->uiSessStartTime;
					pstErrLog->uiSessStartMTime = pstSIGNAL->uiSessStartMTime;
					pstErrLog->usProtoType = DEF_PROTOCOL_L2TP;
					pstErrLog->uiErrorCode = pstRPPILog->uiLastFailReason;
				}
			}
			else if(pstSIGNAL->uiLastUserErrCode > 0) {
				if(pstSIGNAL->uiL2TPRepTime > 0) {
					pstRPPILog->uiLastFailReason = (L2TP_DEFECT + CALL_SETUP + ERR_L2TP_2);
					pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(pstRPPILog->usCallState, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
				}
				else {
					pstRPPILog->uiLastFailReason = (L2TP_DEFECT + CALL_SETUP + ERR_L2TP_1);
					pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(pstRPPILog->usCallState, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
				}

				pstErrLog = dCreateErrLog(pstRPPILog);
				if(pstErrLog != NULL)
				{
					pstErrLog->uiSessStartTime = pstSIGNAL->uiSessStartTime;
					pstErrLog->uiSessStartMTime = pstSIGNAL->uiSessStartMTime;
					pstErrLog->usProtoType = DEF_PROTOCOL_L2TP;
					pstErrLog->uiErrorCode = pstRPPILog->uiLastFailReason;
				}
			}
			else {
				pstRPPILog->usDnLCPReqCnt++;
			}

			break;
		default:
			break;
	}

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

void dUpdateCommonLog(LOG_COMMON *pLOG, LOG_RPPI *pstRPPILog)
{

	/* SERVICE COMMON LOG UPDATE */
	pLOG->uiCallTime = pstRPPILog->uiCallTime;
	pLOG->uiCallMTime = pstRPPILog->uiCallMTime;
	pLOG->uiAccStartTime = pstRPPILog->uiAccStartTime;
	pLOG->uiAccStartMTime = pstRPPILog->uiAccStartMTime;
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

	log_print(LOGN_INFO, "dCreateErrLog IMSI=%s", pstRPPILog->szIMSI);

	if((pNext = nifo_node_alloc(pMEMSINFO)) == NULL)
	{
		log_print(LOGN_CRI, "[%s][%s.%d] nifo_node_alloc NULL ", __FILE__, __FUNCTION__, __LINE__);
		return NULL;
	}

	if((pstErrLog = (LOG_RPPI_ERR*) nifo_tlv_alloc(pMEMSINFO, pNext, LOG_RPPI_ERR_DEF_NUM, LOG_RPPI_ERR_SIZE, DEF_MEMSET_ON)) == NULL)
	{
		log_print(LOGN_CRI, "[%s][%s.%d] nifo_tlv_alloc NULL ", __FILE__, __FUNCTION__, __LINE__);
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

HData *pFindCallWithTime(HData_RPPI *pstRPPIHash, U32 uiAccStartTime, U32 uiAccStartMTime)
{
	S64             llBeforeGapTime, llAfterGapTime;
	U8				szSTime[BUFSIZ];
	U8				szDTime[BUFSIZ];
	U8				szKTime[BUFSIZ];
	HData			*pstHData = NULL;

	if(pstRPPIHash->after.dOffset == 0)
	{
		STG_DeltaTIME64(uiAccStartTime, uiAccStartMTime, pstRPPIHash->before.uiCallTime, pstRPPIHash->before.uiCallMTime, &llBeforeGapTime);
		if(llBeforeGapTime >= 0 || (llBeforeGapTime < 0 && labs(llBeforeGapTime) <= ALLOW_PITIME)) 
		{
			log_print(LOGN_INFO, "Find Before RPPILog CallTime[%s.%u] AccStartTime[%s.%u]",
				util_printtime(pstRPPIHash->before.uiCallTime, szSTime), pstRPPIHash->before.uiCallMTime, 
				util_printtime(uiAccStartTime, szDTime), uiAccStartMTime);

			pstHData = &pstRPPIHash->before;
		}	
		else
		{
			log_print(LOGN_WARN, "[pFindRPPILog] NOT FOUND RPPILOG Before CallTime[%s.%u] AccStartTime[%s.%u]",
					util_printtime(pstRPPIHash->before.uiCallTime, szSTime), pstRPPIHash->before.uiCallMTime, 
					util_printtime(uiAccStartTime, szDTime), uiAccStartMTime);
		}	
	}
	else
	{
		STG_DeltaTIME64(uiAccStartTime, uiAccStartMTime, pstRPPIHash->before.uiCallTime, pstRPPIHash->before.uiCallMTime, &llBeforeGapTime);
		STG_DeltaTIME64(uiAccStartTime, uiAccStartMTime, pstRPPIHash->after.uiCallTime, pstRPPIHash->after.uiCallMTime, &llAfterGapTime);
		if(pstRPPIHash->after.uiCallTime !=0 && llAfterGapTime >= 0)
		{ 
			log_print(LOGN_INFO, "Find After RPPILog CallTime[%s.%u] AccStartTime[%s.%u]",
				util_printtime(pstRPPIHash->after.uiCallTime, szSTime), pstRPPIHash->after.uiCallMTime, 
				util_printtime(uiAccStartTime, szDTime), uiAccStartMTime);

			pstHData = &pstRPPIHash->after;
		}
		else if(pstRPPIHash->before.uiCallTime !=0 && (llBeforeGapTime >= 0 || (llBeforeGapTime < 0 && labs(llBeforeGapTime) <= ALLOW_PITIME))) 
		{
			log_print(LOGN_INFO, "Find Before RPPILog CallTime[%s.%u] AccStartTime[%s.%u]",
				util_printtime(pstRPPIHash->before.uiCallTime, szSTime), pstRPPIHash->before.uiCallMTime, 
				util_printtime(uiAccStartTime, szDTime), uiAccStartMTime);

			pstHData = &pstRPPIHash->before;
		}
		else
		{
			log_print(LOGN_WARN,"[pFindRPPILog] NOT FOUND RPPILOG Before[%s.%u]After[%s.%u]Log[%s.%u]", 
					util_printtime(pstRPPIHash->before.uiCallTime, szSTime), pstRPPIHash->before.uiCallMTime,
					util_printtime(pstRPPIHash->after.uiCallTime, szDTime), pstRPPIHash->after.uiCallMTime,
					util_printtime(uiAccStartTime, szKTime), uiAccStartMTime);
		}	
	} 

	return pstHData;
}

HData *pFindCallWithCorrID(HData_RPPI *pstRPPIHash, U64 ulCorrelationID)
{
	U8				szSTime[BUFSIZ];
	U8				szDTime[BUFSIZ];
	HData			*pstHData = NULL;

	if(pstRPPIHash->after.dOffset == 0)
	{
		if(pstRPPIHash->before.ulCorrelationID == ulCorrelationID) 
		{
			log_print(LOGN_INFO, "Find Before RPPILog CallTime[%s.%u] CorrelationID[%llu]",
				util_printtime(pstRPPIHash->before.uiCallTime, szSTime), pstRPPIHash->before.uiCallMTime, ulCorrelationID);

			pstHData = &pstRPPIHash->before;
		}	
		else
		{
			log_print(LOGN_WARN, "[pFindRPPILog] NOT FOUND RPPILOG Before CallTime[%s.%u] CorrelationID[%llu] Input CorrID[%llu]",
					util_printtime(pstRPPIHash->before.uiCallTime, szSTime), pstRPPIHash->before.uiCallMTime, 
					pstRPPIHash->before.ulCorrelationID, ulCorrelationID);
		}	
	}
	else
	{
		if(pstRPPIHash->after.ulCorrelationID == ulCorrelationID)
		{ 
			log_print(LOGN_INFO, "Find After RPPILog CallTime[%s.%u] CorrelationID[%llu]",
				util_printtime(pstRPPIHash->after.uiCallTime, szSTime), pstRPPIHash->after.uiCallMTime, ulCorrelationID);

			pstHData = &pstRPPIHash->after;
		}
		else if(pstRPPIHash->before.ulCorrelationID == ulCorrelationID) 
		{
			log_print(LOGN_INFO, "Find Before RPPILog CallTime[%s.%u] CorrelationID[%llu]",
				util_printtime(pstRPPIHash->before.uiCallTime, szSTime), pstRPPIHash->before.uiCallMTime, ulCorrelationID);

			pstHData = &pstRPPIHash->before;
		}
		else
		{
			log_print(LOGN_WARN, "[pFindRPPILog] NOT FOUND RPPILOG Before[%s.%u] CorrID[%llu] After[%s.%u] CorrID[%llu] CorrelationID[%llu]", 
					util_printtime(pstRPPIHash->before.uiCallTime, szSTime), 
					pstRPPIHash->before.uiCallMTime, pstRPPIHash->before.ulCorrelationID,
					util_printtime(pstRPPIHash->after.uiCallTime, szDTime), 
					pstRPPIHash->after.uiCallMTime, pstRPPIHash->after.ulCorrelationID,
					ulCorrelationID);
		}	
	} 

	return pstHData;
}

S32 dProcCallStop(HData_RPPI *pstRPPIHash, LOG_RPPI *pstRPPILog, S32 isFirstServ)
{
//	LOG_RPPI_ERR    *pstErrLog;
	U8              szTime[BUFSIZ];
	S32				isFail = 0;

	log_print(LOGN_INFO, "dProcCallStop IMSI=%s TIME=%s.%u isFirstServ=%d CALLSTATE=%d stopFlag=%d SETUPFAIL=%u",
			pstRPPILog->szIMSI, util_printtime(pstRPPILog->uiCallTime, szTime), pstRPPILog->uiCallMTime,
			isFirstServ, pstRPPILog->usCallState, pstRPPILog->stopFlag, pstRPPILog->uiSetupFailReason);

//	if (pstRPPILog->uiUpIPCPEndTime == 0 && pstRPPILog->uiDnIPCPEndTime == 0 && isFirstServ == 0)
	if(!isFirstServ)
	{
		/** Call Setup Fail **/
		if (pstRPPILog->uiSetupFailReason == SETUP_SUCESS)
		{
			isFail = 1;
			switch(pstRPPILog->usCallState)
			{
				case ROAM_CALL_INIT_STATE:
					log_print(LOGN_CRI, "NOT START LOG STOP IMSI=%s TIME=%s.%u RCV isFirstServ=%d CALLSTATE=%d stopFlag=%d",
							pstRPPILog->szIMSI, util_printtime(pstRPPILog->uiCallTime, szTime), pstRPPILog->uiCallMTime,
							isFirstServ, pstRPPILog->usCallState, pstRPPILog->stopFlag);
				case ROAM_ACCESS_STATE:
					pstRPPILog->uiLastFailReason = (AAA_DEFECT + AAA_ACCESS_DEFECT + ERR_RADIUS_CALL_CUT);
					pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(pstRPPILog->usCallState, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
					break;
				case ROAM_ACCOUNT_STATE:
					pstRPPILog->uiLastFailReason = (AAA_DEFECT + AAA_ACCOUNTING_DEFECT + ERR_RADIUS_CALL_CUT);
					pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(pstRPPILog->usCallState, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
					break;
				case ROAM_L2TP_SETUP_STATE:
					pstRPPILog->uiLastFailReason = (L2TP_DEFECT + CALL_SETUP + uiGetL2TPFail(pstRPPILog));
					pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(pstRPPILog->usCallState, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
					break;
				case ROAM_LNS_ACCESS_STATE:
					pstRPPILog->uiLastFailReason = (AAA_DEFECT + AAA_ACCESS_DEFECT + ERR_LNS_RADIUS_CALL_CUT);
					pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(pstRPPILog->usCallState, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
					break;
				case ROAM_AUTH_SETUP_STATE:
					pstRPPILog->uiLastFailReason = (A11_DEFECT + CALL_SETUP + uiGetCHAPFail(pstRPPILog));
					pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(pstRPPILog->usCallState, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
					break;
				case ROAM_IPCP_SETUP_STATE:
					pstRPPILog->uiLastFailReason = (A11_DEFECT + CALL_SETUP + uiGetIPCPFail(pstRPPILog));         
					pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(pstRPPILog->usCallState, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
					break;
				case ROAM_LNS_ACCOUNT_STATE:
					pstRPPILog->uiLastFailReason = (AAA_DEFECT + AAA_ACCOUNTING_DEFECT + ERR_LNS_RADIUS_CALL_CUT);
					pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(pstRPPILog->usCallState, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
					break;
				case ROAM_PPP_TERM_STATE:
					if(pstRPPILog->uiLastUpdateReqTime > 0) {
						pstRPPILog->uiLastFailReason = (AAA_DEFECT + AAA_ACCOUNTING_DEFECT + ERR_LNS_RADIUS_CALL_CUT);
						pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(ROAM_LNS_ACCOUNT_STATE, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
					}
					else if((pstRPPILog->uiDnIPCPStartTime > 0) || (pstRPPILog->uiUpIPCPStartTime > 0)) {
						pstRPPILog->uiLastFailReason = (A11_DEFECT + CALL_PPP_TERM + uiGetIPCPFail(pstRPPILog));
						pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(ROAM_IPCP_SETUP_STATE, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
					}
					else if((pstRPPILog->uiAuthEndTime > 0) || (pstRPPILog->uiAuthResTime > 0)) {
						pstRPPILog->uiLastFailReason = (A11_DEFECT + CALL_PPP_TERM + uiGetCHAPFail(pstRPPILog));
						pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(ROAM_AUTH_SETUP_STATE, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
					}
					else if(pstRPPILog->uiAccessReqTime > 0) {
						pstRPPILog->uiLastFailReason = (AAA_DEFECT + AAA_ACCESS_DEFECT + ERR_LNS_RADIUS_CALL_CUT);
						pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(ROAM_LNS_ACCESS_STATE, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
					}
					/* L2TP */
					else if(pstRPPILog->uiUpLCPStartTime > 0) {
						pstRPPILog->uiLastFailReason = (L2TP_DEFECT + CALL_SETUP + uiGetL2TPFail(pstRPPILog));
						pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(ROAM_L2TP_SETUP_STATE, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
					}
					else if(pstRPPILog->uiAccStartTime > 0) {
						pstRPPILog->uiLastFailReason = (AAA_DEFECT + AAA_ACCOUNTING_DEFECT + ERR_RADIUS_CALL_CUT);
						pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(ROAM_ACCOUNT_STATE, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
					}
					else {
						pstRPPILog->uiLastFailReason = (AAA_DEFECT + AAA_ACCESS_DEFECT + ERR_RADIUS_CALL_CUT);
						pstRPPILog->uiSetupFailReason = uiGetSetupFailReason(ROAM_ACCESS_STATE, pstRPPILog->uiLastFailReason, pstRPPILog->uiSetupFailReason);
					}
					break;
				case ROAM_ON_SERVICE_STATE:
				default:
					isFail = 0;
					log_print(LOGN_CRI, "WRONG STATE IMSI=%s TIME=%s.%u RCV isFirstServ=%d CALLSTATE=%d", 
							pstRPPILog->szIMSI, util_printtime(pstRPPILog->uiCallTime, szTime), pstRPPILog->uiCallMTime,
							isFirstServ, pstRPPILog->usCallState);
					break;
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

		stWatchMsg.usMsgType = WATCH_TYPE_A11;
		stWatchMsg.ucOffice = 0;
		stWatchMsg.ucSYSID = 0;
		stWatchMsg.ucRoamFlag = 1;
		stWatchMsg.ucBSCID = 0;
		stWatchMsg.usBTSID = 0;
		stWatchMsg.ucSec = 0;
		stWatchMsg.ucFA = 0;
		stWatchMsg.uiPCFIP = 0;
		stWatchMsg.ucPCFType = 0;
		stWatchMsg.uiPDSNIP = pstRPPILog->uiNASName;
		stWatchMsg.uiAAAIP = pstRPPILog->uiAAAIP;
		stWatchMsg.uiHSSIP = 0;
		stWatchMsg.uiLNSIP = pstRPPILog->uiPCFIP;
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
				if(pstRPPILog->uiLastFailReason == 0) {
					pstRPPILog->uiLastFailReason = (A11_DEFECT + CALL_SETUP + ERR_CALL_TIMEOUT);
				}
			}
			break;
		default:
			if(pstRPPILog->stateFlag == ROAM_RP_END_STATE) {
				pstRPPILog->usCallState = dGetCallState(pstRPPILog->usCallState, ROAM_RP_END_STATE);
			}
			break;
	}

	return 0;               
}

