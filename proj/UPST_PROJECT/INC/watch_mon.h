#ifndef __WATCH_MON_H__
#define __WATCH_MON_H__

#include <time.h>

//#include <common_stg.h>
#include "typedef.h"


#define MAX_MON_SEC_CNT			3
#define MAX_MON_FA_CNT			1
#define MAX_MON_PCF_CNT			1300		 
#define MAX_MON_BTS_CNT			11000					// old 9000
#define MAX_MON_BSC_CNT			600						// old 400
#define MAX_MON_SVC_CNT			3000 					/*  1600 -> 3000 <2011.07.02> */
#define MAX_MON_PDSN_CNT		40
#define MAX_MON_AAA_CNT			32
#define MAX_MON_HSS_CNT			32						// 8
#define MAX_MON_LNS_CNT			8
#define MAX_MON_FIRST_CNT		66
/*TODO : jhbaek */
#define MAX_MON_PDIF_CNT		8						/* PDIF*/

/* System Type */
#define SYSTEM_TYPE_SECTOR		1
#define SYSTEM_TYPE_FA			2
#define SYSTEM_TYPE_BTS			3
#define SYSTEM_TYPE_BSC			4
#define SYSTEM_TYPE_PCF			5
#define SYSTEM_TYPE_PDSN		6
#define SYSTEM_TYPE_AAA			7
#define SYSTEM_TYPE_HSS			8
#define SYSTEM_TYPE_LNS			9
#define SYSTEM_TYPE_SERVICE		10
#define SYSTEM_TYPE_ROAMAAA		11
/*TODO : jhbaek */
#define SYSTEM_TYPE_PDIF		12
#define MAX_SYSTEM_TYPE_IDX		12

/* Office Index */
#define OFFICE_IDX_GS			1						/* 가산 교환 국사 */
#define OFFICE_IDX_SA			2						/* 상암 교환 국사 */
#define OFFICE_IDX_JA			3						/* 중앙 교환 국사 */
#define OFFICE_IDX_IC			4						/* 인천 교환 국사 */
#define OFFICE_IDX_SW			5						/* 수원 교환 국사 */
#define OFFICE_IDX_WJ			6						/* 원주 교환 국사 */
#define OFFICE_IDX_BS			7						/* 부산 교환 국사 */
#define OFFICE_IDX_DG			8						/* 대구 교환 국사 */
#define OFFICE_IDX_GJ			9						/* 광주 교환 국사 */
#define OFFICE_IDX_DJ			10						/* 대전 교환 국사 */
#define MAX_MON_OFFICE_IDX		10
#define OFFICE_UNKNOWN			OFFICE_IDX_GS

/* Service Index */
#define SVC_IDX_MENU			0						/* MENU 서비스 */
#define SVC_IDX_DN				1						/* DOWNLOAD 서비스 */
#define SVC_IDX_STREAM			2						/* STREAM 서비스 */
#define SVC_IDX_MMS				3						/* MMS 서비스 */
#define SVC_IDX_WIDGET			4						/* WIDGET 서비스 */
#define SVC_IDX_PHONE			5						/* PHONE 서비스 */
#define SVC_IDX_EMS				6						/* EMS 서비스 */
#define SVC_IDX_BANK			7						/* BANK 서비스 */
#define SVC_IDX_FV				8						/* FV 서비스 */
#define SVC_IDX_IM				9						/* IM 서비스 */
#define SVC_IDX_VT				10						/* VT 서비스 */
#define SVC_IDX_ETC				11						/* ETC 서비스 */
#define SVC_IDX_CORP			12						/* CORP 서비스 */
#define SVC_IDX_REGI			13						/* REGI 서비스 */
#define SVC_IDX_INET			14						/* 인터넷 서비스 */
#define SVC_IDX_RECVCALL		15						/* 인터넷 착신 서비스 */
#define SVC_IDX_IM_RECV			16						/* IM 착신 서비스 */
#define SVC_IDX_VT_RECV			17						/* VT 착신 서비스 */
#define SVC_IDX_ROAM			18						/* ROAM 서비스 */
#define CURR_MON_SVC_IDX		19
#define MAX_MON_SVC_IDX			20

#define MAX_MON_DEFECT_IDX		300

/* ALARM TYPE */
#define MON_ALARMTYPE_NORMAL	0
#define MON_ALARMTYPE_RATE		1
#define MON_ALARMTYPE_MIN		2
#define MON_ALARMTYPE_MAX		3

