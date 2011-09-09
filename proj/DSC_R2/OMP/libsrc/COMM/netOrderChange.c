/*

	- Date : 2009-04-23
	- Author : sjjeon
	- Descript : SFM 구조체 Network Order 변경
 */


#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>

#include "netOrderChange.h"

/*########################### Network To  Host ##############################*/

void SFM_SysCommMsgType_N2H(SFM_SysCommMsgType *sysMsg)
{
	int i;
//	printf("mem_usage : %d\n", (int)sysMsg->mem_usage);
	sysMsg->mem_usage = (unsigned short)ntohs(sysMsg->mem_usage);
//	printf("ntohs(), mem_usage : %d\n", (int)sysMsg->mem_usage);

	for(i=0;i<SFM_MAX_CPU_CNT;i++){
//		printf("cpu_usage: %d\n", (int)sysMsg->cpu_usage[i]);
		sysMsg->cpu_usage[i] = (unsigned short)ntohs(sysMsg->cpu_usage[i]);
//		printf("cpu_usage(ntohl): %d\n", (int)sysMsg->cpu_usage[i]);
	}

	for(i=0;i<SFM_MAX_SESSION_CNT;i++){
//		printf("sess_load %d\n", (int)sysMsg->sess_load[i]);
		sysMsg->sess_load[i] = (unsigned short)ntohs(sysMsg->sess_load[i]);
//		printf("sess_load (ntohs)%d\n", (int)sysMsg->sess_load[i]);
//		printf("sess_load (htons)%d\n", htons(sysMsg->sess_load[i]));
	}
//    unsigned short      total_disk_usage;
	sysMsg->total_disk_usage = (unsigned short)ntohs(sysMsg->total_disk_usage);
	
	for(i=0;i<SFM_MAX_PROC_CNT;i++)
		SFM_SysCommProcSts_N2H(&sysMsg->loc_process_sts[i]);
	
	for(i=0;i<SFM_MAX_DISK_CNT;i++)
		SFM_SysCommDiskSts_N2H(&sysMsg->loc_disk_sts[i]);
	
	for(i=0;i<SFM_MAX_LAN_CNT;i++)
		SFM_SysCommLanSts_N2H(&sysMsg->loc_lan_sts[i]);
	
	for(i=0;i<SFM_MAX_RMT_LAN_CNT;i++)
		SFM_SysCommLanSts_N2H(&sysMsg->rmt_lan_sts[i]);	
	
	for(i=0;i<SFM_MAX_QUE_CNT;i++)
		SFM_SysCommQueSts_N2H(&sysMsg->loc_que_sts[i]);
			
//	for(i=0;i<SFM_MAX_DEV_CNT;i++)
//		SFM_SysCommLinkSts_N2H(&sysMsg->loc_link_sts[i]);
			
	SFM_SysDuplicationSts_N2H(&sysMsg->loc_system_dup);
	
	for(i=0;i<SFM_MAX_SUCC_RATE_CNT;i++)
		SFM_SysSuccessRate_N2H(&sysMsg->succ_rate[i]);
		
//	for(i=0;i<MAX_BOND_NUM;i++)
//		SFM_SysIFBond_N2H(&sysMsg->IF_bond[i]);
		
//    SFM_SysSts      sysSts;
//    SFM_Duia        duia;

}

/*SFM_SysCommProcSts struct ntoh 변환 */
void SFM_SysCommProcSts_N2H(SFM_SysCommProcSts *sysProc)
{
	sysProc->pid = (unsigned int)ntohl(sysProc->pid);
	sysProc->uptime = (unsigned long)ntohl(sysProc->uptime);
}

/*SFM_SysCommDiskSts struct ntoh 변환 */
void SFM_SysCommDiskSts_N2H(SFM_SysCommDiskSts *sysDisk)
{
	sysDisk->disk_usage = (unsigned short)ntohs(sysDisk->disk_usage);
}

