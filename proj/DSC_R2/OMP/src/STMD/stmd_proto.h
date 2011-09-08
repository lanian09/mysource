#ifndef __STMD_PROTO_H__
#define __STMD_PROTO_H__

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
#include <ctype.h>

#include <commlib.h>
#include <sysconf.h>
#include <comm_msgtypes.h>
#include <comm_almsts_msgcode.h>
#include <sfmconf.h>
#include <stmconf.h>
#include <stm_msgtypes.h>
#include <omp_filepath.h>
#include <mysql_db_tables.h>
#include <mysql/mysql.h>
#include <proc_version.h>
#include <nmsif.h>

// Log관련 Define
/* 새로 생성하는 directory mode */
#define DIRMODE                                 0777
/* dir 개수 년도, 달, 일, 시 */
#define DIR_NUM                                 4
#define APPEND                                  "a"
#define READ                                    "r"
#define WRITE                                   "w"

#define RULE_SET_FILE		"RULE/RULESET_LIST.conf"
#define SVC_ALM_CTRL_FILE	"DATA/SVC_ALM.conf"
#define PDSN_IP_CTRL_FILE	"DATA/PDSN.conf"
#define MAX_RULE_NUM	5000
#define MAX_ENTRY_NUM	500
#define MAX_SCE_NUM		2
#define MAX_PDSN_NUM	32
#define MAX_SMSC_NUM	32

#define STMD_ADD_MIN    "MINUTE"
#define STMD_ADD_HOUR   "HOUR"
#define STMD_ADD_DAY    "DAY"
#define STMD_ADD_MONTH  "MONTH"

#define KEEPALIVE_DEADLINE	3


/* 07.17 MP pri / second ip 추가  */
char	DSC_IP[16];
char	SCMA_PRI_IP[16];
char	SCMA_SCD_IP[16];
char	SCMB_PRI_IP[16];
char	SCMB_SCD_IP[16];

#define DSC_IP_NUM	5 // VRTS IP(1) + SCMA(2) + SCMB(2)
char	IP_POOL[DSC_IP_NUM][16];

#define ACTIVE			1 // sfdb의 status 값 ACTIVE 
#define STANDBY			2 // sfdb의 status 값 STAND BY 

#define SCMA			1
#define SCMB			2
/* ACTIVE IS WHAT ? 07.17 */
#define ACTIVE_A			1 // 정상 ACTIVE A
#define ACTIVE_B			2 // 정상 ACTIVE B
int	ACT_SYS;	

#define ACT_STANDBY_MODE 	0 // 07.17 정상 ACT / STANDBY 모드 
#define DUAL_STANDBY_MODE 	3 // DUAL STANDBY MODE 
int	SYS_MODE;	 

unsigned int	mysql_timeout;

// SQL문 출력을 보기 위해 setprint명령어시 입력하면 출력된다
#define TRCLEVEL_SQL    6

// 통계 최소 처리 단위는 5분
#define STAT_UNIT           5
#define STAT_MIN_SEC		60
#define STAT_OFFSET_UNIT    300 // 5분단위로 만들기 위한 기준값

// 통계 전체 메시지를 기다리는 시간 5초 -> 25초
#define WAIT_FOR_STAT_MSG       25  

// 통계 보고 메시지의 전체 갯수 짤라서 보내는 것이 있으면 충분히 크게한다.
#define TOTAL_RX_STAT_MSG       60

// 주기적 통계 상태 메시지 번호에서 메시지Id를 구하기 위한 offset
// 주기적 통계 상태 메시지 번호에서 3000을 빼면 메시지 ID가 된다
#define STSCODE_TO_MSGID_STATISTICS 3000

// 주기적 통계 MASK
#define UNMASK      0
#define MASK        1

// 통계 DB를 지우기 위한 offset값
#define STMD_5MIN_OFFSET        (60*60)         // 한 시간  
#define STMD_1HOUR_OFFSET       (60*60*24)      // 하루
#define STMD_1HOUR_WEEK_OFFSET  (60*60*24*7)
#define STMD_1DAY_OFFSET        (60*60*24*30)   // 한달
#define STMD_1WEEK_OFFSET       (60*60*24*30)   // 한달
#define STMD_1MON_OFFSET        (60*60*24*30*12)// 1년

