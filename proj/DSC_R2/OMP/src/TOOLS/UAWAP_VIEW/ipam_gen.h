/**********************************************************
                 ABLEX Gen-Memory DBMS

   Author   : Hwang Woo-Hyoung
   Section  : IPAS Project
   SCCS ID  : %W%
   Date     : %G%
   Revision History :
        '01.  7. 29     Initial

   Description:
		IPAM GEN MEM STRUCTURE
   Copyright (c) ABLEX 2001
***********************************************************/

#ifndef __IPAM_GEN__
#define __IPAM_GEN__

#include 	"ipaf_svc.h"

#define		MAX_MSG_COUNT		32	/* 20040507, sunny : 16 -> 24 , 2004.1207 lander 24 -> 32 */
#define		MAX_SYSTEM_COUNT	32
#define		MAX_MD5_COUNT		8
#define		MAX_RECNT_COUNT		8
#define		MAX_LOC_SIZE		24

#define 	DEF_SVCMODE_ON		1
#define		DEF_SVCMODE_OFF		2

typedef struct _st_SESSMNG
{
    int         dCheckCount;
    int         dCheckPeriod;
    int         dTimeVal;		// CHECK TIMER QUD
	UCHAR		ucCateDefault;	// 1 ~ 100 : DEFAULT SERVICE CATEGORY ID 255 	
    UCHAR       ucSVCMode; 		// 1 : ACT DSCP, 2 : DACT DSCP
	UCHAR		ucReservedp[2];
#if 1	/* 20040320,poopee */
	int			dCallClear;		// call clear limit
	int			dLongCall;		// long call limit
#endif
	/* R2.3.0 Add 2004.1108 (lander) ---> */
	int			dHoldTime;
	int			dReserved;
	/* <--- */
} st_SESSMNG, *pst_SESSMNG;


// TIMEOUT FOR MESSAGE
typedef struct _st_MsgTimeOut
{
	USHORT		usTimeOut[MAX_MSG_COUNT]; 	// 0->ACC_B, 1->ACC_E, 2->ACC_IN, 3->ACC_OFF, 4->QUD, 5->ACC_AAA,
											// 6->PPS, 7->CDR
}st_MsgTimeOut, *pst_MsgTimeOut;

// MD5 CHECK FLAG
typedef struct _st_MD5Check
{
	UCHAR		ucMD5Check[MAX_MD5_COUNT];	//0->PDSN, 1->IWF, 2->AAA sunny add : 3-> GGSN, 4->WAG

}st_MD5Check, *pst_MD5Check;

// retry count
typedef struct _st_RetryCnt
{
	UCHAR		ucRetryCnt[MAX_RECNT_COUNT];	//0->PDSN, 1->IWF, 2->AAA
	UINT		uiAAATimeOutCnt;
	UINT		uiAAACheckCnt;
}st_RetryCnt, *pst_RetryCnt;

// LOAD RATE
typedef struct _st_SessLoad
{
	USHORT		usRate;
	USHORT		usMinTrsMsg;
}st_SessLoad, *pst_SessLoad;

typedef struct _st_IFLoad
{
	USHORT		usCri;
	USHORT		usMaj;
	USHORT		usMin;
	USHORT		usReserved;
}st_IFLoad, *pst_IFLoad;

// LOG LEVEL FOR PROCESS
typedef struct _st_LogLevel
{
	UCHAR		ucLogLevel[MAX_SYSTEM_COUNT];	// SEQ_PROC_## NUMBER
}st_LogLevel, *pst_LogLevel;

// ETC MEMORY
typedef struct _st_IPAMGenInfo
{
//	char				szLocation[MAX_LOC_SIZE];
	st_MsgTimeOut		stMsgTimeOut;
	st_MD5Check			stMD5Check;
	st_SessLoad			stSessLoad[MAX_MSG_COUNT]; // MSG-INDEX
	st_LogLevel			stLogLevel;
	st_LogLevel			stHeadLogLevel;			// 20040609, sunny : add HeadLog Level
	st_RetryCnt			stRetryCnt;
	st_SESSMNG			stSESSMNG;
	st_IFLoad			stIFLoad[MAX_SYSTEM_COUNT];
}st_IPAMGenInfo, *pst_IPAMGenInfo;

#define MAX_SYSNAME_SIZE		15

typedef struct _st_MD5Info
{
	char		szSysName[MAX_SYSNAME_SIZE];
	UCHAR		ucFlag; // 0->not check, 1->check
}st_MD5Info, *pst_MD5Info;

typedef struct _st_MD5InfoList
{
	int			dCount; //MAX->3
	int			dReserved;
	st_MD5Info	stMD5Info[MAX_MD5_COUNT];
}st_MD5InfoList, *pst_MD5InfoList;

typedef struct _st_LogLevelInfo
{
	int			dProID;
	int			dLogLevel;
}st_LogLevelInfo, *pst_LogLevelInfo;

typedef struct _st_LogLevelList
{
	int				dCount;
	int				dReserved;
	st_LogLevelInfo	stLogLevelInfo[MAX_SYSTEM_COUNT];
}st_LogLevelList, *pst_LogLevelList;

typedef struct _st_SessLoadInfo
{
	int			dMsgID;
	USHORT		usRate;
	USHORT		usMinTrsCnt;
}st_SessLoadInfo, *pst_SessLoadInfo;

typedef struct _st_SessLoadInfoList
{
	int				dCount;
	int				dReserved;
	st_SessLoadInfo	stSessLoadInfo[MAX_MSG_COUNT];
}st_SessLoadInfoList, *pst_SessLoadInfoList;

typedef struct _st_MsgTimInfo
{
	int         dMsgID;
	USHORT		usTimOut;			
	USHORT		usReserved;
}st_MsgTimInfo, *pst_MsgTimInfo;

typedef struct _st_MsgTimInfoList
{
	int				dCount;
	int				dReserved;
	st_MsgTimInfo	stMsgTimInfo[MAX_MSG_COUNT];	
}st_MsgTimInfoList, *pst_MsgTimInfoList;

#endif
