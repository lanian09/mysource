#ifndef __SFMCONF_H__
#define __SFMCONF_H__

#include "sysconf.h"
#include "comm_msgtypes.h"

//------------------------------------------------------------------------------
// Status & Fault Management ���õ� ������ �����Ѵ�.
//------------------------------------------------------------------------------

#ifndef COMM_MAX_NAME_LEN
#define COMM_MAX_NAME_LEN	16	
#endif
#define QUE_MAX_NAME_LEN	10 // by helca 08.08
// �� Component�� �ִ� ���� ����
//
#define	SFM_MAX_CPU_CNT					16 // yhshin 4
#define	SFM_MAX_DISK_CNT				8
#define	SFM_MAX_LAN_CNT					SYSCONF_MAX_ASSO_SYS_NUM * 2
#define	SFM_MAX_RMT_LAN_CNT				20
#define	SFM_MAX_PROC_CNT				SYSCONF_MAX_APPL_NUM
#define	SFM_MAX_QUE_CNT					SYSCONF_MAX_APPL_NUM
#define	SFM_MAX_DEV_CNT					2
#define	SFM_MAX_DISK_NAME				COMM_MAX_NAME_LEN
#define	SFM_MAX_LAN_NAME_LEN			COMM_MAX_NAME_LEN
#define	SFM_MAX_TARGET_IP_LEN			16
#define SFM_MAX_HPUX_HW_COM				40
#define SFM_MAX_APP_NETMON				2
#define SFM_MAX_NET_MON_CNT				SFM_MAX_APP_NETMON * 3   // APP�� LAN ��
#define SFM_MAX_SUCC_RATE_CNT			8
#define SFM_REAL_SUCC_RATE_CNT			SFM_MAX_SUCC_RATE_CNT
#define SFM_MAX_DB_CNT					3
#define SFM_MAX_SESSION_CNT				4
#define MAX_HW_NTP						2
#define MAX_BOND_NUM                    4



/* Session load */
#define SESSION_LOADINDEX_UPRESTO		0 /* the others is reserved */


// ��� ������ �����ϱ� ���� ��
// - ���ǵ� ���� Ư���� �ǹ̸� ���� �ʰ�, �ܼ��� ���θ� �����ϴ� �뵵�θ� ���ǰ�
//  key���̳� index ���� �뵵�� ������ �ʴ´�.
// - alarm_history DB�� ��� �ִ� ������ ��ȸ�Ҷ� ���ȴ�.
#define SFM_ALM_TYPE_CPU_USAGE			1
#define SFM_ALM_TYPE_MEMORY_USAGE		2
#define SFM_ALM_TYPE_DISK_USAGE			3
#define SFM_ALM_TYPE_LAN				4
#define SFM_ALM_TYPE_PROC				5
#define SFM_ALM_TYPE_LINK				6
#define SFM_ALM_TYPE_MP_HW              7
#define SFM_ALM_TYPE_STAT				8
#define SFM_ALM_TYPE_DB_REP				9
#define SFM_ALM_TYPE_DB_REP_GAP			10
//#define SFM_ALM_TYPE_SMS_AUTO_CLR		10
//#define SFM_ALM_TYPE_SMS_PASS_CLR		11
#define SFM_ALM_TYPE_CONN_SERVER		11
#define SFM_ALM_TYPE_CALL_INFO			12

/* by helca */
#define SFM_ALM_TYPE_DUP_STS			13
#define SFM_ALM_TYPE_DUP_HEARTBEAT		14
#define SFM_ALM_TYPE_DUP_OOS			15
#define SFM_ALM_TYPE_SUCC_RATE			16
#define SFM_ALM_TYPE_SESS_LOAD			17
#define SFM_ALM_TYPE_DBCON_STST			18
#define SFM_ALM_TYPE_RMT_LAN			19
#define SFM_ALM_TYPE_OPT_LAN			20
#define SFM_ALM_TYPE_HWNTP				21
#define SFM_ALM_TYPE_PD_CPU_USAGE		22
#define SFM_ALM_TYPE_PD_MEMORY_USAGE	23
#define SFM_ALM_TYPE_PD_FAN_STS			24
#define SFM_ALM_TYPE_PD_GIGA_LAN		25
#define SFM_ALM_TYPE_RSRC_LOAD			26
#define SFM_ALM_TYPE_QUEUE_LOAD			27

