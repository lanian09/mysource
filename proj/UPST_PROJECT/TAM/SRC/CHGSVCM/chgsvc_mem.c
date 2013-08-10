/*******************************************************************************
               DQMS Project

     Author   :
     Section  :
     SCCS ID  :
     Date     :
     Revision History :

     Description :

     Copyright (c) uPRESTO 2005
*******************************************************************************/
/** A.1* FILE INCLUDE *************************************/
#include <errno.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/msg.h>

/* User Define */
// LIB
#include "typedef.h"
#include "loglib.h"
#include "hasho.h"
#include "nifo.h"

// PROJECT
#include "chgsvc_list.h"

// .
#include "chgsvc_mem.h"

/** B.1* DEFINITION OF NEW CONSTANTS **********************/
/** C.1* DEFINITION OF NEW TYPES **************************/
/** D.1* DECLARATION OF VARIABLES *************************/

st_TOTLOG_LIST	st_pstTOTLogList;
st_TOTLOG_LIST	*g_pstTOTLogList = &st_pstTOTLogList;
stHASHOINFO		*g_pstHTTPLOGH;	/* */
stHASHOINFO		*g_pstTCPLOGH;	/* */
stHASHOINFO		*g_pstIPLISTH;	/* */
stHASHOINFO			*g_pstGOOGLEPUSH_H;
st_GOOGLEPUSH_MEM	stGooglePushMemList;



/** E.1* DEFINITION OF FUNCTIONS **************************/
int  	dInitHashMem(void);

/** E.2* DEFINITION OF FUNCTIONS **************************/
/*******************************************************************************
 
*******************************************************************************/
int dInitMem(void)
{
	int	dRet;

	/* Hash */
	dRet = dInitHashMem();
	if(dRet < 0)
		return -11;

	return 1;
}

#if 0
/*******************************************************************************

*******************************************************************************/
int  dInitTOTLogShm(void)
{
	int  dID;

	/* TEID */
	if( (dID = shmget(S_SSHM_AUTO_SVCLIST, DEF_TOTLOG_LIST_SIZE, SHMPERM | IPC_CREAT | IPC_EXCL)) < 0) {
		if(errno == EEXIST) {
			if( (dID = shmget(S_SSHM_AUTO_SVCLIST, DEF_TOTLOG_LIST_SIZE, SHMPERM | IPC_CREAT)) < 0) {
				log_print(LOGN_CRI, LH"[ERROR] shmget() Fail. CAUSE[%s]", LT, strerror(errno));
				return -1;
			}

			if( (g_pstTOTLogList = (st_TOTLOG_LIST *)shmat(dID, 0, 0)) == (void*)-1) {
				log_print(LOGN_CRI, LH"[ERROR] shmat() Fail. CAUSE[%s]", LT, strerror(errno));
				return -2;
			}
		} else {
			log_print(LOGN_CRI, LH"[ERROR] shmget() Fail. CAUSE[%s]", LT, strerror(errno));
			return -3;
		}
	} else {
		if( (g_pstTOTLogList = (st_TOTLOG_LIST *)shmat(dID, 0, 0)) == (void *)-1) {
			log_print(LOGN_CRI, LH"[ERROR] shmat() Fail. CAUSE[%s]", LT, strerror(errno));
			return -4;
		}
	}

	return 1;
}
#endif

