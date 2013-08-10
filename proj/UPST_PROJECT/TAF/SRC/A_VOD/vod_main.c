/*******************************************************************************
 *		@file	vod_main.c
 * 		- HTTP Transaction을 관리 하는 프로세스
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: vod_main.c,v 1.2 2011/09/06 12:46:39 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.2 $
 * 		@date		$Date: 2011/09/06 12:46:39 $
 * 		@warning	.
 * 		@ref		http_main.c l4.h http_init.c http_func.c
 * 		@todo		library가 만들어지지 않은 상태, library 생성 이후 함수 대치
 *
 * 		@section	Intro(소개)
 * 		- HTTP Transaction을 관리 하는 프로세스
 *
 * 		@section	Requirement
 * 		 @li library 생성 이후 함수 대치
 *
*******************************************************************************/

/**
 *	Include headers
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>

// TOP
#include "common_stg.h"
#include "path.h"
#include "commdef.h"
#include "procid.h"
#include "capdef.h"
#include "sshmid.h"

// LIB
#include "mems.h"
#include "gifo.h"
#include "nifo.h"
#include "cifo.h"
#include "Analyze_Ext_Abs.h"
#include "loglib.h"
#include "verlib.h"
#include "commdef.h"

// TAF
#include "http.h"
#include "debug.h"

// .
#include "vod_init.h"
#include "vod_msgq.h"
#include "vod_sess.h"

/**
 *	Declare var.
 */
S32				giFinishSignal;		/**< Finish Signal */
S32				giStopFlag;			/**< main loop Flag 0: Stop, 1: Loop */

stMEMSINFO		*pMEMSINFO;			/**< new interface 관리 구조체 */
stCIFO			*gpCIFO;
stHASHGINFO		*pLVODHASH;
stHASHGINFO		*pMENUHASHGINFO;
stTIMERNINFO 	*pTIMERNINFO;		/**< timerN 관리 구조체 */

stHASHOINFO		*pstVODSESSHASH;	/* 2007.08.23 LDH : VOD STREAMING SESSION */
stHASHOINFO		*pstRTCPSESSHASH;

S32				dCallQID[MAX_SMP_NUM];
extern int		gACALLCnt;

/**
 *	Declare func.
 */
int 		dStart_Session( TCP_INFO *pstTCPINFO );
int 		dEnd_Session( TCP_INFO *pstTCPINFO );
stHASHONODE *pCheck_Session( LOG_HTTP_TRANS *pstHTTPLOG );
int 		dProc_VODSESS( LOG_HTTP_TRANS *pstHTTPLOG, stHASHONODE *pstVODSess,
				   		   char *pReqHdr, char *pRespHdr, char *pRespBody, int dReqHdrLen, int dRespHdrLen, int dRespBodyLen );
int 		dRTCP_Session( OFFSET dOffset, INFO_ETH *pstINFOETH, Capture_Header_Msg *pstCAPHEAD );

/**
 *	Implement func.
 */

/*******************************************************************************

*******************************************************************************/
U8 *PrintTYPE(S32 type)
{
	switch(type)
	{
	case TCP_INFO_DEF_NUM: 			return (U8*)"TCP_INFO_DEF_NUM";
	case CAP_HEADER_NUM: 			return (U8*)"CAP_HEADER_NUM";
	case ETH_DATA_NUM: 				return (U8*)"ETH_DATA_NUM";
	case INFO_ETH_NUM: 				return (U8*)"INFO_ETH_NUM";
	case TCP_DATA_NUM: 				return (U8*)"TCP_DATA_NUM"; 
	case LOG_HTTP_TRANS_DEF_NUM: 	return (U8*)"LOG_HTTP_TRANS";
	case HTTP_REQ_HDR_NUM: 			return (U8*)"HTTP REQ HDR";
	case HTTP_REQ_BODY_NUM: 		return (U8*)"HTTP REQ BODY";
	case HTTP_RES_HDR_NUM:			return (U8*)"HTTP RES HDR";
	case HTTP_RES_BODY_NUM:			return (U8*)"HTTP RES BODY";
	default:						return (U8*)"UNKNOWN";
	}
}



