/**		@file	msrp_main.c
 * 		- MSRPT Transaction을 관리 하는 프로세스
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: msrpt_main.c,v 1.4 2011/09/07 06:30:48 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.4 $
 * 		@date		$Date: 2011/09/07 06:30:48 $
 * 		@warning	.
 * 		@ref		msrp_main.c
 * 		@todo		Nothing
 *
 * 		@section	Intro(소개)
 * 		- MSRPT Transaction을 관리 하는 프로세스
 *
 * 		@section	Requirement
 * 		 @li library 생성 이후 함수 대치
 *
 **/

/**
 *	Include headers
 */
#include <stdio.h>
#include <unistd.h>

// TOP
#include "filter.h"
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
#include "utillib.h"

// .
#include "msrpt_init.h"
#include "msrpt_func.h"
#include "msrpt_util.h"

/**
 *	Declare var.
 */
S32				giFinishSignal;		/**< Finish Signal */
S32				giStopFlag;			/**< main loop Flag 0: Stop, 1: Loop */

stMEMSINFO		*pstMEMSINFO;		/**< new interface 관리 구조체 */
stCIFO			*gpCIFO;
stHASHOINFO		*pstMSRPTHASH;		/**< MSRPT Hash Table 관리 구조체 */
stTIMERNINFO 	*pstTIMERNINFO;     /**< timerN 관리 구조체 */
st_Flt_Info		*flt_info;

int          guiTimerValue;

/**
 *	Declare func.
 */

/** main function.
 *
 *  man Function
 *
 *  @param	argc	:	파라미터 개수
 *  @param	*argv[]	:	파라미터
 *
 *  @return			S32
 *  @see			msrp_main.c
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
	TEXT_INFO		*pstTEXTINFO;
	U8				*pDATA;

    char    vERSION[7] = "R3.0.0";


	/* log_print 초기화 */
	log_init(S_SSHM_LOG_LEVEL, getpid(), SEQ_PROC_A_MSRPT, LOG_PATH"/A_MSRPT", "A_MSRPT");
	
	/* A_HTTP 초기화 */
	if((dRet = dInitMSRPT(&pstMEMSINFO, &pstMSRPTHASH, &pstTIMERNINFO)) < 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d dInitMSRPT dRet=%d", __FILE__, __FUNCTION__, __LINE__, dRet);
		exit(0);
	}

    if((dRet = set_version(S_SSHM_VERSION, SEQ_PROC_A_MSRPT, vERSION)) < 0 ) {
        log_print(LOGN_CRI, "SET_VERSION ERROR(RET=%d,IDX=%d,VER=%s)", dRet, SEQ_PROC_A_MSRPT, vERSION);
    }

	guiTimerValue = flt_info->stTimerInfo.usTimerInfo[PI_MSRP_TIMEOUT];
	log_print(LOGN_CRI, "START MSRPT VER[%s] TRANSCNT[%d] TIMER[%d]", vERSION, MSRPT_TRANS_CNT, guiTimerValue);

	/* MAIN LOOP */
	while(giStopFlag)
	{
		timerN_invoke(pstTIMERNINFO);

		if((offset = gifo_read(pstMEMSINFO, gpCIFO, SEQ_PROC_A_MSRPM)) > 0) {

			log_print(LOGN_INFO, "======================================================================");

			/* DB LOG 전송을 목적으로 하는 NODE (삭제 하지 않고 전송하기 위함 )*/
			pNode = nifo_ptr(pstMEMSINFO, offset);
			pNextNode = pNode;

			pstTEXTINFO = NULL;
			pDATA = NULL;

			do {
				p = pNextNode;

				while(p != NULL) {
					if((dRet = nifo_read_tlv_cont(pstMEMSINFO, pNextNode, (U32*)&type, (U32*)&len, &data, &ismalloc, &p)) < 0)
						break;

					log_print(LOGN_INFO, "####################################################################");
					log_print(LOGN_INFO, "TYPE[%d][%s] LEN[%d] ISMALLOC[%s]", 
						type, PrintType(type), len, 
						(ismalloc == DEF_READ_MALLOC) ? "MALLOC MEM" : "ORIGIN MEM");

					switch(type)
					{
					case ETH_DATA_NUM:
						if(pstTEXTINFO != NULL) {
							pDATA = &data[pstTEXTINFO->offset];
							log_print(LOGN_INFO, "PACKET TYPE[%d]", type);
							log_print(LOGN_INFO, "DATA[%.*s]", pstTEXTINFO->len, pDATA);
						} else {
							log_print(LOGN_CRI, "RCV PACKET_DATA BUT NOT RCV TEXT_INFO");	
						}
						break;
					case TEXT_INFO_DEF_NUM:
						pstTEXTINFO = (TEXT_INFO *)data;
					
						log_print(LOGN_INFO, "TEXT_INFO TYPE[%d]", type);
						log_print(LOGN_INFO, "TEXT_INFO OFFSET[%ld]LEN[%d]S[%u.%u]U[%u.%u]A[%u.%u]", 
							pstTEXTINFO->offset, pstTEXTINFO->len, pstTEXTINFO->uiStartTime, pstTEXTINFO->uiStartMTime,
							pstTEXTINFO->uiLastUpdateTime, pstTEXTINFO->uiLastUpdateMTime, pstTEXTINFO->uiAckTime, 
							pstTEXTINFO->uiAckMTime);
						break;
					/* Ignore Case */
					default:
						log_print(LOGN_INFO, "????? UNKNOWN TYPE[%d]", type);
						break;
					}

					if(ismalloc == DEF_READ_MALLOC){ free(data); }
				}
				
				pNextNode = (U8 *)nifo_entry(nifo_ptr(pstMEMSINFO, ((NIFO *)pNextNode)->nont.offset_next), NIFO, nont);

			} while(pNode != pNextNode);

			if((pstTEXTINFO != NULL) && (pDATA != NULL)) {

				if(dProcMSRPTTrans(pstMEMSINFO, pstMSRPTHASH, pstTIMERNINFO, pstTEXTINFO, pDATA) < 0) {

				}
			}
			nifo_node_delete(pstMEMSINFO, pNode);
		} else {
			usleep(0);
		}
			
	}

	FinishProgram();

	return 0;
}