// WEEKLY 통계에서 초기 INSERT를 하기위한 시간 OFFSET
#define MON_WEEK_OFFSET         1*STMD_1HOUR_OFFSET
#define TUE_WEEK_OFFSET         2*STMD_1HOUR_OFFSET
#define WED_WEEK_OFFSET         3*STMD_1HOUR_OFFSET
#define THU_WEEK_OFFSET         4*STMD_1HOUR_OFFSET
#define FRI_WEEK_OFFSET         5*STMD_1HOUR_OFFSET
#define SAT_WEEK_OFFSET         6*STMD_1HOUR_OFFSET

// 시간 type 값
#define STMD_MIN        0
#define STMD_HOUR       1
#define STMD_DAY        2
#define STMD_WEEK       3
#define STMD_MONTH      4

#define STMD_PERIOD_TYPE_NUM 5

// 주기적 통계 메시지 헤더 작성시 필요한 string
#define STMD_STR_MIN   	"5MIN"
#define STMD_STR_HOUR   "HOURLY"
#define STMD_STR_DAY    "DAILY"
#define STMD_STR_WEEK   "WEEKLY"
#define STMD_STR_MONTH  "MONTHLY"

#define STR_RULE_NAME_LEN	48


#define DEF_SYS_SCMA		0
#define DEF_SYS_SCMB		1

#define DEF_PREV			0
#define DEF_CURRENT			1

#define DEF_EXIST			1
#define DEF_NOT_EXIST		0

typedef struct __RuleSetItem__ {
	int 			eFlag;	// exist Flag
	int				real;	// RULESET 에도 있고, INI_VALUE에도 등록 되어있는 패키지.
	int 			rId;
	unsigned int	unblk;
	unsigned int	blk;
	unsigned int	red; // redirect cnt
	unsigned int	tot;
	long double 	uThru;
	long double		dThru;
	long double		tThru;
	long double 	uByte;
	long double		dByte;
	long double		tByte;
	unsigned int	actSub;
	unsigned int	totSub;
	char rName[STR_RULE_NAME_LEN];
	char phBit[5];
} RuleSetItem;

typedef struct __RuleSetList__ {
	int	 ruleSetCnt;		// INI_VALUES table에 등록된 rule set 개수 
	char sce_ip[STR_RULE_NAME_LEN];
	RuleSetItem stRule[MAX_RULE_NUM];
} RuleSetList;

typedef struct __RuleEntryItem__ {
	int 			eFlag;	// exist Flag
	int				real;
	int 			eId;
	unsigned int	unblk;
	unsigned int	blk;
	unsigned int	red; // redirect cnt
	unsigned int	tot;
	long double 	uThru;
	long double		dThru;
	long double		tThru;
	long double 	uByte;
	long double		dByte;
	long double		tByte;
	char 	eName[STR_RULE_NAME_LEN];
} RuleEntryItem;

typedef struct __RuleEntryList__ {
	int	 ruleEntryCnt;		// INI_VALUES table에 등록된 rule entry 개수 
	char sce_ip[STR_RULE_NAME_LEN];
	RuleEntryItem stEntry[MAX_RULE_NUM];
} RuleEntryList;

#define STR_IP_LEN		18
#define STR_DESC_LEN	64
typedef struct __PDSN_ITEM__ {
	int 			eFlag;	// exist Flag
	unsigned int	rx_cnt;
	unsigned int	start;
	unsigned int	interim; // redirect cnt
	unsigned int	stop;
	unsigned int	start_logon_cnt;
	unsigned int	int_logon_cnt;
	unsigned int	logout_cnt;

	unsigned int	log_req;
	unsigned int	log_succ;
	unsigned int	log_fail;
	unsigned int	HBIT_0;
	unsigned int	HBIT_1;
	unsigned int	HBIT_2;
	unsigned int	HBIT_3;
	unsigned int	HBIT_etc;

	unsigned int	sm_int_err;
	unsigned int	op_err;
	unsigned int	op_timeout;
	unsigned int	etc_fail;

	char			ip[STR_IP_LEN];
	char			desc[STR_IP_LEN];
} PDSN_ITEM;

#define STR_PDSN_NAME	10
typedef struct __PDSN_LIST__ {
	int	 		pdsnCnt;		// DB에 있는 PDSN IP 개수 
	char		pdsnName[STR_PDSN_NAME];
	PDSN_ITEM 	stItem[MAX_PDSN_NUM];
} PDSN_LIST;

