#include "fimd_proto.h"

#define INTERFACE_IS(x) (x==1 ? "SMSC" : (x==2 ? "VAS" : (x==3 ? "LMSC" : (x==4 ? "IDR" :(x==5 ? "SCP" : (x==6 ? "WISE" : "UNKNOWN"))))))

//enum {TAPA=4,TAPB,SCEA,SCEB,L2SWA,L2SWB };

extern char		trcBuf[4096], trcTmp[1024];
extern SFM_sfdb		*sfdb;
extern SFM_L3PD		*l3pd;
extern SFM_SCE		*g_pstSCEInfo;
/* hjjung */
//extern SFM_LEG		*g_pstCALLInfo;
extern SFM_CALL		*g_pstCALLInfo; // added by dcham 20110525 for TPS
extern SFM_L2Dev	*g_pstL2Dev;
extern SFM_LOGON    *g_pstLogonRate;
extern SFM_LOGON	g_stLogonRate[LOG_MOD_CNT][2];

SFM_HpUxHWInfo		*hwInfo;
extern int			numMmcHdlr, eqSysCnt;
extern int			trcFlag, trcLogFlag;
extern int			*sound_flag;
char	     	   	resBuf[4096], resHead[1024];
extern FimdMmcHdlrVector        mmcHdlrVector[FIMD_MAX_MMC_HANDLER];

// by helca 08.02
extern char    *rsrcName[SFM_MAX_RSRC_LOAD_CNT];
extern char    *nmsifName[SFM_NMSIF_MASK_CNT]; // by helca 10.30
extern unsigned char		rmtdb_conn_lvl[SYSCONF_MAX_ASSO_SYS_NUM][SFM_MAX_DB_CNT];
extern MYSQL   *mysql_conn, sql;

char *alarmName[MAX_ALRAM_CNT] = {"CPU_USAGE", "MEMORY_USAGE", "DISK_USAGE", "LAN", "PROCESS"
								, "LINK", "MP_HW", "STAT", "DB_REP", "DB_REP_GAP"
								, "CONN_SERVER" , "CALL_INFO", "DUP_STS", "DUPLICATION H/B", "DUP_OOS"
								, "SUCCESS RATE" , "SESS_LOAD", "DB CONNECTION", "REMOTE NETWORK", "MIRRORING PORT"
								, "NETWORK TIME PROTOCOL", "TAP_CPU_USAGE", "TAP_MEMORY_USAGE" , "TAP_FAN_STS", "TAP_GIGA_LAN"
								, "RSRC_LOAD", "QUEUE_LOAD", "NMS_STATUS" , "DUAL_STD", "DUAL_STS_QRY_TIME_OUT"
								, "SCE_FAN", "SCE_CPU_USAGE", "SCE_MEMORY_USAGE" , "SCE_DISK_USAGE", "SCE_POWER_STS" 		/* 31 ~ 35*/
								, "SCE_FAN_STS", "SCE_TEMP_STS", "SCE_VOLT_STS","SCE_PORT_STS", "SCE_RDR_STS"   			/* 36 ~ 40*/
								, "SCE_RDR_CONN_STS", "SCE_STATUS", "L2_CPU_USAGE", "L2_MEM_USAGE", "L2_LAN_USAGE" 			/* 41 ~ 45*/
								, "CPS_OVER_STS", "SCE_PORT_LINK", "PROCESS_SAMD", "PROCESS_IXPC","ROCESS_FIMD"				/* 46 ~ 50 */
								, "PROCESS_COND", "PROCESS_STMD", "PROCESS_MMCD", "PROCESS_MCMD", "PROCESS_NMSIF "  		/* 51 ~ 55 */
								, "PROCESS_CDELAY", "PROCESS_HAMON", "PROCESS_MMCR", "PROCESS_RDRANA ", "PROCESS_RLEG "  	/* 56 ~ 60 */
								, "PROCESS_SMPP", "PROCESS_PANA", "PROCESS_RANA", "PROCESS_RDRCAPD", "PROCESS_CAPD" 		/* 61 ~ 65 */
								, "HW_MIRROR", "TAP_MGMT_STS", "L2SW_MGMT_STS", "ACTIVE_STS", "SCE_USER"					/* 66 ~ 70 */
								, " ","LEG_SESSION_USAGE", " ", "SCM_FAULT"	, "LOGON_SUCCESS_RATE"							/* 71 ~ 75 */
								, " "," ", " ", "LOGOUT_SUCCESS_RATE", "TAP_POWER_STS"                                      /* 76 ~ 80 */
								, "SM_CONNECTION","PROCESS_RLEG0","PPROCESS_RLEG1","PROCESS_RLEG2","PROCESS_RLEG3"          /* 81 ~ 85 */
								, "PROCESS_RLEG4","TPS_LOAD"  															    /* 86 ~ 87 */
};

void fimd_mmc_makeL2swSysMaskStsOutputAllMsg (IxpcQMsgType *rxIxpcMsg, int sysIndex, int *lineCnt, char *seqNo);
void fimd_mmc_makeL2swSysMaskStsOutputMsg ( IxpcQMsgType *rxIxpcMsg, int sysIndex, int *lineCnt, char *seqNo);
extern void hw_name_mapping(char *s1, int slen, char *s2);

