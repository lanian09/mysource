/**     @file   cilog_main.c
 *      - Client Interface
 *
 *      Copyright (c) 2006~ by Upresto Inc. Korea
 *      All rights reserved
 *
 *      $Id: ci_log_main.c,v 1.2 2011/09/06 12:46:38 hhbaek Exp $
 *
 *      @Author     $Author: hhbaek $
 *      @version    $Revision: 
 *      @date       $Date: 2011/09/06 12:46:38 $
 *      @warning    .
 *      @ref        
 *      @todo       
 *
 *      @section    
 *      - Client Interface
 *
 *      @section    Requirement
 *      @li 
 **/

/**
 *	Include headers
 */
#include <unistd.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <errno.h>

// TOP
#include "common_stg.h"
#include "commdef.h"
#include "procid.h"
#include "path.h"
#include "sshmid.h"
#include "filter.h"

// LIB
#include "mems.h"	/* stMEMSINFO */
#include "cifo.h"	/* stCIFO */
#include "gifo.h"	/* gifo_read() */
#include "loglib.h"
#include "verlib.h"
#include "nsocklib.h"

// .
#include "ci_log_init.h"
#include "ci_log_sock.h"
#include "ci_log_func.h"
#include "ci_log_util.h"

/**
 *	Define cons.
 */
#define DEF_LOGOUT_MSGREAD		10000
// TODO dqms_ipclib.h 에 있던 값
#define MAX_MQCHK_LOOP_CNT		100000 

/**
 *	Define var.
 */
st_Flt_Info 	*flt_info;
stMEMSINFO 		*pMEMSINFO;
stCIFO			*gpCIFO;

int     		g_JiSTOPFlag;
int     		g_FinishFlag;

fd_set  		g_readset;
fd_set  		g_writeset;

st_ClientInfo	g_stClientInfo;

int				g_dLogInStatus;   /* 0->log-out, 1->login*/
INT64   		g_llLastNID;

int				g_dServerSocket;
int     		g_dNumOfSocket;

/**
 *	Declare func.
 */
S32 dInitCI_LOG(stMEMSINFO **pMEMSINFO);
int dCheckLogType(int type, char *pdata);
char *PrintTYPE(int type);

/**
 *	Implement func.
 */