/*******************************************************************************
 * 	main function.
 *  @param	argc	:	파라미터 개수
 *  @param	*argv[]	:	파라미터
 *  @return			S32
 *  @see			http_main.c l4.h http_init.c http_func.c
 *  @exception		.
 *  @note			.
*******************************************************************************/
S32 main(S32 argc, S8 *argv[])
{
	S32				dRet;		/**< 함수 Return 값 */
	OFFSET			offset;
	U8				*pNode;
	U8				*pNextNode;
	U8				*p, *data;
	S32				type, len, ismalloc;
	U8				*pBodyNode;
	U16             URLbuf_len;
	U8              URLbuf[BUFSIZ];
	BODY    		*pBODY;
	U8              sss[BUFSIZ];
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
	stHASHGNODE		*pstHASHNODE;
	TAG_KEY_LVOD_CONF	TAG_KEY_LVOD_CONF;
	LVOD_CONF		*pLVOD_CONF;


	UCHAR				ucControlFlag = 0;

	Capture_Header_Msg	*pCAPHEAD;
	INFO_ETH            *pINFOETH;
	stHASHONODE			*pstVODSESSNode;
	TCP_INFO			*pstTCPINFO;

    char    			vERSION[7] = "R3.0.0";


	/* log_print 초기화 */
	log_init( S_SSHM_LOG_LEVEL, getpid(), SEQ_PROC_A_VOD, LOG_PATH"/A_VOD", "A_VOD" );

	/* A_VOD 초기화 */
	if((dRet = dInitVOD( &pMEMSINFO, &pLVODHASH, &pMENUHASHGINFO, &pTIMERNINFO, &pstVODSESSHASH, &pstRTCPSESSHASH)) < 0)
	{
		log_print(LOGN_CRI, LH"dInitVOD dRet[%d]", LT, dRet);
		exit(0);
	}

    if((dRet = set_version(S_SSHM_VERSION, SEQ_PROC_A_VOD, vERSION)) < 0 ) {
        log_print(LOGN_CRI, "SET_VERSION ERROR(RET=%d,IDX=%d,VER=%s)", dRet, SEQ_PROC_A_VOD, vERSION);
    }
	log_print(LOGN_CRI, "START VOD(%s)", vERSION);

	/* MAIN LOOP */
	while(giStopFlag)
	{
		timerN_invoke(pTIMERNINFO);

		if((offset = gifo_read(pMEMSINFO, gpCIFO, SEQ_PROC_A_VOD)) > 0) {

			log_print(LOGN_INFO, "======================================================================");

			/* DB LOG 전송을 목적으로 하는 NODE (삭제 하지 않고 전송하기 위함 )*/
			pNode = nifo_ptr(pMEMSINFO, offset);
			pNextNode = pNode;

			pdata_req_hdr 	= NULL;
			pdata_req_body 	= NULL;
			pdata_resp_hdr 	= NULL;
			pdata_resp_body = NULL;
			pLOG_HTTP_TRANS = NULL;
			pBodyNode 		= NULL;
			URLbuf_len 		= 0;
			URLbuf[0] 		= 0x00;
			pBODY 			= NULL;

			pCAPHEAD		= NULL;
			pINFOETH		= NULL;

			do {
				p = pNextNode;

				while(p != NULL) {
					if((dRet = nifo_read_tlv_cont(pMEMSINFO, pNextNode, (U32*)&type, (U32*)&len, &data, &ismalloc, &p)) < 0)
						break;

					log_print(LOGN_INFO, "NEW_MSG_START : TYPE[%d][%s] LEN[%d] ISMALLOC[%s]", 
									  type, PrintTYPE(type), len, 
									  (ismalloc == DEF_READ_MALLOC) ? "MALLOC MEM" : "ORIGIN MEM");

					switch(type)
					{
					case LOG_HTTP_TRANS_DEF_NUM:
						pLOG_HTTP_TRANS = (LOG_HTTP_TRANS *) data;
						//LOG_HTTP_TRANS_Prt("PRINT LOG_HTTP_TRANS1", pLOG_HTTP_TRANS);
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
						log_print(LOGN_INFO, "REQHDR TYPE[%s] len %d DATA[%.*s]", PrintTYPE(type), len ,len, data);				
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
						log_print(LOGN_INFO, "REQBODY TYPE[%s] len %d DATA[%.*s]", PrintTYPE(type), len ,len, data);				
						if(pLOG_HTTP_TRANS){
						} else {
							log_print(LOGN_CRI, "ERROR %s : LOG_HTTP_TRANS가 존재하지 않는다!!!",PrintTYPE(type));
						}
						break;
					case HTTP_RES_HDR_NUM:
						pdata_resp_hdr = (char *) data;
						data_resp_hdr_len = len;
						log_print(LOGN_INFO, "RESHDR TYPE[%s] len %d DATA[%.*s]", PrintTYPE(type), len ,len, data);				
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
						log_print(LOGN_INFO, "RESBODY TYPE[%s] len %d DATA[%.*s]", PrintTYPE(type), len ,len, data);				
						if(pLOG_HTTP_TRANS){
							if(pdata_resp_body && data_resp_body_len){
								U8		*pbody_data;
								st_LIST    UrlParseList;


								if((pBodyNode = nifo_node_alloc(pMEMSINFO)) == NULL) {
									log_print(LOGN_CRI, LH"NODE IS NULL", LT);
									break;
								}
								if((pbody_data = nifo_tlv_alloc(pMEMSINFO, pBodyNode, BODY_DEF_NUM, BODY_SIZE, DEF_MEMSET_OFF)) == NULL) {
									log_print(LOGN_CRI, LH"TLV IS NULL", LT);
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
								BODY_LEX(pdata_resp_body,data_resp_body_len,pbody_data,&UrlParseList,pMEMSINFO);
								pLOG_HTTP_TRANS->link_cnt = pBODY->link_cnt;
								pLOG_HTTP_TRANS->href_cnt = pBODY->href_cnt;

								if(pBODY->redirect_url_len){
									URL_ANALYSIS *pURL_ANALYSIS;
									pURL_ANALYSIS = (URL_ANALYSIS *)sss;
									memset(sss,0,BUFSIZ);
									URL_ANALYSIS_URL_S_LEX((char*)pBODY->redirect_url,pBODY->redirect_url_len,(char*)sss);
									if(pURL_ANALYSIS->ContentID[0]){
										memcpy(pLOG_HTTP_TRANS->ContentID , pURL_ANALYSIS->ContentID , MAX_CONTENTID_SIZE);
									}
								}
							}
						} else {
							log_print(LOGN_CRI, "ERROR %s : LOG_HTTP_TRANS가 존재하지 않는다!!!",PrintTYPE(type));
						}
						break;
					case TCP_INFO_DEF_NUM:
						pstTCPINFO = (TCP_INFO *)data;

						switch( pstTCPINFO->cTcpFlag )
						{
						case DEF_TCP_START:
							dRet = dStart_Session( pstTCPINFO );
							if( dRet < 0 )
								log_print( LOGN_INFO, "EXCENTIONAL CASE IN dStart_Session dRet:%d", dRet );
							
							break;
						case DEF_TCP_END:
							dRet = dEnd_Session( pstTCPINFO );
							if( dRet < 0 )
								log_print( LOGN_INFO, "EXCENTIONAL CASE IN dEnd_Session dRet:%d", dRet );

							break;
						default:
							log_print( LOGN_DEBUG, "OTHER CASE TCP FLAG:%d %d", pstTCPINFO->cTcpFlag, __LINE__ ); 

							break;
						}

						ucControlFlag = pstTCPINFO->cTcpFlag;

						break;
					case CAP_HEADER_NUM:
						pCAPHEAD = (Capture_Header_Msg *)data;	

						break;
					case ETH_DATA_NUM:
						
						break;
					case INFO_ETH_NUM:
						pINFOETH = (INFO_ETH *)data;
					
						break;
					default:
						log_print(LOGN_INFO, "????? UNKNOWN TYPE[%d]", type);
						break;
					}

					if(ismalloc == DEF_READ_MALLOC){ free(data); }
				}
				
				pNextNode = (U8 *)nifo_entry(nifo_ptr(pMEMSINFO, ((NIFO *)pNextNode)->nont.offset_next), NIFO, nont);

			} while(pNode != pNextNode);

			log_print(LOGN_INFO, "NEW_MSG_END" );


			if( ucControlFlag != 0 ) {
				ucControlFlag = 0;

				dSeqProcID = SEQ_PROC_A_CALL + ( ((LOG_COMMON *)data)->uiClientIP % gACALLCnt );
				/* RETRANSMISSION TO CALL START & STOP MSG */
				dRet = dSend_VOD_Data( pMEMSINFO, dSeqProcID, pNode );
				if( dRet < 0 )
					log_print( LOGN_CRI, "FAIL IN dSend_VOD_Data dRet SEND START & STOP dRet=%d", dRet );

				continue;
			}

			if( pCAPHEAD != NULL && pINFOETH != NULL ) {
				/* CASE : UDP PACKET */
				dRet = dRTCP_Session( offset, pINFOETH, pCAPHEAD );
                if( dRet < 0 ) {
                    log_print( LOGN_INFO, LH"FAIL IN dRTCP_Session dRet:%d", LT, dRet );
                }

                nifo_node_delete(pMEMSINFO, pNode);
                continue;

			}
			else {
				/* CASE : RTSP */
				pstVODSESSNode = pCheck_Session( pLOG_HTTP_TRANS );
                if( pstVODSESSNode == NULL ) {
                    log_print( LOGN_INFO, "CANNOT FIND SESSION!!" );
                
                    nifo_node_delete(pMEMSINFO, pNode);
                    continue;
                }
				else {
					/* VOD SESSION에 대한 정보 처리 */
            		dRet = dProc_VODSESS( pLOG_HTTP_TRANS, pstVODSESSNode,
                                  		  pdata_req_hdr, pdata_resp_hdr, pdata_resp_body,
                                  		  data_req_hdr_len, data_resp_hdr_len, data_resp_body_len );
            		if( dRet < 0 )
                		log_print( LOGN_DEBUG, "FAIL IN dProc_VODSESS dRet:%d", dRet );

				}
			}


			if(pLOG_HTTP_TRANS){
				MENU_TITLE_KEY	aMENU_TITLE_KEY;
				MENU_TITLE_DATA	*pMENU_TITLE_DATA;
				stHASHGNODE 	*pMENUHASHGNODE;

				pst_VODSESS     pstSessData;
				/*
				stHASHONODE     *pstVODSess;
				*/

				/* download 처리 */
				if((pLOG_HTTP_TRANS->usSvcL4Type != L4_VOD_STREAM) && (pLOG_HTTP_TRANS->usSvcL4Type != L4_MBOX)) {
					int dn = 0;
					vod_dn((char*)pLOG_HTTP_TRANS->szLOGURL, pLOG_HTTP_TRANS->usLOGURLSize, &dn);
					if(dn != 1) {
						pLOG_HTTP_TRANS->usSvcL4Type = L4_DN_VOD_NODN;
					}
				}

				/*
				 * SET FB, WB L4Type Code 2008.06.30 BY LDH
				 */
				pstSessData = (pst_VODSESS)nifo_ptr( pstVODSESSHASH, pstVODSESSNode->offset_Data);
				if(pstSessData) {
					if( (pstSessData->stVODSESS.usSvcL4Type == L4_RTS_FB) || (pstSessData->stVODSESS.usSvcL4Type == L4_RTS_WB) )
						pLOG_HTTP_TRANS->usSvcL4Type = pstSessData->stVODSESS.usSvcL4Type;
				}

				if(pBODY != NULL) {
					switch(pLOG_HTTP_TRANS->ucMethod)
					{
					case METHOD_GET :
					case METHOD_POST :
						TAG_KEY_LVOD_CONF.uiContentType 	= pLOG_HTTP_TRANS->usContentsType;
						TAG_KEY_LVOD_CONF.Redirect_Protocol = ((BODY *)pBODY)->redirect_url_type;

						log_print(LOGN_INFO,"### Before Search Key CType=%ld Red = %d AppCode = %u SvcL7Type = %ld",
							pLOG_HTTP_TRANS->usContentsType, ((BODY *)pBODY)->redirect_url_type, pLOG_HTTP_TRANS->uiPageID, pLOG_HTTP_TRANS->usSvcL7Type);

						if( (pstHASHNODE = hashg_find(pLVODHASH, (U8 *) &TAG_KEY_LVOD_CONF)) ){
							pLVOD_CONF 						= (LVOD_CONF *)pstHASHNODE->pstData;
							pLOG_HTTP_TRANS->uiPageID 		= pLVOD_CONF->AppCode;
							pLOG_HTTP_TRANS->usSvcL7Type 	= pLVOD_CONF->L7Code;

							log_print(LOGN_INFO,"After Search AppCode = %u SvcL7Type = %ld",pLOG_HTTP_TRANS->uiPageID, pLOG_HTTP_TRANS->usSvcL7Type);
						}
						break;
					case METHOD_DESCRIBE :
                    case METHOD_SETUP :
                    case METHOD_PLAY :
                    case METHOD_PAUSE :
					case METHOD_TEARDOWN :
					case METHOD_HEAD :
					case METHOD_OPTIONS :
					case METHOD_PUT :
					case METHOD_DELETE :
					case METHOD_TRACE :
					case METHOD_CONNECT :
					case METHOD_ANNOUNCE :
					case METHOD_GET_PARAMETER :
					case METHOD_RECORD :
					case METHOD_REDIRECT :
					case METHOD_SET_PARAMETER :
					case METHOD_RESULT :
					default :
						log_print(LOGN_WARN, "LOG_HTTP_TRANS Not Support BODY =[%ld]", 
							pLOG_HTTP_TRANS->ucMethod);
						break;
					}
				}

				/* Set MenuTitle from the URL */
				/// ??? shlee : get the absolute URL and length.
				/// === URLbuf_len, URLbuf을 사용하시면 됩니다. ///
				memset(&aMENU_TITLE_KEY,0x00,MENU_TITLE_KEY_SIZE);
				aMENU_TITLE_KEY.href_len =  URLbuf_len; 							// get the absolute url length 
				memcpy(aMENU_TITLE_KEY.href , URLbuf , aMENU_TITLE_KEY.href_len); 	// get the absolute url

				pMENUHASHGNODE = hashg_find(pMENUHASHGINFO,(U8 *) &aMENU_TITLE_KEY);

				//MENU_TITLE_KEY_Prt("MENU_TITLE_KEY",&aMENU_TITLE_KEY);
				log_print(LOGN_INFO, "MENU_TITLE_KEY Matching : %p",pMENUHASHGNODE);

				if(pMENUHASHGNODE){
					pMENU_TITLE_DATA = (MENU_TITLE_DATA *) pMENUHASHGNODE->pstData;

					//MENU_TITLE_DATA_Prt("MENU_TITLE_DATA",pMENU_TITLE_DATA);

					pMENU_TITLE_DATA->timerNID = timerN_update(pTIMERNINFO, pMENU_TITLE_DATA->timerNID, time(NULL) + DEF_MENUTITLE_TIMEOUT);   // timer update
					memcpy(pLOG_HTTP_TRANS->szMenuTitle, pMENU_TITLE_DATA->menutitle ,pMENU_TITLE_DATA->menutitle_len) ;
					pLOG_HTTP_TRANS->szMenuTitle[pMENU_TITLE_DATA->menutitle_len] = 0;
				}

				/* DATA NODE DELETE */
				pNextNode = (U8 *)nifo_entry(nifo_ptr(pMEMSINFO, ((NIFO *)pNode)->nont.offset_next), NIFO, nont);
				nifo_node_unlink_nont(pMEMSINFO, pNode);
				nifo_node_delete(pMEMSINFO, pNextNode);
				if(pBodyNode){
					nifo_node_link_nont_prev(pMEMSINFO, pNode, pBodyNode);
				}
				dSeqProcID = SEQ_PROC_A_CALL + ( pLOG_HTTP_TRANS->uiClientIP % gACALLCnt );
				if((dRet = dSend_VOD_Data(pMEMSINFO, dSeqProcID, pNode)) < 0) {
					log_print(LOGN_CRI, LH"MSGQ WRITE FAILE[%d][%s]", LT, dRet, strerror(-dRet));
					break;
				}
			} else {
				log_print(LOGN_CRI,"VOD : pLOG_HTTP_TRANS is NULL");
				/* Node 삭제 */
				nifo_node_delete(pMEMSINFO, pNode);

				continue;
			}
		} else {
			usleep(0);
		}
			
	}

	FinishProgram();

	return 0;
}



/* Code Sample 
dRet = Devide_ReqURL(usURLSize, szURL, &stLIST);
if(dRet < 0) {  // Error 
		log_print(LOGN_CRI, "Devide_ReqURL Ret =[%d] < 0", dRet);
		return -1;
} else if(dRet > 0) { // Warning
		log_print(LOGN_DEBUG, "Devide_ReqURL Ret=[%d] URL[%s] > 0", dRet, szURL);
}
*/


/*
 *  $Log: vod_main.c,v $
 *  Revision 1.2  2011/09/06 12:46:39  hhbaek
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *  NEW OAM SYSTEM
 *
 *  Revision 1.4  2011/08/21 09:07:53  hhbaek
 *  Commit TAF/SRC/ *
 *
 *  Revision 1.3  2011/08/17 07:26:30  dcham
 *  *** empty log message ***
 *
 *  Revision 1.2  2011/08/08 11:05:43  uamyd
 *  modified block added
 *
 *  Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 *  init DQMS2
 *
 *  Revision 1.7  2011/05/09 15:23:29  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.6  2011/01/11 04:09:10  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2010/08/23 01:12:59  uamyd
 *  DQMS With TOTMON, 2nd-import
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
 *  Revision 1.1.1.1  2009/05/26 02:14:29  dqms
 *  Init TAF_RPPI
 *
 *  Revision 1.8  2008/12/19 02:22:42  dark264sh
 *  A_VOD log_print Level 변경
 *
 *  Revision 1.7  2008/12/18 07:18:18  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.6  2008/12/10 06:02:08  jsyoon
 *  bug
 *
 *  Revision 1.5  2008/07/04 08:55:37  jsyoon
 *  LOG_TCP_SESS를 VOD/RTS로 구분하기 위한 처리
 *
 *  Revision 1.4  2008/07/04 08:20:19  dark264sh
 *  LOG_TCP_SESS를 VOD/RTS로 구분하기 위한 처리
 *
 *  Revision 1.3  2008/07/02 07:24:52  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.2  2008/07/02 06:35:06  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2008/06/09 08:17:16  jsyoon
 *  WATAS3 PROJECT START
 *
 *  Revision 1.19  2007/11/07 02:08:50  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.18  2007/10/08 04:55:29  dark264sh
 *  no message
 *
 *  Revision 1.17  2007/09/07 07:58:52  watas
 *  *** empty log message ***
 *
 *  Revision 1.16  2007/09/06 02:17:55  watas
 *  *** empty log message ***
 *
 *  Revision 1.15  2007/09/05 08:34:29  watas
 *  *** empty log message ***
 *
 *  Revision 1.14  2007/09/05 07:29:37  watas
 *  *** empty log message ***
 *
 *  Revision 1.13  2007/09/05 07:18:32  watas
 *  *** empty log message ***
 *
 *  Revision 1.12  2007/09/05 07:16:03  watas
 *  *** empty log message ***
 *
 *  Revision 1.11  2007/09/05 06:34:32  watas
 *  *** empty log message ***
 *
 *  Revision 1.10  2007/09/05 06:25:19  watas
 *  *** empty log message ***
 *
 *  Revision 1.9  2007/09/05 06:16:40  watas
 *  *** empty log message ***
 *
 *  Revision 1.8  2007/09/05 02:32:06  watas
 *  *** empty log message ***
 *
 *  Revision 1.7  2007/09/05 01:34:08  watas
 *  *** empty log message ***
 *
 *  Revision 1.6  2007/09/05 01:18:35  watas
 *  *** empty log message ***
 *
 *  Revision 1.5  2007/09/05 01:06:49  watas
 *  *** empty log message ***
 *
 *  Revision 1.4  2007/09/04 11:09:08  watas
 *  *** empty log message ***
 *
 *  Revision 1.3  2007/09/03 13:25:22  watas
 *  *** empty log message ***
 *
 *  Revision 1.2  2007/09/03 09:35:11  watas
 *  *** empty log message ***
 *
 *  Revision 1.1  2007/08/21 12:54:39  dark264sh
 *  no message
 *
 *  Revision 1.25  2006/11/28 15:33:39  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.24  2006/11/28 12:16:35  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.23  2006/11/16 06:10:46  cjlee
 *  invoke_timer () 추가
 *
 *  Revision 1.22  2006/11/16 05:28:12  cjlee
 *  *** empty log message ***
 *
 *  Revision 1.21  2006/11/16 05:27:13  cjlee
 *  *** empty log message ***
 *
 *  Revision 1.20  2006/11/16 05:18:52  cjlee
 *  내용  : MENU관련 변경
 *  	- INC/   *.h  : init함수의 extern 선언 변경
 *  	- BODY.stc
 *  	   parsing rule 변경
 *  	   url="  http:// 이런 식도 처리 가능
 *  	   <a ...  href=.. 등도 처리 가능
 *  	   hashg를 이용한 add 추가 (href에 대해서)
 *  	   관련 함수 추가
 *  	- aqua.pstg
 *  	   MENUTITLE관련 structure및 deifne 추가
 *  	- A_BREW , A_MEKUN , A_WIPI , A_VOD
 *  	  *init , *main : MENU관련 처리 추가
 *  	  URL을 받은후에 hash에서 비교를 하여 적당한 메뉴명을 넣는다.
 *
 *  Revision 1.19  2006/11/14 10:23:48  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.18  2006/11/14 09:32:00  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.17  2006/11/13 10:48:43  shlee
 *  A_RTSP기능을 A_VOD에 통합 처리
 *
 *  Revision 1.16  2006/11/12 12:06:24  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.15  2006/11/10 12:39:12  cjlee
 *  BODY에 포함되는 link수를 센다.  LOG_HTTP_TRANS->link_cnt @CILOG_HIDDEN@
 *  BODY_LEX()를 한후에 이 값을 HTTP안에 넣어준다.
 *  LOG_PAGE_TRANS->TrialReqCnt에 들어오는 첫번째 이 값을 넣어준다.
 *
 *  Revision 1.14  2006/11/08 07:32:04  shlee
 *  CONF관련 hasho -> hashg로 변경 및 CONF_CNT 101 CONF_PREA_CNT 811로 변경
 *
 *  Revision 1.13  2006/11/06 07:42:33  dark264sh
 *  nifo NODE size 4*1024 => 6*1024로 변경하기
 *  nifo_tlv_alloc에서 argument로 memset할지 말지 결정하도록 수정
 *  nifo_node_free에서 semaphore 삭제
 *
 *  Revision 1.12  2006/11/06 07:07:36  shlee
 *  METHOD AUTO_STRING
 *
 *  Revision 1.11  2006/10/26 10:48:47  shlee
 *  BODY URL LIST
 *
 *  Revision 1.10  2006/10/26 02:47:36  cjlee
 *  *** empty log message ***
 *
 *  Revision 1.9  2006/10/26 02:15:05  shlee
 *  Make Absolute URL
 *
 *  Revision 1.8  2006/10/25 08:51:34  shlee
 *  TAG_KEY Matching
 *
 *  Revision 1.7  2006/10/20 10:03:10  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.6  2006/10/18 08:53:31  dark264sh
 *  nifo debug 코드 추가
 *
 *  Revision 1.5  2006/10/18 03:08:20  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.4  2006/10/17 03:50:55  dark264sh
 *  nifo_tlv_alloc에 memset 추가로 인한 변경
 *
 *  Revision 1.3  2006/10/16 13:47:05  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.2  2006/10/13 05:04:30  cjlee
 *  *** empty log message ***
 *
 *  Revision 1.1  2006/10/12 15:17:21  shlee
 *  INIT & Compile but check TAG_KEY_LVOD_CONF
 *
 */
