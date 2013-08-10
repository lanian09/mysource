
#ifndef __MMC_HDL_H__
#define __MMC_HDL_H__

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/msg.h>

#include "sm_subs_info.h"
#include "trclib.h"
#include "comm_msgtypes.h"


// MMC 명령어 리스트와 처리 function을 등록하는 table
#define CDELAY_MAX_MMC_HANDLER    35     
typedef struct {
    char    cmdName[COMM_MAX_NAME_LEN*2];
    int     (*func)(IxpcQMsgType*);
} COMM_MMCHdlrVector;


extern void mmc_yh_init ();

/* mmc function prototype */
extern int mmcd_exeMMCMsg (IxpcQMsgType *rxIxpcMsg);
extern int comm_txMMLResult ( int SendQid, char *resBuf, char resCode, char contFlag, unsigned short extendTime, char segFlag, char seqNo);
extern int mmcr_mmc_dis_sess_info (IxpcQMsgType *rxIxpcMsg);
extern int cdelay_mmc_stop_delay_check (IxpcQMsgType *rxIxpcMsg);
extern int mmc_sync_rule_file (IxpcQMsgType *rxIxpcMsg);
extern int mmc_sce_log_in (IxpcQMsgType *rxIxpcMsg);
extern int mmc_sce_log_out (SUBS_INFO *psi, int succfail, int dErrCode);
extern int mmc_dis_rule_use(IxpcQMsgType *rxIxpcMsg);
extern int mmc_set_rule_use(IxpcQMsgType *rxIxpcMsg);
extern int mmc_del_rule_use(IxpcQMsgType *rxIxpcMsg);
extern int mmc_dis_sm_tmr(IxpcQMsgType *rxIxpcMsg);
extern int mmc_set_sm_tmr(IxpcQMsgType *rxIxpcMsg);
extern int mmc_set_ovld_ctrl(IxpcQMsgType *rxIxpcMsg);
extern int mmc_dis_ovld_ctrl(IxpcQMsgType *rxIxpcMsg);
extern int mmc_dis_rleg_sess(IxpcQMsgType *rxIxpcMsg);
// BY JUNE, 2011-03-16
extern int mmc_set_smbuf_clr(IxpcQMsgType *rxIxpcMsg);
extern int mmc_del_smbuf_clr(IxpcQMsgType *rxIxpcMsg);
extern int mmc_dis_smbuf_clr(IxpcQMsgType *rxIxpcMsg);

#endif
