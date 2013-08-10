#ifndef __SFMCONF_H__
#define __SFMCONF_H__

#include "sysconf.h"
#include "comm_msgtypes.h"
#include "nmsif.h"

//------------------------------------------------------------------------------
// Status & Fault Management 관련된 구조를 선언한다.
//------------------------------------------------------------------------------

#ifndef COMM_MAX_NAME_LEN
#define COMM_MAX_NAME_LEN	16	
#endif
#define QUE_MAX_NAME_LEN	10 // by helca 08.08
// 각 Component별 최대 실장 갯수
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
#define SFM_MAX_NET_MON_CNT				SFM_MAX_APP_NETMON * 3   // APP당 LAN 수
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


// 장애 유형을 구분하기 위한 값
// - 정의된 값은 특별한 의미를 갖지 않고, 단순히 서로를 구분하는 용도로만 사용되고
//  key값이나 index 등의 용도로 사용되지 않는다.
// - alarm_history DB에 들어 있는 내용을 조회할때 사용된다.


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

/* sjjeon : 프로세스 장애 메시지 코드 추가.*/                    
#define SFM_ALM_TYPE_PROCESS_SAMD      	48  /* OMP,MP SAMD 상태알람  */
#define SFM_ALM_TYPE_PROCESS_IXPC      	49  /* OMP,MP IXPC 상태알람  */
#define SFM_ALM_TYPE_PROCESS_FIMD    	50  /* OMP FIMD 상태알람     */
#define SFM_ALM_TYPE_PROCESS_COND      	51  /* OMP COND 상태알람     */
#define SFM_ALM_TYPE_PROCESS_STMD      	52  /* OMP STMD 상태알람     */
#define SFM_ALM_TYPE_PROCESS_MMCD      	53  /* OMP MMCD 상태알람     */
#define SFM_ALM_TYPE_PROCESS_MCDM      	54  /* OMP MCMD 상태알람     */
#define SFM_ALM_TYPE_PROCESS_NMSIF     	55  /* OMP NMSIF 상태알람    */
#define SFM_ALM_TYPE_PROCESS_CDELAY    	56  /* OMP CDELAY 상태알람   */
#define SFM_ALM_TYPE_PROCESS_HAMON     	57  /* OMP HAMON 상태알람    */
#define SFM_ALM_TYPE_PROCESS_MMCR      	58  /* MP MMCR 상태알람      */
#define SFM_ALM_TYPE_PROCESS_RDRANA    	59  /* MP RDRANA 상태알람    */
//#define SFM_ALM_TYPE_PROCESS_RLEG      	60  /* MP RLEG 상태알람      */
#define SFM_ALM_TYPE_PROCESS_SMPP      	61  /* MP SMPP 상태알람      */
#define SFM_ALM_TYPE_PROCESS_PANA      	62  /* MP PANA 상태알람      */
#define SFM_ALM_TYPE_PROCESS_RANA      	63  /* MP RANA 상태알람      */
#define SFM_ALM_TYPE_PROCESS_RDRCAPD   	64  /* MP RDRCAPD 상태알람   */
#define SFM_ALM_TYPE_PROCESS_CAPD      	65  /* MP CAPD 상태알람      */
#define SFM_ALM_TYPE_HW_MIRROR      	66  /* MP HW MIRROR 알람     */
#define SFM_ALM_TYPE_TAP_MGMT_STS		67	/* TAP MGMT 상태알람 	*/
#define SFM_ALM_TYPE_L2SW_MGMT_STS		68	/* L2 Switch MGMT 상태알람 	*/
#define SFM_ALM_TYPE_ACTIVE_STS			69	/* Dual standby 상태알람 */

/* hjjung_20100823 */
#define SFM_ALM_TYPE_SCE_USER			70
#define SFM_ALM_TYPE_LEG_SESSION		72

/* 20100915 by dcham */
#define SFM_ALM_TYPE_SCM_FAULTED        74

#define SFM_ALM_TYPE_LOGON_SUCCESS_RATE         75 /* LOGON SUCCESS RATE, added by uamyd 20110209 */
#define SFM_ALM_TYPE_PROCESS_SCEM       76  /* OMP SCEM 상태알람      */
#define SFM_ALM_TYPE_PROCESS_CSCM       77  /* OMP CSCM 상태알람      */
#define SFM_ALM_TYPE_PROCESS_DIRM       78  /* OMP DIRM 상태알람      */

