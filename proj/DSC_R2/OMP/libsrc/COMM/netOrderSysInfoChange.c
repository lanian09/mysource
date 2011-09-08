#include <stdio.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>
#include "netOrderSysInfoChange.h"

/*########################### Network To  Host ##############################*/

void SFM_SysInfo_N2H(SFM_SysInfo *sysInfo)   
{
    //SFM_SysAlmInfo_N2H(&sysInfo->almInfo);
    SFM_SysCommInfo_N2H(&sysInfo->commInfo);
    SFM_SysSpecInfo_N2H(&sysInfo->specInfo);
    SFM_SysSuccRateIpInfo_N2H(&sysInfo->succRateIpInfo);
}

void SFM_SysAlmInfo_N2H(SFM_SysAlmInfo *sysMsg)
{
	//unsigned char   level;     // 해당 시스템의 전체 장애 등급
	//unsigned char   prevLevel; // previous 해당 시스템의 전체 장애 등급
	//unsigned char   minCnt;    // 현재 발생되어 있는 minor 장애 갯수
	//unsigned char   majCnt;    // 현재 발생되어 있는 major 장애 갯수
	//unsigned char   criCnt;    // 현재 발생되어 있는 critical 장애 갯수
}

void SFM_SysCommInfo_N2H(SFM_SysCommInfo *sysInfo)
{
	int i;
//    char        type        [COMM_MAX_NAME_LEN];  // system type
//    char        group       [COMM_MAX_NAME_LEN]; // system group name
//    char        name        [COMM_MAX_NAME_LEN];  // system name
//    unsigned char   cpuCnt;   // 관리대상 cpu 갯수
//    unsigned char   diskCnt;  // 관리대상 disk partition 갯수
//    unsigned char   lanCnt;   // 관리대상 lan 갯수
  //  unsigned char   procCnt;  // 관리대상 process 갯수
    //unsigned char   queCnt;   // 관리대상 queue 갯수    added 2004/02/04
    //unsigned short  total_disk_usage;
    sysInfo->total_disk_usage = (unsigned short)ntohs(sysInfo->total_disk_usage);
    //unsigned char   rmtLanCnt;
    //unsigned short  sessLoad[SFM_MAX_SESSION_CNT]; // 현재 사용 하지 않음.
    for(i=0;i<SFM_MAX_SESSION_CNT;i++)
		sysInfo->sessLoad[i] = (unsigned short)ntohs(sysInfo->sessLoad[i]);    
    //SFM_CpuInfo cpuInfo;
    SFM_CpuInfo_N2H(&sysInfo->cpuInfo);
    //SFM_MemInfo memInfo;
    SFM_MemInfo_N2H(&sysInfo->memInfo);    
    //SFM_DiskInfo    diskInfo    [SFM_MAX_DISK_CNT];
    for(i=0;i<SFM_MAX_DISK_CNT;i++)
		SFM_DiskInfo_N2H(&sysInfo->diskInfo[i]);
//    SFM_LanInfo lanInfo     [SFM_MAX_LAN_CNT];
 //   for(i=0;i<SFM_MAX_LAN_CNT;i++)
//		SFM_LanInfo_N2H(&sysInfo->lanInfo[i]);
//    SFM_ProcInfo    procInfo    [SFM_MAX_PROC_CNT];
    for(i=0;i<SFM_MAX_PROC_CNT;i++)
		SFM_ProcInfo_N2H(&sysInfo->procInfo[i]);
//    SFM_QueInfo queInfo     [SFM_MAX_QUE_CNT];          // added 2004/02/04
    for(i=0;i<SFM_MAX_QUE_CNT;i++)
		SFM_QueInfo_N2H(&sysInfo->queInfo[i]);
//    SFM_LanInfo rmtLanInfo  [SFM_MAX_RMT_LAN_CNT];          // 원격지 장비 통신 관련
//    for(i=0;i<SFM_MAX_RMT_LAN_CNT;i++)
//		SFM_LanInfo_N2H(&sysInfo->rmtLanInfo[i]);		
//    SFM_LanInfo optLanInfo  [2];                    // 광통신 관련 정보
 //   for(i=0;i<2;i++)
//		SFM_LanInfo_N2H(&sysInfo->optLanInfo[i]);	
//    SFM_SysDupSts   systemDup;	
	SFM_SysDupSts_N2H(&sysInfo->systemDup);
//    SFM_SysSuccRate succRate    [SFM_MAX_SUCC_RATE_CNT];
    for(i=0;i<SFM_MAX_SUCC_RATE_CNT;i++)
		SFM_SysSuccRate_N2H(&sysInfo->succRate[i]);
//    SFM_SysDBSts    rmtDbSts    [SFM_MAX_DB_CNT];
    for(i=0;i<SFM_MAX_DB_CNT;i++)
		SFM_SysDBSts_N2H(&sysInfo->rmtDbSts[i]);
//    SFM_SysRSRCInfo rsrcSts         [SFM_MAX_RSRC_LOAD_CNT];
    for(i=0;i<SFM_MAX_RSRC_LOAD_CNT;i++)
		SFM_SysRSRCInfo_N2H(&sysInfo->rsrcSts[i]);
//    SFM_NTPSts  ntpSts      [MAX_HW_NTP];	
//    for(i=0;i<MAX_HW_NTP;i++)
//		SFM_NTPSts_N2H(&sysInfo->ntpSts[i]);
		
}

void SFM_CpuInfo_N2H(SFM_CpuInfo *cpuInfo)
{
	int i;
//	unsigned char   mask[SFM_MAX_CPU_CNT];
//    unsigned short  usage[SFM_MAX_CPU_CNT];     // 현재 CPU 사용율
	for(i=0;i<SFM_MAX_CPU_CNT;i++)
    	cpuInfo->usage[i] = (unsigned short)ntohs(cpuInfo->usage[i]);
//    unsigned char   level[SFM_MAX_CPU_CNT];     // 현재 CPU 장애 등급
//    unsigned char   minLimit;  // minor 장애 임계값 -> 이 값을 초과하여 duration 이상 지속되면 장애로 판정한다.
//    unsigned char   majLimit;  // major 장애 임계값
//    unsigned char   criLimit;  // critical 장애 임계값
//    unsigned char   minDurat;  //
//    unsigned char   majDurat;  //
//    unsigned char   criDurat;  //
}

void SFM_MemInfo_N2H(SFM_MemInfo *memInfo)
{
//    unsigned char   mask;
//    unsigned short  usage;     // 현재 memory 사용율
	  memInfo->usage = (unsigned short)ntohs(memInfo->usage);
//    unsigned char   level;     // 현재 memory 장애 등급
//    unsigned char   minLimit;  // minor 장애 임계값 -> 이 값을 초과하여 duration 이상 지속되면 장애로 판정한다.
//    unsigned char   majLimit;  // major 장애 임계값
//    unsigned char   criLimit;  // critical 장애 임계값
//    unsigned char   minDurat;  //
//    unsigned char   majDurat;  //
//    unsigned char   criDurat;  //	
}

