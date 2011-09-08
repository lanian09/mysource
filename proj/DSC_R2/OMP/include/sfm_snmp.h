#ifndef SFM_SNMP_H
#define SFM_SNMP_H


#define PD_FUNCTION_ON		1
#define PD_FUNCTION_OFF		0

#define	MAX_PD_FAN_NUM		4
#define MAX_PD_CPU_CNT		1
#define MAX_PD_POWER_NUM    2 // 20110424 by dcham

#pragma pack(1)
typedef struct {
	unsigned char	minFlag;  // minor ��� �߻��Ǹ� setting�ȴ�. ����� �ϰ��ϴ� ��� ���� �޽����� ������ �ϴ��� �����ϴµ� ����ȴ�.
	unsigned char	majFlag;  // major ��� �߻��Ǹ� setting�ȴ�. ����� �ϰ��ϴ� ��� ���� �޽����� ������ �ϴ��� �����ϴµ� ����ȴ�.
	unsigned char	criFlag;  // critical ��� �߻��Ǹ� setting�ȴ�. ����� �ϰ��ϴ� ��� ���� �޽����� ������ �ϴ��� �����ϴµ� ����ȴ�.
} SFM_PdHwAlmFlag_s;
#pragma pack()

#pragma pack(1)
typedef struct {
    unsigned char   mask;
    unsigned short  usage;     // ���� CPU �����
    unsigned char   level;     // ���� CPU ��� ���
    unsigned char   minLimit;  // minor ��� �Ӱ谪 -> �� ���� �ʰ��Ͽ� duration �̻� ���ӵǸ� ��ַ� �����Ѵ�.
    unsigned char   majLimit;  // major ��� �Ӱ谪
    unsigned char   criLimit;  // critical ��� �Ӱ谪
    unsigned char   minDurat;  //
    unsigned char   majDurat;  //
    unsigned char   criDurat;  //
	SFM_PdHwAlmFlag_s	cpuFlag;
} SFM_PDCpuInfo;
#pragma pack()

#pragma pack(1)
typedef struct {
    unsigned char   mask;
    unsigned short  usage;     // ���� memory �����
    unsigned char   level;     // ���� memory ��� ���
    unsigned char   minLimit;  // minor ��� �Ӱ谪 -> �� ���� �ʰ��Ͽ� duration �̻� ���ӵǸ� ��ַ� �����Ѵ�.
    unsigned char   majLimit;  // major ��� �Ӱ谪
    unsigned char   criLimit;  // critical ��� �Ӱ谪
    unsigned char   minDurat;  //
    unsigned char   majDurat;  //
    unsigned char   criDurat;  //
	SFM_PdHwAlmFlag_s	memFlag;
} SFM_PDMemInfo;
#pragma pack()

#pragma pack(1)
typedef struct {
	char			mask[MAX_PD_FAN_NUM];
	unsigned char	status[MAX_PD_FAN_NUM];
	unsigned char	prevStatus[MAX_PD_FAN_NUM];
	unsigned char	level[MAX_PD_FAN_NUM];
}SFM_PDFanInfo;
#pragma pack()

#pragma pack(1)
typedef struct {
	char			mask;
	unsigned char	status;
	unsigned char	prevStatus;
	unsigned char	level;
}SFM_PDGigaLanInfo;
#pragma pack()

#pragma pack(1)
typedef struct {
	char            mask;
	unsigned char   status;
	unsigned char   prevStatus;
	unsigned char   level;
}SFM_PDPowerInfo; // 20110424 by dcham
#pragma pack()

#pragma pack(1)
typedef struct {
	SFM_PDCpuInfo	cpuInfo;
	SFM_PDMemInfo	memInfo;
	SFM_PDFanInfo	fanInfo;
#define MAX_GIGA_LAN_NUM	23    /*Director���� ���� ��Ʈ�� 23�� : sjjeon*/
	SFM_PDGigaLanInfo	gigaLanInfo[MAX_GIGA_LAN_NUM];
#define MAX_POWER_NUM       2     //  Director�� Power ���� : 2 ��, 20110422 by dcham
	SFM_PDPowerInfo     powerInfo[MAX_POWER_NUM];
}SFM_L3ProbeDev;
#pragma pack()

