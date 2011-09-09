#ifndef __COND_SOCK_H__
#define __COND_SOCK_H__

extern int dSendMessage(int idx, int dSsfd, int dMsgLen, char *szSmsg);
extern int dRecvMessage(int dRsfd, char *szTcpRmsg, int *dRecvLen);
extern int dTcpSvrModeInit(void);

#endif /* __COND_SOCK_H__ */
