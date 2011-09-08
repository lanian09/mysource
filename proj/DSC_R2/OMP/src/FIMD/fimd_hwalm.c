#include "fimd_proto.h"

extern char	trcBuf[4096], trcTmp[1024];
extern SFM_sfdb		*sfdb;
extern SFM_L3PD		*l3pd;
extern SFM_SCE		*g_pstSCEInfo;
/* hjjung */
//extern SFM_LEG		*g_pstLEGInfo;
extern SFM_CALL		*g_pstCALLInfo;
extern SFM_L2Dev	*g_pstL2Dev;
extern SFM_LOGON	*g_pstLogonRate;
extern FimdHwAlmCheckBuff	cpuChkBuff[SYSCONF_MAX_ASSO_SYS_NUM][SFM_MAX_CPU_CNT];
extern FimdHwAlmCheckBuff	memChkBuff[SYSCONF_MAX_ASSO_SYS_NUM];
extern FimdHwAlmCheckBuff	diskChkBuff[SYSCONF_MAX_ASSO_SYS_NUM][SFM_MAX_DISK_CNT];

FimdHwAlmCheckBuff	rsrcLoadChkBuff[SYSCONF_MAX_ASSO_SYS_NUM][SFM_MAX_RSRC_LOAD_CNT];
extern time_t	currentTime;
extern int	trcFlag, trcLogFlag;

void setTapInfo(int sysIndex, int unitType,int unitIndex, SCE_SYS_STATUS_INFO *pStsInfo);
void setL2swInfo(int sysIndex, int unitType,int unitIndex, SCE_SYS_STATUS_INFO *pStsInfo);

/* by helca*/
FimdHwAlmCheckBuff	pdCpuChkBuff[MAX_PROBE_DEV_NUM];
FimdHwAlmCheckBuff	pdMemChkBuff[MAX_PROBE_DEV_NUM];
FimdHwAlmCheckBuff	queLoadChkBuff[SYSCONF_MAX_ASSO_SYS_NUM][SFM_MAX_QUE_CNT];

FimdHwAlmCheckBuff	sceCpuChkBuff[MAX_SCE_DEV_NUM][MAX_SCE_CPU_CNT];
FimdHwAlmCheckBuff	sceMemChkBuff[MAX_SCE_DEV_NUM][MAX_SCE_MEM_CNT];
FimdHwAlmCheckBuff	sceDiskChkBuff[MAX_SCE_DEV_NUM];
/* hjjung_20100823 */
FimdHwAlmCheckBuff	sceUserChkBuff[MAX_SCE_DEV_NUM];
FimdHwAlmCheckBuff	scePwrChkBuff[MAX_SCE_DEV_NUM];
FimdHwAlmCheckBuff	sceFanChkBuff[MAX_SCE_DEV_NUM];
FimdHwAlmCheckBuff	sceTempChkBuff[MAX_SCE_DEV_NUM];
FimdHwAlmCheckBuff	sceVoltChkBuff[MAX_SCE_DEV_NUM];
FimdHwAlmCheckBuff	scePortChkBuff[MAX_SCE_DEV_NUM];
FimdHwAlmCheckBuff	sceRdrChkBuff[MAX_SCE_DEV_NUM];
FimdHwAlmCheckBuff	sceRdrConnChkBuff[MAX_SCE_DEV_NUM];

FimdHwAlmCheckBuff  l2swCpuChkBuff[MAX_L2_DEV_NUM];
FimdHwAlmCheckBuff	l2swMemChkBuff[MAX_L2_DEV_NUM];

/* hjjung */
FimdHwAlmCheckBuff	legSessionChkBuff[MAX_CALL_DEV_NUM];
/* added by dcham 20110525 for TPS */
FimdHwAlmCheckBuff	legTpsChkBuff[MAX_CALL_DEV_NUM];
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int fimd_checkCpuUsageAlm (int sysIndex, int cpuIndex, SFM_CpuInfo *cpuInfo)
{
	int		usage;
	int		changeFlag=0; // 장애 발생 또는 해지인 경우 1로 setting된다.

	// 천분율 값을 백분율 값으로 바꿔서 비교한다.
	usage = cpuInfo->usage[cpuIndex] / 10;
	if ((cpuInfo->usage[cpuIndex] % 10) >= 5){
		usage++;
	}
//	printf(" %s cpu usage : %d\n", sfdb->sys[sysIndex].commInfo.name, usage);

#if 0 
//	else if(sfdb->sys[sysIndex].commInfo.name != "DSCM"){ // 코드가 잘못되었다. sjjeon
		//usage = usage/10;    // 10으로 나누지 않는다. sjjeon
		//printf("DSCM cpu usage : %d\n", usage);
//	}
#endif 

	// minor 장애 기준값보다 낮은 경우,
	// - 이전에 장애가 발생되어 있었으면 장애가 해지된 경우이므로,
	//	발생되었던 각각 해당 등급에 대한 해지 메시지를 만들어 cond로 보낸다.
	//
	if (usage < cpuInfo->minLimit)
	{
		cpuChkBuff[sysIndex][cpuIndex].minStartTime = 0; 
		cpuChkBuff[sysIndex][cpuIndex].majStartTime = 0;
		cpuChkBuff[sysIndex][cpuIndex].criStartTime = 0;

		cpuInfo->level[cpuIndex] = SFM_ALM_NORMAL;  // sfdb->sys에 update

		if (sfdb->hwAlmFlag[sysIndex].cpu[cpuIndex].criFlag) {
			fimd_hdlCpuUsageAlm (sysIndex, cpuIndex, SFM_ALM_NORMAL, SFM_ALM_CRITICAL, 0); // critical clear
			changeFlag = 1;
		}
		if (sfdb->hwAlmFlag[sysIndex].cpu[cpuIndex].majFlag) {
			fimd_hdlCpuUsageAlm (sysIndex, cpuIndex, SFM_ALM_NORMAL, SFM_ALM_MAJOR, 0); // major clear
			changeFlag = 1;
		}
		if (sfdb->hwAlmFlag[sysIndex].cpu[cpuIndex].minFlag) {
			fimd_hdlCpuUsageAlm (sysIndex, cpuIndex, SFM_ALM_NORMAL, SFM_ALM_MINOR, 0); // minor clear
			changeFlag = 1;
		}

		sfdb->hwAlmFlag[sysIndex].cpu[cpuIndex].criFlag = 0;
		sfdb->hwAlmFlag[sysIndex].cpu[cpuIndex].majFlag = 0;
		sfdb->hwAlmFlag[sysIndex].cpu[cpuIndex].minFlag = 0;

	} //-- end of normal --//

	// minor보다 높고 major 기준값보다 낮은 경우, (minor 장애 조건)
	// - 더 높은 등급(major, critical)이 발생되어 있었으면, 등급 하강 조건이므로,
	//	각각 해지 메시지를 만든다.
	// - minor가 이미 발생되어 있으면 no action.
	// - minor 기준값 지속시간 경과여부를 확인하여 장애 판단.
	//
	else if (usage < cpuInfo->majLimit)
	{
		cpuChkBuff[sysIndex][cpuIndex].majStartTime = 0;
		cpuChkBuff[sysIndex][cpuIndex].criStartTime = 0;

		if (sfdb->hwAlmFlag[sysIndex].cpu[cpuIndex].criFlag) {
		//	cpuInfo->level[cpuIndex] = SFM_ALM_MINOR;  // sfdb->sys에 update
			fimd_hdlCpuUsageAlm (sysIndex, cpuIndex, SFM_ALM_MINOR, SFM_ALM_CRITICAL, 0); // critical clear
			sfdb->hwAlmFlag[sysIndex].cpu[cpuIndex].criFlag = 0;
			changeFlag = 1;
		}
		if (sfdb->hwAlmFlag[sysIndex].cpu[cpuIndex].majFlag) {
	//		cpuInfo->level[cpuIndex] = SFM_ALM_MINOR;  // sfdb->sys에 update
			fimd_hdlCpuUsageAlm (sysIndex, cpuIndex, SFM_ALM_MINOR, SFM_ALM_MAJOR, 0); // major clear
			sfdb->hwAlmFlag[sysIndex].cpu[cpuIndex].majFlag = 0;
			changeFlag = 1;
		}

		if (sfdb->hwAlmFlag[sysIndex].cpu[cpuIndex].minFlag) {
			; // 이미 minor가 발생되어 있으면 no action.
		} else {
			if (cpuChkBuff[sysIndex][cpuIndex].minStartTime == 0) // 처음 limit를 넘었는가?
				cpuChkBuff[sysIndex][cpuIndex].minStartTime = currentTime;
			if (cpuInfo->minDurat > (currentTime - cpuChkBuff[sysIndex][cpuIndex].minStartTime)) {
				; // 지속시간 경과하지 않았으면 no action.
			} else {
				cpuInfo->level[cpuIndex] = SFM_ALM_MINOR;  // sfdb->sys에 update
				fimd_hdlCpuUsageAlm (sysIndex, cpuIndex, SFM_ALM_MINOR, 0, 1); // minor occur
				sfdb->hwAlmFlag[sysIndex].cpu[cpuIndex].criFlag = 0;
				sfdb->hwAlmFlag[sysIndex].cpu[cpuIndex].majFlag = 0;
				sfdb->hwAlmFlag[sysIndex].cpu[cpuIndex].minFlag = 1;
				changeFlag = 1;
			}
		}

	} //-- end of minor condition --//

	// major보다 높고 critical 기준값보다 낮은 경우, (major 장애 조건)
	// - 더 높은 등급(critical)이 발생되어 있었으면, 등급 하강 조건이므로 해지 메시지를 만든다.
	// - major가 이미 발생되어 있으면 no action.
	// - major 기준값 지속시간 경과여부를 확인하여 장애 판단.
	//
	else if (usage < cpuInfo->criLimit)
	{
//if(sysIndex == 1)
//printf("[%d]cpu major... !!\n",sysIndex);
		cpuChkBuff[sysIndex][cpuIndex].criStartTime = 0;

		if (sfdb->hwAlmFlag[sysIndex].cpu[cpuIndex].criFlag) {
		//	cpuInfo->level[cpuIndex] = SFM_ALM_MAJOR;  // sfdb->sys에 update
			fimd_hdlCpuUsageAlm (sysIndex, cpuIndex, SFM_ALM_MAJOR, SFM_ALM_CRITICAL, 0); // critical clear
			sfdb->hwAlmFlag[sysIndex].cpu[cpuIndex].criFlag = 0;	
			changeFlag = 1;
		}

		if (sfdb->hwAlmFlag[sysIndex].cpu[cpuIndex].majFlag) {
			; // 이미 major가 발생되어 있으면 no action.
		} else {
			// minor에서 상승하는게 아니라 한방에 major로 올라갈 수 있으므로
			//	minStartTime을 같이 변경해야한다.
			if (cpuChkBuff[sysIndex][cpuIndex].minStartTime == 0) // 처음 limit를 넘었는가?
				cpuChkBuff[sysIndex][cpuIndex].minStartTime = currentTime;
			if (cpuChkBuff[sysIndex][cpuIndex].majStartTime == 0) // 처음 limit를 넘었는가?
				cpuChkBuff[sysIndex][cpuIndex].majStartTime = currentTime;

			if (cpuInfo->majDurat > (currentTime - cpuChkBuff[sysIndex][cpuIndex].majStartTime)) {
				; // 지속시간 경과하지 않았으면 no action.
			} else {
				cpuInfo->level[cpuIndex] = SFM_ALM_MAJOR;  // sfdb->sys에 update
				fimd_hdlCpuUsageAlm (sysIndex, cpuIndex, SFM_ALM_MAJOR, 0, 1); // major occur
				sfdb->hwAlmFlag[sysIndex].cpu[cpuIndex].criFlag = 0;
				sfdb->hwAlmFlag[sysIndex].cpu[cpuIndex].majFlag = 1;
				sfdb->hwAlmFlag[sysIndex].cpu[cpuIndex].minFlag = 0;
				changeFlag = 1;
			}
		}

	} //-- end of major condition --//

	// critical 기준값보다 높은 경우, (critical 장애 조건)
	// - critical이 이미 발생되어 있으면 no action.
	// - critical 기준값 지속시간 경과여부를 확인하여 장애 판단.
	//
	else
	{
//if(sysIndex == 1)
//printf("[%d]cpu critical... !!\n", sysIndex);
		if (sfdb->hwAlmFlag[sysIndex].cpu[cpuIndex].criFlag) {
			; // 이미 발생되어 있으면 no action.
		} else {
			// minor,major에서 상승하는게 아니라 한방에 critical로 올라갈 수 있으므로
			//	minStartTime, majorStartTime을 같이 변경해야한다.
			if (cpuChkBuff[sysIndex][cpuIndex].minStartTime == 0) // 처음 limit를 넘었는가?
				cpuChkBuff[sysIndex][cpuIndex].minStartTime = currentTime;
			if (cpuChkBuff[sysIndex][cpuIndex].majStartTime == 0) // 처음 limit를 넘었는가?
				cpuChkBuff[sysIndex][cpuIndex].majStartTime = currentTime;
			if (cpuChkBuff[sysIndex][cpuIndex].criStartTime == 0) // 처음 limit를 넘었는가?
				cpuChkBuff[sysIndex][cpuIndex].criStartTime = currentTime;

			if (cpuInfo->criDurat > (currentTime - cpuChkBuff[sysIndex][cpuIndex].criStartTime)) {
				; // 지속시간 경과하지 않았으면 no action.
			} else {
				cpuInfo->level[cpuIndex] = SFM_ALM_CRITICAL;  // sfdb->sys에 update
				fimd_hdlCpuUsageAlm (sysIndex, cpuIndex, SFM_ALM_CRITICAL, 0, 1); // critical occur
				sfdb->hwAlmFlag[sysIndex].cpu[cpuIndex].criFlag = 1;
				sfdb->hwAlmFlag[sysIndex].cpu[cpuIndex].majFlag = 0;
				sfdb->hwAlmFlag[sysIndex].cpu[cpuIndex].minFlag = 0;
				changeFlag = 1;
			}
		}

	} //-- end of critical condition --//


	return changeFlag;

} //----- End of fimd_checkCpuUsageAlm -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int fimd_checkMemUsageAlm (int sysIndex, SFM_MemInfo *memInfo)
{
	int		usage, changeFlag=0;

	// 천분율 값을 백분율 값으로 바꿔서 비교한다.
	usage = memInfo->usage / 10;
	if ((memInfo->usage % 10) >= 5)
		usage++;

	// minor 장애 기준값보다 낮은 경우,
	// - 이전에 장애가 발생되어 있었으면 장애가 해지된 경우이므로,
	//	발생되었던 각각 해당 등급에 대한 해지 메시지를 만들어 cond로 보낸다.
	//
	if (usage < memInfo->minLimit)
	{
		memChkBuff[sysIndex].minStartTime = 0;
		memChkBuff[sysIndex].majStartTime = 0;
		memChkBuff[sysIndex].criStartTime = 0;

		memInfo->level = SFM_ALM_NORMAL;  // sfdb->sys에 update

		if (sfdb->hwAlmFlag[sysIndex].mem.criFlag) {
			fimd_hdlMemUsageAlm (sysIndex, SFM_ALM_NORMAL, SFM_ALM_CRITICAL, 0); // critical clear
			changeFlag = 1;
		}
		if (sfdb->hwAlmFlag[sysIndex].mem.majFlag) {
			fimd_hdlMemUsageAlm (sysIndex, SFM_ALM_NORMAL, SFM_ALM_MAJOR, 0); // major clear
			changeFlag = 1;
		}
		if (sfdb->hwAlmFlag[sysIndex].mem.minFlag) {
			fimd_hdlMemUsageAlm (sysIndex, SFM_ALM_NORMAL, SFM_ALM_MINOR, 0); // minor clear
			changeFlag = 1;
		}

		sfdb->hwAlmFlag[sysIndex].mem.criFlag = 0;
		sfdb->hwAlmFlag[sysIndex].mem.majFlag = 0;
		sfdb->hwAlmFlag[sysIndex].mem.minFlag = 0;

	} //-- end of normal --//

	// minor보다 높고 major 기준값보다 낮은 경우, (minor 장애 조건)
	// - 더 높은 등급(major, critical)이 발생되어 있었으면, 등급 하강 조건이므로,
	//	각각 해지 메시지를 만든다.
	// - minor가 이미 발생되어 있으면 no action.
	// - minor 기준값 지속시간 경과여부를 확인하여 장애 판단.
	//
	else if (usage < memInfo->majLimit)
	{
		memChkBuff[sysIndex].majStartTime = 0;
		memChkBuff[sysIndex].criStartTime = 0;

		if (sfdb->hwAlmFlag[sysIndex].mem.criFlag) {
			fimd_hdlMemUsageAlm (sysIndex, SFM_ALM_MINOR, SFM_ALM_CRITICAL, 0); // critical clear
			sfdb->hwAlmFlag[sysIndex].mem.criFlag =0;
			changeFlag = 1;
		}
		if (sfdb->hwAlmFlag[sysIndex].mem.majFlag) {
			fimd_hdlMemUsageAlm (sysIndex, SFM_ALM_MINOR, SFM_ALM_MAJOR, 0); // major clear
			sfdb->hwAlmFlag[sysIndex].mem.majFlag =0;
			changeFlag = 1;
		}

		if (sfdb->hwAlmFlag[sysIndex].mem.minFlag) {
			; // 이미 minor가 발생되어 있으면 no action.
		} else {
			if (memChkBuff[sysIndex].minStartTime == 0) // 처음 limit를 넘었는가?
				memChkBuff[sysIndex].minStartTime = currentTime;
			if (memInfo->minDurat > (currentTime - memChkBuff[sysIndex].minStartTime)) {
				; // 지속시간 경과하지 않았으면 no action.
			}else{
				memInfo->level = SFM_ALM_MINOR;  // sfdb->sys에 update
				fimd_hdlMemUsageAlm (sysIndex, SFM_ALM_MINOR, 0, 1); // minor occur
				sfdb->hwAlmFlag[sysIndex].mem.criFlag = 0;
				sfdb->hwAlmFlag[sysIndex].mem.majFlag = 0;
				sfdb->hwAlmFlag[sysIndex].mem.minFlag = 1;
				changeFlag = 1;
			}
		}

	} //-- end of minor condition --//

	// major보다 높고 critical 기준값보다 낮은 경우, (major 장애 조건)
	// - 더 높은 등급(critical)이 발생되어 있었으면, 등급 하강 조건이므로 해지 메시지를 만든다.
	// - major가 이미 발생되어 있으면 no action.
	// - major 기준값 지속시간 경과여부를 확인하여 장애 판단.
	//
	else if (usage < memInfo->criLimit)
	{
		memChkBuff[sysIndex].criStartTime = 0;

		if (sfdb->hwAlmFlag[sysIndex].mem.criFlag) {
		//	memInfo->level = SFM_ALM_MAJOR;  // sfdb->sys에 update	
			fimd_hdlMemUsageAlm (sysIndex, SFM_ALM_MAJOR, SFM_ALM_CRITICAL, 0); // critical clear
			sfdb->hwAlmFlag[sysIndex].mem.criFlag = 0;	
			changeFlag = 1;
		}

		if (sfdb->hwAlmFlag[sysIndex].mem.majFlag) {
			; // 이미 major가 발생되어 있으면 no action.
		} else {
			// minor에서 상승하는게 아니라 한방에 major로 올라갈 수 있으므로
			//	minStartTime을 같이 변경해야한다.
			if (memChkBuff[sysIndex].minStartTime == 0) // 처음 limit를 넘었는가?
				memChkBuff[sysIndex].minStartTime = currentTime;
			if (memChkBuff[sysIndex].majStartTime == 0) // 처음 limit를 넘었는가?
				memChkBuff[sysIndex].majStartTime = currentTime;

			if (memInfo->majDurat > (currentTime - memChkBuff[sysIndex].majStartTime)) {
				; // 지속시간 경과하지 않았으면 no action.
			} else {
				memInfo->level = SFM_ALM_MAJOR;  // sfdb->sys에 update
				fimd_hdlMemUsageAlm (sysIndex, SFM_ALM_MAJOR, 0, 1); // major occur
				sfdb->hwAlmFlag[sysIndex].mem.criFlag = 0;
				sfdb->hwAlmFlag[sysIndex].mem.majFlag = 1;
				sfdb->hwAlmFlag[sysIndex].mem.minFlag = 0;
				changeFlag = 1;
			}
		}

	} //-- end of major condition --//

	// critical 기준값보다 높은 경우, (critical 장애 조건)
	// - critical이 이미 발생되어 있으면 no action.
	// - critical 기준값 지속시간 경과여부를 확인하여 장애 판단.
	//
	else
	{
		if (sfdb->hwAlmFlag[sysIndex].mem.criFlag) {
			; // 이미 발생되어 있으면 no action.
		} else {
			// minor,major에서 상승하는게 아니라 한방에 critical로 올라갈 수 있으므로
			//	minStartTime, majorStartTime을 같이 변경해야한다.
			if (memChkBuff[sysIndex].minStartTime == 0) // 처음 limit를 넘었는가?
				memChkBuff[sysIndex].minStartTime = currentTime;
			if (memChkBuff[sysIndex].majStartTime == 0) // 처음 limit를 넘었는가?
				memChkBuff[sysIndex].majStartTime = currentTime;
			if (memChkBuff[sysIndex].criStartTime == 0) // 처음 limit를 넘었는가?
				memChkBuff[sysIndex].criStartTime = currentTime;

			if (memInfo->criDurat > (currentTime - memChkBuff[sysIndex].criStartTime)) {
				; // 지속시간 경과하지 않았으면 no action.
			}else{
				memInfo->level = SFM_ALM_CRITICAL;  // sfdb->sys에 update
				fimd_hdlMemUsageAlm (sysIndex, SFM_ALM_CRITICAL, 0, 1); // critical occur
				sfdb->hwAlmFlag[sysIndex].mem.criFlag = 1;
				sfdb->hwAlmFlag[sysIndex].mem.majFlag = 0;
				sfdb->hwAlmFlag[sysIndex].mem.minFlag = 0;
				changeFlag = 1;
			}
		}

	} //-- end of critical condition --//


	return changeFlag;

} //----- End of fimd_checkMemUsageAlm -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int fimd_checkDiskUsageAlm (int sysIndex, int diskIndex, SFM_DiskInfo *diskInfo)
{
	int		usage=0, changeFlag=0;

	usage = diskInfo->usage / 10;
	// 천분율 값을 백분율 값으로 바꿔서 비교한다.
// yhshin
//	if(!strcasecmp(sfdb->sys[sysIndex].commInfo.name, "DSCM"))
//		usage = diskInfo->usage / 10;
//	else usage = diskInfo->usage;

	// sjjeon usage를 추가하는 이유룰 알수 없다. 
	//if ((diskInfo->usage % 10) >= 5) {
	//	usage++;
	//}

//	printf("%s disk_usage: %d\n",sfdb->sys[sysIndex].commInfo.name, usage);
	// minor 장애 기준값보다 낮은 경우,
	// - 이전에 장애가 발생되어 있었으면 장애가 해지된 경우이므로,
	//	발생되었던 각각 해당 등급에 대한 해지 메시지를 만들어 cond로 보낸다.
	//
	if (usage < diskInfo->minLimit)
	{
		diskInfo->level = SFM_ALM_NORMAL;  // sfdb->sys에 update

		if (sfdb->hwAlmFlag[sysIndex].disk[diskIndex].criFlag) {
			fimd_hdlDiskUsageAlm (sysIndex, diskIndex, SFM_ALM_NORMAL, SFM_ALM_CRITICAL, 0); // critical clear
			changeFlag = 1;
		}
		if (sfdb->hwAlmFlag[sysIndex].disk[diskIndex].majFlag) {
			fimd_hdlDiskUsageAlm (sysIndex, diskIndex, SFM_ALM_NORMAL, SFM_ALM_MAJOR, 0); // major clear
			changeFlag = 1;
		}
		if (sfdb->hwAlmFlag[sysIndex].disk[diskIndex].minFlag) {
			fimd_hdlDiskUsageAlm (sysIndex, diskIndex, SFM_ALM_NORMAL, SFM_ALM_MINOR, 0); // minor clear
			changeFlag = 1;
		}

		sfdb->hwAlmFlag[sysIndex].disk[diskIndex].criFlag = 0;
		sfdb->hwAlmFlag[sysIndex].disk[diskIndex].majFlag = 0;
		sfdb->hwAlmFlag[sysIndex].disk[diskIndex].minFlag = 0;

	} //-- end of normal --//

	// minor보다 높고 major 기준값보다 낮은 경우, (minor 장애 조건)
	// - 더 높은 등급(major, critical)이 발생되어 있었으면, 등급 하강 조건이므로,
	//	각각 해지 메시지를 만든다.
	// - minor가 이미 발생되어 있으면 no action.
	// - disk 사용율 장애는 지속시간이 없으므로 곧바로 장애 판단.
	//
	else if (usage < diskInfo->majLimit)
	{
		diskInfo->level = SFM_ALM_MINOR;  // sfdb->sys에 update

		if (sfdb->hwAlmFlag[sysIndex].disk[diskIndex].criFlag) {
			fimd_hdlDiskUsageAlm (sysIndex, diskIndex, SFM_ALM_MINOR, SFM_ALM_CRITICAL, 0); // critical clear
			changeFlag = 1;
		}
		if (sfdb->hwAlmFlag[sysIndex].disk[diskIndex].majFlag) {
			fimd_hdlDiskUsageAlm (sysIndex, diskIndex, SFM_ALM_MINOR, SFM_ALM_MAJOR, 0); // major clear
			changeFlag = 1;
		}

		if (sfdb->hwAlmFlag[sysIndex].disk[diskIndex].minFlag) {
			; // 이미 minor가 발생되어 있으면 no action.
		} else {
			fimd_hdlDiskUsageAlm (sysIndex, diskIndex, SFM_ALM_MINOR, 0, 1); // minor occur
			sfdb->hwAlmFlag[sysIndex].disk[diskIndex].criFlag = 0;
			sfdb->hwAlmFlag[sysIndex].disk[diskIndex].majFlag = 0;
			sfdb->hwAlmFlag[sysIndex].disk[diskIndex].minFlag = 1;
			changeFlag = 1;
		}

	} //-- end of minor condition --//

	// major보다 높고 critical 기준값보다 낮은 경우, (major 장애 조건)
	// - 더 높은 등급(critical)이 발생되어 있었으면, 등급 하강 조건이므로 해지 메시지를 만든다.
	// - major가 이미 발생되어 있으면 no action.
	//
	else if (usage < diskInfo->criLimit)
	{
		diskInfo->level = SFM_ALM_MAJOR;  // sfdb->sys에 update

		if (sfdb->hwAlmFlag[sysIndex].disk[diskIndex].minFlag) {
			fimd_hdlDiskUsageAlm (sysIndex, diskIndex, SFM_ALM_MAJOR, SFM_ALM_MINOR, 0); // minor clear
			sfdb->hwAlmFlag[sysIndex].disk[diskIndex].minFlag = 0; 
			changeFlag = 1;
		}

		if (sfdb->hwAlmFlag[sysIndex].disk[diskIndex].criFlag) {
			fimd_hdlDiskUsageAlm (sysIndex, diskIndex, SFM_ALM_MAJOR, SFM_ALM_CRITICAL, 0); // critical clear
			sfdb->hwAlmFlag[sysIndex].disk[diskIndex].criFlag = 0;	
			changeFlag = 1;
		}

		if (sfdb->hwAlmFlag[sysIndex].disk[diskIndex].majFlag) {
			; // 이미 major가 발생되어 있으면 no action.
		} else {
			fimd_hdlDiskUsageAlm (sysIndex, diskIndex, SFM_ALM_MAJOR, 0, 1); // major occur
			sfdb->hwAlmFlag[sysIndex].disk[diskIndex].criFlag = 0;
			sfdb->hwAlmFlag[sysIndex].disk[diskIndex].majFlag = 1;
			sfdb->hwAlmFlag[sysIndex].disk[diskIndex].minFlag = 0;
			changeFlag = 1;
		}

	} //-- end of major condition --//

	// critical 기준값보다 높은 경우, (critical 장애 조건)
	// - critical이 이미 발생되어 있으면 no action.
	// - critical 기준값 지속시간 경과여부를 확인하여 장애 판단.
	//
	else
	{
		diskInfo->level = SFM_ALM_CRITICAL;  // sfdb->sys에 update

		if (sfdb->hwAlmFlag[sysIndex].disk[diskIndex].criFlag) {
			; // 이미 발생되어 있으면 no action.
		} else {
			fimd_hdlDiskUsageAlm (sysIndex, diskIndex, SFM_ALM_CRITICAL, 0, 1); // critical occur
			sfdb->hwAlmFlag[sysIndex].disk[diskIndex].criFlag = 1;
			sfdb->hwAlmFlag[sysIndex].disk[diskIndex].majFlag = 0;
			sfdb->hwAlmFlag[sysIndex].disk[diskIndex].minFlag = 0;
			changeFlag = 1;
		}

	} //-- end of critical condition --//


	return changeFlag;

} //----- End of fimd_checkDiskUsageAlm -----//

