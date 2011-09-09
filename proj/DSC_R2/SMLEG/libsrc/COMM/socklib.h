#ifndef __SOCKLIB_H__
#define __SOCKLIB_H__

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <signal.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stropts.h>
#include <string.h>
#include <fcntl.h>
#include <sys/errno.h>

#define SOCKLIB_MAX_BIND_CNT			4
#define SOCKLIB_MAX_SERVER_CNT			16
#define SOCKLIB_MAX_CLIENT_CNT			64

#define SOCKLIB_POLL_TIMER				1000
#define SOCKLIB_SEG_BUF_SIZE			32768

#define SOCKLIB_NO_EVENT				0
#define SOCKLIB_NEW_CONNECTION			1
#define SOCKLIB_CLIENT_MSG_RECEIVED		2
#define SOCKLIB_SERVER_MSG_RECEIVED		3
#define SOCKLIB_CLIENT_DISCONNECTED		4
#define SOCKLIB_SERVER_DISCONNECTED		5
#define SOCKLIB_INTERNAL_ERROR			6


typedef struct {
	int		bodyLen;
	int	    mapType;
	char	segFlag;
	char	seqNo;		
} SockLibHeadType;


#define SOCKLIB_HEAD_LEN		sizeof(SockLibHeadType)
#define SOCKLIB_MAX_BODY_LEN	8192-SOCKLIB_HEAD_LEN


typedef struct {
	SockLibHeadType	head;
	char			body[SOCKLIB_MAX_BODY_LEN];
} SockLibMsgType;


typedef struct {
	int			fd;
	int			port;
} BindSockFdContext;

typedef struct {
	int			fd;
	int			port;
	struct sockaddr_in	srvAddr;
	time_t		tStamp;
} ServerSockFdContext;

typedef struct {
	int			fd;
	int			port;
	struct sockaddr_in	cliAddr;
	time_t		tStamp;
} ClientSockFdContext;


extern int errno;

extern int socklib_setAllowClientAddrTbl (char*);
extern int socklib_initTcpBind (int);
extern int socklib_connect (char*, int);
extern int socklib_acceptNewConnection (int, int);
extern int socklib_disconnectSockFd (int);
extern int socklib_disconnectServerFd (int);
extern int socklib_disconnectClientFd (int);
extern int socklib_action (char*, int*);
extern int socklib_readSockFd (char*, int);
extern int socklib_sndMsg (int, char*, int);
extern int socklib_broadcast (char*, int);
extern int socklib_broadcast2Servers (char*, int);
extern int socklib_broadcast2Clients (char*, int);
extern int socklib_setSockOption (int);
extern int socklib_setNonBlockMode (int);
extern int socklib_addBindSockFdTbl (int, int);
extern int socklib_addServerSockFdTbl (struct sockaddr_in *, int, int);
extern int socklib_addClientSockFdTbl (struct sockaddr_in *, int, int);
extern int socklib_delServerSockFdTbl (int);
extern int socklib_delClientSockFdTbl (int);
extern int socklib_addSockFdSet (int);
extern int socklib_delSockFdSet (int);
extern int socklib_pollFdSet (void);
extern int socklib_lookupBindSockFdTbl (int*, int*);
extern int socklib_lookupServerSockFdTbl (void);
extern int socklib_lookupClientSockFdTbl (void);
extern int socklib_ping (char*);

#endif /*__SOCKLIB_H__*/
