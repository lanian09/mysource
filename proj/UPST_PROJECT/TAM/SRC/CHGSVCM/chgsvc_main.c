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
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

/* User Define */
// LIB
#include "loglib.h"
#include "mems.h"
#include "cifo.h"
#include "gifo.h"
#include "nifo.h"
#include "path.h"
#include "sshmid.h"
#include "verlib.h"

// PROJECT
#include "procid.h"

// .
#include "chgsvc_init.h"
#include "chgsvc_proc.h"
#include "chgsvc_mem.h"

/** B.1* DEFINITION OF NEW CONSTANTS **********************/
#define DEF_INETSVC_FTPPUT_SCRIPT	"/TAMAPP/SCRIPT/ftp_inetsvc.sh"

/** C.1* DEFINITION OF NEW TYPES **************************/
/** D.1* DECLARATION OF VARIABLES *************************/
stMEMSINFO          *pMEMSINFO;
stCIFO              *gpCIFO;

int					gdStopFlag = 1;

unsigned long long	gullTotCnt = 0; /* TEST */
/* Extern */

#ifdef PRINT
extern st_TOTLOG_LIST *g_pstTOTLogList;

#endif

/** E.1* DEFINITION OF FUNCTIONS **************************/
int		dConfigCheck(void);
char*	pGetStrTime(time_t time);
char	gStrTimeBuf[32];

#ifdef PRINT
void	PrintTOTLogMem(void);
#endif

/** E.2* DEFINITION OF FUNCTIONS **************************/
/*******************************************************************************
  
*******************************************************************************/
int main(int argc, char *argv[])
{
	int			dRet;
	time_t		curTime;

	log_init(S_SSHM_LOG_LEVEL, getpid(), SEQ_PROC_CHGSVCM, LOG_PATH"/CHGSVCM", "CHGSVCM");

	pMEMSINFO = nifo_init_zone((unsigned char *)"CHGSVCM", SEQ_PROC_CHGSVCM, FILE_NIFO_ZONE);
	if( pMEMSINFO == NULL ){
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN nifo_init, NULL",  __FILE__, __FUNCTION__, __LINE__);
		exit(0);
	}

	gpCIFO = gifo_init_group(FILE_CIFO_CONF, FILE_GIFO_CONF);
	if( gpCIFO == NULL ){
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN gifo_init_group. cifo=%s, gifo=%s",
				__FILE__, __FUNCTION__, __LINE__, FILE_CIFO_CONF, FILE_GIFO_CONF);
		exit(0);
	}

	if( (dRet = dInitProc()) < 0) {
		log_print(LOGN_CRI, "[%s.%d] [ERROR] dInitProc() Fail. RET[%d]", __FUNCTION__, __LINE__, dRet);
		FinishProgram();
		return -2;
	}

	curTime = time(NULL);

	log_print(LOGN_CRI, " *- PROCESS START -*");
	log_print(LOGN_CRI, "START TIME[%s]", pGetStrTime(curTime));

	/* Clear Memory */
	CleanHttpLogMem();
	CleanTcpLogMem();

	dRet = dGetDirecFileList(DEF_BACKUP_LOG_PATH, curTime);
	if(dRet < 0) {
		log_print( LOGN_CRI, "[%s.%d] [ERROR] dGetDirecFileList() Fail. RET[%d] PATH[%s]", __FUNCTION__, __LINE__
						, dRet, DEF_BACKUP_LOG_PATH);
		FinishProgram();
		return -4;
	}

#if 0 /* Except Sort */
	HttpLogQuickSort(g_pstTOTLogList->stHttpLogList, 0, (g_pstTOTLogList->uiHttpLogCnt - 1));
	TcpLogQuickSort(g_pstTOTLogList->stTcpLogList, 0, (g_pstTOTLogList->uiTcpLogCnt - 1));
#endif

#ifdef PRINT
	//PrintTOTLogMem();
#endif

	/* LOG 파일의 통계 파일 생성 */
	dRet = dWriteHttpTcpLogStatisticFile(curTime);
	if(dRet < 0) {
		log_print( LOGN_CRI, "[%s:%s][ERROR] dWriteFileInternetSvcList() Fail. RET[%d]", __FILE__, __FUNCTION__, dRet);
		FinishProgram();
		return -5;
	}

	dRet = dWriteGooglePushList(curTime);
	if(dRet < 0) {
		log_print( LOGN_CRI, "[%s:%s][ERROR] dWriteGooglePushList() Fail. RET[%d]", __FILE__, __FUNCTION__, dRet);
		FinishProgram();
		return -6;
	}

	/* ftp를 통해서 GTAM으로 전송 */
	dRet = system(DEF_INETSVC_FTPPUT_SCRIPT);
	if(dRet != 0)
		log_print( LOGN_CRI, "[%s:%s][ERROR] FTP PUT Fail. SCRIPT FILE[%s]", __FILE__, __FUNCTION__, DEF_INETSVC_FTPPUT_SCRIPT);

	curTime = time(NULL);
	log_print(LOGN_CRI, "END TIME[%s]. LINE[%llu]", pGetStrTime(curTime), gullTotCnt);

	FinishProgram();

	return 1;
}

char* pGetStrTime(time_t time)
{
	struct tm	tm;

	localtime_r(&time, &tm);
	sprintf(gStrTimeBuf, "%d/%d/%d/ %02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday
												, tm.tm_hour, tm.tm_min, tm.tm_sec);
	return gStrTimeBuf;
}

#ifdef PRINT 
/*******************************************************************************
  
*******************************************************************************/
void PrintTOTLogMem(void)
{
	int				i,j;
	st_HTTPLOG_LIST	*pstHttpLog;
	st_IPLIST			*pstIPList;
	st_TCPLOG_LIST		*pstTcpLog;

	log_print(LOGN_DEBUG, " ");
	log_print(LOGN_DEBUG, "----------------[ TOT LOG MEM INFO ]-----------------");
	log_print(LOGN_DEBUG, "[HTTP_LOG]-------------------------------------------");
	log_print(LOGN_DEBUG, "[CNT = %u]", g_pstTOTLogList->uiHttpLogCnt);
	log_print(LOGN_DEBUG, "----------");

	for(i = 0; i < g_pstTOTLogList->uiHttpLogCnt; i++) {
		pstHttpLog = &g_pstTOTLogList->stHttpLogList[i];
		pstIPList = &g_pstTOTLogList->stIPList[pstHttpLog->uiArrayIndex];	

		log_print(LOGN_DEBUG, "* [HOSTNAME]       [%s]", pstIPList->szHostname);
		log_print(LOGN_DEBUG, "  [PKTCNT]         [%lld]", pstHttpLog->llPktCnt);
		for(j = 0; j < pstIPList->uiCnt; j++) {
			log_print(LOGN_DEBUG, "          [IP]    [%u]", pstIPList->uiIPList[j]);
		}
	}

	log_print(LOGN_DEBUG, "[TCP_LOG]--------------------------------------------");
	log_print(LOGN_DEBUG, "[CNT = %u]", g_pstTOTLogList->uiTcpLogCnt);
	log_print(LOGN_DEBUG, "----------");

	for(i = 0; i < g_pstTOTLogList->uiTcpLogCnt; i++) {
		pstTcpLog = &g_pstTOTLogList->stTcpLogList[i];

		log_print(LOGN_DEBUG, "* [IP]             [%u]", pstTcpLog->uiIP);
		log_print(LOGN_DEBUG, "  [PKTCNT]         [%lld]", pstTcpLog->llPktCnt);
	}

	log_print(LOGN_DEBUG, " ");
	return;
}

#endif






/* END */