/* by helca */
//------------------------------------------------------------------------------
int fimd_checkPDCpuUsageAlm (int devIndex, SFM_PDCpuInfo *cpuInfo)
{
	int		usage;
	int		changeFlag=0; // 장애 발생 또는 해지인 경우 1로 setting된다.

	usage = cpuInfo->usage;
	
	// minor 장애 기준값보다 낮은 경우,
	// - 이전에 장애가 발생되어 있었으면 장애가 해지된 경우이므로,
	//	발생되었던 각각 해당 등급에 대한 해지 메시지를 만들어 cond로 보낸다.
	//
	if (usage < cpuInfo->minLimit)
	{
		pdCpuChkBuff[devIndex].minStartTime = 0;
		pdCpuChkBuff[devIndex].majStartTime = 0;
		pdCpuChkBuff[devIndex].criStartTime = 0;

		cpuInfo->level = SFM_ALM_NORMAL;  // sfdb->sys에 update

		if (cpuInfo->cpuFlag.criFlag) {
			fimd_hdlPDCpuUsageAlm (devIndex, SFM_ALM_NORMAL, SFM_ALM_CRITICAL, 0); // critical clear
			cpuInfo->cpuFlag.criFlag = 0;
			changeFlag = 1;
		}
		if (cpuInfo->cpuFlag.majFlag) {
			fimd_hdlPDCpuUsageAlm (devIndex, SFM_ALM_NORMAL, SFM_ALM_MAJOR, 0); // major clear
			cpuInfo->cpuFlag.majFlag = 0;
			changeFlag = 1;
		}
		if (cpuInfo->cpuFlag.minFlag) {
			fimd_hdlPDCpuUsageAlm (devIndex, SFM_ALM_NORMAL, SFM_ALM_MINOR, 0); // minor clear
			cpuInfo->cpuFlag.minFlag = 0;
			changeFlag = 1;
		}

		cpuInfo->cpuFlag.criFlag = 0;
		cpuInfo->cpuFlag.majFlag = 0;
		cpuInfo->cpuFlag.minFlag = 0;

	} //-- end of normal --//

	// minor보다 높고 major 기준값보다 낮은 경우, (minor 장애 조건)
	// - 더 높은 등급(major, critical)이 발생되어 있었으면, 등급 하강 조건이므로,
	//	각각 해지 메시지를 만든다.
	// - minor가 이미 발생되어 있으면 no action.
	// - minor 기준값 지속시간 경과여부를 확인하여 장애 판단.
	//
	else if (usage < cpuInfo->majLimit)
	{
		pdCpuChkBuff[devIndex].majStartTime = 0;
		pdCpuChkBuff[devIndex].criStartTime = 0;

		if (cpuInfo->cpuFlag.criFlag) {
			fimd_hdlPDCpuUsageAlm (devIndex, SFM_ALM_MINOR, SFM_ALM_CRITICAL, 0); // critical clear
			cpuInfo->cpuFlag.criFlag = 0;
			changeFlag = 1;
		}
		if (cpuInfo->cpuFlag.majFlag) {
			fimd_hdlPDCpuUsageAlm (devIndex, SFM_ALM_MINOR, SFM_ALM_MAJOR, 0); // major clear
			cpuInfo->cpuFlag.majFlag = 0;
			changeFlag = 1;
		}

		if (cpuInfo->cpuFlag.minFlag) {
			; // 이미 minor가 발생되어 있으면 no action.
		} else {

			if (pdCpuChkBuff[devIndex].minStartTime == 0) // 처음 limit를 넘었는가?
				pdCpuChkBuff[devIndex].minStartTime = currentTime;

			if (cpuInfo->minDurat > (currentTime - pdCpuChkBuff[devIndex].minStartTime)) {
				; // 지속시간 경과하지 않았으면 no action.
			} else {
				cpuInfo->level = SFM_ALM_MINOR;  // sfdb->sys에 update
				fimd_hdlPDCpuUsageAlm (devIndex, SFM_ALM_MINOR, 0, 1); // minor occur
				cpuInfo->cpuFlag.criFlag = 0;
				cpuInfo->cpuFlag.majFlag = 0;
				cpuInfo->cpuFlag.minFlag = 1;
				changeFlag = 1;
			}
		}

	} //-- end of major condition --//

	// major보다 높고 critical 기준값보다 낮은 경우, (major 장애 조건)
	// - 더 높은 등급(critical)이 발생되어 있었으면, 등급 하강 조건이므로 해지 메시지를 만든다.
	// - major가 이미 발생되어 있으면 no action.
	// - major 기준값 지속시간 경과여부를 확인하여 장애 판단.
	//
	else if (usage < cpuInfo->criLimit)
	{
		pdCpuChkBuff[devIndex].criStartTime = 0;

		if (cpuInfo->cpuFlag.minFlag) {
			fimd_hdlPDCpuUsageAlm (devIndex, SFM_ALM_MAJOR, SFM_ALM_MINOR, 0); // critical clear
			cpuInfo->cpuFlag.minFlag = 0;
			changeFlag = 1;
		}

		if (cpuInfo->cpuFlag.criFlag) {
			fimd_hdlPDCpuUsageAlm (devIndex, SFM_ALM_MAJOR, SFM_ALM_CRITICAL, 0); // critical clear
			cpuInfo->cpuFlag.criFlag = 0;
			changeFlag = 1;
		}

		if (cpuInfo->cpuFlag.majFlag) {
			; // 이미 major가 발생되어 있으면 no action.
		} else {
			// minor에서 상승하는게 아니라 한방에 major로 올라갈 수 있으므로
			//	minStartTime을 같이 변경해야한다.
			if (pdCpuChkBuff[devIndex].minStartTime == 0) // 처음 limit를 넘었는가?
				pdCpuChkBuff[devIndex].minStartTime = currentTime;
			if (pdCpuChkBuff[devIndex].majStartTime == 0) // 처음 limit를 넘었는가?
				pdCpuChkBuff[devIndex].majStartTime = currentTime;

			if (cpuInfo->majDurat > (currentTime - pdCpuChkBuff[devIndex].majStartTime)) {
				; // 지속시간 경과하지 않았으면 no action.
			} else {
				cpuInfo->level = SFM_ALM_MAJOR;  // sfdb->sys에 update
				fimd_hdlPDCpuUsageAlm (devIndex, SFM_ALM_MAJOR, 0, 1); // major occur
				cpuInfo->cpuFlag.criFlag = 0;
				cpuInfo->cpuFlag.majFlag = 1;
				cpuInfo->cpuFlag.minFlag = 0;
				changeFlag = 1;
			}
		}

	} //-- end of major condition --//

	// critical 기준값보다 높은 경우, (critical 장애 조건)
	// - critical이 이미 발생되어 있으면 no action.
	// - critical 기준값 지속시간 경과여부를 확인하여 장애 판단.
	//
	else
	{
		if (cpuInfo->cpuFlag.criFlag) {
			; // 이미 발생되어 있으면 no action.
		} else {
			// minor,major에서 상승하는게 아니라 한방에 critical로 올라갈 수 있으므로
			//	minStartTime, majorStartTime을 같이 변경해야한다.
			if (pdCpuChkBuff[devIndex].minStartTime == 0) // 처음 limit를 넘었는가?
				pdCpuChkBuff[devIndex].minStartTime = currentTime;
			if (pdCpuChkBuff[devIndex].majStartTime == 0) // 처음 limit를 넘었는가?
				pdCpuChkBuff[devIndex].majStartTime = currentTime;
			if (pdCpuChkBuff[devIndex].criStartTime == 0) // 처음 limit를 넘었는가?
				pdCpuChkBuff[devIndex].criStartTime = currentTime;

			if (cpuInfo->criDurat > (currentTime - pdCpuChkBuff[devIndex].criStartTime)) {
				; // 지속시간 경과하지 않았으면 no action.
			} else {
				cpuInfo->level = SFM_ALM_CRITICAL;  // sfdb->sys에 update
				fimd_hdlPDCpuUsageAlm (devIndex, SFM_ALM_CRITICAL, 0, 1); // critical occur
				cpuInfo->cpuFlag.criFlag = 1;
				cpuInfo->cpuFlag.majFlag = 0;
				cpuInfo->cpuFlag.minFlag = 0;
				changeFlag = 1;
			}
		}

	} //-- end of critical condition --//


	return changeFlag;

} //----- End of fimd_checkPDCpuUsageAlm -----//


/* by sjjeon*/
//------------------------------------------------------------------------------
int fimd_checkL2swCpuUsageAlm (int devIndex, SFM_PDCpuInfo *cpuInfo)
{
	int		usage,	changeFlag=0; // 장애 발생 또는 해지인 경우 flag를 1로 setting된다.

	usage = cpuInfo->usage;
	
	// minor 장애 기준값보다 낮은 경우,
	// - 이전에 장애가 발생되어 있었으면 장애가 해지된 경우이므로,
	//	발생되었던 각각 해당 등급에 대한 해지 메시지를 만들어 cond로 보낸다.
	//
	if (usage < cpuInfo->minLimit)
	{
		l2swCpuChkBuff[devIndex].minStartTime = 0;
		l2swCpuChkBuff[devIndex].majStartTime = 0;
		l2swCpuChkBuff[devIndex].criStartTime = 0;

		cpuInfo->level = SFM_ALM_NORMAL;  // sfdb->sys에 update

		if (cpuInfo->cpuFlag.criFlag) {
			fimd_hdlL2swCpuUsageAlm(devIndex, SFM_ALM_NORMAL, SFM_ALM_CRITICAL, 0); // critical clear
			changeFlag = 1;
		}
		if (cpuInfo->cpuFlag.majFlag) {
			fimd_hdlL2swCpuUsageAlm(devIndex, SFM_ALM_NORMAL, SFM_ALM_MAJOR, 0); // major clear
			changeFlag = 1;
		}
		if (cpuInfo->cpuFlag.minFlag) {
			fimd_hdlL2swCpuUsageAlm(devIndex, SFM_ALM_NORMAL, SFM_ALM_MINOR, 0); // minor clear
			changeFlag = 1;
		}

		cpuInfo->cpuFlag.criFlag = 0;
		cpuInfo->cpuFlag.majFlag = 0;
		cpuInfo->cpuFlag.minFlag = 0;

	} //-- end of normal --//

	// minor보다 높고 major 기준값보다 낮은 경우, (minor 장애 조건)
	// - 더 높은 등급(major, critical)이 발생되어 있었으면, 등급 하강 조건이므로,
	//	각각 해지 메시지를 만든다.
	// - minor가 이미 발생되어 있으면 no action.
	// - minor 기준값 지속시간 경과여부를 확인하여 장애 판단.
	//
	else if (usage < cpuInfo->majLimit)
	{
		l2swCpuChkBuff[devIndex].majStartTime = 0;
		l2swCpuChkBuff[devIndex].criStartTime = 0;

		if (cpuInfo->cpuFlag.criFlag) {
			fimd_hdlL2swCpuUsageAlm(devIndex, SFM_ALM_MINOR, SFM_ALM_CRITICAL, 0); // critical clear
			cpuInfo->cpuFlag.criFlag = 0;
			changeFlag = 1;
		}
		if (cpuInfo->cpuFlag.majFlag) {
			fimd_hdlL2swCpuUsageAlm(devIndex, SFM_ALM_MINOR, SFM_ALM_MAJOR, 0); // major clear
			cpuInfo->cpuFlag.majFlag = 0;
			changeFlag = 1;
		}

		if (cpuInfo->cpuFlag.minFlag) {
			; // 이미 minor가 발생되어 있으면 no action.
		} else {
			if (l2swCpuChkBuff[devIndex].minStartTime == 0) // 처음 limit를 넘었는가?
				l2swCpuChkBuff[devIndex].minStartTime = currentTime;
			if (cpuInfo->minDurat > (currentTime - l2swCpuChkBuff[devIndex].minStartTime)) {
				; // 지속시간 경과하지 않았으면 no action.
			} else {
				cpuInfo->level = SFM_ALM_MINOR;  // sfdb->sys에 update
				fimd_hdlL2swCpuUsageAlm(devIndex, SFM_ALM_MINOR, 0, 1); // minor occur
				cpuInfo->cpuFlag.criFlag = 0;
				cpuInfo->cpuFlag.majFlag = 0;
				cpuInfo->cpuFlag.minFlag = 1;
				changeFlag = 1;
			}
		}

	} //-- end of minor condition --//

	// major보다 높고 critical 기준값보다 낮은 경우, (major 장애 조건)
	// - 더 높은 등급(critical)이 발생되어 있었으면, 등급 하강 조건이므로 해지 메시지를 만든다.
	// - major가 이미 발생되어 있으면 no action.
	// - major 기준값 지속시간 경과여부를 확인하여 장애 판단.
	//
	else if (usage < cpuInfo->criLimit)
	{
		l2swCpuChkBuff[devIndex].criStartTime = 0;

		if (cpuInfo->cpuFlag.minFlag) {
			fimd_hdlL2swCpuUsageAlm(devIndex, SFM_ALM_MAJOR, SFM_ALM_MINOR, 0); // minor clear
			cpuInfo->cpuFlag.minFlag = 0;
			changeFlag = 1;
		}

		if (cpuInfo->cpuFlag.criFlag) {
			fimd_hdlL2swCpuUsageAlm(devIndex, SFM_ALM_MAJOR, SFM_ALM_CRITICAL, 0); // critical clear
			cpuInfo->cpuFlag.criFlag = 0;
			changeFlag = 1;
		}

		if (cpuInfo->cpuFlag.majFlag) {
			; // 이미 major가 발생되어 있으면 no action.
		} else {
			// minor에서 상승하는게 아니라 한방에 major로 올라갈 수 있으므로
			//	minStartTime을 같이 변경해야한다.
			if (l2swCpuChkBuff[devIndex].minStartTime == 0) // 처음 limit를 넘었는가?
				l2swCpuChkBuff[devIndex].minStartTime = currentTime;
			if (l2swCpuChkBuff[devIndex].majStartTime == 0) // 처음 limit를 넘었는가?
				l2swCpuChkBuff[devIndex].majStartTime = currentTime;

			if (cpuInfo->majDurat > (currentTime - l2swCpuChkBuff[devIndex].majStartTime)) {
				; // 지속시간 경과하지 않았으면 no action.
			} else {
				cpuInfo->level = SFM_ALM_MAJOR;  // sfdb->sys에 update
				fimd_hdlL2swCpuUsageAlm(devIndex, SFM_ALM_MAJOR, 0, 1); // major occur
				cpuInfo->cpuFlag.criFlag = 0;
				cpuInfo->cpuFlag.majFlag = 1;
				cpuInfo->cpuFlag.minFlag = 0;
				changeFlag = 1;
			}
		}

	} //-- end of major condition --//

	// critical 기준값보다 높은 경우, (critical 장애 조건)
	// - critical이 이미 발생되어 있으면 no action.
	// - critical 기준값 지속시간 경과여부를 확인하여 장애 판단.
	//
	else
	{
		if (cpuInfo->cpuFlag.criFlag) {
			; // 이미 발생되어 있으면 no action.
		} else {
			// minor,major에서 상승하는게 아니라 한방에 critical로 올라갈 수 있으므로
			//	minStartTime, majorStartTime을 같이 변경해야한다.
			if (l2swCpuChkBuff[devIndex].minStartTime == 0) // 처음 limit를 넘었는가?
				l2swCpuChkBuff[devIndex].minStartTime = currentTime;
			if (l2swCpuChkBuff[devIndex].majStartTime == 0) // 처음 limit를 넘었는가?
				l2swCpuChkBuff[devIndex].majStartTime = currentTime;
			if (l2swCpuChkBuff[devIndex].criStartTime == 0) // 처음 limit를 넘었는가?
				l2swCpuChkBuff[devIndex].criStartTime = currentTime;

			if (cpuInfo->criDurat > (currentTime - l2swCpuChkBuff[devIndex].criStartTime)) {
				; // 지속시간 경과하지 않았으면 no action.
			} else {
				cpuInfo->level = SFM_ALM_CRITICAL;  // sfdb->sys에 update
				fimd_hdlL2swCpuUsageAlm(devIndex, SFM_ALM_CRITICAL, 0, 1); // critical occur
				cpuInfo->cpuFlag.criFlag = 1;
				cpuInfo->cpuFlag.majFlag = 0;
				cpuInfo->cpuFlag.minFlag = 0;
				changeFlag = 1;
			}
		}

	} //-- end of critical condition --//
	return changeFlag;

} //----- End of fimd_checkL2swCpuUsageAlm-----//


/* by helca */
int fimd_checkPDMemUsageAlm (int devIndex, SFM_PDMemInfo *memInfo)
{
	int	usage, changeFlag=0;

	usage = memInfo->usage;
	//if ((memInfo->usage % 10) >= 5)
	//	usage++;

	// minor 장애 기준값보다 낮은 경우,
	// - 이전에 장애가 발생되어 있었으면 장애가 해지된 경우이므로,
	//	발생되었던 각각 해당 등급에 대한 해지 메시지를 만들어 cond로 보낸다.
	//
	if (usage < l3pd->l3ProbeDev[devIndex].memInfo.minLimit)
	{
		pdMemChkBuff[devIndex].minStartTime = 0;
		pdMemChkBuff[devIndex].majStartTime = 0;
		pdMemChkBuff[devIndex].criStartTime = 0;

		l3pd->l3ProbeDev[devIndex].memInfo.level = SFM_ALM_NORMAL;  // sfdb->sys에 update
		
		if (l3pd->l3ProbeDev[devIndex].memInfo.memFlag.criFlag) {
			fimd_hdlPDMemUsageAlm (devIndex, SFM_ALM_NORMAL, SFM_ALM_CRITICAL, 0); // critical clear
			changeFlag = 1;
		}
		if (l3pd->l3ProbeDev[devIndex].memInfo.memFlag.majFlag) {
			fimd_hdlPDMemUsageAlm (devIndex, SFM_ALM_NORMAL, SFM_ALM_MAJOR, 0); // major clear
			changeFlag = 1;
		}
		if (l3pd->l3ProbeDev[devIndex].memInfo.memFlag.minFlag) {
			fimd_hdlPDMemUsageAlm (devIndex, SFM_ALM_NORMAL, SFM_ALM_MINOR, 0); // minor clear
			changeFlag = 1;
		}

		l3pd->l3ProbeDev[devIndex].memInfo.memFlag.criFlag = 0;
		l3pd->l3ProbeDev[devIndex].memInfo.memFlag.majFlag = 0;
		l3pd->l3ProbeDev[devIndex].memInfo.memFlag.minFlag = 0;

	} //-- end of normal --//

	// minor보다 높고 major 기준값보다 낮은 경우, (minor 장애 조건)
	// - 더 높은 등급(major, critical)이 발생되어 있었으면, 등급 하강 조건이므로,
	//	각각 해지 메시지를 만든다.
	// - minor가 이미 발생되어 있으면 no action.
	// - minor 기준값 지속시간 경과여부를 확인하여 장애 판단.
	//
	else if (usage < l3pd->l3ProbeDev[devIndex].memInfo.majLimit)
	{
		pdMemChkBuff[devIndex].majStartTime = 0;
		pdMemChkBuff[devIndex].criStartTime = 0;

		if (l3pd->l3ProbeDev[devIndex].memInfo.memFlag.criFlag) {
			fimd_hdlPDMemUsageAlm (devIndex, SFM_ALM_MINOR, SFM_ALM_CRITICAL, 0); // critical clear
			l3pd->l3ProbeDev[devIndex].memInfo.memFlag.criFlag = 0;
			changeFlag = 1;
		}
		if (l3pd->l3ProbeDev[devIndex].memInfo.memFlag.majFlag) {
			fimd_hdlPDMemUsageAlm (devIndex, SFM_ALM_MINOR, SFM_ALM_MAJOR, 0); // major clear
			l3pd->l3ProbeDev[devIndex].memInfo.memFlag.majFlag = 0;
			changeFlag = 1;
		}

		if (l3pd->l3ProbeDev[devIndex].memInfo.memFlag.minFlag) {
			; // 이미 minor가 발생되어 있으면 no action.
		} else {
			if (pdMemChkBuff[devIndex].minStartTime == 0) // 처음 limit를 넘었는가?
				pdMemChkBuff[devIndex].minStartTime = currentTime;
			if (l3pd->l3ProbeDev[devIndex].memInfo.minDurat > (currentTime - pdMemChkBuff[devIndex].minStartTime)) {
				; // 지속시간 경과하지 않았으면 no action.
			}else{
				l3pd->l3ProbeDev[devIndex].memInfo.level = SFM_ALM_MINOR;  // sfdb->sys에 update
				fimd_hdlPDMemUsageAlm (devIndex, SFM_ALM_MINOR, 0, 1); // minor occur
				l3pd->l3ProbeDev[devIndex].memInfo.memFlag.criFlag = 0;
				l3pd->l3ProbeDev[devIndex].memInfo.memFlag.majFlag = 0;
				l3pd->l3ProbeDev[devIndex].memInfo.memFlag.minFlag = 1;
				changeFlag = 1;
			}
		}

	} //-- end of minor condition --//

	// major보다 높고 critical 기준값보다 낮은 경우, (major 장애 조건)
	// - 더 높은 등급(critical)이 발생되어 있었으면, 등급 하강 조건이므로 해지 메시지를 만든다.
	// - major가 이미 발생되어 있으면 no action.
	// - major 기준값 지속시간 경과여부를 확인하여 장애 판단.
	//
	else if (usage < l3pd->l3ProbeDev[devIndex].memInfo.criLimit)
	{
		pdMemChkBuff[devIndex].criStartTime = 0;

		if (l3pd->l3ProbeDev[devIndex].memInfo.memFlag.minFlag) {
			fimd_hdlPDMemUsageAlm (devIndex, SFM_ALM_MAJOR, SFM_ALM_MINOR, 0); // critical clear
			l3pd->l3ProbeDev[devIndex].memInfo.memFlag.minFlag = 0;
			changeFlag = 1;
		}
		if (l3pd->l3ProbeDev[devIndex].memInfo.memFlag.criFlag) {
			fimd_hdlPDMemUsageAlm (devIndex, SFM_ALM_MAJOR, SFM_ALM_CRITICAL, 0); // critical clear
			l3pd->l3ProbeDev[devIndex].memInfo.memFlag.criFlag = 0;
			changeFlag = 1;
		}

		if (l3pd->l3ProbeDev[devIndex].memInfo.memFlag.majFlag) {
			; // 이미 major가 발생되어 있으면 no action.
		} else {
			// minor에서 상승하는게 아니라 한방에 major로 올라갈 수 있으므로
			//	minStartTime을 같이 변경해야한다.
			if (pdMemChkBuff[devIndex].minStartTime == 0) // 처음 limit를 넘었는가?
				pdMemChkBuff[devIndex].minStartTime = currentTime;
			if (pdMemChkBuff[devIndex].majStartTime == 0) // 처음 limit를 넘었는가?
				pdMemChkBuff[devIndex].majStartTime = currentTime;

			if (l3pd->l3ProbeDev[devIndex].memInfo.majDurat > (currentTime - pdMemChkBuff[devIndex].majStartTime)) {
				; // 지속시간 경과하지 않았으면 no action.
			} else {
				l3pd->l3ProbeDev[devIndex].memInfo.level = SFM_ALM_MAJOR;  // sfdb->sys에 update
				fimd_hdlPDMemUsageAlm (devIndex, SFM_ALM_MAJOR, 0, 1); // major occur
				l3pd->l3ProbeDev[devIndex].memInfo.memFlag.criFlag = 0;
				l3pd->l3ProbeDev[devIndex].memInfo.memFlag.majFlag = 1;
				l3pd->l3ProbeDev[devIndex].memInfo.memFlag.minFlag = 0;
				changeFlag = 1;
			}
		}

	} //-- end of major condition --//

	// critical 기준값보다 높은 경우, (critical 장애 조건)
	// - critical이 이미 발생되어 있으면 no action.
	// - critical 기준값 지속시간 경과여부를 확인하여 장애 판단.
	//
	else
	{
		if (l3pd->l3ProbeDev[devIndex].memInfo.memFlag.criFlag) {
			; // 이미 발생되어 있으면 no action.
		} else {
			// minor,major에서 상승하는게 아니라 한방에 critical로 올라갈 수 있으므로
			//	minStartTime, majorStartTime을 같이 변경해야한다.
			if (pdMemChkBuff[devIndex].minStartTime == 0) // 처음 limit를 넘었는가?
				pdMemChkBuff[devIndex].minStartTime = currentTime;
			if (pdMemChkBuff[devIndex].majStartTime == 0) // 처음 limit를 넘었는가?
				pdMemChkBuff[devIndex].majStartTime = currentTime;
			if (pdMemChkBuff[devIndex].criStartTime == 0) // 처음 limit를 넘었는가?
				pdMemChkBuff[devIndex].criStartTime = currentTime;

			if (l3pd->l3ProbeDev[devIndex].memInfo.criDurat > (currentTime - pdMemChkBuff[devIndex].criStartTime)) {
				; // 지속시간 경과하지 않았으면 no action.
			}else{
				l3pd->l3ProbeDev[devIndex].memInfo.level = SFM_ALM_CRITICAL;  // sfdb->sys에 update
				fimd_hdlPDMemUsageAlm (devIndex, SFM_ALM_CRITICAL, 0, 1); // critical occur
				l3pd->l3ProbeDev[devIndex].memInfo.memFlag.criFlag = 1;
				l3pd->l3ProbeDev[devIndex].memInfo.memFlag.majFlag = 0;
				l3pd->l3ProbeDev[devIndex].memInfo.memFlag.minFlag = 0;
				changeFlag = 1;
			}
		}

	} //-- end of critical condition --//


	return changeFlag;

} //----- End of fimd_checkPDMemUsageAlm -----//

