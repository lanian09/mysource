#ifndef __MSGDEF_H__
#define __MSGDEF_H__

#include <time.h>

/*
	서버내 message queue 사용시 필요한 각종 정보들
*/
#define MAX_MSGBODY_SIZE 6144
//#define MAX_MSGBODY_SIZE		12288

typedef struct _st_MsgQ {
	 long int 			llMType;							/* Message Type */
	 int 				uiReserved;

	 long long int		llNID;							 /* Message Unique ID */

	 unsigned char		 ucNTAFID;						 /* IPAF PAIR ID if ucNaType = 2, Used szIPAFID[1] */
	 unsigned char		 ucProID;							/* Process ID : Example = SEQ_PROC_SESSIF */
	 unsigned char		 ucNTAMID;						 /* IPAF PAIR ID if ucNaType = 2, Used szIPAFID[1] */
	 char 				szReserved[5];					/* Call Reference Number */

	 long long int		 llIndex;							/* Local DB Index Number */

	 int					dMsgQID;							/* Source Queue ID */
	 unsigned short int	usBodyLen;						/* Receive Message Body Length */
	 unsigned short int	usRetCode;						/* RetCode */

	 unsigned char		 szBody[MAX_MSGBODY_SIZE];	/* Packet Message : 6144*/
} st_MsgQ, *pst_MsgQ;

#define DEF_MSGQ_SIZE		 sizeof(st_MsgQ)
#define DEF_MSGHEAD_LEN		(DEF_MSGQ_SIZE - MAX_MSGBODY_SIZE)

typedef struct _st_MsgQSub
{
	 unsigned int		usType;			/* 5000 : SYS, 1000 : SVC */
	 unsigned short	usSvcID;			/* SERVICE ID */
	 unsigned short	usMsgID;			/* MESSAGE ID */
} st_MsgQSub, *pst_MsgQSub;

/*** FOR GIFO */
#define DEF_MSGQ_NUM		101		/* st_MsgQ를 위한 gifo number */

/*
* MSG 분기를 위한 SID, MID 값 목록 
*/
#define DEF_SVC				1000
#define DEF_SYS				5000

/** SID : 중분류를 위한 SID */
#define SID_STATUS 				1
#define SID_STATUS_DIRECT		2
#define SID_STATUS_SWITCH		3
#define SID_SVC					6

#define SID_SESS_MANG			9
#define SID_SESS_INFO			10

#define SID_NTAF_MNG		11
#define SID_SERV_MNG		31

#define SID_TCP_INFO			20
#define SID_DATA_INFO			30
#define SID_CHECK_MSG			40
#define SID_LOG					50

/** S_MNG 에서 사용하는....*/
#define SID_FLT					60
#define SID_GFLT				61
#define SID_CHKRES				62
#define SID_CHKREQ				63

/** 명령어 처리시 사용되는 MID */
#define SID_MML					100
#define SID_PATCH				103		/* DQMS에는 100으로 되어있음 */

#define SID_NTAF_INFO			106
#define SID_VER					110
#define SID_NTAF_CHK			120

#define SID_COR					110		/* 중복 */
#define SID_HEARTBEAT			120		/* 중복 */


/** MID : 소분류를 위한 MID */
#define MID_ALARM				1
#define MID_CONSOL				2
#define MID_STAT				3
#define MID_TRAFFIC				4
//#define MID_CHG_NTAF          5 //밑에 있음

/*System Message ID Definition*/
#define MID_STATUS_ALMD			1	/* 중복 */
#define MID_STATUS_COND			2	/* 중복 */
#define MID_STATUS_STAT			3	/* 중복 */
#define MID_STATUS_LINK			4	/* 중복 */

#define MID_VER_STATUS			1	/* 중복 */
#define MID_VER_GET_PATCH		2	/* 중복 */
#define MID_VER_UPDATE_PATCH	3	/* 중복 */
#define MID_VER_ROLLBACK_PATCH	4	/* 중복 */
#define MID_VER_REBOOT			5	/* 중복 */

/* 20040419, sunny : http ipaf sessana & svc block stat */
#define MID_STATUS_STAT_SESS	5	/* 확인 필요 */
#define MID_STATUS_STAT_SVC		6	/* 확인 필요 */
#define MID_STATUS_STAT_NEWSVC	7	/* 확인 필요 */