#define SFM_ALM_TYPE_LOGOUT_SUCCESS_RATE        79 /* LOGOUT SUCCESS RATE, added by uamyd 20110422 */
#define SFM_ALM_TYPE_TAP_POWER_STS      80  /* DIRECTOR POWER 상태알람 */ // 20110422, by dcham
#define SFM_ALM_TYPE_SM_CONN_STS		81  /* SM Connection Link Status */
#define SFM_ALM_TYPE_PROCESS_RLEG0      82  /* MP RLEG0 상태알람 RLEG0~4,added by dcham 2011.04.26 */
#define SFM_ALM_TYPE_PROCESS_RLEG1      83  /* MP RLEG1 상태알람 */
#define SFM_ALM_TYPE_PROCESS_RLEG2      84  /* MP RLEG2 상태알람 */
#define SFM_ALM_TYPE_PROCESS_RLEG3      85  /* MP RLEG3 상태알람 */
#define SFM_ALM_TYPE_PROCESS_RLEG4      86  /* MP RLEG4 상태알람 */
#define SFM_ALM_TYPE_TPS                87  /* TPS 상태알람 , added by dcham 20110525 */
#define SFM_ALM_TYPE_LAST				88  /* Alarm count 75에서 76으로 변경, by uamyd */



// MAX_ALRAM_CNT 
// 아래 추가시 fimd_mmchdl.c 에서 alarmName을 등록해 주어야 한다. 
// MAX_ALRAM_CNT를 증가해 주어야 한다. 
#define MAX_ALRAM_CNT	SFM_ALM_TYPE_LAST

/* hjjung_20100823 */
/* system별 index*/
enum { DSCM=0, SCMA, SCMB, TAPA, TAPB, SCEA, SCEB, RLEGA, RLEGB, L2SWA, L2SWB }; /* system Index */

/* hjjung_20100822 LEG_SESSION 추가 */
/* added by dcham 20110525 for LEG_TPS */
/* 상태 감시항목 index */
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
#define SFM_ALM_REMOVED		2           // Tcp Connection 상태관리 시 대상
                                 	    // IP가 삭제된 경우 현재 상태 표시 값


// 장애 등급
//
#define	SFM_ALM_NORMAL		0
#define	SFM_ALM_MINOR		1
#define	SFM_ALM_MAJOR		2
#define	SFM_ALM_CRITICAL	3

#define SFM_ALM_UNMASKED   	0
#define	SFM_ALM_MASKED		99

// 프로세스 등 일반적 콤포넌트의 상태
//
#define SFM_STATUS_ALIVE	0
#define SFM_STATUS_DEAD		1

// LAN 상태
//
#define SFM_LAN_CONNECTED	0
#define SFM_LAN_DISCONNECTED	1

// CPS OVER 상태  
#define SFM_CPS_NORMAL		0
#define SFM_CPS_OVER		1

#if 0
// HPUX H/W 상태
//
#define SFM_HW_NOT_EQUIP     0   /* must not equip */
#define SFM_HW_ABSENT        1   /* must equip, but absent */
#define SFM_HW_DOWN          2   /* must equip, down state */
#define SFM_HW_ONLINE        3   /* must equip, online state */
#define SFM_HW_MIRROR        4   /* must equip, mirroring */
#define              HOTSTANDBY      0
#define              ACTIVE          1
#endif


