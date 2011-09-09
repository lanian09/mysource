#ifndef __SAMD_H__
#define __SAMD_H__

#include <ctype.h>

#include "sysconf.h"
#include "commlib.h"
#include "sfm_msgtypes.h"
#include "stm_msgtypes.h"
#include "sfm_snmp.h"
#include "samd_ntp.h"
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>
#include "comm_almsts_msgcode.h"
#include <proc_version.h>
#include "comm_msgtypes.h"
#include "socklib.h"

#include <sys/types.h>
#include <sys/shm.h>
#include <sock.h>

#include "netOrderL3pdChange.h"
#include "netOrderChange.h"
#include "netOrderSceInfoChange.h"

#ifdef TRU64
#include <sys/old_procfs.h>
#define	SHIFTPAGE		3
#define pagetok(n)		((n) << SHIFTPAGE)
#else
#include <sys/procfs.h>
#endif

#define NEXT(a,b)	((a+1)%b)

#define PING_CNT		2

/* SAMD에서 사용하는 파일들 */
#define PROC_PATH           "/proc"
#define PARENT_PATH         ".."
#define HOME_PATH           "."

#define NTP_DAEMON_DOWN_COUNT	3

#define SAMD_LOG_FILE       "LOG/OAM/samd_log"
#define SAMD_ERRLOG_FILE    "LOG/OAM/samd_err"
#define SAMD_CONF_FILE    "DATA/samd.conf"

#define MAX_SCE_CNT			2
/** T_loc_sadb에서 사용하기 위한 값 시작 */
#define	MAX_CPU_CNT			8
#define	MAX_DISK_CNT		10
/** T_loc_sadb에서 사용하기 위한 값 종료 */

#define NORMAL_FLOW         0
#define AUTO_FLOW           1
#define INIT_FLOW           1
#define KEEPALIVE_CHECK_TIME     60
#define MSGQ_CLEAR_TIME		3

typedef struct {
    double load_avg[3];
    int    *cpu_state;
    int    *memory_state;
} system_info;


#define MAX_STR_LEN     32
#define MAX_FILE_NAME_LEN   128
#define MAX_TBL_CNT     4 // MP mysql DBAC  backup A×AIºi °³¼o 
#define MAX_SYS_CNT     3
typedef struct _DB_INFO_ {
	char    sysName[MAX_STR_LEN];
	char    dbIp[MAX_STR_LEN];
	char    dbName[MAX_STR_LEN];
	char    dbId[MAX_STR_LEN];
	char    dbPass[MAX_STR_LEN];
	int     tblCnt;
	char    tblName[MAX_TBL_CNT][MAX_STR_LEN];
	char    backFile[MAX_FILE_NAME_LEN];
} DB_INFO_t;

/** T_loc_sadb에서 사용하는 status를 위한 값 시작 */

/** T_loc_sadb에서 사용하는 status를 위한 값 종료 */

typedef struct {
	char	procName[16];
	int		msgQkey;
	int		runCnt;
	pid_t	pid;
	char	exeFile[256]; // 실제 실행파일의 full path
	char	startTime[32];
	unsigned char	old_status;
	unsigned char	new_status;
	unsigned char	mask; // killsys로 죽인 경우 설정되어 auto_restart하지 않는다.
} SAMD_ProcessInfo;

extern int g_dStopFlag;
extern void UserControlledSignal(int sign);
extern void IgnoreSignal(int sign);
extern void FinishProgram(void);

extern void ping_test(void);
extern int report_l3pd2FIMD (void);
extern void MMCResSnd (IxpcQMsgType *rxIxpcMsg, char *resBuf, char resCode, char contFlag);
extern int CompareNTPSTS(st_NTP_STS stCurr, pst_NTP_STS pstOld);
extern int report_SCE2FIMD (void);
extern int report_L2SW2FIMD (void);
extern int runShellProgress(char *cmd);
//extern int mmc_makePDStsOutputMsg (IxpcQMsgType *rxIxpcMsg, int devIndex, char *argSys);
extern int mmc_makePDStsOutputMsg (char *trcBuf_loc, int devIndex, char *argSys);  // by sjjeon
extern void strtoupper( char * buff);
extern void MMCReqBypassSnd (IxpcQMsgType *rxIxpcMsg);
extern int getProcessID (char *procName);
extern int report_SCE2FIMD (void);
extern int HandleRxMsg();
extern int getListID (char *name);
extern void doDisPrcSts(IxpcQMsgType *rxIxpcMsg );
extern void doKillPrc(IxpcQMsgType *rxIxpcMsg );
extern void doRunPrc(IxpcQMsgType *rxIxpcMsg );
extern void doDisLoadSts(IxpcQMsgType *rxIxpcMsg );
extern void doDisLanSts (IxpcQMsgType *rxIxpcMsg);
extern void doDisPdSts (IxpcQMsgType *rxIxpcMsg);
extern void doDisNTPSts(IxpcQMsgType *rxIxpcMsg);
extern void doDisDUPSts(IxpcQMsgType *rxIxpcMsg );
extern void doSetAutoMode(IxpcQMsgType *rxIxpcMsg );
extern void doSetDUPSts(IxpcQMsgType *rxIxpcMsg );
extern void doSetRuleSce (IxpcQMsgType *rxIxpcMsg);
extern void doDisSysVer(IxpcQMsgType *rxIxpcMsg);
extern void doDisFlowCnt(IxpcQMsgType *rxIxpcMsg);
extern void doClrAlm_SysMsg(IxpcQMsgType *rxIxpcMsg);	// by june, 20100312
extern int HandleStatistics(GeneralQMsgType *rxGenQMsg);

extern void doClrScmFaultSts(IxpcQMsgType *rxIxpcMsg); // 20100928 by dcham

extern int samd_initLog (void);
extern int samd_getL3pd (int key);
extern int samd_getSCE (int key);
extern int samd_get_snmp_sce_ipaddress(char *fname);
extern int samd_get_snmp_rdr_ipaddress(char *fname);
extern int	init_SCE_snmp();
extern int samd_getL2SW (int key);
extern int samd_get_snmp_L2_ipaddress(char *fname);
extern int samd_initPingAddrTbl (char *fname);
extern int samd_initProcessTbl (char *fname);
extern long getSumOfPIDs (void);
extern int getProcessStatus(void);
extern int runProcess (int procIdx, int isAuto);
extern void report_sadb2FIMD ();
extern int InitSys (void);
extern void get_system_information (system_info *system_information, int flag);

extern void thread_ping_test (void);
extern void thread_get_TAP_Info(void);
extern void thread_get_sce_info(void);
extern void thread_get_l2sw_info(void);

extern void checkProcessStatus(void);
extern void checkKeepAlive (void);
extern int get_diskUsage (void);
extern int get_queUsage (void);
extern int CheckNTPStS();
extern void QClear ();
extern int ping_test_with_ip (char *ipaddr);

extern char         l3pd_IPaddr[MAX_PROBE_DEV_NUM][20];
extern int samd_get_TAP_ipaddress(char *fname);
extern int report_SceFlow_Samd2Stmd (int flow_num, int sce_idx);

extern char trcBuf[4096], trcBufSig[4096];
#endif //  __SAMD_H__
