/**
	@file		m_msgq.h
	@author
	@version
	@date		2011-07-26
	@brief		m_msgq.c 헤더파일
*/

/**
 *	Include headers
 */

/* SYS HEADER */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/file.h>
#include <errno.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <mysql/mysql.h>
/* LIB HEADER */
#include "commdef.h"
#include "nifo.h"
#include "gifo.h"
#include "cifo.h"
#include "loglib.h"
/* PRO HEADER */
#include "msgdef.h"
#include "procid.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */
#include "fstat_init.h"
#include "m_msgq.h"

/**
 *	Declare variables
 */
unsigned long long	timerNID;
stTIMERNINFO		*pTIMER;

extern MYSQL		stMySQL;

extern stMEMSINFO	*gpRECVMEMS;
extern stCIFO		*pCIFO;

/**
 *	Implement func.
 */
int dIsReceivedMessage(pst_MsgQ *ppstMsgQ)
{
	//int		dRet;

	/* msgq ==> gifo */
	OFFSET	offset;

	offset = gifo_read(gpRECVMEMS, pCIFO, SEQ_PROC_MMCD);
	if(offset > 0)
	{
		*ppstMsgQ = (pst_MsgQ)nifo_get_value(gpRECVMEMS, DEF_MSGQ_NUM, offset);
	}
	else
	{
		log_print(LOGN_CRI, "FAILED IN gifo_read");
		usleep(0);
		return -1;
	}

#if 0
	if( (dRet = msgrcv(gd_MyQid, (st_MsgQ*)pstMsgQ, DEF_MSGQ_SIZE - sizeof(long), 0, IPC_NOWAIT | MSG_NOERROR)) < 0)
	{
		if( (errno != EINTR) && (errno != ENOMSG))
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN msgrcv() [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, errno, strerror(errno));
			return -1;
		}
		return 100;
	}
	else
	{
		if(dRet != pstMsgQ->usBodyLen + DEF_MSGHEAD_LEN - sizeof(long))
		{
			log_print(LOGN_CRI, "dIsReceivedMessage : PROID[%d] MESSAGE SIZE ERROR RCV[%d]BODY[%d]HEAD[%d]",
				pstMsgQ->ucProID, dRet, pstMsgQ->usBodyLen, DEF_MSGHEAD_LEN);
			return 0;
		}
	}
#endif

	return 1;
}

void SendMsg_SI_NMS(void *p)
{
	int				dRet, dSize;
	time_t			tCurrent;
	BLOCK_KEY		BLOCKKEY;
	st_atQueryInfo	statQueryInfo;
	st_MsgQ			stMsgQ;
	pst_MsgQSub		pstMsgQSub;

	/* msgq ==> gifo */
	U8				*pNODE;
	OFFSET			offset;
	pst_MsgQ		pstMsgQ;

	tCurrent	= time(NULL);
	if( (dRet = dCheckStatistics(&stMySQL, ((tCurrent/300)-1)*300)) < 0)
	{
		timerNID	= timerN_add(pTIMER, SendMsg_SI_NMS, (unsigned char*)&BLOCKKEY, sizeof(BLOCK_KEY), tCurrent+60);
		log_print(LOGN_CRI, "F=%s:%s.%d: tCurrent[%lu] - Resume after 1 minute", __FILE__, __FUNCTION__, __LINE__, ((tCurrent/300)-1)*300);
	}
	else
	{
		pstMsgQSub			= (pst_MsgQSub)&stMsgQ.llMType;
		pstMsgQSub->usType	= DEF_SVC;
		pstMsgQSub->usSvcID	= SID_SVC;
		pstMsgQSub->usMsgID	= MID_LOG_STATISTICS;

		stMsgQ.usBodyLen	= sizeof(st_atQueryInfo);
		stMsgQ.ucProID		= SEQ_PROC_FSTAT;

		/* msgq ==> gifo */
		stMsgQ.dMsgQID		= 0;
		//stMsgQ.dMsgQID		= gdSINMSQid;

		statQueryInfo.cPeriod		= STAT_PERIOD_5MIN;
		statQueryInfo.tStartTime	= ((tCurrent/300)-1)*300;
		statQueryInfo.tEndTime		= (((tCurrent/300)-1)*300)+120;

		memcpy(stMsgQ.szBody, &statQueryInfo, sizeof(st_atQueryInfo));

		dSize = DEF_MSGHEAD_LEN + stMsgQ.usBodyLen - sizeof(long);

		/* msgq ==> gifo */
		pNODE = nifo_node_alloc(gpRECVMEMS);
		if(pNODE == NULL)
		{
			log_print(LOGN_CRI, "ERROR IN nifo_node_alloc, RET=NULL");
			return;
		}

		pstMsgQ = (pst_MsgQ)nifo_tlv_alloc(gpRECVMEMS, pNODE, DEF_MSGQ_NUM, DEF_MSGQ_SIZE, DEF_MEMSET_OFF);
		if(pstMsgQ == NULL)
		{
			log_print(LOGN_CRI, "ERROR IN nifo_tlv_alloc, RET=NULL");
			nifo_node_delete(gpRECVMEMS, pNODE);
			return;
		}

		memcpy(pstMsgQ, &stMsgQ, DEF_MSGQ_SIZE);
		offset = nifo_offset(gpRECVMEMS, pNODE);

		if(gifo_write(gpRECVMEMS, pCIFO, SEQ_PROC_MMCD, SEQ_PROC_SI_NMS, offset) < 0)
		{
			// TODO gifo_write 실패시 재시도 루틴
			
			log_print(LOGN_CRI, "[ERROR] gifo_write(from=%d:MMCD, to=%d), offset=%ld", SEQ_PROC_MMCD, SEQ_PROC_SI_NMS, offset);
			nifo_node_delete(gpRECVMEMS, pNODE);
			usleep(0);
			return;
		}
		
#if 0
		if( (dRet = msgsnd(gdSINMSQid, &stMsgQ, dSize, IPC_NOWAIT)) < 0)
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN msgsnd(gdSINMSQid[%d]) dRet[%d]", __FILE__, __FUNCTION__, __LINE__, gdSINMSQid, dRet);
			return;
		}
#endif

		timerNID	= timerN_add(pTIMER, SendMsg_SI_NMS, (unsigned char*)&BLOCKKEY, sizeof(BLOCK_KEY), (((tCurrent/300)+1)*300)+120);
	}

	if( ((tCurrent/SEC_OF_HOUR)*SEC_OF_HOUR) == ((tCurrent/SEC_OF_5MIN)*SEC_OF_5MIN))
	{
		log_print(LOGN_DEBUG, "F=%s:%s.%d: tCurrent[%lu]", __FILE__, __FUNCTION__, __LINE__, tCurrent);
		if( (dRet = dSend_NMS_HOUR(tCurrent)) < 0)
			log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN dSend_NMS_HOUR(tCurrent[%lu])", __FILE__, __FUNCTION__, __LINE__, tCurrent);
	}
}