typedef struct __SMSC_ITEM__ {
	int 			eFlag;	// exist Flag
	unsigned int	req;
	unsigned int	succ;
	unsigned int	fail; // redirect cnt
	unsigned int	smpp_err;
	unsigned int	svr_err;
	unsigned int	smsc_err;
	unsigned int	etc_err;

	char			ip[STR_IP_LEN];
} SMSC_ITEM;

typedef struct __SMSC_LIST__ {
	int	 		smscCnt;		// DB에 있는 PDSN IP 개수 
	SMSC_ITEM 	stItem[MAX_SMSC_NUM];
} SMSC_LIST;

typedef struct __SVC_ALM__ {
	int		logon_min; 	// alm 발생 최소 기준값 
	int		logon_rate;
	int		logout_min; // alm 발생 최소 기준값
	int		logout_rate;
	int		traffic_min; // alm 발생 최소 기준값 SCE 전체(A+B)  Mbps 단위
	int		traffic_rate;
} SVC_ALM;

typedef struct __SVC_VAL__ {
	int		logon_A[2]; // [0:prev / 1:cur]
	int		logon_B[2];
	int		logout_A[2];
	int		logout_B[2];
	int		traffic_A[2];
	int		traffic_B[2];
} SVC_VAL;

// MMC 명령어 리스트와 처리 function을 등록하는 table
#define STMD_MAX_MMC_HANDLER    35     
typedef struct {
    char    cmdName[COMM_MAX_NAME_LEN*2];
    int     (*func)(IxpcQMsgType*);
} StmdMmcHdlrVector;

// 시스템에 type, group,이름등을 저장할 구조
typedef struct {
    char    sysType[COMM_MAX_NAME_LEN];
    char    sysGroup[COMM_MAX_NAME_LEN];
    char    sysName[COMM_MAX_NAME_LEN];
} STMD_StatisticSystemInfo;

// 해당 시스템으로 통계 요청 메시지를 보내기 위한 구조
typedef struct {
    char    sysName[COMM_MAX_NAME_LEN];
    char    prcName[COMM_MAX_NAME_LEN];
} STMD_StatisticProcessInfo;

// 주기적 통계 메시지 헤더 작성시 필요한 string
#define STMD_STR_FAULT      "FAULT"
#define STMD_STR_LOAD       "LOAD"
#define STMD_STR_LINK       "LINK"
#define STMD_STR_LEG		"ACCOUNT"
#define STMD_STR_LOGON		"LOGON"
#define STMD_STR_FLOW		"FLOW"	// 2010.08.23
#define STMD_STR_RULE_SET	"RULE_SET"
#define STMD_STR_RULE_ENT	"RULE_ENT"
#define STMD_STR_SMS		"SMS"
#define STMD_STR_DEL		"DELAY"


#define RPT_LUR				"RPT_LUR"		// 2009.04.18 by jjinri
#define RPT_TR				"RPT_TR"		// 2009.04.18 by jjinri
#define RPT_BLOCK			"RPT_BLOCK"		// 2009.04.28 by jjinri

// Ondemand통계 처리를 위해
#define STMD_FAULT              0
#define STMD_LOAD               1
#define STMD_LINK               2
#define STMD_LEG               	3
#define STMD_LOGON            	4 
#define STMD_FLOW            	5 
#define STMD_RULE_SET			6 
#define STMD_RULE_ENT			7 
#define STMD_SMS				8 
#define STMD_DEL				9 

#ifdef DELAY
#define STMD_MASK_ITEM_NUM      9 // 2010.08.23
#else
#define STMD_MASK_ITEM_NUM      10 // 2010.08.23
#endif

#define ONDEMANDJOB     0
#define CRONJOB         1
#define PERIODIC        2
#define MMCJOB          3

#define MAX_ONDEMAND_NUM    18
#define MAX_CRONJOB_NUM     29
#define NOT_REGISTERED      -1

#define LOCAL_DB  0
#define REMOTE_DB 1

// Select RDR query. 2009.04.18 jjinri

#define SELECT_RPT_LEG "SELECT system_name, sum(login_cnt), max(login_cnt), min(login_cnt), avg(login_cnt), "\
						"sum(logout_cnt), max(logout_cnt), min(logout_cnt), avg(logout_cnt), "\
						"sum(login_succ), max(login_succ), min(login_succ), avg(login_succ), "\
						"sum(logout_succ), max(logout_succ), min(logout_succ), avg(logout_succ), "\
						"sum(rrstart_cnt), max(rrstart_cnt), min(rrstart_cnt), avg(rrstart_cnt), "\
						"sum(rrinterim_cnt), max(rrinterim_cnt), min(rrinterim_cnt), avg(rrinterim_cnt), "\
						"sum(rrstop_cnt), max(rrstop_cnt), min(rrstop_cnt), avg(rrstop_cnt) "\
						"FROM %s "\
						"WHERE log_date >= %ld and log_date < %ld "\
						"GROUP BY system_name, truncate(log_date/300,0)*300" 
