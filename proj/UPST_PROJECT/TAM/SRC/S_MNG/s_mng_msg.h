#ifndef __S_MNG_MSG_H__
#define __S_MNG_MSG_H__

#include "msgdef.h"			/* st_MsgQ */
#include "mmcdef.h"			/* mml_msg, dbm_mst_t */

extern int dIsReceivedMessage(pst_MsgQ *ppstMsgQ);
extern int dSendMsg(pst_MsgQ pstMsg, int dSeqProcID);
extern int dSendMMC(mml_msg *mmsg, dbm_msg_t *smsg, long long llNID);
#endif /* __S_MNG_MSG_H__ */