void SFM_DiskInfo_N2H(SFM_DiskInfo *diskInfo)
{
//   unsigned char   mask;
//   char            name[SFM_MAX_DISK_NAME]; // disk partition name
//   unsigned short  usage;     // 현재 사용율
	 diskInfo->usage = (unsigned short)ntohs(diskInfo->usage);
//   unsigned char   level;     // 현재 장애 등급
//   unsigned char   minLimit;  // minor 장애 임계값
//   unsigned char   majLimit;  // major 장애 임계값
//   unsigned char   criLimit;  // critical 장애 임계값
}

void SFM_LanInfo_N2H(SFM_LanInfo *LanInfo)
{
//	char            mask;
//    char            name[SFM_MAX_LAN_NAME_LEN]; //
//    char            targetIp[SFM_MAX_TARGET_IP_LEN]; //
//    unsigned char   status;     // 현재 상태
//    unsigned char   prevStatus; // 이전 상태
//    unsigned char   level;      // 장애 발생 시 해당 프로세스에 적용될 장애 등급
}

void SFM_ProcInfo_N2H(SFM_ProcInfo *procInfo)
{
//	  unsigned char   mask;
//    char            name[COMM_MAX_NAME_LEN]; //
//    unsigned char   status;     // 현재 상태
//    unsigned char   prevStatus; // 이전 상태
//    unsigned char   level;      // 장애 발생 시 해당 프로세스에 적용될 장애 등급
//    pid_t           pid;
	procInfo->pid = (unsigned int)ntohl(procInfo->pid);
//    time_t          uptime;
	procInfo->uptime = (unsigned int)ntohl(procInfo->uptime);
}

void SFM_QueInfo_N2H(SFM_QueInfo *queInfo)
{
//	int     qID;
	queInfo->qID = (int)ntohl(queInfo->qID);
//	int     qKEY;
	queInfo->qKEY = (int)ntohl(queInfo->qKEY);
//	unsigned char   mask;
//	char            qNAME[QUE_MAX_NAME_LEN];
//	unsigned int    qNUM;
	queInfo->qNUM = (unsigned int)ntohl(queInfo->qNUM);
//	unsigned int    cBYTES;
	queInfo->cBYTES = (unsigned int)ntohl(queInfo->cBYTES);
//	unsigned int    qBYTES;
	queInfo->qBYTES = (unsigned int)ntohl(queInfo->qBYTES);
//	unsigned int    load;     // 현재 사용율
	queInfo->load = (unsigned int)ntohl(queInfo->load);
//	unsigned char   level;     // 현재 장애 등급
//	unsigned char   minLimit;  // minor 장애 임계값
//	unsigned char   majLimit;  // major 장애 임계값
//	unsigned char   criLimit;  // critical 장애 임계값	
}

void SFM_SysDupSts_N2H(SFM_SysDupSts *sysDupSts)
{
//	unsigned char   mask;
//	unsigned char   myStatus;   /* 1 : ACTIVE, 2 : STANDBY */
//	unsigned char   yourStatus; /* 1 : ACTIVE, 2 : STANDBY */
//	unsigned char   sizeMin[17];
//	long long   llCorelationId;
	sysDupSts->llCorelationId = (long long)ntohl(sysDupSts->llCorelationId);
//	unsigned int    uiTrsId;
//	unsigned int    uiUdrSeq;
//	unsigned char   heartbeatAlm;       /* 1 : Normal, 2 : AbNormal(alarm) */
//	unsigned char   timeOutAlm;         /* 1 : Normal, 2 : AbNormal(alarm) */
//	unsigned char   heartbeatLevel;
//	unsigned char   timeOutAlmLevel;
//	unsigned char   dualStsAlmLevel;
//	unsigned char   oosAlm;         /* 1 : Normal, 2 : Out-of-Service occured(alarm) */
}
void SFM_SysSuccRate_N2H(SFM_SysSuccRate *sysSuccRate)
{
//unsigned char   mask;
//unsigned char   name[COMM_MAX_NAME_LEN];
//unsigned int    cnt; /* count occured */
	sysSuccRate->cnt = (unsigned int)ntohl(sysSuccRate->cnt);
//unsigned int    rate;  /* success rate, unit:percent */
//unsigned char   level;
}
void SFM_SysDBSts_N2H(SFM_SysDBSts *sysDBSts)
{
//    unsigned char   mask;
//    unsigned char   sDbAlias[20];
//    unsigned char   sIpAddress[16];
//    unsigned int    iStatus; /* 1 - connect, 0 - not connect */
	sysDBSts->iStatus = (unsigned int)ntohl(sysDBSts->iStatus);
}

void SFM_SysRSRCInfo_N2H(SFM_SysRSRCInfo *sysRSRCInfo)
{
//	unsigned char   mask;
//	unsigned int    rsrcload;
	sysRSRCInfo->rsrcload = (unsigned int)ntohl(sysRSRCInfo->rsrcload);
//	unsigned char   level; // by helca
//	unsigned char   minLimit;  // minor 장애 임계값 -> 이 값을 초과하여 duration 이상 지속되면 장애로 ?   unsigned char   majLimit;  // major 장애 임계값
//	unsigned char   criLimit;  // critical 장애 임계값
//	unsigned char   minDurat;  //
//	unsigned char   majDurat;  //
//	unsigned char   criDurat;  //
//	SFM_HwAlmFlag_s rsrcFlag;  // by helca	
//	SFM_HwAlmFlag_s_N2H(&sysRSRCInfo->rsrcFlag);  // by helca	
}

void SFM_NTPSts_N2H(SFM_NTPSts *NTPSts)
{
//	unsigned char mask;
//    unsigned char status;
//    unsigned char level;
}


void SFM_SysSpecInfo_N2H(SFM_SysSpecInfo *SysSpecInfo)
{
	//union {
		SFM_SpecInfoSMS_N2H(&(SysSpecInfo->u.sms));
	//} u;
}

void SFM_HpUxHWInfo_N2H(SFM_HpUxHWInfo *hpUxHWInfo)
{
	int i;
	for(i=0; i<SFM_MAX_HPUX_HW_COM; i++)
		SFM_HpUxHWInfo_s_N2H(&hpUxHWInfo->hwcom[i]);
}

void SFM_HpUxHWInfo_s_N2H(SFM_HpUxHWInfo_s *HpUxHWInfo_s)
{
//	unsigned char   mask;
//	unsigned char   status;
//	unsigned char   prevStatus;
//	unsigned char   level;
//	char        name[COMM_MAX_NAME_LEN]
}

void SFM_SpecInfoSMS_N2H(SFM_SpecInfoSMS *diskInfo)
{
	SFM_HpUxHWInfo_N2H(&diskInfo->hpuxHWInfo);
//  SFM_NetMonInfo      netMon[SFM_MAX_NET_MON_CNT]; // altibase, openCall 접속 정보. 
//  SFM_DBSyncInfo      sync;
//  unsigned char       lanCnt;   // 관리대상 lan 갯수
}