/*
#define SELECT_RPT_LUR "SELECT RECORD_SOURCE, LINK_ID+1, sum(UPSTREAM_VOLUME), sum(DOWNSTREAM_VOLUME) "\
						"FROM %s "\
						"WHERE time_stamp >= '%s' and time_stamp < '%s' "\
						"GROUP BY RECORD_SOURCE, LINK_ID" 


#define SELECT_RPT_TR "SELECT RECORD_SOURCE, PACKAGE_ID, SERVICE_ID, SUM(SAMPLE_SIZ), "\
									"sum(UPSTREAM_VOLUME*SAMPLE_SIZE), sum(DOWNSTREAM_VOLUME*SAMPLE_SIZE) "\
							"FROM %s "\
							"WHERE time_stamp >= '%s' and time_stamp < '%s' "\
							"GROUP BY RECORD_SOURCE, PACKAGE_ID, SERVICE_ID "
*/

#define SELECT_RPT_BLOCK "SELECT RECORD_SOURCE, SUBSCRIBER_ID, PACKAGE_ID, SERVICE_ID, PROTOCOL_ID, "\
								"INITIATING_SIDE, BLOCK_REASON, SUM(BLOCK_RDR_CNT), REDIRECTED, "\
								"truncate(END_TIME/300,0)*300 "\
						"FROM %s "\
						"WHERE END_TIME >= %ld and END_TIME < %ld "\
						"GROUP BY RECORD_SOURCE, SUBSCRIBER_ID, PACKAGE_ID, SERVICE_ID, PROTOCOL_ID, "\
								"INITIATING_SIDE, BLOCK_REASON, REDIRECTED, truncate(END_TIME/300,0)*300" 


#define SELECT_RPT_LUR "SELECT RECORD_SOURCE, LINK_ID+1, sum(UPSTREAM_VOLUME), sum(DOWNSTREAM_VOLUME) "\
						"FROM %s "\
						"WHERE end_time >= %ld and end_time < %ld "\
						"GROUP BY RECORD_SOURCE, LINK_ID" 


#define SELECT_RPT_TR "SELECT RECORD_SOURCE, PACKAGE_ID, SERVICE_ID, SUM(SAMPLE_SIZ), "\
									"sum(UPSTREAM_VOLUME*SAMPLE_SIZE), sum(DOWNSTREAM_VOLUME*SAMPLE_SIZE) "\
							"FROM %s "\
							"WHERE end_time >= %ld and end_time < %ld "\
							"GROUP BY RECORD_SOURCE, PACKAGE_ID, SERVICE_ID "

#define SELECT_INI_VALUES "SELECT TIME_STAMP, SE_IP, VALUE_TYPE, VALUE_KEY," \
							"VALUE FROM INI_VALUES WHERE se_ip <> '10.160.250.125' "

#define SELECT_RDR_BLOCK "SELECT RECORD_SOURCE, PACKAGE_ID, SERVICE_ID, 0 UNBLK_CNT, sum(BLOCK_RDR_CNT), "\
								"1 stat_cnt, stat_date, stat_week "\
						"FROM %s "\
						"WHERE stat_date = '%s' "\
						"GROUP BY RECORD_SOURCE, PACKAGE_ID, SERVICE_ID"\

#define SELECT_RDR_RULESET " SELECT record_source, rule_set_id, IFNULL(session,0), IFNULL(upstream_volume,0), "\
								" IFNULL(downstream_volume,0) "\
						" FROM %s WHERE stat_date = '%s' and record_source = '%s' and rule_set_id = '%s' "

#define SELECT_RDR_RULEENT " SELECT record_source, rule_ent_id, session, upstream_volume, downstream_volume "\
						" FROM %s WHERE stat_date = '%s' and record_source = '%s' and rule_ent_id = %d "


typedef struct _SCE_
{
	char sce_name[8];
	char sce_ip[32];
	int	sce_id;
} SCE_t;