int fimd_mmc_dis_node_info (IxpcQMsgType *rxIxpcMsg)
{
	char tmpBuf[512];

	memset(tmpBuf, 0, 512);

	strcpy (resBuf, "    RESULT = SUCCESS\n\n");

	if(!strcasecmp(sfdb->active_sys_name, "ACTIVE"))
		strcat(tmpBuf, "      BOTH SYSTME ARE ACTIVE\n\n");
	else if(!strcasecmp(sfdb->active_sys_name, "STANDBY"))
		sprintf(tmpBuf, "      BOTH SYSTME ARE STANDBY\n\n");
	else
		sprintf(tmpBuf, "      %s IS ACTIVE\n\n", sfdb->active_sys_name);

	strcat(resBuf, tmpBuf);

	fimd_txMMLResult (rxIxpcMsg, resBuf, 0, 0, 0, 0, 1);

	return 1;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int fimd_mmc_audit_alm (IxpcQMsgType *rxIxpcMsg)
{
	int		i;

	for (i=0; i<SYSCONF_MAX_ASSO_SYS_NUM; i++) {
		if (!strcasecmp(sfdb->sys[i].commInfo.name, ""))
			continue;
		fimd_updateSysAlmInfo(i);
		fimd_broadcastAlmEvent2Client();
	}

	fimd_txMMLResult (rxIxpcMsg, "    RESULT = SUCCESS\n\n", 0, 0, 0, 0, 1);

	return 1;
	
} //----- End of fimd_mmc_audit_alm -----//

// -------------------------------------------------------------
int fimd_mmc_stop_aud_alm (IxpcQMsgType *rxIxpcMsg)
{
	//fprintf(stderr,"fimd_mmc_stop_aud_alm....\n");
	sprintf(trcBuf,"[fimd_mmc_stop_aud_alm]\n");
	trclib_writeLogErr (FL,trcBuf);

	fimd_AudioAlmEvent2Client();

	fimd_txMMLResult (rxIxpcMsg, "    RESULT = SUCCESS\n\n", 0, 0, 0, 0, 1);

	return 1;
	
} //----- End of fimd_mmc_audit_alm -----//



//------------------------------------------------------------------------------
// 해당 component unit의 alarm level을 표시하는 level field를 SFM_ALM_MASKED로
//	기록하여 더이상 상태관리 및 장애 처리를 하지 않도록 하는 명령을 처리한다.
// - mask로 표시된 component unit에 대해서는 samd로부터 통보되는 상태정보를 sfdb에
//	더이상 상태를 기록하지 않는다.
// - samd로부터 상태정보를 수신했을때 해당 component unit의 level field가 SFM_ALM_MASKED
//	이면, sfdb에 기록하지 않음으로써 관리대상에서 삭제되는 방식이다.
// - 나중에 다시 unmask 할때, level field에는 default(SFM_ALM_NORMAL) 값을 setting하는데,
//	장애를 감지하는 조건이 status, prevStatus의 상태 변경에 의한 것이므로 문제 되지 않는다.
//------------------------------------------------------------------------------
int fimd_mmc_mask_alm (IxpcQMsgType *rxIxpcMsg)
{
	int	    i, sysIndex, unitIndex, devIndex, almFlag=0;
	char	resCode=0;
	char	tmpBuf[256], argSys[32], argType[32], argUnit[32]; 
	MMLReqMsgType	*rxMmlReqMsg;
	char	query[1024], devName[6];
	//sjjeon
	SFM_SCEDev *pSceDev = NULL;
	SFM_L2SW   *pL2Info = NULL;
	/* hjjung */
	LEG_SESS_NUM_INFO    *pLegInfo = NULL;
	TPS_INFO     *pCallInfo = NULL;

	rxMmlReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	// get input parameters
	//
	strcpy (argSys,  rxMmlReqMsg->head.para[0].paraVal);
	strcpy (argType, rxMmlReqMsg->head.para[1].paraVal);
	if (strcasecmp (rxMmlReqMsg->head.para[2].paraVal, ""))
		strcpy (argUnit, rxMmlReqMsg->head.para[2].paraVal);
	else
		strcpy (argUnit, "");
/*
   // PDSN은 사용하지 않음.. sjjeon
	if (strcasecmp (rxMmlReqMsg->head.para[3].paraVal, ""))
		strcpy (argPDSNIP, rxMmlReqMsg->head.para[3].paraVal);
	else
		strcpy (argPDSNIP, "");	
*/

	// 결과 메시지에 대문자로 출력하기 위해 대문자로 바꾼다.
	//
	for (i=0; i<strlen(argSys); i++)  argSys[i]  = toupper(argSys[i]);
	for (i=0; i<strlen(argType); i++) argType[i] = toupper(argType[i]);
	for (i=0; i<strlen(argUnit); i++) argUnit[i] = toupper(argUnit[i]);

	for (sysIndex=0; sysIndex<SYSCONF_MAX_ASSO_SYS_NUM; sysIndex++){
		if(!strcasecmp(argSys, sfdb->sys[sysIndex].commInfo.name))
			break;
	}

	if(!strcasecmp(argSys, "TAPA")){
		devIndex = 0;
		almFlag=1;
		strcpy(devName, "TAPA");	
	}
	else if(!strcasecmp(argSys, "TAPB")){
		devIndex = 1;
		almFlag=1;
		strcpy(devName, "TAPB");
	}
	// by sjjeon
	else if(!strcasecmp(argSys, "SCEA")){
		devIndex = 0;
		almFlag=2;
		sprintf(devName, "SCEA");	
		pSceDev = (SFM_SCEDev*)&g_pstSCEInfo->SCEDev[devIndex];
	}
	else if(!strcasecmp(argSys, "SCEB")){
		devIndex = 1;
		almFlag=2;
		sprintf(devName, "SCEB");	
		pSceDev = (SFM_SCEDev*)&g_pstSCEInfo->SCEDev[devIndex];
	}
	else if(!strcasecmp(argSys, "L2SWA")){
		devIndex = 0;
		almFlag=3;
		sprintf(devName, "L2SWA");	
		pL2Info = (SFM_L2SW*)&g_pstL2Dev->l2Info[devIndex];
	}
	else if(!strcasecmp(argSys, "L2SWB")){
		devIndex = 1;
		almFlag=3;
		sprintf(devName, "L2SWB");	
		pL2Info = (SFM_L2SW*)&g_pstL2Dev->l2Info[devIndex];
	}
	/* hjjung */
	else if(!strcasecmp(argSys, "SCMA")){
		devIndex = 0;
		almFlag=3;
		sprintf(devName, "SCMA");	
		pLegInfo = (LEG_SESS_NUM_INFO*)&g_pstCALLInfo->legInfo[devIndex];
		pCallInfo = (TPS_INFO*)&g_pstCALLInfo->tpsInfo;
	}
	else if(!strcasecmp(argSys, "SCMB")){
		devIndex = 1;
		almFlag=3;
		sprintf(devName, "SCMB");	
		pLegInfo = (LEG_SESS_NUM_INFO*)&g_pstCALLInfo->legInfo[devIndex];
		pCallInfo = (TPS_INFO*)&g_pstCALLInfo->tpsInfo;
	}
	else if(sysIndex >= SYSCONF_MAX_ASSO_SYS_NUM){
		sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN SYS NAME");
		resCode = -1;
		fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
		return -1;
	}

	// type이 아래의 것이 아니면 unit이 들어와야 한다.(unit이 없는 경우)
	if (!strcasecmp(argType, "CPU") || !strcasecmp(argType, "MEM") || !strcasecmp(argType, "QUE") || 
			// TAP 의 CPU/MEM 상태 감시 기능 배제 2009/10/14
			//!strcasecmp(argType, "DUP_HB") || !strcasecmp(argType, "TAP-CPU") || !strcasecmp(argType, "TAP-MEM") ||
			!strcasecmp(argType, "DUP_HB") ||
			!strcasecmp(argType, "SCE-DISK") || !strcasecmp(argType,"SCE-USER") || !strcasecmp(argType,"SCE-FAN") || !strcasecmp(argType,"SCE-PWR") ||
			!strcasecmp(argType, "SCE-TEMP") || !strcasecmp(argType,"SCE-STAT") ||
// hjjung test			!strcasecmp(argType, "RLEG-SESS") ||
			!strcasecmp(argType, "SESSION") ||
// added by uamyd 20110209, LOGON 성공율 감시
			!strcasecmp(argType, "LOGON-SUCC") || !strcasecmp(argType, "LOGOUT-SUCC") ||
			!strcasecmp(argType, "L2-CPU") || !strcasecmp(argType,"L2-MEM") || !strcasecmp(argType, "CPS_LOAD")) 
	{
	/* unit 이 들어오지 않아야 하는 경우인데 들어온 경우. */
		if(strcasecmp(argUnit,"")) {
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNIT PARA NOT REQUIRED\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}

	} else {
		/* unit이 들어와야 하는데 안들어온 경우.*/
		if(!strcasecmp(argUnit,"")) {
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNIT PARA REQUIRED\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}
	}
	
	if (!strcasecmp(argType, "CPU")) {
		for(unitIndex=0; unitIndex<sfdb->sys[sysIndex].commInfo.cpuCnt; unitIndex++ ){
			if (sfdb->sys[sysIndex].commInfo.cpuInfo.mask[unitIndex] == SFM_ALM_MASKED) {
				sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY MASKED\n");
				resCode = -1;
			} 
		}
		if(resCode == 0){
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
					"system_group='%s' AND system_name='%s' AND alarm_type=%d )",
					SFM_CURR_ALM_DB_TABLE_NAME,
					1, 
					commlib_printDateTime(time(NULL)),
					sfdb->sys[sysIndex].commInfo.type,
					sfdb->sys[sysIndex].commInfo.group,
					sfdb->sys[sysIndex].commInfo.name,
					SFM_ALM_TYPE_CPU_USAGE
				   );

			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_mask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				for(unitIndex=0; unitIndex<sfdb->sys[sysIndex].commInfo.cpuCnt; unitIndex++ ){
					sfdb->sys[sysIndex].commInfo.cpuInfo.mask[unitIndex] = SFM_ALM_MASKED;
				}
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}

	else if (!strcasecmp(argType, "DISK")) {
		if ((unitIndex = fimd_getDiskIndexByName(sysIndex, argUnit)) < 0) {
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN DISK PARTITION\n");
			resCode = -1;
		} else if (sfdb->sys[sysIndex].commInfo.diskInfo[unitIndex].mask == SFM_ALM_MASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY MASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
					"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s')",
					SFM_CURR_ALM_DB_TABLE_NAME,
					1, 
					commlib_printDateTime(time(NULL)),
					sfdb->sys[sysIndex].commInfo.type,
					sfdb->sys[sysIndex].commInfo.group,
					sfdb->sys[sysIndex].commInfo.name,
					SFM_ALM_TYPE_DISK_USAGE,
					sfdb->sys[sysIndex].commInfo.diskInfo[unitIndex].name);

			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_mask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				sfdb->sys[sysIndex].commInfo.diskInfo[unitIndex].mask = SFM_ALM_MASKED;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}

	else if (!strcasecmp(argType, "LAN")) 
	{
		if ((unitIndex = fimd_getLanIndexByName(sysIndex, argUnit)) < 0) {
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN LAN NAME\n");
			resCode = -1;
		} else if (sfdb->sys[sysIndex].commInfo.lanInfo[unitIndex].mask == SFM_ALM_MASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY MASKE\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
					"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s(%s)')",
					SFM_CURR_ALM_DB_TABLE_NAME,
					1, 
					commlib_printDateTime(time(NULL)),
					sfdb->sys[sysIndex].commInfo.type,
					sfdb->sys[sysIndex].commInfo.group,
					sfdb->sys[sysIndex].commInfo.name,
					SFM_ALM_TYPE_LAN,
					sfdb->sys[sysIndex].commInfo.lanInfo[unitIndex].name,
					sfdb->sys[sysIndex].commInfo.lanInfo[unitIndex].targetIp);
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_mask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				sfdb->sys[sysIndex].commInfo.lanInfo[unitIndex].mask = SFM_ALM_MASKED;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}

	/* by helca */
	else if (!strcasecmp(argType, "RMTLAN")) 
	{
		if ((unitIndex = fimd_getRmtLanIndexByName(sysIndex, argUnit)) < 0) {
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN LAN NAME\n");
			resCode = -1;
		} else if (sfdb->sys[sysIndex].commInfo.rmtLanInfo[unitIndex].mask == SFM_ALM_MASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY MASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
					"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s(%s)')",
					SFM_CURR_ALM_DB_TABLE_NAME,
					1, 
					commlib_printDateTime(time(NULL)),
					sfdb->sys[sysIndex].commInfo.type,
					sfdb->sys[sysIndex].commInfo.group,
					sfdb->sys[sysIndex].commInfo.name,
					SFM_ALM_TYPE_RMT_LAN,
					&sfdb->sys[sysIndex].commInfo.rmtLanInfo[unitIndex].name[1],
					sfdb->sys[sysIndex].commInfo.rmtLanInfo[unitIndex].targetIp);
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_mask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				sfdb->sys[sysIndex].commInfo.rmtLanInfo[unitIndex].mask = SFM_ALM_MASKED;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}

	/* by helca */
	else if (!strcasecmp(argType, "DUP_HB")) 
	{
		if (sfdb->sys[sysIndex].commInfo.systemDup.mask == SFM_ALM_MASKED) {
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = ALREADY MASKED\n");
			resCode = -1;
		} else {

			int almType[4]={SFM_ALM_TYPE_DUP_HEARTBEAT,SFM_ALM_TYPE_DUAL_ACT,SFM_ALM_TYPE_DUAL_STD,SFM_ALM_TYPE_DUAL_STS_QRY_TIME_OUT};
			for(i=0; i<4; i++){
				sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
						"system_group='%s' AND system_name='%s' AND alarm_type=%d)",
						SFM_CURR_ALM_DB_TABLE_NAME,
						1,
						commlib_printDateTime(time(NULL)),
						sfdb->sys[sysIndex].commInfo.type,
						sfdb->sys[sysIndex].commInfo.group,
						sfdb->sys[sysIndex].commInfo.name,
						almType[i]
					   );

				if (fimd_mysql_query (query) < 0) {
					sprintf(trcBuf,"[fimd_mmc_unmask_alm] fimd_mysql_query fail\n");
					trclib_writeLogErr (FL,trcBuf);
					sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
					resCode = -1;
				} else {
					sfdb->sys[sysIndex].commInfo.systemDup.mask = SFM_ALM_MASKED;
					sprintf(resBuf,"    RESULT = SUCCESS\n");
					resCode = 0;
				}
			}

		}
	}

	/* by helca */
	else if (!strcasecmp(argType, "NTP")) 
	{
		if ((unitIndex = fimd_gethwNtpIndexByName(sysIndex, argUnit)) < 0) {
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN LAN NAME\n");
			resCode = -1;
		}else if (sfdb->sys[sysIndex].commInfo.ntpSts[unitIndex].mask == SFM_ALM_MASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY MASKED\n");
			resCode = -1;
		} else {
			// added by dcham 20110523, where조건절이 틀려서 마스크 처리 안되었음. "NTP-%s: => NTP %s"
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
					"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information like 'NTP %s%%')",
					SFM_CURR_ALM_DB_TABLE_NAME,
					1, 
					commlib_printDateTime(time(NULL)),
					sfdb->sys[sysIndex].commInfo.type,
					sfdb->sys[sysIndex].commInfo.group,
					sfdb->sys[sysIndex].commInfo.name,
					SFM_ALM_TYPE_HWNTP,
					argUnit
				   );
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_unmask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				sfdb->sys[sysIndex].commInfo.ntpSts[unitIndex].mask = SFM_ALM_MASKED;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}

	/* by helca */
	else if (!strcasecmp(argType, "TAP-CPU")) 
	{
		if(!strncasecmp(argSys, "DSC", 3) || !strncasecmp(argSys, "SCM", 3)){
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN TYPE NAME\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}else if (l3pd->l3ProbeDev[devIndex].cpuInfo.mask == SFM_ALM_MASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY MASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
					"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s')",
					SFM_CURR_ALM_DB_TABLE_NAME,
					1, 
					commlib_printDateTime(time(NULL)),
					"TAP",
					"TAP",
					devName,
					//SFM_ALM_TYPE_PD_CPU_USAGE,
					SFM_ALM_TYPE_TAP_CPU_USAGE,
					SQL_PD_CPU_ALM_INFO);

			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_mask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				l3pd->l3ProbeDev[devIndex].cpuInfo.mask = SFM_ALM_MASKED;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}

	/* by helca */
	else if (!strcasecmp(argType, "TAP-MEM")) 
	{
		if(!strncasecmp(argSys, "DSC", 3) || !strncasecmp(argSys, "SCM", 3) ||
			!strncasecmp(argSys, "SCE", 3) || !strncasecmp(argSys, "L2SW", 4)){
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN TYPE NAME\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}else if (l3pd->l3ProbeDev[devIndex].memInfo.mask == SFM_ALM_MASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY MASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
					"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information = 'Memory')",
					SFM_CURR_ALM_DB_TABLE_NAME,
					1, 
					commlib_printDateTime(time(NULL)),
					"TAP",
					"TAP",
					devName,
					//SFM_ALM_TYPE_PD_MEMORY_USAGE
					SFM_ALM_TYPE_TAP_MEMORY_USAGE
				   );

			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_mask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				l3pd->l3ProbeDev[devIndex].memInfo.mask = SFM_ALM_MASKED;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}
#if 0 /* by helca */
	else if (!strcasecmp(argType, "PD-FAN")) {
		unitIndex = atoi(argUnit);
		if(!strncasecmp(argSys, "DSC", 3)){
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN TYPE NAME\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}else if (l3pd->l3ProbeDev[devIndex].fanInfo.mask[unitIndex] == SFM_ALM_MASKED) {
			//sprintf(resBuf,"    RESULT = FAIL\n    REASON = ALREADY MASKED\n");
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY MASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
					"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='PD_FAN %d')",
					SFM_CURR_ALM_DB_TABLE_NAME,
					1, 
					commlib_printDateTime(time(NULL)),
					"PD",
					"PD",
					devName,
					//SFM_ALM_TYPE_PD_FAN_STS,
					SFM_ALM_TYPE_TAP_FAN_STS,
					unitIndex);
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_mask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				l3pd->l3ProbeDev[devIndex].fanInfo.mask[unitIndex] = SFM_ALM_MASKED;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}
#endif

	/* by helca */
	else if (!strcasecmp(argType, "TAP-PORT")) 
	{
		unitIndex = atoi(argUnit)-1;
		if(!strncasecmp(argSys, "DSC", 3) || !strncasecmp(argSys, "SCM", 3) ||
			!strncasecmp(argSys, "SCE", 3) || !strncasecmp(argSys, "L2SW", 4)){
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN TYPE NAME\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}else if (l3pd->l3ProbeDev[devIndex].gigaLanInfo[unitIndex].mask == SFM_ALM_MASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY MASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
					"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s(%d)')",
					SFM_CURR_ALM_DB_TABLE_NAME,
					1, 
					commlib_printDateTime(time(NULL)),
					"TAP",
					"TAP",
					devName,
					//SFM_ALM_TYPE_PD_GIGA_LAN,
					SFM_ALM_TYPE_TAP_PORT_STS,
					SQL_TAP_PORT_ALM_INFO,
					unitIndex+1
				   );
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_mask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				l3pd->l3ProbeDev[devIndex].gigaLanInfo[unitIndex].mask = SFM_ALM_MASKED;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}

		}

	}
	else if (!strcasecmp(argType, "TAP-POWER"))  // 20110424 by dcham
	{
		unitIndex = atoi(argUnit)-1;
		if(!strncasecmp(argSys, "DSC", 3) || !strncasecmp(argSys, "SCM", 3) ||
				!strncasecmp(argSys, "SCE", 3) || !strncasecmp(argSys, "L2SW", 4)){
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN TYPE NAME\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}else if (l3pd->l3ProbeDev[devIndex].powerInfo[unitIndex].mask == SFM_ALM_MASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY MASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
					"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s(%d)')",
					SFM_CURR_ALM_DB_TABLE_NAME,
					1,
					commlib_printDateTime(time(NULL)),
					"TAP",
					"TAP",
					devName,
					SFM_ALM_TYPE_TAP_POWER_STS,
					SQL_TAP_POWER_ALM_INFO,
					unitIndex+1
				   );
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_mask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				l3pd->l3ProbeDev[devIndex].powerInfo[unitIndex].mask = SFM_ALM_MASKED;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}
	else if (!strcasecmp(argType, "PROC")) 
	{

		if ((unitIndex = fimd_getProcIndexByName(sysIndex, argUnit)) < 0) {
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN PROCESS\n");
			resCode = -1;
		} else if (sfdb->sys[sysIndex].commInfo.procInfo[unitIndex].mask == SFM_ALM_MASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY MASKED\n");
			resCode = -1;
		} else {
			int almType;

#ifndef _NOT_USED_PROCESS_ALARM_TYPE_
			/* unitIndex로 alarm type 획득 : sjjeon */
			almType = getAlarmTypeFromProcName(sysIndex, unitIndex);
#else	
			almType = SFM_ALM_TYPE_PROC;
#endif

			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
					"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s')",
					SFM_CURR_ALM_DB_TABLE_NAME,
					1, 
					commlib_printDateTime(time(NULL)),
					sfdb->sys[sysIndex].commInfo.type,
					sfdb->sys[sysIndex].commInfo.group,
					sfdb->sys[sysIndex].commInfo.name,
					almType,
					sfdb->sys[sysIndex].commInfo.procInfo[unitIndex].name);
			//fprintf(stdout,"[Mask PROC] %s\n", query);	

			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_mask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				sfdb->sys[sysIndex].commInfo.procInfo[unitIndex].mask = SFM_ALM_MASKED;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}

	else if (!strcasecmp(argType, "MEM")) 
	{
		if (sfdb->sys[sysIndex].commInfo.memInfo.mask == SFM_ALM_MASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY MASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
					"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='Memory')",
					SFM_CURR_ALM_DB_TABLE_NAME,
					1, 
					commlib_printDateTime(time(NULL)),
					sfdb->sys[sysIndex].commInfo.type,
					sfdb->sys[sysIndex].commInfo.group,
					sfdb->sys[sysIndex].commInfo.name,
					SFM_ALM_TYPE_MEMORY_USAGE);

			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_mask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				sfdb->sys[sysIndex].commInfo.memInfo.mask = SFM_ALM_MASKED;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}

	else if (!strcasecmp(argType, "HW") || !strcasecmp(argType, "MIRR_PORT")) 
	{
		int almType;
		if(!strcasecmp(argType, "MIRR_PORT"))
			almType = SFM_ALM_TYPE_HW_MIRROR;		// mp h/w mirror port
		else
			almType = SFM_ALM_TYPE_MP_HW;


		if ((unitIndex = fimd_getHwIndexByName(sysIndex, argUnit)) < 0) {
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN HW NAME\n");
			resCode = -1;
		} else if (sfdb->sys[sysIndex].specInfo.u.sms.hpuxHWInfo.hwcom[unitIndex].mask == SFM_ALM_MASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY MASKED\n");
			resCode = -1;
		} else {
			char hwName[16]; bzero(hwName,sizeof(hwName));
			hw_name_mapping((char*)argUnit,strlen(argUnit),(char*)hwName);
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
					"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s')",
					SFM_CURR_ALM_DB_TABLE_NAME,
					1, 
					commlib_printDateTime(time(NULL)),
					sfdb->sys[sysIndex].commInfo.type,
					sfdb->sys[sysIndex].commInfo.group,
					sfdb->sys[sysIndex].commInfo.name,
					almType,
					hwName); 
				
			fprintf(stdout,"[Mask HW or MIRR_PORT] %s\n",query);			

			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_mask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				sfdb->sys[sysIndex].specInfo.u.sms.hpuxHWInfo.hwcom[unitIndex].mask = SFM_ALM_MASKED;
				sprintf(resBuf, "    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}

	/* by helca */
	else if (!strcasecmp(argType, "QUE")) 
	{
		for(i=0; i<SFM_MAX_QUE_CNT; i++ ){
			if (sfdb->sys[sysIndex].commInfo.queInfo[i].mask == SFM_ALM_MASKED) {
				sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY MASKED\n");
				resCode = -1;
			} 
		}	
		if(resCode == 0){
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
					"system_group='%s' AND system_name='%s' AND alarm_type=%d)",
					SFM_CURR_ALM_DB_TABLE_NAME,
					1, 
					commlib_printDateTime(time(NULL)),
					sfdb->sys[sysIndex].commInfo.type,
					sfdb->sys[sysIndex].commInfo.group,
					sfdb->sys[sysIndex].commInfo.name,
					SFM_ALM_TYPE_QUEUE_LOAD);

			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_mask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				for(i=0; i<SFM_MAX_QUE_CNT; i++ ){
					sfdb->sys[sysIndex].commInfo.queInfo[i].mask = SFM_ALM_MASKED;
					resCode = 0;
				}
				sprintf(resBuf,"    RESULT = SUCCESS\n");
			}
		}	
	}

	// by helca 10.30
	else if (!strcasecmp(argType, "NMS")) 
	{

		if(strncasecmp(argSys, "DSCM", 4)){
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN SYS NAME\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}

		if ((unitIndex = fimd_getNmsifIndexByName(sysIndex, argUnit)) < 0) {
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN NMSIF NAME\n");
			resCode = -1;
		} else if (sfdb->nmsInfo.mask[unitIndex] == SFM_ALM_MASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY MASKED\n");
			resCode = -1;
		}
		else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
					"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='NMSIF ALARM %s')",
					SFM_CURR_ALM_DB_TABLE_NAME,
					1, 
					commlib_printDateTime(time(NULL)),
					sfdb->sys[sysIndex].commInfo.type,
					sfdb->sys[sysIndex].commInfo.group,
					sfdb->sys[sysIndex].commInfo.name,
					SFM_ALM_TYPE_NMSIF_CONNECT,
					nmsifName[unitIndex]
				   );
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_mask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				sfdb->nmsInfo.mask[unitIndex] = SFM_ALM_MASKED;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}

	// by sjjeon  (SCE Alarm)
	else if (!strcasecmp(argType, "SCE-STAT"))
	{
		//unitIndex = atoi(argUnit)-1;
		if(!strncasecmp(argSys, "DSC", 3) || !strncasecmp(argSys, "SCM", 3) || 
				!strncasecmp(argSys, "TAP", 3) || !strncasecmp(argSys, "L2SW", 4)){
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN TYPE NAME\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}else if (pSceDev->sysStatus.mask == SFM_ALM_MASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY MASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
				"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s')",
				SFM_CURR_ALM_DB_TABLE_NAME,
				1,
				commlib_printDateTime(time(NULL)),
				"SCE",
				"SCE",
				devName,
				SFM_ALM_TYPE_SCE_STATUS,
				SQL_SCE_STATUS_ALM_INFO
			   );
			//printf("SCE-STAT : query [%s]", query);
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_mask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				pSceDev->sysStatus.mask = SFM_ALM_MASKED;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}

	else if (!strcasecmp(argType, "SCE-CPU"))
	{
		//printf("type : %s, argUnit : %s\n", argType, argUnit);
		if(!strcasecmp(argUnit,"CPU-1")) unitIndex=0;
		else if(!strcasecmp(argUnit,"CPU-2")) unitIndex=1;
		else if(!strcasecmp(argUnit,"CPU-3")) unitIndex=2;
		else {
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN UNIT NAME\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}

		if(!strncasecmp(argSys, "DSC", 3) || !strncasecmp(argSys, "SCM", 3) || 
				!strncasecmp(argSys, "TAP", 3) || !strncasecmp(argSys, "L2SW",4))
		{
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN TYPE NAME\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}else if (pSceDev->cpuInfo[unitIndex].mask == SFM_ALM_MASKED) {
			//	}else if (g_pstSCEInfo->SCEDev[devIndex].cpuInfo[unitIndex].mask == SFM_ALM_MASKED) {
			//printf("1. mask index : %d, value : %d, address : %p\n",unitIndex, pSceDev->cpuInfo[unitIndex].mask, &pSceDev->cpuInfo[unitIndex].mask);
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY MASKED\n");
		resCode = -1;

	} else {
		//printf("2. mask index : %d, value : %d, address : %p\n",unitIndex, pSceDev->cpuInfo[unitIndex].mask, &pSceDev->cpuInfo[unitIndex].mask);
		sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
				"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s(%d)')",
				SFM_CURR_ALM_DB_TABLE_NAME,
				1,
				commlib_printDateTime(time(NULL)),
				"SCE",
				"SCE",
				devName,
				SFM_ALM_TYPE_SCE_CPU,
				SQL_SCE_CPU_ALM_INFO,
				unitIndex+1
			   );
		if (fimd_mysql_query (query) < 0) {
			sprintf(trcBuf,"[fimd_mmc_mask_alm] fimd_mysql_query fail\n");
			trclib_writeLogErr (FL,trcBuf);
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
			resCode = -1;
		} else {
			//				pSceDev->cpuInfo[unitIndex].mask = SFM_ALM_MASKED;
			g_pstSCEInfo->SCEDev[devIndex].cpuInfo[unitIndex].mask = SFM_ALM_MASKED;
			//g_pstSCEInfo->SCEDev[devIndex].cpuInfo[unitIndex].mask = 99;
			sprintf(resBuf,"    RESULT = SUCCESS\n");
			resCode = 0;
			}
		}
	}

	else if (!strcasecmp(argType, "SCE-DISK"))
	{

		if(!strncasecmp(argSys, "DSC", 3) || !strncasecmp(argSys, "SCM", 3) || 
				!strncasecmp(argSys, "TAP", 3) || !strncasecmp(argSys, "L2SW", 4)){
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN TYPE NAME\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}else if (pSceDev->diskInfo.mask == SFM_ALM_MASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY MASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
					"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s')",
					SFM_CURR_ALM_DB_TABLE_NAME,
					1,
					commlib_printDateTime(time(NULL)),
					"SCE",
					"SCE",
					devName,
					SFM_ALM_TYPE_SCE_DISK,
					SQL_SCE_DISK_ALM_INFO
				   );
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_mask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				pSceDev->diskInfo.mask = SFM_ALM_MASKED;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}

	/* hjjung */
	else if (!strcasecmp(argType, "SCE-USER"))
	{

		if(!strncasecmp(argSys, "DSC", 3) || !strncasecmp(argSys, "SCM", 3) || 
				!strncasecmp(argSys, "TAP", 3) || !strncasecmp(argSys, "L2SW", 4)){
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN TYPE NAME\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}else if (pSceDev->userInfo.mask == SFM_ALM_MASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY MASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
					"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s')",
					SFM_CURR_ALM_DB_TABLE_NAME,
					1,
					commlib_printDateTime(time(NULL)),
					"SCE",
					"SCE",
					devName,
					SFM_ALM_TYPE_SCE_USER,
					SQL_SCE_USER_ALM_INFO
				   );
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_mask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				pSceDev->userInfo.mask = SFM_ALM_MASKED;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}

	else if (!strcasecmp(argType, "SCE-MEM"))
	{
		if(!strcasecmp(argUnit,"MEM-1")) unitIndex=0;
		else if(!strcasecmp(argUnit,"MEM-2")) unitIndex=1;
		else if(!strcasecmp(argUnit,"MEM-3")) unitIndex=2;
		else {
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN UNIT NAME\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}

		if(!strncasecmp(argSys, "DSC", 3) || !strncasecmp(argSys, "SCM", 3) || 
				!strncasecmp(argSys, "TAP", 3) || !strncasecmp(argSys, "L2SW", 4)){
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN TYPE NAME\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}else if (pSceDev->memInfo[unitIndex].mask == SFM_ALM_MASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY MASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
					"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s(%d)')",
					SFM_CURR_ALM_DB_TABLE_NAME,
					1,
					commlib_printDateTime(time(NULL)),
					"SCE",
					"SCE",
					devName,
					SFM_ALM_TYPE_SCE_MEM,
					SQL_SCE_MEM_ALM_INFO,	
					unitIndex+1
				   );
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_mask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				pSceDev->memInfo[unitIndex].mask = SFM_ALM_MASKED;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}


	else if (!strcasecmp(argType, "SCE-PORT"))
	{
		int almtype;

		if(!strcasecmp(argUnit,"MNG-1")) { unitIndex=0; almtype = SFM_ALM_TYPE_SCE_PORT_MGMT;}
		else if(!strcasecmp(argUnit,"MNG-2")) {unitIndex=1; almtype = SFM_ALM_TYPE_SCE_PORT_MGMT;}
		else if(!strcasecmp(argUnit,"PORT-1")) { unitIndex=2; almtype = SFM_ALM_TYPE_SCE_PORT_LINK;}
		else if(!strcasecmp(argUnit,"PORT-2")) { unitIndex=3; almtype = SFM_ALM_TYPE_SCE_PORT_LINK;}
		else if(!strcasecmp(argUnit,"PORT-3")) { unitIndex=4; almtype = SFM_ALM_TYPE_SCE_PORT_LINK;}
		else if(!strcasecmp(argUnit,"PORT-4")) { unitIndex=5; almtype = SFM_ALM_TYPE_SCE_PORT_LINK;}
		else {
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN UNIT NAME\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}

		if(!strncasecmp(argSys, "DSC", 3) || !strncasecmp(argSys, "SCM", 3) || 
				!strncasecmp(argSys, "TAP", 3) || !strncasecmp(argSys, "L2SW", 4)){
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN TYPE NAME\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}else if (pSceDev->portStatus[unitIndex].mask == SFM_ALM_MASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY MASKED\n");
			resCode = -1;
		} else {

			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
					"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s')",
					SFM_CURR_ALM_DB_TABLE_NAME,
					1,
					commlib_printDateTime(time(NULL)),
					"SCE",
					"SCE",
					devName,
					almtype,
					fimd_getScePortName(unitIndex)
				   );
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_mask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				pSceDev->portStatus[unitIndex].mask = SFM_ALM_MASKED;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}


	else if (!strcasecmp(argType, "SCE-FAN"))
	{

		if(!strncasecmp(argSys, "DSC", 3) || !strncasecmp(argSys, "SCM", 3) || 
				!strncasecmp(argSys, "TAP", 3) || !strncasecmp(argSys, "L2SW", 4)){
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN TYPE NAME\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}else if (pSceDev->fanStatus.mask == SFM_ALM_MASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY MASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
					"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s')",
					SFM_CURR_ALM_DB_TABLE_NAME,
					1,
					commlib_printDateTime(time(NULL)),
					"SCE",
					"SCE",
					devName,
					SFM_ALM_TYPE_SCE_FAN,
					SQL_SCE_FAN_ALM_INFO
				   );
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_mask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				pSceDev->fanStatus.mask = SFM_ALM_MASKED;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}

	else if (!strcasecmp(argType, "SCE-PWR"))
	{

		if(!strncasecmp(argSys, "DSC", 3) || !strncasecmp(argSys, "SCM", 3) || 
				!strncasecmp(argSys, "TAP", 3) || !strncasecmp(argSys, "L2SW", 4)){
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN TYPE NAME\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}else if (pSceDev->pwrStatus.mask == SFM_ALM_MASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY MASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
					"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s')",

					SFM_CURR_ALM_DB_TABLE_NAME,
					1,
					commlib_printDateTime(time(NULL)),
					"SCE",
					"SCE",
					devName,
					SFM_ALM_TYPE_SCE_PWR,
					SQL_SCE_PWR_ALM_INFO
				   );
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_mask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				pSceDev->pwrStatus.mask = SFM_ALM_MASKED;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}

	else if (!strcasecmp(argType, "SCE-TEMP"))
	{

		if(!strncasecmp(argSys, "DSC", 3) || !strncasecmp(argSys, "SCM", 3) || !strncasecmp(argSys, "TAP", 3)){
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN TYPE NAME\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}else if (pSceDev->tempStatus.mask == SFM_ALM_MASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY MASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
					"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s')",

					SFM_CURR_ALM_DB_TABLE_NAME,
					1,
					commlib_printDateTime(time(NULL)),
					"SCE",
					"SCE",
					devName,
					SFM_ALM_TYPE_SCE_TEMP,
					SQL_SCE_TEMP_ALM_INFO
				   );
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_mask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				pSceDev->tempStatus.mask = SFM_ALM_MASKED;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}

	else if (!strcasecmp(argType, "SCE-RDR-CONN"))
	{
		if(!strcasecmp(argUnit,"SCMA")) unitIndex = 0;
		else if(!strcasecmp(argUnit,"SCMB")) unitIndex = 1;
		else unitIndex = -1;

		if(unitIndex < 0){	
			sprintf(resBuf,"[%s] INVALID UNIT NUMBER (%s)\n",__FUNCTION__, argUnit);
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}
	
		if(!strncasecmp(argSys, "DSC", 3) || !strncasecmp(argSys, "SCM", 3) || 
				!strncasecmp(argSys, "TAP", 3) || !strncasecmp(argSys, "L2SW",4)){
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = INVALID TYPE NAME\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}
		
		if (pSceDev->rdrConnStatus[unitIndex].mask == SFM_ALM_MASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY MASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
					"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s(%d)')",
					SFM_CURR_ALM_DB_TABLE_NAME,
					1,
					commlib_printDateTime(time(NULL)),
					"SCE",
					"SCE",
					devName,
					SFM_ALM_TYPE_SCE_RDR_CONN,
					SQL_SCE_RDR_CONN_ALM_INFO,
					unitIndex+1
				   );
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_mask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				//pSceDev->rdrConnStatus.mask = SFM_ALM_MASKED;
				pSceDev->rdrConnStatus[unitIndex].mask = SFM_ALM_MASKED;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}

	/* hjjung */
	else if (!strcasecmp(argType, "SESSION"))
	{

		// argSys 가 DSC, SCM, TAP, SCE인 경우 오류. only L2SW
		if(!strncasecmp(argSys, "DSC", 3) || !strncasecmp(argSys, "L2SW", 3) || 
				!strncasecmp(argSys, "TAP", 3) || !strncasecmp(argSys, "SCE", 3)){
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN TYPE NAME\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}else if (pLegInfo->mask == SFM_ALM_MASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY MASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
				"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s')",
				SFM_CURR_ALM_DB_TABLE_NAME,
				1,
				commlib_printDateTime(time(NULL)),
				"SCM",
				"SCM",
				devName,
				SFM_ALM_TYPE_LEG_SESSION,
				SQL_LEG_SESSION_ALM_INFO
			   );
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[%s] fimd_mysql_query fail\n", __FUNCTION__);
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				pLegInfo->mask = SFM_ALM_MASKED;
				//g_pstCALLInfo->legInfo[devIndex].mask = SFM_ALM_MASKED;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}
    // added by dcham 20110525 for TPS
	else if (!strcasecmp(argType, "TPS_LOAD"))
	{
		if(!strncasecmp(argSys, "DSC", 3) || !strncasecmp(argSys, "L2SW", 3) || 
				!strncasecmp(argSys, "TAP", 3) || !strncasecmp(argSys, "SCE", 3)){
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN TYPE NAME\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}else if (pCallInfo->mask == SFM_ALM_MASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY MASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
				"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s')",
				SFM_CURR_ALM_DB_TABLE_NAME,
				1,
				commlib_printDateTime(time(NULL)),
				"SCM",
				"SCM",
				devName,
				SFM_ALM_TYPE_TPS,
				SQL_TPS_ALM_INFO
			   );
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[%s] fimd_mysql_query fail\n", __FUNCTION__);
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				pCallInfo->mask = SFM_ALM_MASKED;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}

	/* L2 Switch CPU : sjjeon*/
	else if (!strcasecmp(argType, "L2-CPU"))
	{
		//printf("type : %s, argUnit : %s\n", argType, argUnit);

		// argSys 가 DSC, SCM, TAP, SCE인 경우 오류. only L2SW
		if(!strncasecmp(argSys, "DSC", 3) || !strncasecmp(argSys, "SCM", 3) || 
				!strncasecmp(argSys, "TAP", 3) || !strncasecmp(argSys, "SCE", 3)){
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN TYPE NAME\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}else if (pL2Info->cpuInfo.mask == SFM_ALM_MASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY MASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
				"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s')",
				SFM_CURR_ALM_DB_TABLE_NAME,
				1,
				commlib_printDateTime(time(NULL)),
				"L2SW",
				"L2SW",
				devName,
				SFM_ALM_TYPE_L2_CPU,
				SQL_L2SW_CPU_ALM_INFO
			   );
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[%s] fimd_mysql_query fail\n", __FUNCTION__);
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				g_pstL2Dev->l2Info[devIndex].cpuInfo.mask = SFM_ALM_MASKED;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}

	/* L2 Switch MEM : sjjeon*/
	else if (!strcasecmp(argType, "L2-MEM"))
	{
		//printf("type : %s, argUnit : %s\n", argType, argUnit);

		// argSys 가 DSC, SCM, TAP, SCE인 경우 오류. only L2SW
		if(!strncasecmp(argSys, "DSC", 3) || !strncasecmp(argSys, "SCM", 3) || 
				!strncasecmp(argSys, "TAP", 3) || !strncasecmp(argSys, "SCE", 3)){
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN TYPE NAME\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}else if (pL2Info->cpuInfo.mask == SFM_ALM_MASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY MASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
				"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s')",
				SFM_CURR_ALM_DB_TABLE_NAME,
				1,
				commlib_printDateTime(time(NULL)),
				"L2SW",
				"L2SW",
				devName,
				SFM_ALM_TYPE_L2_MEM,
				SQL_L2SW_MEM_ALM_INFO
			   );
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[%s] fimd_mysql_query fail\n", __FUNCTION__);
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				g_pstL2Dev->l2Info[devIndex].memInfo.mask = SFM_ALM_MASKED;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}

	else if (!strcasecmp(argType, "L2-PORT"))
	{
		int portNum;
		portNum = atoi(argUnit);
		// port (1~27)
		if(portNum==0 || portNum>MAX_L2_PORT_NUM)
		{
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN PORT NUM(%d)\n", portNum);
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}

		if(!strncasecmp(argSys, "DSC", 3) || !strncasecmp(argSys, "SCM", 3) || 
				!strncasecmp(argSys, "TAP", 3) || !strncasecmp(argSys, "SCE", 3)){
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN TYPE NAME\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		// mmc로 입력 받은 portNum은 array +1 이므로 ...
		}else if (pL2Info->portInfo[portNum-1].mask == SFM_ALM_MASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY MASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
					"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s(%d)')",
					SFM_CURR_ALM_DB_TABLE_NAME,
					1,
					commlib_printDateTime(time(NULL)),
					"L2SW",
					"L2SW",
					devName,
					SFM_ALM_TYPE_L2_LAN,
					SQL_L2SW_PORT_ALM_INFO,
					portNum
				   );
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_mask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				pL2Info->portInfo[portNum-1].mask = SFM_ALM_MASKED;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}

	// CPS LOAD Mask
	else if (!strcasecmp(argType, "CPS_LOAD"))
	{
		if (sfdb->sys[sysIndex].commInfo.cpsOverSts.mask == SFM_ALM_MASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY MASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
					"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s')",
					SFM_CURR_ALM_DB_TABLE_NAME,
					1,
					commlib_printDateTime(time(NULL)),
					sfdb->sys[sysIndex].commInfo.type,
					sfdb->sys[sysIndex].commInfo.group,
					sfdb->sys[sysIndex].commInfo.name,
					SFM_ALM_TYPE_CPS_OVER,
					"OvER_CPS"
				   );
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_mask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				sfdb->sys[sysIndex].commInfo.cpsOverSts.mask = SFM_ALM_MASKED;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	} /*End of CPS_LOAD*/


	// SM Connection Status added by uamyd 20110425
    else if (!strcasecmp(argType, "SMLINK"))
    {
#if 0
		// added by dcham 20110530 for SM connection(5=>1)
		int sm_ch_id = atoi(argUnit);

		sm_ch_id--; //Client 에서는 1,2,3,4,5 를 보내준다. 

		if( sm_ch_id < 0 || sm_ch_id >= SFM_MAX_SM_CH_CNT ){	
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN CHANNEL ID\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		} else {
#endif
			int sm_ch_id =0; // added by dcham 20110530 for SM connection(5=>1)
			if (sfdb->sys[sysIndex].commInfo.smChSts.each[sm_ch_id].mask == SFM_ALM_MASKED) {
				sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY MASKED\n");
				resCode = -1;
			} else {
				sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
						"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='SMLINK(%d)')",
						SFM_CURR_ALM_DB_TABLE_NAME,
						1,
						commlib_printDateTime(time(NULL)),
						sfdb->sys[sysIndex].commInfo.type,
						sfdb->sys[sysIndex].commInfo.group,
						sfdb->sys[sysIndex].commInfo.name,
						SFM_ALM_TYPE_SM_CONN_STS,
						sm_ch_id
					   );
				if (fimd_mysql_query (query) < 0) {
					sprintf(trcBuf,"[fimd_mmc_mask_alm] fimd_mysql_query fail\n");
					trclib_writeLogErr (FL,trcBuf);
					sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
					resCode = -1;
				} else {
					sfdb->sys[sysIndex].commInfo.smChSts.each[sm_ch_id].mask = SFM_ALM_MASKED;
					sprintf(resBuf,"    RESULT = SUCCESS\n");
					resCode = 0;
				}
			}
	//	} // added by dcham 20110530 for SM connection(5=>1)
    }

	/*
	* LOGON 성공율 감시 마스킹
    * added by uamyd 20110209
    */
	else if (!strcasecmp(argType, "LOGON-SUCC") || !strcasecmp(argType, "LOGOUT-SUCC")) {

		int log_mod, almType;
		if( !strcasecmp(argType, "LOGON-SUCC") ){
			log_mod = 0;
			almType = SFM_ALM_TYPE_LOGON_SUCCESS_RATE;
		} else {
			log_mod = 1;
			almType = SFM_ALM_TYPE_LOGOUT_SUCCESS_RATE;
		}
		if( !devIndex ){ g_pstLogonRate = &g_stLogonRate[log_mod][0]; }
		else           { g_pstLogonRate = &g_stLogonRate[log_mod][1]; }

		if( g_pstLogonRate->mask == SFM_ALM_MASKED ){
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY MASKED\n");
			resCode = -1;
        }

        if(resCode == 0){
            sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
                    "system_group='%s' AND system_name='%s' AND alarm_type=%d )",
                    SFM_CURR_ALM_DB_TABLE_NAME,
                    1,
                    commlib_printDateTime(time(NULL)),
					"MP", "DSC", ( !devIndex? "SCMA" :"SCMB" ),
                    almType
                   );

            if (fimd_mysql_query (query) < 0) {
                sprintf(trcBuf,"[fimd_mmc_mask_alm] fimd_mysql_query fail\n");
                trclib_writeLogErr (FL,trcBuf);
                sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
                resCode = -1;

            } else {
				g_pstLogonRate->mask = SFM_ALM_MASKED;
                sprintf(resBuf,"    RESULT = SUCCESS\n");
                resCode = 0;
            }
        }
    }

	
	// alarm mask 실패...
	else {
		sprintf(trcBuf,"[fimd_mmc_mask_alm] UNKNOWN TYPE (%s)\n", argType);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	if (resCode == 0) {
		if (strcasecmp (argUnit, ""))
			sprintf(tmpBuf,"\n    SYSTEM = %s\n    TYPE   = %s\n    UNIT   = %s\n\n", argSys, argType, argUnit);
		else
			sprintf(tmpBuf,"\n    SYSTEM = %s\n    TYPE   = %s\n\n", argSys, argType);
		strcat(resBuf, tmpBuf);
	}

	fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);

	/*  add by helca 2007-10-10 
		modify by sjjeon 2009-07-17 */
	/* TAP mask/umask-alm 수행시 sysIndex는 setting되지 않아 장애 발생 시킬수 있다. */

	if(almFlag == 1) 		/*PD Alram*/
		fimd_updatePDAlmInfo (devIndex);	
	else if(almFlag == 2) 	/*SCE Alarm*/
		fimd_updateSceAlmInfo (devIndex);	
	else if(almFlag == 3){ 	/*L2SW Alarm*/
		fimd_updateL2SWlmInfo(devIndex);	
		fimd_backupLogon2File(); /* LOGON ALARM */
		fimd_backupSMChSts2File(); /* SM Connection Status */
	}
	else  					/*system Alarm*/
		fimd_updateSysAlmInfo(sysIndex);

	/* shared memory backup */
	//바로 위에서 수행한다.
	//fimd_backupL3pd2File();
	//fimd_backupSCE2File();  //sjjeon
	//fimd_backupL2sw2File(); //sjjeon
	fimd_broadcastAlmEvent2Client();

	return 1;

} //----- End of fimd_mmc_mask_alm -----//


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int fimd_mmc_umask_alm (IxpcQMsgType *rxIxpcMsg)
{
	int	    i, sysIndex, unitIndex, devIndex,  len, almFlag=0;
	char	resCode=0;
	char	tmpBuf[256], argSys[32], argType[32], argUnit[32], argPDSNIP[32];
	MMLReqMsgType	*rxMmlReqMsg;
	char	query[1024], devName[6];
	//sjjeon
	SFM_SCEDev *pSceDev = NULL;
	SFM_L2SW   *pL2Info = NULL;
	/* hjjung */
	LEG_SESS_NUM_INFO    *pLegInfo = NULL;
	TPS_INFO     *pCallInfo = NULL;

	rxMmlReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	// get input parameters
	//
	strcpy (argSys,  rxMmlReqMsg->head.para[0].paraVal);
	strcpy (argType, rxMmlReqMsg->head.para[1].paraVal);
	if (strcasecmp (rxMmlReqMsg->head.para[2].paraVal, ""))
		strcpy (argUnit, rxMmlReqMsg->head.para[2].paraVal);
	else
		strcpy (argUnit, "");

	if (strcasecmp (rxMmlReqMsg->head.para[3].paraVal, ""))
                strcpy (argPDSNIP, rxMmlReqMsg->head.para[3].paraVal);
        else
                strcpy (argPDSNIP, "");

	// 결과 메시지에 대문자로 출력하기 위해 대문자로 바꾼다.
	//
	for (i=0; i<strlen(argSys); i++)  argSys[i]  = toupper(argSys[i]);
	for (i=0; i<strlen(argType); i++) argType[i] = toupper(argType[i]);
	for (i=0; i<strlen(argUnit); i++) argUnit[i] = toupper(argUnit[i]);

	for (sysIndex=0; sysIndex<SYSCONF_MAX_ASSO_SYS_NUM; sysIndex++){
		if(!strcasecmp(argSys, sfdb->sys[sysIndex].commInfo.name))
			break;
	}
	
	if(!strcasecmp(argSys, "TAPA")){
		devIndex = 0;
		almFlag = 1;
		sprintf(devName, "TAPA");
	}
	else if(!strcasecmp(argSys, "TAPB")){
		devIndex = 1;
		almFlag = 1;	
		sprintf(devName, "TAPB");
	}
	else if(!strcasecmp(argSys, "SCEA")){
		devIndex = 0;
		almFlag=2;
		sprintf(devName, "SCEA");	
		pSceDev = (SFM_SCEDev*)&g_pstSCEInfo->SCEDev[devIndex];
	}
	else if(!strcasecmp(argSys, "SCEB")){
		devIndex = 1;
		almFlag=2;
		sprintf(devName, "SCEB");	
		pSceDev = (SFM_SCEDev*)&g_pstSCEInfo->SCEDev[devIndex];
	}
	else if(!strcasecmp(argSys, "L2SWA")){
		devIndex = 0;
		almFlag=3;
		sprintf(devName, "L2SWA");	
		pL2Info = (SFM_L2SW*)&g_pstL2Dev->l2Info[devIndex];
	}
	else if(!strcasecmp(argSys, "L2SWB")){
		devIndex = 1;
		almFlag=3;
		sprintf(devName, "L2SWB");	
		pL2Info = (SFM_L2SW*)&g_pstL2Dev->l2Info[devIndex];
	}
	else if(sysIndex >= SYSCONF_MAX_ASSO_SYS_NUM){
		sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN SYS NAME");
		resCode = -1;
		fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
		return -1;
	}

	/* hjjung */
	else if(!strcasecmp(argSys, "SCMA")){
		devIndex = 0;
		almFlag=3;
		sprintf(devName, "SCMA");	
		pLegInfo = (LEG_SESS_NUM_INFO*)&g_pstCALLInfo->legInfo[devIndex];
		pCallInfo = (TPS_INFO*)&g_pstCALLInfo->tpsInfo;
	}
	else if(!strcasecmp(argSys, "SCMB")){
		devIndex = 1;
		almFlag=3;
		sprintf(devName, "SCMB");	
		pLegInfo = (LEG_SESS_NUM_INFO*)&g_pstCALLInfo->legInfo[devIndex];
		pCallInfo = (TPS_INFO*)&g_pstCALLInfo->tpsInfo;
	}


	// 아래의 type 에는 unit 이 없다.
	if (!strcasecmp(argType, "CPU") || !strcasecmp(argType, "MEM") || !strcasecmp(argType, "QUE") || 
		// TAP 의 CPU/MEM 상태 감시 기능 배제 2009/10/14
//		!strcasecmp(argType, "DUP_HB") || !strcasecmp(argType, "TAP-CPU") || !strcasecmp(argType, "TAP-MEM") ||
		!strcasecmp(argType, "DUP_HB") || 
		!strcasecmp(argType, "SESSION") || // hjjung
		!strcasecmp(argType, "SCE-DISK") || !strcasecmp(argType,"SCE-USER") || !strcasecmp(argType,"SCE-FAN") || !strcasecmp(argType,"SCE-PWR") ||
		!strcasecmp(argType, "SCE-TEMP") || !strcasecmp(argType,"SCE-STAT") ||
// added by uamyd 20110209, LOGON 성공율 감시
		!strcasecmp(argType, "LOGON-SUCC") || !strcasecmp(argType, "LOGOUT-SUCC") ||
		!strcasecmp(argType, "L2-CPU") || !strcasecmp(argType,"L2-MEM") || !strcasecmp(argType,"CPS_LOAD"))
	{
		/* unit 이 들어오지 않아야 하는 경우인데 들어온 경우. */
		if(strcasecmp(argUnit,"")) {
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNIT PARA NOT REQUIRED\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}

	} else {
		if(!strcasecmp(argUnit,"")) {
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNIT PARA REQUIRED\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}
	}

	if (!strcasecmp(argType, "CPU")) {
		for(unitIndex=0; unitIndex<sfdb->sys[sysIndex].commInfo.cpuCnt; unitIndex++ ){
			if (sfdb->sys[sysIndex].commInfo.cpuInfo.mask[unitIndex] != SFM_ALM_MASKED) {
				sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY UNMASKED\n");
				resCode = -1;
			} 
		}
		if(resCode == 0){
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
				"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='CPU%d')",
				SFM_CURR_ALM_DB_TABLE_NAME,
				0, 
				commlib_printDateTime(time(NULL)),
				sfdb->sys[sysIndex].commInfo.type,
				sfdb->sys[sysIndex].commInfo.group,
				sfdb->sys[sysIndex].commInfo.name,
				SFM_ALM_TYPE_CPU_USAGE,
				unitIndex);
			
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_umask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				for(unitIndex=0; unitIndex<sfdb->sys[sysIndex].commInfo.cpuCnt; unitIndex++ ){
					sfdb->sys[sysIndex].commInfo.cpuInfo.mask[unitIndex] = SFM_ALM_NORMAL;
				}
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
		
	}

	else if (!strcasecmp(argType, "DISK")) {
		if ((unitIndex = fimd_getDiskIndexByName(sysIndex, argUnit)) < 0) {
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN DISK PARTITION\n");
			resCode = -1;
		} else if (sfdb->sys[sysIndex].commInfo.diskInfo[unitIndex].mask != SFM_ALM_MASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY UNMASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
				"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s')",
				SFM_CURR_ALM_DB_TABLE_NAME,
				0, 
				commlib_printDateTime(time(NULL)),
				sfdb->sys[sysIndex].commInfo.type,
				sfdb->sys[sysIndex].commInfo.group,
				sfdb->sys[sysIndex].commInfo.name,
				SFM_ALM_TYPE_DISK_USAGE,
				sfdb->sys[sysIndex].commInfo.diskInfo[unitIndex].name);
			
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_umask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				sfdb->sys[sysIndex].commInfo.diskInfo[unitIndex].mask = SFM_ALM_NORMAL;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}

	else if (!strcasecmp(argType, "LAN")) {
		if ((unitIndex = fimd_getLanIndexByName(sysIndex, argUnit)) < 0) {
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN LAN NAME\n");
			resCode = -1;
		} else if (sfdb->sys[sysIndex].commInfo.lanInfo[unitIndex].mask != SFM_ALM_MASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY UNMASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
				"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s(%s)')",
				SFM_CURR_ALM_DB_TABLE_NAME,
				0, 
				commlib_printDateTime(time(NULL)),
				sfdb->sys[sysIndex].commInfo.type,
				sfdb->sys[sysIndex].commInfo.group,
				sfdb->sys[sysIndex].commInfo.name,
				SFM_ALM_TYPE_LAN,
				sfdb->sys[sysIndex].commInfo.lanInfo[unitIndex].name,
				sfdb->sys[sysIndex].commInfo.lanInfo[unitIndex].targetIp);
			
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_umask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				sfdb->sys[sysIndex].commInfo.lanInfo[unitIndex].mask = SFM_ALM_NORMAL;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}
    
	/* by helca */
	else if (!strcasecmp(argType, "RMTLAN")) {
		if ((unitIndex = fimd_getRmtLanIndexByName(sysIndex, argUnit)) < 0) {
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN RMTLAN NAME\n");
			resCode = -1;
		} else if (sfdb->sys[sysIndex].commInfo.rmtLanInfo[unitIndex].mask != SFM_ALM_MASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY UNMASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
				"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s(%s)')",
				SFM_CURR_ALM_DB_TABLE_NAME,
				0, 
				commlib_printDateTime(time(NULL)),
				sfdb->sys[sysIndex].commInfo.type,
				sfdb->sys[sysIndex].commInfo.group,
				sfdb->sys[sysIndex].commInfo.name,
				SFM_ALM_TYPE_RMT_LAN,
				&sfdb->sys[sysIndex].commInfo.rmtLanInfo[unitIndex].name[1],
				sfdb->sys[sysIndex].commInfo.rmtLanInfo[unitIndex].targetIp);
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_unmask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				sfdb->sys[sysIndex].commInfo.rmtLanInfo[unitIndex].mask = SFM_ALM_NORMAL;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}
#if 0
	// sjjeon HW로 편입 (e1000g3, e1000g5)
	/* by helca */
	else if (!strcasecmp(argType, "MIRR_PORT")) {
		if ((unitIndex = fimd_getOptLanIndexByName(sysIndex, argUnit)) < 0) {
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN OPTLAN NAME\n");
			resCode = -1;
		} else if (sfdb->sys[sysIndex].commInfo.optLanInfo[unitIndex].mask != SFM_ALM_MASKED) {
			//sprintf(resBuf,"    RESULT = FAIL\n    REASON = ALREADY UNMASKED\n");
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY UNMASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
				"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s')",
				SFM_CURR_ALM_DB_TABLE_NAME,
				0, 
				commlib_printDateTime(time(NULL)),
				sfdb->sys[sysIndex].commInfo.type,
				sfdb->sys[sysIndex].commInfo.group,
				sfdb->sys[sysIndex].commInfo.name,
				SFM_ALM_TYPE_OPT_LAN,
				sfdb->sys[sysIndex].commInfo.optLanInfo[unitIndex].name
				);
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_unmask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				sfdb->sys[sysIndex].commInfo.optLanInfo[unitIndex].mask = SFM_ALM_NORMAL;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}
#endif

	/* by helca */
	else if (!strcasecmp(argType, "DUP_HB")) {
		if (sfdb->sys[sysIndex].commInfo.systemDup.mask != SFM_ALM_MASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY UNMASKED\n");
			resCode = -1;
		} else {
			#if 0
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
				"system_group='%s' AND system_name='%s' AND alarm_type=%d)",
				SFM_CURR_ALM_DB_TABLE_NAME,
				0, 
				commlib_printDateTime(time(NULL)),
				sfdb->sys[sysIndex].commInfo.type,
				sfdb->sys[sysIndex].commInfo.group,
				sfdb->sys[sysIndex].commInfo.name,
				SFM_ALM_TYPE_DUP_HEARTBEAT
				);
			
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_unmask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				sfdb->sys[sysIndex].commInfo.systemDup.mask = SFM_ALM_NORMAL;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
			#endif
			
			int almType[4]={SFM_ALM_TYPE_DUP_HEARTBEAT,SFM_ALM_TYPE_DUAL_ACT,SFM_ALM_TYPE_DUAL_STD,SFM_ALM_TYPE_DUAL_STS_QRY_TIME_OUT};
			for(i=0;i<4;i++){
				sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
                                	"system_group='%s' AND system_name='%s' AND alarm_type=%d)",
                                	SFM_CURR_ALM_DB_TABLE_NAME,
                                	0,
                                	commlib_printDateTime(time(NULL)),
                                	sfdb->sys[sysIndex].commInfo.type,
                                	sfdb->sys[sysIndex].commInfo.group,
                                	sfdb->sys[sysIndex].commInfo.name,
                                	almType[i]
                                	);

                        	if (fimd_mysql_query (query) < 0) {
                                	sprintf(trcBuf,"[fimd_mmc_unmask_alm] fimd_mysql_query fail\n");
                                	trclib_writeLogErr (FL,trcBuf);
                                	sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
                                	resCode = -1;
                        	} else {
                                	sfdb->sys[sysIndex].commInfo.systemDup.mask = SFM_ALM_NORMAL;
                                	sprintf(resBuf,"    RESULT = SUCCESS\n");
                                	resCode = 0;
                        	}
			}
		}
	}

	/* by helca */
	else if (!strcasecmp(argType, "NTP")) {
		if ((unitIndex = fimd_gethwNtpIndexByName(sysIndex, argUnit)) < 0) {
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN NTP NAME\n");
			resCode = -1;
		}else if (sfdb->sys[sysIndex].commInfo.ntpSts[unitIndex].mask != SFM_ALM_MASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY UNMASKED\n");
			resCode = -1;
		} else {
			// added by dcham 20110523, where조건절이 틀려서 마스크 처리 안되었음. "NTP-%s: => NTP %s"
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
				"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information like 'NTP %s%%')",
				SFM_CURR_ALM_DB_TABLE_NAME,
				0, 
				commlib_printDateTime(time(NULL)),
				sfdb->sys[sysIndex].commInfo.type,
				sfdb->sys[sysIndex].commInfo.group,
				sfdb->sys[sysIndex].commInfo.name,
				SFM_ALM_TYPE_HWNTP,
				argUnit
				);
			
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_unmask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				sfdb->sys[sysIndex].commInfo.ntpSts[unitIndex].mask = SFM_ALM_NORMAL;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}