void SFM_SysSuccRateIpInfo_N2H(SFM_SysSuccRateIpInfo *SuccRateIpInfo)
{
	int i;
	for(i=0; i<MAX_WAPGW_NUM; i++)
		SuccRateIpInfo_N2H(&SuccRateIpInfo->uawap[i]);
		
	for(i=0; i<MAX_AAA_NUM; i++)
		SuccRateIpInfo_N2H(&SuccRateIpInfo->aaa[i]);
	
	for(i=0; i<MAX_ANAAA_NUM; i++)
		SuccRateIpInfo_N2H(&SuccRateIpInfo->anaaa[i]);
		
	for(i=0; i<RADIUS_IP_CNT; i++)
		SuccRate_RadiusInfo_N2H(&SuccRateIpInfo->radius[i]);
			
}

void SuccRateIpInfo_N2H(SuccRateIpInfo *rateIpInfo)
{
//	unigned char   mask;
//	unsigned int    ipAddr;
	rateIpInfo->ipAddr = (unsigned int)ntohl(rateIpInfo->ipAddr);	
//	unsigned int    count; /* count occured */
//	unsigned int    rate;  /* success rate, unit:percent */
//	unsigned char   level;
}

void SuccRate_RadiusInfo_N2H(SuccRate_RadiusInfo *radiusInfo)
{
//	unsigned char   mask;
//	char            ipAddr[SFM_MAX_TARGET_IP_LEN];
//	unsigned int    count; /* count occured */
	radiusInfo->count = (unsigned int)ntohl(radiusInfo->count);	
//	unsigned char   level;
}


void SFM_HwAlmFlag_N2H(SFM_HwAlmFlag *HwAlmFlag)
{
	int i=0;
/*
	for(i=0;i<SFM_MAX_CPU_CNT;i++)
   		SFM_HwAlmFlag_s_N2H(&HwAlmFlag->cpu[i]);

	SFM_HwAlmFlag_s_N2H(&HwAlmFlag->mem);

	for(i=0;i<SFM_MAX_DISK_CNT;i++)
   		SFM_HwAlmFlag_s_N2H(&HwAlmFlag->cpu[i]);

	SFM_HwAlmFlag_s_N2H(&HwAlmFlag->db); 
   		
  	for(i=0;i<SFM_MAX_QUE_CNT;i++)
   		SFM_HwAlmFlag_s_N2H(&HwAlmFlag->cpu[i]);
*/
}



void SFM_HwAlmFlag_s_N2H(SFM_HwAlmFlag_s *hwAlmFlag_s)
{
//	unsigned char   minFlag;  // minor 장애 발생되면 setting된다. 등급이 하강하는 경우 해지 메시지를 보
//    unsigned char   majFlag;  // major 장애 발생되면 setting된다. 등급이 하강하는 경우 해지 메시지를 보
//    unsigned char   criFlag;  // critical 장애 발생되면 setting된다. 등급이 하강하는 경우 해지 메시지를
}


void SFM_NMSInfo_N2H(SFM_NMSInfo *NMSInfo)
{
//	char    mask[MAX_NMS_CON];
//	char    level[MAX_NMS_CON];
//	int fd[MAX_NMS_CON];    // listen fd or accept fd
	int i;
	for(i=0;i<MAX_NMS_CON;i++)
		NMSInfo->fd[i] = (int)ntohl(NMSInfo->fd[i]);	
		
	for(i=0;i<MAX_NMS_CON;i++)
		NMSInfo->port[i] = (int)ntohl(NMSInfo->port[i]);	

	for(i=0;i<MAX_NMS_CON;i++)
		NMSInfo->ptype[i] = (int)ntohl(NMSInfo->ptype[i]);	

	for(i=0;i<MAX_NMS_CON;i++)
		NMSInfo->prev_ptype[i] = (int)ntohl(NMSInfo->prev_ptype[i]);	

	for(i=0;i<MAX_NMS_CON;i++)
		NMSInfo->rxTime[i] = (int)ntohl(NMSInfo->rxTime[i]);	

		
//	int port[MAX_NMS_CON];  // listen port
//	int ipaddr[MAX_NMS_CON];    // local ipaddr or nms ipaddr
//	int ptype[MAX_NMS_CON]; // fd type (FD_TYPE_LISTEN/~DATA)
//	int prev_ptype[MAX_NMS_CON];// prev fd type (FD_TYPE_LISTEN/~DATA)
//	int rxTime[MAX_NMS_CON];    // recent time to receive pkt
}



/*########################### Host To Network  ##############################*/

void SFM_SysInfo_H2N(SFM_SysInfo *sysInfo)   
{
    //SFM_SysAlmInfo_H2N(&sysInfo->almInfo);
    SFM_SysCommInfo_H2N(&sysInfo->commInfo);
    SFM_SysSpecInfo_H2N(&sysInfo->specInfo);
    SFM_SysSuccRateIpInfo_H2N(&sysInfo->succRateIpInfo);
}

void SFM_SysAlmInfo_H2N(SFM_SysAlmInfo *sysMsg)
{
	//unsigned char   level;     // 해당 시스템의 전체 장애 등급
	//unsigned char   prevLevel; // previous 해당 시스템의 전체 장애 등급
	//unsigned char   minCnt;    // 현재 발생되어 있는 minor 장애 갯수
	//unsigned char   majCnt;    // 현재 발생되어 있는 major 장애 갯수
	//unsigned char   criCnt;    // 현재 발생되어 있는 critical 장애 갯수
}

