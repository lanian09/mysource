#include "fimd_proto.h"

extern char	trcBuf[4096], trcTmp[1024], sysLabel[COMM_MAX_NAME_LEN];;
extern SFM_sfdb		*sfdb;
extern int	trcFlag, trcLogFlag;


//------------------------------------------------------------------------------
// CPU Usage 장애 발생,해재 시 호출되어 발생/해지 절차를 수행한다.
// - 장애 발생/해지 로그를 만들어 cond로 보낸다. -> GUI console에 출력된다.
// - 장애 발생/해지 내용을 DB에 넣는다. -> GUI에서 remote로 장애 이력 조회할 수 있다.
// - 장애 발생한 경우 장애 통계 count한다.
//------------------------------------------------------------------------------
int fimd_hdlCpuUsageAlm (int sysIndex, int cpuIndex, int almLevel, int prevAlmLevel, int occurFlag)
{
	// 장애 발생,해지 로그를 만들어 cond로 보낸다.
	if ( occurFlag ){
		fimd_makeCpuUsageAlmMsg (sysIndex, cpuIndex, almLevel, occurFlag);
 		fimd_saveCpuUsageAlmInfo2DB (sysIndex, cpuIndex, almLevel, occurFlag);
	}else {
		fimd_makeCpuUsageAlmMsg (sysIndex, cpuIndex, prevAlmLevel, occurFlag);
 		fimd_saveCpuUsageAlmInfo2DB (sysIndex, cpuIndex, prevAlmLevel, occurFlag);
	}

	// 장애 발생시 장애통계 count한다.
	if (occurFlag) {
		// 시스템별, 유형별, 등급별 구분
		fimd_increaseAlmStat (sysIndex, SFM_ALM_TYPE_CPU_USAGE, almLevel);
	}

	return 1;

} //----- End of fimd_hdlCpuUsageAlm -----//



//------------------------------------------------------------------------------
// Memory Usage 장애 발생,해재 시 호출되어 발생/해지 절차를 수행한다.
// - 장애 발생/해지 로그를 만들어 cond로 보낸다. -> GUI console에 출력된다.
// - 장애 발생/해지 내용을 DB에 넣는다. -> GUI에서 remote로 장애 이력 조회할 수 있다.
// - 장애 발생한 경우 장애 통계 count한다.
//------------------------------------------------------------------------------
int fimd_hdlMemUsageAlm (int sysIndex, int almLevel, int prevAlmLevel, int occurFlag)
{
	// 장애 발생,해지 로그를 만들어 cond로 보낸다.
	if ( occurFlag ){
		fimd_makeMemUsageAlmMsg (sysIndex, almLevel, occurFlag);
		fimd_saveMemUsageAlmInfo2DB (sysIndex, almLevel, occurFlag);
	}else {
		fimd_makeMemUsageAlmMsg (sysIndex, prevAlmLevel, occurFlag);
		fimd_saveMemUsageAlmInfo2DB (sysIndex, prevAlmLevel, occurFlag);
	}

	// 장애 발생시 장애통계 count한다.
	if (occurFlag) {
		// 시스템별, 유형별, 등급별 구분
		fimd_increaseAlmStat (sysIndex, SFM_ALM_TYPE_MEMORY_USAGE, almLevel);
	}

	return 1;

} //----- End of fimd_hdlMemUsageAlm -----//

//------------------------------------------------------------------------------
// Usage 장애 발생,해재 시 호출되어 발생/해지 절차를 수행한다.
// - 장애 발생/해지 로그를 만들어 cond로 보낸다. -> GUI console에 출력된다.
// - 장애 발생/해지 내용을 DB에 넣는다. -> GUI에서 remote로 장애 이력 조회할 수 있다.
// - 장애 발생한 경우 장애 통계 count한다.
//------------------------------------------------------------------------------
int fimd_CommUsageAlm (int sysType, int unitType, int unitIndex, int usage, int almLevel, int preAlmLevel, int occurFlag)
{
	int sysIndex = -1;
	int almType;
	char devName[20];
	
	bzero(devName, sizeof(devName));

	// Sys Index 설정..
	if(sysType == DSCM||sysType==SCMA||sysType ==TAPA||sysType==SCEA||sysType==L2SWA){ sysIndex = 0;}
	else if(sysType == SCMA||sysType==SCMB||sysType==TAPB||sysType==SCEB||sysType==L2SWB){ sysIndex = 1;}
	else if(sysType == SCMB ){ sysIndex = 2;}
	else {
		sprintf(trcBuf,"[%s] UNKNOWN SYSTEM TYPE[%d]\n",__FUNCTION__,sysType);
       	trclib_writeLogErr (FL,trcBuf);
		return -1;
	}


	// 장애 발생,해지 로그를 만들어 cond로 보낸다.
	fimd_makeCommUsageAlmMsg(sysType, unitType, unitIndex, usage, almLevel, preAlmLevel, occurFlag, devName);
	fimd_saveCommStsAlmInfo2DB(sysType, unitType, unitIndex,  almLevel, occurFlag, devName);

	almType = fimd_getAlarmType(unitType);


	// 장애 발생시 장애통계 count한다.
	if (occurFlag) {
		// 시스템별, 유형별, 등급별 구분
		//fimd_increaseAlmStat (sysIndex, SFM_ALM_TYPE_MEMORY_USAGE, almLevel);
		fimd_increaseAlmStatIndex(sysIndex, unitIndex, almType, almLevel );
	}

	return 1;

} //----- End of fimd_hdlMemUsageAlm -----//



