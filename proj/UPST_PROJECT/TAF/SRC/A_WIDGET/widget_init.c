/**		@file	widget_init.c
 * 		- A_WIDGET 프로세스를 초기화 하는 함수들
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: widget_init.c,v 1.2 2011/09/06 12:46:41 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.2 $
 * 		@date		$Date: 2011/09/06 12:46:41 $
 * 		@ref		widget_init.c widget_maic.c
 *
 * 		@section	Intro(소개)
 * 		- A_WIDGET 프로세스를 초기화 하는 함수들
 *
 * 		@section	Requirement
 *
 **/

/**
 * Include headers
 */
#include <stdio.h>
#include <errno.h>
#include <signal.h>

// TOP
#include "common_stg.h"
#include "commdef.h"
#include "procid.h"
#include "path.h"

// LIB
#include "gifo.h"
#include "nifo.h"
#include "loglib.h"
#include "filelib.h"

// .
#include "widget_init.h"

/**
 * Declare variables
 */
extern stCIFO       *gpCIFO;
extern S32			giFinishSignal;
extern S32			giStopFlag;

extern S32			dMyQID;
extern S32			dCallQID[MAX_SMP_NUM];

extern S32			gACALLCnt;

extern U64			nifo_create;
extern U64			nifo_del;

/**
 *	Implement func.
 */

/** dInitWIDGET function.
 *
 *  dInitWIDGET Function
 *
 *  @return			S32
 *  @see			widget_init.c widget_main.c
 *
 **/
S32 dInitWIDGET(stMEMSINFO **pMEMSINFO)
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

	return 0;
}

/** SetUpSignal function.
 *
 *  SetUpSignal Function
 *
 *  @return			void
 *  @see			widget_init.c widget_main.c
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
 *  @see			widget_init.c widget_main.c
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
 *  @see			widget_init.c widget_main.c
 *
 **/
void FinishProgram(void)
{
#ifdef MEM_TEST
    log_print(LOGN_CRI, "CREATE CNT[%llu] DELETE CNT[%llu]", nifo_create, nifo_del);
#endif
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
 *  @see			widget_init.c widget_main.c
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
 * $Log: widget_init.c,v $
 * Revision 1.2  2011/09/06 12:46:41  hhbaek
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * NEW OAM SYSTEM
 *
 * Revision 1.3  2011/08/17 13:16:31  hhbaek
 * A_WIDGET
 *
 * Revision 1.2  2011/08/09 08:17:42  uamyd
 * add blocks
 *
 * Revision 1.1.1.1  2011/08/05 00:27:18  uamyd
 * init DQMS2
 *
 * Revision 1.4  2011/05/09 13:44:47  dark264sh
 * A_WIDGET: A_CALL multi 처리
 *
 * Revision 1.3  2011/01/11 04:09:11  uamyd
 * modified
 *
 * Revision 1.1.1.1  2010/08/23 01:13:01  uamyd
 * DQMS With TOTMON, 2nd-import
 *
 * Revision 1.2  2009/06/29 13:27:35  dark264sh
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2009/05/26 02:14:47  dqms
 * Init TAF_RPPI
 *
 * Revision 1.1  2008/11/25 12:45:57  dark264sh
 * WIDGET 처리
 *
 */
