//------------------------------------------------------------------------------
// file name : sfmconf.h
// project name : LGT SCP �밳ü
// desc : Status & Fault Management ���õ� ������ �����Ѵ�. 
// history 
// -----------------------------------------------------------------------------
// 2004.07.23 #pragma pack(1)�� ���� ��. -- Solaris 64 <-> 32 bit ȣȯ����
//		- OMP������ ���Ǵ� ���°��� typedef�� �տ� SFM�� �߰���.
//------------------------------------------------------------------------------
#ifndef __SFMCONF_H__
#define __SFMCONF_H__

#include <netinet/in.h>

#include "sysconf.h"
#include "comm_msgtypes.h"

// �� Component�� �ִ� ���� ����
//
#define	SFM_MAX_CPU_CNT			16 // yhshin 4
#define	SFM_MAX_DISK_CNT		8
#define	SFM_MAX_LAN_CNT			40
#define	SFM_MAX_PROC_CNT		SYSCONF_MAX_APPL_NUM
#define	SFM_MAX_PROC_NAME		COMM_MAX_NAME_LEN	
#define	SFM_MAX_DISK_NAME		COMM_MAX_NAME_LEN
#define	SFM_MAX_LAN_NAME_LEN	COMM_MAX_NAME_LEN
#define	SFM_MAX_TARGET_IP_LEN	16

// status  ��� H/W ����
#define		SFM_MAX_HW_CNT	40		//�ý��ۿ� ��ġ�� Device ��ġ
#define		SFM_MAX_HW_LAN_CNT	4
#define   SFM_HW_PATTEERN_CNT		SFM_MAX_HW_CNT*2

#define   SFM_ALARMINFO_CNT			100


//-- CRBT ONLY START
//BKUP CRBT OMP���� �� ���ȴ�. BKUP SRV�� �ִ�� ����Ǵ� OPERATION SRV �� 
#define SFM_MAX_SYNC_OP_CNT		10
//BKUP CRBT OMP���� �� ���ȴ�. (Primary , Secondary)
#define SFM_MAX_SYNC_CONN_CNT	2

// BKUP_SRV�� WORK MODE
#define SFM_BACKUP_MODE			0		
#define SFM_OPER_MODE			1

//IPI_CONN MAX Connection Count , MP ���� ����Ʈ�� ���� �� MP1 �� MP2�� ���ϸ� 400��.
#define SFM_MAX_IPI_CONN_CNT	200
#define SFM_MAX_CONN_DESC_LEN	COMM_MAX_NAME_LEN

// CDSI , CPSI 
#define SFM_MAX_CXSI_CONN_CNT	2
//-- CRBT ONLY END

// ��� ������ �����ϱ� ���� ��
// - ���ǵ� ���� Ư���� �ǹ̸� ���� �ʰ�, �ܼ��� ���θ� �����ϴ� �뵵�θ� ���ǰ�
//  key���̳� index ���� �뵵�� ������ �ʴ´�.
// - alarm_history DB�� ��� �ִ� ������ ��ȸ�Ҷ� ���ȴ�.

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

//��� �߻�����
#define SFM_ALM_OCCURED		1
#define SFM_ALM_CLEARED		0

// ��� ���
#define	SFM_ALM_NORMAL		0
#define	SFM_ALM_MINOR		1
#define	SFM_ALM_MAJOR		2
#define	SFM_ALM_CRITICAL	3

// Alarm Masked ���� 
#define SFM_ALM_UNMASKED	0	//100 -> 0
#define	 SFM_ALM_MASKED		1	//99 -> 1


// ���μ��� �� �Ϲ��� ������Ʈ�� ����
#define SFM_STATUS_ALIVE	0
#define SFM_STATUS_DEAD		1

// LAN ����
#define SFM_LAN_CONNECTED		0
#define SFM_LAN_DISCONNECTED	1

#define SFM_IPI_CONN_NAME			"IPI"
#define SFM_CDSI_CONN_NAME			"CDSI"
#define SFM_CPSI_CONN_NAME			"CPSI"
#define SFM_SYNCI_CONN_NAME			"SYNCI"
#define SFM_SYNCB_CONN_NAME			"SYNCB"