//------------------------------------------------------------------------------
// Disk Usage 장애 발생,해재 시 호출되어 발생/해지 절차를 수행한다.
// - 장애 발생/해지 로그를 만들어 cond로 보낸다. -> GUI console에 출력된다.
// - 장애 발생/해지 내용을 DB에 넣는다. -> GUI에서 remote로 장애 이력 조회할 수 있다.
// - 장애 발생한 경우 장애 통계 count한다.
//------------------------------------------------------------------------------
int fimd_hdlDiskUsageAlm (int sysIndex, int diskIndex, int almLevel, int prevAlmLevel, int occurFlag)
{
	// 장애 발생,해지 로그를 만들어 cond로 보낸다.
	if ( occurFlag ){
		//printf("disk_occurFlag\n");
		fimd_makeDiskUsageAlmMsg (sysIndex, diskIndex, almLevel, occurFlag);
		fimd_saveDiskUsageAlmInfo2DB (sysIndex, diskIndex, almLevel, occurFlag);
	}else {
		fimd_makeDiskUsageAlmMsg (sysIndex, diskIndex, prevAlmLevel, occurFlag);
		fimd_saveDiskUsageAlmInfo2DB (sysIndex, diskIndex, prevAlmLevel, occurFlag);
	}

	// 장애 발생시 장애통계 count한다.
	if (occurFlag) {
		// 시스템별, 유형별, 등급별 구분
		fimd_increaseAlmStat (sysIndex, SFM_ALM_TYPE_DISK_USAGE, almLevel);
	}

	return 1;

} //----- End of fimd_hdlDiskUsageAlm -----//



//------------------------------------------------------------------------------
// LAN의 상태가 변경된 경우 호출되어 장애 발생/해지 절차를 수행한다.
// - 장애 발생/해지 로그를 만들어 cond로 보낸다. -> GUI console에 출력된다.
// - 장애 발생/해지 내용을 DB에 넣는다. -> GUI에서 remote로 장애 이력 조회할 수 있다.
// - 장애 발생한 경우 장애 통계 count한다.
//------------------------------------------------------------------------------
int fimd_hdlLanAlm (int sysIndex, int lanIndex)
{
	int	almLevel=SFM_ALM_NORMAL, occurFlag1=0;

	// 장애 발생,해지 로그를 만들어 cond로 보낸다.
	fimd_makeLanAlmMsg (sysIndex, lanIndex, &almLevel, &occurFlag1);

	// 장애 발생,해지 정보를 만들어 DB에 넣는다.
	fimd_saveLanAlmInfo2DB (sysIndex, lanIndex, almLevel, occurFlag1);

	// 장애 발생시 장애통계 count한다.
	if (occurFlag1) {
		// 시스템별, 유형별, 등급별 구분
		fimd_increaseAlmStat (sysIndex, SFM_ALM_TYPE_LAN, almLevel);
	}

	return 1;

} //----- End of fimd_hdlLanAlm -----//


//------------------------------------------------------------------------------
// CPS  상태가 변경된 경우 호출되어 장애 발생/해지 절차를 수행한다.
// - 장애 발생/해지 로그를 만들어 cond로 보낸다. -> GUI console에 출력된다.
// - 장애 발생/해지 내용을 DB에 넣는다. -> GUI에서 remote로 장애 이력 조회할 수 있다.
// - 장애 발생한 경우 장애 통계 count한다.
//------------------------------------------------------------------------------
int fimd_checkCpsOverAlm(int sysIndex, int lanIndex)
{
	int	almLevel=SFM_ALM_NORMAL, occurFlag1=0;

	// 장애 발생,해지 로그를 만들어 cond로 보낸다.
	fimd_makeCpsOverAlmMsg (sysIndex, lanIndex, &almLevel, &occurFlag1);

	// 장애 발생,해지 정보를 만들어 DB에 넣는다.
	fimd_saveCpsAlmInfo2DB (sysIndex, lanIndex, almLevel, occurFlag1);

	// 장애 발생시 장애통계 count한다.
	if (occurFlag1) {
		// 시스템별, 유형별, 등급별 구분
		fimd_increaseAlmStat (sysIndex, SFM_ALM_TYPE_CPS_OVER, almLevel);
	}

	return 1;

} //----- End of fimd_hdlLanAlm -----//

//------------------------------------------------------------------------------
// S/W process의 상태가 변경된 경우 호출되어 장애 발생/해지 절차를 수행한다.
// - 장애 발생/해지 로그를 만들어 cond로 보낸다. -> GUI console에 출력된다.
// - 장애 발생/해지 내용을 DB에 넣는다. -> GUI에서 remote로 장애 이력 조회할 수 있다.
// - 장애 발생한 경우 장애 통계 count한다.
//------------------------------------------------------------------------------
int fimd_hdlProcAlm (int sysIndex, int procIndex, int watchDogFlag)
{
	int	almLevel=SFM_ALM_NORMAL, occurFlag=0;
	// 장애 발생,해지 로그를 만들어 cond로 보낸다.
	fimd_makeProcAlmMsg (sysIndex, procIndex, &almLevel, &occurFlag, watchDogFlag);

	// 장애 발생,해지 정보를 만들어 DB에 넣는다.
	fimd_saveProcAlmInfo2DB (sysIndex, procIndex, almLevel, occurFlag);

	// 장애 발생시 장애통계 count한다.
	if (occurFlag) {
		// 시스템별, 유형별, 등급별 구분
		fimd_increaseAlmStat (sysIndex, SFM_ALM_TYPE_PROC, almLevel);
	}

	return 1;

} //----- End of fimd_hdlProcAlm -----//
//------------------------------------------------------------------------------

