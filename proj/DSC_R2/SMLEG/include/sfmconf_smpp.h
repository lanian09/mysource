//------------------------------------------------------------------------------
// file name : sfmconf.h
// project name : LGT SCP 대개체
// desc : Status & Fault Management 관련된 구조를 선언한다. 
// history 
// -----------------------------------------------------------------------------
// 2004.07.23 #pragma pack(1)을 제거 함. -- Solaris 64 <-> 32 bit 호환문제
//		- OMP에서만 사용되는 상태관리 typedef에 앞에 SFM을 추가함.
//------------------------------------------------------------------------------
#ifndef __SFMCONF_H__
#define __SFMCONF_H__

#include <netinet/in.h>

#include "sysconf.h"
#include "comm_msgtypes.h"

// 각 Component별 최대 실장 갯수
//
#define	SFM_MAX_CPU_CNT			16 // yhshin 4
#define	SFM_MAX_DISK_CNT		8
#define	SFM_MAX_LAN_CNT			40
#define	SFM_MAX_PROC_CNT		SYSCONF_MAX_APPL_NUM
#define	SFM_MAX_PROC_NAME		COMM_MAX_NAME_LEN	
#define	SFM_MAX_DISK_NAME		COMM_MAX_NAME_LEN
#define	SFM_MAX_LAN_NAME_LEN	COMM_MAX_NAME_LEN
#define	SFM_MAX_TARGET_IP_LEN	16

// status  장비 H/W 정보
#define		SFM_MAX_HW_CNT	40		//시스템에 설치된 Device 장치
#define		SFM_MAX_HW_LAN_CNT	4
#define   SFM_HW_PATTEERN_CNT		SFM_MAX_HW_CNT*2

#define   SFM_ALARMINFO_CNT			100


//-- CRBT ONLY START
//BKUP CRBT OMP에서 만 사용된다. BKUP SRV에 최대로 연결되는 OPERATION SRV 수 
#define SFM_MAX_SYNC_OP_CNT		10
//BKUP CRBT OMP에서 만 사용된다. (Primary , Secondary)
#define SFM_MAX_SYNC_CONN_CNT	2

// BKUP_SRV의 WORK MODE
#define SFM_BACKUP_MODE			0		
#define SFM_OPER_MODE			1

//IPI_CONN MAX Connection Count , MP 한쪽 사이트의 갯수 임 MP1 와 MP2를 합하면 400임.
#define SFM_MAX_IPI_CONN_CNT	200
#define SFM_MAX_CONN_DESC_LEN	COMM_MAX_NAME_LEN

// CDSI , CPSI 
#define SFM_MAX_CXSI_CONN_CNT	2
//-- CRBT ONLY END

// 장애 유형을 구분하기 위한 값
// - 정의된 값은 특별한 의미를 갖지 않고, 단순히 서로를 구분하는 용도로만 사용되고
//  key값이나 index 등의 용도로 사용되지 않는다.
// - alarm_history DB에 들어 있는 내용을 조회할때 사용된다.

// COMMON INFO ALM TYPE
#define SFM_ALM_TYPE_CPU_USAGE			1
#define SFM_ALM_TYPE_MEMORY_USAGE		2
#define SFM_ALM_TYPE_DISK_USAGE			3
#define SFM_ALM_TYPE_LAN				4
#define SFM_ALM_TYPE_PROC				5
#define SFM_ALM_TYPE_HW					6
// SPEC INFO ALM TYPE 



// SYSTYPE SPEC INFO
#define SFM_TYPE_CRBT_INFO_IPI_CONN_STS			0
#define SFM_TYPE_CRBT_INFO_CDSI_CONN_STS		(SFM_TYPE_CRBT_INFO_IPI_CONN_STS +1)
#define SFM_TYPE_CRBT_INFO_CPSI_CONN_STS		(SFM_TYPE_CRBT_INFO_CDSI_CONN_STS +1)
#define SFM_TYPE_CRBT_INFO_IPI_CONN_CONF		(SFM_TYPE_CRBT_INFO_CPSI_CONN_STS +1)
#define SFM_MAX_TYPE_CRBT_INFO_CNT				(SFM_TYPE_CRBT_INFO_IPI_CONN_CONF +1)

