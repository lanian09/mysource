/**
 *	INCLUDE HEADER FILES
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <math.h>

// DQMS
#include "common_stg.h"
#include "sshmid.h"
#include "path.h"

// LIB
#include "loglib.h"
#include "utillib.h"
#include "hasho.h"

// TAM
#include "watch_filter.h"		/* st_WatchFilter .. */
#include "rppi_def.h"			/* SERVICE_TYPE .. */

// .
#include "roam_util.h"

/**
 *	DECLARE VARIABLES
 */
extern stHASHOINFO  	*pIRMINFO;
extern stHASHOINFO  	*pMODELINFO;
extern stHASHOINFO  	*pMODELINFO1;
extern stHASHOINFO  	*pMODELINFO2;
extern stHASHOINFO  	*pDEFECTINFO;
extern stHASHOINFO  	*pNASIPINFO;
extern st_WatchFilter	*gWatchFilter;

/**
 *	IMPLEMENT FUNCTIONS
 */
U32 dGetThreshold(SERVICE_TYPE SvcType, ALARM_TYPE AlarmType)
{
	stHASHONODE     *pHASHONODE;
	HData_DEFECT    *pstDefectHash;


	if ( (pHASHONODE = hasho_find(pDEFECTINFO, (U8*)&SvcType)) )
	{
		pstDefectHash = (HData_DEFECT*)nifo_ptr(pDEFECTINFO, pHASHONODE->offset_Data);

		switch(AlarmType)
		{
			case ALARM_TCPSETUPTIME:
				return pstDefectHash->uiTCPSetupTime;
			case ALARM_RESPONSETIME:
				return pstDefectHash->uiResponseTime;
			case ALARM_UPTHROUGHPUT:
				return pstDefectHash->uiUpThroughput;
			case ALARM_DNTHROUGHPUT:
				return pstDefectHash->uiDnThroughput;
			case ALARM_UPRETRANSCNT:
				return pstDefectHash->uiUpRetransCnt;
			case ALARM_DNRETRANSCNT:
				return pstDefectHash->uiDnRetransCnt;
			case ALARM_UPJITTER:
				return pstDefectHash->UpJitter; 
			case ALARM_DNJITTER:
				return pstDefectHash->DnJitter; 
			case ALARM_UPPACKETLOSS:
				return pstDefectHash->UpPacketLoss; 
			case ALARM_DNPACKETLOSS:
				return pstDefectHash->DnPacketLoss;
			default:
				return 0; 
		}
	}
	return 0;
}

S32 dGetNationID(U32 uiNASIP)
{
	stHASHONODE			*pHASHONODE;
	HData_NASIP			*pstNASIPHash;

	if((pHASHONODE = hasho_find(pNASIPINFO, (U8*)&uiNASIP)))
	{
		pstNASIPHash = (HData_NASIP *)nifo_ptr(pNASIPINFO, pHASHONODE->offset_Data);
		return pstNASIPHash->ucNationID;
	}
	return 0;
}

S32 dGetModelInfo(U8 *pIMSI, U8 *pModel)
{
	stHASHONODE     *pHASHONODE;
	HData_Model     stModelHash, *pstModelHash;

	pstModelHash = &stModelHash;

	if ( (pHASHONODE = hasho_find(pMODELINFO, (U8*)pIMSI)) )
	{
		pstModelHash = (HData_Model*)nifo_ptr(pMODELINFO, pHASHONODE->offset_Data);
		memcpy(pModel, pstModelHash->szModel, MAX_MODEL_LEN);	

		return 0;
	}
	return -1;
}

S32 dGetMINInfo(U8 *pIMSI, U8 *pMIN)
{           
	stHASHONODE     *pHASHONODE;
	HData_Model     stModelHash, *pstModelHash;

	pstModelHash = &stModelHash;

	if ( (pHASHONODE = hasho_find(pMODELINFO, (U8*)pIMSI)) )
	{           
		pstModelHash = (HData_Model*)nifo_ptr(pMODELINFO, pHASHONODE->offset_Data);
		memcpy(pMIN, pstModelHash->szMIN, MAX_MIN_LEN);

		return 0;
	}
	return -1;
}

S32 dMakeNASIPHash()
{
	S32					i, j, dLoop, dType, dCnt = 0;
	U32					uiKey, uiBaseIP;
	HData_NASIP			stNASIPHash, *pstNASIPHash;
	U8					szIP[INET_ADDRSTRLEN];	

	stHASHONODE			*pHASHONODE;
	st_LoamEquipList	*pLoamEquipList;


	pstNASIPHash = &stNASIPHash;

	/* reset hash */
	hasho_reset(pNASIPINFO);

	pLoamEquipList = &gWatchFilter->stLoamEquipList;
	log_print(LOGN_INFO, "NASIPLIST COUNT[%d]", pLoamEquipList->dCount);
	for (i = 0; i < pLoamEquipList->dCount; i++)
	{
		if(pLoamEquipList->stRoamEquip[i].dType >= ROAM_NASIP_BASE)
		{
			uiBaseIP = GetSubNet(pLoamEquipList->stRoamEquip[i].uiIP, pLoamEquipList->stRoamEquip[i].uiNetMask);
			dLoop = GetSubNetLoopCnt(pLoamEquipList->stRoamEquip[i].uiNetMask);
			dType = pLoamEquipList->stRoamEquip[i].dType;

			for(j = 0; j < dLoop; j++)
			{
				uiKey = uiBaseIP + j;
				pstNASIPHash->ucNationID = dType;
				dCnt++;

				log_print(LOGN_INFO, "NASIP i=%d j=%d KEY=%s:%u NATIONID=%u", 
						i, j, util_cvtipaddr(szIP, uiKey), uiKey, pstNASIPHash->ucNationID);

				if((pHASHONODE = hasho_add(pNASIPINFO, (U8*)(&uiKey), (U8*)pstNASIPHash)) == NULL)
				{
					log_print(LOGN_CRI, "[%s][%s.%d] hasho_add NULL i=%d j=%d KEY=%s:%u NATIONID=%u", 
						__FILE__, __FUNCTION__, __LINE__, i, j, util_cvtipaddr(szIP, uiKey), uiKey, pstNASIPHash->ucNationID);
				}
			}
		}
	}

	log_print(LOGN_INFO, "NASIP CNT=%d", dCnt);
	return 0;	
}