void invoke_del(void *p)
{
	st_MSRPT_TRANS_KEY		*pstMSRPTTRANSKEY;
	st_MSRPT_TRANS			*pstMSRPTTRANS;
	stHASHONODE				*pstHASHONODE;

    U8						szCIP[INET_ADDRSTRLEN];
    U8						szSIP[INET_ADDRSTRLEN];

	pstMSRPTTRANSKEY = &(((st_MSRPT_COMMON *)p)->stMSRPTTRANSKEY);

	log_print(LOGN_DEBUG, "TIMER TIMEOUT CIP=%s:%d SIP=%s:%d MSGID=%s",
				util_cvtipaddr(szCIP, pstMSRPTTRANSKEY->uiCliIP), pstMSRPTTRANSKEY->usCliPort,
				util_cvtipaddr(szSIP, pstMSRPTTRANSKEY->uiSrvIP), pstMSRPTTRANSKEY->usSrvPort,
				pstMSRPTTRANSKEY->szMSGID);	

	if((pstHASHONODE = hasho_find(pstMSRPTHASH, (U8 *)pstMSRPTTRANSKEY)) != NULL) {
		pstMSRPTTRANS = (st_MSRPT_TRANS *)nifo_ptr(pstMSRPTHASH, pstHASHONODE->offset_Data);
		log_print(LOGN_DEBUG, "INVOKE TIMEOUT CIP=%s:%d SIP=%s:%d MSGID=%s",
					util_cvtipaddr(szCIP, pstMSRPTTRANSKEY->uiCliIP), pstMSRPTTRANSKEY->usCliPort,
					util_cvtipaddr(szSIP, pstMSRPTTRANSKEY->uiSrvIP), pstMSRPTTRANSKEY->usSrvPort,
					pstMSRPTTRANSKEY->szMSGID);	
		dCloseMSRPTTrans(pstMEMSINFO, pstMSRPTHASH, pstMSRPTTRANSKEY, pstMSRPTTRANS);
	} else {
		log_print(LOGN_DEBUG, "INVOKE TIMEOUT BUT NODE NULL CIP=%s:%d SIP=%s:%d MSGID=%s",
					util_cvtipaddr(szCIP, pstMSRPTTRANSKEY->uiCliIP), pstMSRPTTRANSKEY->usCliPort,
					util_cvtipaddr(szSIP, pstMSRPTTRANSKEY->uiSrvIP), pstMSRPTTRANSKEY->usSrvPort,
					pstMSRPTTRANSKEY->szMSGID);	
	}
}

/*
 *  $Log: msrpt_main.c,v $
 *  Revision 1.4  2011/09/07 06:30:48  hhbaek
 *  *** empty log message ***
 *
 *  Revision 1.3  2011/09/05 12:26:40  hhbaek
 *  *** empty log message ***
 *
 *  Revision 1.2  2011/09/05 01:35:33  uamyd
 *  modified to runnable source
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *  NEW OAM SYSTEM
 *
 *  Revision 1.3  2011/08/21 09:07:52  hhbaek
 *  Commit TAF/SRC/ *
 *
 *  Revision 1.2  2011/08/09 05:31:08  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 *  init DQMS2
 *
 *  Revision 1.8  2011/05/09 14:19:31  dark264sh
 *  A_MSRPT: A_CALL multi 처리
 *
 *  Revision 1.7  2011/01/11 04:09:08  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2010/08/23 01:13:00  uamyd
 *  DQMS With TOTMON, 2nd-import
 *
 *  Revision 1.6  2009/09/02 13:55:18  jsyoon
 *  MSRP COUNT SETTING
 *
 *  Revision 1.5  2009/08/04 12:08:17  dqms
 *  TIMER를 공유메모리로 변경
 *
 *  Revision 1.4  2009/07/15 16:42:10  dqms
 *  ADD vMSRPTimerReConstruct()
 *
 *  Revision 1.3  2009/06/28 12:57:45  dqms
 *  ADD set_version
 *
 *  Revision 1.2  2009/06/12 15:51:47  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2009/05/26 02:14:14  dqms
 *  Init TAF_RPPI
 *
 *  Revision 1.1  2008/09/18 06:48:10  dark264sh
 *  IM 서비스 추가 (SIP, XCAP, MSRP)
 *
 *  Revision 1.1.1.1  2007/12/27 08:15:16  uamyd
 *  import
 *
 *  Revision 1.1.1.1  2007/10/22 07:50:44  jsyoon
 *  AQUA3 Project = AQUA2 + IMS Protocol + Packet Dump
 *
 *  Revision 1.2  2007/06/18 10:31:58  dark264sh
 *  timeout 처리 버그
 *
 *  Revision 1.1  2007/05/07 01:48:09  dark264sh
 *  INIT
 *
 */
