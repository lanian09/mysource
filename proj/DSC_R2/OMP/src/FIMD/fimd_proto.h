#ifndef __FIMD_PROTO_H__
#define __FIMD_PROTO_H__

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
#include <pthread.h>
#include <sfm_snmp.h> // by helca
#include <mysql/mysql.h>
#include <ctype.h>

#include <commlib.h>
#include <sysconf.h>
#include <comm_msgtypes.h>
#include <comm_almsts_msgcode.h>
#include <omp_filepath.h>
//#include <sfmconf.h>
#include <sfm_msgtypes.h>
#include <stm_msgtypes.h>
#include <proc_version.h>
#include <samd_ntp.h>

#include <sms_msgtypes.h>
#include <mysql_db_tables.h>
#include <nmsif.h>

#include "netOrderChange.h"
#include "netOrderL3pdChange.h"

#define	SQL_CPU_ALM_INFO			"CPU"
#define SQL_PD_CPU_ALM_INFO			"TAP_CPU"
#define	SQL_TAP_PORT_ALM_INFO		"TAP_PORT"
#define SQL_TAP_POWER_ALM_INFO      "TAP_POWER" // 20110424 by dcham
#define SQL_L2SW_CPU_ALM_INFO		"L2SW_CPU"
#define SQL_L2SW_MEM_ALM_INFO		"L2SW_MEM"
#define SQL_L2SW_PORT_ALM_INFO		"L2SW_PORT"

#define SQL_SCE_CPU_ALM_INFO		"SCE_CPU" 
#define SQL_SCE_MEM_ALM_INFO		"SCE_MEM" 
#define SQL_SCE_DISK_ALM_INFO		"SCE_DISK" 
/* hjjung_20100823 */
#define SQL_SCE_USER_ALM_INFO		"SCE_USER" 
#define SQL_SCE_PWR_ALM_INFO		"SCE_PWR" 
#define SQL_SCE_FAN_ALM_INFO		"SCE_FAN" 
#define SQL_SCE_TEMP_ALM_INFO		"SCE_TEMP" 
#define SQL_SCE_VOLT_ALM_INFO		"SCE_VOLT" 
//#define SQL_SCE_PORT_ALM_INFO		"SCE_PORT"    // 따로 지정...
#define	SQL_SCE_RDR_CONN_ALM_INFO	"SCE_RDR_CONN"
#define	SQL_SCE_STATUS_ALM_INFO		"SCE_STATUS"

/* hjjung_20100823 */
#define SQL_LEG_SESSION_ALM_INFO	"LEG_SESSION" 

#define	SQL_SPC_ALM_INFO			"SPC"
#define	SQL_REPL_ALM_INFO			"DB_REP_STATUS"
#define	SQL_REPLGAP_ALM_INFO		"DB_REP_GAP"

#define SQL_TPS_ALM_INFO	"TPS_LOAD" // added by dcham 2011.05.25

//20100917 by dcham
#define SQL_SCM_ALM_INFO            "SCM_STATUS"

/* added by uamyd 20110209. LOGON 성공율 감시를 위해 추가됨 */
#define SQL_LOGON_SUCCESS_RATE_ALM_INFO  "LOGON_SUCCESS_RATE"
#define SQL_LOGOUT_SUCCESS_RATE_ALM_INFO "LOGOUT_SUCCESS_RATE"
#define FILE_LOGON_LIMIT	"DATA/limit_logon_success_rate_file"
#define FILE_SMCONN_LIMIT	"DATA/limit_sm_conn_file"
#define FILE_DSC_LIMIT		"DATA/limit_dsc_file"
#define FILE_TAP_LIMIT		"DATA/limit_tap_file"
#define FILE_SCE_LIMIT		"DATA/limit_sce_file"
#define FILE_SESS_LIMIT		"DATA/limit_sess_file"
#define FILE_TPS_LIMIT		"DATA/limit_tps_file" // added by dcham 20110525
#define FILE_L2SW_LIMIT		"DATA/limit_l2sw_file"
#define ENABLE_REQ   0x01
#define DISABLE_REQ  0x02
#define ACTIVE_DEV   0x10
#define DEACTIVE_DEV 0x20

#define L3PD_RJ45_STS_PHYSIUP		0
#define L3PD_RJ45_STS_PHYSIDOWN		1
#define L3PD_RJ45_STS_PROTODOWN		2

#define	MAPTYPE_NOTIFICATION_ALARM	0
#define	MAPTYPE_AUDIO_ALARM			1
#define	MAPTYPE_ACTIVE_ALARM		2
#define	MAPTYPE_ALTIBASE_ALARM		3
#define	MAPTYPE_SMS_ALARM			4
#define	MAPTYPE_AUDIO_ACT			5
#define	MAPTYPE_AUDIO_DACT			6
#define MAPTYPE_DUPLICATION_STS		7
#define MAPTYPE_SUCCRATE_STS		8
#define MAPTYPE_NO_TRANSMITTED_ACT	9
#define MAPTYPE_NO_TRANSMITTED_DACT	10
#define MAPTYPE_DUAL_STATUS_ACT		11
#define MAPTYPE_DUAL_STATUS_STD		12

#define PROC_NAME_DB 		"ALTIBASE"
#define PROC_NAME_SAMD		"SAMD"

#define MAX_DB_REPL_FAIL		28	/* *4*29)115~125 sec --> replication 절체되는 시간 보장 */
#define MAX_DB_DEAD_CNT			10	/* 10sec */

#define ALTIBASE_BOOTLOG		"/data/altibase/altibase_home/trc/altibase_boot.log"


