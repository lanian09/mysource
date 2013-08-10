/*******************************************************************************
			DQMS Project

	Author   :
	Section  : SI_SVCMON
	SCCS ID  : @(#)si_svcmon_sock.h	1.1
	Date     : 01/21/05
	Revision History :

	Description :

	Copyright (c) uPRESTO 2005
*******************************************************************************/
#ifndef __SI_SVCMON_SOCK_H__
#define __SI_SVCMON_SOCK_H__

#include "nsocklib.h"

extern int dRecvPacket(st_ClientInfo *stSock, int dIndex, st_FDInfo *stFD);
extern int Check_ClientEvent(st_ClientInfo *stSock, st_FDInfo *stFD);
extern int dSendCheck(st_ClientInfo *stSock, int dIdx, st_FDInfo *stFD);
extern int dSendPacket(st_ClientInfo *stSock, int dIdx, st_FDInfo *stFD, char *str, int slen);
extern int dSendBlockPacket(st_ClientInfo *stSock, int dIdx, st_FDInfo *stFD);
extern int dGetSubSysInfo(UINT uiIP);


#endif	/*	__SI_SVCMON_SOCK_H__	*/