/* by sjjeon*/
int fimd_checkL2swMemUsageAlm (int devIndex, SFM_PDMemInfo *memInfo)
{
	int	usage, changeFlag=0;

	usage = memInfo->usage;

	// minor 장애 기준값보다 낮은 경우,
	// - 이전에 장애가 발생되어 있었으면 장애가 해지된 경우이므로,
	//	발생되었던 각각 해당 등급에 대한 해지 메시지를 만들어 cond로 보낸다.
	//
	if (usage < memInfo->minLimit)
	{
		l2swMemChkBuff[devIndex].minStartTime = 0;
		l2swMemChkBuff[devIndex].majStartTime = 0;
		l2swMemChkBuff[devIndex].criStartTime = 0;

		memInfo->level = SFM_ALM_NORMAL;  // sfdb->sys에 update
		
		if (memInfo->memFlag.criFlag) {
			fimd_hdlL2swMemUsageAlm(devIndex, SFM_ALM_NORMAL, SFM_ALM_CRITICAL, 0); // critical clear
			changeFlag = 1;
		}
		if (memInfo->memFlag.majFlag) {
			fimd_hdlL2swMemUsageAlm(devIndex, SFM_ALM_NORMAL, SFM_ALM_MAJOR, 0); // major clear
			changeFlag = 1;
		}
		if (memInfo->memFlag.minFlag) {
			fimd_hdlL2swMemUsageAlm(devIndex, SFM_ALM_NORMAL, SFM_ALM_MINOR, 0); // minor clear
			changeFlag = 1;
		}

		memInfo->memFlag.criFlag = 0;
		memInfo->memFlag.majFlag = 0;
		memInfo->memFlag.minFlag = 0;

	} //-- end of normal --//

	// minor보다 높고 major 기준값보다 낮은 경우, (minor 장애 조건)
	// - 더 높은 등급(major, critical)이 발생되어 있었으면, 등급 하강 조건이므로,
	//	각각 해지 메시지를 만든다.
	// - minor가 이미 발생되어 있으면 no action.
	// - minor 기준값 지속시간 경과여부를 확인하여 장애 판단.
	//
	else if (usage < memInfo->majLimit)
	{
		l2swMemChkBuff[devIndex].majStartTime = 0;
		l2swMemChkBuff[devIndex].criStartTime = 0;

		if (memInfo->memFlag.criFlag) {
			fimd_hdlL2swMemUsageAlm(devIndex, SFM_ALM_MINOR, SFM_ALM_CRITICAL, 0); // critical clear
			memInfo->memFlag.criFlag = 0;
			changeFlag = 1;
		}
		if (memInfo->memFlag.majFlag) {
			fimd_hdlL2swMemUsageAlm(devIndex, SFM_ALM_MINOR, SFM_ALM_MAJOR, 0); // major clear
			memInfo->memFlag.majFlag = 0;
			changeFlag = 1;
		}

		if (memInfo->memFlag.minFlag) {
			; // 이미 minor가 발생되어 있으면 no action.
		} else {
			if (l2swMemChkBuff[devIndex].minStartTime == 0) // 처음 limit를 넘었는가?
				l2swMemChkBuff[devIndex].minStartTime = currentTime;
			if (memInfo->minDurat > (currentTime - l2swMemChkBuff[devIndex].minStartTime)) {
				; // 지속시간 경과하지 않았으면 no action.
			}else{
				memInfo->level = SFM_ALM_MINOR;  // sfdb->sys에 update
				fimd_hdlL2swMemUsageAlm(devIndex, SFM_ALM_MINOR, 0, 1); // minor occur
				memInfo->memFlag.criFlag = 0;
				memInfo->memFlag.majFlag = 0;
				memInfo->memFlag.minFlag = 1;
				changeFlag = 1;
			}
		}

	} //-- end of major condition --//

	// major보다 높고 critical 기준값보다 낮은 경우, (major 장애 조건)
	// - 더 높은 등급(critical)이 발생되어 있었으면, 등급 하강 조건이므로 해지 메시지를 만든다.
	// - major가 이미 발생되어 있으면 no action.
	// - major 기준값 지속시간 경과여부를 확인하여 장애 판단.
	//
	else if (usage < memInfo->criLimit)
	{
		l2swMemChkBuff[devIndex].criStartTime = 0;

		if (memInfo->memFlag.minFlag) {
			fimd_hdlL2swMemUsageAlm(devIndex, SFM_ALM_MAJOR, SFM_ALM_MINOR, 0); // critical clear
			memInfo->memFlag.minFlag = 0;
			changeFlag = 1;
		}
		if (memInfo->memFlag.criFlag) {
			fimd_hdlL2swMemUsageAlm(devIndex, SFM_ALM_MAJOR, SFM_ALM_CRITICAL, 0); // critical clear
			memInfo->memFlag.criFlag = 0;
			changeFlag = 1;
		}

		if (memInfo->memFlag.majFlag) {
			; // 이미 major가 발생되어 있으면 no action.
		} else {
			// minor에서 상승하는게 아니라 한방에 major로 올라갈 수 있으므로
			//	minStartTime을 같이 변경해야한다.
			if (l2swMemChkBuff[devIndex].minStartTime == 0) // 처음 limit를 넘었는가?
				l2swMemChkBuff[devIndex].minStartTime = currentTime;
			if (l2swMemChkBuff[devIndex].majStartTime == 0) // 처음 limit를 넘었는가?
				l2swMemChkBuff[devIndex].majStartTime = currentTime;

			if (memInfo->majDurat > (currentTime - l2swMemChkBuff[devIndex].majStartTime)) {
				; // 지속시간 경과하지 않았으면 no action.
			} else {
				memInfo->level = SFM_ALM_MAJOR;  // sfdb->sys에 update
				fimd_hdlL2swMemUsageAlm(devIndex, SFM_ALM_MAJOR, 0, 1); // major occur
				memInfo->memFlag.criFlag = 0;
				memInfo->memFlag.majFlag = 1;
				memInfo->memFlag.minFlag = 0;
				changeFlag = 1;
			}
		}

	} //-- end of major condition --//

	// critical 기준값보다 높은 경우, (critical 장애 조건)
	// - critical이 이미 발생되어 있으면 no action.
	// - critical 기준값 지속시간 경과여부를 확인하여 장애 판단.
	//
	else
	{
		if (memInfo->memFlag.criFlag) {
			; // 이미 발생되어 있으면 no action.
		} else {
			// minor,major에서 상승하는게 아니라 한방에 critical로 올라갈 수 있으므로
			//	minStartTime, majorStartTime을 같이 변경해야한다.
			if (l2swMemChkBuff[devIndex].minStartTime == 0) // 처음 limit를 넘었는가?
				l2swMemChkBuff[devIndex].minStartTime = currentTime;
			if (l2swMemChkBuff[devIndex].majStartTime == 0) // 처음 limit를 넘었는가?
				l2swMemChkBuff[devIndex].majStartTime = currentTime;
			if (l2swMemChkBuff[devIndex].criStartTime == 0) // 처음 limit를 넘었는가?
				l2swMemChkBuff[devIndex].criStartTime = currentTime;

			if (memInfo->criDurat > (currentTime - pdMemChkBuff[devIndex].criStartTime)) {
				; // 지속시간 경과하지 않았으면 no action.
			}else{
				memInfo->level = SFM_ALM_CRITICAL;  // sfdb->sys에 update
				fimd_hdlL2swMemUsageAlm(devIndex, SFM_ALM_CRITICAL, 0, 1); // critical occur
				memInfo->memFlag.criFlag = 1;
				memInfo->memFlag.majFlag = 0;
				memInfo->memFlag.minFlag = 0;
				changeFlag = 1;
			}
		}

	} //-- end of critical condition --//

	return changeFlag;
}
//----- End of fimd_checkL2swMemUsageAlm-----//

int fimd_hdlRsrcStsAlm (int sysIndex, int loadIndex, int usage)
{
	int		changeFlag=0; // 장애 발생 또는 해지인 경우 1로 setting된다.

	// minor 장애 기준값보다 낮은 경우,
	// - 이전에 장애가 발생되어 있었으면 장애가 해지된 경우이므로,
	//	발생되었던 각각 해당 등급에 대한 해지 메시지를 만들어 cond로 보낸다.
	//
	// by helca 08.09
	if(loadIndex == 0) {
		if ( usage > 500000) {
			sprintf(trcBuf,"[fimd_hdlRsrcStsAlm] rsrcIdx: %d - usage: %d \n", loadIndex, usage);
			trclib_writeLogErr (FL,trcBuf);
		}
		usage = (usage*100)/500000;
	}
	else if(loadIndex == 1) {
		if ( usage > 150000) {
			sprintf(trcBuf,"[fimd_hdlRsrcStsAlm] rsrcIdx: %d - usage: %d \n", loadIndex, usage);
			trclib_writeLogErr (FL,trcBuf);
		}	
		usage = (usage*100)/150000;
	}
	else if(loadIndex == 5) {
		if ( usage > 150000) {
			sprintf(trcBuf,"[fimd_hdlRsrcStsAlm] rsrcIdx: %d - usage: %d \n", loadIndex, usage);
			trclib_writeLogErr (FL,trcBuf);
		}
		usage = (usage*100)/150000;
	}
	else if(loadIndex == 6) {
		if ( usage > 5000) {
			sprintf(trcBuf,"[fimd_hdlRsrcStsAlm] rsrcIdx: %d - usage: %d \n", loadIndex, usage);
			trclib_writeLogErr (FL,trcBuf);
		}
		usage = (usage*100)/5000;
	}
	else if(loadIndex == 7) {
		if ( usage > 5000) {
			sprintf(trcBuf,"[fimd_hdlRsrcStsAlm] rsrcIdx: %d - usage: %d \n", loadIndex, usage);
			trclib_writeLogErr (FL,trcBuf);
		}	
		usage = (usage*100)/5000;
	}
	else if(loadIndex == 8) {
		if ( usage > 5000) {
			sprintf(trcBuf,"[fimd_hdlRsrcStsAlm] rsrcIdx: %d - usage: %d \n", loadIndex, usage);
			trclib_writeLogErr (FL,trcBuf);
		}	
		usage = (usage*100)/5000;
	}
	else if(loadIndex == 10) {
		if ( usage > 150000) {
			sprintf(trcBuf,"[fimd_hdlRsrcStsAlm] rsrcIdx: %d - usage: %d \n", loadIndex, usage);
			trclib_writeLogErr (FL,trcBuf);
		}
		usage = (usage*100)/150000;
	}
	else if(loadIndex == 11) {
		if ( usage > 40000) {
			sprintf(trcBuf,"[fimd_hdlRsrcStsAlm] rsrcIdx: %d - usage: %d \n", loadIndex, usage);
			trclib_writeLogErr (FL,trcBuf);
		}
		usage = (usage*100)/40000;
	}
	else if(loadIndex == 12) {
		if ( usage > 500000) {
			sprintf(trcBuf,"[fimd_hdlRsrcStsAlm] rsrcIdx: %d - usage: %d \n", loadIndex, usage);
			trclib_writeLogErr (FL,trcBuf);
		}
		usage = (usage*100)/500000;
	}
	else if(loadIndex == 13) {
		if ( usage > 150000) {
			sprintf(trcBuf,"[fimd_hdlRsrcStsAlm] rsrcIdx: %d - usage: %d \n", loadIndex, usage);
			trclib_writeLogErr (FL,trcBuf);
		}
		usage = (usage*100)/150000;
	}
	else if(loadIndex == 15) {
		if ( usage > 40000) {
			sprintf(trcBuf,"[fimd_hdlRsrcStsAlm] rsrcIdx: %d - usage: %d \n", loadIndex, usage);
			trclib_writeLogErr (FL,trcBuf);
		}	
		usage = (usage*100)/40000; 
	}
	else  usage = 0;
	
	if (usage < sfdb->sys[sysIndex].commInfo.rsrcSts[loadIndex].minLimit)
	{
		rsrcLoadChkBuff[sysIndex][loadIndex].minStartTime = 0;
		rsrcLoadChkBuff[sysIndex][loadIndex].majStartTime = 0;
		rsrcLoadChkBuff[sysIndex][loadIndex].criStartTime = 0;

		sfdb->sys[sysIndex].commInfo.rsrcSts[loadIndex].level = SFM_ALM_NORMAL;  // sfdb->sys에 update

		if (sfdb->sys[sysIndex].commInfo.rsrcSts[loadIndex].rsrcFlag.criFlag) {
			fimd_hdlRsrcLoadAlm (sysIndex, loadIndex, usage, SFM_ALM_NORMAL, SFM_ALM_CRITICAL, 0); // critical clear
			changeFlag = 1;
		}
		if (sfdb->sys[sysIndex].commInfo.rsrcSts[loadIndex].rsrcFlag.majFlag) {
			fimd_hdlRsrcLoadAlm (sysIndex, loadIndex, usage, SFM_ALM_NORMAL, SFM_ALM_MAJOR, 0); // major clear
			changeFlag = 1;
		}
		if (sfdb->sys[sysIndex].commInfo.rsrcSts[loadIndex].rsrcFlag.minFlag) {
			fimd_hdlRsrcLoadAlm (sysIndex, loadIndex, usage, SFM_ALM_NORMAL, SFM_ALM_MINOR, 0); // minor clear
			changeFlag = 1;
		}

		sfdb->sys[sysIndex].commInfo.rsrcSts[loadIndex].rsrcFlag.criFlag = 0;
		sfdb->sys[sysIndex].commInfo.rsrcSts[loadIndex].rsrcFlag.majFlag = 0;
		sfdb->sys[sysIndex].commInfo.rsrcSts[loadIndex].rsrcFlag.minFlag = 0;

	} //-- end of normal --//

	// minor보다 높고 major 기준값보다 낮은 경우, (minor 장애 조건)
	// - 더 높은 등급(major, critical)이 발생되어 있었으면, 등급 하강 조건이므로,
	//	각각 해지 메시지를 만든다.
	// - minor가 이미 발생되어 있으면 no action.
	// - minor 기준값 지속시간 경과여부를 확인하여 장애 판단.
	//
	else if (usage < sfdb->sys[sysIndex].commInfo.rsrcSts[loadIndex].majLimit)
	{
		rsrcLoadChkBuff[sysIndex][loadIndex].majStartTime = 0;
		rsrcLoadChkBuff[sysIndex][loadIndex].criStartTime = 0;

		if (sfdb->sys[sysIndex].commInfo.rsrcSts[loadIndex].rsrcFlag.criFlag) {
			sfdb->sys[sysIndex].commInfo.rsrcSts[loadIndex].level = SFM_ALM_MINOR;  // sfdb->sys에 update
			fimd_hdlRsrcLoadAlm (sysIndex, loadIndex, usage, SFM_ALM_MINOR, SFM_ALM_CRITICAL, 0); // critical clear
			sfdb->sys[sysIndex].commInfo.rsrcSts[loadIndex].rsrcFlag.criFlag = 0;	
			changeFlag = 1;
		}
		if (sfdb->sys[sysIndex].commInfo.rsrcSts[loadIndex].rsrcFlag.majFlag) {
			sfdb->sys[sysIndex].commInfo.rsrcSts[loadIndex].level = SFM_ALM_MINOR;  // sfdb->sys에 update
			fimd_hdlRsrcLoadAlm (sysIndex, loadIndex, usage, SFM_ALM_MINOR, SFM_ALM_MAJOR, 0); // major clear
			sfdb->sys[sysIndex].commInfo.rsrcSts[loadIndex].rsrcFlag.majFlag = 0;	
			changeFlag = 1;
		}

		if (sfdb->sys[sysIndex].commInfo.rsrcSts[loadIndex].rsrcFlag.minFlag) {
			; // 이미 minor가 발생되어 있으면 no action.
		} else {
			if (rsrcLoadChkBuff[sysIndex][loadIndex].minStartTime == 0) // 처음 limit를 넘었는가?
				rsrcLoadChkBuff[sysIndex][loadIndex].minStartTime = currentTime;
			if (sfdb->sys[sysIndex].commInfo.rsrcSts[loadIndex].minDurat > (currentTime - rsrcLoadChkBuff[sysIndex][loadIndex].minStartTime)) {
				; // 지속시간 경과하지 않았으면 no action.
			} else {
				sfdb->sys[sysIndex].commInfo.rsrcSts[loadIndex].level = SFM_ALM_MINOR;  // sfdb->sys에 update
				fimd_hdlRsrcLoadAlm (sysIndex, loadIndex, usage, SFM_ALM_MINOR, 0, 1); // minor occur
				sfdb->sys[sysIndex].commInfo.rsrcSts[loadIndex].rsrcFlag.criFlag = 0;
				sfdb->sys[sysIndex].commInfo.rsrcSts[loadIndex].rsrcFlag.majFlag = 0;
				sfdb->sys[sysIndex].commInfo.rsrcSts[loadIndex].rsrcFlag.minFlag = 1;
				changeFlag = 1;
			}
		}

	} //-- end of minor condition --//

	// major보다 높고 critical 기준값보다 낮은 경우, (major 장애 조건)
	// - 더 높은 등급(critical)이 발생되어 있었으면, 등급 하강 조건이므로 해지 메시지를 만든다.
	// - major가 이미 발생되어 있으면 no action.
	// - major 기준값 지속시간 경과여부를 확인하여 장애 판단.
	//
	else if (usage < sfdb->sys[sysIndex].commInfo.rsrcSts[loadIndex].criLimit)
	{
		rsrcLoadChkBuff[sysIndex][loadIndex].criStartTime = 0;

		if (sfdb->sys[sysIndex].commInfo.rsrcSts[loadIndex].rsrcFlag.criFlag) {
			sfdb->sys[sysIndex].commInfo.rsrcSts[loadIndex].level = SFM_ALM_MAJOR;  // sfdb->sys에 update
			fimd_hdlRsrcLoadAlm (sysIndex, loadIndex, usage, SFM_ALM_MAJOR, SFM_ALM_CRITICAL, 0); // critical clear
			sfdb->sys[sysIndex].commInfo.rsrcSts[loadIndex].rsrcFlag.criFlag = 0;	
			changeFlag = 1;
		}

		if (sfdb->sys[sysIndex].commInfo.rsrcSts[loadIndex].rsrcFlag.majFlag) {
			; // 이미 major가 발생되어 있으면 no action.
		} else {
			// minor에서 상승하는게 아니라 한방에 major로 올라갈 수 있으므로
			//	minStartTime을 같이 변경해야한다.
			if (rsrcLoadChkBuff[sysIndex][loadIndex].minStartTime == 0) // 처음 limit를 넘었는가?
				rsrcLoadChkBuff[sysIndex][loadIndex].minStartTime = currentTime;
			if (rsrcLoadChkBuff[sysIndex][loadIndex].majStartTime == 0) // 처음 limit를 넘었는가?
				rsrcLoadChkBuff[sysIndex][loadIndex].majStartTime = currentTime;

			if (sfdb->sys[sysIndex].commInfo.rsrcSts[loadIndex].majDurat > (currentTime - rsrcLoadChkBuff[sysIndex][loadIndex].majStartTime)) {
				; // 지속시간 경과하지 않았으면 no action.
			} else {
				sfdb->sys[sysIndex].commInfo.rsrcSts[loadIndex].level = SFM_ALM_MAJOR;  // sfdb->sys에 update
				fimd_hdlRsrcLoadAlm (sysIndex, loadIndex, usage, SFM_ALM_MAJOR, 0, 1); // major occur
				sfdb->sys[sysIndex].commInfo.rsrcSts[loadIndex].rsrcFlag.criFlag = 0;
				sfdb->sys[sysIndex].commInfo.rsrcSts[loadIndex].rsrcFlag.majFlag = 1;
				sfdb->sys[sysIndex].commInfo.rsrcSts[loadIndex].rsrcFlag.minFlag = 0;
				changeFlag = 1;
			}
		}

	} //-- end of major condition --//

	// critical 기준값보다 높은 경우, (critical 장애 조건)
	// - critical이 이미 발생되어 있으면 no action.
	// - critical 기준값 지속시간 경과여부를 확인하여 장애 판단.
	//
	else if (usage > sfdb->sys[sysIndex].commInfo.rsrcSts[loadIndex].criLimit)
	{
		if (sfdb->sys[sysIndex].commInfo.rsrcSts[loadIndex].rsrcFlag.criFlag) {
			; // 이미 발생되어 있으면 no action.
		} else {
			// minor,major에서 상승하는게 아니라 한방에 critical로 올라갈 수 있으므로
			//	minStartTime, majorStartTime을 같이 변경해야한다.
			if (rsrcLoadChkBuff[sysIndex][loadIndex].minStartTime == 0) // 처음 limit를 넘었는가?
				rsrcLoadChkBuff[sysIndex][loadIndex].minStartTime = currentTime;
			if (rsrcLoadChkBuff[sysIndex][loadIndex].majStartTime == 0) // 처음 limit를 넘었는가?
				rsrcLoadChkBuff[sysIndex][loadIndex].majStartTime = currentTime;
			if (rsrcLoadChkBuff[sysIndex][loadIndex].criStartTime == 0) // 처음 limit를 넘었는가?
				rsrcLoadChkBuff[sysIndex][loadIndex].criStartTime = currentTime;

			if (sfdb->sys[sysIndex].commInfo.rsrcSts[loadIndex].criDurat > (currentTime - rsrcLoadChkBuff[sysIndex][loadIndex].criStartTime)) {
				; // 지속시간 경과하지 않았으면 no action.
			} else {
				sfdb->sys[sysIndex].commInfo.rsrcSts[loadIndex].level = SFM_ALM_CRITICAL;  // sfdb->sys에 update
				fimd_hdlRsrcLoadAlm (sysIndex, loadIndex, usage, SFM_ALM_CRITICAL, 0, 1); // critical occur
				sfdb->sys[sysIndex].commInfo.rsrcSts[loadIndex].rsrcFlag.criFlag = 1;
				sfdb->sys[sysIndex].commInfo.rsrcSts[loadIndex].rsrcFlag.majFlag = 0;
				sfdb->sys[sysIndex].commInfo.rsrcSts[loadIndex].rsrcFlag.minFlag = 0;
				changeFlag = 1;
			}
		}

	} //-- end of critical condition --//


	return changeFlag;
	//----- End of fimd_hdlRsrcStsAlm -----//
} 
// by helca 08.07 
//
#if 1 
#define QueDurat 3
//------------------------------------------------------------------------------
int fimd_checkQueueLoadAlm (int sysIndex, int queLoadIndex, SFM_QueInfo *queInfo)
{
	int		usage;
	int		changeFlag=0; // 장애 발생 또는 해지인 경우 1로 setting된다.


	// minor 장애 기준값보다 낮은 경우,
	// - 이전에 장애가 발생되어 있었으면 장애가 해지된 경우이므로,
	//	발생되었던 각각 해당 등급에 대한 해지 메시지를 만들어 cond로 보낸다.
	//
#if 0
// TEST sjjeon
	queInfo->load = 50;
#endif
	usage = queInfo->load; // by helca

	if (usage < queInfo->minLimit)
	{
		queLoadChkBuff[sysIndex][queLoadIndex].minStartTime = 0;
		queLoadChkBuff[sysIndex][queLoadIndex].majStartTime = 0;
		queLoadChkBuff[sysIndex][queLoadIndex].criStartTime = 0;

		queInfo->level = SFM_ALM_NORMAL;  // sfdb->sys에 update

		if (sfdb->hwAlmFlag[sysIndex].queLoad[queLoadIndex].criFlag) {
			fimd_hdlQueLoadAlm (sysIndex, queLoadIndex, SFM_ALM_NORMAL, SFM_ALM_CRITICAL, 0); // critical clear
			changeFlag = 1;
		}
		if (sfdb->hwAlmFlag[sysIndex].queLoad[queLoadIndex].majFlag) {
			fimd_hdlQueLoadAlm (sysIndex, queLoadIndex, SFM_ALM_NORMAL, SFM_ALM_MAJOR, 0); // major clear
			changeFlag = 1;
		}
		if (sfdb->hwAlmFlag[sysIndex].queLoad[queLoadIndex].minFlag) {
			fimd_hdlQueLoadAlm (sysIndex, queLoadIndex, SFM_ALM_NORMAL, SFM_ALM_MINOR, 0); // minor clear
			changeFlag = 1;
		}

		sfdb->hwAlmFlag[sysIndex].queLoad[queLoadIndex].criFlag = 0;
		sfdb->hwAlmFlag[sysIndex].queLoad[queLoadIndex].majFlag = 0;
		sfdb->hwAlmFlag[sysIndex].queLoad[queLoadIndex].minFlag = 0;

	} //-- end of normal --//

	// minor보다 높고 major 기준값보다 낮은 경우, (minor 장애 조건)
	// - 더 높은 등급(major, critical)이 발생되어 있었으면, 등급 하강 조건이므로,
	//	각각 해지 메시지를 만든다.
	// - minor가 이미 발생되어 있으면 no action.
	// - minor 기준값 지속시간 경과여부를 확인하여 장애 판단.
	//
	else if (usage < queInfo->majLimit)
	{
		queLoadChkBuff[sysIndex][queLoadIndex].majStartTime = 0;
		queLoadChkBuff[sysIndex][queLoadIndex].criStartTime = 0;

		if (sfdb->hwAlmFlag[sysIndex].queLoad[queLoadIndex].criFlag) {
		//	queInfo->level = SFM_ALM_MINOR;  // sfdb->sys에 update
			fimd_hdlQueLoadAlm (sysIndex, queLoadIndex, SFM_ALM_MINOR, SFM_ALM_CRITICAL, 0); // critical clear
			sfdb->hwAlmFlag[sysIndex].queLoad[queLoadIndex].criFlag = 0;	
			changeFlag = 1;
		}
		if (sfdb->hwAlmFlag[sysIndex].queLoad[queLoadIndex].majFlag) {
		//	queInfo->level = SFM_ALM_MINOR;  // sfdb->sys에 update
			fimd_hdlQueLoadAlm (sysIndex, queLoadIndex, SFM_ALM_MINOR, SFM_ALM_MAJOR, 0); // major clear
			sfdb->hwAlmFlag[sysIndex].queLoad[queLoadIndex].majFlag = 0;	
			changeFlag = 1;
		}

		if (sfdb->hwAlmFlag[sysIndex].queLoad[queLoadIndex].minFlag) {
			; // 이미 minor가 발생되어 있으면 no action.
		} else {
			if (queLoadChkBuff[sysIndex][queLoadIndex].minStartTime == 0) // 처음 limit를 넘었는가?
				queLoadChkBuff[sysIndex][queLoadIndex].minStartTime = currentTime;
			if (QueDurat > (currentTime - queLoadChkBuff[sysIndex][queLoadIndex].minStartTime)){
				; // 지속시간 경과하지 않았으면 no action.
			}else {
				queInfo->level = SFM_ALM_MINOR;  // sfdb->sys에 update
				fimd_hdlQueLoadAlm (sysIndex, queLoadIndex, SFM_ALM_MINOR, 0, 1); // minor occur
				sfdb->hwAlmFlag[sysIndex].queLoad[queLoadIndex].criFlag = 0;
				sfdb->hwAlmFlag[sysIndex].queLoad[queLoadIndex].majFlag = 0;
				sfdb->hwAlmFlag[sysIndex].queLoad[queLoadIndex].minFlag = 1;
				changeFlag = 1;
			}
		}

	} //-- end of minor condition --//

	// major보다 높고 critical 기준값보다 낮은 경우, (major 장애 조건)
	// - 더 높은 등급(critical)이 발생되어 있었으면, 등급 하강 조건이므로 해지 메시지를 만든다.
	// - major가 이미 발생되어 있으면 no action.
	// - major 기준값 지속시간 경과여부를 확인하여 장애 판단.
	//
	else if (usage < queInfo->criLimit)
	{
		queLoadChkBuff[sysIndex][queLoadIndex].criStartTime = 0;

		if (sfdb->hwAlmFlag[sysIndex].queLoad[queLoadIndex].criFlag) {
		//	queInfo->level = SFM_ALM_MAJOR;  // sfdb->sys에 update
			fimd_hdlQueLoadAlm (sysIndex, queLoadIndex, SFM_ALM_MAJOR, SFM_ALM_CRITICAL, 0); // critical clear
			sfdb->hwAlmFlag[sysIndex].queLoad[queLoadIndex].criFlag = 0;	
			changeFlag = 1;
		}

		if (sfdb->hwAlmFlag[sysIndex].queLoad[queLoadIndex].majFlag) {
			; // 이미 major가 발생되어 있으면 no action.
		} else {
			// minor에서 상승하는게 아니라 한방에 major로 올라갈 수 있으므로
			//	minStartTime을 같이 변경해야한다.
			if (queLoadChkBuff[sysIndex][queLoadIndex].minStartTime == 0) // 처음 limit를 넘었는가?
				queLoadChkBuff[sysIndex][queLoadIndex].minStartTime = currentTime;
			if (queLoadChkBuff[sysIndex][queLoadIndex].majStartTime == 0) // 처음 limit를 넘었는가?
				queLoadChkBuff[sysIndex][queLoadIndex].majStartTime = currentTime;
			if (QueDurat > (currentTime - queLoadChkBuff[sysIndex][queLoadIndex].majStartTime)) {
				; // 지속시간 경과하지 않았으면 no action.	
			}else {
				queInfo->level = SFM_ALM_MAJOR;  // sfdb->sys에 update
				fimd_hdlQueLoadAlm (sysIndex, queLoadIndex, SFM_ALM_MAJOR, 0, 1); // major occur
				sfdb->hwAlmFlag[sysIndex].queLoad[queLoadIndex].criFlag = 0;
				sfdb->hwAlmFlag[sysIndex].queLoad[queLoadIndex].majFlag = 1;
				sfdb->hwAlmFlag[sysIndex].queLoad[queLoadIndex].minFlag = 0;
				changeFlag = 1;
			}
		}

	} //-- end of major condition --//

	// critical 기준값보다 높은 경우, (critical 장애 조건)
	// - critical이 이미 발생되어 있으면 no action.
	// - critical 기준값 지속시간 경과여부를 확인하여 장애 판단.
	//
	else
	{
		if (sfdb->hwAlmFlag[sysIndex].queLoad[queLoadIndex].criFlag) {
			; // 이미 발생되어 있으면 no action.
		} else {
			// minor,major에서 상승하는게 아니라 한방에 critical로 올라갈 수 있으므로
			//	minStartTime, majorStartTime을 같이 변경해야한다.
			if (queLoadChkBuff[sysIndex][queLoadIndex].minStartTime == 0) // 처음 limit를 넘었는가?
				queLoadChkBuff[sysIndex][queLoadIndex].minStartTime = currentTime;
			if (queLoadChkBuff[sysIndex][queLoadIndex].majStartTime == 0) // 처음 limit를 넘었는가?
				queLoadChkBuff[sysIndex][queLoadIndex].majStartTime = currentTime;
			if (queLoadChkBuff[sysIndex][queLoadIndex].criStartTime == 0) // 처음 limit를 넘었는가?
				queLoadChkBuff[sysIndex][queLoadIndex].criStartTime = currentTime;
			if (QueDurat > (currentTime - queLoadChkBuff[sysIndex][queLoadIndex].criStartTime)){
				; // 지속시간 경과하지 않았으면 no action.
			}else {
				queInfo->level = SFM_ALM_CRITICAL;  // sfdb->sys에 update
				fimd_hdlQueLoadAlm (sysIndex, queLoadIndex, SFM_ALM_CRITICAL, 0, 1); // critical occur
				sfdb->hwAlmFlag[sysIndex].queLoad[queLoadIndex].criFlag = 1;
				sfdb->hwAlmFlag[sysIndex].queLoad[queLoadIndex].majFlag = 0;
				sfdb->hwAlmFlag[sysIndex].queLoad[queLoadIndex].minFlag = 0;
				changeFlag = 1;
			}
		}

	} //-- end of critical condition --//


	return changeFlag;

} //----- End of fimd_checkQueueLoadAlm -----//
#endif