#define SFM_ALM_OCCURED		1
#define SFM_ALM_CLEARED		0
#define SFM_ALM_REMOVED     2           // Tcp Connection ���°��� �� ���
                                        // IP�� ������ ��� ���� ���� ǥ�� ��


// ��� ���
//
#define	SFM_ALM_NORMAL		0
#define	SFM_ALM_MINOR		1
#define	SFM_ALM_MAJOR		2
#define	SFM_ALM_CRITICAL	3

#define SFM_ALM_UNMASKED    0
#define	SFM_ALM_MASKED		99

// ���μ��� �� �Ϲ��� ������Ʈ�� ����
//
#define SFM_STATUS_ALIVE	0
#define SFM_STATUS_DEAD		1

// LAN ����
//
#define SFM_LAN_CONNECTED		0
#define SFM_LAN_DISCONNECTED	1
#define SFM_LAN_NOT_EQUIP		4

#if 0
// HPUX H/W ����
//
#define SFM_HW_NOT_EQUIP     0   /* must not equip */
#define SFM_HW_ABSENT        1   /* must equip, but absent */
#define SFM_HW_DOWN          2   /* must equip, down state */
#define SFM_HW_ONLINE        3   /* must equip, online state */
#define SFM_HW_MIRROR        4   /* must equip, mirroring */
#define              HOTSTANDBY      0
#define              ACTIVE          1
#endif

// Macro related to Hardware status
// by helca 08.03
#define SFM_HW_UP		0
#define SFM_HW_DOWN		1
 
#define  SFM_HW_MAX_DISK_NUM     4
#define  SFM_HW_MAX_CPU_NUM      10 // 5 -> 10
#define  SFM_HW_MAX_LINK_NUM     16 /* sum : 16, link:8, mysql:1, timetens:1, sm:1,cm:1, smNetstat: 4*/
#define  SFM_HW_MAX_PWR_NUM      4 // 3 -> 4
#define  SFM_HW_MAX_FAN_NUM      6

#define SFM_DISK_ATTACHED      0
#define SFM_DISK_DETTACHED      1
// L3 PROBE DEVICE
//
//#define PD_FUNCTION_ON		0
//#define PD_FUNCTION_OFF		1
//#define	MAX_PD_FAN_NUM		4
//
//------------------------------------------------------------------------------
//
// CPU, Memory, Disk �� ��� ���,�ϰ� ����� �ִ� ��� ������ ���� structure
//
#pragma pack(1)
typedef struct {
	unsigned char	minFlag;  // minor ��� �߻��Ǹ� setting�ȴ�. ����� �ϰ��ϴ� ��� ���� �޽����� ������ �ϴ��� �����ϴµ� ����ȴ�.
	unsigned char	majFlag;  // major ��� �߻��Ǹ� setting�ȴ�. ����� �ϰ��ϴ� ��� ���� �޽����� ������ �ϴ��� �����ϴµ� ����ȴ�.
	unsigned char	criFlag;  // critical ��� �߻��Ǹ� setting�ȴ�. ����� �ϰ��ϴ� ��� ���� �޽����� ������ �ϴ��� �����ϴµ� ����ȴ�.
} SFM_HwAlmFlag_s;
#pragma pack()

#pragma pack(1)
typedef struct {
	SFM_HwAlmFlag_s		cpu[SFM_MAX_CPU_CNT];
	SFM_HwAlmFlag_s		mem;
	SFM_HwAlmFlag_s		disk[SFM_MAX_DISK_CNT];
	SFM_HwAlmFlag_s		db;
	SFM_HwAlmFlag_s		queLoad[SFM_MAX_QUE_CNT];
} SFM_HwAlmFlag;
#pragma pack()
//
// �� �ý��ۿ� ���� ���� �߻��Ǿ� �ִ� ��޺� ��� ������ ��ü ��� ���
//
#pragma pack(1)
typedef struct {
	unsigned char	level;     // �ش� �ý����� ��ü ��� ���
	unsigned char	prevLevel; // previous �ش� �ý����� ��ü ��� ���
	unsigned char	minCnt;    // ���� �߻��Ǿ� �ִ� minor ��� ����
	unsigned char	majCnt;    // ���� �߻��Ǿ� �ִ� major ��� ����
	unsigned char	criCnt;    // ���� �߻��Ǿ� �ִ� critical ��� ����
} SFM_SysAlmInfo;
#pragma pack()