/*******************************************************************************

*******************************************************************************/
int dInitHashMem(void)
{
	g_pstHTTPLOGH = hasho_init( 0, DEF_HTTPLOG_KEY_SIZE, DEF_HTTPLOG_KEY_SIZE, DEF_HTTPLOG_DATA_SIZE, MAX_HTTPLOG_HASH_CNT, 0);
	if(g_pstHTTPLOGH == NULL) {
		log_print(LOGN_CRI, LH"[ERROR] hasho_init(HTTPLOG) Fail. TOTAL_SIZE[%ld]", LT
					, (DEF_HTTPLOG_KEY_SIZE + DEF_HTTPLOG_DATA_SIZE) * MAX_HTTPLOG_HASH_CNT);
		return -1;
	}

	g_pstTCPLOGH = hasho_init( 0, DEF_TCPLOG_KEY_SIZE, DEF_TCPLOG_KEY_SIZE, DEF_TCPLOG_DATA_SIZE, MAX_TCPLOG_HASH_CNT, 0);
	if(g_pstTCPLOGH == NULL) {
		log_print(LOGN_CRI, LH"[ERROR] hasho_init(TCPLOG) Fail. TOTAL_SIZE[%ld]", LT
					, (DEF_TCPLOG_KEY_SIZE + DEF_TCPLOG_DATA_SIZE) * MAX_TCPLOG_HASH_CNT);
		return -2;
	}

	g_pstIPLISTH = hasho_init( 0, DEF_IPLIST_KEY_SIZE, DEF_IPLIST_KEY_SIZE, DEF_IPLIST_DATA_SIZE, MAX_IPLIST_HASH_CNT, 0);
	if(g_pstIPLISTH == NULL) {
		log_print(LOGN_CRI, LH"[ERROR] hasho_init(IPLIST) Fail. TOTAL_SIZE[%ld]", LT
					, (DEF_IPLIST_KEY_SIZE + DEF_IPLIST_DATA_SIZE) * MAX_IPLIST_HASH_CNT);
		return -3;
	}

	g_pstGOOGLEPUSH_H = hasho_init( 0, DEF_GOOGLEPUSH_KEY_SIZE, DEF_GOOGLEPUSH_KEY_SIZE, DEF_GOOGLEPUSH_DATA_SIZE
			, MAX_GOOGLE_PUSH_CNT, 0);
	if(g_pstGOOGLEPUSH_H == NULL) {
		log_print(LOGN_CRI, LH"[ERROR] hasho_init(GOOGLE_PUSH) Fail", LT);
		return -4;
	}

	return 1;
}

/*******************************************************************************

*******************************************************************************/
void CleanHttpLogMem(void)
{
	/* */
	hasho_reset(g_pstHTTPLOGH);
	hasho_reset(g_pstIPLISTH);

	g_pstTOTLogList->uiHttpLogCnt = 0;
	memset(&g_pstTOTLogList->stHttpLogList[0], 0x00, (DEF_HTTPLOG_LIST_SIZE * MAX_HTTPLOG_LIST_CNT));
	memset(&g_pstTOTLogList->stIPList[0], 0x00, (DEF_IPLIST_SIZE * MAX_HTTPLOG_LIST_CNT));

	return;
}

/*******************************************************************************

*******************************************************************************/
void CleanTcpLogMem(void)
{
	hasho_reset(g_pstTCPLOGH);
	hasho_reset(g_pstGOOGLEPUSH_H);
	memset(&stGooglePushMemList, 0x00, DEF_GOOGLEPUSH_MEM_SIZE);

	g_pstTOTLogList->uiTcpLogCnt = 0;
	memset(&g_pstTOTLogList->stTcpLogList[0], 0x00, (DEF_TCPLOG_LIST_SIZE * MAX_TCPLOG_LIST_CNT));

	return;
}

/*******************************************************************************
 RETURN : 
 	찾으면 index 리턴, 못 찾으면 0 리턴
*******************************************************************************/
int dSetIPListHash(UINT IP, UINT index)
{
	st_IPLIST_KEY	stKey;
	st_IPLIST_DATA	stData, *pstData;
	stHASHONODE	*pHASHNODE;

	/* Key */
	memset(&stKey, 0x00, DEF_IPLIST_KEY_SIZE);
	stKey.uiIP = IP;

	if((pHASHNODE = hasho_find( g_pstIPLISTH, (U8 *)&stKey)) == NULL) {
		/* Data */
		memset(&stData, 0x00, DEF_IPLIST_DATA_SIZE);
		stData.uiIPIndex = index;

		if( (pHASHNODE = hasho_add( g_pstIPLISTH, (U8 *)&stKey, (U8 *)&stData)) == NULL) {
			log_print(LOGN_CRI, LH"[ERROR] hasho_add(IPLIST) Fail. KEY IP[%u]", LT, stKey.uiIP);
			return -1;
		} else {
			//log_print(LOG_DEBUG, LH"IPLIST ADD SUCCESS. KEY IP[%u]", LT, stKey.uiIP);
		}

		return 0;
	}
	else {
		pstData = (st_IPLIST_DATA *)nifo_ptr( g_pstIPLISTH, pHASHNODE->offset_Data);
		if(pstData != NULL)
		{
			return pstData->uiIPIndex + 1;
		} 
	}
	return -1;
}

