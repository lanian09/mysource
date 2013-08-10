/**		@file	tcp_main.c
 * 		- TCP Session을 관리 하는 프로세스
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: tcp_main.c,v 1.3 2011/09/07 06:30:48 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.3 $
 * 		@date		$Date: 2011/09/07 06:30:48 $
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

/* SYS HEADER */
#include <unistd.h>
#include <tcp_sess.h>
#include <debug.h>
/* LIB HEADER */
#include "commdef.h"
#include "mems.h"
#include "cifo.h"
#include "gifo.h"
#include "loglib.h"
#include "utillib.h"
#include "verlib.h"
#include "ipclib.h"
/* PRO HEADER */
#include "common_stg.h"
#include "procid.h"
#include "sshmid.h"
#include "path.h"
/* TAM HEADER */
/* TAF HEADER */
#include "filter.h"
#include "func_time_check.h"
/* OAM HEADER */
/* LOC HEADER */
#include "tcp_msgq.h"
#include "tcp_func.h"
#include "tcp_init.h"
#include "tcp_util.h"

stCIFO			*gpCIFO;
stMEMSINFO		*pMEMSINFO;
stHASHOINFO		*pTCPHASH;
stTIMERNINFO	*pTIMER;

stHASHOINFO				*pSESSKEYINFO;  	/* TCP SESSION 정리를 위한 HASH */
extern stHASHONODE 		*pSESSKEYNODE;

st_Flt_Info 		*flt_info;

S32				giFinishSignal;				/**< Finish Signal */
S32				giStopFlag;					/**< main loop Flag 0: Stop, 1: Loop */
UINT 			guiSeqProcID;

DEBUG_INFO		*pDEBUGINFO;
ATCP_SUBINFO	ATCPSUBINFO;
ATCP_SUBINFO	*pATCPSUBINFO = &ATCPSUBINFO;

st_FuncTimeCheckList	stFuncTimeCheckList;
st_FuncTimeCheckList	*pFUNC = &stFuncTimeCheckList;

/* FOR MULTIPLE PROCESS */
char 			gszMyProc[32];

/* IP SESSION MANAGER 	*/
pSESSKEY_TBL 	pstSESSKEYTbl;

extern S32 dGetProcID(U16 usAppCode, UINT uiClientIP);

