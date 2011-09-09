#ifndef __IF_MSGQ_H__
#define __IF_MSGQ_H__

#include "msgdef.h"	/* pst_MsgQ */

extern int dIsRcvedMessage(pst_MsgQ pstMsg);
extern int dSndMsgProc(int dSocket, pst_MsgQ pstMsgQ);
#endif /* __IF_MSGQ_H__ */