//------------------------------------------------------------------------------

// CPU ���� ����
//
#pragma pack(1)
typedef struct {
	unsigned char	mask[SFM_MAX_CPU_CNT];
	unsigned short	usage[SFM_MAX_CPU_CNT];     // ���� CPU �����
	unsigned char	level[SFM_MAX_CPU_CNT];     // ���� CPU ��� ���
	unsigned char	minLimit;  // minor ��� �Ӱ谪 -> �� ���� �ʰ��Ͽ� duration �̻� ���ӵǸ� ��ַ� �����Ѵ�.
	unsigned char	majLimit;  // major ��� �Ӱ谪
	unsigned char	criLimit;  // critical ��� �Ӱ谪
	unsigned char	minDurat;  //
	unsigned char	majDurat;  //
	unsigned char	criDurat;  //
} SFM_CpuInfo;
#pragma pack()

// memory ���� ����
//
#pragma pack(1)
typedef struct {
	unsigned char	mask;
	unsigned short	usage;     // ���� memory �����
	unsigned char	level;     // ���� memory ��� ���
	unsigned char	minLimit;  // minor ��� �Ӱ谪 -> �� ���� �ʰ��Ͽ� duration �̻� ���ӵǸ� ��ַ� �����Ѵ�.
	unsigned char	majLimit;  // major ��� �Ӱ谪
	unsigned char	criLimit;  // critical ��� �Ӱ谪
	unsigned char	minDurat;  //
	unsigned char	majDurat;  //
	unsigned char	criDurat;  //
} SFM_MemInfo;
#pragma pack()

// disk ���� ����
//
#pragma pack(1)
typedef struct {
	unsigned char	mask;
	char			name[SFM_MAX_DISK_NAME]; // disk partition name
	unsigned short	usage;     // ���� �����
	unsigned char	level;     // ���� ��� ���
	unsigned char	minLimit;  // minor ��� �Ӱ谪
	unsigned char	majLimit;  // major ��� �Ӱ谪
	unsigned char	criLimit;  // critical ��� �Ӱ谪
} SFM_DiskInfo;
#pragma pack()

// LAN ���� ����
//
#pragma pack(1)
typedef struct {
	char			mask;
	char			name[SFM_MAX_LAN_NAME_LEN]; //
	char			targetIp[SFM_MAX_TARGET_IP_LEN]; //
	unsigned char	status;     // ���� ����
	unsigned char	prevStatus; // ���� ����
	unsigned char	level;      // ��� �߻� �� �ش� ���μ����� ����� ��� ���
} SFM_LanInfo;
#pragma pack()

// Process ���� ����
//
#pragma pack(1)
typedef struct {
	unsigned char	mask;
	char			name[COMM_MAX_NAME_LEN]; //
	unsigned char	status;     // ���� ����
	unsigned char	prevStatus; // ���� ����
	unsigned char	level;      // ��� �߻� �� �ش� ���μ����� ����� ��� ���
	pid_t           pid;
    time_t          uptime;
} SFM_ProcInfo;
#pragma pack()

#if 1
#pragma pack(1)
typedef struct {
	int				qID;
	int				qKEY;
    unsigned char	mask;
	char			qNAME[QUE_MAX_NAME_LEN];
	unsigned int	qNUM;
	unsigned int	cBYTES;
	unsigned int	qBYTES;
	unsigned int	load;     // ���� �����
	unsigned char	level;     // ���� ��� ���
	unsigned char	minLimit;  // minor ��� �Ӱ谪
	unsigned char	majLimit;  // major ��� �Ӱ谪
	unsigned char	criLimit;  // critical ��� �Ӱ谪
} SFM_QueInfo;
#pragma pack()
#endif
// Device 
#pragma pack(1)
typedef struct {
	char			dNAME[COMM_MAX_NAME_LEN];
	unsigned char	status;
} SFM_DevInfo;
#pragma pack()

