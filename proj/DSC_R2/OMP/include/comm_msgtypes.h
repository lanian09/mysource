#ifndef __COMM_MSGTYPES_H__
#define __COMM_MSGTYPES_H__

//------------------------------------------------------------------------------
// 모든 시스템에서 공통으로 사용되는 메시지에 대한 정보를 선언한다.
//------------------------------------------------------------------------------

#define COMM_MAX_NAME_LEN	16	
#define COMM_MAX_VALUE_LEN	100

// mtype range : common : 1 ~ 99
//			   OMP	: 100 ~ 199
//			   BSD	: 200 ~ 299
#define MTYPE_SETPRINT					1
#define MTYPE_IXPC_CONNECTION_CHECK		2
#define MTYPE_MMC_REQUEST				3
#define MTYPE_MMC_RESPONSE				4
#define MTYPE_STATISTICS_REQUEST		5
#define MTYPE_STATISTICS_REPORT			6
#define MTYPE_STATUS_REQUEST			7
#define MTYPE_STATUS_REPORT				8
#define MTYPE_ALARM_REPORT				9
#define MTYPE_STAT_REPORT_SHORT_TERM   	10
#define MTYPE_STAT_REPORT_LONG_TERM		11
#define MTYPE_MMC_SYNTAX_RESPONSE	  	12
#define	MTYPE_DB_DISCONNECT				13
#define MTYPE_TRC_CONSOLE				14
#define MTYPE_MAP_NOTIFICATION			15
#define MTYPE_BSD_ALARM_REPORT			16
#define MTYPE_CONSOLE_REPORT			17

/* NEW MTYPE ADDED FOR UAWAP TRANSLOG (061219) */
#define MTYPE_UAWAP_TRANSLOG		   	18

/* NO TRANSMITTED UDR FILE ACT/DACT */
#define MTYPE_TRANINIT_REQUEST			19
#define MTYPE_NO_TRANSMITTED_ACT		20
#define MTYPE_NO_TRANSMITTED_DACT		21

#define MTYPE_QUEUE_CLEAR_REPORT	   	22

/* NEW MTYPE for UDRGEN/UDRCOL */
#define MTYPE_UDR_GET					31
#define MTYPE_UDR_GET_RESP				32

/* NEW MTYPE for configuration of MMCR and MP Block */
#define MTYPE_BSD_CONFIG				41

// PDSN CONF FILE SYNC 
// MODIFY : by jjinri, 2010-04-15
#define MTYPE_PDSN_CONFIG				43

/* SDMD Duplication Status Request/Respose */
#define MTYPE_DUP_STATUS_REQUEST			50
#define MTYPE_DUP_STATUS_RESPONSE			51
#define MTYPE_DUP_UPDATE_NOTI				52


// msg_id 선언 부분
// msg_id range : MTYPE_STATISTICS_REQUEST	  : 1 ~ 99
//				MTYPE_STATISTICS_REPORT	   : 100 ~ 199
//				MTYPE_STATUS_REQUEST		  : 200 ~ 299
//				MTYPE_STATUS_REPORT		   : 300 ~ 399
//				MTYPE_ALARM_REQUEST		   : 400 ~ 499
//				MTYPE_ALARM_REPORT			: 500 ~ 599
//				MTYPE_MAINTENANCE_NOTIFICATION: 600 ~ 699

#define MSGID_LOAD_STATISTICS_REPORT		100
#define MSGID_FAULT_STATISTICS_REPORT	   	101
#define MSGID_LEG_STATISTICS_REPORT			114
#define MSGID_LOGON_STATISTICS_REPORT		115
#define MSGID_CALL_REPORT                   116

#if 0
#define MSGID_IPAF_STATISTICS_REPORT			104
#define MSGID_AAA_STATISTICS_REPORT		 	105
#define MSGID_UAWAP_STATISTICS_REPORT	   	106
#define MSGID_SVC_TR_STATISTICS_REPORT	  	107
#define MSGID_RADIUS_STATISTICS_REPORT	  	108
#define MSGID_SVC_TTR_STATISTICS_REPORT	 	109
#define MSGID_UDR_STATISTICS_REPORT				110
#define MSGID_CDR_STATISTICS_REPORT		 	111
#define MSGID_UDR_SVCTYPE_STATISTICS_REPORT	 112
#define MSGID_VT_STATISTICS_REPORT				113
#define MSGID_CDR2_STATISTICS_REPORT			114
#define MSGID_FAIL_UDR_STATISTICS_REPORT		115

