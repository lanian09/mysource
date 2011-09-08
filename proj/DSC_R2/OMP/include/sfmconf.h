#ifndef __SFMCONF_H__
#define __SFMCONF_H__

#include "sysconf.h"
#include "comm_msgtypes.h"
#include "nmsif.h"

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
#define MAX_BOND_NUM                   	4
#define MAX_AAA_NUM						32  // the number of AAA IP addresses
#define MAX_ANAAA_NUM					32  // the number of ANAAA IP addresses

#ifndef MAX_WAPGW_NUM
#define MAX_WAPGW_NUM 					3   // the number of UAWAP GWs

#define RADIUS_IP_CNT   				50  // Radius PDSN IP COUNT
#endif



/* Session load */
#define SESSION_LOADINDEX_UPRESTO		0 /* the others is reserved */


// ��� ������ �����ϱ� ���� ��
// - ���ǵ� ���� Ư���� �ǹ̸� ���� �ʰ�, �ܼ��� ���θ� �����ϴ� �뵵�θ� ���ǰ�
//  key���̳� index ���� �뵵�� ������ �ʴ´�.
// - alarm_history DB�� ��� �ִ� ������ ��ȸ�Ҷ� ���ȴ�.


#define SFM_ALM_TYPE_CPU_USAGE			1
#define SFM_ALM_TYPE_MEMORY_USAGE		2
#define SFM_ALM_TYPE_DISK_USAGE			3
/* hjjung */
#define SFM_ALM_TYPE_USER_USAGE			71
#define SFM_ALM_TYPE_SESSION_USAGE		73
#define SFM_ALM_TYPE_LAN				4
#define SFM_ALM_TYPE_PROC				5
#define SFM_ALM_TYPE_LINK				6
#define SFM_ALM_TYPE_MP_HW             	7
#define SFM_ALM_TYPE_STAT				8
#define SFM_ALM_TYPE_DB_REP				9
#define SFM_ALM_TYPE_DB_REP_GAP			10
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
//#define SFM_ALM_TYPE_PD_CPU_USAGE		22
//#define SFM_ALM_TYPE_PD_MEMORY_USAGE	23
//#define SFM_ALM_TYPE_PD_FAN_STS		24
//#define SFM_ALM_TYPE_PD_GIGA_LAN		25
#define SFM_ALM_TYPE_TAP_CPU_USAGE		22
#define SFM_ALM_TYPE_TAP_MEMORY_USAGE	23
#define SFM_ALM_TYPE_TAP_FAN_STS		24
#define SFM_ALM_TYPE_TAP_PORT_STS		25
#define SFM_ALM_TYPE_RSRC_LOAD			26
#define SFM_ALM_TYPE_QUEUE_LOAD			27
#define SFM_ALM_TYPE_NMSIF_CONNECT		28

#define SFM_ALM_TYPE_DUAL_ACT			29
#define SFM_ALM_TYPE_DUAL_STD			30
#define SFM_ALM_TYPE_DUAL_STS_QRY_TIME_OUT	31

/* by june */
#define SFM_ALM_TYPE_SCE_CPU			32
#define SFM_ALM_TYPE_SCE_MEM			33
#define SFM_ALM_TYPE_SCE_DISK			34
#define SFM_ALM_TYPE_SCE_PWR			35
#define SFM_ALM_TYPE_SCE_FAN			36
#define SFM_ALM_TYPE_SCE_TEMP			37
#define SFM_ALM_TYPE_SCE_VOLT			38
#define SFM_ALM_TYPE_SCE_PORT_MGMT		39	/* shin */
#define SFM_ALM_TYPE_SCE_RDR			40
#define SFM_ALM_TYPE_SCE_RDR_CONN		41	
#define SFM_ALM_TYPE_SCE_STATUS			42	 /*sjjeon*/
#define SFM_ALM_TYPE_L2_CPU           	43	 /* shin */
#define SFM_ALM_TYPE_L2_MEM           	44	 /* shin */
#define SFM_ALM_TYPE_L2_LAN           	45	 /* shin */
#define SFM_ALM_TYPE_CPS_OVER          	46	 /* shin */
#define SFM_ALM_TYPE_SCE_PORT_LINK		47

