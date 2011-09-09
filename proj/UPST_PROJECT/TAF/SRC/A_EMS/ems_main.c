/**		@file	ems_main.c
 * 		- EMS Transaction을 관리 하는 프로세스
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: ems_main.c,v 1.2 2011/09/04 12:16:49 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.2 $
 * 		@date		$Date: 2011/09/04 12:16:49 $
 * 		@warning	.
 * 		@ref		ems_main.c ems_init.c
 * 		@todo		library가 만들어지지 않은 상태, library 생성 이후 함수 대치
 *
 * 		@section	Intro(소개)
 * 		- EMS Transaction을 관리 하는 프로세스
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
#include "ems.h"

// TAF
#include "debug.h"

/**
 *	Declare var.
 */
S32				giFinishSignal;		/**< Finish Signal */
S32				giStopFlag;			/**< main loop Flag 0: Stop, 1: Loop */

stMEMSINFO		*pMEMSINFO;			/**< new interface 관리 구조체 */
stCIFO			*gpCIFO;

S32				dMyQID;
S32				dCallQID[MAX_SMP_NUM];
extern int		gACALLCnt;

/**
 *	Declare extern func.
 */
extern S32 dInitEMS(stMEMSINFO **pMEMSINFO);
extern S32 dSend_EMS_Data(stMEMSINFO *pMEMSINFO, S32 dSeqProcID, U8 *pNode);
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
	default:						return (U8*)"UNKNOWN";
	}
}