#define MSGID_OB_STATISTICS_REPORT		  110   
#define MSGID_WISE_STATISTICS_REPORT		111   
#define MSGID_SCIB_STATISTICS_REPORT		112   
#define MSGID_SCPIF_STATISTICS_REPORT	   113   
#define MSGID_RCIF_STATISTICS_REPORT		114   
#define MSGID_DB_STATISTICS_REPORT		  115
#define MSGID_OBC_STATISTICS_REPORT		 116 
#endif

#define MSGID_ALTIBASELOG_REQUEST			210
#define MSGID_ALTIBASELOG_RESPONSE			211

#define MSGID_SYS_COMM_STATUS_REPORT		300
#define	MSGID_SYS_SPEC_CONN_STATUS_REPORT	301
#define MSGID_SYS_SPEC_HW_STATUS_REPORT	 302
#define MSGID_SYS_SPEC_NMDB_STATUS_REPORT   303
#define MSGID_SYS_L3PD_STATUS_REPORT		304
#if 1 /* by june */
#define MSGID_SYS_SCE_STATUS_REPORT			305
#define MSGID_SYS_L2_STATUS_REPORT			306
#define MSGID_SYS_SCEFLOW_STATUS_REPORT     307
#endif

#define	MSGID_WATCHDOG_STATUS_REPORT		310
#define MSGID_SYS_TCPCON_STATUS_REPORT		320

typedef struct {
	unsigned char	pres;
	unsigned char	octet;
} SingleOctetType;

typedef struct {
	unsigned char	pres;
#define MAX_OCTET_STRING_LEN	32
	unsigned char	octet[MAX_OCTET_STRING_LEN];
} OctetStringType;


typedef struct {
	long		mtype;
#define MAX_GEN_QMSG_LEN		8192-(sizeof(long))
	char		body[MAX_GEN_QMSG_LEN];
} GeneralQMsgType;


typedef struct {
	int			msgId;   		// message_id short->int 2004-07-08		
	char		segFlag; 		// segment flag -> 한 메시지가 너무 길어 한번에 보내지 못할때 사용된다.
	char		seqNo;   		// sequence number -> segment된 경우 일련번호
	char		dummy[2];		// for alignment 
#define	BYTE_ORDER_TAG	0x1234
	short		byteOrderFlag;
	short		bodyLen;
	char		srcSysName[COMM_MAX_NAME_LEN];
	char		srcAppName[COMM_MAX_NAME_LEN];
	char		dstSysName[COMM_MAX_NAME_LEN];
	char		dstAppName[COMM_MAX_NAME_LEN];
} IxpcQMsgHeadType;

typedef struct {
	IxpcQMsgHeadType	head;
#define MAX_IXPC_QMSG_LEN	(MAX_GEN_QMSG_LEN)-sizeof(IxpcQMsgHeadType)
	char				body[MAX_IXPC_QMSG_LEN];
} IxpcQMsgType;

#define MML_MAX_CMD_NAME_LEN	32
#define MML_MAX_PARA_CNT		50
#define MML_MAX_PARA_VALUE_LEN  32

// MMCD가 Application으로 보내는 명령어 처리 요구 메시지
//
typedef struct {
	char	paraName[COMM_MAX_NAME_LEN];
	char	paraVal[COMM_MAX_VALUE_LEN];
} CommPara;

typedef struct {
	unsigned short	mmcdJobNo; // mmcd에서 관리하는 key값 -> result에 반드시 동일한 값을 주어야 한다.
	unsigned short	paraCnt; // 입력된 parameter의 갯수 
	char			cmdName[MML_MAX_CMD_NAME_LEN]; // command name
	CommPara		para[MML_MAX_PARA_CNT];
// appplication으로 파라미터 이름 전달
// 파라미터 value는 string으로 들어간다. ex) 10진수 123은 string "123"으로 들어간다.
// 파라미터는 command file에 등록된 순서대로 차례로 들어간다.
// 입력되지 않은 optional 파라미터에는 NULL이 들어간다.
} MMLReqMsgHeadType;

typedef struct {
	MMLReqMsgHeadType	head;
} MMLReqMsgType;


// Application이 MMCD로 보내는 명령어 처리 결과 메시지
//
typedef struct {
	unsigned short	mmcdJobNo;  // mmcd에서 관리하는 key값 -> result에 반드시 동일한 값을 주어야 한다.
	unsigned short	extendTime; // not_last인 경우 다음 메시지까지 timer 연장시간(초) -> mmcd에서 extendTime 시간만큼 timer를 연장시킨다.
	char			resCode;	// 명령어 처리 결과 -> success(0), fail(-1)
	char			contFlag;   // 마지막 메시지 여부 표시 -> last(0), not_last(1)
	char			cmdName[MML_MAX_CMD_NAME_LEN]; // command name
} MMLResMsgHeadType;

