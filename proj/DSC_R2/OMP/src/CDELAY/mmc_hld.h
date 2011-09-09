

#ifndef __MMC_HDL_H__
#define __MMC_HDL_H__

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>


#include "commlib.h"
#include "comm_msgtypes.h"


// MMC 명령어 리스트와 처리 function을 등록하는 table
#define CDELAY_MAX_MMC_HANDLER    35     
typedef struct {
    char    cmdName[COMM_MAX_NAME_LEN*2];
    int     (*func)(IxpcQMsgType*);
} COMM_MMCHdlrVector;






extern int cdelay_mmc_start_delay_check (IxpcQMsgType *rxIxpcMsg);
extern int cdelay_mmc_stop_delay_check (IxpcQMsgType *rxIxpcMsg);

extern int comm_txMMLResult ( int SendQid, IxpcQMsgType *rxIxpcMsg, char *resBuf, char resCode, char contFlag, unsigned short extendTime, char segFlag, char seqNo);
extern int mmcd_exeRxQMsg (GeneralQMsgType *rxGenQMsg);



#endif
