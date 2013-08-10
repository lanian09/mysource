/**		@file	fv_main.c
 * 		- FV Transaction을 관리 하는 프로세스
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: fv_main.c,v 1.2 2011/09/04 12:16:50 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.2 $
 * 		@date		$Date: 2011/09/04 12:16:50 $
 * 		@warning	.
 * 		@ref		fv_main.c fv_init.c
 * 		@todo		library가 만들어지지 않은 상태, library 생성 이후 함수 대치
 *
 * 		@section	Intro(소개)
 * 		- FV Transaction을 관리 하는 프로세스
 *
 * 		@section	Requirement
 * 		 @li library 생성 이후 함수 대치
 *
 **/

#include <stdio.h>
#include <unistd.h>
#include <string.h>

// TOP
#include "common_stg.h"
#include "commdef.h"
#include "procid.h"
#include "sshmid.h"

// LIB
#include "mems.h"
#include "cifo.h"
#include "gifo.h"
#include "nifo.h"
#include "path.h"
#include "loglib.h"
#include "verlib.h"

// TAF
#include "debug.h"
#include "fv.h"

// .

/**
 *	Declare var.
 */
S32				giFinishSignal;		/**< Finish Signal */
S32				giStopFlag;			/**< main loop Flag 0: Stop, 1: Loop */

stMEMSINFO		*pMEMSINFO;			/**< new interface 관리 구조체 */
stCIFO			*gpCIFO;

extern int		gACALLCnt;

/**
 *	Declare extern func.
 */
extern S32 dInitFV(stMEMSINFO **pMEMSINFO);
extern S32 dSend_FV_Data(stMEMSINFO *pMEMSINFO, S32 dSndMsgQ, U8 *pNode);
extern void FinishProgram(void);

/**
 *	Implement func.
 */
