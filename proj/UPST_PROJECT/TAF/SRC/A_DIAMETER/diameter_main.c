/**<  
  @file		diameter_main.c 
  
  @Id		$Id: diameter_main.c,v 1.3 2011/09/07 06:40:42 dcham Exp $ 
  @Author	$Author: dcham $ 
  @Version	$Revision: 1.3 $ 
  @Date		$Date: 2011/09/07 06:40:42 $
  @ref
  @todo		CSCF이외의 구간 메세지 추가

  @Section	Intro
  - DIAMETER Transaction을 관리하는 프로세스
	
 **/

/**
 * Include headers
 */
#include <unistd.h>

// LIB
#include "typedef.h"
#include "mems.h"
#include "cifo.h"
#include "nifo.h"
#include "gifo.h"
#include "hasho.h"
#include "loglib.h"
#include "verlib.h"
#include "timerN.h"

// OAM

// TOP INCLUDE
#include "common_stg.h"
#include "path.h"
#include "commdef.h"
#include "procid.h"
#include "sshmid.h"

// TAF INCLUDE
#include "capdef.h"


// TAF headers
#include "Analyze_Ext_Abs.h"

// .
#include "diameter_comm.h"

/**
 * Declare variables
 */
stMEMSINFO		*pstMEMSINFO;
stCIFO			*gpCIFO;
stHASHOINFO		*pstHASHOINFO;
stTIMERNINFO	*pstTIMERNINFO;

st_TraceList	*pstTRACE;		/* TRACE */

S32				gACALLCnt = 0;

S32				giFinishSignal;
S32				giStopFlag;

S32				gTIMER_TRANS = 1;

/**
 * Declare functions
 */
extern int dump_DebugString(char *debug_str, char *s, int len);
extern S32 dInitDIAMETERProc(stMEMSINFO **pstMEMS, stHASHOINFO **pstHASHO, stTIMERNINFO **pstTIMER);
extern int dProcDIAMETER_Trans( Capture_Header_Msg *pCAPHEAD, INFO_ETH *pINFOETH, UCHAR *pDATA, UCHAR *pETHDATA, st_DiameterInfo *pstDIAMETERINFO);
extern void FinishProgram(void);

/**<  
  main function

  @return		S32
  		
 **/
S32 main()
{
	int					dRet, i;
	int					iLength;

	OFFSET				offset;

	UCHAR 				*pNODE, *pDATA, *pETHData;
	Capture_Header_Msg 	*pCAPHEAD;
	INFO_ETH 			*pINFOETH;

	st_DiameterInfo 	stDIAMETERINFO;
	st_DiameterInfo 	*pstDIAMETERINFO = &stDIAMETERINFO;

	char    vERSION[7] = "R3.0.0";


	log_init(S_SSHM_LOG_LEVEL, getpid(), SEQ_PROC_A_DIAMETER, LOG_PATH"/A_DIAMETER", "A_DIAMETER");

	if((dRet = dInitDIAMETERProc(&pstMEMSINFO, &pstHASHOINFO, &pstTIMERNINFO)) < 0) 
	{
		log_print(LOGN_CRI, "[%s][%s.%d] dInitProc dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);                                                           
		exit(0);                                                              
	}                                                                         

	if((dRet = set_version(S_SSHM_VERSION, SEQ_PROC_A_DIAMETER, vERSION)) < 0 ) {
		log_print(LOGN_CRI, "SET_VERSION ERROR(RET=%d,IDX=%d,VER=%s)", dRet, SEQ_PROC_A_DIAMETER, vERSION);
	}
	log_print(LOGN_CRI, "STARTING A_DIAMETER (%s)", vERSION);

	/**<  MAIN LOOP **/	
	while ( giStopFlag )
	{
		timerN_invoke(pstTIMERNINFO);

		for( i=0; i<DEF_MSGREAD_CNT; i++ ) {
			/**<  READ **/	
			if ( (offset = gifo_read(pstMEMSINFO, gpCIFO, SEQ_PROC_A_DIAMETER)) > 0) {
				pNODE 		= nifo_ptr(pstMEMSINFO, offset);
				pCAPHEAD 	= (Capture_Header_Msg *) nifo_get_value(pstMEMSINFO, CAP_HEADER_NUM, offset);
				pINFOETH 	= (INFO_ETH *) nifo_get_value(pstMEMSINFO, INFO_ETH_NUM, offset);
				pDATA 		= (UCHAR *)nifo_get_value(pstMEMSINFO, SCTP_DATA_NUM, offset);
				pETHData	= (UCHAR *)nifo_get_value(pstMEMSINFO, ETH_DATA_NUM, offset);

				if (pDATA == NULL) {
					log_print(LOGN_CRI, "[%s][%s.%d] nifo_get_value NULL ", __FILE__, __FUNCTION__, __LINE__); 
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
				
					memset(pstDIAMETERINFO, 0, DEF_DIAMETERINFO_SIZE);

//					dump_DebugString("DIAMETER", pDATA, iLength);
					if ( (dRet = dAnalyze_Diameter(pDATA, iLength, &stDIAMETERINFO)) < 0 ) {
						log_print(LOGN_INFO, "[%s][%s.%d] dDecode_DIAMETER_protocol dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet); 
						nifo_node_delete(pstMEMSINFO, pNODE);
						continue;
					}
					else if ( dRet == 2) {
						log_print(LOGN_INFO, "DISCARD MESSAGE TYPE=%d", pstDIAMETERINFO->stDiameterHdr.uiCmdCode);
						nifo_node_delete(pstMEMSINFO, pNODE);
						continue;
					}
					log_print(LOGN_INFO," ::: PUBLIC ID=%s", pstDIAMETERINFO->szPublicID);
					dRet = dProcDIAMETER_Trans(pCAPHEAD, pINFOETH, pDATA, pETHData,  pstDIAMETERINFO);
				}

				/**<  DELETE NIFO NODE **/
				nifo_node_delete(pstMEMSINFO, pNODE);
			}
			else {
				usleep(0);
				break;
			}
		}
	}

	FinishProgram();
	exit(0);
}