int main(int argc, char *argv[])
{
	S32				i;
	S32				dRet;
	OFFSET			offset;
	time_t			oldTime, nowTime;

	Capture_Header_Msg	*pCAPHEAD;
	INFO_ETH			*pINFOETH;
	CALL_KEY 			*pCALLKey;
	U8					*pNode;

	UINT 				dSndQID;
	SESSKEY_LIST 		*pstSESSKEYList;
	char 				szLOGPATH[128];
	int 				dLen, PROCNO;
	int 				LastLogTime;
	
	char    vERSION[7] = "R3.0.0";

	/* process name */
	memcpy(&gszMyProc[0], argv[0], strlen(argv[0]));

	dLen = strlen(gszMyProc);
	PROCNO = atoi(&gszMyProc[dLen-1]);
	guiSeqProcID = SEQ_PROC_A_TCP0 + PROCNO;

	/* log_print 초기화 */
	sprintf(szLOGPATH, LOG_PATH"/%s", gszMyProc);
	log_init(S_SSHM_LOG_LEVEL, getpid(), guiSeqProcID, szLOGPATH, gszMyProc);
	
	/* A_TCP 초기화 */
	if((dRet = dInitTcp(&pMEMSINFO, &pTCPHASH, &pTIMER)) < 0) {
		log_print(LOGN_CRI, "[%s][%s.%d] dInitTcp dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		exit(0);
	}

	if( (dRet = shm_init(S_SSHM_UTIL, sizeof(DEBUG_INFO), (void**)&pDEBUGINFO)) < 0 ){
		log_print(LOGN_CRI, "FAILED IN shm_init(DEBUG_INFO=%d)", S_SSHM_UTIL);
		exit(0);
	}

	if((dRet = set_version(S_SSHM_VERSION, guiSeqProcID, vERSION)) < 0 ) {
		log_print(LOGN_CRI, "SET_VERSION ERROR(RET=%d,IDX=%d,VER=%s)", dRet, guiSeqProcID, vERSION);
	}
	log_print(LOGN_CRI, "START %s[%d] TCP_SESS_CNT[%d]", gszMyProc, PROCNO, TCP_SESS_CNT);

	/* MAIN LOOP */
	while(giStopFlag)
	{
START_FUNC_TIME_CHECK(pFUNC, 0);
		timerN_invoke(pTIMER);
END_FUNC_TIME_CHECK(pFUNC, 0);

		for(i=0; i < 300; i++) {
			if((offset = gifo_read(pMEMSINFO, gpCIFO, guiSeqProcID)) > 0) {
				pCAPHEAD = (Capture_Header_Msg *)nifo_get_value(pMEMSINFO, CAP_HEADER_NUM, offset);
				pINFOETH = (INFO_ETH *)nifo_get_value(pMEMSINFO, INFO_ETH_NUM, offset);
				pCALLKey = (CALL_KEY *)nifo_get_value(pMEMSINFO, CLEAR_CALL_NUM, offset);
				pNode = nifo_ptr(pMEMSINFO, offset);
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
					nifo_node_delete(pMEMSINFO, pNode);
				}
				else if((pCAPHEAD == NULL) || (pINFOETH == NULL)) {
					nifo_node_delete(pMEMSINFO, pNode);
				}
				else {
					/* ETC Traffic To A_SIPM */
					//////////////////////////////////////////////////////////////////////////////////////////////////
					if(pINFOETH->usAppCode == SEQ_PROC_A_VT) {////////////////////////////////////////////////////////
						//////////////////////////////////////////////////////////////////////////////////////////////
						log_print(LOGN_DEBUG, "BYPASS ETC TRAFFIC TO A_SIPT");////////////////////////////////////////
						dSend_TCP_Data(pMEMSINFO, SEQ_PROC_A_SIPT, pNode);////////////////////////////////////////////
						//////////////////////////////////////////////////////////////////////////////////////////////
					}
					else if(pINFOETH->stIP.ucProtocol == DEF_PROTOCOL_TCP) {
						if(dProcTcpSess(pMEMSINFO, pTCPHASH, pTIMER, pCAPHEAD, pINFOETH, pNode) < 0) {
							nifo_node_delete(pMEMSINFO, pNode);
						}
					} else {
						if(pCAPHEAD->bRtxType == DEF_FROM_CLIENT)
							dSndQID = dGetProcID(pINFOETH->usAppCode, pINFOETH->stIP.dwSrcIP);
						else
							dSndQID = dGetProcID(pINFOETH->usAppCode, pINFOETH->stIP.dwDestIP);

						dSend_TCP_Data(pMEMSINFO, dSndQID, pNode);
					}
				}
			} else {
				usleep(0);
				break;
			}
		}
#if 0
		nowTime = time(NULL);
		if(nowTime >= oldTime + 60) {
			int hashoCnt = hasho_get_occupied_node_count(pTCPHASH);
			log_print(LOGN_CRI, 
				"SESS CUR=%lld SESS=%lld RNODE=%lld:%.3fMbps SND=%lld HTTP=%lld ONLI=%lld DIFSEQ=%lld UPD=%lld C7=%lld AFT=%lld HASH=%u:%.2f%%", 
				curSessCnt, sessCnt, rcvNodeCnt, (double)((rcvSize * 8) / (((nowTime - oldTime) == 0) ? 1 : (nowTime - oldTime))) / (1024*1024), allSndCnt, httpSndCnt, onlineSndCnt, diffSeqCnt, updateCnt, case7Cnt, afterCnt, hashoCnt, (float)((float)hashoCnt / ((curSessCnt == 0) ? 1 : curSessCnt)) * 100.0);
			rcvNodeCnt = 0;
			rcvSize = 0;
			httpSndCnt = 0;
			onlineSndCnt = 0;
			allSndCnt = 0;
			diffSeqCnt = 0;
			sessCnt = 0;
			updateCnt = 0;
			case7Cnt = 0;
			afterCnt = 0;
			oldTime = nowTime;
PRINT_FUNC_TIME_CHECK(pFUNC);
		}
#endif
		nowTime = time(NULL);
		if(nowTime >= oldTime + 60) {
			memcpy(&pDEBUGINFO->ATCPINFO[PROCNO].ATCPSUBINFO, pATCPSUBINFO, sizeof(ATCP_SUBINFO));
			pDEBUGINFO->ATCPINFO[PROCNO].curtime = nowTime;
			pATCPSUBINFO->rcvNodeCnt = 0;
			pATCPSUBINFO->rcvSize = 0;
			pATCPSUBINFO->httpSndCnt = 0;
			pATCPSUBINFO->delCnt = 0;
			pATCPSUBINFO->allSndCnt = 0;
			pATCPSUBINFO->diffSeqCnt = 0;
			pATCPSUBINFO->sessCnt = 0;
			pATCPSUBINFO->updateCnt = 0;
			oldTime = nowTime;
PRINT_FUNC_TIME_CHECK(pFUNC);
		}
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
			"@@@ INVOKE TIMEOUT SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]CTIME[%u]UTIME[%u]NTIME[%ld]", 
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
		log_print(LOGN_CRI, "INVOKE TIMEOUT BUT NODE NULL SIP[%s]SPORT[%u]DIP[%s]DPORT[%u]NTIME[%ld]", 
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
 * $Log: tcp_main.c,v $
 * Revision 1.3  2011/09/07 06:30:48  hhbaek
 * *** empty log message ***
 *
 * Revision 1.2  2011/09/06 06:44:52  dcham
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * NEW OAM SYSTEM
 *
 * Revision 1.4  2011/08/21 09:07:53  hhbaek
 * Commit TAF/SRC/ 
 *
 * Revision 1.3  2011/08/17 07:25:47  dcham
 * *** empty log message ***
 *
 * Revision 1.2  2011/08/10 09:57:44  uamyd
 * modified and block added
 *
 * Revision 1.1.1.1  2011/08/05 00:27:18  uamyd
 * init DQMS2
 *
 * Revision 1.23  2011/05/12 01:52:29  dark264sh
 * *** empty log message ***
 *
 * Revision 1.22  2011/05/09 15:03:35  dark264sh
 * A_TCP: A_CALL multi 처리
 *
 * Revision 1.21  2011/01/11 04:09:10  uamyd
 * modified
 *
 * Revision 1.1.1.1  2010/08/23 01:13:01  uamyd
 * DQMS With TOTMON, 2nd-import
 *
 * Revision 1.20  2009/08/18 14:53:42  pkg
 * A_TCP timer del 버그 수정
 *
 * Revision 1.19  2009/08/17 17:55:35  jsyoon
 * CALL 세션관리 리스트 버그 수정
 *
 * Revision 1.18  2009/08/04 12:08:17  dqms
 * TIMER를 공유메모리로 변경
 *
 * Revision 1.17  2009/08/01 08:46:11  dqms
 * *** empty log message ***
 *
 * Revision 1.16  2009/07/31 06:29:54  jsyoon
 * *** empty log message ***
 *
 * Revision 1.15  2009/07/31 06:17:35  jsyoon
 * RADIUS Continue Session 처리
 *
 * Revision 1.14  2009/07/29 07:02:34  dqms
 * *** empty log message ***
 *
 * Revision 1.13  2009/07/22 06:25:21  dqms
 * *** empty log message ***
 *
 * Revision 1.12  2009/07/22 05:09:59  dqms
 * *** empty log message ***
 *
 * Revision 1.11  2009/07/20 05:32:09  dqms
 * ETC 트래픽 패스 변경
 *
 * Revision 1.10  2009/07/15 16:11:13  dqms
 * 멀티프로세스 수정
 *
 * Revision 1.9  2009/06/28 12:57:45  dqms
 * ADD set_version
 *
 * Revision 1.8  2009/06/25 17:41:12  jsyoon
 * *** empty log message ***
 *
 * Revision 1.7  2009/06/19 12:11:35  jsyoon
 * MODIFIED MULTIPLE LOGGING
 *
 * Revision 1.6  2009/06/19 08:54:48  jsyoon
 * MULTIPLE HASHKEY, LOG
 *
 * Revision 1.5  2009/06/15 08:45:42  jsyoon
 * *** empty log message ***
 *
 * Revision 1.4  2009/06/13 19:04:57  jsyoon
 * *** empty log message ***
 *
 * Revision 1.3  2009/06/08 19:08:23  jsyoon
 * *** empty log message ***
 *
 * Revision 1.2  2009/06/08 18:54:42  jsyoon
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2009/05/26 02:14:34  dqms
 * Init TAF_RPPI
 *
 * Revision 1.4  2008/11/24 07:05:47  dark264sh
 * WIPI ONLINE 처리
 *
 * Revision 1.3  2008/09/18 07:47:39  dark264sh
 * IM 서비스 추가 (SIP, XCAP, MSRP)
 *
 * Revision 1.2  2008/06/18 10:39:34  dark264sh
 * A_TCP IV관련 routing 처리
 *
 * Revision 1.1.1.1  2008/06/09 08:17:16  jsyoon
 * WATAS3 PROJECT START
 *
 * Revision 1.3  2007/09/05 07:15:35  dark264sh
 * *** empty log message ***
 *
 * Revision 1.2  2007/09/03 05:29:18  dark264sh
 * *** empty log message ***
 *
 * Revision 1.1  2007/08/21 12:54:17  dark264sh
 * no message
 *
 * Revision 1.19  2007/04/18 05:27:23  dark264sh
 * 20070409 적용 TCP_SESS_CNT 변경
 *
 * Revision 1.18  2006/11/28 15:33:39  dark264sh
 * *** empty log message ***
 *
 * Revision 1.17  2006/11/28 12:58:27  cjlee
 * doxygen
 *
 * Revision 1.16  2006/11/28 12:16:18  dark264sh
 * *** empty log message ***
 *
 * Revision 1.15  2006/11/28 02:21:24  dark264sh
 * *** empty log message ***
 *
 * Revision 1.14  2006/11/10 09:28:12  dark264sh
 * *** empty log message ***
 *
 * Revision 1.13  2006/11/07 11:04:23  dark264sh
 * *** empty log message ***
 *
 * Revision 1.12  2006/11/07 07:13:56  dark264sh
 * A_TCP에 받은 SIZE, A_HTTP, A_ONLINE으로 보낸 개수 Debug용 코드 추가
 *
 * Revision 1.11  2006/11/06 07:35:29  dark264sh
 * nifo NODE size 4*1024 => 6*1024로 변경하기
 * nifo_tlv_alloc에서 argument로 memset할지 말지 결정하도록 수정
 * nifo_node_free에서 semaphore 삭제
 *
 * Revision 1.10  2006/11/03 09:33:04  dark264sh
 * *** empty log message ***
 *
 * Revision 1.9  2006/11/03 08:30:02  dark264sh
 * A_TCP에 func_time_check 추가
 *
 * Revision 1.8  2006/11/02 07:21:52  dark264sh
 * ETH 정보가 다른 것의 개수 추가
 *
 * Revision 1.7  2006/11/01 09:24:44  dark264sh
 * SESS, SEQ, NODE 개수 LOG추가
 *
 * Revision 1.6  2006/11/01 02:33:54  dark264sh
 * *** empty log message ***
 *
 * Revision 1.5  2006/10/30 00:50:47  dark264sh
 * *** empty log message ***
 *
 * Revision 1.4  2006/10/16 14:39:47  dark264sh
 * *** empty log message ***
 *
 * Revision 1.3  2006/10/12 12:56:00  dark264sh
 * *** empty log message ***
 *
 * Revision 1.2  2006/10/11 11:52:33  dark264sh
 * PRE_A, A_TCP, A_HTTP에 SVC filter 적용
 *
 * Revision 1.1.1.1  2006/10/11 07:45:49  dark264sh
 * no message
 *
 * Revision 1.14  2006/10/10 08:38:13  dark264sh
 * 에러 핸들링 추가
 *
 * Revision 1.13  2006/09/29 08:57:37  dark264sh
 * SEQ 순서를 잘못 맞추는 버그, FIN 재전송 체크 수정
 *
 * Revision 1.12  2006/09/15 02:53:44  dark264sh
 * LOG 데이터를 HTTP가 아닌 PAGE로 보내도록 수정
 * nifo_msg_read에서 현재는 자연스러운 처리가 돼지 않음.
 * 향후 nifo_msg_read를 수정해서 처리해야 할꺼 같음.
 *
 * Revision 1.11  2006/09/06 11:43:28  dark264sh
 * *** empty log message ***
 *
 * Revision 1.10  2006/09/04 06:38:44  dark264sh
 * *** empty log message ***
 *
 * Revision 1.9  2006/09/04 05:53:33  dark264sh
 * READ_VAL_LIST들을 global로 처리
 *
 * Revision 1.8  2006/08/21 10:59:24  dark264sh
 * no message
 *
 * Revision 1.7  2006/08/21 10:56:26  dark264sh
 * dInitTcp 함수의 파라미터 변경
 *
 * Revision 1.6  2006/08/21 09:40:41  dark264sh
 * L4FailCode 설정 함수 추가
 *
 * Revision 1.5  2006/08/21 09:38:45  dark264sh
 * L4FailCode 설정 함수 추가
 *
 * Revision 1.4  2006/08/21 09:36:17  dark264sh
 * L4FailCode 설정 함수 추가
 *
 * Revision 1.3  2006/08/21 05:44:43  dark264sh
 * no message
 *
 * Revision 1.2  2006/08/21 03:07:38  dark264sh
 * no message
 *
 * Revision 1.1  2006/08/18 10:17:39  dark264sh
 * TCP INIT
 *
 */
