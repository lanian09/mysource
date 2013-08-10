
#ifndef __CAPD_MML_H__
#define __CAPD_MML_H__

#include <sys/msg.h>
#include <sys/time.h>
#include <time.h>
#include "capd_global.h"
#include "trclib.h"
#include "capd_msgtypes.h"
#include "utillib.h"
#include "comm_util.h"
#include "rdrheader.h"
#include "comm_trace.h"
#include "ipaf_define.h"

extern char trcBuf[TRCBUF_LEN], trcTmp[TRCTMP_LEN];
extern int trcLogId, trcErrLogId, trcFlag, trcLogFlag;

extern int dMyQid, dIxpcQid;
extern int numMmcHdlr;
extern RDRMmcHdlrVector mmcHdlrVector[MAX_MMC_HANDLER];
extern char	mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN];

//extern int               keepalivelib_init(char *processName);
//extern int               keepalivelib_increase(void);




extern int rdrana_mmcHdlrVector_bsrchCmp (const void *a, const void *b);
extern int rdrana_mmcHdlrVector_qsortCmp (const void *a, const void *b);
extern int getMMCMsg( IxpcQMsgType *rxIxpcMsg );
extern int txMMCResult (IxpcQMsgType *rxIxpcMsg, char *resBuf, char resCode, char contFlag);
extern int rdrana_mmc_sync_rule_file2 (IxpcQMsgType *rxIxpcMsg);
////extern int Send_CondTrcMsg_BLOCK(RDR_BLOCK stBlock, int traceType);
////extern int Send_CondTrcMsg_TR(RDR_TR stTR, int traceType);
extern int Send_CondTrcMsg_BLOCK(RDR_BLOCK stBlock, int traceType, char *ip);
extern int Send_CondTrcMsg_TR(RDR_TR stTR, int traceType, char *ip);

extern int rdr_txMMLResult (IxpcQMsgType *rxIxpcMsg, char *buff, char resCode, char contFlag, unsigned short extendTime, char segFlag, char seqNo);
extern int rdrana_mmc_dis_call_trc (IxpcQMsgType *rxIxpcMsg);
extern int rdrana_mmc_canc_call_trc (IxpcQMsgType *rxIxpcMsg);
extern int rdrana_mmc_reg_call_trc (IxpcQMsgType *rxIxpcMsg);
extern int rdrana_mmc_chg_call_trc (IxpcQMsgType *rxIxpcMsg);

#endif
