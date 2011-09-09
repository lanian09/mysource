/**		@file	http_main.c
 * 		- HTTP Transaction을 관리 하는 프로세스
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: http_main.c,v 1.3 2011/09/07 04:22:45 uamyd Exp $
 *
 * 		@Author		$Author: uamyd $
 * 		@version	$Revision: 1.3 $
 * 		@date		$Date: 2011/09/07 04:22:45 $
 * 		@warning	.
 * 		@ref		http_main.c l4.h http_init.c http_func.c
 * 		@todo		library가 만들어지지 않은 상태, library 생성 이후 함수 대치
 *
 * 		@section	Intro(소개)
 * 		- HTTP Transaction을 관리 하는 프로세스
 *
 * 		@section	Requirement
 * 		 @li library 생성 이후 함수 대치
 *
 **/

#include <stdio.h>
#include <unistd.h>

// LIB
#include "typedef.h"
#include "loglib.h"
#include "verlib.h"
#include "mems.h"
#include "cifo.h"
#include "gifo.h"
#include "nifo.h"
#include "hasho.h"

// PROJECT 
#include "path.h"
#include "procid.h"
#include "sshmid.h"
#include "common_stg.h"

// . 
#include "http_init.h"
#include "http_func.h"
#include "http_msgq.h"


S32				giFinishSignal;		/**< Finish Signal */
S32				giStopFlag;			/**< main loop Flag 0: Stop, 1: Loop */

stMEMSINFO		*pMEMSINFO;			/**< new interface 관리 구조체 */
stCIFO			*gpCIFO;
stHASHOINFO		*pTCPHASH;			/**< TCP Hash Table 관리 구조체 */
stHASHOINFO		*pHTTPHASH;			/**< HTTP Hash Table 관리 구조체 */

/* FOR DEBUG */
S64				curSessCnt;		/* Transaction 개수 */
S64				sessCnt;
S64				rcvNodeCnt;		/* 받은 NODE 개수 */
S64				diffSeqCnt;		/* DIFF SEQ가 된 개수 */

UINT 			guiSeqProcID;

