

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

extern int mmcr_mmc_dis_cps_info (IxpcQMsgType *rxIxpcMsg);
extern int mmcr_mmc_dis_sess (IxpcQMsgType *rxIxpcMsg);
extern int mmcr_mmc_dis_sess_cnt (IxpcQMsgType *rxIxpcMsg);
extern int mmcr_mmc_dis_sess_list (IxpcQMsgType *rxIxpcMsg);
extern int mmcr_mmc_dis_rule_info (IxpcQMsgType *rxIxpcMsg);
extern int cdelay_mmc_stop_delay_check (IxpcQMsgType *rxIxpcMsg);
extern int mmcr_mmc_dis_tmr_info (IxpcQMsgType *rxIxpcMsg);
extern int mmcr_mmc_set_tmr_info (IxpcQMsgType *rxIxpcMsg);
extern int mmcr_mmc_reg_call_trc (IxpcQMsgType *rxIxpcMsg);
extern int mmcr_mmc_canc_call_trc (IxpcQMsgType *rxIxpcMsg);
extern int mmcr_mmc_dis_call_trc (IxpcQMsgType *rxIxpcMsg);
extern int mmcr_mmc_set_smbuf_clr (IxpcQMsgType *rxIxpcMsg);
extern int mmcr_mmc_del_smbuf_clr (IxpcQMsgType *rxIxpcMsg);
extern int mmcr_mmc_dis_smbuf_clr (IxpcQMsgType *rxIxpcMsg);
extern int mmcr_mmc_sync_rule_file (IxpcQMsgType *rxIxpcMsg);
extern int mmcr_mmc_sce_log_out (IxpcQMsgType *rxIxpcMsg);
extern int mmcr_mmc_dis_rule_use (IxpcQMsgType *rxIxpcMsg);
extern int mmcr_mmc_set_rule_use (IxpcQMsgType *rxIxpcMsg);
extern int mmcr_mmc_del_rule_use (IxpcQMsgType *rxIxpcMsg);
extern int mmcr_mmc_set_ovld_ctrl (IxpcQMsgType *rxIxpcMsg);
extern int mmcr_mmc_dis_ovld_ctrl (IxpcQMsgType *rxIxpcMsg);
extern int mmcr_mmc_dis_rleg_sess (IxpcQMsgType *rxIxpcMsg);
extern int mmcr_mmc_dis_call_info (IxpcQMsgType *rxIxpcMsg);


extern int comm_txMMLResult ( int SendQid, IxpcQMsgType *rxIxpcMsg, char *resBuf, char resCode, char contFlag, unsigned short extendTime, char segFlag, char seqNo);



#endif
