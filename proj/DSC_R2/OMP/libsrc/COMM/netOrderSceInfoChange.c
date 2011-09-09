#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>

#include "netOrderSceInfoChange.h"

/* hjjung ntohs에서 ntohl로 전부 변경 */

/*########################### Network To Host ##############################*/
/*SCE Main*/
void SFM_SCE_N2H(SFM_SCE *sce) 
{
	int i;
	for(i=0; i<MAX_SCE_DEV_NUM; i++)
	{
		SFM_SCEDev_N2H(&sce->SCEDev[i]);
	}
}

void SFM_SCEDev_N2H(SFM_SCEDev *sceDev)
{
	int i;
	SCE_SYS_INFO_N2H(&sceDev->sysInfo);
	
	for(i=0; i<MAX_SCE_CPU_CNT; i++)
		SCE_SYS_USAGE_INFO_N2H(&sceDev->cpuInfo[i]);
	
	for(i=0; i<MAX_SCE_MEM_CNT; i++)
		SCE_SYS_USAGE_INFO_N2H(&sceDev->memInfo[i]);
		
	for(i=0; i<MAX_SCE_MEM_CNT; i++)
		SCE_SYS_USAGE_INFO_N2H(&sceDev->flowlossInfo[i]);
	
	SCE_SYS_USAGE_INFO_N2H(&sceDev->diskInfo);

	/* hjjung_20100823 */
	SCE_SYS_NUM_INFO_N2H(&sceDev->userInfo);
//	SCE_SYS_STATUS_INFO_N2H(sceDev->sysStatus);
//	SCE_SYS_STATUS_INFO_N2H(sceDev->pwrStatus);
//	SCE_SYS_STATUS_INFO_N2H(sceDev->fanStatus);
//	SCE_SYS_STATUS_INFO_N2H(sceDev->tempStatus);
//	SCE_SYS_STATUS_INFO_N2H(sceDev->voltStatus);
//	SCE_SYS_STATUS_INFO_N2H(sceDev->portModuleStatus);
	
//	for(i=0; i<MAX_SCE_LINK_CNT; i++)
//		SCE_SYS_STATUS_INFO_N2H(&sceDev->portLinkStatus[i]);
		
//	for(i=0; i<MAX_SCE_IFN_CNT; i++)
//		SCE_SYS_STATUS_INFO_N2H(&sceDev->portStatus[i]);
	
//	SCE_SYS_STATUS_INFO_N2H(sceDev->rdrStatus);
//	SCE_SYS_STATUS_INFO_N2H(sceDev->rdrConnStatus);	

}

void SCE_SYS_INFO_N2H(SCE_SYS_INFO *sysInfo)
{
//	unsigned short  intro_user;
	sysInfo->intro_user = (unsigned short)ntohl(sysInfo->intro_user);
//	unsigned short  active_user;
	sysInfo->active_user= (unsigned short)ntohl(sysInfo->active_user);
//	unsigned char   version[32];	
} 

void SCE_SYS_USAGE_INFO_N2H(SCE_SYS_USAGE_INFO *sysUseInfo)
{
//    unsigned char   mask;       /* mml 로 설정된 alarm 발생 여부 설정 */
    unsigned short  usage;      /* 현재 memory 사용율 */
    sysUseInfo->usage = (unsigned short)ntohl(sysUseInfo->usage);
//    unsigned char   level;      /* 현재 memory 장애 등급 */
//    unsigned char   minLimit;   /* minor 장애 임계값 -> 이 값을 초과하여 duration 이상 지속되면 장애로     unsigned char   majLimit;   /* major 장애 임계값 */  
//    unsigned char   criLimit;   /* critical 장애 임계값 */
//    unsigned char   minDurat;   
//    unsigned char   majDurat;   
//    unsigned char   criDurat;   
//    unsigned char   minFlag;    /* minor 장애 발생되면 setting된다. 등급이 하강하는 경우 해지 메시지를     unsigned char   majFlag;    /* major 장애 발생되면 setting된다. 등급이 하강하는 경우 해지 메시지를     unsigned char   criFlag;    /* critical 장애 발생되면 setting된다. 등급이 하강하는 경우 해지 메시지
}

