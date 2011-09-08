#ifndef __SOCKLIB_H__
#define __SOCKLIB_H__

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stropts.h>
#include <string.h>
#include <fcntl.h>
#include <sys/errno.h>

#include "trclib.h"

#define SOCKLIB_MAX_BIND_CNT			4
#define SOCKLIB_MAX_SERVER_CNT			16
#define SOCKLIB_MAX_CLIENT_CNT			32	//64 -> 32

#define SOCKLIB_POLL_TIMER				1000
#define SOCKLIB_SEG_BUF_SIZE			1024*32
#define SOCKLIB_SEG_MAX_BUF_SIZE		1024*1024
#define SOCKLIB_RETRY_CNT			100
#define SOCKLIB_PRINT_TO		stdout
//#define SOCKLIB_PRINT_TO		stderr

#define SOCKLIB_NO_EVENT				0
#define SOCKLIB_NEW_CONNECTION			1
#define SOCKLIB_CLIENT_MSG_RECEIVED		2
#define SOCKLIB_SERVER_MSG_RECEIVED		3
#define SOCKLIB_CLIENT_DISCONNECTED		4
#define SOCKLIB_SERVER_DISCONNECTED		5
#define SOCKLIB_INTERNAL_ERROR			6


// 메시지 큐 통신에 사용되는 mtype이 long으로 선언되어야 하는데, 시스템에 따라 long이
//	4byte 또는 8byte로 다르다.
// - 이를 위해 ixpc에서 msgQ로 받은 메시지 중 mtype을 떼어내 socket_header에 4byte로
//	넣어서 보내고 수신측에서 다시 떼어 붙여서 msgQ로 보낸다.
typedef struct {
	int		bodyLen;
	int		mtype;   // 시스템에 따라 long이 4byte 또는 8byte로 다르다.
} SockLibHeadType;

#define SOCKLIB_HEAD_LEN		sizeof(SockLibHeadType)
#define SOCKLIB_MAX_BODY_LEN	4096-SOCKLIB_HEAD_LEN
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
extern int socklib_readSockFd (char*, int, char*);
extern int socklib_sndMsg (int, char*, int);
extern int socklib_broadcast (char*, int);
extern int socklib_broadcast2Servers (char*, int);
extern int socklib_broadcast2Clients (char*, int);
extern int socklib_setSockOption (int);
extern int socklib_setNonBlockMode (int, int);
extern int socklib_addBindSockFdTbl (int, int);
extern int socklib_addServerSockFdTbl (struct sockaddr_in *, int, int);
extern int socklib_addClientSockFdTbl (struct sockaddr_in *, int, int);
extern int socklib_delServerSockFdTbl (int);
extern int socklib_delClientSockFdTbl (int);
extern int socklib_addSockFdSet (int);
extern int socklib_delSockFdSet (int);
extern int socklib_pollFdSet (void);
extern int socklib_lookupBindSockFdTbl (int*, int*);
extern int socklib_lookupServerSockFdTbl (char*);
extern int socklib_lookupClientSockFdTbl (char*);
extern int socklib_ping (char*);
extern void commlib_microSleep (int);

#endif /*__SOCKLIB_H__*/