S32 dMakeDefectHash()
{
	S32 i, dCount;
	S32 uiDefectKey;
	HData_DEFECT    stDefectHash, *pstDefectHash;

	stHASHONODE     *pHASHONODE;
	st_DefectThresholdList *pDefectThresholdList;


	pstDefectHash = &stDefectHash;

	/**** DEFECT HASH ****/
	hasho_reset(pDEFECTINFO);
	pDefectThresholdList = &gWatchFilter->stDefectThresholdList;
	dCount = min(pDefectThresholdList->dCount, MAX_DEFECT_THRES);
	log_print(LOGN_INFO, "[%s][%s.%d] Count[%d] DEFECTLIST COUNT[%d] MAX_DEFECT_THRES[%d]", __FILE__, __FUNCTION__, __LINE__, 
			dCount, pDefectThresholdList->dCount, MAX_DEFECT_THRES);
	for (i = 0; i < dCount; ++i)
	{
		uiDefectKey = pDefectThresholdList->stDefectThreshold[i].uiSvcType;

		pstDefectHash->uiTCPSetupTime = pDefectThresholdList->stDefectThreshold[i].uiTCPSetupTime;
		pstDefectHash->uiResponseTime = pDefectThresholdList->stDefectThreshold[i].uiResponseTime;
		pstDefectHash->uiUpThroughput = pDefectThresholdList->stDefectThreshold[i].uiUpThroughput;
		pstDefectHash->uiDnThroughput = pDefectThresholdList->stDefectThreshold[i].uiDnThroughput;
		pstDefectHash->uiUpRetransCnt = pDefectThresholdList->stDefectThreshold[i].uiUpRetransCnt;
		pstDefectHash->uiDnRetransCnt = pDefectThresholdList->stDefectThreshold[i].uiDnRetransCnt;
		pstDefectHash->UpJitter = pDefectThresholdList->stDefectThreshold[i].UpJitter;
		pstDefectHash->DnJitter = pDefectThresholdList->stDefectThreshold[i].DnJitter;
		pstDefectHash->UpPacketLoss = pDefectThresholdList->stDefectThreshold[i].UpPacketLoss;
		pstDefectHash->DnPacketLoss = pDefectThresholdList->stDefectThreshold[i].DnPacketLoss;

		log_print(LOGN_CRI, "[TYPE:%u][TCP:%u][RES:%u][UPTHR:%u][DNTHR:%u][UPRE:%u][DNRE:%u][UPJIT:%u][DNJIT:%u][UPLOSS:%u][DNLOSS:%u]",
				uiDefectKey, pstDefectHash->uiTCPSetupTime, pstDefectHash->uiResponseTime, pstDefectHash->uiUpThroughput,
				pstDefectHash->uiDnThroughput, pstDefectHash->uiUpRetransCnt, pstDefectHash->uiDnRetransCnt, pstDefectHash->UpJitter,
				pstDefectHash->DnJitter, pstDefectHash->UpPacketLoss, pstDefectHash->DnPacketLoss);

		log_print(LOGN_INFO, "[%s][%s.%d] IDX[%d]DEFECTKEY[%u]", __FILE__, __FUNCTION__, __LINE__, i, uiDefectKey);

		if ( (pHASHONODE = hasho_add(pDEFECTINFO, (U8*)(&uiDefectKey), (U8*)pstDefectHash)) == NULL )
		{
			log_print(LOGN_CRI, "[%s][%s.%d] hasho_add NULL ", __FILE__, __FUNCTION__, __LINE__);
		}
	}

	return 0;	
}

