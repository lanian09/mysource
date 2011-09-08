/**		@file	widget_main.c
 * 		- WIDGET Transaction을 관리 하는 프로세스
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: widget_main.c,v 1.2 2011/09/06 12:46:41 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.2 $
 * 		@date		$Date: 2011/09/06 12:46:41 $
 * 		@warning	.
 * 		@ref		widget_main.c widget_init.c
 * 		@todo		library가 만들어지지 않은 상태, library 생성 이후 함수 대치
 *
 * 		@section	Intro(소개)
 * 		- EMS Transaction을 관리 하는 프로세스
 *
 * 		@section	Requirement
 * 		 @li library 생성 이후 함수 대치
 *
 **/

/**
 * Include headers
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>

// TOP
#include "commdef.h"
#include "procid.h"
#include "common_stg.h"
#include "path.h"
#include "sshmid.h"

// LIB
#include "gifo.h"
#include "nifo.h"
#include "loglib.h"
#include "verlib.h"

// TAF
#include "widget.h"

// .
#include "widget_init.h"
#include "widget_msgq.h"

/**
 * Declare variables
 */
S32				giFinishSignal;		/**< Finish Signal */
S32				giStopFlag;			/**< main loop Flag 0: Stop, 1: Loop */

stMEMSINFO		*pMEMSINFO;			/**< new interface 관리 구조체 */
stCIFO			*gpCIFO;

S32				gACALLCnt = 0;

/**
 * Declare functions
 */
char *PrintTYPE(S32 type);
S32 dCheckAppFailCode(char *sp);
S32 dGetCALLProcID(U32 uiClientIP);

/**
 *	Implement func.
 */

char *PrintTYPE(S32 type)
{
	switch(type)
	{
	case LOG_HTTP_TRANS_DEF_NUM:
		return "LOG_HTTP_TRANS";
	case HTTP_REQ_HDR_NUM:
		return "HTTP REQ HDR";
	case HTTP_REQ_BODY_NUM:
		return "HTTP REQ BODY";
	case HTTP_RES_HDR_NUM:
		return "HTTP RES HDR";
	case HTTP_RES_BODY_NUM:
		return "HTTP RES BODY";
	default:
		return "UNKNOWN";
	}
}

S32 dCheckAppFailCode(char *sp)
{
	int	dRet;
	int	fail = atoi(sp);

	switch(fail)
	{
	case 0:
	case 2001:
	case 2002:
	case 2777:
	case 2778:
		dRet = 0;
		break;
	default:
		dRet = 1;
		break;
	}

	return dRet;
}

S32 dGetCALLProcID(U32 uiClientIP)
{
	return SEQ_PROC_A_CALL + ( uiClientIP % gACALLCnt );
}

/** main function.
 *
 *  man Function
 *
 *  @param	argc	:	파라미터 개수
 *  @param	*argv[]	:	파라미터
 *
 *  @return			S32
 *  @see			http_main.c l4.h http_init.c http_func.c
 *
 *  @exception		.
 *  @note			.
 **/