/* sjjeon : ���μ��� ��� �޽��� �ڵ� �߰�.*/                    
#define SFM_ALM_TYPE_PROCESS_SAMD      	48  /* OMP,MP SAMD ���¾˶�  */
#define SFM_ALM_TYPE_PROCESS_IXPC      	49  /* OMP,MP IXPC ���¾˶�  */
#define SFM_ALM_TYPE_PROCESS_FIMD    	50  /* OMP FIMD ���¾˶�     */
#define SFM_ALM_TYPE_PROCESS_COND      	51  /* OMP COND ���¾˶�     */
#define SFM_ALM_TYPE_PROCESS_STMD      	52  /* OMP STMD ���¾˶�     */
#define SFM_ALM_TYPE_PROCESS_MMCD      	53  /* OMP MMCD ���¾˶�     */
#define SFM_ALM_TYPE_PROCESS_MCDM      	54  /* OMP MCMD ���¾˶�     */
#define SFM_ALM_TYPE_PROCESS_NMSIF     	55  /* OMP NMSIF ���¾˶�    */
#define SFM_ALM_TYPE_PROCESS_CDELAY    	56  /* OMP CDELAY ���¾˶�   */
#define SFM_ALM_TYPE_PROCESS_HAMON     	57  /* OMP HAMON ���¾˶�    */
#define SFM_ALM_TYPE_PROCESS_MMCR      	58  /* MP MMCR ���¾˶�      */
#define SFM_ALM_TYPE_PROCESS_RDRANA    	59  /* MP RDRANA ���¾˶�    */
//#define SFM_ALM_TYPE_PROCESS_RLEG      	60  /* MP RLEG ���¾˶�      */
#define SFM_ALM_TYPE_PROCESS_SMPP      	61  /* MP SMPP ���¾˶�      */
#define SFM_ALM_TYPE_PROCESS_PANA      	62  /* MP PANA ���¾˶�      */
#define SFM_ALM_TYPE_PROCESS_RANA      	63  /* MP RANA ���¾˶�      */
#define SFM_ALM_TYPE_PROCESS_RDRCAPD   	64  /* MP RDRCAPD ���¾˶�   */
#define SFM_ALM_TYPE_PROCESS_CAPD      	65  /* MP CAPD ���¾˶�      */
#define SFM_ALM_TYPE_HW_MIRROR      	66  /* MP HW MIRROR �˶�     */
#define SFM_ALM_TYPE_TAP_MGMT_STS		67	/* TAP MGMT ���¾˶� 	*/
#define SFM_ALM_TYPE_L2SW_MGMT_STS		68	/* L2 Switch MGMT ���¾˶� 	*/
#define SFM_ALM_TYPE_ACTIVE_STS			69	/* Dual standby ���¾˶� */

/* hjjung_20100823 */
#define SFM_ALM_TYPE_SCE_USER			70
#define SFM_ALM_TYPE_LEG_SESSION		72

/* 20100915 by dcham */
#define SFM_ALM_TYPE_SCM_FAULTED        74

#define SFM_ALM_TYPE_LOGON_SUCCESS_RATE         75 /* LOGON SUCCESS RATE, added by uamyd 20110209 */
#define SFM_ALM_TYPE_PROCESS_SCEM       76  /* OMP SCEM ���¾˶�      */
#define SFM_ALM_TYPE_PROCESS_CSCM       77  /* OMP CSCM ���¾˶�      */
#define SFM_ALM_TYPE_PROCESS_DIRM       78  /* OMP DIRM ���¾˶�      */