#define GUI_SYS_INFO_TYPE           1
#define GUI_MSG_INFO_TYPE          	2
#define GUI_INBHA_TCP_CONN_INFO_TYPE   	3
#define GUI_INBHB_TCP_CONN_INFO_TYPE   	4
#define GUI_INBHA_MSG_INFO_TYPE     	5
#define GUI_INBHB_MSG_INFO_TYPE     	6

#define MP_SYSTEM_TYPE      "MP"
#define MP_SYSTEM_GROUP     "DSC"

#define SYS_STATUS_ACTIVE  1
#define SYS_STATUS_STANDBY 2

#define DUPL_BOOT           0
#define DUPL_ACTIVE         1
#define DUPL_STANDBY        2
#define DUPL_UNKNOWN        3

/*MP TYPE (H/W,S/W,ETC)*/
#define LOC_HW_TYPE			0
#define LOC_SW_TYPE			1
#define LOC_HW_MIRROR_TYPE	2

/* LAN INTERFACE TYPE DEFINE */
#define MAX_LAN_IF_TYPE_NUM				4
#define MAX_LAN_IF_TYPE_NAME_LEN		15	
#define MAX_LAN_IF_TYPE_NAME_SIZE		MAX_LAN_IF_TYPE_NAME_LEN + 1	

#define UNIT_TPS_VALUE_5    5 /* used for TPS alarm calculate */


//#define ALM_LIMIT_FILE "limit_bsd_file"

#pragma pack(1)
typedef struct {
    int             ipAddr;     /* 0인 경우 값이 없는 것으로 간주 */
    short           port;
    char            grpNum;
	char			type;
    unsigned char   maxConn;
    unsigned char   curConn;    /* 0이면 무조건 장애(critical)로 간주 */
    char            mask;
    char            level;
} GUI_WinConnInfo;
#pragma pack()

#pragma pack(1)
typedef struct {
	short			cnt;
	short			almCnt;
    char            level;        // 장애 등급
    GUI_WinConnInfo   srv_conn[MAX_CONN_IP];
} GUI_TcpConnInfo;
#pragma pack()


//
// GUI client의 접속 정보를 저장하기 위한 Table
//
typedef struct {
	int		sockFd;      // client의 socket fd
	int		bindPortNum; // 어떤 binding port로 접속한 놈인지 구분하기 위해
#define FIMD_CLIENT_STATE_CONNECTED		0  // 접속만되고 initial이 되지 않은 상태
#define FIMD_CLIENT_STATE_INITIAL		1  // initial 중인 상태
#define FIMD_CLIENT_STATE_ACTIVE		2  // initial 완료되어 정상적으로 정보를 수신할 수 있는 상태
	int		state;       // 최초접속상태, initial상태, active상태
	time_t	connectTime; // 접속된 시간
	char	cliAddr[32]; // client의 ip_address
} FimdClientContext;

// GUI client가 FIMD로 보내는 메시지 string
#define FIMD_CLIENT_MSG_CONFIG_REQUEST		"CONFIG_REQUEST"
#define FIMD_CLIENT_MSG_INIT_COMPLETE		"INIT_COMPLETE"

// alarm_history DB에 들어 있는 오래된 내용을 삭제하는데,
//	얼마나 오래된 내용을 지울것인지...
#define FIMD_DELETE_PERIOD_OLD_ALARM_HISTORY_DB		3600*24*90	// 90일 (초단위)


//
// CPU, Memory, Disk 등 등급 상승,하강 기능이 있는 장애 감지를 위한 structure
//
typedef struct {
	time_t			minStartTime; // minor limit를 초과하기 시작한 시각
	time_t			majStartTime; // major limit를 초과하기 시작한 시각
	time_t			criStartTime; // critical limit를 초과하기 시작한 시각
} FimdHwAlmCheckBuff;


// MMC 명령어 리스트와 처리 function을 등록하는 table
//
#define FIMD_MAX_MMC_HANDLER	30
typedef struct {
	char	cmdName[MML_MAX_CMD_NAME_LEN];
	int		(*func)(IxpcQMsgType*);
} FimdMmcHdlrVector;

/* keepalive */

#define MAX_KEEPALIVE           	15
#define	KEEPALIVE_CHECK_CNT			10
#define	KEEPALIVE_OVER_CNT			 5
/* 20040921 - by mnpark */
#define INIT_KEEPALIVE_RETRY		0

typedef struct {
	int		sysIdx;		/* MPA, MPB, OMP */
	int		category;	/* COMM, CONN, HW, LINK, SPSS */
	int		cnt;
	int		oldcnt;
	int		retry;
} FimdKeepAlive;

// SMS 프로세스 감시
typedef struct {
	int 			idx;
	char			name[32];
	unsigned char	status;
} FimdProcMon;

typedef struct {
	int			appCnt;
	FimdProcMon	procInfo[SFM_MAX_PROC_CNT];
} FimdProcMonSMS;

typedef struct {
	SFM_SysDupSts *dup;
	char		  sysname[COMM_MAX_NAME_LEN];
	int			  sysindex;
	int			  sysstatus;
	int			  sdmdstatus;
} stDupleCheck;

#if 0
/* by june
 * MP RLEG <--> OMP FIMD interface structure
 * Report CPS of LEG */
typedef struct _st_leg_sum_cps_ {
	unsigned int uiLogOnSumCps;
	unsigned int uiLogOutSumCps;
} LEG_SUM_CPS;
#endif

typedef struct _lan_if_info {
	int	name_size;
	char name[MAX_LAN_IF_TYPE_NAME_SIZE];
} LANIF_INFO, *PLANIF_INFO;

