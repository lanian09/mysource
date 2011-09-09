/**		@file	o_svcmon_log.c
 * 		- O_SVCMON에서 LOG 포맷을 변경처리하는 소스
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: o_svcmon_util.c,v 1.4 2011/09/07 07:07:52 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.4 $
 * 		@date		$Date: 2011/09/07 07:07:52 $
 * 		@ref		o_svcmon_init.c o_svcmon_maic.c
 *
 * 		@section	Intro(소개)
 * 		- O_SVCMON에서 LOG 포맷을 변경처리하는 소스
 *
 * 		@section	Requirement
 *
 **/

#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <math.h>
// LIB
#include "typedef.h"
#include "loglib.h"
#include "hasho.h"

// PROJECT
#include "common_stg.h"
#include "svcmon.h"

// TAM
#include "watch_mon.h"
#include "watch_filter.h"

#include "o_svcmon_util.h"

S8 *PrintMSGTYPE(S32 msgtype)
{
	switch(msgtype)
	{
	case WATCH_TYPE_A11:	return "A11";
	case WATCH_TYPE_AAA:	return "AAA";
	case WATCH_TYPE_HSS:	return "HSS";
	case WATCH_TYPE_LNS:	return "LNS";
	case WATCH_TYPE_SVC:	return "SVC";
	case WATCH_TYPE_A11AAA:	return "A11AAA";
	case WATCH_TYPE_RECALL:	return "RECALL";
	default:				return "UNKNOWN";
	}
}

S8 *PrintOFFICE(S32 office)
{
	switch(office)
	{
	case OFFICE_IDX_GS:		return "GS";
	case OFFICE_IDX_SA:		return "SA";
	case OFFICE_IDX_JA:		return "JA";
	case OFFICE_IDX_IC:		return "IC";
	case OFFICE_IDX_SW:		return "SW";
	case OFFICE_IDX_WJ:		return "WJ";
	case OFFICE_IDX_BS:		return "BS";
	case OFFICE_IDX_DG:		return "DG";
	case OFFICE_IDX_GJ:		return "GJ";
	case OFFICE_IDX_DJ:		return "DJ";
	case 0:					return "N";
	default:				return "UNKNOWN";
	}
}

S8 *PrintSVC(S32 svc)
{
	switch(svc)
	{
	case L4_WAP20:			return "WAP2.0";
	case L4_TODAY:			return "TODAY";
	case L4_WIPI:
	case L4_WIPI_ONLINE:	return "WIPI";
	case L4_DN_2G:
	case L4_DN_2G_NODN:
	case L4_DN_JAVA:
	case L4_DN_VOD:
	case L4_DN_VOD_NODN:
	case L4_OMA_DN:
	case L4_OMA_DN_2G:
	case L4_OMA_DN_VOD:
	case L4_OMA_DN_WIPI:	return "DOWNLOAD";
	case L4_RTS_FB:
	case L4_RTS_WB:			return "RTS";
	case L4_VOD_STREAM:		return "VOD";
	case L4_MMS_UP:
	case L4_MMS_UP_NODN:
	case L4_MMS_DN:
	case L4_MMS_DN_NODN:
	case L4_MMS_NEW:		return "MMS";
	case L4_FB:				return "FB";
	case L4_IV:				return "IV";
	case L4_EMS:
	case L4_P_EMS:
	case L4_EMS_NO:			return "EMS";
	case L4_FV_FB:
	case L4_FV_EMS:
	case L4_FV_IV:			return "FV";
	case L4_SIP_MS:
	case L4_SIP_VENDOR:
	case L4_SIP_CSCF:
	case L4_MSRP_MS:
	case L4_MSRP_VENDOR:
	case L4_XCAP:			return "IM";
	case L4_MBOX:			return "MBOX";
	case L4_BANKON:			return "BANKON";
	case L4_VMBANK:			return "VMBANK";
	case L4_WIDGET:
	case L4_WIDGET_NODN:	return "WIDGET";
	case L4_VT:				return "VT";
	case L4_JNC:			return "JNC";
	case L4_PHONE:
	case L4_PHONE_ETC:		return "PHONE";
	case L4_CORP:			return "CORP";
#if 1 /* INYOUNG */ 
	case L4_INET_TCP_USER:  return "TCP_USER";
	case L4_INET_HTTP_USER: return "HTTP_USER";
	case L4_INET_TCP:       return "TCP";
	case L4_INET_TCP_RECV:  return "TCP_RECV";
	case L4_INET_HTTP:      return "HTTP";
	case L4_INET_HTTP_RECV: return "HTTP_RECV";
#endif
	default:				return "UNKNOWN";
	}
}

