/**		@file	http_init.c
 * 		- A_HTTP 프로세스를 초기화 하는 함수들
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: http_init.c,v 1.3 2011/09/07 04:22:45 uamyd Exp $
 *
 * 		@Author		$Author: uamyd $
 * 		@version	$Revision: 1.3 $
 * 		@date		$Date: 2011/09/07 04:22:45 $
 * 		@ref		http_init.c l4.h http_maic.c a_http_api.h
 *
 * 		@section	Intro(소개)
 * 		- A_HTTP 프로세스를 초기화 하는 함수들
 *
 * 		@section	Requirement
 *
 **/

#include <stdio.h>
#include <errno.h>
#include <signal.h>

// LIB
#include "typedef.h"
#include "loglib.h"
#include "filelib.h"
#include "mems.h"
#include "cifo.h"
#include "gifo.h"
#include "nifo.h"

// PROJECT 
#include "path.h"
#include "procid.h"
#include "sshmid.h"
#include "common_stg.h"

// .
#include "http_init.h"

extern stCIFO		*gpCIFO;
extern S32			giFinishSignal;
extern S32			giStopFlag;
extern UINT 		guiSeqProcID;

extern char         gszMyProc[32];

int					gACALLCnt = 0;

/** dHttpInit function.
 *
 *  dHttpInit Function
 *
 *  @return			S32
 *  @see			http_init.c l4.h http_main.c a_http_api.h
 *
 **/
S32 dInitHttp(stMEMSINFO **pMEMSINFO, stHASHOINFO **pTCPHASH, stHASHOINFO **pHTTPHASH)
{
	int				dLen, ProcNum;
	U32				uiShmTSESSKey, uiShmHTTPKey;

	SetUpSignal();

	/* Init Shared Memory */
	//	if((dRet = dInitShm()) < 0)
	//	{
	//		log_print(LOGN_CRI, "[%s.%d] dInitShm dRet[%d]", __FUNCTION__, __LINE__, dRet);
	//		return -1;
	//	}

	dLen = strlen(gszMyProc);
	ProcNum = atoi(&gszMyProc[dLen-1]);
	uiShmTSESSKey = S_SSHM_A_TSESS0 + ProcNum;
	uiShmHTTPKey = S_SSHM_A_HTTP0 + ProcNum;

	if((*pMEMSINFO = nifo_init_zone((U8*)gszMyProc, guiSeqProcID, FILE_NIFO_ZONE)) == NULL) {
		log_print(LOGN_CRI, LH"FAILED IN nifo_init_zone NULL", LT);
		return -1;
	}

	if((gpCIFO = gifo_init_group(FILE_CIFO_CONF, FILE_GIFO_CONF)) == NULL ){
		log_print(LOGN_CRI, LH"FAILED IN gifo_init_group, cifo=%s, gifo=%s",
			LT, FILE_CIFO_CONF, FILE_GIFO_CONF);
		return -2;
	}

	gACALLCnt = get_block_num(FILE_MC_INIT, "A_CALL");
	log_print(LOGN_INFO, "INIT., A_CALL ProcCount[%d]", gACALLCnt);

	/* TCP Hash Table 초기화 */
	/* TCP 세션 관리를 위한 구조체를 만들어야 함 */
	if((*pTCPHASH = hasho_init(
					uiShmTSESSKey, HTTP_TSESS_KEY_SIZE, HTTP_TSESS_KEY_SIZE, HTTP_TSESS_SIZE, TCP_SESS_CNT, 0)) == NULL) {
		log_print(LOGN_CRI, LH"hasho_init TCP", LT);
		return -2;
	}

	log_print(LOGN_CRI, "##### hashg_init TSESS_KEY_SIZE[%zd] TSESS_SIZE[%zd]", HTTP_TSESS_KEY_SIZE, HTTP_TSESS_SIZE );
	log_print(LOGN_CRI, "##### hashg_init TSESS_KEY_SIZE[%zd] TSESS_SIZE[%zd]", sizeof(HTTP_TSESS_KEY), sizeof(HTTP_TSESS) );

	/* HTTP Hash Table 초기화 */
	/* HTTP Transaction 관리를 위한 구조체를 만들어야 함 */
	if((*pHTTPHASH = hasho_init(
					uiShmHTTPKey, HTTP_TRANS_KEY_SIZE, HTTP_TRANS_KEY_SIZE, HTTP_TRANS_SIZE, HTTP_TRANS_CNT, 0)) == NULL) {
		log_print(LOGN_CRI, LH"hashg_init HTTP",LT);
		return -3;
	}

	return 0;
}

/** SetUpSignal function.
 *
 *  SetUpSignal Function
 *
 *  @return			void
 *  @see			http_init.c l4.h http_main.c a_http_api.h
 *
 **/
void SetUpSignal(void)
{
	giStopFlag = 1;

	/* WANTED SIGNALS */
	signal(SIGTERM, UserControlledSignal);
	signal(SIGINT, UserControlledSignal);
	signal(SIGQUIT, UserControlledSignal);

	/* UNWANTED SIGNALS */
	signal(SIGHUP, IgnoreSignal);
	signal(SIGALRM, IgnoreSignal);
	signal(SIGPIPE, IgnoreSignal);
	signal(SIGPOLL, IgnoreSignal);
	signal(SIGPROF, IgnoreSignal);
	signal(SIGUSR1, IgnoreSignal);
	signal(SIGUSR2, IgnoreSignal);
	signal(SIGVTALRM, IgnoreSignal);
	signal(SIGCLD, IgnoreSignal);
}

/** UserControlledSignal function.
 *
 *  UserControlledSignal Function
 *
 *	@param	isign	:	signal
 *
 *  @return			void
 *  @see			http_init.c l4.h http_main.c a_http_api.h
 *
 **/
