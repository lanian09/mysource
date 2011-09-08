#ifndef __CAPD_MSGTYPES_H__
#define __CAPD_MSGTYPES_H__

#include <stdio.h>
#include <stdlib.h>
//#include <commlib.h>
#include <comm_msgtypes.h>

//일반적으로 사용되는 NAME의 길이
#define COMM_MAX_NAME_LEN	16
#define MAX_MMC_HANDLER 5
#define MAX_IMSI_LIST	5

// MMC 명령어 리스트와 처리 function을 등록하는 table structure
typedef struct {
	char	cmdName[24];
	int		(*func)(IxpcQMsgType*);
} RDRMmcHdlrVector;

extern int rdrana_mmc_dis_call_trc(IxpcQMsgType*);
extern int rdrana_mmc_reg_call_trc(IxpcQMsgType*);
extern int rdrana_mmc_chg_call_trc(IxpcQMsgType*);
extern int rdrana_mmc_canc_call_trc(IxpcQMsgType*);
extern int rdrana_mmcHdlrVector_qsortCmp (const void *a, const void *b);

#endif /*__CAPD_MSGTYPES_H__*/