/*
S32 dMakeModelHash()
{
	S32 i, dCount;
	RPPISESS_KEY    stRPPIKey, *pstRPPIKey;
	HData_Model    stModelHash, *pstModelHash;

	stHASHONODE     *pHASHONODE;

	st_ModelInfoList *pModelInfoList;

	pstModelHash = &stModelHash;
	pstRPPIKey = &stRPPIKey;

	hasho_reset(pMODELINFO);

	pModelInfoList = &gWatchFilter->stModelInfoList;

	dCount = min(pModelInfoList->dCount, MAX_MODEL_INFO);
	log_print(LOGN_CRI, "[%s][%s.%d] Count[%d] MODELLIST COUNT[%d] MAX_MODEL_INFO[%d]", __FILE__, __FUNCTION__, __LINE__,
			dCount, pModelInfoList->dCount, MAX_MODEL_INFO);
	for (i = 0; i < dCount; ++i)
	{
		memset(pstRPPIKey, 0x00, MAX_MIN_SIZE);
		memcpy(pstRPPIKey->szIMSI, pModelInfoList->stModelInfo[i].szIMSI, MAX_MIN_LEN);

		memcpy(pstModelHash->szModel, pModelInfoList->stModelInfo[i].szModel, MAX_MODEL_LEN);
		memcpy(pstModelHash->szMIN, pModelInfoList->stModelInfo[i].szMIN, MAX_MIN_LEN);
		log_print(LOGN_INFO, "[%s][%s.%d] IDX[%d]MODELKEY[%s]", __FILE__, __FUNCTION__, __LINE__, i, pstRPPIKey->szIMSI);

		if ( (pHASHONODE = hasho_add(pMODELINFO, (U8*)(pstRPPIKey), (U8*)pstModelHash)) == NULL )
		{
			log_print(LOGN_CRI, "[%s][%s.%d] hasho_add NULL[%s][%s][%s] ", __FILE__, __FUNCTION__, __LINE__, pstRPPIKey->szIMSI, pstModelHash->szModel, pstModelHash->szMIN);
		}
	}	
	return 0;
}
*/

S32 dMakeModelHash()
{
	if( gWatchFilter->stModelInfoList.dActiveStatus == 1 ) {
		pMODELINFO = pMODELINFO1;
	}
	else if( gWatchFilter->stModelInfoList.dActiveStatus == 2 ) {
		pMODELINFO = pMODELINFO2;
	}
	else
		return -1;	

	return 1;
}

U8 *PrintMonType(U16 MonType)
{
	switch(MonType)
	{
		case WATCH_TYPE_A11:    return "A11";
		case WATCH_TYPE_AAA:    return "AAA";
		case WATCH_TYPE_HSS:    return "HSS";
		case WATCH_TYPE_LNS:    return "LNS";
		case WATCH_TYPE_SVC:    return "SVC";
		default:                return "UNKNOWN";
	}
}

U8 *PrintOFFICE(U8 office)
{
	switch(office)
	{
		case OFFICE_IDX_GS:     return "GASAN";
		case OFFICE_IDX_SA:     return "SANGAM";
		case OFFICE_IDX_JA:     return "JUNGANG";
		case OFFICE_IDX_IC:     return "INCHUN";
		case OFFICE_IDX_SW:     return "SUWON";
		case OFFICE_IDX_WJ:     return "WONJU";
		case OFFICE_IDX_BS:     return "BUSAN";
		case OFFICE_IDX_DG:     return "DAEGU";
		case OFFICE_IDX_GJ:     return "KWANGJU";
		case OFFICE_IDX_DJ:     return "DAEJUN";
		default:                return "UNKNOWN";
	}
}

U8 *PrintSVC(S32 svc)
{
	switch(svc)
	{
		case L4_WAP20:          return "WAP2.0";
		case L4_TODAY:          return "TODAY";
		case L4_WIPI:
		case L4_WIPI_ONLINE:    return "WIPI";
		case L4_DN_2G:
		case L4_DN_2G_NODN:
		case L4_DN_JAVA:
		case L4_DN_VOD:
		case L4_DN_VOD_NODN:
		case L4_OMA_DN:
		case L4_OMA_DN_2G:
		case L4_OMA_DN_VOD:
		case L4_OMA_DN_WIPI:    return "DOWNLOAD";
		case L4_RTS_FB:
		case L4_RTS_WB:
		case L4_VOD_STREAM:     return "VOD";
		case L4_MMS_UP:
		case L4_MMS_UP_NODN:
		case L4_MMS_DN:
		case L4_MMS_DN_NODN:
		case L4_MMS_NEW:        return "MMS";
		case L4_FB:
		case L4_IV:
		case L4_EMS:
		case L4_P_EMS:
		case L4_EMS_NO:
		case L4_FV_FB:
		case L4_FV_EMS:
		case L4_FV_IV:          return "OZ";
		case L4_SIP_MS:
		case L4_SIP_VENDOR:
		case L4_SIP_CSCF:
		case L4_MSRP_MS:
		case L4_MSRP_VENDOR:
		case L4_XCAP:           return "IM";
		case L4_MBOX:
		case L4_BANKON:
		case L4_VMBANK:     	return "INTERNET";
		case L4_WIDGET:
		case L4_WIDGET_NODN:    return "WIDGET";
		case L4_VT:             return "VT";
		case L4_JNC:			return "JNC";
		case L4_FTP:			return "FTP";
		case L4_PHONE:
		case L4_PHONE_ETC:		return "PHONE";
		case L4_DNS:			return "DNS";
		case L4_CORP:			return "CORP";
		default:                return "UNKNOWN";
	}
}