int dSend_NMS_HOUR(time_t tCurrent)
{
	int				dRet;
	//size_t			dSize;
	time_t			tStart, tEnd;
	st_atQueryInfo	statQueryInfo;
	st_MsgQ			stMsgQ;
	pst_MsgQSub		pstMsgQSub;

	/* msgq ==> gifo */
	U8				*pNODE;
	OFFSET			offset;
	pst_MsgQ		pstMsgQ;	

	tStart	= ((tCurrent/SEC_OF_HOUR)-1)*SEC_OF_HOUR;
	tEnd	= (tCurrent/SEC_OF_HOUR)*SEC_OF_HOUR;
	if( (dRet = dCheckHourStat(&stMySQL, tStart, tEnd)) < 0)
		log_print(LOGN_CRI, "F=%s:%s.%d: tCurrent[%lu]", __FILE__, __FUNCTION__, __LINE__, tCurrent);
	else
	{
		pstMsgQSub			= (pst_MsgQSub)&stMsgQ.llMType;
		pstMsgQSub->usType	= DEF_SVC;
		pstMsgQSub->usSvcID	= SID_SVC;
		pstMsgQSub->usMsgID	= MID_LOG_STATISTICS;

		stMsgQ.usBodyLen	= sizeof(st_atQueryInfo);
		stMsgQ.ucProID		= SEQ_PROC_FSTAT;

		/* msgq ==> gifo */
		stMsgQ.dMsgQID		= 0;
		//stMsgQ.dMsgQID		= gdSINMSQid;

		statQueryInfo.cPeriod		= STAT_PERIOD_HOUR;
		statQueryInfo.tStartTime	= tStart;
		statQueryInfo.tEndTime		= tEnd;

		memcpy(stMsgQ.szBody, &statQueryInfo, sizeof(st_atQueryInfo));

		/* msgq ==> gifo */
		pNODE = nifo_node_alloc(gpRECVMEMS);
		if(pNODE == NULL)
		{
			log_print(LOGN_CRI, "[ERROR] nifo_node_alloc, NULL");
			return -1;
		}

		pstMsgQ = (pst_MsgQ)nifo_tlv_alloc(gpRECVMEMS, pNODE, DEF_MSGQ_NUM, DEF_MSGQ_SIZE, DEF_MEMSET_OFF);
		if(pstMsgQ == NULL)
		{
			log_print(LOGN_CRI, "[ERROR] nifo_tlv_alloc, NULL");
			return -2;
		}

		memcpy(pstMsgQ, &stMsgQ, DEF_MSGQ_SIZE);
		offset = nifo_offset(gpRECVMEMS, pNODE);

		if(gifo_write(gpRECVMEMS, pCIFO, SEQ_PROC_MMCD, SEQ_PROC_MMCD, offset) < 0)
		{
			log_print(LOGN_CRI, "[ERROR] gifo_write(from=%d:MMCD, to=%d:MMCD), offset=%ld",
					SEQ_PROC_MMCD, SEQ_PROC_MMCD, offset);
			return -5;
		}

#if 0
		dSize = DEF_MSGHEAD_LEN + stMsgQ.usBodyLen - sizeof(long);
		if( (dRet = msgsnd(gdSINMSQid, &stMsgQ, dSize, IPC_NOWAIT)) < 0)
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN msgsnd(gdSINMSQid[%d]) dRet[%d]", __FILE__, __FUNCTION__, __LINE__, gdSINMSQid, dRet);
			return -1;
		}
#endif

	}

	return 0;
}


