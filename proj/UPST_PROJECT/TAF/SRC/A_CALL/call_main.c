/**		@file	call_main.c
 * 		- CALL 정보를 관리 하는 프로세스
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: call_main.c,v 1.2 2011/09/04 08:04:25 dhkim Exp $
 *
 * 		@Author		$Author: dhkim $
 * 		@version	$Revision: 1.2 $
 * 		@date		$Date: 2011/09/04 08:04:25 $
 * 		@warning	.
 * 		@ref		call_main.c call_init.c
 * 		@todo		library가 만들어지지 않은 상태, library 생성 이후 함수 대치
 *
 * 		@section	Intro(소개)
 * 		- CALL 정보를 관리 하는 프로세스
 *
 * 		@section	Requirement
 * 		 @li library 생성 이후 함수 대치
 *
 **/

#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

// LIB
#include "typedef.h"
#include "loglib.h"
#include "verlib.h"
#include "mems.h"
#include "memg.h"
#include "nifo.h"
#include "cifo.h"
#include "gifo.h"
#include "hasho.h"

#include "Analyze_Ext_Abs.h"

// PROJECT
#include "path.h"
#include "procid.h"
#include "sshmid.h"
//#include "commdef.h"
#include "filter.h"
#include "common_stg.h"
#include "capdef.h"

#include "debug.h"

// .
#include "call_init.h"
#include "call_func.h"
#include "call_msgq.h"
#include "call_utils.h"

S32					giFinishSignal;			/**< Finish Signal */
S32					giStopFlag;				/**< main loop Flag 0: Stop, 1: Loop */
S32					guiTimerValue;
S32					guiSeqProcID;

stMEMSINFO		*pMEMSINFO;			/**< new interface 관리 구조체 */
stCIFO			*gpCIFO;
stHASHOINFO		*pHASHOINFO;		/**< new interface 관리 구조체 */
stTIMERNINFO	*pTIMERNINFO;	/**< new interface 관리 구조체 */

extern U32 			gATCPCnt;
extern U32 			gAINETCnt;			/* A_INET 프로세스 */
extern U32 			gAITCPCnt;			/* A_ITCP 프로세스 */
st_Flt_Info	*flt_info;

U64		CreateSessCnt = 0;
U64		DelSessCnt = 0;

/* FOR MULTIPLE PROCESS */
char			gszMyProc[32];

/** main function.
 *
 *  man Function
 *
 *  @param	argc	:	파라미터 개수
 *  @param	*argv[]	:	파라미터
 *
 *  @return			S32
 *  @see			call_main.c call_init.c
 *
 **/
