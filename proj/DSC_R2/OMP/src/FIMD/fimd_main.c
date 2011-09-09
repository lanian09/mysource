#include "fimd_proto.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

int g_dStopFlag = 0;
int	fimdQid, ixpcQid, condQid, nmsifQid, eqSysCnt=0;
int	dataPortNum, eventPortNum, statFLAG=0, dupChkTime, dualActStdFlag[SYSCONF_MAX_ASSO_SYS_NUM]={0, };
int	wtEndIdx, prevWorkTime=0;
int     WTtable[24];
char    perTimeReport[64], systemModel[5];
char	mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN], sysLabel[COMM_MAX_NAME_LEN];
char	sfdbFileName[256], audioFileName[256], l3pdFileName[256], trcBuf[4096], trcTmp[1024];
char 	sceFileName[256], szl2swFileName[256], legFileName[256], callFileName[256];
char	logonFileName[256], smChFileName[256];
time_t	currentTime, ixpc_rec_Time[SYSCONF_MAX_ASSO_SYS_NUM], samd_rec_Time[SYSCONF_MAX_ASSO_SYS_NUM];
time_t  alarm_Time[SYSCONF_MAX_ASSO_SYS_NUM][RADIUS_IP_CNT] = { {0,0}, };
time_t  alarm_dev_Time[SYSCONF_MAX_ASSO_SYS_NUM];
time_t  dual_sts_alarm_Time[SYSCONF_MAX_ASSO_SYS_NUM];
int  g_hwswType[SFM_MAX_HPUX_HW_COM]; // HW/SW Type 설정..

SFM_sfdb	*sfdb;
SFM_L3PD	*l3pd;
SFM_L2Dev   *g_pstL2Dev;
SFM_SCE     *g_pstSCEInfo;
/* hjjung */
//SFM_LEG       *g_pstLEGInfo;
SFM_CALL       *g_pstCALLInfo; // added by dcham 20110525 for TPS
SFM_LOGON   g_stLogonRate[LOG_MOD_CNT][2], *g_pstLogonRate;;
ListenInfo	*ne_info;

FimdHwAlmCheckBuff	cpuChkBuff[SYSCONF_MAX_ASSO_SYS_NUM][SFM_MAX_CPU_CNT];
FimdHwAlmCheckBuff	memChkBuff[SYSCONF_MAX_ASSO_SYS_NUM];
FimdHwAlmCheckBuff	diskChkBuff[SYSCONF_MAX_ASSO_SYS_NUM][SFM_MAX_DISK_CNT];

STM_SysFltStat		almStat[SYSCONF_MAX_ASSO_SYS_NUM];
FimdClientContext	cliTbl[SOCKLIB_MAX_CLIENT_CNT];
FimdKeepAlive		fimdKeepAlive[MAX_KEEPALIVE];

int	dbSyncFailCnt[SYSCONF_MAX_ASSO_SYS_NUM];
int	dbDeadCnt[SYSCONF_MAX_ASSO_SYS_NUM];

static char		WTperiod[64];

int     active_sys_arr[2];

// by helca 09.29 mmc 명령어중 dis_ppd, act_ppd, dact_ppd는 사용하지 않음. 
// GUI client가 접속해온 경우 fimd_DactAlmEvent2Client가 실행 되지 않도록 sound_flag를 1로 초기화함. 

int		*sound_flag; 
int 	stsIndex[3] = {0,0,0};
int		first_flag = 0;
int     chkCnt2[SYSCONF_MAX_ASSO_SYS_NUM][RADIUS_IP_CNT] = { {0,0} };
void	handleChildProcess(int);

extern 	ClientSockFdContext	clientSockFdTbl[SOCKLIB_MAX_CLIENT_CNT];
extern 	int	trcFlag, trcLogFlag;
extern 	int trcLogId, trcErrLogId;