#define SFM_ALM_TYPE_LOGOUT_SUCCESS_RATE        79 /* LOGOUT SUCCESS RATE, added by uamyd 20110422 */
#define SFM_ALM_TYPE_TAP_POWER_STS      80  /* DIRECTOR POWER ���¾˶� */ // 20110422, by dcham
#define SFM_ALM_TYPE_SM_CONN_STS		81  /* SM Connection Link Status */
#define SFM_ALM_TYPE_PROCESS_RLEG0      82  /* MP RLEG0 ���¾˶� RLEG0~4,added by dcham 2011.04.26 */
#define SFM_ALM_TYPE_PROCESS_RLEG1      83  /* MP RLEG1 ���¾˶� */
#define SFM_ALM_TYPE_PROCESS_RLEG2      84  /* MP RLEG2 ���¾˶� */
#define SFM_ALM_TYPE_PROCESS_RLEG3      85  /* MP RLEG3 ���¾˶� */
#define SFM_ALM_TYPE_PROCESS_RLEG4      86  /* MP RLEG4 ���¾˶� */
#define SFM_ALM_TYPE_TPS                87  /* TPS ���¾˶� , added by dcham 20110525 */
#define SFM_ALM_TYPE_LAST				88  /* Alarm count 75���� 76���� ����, by uamyd */



// MAX_ALRAM_CNT 
// �Ʒ� �߰��� fimd_mmchdl.c ���� alarmName�� ����� �־�� �Ѵ�. 
// MAX_ALRAM_CNT�� ������ �־�� �Ѵ�. 
#define MAX_ALRAM_CNT	SFM_ALM_TYPE_LAST

/* hjjung_20100823 */
/* system�� index*/
enum { DSCM=0, SCMA, SCMB, TAPA, TAPB, SCEA, SCEB, RLEGA, RLEGB, L2SWA, L2SWB }; /* system Index */

/* hjjung_20100822 LEG_SESSION �߰� */
/* added by dcham 20110525 for LEG_TPS */
/* ���� �����׸� index */
enum { SCE_CPU=0, SCE_MEM, SCE_DISK, SCE_POWER, SCE_FAN, SCE_USER, 	//  0 ~  5
	   SCE_TEMP, SCE_VOLT, SCE_PORT_MGMT, SCE_PORT_LINK, SCE_RDR,	//  6 ~ 10
	   SCE_RDR_CONN, SCE_STATUS, TAP_CPU, TAP_MEM, TAP_PORT,		// 11 ~ 15
	   L2SW_CPU, L2SW_MEM, L2SW_PORT, SCM_PORT, SCM_MYSQL,			// 16 ~ 20
	   SCM_SM, SCM_CM, SCM_SM_STAT, SCM_DISK, SCM_FAN,				// 21 ~ 25
	   SCM_PWR, SCM_CPU, SCM_TIMESTEN, SCM_USER, LEG_SESSION,       // 26 ~ 30
       TAP_POWER, TPS_LOAD};	                                        // 31 ~ 32	
enum { SCE_DEV_STATUS_NORMAL=2, SCE_DEV_STATUS_ABNORMAL };


#define SFM_ALM_OCCURED		1
#define SFM_ALM_CLEARED		0
#define SFM_ALM_REMOVED		2           // Tcp Connection ���°��� �� ���
                                 	    // IP�� ������ ��� ���� ���� ǥ�� ��


// ��� ���
//
#define	SFM_ALM_NORMAL		0
#define	SFM_ALM_MINOR		1
#define	SFM_ALM_MAJOR		2
#define	SFM_ALM_CRITICAL	3

#define SFM_ALM_UNMASKED   	0
#define	SFM_ALM_MASKED		99

// ���μ��� �� �Ϲ��� ������Ʈ�� ����
//
#define SFM_STATUS_ALIVE	0
#define SFM_STATUS_DEAD		1

// LAN ����
//
#define SFM_LAN_CONNECTED	0
#define SFM_LAN_DISCONNECTED	1

// CPS OVER ����  
#define SFM_CPS_NORMAL		0
#define SFM_CPS_OVER		1

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


//#################### SCE ���� #####################

/* port status */
#define SCE_PORT_NONE	1		/* none of the following*/
#define SCE_PORT_UP		2		/* the port is up */
#define SCE_PORT_DOWN	3		/* reflection forcing down*/
#define SCE_PORT_FDOWN	4		/* redundancy forcing down*/
#define SCE_PORT_ODOWN	5		/* other reason down*/



