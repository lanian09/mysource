/**		@file	sip_main.c
 *      - SIP Service Processing
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: sip_main.c,v 1.2 2011/09/05 12:26:41 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.2 $
 * 		@date		$Date: 2011/09/05 12:26:41 $
 * 		@warning	.
 * 		@ref		sip_main.c sip_init.c sip_msgq.c
 * 		@todo		Nothing
 *
 * 		@section	Intro(소개)
 *      - SIP Service Processing
 *
 * 		@section	Requirement
 * 		 @li Nothing
 *
 **/

/**
 * Include headers
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

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
#include "Analyze_Ext_Abs.h"

// TAF
#include "debug.h"

// .
#include "sip_init.h"
#include "sip_msgq.h"
#include "sip_util.h"

/**
 * Declare variables
 */
S32				giFinishSignal;		/**< Finish Signal */
S32				giStopFlag;			/**< main loop Flag 0: Stop, 1: Loop */

stMEMSINFO		*pMEMSINFO;			/**< new interface 관리 구조체 */
stCIFO			*gpCIFO;

S32				dMyQID;
S32				dCILOGQID;
S32				dIMQID;
S32				dVTQID;

/**
 *	Implement func.
 */

/** main function.
 *
 *  main Function
 *
 *  @param	argc	:	파라미터 개수
 *  @param	*argv[]	:	파라미터
 *
 *  @return			S32
 *  @see			sip_main.c sip_init.c sip_func.c
 *
 *  @exception		.
 *  @note			.
 **/
