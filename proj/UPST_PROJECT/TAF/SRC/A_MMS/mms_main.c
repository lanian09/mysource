/**		@file	mms_main.c
 * 		- MMS Transaction을 관리 하는 프로세스
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: mms_main.c,v 1.2 2011/09/05 05:28:46 uamyd Exp $
 *
 * 		@Author		$Author: uamyd $
 * 		@version	$Revision: 1.2 $
 * 		@date		$Date: 2011/09/05 05:28:46 $
 * 		@warning	.
 * 		@ref		mms_main.c mms_init.c
 * 		@todo		library가 만들어지지 않은 상태, library 생성 이후 함수 대치
 *
 * 		@section	Intro(소개)
 * 		- MMS Transaction을 관리 하는 프로세스
 *
 * 		@section	Requirement
 * 		 @li library 생성 이후 함수 대치
 *
 **/

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "loglib.h"
#include "verlib.h"
#include "mems.h"
#include "cifo.h"
#include "gifo.h"
#include "nifo.h"

#include "path.h"
#include "common_stg.h"
#include "commdef.h"
#include "procid.h"
#include "sshmid.h"

//#include <debug.h>
#include "http.h"

#include "mms_init.h"
#include "mms_msgq.h"



S32				giFinishSignal;		/**< Finish Signal */
S32				giStopFlag;			/**< main loop Flag 0: Stop, 1: Loop */

stMEMSINFO		*pMEMSINFO;			/**< new interface 관리 구조체 */
stCIFO			*gpCIFO;

extern int		gACALLCnt;

