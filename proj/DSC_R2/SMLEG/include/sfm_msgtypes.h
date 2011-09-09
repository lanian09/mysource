#ifndef __SFM_MSGTYPES_H__
#define __SFM_MSGTYPES_H__

#include <netinet/in.h>
#include <ipaf_stat.h>

#include <sfmconf.h>

#pragma pack(1)
//------------------------------------------------------------------------------

// 주기적으로 OMP의 FIMD로 공통적인 정보(mem, cpu, disk, lan, process)를 전달하기 위한 데이타 구조
#pragma pack(1)
typedef struct {
    char    diskName[SFM_MAX_DISK_NAME];
    unsigned short  disk_usage;
} SFM_SysCommDiskSts;
#pragma pack()

#pragma pack(1)
typedef struct {
    char    processName[COMM_MAX_NAME_LEN];
    unsigned char   status;
    unsigned char   level;
    pid_t           pid;
    time_t          uptime;
} SFM_SysCommProcSts;
#pragma pack()

#pragma pack(1)
typedef struct {
/*    char    target_IPadress[SFM_MAX_TARGET_IP_LEN];*/
    unsigned int target_IPaddress;
    char    target_SYSName[SFM_MAX_LAN_NAME_LEN];
    unsigned char   status;
} SFM_SysCommLanSts;
#pragma pack()

#pragma pack(1)
typedef struct {
	int				qID;
	int				qKEY;
	char			qNAME[QUE_MAX_NAME_LEN];
	unsigned int	qNUM;
	unsigned int	cBYTES;
	unsigned int	qBYTES;
} SFM_SysCommQueSts;
#pragma pack()

typedef struct {
    ushort      req;
    ushort      succ;
} SFM_CallInfo;

#pragma pack(1)
typedef struct {
    char    linkName[SFM_MAX_LAN_NAME_LEN];
    unsigned char   status;
} SFM_SysCommLinkSts;
#pragma pack()

#pragma pack(1)
typedef struct {
	unsigned char   sIpAddress[16];
	unsigned char   sDbPort[8];
	unsigned char   sDbUserName[16];
	unsigned char   sDbPassword[16];
	unsigned int    iStatus; /* 1 - connect, 0 - not connect */
    unsigned int    uiTxnLogId;  
	unsigned char   sDBAlias[20];
} SFM_SysDBConnSts;
#pragma pack()

/** System duplication Information **/
#pragma pack(1)
typedef struct {
	unsigned char 		myLocalDupStatus;   /* 1 : ACTIVE, 2 : STANDBY */
	unsigned char 		yourDupStatus;      /* 1 : ACTIVE, 2 : STANDBY */
	unsigned char 		uawapFlag;			/* 1 : ON,	   2 : OFF     */
	unsigned char 		szMin[17];		
	long long  			llAcctSessID;
	unsigned int    	uiKey;
	unsigned int 		uiUdrSequence;
    SFM_SysDBConnSts   	RmtDbSts[3];
	unsigned int		heartbeatAlarm;  	/* 1 : Normal, 2 : AbNormal(alarm) */
	unsigned int        bsdmTimeoutAlarm;   /* 1 : Normal, 2 : AbNormal(alarm) ,DSC와의 size 문제로 추가 sjjeon*/
	unsigned int		OosAlarm;   		/* 1 : Normal, 2 : Out-of-Service occured(alarm) */
	unsigned char 		mirr_a_status;      /* 0 : No input packet, 1 :input packet */
	unsigned char		mirr_b_status;
	unsigned char 		dual_active;		/* dual-active flag, 
											   0 : none-dual active 
											   1 : dual active **/
	unsigned char 		default_active;		/* default-active flag
											   0 : non default-active
											   1 : default-active 
											 */
} SFM_SysDuplicationSts;
#pragma pack()

/* Index for Success Rate */
#define SUCC_RATE_UAWAP_ITR		0
#define SUCC_RATE_AAA_ITR		1
#define SUCC_RATE_WAP1_ANAL		2
#define SUCC_RATE_WAP2_ANAL		3
#define SUCC_RATE_HTTP_ANAL		4
#define SUCC_RATE_VODS_ANAL		5
#define SUCC_RATE_ANAAA_ANAL	6
#define SUCC_RATE_VT_ANAL		7
#pragma pack(1)
typedef struct{
	unsigned int	count; /* count occured */
	unsigned int	rate;  /* success rate, unit:percent */
}SFM_SysSuccessRate;
#pragma pack()

#pragma pack(1)
typedef struct{
	unsigned int	ipAddr;
	unsigned int	count; /* count occured */
	unsigned char	rate;  /* success rate, unit:percent */
}SFM_SysSuccRateIP;
#pragma pack()

/* Session load */
#define SESSION_LOADINDEX_UPRESTO       0 /* the others is reserved */