//------------------------------------------------------------------------------
/* �Ѱ��� ��� ���� �ϴ� ID���� */
typedef struct{
	int			sysId;	      					// System Id ��, Sysconfig�� ���ο� �������� ����(int)
	char		sysType[COMM_MAX_NAME_LEN];		// System Type (int) ex)
	char		sysName[COMM_MAX_NAME_LEN];		// Host Name 
}SFM_SystemId;

/* ��� ��ü�� ��� ���� - �� �ý��ۿ� ���� ���� �߻��Ǿ� �ִ� ��޺� ��� ������ ��ü ��� ���*/
typedef struct{
	unsigned char		level;			// ���� �ý����� ��ü ��� ���
	unsigned char		prevLevel;	// ���� �ý����� ��ü ��� ���
	unsigned char		minCnt;			// ���� �߻��Ǿ� �ִ� minor ��� ��� ����
	unsigned char		majCnt;			// ���� �߻��Ǿ� �ִ� major ��� ��� ����
	unsigned char		criCnt;			// ���� �߻��Ǿ� �ִ� critical ��� ��� ����
}SFM_AlarmInfo;

/* �� ����� Ưȭ ���� ���� - �ý��� �������� Ư���� ���� �ٸ��� �����Ǿ�� �ϴ� ���� ����
* ���� ���� ������ -- ���� -- INIP NOT USE*/
typedef struct{
	union{
       char dummy;
	}u;
}SFM_SpecConfig;


/* ---------------------------------------------------------------------------------------------- */


/* ---------------------------------------------------------------------------------------------- */
// Common Config Information

/* 1.1. �Ѱ��� ��� ���� Cpu ���� ���� */
typedef struct{
	unsigned char	cnt;  			// ���� �����ϴ� Cpu ����
	unsigned char	minLimit;		// minor    ��� �Ӱ谪 -> �� ���� �ʰ��Ͽ� duration �̻� ���ӵǸ� ��ַ� �����Ѵ�.
	unsigned char	majLimit;		// major    ��� �Ӱ谪
	unsigned char	criLimit;		// critical ��� �Ӱ谪
	unsigned char	minDurat;		// minor duration 
	unsigned char	majDurat;		// major duration
	unsigned char	criDurat;		// critical duration 
}SFM_CpuConfig;

/* 1.2. �Ѱ��� ��� ���� Memory ���� ���� */
typedef struct{
	unsigned char	minLimit;		// minor ��� �Ӱ谪 -> �� ���� �ʰ��Ͽ� duration �̻� ���ӵǸ� ��ַ� �����Ѵ�.
	unsigned char	majLimit;		// major ��� �Ӱ谪	
	unsigned char	criLimit;		// critical ��� �Ӱ谪
	unsigned char	minDurat;
	unsigned char	majDurat;
	unsigned char	criDurat;
}SFM_MemConfig;

/* 1.3. �Ѱ��� ��� ���� Disk ���� ���� */
/* �Ѱ� disk �� ���� ���� */
typedef struct{
	char			name[SFM_MAX_DISK_NAME];	// disk partition name
	unsigned char	minLimit;					// minor    ��� �Ӱ谪
	unsigned char	majLimit;					// major    ��� �Ӱ谪
	unsigned char	criLimit;					// critical ��� �Ӱ谪
}SFM_DiskConfigItem;

typedef struct{
	unsigned char		cnt;						//  �����ϴ� Disk ����
	SFM_DiskConfigItem	item[SFM_MAX_DISK_CNT];		// �� disk�� �������� list(SFM_DiskConfigItem)
}SFM_DiskConfig;

/* 1.4. �Ѱ��� ��� ���� Process ���� ���� */
/* �Ѱ� Process �� ���� ���� */
typedef struct{
	char			name[SFM_MAX_PROC_NAME];		// process name
	unsigned char	baseLevel;						// ��� �߻� �� �ش� ���μ����� ����� ��� ���
}SFM_ProcConfigItem;