typedef struct _lan_if_config {
	int count;
	LANIF_INFO lanif[MAX_LAN_IF_TYPE_NUM];
} LANIF_CONFIG, *PLANIF_CONFIG;

extern int errno;
extern int g_dStopFlag;

extern void FinishProgram(void);
extern void UserControlledSignal(int);
extern void IgnoreSignal(int);
extern void SetUpSignal();

extern int fimd_checkSMChStsAlm(int sysIndex, SFM_SMChInfo *smChInfo, int smChID);
extern int fimd_exeRxQMsg (GeneralQMsgType*);
extern int fimd_exeStsRptMsg (IxpcQMsgType*);
extern int fimd_exeAlmRptMsg (IxpcQMsgType*);
extern int fimd_exeMMCMsg (IxpcQMsgType*);
extern int fimd_exeStatRptMsg (IxpcQMsgType*);
extern int fimd_exeNewConn (int);
extern int fimd_exeDisconn (int);
extern int fimd_exeRxSockMsg (int, SockLibMsgType*);

extern int fimd_initial (void);
extern int fimd_initLog (void);
extern int fimd_getSfdb (int);
extern int fimd_getL3pd (int); // by helca 09.11
extern int fimd_getLogonSuccessRate(void); // added by uamyd 20110210
extern int fimd_getSMChSts(void); // added by uamyd 20110210
extern int fimd_setDefaultSfdb (void);
extern void fimd_backupSfdb2File (void);
extern void fimd_backupSMChSts2File (void);
extern void fimd_backupL3pd2File (void); // by helca 09.11

extern int alm_lmt_dsc_input (void); // by helca 08.17
extern int alm_lmt_pd_input (void); // by helca 08.17 
extern int alm_lmt_sce_input(void);  // by june 09.05
/* hjjung */
extern int alm_lmt_leg_input(void); 
extern int alm_lmt_tps_input(void);  // added by dcham 20110525
extern int alm_lmt_l2sw_input(void); // by sjjeon 09.06

//extern int alm_lmt_bsd_output(void); // by helca 08.17
extern int alm_lmt_dsc_output(void); // by helca 08.17
extern int alm_lmt_pd_output(void); // by helca 08.17
extern int alm_lmt_sce_readInfoFile(void);  // by june 09.05
/* hjjung_20100823 */
extern int alm_lmt_tps_readInfoFile(void); // added by dcham 20110530
extern int alm_lmt_l2sw_readInfoFile(void); // by sjjeon 09.06

extern int alm_lmt_logon_success_rate_readInfoFile(void);
extern int alm_lmt_sm_ch_sts_readInfoFile(void);

extern int fimd_hwInfo_init (void); // by helca 2008.07.21

extern void fimd_transTotalUpDownFrameRate (int); // 08.28

extern int fimd_hdlSysCommStsRpt (IxpcQMsgType*);
extern int fimd_hdll3pdStsRpt (IxpcQMsgType *);
extern int fimd_hdleSCEStsRpt (IxpcQMsgType *);
/* hjjung_20100823 */
extern int fimd_hdleLEGStsRpt (IxpcQMsgType *);
extern int fimd_hdleTPSStsRpt (IxpcQMsgType *); // added by dcham 2011.05.25 for TPS
extern int fimd_hdlSysSpecConnStsRpt (IxpcQMsgType*);
extern int fimd_hdlSysSpecHwStsRpt (IxpcQMsgType*);
extern int fimd_hdleLogonSuccessRate(IxpcQMsgType *); //added uamyd 20110210

extern int fimd_hdlCpuUsageAlm (int, int, int, int, int);
extern int fimd_hdlQueLoadAlm (int, int, int, int, int); // by helca 08.07
extern int fimd_hdlPDCpuUsageAlm (int, int, int,int); // by helca
extern int fimd_hdlMemUsageAlm (int, int, int, int);
extern int fimd_hdlPDMemUsageAlm (int, int, int, int); // by helca
extern int fimd_hdlDiskUsageAlm (int, int, int, int, int);
extern int fimd_hdlLanAlm (int, int);
extern int fimd_hdlPDFanAlm (int, int); // by helca
extern int fimd_hdlGigaLanAlm (int, int, int, int); // by helca
extern int fimd_hdlProcAlm (int, int, int);
extern int fimd_hdlHwComAlm (int, SFM_HpUxHWInfo*, int);
extern int fimd_hdlRsrcStsAlm (int, int, int);
extern int fimd_hdlRsrcLoadAlm (int, int, int, int, int, int); // by helca


/* by helca */
extern int fimd_hdlRmtLanAlm (int , int);
extern int fimd_hdlOptLanAlm (int , int);

extern int fimd_rateSts(int, int, int); // by helca 08.24
extern int fimd_hdlDupstsMsg (int);
extern int fimd_hdlDupHeartAlm (int, SFM_SysDuplicationSts *, int);
extern int fimd_hdlDupOosAlm (int, SFM_SysDuplicationSts *);
extern int fimd_hdlDupDualSts (int, int, int); // add by helca 2009.01.09
extern int fimd_hdlDupDualStsAlmMsg (int, int, int); // add by helca 2009.01.09
extern int fimd_hdlDupTimeOutAlm (int, int); // add by helca 2009.01.13
extern int fimd_makeDupTimeOutAlmMsg (int, int); // add by helca 2009.01.13
extern int fimd_hdlSuccAlm (int, int, int, int, int); // by helca 10.20
extern int fimd_hdlSessLoadAlm (int, unsigned short);
extern int fimd_hdlRmtDbStsAlm (int, int, int);
extern int fimd_hdlhwNTPAlm(int, int, int);
extern int fimd_hdlNmsifStsAlm(int, int, int, int); // by helca 11.7
//extern int fimd_hdlSCMFaultStsAlm(int, int, int); // by dcham 10.09.24
extern int fimd_hdlSCMFaultStsAlm(int, SFM_SysDupSts *, int); // by dcham 10.09.24