// by helca 08.03
int fimd_hdlHwComAlm (int sysIndex, SFM_HpUxHWInfo *hwInfo, int comIndex)
{
	int	almFlag=0, occurFlag=0;
	int almLevel =  hwInfo->hwcom[comIndex].level; 		// sjjeon

	// 장애 발생,해지 로그를 만들어 cond로 보낸다.
	// - 단순 상태 변경인지 장애 발생,해지 인지 almFlag에 setting된다.
	// - 장애 발생,해지 이면 almLevel와 occurFlag에 반영된다.
	fimd_makeHwComAlmMsg (sysIndex, hwInfo, comIndex, &almFlag, &almLevel, &occurFlag);

	// 장애 발생,해지 정보를 만들어 DB에 넣는다.
	if (almFlag) {
		fimd_saveHwComAlmInfo2DB (sysIndex, hwInfo, comIndex, almLevel, occurFlag);
	}

	// 장애 발생시 장애통계 count한다.
	if (occurFlag) {
		// 시스템별, 유형별, 등급별 구분
		fimd_increaseAlmStat (sysIndex, SFM_ALM_TYPE_MP_HW, almLevel);
	}

	return 1;

} //----- End of fimd_hdlHwComAlm -----//

//------------------------------------------------------------------------------
// Call Info 장애 발생,해재 시 호출되어 발생/해지 절차를 수행한다.
// - 장애 발생/해지 로그를 만들어 cond로 보낸다. -> GUI console에 출력된다.
// - 장애 발생/해지 내용을 DB에 넣는다. -> GUI에서 remote로 장애 이력 조회할 수 있다.
// - 장애 발생한 경우 장애 통계 count한다.
//------------------------------------------------------------------------------
int fimd_hdlCallInfoAlm (int sysIndex, int almLevel, int prevAlmLevel, int occurFlag)
{
    // 장애 발생,해지 로그를 만들어 cond로 보낸다.
    if ( occurFlag ){
        fimd_makeCallInfoAlmMsg (sysIndex, almLevel, occurFlag);
        fimd_saveCallInfoAlmInfo2DB (sysIndex, almLevel, occurFlag);
    }else {
        fimd_makeCallInfoAlmMsg (sysIndex, prevAlmLevel, occurFlag);
        fimd_saveCallInfoAlmInfo2DB (sysIndex, prevAlmLevel, occurFlag);
    }

    // 장애 발생시 장애통계 count한다.
    if (occurFlag) {
        // 시스템별, 유형별, 등급별 구분
        fimd_increaseAlmStat (sysIndex, SFM_ALM_TYPE_CALL_INFO, almLevel);
    }

    return 1;

} //----- End of fimd_hdlCallInfoAlm -----//

//------------------------------------------//
//------------------------------------------//
/* by helca */

int fimd_hdlRmtLanAlm (int sysIndex, int lanIndex)
{
	int	almLevel=SFM_ALM_NORMAL, occurFlag=0;

	// 장애 발생,해지 로그를 만들어 cond로 보낸다.
	fimd_makeRmtLanAlmMsg (sysIndex, lanIndex, &almLevel, &occurFlag);

	// 장애 발생,해지 정보를 만들어 DB에 넣는다.
	fimd_saveRmtLanAlmInfo2DB (sysIndex, lanIndex, almLevel, occurFlag);

	// 장애 발생시 장애통계 count한다.
	if (occurFlag) {
		// 시스템별, 유형별, 등급별 구분
		fimd_increaseAlmStatIndex (sysIndex, lanIndex, SFM_ALM_TYPE_RMT_LAN, almLevel);
	}

	return 1;

} //----- End of fimd_hdlRmtLanAlm -----//
/* by helca */
int fimd_hdlOptLanAlm (int sysIndex, int lanIndex)
{
	int	almLevel=SFM_ALM_NORMAL, occurFlag=0;

	// 장애 발생,해지 로그를 만들어 cond로 보낸다.
	fimd_makeOptLanAlmMsg (sysIndex, lanIndex, &almLevel, &occurFlag);

	// 장애 발생,해지 정보를 만들어 DB에 넣는다.
	fimd_saveOptLanAlmInfo2DB (sysIndex, lanIndex, almLevel, occurFlag);

	// 장애 발생시 장애통계 count한다.
	if (occurFlag) {
		// 시스템별, 유형별, 등급별 구분
		fimd_increaseAlmStat (sysIndex, SFM_ALM_TYPE_OPT_LAN, almLevel);
	}

	return 1;

} //----- End of fimd_hdlOptLanAlm -----//

/* by helca */
int fimd_hdlDupHeartAlm (int sysIndex, SFM_SysDuplicationSts *pDup, int occurFlag)
{
	int	almLevel=SFM_ALM_CRITICAL;

	// 장애 발생,해지 로그를 만들어 cond로 보낸다.
	fimd_makeDupHeartAlmMsg (sysIndex, pDup, occurFlag);

	// 장애 발생,해지 정보를 만들어 DB에 넣는다.
	fimd_saveDupHeartAlmInfo2DB (sysIndex, almLevel, occurFlag);

	// 장애 발생시 장애통계 count한다.
	if (occurFlag) {
		// 시스템별, 유형별, 등급별 구분
		fimd_increaseAlmStat (sysIndex, SFM_ALM_TYPE_DUP_HEARTBEAT, almLevel); // by helca 08.01
	}

	// GUI에서 alm_noti를 못받는 경우가 생긴다. add by helca 2008.09.01
	fimd_broadcastAlmEvent2Client ();
	commlib_microSleep(300000);
	fimd_broadcastAlmEvent2Client ();

	return 1;

} //----- End of fimd_hdlDupHeartAlm -----//

