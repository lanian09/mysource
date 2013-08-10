#ifndef _SI_DB_SOCK_H_
#define _SI_DB_SOCK_H_

/**
 *	Include headers
 */
// LIB
#include "typedef.h"
#include "nsocklib.h"

/**
 *	Declare functions
 */
extern int dRecvPacket(st_ClientInfo *stSock, int dIndex, st_FDInfo *stFD);
extern int Check_ClientEvent(st_ClientInfo *stSock, st_FDInfo *stFD);
extern int dSendCheck(st_ClientInfo *stSock, int dIdx, st_FDInfo *stFD);
extern int dSendPacket(st_ClientInfo *stSock, int dIdx, st_FDInfo *stFD, char *str, int slen);
extern int dSendBlockPacket(st_ClientInfo *stSock, int dIdx, st_FDInfo *stFD);
extern int dGetSubSysInfo(UINT uiIP);

#endif	/* _SI_DB_SOCK_H_ */