#define MID_CHG_IPPOOL			1	/* 확인 필요 */
#define MID_CHG_SERGRP			2	/* 확인 필요 */
#define MID_CHG_NTAF			3	/* 확인 필요 */
#define MID_CHG_PROC_PARM		4	/* 확인 필요 */

#define MID_LINK_NTAF			1	/* 확인 필요 */

#define MID_NTAF_CHK_SVC		1	/* 확인 필요 */
#define MID_NTAF_CHK_IPPOOL 	2	/* 확인 필요 */

/* DEFINITION : PARTIAL PKG */
#define MID_VER_PAR_DOWN		10	/* 중복 */
#define MID_VER_PAR_PATCH		11	/* 중복 */
#define MID_VER_PAR_ROLLBACK	12	/* 중복 */

#define MID_SVC_CALLSTOP		10
#define MID_SVC_MONITOR			11
#define MID_SVC_STATCP			12
#define MID_SVC_MONITOR_SG		13

#define MID_SESS_START			11
#define MID_SESS_STOP			12
#define MID_SESS_LOG			13
#define MID_SESS_UPDATE			14
#define MID_SESS_DATA			15

#define MID_STAT_LOAD			14
#define MID_STAT_FAULT			15
#define MID_CHG_TIME			16		/* 중복 */
#define MID_STAT_TRAFFIC		16

#define MID_SVC_MONITOR_KPI		17		/*	KPI 서비스 메시지 추가	*/
#define MID_SVC_MONITOR_INF		18 

#define MID_SVC_TOTAL_MON		19
#define MID_SVC_MONITOR_1MIN	20

/* 2008.01.12 FOR GRX */
#define MID_GRX_START			16
#define MID_GRX_STOP			17
#define MID_GRX_LOG				18
#define MID_GRX_UPDATE			19

#define MID_SERV_GRUP			21	/* 확인 필요 */

/* 2011.08.25 FOR TCP SID (20) */
#define MID_TCP_START			21
#define MID_TCP_STOP			22
#define MID_TCP_DATA			23
#define MID_TCP_RETR_DATA		24
#define MID_TCP_NOACK_DATA		25
#define MID_TCP_UDP_DATA		26

#define MID_IP_POOL				30	/* 확인 필요 */
/* 2011.08.25 FOR TCP DATA (30) */
#define MID_DATA_TCP			31
#define MID_DATA_MMS			33
#define MID_DATA_PAGE			34
#define MID_DATA_VOD			35
#define MID_DATA_SIGNAL			36
#define MID_DATA_GN				37
#define MID_DATA_IUPS			38
#define MID_DATA_IUPS_SIGNAL	39
#define MID_DATA_IUPS_SIGNAL_RAB	40
#define MID_DATA_DNS			41
#define MID_DATA_AAA			42
#define MID_DATA_IPAS			43
#define MID_DATA_DIAMETER		44

/* 20040505,poopee */
#define MID_CHG_IPC_RTE			40	/* 확인 필요 */
#define MID_CHG_IPC_INFO		41	/* 확인 필요 */
#define MID_CHG_IPC_TMR			42	/* 확인 필요 */

/* 20040511, sunny */
#define MID_ADD_IPC_INFO		43	/* 확인 필요 */
#define MID_DEL_IPC_INFO		44	/* 확인 필요 */

/* 2011.08.25 FOR MSG (40) */
#define MID_SOCK_CHECK			41

/* 2011.08.25 FOR LOG (50) */
#define MID_LOG_TCP				51
#define MID_LOG_HTTP			52
#define MID_LOG_PAGE			53
#define MID_LOG_VOD				54
#define MID_LOG_LMS				55
#define MID_LOG_SIGNAL			56
#define MID_LOG_GN				57
#define MID_LOG_IUPS			58
#define MID_LOG_IUPS_SIGNAL		59
#define MID_LOG_IUPS_SIGNAL_RAB	60
#define MID_LOG_MEDIA			61
#define MID_LOG_GNGI			62
#define MID_LOG_CALL			70
#define MID_LOG_HTTPHDR			63
#define MID_LOG_TRACE			64
#define MID_LOG_TCPHDR			65
#define MID_LOG_IUPS_PAGING		66
#define MID_LOG_IUPS_CF			67	/* FOR LOG_IUPS_CF_%Day */

