/**<	
	@file		$file $
	
	@Id		$Id: l2tp_main.c,v 1.2 2011/09/05 07:33:22 dhkim Exp $ 
	@Author	$Author: dhkim $ 
	@Version	$Revision: 1.2 $ 
	@Date		$Date: 2011/09/05 07:33:22 $
	@ref
	@todo		

	@Section	Intro
		- L2TP Transaction을 관리하는 프로세스
 **/
#include <unistd.h>

// LIB
#include "typedef.h"
#include "loglib.h"
#include "verlib.h"
#include "mems.h"
#include "memg.h"
#include "cifo.h"
#include "gifo.h"
#include "timerN.h"

#include "Analyze_Ext_Abs.h"	/* st_L2TP_INFO */


// PROJECT
#include "path.h"
#include "procid.h"
#include "sshmid.h"
#include "capdef.h"
#include "common_stg.h"

// .
#include "l2tp_init.h"
#include "l2tp_func.h"


/**
 * Declare variables
 */
stMEMSINFO			*pstMEMSINFO;
stCIFO				*gpCIFO;

S32					giFinishSignal;
S32					giStopFlag;

extern stTIMERNINFO *pstTIMERNINFO;			/* Call Sess Timer */
extern stTIMERNINFO *pstCTIMERNINFO;		/* Control Sess Timer */
extern stTIMERNINFO *pstTTIMERNINFO;		/* Tunnel ID Timer */
extern stTIMERNINFO *pstSTIMERNINFO;		/* Session ID Timer */

extern int dAnalyze_L2TP(PUCHAR pBuf, int wSize, st_L2TP_INFO *pInfo);

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
	UINT 				dHeaderLen;

	st_L2TP_INFO		stL2TPInfo;
	st_L2TP_INFO		*pstL2TPInfo = &stL2TPInfo;

	char	vERSION[7] = "R3.0.0";

	log_init(S_SSHM_LOG_LEVEL, getpid(), SEQ_PROC_A_L2TP, LOG_PATH"/A_L2TP", "A_L2TP");
	
	if((dRet = dInitL2TPProc(&pstMEMSINFO) ) < 0) {
		log_print(LOGN_CRI, "[%s][%s.%d] dInitRADIUSProc dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);															
		exit(0);																
	}																		 

	if((dRet = set_version(S_SSHM_VERSION, SEQ_PROC_A_L2TP, vERSION)) < 0 ) {
		log_print(LOGN_CRI, "SET_VERSION ERROR(RET=%d,IDX=%d,VER=%s)", dRet, SEQ_PROC_A_L2TP, vERSION);
	}
	log_print(LOGN_CRI, "STARTING A_L2TP(%s)", vERSION);

	/**<	MAIN LOOP **/	
	while ( giStopFlag )
	{
		timerN_invoke(pstTIMERNINFO);
		timerN_invoke(pstCTIMERNINFO);
		timerN_invoke(pstSTIMERNINFO);
		timerN_invoke(pstTTIMERNINFO);

		/**<	MSGQ READ **/
		if ( (offset = gifo_read(pstMEMSINFO, gpCIFO, SEQ_PROC_A_L2TP)) > 0)
		{
			pNODE = nifo_ptr(pstMEMSINFO, offset);

			pCAPHEAD = (Capture_Header_Msg *) nifo_get_value(pstMEMSINFO, CAP_HEADER_NUM, offset);
			pINFOETH = (INFO_ETH *) nifo_get_value(pstMEMSINFO, INFO_ETH_NUM, offset);
			pDATA = (UCHAR *) nifo_get_value (pstMEMSINFO, ETH_DATA_NUM, offset);

			if (pDATA == NULL) {
				log_print(LOGN_CRI, "[%s][%s.%d] nifo_get_value NULL ", __FILE__, __FUNCTION__, __LINE__); 
				/**<	DELETE NIFO NODE **/	
				nifo_node_delete(pstMEMSINFO, pNODE);
				continue;
			}
			else {
				log_print (LOGN_DEBUG, "D(%d) SRC IP:[%d.%d.%d.%d:%d DEST IP:[%d.%d.%d.%d:%d, payload size: %d", 
						pCAPHEAD->bRtxType,
						HIPADDR(pINFOETH->stIP.dwSrcIP), pINFOETH->stUDPTCP.wSrcPort,
						HIPADDR(pINFOETH->stIP.dwDestIP), pINFOETH->stUDPTCP.wDestPort,
						pINFOETH->stUDPTCP.wDataLen);

				dHeaderLen = 14 + pINFOETH->stIP.wIPHeaderLen + pINFOETH->stUDPTCP.wHeaderLen;
				iLength = pINFOETH->stUDPTCP.wDataLen;

				memset( pstL2TPInfo, 0x00, DEF_L2TPINFO_SIZE );
				if ( (dRet = dAnalyze_L2TP( pDATA + dHeaderLen, iLength, pstL2TPInfo)) < 0 ) {
					log_print(LOGN_CRI, "[%s][%s.%d] Analyze_L2TP() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet); 
					/**<	DELETE NIFO NODE **/
					nifo_node_delete(pstMEMSINFO, pNODE);
					continue;
				}
#ifdef __DEBUG__
				printL2TPInfo(pstL2TPInfo);
#endif
				if ( (dRet = dProcL2TP_Trans(pCAPHEAD, pINFOETH, pDATA, pstL2TPInfo)) < 0 ) {
					log_print(LOGN_CRI, "[%s][%s.%d] dProcRADIUS_Trans() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet); 
				}
			}
			/**<	DELETE L2TP NODE **/	
			nifo_node_delete(pstMEMSINFO, pNODE);
		}
		else {
			usleep(0);
		}
	}

	FinishProgram();
	exit(0);
}


