#include "samd.h"

extern  STM_LoadOMPStatMsgType    system_statistic;
extern	int		statistic_cnt;
extern  int     trcFlag, trcLogFlag;
extern  char    trcBuf[4096], trcTmp[1024];
extern  int     ixpcQID;
extern	SFM_SysCommMsgType *loc_sadb;


int HandleStatistics(GeneralQMsgType *rxGenQMsg)
{
    IxpcQMsgType    *rxIxpcMsg;

    GeneralQMsgType txGenQMsg;
    IxpcQMsgType    *txIxpcMsg;
    char			pd;
    int             txLen;
	int				i,j;

    rxIxpcMsg = (IxpcQMsgType*)rxGenQMsg->body;

    txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
    txGenQMsg.mtype = MTYPE_STATISTICS_REPORT;

    strcpy (txIxpcMsg->head.srcSysName, rxIxpcMsg->head.dstSysName);
    strcpy (txIxpcMsg->head.srcAppName, rxIxpcMsg->head.dstAppName);
    strcpy (txIxpcMsg->head.dstSysName, rxIxpcMsg->head.srcSysName);
    strcpy (txIxpcMsg->head.dstAppName, rxIxpcMsg->head.srcAppName);

	txIxpcMsg->head.msgId = MSGID_LOAD_STATISTICS_REPORT;
    txIxpcMsg->head.bodyLen = sizeof(STM_LoadOMPStatMsgType);

    txLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;

////// by helca 07.29 OMP statistic ///////    
    system_statistic.ompInfo.avg_sess = 0;
    system_statistic.ompInfo.max_sess = 0;


	system_statistic.ompInfo.comminfo.cpu_cnt = loc_sadb->cpuCount;
	for ( i = 0 ; i < system_statistic.ompInfo.comminfo.cpu_cnt; i++) {
		if ( statistic_cnt )
			system_statistic.ompInfo.comminfo.average_cpu[i] = system_statistic.ompInfo.comminfo.average_cpu[i] / statistic_cnt;
		else 
			system_statistic.ompInfo.comminfo.average_cpu[i] = system_statistic.ompInfo.comminfo.average_cpu[i];
	}

	if ( statistic_cnt )
		system_statistic.ompInfo.comminfo.avg_mem = system_statistic.ompInfo.comminfo.avg_mem / statistic_cnt;
	else 
		system_statistic.ompInfo.comminfo.avg_mem = system_statistic.ompInfo.comminfo.avg_mem;

	if ( statistic_cnt )
		system_statistic.ompInfo.avg_disk = system_statistic.ompInfo.avg_disk / statistic_cnt;
	else 
		system_statistic.ompInfo.avg_disk = system_statistic.ompInfo.avg_disk;
	
	if ( statistic_cnt )
		system_statistic.ompInfo.max_disk = system_statistic.ompInfo.max_disk / statistic_cnt;
	else 
		system_statistic.ompInfo.max_disk = system_statistic.ompInfo.max_disk;
		
	if ( statistic_cnt )
		system_statistic.ompInfo.avg_msgQ = system_statistic.ompInfo.avg_msgQ / statistic_cnt;
	else 
		system_statistic.ompInfo.avg_msgQ = system_statistic.ompInfo.avg_msgQ;
		

		
// by helca 07.29 PDA/B statistic//	
	for(i=0; i<2; i++){
		system_statistic.pdInfo[i].cpu_cnt = MAX_PD_CPU_CNT;
		for ( j = 0 ; j < system_statistic.pdInfo[i].cpu_cnt; j++) {
			if ( statistic_cnt )
				system_statistic.pdInfo[i].average_cpu[j] = system_statistic.pdInfo[i].average_cpu[j] / statistic_cnt;
			else 
				system_statistic.pdInfo[i].average_cpu[j] = system_statistic.pdInfo[i].average_cpu[j];
		}
		if ( statistic_cnt )
			system_statistic.pdInfo[i].avg_mem = system_statistic.pdInfo[i].avg_mem / statistic_cnt;
		else 
			system_statistic.pdInfo[i].avg_mem = system_statistic.pdInfo[i].avg_mem;
		if(i == 0) pd = 'A';
		else pd = 'B';
	}
	
	/* CISCO SCE load statistic, by june */
	for(i=0; i<MAX_SCE_DEV_NUM; i++){
		system_statistic.sceInfo[i].cpu_cnt = MAX_SCE_CPU_CNT;
		for ( j = 0 ; j < system_statistic.sceInfo[i].cpu_cnt; j++) {
			if ( statistic_cnt )
				system_statistic.sceInfo[i].avg_cpu[j] = system_statistic.sceInfo[i].avg_cpu[j] / statistic_cnt;
			else 
				system_statistic.sceInfo[i].avg_cpu[j] = system_statistic.sceInfo[i].avg_cpu[j];
		}
		system_statistic.sceInfo[i].mem_cnt = MAX_SCE_MEM_CNT;
		for ( j = 0 ; j < system_statistic.sceInfo[i].mem_cnt; j++) {
			if ( statistic_cnt )
				system_statistic.sceInfo[i].avg_mem[j] = system_statistic.sceInfo[i].avg_mem[j] / statistic_cnt;
			else 
				system_statistic.sceInfo[i].avg_mem[j] = system_statistic.sceInfo[i].avg_mem[j];
		}
		if ( statistic_cnt )
			system_statistic.sceInfo[i].avg_disk = system_statistic.sceInfo[i].avg_disk / statistic_cnt;
		else 
			system_statistic.sceInfo[i].avg_disk = system_statistic.sceInfo[i].avg_disk;
		if(i == 0) pd = 'A';
		else pd = 'B';

		// CISCO SCE
		if (trcFlag || trcLogFlag) {
			sprintf(trcBuf, "SCE_%c:" 
							"avr_cpu1[%d], max_cpu1[%d], avr_cpu2[%d], max_cpu2[%d],avr_cpu3[%d], max_cpu3[%d],"
							"avr_mem1[%d], max_mem1[%d], avr_mem2[%d], max_mem2[%d],avr_mem3[%d], max_mem3[%d],"
							"avr_disk[%d], max_disk[%d]\n"
					, pd
					, system_statistic.sceInfo[i].avg_cpu[0], system_statistic.sceInfo[i].max_cpu[0]
					, system_statistic.sceInfo[i].avg_cpu[1], system_statistic.sceInfo[i].max_cpu[1]
					, system_statistic.sceInfo[i].avg_cpu[2], system_statistic.sceInfo[i].max_cpu[2]
					, system_statistic.sceInfo[i].avg_mem[0], system_statistic.sceInfo[i].max_mem[0]
					, system_statistic.sceInfo[i].avg_mem[1], system_statistic.sceInfo[i].max_mem[1]
					, system_statistic.sceInfo[i].avg_mem[2], system_statistic.sceInfo[i].max_mem[2]
					, system_statistic.sceInfo[i].avg_disk, system_statistic.sceInfo[i].max_disk
				   );
			trclib_writeLog(FL, trcBuf);
		}

	}

	
	// OMP
	if (trcFlag || trcLogFlag) {
		sprintf(trcBuf, "OMP: avg_disk[%d], max_disk[%d], avg_msgQ[%d], max_msgQ[%d], avr_mem[%d], max_mem[%d]"
						", cpu_cnt[%d], avr_cpu0[%d], max_cpu0[%d], avr_cpu1[%d], max_cpu1[%d]\n"
						, system_statistic.ompInfo.avg_disk, system_statistic.ompInfo.max_disk
						, system_statistic.ompInfo.avg_msgQ, system_statistic.ompInfo.max_msgQ
						, system_statistic.ompInfo.comminfo.avg_mem, system_statistic.ompInfo.comminfo.max_mem
						, system_statistic.ompInfo.comminfo.cpu_cnt
						, system_statistic.ompInfo.comminfo.average_cpu[0], system_statistic.ompInfo.comminfo.max_cpu[0]
						, system_statistic.ompInfo.comminfo.average_cpu[1], system_statistic.ompInfo.comminfo.max_cpu[1] );
		trclib_writeLog(FL, trcBuf);
	}
	// TAP
	if (trcFlag || trcLogFlag) {
		//sprintf(trcBuf, "PD_%c: avr_mem[%d], max_mem[%d], avr_cpu[%d], max_cpu[%d]\n"
		sprintf(trcBuf, "TAP_%c: avr_mem[%d], max_mem[%d], avr_cpu[%d], max_cpu[%d]\n"
						, pd
						, system_statistic.pdInfo[i].avg_mem, system_statistic.pdInfo[i].max_mem
						, system_statistic.pdInfo[i].average_cpu[0], system_statistic.pdInfo[i].max_cpu[0]
			);
		trclib_writeLog(FL, trcBuf);
	}

    memcpy (txIxpcMsg->body, (char*)&system_statistic, sizeof(STM_LoadOMPStatMsgType));

    if ( msgsnd(ixpcQID, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0 ) {
        sprintf(trcBuf,"HandleStatistics] SEND FAIL system_statistic = %s\n", strerror(errno));
        trclib_writeLogErr (FL,trcBuf);
    } else {
        memset(&system_statistic, 0, sizeof(STM_LoadOMPStatMsgType));
		statistic_cnt = 0;
    }

	memset(&system_statistic, 0, sizeof(STM_LoadOMPStatMsgType));

    return 1;
}
	

