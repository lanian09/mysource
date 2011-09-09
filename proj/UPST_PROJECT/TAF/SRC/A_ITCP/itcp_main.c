/**		@file	tcp_main.c
 * 		- TCP Session을 관리 하는 프로세스
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: itcp_main.c,v 1.2 2011/09/05 12:26:40 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.2 $
 * 		@date		$Date: 2011/09/05 12:26:40 $
 * 		@ref		tcp_main.c
 * 		@todo		library가 만들어지지 않은 상태, library 생성 이후 함수 대치
 *
 * 		@section	Intro(소개)
 * 		- TCP Session을 관리 하는 프로세스
 *
 * 		@section	Requirement
 * 		 @li library 생성 이후 함수 대치
 *
 **/

/**
 * Include headers
 */
#include <unistd.h>

// TOP
#include "common_stg.h"
#include "path.h"
#include "commdef.h"
#include "procid.h"
#include "sshmid.h"
#include "filter.h"
#include "func_time_check.h"	/* st_FuncTimeCheckList */
#include "capdef.h"

// LIB headers
#include "mems.h"
#include "cifo.h"
#include "gifo.h"
#include "loglib.h"
#include "verlib.h"
#include "ipclib.h"
#include "memg.h"
#include "Analyze_Ext_Abs.h"	/* INFO_ETH */

// TAF headers
#include "debug.h"

// .
#include "itcp_sess.h"

/**
 * Declare variables
 */
stCIFO          *gpCIFO;
stMEMSINFO		*pMEMSINFO;
stHASHOINFO		*pTCPHASH;
stTIMERNINFO	*pTIMER;

stHASHOINFO				*pSESSKEYINFO;  	/* TCP SESSION 정리를 위한 HASH */
extern stHASHONODE 		*pSESSKEYNODE;

st_Flt_Info 		*flt_info;

S32				dMyQID;
S32				dHTTPQID[MAX_MP_NUM];
S32				dLOGQID[MAX_SMP_NUM];

S32				giFinishSignal;				/**< Finish Signal */
S32				giStopFlag;					/**< main loop Flag 0: Stop, 1: Loop */
UINT			guiSeqProcID;

DEBUG_INFO		*pDEBUGINFO;
ATCP_SUBINFO	ATCPSUBINFO;
ATCP_SUBINFO	*pATCPSUBINFO = &ATCPSUBINFO;

st_FuncTimeCheckList	stFuncTimeCheckList;
st_FuncTimeCheckList	*pFUNC = &stFuncTimeCheckList;

/* FOR MULTIPLE PROCESS */
char 			gszMyProc[32];

/* IP SESSION MANAGER 	*/
pSESSKEY_TBL 	pstSESSKEYTbl;

/**
 * Declare functions
 */
void invoke_del(void *p);
int Delete_SessList(TCP_SESS_KEY *pTCPSESSKEY);

/**
 *	Implement func.
 */