typedef struct _st_SubAlarmSysStatus
{
	U8					ucCall:2;
	U8					ucAAA:2;
	U8					ucHSS:2;
	U8					ucLNS:2;
	U8					ucReCall:2;						/* 착신 호 알람 */
	U8					ucReserved:6;
} st_SubAlarmSysStatus, *pst_SubAlarmSysStatus;

typedef struct _st_SubAlarmSvcStatus
{
	U8					ucMENU:2;
	U8					ucDN:2;
	U8					ucSTREAM:2;
	U8					ucMMS:2;
	U8					ucWIDGET:2;
	U8					ucPHONE:2;
	U8					ucEMS:2;
	U8					ucBANK:2;
	U8					ucFV:2;
	U8					ucIM:2;
	U8					ucVT:2;
	U8					ucETC:2;
	U8					ucCORP:2;
	U8					ucREGI:2;
	U8					ucINET:2;
	U8					ucINET_RECV:2;
	U8					ucIM_RECV:2;
	U8					ucVT_RECV:2;
	U8					ucReserved:4;
} st_SubAlarmSvcStatus, *pst_SubAlarmSvcStatus;

typedef struct _st_MonAlarm
{
	U8					ucAlarm;						/* 전체 ALARM 상태 0:NORMAL 1:ALARM */
	U8					ucAlarmSysStatus[2];			/* 호 처리 ALARM 상태 */
	U8					szAlarmSvcStatus[5];			/* Service ALARM 상태 */ 
	U16					usAlarmSvcStatus[MAX_MON_SVC_IDX];
} st_MonAlarm, *pst_MonAlarm;
#define DEF_MONALARM_SIZE		sizeof(st_MonAlarm)

typedef struct _st_MonInfo
{
	U32					uiCall[2];						/* Call 시도수 / Call 접속 성공수 */
	U32					uiAAA[2];						/* AAA 시도수 / AAA 성공수 */
	U32					uiHSS[2];						/* HSS 시도수 / HSS 성공수 */
	U32					uiLNS[2];						/* LNS 시도수 / LNS 성공수 */
	U32					uiReCall[2];					/* 착신 호 시도수 / 착신 호 성공수 */
	U32					uiService[MAX_MON_SVC_IDX][2];	/* 서비스 시도수 / 성공수 */
} st_MonInfo, *pst_MonInfo;
#define DEF_MONINFO_SIZE		sizeof(st_MonInfo)

typedef struct _st_Defect
{
	U32					uiFail[MAX_MON_DEFECT_IDX];		/* 해당 DEFECT CODE의 발생수 */
} st_Defect, *pst_Defect;
#define DEF_DEFECT_SIZE			sizeof(st_Defect)

typedef struct _st_MonSvc
{
	U8					ucSvcType;						/* Service Index */
	U8					ucReserved;
	U16					SvcL4Type;						/* Svc L4 Type */
	U32					uiIPAddr;						/* IP Address */
	st_MonAlarm			stMonAlarm;						/* ALARM 상태 */
	st_MonInfo			stMonInfo;						/* Monitoring 정보 */
	st_Defect			stDefect;						/* Defect 정보 */
} st_MonSvc, *pst_MonSvc;
#define DEF_MONSVC_SIZE			sizeof(st_MonSvc)

typedef struct _st_SubBSC
{
	U8					ucOffice;						/* 교환 국사 */
	U8					ucSYSID;						/* BSC ID */
	U8					ucBSCID;						/* BSC ID */
	U8					ucReserved;
} st_SubBSC, *pst_SubBSC;
#define DEF_SUBBSC_SIZE			sizeof(st_SubBSC)

typedef struct _st_MonBSC
{
	U32					uiBSC;							/* 교환 국사 */
	st_MonAlarm			stMonAlarm;						/* ALARM 상태 */
	st_MonInfo			stMonInfo;						/* Monitoring 정보 */
	st_Defect			stDefect;						/* Defect 정보 */
} st_MonBSC, *pst_MonBSC;
#define DEF_MONBSC_SIZE			sizeof(st_MonBSC)

typedef struct _st_DefectS
{
	U16					usFail[MAX_MON_DEFECT_IDX];		/* 해당 DEFECT CODE의 발생수 */
} st_DefectS, *pst_DefectS;
#define DEF_DEFECTS_SIZE		sizeof(st_DefectS)

typedef struct _st_MonInfoS
{
	U16					usCall[2];						/* Call 시도수 / 접속 성공수 */
	U16					usAAA[2];						/* AAA 시도수 / AAA 성공수 */
	U16					usHSS[2];						/* HSS 시도수 / HSS 성공수 */
	U16					usLNS[2];						/* LNS 시도수 / LNS 성공수 */
	U16					usReCall[2];					/* 착신 호 시도수 / 착신 호 성공수 */
	U16					usService[MAX_MON_SVC_IDX][2];	/* 서비스 시도수 / 성공수 */
} st_MonInfoS, *pst_MonInfoS;
#define DEF_MONINFOS_SIZE		sizeof(st_MonInfoS)

