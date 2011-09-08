/***** A.1 * File Include *******************************/

/* SYS HEADER */
#include <string.h>		/* memcpy */
/* LIB HEADER */
#include "clisto.h"		/* U8 */
#include "filedb.h"		/* MAX_CH_COUNT */
#include "loglib.h"
#include "utillib.h"	/* util_makenid() */
/* PRO HEADER */
#include "msgdef.h"		/* st_MsgQ */
#include "sockio.h"		/* st_subsys */
#include "procid.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
#include "almstat.h"	/* st_almsts */
/* LOC HEADER */
#include "if_func.h"	/* dMsgsnd() */
#include "if_msgq.h"	/* dSendMsg() */

extern st_subsys stSubSys[MAX_CH_COUNT];
extern pst_NTAM	 fidb;


/*******************************************************************************
* MAKE COND BODY - FOR 'CHNNEL CONNECT_STATUS OF NTAF's'
*******************************************************************************/
int dMakeCondMsg( int dSysNo, unsigned int uiIP, int dIdx, time_t tWhen, unsigned char ucCurr, unsigned char ucOld )
{
	st_almsts    alm;
	pst_MsgQ     pstMsg;
	pst_MsgQSub  pstSub;
	U8		    *pNODE;
	
	alm.ucSysType = SYSTYPE_TAM;
	alm.ucSysNo   = (unsigned char)dSysNo; 
	alm.ucLocType = LOCTYPE_CHNL;
	alm.ucInvType = INVTYPE_CLIENT;
	alm.ucInvNo   = (unsigned char)dIdx;
	alm.ucAlmLevel    = ucCurr;
	alm.ucOldAlmLevel = ucOld;
	alm.llLoadVal = 0;
	alm.tWhen     = tWhen;
    alm.uiIPAddr  = uiIP;
	
	log_print(LOGN_INFO, "SEND TO COND MSG : [SYSNO]:[%d] [IDX]:[%d] [TWHEN]:[%ld] [IP]:[%u]",
			alm.ucSysNo, alm.ucInvNo, alm.tWhen, alm.uiIPAddr);

	if( dGetNode( &pNODE, &pstMsg ) < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN dGetNode(SI_SVC)",LT);
		return -1;
	}

	pstSub = (pst_MsgQSub)&pstMsg->llMType;
	pstSub->usType  = DEF_SYS;
	pstSub->usSvcID = SID_STATUS;
	pstSub->usMsgID = MID_CONSOL;
	
	util_makenid( SEQ_PROC_SI_SVC, &pstMsg->llNID );

	pstMsg->ucNTAMID  = dSysNo; 
	pstMsg->ucNTAFID  = 0; 
	pstMsg->llIndex   = 0;  /* INDEX */
	pstMsg->dMsgQID   = 0;
	pstMsg->usBodyLen = DEF_ALMSTS_SIZE;
	pstMsg->usRetCode = 0;

	memcpy( pstMsg->szBody, &alm, pstMsg->usBodyLen );


	if( dMsgsnd(SEQ_PROC_COND, pNODE) < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN dMsgsnd(to COND=%d)", LT, SEQ_PROC_COND);
		return -2;
	}
	
	return 0;
}

int dCheck_Channel(int dSysNo, int dFlag, unsigned int uiIP)
{
	if(stSubSys[dSysNo-1].uiIP == uiIP) {
		if( fidb->NTAFChnl[dSysNo - 1] != MASK ){
				if(dFlag == 0 ) {	/* DisConnect */
					log_print( LOGN_DEBUG, LH"CHECK CHANNEL : DEAD CHANNEL SYSNO=%d CONNECT-FLAG=%d IP=%s:%u",
						LT, dSysNo, dFlag, util_cvtipaddr(NULL,htonl(uiIP)),uiIP);
					fidb->NTAFChnl[dSysNo - 1] = CRITICAL;
				} else if(dFlag == 1 ) {
					log_print( LOGN_DEBUG, LH"CHECK CHANNEL : ALIVE CHANNEL SYSNO=%d CONNECT-FLAG=%d IP=%s:%u",
						LT, dSysNo, dFlag, util_cvtipaddr(NULL,htonl(uiIP)),uiIP);
					fidb->NTAFChnl[dSysNo - 1] = NORMAL;
				} else
					log_print( LOGN_DEBUG, LH"CHECK CHANNEL : UNKOWN CHANNEL SYSNO=%d CONNECT-FLAG=%d IP=%s:%u",
						LT, dSysNo, dFlag, util_cvtipaddr(NULL,htonl(uiIP)),uiIP);
		}
	} else {
		log_print( LOGN_CRI, LH"CHECK CHANNEL : MISMATCH SYSNO=%d CONNECT-FLAG=%d SOCKIP=%s:%u<->SubSys=%u",
			LT, dSysNo, dFlag, util_cvtipaddr(NULL,htonl(uiIP)),uiIP, stSubSys[dSysNo - 1].uiIP);
		return -1;
	}

	return 0;
}
