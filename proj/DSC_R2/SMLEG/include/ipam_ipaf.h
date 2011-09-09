/*********************************************************
                 ABLEX IPAS Project (IPAM BLOCK)

   Author   : Hwang Woo-Hyoung
   Section  : IPAS(IPAM) Project
   SCCS ID  : @(#)ipam_svc.h (V1.0)
   Date     : 1/15/03
   Revision History :
        '03.    01. 15. initial

   Description:

   Copyright (c) ABLEX 2003
*********************************************************/

#ifndef _SVCPACKET_H
#define _SVCPACKET_H

#include <time.h>
#pragma pack(1)

#define DEF_SERVER      1
#define DEF_CLIENT      2

#define FATAL_ERROR     5
#define MAJOR_ERROR     4
#define MINOR_ERROR     3
#define WARNING         2
#define NOTIFICATION    1

#define MAX_STRING_LEN	100

/* Service ID Definition */
#define SID_IPAF_MANG	11 /* Service ID : IPAF Manage */
#define SID_ACCT_MANG	22 /* Service ID : Accounting Manage */
#define SID_SERV_MANG	31 /* Service ID : Service Category Manage */
#define SID_IPAF_CHG	41 /* Service ID : Service Category Manage */
/* Service ID Definition : ADD 04-03-19 */
#define SID_SESS_MANG   51 /* SESSION 관련 */

#define SID_LOG_MANG	71 /* SERVICE UD : LOG 관리 */

/* Message ID Definition */

/* Message ID : SID_IPAF_MANG 11 */
#define MID_IPAF_LOGIN	24 /* Message ID : IPAF Log-In */
#define MID_IPAF_LOGOUT	25 /* Message ID : IPAF Log-Out */
#define MID_IPAF_VERNO	26 /* Message ID : IPAF Version Info */
#define MID_IPAF_LINK	27 /* Message ID : IPAF Link Test */
#define MID_IPAF_ALMD	28 /* Message ID : IPAF Almd-Stat */
#define MID_IPAF_EVENT	29 /* Message ID : IPAF Event Msg */
#define MID_IPAF_IPPOOL	30 /* Message ID : IPAF PDSN/IWF IP Pool */
#define MID_IPAF_CHK	31 /* Message ID : IPAF LINK CHECK MSG --> HWH ADD 2006/01/18 */

/* Message ID : SID_ACCT_MANG 22 */
#define MID_ACCT_START	11 /* Message ID : Account Start */
#define MID_ACCT_END	12 /* Message ID : Account End */
#define MID_ACCT_CDR	13 /* Message ID : Account CDR */
#define MID_ACCT_REMAIN	14 /* Message ID : Account Remains End */
#define MID_ACCT_WATCH	15 /* Message ID : Call Watch */
#define MID_ACCT_PERIOD	16 /* Message ID : PREPAID CDR Report Period Update Req */
#define MID_ACCT_PPS	16 /* Message ID : PREPAID CDR Report Period Update Req */
#define MID_ACCT_PPS_CDR	20 /* Message ID : PREPAID CDR Report Period Update Req */
#define MID_ACCT_CDR2		21
#define MID_ACCT_PPS_CDR2	22
#define MID_ACCT_OZCDR		23

/* Message ID : SID_ACCT_MANG 22 -> NEW ADD */
#define MID_ACCT_MERDR    	17
#define MID_ACCT_KUNRDR		18
#define MID_ACCT_MARSRDR	19
#define MID_ACCT_ADSRDR		20
/* add by hwh 2004-11-23 */
#define MID_ACCT_MACSRDR	21
#define MID_ACCT_WICGSRDR	22
#define MID_ACCT_VODMRDR	23
#define MID_ACCT_VODDRDR	24
#define MID_ACCT_VODTCPRDR	25		/* RTSP RDR */
#define MID_ACCT_VODUDPRDR	26		/* RTP/RTCP RDR */
#define MID_ACCT_WAPRDR		27		
#define MID_ACCT_KVMRDR     28      /* KBM RDR */
#define MID_ACCT_VTRDR      29		/* VT RDR */
#define MID_ACCT_FBRDR		30		/* FB RDR */