FimdMmcHdlrVector   mmcHdlrVector[FIMD_MAX_MMC_HANDLER] = 
{
	{"mask-alm",        fimd_mmc_mask_alm},     
	{"umask-alm",       fimd_mmc_umask_alm},    
	{"dis-mask-sts",    fimd_mmc_dis_mask_sts},
	{"dis-alm-lmt",     fimd_mmc_dis_alm_lmt},  
	{"set-alm-lmt",     fimd_mmc_set_alm_lmt},
	/* hjjung dis-session-lmt, set-session-lmt 추가 */
	{"dis-session-lmt", fimd_mmc_dis_session_lmt},  
	{"set-session-lmt", fimd_mmc_set_session_lmt},
	{"dis-alm-sts",     fimd_mmc_dis_alm_sts},
	{"set-svc-alm",		fimd_mmc_set_svc_alm},
	{"dis-svc-alm",		fimd_mmc_dis_svc_alm},
	{"audit-alm",   	fimd_mmc_audit_alm},
	{"dis-ppd",         fimd_mmc_dis_ppd},
	{"act-ppd",         fimd_mmc_act_ppd},
	{"dact-ppd",        fimd_mmc_dact_ppd},
	{"stop-aud-alm",    fimd_mmc_stop_aud_alm},
	{"dis-tps-lmt",    fimd_mmc_dis_tps_lmt}, // added by dcham 20110525  for dis-tps-lmt/set-tps-lmt 
	{"set-tps-lmt",    fimd_mmc_set_tps_lmt}
	//{"dis-call-info",   fimd_mmc_dis_call_info}
};

