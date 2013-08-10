
#ifndef __MMC_HDL_H__
#define __MMC_HDL_H__

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/msg.h>

#include "trclib.h"
#include "comm_msgtypes.h"


// MMC ��ɾ� ����Ʈ�� ó�� function�� ����ϴ� table
#define CDELAY_MAX_MMC_HANDLER    35     
typedef struct {
    char    cmdName[COMM_MAX_NAME_LEN*2];
    int     (*func)(IxpcQMsgType*);
} COMM_MMCHdlrVector;


extern void mmc_yh_init ();

/* mmc function prototype */
extern int mmcd_exeMMCMsg (IxpcQMsgType *rxIxpcMsg);
extern int comm_txMMLResult ( int SendQid, IxpcQMsgType *rxIxpcMsg, char *resBuf, char resCode, char contFlag, unsigned short extendTime, char segFlag, char seqNo);
extern int mmc_prc_sample1 (IxpcQMsgType *rxIxpcMsg);

#endif
