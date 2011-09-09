/**		@file	mms_init.c
 * 		- A_MMS 프로세스를 초기화 하는 함수들
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: mms_init.c,v 1.2 2011/09/05 05:28:46 uamyd Exp $
 *
 * 		@Author		$Author: uamyd $
 * 		@version	$Revision: 1.2 $
 * 		@date		$Date: 2011/09/05 05:28:46 $
 * 		@ref		mms_init.c mms_maic.c
 *
 * 		@section	Intro(소개)
 * 		- A_MMS 프로세스를 초기화 하는 함수들
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
#include "commdef.h"
#include "procid.h"
#include "common_stg.h"

#include "mms_init.h"


extern stCIFO		*gpCIFO;
extern S32			giFinishSignal;
extern S32			giStopFlag;

int					gACALLCnt = 0;

/** dInitMMS function.
 *
 *  dInitMMS Function
 *
 *  @return			S32
 *  @see			mms_init.c mms_main.c
 *
 **/
S32 dInitMMS(stMEMSINFO **pMEMSINFO)
{
	/* Setup Signal */
	SetUpSignal();

	if((*pMEMSINFO = nifo_init_zone((U8*)"A_MMS", SEQ_PROC_A_MMS, FILE_NIFO_ZONE)) == NULL) {
        log_print(LOGN_CRI, LH"FAILED IN nifo_init NULL", LT);
        return -1;
    }

    if((gpCIFO = gifo_init_group(FILE_CIFO_CONF, FILE_GIFO_CONF)) == NULL ){
        log_print(LOGN_CRI, LH"FAILED IN gifo_init_group, cifo=%s, gifo=%s",
            LT, FILE_CIFO_CONF, FILE_GIFO_CONF);
        return -2;
    }
 
	/* Multiple Process : YOON.20110509 */
	gACALLCnt = get_block_num(FILE_MC_INIT, "A_CALL");
	log_print(LOGN_INFO, "INIT., A_CALL ProcCount[%d]", gACALLCnt);
	return 0;
}

/** SetUpSignal function.
 *
 *  SetUpSignal Function
 *
 *  @return			void
 *  @see			mms_init.c mms_main.c
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
 *  @see			mms_init.c mms_main.c
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
 *  @see			mms_init.c mms_main.c
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
 *  @see			mms_init.c mms_main.c
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
 * $Log: mms_init.c,v $
 * Revision 1.2  2011/09/05 05:28:46  uamyd
 * A_MMS modified
 *
 * Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * NEW OAM SYSTEM
 *
 * Revision 1.4  2011/08/21 09:07:51  hhbaek
 * Commit TAF/SRC
 *
 * Revision 1.3  2011/08/17 07:23:55  dcham
 * *** empty log message ***
 *
 * Revision 1.2  2011/08/08 11:05:42  uamyd
 * modified block added
 *
 * Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 * init DQMS2
 *
 * Revision 1.4  2011/05/09 15:21:45  jsyoon
 * *** empty log message ***
 *
 * Revision 1.3  2011/01/11 04:09:08  uamyd
 * modified
 *
 * Revision 1.1.1.1  2010/08/23 01:12:59  uamyd
 * DQMS With TOTMON, 2nd-import
 *
 * Revision 1.2  2009/06/29 13:22:56  dark264sh
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2009/05/26 02:14:40  dqms
 * Init TAF_RPPI
 *
 * Revision 1.1.1.1  2008/06/09 08:17:17  jsyoon
 * WATAS3 PROJECT START
 *
 * Revision 1.1  2007/09/03 08:32:26  dark264sh
 * *** empty log message ***
 *
 */
