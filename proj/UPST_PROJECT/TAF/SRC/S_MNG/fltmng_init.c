/** A.1 * File Include *************************************************************/
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#include "filter.h"			/* flt_info */

#include "path.h"			/* DATA_PATH */
#include "commdef.h"
#include "procid.h"
#include "sshmid.h"
#include "mems.h"
#include "cifo.h"
#include "nifo.h"
#include "gifo.h"

#include "filedb.h"

#include "loglib.h"
#include "ipclib.h"

#include "common_stg.h"

#include "fltmng_init.h"
#include "fltmng_func.h"	/* dReadTimerFile() */
#include "fltmng_file.h"	/* Init_Service_Conf() */

/** B.1 * Definition of New Constants **********************************************/
/** B.1 * Definition of New Type ***************************************************/

/** C.1 * Declaration of Variables *************************************************/
/** C.2 * Declaration of External Variables ****************************************/
extern stMEMSINFO	    	*gpMEMSINFO;
extern stCIFO				*gpCIFO;
extern st_TraceList			*trace_tbl;
extern pst_keepalive_taf 	keepalive;
extern st_Flt_Info			*flt_info;
extern int gdStopFlag;

/** D.1 * Definition of Functions **************************************************/
int dInitProc(void)
{
	int dRet;

	if((gpMEMSINFO = nifo_init_zone((U8*)"S_MNG", SEQ_PROC_S_MNG, FILE_NIFO_ZONE)) == NULL ){
        log_print(LOGN_CRI, LH"FAILED IN nifo_init_zone NULL", LT);
        return -1;
    }

    if((gpCIFO = gifo_init_group(FILE_CIFO_CONF, FILE_GIFO_CONF)) == NULL ){
        log_print(LOGN_CRI, LH"FAILED IN gifo_init_group, cifo=%s, gifo=%s",
            LT, FILE_CIFO_CONF, FILE_GIFO_CONF);
        return -2;
    }

	if( shm_init(S_SSHM_TRACE_INFO, sizeof(st_TraceList), (void**)&trace_tbl) < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN shm_init(TRACE_TBL=%d)", LT, S_SSHM_TRACE_INFO);
		return -3;
	}

	if( shm_init(S_SSHM_KEEPALIVE, DEF_KEEPALIVE_TAF_SIZE, (void**)&keepalive) < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN shm_init(KEEPALIVE=%d)", LT, S_SSHM_KEEPALIVE);
		return -4;
	}

	if( shm_init(S_SSHM_FLT_INFO, DEF_FLT_INFO_SIZE, (void**)&flt_info) < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN shm_init(FLT_INFO=%d)", LT, S_SSHM_FLT_INFO);
		return -5;
	}

	/* timer setting */
	if( (dRet = dReadTimerFile(&flt_info->stTimerInfo)) < 0){
		log_print(LOGN_CRI, LH"ERROR IN dReadTimerFile() dRet[%d]", LT, dRet);
	}
	
	Init_Signal();

	if( (dRet = dInit_Info()) < 0) {
		log_print(LOGN_CRI, LH"ERROR IN dInit_Info() dRet[%d]", LT, dRet);
		return -6;
	}

	flt_info->stHdrLogShm.sHdrCapFlag	= HEADER_LOG_CAPTURE; /* TCP HDR Option Pre Setting*/
	return 0;
}


void Init_Signal(void)
{
	signal(SIGTERM, FinishProgram);
	signal(SIGALRM, IgnoreSignal);
}


void IgnoreSignal(int dSigNo)
{
	if( dSigNo != SIGALRM )
    	log_print(LOGN_DEBUG, "SIGNAL IGNORED. SIGNAL=[%d]", dSigNo);

    signal(dSigNo, IgnoreSignal);
}


void FinishProgram(int dSigNo)
{
    log_print(LOGN_CRI, "PRAGRAM TERMINATED. SIGNAL=[%d]", dSigNo);

	gdStopFlag = 1;
}

int dInit_Info(void)
{
	int		dRet;

	if( (dRet = dInitSysConfig()) < 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN dInitSysConfig() dRet[%d]E", __FILE__, __FUNCTION__, __LINE__, dRet);
		return -1;
	}

	if( (dRet = dInitLogLvl()) < 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN dInitLogLvl() dRet[%d]E", __FILE__, __FUNCTION__, __LINE__, dRet);
		return -2 ;
	}

	/* */
	Init_Service_Conf();

	return 0;
}