void UserControlledSignal(S32 isign)
{
	giStopFlag = 0;
	giFinishSignal = isign;
	log_print(LOGN_CRI, "User Controlled Signal Req = %d", isign);
}

/** FinishProgram function.
 *
 *  FinishProgram Function
 *
 *  @return			void
 *  @see			http_init.c l4.h http_main.c
 *
 **/
void FinishProgram(void)
{
	log_print(LOGN_CRI, "PROGRAM IS NORMALLY TERMINATED, Cause = %d", giFinishSignal);
	exit(0);
}

/** IgnoreSignal function.
 *
 *  IgnoreSignal Function
 *
 *	@param	isign	:	signal
 *
 *  @return			void
 *  @see			http_init.c l4.h http_main.c a_http_api.h
 *
 **/
void IgnoreSignal(S32 isign)
{
	if (isign != SIGALRM)
	{
		log_print(LOGN_CRI, "UNWANTED SIGNAL IS RECEIVED, signal = %d", isign);
	}

	signal(isign, IgnoreSignal);
}

/*
 * $Log: http_init.c,v $
 * Revision 1.3  2011/09/07 04:22:45  uamyd
 * corrected .. etc
 *
 * Revision 1.2  2011/09/04 11:12:12  dhkim
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * NEW OAM SYSTEM
 *
 * Revision 1.3  2011/08/21 09:07:50  hhbaek
 * Commit TAF/SRC
 *
 * Revision 1.2  2011/08/05 09:04:49  uamyd
 * modified
 *
 * Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 * init DQMS2
 *
 * Revision 1.7  2011/05/09 15:18:31  jsyoon
 * *** empty log message ***
 *
 * Revision 1.6  2011/01/11 04:09:07  uamyd
 * modified
 *
 * Revision 1.1.1.1  2010/08/23 01:12:59  uamyd
 * DQMS With TOTMON, 2nd-import
 *
 * Revision 1.5  2009/07/15 16:12:38  dqms
 * 멀티프로세스 수정
 *
 * Revision 1.4  2009/06/29 13:21:34  dark264sh
 * *** empty log message ***
 *
 * Revision 1.3  2009/06/19 13:17:09  jsyoon
 * MODIFIED MULTIPLE HASH
 *
 * Revision 1.2  2009/06/15 08:45:42  jsyoon
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2009/05/26 02:14:20  dqms
 * Init TAF_RPPI
 *
 * Revision 1.5  2008/11/25 12:50:04  dark264sh
 * WIDGET 처리
 *
 * Revision 1.4  2008/09/18 07:40:39  dark264sh
 * IM 서비스 추가 (SIP, XCAP, MSRP)
 *
 * Revision 1.3  2008/06/18 12:28:26  dark264sh
 * A_FB 추가
 *
 * Revision 1.2  2008/06/17 12:23:56  dark264sh
 * A_FV, A_EMS 추가
 *
 * Revision 1.1.1.1  2008/06/09 08:17:17  jsyoon
 * WATAS3 PROJECT START
 *
 * Revision 1.2  2007/08/29 07:41:01  dark264sh
 * *** empty log message ***
 *
 * Revision 1.1  2007/08/21 12:53:00  dark264sh
 * no message
 *
 * Revision 1.8  2006/11/28 12:58:27  cjlee
 * doxygen
 *
 * Revision 1.7  2006/11/08 07:13:41  shlee
 * CONF관련 hasho -> hashg로 변경 및 CONF_CNT 101 CONF_PREA_CNT 811로 변경
 *
 * Revision 1.6  2006/11/07 08:38:41  shlee
 * hasho_init Function
 *
 * Revision 1.5  2006/10/20 10:01:32  dark264sh
 * *** empty log message ***
 *
 * Revision 1.4  2006/10/18 08:53:31  dark264sh
 * nifo debug 코드 추가
 *
 * Revision 1.3  2006/10/12 07:49:37  dark264sh
 * hash key값 초기화 문제 버그 해결
 *
 * Revision 1.2  2006/10/11 11:52:33  dark264sh
 * PRE_A, A_TCP, A_HTTP에 SVC filter 적용
 *
 * Revision 1.1.1.1  2006/10/11 07:45:49  dark264sh
 * no message
 *
 * Revision 1.21  2006/10/09 02:40:14  dark264sh
 * CALL MSGQID 변경
 *
 * Revision 1.20  2006/09/22 08:34:25  dark264sh
 * no message
 *
 * Revision 1.19  2006/09/13 04:30:25  dark264sh
 * strerror 잘못 찍는 부분 수정
 *
 * Revision 1.18  2006/09/06 11:55:30  dark264sh
 * *** empty log message ***
 *
 * Revision 1.17  2006/08/28 01:47:30  dark264sh
 * msgqid를 global로 변경
 *
 * Revision 1.16  2006/08/28 01:46:40  dark264sh
 * msgqid를 global로 변경
 *
 * Revision 1.15  2006/08/28 01:38:56  dark264sh
 * no message
 *
 * Revision 1.14  2006/08/28 01:25:32  dark264sh
 * no message
 *
 * Revision 1.13  2006/08/28 01:16:29  dark264sh
 * a_http_api.h => http_api.h 변경
 *
 * Revision 1.12  2006/08/24 04:08:16  dark264sh
 * HTTP 기본 Flow 구성
 *
 * Revision 1.11  2006/07/26 11:21:09  dark264sh
 * A_HTTP
 * TCP_DATA pseudo 코드 추가
 *
 * Revision 1.10  2006/07/26 03:05:57  dark264sh
 * TCP_START 처리 추가
 *
 */
