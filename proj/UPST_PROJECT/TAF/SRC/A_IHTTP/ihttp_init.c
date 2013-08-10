/**		@file	http_init.c
 * 		- A_HTTP 프로세스를 초기화 하는 함수들
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: ihttp_init.c,v 1.2 2011/09/04 11:40:36 dhkim Exp $
 *
 * 		@Author		$Author: dhkim $
 * 		@version	$Revision: 1.2 $
 * 		@date		$Date: 2011/09/04 11:40:36 $
 * 		@ref		http_init.c l4.h http_maic.c a_http_api.h
 *
 * 		@section	Intro(소개)
 * 		- A_HTTP 프로세스를 초기화 하는 함수들
 *
 * 		@section	Requirement
 *
 **/

#include <signal.h>

// LIB
#include "typedef.h"
#include "loglib.h"
#include "filelib.h"
#include "mems.h"
#include "nifo.h"
#include "gifo.h"
#include "cifo.h"
#include "hasho.h"


// PROJECT
#include "path.h"
#include "sshmid.h"
#include "common_stg.h"


// .
#include "ihttp_init.h"

/**
 * Declare variables
 */
extern stCIFO		*gpCIFO;
extern S32			giFinishSignal;
extern S32			giStopFlag;
extern UINT			guiSeqProcID;

extern S32			gACALLCnt;

extern char			gszMyProc[32];


/** dHttpInit function.
 *
 *	dHttpInit Function
 *
 *	@return			S32
 *	@see			http_init.c l4.h http_main.c a_http_api.h
 *
 **/
S32 dInitHttp(stMEMSINFO **pMEMSINFO, stHASHOINFO **pTCPHASH, stHASHOINFO **pHTTPHASH)
{
	int				dLen, ProcNum;
	U32				uiShmTSESSKey, uiShmHTTPKey;

	SetUpSignal();

	dLen = strlen(gszMyProc);
	ProcNum = atoi(&gszMyProc[dLen-1]);
	uiShmTSESSKey = S_SSHM_A_ITSESS0 + ProcNum;
	uiShmHTTPKey = S_SSHM_A_IHTTP0 + ProcNum;


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
		uiShmTSESSKey, HTTP_TSESS_KEY_SIZE, HTTP_TSESS_KEY_SIZE, HTTP_TSESS_SIZE, ITCP_SESS_CNT, 0)) == NULL) {
		log_print(LOGN_CRI, "[%s][%s.%d] hasho_init TCP", __FILE__, __FUNCTION__, __LINE__);
		return -2;
	}

	log_print(LOGN_CRI, "##### hashg_init TSESS_KEY_SIZE[%zd] TSESS_SIZE[%zd]", HTTP_TSESS_KEY_SIZE, HTTP_TSESS_SIZE );
	log_print(LOGN_CRI, "##### hashg_init TSESS_KEY_SIZE[%zd] TSESS_SIZE[%zd]", sizeof(HTTP_TSESS_KEY), sizeof(HTTP_TSESS) );

	/* HTTP Hash Table 초기화 */
	/* HTTP Transaction 관리를 위한 구조체를 만들어야 함 */
	if((*pHTTPHASH = hasho_init(
		uiShmHTTPKey, HTTP_TRANS_KEY_SIZE, HTTP_TRANS_KEY_SIZE, HTTP_TRANS_SIZE, IHTTP_TRANS_CNT, 0)) == NULL) {
		log_print(LOGN_CRI, "[%s][%s.%d] hashg_init HTTP", __FILE__, __FUNCTION__, __LINE__);
		return -3;
	}

	return 0;
}

/** SetUpSignal function.
 *
 *	SetUpSignal Function
 *
 *	@return			void
 *	@see			http_init.c l4.h http_main.c a_http_api.h
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
 *	UserControlledSignal Function
 *
 *	@param	isign	:	signal
 *
 *	@return			void
 *	@see			http_init.c l4.h http_main.c a_http_api.h
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
 *	FinishProgram Function
 *
 *	@return			void
 *	@see			http_init.c l4.h http_main.c
 *
 **/
void FinishProgram(void)
{
	log_print(LOGN_CRI, "PROGRAM IS NORMALLY TERMINATED, Cause = %d", giFinishSignal);
	exit(0);
}

/** IgnoreSignal function.
 *
 *	IgnoreSignal Function
 *
 *	@param	isign	:	signal
 *
 *	@return			void
 *	@see			http_init.c l4.h http_main.c a_http_api.h
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
 * $Log: ihttp_init.c,v $
 * Revision 1.2  2011/09/04 11:40:36  dhkim
 * *** empty log message ***
 *
 * Revision 1.1.1.1	2011/08/29 05:56:42	dcham
 * NEW OAM SYSTEM
 *
 * Revision 1.3	2011/08/17 12:57:49	hhbaek
 * A_IHTTP
 *
 * Revision 1.2	2011/08/10 09:57:43	uamyd
 * modified and block added
 *
 * Revision 1.1.1.1	2011/08/05 00:27:18	uamyd
 * init DQMS2
 *
 * Revision 1.2	2011/05/09 10:29:12	dark264sh
 * A_IHTTP: A_CALL multi 처리
 *
 * Revision 1.1	2011/04/11 12:06:34	dark264sh
 * A_IHTTP 추가
 *
 */