// added by dcham 15->17 for stop-aud-alm, 20110525 
int	numMmcHdlr=17; 

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main (int ac, char *av[])
{
	int	ret, actFd, loopCnt=0;
	int	tickPerSec=200; // 1초당 loop를 몇번 도는지 횟수; cpu 성능에 따라 달라진다.
	int	sysIndex=0, check_Index;

	SockLibMsgType	rxSockMsg;
	GeneralQMsgType	rxGenQMsg;
	time_t  curTime, prevTime;

   	if((check_Index = check_my_run_status("FIMD")) < 0)
       	exit(0);
	if (fimd_initial() < 0) {
		fprintf(stderr,">>>>>> fimd_initial fail\n");
		return -1;
	}

	// clear previous messages
	while (msgrcv(fimdQid, &rxGenQMsg, sizeof(rxGenQMsg), 0, IPC_NOWAIT) > 0);

	//trcLogFlag = 1;

	curTime = prevTime = time(0);
	memset(ixpc_rec_Time, 0, SYSCONF_MAX_ASSO_SYS_NUM);
	memset(samd_rec_Time, 0, SYSCONF_MAX_ASSO_SYS_NUM);
	active_sys_arr[0] = active_sys_arr[1] = 0;

	memset(&rxGenQMsg, 0, sizeof(GeneralQMsgType));
	
	while (!g_dStopFlag)
	{

		while ((ret = msgrcv(fimdQid, &rxGenQMsg, sizeof(rxGenQMsg), 0, IPC_NOWAIT)) > 0) {
			fimd_exeRxQMsg (&rxGenQMsg);
			memset(&rxGenQMsg, 0, sizeof(GeneralQMsgType));
		}
		if ((ret < 0) && (errno == EINVAL || errno == EFAULT)) {
			sprintf(trcBuf,"[fimd_main] >>> msgrcv fail; err=%d(%s)\n", errno, strerror(errno));
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}
		// GUI와 연결된 socket port에서의 event를 처리한다.
		//
		ret = socklib_action ((char*)&rxSockMsg, &actFd);
		switch (ret)
		{
			// GUI로부터 요구 메시지를 수신한 경우
			case SOCKLIB_CLIENT_MSG_RECEIVED:
				fimd_exeRxSockMsg (actFd, &rxSockMsg);
				break;
			case SOCKLIB_NEW_CONNECTION:
				// client table에 추가한다.
				fimd_exeNewConn (actFd);
				break;
			case SOCKLIB_CLIENT_DISCONNECTED:
				// client table에서 삭제한다.
				fimd_exeDisconn (actFd);
				break;
		}
		signal(SIGCHLD, handleChildProcess); // 041004.lndb.cjs for <defunct>
		
		// 주기적으로 GUI로 sfdb를 보내야 하는데,
		// - 주기적인 전송은 client의 state가 ACTIVE인 놈으로만 보낸다.
		// - 한번에 모두 보내지 않고 시스템별로 나누어 보낸다.
		// - 1초내에 각 시스템을 일정한 시간 간격으로 보낸다.
		if ((++loopCnt) % (tickPerSec*2/eqSysCnt) == 0) 
		{
			fimd_broadcastSfdb2Client (sysIndex);
			sysIndex = (sysIndex+1) % eqSysCnt;
			keepalivelib_increase();
			
			// yhshin
			//fimd_checkKeepAlive();
			
			// yhshin OMP--> MP process
			fimd_checkSysAlive(); 

			//yhshin discret_dupl_status();
			currentTime = time(0);

			/*system status check*/
			fimd_checkStatReportTime();

			/*NMS 상태체크*/
			fimd_check_NMS_status(sysIndex);	
		}
		
		// 주기적으로 current_alarm DB에 있는 잘못된 정보를 삭제한다.
		if (loopCnt % (tickPerSec*30) == 0) {
			fimd_checkCurrentAlarm ();
			
		}
		
		// 주기적(1시간)으로 alarm_history DB에 있는 오래된 내용을 삭제한다.
		if (loopCnt % (tickPerSec*3600) == 0) {
		 	fimd_deleteOldAlmInfoDB ();
		}

	} //-- end of while(1) --//

	FinishProgram();
	return 0;
	
} //----- End of main -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int fimd_exeRxQMsg (GeneralQMsgType *rxGenQMsg)
{
	int	ret=1, i;
	
	char    currentStamp2[32];
	time_t      now;
	struct tm   *pLocalTime;
	now = time(0);
	pLocalTime = (struct tm*)localtime((time_t*)&now);
	strftime (currentStamp2, 32, "%Y-%m-%d %H:%M:%S", pLocalTime);

	IxpcQMsgType *rxIxpcMsg;

	rxIxpcMsg = (IxpcQMsgType *)rxGenQMsg->body;
#if 0
   	// sjjeon log
	printf("mtype: %d, GeneralQMsgType body-Len : %d\n", 
			rxGenQMsg->mtype, rxIxpcMsg->head.bodyLen);
	fprintf(stderr, 
	"Proc=%d, Disk=%d Lan=%d Que=%d Link=%d Dupli=%d, Rate=%d BOND=%d LOAD=%d SYSSTS=%d DUIA=%d\n",
	sizeof(SFM_SysCommProcSts),sizeof(SFM_SysCommDiskSts), sizeof(SFM_SysCommLanSts),
	sizeof(SFM_SysCommQueSts), sizeof(SFM_SysCommLinkSts), sizeof(SFM_SysDuplicationSts),
	sizeof(SFM_SysSuccessRate), sizeof(SFM_SysIFBond), sizeof(SFM_SysRsrcLoad),
	sizeof(SFM_SysSts), sizeof(SFM_Duia));
#endif

	for(i = 0; i < SYSCONF_MAX_ASSO_SYS_NUM; i++){
        	if( !strcasecmp(sfdb->sys[i].commInfo.name, rxIxpcMsg->head.srcSysName)){
				ixpc_rec_Time[i] = time(0);
				if(!strcasecmp(rxIxpcMsg->head.srcAppName, "SAMD")){
					samd_rec_Time[i] = time(0);
				}
				break;
			}
	}

	switch (rxGenQMsg->mtype) 
	{
		case MTYPE_SETPRINT:
			ret = trclib_exeSetPrintMsg ((TrcLibSetPrintMsgType*)rxGenQMsg);
			break;
		case MTYPE_STATUS_REPORT:
			ret = fimd_exeStsRptMsg ((IxpcQMsgType*)rxGenQMsg->body);
			break;
		case MTYPE_ALARM_REPORT:
			ret = fimd_exeAlmRptMsg ((IxpcQMsgType*)rxGenQMsg->body);
			break;
		case MTYPE_MMC_REQUEST:
			ret = fimd_exeMMCMsg ((IxpcQMsgType*)rxGenQMsg->body);
			break;
		case MTYPE_STATISTICS_REQUEST:
			fimd_reportSysStatData2OMP ((IxpcQMsgType*)rxGenQMsg->body);
			break;
		case MTYPE_STATISTICS_REPORT:
			/* hjjung_20100822 */
			// ret = fimd_exeStatRptMsg ((IxpcQMsgType*)rxGenQMsg->body);
			break;
		case MTYPE_DUP_STATUS_REQUEST:
			sprintf(trcBuf,"[fimd_exeRxQMsg] received MTYPE_DUP_STATUS_REQUEST\n");
			trclib_writeLogErr (FL,trcBuf);
			ret = fimd_exeDupStatusRpt ((IxpcQMsgType*)rxGenQMsg->body);
			break;
		case MTYPE_DUP_UPDATE_NOTI:
			ret = fimd_exeDupUpdateNoti ((IxpcQMsgType*)rxGenQMsg->body);
			break;
		default:
			sprintf(trcBuf,"[fimd_exeRxQMsg] received unknown mtype(%d)\n",(int)rxGenQMsg->mtype);
			trclib_writeLogErr (FL,trcBuf);
			ret = -1;
		
		return -1;
	}

	return 1;

} //----- End of fimd_exeRxQMsg -----//