#if 0
	/* by helca */
	else if (!strcasecmp(argType, "SUCC_RATE")) {
		if( !strcasecmp(argUnit, "RADIUS")){
        		if( strlen(argPDSNIP) < 7) {
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = NO INPUT PDSN IP\n");
				resCode = -1;
			}else if ((pdsnIndex = fimd_getPDSNIPIndexByName(sysIndex, argPDSNIP)) < 0) {
                		sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN PDSN IP\n");
                		resCode = -1;
        		} else if (sfdb->sys[sysIndex].succRateIpInfo.radius[pdsnIndex].mask != SFM_ALM_MASKED) {
                		//sprintf(resBuf,"    RESULT = FAIL\n    REASON = ALREADY UMASKED\n");
                		sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY UMASKED\n");
                		resCode = -1;
        		} else {
                		sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' "
                        		"WHERE (system_type='%s' AND system_group='%s' "
                        		"AND system_name='%s' AND alarm_type=%d AND information LIKE '%s(%s)%%')",
                        		SFM_CURR_ALM_DB_TABLE_NAME,
                        		0,
                        		commlib_printDateTime(time(NULL)),
                        		sfdb->sys[sysIndex].commInfo.type,
                        		sfdb->sys[sysIndex].commInfo.group,
                        		sfdb->sys[sysIndex].commInfo.name,
                        		SFM_ALM_TYPE_SUCC_RATE,
                        		"RADIUS",
					sfdb->sys[sysIndex].succRateIpInfo.radius[pdsnIndex].ipAddr
                		);
                		if (fimd_mysql_query (query) < 0) {
                        		sprintf(trcBuf,"[fimd_mmc_umask_alm] fimd_mysql_query fail\n");
                        		trclib_writeLogErr (FL,trcBuf);
                        		sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
                        		resCode = -1;
                		} else {
                        		sfdb->sys[sysIndex].succRateIpInfo.radius[pdsnIndex].mask = SFM_ALM_NORMAL;
                        		sprintf(resBuf,"    RESULT = SUCCESS\n");
                        		resCode = 0;
                		}

        		}
			
		} else if ((unitIndex = fimd_getSuccRateIndexByName(sysIndex, argUnit)) < 0) {
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN SUCC_RATE NAME\n");
			resCode = -1;
		} else if (sfdb->sys[sysIndex].commInfo.succRate[unitIndex].mask != SFM_ALM_MASKED) {
			//sprintf(resBuf,"    RESULT = FAIL\n    REASON = ALREADY UNMASKED\n");
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY UNMASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND system_group='%s' AND system_name='%s' AND alarm_type=%d AND information LIKE '%s(%%')",
				SFM_CURR_ALM_DB_TABLE_NAME,
				0, 
				commlib_printDateTime(time(NULL)),
				sfdb->sys[sysIndex].commInfo.type,
				sfdb->sys[sysIndex].commInfo.group,
				sfdb->sys[sysIndex].commInfo.name,
				SFM_ALM_TYPE_SUCC_RATE,
				sfdb->sys[sysIndex].commInfo.succRate[unitIndex].name
			);
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_unmask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				sfdb->sys[sysIndex].commInfo.succRate[unitIndex].mask = SFM_ALM_NORMAL;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}

	/* by helca */
	else if (!strcasecmp(argType, "UAWAP_DB")) {
		if ((unitIndex = fimd_getRmtDbLanIndexByName(sysIndex, argUnit)) < 0) {
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN UAWAP_DB IP\n");
			resCode = -1;
		} else if (sfdb->sys[sysIndex].commInfo.rmtDbSts[unitIndex].mask != SFM_ALM_MASKED) {
			//sprintf(resBuf,"    RESULT = FAIL\n    REASON = ALREADY UNMASKED\n");
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY UNMASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s(%s)')",
				SFM_CURR_ALM_DB_TABLE_NAME,
				0, 
				commlib_printDateTime(time(NULL)),
				sfdb->sys[sysIndex].commInfo.type,
				sfdb->sys[sysIndex].commInfo.group,
				sfdb->sys[sysIndex].commInfo.name,
				SFM_ALM_TYPE_DBCON_STST,
				sfdb->sys[sysIndex].commInfo.rmtDbSts[unitIndex].sDbAlias,
				sfdb->sys[sysIndex].commInfo.rmtDbSts[unitIndex].sIpAddress
				);
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_unmask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				sfdb->sys[sysIndex].commInfo.rmtDbSts[unitIndex].mask = SFM_ALM_NORMAL;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}
#endif

	/* by helca */
	else if (!strcasecmp(argType, "TAP-CPU")) {
		if(!strncasecmp(argSys, "DSC", 3)){
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN TYPE NAME\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}else if (l3pd->l3ProbeDev[devIndex].cpuInfo.mask != SFM_ALM_MASKED) {
			//sprintf(resBuf,"    RESULT = FAIL\n    REASON = ALREADY UNMASKED\n");
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY UNMASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND system_group='%s' "
													"AND system_name='%s' AND alarm_type=%d AND information='%s')",
				SFM_CURR_ALM_DB_TABLE_NAME,
				0, 
				commlib_printDateTime(time(NULL)),
				"TAP",
				"TAP",
				devName,
				//SFM_ALM_TYPE_PD_CPU_USAGE,
				SFM_ALM_TYPE_TAP_CPU_USAGE,
				SQL_PD_CPU_ALM_INFO);
//fprintf(stderr, "%s.\n", query);

			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_umask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				l3pd->l3ProbeDev[devIndex].cpuInfo.mask = SFM_ALM_NORMAL;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}

	/* by helca */
	else if (!strcasecmp(argType, "TAP-MEM")) {
		if(!strncasecmp(argSys, "DSC", 3)){
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN TYPE NAME\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}else if (l3pd->l3ProbeDev[devIndex].memInfo.mask != SFM_ALM_MASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY UNMASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND system_group='%s' AND "
								"system_name='%s' AND alarm_type=%d AND information='Memory')",
				SFM_CURR_ALM_DB_TABLE_NAME,
				0, 
				commlib_printDateTime(time(NULL)),
				"TAP",
				"TAP",
				devName,
				//SFM_ALM_TYPE_PD_MEMORY_USAGE);
				SFM_ALM_TYPE_TAP_MEMORY_USAGE);

			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_umask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				l3pd->l3ProbeDev[devIndex].memInfo.mask = SFM_ALM_NORMAL;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}

#if 0 /* by helca */
	else if (!strcasecmp(argType, "PD-FAN")) {
		unitIndex = atoi(argUnit);
		if(!strncasecmp(argSys, "DSC", 3)){
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN TYPE NAME\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}else if (l3pd->l3ProbeDev[devIndex].fanInfo.mask[unitIndex] != SFM_ALM_MASKED) {
			//sprintf(resBuf,"    RESULT = FAIL\n    REASON = ALREADY UNMASKED\n");
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY UNMASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='FAN(%d)')",
				SFM_CURR_ALM_DB_TABLE_NAME,
				0, 
				commlib_printDateTime(time(NULL)),
				"PD",
				"PD",
				devName,
				//SFM_ALM_TYPE_PD_FAN_STS,
				SFM_ALM_TYPE_TAP_FAN_STS,
				unitIndex);
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_umask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				l3pd->l3ProbeDev[devIndex].fanInfo.mask[unitIndex] = SFM_ALM_NORMAL;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}
#endif

	/* by helca */
	else if (!strcasecmp(argType, "TAP-PORT")) {
		unitIndex = atoi(argUnit)-1;
		if(!strncasecmp(argSys, "DSC", 3)){
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN TYPE NAME\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}else if (l3pd->l3ProbeDev[devIndex].gigaLanInfo[unitIndex].mask != SFM_ALM_MASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY UNMASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
					"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s(%d)')",
					SFM_CURR_ALM_DB_TABLE_NAME,
					0, 
					commlib_printDateTime(time(NULL)),
					"TAP",
					"TAP",
					devName,
					//SFM_ALM_TYPE_PD_GIGA_LAN,
					SFM_ALM_TYPE_TAP_PORT_STS,
					SQL_TAP_PORT_ALM_INFO,
					unitIndex+1
					);
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_umask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				l3pd->l3ProbeDev[devIndex].gigaLanInfo[unitIndex].mask = SFM_ALM_NORMAL;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;

			}
		}

	}
	else if (!strcasecmp(argType, "TAP-POWER")) { // 20110424 by dcham
		unitIndex = atoi(argUnit)-1;
		if(!strncasecmp(argSys, "DSC", 3)){
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN TYPE NAME\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}else if (l3pd->l3ProbeDev[devIndex].powerInfo[unitIndex].mask != SFM_ALM_MASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY UNMASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
					"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s(%d)')",
					SFM_CURR_ALM_DB_TABLE_NAME,
					0,
					commlib_printDateTime(time(NULL)),
					"TAP",
					"TAP",
					devName,
					//SFM_ALM_TYPE_PD_GIGA_LAN,
					SFM_ALM_TYPE_TAP_POWER_STS,
					SQL_TAP_POWER_ALM_INFO,
					unitIndex+1
				   );
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_umask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				l3pd->l3ProbeDev[devIndex].powerInfo[unitIndex].mask = SFM_ALM_NORMAL;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}
#if 0
	/* by sjjeon */
	else if (!strcasecmp(argType, "L2-PORT")) {
		unitIndex = atoi(argUnit)-1;
		if(!strncasecmp(argSys, "DSC", 3)){
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN TYPE NAME\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}else if (l3pd->l3ProbeDev[devIndex].gigaLanInfo[unitIndex].mask != SFM_ALM_MASKED) {
			//sprintf(resBuf,"    RESULT = FAIL\n    REASON = ALREADY UNMASKED\n");
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY UNMASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
					"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='LINK(%d)')",
					SFM_CURR_ALM_DB_TABLE_NAME,
					0, 
					commlib_printDateTime(time(NULL)),
					"TAP",
					"TAP",
					devName,
					//SFM_ALM_TYPE_PD_GIGA_LAN,
					SFM_ALM_TYPE_TAP_PORT_STS,
					unitIndex+1);
			//  printf("UNMASK SQL\n[%s]\n", query);
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_umask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				l3pd->l3ProbeDev[devIndex].gigaLanInfo[unitIndex].mask = SFM_ALM_NORMAL;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;

			}
		}

	}
#endif 

	else if (!strcasecmp(argType, "PROC")) {
		if ((unitIndex = fimd_getProcIndexByName(sysIndex, argUnit)) < 0) {
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN PROCESS\n");
			resCode = -1;
		} else if (sfdb->sys[sysIndex].commInfo.procInfo[unitIndex].mask != SFM_ALM_MASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY UNMASKED\n");
			resCode = -1;
		} else {
			int almType;

#ifndef _NOT_USED_PROCESS_ALARM_TYPE_
			/* unitIndex로 alarm type 획득 : sjjeon */
			almType = getAlarmTypeFromProcName(sysIndex, unitIndex);
#else
			almType = SFM_ALM_TYPE_PROC;
#endif
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND system_group='%s' AND "
								"system_name='%s' AND alarm_type=%d AND information='%s')",
				SFM_CURR_ALM_DB_TABLE_NAME,
				0, 
				commlib_printDateTime(time(NULL)),
				sfdb->sys[sysIndex].commInfo.type,
				sfdb->sys[sysIndex].commInfo.group,
				sfdb->sys[sysIndex].commInfo.name,
				//SFM_ALM_TYPE_PROC,
				almType,
				sfdb->sys[sysIndex].commInfo.procInfo[unitIndex].name);
			
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_umask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				sfdb->sys[sysIndex].commInfo.procInfo[unitIndex].mask = SFM_ALM_NORMAL;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}

	else if (!strcasecmp(argType, "MEM")) {

		if (sfdb->sys[sysIndex].commInfo.memInfo.mask != SFM_ALM_MASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY UNMASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND system_group='%s' AND "
								"system_name='%s' AND alarm_type=%d AND information='Memory')",
				SFM_CURR_ALM_DB_TABLE_NAME,
				0, 
				commlib_printDateTime(time(NULL)),
				sfdb->sys[sysIndex].commInfo.type,
				sfdb->sys[sysIndex].commInfo.group,
				sfdb->sys[sysIndex].commInfo.name,
				SFM_ALM_TYPE_MEMORY_USAGE);
			
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_umask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				sfdb->sys[sysIndex].commInfo.memInfo.mask = SFM_ALM_NORMAL;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}

	// H/W List에 해당하는 목록들은 여기서 관리 .. e1000g0~7, mysql, timesten, sm,cm, DISK1, DISK2, FAN...
	else if (!strcasecmp(argType, "HW")|| !strcasecmp(argType, "MIRR_PORT")) {

		int almType;
		if(!strcasecmp(argType, "MIRR_PORT")){
			almType = SFM_ALM_TYPE_HW_MIRROR;       // mp h/w mirror port
		}else{
			almType = SFM_ALM_TYPE_MP_HW;
		}

		len = strlen(argUnit);
		if ((unitIndex = fimd_getHwIndexByName(sysIndex, argUnit)) < 0) {
			sprintf(resBuf,"    RESULT = FAIL\n      REASON = UNKNOWN HW NAME\n");
			resCode = -1;
		}
		if (sfdb->sys[sysIndex].specInfo.u.sms.hpuxHWInfo.hwcom[unitIndex].mask != SFM_ALM_MASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n      REASON = ALREADY UNMASKED\n");
			resCode = -1;
		} else {
			char hwName[16]; bzero(hwName,sizeof(hwName));
			hw_name_mapping((char*)argUnit,strlen(argUnit),(char*)hwName);
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
								"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s')",
				SFM_CURR_ALM_DB_TABLE_NAME,
				0, 
				commlib_printDateTime(time(NULL)),
				sfdb->sys[sysIndex].commInfo.type,
				sfdb->sys[sysIndex].commInfo.group,
				sfdb->sys[sysIndex].commInfo.name,
				almType, 
				hwName); 

				fprintf(stdout,"[Umask HW or MIRR_PORT] %s\n",query);			
				
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_umask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n      REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				sfdb->sys[sysIndex].specInfo.u.sms.hpuxHWInfo.hwcom[unitIndex].mask = SFM_ALM_NORMAL;
				sprintf(resBuf, "    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}

	/* by helca */
	else if (!strcasecmp(argType, "QUE")) {
		for(i=0; i<SFM_MAX_QUE_CNT; i++){
			if (sfdb->sys[sysIndex].commInfo.queInfo[i].mask != SFM_ALM_MASKED) {
				sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY UNMASKED\n");
				resCode = -1;
			}
		}
	
		sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND system_group='%s' AND "
						"system_name='%s' AND alarm_type=%d)",
			SFM_CURR_ALM_DB_TABLE_NAME,
			0, 
			commlib_printDateTime(time(NULL)),
			sfdb->sys[sysIndex].commInfo.type,
			sfdb->sys[sysIndex].commInfo.group,
			sfdb->sys[sysIndex].commInfo.name,
			SFM_ALM_TYPE_QUEUE_LOAD);
			
		if (fimd_mysql_query (query) < 0) {
			sprintf(trcBuf,"[fimd_mmc_umask_alm] fimd_mysql_query fail\n");
			trclib_writeLogErr (FL,trcBuf);
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
			resCode = -1;
		} else {
			for(i=0; i<SFM_MAX_QUE_CNT; i++){
				sfdb->sys[sysIndex].commInfo.queInfo[i].mask = SFM_ALM_NORMAL;
				resCode = 0;
			}
			sprintf(resBuf,"    RESULT = SUCCESS\n");
		}
	
	}

	// by helca 10.30
	else if (!strcasecmp(argType, "NMS")) {
		    
		    if(strncasecmp(argSys, "DSCM", 4)){
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN SYS NAME\n");
				resCode = -1;
				fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
				return -1;
		    }
		
		    if ((unitIndex = fimd_getNmsifIndexByName(sysIndex, argUnit)) < 0) {
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN NMSIF NAME\n");
				resCode = -1;
			} else if (sfdb->nmsInfo.mask[unitIndex] != SFM_ALM_MASKED) {
				sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY UNMASKED\n");
				resCode = -1;
			}
			else {
				sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND system_group='%s' AND "
								"system_name='%s' AND alarm_type=%d AND information='NMSIF ALARM %s')",
					SFM_CURR_ALM_DB_TABLE_NAME,
					0, 
					commlib_printDateTime(time(NULL)),
					sfdb->sys[sysIndex].commInfo.type,
					sfdb->sys[sysIndex].commInfo.group,
					sfdb->sys[sysIndex].commInfo.name,
					SFM_ALM_TYPE_NMSIF_CONNECT,
					nmsifName[unitIndex]
					);
				if (fimd_mysql_query (query) < 0) {
					sprintf(trcBuf,"[fimd_mmc_mask_alm] fimd_mysql_query fail\n");
					trclib_writeLogErr (FL,trcBuf);
					sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
					resCode = -1;
				} else {
					sfdb->nmsInfo.mask[unitIndex] = SFM_ALM_NORMAL;
					sprintf(resBuf,"    RESULT = SUCCESS\n");
					resCode = 0;
				}
			}
	}

	// by sjjeon  (SCE Alarm)
	else if (!strcasecmp(argType, "SCE-STAT"))
	{
		//unitIndex = atoi(argUnit)-1;
		if(!strncasecmp(argSys, "DSC", 3) || !strncasecmp(argSys, "SCM", 3) || !strncasecmp(argSys, "TAP", 3)){
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN TYPE NAME\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}else if (pSceDev->sysStatus.mask == SFM_ALM_UNMASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY UNMASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
					"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s')",
					SFM_CURR_ALM_DB_TABLE_NAME,
					0,
					commlib_printDateTime(time(NULL)),
					"SCE",
					"SCE",
					devName,
					SFM_ALM_TYPE_SCE_STATUS,
					SQL_SCE_STATUS_ALM_INFO
				   );
//printf("SCE-STAT : query [%s]", query);
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_mask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				pSceDev->sysStatus.mask = SFM_ALM_UNMASKED;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}

	else if (!strcasecmp(argType, "SCE-CPU"))
	{
//printf("type : %s, argUnit : %s\n", argType, argUnit);
		if(!strcasecmp(argUnit,"CPU-1")) unitIndex=0;
		else if(!strcasecmp(argUnit,"CPU-2")) unitIndex=1;
		else if(!strcasecmp(argUnit,"CPU-3")) unitIndex=2;
		else {
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN UNIT NAME\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}

		if(!strncasecmp(argSys, "DSC", 3) || !strncasecmp(argSys, "SCM", 3) || 
				!strncasecmp(argSys, "TAP", 3) || !strncasecmp(argSys, "L2SW",4)){
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN TYPE NAME\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}else if (pSceDev->cpuInfo[unitIndex].mask == SFM_ALM_UNMASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY UNMASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
					"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s(%d)')",
					SFM_CURR_ALM_DB_TABLE_NAME,
					0,
					commlib_printDateTime(time(NULL)),
					"SCE",
					"SCE",
					devName,
					SFM_ALM_TYPE_SCE_CPU,
					SQL_SCE_CPU_ALM_INFO,
					unitIndex+1
				   );
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_mask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				//	pSceDev->cpuInfo[unitIndex].mask = SFM_ALM_UNMASKED;
				g_pstSCEInfo->SCEDev[devIndex].cpuInfo[unitIndex].mask = SFM_ALM_UNMASKED;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}

	else if (!strcasecmp(argType, "SCE-DISK"))
	{

		if(!strncasecmp(argSys, "DSC", 3) || !strncasecmp(argSys, "SCM", 3) || !strncasecmp(argSys, "TAP", 3)){
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN TYPE NAME\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}else if (pSceDev->diskInfo.mask == SFM_ALM_UNMASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY UNMASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
					"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s')",
					SFM_CURR_ALM_DB_TABLE_NAME,
					0,
					commlib_printDateTime(time(NULL)),
					"SCE",
					"SCE",
					devName,
					SFM_ALM_TYPE_SCE_DISK,
					SQL_SCE_DISK_ALM_INFO
				   );
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_mask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				pSceDev->diskInfo.mask = SFM_ALM_UNMASKED;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}

	/* hjjung */
	else if (!strcasecmp(argType, "SCE-USER"))
	{

		if(!strncasecmp(argSys, "DSC", 3) || !strncasecmp(argSys, "SCM", 3) || !strncasecmp(argSys, "TAP", 3)){
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN TYPE NAME\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}else if (pSceDev->userInfo.mask == SFM_ALM_UNMASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY UNMASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
					"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s')",
					SFM_CURR_ALM_DB_TABLE_NAME,
					0,
					commlib_printDateTime(time(NULL)),
					"SCE",
					"SCE",
					devName,
					SFM_ALM_TYPE_SCE_USER,
					SQL_SCE_USER_ALM_INFO
				   );
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_mask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				pSceDev->userInfo.mask = SFM_ALM_UNMASKED;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}

	else if (!strcasecmp(argType, "SCE-MEM"))
	{
		if(!strcasecmp(argUnit,"MEM-1")) unitIndex=0;
		else if(!strcasecmp(argUnit,"MEM-2")) unitIndex=1;
		else if(!strcasecmp(argUnit,"MEM-3")) unitIndex=2;
		else {
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN UNIT NAME\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}

		if(!strncasecmp(argSys, "DSC", 3) || !strncasecmp(argSys, "SCM", 3) || !strncasecmp(argSys, "TAP", 3)){
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN TYPE NAME\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}else if (pSceDev->memInfo[unitIndex].mask == SFM_ALM_UNMASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY UNMASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
					"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s(%d)')",
					SFM_CURR_ALM_DB_TABLE_NAME,
					0,
					commlib_printDateTime(time(NULL)),
					"SCE",
					"SCE",
					devName,
					SFM_ALM_TYPE_SCE_MEM,
					SQL_SCE_MEM_ALM_INFO,	
					unitIndex+1
				   );
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_mask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				pSceDev->memInfo[unitIndex].mask = SFM_ALM_UNMASKED;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}


	else if (!strcasecmp(argType, "SCE-PORT"))
	{
		int almtype;

		if(!strcasecmp(argUnit,"MNG-1")) { unitIndex=0; almtype = SFM_ALM_TYPE_SCE_PORT_MGMT;}
		else if(!strcasecmp(argUnit,"MNG-2")) {unitIndex=1; almtype = SFM_ALM_TYPE_SCE_PORT_MGMT;}
		else if(!strcasecmp(argUnit,"PORT-1")) { unitIndex=2; almtype = SFM_ALM_TYPE_SCE_PORT_LINK;}
		else if(!strcasecmp(argUnit,"PORT-2")) { unitIndex=3; almtype = SFM_ALM_TYPE_SCE_PORT_LINK;}
		else if(!strcasecmp(argUnit,"PORT-3")) { unitIndex=4; almtype = SFM_ALM_TYPE_SCE_PORT_LINK;}
		else if(!strcasecmp(argUnit,"PORT-4")) { unitIndex=5; almtype = SFM_ALM_TYPE_SCE_PORT_LINK;}
		else {
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN UNIT NAME\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}

		if(!strncasecmp(argSys, "DSC", 3) || !strncasecmp(argSys, "SCM", 3) || !strncasecmp(argSys, "TAP", 3)){
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN TYPE NAME\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}else if (pSceDev->portStatus[unitIndex].mask == SFM_ALM_UNMASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY UNMASKED\n");
			resCode = -1;
		} else {

			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
					"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s')",
					SFM_CURR_ALM_DB_TABLE_NAME,
					0,
					commlib_printDateTime(time(NULL)),
					"SCE",
					"SCE",
					devName,
					almtype,
					fimd_getScePortName(unitIndex)
				   );
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_mask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				pSceDev->portStatus[unitIndex].mask = SFM_ALM_UNMASKED;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}

	else if (!strcasecmp(argType, "SCE-FAN"))
	{

		if(!strncasecmp(argSys, "DSC", 3) || !strncasecmp(argSys, "SCM", 3) || !strncasecmp(argSys, "TAP", 3)){
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN TYPE NAME\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}else if (pSceDev->fanStatus.mask == SFM_ALM_UNMASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY UNMASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
					"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s')",
					SFM_CURR_ALM_DB_TABLE_NAME,
					0,
					commlib_printDateTime(time(NULL)),
					"SCE",
					"SCE",
					devName,
					SFM_ALM_TYPE_SCE_FAN,
					SQL_SCE_FAN_ALM_INFO
				   );
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_mask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				pSceDev->fanStatus.mask = SFM_ALM_UNMASKED;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}

	else if (!strcasecmp(argType, "SCE-PWR"))
	{

		if(!strncasecmp(argSys, "DSC", 3) || !strncasecmp(argSys, "SCM", 3) || !strncasecmp(argSys, "TAP", 3)){
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN TYPE NAME\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}else if (pSceDev->pwrStatus.mask == SFM_ALM_UNMASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY UNMASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
					"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s')",
					SFM_CURR_ALM_DB_TABLE_NAME,
					0,
					commlib_printDateTime(time(NULL)),
					"SCE",
					"SCE",
					devName,
					SFM_ALM_TYPE_SCE_PWR,
					SQL_SCE_PWR_ALM_INFO
				   );
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_mask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				pSceDev->pwrStatus.mask = SFM_ALM_UNMASKED;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}

	else if (!strcasecmp(argType, "SCE-TEMP"))
	{

		if(!strncasecmp(argSys, "DSC", 3) || !strncasecmp(argSys, "SCM", 3) || !strncasecmp(argSys, "TAP", 3)){
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN TYPE NAME\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}else if (pSceDev->tempStatus.mask == SFM_ALM_UNMASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY UNMASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
					"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s')",
					SFM_CURR_ALM_DB_TABLE_NAME,
					0,
					commlib_printDateTime(time(NULL)),
					"SCE",
					"SCE",
					devName,
					SFM_ALM_TYPE_SCE_TEMP,	
					SQL_SCE_TEMP_ALM_INFO	
				   );
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_mask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				pSceDev->tempStatus.mask = SFM_ALM_UNMASKED;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}

	else if (!strcasecmp(argType, "SCE-RDR-CONN"))
	{
		if(!strcasecmp(argUnit,"SCMA")) unitIndex = 0;
		else if(!strcasecmp(argUnit,"SCMB")) unitIndex = 1;
		else unitIndex = -1;

		if(unitIndex < 0){  
			sprintf(resBuf,"[%s] INVALID UNIT NUMBER (%s)\n",__FUNCTION__, argUnit);
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}

		if(!strncasecmp(argSys, "DSC", 3) || !strncasecmp(argSys, "SCM", 3) ||
				!strncasecmp(argSys, "TAP", 3) || !strncasecmp(argSys, "L2SW",4)){
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = INVALID TYPE NAME\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}

		if (pSceDev->rdrConnStatus[unitIndex].mask == SFM_ALM_UNMASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY UNMASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
					"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s(%d)')",
					SFM_CURR_ALM_DB_TABLE_NAME,
					0,
					commlib_printDateTime(time(NULL)),
					"SCE",
					"SCE",
					devName,
					SFM_ALM_TYPE_SCE_RDR_CONN,
					SQL_SCE_RDR_CONN_ALM_INFO,
					unitIndex+1
				   );
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_mask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				pSceDev->rdrConnStatus[unitIndex].mask = SFM_ALM_UNMASKED;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}

	/* hjjung */
	else if (!strcasecmp(argType, "SESSION"))
	{

		if(!strncasecmp(argSys, "DSC", 3) || !strncasecmp(argSys, "SCE", 3) || !strncasecmp(argSys, "TAP", 3)){
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN TYPE NAME\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}else if (pLegInfo->mask == SFM_ALM_UNMASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY UNMASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
					"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s')",
					SFM_CURR_ALM_DB_TABLE_NAME,
					0,
					commlib_printDateTime(time(NULL)),
					"SCM",
					"SCM",
					devName,
					SFM_ALM_TYPE_LEG_SESSION,
					SQL_LEG_SESSION_ALM_INFO
				   );
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_mask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				pLegInfo->mask = SFM_ALM_UNMASKED;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}

    // added by dcham 20110525 for TPS
	else if (!strcasecmp(argType, "TPS_LOAD"))
	{

		if(!strncasecmp(argSys, "DSC", 3) || !strncasecmp(argSys, "SCE", 3) || !strncasecmp(argSys, "TAP", 3)){
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN TYPE NAME\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}else if (pCallInfo->mask == SFM_ALM_UNMASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY UNMASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
					"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s')",
					SFM_CURR_ALM_DB_TABLE_NAME,
					0,
					commlib_printDateTime(time(NULL)),
					"SCM",
					"SCM",
					devName,
					SFM_ALM_TYPE_TPS,
					SQL_TPS_ALM_INFO
				   );
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_mask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				pCallInfo->mask = SFM_ALM_UNMASKED;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}

	/* L2 Switch CPU : sjjeon*/
	else if (!strcasecmp(argType, "L2-CPU"))
	{
		//printf("type : %s, argUnit : %s\n", argType, argUnit);

		// argSys 가 DSC, SCM, TAP, SCE인 경우 오류. only L2SW
		if(!strncasecmp(argSys, "DSC", 3) || !strncasecmp(argSys, "SCM", 3) || 
				!strncasecmp(argSys, "TAP", 3) || !strncasecmp(argSys, "SCE", 3)){
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN TYPE NAME\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}else if (pL2Info->cpuInfo.mask == SFM_ALM_UNMASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY UNMASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
				"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s')",
				SFM_CURR_ALM_DB_TABLE_NAME,
				0,
				commlib_printDateTime(time(NULL)),
				"L2SW",
				"L2SW",
				devName,
				SFM_ALM_TYPE_L2_CPU,
				SQL_L2SW_CPU_ALM_INFO
			   );
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[%s] fimd_mysql_query fail\n", __FUNCTION__);
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				g_pstL2Dev->l2Info[devIndex].cpuInfo.mask = SFM_ALM_UNMASKED;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}

	/* L2 Switch MEM : sjjeon*/
	else if (!strcasecmp(argType, "L2-MEM"))
	{
		//printf("type : %s, argUnit : %s\n", argType, argUnit);

		// argSys 가 DSC, SCM, TAP, SCE인 경우 오류. only L2SW
		if(!strncasecmp(argSys, "DSC", 3) || !strncasecmp(argSys, "SCM", 3) || 
				!strncasecmp(argSys, "TAP", 3) || !strncasecmp(argSys, "SCE", 3)){
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN TYPE NAME\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}else if (pL2Info->cpuInfo.mask == SFM_ALM_UNMASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY UNMASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
				"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s')",
				SFM_CURR_ALM_DB_TABLE_NAME,
				0,
				commlib_printDateTime(time(NULL)),
				"L2SW",
				"L2SW",
				devName,
				SFM_ALM_TYPE_L2_MEM,
				SQL_L2SW_MEM_ALM_INFO
			   );
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[%s] fimd_mysql_query fail\n", __FUNCTION__);
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				g_pstL2Dev->l2Info[devIndex].memInfo.mask = SFM_ALM_UNMASKED;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}

	else if (!strcasecmp(argType, "L2-PORT"))
	{
		int portNum;
		portNum = atoi(argUnit);
		// port (1~27)
		if(portNum==0 || portNum>MAX_L2_PORT_NUM)
		{
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN PORT NUM(%d)\n", portNum);
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}

		if(!strncasecmp(argSys, "DSC", 3) || !strncasecmp(argSys, "SCM", 3) || 
				!strncasecmp(argSys, "TAP", 3) || !strncasecmp(argSys, "SCE", 3)){
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN TYPE NAME\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		// mmc로 입력 받은 portNum은 array +1 이므로 ...
		}else if (pL2Info->portInfo[portNum-1].mask == SFM_ALM_UNMASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY UNMASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
					"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s(%d)')",
					SFM_CURR_ALM_DB_TABLE_NAME,
					0,
					commlib_printDateTime(time(NULL)),
					"L2SW",
					"L2SW",
					devName,
					SFM_ALM_TYPE_L2_LAN,
					SQL_L2SW_PORT_ALM_INFO,
					portNum
				   );
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_mask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				pL2Info->portInfo[portNum-1].mask = SFM_ALM_UNMASKED;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	}

	// CPS LOAD UnMask
	else if (!strcasecmp(argType, "CPS_LOAD"))
	{
		if (sfdb->sys[sysIndex].commInfo.cpsOverSts.mask == SFM_ALM_UNMASKED) {
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY UNMASKED\n");
			resCode = -1;
		} else {
			sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
					"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='%s')",
					SFM_CURR_ALM_DB_TABLE_NAME,
					0,
					commlib_printDateTime(time(NULL)),
					sfdb->sys[sysIndex].commInfo.type,
					sfdb->sys[sysIndex].commInfo.group,
					sfdb->sys[sysIndex].commInfo.name,
					SFM_ALM_TYPE_CPS_OVER,
					"OvER_CPS"
				   );
			if (fimd_mysql_query (query) < 0) {
				sprintf(trcBuf,"[fimd_mmc_mask_alm] fimd_mysql_query fail\n");
				trclib_writeLogErr (FL,trcBuf);
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
				resCode = -1;
			} else {
				sfdb->sys[sysIndex].commInfo.cpsOverSts.mask = SFM_ALM_UNMASKED;
				sprintf(resBuf,"    RESULT = SUCCESS\n");
				resCode = 0;
			}
		}
	} /*End of CPS_LOAD*/

	// SM Connection Status added by uamyd 20110425
	else if (!strcasecmp(argType, "SMLINK"))
	{
#if 0
		// added by dcham 20110530 for SM connectiion(5=>1)
		int sm_ch_id = atoi(argUnit);

		sm_ch_id--; //Client 에서는 1,2,3,4,5 를 보내준다. 

		if( sm_ch_id < 0 || sm_ch_id >= SFM_MAX_SM_CH_CNT ){
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN CHANNEL ID\n");
			resCode = -1;
			fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);
			return -1;
		}else{
#endif
			int sm_ch_id = 0; // added by dcham 20110530
			if (sfdb->sys[sysIndex].commInfo.smChSts.each[sm_ch_id].mask == SFM_ALM_UNMASKED) {
				sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY UNMASKED\n");
				resCode = -1;
			} else {
				sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
						"system_group='%s' AND system_name='%s' AND alarm_type=%d AND information='SMLINK(%d)')",
						SFM_CURR_ALM_DB_TABLE_NAME,
						SFM_ALM_UNMASKED,
						commlib_printDateTime(time(NULL)),
						sfdb->sys[sysIndex].commInfo.type,
						sfdb->sys[sysIndex].commInfo.group,
						sfdb->sys[sysIndex].commInfo.name,
						SFM_ALM_TYPE_SM_CONN_STS,
						sm_ch_id
					   );
				if (fimd_mysql_query (query) < 0) {
					sprintf(trcBuf,"[fimd_mmc_umask_alm] fimd_mysql_query fail\n");
					trclib_writeLogErr (FL,trcBuf);
					sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
					resCode = -1;
				} else {
					sfdb->sys[sysIndex].commInfo.smChSts.each[sm_ch_id].mask = SFM_ALM_UNMASKED;
					sprintf(resBuf,"    RESULT = SUCCESS\n");
					resCode = 0;
				}
			}
			//	} added by dcham 20110530
	}

	/*
	 * LOGON 성공율 통계를 위한 UNMASK
	 * added by uamyd 20110209
	 */
	else if (!strcasecmp(argType, "LOGON-SUCC") || !strcasecmp(argType, "LOGOUT-SUCC")) {

		int log_mod, almType;
		if ( !strcasecmp(argType, "LOGON-SUCC") ){
			log_mod = 0;
			almType = SFM_ALM_TYPE_LOGON_SUCCESS_RATE;
		} else {
			log_mod = 1;
			almType = SFM_ALM_TYPE_LOGOUT_SUCCESS_RATE;
		}
		if ( !devIndex ) { g_pstLogonRate = &g_stLogonRate[log_mod][0]; }
		else             { g_pstLogonRate = &g_stLogonRate[log_mod][1]; }

		if ( g_pstLogonRate->mask != SFM_ALM_MASKED ){
			sprintf(resBuf,"    RESULT = SUCCESS\n    REASON = ALREADY UNMASKED\n");
			resCode = -1;
        }

        if(resCode == 0){

            sprintf(query, "UPDATE %s SET mask=%d,alarm_date='%s' WHERE (system_type='%s' AND "
                "system_group='%s' AND system_name='%s' AND alarm_type=%d )",
                SFM_CURR_ALM_DB_TABLE_NAME,
                0,
                commlib_printDateTime(time(NULL)),
				"MP", "DSC", ( !devIndex? "SCMA":"SCMB"), 
                almType);

            if (fimd_mysql_query (query) < 0) {
                sprintf(trcBuf,"[fimd_mmc_umask_alm] fimd_mysql_query fail\n");
                trclib_writeLogErr (FL,trcBuf);
                sprintf(resBuf,"    RESULT = FAIL\n    REASON = MYSQL UPDATE FAIL\n");
                resCode = -1;

            } else {
				g_pstLogonRate->mask = SFM_ALM_NORMAL;
                sprintf(resBuf,"    RESULT = SUCCESS\n");
                resCode = 0;
            }
        }

    }
	
	// alarm mask 실패...
	else {
		sprintf(trcBuf,"[fimd_mmc_mask_alm] UNKNOWN TYPE (%s)\n", argType);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	if (resCode == 0) {
		if (strcasecmp (argUnit, ""))
			sprintf(tmpBuf,"\n    SYSTEM = %s\n    TYPE   = %s\n    UNIT   = %s\n\n", argSys, argType, argUnit);
		else
			sprintf(tmpBuf,"\n    SYSTEM = %s\n    TYPE   = %s\n\n", argSys, argType);
		strcat(resBuf, tmpBuf);	
	}
	fimd_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 1);

	/* TAP mask/umask-alm 수행시 sysIndex는 setting되지 않아 장애 발생 시킬수 있다. */

	if(almFlag == 1) 		/*TAP Alram*/
		fimd_updatePDAlmInfo (devIndex);	
	else if(almFlag == 2) 	/*SCE Alarm*/
		fimd_updateSceAlmInfo (devIndex);	
	else if(almFlag == 3){ 	/*L2SW Alarm*/
		fimd_updateL2SWlmInfo(devIndex);	
	}
	else  					/*system Alarm*/
		fimd_updateSysAlmInfo(sysIndex);

	/* shared memory backup */
	fimd_backupL3pd2File();
	fimd_backupSCE2File();
	fimd_backupL2sw2File(); //sjjeon
	fimd_backupLogon2File(); /* LOGON 성공율 */
	fimd_backupSMChSts2File(); /* SM Connection Status */
	fimd_broadcastAlmEvent2Client();
	return 1;
	
} //----- End of fimd_mmc_umask_alm -----//


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int fimd_mmc_dis_alm_lmt (IxpcQMsgType *rxIxpcMsg)
{
	int		i, sysIndex;
	char	argSysFlag=0, seqNo=1;

	MMLReqMsgType	*rxMmlReqMsg;
	rxMmlReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	for (i=0; i<strlen(rxMmlReqMsg->head.para[0].paraVal); i++)
			rxMmlReqMsg->head.para[0].paraVal[i] = toupper(rxMmlReqMsg->head.para[0].paraVal[i]);
	// 특정 시스템만 지정했는지 확인한다.
	if (!strcasecmp(rxMmlReqMsg->head.para[0].paraVal, ""))
		goto all;
	else if (strcasecmp(rxMmlReqMsg->head.para[0].paraVal, "ALL")){
		argSysFlag = 1;
#if 0
		// TAP 의 CPU/MEM 상태 감시 기능 배제 2009/10/14
		if( !strcasecmp(rxMmlReqMsg->head.para[0].paraVal,"TAPA") ||  
				!strcasecmp(rxMmlReqMsg->head.para[0].paraVal,"TAPB") ) {
			sysIndex = !strcasecmp(rxMmlReqMsg->head.para[0].paraVal,"TAPA") ? 0 : 1;
			if (sysIndex < 0) {
                sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN SYSTEM\n\n");
                fimd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo);
                return -1;
            }
#endif
       /* SCEA, SCEB 지정 sjjeon */
		if(!strcasecmp(rxMmlReqMsg->head.para[0].paraVal,"SCEA") || 
					!strcasecmp(rxMmlReqMsg->head.para[0].paraVal,"SCEB") ){
			sysIndex = !strcasecmp(rxMmlReqMsg->head.para[0].paraVal,"SCEA") ? 0 : 1;
			if (sysIndex < 0) {
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN SYSTEM\n\n");
				fimd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo);
				return -1;
			}			
		} else if(!strcasecmp(rxMmlReqMsg->head.para[0].paraVal,"L2SWA") || 
					!strcasecmp(rxMmlReqMsg->head.para[0].paraVal,"L2SWB") ){
			sysIndex = !strcasecmp(rxMmlReqMsg->head.para[0].paraVal,"L2SWA") ? 0 : 1;
			if (sysIndex < 0) {
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN SYSTEM\n\n");
				fimd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo);
				return -1;
			}			
		} else {
			if ((sysIndex = fimd_getSysIndexByName (rxMmlReqMsg->head.para[0].paraVal)) < 0) {
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN SYSTEM\n\n");
				fimd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo);
				return -1;
			}
		}
		// 결과 메시지에 대문자로 출력하기 위해 대문자로 바꾼다.
		strcat (resHead, "    RESULT = SUCCESS\n");
		sprintf(resHead, "    SYSTEM = %s\n", rxMmlReqMsg->head.para[0].paraVal);
	} else  {
		all:
		strcat (resHead, "    RESULT = SUCCESS\n");
		sprintf(resHead, "    SYSTEM = ALL\n");
	}
	strcat (resHead,"    ==================================================================\n");
	strcat (resHead,"                   MIN(%) MAJ(%) CRI(%)  MIN(sec) MAJ(sec) CRI(sec)\n");
	strcat (resHead,"    ==================================================================\n");

	strcpy (resBuf, resHead);
	/*특정 시스템만 지정..*/
	if (argSysFlag) {
#if 0
		// TAP 의 CPU/MEM 상태 감시 기능 배제 2009/10/14
		if( !strcasecmp(rxMmlReqMsg->head.para[0].paraVal,"TAPA") || !strcasecmp(rxMmlReqMsg->head.para[0].paraVal,"TAPB") )
			fimd_mmc_makePdAlmClsOutputMsg (rxIxpcMsg, sysIndex, &seqNo, 0);
#endif
		if( !strcasecmp(rxMmlReqMsg->head.para[0].paraVal,"SCEA") || !strcasecmp(rxMmlReqMsg->head.para[0].paraVal,"SCEB") )
			fimd_mmc_makeSceAlmClsOutputMsg(rxIxpcMsg, sysIndex, &seqNo, 0);
		else if( !strcasecmp(rxMmlReqMsg->head.para[0].paraVal,"L2SWA") || !strcasecmp(rxMmlReqMsg->head.para[0].paraVal,"L2SWB"))
			fimd_mmc_makeL2swAlmClsOutputMsg(rxIxpcMsg, sysIndex, &seqNo, 0);
		else
			fimd_mmc_makeAlmClsOutputMsg (rxIxpcMsg, sysIndex, &seqNo, 0);
	} else {
	/* ALL System */
		for (i=0; i<SYSCONF_MAX_ASSO_SYS_NUM; i++) {
			if (strcasecmp(sfdb->sys[i].commInfo.name, ""))
				fimd_mmc_makeAlmClsOutputMsg (rxIxpcMsg, i, &seqNo, 1);
		}
#if 0
		// TAP 의 CPU/MEM 상태 감시 기능 배제 2009/10/14
		for (i=0; i<2; i++ )
			fimd_mmc_makePdAlmClsOutputMsg (rxIxpcMsg, i, &seqNo, 1);
#endif
		/* SCEA, SCEB sjjeon */
		for (i=0; i<MAX_SCE_DEV_NUM; i++ )
			fimd_mmc_makeSceAlmClsOutputMsg (rxIxpcMsg, i, &seqNo, 1);
		for (i=0; i<MAX_L2_DEV_NUM; i++ )
			fimd_mmc_makeL2swAlmClsOutputMsg (rxIxpcMsg, i, &seqNo, 1);
		
	}


	fimd_txMMLResult (rxIxpcMsg, resBuf, 0, 0, 0, 0, seqNo);

	return 1;
	
} //----- End of fimd_mmc_dis_alm_lmt -----//