/*SFM_SysCommLanSts struct ntoh 변환 */
void SFM_SysCommLanSts_N2H(SFM_SysCommLanSts *sysLan)
{
	sysLan->target_IPaddress = (unsigned int)ntohl(sysLan->target_IPaddress);	
//	printf("N2H 변환 : %d -> %s\n", sysLan->target_IPaddress, inet_ntoa(sysLan->target_IPaddress));
}

/*SFM_SysCommQueSts struct ntoh 변환 */
void SFM_SysCommQueSts_N2H(SFM_SysCommQueSts *sysQue)
{
	sysQue->qID = (int)ntohl(sysQue->qID);		
	sysQue->qKEY = (int)ntohl(sysQue->qKEY);	
	sysQue->qNUM = (unsigned int)ntohl(sysQue->qNUM);
	sysQue->cBYTES = (unsigned int)ntohl(sysQue->cBYTES);
	sysQue->qBYTES = (unsigned int)ntohl(sysQue->qBYTES);
}

/*SFM_SysCommLinkSts struct ntoh 변환 */
void SFM_SysCommLinkSts_N2H(SFM_SysCommLinkSts *sysLink)
{
//    char    linkName[SFM_MAX_LAN_NAME_LEN];
//    unsigned char   status;
}

/*SFM_SysDuplicationSts struct ntoh 변환 */
void SFM_SysDuplicationSts_N2H(SFM_SysDuplicationSts *sysDup)
{
	int i;
    sysDup->llAcctSessID = (long long)ntohl(sysDup->llAcctSessID);
    sysDup->uiKey = (unsigned int)ntohl(sysDup->uiKey);
    sysDup->uiUdrSequence = (long long)ntohl(sysDup->uiUdrSequence);

    for(i=0;i<3;i++)
    	SFM_SysDBConnSts_N2H(&sysDup->RmtDbSts[i]);

	//printf("sysDup->heartbeatAlarm : %d\n", sysDup->heartbeatAlarm);
    sysDup->heartbeatAlarm = (unsigned int)ntohl(sysDup->heartbeatAlarm);
	//printf("sysDup->heartbeatAlarm (ntohl): %d\n", sysDup->heartbeatAlarm);
	//printf("sysDup->heartbeatAlarm (htonl): %d\n", htonl(sysDup->heartbeatAlarm));
    sysDup->bsdmTimeoutAlarm = (unsigned int)ntohl(sysDup->bsdmTimeoutAlarm);
	sysDup->OosAlarm = (unsigned int)ntohl(sysDup->OosAlarm);
}

/*SFM_SysDBConnSts struct ntoh 변환 */
void SFM_SysDBConnSts_N2H(SFM_SysDBConnSts *sysDup)
{
	sysDup->iStatus = (unsigned int)ntohl(sysDup->iStatus);
	sysDup->uiTxnLogId = (unsigned int)ntohl(sysDup->uiTxnLogId);
}


/*SFM_SysSuccessRate struct ntoh 변환 */
void SFM_SysSuccessRate_N2H(SFM_SysSuccessRate *sysRate)
{
	sysRate->count = (unsigned int)ntohl(sysRate->count);
	sysRate->rate = (unsigned int)ntohl(sysRate->rate);
}

/*SFM_SysIFBond struct ntoh 변환 */
void SFM_SysIFBond_N2H(SFM_SysIFBond *sysBond)
{
	//unsigned char   bondName[COMM_MAX_NAME_LEN];
    //unsigned char   status;
}

/*SFM_SysRsrcLoad struct ntoh 변환 */
void SFM_SysRsrcLoad_N2H(SFM_SysRsrcLoad *sysRsrc)
{
	//unsigned int    rsrcload[SFM_MAX_RSRC_LOAD_CNT];
}