/* hjjung_20100823 */
void SCE_SYS_NUM_INFO_N2H(SCE_SYS_NUM_INFO *sysUseInfo)
{
    unsigned int  num;      /* 현재 user 사용 수 */
    sysUseInfo->num = (unsigned int)ntohl(sysUseInfo->num);
}
void SCE_SYS_STATUS_INFO_N2H(SCE_SYS_STATUS_INFO *sysStsInfo)
{
//    char            mask;
//    unsigned char   status;
//    unsigned char   preStatus;
//    unsigned char   level;
//    unsigned char   minDurat;  //  
//    unsigned char   majDurat;  //  
//    unsigned char   criDurat;  /
}

void SCE_FLOW_INFO_N2H(SCE_FLOW_INFO *flow)
{
	time_t time;
	unsigned int num;

	time = ntohl(flow->tGetTime); flow->tGetTime = time;
	num = ntohl(flow->uiFlowNum); flow->uiFlowNum = num;
}

/*########################### Host To Network ##############################*/


void SFM_SCE_H2N(SFM_SCE *sce) /*SCE Main*/
{
	int i;
	for(i=0; i<MAX_SCE_DEV_NUM; i++)
	{
		SFM_SCEDev_H2N(&sce->SCEDev[i]);
	}
}

void SFM_SCEDev_H2N(SFM_SCEDev *sceDev)
{
	int i;
	SCE_SYS_INFO_H2N(&sceDev->sysInfo);
	
	for(i=0; i<MAX_SCE_CPU_CNT; i++)
		SCE_SYS_USAGE_INFO_H2N(&sceDev->cpuInfo[i]);
	
	for(i=0; i<MAX_SCE_MEM_CNT; i++)
		SCE_SYS_USAGE_INFO_H2N(&sceDev->memInfo[i]);
		
	for(i=0; i<MAX_SCE_MEM_CNT; i++)
		SCE_SYS_USAGE_INFO_H2N(&sceDev->flowlossInfo[i]);
	
	SCE_SYS_USAGE_INFO_H2N(&sceDev->diskInfo);

	/* hjjung_20100823 */
	SCE_SYS_NUM_INFO_H2N(&sceDev->userInfo);
	
//	SCE_SYS_STATUS_INFO_H2N(sceDev->sysStatus);
//	SCE_SYS_STATUS_INFO_H2N(sceDev->pwrStatus);
//	SCE_SYS_STATUS_INFO_H2N(sceDev->fanStatus);
//	SCE_SYS_STATUS_INFO_H2N(sceDev->tempStatus);
//	SCE_SYS_STATUS_INFO_H2N(sceDev->voltStatus);
//	SCE_SYS_STATUS_INFO_H2N(sceDev->portModuleStatus);

//	for(i=0; i<MAX_SCE_LINK_CNT; i++)
//		SCE_SYS_STATUS_INFO_H2N(&sceDev->portLinkStatus[i]);
		
//	for(i=0; i<MAX_SCE_IFN_CNT; i++)
//		SCE_SYS_STATUS_INFO_H2N(&sceDev->portStatus[i]);
	
//	SCE_SYS_STATUS_INFO_H2N(sceDev->rdrStatus);
//	SCE_SYS_STATUS_INFO_H2N(sceDev->rdrConnStatus);	

}

/* hjjung_20100823 short에서 int로 변경*/
void SCE_SYS_INFO_H2N(SCE_SYS_INFO *sysInfo)
{
//	unsigned short  intro_user;
	sysInfo->intro_user = (unsigned int)htonl(sysInfo->intro_user);
//	unsigned short  active_user;
	sysInfo->active_user = (unsigned int)htonl(sysInfo->active_user);
//	unsigned char   version[32];	
} 

