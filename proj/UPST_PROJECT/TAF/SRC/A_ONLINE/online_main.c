/**		@file	online_main.c
 * 		- HTTP Transaction을 관리 하는 프로세스
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: online_main.c,v 1.2 2011/09/05 08:20:23 dhkim Exp $
 *
 * 		@Author		$Author: dhkim $
 * 		@version	$Revision: 1.2 $
 * 		@date		$Date: 2011/09/05 08:20:23 $
 * 		@warning	.
 * 		@ref		online_main.c aqua.h online_init.c online_func.c
 * 		@todo		library가 만들어지지 않은 상태, library 생성 이후 함수 대치
 *
 * 		@section	Intro(소개)
 * 		- HTTP Transaction을 관리 하는 프로세스
 *
 * 		@section	Requirement
 * 		 @li library 생성 이후 함수 대치
 *
 **/

// LIB
#include "typedef.h"
#include "loglib.h"
#include "verlib.h"
#include "mems.h"
#include "memg.h"
#include "nifo.h"
#include "gifo.h"
#include "cifo.h"
#include "hasho.h"

// PROJECT
#include "path.h"
#include "procid.h"
#include "sshmid.h"

// .
#include "online_init.h"
#include "online_func.h"


/**
 * Declare variables
 */
S32             giFinishSignal;
S32             giStopFlag;

stMEMSINFO      *pMEMSINFO;
stCIFO          *gpCIFO;
stHASHOINFO     *pHASH;

S32             gACALLCnt = 0;

/* FOR DEBUG */
S64             curSessCnt;
S64             sessCnt;
S64             rcvNodeCnt;
S64             diffSeqCnt;

/** main function.
 *
 *  man Function
 *
 *  @param	argc	:	파라미터 개수
 *  @param	*argv[]	:	파라미터
 *
 *  @return			S32
 *  @see			online_main.c aqua.h online_init.c online_func.c
 *
 *  @exception		.
 *  @note			.
 **/
S32 main(S32 argc, S8 *argv[])
{
	S32				dRet;		/**< 함수 Return 값 */
	OFFSET			offset;
	U8				*pNode;
	U8				*pDATA;
	TCP_INFO		*pTCPINFO;
	time_t			oldTime, nowTime;

    char    vERSION[7] = "R3.0.0";


	/* log_print 초기화 */
	log_init(S_SSHM_LOG_LEVEL, getpid(), SEQ_PROC_A_ONLINE, LOG_PATH"/A_ONLINE", "A_ONLINE");

	/* A_HTTP 초기화 */
	if((dRet = dInitOnline(&pMEMSINFO, &pHASH)) < 0)
	{
		log_print(LOGN_CRI, "[%s.%d] dInitOnline dRet[%d]", __FUNCTION__, __LINE__, dRet);
		exit(0);
	}

    if((dRet = set_version(S_SSHM_VERSION, SEQ_PROC_A_ONLINE, vERSION)) < 0 ) {
        log_print(LOGN_CRI, "SET_VERSION ERROR(RET=%d,IDX=%d,VER=%s)", dRet, SEQ_PROC_A_ONLINE, vERSION);
    }
	log_print(LOGN_CRI, "START ONLINE(%s) TCP_SESS_CNT[%d]", vERSION, TCP_SESS_CNT);

	/* MAIN LOOP */
	while(giStopFlag)
	{
		if((offset = gifo_read(pMEMSINFO, gpCIFO, SEQ_PROC_A_ONLINE)) > 0 ){
			pNode = nifo_ptr(pMEMSINFO, offset);
			pTCPINFO = (TCP_INFO *)nifo_get_value(pMEMSINFO, TCP_INFO_DEF_NUM, offset);
			pDATA = (U8 *)nifo_get_value(pMEMSINFO, ETH_DATA_NUM, offset);
			if(pTCPINFO == NULL) {
				log_print(LOGN_CRI, "[%s][%s.%d] TCP_INFO NULL", __FILE__, __FUNCTION__, __LINE__);
				nifo_node_delete(pMEMSINFO, pNode);
			} else {
				if(dProcOnlineTrans(pMEMSINFO, pHASH, pTCPINFO, pNode, pDATA) < 0) {
					nifo_node_delete(pMEMSINFO, pNode);
				}	
			}
		} else {
			usleep(0);
		}

		nowTime = time(NULL);
		if(nowTime >= oldTime + 60) {
			oldTime = nowTime;
			log_print(LOGN_CRI, "CUR_SECC_CNT[%lld] SESS_CNT[%lld] RCVNODE_CNT[%lld] DIFFSEQ_CNT[%lld]", 
				curSessCnt, sessCnt, rcvNodeCnt, diffSeqCnt);
			rcvNodeCnt = 0;
			diffSeqCnt = 0;
			sessCnt = 0;
		}
	}

	FinishProgram();

	return 0;
}

/*
 *  $Log: online_main.c,v $
 *  Revision 1.2  2011/09/05 08:20:23  dhkim
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *  NEW OAM SYSTEM
 *
 *  Revision 1.3  2011/08/17 13:06:40  hhbaek
 *  A_ONLINE
 *
 *  Revision 1.2  2011/08/09 08:17:40  uamyd
 *  add blocks
 *
 *  Revision 1.1.1.1  2011/08/05 00:27:18  uamyd
 *  init DQMS2
 *
 *  Revision 1.6  2011/05/09 14:00:03  dark264sh
 *  A_ONLINE: A_CALL multi 처리
 *
 *  Revision 1.5  2011/01/11 04:09:09  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2010/08/23 01:12:58  uamyd
 *  DQMS With TOTMON, 2nd-import
 *
 *  Revision 1.4  2009/08/06 06:56:09  dqms
 *  로그레벨 공유메모리로 수정
 *
 *  Revision 1.3  2009/07/15 16:44:34  dqms
 *  *** empty log message ***
 *
 *  Revision 1.2  2009/06/28 12:57:45  dqms
 *  ADD set_version
 *
 *  Revision 1.1.1.1  2009/05/26 02:14:41  dqms
 *  Init TAF_RPPI
 *
 *  Revision 1.3  2008/11/24 12:45:37  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.2  2008/09/18 07:46:02  dark264sh
 *  IM 서비스 추가 (SIP, XCAP, MSRP)
 *
 *  Revision 1.1.1.1  2008/06/09 08:17:16  jsyoon
 *  WATAS3 PROJECT START
 *
 *  Revision 1.1  2007/08/21 12:53:54  dark264sh
 *  no message
 *
 *  Revision 1.6  2007/04/18 05:29:27  dark264sh
 *  20070409 적용 TCP_SESS_CNT 변경
 *
 *  Revision 1.5  2006/11/28 15:33:39  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.4  2006/11/28 12:16:18  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.3  2006/11/02 07:21:17  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.2  2006/11/01 09:25:20  dark264sh
 *  SESS, SEQ, NODE 개수 LOG추가
 *
 *  Revision 1.1  2006/10/27 12:35:51  dark264sh
 *  *** empty log message ***
 *
 */
