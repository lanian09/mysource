/**		@file	online_util.c
 * 		- Util 함수들
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: online_util.c,v 1.3 2011/09/07 06:30:48 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.3 $
 * 		@date		$Date: 2011/09/07 06:30:48 $
 * 		@ref		online_util.c online_func.c online_main.c online_init.c l4.h a_online_api.h
 * 		@todo		library가 만들어지지 않은 상태, library 생성 이후 함수 대치
 *
 * 		@section	Intro(소개)
 * 		- Util 함수들
 *
 * 		@section	Requirement
 * 		 @li library 생성 이후 함수 대치
 *
 **/

/**
 * Include headers
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>


// LIB
#include "typedef.h"
#include "loglib.h"

// PROJECT
#include "procid.h"

#include "online_util.h"

/**
 * Declare variables
 */
extern S32		gACALLCnt;

/** MakeHashKey function.
 *
 *  MakeHashKey Function
 *
 *  @param	*pstTcpInfo : TCP Session 정보 (const st_TcpInfo *)
 *  @param	*pstTcpHashKey : TCP Hash Key (st_TcpHashKey *)
 *
 *  @return			void
 *  @see			online_util.c online_func.c online_main.c online_init.c l4.h a_online_api.h
 *
 *  @note			TCP로 부터 읽는 함수 변경될 경우 변경 될수 있음.
 **/
void MakeHashKey(TCP_INFO *pTCPINFO, ONLINE_TSESS_KEY *pTSESSKEY)
{
	pTSESSKEY->uiCliIP = pTCPINFO->uiCliIP;
	pTSESSKEY->usCliPort = pTCPINFO->usCliPort;
	pTSESSKEY->usReserved = 0;	
}

S32 dGetCALLProcID(U32 uiClientIP)
{
	return SEQ_PROC_A_CALL + ( uiClientIP % gACALLCnt );
}

void UpCount(TCP_INFO *pTCPINFO, LOG_ONLINE_TRANS *pLOG)
{
	pLOG->uiIPDataUpPktCnt += pTCPINFO->uiIPDataUpPktCnt;
	pLOG->uiIPDataDnPktCnt += pTCPINFO->uiIPDataDnPktCnt;
	pLOG->uiIPTotUpPktCnt += pTCPINFO->uiIPTotUpPktCnt;
	pLOG->uiIPTotDnPktCnt += pTCPINFO->uiIPTotDnPktCnt;
	pLOG->uiIPDataUpRetransCnt += pTCPINFO->uiIPDataUpRetransCnt;
	pLOG->uiIPDataDnRetransCnt += pTCPINFO->uiIPDataDnRetransCnt;
	pLOG->uiIPTotUpRetransCnt += pTCPINFO->uiIPTotUpRetransCnt;
	pLOG->uiIPTotDnRetransCnt += pTCPINFO->uiIPTotDnRetransCnt;
	pLOG->uiIPDataUpPktSize += pTCPINFO->uiIPDataUpPktSize;
	pLOG->uiIPDataDnPktSize += pTCPINFO->uiIPDataDnPktSize;
	pLOG->uiIPTotUpPktSize += pTCPINFO->uiIPTotUpPktSize;
	pLOG->uiIPTotDnPktSize += pTCPINFO->uiIPTotDnPktSize;
	pLOG->uiIPDataUpRetransSize += pTCPINFO->uiIPDataUpRetransSize;
	pLOG->uiIPDataDnRetransSize += pTCPINFO->uiIPDataDnRetransSize;
	pLOG->uiIPTotUpRetransSize += pTCPINFO->uiIPTotUpRetransSize;
	pLOG->uiIPTotDnRetransSize += pTCPINFO->uiIPTotDnRetransSize;
}

S64 GetGapTime(STIME endtime, MTIME endmtime, STIME starttime, MTIME startmtime)
{
	return (((S64)endtime * 1000000 + (S64)endmtime) - ((S64)starttime * 1000000 + (S64)startmtime));
}

U16 GetFailCode(LOG_ONLINE_TRANS *pLOG, S32 failcode)
{
	if(pLOG->usUserErrorCode == ONLINE_UERR_EMPTY) {
		if(failcode > TCP_NOERR_FIN_E2) pLOG->usUserErrorCode = ONLINE_UERR_900;
	}

	return pLOG->usUserErrorCode;
}

U8 *PrintRtx(U8 ucRtxType)
{
	return (U8*)((ucRtxType == DEF_FROM_SERVER) ? "FROM_SERVER" : "FROM_CLIENT");
}