extern int fimd_hdlLogonSuccessRateAlm (int sysIndex, int log_mod, int almLevel, int prevAlmLevel, int occurFlag); //added by uamyd 20110210
extern int fimd_mmc_checkLogonSuccessRateAlmLimitValidation(int, int, int, int);
extern int fimd_mmc_updateLogonSuccessRateAlmClsValue(int, int, int, int);
extern void fimd_mmc_makeLogonMaskStsOutputMsg(int, int*, int*);
extern void fimd_mmc_makeSMChStsOutputMsg(int, int*, int*);
extern int fimd_mmc_checkSMChAlmLimitValidation( int sysIndex, int level, int limit );
extern int fimd_mmc_updateSMChAlmClsValue (int sysIndex, int level, int limit );

extern int fimd_checkCpuUsageAlm (int, int, SFM_CpuInfo*);
extern int fimd_checkPDCpuUsageAlm (int, SFM_PDCpuInfo *); // by helca
extern int fimd_checkMemUsageAlm (int, SFM_MemInfo*);
extern int fimd_checkPDMemUsageAlm (int, SFM_PDMemInfo *); // by helca
extern int fimd_checkDiskUsageAlm (int, int, SFM_DiskInfo*);
extern int fimd_checkDbRepGapAlm (int sysIndex, SFM_DBSyncInfo *dbRepGap);
extern int fimd_checkQueueLoadAlm (int, int, SFM_QueInfo*); // by helca 08.07
extern int fimd_checkLogonSuccessRateAlm(int, int, int, int); // added by uamyd 20110210
extern int fimd_radiusAlm(int, int, int, int );


extern void fimd_updateSysAlmInfo (int);
extern void fimd_updatePDAlmInfo (int); // by helca
extern void fimd_checkStatReportTime();

extern int fimd_makeSysAlmLevelChgMsg (int);
extern int fimd_makeCpuUsageAlmMsg (int, int, int, int);
extern int fimd_makePDCpuUsageAlmMsg (int, int, int); // by helca
extern int fimd_makeMemUsageAlmMsg (int, int, int);
extern int fimd_makePDMemUsageAlmMsg (int, int, int); // by helca
extern int fimd_makeDiskUsageAlmMsg (int, int, int, int);
extern int fimd_makeQueLoadAlmMsg (int, int, int, int); // by helca 08.07
extern int fimd_makeLanAlmMsg (int, int, int*, int*);
extern int fimd_makeProcAlmMsg (int, int, int*, int*, int);
extern int fimd_makeHwComAlmMsg(int, SFM_HpUxHWInfo*, int, int*, int*, int*);
extern int fimd_makeRsrcLoadAlmMsg (int, int, int, int, int); // by helca
/* by helca */
extern int fimd_makeRmtLanAlmMsg (int, int, int *, int *);
extern int fimd_makePDFanAlmMsg (int, int, int *, int *);
extern int fimd_saveRmtLanAlmInfo2DB (int, int , int, int);
extern int fimd_makeOptLanAlmMsg (int, int, int *, int *);
extern int fimd_makeGigaLanAlmMsg (int, int, int ); // by helca
extern int fimd_saveOptLanAlmInfo2DB (int, int, int, int);
extern int fimd_saveGigaLanAlmInfo2DB (int, int, int, int); // by helca
//extern int fimd_makeDupStsAlmMsg (int, int *, int *);
extern int fimd_makeDupStsAlmMsg (int, SFM_SysDuplicationSts *, int *, int *);
extern int fimd_makeDupHeartAlmMsg (int, SFM_SysDuplicationSts *, int );
extern int fimd_makeDupOosAlmMsg (int, SFM_SysDuplicationSts *, int *, int *);
extern int fimd_makeSuccRateAlmMsg (int, int, int, int, int, int); // by helca 10.20
extern int fimd_hdlSuccRateIpAlm (int, SuccRateIpInfo , SFM_SysSuccRate *, int );
extern int fimd_makeSuccRateIpAlmMsg (int, SuccRateIpInfo , SFM_SysSuccRate *, int, int);
extern int fimd_makeRadiusSuccMsg (int , int , int , int, int );


extern int fimd_makeSessLoadAlmMsg (int, unsigned short, int *, int *);
extern int fimd_makeRmtDbStsAlmMsg (int, int, int );
extern int fimd_getRmtLanIndexByName (int, char *);
extern int fimd_getOptLanIndexByName (int, char *);
extern int fimd_getSuccRateIndexByName(int, char *);
extern int fimd_getRsrcIndexByName (int, char *);
extern int fimd_getRmtDbLanIndexByName (int, char *);
extern int fimd_gethwNtpIndexByName (int, char *);
extern int fimd_makehwNTPAlmMsg (int, int, int);
extern int fimd_makeNmsifstsAlmMsg(int, int, int, int); // by helca 11.7
extern int fimd_makeSCMFaultStsAlmMsg(int, int, int); // by dcham 10.09.24 
//---------------------------------------------------------//
extern int fimd_getPDSNIPIndexByName (int, char *);


extern int fimd_getSysIndexByName (char *);
extern int fimd_getGroupIndexByName (char *);
extern int fimd_getDiskIndexByName (int, char *);
extern int fimd_getLanIndexByName (int, char *);