#pragma pack(1)
typedef struct{
	unsigned char	bondName[COMM_MAX_NAME_LEN];
	unsigned char	status;
}SFM_SysIFBond;
#pragma pack()

// by helca //
#pragma pack(1)
typedef struct{
	unsigned char	StsName[COMM_MAX_NAME_LEN];
	unsigned char	status;
}SFM_fanSts;
#pragma pack()

#pragma pack(1)
typedef struct{
	unsigned char	StsName[COMM_MAX_NAME_LEN];
	unsigned char	status;
}SFM_pwrSts;
#pragma pack()

#pragma pack(1)
typedef struct{
	unsigned char	StsName[COMM_MAX_NAME_LEN];
	unsigned char	status;
}SFM_linkSts;
#pragma pack()

#pragma pack(1)
typedef struct{
	unsigned char	StsName[COMM_MAX_NAME_LEN];
	unsigned char	status;
}SFM_cpuSts;
#pragma pack()

#pragma pack(1)
typedef struct{
	unsigned char	StsName[COMM_MAX_NAME_LEN];
	unsigned char	status;
}SFM_diskSts;
#pragma pack()


#pragma pack(1)

typedef struct{
	unsigned char	StsName[COMM_MAX_NAME_LEN];		/**< hw name **/
	unsigned char	StsInfo[COMM_MAX_NAME_LEN];		/**< inteface 경우 ip address **/

#define SFM_HW_STATUS_ALIVE      0
#define SFM_HW_STATUS_DEAD      1
	unsigned char	status;							/**< status {0: alive, 1: dead} **/
// stsType
#define SFM_HW_STATUS_TYPE      0
#define SFM_SW_STATUS_TYPE      1
	unsigned char   stsType;                        /**< H/W: 0, S/W: 1, etc .. **/
}SFM_HWSts;
#pragma pack()


/* HW 상태 정보 */
#pragma pack(1)
typedef struct{
	SFM_diskSts		diskSts[SFM_HW_MAX_DISK_NUM];
	SFM_cpuSts		cpuSts[SFM_HW_MAX_CPU_NUM];
	SFM_linkSts		linkSts[SFM_HW_MAX_LINK_NUM];
	SFM_pwrSts		pwrSts[SFM_HW_MAX_PWR_NUM];
	SFM_fanSts		fanSts[SFM_HW_MAX_FAN_NUM];
}SFM_SysSts;
#pragma pack()
//------------------//

#pragma pack(1)
typedef struct {

	unsigned int svcType;
}SFM_Duia;
#pragma pack()

#define SFM_MAX_RSRC_LOAD_CNT   16
#define MAX_MP_NUM1				1

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
    #define DEF_MMDB_PCDR       14 
    #define DEF_MMDB_VT       	15 
	unsigned int    rsrcload[SFM_MAX_RSRC_LOAD_CNT][MAX_MP_NUM1];
}SFM_SysRsrcLoad;
#pragma pack()

#pragma pack(1)
typedef struct {
	char    processName[COMM_MAX_NAME_LEN];
	unsigned char   status;
	unsigned char   level;
	pid_t           pid;
	time_t          uptime;
} SFM_SysCommSdmdPrcSts;
#pragma pack()

#define CONNECTED       1
#define DISCONNECTED    0
#pragma pack(1)
typedef struct {
   	unsigned char       	lanCount;
   	unsigned char			rmtlanCount;
	unsigned char       	processCount;
   	unsigned char       	cpuCount;
   	unsigned char       	diskCount;
   	unsigned char       	queCount;
	unsigned char       	hwNTP[MAX_HW_NTP];
   	unsigned short      	mem_usage;
   	unsigned short      	cpu_usage[SFM_MAX_CPU_CNT]; 
	unsigned short			sess_load[SFM_MAX_SESSION_CNT]; 
   	unsigned short          total_disk_usage;
	SFM_SysCommProcSts		loc_process_sts[SFM_MAX_PROC_CNT];
   	SFM_SysCommDiskSts		loc_disk_sts[SFM_MAX_DISK_CNT];
   	SFM_SysCommLanSts		loc_lan_sts[SFM_MAX_LAN_CNT];
	SFM_SysCommLanSts		rmt_lan_sts[SFM_MAX_RMT_LAN_CNT];
   	SFM_SysCommQueSts		loc_que_sts[SFM_MAX_QUE_CNT];
   	SFM_SysCommLinkSts		loc_link_sts[SFM_MAX_DEV_CNT];
	SFM_SysDuplicationSts	loc_system_dup;
	SFM_SysSuccessRate		succ_rate[SFM_MAX_SUCC_RATE_CNT];
	SFM_SysIFBond			IF_bond[MAX_BOND_NUM];
   	SFM_SysRsrcLoad     	rsrc_load;
	SFM_SysSts				sysSts;
	SFM_HWSts				sysHW[SFM_MAX_HPUX_HW_COM];		// 40
	SFM_Duia				duia;
	LEG_SM_CONN				smConn[MAX_RLEG_CNT+1];			// SM CONNECTION STATUS ON:1, OFF:0
	unsigned char			cps_over_alm_flag; 				// CPS 알람 체크 
															// 0: CLEARED, 1:OCCURRED 
} SFM_SysCommMsgType;
#pragma pack()

