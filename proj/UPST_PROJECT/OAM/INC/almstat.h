#ifndef __ALM_STAT_H__
#define __ALM_STAT_H__

#include <time.h>
#include "typedef.h"
/* 
	COND 에서 메세지를 만들기 위해서 사용되는
	ALARM 이 발생했을 때의 각종 Code 값을 정의해 놓은 파일
*/

/* MASKING FLAG */
#define MASK_VALUE		128

/* ALARM LEVEL */

#define NOT_EQUIP		0x00
#define STOP			0x01
#define NORMAL			0x03
#define MINOR			0x04
#define MAJOR			0x05
#define CRITICAL		0x06
#define MASK			0x80

/* SYSTYPE */
#define SYSTYPE_GTAM	0x00
#define SYSTYPE_TAM 	0x01
#define SYSTYPE_TAF		0x02
#define SYSTYPE_DIRECT  0x03
#define SYSTYPE_SWITCH	0x04

/* LOCTYPE */
#define LOCTYPE_PHSC	0x01
#define LOCTYPE_LOAD	0x02
#define LOCTYPE_PROCESS 0x03
#define LOCTYPE_CHNL	0x04
#define LOCTYPE_NTP		0x05
#define LOCTYPE_SVC		0x06

/* INVTYPE - PHSC */
#define INVTYPE_POWER			0x01
#define INVTYPE_LINK			0x02
#define INVTYPE_FAN				0x03
#define INVTYPE_DISKARRAY 		0x04
//TAF
#define INVTYPE_PORT			0x05	/* ENDACE */
#define INVTYPE_MIRROR_STS		0x06	/* MIRROR STS */
#define INVTYPE_MIRROR_ACT		0x07	/* MIRROR ACT */
//TAM
#define INVTYPE_PORT_MONITOR 	0x05	/* DIRECTOR */
#define INVTYPE_PORT_MIRROR	 	0x06	/* DIRECTOR */
#define INVTYPE_PORT_SWITCH		0x0A	/* SWITCH */
#define INVTYPE_POWER_DIRECTOR 	0x0B	/* DIRECTOR */

/* INVTYPE - LOAD */
#define INVTYPE_CPU				0x01
#define INVTYPE_MEMORY			0x02
#define INVTYPE_DISK			0x03
#define INVTYPE_QUEUE			0x04
#define INVTYPE_SESSION			0x05
#define INVTYPE_TRAFFIC			0x06
#define INVTYPE_DBSTATUS		0x06
#define INVTYPE_NIFO			0x07
#define INVTYPE_CPU_SWITCH  	0x08
#define INVTYPE_MEMORY_SWITCH	0x09

/* INVTYPE - PROCESS */
#define INVTYPE_USERPROC		0x01

/* INVTYPE - CHANNEL */
#define INVTYPE_CLIENT		0x01
#define INVTYPE_DBMS		0x02
#define INVTYPE_NMS			0x03
#define INVTYPE_DNMS		0x04

/* INVTYPE - NTP */
#define INVTYPE_NTPSVR		0x01
#define INVTYPE_TIMESYNC	0x02

/* INVTYPE - SERVICE */
#define INVTYPE_CALL            0x01
#define INVTYPE_RECALL          0x02 
#define INVTYPE_AAA             0x03
#define INVTYPE_HSS             0x04
#define INVTYPE_LNS             0x05
#define INVTYPE_MENU            0x06
#define INVTYPE_DN              0x07
#define INVTYPE_STREAM          0x08
#define INVTYPE_MMS             0x09
#define INVTYPE_WIDGET          0x0A
#define INVTYPE_PHONE           0x0B
#define INVTYPE_EMS             0x0C
#define INVTYPE_BANK            0x0D
#define INVTYPE_FV              0x0E
#define INVTYPE_IM              0x0F
#define INVTYPE_VT              0x10
#define INVTYPE_ETC             0x11
#define INVTYPE_CORP            0x12
#define INVTYPE_REGI            0x13  
#define INVTYPE_INET            0x14
#define INVTYPE_RECVCALL        0x15
#define INVTYPE_IM_RECV         0x16
#define INVTYPE_VT_RECV         0x17
#define INVTYPE_TAF_TRAFFIC     0x18

typedef struct _st_Alm_Status {
    unsigned char   ucLocType;
    unsigned char   ucSysType;
    unsigned char   ucSysNo;
    unsigned char   ucInvType;
    unsigned char   ucInvNo;
    unsigned char   ucAlmLevel;
    unsigned char   ucOldAlmLevel;
    unsigned char   ucReserv;
    time_t          tWhen;
    unsigned int    uiIPAddr;
    long long       llLoadVal;
} st_almsts, *pst_almsts;
#define DEF_ALMSTS_SIZE sizeof(st_almsts)