/* FOR MULTIPLE PROCESS */
char            gszMyProc[32];

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
	U8				*pDATA;
	TCP_INFO		*pTCPINFO;
	time_t			oldTime, nowTime;
	int 			dLen, PROCNO;

	char 			szLOGPATH[128];

    char    		vERSION[7] = "R3.0.0";


	memcpy(&gszMyProc[0], argv[0], strlen(argv[0]));

	dLen = strlen(gszMyProc);
	PROCNO = atoi(&gszMyProc[dLen-1]);
	guiSeqProcID = SEQ_PROC_A_HTTP0 + PROCNO;

	/* log_print 초기화 */
	sprintf(szLOGPATH, LOG_PATH"/%s", gszMyProc);
	log_init(S_SSHM_LOG_LEVEL, getpid(), guiSeqProcID, szLOGPATH, gszMyProc);

	/* A_HTTP 초기화 */
	if((dRet = dInitHttp(&pMEMSINFO, &pTCPHASH, &pHTTPHASH)) < 0)
	{
		log_print(LOGN_CRI, "[%s.%d] dInitHttp dRet[%d]", __FUNCTION__, __LINE__, dRet);
		exit(0);
	}

    if((dRet = set_version(S_SSHM_VERSION, guiSeqProcID, vERSION)) < 0 ) {
        log_print(LOGN_CRI, "SET_VERSION ERROR(RET=%d,IDX=%d,VER=%s)", dRet, guiSeqProcID, vERSION);
    }
	log_print(LOGN_CRI, "START HTTP[%d](%s) TCP_SESS_CNT[%d] HTTP_TRANS_CNT[%d]", PROCNO, vERSION, TCP_SESS_CNT, HTTP_TRANS_CNT);

	/* MAIN LOOP */
	while(giStopFlag)
	{
		if((offset = gifo_read(pMEMSINFO, gpCIFO, guiSeqProcID)) > 0) {
			pNode = nifo_ptr(pMEMSINFO, offset);
			pTCPINFO = (TCP_INFO *)nifo_get_value(pMEMSINFO, TCP_INFO_DEF_NUM, offset);
			pDATA = (U8 *)nifo_get_value(pMEMSINFO, ETH_DATA_NUM, offset);
			if(pTCPINFO == NULL) {
				log_print(LOGN_INFO, "SEND RTP TO A_VOD:%d", SEQ_PROC_A_VOD);
				if(dSend_HTTP_Data(pMEMSINFO, SEQ_PROC_A_VOD, pNode) < 0) {
					nifo_node_delete(pMEMSINFO, pNode);
				}
			} else {
				if(dProcHttpTrans(pMEMSINFO, pTCPHASH, pHTTPHASH, pTCPINFO, pNode, pDATA) < 0) {
					nifo_node_delete(pMEMSINFO, pNode);
				}	
			}
		} else {
			usleep(0);
		}

		nowTime = time(NULL);
		if(nowTime >= oldTime + 60) {
			oldTime = nowTime;
			log_print(LOGN_CRI, "CUR_SESS_CNT[%lld] SESS_CNT[%lld] RCVNODE_CNT[%lld] DIFFSEQ_CNT[%lld]", 
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
 *  $Log: http_main.c,v $
 *  Revision 1.3  2011/09/07 04:22:45  uamyd
 *  corrected .. etc
 *
 *  Revision 1.2  2011/09/04 11:12:12  dhkim
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *  NEW OAM SYSTEM
 *
 *  Revision 1.4  2011/08/21 09:07:51  hhbaek
 *  Commit TAF/SRC
 *
 *  Revision 1.3  2011/08/10 09:57:43  uamyd
 *  modified and block added
 *
 *  Revision 1.2  2011/08/05 09:04:49  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 *  init DQMS2
 *
 *  Revision 1.9  2011/05/09 15:18:31  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.8  2011/01/11 04:09:07  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2010/08/23 01:12:59  uamyd
 *  DQMS With TOTMON, 2nd-import
 *
 *  Revision 1.7  2009/08/10 19:24:39  dqms
 *  패킷이 빠졌을 때 연산에서 MICRO 시간까지 비교
 *
 *  Revision 1.6  2009/08/06 06:56:09  dqms
 *  로그레벨 공유메모리로 수정
 *
 *  Revision 1.5  2009/07/15 16:12:38  dqms
 *  멀티프로세스 수정
 *
 *  Revision 1.4  2009/06/28 12:57:45  dqms
 *  ADD set_version
 *
 *  Revision 1.3  2009/06/19 12:11:27  jsyoon
 *  MODIFIED MULTIPLE LOGGING
 *
 *  Revision 1.2  2009/06/15 08:45:42  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2009/05/26 02:14:21  dqms
 *  Init TAF_RPPI
 *
 *  Revision 1.5  2008/11/25 12:50:04  dark264sh
 *  WIDGET 처리
 *
 *  Revision 1.4  2008/09/18 07:40:39  dark264sh
 *  IM 서비스 추가 (SIP, XCAP, MSRP)
 *
 *  Revision 1.3  2008/06/18 12:28:26  dark264sh
 *  A_FB 추가
 *
 *  Revision 1.2  2008/06/17 12:23:56  dark264sh
 *  A_FV, A_EMS 추가
 *
 *  Revision 1.1.1.1  2008/06/09 08:17:17  jsyoon
 *  WATAS3 PROJECT START
 *
 *  Revision 1.3  2007/09/05 06:13:07  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.2  2007/08/29 07:41:01  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.1  2007/08/21 12:53:00  dark264sh
 *  no message
 *
 *  Revision 1.13  2007/04/18 05:28:39  dark264sh
 *  20070409 적용 TCP_SESS_CNT 변경
 *
 *  Revision 1.12  2006/11/28 15:33:28  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.11  2006/11/28 12:16:00  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.10  2006/11/08 07:13:41  shlee
 *  CONF관련 hasho -> hashg로 변경 및 CONF_CNT 101 CONF_PREA_CNT 811로 변경
 *
 *  Revision 1.9  2006/11/02 07:19:42  dark264sh
 *  REQ 메시지가 두개로 나누어 진 경우 URL 처리가 잘못되는 문제 해결
 *
 *  Revision 1.8  2006/11/01 09:25:02  dark264sh
 *  SESS, SEQ, NODE 개수 LOG추가
 *
 *  Revision 1.7  2006/10/18 08:53:31  dark264sh
 *  nifo debug 코드 추가
 *
 *  Revision 1.6  2006/10/16 14:42:21  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.5  2006/10/16 12:46:26  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.4  2006/10/16 12:34:41  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.3  2006/10/12 12:56:00  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.2  2006/10/11 11:52:33  dark264sh
 *  PRE_A, A_TCP, A_HTTP에 SVC filter 적용
 *
 *  Revision 1.1.1.1  2006/10/11 07:45:49  dark264sh
 *  no message
 *
 *  Revision 1.25  2006/10/09 02:40:14  dark264sh
 *  CALL MSGQID 변경
 *
 *  Revision 1.24  2006/09/25 06:28:11  dark264sh
 *  http_func.c
 *
 *  Revision 1.23  2006/09/25 02:58:47  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.22  2006/09/18 04:49:23  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.21  2006/09/06 11:55:30  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.20  2006/09/06 10:58:25  dark264sh
 *  ../../INCOLD 변경
 *
 *  Revision 1.19  2006/08/29 04:27:57  dark264sh
 *  no message
 *
 *  Revision 1.18  2006/08/28 04:04:27  dark264sh
 *  no message
 *
 *  Revision 1.17  2006/08/28 03:56:28  dark264sh
 *  no message
 *
 *  Revision 1.16  2006/08/28 01:46:40  dark264sh
 *  msgqid를 global로 변경
 *
 *  Revision 1.15  2006/08/24 04:08:16  dark264sh
 *  HTTP 기본 Flow 구성
 *
 *  Revision 1.14  2006/08/14 07:04:40  dark264sh
 *  no message
 *
 *  Revision 1.13  2006/07/26 03:05:57  dark264sh
 *  TCP_START 처리 추가
 *
 *  Revision 1.12  2006/07/26 00:24:43  dark264sh
 *  변경 저장 옵션 추가
 *
 */