void SFM_SysCommInfo_H2N(SFM_SysCommInfo *sysInfo)
{
		int i;
//		char        type        [COMM_MAX_NAME_LEN];  // system type
//		char        group       [COMM_MAX_NAME_LEN]; // system group name
//		char        name        [COMM_MAX_NAME_LEN];  // system name
//		unsigned char   cpuCnt;   // 관리대상 cpu 갯수
//		unsigned char   diskCnt;  // 관리대상 disk partition 갯수
//		unsigned char   lanCnt;   // 관리대상 lan 갯수
//		unsigned char   procCnt;  // 관리대상 process 갯수
//		unsigned char   queCnt;   // 관리대상 queue 갯수    added 2004/02/04
    //unsigned short  total_disk_usage;
    sysInfo->total_disk_usage = (unsigned short)htons(sysInfo->total_disk_usage);

    //unsigned char   rmtLanCnt;
    //unsigned short  sessLoad[SFM_MAX_SESSION_CNT]; // 현재 사용 하지 않음.
    for(i=0;i<SFM_MAX_SESSION_CNT;i++)
		sysInfo->sessLoad[i] = (unsigned short)htons(sysInfo->sessLoad[i]);    

    //SFM_CpuInfo cpuInfo;
    SFM_CpuInfo_H2N(&sysInfo->cpuInfo);

    //SFM_MemInfo memInfo;
    SFM_MemInfo_H2N(&sysInfo->memInfo);    

    //SFM_DiskInfo    diskInfo    [SFM_MAX_DISK_CNT];
    for(i=0;i<SFM_MAX_DISK_CNT;i++)
		SFM_DiskInfo_H2N(&sysInfo->diskInfo[i]);

//    SFM_LanInfo lanInfo     [SFM_MAX_LAN_CNT];
 //   for(i=0;i<SFM_MAX_LAN_CNT;i++)
//		SFM_LanInfo_H2N(&sysInfo->lanInfo[i]);

//    SFM_ProcInfo    procInfo    [SFM_MAX_PROC_CNT];
    for(i=0;i<SFM_MAX_PROC_CNT;i++)
		SFM_ProcInfo_H2N(&sysInfo->procInfo[i]);

//    SFM_QueInfo queInfo     [SFM_MAX_QUE_CNT];          // added 2004/02/04
    for(i=0;i<SFM_MAX_QUE_CNT;i++)
		SFM_QueInfo_H2N(&sysInfo->queInfo[i]);

//    SFM_LanInfo rmtLanInfo  [SFM_MAX_RMT_LAN_CNT];          // 원격지 장비 통신 관련
//    for(i=0;i<SFM_MAX_RMT_LAN_CNT;i++)
//		SFM_LanInfo_H2N(&sysInfo->rmtLanInfo[i]);		

//    SFM_LanInfo optLanInfo  [2];                    // 광통신 관련 정보
 //   for(i=0;i<2;i++)
//		SFM_LanInfo_H2N(&sysInfo->optLanInfo[i]);	

//    SFM_SysDupSts   systemDup;	
	SFM_SysDupSts_H2N(&sysInfo->systemDup);
//    SFM_SysSuccRate succRate    [SFM_MAX_SUCC_RATE_CNT];
    for(i=0;i<SFM_MAX_SUCC_RATE_CNT;i++)
		SFM_SysSuccRate_H2N(&sysInfo->succRate[i]);
//    SFM_SysDBSts    rmtDbSts    [SFM_MAX_DB_CNT];
    for(i=0;i<SFM_MAX_DB_CNT;i++)
		SFM_SysDBSts_H2N(&sysInfo->rmtDbSts[i]);
//    SFM_SysRSRCInfo rsrcSts         [SFM_MAX_RSRC_LOAD_CNT];
    for(i=0;i<SFM_MAX_RSRC_LOAD_CNT;i++)
		SFM_SysRSRCInfo_H2N(&sysInfo->rsrcSts[i]);
//    SFM_NTPSts  ntpSts      [MAX_HW_NTP];	
//    for(i=0;i<MAX_HW_NTP;i++)
//		SFM_NTPSts_H2N(&sysInfo->ntpSts[i]);
		
}

void SFM_CpuInfo_H2N(SFM_CpuInfo *cpuInfo)
{
	int i;
//	unsigned char   mask[SFM_MAX_CPU_CNT];
//    unsigned short  usage[SFM_MAX_CPU_CNT];     // 현재 CPU 사용율
	for(i=0;i<SFM_MAX_CPU_CNT;i++)
    	cpuInfo->usage[i] = (unsigned short)htons(cpuInfo->usage[i]);
//    unsigned char   level[SFM_MAX_CPU_CNT];     // 현재 CPU 장애 등급
//    unsigned char   minLimit;  // minor 장애 임계값 -> 이 값을 초과하여 duration 이상 지속되면 장애로 판정한다.
//    unsigned char   majLimit;  // major 장애 임계값
//    unsigned char   criLimit;  // critical 장애 임계값
//    unsigned char   minDurat;  //
//    unsigned char   majDurat;  //
//    unsigned char   criDurat;  //
}

void SFM_MemInfo_H2N(SFM_MemInfo *memInfo)
{
//    unsigned char   mask;
//    unsigned short  usage;     // 현재 memory 사용율
	  memInfo->usage = (unsigned short)htons(memInfo->usage);
//    unsigned char   level;     // 현재 memory 장애 등급
//    unsigned char   minLimit;  // minor 장애 임계값 -> 이 값을 초과하여 duration 이상 지속되면 장애로 판정한다.
//    unsigned char   majLimit;  // major 장애 임계값
//    unsigned char   criLimit;  // critical 장애 임계값
//    unsigned char   minDurat;  //
//    unsigned char   majDurat;  //
//    unsigned char   criDurat;  //	
}

void SFM_DiskInfo_H2N(SFM_DiskInfo *diskInfo)
{
//   unsigned char   mask;
//   char            name[SFM_MAX_DISK_NAME]; // disk partition name
//   unsigned short  usage;     // 현재 사용율
	 diskInfo->usage = (unsigned short)htons(diskInfo->usage);
//   unsigned char   level;     // 현재 장애 등급
//   unsigned char   minLimit;  // minor 장애 임계값
//   unsigned char   majLimit;  // major 장애 임계값
//   unsigned char   criLimit;  // critical 장애 임계값
}

void SFM_LanInfo_H2N(SFM_LanInfo *LanInfo)
{
//	char            mask;
//    char            name[SFM_MAX_LAN_NAME_LEN]; //
//    char            targetIp[SFM_MAX_TARGET_IP_LEN]; //
//    unsigned char   status;     // 현재 상태
//    unsigned char   prevStatus; // 이전 상태
//    unsigned char   level;      // 장애 발생 시 해당 프로세스에 적용될 장애 등급
}

void SFM_ProcInfo_H2N(SFM_ProcInfo *procInfo)
{
//	  unsigned char   mask;
//    char            name[COMM_MAX_NAME_LEN]; //
//    unsigned char   status;     // 현재 상태
//    unsigned char   prevStatus; // 이전 상태
//    unsigned char   level;      // 장애 발생 시 해당 프로세스에 적용될 장애 등급
//    pid_t           pid;
	procInfo->pid = (unsigned int)htonl(procInfo->pid);
//    time_t          uptime;
	procInfo->uptime = (unsigned int)htonl(procInfo->uptime);
}

void SFM_QueInfo_H2N(SFM_QueInfo *queInfo)
{
//	int     qID;
	queInfo->qID = (int)htonl(queInfo->qID);
//	int     qKEY;
	queInfo->qKEY = (int)htonl(queInfo->qKEY);
//	unsigned char   mask;
//	char            qNAME[QUE_MAX_NAME_LEN];
//	unsigned int    qNUM;
	queInfo->qNUM = (unsigned int)htonl(queInfo->qNUM);
//	unsigned int    cBYTES;
	queInfo->cBYTES = (unsigned int)htonl(queInfo->cBYTES);
//	unsigned int    qBYTES;
	queInfo->qBYTES = (unsigned int)htonl(queInfo->qBYTES);
//	unsigned int    load;     // 현재 사용율
	queInfo->load = (unsigned int)htonl(queInfo->load);
//	unsigned char   level;     // 현재 장애 등급
//	unsigned char   minLimit;  // minor 장애 임계값
//	unsigned char   majLimit;  // major 장애 임계값
//	unsigned char   criLimit;  // critical 장애 임계값	
}

