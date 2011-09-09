#ifndef __M_SVCMON_MSGQ_H__
#define __M_SVCMON_MSGQ_H__

#include "msgdef.h"

extern int dIsReceivedMessage(st_MsgQ *pstMsgQ);
extern int dSendMsg(int procid, st_MsgQ *pstMsgQ);

#endif /* __M_SVCMON_MSGQ_H__ */