S32 dGetSvcIndex(S32 PlatformType)
{
	switch (PlatformType)
	{
		case DEF_PLATFORM_MENU: return SVC_IDX_MENU;
		case DEF_PLATFORM_DN: return SVC_IDX_DN;
		case DEF_PLATFORM_STREAM: return SVC_IDX_STREAM;
		case DEF_PLATFORM_MMS: return SVC_IDX_MMS;
		case DEF_PLATFORM_WIDGET: return SVC_IDX_WIDGET;
		case DEF_PLATFORM_PHONE: return SVC_IDX_PHONE;
		case DEF_PLATFORM_EMS: return SVC_IDX_EMS;
		case DEF_PLATFORM_BANK: return SVC_IDX_BANK;
		case DEF_PLATFORM_FV: return SVC_IDX_FV;
		case DEF_PLATFORM_IM: return SVC_IDX_IM;
		case DEF_PLATFORM_VT: return SVC_IDX_VT;
		case DEF_PLATFORM_ETC:  return SVC_IDX_ETC;
		case DEF_PLATFORM_CORP:  return SVC_IDX_CORP;
		default: return -1;
	}
}


U8 *PrintMsg(LOG_SIGNAL *pstSIGNAL)
{
	switch(pstSIGNAL->uiProtoType)
	{
		case START_CALL_NUM:
		case A11_PROTO:
			switch(pstSIGNAL->uiMsgType)
			{
				case A11_REGIREQ_MSG:
					switch(pstSIGNAL->ucAirLink)
					{   
						case CONNSETUP :                return "A11 REGI CONN SETUP";
						case ACTIVE_START :             return "A11 REGI ACTIVE START";
						case CONNSETUP_ACTIVE_START :   return "A11 REGI CONN SETUP & ACTIVE START";
						case ACTIVE_STOP :              return "A11 REGI ACTIVE STOP";
						case ACTIVE_START_STOP:			return "A11 REGI ACTIVE START & STOP";
						default :                       return "A11 REGI UNKNOWN MSG";
					}   					
				case A11_REGIUPDATE_MSG: 				return "A11 REGIUPDATE";
				case A11_SESSUPDATE_MSG:				return "A11 SESSUPDATE";
				default:								return "A11 UNKNOWN MSG";
			}
			break;
		case RADIUS_PROTO:
			switch(pstSIGNAL->uiMsgType)
			{
				case RADIUS_ACCESS_MSG:					return "RADIUS ACCESS";
				case RADIUS_ACCOUNT_MSG:
														switch (pstSIGNAL->ucAcctType)
														{
															case ACCOUNTING_START :     	return "RADIUS ACCOUNT START";
															case ACCOUNTING_STOP :      	return "RADIUS ACCOUNT STOP";
															case ACCOUNTING_INTERIM:		return "RADIUS ACCOUNT INTERIM";
															default :                   	return "RADIUS ACCOUNT UNKNOWN MSG";
														}
				default:								return "RADIUS UNKNOWN MSG";
			}
		case DIAMETER_PROTO:
			switch (pstSIGNAL->uiMsgType)
			{
				case USER_AUTHORIZATION_TRANS:			return "DIAMETER USER_AUTHORIZATION_TRANS";
				case SERVER_ASSIGNMENT_TRANS:			return "DIAMETER SERVER_ASSIGNMENT_TRANS";
				case LOCATION_INFO_TRANS:				return "DIAMETER LOCATION_INFO_TRANS";
				case MULTIMEDIA_AUTH_TRANS:				return "DIAMETER MULTIMEDIA_AUTH_TRANS";
				case REGISTRATION_TERMINATION_TRANS:	return "DIAMETER REGISTRATION_TERMINATION_TRANS";
				case PUSH_PROFILE_TRANS:				return "DIAMETER PUSH_PROFILE_TRANS";
				case USER_DATA_TRANS:					return "DIAMETER USER_DATA_TRANS";
				case PROFILE_UPDATE_TRANS:				return "DIAMETER PROFILE_UPDATE_TRANS";
				case SUBSCRIBE_NOTIFICATIONS_TRANS:		return "DIAMETER SUBSCRIBE_NOTIFICATIONS_TRANS";
				case PUSH_NOTIFICATION_TRANS:			return "DIAMETER PUSH_NOTIFICATION_TRANS";
				case BOOSTRAPPING_INFO_TRANS:			return "DIAMETER BOOSTRAPPING_INFO_TRANS";
				case MESSAGE_PROCES_TRANS:				return "DIAMETER MESSAGE_PROCES_TRANS";
				case ACCOUNTING_REQUEST_TRANS:			return "DIAMETER ACCOUNTING_REQUEST_TRANS";			
				case DEVICE_WATCHDOG_TRANS:				return "DIAMETER DEVICE_WATCHDOG_TRANS";
				default :                   			return "DIAMETER UNKNOWN MSG";
			}		
		case UPLCP_PROTO:
			switch (pstSIGNAL->uiMsgType)
			{
				case PPP_SETUP :            			return "UPLCP REQ";
				case PPP_TERM :             			return "UPLCP TERM";
				default :                   			return "UPLCP UNKNOWN MSG";
			}
		case DNLCP_PROTO:
			switch (pstSIGNAL->uiMsgType)
			{
				case PPP_SETUP :            			return "DNLCP REQ";
				case PPP_TERM :             			return "DNLCP TERM";
				default :                   			return "DNLCP UNKNOWN MSG";
			}
		case UPIPCP_PROTO:
			switch (pstSIGNAL->uiMsgType)
			{
				case PPP_SETUP :            			return "UPIPCP REQ";
				case PPP_TERM :             			return "UPIPCP TERM";
				default :                   			return "UPIPCP UNKNOWN MSG";
			}
		case DNIPCP_PROTO:
			switch (pstSIGNAL->uiMsgType)
			{
				case PPP_SETUP :            			return "DNIPCP REQ";
				case PPP_TERM :             			return "DNIPCP TERM";
				default :                   			return "DNIPCP UNKNOWN MSG";
			}
		case CHAP_PROTO:								return "CHAP";
		case PAP_PROTO:									return "PAP";
		case OTHERPPP_PROTO:							return "OTHERPPP";
		case DEF_PROTOCOL_L2TP:
			switch(pstSIGNAL->uiMsgType)
			{
				case MSG_L2TP_TUNNEL_START:				return "L2TP TUNNEL START";
				case MSG_L2TP_TUNNEL_STOP:				return "L2TP TUNNEL STOP";
				case MSG_L2TP_TUNNEL_INTERIM:			return "L2TP TUNNEL INERIM";
				case MSG_L2TP_CALL_START:				return "L2TP CALL START";
				case MSG_L2TP_CALL_STOP:				return "L2TP CALL STOP";
				default:								return "L2TP UNKNOWN MSG";
			}
		default:										return "UNKNOWN PROTO";
	}
} 