typedef struct{
	unsigned char		cnt;						   // �����ϴ� Process ����(unsigned char)
	SFM_ProcConfigItem	item[SFM_MAX_PROC_CNT];		   // �� Process�� ���� ���� list(SFM_ProcConfigItem)
}SFM_ProcConfig;

/* 1.5. �Ѱ��� ��� ���� LAN ���� ���� */

/* �Ѱ� Lan �� ���� ���� */
typedef struct{
//	char			name[SFM_MAX_LAN_NAME_LEN];			// Lan connection Name
//	char			targetIp[SFM_MAX_TARGET_IP_LEN];	// connected ip address
	unsigned char	baseLevel;							// ��� �߻� �� �ش� ���μ����� ����� ��� ���
}SFM_LanConfigItem;

typedef struct{
	unsigned char		cnt;							// �����ϴ� Lan ����(unsigned char)
	SFM_LanConfigItem	item[SFM_MAX_LAN_CNT];			// �� Lan�� ���� ���� list(SFM_LanConfigItem)
}SFM_LanConfig;

/* �Ѱ��� ��� ���� ���� ���� ���� */
typedef struct{
	SFM_CpuConfig		cpuConfig;		// 1.1. CPU ���� ����
	SFM_MemConfig		memConfig;		// 1.2. Memory ���� ����
	SFM_DiskConfig		diskConfig;		// 1.3. Disk ���� ����
	SFM_ProcConfig		procConfig;		// 1.4. Proccess ���� ���� 
	SFM_LanConfig			lanConfig;
}SFM_CommConfig;
/* ---------------------------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------------------------- */

/* 2.1 �Ѱ��� ��� ���� Cpu ���� ���� */
/* Cpu �� ���� ���� */
typedef struct{
	unsigned short		usage;			// ���� cpu �����
	unsigned char		level;			// ���� CPU ��� ���
	unsigned char		maskFlag;		// alarm maks flag ex)SFM_ALM_MASKED | SFM_ALM_UNMASKED
	time_t				when[SFM_ALM_CRITICAL];			// ��� �߻� �ð�
}SFM_CpuStateItem;

typedef struct{
	unsigned char		cnt;					// �����ϴ� cpu ����
	unsigned char		level;					// cpu ��ü ��� ���
	SFM_CpuStateItem	item[SFM_MAX_CPU_CNT];  // �� cpu�� ���� ����
}SFM_CpuState;

/* 2.2 �Ѱ��� ��� ���� Memory ���� ���� */
typedef struct{
	unsigned short		usage;			// ���� memory �����
	unsigned char		level;			// ���� memory ��� ���
	unsigned char		maskFlag;		// alarm maks flag ex)SFM_ALM_MASKED | SFM_ALM_UNMASKED
	time_t				when[SFM_ALM_CRITICAL];			// ��� �߻� �ð�
}SFM_MemState;

/* 2.3 �Ѱ��� ��� ���� Disk ���� ���� */
/* Disk �� ���� ���� */
typedef struct{
	unsigned short		usage;			// disk usage
	unsigned char		level;			// ���� ��� ���
	unsigned char		maskFlag;		// alarm maks flag ex)SFM_ALM_MASKED | SFM_ALM_UNMASKED
	time_t				when[SFM_ALM_CRITICAL];			// ��� �߻� �ð�
}SFM_DiskStateItem;

typedef struct{
	unsigned char		cnt;					// �����ϴ� Disk ����
	unsigned char		level;					// disk ��ü ��� ���
	SFM_DiskStateItem	item[SFM_MAX_DISK_CNT]; // �� Disk ���� ����
}SFM_DiskState;

/* 2.4 �Ѱ��� ��� ���� Process ���� ���� */

/* Process �� ���� ���� */
typedef struct{
	unsigned char		status;			// ���� ����
	unsigned char		prevStatus;		// ���� ����
	unsigned char		level;			// ���� ��� ���
	unsigned char		maskFlag;		// alarm maks flag ex)SFM_ALM_MASKED | SFM_ALM_UNMASKED
	time_t				when;			// ��� �߻� �ð�
}SFM_ProcStateItem;