//###################################################


// Macro related to Hardware status
// by helca 08.03
#define SFM_HW_UP		0
#define SFM_HW_DOWN		1
#define SFM_HW_NOT_EQUIP 4

// 20100915 by dcham
#define SFM_SCM_FAULTED         1
#define SFM_SCM_NOT_FAULTED     0
 
#define  SFM_HW_MAX_DISK_NUM     4
#define  SFM_HW_MAX_CPU_NUM      10 // 5 -> 10
#define  SFM_HW_MAX_LINK_NUM     16  /*sum : 16, link:8, mysql:1, timetens:1, sm:1,cm:1, smNetstat: 4*/
#define  SFM_HW_MAX_PWR_NUM      4  // 3 -> 4
#define  SFM_HW_MAX_FAN_NUM      6

#define SFM_DISK_ATTACHED       0
#define SFM_DISK_DETTACHED      1 // L3 PROBE DEVICE
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
	int		qID;
	int		qKEY;
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
	char		dNAME[COMM_MAX_NAME_LEN];
	unsigned char	status;
} SFM_DevInfo;
#pragma pack()

/* by helca */
// System ����
#pragma pack(1)
typedef struct {
	unsigned char	mask;
	unsigned char 	myStatus;   /* 1 : ACTIVE, 2 : STANDBY, 3 : FAULTED */ // FAULTED�� �߰� by dcham
	unsigned char 	yourStatus; /* 1 : ACTIVE, 2 : STANDBY, 3 : FAULTED */ // FAULTED�� �߰� by dcham
	unsigned char 	sizeMin[17];		
	long long  	llCorelationId;
	unsigned int    uiTrsId;
	unsigned int 	uiUdrSeq;
	unsigned char	heartbeatAlm;  		/* 1 : Normal, 2 : AbNormal(alarm) */
	unsigned char	timeOutAlm;  		/* 1 : Normal, 2 : AbNormal(alarm) */
	unsigned char	heartbeatLevel;
	unsigned char	timeOutAlmLevel;
	unsigned char	dualStsAlmLevel;
	//unsigned char   scmFaultStsAlmLevel;  // 20100916 by dcham
	unsigned char	oosAlm;   		/* 1 : Normal, 2 : Out-of-Service occured(alarm) */
}SFM_SysDupSts;
#pragma pack()