S32 dGetCallState(S32 dCurState, S32 dInputState)
{
	return ((dCurState < dInputState) ? dInputState : dCurState);
}

U32 uiGetSetupFailReason(S32 dCallState, U32 uiLastFailReason, U32 uiSetupFailReason)
{
	if(uiSetupFailReason) 			return uiSetupFailReason;

	switch(dCallState)
	{
		case ROAM_ACCESS_STATE:
		case ROAM_ACCOUNT_STATE:
		case ROAM_L2TP_SETUP_STATE:
		case ROAM_LNS_ACCESS_STATE:
		case ROAM_AUTH_SETUP_STATE:
		case ROAM_IPCP_SETUP_STATE:
		case ROAM_LNS_ACCOUNT_STATE:	return uiLastFailReason;
		default:						return uiSetupFailReason;
	}
}

void vGetLCPDuration(LOG_RPPI *pLOG)
{
	S64		llUpStart, llUpEnd, llDnStart, llDnEnd;
	S64		llStart, llEnd;

	llUpStart = (S64)pLOG->uiUpLCPStartTime * (S64)1000000 + (S64)pLOG->uiUpLCPStartMTime;
	llUpEnd = (S64)pLOG->uiUpLCPEndTime * (S64)1000000 + (S64)pLOG->uiUpLCPEndMTime;
	llDnStart = (S64)pLOG->uiDnLCPStartTime * (S64)1000000 + (S64)pLOG->uiDnLCPStartMTime;
	llDnEnd = (S64)pLOG->uiDnLCPEndTime * (S64)1000000 + (S64)pLOG->uiDnLCPEndMTime;

	llStart = (llUpStart > llDnStart) ? llUpStart : llDnStart;
	llEnd = (llUpEnd > llDnEnd) ? llUpEnd : llDnEnd;
	
	pLOG->llLCPDuration = ((llEnd - llStart) > 0) ? (llEnd - llStart) : 0;
}

void vGetIPCPDuration(LOG_RPPI *pLOG)
{
	S64		llUpStart, llUpEnd, llDnStart, llDnEnd;
	S64		llStart, llEnd;

	llUpStart = (S64)pLOG->uiUpIPCPStartTime * (S64)1000000 + (S64)pLOG->uiUpIPCPStartMTime;
	llUpEnd = (S64)pLOG->uiUpIPCPEndTime * (S64)1000000 + (S64)pLOG->uiUpIPCPEndMTime;
	llDnStart = (S64)pLOG->uiDnIPCPStartTime * (S64)1000000 + (S64)pLOG->uiDnIPCPStartMTime;
	llDnEnd = (S64)pLOG->uiDnIPCPEndTime * (S64)1000000 + (S64)pLOG->uiDnIPCPEndMTime;

	llStart = (llUpStart > llDnStart) ? llUpStart : llDnStart;
	llEnd = (llUpEnd > llDnEnd) ? llUpEnd : llDnEnd;
	
	pLOG->llIPCPDuration = ((llEnd - llStart) > 0) ? (llEnd - llStart) : 0;
	
}