#if 0
S32 GetOnlineType(S32 l4code)
{
	S32		type;

	switch(l4code)
	{
	case L4_BREW_MACS_COMMON:
	case L4_BREW_MACS_VOD:
	case L4_BREW_MACS_PAID:
		type = ONLINE_MACS;
		break;		
	case L4_WIPI_WIGCS_COMMON:
	case L4_WIPI_WICGS_VOD:
	case L4_WIPI_WICGS_PAID:
	case L4_WIPI_WICGS_POPUP:
		type = ONLINE_WICGS;
		break;
	default:
		type = 0;
		break;
	}

	return type;
}
#endif

S32 GetOnlineType(S32 l4code)
{
	S32		type;

	switch(l4code)
	{
	case L4_WIPI_ONLINE:
		type = ONLINE_WICGS;
		break;
	default:
		type = 0;
		break;
	}

	return type;
}

S32 GetSvcType(ONLINE_TSESS *pTSESS, TCP_INFO *pTCPINFO)
{
	S32		svcType;

    if(pTCPINFO->ucRtx == pTSESS->ucSynRtx) {
        svcType = (pTSESS->type == ONLINE_WICGS) ? ONLINE_WICGS_BILL : ONLINE_MACS_BILL;
    } else {
        svcType = (pTSESS->type == ONLINE_WICGS) ? ONLINE_WICGS_HDR : ONLINE_MACS_HDR;
    }
	
	return svcType;
}

S32 CheckWicgsBillComHeader(void *input)
{
	U8		*HandsetMIN;
	S32		PacketLength;
	S64		llMin;
	S32		ClassID;
	st_WicgsBillCom		*p;

	p = (st_WicgsBillCom *)input;

	HandsetMIN = p->szMIN;
	HandsetMIN[15] = 0x00;
	PacketLength = ntohl(p->dPacketLen);

	p->szAppID[8] = 0x00;
	ClassID = strtol((char*)p->szAppID, NULL, 16);

// log_print(LOGN_CRI, "WICGS MIN[%s] CLASSID[%d] LEN[%d]", HandsetMIN,  ClassID, PacketLength);

	if(HandsetMIN[0] == '0') 
	{
		if(HandsetMIN[1] != '1')
		{
    		log_print(LOGN_CRI, "WICGS STRANGE 1 MIN[%s]", HandsetMIN);
			return ONLINE_UERR_113;
		}
	} 
	else if(HandsetMIN[0] == '1') 
	{
		if(HandsetMIN[1] != '0' && HandsetMIN[1] != '6' &&
			HandsetMIN[1] != '8' && HandsetMIN[1] != '1' &&
			HandsetMIN[1] != '7' && HandsetMIN[1] != '9') 
		{
    		log_print(LOGN_CRI, "WICGS STRANGE 2 MIN[%s]", HandsetMIN);
			return ONLINE_UERR_113;
		}
	} else {
    	log_print(LOGN_CRI, "WICGS STRANGE 3 MIN[%s]", HandsetMIN);
		return ONLINE_UERR_113;
	}

	llMin = atoll((char*)HandsetMIN);
	
	if(llMin <= MIN_MIN_NUM || llMin > MAX_MIN_NUM)
	{
    	log_print(LOGN_CRI, "WICGS STRANGE 4 MIN[%s]", HandsetMIN);
		return ONLINE_UERR_113;
	}

/*
	if((ClassID >= MIN_APP_ID) && (ClassID <= MAX_APP_ID)) 
	{
*/
		if(PacketLength <= 0) 
		{
			log_print(LOGN_CRI, "WICGS STRANGE LENGTH[%d]", PacketLength); 
			return ONLINE_UERR_105;

		} 
		else 
		{
    		return ONLINE_UERR_EMPTY;
		}
/*
	}

    log_print(LOGN_CRI, "WICGS STRANGE ClassID[%d]", ClassID);

    return 0;	
*/
}

S32 CheckWicgsServerHeader(void *input)
{
	st_Wicgs	*p = (st_Wicgs *)input;

	if(ntohl(p->dSignature1) != ONLINE_MAGIC_CODE)
	{
    	log_print(LOGN_CRI, "WICGS STRANGE MAGIC CODE[%d]:[%d]", ntohl(p->dSignature1), ONLINE_MAGIC_CODE);
		return ONLINE_UERR_102;
	}
	else
		return ONLINE_UERR_EMPTY;
}