S32 getSvcIndex(S32 svcl4, S32 svcl7)
{
	switch(svcl4)
	{
	case L4_CORP:			return SVC_IDX_CORP;
	case L4_FB:
	case L4_IV:
	case L4_WAP20:			return SVC_IDX_MENU;
	case L4_DN_2G:
	case L4_DN_2G_NODN:
	case L4_DN_JAVA:
	case L4_DN_VOD:
	case L4_DN_VOD_NODN:
	case L4_OMA_DN:
	case L4_OMA_DN_2G:
	case L4_OMA_DN_VOD:
	case L4_OMA_DN_WIPI:	return SVC_IDX_DN;
	case L4_RTS_FB:
	case L4_RTS_WB:
	case L4_MBOX:
	case L4_VOD_STREAM:		return SVC_IDX_STREAM;
	case L4_MMS_UP:
	case L4_MMS_UP_NODN:
	case L4_MMS_DN:
	case L4_MMS_DN_NODN:
	case L4_MMS_NEW:		return SVC_IDX_MMS;
	case L4_TODAY:
	case L4_WIDGET:
	case L4_WIDGET_NODN:	return SVC_IDX_WIDGET;
	case L4_EMS:
	case L4_P_EMS:
	case L4_EMS_NO:			return SVC_IDX_EMS;
	case L4_PHONE:
	case L4_PHONE_ETC:		return SVC_IDX_PHONE;
	case L4_BANKON:
	case L4_VMBANK:			return SVC_IDX_BANK;
	case L4_FV_FB:
	case L4_FV_EMS:
	case L4_FV_IV:			return SVC_IDX_FV;
	case L4_SIP_MS:
	case L4_SIP_VENDOR:
	case L4_SIP_CSCF:
	case L4_MSRP_MS:
	case L4_MSRP_VENDOR:
	case L4_XCAP:
		switch(svcl7)
		{
			case APP_IM_UP:
				return SVC_IDX_IM;
			case APP_IM_DN:
				return SVC_IDX_IM_RECV;
			default:
				return -4;
		}

	case L4_VT:
		switch(svcl7)
		{
			case APP_IM_UP:
				return SVC_IDX_VT;
			case APP_IM_DN:
				return SVC_IDX_VT_RECV;
			default:
				return -3;
		}
		
	/* TODO : service_index_add position */
	/* L7에 대한 내용을 추가하여 recv 관련 정보 출력.*/
	case L4_JNC:
	case L4_WIPI_ONLINE:	return SVC_IDX_ETC;
	case L4_WIPI:
		switch(svcl7)
		{
		case APP_MENU:		return SVC_IDX_MENU;
		case APP_DOWN:		return SVC_IDX_DN;
		default:			return -2;
		}

	case L4_INET_TCP:
	case L4_INET_HTTP:
	case L4_INET_TCP_USER:
	case L4_INET_HTTP_USER:
		return SVC_IDX_INET;
	case L4_INET_TCP_RECV:
	case L4_INET_HTTP_RECV:
		return SVC_IDX_RECVCALL;

	default:				return -1;
	}
}

S8 *PrintSYSTYPE(S32 systype)
{
	switch(systype)
	{
	case SYSTEM_TYPE_SECTOR:		return "SECTOR";
	case SYSTEM_TYPE_FA:			return "FA";
	case SYSTEM_TYPE_BTS:			return "BTS";
	case SYSTEM_TYPE_BSC:			return "BSC";
	case SYSTEM_TYPE_PCF:			return "PCF";
	case SYSTEM_TYPE_PDSN:			return "PDSN";
	case SYSTEM_TYPE_AAA:			return "AAA";
	case SYSTEM_TYPE_HSS:			return "HSS";
	case SYSTEM_TYPE_LNS:			return "LNS";
	case SYSTEM_TYPE_SERVICE:		return "SERVICE";
	case SYSTEM_TYPE_ROAMAAA:		return "ROAMAAA";
	default:						return "UNKNOWN";
	}
}