U32 uiGetLCPFail(LOG_RPPI *pLOG)
{
	U32			err = ERR_LCP_CALL_CUT;

	if(pLOG->uiUpLCPStartTime > 0 && pLOG->uiDnLCPStartTime > 0)
	{
		if(pLOG->uiUpLCPEndTime > 0 && pLOG->uiDnLCPEndTime == 0) {
			err = ERR_LCP_912;
		}
		else if(pLOG->uiUpLCPEndTime == 0 && pLOG->uiDnLCPEndTime == 0) {
			err = ERR_LCP_913;
		}
		else if(pLOG->uiUpLCPEndTime == 0 && pLOG->uiDnLCPEndTime > 0) {
			err = ERR_LCP_915;
		}
		else {
			if(pLOG->usUpLCPRetrans > 0) {
				err = ERR_LCP_916;
			}
			else if(pLOG->usDnLCPRetrans > 0) {
				err = ERR_LCP_917;	
			}
			else {
				log_print(LOGN_WARN, "CUT LCP1 IMSI=%s UPS=%u.%u UPE=%u.%u DNS=%u.%u DNE=%u.%u UPR=%u DNR=%u",
					pLOG->szIMSI, 
					pLOG->uiUpLCPStartTime, pLOG->uiUpLCPStartMTime,
					pLOG->uiUpLCPEndTime, pLOG->uiUpLCPEndMTime,
					pLOG->uiDnLCPStartTime, pLOG->uiDnLCPStartMTime,
					pLOG->uiDnLCPEndTime, pLOG->uiDnLCPEndMTime,
					pLOG->usUpLCPRetrans, pLOG->usDnLCPRetrans);
			}
		}
	}
	else if(pLOG->uiUpLCPStartTime > 0 && pLOG->uiDnLCPStartTime == 0)
	{
		if(pLOG->uiUpLCPEndTime == 0) {
			err = ERR_LCP_911;			
		}
		else {
			log_print(LOGN_WARN, "CUT LCP2 IMSI=%s UPS=%u.%u UPE=%u.%u DNS=%u.%u DNE=%u.%u UPR=%u DNR=%u",
				pLOG->szIMSI, 
				pLOG->uiUpLCPStartTime, pLOG->uiUpLCPStartMTime,
				pLOG->uiUpLCPEndTime, pLOG->uiUpLCPEndMTime,
				pLOG->uiDnLCPStartTime, pLOG->uiDnLCPStartMTime,
				pLOG->uiDnLCPEndTime, pLOG->uiDnLCPEndMTime,
				pLOG->usUpLCPRetrans, pLOG->usDnLCPRetrans);
		}
	}	
	else if(pLOG->uiUpLCPStartTime == 0 && pLOG->uiDnLCPStartTime > 0)
	{
		if(pLOG->uiDnLCPEndTime == 0) {
			err = ERR_LCP_914;
		}
		else {
			log_print(LOGN_WARN, "CUT LCP3 IMSI=%s UPS=%u.%u UPE=%u.%u DNS=%u.%u DNE=%u.%u UPR=%u DNR=%u",
				pLOG->szIMSI, 
				pLOG->uiUpLCPStartTime, pLOG->uiUpLCPStartMTime,
				pLOG->uiUpLCPEndTime, pLOG->uiUpLCPEndMTime,
				pLOG->uiDnLCPStartTime, pLOG->uiDnLCPStartMTime,
				pLOG->uiDnLCPEndTime, pLOG->uiDnLCPEndMTime,
				pLOG->usUpLCPRetrans, pLOG->usDnLCPRetrans);
		}
	}
	else
	{
		log_print(LOGN_WARN, "CUT LCP4 IMSI=%s UPS=%u.%u UPE=%u.%u DNS=%u.%u DNE=%u.%u UPR=%u DNR=%u",
			pLOG->szIMSI, 
			pLOG->uiUpLCPStartTime, pLOG->uiUpLCPStartMTime,
			pLOG->uiUpLCPEndTime, pLOG->uiUpLCPEndMTime,
			pLOG->uiDnLCPStartTime, pLOG->uiDnLCPStartMTime,
			pLOG->uiDnLCPEndTime, pLOG->uiDnLCPEndMTime,
			pLOG->usUpLCPRetrans, pLOG->usDnLCPRetrans);
	}

	return err;
}

U32 uiGetCHAPFail(LOG_RPPI *pLOG)
{
	U32			err = ERR_CHAP_CALL_CUT;

	if(pLOG->uiAuthResTime == 0 && pLOG->uiAuthEndTime == 0) {
		err = ERR_CHAP_921;	
	}
	else if(pLOG->uiAuthResTime > 0 && pLOG->uiAuthEndTime == 0) {
		err = ERR_CHAP_922;
	}
	else {
		if(pLOG->uiCHAPRespCode == 4) {
			err = ERR_CHAP_923;
		}
		else if(pLOG->uiCHAPRespCode == 3) {
			err = ERR_CHAP_924;
		}
		else {
			log_print(LOGN_WARN, "CUT CHAP IMSI=%s REQ=%u.%u RES=%u.%u END=%u.%u RST=%u",
				pLOG->szIMSI, 
				pLOG->uiAuthReqTime, pLOG->uiAuthReqMTime,
				pLOG->uiAuthResTime, pLOG->uiAuthResMTime,
				pLOG->uiAuthEndTime, pLOG->uiAuthEndMTime,
				pLOG->uiCHAPRespCode);
		}
	}

	return err;
}