/* by helca */ 
int fimd_hdlDupOosAlm (int sysIndex, SFM_SysDuplicationSts *pDup)
{
	int		almLevel=SFM_ALM_CRITICAL, occurFlag=0;

	// 장애 발생,해지 로그를 만들어 cond로 보낸다.
	fimd_makeDupOosAlmMsg (sysIndex, pDup, &almLevel, &occurFlag);

	// 장애 발생,해지 정보를 만들어 DB에 넣는다.
	fimd_saveDupOosAlmInfo2DB (sysIndex, almLevel, occurFlag);

	// 장애 발생시 장애통계 count한다.
	if (occurFlag) {
		// 시스템별, 유형별, 등급별 구분
		fimd_increaseAlmStat (sysIndex, SFM_ALM_TYPE_DUP_OOS, almLevel); // by helca 07.31
	}

	return 1;

} //----- End of fimd_hdlDupOosAlm -----//
/* by helca */
int fimd_hdlSuccAlm (int sysIndex, int succIndex, int count, int rate, int occurFlag) // by helca 10.20
{
	int	almLevel=SFM_ALM_CRITICAL;

	// 장애 발생,해지 로그를 만들어 cond로 보낸다.
	fimd_makeSuccRateAlmMsg (sysIndex, succIndex, almLevel, count, rate, occurFlag); // by helca 10.20
	
	// 장애 발생,해지 정보를 만들어 DB에 넣는다.
	fimd_saveSuccRateAlmInfo2DB (sysIndex, succIndex, almLevel, count, rate, occurFlag);

	// 장애 발생시 장애통계 count한다.
	if (occurFlag) {
		// 시스템별, 유형별, 등급별 구분
		fimd_increaseAlmStatIndex (sysIndex, succIndex, SFM_ALM_TYPE_SUCC_RATE, almLevel); // by helca 07.31
	}

	return 1;

} //----- End of fimd_hdlSuccAlm -----//

int fimd_hdlSuccRateIpAlm(int sysIndex, SuccRateIpInfo succRateIp, SFM_SysSuccRate *succRate, int occurFlag) 
{
	int		i;
	int		almLevel=SFM_ALM_CRITICAL;
	// 장애 발생,해지 로그를 만들어 cond로 보낸다.
	fimd_makeSuccRateIpAlmMsg (sysIndex, succRateIp, succRate, almLevel, occurFlag);

	// 장애 발생,해지 정보를 만들어 DB에 넣는다.
	fimd_saveSuccRateIpAlmInfo2DB (sysIndex, succRateIp, succRate, (char*)succRate->name, almLevel, occurFlag);

	// 장애 발생시 장애통계 count한다.
	if (occurFlag) {
		// 시스템별, 유형별, 등급별 구분

		for(i = 0;i < SFM_REAL_SUCC_RATE_CNT; i++){
			if(!strcasecmp((char*)sfdb->sys[sysIndex].commInfo.succRate[i].name, (char*)succRate->name))	
				break;
		}
		fimd_increaseAlmStatIndex (sysIndex, i, SFM_ALM_TYPE_SUCC_RATE, almLevel);
	}

	return 1;

}
#if 0
int fimd_hdlSessLoadAlm (int sysIndex, unsigned short sess_load)
{
	int	almLevel=SFM_ALM_CRITICAL, occurFlag=0;

	// 장애 발생,해지 로그를 만들어 cond로 보낸다.
	fimd_makeSessLoadAlmMsg (sysIndex, sess_load, &almLevel, &occurFlag);

	// 장애 발생,해지 정보를 만들어 DB에 넣는다.
	fimd_saveSessLoadAlmInfo2DB (sysIndex, sess_load, almLevel, occurFlag);

	// 장애 발생시 장애통계 count한다.
	if (occurFlag) {
		// 시스템별, 유형별, 등급별 구분
		fimd_increaseAlmStat (sysIndex, SFM_ALM_TYPE_SESS_LOAD, almLevel); // by helca 08.01
	}

	return 1;

} //----- End of fimd_hdlSessLoadAlm -----//
#endif 

/* by helca */
int fimd_hdlRmtDbStsAlm (int sysIndex, int rmtDbIndex, int occurFlag)
{
	int	almLevel=SFM_ALM_CRITICAL;

	// 장애 발생,해지 로그를 만들어 cond로 보낸다.
	fimd_makeRmtDbStsAlmMsg (sysIndex, rmtDbIndex, occurFlag);

	// 장애 발생,해지 정보를 만들어 DB에 넣는다.
	fimd_saveRmtDbStsAlmInfo2DB (sysIndex, rmtDbIndex, almLevel, occurFlag);

	// 장애 발생시 장애통계 count한다.
	if (occurFlag) {
		// 시스템별, 유형별, 등급별 구분
		fimd_increaseAlmStat (sysIndex, SFM_ALM_TYPE_DBCON_STST, almLevel); // by helca 07.31
	}

	return 1;

} //----- End of fimd_hdlRmtDbStsAlm -----//

