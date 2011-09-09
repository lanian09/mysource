/** A.1 * File Include *************************************************************/
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>		/* GETPID(2) */

#include "path.h"		/* DATA_PATH */
#include "procid.h"
#include "sshmid.h"
#include "mems.h"
#include "cifo.h"

#include "loglib.h"
#include "verlib.h"

#define VERSION			"R3.0.0"

#include "fltmng_init.h"	/* dInitProc() */
#include "fltmng_func.h"	/* FLT_SHORT_CHECK, dGetNode(), dMakeSendMsg(), dMsgsnd(), do_while() */

/** B.1 * Definition of New Constants **********************************************/
/** B.1 * Definition of New Type ***************************************************/
/** C.1 * Declaration of Variables *************************************************/
stMEMSINFO	*gpMEMSINFO;
stCIFO		*gpCIFO;
time_t		g_tUpdate;
time_t		g_tInterval;

/** C.2 * Declaration of External Variables ****************************************/
/** D.1 * Definition of Functions **************************************************/

int main(int args, char *argv[])
{
	int			dRet;
	pst_MsgQ	pstMsg;
	U8			*pNODE;


	g_tInterval	= FLT_SHORT_CHECK;
	g_tUpdate	= time(0);


	log_init(S_SSHM_LOG_LEVEL, getpid(), SEQ_PROC_S_MNG, LOG_PATH"/S_MNG", "S_MNG");


	if( (dRet = dInitProc()) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dInitProc() dRet[%d]E",LT, dRet);
		exit(-1);
	}

	if( (dRet = set_version(S_SSHM_VERSION, SEQ_PROC_S_MNG, VERSION)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN set_version(%s) dRet[%d]E", LT, VERSION, dRet);
		exit(-2);
	}


	/*	프로세스 구동 직후 필터 정보를 요청한다	*/
	log_print(LOGN_DEBUG, LH"MAKE & SEND REQUEST. INTERVAL[%ld]", LT, g_tInterval);

	if( !dGetNode(&pNODE, &pstMsg) ){
		/* Node 를 획득한 뒤에 filter 를 요청할 수 있다. 재요청은.....--, 일단 reserved */
		if( (dRet = dMakeSendMsg(pstMsg)) >= 0){
			dMsgsnd(SEQ_PROC_CI_SVC, nifo_offset(gpMEMSINFO, pNODE));
		}
	}else{
		log_print(LOGN_WARN, LH"FAILED IN dGetNode(S_MNG) in Main", LT);
	}

	/*	DO while loop	*/
	do_while();

	log_print(LOGN_CRI, LH"END OF LOG :: S_MNG FINISHED!", LT);

	return 0;
}
