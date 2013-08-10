#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>

#include "netOrderL3pdChange.h"

/*########################### Network To Host ##############################*/
/*SFM_L3PD Main*/
void SFM_L3PD_N2H(SFM_L3PD *l3pd) 
{
	int i;
	for(i=0; i<MAX_PROBE_DEV_NUM; i++)
	{
		SFM_L3ProbeDev_N2H(&l3pd->l3ProbeDev[i]);
	}
}

void SFM_L3ProbeDev_N2H(SFM_L3ProbeDev *l3ProbeDev)
{
	int i;
	SFM_PDCpuInfo_N2H(&l3ProbeDev->cpuInfo);
	SFM_PDMemInfo_N2H(&l3ProbeDev->memInfo);
	//SFM_PDFanInfo_N2H(&L3ProbeDev->fanInfo);
	
//	for(i=0; i<MAX_GIGA_LAN_NUM; i++)
//		SFM_PDGigaLanInfo_N2H(&L3ProbeDev->gigaLanInfo[i]);
	
}

void SFM_PDCpuInfo_N2H(SFM_PDCpuInfo *cpuInfo)
{
//    unsigned char   mask;
//    unsigned short  usage;
		cpuInfo->usage= (unsigned short)ntohs(cpuInfo->usage);
//    unsigned char   level;
//    unsigned char   minLimit;
//    unsigned char   majLimit;
//    unsigned char   criLimit;
//    unsigned char   minDurat;
//    unsigned char   majDurat;
}

void SFM_PDMemInfo_N2H(SFM_PDMemInfo *memInfo)
{
	//unsigned char   mask;
	//unsigned short  usage;   
	memInfo->usage= (unsigned short)ntohs(memInfo->usage);
	//unsigned char   level;   
	//unsigned char   minLimit;
	//unsigned char   majLimit;
	//unsigned char   criLimit;
	//unsigned char   minDurat;
	//unsigned char   majDurat;
	//unsigned char   criDurat;
	//SFM_PdHwAlmFlag_s   memFlag;
	//SFM_PdHwAlmFlag_s_N2H(&memInfo->cpuFlag);
}

void SFM_PdHwAlmFlag_s_N2H(SFM_PdHwAlmFlag_s * hwAlmflag)
{
	//unsigned char   minFla;
	//unsigned char   majFlag;
	//unsigned char   criFlag;
}

void SFM_PDFanInfo_N2H(SFM_PDFanInfo *fanInfo)
{
   // char            mask[MAX_PD_FAN_NUM];
   // unsigned char   status[MAX_PD_FAN_NUM];
   // unsigned char   prevStatus[MAX_PD_FAN_NUM];
   // unsigned char   level[MAX_PD_FAN_NUM];
}

void SFM_PDGigaLanInfo_N2H(SFM_PDGigaLanInfo * gigaLanInfo)
{
   // char            mask;
   // unsigned char   status;
   // unsigned char   prevStatus;
   // unsigned char   level;
}

/*########################### Host To Network ##############################*/
/*SFM_L2DEV Main*/
void SFM_L2SW_H2N(SFM_L2SW *l2sw)
{
	int i;
	SFM_PDCpuInfo_H2N(&l2sw->cpuInfo);
	SFM_PDMemInfo_H2N(&l2sw->memInfo);
	
//	for(i=0; i<MAX_GIGA_LAN_NUM; i++)
//		SFM_PDGigaLanInfo_H2N(&l2sw->portInfo[i]);
}

void SFM_L2DEV_H2N(SFM_L2Dev *l2dev) 
{
	int i;
	for(i=0; i<MAX_L2_DEV_NUM; i++)
	{
		SFM_L2SW_H2N(&l2dev->l2Info[i]);
	}
}

////////////////////////////////////////////////////////////////////////////
/*SFM_L3PD Main*/
void SFM_L3PD_H2N(SFM_L3PD *l3pd) 
{
	int i;
	for(i=0; i<MAX_PROBE_DEV_NUM; i++)
	{
		SFM_L3ProbeDev_H2N(&l3pd->l3ProbeDev[i]);
	}
}

void SFM_L3ProbeDev_H2N(SFM_L3ProbeDev *L3ProbeDev)
{
	int i;
	SFM_PDCpuInfo_H2N(&L3ProbeDev->cpuInfo);
	SFM_PDMemInfo_H2N(&L3ProbeDev->memInfo);
	//SFM_PDFanInfo_H2N(&L3ProbeDev->fanInfo);
	
//	for(i=0; i<MAX_GIGA_LAN_NUM; i++)
//		SFM_PDGigaLanInfo_H2N(&L3ProbeDev->gigaLanInfo[i]);
	
}

void SFM_PDCpuInfo_H2N(SFM_PDCpuInfo *cpuInfo)
{
//    unsigned char   mask;
//    unsigned short  usage;
		cpuInfo->usage= (unsigned short)htons(cpuInfo->usage);
//    unsigned char   level;
//    unsigned char   minLimit;
//    unsigned char   majLimit;
//    unsigned char   criLimit;
//    unsigned char   minDurat;
//    unsigned char   majDurat;
}

void SFM_PDMemInfo_H2N(SFM_PDMemInfo *memInfo)
{
	//unsigned char   mask;
	//unsigned short  usage;   
	memInfo->usage= (unsigned short)htons(memInfo->usage);
	//unsigned char   level;   
	//unsigned char   minLimit;
	//unsigned char   majLimit;
	//unsigned char   criLimit;
	//unsigned char   minDurat;
	//unsigned char   majDurat;
	//unsigned char   criDurat;
	//SFM_PdHwAlmFlag_s   memFlag;
	//SFM_PdHwAlmFlag_s_H2N(&memInfo->cpuFlag);
}

void SFM_PdHwAlmFlag_s_H2N(SFM_PdHwAlmFlag_s * hwAlmflag)
{
	//unsigned char   minFla;
	//unsigned char   majFlag;
	//unsigned char   criFlag;
}

void SFM_PDFanInfo_H2N(SFM_PDFanInfo *fanInfo)
{
   // char            mask[MAX_PD_FAN_NUM];
   // unsigned char   status[MAX_PD_FAN_NUM];
   // unsigned char   prevStatus[MAX_PD_FAN_NUM];
   // unsigned char   level[MAX_PD_FAN_NUM];
}

void SFM_PDGigaLanInfo_H2N(SFM_PDGigaLanInfo * gigaLanInfo)
{
   // char            mask;
   // unsigned char   status;
   // unsigned char   prevStatus;
   // unsigned char   level;
}