S32 main(S32 argc, S8 *argv[])
{
	S32					dRet;		/**< 함수 Return 값 */
	OFFSET				offset;
	U8					*pNode, *pLOGNODE, *pNEXT;
	U8					*pNextNode;
	U8					*p, *data;
	S32					type, len, ismalloc;
	time_t				nowtime;
	struct timeval		stTime;
	TEXT_INFO			*pTEXTINFO;
	U8					*pDATA;
	LOG_SIP_TRANS		*pLOG;
	INFO_ETH 			*pINFOETH;
	UINT 				dSeqProcID = 0;

    char    vERSION[7] = "R3.0.0";


	/* log_print 초기화 */
	log_init(S_SSHM_LOG_LEVEL, getpid(), SEQ_PROC_A_SIP, LOG_PATH"/A_SIP", "A_SIP");

	/* A_MEKUN 초기화 */
	if((dRet = dInitSIP(&pMEMSINFO)) < 0)
	{
		log_print(LOGN_CRI, LH"dInitSIP dRet[%d]", LT, dRet);
		exit(0);
	}

    if((dRet = set_version(S_SSHM_VERSION, SEQ_PROC_A_SIP, vERSION)) < 0 ) {
        log_print(LOGN_CRI, "SET_VERSION ERROR(RET=%d,IDX=%d,VER=%s)", dRet, SEQ_PROC_A_SIP, vERSION);
    }
	log_print(LOGN_CRI, "START SIP(%s)", vERSION);

	nowtime = time(NULL);
	/* MAIN LOOP */
	while(giStopFlag)
	{
		if((offset = gifo_read(pMEMSINFO, gpCIFO, SEQ_PROC_A_SIP)) > 0) {

			log_print(LOGN_INFO, "======================================================================");

			/* DB LOG 전송을 목적으로 하는 NODE (삭제 하지 않고 전송하기 위함 )*/
			pNode = nifo_ptr(pMEMSINFO, offset);
			pNextNode = pNode;

			pTEXTINFO = NULL;
			pDATA = NULL;
			pLOG = NULL;

			do {
				p = pNextNode;

				while(p != NULL) {
					if((dRet = nifo_read_tlv_cont(pMEMSINFO, pNextNode, &type, &len, &data, &ismalloc, &p)) < 0)
						break;

					log_print(LOGN_INFO, "####################################################################");
					log_print(LOGN_INFO, "TYPE[%d][%s] LEN[%d] ISMALLOC[%s]", type, PrintTYPE(type), len, 
							(ismalloc == DEF_READ_MALLOC) ? "MALLOC MEM" : "ORIGIN MEM");

					switch(type)
					{
					case LOG_SIP_TRANS_DEF_NUM:
						pLOG = (LOG_SIP_TRANS *)data;
						if(pINFOETH != NULL) {
							if (pLOG->usPlatformType == DEF_PLATFORM_IM)
								dSeqProcID = SEQ_PROC_A_IM;
							else
								dSeqProcID = SEQ_PROC_A_VT;
						}
						log_print(LOGN_INFO, "LOG_SIP_TRANS TYPE[%d]", type);
//						LOG_SIP_TRANS_Prt("BEFORE LOG_SIP_TRANS", pLOG);
						break;
					case ETH_DATA_NUM:
						if((pTEXTINFO != NULL) && (pLOG != NULL)){
							pDATA = &data[pTEXTINFO->offset];
							log_print(LOGN_INFO, "PACKET TYPE[%d]", type);
							log_print(LOGN_INFO, "DATA[\n%.*s]", pTEXTINFO->len, pDATA);

							if(dGetSIP(pDATA, pTEXTINFO->len, pLOG) < 0) {
								pTEXTINFO = NULL;
								pDATA = NULL;
								pLOG = NULL;
							} else {
								gettimeofday(&stTime, NULL);
								pLOG->OpEndTime = stTime.tv_sec;
								pLOG->OpEndMTime = stTime.tv_usec;
//								LOG_SIP_TRANS_Prt("AFTER LOG_SIP_TRANS", pLOG);
							}
						} else {
							log_print(LOGN_CRI, "RCV ETH_DATA BUT NOT RCV TEXT_INFO");
						}
						break;
					case TEXT_INFO_DEF_NUM:
						pTEXTINFO = (TEXT_INFO *)data;
					
						log_print(LOGN_INFO, "TEXT_INFO TYPE[%d]", type);
						log_print(LOGN_INFO, "TEXT_INFO OFFSET[%u]LEN[%d]S[%u.%u]U[%u.%u]A[%u.%u]", 
							pTEXTINFO->offset, pTEXTINFO->len, pTEXTINFO->uiStartTime, pTEXTINFO->uiStartMTime,
							pTEXTINFO->uiLastUpdateTime, pTEXTINFO->uiLastUpdateMTime, pTEXTINFO->uiAckTime, 
							pTEXTINFO->uiAckMTime);
						break;
					/* Ignore Case */
					case CAP_HEADER_NUM:
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

			if((pTEXTINFO != NULL) && (pDATA != NULL) && (pLOG != NULL)) {
				log_print(LOGN_INFO, "SEND DATA SeqProcID[%d] TO [%s]", dSeqProcID, dGetQStr(pLOG->usPlatformType));
				pLOGNODE = pNode;
				pNEXT = (U8 *)nifo_entry(nifo_ptr(pMEMSINFO, ((NIFO *)pNode)->nont.offset_next), NIFO, nont);
				nifo_node_unlink_nont(pMEMSINFO, pLOGNODE);
				/* TODO: A_VT, A_IM 구분 */
				if( dSeqProcID > 0 ) dSend_SIP_Data(pMEMSINFO, dSeqProcID, pLOGNODE);
				else				 log_print(LOGN_WARN, "SeqProcID is ZERO");
				pNode = pNEXT;
			}

			/* Node 삭제 */
			nifo_node_delete(pMEMSINFO, pNode);

		} else {
			usleep(0);
		}
			
	}

	FinishProgram();

	return 0;
}


/*
 *  $Log: sip_main.c,v $
 *  Revision 1.2  2011/09/05 12:26:41  hhbaek
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *  NEW OAM SYSTEM
 *
 *  Revision 1.4  2011/08/25 07:25:47  uamyd
 *  nifo_msg_write api or log changed to gifo_write
 *
 *  Revision 1.3  2011/08/17 13:12:03  hhbaek
 *  A_SIPD
 *
 *  Revision 1.2  2011/08/09 08:17:41  uamyd
 *  add blocks
 *
 *  Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 *  init DQMS2
 *
 *  Revision 1.11  2011/05/09 14:14:37  dark264sh
 *  A_SIPD: A_CALL multi 처리
 *
 *  Revision 1.10  2011/01/11 04:09:09  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2010/08/23 01:13:02  uamyd
 *  DQMS With TOTMON, 2nd-import
 *
 *  Revision 1.9  2009/08/22 18:37:09  jsyoon
 *  INVITE 외의 MSG는 SIPD -> A_VT -> A_IM
 *
 *  Revision 1.8  2009/08/19 12:28:10  pkg
 *  LOG_XXX_Prt 함수 주석 처리
 *
 *  Revision 1.7  2009/08/06 06:56:09  dqms
 *  로그레벨 공유메모리로 수정
 *
 *  Revision 1.6  2009/07/15 17:10:56  dqms
 *  set_version 위치 및 Plastform Type 변경
 *
 *  Revision 1.5  2009/07/05 15:39:40  dqms
 *  *** empty log message ***
 *
 *  Revision 1.4  2009/06/28 15:02:16  dqms
 *  ADD PLATFORM TYPE
 *
 *  Revision 1.3  2009/06/28 12:57:45  dqms
 *  ADD set_version
 *
 *  Revision 1.2  2009/06/13 11:39:46  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2009/05/26 02:14:37  dqms
 *  Init TAF_RPPI
 *
 *  Revision 1.4  2009/02/11 06:17:49  dark264sh
 *  SIP Key 변경 (SeqType 추가) | 구간 세분화 | 486,487,603 성공 처리
 *
 *  Revision 1.3  2008/11/27 06:01:16  dark264sh
 *  SIP Vendor 필드 임시 처리
 *
 *  Revision 1.2  2008/09/21 12:27:29  dark264sh
 *  A_SIPD ignore case 처리
 *
 *  Revision 1.1  2008/09/18 06:50:03  dark264sh
 *  IM 서비스 추가 (SIP, XCAP, MSRP)
 *
 *  Revision 1.1.1.1  2007/12/27 08:17:40  uamyd
 *  import
 *
 *  Revision 1.1.1.1  2007/10/22 07:50:43  jsyoon
 *  AQUA3 Project = AQUA2 + IMS Protocol + Packet Dump
 *
 *  Revision 1.2  2007/03/14 09:30:17  dark264sh
 *  remove prefix
 *
 *  Revision 1.1  2007/03/07 01:15:19  dark264sh
 *  *** empty log message ***
 *
 */