S32 main(S32 argc, S8 *argv[])
{
	S32						dRet;		/**< 함수 Return 값 */
	OFFSET					offset;
	U8						*pNode;
	U8						*pNextNode;
	U8						*p;
	S8						*data;
	U32						type, len;
	S32						ismalloc, udpFlag, callFlag, dnsFlag;
	UINT 					dSeqProcID;

#ifdef _RADIUS_ACCESS_TEST_
	U8                      *pCopyNode;
	LOG_SIGNAL 				*pCopyLOGSIGNAL;
#endif

	LOG_TCP_SESS			*pLOGTCPSESS;
	LOG_INET				*pLOGINETSESS;
	LOG_ITCP_SESS			*pLOGITCPSESS;
	LOG_HTTP_TRANS			*pLOGHTTPTRANS;
	LOG_IHTTP_TRANS			*pLOGIHTTPTRANS;
	LOG_SIGNAL 				*pLOGSIGNAL;
	LOG_DNS 				*pLOGDNS;
	TCP_INFO				*pTCPINFO;
	BODY					*pBODY;

	Capture_Header_Msg		*pCAPHEAD;
	INFO_ETH				*pINFOETH;

	stHASHONODE				*pHASHONODE;

	TAG_KEY_LOG_COMMON 		aTAG_KEY_LOG_COMMON;
	TAG_KEY_LOG_COMMON 		*pTAG_KEY_LOG_COMMON;
	CALL_SESSION_HASH_DATA 	*pCALL_SESSION_HASH_DATA;
	CALL_DUP_HASH_DATA 		*pstCallDupList;

	int						CallIndex = 0;
	
	char					szLOGPATH[128];
	int						dLen, PROCNO;
//	int						LastLogTime;

	time_t					oldTime=0, nowTime=0;

	char    vERSION[7] = "R3.0.0";

	/* process name */
	memcpy(&gszMyProc[0], argv[0], strlen(argv[0]));

	dLen = strlen(gszMyProc);
	PROCNO = atoi(&gszMyProc[dLen-1]);
	guiSeqProcID = SEQ_PROC_A_CALL0 + PROCNO;

	/* log_print 초기화 */
	sprintf(szLOGPATH, LOG_PATH"/%s", gszMyProc);
	log_init(S_SSHM_LOG_LEVEL, getpid(), guiSeqProcID, szLOGPATH, gszMyProc);
	
	/* A_CALL 초기화 */
	if((dRet = dInitCALL(&pMEMSINFO , &pHASHOINFO , &pTIMERNINFO)) < 0)
	{
		log_print(LOGN_CRI, "[%s][%s.%d] dInitCALL dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		exit(0);
	}

    if((dRet = set_version(S_SSHM_VERSION, guiSeqProcID, vERSION)) < 0 ) {
        log_print(LOGN_CRI, "SET_VERSION ERROR(RET=%d,IDX=%d,VER=%s)", dRet, guiSeqProcID, vERSION);
    }
	log_print(LOGN_CRI, "START CALL[%s]IDX[%d] SESS CALL[%d]TCP[%d]HTTP[%d]INET[%d]ITCP[%d]IHTTP[%d]", vERSION, PROCNO,
			CALL_SESS_CNT, TCP_SESS_CNT, HTTP_TRANS_CNT, INET_HASH_CNT, ITCP_SESS_CNT, IHTTP_TRANS_CNT);
	log_print(LOGN_CRI, 
			"RPPI_CALL_TIMEOUT[%d]	RPPI_WAIT_TIMEOUT[%d]	PI_VT_TIMEOUT[%d]		PI_IM_TIMEOUT[%d]\n" 	
			"PI_TCP_RSTWAIT[%d]		PI_TCP_TIMEOUT[%d]		PI_DNS_TIMEOUT[%d]		PI_SIP_TIMEOUT[%d]\n"	
			"PI_MSRP_TIMEOUT[%d]	PI_RAD_TIMEOUT[%d]		PI_DIA_TIMEOUT[%d]		PI_CALL_TIMEOUT[%d]\n"	
			"PI_WAIT_TIMEOUT[%d]	PI_DORM_TIMEOUT[%d]		RP_CALL_TIMEOUT[%d]		RP_DORM_TIMEOUT[%d]\n"	
			"PI_INET_TIMEOUT[%d]	PI_RCALL_TIMEOUT[%d]	RP_RCALL_TIMEOUT[%d]	PI_RCALL_SIGWAIT[%d]	RP_RCALL_SIGWAIT[%d]\n",
			flt_info->stTimerInfo.usTimerInfo[RPPI_CALL_TIMEOUT], flt_info->stTimerInfo.usTimerInfo[RPPI_WAIT_TIMEOUT],
			flt_info->stTimerInfo.usTimerInfo[PI_VT_TIMEOUT], flt_info->stTimerInfo.usTimerInfo[PI_IM_TIMEOUT],
			flt_info->stTimerInfo.usTimerInfo[PI_TCP_RSTWAIT], flt_info->stTimerInfo.usTimerInfo[PI_TCP_TIMEOUT],
			flt_info->stTimerInfo.usTimerInfo[PI_DNS_TIMEOUT], flt_info->stTimerInfo.usTimerInfo[PI_SIP_TIMEOUT],
			flt_info->stTimerInfo.usTimerInfo[PI_MSRP_TIMEOUT], flt_info->stTimerInfo.usTimerInfo[PI_RAD_TIMEOUT],
			flt_info->stTimerInfo.usTimerInfo[PI_DIA_TIMEOUT], flt_info->stTimerInfo.usTimerInfo[PI_CALL_TIMEOUT],
			flt_info->stTimerInfo.usTimerInfo[PI_WAIT_TIMEOUT], flt_info->stTimerInfo.usTimerInfo[PI_DORM_TIMEOUT],
			flt_info->stTimerInfo.usTimerInfo[RP_CALL_TIMEOUT], flt_info->stTimerInfo.usTimerInfo[RP_DORM_TIMEOUT],
			flt_info->stTimerInfo.usTimerInfo[PI_INET_TIMEOUT], flt_info->stTimerInfo.usTimerInfo[PI_RCALL_TIMEOUT],
			flt_info->stTimerInfo.usTimerInfo[RP_RCALL_TIMEOUT], flt_info->stTimerInfo.usTimerInfo[PI_RCALL_SIGWAIT], flt_info->stTimerInfo.usTimerInfo[PI_RCALL_SIGWAIT]);

	/* MAIN LOOP */
	while(giStopFlag)
	{
	 	timerN_invoke(pTIMERNINFO);

		nowTime = time(NULL);
		if(nowTime >= oldTime + 60) {
			log_print( LOGN_CRI, "### ALLOC SESS[%d] CALL SESS CREATE[%llu] DEL[%llu]", 
					((stMEMGINFO *)HASHO_PTR(pHASHOINFO, pHASHOINFO->offset_memginfo))->uiMemNodeAllocedCnt, 
					CreateSessCnt, DelSessCnt);

			CreateSessCnt = 0;
			DelSessCnt = 0;
			oldTime = nowTime;
		}

		//if((offset = nifo_msg_read(pMEMSINFO, dMyQID, NULL)) > 0) {
		if((offset = gifo_read(pMEMSINFO, gpCIFO, guiSeqProcID)) > 0) {

			log_print(LOGN_INFO, "======================================================================");

			/* DB LOG 전송을 목적으로 하는 NODE (삭제 하지 않고 전송하기 위함 )*/
			pNode = nifo_ptr(pMEMSINFO, offset);
#ifdef MEM_TEST
			nifo_node_delete(pMEMSINFO, pNode);
			continue;
#endif
			pNextNode = pNode;

			pLOGTCPSESS = NULL;
			pLOGINETSESS = NULL;
			pLOGITCPSESS = NULL;
			pLOGHTTPTRANS = NULL;
			pLOGIHTTPTRANS = NULL;
			pLOGSIGNAL = NULL;
			pLOGDNS = NULL;
			pTCPINFO = NULL;
			pBODY = NULL;
			pCAPHEAD = NULL;
			pINFOETH = NULL;
			udpFlag = 0;
			callFlag = 0;
			dnsFlag = 0;
			CallIndex = 0;

			do {
				p = pNextNode;

				while(p != NULL) {
					if((dRet = nifo_read_tlv_cont(pMEMSINFO, pNextNode, &type, &len, (U8 **)&data, &ismalloc, &p)) < 0)
						break;

					log_print(LOGN_INFO, "RECV : NIFO TYPE[%d][%s] LEN[%d] ISMALLOC[%s]", type, 
							((type==START_CALL_NUM || type==STOP_CALL_NUM || type==RADIUS_START_NUM || type==LOG_PISIGNAL_DEF_NUM || 
							  type==START_RP_SIG_RECALL_NUM || type==START_PI_DATA_RECALL_NUM ||
							  type==START_RP_DATA_RECALL_NUM || type==STOP_RP_RECALL_NUM) 
							 ? PRINT_TAG_DEF_ALL_CALL_INPUT(type) : PRINT_DEF_NUM_table_log(type)), 
							len, (ismalloc == DEF_READ_MALLOC) ? "MALLOC MEM" : "ORIGIN MEM");

					switch(type)
					{
						/*
						 * 시그널분석 로그 
						 */
						case START_PI_DATA_RECALL_NUM:
							Call_Session_Process(type,len,data);
//							LOG_SIGNAL_Prt("PI DATA START RECALL", (LOG_SIGNAL *)data);
							callFlag=1;
							break;
						case START_RP_DATA_RECALL_NUM:
//							LOG_SIGNAL_Prt("RP DATA START RECALL", (LOG_SIGNAL *)data);
							break;
						case START_RP_SIG_RECALL_NUM:
//							LOG_SIGNAL_Prt("RP SIG START RECALL", (LOG_SIGNAL *)data);
							break;
						case START_PI_SIG_RECALL_NUM:
							Call_Session_Process(type,len,data);
//							LOG_SIGNAL_Prt("PI SIG START RECALL", (LOG_SIGNAL *)data);
							break;
						case STOP_PI_RECALL_NUM:
							Call_Session_Process(type,len,data);
//							LOG_SIGNAL_Prt("PI SIG STOP RECALL", (LOG_SIGNAL *)data);
							break;
						case STOP_RP_RECALL_NUM:
//							LOG_SIGNAL_Prt("RP SIG STOP RECALL", (LOG_SIGNAL *)data);
							break;
						case START_CALL_NUM:
//							Call_Session_Process(type,len,data);
//							LOG_SIGNAL_Prt("A11 START CALL", (LOG_SIGNAL *)data);
							break;
						case STOP_CALL_NUM:
//							Call_Session_Process(type,len,data);
//							LOG_SIGNAL_Prt("A11 STOP CALL", (LOG_SIGNAL *)data);
							break;
						case RADIUS_START_NUM:
							Call_Session_Process(type,len,data);
//							LOG_SIGNAL_Prt("RADIUS START", (LOG_SIGNAL *)data);
							callFlag=1;
							break;
						case LOG_SIGNAL_DEF_NUM:
//							LOG_SIGNAL_Prt("A11 LOG_SIGNAL", (LOG_SIGNAL *)data);
							break;
						case LOG_PISIGNAL_DEF_NUM:
							pLOGSIGNAL = (LOG_SIGNAL *)data;
							if( (pLOGSIGNAL->uiProtoType == DEF_PROTOCOL_RADIUS) && 
								(pLOGSIGNAL->ucAcctType != DEF_MSG_ACCESS))  {
								Call_Session_Process(type,len,data);
							}
//							LOG_SIGNAL_Prt("LOG_SIGNAL", (LOG_SIGNAL *)data);
							break;
						/*
						 *	프로토콜 분석로그 
						 */
						case LOG_IM_SESS_DEF_NUM:
							Call_Session_Process(type,len,data);
//							LOG_IM_SESS_Prt("LOG_IM_SESS", (LOG_IM_SESS *)data);
							break;
						case LOG_VT_SESS_DEF_NUM:
							Call_Session_Process(type,len,data);
//							LOG_VT_SESS_Prt("LOG_VT_SESS", (LOG_VT_SESS *)data);
							break;
						case LOG_SIP_TRANS_DEF_NUM:
							if( ((LOG_SIP_TRANS *)data)->usSvcL4Type == L4_SIP_CSCF || 
							  	((LOG_SIP_TRANS *)data)->usSvcL4Type == L4_VT ) {
								Call_Session_Process(type,len,data);
							}
//							LOG_SIP_TRANS_Prt("LOG_SIP_TRANS", (LOG_SIP_TRANS *)data);
							break;
						case LOG_MSRP_TRANS_DEF_NUM:
							Call_Session_Process(type,len,data);
//							LOG_MSRP_TRANS_Prt("LOG_MSRP_TRANS", (LOG_MSRP_TRANS *)data);
							break;
						case LOG_IV_DEF_NUM:
							Call_Session_Process(type,len,data);
//							LOG_IV_Prt("LOG_IV_TRANS", (LOG_IV *)data);
							break;
						case LOG_VOD_SESS_DEF_NUM:
							Call_Session_Process(type,len,data);
//							LOG_VOD_SESS_Prt("LOG_VOD_SESS", (LOG_VOD_SESS *)data);
							break;
						case LOG_JNC_TRANS_DEF_NUM:
							Call_Session_Process(type,len,data);
//							LOG_JNC_TRANS_Prt("LOG_JNC_TRANS", (LOG_JNC_TRANS *)data);
							break;
						case LOG_ONLINE_TRANS_DEF_NUM:
							Call_Session_Process(type,len,data);
//							LOG_ONLINE_TRANS_Prt("LOG_ONLINE_TRANS", (LOG_ONLINE_TRANS *)data);
							break;
						case LOG_HTTP_TRANS_DEF_NUM:
							Call_Session_Process(type,len,data);
							pLOGHTTPTRANS = (LOG_HTTP_TRANS *)data;
//							LOG_HTTP_TRANS_Prt("LOG_HTTP_TRANS", pLOGHTTPTRANS);
							break;
						case LOG_IHTTP_TRANS_DEF_NUM:
							Call_Session_Process(type,len,data);
							pLOGIHTTPTRANS = (LOG_IHTTP_TRANS *)data;
//							LOG_IHTTP_TRANS_Prt("LOG_IHTTP_TRANS", pLOGIHTTPTRANS);
						case LOG_TCP_SESS_DEF_NUM:
							Call_Session_Process(type,len,data);
							pLOGTCPSESS = (LOG_TCP_SESS *)data;
//							LOG_TCP_SESS_Prt("LOG_TCP_SESS", pLOGTCPSESS);
							break;
						case LOG_ITCP_SESS_DEF_NUM:
							Call_Session_Process(type,len,data);
							pLOGITCPSESS = (LOG_ITCP_SESS *)data;
//							LOG_ITCP_SESS_Prt("LOG_ITCP_SESS", pLOGITCPSESS);
							break;
						case LOG_INET_DEF_NUM:
							Call_Session_Process(type,len,data);
							pLOGINETSESS = (LOG_INET *)data;
//							LOG_INET_SESS_Prt("LOG_INET_SESS", pLOGINETSESS);
							break;
						case LOG_DNS_DEF_NUM:
							pLOGDNS = (LOG_DNS *)data;
							Call_Session_Process(type,len,data);
//							LOG_DNS_Prt("LOG_DNS", pLOGDNS);
							break;
						case LOG_FTP_DEF_NUM:
							Call_Session_Process(type,len,data);
//							LOG_FTP_Prt("LOG_FTP", (LOG_FTP *)data);
							break;
						case TCP_INFO_DEF_NUM:
							/* TCP START/STOP */
							pTCPINFO = (TCP_INFO *)data;
							log_print(LOGN_INFO, "TCP_INFO [%d][%s] CIP: %d.%d.%d.%d", 
									pTCPINFO->cTcpFlag, PrintTcpFlag(pTCPINFO->cTcpFlag), HIPADDR(pTCPINFO->uiCliIP));
							{
								pTAG_KEY_LOG_COMMON = &aTAG_KEY_LOG_COMMON;
								pTAG_KEY_LOG_COMMON->uiClientIP = pTCPINFO->uiCliIP;

								if( (pHASHONODE = hasho_find(pHASHOINFO, (U8 *)pTAG_KEY_LOG_COMMON)) != NULL) {

									pstCallDupList = (CALL_DUP_HASH_DATA *)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);
									if(pstCallDupList->NextCallSessF) {
										if(pTCPINFO->uiCapTime >= pstCallDupList->NextCallTime) {
											CallIndex = 1;
										}
									}
									log_print(LOGN_INFO, "TCP_INFO SELECT CALL[%d] hasho_find", CallIndex);

									pCALL_SESSION_HASH_DATA = (CALL_SESSION_HASH_DATA *) 
										nifo_get_value(pMEMSINFO, CALL_SESSION_HASH_DATA_DEF_NUM, pstCallDupList->CallDupList[CallIndex]);
									if( pCALL_SESSION_HASH_DATA->isStopFlag == DEF_CALLSTATE_INIT ) {
										guiTimerValue = flt_info->stTimerInfo.usTimerInfo[PI_CALL_TIMEOUT];
										pCALL_SESSION_HASH_DATA->timerNID = 
											timerN_update(pTIMERNINFO, pCALL_SESSION_HASH_DATA->timerNID, time(NULL) + guiTimerValue);
										log_print(LOGN_INFO, "UPDATE EXIST TIMER IMSI[%s] STOPFLAG[%d] TIMEOUT[%d]", 
												pCALL_SESSION_HASH_DATA->aLOG_COMMON.szIMSI, pCALL_SESSION_HASH_DATA->isStopFlag, guiTimerValue);
									}
									if(!pCALL_SESSION_HASH_DATA->isServiceFlag && pCALL_SESSION_HASH_DATA->isCallType == DEF_CALL_NORMAL) {
										dSend_Service_Start_Signal(pCALL_SESSION_HASH_DATA, pTCPINFO->uiCapTime, pTCPINFO->uiCapMTime);
										pCALL_SESSION_HASH_DATA->isServiceFlag = 1;
									}
								}
							}
							break;
						case BODY_DEF_NUM:
							/* HTTP BODY Parsing */
							pBODY = (BODY *)data;
							log_print(LOGN_INFO, "BODY CNT[%d]", pBODY->aLIST.listcnt);
							/* BODY_Prt("CALL-BODY", pBODY); */
							break;
							/* RTP :::  UDP인 경우에 pCAPHEAD pINFOETH : 이 2가지가 존재해야 한다. */ 
						case CAP_HEADER_NUM:
							pCAPHEAD = (Capture_Header_Msg *)data;
							break;
						case INFO_ETH_NUM:
							pINFOETH = (INFO_ETH *)data;
							break;
						case ETH_DATA_NUM: /* CAPTURE RAW DATA */
							break;
						default:
							log_print(LOGN_WARN, "????? UNKNOWN TYPE[%d]", type);
							break;
					}
					if(ismalloc == DEF_READ_MALLOC){ free(data); }
				}
				pNextNode = (U8 *)nifo_entry(nifo_ptr(pMEMSINFO, ((NIFO *)pNextNode)->nont.offset_next), NIFO, nont);

			} while(pNode != pNextNode);

			if( pCAPHEAD && pINFOETH) {
				// ETC Traffic Process
				ETC_Process(pCAPHEAD ,pINFOETH);
				udpFlag = 1;
			} else if(pCAPHEAD || pINFOETH) {
				udpFlag = 1;
				log_print(LOGN_WARN, "pCAPHEAD == [%p] pINFOETH == [%p] CHECK PROCESS", pCAPHEAD, pINFOETH);
			} else if (pLOGINETSESS) {
				INET_Process(pLOGINETSESS);
			} else if(pLOGDNS) {
				/* DNS 쿼리가 최초로 성공 할 때까지의 DNS 로그만을 전송하며 
			   	성공로그를 전송한 이후에는 전송하지 않는다.  */
				pTAG_KEY_LOG_COMMON = &aTAG_KEY_LOG_COMMON;
				pTAG_KEY_LOG_COMMON->uiClientIP = pLOGDNS->uiClientIP;

				if( (pHASHONODE = hasho_find(pHASHOINFO, (U8 *)pTAG_KEY_LOG_COMMON)) != NULL) {
					pstCallDupList = (CALL_DUP_HASH_DATA *)nifo_ptr(pHASHOINFO, pHASHONODE->offset_Data);

					if(pstCallDupList->NextCallSessF) {
						if(pLOGDNS->uiCallTime >= pstCallDupList->NextCallTime) {
							CallIndex = 1;
						}
					}
					log_print(LOGN_INFO, "CHECK DNS SUCCESS FLAG : CALL[%d] LOG_DNS hasho_find", CallIndex);

					pCALL_SESSION_HASH_DATA =
						(CALL_SESSION_HASH_DATA *)nifo_get_value(pMEMSINFO, CALL_SESSION_HASH_DATA_DEF_NUM, pstCallDupList->CallDupList[CallIndex]);

					if(pCALL_SESSION_HASH_DATA->isSuccDNSFlag != 1) {
						if(pLOGDNS->ucErrorCode == 0) {
							pCALL_SESSION_HASH_DATA->isSuccDNSFlag = 1;
							log_print(LOGN_INFO, "DNS QUERY SUCCESS FLAG: %d", pCALL_SESSION_HASH_DATA->isSuccDNSFlag);
						}
					} else {
						log_print(LOGN_INFO, "DNS LOG DELETE FLAG: %d", pCALL_SESSION_HASH_DATA->isSuccDNSFlag);
						dnsFlag = 1;
					}
				}
			} else if(pLOGHTTPTRANS) {
				U8	*pBODYNODE = NULL;
				if(pBODY){
					pBODYNODE = nifo_ptr(pMEMSINFO, nifo_get_offset_node(pMEMSINFO, (U8 *) pBODY));
					nifo_node_unlink_nont(pMEMSINFO, pBODYNODE);
				}

				if((pLOGHTTPTRANS->usSvcL7Type == APP_MENU) || (pLOGHTTPTRANS->usSvcL7Type == APP_DOWN)) {
					if((dRet = Page_Process(pLOGHTTPTRANS,pBODY)) < 0) {
						log_print(LOGN_WARN, "Page_Process dRet=%d", dRet);
						if(pBODYNODE != NULL) {
							nifo_node_delete(pMEMSINFO, pBODYNODE);
						}
					}
/* java script 예외 처리
					int	js = 0;
					page_js(pLOGHTTPTRANS->szURL, pLOGHTTPTRANS->usURLSize, &js);
					if(js == 0) {
						Page_Process(pLOGHTTPTRANS,pBODY);
					} else if(pBODYNODE != NULL) {
						nifo_node_delete(pMEMSINFO, pBODYNODE);
					}
*/
				} else if(pBODYNODE != NULL) {
					nifo_node_delete(pMEMSINFO, pBODYNODE);
				}

				pBODY = NULL;
			} else if(pBODY){
				log_print(LOGN_CRI, "%s : ERROR pLOGHTTPTRANS NULL, pBODY is not NULL : unexpected condtion",
					(char *)__FUNCTION__);
			}

			if(udpFlag == 1 || callFlag == 1 || dnsFlag == 1) {
				nifo_node_delete(pMEMSINFO, pNode);
			} else {
				/* SEND CALLKEY TO A_TCP, A_SIPT */
				if(pLOGSIGNAL && (pLOGSIGNAL->uiProtoType == DEF_PROTOCOL_RADIUS) && 
						(pLOGSIGNAL->ucAcctType == DEF_MSG_ACCREQ_STOP || pLOGSIGNAL->ucAcctType == DEF_MSG_ACCREQ_LINKSTOP) && 
						(pLOGSIGNAL->ucStopFlag == 0)) {

					dSeqProcID = SEQ_PROC_A_TCP + (pLOGSIGNAL->uiClientIP % gATCPCnt);
					Send_Clear_Msg(dSeqProcID, pLOGSIGNAL->uiClientIP, 0);
					dSeqProcID = SEQ_PROC_A_ITCP + (pLOGSIGNAL->uiClientIP % gAITCPCnt);
					Send_Clear_Msg(dSeqProcID, pLOGSIGNAL->uiClientIP, 0);
					dSeqProcID = SEQ_PROC_A_INET + (pLOGSIGNAL->uiClientIP % gAINETCnt);
					Send_Clear_Msg(dSeqProcID, pLOGSIGNAL->uiClientIP, 0);
					Send_Clear_Msg(dSeqProcID, pLOGSIGNAL->uiClientIP, 0);
				}

#ifdef _RADIUS_ACCESS_TEST_
				if(pLOGSIGNAL && (pLOGSIGNAL->uiProtoType == DEF_PROTOCOL_RADIUS) && (pLOGSIGNAL->ucAcctType == DEF_MSG_ACCESS)) {
					if( (pCopyNode = nifo_node_alloc(pMEMSINFO)) == NULL )
					{
						log_print (LOGN_CRI, "[%s][%s.%d] nifo_node_alloc NULL ", __FILE__, __FUNCTION__, __LINE__);
					}
					else {
						if( (pCopyLOGSIGNAL = (LOG_SIGNAL *) nifo_tlv_alloc(
										pMEMSINFO, pCopyNode, LOG_PISIGNAL_DEF_NUM, LOG_SIGNAL_SIZE, DEF_MEMSET_ON)) == NULL )
						{
							log_print (LOGN_CRI, "[%s][%s.%d] nifo_tlv_alloc NULL ", __FILE__, __FUNCTION__, __LINE__);
							nifo_node_delete(pMEMSINFO, pCopyNode);
						}
						else {
							memcpy(pCopyLOGSIGNAL, pLOGSIGNAL, LOG_SIGNAL_SIZE);
							if((dRet = dSend_CALL_Data(pMEMSINFO, SEQ_PROC_CI_LOG, pCopyNode)) < 0) {
								log_print(LOGN_CRI, "[%s] ERROR [%s.%d] MSGQ WRITE FAILE[%d][%s]", __FILE__, __FUNCTION__, __LINE__,
										dRet, strerror(-dRet));
							}
							log_print(LOGN_INFO, "Send Copy Access Message");
						}
					}
				}
#endif
				if((dRet = dSend_CALL_Data(pMEMSINFO, SEQ_PROC_CI_LOG, pNode)) < 0) {
					log_print(LOGN_CRI, "[%s] ERROR [%s.%d] MSGQ WRITE FAILE[%d][%s]", __FILE__, __FUNCTION__, __LINE__, 
							dRet, strerror(-dRet));
					break;
				}
			}
		} else {
			usleep(0);
		}
	}

	FinishProgram();

	return 0;
}

