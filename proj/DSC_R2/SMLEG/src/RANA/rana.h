#ifndef __RLEG_H__
#define __RLEG_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "comm_define.h"
#include "comm_call_errcode.h"
#include "SmApiBlocking_c.h"
#include "SmApiNonBlocking_c.h"
#include "ipaf_svc.h"
#include "ipaf_stat.h"
#include "comm_msgtypes.h"
#include "comm_trace.h"
#include "sfm_msgtypes.h"
#include "utillib.h"
#include "keepalivelib.h"
#include "rana_mmc_hld.h"
#include "comm_timer.h"
#include "sm_subs_info.h"
#include "common_ana.h"

enum { NAS_NULL=0, NAS_1x=58, NAS_EVDO=59, };
enum { DO_CONN_OFF=0, DO_CONN_ON_NB, DO_CONN_ON_B, DO_CONN_ON_BOTH };

#define     HIPADDR(d)      ((d>>24)&0xff),((d>>16)&0xff),((d>>8)&0xff),(d&0xff)
#define     NIPADDR(d)      (d&0xff),((d>>8)&0xff),((d>>16)&0xff),((d>>24)&0xff)

/* Connection munber with SM */
#define MAX_SM_CONN_COUNT					5	
#define SM_CONN_MMC_NUM						MAX_SM_CONN_COUNT

#define _SYS_NAME_MPA						"SCMA"
#define _SYS_NAME_MPB						"SCMB"

#define PSDN_FILE            				"NEW/DATA/PDSN.conf"
#define RULESET_USED_FILE            		"NEW/DATA/RULESET_USED.conf"
#define CALL_OVER_CTRL_FILE           		"NEW/DATA/CALL_OVER_CTRL.conf"
#define TIMEOUT_FILE           				"NEW/DATA/TIMEOUT.conf"
#define MMC_MSG_SIZE              			4096
/* DEBUGGING 용 */
#define TRAFFIC_CHECK_TIME					300

#define MAX_CALLING_STATION_ID_SIZE			16
#define MAX_FRAMEDIP_SIZE		 			16
#define MAX_DOMAIN_SIZE		 				16
#define LEG_SCM_ADDR						"211.254.95.210"
#define LEG_SCM_PORT						14374
#define MAX_LEG2SM_CONN_DELAY_CNT			60


#define	TIMER_HB_PERIOD						1000
#define	TIMER_CPS_PERIOD					1000*5
#define	TIMER_STATISTIC_PERIOD				1000*5*60
#define	DAY2SEC								24*60*60
#define	DAY2HOUR							24
#define	HOUR2SEC							60*60
#define	HOUR2MILISEC						60*60*1000
#define LOOP_MSG_COUNT						10000

/* CALL CONTROL ERROR CODE */
#if 0
/* 1. account-request(start) */
#define ERR_10001							-10001			// p/h bit escapes a scope	
#define ERR_10002							-10002			// ruleset used flag off
#define ERR_10003							-10003			// not match service option
#define ERR_10004							-10004			// not exist p/hbit at rulefile
#define ERR_10005							-10005			// ruleset is negative value
#define ERR_10006							-10006			// ruleset equal
#define ERR_10007							-10007			// cps over
#define ERR_10008							-10008			// session not created
#define ERR_10009							-10009			// session not found
#define ERR_10010							-10010			// route to RLEG fail
#define ERR_10011							-10011			// send to RLEG fail
#define ERR_10012							-10012			//

/* 2. account-request(interim) */
#define ERR_20001							-20001			// p/h bit escapes a scope
#define ERR_20002							-20002			// ruleset used flag off
#define ERR_20003							-20003			// not match service option
#define ERR_20004							-20004			// not exist p/hbit at rulefile
#define ERR_20005							-20005			// ruleset is negative value
#define ERR_20006							-20006			// ruleset equal
#define ERR_20007							-20007			// cps over
#define ERR_20008							-20008			// session not created
#define ERR_20009							-20009			// session not found
#define ERR_20010							-20010			// route to RLEG fail
#define ERR_20011							-20011			// send to RLEG fail
#define ERR_20012							-20012