/*SFM_SysSts struct ntoh 변환 */
void SFM_SysSts_N2H(SFM_SysSts *sysSts)
{
	int i;
//	for(i=0;i<SFM_HW_MAX_DISK_NUM;i++)
//		SFM_diskSts_N2H(&sysSts->diskSts[i]);

//	for(i=0;i<SFM_HW_MAX_CPU_NUM;i++)
//		SFM_cpuSts_N2H(&sysSts->cpuSts[i]);
		
//	for(i=0;i<SFM_HW_MAX_LINK_NUM;i++)
//		SFM_linkSts_N2H(&sysSts->linkSts[i]);

//	for(i=0;i<SFM_HW_MAX_PWR_NUM;i++)
//		SFM_pwrSts_N2H(&sysSts->pwrSts[i]);

//	for(i=0;i<SFM_HW_MAX_PWR_NUM;i++)
//		SFM_fanSts_N2H(&sysSts->fanSts[i]);
}


/*SFM_diskSts struct ntoh 변환 */
void SFM_diskSts_N2H(SFM_diskSts *diskSts)
{
    //unsigned char   StsName[COMM_MAX_NAME_LEN];
    //unsigned char   status;
}

/*SFM_cpuSts struct ntoh 변환 */
void SFM_cpuSts_N2H(SFM_cpuSts *cpuSts)
{
   //unsigned char   StsName[COMM_MAX_NAME_LEN];
   //unsigned char   status;
}

/*SFM_linkSts struct ntoh 변환 */
void SFM_linkSts_N2H(SFM_linkSts *linkSts)
{
   //unsigned char   StsName[COMM_MAX_NAME_LEN];
   //unsigned char   status;
}

/*SFM_pwrSts struct ntoh 변환 */
void SFM_pwrSts_N2H(SFM_pwrSts *pwrSts)
{
   // unsigned char   StsName[COMM_MAX_NAME_LEN];
   // unsigned char   status;
}

/*SFM_fanSts struct ntoh 변환 */
void SFM_fanSts_N2H(SFM_fanSts *fanSts)
{
    //unsigned char   StsName[COMM_MAX_NAME_LEN];
    //unsigned char   status;
}


/*SFM_Duia struct ntoh 변환 */
void SFM_Duia_N2H(SFM_Duia *sysDuia)
{
	sysDuia->svcType = (unsigned int)ntohl(sysDuia->svcType);
}


/*########################### Host To Network  ##############################*/


/*struct ntoh 변환 */
void SFM_SysCommMsgType_H2N(SFM_SysCommMsgType *sysMsg)
{
	int i;
//	printf("mem_usage : %d\n", (int)sysMsg->mem_usage);
	sysMsg->mem_usage = (unsigned short)htons(sysMsg->mem_usage);
//	printf("ntohs(), mem_usage : %d\n", (int)sysMsg->mem_usage);

	for(i=0;i<SFM_MAX_CPU_CNT;i++){
//		printf("cpu_usage: %d\n", (int)sysMsg->cpu_usage[i]);
		sysMsg->cpu_usage[i] = (unsigned short)htons(sysMsg->cpu_usage[i]);
//		printf("cpu_usage(ntohl): %d\n", (int)sysMsg->cpu_usage[i]);
	}

	for(i=0;i<SFM_MAX_SESSION_CNT;i++){
//		printf("sess_load %d\n", (int)sysMsg->sess_load[i]);
		sysMsg->sess_load[i] = (unsigned short)htons(sysMsg->sess_load[i]);
//		printf("sess_load (ntohs)%d\n", (int)sysMsg->sess_load[i]);
//		printf("sess_load (htons)%d\n", htons(sysMsg->sess_load[i]));
	}
//    unsigned short      total_disk_usage;
	sysMsg->total_disk_usage = (unsigned short)htons(sysMsg->total_disk_usage);
	
	for(i=0;i<SFM_MAX_PROC_CNT;i++)
		SFM_SysCommProcSts_H2N(&sysMsg->loc_process_sts[i]);
	
	for(i=0;i<SFM_MAX_DISK_CNT;i++)
		SFM_SysCommDiskSts_H2N(&sysMsg->loc_disk_sts[i]);
	
	for(i=0;i<SFM_MAX_LAN_CNT;i++)
		SFM_SysCommLanSts_H2N(&sysMsg->loc_lan_sts[i]);
	
	for(i=0;i<SFM_MAX_RMT_LAN_CNT;i++)
		SFM_SysCommLanSts_H2N(&sysMsg->rmt_lan_sts[i]);	
	
	for(i=0;i<SFM_MAX_QUE_CNT;i++)
		SFM_SysCommQueSts_H2N(&sysMsg->loc_que_sts[i]);
			
//	for(i=0;i<SFM_MAX_DEV_CNT;i++)
//		SFM_SysCommLinkSts_H2N(&sysMsg->loc_link_sts[i]);
			
	SFM_SysDuplicationSts_H2N(&sysMsg->loc_system_dup);
	
	for(i=0;i<SFM_MAX_SUCC_RATE_CNT;i++)
		SFM_SysSuccessRate_H2N(&sysMsg->succ_rate[i]);
		
//	for(i=0;i<MAX_BOND_NUM;i++)
//		SFM_SysIFBond_H2N(&sysMsg->IF_bond[i]);
		
//    SFM_SysSts      sysSts;
//    SFM_Duia        duia;

}