void SFM_SysDupSts_H2N(SFM_SysDupSts *sysDupSts)
{
//	unsigned char   mask;
//	unsigned char   myStatus;   /* 1 : ACTIVE, 2 : STANDBY */
//	unsigned char   yourStatus; /* 1 : ACTIVE, 2 : STANDBY */
//	unsigned char   sizeMin[17];
//	long long   llCorelationId;
	sysDupSts->llCorelationId = (long long)htonl(sysDupSts->llCorelationId);
//	unsigned int    uiTrsId;
//	unsigned int    uiUdrSeq;
//	unsigned char   heartbeatAlm;       /* 1 : Normal, 2 : AbNormal(alarm) */
//	unsigned char   timeOutAlm;         /* 1 : Normal, 2 : AbNormal(alarm) */
//	unsigned char   heartbeatLevel;
//	unsigned char   timeOutAlmLevel;
//	unsigned char   dualStsAlmLevel;
//	unsigned char   oosAlm;         /* 1 : Normal, 2 : Out-of-Service occured(alarm) */
}
void SFM_SysSuccRate_H2N(SFM_SysSuccRate *sysSuccRate)
{
//unsigned char   mask;
//unsigned char   name[COMM_MAX_NAME_LEN];
//unsigned int    cnt; /* count occured */
	sysSuccRate->cnt = (unsigned int)htonl(sysSuccRate->cnt);
//unsigned int    rate;  /* success rate, unit:percent */
//unsigned char   level;
}
void SFM_SysDBSts_H2N(SFM_SysDBSts *sysDBSts)
{
//    unsigned char   mask;
//    unsigned char   sDbAlias[20];
//    unsigned char   sIpAddress[16];
//    unsigned int    iStatus; /* 1 - connect, 0 - not connect */
	sysDBSts->iStatus = (unsigned int)htonl(sysDBSts->iStatus);
}

void SFM_SysRSRCInfo_H2N(SFM_SysRSRCInfo *sysRSRCInfo)
{
//	unsigned char   mask;
//	unsigned int    rsrcload;
	sysRSRCInfo->rsrcload = (unsigned int)htonl(sysRSRCInfo->rsrcload);
//	unsigned char   level; // by helca
//	unsigned char   minLimit;  // minor 장애 임계값 -> 이 값을 초과하여 duration 이상 지속되면 장애로 ?   unsigned char   majLimit;  // major 장애 임계값
//	unsigned char   criLimit;  // critical 장애 임계값
//	unsigned char   minDurat;  //
//	unsigned char   majDurat;  //
//	unsigned char   criDurat;  //
//	SFM_HwAlmFlag_s rsrcFlag;  // by helca	
//	SFM_HwAlmFlag_s_H2N(&sysRSRCInfo->rsrcFlag);  // by helca	
}

void SFM_NTPSts_H2N(SFM_NTPSts *NTPSts)
{
//	unsigned char mask;
//    unsigned char status;
//    unsigned char level;
}


void SFM_SysSpecInfo_H2N(SFM_SysSpecInfo *SysSpecInfo)
{
	//union {
		SFM_SpecInfoSMS_H2N(&(SysSpecInfo->u.sms));
	//} u;
}

void SFM_HpUxHWInfo_H2N(SFM_HpUxHWInfo *hpUxHWInfo)
{
	int i;
	for(i=0; i<SFM_MAX_HPUX_HW_COM; i++)
		SFM_HpUxHWInfo_s_H2N(&hpUxHWInfo->hwcom[i]);
}

void SFM_HpUxHWInfo_s_H2N(SFM_HpUxHWInfo_s *HpUxHWInfo_s)
{
//	unsigned char   mask;
//	unsigned char   status;
//	unsigned char   prevStatus;
//	unsigned char   level;
//	char        name[COMM_MAX_NAME_LEN]
}

void SFM_SpecInfoSMS_H2N(SFM_SpecInfoSMS *diskInfo)
{
	SFM_HpUxHWInfo_H2N(&diskInfo->hpuxHWInfo);
//  SFM_NetMonInfo      netMon[SFM_MAX_NET_MON_CNT]; // altibase, openCall 접속 정보. 
//  SFM_DBSyncInfo      sync;
//  unsigned char       lanCnt;   // 관리대상 lan 갯수
}


void SFM_SysSuccRateIpInfo_H2N(SFM_SysSuccRateIpInfo *SuccRateIpInfo)
{
	int i;
	for(i=0; i<MAX_WAPGW_NUM; i++)
		SuccRateIpInfo_H2N(&SuccRateIpInfo->uawap[i]);
		
	for(i=0; i<MAX_AAA_NUM; i++)
		SuccRateIpInfo_H2N(&SuccRateIpInfo->aaa[i]);
	
	for(i=0; i<MAX_ANAAA_NUM; i++)
		SuccRateIpInfo_H2N(&SuccRateIpInfo->anaaa[i]);
		
	for(i=0; i<RADIUS_IP_CNT; i++)
		SuccRate_RadiusInfo_H2N(&SuccRateIpInfo->radius[i]);
			
}

void SuccRateIpInfo_H2N(SuccRateIpInfo *rateIpInfo)
{
//	unigned char   mask;
//	unsigned int    ipAddr;
	rateIpInfo->ipAddr = (unsigned int)htonl(rateIpInfo->ipAddr);	
//	unsigned int    count; /* count occured */
//	unsigned int    rate;  /* success rate, unit:percent */
//	unsigned char   level;
}

void SuccRate_RadiusInfo_H2N(SuccRate_RadiusInfo *radiusInfo)
{
//	unsigned char   mask;
//	char            ipAddr[SFM_MAX_TARGET_IP_LEN];
//	unsigned int    count; /* count occured */
	radiusInfo->count = (unsigned int)htonl(radiusInfo->count);	
//	unsigned char   level;
}


void SFM_HwAlmFlag_H2N(SFM_HwAlmFlag *HwAlmFlag)
{
	int i=0;
/*
	for(i=0;i<SFM_MAX_CPU_CNT;i++)
   		SFM_HwAlmFlag_s_H2N(&HwAlmFlag->cpu[i]);

	SFM_HwAlmFlag_s_H2N(&HwAlmFlag->mem);

	for(i=0;i<SFM_MAX_DISK_CNT;i++)
   		SFM_HwAlmFlag_s_H2N(&HwAlmFlag->cpu[i]);

	SFM_HwAlmFlag_s_H2N(&HwAlmFlag->db); 
   		
  	for(i=0;i<SFM_MAX_QUE_CNT;i++)
   		SFM_HwAlmFlag_s_H2N(&HwAlmFlag->cpu[i]);
*/
}



void SFM_HwAlmFlag_s_H2N(SFM_HwAlmFlag_s *hwAlmFlag_s)
{
//	unsigned char   minFlag;  // minor 장애 발생되면 setting된다. 등급이 하강하는 경우 해지 메시지를 보
//    unsigned char   majFlag;  // major 장애 발생되면 setting된다. 등급이 하강하는 경우 해지 메시지를 보
//    unsigned char   criFlag;  // critical 장애 발생되면 setting된다. 등급이 하강하는 경우 해지 메시지를
}