// 20100916 by dcham
int fimd_hdlSCMFaultStsAlm (int sysIndex, SFM_SysDupSts *pDup, int actSts)
{
    int almLevel = SFM_ALM_CRITICAL;
    int occurFlag = 0;

    if(pDup->myStatus!= 3 && actSts == 3) {
        occurFlag = 1; // 장애발생
        pDup->dualStsAlmLevel = SFM_ALM_CRITICAL;
    } else if(pDup->myStatus == 3 && actSts != 3) {
        occurFlag = 0; // 장애해제
        pDup->dualStsAlmLevel = SFM_ALM_NORMAL;
    } else if(pDup->myStatus != 3 && pDup->myStatus != 3) {
        pDup->dualStsAlmLevel = SFM_ALM_NORMAL;
        return 1;
    }

    // 장애 발생,해지 로그를 만들어 cond로 보낸다.
    fimd_makeSCMFaultStsAlmMsg (sysIndex, almLevel, occurFlag);

    // 장애 발생,해지 정보를 만들어 DB에 넣는다.
    fimd_saveSCMFaultStsAlmInfo2DB (sysIndex, almLevel, occurFlag);

    // 장애 발생시 장애통계 count한다.
    if (occurFlag) {
    // 시스템별, 유형별, 등급별 구분
    fimd_increaseAlmStat (sysIndex, SFM_ALM_TYPE_SCM_FAULTED, almLevel);
    }

    return 1;

} //----- End of fimd_hdlRmtDbStsAlm -----//

/* by helca */
int fimd_hdlhwNTPAlm(int sysIndex, int hwNTPIndex, int occurFlag){
	int	almLevel = SFM_ALM_MINOR; // by helca 08.30

	// 장애 발생,해지 로그를 만들어 cond로 보낸다.
	fimd_makehwNTPAlmMsg (sysIndex, hwNTPIndex, occurFlag);

	// 장애 발생,해지 정보를 만들어 DB에 넣는다.
	fimd_savehwNTPAlmInfo2DB (sysIndex, hwNTPIndex, almLevel, occurFlag);

	// 장애 발생시 장애통계 count한다.
	if (occurFlag) {
		// 시스템별, 유형별, 등급별 구분
		fimd_increaseAlmStatIndex (sysIndex, hwNTPIndex, SFM_ALM_TYPE_HWNTP, almLevel); // by helca 07.31
	}
	return 0;

} //----- End of fimd_hdlhwNTPAlm -----//

/* by helca */
int fimd_hdlPDCpuUsageAlm (int devIndex, int almLevel, int prevAlmLevel, int occurFlag)
{
	
	// 장애 발생,해지 로그를 만들어 cond로 보낸다.
	if ( occurFlag ){
		fimd_makePDCpuUsageAlmMsg (devIndex, almLevel, occurFlag);
 		fimd_savePDCpuUsageAlmInfo2DB (devIndex, almLevel, occurFlag);
	}else {
		fimd_makePDCpuUsageAlmMsg (devIndex, prevAlmLevel, occurFlag);
 		fimd_savePDCpuUsageAlmInfo2DB (devIndex, prevAlmLevel, occurFlag);
	}

	// 장애 발생시 장애통계 count한다.
	if (occurFlag) {
		// 시스템별, 유형별, 등급별 구분
		//fimd_increaseAlmStatIndex (0, devIndex, SFM_ALM_TYPE_PD_CPU_USAGE, almLevel);
		fimd_increaseAlmStatIndex (0, devIndex, SFM_ALM_TYPE_TAP_CPU_USAGE, almLevel);
	}

	return 1;
	
} //----- End of fimd_hdlPDCpuUsageAlm -----//

// sjjeon
int fimd_hdlL2swCpuUsageAlm (int devIndex, int almLevel, int prevAlmLevel, int occurFlag)
{
	// 장애 발생,해지 로그를 만들어 cond로 보낸다.
	if (occurFlag ){
		fimd_makeL2swCpuUsageAlmMsg(devIndex, almLevel, occurFlag);
 		fimd_saveL2swCpuUsageAlmInfo2DB(devIndex, almLevel, occurFlag);
	}else {
		fimd_makeL2swCpuUsageAlmMsg(devIndex, prevAlmLevel, occurFlag);
 		fimd_saveL2swCpuUsageAlmInfo2DB(devIndex, prevAlmLevel, occurFlag);
	}

	// 장애 발생시 장애통계 count한다.
	if (occurFlag) {
		// 시스템별, 유형별, 등급별 구분
		fimd_increaseAlmStatIndex (0, devIndex, SFM_ALM_TYPE_L2_CPU, almLevel);
	}

	return 1;
	
} //----- End of fimd_hdlL2swCpuUsageAlm-----//


/* by helca */
int fimd_hdlPDMemUsageAlm (int devIndex, int almLevel, int prevAlmLevel, int occurFlag)
{
	// 장애 발생,해지 로그를 만들어 cond로 보낸다.
	if ( occurFlag ){
		fimd_makePDMemUsageAlmMsg (devIndex, almLevel, occurFlag);
		fimd_savePDMemUsageAlmInfo2DB (devIndex, almLevel, occurFlag);
	}else {
		fimd_makePDMemUsageAlmMsg (devIndex, prevAlmLevel, occurFlag);
		fimd_savePDMemUsageAlmInfo2DB (devIndex, prevAlmLevel, occurFlag);
	}

	// 장애 발생시 장애통계 count한다.
	if (occurFlag) {
		// 시스템별, 유형별, 등급별 구분
		//fimd_increaseAlmStatIndex (0, devIndex, SFM_ALM_TYPE_PD_MEMORY_USAGE, almLevel);
		fimd_increaseAlmStatIndex (0, devIndex, SFM_ALM_TYPE_TAP_MEMORY_USAGE, almLevel);
	}

	return 1;

} //----- End of fimd_hdlL2swMemUsageAlm -----//