void SCE_SYS_USAGE_INFO_H2N(SCE_SYS_USAGE_INFO *sysUseInfo)
{
//    unsigned char   mask;       /* mml 로 설정된 alarm 발생 여부 설정 */
    unsigned short  usage;      /* 현재 memory 사용율 */
    sysUseInfo->usage = (unsigned short)htons(sysUseInfo->usage);
//    unsigned char   level;      /* 현재 memory 장애 등급 */
//    unsigned char   minLimit;   /* minor 장애 임계값 -> 이 값을 초과하여 duration 이상 지속되면 장애로     unsigned char   majLimit;   /* major 장애 임계값 */  
//    unsigned char   criLimit;   /* critical 장애 임계값 */
//    unsigned char   minDurat;   
//    unsigned char   majDurat;   
//    unsigned char   criDurat;   
//    unsigned char   minFlag;    /* minor 장애 발생되면 setting된다. 등급이 하강하는 경우 해지 메시지를     unsigned char   majFlag;    /* major 장애 발생되면 setting된다. 등급이 하강하는 경우 해지 메시지를     unsigned char   criFlag;    /* critical 장애 발생되면 setting된다. 등급이 하강하는 경우 해지 메시지
}

/* hjjung_20100823 */
void SCE_SYS_NUM_INFO_H2N(SCE_SYS_NUM_INFO *sysUseInfo)
{
    unsigned int  num; 
    sysUseInfo->num = (unsigned int)ntohl(sysUseInfo->num);
}

void SCE_SYS_STATUS_INFO_H2N(SCE_SYS_STATUS_INFO *sysStsInfo )
{
//    char            mask;
//    unsigned char   status;
//    unsigned char   preStatus;
//    unsigned char   level;
//    unsigned char   minDurat;  //  
//    unsigned char   majDurat;  //  
//    unsigned char   criDurat;  /
}

/*###########################    LEG CPS ##############################*/
#if 1
/* hjjung_20100823 */
void SFM_LEG_CPS_H2N (SFM_CALL *pCallData)
{
	int i;
	
	pCallData->cps.uiLogOnSumCps  = (unsigned int)htonl(pCallData->cps.uiLogOnSumCps);
	pCallData->cps.uiLogOutSumCps = (unsigned int)htonl(pCallData->cps.uiLogOutSumCps);
	
	for(i=0; i<2; i++){
		pCallData->legInfo[i].num = (unsigned int)htonl(pCallData->legInfo[i].num);
	}
	for(i=0; i<2; i++){ // added by dcham 20110525 for TPS
		pCallData->tpsInfo[i].num = (unsigned int)htonl(pCallData->tpsInfo[i].num);
	}
}
#endif

void SFM_TPS_CALL_H2N (SFM_CALL *pCallData) // added by dcham 20110525 for TPS
{
	int i;
	
	for(i=0; i<2; i++){ 
		pCallData->tpsInfo[i].num = (unsigned int)htonl(pCallData->tpsInfo[i].num);
	}
}

/*###########################    DEBUG PRINT  ##############################*/


void SFM_SCE_PRINT(SFM_SCE *sce) /*SCE Main*/
{
	int i;
	for(i=0; i<MAX_SCE_DEV_NUM; i++)
	{
		SFM_SCEDev_PRINT(&sce->SCEDev[i]);
	}
}

