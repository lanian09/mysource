/**		@file	m_svcmon_init.c
 * 		- M_SVCMON 프로세스를 초기화 하는 함수들
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: m_svcmon_init.c,v 1.2 2011/09/01 07:49:50 dcham Exp $
 *
 * 		@Author		$Author: dcham $
 * 		@version	$Revision: 1.2 $
 * 		@date		$Date: 2011/09/01 07:49:50 $
 * 		@ref		m_svcmon_init.c m_svcmon_maic.c
 *
 * 		@section	Intro(소개)
 * 		- M_SVCMON 프로세스를 초기화 하는 함수들
 *
 * 		@section	Requirement
 *
 **/
// System Header
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
// LIB Header
#include "typedef.h"
#include "loglib.h"
//TAM Header
#include "watch_mon.h"
// Local Header
#include "m_svcmon_init.h"

extern S32			giFinishSignal;
extern S32			giStopFlag;

extern S32			dMyQID;
extern S32			dSISVCMON;

st_MonTotal	*gMonTotal;

/** dInitMLOG function.
 *
 *  dInitMLOG Function
 *
 *  @return			S32
 *  @see			m_svcmon_init.c m_svcmon_main.c
 *
 **/
S32 dInitMSVCMON()
{
	//S32				dRet;

	/* Setup Signal */
	SetUpSignal();
#if 0
	/* st_MonTotal */
	if((dRet = dInitMonTotalShm()) < 0) {
		log_print(LOGN_CRI, "F=%s:%s.%d dInitMonTotalShm dRet=%d", __FILE__, __FUNCTION__, __LINE__, dRet);
		return -1;
	} 

	/* st_MonTotal_1Min */
	if((dRet = dInitMonTotal1MinShm()) < 0) {
		log_print(LOGN_CRI, "F=%s:%s.%d dInitMonTotal1MinShm dRet=%d", __FILE__, __FUNCTION__, __LINE__, dRet);
		return -1;
	} 

    if((dMyQID = msgget(S_MSGQ_M_SVCMON, 0666|IPC_CREAT)) < 0) {
        log_print(LOGN_CRI, "F=%s:%s.%d msgget S_MSGQ_M_SVCMON error=%d:%s", 
                __FILE__, __FUNCTION__, __LINE__, errno, strerror(errno));
        return -2;
    }

    if((dSISVCMON = msgget(S_MSGQ_SI_SVCMON, 0666|IPC_CREAT)) < 0) {
        log_print(LOGN_CRI, "F=%s:%s.%d msgget S_MSGQ_SI_SVCMON error=%d:%s", 
                __FILE__, __FUNCTION__, __LINE__, errno, strerror(errno));
        return -3;
    }
#endif
	return 0;
}

/** SetUpSignal function.
 *
 *  SetUpSignal Function
 *
 *  @return			void
 *  @see			m_svcmon_init.c m_svcmon_main.c
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
 *  @see			m_svcmon_init.c m_svcmon_main.c
 *
 **/
void UserControlledSignal(S32 isign)
{
	giStopFlag = 0;
	giFinishSignal = isign;
	log_print(LOGN_CRI, "############## User Controlled Signal Req = %d", isign);
}

/** FinishProgram function.
 *
 *  FinishProgram Function
 *
 *  @return			void
 *  @see			m_svcmon_init.c m_svcmon_main.c
 *
 **/
void FinishProgram(void)
{
	log_print(LOGN_CRI, "############### PROGRAM IS NORMALLY TERMINATED, Cause = %d", giFinishSignal);
}

/** IgnoreSignal function.
 *
 *  IgnoreSignal Function
 *
 *	@param	isign	:	signal
 *
 *  @return			void
 *  @see			m_svcmon_init.c m_svcmon_main.c
 *
 **/
void IgnoreSignal(S32 isign)
{
	if (isign != SIGALRM)
		log_print(LOGN_CRI, "############# UNWANTED SIGNAL IS RECEIVED, signal = %d", isign);

	signal(isign, IgnoreSignal);
}

/*
 * $Log: m_svcmon_init.c,v $
 * Revision 1.2  2011/09/01 07:49:50  dcham
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * NEW OAM SYSTEM
 *
 * Revision 1.2  2011/08/23 14:28:16  dcham
 * *** empty log message ***
 *
 * Revision 1.1  2011/08/23 10:59:21  dcham
 * *** empty log message ***
 *
 * Revision 1.4  2011/01/11 04:09:17  uamyd
 * modified
 *
 * Revision 1.2  2010/11/14 10:22:44  jwkim96
 * STP 작업 내용 반영.
 *
 * Revision 1.1.1.1  2010/08/23 01:13:10  uamyd
 * DQMS With TOTMON, 2nd-import
 *
 * Revision 1.3  2009/06/20 13:37:27  dark264sh
 * *** empty log message ***
 *
 * Revision 1.2  2009/06/17 11:03:53  dark264sh
 * O_SVCMON, M_SVCMON MsgQ 사용 변경
 *
 * Revision 1.1  2009/06/16 08:05:51  dark264sh
 * M_SVCMON 기본 동작 처리
 *
 */