/*
 *  $Log: call_main.c,v $
 *  Revision 1.2  2011/09/04 08:04:25  dhkim
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *  NEW OAM SYSTEM
 *
 *  Revision 1.4  2011/08/21 07:21:31  hhbaek
 *  *** empty log message ***
 *
 *  Revision 1.3  2011/08/18 04:18:30  hhbaek
 *  *** empty log message ***
 *
 *  Revision 1.2  2011/08/17 07:15:03  dcham
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 *  init DQMS2
 *
 *  Revision 1.72  2011/07/14 14:13:55  night1700
 *  *** empty log message ***
 *
 *  Revision 1.71  2011/05/12 01:32:31  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.70  2011/05/11 23:11:08  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.69  2011/05/09 15:14:06  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.68  2011/05/02 09:01:03  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.67  2011/04/30 19:50:57  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.66  2011/04/24 21:19:21  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.65  2011/04/24 16:37:02  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.64  2011/04/24 15:24:55  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.63  2011/04/23 18:59:06  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.62  2011/04/22 20:26:52  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.61  2011/04/18 15:42:03  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.60  2011/01/11 04:09:05  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2010/08/23 01:13:01  uamyd
 *  DQMS With TOTMON, 2nd-import
 *
 *  Revision 1.59  2010/05/04 12:12:25  dqms
 *  *** empty log message ***
 *
 *  Revision 1.58  2010/04/23 06:45:27  dqms
 *  TUNNEL_LINK_START/STOP 타입 추가
 *
 *  Revision 1.57  2010/03/10 13:24:03  jsyoon
 *  ADD TRACE FUNCTION IN A_L2TP
 *
 *  Revision 1.56  2009/10/08 07:06:31  pkg
 *  A_CALL CALL없는 경우 nifo node 누수 문제 수정
 *
 *  Revision 1.55  2009/09/29 13:50:42  pkg
 *  CLEAR 메세지 전송할때 현재 호의 LastPktTime을 전송함
 *
 *  Revision 1.54  2009/09/28 09:16:15  pkg
 *  NEXT호가 있는 상태에서 INVITE의 CallTime CURR호의 LastPktTime이면 NEXT호로 설정한다
 *
 *  Revision 1.53  2009/09/22 13:22:40  pkg
 *  INVITE 메세지의 CALL선택방법 변경
 *
 *  Revision 1.52  2009/09/14 17:20:27  pkg
 *  NEXT CALL이 생성되었을 때 START SERVICE SIGNAL이 전송되지 않은 버그 수정
 *
 *  Revision 1.51  2009/09/02 14:32:10  jsyoon
 *  서비스 시작 시그널에 시작시간을 설정
 *
 *  Revision 1.50  2009/08/29 16:30:49  jsyoon
 *  Call의 마지막 서비스가 WIPI DN 이면 TCP의 플랫폼타입을 DN으로 변경한다.
 *
 *  Revision 1.49  2009/08/27 17:53:29  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.48  2009/08/27 14:19:09  jsyoon
 *  NEXT CALL을 생성할 때 CURR->CallTime보다 빠른 시간이면 콜을 생성하지 않는다
 *
 *  Revision 1.47  2009/08/25 16:43:14  jsyoon
 *  Dorment콜 Invoke시 CallWaitF = 0으로 설정
 *
 *  Revision 1.46  2009/08/25 15:46:42  jsyoon
 *  DORMENT 타이머가 INVOKE 되었을때 Accounting Start Continue 메세지가 왔을 때
 *
 *  Revision 1.45  2009/08/19 18:57:25  pkg
 *  A_CALL log_print 레벨 변경
 *
 *  Revision 1.44  2009/08/17 14:32:42  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.43  2009/08/17 14:31:00  pkg
 *  *** empty log message ***
 *
 *  Revision 1.42  2009/08/16 16:13:07  jsyoon
 *  REMOVE COMPARE DEBUG_LOG
 *
 *  Revision 1.41  2009/08/16 10:29:10  jsyoon
 *  ADD DEBUG LOG
 *
 *  Revision 1.40  2009/08/15 21:04:57  pkg
 *  *** empty log message ***
 *
 *  Revision 1.39  2009/08/15 20:33:51  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.38  2009/08/15 20:27:56  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.37  2009/08/15 20:15:47  jsyoon
 *  UDP 트래픽 처리 로직 수정
 *
 *  Revision 1.36  2009/08/15 14:10:53  jsyoon
 *  처음 콜을 생성할때 노드를 생성하지 못하면 추가한 해시를 삭제한다
 *
 *  Revision 1.35  2009/08/15 13:14:05  jsyoon
 *  DELETE LOG_PRT
 *
 *  Revision 1.34  2009/08/15 12:09:39  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.33  2009/08/15 11:20:03  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.32  2009/08/15 11:12:51  jsyoon
 *  NIFO NODE FULL일때 에러처리 추가
 *
 *  Revision 1.31  2009/08/14 11:20:30  pkg
 *  A_CALL Call 정리시 pstCallDupList->CallDupList 초기화 버그
 *
 *  Revision 1.30  2009/08/11 15:41:47  dqms
 *  NEXT콜 정리할때 CURR콜 함께 정리
 *
 *  Revision 1.29  2009/08/10 19:13:49  dqms
 *  pLOG_DIALUP_SESS->SessStartTime 설정
 *
 *  Revision 1.28  2009/08/07 14:38:55  dqms
 *  Dorment 상태에서 로그가 왔을때 업데이트 하지 않음
 *
 *  Revision 1.27  2009/08/06 06:56:09  dqms
 *  로그레벨 공유메모리로 수정
 *
 *  Revision 1.26  2009/08/04 12:08:17  dqms
 *  TIMER를 공유메모리로 변경
 *
 *  Revision 1.25  2009/08/01 08:46:11  dqms
 *  *** empty log message ***
 *
 *  Revision 1.24  2009/08/01 05:40:02  dqms
 *  TIMER 함수 호출 할 때 CallTime 비교하여 현재 콜 선택
 *
 *  Revision 1.23  2009/07/31 06:17:35  jsyoon
 *  RADIUS Continue Session 처리
 *
 *  Revision 1.22  2009/07/26 09:26:59  jsyoon
 *  DNS 성공이후 전송 여부 판단 플래그 추가
 *
 *  Revision 1.21  2009/07/26 08:48:43  dqms
 *  *** empty log message ***
 *
 *  Revision 1.20  2009/07/22 06:25:21  dqms
 *  *** empty log message ***
 *
 *  Revision 1.19  2009/07/19 12:04:13  dqms
 *  타이머 업데이트 및 콜스탑 메세지 처리
 *
 *  Revision 1.18  2009/07/17 09:56:20  jsyoon
 *  CALL_STOP_NUM 메세지 처리
 *
 *  Revision 1.17  2009/07/16 15:38:50  jsyoon
 *  A_CALL 재시작 할때 포인터를 복구하도록 offset 추가
 *
 *  Revision 1.16  2009/07/16 14:31:31  dqms
 *  *** empty log message ***
 *
 *  Revision 1.15  2009/07/16 12:39:37  dqms
 *  ADD PROCESS FOR L4_PHONE_ETC
 *
 *  Revision 1.14  2009/07/15 13:17:33  dqms
 *  ADD vCALLTimerReConstruct()
 *
 *  Revision 1.13  2009/07/08 12:55:06  dqms
 *  ADD LOG_PISIGNAL_DEF_NUM
 *
 *  Revision 1.12  2009/07/07 06:30:29  dqms
 *  A11 START 와 RADIUS START 구분
 *
 *  Revision 1.11  2009/07/05 15:39:40  dqms
 *  *** empty log message ***
 *
 *  Revision 1.10  2009/06/29 11:15:29  dqms
 *  *** empty log message ***
 *
 *  Revision 1.9  2009/06/28 12:57:45  dqms
 *  ADD set_version
 *
 *  Revision 1.8  2009/06/16 15:16:00  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.7  2009/06/13 18:59:50  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.6  2009/06/13 12:24:59  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.5  2009/06/10 21:25:17  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.4  2009/06/08 02:50:18  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.3  2009/06/08 02:46:22  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.2  2009/06/05 05:30:16  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2009/05/26 02:14:22  dqms
 *  Init TAF_RPPI
 *
 *  Revision 1.14  2008/12/12 16:53:04  dark264sh
 *  LOG_SIP MIN 정보 parsing 규칙 변경
 *
 *  Revision 1.13  2008/11/24 07:03:33  dark264sh
 *  WIPI ONLINE 처리
 *
 *  Revision 1.12  2008/09/18 07:38:42  dark264sh
 *  IM 서비스 추가 (SIP, XCAP, MSRP)
 *
 *  Revision 1.11  2008/07/16 08:08:49  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.10  2008/07/15 17:57:56  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.9  2008/07/15 14:52:42  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.8  2008/07/14 07:20:22  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.7  2008/07/04 10:19:13  jsyoon
 *  IV 정보 처리 추가
 *
 *  Revision 1.6  2008/07/04 07:53:13  dark264sh
 *  LOG_TCP_SESS를 VOD/RTS로 구분하기 위한 처리
 *
 *  Revision 1.5  2008/07/02 13:16:11  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.4  2008/07/02 07:42:19  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.3  2008/07/02 06:35:06  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.2  2008/06/30 12:36:04  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2008/06/09 08:17:16  jsyoon
 *  WATAS3 PROJECT START
 *
 *  Revision 1.10  2007/10/08 04:36:01  dark264sh
 *  no message
 *
 *  Revision 1.9  2007/09/10 08:08:31  watas
 *  *** empty log message ***
 *
 *  Revision 1.8  2007/09/10 07:41:21  watas
 *  *** empty log message ***
 *
 *  Revision 1.7  2007/09/10 06:48:17  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.6  2007/09/10 06:08:14  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.5  2007/09/10 05:33:05  watas
 *  *** empty log message ***
 *
 *  Revision 1.4  2007/09/03 12:19:26  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.3  2007/09/03 05:29:47  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.2  2007/08/27 13:57:34  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.1  2007/08/21 12:52:37  dark264sh
 *  no message
 *
 *  Revision 1.65  2007/01/04 03:07:30  shlee
 *  Update PktTime using UDP Pkt
 *
 *  Revision 1.64  2006/12/28 09:51:59  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.63  2006/12/13 03:13:52  shlee
 *  PAGE Error Code
 *
 *  Revision 1.62  2006/12/08 07:42:45  shlee
 *  A_CALL의 PAGE LOG UserErrorCode를 L7FailCode가 0이상일 경우에만,L4FailCode를 보도록 변경함.
 *
 *  Revision 1.61  2006/12/06 06:37:38  shlee
 *  In MakePageError, default of switch set
 *
 *  Revision 1.60  2006/12/06 06:23:29  shlee
 *  HTTP 911 941 Error Setting
 *
 *  Revision 1.59  2006/12/05 10:36:25  shlee
 *  HTTP 970 ErrorCode -> http_err = 9 Setting
 *
 *  Revision 1.58  2006/12/05 07:36:02  shlee
 *  1305 Error Code Update
 *
 *  Revision 1.57  2006/12/04 09:24:22  shlee
 *  PAGE_DATA INTIALIZED
 *
 *  Revision 1.56  2006/12/01 06:47:37  shlee
 *  1300 UserErrorCode seperated
 *
 *  Revision 1.55  2006/12/01 06:40:14  cjlee
 *  add the response code (304)
 *
 *  Revision 1.54  2006/11/28 15:33:28  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.53  2006/11/28 12:58:27  cjlee
 *  doxygen
 *
 *  Revision 1.52  2006/11/28 12:15:43  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.51  2006/11/27 10:23:08  shlee
 *  PAGE->UserErrorCode is Setted at SendPageLog
 *
 *  Revision 1.50  2006/11/16 06:11:17  cjlee
 *  UDP RTP관련 timer_update 삭제
 *
 *  Revision 1.49  2006/11/14 09:16:53  cjlee
 *  ONLINE 추가
 *
 *  Revision 1.48  2006/11/12 12:05:21  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.47  2006/11/10 11:48:48  shlee
 *  *** empty log message ***
 *
 *  Revision 1.46  2006/11/10 11:39:24  cjlee
 *  BODY_Prt가 길어서 주석처리를 하고
 *  새로 url_cnt 만큼 만 찍도록 함.
 *  print_cnt는 몇번 들어왔는지를 check한다. (최대 몇개인지를 알수 있을 것이다.)
 *
 *  Revision 1.45  2006/11/06 07:40:36  dark264sh
 *  nifo NODE size 4*1024 => 6*1024로 변경하기
 *  nifo_tlv_alloc에서 argument로 memset할지 말지 결정하도록 수정
 *  nifo_node_free에서 semaphore 삭제
 *
 *  Revision 1.44  2006/10/31 02:54:31  cjlee
 *  TCP_INFO 추가 : timer update
 *
 *  Revision 1.43  2006/10/27 12:34:36  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.42  2006/10/27 10:24:30  cjlee
 *  RTP timer update추가
 *
 *  Revision 1.41  2006/10/27 00:47:35  cjlee
 *  One_At_Last 함수 수행 추가
 *
 *  Revision 1.40  2006/10/26 23:51:47  cjlee
 *  OpStartTime 추가
 *
 *  Revision 1.39  2006/10/26 02:48:45  cjlee
 *  *** empty log message ***
 *
 *  Revision 1.38  2006/10/26 01:09:06  cjlee
 *  *** empty log message ***
 *
 *  Revision 1.37  2006/10/25 08:27:20  shlee
 *  CALL URL Function
 *
 *  Revision 1.36  2006/10/25 07:40:49  cjlee
 *  *** empty log message ***
 *
 *  Revision 1.35  2006/10/23 07:30:54  cjlee
 *  *** empty log message ***
 *
 *  Revision 1.34  2006/10/23 07:28:46  cjlee
 *  *** empty log message ***
 *
 *  Revision 1.33  2006/10/23 07:14:21  cjlee
 *  NASName 변경
 *
 *  Revision 1.32  2006/10/23 06:38:45  cjlee
 *  *** empty log message ***
 *
 *  Revision 1.31  2006/10/23 06:03:25  cjlee
 *  *** empty log message ***
 *
 *  Revision 1.30  2006/10/23 05:51:48  cjlee
 *  *** empty log message ***
 *
 *  Revision 1.29  2006/10/23 04:53:10  cjlee
 *  *** empty log message ***
 *
 *  Revision 1.28  2006/10/23 03:00:19  cjlee
 *  RTP count 추가
 *
 *  Revision 1.27  2006/10/20 10:00:53  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.26  2006/10/20 09:06:24  shlee
 *  Make URL & LOCATION
 *
 *  Revision 1.25  2006/10/20 06:57:44  cjlee
 *  Make_Page_UserError() 추가
 *
 *  Revision 1.24  2006/10/20 03:07:07  cjlee
 *  *** empty log message ***
 *
 *  Revision 1.23  2006/10/20 03:05:50  cjlee
 *  *** empty log message ***
 *
 *  Revision 1.22  2006/10/19 03:43:43  cjlee
 *  Send_Page... () 에서 pInBOODY에서 삭제하지 않음
 *
 *  Revision 1.21  2006/10/18 09:35:15  cjlee
 *  *** empty log message ***
 *
 *  Revision 1.20  2006/10/18 09:11:22  cjlee
 *  *** empty log message ***
 *
 *  Revision 1.19  2006/10/18 09:01:21  cjlee
 *  flow관련 처리 내용 변경 (PAGE관련)
 *  1. send 뒤에서도 alloc을 한다. page의경우
 *  2. PAGE_OLD_STATE를 추가
 *  3. action후에  state를 set하는 것으로 변경
 *  4. PAgeID는 1부터..   증가는 LOG_PAGE_TRANS를 alloc한후에 증가
 *  5. HTTP->PageID도 set   위치는 state_go 후에 update하는 장소에서
 *
 *  Revision 1.18  2006/10/18 08:53:31  dark264sh
 *  nifo debug 코드 추가
 *
 *  Revision 1.17  2006/10/18 03:08:20  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.16  2006/10/18 02:33:46  cjlee
 *  *** empty log message ***
 *
 *  Revision 1.15  2006/10/18 02:22:58  cjlee
 *  page flow_PAGE_state_go 추가
 *
 *  Revision 1.14  2006/10/17 08:49:17  cjlee
 *  *** empty log message ***
 *
 *  Revision 1.13  2006/10/17 06:52:01  cjlee
 *  bug fix : infinite loop
 *
 *  Revision 1.12  2006/10/17 06:39:13  cjlee
 *  compile bug fix
 *
 *  Revision 1.11  2006/10/17 06:36:32  dark264sh
 *  pBODY unlinked
 *
 *  Revision 1.10  2006/10/17 06:26:12  cjlee
 *  *** empty log message ***
 *
 *  Revision 1.9  2006/10/16 09:11:47  cjlee
 *  *** empty log message ***
 *
 *  Revision 1.8  2006/10/16 07:37:57  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.7  2006/10/16 06:57:22  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.6  2006/10/16 06:45:01  cjlee
 *  compile 완료 : page state_go 말고 적용 완료
 *
 *  Revision 1.5  2006/10/16 01:51:04  dark264sh
 *  CALL NODE, PAGE NODE 할당 부분 추가
 *
 *  Revision 1.4  2006/10/13 09:12:47  cjlee
 *  상득씨 마무리 부탁해요
 *
 *  Revision 1.3  2006/10/12 12:56:00  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.2  2006/10/12 08:28:43  cjlee
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2006/10/11 07:45:49  dark264sh
 *  no message
 *
 *  Revision 1.8  2006/10/10 08:58:32  cjlee
 *  debug를 위한 compare추가
 *
 *  Revision 1.7  2006/10/10 07:00:29  dark264sh
 *  A_CALL에 전송하는 부분 추가
 *  nifo_node_alloc 함수 변경에 따른 변경
 *  A_TCP에서 timerN_update의 리턴으로 timerNID 업데이트 하도록 변경
 *
 *  Revision 1.6  2006/10/10 03:00:59  cjlee
 *  *** empty log message ***
 *
 *  Revision 1.5  2006/10/09 10:36:13  cjlee
 *  *** empty log message ***
 *
 *  Revision 1.4  2006/10/09 10:22:18  cjlee
 *  Is_LOG_Type() 사용 없앰
 *
 *  Revision 1.3  2006/10/09 08:53:36  cjlee
 *  *** empty log message ***
 *
 *  Revision 1.2  2006/10/09 01:59:00  dark264sh
 *  주석 추가
 *
 *  Revision 1.1  2006/10/09 01:55:58  dark264sh
 *  INIT
 *
 */