/*SFM_SysCommProcSts struct ntoh 변환 */
void SFM_SysCommProcSts_H2N(SFM_SysCommProcSts *sysProc)
{
	sysProc->pid = (unsigned int)htonl(sysProc->pid);
	sysProc->uptime = (unsigned long)htonl(sysProc->uptime);
}

/*SFM_SysCommDiskSts struct ntoh 변환 */
void SFM_SysCommDiskSts_H2N(SFM_SysCommDiskSts *sysDisk)
{
	sysDisk->disk_usage = (unsigned short)htons(sysDisk->disk_usage);
}

/*SFM_SysCommLanSts struct ntoh 변환 */
void SFM_SysCommLanSts_H2N(SFM_SysCommLanSts *sysLan)
{
	sysLan->target_IPaddress = (unsigned int)htonl(sysLan->target_IPaddress);	
//	printf("H2N 변환 : %d -> %s\n", sysLan->target_IPaddress, inet_ntoa(sysLan->target_IPaddress));
}

/*SFM_SysCommQueSts struct ntoh 변환 */
void SFM_SysCommQueSts_H2N(SFM_SysCommQueSts *sysQue)
{
	sysQue->qID = (int)htonl(sysQue->qID);		
	sysQue->qKEY = (int)htonl(sysQue->qKEY);	
	sysQue->qNUM = (unsigned int)htonl(sysQue->qNUM);
	sysQue->cBYTES = (unsigned int)htonl(sysQue->cBYTES);
	sysQue->qBYTES = (unsigned int)htonl(sysQue->qBYTES);
}

/*SFM_SysCommLinkSts struct ntoh 변환 */
void SFM_SysCommLinkSts_H2N(SFM_SysCommLinkSts *sysLink)
{
//    char    linkName[SFM_MAX_LAN_NAME_LEN];
//    unsigned char   status;
}

/*SFM_SysDuplicationSts struct ntoh 변환 */
void SFM_SysDuplicationSts_H2N(SFM_SysDuplicationSts *sysDup)
{
	int i;
    sysDup->llAcctSessID = (long long)htonl(sysDup->llAcctSessID);
    sysDup->uiKey = (unsigned int)htonl(sysDup->uiKey);
    sysDup->uiUdrSequence = (unsigned int)htonl(sysDup->uiUdrSequence);

    for(i=0;i<3;i++)
    	SFM_SysDBConnSts_H2N(&sysDup->RmtDbSts[i]);

	//printf("sysDup->heartbeatAlarm : %d\n", sysDup->heartbeatAlarm);
    sysDup->heartbeatAlarm = (unsigned int)htonl(sysDup->heartbeatAlarm);
	//printf("sysDup->heartbeatAlarm (htonl): %d\n", sysDup->heartbeatAlarm);
	//printf("sysDup->heartbeatAlarm (htonl): %d\n", htonl(sysDup->heartbeatAlarm));
    sysDup->bsdmTimeoutAlarm = (unsigned int)htonl(sysDup->bsdmTimeoutAlarm);
	sysDup->OosAlarm = (unsigned int)htonl(sysDup->OosAlarm);
}