/* 3. account-request(stop) */
#define ERR_30001							-30001			// p/h bit escapes a scope
#define ERR_30002							-30002			// ruleset used flag off
#define ERR_30003							-30003			// not match service option
#define ERR_30004							-30004			// not exist p/hbit at rulefile
#define ERR_30005							-30005			// ruleset is negative value
#define ERR_30006							-30006			// ruleset equal
#define ERR_30007							-30007			// cps over
#define ERR_30008							-30008			// session not created
#define ERR_30009							-30009			// session not found 
#define ERR_30010							-30010			// route to RLEG fail
#define ERR_30011							-30011			// send to RLEG fail
#define ERR_30012							-30012			// used flag zero and session not found
#define ERR_30013							-30013			// used flag zero and non-logon session
#define ERR_30014							-30014			// not exist p/hbit at rulefile

/* 4. disconnect-request */
#define ERR_40001							-40001			// p/h bit escapes a scope
#define ERR_40002							-40002			// ruleset used flag off
#define ERR_40003							-40003			// not match service option
#define ERR_40004							-40004			// not exist p/hbit at rulefile
#define ERR_40005							-40005			// ruleset is negative value
#define ERR_40006							-40006			// ruleset equal
#define ERR_40007							-40007			// cps over
#define ERR_40008							-40008			// session not created
#define ERR_40009							-40009			// session not exist
#define ERR_40010							-40010			// route to RLEG fail
#define ERR_40011							-40011			// send to RLEG fail
#define ERR_40012							-40012			// hbit attribute not found
#define ERR_40013							-40013			// hbit equal 

/* 5. session timeout */
#define ERR_50001							-50001			// p/h bit escapes a scope
#define ERR_50002							-50002			// ruleset used flag off
#define ERR_50003							-50003			// not match service option
#define ERR_50004							-50004			// not exist p/hbit at rulefile
#define ERR_50005							-50005			// ruleset is negative value
#define ERR_50006							-50006			// ruleset equal
#define ERR_50007							-50007			// cps over
#define ERR_50008							-50008			// session not created
#define ERR_50009							-50009			// session not found
#define ERR_50010							-50010			// route to RLEG fail
#define ERR_50011							-50011			// send to RLEG fail
#define ERR_50012							-50012			// system mode is standby
#define ERR_50013							-50013			// system mode is fault
#define ERR_50014							-50014			// not logon session

#endif
/////////////////////////////////////////////////////////////////////////////////////////////
typedef struct _st_sm_info_ {
#define MAX_SM_NUM					2
#define MAX_SM_IP_SIZE				16
#define MAX_SVCOPT_COUNT			5
	int             port;
	char            ip[MAX_SM_IP_SIZE];
	int				svc_type[MAX_SVCOPT_COUNT];
} SM_INFO;

#define DEF_ST_SM_INFO_SIZE		sizeof(SM_INFO)

#if 0
typedef struct __subscribers_info {

#define		MAX_PROPORTIES_STR_SIZE			16
#define		MAX_PROPERY_CNT					2

#define		PROPERTY_PACKAGE_ID				"packageId"
#define		PROPERTY_MONITOR				"monitor"

#define		PROPERTY_MONITOR_MODE_OFF		0
#define		PROPERTY_MONITOR_MODE_ON		1

	SHORT 			sPkgNo;
	UINT			uiCBit;
	UINT			uiPBit;
	UINT			uiHBit;
	unsigned char 	szMIN[MAX_CALLING_STATION_ID_SIZE];
	unsigned char 	szFramedIP[MAX_FRAMEDIP_SIZE];
	unsigned char 	szDomain[MAX_DOMAIN_SIZE];
	MappingType 	type;
	char 			*prop_key[MAX_PROPERY_CNT];
	char 			*prop_val[MAX_PROPERY_CNT];
	int				prop_size;
	int 			dlogoutTmrMode;
	int				dTrcFlag;
} SUBS_INFO, *PSUBS_INFO;


typedef struct _st_login_info_{
	int 			dArgIsAdditive;
	int				dargAutoLogoutTime;
} SM_LOGIN_INFO, *PSM_LOGIN_INFO;

/* PH Bit Table Information */
typedef struct _st_Pkginfo_ {
	UCHAR			ucUsedFlag;
	UCHAR			ucSMSFlag;
	UCHAR 			ucReserved[2];
	SHORT			sPkgNo;
	SHORT 			sRePkgNo;
} ST_PKG_INFO, *PST_PKG_INFO;