typedef struct _PDSN_
{
	char pdsn_ip[16];
} PDSN_t;

typedef struct {
    short   statisticsType; // 통계 종류를 기록하고 없을 경우 NOT_REGISTERED
    short   period; // 통계 데이타를 취합할 주기
    short   count; // 통계 데이타를 취합할 횟수
    short   Txcount; // 통계 데이타를 보낸 횟수
    short   cmdId; // canc-exe-cmd를 위해
    char    sysName[COMM_MAX_NAME_LEN]; // 취합할 시스템 이름
    char    ipAddr[COMM_MAX_NAME_LEN]; // 취합할 ip 주소
    char    measureTime[32]; // 통계 데이타를 취합할 시간
    char    statType[COMM_MAX_NAME_LEN]; // 통계 항목의 type을 기록
    char    svcName[COMM_MAX_NAME_LEN]; // 통계 서비스 이름
    int     svc_type; // Service type (wap1, wap2, http, java, vods, wipi)
    int     sysNO; // AAA and AN_AAA STAT_NAME TYPE by helca 2007.01.05 
    int     mscId; // MSC ID을 기록
} OnDemandList;

typedef struct {
    short   statisticsType; // 통계 종류를 기록하고 없을 경우 NOT_REGISTERED
    short   period; // 통계 데이타를 취합할 주기
    char    sysName[COMM_MAX_NAME_LEN]; // 취합할 시스템 이름
    char    ipAddr[COMM_MAX_NAME_LEN]; // 취합할 ip 주소
    char    measureTime[32]; // 통계 데이타를 취합할 시간
} CronList;

// stmd_main.c
extern int check_my_run_status (char *procname);
extern int InitSys(void);
extern int isTimeToWork (void);
extern int mysqlLiveCheck(void);
extern int readyToWork (int min, int sec);

extern int stmd_exeUpdateHourArea(void);
extern int stmd_exeUpdateDayArea(void);
extern int stmd_exeUpdateWeekArea(void);
extern int stmd_exeUpdateMonthArea(void);

extern int doPeriodicJob(void);
extern int doOnDemandJob(void);
extern int doCronJob(void);
extern int stmd_exeStatMsg (IxpcQMsgType *rxIxpcMsg);
extern int stmd_exeMMCMsg (IxpcQMsgType *rxIxpcMsg);

extern int stmd_hdlLegStatisticRpt(IxpcQMsgType *rxIxpcMsg);
extern int stmd_hdlLogonStatisticRpt(IxpcQMsgType *rxIxpcMsg);
extern int stmd_hdlFlowStatisticRpt(IxpcQMsgType *rxIxpcMsg);
extern int stmd_LoadStatisticRDReport(time_t t_start);
extern int stmd_InitStatisticRDReport(time_t t_start);
extern int stmd_LoadStatisticLUR(time_t t_start);
extern int stmd_LoadStatisticTR(time_t t_start);
extern int stmd_LoadStatisticBLOCK(time_t t_start);
extern int stmd_LoadStatisticRULESET(time_t t_start);
extern int stmd_LoadStatisticRULEENTRY(time_t t_start);
extern int stmd_LoadStatisticSMS(time_t t_start);
extern int stmd_LoadStatisticDELAY(time_t t_start);
extern int stmd_LoadStatisticLeg(time_t t_start);
extern int stmd_LoadStatisticLogon(time_t t_start);
extern int stmd_LoadStatisticFLOW(time_t t_start); // 2010.08.23

// stmd_init.c
extern int stmd_initLog (void);
extern int readPrintTime(void);
extern int readPrintMaskValue(void);
extern int readCronJobInFile(void);
extern int stmd_initStatisticReq_ProcInfo(void);
extern int getSceIP(void);
extern int writePrintTime(void);
extern int writePrintMaskValue(void);

extern int Insert_default_lur(void);
extern int Insert_default_ruleset(void);
extern int Insert_default_ruleent(void);
extern int Insert_default_sms(void);
extern int Insert_default_flow(void);

extern  void stmd_exeTxStatisticReqMsg(int*, int*);

extern int stmd_txMsg2Nmsib(char *buff, int msgId, char segFlag, char seqNo);

