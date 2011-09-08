/** A.1* FILE INCLUSION ***********************************/
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/ipc.h>
#include <errno.h>
#include <string.h>

#include "path.h"
#include "commdef.h"
#include "procid.h"
#include "sshmid.h"
#include "mems.h"
#include "cifo.h"
#include "nifo.h"
#include "gifo.h"

#include "sockio.h"		/* st_subsys_mng */

#include "loglib.h"
#include "filelib.h"
#include "ipclib.h"
#include "silog_init.h"
/** B.1* DEFINITION OF NEW CONSTANTS **********************/
/** C.1* DEFINITION OF NEW TYPES **************************/
/** D.1* DECLARATION OF VARIABLES *************************/
extern int  		JiSTOPFlag;
extern int  		Finishflag;

extern stMEMSINFO	*pMEMSINFO;
extern stCIFO		*gpCIFO;

extern st_subsys_mng *stSubSys;

extern int			gARPPICnt;
/** E.1* DEFINITION OF FUNCTIONS **************************/
/** E.2* DEFINITION OF FUNCTIONS **************************/

int dInitProc()
{

    SetUpSignal();

	if((pMEMSINFO = nifo_init_zone((U8*)"SI_LOG", SEQ_PROC_SI_LOG, FILE_NIFO_ZONE)) == NULL) {
		log_print(LOGN_CRI, "F=%s:%s.%d nifo_init_zone NULL", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}

	if((gpCIFO = gifo_init_group(FILE_CIFO_CONF, FILE_GIFO_CONF)) == NULL ){
        log_print(LOGN_CRI, LH"FAILED IN gifo_init_group, cifo=%s, gifo=%s",
            LT, FILE_CIFO_CONF, FILE_GIFO_CONF);
        return -2;
    }

	gARPPICnt = get_block_num(FILE_MC_INIT, "A_RPPI");
    log_print(LOGN_INFO, "INIT., A_RPPI ProcCount[%d]", gARPPICnt);

	if( shm_init(S_SSHM_SUBSYS, sizeof(st_subsys_mng), (void**)&stSubSys) < 0 ){
        log_print(LOGN_CRI, "FAILED IN shm_init(SUB_SYS=%d)", S_SSHM_SUBSYS);
        return -3;
    }

    return 0;
} /* end of dInitProc */

void SetUpSignal()
{
	JiSTOPFlag = 1;

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

	log_print(LOGN_DEBUG, 
	"SetUpSignal : SIGNAL HANDLER WAS INSTALLED[%d]", JiSTOPFlag);

    return;
} /* end of SetUpSignal */

void UserControlledSignal(int sign)
{
    JiSTOPFlag = 0;
    Finishflag = sign;
} /* end of UserControlledSignal */

void FinishProgram()
{
    log_print(LOGN_CRI, 
	"FinishProgram : PROGRAM IS NORMALLY TERMINATED, Cause = %d", Finishflag);
    exit(0);	
} /* end of FinishProgram */

void IgnoreSignal(int sign)
{
    if (sign != SIGALRM)
        log_print(LOGN_CRI, 
		"IgnoreSignal : UNWANTED SIGNAL IS RECEIVED, signal = %d", sign);
    signal(sign, IgnoreSignal);
} /* end of IgnoreSignal */
