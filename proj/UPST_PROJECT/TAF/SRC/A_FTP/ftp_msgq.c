/*******************************************************************************
                KTF IPAF Project

   Author   : Lee Dong-Hwan
   Section  : MESVC
   SCCS ID  : @(#)mesvc_msgq.c (V1.0)
   Date     : 3/22/04
   Revision History :
        '04.    03. 03. initial

   Description :

   Copyright (c) Infravalley 2003
*******************************************************************************/

/** A. FILE INCLUSION *********************************************************/
#include <errno.h>
#include <sys/msg.h>

#include <ipaf_svc.h>
#include <ipaf_names.h>
#include <utillib.h>
#include <wntas_sessinfo.h>

/** B. DEFINITION OF NEW CONSTANTS ********************************************/

/** C. DEFINITION OF NEW TYPES ************************************************/

/** D. DECLARATION OF VARIABLES ***********************************************/
extern int     dMyQid;

/** E.1 DEFINITION OF FUNCTIONS ***********************************************/

/** E.2 DEFINITION OF FUNCTIONS ***********************************************/
void dSendMsg(int qid, pst_MsgQ pstMsgQ);

/*******************************************************************************
 dIsRcvedMessage
*******************************************************************************/
int dIsRcvedMessage(pst_MsgQ pstMsgQ)
{
    int     dRet;

    dRet = msgrcv(dMyQid, pstMsgQ, sizeof(st_MsgQ) - sizeof(long int), 0, IPC_NOWAIT | MSG_NOERROR);
    if(dRet < 0) {
        if(errno != EINTR && errno != ENOMSG)
        {
            dAppLog( LOG_CRI, "[FAIL:%d] MSGRCV MYQ : [%s]", errno, strerror(errno));
            return -1;      /* Error */
        }

        return 0;   /* Do Nothing */
    }
	else {
	#ifdef _PATCH_MESSAGE_QUEUE
		/***************************************************************************************
			msgrcv() 의 반환값은 처음 long int 형 만큼의 사이즈는 제외하고 가져온 바이트 수가 됨.
			자세한 항목은 man -S2 msgrcv 참고
				Writer: Han-jin Park
				Date: 2008.08.13
		***************************************************************************************/
        if(dRet != pstMsgQ->usBodyLen + DEF_MSGHEAD_LEN - sizeof(long int))
	#else	/*	ELSE: #ifdef _PATCH_MESSAGE_QUEUE	*/
        if(dRet != pstMsgQ->usBodyLen + DEF_MSGHEAD_LEN)
	#endif	/*	END: #ifdef _PATCH_MESSAGE_QUEUE	*/
        {
            dAppLog( LOG_CRI, "PROID[%d] MESSAGE SIZE ERROR RCV[%d]BODY[%d]HEAD[%d]",
                pstMsgQ->ucProID, dRet, pstMsgQ->usBodyLen, DEF_MSGHEAD_LEN );
            return 10;
        }
		else {
			pstMsgQ->szBody[pstMsgQ->usBodyLen] = 0x00;
			dAppLog( LOG_INFO, "[MESSAGE RECEIVED] RCV[%d]BODY[%d]HEAD[%d]", dRet, pstMsgQ->usBodyLen, DEF_MSGHEAD_LEN );
		}
    }

    return dRet;        /* Good */
}


/*******************************************************************************
 dSendMsg
*******************************************************************************/
void dSendMsg(int qid, pst_MsgQ pstMsgQ)
{
    int     isize, dRet;

    pstMsgQ->dMsgQID = dMyQid;

    isize = sizeof(st_MsgQ) - MAX_MSGBODY_SIZE + pstMsgQ->usBodyLen - sizeof(long int);

    if ((dRet = msgsnd(qid, pstMsgQ, isize, 0)) < 0)
    {
        dAppLog( LOG_CRI, "[Qid = %d] ERROR SEND : %d[%s]", qid, errno, strerror(errno));
        return;
    } else
        dAppLog( LOG_INFO, "SEND TO MSGQ=%d LEN[%u]", qid, pstMsgQ->usBodyLen );
}