/*SFM_SysDBConnSts struct ntoh 변환 */
void SFM_SysDBConnSts_H2N(SFM_SysDBConnSts *sysDup)
{
	sysDup->iStatus = (unsigned int)htonl(sysDup->iStatus);
	sysDup->uiTxnLogId = (unsigned int)htonl(sysDup->uiTxnLogId);
}


/*SFM_SysSuccessRate struct ntoh 변환 */
void SFM_SysSuccessRate_H2N(SFM_SysSuccessRate *sysRate)
{
	sysRate->count = (unsigned int)htonl(sysRate->count);
	sysRate->rate = (unsigned int)htonl(sysRate->rate);
}

/*SFM_SysIFBond struct ntoh 변환 */
void SFM_SysIFBond_H2N(SFM_SysIFBond *sysBond)
{
	//unsigned char   bondName[COMM_MAX_NAME_LEN];
    //unsigned char   status;
}

/*SFM_SysRsrcLoad struct ntoh 변환 */
void SFM_SysRsrcLoad_H2N(SFM_SysRsrcLoad *sysRsrc)
{
	//unsigned int    rsrcload[SFM_MAX_RSRC_LOAD_CNT];
	int i;
	for(i=0;i<SFM_MAX_RSRC_LOAD_CNT;i++)
		sysRsrc->rsrcload[i] = (unsigned int) htonl(sysRsrc->rsrcload[i]);
	
}

/*SFM_SysSts struct ntoh 변환 */
void SFM_SysSts_H2N(SFM_SysSts *sysSts)
{
	int i;
//	for(i=0;i<SFM_HW_MAX_DISK_NUM;i++)
//		SFM_diskSts_H2N(&sysSts->diskSts[i]);

//	for(i=0;i<SFM_HW_MAX_CPU_NUM;i++)
//		SFM_cpuSts_H2N(&sysSts->cpuSts[i]);
		
//	for(i=0;i<SFM_HW_MAX_LINK_NUM;i++)
//		SFM_linkSts_H2N(&sysSts->linkSts[i]);

//	for(i=0;i<SFM_HW_MAX_PWR_NUM;i++)
//		SFM_pwrSts_H2N(&sysSts->pwrSts[i]);

//	for(i=0;i<SFM_HW_MAX_PWR_NUM;i++)
//		SFM_fanSts_H2N(&sysSts->fanSts[i]);
}


/*SFM_diskSts struct ntoh 변환 */
void SFM_diskSts_H2N(SFM_diskSts *diskSts)
{
    //unsigned char   StsName[COMM_MAX_NAME_LEN];
    //unsigned char   status;
}

/*SFM_cpuSts struct ntoh 변환 */
void SFM_cpuSts_H2N(SFM_cpuSts *cpuSts)
{
   //unsigned char   StsName[COMM_MAX_NAME_LEN];
   //unsigned char   status;
}

/*SFM_linkSts struct ntoh 변환 */
void SFM_linkSts_H2N(SFM_linkSts *linkSts)
{
   //unsigned char   StsName[COMM_MAX_NAME_LEN];
   //unsigned char   status;
}

/*SFM_pwrSts struct ntoh 변환 */
void SFM_pwrSts_H2N(SFM_pwrSts *pwrSts)
{
   // unsigned char   StsName[COMM_MAX_NAME_LEN];
   // unsigned char   status;
}

/*SFM_fanSts struct ntoh 변환 */
void SFM_fanSts_H2N(SFM_fanSts *fanSts)
{
    //unsigned char   StsName[COMM_MAX_NAME_LEN];
    //unsigned char   status;
}

