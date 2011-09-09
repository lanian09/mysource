#ifndef __SVCMON_H__
#define __SVCMON_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <typedef.h>

#define ALARM_TYPE_CALL     1
#define ALARM_TYPE_RECALL   2 
#define ALARM_TYPE_AAA     	3 
#define ALARM_TYPE_HSS      4
#define ALARM_TYPE_LNS      5
#define ALARM_TYPE_SVC      6

#define FIRSTMON_TYPE_PDSN	10
#define FIRSTMON_TYPE_AAA	20
#define FIRSTMON_TYPE_HSS	30
#define FIRSTMON_TYPE_LNS	40

#define PCF_TYPE_1x			10
#define PCF_TYPE_EvDO		20

/* array type */
#define ARRAY_TYPE_CORE		1
#define ARRAY_TYPE_BTS		2
#define ARRAY_TYPE_BSC		3
#define ARRAY_TYPE_SVC		4
#define ARRAY_TYPE_FIRST	5

/* 서비스 알람 (우선순위 없이 알람 중복 체크를 위한 DEFINE) */
#define DEF_ALARMTYPE_MAX		0x0100
#define DEF_ALARMTYPE_RATE		0x0010
#define DEF_ALARMTYPE_MIN		0x0001


typedef struct _st_SvcMonHash_Key
{
	U8				ucOffice;
	U8				ucSysType;
	U16				usSubType;
	U32				uiIP;
	U8				ucSYSID;
	U8				ucBSCID;
	U16				usBTSID;
	U32				uiL4SvcType;
	U32				uiArrayType;		/* Array Type ARRAY_TYPE_CORE:st_MonCore, ARRAY_TYPE_BTS:st_MonBTS,
											ARRAY_TYPE_BSC:st_MonBSC, ARRAY_TYPE_SVC:st_MonSvc, ARRAY_TYPE_FIRST:st_FirstMon */
	U32				uiReserved;
} st_SvcMonHash_Key, *pstSvcMon_Key;
#define DEF_SVCMONHASH_KEY_SIZE			sizeof(st_SvcMonHash_Key)

typedef struct _st_SvcMonHash_Data
{
	U32				uiArrayIndex;		/* Array Index */
	//U32				uiReserved;
	U32				ui1MinMonFlag;
} st_SvcMonHash_Data, *pst_SvcMonHash_Data;
#define DEF_SVCMONHASH_DATA_SIZE		sizeof(st_SvcMonHash_Data)

#define MAX_SVCMONHASH_SIZE				30011

typedef struct _st_DefHash_Key
{
	U32				uiDefectCode;
} st_DefHash_Key, *pst_DefHash_Key;
#define DEF_DEFHASH_KEY_SIZE			sizeof(st_DefHash_Key)

typedef struct _st_DefHash_Data
{
	U32				uiArrayIndex;
} st_DefHash_Data, *pst_DefHash_Data;
#define DEF_DEFHASH_DATA_SIZE			sizeof(st_DefHash_Data)

#define MAX_DEFHASH_SIZE				211

#define DEF_THRESFLAG_DAY				10
#define DEF_THRESFLAG_NIGHT				20
typedef struct _st_ThresHash_Key
{
	U8				ucOffice;
	U8				ucSysType;
	U8				ucAlarmType;
	U8				ucReserved;
	UINT			uiIP;
} st_ThresHash_Key, *pst_ThresHash_Key;
#define DEF_THRESHASH_KEY_SIZE			sizeof(st_ThresHash_Key)

typedef struct _st_ThresHash_Data
{
	U8				ucDayFlag[24];;
	U8				ucDayTimeRate;
	U8				ucNightTimeRate;
	U8				ucReserved[2];
	U32				uiDayTimeMinTrial;
	U32				uiNightTimeMinTrial;
	U32				uiPeakTrial;
} st_ThresHash_Data, *pst_ThresHash_Data;
#define DEF_THRESHASH_DATA_SIZE			sizeof(st_ThresHash_Data)

#define MAX_THRESHASH_SIZE				5003

typedef struct _st_NasIPHash_Key
{
	U32				uiNasIP;
} st_NasIPHash_Key, *pst_NasIPHash_Key;
#define DEF_NASIPHASH_KEY_SIZE			sizeof(st_NasIPHash_Key)

typedef struct _st_NasIPHash_Data
{
	U32				uiNasIPPool;
} st_NasIPHash_Data, *pst_NasIPHash_Data;
#define DEF_NASIPHASH_DATA_SIZE			sizeof(st_NasIPHash_Data)

#define MAX_NASIPHASH_SIZE				200003


#endif
