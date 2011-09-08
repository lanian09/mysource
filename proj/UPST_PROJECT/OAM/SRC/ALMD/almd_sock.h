/*******************************************************************************
			DQMS Project

	Author   :
	Section  : ALMD
	SCCS ID  : @(#)almd_sock.h	1.1
	Date     :
	Revision History :

	Description :

	Copyright (c) uPRESTO 2011
*******************************************************************************/
#ifndef __ALMD_SOCK_H__
#define __ALMD_SOCK_H__

#include "mmcdef.h"
#include "filedb.h"

#define MAX_STAT_LIST         12
#define MASK_VALUE           128
//#define MI_MASK_NTP_ALM        1		// mmcdef.h ม฿บน
#define MAX_ALMDBUF_LEN 69918720

extern int dTcpSvrModeInit(void);
extern int dMakeNTAMMngPkt( st_MngPkt *stPkt, pst_NTAM pstNTAM, int dHostNo);
extern int dMakeDirectMngPkt(st_MngPkt *stPkt, st_DIRECT_MNG *pstDIRECT);
extern int dMakeSwitchMngPkt(st_MngPkt *stPkt, st_SWITCH_MNG *pstSWITCH);
extern int dMakeSTATMngPkt( st_MngPkt *stPkt, int dBodyLen );
extern int dRecvMessage( int dRsfd, char* szTcpRmsg, int* dRecvLen );
extern int dSendMessage( int dSsfd, int dMsgLen, char *szSmsg, int stConIdx );
extern int SendMess(mml_msg *mmsg, dbm_msg_t *smsg);

#endif /* __ALMD_SOCK_H__ */