#define MID_LOG_CALL			70	/* FOR CALL LOG */
#define MID_LOG_IUPS_CALL		71	/* FOR IUPS CALL LOG */

#define MID_LOG_FTP				80
#define MID_LOG_SMS				90

#define MID_LOG_COREA			91
#define MID_LOG_CAP				92
#define MID_LOG_TCM				93
#define MID_LOG_BSSAP			94

/* 2008.01.12 FOR IMS */
#define MID_LOG_SIP				95
#define MID_LOG_MSRP			96
#define MID_LOG_XCAP			97

/* 2008.01.12 FOR GRX */
#define MID_LOG_GRX				101
#define MID_LOG_GSIGNAL			102
#define MID_LOG_GTCP			103

/* 2008.02.16 FOR INTERNET */
#define MID_LOG_INET			104
#define MID_LOG_IPAS			105
#define MID_LOG_AAA				106

/* 2009.04.17 FOR DIAMETER */
#define MID_LOG_DIAMETER		107

/* 2011.08.25 FOR FLT MSG (60) */
#define MID_FLT_TMF				61
#define MID_FLT_DEST			62
#define MID_FLT_ALM		 		63
#define MID_FLT_COMMON			64
#define MID_FLT_LOG		 		65
#define MID_FLT_ALL		 		66
#define MID_FLT_MNIP			67
#define MID_FLT_SVC		 		68
#define MID_FLT_USER			69
#define MID_FLT_THRES			70
#define MID_FLT_SNA		 		71
#define MID_FLT_MS				72
#define MID_FLT_SYS		 		73
#define MID_FLT_TCP				74
#define MID_FLT_TRC				75
#define MID_DIS_NTAF_CONF		76
#define MID_FLT_MSC				77
#define MID_FLT_EQUIP			78

#define MID_FLT_SVRTRC			80
#define MID_FLT_SCTP			81
#define MID_FLT_MON_THRES		83
#define MID_FLT_TIMER			85
#define MID_FLT_IRM				86
#define MID_FLT_ONOFF			87
#define MID_FLT_MODEL			88
#define MID_FLT_ACCESS			89
#define MID_FLT_DEFECT_THRES	90

#define MID_TRC					100	/* 확인 필요 */

#define MID_MML_RST				101
#define MID_MML_REQ				102
#define MID_MML_ACK				104
#define MID_MML_CHK				105

#define MID_MML_CHL				5	/* 확인 필요 */

/** FOR NMS */
#define MID_LOG_CONSOLE			109
#define MID_LOG_ALARM			110
#define MID_LOG_STATISTICS		111

/* 2011.08.25 FOR COR (110) */
#define MID_COR_START			20
#define MID_COR_STOP			21

/* 2011.8.25 FOR HEARTBEAT (120) */
#define MID_HEARTBEAT_REQ		1
#define MID_HEARTBEAT_RES		2

#define MID_COR_START			20
#define MID_COR_STOP			21

#define MID_NTAM_DOWN			101
#define MID_NTAM_PATCH			102
#define MID_NTAM_ROLLBACK		103
#define MID_NTAF_DOWN			104
#define MID_NTAF_PATCH			105
#define MID_NTAF_ROLLBACK		106
#define MID_SMNG_START			107

/* 추후 위치 변경 */
enum {
	IDX_LOAD_STATISTICS = 0,
	IDX_FAULT_STATISTICS,
	IDX_TRAFFIC_STATISTICS,
	IDX_MAXIMUM_STATISTICS
};

enum {
	STAT_PERIOD_5MIN = 0,
	STAT_PERIOD_HOUR,
	STAT_PERIOD_MAX
};

typedef struct _st_atQueryInfo
{
	char	cPeriod;		/*	STAT_PERIOD_5MIN, STAT_PERIOD_HOUR */
	char	cReserved[7];
	time_t	tStartTime;		/*	START TIME */
	time_t	tEndTime;		/*	END TIME */
} st_atQueryInfo;

#define DEF_ATQUERYINFO_SIZE	sizeof(st_atQueryInfo)

#endif /* __MSGDEF_H__ */