//------------------------------------------------------------------------------
// 각 시스템에서 통보되는 상태 정보 메시지를 처리한다.
//------------------------------------------------------------------------------
int fimd_exeStsRptMsg (IxpcQMsgType *rxIxpcMsg)
{

	switch (rxIxpcMsg->head.msgId) 
	{
		// 모든 시스템에서 공통으로 관리되는 상태정보 report - SAMD
		case MSGID_SYS_COMM_STATUS_REPORT:      //300
			fimd_hdlSysCommStsRpt (rxIxpcMsg);
			break;
		case MSGID_SYS_L3PD_STATUS_REPORT:		//304
			fimd_hdll3pdStsRpt (rxIxpcMsg);
			break;
		/* by june */
		case MSGID_SYS_SCE_STATUS_REPORT:		// 305
			fimd_hdleSCEStsRpt (rxIxpcMsg);
			break;
		case MSGID_SYS_L2_STATUS_REPORT:		// 306
			fimd_hdleL2StsRpt (rxIxpcMsg);
			break;

		/* by uamyd 20110208 */
		case MSGID_LOGON_STATISTICS_REPORT:		// 115
			fimd_hdleLogonSuccessRate (rxIxpcMsg);

			break;
		/* hjjung_20100822 */
		case MSGID_CALL_REPORT:					// 116
			fimd_exeStatRptMsg (rxIxpcMsg);
			fimd_hdleLEGStsRpt (rxIxpcMsg);
#ifdef _TPS_
			fimd_hdleTPSStsRpt (rxIxpcMsg); // added by dcham 2011.05.25 for TPS
			break;
#endif
		default:
			sprintf(trcBuf,"[fimd_exeStsRptMsg] unknown status_report msg_id(%d)\n", rxIxpcMsg->head.msgId);
			trclib_writeLogErr (FL,trcBuf);
			break;
	}

	fimd_keepAliveIncrease(rxIxpcMsg->head.msgId, rxIxpcMsg->head.srcSysName);

	return 1;

} //----- End of fimd_exeStsRptMsg -----//

//------------------------------------------------------------------------------
// 각 시스템에서 통보되는 장애 통보 메시지를 처리한다.
//------------------------------------------------------------------------------
int fimd_exeAlmRptMsg (IxpcQMsgType *rxIxpcMsg)
{
	

	switch (rxIxpcMsg->head.msgId) {
		default:
			sprintf(trcBuf,"[fimd_exeAlmRptMsg] unknown alarm_report msg_id(%d)\n", rxIxpcMsg->head.msgId);
			trclib_writeLogErr (FL,trcBuf);
		break;
	}

	return 1;

} //----- End of fimd_exeAlmRptMsg -----//