S8 *PrintSUBTYPE(S32 systype, S32 subtype)
{
	if(systype != SYSTEM_TYPE_SERVICE) {
		return "N";
	}

	return PrintSVCTYPE(subtype);
}

S8 *PrintPCFTYPE(S32 pcftype)
{
	switch(pcftype)
	{
	case PCFTYPE_LG_1x:				return "LG_1x";
	case PCFTYPE_LG_EvDO:			return "LG_EvDO";
	case PCFTYPE_LG_BOTH:			return "LG_BOTH";
	case PCFTYPE_SS_BOTH:			return "SS_BOTH";
	default:						return "UNKNOWN";
	}
}

S8 *PrintALARMTYPE(S32 alarmtype)
{
	switch(alarmtype)
	{
	case DEF_ALARMTYPE_CALL:		return "CALL";
	case DEF_ALARMTYPE_AAA:			return "AAA";
	case DEF_ALARMTYPE_HSS:			return "HSS";
	case DEF_ALARMTYPE_LNS:			return "LNS";
	case DEF_ALARMTYPE_MENU:		return "MENU";
	case DEF_ALARMTYPE_DN:			return "DOWNLOAD";
	case DEF_ALARMTYPE_STREAM:		return "STREAM";
	case DEF_ALARMTYPE_MMS:			return "MMS";
	case DEF_ALARMTYPE_WIDGET:		return "WIDGET";
	case DEF_ALARMTYPE_PHONE:		return "PHONE";
	case DEF_ALARMTYPE_EMS:			return "EMS";
	case DEF_ALARMTYPE_BANK:		return "BANK";
	case DEF_ALARMTYPE_FV:			return "FV";
	case DEF_ALARMTYPE_IM:			return "IM";
	case DEF_ALARMTYPE_VT:			return "VT";
	case DEF_ALARMTYPE_ETC:			return "ETC";
	case DEF_ALARMTYPE_CORP:		return "CORP";
	case DEF_ALARMTYPE_REGI:		return "REGI";
	/*TODO : jhbaek */
	case DEF_ALARMTYPE_VT_RECV:		return "VT_RECV";
	case DEF_ALARMTYPE_INET:		return "INET";
	case DEF_ALARMTYPE_RECVCALL:	return "RECVCALL";
	case DEF_ALARMTYPE_IM_RECV:		return "IM_RECV";

	default:						return "UNKNOWN";
	}
}

S8 *PrintSVCTYPE(S32 svctype)
{
	switch(svctype)
	{
	case SVC_IDX_MENU:				return "MENU";
	case SVC_IDX_DN:				return "DOWNLOAD";
	case SVC_IDX_STREAM:			return "STREAM";
	case SVC_IDX_MMS:				return "MMS";
	case SVC_IDX_WIDGET:			return "WIDGET";
	case SVC_IDX_PHONE:				return "PHONE";
	case SVC_IDX_EMS:				return "EMS";
	case SVC_IDX_BANK:				return "BANK";
	case SVC_IDX_FV:				return "FV";
	case SVC_IDX_IM:				return "IM";
	case SVC_IDX_VT:				return "VT";
	case SVC_IDX_ETC:				return "ETC";
	case SVC_IDX_CORP:				return "CORP";
	case SVC_IDX_REGI:				return "REGI";
	case SVC_IDX_ROAM:				return "ROAM";
	/*jhbaek*/
	case SVC_IDX_INET:				return "INET";
	case SVC_IDX_RECVCALL:			return "INETRECV";
	case SVC_IDX_IM_RECV:			return "IM_RECV";
	case SVC_IDX_VT_RECV:			return "VT_RECV";
	default:						return "UNKNOWN";
	}
}