int main(int argc, char *argv[])
{
	S32				i;
	S32				dRet;
	OFFSET			offset, sub_offset;
	time_t			oldTime, nowTime;

	Capture_Header_Msg	*pCAPHEAD;
	INFO_ETH			*pINFOETH;
	CALL_KEY 			*pCALLKey;
	U8					*pNode, *pCurrNode, *pNextNode;

	SESSKEY_LIST 		*pstSESSKEYList;
	char 				szLOGPATH[128];
	int 				dLen, PROCNO;
	int 				LastLogTime;
	
	char    vERSION[7] = "R3.0.0";

	/* process name */
	memcpy(&gszMyProc[0], argv[0], strlen(argv[0]));

	dLen = strlen(gszMyProc);
	PROCNO = atoi(&gszMyProc[dLen-1]);
	guiSeqProcID = SEQ_PROC_A_ITCP0 + PROCNO;

	/* log_print 초기화 */
	sprintf(szLOGPATH, LOG_PATH"/%s", gszMyProc);
	log_init(S_SSHM_LOG_LEVEL,getpid(), guiSeqProcID, szLOGPATH, gszMyProc);
	
	/* A_TCP 초기화 */
	if((dRet = dInitTcp(&pMEMSINFO, &pTCPHASH, &pTIMER)) < 0) {
		log_print(LOGN_CRI, LH"dInitTcp dRet[%d]", LT, dRet);
		exit(0);
	}

	if( (dRet = shm_init(S_SSHM_UTIL, sizeof(DEBUG_INFO), (void**)&pDEBUGINFO)) < 0 ){
        log_print(LOGN_CRI, "FAILED IN shm_init(DEBUG_INFO=%d)", S_SSHM_UTIL);
        exit(0);
    }

	if((dRet = set_version(S_SSHM_VERSION, guiSeqProcID, vERSION)) < 0 ) {
		log_print(LOGN_CRI, "SET_VERSION ERROR(RET=%d,IDX=%d,VER=%s)", dRet, guiSeqProcID, vERSION);
	}
	log_print(LOGN_CRI, "START %s[%d] ITCP_SESS_CNT[%d]", gszMyProc, PROCNO, ITCP_SESS_CNT);

	nowTime = time(NULL);
	oldTime = nowTime;	

	/* MAIN LOOP */
	while(giStopFlag)
	{
START_FUNC_TIME_CHECK(pFUNC, 0);
		timerN_invoke(pTIMER);
END_FUNC_TIME_CHECK(pFUNC, 0);

		for(i=0; i < 20000; i++) {
			if((offset = gifo_read(pMEMSINFO, gpCIFO, guiSeqProcID)) > 0) {
				pNextNode = nifo_ptr(pMEMSINFO, offset);
				pNode = pNextNode;
				pCurrNode = NULL;
				while(pCurrNode != pNextNode)
				{
					pCurrNode = pNextNode;
					sub_offset = nifo_offset(pMEMSINFO, pCurrNode);
					pNextNode = (U8 *)nifo_entry(nifo_ptr(pMEMSINFO, ((NIFO *)pNextNode)->nont.offset_next), NIFO, nont);

					pCAPHEAD = (Capture_Header_Msg *)nifo_get_value(pMEMSINFO, CAP_HEADER_NUM, sub_offset);
					pINFOETH = (INFO_ETH *)nifo_get_value(pMEMSINFO, INFO_ETH_NUM, sub_offset);
					pCALLKey = (CALL_KEY *)nifo_get_value(pMEMSINFO, CLEAR_CALL_NUM, sub_offset);

					nifo_node_unlink_nont(pMEMSINFO, pCurrNode);

					if(pCALLKey) {
						log_print(LOGN_DEBUG, "#### RECEIVE CALL CLEAR MSG IP: %d.%d.%d.%d LASTLOGTIME: %u ", 
								HIPADDR(pCALLKey->uiSrcIP), pCALLKey->uiReserved);
						LastLogTime = pCALLKey->uiReserved;
						pCALLKey->uiReserved = 0;
						if( (pSESSKEYNODE = hasho_find(pSESSKEYINFO, (U8 *)pCALLKey)) != NULL ) {
							log_print(LOGN_INFO, "#### FIND TCP SESSION LIST IP: %d.%d.%d.%d ", HIPADDR(pCALLKey->uiSrcIP));

							pstSESSKEYList = (SESSKEY_LIST *)nifo_ptr(pSESSKEYINFO, pSESSKEYNODE->offset_Data);
							dRet = dDelSessKeyList(pstSESSKEYList, pTCPHASH, pMEMSINFO, LastLogTime, pTIMER);
							if(dRet == 0) {
								hasho_del(pSESSKEYINFO, (U8 *)pCALLKey);
							}
						} else {
							log_print(LOGN_INFO, "#### NOT EXIST CALL IP: %d.%d.%d.%d ", HIPADDR(pCALLKey->uiSrcIP));
						}
						nifo_node_delete(pMEMSINFO, pCurrNode);
					}
					else if((pCAPHEAD == NULL) || (pINFOETH == NULL)) {
						nifo_node_delete(pMEMSINFO, pCurrNode);
					}
					else {
						if(pINFOETH->stIP.ucProtocol == DEF_PROTOCOL_TCP) {
							if(dProcTcpSess(pMEMSINFO, pTCPHASH, pTIMER, pCAPHEAD, pINFOETH, pCurrNode) < 0) {
								nifo_node_delete(pMEMSINFO, pCurrNode);
							}
						} else {
							nifo_node_delete(pMEMSINFO, pCurrNode);
						}
					}
				}
			} else {
				usleep(0);
				break;
			}
		}
#if 1
		nowTime = time(NULL);
		if(nowTime >= oldTime + 60) {
			log_print(LOGN_CRI, 
				"SESS CUR=%lld SESS=%lld RNODE=%lld:%.3fMbps SND=%lld DEL=%lld SIG=%lld HTTP=%lld DIFSEQ=%lld UPD=%lld HASH=%u CALL=%u TCP_TIMER=%u RST_TIMER=%u", 
				pATCPSUBINFO->curSessCnt, pATCPSUBINFO->sessCnt, pATCPSUBINFO->rcvNodeCnt, 
				(double)((pATCPSUBINFO->rcvSize * 8) / (((nowTime - oldTime) == 0) ? 1 : (nowTime - oldTime))) / (1024*1024), 
				pATCPSUBINFO->allSndCnt, pATCPSUBINFO->delCnt, pATCPSUBINFO->sigCnt, 
				pATCPSUBINFO->httpSndCnt, pATCPSUBINFO->diffSeqCnt, pATCPSUBINFO->updateCnt,
				((stMEMGINFO *)HASHO_PTR(pTCPHASH, pTCPHASH->offset_memginfo))->uiMemNodeAllocedCnt, 
				((stMEMGINFO *)HASHO_PTR(pSESSKEYINFO, pSESSKEYINFO->offset_memginfo))->uiMemNodeAllocedCnt,
				flt_info->stTimerInfo.usTimerInfo[PI_TCP_TIMEOUT],
				flt_info->stTimerInfo.usTimerInfo[PI_TCP_RSTWAIT]);
			pATCPSUBINFO->rcvNodeCnt = 0;
			pATCPSUBINFO->rcvSize = 0;
			pATCPSUBINFO->httpSndCnt = 0;
			pATCPSUBINFO->delCnt = 0;
			pATCPSUBINFO->allSndCnt = 0;
			pATCPSUBINFO->diffSeqCnt = 0;
			pATCPSUBINFO->sessCnt = 0;
			pATCPSUBINFO->updateCnt = 0;
			pATCPSUBINFO->sigCnt = 0;
			oldTime = nowTime;
PRINT_FUNC_TIME_CHECK(pFUNC);
		}
#endif

#if 0
		nowTime = time(NULL);
		if(nowTime >= oldTime + 60) {
			memcpy(&pDEBUGINFO->ATCPINFO[PROCNO].ATCPSUBINFO, pATCPSUBINFO, sizeof(ATCP_SUBINFO));
			pDEBUGINFO->ATCPINFO[PROCNO].curtime = nowTime;
			pATCPSUBINFO->rcvNodeCnt = 0;
			pATCPSUBINFO->rcvSize = 0;
			pATCPSUBINFO->httpSndCnt = 0;
			pATCPSUBINFO->onlineSndCnt = 0;
			pATCPSUBINFO->allSndCnt = 0;
			pATCPSUBINFO->diffSeqCnt = 0;
			pATCPSUBINFO->sessCnt = 0;
			pATCPSUBINFO->updateCnt = 0;
			oldTime = nowTime;
PRINT_FUNC_TIME_CHECK(pFUNC);
		}
#endif
	} /* while-loop end (main) */

	FinishProgram();
	exit(0);
}