/*SFM_Duia struct ntoh 변환 */
void SFM_Duia_H2N(SFM_Duia *sysDuia)
{
	sysDuia->svcType = (unsigned int)htonl(sysDuia->svcType);
}

#ifdef __DEBUG__

/*########################### DEBUG PRINT  ##############################*/


/*struct ntoh 변환 */
void SFM_SysCommMsgType_PRINT(SFM_SysCommMsgType *sysMsg)
{
	int i;
	printf("%s:%d] mem_usage: %d\n",__FILE__, __LINE__,(int)sysMsg->mem_usage);

	for(i=0;i<SFM_MAX_CPU_CNT;i++){
		//sysMsg->cpu_usage[i] = (unsigned short)htons(sysMsg->cpu_usage[i]);
		printf("%s:%d] cpu_usage: %d\n",__FILE__, __LINE__,(int)sysMsg->cpu_usage[i]);
	}

	for(i=0;i<SFM_MAX_SESSION_CNT;i++){
		//sysMsg->sess_load[i] = (unsigned short)htons(sysMsg->sess_load[i]);
		printf("%s:%d] sess_load: %d\n",__FILE__, __LINE__,(int)sysMsg->sess_load[i]);
	}
//    unsigned short      total_disk_usage;
	//sysMsg->total_disk_usage = (unsigned short)htons(sysMsg->total_disk_usage);
	printf("%s:%d] total_disk_usage: %d\n",__FILE__, __LINE__,(int)sysMsg->total_disk_usage);
	
	for(i=0;i<SFM_MAX_PROC_CNT;i++)
		SFM_SysCommProcSts_PRINT(&sysMsg->loc_process_sts[i]);
	
	for(i=0;i<SFM_MAX_DISK_CNT;i++)
		SFM_SysCommDiskSts_PRINT(&sysMsg->loc_disk_sts[i]);
	
	for(i=0;i<SFM_MAX_LAN_CNT;i++)
		SFM_SysCommLanSts_PRINT(&sysMsg->loc_lan_sts[i]);
	
	for(i=0;i<SFM_MAX_RMT_LAN_CNT;i++)
		SFM_SysCommLanSts_PRINT(&sysMsg->rmt_lan_sts[i]);	
	
	for(i=0;i<SFM_MAX_QUE_CNT;i++)
		SFM_SysCommQueSts_PRINT(&sysMsg->loc_que_sts[i]);
			
//	for(i=0;i<SFM_MAX_DEV_CNT;i++)
//		SFM_SysCommLinkSts_PRINT(&sysMsg->loc_link_sts[i]);
			
	SFM_SysDuplicationSts_PRINT(&sysMsg->loc_system_dup);
	
	for(i=0;i<SFM_MAX_SUCC_RATE_CNT;i++)
		SFM_SysSuccessRate_PRINT(&sysMsg->succ_rate[i]);
		
//	for(i=0;i<MAX_BOND_NUM;i++)
//		SFM_SysIFBond_PRINT(&sysMsg->IF_bond[i]);
		
//    SFM_SysSts      sysSts;
//    SFM_Duia        duia;

}

/*SFM_SysCommProcSts struct ntoh 변환 */
void SFM_SysCommProcSts_PRINT(SFM_SysCommProcSts *sysProc)
{
	//sysProc->pid = (unsigned short)htons(sysProc->pid);
	printf("%s:%d] pid: %d\n",__FILE__, __LINE__,(int)sysProc->pid);
	//sysProc->uptime = (unsigned long)htons(sysProc->uptime);
	printf("%s:%d] uptime: %ld\n",__FILE__, __LINE__,(int)sysProc->uptime);
}

/*SFM_SysCommDiskSts struct ntoh 변환 */
void SFM_SysCommDiskSts_PRINT(SFM_SysCommDiskSts *sysDisk)
{
	//sysDisk->disk_usage = (unsigned short)htons(sysDisk->disk_usage);
	printf("%s:%d] disk_usage: %d\n",__FILE__, __LINE__,(int)sysDisk->disk_usage);
}