S8 *PrintALMVALUE(S32 svcvalue)
{
	switch(svcvalue) {
		case MON_ALARMTYPE_NORMAL:	return "NORMAL";
		case MON_ALARMTYPE_RATE:	return "RATE";
		case MON_ALARMTYPE_MIN:		return "MIN";
		case MON_ALARMTYPE_MAX:		return "MAX";
		default:					return "UNKNOWN";
	}
}

S32 getDefectIndex(stHASHOINFO *pDefHash, U32 result)
{
	int		index = -1;
	st_DefHash_Key		stDefHashKey;
	st_DefHash_Key		*pKey = &stDefHashKey;
	st_DefHash_Data		*pData;
	stHASHONODE			*pHASHNODE;

	pKey->uiDefectCode = result;

	if((pHASHNODE = hasho_find(pDefHash, (U8 *)pKey)) != NULL)
	{
		pData = (st_DefHash_Data *)nifo_ptr(pDefHash, pHASHNODE->offset_Data);
		index = pData->uiArrayIndex;
	}

	return index;
}

st_ThresHash_Data *getBaseValue(stHASHOINFO *pThresHash, S32 office, S32 systype, S32 alarmtype, UINT ip)
{

	st_ThresHash_Key	stThresHashKey;
	st_ThresHash_Key	*pKey = &stThresHashKey;
	st_ThresHash_Data	*pData = NULL;
	stHASHONODE			*pHASHONODE;

	pKey->ucOffice = office;
	pKey->ucSysType = systype;
	pKey->ucAlarmType = alarmtype;
	pKey->ucReserved = 0;
	pKey->uiIP = ip;

	if((pHASHONODE = hasho_find(pThresHash, (U8 *)pKey)) != NULL)
	{
		pData = (st_ThresHash_Data *)nifo_ptr(pThresHash, pHASHONODE->offset_Data);
	} 

	return pData;
}

#if 0
S32 isAlarm(st_ThresHash_Data *pData, time_t stattime, U32 trialcnt, U32 succcnt)
{
	U32 min, max, rate;
	U32 succrate = (float)succcnt / (float)trialcnt * 100.0;

	/*
	 *	Alarm 체크 순서 및 우선 순위
	 *	최대시도수 > 최소시도수 > 성공률
	 *	야간의 경우 최소시도수를 체크하지 않고, 최소시도수보다 많은 경우에만 성공률 체크
	 */

	if(getThresFlag(pData, stattime) == DEF_THRESFLAG_DAY) {
		/* 주간 ALARM 체크 */
		min = pData->uiDayTimeMinTrial;
		max = pData->uiPeakTrial;
		rate = pData->ucDayTimeRate;

		if(max > 0 && trialcnt > max) return MON_ALARMTYPE_MAX;				/* 최대시도수 체크 */
		else if(min > 0 && trialcnt < min) return MON_ALARMTYPE_MIN;		/* 최소시도수 체크 */
		else if(rate > 0 && trialcnt > 0 && succrate < rate) return MON_ALARMTYPE_RATE;		/* 성공률 체크 */
		else return MON_ALARMTYPE_NORMAL;
	}
	else {
		/* 야간 ALARM 체크 */
		min = pData->uiNightTimeMinTrial;
		max = pData->uiPeakTrial;
		rate = pData->ucNightTimeRate;

		if(max > 0 && trialcnt > max) return MON_ALARMTYPE_MAX;											/* 최대시도수 체크 */
		else if(min > 0 && trialcnt > min && rate > 0 && succrate < rate) return MON_ALARMTYPE_RATE;	/* 성공률 체크 */
		else return MON_ALARMTYPE_NORMAL;
	}	
}
#endif