/* by helca */
// System ����
#pragma pack(1)
typedef struct {
	unsigned char	mask;
	unsigned char 	myStatus;   /* 1 : ACTIVE, 2 : STANDBY */
	unsigned char 	yourStatus; /* 1 : ACTIVE, 2 : STANDBY */
	unsigned char 	sizeMin[17];		
	long long  		llCorelationId;
	unsigned int    uiTrsId;
	unsigned int 	uiUdrSeq;
	unsigned int	heartbeatAlm;  	/* 1 : Normal, 2 : AbNormal(alarm) */
	unsigned int	oosAlm;   		/* 1 : Normal, 2 : Out-of-Service occured(alarm) */
}SFM_SysDupSts;
#pragma pack()

#pragma pack(1)
typedef struct {
	unsigned char	mask;
	unsigned char	name[COMM_MAX_NAME_LEN];
	unsigned int	cnt; /* count occured */
	unsigned int	rate;  /* success rate, unit:percent */
}SFM_SysSuccRate;
#pragma pack()

#pragma pack(1)
typedef struct {
	unsigned char	mask;
	unsigned char   sDbAlias[20];
	unsigned char   sIpAddress[16];
	unsigned int    iStatus; /* 1 - connect, 0 - not connect */
}SFM_SysDBSts;
#pragma pack()

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

/* ���� Server �� ȣó���� ��� ���� */
#pragma pack(1)
typedef struct {
    unsigned short  req;
    unsigned short  succ;
    unsigned short  underLimit;   // ��� �Ӱ谪
    char            level;
    char            maskFlag;   // ��� �ݸ� ����
    //time_t          when;       // ��� �߻� �ð�
} SFM_MsgInfo;
#pragma pack()

#define SFM_MAX_RSRC_LOAD_CNT   16

#pragma pack(1)
typedef struct{
#define DEF_MMDB_SESS       0
#define DEF_MMDB_OBJ        1
#define DEF_MMDB_CDR        2
#define DEF_MMDB_CALL       5
#define DEF_MMDB_WAP1       6
#define DEF_MMDB_WAP2       7
#define DEF_MMDB_HTTP       8
#define DEF_MMDB_UDR        10
#define DEF_MMDB_VODS       11
#define DEF_MMDB_SESS2		12
#define DEF_MMDB_OBJ2		13
#define DEF_MMDB_CDR2		14
#define DEF_MMDB_VT         15
        unsigned char   mask;
        unsigned int    rsrcload;
		unsigned char	level; // by helca
        unsigned char 	minLimit;  // minor ��� �Ӱ谪 -> �� ���� �ʰ��Ͽ� duration �̻� ���ӵǸ� ��ַ� �����Ѵ�.
        unsigned char 	majLimit;  // major ��� �Ӱ谪
        unsigned char	criLimit;  // critical ��� �Ӱ谪
        unsigned char	minDurat;  //
        unsigned char	majDurat;  //
        unsigned char	criDurat;  //
		SFM_HwAlmFlag_s	rsrcFlag;  // by helca
}SFM_SysRSRCInfo;
#pragma pack()

#pragma pack(1)
typedef struct{
	unsigned char mask;
	unsigned char status;
}SFM_NTPSts;
#pragma pack()

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//
// ��� �ý��ۿ��� �������� �����Ǿ�� �ϴ� ���� ����
//
/* by helca */
#if 1
#pragma pack(1)
typedef struct {
	char			type		[COMM_MAX_NAME_LEN];  // system type
	char			group		[COMM_MAX_NAME_LEN]; // system group name
	char			name		[COMM_MAX_NAME_LEN];  // system name
	unsigned char	cpuCnt;   // ������� cpu ����
	unsigned char	diskCnt;  // ������� disk partition ����
	unsigned char	lanCnt;   // ������� lan ����
	unsigned char	procCnt;  // ������� process ����
	unsigned char	queCnt;   // ������� queue ����	added 2004/02/04
	unsigned char	rmtLanCnt;
	unsigned short	sessLoad[SFM_MAX_SESSION_CNT]; // ���� ��� ���� ����.
	SFM_CpuInfo		cpuInfo;
	SFM_MemInfo		memInfo;
	SFM_DiskInfo	diskInfo	[SFM_MAX_DISK_CNT];
	SFM_LanInfo		lanInfo		[SFM_MAX_LAN_CNT];
	SFM_ProcInfo	procInfo	[SFM_MAX_PROC_CNT];
	SFM_QueInfo		queInfo		[SFM_MAX_QUE_CNT];			// added 2004/02/04
	SFM_LanInfo		rmtLanInfo	[SFM_MAX_RMT_LAN_CNT];			// ������ ��� ��� ����
	SFM_LanInfo		optLanInfo	[2];						// ����� ���� ����
	SFM_SysDupSts	systemDup;
	SFM_SysSuccRate	succRate	[SFM_MAX_SUCC_RATE_CNT];
	SFM_SysDBSts	rmtDbSts	[SFM_MAX_DB_CNT];
	SFM_SysRSRCInfo rsrcSts     [SFM_MAX_RSRC_LOAD_CNT];
	SFM_NTPSts		ntpSts		[MAX_HW_NTP];
} SFM_SysCommInfo;
#pragma pack()
#endif

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