void SFM_SCEDev_PRINT(SFM_SCEDev *sceDev)
{
	int i;
	SCE_SYS_INFO_PRINT(&sceDev->sysInfo);
	
	for(i=0; i<MAX_SCE_CPU_CNT; i++)
		SCE_SYS_USAGE_INFO_PRINT(&sceDev->cpuInfo[i]);
	
	for(i=0; i<MAX_SCE_MEM_CNT; i++)
		SCE_SYS_USAGE_INFO_PRINT(&sceDev->memInfo[i]);
		
	for(i=0; i<MAX_SCE_MEM_CNT; i++)
		SCE_SYS_USAGE_INFO_PRINT(&sceDev->flowlossInfo[i]);
	
	SCE_SYS_USAGE_INFO_PRINT(&sceDev->diskInfo);
	/* hjjung */
	SCE_SYS_NUM_INFO_PRINT(&sceDev->userInfo);
	SCE_SYS_STATUS_INFO_PRINT(&sceDev->sysStatus);
	SCE_SYS_STATUS_INFO_PRINT(&sceDev->pwrStatus);
	SCE_SYS_STATUS_INFO_PRINT(&sceDev->fanStatus);
	SCE_SYS_STATUS_INFO_PRINT(&sceDev->tempStatus);
	SCE_SYS_STATUS_INFO_PRINT(&sceDev->voltStatus);
	SCE_SYS_STATUS_INFO_PRINT(&sceDev->portModuleStatus);
	
	for(i=0; i<MAX_SCE_LINK_CNT; i++)
		SCE_SYS_STATUS_INFO_PRINT(&sceDev->portLinkStatus[i]);
		
	for(i=0; i<MAX_SCE_IFN_CNT; i++)
		SCE_SYS_STATUS_INFO_PRINT(&sceDev->portStatus[i]);
	
	SCE_SYS_STATUS_INFO_PRINT(sceDev->rdrStatus);
	SCE_SYS_STATUS_INFO_PRINT(sceDev->rdrConnStatus);	

}

void SCE_SYS_INFO_PRINT(SCE_SYS_INFO *sysInfo)
{
//	unsigned short  intro_user;
	fprintf(stdout,"SCE_SYS_INFO->intro_user : %d\n", sysInfo->intro_user);
//	unsigned short  active_user;
	fprintf(stdout,"SCE_SYS_INFO->active_user: %d\n", sysInfo->active_user);
//	unsigned char   version[32];	
	fprintf(stdout,"SCE_SYS_INFO->version: %s\n", sysInfo->version);
} 

void SCE_SYS_USAGE_INFO_PRINT(SCE_SYS_USAGE_INFO *sysUseInfo)
{
//    unsigned char   mask;       /* mml 로 설정된 alarm 발생 여부 설정 */
	fprintf(stdout,"SCE_SYS_USAGE_INFO->mask: %c\n", sysUseInfo->mask);
//    unsigned short  usage;      /* 현재 memory 사용율 */
    sysUseInfo->usage = (unsigned short)ntohl(sysUseInfo->usage);
	fprintf(stdout,"SCE_SYS_USAGE_INFO->usage: %d\n", (int)sysUseInfo->usage);
//    unsigned char   level;      /* 현재 memory 장애 등급 */
	fprintf(stdout,"SCE_SYS_USAGE_INFO->level: %c\n", sysUseInfo->level);
//    unsigned char   minLimit;   /* minor 장애 임계값 -> 이 값을 초과하여 duration 이상 지속되면 장애로     unsigned char   majLimit;   /* major 장애 임계값 */  
	fprintf(stdout,"SCE_SYS_USAGE_INFO->minLimit: %c\n", sysUseInfo->minLimit);
//    unsigned char   criLimit;   /* critical 장애 임계값 */
	fprintf(stdout,"SCE_SYS_USAGE_INFO->criLimit: %c\n", sysUseInfo->criLimit);
//    unsigned char   minDurat;   
	fprintf(stdout,"SCE_SYS_USAGE_INFO->minDurat: %c\n", sysUseInfo->minDurat);
//    unsigned char   majDurat;   
	fprintf(stdout,"SCE_SYS_USAGE_INFO->majDurat: %c\n", sysUseInfo->majDurat);
//    unsigned char   criDurat;   
	fprintf(stdout,"SCE_SYS_USAGE_INFO->criDurat: %c\n", sysUseInfo->criDurat);
//    unsigned char   minFlag;    /* minor 장애 발생되면 setting된다. 등급이 하강하는 경우 해지 메시지를     unsigned char   majFlag;    /* major 장애 발생되면 setting된다. 등급이 하강하는 경우 해지 메시지를     unsigned char   criFlag;    /* critical 장애 발생되면 setting된다. 등급이 하강하는 경우 해지 메시지
	fprintf(stdout,"SCE_SYS_USAGE_INFO->minFlag: %c\n", sysUseInfo->minFlag);
}