#pragma pack(1)
typedef struct {
	unsigned char	mask;
	unsigned char	name[COMM_MAX_NAME_LEN];
	unsigned int	cnt; /* count occured */
	unsigned int	rate;  /* success rate, unit:percent */
	unsigned char	level;
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
#define DEF_MMDB_SESS2      12
#define DEF_MMDB_OBJ2       13
#define DEF_MMDB_CDR2       14
#define DEF_MMDB_VT	    15
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

#define NTP_ALARM_TIME		(60*60)*24 // NTP Channel ��� ���� �ð�
#define NTP_INDEX_DEAMON	0
#define NTP_INDEX_CHANNEL	1
#pragma pack(1)
typedef struct{
	unsigned char mask;
	unsigned char status;
	unsigned char level;
}SFM_NTPSts;
#pragma pack()

// ������ ���� : sjjeon
#pragma pack(1)
typedef struct{
	unsigned char mask;
	unsigned char preStatus;
	unsigned char status;
	unsigned char level;
}SFM_CpsOverLoadsts;
#pragma pack()

#define SFM_SM_CONN_STATUS_LINK_UP 1
#define SFM_SM_CONN_STATUS_LINK_DN 0
#pragma pack(1)
typedef struct{
	unsigned char mask;
	unsigned char status;
	unsigned char preStatus;
	unsigned char level;      // ��� �߻� �� �ش� ���μ����� ����� ��� ���
	
}SFM_SubSMInfo;
typedef struct{
#ifndef MAX_RLEG_CNT
#define SFM_MAX_SM_CH_CNT 1 // added by dcham 20110530 for SM connection(5=>1)
#else
#define SFM_MAX_SM_CH_CNT MAX_RLEG_CNT
#endif
	// SM Channel Count by uamyd 20110425, MAX_RLEG_CNT �� �����ϰų� �� �̻��� ���� ������ ��.
    SFM_SubSMInfo each[SFM_MAX_SM_CH_CNT];
	unsigned char level;
	unsigned char minLimit;
	unsigned char majLimit;
	unsigned char criLimit;
	unsigned char minFlag;
	unsigned char majFlag;
	unsigned char criFlag;
}SFM_SMChInfo;
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
	char		type		[COMM_MAX_NAME_LEN];  // system type
	char		group		[COMM_MAX_NAME_LEN]; // system group name
	char		name		[COMM_MAX_NAME_LEN];  // system name
	unsigned char	cpuCnt;   // ������� cpu ����
	unsigned char	diskCnt;  // ������� disk partition ����
	unsigned char	lanCnt;   // ������� lan ����
	unsigned char	procCnt;  // ������� process ����
	unsigned char	queCnt;   // ������� queue ����	added 2004/02/04
    unsigned short  total_disk_usage;
	unsigned char	rmtLanCnt;
	unsigned short	sessLoad[SFM_MAX_SESSION_CNT]; 			// ���� ��� ���� ����.
	SFM_CpuInfo		cpuInfo;
	SFM_MemInfo		memInfo;
	SFM_DiskInfo	diskInfo	[SFM_MAX_DISK_CNT];
	SFM_LanInfo		lanInfo		[SFM_MAX_LAN_CNT];
	SFM_ProcInfo	procInfo	[SFM_MAX_PROC_CNT];
	SFM_QueInfo		queInfo		[SFM_MAX_QUE_CNT];			// added 2004/02/04
	SFM_LanInfo		rmtLanInfo	[SFM_MAX_RMT_LAN_CNT];	 	//������ ��� ��� ����
	SFM_LanInfo		optLanInfo	[2];						// ����� ���� ����
	SFM_SysDupSts	systemDup;
	SFM_SysSuccRate	succRate	[SFM_MAX_SUCC_RATE_CNT];
	SFM_SysDBSts	rmtDbSts	[SFM_MAX_DB_CNT];
	SFM_SysRSRCInfo rsrcSts     [SFM_MAX_RSRC_LOAD_CNT];
	SFM_NTPSts		ntpSts		[MAX_HW_NTP];
	SFM_CpsOverLoadsts cpsOverSts;							// ������ ���� : sjjeon
	SFM_SMChInfo    smChSts;
//	SFM_TpsOverLoadsts tpsOverSts;	// ������ ���� : 20110525 added by dcham 
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
	char		mask;
	char		name[SFM_MAX_LAN_NAME_LEN]; //
	char		targetIp[SFM_MAX_TARGET_IP_LEN]; //
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
	#define          SFM_DB_REPL_STOP        0   /* replication�� stop�� ���� :  ������ ������ */
	#define          SFM_DB_REPL_STARTED     1   /* replication�� start�� ���� : ���� ������ */
	#define          SFM_DB_REPL_SYNC_RUN    2   /* ��ü table sync��... : ����   ������. replGap�� �ǹ� ���� */
	#define          SFM_DB_REPL_GIVE_UP     3   /* replication give_up��. replGap �ǹ� ���� */
	#define          SFM_DB_REPL_LAN_FAIL    4   /* db ��� lan fail: ������ ������ */
	#define			SFM_DB_CHECK_DOWN		5
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

typedef struct{
	unsigned char   mask;
	unsigned int    ipAddr;
	unsigned int    count; /* count occured */
	unsigned int    rate;  /* success rate, unit:percent */
	unsigned char   level;
}SuccRateIpInfo;

typedef struct{
	unsigned char   mask;
	char            ipAddr[SFM_MAX_TARGET_IP_LEN]; 
	unsigned int    count; /* count occured */
	unsigned char   level;
}SuccRate_RadiusInfo;

typedef struct{
	SuccRateIpInfo uawap[MAX_WAPGW_NUM];
	SuccRateIpInfo aaa[MAX_AAA_NUM];
	SuccRateIpInfo anaaa[MAX_ANAAA_NUM];
	SuccRate_RadiusInfo radius[RADIUS_IP_CNT];
}SFM_SysSuccRateIpInfo;

#if 1 // by helca
#pragma pack(1)
typedef struct {
	SFM_SysAlmInfo		almInfo;
	SFM_SysCommInfo		commInfo;
	SFM_SysSpecInfo		specInfo;
	SFM_SysSuccRateIpInfo	succRateIpInfo;
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
/* by june, hjjung
 * MP RLEG <--> OMP FIMD interface structure
 * Report CPS of LEG */
#pragma pack(1)
typedef struct _st_leg_sum_cps_ {
	unsigned int	uiLogOnSumCps;
	unsigned int	uiLogOutSumCps;
} LEG_SUM_CPS;
#pragma pack()

#pragma pack(1)
typedef struct _st_leg_sess_data_{
	unsigned int	amount;
} LEG_SESS_DATA;
#pragma pack()

#pragma pack(1)
typedef struct _st_leg_info_{
	LEG_SUM_CPS     cps;
	LEG_SESS_DATA   sess;
	// 20110525 added by dcham, TPS(Transaction Per Second)��
	unsigned int tps; 
} CALL_DATA;
#pragma pack()

#pragma pack(1)
typedef struct _st_leg_sess_status_{
#define MAX_CALL_DEV_NUM 2
	unsigned char   mask;       /* mml �� ������ alarm �߻� ���� ���� */
	unsigned int	num;        /* ���� ��� �� */
	unsigned char   level;      /* ���� ��� ��� */
	unsigned int    minLimit;   /* minor ��� �Ӱ谪 -> �� ���� �ʰ��Ͽ� duration �̻� ���ӵǸ� ��ַ� �����Ѵ�. */
	unsigned int    majLimit;   /* major ��� �Ӱ谪 */
	unsigned int    criLimit;   /* critical ��� �Ӱ谪 */
	unsigned char   minDurat;
	unsigned char   majDurat;   
	unsigned char   criDurat;   
	unsigned char   minFlag;    /* minor ��� �߻��Ǹ� setting�ȴ�. ����� �ϰ��ϴ� ��� ���� �޽����� ������ �ϴ��� �����ϴµ� ����ȴ�. */
	unsigned char   majFlag;    /* major ��� �߻��Ǹ� setting�ȴ�. ����� �ϰ��ϴ� ��� ���� �޽����� ������ �ϴ��� �����ϴµ� ����ȴ�. */
	unsigned char   criFlag;    /* critical ��� �߻��Ǹ� setting�ȴ�. ����� �ϰ��ϴ� ��� ���� �޽����� ������ �ϴ��� �����ϴµ� ����ȴ�. */
} LEG_SESS_NUM_INFO;
#pragma pack()

/* TPS added by dcham 20110525 */
#pragma pack(1)
typedef struct _st__tps_call_status_{
#define MAX_CALL_DEV_NUM 2
	unsigned char   mask;       /* mml �� ������ alarm �߻� ���� ���� */
	unsigned int	num;        /* ���� ��� �� */
	unsigned char   level;      /* ���� ��� ��� */
	unsigned int    minLimit;   /* minor ��� �Ӱ谪 -> �� ���� �ʰ��Ͽ� duration �̻� ���ӵǸ� ��ַ� �����Ѵ�. */
	unsigned int    majLimit;   /* major ��� �Ӱ谪 */
	unsigned int    criLimit;   /* critical ��� �Ӱ谪 */
	unsigned char   minDurat;
	unsigned char   majDurat;   
	unsigned char   criDurat;   
	unsigned char   minFlag;    /* minor ��� �߻��Ǹ� setting�ȴ�. ����� �ϰ��ϴ� ��� ���� �޽����� ������ �ϴ��� �����ϴµ� ����ȴ�. */
	unsigned char   majFlag;    /* major ��� �߻��Ǹ� setting�ȴ�. ����� �ϰ��ϴ� ��� ���� �޽����� ������ �ϴ��� �����ϴµ� ����ȴ�. */
	unsigned char   criFlag;    /* critical ��� �߻��Ǹ� setting�ȴ�. ����� �ϰ��ϴ� ��� ���� �޽����� ������ �ϴ��� �����ϴµ� ����ȴ�. */
} TPS_INFO;
#pragma pack()

#pragma pack(1)
typedef struct {
#define MAX_CALL_DEV_NUM 2
	LEG_SUM_CPS				cps;
	LEG_SESS_NUM_INFO		legInfo[MAX_CALL_DEV_NUM];
	/* TPS added by dcham 20110525 */
	TPS_INFO				tpsInfo[MAX_CALL_DEV_NUM];
} SFM_CALL;
#pragma pack()  

#define SFM_CPS_SIZE    sizeof(LEG_SUM_CPS)
#define SFM_SESS_SIZE   sizeof(LEG_SESS_NUM_INFO)
#define SFM_TPS_SIZE    sizeof(TPS_INFO) 
#define SFM_CALL_SIZE   (SFM_CPS_SIZE + SFM_SESS_SIZE + SFM_TPS_SIZE)

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/* by jjinri
 * OMP STMD <--> OMP FIMD interface structure
 * Report SVC ALM of STMD */
typedef struct _st_stmd_stat_ {
	unsigned int uiLogOnSumCps;
	unsigned int uiLogOutSumCps;
} STMD_ALM_STAT;
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/* by uamyd 20110208
 * OMP STMD <--> OMP FIMD interface structure
 * Report LOGON SUCCESS RATE */
typedef struct {
	char           szSysName[LEG_STAT_SYSNAME];
	char           stat_date[32];
	unsigned short rate;
	unsigned int   request;
	unsigned int   success;
} STAT_LOGON_RATE;

#pragma pack(1)
typedef struct _st_logon_success_rate_status_{
	unsigned char  mask;
	unsigned short rate;
	unsigned char  level;
	unsigned int   minLimit;
	unsigned int   majLimit;
	unsigned int   criLimit;
	unsigned char  minFlag;
	unsigned char  majFlag;
	unsigned char  criFlag;
} SFM_LOGON;
#pragma pack()
//------------------------------------------------------------------------------

#if 1
#pragma pack(1)
typedef struct {
	SFM_SysGroupInfo	group[SYSCONF_MAX_GROUP_NUM];
	SFM_SysInfo			sys[SYSCONF_MAX_ASSO_SYS_NUM];
	SFM_HwAlmFlag		hwAlmFlag[SYSCONF_MAX_ASSO_SYS_NUM];
	SFM_NMSInfo			nmsInfo;
	char				active_sys_name[16];
	char				last_active_name[16];
	char				auto_sce_mode;		// auto-sce-mode  0: Atuo(default) 1: manual
	/* hjjung_20100823 */
//	//SFM_LEG				legData;
        SFM_CALL            callData; // added by dcham 20110525
} SFM_sfdb;
#pragma pack()
#endif


typedef struct {
	unsigned char   minLimit;  // minor ��� �Ӱ谪 -> �� ���� �ʰ��Ͽ� duration �̻� ���ӵǸ� ��ַ� �����Ѵ�.
    unsigned char   majLimit;  // major ��� �Ӱ谪
    unsigned char   criLimit;  // critical ��� �Ӱ谪
 
	unsigned char	minFlag;  // minor ��� �߻��Ǹ� setting�ȴ�. ����� �ϰ��ϴ� ��� ���� �޽����� ������ �ϴ��� �����ϴµ� ����ȴ�.
	unsigned char	majFlag;  // major ��� �߻��Ǹ� setting�ȴ�. ����� �ϰ��ϴ� ��� ���� �޽����� ������ �ϴ��� �����ϴµ� ����ȴ�.
	unsigned char	criFlag;  // critical ��� �߻��Ǹ� setting�ȴ�. 
} Threshold_st;

/* hjjung */
typedef struct {
	unsigned int    minLimit;  // minor ��� �Ӱ谪 -> �� ���� �ʰ��Ͽ� duration �̻� ���ӵǸ� ��ַ� �����Ѵ�.
    unsigned int    majLimit;  // major ��� �Ӱ谪
    unsigned int    criLimit;  // critical ��� �Ӱ谪
 
	unsigned char	minFlag;  // minor ��� �߻��Ǹ� setting�ȴ�. ����� �ϰ��ϴ� ��� ���� �޽����� ������ �ϴ��� �����ϴµ� ����ȴ�.
	unsigned char	majFlag;  // major ��� �߻��Ǹ� setting�ȴ�. ����� �ϰ��ϴ� ��� ���� �޽����� ������ �ϴ��� �����ϴµ� ����ȴ�.
	unsigned char	criFlag;  // critical ��� �߻��Ǹ� setting�ȴ�. 
} Threshold_st_int;
/* added by dcham 20110525 */
typedef struct {
	unsigned int    minLimit;  // minor ��� �Ӱ谪 -> �� ���� �ʰ��Ͽ� duration �̻� ���ӵǸ� ��ַ� �����Ѵ�.
    unsigned int    majLimit;  // major ��� �Ӱ谪
    unsigned int    criLimit;  // critical ��� �Ӱ谪
 
	unsigned char	minFlag;  // minor ��� �߻��Ǹ� setting�ȴ�. ����� �ϰ��ϴ� ��� ���� �޽����� ������ �ϴ��� �����ϴµ� ����ȴ�.
	unsigned char	majFlag;  // major ��� �߻��Ǹ� setting�ȴ�. ����� �ϰ��ϴ� ��� ���� �޽����� ������ �ϴ��� �����ϴµ� ����ȴ�.
	unsigned char	criFlag;  // critical ��� �߻��Ǹ� setting�ȴ�. 
} Threshold_st_tps;

typedef struct
{
	int	sysIndex;
	int	devKind;
	int	devIndex;
	int	usage;
	int	curStatus;
	int	preStatus;
	int occurFlag;
	int changeFlag;
} SCE_USAGE_PARAM;

typedef struct
{
	int	sysIndex;
	int	devKind;
	int	devIndex;
	int	status;
	int	curStatus;
	int	preStatus;
	int occurFlag;
	int changeFlag;
} SCE_STATUS_PARAM;

// by sjjeon
typedef struct COMM_STS_PARM_
{
	int	sysType;
	int	devKind;
	int	devIndex;
	int	status;
//	int	curStatus;
	int	preStatus;
	int occurFlag;
	int changeFlag;
} COMM_STATUS_PARAM;

/* hjjung */
typedef struct
{
	int	sysIndex;
	int	devKind;
	int	devIndex;
	int	usage;
	int	curStatus;
	int	preStatus;
	int occurFlag;
	int changeFlag;
} LEG_USAGE_PARAM;
/* added by dcham 20110525 */
typedef struct
{
	int	sysIndex;
	int	devKind;
	int	devIndex;
	int	usage;
	int	curStatus;
	int	preStatus;
	int occurFlag;
	int changeFlag;
} TPS_PARAM;


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#define SFM_SFDB_SIZE	sizeof(SFM_sfdb)
#define SFM_SFDB_FILE	"DATA/sfdb_file"
#define SFM_L3PD_FILE   "DATA/l3pd_file"
/* 2009.04.14 by dhkim */
#define SFM_SCE_FILE	"DATA/sce_file"
#define SFM_L2SW_FILE   "DATA/l2sw_file"

/* hjjung_20100822 */
#define SFM_CALL_FILE	"DATA/call_file"
//#define SFM_TPS_FILE	"DATA/tps_file" //added by dcham 20110525

#define SFM_AUDIO_FILE	"DATA/audio_file"

/* LOGON ������ ����. added by uamyd 20110210 */
#define SFM_LOGON_FILE  "DATA/logon_success_rate_file"
#define SFM_SM_CONN_FILE  "DATA/sm_conn_sts_file"

#endif /*__SFMCONF_H__*/
