#ifndef __TIFB_UTIL_H__
#define __TIFB_UTIL_H__

#include <errno.h>
#include "config.h"
#include "msgdef.h"		/* st_MsgQ */

#define _YES		          0 
#define _NO		             -1
#define MAX_MSGBUF_LEN     1024
#define TIFB_FAIL            -1
#define TIFB_SUCCESS          0

extern int GetYorN(void);
extern int dGetUserPermission(void);
extern int GetProcessID(char *szName);
extern int is_registered_block(char *szComm);
extern void PrintOut(int flag, char *buf);
extern int dGetNode(unsigned char **ppNODE, pst_MsgQ *ppstMsgQ);
extern int dMsgsnd(int procID, unsigned char *pNODE);
extern int init_proc();
extern int dGetBlocks(char *fn, char (*p)[30]);
extern int dGetBlockBIN(char *sBlockName, char *sBinName, int dBinLength);

#endif /* __TIFB_UTIL_H__ */