/* hjjung */
int fimd_mmc_dis_session_lmt (IxpcQMsgType *rxIxpcMsg)
{
	int		i, sysIndex;
	char	argSysFlag=0, seqNo=1;

	MMLReqMsgType	*rxMmlReqMsg;
	rxMmlReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	for (i=0; i<strlen(rxMmlReqMsg->head.para[0].paraVal); i++)
			rxMmlReqMsg->head.para[0].paraVal[i] = toupper(rxMmlReqMsg->head.para[0].paraVal[i]);
	// 특정 시스템만 지정했는지 확인한다.
	if (!strcasecmp(rxMmlReqMsg->head.para[0].paraVal, ""))
		goto all;
	else if (strcasecmp(rxMmlReqMsg->head.para[0].paraVal, "ALL")){
		argSysFlag = 1;
       /* SCEA, SCEB 지정 sjjeon */
		if(!strcasecmp(rxMmlReqMsg->head.para[0].paraVal,"SCEA") || 
					!strcasecmp(rxMmlReqMsg->head.para[0].paraVal,"SCEB") ){
			sysIndex = !strcasecmp(rxMmlReqMsg->head.para[0].paraVal,"SCEA") ? 0 : 1;
			if (sysIndex < 0) {
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN SYSTEM\n\n");
				fimd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo);
				return -1;
			}			
		} else if(!strcasecmp(rxMmlReqMsg->head.para[0].paraVal,"SCMA") || 
					!strcasecmp(rxMmlReqMsg->head.para[0].paraVal,"SCMB") ){
			sysIndex = !strcasecmp(rxMmlReqMsg->head.para[0].paraVal,"SCMA") ? 0 : 1;
			if (sysIndex < 0) {
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN SYSTEM\n\n");
				fimd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo);
				return -1;
			}			
		} else {
			if ((sysIndex = fimd_getSysIndexByName (rxMmlReqMsg->head.para[0].paraVal)) < 0) {
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN SYSTEM\n\n");
				fimd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo);
				return -1;
			}
		}
		// 결과 메시지에 대문자로 출력하기 위해 대문자로 바꾼다.
		strcat (resHead, "    RESULT = SUCCESS\n");
		sprintf(resHead, "    SYSTEM = %s\n", rxMmlReqMsg->head.para[0].paraVal);
	} else  {
		all:
		strcat (resHead, "    RESULT = SUCCESS\n");
		sprintf(resHead, "    SYSTEM = ALL\n");
	}
	strcat (resHead,"    ==================================================================\n");
	strcat (resHead,"                    MIN    MAJ    CRI      MIN(sec) MAJ(sec) CRI(sec)\n");
	strcat (resHead,"    ==================================================================\n");

	strcpy (resBuf, resHead);
	/*특정 시스템만 지정..*/

	if (argSysFlag) {
		if( !strcasecmp(rxMmlReqMsg->head.para[0].paraVal,"SCEA") || !strcasecmp(rxMmlReqMsg->head.para[0].paraVal,"SCEB") )
			fimd_mmc_makeSceSessionAlmClsOutputMsg(rxIxpcMsg, sysIndex, &seqNo, 0);
		else if( !strcasecmp(rxMmlReqMsg->head.para[0].paraVal,"SCMA") || !strcasecmp(rxMmlReqMsg->head.para[0].paraVal,"SCMB") ) {
			fimd_mmc_makeLegAlmClsOutputMsg(rxIxpcMsg, sysIndex, &seqNo, 0);
		}
	} else {
	/* ALL System */
		/* SCEA, SCEB sjjeon */
		for (i=0; i<MAX_SCE_DEV_NUM; i++ )
			fimd_mmc_makeSceSessionAlmClsOutputMsg (rxIxpcMsg, i, &seqNo, 1);
		for (i=0; i<MAX_CALL_DEV_NUM; i++ )
			fimd_mmc_makeLegAlmClsOutputMsg (rxIxpcMsg, i, &seqNo, 1);
	}

	fimd_txMMLResult (rxIxpcMsg, resBuf, 0, 0, 0, 0, seqNo);

	return 1;
	
} //----- End of fimd_mmc_dis_session_lmt -----//

/* added by dcham 20110525 for TPS MML Command */
int fimd_mmc_dis_tps_lmt (IxpcQMsgType *rxIxpcMsg)
{
	int		i, sysIndex;
	char	argSysFlag=0, seqNo=1;

	MMLReqMsgType	*rxMmlReqMsg;
	rxMmlReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	for (i=0; i<strlen(rxMmlReqMsg->head.para[0].paraVal); i++)
			rxMmlReqMsg->head.para[0].paraVal[i] = toupper(rxMmlReqMsg->head.para[0].paraVal[i]);

	if (!strcasecmp(rxMmlReqMsg->head.para[0].paraVal, "") ||
		!strcasecmp(rxMmlReqMsg->head.para[0].paraVal, "ALL")){ // ALL
		strcat (resHead, "    RESULT = SUCCESS\n");
		sprintf(resHead, "    SYSTEM = ALL\n");
	}
	else{ // optional
		argSysFlag = 1;
		if(!strcasecmp(rxMmlReqMsg->head.para[0].paraVal,"SCMA") || 
	       !strcasecmp(rxMmlReqMsg->head.para[0].paraVal,"SCMB") ){
	       	sysIndex = !strcasecmp(rxMmlReqMsg->head.para[0].paraVal,"SCMA") ? 0 : 1;
			if (sysIndex < 0) {
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN SYSTEM\n\n");
				fimd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo);
				return -1;
			}			
		} else {
			if ((sysIndex = fimd_getSysIndexByName (rxMmlReqMsg->head.para[0].paraVal)) < 0) {
				sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN SYSTEM\n\n");
				fimd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo);
				return -1;
			}
		}
		// 결과 메시지에 대문자로 출력하기 위해 대문자로 바꾼다.
		strcat (resHead, "    RESULT = SUCCESS\n");
		sprintf(resHead, "    SYSTEM = %s\n", rxMmlReqMsg->head.para[0].paraVal);
	}

	strcat (resHead,"    ==================================================================\n");
	strcat (resHead,"                    MIN    MAJ    CRI     MIN(sec) MAJ(sec) CRI(sec)\n");
	strcat (resHead,"    ==================================================================\n");

	strcpy (resBuf, resHead);

	if (argSysFlag) // optional
		fimd_mmc_makeCallAlmClsOutputMsg(rxIxpcMsg, sysIndex, &seqNo, 0);
	else{
		for (i=0; i<MAX_CALL_DEV_NUM; i++ ) // ALL
			fimd_mmc_makeCallAlmClsOutputMsg (rxIxpcMsg, i, &seqNo, 1);
	}

	fimd_txMMLResult (rxIxpcMsg, resBuf, 0, 0, 0, 0, seqNo);

	return 1;

} //----- End of fimd_mmc_dis_tps_lmt -----//

int fimd_mmc_dis_svc_alm (IxpcQMsgType *rxIxpcMsg)
{
	int		i, totalCnt=0, sysIndex;
	char	argSysFlag=0, seqNo=1;
	
	char    cntBuf[256];
	char	rcBuf[256], trBuf[256], trHead[256];
	char	name[5];
	MMLReqMsgType	*rxMmlReqMsg;

	rxMmlReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	memset(cntBuf, 0x00, sizeof(cntBuf));
	memset(resBuf, 0x00, sizeof(resBuf));
	memset(resHead, 0x00, sizeof(resHead));
	memset(trBuf, 0x00, sizeof(trBuf));
	memset(trHead, 0x00, sizeof(trHead));
	// 특정 시스템만 지정했는지 확인한다.
	if (strcasecmp(rxMmlReqMsg->head.para[0].paraVal, ""))
	{
		argSysFlag = 1;
		if(!strcasecmp(rxMmlReqMsg->head.para[0].paraVal, "ALL")) {
				
		}else if ((sysIndex = fimd_getSysIndexByName (rxMmlReqMsg->head.para[0].paraVal)) < 0) {
			sprintf(resBuf,"    RESULT = FAIL\n      REASON = UNKNOWN SYSTEM\n\n");
			fimd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo);
			return -1;
		}
		// 결과 메시지에 대문자로 출력하기 위해 대문자로 바꾼다.
		for (i=0; i<strlen(rxMmlReqMsg->head.para[0].paraVal); i++)
			rxMmlReqMsg->head.para[0].paraVal[i] = toupper(rxMmlReqMsg->head.para[0].paraVal[i]);
	}

	strcpy(name, rxMmlReqMsg->head.para[0].paraVal);
	
	if(strcasecmp(name, "ALL")){
		sprintf(rcBuf, "    RESULT = SUCCESS\n");
		sprintf(resHead,"    SYSTEM = %s\n", rxMmlReqMsg->head.para[0].paraVal);
		strcat (resHead,"    ================================================================\n");
		strcat (resHead,"      PROTOCOL_TYPE     MINIMUM_TRANSACTION_COUNT     SUCCESS_RATE  \n");
		strcat (resHead,"    ================================================================\n");
		strcat (rcBuf, resHead);
		strcpy (resBuf, rcBuf);
		if (argSysFlag) {
			fimd_mmc_makeAlmRateOutputMsg (rxIxpcMsg, sysIndex, &seqNo);
		} else {
			memset(resBuf, 0x00, sizeof(resBuf));
			goto all;

		}
		sprintf(cntBuf,"       TOTAL COUNT = %d \n", SFM_REAL_SUCC_RATE_CNT);
		strcat(resBuf, cntBuf);
		strcat (resBuf,"    ================================================================\n");

	}
	else if(!strcasecmp(name, "ALL")){
		all:
		totalCnt = (SFM_REAL_SUCC_RATE_CNT)*2;	
		sprintf(trBuf, "    RESULT = SUCCESS\n");
		strcat(trBuf,"    SYSTEM = ALL\n");
		strcat (trHead,"    ==========================================================================\n");
		strcat (trHead,"                PROTOCOL_TYPE     MINIMUM_TRANSACTION_COUNT     SUCCESS_RATE  \n");
		strcat (trHead,"    ==========================================================================\n");
		strcat (trBuf, trHead);
		strcat (resBuf, trBuf);
		for(i=1; i<3; i++){
			fimd_mmc_makeAlmRateAllOutputMsg (rxIxpcMsg, i, &seqNo);
		}
		sprintf(cntBuf,"       TOTAL COUNT = %d \n", totalCnt);
		strcat(resBuf, cntBuf);
		strcat(resBuf,"    ==========================================================================\n");
		strcat(resBuf, "\n");
	}

	fimd_txMMLResult (rxIxpcMsg, resBuf, 0, 0, 0, 0, seqNo);
		
	return 1;
	
} //----- End of fimd_mmc_dis_svc_alm -----//

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void fimd_mmc_makeAlmClsOutputMsg (
			IxpcQMsgType *rxIxpcMsg,
			int sysIndex,
			char *seqNo, int all
			)
{

	char	tmpBuf[256], trcBuf[25];

	// 시스템 공통 정보
	//
	if(all == 1) {
		sprintf(trcBuf, "    SYSTEM = [%s]\n", sfdb->sys[sysIndex].commInfo.name);
		strcat(resBuf, trcBuf);
    }
	sprintf(tmpBuf,"    CPU          :  %-6d %-6d %-6d   %-8d %-8d %-8d\n",
			sfdb->sys[sysIndex].commInfo.cpuInfo.minLimit,
			sfdb->sys[sysIndex].commInfo.cpuInfo.majLimit,
			sfdb->sys[sysIndex].commInfo.cpuInfo.criLimit,
			sfdb->sys[sysIndex].commInfo.cpuInfo.minDurat,
			sfdb->sys[sysIndex].commInfo.cpuInfo.majDurat,
			sfdb->sys[sysIndex].commInfo.cpuInfo.criDurat);
	strcat (resBuf,tmpBuf);
	sprintf(tmpBuf,"    MEM          :  %-6d %-6d %-6d   %-8d %-8d %-8d\n",
			sfdb->sys[sysIndex].commInfo.memInfo.minLimit,
			sfdb->sys[sysIndex].commInfo.memInfo.majLimit,
			sfdb->sys[sysIndex].commInfo.memInfo.criLimit,
			sfdb->sys[sysIndex].commInfo.memInfo.minDurat,
			sfdb->sys[sysIndex].commInfo.memInfo.majDurat,
			sfdb->sys[sysIndex].commInfo.memInfo.criDurat);
		strcat (resBuf,tmpBuf);

	// by helca 08.07 disk, queue의 각 partition을 하나로 관리 하므로 partition 마다 같은 값을 가진다. 
	sprintf(tmpBuf,"    DISK         :  %-6d %-6d %-6d\n",
				sfdb->sys[sysIndex].commInfo.diskInfo[0].minLimit,
				sfdb->sys[sysIndex].commInfo.diskInfo[0].majLimit,
				sfdb->sys[sysIndex].commInfo.diskInfo[0].criLimit);
	strcat (resBuf,tmpBuf);
	sprintf(tmpBuf,"    QUEUE        :  %-6d %-6d %-6d\n",
				sfdb->sys[sysIndex].commInfo.queInfo[0].minLimit,
				sfdb->sys[sysIndex].commInfo.queInfo[0].majLimit,
				sfdb->sys[sysIndex].commInfo.queInfo[0].criLimit);
	strcat (resBuf,tmpBuf);
	// RSRC by helca 08.02
	// SFM_SysRSRCInfo 전체 Limit 설정값이 동일 하다..
	/*
	   // session 값은 사용하지 않는다.  sjjeon
	if(sysIndex == 0){
	}else{
		sprintf(tmpBuf,"    SESS         :  %-6d %-6d %-6d   %-8d %-8d %-8d\n",
			sfdb->sys[sysIndex].commInfo.rsrcSts[0].minLimit,
			sfdb->sys[sysIndex].commInfo.rsrcSts[0].majLimit,
			sfdb->sys[sysIndex].commInfo.rsrcSts[0].criLimit,
			sfdb->sys[sysIndex].commInfo.rsrcSts[0].minDurat,
			sfdb->sys[sysIndex].commInfo.rsrcSts[0].majDurat,
			sfdb->sys[sysIndex].commInfo.rsrcSts[0].criDurat);
		strcat (resBuf,tmpBuf);
	}
	*/
	/*
	* LOGON 성공율 감시 threshold
	* added by uamyd 20110210
	*/
	if( sysIndex ){ // SCMA(1), SCMB(2)
		sprintf(tmpBuf,"    LOGON-SUCC   :  %-6d %-6d %-6d\n",
					g_stLogonRate[0][sysIndex-1].minLimit,
					g_stLogonRate[0][sysIndex-1].majLimit,
					g_stLogonRate[0][sysIndex-1].criLimit);
		strcat (resBuf,tmpBuf);

		sprintf(tmpBuf,"    LOGOUT-SUCC  :  %-6d %-6d %-6d\n",
					g_stLogonRate[1][sysIndex-1].minLimit,
					g_stLogonRate[1][sysIndex-1].majLimit,
					g_stLogonRate[1][sysIndex-1].criLimit);
		strcat (resBuf,tmpBuf);
	}

	/** SM Connection Status */ //작동 하지 않음
	if( 0 ){ // SCMA(1), SCMB(2)
		SFM_SMChInfo *pstInfo = &sfdb->sys[sysIndex].commInfo.smChSts;
		strcat (resBuf,"    ==================================================================\n");
		strcat (resBuf,"                   MIN    MAJ    CRI\n");
		strcat (resBuf,"    ==================================================================\n");
		sprintf(tmpBuf,"    SM CONNECTION:  %-6d %-6d %-6d\n",
					pstInfo->minLimit,
					pstInfo->majLimit,
					pstInfo->criLimit);

		strcat (resBuf, tmpBuf);
	}

	strcat (resBuf,"    ==================================================================\n");

	if (strlen(resBuf) > 3000) { // 3000 byte이상이면 mmcd로 결과메시지를 보낸다.
		strcat (resBuf,"\n");
		fimd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, 5, 1, (*seqNo)++);
		strcpy (resBuf, resHead);
		commlib_microSleep(50000); // 50 MS
	}
	
	memset(trcBuf, 0x00, sizeof(trcBuf));
	return;

} //----- End of fimd_mmc_makeAlmClsOutputMsg -----//

/* by helca */

//------------------------------------------------------------------------------
void fimd_mmc_makeAlmRateOutputMsg (
			IxpcQMsgType *rxIxpcMsg,
			int sysIndex,
			char *seqNo
			)
{

	char	tmpBuf[256];

	memset(tmpBuf, 0x00, sizeof(tmpBuf));
	sprintf(tmpBuf,"      %-5s             %-10d                    %-10d       \n",
			sfdb->sys[sysIndex].commInfo.succRate[0].name,
			sfdb->sys[sysIndex].commInfo.succRate[0].cnt,
			sfdb->sys[sysIndex].commInfo.succRate[0].rate
			);
	strcat (resBuf,tmpBuf);
	sprintf(tmpBuf,"      %-5s             %-10d                    %-10d       \n",
			sfdb->sys[sysIndex].commInfo.succRate[1].name,
			sfdb->sys[sysIndex].commInfo.succRate[1].cnt,
			sfdb->sys[sysIndex].commInfo.succRate[1].rate
			);
	strcat (resBuf,tmpBuf);
	sprintf(tmpBuf,"      %-5s             %-10d                    %-10d       \n",
			sfdb->sys[sysIndex].commInfo.succRate[2].name,
			sfdb->sys[sysIndex].commInfo.succRate[2].cnt,
			sfdb->sys[sysIndex].commInfo.succRate[2].rate
			);
	strcat (resBuf,tmpBuf);
	sprintf(tmpBuf,"      %-5s             %-10d                    %-10d       \n",
			sfdb->sys[sysIndex].commInfo.succRate[3].name,
			sfdb->sys[sysIndex].commInfo.succRate[3].cnt,
			sfdb->sys[sysIndex].commInfo.succRate[3].rate
			);
	strcat (resBuf,tmpBuf);
	sprintf(tmpBuf,"      %-5s             %-10d                    %-10d       \n",
			sfdb->sys[sysIndex].commInfo.succRate[4].name,
			sfdb->sys[sysIndex].commInfo.succRate[4].cnt,
			sfdb->sys[sysIndex].commInfo.succRate[4].rate
			);
	strcat (resBuf,tmpBuf);
	sprintf(tmpBuf,"      %-5s             %-10d                    %-10d       \n",
			sfdb->sys[sysIndex].commInfo.succRate[5].name,
			sfdb->sys[sysIndex].commInfo.succRate[5].cnt,
			sfdb->sys[sysIndex].commInfo.succRate[5].rate
			);
	strcat (resBuf,tmpBuf);
	sprintf(tmpBuf,"      %-5s             %-10d                    %-10d       \n",
			sfdb->sys[sysIndex].commInfo.succRate[6].name,
			sfdb->sys[sysIndex].commInfo.succRate[6].cnt,
			sfdb->sys[sysIndex].commInfo.succRate[6].rate
			);
	strcat (resBuf,tmpBuf);
	sprintf(tmpBuf,"      %-5s             %-10d                    %-10d       \n",
                        sfdb->sys[sysIndex].commInfo.succRate[7].name,
                        sfdb->sys[sysIndex].commInfo.succRate[7].cnt,
                        sfdb->sys[sysIndex].commInfo.succRate[7].rate
                        );
        strcat (resBuf,tmpBuf);	

	strcat (resBuf,"    ----------------------------------------------------------------\n");
	
	memset(tmpBuf, 0x00, sizeof(tmpBuf));
	
	if (strlen(resBuf) > 3000) { // 3000 byte이상이면 mmcd로 결과메시지를 보낸다.
		strcat (resBuf,"\n");
		fimd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, 5, 1, (*seqNo)++);
		strcpy (resBuf, resHead);
		commlib_microSleep(50000); // 50 MS
	}

	return;

} //----- End of fimd_mmc_makeAlmRateOutputMsg -----//

void fimd_mmc_makeAlmRateAllOutputMsg (
			IxpcQMsgType *rxIxpcMsg,
			int sysIndex,
			char *seqNo
			)
{

	char	tmpBuf[256], trcBuf[256];

	memset(tmpBuf, 0x00, sizeof(tmpBuf));
	sprintf(trcBuf, "     [%s] \n", sfdb->sys[sysIndex].commInfo.name);
	strcat (resBuf,trcBuf);
	sprintf(tmpBuf,"                %-5s             %-10d                    %-10d \n",
			sfdb->sys[sysIndex].commInfo.succRate[0].name,
			sfdb->sys[sysIndex].commInfo.succRate[0].cnt,
			sfdb->sys[sysIndex].commInfo.succRate[0].rate
			);
	strcat (resBuf,tmpBuf);
	sprintf(tmpBuf,"                %-5s             %-10d                    %-10d \n",
			sfdb->sys[sysIndex].commInfo.succRate[1].name,
			sfdb->sys[sysIndex].commInfo.succRate[1].cnt,
			sfdb->sys[sysIndex].commInfo.succRate[1].rate
			);
	strcat (resBuf,tmpBuf);
	sprintf(tmpBuf,"                %-5s             %-10d                    %-10d \n",
			sfdb->sys[sysIndex].commInfo.succRate[2].name,
			sfdb->sys[sysIndex].commInfo.succRate[2].cnt,
			sfdb->sys[sysIndex].commInfo.succRate[2].rate
			);
	strcat (resBuf,tmpBuf);
	sprintf(tmpBuf,"                %-5s             %-10d                    %-10d \n",
			sfdb->sys[sysIndex].commInfo.succRate[3].name,
			sfdb->sys[sysIndex].commInfo.succRate[3].cnt,
			sfdb->sys[sysIndex].commInfo.succRate[3].rate
			);
	strcat (resBuf,tmpBuf);
	sprintf(tmpBuf,"                %-5s             %-10d                    %-10d \n",
			sfdb->sys[sysIndex].commInfo.succRate[4].name,
			sfdb->sys[sysIndex].commInfo.succRate[4].cnt,
			sfdb->sys[sysIndex].commInfo.succRate[4].rate
			);
	strcat (resBuf,tmpBuf);
	sprintf(tmpBuf,"                %-5s             %-10d                    %-10d \n",
			sfdb->sys[sysIndex].commInfo.succRate[5].name,
			sfdb->sys[sysIndex].commInfo.succRate[5].cnt,
			sfdb->sys[sysIndex].commInfo.succRate[5].rate
			);
	strcat (resBuf,tmpBuf);
	sprintf(tmpBuf,"                %-5s             %-10d                    %-10d \n",
			sfdb->sys[sysIndex].commInfo.succRate[6].name,
			sfdb->sys[sysIndex].commInfo.succRate[6].cnt,
			sfdb->sys[sysIndex].commInfo.succRate[6].rate
			);
	strcat (resBuf,tmpBuf);
	sprintf(tmpBuf,"                %-5s             %-10d                    %-10d \n",
                        sfdb->sys[sysIndex].commInfo.succRate[7].name,
                        sfdb->sys[sysIndex].commInfo.succRate[7].cnt,
                        sfdb->sys[sysIndex].commInfo.succRate[7].rate
                        );
        strcat (resBuf,tmpBuf);	
	strcat (resBuf,"    --------------------------------------------------------------------------\n");
	
	memset(tmpBuf, 0x00, sizeof(tmpBuf));
	
	if (strlen(resBuf) > 3000) { // 3000 byte이상이면 mmcd로 결과메시지를 보낸다.
		strcat (resBuf,"\n");
		fimd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, 5, 1, (*seqNo)++);
		strcpy (resBuf, resHead);
		commlib_microSleep(50000); // 50 MS
	}

	return;

} //----- End of fimd_mmc_makeAlmRateAllOutputMsg -----//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int fimd_mmc_set_alm_lmt (IxpcQMsgType *rxIxpcMsg)
{
	int	    i, k, sysIndex=0, type, level, limit, durat=0, devIndex;
	char	argSysFlag=0, argDurFlag=0;
	char	argSys[32], argType[32], argLevel[32], argLimit[32], argDurat[10];
	char	tmpBuf[256];
		
	MMLReqMsgType	*rxMmlReqMsg;

	rxMmlReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
	// get input parameters
	//
	if (!strcasecmp(rxMmlReqMsg->head.para[0].paraName, "SYS")) {
		argSysFlag = 1;
		strcpy (argSys, rxMmlReqMsg->head.para[0].paraVal);
		for (i=0; i<strlen(argSys); i++) argSys[i] = toupper(argSys[i]);

		strcpy (argType, rxMmlReqMsg->head.para[1].paraVal);
		for (i=0; i<strlen(argType); i++) argType[i] = toupper(argType[i]);

		strcpy (argLevel, rxMmlReqMsg->head.para[2].paraVal);
		for (i=0; i<strlen(argLevel); i++) argLevel[i] = toupper(argLevel[i]);

		strcpy (argLimit, rxMmlReqMsg->head.para[3].paraVal);
		limit = atoi(argLimit);

		if (strcasecmp(rxMmlReqMsg->head.para[4].paraVal, "")) {
			argDurFlag = 1;
			strcpy (argDurat, rxMmlReqMsg->head.para[4].paraVal);
			durat = atoi(argDurat);
		} else {
			strcpy (argDurat, "");
		}
	} else {
		strcpy (argSys, "");

		strcpy (argType, rxMmlReqMsg->head.para[0].paraVal);
		for (i=0; i<strlen(argType); i++) argType[i] = toupper(argType[i]);

		strcpy (argLevel, rxMmlReqMsg->head.para[1].paraVal);
		for (i=0; i<strlen(argLevel); i++) argLevel[i] = toupper(argLevel[i]);

		strcpy (argLimit, rxMmlReqMsg->head.para[2].paraVal);
		limit = atoi(argLimit);

		if (strcasecmp(rxMmlReqMsg->head.para[3].paraVal, "")) {
			argDurFlag = 1;
			strcpy (argDurat, rxMmlReqMsg->head.para[3].paraVal);
			durat = atoi(argDurat);
		} else {
			strcpy (argDurat, "");
		}
	}

	//
	// - optional parameter의 입력 여부를 확인한다.
	// - 입력된 value를 꺼낸다.
	//
#if 0
	// TAP 의 CPU/MEM 상태 감시 기능 배제 2009/10/14
	if(!strcasecmp(argSys, "TAPA")) devIndex = 0;
	else if (!strcasecmp(argSys, "TAPB")) devIndex = 1;
#endif
	if (!strcasecmp(argSys, "SCEA")) devIndex = 0;
	else if (!strcasecmp(argSys, "SCEB")) devIndex = 1;
	else if (!strcasecmp(argSys, "L2SWA")) devIndex = 0;
	else if (!strcasecmp(argSys, "L2SWB")) devIndex = 1;
	else if (strcasecmp(argSys, ""))	 {
		if ((sysIndex = fimd_getSysIndexByName (argSys)) < 0) {
			sprintf(resBuf,"    RESULT = FAIL\n      REASON = UNKNOWN SYSTEM\n\n");
			fimd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, 1);
			return -1;
		}
	}

