
/*********************************************************
                 ABLEX IPAS Project (IPAM BLOCK)

   Author   : LEE SANG HO
   Section  : IPAS(IPAM) Project
   SCCS ID  : %W%
   Date     : %G%
   Revision History :
        '03.    01. 15. initial

   Description:

   Copyright (c) ABLEX 2003
*********************************************************/



#ifndef _IPAM_HEADLOG_H
#define _IPAM_HEADLOG_H

#include <sys/time.h>


#define MAX_HEAD_LOG_INDEX  5

// 060711, poopee
#define UAWAP_HEADLOG_INDEX 0
#define AAA_HEADLOG_INDEX   1

typedef struct _st_LogFile {
	char		procname[16];
	time_t 		lllastfileno;
	time_t		tStartTime;
	unsigned int uiMaxLogSize;
	unsigned int uiTimeVal;
} st_LogFile, *pst_LogFile; 

typedef struct _st_LogFileList {
	st_LogFile   stLogFile[MAX_HEAD_LOG_INDEX];
} st_LogFileList, *pst_LogFileList; 

typedef struct _st_LogFileHead {
	time_t    tStartTime;
	unsigned short usProcIdx;
	unsigned short usResve;
} st_LogFileHead, *pst_LogFileHead;

typedef struct _st_LogDataHead {
	long long		llMagicNumber;
	struct timeval  stTmval;
	unsigned short 	usMType;
	unsigned short 	usINFType;
	unsigned int 	uiLogLen;
	unsigned int   	uiSrc;
	unsigned int   	uiDst;
} st_LogDataHead, *pst_LogDataHead;

typedef struct _st_LogDataHead2 {
	long long		llMagicNumber;
	struct timeval  stTmval;
	unsigned short 	usMType;
	unsigned short 	usINFType;
	unsigned int 	uiLogLen;
	unsigned int   	uiSrc;
	unsigned int   	uiDst;
	unsigned char   ucADR;
	unsigned char   szReserved[7];
} st_LogDataHead2, *pst_LogDataHead2;


#endif
