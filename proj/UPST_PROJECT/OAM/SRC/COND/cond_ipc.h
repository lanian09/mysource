#ifndef __COND_IPC_H__
#define __COND_IPC_H__

#include "clisto.h"		/* U8, OFFSET */
#include "msgdef.h"		/* st_MsgQ */

#define NO_MSG 0

extern int dGetNode(U8 **ppNODE, pst_MsgQ *ppstMsgQ);
extern int dMsgrcv(pst_MsgQ *ppstMsg);
extern int dMsgsnd(int procID, U8 *pNODE);
extern int dMsgsnd2(int procID);

#endif /* __COND_IPC_H__ */
