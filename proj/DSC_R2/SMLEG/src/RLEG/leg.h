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
#include "comm_timer.h"
#include "sm_subs_info.h"
#include "common_ana.h"

enum { NAS_NULL=0, NAS_1x=58, NAS_EVDO=59, };
enum { DO_CONN_OFF=0, DO_CONN_ON_NB, DO_CONN_ON_B, DO_CONN_ON_BOTH };

enum {PERIOD_HOUR=0, PERIOD_DAY, PERIOD_WEEK, PERIOD_MONTH};
enum {CMD_USE_OFF=0, CMD_USE_ON};
#define     HIPADDR(d)      ((d>>24)&0xff),((d>>16)&0xff),((d>>8)&0xff),(d&0xff)
#define     NIPADDR(d)      (d&0xff),((d>>8)&0xff),((d>>16)&0xff),((d>>24)&0xff)

/* SM API */
#define SM_API_BUF_SIZE						4000000		// CISCO 권고사항(수정하지 말것)

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

#define MAX_PBIT_CNT						100
#define MAX_HBIT_CNT						100		// RULESET 관리에 사용. ex) RULESET[MAX_PBIT_CNT][MAX_HBIT_CNT]
#define MAX_HBIT_STAT_CNT					32		// LOGON STATISTIC 에 사용.

#define	TIMER_HB_PERIOD						1000
#define	TIMER_CPS_PERIOD					1000*5
#define	TIMER_STATISTIC_PERIOD				1000*5*60
#define	DAY2SEC								24*60*60
#define	DAY2HOUR							24
#define	HOUR2SEC							60*60
#define	HOUR2MILISEC						60*60*1000

#define	_ACTIVE								1
#define	_STANDBY							2
#define	_FAULTED							3

#define LOOP_MSG_COUNT						10000

/* CALL CONTROL ERROR CODE */
/* 1. LOGON */
#define ERR_1001							-1001			// SMAPI Handler is NULL
#define ERR_1002							-1002			// IP is NULL
#define ERR_1003							-1003			// SMAPI Return is (-1)
#define ERR_1004							-1004			// session not found
#define ERR_1005							-1005			// ruleset is negative value

/* 2. LOGOUT */
#define ERR_2001							-2001			// SMAPI Handler is NULL
#define ERR_2002							-2002			// IP is NULL
#define ERR_2003							-2003			// SMAPI Return is (-1)
#define ERR_2004							-2004			// session not found
#define ERR_2005							-2005			// ruleset is negative value

/* 3. RESPONSE CALLBACK */
#define ERR_3001							-3001			// SUCCESS CALLBACK - session not found
#define ERR_3002							-3002			// SUCCESS CALLBACK - oper mode invalid
#define ERR_4001							-4001			// FAIL CALLBACK - session not found
#define ERR_4002							-4002			// FAIL CALLBACK - oper mode invalid

typedef struct _st_sm_info_ {
#define MAX_SM_NUM					2
#define MAX_SM_IP_SIZE				16
#define MAX_SVCOPT_COUNT			5
	int             port;
	char            ip[MAX_SM_IP_SIZE];
	int				svc_type[MAX_SVCOPT_COUNT];
} SM_INFO;

#define DEF_ST_SM_INFO_SIZE		sizeof(SM_INFO)

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

typedef struct _st_handle_ {
	int 	argHandle;
	int		dCBType;
	int		dErrCode;
	char 	szMessage[1];	// 성능 문제로 일단 Error Reason String 은 전송하지 않는다.
} HANDLE_t;

/////////////////////////////////////////////////////////////////////////////////////
/* global variable definition */
extern char sysLable[COMM_MAX_NAME_LEN], mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN];
extern int  dMyQid, dIxpcQid, dRADIUSQid;
extern int	JiSTOPFlag, FinishFlag;
extern SM_INFO				gSCM[MAX_SM_NUM];
extern SM_CONN				gSMConn;
//extern MMC_LOGOUT_INFO 		mmcLOGOUT[NUM_HANDLE];
extern SFM_SysCommMsgType	*loc_sadb;
extern MPTimer				*gpMPTimer[DEF_SET_CNT];
extern MPTimer				*gpCurMPTimer;
extern SM_BUF_CLR			gSmBufClr[MAX_RLEG_CNT];
extern SM_BUF_CLR			*gpMySmBufClr;

/////////////////////////////////////////////////////////////////////////////////////
/* leg_init.c */
extern void SetUpSignal(void);
extern int initProc(void);
extern void finProc(void);
int dGetConfig_LEG (void);
#if 0
extern int dLoadTimeOut(void);
extern void dLogTimeOut(int level);
extern int dWriteTimeOut(void);
#endif

/* leg_radius.c */
extern void makeSubsRecords (pst_RADInfo pstRADInfo, PSUBS_INFO psi);
extern int branchMessage (GeneralQMsgType *prxGenQMsg);

/* leg_sce_comm.c */
extern void connectionIsDown(void);
extern int connectSCE (int mode);
extern int connect_sm (int mode);
extern void handleError (Uint32 argHandle, ReturnCode* argReturnCode);
extern void handleSuccess (Uint32 argHandle, ReturnCode* argReturnCode);
//extern void handleError_MMC (Uint32 argHandle, ReturnCode* argReturnCode);
//extern void handleSuccess_MMC (Uint32 argHandle, ReturnCode* argReturnCode);
extern void disconnSCE (void);
extern int	loginSCE (SUBS_INFO *si);
extern int	logoutSCE (SUBS_INFO *si, unsigned short usSvcID);
extern int StartThread (int nProcess, void *pProc);
extern void theApp (void);
extern int checkConnectSCE (void);

#define 	MMC_LOGOUT_SUCC			0
#define 	MMC_LOGOUT_FAIL			1

/** Trace **/
#define 	TRACE_METHOD_IMSI			1
#define 	TRACE_METHOD_IP				2

#define 	TRACE_TYPE_ACC_START		1
#define		TRACE_TYPE_ACC_INTERIM  	2
#define 	TRACE_TYPE_ACC_END			3
#define 	TRACE_TYPE_LOGIN			4
#define 	TRACE_TYPE_LOGOUT			5
#define 	TRACE_TYPE_LOGOUT_TIMEOUT	6
#define 	TRACE_TYPE_DISCONN_REQ		7
#define 	TRACE_TYPE_LOGIN_FAIL		8
#define 	TRACE_TYPE_LOGIN_SUCCESS	9
#define 	TRACE_TYPE_LOGOUT_FAIL		10
#define 	TRACE_TYPE_LOGOUT_SUCCESS	11

// Handle
#define		SM_CB_HANDLE_SUCC			0
#define		SM_CB_HANDLE_FAIL			1
#define		SM_CB_HANDLE_DISCON			2

extern void Trace_LOGIN (SUBS_INFO *pSub, int TrcType, int dErrCode);
extern void Trace_LOGOUT(SUBS_INFO *pSub, int TrcType, int dErrCode);
extern int Send_CondTrcMsg_RLEG (SUBS_INFO *pSub, int TrcType, int dErrCode);

/** timer **/
extern void set_cb_timeout(void (*func)(int, void*), int key, void* data, unsigned int msec);

extern int check_my_run_status (char *procname);

extern int dProcHandle (HANDLE_t *pHandle);
extern int dSendCallBack (int argHandle, int dHandle, int dErrCode, char *msg);

#endif /* __RLEG_H__ */