// HPUX H/W ���� ����
//
#pragma pack(1)
typedef struct {
	unsigned char 	mask;
    unsigned char   status;
    unsigned char   prevStatus;
    unsigned char   level;
	char			name[COMM_MAX_NAME_LEN];
} SFM_HpUxHWInfo_s;
#pragma pack()

#pragma pack(1)
typedef struct {
    SFM_HpUxHWInfo_s  hwcom[SFM_MAX_HPUX_HW_COM];
} SFM_HpUxHWInfo;
#pragma pack()

//------------------------------------------------------------------------------

// AltiBase, OpenCall ������� ���� 
//
#pragma pack(1)
typedef struct {
	char			mask;
	char			name[SFM_MAX_LAN_NAME_LEN]; //
	char			targetIp[SFM_MAX_TARGET_IP_LEN]; //
	unsigned char	status;     // ���� ����
	unsigned char	prevStatus; // ���� ����
	unsigned char	level;      // ��� �߻� �� �ش� ���μ����� ����� ��� ���
} SFM_NetMonInfo;
#pragma pack()

#pragma pack(1)
typedef struct {
	char            mask;
	unsigned char   prevStatus;
	unsigned char   status;
//# define          SFM_DB_SYNC_OK      0
//# define          SFM_DB_SYNC_NOK     1
#	define          SFM_DB_REPL_STOP        0   /* replication�� stop�� ���� :  ������ ������ */
#	define          SFM_DB_REPL_STARTED     1   /* replication�� start�� ���� : ���� ������ */
#	define          SFM_DB_REPL_SYNC_RUN    2   /* ��ü table sync��... : ����   ������. replGap�� �ǹ� ���� */
#   define          SFM_DB_REPL_GIVE_UP     3   /* replication give_up��. replGap �ǹ� ���� */
#   define          SFM_DB_REPL_LAN_FAIL    4   /* db ��� lan fail: ������ ������ */
#	define			SFM_DB_CHECK_DOWN		5
	unsigned char   level; 						/* replication state level */ 
	unsigned char   gaplevel;  					/* replication gap level */
	unsigned int	prevReplGap;				/* previous replication gap */
	unsigned int    replGap;                    /* status SFM_DB_REPL_STARTED��  ���¿����� �����Ѵ�. */
#if 1 
	unsigned char	minLimit;  // minor ��� �Ӱ谪
	unsigned char	majLimit;  // major ��� �Ӱ谪
	unsigned char	criLimit;  // critical ��� �Ӱ谪
#endif
} SFM_DBSyncInfo;
#pragma pack()

//
// SMS������ �����Ǿ�� �ϴ� ���� ����
//
/* by helca */
#pragma pack(1)
typedef struct {
	SFM_HpUxHWInfo    	hpuxHWInfo;
	SFM_NetMonInfo		netMon[SFM_MAX_NET_MON_CNT]; // altibase, openCall ���� ����. 
//	SFM_DBSyncInfo		sync;
	unsigned char		lanCnt;   // ������� lan ����
} SFM_SpecInfoSMS;
#pragma pack()

//
// �ý��� �������� Ư���� ���� �ٸ��� �����Ǿ�� �ϴ� ���� ����
//
#pragma pack(1)
typedef struct {
	union {
		SFM_SpecInfoSMS		sms;
	} u;
} SFM_SysSpecInfo;
#pragma pack()

//-----------------------------------------------------------------------------

//
// �� �ý��ۺ��� �����Ǿ�� �ϴ� ����/��� ����
//