void SFM_NMSInfo_H2N(SFM_NMSInfo *NMSInfo)
{
//	char    mask[MAX_NMS_CON];
//	char    level[MAX_NMS_CON];
//	int fd[MAX_NMS_CON];    // listen fd or accept fd
	int i;
	for(i=0;i<MAX_NMS_CON;i++)
		NMSInfo->fd[i] = (int)htonl(NMSInfo->fd[i]);	
		
	for(i=0;i<MAX_NMS_CON;i++)
		NMSInfo->port[i] = (int)htonl(NMSInfo->port[i]);	

	for(i=0;i<MAX_NMS_CON;i++)
		NMSInfo->ptype[i] = (int)htonl(NMSInfo->ptype[i]);	

	for(i=0;i<MAX_NMS_CON;i++)
		NMSInfo->prev_ptype[i] = (int)htonl(NMSInfo->prev_ptype[i]);	

	for(i=0;i<MAX_NMS_CON;i++)
		NMSInfo->rxTime[i] = (int)htonl(NMSInfo->rxTime[i]);	

		
//	int port[MAX_NMS_CON];  // listen port
//	int ipaddr[MAX_NMS_CON];    // local ipaddr or nms ipaddr
//	int ptype[MAX_NMS_CON]; // fd type (FD_TYPE_LISTEN/~DATA)
//	int prev_ptype[MAX_NMS_CON];// prev fd type (FD_TYPE_LISTEN/~DATA)
//	int rxTime[MAX_NMS_CON];    // recent time to receive pkt
}


/*########################### Debug print  ##############################*/

void SFM_SysInfo_PRINT(SFM_SysInfo *sysInfo)   
{
    //SFM_SysAlmInfo_PRINT(&sysInfo->almInfo);
    SFM_SysCommInfo_PRINT(&sysInfo->commInfo);
    SFM_SysSpecInfo_PRINT(&sysInfo->specInfo);
    SFM_SysSuccRateIpInfo_PRINT(&sysInfo->succRateIpInfo);
}

void SFM_SysAlmInfo_PRINT(SFM_SysAlmInfo *sysMsg)
{
	//unsigned char   level;     // 해당 시스템의 전체 장애 등급
	//unsigned char   prevLevel; // previous 해당 시스템의 전체 장애 등급
	//unsigned char   minCnt;    // 현재 발생되어 있는 minor 장애 갯수
	//unsigned char   majCnt;    // 현재 발생되어 있는 major 장애 갯수
	//unsigned char   criCnt;    // 현재 발생되어 있는 critical 장애 갯수
}

void SFM_SysCommInfo_PRINT(SFM_SysCommInfo *sysInfo)
{
	int i;
    //sysInfo->total_disk_usage = (unsigned short)htons(sysInfo->total_disk_usage);
	fprintf(stdout,"%s:%d] total_disk_usage: %d\n",__FILE__, __LINE__,
			(int)sysInfo->total_disk_usage);

    for(i=0;i<SFM_MAX_SESSION_CNT;i++)
		fprintf(stdout,"%s:%d] sessLoad: %d\n",__FILE__, __LINE__, (int)sysInfo->sessLoad[i]);
		//sysInfo->sessLoad[i] = (unsigned short)htons(sysInfo->sessLoad[i]);    

    //SFM_CpuInfo cpuInfo;
    SFM_CpuInfo_PRINT(&sysInfo->cpuInfo);
    //SFM_MemInfo memInfo;
    SFM_MemInfo_PRINT(&sysInfo->memInfo);    
    //SFM_DiskInfo    diskInfo    [SFM_MAX_DISK_CNT];
    for(i=0;i<SFM_MAX_DISK_CNT;i++)
		SFM_DiskInfo_PRINT(&sysInfo->diskInfo[i]);

    for(i=0;i<SFM_MAX_PROC_CNT;i++)
		SFM_ProcInfo_PRINT(&sysInfo->procInfo[i]);
//    SFM_QueInfo queInfo     [SFM_MAX_QUE_CNT];          // added 2004/02/04
		
    for(i=0;i<SFM_MAX_QUE_CNT;i++)
		SFM_QueInfo_PRINT(&sysInfo->queInfo[i]);
//    SFM_LanInfo rmtLanInfo  [SFM_MAX_RMT_LAN_CNT];          // 원격지 장비 통신 관련
//    for(i=0;i<SFM_MAX_RMT_LAN_CNT;i++)
//		SFM_LanInfo_PRINT(&sysInfo->rmtLanInfo[i]);		
//    SFM_LanInfo optLanInfo  [2];                    // 광통신 관련 정보
 //   for(i=0;i<2;i++)
//		SFM_LanInfo_PRINT(&sysInfo->optLanInfo[i]);	
//    SFM_SysDupSts   systemDup;	
	SFM_SysDupSts_PRINT(&sysInfo->systemDup);
//    SFM_SysSuccRate succRate    [SFM_MAX_SUCC_RATE_CNT];
    for(i=0;i<SFM_MAX_SUCC_RATE_CNT;i++)
		SFM_SysSuccRate_PRINT(&sysInfo->succRate[i]);
//    SFM_SysDBSts    rmtDbSts    [SFM_MAX_DB_CNT];
    for(i=0;i<SFM_MAX_DB_CNT;i++)
		SFM_SysDBSts_PRINT(&sysInfo->rmtDbSts[i]);
//    SFM_SysRSRCInfo rsrcSts         [SFM_MAX_RSRC_LOAD_CNT];
    for(i=0;i<SFM_MAX_RSRC_LOAD_CNT;i++)
		SFM_SysRSRCInfo_PRINT(&sysInfo->rsrcSts[i]);
//    SFM_NTPSts  ntpSts      [MAX_HW_NTP];	
//    for(i=0;i<MAX_HW_NTP;i++)
//		SFM_NTPSts_PRINT(&sysInfo->ntpSts[i]);
		
}

void SFM_CpuInfo_PRINT(SFM_CpuInfo *cpuInfo)
{
	int i;
//	unsigned char   mask[SFM_MAX_CPU_CNT];
//    unsigned short  usage[SFM_MAX_CPU_CNT];     // 현재 CPU 사용율
	for(i=0;i<SFM_MAX_CPU_CNT;i++)
		fprintf(stdout,"%s:%d] usage: %d\n",__FILE__, __LINE__, (int)cpuInfo->usage[i]);
    //	cpuInfo->usage[i] = (unsigned short)htons(cpuInfo->usage[i]);
//    unsigned char   level[SFM_MAX_CPU_CNT];     // 현재 CPU 장애 등급
//    unsigned char   minLimit;  // minor 장애 임계값 -> 이 값을 초과하여 duration 이상 지속되면 장애로 판정한다.
//    unsigned char   majLimit;  // major 장애 임계값
//    unsigned char   criLimit;  // critical 장애 임계값
//    unsigned char   minDurat;  //
//    unsigned char   majDurat;  //
//    unsigned char   criDurat;  //
}

void SFM_MemInfo_PRINT(SFM_MemInfo *memInfo)
{
//    unsigned char   mask;
//    unsigned short  usage;     // 현재 memory 사용율
	 // memInfo->usage = (unsigned short)htons(memInfo->usage);
	  fprintf(stdout,"%s:%d] usage: %d\n",__FILE__, __LINE__, (int)memInfo->usage);
//    unsigned char   level;     // 현재 memory 장애 등급
//    unsigned char   minLimit;  // minor 장애 임계값 -> 이 값을 초과하여 duration 이상 지속되면 장애로 판정한다.
//    unsigned char   majLimit;  // major 장애 임계값
//    unsigned char   criLimit;  // critical 장애 임계값
//    unsigned char   minDurat;  //
//    unsigned char   majDurat;  //
//    unsigned char   criDurat;  //	
}