typedef struct _st_MonSec
{
	U8					ucSec;							/* Sector ID */
	U8					ucReserved[3];
	st_MonAlarm			stMonAlarm;						/* ALARM 정보 */
	st_MonInfoS			stMonInfoS;						/* Monitoring 정보 */
	st_DefectS			stDefectS;						/* Defect 정보 */
} st_MonSec, *pst_MonSec;
#define DEF_MONSEC_SIZE			sizeof(st_MonSec)

typedef struct _st_MonFA
{
	U8					ucFA;							/* FA ID */
	U8					ucReserved[3];				
	st_MonAlarm			stMonAlarm;						/* ALARM 상태 */
	st_MonInfoS			stMonInfoS;						/* Monitoring 정보 */
	st_DefectS			stDefectS;						/* Defect 정보 */
	st_MonSec			stMonSec[MAX_MON_SEC_CNT];		/* Sector 상태 정보 */
} st_MonFA, *pst_MonFA;
#define DEF_MONFA_SIZE			sizeof(st_MonFA)

typedef struct _st_SubBTS
{
	U8					ucOffice;						/* 교환 국사 */
	U8					ucSYSID;						/* 교환 국사 */
	U8					ucBSCID;						/* BSC ID */
	U8					ucReserved;
	U16					usBTSID;						/* BTS ID */
	U8					szReserved[2];
} st_SubBTS, *pst_SubBTS;
#define DEF_SUBBTS_SIZE			sizeof(st_SubBTS)

typedef struct _st_MonBTS
{
	U64					ullBTS;							/* 교환 국사 : st_SubBTS casting */
	st_MonAlarm			stMonAlarm;						/* ALARM 상태 */
	st_MonInfo			stMonInfo;						/* Monitoring 정보 */
	st_Defect			stDefect;						/* Defect 정보 */
	st_MonFA			stMonFA[MAX_MON_FA_CNT];		/* FA 상태 정보 */
} st_MonBTS, *pst_MonBTS;
#define DEF_MONBTS_SIZE			sizeof(st_MonBTS)

typedef struct _st_FirstMon
{
	U8					ucOffice;						/* 지역국사 ID */
	U8					ucSysType;						/* System Type 1:SECTOR 2:FA 3:BTS 4:BSC/CAN/ALLIP/SEC 5:PCF 10:SERVICE */
	U16					usSubType;						/* IF ucSysType = 10 THEN Service Index ELSE 0 */
	st_MonAlarm			stMonAlarm;						/* Alarm 상태 */
	st_MonInfo			stMonInfo;						/* Monitoring 정보 */
	st_Defect			stDefect;						/* Defect 정보 */
} st_FirstMon, *pst_FirstMon;
#define DEF_FIRSTMON_SIZE		sizeof(st_FirstMon)

typedef struct _st_MonCore
{
	U8					ucOffice;						/* 지역국사 ID */
	U8					ucSysType;						/* System Type 5:PCF 6:PDSN 7:AAA 8:HSS 9:LNS 11:ROAM_AAA */
	U8					szReserved[2];
	U32					uiIPAddr;						/* IP Address */
	st_MonAlarm			stMonAlarm;						/* Alarm 상태 */
	st_MonInfo			stMonInfo;						/* Monitoring 정보 */
	st_Defect			stDefect;						/* Defect 정보 */
} st_MonCore, *pst_MonCore;
#define DEF_MONCORE_SIZE		sizeof(st_MonCore)

typedef struct _st_FirstMonList
{
	time_t				lTime;							/* Monitoring Time (5분 단위 time_t) */
	U16					usPDSNListCnt;					/* PDSN List Count */
	U16					usAAAListCnt;					/* AAA List Count */
	U16					usHSSListCnt;					/* HSS List Count */
	U16					usLNSListCnt;					/* LNS List Count */
	U16					usFirstListCnt;					/* 초기화면 Memory Count */
	U16					usReserved;
	st_MonCore			stPDSN[MAX_MON_PDSN_CNT];		/* PDSN 정보 */
	st_MonCore			stAAA[MAX_MON_AAA_CNT];			/* AAA 정보 */
	st_MonCore			stHSS[MAX_MON_HSS_CNT];			/* HSS 정보 */
	st_MonCore			stLNS[MAX_MON_LNS_CNT];			/* LNS 정보 */
	st_FirstMon			stFirstMon[MAX_MON_FIRST_CNT];	/* 초기화면 정보 */
} st_FirstMonList, *pst_FirstMonList;
#define DEF_FIRSTMONLIST_SIZE	sizeof(st_FirstMonList)

