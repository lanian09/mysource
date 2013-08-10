/**
	@file		mmcd_sock.h
	@author		
	@version
	@date		2011-07-14
	@brief		mmcd_sock.c 헤더파일
*/

#ifndef __MMCD_SOCK_H__
#define __MMCD_SOCK_H__

#include "sockio.h"
#include "mmcdef.h"

/**
	Declare functions
*/



extern int dTcpSvrModeInit(void);
extern int dRecvMessage(int dRsfd, char *szTcpRmsg, int* dRecvLen);
extern int dSendBlockMessage(int dSsfd, int dMsgLen, char *szSmsg, int stConIdx);

#endif	/* __MMCD_SOCK_H__ */
