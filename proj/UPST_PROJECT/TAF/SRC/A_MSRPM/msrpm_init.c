/**		@file	msrpm_init.c
 * 		- A_MSRPM 프로세스를 초기화 하는 함수들
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: msrpm_init.c,v 1.2 2011/09/05 05:43:37 uamyd Exp $
 *
 * 		@Author		$Author: uamyd $
 * 		@version	$Revision: 1.2 $
 * 		@date		$Date: 2011/09/05 05:43:37 $
 * 		@ref		msrpm_init.c
 *
 * 		@section	Intro(소개)
 * 		- A_MSRPM 프로세스를 초기화 하는 함수들
 *
 * 		@section	Requirement
 *
 **/

#include <stdio.h>
#include <errno.h>
#include <signal.h>

#include "loglib.h"
#include "mems.h"
#include "cifo.h"
#include "nifo.h"
#include "gifo.h"

#include "path.h"
#include "common_stg.h"
#include "procid.h"
#include "sshmid.h"
#include "commdef.h"

#include "msrpm_func.h"
#include "msrpm_init.h"


extern stCIFO		*gpCIFO;
extern S32			giFinishSignal;
extern S32			giStopFlag;

/** dInitMSRPM function.
 *
 *  dInitMSRPM Function
 *
 *  @return			S32
 *  @see			msrpm_init.c
 *
 **/
S32 dInitMSRPM(stMEMSINFO **pstMEMSINFO, stHASHOINFO **pstMSRPMHASH)
{
	/* Setup Signal */
	SetUpSignal();

	/* Init Shared Memory */
	if((*pstMEMSINFO = nifo_init_zone((U8*)"A_MSRPM", SEQ_PROC_A_MSRPM, FILE_NIFO_ZONE)) == NULL) {
        log_print(LOGN_CRI, LH"FAILED IN nifo_init NULL", LT);
        return -1;
    }

    if((gpCIFO = gifo_init_group(FILE_CIFO_CONF, FILE_GIFO_CONF)) == NULL ){
        log_print(LOGN_CRI, LH"FAILED IN gifo_init_group, cifo=%s, gifo=%s",
            LT, FILE_CIFO_CONF, FILE_GIFO_CONF);
        return -2;
    }

	/* MSRPM Hash Table 초기화 */
	/* MSRPM Transaction 관리를 위한 구조체를 만들어야 함 */
	if((*pstMSRPMHASH = hasho_init(
		S_SSHM_A_MSRPM, DEF_MSRPMTSESSKEY_SIZE, DEF_MSRPMTSESSKEY_SIZE, DEF_MSRPMTSESS_SIZE, TCP_SESS_CNT, 0)) == NULL) {
		log_print(LOGN_CRI, "F=%s:%s.%d hasho_init MSRPM", __FILE__, __FUNCTION__, __LINE__);
		return -2;
	}

	return 0;
}

/** SetUpSignal function.
 *
 *  SetUpSignal Function
 *
 *  @return			void
 *  @see			msrpm_init.c
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
 *  @see			msrpm_init.c
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
 *  @see			msrpm_init.c
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
 *  @see			msrpm_init.c
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
 * $Log: msrpm_init.c,v $
 * Revision 1.2  2011/09/05 05:43:37  uamyd
 * MSRPM modified
 *
 * Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * NEW OAM SYSTEM
 *
 * Revision 1.4  2011/08/21 09:07:52  hhbaek
 * Commit TAF/SRC
 *
 * Revision 1.3  2011/08/17 12:12:18  dcham
 * *** empty log message ***
 *
 * Revision 1.2  2011/08/08 11:05:43  uamyd
 * modified block added
 *
 * Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 * init DQMS2
 *
 * Revision 1.3  2011/01/11 04:09:08  uamyd
 * modified
 *
 * Revision 1.1.1.1  2010/08/23 01:12:58  uamyd
 * DQMS With TOTMON, 2nd-import
 *
 * Revision 1.2  2009/06/29 13:23:23  dark264sh
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2009/05/26 02:14:39  dqms
 * Init TAF_RPPI
 *
 * Revision 1.1  2008/09/18 06:35:03  dark264sh
 * IM 서비스 추가 (SIP, XCAP, MSRP)
 *
 * Revision 1.1.1.1  2007/12/27 08:17:40  uamyd
 * import
 *
 * Revision 1.1.1.1  2007/10/22 07:50:43  jsyoon
 * AQUA3 Project = AQUA2 + IMS Protocol + Packet Dump
 *
 * Revision 1.1  2007/05/07 01:46:17  dark264sh
 * INIT
 *
 */
