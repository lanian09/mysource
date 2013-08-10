/**		@file	fb_main.c
 * 		- FB Transaction을 관리 하는 프로세스
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: fb_main.c,v 1.2 2011/09/04 09:56:04 dhkim Exp $
 *
 * 		@Author		$Author: dhkim $
 * 		@version	$Revision: 1.2 $
 * 		@date		$Date: 2011/09/04 09:56:04 $
 * 		@warning	.
 * 		@ref		fb_main.c fb_init.c
 * 		@todo		library가 만들어지지 않은 상태, library 생성 이후 함수 대치
 *
 * 		@section	Intro(소개)
 * 		- FB Transaction을 관리 하는 프로세스
 *
 * 		@section	Requirement
 * 		 @li library 생성 이후 함수 대치
 *
 **/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <zlib.h>

// LIB
#include "typedef.h"
#include "commdef.h"
#include "mems.h"
#include "cifo.h"
#include "gifo.h"
#include "nifo.h"
#include "loglib.h"
#include "verlib.h"

// PROJECT
#include "path.h"
#include "procid.h"
#include "sshmid.h"
#include "common_stg.h"

//#include <debug.h>
// TAF
#include "fb.h"


#include "fb_init.h"
#include "fb_uncomp.h"
#include "fb_chunked.h"
#include "fb_msgq.h"
#include "fb_multipart.h"

#define MAX_MULTIBUF_SIZE		(1024 * 1024 * 10)
#define MAX_CHUNKEDBUF_SIZE		(1024 * 1024 * 10)

S32				giFinishSignal;		/**< Finish Signal */
S32				giStopFlag;			/**< main loop Flag 0: Stop, 1: Loop */

stMEMSINFO		*pMEMSINFO;			/**< new interface 관리 구조체 */
stCIFO			*gpCIFO;

S32				dCallQID[MAX_SMP_NUM];
extern int		gACALLCnt;

S32				dUncompLen;
U8				szUncomp[MAX_UNCOMP_SIZE];

S32				dMultiBufLen;
U8				szMultiBuf[MAX_MULTIBUF_SIZE];
U8				szMultiTmpBuf[MAX_MULTIBUF_SIZE];

S32				dChunkedBufLen;
U8				szChunkedBuf[MAX_CHUNKEDBUF_SIZE];

U8 *PrintTYPE(S32 type)
{
	switch(type)
	{
		case LOG_HTTP_TRANS_DEF_NUM: 	return (U8*)"LOG_HTTP_TRANS";
		case HTTP_REQ_HDR_NUM: 			return (U8*)"HTTP REQ HDR";
		case HTTP_REQ_BODY_NUM: 		return (U8*)"HTTP REQ BODY";
		case HTTP_RES_HDR_NUM: 			return (U8*)"HTTP RES HDR";
		case HTTP_RES_BODY_NUM: 		return (U8*)"HTTP RES BODY";
		default: 						return (U8*)"UNKNOWN";
	}
}

