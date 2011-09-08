#ifndef __SAMD_H__
#define __SAMD_H__


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/statvfs.h>
#include <sys/procfs.h>

#include <sys/file.h>
#include <sys/swap.h>
#include <sys/sysinfo.h>

/*
#include <sys/mnttab.h>
#include <sys/vfstab.h>
#include <sys/old_procfs.h>
#include <kvm.h>
#include <sys/proc.h>
#include <sys/var.h>
#include <sys/cpuvar.h>
#include <kstat.h>
*/

#include "comm_define.h"
#include "sysconf.h"
#include "comm_msgtypes.h"
#include "commlib.h"
#include "sfm_msgtypes.h"
#include "stm_msgtypes.h"

#include "comm_almsts_msgcode.h"
#include "sfm_snmp.h" // by helca
#include "samd_ntp.h"
#include "ipaf_names.h"


#ifdef TRU64
#define	SHIFTPAGE		3
#define pagetok(n)		((n) << SHIFTPAGE)
#endif

#define NEXT(a,b)	((a+1)%b)

/* SAMD에서 사용하는 파일들 */
#define PROC_PATH           "/proc"
#define PARENT_PATH         ".."
#define HOME_PATH           "."

#define NTP_DAEMON_DOWN_COUNT   3
#define START_PRC 1
#define KILL_PRC  0

#define MESSAGE_FILE        "/var/log/messages"
#define MESSAGE_FILE_1      "/var/log/messages.1"

#define SAMD_LOG_FILE       "APPLOG/OAM/samd_log"
#define SAMD_ERRLOG_FILE    "APPLOG/OAM/samd_err"

/* ping configuration file */
#define AAA_CONF_FILE			"NEW/DATA/AAAIF.conf"
#define AN_AAA_CONF_FILE		"NEW/DATA/AN_AAAIF.conf"
#define UAWAP_GW_CONF_FILE		"NEW/DATA/UAWAP_GW.conf"
//#define SCE_CONF_FILE   		"NEW/DATA/SCE_INFO.conf"  // ==> RMT_LAN_INFO.conf 변경..sjjeon
#define RMT_LAN_CONF_FILE  		"NEW/DATA/RMT_LAN_INFO.conf"
#define SAMD_CONF_FILE			"NEW/DATA/samd.conf" // 2009.05.22 jjinri DB INFO

/* NO TRANSMITTED UDR FILE */
#define AAA_FAIL			 "/BSD/LOG/HEADLOG/AAA_FAIL"

/* rsync location */
//yhshin #define RSYNC       "/usr/bin/rsync"
#define RSYNC       "/usr/local/bin/rsync"

/** T_loc_sadb에서 사용하기 위한 값 시작 */
#define	MAX_CPU_CNT			8
#define	MAX_DISK_CNT		10
/** T_loc_sadb에서 사용하기 위한 값 종료 */

#define NORMAL_FLOW         	0
#define AUTO_FLOW           	1
#define INIT_FLOW           	1
#define KEEPALIVE_CHECK_TIME    60
#define MSGQ_CLEAR_TIME			5
#define MSGQ_CLEAR_CNT			150000

typedef struct {
    double load_avg[3];
    int    *cpu_state;
    int    *memory_state;
} system_info;

#define MAX_STR_LEN		32
#define MAX_FILE_NAME_LEN	128
#define MAX_TBL_CNT		4 // MP mysql DB의  backup 테이블 개수 
#define MAX_SYS_CNT		3
typedef struct _DB_INFO_ {
	char	sysName[MAX_STR_LEN];
	char	dbIp[MAX_STR_LEN];
	char	dbName[MAX_STR_LEN];
	char	dbId[MAX_STR_LEN];
	char	dbPass[MAX_STR_LEN];
	int		tblCnt;
	char	tblName[MAX_TBL_CNT][MAX_STR_LEN];
	char	backFile[MAX_FILE_NAME_LEN];
} DB_INFO_t;

// RESTORE 
#define MAX_KILL_LIST	8
typedef struct _KILL_ {
	int		procCnt;
	char	sysName[MAX_STR_LEN];
	char	procName[MAX_KILL_LIST][MAX_STR_LEN];
} KILL_LIST_t;



/** T_loc_sadb에서 사용하는 status를 위한 값 시작 */

/** T_loc_sadb에서 사용하는 status를 위한 값 종료 */

/* // 2006.06.26 by helca
typedef struct {
	char	procName[16];
	int		msgQkey[SYSCONF_MAX_APPL_MSGQ_NUM];
	int		runCnt;
	pid_t	pid;
	char	exeFile[256]; // 실제 실행파일의 full path
	char	startTime[32];
	unsigned char	old_status;
	unsigned char	new_status;
	unsigned char	mask; // killsys로 죽인 경우 설정되어 auto_restart하지 않는다.
} SAMD_ProcessInfo;
*/

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

#if 0
typedef struct {
	char	procName[16];
	int		msgQkey[SYSCONF_MAX_APPL_MSGQ_NUM];
} SAMD_AuxilaryInfo;
#endif
#if 1 // by helca
typedef struct {
	char	procName[16];
	int		msgQkey;
} SAMD_AuxilaryInfo;
#endif
typedef struct {
	char	procName[16];
	char	exeFile[256]; // 실제 실행파일의 full path
	unsigned char	status;
} SAMD_DaemonInfo;

