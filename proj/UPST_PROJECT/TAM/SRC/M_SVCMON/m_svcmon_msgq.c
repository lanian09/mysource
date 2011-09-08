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
#include <string.h>

#include "loglib.h"
#include "m_svcmon_if.h"
#include "m_svcmon_msgq.h"

int dIsReceivedMessage(st_MsgQ *pstMsgQ)
{

	int dRet;

	if( (dRet = dMsgrcv(&pstMsgQ)) < 0 ){
		if( dRet != -1 ){
			log_print(LOGN_CRI, LH"FAILED IN dMsgrcv(ALMD)", LT);
		}
		return -2;
	}
    return 0;
} /* end of dIsReceivedMessage */

int dSendMsg(int procid, st_MsgQ *pstMsgQ)
{
	unsigned char   *pNODE;
    int     dRet;

	if( (dRet = dGetNode(&pNODE, &pstMsgQ)) < 0 ){
		log_print(LOGN_CRI,LH"FAILED IN dGetNode(MOND), errno=%d:%s",
				 LT,errno,strerror(errno));
		return -1;
	}

	if( (dRet = dMsgsnd(procid, pNODE)) < 0 ){
		log_print(LOGN_CRI, "SEND FAIL  FOR FSTAT IS NOT DELIVERED : %s",
				strerror(errno) );
		return -1;
	}

    return dRet;
}