U8 *PrintUncomp(S32 type)
{
	switch(type)
	{
		case Z_OK:				return (U8*)"success";
		case Z_MEM_ERROR:		return (U8*)"not enough memory";
		case Z_BUF_ERROR:		return (U8*)"not enough root in the output buffer";
		case Z_DATA_ERROR:		return (U8*)"the input data was corrupted";
		default:				return (U8*)"unknown";
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
	S32				dRet, zip, i;		/**< 함수 Return 값 */
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
	log_init(S_SSHM_LOG_LEVEL, getpid(), SEQ_PROC_A_FB, LOG_PATH"/A_FB", "A_FB");

	/* A_FB 초기화 */
	if((dRet = dInitFB(&pMEMSINFO)) < 0)
	{
		log_print(LOGN_CRI, "[%s][%s.%d] dInitFB dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		exit(0);
	}

    if((dRet = set_version(S_SSHM_VERSION, SEQ_PROC_A_FB, vERSION)) < 0 ) {
        log_print(LOGN_CRI, "SET_VERSION ERROR(RET=%d,IDX=%d,VER=%s)", dRet, SEQ_PROC_A_FB, vERSION);
    }
	log_print(LOGN_CRI, "START FB");

	/* MAIN LOOP */
	while(giStopFlag)
	{
		if((offset = gifo_read(pMEMSINFO, gpCIFO, SEQ_PROC_A_FB)) > 0) {

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
								fb_min(pdata_req_hdr, data_req_hdr_len, (char*)pLOG_HTTP_TRANS->szMIN);
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
								LOG_HTTP_TRANS_WIPI_RESP_HDR_LEX(pdata_resp_hdr,data_resp_hdr_len,(char *)pLOG_HTTP_TRANS);
							}
						} else {
							log_print(LOGN_CRI, "ERROR %s : LOG_HTTP_TRANS가 존재하지 않는다!!!",PrintTYPE(type));
						}
						break;
					case HTTP_RES_BODY_NUM:
						pdata_resp_body = (char *) data;
						data_resp_body_len = len;
						log_print(LOGN_INFO, "TYPE[%s] len %d", PrintTYPE(type), len);				
						if(pLOG_HTTP_TRANS){

							/* for chunked */
							if(pdata_resp_body && data_resp_body_len && pLOG_HTTP_TRANS->dChunked > 0) {
								log_print(LOGN_INFO, "#== CHUNKED DATA LEN=%d", data_resp_body_len);
								dChunkedBufLen = MAX_CHUNKEDBUF_SIZE;
								if((dRet = dGetChunked(szChunkedBuf, &dChunkedBufLen, (U8*)pdata_resp_body, data_resp_body_len)) < 0) {
									log_print(LOGN_CRI, "F=%s:%s.%d chunked dRet=%d", __FILE__, __FUNCTION__, __LINE__, dRet);
									pdata_resp_body = NULL;
									data_resp_body_len = 0;
								}
								else {
									pdata_resp_body = (char*)szChunkedBuf;
									data_resp_body_len = dChunkedBufLen;
									if(pLOG_HTTP_TRANS->dResZip == 0) {
										for(i = 0; i < data_resp_body_len; i++) {
											TOLOWER(pdata_resp_body[i], pdata_resp_body[i]);
										}
									} 
									log_print(LOGN_INFO, "#== AFTER CHUNKED TYPE[%s] len %d", PrintTYPE(type), data_resp_body_len);
								}	
							}

							/* for multi-part */
							if(pdata_resp_body && data_resp_body_len && pLOG_HTTP_TRANS->dResMultiLen > 0) {
								log_print(LOGN_INFO, "#== MULTI-PART DATA LEN=%d", data_resp_body_len);

								int multi_offset = 0;
								int multi_len = MAX_MULTIBUF_SIZE;
								int input_offset = 0;
								int input_len = data_resp_body_len;

								dMultiBufLen = 0;

								while(multi_offset < data_resp_body_len)
								{
									zip = 0;
									multi_len = MAX_MULTIBUF_SIZE - multi_offset;
									input_len = data_resp_body_len - input_offset;

									if((dRet = dGetMultiPart(szMultiTmpBuf, &multi_len, &zip, (U8*)&pdata_resp_body[input_offset], input_len, pLOG_HTTP_TRANS->szResMulti, pLOG_HTTP_TRANS->dResMultiLen)) < 0) {
										log_print(LOGN_CRI, "F=%s:%s.%d multi-part dRet=%d", __FILE__, __FUNCTION__, __LINE__, dRet);
										dMultiBufLen = 0;
										break;
									}
									else if(dRet == 0) {
										break;
									}
									else {
										input_offset += dRet;
										multi_offset += multi_len;

										if(zip > 0) {
											/* for uncompress */
											log_print(LOGN_INFO, "#S= COMPRESS DATA LEN=%d", multi_len);
											dUncompLen = MAX_UNCOMP_SIZE;
											if((dRet = dUncompress(szUncomp, &dUncompLen, szMultiTmpBuf, multi_len)) != Z_OK) {
												log_print(LOGN_CRI, "F=%s:%s.%d uncompress dRet=%d:%s",
													__FILE__, __FUNCTION__, __LINE__, dRet, PrintUncomp(dRet));
												dMultiBufLen = 0;
												break;
											} else {
												if(dMultiBufLen + dUncompLen >= MAX_MULTIBUF_SIZE) {
													log_print(LOGN_CRI, "F=%s:%s.%d multi-part overflow max=%d dMultiBufLen=%d dUncompLen=%d", __FILE__, __FUNCTION__, __LINE__, MAX_MULTIBUF_SIZE, dMultiBufLen, dUncompLen);
													dMultiBufLen = 0;
													break;
//													dUncompLen = (dMultiBufLen + dUncompLen) - MAX_MULTIBUF_SIZE - 1;
												}

												for(i = 0; i < dUncompLen; i++) {
													TOLOWER(szMultiBuf[dMultiBufLen + i], szUncomp[i]);
												}
												dMultiBufLen += dUncompLen;
												log_print(LOGN_INFO, "#S= AFTER COMPRESS TYPE[%s] len %d", PrintTYPE(type), dUncompLen);
											}

										} else {
											if(dMultiBufLen + multi_len >= MAX_MULTIBUF_SIZE) {
												log_print(LOGN_CRI, "F=%s:%s.%d multi-part overflow max=%d dMultiBufLen=%d multi_len=%d", __FILE__, __FUNCTION__, __LINE__, MAX_MULTIBUF_SIZE, dMultiBufLen, multi_len);
												dMultiBufLen = 0;
												break;
//												multi_len = (dMultiBufLen + multi_len) - MAX_MULTIBUF_SIZE - 1;
											}

											for(i = 0; i < multi_len; i++) {
												TOLOWER(szMultiBuf[dMultiBufLen + i], szMultiTmpBuf[i]);
											}
											dMultiBufLen += multi_len;
											log_print(LOGN_INFO, "#S= AFTER MULTI-PART TYPE[%s] len %d", PrintTYPE(type), multi_len);
										}
									}

								}

								if(dMultiBufLen == 0) {
									pdata_resp_body = NULL;
									data_resp_body_len = 0;
								} else {
									pdata_resp_body = (char*)szMultiBuf;
									data_resp_body_len = dMultiBufLen;
									log_print(LOGN_INFO, "#== AFTER MULTI-PART TYPE[%s] len %d", PrintTYPE(type), data_resp_body_len);
								}
							}

							/* for uncompress */
							if(pdata_resp_body && data_resp_body_len && pLOG_HTTP_TRANS->dResZip > 0) {
								log_print(LOGN_INFO, "#== COMPRESS DATA LEN=%d", data_resp_body_len);
								dUncompLen = MAX_UNCOMP_SIZE;
								if((dRet = dUncompress(szUncomp, &dUncompLen, (U8*)pdata_resp_body, data_resp_body_len)) != Z_OK) {
									log_print(LOGN_CRI, "F=%s:%s.%d uncompress dRet=%d:%s",
										__FILE__, __FUNCTION__, __LINE__, dRet, PrintUncomp(dRet));
									pdata_resp_body = NULL;
									data_resp_body_len = 0;
								} else {
									pdata_resp_body = (char*)szUncomp;
									data_resp_body_len = dUncompLen;
									for(i = 0; i < data_resp_body_len; i++) {
										TOLOWER(pdata_resp_body[i], pdata_resp_body[i]);
									}
									log_print(LOGN_INFO, "#== AFTER COMPRESS TYPE[%s] len %d", PrintTYPE(type), data_resp_body_len);
								}
							}

							log_print(LOGN_INFO, "AFTER TYPE[%s] len %d DATA[%.*s]", 
										PrintTYPE(type), data_resp_body_len, data_resp_body_len, pdata_resp_body);				

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
								
								OFFSET	print_offset;
								U8		*pROOTNODE, *pDATANODE, *pNEXTNODE;
								BODY	*pPRINTBODY;

								pROOTNODE = (U8 *)nifo_ptr(pMEMSINFO, nifo_get_offset_node(pMEMSINFO, (U8 *)pBODY));
								pDATANODE = pROOTNODE;
								pNEXTNODE = (U8 *)nifo_entry(nifo_ptr(pMEMSINFO, ((NIFO *)pDATANODE)->cont.offset_next), NIFO, cont);

								if(pDATANODE == pNEXTNODE) {
									pLOG_HTTP_TRANS->link_cnt = pBODY->link_cnt;
									pLOG_HTTP_TRANS->href_cnt = pBODY->href_cnt;
								}
								else {
									while(pDATANODE != NULL) {
										print_offset = nifo_offset(pMEMSINFO, pDATANODE);

										pPRINTBODY = (BODY *)nifo_get_value(pMEMSINFO, BODY_DEF_NUM, print_offset);
										pLOG_HTTP_TRANS->link_cnt += pPRINTBODY->link_cnt;
										pLOG_HTTP_TRANS->href_cnt += pPRINTBODY->href_cnt;

										pNEXTNODE = (U8 *)nifo_entry(nifo_ptr(pMEMSINFO, ((NIFO *)pDATANODE)->cont.offset_next), NIFO, cont);
										if(pROOTNODE == pNEXTNODE) pNEXTNODE = NULL;

										pDATANODE = pNEXTNODE;
}
								}
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
				pNextNode = (U8 *)nifo_entry(nifo_ptr(pMEMSINFO, ((NIFO *)pNode)->nont.offset_next), NIFO, nont);
				nifo_node_unlink_nont(pMEMSINFO, pNode);
				nifo_node_delete(pMEMSINFO, pNextNode);
				if(pBodyNode){
					nifo_node_link_nont_prev(pMEMSINFO, pNode, pBodyNode);
				}
				dSeqProcID = SEQ_PROC_A_CALL + ( pLOG_HTTP_TRANS->uiClientIP % gACALLCnt );
				if((dRet = dSend_FB_Data(pMEMSINFO, dSeqProcID, pNode)) < 0) {
					log_print(LOGN_CRI, "[%s][%s.%d] MSGQ WRITE FAILE[%d][%s]", __FILE__, __FUNCTION__, __LINE__, dRet, strerror(-dRet));
					break;
				}
			} else {
				log_print(LOGN_CRI,"FB : pLOG_HTTP_TRANS is NULL");
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
 *  $Log: fb_main.c,v $
 *  Revision 1.2  2011/09/04 09:56:04  dhkim
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *  NEW OAM SYSTEM
 *
 *  Revision 1.4  2011/08/21 09:07:50  hhbaek
 *  Commit TAF/SRC/
 *
 *  Revision 1.3  2011/08/17 07:16:34  dcham
 *  *** empty log message ***
 *
 *  Revision 1.2  2011/08/08 11:05:41  uamyd
 *  modified block added
 *
 *  Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 *  init DQMS2
 *
 *  Revision 1.11  2011/05/09 15:16:14  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.10  2011/01/11 04:09:06  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2010/08/23 01:12:59  uamyd
 *  DQMS With TOTMON, 2nd-import
 *
 *  Revision 1.9  2009/11/02 15:01:55  pkg
 *  *** empty log message ***
 *
 *  Revision 1.8  2009/11/02 14:47:23  pkg
 *  A_FB MultiPart Zip overflow 버그 수정
 *
 *  Revision 1.7  2009/10/26 06:29:53  pkg
 *  A_FB log_print argument 버그 수정
 *
 *  Revision 1.6  2009/08/19 12:23:08  pkg
 *  LOG_XXX_Prt 함수 주석 처리
 *
 *  Revision 1.5  2009/08/06 06:56:09  dqms
 *  로그레벨 공유메모리로 수정
 *
 *  Revision 1.4  2009/07/15 17:10:56  dqms
 *  set_version 위치 및 Plastform Type 변경
 *
 *  Revision 1.3  2009/06/28 12:57:45  dqms
 *  ADD set_version
 *
 *  Revision 1.2  2009/06/12 11:09:36  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2009/05/26 02:14:14  dqms
 *  Init TAF_RPPI
 *
 *  Revision 1.3  2008/07/02 07:38:18  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.2  2008/06/22 10:18:02  dark264sh
 *  A_FB chunked, multipart, gzip, deflate, min 처리
 *
 *  Revision 1.1  2008/06/18 12:26:32  dark264sh
 *  A_FB 추가
 *
 */



