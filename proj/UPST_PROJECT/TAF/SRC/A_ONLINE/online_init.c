/**		@file	online_init.c
 * 		- A_ONLINE 프로세스를 초기화 하는 함수들
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: online_init.c,v 1.2 2011/09/05 08:20:23 dhkim Exp $
 *
 * 		@Author		$Author: dhkim $
 * 		@version	$Revision: 1.2 $
 * 		@date		$Date: 2011/09/05 08:20:23 $
 * 		@ref		online_init.c aqua.h online_maic.c online_api.h
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
#include "hasho.h"

// PROJECT
#include "path.h"
#include "commdef.h"
#include "procid.h"
#include "sshmid.h"
#include "common_stg.h"

// .
#include "online_init.h"

/**
 * Declare variables
 */
extern stCIFO		*gpCIFO;
extern S32			giFinishSignal;
extern S32			giStopFlag;
extern S32			gACALLCnt;

/** dInitOnline function.
 *
 *	dInitOnline Function
 *
 *	@return			S32
 *	@see			online_init.c aqua.h online_main.c online_api.h
 *
 **/
S32 dInitOnline(stMEMSINFO **pMEMSINFO, stHASHOINFO **pHASH)
{
	/* Setup Signal */
	SetUpSignal();

	if((*pMEMSINFO = nifo_init_zone((U8*)"A_VOD", SEQ_PROC_A_VOD, FILE_NIFO_ZONE)) == NULL) {
		log_print(LOGN_CRI, LH"FAILED IN nifo_init NULL", LT);
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
	if((*pHASH = hasho_init(S_SSHM_A_ONLINE, ONLINE_TSESS_KEY_SIZE, ONLINE_TSESS_KEY_SIZE, 
		ONLINE_TSESS_SIZE, TCP_SESS_CNT, 0)) == NULL)
	{
		log_print(LOGN_CRI, "[%s][%s.%d] hasho_init ONLINE", __FILE__, __FUNCTION__, __LINE__);
		return -2;
	}

	return 0;
}

/** SetUpSignal function.
 *
 *	SetUpSignal Function
 *
 *	@return			void
 *	@see			online_init.c aqua.h online_main.c online_api.h
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
 *	@see			online_init.c aqua.h online_main.c online_api.h
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
 *	@see			online_init.c aqua.h online_main.c
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
 *	@see			online_init.c aqua.h online_main.c online_api.h
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
 * $Log: online_init.c,v $
 * Revision 1.2  2011/09/05 08:20:23  dhkim
 * *** empty log message ***
 *
 * Revision 1.1.1.1	2011/08/29 05:56:42	dcham
 * NEW OAM SYSTEM
 *
 * Revision 1.3	2011/08/17 13:06:40	hhbaek
 * A_ONLINE
 *
 * Revision 1.2	2011/08/09 08:17:40	uamyd
 * add blocks
 *
 * Revision 1.1.1.1	2011/08/05 00:27:18	uamyd
 * init DQMS2
 *
 * Revision 1.4	2011/05/09 13:59:39	dark264sh
 * A_ONLINE: A_CALL multi 처리
 *
 * Revision 1.3	2011/01/11 04:09:09	uamyd
 * modified
 *
 * Revision 1.1.1.1	2010/08/23 01:12:58	uamyd
 * DQMS With TOTMON, 2nd-import
 *
 * Revision 1.2	2009/06/29 13:23:58	dark264sh
 * *** empty log message ***
 *
 * Revision 1.1.1.1	2009/05/26 02:14:42	dqms
 * Init TAF_RPPI
 *
 * Revision 1.1.1.1	2008/06/09 08:17:16	jsyoon
 * WATAS3 PROJECT START
 *
 * Revision 1.1	2007/08/21 12:53:54	dark264sh
 * no message
 *
 * Revision 1.3	2006/11/28 12:58:27	cjlee
 * doxygen
 *
 * Revision 1.2	2006/11/07 08:44:48	shlee
 * hasho_init function
 *
 * Revision 1.1	2006/10/27 12:35:51	dark264sh
 * *** empty log message ***
 *
 */