/* ADD : by june, 2010-0907
 * DISCRIPTION :
 *   	- alarm status 변경시 alarm clear message가 발생하지 않는 버그 수정.
 *   	- 기존 함수 주석 처리하고 copy 해서 재작성.
 * LINE : 1579 - 1702
 * TODO : 테스트 후 문제 없으면 기존 함수 제거.
 */
//------------------------------------------------------------------------------
int  fimd_thresholdCheckUsage(Threshold_st *pthresh, SCE_USAGE_PARAM *pParam)
{
	// minor 장애 기준값보다 낮은 경우,
	// - 이전에 장애가 발생되어 있었으면 장애가 해지된 경우이므로,
	//	발생되었던 각각 해당 등급에 대한 해지 메시지를 만들어 cond로 보낸다.
	// - Current Alarm Level 이상의 AlarmStartTime 변수는 초기화 한다.
	if (pParam->usage < pthresh->minLimit)
	{
		sceDiskChkBuff[pParam->sysIndex].minStartTime = 0;
		sceDiskChkBuff[pParam->sysIndex].majStartTime = 0;
		sceDiskChkBuff[pParam->sysIndex].criStartTime = 0;
		pParam->curStatus = SFM_ALM_NORMAL;

		if (pthresh->criFlag) {
			pParam->preStatus = SFM_ALM_CRITICAL;
			fimd_makeSceUsageAlmMsg (pParam);
			fimd_saveSceUsageAlmInfo2DB (pParam);
			pParam->changeFlag = 1;
			pParam->occurFlag = 0;
		}
		if (pthresh->majFlag) {
			pParam->preStatus = SFM_ALM_MAJOR;
			fimd_makeSceUsageAlmMsg (pParam);
			fimd_saveSceUsageAlmInfo2DB (pParam);
			pParam->changeFlag = 1;
			pParam->occurFlag = 0;
		}
		if (pthresh->minFlag) {
			pParam->preStatus = SFM_ALM_MINOR;
			fimd_makeSceUsageAlmMsg (pParam);
			fimd_saveSceUsageAlmInfo2DB (pParam);
			pParam->changeFlag = 1;
			pParam->occurFlag = 0;
		}
		return SFM_ALM_NORMAL;
	} //-- end of normal --//

	// minor보다 높고 major 기준값보다 낮은 경우, (minor 장애 조건)
	// - 더 높은 등급(major, critical)이 발생되어 있었으면, 등급 하강 조건이므로,
	//	각각 해지 메시지를 만든다.
	// - minor가 이미 발생되어 있으면 no action.
	// - minor 기준값 지속시간 경과여부를 확인하여 장애 판단.
	else if (pParam->usage < pthresh->majLimit)
	{
		sceDiskChkBuff[pParam->sysIndex].majStartTime = 0;
		sceDiskChkBuff[pParam->sysIndex].criStartTime = 0;
		pParam->curStatus = SFM_ALM_MINOR;

		if (pthresh->criFlag) {
			pParam->preStatus = SFM_ALM_CRITICAL;
			fimd_makeSceUsageAlmMsg (pParam);
			fimd_saveSceUsageAlmInfo2DB (pParam);
		}
		if (pthresh->majFlag) {
			pParam->preStatus = SFM_ALM_MAJOR;
			fimd_makeSceUsageAlmMsg (pParam);
			fimd_saveSceUsageAlmInfo2DB (pParam);
		}
		if (pthresh->minFlag) {
			pParam->preStatus = SFM_ALM_MINOR;
			pParam->changeFlag = 0;
			pParam->occurFlag = 0;
		}
		else {
			pParam->changeFlag = 1;
			pParam->occurFlag = 1;
		}

		return SFM_ALM_MINOR;
	} //-- end of minor condition --//

	// major보다 높고 critical 기준값보다 낮은 경우, (major 장애 조건)
	// - 더 높은 등급(critical)이 발생되어 있었으면, 등급 하강 조건이므로 해지 메시지를 만든다.
	// - major가 이미 발생되어 있으면 no action.
	// - major 기준값 지속시간 경과여부를 확인하여 장애 판단.
	else if (pParam->usage < pthresh->criLimit)
	{
		sceDiskChkBuff[pParam->sysIndex].criStartTime = 0;
		pParam->curStatus = SFM_ALM_MAJOR;

		if (pthresh->criFlag) {
			pParam->preStatus = SFM_ALM_CRITICAL;
			fimd_makeSceUsageAlmMsg (pParam);
			fimd_saveSceUsageAlmInfo2DB (pParam);
		}

		if (pthresh->majFlag) {
			pParam->preStatus = SFM_ALM_MAJOR;
			pParam->changeFlag = 0;
			pParam->occurFlag = 0;
		}
		else {
			pParam->changeFlag = 1;
			pParam->occurFlag = 1;
		}
		return SFM_ALM_MAJOR;
	} //-- end of major condition --//

	// critical 기준값보다 높은 경우, (critical 장애 조건)
	// - critical이 이미 발생되어 있으면 no action.
	// - critical 기준값 지속시간 경과여부를 확인하여 장애 판단.
	//
	else
	{
		pParam->curStatus = SFM_ALM_CRITICAL;
		if (pthresh->criFlag) {
			pParam->preStatus = SFM_ALM_CRITICAL; //추가 sjjeon
			pParam->changeFlag = 0;
			pParam->occurFlag = 0;
		}
		else {
			pParam->changeFlag = 1;
			pParam->occurFlag = 1;
		}

		return SFM_ALM_CRITICAL;
	} //-- end of critical condition --//
}

#if 0
int  fimd_thresholdCheckUsage(Threshold_st *pthresh, SCE_USAGE_PARAM *pParam)
{

//	if ((pParam->usage % 10) >=5)
//		pParam->usage++;

	// minor 장애 기준값보다 낮은 경우,
	// - 이전에 장애가 발생되어 있었으면 장애가 해지된 경우이므로,
	//	발생되었던 각각 해당 등급에 대한 해지 메시지를 만들어 cond로 보낸다.
	//
	if (pParam->usage < pthresh->minLimit)
	{
		pParam->curStatus = SFM_ALM_NORMAL;
		if (pthresh->criFlag) {
			pParam->preStatus = SFM_ALM_CRITICAL;
			pParam->changeFlag = 1;
			pParam->occurFlag = 0;
		}
		if (pthresh->majFlag) {
//		else if (pthresh->majFlag) {
			pParam->preStatus = SFM_ALM_MAJOR;
			pParam->changeFlag = 1;
			pParam->occurFlag = 0;
		}
		if (pthresh->minFlag) {
//		else if(pthresh->minFlag) {
			pParam->preStatus = SFM_ALM_MINOR;
			pParam->changeFlag = 1;
			pParam->occurFlag = 0;
		}

		return SFM_ALM_NORMAL;
	} //-- end of normal --//

	// minor보다 높고 major 기준값보다 낮은 경우, (minor 장애 조건)
	// - 더 높은 등급(major, critical)이 발생되어 있었으면, 등급 하강 조건이므로,
	//	각각 해지 메시지를 만든다.
	// - minor가 이미 발생되어 있으면 no action.
	// - minor 기준값 지속시간 경과여부를 확인하여 장애 판단.
	//
	else if (pParam->usage < pthresh->majLimit)
	{
		pParam->curStatus = SFM_ALM_MINOR;
		if (pthresh->criFlag) {
			pParam->preStatus = SFM_ALM_CRITICAL;
			pParam->changeFlag = 1;
			pParam->occurFlag = 0;
		}
		if (pthresh->majFlag) {
//		else if (pthresh->majFlag) {
			pParam->preStatus = SFM_ALM_MAJOR;
			pParam->changeFlag = 1;
			pParam->occurFlag = 0;
		}
		if (pthresh->minFlag) {
//		else if (pthresh->minFlag) {
			pParam->preStatus = SFM_ALM_MINOR;
			pParam->changeFlag = 0;
			pParam->occurFlag = 0;
		} else {
			pParam->preStatus = SFM_ALM_MINOR;
			pParam->changeFlag = 1;
			pParam->occurFlag = 1;
		}

		// 하나로 구분이 가능할찌는 고민.. 
		return SFM_ALM_MINOR;
	} //-- end of minor condition --//

	// major보다 높고 critical 기준값보다 낮은 경우, (major 장애 조건)
	// - 더 높은 등급(critical)이 발생되어 있었으면, 등급 하강 조건이므로 해지 메시지를 만든다.
	// - major가 이미 발생되어 있으면 no action.
	// - major 기준값 지속시간 경과여부를 확인하여 장애 판단.
	//
	else if (pParam->usage < pthresh->criLimit)
	{

		pParam->curStatus = SFM_ALM_MAJOR;
		if (pthresh->criFlag) {
			pParam->preStatus = SFM_ALM_CRITICAL;
			pParam->changeFlag = 1;
			pParam->occurFlag = 0;
		}
		if (pthresh->minFlag) {
//		else if (pthresh->minFlag) {
			pParam->preStatus = SFM_ALM_MINOR;
			pParam->changeFlag = 1;
			pParam->occurFlag = 0;
		}
		if (pthresh->majFlag) {
//		else if (pthresh->majFlag) {
			pParam->preStatus = SFM_ALM_MAJOR;
			pParam->changeFlag = 0;
			pParam->occurFlag = 0;
		} else {
			pParam->preStatus = SFM_ALM_MAJOR;
			pParam->changeFlag = 1;
			pParam->occurFlag = 1;
		}

		return SFM_ALM_MAJOR;
	} //-- end of major condition --//

	// critical 기준값보다 높은 경우, (critical 장애 조건)
	// - critical이 이미 발생되어 있으면 no action.
	// - critical 기준값 지속시간 경과여부를 확인하여 장애 판단.
	//
	else
	{
		pParam->curStatus = SFM_ALM_CRITICAL;
		if (pthresh->criFlag) {
			pParam->preStatus = SFM_ALM_CRITICAL; //추가 sjjeon
			pParam->changeFlag = 0;
			pParam->occurFlag = 0;
		} else {
			pParam->preStatus = SFM_ALM_CRITICAL;
			pParam->changeFlag = 1;
			pParam->occurFlag = 1;
		}

		return SFM_ALM_CRITICAL;

	} //-- end of critical condition --//
}
#endif

/* hjjung */
int  fimd_thresholdCheckSceUserUsage(Threshold_st_int *pthresh, SCE_USAGE_PARAM *pParam)
{
	// minor 장애 기준값보다 낮은 경우,
	// - 이전에 장애가 발생되어 있었으면 장애가 해지된 경우이므로,
	//	발생되었던 각각 해당 등급에 대한 해지 메시지를 만들어 cond로 보낸다.
	if (pParam->usage < pthresh->minLimit)
	{
		sceUserChkBuff[pParam->sysIndex].minStartTime = 0;
		sceUserChkBuff[pParam->sysIndex].majStartTime = 0;
		sceUserChkBuff[pParam->sysIndex].criStartTime = 0;
		pParam->curStatus = SFM_ALM_NORMAL;

		if (pthresh->criFlag) {
			pParam->preStatus = SFM_ALM_CRITICAL;
			fimd_makeSceUsageAlmMsg (pParam);
			fimd_saveSceUsageAlmInfo2DB (pParam);
			pParam->changeFlag = 1;
			pParam->occurFlag = 0;
		}
		if (pthresh->majFlag) {
			pParam->preStatus = SFM_ALM_MAJOR;
			fimd_makeSceUsageAlmMsg (pParam);
			fimd_saveSceUsageAlmInfo2DB (pParam);
			pParam->changeFlag = 1;
			pParam->occurFlag = 0;
		}
		if (pthresh->minFlag) {
			pParam->preStatus = SFM_ALM_MINOR;
			fimd_makeSceUsageAlmMsg (pParam);
			fimd_saveSceUsageAlmInfo2DB (pParam);
			pParam->changeFlag = 1;
			pParam->occurFlag = 0;
		}
		return SFM_ALM_NORMAL;
	} //-- end of normal --//

	// minor보다 높고 major 기준값보다 낮은 경우, (minor 장애 조건)
	// - 더 높은 등급(major, critical)이 발생되어 있었으면, 등급 하강 조건이므로,
	//	각각 해지 메시지를 만든다.
	// - minor가 이미 발생되어 있으면 no action.
	// - minor 기준값 지속시간 경과여부를 확인하여 장애 판단.
	//
	else if (pParam->usage < pthresh->majLimit)
	{
		sceUserChkBuff[pParam->sysIndex].majStartTime = 0;
		sceUserChkBuff[pParam->sysIndex].criStartTime = 0;
		pParam->curStatus = SFM_ALM_MINOR;

		if (pthresh->criFlag) {
			pParam->preStatus = SFM_ALM_CRITICAL;
			fimd_makeSceUsageAlmMsg (pParam);
			fimd_saveSceUsageAlmInfo2DB (pParam);
		}
		if (pthresh->majFlag) {
			pParam->preStatus = SFM_ALM_MAJOR;
			fimd_makeSceUsageAlmMsg (pParam);
			fimd_saveSceUsageAlmInfo2DB (pParam);
		}

		if (pthresh->minFlag) {
			pParam->preStatus = SFM_ALM_MINOR;
			pParam->changeFlag = 0;
			pParam->occurFlag = 0;
		}
		else {
			pParam->changeFlag = 1;
			pParam->occurFlag = 1;
		}

		return SFM_ALM_MINOR;
	} //-- end of minor condition --//

	// major보다 높고 critical 기준값보다 낮은 경우, (major 장애 조건)
	// - 더 높은 등급(critical)이 발생되어 있었으면, 등급 하강 조건이므로 해지 메시지를 만든다.
	// - major가 이미 발생되어 있으면 no action.
	// - major 기준값 지속시간 경과여부를 확인하여 장애 판단.
	//
	else if (pParam->usage < pthresh->criLimit)
	{
		sceUserChkBuff[pParam->sysIndex].criStartTime = 0;
		pParam->curStatus = SFM_ALM_MAJOR;

		if (pthresh->criFlag) {
			pParam->preStatus = SFM_ALM_CRITICAL;
			fimd_makeSceUsageAlmMsg (pParam);
			fimd_saveSceUsageAlmInfo2DB (pParam);
		}
		if (pthresh->majFlag) {
			pParam->preStatus = SFM_ALM_MAJOR;
			pParam->changeFlag = 0;
			pParam->occurFlag = 0;
		}
		else {
			pParam->changeFlag = 1;
			pParam->occurFlag = 1;
		}
		return SFM_ALM_MAJOR;
	} //-- end of major condition --//

	// critical 기준값보다 높은 경우, (critical 장애 조건)
	// - critical이 이미 발생되어 있으면 no action.
	// - critical 기준값 지속시간 경과여부를 확인하여 장애 판단.
	//
	else
	{
		pParam->curStatus = SFM_ALM_CRITICAL;
		if (pthresh->criFlag) {
			pParam->preStatus = SFM_ALM_CRITICAL; //추가 sjjeon
			pParam->changeFlag = 0;
			pParam->occurFlag = 0;
		}
		else {
			pParam->changeFlag = 1;
			pParam->occurFlag = 1;
		}
		return SFM_ALM_CRITICAL;
	} //-- end of critical condition --//
}

/* hjjung */
int  fimd_thresholdCheckLegUsage(Threshold_st_int *pthresh, LEG_USAGE_PARAM *pParam)
{
	// minor 장애 기준값보다 낮은 경우,
	// - 이전에 장애가 발생되어 있었으면 장애가 해지된 경우이므로,
	//	발생되었던 각각 해당 등급에 대한 해지 메시지를 만들어 cond로 보낸다.
	if (pParam->usage < pthresh->minLimit)
	{
		legSessionChkBuff[pParam->sysIndex].minStartTime = 0;
		legSessionChkBuff[pParam->sysIndex].majStartTime = 0;
		legSessionChkBuff[pParam->sysIndex].criStartTime = 0;
		pParam->curStatus = SFM_ALM_NORMAL;

		if (pthresh->criFlag) {
			pParam->preStatus = SFM_ALM_CRITICAL;
			fimd_makeLegUsageAlmMsg (pParam);                                                                                     
			fimd_saveLegUsageAlmInfo2DB (pParam);
			pParam->changeFlag = 1;
			pParam->occurFlag = 0;
		}
		if (pthresh->majFlag) {
			pParam->preStatus = SFM_ALM_MAJOR;
			fimd_makeLegUsageAlmMsg (pParam);                                                                                     
			fimd_saveLegUsageAlmInfo2DB (pParam);
			pParam->changeFlag = 1;
			pParam->occurFlag = 0;
		}
		if (pthresh->minFlag) {
			pParam->preStatus = SFM_ALM_MINOR;
			fimd_makeLegUsageAlmMsg (pParam);                                                                                     
			fimd_saveLegUsageAlmInfo2DB (pParam);
			pParam->changeFlag = 1;
			pParam->occurFlag = 0;
		}
		return SFM_ALM_NORMAL;
	} //-- end of normal --//

	// minor보다 높고 major 기준값보다 낮은 경우, (minor 장애 조건)
	// - 더 높은 등급(major, critical)이 발생되어 있었으면, 등급 하강 조건이므로,
	//	각각 해지 메시지를 만든다.
	// - minor가 이미 발생되어 있으면 no action.
	// - minor 기준값 지속시간 경과여부를 확인하여 장애 판단.
	//
	else if (pParam->usage < pthresh->majLimit)
	{
		legSessionChkBuff[pParam->sysIndex].majStartTime = 0;
		legSessionChkBuff[pParam->sysIndex].criStartTime = 0;
		pParam->curStatus = SFM_ALM_MINOR;

		if (pthresh->criFlag) {
			pParam->preStatus = SFM_ALM_CRITICAL;
			fimd_makeLegUsageAlmMsg (pParam);                                                                                     
			fimd_saveLegUsageAlmInfo2DB (pParam);
		}
		if (pthresh->majFlag) {
			pParam->preStatus = SFM_ALM_MAJOR;
			fimd_makeLegUsageAlmMsg (pParam);                                                                                     
			fimd_saveLegUsageAlmInfo2DB (pParam);
		}

		if (pthresh->minFlag) {
			pParam->preStatus = SFM_ALM_MINOR;
			pParam->changeFlag = 0;
			pParam->occurFlag = 0;
		}
		else {
			pParam->changeFlag = 1;
			pParam->occurFlag = 1;
		}
		return SFM_ALM_MINOR;
	} //-- end of minor condition --//

	// major보다 높고 critical 기준값보다 낮은 경우, (major 장애 조건)
	// - 더 높은 등급(critical)이 발생되어 있었으면, 등급 하강 조건이므로 해지 메시지를 만든다.
	// - major가 이미 발생되어 있으면 no action.
	// - major 기준값 지속시간 경과여부를 확인하여 장애 판단.
	//
	else if (pParam->usage < pthresh->criLimit)
	{
		legSessionChkBuff[pParam->sysIndex].criStartTime = 0;
		pParam->curStatus = SFM_ALM_MAJOR;

		if (pthresh->criFlag) {
			pParam->preStatus = SFM_ALM_CRITICAL;
			fimd_makeLegUsageAlmMsg (pParam);                                                                                     
			fimd_saveLegUsageAlmInfo2DB (pParam);
		}
		if (pthresh->majFlag) {
			pParam->preStatus = SFM_ALM_MAJOR;
			pParam->changeFlag = 0;
			pParam->occurFlag = 0;
		}
		else {
			pParam->changeFlag = 1;
			pParam->occurFlag = 1;
		}
		return SFM_ALM_MAJOR;
	} //-- end of major condition --//

	// critical 기준값보다 높은 경우, (critical 장애 조건)
	// - critical이 이미 발생되어 있으면 no action.
	// - critical 기준값 지속시간 경과여부를 확인하여 장애 판단.
	//
	else
	{
		pParam->curStatus = SFM_ALM_CRITICAL;
		if (pthresh->criFlag) {
			pParam->preStatus = SFM_ALM_CRITICAL; //추가 sjjeon
			pParam->changeFlag = 0;
			pParam->occurFlag = 0;
		}
		else {
			pParam->changeFlag = 1;
			pParam->occurFlag = 1;
		}
		return SFM_ALM_CRITICAL;
	} //-- end of critical condition --//
}

