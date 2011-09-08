/** A. FILE INCLUSION *********************************************************/
#include <unistd.h>

// LIB
#include "typedef.h"
#include "loglib.h"
#include "verlib.h"
#include "mems.h"
#include "nifo.h"
#include "cifo.h"
#include "gifo.h"
#include "hasho.h"
#include "timerN.h"

#include "Analyze_Ext_Abs.h"

// PROJECT
#include "path.h"
#include "procid.h"
#include "msgdef.h"
#include "sshmid.h"
#include "filter.h"
#include "common_stg.h"
#include "capdef.h"

#include "adns_init.h"
#include "adns_serv.h"

/** B. DEFINITION OF NEW CONSTANTS ********************************************/
#define MAX_MSG_CNT		100000

/** C. DEFINITION OF NEW TYPES ************************************************/

/** D. DECLARATION OF VARIABLES ***********************************************/
int				JiSTOPFlag;
int				FinishFlag;
int				guiTimerValue;

time_t			g_lCurTime;

stMEMSINFO		*pMEMSINFO;
stCIFO			*gpCIFO;
stHASHOINFO		*pDNSSESSHASH;
stTIMERNINFO	*pTIMERNINFO;

st_Flt_Info		*flt_info;

char				vERSION[7] = "R3.0.0";

/* VERSION INFORMATION : ADDED BY POOPEE 20040127 */

/** E.1 DEFINITION OF FUNCTIONS ***********************************************/

/** E.2 DEFINITION OF FUNCTIONS ***********************************************/

/*******************************************************************************
 MAIN
*******************************************************************************/
int main()
{
	int					dRet, i, dCheckTime;
	OFFSET				offset;
	U8					*pNode;
	U8					*pDATA;
	Capture_Header_Msg	*pCAPHEAD;
	INFO_ETH			*pINFOETH;
	TCP_INFO			*pTCPINFO;

	log_init(S_SSHM_LOG_LEVEL, getpid(), SEQ_PROC_A_DNS, LOG_PATH"/A_DNS", "A_DNS");

	/* Process Initial */
	dRet = dInitDNSProc();
	if(dRet < 0) {
		log_print( LOGN_CRI, "[ERR] [PROCESS INIT FAIL] [RET]:[%d]", dRet);
		exit(0);
	}

	dRet = set_version( S_SSHM_VERSION, SEQ_PROC_A_DNS, vERSION );
	if ( dRet < 0 )
		log_print(LOGN_DEBUG,"set_version error(ret=%d,idx=%d,ver=%s)\n", dRet, SEQ_PROC_A_DNS, vERSION);

	log_print( LOGN_CRI, "[A_DNS] PROCESS STARTED : VERSION[%s]", vERSION );

	dCheckTime = time(0);

	while(JiSTOPFlag)
	{
		guiTimerValue = flt_info->stTimerInfo.usTimerInfo[PI_DNS_TIMEOUT];

		timerN_invoke(pTIMERNINFO);

		g_lCurTime = time(0);

		for( i=0; i<MAX_MSG_CNT; i++ ) {
			if((offset = gifo_read(pMEMSINFO, gpCIFO, SEQ_PROC_A_DNS)) > 0) {
				pNode 		= nifo_ptr(pMEMSINFO, offset);
				pCAPHEAD 	= (Capture_Header_Msg *)nifo_get_value(pMEMSINFO, CAP_HEADER_NUM, offset);
				pINFOETH 	= (INFO_ETH *)nifo_get_value(pMEMSINFO, INFO_ETH_NUM, offset);
				pTCPINFO 	= (TCP_INFO *)nifo_get_value(pMEMSINFO, TCP_INFO_DEF_NUM, offset);
				pDATA 		= (U8 *)nifo_get_value(pMEMSINFO, ETH_DATA_NUM, offset);

				dRet = dDNSProcess( pCAPHEAD, pINFOETH, pTCPINFO, pDATA, offset );
				if( dRet < 0 )
					log_print( LOGN_INFO, "ERROR IN dSvcProcess dRet:%d", dRet );

				nifo_node_delete(pMEMSINFO, pNode);
			}
			else {
				usleep(0);
				break;
			}
		} /* FOR END (Msg-Q) */
	} /* while-loop end (main) */

	FinishProgram();
	exit(0);
}