void SFM_DiskInfo_PRINT(SFM_DiskInfo *diskInfo)
{
//   unsigned char   mask;
//   char            name[SFM_MAX_DISK_NAME]; // disk partition name
//   unsigned short  usage;     // 현재 사용율
	 //diskInfo->usage = (unsigned short)htons(diskInfo->usage);
	 fprintf(stdout,"%s:%d] usage: %d\n",__FILE__, __LINE__, (int)diskInfo->usage);
//   unsigned char   level;     // 현재 장애 등급
//   unsigned char   minLimit;  // minor 장애 임계값
//   unsigned char   majLimit;  // major 장애 임계값
//   unsigned char   criLimit;  // critical 장애 임계값
}

void SFM_LanInfo_PRINT(SFM_LanInfo *LanInfo)
{
//	char            mask;
//    char            name[SFM_MAX_LAN_NAME_LEN]; //
//    char            targetIp[SFM_MAX_TARGET_IP_LEN]; //
//    unsigned char   status;     // 현재 상태
//    unsigned char   prevStatus; // 이전 상태
//    unsigned char   level;      // 장애 발생 시 해당 프로세스에 적용될 장애 등급
}

void SFM_ProcInfo_PRINT(SFM_ProcInfo *procInfo)
{
//	  unsigned char   mask;
//    char            name[COMM_MAX_NAME_LEN]; //
//    unsigned char   status;     // 현재 상태
//    unsigned char   prevStatus; // 이전 상태
//    unsigned char   level;      // 장애 발생 시 해당 프로세스에 적용될 장애 등급
//    pid_t           pid;
	//procInfo->pid = (unsigned int)htonl(procInfo->pid);
	 fprintf(stdout,"%s:%d] pid: %d\n",__FILE__, __LINE__, (int)procInfo->pid);
//    time_t          uptime;
	 fprintf(stdout,"%s:%d] uptime: %d\n",__FILE__, __LINE__, (int)procInfo->uptime);
}

void SFM_QueInfo_PRINT(SFM_QueInfo *queInfo)
{
//	int     qID;
	//queInfo->qID = (int)htonl(queInfo->qID);
	 fprintf(stdout,"%s:%d] qID: %d\n",__FILE__, __LINE__, (int)queInfo->qID);
//	int     qKEY;
	//queInfo->qKEY = (int)htonl(queInfo->qKEY);
	 fprintf(stdout,"%s:%d] qKEY: %d\n",__FILE__, __LINE__, (int)queInfo->qKEY);
//	unsigned char   mask;
//	char            qNAME[QUE_MAX_NAME_LEN];
//	unsigned int    qNUM;
	//queInfo->qNUM = (unsigned int)htonl(queInfo->qNUM);
	 fprintf(stdout,"%s:%d] qNUM: %d\n",__FILE__, __LINE__, (int)queInfo->qNUM);
//	unsigned int    cBYTES;
	//queInfo->cBYTES = (unsigned int)htonl(queInfo->cBYTES);
	 fprintf(stdout,"%s:%d] cBYTES: %d\n",__FILE__, __LINE__, (int)queInfo->cBYTES);
//	unsigned int    qBYTES;
	//queInfo->qBYTES = (unsigned int)htonl(queInfo->qBYTES);
	 fprintf(stdout,"%s:%d] qBYTES: %d\n",__FILE__, __LINE__, (int)queInfo->qBYTES);
//	unsigned int    load;     // 현재 사용율
	//queInfo->load = (unsigned int)htonl(queInfo->load);
	 fprintf(stdout,"%s:%d] load: %d\n",__FILE__, __LINE__, (int)queInfo->load);
//	unsigned char   level;     // 현재 장애 등급
//	unsigned char   minLimit;  // minor 장애 임계값
//	unsigned char   majLimit;  // major 장애 임계값
//	unsigned char   criLimit;  // critical 장애 임계값	
}

void SFM_SysDupSts_PRINT(SFM_SysDupSts *sysDupSts)
{
//	unsigned char   mask;
//	unsigned char   myStatus;   /* 1 : ACTIVE, 2 : STANDBY */
//	unsigned char   yourStatus; /* 1 : ACTIVE, 2 : STANDBY */
//	unsigned char   sizeMin[17];
//	long long   llCorelationId;
//	sysDupSts->llCorelationId = (long long)htonl(sysDupSts->llCorelationId);
	 fprintf(stdout,"%s:%d] llCorelationId: %d\n",__FILE__, __LINE__, (int)sysDupSts->llCorelationId);
//	unsigned int    uiTrsId;
//	unsigned int    uiUdrSeq;
//	unsigned char   heartbeatAlm;       /* 1 : Normal, 2 : AbNormal(alarm) */
//	unsigned char   timeOutAlm;         /* 1 : Normal, 2 : AbNormal(alarm) */
//	unsigned char   heartbeatLevel;
//	unsigned char   timeOutAlmLevel;
//	unsigned char   dualStsAlmLevel;
//	unsigned char   oosAlm;         /* 1 : Normal, 2 : Out-of-Service occured(alarm) */
}
void SFM_SysSuccRate_PRINT(SFM_SysSuccRate *sysSuccRate)
{
//unsigned char   mask;
//unsigned char   name[COMM_MAX_NAME_LEN];
//unsigned int    cnt; /* count occured */
	//sysSuccRate->cnt = (unsigned int)htonl(sysSuccRate->cnt);
	 fprintf(stdout,"%s:%d] cnt: %d\n",__FILE__, __LINE__, (int)sysSuccRate->cnt);
//unsigned int    rate;  /* success rate, unit:percent */
//unsigned char   level;
}
void SFM_SysDBSts_PRINT(SFM_SysDBSts *sysDBSts)
{
//    unsigned char   mask;
//    unsigned char   sDbAlias[20];
//    unsigned char   sIpAddress[16];
//    unsigned int    iStatus; /* 1 - connect, 0 - not connect */
	//sysDBSts->iStatus = (unsigned int)htonl(sysDBSts->iStatus);
	 fprintf(stdout,"%s:%d] iStatus: %d\n",__FILE__, __LINE__, (int)sysDBSts->iStatus);
}

void SFM_SysRSRCInfo_PRINT(SFM_SysRSRCInfo *sysRSRCInfo)
{
//	unsigned char   mask;
//	unsigned int    rsrcload;
	//sysRSRCInfo->rsrcload = (unsigned int)htonl(sysRSRCInfo->rsrcload);
	 fprintf(stdout,"%s:%d] rsrcload: %d\n",__FILE__, __LINE__, (int)sysRSRCInfo->rsrcload);
//	unsigned char   level; // by helca
//	unsigned char   minLimit;  // minor 장애 임계값 -> 이 값을 초과하여 duration 이상 지속되면 장애로 ?   unsigned char   majLimit;  // major 장애 임계값
//	unsigned char   criLimit;  // critical 장애 임계값
//	unsigned char   minDurat;  //
//	unsigned char   majDurat;  //
//	unsigned char   criDurat;  //
//	SFM_HwAlmFlag_s rsrcFlag;  // by helca	
//	SFM_HwAlmFlag_s_PRINT(&sysRSRCInfo->rsrcFlag);  // by helca	
}