/* added by dcham 20110525 for TPS */
int  fimd_thresholdCheckLegTps(Threshold_st_tps *pthresh, TPS_PARAM *pParam)
{
	// minor 장애 기준값보다 낮은 경우,
	// - 이전에 장애가 발생되어 있었으면 장애가 해지된 경우이므로,
	//	발생되었던 각각 해당 등급에 대한 해지 메시지를 만들어 cond로 보낸다.
	if (pParam->usage < pthresh->minLimit)
	{
		legTpsChkBuff[pParam->sysIndex].minStartTime = 0;
		legTpsChkBuff[pParam->sysIndex].majStartTime = 0;
		legTpsChkBuff[pParam->sysIndex].criStartTime = 0;
		pParam->curStatus = SFM_ALM_NORMAL;

		if (pthresh->criFlag) {
			pParam->preStatus = SFM_ALM_CRITICAL;
			fimd_makeLegTpsAlmMsg (pParam);                                                                                     
			fimd_saveLegTpsAlmInfo2DB (pParam);
			pParam->changeFlag = 1;
			pParam->occurFlag = 0;
		}
		if (pthresh->majFlag) {
			pParam->preStatus = SFM_ALM_MAJOR;
			fimd_makeLegTpsAlmMsg (pParam);                                                                                     
			fimd_saveLegTpsAlmInfo2DB (pParam);
			pParam->changeFlag = 1;
			pParam->occurFlag = 0;
		}
		if (pthresh->minFlag) {
			pParam->preStatus = SFM_ALM_MINOR;
			fimd_makeLegTpsAlmMsg (pParam);                                                                                     
			fimd_saveLegTpsAlmInfo2DB (pParam);
			pParam->changeFlag = 1;
			pParam->occurFlag = 0;
		}
		return SFM_ALM_NORMAL;
	} //-- end of normal --//

	// minor보다 높고 major 기준값보다 낮은 경우, (minor 장애 조건)
	// - 더 높은 등급(major, critical)이 발생되어 있었으면, 등급 하강 조건이므로,
	//	각각 해지 메시지를 만든다.
	// - minor가 이미 발생되어 있으면 no action.
	// - minor 기준값 지속시간 경과여부를 확인하여 장애 판단.
	//
	else if (pParam->usage < pthresh->majLimit)
	{
		legTpsChkBuff[pParam->sysIndex].majStartTime = 0;
		legTpsChkBuff[pParam->sysIndex].criStartTime = 0;
		pParam->curStatus = SFM_ALM_MINOR;

		if (pthresh->criFlag) {
			pParam->preStatus = SFM_ALM_CRITICAL;
			fimd_makeLegTpsAlmMsg (pParam);                                                                                     
			fimd_saveLegTpsAlmInfo2DB (pParam);
		}
		if (pthresh->majFlag) {
			pParam->preStatus = SFM_ALM_MAJOR;
			fimd_makeLegTpsAlmMsg (pParam);                                                                                     
			fimd_saveLegTpsAlmInfo2DB (pParam);
		}

		if (pthresh->minFlag) {
			pParam->preStatus = SFM_ALM_MINOR;
			pParam->changeFlag = 0;
			pParam->occurFlag = 0;
		}
		else {
			pParam->changeFlag = 1;
			pParam->occurFlag = 1;
		}
		return SFM_ALM_MINOR;
	} //-- end of minor condition --//

	// major보다 높고 critical 기준값보다 낮은 경우, (major 장애 조건)
	// - 더 높은 등급(critical)이 발생되어 있었으면, 등급 하강 조건이므로 해지 메시지를 만든다.
	// - major가 이미 발생되어 있으면 no action.
	// - major 기준값 지속시간 경과여부를 확인하여 장애 판단.
	//
	else if (pParam->usage < pthresh->criLimit)
	{
		legTpsChkBuff[pParam->sysIndex].criStartTime = 0;
		pParam->curStatus = SFM_ALM_MAJOR;

		if (pthresh->criFlag) {
			pParam->preStatus = SFM_ALM_CRITICAL;
			fimd_makeLegTpsAlmMsg (pParam);                                                                                     
			fimd_saveLegTpsAlmInfo2DB (pParam);
		}
		if (pthresh->majFlag) {
			pParam->preStatus = SFM_ALM_MAJOR;
			pParam->changeFlag = 0;
			pParam->occurFlag = 0;
		}
		else {
			pParam->changeFlag = 1;
			pParam->occurFlag = 1;
		}
		return SFM_ALM_MAJOR;
	} //-- end of major condition --//

	// critical 기준값보다 높은 경우, (critical 장애 조건)
	// - critical이 이미 발생되어 있으면 no action.
	// - critical 기준값 지속시간 경과여부를 확인하여 장애 판단.
	//
	else
	{
		pParam->curStatus = SFM_ALM_CRITICAL;
		if (pthresh->criFlag) {
			pParam->preStatus = SFM_ALM_CRITICAL; //추가 sjjeon
			pParam->changeFlag = 0;
			pParam->occurFlag = 0;
		}
		else {
			pParam->changeFlag = 1;
			pParam->occurFlag = 1;
		}
		return SFM_ALM_CRITICAL;
	} //-- end of critical condition --//
}
/*
   by sjjeon
   지정된 Normal status 에 따라서 이전상태와 비교하며, 상태 변경시 해당 Level로 
   반환한다. 상태 체크만 사용한다. 
*/
int fimd_thresholdCommCheckStatus(COMM_STATUS_PARAM *pParam)
{
	int normalStat,  abnormalLevel;
	int level=-1;

	// 장비의 abnormal level 값을 얻어온다.
	abnormalLevel = fimd_getAbnormalAlarmLevel_byDevName(pParam);

	// 장비의 정상 상태 값을 얻어온다.
	normalStat = fimd_getNormalStatValue(pParam->sysType, pParam->devKind);

	if (normalStat < 0 || abnormalLevel <0) {
		sprintf(trcBuf,"[%s:%d] get value fail. level : %d, normalstat : %d\n", 
				__FUNCTION__, __LINE__, abnormalLevel, normalStat);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}
	// 현재 상태 NORMAL
	if (pParam->status == normalStat)
	{
		// 이전상태 ABNORMAL, abnor -> nor
		if (pParam->preStatus != pParam->status) {
			pParam->changeFlag = 1;
			pParam->occurFlag = 0;
		}else{
			pParam->changeFlag = 0;
			pParam->occurFlag = 0;
		}
		pParam->preStatus = pParam->status;
		level = SFM_ALM_NORMAL;
	}
	else{
		// 이전상태 NORMAL,  nor ->  abnor
		if (pParam->preStatus != pParam->status) {
			pParam->changeFlag = 1;
			pParam->occurFlag = 1;
		}else{
			pParam->changeFlag = 0;
			pParam->occurFlag = 0;
		}
		pParam->preStatus = pParam->status;
		level = abnormalLevel;
	}
/*
	if(pParam->devKind==SCE_RDR_CONN){
		fprintf(stderr, "sysType:%d, devIdx:%d, sts:%d, normalsts:%d, level:%d\n",
				pParam->sysType, pParam->devIndex, pParam->status, normalStat,level );
		fprintf(stderr,"=========================\n");
	}
*/
	return level;
}

#if 0 /* by june */
int fimd_checkSceCpuUsageAlm (int devIndex, int cpuIndex)
{
	int		usage;
	int		changeFlag=0; // 장애 발생 또는 해지인 경우 1로 setting된다.

	usage = g_pstSCEInfo->SCEDev[devIndex].cpuInfo[cpuIndex].usage;
	if ((g_pstSCEInfo->SCEDev[devIndex].cpuInfo[cpuIndex].usage % 10) >= 5)
		usage++;
	
	// minor 장애 기준값보다 낮은 경우,
	// - 이전에 장애가 발생되어 있었으면 장애가 해지된 경우이므로,
	//	발생되었던 각각 해당 등급에 대한 해지 메시지를 만들어 cond로 보낸다.
	//
	if (usage < g_pstSCEInfo->SCEDev[devIndex].cpuInfo[cpuIndex].minLimit)
	{
		pdCpuChkBuff[devIndex].minStartTime = 0;
		pdCpuChkBuff[devIndex].majStartTime = 0;
		pdCpuChkBuff[devIndex].criStartTime = 0;

		g_pstSCEInfo->SCEDev[devIndex].cpuInfo[cpuIndex].level = SFM_ALM_NORMAL;  // sfdb->sys에 update

		if (g_pstSCEInfo->SCEDev[devIndex].cpuInfo[cpuIndex].cpuFlag.criFlag) {
			fimd_hdlSceCpuUsageAlm (devIndex, SFM_ALM_TYPE_SCE_CPU_USAGE, cpuIndex, usage, SFM_ALM_NORMAL, SFM_ALM_CRITICAL, 0); // critical clear
			changeFlag = 1;
		}
		if (g_pstSCEInfo->SCEDev[devIndex].cpuInfo[cpuIndex].cpuFlag.majFlag) {
			fimd_hdlSceCpuUsageAlm (devIndex, cpuIndex, SFM_ALM_NORMAL, SFM_ALM_MAJOR, 0); // major clear
			changeFlag = 1;
		}
		if (g_pstSCEInfo->SCEDev[devIndex].cpuInfo[cpuIndex].cpuFlag.minFlag) {
			fimd_hdlSceCpuUsageAlm (devIndex, cpuIndex, SFM_ALM_NORMAL, SFM_ALM_MINOR, 0); // minor clear
			changeFlag = 1;
		}

		g_pstSCEInfo->SCEDev[devIndex].cpuInfo[cpuIndex].cpuFlag.criFlag = 0;
		g_pstSCEInfo->SCEDev[devIndex].cpuInfo[cpuIndex].cpuFlag.majFlag = 0;
		g_pstSCEInfo->SCEDev[devIndex].cpuInfo[cpuIndex].cpuFlag.minFlag = 0;

	} //-- end of normal --//

	// minor보다 높고 major 기준값보다 낮은 경우, (minor 장애 조건)
	// - 더 높은 등급(major, critical)이 발생되어 있었으면, 등급 하강 조건이므로,
	//	각각 해지 메시지를 만든다.
	// - minor가 이미 발생되어 있으면 no action.
	// - minor 기준값 지속시간 경과여부를 확인하여 장애 판단.
	//
	else if (usage < g_pstSCEInfo->SCEDev[devIndex].cpuInfo[cpuIndex].majLimit)
	{
		pdCpuChkBuff[devIndex].majStartTime = 0;
		pdCpuChkBuff[devIndex].criStartTime = 0;

		if (g_pstSCEInfo->SCEDev[devIndex].cpuInfo[cpuIndex].cpuFlag.criFlag) {
			g_pstSCEInfo->SCEDev[devIndex].cpuInfo[cpuIndex].level = SFM_ALM_MINOR;  // sfdb->sys에 update
			fimd_hdlSceCpuUsageAlm (devIndex, cpuIndex, SFM_ALM_MINOR, SFM_ALM_CRITICAL, 0); // critical clear
			changeFlag = 1;
		}
		if (g_pstSCEInfo->SCEDev[devIndex].cpuInfo[cpuIndex].cpuFlag.majFlag) {
			g_pstSCEInfo->SCEDev[devIndex].cpuInfo[cpuIndex].level = SFM_ALM_MINOR;  // sfdb->sys에 update
			fimd_hdlSceCpuUsageAlm (devIndex, cpuIndex, SFM_ALM_MINOR, SFM_ALM_MAJOR, 0); // major clear
			changeFlag = 1;
		}

		if (g_pstSCEInfo->SCEDev[devIndex].cpuInfo[cpuIndex].cpuFlag.minFlag) {
			; // 이미 minor가 발생되어 있으면 no action.
		} else {
			if (pdCpuChkBuff[devIndex].minStartTime == 0) // 처음 limit를 넘었는가?
				pdCpuChkBuff[devIndex].minStartTime = currentTime;
			if (g_pstSCEInfo->SCEDev[devIndex].cpuInfo[cpuIndex].minDurat > (currentTime - pdCpuChkBuff[devIndex].minStartTime)) {
				; // 지속시간 경과하지 않았으면 no action.
			} else {
				g_pstSCEInfo->SCEDev[devIndex].cpuInfo[cpuIndex].level = SFM_ALM_MINOR;  // sfdb->sys에 update
				fimd_hdlSceCpuUsageAlm (devIndex, cpuIndex, SFM_ALM_MINOR, 0, 1); // minor occur
				g_pstSCEInfo->SCEDev[devIndex].cpuInfo[cpuIndex].cpuFlag.criFlag = 0;
				g_pstSCEInfo->SCEDev[devIndex].cpuInfo[cpuIndex].cpuFlag.majFlag = 0;
				g_pstSCEInfo->SCEDev[devIndex].cpuInfo[cpuIndex].cpuFlag.minFlag = 1;
				changeFlag = 1;
			}
		}

	} //-- end of minor condition --//

	// major보다 높고 critical 기준값보다 낮은 경우, (major 장애 조건)
	// - 더 높은 등급(critical)이 발생되어 있었으면, 등급 하강 조건이므로 해지 메시지를 만든다.
	// - major가 이미 발생되어 있으면 no action.
	// - major 기준값 지속시간 경과여부를 확인하여 장애 판단.
	//
	else if (usage < g_pstSCEInfo->SCEDev[devIndex].cpuInfo[cpuIndex].criLimit)
	{
		pdCpuChkBuff[devIndex].criStartTime = 0;

		if (g_pstSCEInfo->SCEDev[devIndex].cpuInfo[cpuIndex].cpuFlag.minFlag) {
			g_pstSCEInfo->SCEDev[devIndex].cpuInfo[cpuIndex].level = SFM_ALM_MAJOR;  // sfdb->sys에 update
			fimd_hdlSceCpuUsageAlm (devIndex, cpuIndex, SFM_ALM_MAJOR, SFM_ALM_CRITICAL, 0); // critical clear
			changeFlag = 1;
		}

		if (g_pstSCEInfo->SCEDev[devIndex].cpuInfo[cpuIndex].cpuFlag.criFlag) {
			g_pstSCEInfo->SCEDev[devIndex].cpuInfo[cpuIndex].level = SFM_ALM_MAJOR;  // sfdb->sys에 update
			fimd_hdlSceCpuUsageAlm (devIndex, cpuIndex, SFM_ALM_MAJOR, SFM_ALM_CRITICAL, 0); // critical clear
			changeFlag = 1;
		}

		if (g_pstSCEInfo->SCEDev[devIndex].cpuInfo[cpuIndex].cpuFlag.majFlag) {
			; // 이미 major가 발생되어 있으면 no action.
		} else {
			// minor에서 상승하는게 아니라 한방에 major로 올라갈 수 있으므로
			//	minStartTime을 같이 변경해야한다.
			if (pdCpuChkBuff[devIndex].minStartTime == 0) // 처음 limit를 넘었는가?
				pdCpuChkBuff[devIndex].minStartTime = currentTime;
			if (pdCpuChkBuff[devIndex].majStartTime == 0) // 처음 limit를 넘었는가?
				pdCpuChkBuff[devIndex].majStartTime = currentTime;

			if (g_pstSCEInfo->SCEDev[devIndex].cpuInfo[cpuIndex].majDurat > (currentTime - pdCpuChkBuff[devIndex].majStartTime)) {
				; // 지속시간 경과하지 않았으면 no action.
			} else {
				g_pstSCEInfo->SCEDev[devIndex].cpuInfo[cpuIndex].level = SFM_ALM_MAJOR;  // sfdb->sys에 update
				fimd_hdlSceCpuUsageAlm (devIndex, cpuIndex, SFM_ALM_MAJOR, 0, 1); // major occur
				g_pstSCEInfo->SCEDev[devIndex].cpuInfo[cpuIndex].cpuFlag.criFlag = 0;
				g_pstSCEInfo->SCEDev[devIndex].cpuInfo[cpuIndex].cpuFlag.majFlag = 1;
				g_pstSCEInfo->SCEDev[devIndex].cpuInfo[cpuIndex].cpuFlag.minFlag = 0;
				changeFlag = 1;
			}
		}

	} //-- end of major condition --//

	// critical 기준값보다 높은 경우, (critical 장애 조건)
	// - critical이 이미 발생되어 있으면 no action.
	// - critical 기준값 지속시간 경과여부를 확인하여 장애 판단.
	//
	else
	{
		if (g_pstSCEInfo->SCEDev[devIndex].cpuInfo[cpuIndex].cpuFlag.criFlag) {
			; // 이미 발생되어 있으면 no action.
		} else {
			// minor,major에서 상승하는게 아니라 한방에 critical로 올라갈 수 있으므로
			//	minStartTime, majorStartTime을 같이 변경해야한다.
			if (pdCpuChkBuff[devIndex].minStartTime == 0) // 처음 limit를 넘었는가?
				pdCpuChkBuff[devIndex].minStartTime = currentTime;
			if (pdCpuChkBuff[devIndex].majStartTime == 0) // 처음 limit를 넘었는가?
				pdCpuChkBuff[devIndex].majStartTime = currentTime;
			if (pdCpuChkBuff[devIndex].criStartTime == 0) // 처음 limit를 넘었는가?
				pdCpuChkBuff[devIndex].criStartTime = currentTime;

			if (g_pstSCEInfo->SCEDev[devIndex].cpuInfo[cpuIndex].criDurat > (currentTime - pdCpuChkBuff[devIndex].criStartTime)) {
				; // 지속시간 경과하지 않았으면 no action.
			} else {
				g_pstSCEInfo->SCEDev[devIndex].cpuInfo[cpuIndex].level = SFM_ALM_CRITICAL;  // sfdb->sys에 update
				fimd_hdlSceCpuUsageAlm (devIndex, cpuIndex, SFM_ALM_CRITICAL, 0, 1); // critical occur
				g_pstSCEInfo->SCEDev[devIndex].cpuInfo[cpuIndex].cpuFlag.criFlag = 1;
				g_pstSCEInfo->SCEDev[devIndex].cpuInfo[cpuIndex].cpuFlag.majFlag = 0;
				g_pstSCEInfo->SCEDev[devIndex].cpuInfo[cpuIndex].cpuFlag.minFlag = 0;
				changeFlag = 1;
			}
		}

	} //-- end of critical condition --//


	return changeFlag;

} //----- End of fimd_checkSceCpuUsageAlm -----//
#endif /* by june */


/* ADD : by june, 2010-0907
 *	+ DISCRIPTION :
 *	  - alarm status 변경시 alarm clear message가 발생하지 않는 버그 수정.
 *	  - 기존 함수 주석 처리하고 copy 해서 재작성.
 *	+ LINE : 2184 - 2350
 *	+ TODO : 테스트 후 문제 없으면 기존 함수 제거.
 */
int fimd_checkSceCpuUsageAlm (int sysIndex, int cpuIndex)
{
	Threshold_st    thresh;
	SCE_USAGE_PARAM param;
	int status;

	param.sysIndex = sysIndex;
	param.devKind  = SCE_CPU; 
	param.devIndex = cpuIndex;
	param.usage = g_pstSCEInfo->SCEDev[sysIndex].cpuInfo[cpuIndex].usage;
	
	param.curStatus = 0;
	param.preStatus = 0; 
	param.occurFlag = 0; 
	param.changeFlag = 0;

	thresh.minLimit =  g_pstSCEInfo->SCEDev[sysIndex].cpuInfo[cpuIndex].minLimit;
	thresh.majLimit =  g_pstSCEInfo->SCEDev[sysIndex].cpuInfo[cpuIndex].majLimit;
	thresh.criLimit =  g_pstSCEInfo->SCEDev[sysIndex].cpuInfo[cpuIndex].criLimit;

	thresh.minFlag =  g_pstSCEInfo->SCEDev[sysIndex].cpuInfo[cpuIndex].minFlag;
	thresh.majFlag =  g_pstSCEInfo->SCEDev[sysIndex].cpuInfo[cpuIndex].majFlag;
	thresh.criFlag =  g_pstSCEInfo->SCEDev[sysIndex].cpuInfo[cpuIndex].criFlag;;

	status = fimd_thresholdCheckUsage(&thresh, &param);
	// 1. 상태 판단 해 
	// 2. 상태 변환 유무 확인. (changeFlag 확인) 
	// 2.1 상태 변화가 있는 경우. 
	//     cur_status, pre_status , 0
	//     현재 상태 정리. 
	// 2.2 상태 변화가 없는 경우. 
	//     현재 상태 유지, (no action)
	switch (status) {
	case SFM_ALM_NORMAL:
		g_pstSCEInfo->SCEDev[sysIndex].cpuInfo[cpuIndex].level = SFM_ALM_NORMAL;  // sfdb->sys에 update
		g_pstSCEInfo->SCEDev[sysIndex].cpuInfo[cpuIndex].criFlag = 0;
		g_pstSCEInfo->SCEDev[sysIndex].cpuInfo[cpuIndex].majFlag = 0;
		g_pstSCEInfo->SCEDev[sysIndex].cpuInfo[cpuIndex].minFlag = 0;
		break;

	case  SFM_ALM_MINOR:

		if (param.changeFlag == 1) {
			if (sceCpuChkBuff[sysIndex][cpuIndex].minStartTime == 0) // 처음 limit를 넘었는가?
				sceCpuChkBuff[sysIndex][cpuIndex].minStartTime = currentTime;
			if (g_pstSCEInfo->SCEDev[sysIndex].cpuInfo[cpuIndex].minDurat > (currentTime - sceCpuChkBuff[sysIndex][cpuIndex].minStartTime)) {
				; // 지속시간 경과하지 않았으면 no action.
			} else {
				g_pstSCEInfo->SCEDev[sysIndex].cpuInfo[cpuIndex].level = SFM_ALM_MINOR;  // sfdb->sys에 update
				fimd_makeSceUsageAlmMsg (&param);
				fimd_saveSceUsageAlmInfo2DB (&param);
				// 장애 발생시 장애통계 count한다.
				if (param.occurFlag) {
					// 시스템별, 유형별, 등급별 구분
					//fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getSceAlarmType(param.devKind), param.curStatus);
					fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getAlarmType(param.devKind), param.curStatus);
				}
				g_pstSCEInfo->SCEDev[sysIndex].cpuInfo[cpuIndex].criFlag = 0;
				g_pstSCEInfo->SCEDev[sysIndex].cpuInfo[cpuIndex].majFlag = 0;
				g_pstSCEInfo->SCEDev[sysIndex].cpuInfo[cpuIndex].minFlag = 1;
			}
		}
		break;

	case  SFM_ALM_MAJOR:

		if (param.changeFlag == 1) {
			// minor에서 상승하는게 아니라 한방에 major로 올라갈 수 있으므로
			//  minStartTime을 같이 변경해야한다.
			if (sceCpuChkBuff[sysIndex][cpuIndex].minStartTime == 0) // 처음 limit를 넘었는가?
				sceCpuChkBuff[sysIndex][cpuIndex].minStartTime = currentTime;
			if (sceCpuChkBuff[sysIndex][cpuIndex].majStartTime == 0) // 처음 limit를 넘었는가?
				sceCpuChkBuff[sysIndex][cpuIndex].majStartTime = currentTime;

			if (g_pstSCEInfo->SCEDev[sysIndex].cpuInfo[cpuIndex].majDurat > (currentTime - sceCpuChkBuff[sysIndex][cpuIndex].majStartTime)) {
				; // 지속시간 경과하지 않았으면 no action.
			} else {
				g_pstSCEInfo->SCEDev[sysIndex].cpuInfo[cpuIndex].level = SFM_ALM_MAJOR;  // sfdb->sys에 update
				fimd_makeSceUsageAlmMsg (&param);
				fimd_saveSceUsageAlmInfo2DB (&param);
				if (param.occurFlag) {
					//fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getSceAlarmType(param.devKind), param.curStatus);
					fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getAlarmType(param.devKind), param.curStatus);
				}
				g_pstSCEInfo->SCEDev[sysIndex].cpuInfo[cpuIndex].criFlag = 0;
				g_pstSCEInfo->SCEDev[sysIndex].cpuInfo[cpuIndex].majFlag = 1;
				g_pstSCEInfo->SCEDev[sysIndex].cpuInfo[cpuIndex].minFlag = 0;
			}
		}
		break;

	default:        // CRITICAL

		if (param.changeFlag == 1) {
			
			// minor,major에서 상승하는게 아니라 한방에 critical로 올라갈 수 있으므로
			//  minStartTime, majorStartTime을 같이 변경해야한다.
			if (sceCpuChkBuff[sysIndex][cpuIndex].minStartTime == 0) // 처음 limit를 넘었는가?
				sceCpuChkBuff[sysIndex][cpuIndex].minStartTime = currentTime;
			if (sceCpuChkBuff[sysIndex][cpuIndex].majStartTime == 0) // 처음 limit를 넘었는가?
				sceCpuChkBuff[sysIndex][cpuIndex].majStartTime = currentTime;
			if (sceCpuChkBuff[sysIndex][cpuIndex].criStartTime == 0) // 처음 limit를 넘었는가?
				sceCpuChkBuff[sysIndex][cpuIndex].criStartTime = currentTime;

			if (g_pstSCEInfo->SCEDev[sysIndex].cpuInfo[cpuIndex].criDurat > (currentTime - sceCpuChkBuff[sysIndex][cpuIndex].criStartTime)) {
				;
			} else {
				g_pstSCEInfo->SCEDev[sysIndex].cpuInfo[cpuIndex].level = SFM_ALM_CRITICAL;  // sfdb->sys에 update
				fimd_makeSceUsageAlmMsg (&param);
				fimd_saveSceUsageAlmInfo2DB (&param);
				if (param.occurFlag) {
					//fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getSceAlarmType(param.devKind), param.curStatus);
					fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getAlarmType(param.devKind), param.curStatus);
				}
				g_pstSCEInfo->SCEDev[sysIndex].cpuInfo[cpuIndex].criFlag = 1;
				g_pstSCEInfo->SCEDev[sysIndex].cpuInfo[cpuIndex].majFlag = 0;
				g_pstSCEInfo->SCEDev[sysIndex].cpuInfo[cpuIndex].minFlag = 0;
			}
		}
		break;
	}

	return param.changeFlag;
}


/* ADD : by june, 2010-09-07
 * DSCRIPTION:
 * LINE:
 * TODO:
 */
