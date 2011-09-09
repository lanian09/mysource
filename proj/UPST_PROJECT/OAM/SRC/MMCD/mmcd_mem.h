/**
	@file		mmcd_mem.h
	@author
	@version
	@date		2011-07-14
	@brief		mmcd_mem.c 헤더파일
*/

#ifndef __MMCD_MEM_H__
#define __MMCD_MEM_H__

/**
 *	Include headers
 */
// DQMS
#include "msgdef.h"

/**
	Declare functions
*/
enum {
	TYPE_TREND_INFO	= 0,
	TYPE_DEFECT_INFO
};

extern int dRcvPkt_Handle(int dIdx, int dSsfd, st_MngPkt stMyPkt);
extern int dMergeBuffer(int dIdx, int dSfd, char* szTcpRmsg, int dRecvLen);
extern int dIsRcvedMessage(pst_MsgQ *ppstMsg);
extern int SendToPROC(st_MsgQ *in_msg, int MsgID);

#endif /* __MMCD_MEM_H__ */