typedef struct {
	MMLResMsgHeadType	head;
#define MAX_MML_RESULT_LEN	((MAX_GEN_QMSG_LEN)-sizeof(IxpcQMsgHeadType))-sizeof(MMLResMsgHeadType)
	char				body[MAX_MML_RESULT_LEN];
} MMLResMsgType;


// MMCD가  RMI, GUI-MMC 등 client로부터 수신하는 명령어 처리 요구 메시지
// 

typedef struct {
	int		cliReqId; // client에서 할당한 key값 -> client에서 할당해서 보내고, MMCD는 결과 전송시 수신한 값을 그대로 돌려준다.
	int 	confirm;  /* (-1)=X, (0)=NO, (1)=YES */ 
	int		batchFlag;
	int		clientType; // GUI(1), RMI(0), OMDMMC(2) 
} MMLClientReqMsgHeadType;

#pragma pack(1)
typedef struct {
	MMLClientReqMsgHeadType	head;
	char					body[4000];
} MMLClientReqMsgType;
#pragma pack()

// MMCD가  RMI, GUI-MMC 등 client로 보내는 명령어 처리 결과 메시지
// 
typedef struct {
	int		cliReqId; // client에서 보낸 key값 -> client에서 할당해서 보내고, MMCD는 수신한 값을 그대로 돌려준다.
	int		confirm;
	int		batchFlag;
	int	 errCode;  /* login error code */
	char	resCode;  // 명령어 처리 결과 -> success(0), fail(-1)
	char	contFlag; // 마지막 메시지 여부 표시 -> last(0), not_last(1)
	char	segFlag;  // segment flag -> 한 메시지가 너무 길어 한번에 보내지 못할때 사용된다.
	char	seqNo;	// sequence number -> segment된 경우 일련번호
} MMLClientResMsgHeadType;

#pragma pack(1)
typedef struct {
	MMLClientResMsgHeadType	head;
	char					body[1024*4];
} MMLClientResMsgType;
#pragma pack()

typedef struct {
	char	processName[COMM_MAX_NAME_LEN];
	int	 Pid;
	int 	type; //0:kill-prc, 1:start-prc 
} IFB_KillPrcNotiMsgType; /* killsys시 사용되는 데이타 구조 */

/**  Duplication Status Response Message Type : SDMD (2006.07.23) **/
typedef struct {
	char		dup_status;		 /* 1 : ACTIVE, 2 : STANDBY, 3 : UNKNOWN */
	char		response_result;
	char		reserved[2];
	time_t	  dup_update_time;	/* duplication status update time */
} dup_status_res;

/* By june
	* RLEG Statistic structure 
	 * Use RLEG, STMP Block
	  */
/*
#define SYS_NAME_LEN	8
typedef struct _st_leg_statistic_ {
	unsigned int	   uiRadStartCnt;
	unsigned int	   uiRadInterimCnt;
	unsigned int	   uiRadStopCnt;
	unsigned int	   uiLoginCnt;
	unsigned int	   uiLoginSuccCnt;
	unsigned int	   uiLogoutCnt;
	unsigned int	   uiLogoutSuccCnt;
	char			szSysName[SYS_NAME_LEN];
} LEG_STAT;

#define DEF_PDSN_CNT	32
typedef struct _st_statistic_ {
	long long	   llRadStartCnt;
	long long	   llRadInterimCnt;
	long long	   llRadStopCnt;
	long long	   llLoginCnt;
	long long	   llLoginSuccCnt;
	long long	   llLogoutCnt;
	long long	   llLogoutSuccCnt;

} LEG_STATISTIC;

typedef struct _st_rleg_stat_ {
	unsigned int	thread_id;

	unsigned int	uiRecvCnt;
	unsigned int	uiFailCnt;
	LEG_STAT		stat;
} RLEG_STAT;
*/
/* 2009.06.10 by dhkim */
typedef struct __st_leg_pdsn_statistic_ {
	unsigned int	uiPDSN_IP;
	unsigned int	uiPDSN_RecvCnt;		// PDSN 별 ACCOUNT-REQUEST를 받은 개수
	unsigned int	uiPDSN_StartCnt;	// PDSN 별 ACCOUNT-REQUEST START를 받은 개수 
	unsigned int	uiPDSN_InterimCnt;	// PDSN 별 ACCOUNT-REQUEST INTERIM를 받은 개수
	unsigned int	uiPDSN_StopCnt;		// PDSN 별 ACCOUNT-REQUEST STOP를 받은 개수
	unsigned int	uiPDSN_DiscReqCnt; 	// PDSN 별 DISCONNECT-REQUEST를 받은 개수, 2010.09.29 DISCONNECT
	unsigned int	uiLogOn_StartCnt;	// PDSN 별 ACCOUNT-REQUEST (START)로 인한 LOGON 시도수
	unsigned int	uiLogOn_InterimCnt;	// PDSN 별 ACCOUNT-REQUEST (INTERIM)로 인한 LOGON 시도수
	unsigned int	uiLogOn_StopCnt;	// PDSN 별 ACCOUNT-REQUEST (STOP)로 인한 LOGOUT 시도수
	unsigned int	uiLogOn_DiscReqCnt; // PDSN 별 DISCONNECT-REQUEST로 인한 LOGON 시도수, 2010.09.29 DISCONNECT
} LEG_PDSN_STAT;