#define	DEF_ST_PKG_INFO_SIZE 	sizeof(ST_PKG_INFO)

typedef struct _st_PBTable_List_ {
	UINT 			dCount;
	ST_PKG_INFO		stPBTable[MAX_PBIT_CNT][MAX_HBIT_CNT];		/* PBit, HBit Table */
} ST_PBTABLE_LIST, *PST_PBTABLE_LIST;

#define	DEF_ST_PBTABLE_SIZE 	sizeof(ST_PBTABLE_LIST)

typedef struct _st_pdsn_attr_ {
#define DEF_PDSN_NAME_SIZE			16
	UINT			uiAddr;
	UCHAR			ucName[DEF_PDSN_NAME_SIZE];
} PDSN_ATTR, *PPDSN_ATTR;

typedef struct _st_pdsn_list_ {
	UINT			uiCount;
	UINT			uiAddr[DEF_PDSN_CNT];
} PDSN_LIST, *PPDSN_LIST;
#define DEF_PDSN_LIST_SIZE		sizeof(PDSN_LIST)

typedef struct _st_ruleset_used_info_ {
	UINT			uiPBit;
	UINT			uiHBit;
	UINT 			uiOperF;		/* mmc 로 uiUsedF 설정 여부 */
	UINT			uiUsedF;		/* rule set 사용 여부 */
} RULESET_USED_INFO, *pRULESET_USED_INFO;

typedef struct _st_ruleset_used_flag_{
	UINT				uiCount;
	RULESET_USED_INFO	stRule[MAX_PBIT_CNT][MAX_HBIT_CNT];
} RULESET_USED_FLAG, *PRULESET_USED_FLAG;

typedef struct _st_sm_info_ {
#define MAX_SM_NUM					2
#define MAX_SM_IP_SIZE				16
#define MAX_SVCOPT_COUNT			5
	int             port;
	char            ip[MAX_SM_IP_SIZE];
	int				svc_type[MAX_SVCOPT_COUNT];
} SM_INFO;

#define DEF_ST_SM_INFO_SIZE		sizeof(SM_INFO)

typedef struct _st_sm_conn_ {
	int 			isPreConnect;
	int 			isConnect;
	SMNB_HANDLE     hdl_nbSceApi;
	SMB_HANDLE      hdl_bSceApi;
} SM_CONN;

typedef struct _st_sm_resp_ {
	long long		llSuccCnt;
	long long		llFailCnt;
} SM_RSP;

typedef struct _st_statistic_ {
	long long       llRadStartCnt;
	long long       llRadInterimCnt;
	long long       llRadStopCnt;
	long long       llLoginCnt;
	long long       llLoginSuccCnt;
	long long       llLogoutCnt;
	long long       llLogoutSuccCnt;

} LEG_STATISTIC;

typedef struct _st_rleg_stat_ {
	unsigned int	uiRecvCnt;
	unsigned int	uiFailCnt;
	LEG_STAT1		stat;
} RLEG_STAT;

typedef struct _st_leg_cps_ {
	long long	llLogOnCps;
	long long	llLogOutCps;
} LEG_CPS;

typedef struct _st_leg_cps_info_ {
	unsigned int uiPrevLogOnCps;
	unsigned int uiLogOnCps;
	unsigned int uiLogOutCps;
	unsigned int uiSuccCps;
	unsigned int uiFailCps;
} LEG_CPS_INFO;

typedef struct _st_leg_cps_conut_ {
	unsigned int llLogOnCps;
	unsigned int llLogOutCps;
	unsigned int llSuccCps;
	unsigned int llFailCps;
} LEG_CPS_COUNT;

typedef struct _st_cps_ovld_ctrl_ {
	int				over_cps;
	int				over_rate;
	unsigned char 	over_flag;
} CPS_OVLD_CTRL;
/* end 2009.06.10 by dhkim */
#endif

#define NUM_PROPERTY	2
#define NUM_HANDLE		0xFFFF
#define SM_HANDLE_NUM	0xFFFF
#define GETHANDLE(d)	((Uint32)(d%NUM_HANDLE))
typedef struct _hLOGIN {
	char *key[NUM_PROPERTY];
	char *val[NUM_PROPERTY];
} hLOGIN;

