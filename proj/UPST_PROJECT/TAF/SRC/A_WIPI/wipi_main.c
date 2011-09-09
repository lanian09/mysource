/**		@file	wipi_main.c
 * 		- HTTP Transaction�� ���� �ϴ� ���μ���
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: wipi_main.c,v 1.2 2011/09/06 12:46:41 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.2 $
 * 		@date		$Date: 2011/09/06 12:46:41 $
 * 		@warning	.
 * 		@ref		http_main.c l4.h http_init.c http_func.c
 * 		@todo		library�� ��������� ���� ����, library ���� ���� �Լ� ��ġ
 *
 * 		@section	Intro(�Ұ�)
 * 		- HTTP Transaction�� ���� �ϴ� ���μ���
 *
 * 		@section	Requirement
 * 		 @li library ���� ���� �Լ� ��ġ
 *
 **/

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
#include "sshmid.h"

// LIB
#include "mems.h"
#include "cifo.h"
#include "gifo.h"
#include "nifo.h"
#include "loglib.h"
#include "verlib.h"

// TAF
#include "debug.h"

// TOOLS
#include "tools.h"	/* dGetPlatformType() */

// .
#include "wipi_init.h"
#include "wipi_msgq.h"

/**
 *	Declare var.
 */
S32				giFinishSignal;		/**< Finish Signal */
S32				giStopFlag;			/**< main loop Flag 0: Stop, 1: Loop */

stMEMSINFO 	    *pMEMSINFO;			/**< new interface ���� ����ü */
stCIFO		    *gpCIFO;
static stHASHGINFO	*pLWIPIHASH;

extern int      gACALLCnt;

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
	default: 						return (U8*)"UNKNOWN";
	}
}
/** main function.
 *
 *  man Function
 *
 *  @param	argc	:	�Ķ���� ����
 *  @param	*argv[]	:	�Ķ����
 *
 *  @return			S32
 *  @see			http_main.c l4.h http_init.c http_func.c
 *
 *  @exception		.
 *  @note			.
 **/