#if 1 // by helca
#pragma pack(1)
typedef struct {
	SFM_SysAlmInfo		almInfo;
	SFM_SysCommInfo		commInfo;
	SFM_SysSpecInfo		specInfo;
} SFM_SysInfo;
#pragma pack()
#endif

//------------------------------------------------------------------------------

#pragma pack(1)
typedef struct {
	char			name[COMM_MAX_NAME_LEN]; // group name
    char			type[COMM_MAX_NAME_LEN];  // member system type�� ����
	char			memberName[SYSCONF_MAX_GROUP_MEMBER][COMM_MAX_NAME_LEN];
	unsigned char	memberCnt;
	unsigned char	level;     // �ش� �ý��� �׷��� ��ü ��� ���
	unsigned char	minCnt;    // ���� �߻��Ǿ� �ִ� minor ��� ����
	unsigned char	majCnt;    // ���� �߻��Ǿ� �ִ� major ��� ����
	unsigned char	criCnt;    // ���� �߻��Ǿ� �ִ� critical ��� ����
} SFM_SysGroupInfo;
#pragma pack()

//------------------------------------------------------------------------------
#define MAX_CONN_IP     200


/* ���� Server or Client ���� TCP Connection ��� ���� *************/
// ��ϵ� IP �߿��� 'connNum == 0' �� ��� ������ Critical Alarm�� �߻���Ų��.

/* ���� IP �� ���� ���� */
#pragma pack(1)
typedef struct {
    in_addr_t       ipAddr;     // Server or Client IP �ּ�
    int             port;       // Server or Client ���� Port ��ȣ
    unsigned char   grpNum;     // Server or Client �ý��� ���� (SMPP, LDSPC, ...)
#   define      SYS_SMSC        1 
#   define      SYS_VAS         2 
#   define      SYS_LMSC        3
#   define      SYS_IDR         4
#   define      SYS_SCP         5
#   define      SYS_WISE        6
#   define      SYS_BACKUP		7

    unsigned char   type;
#define      TCPINFO_SUB_TYPE   	1
#define      TCPINFO_BILL_TYPE   	2
#define      TCPINFO_SCPIF_TYPE  	3
#define      TCPINFO_WISE_TYPE   	4
#define      TCPINFO_BKUP_TYPE		5

#if 0
#   define      WISEIF_TYPE  	0
#   define      SCPIF_TYPE  	1
#   define      SCIB_TYPE   	2
#   define      RCIF_TYPE   	3
#   define      BACKUP_TYPE   	4
#endif

    short           curConn;    // Current Connection ����
    short           preConn;    // Previos Connection ����
    short           maxConn;    // �ִ� Connection ����
    char            maskFlag;   // ��� �ݸ� ����
    char            level;  	// Connection ��� ���
    time_t          when;       // ��� �߻� �ð�
} SFM_TcpState;
#pragma pack()

/* */
#pragma pack(1)
typedef struct {
    short           cnt;    // IP ����
    short           almCnt; // Connection ��� Ƚ��
    char            level;  // Connection ��� ���
    SFM_TcpState    srv[MAX_CONN_IP];
} SFM_TcpConnSys;
#pragma pack()


/* ���� Server �� Client���� ��� ���� */
#pragma pack(1)
typedef struct {
    char            level;
    SFM_TcpConnSys  sys[2];
} SFM_TcpConnInfo;
#pragma pack()

//------------------------------------------------------------------------------
#if 0
typedef struct {
    int           	req;
    int           	succ;
} SFM_MsgSts;
#endif

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#if 1
#pragma pack(1)
typedef struct {
	SFM_SysGroupInfo	group[SYSCONF_MAX_GROUP_NUM];
	SFM_SysInfo			sys[SYSCONF_MAX_ASSO_SYS_NUM];
	SFM_HwAlmFlag		hwAlmFlag[SYSCONF_MAX_ASSO_SYS_NUM];
	SFM_TcpConnInfo     tcp_con;
	char				active_sys_name[16];
	char				last_active_name[16];
} SFM_sfdb;
#pragma pack()
#endif


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#define SFM_SFDB_SIZE	sizeof(SFM_sfdb)
#define SFM_SFDB_FILE	"DATA/sfdb_file"

#define SFM_AUDIO_FILE	"DATA/audio_file"

#endif /*__SFMCONF_H__*/