int fimd_checkSceMemUsageAlm (int sysIndex, int memIndex)
{
	Threshold_st    thresh;
	SCE_USAGE_PARAM param;
	int status;

	param.sysIndex = sysIndex;
	param.devKind  = SCE_MEM; 
	param.devIndex = memIndex;
	param.usage = g_pstSCEInfo->SCEDev[sysIndex].memInfo[memIndex].usage;

	param.curStatus = 0;
	param.preStatus = 0; 
	param.occurFlag = 0; 
	param.changeFlag = 0;

	thresh.minLimit =  g_pstSCEInfo->SCEDev[sysIndex].memInfo[memIndex].minLimit;
	thresh.majLimit =  g_pstSCEInfo->SCEDev[sysIndex].memInfo[memIndex].majLimit;
	thresh.criLimit =  g_pstSCEInfo->SCEDev[sysIndex].memInfo[memIndex].criLimit;

	thresh.minFlag =  g_pstSCEInfo->SCEDev[sysIndex].memInfo[memIndex].minFlag;
	thresh.majFlag =  g_pstSCEInfo->SCEDev[sysIndex].memInfo[memIndex].majFlag;
	thresh.criFlag =  g_pstSCEInfo->SCEDev[sysIndex].memInfo[memIndex].criFlag;;

	status = fimd_thresholdCheckUsage(&thresh, &param);

	switch (status) {
	case SFM_ALM_NORMAL:
		g_pstSCEInfo->SCEDev[sysIndex].memInfo[memIndex].level = SFM_ALM_NORMAL;  // sfdb->sys에 update
		g_pstSCEInfo->SCEDev[sysIndex].memInfo[memIndex].criFlag = 0;
		g_pstSCEInfo->SCEDev[sysIndex].memInfo[memIndex].majFlag = 0;
		g_pstSCEInfo->SCEDev[sysIndex].memInfo[memIndex].minFlag = 0;
		break;

	case  SFM_ALM_MINOR:

		if (param.changeFlag == 1) {

			if (sceMemChkBuff[sysIndex][memIndex].minStartTime == 0) // 처음 limit를 넘었는가?
				sceMemChkBuff[sysIndex][memIndex].minStartTime = currentTime;
			if (g_pstSCEInfo->SCEDev[sysIndex].memInfo[memIndex].minDurat > (currentTime - sceMemChkBuff[sysIndex][memIndex].minStartTime)) {
				; // 지속시간 경과하지 않았으면 no action.
			} else {
				g_pstSCEInfo->SCEDev[sysIndex].memInfo[memIndex].level = SFM_ALM_MINOR;  // sfdb->sys에 update
				fimd_makeSceUsageAlmMsg (&param);
				fimd_saveSceUsageAlmInfo2DB (&param);
				// 장애 발생시 장애통계 count한다.
				if (param.occurFlag) {
					// 시스템별, 유형별, 등급별 구분
					//fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getSceAlarmType(param.devKind), param.curStatus);
					fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getAlarmType(param.devKind), param.curStatus);
				}
				g_pstSCEInfo->SCEDev[sysIndex].memInfo[memIndex].criFlag = 0;
				g_pstSCEInfo->SCEDev[sysIndex].memInfo[memIndex].majFlag = 0;
				g_pstSCEInfo->SCEDev[sysIndex].memInfo[memIndex].minFlag = 1;
			}
		}
		break;

	case  SFM_ALM_MAJOR:

		if (param.changeFlag == 1) {
			// minor에서 상승하는게 아니라 한방에 major로 올라갈 수 있으므로
			//  minStartTime을 같이 변경해야한다.
			if (sceMemChkBuff[sysIndex][memIndex].minStartTime == 0) // 처음 limit를 넘었는가?
				sceMemChkBuff[sysIndex][memIndex].minStartTime = currentTime;
			if (sceMemChkBuff[sysIndex][memIndex].majStartTime == 0) // 처음 limit를 넘었는가?
				sceMemChkBuff[sysIndex][memIndex].majStartTime = currentTime;

			if (g_pstSCEInfo->SCEDev[sysIndex].memInfo[memIndex].majDurat > (currentTime - sceMemChkBuff[sysIndex][memIndex].majStartTime)) {
				; // 지속시간 경과하지 않았으면 no action.
			} else {
				g_pstSCEInfo->SCEDev[sysIndex].memInfo[memIndex].level = SFM_ALM_MAJOR;  // sfdb->sys에 update
				fimd_makeSceUsageAlmMsg (&param);
				fimd_saveSceUsageAlmInfo2DB (&param);
				if (param.occurFlag) {
					//fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getSceAlarmType(param.devKind), param.curStatus);
					fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getAlarmType(param.devKind), param.curStatus);
				}
				g_pstSCEInfo->SCEDev[sysIndex].memInfo[memIndex].criFlag = 0;
				g_pstSCEInfo->SCEDev[sysIndex].memInfo[memIndex].majFlag = 1;
				g_pstSCEInfo->SCEDev[sysIndex].memInfo[memIndex].minFlag = 0;
			}
		}
		break;

	default:        // CRITICAL

		if (param.changeFlag == 1) {
			// minor,major에서 상승하는게 아니라 한방에 critical로 올라갈 수 있으므로
			//  minStartTime, majorStartTime을 같이 변경해야한다.
			if (sceMemChkBuff[sysIndex][memIndex].minStartTime == 0) // 처음 limit를 넘었는가?
				sceMemChkBuff[sysIndex][memIndex].minStartTime = currentTime;
			if (sceMemChkBuff[sysIndex][memIndex].majStartTime == 0) // 처음 limit를 넘었는가?
				sceMemChkBuff[sysIndex][memIndex].majStartTime = currentTime;
			if (sceMemChkBuff[sysIndex][memIndex].criStartTime == 0) // 처음 limit를 넘었는가?
				sceMemChkBuff[sysIndex][memIndex].criStartTime = currentTime;

			if (g_pstSCEInfo->SCEDev[sysIndex].memInfo[memIndex].criDurat > (currentTime - sceMemChkBuff[sysIndex][memIndex].criStartTime)) {
				;
			} else {
				g_pstSCEInfo->SCEDev[sysIndex].memInfo[memIndex].level = SFM_ALM_CRITICAL;  // sfdb->sys에 update
				fimd_makeSceUsageAlmMsg (&param);
				fimd_saveSceUsageAlmInfo2DB (&param);
				if (param.occurFlag) {
					//fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getSceAlarmType(param.devKind), param.curStatus);
					fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getAlarmType(param.devKind), param.curStatus);
				}
				g_pstSCEInfo->SCEDev[sysIndex].memInfo[memIndex].criFlag = 1;
				g_pstSCEInfo->SCEDev[sysIndex].memInfo[memIndex].majFlag = 0;
				g_pstSCEInfo->SCEDev[sysIndex].memInfo[memIndex].minFlag = 0;
			}
		}
		break;
	}
	return param.changeFlag;
}


/*
	by sjjeon (09.6) - 재정의....
	SCE Memory Usage 상태 관리.
*/
int fimd_checkSceMemUsageAlm2 (int sysType, int unitType, int unitIndex)
{
	int  usage=0, changeFlag=0, sysIndex=0;
	SCE_SYS_USAGE_INFO     *memInfo;

	if(sysType == SCEA) sysIndex = 0;
	else if(sysType == SCEB) sysIndex =1;
	else {
		sprintf(trcBuf,"[%s] UNKNOWN SYSTYPE[%d]\n",__FUNCTION__,sysType);
        trclib_writeLogErr (FL,trcBuf);
		return -1;
	}
	if(unitIndex<0 || unitIndex >=MAX_SCE_MEM_CNT){
		sprintf(trcBuf,"[%s] INVALID UNIT INDEX[%d]\n",__FUNCTION__,unitIndex);
        trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	memInfo = &g_pstSCEInfo->SCEDev[sysIndex].memInfo[unitIndex];
	usage = memInfo->usage;

	// minor 장애 기준값보다 낮은 경우, 
	// - 이전에 장애가 발생되어 있었으면 장애가 해지된 경우이므로,
	//  발생되었던 각각 해당 등급에 대한 해지 메시지를 만들어 cond로 보낸다.
	//  
	if (usage < memInfo->minLimit)
	{   
		memInfo->level = SFM_ALM_NORMAL;  // g_pstSCEInfo->SCEDev 에 update
		if (memInfo->criFlag) {
			fimd_CommUsageAlm(sysType, unitType, unitIndex, usage, SFM_ALM_NORMAL, SFM_ALM_CRITICAL, 0);
			changeFlag = 1; 
		}
		if (memInfo->majFlag){
			fimd_CommUsageAlm(sysType, unitType, unitIndex, usage, SFM_ALM_NORMAL, SFM_ALM_MAJOR, 0);
			changeFlag = 1;
		}
		if (memInfo->minFlag){
			fimd_CommUsageAlm(sysType, unitType, unitIndex, usage, SFM_ALM_NORMAL, SFM_ALM_MINOR, 0);
			changeFlag = 1;
		}

		memInfo->criFlag = 0;
		memInfo->majFlag = 0;
		memInfo->minFlag = 0;

	} //-- end of normal --//

	// minor보다 높고 major 기준값보다 낮은 경우, (minor 장애 조건)
	// - 더 높은 등급(major, critical)이 발생되어 있었으면, 등급 하강 조건이므로,
	//  각각 해지 메시지를 만든다.
	// - minor가 이미 발생되어 있으면 no action.
	// - mem 사용율 장애는 지속시간이 없으므로 곧바로 장애 판단.
	//
	else if (usage < memInfo->majLimit)
	{
		memInfo->level = SFM_ALM_MINOR;  // g_pstSCEInfo->SCEDev 에 update
		if (memInfo->criFlag) {
			fimd_CommUsageAlm(sysType, unitType, unitIndex, usage, SFM_ALM_MINOR, SFM_ALM_CRITICAL, 0);
			changeFlag = 1;
		}

		if (memInfo->majFlag){
			fimd_CommUsageAlm(sysType, unitType, unitIndex, usage, SFM_ALM_MINOR, SFM_ALM_MAJOR, 0);
			changeFlag = 1;
		}

		if (memInfo->minFlag){
			; // 이미 minor가 발생되어 있으면 no action.
		} else {
			fimd_CommUsageAlm(sysType, unitType, unitIndex, usage, SFM_ALM_MINOR, SFM_ALM_MAJOR, 1);
			memInfo->criFlag = 0;
			memInfo->majFlag = 0;
			memInfo->minFlag = 1;
			changeFlag = 1;
		}

	} //-- end of minor condition --//

	// major보다 높고 critical 기준값보다 낮은 경우, (major 장애 조건)
	// - 더 높은 등급(critical)이 발생되어 있었으면, 등급 하강 조건이므로 해지 메시지를 만든다.
	// - major가 이미 발생되어 있으면 no action.
	//
	else if (usage < memInfo->criLimit)
	{
		memInfo->level = SFM_ALM_MAJOR;  // g_pstSCEInfo->SCEDev 에 update

		if (memInfo->minFlag){
			fimd_CommUsageAlm(sysType, unitType, unitIndex, usage, SFM_ALM_MAJOR, SFM_ALM_MINOR, 0);
			memInfo->minFlag = 0;
			changeFlag = 1;
		}

		if (memInfo->criFlag){
			fimd_CommUsageAlm(sysType, unitType, unitIndex, usage, SFM_ALM_MAJOR, SFM_ALM_CRITICAL, 0);
			memInfo->criFlag = 0;
			changeFlag = 1;
		}

		if (memInfo->majFlag) {
			; // 이미 major가 발생되어 있으면 no action.
		} else {
			fimd_CommUsageAlm(sysType, unitType, unitIndex, usage, SFM_ALM_MAJOR, SFM_ALM_CRITICAL, 1);
			memInfo->criFlag = 0;
			memInfo->majFlag = 1;
			memInfo->minFlag = 0;
			changeFlag = 1;
		}

	} //-- end of major condition --//

	// critical 기준값보다 높은 경우, (critical 장애 조건)
	// - critical이 이미 발생되어 있으면 no action.
	// - critical 기준값 지속시간 경과여부를 확인하여 장애 판단.
	//
	else
	{
		memInfo->level = SFM_ALM_CRITICAL;  // g_pstSCEInfo->SCEDev 에 update

		if (memInfo->criFlag) {
			; // 이미 발생되어 있으면 no action.
		} else {
			fimd_CommUsageAlm(sysType, unitType, unitIndex, usage,  SFM_ALM_CRITICAL, SFM_ALM_NORMAL, 1);
			memInfo->criFlag = 1;
			memInfo->majFlag = 0;
			memInfo->minFlag = 0;
			changeFlag = 1;
		}

	} //-- end of critical condition --//

	return changeFlag;
}
/*End of fimd_checkSceMemUsageAlm2*/

/* ADD : by june, 2010-09-07
 * DSCRIPTION:
 * LINE:
 * TODO:
 */
int fimd_checkSceDiskUsageAlm (int sysIndex, int diskIndex)
{
	Threshold_st    thresh;
	SCE_USAGE_PARAM param;
	int status;

	param.sysIndex = sysIndex;
	param.devKind  = SCE_DISK; 
	param.devIndex = diskIndex;
	param.usage = g_pstSCEInfo->SCEDev[sysIndex].diskInfo.usage;

	param.curStatus = 0;
	param.preStatus = 0; 
	param.occurFlag = 0; 
	param.changeFlag = 0;

	thresh.minLimit =  g_pstSCEInfo->SCEDev[sysIndex].diskInfo.minLimit;
	thresh.majLimit =  g_pstSCEInfo->SCEDev[sysIndex].diskInfo.majLimit;
	thresh.criLimit =  g_pstSCEInfo->SCEDev[sysIndex].diskInfo.criLimit;

	thresh.minFlag =  g_pstSCEInfo->SCEDev[sysIndex].diskInfo.minFlag;
	thresh.majFlag =  g_pstSCEInfo->SCEDev[sysIndex].diskInfo.majFlag;
	thresh.criFlag =  g_pstSCEInfo->SCEDev[sysIndex].diskInfo.criFlag;;

	status = fimd_thresholdCheckUsage(&thresh, &param);

	switch (status) {
	case SFM_ALM_NORMAL:
		g_pstSCEInfo->SCEDev[sysIndex].diskInfo.level = SFM_ALM_NORMAL;  // sfdb->sys에 update
		g_pstSCEInfo->SCEDev[sysIndex].diskInfo.criFlag = 0;
		g_pstSCEInfo->SCEDev[sysIndex].diskInfo.majFlag = 0;
		g_pstSCEInfo->SCEDev[sysIndex].diskInfo.minFlag = 0;
		break;

	case  SFM_ALM_MINOR:

		if (param.changeFlag == 1) {
			g_pstSCEInfo->SCEDev[sysIndex].diskInfo.level = SFM_ALM_MINOR;  // sfdb->sys에 update
			fimd_makeSceUsageAlmMsg (&param);
			fimd_saveSceUsageAlmInfo2DB (&param);
			// 장애 발생시 장애통계 count한다.
			if (param.occurFlag) {
				// 시스템별, 유형별, 등급별 구분
				//fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getSceAlarmType(param.devKind), param.curStatus);
				fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getAlarmType(param.devKind), param.curStatus);
			}
			g_pstSCEInfo->SCEDev[sysIndex].diskInfo.criFlag = 0;
			g_pstSCEInfo->SCEDev[sysIndex].diskInfo.majFlag = 0;
			g_pstSCEInfo->SCEDev[sysIndex].diskInfo.minFlag = 1;
		}
		break;

	case  SFM_ALM_MAJOR:

		if (param.changeFlag == 1) {
			g_pstSCEInfo->SCEDev[sysIndex].diskInfo.level = SFM_ALM_MAJOR;  // sfdb->sys에 update
			fimd_makeSceUsageAlmMsg (&param);
			fimd_saveSceUsageAlmInfo2DB (&param);
			if (param.occurFlag) {
				//fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getSceAlarmType(param.devKind), param.curStatus);
				fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getAlarmType(param.devKind), param.curStatus);
			}
			g_pstSCEInfo->SCEDev[sysIndex].diskInfo.criFlag = 0;
			g_pstSCEInfo->SCEDev[sysIndex].diskInfo.majFlag = 1;
			g_pstSCEInfo->SCEDev[sysIndex].diskInfo.minFlag = 0;
		}
		break;

	default:        // CRITICAL

		if (param.changeFlag == 1) {
			g_pstSCEInfo->SCEDev[sysIndex].diskInfo.level = SFM_ALM_CRITICAL;  // sfdb->sys에 update
			fimd_makeSceUsageAlmMsg (&param);
			fimd_saveSceUsageAlmInfo2DB (&param);
			if (param.occurFlag) {
				//fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getSceAlarmType(param.devKind), param.curStatus);
				fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getAlarmType(param.devKind), param.curStatus);
			}
			g_pstSCEInfo->SCEDev[sysIndex].diskInfo.criFlag = 1;
			g_pstSCEInfo->SCEDev[sysIndex].diskInfo.majFlag = 0;
			g_pstSCEInfo->SCEDev[sysIndex].diskInfo.minFlag = 0;
		}
		break;
	}

	return param.changeFlag;
}


/* hjjung_20100823 */
#if 1
int fimd_checkSceUserUsageAlm (int sysIndex, int userIndex)
{
	Threshold_st_int   thresh;
	SCE_USAGE_PARAM 	param;
	int status;

	param.sysIndex = sysIndex;
	param.devKind  = SCE_USER; 
	param.devIndex = userIndex;
	param.usage = g_pstSCEInfo->SCEDev[sysIndex].userInfo.num;

	param.curStatus = 0;
	param.preStatus = 0; 
	param.occurFlag = 0; 
	param.changeFlag = 0;

	thresh.minLimit =  g_pstSCEInfo->SCEDev[sysIndex].userInfo.minLimit;
	thresh.majLimit =  g_pstSCEInfo->SCEDev[sysIndex].userInfo.majLimit;
	thresh.criLimit =  g_pstSCEInfo->SCEDev[sysIndex].userInfo.criLimit;

	thresh.minFlag =  g_pstSCEInfo->SCEDev[sysIndex].userInfo.minFlag;
	thresh.majFlag =  g_pstSCEInfo->SCEDev[sysIndex].userInfo.majFlag;
	thresh.criFlag =  g_pstSCEInfo->SCEDev[sysIndex].userInfo.criFlag;;

	status = fimd_thresholdCheckSceUserUsage(&thresh, &param);

	switch (status) {
	case SFM_ALM_NORMAL:
		g_pstSCEInfo->SCEDev[sysIndex].userInfo.level = SFM_ALM_NORMAL;  // sfdb->sys에 update
		g_pstSCEInfo->SCEDev[sysIndex].userInfo.criFlag = 0;
		g_pstSCEInfo->SCEDev[sysIndex].userInfo.majFlag = 0;
		g_pstSCEInfo->SCEDev[sysIndex].userInfo.minFlag = 0;
		break;

	case  SFM_ALM_MINOR:
		//g_pstSCEInfo->SCEDev[sysIndex].userInfo.level = SFM_ALM_MINOR;  // sfdb->sys에 update

		if (param.changeFlag == 1) {
			if (sceUserChkBuff[sysIndex].minStartTime == 0) // 처음 limit를 넘었는가?
				sceUserChkBuff[sysIndex].minStartTime = currentTime;
			if (g_pstSCEInfo->SCEDev[sysIndex].userInfo.minDurat > (currentTime - sceUserChkBuff[sysIndex].minStartTime)) {
				; // 지속시간 경과하지 않았으면 no action.
			} else {
				g_pstSCEInfo->SCEDev[sysIndex].userInfo.level = SFM_ALM_MINOR;  // sfdb->sys에 update
				fimd_makeSceUsageAlmMsg (&param);
				fimd_saveSceUsageAlmInfo2DB (&param);
				// 장애 발생시 장애통계 count한다.
				if (param.occurFlag) {
					// 시스템별, 유형별, 등급별 구분
					fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getAlarmType(param.devKind), param.curStatus);
				}
				g_pstSCEInfo->SCEDev[sysIndex].userInfo.criFlag = 0;
				g_pstSCEInfo->SCEDev[sysIndex].userInfo.majFlag = 0;
				g_pstSCEInfo->SCEDev[sysIndex].userInfo.minFlag = 1;
			}
		}
		break;

	case  SFM_ALM_MAJOR:
		//g_pstSCEInfo->SCEDev[sysIndex].userInfo.level = SFM_ALM_MAJOR;  // sfdb->sys에 update

		if (param.changeFlag == 1) {
			// minor에서 상승하는게 아니라 한방에 major로 올라갈 수 있으므로
			//  minStartTime을 같이 변경해야한다.
			if (sceUserChkBuff[sysIndex].minStartTime == 0) // 처음 limit를 넘었는가?
				sceUserChkBuff[sysIndex].minStartTime = currentTime;
			if (sceUserChkBuff[sysIndex].majStartTime == 0) // 처음 limit를 넘었는가?
				sceUserChkBuff[sysIndex].majStartTime = currentTime;

			if (g_pstSCEInfo->SCEDev[sysIndex].userInfo.majDurat > (currentTime - sceUserChkBuff[sysIndex].majStartTime)) {
				; // 지속시간 경과하지 않았으면 no action.
			} else {
				g_pstSCEInfo->SCEDev[sysIndex].userInfo.level = SFM_ALM_MAJOR;  // sfdb->sys에 update
				fimd_makeSceUsageAlmMsg (&param);
				fimd_saveSceUsageAlmInfo2DB (&param);
				if (param.occurFlag) {
					fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getAlarmType(param.devKind), param.curStatus);
				}
				g_pstSCEInfo->SCEDev[sysIndex].userInfo.criFlag = 0;
				g_pstSCEInfo->SCEDev[sysIndex].userInfo.majFlag = 1;
				g_pstSCEInfo->SCEDev[sysIndex].userInfo.minFlag = 0;
			}
		}
		break;

	default:        // CRITICAL
		//g_pstSCEInfo->SCEDev[sysIndex].userInfo.level = SFM_ALM_CRITICAL;  // sfdb->sys에 update

		if (param.changeFlag == 1) {
			// minor,major에서 상승하는게 아니라 한방에 critical로 올라갈 수 있으므로
			//  minStartTime, majorStartTime을 같이 변경해야한다.
			if (sceUserChkBuff[sysIndex].minStartTime == 0) // 처음 limit를 넘었는가?
				sceUserChkBuff[sysIndex].minStartTime = currentTime;
			if (sceUserChkBuff[sysIndex].majStartTime == 0) // 처음 limit를 넘었는가?
				sceUserChkBuff[sysIndex].majStartTime = currentTime;
			if (sceUserChkBuff[sysIndex].criStartTime == 0) // 처음 limit를 넘었는가?
				sceUserChkBuff[sysIndex].criStartTime = currentTime;

			if (g_pstSCEInfo->SCEDev[sysIndex].userInfo.criDurat > (currentTime - sceUserChkBuff[sysIndex].criStartTime)) {
				;
			} else {
				g_pstSCEInfo->SCEDev[sysIndex].userInfo.level = SFM_ALM_CRITICAL;  // sfdb->sys에 update
				fimd_makeSceUsageAlmMsg (&param);
				fimd_saveSceUsageAlmInfo2DB (&param);
				if (param.occurFlag) {
					fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getAlarmType(param.devKind), param.curStatus);
				}
				g_pstSCEInfo->SCEDev[sysIndex].userInfo.criFlag = 1;
				g_pstSCEInfo->SCEDev[sysIndex].userInfo.majFlag = 0;
				g_pstSCEInfo->SCEDev[sysIndex].userInfo.minFlag = 0;
			}
		}
		break;
	}

	return param.changeFlag;
}
#endif


