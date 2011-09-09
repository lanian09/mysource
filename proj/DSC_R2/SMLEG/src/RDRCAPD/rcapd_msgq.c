/* File Include */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <unistd.h>

#include "comm_msgtypes.h"
#include "utillib.h"
#include "rcapd.h"

/* Definition of New Constants */

/* Declaration of Global Variable */

/* Declaration of Extern Global Variable */
extern int	 	JiSTOPFlag;
extern int	 	FinishFlag;

extern int		pid;
extern int	 	proc_idx;
extern char		logfilepath[256];


/* Declaration of Function Definition */

/** MSGQ_read function.  
 *  
 *  전송된 RADIUS DATA를 읽는 함수 
 *   
 *  @param uiMsgqID	 : MsgQ ID  
 *  @param *pstMsgQ	 : Message Queue Pointer 
 *  
 *  @return int	 SUCC: 0, FAIL: -1
 *   
 *  @exception  
 **/ 

int msgQRead(int tid, st_MsgQ *pstMsgQ) 
{
	int	dRet;

	if((dRet = msgrcv(dANAQid, pstMsgQ, DEF_MSGQ_SIZE - sizeof(long), 0, IPC_NOWAIT | MSG_NOERROR)) < 0)
	{
		if (errno != EINTR && errno != ENOMSG)
		{
			dAppLog(LOG_CRI, "[THREAD ID:%d][FAIL:%d] MSGRCV MYQ : [%s]"
					, tid, errno, strerror(errno));
			return -errno;
		}
		return 0;
	}
	return 1;
}


int dIsRcvedMessage(pst_MsgQ pstMsgQ)
{
	int	dRet;

	dRet = msgrcv(dANAQid, pstMsgQ, sizeof(st_MsgQ) - sizeof(long), 0, IPC_NOWAIT | MSG_NOERROR);
	if(dRet < 0) {
		if(errno != EINTR && errno != ENOMSG) {
			dAppLog(LOG_CRI, "[FAIL:%d] MSGRCV MYQ : [%s]", errno, strerror(errno));
			return -1;	  /* Error */
		}
		return 0;   /* Do Nothing */
	}
	return dRet;		/* Good */
}

char *getStrRdrType(int rdr_type)
{
	switch (rdr_type)
	{
	case RDR_TYPE_TRANSACTION:
		return "TRANSACTION RDR";
	case RDR_TYPE_BLOCK:
		return "BLOCK RDR";
	default:
		return "UNKNOWN RDR";
	}
}

void dSendMsg_RDRANA(int qid, int msg_type, char *data, int dataLen)
{
	GeneralQMsgType txGenQMsg;
	IxpcQMsgType    *txIxpcMsg;
	int dTxLen=0, dMsgLen;

	if (msg_type == 1) {
		txGenQMsg.mtype = MTYPE_TRANS_REPORT;
	} else {
		txGenQMsg.mtype = MTYPE_BLOCK_REPORT;
	}
	txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
	memset ((void*)&txIxpcMsg->head, 0, sizeof(txIxpcMsg->head));
	txIxpcMsg->head.msgId = 0;
	strcpy (txIxpcMsg->head.srcSysName, mySysName);
	strcpy (txIxpcMsg->head.srcAppName, myAppName);
	strcpy (txIxpcMsg->head.dstSysName, mySysName);
	strcpy (txIxpcMsg->head.dstAppName, "RDRANA");

	dMsgLen = dataLen;
	memcpy(txIxpcMsg->body, data, dMsgLen);
	txIxpcMsg->head.bodyLen = dMsgLen;

	dTxLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;

	if( msgsnd(dANAQid, (void*)&txGenQMsg, dTxLen, IPC_NOWAIT) < 0 ) {
		dAppLog(LOG_CRI, "[FAIL] %s MSGQ SEND ERROR %d(%s)", getStrRdrType(msg_type), errno, strerror(errno));
	}
	else {
		dAppLog(LOG_CRI, "%s SEND SUCCESS(txLen=%d)", getStrRdrType(msg_type), dataLen);
	}
}

int branchMessage (GeneralQMsgType *prxGenQMsg)
{
	IxpcQMsgType	*rxIxpcMsg;

	switch(prxGenQMsg->mtype)
	{
		case MTYPE_MMC_REQUEST:
			rxIxpcMsg = (IxpcQMsgType *)prxGenQMsg->body;
			mmcd_exeMMCMsg (rxIxpcMsg);
			break;

		default:
			dAppLog(LOG_DEBUG, "ERROR NOT-MATCHED GeneralQMsgType][MTYPE:%d"
					, prxGenQMsg->mtype);
			break;
	}

	return 0;
}

