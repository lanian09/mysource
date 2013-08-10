#ifndef __COND_MEM_H__
#define __COND_MEM_H__

#include "almstat.h"	/* st_almsts */
#include "msgdef.h"		/* st_MsgQ */
#include "mmcdef.h"		/* st_MngPkt */

#define DEF_FLT_LOG_PATH  "FLT/"
#define DEF_STAT_LOG_PATH "STAT/"
#define DEF_STS_LOG_PATH  "STS/"

extern int dSendMessage(int idx, int dSsfd, int dMsgLen, char *szSmsg);
extern int dRcvPkt_Handle(int dIdx, int dSsfd, st_MngPkt stMyPkt);
extern int dSendFLTDATA(int dSocketFd, int dConTblIdx, int MsgLen, char *szMsg);
extern int dSend_SI_NMS_Alarm(st_almsts *psData);
extern int dSend_SI_NMS(char *psData, size_t szStrLen);
extern int dSend_FSTAT(st_MsgQ *pstMsg);
extern int dIsRcvedMessage(st_MsgQ *pstMsg);

/* cond_main.h */
extern int AppendNonSendMsg(int idx, int dMsgLen, char *szMsg);

#endif /* __COND_MEM_H__ */