extern int fimd_getProcIndexByName (int, char *);
extern int fimd_getHwIndexByName (int, char *);
extern int fimd_getCliIndex (int);
extern int fimd_txMsg2Cond (char*, long, int);
extern int fimd_txMsg2Nmsif (char *, int, int, char *, int); // by helca 08.17
extern int fimd_txMMLResult (IxpcQMsgType*, char*, char, char, unsigned short, char, char);
extern int fimd_broadcastSfdb2Client (int);
extern int fimd_sendSfdb2Client (int, int);
extern int fimd_broadcastAlmEvent2Client (void);
extern int fimd_broadcastDualAlmEvent2Client (int);
extern int fimd_sendAlmEvent2Client (int);
extern int fimd_sendDualAlmEvent2Client (int, int);
extern int fimd_rxConfigRequest (int);
extern void *fimd_rxConfReqThread (void*);
extern int fimd_rxInitComplete (int);

extern void fimd_initSysAlmStat (void);
extern void fimd_increaseAlmStat (int, int, int);
extern void fimd_increaseAlmStatIndex (int, int, int, int); // by helca
extern void fimd_reportSysStatData2OMP (IxpcQMsgType*);
extern void init_l3pd_status_shm(void);

extern int fimd_mysql_init(void);
extern int fimd_mysql_query (char*);
extern int fimd_saveCpuUsageAlmInfo2DB (int, int, int, int);
extern int fimd_savePDCpuUsageAlmInfo2DB (int, int, int); // by helca
extern int fimd_saveMemUsageAlmInfo2DB (int, int, int);
extern int fimd_savePDMemUsageAlmInfo2DB (int, int, int); // by helca
extern int fimd_savePDFanAlmInfo2DB (int, int, int, int); // by helca
extern int fimd_saveRsrcLoadAlmInfo2DB (int, int, int, int); // by helca
extern int fimd_saveDiskUsageAlmInfo2DB (int, int, int, int);
extern int fimd_saveQueLoadAlmInfo2DB (int, int, int, int); // by helca 08.07
extern int fimd_saveLanAlmInfo2DB (int, int, int, int);
extern int fimd_saveProcAlmInfo2DB (int, int, int, int);
extern int fimd_saveHwComAlmInfo2DB (int, SFM_HpUxHWInfo*, int, int, int);
extern int fimd_deleteOldAlmInfoDB (void);

/* by helca */
extern int fimd_saveDupStsAlmInfo2DB (int, int, int);
extern int fimd_saveDupHeartAlmInfo2DB (int, int, int);
extern int fimd_saveDupOosAlmInfo2DB (int, int, int);
extern int fimd_saveSuccRateAlmInfo2DB (int, int, int, int, int, int);
extern int fimd_saveSessLoadAlmInfo2DB (int, unsigned short, int, int);
extern int fimd_saveRmtDbStsAlmInfo2DB (int, int, int, int);
extern int fimd_saveNmsifstsAlmInfo2DB (int, int, int, int); // by helca 11.7
extern int fimd_saveSuccRateIpAlmInfo2DB (int ,SuccRateIpInfo, SFM_SysSuccRate * ,char *, int , int );
extern int fimd_saveRadiusSuccAlmInfo2DB (int, int, int, int, int);
extern int fimd_saveDualActStsAlmInfo2DB (int, int, int, int);
extern int fimd_saveDupTimeOutAlmInfo2DB (int, int, int);
extern int fimd_saveSCMFaultStsAlmInfo2DB (int, int, int); // by dcham 10.09.24

extern int fimd_queryRadius(int);
extern int evaluate_successRate(int , SFM_SysSuccRate *, SuccRateIpInfo *, SuccRateIpInfo *, int);

extern int fimd_mmc_stop_aud_alm (IxpcQMsgType*);
extern int fimd_mmc_mask_alm (IxpcQMsgType*);
extern int fimd_mmc_umask_alm (IxpcQMsgType*);
extern int fimd_mmc_dis_mask_sts (IxpcQMsgType*);
extern int fimd_mmc_dis_alm_lmt (IxpcQMsgType*);
/* hjjung */
extern int fimd_mmc_dis_session_lmt (IxpcQMsgType*);
extern int fimd_mmc_dis_tps_lmt (IxpcQMsgType*); /// added by dcham 20110525
extern int fimd_mmc_dis_svc_alm (IxpcQMsgType*); // by helca
extern void fimd_mmc_makeAlmClsOutputMsg (IxpcQMsgType*, int, char*, int);
extern void fimd_mmc_makeAlmRateOutputMsg (IxpcQMsgType *, int, char *);
extern void fimd_mmc_makeAlmRateAllOutputMsg (IxpcQMsgType *, int, char *); // by helca 08.17
extern void fimd_mmc_makeSysMaskStsOutputMsg (IxpcQMsgType*, int, int*, char*);
extern void fimd_mmc_makeSysMaskStsOutputAllMsg (IxpcQMsgType*, int, int*, char*); // by helca 08.15
extern void fimd_mmc_makePDSysMaskStsOutputMsg (IxpcQMsgType*, int, int*, char*); // by helca 08.10
extern void fimd_mmc_makeSCESysMaskStsOutputMsg (IxpcQMsgType*, int, int*, char*); // by sjjeon  2009-05-18
/* hjjung */
extern void fimd_mmc_makeLEGSysMaskStsOutputMsg (IxpcQMsgType*, int, int*, char*); 
extern void fimd_mmc_makePDSysMaskStsOutputAllMsg (IxpcQMsgType*, int, int*, char*); // by helca 08.15
extern void fimd_mmc_makeSCESysMaskStsOutputAllMsg (IxpcQMsgType*, int, int*, char*); // by sjjeon 2009-05-18
/* hjjung */
extern void fimd_mmc_makeLEGSysMaskStsOutputAllMsg (IxpcQMsgType*, int, int*, char*); 
extern int fimd_mmc_set_alm_lmt (IxpcQMsgType*);
/* hjjung */
extern int fimd_mmc_set_session_lmt (IxpcQMsgType*);
extern int fimd_mmc_set_tps_lmt (IxpcQMsgType*); // added by dcham 20110525
extern int fimd_mmc_set_svc_alm (IxpcQMsgType*); // by helca
extern int fimd_mmc_checkAlmLimitValidation (int, int, int, void*);
extern int fimd_mmc_updateAlmClsValue (int, int, int, int, void*);
extern int fimd_mmc_updateAlmRateValue (int, int, int, int, void *); // by helca
extern int fimd_mmc_dis_alm_sts (IxpcQMsgType*);
extern int fimd_mmc_makeSysAlmStsOutputMsg (IxpcQMsgType*, int, int*, char*, int);
extern int fimd_mmc_makeSysAlmPDStsOutputMsg (IxpcQMsgType*, int, int*, char*, int); // by helca 08.21