void SFM_NTPSts_PRINT(SFM_NTPSts *NTPSts)
{
//	unsigned char mask;
//    unsigned char status;
//    unsigned char level;
}


void SFM_SysSpecInfo_PRINT(SFM_SysSpecInfo *SysSpecInfo)
{
	//union {
		SFM_SpecInfoSMS_PRINT(&(SysSpecInfo->u.sms));
	//} u;
}

void SFM_HpUxHWInfo_PRINT(SFM_HpUxHWInfo *hpUxHWInfo)
{
	int i;
	for(i=0; i<SFM_MAX_HPUX_HW_COM; i++)
		SFM_HpUxHWInfo_s_PRINT(&hpUxHWInfo->hwcom[i]);
}

void SFM_HpUxHWInfo_s_PRINT(SFM_HpUxHWInfo_s *HpUxHWInfo_s)
{
//	unsigned char   mask;
//	unsigned char   status;
//	unsigned char   prevStatus;
//	unsigned char   level;
//	char        name[COMM_MAX_NAME_LEN]
}

void SFM_SpecInfoSMS_PRINT(SFM_SpecInfoSMS *diskInfo)
{
	SFM_HpUxHWInfo_PRINT(&diskInfo->hpuxHWInfo);
//  SFM_NetMonInfo      netMon[SFM_MAX_NET_MON_CNT]; // altibase, openCall 접속 정보. 
//  SFM_DBSyncInfo      sync;
//  unsigned char       lanCnt;   // 관리대상 lan 갯수
}


void SFM_SysSuccRateIpInfo_PRINT(SFM_SysSuccRateIpInfo *SuccRateIpInfo)
{
	int i;
	for(i=0; i<MAX_WAPGW_NUM; i++)
		SuccRateIpInfo_PRINT(&SuccRateIpInfo->uawap[i]);
		
	for(i=0; i<MAX_AAA_NUM; i++)
		SuccRateIpInfo_PRINT(&SuccRateIpInfo->aaa[i]);
	
	for(i=0; i<MAX_ANAAA_NUM; i++)
		SuccRateIpInfo_PRINT(&SuccRateIpInfo->anaaa[i]);
		
	for(i=0; i<RADIUS_IP_CNT; i++)
		SuccRate_RadiusInfo_PRINT(&SuccRateIpInfo->radius[i]);
			
}

void SuccRateIpInfo_PRINT(SuccRateIpInfo *rateIpInfo)
{
//	unigned char   mask;
//	unsigned int    ipAddr;
	//rateIpInfo->ipAddr = (unsigned int)htonl(rateIpInfo->ipAddr);	
	 fprintf(stdout,"%s:%d] ipAddr: %d\n",__FILE__, __LINE__, (int)rateIpInfo->ipAddr);
//	unsigned int    count; /* count occured */
//	unsigned int    rate;  /* success rate, unit:percent */
//	unsigned char   level;
}

void SuccRate_RadiusInfo_PRINT(SuccRate_RadiusInfo *radiusInfo)
{
//	unsigned char   mask;
//	char            ipAddr[SFM_MAX_TARGET_IP_LEN];
//	unsigned int    count; /* count occured */
	//radiusInfo->count = (unsigned int)htonl(radiusInfo->count);	
	 fprintf(stdout,"%s:%d] count: %d\n",__FILE__, __LINE__, (int)radiusInfo->count);
//	unsigned char   level;
}


void SFM_HwAlmFlag_PRINT(SFM_HwAlmFlag *HwAlmFlag)
{
	int i=0;
/*
	for(i=0;i<SFM_MAX_CPU_CNT;i++)
   		SFM_HwAlmFlag_s_PRINT(&HwAlmFlag->cpu[i]);

	SFM_HwAlmFlag_s_PRINT(&HwAlmFlag->mem);

	for(i=0;i<SFM_MAX_DISK_CNT;i++)
   		SFM_HwAlmFlag_s_PRINT(&HwAlmFlag->cpu[i]);

	SFM_HwAlmFlag_s_PRINT(&HwAlmFlag->db); 
   		
  	for(i=0;i<SFM_MAX_QUE_CNT;i++)
   		SFM_HwAlmFlag_s_PRINT(&HwAlmFlag->cpu[i]);
*/
}



void SFM_HwAlmFlag_s_PRINT(SFM_HwAlmFlag_s *hwAlmFlag_s)
{
//	unsigned char   minFlag;  // minor 장애 발생되면 setting된다. 등급이 하강하는 경우 해지 메시지를 보
//    unsigned char   majFlag;  // major 장애 발생되면 setting된다. 등급이 하강하는 경우 해지 메시지를 보
//    unsigned char   criFlag;  // critical 장애 발생되면 setting된다. 등급이 하강하는 경우 해지 메시지를
}


void SFM_NMSInfo_PRINT(SFM_NMSInfo *NMSInfo)
{
//	char    mask[MAX_NMS_CON];
//	char    level[MAX_NMS_CON];
//	int fd[MAX_NMS_CON];    // listen fd or accept fd
	int i;
	for(i=0;i<MAX_NMS_CON;i++)
	 	fprintf(stdout,"%s:%d] fd: %d\n",__FILE__, __LINE__, (int)NMSInfo->fd[i]);
	//	NMSInfo->fd[i] = (int)htonl(NMSInfo->fd[i]);	
		
	for(i=0;i<MAX_NMS_CON;i++)
	 	fprintf(stdout,"%s:%d] port: %d\n",__FILE__, __LINE__, (int)NMSInfo->port[i]);
		//NMSInfo->port[i] = (int)htonl(NMSInfo->port[i]);	

	for(i=0;i<MAX_NMS_CON;i++)
	 	fprintf(stdout,"%s:%d] ptype: %d\n",__FILE__, __LINE__, (int)NMSInfo->ptype[i]);
		//NMSInfo->ptype[i] = (int)htonl(NMSInfo->ptype[i]);	

	for(i=0;i<MAX_NMS_CON;i++)
	 	fprintf(stdout,"%s:%d] prev_ptype: %d\n",__FILE__, __LINE__, (int)NMSInfo->prev_ptype[i]);
		//NMSInfo->prev_ptype[i] = (int)htonl(NMSInfo->prev_ptype[i]);	

	for(i=0;i<MAX_NMS_CON;i++)
	 	fprintf(stdout,"%s:%d] rxTime: %d\n",__FILE__, __LINE__, (int)NMSInfo->rxTime[i]);
		//NMSInfo->rxTime[i] = (int)htonl(NMSInfo->rxTime[i]);	

		
//	int port[MAX_NMS_CON];  // listen port
//	int ipaddr[MAX_NMS_CON];    // local ipaddr or nms ipaddr
//	int ptype[MAX_NMS_CON]; // fd type (FD_TYPE_LISTEN/~DATA)
//	int prev_ptype[MAX_NMS_CON];// prev fd type (FD_TYPE_LISTEN/~DATA)
//	int rxTime[MAX_NMS_CON];    // recent time to receive pkt
}



