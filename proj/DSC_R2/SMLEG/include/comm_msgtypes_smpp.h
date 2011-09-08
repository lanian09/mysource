//------------------------------------------------------------------------------
// file name : comm_msgtypes.h
// project name : LGT_SCP 
// ��� �ý��ۿ��� �������� ���Ǵ� �޽����� ���� ������ �����Ѵ�.
//------------------------------------------------------------------------------
#ifndef __COMM_MSGTYPES_H__
#define __COMM_MSGTYPES_H__

//INIP ONLY
#define COMM_MAX_IPCBUF_LEN		8192

//�Ϲ������� ���Ǵ� NAME�� ����
#define COMM_MAX_NAME_LEN	16


// mtype range : 1 ~ 127
//
#define MTYPE_SETPRINT					1
#define MTYPE_IXPC_CONNECTION_CHECK		2
#define MTYPE_MMC_REQUEST				3
#define MTYPE_MMC_RESPONSE				4
#define MTYPE_MMC_CANCEL				5
#define MTYPE_STATISTICS_REQUEST		6
#define MTYPE_STATISTICS_REPORT			7
#define MTYPE_STATUS_REQUEST			8
#define MTYPE_STATUS_REPORT				9
#define MTYPE_ALARM_REQUEST				10
#define MTYPE_ALARM_REPORT				11
#define MTYPE_MAINTENANCE_NOTIFICATION	12
#define MTYPE_STAT_REPORT_SHORT_TERM	13
#define MTYPE_STAT_REPORT_LONG_TERM		14
#define MTYPE_MMC_SYNTAX_RESPONSE		15
#define MTYPE_WINDOW_TRACE              16
#define MTYPE_TRACE_CONSOLE             17 

#define MTYPE_LAN_STATE							18

#define MTYPE_NMS_NOTI_REPORT			19

// msg_id ���� �κ�
// msg_id range : MTYPE_STATISTICS_REQUEST      : 1 ~ 99
//                MTYPE_STATISTICS_REPORT       : 100 ~ 199
//                MTYPE_STATUS_REQUEST          : 200 ~ 299
//                MTYPE_STATUS_REPORT           : 300 ~ 399
//                MTYPE_ALARM_REQUEST           : 400 ~ 499
//                MTYPE_ALARM_REPORT            : 500 ~ 599
//                MTYPE_MAINTENANCE_NOTIFICATION: 600 ~ 699
#define MSGID_LOAD_STATISTICS_REPORT			100
#define MSGID_FAULT_STATISTICS_REPORT			101
#define MSGID_IPI_STATISTICS_REPORT				102
#define MSGID_CDSI_STATISTICS_REPORT			103
#define MSGID_CPSI_STATISTICS_REPORT			104
#define MSGID_CPSI_PLUS_STATISTICS_REPORT		105
#define MSGID_SYNCI_STATISTICS_REPORT			106
#define MSGID_SYNCB_STATISTICS_REPORT			107



#define MSGID_SYS_COMM_STATUS_REPORT			300
#define MSGID_SYS_SPEC_STATUS_REPORT			301
#define MSGID_KILLPRC_STATUS_REPORT				302
#define MSGID_WATCHDOG_STATUS_REPORT			303

#define MSGID_SYS_ALMD_STATUS_REPORT			304


// 700���ʹ� 999������ �ֱ��� ��� �޽����� ������ ������ �����Ѵ�.
// ���� ��� ���� 2�ڸ��� ��� item�����̰� �� ���ڸ��� ��� �ֱ��̴�.
// ��� �ֱ�� 0 : 5minutely, 1 : hourly, 2 : daily, 3 : weekly, 4 : monthly, 5 : schedule
// ��� item��  70x : fault statistics
//				71x : load statistics
//				72x : ipi statistics
//				73x : cdsi statistics
//				74x : cpsi statistics
//				75x : cpsi_plus statistics
//				76x : synci statistics
//				77x : syncb statistics

#define MSGID_PERIODIC_FAULT_MINUTE		700 
#define MSGID_PERIODIC_FAULT_HOUR		701 
#define MSGID_PERIODIC_FAULT_DAY		702 
#define MSGID_PERIODIC_FAULT_WEEK		703 
#define MSGID_PERIODIC_FAULT_MONTH		704 
#define MSGID_SCHEDULE_FAULT			705 

#define MSGID_PERIODIC_LOAD_MINUTE		710 
#define MSGID_PERIODIC_LOAD_HOUR		711 
#define MSGID_PERIODIC_LOAD_DAY			712 
#define MSGID_PERIODIC_LOAD_WEEK		713 
#define MSGID_PERIODIC_LOAD_MONTH		714 
#define MSGID_SCHEDULE_LOAD				715 

#if 0 /* by june */
typedef struct {
	unsigned char	pres;
	unsigned char	octet;
} SingleOctetType;

//INIP ONLY ADD
typedef struct FourOctetType {
	unsigned char	len;        /* length of octets */
	unsigned char	ostring[4]; /* contents */
} FourOctetType;

