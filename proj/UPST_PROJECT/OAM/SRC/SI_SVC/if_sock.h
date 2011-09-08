#ifndef __IF_SOCK_H__
#define __IF_SOCK_H__

#include "nsocklib.h"		/* st_ClientInfo, st_FDInfo */

#define MAX_CHANNEL_TIMEOUT	15

extern int dGetSubSysInfo(unsigned int uiIP);
extern int dSendBlockPacket(st_ClientInfo *stSock, int dIdx, st_FDInfo *stFD);
extern int Check_ClientEvent(st_ClientInfo *stSock, st_FDInfo *stFD);
extern int dSendCheck(st_ClientInfo *stSock, int dIdx, st_FDInfo *stFD);
extern int dInitSockFd(st_FDInfo *stFD, int dPort);
extern int dSendPacket(st_ClientInfo *stSock, int dIdx, st_FDInfo *stFD, char *str, int slen);
extern int dAcceptSockFd(st_ClientInfo *stSock, st_FDInfo *stFD, int *pdPos);
extern int dDisConnSock(st_ClientInfo *stSock, int dIdx, st_FDInfo *stFD);
#endif /* __IF_SOCK_H__ */