int dCheckStatistics(MYSQL *pstMySQL, time_t tStatTime)
{
	int			i, dResult;
	char		sBuf[MAX_SQLQUERY_SIZE];
	MYSQL_ROW	stRow;
	MYSQL_RES	*pstRst;

	for(i = 0; i < IDX_MAXIMUM_STATISTICS; i++)
	{
		switch(i)
		{
			case IDX_LOAD_STATISTICS:
				sprintf(sBuf, "SELECT COUNT(*) FROM STAT_LOAD_5MIN WHERE STATTIME=%lu", tStatTime);
				break;
			case IDX_FAULT_STATISTICS:
				sprintf(sBuf, "SELECT COUNT(*) FROM STAT_FAULT_5MIN WHERE STATTIME=%lu", tStatTime);
				break;
			case IDX_TRAFFIC_STATISTICS:
				sprintf(sBuf, "SELECT COUNT(*) FROM STAT_TRAFFIC_5MIN WHERE STATTIME=%lu", tStatTime);
				break;
		}

		if(mysql_query(pstMySQL, sBuf) != 0)
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_query(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
				sBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
			return -1;
		}

		if( (pstRst = mysql_store_result(pstMySQL)) == NULL)
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_store_result(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
				sBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
			return -2;
		}

		dResult	= 0;
		while( (stRow = mysql_fetch_row(pstRst)) != NULL)
			dResult	= atoi(*stRow);

		mysql_free_result(pstRst);

		if(dResult == 0)
		{
			switch(i)
			{
				case IDX_LOAD_STATISTICS:
					sprintf(sBuf, "STAT_LOAD_5MIN");
					break;
				case IDX_FAULT_STATISTICS:
					sprintf(sBuf, "STAT_FAULT_5MIN");
					break;
				case IDX_TRAFFIC_STATISTICS:
					sprintf(sBuf, "STAT_TRAFFIC_5MIN");
					break;
			}
			log_print(LOGN_CRI, "F=%s:%s.%d: COUNT[%d] IN TABLE[%s]", __FILE__, __FUNCTION__, __LINE__, dResult, sBuf);
			return -1;
		}
	}

	return 0;
}

int dCheckHourStat(MYSQL *pstMySQL, time_t tStart, time_t tEnd)
{
	int			i, dResult;
	char		sBuf[MAX_SQLQUERY_SIZE];
	MYSQL_ROW	stRow;
	MYSQL_RES	*pstRst;

	for(i = 0; i < IDX_MAXIMUM_STATISTICS; i++)
	{
		switch(i)
		{
			case IDX_LOAD_STATISTICS:
				sprintf(sBuf, "SELECT COUNT(*) FROM STAT_LOAD_5MIN WHERE STATTIME>=%lu AND STATTIME<=%lu", tStart, tEnd);
				break;
			case IDX_FAULT_STATISTICS:
				sprintf(sBuf, "SELECT COUNT(*) FROM STAT_FAULT_5MIN WHERE STATTIME>=%lu AND STATTIME<=%lu", tStart, tEnd);
				break;
			case IDX_TRAFFIC_STATISTICS:
				sprintf(sBuf, "SELECT COUNT(*) FROM STAT_TRAFFIC_5MIN WHERE STATTIME>=%lu AND STATTIME<=%lu", tStart, tEnd);
				break;
		}

		if(mysql_query(pstMySQL, sBuf) != 0)
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_query(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
				sBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
			return -1;
		}

		if( (pstRst = mysql_store_result(pstMySQL)) == NULL)
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_store_result(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
				sBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
			return -2;
		}

		dResult	= 0;
		if( (stRow = mysql_fetch_row(pstRst)) != NULL)
			dResult	= atoi(*stRow);

		mysql_free_result(pstRst);

		if(dResult == 0)
		{
			switch(i)
			{
				case IDX_LOAD_STATISTICS:
					sprintf(sBuf, "STAT_LOAD_5MIN");
					break;
				case IDX_FAULT_STATISTICS:
					sprintf(sBuf, "STAT_FAULT_5MIN");
					break;
				case IDX_TRAFFIC_STATISTICS:
					sprintf(sBuf, "STAT_TRAFFIC_5MIN");
					break;
			}
			log_print(LOGN_CRI, "F=%s:%s.%d: COUNT[%d] IN TABLE[%s]", __FILE__, __FUNCTION__, __LINE__, dResult, sBuf);
			return -1;
		}
	}

	return 0;
}