void invoke_del(void *p)
{
START_FUNC_TIME_CHECK(pFUNC, 1);
	TCP_SESS		*pTCPSESS;
	TCP_SESS_KEY	*pTCPSESSKEY;
	stHASHONODE		*pHASHNODE;
	U8				szSIP[INET_ADDRSTRLEN];
	U8				szDIP[INET_ADDRSTRLEN];
	
	pTCPSESSKEY = &(((TCP_COMMON *)p)->TCPSESSKEY);

log_print(LOGN_DEBUG, 
"@@@ TIMER TIMEOUT SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]", 
util_cvtipaddr(szSIP, pTCPSESSKEY->uiSIP), pTCPSESSKEY->usSPort, 
util_cvtipaddr(szDIP, pTCPSESSKEY->uiDIP), pTCPSESSKEY->usDPort);

	if((pHASHNODE = hasho_find(pTCPHASH, (U8 *)pTCPSESSKEY)) != NULL) {
		pTCPSESS = (TCP_SESS *)nifo_ptr(pTCPHASH, pHASHNODE->offset_Data);
		log_print(LOGN_DEBUG, 
			"@@@ INVOKE TIMEOUT SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]CTIME[%u]UTIME[%u]NTIME[%u]", 
			util_cvtipaddr(szSIP, pTCPSESSKEY->uiSIP), pTCPSESSKEY->usSPort, 
			util_cvtipaddr(szDIP, pTCPSESSKEY->uiDIP), pTCPSESSKEY->usDPort,
			pTCPSESS->uiSessCreateTime, pTCPSESS->uiSessCreateMTime, time(NULL));

		/* TimeOut으로 종료된 경우와 RST으로 종료 되어 기다리는 경우를 분리 하기 위함 */
		if(pTCPSESS->ucEndStatus != DEF_END_RST)
			pTCPSESS->ucEndStatus = DEF_END_LONGLAST;

		dCloseSession(pMEMSINFO, pTCPHASH, pTCPSESSKEY, pTCPSESS);
		/* Delete Call Session List */
		Delete_SessList(pTCPSESSKEY);
	} else {
		log_print(LOGN_CRI, "INVOKE TIMEOUT BUT NODE NULL SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]NTIME[%u]", 
			util_cvtipaddr(szSIP, pTCPSESSKEY->uiSIP), pTCPSESSKEY->usSPort, 
			util_cvtipaddr(szDIP, pTCPSESSKEY->uiDIP), pTCPSESSKEY->usDPort, time(NULL));
	}
END_FUNC_TIME_CHECK(pFUNC, 1);
}

