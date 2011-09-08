#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <commdef.h>

#define	MAX_PFRING_CNT		8

typedef struct _CAPD_INFO {
	time_t			curtime;
	int				ethidx[MAX_PFRING_CNT];
//	pfring_stat		pfringstat[MAX_PFRING_CNT];
} CAPD_INFO;


typedef struct _PREA_SUBINFO {
	U64				totCnt;
	U64				totSize;
	U64				totTcpCnt;
	U64				sndTcpCnt;
	U64				chksumCnt;
	U64				notSvcCnt;
	U64				totUdpCnt;
	U64				sndUdpCnt;
} PREA_SUBINFO;

typedef struct _PREA_INFO {
	time_t			curtime;
	PREA_SUBINFO	PREASUBINFO;	
} PREA_INFO;

typedef struct _ATCP_SUBINFO {
	U64				curSessCnt; 
	U64				sessCnt;
	U64				rcvNodeCnt;
	U64				diffSeqCnt;
	U64				updateCnt;
	U64				rcvSize;
	U64				httpSndCnt;
	U64				delCnt;
	U64				allSndCnt;
	U64				sigCnt;
} ATCP_SUBINFO;

typedef struct _ATCP_INFO {
	time_t			curtime;
	ATCP_SUBINFO	ATCPSUBINFO;
} ATCP_INFO;

typedef struct _DEBUG_INFO {
    CAPD_INFO       CAPDINFO;
	PREA_INFO		PREAINFO;
	ATCP_INFO		ATCPINFO[MAX_MP_NUM];
} DEBUG_INFO;

extern DEBUG_INFO *InitDEBUGINFO();

#endif