// stmd_mmchdl.c
extern int makeMsgDb(int type, int list, int code, char* tbl_type, char* msgBuf, char* startTime, char* endTime, char* cmdName, IxpcQMsgType *rxIxpcMsg);
extern int doDisDbStatHis(IxpcQMsgType *rxIxpcMsg, int year, int mon, int day, int hour, int min, int cnt);
extern char *get_select_endtime(int type, int year, int mon, int day, int hour, int min, int cnt);
extern int stmd_checkParaTimeValue (char *time, int *year, int *mon, int *day, int *hour, int *min);
extern int checkFreeList (int type);
extern int get_diff_time(char *pTime);
extern int writeCronJobInFile(void);


extern  int stmd_mmc_dis_stat_his(IxpcQMsgType*);
extern  int stmd_mmc_canc_exe_cmd(IxpcQMsgType*);
extern  int stmd_mmc_stat_load(IxpcQMsgType*);
extern  int stmd_mmc_stat_fault(IxpcQMsgType*);
extern  int stmd_mmc_stat_link(IxpcQMsgType*);
extern  int stmd_mmc_stat_leg(IxpcQMsgType*);
extern  int stmd_mmc_stat_logon(IxpcQMsgType*);
extern  int stmd_mmc_stat_flow(IxpcQMsgType*);		// 2010.08.23
extern  int stmd_mmc_stat_rule_set(IxpcQMsgType*);
extern  int stmd_mmc_stat_rule_ent(IxpcQMsgType*);
extern  int stmd_mmc_stat_sms(IxpcQMsgType*);
#ifdef DELAY
extern  int stmd_mmc_stat_delay2(IxpcQMsgType*);
#else
extern  int stmd_mmc_stat_delay(IxpcQMsgType*);
#endif
extern  int stmd_mmc_stat_leg(IxpcQMsgType*);
extern  int stmd_mmc_set_svc_alm(IxpcQMsgType*);
extern  int stmd_mmc_dis_svc_alm(IxpcQMsgType*);
extern  int stmd_mmc_add_pdsn_info(IxpcQMsgType*);
extern  int stmd_mmc_dis_pdsn_info(IxpcQMsgType*);
extern  int stmd_mmc_del_pdsn_info(IxpcQMsgType*);
extern  int stmd_mmc_chg_pdsn_info(IxpcQMsgType*);


extern int dWrite_PdsnIpConf(void);
extern int dLoad_PdsnIpConf(void);
extern void dLog_PdsnIpConf(void);

extern  int stmd_mmc_stat_db(IxpcQMsgType *);

extern  int stmd_mmc_del_stat_schd(IxpcQMsgType*);
extern  int stmd_mmc_add_stat_schd(IxpcQMsgType*);
extern  int stmd_mmc_dis_stat_nms(IxpcQMsgType*);
extern  int stmd_mmc_dis_stat_schd(IxpcQMsgType*);
extern  int stmd_mmc_dis_stat_info(IxpcQMsgType*);
extern  int stmd_mmc_dis_stat_ptime(IxpcQMsgType*);
extern  int stmd_mmc_chg_stat_ptime(IxpcQMsgType*);
extern  int stmd_mmc_mask_stat_item(IxpcQMsgType*);
extern  int stmd_mmc_umask_stat_item(IxpcQMsgType*);
extern  int stmd_mmc_dis_stat_mask(IxpcQMsgType*);

extern  int stmd_mmc_srch_stat_load(IxpcQMsgType*);
extern  int stmd_mmc_srch_stat_fault(IxpcQMsgType*);
extern  int stmd_mmc_srch_stat_link(IxpcQMsgType*);
extern  int stmd_mmc_srch_stat_leg(IxpcQMsgType*);
extern  int stmd_mmc_srch_stat_logon(IxpcQMsgType*);
extern  int stmd_mmc_srch_stat_flow(IxpcQMsgType*);		// 2010.08.23
extern  int stmd_mmc_srch_stat_rule_set(IxpcQMsgType*);
extern  int stmd_mmc_srch_stat_rule_ent(IxpcQMsgType*);
extern  int stmd_mmc_srch_stat_sms(IxpcQMsgType*);
extern  int stmd_mmc_srch_stat_delay(IxpcQMsgType*);

extern  char* toUpper(char* str);

extern  int stmd_hdlLoadStatisticRpt(IxpcQMsgType*);
extern  int stmd_hdlFltStatisticRpt(IxpcQMsgType*);
extern  int stmd_hdlRlegStatisticRpt(IxpcQMsgType*);

extern  int stmd_mmcHdlrVector_qsortCmp (const void *a, const void *b);
extern  int stmd_mmcHdlrVector_bsrchCmp (const void *a, const void *b);
extern  int get_system_information(char*, char*, char*);