S32 isAlarm(st_ThresHash_Data *pData, time_t stattime, U32 trialcnt, U32 succcnt)
{
	U32 min, max, rate, valid;
	U32 succrate = (float)succcnt / (float)trialcnt * 100.0;

	/*
	 *  Alarm 체크 순서 및 우선 순위
	 *  최대시도수 > 유효시도수 > 성공률 > 최소시도수
	 *  야간의 경우 최소시도수를 체크하지 않고, 최소시도수보다 많은 경우에만 성공률 체크
	 */

	if(getThresFlag(pData, stattime) == DEF_THRESFLAG_DAY) {
		/* 주간 ALARM 체크 */
		min = pData->uiDayTimeMinTrial;
		max = pData->uiPeakTrial;
		rate = pData->ucDayTimeRate;
		valid = pData->uiNightTimeMinTrial;

		if(max > 0 && trialcnt > max)                               return MON_ALARMTYPE_MAX;       /* 최대시도수 체크 */
		else if(trialcnt > valid) {
			if(rate > 0 && succrate < rate)                         return MON_ALARMTYPE_RATE;      /* 성공률 체크 */
			else if(min > 0 && trialcnt < min)                      return MON_ALARMTYPE_MIN;       /* 최소시도수 체크 */
			else                                                    return MON_ALARMTYPE_NORMAL;
		}
		else                                                        return MON_ALARMTYPE_NORMAL;
	}
	else {
		/* 야간 ALARM 체크 */
		valid = pData->uiNightTimeMinTrial;
		max = pData->uiPeakTrial;
		rate = pData->ucNightTimeRate;

		if(max > 0 && trialcnt > max)                               return MON_ALARMTYPE_MAX;       /* 최대시도수 체크 */
		else if(trialcnt > valid && rate > 0 && succrate < rate)    return MON_ALARMTYPE_RATE;      /* 성공률 체크 */
		else                                                        return MON_ALARMTYPE_NORMAL;
	}
}

S32 isAlarmAll(st_ThresHash_Data *pData, time_t stattime, U32 trialcnt, U32 succcnt)
{
	U32 min, max, rate, valid;
	U32 succrate = (float)succcnt / (float)trialcnt * 100.0;
	S32	alm = 0;

	/*
	 *	Alarm 체크 순서 및 우선 순위
	 *	최대시도수 > 유효시도수 > 성공률 > 최소시도수
	 *	야간의 경우 최소시도수를 체크하지 않고, 최소시도수보다 많은 경우에만 성공률 체크
	 */

	if(getThresFlag(pData, stattime) == DEF_THRESFLAG_DAY) {
		/* 주간 ALARM 체크 */
		min = pData->uiDayTimeMinTrial;
		max = pData->uiPeakTrial;
		rate = pData->ucDayTimeRate;
		valid = pData->uiNightTimeMinTrial;

		if(max > 0 && trialcnt > max)			/* 최대시도수 체크 */
			alm = DEF_ALARMTYPE_MAX;
		if(trialcnt > valid) {
			if(rate > 0 && succrate < rate)		/* 성공률 체크 */
				alm = alm | DEF_ALARMTYPE_RATE;
			if(min > 0 && trialcnt < min)	/* 최소시도수 체크 */
				alm = alm | DEF_ALARMTYPE_MIN;
		}
	} else {
		/* 야간 ALARM 체크 */
		valid = pData->uiNightTimeMinTrial;
		max = pData->uiPeakTrial;
		rate = pData->ucNightTimeRate;

		if(max > 0 && trialcnt > max)			/* 최대시도수 체크 */
			alm = DEF_ALARMTYPE_MAX;
		if(trialcnt > valid && rate > 0 && succrate < rate)		/* 성공률 체크 */
			alm = alm | DEF_ALARMTYPE_RATE;
	}

	return alm;
}

