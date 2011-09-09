/**		@file	sipt_main.c
 * 		- SIP Transaction을 관리 하는 프로세스
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: sipt_main.c,v 1.2 2011/09/06 12:46:39 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.2 $
 * 		@date		$Date: 2011/09/06 12:46:39 $
 * 		@warning	.
 * 		@ref		sipt_main.c sipt_init.c sipt_msgq.c
 * 		@todo		Nothing
 *
 * 		@section	Intro(소개)
 * 		- SIP Transaction을 관리 하는 프로세스
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

// TOP
#include "common_stg.h"
#include "path.h"
#include "commdef.h"
#include "procid.h"
#include "filter.h"
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

// ./SIGCOMP_INC
#include "sigcomp.h"
#include "udvm.h"

// .
#include "sipt_sess.h"
#include "sipt_init.h"
#include "sipt_util.h"
#include "sipt_msgq.h"

/**
 * Declare variables
 */
S32				giFinishSignal;		/**< Finish Signal */
S32				giStopFlag;			/**< main loop Flag 0: Stop, 1: Loop */

S64				total_size;
S64				total_cnt;
S64				snd_cnt;

st_Flt_Info 	*flt_info;
int 		    guiTimerValue;

stMEMSINFO		*pMEMSINFO;			/**< new interface 관리 구조체 */
stCIFO			*gpCIFO;
stHASHOINFO		*pSIPHASH;
stTIMERNINFO 	*pTIMERNINFO;		/**< timerN 관리 구조체 */

stHASHOINFO		*pSESSKEYINFO;      	/* SIP TRANSACTION 정리를 위한 HASH */
stHASHONODE		*pSESSKEYNODE;

/**
 *	Declare functions
 */
int Delete_SessList(SIP_INFO_KEY *pSIPINFOKEY);

/**
 *	Implement func.
 */

/** main function.
 *
 *  man Function
 *
 *  @param	argc	:	파라미터 개수
 *  @param	*argv[]	:	파라미터
 *
 *  @return			S32
 *  @see			sipt_main.c sipt_init.c sipt_func.c
 *
 *  @exception		.
 *  @note			.
 **/
