#include "fimd_proto.h"

extern char		trcBuf[4096], trcTmp[1024];
extern SFM_sfdb		*sfdb;
extern SFM_L3PD		*l3pd;
extern SFM_L2Dev    *g_pstL2Dev;
extern SFM_SCE      *g_pstSCEInfo;
/* hjjung */
//extern SFM_LEG      *g_pstLEGInfo;
extern SFM_CALL     *g_pstCALLInfo; // added by dcham 20110525
extern int	trcFlag, trcLogFlag;


//------------------------------------------------------------------------------
// 장애 변경 및 해지 시 호출되어 해당 시스템에 대한 전체 장애 등급과 각 등급별
//  장애 갯수를 update한다.
// - 각 정보를 모두 확인하여 가장 높은 장애 등급을 최종 등급으로 설정한다.
// - 해당 시스템이 속한 group의 장애 등급 및 갯수를 함께 update한다.
// - sfdb 정보를 file에 backup한다.
//------------------------------------------------------------------------------
void fimd_updateSysAlmInfo (int sysIndex)
{
	unsigned char	minCnt=0, majCnt=0, criCnt=0;
	unsigned char	sysAlmLevel=SFM_ALM_NORMAL, groupAlmLevel=SFM_ALM_NORMAL;
	int	i,groupIndex;


	//
	// system common informations
	//

	// CPU
	//
	for (i=0; i<sfdb->sys[sysIndex].commInfo.cpuCnt; i++) {
		if (sfdb->sys[sysIndex].commInfo.cpuInfo.mask[i] == SFM_ALM_MASKED)
			continue;
		// CPU, Memory, Disk는 등급 상승,하강 기능이 있고, 여러등급의 장애가 함께 발생되어
		//  있을 수 있으므로 각각에 대해 count 해야 한다.
		if (sfdb->hwAlmFlag[sysIndex].cpu[i].minFlag) minCnt++;
        	if (sfdb->hwAlmFlag[sysIndex].cpu[i].majFlag) majCnt++;
        	if (sfdb->hwAlmFlag[sysIndex].cpu[i].criFlag) criCnt++;
		if (sfdb->sys[sysIndex].commInfo.cpuInfo.level[i] > sysAlmLevel)
			sysAlmLevel = sfdb->sys[sysIndex].commInfo.cpuInfo.level[i];
	}
	// Memory
	//
	if (sfdb->sys[sysIndex].commInfo.memInfo.mask != SFM_ALM_MASKED) {
		// CPU, Memory, Disk는 등급 상승,하강 기능이 있고, 여러등급의 장애가 함께 발생되어
		//  있을 수 있으므로 각각에 대해 count 해야 한다.
		if (sfdb->hwAlmFlag[sysIndex].mem.minFlag) minCnt++;
        	if (sfdb->hwAlmFlag[sysIndex].mem.majFlag) majCnt++;
        	if (sfdb->hwAlmFlag[sysIndex].mem.criFlag) criCnt++;
		if (sfdb->sys[sysIndex].commInfo.memInfo.level > sysAlmLevel)
			sysAlmLevel = sfdb->sys[sysIndex].commInfo.memInfo.level;
	}
	// Disk
	//
	for (i=0; i<sfdb->sys[sysIndex].commInfo.diskCnt; i++) {
		if (sfdb->sys[sysIndex].commInfo.diskInfo[i].mask == SFM_ALM_MASKED)
			continue;
		// CPU, Memory, Disk는 등급 상승,하강 기능이 있고, 여러등급의 장애가 함께 발생되어
		//  있을 수 있으므로 각각에 대해 count 해야 한다.
		if (sfdb->hwAlmFlag[sysIndex].disk[i].minFlag) minCnt++;
        	if (sfdb->hwAlmFlag[sysIndex].disk[i].majFlag) majCnt++;
        	if (sfdb->hwAlmFlag[sysIndex].disk[i].criFlag) criCnt++;
		if (sfdb->sys[sysIndex].commInfo.diskInfo[i].level > sysAlmLevel)
            		sysAlmLevel = sfdb->sys[sysIndex].commInfo.diskInfo[i].level;
	}

	//QUEUE
	//
	for (i=0; i<sfdb->sys[sysIndex].commInfo.queCnt; i++) {
		if (sfdb->sys[sysIndex].commInfo.queInfo[i].mask == SFM_ALM_MASKED)
			continue;
	    	switch (sfdb->sys[sysIndex].commInfo.queInfo[i].level) {
			case SFM_ALM_MINOR:    minCnt++; break;
			case SFM_ALM_MAJOR:    majCnt++; break;
			case SFM_ALM_CRITICAL: criCnt++; break;
		}

		if (sfdb->sys[sysIndex].commInfo.queInfo[i].level > sysAlmLevel)
			 sysAlmLevel = sfdb->sys[sysIndex].commInfo.queInfo[i].level;
	}
	// LAN
	//
	for (i=0; i<sfdb->sys[sysIndex].commInfo.lanCnt+2; i++) {
		if (sfdb->sys[sysIndex].commInfo.lanInfo[i].status == SFM_LAN_CONNECTED ||
        		sfdb->sys[sysIndex].commInfo.lanInfo[i].mask == SFM_ALM_MASKED)
            		continue;
		criCnt++;  // LAN 장애는 무조건 Critical로 처리한다.
		sysAlmLevel = SFM_ALM_CRITICAL;
    	}
	// RMTLAN
	//
	for (i=0; i<sfdb->sys[sysIndex].commInfo.lanCnt+2; i++) {
		if (sfdb->sys[sysIndex].commInfo.rmtLanInfo[i].status == SFM_LAN_CONNECTED ||
        		sfdb->sys[sysIndex].commInfo.rmtLanInfo[i].mask == SFM_ALM_MASKED)
            		continue;
		criCnt++;  // LAN 장애는 무조건 Critical로 처리한다.
		sysAlmLevel = SFM_ALM_CRITICAL;
    	}
	// OPTLAN
	//
	for (i=0; i<2; i++) {
		if (sfdb->sys[sysIndex].commInfo.optLanInfo[i].status == SFM_LAN_CONNECTED ||
        		sfdb->sys[sysIndex].commInfo.optLanInfo[i].mask == SFM_ALM_MASKED)
            		continue;
		criCnt++;  // LAN 장애는 무조건 Critical로 처리한다.
		sysAlmLevel = SFM_ALM_CRITICAL;
    	}
	// Process
	//
	for (i=0; i<sfdb->sys[sysIndex].commInfo.procCnt; i++) {
        	if (sfdb->sys[sysIndex].commInfo.procInfo[i].status == SFM_STATUS_ALIVE ||
        		sfdb->sys[sysIndex].commInfo.procInfo[i].mask == SFM_ALM_MASKED)
			continue;
        	switch (sfdb->sys[sysIndex].commInfo.procInfo[i].level) {
            		case SFM_ALM_MINOR:    minCnt++; break;
            		case SFM_ALM_MAJOR:    majCnt++; break;
            		case SFM_ALM_CRITICAL: criCnt++; break;
        	}
        	if (sfdb->sys[sysIndex].commInfo.procInfo[i].level > sysAlmLevel)
            		sysAlmLevel = sfdb->sys[sysIndex].commInfo.procInfo[i].level;
    	}

	// DUPLICATION HEART BEAT
	//
	if (sfdb->sys[sysIndex].commInfo.systemDup.mask != SFM_ALM_MASKED)
	{	
		if (sfdb->sys[sysIndex].commInfo.systemDup.heartbeatLevel > sysAlmLevel)
			sysAlmLevel = sfdb->sys[sysIndex].commInfo.systemDup.heartbeatLevel;
	
		if (sfdb->sys[sysIndex].commInfo.systemDup.dualStsAlmLevel > sysAlmLevel)
			sysAlmLevel = sfdb->sys[sysIndex].commInfo.systemDup.dualStsAlmLevel;

		if (sfdb->sys[sysIndex].commInfo.systemDup.timeOutAlmLevel > sysAlmLevel)
			sysAlmLevel = sfdb->sys[sysIndex].commInfo.systemDup.timeOutAlmLevel;
#if 0
		// SCM절체 FAULTED ALARM, 20100916 by dcham
	    if (sfdb->sys[sysIndex].commInfo.systemDup.scmFaultStsAlmLevel > sysAlmLevel)
	        sysAlmLevel = sfdb->sys[sysIndex].commInfo.systemDup.scmFaultStsAlmLevel;
#endif
	}

	// RSRC by helca 08.02
	//
	for (i=0; i<SFM_MAX_RSRC_LOAD_CNT; i++) {
        	if (sfdb->sys[sysIndex].commInfo.rsrcSts[i].mask == SFM_ALM_MASKED)
			continue;
        	switch (sfdb->sys[sysIndex].commInfo.rsrcSts[i].level) {
            		case SFM_ALM_MINOR:    minCnt++; break;
            		case SFM_ALM_MAJOR:    majCnt++; break;
            		case SFM_ALM_CRITICAL: criCnt++; break;
        	}
        	if (sfdb->sys[sysIndex].commInfo.rsrcSts[i].level > sysAlmLevel)
            		sysAlmLevel = sfdb->sys[sysIndex].commInfo.rsrcSts[i].level;
    	}

	// Success Rate by samuel 11.23
	for (i=0; i<SFM_REAL_SUCC_RATE_CNT; i++) {
        	if (sfdb->sys[sysIndex].commInfo.succRate[i].mask == SFM_ALM_MASKED)
			continue;
        	switch(i){
   	    		case 0:		//UAWAP
			{
				int i;
				for(i=0;i<MAX_WAPGW_NUM;i++){
					switch (sfdb->sys[sysIndex].succRateIpInfo.uawap[i].level) {
						case SFM_ALM_MINOR:    minCnt++; break;
						case SFM_ALM_MAJOR:    majCnt++; break;
						case SFM_ALM_CRITICAL: criCnt++; break;
					}
					if (sfdb->sys[sysIndex].succRateIpInfo.uawap[i].level > sysAlmLevel)
						sysAlmLevel = sfdb->sys[sysIndex].succRateIpInfo.uawap[i].level;
				}
			}
			break;
  	     		case 1:		//AAA
			{
				int i;
				for(i=0;i<MAX_AAA_NUM;i++){
					switch (sfdb->sys[sysIndex].succRateIpInfo.aaa[i].level) {
						case SFM_ALM_MINOR:    minCnt++; break;
						case SFM_ALM_MAJOR:    majCnt++; break;
						case SFM_ALM_CRITICAL: criCnt++; break;
					}
					if (sfdb->sys[sysIndex].succRateIpInfo.aaa[i].level > sysAlmLevel)
						sysAlmLevel = sfdb->sys[sysIndex].succRateIpInfo.aaa[i].level;
				}
			}
			break;
        		case 2:
        		case 3:
        		case 4:
			default:
				switch (sfdb->sys[sysIndex].commInfo.succRate[i].level) {
					case SFM_ALM_MINOR:    minCnt++; break;
					case SFM_ALM_MAJOR:    majCnt++; break;
					case SFM_ALM_CRITICAL: criCnt++; break;
				}
			
			if (sfdb->sys[sysIndex].commInfo.succRate[i].level > sysAlmLevel)
				sysAlmLevel = sfdb->sys[sysIndex].commInfo.succRate[i].level;
			break;
        	}
    	}
	// SUCC RATE RADIUS
	//
	for (i=0; i<RADIUS_IP_CNT; i++) {
		if (sfdb->sys[sysIndex].succRateIpInfo.radius[i].mask == SFM_ALM_MASKED)
			continue;
		switch( sfdb->sys[sysIndex].succRateIpInfo.radius[i].level) {
			case SFM_ALM_MINOR:    minCnt++; break;
			case SFM_ALM_MAJOR:    majCnt++; break;
			case SFM_ALM_CRITICAL: criCnt++; break;
		}
		if (sfdb->sys[sysIndex].succRateIpInfo.radius[i].level > sysAlmLevel)
			sysAlmLevel = sfdb->sys[sysIndex].succRateIpInfo.radius[i].level;
	}
	
    	//
    	// sfdb->sys에 장애 등급별 갯수와 전체 등급을 update한다.
    	// - 시스템 전체 등급이 변경된 경우, alarm level change 상태 메시지를 만들어
    	//  cond로 보낸다.
    	//
	if (!strcasecmp (sfdb->sys[sysIndex].commInfo.type, SYSCONF_SYSTYPE_BSD)) {
		sfdb->sys[sysIndex].almInfo.minCnt = minCnt;
		sfdb->sys[sysIndex].almInfo.majCnt = majCnt;
		sfdb->sys[sysIndex].almInfo.criCnt = criCnt;
	}

	sfdb->sys[sysIndex].almInfo.prevLevel = sfdb->sys[sysIndex].almInfo.level;
	sfdb->sys[sysIndex].almInfo.level = sysAlmLevel;

	if (sfdb->sys[sysIndex].almInfo.prevLevel != sfdb->sys[sysIndex].almInfo.level) {
		fimd_makeSysAlmLevelChgMsg (sysIndex);
	}

	if ((groupIndex = fimd_getGroupIndexByName(sfdb->sys[sysIndex].commInfo.group)) < 0){
		return;
	}

    	//
    	// 같은 group에 속한 시스템들의 전체 alarm count와 level을 update한다.
    	// - group alarm level 변경에 대한 상태 메시지는 만들지 않는다.
    	//

    	minCnt = majCnt = criCnt = 0;
    	for (i=0; i<SYSCONF_MAX_ASSO_SYS_NUM; i++) {
        	if (strcasecmp (sfdb->sys[i].commInfo.group, sfdb->sys[sysIndex].commInfo.group))
            		continue;
        	minCnt += sfdb->sys[sysIndex].almInfo.minCnt;
        	majCnt += sfdb->sys[sysIndex].almInfo.majCnt;
        	criCnt += sfdb->sys[sysIndex].almInfo.criCnt;
        	if (groupAlmLevel < sfdb->sys[i].almInfo.level)
            		groupAlmLevel = sfdb->sys[i].almInfo.level;
    	}
    	sfdb->group[groupIndex].level  = groupAlmLevel;
    	sfdb->group[groupIndex].minCnt = minCnt;
    	sfdb->group[groupIndex].majCnt = majCnt;
    	sfdb->group[groupIndex].criCnt = criCnt;
	
	// sfdb 정보를 file에 backup한다.
	//
	fimd_backupSfdb2File ();

	return;

} //----- End of fimd_updateSysAlmInfo -----//

