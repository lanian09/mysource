/*******************************************************************************
			DQMS Project

	Author   :
	Section  : ALMD
	SCCS ID  : @(#)almd_sock.h	1.1
	Date     :
	Revision History :

	Description :

	Copyright (c) uPRESTO 2005
*******************************************************************************/
#ifndef __ALMD_SOCK_H__
#define __ALMD_SOCK_H__

#include "filedb.h"
#include "mmcdef.h"
#include "sockio.h"

extern int dTcpSvrModeInit(void);
extern int dMakeNTAMMngPkt( st_MngPkt *stPkt, pst_NTAM pstNTAM, int dHostNo);
extern int dMakeSTATMngPkt( st_MngPkt *stPkt, int dBodyLen );
extern int dMakeDirectMngPkt(st_MngPkt *stPkt, st_DIRECT_MNG *pstDIRECT);
extern int dRecvMessage( int dRsfd, char* szTcpRmsg, int* dRecvLen );
extern int dSendMessage(int dSsfd, int dMsgLen, char *szSmsg, int stConIdx);

#endif /* __ALMD_SOCK_H__ */
