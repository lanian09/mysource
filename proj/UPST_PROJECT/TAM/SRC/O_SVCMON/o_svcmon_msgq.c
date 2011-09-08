/*******************************************************************************
            UPRESTO W-NTAS Project
    Author      :   JOSHUA
    Section     :   NTAM M_SIGNAL_MSGQ
    SCCS ID     :   %W%
    DATE        :   %G%

    Revision History :
        06. 06. 01      Initial

    Description:

    Copyright (C) UPRESTO 2005
*******************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>

// LIB
#include "typedef.h"
#include "loglib.h"
#include "ipclib.h"

// PROJECT
#include "common_stg.h"
#include "func_time_check.h"
#include "svcmon.h"
#include "msgdef.h"
#include "mmcdef.h"
#include "procid.h"

// TAM
#include "watch_mon.h"
#include "watch_filter.h"

// OAM
#include "almstat.h"

#include "o_svcmon_alarm.h"
#include "o_svcmon_util.h"
#include "o_svcmon_get.h"
#include "o_svcmon_set.h"
#include "o_svcmon_msgq.h"

extern S32			dMyQID;
extern S32			dSysNo;
extern S32			gdIndex;
extern S32			dCONDQID;

int dIsReceivedMessage(st_MsgQ *pstMsgQ)
{
    int     dRet;

    dRet = msgrcv(dMyQID, (st_MsgQ *)pstMsgQ, DEF_MSGQ_SIZE - sizeof(long), 0, IPC_NOWAIT | MSG_NOERROR);
    if(dRet < 0) {
        if(errno != EINTR && errno != ENOMSG) { 	
			log_print(LOGN_CRI, "[FAIL:%d] MSGRCV MYQ : [%s]", errno, strerror(errno));
            return -1;
        }

        return 100;
    } else {
        if(dRet != pstMsgQ->usBodyLen + DEF_MSGHEAD_LEN - sizeof(long)) {
            log_print(LOGN_CRI,
            "dIsReceivedMessage : PROID[%d] MESSAGE SIZE ERROR RCV[%d]BODY[%d]HEAD[%ld]",
            pstMsgQ->ucProID, dRet, pstMsgQ->usBodyLen, DEF_MSGHEAD_LEN);
            return 0;
        }
    }

    return 1;
} /* end of dIsReceivedMessage */

int dSendMsg(int qid, st_MsgQ *pstMsgQ)
{
    int     isize, dRet;

    pstMsgQ->dMsgQID = dMyQID;

    isize = sizeof(st_MsgQ) - MAX_MSGBODY_SIZE + pstMsgQ->usBodyLen - sizeof(long);

    if ((dRet = msgsnd(qid, pstMsgQ, isize, 0)) < 0) {
        log_print( LOGN_CRI, "[Qid = %d] ERROR SEND : %d[%s]", qid, errno, strerror(errno));
        return -1;
    } else
        log_print( LOGN_INFO, "SEND TO MSGQ=%d LEN[%d]", qid, dRet);
    
    return dRet;
}

void Send_CondMess(int sysno, int loctype, char invtype, short invno, char almstatus, char oldalm, U8 tmpl4type, UINT ip 
				, long long loadval)
{
	int				dRet, dMsgLen;
	st_Qentry		smesg;
	char			levelstr[8];
	st_almsts		almsts;
	st_MsgQ			stSndMsg;
	pst_MsgQSub		pstSndSub;

	smesg.mtype = 0;
	levelstr[0] = 0;

	memset(&stSndMsg, 0x00, sizeof(st_MsgQ));
	memset(&smesg.mtext, 0x00, 4096);
	memset(&almsts, 0x00, sizeof(st_almsts) );

	almsts.ucSysType = sysno;
	almsts.ucSysNo = dSysNo;
	almsts.ucLocType = loctype;
	almsts.ucInvType = invtype;
	almsts.ucInvNo = invno;
	almsts.ucAlmLevel = almstatus;
	almsts.ucOldAlmLevel = oldalm;
	almsts.ucReserv = tmpl4type;
	almsts.uiIPAddr = ip;
	almsts.llLoadVal = loadval;

	time(&almsts.tWhen);

	memcpy( smesg.mtext, &almsts, sizeof(st_almsts) );


	pstSndSub = (pst_MsgQSub)&stSndMsg.llMType;
	pstSndSub->usType = DEF_SYS;
	pstSndSub->usSvcID = SID_STATUS;
	pstSndSub->usMsgID = MID_CONSOL;

	//dRet = dMakeNID( SEQ_PROC_O_SVCMON, &stSndMsg.llNID );
	stSndMsg.ucNTAFID = 0;
	stSndMsg.ucProID = SEQ_PROC_O_SVCMON;
	stSndMsg.llIndex = gdIndex;
	gdIndex++;

	stSndMsg.dMsgQID = dCONDQID;
	stSndMsg.usBodyLen = sizeof(st_almsts);
	stSndMsg.usRetCode = 0;

	//memset( stSndMsg.szMIN, 0x00, MAX_MIN_SIZE );
	//memset( stSndMsg.szExtra, 0x00, MAX_EXTRA_SIZE );
	memcpy( stSndMsg.szBody, smesg.mtext, stSndMsg.usBodyLen );

	dMsgLen = DEF_MSGHEAD_LEN + stSndMsg.usBodyLen - sizeof(long);

	/****************************************************************************************/
	/* COND로 메세지 전송                                                                   */
	/****************************************************************************************/
	if( (dRet = msgsnd( dCONDQID, &stSndMsg, dMsgLen, IPC_NOWAIT )) < 0 ) {
		log_print(LOGN_DEBUG,"[FAIL] CONSOL MSGSND ERROR. CAUSE[%s]", strerror(errno));
	} else {
		log_print(LOGN_DEBUG,"CONSOL MSGSND OK");
	}

	return;
}