//------------------------------------------------------------------------------

#pragma pack(1)
typedef struct {
    unsigned char   status;
    unsigned char   level;
	char			name[COMM_MAX_NAME_LEN];
} SFM_SpecInfoHpUxHWSts;
#pragma pack()

#pragma pack(1)
typedef struct {
    SFM_SpecInfoHpUxHWSts hwcom[SFM_MAX_HPUX_HW_COM];
} SFM_SpecInfoHpUxHWMsgType;
#pragma pack()


//------------------------------------------------------------------------------

#pragma pack(1)
typedef struct {
	unsigned char   connSts;	
	char			name[COMM_MAX_NAME_LEN];  // system type
} IxpcCon;
#pragma pack()

#pragma pack(1)
typedef struct {
	IxpcCon			ixpcCon[SYSCONF_MAX_ASSO_SYS_NUM];
} IxpcConSts;
#pragma pack()

#pragma pack(1)
typedef struct {
    char    target_IPadress[SFM_MAX_TARGET_IP_LEN];
	char    target_Name[SFM_MAX_LAN_NAME_LEN];
	unsigned char   status;
	unsigned char   level;
} SFM_SpecInfoLanMsgType;
#pragma pack()

#pragma pack(1)
typedef struct {
    unsigned char   status;
    #define          SFM_DB_REPL_STOP        0   /* replication이 stop된 상태 : 비정상 상태임 */
    #define          SFM_DB_REPL_STARTED     1   /* replication이 start된 상태 : 정상 상태임 */
    #define          SFM_DB_REPL_SYNC_RUN    2   /* 전체 table sync중... : 정상 상태임. replGap이 의미 없음 */
    #define          SFM_DB_REPL_GIVE_UP     3   /* replication give_up됨. replGap 의미 없음 */
	unsigned char   level;  
	unsigned int    replGap;                    /* status SFM_DB_REPL_STARTED인 상태에서만 참조한다. */
} SFM_SpecInfoDBSyncMsgType;
#pragma pack()

#pragma pack(1)
typedef struct {
	unsigned char               lanCount;
	unsigned char               active;
	SFM_SpecInfoLanMsgType      netMon[SFM_MAX_NET_MON_CNT];
	SFM_SpecInfoDBSyncMsgType   dbSync;
} SFM_SpecInfoMsgType;
#pragma pack()

#if 0 
#pragma pack(1)
typedef struct {
    int         cnt;            // Client IP 갯수 
    TcpConnInfo srv[MAX_CONN_IP];
} SFM_TcpConnInfoMsg;
#pragma pack()

/* value of grpNum : in struct ServerInfo */
#define SYS_SMSC        1 
#define SYS_VAS         2 
#define SYS_LMSC        3
#define SYS_IDR         4
#define SYS_SCP         5
#define SYS_WISE        6
#endif

#pragma pack(1)
typedef struct {
    in_addr_t       ipAddr;     // Server IP 주소
    short           port;       // Rx or Tx port
    unsigned char   grpNum;     // 연동 시스템 유형 식별자 (SMSC, ...)
    unsigned char   type;        // 연동 시스템 식별자 (SMSC1~n, ...)
    short           connNum;    // Connection 갯수
    short           maxConn;    // Connection 갯수
} TcpConnInfo;
#pragma pack()


/*  Update to TcpConnInfo.grpNum    */
#define         TCPINFO_SMSC        1
#define         TCPINFO_VAS         2
#define         TCPINFO_LMSC        3
#define         TCPINFO_IDR         4
#define         TCPINFO_SCP         5
#define         TCPINFO_WISE        6
#define         TCPINFO_BACKUP      7

/* Update to TcpConnInfo.nId        */
#define      TCPINFO_SUB_TYPE       1
#define      TCPINFO_BILL_TYPE      2
#define      TCPINFO_SCPIF_TYPE     3
#define      TCPINFO_WISE_TYPE      4
#define      TCPINFO_BKUP_TYPE      5

/* 2006.02.38 KHL for HP */
typedef struct {
#define  MAX_TCP_CONN_NUM   200
    TcpConnInfo data[MAX_TCP_CONN_NUM];
} SFM_ConCommMsgType;

#pragma pack()
#endif /* __SFM_MSGTYPES_H__*/