/*SFM_SysCommLanSts struct ntoh 변환 */
void SFM_SysCommLanSts_PRINT(SFM_SysCommLanSts *sysLan)
{
	//sysLan->target_IPaddress = (unsigned int)htonl(sysLan->target_IPaddress);	
	printf("%s:%d] target_IPaddress: %d\n",__FILE__, __LINE__,(int)sysLan->target_IPaddress);
}

/*SFM_SysCommQueSts struct ntoh 변환 */
void SFM_SysCommQueSts_PRINT(SFM_SysCommQueSts *sysQue)
{
	//sysQue->qID = (int)htonl(sysQue->qID);		
	printf("%s:%d] qID: %d\n",__FILE__, __LINE__,(int)sysQue->qID);
	//sysQue->qKEY = (int)htonl(sysQue->qKEY);	
	printf("%s:%d] qKEY: %d\n",__FILE__, __LINE__,(int)sysQue->qKEY);
	//sysQue->qNUM = (unsigned int)htonl(sysQue->qNUM);
	printf("%s:%d] qNUM: %d\n",__FILE__, __LINE__,(int)sysQue->qNUM);
	//sysQue->cBYTES = (unsigned int)htonl(sysQue->cBYTES);
	printf("%s:%d] cBYTES: %d\n",__FILE__, __LINE__,(int)sysQue->cBYTES);
	//sysQue->qBYTES = (unsigned int)htonl(sysQue->qBYTES);
	printf("%s:%d] qBYTES: %d\n",__FILE__, __LINE__,(int)sysQue->qBYTES);
}

/*SFM_SysCommLinkSts struct ntoh 변환 */
void SFM_SysCommLinkSts_PRINT(SFM_SysCommLinkSts *sysLink)
{
//    char    linkName[SFM_MAX_LAN_NAME_LEN];
//    unsigned char   status;
}

/*SFM_SysDuplicationSts struct ntoh 변환 */
void SFM_SysDuplicationSts_PRINT(SFM_SysDuplicationSts *sysDup)
{
	int i;
    //sysDup->llAcctSessID = (long long)htonl(sysDup->llAcctSessID);
	printf("%s:%d] llAcctSessID: %d\n",__FILE__, __LINE__,(int)sysDup->llAcctSessID);
    //sysDup->uiKey = (unsigned int)htonl(sysDup->uiKey);
	printf("%s:%d] uiKey: %d\n",__FILE__, __LINE__,(int)sysDup->uiKey);
    //sysDup->uiUdrSequence = (long long)htonl(sysDup->uiUdrSequence);
	printf("%s:%d] uiUdrSequence: %d\n",__FILE__, __LINE__,(int)sysDup->uiUdrSequence);

    for(i=0;i<3;i++)
    	SFM_SysDBConnSts_PRINT(&sysDup->RmtDbSts[i]);

    //sysDup->heartbeatAlarm = (unsigned int)htonl(sysDup->heartbeatAlarm);
	printf("%s:%d] heartbeatAlarm: %d\n",__FILE__, __LINE__,(int)sysDup->heartbeatAlarm);
    //sysDup->bsdmTimeoutAlarm = (unsigned int)htonl(sysDup->bsdmTimeoutAlarm);
	printf("%s:%d] bsdmTimeoutAlarm: %d\n",__FILE__, __LINE__,(int)sysDup->bsdmTimeoutAlarm);
	//sysDup->OosAlarm = (unsigned int)htonl(sysDup->OosAlarm);
	printf("%s:%d] OosAlarm: %d\n",__FILE__, __LINE__,(int)sysDup->OosAlarm);
}