extern int fimd_mmc_dis_ppd (IxpcQMsgType*);
extern int fimd_mmc_act_ppd (IxpcQMsgType*);
extern int fimd_mmc_dact_ppd (IxpcQMsgType*);
extern int fimd_mmc_dis_node_info (IxpcQMsgType*);
extern int fimd_mmc_audit_alm (IxpcQMsgType*);

extern int fimd_mmcHdlrVector_qsortCmp (const void *a, const void *b);
extern int fimd_mmcHdlrVector_bsrchCmp (const void *a, const void *b);
extern char *fimd_printAlmLevel (unsigned char);
extern char *fimd_printAlmLevelSymbol (int, int);
extern char *fimd_printProcStatus (unsigned char);
extern char *fimd_printLanStatus (unsigned char);
extern void fimd_dumpSysAlmStat (STM_SysFltStat*, int);
extern char	*fimd_printHWStatus(unsigned char);

extern void fimd_keepAliveIncrease(int, char *);
extern void fimd_checkKeepAlive(void);
extern void fimd_checkSysAlive(void);
extern int rmtProcStatusCheck(int); // 2007.01.12 by helca
extern int rshellResult(int sysIndex, char *checkip);

extern void linktest(int); // 2007.01.12 by helca
extern void review_linktest(int);

/*20040921-mnpark*/
extern int fimd_checkDBRepl(void);
extern int fimd_getOc7StandbySysIndex();
extern int fimd_doKillprc(int, int , char *);
extern int fimd_checkMPProc(int, int);
extern int fimd_getKeepAliveStsRty(int);
extern int fimd_getMPProcAllAlive(char *);
/* 20041129.mnpark */
extern int fimd_checkAltibaseStatus(void);	
extern int getProcStatus(int , char *);


/* fimd_dupl.c */
void discret_dupl_status();
void normal_active(stDupleCheck dup[SYSCONF_MAX_ASSO_SYS_NUM-1]);
void asynch_active(stDupleCheck dup[SYSCONF_MAX_ASSO_SYS_NUM-1]);
void limited_active(stDupleCheck knowdup, stDupleCheck unknowdup);

/* fimd_rxtxmsg.c */
int fimd_exeDupStatusRpt(IxpcQMsgType *rxIxpcMsg);

/* fimd_tool.c */
void strtoupper(char *s1, int slen);
void hw_name_mapping(char *s1, int slen, char *s2);


extern int fimd_SuccessRate_AAA_UAWAP(int sysIndex);

/* by june */
extern void fimd_backupSCE2File (void); /* by june '09.04.15 */
/* hjjung */
extern void fimd_backupCALLFile (void);   /* added by dcham 20110530 for TPS */
//extern void fimd_backupTPSFile (void);
extern int fimd_getSCE (int); // by june '09.4.15
/* hjjung_20100823 */
extern int fimd_getCALL (int);
extern int fimd_getTPS (int); // added by dcham 20110525

extern void  fimd_backupLogon2File(); // added by uamyd 20110210
extern void init_sce_status_shm(void);
/* hjjung */
extern void init_leg_status_shm(void);
extern int alm_lmt_sce_input (void); /* by june '09.04.15 */
/* hjjung */
//extern int alm_lmt_leg_input (void); /* by june '09.04.15 */
//extern int alm_lmt_sce_output(void); // by june '09.4.15
extern int alm_lmt_sce_readInfoFile(void); // rename alm_lmt_sce_output -> alm_lmt_sce_readInfoFile : sjjeon
/* hjjung */
//extern int alm_lmt_leg_readInfoFile(void); 
extern int alm_lmt_sess_readInfoFile(void);  // added by dcham 20110525
extern int fimd_hdlSceCpuUsageAlm(int, int, int, int,int); 
//extern int fimd_getSceAlarmCode (int dev_kind);
extern int fimd_getAlarmCode (int dev_kind); /* by sjjeon */
extern char *fimd_getSceDevName (int dev_kind);
/* hjjung */
extern char *fimd_getLegDevName (int dev_kind);
extern int fimd_getSceAlarmType (int dev_kind);
extern int fimd_makeSceUsageAlmMsg(SCE_USAGE_PARAM *param);
/* hjjung */
extern int fimd_makeLegUsageAlmMsg(LEG_USAGE_PARAM *param);
extern int fimd_makeLegTpsAlmMsg(TPS_PARAM *param); // added by dcham 20110525 for TPS
extern int fimd_saveSceUsageAlmInfo2DB (SCE_USAGE_PARAM *param);
/* hjjung */
extern int fimd_saveLegUsageAlmInfo2DB (LEG_USAGE_PARAM *param);
extern int fimd_saveLegTpsAlmInfo2DB (TPS_PARAM *param); // added by dcham 20110525 for TPS
extern int fimd_thresholdCheckUsage (Threshold_st *pthresh, SCE_USAGE_PARAM *pParam);