#if 0
	// TAP 의 CPU/MEM 상태 감시 기능 배제 2009/10/14
	if( !strcasecmp(argSys, "TAPA") || !strcasecmp(argSys, "TAPB"))
		return fimd_mmc_pd_set_alm_lmt( rxIxpcMsg, argSys, argType, argLevel,limit,durat,devIndex);
#endif
	if( !strcasecmp(argSys, "SCEA") || !strcasecmp(argSys, "SCEB")){
		return fimd_mmc_sce_set_alm_lmt( rxIxpcMsg, argSys, argType, argLevel,limit,durat,devIndex);
	}else if( !strcasecmp(argSys, "L2SWA") || !strcasecmp(argSys, "L2SWB") ){
		return fimd_mmc_l2sw_set_alm_lmt( rxIxpcMsg, argSys, argType, argLevel,limit,durat,devIndex);
	}

	if (!strcasecmp(argType, "CPU")) {
		type = SFM_ALM_TYPE_CPU_USAGE;
	} else if (!strcasecmp(argType, "MEM")) {
		type = SFM_ALM_TYPE_MEMORY_USAGE;
	} else if (!strcasecmp(argType, "QUEUE")) {
		type = SFM_ALM_TYPE_QUEUE_LOAD;
	} else if (!strcasecmp(argType, "SESS")) { // by helca 08.02 RSRC->SESS 명칭 변경
		type = SFM_ALM_TYPE_SESS_LOAD;
	} else if (!strcasecmp(argType, "DISK")){ // by helca 08.07
		type = SFM_ALM_TYPE_DISK_USAGE;
	} else if (!strcasecmp(argType, "LOGON-SUCC")){ // by uamyd 20110210
		type = SFM_ALM_TYPE_LOGON_SUCCESS_RATE;
	} else if (!strcasecmp(argType, "LOGOUT-SUCC")){ // by uamyd 20110424
		type = SFM_ALM_TYPE_LOGOUT_SUCCESS_RATE;
	} else if (!strcasecmp(argType, "SMLINK")){ // by uamyd 20110424
		type = SFM_ALM_TYPE_SM_CONN_STS;
	}

	if (!strcasecmp(argLevel, "MINOR")) {
		level = SFM_ALM_MINOR;
	} else if (!strcasecmp(argLevel, "MAJOR")) {
		level = SFM_ALM_MAJOR;
	} else if (!strcasecmp(argLevel, "CRITICAL")) {
		level = SFM_ALM_CRITICAL;
	}

	//
	// 결과 출력시 echo 출력을 위해 입력된 내용을 결과 메시지에 표시한다.
	//

	sprintf(resBuf,"    RESULT = SUCCESS\n");

	if (argSysFlag) {
		sprintf(tmpBuf,"      SYSTEM = %s\n", argSys);
		strcat (resBuf,tmpBuf);
	}
	sprintf(tmpBuf,"      TYPE   = %s\n", argType);
	strcat (resBuf,tmpBuf);
	sprintf(tmpBuf,"      LEVEL  = %s\n", argLevel);
	strcat (resBuf,tmpBuf);
	sprintf(tmpBuf,"      LIMIT  = %d\n", limit);
	strcat (resBuf,tmpBuf);
	if (argDurFlag) {
		sprintf(tmpBuf,"      DURAT  = %d\n", durat);
		strcat (resBuf,tmpBuf);
	}
	strcat(resBuf, "\n");

	//
	// 입력된 limit값이 유효한지 확인한다.
	// - minor limit은 major보다 작아야 하고, major는 minor보다 크고 critical 보다
	//	작아야 하고, critical은 major보다 작아야 한다.
	// - 시스템을 지정하지 않은 경우 모든 시스템에서 유효하지 않은 놈이 하나라도 있으면
	//	error처리한다.
	//
	if (argSysFlag) {
		switch(type){
			case SFM_ALM_TYPE_CPU_USAGE:
				if (fimd_mmc_checkAlmLimitValidation (type, level, limit,
					(void*)&sfdb->sys[sysIndex].commInfo.cpuInfo) < 0) {
						fimd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, 1);
						return -1;
				}
				break;
			case SFM_ALM_TYPE_MEMORY_USAGE:
				if (fimd_mmc_checkAlmLimitValidation (type, level, limit,
                   (void*)&sfdb->sys[sysIndex].commInfo.memInfo) < 0) {
                   		fimd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, 1);
                       	return -1;
                   }
				break;
			case SFM_ALM_TYPE_DISK_USAGE:
				if (fimd_mmc_checkAlmLimitValidation (type, level, limit,
               		(void*)&sfdb->sys[sysIndex].commInfo.diskInfo[0]) < 0) {
                   		fimd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, 1);
                       	return -1;
				}
				break;
			case SFM_ALM_TYPE_QUEUE_LOAD:
				if (fimd_mmc_checkAlmLimitValidation (type, level, limit,
               		(void*)&sfdb->sys[sysIndex].commInfo.queInfo[0]) < 0) {
                   		fimd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, 1);
                       	return -1;
                }
                break;
			case SFM_ALM_TYPE_SESS_LOAD:
				if (fimd_mmc_checkAlmLimitValidation (type, level, limit,
                	(void*)&sfdb->sys[sysIndex].commInfo.rsrcSts[0]) < 0) {
                   		fimd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, 1);
                       	return -1;
                }
                break;
			case SFM_ALM_TYPE_LOGON_SUCCESS_RATE:
				if (fimd_mmc_checkLogonSuccessRateAlmLimitValidation( sysIndex, 0, level, limit ) < 0 ) {
					fimd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, 1);
					return -1;
				}
				break;
			case SFM_ALM_TYPE_LOGOUT_SUCCESS_RATE:
				if (fimd_mmc_checkLogonSuccessRateAlmLimitValidation( sysIndex, 1, level, limit ) < 0 ) {
					fimd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, 1);
					return -1;
				}
				break;
			case SFM_ALM_TYPE_SM_CONN_STS:
				if (fimd_mmc_checkSMChAlmLimitValidation( sysIndex, level, limit ) < 0 ) {
					fimd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, 1);
					return -1;
				}
				break;
		}
	} else {
		switch(type){
			case SFM_ALM_TYPE_CPU_USAGE:
				for (i=0; i<eqSysCnt; i++) {
					if (fimd_mmc_checkAlmLimitValidation (type, level, limit,
								(void*)&sfdb->sys[i].commInfo.cpuInfo) < 0) {
						fimd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, 1);
						return -1;
					}
				}
				break;
			case SFM_ALM_TYPE_MEMORY_USAGE:
				for (i=0; i<eqSysCnt; i++) {
					if (fimd_mmc_checkAlmLimitValidation (type, level, limit,
								(void*)&sfdb->sys[i].commInfo.memInfo) < 0) {
						fimd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, 1);
						return -1;
					}
				}
				break;
			case SFM_ALM_TYPE_DISK_USAGE:
				for (i=0; i<eqSysCnt; i++) {
					if (fimd_mmc_checkAlmLimitValidation (type, level, limit,
								(void*)&sfdb->sys[i].commInfo.diskInfo[0]) < 0) {
						fimd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, 1);
						return -1;
					}
				}
				break;
			case SFM_ALM_TYPE_QUEUE_LOAD:
				for (i=0; i<eqSysCnt; i++) {
					if (fimd_mmc_checkAlmLimitValidation (type, level, limit,
								(void*)&sfdb->sys[i].commInfo.queInfo[0]) < 0) {
						fimd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, 1);
						return -1;
					}
				}
				break;
			case SFM_ALM_TYPE_SESS_LOAD:
				for (i=0; i<eqSysCnt; i++) {
					if (fimd_mmc_checkAlmLimitValidation (type, level, limit,
								(void*)&sfdb->sys[i].commInfo.rsrcSts[0]) < 0) {
						fimd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, 1);
						return -1;
					}
				}
				break;	
			case SFM_ALM_TYPE_LOGON_SUCCESS_RATE:
				if (fimd_mmc_checkLogonSuccessRateAlmLimitValidation( sysIndex, 0, level, limit ) < 0 ) {
					fimd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, 1);
					return -1;
				}
				break;
			case SFM_ALM_TYPE_LOGOUT_SUCCESS_RATE:
				if (fimd_mmc_checkLogonSuccessRateAlmLimitValidation( sysIndex, 1, level, limit ) < 0 ) {
					fimd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, 1);
					return -1;
				}
				break;
			case SFM_ALM_TYPE_SM_CONN_STS:
				if (fimd_mmc_checkSMChAlmLimitValidation( sysIndex, level, limit ) < 0 ) {
					fimd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, 1);
					return -1;
				}
				break;
		}
	}

	//
	// 해당 값으로 update한다.
	//
	if (argSysFlag) {
		switch(type){
			case SFM_ALM_TYPE_CPU_USAGE:
				fimd_mmc_updateAlmClsValue (type, level, limit, durat,
						(void*)&sfdb->sys[sysIndex].commInfo.cpuInfo);
				break;
			case SFM_ALM_TYPE_MEMORY_USAGE:
				fimd_mmc_updateAlmClsValue (type, level, limit, durat,
						(void*)&sfdb->sys[sysIndex].commInfo.memInfo);
				break;
			case SFM_ALM_TYPE_DISK_USAGE:
				for(k=0; k<SFM_MAX_DISK_CNT; k++){		
					fimd_mmc_updateAlmClsValue (type, level, limit, durat,
							(void*)&sfdb->sys[sysIndex].commInfo.diskInfo[k]);
				} 
				break;
			case SFM_ALM_TYPE_QUEUE_LOAD:
				for(k=0; k<SFM_MAX_QUE_CNT; k++){	
					fimd_mmc_updateAlmClsValue (type, level, limit, durat,
							(void*)&sfdb->sys[sysIndex].commInfo.queInfo[k]);
				} 
				break;
			case SFM_ALM_TYPE_SESS_LOAD:
				for(k=0; k<SFM_MAX_RSRC_LOAD_CNT; k++){	
					fimd_mmc_updateAlmClsValue (type, level, limit, durat,
							(void*)&sfdb->sys[sysIndex].commInfo.rsrcSts[k]);
				} 
				break;
			case SFM_ALM_TYPE_LOGON_SUCCESS_RATE:
				fimd_mmc_updateLogonSuccessRateAlmClsValue(sysIndex, 0, level, limit);
				break;
			case SFM_ALM_TYPE_LOGOUT_SUCCESS_RATE:
				fimd_mmc_updateLogonSuccessRateAlmClsValue(sysIndex, 1, level, limit);
				break;
			case SFM_ALM_TYPE_SM_CONN_STS:
				fimd_mmc_updateSMChAlmClsValue(sysIndex, level, limit);
				break;
		}
	} else {
		switch(type){
			case SFM_ALM_TYPE_CPU_USAGE:
				for (i=0; i<eqSysCnt; i++) {
					fimd_mmc_updateAlmClsValue (type, level, limit, durat,
						(void*)&sfdb->sys[i].commInfo.cpuInfo);
				}
				break;
			case SFM_ALM_TYPE_MEMORY_USAGE:
				for (i=0; i<eqSysCnt; i++) {
                  	fimd_mmc_updateAlmClsValue (type, level, limit, durat,
                   		(void*)&sfdb->sys[i].commInfo.memInfo);
               	}
                break;
			case SFM_ALM_TYPE_DISK_USAGE:
				for (i=0; i<eqSysCnt; i++) {
  					for(k=0; k<SFM_MAX_DISK_CNT; k++){ 
						fimd_mmc_updateAlmClsValue (type, level, limit, durat,
               				(void*)&sfdb->sys[i].commInfo.diskInfo[k]);
                   	} 
				}
                break;
			case SFM_ALM_TYPE_QUEUE_LOAD:
				for (i=0; i<eqSysCnt; i++) {
					for(k=0; k<SFM_MAX_QUE_CNT; k++){
                    	fimd_mmc_updateAlmClsValue (type, level, limit, durat,
                       		(void*)&sfdb->sys[i].commInfo.queInfo[k]);
                    } 
				}
                break;
			case SFM_ALM_TYPE_SESS_LOAD:
				for (i=0; i<eqSysCnt; i++) {
					for(k=0; k<SFM_MAX_RSRC_LOAD_CNT; k++){
                    	fimd_mmc_updateAlmClsValue (type, level, limit, durat,
                       		(void*)&sfdb->sys[i].commInfo.rsrcSts[k]);
                    } 
				}
           		break;
			case SFM_ALM_TYPE_LOGON_SUCCESS_RATE:
				fimd_mmc_updateLogonSuccessRateAlmClsValue(sysIndex, 0, level, limit);
				break;
			case SFM_ALM_TYPE_LOGOUT_SUCCESS_RATE:
				fimd_mmc_updateLogonSuccessRateAlmClsValue(sysIndex, 1, level, limit);
				break;
			case SFM_ALM_TYPE_SM_CONN_STS:
				fimd_mmc_updateSMChAlmClsValue(sysIndex, level, limit);
				break;
		}
	}

	fimd_txMMLResult (rxIxpcMsg, resBuf, 0, 0, 0, 0, 1);
	
	alm_lmt_dsc_input(); // by helca 08.17
	
	if (argSysFlag) {
		fimd_updateSysAlmInfo(sysIndex);
		fimd_updatePDAlmInfo (devIndex);
		fimd_broadcastAlmEvent2Client();
	}

	return 1;
	
} //----- End of fimd_mmc_set_alm_lmt -----//

/* hjjung */
int fimd_mmc_set_session_lmt (IxpcQMsgType *rxIxpcMsg)
{
	int	i, sysIndex=0, limit, durat=0, devIndex;
	char	argSysFlag=0, argDurFlag=0;
	char	argSys[32], argType[32], argLevel[32], argLimit[32], argDurat[10];
		
	MMLReqMsgType	*rxMmlReqMsg;

	rxMmlReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
	// get input parameters
	//
	if (!strcasecmp(rxMmlReqMsg->head.para[0].paraName, "SYS")) {
		argSysFlag = 1;
		strcpy (argSys, rxMmlReqMsg->head.para[0].paraVal);
		for (i=0; i<strlen(argSys); i++) argSys[i] = toupper(argSys[i]);

		strcpy (argType, rxMmlReqMsg->head.para[1].paraVal);
		for (i=0; i<strlen(argType); i++) argType[i] = toupper(argType[i]);

		strcpy (argLevel, rxMmlReqMsg->head.para[2].paraVal);
		for (i=0; i<strlen(argLevel); i++) argLevel[i] = toupper(argLevel[i]);

		strcpy (argLimit, rxMmlReqMsg->head.para[3].paraVal);
		limit = atoi(argLimit);

		if (strcasecmp(rxMmlReqMsg->head.para[4].paraVal, "")) {
			argDurFlag = 1;
			strcpy (argDurat, rxMmlReqMsg->head.para[4].paraVal);
			durat = atoi(argDurat);
		} else {
			strcpy (argDurat, "");
		}
	} else {
		strcpy (argSys, "");

		strcpy (argType, rxMmlReqMsg->head.para[0].paraVal);
		for (i=0; i<strlen(argType); i++) argType[i] = toupper(argType[i]);

		strcpy (argLevel, rxMmlReqMsg->head.para[1].paraVal);
		for (i=0; i<strlen(argLevel); i++) argLevel[i] = toupper(argLevel[i]);

		strcpy (argLimit, rxMmlReqMsg->head.para[2].paraVal);
		limit = atoi(argLimit);

		if (strcasecmp(rxMmlReqMsg->head.para[3].paraVal, "")) {
			argDurFlag = 1;
			strcpy (argDurat, rxMmlReqMsg->head.para[3].paraVal);
			durat = atoi(argDurat);
		} else {
			strcpy (argDurat, "");
		}
	}

	//
	// - optional parameter의 입력 여부를 확인한다.
	// - 입력된 value를 꺼낸다.
	//
	if (!strcasecmp(argSys, "SCEA")) devIndex = 0;
	else if (!strcasecmp(argSys, "SCEB")) devIndex = 1;
	/* hjjung */
	else if (!strcasecmp(argSys, "SCMA")) devIndex = 0;
	else if (!strcasecmp(argSys, "SCMB")) devIndex = 1;
	else if (strcasecmp(argSys, ""))	 {
		if ((sysIndex = fimd_getSysIndexByName (argSys)) < 0) {
			sprintf(resBuf,"    RESULT = FAIL\n      REASON = UNKNOWN SYSTEM\n\n");
			fimd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, 1);
			return -1;
		}
	}

	if( !strcasecmp(argSys, "SCEA") || !strcasecmp(argSys, "SCEB"))
		return fimd_mmc_sce_set_alm_lmt( rxIxpcMsg, argSys, argType, argLevel,limit,durat,devIndex);
	/* hjjung */
	else if( !strcasecmp(argSys, "SCMA") || !strcasecmp(argSys, "SCMB"))
		return fimd_mmc_leg_set_alm_lmt( rxIxpcMsg, argSys, argType, argLevel,limit,durat,devIndex);

	return -1;
} //----- End of fimd_mmc_set_session_lmt -----//

/* added by dcham 20110525*/
int fimd_mmc_set_tps_lmt (IxpcQMsgType *rxIxpcMsg)
{
	int	i, sysIndex=0, limit, durat=0, devIndex;
	char	argSysFlag=0, argDurFlag=0;
	char	argSys[32], argType[32], argLevel[32], argLimit[32], argDurat[10];
		
	MMLReqMsgType	*rxMmlReqMsg;

	rxMmlReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
	// get input parameters
	//
	if (!strcasecmp(rxMmlReqMsg->head.para[0].paraName, "SYS")) {
		argSysFlag = 1;
		strcpy (argSys, rxMmlReqMsg->head.para[0].paraVal);
		for (i=0; i<strlen(argSys); i++) argSys[i] = toupper(argSys[i]);
#if 0	
		strcpy (argType, rxMmlReqMsg->head.para[1].paraVal);
		for (i=0; i<strlen(argType); i++) argType[i] = toupper(argType[i]);
#endif

		strcpy (argLevel, rxMmlReqMsg->head.para[1].paraVal);
		for (i=0; i<strlen(argLevel); i++) argLevel[i] = toupper(argLevel[i]);

		strcpy (argLimit, rxMmlReqMsg->head.para[2].paraVal);
		limit = atoi(argLimit);

		if (strcasecmp(rxMmlReqMsg->head.para[3].paraVal, "")) {
			argDurFlag = 1;
			strcpy (argDurat, rxMmlReqMsg->head.para[3].paraVal);
			durat = atoi(argDurat);
		} else {
			strcpy (argDurat, "");
		}
	} else {
		strcpy (argSys, "");
#if 0
		strcpy (argType, rxMmlReqMsg->head.para[0].paraVal);
		for (i=0; i<strlen(argType); i++) argType[i] = toupper(argType[i]);
#endif

		strcpy (argLevel, rxMmlReqMsg->head.para[0].paraVal);
		for (i=0; i<strlen(argLevel); i++) argLevel[i] = toupper(argLevel[i]);

		strcpy (argLimit, rxMmlReqMsg->head.para[1].paraVal);
		limit = atoi(argLimit);

		if (strcasecmp(rxMmlReqMsg->head.para[2].paraVal, "")) {
			argDurFlag = 1;
			strcpy (argDurat, rxMmlReqMsg->head.para[2].paraVal);
			durat = atoi(argDurat);
		} else {
			strcpy (argDurat, "");
		}
	}

	// - optional parameter의 입력 여부를 확인한다.
	// - 입력된 value를 꺼낸다.
	if (!strcasecmp(argSys, "SCMA")) devIndex = 0;
	else if (!strcasecmp(argSys, "SCMB")) devIndex = 1;
	else if (strcasecmp(argSys, ""))	 {
       	if ((sysIndex = fimd_getSysIndexByName (argSys)) < 0) {
			sprintf(resBuf,"    RESULT = FAIL\n      REASON = UNKNOWN SYSTEM\n\n");
			fimd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, 1);
			return -1;
		}
	}

	if( !strcasecmp(argSys, "SCMA") || !strcasecmp(argSys, "SCMB"))
       	return fimd_mmc_call_set_alm_lmt( rxIxpcMsg, argSys, argType, argLevel,limit,durat,devIndex);

	return -1;
} //----- End of fimd_mmc_set_tps_lmt -----//

/* by helca */
int fimd_mmc_set_svc_alm (IxpcQMsgType *rxIxpcMsg)
{
	int		i, j,sysIndex=0, rateIndex, type, mincnt, rate;
	char	argSysFlag=0;
	char	argSys[32], argSvcName[32], argMinCnt[32], argRate[32];
	char	tmpBuf[256], fname[256];
	char	psBuf[256];	
	char    name[6];
	char    *env;
	
	FILE *fp;
	MMLReqMsgType	*rxMmlReqMsg;
	rxMmlReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	if ((env = getenv(IV_HOME)) == NULL)
	{
		sprintf(trcBuf,"[%s] getenv fail\n", __FUNCTION__);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	// get input parameters
	//
	if (!strcasecmp(rxMmlReqMsg->head.para[0].paraName, "SYS")) {
		argSysFlag = 1;
		strcpy (argSys, rxMmlReqMsg->head.para[0].paraVal);
		for (i=0; i<strlen(argSys); i++) argSys[i] = toupper(argSys[i]);

		strcpy (argSvcName, rxMmlReqMsg->head.para[1].paraVal);
		for (i=0; i<strlen(argSvcName); i++) argSvcName[i] = toupper(argSvcName[i]);

		strcpy (argMinCnt, rxMmlReqMsg->head.para[2].paraVal);
		mincnt = atoi(argMinCnt);

		strcpy (argRate, rxMmlReqMsg->head.para[3].paraVal);
		rate = atoi(argRate);

	} else {
		strcpy (argSys, "");

		strcpy (argSvcName, rxMmlReqMsg->head.para[0].paraVal);
		for (i=0; i<strlen(argSvcName); i++) argSvcName[i] = toupper(argSvcName[i]);

		strcpy (argMinCnt, rxMmlReqMsg->head.para[1].paraVal);
		mincnt = atoi(argMinCnt);

		strcpy (argRate, rxMmlReqMsg->head.para[2].paraVal);
		rate = atoi(argRate);
	}

	if (strcasecmp(argSys, ""))	 {
		if(!strcasecmp(argSys, "ALL")) {
			strcpy(name, "ALL");
		}else if ((sysIndex = fimd_getSysIndexByName (argSys)) < 0) {
			sprintf(resBuf,"    RESULT = FAIL\n      REASON = UNKNOWN SYSTEM\n\n");
			fimd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, 1);
			return -1;
		}
	}

	if (!strcasecmp(argSvcName, "UAWAP")) {
		type = 0;
	} else if (!strcasecmp(argSvcName, "AAA")) {
		type = 1;
	} else if (!strcasecmp(argSvcName, "WAP1")) {
		type = 2; 
	} else if (!strcasecmp(argSvcName, "WAP2")) {
		type = 3; 
    	} else if (!strcasecmp(argSvcName, "HTTP")) {
        	type = 4;
	} else if (!strcasecmp(argSvcName, "VODS")) {
        	type = 5;
	} else if (!strcasecmp(argSvcName, "ANAAA")) {
        	type = 6;
	} else if (!strcasecmp(argSvcName, "VT")) {
                type = 7;
        }
	 
	memset(psBuf, 0x00, sizeof(psBuf));
	memset(resBuf, 0x00, sizeof(resBuf));
	memset(tmpBuf, 0x00, sizeof(tmpBuf));
	if(strcasecmp(name, "ALL")){
		sprintf(tmpBuf,"    RESULT = SUCCESS \n");
		strcat (tmpBuf,"    ================================================================================\n");
		strcat (tmpBuf,"      SYSTEM           PROTOCOL_TYPE    MINIMUM_TRANSACTION_COUNT     SUCCESS_RATE  \n");
		strcat (tmpBuf,"    ================================================================================\n");
		sprintf (psBuf,"      %-4s", sfdb->sys[sysIndex].commInfo.name);
		strcat (tmpBuf, psBuf);
		strcpy (resBuf,tmpBuf);
		
		sprintf(tmpBuf,"      OLD    %-5s            %-10d                    %-10d\n",
				sfdb->sys[sysIndex].commInfo.succRate[type].name,
				sfdb->sys[sysIndex].commInfo.succRate[type].cnt,
				sfdb->sys[sysIndex].commInfo.succRate[type].rate);
	
		strcat (resBuf,tmpBuf);
		sprintf(tmpBuf,"                NEW    %-5s            %-10d                    %-10d\n", 
				argSvcName, mincnt, rate);
		strcat (resBuf,tmpBuf);
		
		
	}
	else if(!strcasecmp(name, "ALL")){
		//	memset(tmpBuf, 0x00, sizeof(tmpBuf));
			
		sprintf(tmpBuf,"    RESULT = SUCCESS \n");
		strcat (tmpBuf,"    ================================================================================\n");
    		strcat (tmpBuf,"      SYSTEM           PROTOCOL_TYPE    MINIMUM_TRANSACTION_COUNT     SUCCESS_RATE  \n");
		strcat (tmpBuf,"    ================================================================================\n");
			
		for(i=1;i<3;i++){
			sprintf(psBuf,"      %-4s", sfdb->sys[i].commInfo.name);
			strcat (tmpBuf, psBuf);
			strcat (resBuf,tmpBuf);
				
			sprintf(tmpBuf,"      OLD    %-5s            %-10d                    %-10d\n",
				sfdb->sys[i].commInfo.succRate[type].name,
				sfdb->sys[i].commInfo.succRate[type].cnt,
				sfdb->sys[i].commInfo.succRate[type].rate);
	
			strcat (resBuf,tmpBuf);
			sprintf(tmpBuf,"                NEW    %-5s            %-10d                    %-10d\n", 
				argSvcName, mincnt, rate);
			strcat (resBuf,tmpBuf);
				 
			sprintf (tmpBuf,"    --------------------------------------------------------------------------------\n");
				
		}
	}

	strcat (resBuf,"    ================================================================================\n");
	strcat (resBuf, "\n");


	for (i=0; i<SFM_REAL_SUCC_RATE_CNT; i++) {
		if (!strcasecmp((char*)sfdb->sys[sysIndex].commInfo.succRate[i].name, argSvcName)) {
			rateIndex = i;
			break;
		}
	}

	//
	// 해당 값으로 update한다.
	//
	if(strcasecmp(name, "ALL")){

		if (argSysFlag) {
			fimd_mmc_updateAlmRateValue (sysIndex, type, mincnt, rate,
						(void*)&sfdb->sys[sysIndex].commInfo.succRate[rateIndex]);
		} else {
			for (i=0; i<eqSysCnt; i++) {
				fimd_mmc_updateAlmRateValue (sysIndex, type, mincnt, rate,
						(void*)&sfdb->sys[sysIndex].commInfo.succRate[i]);
			}
		}
	}
	else if (!strcasecmp(name, "ALL")){
		for(j=1; j<3; j++){
			if (argSysFlag) {
				fimd_mmc_updateAlmRateValue (j, type, mincnt, rate,
							(void*)&sfdb->sys[j].commInfo.succRate[rateIndex]);
			} else {
				for (i=0; i<eqSysCnt; i++) {
					fimd_mmc_updateAlmRateValue (j, type, mincnt, rate,
							(void*)&sfdb->sys[j].commInfo.succRate[i]);
				}
			}
		} // -- end for() --//
	}
	
	fimd_txMMLResult (rxIxpcMsg, resBuf, 0, 0, 0, 0, 1);

	if (argSysFlag) {
		fimd_updateSysAlmInfo(sysIndex);
		fimd_broadcastAlmEvent2Client();
	}

	sprintf(fname, "%s/DATA/rate_file", env); // by helca 07.31

	if((fp = fopen(fname, "w+")) == NULL)
	{
		sprintf (trcBuf, "[disprc] fopen fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno));
		trclib_writeLogErr (FL,trcBuf);
		return 0;
	}

	fprintf(fp,"%s %d %d %s %d %d %s %d %d %s %d %d %s %d %d %s %d %d %s %d %d %s %d %d\n", 
		sfdb->sys[1].commInfo.succRate[0].name, sfdb->sys[1].commInfo.succRate[0].cnt, sfdb->sys[1].commInfo.succRate[0].rate,
		sfdb->sys[1].commInfo.succRate[1].name, sfdb->sys[1].commInfo.succRate[1].cnt, sfdb->sys[1].commInfo.succRate[1].rate,
		sfdb->sys[1].commInfo.succRate[2].name, sfdb->sys[1].commInfo.succRate[2].cnt, sfdb->sys[1].commInfo.succRate[2].rate,
		sfdb->sys[1].commInfo.succRate[3].name, sfdb->sys[1].commInfo.succRate[3].cnt, sfdb->sys[1].commInfo.succRate[3].rate,
		sfdb->sys[1].commInfo.succRate[4].name, sfdb->sys[1].commInfo.succRate[4].cnt, sfdb->sys[1].commInfo.succRate[4].rate,
		sfdb->sys[1].commInfo.succRate[5].name, sfdb->sys[1].commInfo.succRate[5].cnt, sfdb->sys[1].commInfo.succRate[5].rate,
		sfdb->sys[1].commInfo.succRate[6].name, sfdb->sys[1].commInfo.succRate[6].cnt, sfdb->sys[1].commInfo.succRate[6].rate,
		sfdb->sys[1].commInfo.succRate[7].name, sfdb->sys[1].commInfo.succRate[7].cnt, sfdb->sys[1].commInfo.succRate[7].rate
	);

	memset(name, 0x00, sizeof(name));
	memset(psBuf, 0x00, sizeof(psBuf));
	memset(resBuf, 0x00, sizeof(resBuf));
	memset(tmpBuf, 0x00, sizeof(tmpBuf));
	if(fp)fclose(fp);
	return 1;
	
} //----- End of fimd_mmc_set_svc_alm -----//

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int fimd_mmc_checkAlmLimitValidation (int type, int level, int limit, void *ptr)
{
	SFM_CpuInfo		*cpuInfo;
	SFM_MemInfo		*memInfo;
	SFM_DiskInfo		*diskInfo;
	SFM_QueInfo		*queInfo;
	SFM_SysRSRCInfo		*rsrcInfo; // by helca 08.02 rsrc

	unsigned char		minLimit, majLimit, criLimit;



	switch (type)
	{
		case SFM_ALM_TYPE_CPU_USAGE:
			cpuInfo = (SFM_CpuInfo*)ptr;

			minLimit = cpuInfo->minLimit;
			majLimit = cpuInfo->majLimit;
			criLimit = cpuInfo->criLimit;
			break;

		case SFM_ALM_TYPE_MEMORY_USAGE:
			memInfo = (SFM_MemInfo*)ptr;
				
			minLimit = memInfo->minLimit;
			majLimit = memInfo->majLimit;
			criLimit = memInfo->criLimit;
			break;

		case SFM_ALM_TYPE_DISK_USAGE:
			diskInfo = (SFM_DiskInfo*)ptr;
	
			minLimit = diskInfo->minLimit;
			majLimit = diskInfo->majLimit;
			criLimit = diskInfo->criLimit;
			break;
		
		// by helca 08.08
		case SFM_ALM_TYPE_QUEUE_LOAD:
			queInfo = (SFM_QueInfo*)ptr;
			
			minLimit = queInfo->minLimit;
			majLimit = queInfo->majLimit;
			criLimit = queInfo->criLimit;
			break;

		// by helca 08.02 rsrc -> sess 명칭 변경
		case SFM_ALM_TYPE_SESS_LOAD:
			rsrcInfo = (SFM_SysRSRCInfo*)ptr;
			
			minLimit = rsrcInfo->minLimit;
			majLimit = rsrcInfo->majLimit;
			criLimit = rsrcInfo->criLimit;
			break;
		// 알람 통계 기능 삭제 
	}
#if 1
	switch (level) {
		case SFM_ALM_MINOR: // minor limit은 major보다 작아야 한다.
			if (limit >= majLimit) {
				sprintf(resBuf,"    RESULT = FAIL\n      REASON = INVALID LIMIT VALUE(MORE THAN MAJOR(%d))\n\n", majLimit);
				return -1;
			}
			break;

		case SFM_ALM_MAJOR: // major limit은 minor보다 크고, critical보다 작아야 한다.
			if (limit <= minLimit) {
				sprintf(resBuf,"    RESULT = FAIL\n      REASON = INVALID LIMIT VALUE(LESS THAN MINOR(%d))\n\n", minLimit);
				return -1;
			}
			if (limit >= criLimit) {
				sprintf(resBuf,"    RESULT = FAIL\n      REASON = INVALID LIMIT VALUE(MORE THAN CRITICAL(%d))\n\n", criLimit);
				return -1;
			}
			break;

		case SFM_ALM_CRITICAL: // critical limit은 major보다 커야 한다.
			if (limit <= majLimit) {
				sprintf(resBuf,"    RESULT = FAIL\n      REASON = INVALID LIMIT VALUE(LESS THAN MAJOR(%d))\n", majLimit);
				return -1;
			}
			break;
	}
#endif
	return 1;
	
} //----- End of fimd_mmc_checkAlmLimitValidation -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int fimd_mmc_updateAlmClsValue (int type, int level, int limit, int durat, void *ptr)
{
	SFM_CpuInfo		*cpuInfo;
	SFM_MemInfo		*memInfo;
	SFM_DiskInfo		*diskInfo;
	SFM_QueInfo		*queInfo;
	SFM_SysRSRCInfo		*rsrcInfo;

	switch (type)
	{
	case SFM_ALM_TYPE_CPU_USAGE:
		cpuInfo = (SFM_CpuInfo*)ptr;
		switch (level) {
			case SFM_ALM_MINOR:
				cpuInfo->minLimit = (unsigned char)limit;
				if (durat) cpuInfo->minDurat = (unsigned char)durat;
				break;
			case SFM_ALM_MAJOR:
				cpuInfo->majLimit = (unsigned char)limit;
				if (durat) cpuInfo->majDurat = (unsigned char)durat;
				break;
			case SFM_ALM_CRITICAL:
				cpuInfo->criLimit = (unsigned char)limit;
				if (durat) cpuInfo->criDurat = (unsigned char)durat;
				break;
		}
		break;

	case SFM_ALM_TYPE_MEMORY_USAGE:
		memInfo = (SFM_MemInfo*)ptr;
		switch (level) {
		case SFM_ALM_MINOR:
			memInfo->minLimit = (unsigned char)limit;
			if (durat) memInfo->minDurat = (unsigned char)durat;
			break;
		case SFM_ALM_MAJOR:
			memInfo->majLimit = (unsigned char)limit;
			if (durat) memInfo->majDurat = (unsigned char)durat;
			break;
		case SFM_ALM_CRITICAL:
			memInfo->criLimit = (unsigned char)limit;
			if (durat) memInfo->criDurat = (unsigned char)durat;
			break;
		}
		break;

	case SFM_ALM_TYPE_DISK_USAGE:
		diskInfo = (SFM_DiskInfo*)ptr;
		switch (level) {
			case SFM_ALM_MINOR:    diskInfo->minLimit = (unsigned char)limit; break;
			case SFM_ALM_MAJOR:    diskInfo->majLimit = (unsigned char)limit; break;
			case SFM_ALM_CRITICAL: diskInfo->criLimit = (unsigned char)limit; break;
		}
		break;

	// by helca 08.08
	case SFM_ALM_TYPE_QUEUE_LOAD:
		queInfo = (SFM_QueInfo*)ptr;
		switch (level) {
			case SFM_ALM_MINOR:    queInfo->minLimit = (unsigned char)limit; break;
			case SFM_ALM_MAJOR:    queInfo->majLimit = (unsigned char)limit; break;
			case SFM_ALM_CRITICAL: queInfo->criLimit = (unsigned char)limit; break;
		}
		break;
	// by helca 08.02
	// RSRC -> SESS 명칭 변경
	case SFM_ALM_TYPE_SESS_LOAD:
		rsrcInfo = (SFM_SysRSRCInfo*)ptr;
		switch (level) {
		case SFM_ALM_MINOR:
			rsrcInfo->minLimit = (unsigned char)limit;
			if (durat) rsrcInfo->minDurat = (unsigned char)durat;
			break;
		case SFM_ALM_MAJOR:
			rsrcInfo->majLimit = (unsigned char)limit;
			if (durat) rsrcInfo->majDurat = (unsigned char)durat;
			break;
		case SFM_ALM_CRITICAL:
			rsrcInfo->criLimit = (unsigned char)limit;
			if (durat) rsrcInfo->criDurat = (unsigned char)durat;
		break;
		}
		break;

	}

	return 1;
	
} //----- End of fimd_mmc_updateAlmClsValue -----//

/* by helca */
//-------------------------------------------------------------------------
int fimd_mmc_updateAlmRateValue (int sysIndex, int type, int mincnt, int rate, void *ptr)
{

	//SFM_SysSuccRate		*succRate;
	switch (type)
	{
		case 0:
//			succRate = (SFM_SysSuccRate *)ptr;
			strcpy((char*)sfdb->sys[sysIndex].commInfo.succRate[type].name, "UAWAP");
			sfdb->sys[sysIndex].commInfo.succRate[type].cnt = (unsigned int)mincnt;
			sfdb->sys[sysIndex].commInfo.succRate[type].rate = (unsigned int)rate;

		break;
		case 1:
//			succRate = (SFM_SysSuccRate*)ptr;
			strcpy((char*)sfdb->sys[sysIndex].commInfo.succRate[type].name, "AAA");
			sfdb->sys[sysIndex].commInfo.succRate[type].cnt = (unsigned int)mincnt;
			sfdb->sys[sysIndex].commInfo.succRate[type].rate = (unsigned int)rate;
		break;
		case 2:
//		succRate = (SFM_SysSuccRate*)ptr;
			strcpy((char*)sfdb->sys[sysIndex].commInfo.succRate[type].name, "WAP1");
			sfdb->sys[sysIndex].commInfo.succRate[type].cnt = (unsigned int)mincnt;
			sfdb->sys[sysIndex].commInfo.succRate[type].rate = (unsigned int)rate;
		break;
		case 3:
//		succRate = (SFM_SysSuccRate*)ptr;
			strcpy((char*)sfdb->sys[sysIndex].commInfo.succRate[type].name, "WAP2");
			sfdb->sys[sysIndex].commInfo.succRate[type].cnt = (unsigned int)mincnt;
			sfdb->sys[sysIndex].commInfo.succRate[type].rate = (unsigned int)rate;
		break;

    		case 4:
//        	succRate = (SFM_SysSuccRate*)ptr;
			strcpy((char*)sfdb->sys[sysIndex].commInfo.succRate[type].name, "HTTP");
			sfdb->sys[sysIndex].commInfo.succRate[type].cnt = (unsigned int)mincnt;
			sfdb->sys[sysIndex].commInfo.succRate[type].rate = (unsigned int)rate;
        	break;
	
		case 5:
//        	succRate = (SFM_SysSuccRate*)ptr;
			strcpy((char*)sfdb->sys[sysIndex].commInfo.succRate[type].name, "VODS");
			sfdb->sys[sysIndex].commInfo.succRate[type].cnt = (unsigned int)mincnt;
			sfdb->sys[sysIndex].commInfo.succRate[type].rate = (unsigned int)rate;
        	break;
        
    		case 6:
//        	succRate = (SFM_SysSuccRate*)ptr;
			strcpy((char*)sfdb->sys[sysIndex].commInfo.succRate[type].name, "ANAAA");
			sfdb->sys[sysIndex].commInfo.succRate[type].cnt = (unsigned int)mincnt;
			sfdb->sys[sysIndex].commInfo.succRate[type].rate = (unsigned int)rate;
        	break;    
		case 7:
//              succRate = (SFM_SysSuccRate*)ptr;
                        strcpy((char*)sfdb->sys[sysIndex].commInfo.succRate[type].name, "VT");
                        sfdb->sys[sysIndex].commInfo.succRate[type].cnt = (unsigned int)mincnt;
                        sfdb->sys[sysIndex].commInfo.succRate[type].rate = (unsigned int)rate;
                break;
	}

	return 1;
	
} //----- End of fimd_mmc_updateAlmRateValue -----//

//------------------------------------------------------------------------------
//cosmoslight change 
//------------------------------------------------------------------------------
int fimd_mmc_dis_alm_sts (IxpcQMsgType *rxIxpcMsg)
{
	int	i, sysIndex, minCnt=0, majCnt=0, criCnt=0;
	char	seqNo=1;
	char	tmpBuf[4096], rcBuf[4096];
	MMLReqMsgType	*rxMmlReqMsg;
	rxMmlReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	char    query[512];
	int     rowCount = 0, alarmIndex, alarmSts;
	char    sysname[4];
	MYSQL_RES   *result;
	MYSQL_ROW   row;

	memset(tmpBuf, 0x00, sizeof(tmpBuf));
	memset(query, 0x00, sizeof(query));
	
	// 특정 시스템만 지정했는지 확인한다.
	// 결과 메시지에 대문자로 출력하기 위해 대문자로 바꾼다.
	for (i=0; i<strlen(rxMmlReqMsg->head.para[0].paraVal); i++)
		rxMmlReqMsg->head.para[0].paraVal[i] = toupper(rxMmlReqMsg->head.para[0].paraVal[i]);

	if (!strcasecmp(rxMmlReqMsg->head.para[0].paraVal, ""))
		goto all;
	else if (strcasecmp(rxMmlReqMsg->head.para[0].paraVal, "ALL")){
		if (!strcasecmp(rxMmlReqMsg->head.para[0].paraVal, "TAPA")){
			;	
		} else if (!strcasecmp(rxMmlReqMsg->head.para[0].paraVal, "TAPB")){
			;
		} else if (!strcasecmp(rxMmlReqMsg->head.para[0].paraVal, "TAPB")){
			;
		} else if (!strcasecmp(rxMmlReqMsg->head.para[0].paraVal, "SCEA")){
			;
		} else if (!strcasecmp(rxMmlReqMsg->head.para[0].paraVal, "SCEB")){
			;
		} else if (!strcasecmp(rxMmlReqMsg->head.para[0].paraVal, "L2SWA")){
			;
		} else if (!strcasecmp(rxMmlReqMsg->head.para[0].paraVal, "L2SWB")){
			;
		} else if ((sysIndex = fimd_getSysIndexByName (rxMmlReqMsg->head.para[0].paraVal)) < 0) {
			sprintf(resBuf,"    RESULT = FAIL\n    REASON = UNKNOWN SYSTEM\n\n");
			fimd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo);
			return -1;
		}
		sprintf(rcBuf,"    RESULT = SUCCESS\n");
		sprintf(resHead,"    SYSTEM = %s\n", rxMmlReqMsg->head.para[0].paraVal);
		
		sprintf(query, "SELECT * FROM current_alarm WHERE system_name = '%s' AND mask = 0 "
			"ORDER BY system_name, alarm_level, alarm_type",
			rxMmlReqMsg->head.para[0].paraVal);
	
	}else if (!strcasecmp(rxMmlReqMsg->head.para[0].paraVal, "ALL")){
		all:
		sprintf(rcBuf,"    RESULT = SUCCESS\n");
		sprintf(resHead,"    SYSTEM = ALL\n");
		sprintf(query, "SELECT * FROM current_alarm WHERE mask = 0 "
			"ORDER BY system_name, alarm_level, alarm_type");
	}	

	// current_alarm 에서 장애 목록을 가져온다.
	//

	if(fimd_mysql_query(query) < 0){
        	sprintf(trcBuf, "fimd_Current_alarm mysql_query fail \n");
        	trclib_writeLogErr (FL,trcBuf);
        	return -1;
	}

	result = mysql_store_result(mysql_conn);

	strcat (resHead,"    ============================================================================================\n");
	strcat (resHead,"      SYSTEM  TYPE                   LEVEL     DATE                 INFORMATION\n");
	strcat (resHead,"    ============================================================================================\n");
	strcat (rcBuf, resHead);
	strcpy (resBuf, rcBuf);

	while((row = mysql_fetch_row(result)) != NULL) {
		alarmIndex = atoi(row[3]) -1;
		alarmSts = atoi(row[5]);
		if (atoi(row[5]) == 1) minCnt++;
		else if (atoi(row[5]) == 2) majCnt++;
		else if (atoi(row[5]) == 3) criCnt++;
		
		if (rowCount > 1 && strcasecmp(sysname, row[2])){
			sprintf(tmpBuf,"    --------------------------------------------------------------------------------------------\n");
			strcat (resBuf,tmpBuf);
		}
		sprintf(tmpBuf, "      %-6s  %-21s  %-8s  %-19s  %-35s\n"
				,row[2], alarmName[alarmIndex], fimd_printAlmLevel(alarmSts), row[6], row[7]);
		strcat (resBuf,tmpBuf);
		strcpy(sysname, row[2]);
		rowCount++;

		if (strlen(resBuf) > 2000) {
			strcat (resBuf,"\n");
			fimd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, 5, 1, (seqNo)++);
			strcpy (resBuf, resHead);
		}
	}
	if (rowCount == 0) {
		sprintf(tmpBuf, "      NO ALARM DATA\n");
		strcat (resBuf,tmpBuf);

	}
	mysql_free_result(result);
	
	sprintf(tmpBuf,"    --------------------------------------------------------------------------------------------\n");
	strcat (resBuf,tmpBuf);
	sprintf(tmpBuf,"      MINOR = %d,  MAJOR = %d,  CRITICAL = %d\n", minCnt, majCnt, criCnt);
	strcat (resBuf,tmpBuf);
	strcat (resBuf,"    ============================================================================================\n");
	
	fimd_txMMLResult (rxIxpcMsg, resBuf, 0, 0, 0, 0, seqNo);
	memset(resBuf, 0x00, sizeof(resBuf));

	return 1;
	
} //----- End of fimd_mmc_dis_alm_sts -----//