S32 getAlarm(S32 curalarm, S32 inputalarm)
{
	S32		alarm = curalarm;

	/* MON_ALARMTYPE_MAX > MON_ALARMTYPE_MIN > MON_ALARMTYPE_RATE > MON_ALARMTYPE_NORMAL */

	switch(curalarm)
	{
	case MON_ALARMTYPE_NORMAL:
		switch(inputalarm)
		{
		case MON_ALARMTYPE_RATE:
		case MON_ALARMTYPE_MIN:
		case MON_ALARMTYPE_MAX:
			alarm = inputalarm;
			break;
		default:
			log_print(LOGN_CRI, "F=%s:%s.%d UNKNOWN INPUTALARM TYPE=%d", __FILE__, __FUNCTION__, __LINE__, inputalarm);
			alarm = curalarm;
			break;
		}
		break;
	case MON_ALARMTYPE_RATE:
		switch(inputalarm)
		{
		case MON_ALARMTYPE_MIN:
		case MON_ALARMTYPE_MAX:
			alarm = inputalarm;
			break;
		case MON_ALARMTYPE_RATE:
			alarm = curalarm;
			break;
		default:
			log_print(LOGN_CRI, "F=%s:%s.%d UNKNOWN INPUTALARM TYPE=%d", __FILE__, __FUNCTION__, __LINE__, inputalarm);
			alarm = curalarm;
			break;
		}
		break;
	case MON_ALARMTYPE_MIN:
		switch(inputalarm)
		{
		case MON_ALARMTYPE_MAX:
			alarm = inputalarm;
			break;
		case MON_ALARMTYPE_RATE:
		case MON_ALARMTYPE_MIN:
			alarm = curalarm;
			break;
		default:
			log_print(LOGN_CRI, "F=%s:%s.%d UNKNOWN INPUTALARM TYPE=%d", __FILE__, __FUNCTION__, __LINE__, inputalarm);
			alarm = curalarm;
			break;
		}
		break;
	case MON_ALARMTYPE_MAX:
		switch(inputalarm)
		{
		case MON_ALARMTYPE_RATE:
		case MON_ALARMTYPE_MIN:
		case MON_ALARMTYPE_MAX:
			alarm = curalarm;
			break;
		default:
			log_print(LOGN_CRI, "F=%s:%s.%d UNKNOWN INPUTALARM TYPE=%d", __FILE__, __FUNCTION__, __LINE__, inputalarm);
			alarm = curalarm;
			break;
		}
		break;
	default:
		log_print(LOGN_CRI, "F=%s:%s.%d UNKNOWN CURALARM TYPE=%d", __FILE__, __FUNCTION__, __LINE__, curalarm);
		switch(inputalarm)
		{
		case MON_ALARMTYPE_RATE:
		case MON_ALARMTYPE_MIN:
		case MON_ALARMTYPE_MAX:
			alarm = inputalarm;
			break;
		default:
			log_print(LOGN_CRI, "F=%s:%s.%d UNKNOWN INPUTALARM TYPE=%d", __FILE__, __FUNCTION__, __LINE__, inputalarm);
			alarm = MON_ALARMTYPE_NORMAL;
			break;
		}
		break;
	}

	return alarm;
}

S32 getThresFlag(st_ThresHash_Data *pData, time_t stattime)
{
	struct tm		stTime;

	localtime_r(&stattime, &stTime);

	return pData->ucDayFlag[stTime.tm_hour];
}