typedef struct _st_MonList
{
	time_t				lTime;							/* Monitoring Time (5분 단위 time_t) */
	U16					usPCFCnt;						/* PCF Memory Count */
	U16					usBTSCnt;						/* BTS Memory Count */
	U16					usSvcCnt;						/* SVC Memory Count */
	U8					reserved[2];
	st_FirstMonList		stFirstMonList;					/* 초기화면 품질 감시 정보 */
	st_MonCore			stMonPCF[MAX_MON_PCF_CNT];		/* PCF 정보 */
	st_MonBTS			stMonBTS[MAX_MON_BTS_CNT];		/* BTS/FA/SEC 정보 */
	st_MonSvc			stMonSvc[MAX_MON_SVC_CNT];		/* 서비스 정보 */
} st_MonList, *pst_MonList;
#define DEF_MONLIST_SIZE		sizeof(st_MonList)


typedef struct _st_MonList_1Min
{
    time_t              lTime;                          /* Monitoring Time (5분 단위 time_t) */
    U32                 uiReserved;
    U16                 usBSCCnt;                       /* BSC Memory Count */
    st_FirstMonList     stFirstMonList;                 /* 초기화면 품질 감시 정보 */
    st_MonBSC           stMonBSC[MAX_MON_BSC_CNT];      /* BSC 정보 */
} st_MonList_1Min, *pst_MonList_1Min;
#define DEF_MONLIST_1MIN_SIZE        sizeof(st_MonList_1Min)

#define DEF_MON_PERIOD			(60 * 5)
#define USED_MONLIST_CNT		(24 * 12)
#define TOTAL_MONLIST_CNT		(USED_MONLIST_CNT + 1)
typedef struct _st_MonTotal
{
	int					dCurIdx;
	int					dUsedCnt;
	int					dID;
	st_MonList			stMonList[TOTAL_MONLIST_CNT];
} st_MonTotal, *pst_MonTotal;
#define DEF_MONTOTAL_SIZE		sizeof(st_MonTotal)

#define DEF_MON_PERIOD_1MIN          (60 * 1)
#define USED_MONLIST_1MIN_CNT        (24 * 60)
#define TOTAL_MONLIST_1MIN_CNT       (USED_MONLIST_1MIN_CNT + 1)
typedef struct _st_MonTotal_1Min
{
    int                 dCurIdx;
    int                 dUsedCnt;
    int                 dID;
    st_MonList_1Min     stMonList1Min[TOTAL_MONLIST_1MIN_CNT];
} st_MonTotal_1Min, *pst_MonTotal_1Min;
#define DEF_MONTOTAL_1MIN_SIZE       sizeof(st_MonTotal_1Min)


/* watch msg type */
#define WATCH_TYPE_A11			1
#define WATCH_TYPE_AAA			2
#define WATCH_TYPE_HSS			3
#define WATCH_TYPE_LNS			4
#define WATCH_TYPE_SVC			5
#define WATCH_TYPE_A11AAA		6
#define WATCH_TYPE_RECALL		7 						/* 착신 호 타입  */

#define UNKNOWN_TYPE_PCF		0x01
#define UNKNOWN_TYPE_BSC		0x02
#define UNKNOWN_TYPE_BTS		0x04
typedef struct _st_WatchMsg
{
//	time_t				lTime;
	U16					usMsgType;						/* 1:A11 2:AAA 3:HSS 4:LNS 5:SVC */
	U8					ucOffice;
	U8					ucSYSID;
	U8					ucRoamFlag;						/* 1:ROAM */
	U8					ucBSCID;
	U16					usBTSID;
	U8					ucSec;
	U8					ucFA;
	U16					usSvcL4Type;
	U8					ucPCFType;
	U8					ucSvcIdx;
	U8					ucUnknownType;
	U8					ucReserved;
	U32					uiPCFIP;
	U32					uiPDSNIP;
	U32					uiAAAIP;
	U32					uiHSSIP;
	U32					uiLNSIP;
	U32					uiSVCIP;
	U32					uiResult;
} st_WatchMsg, *pst_WatchMsg;
#define DEF_WATCHMSG_SIZE		sizeof(st_WatchMsg)

typedef struct _st_SvcMonMsg
{
	time_t				lTime;
	U32					uiIdx;
} st_SvcMonMsg, *pst_SvcMonMsg;
#define DEF_SVCMONMSG_SIZE		sizeof(st_SvcMonMsg)

#endif