/** changed new struct, by uamyd 20110424 
typedef struct __st_leg_statistic_ {
	int			 nCount;
#define LEG_STAT_SYSNAME		8
	char			szSysName[LEG_STAT_SYSNAME];
#ifndef DEF_PDSN_CNT
#define DEF_PDSN_CNT	32
#endif
	LEG_PDSN_STAT   stPDSNStat[DEF_PDSN_CNT];
} LEG_STAT;
*/

typedef struct __st_leg_statistic_ {
	unsigned int	uiCount;
#ifndef DEF_PDSN_CNT
#define DEF_PDSN_CNT	32
#endif
	LEG_PDSN_STAT   stPDSNStat[DEF_PDSN_CNT];
	
} LEG_STAT, *PLEG_STAT;

#define DEF_HBIT_CNT	32
typedef struct _st_logon_statistic_ {
    unsigned int    uiSMIndex;          // RLEG 구분자 . [0, 1, 2, 3, 4]
    unsigned int    uiLogMode;          // LogOn 인지 LogOut 인지 구분하는 0 : On, 1 : Out
	unsigned int	uiLogOn_Request;	// SM에 대한 LOGON/LOGOUT 요청수의 합
	unsigned int	uiLogOn_Success;	// SM에 대한 LOGON/LOGOUT 요청 성공수의 합
	unsigned int	uiLogOn_Fail;		// SM에 대한 LOGON/LOGOUT 요청 실패수의 합
	unsigned int	uiLogOn_Reason1;	// SM internal error 밟생수
	unsigned int	uiLogOn_Reason2;	// SM internal error 외에 error 발생수 
	unsigned int	uiLogOn_Reason3;	// SM으로 요청한 LOGON/LOGOUT에 대한 timeout 발생 수
	unsigned int	uiLogOn_Reason4;	// 기타 LOGON/LOGOUT 처리 오류 발생수
	unsigned int	uiLogOn_APIReqErr;	// SM 처리시, -1을 return 하는 경우
	unsigned int	uiLogOn_APITimeout;	// SM 처리시, API에서 timeout이 발생하는 경우
	unsigned int	uiLogOn_HBIT[DEF_HBIT_CNT];
                                        // P BIT=1 & H BIT=0인 가입자에 대한 LOGON 성공수.
                                        // ADD, BY JUNE, 2010-08-22, HBIT 변경(1,2,3,etc -> 32)
} LOGON_STAT, *PLOGON_STAT;

typedef struct st_leg_total_statistic_ {
#define LEG_STAT_SYSNAME 8
	char			szSysName[LEG_STAT_SYSNAME];
	LEG_STAT		stAcct;
#define LOG_MOD_CNT  2
//#define MAX_RLEG_CNT 5
#define MAX_RLEG_CNT 1 // added by dcham 20110530 for SM connection 축소(5=>1)
	LOGON_STAT		stLogon[MAX_RLEG_CNT][LOG_MOD_CNT];
} LEG_TOT_STAT, *PLEG_TOT_STAT;

/* fimd->nmsif alarm information : 2006.08.17 by sdlee
*/
typedef struct {
	char	sysname[LEG_STAT_SYSNAME];	// DSCA, DSCB, DSCM, PD_A, PD_B
	int		atype;				// SFM_ALM_TYPE_CPU_USAGE, SFM_ALM_TYPE_MEMORY_USAGE, ...
	int		aclass;				// MINOR, MAJOR, CRITICAL
	char	desc[64];			// alarm description in current_alarm
	int		time;				// alarm occurred time
} NmsAlmInfo;

typedef struct {
#define	MAX_FLOW_IDX	3
	char			szSysName[8];	// SCEA, SCEB
	time_t			tGetTime;		// Getting time of SCE_FLOW 
	unsigned int	uiFlowNum;		// FLOW1, FLOW2, FLOW3
} SCE_FLOW_INFO;

#endif /*__COMM_MSGTYPES_H__*/
