/**		@file	sipm_main.c
 * 		- SIP Transaction을 관리 하는 프로세스
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: sipm_main.c,v 1.2 2011/09/05 12:26:42 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.2 $
 * 		@date		$Date: 2011/09/05 12:26:42 $
 * 		@warning	.
 * 		@ref		sipm_main.c
 * 		@todo		Nothing
 *
 * 		@section	Intro(소개)
 * 		- SIP Transaction을 관리 하는 프로세스
 *
 * 		@section	Requirement
 * 		 @li library 생성 이후 함수 대치
 *
 **/

/**
 * Include headers
 */
#include <stdio.h>
#include <unistd.h>

// TOP
#include "commdef.h"
#include "procid.h"
#include "common_stg.h"
#include "path.h"
#include "capdef.h"
#include "sshmid.h"

// LIB
#include "mems.h"
#include "cifo.h"
#include "gifo.h"
#include "nifo.h"
#include "loglib.h"
#include "verlib.h"
#include "Analyze_Ext_Abs.h"

// .
#include "sipm_init.h"
#include "sipm_func.h"

/**
 *	Declare variables
 */
S32				giFinishSignal;			/**< Finish Signal */
S32				giStopFlag;				/**< main loop Flag 0: Stop, 1: Loop */

stMEMSINFO		*pstMEMSINFO;			/**< new interface 관리 구조체 */
stCIFO			*gpCIFO;
stHASHOINFO		*pstSIPMHASH;			/**< SIPM Hash Table 관리 구조체 */

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
 *  @see			sipm_main.c
 *
 *  @exception		.
 *  @note			.
 **/
S32 main(S32 argc, S8 *argv[])
{
	S32				dRet;		/**< 함수 Return 값 */
	OFFSET			offset;
	U8				*pNode, *pText;
	TCP_INFO		*pstTCPINFO;
	U8				*pDATA;

	Capture_Header_Msg	*pCAPHEAD;
	INFO_ETH			*pINFOETH;
	TEXT_INFO			*pTEXTINFO;
	
    char    vERSION[7] = "R3.0.0";


	/* log_print 초기화 */
	log_init(S_SSHM_LOG_LEVEL,getpid(), SEQ_PROC_A_SIPM, LOG_PATH"/A_SIPM", "A_SIPM");

	/* A_HTTP 초기화 */
	if((dRet = dInitSIPM(&pstMEMSINFO, &pstSIPMHASH)) < 0)
	{
		log_print(LOGN_CRI, LH"dInitSIPM dRet=%d", LT, dRet);
		exit(0);
	}

    if((dRet = set_version(S_SSHM_VERSION, SEQ_PROC_A_SIPM, vERSION)) < 0 ) {
        log_print(LOGN_CRI, "SET_VERSION ERROR(RET=%d,IDX=%d,VER=%s)", dRet, SEQ_PROC_A_SIPM, vERSION);
    }
	log_print(LOGN_CRI, "START SIPM(%s)", vERSION);

	/* MAIN LOOP */
	while(giStopFlag)
	{
		if((offset = gifo_read(pstMEMSINFO, gpCIFO, SEQ_PROC_A_SIPM)) > 0) {
			pNode = nifo_ptr(pstMEMSINFO, offset);
			pstTCPINFO = (TCP_INFO *)nifo_get_value(pstMEMSINFO, TCP_INFO_DEF_NUM, offset);
			pDATA = (U8 *)nifo_get_value(pstMEMSINFO, ETH_DATA_NUM, offset);
			if(pstTCPINFO == NULL) {
	
				pCAPHEAD = (Capture_Header_Msg *)nifo_get_value(pstMEMSINFO, CAP_HEADER_NUM, offset);
				pINFOETH = (INFO_ETH *)nifo_get_value(pstMEMSINFO, INFO_ETH_NUM, offset);

				if((pCAPHEAD == NULL) || (pINFOETH == NULL)) {
					nifo_node_delete(pstMEMSINFO, pNode);
					continue;
				}

				/* ETC Traffic */
				if(pINFOETH->usAppCode == SEQ_PROC_A_VT) {
					log_print(LOGN_INFO, "BYPASS ETC TRAFFIC TO A_SIPT");
					dSend_SIPM_Data(pstMEMSINFO, SEQ_PROC_A_VT, pNode);
					continue;
				}

				/* UDP 처리 */
				if((pText = nifo_node_alloc(pstMEMSINFO)) == NULL) {
					log_print(LOGN_CRI, "[%s][%s.%d] nifo_node_alloc NULL", LT);
					nifo_node_delete(pstMEMSINFO, pNode);
					continue;
				}

				if((pTEXTINFO = (TEXT_INFO *)nifo_tlv_alloc(pstMEMSINFO, pText, TEXT_INFO_DEF_NUM, TEXT_INFO_SIZE, DEF_MEMSET_ON)) == NULL) {
					log_print(LOGN_CRI, "[%s][%s.%d] nifo_tlv_alloc NULL", LT);
					nifo_node_delete(pstMEMSINFO, pNode);
					continue;
				}

				SetTextInfo(pCAPHEAD, pINFOETH, pTEXTINFO);

				nifo_node_link_nont_next(pstMEMSINFO, pText, pNode);

				dSend_SIPM_Data(pstMEMSINFO, SEQ_PROC_A_VT, pText);

			} else {
				if((dRet = dProcSIPM(pstMEMSINFO, pstSIPMHASH, pstTCPINFO, pDATA)) < 0) {
					log_print(LOGN_CRI, LH"dProcSIPM dRet=%d", LT, dRet);
				}
				nifo_node_delete(pstMEMSINFO, pNode);
			}
		} else {
			usleep(0);
		}
	}

	FinishProgram();

	return 0;
}

/*
 *  $Log: sipm_main.c,v $
 *  Revision 1.2  2011/09/05 12:26:42  hhbaek
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *  NEW OAM SYSTEM
 *
 *  Revision 1.3  2011/08/18 01:38:46  hhbaek
 *  A_SIPM
 *
 *  Revision 1.2  2011/08/09 05:31:08  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 *  init DQMS2
 *
 *  Revision 1.6  2011/01/11 04:09:10  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2010/08/23 01:13:02  uamyd
 *  DQMS With TOTMON, 2nd-import
 *
 *  Revision 1.5  2009/08/06 06:56:09  dqms
 *  로그레벨 공유메모리로 수정
 *
 *  Revision 1.4  2009/07/20 05:32:09  dqms
 *  ETC 트래픽 패스 변경
 *
 *  Revision 1.3  2009/07/15 17:10:56  dqms
 *  set_version 위치 및 Plastform Type 변경
 *
 *  Revision 1.2  2009/06/28 12:57:45  dqms
 *  ADD set_version
 *
 *  Revision 1.1.1.1  2009/05/26 02:14:35  dqms
 *  Init TAF_RPPI
 *
 *  Revision 1.2  2009/01/28 16:36:42  dark264sh
 *  A_SIPT TCP/UDP 역전 처리
 *
 *  Revision 1.1  2008/09/18 07:19:52  dark264sh
 *  IM 서비스 추가 (SIP, XCAP, MSRP)
 *
 *  Revision 1.1.1.1  2007/12/27 08:17:38  uamyd
 *  import
 *
 *  Revision 1.1.1.1  2007/10/22 07:50:44  jsyoon
 *  AQUA3 Project = AQUA2 + IMS Protocol + Packet Dump
 *
 *  Revision 1.1  2007/05/10 02:57:30  dark264sh
 *  A_SIPM (TCP Merge) 추가
 *
 */