/* LOGON 성공율 감시. added by uamyd 20110210 */
extern int fimd_makeLogonSuccessRateAlmMsg ( int sysIndex, int log_mod, int almLevel, int occurFlag);
extern int fimd_saveLogonSuccessRateAlmInfo2DB ( int sysIndex, int log_mod, int almLevel, int occurFlag);

extern int fimd_makeSMChStsAlmMsg(int sysIndex, int almLevel, int occurFlag, int smChID);
extern int fimd_saveSMChStsAlmInfo2DB (int sysIndex, int almLevel, int occurFlag, int smChID);
extern int fimd_hdlSMChStsAlm (int sysIndex, int smChID, SFM_SMChInfo *smChInfo, int almLevel, int occurFlag);

/* hjjung */
extern int fimd_thresholdCheckSceUserUsage (Threshold_st_int *pthresh, SCE_USAGE_PARAM *pParam);
extern int fimd_thresholdCheckLegUsage (Threshold_st_int *pthresh, LEG_USAGE_PARAM *pParam);
extern int fimd_thresholdCheckLegTps (Threshold_st_tps *pthresh, TPS_PARAM *pParam); // added by dcham 20110525 for TPS
//extern int fimd_thresholdCheckStatus(SCE_USAGE_PARAM*param);
extern int fimd_thresholdCommCheckStatus(COMM_STATUS_PARAM *param);
extern int fimd_checkSceCpuUsageAlm(int,int);
extern int fimd_checkSceRdrConnStatusAlm (int sysIndex, int rdrConnIndex);
extern int fimd_checkSceMemUsageAlm (int sysIndex, int memIndex);
extern int fimd_checkSceDiskUsageAlm (int sysIndex, int diskIndex);
/* hjjung */
extern int fimd_checkSceUserUsageAlm (int sysIndex, int userIndex);
extern int fimd_checkScePowerStatusAlm (int sysIndex, int pwrIndex);
extern int fimd_checkSceFanStatusAlm (int sysIndex, int fanIndex);
extern int fimd_checkSceTempStatusAlm (int sysIndex, int tempIndex);
extern int fimd_checkScePortStatusAlm (int sysIndex, int portIndex);
extern int fimd_checkSceRdrConnStatusAlm (int sysIndex, int rdrConnIndex);

extern int fimd_checkLegSessionUsageAlm (int sysIndex);
extern int fimd_checkTpsAlm (int sysIndex); // added by dcham 20110525 for TPS


/* by sjjeon */
extern void fimd_mmc_makeSceAlmClsOutputMsg(IxpcQMsgType*, int sysIndex, char *seqNo, int all);
/* hjjung */
extern void fimd_mmc_makeSceSessionAlmClsOutputMsg(IxpcQMsgType*, int sysIndex, char *seqNo, int all);
extern int fimd_mmc_updateSceAlmClsValue (int type, int level, int limit, int durat, int sysIndex);
extern void fimd_updateSceAlmInfo ();
extern int fimd_mmc_sce_set_alm_lmt(IxpcQMsgType *rxIxpcMsg, char argSys[32], char argType[32],
			char argLevel[32], int limit, int durat, int sysIndex );

/* hjjung */
extern void fimd_mmc_makeLegAlmClsOutputMsg(IxpcQMsgType*, int sysIndex, char *seqNo, int all);
extern void fimd_mmc_makeCallAlmClsOutputMsg(IxpcQMsgType*, int sysIndex, char *seqNo, int all); // added by dcham 20110525
extern int fimd_mmc_updateLegAlmClsValue (int type, int level, int limit, int durat, int sysIndex);
extern void fimd_updateLegAlmInfo ();
extern void fimd_updateCallAlmInfo (); // added by dcham 2011.05.25
extern int fimd_mmc_leg_set_alm_lmt(IxpcQMsgType *rxIxpcMsg, char argSys[32], char argType[32],
			char argLevel[32], int limit, int durat, int sysIndex );

extern int fimd_mmc_call_set_alm_lmt(IxpcQMsgType *rxIxpcMsg, char argSys[32], char argType[32],
			char argLevel[32], int limit, int durat, int sysIndex ); // added by dcham 20110525

extern int fimd_hdlDupstsMsg_yh (int sysIndex, int srcStatus, int destStatus);
extern int fimd_hdlL2LanAlm (int devIndex, int gigaIndex, int almLevel, int occurFlag);

