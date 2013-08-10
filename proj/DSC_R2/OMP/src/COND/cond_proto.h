#ifndef __COND_PROTO_H__
#define __COND_PROTO_H__

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/ecppio.h>
#include <sys/bpp_io.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#include <commlib.h>
#include <sysconf.h>
#include <comm_msgtypes.h>
#include <comm_almsts_msgcode.h>
#include <sfmconf.h>
#include <sfm_msgtypes.h>
#include <omp_filepath.h>
#include <proc_version.h>

#include <sfm_snmp.h>
#include <stm_msgtypes.h>


#define        MAX_INH_MSG_CNT       200 
#define        INH_MSG_NUM_LEN       8
#define        INH_MSG_INFO_LEN      1024

#define        MSG_INHIB             1
#define        MSG_ALLOW             0

typedef struct {
    char    msgFlag[MAX_INH_MSG_CNT];
    char    msgType[MAX_INH_MSG_CNT][INH_MSG_NUM_LEN];
    char    msgNum[MAX_INH_MSG_CNT][INH_MSG_NUM_LEN];
    char    msgInfo[MAX_INH_MSG_CNT][INH_MSG_INFO_LEN];
}InhMsgTbl;

extern int errno;

extern int cond_initial (void);
extern int cond_initLog (void);

extern int cond_exeRxQMsg (GeneralQMsgType*);
extern int cond_exeConsoleMsg (GeneralQMsgType*);
extern int cond_exeMMCMsg (IxpcQMsgType*);
extern int cond_exeRxSockMsg (int, SockLibMsgType*);
extern int cond_exeNewConn (int);
extern int cond_exeDisconn (int);

extern void *cond_mmc_srch_log_his (void*);
extern int cond_srch_log_his_checkParaTimeValue (char*, int*, int*, int*, int*, int*);
extern void cond_srch_log_his_searchLogFile (IxpcQMsgType*, char*, char*, char*, int*, char*);
extern int cond_srch_log_his_isTimeRange (char*, char*, char*);

extern int cond_txConsoleMsg2GUI (GeneralQMsgType*);
extern int cond_txConsoleMsg2nmsib (GeneralQMsgType*);
extern int cond_txMMLResult (IxpcQMsgType*, char*, char, char, unsigned short, char, char);

extern void cond_transmit_initReqMsg();

extern int cond_txConsoleMsg2nmsif (GeneralQMsgType *rxGenQMsg);
extern void *cond_mmc_alw_msg (void *arg);
extern void *cond_mmc_dis_inh_msg (void *arg);
extern int Load_InhMsg_Info(void *arg);
extern void *cond_mmc_inh_msg(void *arg);
extern int Write_InhMsg_Info (InhMsgTbl *InhMsgt);

#endif //__COND_PROTO_H__
