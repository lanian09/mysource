/**
 * Include Headers
 */
#include <unistd.h>

// TOP 
#include "common_stg.h"
#include "procid.h"
#include "sshmid.h"
#include "commdef.h"
#include "path.h"
#include "capdef.h"

// LIB
#include "loglib.h"
#include "verlib.h"
#include "mems.h"
#include "cifo.h"
#include "nifo.h"
#include "gifo.h"

// TAF
#include "Analyze_Ext_Abs.h"	/* INFO_ETH */
#include "mmdblist_ftp.h"

/**
 * Define constants
 */
#define MAX_MSG_CNT		100000 	/* 반복을 통해 처리하는 메시지큐 최대 개수    */

/**
 *  Declare variables
 */
int     			JiSTOPFlag;
int     			FinishFlag;
time_t				g_lCurTime;
stMEMSINFO			*pstMEMSINFO;
stCIFO				*gpCIFO;
char                vERSION[7] = "R3.0.0";

/**
 * Declare func.
 */
int dTimeOutSess();

extern int dInitProc();
extern int dSvcProcess( TCP_INFO *pstTCPINFO, Capture_Header_Msg *pCAPHEAD, U8 *pDATA );
extern void FinishProgram();
extern int dTermTCP( PSESS_KEY pstKey );

/**
 *	Implement func.
 */
/*******************************************************************************
 MAIN
*******************************************************************************/
int main()
{
	int					dRet, i, dCheckTime;
	OFFSET          	offset;
	U8					*pNode;
    U8              	*pDATA;
	U8					*pBODYDATA;
	Capture_Header_Msg	*pCAPHEAD;
	INFO_ETH            *pINFOETH;
    TCP_INFO        	*pTCPINFO;

	log_init(S_SSHM_LOG_LEVEL, getpid(), SEQ_PROC_A_FTP, LOG_PATH"/A_FTP", "A_FTP");

	/* Process Initial */
	dRet = dInitProc();
	if(dRet < 0) {
		log_print( LOGN_CRI, "[ERR] [PROCESS INIT FAIL] [RET]:[%d]", dRet);
		exit(0);
	}

	dRet = set_version(S_SSHM_VERSION, SEQ_PROC_A_FTP,vERSION);
    if ( dRet < 0 )
        log_print(LOGN_DEBUG,"set_version error(ret=%d,idx=%d,ver=%s)\n",dRet,SEQ_PROC_A_FTP,vERSION);

    log_print( LOGN_CRI, "[A_FTP] PROCESS STARTED : VERSION[%s]", vERSION );

	dCheckTime = time(0);

	while(JiSTOPFlag)
	{
		g_lCurTime = time(0);

		if ( dCheckTime!= g_lCurTime ) {
			dTimeOutSess();
			dCheckTime = g_lCurTime;
		}

		for( i=0; i<MAX_MSG_CNT; i++ ) {
			if((offset = gifo_read(pstMEMSINFO, gpCIFO, SEQ_PROC_A_FTP)) > 0) {
				pNode = nifo_ptr(pstMEMSINFO, offset);
				pCAPHEAD = (Capture_Header_Msg *)nifo_get_value(pstMEMSINFO, CAP_HEADER_NUM, offset);
				pINFOETH    = (INFO_ETH *)nifo_get_value(pstMEMSINFO, INFO_ETH_NUM, offset);
				pTCPINFO = (TCP_INFO *)nifo_get_value(pstMEMSINFO, TCP_INFO_DEF_NUM, offset);
            	pDATA = (U8 *)nifo_get_value(pstMEMSINFO, ETH_DATA_NUM, offset);

				pBODYDATA = &pDATA[pTCPINFO->uiSOffset];

				dRet = dSvcProcess( pTCPINFO, pCAPHEAD, pBODYDATA );
				if( dRet < 0 )
					log_print( LOGN_INFO, "ERROR IN dSvcProcess dRet:%d", dRet );

				nifo_node_delete(pstMEMSINFO, pNode);
				
			}
			else {
				usleep(0);
				break;
			}
		} /* FOR END (Msg-Q) */

		usleep(0);
	} /* while-loop end (main) */

	FinishProgram();
	exit(0);
}

int dTimeOutSess()
{
	SESS_KEY	stFKEY, stLKEY;
	SESS_DATA	*pFTP;
	int 		i, dCount;

	memset( &stFKEY, 0x00, sizeof(SESS_KEY) );
	memset( &stLKEY, 0xFF, sizeof(SESS_KEY) );

	dCount = pstSessTbl->uiSessCount;

	for ( i=0; i<dCount; i++ )
    {
        pFTP = pstSelectMMDB( &stFKEY, &stLKEY );
		if ( pFTP == 0x0 )
        {
            log_print( LOGN_DEBUG, "MMDB BROKEN [SESS COUNT TOO MUCH [%d/%d]", dCount, i );
            break;
        }

		stFKEY = pFTP->stKey;

		if ( pFTP->dTimeOutTime==0 )
			continue;

		if ( pFTP->dTimeOutTime < g_lCurTime )
			dTermTCP( &pFTP->stKey );
	}

	return 1;
}

