#ifndef __COMM_MSGTYPES_H__
#define __COMM_MSGTYPES_H__

#include <time.h>
#include <define.h>

#pragma pack(1)
//------------------------------------------------------------------------------
// ��� �ý��ۿ��� �������� ���Ǵ� �޽����� ���� ������ �����Ѵ�.
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
#define MTYPE_STAT_REPORT_SHORT_TERM	10
#define MTYPE_STAT_REPORT_LONG_TERM	 11
#define MTYPE_MMC_SYNTAX_RESPONSE	   12
#define	MTYPE_DB_DISCONNECT				13
#define MTYPE_TRC_CONSOLE				14
#define MTYPE_MAP_NOTIFICATION			15

#define MTYPE_BSD_ALARM_REPORT			16

/* NEW MTYPE ADDED FOR UAWAP TRANSLOG (061219) */
#define MTYPE_UAWAP_TRANSLOG			18

/* NO TRANSMITTED UDR FILE ACT/DACT */
#define MTYPE_TRANINIT_REQUEST			19
#define MTYPE_NO_TRANSMITTED_ACT		20
#define MTYPE_NO_TRANSMITTED_DACT	   21

#define MTYPE_QUEUE_CLEAR_REPORT		22

/* NEW MTYPE for UDRGEN/UDRCOL */
#define MTYPE_UDR_GET						   31
#define MTYPE_UDR_GET_RESP					  32

/* NEW MTYPE for configuration of MMCR and MP Block */
#define MTYPE_BSD_CONFIG						41
#define MTYPE_TRC_CONFIG						42
#define MTYPE_PDSN_CONFIG					   43
#define MTYPE_TIMER_CONFIG						44
/* NEW MYPE for RADIUS TRANSMIT */
#define MTYPE_RADIUS_TRANSMIT					45

/* SET_SM_BUF_CLR */
#define MTYPE_SM_BUF_CLR						46
#define MTYPE_CALL_BACK							47


/* SDMD Duplication Status Request/Respose */
#define MTYPE_DUP_STATUS_REQUEST				50
#define MTYPE_DUP_STATUS_RESPONSE				51
#define MTYPE_DUP_UPDATE_NOTI					52

#define MTYPE_BLOCK_REPORT						60	
#define MTYPE_TRANS_REPORT						61	


// msg_id ���� �κ�
// msg_id range : MTYPE_STATISTICS_REQUEST	  : 1 ~ 99
//				MTYPE_STATISTICS_REPORT	   : 100 ~ 199
//				MTYPE_STATUS_REQUEST		  : 200 ~ 299
//				MTYPE_STATUS_REPORT		   : 300 ~ 399
//				MTYPE_ALARM_REQUEST		   : 400 ~ 499
//				MTYPE_ALARM_REPORT			: 500 ~ 599
//				MTYPE_MAINTENANCE_NOTIFICATION: 600 ~ 699

#define MSGID_LOAD_STATISTICS_REPORT			100
#define MSGID_FAULT_STATISTICS_REPORT			101
#define MSGID_IPAF_STATISTICS_REPORT			104
#define MSGID_AAA_STATISTICS_REPORT			 	105
#define MSGID_UAWAP_STATISTICS_REPORT		   	106
#define MSGID_SVC_TR_STATISTICS_REPORT		  	107
#define MSGID_RADIUS_STATISTICS_REPORT		  	108
#define MSGID_SVC_TTR_STATISTICS_REPORT		 	109
#define MSGID_UDR_STATISTICS_REPORT			 	110
#define MSGID_CDR_STATISTICS_REPORT			 	111
#define MSGID_UDR_SVCTYPE_STATISTICS_REPORT	 	112
#define MSGID_VT_STATISTICS_REPORT			  	113
#define MSGID_LEG_STATISTICS_REPORT				114
#define MSGID_LOGON_STATISTICS_REPORT			115
#define MSGID_CALL_REPORT						116

