#ifndef _CI_LOG_SOCK_H_
#define _CI_LOG_SOCK_H_

/**
 *	Include headers
 */
#define DEF_SOCK_BUF_SIZE       8192000
#define MAX_RETRY_CNT			3

#define SOCK_TRUE 	0
#define SOCK_FALSE 	2

/**
 *	Declare func.
 */
extern int dRecvPacket(int dSocket);
extern int dInit_CILOG_Client(int *dServerSocket);
extern int dSendToProc(int dLen, char *szBuf);
extern void write_proc(int fd);
extern int writeSocket(int dSocket, char *str, int slen);
extern int dGetIPAddr( char *conf_file, char *primary_addr, char *secondary_addr );

#endif	/* _CI_LOG_SOCK_H_ */