S32 main(S32 argc, S8 *argv[])
{
	S32				dRet;		/**< �Լ� Return �� */
	OFFSET			offset;
	U8				*pNode;
	U8				*pNextNode;
	U8				*p, *data;
	S32				type, len, ismalloc;
	U8				*pBodyNode;
	U16             URLbuf_len;
	U8              URLbuf[BUFSIZ];
	BODY    		*pBODY;
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
	TAG_KEY_LWIPI_CONF	aTAG_KEY_LWIPI_CONF;
	LWIPI_CONF		*pLWIPI_CONF;

    char    vERSION[7] = "R3.0.0";


	/* log_print �ʱ�ȭ */
	log_init(S_SSHM_LOG_LEVEL, getpid(), SEQ_PROC_A_WIPI, LOG_PATH"/A_WIPI", "A_WIPI");

	/* A_WIPI �ʱ�ȭ */
	if((dRet = dInitWIPI(&pMEMSINFO, &pLWIPIHASH)) < 0)
	{
		log_print(LOGN_CRI, LH"dInitWIPI dRet[%d]", LT, dRet);
		exit(0);
	}

    if((dRet = set_version(S_SSHM_VERSION, SEQ_PROC_A_WIPI, vERSION)) < 0 ) {
        log_print(LOGN_CRI, "SET_VERSION ERROR(RET=%d,IDX=%d,VER=%s)", dRet, SEQ_PROC_A_WIPI, vERSION);
    }
	log_print(LOGN_CRI, "START WIPI(%s)", vERSION);

	/* MAIN LOOP */
	while(giStopFlag)
	{
		if((offset = gifo_read(pMEMSINFO, gpCIFO, SEQ_PROC_A_WIPI)) > 0) {

			log_print(LOGN_INFO, "======================================================================");

			/* DB LOG ������ �������� �ϴ� NODE (���� ���� �ʰ� �����ϱ� ���� )*/
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
						// URL���� port ������ buffer�Ѱ� ������ �ֱ� : shlee
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
							log_print(LOGN_CRI, "ERROR %s : LOG_HTTP_TRANS�� �������� �ʴ´�!!!",PrintTYPE(type));
						}
						break;
					case HTTP_REQ_BODY_NUM:
						pdata_req_body = (char *) data;
						data_req_body_len = len;
						log_print(LOGN_INFO, "TYPE[%s] len %d DATA[%.*s]", PrintTYPE(type), len ,len, data);				
						if(pLOG_HTTP_TRANS){
						} else {
							log_print(LOGN_CRI, "ERROR %s : LOG_HTTP_TRANS�� �������� �ʴ´�!!!",PrintTYPE(type));
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
							log_print(LOGN_CRI, "ERROR %s : LOG_HTTP_TRANS�� �������� �ʴ´�!!!",PrintTYPE(type));
						}
						break;
					case HTTP_RES_BODY_NUM:
						pdata_resp_body = (char *) data;
						data_resp_body_len = len;
						log_print(LOGN_INFO, "TYPE[%s] len %d DATA[%.*s]", PrintTYPE(type), len ,len, data);				
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
								BODY_LEX(pdata_resp_body, data_resp_body_len, pbody_data, &UrlParseList, pMEMSINFO);
								pLOG_HTTP_TRANS->link_cnt = pBODY->link_cnt;
								pLOG_HTTP_TRANS->href_cnt = pBODY->href_cnt;
							}
						} else {
							log_print(LOGN_CRI, "ERROR %s : LOG_HTTP_TRANS�� �������� �ʴ´�!!!",PrintTYPE(type));
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
				aTAG_KEY_LWIPI_CONF.URL = Get_TAG_AUTO_STRING_DEF_SVCACTION((char*)pLOG_HTTP_TRANS->SvcAction);
				if( (pstHASHNODE = hashg_find(pLWIPIHASH, (U8 *) &aTAG_KEY_LWIPI_CONF)) ){
					pLWIPI_CONF = (LWIPI_CONF *)pstHASHNODE->pstData;

					pLOG_HTTP_TRANS->uiPageID = pLWIPI_CONF->AppCode;
					pLOG_HTTP_TRANS->usSvcL7Type = pLWIPI_CONF->L7Code;
					pLOG_HTTP_TRANS->usPlatformType = 
						dGetPlatformType(pLOG_HTTP_TRANS->usSvcL4Type, pLOG_HTTP_TRANS->usSvcL7Type);
					log_print(LOGN_INFO,"CommandID = %lu AppCode = %lu L7Type = %ld\n", 
						pLWIPI_CONF->URL, pLWIPI_CONF->AppCode, pLWIPI_CONF->L7Code);
//					LOG_HTTP_TRANS_Prt("PRINT LOG_HTTP_TRANS2", pLOG_HTTP_TRANS);
				}

				/* DATA NODE DELETE */
				pNextNode = (U8 *)nifo_entry(nifo_ptr(pMEMSINFO, ((NIFO *)pNode)->nont.offset_next), NIFO, nont);
				nifo_node_unlink_nont(pMEMSINFO, pNode);
				nifo_node_delete(pMEMSINFO, pNextNode);
				if(pBodyNode){
					nifo_node_link_nont_prev(pMEMSINFO, pNode, pBodyNode);
				}
				dSeqProcID = SEQ_PROC_A_CALL + ( pLOG_HTTP_TRANS->uiClientIP % gACALLCnt );
				if((dRet = dSend_WIPI_Data(pMEMSINFO, dSeqProcID, pNode)) < 0) {
					log_print(LOGN_CRI, LH"FAILED IN GIFO WRITE, TO=%d"EH, LT, dSeqProcID, ET);
					break;
				}
			} else {
				log_print(LOGN_CRI,"WIPI : pLOG_HTTP_TRANS is NULL");
				/* Node ���� */
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
 * $Log: wipi_main.c,v $
 * Revision 1.2  2011/09/06 12:46:41  hhbaek
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * NEW OAM SYSTEM
 *
 * Revision 1.4  2011/08/21 09:07:53  hhbaek
 * Commit TAF/SRC/ *
 *
 * Revision 1.3  2011/08/17 07:27:42  dcham
 * *** empty log message ***
 *
 * Revision 1.2  2011/08/05 09:04:50  uamyd
 * modified
 *
 * Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 * init DQMS2
 *
 * Revision 1.9  2011/05/09 15:24:41  jsyoon
 * *** empty log message ***
 *
 * Revision 1.8  2011/01/11 04:09:11  uamyd
 * modified
 *
 * Revision 1.1.1.1  2010/08/23 01:12:58  uamyd
 * DQMS With TOTMON, 2nd-import
 *
 * Revision 1.7  2009/08/25 02:02:58  jsyoon
 * WIPI�� L7CODE�� APP_DOWN�� ��� �÷���Ÿ���� DOWNLOAD�� ����
 *
 * Revision 1.6  2009/08/19 12:31:40  pkg
 * LOG_XXX_Prt �Լ� �ּ� ó��
 *
 * Revision 1.5  2009/08/06 06:56:09  dqms
 * �α׷��� �����޸𸮷� ����
 *
 * Revision 1.4  2009/07/15 17:10:56  dqms
 * set_version ��ġ �� Plastform Type ����
 *
 * Revision 1.3  2009/06/28 12:57:45  dqms
 * ADD set_version
 *
 * Revision 1.2  2009/06/12 11:09:36  jsyoon
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2009/05/26 02:14:27  dqms
 * Init TAF_RPPI
 *
 * Revision 1.2  2008/07/02 07:23:49  dark264sh
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2008/06/09 08:17:17  jsyoon
 * WATAS3 PROJECT START
 *
 * Revision 1.3  2007/08/29 12:44:55  dark264sh
 * *** empty log message ***
 *
 * Revision 1.2  2007/08/29 09:25:14  dark264sh
 * *** empty log message ***
 *
 * Revision 1.1  2007/08/21 12:55:11  dark264sh
 * no message
 *
 * Revision 1.23  2006/11/28 15:33:52  dark264sh
 * *** empty log message ***
 *
 * Revision 1.22  2006/11/28 12:16:35  dark264sh
 * *** empty log message ***
 *
 * Revision 1.21  2006/11/16 06:10:46  cjlee
 * invoke_timer () �߰�
 *
 * Revision 1.20  2006/11/16 06:02:09  cjlee
 * *** empty log message ***
 *
 * Revision 1.19  2006/11/16 05:18:52  cjlee
 * ����  : MENU���� ����
 * 	- INC/ *.h  : init�Լ��� extern ���� ����
 * 	- BODY.stc
 * 	   parsing rule ����
 * 	   url="  http:// �̷� �ĵ� ó�� ����
 * 	   <a ...  href=.. � ó�� ����
 * 	   hashg�� �̿��� add �߰� (href�� ���ؼ�)
 * 	   ���� �Լ� �߰�
 * 	- aqua.pstg
 * 	   MENUTITLE���� structure�� deifne �߰�
 * 	- A_BREW , A_MEKUN , A_WIPI , A_VOD
 * 	  *init , *main : MENU���� ó�� �߰�
 * 	  URL�� �����Ŀ� hash���� �񱳸� �Ͽ� ������ �޴����� �ִ´�.
 *
 * Revision 1.18  2006/11/14 10:23:24  dark264sh
 * *** empty log message ***
 *
 * Revision 1.17  2006/11/14 09:31:10  dark264sh
 * *** empty log message ***
 *
 * Revision 1.16  2006/11/12 12:04:39  dark264sh
 * *** empty log message ***
 *
 * Revision 1.15  2006/11/10 12:39:12  cjlee
 * BODY�� ���ԵǴ� link���� ����.  LOG_HTTP_TRANS->link_cnt @CILOG_HIDDEN@
 * BODY_LEX()�� ���Ŀ� �� ���� HTTP�ȿ� �־��ش�.
 * LOG_PAGE_TRANS->TrialReqCnt�� ������ ù��° �� ���� �־��ش�.
 *
 * Revision 1.14  2006/11/08 07:29:43  shlee
 * CONF���� hasho -> hashg�� ���� �� CONF_CNT 101 CONF_PREA_CNT 811�� ����
 *
 * Revision 1.13  2006/11/06 07:39:13  dark264sh
 * nifo NODE size 4*1024 => 6*1024�� �����ϱ�
 * nifo_tlv_alloc���� argument�� memset���� ���� �����ϵ��� ����
 * nifo_node_free���� semaphore ����
 *
 * Revision 1.12  2006/10/26 10:48:43  shlee
 * BODY URL LIST
 *
 * Revision 1.11  2006/10/26 02:47:54  cjlee
 * *** empty log message ***
 *
 * Revision 1.10  2006/10/20 10:03:39  dark264sh
 * *** empty log message ***
 *
 * Revision 1.9  2006/10/20 05:04:49  cjlee
 * *** empty log message ***
 *
 * Revision 1.8  2006/10/18 08:53:31  dark264sh
 * nifo debug �ڵ� �߰�
 *
 * Revision 1.7  2006/10/18 03:08:20  dark264sh
 * *** empty log message ***
 *
 * Revision 1.6  2006/10/17 03:50:55  dark264sh
 * nifo_tlv_alloc�� memset �߰��� ���� ����
 *
 * Revision 1.5  2006/10/16 13:46:44  dark264sh
 * *** empty log message ***
 *
 * Revision 1.4  2006/10/13 05:11:36  shlee
 * SvcAction Update
 *
 * Revision 1.3  2006/10/13 05:05:24  cjlee
 * *** empty log message ***
 *
 * Revision 1.2  2006/10/12 14:53:40  shlee
 * Compile Succ, Not testedw
 *
 * Revision 1.1  2006/10/12 14:31:13  shlee
 * INIT
 *
 */