#pragma pack(1)
typedef struct {
#define MAX_PROBE_DEV_NUM	2
	SFM_L3ProbeDev		l3ProbeDev[MAX_PROBE_DEV_NUM];
}SFM_L3PD;
#pragma pack()

#define SFM_L3PD_SIZE	sizeof(SFM_L3PD)

#pragma pack(1)
typedef struct {
	SFM_PDCpuInfo		cpuInfo;
	SFM_PDMemInfo		memInfo;
#define MAX_L2_PORT_NUM					27 
	SFM_PDGigaLanInfo	portInfo[MAX_L2_PORT_NUM];
} SFM_L2SW;
#pragma pack()

#pragma pack(1)
typedef struct {
#define MAX_L2_DEV_NUM					2
	SFM_L2SW			l2Info[MAX_L2_DEV_NUM];
} SFM_L2Dev;
#pragma pack()

#define SFM_L2DEV_SIZE	sizeof(SFM_L2Dev)


#define SCE_ALARM_OTHER			0x01
#define SCE_ALARM_OFF			0x02
#define SCE_ALARM_ON			0x03

#define SCE_MODULE_STAT_OTHER           0x01
#define SCE_MODULE_STAT_ACTIVE          0x02
#define SCE_MODULE_STAT_STANDBY         0x03
 
#define SCE_LINK_STAT_BYPASS            0x01
#define SCE_LINK_STAT_FORWARD           0x02
#define SCE_LINK_STAT_SNIFFING          0x03

#define SCE_IFN_STAT_OTHER				0x01
#define SCE_IFN_STAT_UP					0x02
#define SCE_IFN_STAT_DOWN_REF			0x03
#define SCE_IFN_STAT_DOWN_RED			0x04
#define SCE_IFN_STAT_DOWN_OTHER			0x05

#define SCE_RDR_STAT_OTHER              0x01
#define SCE_RDR_STAT_ACTIVE             0x02
#define SCE_RDR_STAT_STANDBY            0x03

#define SCE_RDR_CONNECT_STAT_OTHER      0x01
#define SCE_RDR_CONNECT_STAT_UP         0x02
#define SCE_RDR_CONNECT_STAT_DOWN       0x03


/* SCE system���� �ܼ��� ������ ��� �ʵ� ���� */
/* ���� ���� ������ �����ؾ� �� ���, ���¿� ���ؼ� alarm���θ� �Ǵ��ؾ� �� ���
   SCE_SYS_ALARM_INFO ����ü�� ����Ѵ�. */

/* hjjung_20100823 short���� int�� ����*/
#pragma pack(1)
typedef struct {
	unsigned int	intro_user;  
    unsigned int	active_user;  
//    unsigned char   status;
    unsigned char   version[64];  
} SCE_SYS_INFO;
#pragma pack()

#pragma pack(1)
typedef struct {
    unsigned char   mask;		/* mml �� ������ alarm �߻� ���� ���� */
    unsigned short  usage;     	/* ���� ����� */
    unsigned char   level;     	/* ���� ��� ��� */
    unsigned char   minLimit;  	/* minor ��� �Ӱ谪 -> �� ���� �ʰ��Ͽ� duration �̻� ���ӵǸ� ��ַ� �����Ѵ�. */
    unsigned char   majLimit;  	/* major ��� �Ӱ谪 */  
    unsigned char   criLimit;  	/* critical ��� �Ӱ谪 */
    unsigned char   minDurat;  	
    unsigned char   majDurat;  	
    unsigned char   criDurat;  	
	unsigned char	minFlag;  	/* minor ��� �߻��Ǹ� setting�ȴ�. ����� �ϰ��ϴ� ��� ���� �޽����� ������ �ϴ��� �����ϴµ� ����ȴ�. */
	unsigned char	majFlag;  	/* major ��� �߻��Ǹ� setting�ȴ�. ����� �ϰ��ϴ� ��� ���� �޽����� ������ �ϴ��� �����ϴµ� ����ȴ�. */
	unsigned char	criFlag;  	/* critical ��� �߻��Ǹ� setting�ȴ�. ����� �ϰ��ϴ� ��� ���� �޽����� ������ �ϴ��� �����ϴµ� ����ȴ�. */
} SCE_SYS_USAGE_INFO;
#pragma pack()