void fimd_updatePDAlmInfo (int sysIndex)
{
	unsigned char	minCnt=0, majCnt=0, criCnt=0;
	unsigned char	sysAlmLevel=SFM_ALM_NORMAL;
	int				i,j;

	// Probe Device CPU
	//
	for(i=0; i<2; i++){	
		if (l3pd->l3ProbeDev[i].cpuInfo.mask == SFM_ALM_MASKED)
			continue;
		// CPU, Memory, Disk는 등급 상승,하강 기능이 있고, 여러등급의 장애가 함께 발생되어
		//  있을 수 있으므로 각각에 대해 count 해야 한다.
		if (l3pd->l3ProbeDev[i].cpuInfo.cpuFlag.minFlag) minCnt++;
        if (l3pd->l3ProbeDev[i].cpuInfo.cpuFlag.majFlag) majCnt++;
        if (l3pd->l3ProbeDev[i].cpuInfo.cpuFlag.criFlag) criCnt++;
		if (l3pd->l3ProbeDev[i].cpuInfo.level > sysAlmLevel)
			sysAlmLevel = l3pd->l3ProbeDev[i].cpuInfo.level;
	}
	// Probe Device Memory
	//
	for(i=0; i<2; i++){	
		if (l3pd->l3ProbeDev[i].memInfo.mask != SFM_ALM_MASKED) {
			// CPU, Memory, Disk는 등급 상승,하강 기능이 있고, 여러등급의 장애가 함께 발생되어
			//  있을 수 있으므로 각각에 대해 count 해야 한다.
		if (l3pd->l3ProbeDev[i].memInfo.memFlag.minFlag) minCnt++;
		if (l3pd->l3ProbeDev[i].memInfo.memFlag.majFlag) majCnt++;
		if (l3pd->l3ProbeDev[i].memInfo.memFlag.criFlag) criCnt++;
		if (l3pd->l3ProbeDev[i].memInfo.level > sysAlmLevel)
			sysAlmLevel = l3pd->l3ProbeDev[i].memInfo.level;
	    }
	}
	// Probe Device FAN
	//
	for (i=0; i<2; i++) {
		for(j=0; j<4; j++){
			if (l3pd->l3ProbeDev[i].fanInfo.status[j] == SFM_LAN_CONNECTED ||
				l3pd->l3ProbeDev[i].fanInfo.mask[j] == SFM_ALM_MASKED)
				continue;
			majCnt++; 
			sysAlmLevel = SFM_ALM_MAJOR;
		}
    }
	// Probr Device GIGA_LAN
	//
	for (i=0; i<2; i++) {
		//for(j=0; j<52; j++){	
		for(j=0; j<MAX_GIGA_LAN_NUM; j++){	
			if (l3pd->l3ProbeDev[i].gigaLanInfo[j].status == SFM_LAN_CONNECTED ||
        		l3pd->l3ProbeDev[i].gigaLanInfo[j].mask == SFM_ALM_MASKED)
				continue;
			criCnt++;  // LAN 장애는 무조건 Critical로 처리한다.
			sysAlmLevel = SFM_ALM_CRITICAL;
		}
	}
	// Probr Device POWER 
	// 20110422 by dcham
	for (i=0; i<2; i++) {
		for(j=0; j<MAX_POWER_NUM; j++){
			if (l3pd->l3ProbeDev[i].powerInfo[j].status == SFM_LAN_CONNECTED ||
					l3pd->l3ProbeDev[i].powerInfo[j].mask == SFM_ALM_MASKED)
				continue;
			criCnt++;
			//sysAlmLevel = SFM_HW_NOT_EQUIP;
			sysAlmLevel = SFM_ALM_CRITICAL;
		}
	}
	fimd_backupL3pd2File();
	}


