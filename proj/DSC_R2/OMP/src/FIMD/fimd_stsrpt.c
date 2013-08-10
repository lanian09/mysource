#include "fimd_proto.h"

#define max(x,y)        (x>=y ? x : y)

extern char			trcBuf[4096], trcTmp[1024];
extern SFM_sfdb		*sfdb;
extern SFM_L3PD		*l3pd;
extern SFM_L2Dev   	*g_pstL2Dev;
extern SFM_SCE		*g_pstSCEInfo;
/* hjjung_20100823 */
//extern SFM_LEG		*g_pstCALLInfo;
extern SFM_CALL		*g_pstCALLInfo; // added by dcham 20110525 for TPS
extern ListenInfo	*ne_info;
extern int			trcFlag, trcLogFlag, trcLogId;
extern int			active_sys_arr[2], dupChkTime, dualActStdFlag[SYSCONF_MAX_ASSO_SYS_NUM];
extern time_t   	ixpc_rec_Time[SYSCONF_MAX_ASSO_SYS_NUM], samd_rec_Time[SYSCONF_MAX_ASSO_SYS_NUM];
extern time_t  		alarm_dev_Time[SYSCONF_MAX_ASSO_SYS_NUM];
extern time_t  		dual_sts_alarm_Time[SYSCONF_MAX_ASSO_SYS_NUM];
unsigned char		rmtdb_conn_lvl[SYSCONF_MAX_ASSO_SYS_NUM][SFM_MAX_DB_CNT];
extern time_t		currentTime;

extern LANIF_CONFIG	lanConf;
int minorcnt=0;

int	dualAlrmCnt[SYSCONF_MAX_ASSO_SYS_NUM] = {0, };

// h/w re-try check
int g_hw_retry_cnt[2][SFM_MAX_HPUX_HW_COM];