#define SFM_TYPE_CRBT_INFO_IPI_CONN_STR			"IPI"
#define SFM_TYPE_CRBT_INFO_CDSI_CONN_STR		"CDSI"
#define SFM_TYPE_CRBT_INFO_CPSI_CONN_STR		"CPSI"


// CRBT ONLY
#define SFM_ALM_TYPE_IPI_CONN			6
#define SFM_ALM_TYPE_CDSI_CONN			7
#define SFM_ALM_TYPE_CPSI_CONN			8
#define SFM_ALM_TYPE_SYNCI_CONN			9		//OP_IPS
#define SFM_ALM_TYPE_SYNCB_CONN			10		//BACKUP_OMP

//장애 발생여부
#define SFM_ALM_OCCURED		1
#define SFM_ALM_CLEARED		0

// 장애 등급
#define	SFM_ALM_NORMAL		0
#define	SFM_ALM_MINOR		1
#define	SFM_ALM_MAJOR		2
#define	SFM_ALM_CRITICAL	3

// Alarm Masked 여부 
#define SFM_ALM_UNMASKED	0	//100 -> 0
#define	 SFM_ALM_MASKED		1	//99 -> 1


// 프로세스 등 일반적 콤포넌트의 상태
#define SFM_STATUS_ALIVE	0
#define SFM_STATUS_DEAD		1

// LAN 상태
#define SFM_LAN_CONNECTED		0
#define SFM_LAN_DISCONNECTED	1

#define SFM_IPI_CONN_NAME			"IPI"
#define SFM_CDSI_CONN_NAME			"CDSI"
#define SFM_CPSI_CONN_NAME			"CPSI"
#define SFM_SYNCI_CONN_NAME			"SYNCI"
#define SFM_SYNCB_CONN_NAME			"SYNCB"


//------------------------------------------------------------------------------
/* 한개의 장비를 구별 하는 ID정보 */
typedef struct{
	int			sysId;	      					// System Id 즉, Sysconfig에 새로운 섹션으로 정의(int)
	char		sysType[COMM_MAX_NAME_LEN];		// System Type (int) ex)
	char		sysName[COMM_MAX_NAME_LEN];		// Host Name 
}SFM_SystemId;

/* 장비 전체의 장애 정보 - 각 시스템에 대한 현재 발생되어 있는 등급별 장애 갯수와 전체 장애 등급*/
typedef struct{
	unsigned char		level;			// 현재 시스템의 전체 장애 등급
	unsigned char		prevLevel;	// 이전 시스템의 전체 장애 등급
	unsigned char		minCnt;			// 현재 발생되어 있는 minor 장애 등급 갯수
	unsigned char		majCnt;			// 현재 발생되어 있는 major 장애 등급 갯수
	unsigned char		criCnt;			// 현재 발생되어 있는 critical 장애 등급 갯수
}SFM_AlarmInfo;

/* 한 장비의 특화 설정 정보 - 시스템 유형별로 특성에 따라 다르게 관리되어야 하는 상태 정보
* 하위 정보 구성은 -- 미정 -- INIP NOT USE*/
typedef struct{
	union{
       char dummy;
	}u;
}SFM_SpecConfig;


/* ---------------------------------------------------------------------------------------------- */


/* ---------------------------------------------------------------------------------------------- */
// Common Config Information