S32 main(S32 argc, S8 *argv[])
{
	S32				dRet;		/**< 함수 Return 값 */
	OFFSET			offset;
	U8				*pNode;
	U8				*pNextNode;
	U8				*p, *data;
	S32				type, len, ismalloc;
	U8				*pBodyNode;
	U16				URLbuf_len;
	U8 				URLbuf[BUFSIZ];
	BODY			*pBODY;
	int				msgtype, isAppFail, isAck;

	char 			*pdata_req_hdr;
	char 			*pdata_req_body;
	char 			*pdata_resp_hdr;
	char 			*pdata_resp_body;
	LOG_HTTP_TRANS	*pLOG_HTTP_TRANS;
	int 			data_req_hdr_len;
	int 			data_req_body_len;
	int 			data_resp_hdr_len;
	int 			data_resp_body_len;

    char    vERSION[7] = "R3.0.0";


	/* log_print 초기화 */
	log_init(S_SSHM_LOG_LEVEL, getpid(), SEQ_PROC_A_WIDGET, LOG_PATH"/A_WIDGET", "A_WIDGET");

	/* A_WIDGET 초기화 */
	if((dRet = dInitWIDGET(&pMEMSINFO)) < 0)
	{
		log_print(LOGN_CRI, LH"dInitWIDGET dRet[%d]", LT, dRet);
		exit(0);
	}

    if((dRet = set_version(S_SSHM_VERSION, SEQ_PROC_A_WIDGET, vERSION)) < 0 ) {
        log_print(LOGN_CRI, "SET_VERSION ERROR(RET=%d,IDX=%d,VER=%s)", dRet, SEQ_PROC_A_WIDGET, vERSION);
    }
	log_print(LOGN_CRI, "START WIDGET(%s)", vERSION);

	/* MAIN LOOP */
	while(giStopFlag)
	{
		if((offset = gifo_read(pMEMSINFO, gpCIFO, SEQ_PROC_A_WIDGET)) > 0 ){

			log_print(LOGN_INFO, "======================================================================");

			/* DB LOG 전송을 목적으로 하는 NODE (삭제 하지 않고 전송하기 위함 )*/
			pNode = nifo_ptr(pMEMSINFO, offset);
			pNextNode = pNode;

			pdata_req_hdr = NULL;
			pdata_req_body = NULL;
			pdata_resp_hdr = NULL;
			pdata_resp_body = NULL;
			pLOG_HTTP_TRANS = NULL;
			pBodyNode = NULL;
			URLbuf_len = 0;
			URLbuf[0] = 0x00; 
			pBODY = NULL;

			do {
				p = pNextNode;

				while(p != NULL) {
					if((dRet = nifo_read_tlv_cont(pMEMSINFO, pNextNode, (U32*)&type, (U32*)&len, &data, &ismalloc, &p)) < 0)
						break;

					log_print(LOGN_INFO, "####################################################################");
					log_print(LOGN_INFO, "TYPE[%d][%s] LEN[%d] ISMALLOC[%s]", 
						type, PrintTYPE(type), len, 
						(ismalloc == DEF_READ_MALLOC) ? "MALLOC MEM" : "ORIGIN MEM");

					switch(type)
					{
					case LOG_HTTP_TRANS_DEF_NUM:
						pLOG_HTTP_TRANS = (LOG_HTTP_TRANS *) data;
						isAck = 0;
						widgetack((char*)pLOG_HTTP_TRANS->szLOGURL, pLOG_HTTP_TRANS->usLOGURLSize, &isAck);
						switch(isAck)
						{
						case 1:
							pLOG_HTTP_TRANS->usSvcL4Type = L4_WIDGET_NODN;
							break;
						case 2:
							pLOG_HTTP_TRANS->usSvcL7Type = APP_WIDGET_WDL;
							break;
						case 3:
							pLOG_HTTP_TRANS->usSvcL7Type = APP_WIDGET_WIN;
							break;
						}

//						LOG_HTTP_TRANS_Prt("PRINT LOG_HTTP_TRANS1", pLOG_HTTP_TRANS);
						// URL에서 port 제거한 buffer한개 가지고 있기 : shlee
						// Set URLbuf  URLbuf_len
						dRet = Remake_URL_Buf(
									pLOG_HTTP_TRANS->usURLSize, pLOG_HTTP_TRANS->szURL, &URLbuf_len, URLbuf);
						if(dRet < 0) {
							log_print(LOGN_CRI, "%s REMAKE_URL_BUF Err[%d]", (char *)__FUNCTION__, dRet);
							URLbuf_len = 0;
							URLbuf[0] = 0x00;
						}
						break;
					case HTTP_REQ_HDR_NUM:
						pdata_req_hdr = (char *) data;
						data_req_hdr_len = len;
						log_print(LOGN_INFO, "TYPE[%s] len %d DATA[\n%.*s]", PrintTYPE(type), len ,len, data);				
						if(pLOG_HTTP_TRANS){
							if(pdata_req_hdr && data_req_hdr_len){
								LOG_HTTP_TRANS_WIPI_REQ_HDR_LEX(pdata_req_hdr,data_req_hdr_len,(char *)pLOG_HTTP_TRANS);
								/* Setting L7TYPE */
								msgtype = APP_UNKNOWN;
								widgetmsgtype((char*)pLOG_HTTP_TRANS->szMSGTYPE, MAX_MSGTYPE_LEN, &msgtype);

								if(msgtype != APP_UNKNOWN) 
								{
									if(pLOG_HTTP_TRANS->usSvcL7Type != APP_UNKNOWN) {
										log_print(LOGN_CRI, "DUP MSGTYPE DATA=%.*s", data_req_hdr_len, pdata_req_hdr);
									}	
									pLOG_HTTP_TRANS->usSvcL7Type = msgtype;
								}
								else if((pLOG_HTTP_TRANS->usSvcL4Type == L4_WIDGET) && (pLOG_HTTP_TRANS->usSvcL7Type == APP_UNKNOWN))
							 	{
									log_print(LOGN_CRI, "NO MSGTYPE DATA=%.*s", data_req_hdr_len, pdata_req_hdr);
								}

//								LOG_HTTP_TRANS_Prt("PRINT LOG_HTTP_TRANS_WIDGET_REQ_HDR", pLOG_HTTP_TRANS);
							}
						} else {
							log_print(LOGN_CRI, "ERROR %s : LOG_HTTP_TRANS가 존재하지 않는다!!!",PrintTYPE(type));
						}
						break;
					case HTTP_REQ_BODY_NUM:
						pdata_req_body = (char *) data;
						data_req_body_len = len;
						log_print(LOGN_INFO, "TYPE[%s] len %d DATA[\n%.*s]", PrintTYPE(type), len ,len, data);				
						if(pLOG_HTTP_TRANS){
							if(pdata_req_body && data_req_body_len) {
							}
						} else {
							log_print(LOGN_CRI, "ERROR %s : LOG_HTTP_TRANS가 존재하지 않는다!!!",PrintTYPE(type));
						}
						break;
					case HTTP_RES_HDR_NUM:
						pdata_resp_hdr = (char *) data;
						data_resp_hdr_len = len;
						log_print(LOGN_INFO, "TYPE[%s] len %d DATA[\n%.*s]", PrintTYPE(type), len ,len, data);				
						if(pLOG_HTTP_TRANS){
							if(pdata_resp_hdr && data_resp_hdr_len){
								LOG_HTTP_TRANS_WIPI_RESP_HDR_LEX(pdata_resp_hdr,data_resp_hdr_len,(char *)pLOG_HTTP_TRANS);
								/* for AppFailCode */
								isAppFail = dCheckAppFailCode((char*)pLOG_HTTP_TRANS->szAppFailCode);
								if((pLOG_HTTP_TRANS->usUserErrorCode == 0) && (isAppFail)) {
									pLOG_HTTP_TRANS->usUserErrorCode = HTTP_UERR_980;
									pLOG_HTTP_TRANS->usL7FailCode = HTTP_UERR_980;
								}
//								LOG_HTTP_TRANS_Prt("PRINT LOG_HTTP_TRANS_WIDGET_RES_HDR", pLOG_HTTP_TRANS);
							}
						} else {
							log_print(LOGN_CRI, "ERROR %s : LOG_HTTP_TRANS가 존재하지 않는다!!!",PrintTYPE(type));
						}
						break;
					case HTTP_RES_BODY_NUM:
						pdata_resp_body = (char *) data;
						data_resp_body_len = len;
						log_print(LOGN_INFO, "TYPE[%s] len %d DATA[\n%.*s]", PrintTYPE(type), len ,len, data);				
						if(pLOG_HTTP_TRANS){
							if(pdata_resp_body && data_resp_body_len) {
							}
						} else {
							log_print(LOGN_CRI, "ERROR %s : LOG_HTTP_TRANS가 존재하지 않는다!!!",PrintTYPE(type));
						}
						break;
					default:
						log_print(LOGN_INFO, "????? UNKNOWN TYPE[%d]", type);
						break;
					}

					if(ismalloc == DEF_READ_MALLOC){ free(data); }
				}
				
				pNextNode = (U8 *)nifo_entry(nifo_ptr(pMEMSINFO, ((NIFO *)pNextNode)->nont.offset_next), NIFO, nont);

			} while(pNode != pNextNode);

			if(pLOG_HTTP_TRANS){
				/* DATA NODE DELETE */
				pNextNode = (U8 *)nifo_entry(nifo_ptr(pMEMSINFO, ((NIFO *)pNode)->nont.offset_next), NIFO, nont);
				nifo_node_unlink_nont(pMEMSINFO, pNode);
				nifo_node_delete(pMEMSINFO, pNextNode);
				if(pBodyNode){
					nifo_node_link_nont_prev(pMEMSINFO, pNode, pBodyNode);
				}
				if((dRet = dSend_WIDGET_Data(pMEMSINFO, dGetCALLProcID(pLOG_HTTP_TRANS->uiClientIP), pNode)) < 0) {
					log_print(LOGN_CRI, LH"MSGQ WRITE FAILE[%d][%s]", LT, dRet, strerror(-dRet));
					break;
				}
			} else {
				log_print(LOGN_CRI,"WIDGET : pLOG_HTTP_TRANS is NULL");
				/* Node 삭제 */
				nifo_node_delete(pMEMSINFO, pNode);
			}

		} else {
			usleep(0);
		}
			
	}

	FinishProgram();

	return 0;
}