U32 uiGetIPCPFail(LOG_RPPI *pLOG)
{
	U32			err = ERR_IPCP_CALL_CUT;

	if(pLOG->uiUpIPCPStartTime > 0 && pLOG->uiDnIPCPStartTime > 0)
	{
		if(pLOG->uiUpIPCPEndTime == 0 && pLOG->uiDnIPCPEndTime == 0) {
			err = ERR_IPCP_943;
		}
		else if(pLOG->uiUpIPCPEndTime > 0 && pLOG->uiDnIPCPEndTime == 0) {
			err = ERR_IPCP_944;
		}
		else if(pLOG->uiUpIPCPEndTime == 0 && pLOG->uiDnIPCPEndTime > 0) {
			err = ERR_IPCP_945;
		}
		else {
			if(pLOG->usUpIPCPRetrans > 0) {
				err = ERR_IPCP_946;
			}
			else if(pLOG->usDnIPCPRetrans > 0) {
				err = ERR_IPCP_947;
			}
			else {
				log_print(LOGN_WARN, "CUT IPCP1 IMSI=%s UPS=%u.%u UPE=%u.%u DNS=%u.%u DNE=%u.%u UPR=%u DNR=%u",
					pLOG->szIMSI, 
					pLOG->uiUpIPCPStartTime, pLOG->uiUpIPCPStartMTime,
					pLOG->uiUpIPCPEndTime, pLOG->uiUpIPCPEndMTime,
					pLOG->uiDnIPCPStartTime, pLOG->uiDnIPCPStartMTime,
					pLOG->uiDnIPCPEndTime, pLOG->uiDnIPCPEndMTime,
					pLOG->usUpIPCPRetrans, pLOG->usDnIPCPRetrans);
			}
		}
	}
	else if(pLOG->uiUpIPCPStartTime > 0 && pLOG->uiDnIPCPStartTime == 0)
	{
		if(pLOG->uiUpIPCPEndTime == 0) {
			err = ERR_IPCP_942;
		}
		else {
			log_print(LOGN_WARN, "CUT IPCP2 IMSI=%s UPS=%u.%u UPE=%u.%u DNS=%u.%u DNE=%u.%u UPR=%u DNR=%u",
				pLOG->szIMSI, 
				pLOG->uiUpIPCPStartTime, pLOG->uiUpIPCPStartMTime,
				pLOG->uiUpIPCPEndTime, pLOG->uiUpIPCPEndMTime,
				pLOG->uiDnIPCPStartTime, pLOG->uiDnIPCPStartMTime,
				pLOG->uiDnIPCPEndTime, pLOG->uiDnIPCPEndMTime,
				pLOG->usUpIPCPRetrans, pLOG->usDnIPCPRetrans);
		}
	}	
	else if(pLOG->uiUpIPCPStartTime == 0 && pLOG->uiDnIPCPStartTime > 0)
	{
		if(pLOG->uiDnIPCPEndTime == 0) {
			err = ERR_IPCP_941;
		}
		else {
			log_print(LOGN_WARN, "CUT IPCP3 IMSI=%s UPS=%u.%u UPE=%u.%u DNS=%u.%u DNE=%u.%u UPR=%u DNR=%u",
				pLOG->szIMSI, 
				pLOG->uiUpIPCPStartTime, pLOG->uiUpIPCPStartMTime,
				pLOG->uiUpIPCPEndTime, pLOG->uiUpIPCPEndMTime,
				pLOG->uiDnIPCPStartTime, pLOG->uiDnIPCPStartMTime,
				pLOG->uiDnIPCPEndTime, pLOG->uiDnIPCPEndMTime,
				pLOG->usUpIPCPRetrans, pLOG->usDnIPCPRetrans);
		}
	}
	else
	{
		log_print(LOGN_WARN, "CUT IPCP4 IMSI=%s UPS=%u.%u UPE=%u.%u DNS=%u.%u DNE=%u.%u UPR=%u DNR=%u",
			pLOG->szIMSI, 
			pLOG->uiUpIPCPStartTime, pLOG->uiUpIPCPStartMTime,
			pLOG->uiUpIPCPEndTime, pLOG->uiUpIPCPEndMTime,
			pLOG->uiDnIPCPStartTime, pLOG->uiDnIPCPStartMTime,
			pLOG->uiDnIPCPEndTime, pLOG->uiDnIPCPEndMTime,
			pLOG->usUpIPCPRetrans, pLOG->usDnIPCPRetrans);
	}

	return err;
}

U32 uiGetL2TPFail(LOG_RPPI *pLOG)
{
	U32			err = ERR_L2TP_1;
	
	if(pLOG->uiDnLCPStartTime > 0) {
		/* Incoming-Call-Connected(ICCN) 이후에 호 종료 */
		err = ERR_L2TP_3;
	}
	else if(pLOG->uiUpLCPEndTime > 0) {
		/* Incoming-Call-Reply(ICRP) 이후에 호 종료 */
		err = ERR_L2TP_2;
	}
	else if(pLOG->uiUpLCPStartTime > 0) {
		/* Incoming-Call-Request 이후에 호 종료 */
		err = ERR_L2TP_1;
	}
	else {
		log_print(LOGN_WARN, "CUT L2TP IMSI=%s ICRQ=%u.%u ICRP=%u.%u ICCN=%u.%u",
			pLOG->szIMSI, 
			pLOG->uiUpLCPStartTime, pLOG->uiUpLCPStartMTime,
			pLOG->uiUpLCPEndTime, pLOG->uiUpLCPEndMTime,
			pLOG->uiDnLCPStartTime, pLOG->uiDnLCPStartMTime);
	}

	return err;
}

S32 dGetCallControl(S32 dID)
{
	switch(dID)
	{
#ifdef _PDIF_
    case TYPE_PDIF:
#endif
    case TYPE_CRX:			return 1;
    case TYPE_LAC:
    case TYPE_LNS:
    default:				return 0;
	}
}

unsigned int GetSubNet(unsigned int ip, int netmask)
{
	return (ip & (0xFFFFFFFFU >> (32 - netmask)) << (32 - netmask));
}