extern  int stmd_exeRxQMsg (GeneralQMsgType*);
extern  int stmd_msg_receiver();

extern char *get_select_time2(int time_type);
extern char *get_period_end_time2(int time_type);
extern char *get_current_time();

extern  char *get_delete_time(int);
extern  char *get_insert_time();
extern  char *get_insert_time3();
extern  char *get_insert_week();
extern  char *get_insert_week2();
extern  char *get_select_time(int);
extern  char *get_now_time();
extern  char *get_on_demand_time(int);
extern  char *get_ondemand_time2(int period);
extern  char *get_sel_prev_min_time();
extern char *time_to_string(time_t time);
//extern void st_UpDownStat2HostStream(st_UpDownStat *pst_UpDownStat);
//extern void st_TotReqResStat2HostStream(st_TotReqResStat *pst_TotReqResStat);
//extern void st_ReqResStat2HostStream(st_ReqResStat *pst_ReqResStat);
//extern void st_TranInfo2HostStream(st_TranInfo *pst_TranInfo);
long long htonll(long long );
long long ntohll(long long );

int stmd_mysql_init(int type);
int stmd_mysql_query (char *query);
int stmd_mysql_query_with_conn(MYSQL *con, char *query);
int stmd_mysql_select_query (char *query);
int stmd_ondemand_txMMLResult (int list, char *resBuf, char resCode, char contFlag, unsigned short extendTime, char *cmdName, char segFlag, char seqNo);
char *get_ondemand_time(int period);
char *get_ondemand_delay();

int stmd_cron_txMsg2Cond(char *buff, int msgId, char segFlag, char seqNo);
int stmd_txMsg2Cond(char *buff, int msgId, char segFlag, char seqNo);
int stmd_txMsg2FIMD(char *buff);
int stmd_txMMLResult ( IxpcQMsgType *rxIxpcMsg, char *resBuf, char resCode, char contFlag, unsigned short extendTime, char segFlag, char seqNo);

extern  int SendMsg(int , int, int,  char* , char* , char , char , char, IxpcQMsgType * );
extern  int sendCondResultMsg(int type, char* msgBuf, int code);

// RDR Report 관련 function  in main.c by jjinri 2009.04.18
int stmd_LoadStatisticRDReport(time_t t_now);
int stmd_LoadStatisticLUR(time_t t_start);
int stmd_LoadStatisticTR(time_t t_start);
int connectLoDB(void);
int connectRmDB(void);
int keepaliveLoDB(void);
int keepaliveRmDB(void);
char *get_insert_week_RDR(time_t t_now);
int readRuleConfFile(char *fname);

// SVC ALM 관련 함수 
extern int dInit_SvcAlmConf(void);
extern int dLoad_SvcAlmConf(void);
extern void dLog_SvcAlmConf(void);
extern int dWrite_SvcAlmConf(void);
extern int makeAlmMessage (char *system, char *svcname, double rate, int RATE, int prev, int curr, int MIN );
extern int stmd_txAlmMsg2Cond (char *buff, long mtype, int msgId);
extern int checkSvcAlm(void);

// Update ()
int stmd_exeUpdateHourLoad(void);
int stmd_exeUpdateHourFlt(void);
int stmd_exeUpdateHourLUR(void);
int stmd_exeUpdateHourBLOCK(void);
int stmd_exeUpdateHourLogOn(void);
int stmd_exeUpdateHourFlow(void);		// 2010.08.23
int stmd_exeUpdateHourLeg(void);
int stmd_exeUpdateHourRULESET(void);
int stmd_exeUpdateHourRULEENT(void);
int stmd_exeUpdateHourSms(void);
int stmd_exeUpdateHourDelay(void);

int stmd_exeUpdateDayLoad(void);
int stmd_exeUpdateDayFlt(void);
int stmd_exeUpdateDayLUR(void);
int stmd_exeUpdateDayBLOCK(void);
int stmd_exeUpdateDayLeg(void);
int stmd_exeUpdateDayLogOn(void);
int stmd_exeUpdateDayFlow(void);		// 2010.08.23
int stmd_exeUpdateDayRULESET(void);
int stmd_exeUpdateDayRULEENT(void);
int stmd_exeUpdateDaySms(void);
int stmd_exeUpdateDayDelay(void);