/*******************************************************************************

*******************************************************************************/
int dSetHttpLogHash(char *hostname, UINT IP, long long pktcnt)
{
	int				dRet, len;
	st_HTTPLOG_KEY	stKey;
	st_HTTPLOG_DATA	stData, *pstData = NULL;
	stHASHONODE		*pHASHNODE;
	st_IPLIST		*pstIPLIST;
	st_HTTPLOG_LIST	*pstHttpLog;

	len = strlen(hostname);
	if(len > 64)
		len = 64;

	/* Key */
	memset(&stKey, 0x00, DEF_HTTPLOG_KEY_SIZE);
	memcpy(stKey.szHostname, hostname, len);	
	
	if((pHASHNODE = hasho_find(g_pstHTTPLOGH, (U8 *)&stKey)) == NULL) {
		if(g_pstTOTLogList->uiHttpLogCnt >= MAX_HTTPLOG_LIST_CNT) {
			log_print( LOGN_CRI, LH"[ERROR] HTTP LOG FULL. COUNT[%d]", LT
					, g_pstTOTLogList->uiHttpLogCnt);
			return -1;
		}

		/* Data */
		memset(&stData, 0x00, DEF_HTTPLOG_DATA_SIZE);
		stData.uiArrayIndex = g_pstTOTLogList->uiHttpLogCnt;

		pstHttpLog = &g_pstTOTLogList->stHttpLogList[g_pstTOTLogList->uiHttpLogCnt];
		pstIPLIST = &g_pstTOTLogList->stIPList[g_pstTOTLogList->uiHttpLogCnt];

		//pstHttpLog->llPktCnt += pktcnt;									/* 패킷수 */
		pstHttpLog->uiArrayIndex = g_pstTOTLogList->uiHttpLogCnt;
		memcpy(pstIPLIST->szHostname, hostname, len);					/* 도메인네임 */
		pstIPLIST->szHostname[len] = '\0';

		dRet = dSetIPListHash(IP, pstIPLIST->uiCnt);
		if(dRet == 0)	/* IPLIST 해시에 새로 등록 */ {
			pstIPLIST->uiIPList[pstIPLIST->uiCnt] = IP;			/* 새로운 IP 리스트에 저장 */
			pstIPLIST->uiIPList[pstIPLIST->uiCnt] += pktcnt;
			pstIPLIST->uiCnt++;
#if 0
			log_print(LOG_DEBUG, LH"ADD IPLIST NEW IP[%u]. IPLIST_INDEX[%u] DOMAIN[%s] IPLIST_CNT[%u]"
						, LT
						, IP, g_pstTOTLogList->uiHttpLogCnt, stKey.szHostname, pstIPLIST->uiCnt);
#endif
		}
		else if(dRet > 0) {
			pstIPLIST->uiIPList[dRet-1] += pktcnt;

		} else {
			log_print(LOGN_CRI, LH"[ERROR] dSetIPListHash() Fail. HOSTNAME[%s] IP[%u]", LT
					, stKey.szHostname, IP);
		}

		if( (pHASHNODE = hasho_add(g_pstHTTPLOGH, (U8 *)&stKey, (U8 *)&stData)) == NULL) {
			log_print(LOGN_CRI, LH"[ERROR] hasho_add(HTTP_LOG) Fail. HOSTNAME[%s]", LT
					, stKey.szHostname);
			return -2;
		}

		g_pstTOTLogList->uiHttpLogCnt++;
	}
	else
	{
		pstData = (st_HTTPLOG_DATA *)nifo_ptr( g_pstHTTPLOGH, pHASHNODE->offset_Data);
		if(pstData != NULL)
		{
			pstHttpLog = &g_pstTOTLogList->stHttpLogList[pstData->uiArrayIndex];
			pstIPLIST = &g_pstTOTLogList->stIPList[pstData->uiArrayIndex];

			//pstHttpLog->llPktCnt += pktcnt;		/* 패킷수 */
			//pstIPLIST->uiIPList[pstIPLIST->uiCnt] += pktcnt;

			dRet = dSetIPListHash(IP, pstIPLIST->uiCnt);
			if(dRet == 0) {	/* IPLIST 해시에 새로 등록 */

				if(pstIPLIST->uiCnt >= MAX_IPLIST_CNT) {
					log_print( LOGN_CRI, LH"[ERROR] IPLIST FULL. IPLIST_INDEX[%u] DOMAIN[%s] COUNT[%d]"
							, LT
							, pstData->uiArrayIndex, pstIPLIST->szHostname, pstIPLIST->uiCnt);
					return -3;
				}

				pstIPLIST->uiIPList[pstIPLIST->uiCnt] += pktcnt;
				pstIPLIST->uiIPList[pstIPLIST->uiCnt] = IP;
				pstIPLIST->uiCnt++;
#if 0 
				log_print(LOG_DEBUG, LH"ADD IPLIST NEW IP[%u]. IPLIST_INDEX[%u] DOMAIN[%s] IPLIST_CNT[%u]"
						, LT
						, IP, g_pstTOTLogList->uiHttpLogCnt, stKey.szHostname, pstIPLIST->uiCnt);
#endif
			}
			else if(dRet > 0) {
				pstIPLIST->uiIPList[dRet-1] += pktcnt;
			} else {
				log_print(LOGN_CRI, LH"[ERROR] dSetIPListHash() Fail. HOSTNAME[%s] IP[%u]", LT
						, stKey.szHostname, IP);
			}
		} else {
			log_print(LOGN_CRI, LH"[ERROR] INVALID HASH(HTTP_LOG) DATA POINTER. DELETE HASH. KEY HOSTNAME[%s]"
						, LT, stKey.szHostname);

			hasho_del(g_pstHTTPLOGH, (U8 *)&stKey);
		}
	}

	return 1;
}


