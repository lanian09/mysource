#ifndef __CHSMD_FUNC_H__
#define __CHSMD_FUNC_H__

#include "clisto.h"		/* U8, OFFSET */
#include "msgdef.h"		/* st_MsgQ */

extern int dGetNode(U8 **ppNODE, pst_MsgQ *ppstMsgQ);
extern int dMsgsnd(int procID, U8 *pNODE);
extern int dMsgrcv(pst_MsgQ *ppstMsg);

#endif /* __CHSMD_FUNC_H__ */
