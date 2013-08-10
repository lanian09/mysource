#ifndef __IF_SOCK_H__
#define __IF_SOCK_H__

#include "sockio.h"		/* st_NTAFTHeader */

#define SOCK_TRUE		0
#define SOCK_FALSE		2
#define MAX_RETRY_CNT	3

extern int writeSocket(int dSocket, void *buffer, int slen);
extern int dHandleMsg(st_NTAFTHeader *pstHeader, char *szBuf);
extern int dInit_Tcp_Client(int *dServerSocket);
extern void write_proc(int fd);
extern int dRecvPacket(int dSocket);

#endif /* __IF_SOCK_H__ */