extern int fimd_AudioAlmEvent2Client (void);
extern int fimd_getNmsifIndexByName (int sysIndex, char *IndexName);
extern void fimd_mmc_makePdAlmClsOutputMsg ( IxpcQMsgType *rxIxpcMsg, int sysIndex, char *seqNo, int all);
extern int fimd_mmc_pd_set_alm_lmt( IxpcQMsgType *rxIxpcMsg, char argSys[32],char argType[32],char argLevel[32],int limit,int durat, int sysIndex );
extern int	fimd_makeSCELinkalmMsg (SCE_USAGE_PARAM *param, char *dev_name);
/* hjjung */
extern int	fimd_makeLEGLinkalmMsg (LEG_USAGE_PARAM *param, char *dev_name);
extern int fimd_saveSceLinkAlmInfo2DB (SCE_USAGE_PARAM *param, char *dev_type);
/* hjjung */
extern int fimd_saveLegLinkAlmInfo2DB (LEG_USAGE_PARAM *param, char *dev_type);
extern int	fimd_makeL2LanAlmMsg (int devIndex, int gigaIndex, int occurFlag);
extern int fimd_saveL2SWLanAlmInfo2DB (int devIndex, int gigaIndex, int almLevel, int occurFlag);
extern int fimd_savehwNTPAlmInfo2DB (int sysIndex, int hwNTPIndex, int almLevel, int occurFlag);
extern int fimd_makeCallInfoAlmMsg(int sysIndex, int almLevel, int occurFlag);
extern int fimd_saveCallInfoAlmInfo2DB (int sysIndex, int almLevel, int occurFlag);
extern void fimd_backupAudio2File (void);
extern int fimd_ActAlmEvent2Client (void);
extern int fimd_DactAlmEvent2Client (void);
extern int fimd_getAudio (int key);
extern int fimd_getL2Dev (int key);
extern int fimd_getConnectedLanIdx(int targetSysIndex);
extern int	fimd_makeKillprcAlmMsg (int almReasonType, int sysIndex, int procIdx, int altibaseIdx);
extern int fimd_sendStatEvent2Client (int cliIndex, char *tStamp);

extern int get_devKind_byName(char *devName);
extern int fimd_exeDupUpdateNoti(IxpcQMsgType *rxIxpcMsg);
extern int fimd_hdleL2StsRpt (IxpcQMsgType *rxIxpcMsg); 
extern int fimd_exeAlmRptMsg (IxpcQMsgType *rxIxpcMsg);
extern int fimd_broadcastStatEvent2Client (char *tStamp);
/* hjjung */
extern char *fimd_getLegDevName (int dev_kind);
extern char *fimd_getScePortName(int dev_num);
extern int fimd_getAlarmType (int dev_kind);
extern void fimd_backupL2sw2File (void);
extern int fimd_DuplicationStsEvent2Client (void);
extern int fimd_makeCommUsageAlmMsg (int sysType, int unitType, int unitIndex, int usage, int curLevel, int preLevel, int occurFlag, char *devName);
extern int fimd_saveCommStsAlmInfo2DB(int sysType, int unitType, int unitIndex, int curLevel, int occurFlag, char *devInfo);
extern int fimd_makeCpsOverAlmMsg(int sysIndex, int lanIndex, int *almLevel, int *occurFlag1);
extern int fimd_saveCommStsAlmInfo2DB(int sysType, int unitType, int unitIndex, int curLevel, int occurFlag, char *devInfo);
extern int fimd_makeL2swCpuUsageAlmMsg (int devIndex, int almLevel, int occurFlag);
extern int fimd_saveL2swCpuUsageAlmInfo2DB (int devIndex, int almLevel, int occurFlag);
extern int fimd_makeL2swMemUsageAlmMsg (int devIndex, int almLevel, int occurFlag);
extern int fimd_saveL2swMemUsageAlmInfo2DB (int devIndex, int almLevel, int occurFlag);
//extern int getAlarmCodeFromType(int almType);
extern int getAlarmCodeFromType(int almType, int almLevel);
extern int fimd_saveCpsAlmInfo2DB (int sysIndex, int lanIndex, int almLevel, int occurFlag1);
extern int fimd_hdlL2swCpuUsageAlm (int devIndex, int almLevel, int prevAlmLevel, int occurFlag);
extern int fimd_hdlL2swMemUsageAlm (int devIndex, int almLevel, int prevAlmLevel, int occurFlag);
extern int fimd_getAbnormalAlarmLevel_byDevName(COMM_STATUS_PARAM *pParam);
extern int fimd_getNormalStatValue(int sysIndex, int unitIndex);
extern int fimd_CommUsageAlm (int sysType, int unitType, int unitIndex, int usage, int almLevel, int preAlmLevel, int occurFlag);
extern void fimd_updateSysAlmInfo (int sysIndex);
extern void fimd_mmc_makeL2swAlmClsOutputMsg ( IxpcQMsgType *rxIxpcMsg, int sysIndex, char *seqNo, int all);
extern void fimd_updateL2SWlmInfo (int sysIndex);
extern int fimd_mmc_l2sw_set_alm_lmt( IxpcQMsgType *rxIxpcMsg, char argSys[32],char argType[32],
		                                char argLevel[32],int limit,int durat, int sysIndex);
extern int fimd_makeCommStsAlmMsg (int sysType, int unitType, int unitIndex, int curLevel, int preLevel, int occurFlag, char *devName);
extern int fimd_checkCpsOverAlm(int sysIndex, int lanIndex);
extern int fimd_checkCommonStatusAlm(int sysType, int unitType, int unitIndex);
extern int fimd_checkL2swCpuUsageAlm (int devIndex, SFM_PDCpuInfo *cpuInfo);
extern int fimd_checkL2swMemUsageAlm (int devIndex, SFM_PDMemInfo *memInfo);
extern void fimd_check_NMS_status(int sysIndex);
extern int fimd_checkCurrentAlarm ();
extern int pingtest(char* addr,int npacket,int tsec,int usec);
extern int getAlarmTypeFromProcName(int sysIndex, int procIndex);
extern int getNmsLevel(int curLevel, int occurflag);
#endif //__FIMD_PROTO_H__