//------------------------------------------------------------------------------
int fimd_mmc_dis_mask_sts (IxpcQMsgType *rxIxpcMsg)
{
	int     i, j, lineCnt, sysIndex=0;

	char    argSysFlag=0, seqNo=1;
	char    rcBuf[2048];
    
	MMLReqMsgType   *rxMmlReqMsg;
	memset(resBuf, 0x00, sizeof(resBuf));
	memset(rcBuf, 0x00, sizeof(rcBuf));
	memset(resHead, 0x00, sizeof(resHead));
    
	rxMmlReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
	// 특정 시스템만 지정했는지 확인한다.
	if (!strcasecmp(rxMmlReqMsg->head.para[0].paraVal, ""))
		goto all;
	else if (strcasecmp(rxMmlReqMsg->head.para[0].paraVal, "ALL"))
	{
		argSysFlag = 1;
		if(!strcasecmp(rxMmlReqMsg->head.para[0].paraVal, "TAPA")){
			sysIndex = TAPA; 
		} else if(!strcasecmp(rxMmlReqMsg->head.para[0].paraVal, "TAPB")){
			sysIndex = TAPB; 
		} else if(!strcasecmp(rxMmlReqMsg->head.para[0].paraVal, "SCEA")){
			sysIndex = SCEA; 
		} else if(!strcasecmp(rxMmlReqMsg->head.para[0].paraVal, "SCEB")){
			sysIndex = SCEB; 
		/* hjjung */
		} else if(!strcasecmp(rxMmlReqMsg->head.para[0].paraVal, "SCMA")){
			sysIndex = SCMA; 
		} else if(!strcasecmp(rxMmlReqMsg->head.para[0].paraVal, "SCMB")){
			sysIndex = SCMB; 
		} else if(!strcasecmp(rxMmlReqMsg->head.para[0].paraVal, "L2SWA")){
			sysIndex = L2SWA; 
		} else if(!strcasecmp(rxMmlReqMsg->head.para[0].paraVal, "L2SWB")){
			sysIndex = L2SWB; 
		}else if ((sysIndex = fimd_getSysIndexByName (rxMmlReqMsg->head.para[0].paraVal)) < 0) {
			sprintf(resBuf,"\n    SYSTEM = %s\n    RESULT = FAIL\n      REASON = UNKNOWN_SYSTEM\n",
							rxMmlReqMsg->head.para[0].paraVal);
			fimd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, seqNo);
			return -1;
		}
        
		// 결과 메시지에 대문자로 출력하기 위해 대문자로 바꾼다.
		for (i=0; i < strlen(rxMmlReqMsg->head.para[0].paraVal); i++){
			rxMmlReqMsg->head.para[0].paraVal[i] = toupper(rxMmlReqMsg->head.para[0].paraVal[i]);
		}	
		sprintf(rcBuf,"    RESULT = SUCCESS");
		sprintf(resHead,"\n    SYSTEM = %s\n", rxMmlReqMsg->head.para[0].paraVal);

		strcat (resHead,"    ======================================================\n");
		strcat (resHead,"      TYPE             STATUS        INFORMATION\n");
		strcat (resHead,"    ======================================================\n");
		strcat (rcBuf, resHead);
		strcpy (resBuf, rcBuf);
		lineCnt = 10;

		if (argSysFlag) {
			if(sysIndex == TAPA || sysIndex == TAPB) {
				fimd_mmc_makePDSysMaskStsOutputMsg (rxIxpcMsg, sysIndex, &lineCnt, &seqNo);
			}
			else if(sysIndex == SCEA || sysIndex == SCEB){
				fimd_mmc_makeSCESysMaskStsOutputMsg (rxIxpcMsg, sysIndex, &lineCnt, &seqNo);
			}
			else if(sysIndex == L2SWA || sysIndex == L2SWB){
				fimd_mmc_makeL2swSysMaskStsOutputMsg(rxIxpcMsg, sysIndex, &lineCnt, &seqNo);
			}
			else
				fimd_mmc_makeSysMaskStsOutputMsg (rxIxpcMsg, sysIndex, &lineCnt, &seqNo);

			strcat (resBuf,"    ======================================================\n");
		}
		fimd_txMMLResult (rxIxpcMsg, resBuf, 0, 0, 0, 0, seqNo);
    
	}else {
		all:
		for (i=0; i<SYSCONF_MAX_ASSO_SYS_NUM; i++) {	
			
			sprintf(rcBuf,"    RESULT = SUCCESS");
			sprintf(resHead,"\n    SYSTEM = %s\n", "ALL");
			strcat (resHead,"    ===================================================================\n");
			strcat (resHead,"      SYSNAME        TYPE           STATUS        INFORMATION\n");
			strcat (resHead,"    ===================================================================\n");
			strcat (rcBuf, resHead);
			strcat (resBuf, rcBuf);
		
			fimd_mmc_makeSysMaskStsOutputAllMsg (rxIxpcMsg, i, &lineCnt, &seqNo);
			strcat (resBuf,"    ===================================================================\n");
			strcat (resBuf,"\n");	
		}
		
		//fimd_txMMLResult (rxIxpcMsg, resBuf, 0, 0, 0, 0, seqNo);	
		//fimd_txMMLResult (rxIxpcMsg, resBuf, 0, 0, 0, 1, seqNo);
		//memset(resBuf, 0x00, sizeof(resBuf));	

		/* Director */
		for(j=0; j<2; j++) {	
			sprintf(rcBuf,"    RESULT = SUCCESS");
			sprintf(resHead,"\n    SYSTEM = %s\n", "ALL");
			strcat (resHead,"    ===================================================================\n");
			strcat (resHead,"      SYSNAME        TYPE           STATUS        INFORMATION\n");
			strcat (resHead,"    ===================================================================\n");
			strcat (rcBuf, resHead);
			strcat (resBuf, rcBuf);
			fimd_mmc_makePDSysMaskStsOutputAllMsg (rxIxpcMsg, j, &lineCnt, &seqNo);
			strcat (resBuf,"    ===================================================================\n");
			strcat (resBuf, "\n");	
			//fimd_txMMLResult (rxIxpcMsg, resBuf, 0, 0, 0, 0, seqNo);	
			//fimd_txMMLResult (rxIxpcMsg, resBuf, 0, 0, 0, 1, seqNo);

			//strcat (resBuf,"\n");
			fimd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, 5, 1, seqNo++);
			commlib_microSleep(500000); // 50 MS
			memset(resBuf, 0x00, sizeof(resBuf));
		}

		/* SCE */
		for(j=0; j<2; j++){
			sprintf(rcBuf,"    RESULT = SUCCESS");
			sprintf(resHead,"\n    SYSTEM = %s\n", "ALL");
			strcat (resHead,"    ===================================================================\n");
			strcat (resHead,"      SYSNAME        TYPE             STATUS        INFORMATION\n");
			strcat (resHead,"    ===================================================================\n");
			strcat (rcBuf, resHead);
			strcat (resBuf, rcBuf);
			fimd_mmc_makeSCESysMaskStsOutputAllMsg (rxIxpcMsg, j, &lineCnt, &seqNo);
			strcat (resBuf,"    ===================================================================\n");
			strcat (resBuf, "\n");	

			fimd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, 5, 1, seqNo++);
			commlib_microSleep(500000); // 50 MS
			memset(resBuf, 0x00, sizeof(resBuf));
		}

		/* L2 Switch  */
		for(j=0; j<2; j++) {	
			sprintf(rcBuf,"    RESULT = SUCCESS");
			sprintf(resHead,"\n    SYSTEM = %s\n", "ALL");
			strcat (resHead,"    ===================================================================\n");
			strcat (resHead,"      SYSNAME        TYPE           STATUS        INFORMATION\n");
			strcat (resHead,"    ===================================================================\n");
			strcat (rcBuf, resHead);
			strcat (resBuf, rcBuf);
			fimd_mmc_makeL2swSysMaskStsOutputAllMsg (rxIxpcMsg, j, &lineCnt, &seqNo);
			strcat (resBuf,"    ===================================================================\n");
			strcat (resBuf, "\n");	
			fimd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, 5, 1, seqNo++);
			commlib_microSleep(500000); // 50 MS
			memset(resBuf, 0x00, sizeof(resBuf));
		}

		fimd_txMMLResult (rxIxpcMsg, resBuf, 0, 0, 0, 0, seqNo);

	}
   	memset(resBuf, 0x00, sizeof(resBuf)); 
	return 1;
    
} //----- End of fimd_mmc_dis_mask_sts -----//


#define DIS_ALM_STS_CHECK_LINE_CNT(lineCnt)	
/**
do {	\
	if (++(lineCnt) >= 40) {						\
		strcat (resBuf,"    -------------------------------------------------------------------------");	\
		strcat (resBuf,"\n");											\
		fimd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, 5, 1, (*seqNo)++);	\
		(lineCnt) = 0;													\
		commlib_microSleep(100000);										\
		strcpy (resBuf, resHead);										\
	}																	\
} while(0);
**/

void fimd_mmc_makeSysMaskStsOutputMsg (
            IxpcQMsgType *rxIxpcMsg,
            int sysIndex,
            int *lineCnt,
            char *seqNo
            )
{
	int     i,j;
	char    tmpBuf[256], ntpName[2][10];
	
	
	strcpy(ntpName[0], "DAEMON");
	strcpy(ntpName[1], "CHANNEL");
	
    
	//fprintf (stderr, "sysIndex: %d. %d \n", sysIndex, sfdb->sys[sysIndex].commInfo.cpuCnt);
	// CPU Mask 
	//
	for (i=0; i<sfdb->sys[sysIndex].commInfo.cpuCnt; i++) {
		if (sfdb->sys[sysIndex].commInfo.cpuInfo.mask[i] == SFM_ALM_MASKED) {
			sprintf(tmpBuf,"      CPU              MASKED        CPU%d\n",
					i);
			strcat (resBuf,tmpBuf);

			// 한번에 40줄까지만 출력하기 위해, 40줄이면 mmcd로 결과 메시지를 보내고
			//  lineCnt를 clear한다.
			DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
		}
	}

	// Memory Mask 
	//
	if (sfdb->sys[sysIndex].commInfo.memInfo.mask == SFM_ALM_MASKED) {
		sprintf(tmpBuf,"      MEMORY           MASKED        Memory\n");
		strcat (resBuf,tmpBuf);
		DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
	}

	// Disk Mask 
	//
	for (i=0; i<sfdb->sys[sysIndex].commInfo.diskCnt; i++) {
		if (sfdb->sys[sysIndex].commInfo.diskInfo[i].mask == SFM_ALM_MASKED) {
			sprintf(tmpBuf,"      DISK             MASKED        %s\n",
					sfdb->sys[sysIndex].commInfo.diskInfo[i].name);
			strcat (resBuf,tmpBuf);
			DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
		}
	}

	// S/W Process 
	//
	for (i=0; i<sfdb->sys[sysIndex].commInfo.procCnt; i++) {
		if (sfdb->sys[sysIndex].commInfo.procInfo[i].mask  == SFM_ALM_MASKED) {
			sprintf(tmpBuf,"      PROCESS          MASKED        %s\n",
					sfdb->sys[sysIndex].commInfo.procInfo[i].name);
			strcat (resBuf,tmpBuf);
			DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
		}
	}
	// LAN Status
	//
	for (i=0; i<sfdb->sys[sysIndex].commInfo.lanCnt; i++) {
		if (sfdb->sys[sysIndex].commInfo.lanInfo[i].mask == SFM_ALM_MASKED) {
			sprintf(tmpBuf,"      LAN              MASKED        %s\n",
					sfdb->sys[sysIndex].commInfo.lanInfo[i].name);
			strcat (resBuf,tmpBuf);
			DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
		}
	}
	/* by helca */
	
	// QUE Status
	for (i=0; i<SFM_MAX_QUE_CNT; i++) {
		if (sfdb->sys[sysIndex].commInfo.queInfo[i].mask == SFM_ALM_MASKED) {
			sprintf(tmpBuf,"      QUE              MASKED        Queue\n");
			strcat (resBuf,tmpBuf);
			DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
			break; // queue를 하나로 관리 masking하므로 같은 값을가진다.
		}
	}

	// RMT LAN Status
	for (i=0; i<SFM_MAX_RMT_LAN_CNT; i++) {
		if (sfdb->sys[sysIndex].commInfo.rmtLanInfo[i].mask == SFM_ALM_MASKED) {
			sprintf(tmpBuf,"      RMT_LAN          MASKED        %s\n",
					sfdb->sys[sysIndex].commInfo.rmtLanInfo[i].name);
			strcat (resBuf,tmpBuf);
			DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
		}
	}

	// OPT LAN Status
	for (i=0; i<2; i++) {
		if (sfdb->sys[sysIndex].commInfo.optLanInfo[i].mask == SFM_ALM_MASKED) {
			sprintf(tmpBuf,"      OPT_LAN          MASKED        %s\n",
					sfdb->sys[sysIndex].commInfo.optLanInfo[i].name);
			strcat (resBuf,tmpBuf);
			DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
		}
	}

	// RMT DB Status
	for (i=0; i<SFM_MAX_DB_CNT; i++) {
		if (sfdb->sys[sysIndex].commInfo.rmtDbSts[i].mask == SFM_ALM_MASKED) {
			sprintf(tmpBuf,"      RMT_DB           MASKED        %s\n",
					sfdb->sys[sysIndex].commInfo.rmtDbSts[i].sDbAlias);
			strcat (resBuf,tmpBuf);
			DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
		}
	}

	// SuccessRate
	for (i=0; i<SFM_REAL_SUCC_RATE_CNT; i++) {
		if (sfdb->sys[sysIndex].commInfo.succRate[i].mask == SFM_ALM_MASKED) {
			sprintf(tmpBuf,"      SUCCESS_RATE     MASKED        (%s)%d %d\n",
					sfdb->sys[sysIndex].commInfo.succRate[i].name,
					sfdb->sys[sysIndex].commInfo.succRate[i].cnt,
					sfdb->sys[sysIndex].commInfo.succRate[i].rate
				   );
			strcat (resBuf,tmpBuf);
			DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
		}
	}
	for(i=0; i<RADIUS_IP_CNT; i++) {
		if (sfdb->sys[sysIndex].succRateIpInfo.radius[i].mask == SFM_ALM_MASKED) {
			sprintf(tmpBuf,"      SUCCESS_RATE     MASKED        RADIUS(%s)\n",
					sfdb->sys[sysIndex].succRateIpInfo.radius[i].ipAddr);

			strcat (resBuf,tmpBuf);
			DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);

		}

	}
	
	// Duplication Sts
	if (sfdb->sys[sysIndex].commInfo.systemDup.mask == SFM_ALM_MASKED) {
		sprintf(tmpBuf,"      DUPLICATION      MASKED        HEART BEAT\n");
		strcat (resBuf,tmpBuf);
		sprintf(tmpBuf,"      DUPLICATION      MASKED        DUAL ACTIVE\n");
		strcat (resBuf,tmpBuf);
		sprintf(tmpBuf,"      DUPLICATION      MASKED        DUAL STANDBY\n");
		strcat (resBuf,tmpBuf);
		sprintf(tmpBuf,"      DUPLICATION      MASKED        DUAL STATUS TIME OUT\n");
		strcat (resBuf,tmpBuf);
		DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
	}

	// NTP
	for(i=0; i<MAX_HW_NTP; i++){

		if (sfdb->sys[sysIndex].commInfo.ntpSts[i].mask == SFM_ALM_MASKED) {
			sprintf(tmpBuf,"      NTP              MASKED        %s\n",
					ntpName[i]			
				   );
			strcat (resBuf,tmpBuf);
			DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
		}
	}
  
	
	// Rsrc
	for (i=0; i<SFM_MAX_RSRC_LOAD_CNT; i++) {
		if((i > 1 && i < 5) || (i == 9)) continue; // skip CDR, and null vlaue
		if(i > 11) break;                          // the end of RSRC

		if (sfdb->sys[sysIndex].commInfo.rsrcSts[i].mask == SFM_ALM_MASKED) {
			sprintf(tmpBuf,"      RSRC             MASKED        %s\n",
					rsrcName[i]
				   );
			strcat (resBuf,tmpBuf);
			DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
		}
	}

	// NMSIF
	for(i=0; i<SFM_NMSIF_MASK_CNT; i++) {
		if(i > 4) break;

		if(!strcasecmp(sfdb->sys[sysIndex].commInfo.name, "DSCM")) {
			if(sfdb->nmsInfo.mask[i] == SFM_ALM_MASKED) {
				sprintf(tmpBuf,"      NMSIF            MASKED        %s\n",
						nmsifName[i]
					   );
				strcat (resBuf,tmpBuf);
				DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
			}
		}
	}
    //
    // Compaq HW system specific
    //
	if (!strcasecmp (sfdb->sys[sysIndex].commInfo.type, SYSCONF_SYSTYPE_OMP))
	{
		;
	}
	else if (!strcasecmp (sfdb->sys[sysIndex].commInfo.type, SYSCONF_SYSTYPE_BSD))
	{
#if 1 // by helca
		// Compaq HW Alarm
		//
		for (i=0; i<SFM_MAX_HPUX_HW_COM; i++) {
			for (j=0; j<strlen(sfdb->sys[sysIndex].specInfo.u.sms.hpuxHWInfo.hwcom[i].name); j++)
				sfdb->sys[sysIndex].specInfo.u.sms.hpuxHWInfo.hwcom[i].name[j] = toupper(sfdb->sys[sysIndex].specInfo.u.sms.hpuxHWInfo.hwcom[i].name[j]);
			if (sfdb->sys[sysIndex].specInfo.u.sms.hpuxHWInfo.hwcom[i].mask  == SFM_ALM_MASKED) {
				sprintf(tmpBuf,"      H/W              MASKED        %s\n",
						sfdb->sys[sysIndex].specInfo.u.sms.hpuxHWInfo.hwcom[i].name);

				strcat (resBuf,tmpBuf);
				DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
			}
		}
#endif

	}

	if (!strcasecmp (sfdb->sys[sysIndex].commInfo.type, SYSCONF_SYSTYPE_BSD))
	{
    	// CPS LoadMask 
	    //
		if (sfdb->sys[sysIndex].commInfo.cpsOverSts.mask == SFM_ALM_MASKED) {
			sprintf(tmpBuf,"      CPS LOAD         MASKED          -   \n");
			strcat (resBuf,tmpBuf);
			DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
		}

	}

	/*
	* added by uamyd 20110209, LOGON 성공율 감시를 위한 구문
	*/
    if( sysIndex ){
		if( g_stLogonRate[0][sysIndex-1].mask == SFM_ALM_MASKED ){
			sprintf(tmpBuf,"     SUCC_RATE        MASKED        LOGON\n");
			//sprintf(tmpBuf,"      LOGON_SUCC_RATE  MASKED          -   \n");
			strcat (resBuf,tmpBuf);
			DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
		}
		if( g_stLogonRate[1][sysIndex-1].mask == SFM_ALM_MASKED ){
			sprintf(tmpBuf,"     SUCC_RATE        MASKED        LOGOUT\n");
			//sprintf(tmpBuf,"      LOGOUT_SUCC_RATE  MASKED          -   \n");
			strcat (resBuf,tmpBuf);
			DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
		}

		SFM_SMChInfo *pstInfo = &sfdb->sys[sysIndex].commInfo.smChSts;
		int i;
		for( i = 0; i< SFM_MAX_SM_CH_CNT; i++ ){
			if( pstInfo->each[i].mask == SFM_ALM_MASKED ){
				sprintf(tmpBuf,"     SM CONNECTION(%d) MASKED        -\n",i+1);
				strcat (resBuf,tmpBuf);
				DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
			}
		}
	    	// added by dcham 20110525 for TPS_LOAD MASK
		if (sfdb->callData.tpsInfo[sysIndex-1].mask == SFM_ALM_MASKED) {
			sprintf(tmpBuf,"      TPS LOAD         MASKED          -\n");
			strcat (resBuf,tmpBuf);
			DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
		}
	}    
	return;

} //----- End of fimd_mmc_makeSysMaskStsOutputMsg -----//

