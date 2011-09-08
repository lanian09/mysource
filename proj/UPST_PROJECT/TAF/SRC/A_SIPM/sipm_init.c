/**		@file	sipm_init.c
 * 		- A_SIPM 프로세스를 초기화 하는 함수들
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: sipm_init.c,v 1.2 2011/09/05 12:26:42 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.2 $
 * 		@date		$Date: 2011/09/05 12:26:42 $
 * 		@ref		sipm_init.c
 *
 * 		@section	Intro(소개)
 * 		- A_SIPM 프로세스를 초기화 하는 함수들
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
#include "procid.h"
#include "sshmid.h"
#include "commdef.h"
#include "common_stg.h"
#include "path.h"

// LIB
#include "mems.h"
#include "cifo.h"
#include "nifo.h"
#include "gifo.h"
#include "loglib.h"
#include "hasho.h"

// .
#include "sipm_func.h"
#include "sipm_init.h"

/**
 * Declare variables
 */
extern stCIFO		*gpCIFO;
extern S32			giFinishSignal;
extern S32			giStopFlag;

extern S32			dMyQID;
extern S32			dSIPTQID;

extern U64			nifo_create;
extern U64			nifo_del;

/**
 *	Implement func.
 */

/** dInitSIPM function.
 *
 *  dInitSIPM Function
 *
 *  @return			S32
 *  @see			sipm_init.c
 *
 **/
S32 dInitSIPM(stMEMSINFO **pstMEMSINFO, stHASHOINFO **pstSIPMHASH)
{
	/* Setup Signal */
	SetUpSignal();

	if((*pstMEMSINFO = nifo_init_zone((U8*)"A_SIPM", SEQ_PROC_A_SIPM, FILE_NIFO_ZONE)) == NULL) {
        log_print(LOGN_CRI, LH"FAILED IN nifo_init NULL", LT);
        return -1;
    }

    if((gpCIFO = gifo_init_group(FILE_CIFO_CONF, FILE_GIFO_CONF)) == NULL ){
        log_print(LOGN_CRI, LH"FAILED IN gifo_init_group, cifo=%s, gifo=%s",
            LT, FILE_CIFO_CONF, FILE_GIFO_CONF);
        return -2;
    }

	/* SIPM Hash Table 초기화 */
	/* SIPM Transaction 관리를 위한 구조체를 만들어야 함 */
	if((*pstSIPMHASH = hasho_init(
		S_SSHM_A_SIPM, DEF_SIPMTSESSKEY_SIZE, DEF_SIPMTSESSKEY_SIZE, DEF_SIPMTSESS_SIZE, TCP_SESS_CNT, 0)) == NULL) {
		log_print(LOGN_CRI, LH"hasho_init SIPM", LT);
		return -3;
	}

	return 0;
}

/** SetUpSignal function.
 *
 *  SetUpSignal Function
 *
 *  @return			void
 *  @see			sipm_init.c
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
 *  @see			sipm_init.c
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
 *  @see			sipm_init.c
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
 *  @see			sipm_init.c
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
 * $Log: sipm_init.c,v $
 * Revision 1.2  2011/09/05 12:26:42  hhbaek
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * NEW OAM SYSTEM
 *
 * Revision 1.3  2011/08/18 01:38:45  hhbaek
 * A_SIPM
 *
 * Revision 1.2  2011/08/09 05:31:08  uamyd
 * modified
 *
 * Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 * init DQMS2
 *
 * Revision 1.3  2011/01/11 04:09:10  uamyd
 * modified
 *
 * Revision 1.1.1.1  2010/08/23 01:13:02  uamyd
 * DQMS With TOTMON, 2nd-import
 *
 * Revision 1.2  2009/06/29 13:25:39  dark264sh
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2009/05/26 02:14:36  dqms
 * Init TAF_RPPI
 *
 * Revision 1.1  2008/09/18 07:19:52  dark264sh
 * IM 서비스 추가 (SIP, XCAP, MSRP)
 *
 * Revision 1.1.1.1  2007/12/27 08:17:38  uamyd
 * import
 *
 * Revision 1.1.1.1  2007/10/22 07:50:44  jsyoon
 * AQUA3 Project = AQUA2 + IMS Protocol + Packet Dump
 *
 * Revision 1.1  2007/05/10 02:57:30  dark264sh
 * A_SIPM (TCP Merge) 추가
 *
 */