int Delete_SessList(TCP_SESS_KEY *pTCPSESSKEY)
{
	int             dRet;
	CALL_KEY        CALLKey;
	CALL_KEY        *pCALLKey = &CALLKey;
	SESSKEY_LIST    *pstSESSKEYList;

	pCALLKey->uiSrcIP = pTCPSESSKEY->uiSIP;
	pCALLKey->uiReserved = 0;

	log_print(LOGN_DEBUG, "#### DELETE SESS LIST IP: %d.%d.%d.%d ", HIPADDR(pCALLKey->uiSrcIP));

	if( (pSESSKEYNODE = hasho_find(pSESSKEYINFO, (U8 *)pCALLKey)) != NULL ) {
		log_print(LOGN_INFO, "#### FIND SIP TRANSACTION LIST IP: %d.%d.%d.%d ", HIPADDR(pCALLKey->uiSrcIP));

		pstSESSKEYList = (SESSKEY_LIST *)nifo_ptr(pSESSKEYINFO, pSESSKEYNODE->offset_Data);
		dRet = dDelSessKeyNext(pstSESSKEYList, pTCPSESSKEY);
		if(dRet == 0) {
			hasho_del(pSESSKEYINFO, (U8 *)pCALLKey);
		}
	} else {
		log_print(LOGN_INFO, "#### NOT EXIST CALL IP: %d.%d.%d.%d ", HIPADDR(pCALLKey->uiSrcIP));
	}

	return dRet;
}

/*
 * $Log: itcp_main.c,v $
 * Revision 1.2  2011/09/05 12:26:40  hhbaek
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * NEW OAM SYSTEM
 *
 * Revision 1.3  2011/08/17 13:02:39  hhbaek
 * A_ITCP
 *
 * Revision 1.2  2011/08/10 09:57:44  uamyd
 * modified and block added
 *
 * Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 * init DQMS2
 *
 * Revision 1.10  2011/05/12 01:47:06  dark264sh
 * *** empty log message ***
 *
 * Revision 1.9  2011/05/12 00:42:09  dark264sh
 * *** empty log message ***
 *
 * Revision 1.8  2011/05/11 15:25:01  dark264sh
 * *** empty log message ***
 *
 * Revision 1.7  2011/05/11 07:46:06  jsyoon
 * *** empty log message ***
 *
 * Revision 1.6  2011/05/10 18:08:09  dark264sh
 * A_ITCP: buffering 처리
 *
 * Revision 1.5  2011/05/09 09:58:28  dark264sh
 * A_ITCP: A_CALL multi 처리
 *
 * Revision 1.4  2011/04/24 11:31:32  dark264sh
 * *** empty log message ***
 *
 * Revision 1.3  2011/04/24 11:27:36  dark264sh
 * A_ITCP: Call, Session, 사용량 log_print 추가
 *
 * Revision 1.2  2011/04/20 10:16:29  dark264sh
 * A_ITCP: TCP와 메모리 충돌 버그 수정
 *
 * Revision 1.1  2011/04/12 02:51:50  dark264sh
 * A_ITCP 추가
 *
 */
