/**		@file	jnc_init.c
 * 		- A_JNC 프로세스를 초기화 하는 함수들
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: jnc_init.c,v 1.2 2011/09/05 05:21:21 uamyd Exp $
 *
 * 		@Author		$Author: uamyd $
 * 		@version	$Revision: 1.2 $
 * 		@date		$Date: 2011/09/05 05:21:21 $
 * 		@ref		jnc_init.c jnc_main.c 
 *
 * 		@section	Intro(소개)
 * 		- A_JNC 프로세스를 초기화 하는 함수들
 *
 * 		@section	Requirement
 *
 **/

#include <stdio.h>
#include <errno.h>
#include <signal.h>

#include "loglib.h"
#include "filelib.h"

#include "mems.h"
#include "cifo.h"
#include "gifo.h"
#include "nifo.h"

#include "path.h"
#include "common_stg.h"
#include "commdef.h"
#include "procid.h"
#include "sshmid.h"

#include "jnc_init.h"
#include "jnc_func.h"	/* JNC_SESS_KEY/DATA XX */

extern stCIFO		*gpCIFO;
extern S32			giFinishSignal;
extern S32			giStopFlag;

int					gACALLCnt = 0;

/** dHttpInit function.
 *
 *  dHttpInit Function
 *
 *  @return			S32
 *  @see			http_init.c l4.h http_main.c a_http_api.h
 *
 **/
S32 dInitJNC(stMEMSINFO **pMEMSINFO, stHASHOINFO **pJNCHASH)
{
	/* Setup Signal */
	SetUpSignal();

	if((*pMEMSINFO = nifo_init_zone((U8*)"A_JNC", SEQ_PROC_A_JNC, FILE_NIFO_ZONE)) == NULL) {
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

	/* JNC Session을 관리를 위한 구조체 생성 */
	if((*pJNCHASH = hasho_init(S_SSHM_A_JNC, JNC_SESS_KEY_SIZE, JNC_SESS_KEY_SIZE, JNC_SESS_SIZE, TCP_SESS_CNT, 0))==NULL){
		log_print(LOGN_CRI, "[%s,%d] hashg_init LJNCHASH NULL", __FUNCTION__,__LINE__);
		return -3;
	}
	
	return 0;
}

/** SetUpSignal function.
 *
 *  SetUpSignal Function
 *
 *  @return			void
 *  @see			jnet_init.c jnet_main.c jnet_api.h
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
 *  @see			jnet_init.c jnet_main.c jnet_api.h
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
 *  @see			jnc_init.c jnc_main.c a_jnc_api.h
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

