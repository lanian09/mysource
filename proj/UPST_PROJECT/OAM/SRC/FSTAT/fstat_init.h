/**
	@file		fstat_init.h	
	@author		
	@version
	@date		
	@brief		fstat_init.c 헤더파일
*/

#ifndef __FSTAT_INIT_H__
#define __FSTAT_INIT_H__

/**
	Include headers
*/

/* SYS HEADER */
#include <mysql/mysql.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>			/* getpid(void) */

#include "mems.h"			/* getpid(void) */
#include "cifo.h"			/* getpid(void) */
#include "nsocklib.h"
#include "timerN.h"
#include "filedb.h"

#include "mmcdef.h"			/* getpid(void) */
#include "msgdef.h"			/* getpid(void) */

#include "almstat.h"			/* getpid(void) */
#if 0
/* LIB HEADER */
#include "nsocklib.h"
#include "nifo.h"
#include "gifo.h"
#include "cifo.h"
#include "mems.h"
#include "dblib.h"
#include "filelib.h"
#include "loglib.h"
#include "verlib.h"
#include "filedb.h"
#include "commdef.h"
#include "hashg.h"
#include "timerN.h"
/* PRO HEADER */
#include "path.h"
#include "mmcdef.h"
#include "msgdef.h"
#include "sshmid.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
#include "almstat.h"
/* LOC HEADER */
#endif


/**
	Define constants
*/
#define DEF_NTAM				1
#define DEF_NTAM_POS			0
#define MAX_SUBDESC_SIZE    	32
#define MAX_SUBSYS_NUM      	8
#define DEF_SGTAF_TYPE			4
#define MAX_STAY_CNT			4
#define DEF_FIRST_TIMEFLAG		0
#define MAX_SYS_CNT				9

/**
	Define structures
*/
typedef struct _st_LOAD
{
    unsigned int    uiStatTime;
    unsigned short  usSystemType;   /*  NTAM: 0, NTAF: 1    */
    unsigned short  usSystemID;
    unsigned int    usCPUAVG;
    unsigned int    usCPUMAX;
    unsigned int    usCPUMIN;
    unsigned int    usMEMAVG;
    unsigned int    usMEMMAX;
    unsigned int    usMEMMIN;
    unsigned int    usQUEAVG;
    unsigned int    usQUEMAX;
    unsigned int    usQUEMIN;
    unsigned int    usNIFOAVG;
    unsigned int    usNIFOMAX;
    unsigned int    usNIFOMIN;
    unsigned int    uTrafficAVG;
    unsigned int    uTrafficMAX;
    unsigned int    uTrafficMIN;
    unsigned int    usDISK1AVG;
    unsigned int    usDISK1MAX;
    unsigned int    usDISK1MIN;
    unsigned int    usDISK2AVG;
    unsigned int    usDISK2MAX;
    unsigned int    usDISK2MIN;
    unsigned int    usDISK3AVG;
    unsigned int    usDISK3MAX;
    unsigned int    usDISK3MIN;
    unsigned int    usDISK4AVG;
    unsigned int    usDISK4MAX;
    unsigned int    usDISK4MIN;

    unsigned short  usCount;
} st_LOAD;

typedef struct _st_LOADLIST
{
    unsigned int    uiArrCount;
    st_LOAD         stLOAD[MAX_SYS_CNT*2];
} st_LOADLIST;

typedef struct _st_FAULT
{
    unsigned int    uiStatTime;
    unsigned short  usSystemType;
    unsigned short  usSystemID;
    unsigned char   szType[9];
    unsigned int    uiCRI;
    unsigned int    uiMAJ;
    unsigned int    uiMIN;
    unsigned int    uiSTOP;
    unsigned int    uiNORMAL;
} st_FAULT;

typedef struct _st_FAULTLIST
{
#define MAX_FAULT_CNT       8
    unsigned int    uiArrCount;
    st_FAULT        stFAULT[MAX_SYS_CNT*MAX_FAULT_CNT*2];
}st_FAULTLIST;

typedef struct _st_BLOCK_Key
{
	short	usLoadFaultFlag;	/*	Load: 1 FAULT: 2	*/
	short	usgdTimeFlag;		/*	First: 0 Second: 1	*/
} BLOCK_KEY;

typedef struct _st_TRAFFIC
{
    time_t          uiStatTime;
    unsigned short  usTAFID;
    char            cReserved[6];

    st_NtafStat     ThruStat;

    st_NtafStat     TotStat;
    st_NtafStat     IPStat;
    st_NtafStat     UDPStat;
    st_NtafStat     TCPStat;
    st_NtafStat     SCTPStat;
    st_NtafStat     ETCStat;

    st_NtafStat     IPError;
    st_NtafStat     UTCPError;
    st_NtafStat     FailData;
    st_NtafStat     FilterOut;
} st_TRAFFIC;






int JiSTOPFlag;
int FinishFlag;

int gdSHMLOADID;
int gdSHMFAULTID;
int gdSHMTRAFFICID;

stMEMSINFO			*gpRECVMEMS;
stCIFO				*pCIFO;

unsigned long long timerNID;

st_SubSysInfoList   gstSubList;
st_LOADLIST         *pstSHMLOADTbl;
st_FAULTLIST        *pstSHMFAULTTbl;
short               gdTimeFlag;
time_t              tLocalTime;

stTIMERNINFO        *pTIMER;

/* MYSQL을 사용하기 위한 변수 */
char        		szIP[16], szName[32], szPass[32], szAlias[32];
st_ConnInfo			stConnInfo;
MYSQL				stMySQL;

/**
	Declare functions
*/
extern void SetUpSignal(void);
extern int dInit_ipcs(void);
extern void InitProc(void);
extern int dIsReceivedMessage(pst_MsgQ *pstMsgQ);
extern int dCheckNTAFLoadList(time_t tLocalTime, st_NTAF *stNTAF, short usNTAFID);
extern int dCheckNTAMLoadList(time_t tLocalTime, st_NTAM * stNTAM);
extern int dCheckFaultList(time_t tLocalTime, st_almsts * stAlmStatus, short usNTAFID);
extern int dCheckTrafficList(time_t tLocalTime, st_NtafStatList *stTRAFFIC);
extern void FinishProgram(void);

/**
	Declare functions
*/
extern void InitProc(void);
extern int dInit_ipcs(void);
extern void SetUpSignal(void);
extern void UserControlledSignal(int sign);
extern void FinishProgram(void);
extern void IgnoreSignal(int sign);
extern int dReadSubSysInfoFile(pst_SubSysInfoList pstList);
extern void Chg_TimeFlag(void *p);
extern void InitSHMFAULT(short TimeFlag, unsigned int uiStatTime);

#endif	/* __FSTAT_INIT_H__ */
