/**
	@file		fstat_fault.h	
	@author		
	@version	
	@date		2011-07-26
	@brief		fstat_fault.c 헤더파일
*/

#ifndef __FSTAT_COMM_H__
#define __FSTAT_COMM_H__

/**
	Include headers
*/
#include "hashg.h"
#include "timerN.h"
#include "sshmid.h"

#include "commdef.h"
#include "almstat.h"

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


#endif	/* __FSTAT_COMM_H__ */