int stmd_exeUpdateWeekLoad(void);
int stmd_exeUpdateWeekFlt(void);
int stmd_exeUpdateWeekLUR(void);
int stmd_exeUpdateWeekBLOCK(void);
int stmd_exeUpdateWeekLeg(void);
int stmd_exeUpdateWeekLogOn(void);
int stmd_exeUpdateWeekFlow(void);		// 2010.08.23
int stmd_exeUpdateWeekRULESET(void);
int stmd_exeUpdateWeekRULEENT(void);
int stmd_exeUpdateWeekSms(void);
int stmd_exeUpdateWeekDelay(void);

int stmd_exeUpdateMonthLoad(void);
int stmd_exeUpdateMonthFlt(void);
int stmd_exeUpdateMonthLUR(void);
int stmd_exeUpdateMonthBLOCK(void);
int stmd_exeUpdateMonthLeg(void);
int stmd_exeUpdateMonthLogOn(void);
int stmd_exeUpdateMonthFlow(void);		// 2010.08.23
int stmd_exeUpdateMonthRULESET(void);
int stmd_exeUpdateMonthRULEENT(void);
int stmd_exeUpdateMonthSms(void);
int stmd_exeUpdateMonthDelay(void);

// Periodic
void getFilePath ( char *path, char tm[][8], time_t *tt );
void makeDirectory (int time_type,char *path, char tm[][8] );
void makeFileName ( char *fileName, int statType, int timeOpt , char tm[][8] );
char *get_period_start_time(int time_type);
char *get_period_end_time(int time_type);
char *get_period_select_time(int time_type);
int send_query_info (int mprd, char *stm, char *etm);

int doPeriodicHourly(void);
int doPeriodicDaily(void);
int doPeriodicWeekly(void);
int doPeriodicMonthly(void);

int doPeriodicHourRuleEnt(void);
int doPeriodicHourLoad(void);
int doPeriodicHourFlt(void);
int doPeriodicHourLink(void);
int doPeriodicHourRuleSet(void);
int doPeriodicHourLeg(void);
int doPeriodicHourLogon(void);
int doPeriodicHourFlow(void);		// 2010.08.23
int doPeriodicHourSms(void);
int doPeriodicHourDelay(void);

int doPeriodicDayRuleEnt(void);
int doPeriodicDayLoad(void);
int doPeriodicDayFlt(void);
int doPeriodicDayLink(void);
int doPeriodicDayRuleSet(void);
int doPeriodicDayLeg(void);
int	doPeriodicDayLogon(void);
int doPeriodicDayFlow(void);		// 2010.08.23
int doPeriodicDaySms(void);
int doPeriodicDayDelay(void);

int doPeriodicWeekRuleEnt(void);
int doPeriodicWeekLoad(void);
int doPeriodicWeekFlt(void);
int doPeriodicWeekLink(void);
int doPeriodicWeekRuleSet(void);
int doPeriodicWeekLeg(void);
int doPeriodicWeekLogon(void);
int doPeriodicWeekFlow(void);		// 2010.08.23
int doPeriodicWeekSms(void);
int doPeriodicWeekDelay(void);

int doPeriodicMonthRuleEnt(void);
int doPeriodicMonthLoad(void);
int doPeriodicMonthFlt(void);
int doPeriodicMonthLink(void);
int doPeriodicMonthRuleSet(void);
int doPeriodicMonthLeg(void);
int doPeriodicMonthLogon(void);
int doPeriodicMonthFlow(void);		// 2010.08.23
int doPeriodicMonthSms(void);
int doPeriodicMonthDelay(void);

// OnDemand
int doOnDemandLink(int list);
int doOnDemandFault(int list);
int doOnDemandLoad(int list);
int doOnDemandLeg(int list);
int doOnDemandLogon(int list);
int doOnDemandFlow(int list);		// 2010.08.23
int doOnDemandRuleSet(int list);
int doOnDemandRuleEnt(int list);
int doOnDemandSms(int list);
#ifdef DELAY
int doOnDemandDelay2(int list);
#else
int doOnDemandDelay(int list);
#endif

// Cron
int doCronFault(int list);
int doCronLoad(int list);
int doCronLink(int list);
int doCronRuleSet(int list);
int doCronRuleEnt(int list);
int doCronLeg(int list);
int doCronLogon(int list);
int doCronFlow(int list);			// 2010.08.23
int doCronSms(int list);
int doCronDel(int list);

#endif