typedef struct{
	unsigned char		cnt;					// �����ϴ� process ����
	unsigned char		level;					// process ��ü ��� ���
	SFM_ProcStateItem	item[SFM_MAX_PROC_CNT];	// �� Process ���� ����
}SFM_ProcState;

//2006/9/2 choi hye kyung ADD
typedef struct{
	unsigned char		status;			// ���� ����
	unsigned char		prevStatus;		// ���� ����
	unsigned char		level;			// ���� ��� ���
	unsigned char		maskFlag;		// alarm maks flag ex)SFM_ALM_MASKED | SFM_ALM_UNMASKED
	time_t				when;			// ��� �߻� �ð�
}SFM_HWStateItem;


typedef struct{
	unsigned char		cnt;					// �����ϴ� HW ��� ����
	unsigned char		level;					// HW ��ü ��� ���
	SFM_HWStateItem	item[SFM_MAX_HW_CNT];	// �� HW ��� ���� ����
}SFM_HWState;



/* �� Lan �� ���� ����(�ǽð� ó���� ���� IXPC �� ���� ���� ������) */
typedef struct{
	//unsigned char		prevStatus;	// ���� ����
	//unsigned char		level;				// ���� ��� ���
	//unsigned char		maskFlag;		// alarm maks flag ex)SFM_ALM_MASKED | SFM_ALM_UNMASKED
	char    target_IPadress[SFM_MAX_TARGET_IP_LEN];
	char    target_SYSName[SFM_MAX_LAN_NAME_LEN];
	char    target_proName[SFM_MAX_PROC_NAME];
	int port;
	time_t				when;			// ��� �߻� �ð�
	unsigned char		status;			// ���� ����

}IXPC_LanStateItem;

typedef struct{
	unsigned char	cnt;
	IXPC_LanStateItem tcpcon[SFM_MAX_LAN_CNT];
}TCP_ConTbl;

/* 2.5 �Ѱ��� ��� ���� Lan ���� ���� */

/* Lan �� ���� ���� */
typedef struct{
	unsigned char		status;			// ���� ����
	unsigned char		prevStatus;		// ���� ����
	unsigned char		level;			// ���� ��� ���
	unsigned char		maskFlag;		// alarm maks flag ex)SFM_ALM_MASKED | SFM_ALM_UNMASKED
   char    target_IPadress[SFM_MAX_TARGET_IP_LEN];
   char    target_SYSName[SFM_MAX_LAN_NAME_LEN];
   char    target_proName[SFM_MAX_PROC_NAME];

   int port;
	time_t				when;			// ��� �߻� �ð�
}SFM_LanStateItem;

typedef struct{
	unsigned char		cnt;					// �����ϴ� Lan ����
	unsigned char		level;					// lan ��ü ��� ���
	SFM_LanStateItem	item[SFM_MAX_LAN_CNT];	// �� Lan ���� ����
}SFM_LanState;


/* �Ѱ��� ��� ���� ���� ���� ���� */
typedef struct{
	unsigned char		level;		// ���� ��� ���
	SFM_AlarmInfo		commAlmInfo;
	SFM_CpuState		cpuState;	// 2.1. CPU ���� ����
	SFM_MemState		memState;	// 2.2. Memory ���� ����
	SFM_DiskState		diskState;	// 2.3. Disk ���� ����
	SFM_ProcState		procState;	// 2.4. Proccess ���� ����
	SFM_LanState	lanState;   // 2.5. LAN ���� ����
	SFM_HWState		hwState;
}SFM_CommState;



//------------------------------------------------------------------------------
// CRBT ONLY