int main()
{
	int				i;
	int				dRet, dSendFlag = 1;

	OFFSET 			offset;
	UCHAR 			*pNode, *pNextNode;
	UCHAR 			*p, *pdata;
	U32 			type, len;
	S32				ismalloc;

	st_TraceMsgHdr  *pTRACEHDR = NULL;
	UCHAR 			*pPACKET = NULL;
	int 			TraceHdrLen, PacketLen;

    char    vERSION[7] = "R3.0.0";


	g_dLogInStatus = DEF_LOGOUT;

	log_init(S_SSHM_LOG_LEVEL,getpid(), SEQ_PROC_CI_LOG, LOG_PATH"/CI_LOG", "CI_LOG");

	dRet = dInitCI_LOG(&pMEMSINFO);
	if( dRet < 0 ) {
		log_print( LOGN_CRI, "Main : Failed in dInitProc" );
		exit(0);
	}

    if((dRet = set_version(S_SSHM_VERSION, SEQ_PROC_CI_LOG, vERSION)) < 0 ) {
        log_print(LOGN_CRI, "SET_VERSION ERROR(RET=%d,IDX=%d,VER=%s)", dRet, SEQ_PROC_CI_LOG, vERSION);
    }
	log_print( LOGN_CRI, "CI_LOG(%s) : PROCESS INIT SUCCESS, PROCESS START", vERSION);


	while( g_JiSTOPFlag ) 
	{
		/*** #### CHECK LOGOUT #### ****/
		if( g_dLogInStatus == DEF_LOGOUT ) {

			if(dCheckSock() < 0) {
				log_print(LOGN_CRI, "dProc_Msg : failed in dCheckSock");

				for( i=0; i<DEF_LOGOUT_MSGREAD; i++ ) {
					if((offset = gifo_read(pMEMSINFO, gpCIFO, SEQ_PROC_CI_LOG)) > 0) {
						pNode = nifo_ptr(pMEMSINFO, offset);
						nifo_node_delete(pMEMSINFO, pNode);
					}
					else
						break;
				}

				continue;
			}
		}

		/*** #### Check if MQ Msg In -> Process ##### ****/
		for( i = 0; i < MAX_MQCHK_LOOP_CNT; i++ ) {

			if((offset = gifo_read(pMEMSINFO, gpCIFO, SEQ_PROC_CI_LOG)) > 0) {

				pNode = nifo_ptr(pMEMSINFO, offset);
				pNextNode = pNode;

				do {
					p = pNextNode;

					while(p != NULL) {
						dSendFlag = 1;

						if((dRet = nifo_read_tlv_cont(pMEMSINFO, pNextNode, &type, &len, &pdata, &ismalloc, &p)) < 0)
							break;

						log_print(LOGN_INFO, "TYPE=%d:%s LEN=%d ISMAL=%s", 
								type, PrintTYPE(type),
								len, (ismalloc == DEF_READ_MALLOC) ? "MALL" : "ORI");

						switch(type)
						{
							case LOG_HTTP_TRANS_DEF_NUM:
							case LOG_PAGE_TRANS_DEF_NUM:
							case LOG_TCP_SESS_DEF_NUM:
								dSendFlag = dCheckLogType(type, (char*)pdata);
							case START_CALL_NUM:
							case STOP_CALL_NUM:
							case LOG_SIGNAL_DEF_NUM:
							case LOG_PISIGNAL_DEF_NUM:
							case LOG_IM_SESS_DEF_NUM:
							case LOG_VT_SESS_DEF_NUM:
							case LOG_SIP_TRANS_DEF_NUM:
							case LOG_MSRP_TRANS_DEF_NUM:
							case LOG_VOD_SESS_DEF_NUM:
							case START_SERVICE_DEF_NUM:
							case LOG_DIALUP_SESS_DEF_NUM:
							case LOG_DNS_DEF_NUM:
							case LOG_FTP_DEF_NUM:

							case LOG_INET_DEF_NUM:
							case LOG_ITCP_SESS_DEF_NUM:
							case LOG_IHTTP_TRANS_DEF_NUM:
							case START_PI_DATA_RECALL_NUM:
							case START_RP_DATA_RECALL_NUM:
							case START_PI_SIG_RECALL_NUM:
							case START_RP_SIG_RECALL_NUM:
							case STOP_PI_RECALL_NUM:
							case STOP_RP_RECALL_NUM:

								/**** #####  SEND LOG TO TAM_APP  ##### ********/
								if(dSendFlag) {
									dRet = dSndMsgProc(type, len, (char*)pdata);
									if( dRet < 0 ) {

										log_print( LOGN_CRI, "ERROR : FAILED IN dSndMsgProc" );

										/* If Client is SND SOCKET ERROR, re-connect server */
										g_dLogInStatus = DEF_LOGOUT;
										close(g_dServerSocket);
									}
								}

								break;
							case st_TraceMsgHdr_DEF_NUM: 	/* TRACE INFO */
								pTRACEHDR = (st_TraceMsgHdr *)pdata;
								TraceHdrLen = len;
								break;
							case ETH_DATA_NUM:
								pPACKET = pdata;
								PacketLen = len;
								break;
							case LOG_ONLINE_TRANS_DEF_NUM:
							case LOG_IV_DEF_NUM:
							case LOG_JNC_TRANS_DEF_NUM:
							case LOG_CALL_TRANS_DEF_NUM:
							case TCP_INFO_DEF_NUM:
							case HTTP_REQ_HDR_NUM:
							case HTTP_REQ_BODY_NUM:
							case HTTP_RES_HDR_NUM:
							case HTTP_RES_BODY_NUM:
							case CAP_HEADER_NUM:
							case INFO_ETH_NUM:
								/* No Transfer LOG Type */
								break;
							default:
								log_print(LOGN_CRI, "NOT SUPPORT LOG TYPE=[%d]", type);
								break;
						}

						/* THIS NODE IS TRACE INFOMATION */
						if( pTRACEHDR && pPACKET ) {
							dRet = dSndTraceMsgProc((char *)pTRACEHDR, (char *)pPACKET, TraceHdrLen, PacketLen);
							if( dRet < 0 ) {
								log_print( LOGN_CRI, "ERROR : FAILED IN dSndTraceMsgProc" );

								/* If Client is SND SOCKET ERROR, re-connect server */
								g_dLogInStatus = DEF_LOGOUT;
								close(g_dServerSocket);
							}
							pTRACEHDR = NULL;
							pPACKET = NULL;
						}

						if(ismalloc == DEF_READ_MALLOC){ free(pdata); }
					}
					pNextNode = (U8 *)nifo_entry(nifo_ptr(pMEMSINFO, ((NIFO *)pNextNode)->nont.offset_next), NIFO, nont);

				} while(pNode != pNextNode);
	
				/* DELETE ALL NODE */
				nifo_node_delete(pMEMSINFO, pNode);
			} else {
				usleep(0);
				break;
			}

		} /* for loop */

		/***  #### Event Check And Process #### *****/
		if( dCheckNProc_Event() < 0 )
			log_print( LOGN_WARN, "dProc_Msg : Failed in dCheckNProc_Event()" );

	} /* WHILE() */

	FinishProgram();

	return 0;
}