//------------------------------------------------------------------------------
// MMCD에서 명령어를 수신한 경우
//------------------------------------------------------------------------------
int fimd_exeMMCMsg (IxpcQMsgType *rxIxpcMsg)
{
	MMLReqMsgType		*mmlReqMsg;
	FimdMmcHdlrVector	*mmcHdlr;

	mmlReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
	if ((mmcHdlr = (FimdMmcHdlrVector*) bsearch (
					mmlReqMsg->head.cmdName,
					mmcHdlrVector,
					numMmcHdlr,
					sizeof(FimdMmcHdlrVector),
					fimd_mmcHdlrVector_bsrchCmp)) == NULL) {

		sprintf(trcBuf,"[fimd_exeMMCMsg] received unknown mml_cmd(%s)\n", mmlReqMsg->head.cmdName);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}
	
	// 처리 function을 호출한다.
	//
	(int)(*(mmcHdlr->func)) (rxIxpcMsg);
	return 1;

} //----- End of fimd_exeMMCMsg -----//

/*
*	Logon 통계 감시를 위한 함수
*   added by uamyd 20110208
*/
int fimd_hdleLogonSuccessRate (IxpcQMsgType *rxIxpcMsg)
{
	STAT_LOGON_RATE *pstLogonRate = NULL;
	int	             log_mod, sysIndex = 0, changeFlag = 0, almChkFlag = ENABLE_REQ; /* 0x01 */

	if ((sysIndex = fimd_getSysIndexByName (rxIxpcMsg->head.srcSysName)) < 0) {
		sprintf(trcBuf,"[fimd_exeStatRptMsg] unknown sysName[%s]\n", rxIxpcMsg->head.srcSysName);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	for( log_mod = 0; log_mod < LOG_MOD_CNT; log_mod++ ){
	
		if( !log_mod ){
			pstLogonRate = (STAT_LOGON_RATE*)rxIxpcMsg->body;
		}else{
			pstLogonRate = (STAT_LOGON_RATE*)&rxIxpcMsg->body[sizeof(STAT_LOGON_RATE)];
		}

		if( !strncasecmp(pstLogonRate->szSysName,"SCMA",4) ){
			sysIndex = 1; //SCMA
		} else {
			sysIndex = 2; //SCMB
		}
		g_pstLogonRate = &g_stLogonRate[log_mod][sysIndex-1]; // minus 1, for index

		/** Request 의 값이 0이거나, Deactive SCM 장비인 경우에 한하여, flag를 설정하여 전달한다.
			Request 의 값이 0인 경우에는, 장애등급은 NORMAL로 판정되어야 한다.
			Deactive SCM 장비인 경우는 일단 보류됨. noted by uamyd 20110222
	#define ENABLE_REQ   0x01
	#define DISABLE_REQ  0x02
	#define ACTIVE_DEV   0x10
	#define DEACTIVE_DEV 0x20
		**/
		if( !pstLogonRate->request ){
			almChkFlag = DISABLE_REQ;
		}
#ifdef DEBUG
        sprintf(trcBuf,"SYSTEM=%d:%s, Log_Mod=%d rate=%d success=%d request=%d\n",
                sysIndex, rxIxpcMsg->head.srcSysName, log_mod, 
				pstLogonRate->rate, pstLogonRate->success, pstLogonRate->request);
		trclib_writeLogErr(FL, trcBuf);
#endif

		if( g_pstLogonRate->mask != SFM_ALM_MASKED ){
			if( fimd_checkLogonSuccessRateAlm( sysIndex, log_mod, pstLogonRate->rate, almChkFlag ) ){
				changeFlag = 1;
				sprintf(trcBuf,"[%s] status chagned. LOGON STAT, level=%d\n", __FUNCTION__,g_pstLogonRate->level);
				trclib_writeLogErr (FL, trcBuf);
			}
		}

		if( changeFlag ){
			fimd_backupLogon2File(); //원래 최대 Alarm 값을 구해서 저장하는 로직이 보통 들어가지만, SM, OMP 시스템 외에 작동 안함.
			fimd_broadcastAlmEvent2Client();
		}
	}/* for, log_mod 별로 2번 연속해서 감시하도록 한다. by uamyd 20110424 */

	return 1;

} //----- End of fimd_exeStatRptMsg -----//

/* hjjung_20100823 */
//------------------------------------------------------------------------------
// RLEG 에서 통보되는 통계 정보 메시지를 처리한다.(CPS)
//------------------------------------------------------------------------------
int fimd_exeStatRptMsg (IxpcQMsgType *rxIxpcMsg)
{
	CALL_DATA * pCallData = NULL;
	int	sysIndex=0;

	if ((sysIndex = fimd_getSysIndexByName (rxIxpcMsg->head.srcSysName)) < 0) {
		sprintf(trcBuf,"[fimd_exeStatRptMsg] unknown sysName[%s]\n", rxIxpcMsg->head.srcSysName);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	if (sfdb->sys[sysIndex].commInfo.systemDup.myStatus == 1) {
		pCallData = (CALL_DATA *)rxIxpcMsg->body;
		if(g_pstCALLInfo!=NULL) { // modified by dcham 20110530 for g_pstLEGInfo->g_pstCALLInfo
			g_pstCALLInfo->cps.uiLogOnSumCps = ntohl(pCallData->cps.uiLogOnSumCps);
			g_pstCALLInfo->cps.uiLogOutSumCps= ntohl(pCallData->cps.uiLogOutSumCps);
		    sprintf(trcBuf,"[fimd_main] CPS_ON:[%d], CPS_OUT[%d]\n",g_pstCALLInfo->cps.uiLogOnSumCps,g_pstCALLInfo->cps.uiLogOutSumCps);
		    trclib_writeLogErr (FL,trcBuf);
		}
	}
	return 1;

} //----- End of fimd_exeStatRptMsg -----//

//------------------------------------------------------------------------------
// GUI client가 접속해온 경우 호출되어 client table에 추가한다.
//------------------------------------------------------------------------------
int fimd_exeNewConn (int fd)
{
	int	i, bindPortNum;
	char	cliAddr[32];
   	int     txLen;
   	SockLibMsgType  txSockMsg;
   	IxpcQMsgType rxIxpcMsg;

	// 접속해온 GUI client의 ip_adress와 어떤 binding port로 접속한 놈인지 정보를
	//	얻기위해 socklib에서 관리하는 clientSockFdTbl을 검색한다.
	//
	for (i=0; i<SOCKLIB_MAX_CLIENT_CNT; i++) {
		if (clientSockFdTbl[i].fd == fd)
			break;
	}
	if (i >= SOCKLIB_MAX_CLIENT_CNT) {
		sprintf(trcBuf,"[fimd_exeNewConn] not found fd[%d] in clientSockFdTbl\n", fd);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}
	
	sprintf (cliAddr, "%s", inet_ntoa(clientSockFdTbl[i].cliAddr.sin_addr));
	bindPortNum = clientSockFdTbl[i].port;
	
	// cliTbl에서 비어있는 index를 찾는다.
	//
	for (i=0; i<SOCKLIB_MAX_CLIENT_CNT; i++) {
		if (cliTbl[i].sockFd == 0)
			break;
	}
	if (i >= SOCKLIB_MAX_CLIENT_CNT) {
		socklib_disconnectSockFd (fd);
		sprintf(trcBuf,"[fimd_exeNewConn] can't add client any more to cliTbl; cliAddr=%s, fd=%d\n", cliAddr, fd);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	// cliTbl에 정보를 저장한다.
	//
	cliTbl[i].sockFd      = fd;
	cliTbl[i].bindPortNum = bindPortNum;
	cliTbl[i].connectTime = currentTime;
	cliTbl[i].state = FIMD_CLIENT_STATE_CONNECTED;
	strcpy (cliTbl[i].cliAddr, cliAddr);

	sprintf(trcBuf,"[fimd_exeNewConn] connected new client; cliAddr=%s, fd=%d\n",
			cliAddr, fd);
	trclib_writeLog(FL,trcBuf);

	if (cliTbl[i].bindPortNum == eventPortNum) {
		memset(txSockMsg.body, 0x00, sizeof(txSockMsg.body));
		if(strlen(sfdb->active_sys_name) <= 0)
			strcpy (sfdb->active_sys_name, "ACTIVE");

		//yhshin
		strcpy (txSockMsg.body, sfdb->active_sys_name);
		txSockMsg.head.mapType = (MAPTYPE_ACTIVE_ALARM);// active side(2), audio stop(1), event alarm(0)
		txSockMsg.head.segFlag = 0;
		txSockMsg.head.seqNo = 1;
		txSockMsg.head.bodyLen = (strlen(txSockMsg.body));
		txLen = sizeof(txSockMsg.head) + strlen(txSockMsg.body); //txSockMsg.head.bodyLen;

//		if (socklib_sndMsg (cliTbl[i].sockFd, (char*)&txSockMsg, txLen) < 0) {
		if (socklib_sndMsg_hdr_chg (cliTbl[i].sockFd, (char*)&txSockMsg, txLen) < 0) {
			sprintf(trcBuf,"[fimd_exeNewConn] socklib_sndMsg fail to %s(fd=%d)\n",
       				cliTbl[i].cliAddr, cliTbl[i].sockFd);
			trclib_writeLogErr (FL,trcBuf);
			// client table에서 삭제한다.
			memset ((void*)&cliTbl[i], 0, sizeof(FimdClientContext));
			return -1;
		}

		memset(&rxIxpcMsg, 0, sizeof(IxpcQMsgType));

		if(*sound_flag == 0)
			fimd_DactAlmEvent2Client ();
		else
			fimd_ActAlmEvent2Client ();

	}

	// GUI와 새로운 Connection이 이뤄 졌을시 alarm_event noti를 준다. 
	fimd_broadcastAlmEvent2Client ();
	fimd_checkCurrentAlarm ();		
	return 1;

} //----- End of fimd_exeNewConn -----//

//------------------------------------------------------------------------------
// GUI client와 접속된 connection이 끊어진 경우 호출되어 client table에서 삭제한다.
//------------------------------------------------------------------------------
int fimd_exeDisconn (int fd)
{
	int	cliIndex;
	

	// 접속이 끊어진 client를 검색한다.
	//
	if ((cliIndex = fimd_getCliIndex (fd)) < 0) {
		sprintf(trcBuf,"[fimd_exeDisconn] not found fd(%d) in cliTbl\n", fd);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}
	
	sprintf(trcBuf,"[fimd_exeDisconn] disconnected client; cliAddr=%s, fd=%d\n", cliTbl[cliIndex].cliAddr, fd);
	trclib_writeLog(FL,trcBuf);
	trclib_writeLogErr (FL,trcBuf);

	// client table에서 삭제한다.
	memset ((void*)&cliTbl[cliIndex], 0, sizeof(FimdClientContext));

	return 1;

} //----- End of fimd_exeDisconn -----//

//------------------------------------------------------------------------------
// GUI로부터 요구 메시지를 수신한 경우
// - GUI가 처음 기동되어 각종 객체들을 생성하기 위한 initial 시간이 필요하므로
//	초기에 접속되어 먼저 FIMD_CONFIG_REQUEST를 보내면 각 시스템에 대해 sfdb의 정보를
//	한번만 전송하고 clint의 state를 INITIAL state로 변경한다.
// - GUI에서 initial이 완료되어 INIT_COMPLETE를 보내면 client의 state를 ACTIVE로 변경하여
//	주기적으로 각 시스템에 대한 sfdb 정보를 전송될 수 있도록 한다.
// - 즉, client의 state가 ACTIVE가 아니면 sfdb 정보를 주기적으로 전송하지 않는다.
//------------------------------------------------------------------------------
int fimd_exeRxSockMsg (int sockFd, SockLibMsgType *rxSockMsg)
{
	int	cliIndex;

	// data port가 아닌 놈에서 받은 메시지는 버린다.
	if ((cliIndex = fimd_getCliIndex (sockFd)) < 0) {
		sprintf(trcBuf,"[fimd_exeRxSockMsg] not found fd(%d) in cliTbl\n", sockFd);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}
	
	if (cliTbl[cliIndex].bindPortNum != dataPortNum) {
		sprintf(trcBuf,"[fimd_exeRxSockMsg] not data_port; dataPortNum=%d, eventPortNum=%d, rxPortNum=%d\n",
				dataPortNum, eventPortNum, cliTbl[cliIndex].bindPortNum);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	sprintf(trcBuf,"[fimd_exeRxSockMsg] receive [%s]; cliAddr=%s, fd=%d\n",
			rxSockMsg->body, cliTbl[cliIndex].cliAddr, sockFd);
	trclib_writeLogErr(FL,trcBuf);

	if (!strcasecmp (rxSockMsg->body, FIMD_CLIENT_MSG_CONFIG_REQUEST)) {
		// GUI가 처음 기동되어 각종 객체들을 생성하는 등 initial 절차에 필요한 정보를
		//	요청한 경우, 각 시스템에 대한 정보를 일정시간 간격을 두고 한번만 전송한다.
		// - 시간 간격을 두고 전송해야 하므로 thread를 생성하여 수행한다.
		fimd_rxConfigRequest (sockFd);
	}else if (!strcasecmp (rxSockMsg->body, FIMD_CLIENT_MSG_INIT_COMPLETE)) {
		fimd_rxInitComplete (sockFd);
	} else {
		sprintf(trcBuf,"[fimd_exeRxSockMsg] unknown client message(%s); cliAddr=%s, fd=%d\n",
				rxSockMsg->body, cliTbl[cliIndex].cliAddr, sockFd);
		trclib_writeLogErr (FL,trcBuf);
	}

	fimd_broadcastAlmEvent2Client ();

	return 1;

}
//----- End of fimd_exeRxSockMsg -----//

void    fimd_checkStatReportTime()
{
	time_t      cur_time;
	struct tm   cur_tMS;
	char        tStamp[32];

	cur_time  = time ( (time_t *)0);
	localtime_r ( &cur_time, &cur_tMS );

	if ( (cur_tMS.tm_min%5) == 1 && statFLAG == 0 ){
        	statFLAG = 1;
        	sprintf(tStamp,"%04d-%02d-%02d %02d:%02d:00",
            		(cur_tMS.tm_year+1900),(cur_tMS.tm_mon+1),cur_tMS.tm_mday,cur_tMS.tm_hour,(cur_tMS.tm_min-6));

        	fimd_broadcastStatEvent2Client (tStamp);

    	} else if ( (cur_tMS.tm_min%5) != 1 ){
        	statFLAG = 0;
    	} 

} //----- End of fimd_checkStatReportTime -----//

void handleChildProcess(int signo)
{
	int	status,pid;

	/* zombie 때문에 , Nonblocking*/
	while (( pid = waitpid(-1,&status,WNOHANG)) > 0);
	return;	
}

int isTimeToWork ()
{
	int	i, periodTime;
	time_t	cur_time, prev_time;

    	struct tm   cur_tMS, prev_tMS;

    	cur_time  = time ( (time_t *)0);
    	localtime_r ( &cur_time, &cur_tMS );

	if(cur_tMS.tm_min != 7) return -1; //매시 7분에 수집요청
	if(cur_tMS.tm_hour == prevWorkTime) return -1;

	prevWorkTime = cur_tMS.tm_hour;

	// sysconfig의 WORK_TIME 에 altibase error log 요청을 보낸다.
	for(i=0; i<wtEndIdx; i++) {
    		if (cur_tMS.tm_hour==WTtable[i]){
			if(i==0) {
        			periodTime = WTtable[i]+24 - WTtable[wtEndIdx-1];
			} else {
        			periodTime = WTtable[i] - WTtable[i-1];
			}

			if(periodTime==0) return -1;
			if(periodTime==24) cur_time = cur_time-WTtable[i]*3600;

			prev_time = (cur_time - periodTime*3600);
			localtime_r ( &prev_time, &prev_tMS );
			sprintf(WTperiod,"%d-%02d-%02d %02d:00:00 -- %d-%02d-%02d %02d:00:00",
					prev_tMS.tm_year+1900, prev_tMS.tm_mon+1, prev_tMS.tm_mday, prev_tMS.tm_hour, 
					cur_tMS.tm_year+1900, cur_tMS.tm_mon+1, cur_tMS.tm_mday, cur_tMS.tm_hour);

        		return periodTime;
    		} 
	}

    	return -1;
}

