/**		@file	pciv_init.c
 * 		- A_IV 프로세스를 초기화 하는 함수들
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: pciv_init.c,v 1.3 2011/09/05 05:05:57 dhkim Exp $
 *
 * 		@Author		$Author: dhkim $
 * 		@version	$Revision: 1.3 $
 * 		@date		$Date: 2011/09/05 05:05:57 $
 * 		@ref		pciv_init.c 
 *
 * 		@section	Intro(소개)
 * 		- A_IV 프로세스를 초기화 하는 함수들
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
#include "memg.h"
#include "cifo.h"
#include "gifo.h"
#include "nifo.h"
//#include "comm_def.h"

// PROJECT
#include "path.h"
#include "procid.h"
#include "sshmid.h"
#include "common_stg.h"

// .
#include "pciv_init.h"
#include "pciv_func.h"


extern stCIFO		*gpCIFO;
extern S32			giFinishSignal;
extern S32			giStopFlag;
extern S32			gACALLCnt;

/** dInitProc function.
 *
 *  dInitProc Function
 *
 *  @return			S32
 *  @see			pciv_init.c pciv_main.c
 *
 **/
S32 dInitProc(stMEMSINFO** ppMEMSINFO, stHASHOINFO** ppHASH)
{
	/* Setup Signal */
	SetUpSignal();

	/* Init Shared Memory */
	if((*ppMEMSINFO = nifo_init_zone((U8*)"A_IV", SEQ_PROC_A_IV, FILE_NIFO_ZONE)) == NULL) {
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

	/* Init Hash table */
	if( (*ppHASH = hasho_init(S_SSHM_A_IV, IV_SESS_KEY_SIZE, IV_SESS_KEY_SIZE, IV_SESS_SIZE, TCP_SESS_CNT, 0)) == NULL ){
			log_print(LOGN_CRI, "[%s,%d] hashg_init pHASH", __FUNCTION__, __LINE__);
			return -3;
	}

	return 0;
}

/** SetUpSignal function.
 *
 *  SetUpSignal Function
 *
 *  @return			void
 *  @see			pciv_init.c pciv_main.c
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
 *  @see			pciv_init.c pciv_main.c
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
 *  @see			pciv_init.c pciv_main.c
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
 *  @see			pciv_init.c pciv_main.c
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
 * $Log: pciv_init.c,v $
 * Revision 1.3  2011/09/05 05:05:57  dhkim
 * *** empty log message ***
 *
 * Revision 1.2  2011/09/05 04:34:09  dhkim
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * NEW OAM SYSTEM
 *
 * Revision 1.4  2011/08/21 09:07:51  hhbaek
 * Commit TAF/SRC
 *
 * Revision 1.3  2011/08/17 07:19:54  dcham
 * *** empty log message ***
 *
 * Revision 1.2  2011/08/08 11:05:41  uamyd
 * modified block added
 *
 * Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 * init DQMS2
 *
 * Revision 1.5  2011/05/09 14:38:34  dark264sh
 * A_IV: A_CALL multi 처리
 *
 * Revision 1.4  2011/01/11 04:09:07  uamyd
 * modified
 *
 * Revision 1.1.1.1  2010/08/23 01:13:03  uamyd
 * DQMS With TOTMON, 2nd-import
 *
 * Revision 1.3  2009/07/15 17:10:56  dqms
 * set_version 위치 및 Plastform Type 변경
 *
 * Revision 1.2  2009/06/29 13:22:08  dark264sh
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2009/05/26 02:14:38  dqms
 * Init TAF_RPPI
 *
 * Revision 1.1  2008/06/23 04:06:41  jyjung
 * A_IV 추가
 *
 *
 */