S32 getPCFType(S32 pcftype)
{
	switch(pcftype)
	{
	case PCFTYPE_LG_1x:		return PCF_TYPE_1x;
	case PCFTYPE_LG_EvDO:
	case PCFTYPE_LG_BOTH:
	case PCFTYPE_SS_BOTH:	return PCF_TYPE_EvDO;
	default:				return -1;
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

U32 getNasIP(stHASHOINFO *pNasIPHash, U32 ip)
{
	U32						nasip = 0;
	st_NasIPHash_Key		stDefHashKey;
	st_NasIPHash_Key		*pKey = &stDefHashKey;
	st_NasIPHash_Data		*pData;
	stHASHONODE				*pHASHNODE;

	pKey->uiNasIP = ip;

	if((pHASHNODE = hasho_find(pNasIPHash, (U8 *)pKey)) != NULL)
	{
		pData = (st_NasIPHash_Data *)nifo_ptr(pNasIPHash, pHASHNODE->offset_Data);
		nasip = pData->uiNasIPPool;
	}

	return nasip;
}

/*
 * $Log: o_svcmon_util.c,v $
 * Revision 1.4  2011/09/07 07:07:52  hhbaek
 * *** empty log message ***
 *
 * Revision 1.3  2011/09/07 04:32:00  hhbaek
 * *** empty log message ***
 *
 * Revision 1.2  2011/08/31 16:08:08  dhkim
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * NEW OAM SYSTEM
 *
 * Revision 1.1  2011/08/23 08:58:49  dcham
 * *** empty log message ***
 *
 * Revision 1.33  2011/06/23 14:47:57  innaei
 * *** empty log message ***
 *
 * Revision 1.32  2011/06/23 00:58:18  innaei
 * *** empty log message ***
 *
 * Revision 1.31  2011/04/24 19:24:28  innaei
 * *** empty log message ***
 *
 * Revision 1.30  2011/04/22 19:17:23  innaei
 * *** empty log message ***
 *
 * Revision 1.29  2011/04/22 12:24:52  dark264sh
 * O_SVCMON: Print 함수에 type 추가
 *
 * Revision 1.28  2011/04/22 08:19:40  jhbaek
 * *** empty log message ***
 *
 * Revision 1.27  2011/04/20 06:20:41  innaei
 * *** empty log message ***
 *
 * Revision 1.26  2011/04/12 07:42:45  jsyoon
 * *** empty log message ***
 *
 * Revision 1.25  2011/01/11 04:09:18  uamyd
 * modified
 *
 * Revision 1.1.1.1  2010/08/23 01:13:11  uamyd
 * DQMS With TOTMON, 2nd-import
 *
 * Revision 1.24  2010/03/29 12:23:34  dark264sh
 * *** empty log message ***
 *
 * Revision 1.23  2010/03/04 06:00:53  dark264sh
 * ROAM NASIP NetMask 처리
 *
 * Revision 1.22  2010/02/25 11:13:45  dark264sh
 * O_SVCMON ROAM 처리
 *
 * Revision 1.21  2010/02/24 12:19:46  dark264sh
 * 망감시 헤더 변경에 따른 변경
 *
 * Revision 1.20  2009/10/21 11:45:08  pkg
 * 망감시 REGI 서비스 추가 및 define값 변경
 *
 * Revision 1.19  2009/09/08 15:26:20  pkg
 * *** empty log message ***
 *
 * Revision 1.18  2009/09/08 15:22:06  pkg
 * 망감시 주간 유효시도수 추가
 *
 * Revision 1.17  2009/09/07 11:14:43  pkg
 * 법인 서비스 추가
 *
 * Revision 1.16  2009/08/22 12:16:00  pkg
 * *** empty log message ***
 *
 * Revision 1.15  2009/08/17 14:32:30  pkg
 * O_SVCMON 성공률 체크 변경
 *
 * Revision 1.14  2009/07/22 12:05:42  pkg
 * O_SVCMON VMBANK, BANKON L4 CODE 변경 처리
 *
 * Revision 1.13  2009/07/20 01:43:42  dark264sh
 * *** empty log message ***
 *
 * Revision 1.12  2009/07/19 20:12:38  dark264sh
 * *** empty log message ***
 *
 * Revision 1.11  2009/07/12 12:04:01  dark264sh
 * O_SVCMON DEF_PLATFORM_PHONE 관련 예외 처리 추가
 *
 * Revision 1.10  2009/07/10 06:33:42  dark264sh
 * 망감시 관련 서비스 구분 변경
 *
 * Revision 1.9  2009/07/08 18:13:47  dark264sh
 * *** empty log message ***
 *
 * Revision 1.8  2009/07/01 17:44:24  dark264sh
 * *** empty log message ***
 *
 * Revision 1.7  2009/07/01 17:30:26  dark264sh
 * *** empty log message ***
 *
 * Revision 1.6  2009/07/01 14:51:20  dark264sh
 * *** empty log message ***
 *
 * Revision 1.5  2009/06/21 13:34:33  dark264sh
 * *** empty log message ***
 *
 * Revision 1.4  2009/06/20 15:51:23  dark264sh
 * *** empty log message ***
 *
 * Revision 1.3  2009/06/20 10:51:39  dark264sh
 * *** empty log message ***
 *
 * Revision 1.2  2009/06/18 17:01:48  dark264sh
 * O_SVCMON Filter 처리
 *
 * Revision 1.1  2009/06/15 08:06:04  dark264sh
 * O_SVCMON 기본 동작 처리
 *
 */