typedef struct {
	unsigned char	len;
#define MAX_OCTET_STRING_LEN	64
	unsigned char	ostring[MAX_OCTET_STRING_LEN];
} OctetStringType;





typedef struct {
	short		msgId;   // message_id
	char		segFlag; // segment flag -> �� �޽����� �ʹ� ��� �ѹ��� ������ ���Ҷ� ���ȴ�.
	char		seqNo;   // sequence number -> segment�� ��� �Ϸù�ȣ
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
#define MAX_IXPC_QMSG_LEN	8096
	char				body[MAX_IXPC_QMSG_LEN];
} IxpcQMsgType;

typedef struct {
	long		mtype;
#define MAX_GEN_QMSG_LEN		(sizeof(IxpcQMsgType)-sizeof(long))
	char		body[MAX_GEN_QMSG_LEN];
} GeneralQMsgType;



#define MML_MAX_CMD_NAME_LEN	30
#define MML_MAX_LINK_CMD		15
#define MML_MAX_PARA_NAME_LEN	16
#define MML_MAX_PARA_VALUE_LEN	32
#define MML_MAX_PARA_CNT		32
#define MML_MAX_ENUM_ITEM		20
#define MML_MAX_ENUM_NAME_LEN	16
#define MML_MAX_MMC_HANDLER		30
// MMCD�� Application���� ������ ��ɾ� ó�� �䱸 �޽���
//
typedef struct {
	unsigned short	mmcdJobNo; // mmcd���� �����ϴ� key�� -> result�� �ݵ�� ������ ���� �־�� �Ѵ�.
	/**** 2007.07.04 added userName ****/
	char	userName[COMM_MAX_NAME_LEN]; // user name
	char	cmdName[MML_MAX_CMD_NAME_LEN]; // command name
	char	para[MML_MAX_PARA_CNT][MML_MAX_PARA_VALUE_LEN];
// appplication���� �Ķ���� �̸��� �������� �ʰ� value�� �����Ѵ�.
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
	char			resCode;    // ��ɾ� ó�� ��� -> success(0), fail(-1)
	char			contFlag;   // ������ �޽��� ���� ǥ�� -> last(0), not_last(1)
	char			cmdName[MML_MAX_CMD_NAME_LEN]; // command name
	/**** 2007.07.04 added userName ****/
	char	userName[COMM_MAX_NAME_LEN]; // user name
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
} MMLClientReqMsgHeadType;

typedef struct {
	MMLClientReqMsgHeadType	head;
	char					body[8000];	//INIP 4000 -> 8000
} MMLClientReqMsgType;

// MMCD��  RMI, GUI-MMC �� client�� ������ ��ɾ� ó�� ��� �޽���
// 
typedef struct {
	int		cliReqId; // client���� ���� key�� -> client���� �Ҵ��ؼ� ������, MMCD�� ������ ���� �״�� �����ش�.
	char	resCode;  // ��ɾ� ó�� ��� -> success(0), fail(-1)
	char	contFlag; // ������ �޽��� ���� ǥ�� -> last(0), not_last(1)
	char	segFlag;  // segment flag -> �� �޽����� �ʹ� ��� �ѹ��� ������ ���Ҷ� ���ȴ�.
	char	seqNo;    // sequence number -> segment�� ��� �Ϸù�ȣ
} MMLClientResMsgHeadType;

typedef struct {
	MMLClientResMsgHeadType	head;
	char					body[8000];	//INIP 4000 -> 8000
} MMLClientResMsgType;

typedef struct {
    char    processName[COMM_MAX_NAME_LEN];
    int     Pid;
} IFB_KillPrcNotiMsgType; /* killsys�� ���Ǵ� ����Ÿ ���� */

// INIP ONLY ADD
// MMC ��ɾ� ����Ʈ�� ó�� function�� ����ϴ� table structure
typedef struct {
	char	cmdName[24];
	int		(*func)(IxpcQMsgType*);
} MmcHdlrVector;

typedef struct {
	char sysType; //0 SCP, 1 SMP, 2 OMP
	char tableType; //0=5��, 1 = 1�ð�, 2 = �Ϸ�, 3 = 1��, 4 = �Ѵ�
	char date[30];  //��¥
}NMSMsgType;

/* fimd->nmsif alarm information : 2006.11.16 by sdlee
*/
typedef struct {
    char    sysname[8];         //
    char    almcode[8];
    int     time;
    int     aclass;             // MINOR, MAJOR, CRITICAL
	char	rscName[16];		// ��� �̸�
    char    desc[80];           // alarm description in current_alarm
} NmsAlmInfo;

#define MAX_SCP_COUNT 8
typedef struct {
        char scpID[MAX_SCP_COUNT];
        char bk_scp;
        char nscp[MAX_SCP_COUNT];
} Sys_ScpID;
#endif
#endif /*__COMM_MSGTYPES_H__*/