/* 1.1. 한개의 장비에 대한 Cpu 설정 정보 */
typedef struct{
	unsigned char	cnt;  			// 현재 관리하는 Cpu 갯수
	unsigned char	minLimit;		// minor    장애 임계값 -> 이 값을 초과하여 duration 이상 지속되면 장애로 판정한다.
	unsigned char	majLimit;		// major    장애 임계값
	unsigned char	criLimit;		// critical 장애 임계값
	unsigned char	minDurat;		// minor duration 
	unsigned char	majDurat;		// major duration
	unsigned char	criDurat;		// critical duration 
}SFM_CpuConfig;

/* 1.2. 한개의 장비에 대한 Memory 설정 정보 */
typedef struct{
	unsigned char	minLimit;		// minor 장애 임계값 -> 이 값을 초과하여 duration 이상 지속되면 장애로 판정한다.
	unsigned char	majLimit;		// major 장애 임계값	
	unsigned char	criLimit;		// critical 장애 임계값
	unsigned char	minDurat;
	unsigned char	majDurat;
	unsigned char	criDurat;
}SFM_MemConfig;

/* 1.3. 한개의 장비에 대한 Disk 설정 정보 */
/* 한개 disk 별 설정 정보 */
typedef struct{
	char			name[SFM_MAX_DISK_NAME];	// disk partition name
	unsigned char	minLimit;					// minor    장애 임계값
	unsigned char	majLimit;					// major    장애 임계값
	unsigned char	criLimit;					// critical 장애 임계값
}SFM_DiskConfigItem;

typedef struct{
	unsigned char		cnt;						//  관리하는 Disk 갯수
	SFM_DiskConfigItem	item[SFM_MAX_DISK_CNT];		// 각 disk별 설정정보 list(SFM_DiskConfigItem)
}SFM_DiskConfig;

/* 1.4. 한개의 장비에 대한 Process 설정 정보 */
/* 한개 Process 별 설정 정보 */
typedef struct{
	char			name[SFM_MAX_PROC_NAME];		// process name
	unsigned char	baseLevel;						// 장애 발생 시 해당 프로세스에 적용될 장애 등급
}SFM_ProcConfigItem;

typedef struct{
	unsigned char		cnt;						   // 관리하는 Process 갯수(unsigned char)
	SFM_ProcConfigItem	item[SFM_MAX_PROC_CNT];		   // 각 Process별 설정 정보 list(SFM_ProcConfigItem)
}SFM_ProcConfig;

/* 1.5. 한개의 장비에 대한 LAN 설정 정보 */

/* 한개 Lan 별 설정 정보 */
typedef struct{
//	char			name[SFM_MAX_LAN_NAME_LEN];			// Lan connection Name
//	char			targetIp[SFM_MAX_TARGET_IP_LEN];	// connected ip address
	unsigned char	baseLevel;							// 장애 발생 시 해당 프로세스에 적용될 장애 등급
}SFM_LanConfigItem;

typedef struct{
	unsigned char		cnt;							// 관리하는 Lan 갯수(unsigned char)
	SFM_LanConfigItem	item[SFM_MAX_LAN_CNT];			// 각 Lan별 설정 정보 list(SFM_LanConfigItem)
}SFM_LanConfig;

/* 한개의 장비에 대한 공통 설정 정보 */
typedef struct{
	SFM_CpuConfig		cpuConfig;		// 1.1. CPU 설정 정보
	SFM_MemConfig		memConfig;		// 1.2. Memory 설정 정보
	SFM_DiskConfig		diskConfig;		// 1.3. Disk 설정 정보
	SFM_ProcConfig		procConfig;		// 1.4. Proccess 설정 정보 
	SFM_LanConfig			lanConfig;
}SFM_CommConfig;
/* ---------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------- */

/* 2.1 한개의 장비에 대한 Cpu 상태 정보 */
/* Cpu 별 상태 정보 */
typedef struct{
	unsigned short		usage;			// 현재 cpu 사용율
	unsigned char		level;			// 현재 CPU 장애 등급
	unsigned char		maskFlag;		// alarm maks flag ex)SFM_ALM_MASKED | SFM_ALM_UNMASKED
	time_t				when[SFM_ALM_CRITICAL];			// 장애 발생 시각
}SFM_CpuStateItem;

