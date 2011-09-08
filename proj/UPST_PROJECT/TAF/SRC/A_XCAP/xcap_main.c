/**		@file	xcap_main.c
 *      - XCAP Service Processing
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: xcap_main.c,v 1.2 2011/09/06 12:46:42 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.2 $
 * 		@date		$Date: 2011/09/06 12:46:42 $
 * 		@warning	.
 * 		@ref		xcap_main.c xcap_init.c xcap_msgq.c
 * 		@todo		Nothing
 *
 * 		@section	Intro(소개)
 *      - XCAP Service Processing
 *
 * 		@section	Requirement
 * 		 @li Nothing
 *
 **/

/**
 *	Include headers
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

// TAF
#include "debug.h"
#include "xcap.h"

// .
#include "xcap_init.h"
#include "xcap_msgq.h"
#include "xcap_util.h"

/**
 *	Declare var.
 */
S32				giFinishSignal;		/**< Finish Signal */
S32				giStopFlag;			/**< main loop Flag 0: Stop, 1: Loop */

stMEMSINFO		*pMEMSINFO;			/**< new interface 관리 구조체 */
stCIFO			*gpCIFO;

S32				dMyQID;

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
 *  @see			xcap_main.c xcap_init.c xcap_func.c
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
	U8					*pDATA;
	LOG_HTTP_TRANS		*pLOG;

    char    vERSION[7] = "R3.0.0";


	/* log_print 초기화 */
	log_init(S_SSHM_LOG_LEVEL, getpid(), SEQ_PROC_A_XCAP, LOG_PATH"/A_XCAP", "A_XCAP");

	/* A_MEKUN 초기화 */
	if((dRet = dInitXCAP(&pMEMSINFO)) < 0)
	{
		log_print(LOGN_CRI, LH"dInitXCAP dRet[%d]", LT, dRet);
		exit(0);
	}

    if((dRet = set_version(S_SSHM_VERSION, SEQ_PROC_A_XCAP, vERSION)) < 0 ) {
        log_print(LOGN_CRI, "SET_VERSION ERROR(RET=%d,IDX=%d,VER=%s)", dRet, SEQ_PROC_A_XCAP, vERSION);
    }
	log_print(LOGN_CRI, "START XCAP(%s)", vERSION);

	nowtime = time(NULL);
	/* MAIN LOOP */
	while(giStopFlag)
	{
		if((offset = gifo_read(pMEMSINFO, gpCIFO, SEQ_PROC_A_XCAP)) > 0) {

			log_print(LOGN_INFO, "======================================================================");

			/* DB LOG 전송을 목적으로 하는 NODE (삭제 하지 않고 전송하기 위함 )*/
			pNode = nifo_ptr(pMEMSINFO, offset);
			pNextNode = pNode;

			pDATA = NULL;
			pLOG = NULL;

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
						pLOG = (LOG_HTTP_TRANS *)data;
						xcap_min((char*)pLOG->szLOGURL, pLOG->usLOGURLSize, (char*)pLOG->szMIN);
						log_print(LOGN_INFO, "LOG_HTTP_TRANS TYPE[%d]", type);
//						LOG_HTTP_TRANS_Prt("BEFORE LOG_HTTP_TRANS", pLOG);
						break;
					case HTTP_REQ_HDR_NUM:
						if(pLOG != NULL) {
							pDATA = data;
							log_print(LOGN_INFO, "DATA[\n%.*s]", len, pDATA);
						} else {
							log_print(LOGN_CRI, "RCV HTTP_REQ_HDR BUT NOT RCV LOG");
						}
						break;
					case HTTP_RES_HDR_NUM:
						if(pLOG != NULL) {
							pDATA = data;
							log_print(LOGN_INFO, "DATA[\n%.*s]", len, pDATA);
						} else {
							log_print(LOGN_CRI, "RCV HTTP_RES_HDR BUT NOT RCV LOG");
						}
						break;
					case HTTP_REQ_BODY_NUM:
					case HTTP_RES_BODY_NUM:
						if(pLOG != NULL) {
							pDATA = data;
							log_print(LOGN_INFO, "DATA[\n%.*s]", len, pDATA);
						} else {
							log_print(LOGN_CRI, "RCV DATA BUT NOT RCV LOG");
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

			if(pLOG != NULL) {
				log_print(LOGN_INFO, "SEND DATA");
				pLOGNODE = pNode;
				pNEXT = (U8 *)nifo_entry(nifo_ptr(pMEMSINFO, ((NIFO *)pNode)->nont.offset_next), NIFO, nont);
				nifo_node_unlink_nont(pMEMSINFO, pLOGNODE);
				dSend_XCAP_Data(pMEMSINFO, SEQ_PROC_A_IM, pLOGNODE);
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
 *  $Log: xcap_main.c,v $
 *  Revision 1.2  2011/09/06 12:46:42  hhbaek
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *  NEW OAM SYSTEM
 *
 *  Revision 1.4  2011/08/21 09:07:53  hhbaek
 *  Commit TAF/SRC/ *
 *
 *  Revision 1.3  2011/08/17 07:28:12  dcham
 *  *** empty log message ***
 *
 *  Revision 1.2  2011/08/08 11:05:44  uamyd
 *  modified block added
 *
 *  Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 *  init DQMS2
 *
 *  Revision 1.9  2011/05/09 14:26:11  dark264sh
 *  A_XCAP: A_CALL multi 처리
 *
 *  Revision 1.8  2011/01/11 04:09:11  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2010/08/23 01:13:01  uamyd
 *  DQMS With TOTMON, 2nd-import
 *
 *  Revision 1.7  2009/08/19 12:32:18  pkg
 *  LOG_XXX_Prt 함수 주석 처리
 *
 *  Revision 1.6  2009/08/06 06:56:09  dqms
 *  로그레벨 공유메모리로 수정
 *
 *  Revision 1.5  2009/07/15 17:10:56  dqms
 *  set_version 위치 및 Plastform Type 변경
 *
 *  Revision 1.4  2009/06/28 12:57:45  dqms
 *  ADD set_version
 *
 *  Revision 1.3  2009/06/13 11:39:46  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.2  2009/06/12 15:53:55  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2009/05/26 02:14:30  dqms
 *  Init TAF_RPPI
 *
 *  Revision 1.2  2008/12/14 09:45:54  dark264sh
 *  LOG_XCAP MIN 정보 parsing 규칙 변경
 *
 *  Revision 1.1  2008/09/18 07:24:37  dark264sh
 *  IM 서비스 추가 (SIP, XCAP, MSRP)
 *
 *  Revision 1.1.1.1  2007/12/27 08:17:38  uamyd
 *  import
 *
 *  Revision 1.1.1.1  2007/10/22 07:50:44  jsyoon
 *  AQUA3 Project = AQUA2 + IMS Protocol + Packet Dump
 *
 *  Revision 1.2  2007/03/14 09:27:46  dark264sh
 *  remove prefix
 *
 *  Revision 1.1  2007/03/07 10:33:41  dark264sh
 *  *** empty log message ***
 *
 *
 */