/* hjjung_20100822 */
int fimd_checkLegSessionUsageAlm (int sysIndex)
{
	Threshold_st_int    thresh;
	LEG_USAGE_PARAM param;
	int status;

	param.sysIndex = sysIndex;
	param.devKind  = LEG_SESSION;
	param.devIndex = 0;
	param.usage = g_pstCALLInfo->legInfo[sysIndex].num;

	param.curStatus = 0;
	param.preStatus = 0; 
	param.occurFlag = 0; 
	param.changeFlag = 0;

	thresh.minLimit =  g_pstCALLInfo->legInfo[sysIndex].minLimit;
	thresh.majLimit =  g_pstCALLInfo->legInfo[sysIndex].majLimit;
	thresh.criLimit =  g_pstCALLInfo->legInfo[sysIndex].criLimit;

	thresh.minFlag =  g_pstCALLInfo->legInfo[sysIndex].minFlag;
	thresh.majFlag =  g_pstCALLInfo->legInfo[sysIndex].majFlag;
	thresh.criFlag =  g_pstCALLInfo->legInfo[sysIndex].criFlag;;

	status = fimd_thresholdCheckLegUsage(&thresh, &param);

	switch (status) {
	case SFM_ALM_NORMAL:
		g_pstCALLInfo->legInfo[sysIndex].level = SFM_ALM_NORMAL;  // sfdb->sys에 update
		g_pstCALLInfo->legInfo[sysIndex].criFlag = 0;
		g_pstCALLInfo->legInfo[sysIndex].majFlag = 0;
		g_pstCALLInfo->legInfo[sysIndex].minFlag = 0;
		break;

	case  SFM_ALM_MINOR:
		//g_pstLEGInfo->legInfo[sysIndex].level = SFM_ALM_MINOR;  // sfdb->sys에 update

		if (param.changeFlag == 1) {
			if (legSessionChkBuff[sysIndex].minStartTime == 0) // 처음 limit를 넘었는가?
				legSessionChkBuff[sysIndex].minStartTime = currentTime;
			if (g_pstCALLInfo->legInfo[sysIndex].minDurat > (currentTime - legSessionChkBuff[sysIndex].minStartTime)) {
				; // 지속시간 경과하지 않았으면 no action.
			} else {
				g_pstCALLInfo->legInfo[sysIndex].level = SFM_ALM_MINOR;  // sfdb->sys에 update
				fimd_makeLegUsageAlmMsg (&param);
				fimd_saveLegUsageAlmInfo2DB (&param);
				// 장애 발생시 장애통계 count한다.
				if (param.occurFlag) {
					// 시스템별, 유형별, 등급별 구분
					fimd_increaseAlmStat(sysIndex+1, fimd_getAlarmType(param.devKind), param.curStatus);
				}
				g_pstCALLInfo->legInfo[sysIndex].criFlag = 0;
				g_pstCALLInfo->legInfo[sysIndex].majFlag = 0;
				g_pstCALLInfo->legInfo[sysIndex].minFlag = 1;
			}
		}
		break;

	case  SFM_ALM_MAJOR:
		//g_pstLEGInfo->legInfo[sysIndex].level = SFM_ALM_MAJOR;  // sfdb->sys에 update

		if (param.changeFlag == 1) {
			// minor에서 상승하는게 아니라 한방에 major로 올라갈 수 있으므로
			//  minStartTime을 같이 변경해야한다.
			if (legSessionChkBuff[sysIndex].minStartTime == 0) // 처음 limit를 넘었는가?
				legSessionChkBuff[sysIndex].minStartTime = currentTime;
			if (legSessionChkBuff[sysIndex].majStartTime == 0) // 처음 limit를 넘었는가?
				legSessionChkBuff[sysIndex].majStartTime = currentTime;

			if (g_pstCALLInfo->legInfo[sysIndex].majDurat > (currentTime - legSessionChkBuff[sysIndex].majStartTime)) {
				; // 지속시간 경과하지 않았으면 no action.
			} else {
				g_pstCALLInfo->legInfo[sysIndex].level = SFM_ALM_MAJOR;  // sfdb->sys에 update
				fimd_makeLegUsageAlmMsg (&param);
				fimd_saveLegUsageAlmInfo2DB (&param);
				if (param.occurFlag) {
					fimd_increaseAlmStat(sysIndex+1, fimd_getAlarmType(param.devKind), param.curStatus);
				}
				g_pstCALLInfo->legInfo[sysIndex].criFlag = 0;
				g_pstCALLInfo->legInfo[sysIndex].majFlag = 1;
				g_pstCALLInfo->legInfo[sysIndex].minFlag = 0;
			}
		}
		break;

	default:        // CRITICAL
		//g_pstLEGInfo->legInfo[sysIndex].level = SFM_ALM_CRITICAL;  // sfdb->sys에 update

		if (param.changeFlag == 1) {
			// minor,major에서 상승하는게 아니라 한방에 critical로 올라갈 수 있으므로
			//  minStartTime, majorStartTime을 같이 변경해야한다.
			if (legSessionChkBuff[sysIndex].minStartTime == 0) // 처음 limit를 넘었는가?
				legSessionChkBuff[sysIndex].minStartTime = currentTime;
			if (legSessionChkBuff[sysIndex].majStartTime == 0) // 처음 limit를 넘었는가?
				legSessionChkBuff[sysIndex].majStartTime = currentTime;
			if (legSessionChkBuff[sysIndex].criStartTime == 0) // 처음 limit를 넘었는가?
				legSessionChkBuff[sysIndex].criStartTime = currentTime;

			if (g_pstCALLInfo->legInfo[sysIndex].criDurat > (currentTime - legSessionChkBuff[sysIndex].criStartTime)) {
				;
			} else {
				g_pstCALLInfo->legInfo[sysIndex].level = SFM_ALM_CRITICAL;  // sfdb->sys에 update
				fimd_makeLegUsageAlmMsg (&param);
				fimd_saveLegUsageAlmInfo2DB (&param);
				if (param.occurFlag) {
					fimd_increaseAlmStat(sysIndex+1, fimd_getAlarmType(param.devKind), param.curStatus);
				}
				g_pstCALLInfo->legInfo[sysIndex].criFlag = 1;
				g_pstCALLInfo->legInfo[sysIndex].majFlag = 0;
				g_pstCALLInfo->legInfo[sysIndex].minFlag = 0;
			}
		}
		break;
	}

	return param.changeFlag;
}
/* added by dcham 20110515 for TPS*/
int fimd_checkTpsAlm (int sysIndex)
{
	Threshold_st_tps    thresh;
	TPS_PARAM param;
	int status;

	param.sysIndex = sysIndex;
	param.devKind  = TPS_LOAD;
	param.devIndex = 0;
	param.usage = g_pstCALLInfo->tpsInfo[sysIndex].num/UNIT_TPS_VALUE_5;

	param.curStatus = 0;
	param.preStatus = 0; 
	param.occurFlag = 0; 
	param.changeFlag = 0;

	thresh.minLimit =  g_pstCALLInfo->tpsInfo[sysIndex].minLimit;
	thresh.majLimit =  g_pstCALLInfo->tpsInfo[sysIndex].majLimit;
	thresh.criLimit =  g_pstCALLInfo->tpsInfo[sysIndex].criLimit;

	thresh.minFlag =  g_pstCALLInfo->tpsInfo[sysIndex].minFlag;
	thresh.majFlag =  g_pstCALLInfo->tpsInfo[sysIndex].majFlag;
	thresh.criFlag =  g_pstCALLInfo->tpsInfo[sysIndex].criFlag;;

	status = fimd_thresholdCheckLegTps(&thresh, &param);

	switch (status) {
	case SFM_ALM_NORMAL:
		g_pstCALLInfo->tpsInfo[sysIndex].level = SFM_ALM_NORMAL;  // sfdb->sys에 update
		g_pstCALLInfo->tpsInfo[sysIndex].criFlag = 0;
		g_pstCALLInfo->tpsInfo[sysIndex].majFlag = 0;
		g_pstCALLInfo->tpsInfo[sysIndex].minFlag = 0;
		break;

	case  SFM_ALM_MINOR:
		if (param.changeFlag == 1) {
			if (legTpsChkBuff[sysIndex].minStartTime == 0) // 처음 limit를 넘었는가?
				legTpsChkBuff[sysIndex].minStartTime = currentTime;
			if (g_pstCALLInfo->tpsInfo[sysIndex].minDurat > (currentTime - legTpsChkBuff[sysIndex].minStartTime)) {
				; // 지속시간 경과하지 않았으면 no action.
			} else {
				g_pstCALLInfo->tpsInfo[sysIndex].level = SFM_ALM_MINOR;  // sfdb->sys에 update
				fimd_makeLegTpsAlmMsg (&param);
				fimd_saveLegTpsAlmInfo2DB (&param);
				// 장애 발생시 장애통계 count한다.
				if (param.occurFlag) {
					// 시스템별, 유형별, 등급별 구분
					fimd_increaseAlmStat(sysIndex+1, fimd_getAlarmType(param.devKind), param.curStatus);
				}
				g_pstCALLInfo->tpsInfo[sysIndex].criFlag = 0;
				g_pstCALLInfo->tpsInfo[sysIndex].majFlag = 0;
				g_pstCALLInfo->tpsInfo[sysIndex].minFlag = 1;
			}
		}
		break;

	case  SFM_ALM_MAJOR:
		//g_pstLEGInfo->legInfo[sysIndex].level = SFM_ALM_MAJOR;  // sfdb->sys에 update

		if (param.changeFlag == 1) {
			// minor에서 상승하는게 아니라 한방에 major로 올라갈 수 있으므로
			//  minStartTime을 같이 변경해야한다.
			if (legTpsChkBuff[sysIndex].minStartTime == 0) // 처음 limit를 넘었는가?
				legTpsChkBuff[sysIndex].minStartTime = currentTime;
			if (legTpsChkBuff[sysIndex].majStartTime == 0) // 처음 limit를 넘었는가?
				legTpsChkBuff[sysIndex].majStartTime = currentTime;

			if (g_pstCALLInfo->tpsInfo[sysIndex].majDurat > (currentTime - legTpsChkBuff[sysIndex].majStartTime)) {
				; // 지속시간 경과하지 않았으면 no action.
			} else {
				g_pstCALLInfo->tpsInfo[sysIndex].level = SFM_ALM_MAJOR;  // sfdb->sys에 update
				fimd_makeLegTpsAlmMsg (&param);
				fimd_saveLegTpsAlmInfo2DB (&param);
				if (param.occurFlag) {
					fimd_increaseAlmStat(sysIndex+1, fimd_getAlarmType(param.devKind), param.curStatus);
				}
				g_pstCALLInfo->tpsInfo[sysIndex].criFlag = 0;
				g_pstCALLInfo->tpsInfo[sysIndex].majFlag = 1;
				g_pstCALLInfo->tpsInfo[sysIndex].minFlag = 0;
			}
		}
		break;

	default:        // CRITICAL
		//g_pstLEGInfo->legInfo[sysIndex].level = SFM_ALM_CRITICAL;  // sfdb->sys에 update

		if (param.changeFlag == 1) {
			// minor,major에서 상승하는게 아니라 한방에 critical로 올라갈 수 있으므로
			//  minStartTime, majorStartTime을 같이 변경해야한다.
			if (legTpsChkBuff[sysIndex].minStartTime == 0) // 처음 limit를 넘었는가?
				legTpsChkBuff[sysIndex].minStartTime = currentTime;
			if (legTpsChkBuff[sysIndex].majStartTime == 0) // 처음 limit를 넘었는가?
				legTpsChkBuff[sysIndex].majStartTime = currentTime;
			if (legTpsChkBuff[sysIndex].criStartTime == 0) // 처음 limit를 넘었는가?
				legTpsChkBuff[sysIndex].criStartTime = currentTime;

			if (g_pstCALLInfo->tpsInfo[sysIndex].criDurat > (currentTime - legTpsChkBuff[sysIndex].criStartTime)) {
				;
			} else {
				g_pstCALLInfo->tpsInfo[sysIndex].level = SFM_ALM_CRITICAL;  // sfdb->sys에 update
				fimd_makeLegTpsAlmMsg (&param);
				fimd_saveLegTpsAlmInfo2DB (&param);
				if (param.occurFlag) {
					fimd_increaseAlmStat(sysIndex+1, fimd_getAlarmType(param.devKind), param.curStatus);
				}
				g_pstCALLInfo->tpsInfo[sysIndex].criFlag = 1;
				g_pstCALLInfo->tpsInfo[sysIndex].majFlag = 0;
				g_pstCALLInfo->tpsInfo[sysIndex].minFlag = 0;
			}
		}
		break;
	}

	return param.changeFlag;
}
/*
	by sjjeon (09.6) - 재정의....
	SCE Disk Usage 상태 관리.
*/
int fimd_checkSceDiskUsageAlm2 (int sysType,int unitType)
{
	int  usage=0, changeFlag=0, sysIndex=0;
	SCE_SYS_USAGE_INFO     *diskInfo;

	if(sysType == SCEA) sysIndex = 0;
	else if(sysType == SCEB) sysIndex =1;
	else {
		sprintf(trcBuf,"[%s] UNKNOWN SYSTYPE[%d]\n",__FUNCTION__,sysType);
        trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	diskInfo = &g_pstSCEInfo->SCEDev[sysIndex].diskInfo;
	usage = diskInfo->usage;

	// minor 장애 기준값보다 낮은 경우, 
	// - 이전에 장애가 발생되어 있었으면 장애가 해지된 경우이므로,
	//  발생되었던 각각 해당 등급에 대한 해지 메시지를 만들어 cond로 보낸다.
	//  
	if (usage < diskInfo->minLimit)
	{   
		diskInfo->level = SFM_ALM_NORMAL;  // g_pstSCEInfo->SCEDev 에 update
		if (diskInfo->criFlag) {
			fimd_CommUsageAlm(sysType, unitType, 0, usage, SFM_ALM_NORMAL, SFM_ALM_CRITICAL, 0);
			changeFlag = 1; 
		}
		if (diskInfo->majFlag){
			fimd_CommUsageAlm(sysType, unitType, 0, usage, SFM_ALM_NORMAL, SFM_ALM_MAJOR, 0);
			changeFlag = 1;
		}
		if (diskInfo->minFlag){
			fimd_CommUsageAlm(sysType, unitType, 0, usage, SFM_ALM_NORMAL, SFM_ALM_MINOR, 0);
			changeFlag = 1;
		}

		diskInfo->criFlag = 0;
		diskInfo->majFlag = 0;
		diskInfo->minFlag = 0;

	} //-- end of normal --//

	// minor보다 높고 major 기준값보다 낮은 경우, (minor 장애 조건)
	// - 더 높은 등급(major, critical)이 발생되어 있었으면, 등급 하강 조건이므로,
	//  각각 해지 메시지를 만든다.
	// - minor가 이미 발생되어 있으면 no action.
	// - disk 사용율 장애는 지속시간이 없으므로 곧바로 장애 판단.
	//
	else if (usage < diskInfo->majLimit)
	{
		diskInfo->level = SFM_ALM_MINOR;  // g_pstSCEInfo->SCEDev 에 update
		if (diskInfo->criFlag) {
			fimd_CommUsageAlm(sysType, unitType, 0, usage, SFM_ALM_MINOR, SFM_ALM_CRITICAL, 0);
			changeFlag = 1;
		}

		if (diskInfo->majFlag){
			fimd_CommUsageAlm(sysType, unitType, 0, usage, SFM_ALM_MINOR, SFM_ALM_MAJOR, 0);
			changeFlag = 1;
		}

		if (diskInfo->minFlag){
			; // 이미 minor가 발생되어 있으면 no action.
		} else {
			fimd_CommUsageAlm(sysType, unitType, 0, usage, SFM_ALM_MINOR, SFM_ALM_MAJOR, 1);
			diskInfo->criFlag = 0;
			diskInfo->majFlag = 0;
			diskInfo->minFlag = 1;
			changeFlag = 1;
		}

	} //-- end of minor condition --//

	// major보다 높고 critical 기준값보다 낮은 경우, (major 장애 조건)
	// - 더 높은 등급(critical)이 발생되어 있었으면, 등급 하강 조건이므로 해지 메시지를 만든다.
	// - major가 이미 발생되어 있으면 no action.
	//
	else if (usage < diskInfo->criLimit)
	{
		diskInfo->level = SFM_ALM_MAJOR;  // g_pstSCEInfo->SCEDev 에 update

		if (diskInfo->minFlag){
			fimd_CommUsageAlm(sysType, unitType, 0, usage, SFM_ALM_MAJOR, SFM_ALM_MINOR, 0);
			diskInfo->minFlag = 0;
			changeFlag = 1;
		}

		if (diskInfo->criFlag){
			fimd_CommUsageAlm(sysType, unitType, 0, usage, SFM_ALM_MAJOR, SFM_ALM_CRITICAL, 0);
			diskInfo->criFlag = 0;
			changeFlag = 1;
		}

		if (diskInfo->majFlag) {
			; // 이미 major가 발생되어 있으면 no action.
		} else {
			fimd_CommUsageAlm(sysType, unitType, 0, usage, SFM_ALM_MAJOR, SFM_ALM_CRITICAL, 1);
			diskInfo->criFlag = 0;
			diskInfo->majFlag = 1;
			diskInfo->minFlag = 0;
			changeFlag = 1;
		}

	} //-- end of major condition --//

	// critical 기준값보다 높은 경우, (critical 장애 조건)
	// - critical이 이미 발생되어 있으면 no action.
	// - critical 기준값 지속시간 경과여부를 확인하여 장애 판단.
	//
	else
	{
		diskInfo->level = SFM_ALM_CRITICAL;  // g_pstSCEInfo->SCEDev 에 update
/*
		if(diskInfo->minFlag){
			fimd_CommUsageAlm(sysType, unitType, 0, usage, SFM_ALM_CRITICAL, SFM_ALM_MINOR, 1);
			diskInfo->minFlag = 0;
			changeFlag = 1;
		}

		if (diskInfo->majFlag){
			fimd_CommUsageAlm(sysType, unitType, 0, usage, SFM_ALM_CRITICAL, SFM_ALM_MAJOR, 1);
			diskInfo->majFlag = 0;
			changeFlag = 1;
		}
*/
		if (diskInfo->criFlag) {
			; // 이미 발생되어 있으면 no action.
		} else {
			fimd_CommUsageAlm(sysType, unitType, 0, usage,  SFM_ALM_CRITICAL, SFM_ALM_NORMAL, 1);
			diskInfo->criFlag = 1;
			diskInfo->majFlag = 0;
			diskInfo->minFlag = 0;
			changeFlag = 1;
		}

	} //-- end of critical condition --//

	return changeFlag;
}
/* End of fimd_checkSceDiskUsageAlm2 */


#if 0 /* by june */
int fimd_checkScePowerStatusAlm (int sysIndex, int pwrIndex)
{
	SCE_USAGE_PARAM param;
	int status;

	param.sysIndex = sysIndex;
	param.devKind  = SCE_POWER; 
	param.devIndex = pwrIndex;
	param.usage    = g_pstSCEInfo->SCEDev[sysIndex].pwrStatus.status;
	param.curStatus = 0;

	// sjjeon : preStatus 값을 0 으로 두면 지속적으로 올라간다. error..
	//param.preStatus = 0; 
	param.preStatus = g_pstSCEInfo->SCEDev[sysIndex].pwrStatus.preStatus;
	param.occurFlag = 0; 
	param.changeFlag = 0;

	status = fimd_thresholdCheckStatus(&param);
	
//	g_pstSCEInfo->SCEDev[sysIndex].pwrStatus.level = SFM_ALM_NORMAL;  // sfdb->sys에 update
	if (param.changeFlag == 1){
		fimd_makeSceUsageAlmMsg (&param);
 		fimd_saveSceUsageAlmInfo2DB (&param);
		// 장애 발생시 장애통계 count한다.
		if (param.occurFlag) {
			// 시스템별, 유형별, 등급별 구분
			//fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getSceAlarmType(param.devKind), param.curStatus);
			fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getAlarmType(param.devKind), param.curStatus);
		}
	}
	g_pstSCEInfo->SCEDev[sysIndex].pwrStatus.preStatus = param.preStatus;

#if 1
	switch (status) {
	case SFM_ALM_NORMAL:
		g_pstSCEInfo->SCEDev[sysIndex].pwrStatus.level = SFM_ALM_NORMAL;  // sfdb->sys에 update
		if (param.changeFlag == 1){
			fimd_makeSceUsageAlmMsg (&param);
 			fimd_saveSceUsageAlmInfo2DB (&param);
			// 장애 발생시 장애통계 count한다.
			if (param.occurFlag) {
				// 시스템별, 유형별, 등급별 구분
				//fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getSceAlarmType(param.devKind), param.curStatus);
				fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getAlarmType(param.devKind), param.curStatus);
			}
		}
		g_pstSCEInfo->SCEDev[sysIndex].pwrStatus.preStatus = param.preStatus;
		break;


	default:        // CRITICAL
		g_pstSCEInfo->SCEDev[sysIndex].pwrStatus.level = SFM_ALM_CRITICAL;  // sfdb->sys에 update

		if (param.changeFlag == 1) {
			fimd_makeSceUsageAlmMsg (&param);
			fimd_saveSceUsageAlmInfo2DB (&param);
			if (param.occurFlag) {
				//fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getSceAlarmType(param.devKind), param.curStatus);
				fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getAlarmType(param.devKind), param.curStatus);
			}
			// minor,major에서 상승하는게 아니라 한방에 critical로 올라갈 수 있으므로
			//  minStartTime, majorStartTime을 같이 변경해야한다.
			if (scePwrChkBuff[sysIndex].minStartTime == 0) // 처음 limit를 넘었는가?
				scePwrChkBuff[sysIndex].minStartTime = currentTime;
			if (scePwrChkBuff[sysIndex].majStartTime == 0) // 처음 limit를 넘었는가?
				scePwrChkBuff[sysIndex].majStartTime = currentTime;
			if (scePwrChkBuff[sysIndex].criStartTime == 0) // 처음 limit를 넘었는가?
				scePwrChkBuff[sysIndex].criStartTime = currentTime;

			if (g_pstSCEInfo->SCEDev[sysIndex].pwrStatus.criDurat > (currentTime - scePwrChkBuff[sysIndex].criStartTime)) {
				;
			} else {
				g_pstSCEInfo->SCEDev[sysIndex].pwrStatus.level = SFM_ALM_CRITICAL;  // sfdb->sys에 update
				fimd_makeSceUsageAlmMsg (&param);
				fimd_saveSceUsageAlmInfo2DB (&param);
				if (param.occurFlag) {
					//fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getSceAlarmType(param.devKind), param.curStatus);
					fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getAlarmType(param.devKind), param.curStatus);
				}
				g_pstSCEInfo->SCEDev[sysIndex].pwrStatus.preStatus = param.preStatus;
			}
		}
		break;
	}
#endif
	return param.changeFlag;
}
#endif

#if 0 /* by june */
int fimd_checkSceFanStatusAlm (int sysIndex, int fanIndex)
{
	SCE_USAGE_PARAM param;
	int status;

	param.sysIndex  = sysIndex;
	param.devKind   = SCE_FAN; 
	param.devIndex  = fanIndex;
	param.usage     = g_pstSCEInfo->SCEDev[sysIndex].fanStatus.status;
	param.curStatus = SCE_DEV_STATUS_NORMAL;
	param.preStatus = g_pstSCEInfo->SCEDev[sysIndex].fanStatus.preStatus; 
	param.occurFlag = 0; 
	param.changeFlag = 0;

	status = fimd_thresholdCheckStatus(&param);
#if 1
	switch (status) {
	case SFM_ALM_NORMAL:
		sceFanChkBuff[sysIndex].minStartTime = 0;
		sceFanChkBuff[sysIndex].majStartTime = 0;
		sceFanChkBuff[sysIndex].criStartTime = 0;
		g_pstSCEInfo->SCEDev[sysIndex].fanStatus.level = SFM_ALM_NORMAL;  // sfdb->sys에 update
		if (param.changeFlag == 1){
			fimd_makeSceUsageAlmMsg (&param);
 			fimd_saveSceUsageAlmInfo2DB (&param);
			// 장애 발생시 장애통계 count한다.
			if (param.occurFlag) {
				// 시스템별, 유형별, 등급별 구분
				//fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getSceAlarmType(param.devKind), param.curStatus);
				fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getAlarmType(param.devKind), param.curStatus);
			}
		}
		g_pstSCEInfo->SCEDev[sysIndex].fanStatus.preStatus = param.preStatus;
		break;

	case  SFM_ALM_MINOR:
		sceFanChkBuff[sysIndex].majStartTime = 0;
		sceFanChkBuff[sysIndex].criStartTime = 0;

		g_pstSCEInfo->SCEDev[sysIndex].fanStatus.level = SFM_ALM_MINOR;  // sfdb->sys에 update

		if (param.changeFlag == 1) {
			fimd_makeSceUsageAlmMsg (&param);
 			fimd_saveSceUsageAlmInfo2DB (&param);
			// 장애 발생시 장애통계 count한다.
			if (param.occurFlag) {
				// 시스템별, 유형별, 등급별 구분
				//fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getSceAlarmType(param.devKind), param.curStatus);
				fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getAlarmType(param.devKind), param.curStatus);
			}

			if (g_pstSCEInfo->SCEDev[sysIndex].fanStatus.minDurat > (currentTime - sceFanChkBuff[sysIndex].minStartTime)) {
				; // 지속시간 경과하지 않았으면 no action.
			} else {
				g_pstSCEInfo->SCEDev[sysIndex].fanStatus.level = SFM_ALM_MINOR;  // sfdb->sys에 update
				fimd_makeSceUsageAlmMsg (&param);
				fimd_saveSceUsageAlmInfo2DB (&param);
				// 장애 발생시 장애통계 count한다.
				if (param.occurFlag) {
					// 시스템별, 유형별, 등급별 구분
					//fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getSceAlarmType(param.devKind), param.curStatus);
					fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getAlarmType(param.devKind), param.curStatus);
				}
				g_pstSCEInfo->SCEDev[sysIndex].fanStatus.preStatus = param.preStatus;
			}
		}

		break;

	case  SFM_ALM_MAJOR:
		sceFanChkBuff[sysIndex].criStartTime = 0;

		g_pstSCEInfo->SCEDev[sysIndex].fanStatus.level = SFM_ALM_MAJOR;  // sfdb->sys에 update

		if (param.changeFlag == 1) {
			fimd_makeSceUsageAlmMsg (&param);
			fimd_saveSceUsageAlmInfo2DB (&param);
			if (param.occurFlag) {
				//fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getSceAlarmType(param.devKind), param.curStatus);
				fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getAlarmType(param.devKind), param.curStatus);
			}

			// minor에서 상승하는게 아니라 한방에 major로 올라갈 수 있으므로
			//  minStartTime을 같이 변경해야한다.
			if (sceFanChkBuff[sysIndex].minStartTime == 0) // 처음 limit를 넘었는가?
				sceFanChkBuff[sysIndex].minStartTime = currentTime;
			if (sceFanChkBuff[sysIndex].majStartTime == 0) // 처음 limit를 넘었는가?
				sceFanChkBuff[sysIndex].majStartTime = currentTime;

			if (g_pstSCEInfo->SCEDev[sysIndex].fanStatus.majDurat > (currentTime - sceFanChkBuff[sysIndex].majStartTime)) {
				; // 지속시간 경과하지 않았으면 no action.
			} else {
				g_pstSCEInfo->SCEDev[sysIndex].fanStatus.level = SFM_ALM_MINOR;  // sfdb->sys에 update
				fimd_makeSceUsageAlmMsg (&param);
				fimd_saveSceUsageAlmInfo2DB (&param);
				if (param.occurFlag) {
					//fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getSceAlarmType(param.devKind), param.curStatus);
					fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getAlarmType(param.devKind), param.curStatus);
				}
				g_pstSCEInfo->SCEDev[sysIndex].fanStatus.preStatus = param.preStatus;
			}
		}
		break;

	default:        // CRITICAL
		g_pstSCEInfo->SCEDev[sysIndex].fanStatus.level = SFM_ALM_CRITICAL;  // sfdb->sys에 update

		if (param.changeFlag == 1) {
			fimd_makeSceUsageAlmMsg (&param);
			fimd_saveSceUsageAlmInfo2DB (&param);
			if (param.occurFlag) {
				//fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getSceAlarmType(param.devKind), param.curStatus);
				fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getAlarmType(param.devKind), param.curStatus);
			}
			// minor,major에서 상승하는게 아니라 한방에 critical로 올라갈 수 있으므로
			//  minStartTime, majorStartTime을 같이 변경해야한다.
			if (sceFanChkBuff[sysIndex].minStartTime == 0) // 처음 limit를 넘었는가?
				sceFanChkBuff[sysIndex].minStartTime = currentTime;
			if (sceFanChkBuff[sysIndex].majStartTime == 0) // 처음 limit를 넘었는가?
				sceFanChkBuff[sysIndex].majStartTime = currentTime;
			if (sceFanChkBuff[sysIndex].criStartTime == 0) // 처음 limit를 넘었는가?
				sceFanChkBuff[sysIndex].criStartTime = currentTime;

			if (g_pstSCEInfo->SCEDev[sysIndex].fanStatus.criDurat > (currentTime - sceFanChkBuff[sysIndex].criStartTime)) {
				;
			} else {
				g_pstSCEInfo->SCEDev[sysIndex].fanStatus.level = SFM_ALM_CRITICAL;  // sfdb->sys에 update
				fimd_makeSceUsageAlmMsg (&param);
				fimd_saveSceUsageAlmInfo2DB (&param);
				if (param.occurFlag) {
					//fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getSceAlarmType(param.devKind), param.curStatus);
					fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getAlarmType(param.devKind), param.curStatus);
				}
				g_pstSCEInfo->SCEDev[sysIndex].fanStatus.preStatus = param.preStatus;
			}
		}
		break;
	}
#endif
	return param.changeFlag;
}
#endif