typedef struct{
	unsigned char		cnt;					// 관리하는 cpu 갯수
	unsigned char		level;					// cpu 전체 장애 등급
	SFM_CpuStateItem	item[SFM_MAX_CPU_CNT];  // 각 cpu별 상태 정보
}SFM_CpuState;

/* 2.2 한개의 장비에 대한 Memory 상태 정보 */
typedef struct{
	unsigned short		usage;			// 현재 memory 사용율
	unsigned char		level;			// 현재 memory 장애 등급
	unsigned char		maskFlag;		// alarm maks flag ex)SFM_ALM_MASKED | SFM_ALM_UNMASKED
	time_t				when[SFM_ALM_CRITICAL];			// 장애 발생 시각
}SFM_MemState;

/* 2.3 한개의 장비에 대한 Disk 상태 정보 */
/* Disk 별 상태 정보 */
typedef struct{
	unsigned short		usage;			// disk usage
	unsigned char		level;			// 현재 장애 등급
	unsigned char		maskFlag;		// alarm maks flag ex)SFM_ALM_MASKED | SFM_ALM_UNMASKED
	time_t				when[SFM_ALM_CRITICAL];			// 장애 발생 시각
}SFM_DiskStateItem;

typedef struct{
	unsigned char		cnt;					// 관리하는 Disk 갯수
	unsigned char		level;					// disk 전체 장애 등급
	SFM_DiskStateItem	item[SFM_MAX_DISK_CNT]; // 각 Disk 상태 정보
}SFM_DiskState;

/* 2.4 한개의 장비에 대한 Process 상태 정보 */

/* Process 별 상태 정보 */
typedef struct{
	unsigned char		status;			// 현재 상태
	unsigned char		prevStatus;		// 이전 상태
	unsigned char		level;			// 현재 장애 등급
	unsigned char		maskFlag;		// alarm maks flag ex)SFM_ALM_MASKED | SFM_ALM_UNMASKED
	time_t				when;			// 장애 발생 시각
}SFM_ProcStateItem;

typedef struct{
	unsigned char		cnt;					// 관리하는 process 갯수
	unsigned char		level;					// process 전체 장애 등급
	SFM_ProcStateItem	item[SFM_MAX_PROC_CNT];	// 각 Process 상태 정보
}SFM_ProcState;

//2006/9/2 choi hye kyung ADD
typedef struct{
	unsigned char		status;			// 현재 상태
	unsigned char		prevStatus;		// 이전 상태
	unsigned char		level;			// 현재 장애 등급
	unsigned char		maskFlag;		// alarm maks flag ex)SFM_ALM_MASKED | SFM_ALM_UNMASKED
	time_t				when;			// 장애 발생 시각
}SFM_HWStateItem;


typedef struct{
	unsigned char		cnt;					// 관리하는 HW 장비 갯수
	unsigned char		level;					// HW 전체 장애 등급
	SFM_HWStateItem	item[SFM_MAX_HW_CNT];	// 각 HW 장비 상태 정보
}SFM_HWState;



/* 각 Lan 별 상태 정보(실시간 처리를 위해 IXPC 로 보낼 정보 데이터) */
typedef struct{
	//unsigned char		prevStatus;	// 이전 상태
	//unsigned char		level;				// 현재 장애 등급
	//unsigned char		maskFlag;		// alarm maks flag ex)SFM_ALM_MASKED | SFM_ALM_UNMASKED
	char    target_IPadress[SFM_MAX_TARGET_IP_LEN];
	char    target_SYSName[SFM_MAX_LAN_NAME_LEN];
	char    target_proName[SFM_MAX_PROC_NAME];
	int port;
	time_t				when;			// 장애 발생 시각
	unsigned char		status;			// 현재 상태

}IXPC_LanStateItem;

