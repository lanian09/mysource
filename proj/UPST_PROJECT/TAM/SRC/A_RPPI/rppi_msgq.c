#include <unistd.h>		/* USLEEP(3) */

// LIB
#include "mems.h"
#include "hasho.h"
#include "cifo.h"
#include "nifo.h"
#include "gifo.h"
#include "loglib.h"
#include "utillib.h"	/* util_makenid() */
#include "timerN.h"		/* stTIMERNINFO */

// PROJECT
#include "procid.h"		/* SEQ_PROC_XX */
#include "msgdef.h"		/* st_MsgQ */
#include "common_stg.h"

// TAM
#include "watch_mon.h"

// .
#include "rppi_def.h"

extern stMEMSINFO		*pMEMSINFO;
extern stCIFO			*gpCIFO;
extern stTIMERNINFO 	*pTIMERNINFO;
extern stHASHOINFO  	*pHASHOINFO; 

#ifdef _RPPI_MULTI_
extern UINT				guiSeqProcKey;
#endif

int dGetNode(U8 **ppNODE, pst_MsgQ *ppstMsgQ)
{
    *ppNODE   = NULL;
    *ppstMsgQ = NULL;

    if( (*ppNODE = nifo_node_alloc(pMEMSINFO)) == NULL ){
        log_print(LOGN_WARN, LH"FAILED IN nifo_node_alloc"EH, LT, ET);
        return -1;
    }

    if( (*ppstMsgQ = (pst_MsgQ)nifo_tlv_alloc(pMEMSINFO, *ppNODE, DEF_MSGQ_NUM, DEF_MSGQ_SIZE, DEF_MEMSET_OFF)) == NULL ){
        log_print(LOGN_WARN, LH"FAILED IN nifo_tlv_alloc, return NULL", LT);
        nifo_node_delete(pMEMSINFO, *ppNODE);
        return -2;
    }

    return 0;
}

int dMsgsnd(int procID, OFFSET offset)
{
    if( gifo_write( pMEMSINFO, gpCIFO, guiSeqProcKey, procID, offset ) < 0 ){
        log_print(LOGN_CRI, LH"FAILED IN gifo_write(A_RPPI ProcKey:%d > TARGET:%d), offset=%ld",
            LT, guiSeqProcKey, procID, offset);
        nifo_node_delete(pMEMSINFO, nifo_ptr( pMEMSINFO, offset ));
        usleep(0);
        return -1;
    }

    log_print(LOGN_INFO, LH"SND A_RPPI:%d > TARGET:%d, offset=%ld", LT, guiSeqProcKey, procID, offset);
    return 0;
}

S32 dSendMonInfo( pst_WatchMsg pstWatch )
{
    pst_MsgQ            pstMsgQ;
    pst_MsgQSub         pstMsgQSub;
	U8					*pNODE;

	if( dGetNode(&pNODE, &pstMsgQ) < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN dGetNode(A_RPPI=%d)", LT, guiSeqProcKey);
		return -1;
	}

    pstMsgQ->llMType = 0;
    pstMsgQSub = (pst_MsgQSub)&pstMsgQ->llMType;

    pstMsgQSub->usType = DEF_SYS;
    pstMsgQSub->usSvcID = SID_STATUS;
    pstMsgQSub->usMsgID = MID_SVC_MONITOR;

#ifdef _RPPI_MULTI_
    pstMsgQ->ucProID = guiSeqProcKey;
    util_makenid( guiSeqProcKey, &pstMsgQ->llNID );
#else
    pstMsgQ->ucProID = SEQ_PROC_A_RPPI;
    util_makenid( SEQ_PROC_A_RPPI, &pstMsgQ->llNID );
#endif
    pstMsgQ->llIndex = 1;
    pstMsgQ->usRetCode = 0;
    pstMsgQ->dMsgQID = 0;
    pstMsgQ->usBodyLen = sizeof(st_WatchMsg);

    memcpy( pstMsgQ->szBody, pstWatch, sizeof( st_WatchMsg ) );

	if( dMsgsnd( SEQ_PROC_O_SVCMON, nifo_offset(pMEMSINFO, pNODE) ) < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN dMsgsnd(to=A_RPPI:0x%0x)"EH, LT, guiSeqProcKey, ET);
		return -2;
	}

	log_print( LOGN_INFO, "[MONITOR] SEND TO O_SVCMON=%d", SEQ_PROC_O_SVCMON);
    return 0;
}