int GetSubNetLoopCnt(int netmask)
{
	return pow(2, (32 - netmask));
}

S32 dGetCallType(S32 nat)
{
	S32			calltype = 0;

	if(nat >= 30 && nat < 119)				calltype = INIT_ROAM_CALLSTART;
	else if(nat >= 120 && nat < 250) 		calltype = INIT_ROAM_CALLSTART_LNS;

	return calltype;
}

S32 dMakeIRMHash()
{
	int		dLine;
	FILE	*fa;
	char	szBuf[1024];
	char	szInfo_1[64], szInfo_2[64], szInfo_3[64];

	st_IRMHash_Key		stIRMHashKey;
	st_IRMHash_Key		*pKey = &stIRMHashKey;
	st_IRMHash_Data		stIRMHashData;
	st_IRMHash_Data		*pData = &stIRMHashData;
	stHASHONODE			*pHASHNODE;

	/* reset hash */
	hasho_reset(pIRMINFO);

	fa = fopen(DATA_PATH"IRM.DAT", "r");
	if(fa == NULL) {
		log_print(LOGN_CRI,"dMakeIRMHash : IRM.DAT FILE OPEN FAIL (%s)", strerror(errno));
		return -1;
	}

	dLine = 0;
	while(fgets(szBuf,1024,fa) != NULL)
	{
		if(szBuf[0] != '#') {
			log_print(LOGN_CRI,"dMakeIRMHash : IRM.DAT File [%d] ROW FORMAT ERR", dLine+1);
			fclose(fa);
			return -2;
		}

		if(szBuf[1] == '#')
			continue;
		else if(szBuf[1] == 'E')
			break;
		else if(szBuf[1] == '@') {
			if(sscanf(&szBuf[2],"%s %s %s", szInfo_1, szInfo_2, szInfo_3) == 3) {
				log_print(LOGN_CRI, "READ IRM DATA i=%d INFO1=%s INFO2=%s INFO3=%s", dLine, szInfo_1, szInfo_2, szInfo_3);

				memset(pKey, 0x00, DEF_IRMHASH_KEY_SIZE);
				memset(pData, 0x00, DEF_IRMHASH_DATA_SIZE);	

				memcpy(pKey->szIRM, szInfo_2, DEF_IRM_PATTERN_LEN);
				pKey->szIRM[DEF_IRM_PATTERN_LEN] = 0x00;

				memcpy(pData->szIMSI, szInfo_1, DEF_IRM_PATTERN_LEN);
				pData->szIMSI[DEF_IRM_PATTERN_LEN] = 0x00;

				memcpy(pData->szPrefix, szInfo_3, DEF_IRM_PREFIX_LEN);
				pData->szPrefix[DEF_IRM_PREFIX_LEN] = 0x00;

				if((pHASHNODE = hasho_add(pIRMINFO, (U8 *)pKey, (U8 *)pData)) != NULL) {
					log_print(LOGN_CRI, "HASH ADD IRM i=%d IRM=%s IMSI=%s PREFIX=%s", 
						dLine, pKey->szIRM, pData->szIMSI, pData->szPrefix);
				}
				else {
					log_print(LOGN_CRI, "HASH ADD FAIL IRM i=%d IRM=%s IMSI=%s PREFIX=%s", 
						dLine, pKey->szIRM, pData->szIMSI, pData->szPrefix);
				}
			}
		}
		dLine++;
	}

	fclose(fa);

	return 0;	
}

S32 dCvtIRM(U8 *szIMSI)
{
	int					len, offset;
	U8					szRst[MAX_MIN_SIZE];

	st_IRMHash_Key		stIRMHashKey;
	st_IRMHash_Key		*pKey = &stIRMHashKey;
	st_IRMHash_Data		*pData;
	stHASHONODE			*pHASHNODE;

	len = strlen(szIMSI);
	len = (len > MAX_MIN_LEN) ? MAX_MIN_LEN : len;	

	if(len >= DEF_IRM_MIN_LEN)
	{
		memset(pKey, 0x00, DEF_IRMHASH_KEY_SIZE);
		offset = len - DEF_IRM_MIN_LEN;
		memcpy(pKey->szIRM, &szIMSI[offset], DEF_IRM_PATTERN_LEN);
		pKey->szIRM[DEF_IRM_PATTERN_LEN] = 0x00;
		offset = offset + DEF_IRM_PATTERN_LEN;

		if((pHASHNODE = hasho_find(pIRMINFO, (U8 *)pKey)) != NULL)
		{
			pData = (st_IRMHash_Data *)nifo_ptr(pIRMINFO, pHASHNODE->offset_Data);
			sprintf(szRst, "%s%s%s", pData->szPrefix, pData->szIMSI, &szIMSI[offset]);
			log_print(LOGN_INFO, "CVTIRM CVT IRM=%s IMSI=%s", szIMSI, szRst);
			memcpy(szIMSI, szRst, MAX_MIN_LEN);
			szIMSI[MAX_MIN_LEN] = 0x00;
		}
		else 
		{
			log_print(LOGN_CRI, "CVTIRM hasho_find NULL IMSI=%s IRM=%s", szIMSI, pKey->szIRM);
		}
	}
	else
	{
		log_print(LOGN_INFO, "CVTIRM SHORT LENGTH=%d IMSI=%s", len, szIMSI);
	}

	return 0;
}