/*
 *  $Log: widget_main.c,v $
 *  Revision 1.2  2011/09/06 12:46:41  hhbaek
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *  NEW OAM SYSTEM
 *
 *  Revision 1.3  2011/08/17 13:16:31  hhbaek
 *  A_WIDGET
 *
 *  Revision 1.2  2011/08/09 08:17:42  uamyd
 *  add blocks
 *
 *  Revision 1.1.1.1  2011/08/05 00:27:18  uamyd
 *  init DQMS2
 *
 *  Revision 1.7  2011/05/09 13:45:07  dark264sh
 *  A_WIDGET: A_CALL multi 처리
 *
 *  Revision 1.6  2011/01/11 04:09:11  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2010/08/23 01:13:01  uamyd
 *  DQMS With TOTMON, 2nd-import
 *
 *  Revision 1.5  2009/08/19 12:31:13  pkg
 *  LOG_XXX_Prt 함수 주석 처리
 *
 *  Revision 1.4  2009/08/06 06:56:09  dqms
 *  로그레벨 공유메모리로 수정
 *
 *  Revision 1.3  2009/07/15 17:10:56  dqms
 *  set_version 위치 및 Plastform Type 변경
 *
 *  Revision 1.2  2009/06/28 12:57:45  dqms
 *  ADD set_version
 *
 *  Revision 1.1.1.1  2009/05/26 02:14:47  dqms
 *  Init TAF_RPPI
 *
 *  Revision 1.6  2009/01/08 06:55:20  bgpark
 *  objectdownload, installnoti L7 정의
 *
 *  Revision 1.5  2008/12/19 09:56:49  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.4  2008/12/18 06:03:31  dark264sh
 *  WIDGET Result-code 없는 경우 에러 처리
 *
 *  Revision 1.3  2008/12/17 12:24:03  dark264sh
 *  WIDGET Ack Msg인 경우 L4CODE 변경
 *
 *  Revision 1.2  2008/12/17 08:47:04  dark264sh
 *  WIDGET Ack Msg에 대한 failcode 세팅 변경
 *
 *  Revision 1.1  2008/11/25 12:45:57  dark264sh
 *  WIDGET 처리
 *
 */