/*******************************************************************************

*******************************************************************************/
int dSetTcpLogHash(UINT IP, long long pktcnt)
{
	st_TCPLOG_KEY	stKey;
	st_TCPLOG_DATA	stData, *pstData = NULL;
	stHASHONODE	*pHASHNODE;
	st_TCPLOG_LIST	*pstTcpLog;

	/* Key */
	memset(&stKey, 0x00, DEF_TCPLOG_KEY_SIZE);
	stKey.uiIP = IP;

	if((pHASHNODE = hasho_find(g_pstTCPLOGH, (U8 *)&stKey)) == NULL) {
		if(g_pstTOTLogList->uiTcpLogCnt >= MAX_TCPLOG_LIST_CNT) {
			log_print(LOGN_CRI, LH"[ERROR] TCP LIST FULL. COUNT[%d]", LT
					, g_pstTOTLogList->uiTcpLogCnt);
			return -1;
		}

		/* Data */
		memset(&stData, 0x00, DEF_TCPLOG_DATA_SIZE);
		stData.uiArrayIndex = g_pstTOTLogList->uiTcpLogCnt;

		pstTcpLog = &g_pstTOTLogList->stTcpLogList[g_pstTOTLogList->uiTcpLogCnt];

		pstTcpLog->llPktCnt += pktcnt;			/* 패킷 수 */
		pstTcpLog->uiIP = IP;					/* IP */

		if( (pHASHNODE = hasho_add(g_pstTCPLOGH, (U8 *)&stKey, (U8 *)&stData)) == NULL) {
			log_print(LOGN_CRI, LH"[ERROR] hasho_add(TCP_LOG) Fail. IP[%u]", LT, stKey.uiIP);
			return -2;
		}

		g_pstTOTLogList->uiTcpLogCnt++;
	} else {
		pstData = (st_TCPLOG_DATA *)nifo_ptr( g_pstTCPLOGH, pHASHNODE->offset_Data);
		if(pstData != NULL) {
			pstTcpLog = &g_pstTOTLogList->stTcpLogList[pstData->uiArrayIndex];

			pstTcpLog->llPktCnt += pktcnt;		/* 패킷 수 */
		} else {
			log_print(LOGN_CRI, LH"[ERROR] INVALID HASH(TCP_LOG) DATA POINTER. DELETE HASH. KEY IP[%u]"
						, LT, stKey.uiIP);

			hasho_del(g_pstTCPLOGH, (U8 *)&stKey);
		}
	}

	return 1;
}

