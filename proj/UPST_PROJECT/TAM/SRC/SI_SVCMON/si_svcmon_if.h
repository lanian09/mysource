#ifndef __SI_SVCMON_IF_H__
#define __SI_SVCMON_IF_H__  

#include <errno.h>
#include "msgdef.h"		/* st_MsgQ */

extern int dGetNode(unsigned char **ppNODE, pst_MsgQ *ppstMsgQ);
extern int dMsgsnd(int procID, unsigned char *pNODE);
extern int dMsgrcv(pst_MsgQ *ppstMsg);

#endif /* __QMON_ALMD_H__ */
