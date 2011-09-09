/**		@file	pciv_main.c
 * 		- PC Internet Viewer Transaction을 관리 하는 프로세스
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: pciv_main.c,v 1.2 2011/09/05 04:34:10 dhkim Exp $
 *
 * 		@Author		$Author: dhkim $
 * 		@version	$Revision: 1.2 $
 * 		@date		$Date: 2011/09/05 04:34:10 $
 * 		@warning	.
 * 		@ref		pciv_main.c pciv_init.c
 * 		@todo		library가 만들어지지 않은 상태, library 생성 이후 함수 대치
 *
 * 		@section	Intro(소개)
 * 		- PC Internet Viewer Transaction을 관리 하는 프로세스
 *
 * 		@section	Requirement
 * 		 @li library 생성 이후 함수 대치
 *
 **/
#include <unistd.h>

// LIB
#include "typedef.h"
#include "loglib.h"
#include "verlib.h"
#include "mems.h"
#include "cifo.h"
#include "gifo.h"
#include "nifo.h"

// PROJECT headers
#include "path.h"
#include "procid.h"
#include "sshmid.h"
#include "common_stg.h"

//#include <debug.h>

// .
#include "pciv_init.h"
#include "pciv_func.h"


S32				giFinishSignal;		/**< Finish Signal */
S32				giStopFlag;			/**< main loop Flag 0: Stop, 1: Loop */

stMEMSINFO		*pMEMSINFO;			/**< new interface 관리 구조체 */
stCIFO			*gpCIFO;

static stHASHOINFO	*pHASH;			/**< IV Hash Table 관리 구조체 */
struct timeval      stNowTime;

S32				dMyQID;
S32				dCallQID[MAX_SMP_NUM];

S32				gACALLCnt = 0;

/* FOR DEBUG */
S64				curSessCnt;		/* Transaction 개수 */

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
 *  @note			TCP 프로세스에서 IP/TCP 헤더 정보가 분석된 구조체와 함께, TCP payload 부분을 전달해 준다.
						
					우선은 CheckAndVersion ~ Exit 를 PCIV_Session 생명주기로 정한다.
 **/
S32 main(S32 argc, S8 *argv[])
{
	S32			dRet;		/**< 함수 Return 값 */
	OFFSET		offset;
	U8			*pNode;
	U8			*pDATA;
	TCP_INFO	*pTCPINFO;
#if DEBUG_MODE
	time_t		oldTime, nowTime;
#endif

    char    vERSION[7] = "R3.0.0";


	/* log_print 초기화 */
	log_init(S_SSHM_LOG_LEVEL, getpid(), SEQ_PROC_A_IV, LOG_PATH"/A_IV", "A_IV");

	/* A_IV 초기화 */
	curSessCnt = 0;
	if((dRet = dInitProc(&pMEMSINFO, &pHASH)) < 0)
	{
		log_print(LOGN_CRI, "[%s][%s.%d] dInitProc dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		exit(0);
	}

    if((dRet = set_version(S_SSHM_VERSION, SEQ_PROC_A_IV, vERSION)) < 0 ) {
        log_print(LOGN_CRI, "SET_VERSION ERROR(RET=%d,IDX=%d,VER=%s)", dRet, SEQ_PROC_A_IV, vERSION);
    }
	log_print(LOGN_CRI, "START A_IV(%s)", vERSION);

	/* MAIN LOOP */
	while(giStopFlag)
	{
		if((offset = gifo_read(pMEMSINFO, gpCIFO, SEQ_PROC_A_IV)) > 0) {
			pNode = nifo_ptr(pMEMSINFO, offset);
			pTCPINFO = (TCP_INFO *)nifo_get_value(pMEMSINFO, TCP_INFO_DEF_NUM, offset);
			pDATA = (U8 *)nifo_get_value(pMEMSINFO, ETH_DATA_NUM, offset);
			if(pTCPINFO == NULL) {
				log_print(LOGN_CRI, "[%s][%s.%d] TCP_INFO NULL", __FILE__, __FUNCTION__, __LINE__);
				nifo_node_delete(pMEMSINFO, pNode);
			} 
			else {
				if((dRet = dSvcProcess(pMEMSINFO, pHASH, pTCPINFO, pNode, pDATA)) < 0) {
					log_print(LOGN_DEBUG, "[%s][%s.%d] dSvcProcess dRet[%d]", __FILE__, __FUNCTION__, __LINE__,dRet);
					nifo_node_delete(pMEMSINFO, pNode);
				}	
			}
		} else {
			usleep(0);
		}
#if DEBUG_MODE
		nowTime = time(NULL);
		if(nowTime >= oldTime + 60) {
			oldTime = nowTime;
			log_print(LOGN_CRI, "CUR_SESS_CNT[%lld]",curSessCnt);
		}
#endif
	}

	FinishProgram();

	return 0;
}

/*
 *  $Log: pciv_main.c,v $
 *  Revision 1.2  2011/09/05 04:34:10  dhkim
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *  NEW OAM SYSTEM
 *
 *  Revision 1.5  2011/08/21 09:07:51  hhbaek
 *  Commit TAF/SRC/
 *
 *  Revision 1.4  2011/08/17 07:19:50  dcham
 *  *** empty log message ***
 *
 *  Revision 1.3  2011/08/17 04:25:36  dcham
 *  *** empty log message ***
 *
 *  Revision 1.2  2011/08/08 11:05:41  uamyd
 *  modified block added
 *
 *  Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 *  init DQMS2
 *
 *  Revision 1.6  2011/05/09 14:38:34  dark264sh
 *  A_IV: A_CALL multi 처리
 *
 *  Revision 1.5  2011/01/11 04:09:07  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2010/08/23 01:13:03  uamyd
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
 *  Revision 1.1.1.1  2009/05/26 02:14:38  dqms
 *  Init TAF_RPPI
 *
 *  Revision 1.5  2008/09/18 07:42:50  dark264sh
 *  IM 서비스 추가 (SIP, XCAP, MSRP)
 *
 *  Revision 1.4  2008/07/14 08:50:22  jyjung
 *  sequence number 다를 경우, 해당 Page는 에러처리하고, 그 이후 Page는 처리하도록 수정.
 *
 *  Revision 1.3  2008/07/10 08:52:36  jsyoon
 *  SESS 못찾을 경우의 로그가 너무 많아, 우선 LOG_DEBUG로 수정
 *
 *  Revision 1.2  2008/07/07 15:04:56  jyjung
 *  IsCompleted가 Page시작 위치
 *
 *  Revision 1.1  2008/06/23 04:06:41  jyjung
 *  A_IV 추가
 *
 *
 */
