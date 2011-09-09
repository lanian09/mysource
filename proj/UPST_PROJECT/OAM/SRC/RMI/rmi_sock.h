#ifndef _RIM_SOCK_H_
#define _RIM_SOCK_H_

/**
 *	INCLUDE HEADER FILES
 */
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

// OAM/INC
#include "mmcdef.h"				/* st_MSG_BUF */
#include "sockio.h"				/* MAGIC_NUMBER */

// .
#define MI_LOGIN		8
#define MI_LOGOUT		9

/**
 *	DECLARE VARIABLES
 */
st_MSG_BUF    gstMsgBuf;
fd_set  stRfds;
int dSfd;

extern time_t tCheck_time;
extern int errno;
extern int dSfd;
extern st_MngPkt cli_msg;
extern char SmsName[];
extern char szPriAddr[64];

/**
 *	DECLARE FUNCTIONS
 */

extern int	dRecvMessage(int dRsfd, char *szTcpRmsg, int *dRecvLen);
extern int	dMergeBuffer(int dSfd, char *szTcpRmsg, int dRecvLen);
extern int	dRcvPkt_Handle(int dSsfd, st_MngPkt stMyPkt);
extern void	SockCheck(int dSig);
extern int  dSockCheck(void);
extern void	logout_win(void);
extern int	send_login_sts(int msgtype, char *result);
extern int  dSockInit(char *szIPAddr);


#endif	/* _RIM_SOCK_H_ */