#if 0
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
	int		iIsCount;
	int		offset;
	char	szMin[MAX_MIN_LEN];
} InnerGenTrcMsgHead;

typedef struct {
	InnerGenTrcMsgHead head;
#define MAX_GENTRC_QMSG_LEN	(MAX_GEN_QMSG_LEN)- sizeof( InnerGenTrcMsgHead )
	char	data[MAX_GENTRC_QMSG_LEN];
} InnerGenTrcMsg;



typedef struct {
	int			msgId;   		// message_id short->int 2004-07-08		
	char		segFlag; 		// segment flag -> �� �޽����� �ʹ� ��� �ѹ��� ������ ���Ҷ� ���ȴ�.
	char		seqNo;   		// sequence number -> segment�� ��� �Ϸù�ȣ
	char		dummy[2];		// for alignment 
#define	BYTE_ORDER_TAG	0x1234
	short		byteOrderFlag;
	short		bodyLen;
	char		srcSysName[COMM_MAX_NAME_LEN];
	char		srcAppName[COMM_MAX_NAME_LEN];
	char		dstSysName[COMM_MAX_NAME_LEN];
	char		dstAppName[COMM_MAX_NAME_LEN];
} IxpcQMsgHeadType;
#define MAX_IXPC_QMSG_HEAD_LEN	sizeof(IxpcQMsgHeadType)

typedef struct {
	IxpcQMsgHeadType	head;
#define MAX_IXPC_QMSG_LEN	(MAX_GEN_QMSG_LEN)-sizeof(IxpcQMsgHeadType)
	char				body[MAX_IXPC_QMSG_LEN];
} IxpcQMsgType;

#define MML_MAX_CMD_NAME_LEN	32
#define MML_MAX_PARA_CNT		50
#define MML_MAX_PARA_VALUE_LEN  32

// MMCD�� Application���� ������ ��ɾ� ó�� �䱸 �޽���
//
typedef struct {
	char	paraName[COMM_MAX_NAME_LEN];
	char	paraVal[COMM_MAX_VALUE_LEN];
} CommPara;

typedef struct {
	unsigned short	mmcdJobNo; // mmcd���� �����ϴ� key�� -> result�� �ݵ�� ������ ���� �־�� �Ѵ�.
	unsigned short	paraCnt; // �Էµ� parameter�� ���� 
	char			cmdName[MML_MAX_CMD_NAME_LEN]; // command name
	CommPara		para[MML_MAX_PARA_CNT];
// appplication���� �Ķ���� �̸� ����
// �Ķ���� value�� string���� ����. ex) 10���� 123�� string "123"���� ����.
// �Ķ���ʹ� command file�� ��ϵ� ������� ���ʷ� ����.
// �Էµ��� ���� optional �Ķ���Ϳ��� NULL�� ����.
} MMLReqMsgHeadType;

typedef struct {
	MMLReqMsgHeadType	head;
} MMLReqMsgType;


// Application�� MMCD�� ������ ��ɾ� ó�� ��� �޽���
//
typedef struct {
	unsigned short	mmcdJobNo;  // mmcd���� �����ϴ� key�� -> result�� �ݵ�� ������ ���� �־�� �Ѵ�.
	unsigned short	extendTime; // not_last�� ��� ���� �޽������� timer ����ð�(��) -> mmcd���� extendTime �ð���ŭ timer�� �����Ų��.
	char			resCode;	// ��ɾ� ó�� ��� -> success(0), fail(-1)
	char			contFlag;   // ������ �޽��� ���� ǥ�� -> last(0), not_last(1)
	char			cmdName[MML_MAX_CMD_NAME_LEN]; // command name
} MMLResMsgHeadType;

typedef struct {
	MMLResMsgHeadType	head;
#define MAX_MML_RESULT_LEN	((MAX_GEN_QMSG_LEN)-sizeof(IxpcQMsgHeadType))-sizeof(MMLResMsgHeadType)
	char				body[MAX_MML_RESULT_LEN];
} MMLResMsgType;


