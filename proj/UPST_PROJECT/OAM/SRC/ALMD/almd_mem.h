/*******************************************************************************
			DQMS Project

	Author   :
	Section  : ALMD
	SCCS ID  : @(#)almd_mem.h	1.1
	Date     :
	Revision History :

	Description :

	Copyright (c) uPRESTO 2011
*******************************************************************************/
#ifndef __ALMD_MEM_H__
#define __ALMD_MEM_H__

#include "almstat.h"
#include "msgdef.h"
#include "mmcdef.h"

#define MAX_STAT_LIST         12
#define MASK_VALUE           128
//#define MI_MASK_NTP_ALM        1		// mmcdef.h ม฿บน

extern int  dIsRcvedMessage( pst_MsgQ pstMsg );
extern int  dMergeBuffer( int dIdx, int dSfd, char* szTcpRmsg, int dRecvLen );
extern int  dRcvPkt_Handle( int dIdx, int dSsfd, st_MngPkt stMyPkt );
extern int  dSend_FSTAT(pst_MsgQ pstMsg, char *data);
extern int  dSend_Traffic_FSTAT(st_MsgQ *pstMsg);
extern void Send_CondMess(int systype, int sysno, int loctype, char invtype, short invno, char almstatus, char oldalm, long long llLoadVal );



#endif /* __ALMD_MEM_H__ */