typedef struct{
	unsigned char	cnt;
	IXPC_LanStateItem tcpcon[SFM_MAX_LAN_CNT];
}TCP_ConTbl;

/* 2.5 한개의 장비에 대한 Lan 상태 정보 */

/* Lan 별 상태 정보 */
typedef struct{
	unsigned char		status;			// 현재 상태
	unsigned char		prevStatus;		// 이전 상태
	unsigned char		level;			// 현재 장애 등급
	unsigned char		maskFlag;		// alarm maks flag ex)SFM_ALM_MASKED | SFM_ALM_UNMASKED
   char    target_IPadress[SFM_MAX_TARGET_IP_LEN];
   char    target_SYSName[SFM_MAX_LAN_NAME_LEN];
   char    target_proName[SFM_MAX_PROC_NAME];

   int port;
	time_t				when;			// 장애 발생 시각
}SFM_LanStateItem;

typedef struct{
	unsigned char		cnt;					// 관리하는 Lan 갯수
	unsigned char		level;					// lan 전체 장애 등급
	SFM_LanStateItem	item[SFM_MAX_LAN_CNT];	// 각 Lan 상태 정보
}SFM_LanState;


/* 한개의 장비에 대한 공통 상태 정보 */
typedef struct{
	unsigned char		level;		// 현재 장애 등급
	SFM_AlarmInfo		commAlmInfo;
	SFM_CpuState		cpuState;	// 2.1. CPU 상태 정보
	SFM_MemState		memState;	// 2.2. Memory 상태 정보
	SFM_DiskState		diskState;	// 2.3. Disk 상태 정보
	SFM_ProcState		procState;	// 2.4. Proccess 상태 정보
	SFM_LanState	lanState;   // 2.5. LAN 상태 정보
	SFM_HWState		hwState;
}SFM_CommState;



//------------------------------------------------------------------------------
// CRBT ONLY


// Sync Connection 관련 SpecInfo
typedef struct{
	in_addr_t	 		connIp;		//connected IP address
	unsigned char		preStatus;	
	unsigned char		status;		// Connection Status [Connected | Disconnected]
	unsigned char		level;
	unsigned char		maskFlag;		// alarm maks flag ex)SFM_ALM_MASKED | SFM_ALM_UNMASKED
	time_t				when;			// 장애 발생 시각
} SFM_MGT_SyncConnInfo_s;

typedef struct{
	unsigned char				level;
	SFM_MGT_SyncConnInfo_s		sync[SFM_MAX_SYNC_CONN_CNT];
} SFM_MGT_SYNCI_ConnInfo;	

typedef struct{
	unsigned char				dstId;
	unsigned char				level;
	SFM_MGT_SyncConnInfo_s		sync[SFM_MAX_SYNC_CONN_CNT];	
} SFM_MGT_SYNCB_ConnInfo_s;

typedef struct{
	unsigned char				cnt;					
	unsigned char				level;	
	SFM_MGT_SYNCB_ConnInfo_s	info[SFM_MAX_SYNC_OP_CNT];
} SFM_MGT_SYNCB_ConnInfo;

typedef struct{
	unsigned char				preWorkId;	// SFM_OPER_MODE  = dstId , SFM_BACKUP_MODE = 1
	unsigned char				workId;		// SFM_OPER_MODE  = dstId , SFM_BACKUP_MODE = 1
	unsigned char				preWorkMode; //이전 WorkMode
	unsigned char				workMode;	// 현재 WorkMode
} SFM_SYNCB_Conf;

typedef struct {
    int    total;
    int    fail;
} SFM_TransLoad;

/* Real Time Traffic Information */
typedef struct {
    SFM_TransLoad   cdsi;
    SFM_TransLoad   cpsi;
    SFM_TransLoad   ipi;
} SFM_TrafficInfo;

