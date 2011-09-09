/**A.1*  File Inclusion *******************************************************/
#include <unistd.h>			/* USLEEP() */

#include "procid.h"
#include "mems.h"
#include "cifo.h"
#include "nifo.h"
#include "gifo.h"

#include "msgdef.h"			/* st_MsgQ */
#include "mmcdef.h"			/* mml_msg, dbm_mst_t */

#include "loglib.h"

extern stMEMSINFO	*pMEMSINFO;
extern stCIFO		*gpCIFO;

int dIsReceivedMessage(pst_MsgQ *ppstMsgQ)
{
	OFFSET  offset;

	if( (offset = gifo_read(pMEMSINFO, gpCIFO, SEQ_PROC_S_MNG)) <= 0 ){
		usleep(0);
		return 100;
	}

	*ppstMsgQ = (pst_MsgQ)nifo_get_value(pMEMSINFO, DEF_MSGQ_NUM, offset);
	if( *ppstMsgQ == NULL ){
		log_print(LOGN_CRI, LH"FAILED IN nifo_get_value(st_MsgQ=%d), offset=%ld", LT, DEF_MSGQ_NUM, offset);
		usleep(0);
		return 100;
	}

	return 0;

} /* end of dIsReceivedMessage */

int dSendMsg(pst_MsgQ pstMsg, int dSeqProcID)
{

	U8 *pNODE, *pBUF;
	
	pNODE = nifo_node_alloc(pMEMSINFO);
	if( pNODE == NULL ){
		log_print(LOGN_WARN, LH"FAILED IN nifo_node_alloc"EH, LT, ET);
		return -1;
	}

	pBUF = nifo_tlv_alloc(pMEMSINFO, pNODE, DEF_MSGQ_NUM, DEF_MSGQ_SIZE, DEF_MEMSET_OFF);
	if( pBUF == NULL ){
		log_print(LOGN_WARN, LH"FAILED IN nifo_tlv_alloc, return NULL", LT);
		nifo_node_delete(pMEMSINFO, pNODE);
		return -2;
	}

	memcpy( (char*)pBUF, (char*)pstMsg, DEF_MSGHEAD_LEN + pstMsg->usBodyLen );

	if( gifo_write( pMEMSINFO, gpCIFO, SEQ_PROC_S_MNG, dSeqProcID, nifo_offset(pMEMSINFO, pNODE) ) < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN gifo_write(S_MNG:%d > TARGET:%d), offset=%ld"EH,
			LT, SEQ_PROC_S_MNG, dSeqProcID, nifo_offset(pMEMSINFO, pNODE), ET);
		nifo_node_delete(pMEMSINFO, pNODE);
		usleep(0);
		return -3;
	}

	log_print(LOGN_INFO, LH"SUCCESS SENT TO ProcID=%d", LT, dSeqProcID);
	return 0;

} /* end of dSendMsg */

int dSendMMC(mml_msg *mmsg, dbm_msg_t *smsg, long long llNID)
{
	pst_MsgQ		pstSndMsg;
	pst_MsgQSub		pstMsgQSub;
	U8*				pNODE;

	pNODE = nifo_node_alloc(pMEMSINFO);
	if( pNODE == NULL ){
		log_print(LOGN_WARN, LH"FAILED IN nifo_node_alloc"EH, LT, ET);
		return -1;
	}

	pstSndMsg = (pst_MsgQ)nifo_tlv_alloc(pMEMSINFO, pNODE, DEF_MSGQ_NUM, DEF_MSGQ_SIZE, DEF_MEMSET_OFF);
	if( pstSndMsg == NULL ){
		log_print(LOGN_WARN, LH"FAILED IN nifo_tlv_alloc, return NULL", LT);
		nifo_node_delete(pMEMSINFO, pNODE);
		return -2;
	}

	smsg->head.src_proc	= SEQ_PROC_S_MNG;
	smsg->head.dst_func	= mmsg->src_func;
	smsg->head.dst_proc	= mmsg->src_proc;
	smsg->head.cmd_id	= mmsg->cmd_id;
	smsg->head.msg_id	= mmsg->msg_id;

	pstMsgQSub = (pst_MsgQSub)&pstSndMsg->llMType;

	pstMsgQSub->usType	= DEF_SYS;
	pstMsgQSub->usSvcID	= SID_MML;
	pstMsgQSub->usMsgID	= MID_MML_RST;

	pstSndMsg->llNID	= llNID;

	pstSndMsg->ucProID	= SEQ_PROC_S_MNG;

	pstSndMsg->usBodyLen	= sizeof(dbm_msg_t)-MSG_DATA_LEN+smsg->head.msg_len;
	pstSndMsg->usRetCode	= 0;
	memcpy(pstSndMsg->szBody, smsg, pstSndMsg->usBodyLen);

	if( gifo_write(pMEMSINFO, gpCIFO, SEQ_PROC_S_MNG, SEQ_PROC_MMCD, nifo_offset(pMEMSINFO, pNODE)) < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN gifo_write(S_MNG:%d > MMCD:%d), offset=%ld"EH,
			LT, SEQ_PROC_S_MNG, SEQ_PROC_MMCD, nifo_offset(pMEMSINFO, pNODE), ET);
		nifo_node_delete(pMEMSINFO, pNODE);
		usleep(0);
		return -3;
	}

	return 0;
} /* end of dSendMMC */