U8 *PrintTYPE(S32 type)
{
	switch(type)
	{
	case LOG_HTTP_TRANS_DEF_NUM: 	return (U8*)"LOG_HTTP_TRANS";
	case HTTP_REQ_HDR_NUM:			return (U8*)"HTTP REQ HDR";
	case HTTP_REQ_BODY_NUM:			return (U8*)"HTTP REQ BODY";
	case HTTP_RES_HDR_NUM:			return (U8*)"HTTP RES HDR";
	case HTTP_RES_BODY_NUM:			return (U8*)"HTTP RES BODY";
	default:						return (U8*)"UNKNOWN";
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

	char    vERSION[7] = "R3.0.0";

	/* log_print 초기화 */
	log_init(S_SSHM_LOG_LEVEL, getpid(), SEQ_PROC_A_MMS, LOG_PATH"/A_MMS", "A_MMS");

	/* A_MMS 초기화 */
	if((dRet = dInitMMS(&pMEMSINFO)) < 0)
	{
		log_print(LOGN_CRI, "[%s][%s.%d] dInitMMS dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		exit(0);
	}

	if((dRet = set_version(S_SSHM_VERSION, SEQ_PROC_A_MMS, vERSION)) < 0 ) {
		log_print(LOGN_CRI, "SET_VERSION ERROR(RET=%d,IDX=%d,VER=%s)", dRet, SEQ_PROC_A_MMS, vERSION);
	}

	log_print(LOGN_CRI, "START MMS(%s)", vERSION);

	/* MAIN LOOP */
	while(giStopFlag)
	{
		if((offset = gifo_read(pMEMSINFO, gpCIFO, SEQ_PROC_A_MMS)) > 0) {

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
							if(pLOG_HTTP_TRANS->szMIN[0] == 0x00) {
								mms_from(pdata_req_body, data_req_body_len, (char *)pLOG_HTTP_TRANS->szMIN);
							}
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
								LOG_HTTP_TRANS_WIPI_RESP_HDR_LEX(pdata_resp_hdr,data_resp_hdr_len,(char *)pLOG_HTTP_TRANS);
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
								BODY_LEX(pdata_resp_body, data_resp_body_len, pbody_data, &UrlParseList, pMEMSINFO);
								pLOG_HTTP_TRANS->link_cnt = pBODY->link_cnt;
								pLOG_HTTP_TRANS->href_cnt = pBODY->href_cnt;
								log_print(LOGN_INFO, "HREF_CNT %d", pLOG_HTTP_TRANS->href_cnt);

								if(pLOG_HTTP_TRANS->szMIN[0] == 0x00) {
									mms_to(pdata_resp_body, data_resp_body_len, (char *)pLOG_HTTP_TRANS->szMIN);
								}

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

			if(pLOG_HTTP_TRANS) {
				/* download 처리 */
				int	dn = 0;
				switch(pLOG_HTTP_TRANS->usSvcL4Type)
				{
				case L4_MMS_UP:
					mms_from_dn((char *)pLOG_HTTP_TRANS->szLOGURL, pLOG_HTTP_TRANS->usLOGURLSize, &dn);
					if(dn != 1) {
						pLOG_HTTP_TRANS->usSvcL4Type = L4_MMS_UP_NODN;
					}
					break;
				case L4_MMS_DN:
					mms_to_dn((char *)pLOG_HTTP_TRANS->szLOGURL, pLOG_HTTP_TRANS->usLOGURLSize, &dn);
					if(dn != 1) {
						pLOG_HTTP_TRANS->usSvcL4Type = L4_MMS_DN_NODN;
					}
					break;
				case L4_MMS_NEW:
				default:
					break;	
				}

//				LOG_HTTP_TRANS_Prt("PRINT LOG_HTTP_TRANS2", pLOG_HTTP_TRANS);
				/* DATA NODE DELETE */
				pNextNode = (U8 *)nifo_entry(nifo_ptr(pMEMSINFO, ((NIFO *)pNode)->nont.offset_next), NIFO, nont);
				nifo_node_unlink_nont(pMEMSINFO, pNode);
				nifo_node_delete(pMEMSINFO, pNextNode);
				if(pBodyNode){
					nifo_node_link_nont_prev(pMEMSINFO, pNode, pBodyNode);
				}
				dSeqProcID = SEQ_PROC_A_CALL + ( pLOG_HTTP_TRANS->uiClientIP % gACALLCnt );
				if((dRet = dSend_MMS_Data(pMEMSINFO, dSeqProcID, pNode)) < 0) {
					log_print(LOGN_CRI, "[%s][%s.%d] MSGQ WRITE FAILE[%d][%s]", __FILE__, __FUNCTION__, __LINE__, dRet, strerror(-dRet));
					break;
				}
			} else {
				log_print(LOGN_CRI,"MMS : pLOG_HTTP_TRANS is NULL");
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
 *  $Log: mms_main.c,v $
 *  Revision 1.2  2011/09/05 05:28:46  uamyd
 *  A_MMS modified
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *  NEW OAM SYSTEM
 *
 *  Revision 1.4  2011/08/21 09:07:51  hhbaek
 *  Commit TAF/SRC
 *
 *  Revision 1.3  2011/08/17 07:23:55  dcham
 *  *** empty log message ***
 *
 *  Revision 1.2  2011/08/08 11:05:43  uamyd
 *  modified block added
 *
 *  Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 *  init DQMS2
 *
 *  Revision 1.7  2011/05/09 15:21:45  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.6  2011/01/11 04:09:08  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2010/08/23 01:12:58  uamyd
 *  DQMS With TOTMON, 2nd-import
 *
 *  Revision 1.5  2009/08/19 12:26:46  pkg
 *  LOG_XXX_Prt 함수 주석 처리
 *
 *  Revision 1.4  2009/08/06 06:56:09  dqms
 *  로그레벨 공유메모리로 수정
 *
 *  Revision 1.3  2009/07/02 11:08:47  dqms
 *  ADD set_version()
 *
 *  Revision 1.2  2009/06/12 11:09:36  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2009/05/26 02:14:40  dqms
 *  Init TAF_RPPI
 *
 *  Revision 1.3  2008/12/17 06:26:18  dark264sh
 *  LOG_MMS L7Code L4_MMS_NEW 추가
 *
 *  Revision 1.2  2008/07/02 07:25:22  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2008/06/09 08:17:17  jsyoon
 *  WATAS3 PROJECT START
 *
 *  Revision 1.2  2007/10/08 04:41:31  dark264sh
 *  no message
 *
 *  Revision 1.1  2007/09/03 08:32:26  dark264sh
 *  *** empty log message ***
 *
 */