/* hjjung_20100823 */
#pragma pack(1)
typedef struct {
    unsigned char   mask;		/* mml �� ������ alarm �߻� ���� ���� */
    unsigned int	num;		/* ���� ��� �� */
    unsigned char   level;     	/* ���� ��� ��� */
    unsigned int    minLimit;  	/* minor ��� �Ӱ谪 -> �� ���� �ʰ��Ͽ� duration �̻� ���ӵǸ� ��ַ� �����Ѵ�. */
    unsigned int    majLimit;  	/* major ��� �Ӱ谪 */  
    unsigned int    criLimit;  	/* critical ��� �Ӱ谪 */
    unsigned char   minDurat;  	
    unsigned char   majDurat;  	
    unsigned char   criDurat;  	
	unsigned char	minFlag;  	/* minor ��� �߻��Ǹ� setting�ȴ�. ����� �ϰ��ϴ� ��� ���� �޽����� ������ �ϴ��� �����ϴµ� ����ȴ�. */
	unsigned char	majFlag;  	/* major ��� �߻��Ǹ� setting�ȴ�. ����� �ϰ��ϴ� ��� ���� �޽����� ������ �ϴ��� �����ϴµ� ����ȴ�. */
	unsigned char	criFlag;  	/* critical ��� �߻��Ǹ� setting�ȴ�. ����� �ϰ��ϴ� ��� ���� �޽����� ������ �ϴ��� �����ϴµ� ����ȴ�. */
} SCE_SYS_NUM_INFO;
#pragma pack()

#pragma pack(1)
typedef struct {
	char			mask;
	unsigned char	status;
	unsigned char	preStatus;
	unsigned char	level;
    unsigned char   minDurat;  //
    unsigned char   majDurat;  //
    unsigned char   criDurat;  //
} SCE_SYS_STATUS_INFO;
#pragma pack()

#pragma pack(1)
typedef struct {
#define MAX_SCE_CPU_CNT			3	
#define MAX_SCE_MEM_CNT			3	
	SCE_SYS_INFO			sysInfo;
	SCE_SYS_USAGE_INFO		cpuInfo[MAX_SCE_CPU_CNT];
	SCE_SYS_USAGE_INFO		memInfo[MAX_SCE_MEM_CNT];
	SCE_SYS_USAGE_INFO		flowlossInfo[MAX_SCE_MEM_CNT];
	SCE_SYS_USAGE_INFO		diskInfo;
	SCE_SYS_NUM_INFO		userInfo;				/* hjjung_20100823 */
	SCE_SYS_STATUS_INFO		sysStatus;								/* SCE system status */
	SCE_SYS_STATUS_INFO		pwrStatus;								/* SCE power status */
	SCE_SYS_STATUS_INFO		fanStatus;								/* SCE fan status */
	SCE_SYS_STATUS_INFO		tempStatus;								/* SCE temperature status */
	SCE_SYS_STATUS_INFO		voltStatus;								/* SCE voltage status */
#define MAX_SCE_MODULE_CNT      1
#define MAX_SCE_LINK_CNT        2
#define MAX_SCE_IFN_CNT			6
	SCE_SYS_STATUS_INFO		portModuleStatus;						/* SCE module status, one module */
	SCE_SYS_STATUS_INFO		portLinkStatus[MAX_SCE_LINK_CNT];		/* SCE port Link status
																	   data port: 3,4 port first link, 5,6 port seconds link */
	SCE_SYS_STATUS_INFO		portStatus[MAX_SCE_IFN_CNT];			/* SCE port status, manage port:2, data port:4 */
#define MAX_SCE_RDR_INFO_CNT	2
	SCE_SYS_STATUS_INFO     rdrStatus[MAX_SCE_RDR_INFO_CNT];        /* SCE rdr status */    
	SCE_SYS_STATUS_INFO     rdrConnStatus[MAX_SCE_RDR_INFO_CNT];    /* SCE rdr connect status */
//	SCE_SYS_STATUS_INFO		rdrStatus;								/* SCE rdr status */
//	SCE_SYS_STATUS_INFO		rdrConnStatus;							/* SCE rdr connect status */
}SFM_SCEDev;
#pragma pack()

#pragma pack(1)
typedef struct {
#define MAX_SCE_DEV_NUM	2
	SFM_SCEDev		SCEDev[MAX_SCE_DEV_NUM];
}SFM_SCE;
#pragma pack()

#define SFM_SCE_SIZE	sizeof(SFM_SCE)

#endif /* SFM_SNMP_H */