/* Message ID : SID_ACCT_MANG 51 -> NEW ADD */
#define MID_SESS_DATA       	11	/* Req, Rep Data */
#define MID_SESS_STOP       	12 	/* TCP FIN으로 인한 TCP 세션 종료 */
#define MID_CALL_STOP       	13 	/* Account Stop으로 인한 호 종료 */
#define MID_CALL_STOP_ACK   	14 	/* Account Stop으로 인한 호 종료 처리 후 ACK */
#define MID_CALL_STOP_NOT_RDR	15	/* RDR 생성 없이 호 세션 정리 */
#define MID_CALL_STOP_SND_RDR	16	/* RDR 생성 한 후, 호 세션 정리 */
#define MID_SESS_RETR_DATA		17	/* 재전송 데이타 */
#define MID_SESS_STOP_ACK		18	/* FIN ACK : 데이터 섬 */ 
#define MID_SESS_UDP_DATA		19	/* UDP DATA PACKET : add by HWH 20041111*/
#define MID_SESS_VT_DATA		20  /* VTSVC DATA PACKET : added by LYH */

/* Message ID : SID_SERV_MANG 31 */
#define MID_SERV_INFO	21 /* Message ID : Service Category Info Req */

/* Message ID :  SID_LOG_MANG 71 */
#define MID_LOG_CDR		71		/* CDR CRITICAL LOG */
#define MID_LOG_CDR2 	72
#define MID_IPPOOL_RELOAD	41
#define MID_SERCAT_RELOAD	42

#define MAX_UNSENT_MSG_COUNT	130
#define MAX_CATEGORY_COUNT		200
#define MAX_STATIC_COUNT        50
#define MAX_STATIC_INFO_SIZE	13
#define MAX_SWINFO_SIZE			256
#define MAX_HWINFO_SIZE			256
#define MAX_MSG_SIZE			256
#define	MAX_SERVICEID_SIZE		4
#define MAX_PACKETAMOUNT_SIZE 	4

#define DEF_TYPE_POSTPAID       1
#define DEF_TYPE_PREPAID        2

/* 지능망 선불 가입자 정액 서비스 사용 */
#define DEF_TYPE_NOREMAIN       4

typedef struct _st_ReqLogIn
{
	UCHAR	ucIPAFID;
	UINT	uiIPAddr;
	UINT	uiSWVerNo;
	UCHAR	ucDownLoadType;
	USHORT	usUnSentMsgCount;
	INT64	llUnSentMsg[MAX_UNSENT_MSG_COUNT];
}st_ReqLogIn, *pst_ReqLogIn;

typedef struct _st_ReqLogOut
{
	UCHAR	ucIPAFID;
}st_ReqLogOut, *pst_ReqLogOut;

typedef struct _st_IPAFAlmd
{
	UCHAR	ucIPAFID;
	UCHAR	ucSWSize;
	UCHAR	szSWInfo[MAX_SWINFO_SIZE];
	UCHAR	ucHWSize;
	UCHAR	szHWInfo[MAX_HWINFO_SIZE];
	UCHAR	ucStaticsFlag;
	time_t	uiStaticsTime;
	UCHAR	ucStaticsCount;
	UCHAR	szStaticInfo[MAX_STATIC_COUNT][MAX_STATIC_INFO_SIZE];
}st_IPAFAlmd, *pst_IPAFAlmd;

typedef struct _st_IPAFEvent
{
	UCHAR	ucIPAFID;
	UINT	uiIPAFAddr;
	UCHAR	ucMsgType;
	UCHAR	usMsgSize;
	UCHAR	szMsg[MAX_MSG_SIZE];
}st_IPAFEvent, *pst_IPAFEvent;

typedef struct _st_CategoryUse
{
	UCHAR	szServiceID[MAX_SERVICEID_SIZE];
	UCHAR	szPacketAmount[MAX_PACKETAMOUNT_SIZE];
	USHORT	usPacketAmountTime;
}st_CategoryUse, *pst_CategoryUse;

typedef struct _st_CDRUse
{
	INT64			llCRNNum;
	INT64			llMINNum;
	UINT			uiSrcIPAddr;
	UCHAR			ucPayType;
	UINT			uiTotlUseAmount;
	UINT			uiTotlUseAmountTime;
	UINT			uiTotlRemainsAmount;
	UINT			uiTotlRemainsAmountTime;
	UCHAR			ucCategoryCount;
	st_CategoryUse	stCategoryUse[MAX_CATEGORY_COUNT];
}st_CDRUse, *pst_CDRUse;

typedef struct _st_RemainsEnd
{
	INT64			llCRNNum;
	INT64			llMINNum;
	UINT			uiSrcIPAddr;	
}st_RemainsEnd, *pst_RemainsEnd;

#pragma pack()
#endif