void fimd_mmc_makeSysMaskStsOutputAllMsg (
		IxpcQMsgType *rxIxpcMsg,
		int sysIndex,
		int *lineCnt,
		char *seqNo
            )
{
	int     i,j, cnt=0;
	char    tmpBuf[256], trcBuf[256], ntpName[2][10];

	strcpy(ntpName[0], "DAEMON");
	strcpy(ntpName[1], "CHANNEL"); 
	
	// CPU Mask 
	//
	sprintf(trcBuf, "      %-4s", sfdb->sys[sysIndex].commInfo.name);
	strcat(resBuf, trcBuf);
	for (i=0; i<sfdb->sys[sysIndex].commInfo.cpuCnt; i++) {
		if (sfdb->sys[sysIndex].commInfo.cpuInfo.mask[i] == SFM_ALM_MASKED) {
			if(cnt!=0)
				strcat(resBuf, "          ");

			sprintf(tmpBuf,"           CPU            MASKED        CPU%d\n", i);
			strcat (resBuf,tmpBuf);
			cnt++;
			// 한번에 40줄까지만 출력하기 위해, 40줄이면 mmcd로 결과 메시지를 보내고
			//  lineCnt를 clear한다.
			DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
		}

	}

	// Memory Mask 
	//
	if (sfdb->sys[sysIndex].commInfo.memInfo.mask == SFM_ALM_MASKED) {
		if(cnt!=0)
			strcat(resBuf, "          ");
		sprintf(tmpBuf,"           MEMORY         MASKED        Memory\n");
		strcat (resBuf,tmpBuf);
		cnt++;
		DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
	}

	// Disk Mask 
	//
	for (i=0; i<sfdb->sys[sysIndex].commInfo.diskCnt; i++) {
		if (sfdb->sys[sysIndex].commInfo.diskInfo[i].mask == SFM_ALM_MASKED) {
			if(cnt!=0)
				strcat(resBuf, "          ");
			sprintf(tmpBuf,"           DISK           MASKED        %s\n",
					sfdb->sys[sysIndex].commInfo.diskInfo[i].name);
			strcat (resBuf,tmpBuf);
			cnt++;
			DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
		}
	}

	// S/W Process 
	//
	for (i=0; i<sfdb->sys[sysIndex].commInfo.procCnt; i++) {
		if (sfdb->sys[sysIndex].commInfo.procInfo[i].mask  == SFM_ALM_MASKED) {
			if(cnt!=0)
				strcat(resBuf, "          ");
			sprintf(tmpBuf,"           PROCESS        MASKED        %s\n",
					sfdb->sys[sysIndex].commInfo.procInfo[i].name);
			strcat (resBuf,tmpBuf);
			cnt++;
			DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
		}
	}

	// LAN Status
	//
	for (i=0; i<sfdb->sys[sysIndex].commInfo.lanCnt; i++) {
		if (sfdb->sys[sysIndex].commInfo.lanInfo[i].mask == SFM_ALM_MASKED) {
			if(cnt!=0)
				strcat(resBuf, "          ");
			sprintf(tmpBuf,"           LAN            MASKED        %s\n",
					sfdb->sys[sysIndex].commInfo.lanInfo[i].name);
			strcat (resBuf,tmpBuf);
			cnt++;
			DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
		}
	}

	/* by helca */

	// QUE Status
	for (i=0; i<SFM_MAX_QUE_CNT; i++) {
		if (sfdb->sys[sysIndex].commInfo.queInfo[i].mask == SFM_ALM_MASKED) {
			if(cnt!=0)
				strcat(resBuf, "          ");
			sprintf(tmpBuf,"           QUE            MASKED        Queue\n");
			strcat (resBuf,tmpBuf);
			cnt++;
			DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
			break; // queue를 하나로 관리 masking하므로 같은 값을가진다.
		}
	}

	// RMT LAN Status
	for (i=0; i<SFM_MAX_RMT_LAN_CNT; i++) {
		if (sfdb->sys[sysIndex].commInfo.rmtLanInfo[i].mask == SFM_ALM_MASKED) {
			if(cnt!=0)
				strcat(resBuf, "          ");
			sprintf(tmpBuf,"           RMT_LAN        MASKED        %s\n",
					sfdb->sys[sysIndex].commInfo.rmtLanInfo[i].name);
			strcat (resBuf,tmpBuf);
			cnt++;
			DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
		}
	}

	// OPT LAN Status
	for (i=0; i<2; i++) {
		if (sfdb->sys[sysIndex].commInfo.optLanInfo[i].mask == SFM_ALM_MASKED) {
			if(cnt!=0)
				strcat(resBuf, "          ");
			sprintf(tmpBuf,"           OPT_LAN        MASKED        %s\n",
					sfdb->sys[sysIndex].commInfo.optLanInfo[i].name);
			strcat (resBuf,tmpBuf);
			cnt++;
			DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
		}
	}

	// RMT DB Status
	for (i=0; i<SFM_MAX_DB_CNT; i++) {
		if (sfdb->sys[sysIndex].commInfo.rmtDbSts[i].mask == SFM_ALM_MASKED) {
			if(cnt!=0)
				strcat(resBuf, "          ");
			sprintf(tmpBuf,"           RMT_DB         MASKED        %s\n",
					sfdb->sys[sysIndex].commInfo.rmtDbSts[i].sDbAlias);
			strcat (resBuf,tmpBuf);
			cnt++;
			DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
		}
	}

	// SuccessRate
	for (i=0; i<SFM_REAL_SUCC_RATE_CNT; i++) {
		if (sfdb->sys[sysIndex].commInfo.succRate[i].mask == SFM_ALM_MASKED) {
			if(cnt!=0)
				strcat(resBuf, "          ");

			sprintf(tmpBuf,"           SUCCESS_RATE   MASKED        (%s)%d %d\n",
					sfdb->sys[sysIndex].commInfo.succRate[i].name,
					sfdb->sys[sysIndex].commInfo.succRate[i].cnt,
					sfdb->sys[sysIndex].commInfo.succRate[i].rate
				   );
			strcat (resBuf,tmpBuf);
			cnt++;
			DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
		}
	}

	for(i=0; i<RADIUS_IP_CNT; i++) {
		if (sfdb->sys[sysIndex].succRateIpInfo.radius[i].mask == SFM_ALM_MASKED) {
			if(cnt!=0) strcat(resBuf, "          "); 
			sprintf(tmpBuf,"           SUCCESS_RATE   MASKED        (RADIUS)%s\n",
					sfdb->sys[sysIndex].succRateIpInfo.radius[i].ipAddr);

			strcat (resBuf,tmpBuf);
			cnt++;
			DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);

		}

	}

    // Duplication Sts

	if (sfdb->sys[sysIndex].commInfo.systemDup.mask == SFM_ALM_MASKED) {
		if(cnt!=0)
			strcat(resBuf, "          ");
		sprintf(tmpBuf,"           DUPLICATION    MASKED        HEART BEAT\n");
		strcat (resBuf,tmpBuf);
		sprintf(tmpBuf,"           DUPLICATION    MASKED        DUAL ACTIVE\n");
		strcat (resBuf,tmpBuf);
		sprintf(tmpBuf,"           DUPLICATION    MASKED        DUAL STANDBY\n");
        strcat (resBuf,tmpBuf);
		sprintf(tmpBuf,"           DUPLICATION    MASKED        DUAL STATUS TIME OUT\n");
        strcat (resBuf,tmpBuf);
		cnt++;
		DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
	}
  
	
	// Rsrc
	for (i=0; i<SFM_MAX_RSRC_LOAD_CNT; i++) {
		if((i > 1 && i < 5) || (i == 9)) continue; // skip CDR, and null vlaue
		if(i > 11) break;                          // the end of RSRC

		if (sfdb->sys[sysIndex].commInfo.rsrcSts[i].mask == SFM_ALM_MASKED) {
			if(cnt!=0)
				strcat(resBuf, "          ");
			sprintf(tmpBuf,"           RSRC           MASKED        %s\n",
					rsrcName[i]
				   );
			strcat (resBuf,tmpBuf);
			cnt++;
			DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
		}
	}
	// NTP
	for(i=0; i<MAX_HW_NTP; i++){

		if (sfdb->sys[sysIndex].commInfo.ntpSts[i].mask == SFM_ALM_MASKED) {
			sprintf(tmpBuf,"           NTP              MASKED        %s\n",
					ntpName[i]
				   );
			strcat (resBuf,tmpBuf);
			DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
		}
	}

	// NMSIF
	for(i=0; i<SFM_NMSIF_MASK_CNT; i++) {

		if(i > 4) break;

		if(!strcasecmp(sfdb->sys[sysIndex].commInfo.name, "DSCM")) {
			if(sfdb->nmsInfo.mask[i] == SFM_ALM_MASKED) {
				if(cnt!=0)
					strcat(resBuf, "          ");
				sprintf(tmpBuf,"           NMSIF          MASKED        %s\n",
						nmsifName[i]
					   );
				strcat (resBuf,tmpBuf);
				cnt++; 
				DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
			}
		}
	}
    //
    // Compaq HW system specific
    //
    if (!strcasecmp (sfdb->sys[sysIndex].commInfo.type, SYSCONF_SYSTYPE_OMP))
    {
        ;
    }
    else if (!strcasecmp (sfdb->sys[sysIndex].commInfo.type, SYSCONF_SYSTYPE_BSD))
    {
#if 1 // by helca
        // Compaq HW Alarm
        //
		for (i=0; i<SFM_MAX_HPUX_HW_COM; i++) {
            for (j=0; j<strlen(sfdb->sys[sysIndex].specInfo.u.sms.hpuxHWInfo.hwcom[i].name); j++)
				sfdb->sys[sysIndex].specInfo.u.sms.hpuxHWInfo.hwcom[i].name[j] = toupper(sfdb->sys[sysIndex].specInfo.u.sms.hpuxHWInfo.hwcom[i].name[j]);

			if (sfdb->sys[sysIndex].specInfo.u.sms.hpuxHWInfo.hwcom[i].mask  == SFM_ALM_MASKED) {
				if(cnt!=0)
					strcat(resBuf, "          ");
                sprintf(tmpBuf,"           H/W            MASKED        %s\n",
                    sfdb->sys[sysIndex].specInfo.u.sms.hpuxHWInfo.hwcom[i].name);

                strcat (resBuf,tmpBuf);
				cnt++;
                DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
            }
        }
#endif
    }

	if (!strcasecmp (sfdb->sys[sysIndex].commInfo.type, SYSCONF_SYSTYPE_BSD))
	{
		// CPS LoadMask 
		//
		if (sfdb->sys[sysIndex].commInfo.cpsOverSts.mask == SFM_ALM_MASKED) {
			if(cnt!=0)
				strcat(resBuf, "          ");
			sprintf(tmpBuf,"           CPS ROAD       MASKED          -  \n");
			strcat (resBuf,tmpBuf);
			cnt++;
			DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
		}

	}
	/* LOGON 성공율을 위한 ....added by uamyd 20110209 */
	fimd_mmc_makeLogonMaskStsOutputMsg ( sysIndex, &cnt, lineCnt );
	fimd_mmc_makeSMChStsOutputMsg ( sysIndex, &cnt, lineCnt );

	strcat(resBuf, "\n");

    return;

} //----- End of fimd_mmc_makeSysMaskStsOutputAllMsg -----//
// by helca 08.10
//
void fimd_mmc_makePDSysMaskStsOutputMsg (
            IxpcQMsgType *rxIxpcMsg,
            int sysIndex,
            int *lineCnt,
            char *seqNo
            )
{
    int     i;
    char    tmpBuf[256];
    
	if(sysIndex == TAPA) sysIndex = 0;
	else if(sysIndex == TAPB) sysIndex = 1;
	else ;

	// TAP_CPU STS
	if (l3pd->l3ProbeDev[sysIndex].cpuInfo.mask == SFM_ALM_MASKED) {
		sprintf(tmpBuf,"      TAP_CPU           MASKED        CPU(%d)\n", 0);
        strcat (resBuf,tmpBuf);
        DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
     }

	// TAP_MEM STS
	if (l3pd->l3ProbeDev[sysIndex].memInfo.mask == SFM_ALM_MASKED) {
        sprintf(tmpBuf,"      TAP_MEM           MASKED        MEM(%d)\n", 0);
        strcat (resBuf,tmpBuf);
		DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
     }
#if 0
	for(i=0; i<4; i++){
		if (l3pd->l3ProbeDev[sysIndex].fanInfo.mask[i] == SFM_ALM_MASKED) {
			sprintf(tmpBuf,"      TAP_FAN           MASKED        FAN(%d\)n", 0);
			strcat (resBuf,tmpBuf);
            DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
		}
	}
#endif
	// PD_PORT LAN STS
	for(i=0; i<MAX_GIGA_LAN_NUM; i++){  /* director 23개 sjjeon */
		if (l3pd->l3ProbeDev[sysIndex].gigaLanInfo[i].mask == SFM_ALM_MASKED) {
			sprintf(tmpBuf,"      TAP_PORT          MASKED        PORT(%d)\n", i+1);
			strcat (resBuf,tmpBuf);
			DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
		}
	}
	for(i=0; i<MAX_POWER_NUM; i++){  /* director power 대당 2개 20110424 by dcham */
		if (l3pd->l3ProbeDev[sysIndex].powerInfo[i].mask == SFM_ALM_MASKED) {
			sprintf(tmpBuf,"      TAP_POWER         MASKED        POWER(%d)\n", i+1);
			strcat (resBuf,tmpBuf);
			DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
		}           
	}
	return;
} //----- End of fimd_mmc_makePDSysMaskStsOutputMsg -----//

/*by sjjeon*/
void fimd_mmc_makeSCESysMaskStsOutputMsg (
            IxpcQMsgType *rxIxpcMsg,
            int sysIndex,
            int *lineCnt,
            char *seqNo
            )
{
    int     i;
    char    tmpBuf[256]; 
    

	bzero(tmpBuf,sizeof(tmpBuf));

	if(sysIndex == SCEA) sysIndex = 0;
	else if(sysIndex == SCEB) sysIndex = 1;
	else ;

	// SCE-STAT 
	if (g_pstSCEInfo->SCEDev[sysIndex].sysStatus.mask == SFM_ALM_MASKED) {
		sprintf(tmpBuf,"      SCE_STAT         MASKED\n");
        strcat (resBuf,tmpBuf);
        DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
     }

	// SCE-CPU STS 
	for(i=0 ; i< MAX_SCE_CPU_CNT; i++)
	{
		if (g_pstSCEInfo->SCEDev[sysIndex].cpuInfo[i].mask == SFM_ALM_MASKED) {
			sprintf(tmpBuf,"      SCE_MEM          MASKED        CPU(%d)\n", i+1);
			strcat (resBuf,tmpBuf);
			DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
		}
	}

	// SCE-DISK STS 
	if (g_pstSCEInfo->SCEDev[sysIndex].diskInfo.mask == SFM_ALM_MASKED) {
		sprintf(tmpBuf,"      SCE_DISK         MASKED\n");
		strcat (resBuf,tmpBuf);
		DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
   	}

	/* hjjung */
	// SCE-USER STS 
	if (g_pstSCEInfo->SCEDev[sysIndex].userInfo.mask == SFM_ALM_MASKED) {
		sprintf(tmpBuf,"      SCE_USER         MASKED\n");
		strcat (resBuf,tmpBuf);
		DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
   	}

	// SCE-MEM STS 
	for(i=0 ; i< MAX_SCE_MEM_CNT; i++)
	{
		if (g_pstSCEInfo->SCEDev[sysIndex].memInfo[i].mask == SFM_ALM_MASKED) {
			sprintf(tmpBuf,"      SCE_MEM          MASKED        MEM(%d) \n",i+1);
			strcat (resBuf,tmpBuf);
			DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
		}
	}

	// SCE-PORT STS 
	for(i=0 ; i< MAX_SCE_IFN_CNT; i++)
	{
		if (g_pstSCEInfo->SCEDev[sysIndex].portStatus[i].mask == SFM_ALM_MASKED) {
			if(i==0 || i==1) /* Mgmt# 0,1 */
				sprintf(tmpBuf,"      SCE_PORT         MASKED        Mgmt#%d \n",i+1);
			else{ /* LINK# 2,3,4,5 */
				if(i==2||i==4)
					sprintf(tmpBuf,"      SCE_PORT         MASKED        Link#%d_In \n",i-1);
				else
					sprintf(tmpBuf,"      SCE_PORT         MASKED        Link#%d_Out \n",i-1);

			}
			strcat (resBuf,tmpBuf);
			DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
		}
	}

	// SCE-FAN STS 
	if (g_pstSCEInfo->SCEDev[sysIndex].fanStatus.mask == SFM_ALM_MASKED) {
		sprintf(tmpBuf,"      SCE_FAN          MASKED\n");
		strcat (resBuf,tmpBuf);
		DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
   	}

	// SCE-PWR STS 
	if (g_pstSCEInfo->SCEDev[sysIndex].pwrStatus.mask == SFM_ALM_MASKED) {
		sprintf(tmpBuf,"      SCE_PWR          MASKED\n");
		strcat (resBuf,tmpBuf);
		DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
   	}

	// SCE-TEMP STS 
	if (g_pstSCEInfo->SCEDev[sysIndex].tempStatus.mask == SFM_ALM_MASKED) {
		sprintf(tmpBuf,"      SCE_TEMP         MASKED\n");
		strcat (resBuf,tmpBuf);
		DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
   	}

	// SCE-RDR-CONN STS 
#if 0	
	if (g_pstSCEInfo->SCEDev[sysIndex].rdrConnStatus.mask == SFM_ALM_MASKED) {
		sprintf(tmpBuf,"      SCE_RDR_CONN     MASKED\n");
		strcat (resBuf,tmpBuf);
		DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
   	}
#else
	for(i=0; i<2; i++){
		if (g_pstSCEInfo->SCEDev[sysIndex].rdrConnStatus[i].mask == SFM_ALM_MASKED) {
			sprintf(tmpBuf,"      SCE_RDR_CONN(%d)    MASKED\n",i+1);
			strcat (resBuf,tmpBuf);
			DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
		}
	}
#endif
	memset(trcBuf, 0x00, sizeof(trcBuf));
	memset(tmpBuf, 0x00, sizeof(tmpBuf));
	return;

} //----- End of fimd_mmc_makeSCESysMaskStsOutputMsg -----//


void fimd_mmc_makePDSysMaskStsOutputAllMsg (
            IxpcQMsgType *rxIxpcMsg,
            int sysIndex,
            int *lineCnt,
            char *seqNo
            )
{
	int     i, cnt=0;
	char    tmpBuf[256],trcBuf[256], devName[2][5];
	
	
	if(sysIndex == 0) 
		strcpy(devName[sysIndex], "TAPA");
	else 
		strcpy(devName[sysIndex], "TAPB");

	// TAP_CPU STS
	sprintf(trcBuf, "      %-4s", devName[sysIndex]);
	strcat(resBuf, trcBuf);
	
	if (l3pd->l3ProbeDev[sysIndex].cpuInfo.mask == SFM_ALM_MASKED) {
		sprintf(tmpBuf,"           TAP_CPU         MASKED        CPU\n");
		strcat (resBuf,tmpBuf);
		cnt++;
       	DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
   	}
	// TAP_MEM STS
	if (l3pd->l3ProbeDev[sysIndex].memInfo.mask == SFM_ALM_MASKED) {
		if(cnt!=0)
		strcat(resBuf, "          ");
		sprintf(tmpBuf,"           TAP_MEM         MASKED        MEM\n");
		strcat (resBuf,tmpBuf);
		cnt++;
		DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
   	}
	// TAP_PORT LAN STS
	for(i=0; i<MAX_GIGA_LAN_NUM; i++){  /* director 23개 sjjeon */
		
		if (l3pd->l3ProbeDev[sysIndex].gigaLanInfo[i].mask == SFM_ALM_MASKED) {
			if(cnt!=0)
			strcat(resBuf, "          ");
			sprintf(tmpBuf,"           TAP_PORT        MASKED        PORT%d\n", i+1);
			strcat (resBuf,tmpBuf);
			cnt++;
			DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
		}
	}
	// TAP_POWER LAN STS
	for(i=0; i<MAX_POWER_NUM; i++){  /* director pwer 대당 2개 20110424 by dcham*/

		if (l3pd->l3ProbeDev[sysIndex].powerInfo[i].mask == SFM_ALM_MASKED) {
			if(cnt!=0)
				strcat(resBuf, "          ");
			sprintf(tmpBuf,"           TAP_POWER       MASKED        PORWER%d\n", i+1);
			strcat (resBuf,tmpBuf);
			cnt++;  
			DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
		}           
	}
	/*
	   if (strlen(resBuf) > 1000) { // 3000 byte이상이면 mmcd로 결과메시지를 보낸다.
        	strcat (resBuf,"\n");
        	fimd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, 5, 1, (*seqNo)++);
        	strcpy (resBuf, resHead);
        	commlib_microSleep(500000); // 50 MS
    	}
	*/

	strcat(resBuf, "\n");
	memset(trcBuf, 0x00, sizeof(trcBuf));
	memset(tmpBuf, 0x00, sizeof(tmpBuf));
	return;

} //----- End of fimd_mmc_makePDSysMaskStsOutputMsg -----//

// sjjeon
void fimd_mmc_makeL2swSysMaskStsOutputMsg (
            IxpcQMsgType *rxIxpcMsg,
            int sysIndex,
            int *lineCnt,
            char *seqNo
            )
{
	int     i;
	char    tmpBuf[256],trcBuf[256], devName[6];

	SFM_L2SW   *pL2Info = NULL;

	if(sysIndex == L2SWA){ 
		sysIndex = 0;
		strcpy(devName, "L2SWA");
	}
	else {
		sysIndex = 1;
		strcpy(devName, "L2SWB");
	}

	pL2Info = (SFM_L2SW*)&g_pstL2Dev->l2Info[sysIndex];

	// L2 Switch CPU STS
	if (pL2Info->cpuInfo.mask == SFM_ALM_MASKED) {
		sprintf(tmpBuf,"      L2SW_CPU        MASKED        CPU\n");
		strcat (resBuf,tmpBuf);
		DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
	}
	// L2 Switch MEM STS
	if (pL2Info->memInfo.mask == SFM_ALM_MASKED) {
		sprintf(tmpBuf,"      L2SW_MEM        MASKED        MEM\n");
		strcat (resBuf,tmpBuf);
		DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
	}
	// TAP_PORT LAN STS
	for(i=0; i<MAX_L2_PORT_NUM; i++){  /* L2 Switch 24개 sjjeon */
		if (pL2Info->portInfo[i].mask == SFM_ALM_MASKED) {
			sprintf(tmpBuf,"      L2SW_PORT       MASKED        PORT%d\n", i+1);
			strcat (resBuf,tmpBuf);
			DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
		}
	}

	memset(trcBuf, 0x00, sizeof(trcBuf));
	memset(tmpBuf, 0x00, sizeof(tmpBuf));
	return;

} //----- End of fimd_mmc_makeL2swSysMaskStsOutputMsg -----//

// sjjeon All Msg
void fimd_mmc_makeL2swSysMaskStsOutputAllMsg (
            IxpcQMsgType *rxIxpcMsg,
            int sysIndex,
            int *lineCnt,
            char *seqNo
            )
{
	int     i, cnt=0;
	char    tmpBuf[256],trcBuf[256], devName[6];
	
	SFM_L2SW   *pL2Info = NULL;

	if(sysIndex == 0){ 
		strcpy(devName, "L2SWA");
	}
	else {
		strcpy(devName, "L2SWB");
	}

	sprintf(trcBuf, "        %-5s", devName);
	strcat(resBuf, trcBuf);

	pL2Info = (SFM_L2SW*)&g_pstL2Dev->l2Info[sysIndex];

	// L2 Switch CPU STS
	if (pL2Info->cpuInfo.mask == SFM_ALM_MASKED) {
		if(cnt!=0) strcat(resBuf, "             ");
		sprintf(tmpBuf,"      %10s      %8s      %6s\n","L2SW_CPU","MASKED","CPU");
		strcat (resBuf,tmpBuf);
		cnt++;
		DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
	}
	// L2 Switch MEM STS
	if (pL2Info->memInfo.mask == SFM_ALM_MASKED) {
		if(cnt!=0) strcat(resBuf, "             ");
		sprintf(tmpBuf,"      %10s      %8s      %6s\n","L2SW_MEM","MASKED","MEM");
		strcat (resBuf,tmpBuf);
		cnt++;
		DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
	}
	// L2 Switch PORT STS
	for(i=0; i<MAX_L2_PORT_NUM; i++){  /* L2 Switch 24개 sjjeon */
		if (pL2Info->portInfo[i].mask == SFM_ALM_MASKED) {
			if(cnt!=0) strcat(resBuf, "             ");
			sprintf(tmpBuf,"      %10s      %8s      %4s(%d)\n","L2SW_PORT","MASKED","PORT",i+1);
			strcat (resBuf,tmpBuf);
			cnt++;
			DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
		}
	}

	strcat(resBuf, "\n");
	memset(trcBuf, 0x00, sizeof(trcBuf));
	memset(tmpBuf, 0x00, sizeof(tmpBuf));
	return;

} //----- End of fimd_mmc_makeL2swSysMaskStsOutputAllMsg-----//

// sjjeon : SCE fimd-mmc-make
void fimd_mmc_makeSCESysMaskStsOutputAllMsg (
            IxpcQMsgType *rxIxpcMsg,
            int sysIndex,
            int *lineCnt,
            char *seqNo
            )
{
	int     i, cnt=0;
	char    tmpBuf[256],trcBuf[256], devName[2][5];

	
	if(sysIndex == 0) {
		strcpy(devName[sysIndex], "SCEA");
	}
	else {
		strcpy(devName[sysIndex], "SCEB");
		sysIndex = 1;
	}

	sprintf(trcBuf, "      %-4s", devName[sysIndex]);
	strcat(resBuf, trcBuf);

	// SCE-STAT 
	if (g_pstSCEInfo->SCEDev[sysIndex].sysStatus.mask == SFM_ALM_MASKED) {
		if(cnt!=0)
			strcat(resBuf, "          ");
		sprintf(tmpBuf,"           SCE_STAT         MASKED\n");
		strcat (resBuf,tmpBuf);
		cnt++;
		DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
   	}

	// SCE-CPU STS 
	for(i=0 ; i< MAX_SCE_CPU_CNT; i++)
	{
		if (g_pstSCEInfo->SCEDev[sysIndex].cpuInfo[i].mask == SFM_ALM_MASKED) {
			sprintf(tmpBuf,"           SCE_CPU         MASKED        CPU-%d\n", i+1);
			strcat (resBuf,tmpBuf);
			cnt++;
			DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
		}
	}

	// SCE-DISK STS 
	if (g_pstSCEInfo->SCEDev[sysIndex].diskInfo.mask == SFM_ALM_MASKED) {
		if(cnt!=0)
			strcat(resBuf, "          ");
		sprintf(tmpBuf,"           SCE_DISK         MASKED\n");
		strcat (resBuf,tmpBuf);
		cnt++;
		DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
   	}

	/* hjjung */
	// SCE-USER STS 
	if (g_pstSCEInfo->SCEDev[sysIndex].userInfo.mask == SFM_ALM_MASKED) {
		if(cnt!=0)
			strcat(resBuf, "          ");
		sprintf(tmpBuf,"           SCE_USER         MASKED\n");
		strcat (resBuf,tmpBuf);
		cnt++;
		DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
   	}

	// SCE-MEM STS 
	for(i=0 ; i< MAX_SCE_MEM_CNT; i++)
	{
		if (g_pstSCEInfo->SCEDev[sysIndex].memInfo[i].mask == SFM_ALM_MASKED) {
			if(cnt!=0)
				strcat(resBuf, "          ");
			sprintf(tmpBuf,"           SCE_MEM         MASKED      MEM-%d \n",i+1);
			strcat (resBuf,tmpBuf);
			cnt++;
			DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
		}
	}

	// Director SCE-PORT STS 
	for(i=0 ; i< MAX_SCE_IFN_CNT; i++)
	{
		if (g_pstSCEInfo->SCEDev[sysIndex].portStatus[i].mask == SFM_ALM_MASKED) {
			if(cnt!=0)
				strcat(resBuf, "          ");
			if(i==0 || i==1) /* Mgmt# 0,1 */
				sprintf(tmpBuf,"           SCE_PORT        MASKED        Mgmt#%d \n",i+1);
			else{ /* Link# 2,3,4,5 */
				if(i==2||i==4)
					sprintf(tmpBuf,"      SCE_PORT         MASKED        Link#%d_In \n",i-1);
				else
					sprintf(tmpBuf,"      SCE_PORT         MASKED        Link#%d_Out \n",i-1);
			}

			strcat (resBuf,tmpBuf);
			cnt++;
			DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
		}
	}

	// Director SCE-FAN STS 
	if (g_pstSCEInfo->SCEDev[sysIndex].fanStatus.mask == SFM_ALM_MASKED) {
		if(cnt!=0)
			strcat(resBuf, "          ");
		sprintf(tmpBuf,"           SCE_FAN         MASKED\n");
		strcat (resBuf,tmpBuf);
		cnt++;
		DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
   	}

	// Director SCE-PWR STS 
	if (g_pstSCEInfo->SCEDev[sysIndex].pwrStatus.mask == SFM_ALM_MASKED) {
		if(cnt!=0)
			strcat(resBuf, "          ");
		sprintf(tmpBuf,"           SCE_PWR         MASKED\n");
		strcat (resBuf,tmpBuf);
		cnt++;
		DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
   	}

	// Director SCE-TEMP STS 
	if (g_pstSCEInfo->SCEDev[sysIndex].tempStatus.mask == SFM_ALM_MASKED) {
		if(cnt!=0)
			strcat(resBuf, "          ");
		sprintf(tmpBuf,"           SCE_TEMP        MASKED\n");
		strcat (resBuf,tmpBuf);
		cnt++;
		DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
   	}

	// Director SCE-RDR-CONN STS 
#if 0	
	if (g_pstSCEInfo->SCEDev[sysIndex].rdrConnStatus.mask == SFM_ALM_MASKED) {
		if(cnt!=0)
			strcat(resBuf, "          ");
		sprintf(tmpBuf,"           SCE_RDR_CONN    MASKED\n");
		strcat (resBuf,tmpBuf);
		cnt++;
		DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
   	}
#else
	for(i=0;i<MAX_SCE_RDR_INFO_CNT; i++){
		if (g_pstSCEInfo->SCEDev[sysIndex].rdrConnStatus[i].mask == SFM_ALM_MASKED) {
			if(cnt!=0)
				strcat(resBuf, "          ");
			sprintf(tmpBuf,"           SCE_RDR_CONN(%d)   MASKED\n", i+1);
			strcat (resBuf,tmpBuf);
			cnt++;
			DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
		}
	}
#endif

	strcat(resBuf, "\n");
	memset(trcBuf, 0x00, sizeof(trcBuf));
	memset(tmpBuf, 0x00, sizeof(tmpBuf));
	return;

} //----- End of fimd_mmc_makeSCESysMaskStsOutputAllMsg-----//

/* hjjung */
void fimd_mmc_makeLEGSysMaskStsOutputAllMsg (
            IxpcQMsgType *rxIxpcMsg,
            int sysIndex,
            int *lineCnt,
            char *seqNo
            )
{
	int     cnt=0;
	char    tmpBuf[256],trcBuf[256], devName[2][5];

	
	if(sysIndex == 0) {
		strcpy(devName[sysIndex], "SCMA");
	}
	else {
		strcpy(devName[sysIndex], "SCMB");
		sysIndex = 1;
	}

	sprintf(trcBuf, "      %-4s", devName[sysIndex]);
	strcat(resBuf, trcBuf);

	// RLEG-SESSION STS 
	if (g_pstCALLInfo->legInfo[sysIndex].mask == SFM_ALM_MASKED) {
		if(cnt!=0)
			strcat(resBuf, "          ");
		sprintf(tmpBuf,"           RLEG         MASKED\n");
		strcat (resBuf,tmpBuf);
		cnt++;
		DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
   	}

	strcat(resBuf, "\n");
	memset(trcBuf, 0x00, sizeof(trcBuf));
	memset(tmpBuf, 0x00, sizeof(tmpBuf));
	return;

} //----- End of fimd_mmc_makeLEGSysMaskStsOutputAllMsg-----//


int fimd_mmc_dis_ppd (IxpcQMsgType *rxIxpcMsg)
{
    strcpy (resBuf,"\n      TYPE      STATUS\n      --------------------\n");

	strcat (resBuf,"      SPEAKER   ");

	if(*sound_flag == 1)
		strcat(resBuf, "ACTIVE\n");
	else
		strcat(resBuf, "INACTIVE\n");

	strcat(resBuf, "\n    RESULT = SUCCESS\n\n");

    fimd_txMMLResult (rxIxpcMsg, resBuf, 0, 0, 0, 0, 1);

	return 1;
}

int fimd_mmc_act_ppd (IxpcQMsgType *rxIxpcMsg)
{
	char argVal[32];
	MMLReqMsgType	*rxMmlReqMsg;

	rxMmlReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	strcpy (argVal,  rxMmlReqMsg->head.para[0].paraVal);

	if(strcasecmp(argVal, "SPK") == 0) {
		strcpy (resBuf,"\n      TYPE   = SPEAKER\n");

		strcat (resBuf,"\n    RESULT = SUCCESS\n\n");

		if( *sound_flag == 1) 
			strcat (resBuf,"      PRE_STS = ALREADY_ACTIVE\n");
		
		strcat (resBuf,"      CUR_STS = SPEAKER ACTIVE\n\n");

		fimd_backupAudio2File ();
		fimd_ActAlmEvent2Client ();
		*sound_flag = 1;
		fimd_txMMLResult (rxIxpcMsg, resBuf, 0, 0, 0, 0, 1);
	}

	return 1;
}

int fimd_mmc_dact_ppd (IxpcQMsgType *rxIxpcMsg)
{
	char argVal[32];
	MMLReqMsgType	*rxMmlReqMsg;

	rxMmlReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	strcpy (argVal,  rxMmlReqMsg->head.para[0].paraVal);

	if(strcasecmp(argVal, "SPK") == 0) {
		strcpy (resBuf,"\n      TYPE   = SPEAKER\n");

		strcat (resBuf,"\n    RESULT = SUCCESS\n\n");

		if( *sound_flag == 0) 
			strcat (resBuf,"      PRE_STS = ALREADY_INACTIVE\n");
		
		strcat (resBuf,"      CUR_STS = SPEAKER INACTIVE\n\n");

		fimd_backupAudio2File ();
		fimd_DactAlmEvent2Client ();
		*sound_flag = 0;
		fimd_txMMLResult (rxIxpcMsg, resBuf, 0, 0, 0, 0, 1);
	}

	return 1;
}

