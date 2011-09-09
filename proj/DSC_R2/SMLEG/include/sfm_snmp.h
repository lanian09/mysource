#ifndef SFM_SNMP_H
#define SFM_SNMP_H


#define PD_FUNCTION_ON		1
#define PD_FUNCTION_OFF		0

#define	MAX_PD_FAN_NUM		4
#define MAX_PD_CPU_CNT			1

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
	SFM_PDCpuInfo	cpuInfo;
	SFM_PDMemInfo	memInfo;
	SFM_PDFanInfo	fanInfo;
#define MAX_GIGA_LAN_NUM	52
	SFM_PDGigaLanInfo	gigaLanInfo[MAX_GIGA_LAN_NUM];
}SFM_L3ProbeDev;
#pragma pack()

#pragma pack(1)
typedef struct {
#define MAX_PROBE_DEV_NUM	2
	SFM_L3ProbeDev		l3ProbeDev[MAX_PROBE_DEV_NUM];
}SFM_L3PD;
#pragma pack()

#define SFM_L3PD_SIZE	sizeof(SFM_L3PD)

#endif /* SFM_SNMP_H */