/*
 * $Log: ci_log_main.c,v $
 * Revision 1.2  2011/09/06 12:46:38  hhbaek
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * NEW OAM SYSTEM
 *
 * Revision 1.2  2011/08/21 09:07:54  hhbaek
 * Commit TAF/SRC/ *
 *
 * Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 * init DQMS2
 *
 * Revision 1.20  2011/04/21 08:21:25  dark264sh
 * CI_LOG: PrintTYPE 함수 추가
 *
 * Revision 1.19  2011/04/16 13:29:07  dark264sh
 * CI_LOG: 착신, 인터넷 감시 관련 처리
 *
 * Revision 1.18  2011/01/11 04:09:12  uamyd
 * modified
 *
 * Revision 1.1.1.1  2010/08/23 01:12:58  uamyd
 * DQMS With TOTMON, 2nd-import
 *
 * Revision 1.17  2009/08/17 15:21:27  pkg
 * *** empty log message ***
 *
 * Revision 1.16  2009/08/04 12:08:17  dqms
 * TIMER를 공유메모리로 변경
 *
 * Revision 1.15  2009/07/29 15:23:53  jsyoon
 * *** empty log message ***
 *
 * Revision 1.14  2009/07/29 15:00:34  jsyoon
 * *** empty log message ***
 *
 * Revision 1.13  2009/07/27 05:32:22  jsyoon
 * ADD CALL_STOP_NUM
 *
 * Revision 1.12  2009/07/22 06:25:21  dqms
 * *** empty log message ***
 *
 * Revision 1.11  2009/07/19 12:04:13  dqms
 * 타이머 업데이트 및 콜스탑 메세지 처리
 *
 * Revision 1.10  2009/07/15 17:10:56  dqms
 * set_version 위치 및 Plastform Type 변경
 *
 * Revision 1.9  2009/07/08 12:54:32  dqms
 * ADD LOG_PISIGNAL_DEF_NUM
 *
 * Revision 1.8  2009/07/08 12:14:19  dqms
 * Add TRACE INFO
 *
 * Revision 1.7  2009/07/01 11:33:47  dqms
 * Checkng Socket Closed
 *
 * Revision 1.6  2009/06/29 11:15:15  dqms
 * *** empty log message ***
 *
 * Revision 1.5  2009/06/28 12:57:45  dqms
 * ADD set_version
 *
 * Revision 1.4  2009/06/28 08:51:27  dqms
 * ADD START_CALL_NUM
 *
 * Revision 1.3  2009/06/25 17:41:12  jsyoon
 * *** empty log message ***
 *
 */