//#################### SCE 상태 #####################

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
// CPU, Memory, Disk 등 등급 상승,하강 기능이 있는 장애 감지를 위한 structure
//
#pragma pack(1)
typedef struct {
	unsigned char	minFlag;  // minor 장애 발생되면 setting된다. 등급이 하강하는 경우 해지 메시지를 보내야 하는지 결정하는데 참고된다.
	unsigned char	majFlag;  // major 장애 발생되면 setting된다. 등급이 하강하는 경우 해지 메시지를 보내야 하는지 결정하는데 참고된다.
	unsigned char	criFlag;  // critical 장애 발생되면 setting된다. 등급이 하강하는 경우 해지 메시지를 보내야 하는지 결정하는데 참고된다.
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
// 각 시스템에 대한 현재 발생되어 있는 등급별 장애 갯수와 전체 장애 등급
//
#pragma pack(1)
typedef struct {
	unsigned char	level;     // 해당 시스템의 전체 장애 등급
	unsigned char	prevLevel; // previous 해당 시스템의 전체 장애 등급
	unsigned char	minCnt;    // 현재 발생되어 있는 minor 장애 갯수
	unsigned char	majCnt;    // 현재 발생되어 있는 major 장애 갯수
	unsigned char	criCnt;    // 현재 발생되어 있는 critical 장애 갯수
} SFM_SysAlmInfo;
#pragma pack()


//------------------------------------------------------------------------------

// CPU 상태 정보
//
#pragma pack(1)
typedef struct {
	unsigned char	mask[SFM_MAX_CPU_CNT];
	unsigned short	usage[SFM_MAX_CPU_CNT];     // 현재 CPU 사용율
	unsigned char	level[SFM_MAX_CPU_CNT];     // 현재 CPU 장애 등급
	unsigned char	minLimit;  // minor 장애 임계값 -> 이 값을 초과하여 duration 이상 지속되면 장애로 판정한다.
	unsigned char	majLimit;  // major 장애 임계값
	unsigned char	criLimit;  // critical 장애 임계값
	unsigned char	minDurat;  //
	unsigned char	majDurat;  //
	unsigned char	criDurat;  //
} SFM_CpuInfo;
#pragma pack()

// memory 상태 정보
//
#pragma pack(1)
typedef struct {
	unsigned char	mask;
	unsigned short	usage;     // 현재 memory 사용율
	unsigned char	level;     // 현재 memory 장애 등급
	unsigned char	minLimit;  // minor 장애 임계값 -> 이 값을 초과하여 duration 이상 지속되면 장애로 판정한다.
	unsigned char	majLimit;  // major 장애 임계값
	unsigned char	criLimit;  // critical 장애 임계값
	unsigned char	minDurat;  //
	unsigned char	majDurat;  //
	unsigned char	criDurat;  //
} SFM_MemInfo;
#pragma pack()

// disk 상태 정보
//
#pragma pack(1)
typedef struct {
	unsigned char	mask;
	char			name[SFM_MAX_DISK_NAME]; // disk partition name
	unsigned short	usage;     // 현재 사용율
	unsigned char	level;     // 현재 장애 등급
	unsigned char	minLimit;  // minor 장애 임계값
	unsigned char	majLimit;  // major 장애 임계값
	unsigned char	criLimit;  // critical 장애 임계값
} SFM_DiskInfo;
#pragma pack()

// LAN 상태 정보
//
#pragma pack(1)
typedef struct {
	char			mask;
	char			name[SFM_MAX_LAN_NAME_LEN]; //
	char			targetIp[SFM_MAX_TARGET_IP_LEN]; //
	unsigned char	status;     // 현재 상태
	unsigned char	prevStatus; // 이전 상태
	unsigned char	level;      // 장애 발생 시 해당 프로세스에 적용될 장애 등급
} SFM_LanInfo;
#pragma pack()

// Process 상태 정보
//
#pragma pack(1)
typedef struct {
	unsigned char	mask;
	char			name[COMM_MAX_NAME_LEN]; //
	unsigned char	status;     // 현재 상태
	unsigned char	prevStatus; // 이전 상태
	unsigned char	level;      // 장애 발생 시 해당 프로세스에 적용될 장애 등급
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
	unsigned int	load;     // 현재 사용율
	unsigned char	level;     // 현재 장애 등급
	unsigned char	minLimit;  // minor 장애 임계값
	unsigned char	majLimit;  // major 장애 임계값
	unsigned char	criLimit;  // critical 장애 임계값
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
// System 정보
#pragma pack(1)
typedef struct {
	unsigned char	mask;
	unsigned char 	myStatus;   /* 1 : ACTIVE, 2 : STANDBY, 3 : FAULTED */ // FAULTED값 추가 by dcham
	unsigned char 	yourStatus; /* 1 : ACTIVE, 2 : STANDBY, 3 : FAULTED */ // FAULTED값 추가 by dcham
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

/* 연동 Server 별 호처리량 장애 상태 */
#pragma pack(1)
typedef struct {
    unsigned short  req;
    unsigned short  succ;
    unsigned short  underLimit;   // 장애 임계값
    char            level;
    char            maskFlag;   // 장애 격리 여부
    //time_t          when;       // 장애 발생 시각
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
	unsigned char 	minLimit;  // minor 장애 임계값 -> 이 값을 초과하여 duration 이상 지속되면 장애로 판정한다.
	unsigned char 	majLimit;  // major 장애 임계값
	unsigned char	criLimit;  // critical 장애 임계값
	unsigned char	minDurat;  //
	unsigned char	majDurat;  //
	unsigned char	criDurat;  //
	SFM_HwAlmFlag_s	rsrcFlag;  // by helca
}SFM_SysRSRCInfo;
#pragma pack()

#define NTP_ALARM_TIME		(60*60)*24 // NTP Channel 장애 유지 시간
#define NTP_INDEX_DEAMON	0
#define NTP_INDEX_CHANNEL	1
#pragma pack(1)
typedef struct{
	unsigned char mask;
	unsigned char status;
	unsigned char level;
}SFM_NTPSts;
#pragma pack()

// 과부하 제어 : sjjeon
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
	unsigned char level;      // 장애 발생 시 해당 프로세스에 적용될 장애 등급
	
}SFM_SubSMInfo;
typedef struct{
#ifndef MAX_RLEG_CNT
#define SFM_MAX_SM_CH_CNT 1 // added by dcham 20110530 for SM connection(5=>1)
#else
#define SFM_MAX_SM_CH_CNT MAX_RLEG_CNT
#endif
	// SM Channel Count by uamyd 20110425, MAX_RLEG_CNT 와 동일하거나 그 이상의 값을 가져야 함.
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
// 모든 시스템에서 공통으로 관리되어야 하는 상태 정보
//
/* by helca */
#if 1
#pragma pack(1)
typedef struct {
	char		type		[COMM_MAX_NAME_LEN];  // system type
	char		group		[COMM_MAX_NAME_LEN]; // system group name
	char		name		[COMM_MAX_NAME_LEN];  // system name
	unsigned char	cpuCnt;   // 관리대상 cpu 갯수
	unsigned char	diskCnt;  // 관리대상 disk partition 갯수
	unsigned char	lanCnt;   // 관리대상 lan 갯수
	unsigned char	procCnt;  // 관리대상 process 갯수
	unsigned char	queCnt;   // 관리대상 queue 갯수	added 2004/02/04
    unsigned short  total_disk_usage;
	unsigned char	rmtLanCnt;
	unsigned short	sessLoad[SFM_MAX_SESSION_CNT]; 			// 현재 사용 하지 않음.
	SFM_CpuInfo		cpuInfo;
	SFM_MemInfo		memInfo;
	SFM_DiskInfo	diskInfo	[SFM_MAX_DISK_CNT];
	SFM_LanInfo		lanInfo		[SFM_MAX_LAN_CNT];
	SFM_ProcInfo	procInfo	[SFM_MAX_PROC_CNT];
	SFM_QueInfo		queInfo		[SFM_MAX_QUE_CNT];			// added 2004/02/04
	SFM_LanInfo		rmtLanInfo	[SFM_MAX_RMT_LAN_CNT];	 	//원격지 장비 통신 관련
	SFM_LanInfo		optLanInfo	[2];						// 광통신 관련 정보
	SFM_SysDupSts	systemDup;
	SFM_SysSuccRate	succRate	[SFM_MAX_SUCC_RATE_CNT];
	SFM_SysDBSts	rmtDbSts	[SFM_MAX_DB_CNT];
	SFM_SysRSRCInfo rsrcSts     [SFM_MAX_RSRC_LOAD_CNT];
	SFM_NTPSts		ntpSts		[MAX_HW_NTP];
	SFM_CpsOverLoadsts cpsOverSts;							// 과부하 제어 : sjjeon
	SFM_SMChInfo    smChSts;
//	SFM_TpsOverLoadsts tpsOverSts;	// 과부하 제어 : 20110525 added by dcham 
} SFM_SysCommInfo;
#pragma pack()
#endif

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

// HPUX H/W 상태 정보
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

// AltiBase, OpenCall 연결상태 정보 
//
#pragma pack(1)
typedef struct {
	char		mask;
	char		name[SFM_MAX_LAN_NAME_LEN]; //
	char		targetIp[SFM_MAX_TARGET_IP_LEN]; //
	unsigned char	status;     // 현재 상태
	unsigned char	prevStatus; // 이전 상태
	unsigned char	level;      // 장애 발생 시 해당 프로세스에 적용될 장애 등급
} SFM_NetMonInfo;
#pragma pack()

#pragma pack(1)
typedef struct {
	char            mask;
	unsigned char   prevStatus;
	unsigned char   status;
//# define          SFM_DB_SYNC_OK      0
//# define          SFM_DB_SYNC_NOK     1
	#define          SFM_DB_REPL_STOP        0   /* replication이 stop된 상태 :  비정상 상태임 */
	#define          SFM_DB_REPL_STARTED     1   /* replication이 start된 상태 : 정상 상태임 */
	#define          SFM_DB_REPL_SYNC_RUN    2   /* 전체 table sync중... : 정상   상태임. replGap이 의미 없음 */
	#define          SFM_DB_REPL_GIVE_UP     3   /* replication give_up됨. replGap 의미 없음 */
	#define          SFM_DB_REPL_LAN_FAIL    4   /* db 모든 lan fail: 비정상 상태임 */
	#define			SFM_DB_CHECK_DOWN		5
	unsigned char   level; 						/* replication state level */ 
	unsigned char   gaplevel;  					/* replication gap level */
	unsigned int	prevReplGap;				/* previous replication gap */
	unsigned int    replGap;                    /* status SFM_DB_REPL_STARTED인  상태에서만 참조한다. */
#if 1 
	unsigned char	minLimit;  // minor 장애 임계값
	unsigned char	majLimit;  // major 장애 임계값
	unsigned char	criLimit;  // critical 장애 임계값
#endif
} SFM_DBSyncInfo;
#pragma pack()

//
// SMS에서만 관리되어야 하는 상태 정보
//
/* by helca */
#pragma pack(1)
typedef struct {
	SFM_HpUxHWInfo    	hpuxHWInfo;
	SFM_NetMonInfo		netMon[SFM_MAX_NET_MON_CNT]; // altibase, openCall 접속 정보. 
//	SFM_DBSyncInfo		sync;
	unsigned char		lanCnt;   // 관리대상 lan 갯수
} SFM_SpecInfoSMS;
#pragma pack()

//
// 시스템 유형별로 특성에 따라 다르게 관리되어야 하는 상태 정보
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
// 각 시스템별로 관리되어야 하는 상태/장애 정보
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
    char			type[COMM_MAX_NAME_LEN];  // member system type와 동일
	char			memberName[SYSCONF_MAX_GROUP_MEMBER][COMM_MAX_NAME_LEN];
	unsigned char	memberCnt;
	unsigned char	level;     // 해당 시스템 그룹의 전체 장애 등급
	unsigned char	minCnt;    // 현재 발생되어 있는 minor 장애 갯수
	unsigned char	majCnt;    // 현재 발생되어 있는 major 장애 갯수
	unsigned char	criCnt;    // 현재 발생되어 있는 critical 장애 갯수
} SFM_SysGroupInfo;
#pragma pack()

//------------------------------------------------------------------------------
#define MAX_CONN_IP     200


/* 연동 Server or Client 와의 TCP Connection 장애 정보 *************/
// 등록된 IP 중에서 'connNum == 0' 인 경우 무조건 Critical Alarm을 발생시킨다.

/* 연동 IP 별 접속 상태 */
#pragma pack(1)
typedef struct {
    in_addr_t       ipAddr;     // Server or Client IP 주소
    int             port;       // Server or Client 접속 Port 번호
    unsigned char   grpNum;     // Server or Client 시스템 유형 (SMPP, LDSPC, ...)
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

    short           curConn;    // Current Connection 갯수
    short           preConn;    // Previos Connection 갯수
    short           maxConn;    // 최대 Connection 갯수
    char            maskFlag;   // 장애 격리 여부
    char            level;  	// Connection 장애 등급
    time_t          when;       // 장애 발생 시각
} SFM_TcpState;
#pragma pack()

/* */
#pragma pack(1)
typedef struct {
    short           cnt;    // IP 갯수
    short           almCnt; // Connection 장애 횟수
    char            level;  // Connection 장애 등급
    SFM_TcpState    srv[MAX_CONN_IP];
} SFM_TcpConnSys;
#pragma pack()


/* 연동 Server 및 Client와의 장애 상태 */
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
	// 20110525 added by dcham, TPS(Transaction Per Second)량
	unsigned int tps; 
} CALL_DATA;
#pragma pack()

#pragma pack(1)
typedef struct _st_leg_sess_status_{
#define MAX_CALL_DEV_NUM 2
	unsigned char   mask;       /* mml 로 설정된 alarm 발생 여부 설정 */
	unsigned int	num;        /* 현재 사용 수 */
	unsigned char   level;      /* 현재 장애 등급 */
	unsigned int    minLimit;   /* minor 장애 임계값 -> 이 값을 초과하여 duration 이상 지속되면 장애로 판정한다. */
	unsigned int    majLimit;   /* major 장애 임계값 */
	unsigned int    criLimit;   /* critical 장애 임계값 */
	unsigned char   minDurat;
	unsigned char   majDurat;   
	unsigned char   criDurat;   
	unsigned char   minFlag;    /* minor 장애 발생되면 setting된다. 등급이 하강하는 경우 해지 메시지를 보내야 하는지 결정하는데 참고된다. */
	unsigned char   majFlag;    /* major 장애 발생되면 setting된다. 등급이 하강하는 경우 해지 메시지를 보내야 하는지 결정하는데 참고된다. */
	unsigned char   criFlag;    /* critical 장애 발생되면 setting된다. 등급이 하강하는 경우 해지 메시지를 보내야 하는지 결정하는데 참고된다. */
} LEG_SESS_NUM_INFO;
#pragma pack()

/* TPS added by dcham 20110525 */
#pragma pack(1)
typedef struct _st__tps_call_status_{
#define MAX_CALL_DEV_NUM 2
	unsigned char   mask;       /* mml 로 설정된 alarm 발생 여부 설정 */
	unsigned int	num;        /* 현재 사용 수 */
	unsigned char   level;      /* 현재 장애 등급 */
	unsigned int    minLimit;   /* minor 장애 임계값 -> 이 값을 초과하여 duration 이상 지속되면 장애로 판정한다. */
	unsigned int    majLimit;   /* major 장애 임계값 */
	unsigned int    criLimit;   /* critical 장애 임계값 */
	unsigned char   minDurat;
	unsigned char   majDurat;   
	unsigned char   criDurat;   
	unsigned char   minFlag;    /* minor 장애 발생되면 setting된다. 등급이 하강하는 경우 해지 메시지를 보내야 하는지 결정하는데 참고된다. */
	unsigned char   majFlag;    /* major 장애 발생되면 setting된다. 등급이 하강하는 경우 해지 메시지를 보내야 하는지 결정하는데 참고된다. */
	unsigned char   criFlag;    /* critical 장애 발생되면 setting된다. 등급이 하강하는 경우 해지 메시지를 보내야 하는지 결정하는데 참고된다. */
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
	unsigned char   minLimit;  // minor 장애 임계값 -> 이 값을 초과하여 duration 이상 지속되면 장애로 판정한다.
    unsigned char   majLimit;  // major 장애 임계값
    unsigned char   criLimit;  // critical 장애 임계값
 
	unsigned char	minFlag;  // minor 장애 발생되면 setting된다. 등급이 하강하는 경우 해지 메시지를 보내야 하는지 결정하는데 참고된다.
	unsigned char	majFlag;  // major 장애 발생되면 setting된다. 등급이 하강하는 경우 해지 메시지를 보내야 하는지 결정하는데 참고된다.
	unsigned char	criFlag;  // critical 장애 발생되면 setting된다. 
} Threshold_st;

/* hjjung */
typedef struct {
	unsigned int    minLimit;  // minor 장애 임계값 -> 이 값을 초과하여 duration 이상 지속되면 장애로 판정한다.
    unsigned int    majLimit;  // major 장애 임계값
    unsigned int    criLimit;  // critical 장애 임계값
 
	unsigned char	minFlag;  // minor 장애 발생되면 setting된다. 등급이 하강하는 경우 해지 메시지를 보내야 하는지 결정하는데 참고된다.
	unsigned char	majFlag;  // major 장애 발생되면 setting된다. 등급이 하강하는 경우 해지 메시지를 보내야 하는지 결정하는데 참고된다.
	unsigned char	criFlag;  // critical 장애 발생되면 setting된다. 
} Threshold_st_int;
/* added by dcham 20110525 */
typedef struct {
	unsigned int    minLimit;  // minor 장애 임계값 -> 이 값을 초과하여 duration 이상 지속되면 장애로 판정한다.
    unsigned int    majLimit;  // major 장애 임계값
    unsigned int    criLimit;  // critical 장애 임계값
 
	unsigned char	minFlag;  // minor 장애 발생되면 setting된다. 등급이 하강하는 경우 해지 메시지를 보내야 하는지 결정하는데 참고된다.
	unsigned char	majFlag;  // major 장애 발생되면 setting된다. 등급이 하강하는 경우 해지 메시지를 보내야 하는지 결정하는데 참고된다.
	unsigned char	criFlag;  // critical 장애 발생되면 setting된다. 
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

/* LOGON 성공율 감시. added by uamyd 20110210 */
#define SFM_LOGON_FILE  "DATA/logon_success_rate_file"
#define SFM_SM_CONN_FILE  "DATA/sm_conn_sts_file"

#endif /*__SFMCONF_H__*/