/* hjjung */
void SCE_SYS_NUM_INFO_PRINT(SCE_SYS_NUM_INFO *sysNumInfo)
{
//    unsigned char   mask;       /* mml 로 설정된 alarm 발생 여부 설정 */
	fprintf(stdout,"SCE_SYS_NUM_INFO->mask: %c\n", sysNumInfo->mask);
//    unsigned short  usage;      /* 현재 memory 사용율 */
    sysNumInfo->num = (unsigned int)ntohl(sysNumInfo->num);
	fprintf(stdout,"SCE_SYS_NUM_INFO->num: %d\n", (int)sysNumInfo->num);
//    unsigned char   level;      /* 현재 memory 장애 등급 */
	fprintf(stdout,"SCE_SYS_NUM_INFO->level: %c\n", sysNumInfo->level);
//    unsigned char   minLimit;   /* minor 장애 임계값 -> 이 값을 초과하여 duration 이상 지속되면 장애로     unsigned char   majLimit;   /* major 장애 임계값 */  
	fprintf(stdout,"SCE_SYS_NUM_INFO->minLimit: %c\n", sysNumInfo->minLimit);
//    unsigned char   criLimit;   /* critical 장애 임계값 */
	fprintf(stdout,"SCE_SYS_NUM_INFO->criLimit: %c\n", sysNumInfo->criLimit);
//    unsigned char   minDurat;   
	fprintf(stdout,"SCE_SYS_NUM_INFO->minDurat: %c\n", sysNumInfo->minDurat);
//    unsigned char   majDurat;   
	fprintf(stdout,"SCE_SYS_NUM_INFO->majDurat: %c\n", sysNumInfo->majDurat);
//    unsigned char   criDurat;   
	fprintf(stdout,"SCE_SYS_NUM_INFO->criDurat: %c\n", sysNumInfo->criDurat);
//    unsigned char   minFlag;    /* minor 장애 발생되면 setting된다. 등급이 하강하는 경우 해지 메시지를     unsigned char   majFlag;    /* major 장애 발생되면 setting된다. 등급이 하강하는 경우 해지 메시지를     unsigned char   criFlag;    /* critical 장애 발생되면 setting된다. 등급이 하강하는 경우 해지 메시지
	fprintf(stdout,"SCE_SYS_NUM_INFO->minFlag: %c\n", sysNumInfo->minFlag);
}

void SCE_SYS_STATUS_INFO_PRINT(SCE_SYS_STATUS_INFO *sysStsInfo)
{
//    char            mask;
	fprintf(stdout,"SCE_SYS_STATUS_INFO->sysStsInfo: %c\n", sysStsInfo->mask);
//    unsigned char   status;
	fprintf(stdout,"SCE_SYS_STATUS_INFO->status: %c\n", sysStsInfo->status);
//    unsigned char   preStatus;
	fprintf(stdout,"SCE_SYS_STATUS_INFO->preStatus: %c\n", sysStsInfo->preStatus);
//    unsigned char   level;
	fprintf(stdout,"SCE_SYS_STATUS_INFO->level: %c\n", sysStsInfo->level);
//    unsigned char   minDurat;  //  
	fprintf(stdout,"SCE_SYS_STATUS_INFO->minDurat: %c\n", sysStsInfo->minDurat);
//    unsigned char   majDurat;  //  
	fprintf(stdout,"SCE_SYS_STATUS_INFO->majDurat: %c\n", sysStsInfo->majDurat);
//    unsigned char   criDurat;  /
	fprintf(stdout,"SCE_SYS_STATUS_INFO->criDurat: %c\n", sysStsInfo->criDurat);
}