/* 
 by sjjeon
   */
int fimd_hdlL2swMemUsageAlm (int devIndex, int almLevel, int prevAlmLevel, int occurFlag)
{
	// 장애 발생,해지 로그를 만들어 cond로 보낸다.
	if ( occurFlag ){
		fimd_makeL2swMemUsageAlmMsg(devIndex, almLevel, occurFlag);
		fimd_saveL2swMemUsageAlmInfo2DB(devIndex, almLevel, occurFlag);
	}else {
		fimd_makeL2swMemUsageAlmMsg(devIndex, prevAlmLevel, occurFlag);
		fimd_saveL2swMemUsageAlmInfo2DB(devIndex, prevAlmLevel, occurFlag);
	}

	// 장애 발생시 장애통계 count한다.
	if (occurFlag) {
		// 시스템별, 유형별, 등급별 구분
		fimd_increaseAlmStatIndex (0, devIndex, SFM_ALM_TYPE_L2_MEM, almLevel);
	}

	return 1;

} //----- End of fimd_hdlL2swMemUsageAlm -----//

/* by helca */
int fimd_hdlPDFanAlm (int devIndex, int fanIndex)
{
	int		almLevel=SFM_ALM_NORMAL, occurFlag=0;

	// 장애 발생,해지 로그를 만들어 cond로 보낸다.
	fimd_makePDFanAlmMsg (devIndex, fanIndex, &almLevel, &occurFlag);

	// 장애 발생,해지 정보를 만들어 DB에 넣는다.
	fimd_savePDFanAlmInfo2DB (devIndex, fanIndex, almLevel, occurFlag);

	// 장애 발생시 장애통계 count한다.
	if (occurFlag) {
		// 시스템별, 유형별, 등급별 구분
		//fimd_increaseAlmStatIndex (0, devIndex, SFM_ALM_TYPE_PD_FAN_STS, almLevel); // by helca 07.31
		fimd_increaseAlmStatIndex (0, devIndex, SFM_ALM_TYPE_TAP_FAN_STS, almLevel); // by helca 07.31
	}

	return 1;

} //----- End of fimd_hdlPDFanAlm -----//

/* by helca */
int fimd_hdlGigaLanAlm (int devIndex, int gigaIndex, int almLevel, int occurFlag)
{
	// 장애 발생,해지 로그를 만들어 cond로 보낸다.
	fimd_makeGigaLanAlmMsg (devIndex, gigaIndex, occurFlag);
	// 장애 발생,해지 정보를 만들어 DB에 넣는다.
	fimd_saveGigaLanAlmInfo2DB (devIndex, gigaIndex, almLevel, occurFlag);

	// 장애 발생시 장애통계 count한다.
	if (occurFlag) {
		// 시스템별, 유형별, 등급별 구분
		//fimd_increaseAlmStatIndex (0, devIndex, SFM_ALM_TYPE_PD_GIGA_LAN, almLevel);
		fimd_increaseAlmStatIndex (0, devIndex, SFM_ALM_TYPE_TAP_PORT_STS, almLevel);
	}

	return 1;

} //----- End of fimd_hdlGigaLanAlm -----//

/* by helca */
int fimd_hdlRsrcLoadAlm (int sysIndex, int loadIndex, int usage, int almLevel, int prevAlmLevel, int occurFlag)
{
	
	// 장애 발생,해지 로그를 만들어 cond로 보낸다.
	if ( occurFlag ){
		fimd_makeRsrcLoadAlmMsg (sysIndex, loadIndex, usage, almLevel, occurFlag);
 		fimd_saveRsrcLoadAlmInfo2DB (sysIndex, loadIndex, almLevel, occurFlag);
	}else {
		fimd_makeRsrcLoadAlmMsg (sysIndex, loadIndex, usage, prevAlmLevel, occurFlag);
 		fimd_saveRsrcLoadAlmInfo2DB (sysIndex, loadIndex, prevAlmLevel, occurFlag);
	}

	// 장애 발생시 장애통계 count한다.
	if (occurFlag) {
		// 시스템별, 유형별, 등급별 구분
		fimd_increaseAlmStat (sysIndex, SFM_ALM_TYPE_RSRC_LOAD, almLevel); // by helca 07.31
	}

	return 1;
	
} //----- End of fimd_hdlRsrcLoadAlm -----//

#if 1 /* by helca */
int fimd_hdlQueLoadAlm (int sysIndex, int queLoadIndex, int almLevel, int prevAlmLevel, int occurFlag)
{
	// 장애 발생,해지 로그를 만들어 cond로 보낸다.
	if ( occurFlag ){
		fimd_makeQueLoadAlmMsg (sysIndex, queLoadIndex, almLevel, occurFlag);
		fimd_saveQueLoadAlmInfo2DB (sysIndex, queLoadIndex, almLevel, occurFlag);
	}else {
		fimd_makeQueLoadAlmMsg (sysIndex, queLoadIndex, prevAlmLevel, occurFlag);
		fimd_saveQueLoadAlmInfo2DB (sysIndex, queLoadIndex, prevAlmLevel, occurFlag);
	}

/*
	// 장애 발생시 장애통계 count한다.
	if (occurFlag) {
		// 시스템별, 유형별, 등급별 구분
		fimd_increaseAlmStat (sysIndex, SFM_ALM_TYPE_QUEUE_LOAD, almLevel);
	}
*/
	return 1;

} //----- End of fimd_hdlQueLoadAlm -----//
#endif 