typedef struct _st_logout_info_ {
	int				flag;
	char    		szIMSI[MAX_CALLING_STATION_ID_SIZE];
	IxpcQMsgType 	rxIxpcMsg;
} MMC_LOGOUT_INFO;

/////////////////////////////////////////////////////////////////////////////////////
/* global variable definition */
extern char sysLable[COMM_MAX_NAME_LEN], mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN];
extern int  dMyQid, dIxpcQid, dRADIUSQid;
extern int	JiSTOPFlag, FinishFlag;
extern SM_INFO          	gSCM[MAX_SM_NUM];
extern SM_CONN				gSMConn;
#define HOURTO5MIN			12	
extern int					g_dReloadPDSN;

extern SFM_SysCommMsgType	*loc_sadb;
/////////////////////////////////////////////////////////////////////////////////////
/* leg_init.c */
extern void SetUpSignal(void);
extern int initProc(void);
extern void finProc(void);
extern int dCheck_PDSN_IP(INT tid, UINT addr);
extern int dCheck_SvcOpt(int svc_opt);
extern int dGetConfig_LEG (void);

/* leg_radius.c */
extern void makeSubsRecords (pst_RADInfo pstRADInfo, PSUBS_INFO psi);
//extern void makeSubsRecords_bySess (rad_sess_key *pkey, rad_sess_body *pBody, PSUBS_INFO psi);

/* leg_msgq.c */
extern int msgQRead	(int tid, pst_MsgQ pstMsgQ);
extern void makeCpsData(void);
extern void makeStatData(int idx);
extern void report_StatData2STMD(void);
extern void report_CallData2FIMD(void);

/** Trace **/
#define 	TRACE_METHOD_IMSI					1
#define 	TRACE_METHOD_IP						2

#define 	TRACE_TYPE_ACC_START				1
#define		TRACE_TYPE_ACC_INTERIM  			2
#define 	TRACE_TYPE_ACC_END					3
#define 	TRACE_TYPE_DISCONN_REQ				4
#define 	TRACE_TYPE_LOGIN					5
#define 	TRACE_TYPE_LOGIN_OK					6
#define 	TRACE_TYPE_LOGIN_FAIL				7
#define 	TRACE_TYPE_NOT_LOGIN				8
#define 	TRACE_TYPE_LOGOUT					9
#define 	TRACE_TYPE_LOGOUT_OK				10
#define 	TRACE_TYPE_LOGOUT_FAIL				11
#define 	TRACE_TYPE_NOT_LOGOUT				12
#define 	TRACE_TYPE_TIMEOUT					13
#define 	TRACE_TYPE_TIMEOUT_LOGOUT			14
#define 	TRACE_TYPE_TIMEOUT_NOT_LOGOUT		15

extern void Trace_LOGIN (st_RADInfo *pstRADInfo, int acc_type, ST_PKG_INFO *pstPKGInfo);
extern int Trace_LOGIN_Req (st_RADInfo *pstRADInfo, int acc_type);
extern int Trace_LOGOUT(st_RADInfo *pstRADInfo, int acc_type);
extern int dGetTraceData(pst_SESSInfo pstInfo);
extern int dLoadTraceData(pst_SESSInfo pstInfo);
extern int Send_CondTrcMsg_RLEG (pst_RADInfo pstRADInfo, int acc_type, int trace_type, int pkgNo);

/** timer **/

extern int check_my_run_status (char *procname);
extern int dSend_CpsOverLoadReport(unsigned char over_flag);
extern int dCheck_CpsOverLoad(int cur_cps, CPS_OVLD_CTRL *ctrl);
extern int SendToRLEG(int tidx, SUBS_INFO *pSubInfo, int logmode);

extern void PrintCallData(const LEG_DATA_SUM *p, const unsigned int uiAmount);
extern void InitCallData(LEG_DATA_SUM *p);
extern void PrintStat(const LEG_TOT_STAT_t *pTotStat);
extern void InitStat(LEG_TOT_STAT_t *pTotStat);
extern void set_cb_timeout(void (*func)(int, void*), int key, void* data, unsigned int msec);

/** mmc ***/

#endif /* __RLEG_H__ */