// MMCD��  RMI, GUI-MMC �� client�κ��� �����ϴ� ��ɾ� ó�� �䱸 �޽���
// 

typedef struct {
	int		cliReqId; // client���� �Ҵ��� key�� -> client���� �Ҵ��ؼ� ������, MMCD�� ��� ���۽� ������ ���� �״�� �����ش�.
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

// MMCD��  RMI, GUI-MMC �� client�� ������ ��ɾ� ó�� ��� �޽���
// 
typedef struct {
	int		cliReqId; // client���� ���� key�� -> client���� �Ҵ��ؼ� ������, MMCD�� ������ ���� �״�� �����ش�.
	int		confirm;
	int		batchFlag;
	int	 errCode;  /* login error code */
	char	resCode;  // ��ɾ� ó�� ��� -> success(0), fail(-1)
	char	contFlag; // ������ �޽��� ���� ǥ�� -> last(0), not_last(1)
	char	segFlag;  // segment flag -> �� �޽����� �ʹ� ��� �ѹ��� ������ ���Ҷ� ���ȴ�.
	char	seqNo;	// sequence number -> segment�� ��� �Ϸù�ȣ
} MMLClientResMsgHeadType;

#pragma pack(1)
typedef struct {
	MMLClientResMsgHeadType	head;
	char					body[4000];
} MMLClientResMsgType;
#pragma pack()

typedef struct {
	char	processName[COMM_MAX_NAME_LEN];
	int	 Pid;
	int 	type; //0:kill-prc, 1:start-prc 
} IFB_KillPrcNotiMsgType; /* killsys�� ���Ǵ� ����Ÿ ���� */


/**  Duplication Status Response Message Type : SDMD (2006.07.23) **/
typedef struct {
	char		dup_status;			/* 1 : ACTIVE, 2 : STANDBY, 3 : UNKNOWN */
	char		response_result;	/* 0 : UNAVAILABLE, 1 : AVAILABLE */
	char		reserved[2];
	time_t		dup_update_time;	/* duplication status update time **/
} dup_status_res;


/* By june
 * RLEG Statistic structure 
 * Use RLEG, STMP Block
 */
#if 0
typedef struct _st_leg_statistic_ {
	long long	   llRadStartCnt;
	long long	   llRadInterimCnt;
	long long	   llRadStopCnt;
	long long	   llLoginCnt;
	long long	   llLoginSuccCnt;
	long long	   llLogoutCnt;
	long long	   llLogoutSuccCnt;

} LEG_STAT;
#endif

typedef struct __st_leg_statistic_1 {
#define	LEG_STAT_SYSNAME		8
	unsigned int	uiRadStartCnt;
	unsigned int	uiRadInterimCnt;
	unsigned int	uiRadStopCnt;
	unsigned int	uiLoginCnt;
	unsigned int	uiLoginSuccCnt;
	unsigned int	uiLogoutCnt;
	unsigned int 	uiLogoutSuccCnt;
	char			szSysName[LEG_STAT_SYSNAME];
} LEG_STAT1;

#if 0
typedef struct __st_leg_pdsn_statistic_ {
	unsigned int	uiPDSN_IP;
	unsigned int	uiPDSN_RecvCnt;
	unsigned int	uiPDSN_StartCnt;
	unsigned int	uiPDSN_InterimCnt;
	unsigned int	uiPDSN_StopCnt;
	unsigned int	uiLogOn_StartCnt;
	unsigned int	uiLogOn_InterimCnt;
	unsigned int	uiLogOn_StopCnt;
} LEG_PDSN_STAT;
#else
typedef struct __st_leg_pdsn_statistic_ {
	unsigned int	uiPDSN_IP;
	unsigned int	uiPDSN_RecvCnt;
	unsigned int	uiPDSN_StartCnt;
	unsigned int	uiPDSN_InterimCnt;
	unsigned int	uiPDSN_DiscReqCnt;		// ADD DISCONN_REQ
	unsigned int	uiPDSN_StopCnt;
	unsigned int	uiLogOn_StartCnt;
	unsigned int	uiLogOn_InterimCnt;
	unsigned int	uiLogOn_StopCnt;
	unsigned int	uiLogOn_DiscReqCnt; 	// ADD DISCONN_REQ
} LEG_PDSN_STAT;
#endif

typedef struct __st_leg_statistic_ {
	int				nCount;
#define LEG_STAT_SYSNAME		8
	char			szSysName[LEG_STAT_SYSNAME];
#ifndef	DEF_PDSN_CNT
#define DEF_PDSN_CNT	32
#endif
	LEG_PDSN_STAT	stPDSNStat[DEF_PDSN_CNT];
} LEG_STAT;

#if 0
typedef struct _st_logon_statistic_ {
	unsigned int	uiLogOn_Request;
	unsigned int	uiLogOn_Success;
	unsigned int	uiLogOn_Fail;
	unsigned int	uiLogOn_Reason1;
	unsigned int	uiLogOn_Reason2;
	unsigned int	uiLogOn_Reason3;
	unsigned int	uiLogOn_Reason4;
	unsigned int	uiLogOn_HBIT0;
	unsigned int	uiLogOn_HBIT1;
	unsigned int	uiLogOn_HBIT2;
	unsigned int	uiLogOn_HBIT3;
	unsigned int	uiLogOn_HBIT4;
	char			szSysName[LEG_STAT_SYSNAME];
} LOGON_STAT;
#endif

typedef struct _st_logon_statistic_ {
	unsigned int	uiLogOn_Request;
	unsigned int	uiLogOn_Success;
	unsigned int	uiLogOn_Fail;
	unsigned int	uiLogOn_Reason1;
	unsigned int	uiLogOn_Reason2;
	unsigned int	uiLogOn_Reason3;
	unsigned int	uiLogOn_Reason4;
	unsigned int    uiLogOn_APIReqErr;
	unsigned int    uiLogOn_APITimeout;
	unsigned int	uiLogOn_HBIT[32];
	char			szSysName[LEG_STAT_SYSNAME];
} LOGON_STAT;

#define NOTI_TRACE_TYPE	1 // TRACE_INFO.conf
#define NOTI_PDSN_TYPE	2 // PDSN.conf
#define NOTI_PB_TYPE	3 // RULESET_LIST.conf
#define NOTI_RULE_TYPE	4 // RULESET_USED.conf
#define NOTI_CPS_TYPE	5 // CALL_OVER_CTRL.conf
#define NOTI_TIME_TYPE	6 // TIMEOUT.conf
#define NOTI_SCE_TYPE	7 // sce-log-out ��ɾ� 
#define NOTI_ALL		8 // �ʱ�ȭ�� ALL �� 

#define DEF_MIN_LEN		16
#define DEF_IP_LEN		16
typedef struct _st_NOTI_ {
	int		dTrcIdx;
	int		dPdsnIdx;
	int		dPdsnCnt;
	int		dRsetListIdx;
	int		dRsetUsedIdx;
	int		dCpsIdx;
	int		dTimeIdx;
	char	szMIN[DEF_MIN_LEN];
	char	szIP[DEF_IP_LEN];
} st_NOTI, *pst_NOTI; 
#define DEF_NOTI_BODY_SIZE sizeof(st_NOTI)

typedef struct _NOTIFY_SIG_ {   
	int			dFltType;
	st_NOTI		stNoti;
} NOTIFY_SIG;   
#define DEF_NOTIFY_SIG_SIZE sizeof(NOTIFY_SIG)

////////////////////////////////////////////////////////////////////

#pragma pack(0)
#endif /*__COMM_MSGTYPES_H__*/
