#ifndef SFM_SNMP_H
#define SFM_SNMP_H


#define PD_FUNCTION_ON		1
#define PD_FUNCTION_OFF		0

#define	MAX_PD_FAN_NUM		4
#define MAX_PD_CPU_CNT			1

#pragma pack(1)
typedef struct {
	unsigned char	minFlag;  // minor 장애 발생되면 setting된다. 등급이 하강하는 경우 해지 메시지를 보내야 하는지 결정하는데 참고된다.
	unsigned char	majFlag;  // major 장애 발생되면 setting된다. 등급이 하강하는 경우 해지 메시지를 보내야 하는지 결정하는데 참고된다.
	unsigned char	criFlag;  // critical 장애 발생되면 setting된다. 등급이 하강하는 경우 해지 메시지를 보내야 하는지 결정하는데 참고된다.
} SFM_PdHwAlmFlag_s;
#pragma pack()

#pragma pack(1)
typedef struct {
    unsigned char   mask;
    unsigned short  usage;     // 현재 CPU 사용율
    unsigned char   level;     // 현재 CPU 장애 등급
    unsigned char   minLimit;  // minor 장애 임계값 -> 이 값을 초과하여 duration 이상 지속되면 장애로 판정한다.
    unsigned char   majLimit;  // major 장애 임계값
    unsigned char   criLimit;  // critical 장애 임계값
    unsigned char   minDurat;  //
    unsigned char   majDurat;  //
    unsigned char   criDurat;  //
	SFM_PdHwAlmFlag_s	cpuFlag;
} SFM_PDCpuInfo;
#pragma pack()

#pragma pack(1)
typedef struct {
    unsigned char   mask;
    unsigned short  usage;     // 현재 memory 사용율
    unsigned char   level;     // 현재 memory 장애 등급
    unsigned char   minLimit;  // minor 장애 임계값 -> 이 값을 초과하여 duration 이상 지속되면 장애로 판정한다.
    unsigned char   majLimit;  // major 장애 임계값
    unsigned char   criLimit;  // critical 장애 임계값
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
