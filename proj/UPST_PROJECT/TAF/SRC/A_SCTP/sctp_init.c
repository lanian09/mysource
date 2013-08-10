/******************************************************************************* 
		@file   sctp_init.c
 *      - A_SCTP 프로세스를 초기화 하는 함수들
 *
 *      Copyright (c) 2006~ by Upresto Inc. Korea
 *      All rights reserved
 *
 *      $Id: sctp_init.c,v 1.2 2011/09/06 02:07:44 dcham Exp $
 *
 *      @Author     $Author: dcham $
 *      @version    $Revision: 1.2 $
 *      @date       $Date: 2011/09/06 02:07:44 $
 *      @ref        sctp_init.c
 *
 *      @section    Intro(소개)
 *      - A_SCTP 프로세스를 초기화 하는 함수들
 *
 *      @section    Requirement
 *
*******************************************************************************/

/* INCLUDE ********************************************************************/

/* SYS HEADER */
#include <signal.h> 
/* LIB HEADER */
#include "commdef.h"
#include "common_stg.h"
#include "mems.h"
#include "cifo.h"
#include "gifo.h"
#include "nifo.h"
#include "loglib.h"
/* PRO HEADER */
#include "path.h"
#include "procid.h"
#include "sshmid.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */
#include "sctp_init.h"
#include "sctpstack.h"

/* VARIABLES ******************************************************************/
int	gdFinishFlag;

/* VARIABLES ( External ) *****************************************************/
extern stMEMSINFO		*gpMEMSINFO;
extern stCIFO			*gpCIFO;
extern int				gdStopFlag;
/* FUNCTION *******************************************************************/

/*******************************************************************************
 INITIALIZE SCTP BLOCK
*******************************************************************************/
int dInitSCTP()
{
	int		dRet;

	/* INIT SIGNAL */
	SetUpSignal();

	gpMEMSINFO = nifo_init_zone((U8*)"A_SCTP", SEQ_PROC_A_SCTP, FILE_NIFO_ZONE);
    if( gpMEMSINFO == NULL ){ 
        log_print(LOGN_CRI, LH"FAILED IN nifo_init, NULL", LT);
        return -1;
    }   
    
    //GIFO를 사용하기 위한 group 설정
    gpCIFO = gifo_init_group(FILE_CIFO_CONF, FILE_GIFO_CONF);
    if( gpCIFO == NULL ){
        log_print(LOGN_CRI, LH"FAILED IN gifo_init_group. cifo=%s, gifo=%s",
                LT, FILE_CIFO_CONF, FILE_GIFO_CONF);
        return -2;
    }   

	dRet = dInitSHMMMAB( S_SSHM_A_SCTP_ASSOC );
	if( dRet < 0 ) {
		log_print( LOGN_DEBUG, "[%s][%s.%d] dInitSHMMMAB FAIL RET:%d", 
							__FILE__, __FUNCTION__, __LINE__, dRet );
		return -3;
	}

	return 0;
}


/*******************************************************************************
 USERCONTROLLED SIGNAL
*******************************************************************************/
void UserControlledSignal(int sign)
{
    gdStopFlag = 0;
    gdFinishFlag = sign;
}


/*******************************************************************************
 IGNORE SIGNAL
*******************************************************************************/
void IgnoreSignal(int sign)
{
    if (sign != SIGALRM)
        log_print( LOGN_CRI, "UNWANTED SIGNAL IS RECEIVED, signal = %d", sign);
    signal(sign, IgnoreSignal);
}


/*******************************************************************************
 SETUP SIGNAL
*******************************************************************************/
void SetUpSignal()
{
    gdStopFlag = 1;

    /* WANTED SIGNALS   */
    signal(SIGTERM, UserControlledSignal);
    signal(SIGINT,  UserControlledSignal);
    signal(SIGQUIT, UserControlledSignal);

    /* UNWANTED SIGNALS */
    signal(SIGHUP,  IgnoreSignal);
    signal(SIGALRM, IgnoreSignal);
    signal(SIGPIPE, IgnoreSignal);
    signal(SIGPOLL, IgnoreSignal);
    signal(SIGPROF, IgnoreSignal);
    signal(SIGUSR1, IgnoreSignal);
    signal(SIGUSR2, IgnoreSignal);
    signal(SIGVTALRM, IgnoreSignal);
    signal(SIGCLD, SIG_IGN);
}

/*******************************************************************************
 FinishProgram
*******************************************************************************/
void FinishProgram()
{
    log_print( LOGN_CRI, "PROGRAM IS NORMALLY TERMINATED, Cause = %d", gdFinishFlag);
    exit(0);
}

/*
* $Log: sctp_init.c,v $
* Revision 1.2  2011/09/06 02:07:44  dcham
* *** empty log message ***
*
* Revision 1.1.1.1  2011/08/29 05:56:42  dcham
* NEW OAM SYSTEM
*
* Revision 1.4  2011/08/21 09:07:52  hhbaek
* Commit TAF/SRC/ *
*
* Revision 1.3  2011/08/17 07:24:32  dcham
* *** empty log message ***
*
* Revision 1.2  2011/08/05 02:38:56  uamyd
* A_SCTP modified
*
* Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
* init DQMS2
*
* Revision 1.4  2011/01/11 04:09:09  uamyd
* modified
*
* Revision 1.1.1.1  2010/08/23 01:12:59  uamyd
* DQMS With TOTMON, 2nd-import
*
* Revision 1.3  2009/06/29 13:25:06  dark264sh
* *** empty log message ***
*
* Revision 1.2  2009/05/27 14:24:48  dqms
* *** empty log message ***
*
* Revision 1.1  2009/05/27 07:38:13  dqms
* *** empty log message ***
*
* Revision 1.1  2009/05/13 11:38:41  upst_cvs
* NEW
*
* Revision 1.2  2008/03/19 04:56:45  doit1972
* *** empty log message ***
*
* Revision 1.1  2008/01/11 12:09:08  pkg
* import two-step by uamyd
*
* Revision 1.5  2007/06/07 08:04:20  doit1972
* MODIFY IP LIST INFO
*
* Revision 1.4  2007/05/11 08:34:25  doit1972
* ADD SYSTEM INFO INIT FUNC
*
* Revision 1.3  2007/05/10 06:35:56  doit1972
* ADD QID
*
* Revision 1.2  2007/05/04 12:33:31  doit1972
* MODIFY MSGQ ID
*
* Revision 1.1  2007/05/04 00:43:18  doit1972
* NEW FILE
*
*/