int alm_lmt_dsc_input(void)
{

	char fname[30];
	char	*env;
	FILE *fp;

	if ((env = getenv(IV_HOME)) == NULL){
		sprintf(trcBuf,"[%s] getenv fail\n", __FUNCTION__);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	sprintf(fname, "%s/%s", env, FILE_DSC_LIMIT); // by helca 07.31

	fp = fopen(fname, "w+");
	fprintf(fp, "# system_name cpu, mem, disk, queue, rsrc등이 min, maj, cri 순서로 limit, durat 저장\n"
				"DSCM %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n"
				"SCMA %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n"
				"SCMB %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
				 sfdb->sys[0].commInfo.cpuInfo.minLimit, sfdb->sys[0].commInfo.cpuInfo.majLimit, sfdb->sys[0].commInfo.cpuInfo.criLimit, 
				 sfdb->sys[0].commInfo.cpuInfo.minDurat, sfdb->sys[0].commInfo.cpuInfo.majDurat, sfdb->sys[0].commInfo.cpuInfo.criDurat, 
				 sfdb->sys[0].commInfo.memInfo.minLimit, sfdb->sys[0].commInfo.memInfo.majLimit, sfdb->sys[0].commInfo.memInfo.criLimit, 
				 sfdb->sys[0].commInfo.memInfo.minDurat, sfdb->sys[0].commInfo.memInfo.majDurat, sfdb->sys[0].commInfo.memInfo.criDurat, 
				 sfdb->sys[0].commInfo.diskInfo[0].minLimit, sfdb->sys[0].commInfo.diskInfo[0].majLimit, sfdb->sys[0].commInfo.diskInfo[0].criLimit, 
				 sfdb->sys[0].commInfo.queInfo[0].minLimit, sfdb->sys[0].commInfo.queInfo[0].majLimit, sfdb->sys[0].commInfo.queInfo[0].criLimit, 
				 sfdb->sys[0].commInfo.rsrcSts[0].minLimit, sfdb->sys[0].commInfo.rsrcSts[0].majLimit, sfdb->sys[0].commInfo.rsrcSts[0].criLimit, 
				 sfdb->sys[0].commInfo.rsrcSts[0].minDurat, sfdb->sys[0].commInfo.rsrcSts[0].majDurat, sfdb->sys[0].commInfo.rsrcSts[0].criDurat,

				 sfdb->sys[1].commInfo.cpuInfo.minLimit, sfdb->sys[1].commInfo.cpuInfo.majLimit, sfdb->sys[1].commInfo.cpuInfo.criLimit, 
				 sfdb->sys[1].commInfo.cpuInfo.minDurat, sfdb->sys[1].commInfo.cpuInfo.majDurat, sfdb->sys[1].commInfo.cpuInfo.criDurat, 
				 sfdb->sys[1].commInfo.memInfo.minLimit, sfdb->sys[1].commInfo.memInfo.majLimit, sfdb->sys[1].commInfo.memInfo.criLimit, 
				 sfdb->sys[1].commInfo.memInfo.minDurat, sfdb->sys[1].commInfo.memInfo.majDurat, sfdb->sys[1].commInfo.memInfo.criDurat, 
				 sfdb->sys[1].commInfo.diskInfo[0].minLimit, sfdb->sys[1].commInfo.diskInfo[0].majLimit, sfdb->sys[1].commInfo.diskInfo[0].criLimit, 
				 sfdb->sys[1].commInfo.queInfo[0].minLimit, sfdb->sys[1].commInfo.queInfo[0].majLimit, sfdb->sys[1].commInfo.queInfo[0].criLimit, 
				 sfdb->sys[1].commInfo.rsrcSts[0].minLimit, sfdb->sys[1].commInfo.rsrcSts[0].majLimit, sfdb->sys[1].commInfo.rsrcSts[0].criLimit, 
				 sfdb->sys[1].commInfo.rsrcSts[0].minDurat, sfdb->sys[1].commInfo.rsrcSts[0].majDurat, sfdb->sys[1].commInfo.rsrcSts[0].criDurat,
	
				 sfdb->sys[2].commInfo.cpuInfo.minLimit, sfdb->sys[2].commInfo.cpuInfo.majLimit, sfdb->sys[2].commInfo.cpuInfo.criLimit, 
				 sfdb->sys[2].commInfo.cpuInfo.minDurat, sfdb->sys[2].commInfo.cpuInfo.majDurat, sfdb->sys[2].commInfo.cpuInfo.criDurat, 
				 sfdb->sys[2].commInfo.memInfo.minLimit, sfdb->sys[2].commInfo.memInfo.majLimit, sfdb->sys[2].commInfo.memInfo.criLimit, 
				 sfdb->sys[2].commInfo.memInfo.minDurat, sfdb->sys[2].commInfo.memInfo.majDurat, sfdb->sys[2].commInfo.memInfo.criDurat, 
				 sfdb->sys[2].commInfo.diskInfo[0].minLimit, sfdb->sys[2].commInfo.diskInfo[0].majLimit, sfdb->sys[2].commInfo.diskInfo[0].criLimit, 
				 sfdb->sys[2].commInfo.queInfo[0].minLimit, sfdb->sys[2].commInfo.queInfo[0].majLimit, sfdb->sys[2].commInfo.queInfo[0].criLimit, 
				 sfdb->sys[2].commInfo.rsrcSts[0].minLimit, sfdb->sys[2].commInfo.rsrcSts[0].majLimit, sfdb->sys[2].commInfo.rsrcSts[0].criLimit, 
				 sfdb->sys[2].commInfo.rsrcSts[0].minDurat, sfdb->sys[2].commInfo.rsrcSts[0].majDurat, sfdb->sys[2].commInfo.rsrcSts[0].criDurat
			    );
	if(fp)fclose(fp);
	return 0;
}

/*
TAP Alarm limit을 저장..
 */
int alm_lmt_pd_input(void)
{

	char fname[30];
	char	*env;
	FILE *fp;

	if ((env = getenv(IV_HOME)) == NULL){
		sprintf(trcBuf,"[%s] getenv fail\n", __FUNCTION__);
		trclib_writeLogErr (FL,trcBuf);
		return -1 ;
	}

	sprintf(fname, "%s/%s", env,FILE_TAP_LIMIT); // by helca 07.31

	fp = fopen(fname, "w+");
	fprintf(fp, "# system_name cpu, mem등이 min, maj, cri 순서로 limit, durat 저장\n"
				"TAPA %d %d %d %d %d %d %d %d %d %d %d %d\n"
				"TAPB %d %d %d %d %d %d %d %d %d %d %d %d\n",

				 l3pd->l3ProbeDev[0].cpuInfo.minLimit, l3pd->l3ProbeDev[0].cpuInfo.majLimit, l3pd->l3ProbeDev[0].cpuInfo.criLimit,
				 l3pd->l3ProbeDev[0].cpuInfo.minDurat, l3pd->l3ProbeDev[0].cpuInfo.majDurat, l3pd->l3ProbeDev[0].cpuInfo.criDurat,
				 l3pd->l3ProbeDev[0].memInfo.minLimit, l3pd->l3ProbeDev[0].memInfo.majLimit, l3pd->l3ProbeDev[0].memInfo.criLimit,
				 l3pd->l3ProbeDev[0].memInfo.minDurat, l3pd->l3ProbeDev[0].memInfo.majDurat, l3pd->l3ProbeDev[0].memInfo.criDurat,

				 l3pd->l3ProbeDev[1].cpuInfo.minLimit, l3pd->l3ProbeDev[1].cpuInfo.majLimit, l3pd->l3ProbeDev[1].cpuInfo.criLimit,
				 l3pd->l3ProbeDev[1].cpuInfo.minDurat, l3pd->l3ProbeDev[1].cpuInfo.majDurat, l3pd->l3ProbeDev[1].cpuInfo.criDurat,
				 l3pd->l3ProbeDev[1].memInfo.minLimit, l3pd->l3ProbeDev[1].memInfo.majLimit, l3pd->l3ProbeDev[1].memInfo.criLimit,
				 l3pd->l3ProbeDev[1].memInfo.minDurat, l3pd->l3ProbeDev[1].memInfo.majDurat, l3pd->l3ProbeDev[1].memInfo.criDurat
			     
				 
				 );
	if(fp)fclose(fp);
	return 0;
}

/* by june '09.05\4.15 */
int alm_lmt_sce_input(void)
{

	char 	fname[30];
	char	*env;
	FILE 	*fp;

	if ((env = getenv(IV_HOME)) == NULL){
		sprintf(trcBuf,"[%s] getenv fail\n", __FUNCTION__);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	sprintf(fname, "%s/%s", env, FILE_SCE_LIMIT);


	printf("%d\n", g_pstSCEInfo->SCEDev[0].userInfo.criLimit);
	printf("# system_name cpu, mem등이 min, maj, cri 순서로 limit, durat 저장\n"
				"SCEA %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d" 	/* SCEA + 42 */
				" %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d"
				" %d %d %d %d %d %d %d %d %d %d %d %d\n"
				"SCEB %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d"	/* SCEB + 42 */
				" %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d"
				" %d %d %d %d %d %d %d %d %d %d %d %d\n",

				 g_pstSCEInfo->SCEDev[0].cpuInfo[0].minLimit, g_pstSCEInfo->SCEDev[0].cpuInfo[0].majLimit, g_pstSCEInfo->SCEDev[0].cpuInfo[0].criLimit,
				 g_pstSCEInfo->SCEDev[0].cpuInfo[0].minDurat, g_pstSCEInfo->SCEDev[0].cpuInfo[0].majDurat, g_pstSCEInfo->SCEDev[0].cpuInfo[0].criDurat,
				 g_pstSCEInfo->SCEDev[0].cpuInfo[1].minLimit, g_pstSCEInfo->SCEDev[0].cpuInfo[1].majLimit, g_pstSCEInfo->SCEDev[0].cpuInfo[1].criLimit,
				 g_pstSCEInfo->SCEDev[0].cpuInfo[1].minDurat, g_pstSCEInfo->SCEDev[0].cpuInfo[1].majDurat, g_pstSCEInfo->SCEDev[0].cpuInfo[1].criDurat,
				 g_pstSCEInfo->SCEDev[0].cpuInfo[2].minLimit, g_pstSCEInfo->SCEDev[0].cpuInfo[2].majLimit, g_pstSCEInfo->SCEDev[0].cpuInfo[2].criLimit,
				 g_pstSCEInfo->SCEDev[0].cpuInfo[2].minDurat, g_pstSCEInfo->SCEDev[0].cpuInfo[2].majDurat, g_pstSCEInfo->SCEDev[0].cpuInfo[2].criDurat,

				 g_pstSCEInfo->SCEDev[0].memInfo[0].minLimit, g_pstSCEInfo->SCEDev[0].memInfo[0].majLimit, g_pstSCEInfo->SCEDev[0].memInfo[0].criLimit,
				 g_pstSCEInfo->SCEDev[0].memInfo[0].minDurat, g_pstSCEInfo->SCEDev[0].memInfo[0].majDurat, g_pstSCEInfo->SCEDev[0].memInfo[0].criDurat,
				 g_pstSCEInfo->SCEDev[0].memInfo[1].minLimit, g_pstSCEInfo->SCEDev[0].memInfo[1].majLimit, g_pstSCEInfo->SCEDev[0].memInfo[1].criLimit,
				 g_pstSCEInfo->SCEDev[0].memInfo[1].minDurat, g_pstSCEInfo->SCEDev[0].memInfo[1].majDurat, g_pstSCEInfo->SCEDev[0].memInfo[1].criDurat,
				 g_pstSCEInfo->SCEDev[0].memInfo[2].minLimit, g_pstSCEInfo->SCEDev[0].memInfo[2].majLimit, g_pstSCEInfo->SCEDev[0].memInfo[2].criLimit,
				 g_pstSCEInfo->SCEDev[0].memInfo[2].minDurat, g_pstSCEInfo->SCEDev[0].memInfo[2].majDurat, g_pstSCEInfo->SCEDev[0].memInfo[2].criDurat,

				 g_pstSCEInfo->SCEDev[0].diskInfo.minLimit,   g_pstSCEInfo->SCEDev[0].diskInfo.majLimit,   g_pstSCEInfo->SCEDev[0].diskInfo.criLimit,
				 g_pstSCEInfo->SCEDev[0].diskInfo.minDurat,   g_pstSCEInfo->SCEDev[0].diskInfo.majDurat,   g_pstSCEInfo->SCEDev[0].diskInfo.criDurat,

				 /* hjjung */
				 g_pstSCEInfo->SCEDev[0].userInfo.minLimit,   g_pstSCEInfo->SCEDev[0].userInfo.majLimit,   g_pstSCEInfo->SCEDev[0].userInfo.criLimit,
				 g_pstSCEInfo->SCEDev[0].userInfo.minDurat,   g_pstSCEInfo->SCEDev[0].userInfo.majDurat,   g_pstSCEInfo->SCEDev[0].userInfo.criDurat,
				 
				 g_pstSCEInfo->SCEDev[1].cpuInfo[0].minLimit, g_pstSCEInfo->SCEDev[1].cpuInfo[0].majLimit, g_pstSCEInfo->SCEDev[1].cpuInfo[0].criLimit,
				 g_pstSCEInfo->SCEDev[1].cpuInfo[0].minDurat, g_pstSCEInfo->SCEDev[1].cpuInfo[0].majDurat, g_pstSCEInfo->SCEDev[1].cpuInfo[0].criDurat,
				 g_pstSCEInfo->SCEDev[1].cpuInfo[1].minLimit, g_pstSCEInfo->SCEDev[1].cpuInfo[1].majLimit, g_pstSCEInfo->SCEDev[1].cpuInfo[1].criLimit,
				 g_pstSCEInfo->SCEDev[1].cpuInfo[1].minDurat, g_pstSCEInfo->SCEDev[1].cpuInfo[1].majDurat, g_pstSCEInfo->SCEDev[1].cpuInfo[1].criDurat,
				 g_pstSCEInfo->SCEDev[1].cpuInfo[2].minLimit, g_pstSCEInfo->SCEDev[1].cpuInfo[2].majLimit, g_pstSCEInfo->SCEDev[1].cpuInfo[2].criLimit,
				 g_pstSCEInfo->SCEDev[1].cpuInfo[2].minDurat, g_pstSCEInfo->SCEDev[1].cpuInfo[2].majDurat, g_pstSCEInfo->SCEDev[1].cpuInfo[2].criDurat,

				 g_pstSCEInfo->SCEDev[1].memInfo[0].minLimit, g_pstSCEInfo->SCEDev[1].memInfo[0].majLimit, g_pstSCEInfo->SCEDev[1].memInfo[0].criLimit,
				 g_pstSCEInfo->SCEDev[1].memInfo[0].minDurat, g_pstSCEInfo->SCEDev[1].memInfo[0].majDurat, g_pstSCEInfo->SCEDev[1].memInfo[0].criDurat,
				 g_pstSCEInfo->SCEDev[1].memInfo[1].minLimit, g_pstSCEInfo->SCEDev[1].memInfo[1].majLimit, g_pstSCEInfo->SCEDev[1].memInfo[1].criLimit,
				 g_pstSCEInfo->SCEDev[1].memInfo[1].minDurat, g_pstSCEInfo->SCEDev[1].memInfo[1].majDurat, g_pstSCEInfo->SCEDev[1].memInfo[1].criDurat,
				 g_pstSCEInfo->SCEDev[1].memInfo[2].minLimit, g_pstSCEInfo->SCEDev[1].memInfo[2].majLimit, g_pstSCEInfo->SCEDev[1].memInfo[2].criLimit,
				 g_pstSCEInfo->SCEDev[1].memInfo[2].minDurat, g_pstSCEInfo->SCEDev[1].memInfo[2].majDurat, g_pstSCEInfo->SCEDev[1].memInfo[2].criDurat,

				 g_pstSCEInfo->SCEDev[1].diskInfo.minLimit,   g_pstSCEInfo->SCEDev[1].diskInfo.majLimit,   g_pstSCEInfo->SCEDev[1].diskInfo.criLimit,
				 g_pstSCEInfo->SCEDev[1].diskInfo.minDurat,   g_pstSCEInfo->SCEDev[1].diskInfo.majDurat,   g_pstSCEInfo->SCEDev[1].diskInfo.criDurat,

				 /* hjjung */
				 g_pstSCEInfo->SCEDev[1].userInfo.minLimit,   g_pstSCEInfo->SCEDev[1].userInfo.majLimit,   g_pstSCEInfo->SCEDev[1].userInfo.criLimit,
				 g_pstSCEInfo->SCEDev[1].userInfo.minDurat,   g_pstSCEInfo->SCEDev[1].userInfo.majDurat,   g_pstSCEInfo->SCEDev[1].userInfo.criDurat
			);


	fp = fopen(fname, "w+");
	
	fprintf(fp, "# system_name cpu, mem등이 min, maj, cri 순서로 limit, durat 저장\n"
				"SCEA %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d" 	/* SCEA + 42 */
				" %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d"
				" %d %d %d %d %d %d %d %d %d %d %d %d\n"
				"SCEB %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d"	/* SCEB + 42 */
				" %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d"
				" %d %d %d %d %d %d %d %d %d %d %d %d\n",

				 g_pstSCEInfo->SCEDev[0].cpuInfo[0].minLimit, g_pstSCEInfo->SCEDev[0].cpuInfo[0].majLimit, g_pstSCEInfo->SCEDev[0].cpuInfo[0].criLimit,
				 g_pstSCEInfo->SCEDev[0].cpuInfo[0].minDurat, g_pstSCEInfo->SCEDev[0].cpuInfo[0].majDurat, g_pstSCEInfo->SCEDev[0].cpuInfo[0].criDurat,
				 g_pstSCEInfo->SCEDev[0].cpuInfo[1].minLimit, g_pstSCEInfo->SCEDev[0].cpuInfo[1].majLimit, g_pstSCEInfo->SCEDev[0].cpuInfo[1].criLimit,
				 g_pstSCEInfo->SCEDev[0].cpuInfo[1].minDurat, g_pstSCEInfo->SCEDev[0].cpuInfo[1].majDurat, g_pstSCEInfo->SCEDev[0].cpuInfo[1].criDurat,
				 g_pstSCEInfo->SCEDev[0].cpuInfo[2].minLimit, g_pstSCEInfo->SCEDev[0].cpuInfo[2].majLimit, g_pstSCEInfo->SCEDev[0].cpuInfo[2].criLimit,
				 g_pstSCEInfo->SCEDev[0].cpuInfo[2].minDurat, g_pstSCEInfo->SCEDev[0].cpuInfo[2].majDurat, g_pstSCEInfo->SCEDev[0].cpuInfo[2].criDurat,

				 g_pstSCEInfo->SCEDev[0].memInfo[0].minLimit, g_pstSCEInfo->SCEDev[0].memInfo[0].majLimit, g_pstSCEInfo->SCEDev[0].memInfo[0].criLimit,
				 g_pstSCEInfo->SCEDev[0].memInfo[0].minDurat, g_pstSCEInfo->SCEDev[0].memInfo[0].majDurat, g_pstSCEInfo->SCEDev[0].memInfo[0].criDurat,
				 g_pstSCEInfo->SCEDev[0].memInfo[1].minLimit, g_pstSCEInfo->SCEDev[0].memInfo[1].majLimit, g_pstSCEInfo->SCEDev[0].memInfo[1].criLimit,
				 g_pstSCEInfo->SCEDev[0].memInfo[1].minDurat, g_pstSCEInfo->SCEDev[0].memInfo[1].majDurat, g_pstSCEInfo->SCEDev[0].memInfo[1].criDurat,
				 g_pstSCEInfo->SCEDev[0].memInfo[2].minLimit, g_pstSCEInfo->SCEDev[0].memInfo[2].majLimit, g_pstSCEInfo->SCEDev[0].memInfo[2].criLimit,
				 g_pstSCEInfo->SCEDev[0].memInfo[2].minDurat, g_pstSCEInfo->SCEDev[0].memInfo[2].majDurat, g_pstSCEInfo->SCEDev[0].memInfo[2].criDurat,

				 g_pstSCEInfo->SCEDev[0].diskInfo.minLimit,   g_pstSCEInfo->SCEDev[0].diskInfo.majLimit,   g_pstSCEInfo->SCEDev[0].diskInfo.criLimit,
				 g_pstSCEInfo->SCEDev[0].diskInfo.minDurat,   g_pstSCEInfo->SCEDev[0].diskInfo.majDurat,   g_pstSCEInfo->SCEDev[0].diskInfo.criDurat,

				 /* hjjung */
				 g_pstSCEInfo->SCEDev[0].userInfo.minLimit,   g_pstSCEInfo->SCEDev[0].userInfo.majLimit,   g_pstSCEInfo->SCEDev[0].userInfo.criLimit,
				 g_pstSCEInfo->SCEDev[0].userInfo.minDurat,   g_pstSCEInfo->SCEDev[0].userInfo.majDurat,   g_pstSCEInfo->SCEDev[0].userInfo.criDurat,
				 
				 g_pstSCEInfo->SCEDev[1].cpuInfo[0].minLimit, g_pstSCEInfo->SCEDev[1].cpuInfo[0].majLimit, g_pstSCEInfo->SCEDev[1].cpuInfo[0].criLimit,
				 g_pstSCEInfo->SCEDev[1].cpuInfo[0].minDurat, g_pstSCEInfo->SCEDev[1].cpuInfo[0].majDurat, g_pstSCEInfo->SCEDev[1].cpuInfo[0].criDurat,
				 g_pstSCEInfo->SCEDev[1].cpuInfo[1].minLimit, g_pstSCEInfo->SCEDev[1].cpuInfo[1].majLimit, g_pstSCEInfo->SCEDev[1].cpuInfo[1].criLimit,
				 g_pstSCEInfo->SCEDev[1].cpuInfo[1].minDurat, g_pstSCEInfo->SCEDev[1].cpuInfo[1].majDurat, g_pstSCEInfo->SCEDev[1].cpuInfo[1].criDurat,
				 g_pstSCEInfo->SCEDev[1].cpuInfo[2].minLimit, g_pstSCEInfo->SCEDev[1].cpuInfo[2].majLimit, g_pstSCEInfo->SCEDev[1].cpuInfo[2].criLimit,
				 g_pstSCEInfo->SCEDev[1].cpuInfo[2].minDurat, g_pstSCEInfo->SCEDev[1].cpuInfo[2].majDurat, g_pstSCEInfo->SCEDev[1].cpuInfo[2].criDurat,

				 g_pstSCEInfo->SCEDev[1].memInfo[0].minLimit, g_pstSCEInfo->SCEDev[1].memInfo[0].majLimit, g_pstSCEInfo->SCEDev[1].memInfo[0].criLimit,
				 g_pstSCEInfo->SCEDev[1].memInfo[0].minDurat, g_pstSCEInfo->SCEDev[1].memInfo[0].majDurat, g_pstSCEInfo->SCEDev[1].memInfo[0].criDurat,
				 g_pstSCEInfo->SCEDev[1].memInfo[1].minLimit, g_pstSCEInfo->SCEDev[1].memInfo[1].majLimit, g_pstSCEInfo->SCEDev[1].memInfo[1].criLimit,
				 g_pstSCEInfo->SCEDev[1].memInfo[1].minDurat, g_pstSCEInfo->SCEDev[1].memInfo[1].majDurat, g_pstSCEInfo->SCEDev[1].memInfo[1].criDurat,
				 g_pstSCEInfo->SCEDev[1].memInfo[2].minLimit, g_pstSCEInfo->SCEDev[1].memInfo[2].majLimit, g_pstSCEInfo->SCEDev[1].memInfo[2].criLimit,
				 g_pstSCEInfo->SCEDev[1].memInfo[2].minDurat, g_pstSCEInfo->SCEDev[1].memInfo[2].majDurat, g_pstSCEInfo->SCEDev[1].memInfo[2].criDurat,

				 g_pstSCEInfo->SCEDev[1].diskInfo.minLimit,   g_pstSCEInfo->SCEDev[1].diskInfo.majLimit,   g_pstSCEInfo->SCEDev[1].diskInfo.criLimit,
				 g_pstSCEInfo->SCEDev[1].diskInfo.minDurat,   g_pstSCEInfo->SCEDev[1].diskInfo.majDurat,   g_pstSCEInfo->SCEDev[1].diskInfo.criDurat,

				 /* hjjung */
				 g_pstSCEInfo->SCEDev[1].userInfo.minLimit,   g_pstSCEInfo->SCEDev[1].userInfo.majLimit,   g_pstSCEInfo->SCEDev[1].userInfo.criLimit,
				 g_pstSCEInfo->SCEDev[1].userInfo.minDurat,   g_pstSCEInfo->SCEDev[1].userInfo.majDurat,   g_pstSCEInfo->SCEDev[1].userInfo.criDurat
			);
	if(fp)fclose(fp);
	return 0;
}

/* hjjung */
int alm_lmt_leg_input(void)
{

	char 	fname[30];
	char	*env;
	FILE 	*fp;

	if ((env = getenv(IV_HOME)) == NULL){
		sprintf(trcBuf,"[%s] getenv fail\n", __FUNCTION__);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	sprintf(fname, "%s/%s", env,FILE_SESS_LIMIT);

	fp = fopen(fname, "w+");
	
	fprintf(fp, "# system_name session이 min, maj, cri 순서로 limit, durat 저장\n"
				"SCMA %d %d %d %d %d %d\n" 
				"SCMB %d %d %d %d %d %d\n",

				 g_pstCALLInfo->legInfo[0].minLimit,   g_pstCALLInfo->legInfo[0].majLimit,   g_pstCALLInfo->legInfo[0].criLimit,
				 g_pstCALLInfo->legInfo[0].minDurat,   g_pstCALLInfo->legInfo[0].majDurat,   g_pstCALLInfo->legInfo[0].criDurat,

				 g_pstCALLInfo->legInfo[1].minLimit,   g_pstCALLInfo->legInfo[1].majLimit,   g_pstCALLInfo->legInfo[1].criLimit,
				 g_pstCALLInfo->legInfo[1].minDurat,   g_pstCALLInfo->legInfo[1].majDurat,   g_pstCALLInfo->legInfo[1].criDurat
			);
	if(fp)fclose(fp);
	return 0;
}

/* added by dcham 20110525 */
int alm_lmt_tps_input(void)
{

	char 	fname[30];
	char	*env;
	FILE 	*fp;

	if ((env = getenv(IV_HOME)) == NULL){
		sprintf(trcBuf,"[%s] getenv fail\n", __FUNCTION__);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	sprintf(fname, "%s/%s", env,FILE_TPS_LIMIT);

	fp = fopen(fname, "w+");
	
        fprintf(fp, "# system_name session이 min, maj, cri 순서로 limit, durat 저장\n"
                "SCMA %d %d %d %d %d %d\n" 
                "SCMB %d %d %d %d %d %d\n",
                
                 g_pstCALLInfo->tpsInfo[0].minLimit,   g_pstCALLInfo->tpsInfo[0].majLimit,   g_pstCALLInfo->tpsInfo[0].criLimit,
                 g_pstCALLInfo->tpsInfo[0].minDurat,   g_pstCALLInfo->tpsInfo[0].majDurat,   g_pstCALLInfo->tpsInfo[0].criDurat,
            
                 g_pstCALLInfo->tpsInfo[1].minLimit,   g_pstCALLInfo->tpsInfo[1].majLimit,   g_pstCALLInfo->tpsInfo[1].criLimit,
                 g_pstCALLInfo->tpsInfo[1].minDurat,   g_pstCALLInfo->tpsInfo[1].majDurat,   g_pstCALLInfo->tpsInfo[1].criDurat
            );	
        if(fp)fclose(fp);
	return 0;
}
/*
by sjjeon
L2 Switch alarm limit 설정.
 */
int alm_lmt_l2sw_input(void)
{

	char fname[30];
	char	*env;
	FILE *fp;

	if ((env = getenv(IV_HOME)) == NULL){
		sprintf(trcBuf,"[%s] getenv fail\n", __FUNCTION__);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	sprintf(fname, "%s/%s", env,FILE_L2SW_LIMIT); // by helca 07.31

	fp = fopen(fname, "w+");
	fprintf(fp, "# system_name cpu, mem의 min, maj, cri 순서로 limit, durat 저장\n"
				"L2SWA %d %d %d %d %d %d %d %d %d %d %d %d\n"
				"L2SWB %d %d %d %d %d %d %d %d %d %d %d %d\n",

				 g_pstL2Dev->l2Info[0].cpuInfo.minLimit, g_pstL2Dev->l2Info[0].cpuInfo.majLimit, g_pstL2Dev->l2Info[0].cpuInfo.criLimit,
				 g_pstL2Dev->l2Info[0].cpuInfo.minDurat, g_pstL2Dev->l2Info[0].cpuInfo.majDurat, g_pstL2Dev->l2Info[0].cpuInfo.criDurat,
				 g_pstL2Dev->l2Info[0].memInfo.minLimit, g_pstL2Dev->l2Info[0].memInfo.majLimit, g_pstL2Dev->l2Info[0].memInfo.criLimit,
				 g_pstL2Dev->l2Info[0].memInfo.minDurat, g_pstL2Dev->l2Info[0].memInfo.majDurat, g_pstL2Dev->l2Info[0].memInfo.criDurat,

				 g_pstL2Dev->l2Info[1].cpuInfo.minLimit, g_pstL2Dev->l2Info[1].cpuInfo.majLimit, g_pstL2Dev->l2Info[1].cpuInfo.criLimit,
				 g_pstL2Dev->l2Info[1].cpuInfo.minDurat, g_pstL2Dev->l2Info[1].cpuInfo.majDurat, g_pstL2Dev->l2Info[1].cpuInfo.criDurat,
				 g_pstL2Dev->l2Info[1].memInfo.minLimit, g_pstL2Dev->l2Info[1].memInfo.majLimit, g_pstL2Dev->l2Info[1].memInfo.criLimit,
				 g_pstL2Dev->l2Info[1].memInfo.minDurat, g_pstL2Dev->l2Info[1].memInfo.majDurat, g_pstL2Dev->l2Info[1].memInfo.criDurat
			     
				 
				 );
	if(fp)fclose(fp);
	return 0;
}

/* SM Connection Status */
void fimd_mmc_makeSMChStsOutputMsg(int sysIndex, int *cnt, int *lineCnt)
{
    char          tmpBuf[256];
	int           chid;
    SFM_SMChInfo *pstInfo = &sfdb->sys[sysIndex].commInfo.smChSts;
	for( chid = 0; chid < SFM_MAX_SM_CH_CNT; chid++ ){
		if( pstInfo->each[chid].mask == SFM_ALM_MASKED ){
			if( *cnt != 0 ) strcat(resBuf, "          ");
			sprintf(tmpBuf,"       SM CONNECTION(%d)   MASKED        -\n",chid+1);
			strcat (resBuf,tmpBuf);
			(*cnt)++;
			DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
		}
	}
    return;

}

/*
* LOGON 통계를 위해서 DIS-MASK ...를 위한 함수
* added by uamyd 20110209
*/
void fimd_mmc_makeLogonMaskStsOutputMsg (int sysIndex, int *cnt, int *lineCnt)
{
    char    tmpBuf[256];
    
	if( g_stLogonRate[0][sysIndex-1].mask == SFM_ALM_MASKED ){
		if( *cnt != 0 ) strcat(resBuf, "          ");
		sprintf(tmpBuf,"         SUCC_RATE        MASKED        LOGON\n");
		//sprintf(tmpBuf,"      LOGOON_SUCC_RATE MASKED\n");
        strcat (resBuf,tmpBuf);
		(*cnt)++;
        DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
	}

	if( g_stLogonRate[1][sysIndex-1].mask == SFM_ALM_MASKED ){
		if( *cnt != 0 ) strcat(resBuf, "          ");
		sprintf(tmpBuf,"         SUCC_RATE        MASKED        LOGOUT\n");
		//sprintf(tmpBuf,"      LOGOUT_SUCC_RATE MASKED\n");
        strcat (resBuf,tmpBuf);
		(*cnt)++;
        DIS_ALM_STS_CHECK_LINE_CNT(*lineCnt);
	}

    return;
}

/*
* LOGON 성공율 감시를 위해서 limit 값을 저장하기 위한 함수
* added by uamyd 20110210
*/
int alm_lmt_logon_success_rate_input(void)
{

	char fname[30];
	char *env;
	FILE *fp;
	int  log_mod;

	if ((env = getenv(IV_HOME)) == NULL){
		sprintf(trcBuf,"[%s] getenv fail\n", __FUNCTION__);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	sprintf(fname, "%s/%s", env,FILE_LOGON_LIMIT);

	fp = fopen(fname, "w+");
	fprintf(fp, "# type, sysName, min, maj, cri 순서로 limit 저장\n");
	for( log_mod = 0; log_mod < LOG_MOD_CNT; log_mod++){
		fprintf(fp, "%s SCMA %d %d %d \n"
					"%s SCMB %d %d %d \n",
					!log_mod?"LOGON":"LOGOUT",
					g_stLogonRate[log_mod][0].minLimit,
					g_stLogonRate[log_mod][0].majLimit,
					g_stLogonRate[log_mod][0].criLimit,
					!log_mod?"LOGON":"LOGOUT",
					g_stLogonRate[log_mod][1].minLimit,
					g_stLogonRate[log_mod][1].majLimit,
					g_stLogonRate[log_mod][1].criLimit);
	}
	if(fp)fclose(fp);
	return 0;
}

/*
* LOGON 성공율 감시를 위해서 변경되는 threshold 의 validation check를 위한 함수
* added by uamyd 20110210
*/
int fimd_mmc_checkLogonSuccessRateAlmLimitValidation( int sysIndex, int log_mod, int level, int limit )
{
	g_pstLogonRate = &g_stLogonRate[log_mod][sysIndex-1];

    switch( level ){
        case SFM_ALM_MINOR:
            if( limit <= g_pstLogonRate->majLimit ){
                sprintf(resBuf,"    RESULT = FAIL\n    REASON = INVALID LIMIT VALUE(IS EQUAL TO OR LESS THAN MAJOR(%d))\n\n", g_pstLogonRate->majLimit);
                return -1; 
            }   
            break;

        case SFM_ALM_MAJOR:
            if( limit <= g_pstLogonRate->criLimit ){
                sprintf(resBuf,"    RESULT = FAIL\n    REASON = INVALID LIMIT VALUE(IS EQUAL TO OR LESS THAN CRITICAL(%d))\n\n", g_pstLogonRate->criLimit);
                return -1; 
            }   
            if( limit >= g_pstLogonRate->minLimit ){
                sprintf(resBuf,"    RESULT = FAIL\n    REASON = INVALID LIMIT VALUE(IS EQUAL TO OR MORE THAN MINOR(%d))\n\n", g_pstLogonRate->minLimit);
                return -1; 
            }   
            break;

        case SFM_ALM_CRITICAL:
            if( limit >= g_pstLogonRate->majLimit ){
                sprintf(resBuf,"    RESULT = FAIL\n    REASON = INVALID LIMIT VALUE(IS EQUAL TO OR MORE THAN MAJOR(%d))\n\n", g_pstLogonRate->majLimit);
                return -1; 
            }   
            break;
    }   

    return 0;
}

/*
* LOGON 성공율 감시 threshold change 함수
* added by uamyd 20110210
*/
int fimd_mmc_updateLogonSuccessRateAlmClsValue (int sysIndex,  int log_mod, int level, int limit )
{
	g_pstLogonRate = &g_stLogonRate[log_mod][sysIndex-1];
    switch( level ){
        case SFM_ALM_MINOR:
            g_pstLogonRate->minLimit = limit;
            break;

        case SFM_ALM_MAJOR:
            g_pstLogonRate->majLimit = limit;
            break;

        case SFM_ALM_CRITICAL:
            g_pstLogonRate->criLimit = limit;
            break;
    }   

	alm_lmt_logon_success_rate_input(); //file save
    return 0;
}

int alm_lmt_sm_ch_input(void)
{

	char fname[30];
	char *env;
	FILE *fp;
	SFM_SMChInfo *pstInfo1 = &sfdb->sys[1].commInfo.smChSts;
	SFM_SMChInfo *pstInfo2 = &sfdb->sys[2].commInfo.smChSts;

	if ((env = getenv(IV_HOME)) == NULL){
		sprintf(trcBuf,"[%s] getenv fail\n", __FUNCTION__);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	sprintf(fname, "%s/%s", env, FILE_SMCONN_LIMIT);

	fp = fopen(fname, "w+");
	fprintf(fp, "# system, min, maj, cri 순서로 limit 저장\n");
	fprintf(fp, "SCMA %d %d %d \n"
				"SCMB %d %d %d \n",
				pstInfo1->minLimit,
				pstInfo1->majLimit,
				pstInfo1->criLimit,
				pstInfo2->minLimit,
				pstInfo2->majLimit,
				pstInfo2->criLimit);
	if(fp)fclose(fp);
	return 0;
}

int fimd_mmc_checkSMChAlmLimitValidation( int sysIndex, int level, int limit )
{
	SFM_SMChInfo *pstInfo = &sfdb->sys[sysIndex].commInfo.smChSts;

    switch( level ){
        case SFM_ALM_MINOR:
            if( limit >= pstInfo->majLimit ){
                sprintf(resBuf,"    RESULT = FAIL\n    REASON = INVALID LIMIT VALUE(IS EQUAL TO OR MORE THAN MAJOR(%d))\n\n", pstInfo->majLimit);
                return -1; 
            }   
            break;

        case SFM_ALM_MAJOR:
            if( limit >= pstInfo->criLimit ){
                sprintf(resBuf,"    RESULT = FAIL\n    REASON = INVALID LIMIT VALUE(IS EQUAL TO OR MORE THAN CRITICAL(%d))\n\n", pstInfo->criLimit);
                return -1; 
            }   
            if( limit <= pstInfo->minLimit ){
                sprintf(resBuf,"    RESULT = FAIL\n    REASON = INVALID LIMIT VALUE(IS EQUAL TO OR LESS THAN MINOR(%d))\n\n", pstInfo->minLimit);
                return -1; 
            }   
            break;

        case SFM_ALM_CRITICAL:
            if( limit <= pstInfo->majLimit ){
                sprintf(resBuf,"    RESULT = FAIL\n    REASON = INVALID LIMIT VALUE(IS EQUAL TO OR LESS THAN MAJOR(%d))\n\n", pstInfo->majLimit);
                return -1; 
            }   
            break;
    }   

    return 0;
}

/*
* SM Connection Status 감시 threshold change 함수
* added by uamyd 20110210
*/
int fimd_mmc_updateSMChAlmClsValue (int sysIndex, int level, int limit )
{
	SFM_SMChInfo *pstInfo = &sfdb->sys[sysIndex].commInfo.smChSts;
    switch( level ){
        case SFM_ALM_MINOR:
            pstInfo->minLimit = limit;
            break;

        case SFM_ALM_MAJOR:
            pstInfo->majLimit = limit;
            break;

        case SFM_ALM_CRITICAL:
            pstInfo->criLimit = limit;
            break;
    }   

	alm_lmt_sm_ch_input(); //file save
    return 0;
}