//------------------------------------------------------------------------------
// 한 장비의 특화 상태 정보 - 시스템 유형별로 특성에 따라 다르게 관리되어야 하는 상태 정보
// 하위 정보 구성은
typedef struct{
	unsigned char			level;	
	SFM_AlarmInfo			specAlmInfo;
	SFM_TrafficInfo			traffic;

#ifdef OP_IPS
	SFM_MGT_SYNCI_ConnInfo	synci;
#endif
#ifdef BK_IPS
	SFM_MGT_SYNCB_ConnInfo	syncb;
#endif
}SFM_SpecState;

/* 한개의 장비에 대한 정보 최상위 그룹 */
typedef struct{
//	SFM_SystemId		systemId;		  	// 장비 구분 정보(SFM_DeviceId)
//	SFM_CommConfig		commConfig;		// 공통 설정 정보(SFM_CommConfig)
	SFM_CommState	  	commState;		// 공통 상태 정보(SFM_CommState)
	SFM_AlarmInfo	  	alarmInfo;		// 전체 장애 정보(SFM_AlarmInfo)
} SFM_SysInfo;



/* ---------------------------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------------------------- */
// CPU, Memory, Disk 등 등급 상승,하강 기능이 있는 장애 감지를 위한 structure

typedef struct {
	unsigned char	minFlag;  // minor 장애 발생되면 setting된다. 등급이 하강하는 경우 해지 메시지를 보내야 하는지 결정하는데 참고된다.
	unsigned char	majFlag;  // major 장애 발생되면 setting된다. 등급이 하강하는 경우 해지 메시지를 보내야 하는지 결정하는데 참고된다.
	unsigned char	criFlag;  // critical 장애 발생되면 setting된다. 등급이 하강하는 경우 해지 메시지를 보내야 하는지 결정하는데 참고된다.
} SFM_UsageAlmFlag_s;

typedef struct {
	SFM_UsageAlmFlag_s	cpu[SFM_MAX_CPU_CNT];
	SFM_UsageAlmFlag_s	mem;
	SFM_UsageAlmFlag_s	disk[SFM_MAX_DISK_CNT];
} SFM_UsageAlmFlag;
/* ---------------------------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------------------------- */
// 초기 전체 구성 정보
typedef struct{
	char				sysType[COMM_MAX_NAME_LEN];
	unsigned char		sysCnt;
	int					sysId[SYSCONF_MAX_SYS_TYPE_MEMBER];
	char				sysName[SYSCONF_MAX_SYS_TYPE_MEMBER][COMM_MAX_NAME_LEN];
} SFM_SystemTypeData;

typedef struct{
	unsigned char		totalCnt;	// 관리장비 전체 갯수 (VP-2, SP-2, OMP-1) 즉, 5개
	unsigned char		typeCnt;	// 장비 Type의 종류   (OMP,VP,SP) INIP -> 3
	SFM_SystemTypeData	typeData[SYSCONF_MAX_SYS_TYPE_NUM];  // 장비type별 하위장비 정보
} SFM_InitConfig;
/* ---------------------------------------------------------------------------------------------- */

//------------------------------------------------------------------------------
//-- CRBT ONLY START
// IPI 
// CDSI , CPSI Connection Info
typedef struct{
	unsigned short 	sysId;		//Source System Id	 [xxxx]
	in_addr_t	 	connIp;		//connected IP address
	unsigned char	status;		// Connection Status [Connected | Disconnected]
	char			desc[SFM_MAX_CONN_DESC_LEN];	//Connection Desc	
} SFM_ConnInfo;


// CDSI , CPSI ,IPI
typedef struct{
	unsigned short 		sysId;		//Source System Id	 [xxxx]
	in_addr_t	 		connIp;		//connected IP address
	int					connSysId;	//Connected System Id [MP1 | MP2]
	unsigned char		preStatus;	
	unsigned char		status;		// Connection Status [Connected | Disconnected]
	unsigned char		level;
	unsigned char		maskFlag;		// alarm maks flag ex)SFM_ALM_MASKED | SFM_ALM_UNMASKED
	time_t				when;			// 장애 발생 시각
	char				desc[SFM_MAX_CONN_DESC_LEN];	//Connection Desc	
} SFM_MGT_ConnInfo;