// Sync Connection ���� SpecInfo
typedef struct{
	in_addr_t	 		connIp;		//connected IP address
	unsigned char		preStatus;	
	unsigned char		status;		// Connection Status [Connected | Disconnected]
	unsigned char		level;
	unsigned char		maskFlag;		// alarm maks flag ex)SFM_ALM_MASKED | SFM_ALM_UNMASKED
	time_t				when;			// ��� �߻� �ð�
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
	unsigned char				preWorkMode; //���� WorkMode
	unsigned char				workMode;	// ���� WorkMode
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
// �� ����� Ưȭ ���� ���� - �ý��� �������� Ư���� ���� �ٸ��� �����Ǿ�� �ϴ� ���� ����
// ���� ���� ������
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

/* �Ѱ��� ��� ���� ���� �ֻ��� �׷� */
typedef struct{
//	SFM_SystemId		systemId;		  	// ��� ���� ����(SFM_DeviceId)
//	SFM_CommConfig		commConfig;		// ���� ���� ����(SFM_CommConfig)
	SFM_CommState	  	commState;		// ���� ���� ����(SFM_CommState)
	SFM_AlarmInfo	  	alarmInfo;		// ��ü ��� ����(SFM_AlarmInfo)
} SFM_SysInfo;



/* ---------------------------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------------------------- */
// CPU, Memory, Disk �� ��� ���,�ϰ� ����� �ִ� ��� ������ ���� structure

typedef struct {
	unsigned char	minFlag;  // minor ��� �߻��Ǹ� setting�ȴ�. ����� �ϰ��ϴ� ��� ���� �޽����� ������ �ϴ��� �����ϴµ� ����ȴ�.
	unsigned char	majFlag;  // major ��� �߻��Ǹ� setting�ȴ�. ����� �ϰ��ϴ� ��� ���� �޽����� ������ �ϴ��� �����ϴµ� ����ȴ�.
	unsigned char	criFlag;  // critical ��� �߻��Ǹ� setting�ȴ�. ����� �ϰ��ϴ� ��� ���� �޽����� ������ �ϴ��� �����ϴµ� ����ȴ�.
} SFM_UsageAlmFlag_s;

typedef struct {
	SFM_UsageAlmFlag_s	cpu[SFM_MAX_CPU_CNT];
	SFM_UsageAlmFlag_s	mem;
	SFM_UsageAlmFlag_s	disk[SFM_MAX_DISK_CNT];
} SFM_UsageAlmFlag;
/* ---------------------------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------------------------- */
// �ʱ� ��ü ���� ����
typedef struct{
	char				sysType[COMM_MAX_NAME_LEN];
	unsigned char		sysCnt;
	int					sysId[SYSCONF_MAX_SYS_TYPE_MEMBER];
	char				sysName[SYSCONF_MAX_SYS_TYPE_MEMBER][COMM_MAX_NAME_LEN];
} SFM_SystemTypeData;

typedef struct{
	unsigned char		totalCnt;	// ������� ��ü ���� (VP-2, SP-2, OMP-1) ��, 5��
	unsigned char		typeCnt;	// ��� Type�� ����   (OMP,VP,SP) INIP -> 3
	SFM_SystemTypeData	typeData[SYSCONF_MAX_SYS_TYPE_NUM];  // ���type�� ������� ����
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
	time_t				when;			// ��� �߻� �ð�
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
// ��ü ���� ���� ����ü
//------------------------------------------------------------------------------
typedef struct {
	SFM_SysInfo				sys;
   SFM_UsageAlmFlag	usageAlmFlag;	// CPU,MEM,HDD �� ���� ��ֵ��
} SFM_sfdb;

//------------------------------------------------------------------------------
// MP���� �ǽð����� �����ִ� Fault ��������ü
//------------------------------------------------------------------------------
#pragma pack (1)
typedef struct {
	char   		alarmCode[8];		//A1001 ~
	int     	when;				//�߻��ð�
	int     	mswhen;			/*** msec ***/
	int     	alarmLevel;		//�˶����
	int     	notiFlag;  /*** 0 : email, sms ������ ����, 1 : ���� ***/
	char    	rscName[16];	//��� �̸�
	char    	Desc[80];		//�ΰ�����
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
// �˶��ڵ庰 ���� ����(�����޸𸮿� ���Ϸ� ����)
typedef struct
{
  int    cnt;
  int     alarmCode[SFM_ALARMINFO_CNT];		
  int     alarmLevel[SFM_ALARMINFO_CNT];		
  int     notiFlag[SFM_ALARMINFO_CNT];  /*** 0 : email, sms ������ ����, 1 : ���� ***/
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
