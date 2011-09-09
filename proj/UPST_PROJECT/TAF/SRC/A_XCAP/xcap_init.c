/**		@file	xcap_init.c
 *      - XCAP Service Processing
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: xcap_init.c,v 1.2 2011/09/06 12:46:42 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.2 $
 * 		@date		$Date: 2011/09/06 12:46:42 $
 * 		@ref		xcap_init.c xcap_maic.c
 *
 * 		@section	Intro(소개)
 *      - XCAP Service Processing
 *
 * 		@section	Requirement
 *
 **/

/**
 *	Include headers
 */
#include <stdio.h>
#include <errno.h>
#include <signal.h>

// TOP
#include "commdef.h"
#include "procid.h"
#include "common_stg.h"
#include "path.h"

// LIB
#include "mems.h"
#include "cifo.h"
#include "gifo.h"
#include "nifo.h"
#include "loglib.h"
#include "filelib.h"

// ,
#include "xcap_init.h"

/**
 *	Declare var.
 */
extern stCIFO		*gpCIFO;
extern S32			giFinishSignal;
extern S32			giStopFlag;

/**
 *	Implement func.
 */

/** dInitXCAP function.
 *
 *  dInitXCAP Function
 *
 *  @return			S32
 *  @see			xcap_init.c xcap_main.c
 *
 **/
S32 dInitXCAP(stMEMSINFO **pMEMSINFO)
{
	/* Setup Signal */
	SetUpSignal();

	if((*pMEMSINFO = nifo_init_zone((U8*)"A_XCAP", SEQ_PROC_A_XCAP, FILE_NIFO_ZONE)) == NULL) {
        log_print(LOGN_CRI, LH"FAILED IN nifo_init NULL", LT);
        return -1;
    }

    if((gpCIFO = gifo_init_group(FILE_CIFO_CONF, FILE_GIFO_CONF)) == NULL ){
        log_print(LOGN_CRI, LH"FAILED IN gifo_init_group, cifo=%s, gifo=%s",
            LT, FILE_CIFO_CONF, FILE_GIFO_CONF);
        return -2;
    }

	return 0;
}

/** SetUpSignal function.
 *
 *  SetUpSignal Function
 *
 *  @return			void
 *  @see			xcap_init.c xcap_main.c
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
 *  @see			xcap_init.c xcap_main.c
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
 *  @see			xcap_init.c xcap_main.c
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
 *  @see			xcap_init.c xcap_main.c
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
 * $Log: xcap_init.c,v $
 * Revision 1.2  2011/09/06 12:46:42  hhbaek
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * NEW OAM SYSTEM
 *
 * Revision 1.4  2011/08/21 09:07:53  hhbaek
 * Commit TAF/SRC/ *
 *
 * Revision 1.3  2011/08/17 07:28:12  dcham
 * *** empty log message ***
 *
 * Revision 1.2  2011/08/08 11:05:44  uamyd
 * modified block added
 *
 * Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 * init DQMS2
 *
 * Revision 1.5  2011/05/09 14:26:11  dark264sh
 * A_XCAP: A_CALL multi 처리
 *
 * Revision 1.4  2011/01/11 04:09:11  uamyd
 * modified
 *
 * Revision 1.1.1.1  2010/08/23 01:13:01  uamyd
 * DQMS With TOTMON, 2nd-import
 *
 * Revision 1.3  2009/06/29 13:28:07  dark264sh
 * *** empty log message ***
 *
 * Revision 1.2  2009/06/12 15:53:55  jsyoon
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2009/05/26 02:14:30  dqms
 * Init TAF_RPPI
 *
 * Revision 1.1  2008/09/18 07:24:37  dark264sh
 * IM 서비스 추가 (SIP, XCAP, MSRP)
 *
 * Revision 1.1.1.1  2007/12/27 08:17:38  uamyd
 * import
 *
 * Revision 1.1.1.1  2007/10/22 07:50:44  jsyoon
 * AQUA3 Project = AQUA2 + IMS Protocol + Packet Dump
 *
 * Revision 1.2  2007/05/17 02:36:20  dark264sh
 * SEQ_PROC_CILOG => SEQ_PROC_CI_LOG, S_MSGQ_CILOG => S_MSGQ_CI_LOG 변경
 *
 * Revision 1.1  2007/03/07 10:33:41  dark264sh
 * *** empty log message ***
 *
 * Revision 1.1  2007/03/07 01:15:19  dark264sh
 * *** empty log message ***
 *
 */