typedef struct{
	unsigned char			cnt;
	SFM_ConnInfo			info[SFM_MAX_IPI_CONN_CNT];
} SFM_MGT_IPISideConnInfo;

typedef struct{
	SFM_AlarmInfo			almInfo;
	unsigned char			cnt;
	SFM_MGT_ConnInfo		result[SFM_MAX_IPI_CONN_CNT];
	SFM_MGT_IPISideConnInfo	item[SFM_MAX_CXSI_CONN_CNT];
} SFM_MGT_IPI_ConnInfo;

typedef struct{
	SFM_AlarmInfo			almInfo;
	SFM_MGT_ConnInfo		result;
	SFM_ConnInfo			item[SFM_MAX_CXSI_CONN_CNT];
} SFM_MGT_CDSI_ConnInfo;

typedef struct{
	SFM_AlarmInfo			almInfo;
	SFM_MGT_ConnInfo		result;
	SFM_ConnInfo			item[SFM_MAX_CXSI_CONN_CNT];
} SFM_MGT_CPSI_ConnInfo;


//-- CRBT ONLY END
//------------------------------------------------------------------------------


/* ---------------------------------------------------------------------------------------------- */
//------------------------------------------------------------------------------
// 전체 관리 정보 구조체
//------------------------------------------------------------------------------
typedef struct {
	SFM_SysInfo				sys;
   SFM_UsageAlmFlag	usageAlmFlag;	// CPU,MEM,HDD 별 현재 장애등급
} SFM_sfdb;

//------------------------------------------------------------------------------
// MP에서 실시간으로 보내주는 Fault 정보구조체
//------------------------------------------------------------------------------
#pragma pack (1)
typedef struct {
	char   		alarmCode[8];		//A1001 ~
	int     	when;				//발생시간
	int     	mswhen;			/*** msec ***/
	int     	alarmLevel;		//알람등급
	int     	notiFlag;  /*** 0 : email, sms 보내지 않음, 1 : 보냄 ***/
	char    	rscName[16];	//대상 이름
	char    	Desc[80];		//부가정보
} T_AlarmReport;
#pragma pack ()

#pragma pack (1)
typedef struct{
	unsigned short      sysId;
	T_AlarmReport   alm_data;
}T_AlarmReportMsgType;
#pragma pack ()

typedef struct {
	char	sysName[8];
	T_AlarmReport	msg;
} T_SMPP_AlarmReport;
////////////////////////////////////////////////////////////////////
// 알람코드별 관리 정보(공유메모리와 파일로 관리)
typedef struct
{
  int    cnt;
  int     alarmCode[SFM_ALARMINFO_CNT];		
  int     alarmLevel[SFM_ALARMINFO_CNT];		
  int     notiFlag[SFM_ALARMINFO_CNT];  /*** 0 : email, sms 보내지 않음, 1 : 보냄 ***/
  int	    maskFlag[SFM_ALARMINFO_CNT];
  char   deviceName[SFM_ALARMINFO_CNT][16];
  char   desc[SFM_ALARMINFO_CNT][32];
 }SFM_AlarmCodeInfo;

////////////////////////////////////////////////////////////////////

#define SFM_SFDB_SIZE            sizeof(SFM_sfdb)
#define SFM_ALARMINFO_SIZE   sizeof(SFM_AlarmCodeInfo)
#define SFM_SFDB_FILE            "data/sfdb_file"
#define FIMD_CONFIG_FILE       "data/fimd_config"
#define SFM_ALARMINFO_FILE   "data/alarmInfo"
#endif // __SFMCONF_H__