/*SFM_SysDBConnSts struct ntoh 변환 */
void SFM_SysDBConnSts_PRINT(SFM_SysDBConnSts *sysDup)
{
	//sysDup->iStatus = (unsigned int)htonl(sysDup->iStatus);
	printf("%s:%d] iStatus: %d\n",__FILE__, __LINE__,(int)sysDup->iStatus);
	//sysDup->uiTxnLogId = (unsigned int)htonl(sysDup->uiTxnLogId);
	printf("%s:%d] uiTxnLogId: %d\n",__FILE__, __LINE__,(int)sysDup->uiTxnLogId);
}


/*SFM_SysSuccessRate struct ntoh 변환 */
void SFM_SysSuccessRate_PRINT(SFM_SysSuccessRate *sysRate)
{
	//sysRate->count = (unsigned int)htonl(sysRate->count);
	printf("%s:%d] count: %d\n",__FILE__, __LINE__,(int)sysDup->count);
	//sysRate->rate = (unsigned int)htonl(sysRate->rate);
	printf("%s:%d] rate: %d\n",__FILE__, __LINE__,(int)sysDup->rate);
}

/*SFM_SysIFBond struct ntoh 변환 */
void SFM_SysIFBond_PRINT(SFM_SysIFBond *sysBond)
{
	//unsigned char   bondName[COMM_MAX_NAME_LEN];
    //unsigned char   status;
}

/*SFM_SysRsrcLoad struct ntoh 변환 */
void SFM_SysRsrcLoad_PRINT(SFM_SysRsrcLoad *sysRsrc)
{
	//unsigned int    rsrcload[SFM_MAX_RSRC_LOAD_CNT];
}

/*SFM_SysSts struct ntoh 변환 */
void SFM_SysSts_PRINT(SFM_SysSts *sysSts)
{
	int i;
//	for(i=0;i<SFM_HW_MAX_DISK_NUM;i++)
//		SFM_diskSts_PRINT(&sysSts->diskSts[i]);

//	for(i=0;i<SFM_HW_MAX_CPU_NUM;i++)
//		SFM_cpuSts_PRINT(&sysSts->cpuSts[i]);
		
//	for(i=0;i<SFM_HW_MAX_LINK_NUM;i++)
//		SFM_linkSts_PRINT(&sysSts->linkSts[i]);

//	for(i=0;i<SFM_HW_MAX_PWR_NUM;i++)
//		SFM_pwrSts_PRINT(&sysSts->pwrSts[i]);

//	for(i=0;i<SFM_HW_MAX_PWR_NUM;i++)
//		SFM_fanSts_PRINT(&sysSts->fanSts[i]);
}


/*SFM_diskSts struct ntoh 변환 */
void SFM_diskSts_PRINT(SFM_diskSts *diskSts)
{
    //unsigned char   StsName[COMM_MAX_NAME_LEN];
    //unsigned char   status;
}

/*SFM_cpuSts struct ntoh 변환 */
void SFM_cpuSts_PRINT(SFM_cpuSts *cpuSts)
{
   //unsigned char   StsName[COMM_MAX_NAME_LEN];
   //unsigned char   status;
}

/*SFM_linkSts struct ntoh 변환 */
void SFM_linkSts_PRINT(SFM_linkSts *linkSts)
{
   //unsigned char   StsName[COMM_MAX_NAME_LEN];
   //unsigned char   status;
}

/*SFM_pwrSts struct ntoh 변환 */
void SFM_pwrSts_PRINT(SFM_pwrSts *pwrSts)
{
   // unsigned char   StsName[COMM_MAX_NAME_LEN];
   // unsigned char   status;
}

/*SFM_fanSts struct ntoh 변환 */
void SFM_fanSts_PRINT(SFM_fanSts *fanSts)
{
    //unsigned char   StsName[COMM_MAX_NAME_LEN];
    //unsigned char   status;
}

/*SFM_Duia struct ntoh 변환 */
void SFM_Duia_PRINT(SFM_Duia *sysDuia)
{
//	sysDuia->svcType = (unsigned int)htonl(sysDuia->svcType);
	printf("%s:%d] svcType: %d\n",__FILE__, __LINE__,(int)sysDup->svcType);
}


#endif /*End of __DEBUG__*/