U16 SetCommandValue(char *sp)
{
	U16		uRet;

	if (!strcmp(sp, "mailserveradd")) { uRet = APP_EMS_SRVADD; }
	else if (!strcmp(sp, "mailserveradddirect")) { uRet = APP_EMS_SRVADD; }
	else if (!strcmp(sp, "mailservermodify")) { uRet = APP_EMS_SRVMOD; }
	else if (!strcmp(sp, "mailservermodifydirect")) { uRet = APP_EMS_SRVMOD; }
	else if (!strcmp(sp, "mailserverdelete")) { uRet = APP_EMS_SRVDEL; }
	else if (!strcmp(sp, "spamaddlist")) { uRet = APP_EMS_SPAMADD; }
	else if (!strcmp(sp, "spamdellist")) { uRet = APP_EMS_SPAMDEL; }
	else if (!strcmp(sp, "mailrecvtime")) { uRet = APP_EMS_RCVTIME; }
	else if (!strcmp(sp, "newmaillist")) { uRet = APP_EMS_NEWMAIL; }
	else if (!strcmp(sp, "mailbodyrecv")) { uRet = APP_EMS_BODY; }
	else if (!strcmp(sp, "sendmail")) { uRet = APP_EMS_SEND; }
	else if (!strcmp(sp, "acknowledgement")) { uRet = APP_EMS_ACK; }
	else if (!strcmp(sp, "mailserversync")) { uRet = APP_EMS_SYNC; }
	else if (!strcmp(sp, "spamsync")) { uRet = APP_EMS_SYNC; }
	else if (!strcmp(sp, "mailrecvtimesync")) { uRet = APP_EMS_SYNC; }
	else {
		log_print(LOGN_CRI, "UNKNOWN COMMAND[%s]", sp);
		uRet = 0;
	}

	return uRet;
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

	char			szCommand[MAX_COMMAND_SIZE];

    char    vERSION[7] = "R3.0.0";


	/* log_print 초기화 */
	log_init(S_SSHM_LOG_LEVEL,getpid(), SEQ_PROC_A_EMS, LOG_PATH"/A_EMS", "A_EMS");

	/* A_EMS 초기화 */
	if((dRet = dInitEMS(&pMEMSINFO)) < 0)
	{
		log_print(LOGN_CRI, "[%s][%s.%d] dInitEMS dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		exit(0);
	}

    if((dRet = set_version(S_SSHM_VERSION, SEQ_PROC_A_EMS, vERSION)) < 0 ) {
        log_print(LOGN_CRI, "SET_VERSION ERROR(RET=%d,IDX=%d,VER=%s)", dRet, SEQ_PROC_A_EMS, vERSION);
    }
	log_print(LOGN_CRI, "START EMS(%s)", vERSION);

	/* MAIN LOOP */
	while(giStopFlag)
	{
		if((offset = gifo_read(pMEMSINFO, gpCIFO, SEQ_PROC_A_EMS)) > 0) {

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
						log_print(LOGN_INFO, "TYPE[%s] len %d DATA[\n%.*s]", PrintTYPE(type), len ,len, data);				
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
						log_print(LOGN_INFO, "TYPE[%s] len %d DATA[\n%.*s]", PrintTYPE(type), len ,len, data);				
						if(pLOG_HTTP_TRANS){
							if(pdata_req_body && data_req_body_len) {
								emsreqbody(pdata_req_body, data_req_body_len, (char*)pLOG_HTTP_TRANS->szTransactionID, &pLOG_HTTP_TRANS->usClientType, &pLOG_HTTP_TRANS->usClientPlatform, (char*)pLOG_HTTP_TRANS->szClientVersion, szCommand, (char*)pLOG_HTTP_TRANS->szSmtpServer, (char*)pLOG_HTTP_TRANS->szPop3Server, &pLOG_HTTP_TRANS->usSmtpSsl, &pLOG_HTTP_TRANS->usPop3Ssl, &pLOG_HTTP_TRANS->uiPeriodTime, &pLOG_HTTP_TRANS->usParam, &pLOG_HTTP_TRANS->usImageRecv);
								pLOG_HTTP_TRANS->usCommand = SetCommandValue(szCommand);
								pLOG_HTTP_TRANS->usSvcL7Type = pLOG_HTTP_TRANS->usCommand;
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
								emsrespbody(pdata_resp_body, data_resp_body_len, (char*)pLOG_HTTP_TRANS->szTransactionID, (char*)pLOG_HTTP_TRANS->szAppFailCode);
								/* for AppFailCode */
								if( (pLOG_HTTP_TRANS->usUserErrorCode == 0) && 
									(atoi((char*)pLOG_HTTP_TRANS->szAppFailCode) > 5)) {
									pLOG_HTTP_TRANS->usUserErrorCode = HTTP_UERR_980;
									pLOG_HTTP_TRANS->usL7FailCode = HTTP_UERR_980;
								}
								if( atoi((char*)pLOG_HTTP_TRANS->szAppFailCode) == 32 || 
									atoi((char*)pLOG_HTTP_TRANS->szAppFailCode) == 68) {
									pLOG_HTTP_TRANS->usSvcL4Type = L4_EMS_NO;
								}
//								LOG_HTTP_TRANS_Prt("PRINT LOG_HTTP_TRANS_EMS_RESP_BODY", pLOG_HTTP_TRANS);
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
				if((dRet = dSend_EMS_Data(pMEMSINFO, dSeqProcID, pNode)) < 0) {
					log_print(LOGN_CRI, "[%s][%s.%d] MSGQ WRITE FAILE[%d][%s]", __FILE__, __FUNCTION__, __LINE__, dRet, strerror(-dRet));
					break;
				}
			} else {
				log_print(LOGN_CRI,"EMS : pLOG_HTTP_TRANS is NULL");
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
 *  $Log: ems_main.c,v $
 *  Revision 1.2  2011/09/04 12:16:49  hhbaek
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *  NEW OAM SYSTEM
 *
 *  Revision 1.4  2011/08/21 09:07:50  hhbaek
 *  Commit TAF/SRC/ *
 *
 *  Revision 1.3  2011/08/17 07:16:00  dcham
 *  *** empty log message ***
 *
 *  Revision 1.2  2011/08/08 11:05:40  uamyd
 *  modified block added
 *
 *  Revision 1.7  2011/05/09 15:15:19  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.6  2011/01/11 04:09:06  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2010/08/23 01:13:03  uamyd
 *  DQMS With TOTMON, 2nd-import
 *
 *  Revision 1.5  2009/08/19 12:22:40  pkg
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
 *  Revision 1.1.1.1  2009/05/26 02:14:40  dqms
 *  Init TAF_RPPI
 *
 *  Revision 1.6  2008/07/14 07:47:31  jsyoon
 *  32,68 L4_EMS_NO로 설정
 *
 *  Revision 1.5  2008/06/26 13:24:51  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.4  2008/06/24 20:12:57  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.3  2008/06/19 12:12:26  jsyoon
 *  L4CODE 정의 변경, EMS 관련 L7CODE 추가
 *
 *  Revision 1.2  2008/06/19 08:16:43  jsyoon
 *  ems.h 파일 추가
 *
 *  Revision 1.1  2008/06/17 12:17:49  dark264sh
 *  init
 *
 */