U8 *PrintTYPE(S32 type)
{
	switch(type)
	{
		case LOG_HTTP_TRANS_DEF_NUM: 	return (U8*)"LOG_HTTP_TRANS";
		case HTTP_REQ_HDR_NUM: 			return (U8*)"HTTP REQ HDR";
		case HTTP_REQ_BODY_NUM: 		return (U8*)"HTTP REQ BODY";
		case HTTP_RES_HDR_NUM: 			return (U8*)"HTTP RES HDR";
		case HTTP_RES_BODY_NUM: 		return (U8*)"HTTP RES BODY";
		default:					 	return (U8*)"UNKNOWN";
	}
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
	S32				dSeqProcID;

	char 			*pdata_req_hdr;
	char 			*pdata_req_body;
	char 			*pdata_resp_hdr;
	char 			*pdata_resp_body;
	LOG_HTTP_TRANS	*pLOG_HTTP_TRANS;
	int 			data_req_hdr_len;
	int 			data_req_body_len;
	int 			data_resp_hdr_len;
	int 			data_resp_body_len;

	int				fv_svc_type;
	char        	vERSION[7] = "R3.0.0";

	/* log_print 초기화 */
	log_init(S_SSHM_LOG_LEVEL,getpid(), SEQ_PROC_A_FV, LOG_PATH"/A_FV", "A_FV");

	/* A_FV 초기화 */
	if((dRet = dInitFV(&pMEMSINFO)) < 0)
	{
		log_print(LOGN_CRI, "[%s][%s.%d] dInitFV dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		exit(0);
	}

	if((dRet = set_version(S_SSHM_VERSION, SEQ_PROC_A_FV, vERSION)) < 0 ) {
		log_print(LOGN_CRI, "SET_VERSION ERROR(RET=%d,IDX=%d,VER=%s)", dRet, SEQ_PROC_A_FV, vERSION);
	}
	log_print(LOGN_CRI, "START FV (%s)", vERSION);

	/* MAIN LOOP */
	while(giStopFlag)
	{
		if((offset = gifo_read(pMEMSINFO, gpCIFO, SEQ_PROC_A_FV)) > 0) {

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
//						LOG_HTTP_TRANS_Prt("PRINT LOG_HTTP_TRANS1", pLOG_HTTP_TRANS);

						fv_svc_type = APP_ETC;
						fv_type((char*)pLOG_HTTP_TRANS->szLOGURL, pLOG_HTTP_TRANS->usLOGURLSize, &fv_svc_type);
						log_print(LOGN_INFO, "FV_SVC_TYPE=%s:%d INPUT=%s", PRINT_TAG_DEF_ALL_L7CODE(fv_svc_type), fv_svc_type, pLOG_HTTP_TRANS->szLOGURL);
						pLOG_HTTP_TRANS->usSvcL7Type = fv_svc_type;

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
						log_print(LOGN_INFO, "TYPE[%s] len %d DATA[%.*s]", PrintTYPE(type), len ,len, data);				
						if(pLOG_HTTP_TRANS){
							if(pdata_req_hdr && data_req_hdr_len){
								LOG_HTTP_TRANS_WIPI_REQ_HDR_LEX(pdata_req_hdr,data_req_hdr_len,(char *)pLOG_HTTP_TRANS);
							}
						} else {
							log_print(LOGN_CRI, "ERROR %s : LOG_HTTP_TRANS가 존재하지 않는다!!!",PrintTYPE(type));
						}
						break;
					case HTTP_REQ_BODY_NUM:
						pdata_req_body = (char *) data;
						data_req_body_len = len;
						log_print(LOGN_INFO, "TYPE[%s] len %d DATA[%.*s]", PrintTYPE(type), len ,len, data);				
						if(pLOG_HTTP_TRANS){
						} else {
							log_print(LOGN_CRI, "ERROR %s : LOG_HTTP_TRANS가 존재하지 않는다!!!",PrintTYPE(type));
						}
						break;
					case HTTP_RES_HDR_NUM:
						pdata_resp_hdr = (char *) data;
						data_resp_hdr_len = len;
						log_print(LOGN_INFO, "TYPE[%s] len %d DATA[%.*s]", PrintTYPE(type), len ,len, data);				
						if(pLOG_HTTP_TRANS){
							if(pdata_resp_hdr && data_resp_hdr_len){
								LOG_HTTP_TRANS_WIPI_RESP_HDR_LEX((char*)pdata_resp_hdr,data_resp_hdr_len,(char *)pLOG_HTTP_TRANS);
						
								/* for AppFailCode */
								if( (pLOG_HTTP_TRANS->usUserErrorCode == 0) && 
									(atoi((char*)pLOG_HTTP_TRANS->szAppFailCode) != 0)) {
									pLOG_HTTP_TRANS->usUserErrorCode = HTTP_UERR_980;
									pLOG_HTTP_TRANS->usL7FailCode = HTTP_UERR_980;
								}
								
							}
						} else {
							log_print(LOGN_CRI, "ERROR %s : LOG_HTTP_TRANS가 존재하지 않는다!!!",PrintTYPE(type));
						}
						break;
					case HTTP_RES_BODY_NUM:
						pdata_resp_body = (char *) data;
						data_resp_body_len = len;
						log_print(LOGN_INFO, "TYPE[%s] len %d DATA[%.*s]", PrintTYPE(type), len ,len, data);				
						if(pLOG_HTTP_TRANS){
							if(pdata_resp_body && data_resp_body_len){
								U8		*pbody_data;
								st_LIST	UrlParseList;

								if((pBodyNode = nifo_node_alloc(pMEMSINFO)) == NULL) {
									log_print(LOGN_CRI, "[%s][%s.%d] NODE IS NULL", __FILE__, __FUNCTION__, __LINE__);
									break;
								}
								if((pbody_data = nifo_tlv_alloc(pMEMSINFO, pBodyNode, BODY_DEF_NUM, BODY_SIZE, DEF_MEMSET_OFF)) == NULL) {
									log_print(LOGN_CRI, "[%s][%s.%d] TLV IS NULL", __FILE__, __FUNCTION__, __LINE__);
									break;
								}

								// URLParseList Set : shlee
								dRet = Devide_ReqURL(URLbuf_len, URLbuf, &UrlParseList);
								if(dRet < 0) {  /* Error */
									log_print(LOGN_CRI, "Devide_ReqURL Ret =[%d] < 0 [%d] [%.*s]", dRet, URLbuf_len, MAX_URL_SIZE, URLbuf);
									break;
								} else if(dRet > 0) { /* Warning */
									log_print(LOGN_CRI, "Devide_ReqURL Ret =[%d] > 0 [%d] [%.*s]", dRet, URLbuf_len, MAX_URL_SIZE, URLbuf);
								}

								pBODY = (BODY *)pbody_data;
								BODY_LEX(pdata_resp_body,data_resp_body_len, pbody_data, &UrlParseList, pMEMSINFO);
								pLOG_HTTP_TRANS->link_cnt = pBODY->link_cnt;
								pLOG_HTTP_TRANS->href_cnt = pBODY->href_cnt;
								log_print(LOGN_INFO, "HREF_CNT %d", pLOG_HTTP_TRANS->href_cnt);

/*		This code is available to run in VOD.
								U8 	sss[BUFSIZ];
								if(pBODY->redirect_url_len){
									URL_ANALYSIS *pURL_ANALYSIS;
									pURL_ANALYSIS = (URL_ANALYSIS *)sss;
									memset(sss,0,BUFSIZ);
									URL_ANALYSIS_URL_S_LEX(pBODY->redirect_url,pBODY->redirect_url_len,sss);
									if(pURL_ANALYSIS->ContentID[0]){
										memcpy(pLOG_HTTP_TRANS->ContentID , pURL_ANALYSIS->ContentID , MAX_CONTENTID_SIZE);
									}
								}
*/
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
//				LOG_HTTP_TRANS_Prt("PRINT FINISH LOG_HTTP_TRANS", pLOG_HTTP_TRANS);
				pNextNode = (U8 *)nifo_entry(nifo_ptr(pMEMSINFO, ((NIFO *)pNode)->nont.offset_next), NIFO, nont);
				nifo_node_unlink_nont(pMEMSINFO, pNode);
				nifo_node_delete(pMEMSINFO, pNextNode);
				if(pBodyNode){
					nifo_node_link_nont_prev(pMEMSINFO, pNode, pBodyNode);
				}
				dSeqProcID = SEQ_PROC_A_CALL + ( pLOG_HTTP_TRANS->uiClientIP % gACALLCnt );
				if((dRet = dSend_FV_Data(pMEMSINFO, dSeqProcID, pNode)) < 0) {
					log_print(LOGN_CRI, "[%s][%s.%d] MSGQ WRITE FAILE[%d][%s]", __FILE__, __FUNCTION__, __LINE__, dRet, strerror(-dRet));
					break;
				}
			} else {
				log_print(LOGN_CRI,"FV : pLOG_HTTP_TRANS is NULL");
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
 *  $Log: fv_main.c,v $
 *  Revision 1.2  2011/09/04 12:16:50  hhbaek
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *  NEW OAM SYSTEM
 *
 *  Revision 1.4  2011/08/21 09:07:50  hhbaek
 *  Commit TAF/SRC/ *
 *
 *  Revision 1.3  2011/08/17 07:17:33  dcham
 *  *** empty log message ***
 *
 *  Revision 1.2  2011/08/08 11:05:41  uamyd
 *  modified block added
 *
 *  Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 *  init DQMS2
 *
 *  Revision 1.8  2011/05/09 15:17:08  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.7  2011/01/11 04:09:06  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2010/08/23 01:13:02  uamyd
 *  DQMS With TOTMON, 2nd-import
 *
 *  Revision 1.6  2009/08/19 12:24:11  pkg
 *  LOG_XXX_Prt 함수 주석 처리
 *
 *  Revision 1.5  2009/08/06 06:56:09  dqms
 *  로그레벨 공유메모리로 수정
 *
 *  Revision 1.4  2009/07/15 17:10:56  dqms
 *  set_version 위치 및 Plastform Type 변경
 *
 *  Revision 1.3  2009/06/16 18:08:15  dqms
 *  *** empty log message ***
 *
 *  Revision 1.2  2009/06/12 11:09:36  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2009/05/26 02:14:36  dqms
 *  Init TAF_RPPI
 *
 *  Revision 1.4  2008/07/02 07:28:17  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.3  2008/06/18 11:11:56  dark264sh
 *  A_FV Error-Code값이 발생하는 경우 UserErrorCode, L7FailCode 세팅
 *
 *  Revision 1.2  2008/06/18 09:01:01  dark264sh
 *  A_FV L7Code 파싱 추가
 *
 *  Revision 1.1  2008/06/17 12:17:04  dark264sh
 *  init
 *
 */