/*
   by sjjeon
 */
void fimd_updateSceAlmInfo ()
{
	unsigned char	minCnt=0, majCnt=0, criCnt=0;
	unsigned char	sysAlmLevel=SFM_ALM_NORMAL;
	int				i,j;

	// SCE Device CPU
	//
	for(i=0; i<MAX_SCE_DEV_NUM; i++){
		for(j=0; j<MAX_SCE_CPU_CNT; j++){	
			if (g_pstSCEInfo->SCEDev[i].cpuInfo[j].mask == SFM_ALM_MASKED)
				continue;
			// CPU, Memory, Disk는 등급 상승,하강 기능이 있고, 여러등급의 장애가 함께 발생되어
			//  있을 수 있으므로 각각에 대해 count 해야 한다.
//		if (g_pstSCEInfo->SCEDev[i].cpuInfo[j].minFlag) minCnt++;
//			if (g_pstSCEInfo->SCEDev[i].cpuInfo[j].majFlag) majCnt++;
//			if (g_pstSCEInfo->SCEDev[i].cpuInfo[j].criFlag) criCnt++;
			if (g_pstSCEInfo->SCEDev[i].cpuInfo[j].level > sysAlmLevel)
				sysAlmLevel = g_pstSCEInfo->SCEDev[i].cpuInfo[j].level;
		}
	}
	// SCE Device Memory
	//
	for(i=0; i<MAX_SCE_DEV_NUM; i++){	
		for(j=0; j<MAX_SCE_MEM_CNT; j++){	
			if (g_pstSCEInfo->SCEDev[i].memInfo[j].mask != SFM_ALM_MASKED) {
				// CPU, Memory, Disk는 등급 상승,하강 기능이 있고, 여러등급의 장애가 함께 발생되어
				//  있을 수 있으므로 각각에 대해 count 해야 한다.
			if (g_pstSCEInfo->SCEDev[i].memInfo[j].minFlag) minCnt++;
			if (g_pstSCEInfo->SCEDev[i].memInfo[j].majFlag) majCnt++;
			if (g_pstSCEInfo->SCEDev[i].memInfo[j].criFlag) criCnt++;
			if (g_pstSCEInfo->SCEDev[i].memInfo[j].level > sysAlmLevel)
				sysAlmLevel = g_pstSCEInfo->SCEDev[i].memInfo[j].level;
	    	}
		}
	}
	// SCE Device Disk
	//
	for(i=0; i<MAX_SCE_DEV_NUM; i++){	
		if (g_pstSCEInfo->SCEDev[i].diskInfo.mask != SFM_ALM_MASKED) {
			// CPU, Memory, Disk는 등급 상승,하강 기능이 있고, 여러등급의 장애가 함께 발생되어
			//  있을 수 있으므로 각각에 대해 count 해야 한다.
		if (g_pstSCEInfo->SCEDev[i].diskInfo.minFlag) minCnt++;
		if (g_pstSCEInfo->SCEDev[i].diskInfo.majFlag) majCnt++;
		if (g_pstSCEInfo->SCEDev[i].diskInfo.criFlag) criCnt++;
		if (g_pstSCEInfo->SCEDev[i].diskInfo.level > sysAlmLevel)
			sysAlmLevel = g_pstSCEInfo->SCEDev[i].diskInfo.level;
		}
	}

	/* hjjung_20100823 */
	// SCE Device User
	//
	for(i=0; i<MAX_SCE_DEV_NUM; i++){	
		if (g_pstSCEInfo->SCEDev[i].userInfo.mask != SFM_ALM_MASKED) {
			// CPU, Memory, Disk는 등급 상승,하강 기능이 있고, 여러등급의 장애가 함께 발생되어
			//  있을 수 있으므로 각각에 대해 count 해야 한다.
		if (g_pstSCEInfo->SCEDev[i].userInfo.minFlag) minCnt++;
		if (g_pstSCEInfo->SCEDev[i].userInfo.majFlag) majCnt++;
		if (g_pstSCEInfo->SCEDev[i].userInfo.criFlag) criCnt++;
		if (g_pstSCEInfo->SCEDev[i].userInfo.level > sysAlmLevel)
			sysAlmLevel = g_pstSCEInfo->SCEDev[i].userInfo.level;
		}
	}


	// SCE Device Power
	//
	for(i=0; i<MAX_SCE_DEV_NUM; i++){	
		if (g_pstSCEInfo->SCEDev[i].pwrStatus.mask != SFM_ALM_MASKED) {
		if (g_pstSCEInfo->SCEDev[i].pwrStatus.level > sysAlmLevel)
			sysAlmLevel = g_pstSCEInfo->SCEDev[i].pwrStatus.level;
		}
	}

	// SCE Device Fan
	//
	for(i=0; i<MAX_SCE_DEV_NUM; i++){	
		if (g_pstSCEInfo->SCEDev[i].fanStatus.mask != SFM_ALM_MASKED) {
		if (g_pstSCEInfo->SCEDev[i].fanStatus.level > sysAlmLevel)
			sysAlmLevel = g_pstSCEInfo->SCEDev[i].fanStatus.level;
		}
	}

	// SCE Device Temperature
	//
	for(i=0; i<MAX_SCE_DEV_NUM; i++){	
		if (g_pstSCEInfo->SCEDev[i].tempStatus.mask != SFM_ALM_MASKED) {
		if (g_pstSCEInfo->SCEDev[i].tempStatus.level > sysAlmLevel)
			sysAlmLevel = g_pstSCEInfo->SCEDev[i].tempStatus.level;
		}
	}

	// SCE Device PORT
	// MAX_SCE_IFN_CNT
	for(i=0; i<MAX_SCE_DEV_NUM; i++){	
		for(j=0; j<MAX_SCE_IFN_CNT; j++){
			if (g_pstSCEInfo->SCEDev[i].portStatus[j].mask != SFM_ALM_MASKED) {
			if (g_pstSCEInfo->SCEDev[i].portStatus[j].level > sysAlmLevel)
				sysAlmLevel = g_pstSCEInfo->SCEDev[i].portStatus[j].level;
			}
		}
	}

	// SCE Device RDR Connect
	//
	for(i=0; i<MAX_SCE_DEV_NUM; i++){	
		// sjjeon : sysAlmLevel 적용하는데... for문으로 여러값을 설정하는데...
		// 값 설정시 이전 값은 무시되는 현상???? 분석해야 한다.. 
		for(j=0; j<MAX_SCE_RDR_INFO_CNT; j++){	
			if (g_pstSCEInfo->SCEDev[i].rdrConnStatus[j].mask != SFM_ALM_MASKED) 
			{
				if (g_pstSCEInfo->SCEDev[i].rdrConnStatus[j].level > sysAlmLevel)
					sysAlmLevel = g_pstSCEInfo->SCEDev[i].rdrConnStatus[j].level;
			}
		}
	}
	// 상태 file backup
	fimd_backupSCE2File();

}

