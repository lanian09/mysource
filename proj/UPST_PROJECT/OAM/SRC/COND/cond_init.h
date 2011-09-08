#ifndef __COND_INIT_H__
#define __COND_INIT_H__

#include "cond_msg.h"		/* pst_CondCount */
#include "mmcdef.h"		    /* st_MngPkt */

#ifdef _ENABLE_HEARTBEAT_
 #define HEARTBEAT_TIMEOUT   (60*5)
#endif /* _ENABLE_HEARTBEAT_ */

extern int AppendNonSendMsg(int idx, int dMsgLen, char *szMsg);
extern int SendToOMP( unsigned short usSvcID, unsigned short usMsgID, pst_CondCount pstCnt, int MsgLen, st_MngPkt *pstSpkt );
extern int SendToIF( st_almsts *palm, unsigned char ucTAFID, unsigned char ucTAMID );
extern int dSocketCheck(void);

extern void SetUpSignal(void);
extern void	UserControlledSignal(int sign);
extern void	FinishProgram(void);
extern void	IgnoreSignal(int sign);
extern int	dGetHostName(char *szNTAMName);
extern int  dGetBlocks(char *fn, char (*p)[30]);
extern int  dInitIPCs(void);
extern int  dGetSysNo(void);
extern int  Init_Fidb(void);

#endif /* __COND_INIT_H__ */
