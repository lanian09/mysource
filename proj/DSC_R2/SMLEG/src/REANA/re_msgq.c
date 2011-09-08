/*********************************************************
                 ABLEX IPAS Project (IPAF BLOCK)

   Author   : Hwang Woo-Hyoung
   Modefied : Yoon JinSeok
   Section  : IPAS(IPAM) Project
   SCCS ID  : @(#)sessana_msgq.c (V1.0)
   Date     : 03/03/04
   Revision History :
        '04.    03. 03. initial

   Description:

   Copyright (c) ABLEX 2004
*********************************************************/

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
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

#include "utillib.h"
#include "ipaf_svc.h"
#include "ipam_ipaf.h"
#include "calltcpstack.h"
#include "packet_def.h"

#include "mems.h"
#include "nifo.h"

/* Definition of New Constants */

/* Declaration of Global Variable */
extern stMEMSINFO 	*pstMEMSINFO;

/* Declaration of Extern Global Variable */

extern int		dREANAQid;
extern int		dRLEGQid;

/* Declaration of Function Definition */

/** mif_msg_read function.  
 *  
 *  전송된 NODE의 OFFSET을 읽는 함수 
 *   
 *  @param *pstMEMSINFO : 메모리 관리 정보 
 *  @param uiMsgqID     : MsgQ ID  
 *  @param *pstMsgQ     : Message Queue Pointer 
 *  
 *  @return     OFFSET      SUCC: 전송된 OFFSET 값, FAIL: 0 
 *  @see        nifo.h nifo.c 
 *   
 *  @exception  규칙에 틀린 곳을 찾아주세요. 
 *  @note       *pREADVALLIST가 NULL인 경우 OFFSET만 return 함. 
 **/ 
int dIsRcvedMessage(pst_MsgQ pstMsgQ)
{
    int     dRet;

    dRet = msgrcv(dREANAQid, pstMsgQ, sizeof(st_MsgQ) - sizeof(long), 0, IPC_NOWAIT | MSG_NOERROR);
    if(dRet < 0) {
        if(errno != EINTR && errno != ENOMSG) {
            dAppLog(LOG_CRI, "[FAIL:%d] MSGRCV RANA : [%s]", errno, strerror(errno));
            return -1;      /* Error */
        }

        return 0;   /* Do Nothing */
    }

    return dRet;        /* Good */
}

void dSendToMsg(int qid, pst_MsgQ pstMsgQ, st_IPTCPHeader *pstIPTCPHeader)
{
    int     isize, dRet;

	pst_MsgQSub pstMsgQSub;

	pstMsgQ->llMType = 0;
	pstMsgQSub = (pst_MsgQSub)&pstMsgQ->llMType;

	//pstMsgQSub->uiType = DEF_SVC;
	pstMsgQSub->usType = DEF_SVC;
	pstMsgQSub->usSvcID = SID_REDIRECT;
	pstMsgQSub->usMsgID = MID_SUBS_INFO;

	/* dMsgQID가 dREANAQid 이면 가입자 IP 로 판단 */
    pstMsgQ->dMsgQID = dREANAQid;
	pstMsgQ->usBodyLen = DEF_PACKHDR_SIZE;
	memcpy(&pstMsgQ->szBody[0], pstIPTCPHeader, DEF_PACKHDR_SIZE);

    isize = sizeof(st_MsgQ) - MAX_MSGBODY_SIZE + pstMsgQ->usBodyLen - sizeof(long);
    if ((dRet = msgsnd(qid, pstMsgQ, isize, 0)) < 0) {
        dAppLog(LOG_CRI, "[ERROR] SEND TO [Qid = %d] [ERROR: %d, %s]", qid, errno, strerror(errno));
        return;
    } else {
        dAppLog(LOG_INFO, "[SUCCESS] SEND TO RLEG MSGQ=%d SIZE=%d", qid, isize);
	}
}


void dSendToMsg2 (int qid, pst_MsgQ pstMsgQ, st_IPTCPHeader *pstIPTCPHeader)
{
    int     isize, dRet;
	
	GeneralQMsgType txGenQMsg;
	pst_MsgQ 		pstMsg_Q;
	pst_MsgQSub 	pstMsgQSub;

    dAppLog(LOG_DEBUG, "redirection dSendToMsg2" );

	txGenQMsg.mtype = MTYPE_RADIUS_TRANSMIT;
	memcpy(&txGenQMsg.body, pstMsgQ, sizeof(st_MsgQ));
	pstMsg_Q = (st_MsgQ *)txGenQMsg.body;
	pstMsg_Q->llMType = 0;
	pstMsgQSub = (st_MsgQSub *)&pstMsg_Q->llMType;

	//pstMsgQSub->uiType = DEF_SVC;
	pstMsgQSub->usType = DEF_SVC;
	pstMsgQSub->usSvcID = SID_REDIRECT;
	pstMsgQSub->usMsgID = MID_SUBS_INFO;

	/* dMsgQID가 dREANAQid 이면 가입자 IP 로 판단 */
    pstMsg_Q->dMsgQID = dREANAQid;
	pstMsg_Q->usBodyLen = DEF_PACKHDR_SIZE;
	memcpy(pstMsg_Q->szBody, pstIPTCPHeader, DEF_PACKHDR_SIZE);
    isize = sizeof(st_MsgQ) - MAX_MSGBODY_SIZE + pstMsg_Q->usBodyLen;
    
    if ((dRet = msgsnd(qid, &txGenQMsg, isize, 0)) < 0) {
        dAppLog(LOG_CRI, "[ERROR] SEND TO [Qid = %d] [ERROR: %d, %s]", qid, errno, strerror(errno));
        return;
    } else {
        dAppLog(LOG_INFO, "[SUCCESS] SEND TO RLEG MSGQ=%d SIZE=%d", qid, isize);
	}
}