/* define 2004. 10. 1 */
#define	MAX_PARTITION_NUM		20

typedef struct {
	int		cnt;
	char	name[MAX_PARTITION_NUM][20];
} DiskConfig;

typedef struct
{
    //long long cpustate[4];
    long long tot;
    long long state[4];
    //long long newstate[4];
    //long long diffstate[4];
}
cpu_stat;

typedef struct
{
    int         ncnt;
    cpu_stat    stst[8];
}
CpuInfo;

typedef struct
{
	char	side;
	char	name[20];
}OpticalLan;

typedef struct{
	char	*name;
	int		index;
}VersionIndexTable;

extern int g_dStopFlag;
extern void IgnoreSignal(int);
extern void handleChildProcess(int);
extern void UserControlledSignal(int);
extern void FinishProgram(void);

// main.c
extern void handleChildProcess (int);
#ifdef __HPLOG__
extern void fan_info (void);
extern void power_info (void);
#endif

#if defined(__HPLOG__) || defined(__LINUX__)
extern void cpu_sts_info(void);
#endif

extern void link_info(void);

// init.c
extern int InitSys (void);
extern int samd_initLog (void);
extern int samd_initProcessTbl (char *fname);
extern int samd_initPingAddrTbl (char *fname);
extern int samd_opticalLanInfo (char *fname);
extern int init_disk_info ();
extern int UnBlockSignal(int SIGNAL);
extern void samd_Duplication_init (void);
extern void samd_SuccessRate_init (void);
extern void samd_DBConnSts_init (void);

// ping.c
extern pthread_t ping_test (void);

// link.c
#ifdef __LINUX__
extern int link_test(); // by helca
#endif

// proc.c
extern long getSumOfPIDs (void);
extern int getProcessStatus(void);
extern int getDaemonStatus(void);
extern void checkProcessStatus(void);
extern void checkSdmdForPrcSts(void); // add by helca 2008.11.21
extern void sdmdForPrcSts_init(void); // add by helca 2008.12.01
extern void checkKeepAlive (void);
extern int runProcess (int procIdx, int isAuto);
extern int getProcessID (char *procName);
extern void doDisPrcSts(IxpcQMsgType *rxIxpcMsg );
extern void doKillPrc(IxpcQMsgType *rxIxpcMsg );
extern void doRunPrc(IxpcQMsgType *rxIxpcMsg );
extern void doDisLoadSts(IxpcQMsgType *rxIxpcMsg );
extern void doDisLanSts (IxpcQMsgType *rxIxpcMsg);
extern void doDisNTPSts (IxpcQMsgType *rxIxpcMsg);
extern void doDisDupSts (IxpcQMsgType *rxIxpcMsg);
extern void doDisSysSts (IxpcQMsgType *rxIxpcMsg);
extern void doDisSysSts2 (IxpcQMsgType *rxIxpcMsg);
extern void doDisAaaSts (IxpcQMsgType *rxIxpcMsg); // by helca 10.23
extern void doDisANAaaSts (IxpcQMsgType *rxIxpcMsg); // by helca 01.08
extern void doDisDuiaType (IxpcQMsgType *rxIxpcMsg); // by helca 2007.05.17
extern void doSetDuiaType (IxpcQMsgType *rxIxpcMsg); // by helca 2007.05.17
extern void doClrAlm_SysMsg (IxpcQMsgType *rxIxpcMsg); // by june 20100312
// qclear.c
extern void QClear (void);

// rxtxmsg.c
extern int HandleRxMsg (void);
extern int report_sadb2FIMD(void);
extern int MMCReqBypassSnd (IxpcQMsgType *rxIxpcMsg);
extern int MMCReqMCDMBypassSnd (IxpcQMsgType *rxIxpcMsg);
extern int MMCResSnd (IxpcQMsgType *rxIxpcMsg, char *resBuf, char resCode, char contFlag);
extern int sendWatchdogMsg2COND (int procIdx);
extern int sendQueClearMsg2COND (int procIdx, int qcnt); // add by helca 2007.07.18
// stat.c
extern int HandleStatistics(GeneralQMsgType *rxGenQMsg);

// sys.c
extern void get_cpu_relate_information (int count, int *cpu_info, long *new, long *old, long *diffs);
extern int get_cpu_count(void);
extern void get_system_information (int flag);
extern void Error(char *error_name);

// disk.c
extern int get_diskUsage (void);
extern int mount_disk_check (char *mnt_mountp);
extern int get_queUsage (void);
#if 0
extern int check_disk_status();
#endif
//link.c
extern void set_optical_lan_status(char side, char *value);

// ntp.c
extern int CheckNTPStS();
extern int CompareNTPSTS(st_NTP_STS stCurr, pst_NTP_STS pstOld);



extern void doDisSysVer(IxpcQMsgType *rxIxpcMsg);
// util.c
#if 0
char *get_ver_str(char *procname);
#endif
void sync_files_to_remote(void);


extern int get_hwinfo_index (char *hwname);
extern void set_hwinfo_sts (int i, char status);
extern int pingtest(char* addr,int npacket,int tsec,int usec);
extern int my_system(const char *cmd);

#endif //  __SAMD_H__