S32 main(S32 argc, S8 *argv[])
{
	S32					dRet;		/**< 함수 Return 값 */
	OFFSET				offset;
	U8					*pNode, *pSIGNODE, *pDEL;
	U8					*pNextNode;
	U8					*p, *data;
	S32					type, len, ismalloc;
	time_t				nowtime, oldtime;
	TEXT_INFO			*pTEXTINFO;
	U8					*pDATA, *pSIGDATA;
	TLV					*pSIGTLV;
	int 				isETC;
    char    			vERSION[7] = "R3.0.0";

	CALL_KEY            *pCALLKey;
	SESSKEY_LIST        *pstSESSKEYList;
	int                 LastLogTime;


	/* log_print 초기화 */
	log_init(S_SSHM_LOG_LEVEL, getpid(), SEQ_PROC_A_SIPT, LOG_PATH"/A_SIPT", "A_SIPT");
	
	/* A_MEKUN 초기화 */
	if((dRet = dInitSIPT(&pMEMSINFO, &pSIPHASH, &pTIMERNINFO)) < 0)
	{
		log_print(LOGN_CRI, LH"dInitSIPT dRet[%d]", LT, dRet);
		exit(0);
	}

	sigcomp_init_protocol();

    if((dRet = set_version(S_SSHM_VERSION, SEQ_PROC_A_SIPT, vERSION)) < 0 ) {
        log_print(LOGN_CRI, "SET_VERSION ERROR(RET=%d,IDX=%d,VER=%s)", dRet, SEQ_PROC_A_SIPT, vERSION);
    }
	guiTimerValue = flt_info->stTimerInfo.usTimerInfo[PI_SIP_TIMEOUT];
	log_print(LOGN_CRI, "START SIPT VER[%s] SESSCNT[%d] TIMEOUT[%d]", vERSION, SIP_INFO_CNT, guiTimerValue);

	total_size = 0;
	total_cnt = 0;

	/* MAIN LOOP */
	while(giStopFlag)
	{
		timerN_invoke(pTIMERNINFO);

		nowtime = time(NULL);
		if(nowtime > oldtime) {
			oldtime = nowtime;
		}

		if((offset = gifo_read(pMEMSINFO, gpCIFO, SEQ_PROC_A_SIPT)) > 0) {

			log_print(LOGN_INFO, "======================================================================");

			/* DB LOG 전송을 목적으로 하는 NODE (삭제 하지 않고 전송하기 위함 )*/
			pNode = nifo_ptr(pMEMSINFO, offset);
			pNextNode = pNode;

			pTEXTINFO = NULL;
			pCALLKey = NULL;
			pDATA = NULL;
			isETC = 0;

			do {
				p = pNextNode;

				while(p != NULL) {
					if((dRet = nifo_read_tlv_cont(pMEMSINFO, pNextNode, (U32*)&type, (U32*)&len, &data, &ismalloc, &p)) < 0)
						break;

					log_print(LOGN_INFO, "####################################################################");
					log_print(LOGN_INFO, "TYPE[%d][%s] LEN[%d] ISMALLOC[%s]", type, PrintTYPE(type), len, 
							(ismalloc == DEF_READ_MALLOC) ? "MALLOC MEM" : "ORIGIN MEM");

					switch(type)
					{
					case CLEAR_CALL_NUM:
						pCALLKey = (CALL_KEY *)data;
						break;
					case ETH_DATA_NUM:
						if(pTEXTINFO) {
							pDATA = &data[pTEXTINFO->offset];
							log_print(LOGN_INFO, "PACKET TYPE[%d]", type);
						}
						break;
					case TEXT_INFO_DEF_NUM:
						pTEXTINFO = (TEXT_INFO *)data;
					
						log_print(LOGN_INFO, "TEXT_INFO TYPE[%d]", type);
						log_print(LOGN_INFO, "TEXT_INFO OFFSET[%ld]LEN[%d]S[%u.%u]U[%u.%u]A[%u.%u]", 
							pTEXTINFO->offset, pTEXTINFO->len, pTEXTINFO->uiStartTime, pTEXTINFO->uiStartMTime,
							pTEXTINFO->uiLastUpdateTime, pTEXTINFO->uiLastUpdateMTime, pTEXTINFO->uiAckTime, 
							pTEXTINFO->uiAckMTime);
						break;
					/* Ignore Case */
					case CAP_HEADER_NUM:
						break;
					case INFO_ETH_NUM:
						if( ((INFO_ETH *)data)->usAppCode == SEQ_PROC_A_VT ) {
							isETC = 1;
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
			
			if(pCALLKey) {
				log_print(LOGN_DEBUG, "#### RECEIVE CALL CLEAR MSG IP: %d.%d.%d.%d LASTLOGTIME: %u ",
						HIPADDR(pCALLKey->uiSrcIP), pCALLKey->uiReserved);
				LastLogTime = pCALLKey->uiReserved;
				pCALLKey->uiReserved = 0;
				if( (pSESSKEYNODE = hasho_find(pSESSKEYINFO, (U8 *)pCALLKey)) != NULL ) {
					log_print(LOGN_INFO, "#### FIND SIP TRANSACTION LIST IP: %d.%d.%d.%d ", HIPADDR(pCALLKey->uiSrcIP));

					pstSESSKEYList = (SESSKEY_LIST *)nifo_ptr(pSESSKEYINFO, pSESSKEYNODE->offset_Data);
					dRet = dDelSessKeyList(pstSESSKEYList, pSIPHASH, pMEMSINFO, LastLogTime);
					if(dRet == 0) {
						hasho_del(pSESSKEYINFO, (U8 *)pCALLKey);
					}
				} else {
					log_print(LOGN_INFO, "#### NOT EXIST CALL IP: %d.%d.%d.%d ", HIPADDR(pCALLKey->uiSrcIP));
				}
				nifo_node_delete(pMEMSINFO, pNode);
				continue;
			}

			if((pTEXTINFO != NULL) && (pDATA != NULL)) {

				total_size += pTEXTINFO->len;
				total_cnt++;

				if((pDATA[0] & 0xf8) != 0xf8) {
				/* normal sip message */
					log_print(LOGN_INFO, "RCV NORMAL SIP LEN=%d DATA=\n%.*s", pTEXTINFO->len, pTEXTINFO->len, pDATA);
				} else {
				/* sigcomp sip message */

					/* alloc node for decompressing sigcomp message */
					if((pSIGNODE = nifo_node_alloc(pMEMSINFO)) == NULL) {
						log_print(LOGN_CRI, LH"nifo_node_alloc NULL", LT);
						nifo_node_delete(pMEMSINFO, pNode);
						continue;
					}	

					if((pSIGDATA = nifo_tlv_alloc(pMEMSINFO, pSIGNODE, ETH_DATA_NUM, 0, DEF_MEMSET_OFF)) == NULL) {
						log_print(LOGN_CRI, LH"nifo_tlv_alloc NULL", LT);
						nifo_node_delete(pMEMSINFO, pNode);
						nifo_node_delete(pMEMSINFO, pSIGNODE);
						continue;
					}

					pSIGTLV = (TLV *)nifo_ptr(pMEMSINFO, nifo_offset(pMEMSINFO, pSIGNODE) + NIFO_SIZE);

					/* decompress sigcomp message */
					if((dRet = decomp_sig((char*)pDATA, pTEXTINFO->len, (char*)pSIGDATA, (int*)&pSIGTLV->len)) <= 0) {
						log_print(LOGN_CRI, LH"SIGCOMP DECOMPRESS FAIL dRet=%d", LT, dRet);
//						log_hexa(pDATA, pTEXTINFO->len);
						nifo_node_delete(pMEMSINFO, pNode);
						continue;
					}

					log_print(LOGN_INFO, "RCV SIGCOMP SIP LEN=%d DATA=\n%.*s", pSIGTLV->len, pSIGTLV->len, pSIGDATA);

					/* delete compressed sigcomp message */
					pDEL = nifo_ptr(pMEMSINFO, nifo_get_offset_node(pMEMSINFO, pDATA));
					nifo_node_unlink_nont(pMEMSINFO, pDEL);
					nifo_node_delete(pMEMSINFO, pDEL);
					
					/* link decompressed sigcomp message */
					nifo_node_link_nont_next(pMEMSINFO, pNode, pSIGNODE);
					pTEXTINFO->len = pSIGTLV->len;
					pTEXTINFO->offset = nifo_offset(pSIGNODE, pSIGDATA);
					pDATA = pSIGDATA;
				}

				if(dProcSIPTrans(pMEMSINFO, pSIPHASH, pTIMERNINFO, pTEXTINFO, pDATA, pTEXTINFO->len, offset) < 0) {
					nifo_node_delete(pMEMSINFO, pNode);
				}
			} else if(isETC == 1) {
				log_print(LOGN_INFO, "BYPASS ETC TRAFFIC TO A_SIPT");
				nifo_node_unlink_nont(pMEMSINFO, pNode);
				dSend_SIPT_Data(pMEMSINFO, SEQ_PROC_A_VT, pNode);
			} else {
				nifo_node_delete(pMEMSINFO, pNode);
			}
		} else {
			usleep(0);
		}
			
	}

	FinishProgram();

	return 0;
}

void invoke_del(void *p)
{
	SIP_INFO_KEY		*pSIPINFOKEY;
	SIP_INFO			*pSIPINFO;
	stHASHONODE			*pHASHNODE;

	pSIPINFOKEY = &(((SIP_COMMON *)p)->SIPINFOKEY);

	log_print(LOGN_DEBUG, "TIMER TIMEOUT CallID=%s CSeq=%u FromTag=%s", 
			pSIPINFOKEY->CallID, pSIPINFOKEY->CSeq, pSIPINFOKEY->FromTag);

	if((pHASHNODE = hasho_find(pSIPHASH, (U8 *)pSIPINFOKEY)) != NULL) {
		pSIPINFO = (SIP_INFO *)nifo_ptr(pSIPHASH, pHASHNODE->offset_Data);
		log_print(LOGN_DEBUG, "INVOKE TIMEOUT CallID=%s CSeq=%u FromTag=%s",
				pSIPINFOKEY->CallID, pSIPINFOKEY->CSeq, pSIPINFOKEY->FromTag);
		dCloseSIPTrans(pMEMSINFO, pSIPHASH, pSIPINFOKEY, pSIPINFO);
		if(pSIPINFOKEY->ClientIP != 0) {
			Delete_SessList(pSIPINFOKEY);
		}
	} else {
		log_print(LOGN_DEBUG, "INVOKE TIMEOUT BUT NODE NULL CallID=%s CSeq=%u FromTag=%s",
				pSIPINFOKEY->CallID, pSIPINFOKEY->CSeq, pSIPINFOKEY->FromTag);
	}
	
}

int Delete_SessList(SIP_INFO_KEY *pSIPINFOKEY)
{
	int 			dRet;
	CALL_KEY		CALLKey;
	CALL_KEY		*pCALLKey = &CALLKey;
	SESSKEY_LIST	*pstSESSKEYList;

	pCALLKey->uiSrcIP = pSIPINFOKEY->ClientIP;
	pCALLKey->uiReserved = 0;

	log_print(LOGN_DEBUG, "#### DELETE SESS LIST IP: %d.%d.%d.%d ", HIPADDR(pCALLKey->uiSrcIP));

	if( (pSESSKEYNODE = hasho_find(pSESSKEYINFO, (U8 *)pCALLKey)) != NULL ) {
		log_print(LOGN_INFO, "#### FIND SIP TRANSACTION LIST IP: %d.%d.%d.%d ", HIPADDR(pCALLKey->uiSrcIP));

		pstSESSKEYList = (SESSKEY_LIST *)nifo_ptr(pSESSKEYINFO, pSESSKEYNODE->offset_Data);
		dRet = dDelSessKeyNext(pstSESSKEYList, pSIPINFOKEY);
		if(dRet == 0) {
			hasho_del(pSESSKEYINFO, (U8 *)pCALLKey);
		}
	} else {
		log_print(LOGN_INFO, "#### NOT EXIST CALL IP: %d.%d.%d.%d ", HIPADDR(pCALLKey->uiSrcIP));
	}

	return dRet;
}

/*
 *  $Log: sipt_main.c,v $
 *  Revision 1.2  2011/09/06 12:46:39  hhbaek
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *  NEW OAM SYSTEM
 *
 *  Revision 1.4  2011/08/17 13:13:55  hhbaek
 *  A_SIPT
 *
 *  Revision 1.3  2011/08/09 08:17:41  uamyd
 *  add blocks
 *
 *  Revision 1.2  2011/08/09 05:31:09  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2011/08/05 00:27:18  uamyd
 *  init DQMS2
 *
 *  Revision 1.12  2011/01/11 04:09:10  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2010/08/23 01:12:59  uamyd
 *  DQMS With TOTMON, 2nd-import
 *
 *  Revision 1.11  2009/09/01 12:18:07  jsyoon
 *  프로세스 시작할때 세션수, 타임아웃값 출력
 *
 *  Revision 1.10  2009/08/17 17:31:05  pkg
 *  CALL 세션관리 리스트 버그 수정
 *
 *  Revision 1.9  2009/08/06 06:56:09  dqms
 *  로그레벨 공유메모리로 수정
 *
 *  Revision 1.8  2009/08/04 12:08:17  dqms
 *  TIMER를 공유메모리로 변경
 *
 *  Revision 1.7  2009/07/20 05:54:19  dqms
 *  *** empty log message ***
 *
 *  Revision 1.6  2009/07/20 05:32:09  dqms
 *  ETC 트래픽 패스 변경
 *
 *  Revision 1.5  2009/07/15 16:39:16  dqms
 *  *** empty log message ***
 *
 *  Revision 1.4  2009/07/05 15:39:40  dqms
 *  *** empty log message ***
 *
 *  Revision 1.3  2009/06/28 12:57:45  dqms
 *  ADD set_version
 *
 *  Revision 1.2  2009/06/10 21:25:17  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2009/05/26 02:14:35  dqms
 *  Init TAF_RPPI
 *
 *  Revision 1.1  2008/09/18 07:21:33  dark264sh
 *  IM 서비스 추가 (SIP, XCAP, MSRP)
 *
 *  Revision 1.1.1.1  2007/12/27 08:17:40  uamyd
 *  import
 *
 *  Revision 1.1.1.1  2007/10/22 07:50:43  jsyoon
 *  AQUA3 Project = AQUA2 + IMS Protocol + Packet Dump
 *
 *  Revision 1.8  2007/06/06 15:04:39  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.7  2007/04/25 11:03:17  dark264sh
 *  NFO_ETH, Capture_Header_Msg 삭제, st_PKT_INFO 추가에 따른 변경
 *
 *  Revision 1.6  2007/03/28 14:28:21  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.5  2007/03/28 07:01:30  dark264sh
 *  A_SIPT Timeout 처리 되는 경우 EndTime 처리 못하는 문제 해결 log_print 수정
 *
 *  Revision 1.4  2007/03/24 07:49:48  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.3  2007/03/14 09:29:30  dark264sh
 *  remove prefix
 *
 *  Revision 1.2  2007/03/07 01:12:06  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.1  2007/03/05 00:37:21  dark264sh
 *  *** empty log message ***
 *
 */



