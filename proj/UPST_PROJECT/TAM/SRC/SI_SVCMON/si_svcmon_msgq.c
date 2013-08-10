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
#include <string.h> 
#include <errno.h> 

#include "loglib.h"
#include "si_svcmon_if.h"
#include "si_svcmon_msgq.h"

int dIsReceivedMessage(st_MsgQ *pstMsgQ)
{
	int dRet;

	if( (dRet = dMsgrcv(&pstMsgQ)) < 0 ){
		if( dRet != -1 ){
			log_print(LOGN_CRI, "%s.%d:%s FAILED IN dMsgrcv(SI_SVCMON)",__FILE__,__LINE__,__FUNCTION__);
		}
		return -2;
	}
	return 0;

} /* end of dIsReceivedMessage */

