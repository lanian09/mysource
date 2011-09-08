/**
 *	INCLUDE HEADER FILES
 */
#include <unistd.h>
#include <time.h>

// DQMS
#include "procid.h"
#include "msgdef.h"
#include "common_stg.h"
#include "timerN.h"			/* stTIMERNINFO */
#include "hasho.h"			/* stHASHOINFO */
#include "sshmid.h"

// LIB
#include "utillib.h"		/* util_makenid(2) */
#include "loglib.h"
#include "nifo.h"
#include "gifo.h"
#include "cifo.h"

// TAM
#include "watch_mon.h"		/* pst_WatchMsg */
#include "rppi_def.h"		/* HData_RPPI */

// .
#include "roam_msgq.h"

/**
 *	DECLARE VARIABLES
 */
extern stMEMSINFO		*pMEMSINFO;
extern stCIFO			*pCIFO;
extern stTIMERNINFO 	*pTIMERNINFO;
extern stHASHOINFO  	*pHASHOINFO; 

/**
 *	IMPLEMENT FUNCTIONS
 */
S32 dSendMonInfo( pst_WatchMsg pstWatch )
{
	pst_MsgQ		pstMsgQ;
    pst_MsgQSub		pstMsgQSub;
    int				dRet;

	U8				*pNODE;
	OFFSET			offset;
	
    
	pNODE = nifo_node_alloc(pMEMSINFO);
	if(pNODE == NULL)
	{
		log_print(LOGN_CRI, LH"FAILED IN nifo_node_alloc, NULL", LT);
		return -1;
	}

	pstMsgQ = (pst_MsgQ)nifo_tlv_alloc(pMEMSINFO, pNODE, DEF_MSGQ_NUM, DEF_MSGQ_SIZE, DEF_MEMSET_OFF);
	if(pstMsgQ == NULL)
	{
		log_print(LOGN_CRI, LH"FAILED IN nifo_tlv_alloc, NULL", LT);
		nifo_node_delete(pMEMSINFO, pNODE);
		return -2;
	}

	pstMsgQ->llMType = 0;
    pstMsgQSub = (pst_MsgQSub)&pstMsgQ->llMType;

    pstMsgQSub->usType = DEF_SYS;
    pstMsgQSub->usSvcID = SID_STATUS;
    pstMsgQSub->usMsgID = MID_SVC_MONITOR;

    pstMsgQ->ucProID = SEQ_PROC_A_RPPI;
    //dMakeNID( SEQ_PROC_A_RPPI, &stMsgQ.llNID );
	util_makenid(SEQ_PROC_A_RPPI, &pstMsgQ->llNID);
    pstMsgQ->llIndex = 1;
    pstMsgQ->usRetCode = 0;
    //stMsgQ.dMsgQID = dMyQID;
    pstMsgQ->usBodyLen = sizeof(st_WatchMsg);

    memcpy( pstMsgQ->szBody, pstWatch, sizeof( st_WatchMsg ) );

	offset = nifo_offset(pMEMSINFO, pNODE);
	dRet = gifo_write(pMEMSINFO, pCIFO, SEQ_PROC_A_ROAM, SEQ_PROC_O_SVCMON, offset);
	if(dRet < 0)
	{
		log_print(LOGN_CRI, LH"FAILED IN gifo_write from=A_ROAM:%d, to=O_SVCMON:%d, offset=%ld", LT,
					SEQ_PROC_A_ROAM, SEQ_PROC_O_SVCMON, offset);
		nifo_node_delete(pMEMSINFO, pNODE);
		usleep(0);
		return -3;
	}
	else
	{
		log_print(LOGN_INFO, LH"[MONITOR] SEND TO O_SVCMON:%d, [%u]", LT,
					SEQ_PROC_O_SVCMON, pstMsgQ->usBodyLen);
	}
	
#if 0
    if ((dRet = msgsnd(dMonQID, &stMsgQ,
            sizeof(st_MsgQ) - MAX_MSGBODY_SIZE + stMsgQ.usBodyLen - sizeof(long), 0)) < 0) {
        dAppLog( LOG_CRI, "[MONITOR] [Qid = %d] ERROR SEND : %d[%s]",
            dMonQID, errno, strerror(errno));
        return -1;
    } else
        dAppLog( LOG_INFO, "[MONITOR] SEND TO MSGQ=%d [%u]", dMonQID, stMsgQ.usBodyLen);
#endif

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

	if(pstRPPILog->usCallType == INIT_ROAM_CALLSTART) {
		pstRPPILog->uiAuthReqTime = pstRPPILog->uiTmpAuthReqTime;
		pstRPPILog->uiAuthReqMTime = pstRPPILog->uiTmpAuthReqMTime;
		pstRPPILog->uiAuthEndTime = pstRPPILog->uiTmpAuthEndTime;
		pstRPPILog->uiAuthEndMTime = pstRPPILog->uiTmpAuthEndMTime;
		STG_DiffTIME64(pstRPPILog->uiAuthEndTime, pstRPPILog->uiAuthEndMTime, pstRPPILog->uiAuthReqTime, pstRPPILog->uiAuthReqMTime, &pstRPPILog->llAuthDuration);
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

		dRet = gifo_write(pMEMSINFO, pCIFO, SEQ_PROC_A_ROAM, SEQ_PROC_M_LOG, offset);
		if(dRet < 0)
		{
        	log_print(LOGN_CRI, LH"FAILED IN gifo_write dRet:[%d]", LT, dRet);
			nifo_node_delete(pMEMSINFO, pCurNode);

			return -1;
		} 		

#if 0
		dRet = nifo_msg_write(pMEMSINFO, dLogQID, pCurNode);
    	if (dRet < 0)
    	{               
        	log_print(LOGN_CRI, LH"nifo_msg_write dRet:[%d]", LT, dRet);
        	nifo_node_delete(pMEMSINFO, pCurNode);
        	return -1;
    	}
#endif

    }

	offset = nifo_offset(pMEMSINFO, pstLogNode);

//	LOG_RPPI_Prt("PRINT LOG_RPPI", pstRPPILog);

	dRet = gifo_write(pMEMSINFO, pCIFO, SEQ_PROC_A_ROAM, SEQ_PROC_M_LOG, offset);
	if(dRet < 0)
	{
       	log_print(LOGN_CRI, LH"FAILED IN gifo_write dRet:[%d]", LT, dRet);
		nifo_node_delete(pMEMSINFO, pCurNode);

		return -1;
	}

#if 0
	dRet = nifo_msg_write(pMEMSINFO, dLogQID, pstLogNode);
    if (dRet < 0)
    {               
        log_print(LOGN_CRI, LH"nifo_msg_write dRet:[%d]", LT, dRet);
        nifo_node_delete(pMEMSINFO, pstLogNode);
        return -1;
    } 
#endif

	if (offset == pstRPPIHash->before.dOffset)
    {
        timerN_del(pTIMERNINFO, pstRPPIHash->before.timerNID);
        log_print(LOGN_DEBUG, LH"Del Before Timer[%lld]", LT, pstRPPIHash->before.timerNID);
        pstRPPIHash->before = pstRPPIHash->after;
        memset(&pstRPPIHash->after, 0x00, sizeof(HData));
        if (pstRPPIHash->before.dOffset == 0)
        {
            memset(pstRPPIKey, 0x00, MAX_MIN_SIZE);
            memcpy(pstRPPIKey, pstRPPILog->szIMSI, MAX_MIN_SIZE);
            hasho_del (pHASHOINFO, (U8 *)pstRPPIKey);
            log_print(LOGN_DEBUG, LH"DEL HASH IMSI:%s", LT, pstRPPIKey->szIMSI);
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