typedef struct _st_NTP {
    unsigned char   ucSymF;
    unsigned char   ucTF;
    unsigned char   ucResev[6];
    unsigned short  usWhen;
    unsigned short  usPoll;
    unsigned short  usReach;
    unsigned short  usST;
    float           fDelay;
    float           fOffset;
    float           fJitter;
    float           fResefv;
    char            szIP[32];
    char            szRefIP[32];
} st_NTP, *pst_NTP;
#define DEF_NTP_SIZE sizeof(st_NTP)

typedef struct _st_NTP_STS {
    int     dNtpCnt;
	int     dReserv;
	st_NTP  stNTP[32];
} st_NTP_STS, *pst_NTP_STS;

typedef struct _st_Load {
	int    dCnt;
	float  fMin;
	float  fMax;
	float  fAvg;
} st_Load, *pst_Load;

typedef struct _st_StatLoad {
	st_Load stCpu;
	st_Load stMem;
	st_Load stDisk;
	st_Load stQueue;
	st_Load stNifo;
	st_Load stCifo;
} st_StatLoad, *pst_StatLoad ;
#define DEF_NTPSTS_SIZE sizeof(st_NTP_STS)

typedef struct _st_WNTAM_LOAD {                                                            
	time_t      tWhen;                                                                     
	time_t      tReserv;                                                                   
	st_StatLoad stNTAM;                                                                    
	st_StatLoad stNTAF[32];                                                                
} st_WNTAM_LOAD, *pst_WNTAM_LOAD;                                                          

#define MAX_STAT_LIST  12
typedef struct _st_WNTAM_LOADSTAT {                                                        
	unsigned short  usStartIdx;
	unsigned short  usCurIdx;
	unsigned int    uiReserv;
	st_WNTAM_LOAD stWNTAMLOAD[MAX_STAT_LIST];                                              
} st_WNTAM_LOADSTAT, *pst_WNTAM_LOADSTAT ; 

//WNTAS2
typedef struct _st_TotalReqResStat
{
	unsigned int uiReqCnt;
	unsigned int uiReqRealCnt;
	unsigned int uiReqByte;

	unsigned int uiResCnt;
	unsigned int uiResRealCnt;
	unsigned int uiResByte;
}st_TotalReqResStat, *pst_TotalReqResStat;

typedef struct {
	//DQMS
	time_t          KeyTime;
	unsigned int    uiFrames;
	unsigned long   ulBytes;
	//WNTAS2
	unsigned int    uiUpFrames;
	unsigned int    uiDownFrames;
	unsigned long   ulUpBytes;
	unsigned long   ulDownBytes;
} st_UpDownStat;

#define MAX_STAT_SIZE   12
typedef struct {
	char            upinf;
	char            downinf;
	char            DebugLevel;
	char            CDRLevel;
	char            IPAFID;
	char            reserved[3];

	int             IdleTime;
	int             RetryTime;

	st_UpDownStat   ThruStat[MAX_STAT_SIZE];

	st_UpDownStat   TotStat[MAX_STAT_SIZE];
	st_UpDownStat   IPStat[MAX_STAT_SIZE];
	st_UpDownStat   UDPStat[MAX_STAT_SIZE];
	st_UpDownStat   TCPStat[MAX_STAT_SIZE];
	st_UpDownStat   SCTPStat[MAX_STAT_SIZE];
	st_UpDownStat   ETCStat[MAX_STAT_SIZE];

	st_UpDownStat   IPError[MAX_STAT_SIZE];
	st_UpDownStat   UTCPError[MAX_STAT_SIZE];
	st_UpDownStat   TCPReTrans[MAX_STAT_SIZE]; //WNTAS2

	st_UpDownStat   FailData[MAX_STAT_SIZE];
	st_UpDownStat   FilterOut[MAX_STAT_SIZE]; //WNTAS:OutOfIP
	st_UpDownStat   DropData[MAX_STAT_SIZE];

} st_GEN_INFO, *pst_GEN_INFO;

typedef struct _st_NtafStat
{
    unsigned int    uiFrames;
    char            cReserved[4];

    unsigned long   ulBytes;
} st_NtafStat, *pst_NtafStat;

#define DEF_NTAFTRAFFIC_LEN     sizeof(st_NtafStat)

typedef struct _st_NtafStatList
{
	time_t          KeyTime;
	UCHAR           ucTAFID;
	UCHAR           Reserved[3];

	st_NtafStat     ThruStat;

	st_NtafStat     TotStat;
	st_NtafStat     IPStat;
	st_NtafStat     UDPStat;
	st_NtafStat     TCPStat;
	st_NtafStat     SCTPStat;
	st_NtafStat     ETCStat;

	st_NtafStat     IPError;
	st_NtafStat     UTCPError;
	st_NtafStat     FailData;
	st_NtafStat     FilterOut;
} st_NtafStatList, *pst_NtafStatList;

#define DEF_NTAFSTAT_LEN	sizeof(st_NtafStatList)

#endif /* __ALM_STAT_H__ */