/**NMFIS Status Alarm by helca 11.7**/
int fimd_hdlNmsifStsAlm (int sysIndex, int nmsIndex, int almLevel, int occurFlag)
{

        // 장애 발생,해지 로그를 만들어 cond로 보낸다.
        fimd_makeNmsifstsAlmMsg (sysIndex, nmsIndex, almLevel, occurFlag);
        // 장애 발생,해지 정보를 만들어 DB에 넣는다.
        fimd_saveNmsifstsAlmInfo2DB (sysIndex, nmsIndex, almLevel, occurFlag); //DB오류로 막아 놓은것 푼다. sjjeon
        // 장애 발생시 장애통계 count한다.
        if (occurFlag) {
        // 시스템별, 유형별, 등급별 구분
            fimd_increaseAlmStatIndex (sysIndex, nmsIndex, SFM_ALM_TYPE_NMSIF_CONNECT, almLevel);
        }
	return 1;
}

/* Dual Active/Standby 장애 처리 add by helca 2009.01.09 */
int fimd_hdlDupDualSts(int sysIndex, int dualActStdFlag, int dupDualOccured){

	int     almLevel=SFM_ALM_CRITICAL;

	// 장애 발생,해지 로그를 만들어 cond로 보낸다.
	fimd_hdlDupDualStsAlmMsg (sysIndex, dualActStdFlag, dupDualOccured);
	
	// 장애 발생,해지 정보를 만들어 DB에 넣는다.
	fimd_saveDualActStsAlmInfo2DB (sysIndex, almLevel, dualActStdFlag, dupDualOccured);
	
	// 장애 발생시 장애통계 count한다.
	if(dupDualOccured){
		// 시스템별, 유형별, 등급별 구분
		if(dualActStdFlag == 1)
			fimd_increaseAlmStat (sysIndex, SFM_ALM_TYPE_DUAL_ACT, almLevel);
		else if(dualActStdFlag == 2)
			fimd_increaseAlmStat (sysIndex, SFM_ALM_TYPE_DUAL_STD, almLevel);
	}

	if (dupDualOccured){
		fimd_broadcastDualAlmEvent2Client (dualActStdFlag);
		commlib_microSleep(300000);
		fimd_broadcastDualAlmEvent2Client (dualActStdFlag);
		//fprintf(stderr, "TEST fimd_broadcastDualAlmEvent2Client dualActStdFlag: %d\n", dualActStdFlag);
	}

	fimd_broadcastAlmEvent2Client ();
        commlib_microSleep(300000);
        fimd_broadcastAlmEvent2Client ();
	
	return 1;
} /* End of fimd_hdlDupDualSts */

int fimd_hdlDupTimeOutAlm (int sysIndex, int occurFlag)
{
        int     almLevel=SFM_ALM_CRITICAL;

        // 장애 발생,해지 로그를 만들어 cond로 보낸다.
        fimd_makeDupTimeOutAlmMsg (sysIndex, occurFlag);

        // 장애 발생,해지 정보를 만들어 DB에 넣는다.
        fimd_saveDupTimeOutAlmInfo2DB (sysIndex, almLevel, occurFlag);

        // 장애 발생시 장애통계 count한다.
        if (occurFlag) {
                // 시스템별, 유형별, 등급별 구분
                fimd_increaseAlmStat (sysIndex, SFM_ALM_TYPE_DUAL_STS_QRY_TIME_OUT, almLevel); // by helca 08.01
        }

	fimd_broadcastAlmEvent2Client ();
	commlib_microSleep(300000);
       	fimd_broadcastAlmEvent2Client ();
	return 1;

} /* End of fimd_hdlDupTimeOutAlm */

/* by helca */
int fimd_hdlL2LanAlm (int devIndex, int gigaIndex, int almLevel, int occurFlag)
{
	// 장애 발생,해지 로그를 만들어 cond로 보낸다.
	fimd_makeL2LanAlmMsg (devIndex, gigaIndex, occurFlag);
	// 장애 발생,해지 정보를 만들어 DB에 넣는다.
	fimd_saveL2SWLanAlmInfo2DB (devIndex, gigaIndex, almLevel, occurFlag);

	// 장애 발생시 장애통계 count한다.
	if (occurFlag) {
		// 시스템별, 유형별, 등급별 구분
		fimd_increaseAlmStatIndex (0, devIndex, SFM_ALM_TYPE_L2_LAN, almLevel);
	}

	return 1;

} //----- End of fimd_hdlL2LanAlm -----//