//------------------------------------------------------------------------------
// 모든 시스템에서 공통으로 관리되는 상태정보 report를 수신한 경우 호출되어
// - sfdb->sys에 수신한 정보를 update한다.
// - 상태가 변경된 경우 시스템 전체 장애 등급 및 현재 장애 갯수 정보를 update하고,
//	GUI client로 event port를 통해 통보한다.
//	-> GUI는 event port에서 데이터를 수신하면 alarm_history DB를 다시 읽어 현재 발생되어
//		있는 장애 리스트 table을 갱신한다.
//------------------------------------------------------------------------------
int fimd_hdlSysCommStsRpt (IxpcQMsgType *rxIxpcMsg)
{
	int		sysIndex, i;//,  goContinue=0;
	int		changeFlag=0; // 장애 발생 또는 해지인 경우 setting된다
	time_t	now;
	struct	tm*	Time;
	static  int     chkCnt = 0;
	SFM_SysCommMsgType	*rxSysCommMsg;
	COMM_STATUS_PARAM		param;
	SFM_HWSts			*pHWSts;
	SFM_HpUxHWInfo  *hwInfo =NULL;

	memset (&param, 0x00, sizeof(COMM_STATUS_PARAM));

	//	printf("[%s] msgId : %d, bodyLen : %d\n",__FUNCTION__,rxIxpcMsg->head.msgId,rxIxpcMsg->head.bodyLen );
#if 0
	printf("fimd SFM_SysCommMsgType size : %d\n",sizeof(SFM_SysCommMsgType));
	printf("fimd SFM_SysCommProcSts size : %d\n",sizeof(SFM_SysCommProcSts));
	printf("fimd SFM_SysCommDiskSts size : %d\n",sizeof(SFM_SysCommDiskSts));
	printf("fimd SFM_SysCommLanSts  size : %d\n",sizeof(SFM_SysCommLanSts));
	printf("fimd SFM_SysCommQueSts  size : %d\n",sizeof(SFM_SysCommQueSts));
	printf("fimd SFM_SysCommLinkSts size : %d\n",sizeof(SFM_SysCommLinkSts));
	printf("fimd SFM_SysDuplicationSts size : %d\n",sizeof(SFM_SysDuplicationSts));
	printf("fimd SFM_SysSuccessRate size : %d\n",sizeof(SFM_SysSuccessRate));
	printf("fimd SFM_SysIFBond size : %d\n",sizeof(SFM_SysIFBond));
	printf("fimd SFM_SysRsrcLoad size : %d\n",sizeof(SFM_SysIFBond));
	printf("fimd SFM_SysSts size : %d\n",sizeof(SFM_SysSts));
	printf("fimd SFM_Duia size : %d\n",sizeof(SFM_Duia));
#endif
	now = time((time_t *)0);
	Time = (struct tm *)localtime((time_t *)&now);

	/*
	   for(i = 0; i < SYSCONF_MAX_ASSO_SYS_NUM; i++){
	   if( !strcasecmp(sfdb->sys[i].commInfo.name, rxIxpcMsg->head.srcSysName)){
	   if( !strcasecmp(rxIxpcMsg->head.srcAppName, "SAMD"))
	   samd_rec_Time[i] = time(0);	
	   }
	   }
	 */

	// 메시지를 보낸 시스템 이름과 일치하는 index를 찾는다.
	if ((sysIndex = fimd_getSysIndexByName (rxIxpcMsg->head.srcSysName)) < 0) {
		sprintf(trcBuf,"[fimd_hdlSysCommStsRpt] unknown sysName[%s]\n", rxIxpcMsg->head.srcSysName);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	/* Network Order 변경: sjjeon*/
	//sjjeon SFM_SysCommMsgType_N2H ((SFM_SysCommMsgType*)&rxIxpcMsg->body);
	SFM_SysCommMsgType_N2H ((SFM_SysCommMsgType*)rxIxpcMsg->body);
	//yhshin rxSysCommMsg = (SFM_SysCommMsgType*)rxIxpcMsg->body;
	rxSysCommMsg = (SFM_SysCommMsgType*)rxIxpcMsg->body;

	//
	// update status informations
	//

	sfdb->sys[sysIndex].commInfo.cpuCnt  = rxSysCommMsg->cpuCount;
	sfdb->sys[sysIndex].commInfo.diskCnt = rxSysCommMsg->diskCount;
	sfdb->sys[sysIndex].commInfo.lanCnt  = rxSysCommMsg->lanCount;
	sfdb->sys[sysIndex].commInfo.procCnt = rxSysCommMsg->processCount;
	sfdb->sys[sysIndex].commInfo.queCnt  = rxSysCommMsg->queCount;
	//	printf (" rxSysCommMsg->processCount;: %d\n",  rxSysCommMsg->processCount);

	// CPU 사용율
	for (i=0; i<rxSysCommMsg->cpuCount; i++) {
		// mask된 놈은 상태관리에서 제외한다.
		if (sfdb->sys[sysIndex].commInfo.cpuInfo.mask[i] == SFM_ALM_MASKED)
			continue;
		//yhshin sfdb->sys[sysIndex].commInfo.cpuInfo.usage[i] = ntohs(rxSysCommMsg->cpu_usage[i]);
		sfdb->sys[sysIndex].commInfo.cpuInfo.usage[i] = (rxSysCommMsg->cpu_usage[i]);
		// 사용율과 장애 발생 기준치와 지속시간을 참조하여,
		// - 장애 발생,해지 상황을 감지하고
		// - 발생,해지 시 cond로 메시지를 만들어 보낸다.
		if (fimd_checkCpuUsageAlm (sysIndex, i, &sfdb->sys[sysIndex].commInfo.cpuInfo))
			changeFlag = 1;
		//fprintf(stderr, "sysIndex : %d cpu_usage: %d, flag = %d\n", 
		//	sysIndex, sfdb->sys[sysIndex].commInfo.cpuInfo.usage[i],changeFlag);
	}

	// Memory 사용율
	if(sfdb->sys[sysIndex].commInfo.memInfo.mask != SFM_ALM_MASKED){
		//yhshin sfdb->sys[sysIndex].commInfo.memInfo.usage = ntohs(rxSysCommMsg->mem_usage);
		sfdb->sys[sysIndex].commInfo.memInfo.usage = (rxSysCommMsg->mem_usage);
		if (fimd_checkMemUsageAlm (sysIndex, &sfdb->sys[sysIndex].commInfo.memInfo))
			changeFlag = 1;
	}

	// 과부하 상태 점검  : sjjeon
	if(sfdb->sys[sysIndex].commInfo.cpsOverSts.mask != SFM_ALM_MASKED)
	{
		// sfdb info
		SFM_CpsOverLoadsts *cpsInfo = (SFM_CpsOverLoadsts *)&sfdb->sys[sysIndex].commInfo.cpsOverSts;
		cpsInfo->preStatus	= cpsInfo->status;
		// loc_
		cpsInfo->status 	= (unsigned char)rxSysCommMsg->cps_over_alm_flag;

		if ( cpsInfo->status == SFM_CPS_OVER){
			cpsInfo->level = SFM_ALM_MINOR;
		}
		else {
			cpsInfo->level = SFM_ALM_NORMAL;
		}
		sprintf(trcBuf," CHECK CPS OVERLOAD @@@ [%s]SYS=%d preStatus=%d status=%d\n",
				__FUNCTION__, sysIndex,
				cpsInfo->preStatus, cpsInfo->status);
		trclib_writeLogErr (FL,trcBuf);


		if (cpsInfo->preStatus != cpsInfo->status) {
			fimd_checkCpsOverAlm (sysIndex, i);
			changeFlag = 1;
		}
	}

	// Disk 사용율
	for (i=0; i<rxSysCommMsg->diskCount; i++) {
		SFM_DiskInfo *diskInfo=&sfdb->sys[sysIndex].commInfo.diskInfo[i];
		if (diskInfo->mask == SFM_ALM_MASKED) // mask된 놈은 상태관리에서 제외한다.
			continue;
		strcpy (diskInfo->name, rxSysCommMsg->loc_disk_sts[i].diskName);
		diskInfo->usage = (rxSysCommMsg->loc_disk_sts[i].disk_usage);
		if (fimd_checkDiskUsageAlm (sysIndex, i, diskInfo))
			changeFlag = 1;
	}

	// Disk Total usage
	// by helca 11.22
	if(!strcasecmp(sfdb->sys[sysIndex].commInfo.name, "DSCM")){ 
		sfdb->sys[sysIndex].commInfo.total_disk_usage = 0;
	}
	else{		
		//yhshin sfdb->sys[sysIndex].commInfo.total_disk_usage = ntohs(rxSysCommMsg->total_disk_usage);
		sfdb->sys[sysIndex].commInfo.total_disk_usage = (rxSysCommMsg->total_disk_usage);
		//fprintf(stderr, "Total_disk_usage: %d\n", sfdb->sys[sysIndex].commInfo.total_disk_usage);
	}

	// LAN 상태
	for (i=0; i<rxSysCommMsg->lanCount; i++) {
		struct in_addr	ipAddr;
		SFM_LanInfo	*lanInfo=&sfdb->sys[sysIndex].commInfo.lanInfo[i];
		if (lanInfo->mask == SFM_ALM_MASKED) // mask된 놈은 상태관리에서 제외한다.
			continue;
		strcpy (lanInfo->name, rxSysCommMsg->loc_lan_sts[i].target_SYSName);

		// sjjeon : dsca/dscb 면 htonl을 한번더 변환 (output 할때는 string으로 변경 되므로 
		// 주소는 "211.254.95.230" 형태로 변경되므로 타시스템 전송시 네트웍 오더 변환이 없다.)
		if(!strcasecmp(rxIxpcMsg->head.srcSysName, "DSCM"))
			ipAddr.s_addr = rxSysCommMsg->loc_lan_sts[i].target_IPaddress;
		else
			ipAddr.s_addr = htonl(rxSysCommMsg->loc_lan_sts[i].target_IPaddress);

		strcpy (lanInfo->targetIp, inet_ntoa(ipAddr));
		//fprintf(stderr, "%d 번째 : lan name : %s, ip : %s\n",i, lanInfo->name, lanInfo->targetIp);
		lanInfo->prevStatus 	= lanInfo->status;
		lanInfo->status 		= rxSysCommMsg->loc_lan_sts[i].status;
#if 0
		sprintf(trcBuf,"[%s]SYS=%d lan index=%d target_Sys=%s(<=%s)  preStatus=%d status=%d level=%d\n",
				__FUNCTION__, sysIndex, i, 
				sfdb->sys[sysIndex].commInfo.lanInfo[i].name,
				rxSysCommMsg->loc_lan_sts[i].target_SYSName,
				lanInfo->prevStatus, lanInfo->status, lanInfo->level);
		trclib_writeLogErr (FL,trcBuf);
#endif
		if ( lanInfo->status == SFM_LAN_DISCONNECTED ){
			lanInfo->level = SFM_ALM_CRITICAL;
		}
		else {
			lanInfo->level = SFM_ALM_NORMAL;
		}
		if (lanInfo->prevStatus != lanInfo->status) {
			sprintf(trcBuf,"[%s] Name:%s, IP:%s, CHANGE STATUS: %d->%d\n",
					__FUNCTION__, lanInfo->name, lanInfo->targetIp, lanInfo->prevStatus,lanInfo->status);
			 trclib_writeLogErr (FL,trcBuf);
			fimd_hdlLanAlm (sysIndex, i);
			changeFlag = 1;
		}
	}

	// Process 상태
	for (i=0; i<rxSysCommMsg->processCount; i++) {

		SFM_ProcInfo *procInfo=&sfdb->sys[sysIndex].commInfo.procInfo[i];
		if (procInfo->mask == SFM_ALM_MASKED) // mask된 놈은 상태관리에서 제외한다.
			continue;
		strcpy (procInfo->name, rxSysCommMsg->loc_process_sts[i].processName);
		procInfo->level 		= rxSysCommMsg->loc_process_sts[i].level;
		procInfo->prevStatus 	= procInfo->status;
		procInfo->status 		= rxSysCommMsg->loc_process_sts[i].status;
#if 0
		if( !sysIndex ){
sprintf(trcBuf," @@@ PROCESS=%s:%ld STATUS=%d\n", 
		rxSysCommMsg->loc_process_sts[i].processName, 
		rxSysCommMsg->loc_process_sts[i].pid, 
		rxSysCommMsg->loc_process_sts[i].status);
trclib_writeLogErr(FL,trcBuf);
		}
#endif
		// 이전상태와 다르면 장애 처리
		if( procInfo->prevStatus != procInfo->status ){
			fimd_hdlProcAlm (sysIndex, i, 0);
			changeFlag = 1;
		}

		//yhshin procInfo->pid = ntohl(rxSysCommMsg->loc_process_sts[i].pid);
		procInfo->pid = (rxSysCommMsg->loc_process_sts[i].pid);
		//		printf("rcv: procInfo->pid: %d\n", procInfo->pid);

		if(procInfo->status == SFM_STATUS_DEAD) procInfo->uptime = 0;
		else procInfo->uptime = (rxSysCommMsg->loc_process_sts[i].uptime);
		//yhshin 	else procInfo->uptime = ntohl(rxSysCommMsg->loc_process_sts[i].uptime);
	}

	// Queuse 상태 
	sfdb->sys[sysIndex].commInfo.queCnt = rxSysCommMsg->queCount;
	for (i=0; i<rxSysCommMsg->queCount; i++) {
		SFM_QueInfo *queInfo=&sfdb->sys[sysIndex].commInfo.queInfo[i];
		strncpy (queInfo->qNAME, rxSysCommMsg->loc_que_sts[i].qNAME, QUE_MAX_NAME_LEN);
		queInfo->qID 	= rxSysCommMsg->loc_que_sts[i].qID;
		queInfo->qKEY 	= rxSysCommMsg->loc_que_sts[i].qKEY;
		queInfo->qNUM 	= rxSysCommMsg->loc_que_sts[i].qNUM;
		queInfo->cBYTES = rxSysCommMsg->loc_que_sts[i].cBYTES;
		queInfo->qBYTES = rxSysCommMsg->loc_que_sts[i].qBYTES;

		queInfo->qID = queInfo->qID;
		queInfo->qKEY = queInfo->qKEY;
		queInfo->qNUM = queInfo->qNUM;
		queInfo->cBYTES = queInfo->cBYTES;
		queInfo->qBYTES = queInfo->qBYTES;

#ifdef DEBUG1
		if( sysIndex ){
			sprintf(trcBuf," @@@ [QUE CHECK:%s(mask=%d,load=%d)] qID=%d(%d) qKey=%d(%d) qNUM=%d(%d) cBYTES=%d(%d) qBYTES=%d(%d)\n",
					(1==sysIndex)?"SCMA":"SCMB",
					queInfo->mask, 
					!queInfo->cBYTES?0:(queInfo->cBYTES*100)/queInfo->qBYTES,
					queInfo->qID = queInfo->qID, ntohl(queInfo->qID = queInfo->qID),
					queInfo->qKEY = queInfo->qKEY, ntohl(queInfo->qKEY = queInfo->qKEY),
					queInfo->qNUM = queInfo->qNUM, ntohl(queInfo->qNUM = queInfo->qNUM),
					queInfo->cBYTES = queInfo->cBYTES, ntohl(queInfo->cBYTES = queInfo->cBYTES),
					queInfo->qBYTES = queInfo->qBYTES, ntohl(queInfo->qBYTES = queInfo->qBYTES));
			trclib_writeLogErr (FL,trcBuf);
		}
#endif

		if (queInfo->mask == SFM_ALM_MASKED) // mask된 놈은 상태관리에서 제외한다.
			continue;
		if(sfdb->sys[sysIndex].commInfo.queInfo[i].cBYTES == 0)
			sfdb->sys[sysIndex].commInfo.queInfo[i].load = 0;
		else{
			sfdb->sys[sysIndex].commInfo.queInfo[i].load =
				(sfdb->sys[sysIndex].commInfo.queInfo[i].cBYTES * 100)/sfdb->sys[sysIndex].commInfo.queInfo[i].qBYTES;
		}
		if (fimd_checkQueueLoadAlm (sysIndex, i, queInfo))
			changeFlag = 1;

		// yhshin
		sfdb->sys[sysIndex].commInfo.queInfo[i].level = queInfo->level;
	}

	/*=== by helca===*/
	// Remote_LAN 상태
	sfdb->sys[sysIndex].commInfo.rmtLanCnt = rxSysCommMsg->rmtlanCount;
	for (i=0; i<rxSysCommMsg->rmtlanCount; i++) {
		struct in_addr	ipAddr;
		SFM_LanInfo	*rmtLanInfo = &sfdb->sys[sysIndex].commInfo.rmtLanInfo[i];
		if (rmtLanInfo->mask == SFM_ALM_MASKED) // mask된 놈은 상태관리에서 제외한다.
			continue;
		strcpy (rmtLanInfo->name, rxSysCommMsg->rmt_lan_sts[i].target_SYSName);
/* DEBUG : by june, 2010-10-07
 * DESC  : 아래 #if 1 LOG 추가.
 */
#if 0
		sprintf(trcBuf,"[fimd_hdlSysCommStsRpt] SYS[%d] rmt_lan_sts[%d]=%s(%s)\n"
				, sysIndex
				, i
				, sfdb->sys[sysIndex].commInfo.rmtLanInfo[i].name
				, rxSysCommMsg->rmt_lan_sts[i].target_SYSName);
		trclib_writeLogErr (FL,trcBuf);
#endif
		// sjjeon : dsca/dscb 면 htonl을 한번더 변환 (output 할때는 string으로 변경 되므로 
		// 주소는 "211.254.95.230" 형태로 변경되므로 타시스템 전송시 네트웍 오더 변환이 없다.)
		if(!strcasecmp(rxIxpcMsg->head.srcSysName, "DSCM"))
			ipAddr.s_addr = rxSysCommMsg->rmt_lan_sts[i].target_IPaddress;
		else
			ipAddr.s_addr = htonl(rxSysCommMsg->rmt_lan_sts[i].target_IPaddress);

		strcpy (rmtLanInfo->targetIp, inet_ntoa(ipAddr));
		//fprintf(stderr, "%d 번째 : rmt name : %s, ip : %s\n",i, rmtLanInfo->name, rmtLanInfo->targetIp);
		rmtLanInfo->prevStatus 	= rmtLanInfo->status;
		rmtLanInfo->status 		= rxSysCommMsg->rmt_lan_sts[i].status;
		if ( rmtLanInfo->status == SFM_LAN_DISCONNECTED ){
			rmtLanInfo->level = SFM_ALM_CRITICAL;
		}
		else {
			rmtLanInfo->level = SFM_ALM_NORMAL;
		}
		// 이전상태와 다르면 장애 처리
		if (rmtLanInfo->prevStatus != rmtLanInfo->status) {
			sprintf(trcBuf,"[%s] RMT Name:%s, IP:%s, CHANGE STATUS: %d->%d\n",
					__FUNCTION__, rmtLanInfo->name, rmtLanInfo->targetIp, rmtLanInfo->prevStatus, rmtLanInfo->status);
			fimd_hdlLanAlm (sysIndex, i);
			fimd_hdlRmtLanAlm (sysIndex, i);
			changeFlag = 1;
		}
	}
#if 0
	sprintf(trcBuf,"[fimd_stsrpt:fault0]scma : [%d], scmb : [%d]\n",sfdb->sys[1].commInfo.systemDup.myStatus,sfdb->sys[2].commInfo.systemDup.myStatus );
	trclib_writeLogErr (FL,trcBuf);
#endif
	// SFM_SysDuplicationSts //
	// 장애 판정 조건 주의 할것 add by helca
	// yhshin 
	// ???
	if(sysIndex > 0) {	// DSCM 제외, SCMA/SCMB
		SFM_SysDupSts 			*systemDup = &sfdb->sys[sysIndex].commInfo.systemDup;	
		SFM_SysDuplicationSts 	*pDup = &rxSysCommMsg->loc_system_dup;

		// System Status 			: systemDup
		// loc에서 올라온 경우.		: pDup

		// 상태 변화가 있을 경우만 Cond로 전송.
#if 0
	sprintf(trcBuf,"[fimd_stsrpt:fault1]scma : [%d], scmb : [%d], i = %d\n",sfdb->sys[i].commInfo.systemDup.myStatus,sfdb->sys[i].commInfo.systemDup.yourStatus,i );
	trclib_writeLogErr (FL,trcBuf);
#endif
		// sjjeon
		if(systemDup->mask != SFM_ALM_MASKED) // mask된 놈은 상태관리에서 제외한다.
		{    
			// 이전상태와 받은 상태를 비교.
			if(systemDup->myStatus != pDup->myLocalDupStatus) 
			{    
				fimd_hdlDupstsMsg_yh (sysIndex, systemDup->myStatus, pDup->myLocalDupStatus);

		     	// SCM Fault상태 알람처리 20100916 by dcham
				fimd_hdlSCMFaultStsAlm (sysIndex, systemDup, pDup->myLocalDupStatus);

				// 상태 업데이트 
				systemDup->myStatus = pDup->myLocalDupStatus;
				changeFlag = 1; 
			}    
			//Dual Acive ?? 

			// pDup->myLocalDupStatus 의 상태값을 받아오지 못했을때....
			// SCMA, SCMB 모두 pDup->myLocalDupStatus = 1일때는 ???

			// ACTIVE 상태인 것만 상태를 변경해 준다. 
			if (pDup->myLocalDupStatus == 1) { // ACTIVE
				if (sysIndex == 1) { 
					strcpy (sfdb->active_sys_name, "SCMA");
					sfdb->sys[1].commInfo.systemDup.myStatus = 1; // SCMA Active / by sjjeon
					if(sfdb->sys[2].commInfo.systemDup.myStatus == 3) // by dcham
					    sfdb->sys[2].commInfo.systemDup.myStatus = 3; // SCMB Standby
					else 
						sfdb->sys[2].commInfo.systemDup.myStatus = 2;
				} else if (sysIndex == 2) { 
					strcpy (sfdb->active_sys_name, "SCMB");
					if(sfdb->sys[1].commInfo.systemDup.myStatus == 3)
					    sfdb->sys[1].commInfo.systemDup.myStatus = 3; // SCMA Standby
					else
						sfdb->sys[1].commInfo.systemDup.myStatus = 2;
					sfdb->sys[2].commInfo.systemDup.myStatus = 1; // SCMB Active
				}    
#if 0
			}else{ // by dcham
				if(pDup->myLocalDupStatus == 3) {
                    if(sysIndex == 1){
						sfdb->sys[1].commInfo.systemDup.myStatus = 3;
						strcpy (sfdb->active_sys_name, "SCMA");
					}
					else if(sysIndex == 2) {
						sfdb->sys[2].commInfo.systemDup.myStatus = 3;
						strcpy (sfdb->active_sys_name, "SCMB");
					}
				}
#endif
			}
		}    
 
	}
    //sfdb->sys[2].commInfo.systemDup.myStatus = 3;
	//sfdb->sys[1].commInfo.systemDup.myStatus = 1;
#if 0
	sprintf(trcBuf,"[fimd_stsrpt:fault2]scma : [%d], scmb : [%d], i=%d\n",sfdb->sys[i].commInfo.systemDup.myStatus,sfdb->sys[i].commInfo.systemDup.yourStatus,i );
	trclib_writeLogErr (FL,trcBuf);
#endif
	// SFM_SysDBConnSts //
	for (i=0; i<SFM_MAX_DB_CNT; i++) {
		SFM_SysDBSts *rmtDbSts = &sfdb->sys[sysIndex].commInfo.rmtDbSts[i];

		if(rmtDbSts->mask == SFM_ALM_MASKED) // mask된 놈은 상태관리에서 제외한다.
			continue;
		if(strlen((char*)rxSysCommMsg->loc_system_dup.RmtDbSts[i].sIpAddress) < 7){
			memset(rmtDbSts, 0x00, sizeof(SFM_SysDBSts));
			continue;
		}
		if(strlen((char*)rxSysCommMsg->loc_system_dup.RmtDbSts[i].sDBAlias) > 0){ // by helca 08.24
			rmtDbSts->iStatus = rxSysCommMsg->loc_system_dup.RmtDbSts[i].iStatus; 
			strcpy((char*)rmtDbSts->sDbAlias, (char*)rxSysCommMsg->loc_system_dup.RmtDbSts[i].sDBAlias);
			strcpy((char*)rmtDbSts->sIpAddress, (char*)rxSysCommMsg->loc_system_dup.RmtDbSts[i].sIpAddress);
			//printf("DBAlias[%d] : %s, DBIP[%d] : %s\n",i, rmtDbSts->sDbAlias,i, rmtDbSts->sIpAddress);
		}
		if(rmtDbSts->iStatus == 0){  /* 1 - connect, 0 - not connect */
			if(rmtdb_conn_lvl[sysIndex][i] != SFM_ALM_CRITICAL){
				rmtdb_conn_lvl[sysIndex][i] = SFM_ALM_CRITICAL;
				fimd_hdlRmtDbStsAlm (sysIndex, i, 1);
				changeFlag = 1;
			}
		}else if(rmtDbSts->iStatus == 1){ /* 1 - connect, 0 - not connect */
			if(rmtdb_conn_lvl[sysIndex][i] == SFM_ALM_CRITICAL){
				rmtdb_conn_lvl[sysIndex][i] = SFM_ALM_NORMAL;
				fimd_hdlRmtDbStsAlm (sysIndex, i, 0);
				changeFlag = 1;
			}
		}
	}

	// NTP DEAMON ALARM CHECK
	if (sfdb->sys[sysIndex].commInfo.ntpSts[NTP_INDEX_DEAMON].mask != SFM_ALM_MASKED) {

		sfdb->sys[sysIndex].commInfo.ntpSts[NTP_INDEX_DEAMON].status = rxSysCommMsg->hwNTP[NTP_INDEX_DEAMON];

		if(rxSysCommMsg->hwNTP[NTP_INDEX_DEAMON] == NTP_CRITICAL){

			if(sfdb->sys[sysIndex].commInfo.ntpSts[NTP_INDEX_DEAMON].level != SFM_ALM_MINOR){
				sfdb->sys[sysIndex].commInfo.ntpSts[NTP_INDEX_DEAMON].level = SFM_ALM_MINOR;
				fimd_hdlhwNTPAlm(sysIndex, NTP_INDEX_DEAMON, 1);
				changeFlag = 1;
			}
			// when ntp daemon is down, channel also down.
			if(sfdb->sys[sysIndex].commInfo.ntpSts[NTP_INDEX_CHANNEL].level != SFM_ALM_MINOR){
				sfdb->sys[sysIndex].commInfo.ntpSts[NTP_INDEX_CHANNEL].level = SFM_ALM_MINOR;
				fimd_hdlhwNTPAlm(sysIndex, NTP_INDEX_CHANNEL, 1);
				changeFlag = 1;
			}
		}else if(rxSysCommMsg->hwNTP[NTP_INDEX_DEAMON] == NTP_INITIAL){
			;
		}else{
			if(sfdb->sys[sysIndex].commInfo.ntpSts[NTP_INDEX_DEAMON].level == SFM_ALM_MINOR){
				sfdb->sys[sysIndex].commInfo.ntpSts[NTP_INDEX_DEAMON].level = SFM_ALM_NORMAL;
				fimd_hdlhwNTPAlm(sysIndex, NTP_INDEX_DEAMON, 0);
				changeFlag = 1;
			}
		}
	}

	// NTP CHANNEL ALARM CHECK
	if (sfdb->sys[sysIndex].commInfo.ntpSts[NTP_INDEX_CHANNEL].mask != SFM_ALM_MASKED) {

		sfdb->sys[sysIndex].commInfo.ntpSts[NTP_INDEX_CHANNEL].status = rxSysCommMsg->hwNTP[NTP_INDEX_CHANNEL];

		if(rxSysCommMsg->hwNTP[NTP_INDEX_CHANNEL] == NTP_CRITICAL){

			if (chkCnt == 0) {
				alarm_dev_Time[sysIndex] = time(0);
				chkCnt = 1;  			
			}

			if((time(0) - alarm_dev_Time[sysIndex]) > NTP_ALARM_TIME){
				if(sfdb->sys[sysIndex].commInfo.ntpSts[NTP_INDEX_CHANNEL].level != SFM_ALM_MINOR){
					sfdb->sys[sysIndex].commInfo.ntpSts[NTP_INDEX_CHANNEL].level = SFM_ALM_MINOR;
					fimd_hdlhwNTPAlm(sysIndex, NTP_INDEX_CHANNEL, 1);
					changeFlag = 1;
				}

			}

		}else if(rxSysCommMsg->hwNTP[NTP_INDEX_CHANNEL] == NTP_INITIAL){
			alarm_dev_Time[sysIndex] = time(0);
			chkCnt = 0;	
		}else{
			if(sfdb->sys[sysIndex].commInfo.ntpSts[NTP_INDEX_CHANNEL].level == SFM_ALM_MINOR){
				sfdb->sys[sysIndex].commInfo.ntpSts[NTP_INDEX_CHANNEL].level = SFM_ALM_NORMAL;
				fimd_hdlhwNTPAlm(sysIndex, NTP_INDEX_CHANNEL, 0);
				alarm_dev_Time[sysIndex] = time(0);
				chkCnt = 0;
				changeFlag = 1;
			}
		}
	}


	// RSRC_LOAD
	for(i=0; i<SFM_MAX_RSRC_LOAD_CNT; i++){
		SFM_SysRSRCInfo *rsrcSts = &sfdb->sys[sysIndex].commInfo.rsrcSts[i];
		if((i > 1 && i < 5) || (i == 9) || (i == 14)) continue; // skip CDR, and null vlaue
		if(i > 15) break;                          // the end of RSRC

		if(sysIndex == 0) continue;
		if (rsrcSts->mask == SFM_ALM_MASKED) // mask된 놈은 상태관리에서 제외한다.
			continue;

		/* add by helca 2008.07.18 */
		if( rxSysCommMsg->rsrc_load.rsrcload[i] < 0 ){
			sprintf(trcBuf,"[fimd_hdlSysCommStsRpt] fail message rsrcLoad[%d]:%d\n", i, rxSysCommMsg->rsrc_load.rsrcload[i]);	
			trclib_writeLogErr (FL,trcBuf);
			continue;
		} else if ( (rxSysCommMsg->rsrc_load.rsrcload[DEF_MMDB_SESS] > 500000) || (rxSysCommMsg->rsrc_load.rsrcload[DEF_MMDB_OBJ] > 150000) 
				|| (rxSysCommMsg->rsrc_load.rsrcload[DEF_MMDB_SESS2] > 500000) || (rxSysCommMsg->rsrc_load.rsrcload[DEF_MMDB_OBJ2] > 150000)
				|| (rxSysCommMsg->rsrc_load.rsrcload[DEF_MMDB_CALL] > 150000) || (rxSysCommMsg->rsrc_load.rsrcload[DEF_MMDB_WAP1] > 5000)
				|| (rxSysCommMsg->rsrc_load.rsrcload[DEF_MMDB_WAP2] > 5000) || (rxSysCommMsg->rsrc_load.rsrcload[DEF_MMDB_HTTP] > 5000)
				|| (rxSysCommMsg->rsrc_load.rsrcload[DEF_MMDB_VODS] > 20000) || (rxSysCommMsg->rsrc_load.rsrcload[DEF_MMDB_VT] > 40000)
				|| (rxSysCommMsg->rsrc_load.rsrcload[DEF_MMDB_UDR] > 150000)) {

			continue;
		} else {
			//yhshin rsrcSts->rsrcload = ntohl(rxSysCommMsg->rsrc_load.rsrcload[i]);
			rsrcSts->rsrcload = (rxSysCommMsg->rsrc_load.rsrcload[i]);
		}

		if (fimd_hdlRsrcStsAlm (sysIndex, i, rsrcSts->rsrcload))
			changeFlag = 1;
	}

	/*
		OMC의 기타 상태 정보를 체크한다.  (etc. mysql)
		by sjjeon 2009/10/06
	 */
	if(sysIndex == 0)
	{
		SFM_HpUxHWInfo  *hwOmpInfo=NULL;
		SFM_SysSts	 	*pOmpHWSts;

		hwOmpInfo = (SFM_HpUxHWInfo*)&sfdb->sys[sysIndex].specInfo.u.sms.hpuxHWInfo;
		pOmpHWSts = &rxSysCommMsg->sysSts;

		for(i=0; i<1; i++) // 현재는  mysql만 체크한다.
		{
			if(hwOmpInfo->hwcom[i].mask != SFM_ALM_MASKED) // mask된 놈은 상태관리에서 제외한다.
			{		
				// mysql check
				if(!strcasecmp((char*)pOmpHWSts->linkSts[0].StsName,"mysql"))
				{
					param.devKind = SCM_MYSQL; // SCM Mysql과 동일한 Level 설정.
					hwOmpInfo->hwcom[i].level = fimd_getAbnormalAlarmLevel_byDevName(&param);
					hwOmpInfo->hwcom[i].prevStatus = hwOmpInfo->hwcom[i].status;
					hwOmpInfo->hwcom[i].status = pOmpHWSts->linkSts[i].status;

					if(hwOmpInfo->hwcom[i].prevStatus != hwOmpInfo->hwcom[i].status)
					{
						fimd_hdlHwComAlm (sysIndex, hwOmpInfo, i);
						changeFlag = 1;
						sprintf(trcBuf,"[%s]SysIdx:%d, CHANGE STATUS(%s) %d->%d\n",
								__FUNCTION__,sysIndex, hwOmpInfo->hwcom[i].name,
								hwOmpInfo->hwcom[i].prevStatus, hwOmpInfo->hwcom[i].status);	
						trclib_writeLogErr (FL,trcBuf);
						//fprintf(stderr,"omp mysql check..\n");

						// OMP DB와의 접속이 끊어 졌으므로... 재접속을 시도한다.
						if(hwOmpInfo->hwcom[i].prevStatus == 1){ // 1: not running
							if(fimd_mysql_init()<0){
								sprintf(trcBuf,"[%s] mysql reconnect fail\n", __FUNCTION__);
								trclib_writeLogErr (FL,trcBuf);
							}else{
								sprintf(trcBuf,"[%s] mysql reconnect(omp).\n", __FUNCTION__);
								trclib_writeLogErr (FL,trcBuf);
							}
						}
					}
				}
			}/* if() */
		}/* for() */
	}

#define _NEW_HW_STRC_
#ifdef _NEW_HW_STRC_ 

	/*
	   by sjjeon 2009.07
	   기존의 HW 상태 정보 체크 하는 부분을 수정한다. (단순화 시킨다)
	   SFM_SysCommMsgType.SFM_HWSts[40]
	 */ 
	
	if((sysIndex == 1)||(sysIndex == 2)) // sysIndex -> 1:SCMA, 2:SCMB
	{
		int idx=sysIndex-1; // idx를 설정... sysindex -1

		hwInfo = (SFM_HpUxHWInfo*)&sfdb->sys[sysIndex].specInfo.u.sms.hpuxHWInfo;
		pHWSts = &rxSysCommMsg->sysHW[0];

		for(i=0; i<SFM_MAX_HPUX_HW_COM; i++)
		{
			if(hwInfo->hwcom[i].mask == SFM_ALM_MASKED) // mask된 놈은 상태관리에서 제외한다.
				continue;

			// NOT_EQUIP(4)
			if(pHWSts[i].status == 4){ 
				hwInfo->hwcom[i].level = 4;
				continue;
			}

			//if(hwInfo->hwcom[i].name==NULL) break;
			if(!strcasecmp(hwInfo->hwcom[i].name, "noname")) break;
			if(pHWSts[i].StsName==NULL) break;

			if( !strcasecmp(hwInfo->hwcom[i].name, (char*)pHWSts[i].StsName)) 
			{
				// name으로 device number를 획득한다.	
				param.devKind = get_devKind_byName(hwInfo->hwcom[i].name);
				if(param.devKind<0)
				{
					sprintf(trcBuf,"[%s] INVALID DEVICE NAME(%s),l=%d. rv = %d\n",
							__FUNCTION__, pHWSts[i].StsName,i, param.devKind);
					trclib_writeLogErr (FL,trcBuf);
					return -1;
				}

				// 허위 알람으로 re-try check를 넣는다. sjjeon
				if(pHWSts[i].status == 1){				// down:1
					if(g_hw_retry_cnt[idx][i] < 2){		// retry count 2 이하.
						g_hw_retry_cnt[idx][i]+=1;
#if 0
						{
							// H/W 상태가 비정상적일 때.....전체 상태를 로그로 남긴다. sjjeon
							int k=0;
							char logTmp[2048],tmp[100];
							bzero(logTmp,sizeof(logTmp));
							bzero(tmp, sizeof(tmp));
							sprintf(logTmp,"[DEBUG] SYS-HW INFO\n");
							for(k=0; k<SFM_MAX_HPUX_HW_COM; k++)
							{
								sprintf(tmp, "index:%d, status:%d\n",k, pHWSts[k].status);
								strcat(logTmp, tmp);
								bzero(tmp, sizeof(tmp));
							}
							trclib_writeLogErr (FL,logTmp);
						}
#endif
#if 0
						sprintf(trcBuf,"[%s]SysIdx:%d,H/W(idx:%d), RE-TRY CHECK(%s) %d->%d, cnt:%d\n",
								__FUNCTION__,sysIndex,i, hwInfo->hwcom[i].name,
								hwInfo->hwcom[i].status, pHWSts[i].status, g_hw_retry_cnt[idx][i]);	
						trclib_writeLogErr (FL,trcBuf);
#endif
						continue;
					}
				}
				
				g_hw_retry_cnt[idx][i]=0;
				// abnormal alarm level을 설정한다. 
				hwInfo->hwcom[i].level = fimd_getAbnormalAlarmLevel_byDevName(&param);
				hwInfo->hwcom[i].prevStatus = hwInfo->hwcom[i].status;
				hwInfo->hwcom[i].status = pHWSts[i].status;

				//fprintf(stderr,"hw:%s, prests: %d, stat:%d, level:%d\n",
				//		hwInfo->hwcom[i].name, hwInfo->hwcom[i].prevStatus,
				//		hwInfo->hwcom[i].status, hwInfo->hwcom[i].level);

				if (hwInfo->hwcom[i].prevStatus != hwInfo->hwcom[i].status) 
				{
					fimd_hdlHwComAlm (sysIndex, hwInfo, i);
					changeFlag = 1;
					sprintf(trcBuf,"[%s]SysIdx:%d, CHANGE STATUS(%s) %d->%d\n",
							__FUNCTION__,sysIndex, hwInfo->hwcom[i].name,
							hwInfo->hwcom[i].prevStatus, hwInfo->hwcom[i].status);	
					trclib_writeLogErr (FL,trcBuf);
				} 
			}
		} /* for() */
	} /* if() */

#else  /* NOT _NEW_HW_STRC_ */

	// H/W Status by helca 2008.07.22
	hwInfo = &sfdb->sys[sysIndex].specInfo.u.sms.hpuxHWInfo;
	hwsts  = &rxSysCommMsg->sysSts;
	/*
		H/W 상태 체크  
		1 개의 for문에서 관리하도록 소스 수정.
		추후 확인.
	*/
	for(i=0; i<SFM_MAX_HPUX_HW_COM; i++)
	{
		if(hwInfo->hwcom[i].mask == SFM_ALM_MASKED) // mask된 놈은 상태관리에서 제외한다.
			continue;
		goContinue = 0;  // continue init
			sprintf(trcBuf,"[H/W status check] check!!! \n");
			trclib_writeLogErr (FL,trcBuf);

		// FAN
		for( j=0; j<SFM_HW_MAX_FAN_NUM; j++) { // fan1 ~
			if(strlen(hwsts->linkSts[j].StsName) > 0)
			{
				if( !strcasecmp(hwInfo->hwcom[i].name, hwsts->fanSts[j].StsName)) {
					param.devKind = SCM_FAN;
					hwInfo->hwcom[i].level = fimd_getAbnormalAlarmLevel_byDevName(&param);
				//	hwInfo->hwcom[i].level = SFM_ALM_CRITICAL;
					hwInfo->hwcom[i].prevStatus = hwInfo->hwcom[i].status;
					hwInfo->hwcom[i].status = hwsts->fanSts[j].status;

					if (hwInfo->hwcom[i].prevStatus != hwInfo->hwcom[i].status) {
						fimd_hdlHwComAlm (sysIndex, hwInfo, i);
						changeFlag = 1;
						sprintf(trcBuf,"[%s] CHANGE STATUS : %s \n",__FUNCTION__, hwInfo->hwcom[i].name);	
						trclib_writeLogErr (FL,trcBuf);
					} 
					// 상태 설정후, break;
					goContinue =1;
					break;

				} else {
					continue;
				}
			}
		}
		// 상태값이 바뀌면 contiue...
		if(goContinue==1) continue;

		// POWER
		for( j=0; j<SFM_HW_MAX_PWR_NUM; j++) { // pwr1 ~ 
			if(strlen(hwsts->linkSts[j].StsName) > 0)
			{
				if( !strcasecmp(hwInfo->hwcom[i].name, hwsts->pwrSts[j].StsName)) {
					param.devKind = SCM_PWR;
					hwInfo->hwcom[i].level = fimd_getAbnormalAlarmLevel_byDevName(&param);
					//hwInfo->hwcom[i].level = SFM_ALM_CRITICAL;
					hwInfo->hwcom[i].prevStatus = hwInfo->hwcom[i].status;
					hwInfo->hwcom[i].status = hwsts->pwrSts[j].status;

					if (hwInfo->hwcom[i].prevStatus != hwInfo->hwcom[i].status) {
						fimd_hdlHwComAlm (sysIndex, hwInfo, i);
						changeFlag = 1;
						sprintf(trcBuf,"[%s]h/w:%d, change status : %s, stat(%d->%d)\n", __FUNCTION__,i, 
								hwInfo->hwcom[i].name, hwInfo->hwcom[i].prevStatus, hwsts->pwrSts[j].status);
						trclib_writeLogErr (FL,trcBuf);
					}	
					// 상태 설정후, break;
					goContinue =1;
					break;
				} else {
					continue;
				}
			}
		}
		// 상태가 설정되면 continue
		if(goContinue==1) continue;

		// LINK (PORT)
		for( j=0; j<SFM_HW_MAX_LINK_NUM; j++) { // link0 ~ 
			if(strlen(hwsts->linkSts[j].StsName) > 0)
			{
				if( !strcasecmp(hwInfo->hwcom[i].name, hwsts->linkSts[j].StsName)) {
					//fprintf(stdout,"sysIndex : %d, sadb->hwname(%d): %s ,sadb->lName: %s, sadb->sts: %d \n", 
					//		sysIndex, j, hwsts->linkSts[j].StsName, hwsts->linkSts[j].StsName, hwsts->linkSts[j].status);
					if( !strncasecmp(hwInfo->hwcom[i].name, "SMCON",5) ||
						!strncasecmp(hwInfo->hwcom[i].name, "SMSYN",5))
					{
						param.devKind = SCM_SM_STAT;
						hwInfo->hwcom[i].level = fimd_getAbnormalAlarmLevel_byDevName(&param);
						//hwInfo->hwcom[i].level = SFM_ALM_MAJOR; // SM conn, sync는 major
					}else{
						param.devKind = SCM_PORT;
						hwInfo->hwcom[i].level = fimd_getAbnormalAlarmLevel_byDevName(&param);
						//hwInfo->hwcom[i].level = SFM_ALM_CRITICAL;
					}
					hwInfo->hwcom[i].prevStatus = hwInfo->hwcom[i].status;
					hwInfo->hwcom[i].status = hwsts->linkSts[j].status;


					if (hwInfo->hwcom[i].prevStatus != hwInfo->hwcom[i].status) {
						fimd_hdlHwComAlm (sysIndex, hwInfo, i);
						changeFlag = 1;
						sprintf(trcBuf,"[%s] change status : %s \n",__FUNCTION__, hwInfo->hwcom[i].name);	
						trclib_writeLogErr (FL,trcBuf);
					}
					// 상태 설정후, break;
					goContinue =1;
					break;
				} else {
					continue;
				}
			}
		}
		// 상태가 설정되면 continue
		if(goContinue==1) continue;
/*
   		// SCM H/W CPU 상태 감시는 DSC에서 제외...
		// CPU
		for( j=0; j<SFM_HW_MAX_CPU_NUM; j++) { // cpu0 ~ 
			if(strlen(hwsts->linkSts[j].StsName) > 0)
			{
				if( !strcasecmp(hwInfo->hwcom[i].name, hwsts->cpuSts[j].StsName)) {
					 hwInfo->hwcom[i].level = fimd_getAbnormalAlarmLevel_byDevName(SCM_PORT);
					//hwInfo->hwcom[i].level = SFM_ALM_CRITICAL;
					hwInfo->hwcom[i].prevStatus = hwInfo->hwcom[i].status;
					hwInfo->hwcom[i].status = hwsts->cpuSts[j].status;
					if (hwInfo->hwcom[i].prevStatus != hwInfo->hwcom[i].status) {
						fimd_hdlHwComAlm (sysIndex, hwInfo, i);
						changeFlag = 1;
						sprintf(trcBuf,"[%s] change status : %s \n",__FUNCTION__, hwInfo->hwcom[i].name);	
						trclib_writeLogErr (FL,trcBuf);
					}
					// 상태 설정후, break;
					goContinue =1;
					break;
				} else {
					continue;
				}
			}
		}
		// 상태가 설정되면 continue
		if(goContinue==1) continue;
*/
		// DISK
		for( j=0; j<SFM_HW_MAX_DISK_NUM; j++) { // disk0 ~
			if( !strcasecmp(hwInfo->hwcom[i].name, hwsts->diskSts[j].StsName)) {
				param.devKind = SCM_DISK;
				hwInfo->hwcom[i].level = fimd_getAbnormalAlarmLevel_byDevName(&param);
				//fprintf(stderr,"disk(%d) : %d\n",i,hwInfo->hwcom[i].level);
				//hwInfo->hwcom[i].level = SFM_ALM_CRITICAL;
                hwInfo->hwcom[i].prevStatus = hwInfo->hwcom[i].status;
                hwInfo->hwcom[i].status = hwsts->diskSts[j].status;
				
				if (hwInfo->hwcom[i].prevStatus != hwInfo->hwcom[i].status) {
                    fimd_hdlHwComAlm (sysIndex, hwInfo, i);
                    changeFlag = 1;
					sprintf(trcBuf,"[%s] change status : %s \n",__FUNCTION__, hwInfo->hwcom[i].name);	
					trclib_writeLogErr (FL,trcBuf);
					// 상태 설정후, break;
					goContinue =1;
                }
				break;
			} else {
				continue;
			}
		}

	}
#endif 

	if( sysIndex ){
		//SM Connection Status

		SFM_SMChInfo *smChSts = &sfdb->sys[sysIndex].commInfo.smChSts;
		int           smChEvent, status, level, errOccur, actOrStnby;
#if 0
		sprintf(trcBuf,"SM LINK CHECK(DN=0,NM=0) :SYS=%d CH0=%d:%d CH1=%d:%d CH2=%d:%d CH3=%d:%d CH4=%d:%d\n",
				sysIndex,
				ntohl(rxSysCommMsg->smConn[0].dConn), smChSts->each[0].level,
				ntohl(rxSysCommMsg->smConn[1].dConn), smChSts->each[1].level,
				ntohl(rxSysCommMsg->smConn[2].dConn), smChSts->each[2].level,
				ntohl(rxSysCommMsg->smConn[3].dConn), smChSts->each[3].level,
				ntohl(rxSysCommMsg->smConn[4].dConn), smChSts->each[4].level);
		trclib_writeLogErr(FL,trcBuf);
#endif
		for( i = 0, smChEvent = 0, errOccur = 0; i< SFM_MAX_SM_CH_CNT; i++ ){
			if( smChSts->each[i].mask == SFM_ALM_MASKED ){
				continue;
			}

			// STANDBY MODE 인 경우에는 NORMAL로 setting, 1st version added by uamyd 20110504
			if( sfdb->sys[sysIndex].commInfo.systemDup.myStatus == SYS_STATUS_ACTIVE ){

				/** 일단 값을 바꿔준다. */
				status = ntohl(rxSysCommMsg->smConn[i].dConn);
				if( status == -1 ){
					status = SFM_SM_CONN_STATUS_LINK_DN;
				}
				actOrStnby = 1;
			}else{
				status = SFM_SM_CONN_STATUS_LINK_UP;
				actOrStnby = 0;
			}

			smChSts->each[i].preStatus = smChSts->each[i].status;
			smChSts->each[i].status    = status;

			//level 도 확인해 준다. 
			if( status != SFM_SM_CONN_STATUS_LINK_UP ){
				level  = SFM_ALM_CRITICAL;
				errOccur++;
			} else {
				level  = SFM_ALM_NORMAL;
			}


			if( (smChSts->each[i].preStatus != smChSts->each[i].status) ||
				(smChSts->each[i].level     != level ) ){

				smChSts->each[i].level = level;

				fimd_checkSMChStsAlm(sysIndex, smChSts, i);
				fimd_backupSMChSts2File();
				smChEvent++;
			}

		}

		if( errOccur ){
			smChSts->level = SFM_ALM_CRITICAL;
		}

		if( smChEvent ){
			changeFlag++;
		}
	}

	
	if (changeFlag)
	{
		// 장애상태가 변경된 경우,
		// - 시스템 전체 장애 등급 및 현재 장애 갯수 정보를 update한다.
		// - 시스템 전체 등급이 변경된 경우, alarm level change 상태 메시지를 만들어 cond로 보낸다.
		// - GUI client로 event port를 통해 이를 통보한다.
		//	-> 이를 수신하면 alarm_history DB를 다시 읽어 current alarm list를 update한다.

			//sprintf(trcBuf,"[fimd_stsrpt:fault]scma : [%d], scmb : [%d]\n",sfdb->sys[1].commInfo.systemDup.myStatus,sfdb->sys[2].commInfo.systemDup.myStatus );	
			//trclib_writeLogErr (FL,trcBuf);

		fimd_updateSysAlmInfo (sysIndex);
		//for(i=0; i<2; i++) fimd_updatePDAlmInfo(i); // fimd_updatePDAlmInfo은 l3pd 쪽에서 up
		fimd_broadcastAlmEvent2Client ();
	}
#if 0
	sprintf(trcBuf,"[fimd_stsrpt:fault]scma : [%d], scmb : [%d]\n",sfdb->sys[1].commInfo.systemDup.myStatus,sfdb->sys[2].commInfo.systemDup.myStatus );
	trclib_writeLogErr (FL,trcBuf);
#endif

	return 1;

} //----- End of fimd_hdlSysCommStsRpt -----//


int fimd_hdll3pdStsRpt (IxpcQMsgType *rxIxpcMsg)
{
	int		devIndex, i, j;
	int		changeFlag=0; // 장애 발생 또는 해지인 경우 setting된다
	SFM_L3PD	*rxl3pdMsg;
	// 메시지를 보낸 시스템 이름과 일치하는 index를 찾는다.
	if ((devIndex = fimd_getSysIndexByName (rxIxpcMsg->head.srcSysName)) < 0) {
		sprintf(trcBuf,"[%s] unknown sysName[%s]\n",__FUNCTION__, rxIxpcMsg->head.srcSysName);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	rxl3pdMsg = (SFM_L3PD*)rxIxpcMsg->body;

	//ntoh : sjjeon
	SFM_L3PD_N2H((SFM_L3PD*)rxl3pdMsg);
	
	// TAP CPU 사용율
	for(i=0; i<2; i++){
		if (l3pd->l3ProbeDev[i].cpuInfo.mask == SFM_ALM_MASKED)
			continue;
		if(fimd_checkPDCpuUsageAlm (i, &l3pd->l3ProbeDev[i].cpuInfo)){
			changeFlag = 1;
			sprintf(trcBuf,"[%s] status changed, TAP(%d), CPU level:%d \n",__FUNCTION__, i, l3pd->l3ProbeDev[i].cpuInfo.level);
			trclib_writeLogErr (FL,trcBuf);
		}
		
	}
	// TAP Memory 사용율
	for(i=0; i<2; i++){ 
		if(l3pd->l3ProbeDev[i].memInfo.mask == SFM_ALM_MASKED)
			continue;
		if(fimd_checkPDMemUsageAlm(i, &l3pd->l3ProbeDev[i].memInfo)){
			changeFlag = 1;
			sprintf(trcBuf,"[%s] status changed, TAP(%d), MEM level:%d \n",__FUNCTION__, i, l3pd->l3ProbeDev[i].memInfo.level);
			trclib_writeLogErr (FL,trcBuf);
		}
	}
	
	// TAP Port 상태
#if 1
	for(i=0; i<2; i++){
		for(j=0; j<MAX_GIGA_LAN_NUM; j++){
			if(l3pd->l3ProbeDev[i].gigaLanInfo[j].mask != SFM_ALM_MASKED)
			{
				if(i>2) break;
				if(fimd_checkCommonStatusAlm(TAPA+i,TAP_PORT,j)==1)
					changeFlag = 1;
			}
		}
	}

#else
	for(i=0; i<2; i++){
		for(j=0; j<MAX_GIGA_LAN_NUM; j++ )
		{
			if(l3pd->l3ProbeDev[i].gigaLanInfo[j].mask == SFM_ALM_MASKED)
				continue;
			if ( l3pd->l3ProbeDev[i].gigaLanInfo[j].status == L3PD_RJ45_STS_PHYSIDOWN ){
				// 현재 : down, 이전 : up (nor -> cri)
				if(l3pd->l3ProbeDev[i].gigaLanInfo[j].level != SFM_ALM_CRITICAL){
					fimd_hdlGigaLanAlm (i, j, SFM_ALM_CRITICAL, 1);
					l3pd->l3ProbeDev[i].gigaLanInfo[j].level = SFM_ALM_CRITICAL;
					changeFlag = 1;
					sprintf(trcBuf,"[%s] status changed, TAP(%d), Port[%d], Level:%d, stat:%d->%d \n", __FUNCTION__, i, j, l3pd->l3ProbeDev[i].gigaLanInfo[j].level,l3pd->l3ProbeDev[i].gigaLanInfo[j].prevStatus, l3pd->l3ProbeDev[i].gigaLanInfo[j].status);
					trclib_writeLogErr (FL,trcBuf);
				}
			}else if( l3pd->l3ProbeDev[i].gigaLanInfo[j].status == L3PD_RJ45_STS_PROTODOWN ){
				if(j < MAX_GIGA_LAN_NUM){
					// 현재 : down (protodown), 이전 : up
					if(l3pd->l3ProbeDev[i].gigaLanInfo[j].level != SFM_ALM_CRITICAL){
						fimd_hdlGigaLanAlm (i, j, SFM_ALM_CRITICAL, 1);
						l3pd->l3ProbeDev[i].gigaLanInfo[j].level = SFM_ALM_CRITICAL;
						changeFlag = 1;
						sprintf(trcBuf,"[%s] status changed, TAP(%d), Port[%d], Level:%d, stat:%d->%d \n", __FUNCTION__, i, j, l3pd->l3ProbeDev[i].gigaLanInfo[j].level,l3pd->l3ProbeDev[i].gigaLanInfo[j].prevStatus, l3pd->l3ProbeDev[i].gigaLanInfo[j].status);
						trclib_writeLogErr (FL,trcBuf);
					}
				}else{
					if(l3pd->l3ProbeDev[i].gigaLanInfo[j].level != SFM_ALM_NORMAL){
						fimd_hdlGigaLanAlm (i, j, SFM_ALM_NORMAL, 0);
						//fimd_hdlGigaLanAlm (i, j, SFM_ALM_CRITICAL, 0); // 장애 복구시 NORMAL로 DB 설정을 CRITICAL Clear로 표현하는게 맞다. sjjeon
						l3pd->l3ProbeDev[i].gigaLanInfo[j].level = SFM_ALM_NORMAL;
						changeFlag = 1;
						sprintf(trcBuf,"[%s] status changed, TAP(%d), Port[%d], Level:%d, stat:%d->%d \n", __FUNCTION__, i, j, l3pd->l3ProbeDev[i].gigaLanInfo[j].level,l3pd->l3ProbeDev[i].gigaLanInfo[j].prevStatus, l3pd->l3ProbeDev[i].gigaLanInfo[j].status);
						trclib_writeLogErr (FL,trcBuf);
					}
				}	
			}else{
				// L3PD_RJ45_STS_PHYSIUP
				if(l3pd->l3ProbeDev[i].gigaLanInfo[j].level != SFM_ALM_NORMAL){
					fimd_hdlGigaLanAlm (i, j, SFM_ALM_NORMAL, 0);
					//fimd_hdlGigaLanAlm (i, j, SFM_ALM_CRITICAL, 0); // 장애 복구시 NORMAL로 DB 설정을 CRITICAL Clear로 표현하는게 맞다. sjjeon l3pd->l3ProbeDev[i].gigaLanInfo[j].level = SFM_ALM_NORMAL;
					changeFlag = 1;
					sprintf(trcBuf,"[%s] status changed, TAP(%d), Port[%d], Level:%d, stat:%d->%d \n", __FUNCTION__, i, j, l3pd->l3ProbeDev[i].gigaLanInfo[j].level,l3pd->l3ProbeDev[i].gigaLanInfo[j].prevStatus, l3pd->l3ProbeDev[i].gigaLanInfo[j].status);
					trclib_writeLogErr (FL,trcBuf);
				}
			}
			l3pd->l3ProbeDev[i].gigaLanInfo[j].prevStatus = l3pd->l3ProbeDev[i].gigaLanInfo[j].status;
		}
	}
#endif 
	/* TAP POWER status 20110422 by dcham */
	for(i=0; i<2; i++){
		for(j=0; j<MAX_POWER_NUM; j++){
			if(l3pd->l3ProbeDev[i].powerInfo[j].mask != SFM_ALM_MASKED)
			{
				//sprintf(trcBuf,"[%s] TAP[%d], Power[%d], Level[%d], stat[%d]->[%d] \n", __FUNCTION__, i, j, l3pd->l3ProbeDev[i].powerInfo[j].level,l3pd->l3ProbeDev[i].powerInfo[j].prevStatus, l3pd->l3ProbeDev[i].powerInfo[j].status);
				//trclib_writeLogErr (FL,trcBuf);
				if(i>2) break;
				if(fimd_checkCommonStatusAlm(TAPA+i,TAP_POWER,j)==1)
					changeFlag = 1;
			}
		}
	}
	if (changeFlag)
	{
		// 장애상태가 변경된 경우,
		// - 시스템 전체 장애 등급 및 현재 장애 갯수 정보를 update한다.
		// - 시스템 전체 등급이 변경된 경우, alarm level change 상태 메시지를 만들어 cond로 보낸다.
		// - GUI client로 event port를 통해 이를 통보한다.
		//	-> 이를 수신하면 alarm_history DB를 다시 읽어 current alarm list를 update한다.
		for(i=0; i<2; i++) fimd_updatePDAlmInfo(i);
		fimd_broadcastAlmEvent2Client ();

	}

	return 1;
}

int fimd_hdleSCEStsRpt (IxpcQMsgType *rxIxpcMsg)
{
	int		devIndex, i, j;
	int		changeFlag=0; // 장애 발생 또는 해지인 경우 setting된다
	SFM_SCE	*rxSCEMsg;

	// 메시지를 보낸 시스템 이름과 일치하는 index를 찾는다.
	if ((devIndex = fimd_getSysIndexByName (rxIxpcMsg->head.srcSysName)) < 0) {
		sprintf(trcBuf,"[%s] unknown sysName[%s]\n",__FUNCTION__, rxIxpcMsg->head.srcSysName);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	rxSCEMsg = (SFM_SCE*)rxIxpcMsg->body;

	// SCE Device CPU 사용율
	for(i=0; i<MAX_SCE_DEV_NUM; i++){
		for(j=0; j<MAX_SCE_CPU_CNT; j++){
			if (g_pstSCEInfo->SCEDev[i].cpuInfo[j].mask == SFM_ALM_MASKED)
				continue;

			if(fimd_checkSceCpuUsageAlm (i, j)){
				sprintf(trcBuf,"[%s] status changed, SCE(%d), CPU(%d),Level :%d \n",__FUNCTION__, i,j, g_pstSCEInfo->SCEDev[i].cpuInfo[j].level);
				trclib_writeLogErr (FL,trcBuf);
				changeFlag = 1;
			}
		}
	}

	// SCE Device MEMORY 사용율
	for(i=0; i<MAX_SCE_DEV_NUM; i++){
		for(j=0; j<MAX_SCE_MEM_CNT; j++){
			if (g_pstSCEInfo->SCEDev[i].memInfo[j].mask == SFM_ALM_MASKED)
				continue;

			if(fimd_checkSceMemUsageAlm (i, j)){
				sprintf(trcBuf,"[%s] status changed, SCE(%d) MEM(%d). Level :%d \n",__FUNCTION__, i,j, g_pstSCEInfo->SCEDev[i].memInfo[j].level);
				trclib_writeLogErr (FL,trcBuf);
				changeFlag = 1;
			}
		}
	}

	// SCE Device DISK 사용율
	for(i=0; i<MAX_SCE_DEV_NUM; i++){
		if(g_pstSCEInfo->SCEDev[i].diskInfo.mask != SFM_ALM_MASKED){
			//if(fimd_checkSceDiskUsageAlm2(SCEA+i, SCE_DISK)) // by sjjeon
			if(fimd_checkSceDiskUsageAlm(i, 0))
			{
				sprintf(trcBuf,"[%s] status changed, SCE(%d) DISK. Level :%d \n",__FUNCTION__, i, g_pstSCEInfo->SCEDev[i].diskInfo.level);
				trclib_writeLogErr (FL,trcBuf);
				changeFlag = 1;
			}
		}
	}

	/* hjjung_20100823 */
	// SCE Device USER 사용율
	for(i=0; i<MAX_SCE_DEV_NUM; i++){
		if(g_pstSCEInfo->SCEDev[i].userInfo.mask != SFM_ALM_MASKED){
			if(fimd_checkSceUserUsageAlm(i, 0))
			{
				sprintf(trcBuf,"[%s] status changed, SCE(%d) USER. Level :%d \n",__FUNCTION__, i, g_pstSCEInfo->SCEDev[i].userInfo.level);
				trclib_writeLogErr (FL,trcBuf);
				changeFlag = 1;
			}
		}
	}

	// SCE Device STATUS Alarm check
	for(i=0; i<MAX_SCE_DEV_NUM; i++){
		if(g_pstSCEInfo->SCEDev[i].sysStatus.mask != SFM_ALM_MASKED){
			if(i>2) break;
			if(fimd_checkCommonStatusAlm(SCEA+i,SCE_STATUS,0)==1){
				sprintf(trcBuf,"[%s] status changed, SCE(%d) STATUS. level : %d\n",
						__FUNCTION__, i, g_pstSCEInfo->SCEDev[i].sysStatus.level);
				trclib_writeLogErr (FL,trcBuf);
				changeFlag = 1;
			}
		}	
	}

	// SCE Device POWER Alarm check
	for(i=0; i<MAX_SCE_DEV_NUM; i++){
		if(g_pstSCEInfo->SCEDev[i].pwrStatus.mask == SFM_ALM_MASKED)
			continue;
		if(fimd_checkCommonStatusAlm(SCEA+i,SCE_POWER,0)==1)
		{
			sprintf(trcBuf,"[%s] status changed, SCE(%d) Power. status :%d \n",__FUNCTION__, i, g_pstSCEInfo->SCEDev[i].pwrStatus.status);
			trclib_writeLogErr (FL,trcBuf);
			changeFlag = 1;
		}
	}

	// SCE Device FAN Alarm check
	for(i=0; i<MAX_SCE_DEV_NUM; i++){
		if(g_pstSCEInfo->SCEDev[i].fanStatus.mask == SFM_ALM_MASKED)
			continue;
		// by sjjeon
		if(fimd_checkCommonStatusAlm(SCEA+i,SCE_FAN,0)==1){
			sprintf(trcBuf,"[%s] status changed, SCE(%d) FAN. status :%d \n",__FUNCTION__, i, g_pstSCEInfo->SCEDev[i].fanStatus.status);
			trclib_writeLogErr (FL,trcBuf);
			changeFlag = 1;
		}
	}

	// SCE Device TEMP Alarm check
	for(i=0; i<MAX_SCE_DEV_NUM; i++){
		if(g_pstSCEInfo->SCEDev[i].tempStatus.mask == SFM_ALM_MASKED)
			continue;
#if 0
		if(fimd_checkSceTempStatusAlm(i, 0)){
			sprintf(trcBuf,"[%s] status changed, SCE(%d) TEMP. status :%d \n",__FUNCTION__, i, g_pstSCEInfo->SCEDev[i].tempStatus.status);
			trclib_writeLogErr (FL,trcBuf);
			changeFlag = 1;
		}
#else
		//sjjeon
		if(fimd_checkCommonStatusAlm(SCEA+i,SCE_TEMP,0)==1)
		{
			sprintf(trcBuf,"[%s] status changed, SCE(%d) TEMP. status :%d \n",__FUNCTION__, i, g_pstSCEInfo->SCEDev[i].tempStatus.status);
			trclib_writeLogErr (FL,trcBuf);
			changeFlag = 1;
		}
#endif
	}

	// SCE Device PORT Alarm check
	for(i=0; i<MAX_SCE_DEV_NUM; i++)
	{
		/** Mgmt port **/
		for(j=0; j< (MAX_SCE_IFN_CNT-4); j++)
		{
			if(g_pstSCEInfo->SCEDev[i].portStatus[j].mask != SFM_ALM_MASKED)
			{
				if(fimd_checkCommonStatusAlm(SCEA+i,SCE_PORT_MGMT,j)==1)	
				{
					sprintf(trcBuf,"[%s] status changed, SCE(%d) Port(%d) status :%d->%d \n", 
							__FUNCTION__, i,j,g_pstSCEInfo->SCEDev[i].portLinkStatus[j].preStatus, g_pstSCEInfo->SCEDev[i].portLinkStatus[j].status);
					trclib_writeLogErr (FL,trcBuf);
					changeFlag = 1;
				}
			}
		}

		/**< Link port **/
		for(j=2; j<MAX_SCE_IFN_CNT; j++)
		{
			if(g_pstSCEInfo->SCEDev[i].portStatus[j].mask != SFM_ALM_MASKED)
			{
				if(fimd_checkCommonStatusAlm(SCEA+i,SCE_PORT_LINK,j)==1)	
				{
					sprintf(trcBuf,"[%s] status changed, SCE(%d) Port(%d) status :%d->%d \n", 
							__FUNCTION__, i,j,g_pstSCEInfo->SCEDev[i].portLinkStatus[j].preStatus, g_pstSCEInfo->SCEDev[i].portLinkStatus[j].status);
					trclib_writeLogErr (FL,trcBuf);
					changeFlag = 1;
				}
			}
		}

	}

	// SCE Device RDR CONNECT Alarm check
	for(i=0; i<MAX_SCE_DEV_NUM; i++){

		for(j=0; j<MAX_SCE_RDR_INFO_CNT; j++){
			if(g_pstSCEInfo->SCEDev[i].rdrConnStatus[j].mask != SFM_ALM_MASKED)
			{
				if(i>2) break;
				if(fimd_checkCommonStatusAlm(SCEA+i,SCE_RDR_CONN,j)==1){
					sprintf(trcBuf,"[%s] status changed, SCE(%d) RDR_CONNECTION(%d). status :%d \n",__FUNCTION__, i,j, g_pstSCEInfo->SCEDev[i].rdrConnStatus[j].status);
					trclib_writeLogErr (FL,trcBuf);
					changeFlag = 1;
				}
			}
		}
	}

	if (changeFlag)
	{
		// 장애상태가 변경된 경우,
		// - 시스템 전체 장애 등급 및 현재 장애 갯수 정보를 update한다.
		// - 시스템 전체 등급이 변경된 경우, alarm level change 상태 메시지를 만들어 cond로 보낸다.
		// - GUI client로 event port를 통해 이를 통보한다.
		//	-> 이를 수신하면 alarm_history DB를 다시 읽어 current alarm list를 update한다.
		//for(i=0; i<2; i++) fimd_updatePDAlmInfo(i);
		fimd_updateSceAlmInfo();  /* 알람 정보 Update.  sjjeon*/
		fimd_broadcastAlmEvent2Client ();
	}
	return 1;
}
/* End of fimd_hdleSCEStsRpt */

/* hjjung_20100822 */
int fimd_hdleLEGStsRpt (IxpcQMsgType *rxIxpcMsg)
{
	int		devIndex, i, j;
	int		changeFlag=0; // 장애 발생 또는 해지인 경우 setting된다
	CALL_DATA *rxLEGMsg;

	// 메시지를 보낸 시스템 이름과 일치하는 index를 찾는다.
	if ((devIndex = fimd_getSysIndexByName (rxIxpcMsg->head.srcSysName)) < 0) {
		sprintf(trcBuf,"[%s] unknown sysName[%s]\n",__FUNCTION__, rxIxpcMsg->head.srcSysName);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	rxLEGMsg = (CALL_DATA*)rxIxpcMsg->body;

	if(!strcasecmp(rxIxpcMsg->head.srcSysName, "SCMA"))
		g_pstCALLInfo->legInfo[0].num = ntohl(rxLEGMsg->sess.amount);
	else
		g_pstCALLInfo->legInfo[1].num = ntohl(rxLEGMsg->sess.amount);

	for(j=0;j<2;j++){
		sprintf(trcBuf,"[%s] SESSION[%d]:%d\n",__FUNCTION__,j,g_pstCALLInfo->legInfo[j].num);
		trclib_writeLogErr (FL,trcBuf);
	}

	// RLEG Session 사용율
	for(i=0; i<MAX_CALL_DEV_NUM; i++){
		if(g_pstCALLInfo->legInfo[i].mask != SFM_ALM_MASKED){
			if(fimd_checkLegSessionUsageAlm(i))
			{
				sprintf(trcBuf,"[%s] status changed, RLEG(%d) Session. Level :%d \n",__FUNCTION__, i, g_pstCALLInfo->legInfo[i].level);
				trclib_writeLogErr (FL,trcBuf);
				changeFlag = 1;
			}
		}
	}

	if (changeFlag)
	{
		// 장애상태가 변경된 경우,
		// - 시스템 전체 장애 등급 및 현재 장애 갯수 정보를 update한다.
		// - 시스템 전체 등급이 변경된 경우, alarm level change 상태 메시지를 만들어 cond로 보낸다.
		// - GUI client로 event port를 통해 이를 통보한다.
		//	-> 이를 수신하면 alarm_history DB를 다시 읽어 current alarm list를 update한다.
		//for(i=0; i<2; i++) fimd_updatePDAlmInfo(i);
		fimd_updateLegAlmInfo();  /* 알람 정보 Update.  sjjeon*/
		fimd_broadcastAlmEvent2Client ();
	}
	return 1;
}
/* End of fimd_hdleRLEGStsRpt */

/* added by dcham 2011.05.25 for TPS */
int fimd_hdleTPSStsRpt (IxpcQMsgType *rxIxpcMsg)
{
	int		devIndex, i,j;
	int		changeFlag=0; // 장애 발생 또는 해지인 경우 setting된다
	CALL_DATA *rxLEGMsg;

	// 메시지를 보낸 시스템 이름과 일치하는 index를 찾는다.
	if ((devIndex = fimd_getSysIndexByName (rxIxpcMsg->head.srcSysName)) < 0) {
		sprintf(trcBuf,"[%s] unknown sysName[%s]\n",__FUNCTION__, rxIxpcMsg->head.srcSysName);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	if(!strcasecmp(rxIxpcMsg->head.srcSysName, "SCMA"))
		g_pstCALLInfo->tpsInfo[0].num = ntohl(rxLEGMsg->tps);
	else
		g_pstCALLInfo->tpsInfo[1].num = ntohl(rxLEGMsg->tps);
	for(j=0;j<2;j++){
		sprintf(trcBuf,"[%s] TPS(%d) : %d \n",__FUNCTION__, j, g_pstCALLInfo->tpsInfo[j].num);
		trclib_writeLogErr (FL,trcBuf);
	}

	// TPS 량 
	for(i=0; i<MAX_CALL_DEV_NUM; i++){
		if(g_pstCALLInfo->tpsInfo[i].mask != SFM_ALM_MASKED){
			if(fimd_checkTpsAlm(i))
			{
				sprintf(trcBuf,"[%s] status changed, RLEG(%d) TPS. Level :%d \n",__FUNCTION__, i, g_pstCALLInfo->tpsInfo[i].level);
				trclib_writeLogErr (FL,trcBuf);
				changeFlag = 1;
			}
		}
	}

	if (changeFlag)
	{
		// 장애상태가 변경된 경우,
		// - 시스템 전체 장애 등급 및 현재 장애 갯수 정보를 update한다.
		// - 시스템 전체 등급이 변경된 경우, alarm level change 상태 메시지를 만들어 cond로 보낸다.
		// - GUI client로 event port를 통해 이를 통보한다.
		//	-> 이를 수신하면 alarm_history DB를 다시 읽어 current alarm list를 update한다.
		//for(i=0; i<2; i++) fimd_updatePDAlmInfo(i);
		fimd_updateCallAlmInfo();  /* 알람 정보 Update.  2011.05.25 added by dcham*/
		fimd_broadcastAlmEvent2Client ();
	}
	return 1;
}
int fimd_hdleL2StsRpt (IxpcQMsgType *rxIxpcMsg)
{
	int		 devIndex, i, j ;
	int		changeFlag=0; // 장애 발생 또는 해지인 경우 setting된다
	SFM_L2Dev *rxL2DevMsg;

	// 메시지를 보낸 시스템 이름과 일치하는 index를 찾는다.
	if ((devIndex = fimd_getSysIndexByName (rxIxpcMsg->head.srcSysName)) < 0) {
		sprintf(trcBuf,"[fimd_hdleL2StsRpt] unknown sysName[%s]\n", rxIxpcMsg->head.srcSysName);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	rxL2DevMsg = (SFM_L2Dev *)rxIxpcMsg->body;
	SFM_L2DEV_H2N((SFM_L2Dev *)rxL2DevMsg );
	
	// L2SW CPU 사용율
	for(i=0; i<MAX_L2_DEV_NUM; i++){
		if (g_pstL2Dev->l2Info[i].cpuInfo.mask == SFM_ALM_MASKED)
			continue;
		// 상태체크 함수. min, maj, cri and alarm , db
		if(fimd_checkL2swCpuUsageAlm(i, &g_pstL2Dev->l2Info[i].cpuInfo)){
			changeFlag = 1;
			sprintf(trcBuf,"[%s] status changed, L2SW(%d), CPU level:%d \n",__FUNCTION__, i, g_pstL2Dev->l2Info[i].cpuInfo.level);
			trclib_writeLogErr (FL,trcBuf);
		}
	}

	// L2SW Memory 사용율
	for(i=0; i<MAX_L2_DEV_NUM; i++){ 
		if(g_pstL2Dev->l2Info[i].memInfo.mask == SFM_ALM_MASKED)
			continue;
		// 상태체크 함수. min, maj, cri and alarm , db
		if(fimd_checkL2swMemUsageAlm(i, &g_pstL2Dev->l2Info[i].memInfo)){
			changeFlag = 1;
			sprintf(trcBuf,"[%s] status changed, L2SW(%d), MEM level:%d \n",__FUNCTION__, i, g_pstL2Dev->l2Info[i].memInfo.level);
			trclib_writeLogErr (FL,trcBuf);
		}
	}

	// L2SW Port 상태
	for(i=0; i<MAX_L2_DEV_NUM; i++){
		for(j=0; j<MAX_L2_PORT_NUM; j++ ){
			if(g_pstL2Dev->l2Info[i].portInfo[j].mask == SFM_ALM_MASKED)
				continue;
			if(fimd_checkCommonStatusAlm(L2SWA+i,L2SW_PORT,j)==1){
				changeFlag = 1;
				sprintf(trcBuf,"[%s] status changed, L2SW(%d), Port[%d], Level:%d, stat:%d->%d \n", __FUNCTION__, i, j, 
						g_pstL2Dev->l2Info[i].portInfo[j].level,
						g_pstL2Dev->l2Info[i].portInfo[j].prevStatus, 
						g_pstL2Dev->l2Info[i].portInfo[j].status);
				trclib_writeLogErr (FL,trcBuf);
			}
		}
	}

	if (changeFlag)
	{
		// 장애상태가 변경된 경우,
		// - 시스템 전체 장애 등급 및 현재 장애 갯수 정보를 update한다.
		// - 시스템 전체 등급이 변경된 경우, alarm level change 상태 메시지를 만들어 cond로 보낸다.
		// - GUI client로 event port를 통해 이를 통보한다.
		//	-> 이를 수신하면 alarm_history DB를 다시 읽어 current alarm list를 update한다.
		for(i=0; i<MAX_L2_DEV_NUM; i++) fimd_updateL2SWlmInfo(i);
		fimd_broadcastAlmEvent2Client ();
	}



	return 1;
}

/*
   // by sjjeon
	- H/W,S/W 이름을 device number를 부여한다. 

 */
int get_devKind_byName(char *devName)
{
	int devNum=-1, i;

	if(devName == NULL)
	{
		sprintf(trcBuf,"[%s] invalid para.(parameter is null) \n",__FUNCTION__);
		trclib_writeLogErr (FL,trcBuf);
		return devNum;
	}
	
	for (i=0 ; i<lanConf.count ; i++)
	{
		if((strncasecmp(devName, lanConf.lanif[i].name, lanConf.lanif[i].name_size)==0))
			return SCM_PORT;

	}
 	
	if(!strcasecmp("mysql",devName)){
		devNum = SCM_MYSQL;
	}else if(!strcasecmp("timesten",devName)){
		devNum = SCM_TIMESTEN;
	}else if(!strcasecmp("sm",devName)){
		devNum = SCM_SM;
	}else if(!strcasecmp("cm",devName)){
		devNum = SCM_CM;
	}else if(!strncasecmp("SMCON",devName,5) || !strncasecmp("SMSYN",devName,5)){
		devNum = SCM_SM_STAT;
	}else if(!strncasecmp("DISK",devName,4)){
		devNum = SCM_DISK;
	}else if(!strncasecmp("FAN",devName,3)){
		devNum = SCM_FAN;
	}else if(!strncasecmp("PWR",devName,3)){
		devNum = SCM_PWR;
	}

	return devNum;
	
}
/* End of get_devKind_byName */

