/*******************************************************************************
			DQMS Project

	Author   :
	Section  : ALMD
	SCCS ID  : @(#)almd_mem.h	1.1
	Date     :
	Revision History :

	Description :

	Copyright (c) uPRESTO 2005
*******************************************************************************/
#ifndef __ALMD_MEM_H__
#define __ALMD_MEM_H__

#include "msgdef.h"

extern int dIsRcvedMessage(st_MsgQ *pstMsg);
extern int dSend_FSTAT(pst_MsgQ pstMsg, char *data);
extern int dSend_Traffic_FSTAT(st_MsgQ *pstMsg);
extern void Send_CondMess(int systype, int sysno, int loctype, char invtype, short invno, char almstatus, char oldalm, long long llLoadVal );
extern int dMergeBuffer( int dIdx, int dSfd, char* szTcpRmsg, int dRecvLen );

#endif /* __ALMD_MEM_H__ */
