/**		@file	msrpm_main.c
 * 		- MSRP Transaction을 관리 하는 프로세스
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: msrpm_main.c,v 1.2 2011/09/05 05:43:37 uamyd Exp $
 *
 * 		@Author		$Author: uamyd $
 * 		@version	$Revision: 1.2 $
 * 		@date		$Date: 2011/09/05 05:43:37 $
 * 		@warning	.
 * 		@ref		msrpm_main.c
 * 		@todo		Nothing
 *
 * 		@section	Intro(소개)
 * 		- MSRP Transaction을 관리 하는 프로세스
 *
 * 		@section	Requirement
 * 		 @li library 생성 이후 함수 대치
 *
 **/

#include <stdio.h>
#include <unistd.h>

#include "loglib.h"
#include "verlib.h"
#include "mems.h"
#include "cifo.h"
#include "gifo.h"
#include "nifo.h"

#include "path.h"
#include "commdef.h"
#include "procid.h"
#include "sshmid.h"
#include "common_stg.h"

#include "msrpm_init.h"
#include "msrpm_func.h"

S32				giFinishSignal;		/**< Finish Signal */
S32				giStopFlag;			/**< main loop Flag 0: Stop, 1: Loop */

stMEMSINFO		*pstMEMSINFO;			/**< new interface 관리 구조체 */
stCIFO			*gpCIFO;
stHASHOINFO		*pstMSRPMHASH;			/**< MSRPM Hash Table 관리 구조체 */

/** main function.
 *
 *  man Function
 *
 *  @param	argc	:	파라미터 개수
 *  @param	*argv[]	:	파라미터
 *
 *  @return			S32
 *  @see			msrpm_main.c
 *
 *  @exception		.
 *  @note			.
 **/
S32 main(S32 argc, S8 *argv[])
{
	S32				dRet;		/**< 함수 Return 값 */
	OFFSET			offset;
	U8				*pNode;
	TCP_INFO		*pstTCPINFO;
	U8				*pDATA;

    char    vERSION[7] = "R3.0.0";


	/* log_print 초기화 */
	log_init(S_SSHM_LOG_LEVEL, getpid(), SEQ_PROC_A_MSRPM, LOG_PATH"/A_MSRPM", "A_MSRPM");

	/* A_HTTP 초기화 */
	if((dRet = dInitMSRPM(&pstMEMSINFO, &pstMSRPMHASH)) < 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d dInitMSRPM dRet=%d", __FILE__, __FUNCTION__, __LINE__, dRet);
		exit(0);
	}

    if((dRet = set_version(S_SSHM_VERSION, SEQ_PROC_A_MSRPM, vERSION)) < 0 ) {
        log_print(LOGN_CRI, "SET_VERSION ERROR(RET=%d,IDX=%d,VER=%s)", dRet, SEQ_PROC_A_MSRPM, vERSION);
    }
	log_print(LOGN_CRI, "START MSRPM(%s)", vERSION);

	/* MAIN LOOP */
	while(giStopFlag)
	{
		if((offset = gifo_read(pstMEMSINFO, gpCIFO, SEQ_PROC_A_MSRPM)) > 0) {
			pNode = nifo_ptr(pstMEMSINFO, offset);
			pstTCPINFO = (TCP_INFO *)nifo_get_value(pstMEMSINFO, TCP_INFO_DEF_NUM, offset);
			pDATA = (U8 *)nifo_get_value(pstMEMSINFO, ETH_DATA_NUM, offset);
			if(pstTCPINFO == NULL) {
				log_print(LOGN_CRI, "F=%s:%s.%d TCP_INFO NULL", __FILE__, __FUNCTION__, __LINE__);
			} else {
				if((dRet = dProcMSRPM(pstMEMSINFO, pstMSRPMHASH, pstTCPINFO, pDATA)) < 0) {
					log_print(LOGN_CRI, "F=%s:%s.%d dProcMSRPM dRet=%d", __FILE__, __FUNCTION__, __LINE__, dRet);
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

/*
 *  $Log: msrpm_main.c,v $
 *  Revision 1.2  2011/09/05 05:43:37  uamyd
 *  MSRPM modified
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *  NEW OAM SYSTEM
 *
 *  Revision 1.4  2011/08/21 09:07:52  hhbaek
 *  Commit TAF/SRC
 *
 *  Revision 1.3  2011/08/17 12:12:18  dcham
 *  *** empty log message ***
 *
 *  Revision 1.2  2011/08/08 11:05:43  uamyd
 *  modified block added
 *
 *  Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 *  init DQMS2
 *
 *  Revision 1.5  2011/01/11 04:09:08  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2010/08/23 01:12:58  uamyd
 *  DQMS With TOTMON, 2nd-import
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
 *  Revision 1.1  2008/09/18 06:35:03  dark264sh
 *  IM 서비스 추가 (SIP, XCAP, MSRP)
 *
 *  Revision 1.1.1.1  2007/12/27 08:17:40  uamyd
 *  import
 *
 *  Revision 1.1.1.1  2007/10/22 07:50:42  jsyoon
 *  AQUA3 Project = AQUA2 + IMS Protocol + Packet Dump
 *
 *  Revision 1.1  2007/05/07 01:46:17  dark264sh
 *  INIT
 *
 */