#if 0 /* by june */
int fimd_checkSceTempStatusAlm (int sysIndex, int tempIndex)
{
	SCE_USAGE_PARAM param;
	int status;

	param.sysIndex  = sysIndex;
	param.devKind   = SCE_TEMP; 
	param.devIndex  = tempIndex;
	param.usage     = g_pstSCEInfo->SCEDev[sysIndex].tempStatus.status;
	param.curStatus = SCE_DEV_STATUS_NORMAL;
	param.preStatus = g_pstSCEInfo->SCEDev[sysIndex].tempStatus.preStatus; 
	param.occurFlag = 0; 
	param.changeFlag = 0;

	status = fimd_thresholdCheckStatus(&param);
#if 1
	switch (status) {
	case SFM_ALM_NORMAL:
		sceTempChkBuff[sysIndex].minStartTime = 0;
		sceTempChkBuff[sysIndex].majStartTime = 0;
		sceTempChkBuff[sysIndex].criStartTime = 0;
		g_pstSCEInfo->SCEDev[sysIndex].tempStatus.level = SFM_ALM_NORMAL;  // sfdb->sys에 update
		if (param.changeFlag == 1){
			fimd_makeSceUsageAlmMsg (&param);
 			fimd_saveSceUsageAlmInfo2DB (&param);
			// 장애 발생시 장애통계 count한다.
			if (param.occurFlag) {
				// 시스템별, 유형별, 등급별 구분
				//fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getSceAlarmType(param.devKind), param.curStatus);
				fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getAlarmType(param.devKind), param.curStatus);
			}
		}
		g_pstSCEInfo->SCEDev[sysIndex].tempStatus.preStatus = param.preStatus;
		break;

	case  SFM_ALM_MINOR:
		sceTempChkBuff[sysIndex].majStartTime = 0;
		sceTempChkBuff[sysIndex].criStartTime = 0;

		g_pstSCEInfo->SCEDev[sysIndex].tempStatus.level = SFM_ALM_MINOR;  // sfdb->sys에 update

		if (param.changeFlag == 1) {
			fimd_makeSceUsageAlmMsg (&param);
 			fimd_saveSceUsageAlmInfo2DB (&param);
			// 장애 발생시 장애통계 count한다.
			if (param.occurFlag) {
				// 시스템별, 유형별, 등급별 구분
				//fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getSceAlarmType(param.devKind), param.curStatus);
				fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getAlarmType(param.devKind), param.curStatus);
			}

			if (g_pstSCEInfo->SCEDev[sysIndex].tempStatus.minDurat > (currentTime - sceTempChkBuff[sysIndex].minStartTime)) {
				; // 지속시간 경과하지 않았으면 no action.
			} else {
				g_pstSCEInfo->SCEDev[sysIndex].tempStatus.level = SFM_ALM_MINOR;  // sfdb->sys에 update
				fimd_makeSceUsageAlmMsg (&param);
				fimd_saveSceUsageAlmInfo2DB (&param);
				// 장애 발생시 장애통계 count한다.
				if (param.occurFlag) {
					// 시스템별, 유형별, 등급별 구분
					//fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getSceAlarmType(param.devKind), param.curStatus);
					fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getAlarmType(param.devKind), param.curStatus);
				}
				g_pstSCEInfo->SCEDev[sysIndex].tempStatus.preStatus = param.preStatus;
			}
		}

		break;

	case  SFM_ALM_MAJOR:
		sceTempChkBuff[sysIndex].criStartTime = 0;

		g_pstSCEInfo->SCEDev[sysIndex].tempStatus.level = SFM_ALM_MAJOR;  // sfdb->sys에 update

		if (param.changeFlag == 1) {
			fimd_makeSceUsageAlmMsg (&param);
			fimd_saveSceUsageAlmInfo2DB (&param);
			if (param.occurFlag) {
				//fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getSceAlarmType(param.devKind), param.curStatus);
				fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getAlarmType(param.devKind), param.curStatus);
			}

			// minor에서 상승하는게 아니라 한방에 major로 올라갈 수 있으므로
			//  minStartTime을 같이 변경해야한다.
			if (sceTempChkBuff[sysIndex].minStartTime == 0) // 처음 limit를 넘었는가?
				sceTempChkBuff[sysIndex].minStartTime = currentTime;
			if (sceTempChkBuff[sysIndex].majStartTime == 0) // 처음 limit를 넘었는가?
				sceTempChkBuff[sysIndex].majStartTime = currentTime;

			if (g_pstSCEInfo->SCEDev[sysIndex].tempStatus.majDurat > (currentTime - sceTempChkBuff[sysIndex].majStartTime)) {
				; // 지속시간 경과하지 않았으면 no action.
			} else {
				g_pstSCEInfo->SCEDev[sysIndex].tempStatus.level = SFM_ALM_MINOR;  // sfdb->sys에 update
				fimd_makeSceUsageAlmMsg (&param);
				fimd_saveSceUsageAlmInfo2DB (&param);
				if (param.occurFlag) {
					//fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getSceAlarmType(param.devKind), param.curStatus);
					fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getAlarmType(param.devKind), param.curStatus);
				}
				g_pstSCEInfo->SCEDev[sysIndex].tempStatus.preStatus = param.preStatus;
			}
		}
		break;

	default:        // CRITICAL
		g_pstSCEInfo->SCEDev[sysIndex].tempStatus.level = SFM_ALM_CRITICAL;  // sfdb->sys에 update

		if (param.changeFlag == 1) {
			fimd_makeSceUsageAlmMsg (&param);
			fimd_saveSceUsageAlmInfo2DB (&param);
			if (param.occurFlag) {
				//fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getSceAlarmType(param.devKind), param.curStatus);
				fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getAlarmType(param.devKind), param.curStatus);
			}
			// minor,major에서 상승하는게 아니라 한방에 critical로 올라갈 수 있으므로
			//  minStartTime, majorStartTime을 같이 변경해야한다.
			if (sceTempChkBuff[sysIndex].minStartTime == 0) // 처음 limit를 넘었는가?
				sceTempChkBuff[sysIndex].minStartTime = currentTime;
			if (sceTempChkBuff[sysIndex].majStartTime == 0) // 처음 limit를 넘었는가?
				sceTempChkBuff[sysIndex].majStartTime = currentTime;
			if (sceTempChkBuff[sysIndex].criStartTime == 0) // 처음 limit를 넘었는가?
				sceTempChkBuff[sysIndex].criStartTime = currentTime;

			if (g_pstSCEInfo->SCEDev[sysIndex].tempStatus.criDurat > (currentTime - sceTempChkBuff[sysIndex].criStartTime)) {
				;
			} else {
				g_pstSCEInfo->SCEDev[sysIndex].tempStatus.level = SFM_ALM_CRITICAL;  // sfdb->sys에 update
				fimd_makeSceUsageAlmMsg (&param);
				fimd_saveSceUsageAlmInfo2DB (&param);
				if (param.occurFlag) {
					//fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getSceAlarmType(param.devKind), param.curStatus);
					fimd_increaseAlmStatIndex (0, param.sysIndex, fimd_getAlarmType(param.devKind), param.curStatus);
				}
				g_pstSCEInfo->SCEDev[sysIndex].tempStatus.preStatus = param.preStatus;
			}
		}
		break;
	}
#endif
	return param.changeFlag;
}
#endif


/*
sysType : DSCM=1, SCMA, SCMB, TAPA, TAPB, SCEA, SCEB, L2SWA, L2SWB 
unitType  : ex) mem, cpu  ...
unitIndex : ex) 1th, 2nd ...
 */
int fimd_checkCommonStatusAlm(int sysType, int unitType, int unitIndex)
{


	COMM_STATUS_PARAM 	*param 	= NULL; // 내부 환경 설정체크..
	SFM_SCEDev  		*pSceDev= NULL;
	SFM_L3ProbeDev 		*pTapDev= NULL;
	SFM_L2SW			*pL2Dev=  NULL;

	SCE_SYS_STATUS_INFO *pStsInfo = NULL;
	SCE_SYS_STATUS_INFO HWInfo;

	int rv=0, sysIndex, level, almType;
	char devName[20];

	bzero(devName, sizeof(devName));

	param = (COMM_STATUS_PARAM*) malloc(sizeof(SCE_STATUS_PARAM));
	if(param==NULL){
		sprintf(trcBuf,"[%s] malloc fail \n",__FUNCTION__);
		trclib_writeLogErr (FL,trcBuf);
		rv = 0;
		goto GO_OUT;
	}

	// 시스템에 따라 상태값을 설정한다.(현재는 SCE 만 설정)
	switch(sysType)
	{
		case TAPA:		/* TAP-A */
		case TAPB:		/* TAP-B */
			if(sysType == TAPA) sysIndex = 0;
			else sysIndex = 1;

			pTapDev = (SFM_L3ProbeDev*)&l3pd->l3ProbeDev[sysIndex];
			switch(unitType)
			{
				case  TAP_PORT:
					param->status		= pTapDev->gigaLanInfo[unitIndex].status;
					param->preStatus 	= pTapDev->gigaLanInfo[unitIndex].prevStatus;

					// 구조체 크기가 달라 수동설정..(default가 sce용이라. 별도의 수동 설정이 필요)
					HWInfo.mask			= pTapDev->gigaLanInfo[unitIndex].mask;
					HWInfo.status		= pTapDev->gigaLanInfo[unitIndex].status;
					HWInfo.preStatus	= pTapDev->gigaLanInfo[unitIndex].prevStatus;
					HWInfo.level		= pTapDev->gigaLanInfo[unitIndex].level;
					pStsInfo			= &HWInfo;
					break;
				case TAP_POWER: // 20110422 by dcham
					param->status       = pTapDev->powerInfo[unitIndex].status;
					param->preStatus    = pTapDev->powerInfo[unitIndex].prevStatus;

					// 구조체 크기가 달라 수동설정..(default가 sce용이라. 별도의 수동 설정이 필요)
					HWInfo.mask         = pTapDev->powerInfo[unitIndex].mask;
					HWInfo.status       = pTapDev->powerInfo[unitIndex].status;
					HWInfo.preStatus    = pTapDev->powerInfo[unitIndex].prevStatus;
					//HWInfo.level        = pTapDev->powerInfo[unitIndex].level;
					HWInfo.level        = SFM_HW_NOT_EQUIP; 
					pStsInfo            = &HWInfo;
					break;

				default:
					sprintf(trcBuf,"[%s] UNKNOWN TAP UNIT TYPE. (%d) \n",__FUNCTION__, unitType);
					trclib_writeLogErr (FL,trcBuf);
					goto GO_OUT;
				break;
			}

			// alarm type return
			almType = fimd_getAlarmType(unitType);
			break;	


		/* L2 Switch */
		case L2SWA:		/* L2SW-A */
		case L2SWB:		/* L2SW-B */
			if(sysType == L2SWA) sysIndex = 0;
			else sysIndex = 1;

			pL2Dev = (SFM_L2SW*)&g_pstL2Dev->l2Info[sysIndex];
			switch(unitType)
			{
				case  L2SW_PORT:
					param->preStatus 	= pL2Dev->portInfo[unitIndex].prevStatus;
					param->status		= pL2Dev->portInfo[unitIndex].status;
					
					// 구조체 크기가 달라 수동설정..(default가 sce용이라. 별도의 수동 설정이 필요)
					HWInfo.mask 		= pL2Dev->portInfo[unitIndex].mask;
					HWInfo.status		= pL2Dev->portInfo[unitIndex].status;
					HWInfo.preStatus	= pL2Dev->portInfo[unitIndex].prevStatus;
					HWInfo.level		= pL2Dev->portInfo[unitIndex].level;
					pStsInfo			= &HWInfo;
				break;

				default:
					sprintf(trcBuf,"[%s] UNKNOWN L2SW UNIT TYPE. (%d) \n",__FUNCTION__, unitType);
					trclib_writeLogErr (FL,trcBuf);
					goto GO_OUT;
				break;
			}

			// alarm type return
			almType = fimd_getAlarmType(unitType);
			break;	

		case SCEA:		/* SCE-A */
		case SCEB:		/* SCE-B */

			if(sysType == SCEA) sysIndex = 0;
			else sysIndex = 1;

			pSceDev = (SFM_SCEDev*)&g_pstSCEInfo->SCEDev[sysIndex];

			switch(unitType)
			{
				case SCE_POWER:		// sce power
					param->status		= pSceDev->pwrStatus.status;
					param->preStatus 	= pSceDev->pwrStatus.preStatus; 
					pStsInfo 			= &pSceDev->pwrStatus;
					break;

				case SCE_FAN: 		// sce fan
					param->status		= pSceDev->fanStatus.status;
					param->preStatus 	= pSceDev->fanStatus.preStatus; 
					pStsInfo 			= &pSceDev->fanStatus;
					break;

				case SCE_TEMP: 		// sce temperature
					param->status		= pSceDev->tempStatus.status;
					param->preStatus 	= pSceDev->tempStatus.preStatus; 
					pStsInfo 			= &pSceDev->tempStatus;
					break;

				case SCE_VOLT: 		// sce voltage
					param->status		= pSceDev->voltStatus.status;
					param->preStatus	= pSceDev->voltStatus.preStatus; 
					pStsInfo 			= &pSceDev->voltStatus;
					break;

				case SCE_PORT_MGMT: 		// sce port
				case SCE_PORT_LINK: 		// sce port
					if(unitIndex<0 || unitIndex>=MAX_SCE_IFN_CNT){
						sprintf(trcBuf,"[%s:%d] invalid index number. (%d) \n",__FUNCTION__, __LINE__, unitIndex);
						trclib_writeLogErr (FL,trcBuf);
						rv=0; goto GO_OUT;
					}
					param->status 		= pSceDev->portStatus[unitIndex].status;
					param->preStatus 	= pSceDev->portStatus[unitIndex].preStatus; 
					pStsInfo 			= &pSceDev->portStatus[unitIndex];
					break;

				case SCE_STATUS:
					param->status		= pSceDev->sysStatus.status;
					param->preStatus	= pSceDev->sysStatus.preStatus; 
					pStsInfo 			= &pSceDev->sysStatus;
					break;

				case SCE_RDR_CONN: // sce rdr connection
					if(unitIndex<0 || unitIndex>=MAX_SCE_RDR_INFO_CNT){
						sprintf(trcBuf,"[%s:%d] invalid RDR_CONN index number. (%d) \n",__FUNCTION__, __LINE__, unitIndex);
						trclib_writeLogErr (FL,trcBuf);
						rv=0; goto GO_OUT;
					}
					param->status		= pSceDev->rdrConnStatus[unitIndex].status;
					param->preStatus 	= pSceDev->rdrConnStatus[unitIndex].preStatus; 
					pStsInfo 			= &pSceDev->rdrConnStatus[unitIndex];
					break;

				default:
					sprintf(trcBuf,"[%s] UNKNOWN SCE UNIT TYPE. (%d) \n",__FUNCTION__, unitType);
					trclib_writeLogErr (FL,trcBuf);
					goto GO_OUT;
			}
			// alarm type return
			//almType = fimd_getSceAlarmType(unitType);
			almType = fimd_getAlarmType(unitType);
			break;

		default:
			sprintf(trcBuf,"[%s] UNKNOWN SYSTEM TYPE (%d) \n", __FUNCTION__, sysType );
			trclib_writeLogErr (FL,trcBuf);
			goto GO_OUT;
	}

	param->sysType   = sysType;
	param->devKind   = unitType;    //(ex) SCE_FAN , SCE_PORT
	param->devIndex  = unitIndex;
	param->occurFlag = 0; 
	param->changeFlag = 0;

	// 상태를 체크하여 Level을 결정한다.
	level = fimd_thresholdCommCheckStatus(param);

	/* Level에 따라 DB 및 Alarm 정보... */
	switch (level) {
		case SFM_ALM_NORMAL:
			if (param->changeFlag == 1){
				int abnormal = fimd_getAbnormalAlarmLevel_byDevName(param);
				fimd_makeCommStsAlmMsg(sysType, unitType, unitIndex, SFM_ALM_NORMAL, abnormal, param->occurFlag, devName);
				fimd_saveCommStsAlmInfo2DB(sysType, unitType, unitIndex,  abnormal, param->occurFlag, devName);
				rv = 1;
			}
			pStsInfo->level = SFM_ALM_NORMAL;  
			pStsInfo->preStatus = param->preStatus; // 이전 상태 값 설정.
			if(sysType == TAPA || sysType == TAPB)
				setTapInfo(sysIndex, unitType, unitIndex, pStsInfo);
			else if(sysType == L2SWA|| sysType == L2SWB)
				setL2swInfo(sysIndex, unitType, unitIndex, pStsInfo);

			goto GO_OUT;

		case SFM_ALM_MINOR:
			if (param->changeFlag == 1){
				fimd_makeCommStsAlmMsg(sysType, unitType, unitIndex, SFM_ALM_MINOR, SFM_ALM_NORMAL, param->occurFlag, devName);
				fimd_saveCommStsAlmInfo2DB(sysType, unitType, unitIndex,  SFM_ALM_MINOR, param->occurFlag, devName);

				if (param->occurFlag) {	// 장애 발생시 장애통계 count한다.
					fimd_increaseAlmStatIndex (0, sysIndex, almType, level);	// 시스템별, 유형별, 등급별 구분
				}
				rv = 1;
			}
			pStsInfo->level = SFM_ALM_MINOR;  
			pStsInfo->preStatus = param->preStatus; // 이전 상태 값 설정.
			if(sysType == TAPA || sysType == TAPB)
				setTapInfo(sysIndex, unitType, unitIndex, pStsInfo);
			else if(sysType == L2SWA|| sysType == L2SWB)
				setL2swInfo(sysIndex, unitType, unitIndex, pStsInfo);

			goto GO_OUT;

		case SFM_ALM_MAJOR:
			if (param->changeFlag == 1){
				fimd_makeCommStsAlmMsg(sysType, unitType, unitIndex, SFM_ALM_MAJOR, SFM_ALM_NORMAL, param->occurFlag, devName);
				fimd_saveCommStsAlmInfo2DB(sysType, unitType, unitIndex,  SFM_ALM_MAJOR, param->occurFlag, devName);

				if (param->occurFlag) {	// 장애 발생시 장애통계 count한다.
					fimd_increaseAlmStatIndex (0, sysIndex, almType, level);	// 시스템별, 유형별, 등급별 구분
				}
				rv = 1;
			}
			pStsInfo->level = SFM_ALM_MAJOR;  
			pStsInfo->preStatus = param->preStatus; //  이전 상태 값 설정
			if(sysType == TAPA || sysType == TAPB)
				setTapInfo(sysIndex, unitType, unitIndex, pStsInfo);
			else if(sysType == L2SWA|| sysType == L2SWB)
				setL2swInfo(sysIndex, unitType, unitIndex, pStsInfo);

			goto GO_OUT;

		case SFM_ALM_CRITICAL:
			if (param->changeFlag == 1){
				fimd_makeCommStsAlmMsg(sysType, unitType, unitIndex, SFM_ALM_CRITICAL, SFM_ALM_NORMAL, param->occurFlag, devName);
				fimd_saveCommStsAlmInfo2DB(sysType, unitType, unitIndex, SFM_ALM_CRITICAL, param->occurFlag, devName);

				if (param->occurFlag) {	// 장애 발생시 장애통계 count한다.
					fimd_increaseAlmStatIndex (0, sysIndex, almType, level);//시스템별, 유형별, 등급별.
				}
				rv = 1;
			}
			pStsInfo->level = SFM_ALM_CRITICAL; 
			pStsInfo->preStatus = param->preStatus; // 이전 상태 값 설정.
			if(sysType == TAPA || sysType == TAPB)
				setTapInfo(sysIndex, unitType, unitIndex, pStsInfo);
			else if(sysType == L2SWA|| sysType == L2SWB)
				setL2swInfo(sysIndex, unitType, unitIndex, pStsInfo);

			goto GO_OUT;

		default:  
			//sprintf(trcBuf,"[%s:%d] INVALID LEVEL VALUE(%d) \n", __FUNCTION__, __LINE__, level);
			//trclib_writeLogErr (FL,trcBuf);
			break;
	}

	rv = 0;

GO_OUT:

	if(param) free(param);
	return rv;
}

// TAP의 구조체가 COMM_STATUS_PARAM 에 맞지않아 수동 설정해 주어야 한다.
// 값을 설정한다. 
void setTapInfo(int sysIndex, int unitType,int unitIndex, SCE_SYS_STATUS_INFO *pStsInfo)
{
	if(unitType == TAP_PORT ){
		l3pd->l3ProbeDev[sysIndex].gigaLanInfo[unitIndex].mask       	= pStsInfo->mask;
		l3pd->l3ProbeDev[sysIndex].gigaLanInfo[unitIndex].status      	= pStsInfo->status;
		l3pd->l3ProbeDev[sysIndex].gigaLanInfo[unitIndex].prevStatus	= pStsInfo->preStatus;
		l3pd->l3ProbeDev[sysIndex].gigaLanInfo[unitIndex].level       	= pStsInfo->level;
	} else if(unitType == TAP_POWER){ // 20110424 by dcham
		l3pd->l3ProbeDev[sysIndex].powerInfo[unitIndex].mask          = pStsInfo->mask;
		l3pd->l3ProbeDev[sysIndex].powerInfo[unitIndex].status        = pStsInfo->status;
		l3pd->l3ProbeDev[sysIndex].powerInfo[unitIndex].prevStatus    = pStsInfo->preStatus;
		//l3pd->l3ProbeDev[sysIndex].powerInfo[unitIndex].level         = pStsInfo->level;
		l3pd->l3ProbeDev[sysIndex].powerInfo[unitIndex].level         = SFM_HW_NOT_EQUIP;
	}
}

// L2 Switch 의 구조체가 COMM_STATUS_PARAM 에 맞지않아 수동 설정해 주어야 한다.
// 값을 설정한다. 
void setL2swInfo(int sysIndex, int unitType,int unitIndex, SCE_SYS_STATUS_INFO *pStsInfo)
{
	if(unitType == L2SW_PORT){
		g_pstL2Dev->l2Info[sysIndex].portInfo[unitIndex].mask			= pStsInfo->mask;
		g_pstL2Dev->l2Info[sysIndex].portInfo[unitIndex].status			= pStsInfo->status;
		g_pstL2Dev->l2Info[sysIndex].portInfo[unitIndex].prevStatus		= pStsInfo->preStatus;
		g_pstL2Dev->l2Info[sysIndex].portInfo[unitIndex].level			= pStsInfo->level;
	}
}

/***************************************************************
* Logon 통계 감시 Alarm level 확인을 위한 함수
* added by uamyd 20110208
****************************************************************/
int fimd_checkLogonSuccessRateAlm(int sysIndex, int log_mod, int rate, int almChkFlag)
{
	int     changeFlag=0; // 장애 발생 또는 해지인 경우 1로 setting된다.
    
	// 일단 새로운 값을 세팅
	g_pstLogonRate->rate = rate;

	/** Request 값이 0 인 경우에는 NORMAL STATUS 로 체크 한다. 즉!
        이 시점에서 RATE 값이 100 이라고 가정을 하면, 퍼펙트한 NORMAL 상태가 됨.
        added by uamyd 20110222
    **/
	if( almChkFlag & DISABLE_REQ ){
		almChkFlag = 0;// 0으로 세팅을 하고, 이 변수를 사용하기로 함.
                       // almChkFlag 는 이 위치부터는 더 이상 사용하지 않으므로, 활용 가능!
	}

    // minor 장애 기준값보다 높은!!!!!!! 경우,
    // - 이전에 장애가 발생되어 있었으면 장애가 해지된 경우이므로,
    //  발생되었던 각각 해당 등급에 대한 해지 메시지를 만들어 cond로 보낸다.
    //      
    if ( !almChkFlag || rate > g_pstLogonRate->minLimit)
    {       
        g_pstLogonRate->level = SFM_ALM_NORMAL;  // sfdb->sys에 update
    
        if (g_pstLogonRate->criFlag) {
            fimd_hdlLogonSuccessRateAlm (sysIndex, log_mod, SFM_ALM_NORMAL, SFM_ALM_CRITICAL, 0); // critical clear
            g_pstLogonRate->criFlag = 0;
            changeFlag = 1;
        }   
        if (g_pstLogonRate->majFlag) {
            fimd_hdlLogonSuccessRateAlm (sysIndex, log_mod, SFM_ALM_NORMAL, SFM_ALM_MAJOR, 0); // major clear
            g_pstLogonRate->majFlag = 0;
            changeFlag = 1;
        }
        if (g_pstLogonRate->minFlag) {
            fimd_hdlLogonSuccessRateAlm (sysIndex, log_mod, SFM_ALM_NORMAL, SFM_ALM_MINOR, 0); // minor clear
            g_pstLogonRate->minFlag = 0;
            changeFlag = 1;
        }

    } //-- end of normal --//

    // minor보다 낮고 major 기준값보다 높은 경우, (minor 장애 조건)
    // - 더 높은 등급(major, critical)이 발생되어 있었으면, 등급 하강 조건이므로,
    //  각각 해지 메시지를 만든다.
    //
    else if (rate > g_pstLogonRate->majLimit)
    {
        if (g_pstLogonRate->criFlag) {
            fimd_hdlLogonSuccessRateAlm (sysIndex, log_mod, SFM_ALM_MINOR, SFM_ALM_CRITICAL, 0); // critical clear
            g_pstLogonRate->criFlag = 0;
            changeFlag = 1;
        }

        if (g_pstLogonRate->majFlag) {
            fimd_hdlLogonSuccessRateAlm (sysIndex, log_mod, SFM_ALM_MINOR, SFM_ALM_MAJOR, 0); // major clear
            g_pstLogonRate->majFlag = 0;
            changeFlag = 1;
        }

		if( !g_pstLogonRate->minFlag ){
			g_pstLogonRate->minFlag = 1;
			changeFlag = 1;
		}

		// 일단 duration이 없는 것으로 구현. => 5분 통계 데이타를 조회하는 결과이기 때문.
		g_pstLogonRate->level   = SFM_ALM_MINOR;
		fimd_hdlLogonSuccessRateAlm (sysIndex, log_mod, SFM_ALM_MINOR, 0, 1);

    } //-- end of major condition --//

    // major보다 낮고 critical 기준값보다 높은 경우, (major 장애 조건)
    // - 더 높은 등급(critical)이 발생되어 있었으면, 등급 하강 조건이므로 해지 메시지를 만든다.
    //
    else if (rate > g_pstLogonRate->criLimit)
    {

        if (g_pstLogonRate->criFlag) {
            fimd_hdlLogonSuccessRateAlm (sysIndex, log_mod, SFM_ALM_MAJOR, SFM_ALM_CRITICAL, 0); // critical clear
            g_pstLogonRate->criFlag = 0;
            changeFlag = 1;
        }

		if( !g_pstLogonRate->majFlag ){
			g_pstLogonRate->majFlag = 1;
			g_pstLogonRate->minFlag = 0; //무조건 해제, DB에서는 자동으로 삭제됨.
            changeFlag = 1;
		}

		g_pstLogonRate->level   = SFM_ALM_MAJOR;
		fimd_hdlLogonSuccessRateAlm (sysIndex, log_mod, SFM_ALM_MAJOR, 0, 1);

    } //-- end of major condition --//

    // critical 기준값보다 낮은 경우, (critical 장애 조건)
    //
    else
    {
		if( !g_pstLogonRate->criFlag ){
			g_pstLogonRate->criFlag = 1;
			g_pstLogonRate->majFlag = 0;
			g_pstLogonRate->minFlag = 0;
			changeFlag = 1;
		}

		g_pstLogonRate->level   = SFM_ALM_CRITICAL;
		fimd_hdlLogonSuccessRateAlm (sysIndex, log_mod, SFM_ALM_CRITICAL, 0, 1);

    } //-- end of critical condition --//

    return changeFlag;
}


int fimd_checkSMChStsAlm(int sysIndex, SFM_SMChInfo *smChInfo, int smChID)
{
	int occurFlag = 1;

	if( smChInfo->each[smChID].status != SFM_SM_CONN_STATUS_LINK_DN ){
		//release
		occurFlag  = 0;
	}
	fimd_hdlSMChStsAlm(sysIndex, smChID, smChInfo, SFM_ALM_CRITICAL, occurFlag);

	return 0;
}
