/**<  
  @file		radius_main.c 
  
  @Id		$Id: radius_main.c,v 1.3 2011/09/05 09:01:46 dhkim Exp $ 
  @Author	$Author: dhkim $ 
  @Version	$Revision: 1.3 $ 
  @Date		$Date: 2011/09/05 09:01:46 $
  @ref
  @todo		

  @Section	Intro
	  - RADIUS Transaction을 관리하는 프로세스
 **/
#include <unistd.h>

// LIB
#include "typedef.h"
#include "loglib.h"
#include "verlib.h"
#include "mems.h"
#include "nifo.h"
#include "gifo.h"
#include "cifo.h"

#include "Analyze_Ext_Abs.h"

// PROJECT
#include "path.h"
#include "procid.h"
#include "sshmid.h"
#include "capdef.h"			/* Capture_Header_Msg */
#include "common_stg.h"

// .
#include "radius_init.h"
#include "radius_func.h"
#include "radius_decode.h"

/**
 * Declare variables
 */
stMEMSINFO      *pstMEMSINFO;
stCIFO          *gpCIFO;
stHASHOINFO     *pstHASHOINFO;
stTIMERNINFO    *pstTIMERNINFO;

st_TraceList    *pstTRACE;      /* TRACE */

S32             gACALLCnt = 0;

S32             giFinishSignal;
S32             giStopFlag;

S32             gTIMER_TRANS;

st_ippool       stIPPool;


/**<  
  main function

  @return		S32
 **/
S32 main()
{
	int					dRet;
	int					iLength;

	OFFSET				offset;

	UCHAR 				*pNODE, *pDATA;
	Capture_Header_Msg 	*pCAPHEAD;
	INFO_ETH 			*pINFOETH;

	st_ACCInfo 			stACCINFO;
	st_ACCInfo 			*pstACCINFO = &stACCINFO;

	char    vERSION[7] = "R3.0.0";

	log_init(S_SSHM_LOG_LEVEL, getpid(), SEQ_PROC_A_RADIUS, LOG_PATH"/A_RADIUS", "A_RADIUS");
	
	if((dRet = dInitRADIUSProc(&pstMEMSINFO, &pstHASHOINFO, &pstTIMERNINFO)) < 0) {
		log_print(LOGN_CRI, "[%s][%s.%d] dInitRADIUSProc dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);                                                           
		exit(0);                                                              
	}                                                                         

	if((dRet = set_version(S_SSHM_VERSION, SEQ_PROC_A_RADIUS, vERSION)) < 0 ) {
		log_print(LOGN_CRI, "SET_VERSION ERROR(RET=%d,IDX=%d,VER=%s)", dRet, SEQ_PROC_A_RADIUS, vERSION);
	}
	log_print(LOGN_CRI, "STARTING A_RADIUS(%s)", vERSION);

	/**<  MAIN LOOP **/	
	while ( giStopFlag )
	{
		timerN_invoke(pstTIMERNINFO);

		/**<  MSGQ READ **/	
		if ( (offset = gifo_read(pstMEMSINFO, gpCIFO, SEQ_PROC_A_RADIUS)) > 0)
		{
			pNODE = nifo_ptr(pstMEMSINFO, offset);

			pCAPHEAD = (Capture_Header_Msg *) nifo_get_value(pstMEMSINFO, CAP_HEADER_NUM, offset);
			pINFOETH = (INFO_ETH *) nifo_get_value(pstMEMSINFO, INFO_ETH_NUM, offset);
			pDATA = (UCHAR *) nifo_get_value (pstMEMSINFO, ETH_DATA_NUM, offset);

			if (pDATA == NULL) {
				log_print(LOGN_CRI, "[%s][%s.%d] nifo_get_value NULL ", __FILE__, __FUNCTION__, __LINE__); 
				/**<  DELETE NIFO NODE **/	
				nifo_node_delete(pstMEMSINFO, pNODE);
				continue;
			}
			else {
				log_print (LOGN_DEBUG, "D(%d) SRC IP:[%d.%d.%d.%d],%d DEST IP:[%d.%d.%d.%d],%d, payload size: %d", 
						pCAPHEAD->bRtxType,
						HIPADDR(pINFOETH->stIP.dwSrcIP), pINFOETH->stUDPTCP.wSrcPort,
						HIPADDR(pINFOETH->stIP.dwDestIP), pINFOETH->stUDPTCP.wDestPort,
						pINFOETH->stUDPTCP.wDataLen);

				iLength = pINFOETH->stUDPTCP.wDataLen;

				memset( pstACCINFO, 0x00, DEF_ACCINFO_SIZE );

				if ( (dRet = dAnalyze_RADIUS( pDATA, pstACCINFO, pINFOETH )) < 0 ) {
					log_print(LOGN_CRI, "[%s][%s.%d] dAnalyze_RADIUS() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet); 
					/**<  DELETE NIFO NODE **/	
					nifo_node_delete(pstMEMSINFO, pNODE);
					continue;
				}

				if ( (dRet = dProcRADIUS_Trans(pCAPHEAD, pINFOETH, pDATA, pstACCINFO)) < 0 ) {
					log_print(LOGN_CRI, "[%s][%s.%d] dProcRADIUS_Trans() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet); 
				}
			}
			/**<  DELETE RADIUS NODE **/	
			nifo_node_delete(pstMEMSINFO, pNODE);
		}
		else {
			usleep(0);
		}
	}

	FinishProgram();
	exit(0);
}