int dAddGooglePushHash(UINT IP)
{
	stHASHONODE         *pHASHNODE;
	st_GOOGLEPUSH_KEY   stGoogleListKey;
	st_GOOGLEPUSH_DATA  stGoogleListData;

	memset(&stGoogleListKey, 0x00, DEF_GOOGLEPUSH_KEY_SIZE);
	memset(&stGoogleListData, 0x00, DEF_GOOGLEPUSH_DATA_SIZE);

	stGoogleListKey.uiIP = IP;

	if((pHASHNODE = hasho_find(g_pstGOOGLEPUSH_H, (U8 *)&stGoogleListKey)) == NULL) {

		if(stGooglePushMemList.dCnt <= MAX_GOOGLE_PUSH_CNT) {
			pHASHNODE = hasho_add(g_pstGOOGLEPUSH_H, (U8 *)&stGoogleListKey, (U8 *)&stGoogleListData);
			stGooglePushMemList.IPList[stGooglePushMemList.dCnt] = IP;
			stGooglePushMemList.dCnt++;
		} else {
			log_print(LOGN_CRI, LH"OVERFLOW GOOGLE_PUSH LIST COUNT. MAX=%d", LT, stGooglePushMemList.dCnt
				   );
		}
	}

	return 1;
}

/*******************************************************************************
 재귀함수 처리
 TODO :
 	재귀처리에 의한 오버플로우가 발생할 수 있다. 
*******************************************************************************/
void HttpLogQuickSort(st_HTTPLOG_LIST httplog[], int low, int high)
{
	int				i, j;
	UINT				pivot;
	st_HTTPLOG_LIST	TmpHttp;

	if (low < high) {
		pivot = httplog[(low + high) / 2].llPktCnt;
		i = low - 1;
		j = high + 1;

		while (1) {
			/* 내림차순 */
			while (httplog[++i].llPktCnt > pivot);
			while (httplog[--j].llPktCnt < pivot);
#if 0
			/* 올림차순 */
			while (httplog[++i].llPktCnt < pivot);
			while (httplog[--j].llPktCnt > pivot);
#endif

			if (i >= j) break;

			memcpy(&TmpHttp, &httplog[i], DEF_HTTPLOG_LIST_SIZE);
			memcpy(&httplog[i], &httplog[j], DEF_HTTPLOG_LIST_SIZE);
			memcpy(&httplog[j], &TmpHttp, DEF_HTTPLOG_LIST_SIZE);
		}

		HttpLogQuickSort(httplog, low, i-1);
		HttpLogQuickSort(httplog, j+1, high);
	}
	return;
}     

/*******************************************************************************
 재귀함수 처리
 TODO :
 	재귀처리에 의한 오버플로우가 발생할 수 있다. 
*******************************************************************************/
void TcpLogQuickSort(st_TCPLOG_LIST tcplog[], int low, int high)
{
	int				i, j;
	UINT				pivot;
	st_TCPLOG_LIST		TmpTcp;

	if (low < high) {
		pivot = tcplog[(low + high) / 2].llPktCnt;
		i = low - 1;
		j = high + 1;

		while (1) {
			/* 내림차순 */
			while (tcplog[++i].llPktCnt > pivot);
			while (tcplog[--j].llPktCnt < pivot);
#if 0
			/* 올림차순 */
			while (tcplog[++i].llPktCnt < pivot);
			while (tcplog[--j].llPktCnt > pivot);
#endif

			if (i >= j) break;

			memcpy(&TmpTcp, &tcplog[i], DEF_TCPLOG_LIST_SIZE);
			memcpy(&tcplog[i], &tcplog[j], DEF_TCPLOG_LIST_SIZE);
			memcpy(&tcplog[j], &TmpTcp, DEF_TCPLOG_LIST_SIZE);
		}

		TcpLogQuickSort(tcplog, low, i-1);
		TcpLogQuickSort(tcplog, j+1, high);
	}
	return;
}     


/* END */