S32 dSendLogRPPI(HData_RPPI *pstRPPIHash, LOG_RPPI *pstRPPILog)
{
	S32         	dRet;
    OFFSET      	offset;
	U8				*pNextNode, *pstLogNode, *pCurNode;
   	LOG_RPPI_ERR	*pstErrLog; 
	RPPISESS_KEY    stRPPIKey, *pstRPPIKey;
	pstRPPIKey = &stRPPIKey;

	if(pstRPPILog->uiPPPSetupTime == 0 && pstRPPILog->uiLastFailReason == 0) {
		log_print(LOGN_CRI, "dark264sh IMSI=%s Setup=%u Fail=%u",
		pstRPPILog->szIMSI, pstRPPILog->uiPPPSetupTime, pstRPPILog->uiLastFailReason);
	}
	pstLogNode = nifo_ptr(pMEMSINFO, nifo_get_offset_node(pMEMSINFO, (U8*)pstRPPILog));
	
    pNextNode = (U8 *)nifo_entry(nifo_ptr(pMEMSINFO, ((NIFO *)pstLogNode)->nont.offset_next), NIFO, nont);
    while (pstLogNode != pNextNode)
	{
		pCurNode = pNextNode;	
        pNextNode = (U8 *)nifo_entry(nifo_ptr(pMEMSINFO, ((NIFO *)pCurNode)->nont.offset_next), NIFO, nont);
		
		offset = nifo_offset(pMEMSINFO, pCurNode);
		pstErrLog = (LOG_RPPI_ERR*)nifo_get_value(pMEMSINFO, LOG_RPPI_ERR_DEF_NUM, offset);

		/* LOG_RPPI_ERR COMMON LOG UPDATE */
		memcpy(pstErrLog, pstRPPILog, LOG_COMMON_SIZE);
		pstErrLog->uiAAAIP = pstRPPILog->uiAAAIP;
		pstErrLog->uiHSSIP = pstRPPILog->uiHSSIP;
		pstErrLog->uiOpStartTime = pstRPPILog->uiOpStartTime;
		pstErrLog->uiOpStartMTime = pstRPPILog->uiOpStartMTime;
		pstErrLog->uiOpEndTime = pstRPPILog->uiOpEndTime;
		pstErrLog->uiOpEndMTime = pstRPPILog->uiOpEndMTime;
			
		
		nifo_node_unlink_nont(pMEMSINFO, pCurNode);

//		LOG_RPPI_ERR_Prt("PRINT LOG_RPPI_ERR", pstErrLog);

		dRet = gifo_write(pMEMSINFO, gpCIFO, guiSeqProcKey, SEQ_PROC_M_LOG, nifo_offset(pMEMSINFO, pCurNode));
    	if (dRet < 0)
    	{               
        	log_print (LOGN_CRI, LH"gifo_write dRet:[%d]", LT, dRet);
        	nifo_node_delete(pMEMSINFO, pCurNode);
        	return -1;
    	}           

    }

	offset = nifo_offset(pMEMSINFO, pstLogNode);

//	LOG_RPPI_Prt("PRINT LOG_RPPI", pstRPPILog);
	
	dRet = gifo_write(pMEMSINFO, gpCIFO, guiSeqProcKey, SEQ_PROC_M_LOG, nifo_offset(pMEMSINFO,pstLogNode));
    if (dRet < 0)
    {               
        log_print (LOGN_CRI, LH"gifo_write dRet:[%d]", LT, dRet);
        nifo_node_delete(pMEMSINFO, pstLogNode);
        return -1;
    } 

	if (offset == pstRPPIHash->before.dOffset)
    {
        timerN_del(pTIMERNINFO, pstRPPIHash->before.timerNID);
        log_print(LOGN_DEBUG, LH"Del Before Timer[%lld]", LT, pstRPPIHash->before.timerNID);
        pstRPPIHash->before = pstRPPIHash->after;
        memset(&pstRPPIHash->after, 0x00, sizeof(HData));
        if (pstRPPIHash->before.dOffset == 0)
        {
            memset(pstRPPIKey, 0x00, RPPISESS_KEY_SIZE);
            memcpy(pstRPPIKey, pstRPPILog->szIMSI, MAX_MIN_SIZE);
            hasho_del (pHASHOINFO, (U8 *)pstRPPIKey);
            log_print (LOGN_DEBUG, LH"DEL HASH IMSI:%s", LT, pstRPPIKey->szIMSI);
        }   
    }   
    else if (offset == pstRPPIHash->after.dOffset)
    {       
        timerN_del(pTIMERNINFO, pstRPPIHash->after.timerNID);
        log_print(LOGN_DEBUG, LH"Del After Timer[%lld]", LT, pstRPPIHash->after.timerNID);
        memset(&pstRPPIHash->after, 0x00, sizeof(HData));
    }
    else
    {
        log_print(LOGN_CRI, LH"NOT MATCHED OFFSET Before[%ld], After[%ld], NODE[%ld]", LT,
                pstRPPIHash->before.dOffset, pstRPPIHash->after.dOffset, offset);
    }          
	return 0;
}
