
/**
 *	Include headers
 */
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>

// TOP
#include "common_stg.h"
#include "procid.h"
#include "commdef.h"
#include "path.h"

// LIB
#include "mems.h"	/* stMEMSINFO */
#include "gifo.h"	/* gifo_init_goup() */
#include "cifo.h"	/* stCIFO */
#include "nifo.h"	/* nifo_init_zone() */
#include "loglib.h"

// .
#include "ci_log_init.h"

/**
 *	Declare var.
 */
extern stCIFO	*gpCIFO;
extern int		g_JiSTOPFlag;
extern int		g_FinishFlag;
extern int		g_dServerSocket;

/**
 *	Implement func.
 */

S32 dInitCI_LOG(stMEMSINFO **pMEMSINFO)
{
	SetUpSignal();

	*pMEMSINFO = nifo_init_zone((U8*)"CI_LOG", SEQ_PROC_CI_LOG, FILE_NIFO_ZONE);
    if( *pMEMSINFO == NULL ){
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

	return 0;
}

void UserControlledSignal(int sign)
{
    g_JiSTOPFlag = 0;
    g_FinishFlag = sign;

	FinishProgram();
}

void FinishProgram()
{
	close( g_dServerSocket );
    log_print( LOGN_CRI, "PROGRAM IS NORMALLY TERMINATED, Cause = [%d]", g_FinishFlag );
    exit(0);
}

void IgnoreSignal(int sign)
{
    if( sign != SIGALRM )
        log_print( LOGN_WARN, "UNWANTED SIGNAL IS RECEIVED, signal = %d", sign );

    signal( sign, IgnoreSignal );
}

void SetUpSignal()
{
    g_JiSTOPFlag = 1;

    /* WANTED SIGNALS */
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

    log_print( LOGN_DEBUG, "SIGNAL HANDLER WAS INSTALLED" );
}