#if 0 /* by june */
int fimd_hdlSceCpuUsageAlm (int devIndex, int cpuIndex, int almLevel, int prevAlmLevel, int occurFlag)
{
	
	// 장애 발생,해지 로그를 만들어 cond로 보낸다.
	if ( occurFlag ){
		fimd_makeSceCpuUsageAlmMsg (devIndex, cpuIndex, almLevel, occurFlag);
 		fimd_saveSceCpuUsageAlmInfo2DB (devIndex, almLevel, occurFlag);
	}else {
		fimd_makeSceCpuUsageAlmMsg (devIndex, cpuIndex, prevAlmLevel, occurFlag);
 		fimd_saveSceCpuUsageAlmInfo2DB (devIndex, prevAlmLevel, occurFlag);
	}

	// 장애 발생시 장애통계 count한다.
	if (occurFlag) {
		// 시스템별, 유형별, 등급별 구분
		//fimd_increaseAlmStatIndex (0, devIndex, SFM_ALM_TYPE_PD_CPU_USAGE, almLevel);
		fimd_increaseAlmStatIndex (0, devIndex, SFM_ALM_TYPE_TAP_CPU_USAGE, almLevel);
	}

	return 1;
	
} //----- End of fimd_hdlSceCpuUsageAlm -----//

int fimd_hdlSceDiskUsageAlm (int devIndex, int almLevel, int prevAlmLevel, int occurFlag)
{
	// 장애 발생,해지 로그를 만들어 cond로 보낸다.
	if ( occurFlag ){
		fimd_makePDMemUsageAlmMsg (devIndex, almLevel, occurFlag);
		fimd_savePDMemUsageAlmInfo2DB (devIndex, almLevel, occurFlag);
	}else {
		fimd_makePDMemUsageAlmMsg (devIndex, prevAlmLevel, occurFlag);
		fimd_savePDMemUsageAlmInfo2DB (devIndex, prevAlmLevel, occurFlag);
	}

	// 장애 발생시 장애통계 count한다.
	if (occurFlag) {
		// 시스템별, 유형별, 등급별 구분
		//fimd_increaseAlmStatIndex (0, devIndex, SFM_ALM_TYPE_PD_MEMORY_USAGE, almLevel);
		fimd_increaseAlmStatIndex (0, devIndex, SFM_ALM_TYPE_TAP_MEMORY_USAGE, almLevel);
	}

	return 1;

} //----- End of fimd_hdlSceMemUsageAlm -----//


int fimd_hdlSceUsageAlm (SCE_USAGE_PARAM *param)
{
	
	// 장애 발생,해지 로그를 만들어 cond로 보낸다.
	if ( param->occurFlag ){
		fimd_makeSceCpuUsageAlmMsg (devIndex, cpuIndex, almLevel, occurFlag);
 		fimd_saveSceCpuUsageAlmInfo2DB (devIndex, almLevel, occurFlag);
	}else {
		fimd_makeSceCpuUsageAlmMsg (devIndex, cpuIndex, prevAlmLevel, occurFlag);
 		fimd_saveSceCpuUsageAlmInfo2DB (devIndex, prevAlmLevel, occurFlag);
	}

	// 장애 발생시 장애통계 count한다.
	if (param->occurFlag) {
		// 시스템별, 유형별, 등급별 구분
		//fimd_increaseAlmStatIndex (0, devIndex, SFM_ALM_TYPE_PD_CPU_USAGE, almLevel);
		fimd_increaseAlmStatIndex (0, devIndex, SFM_ALM_TYPE_TAP_CPU_USAGE, almLevel);
	}

	return 1;
	
} //----- End of fimd_hdlSceUsageAlm -----//
#endif /* by june */

/*
* LOGON 통계 감시를 위해서 Alarm 을 발생시키는 함수
* added by uamyd 20110209
*/
int fimd_hdlLogonSuccessRateAlm (int sysIndex, int log_mod, int almLevel, int prevAlmLevel, int occurFlag)
{
	
	// 장애 발생,해지 로그를 만들어 cond로 보낸다.
	if ( occurFlag ){
		fimd_makeLogonSuccessRateAlmMsg ( sysIndex, log_mod, almLevel, occurFlag);
 		fimd_saveLogonSuccessRateAlmInfo2DB ( sysIndex, log_mod, almLevel, occurFlag);
	}else {
		fimd_makeLogonSuccessRateAlmMsg ( sysIndex, log_mod, prevAlmLevel, occurFlag);
 		fimd_saveLogonSuccessRateAlmInfo2DB ( sysIndex, log_mod, prevAlmLevel, occurFlag);
	}

	// 장애 발생시 장애통계 count한다.
	if (occurFlag) {
		// 시스템별, 유형별, 등급별 구분
		if( !log_mod ){
			fimd_increaseAlmStatIndex (sysIndex, 0, SFM_ALM_TYPE_LOGON_SUCCESS_RATE, almLevel);
		} else {
			fimd_increaseAlmStatIndex (sysIndex, 0, SFM_ALM_TYPE_LOGOUT_SUCCESS_RATE, almLevel);
		}
	}

	return 1;
	
}

int fimd_hdlSMChStsAlm (int sysIndex, int smChID, SFM_SMChInfo *smChInfo, int almLevel, int occurFlag)
{
	
	// 장애 발생,해지 로그를 만들어 cond로 보낸다.
	fimd_makeSMChStsAlmMsg (sysIndex, almLevel, occurFlag, smChID);

	// 장애 발생,해지 정보를 만들어 DB에 넣는다.
	fimd_saveSMChStsAlmInfo2DB (sysIndex, almLevel, occurFlag, smChID);

	// 장애 발생시 장애통계 count한다.
	if (occurFlag) {
		// 시스템별, 유형별, 등급별 구분
		fimd_increaseAlmStatIndex (sysIndex, smChID, SFM_ALM_TYPE_SM_CONN_STS, almLevel);
	}

	return 1;

}