S32 CheckMacsBillComHeader(void *input)
{
	U8		*HandsetMIN;
	S32		ClassID;
	S32		PacketLength;
	S64		llMin;
	st_MacsBillCom		*p;

	p = (st_MacsBillCom *)input;

	HandsetMIN = p->szMIN;
	HandsetMIN[15] = 0x00;
	ClassID = ntohl(p->dClassID);
	PacketLength = ntohl(p->dPacketLen);

// log_print(LOGN_CRI, "MACS MIN[%s] CLASSID[%d] LEN[%d]", HandsetMIN,  ClassID, PacketLength);

	if(HandsetMIN[0] == '0') 
	{
		if(HandsetMIN[1] != '1')
		{
    		log_print(LOGN_CRI, "MACS STRANGE 1 MIN[%s]", HandsetMIN);
			return ONLINE_UERR_113;
		}
	} 
	else if(HandsetMIN[0] == '1') 
	{
		if(HandsetMIN[1] != '0' && HandsetMIN[1] != '6' &&
			HandsetMIN[1] != '8' && HandsetMIN[1] != '1' &&
			HandsetMIN[1] != '7' && HandsetMIN[1] != '9') 
		{
    		log_print(LOGN_CRI, "MACS STRANGE 2 MIN[%s]", HandsetMIN);
			return ONLINE_UERR_113;
		}
	} else {
    	log_print(LOGN_CRI, "MACS STRANGE 3 MIN[%s]", HandsetMIN);
		return ONLINE_UERR_113;
	}

	llMin = atoll((char*)HandsetMIN);
	
	if(llMin <= MIN_MIN_NUM || llMin > MAX_MIN_NUM)
	{
    	log_print(LOGN_CRI, "MACS STRANGE 4 MIN[%s]", HandsetMIN);
		return ONLINE_UERR_113;
	}

/*
	if((ClassID >= MIN_APP_ID) && (ClassID <= MAX_APP_ID)) 
	{
*/
		if(PacketLength <= 0) 
		{
			log_print(LOGN_CRI, "MACS STRANGE LENGTH[%d]", PacketLength); 
			return ONLINE_UERR_105;

		} else 
		{
    		return ONLINE_UERR_EMPTY;
		}
/*
	}

    log_print(LOGN_CRI, "MACS STRANGE ClassID[%d]", ClassID);

    return 0;	
*/
}

S32 CheckMacsServerHeader(void *input)
{
	st_Macs	*p = (st_Macs *)input;

	if(ntohl(p->dSignature1) != ONLINE_MAGIC_CODE)
	{
    	log_print(LOGN_CRI, "MACS STRANGE MAGIC CODE[%d]:[%d]", ntohl(p->dSignature1), ONLINE_MAGIC_CODE);
		return ONLINE_UERR_102;
	}
	else
		return ONLINE_UERR_EMPTY;
}

/*
 * $Log: online_util.c,v $
 * Revision 1.3  2011/09/07 06:30:48  hhbaek
 * *** empty log message ***
 *
 * Revision 1.2  2011/09/05 08:20:24  dhkim
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * NEW OAM SYSTEM
 *
 * Revision 1.3  2011/08/17 13:06:40  hhbaek
 * A_ONLINE
 *
 * Revision 1.2  2011/08/09 08:17:40  uamyd
 * add blocks
 *
 * Revision 1.1.1.1  2011/08/05 00:27:18  uamyd
 * init DQMS2
 *
 * Revision 1.3  2011/05/09 14:00:35  dark264sh
 * A_ONLINE: A_CALL multi 처리
 *
 * Revision 1.2  2011/01/11 04:09:09  uamyd
 * modified
 *
 * Revision 1.1.1.1  2010/08/23 01:12:58  uamyd
 * DQMS With TOTMON, 2nd-import
 *
 * Revision 1.1.1.1  2009/05/26 02:14:42  dqms
 * Init TAF_RPPI
 *
 * Revision 1.3  2008/11/24 12:46:18  dark264sh
 * *** empty log message ***
 *
 * Revision 1.2  2008/11/24 07:04:24  dark264sh
 * WIPI ONLINE 처리
 *
 * Revision 1.1.1.1  2008/06/09 08:17:16  jsyoon
 * WATAS3 PROJECT START
 *
 * Revision 1.1  2007/08/21 12:53:54  dark264sh
 * no message
 *
 * Revision 1.5  2006/12/05 08:22:43  dark264sh
 * *** empty log message ***
 *
 * Revision 1.4  2006/12/04 08:19:35  dark264sh
 * *** empty log message ***
 *
 * Revision 1.3  2006/12/04 08:03:52  dark264sh
 * *** empty log message ***
 *
 * Revision 1.2  2006/11/28 12:58:27  cjlee
 * doxygen
 *
 * Revision 1.1  2006/10/27 12:35:51  dark264sh
 * *** empty log message ***
 *
 */
