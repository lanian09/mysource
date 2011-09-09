/** A. FILE INCLUSION *********************************************************/

/* SYS HEADER */
#include <errno.h>		/* errno */
/* LIB HEADER */
#include "loglib.h"
#include "utillib.h"	/* pst_NID */
/* PRO HEADER */
#include "msgdef.h"		/* st_MsgQ */
#include "sockio.h"		/* MAGIC_NUMBER, pst_NTAFTHeader */
#include "mmcdef.h"		/* SID_MML, SID_FLT, SID_CHKRES */
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */
#include "if_func.h"	/* dMsgrcv(), NO_MSG */
#include "if_msgq.h"

/** B.1* DEFINITION OF NEW CONSTANTS **********************/
/** C.1* DEFINITION OF NEW TYPES **************************/
/** D.1* DECLARATION OF VARIABLES *************************/
extern int writeSocket(int dSocket, void *buffer, int slen);	//sock.c

int dIsRcvedMessage(pst_MsgQ pstMsg)
{
	
	int	dRet;
	if( (dRet = dMsgrcv( &pstMsg )) < 0 ){
		if( dRet != -1 ){
			log_print(LOGN_CRI, LH"FAILED IN dMsgrcv(CI_SVC)",LT);
		}
		return NO_MSG;
	}
	return 1;

} /*** end of dIsRcvedMessage ***/

int dSndMsgProc(int dSocket, pst_MsgQ pstMsgQ)
{
	pst_MsgQSub		    pstMsgQSub;
	pst_NID				pstNID;
	pst_NTAFTHeader		pstHeader;
	unsigned short		usType, size;
	unsigned char		ucSvcID, ucMsgID;
	unsigned char		szBuf[1024 * 10];
	int					dRet;

	pstMsgQSub = (pst_MsgQSub)&pstMsgQ->llMType;
	pstNID	= (st_NID *)&pstMsgQ->llNID;

	usType	= pstMsgQSub->usType;
	ucSvcID	= pstMsgQSub->usSvcID;
    ucMsgID	= pstMsgQSub->usMsgID;

	switch(usType)
	{
		case DEF_SYS:
			pstHeader					= (pst_NTAFTHeader)szBuf;
			size						= pstMsgQ->usBodyLen;
			pstHeader->ucSvcID			= ucSvcID;
			pstHeader->ucMsgID			= ucMsgID;
			pstHeader->ucNTAFID			= pstMsgQ->ucNTAFID;
			pstHeader->llMagicNumber	= MAGIC_NUMBER;
			pstHeader->usTotlLen		= size+NTAFT_HEADER_LEN;
			pstHeader->usBodyLen		= pstMsgQ->usBodyLen;
			pstHeader->llIndex			= pstMsgQ->llIndex;

			log_print(LOGN_INFO, "dSndMsgProc >>> [PID]:[%d] [TOT]:[%d] [BODY]:[%d] [SYSNO]:[%d] [SID]:[%d] [MID][%d]",
				pstMsgQ->ucProID, pstHeader->usTotlLen, pstHeader->usBodyLen, pstHeader->ucNTAFID, ucSvcID, ucMsgID);

			switch(ucSvcID)
			{
				case SID_MML:
					log_print(LOGN_INFO, "SEND MML MSG TO SI_SVC");
				case SID_STATUS:
				case SID_CHKRES:
					memcpy(&szBuf[NTAFT_HEADER_LEN], &pstMsgQ->szBody[0], size);
					break;

				case SID_FLT:
					break;

				default:
					log_print(LOGN_INFO, "INVALID SID[%d]", ucSvcID);
					return 0;
			}

			dRet = writeSocket(dSocket, (char*)szBuf, pstHeader->usTotlLen);
			if(dRet < 0)
				return -1;

			break;

		default:
			log_print(LOGN_DEBUG,"[dRecvToSvcProc] [TYPE][%d] [ERROR]", usType);
			return 0;
	}

	return 0;
} /*** end of dSndMsgProc ***/