/* hjjung_20100823 */
void fimd_updateLegAlmInfo ()
{
	unsigned char	minCnt=0, majCnt=0, criCnt=0;
	unsigned char	sysAlmLevel=SFM_ALM_NORMAL;
	int				i;
	
	for(i=0; i<MAX_CALL_DEV_NUM; i++){	
		if (g_pstCALLInfo->legInfo[i].mask != SFM_ALM_MASKED) {
			// LEG Session은 등급 상승,하강 기능이 있고, 여러등급의 장애가 함께 발생되어
			// 있을 수 있으므로 각각에 대해 count 해야 한다.
			if (g_pstCALLInfo->legInfo[i].minFlag) minCnt++;
			if (g_pstCALLInfo->legInfo[i].majFlag) majCnt++;
			if (g_pstCALLInfo->legInfo[i].criFlag) criCnt++;
			if (g_pstCALLInfo->legInfo[i].level > sysAlmLevel)
				sysAlmLevel = g_pstCALLInfo->legInfo[i].level;
		}
	}
	// 상태 file backup
	fimd_backupCALLFile();
}

/* dcham 2011.05.25 */
void fimd_updateCallAlmInfo ()
{
	unsigned char	minCnt=0, majCnt=0, criCnt=0;
	unsigned char	sysAlmLevel=SFM_ALM_NORMAL;
	int				i;
#if 0
	for(i=0; i<MAX_CALL_DEV_NUM; i++){	
		if (g_pstCALLInfo->tpsInfo.mask != SFM_ALM_MASKED) {
			// TPS는 등급 상승,하강 기능이 있고, 여러등급의 장애가 함께 발생되어
			// 있을 수 있으므로 각각에 대해 count 해야 한다.
		if (g_pstCALLInfo->tpsInfo.minFlag) minCnt++;
		if (g_pstCALLInfo->tpsInfo.majFlag) majCnt++;
		if (g_pstCALLInfo->tpsInfo.criFlag) criCnt++;
		if (g_pstCALLInfo->tpsInfo.level > sysAlmLevel)
			sysAlmLevel = g_pstCALLInfo->tpsInfo.level;
		}
	}
#endif
        for(i=0; i<MAX_CALL_DEV_NUM; i++){
		if (g_pstCALLInfo->tpsInfo[i].mask != SFM_ALM_MASKED) {
			// TPS는 등급 상승,하강 기능이 있고, 여러등급의 장애가 함께 발생되어
			// 있을 수 있으므로 각각에 대해 count 해야 한다.
			if (g_pstCALLInfo->tpsInfo[i].minFlag) minCnt++;
			if (g_pstCALLInfo->tpsInfo[i].majFlag) majCnt++;
			if (g_pstCALLInfo->tpsInfo[i].criFlag) criCnt++;
			if (g_pstCALLInfo->tpsInfo[i].level > sysAlmLevel)
				sysAlmLevel = g_pstCALLInfo->tpsInfo[i].level;
		}
        }
	// 상태 file backup
	fimd_backupCALLFile();
}
void fimd_updateL2SWlmInfo (int sysIndex)
{
	unsigned char	minCnt=0, majCnt=0, criCnt=0;
	unsigned char	sysAlmLevel=SFM_ALM_NORMAL;
	int				i,j;

	// Probe Device CPU
	//
	for(i=0; i<2; i++){	
		if (g_pstL2Dev->l2Info[i].cpuInfo.mask == SFM_ALM_MASKED)
			continue;
		// CPU, Memory, Disk는 등급 상승,하강 기능이 있고, 여러등급의 장애가 함께 발생되어
		//  있을 수 있으므로 각각에 대해 count 해야 한다.
		if (g_pstL2Dev->l2Info[i].cpuInfo.cpuFlag.minFlag) minCnt++;
        if (g_pstL2Dev->l2Info[i].cpuInfo.cpuFlag.majFlag) majCnt++;
        if (g_pstL2Dev->l2Info[i].cpuInfo.cpuFlag.criFlag) criCnt++;
		if (g_pstL2Dev->l2Info[i].cpuInfo.level > sysAlmLevel)
			sysAlmLevel = g_pstL2Dev->l2Info[i].cpuInfo.level;
	}
	// L2sw Memory
	//
	for(i=0; i<2; i++){	
		if (g_pstL2Dev->l2Info[i].memInfo.mask != SFM_ALM_MASKED) {
			// CPU, Memory, Disk는 등급 상승,하강 기능이 있고, 여러등급의 장애가 함께 발생되어
			//  있을 수 있으므로 각각에 대해 count 해야 한다.
		if (g_pstL2Dev->l2Info[i].memInfo.memFlag.minFlag) minCnt++;
		if (g_pstL2Dev->l2Info[i].memInfo.memFlag.majFlag) majCnt++;
		if (g_pstL2Dev->l2Info[i].memInfo.memFlag.criFlag) criCnt++;
		if (g_pstL2Dev->l2Info[i].memInfo.level > sysAlmLevel)
			sysAlmLevel = g_pstL2Dev->l2Info[i].memInfo.level;
	    }
	}

	// Probr Device GIGA_LAN
	//
	for (i=0; i<MAX_L2_DEV_NUM; i++) {
		for(j=0; j<MAX_L2_PORT_NUM; j++){	
			if (g_pstL2Dev->l2Info[i].portInfo[j].status == SFM_LAN_CONNECTED ||
        		g_pstL2Dev->l2Info[i].portInfo[j].mask == SFM_ALM_MASKED)
				continue;
			criCnt++;  // LAN 장애는 무조건 Critical로 처리한다.
			sysAlmLevel = SFM_ALM_CRITICAL;
		}
    }
	fimd_backupL2sw2File();
}

