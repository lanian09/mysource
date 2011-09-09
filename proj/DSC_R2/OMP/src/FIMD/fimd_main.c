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
int  g_hwswType[SFM_MAX_HPUX_HW_COM]; // HW/SW Type ����..

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

// by helca 09.29 mmc ��ɾ��� dis_ppd, act_ppd, dact_ppd�� ������� ����. 
// GUI client�� �����ؿ� ��� fimd_DactAlmEvent2Client�� ���� ���� �ʵ��� sound_flag�� 1�� �ʱ�ȭ��. 

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
	/* hjjung dis-session-lmt, set-session-lmt �߰� */
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
	int	tickPerSec=200; // 1�ʴ� loop�� ��� ������ Ƚ��; cpu ���ɿ� ���� �޶�����.
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
		// GUI�� ����� socket port������ event�� ó���Ѵ�.
		//
		ret = socklib_action ((char*)&rxSockMsg, &actFd);
		switch (ret)
		{
			// GUI�κ��� �䱸 �޽����� ������ ���
			case SOCKLIB_CLIENT_MSG_RECEIVED:
				fimd_exeRxSockMsg (actFd, &rxSockMsg);
				break;
			case SOCKLIB_NEW_CONNECTION:
				// client table�� �߰��Ѵ�.
				fimd_exeNewConn (actFd);
				break;
			case SOCKLIB_CLIENT_DISCONNECTED:
				// client table���� �����Ѵ�.
				fimd_exeDisconn (actFd);
				break;
		}
		signal(SIGCHLD, handleChildProcess); // 041004.lndb.cjs for <defunct>
		
		// �ֱ������� GUI�� sfdb�� ������ �ϴµ�,
		// - �ֱ����� ������ client�� state�� ACTIVE�� �����θ� ������.
		// - �ѹ��� ��� ������ �ʰ� �ý��ۺ��� ������ ������.
		// - 1�ʳ��� �� �ý����� ������ �ð� �������� ������.
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

			/*NMS ����üũ*/
			fimd_check_NMS_status(sysIndex);	
		}
		
		// �ֱ������� current_alarm DB�� �ִ� �߸��� ������ �����Ѵ�.
		if (loopCnt % (tickPerSec*30) == 0) {
			fimd_checkCurrentAlarm ();
			
		}
		
		// �ֱ���(1�ð�)���� alarm_history DB�� �ִ� ������ ������ �����Ѵ�.
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
// �� �ý��ۿ��� �뺸�Ǵ� ���� ���� �޽����� ó���Ѵ�.
//------------------------------------------------------------------------------
int fimd_exeStsRptMsg (IxpcQMsgType *rxIxpcMsg)
{

	switch (rxIxpcMsg->head.msgId) 
	{
		// ��� �ý��ۿ��� �������� �����Ǵ� �������� report - SAMD
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
// �� �ý��ۿ��� �뺸�Ǵ� ��� �뺸 �޽����� ó���Ѵ�.
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
// MMCD���� ��ɾ ������ ���
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
	
	// ó�� function�� ȣ���Ѵ�.
	//
	(int)(*(mmcHdlr->func)) (rxIxpcMsg);
	return 1;

} //----- End of fimd_exeMMCMsg -----//

/*
*	Logon ��� ���ø� ���� �Լ�
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

		/** Request �� ���� 0�̰ų�, Deactive SCM ����� ��쿡 ���Ͽ�, flag�� �����Ͽ� �����Ѵ�.
			Request �� ���� 0�� ��쿡��, ��ֵ���� NORMAL�� �����Ǿ�� �Ѵ�.
			Deactive SCM ����� ���� �ϴ� ������. noted by uamyd 20110222
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
			fimd_backupLogon2File(); //���� �ִ� Alarm ���� ���ؼ� �����ϴ� ������ ���� ������, SM, OMP �ý��� �ܿ� �۵� ����.
			fimd_broadcastAlmEvent2Client();
		}
	}/* for, log_mod ���� 2�� �����ؼ� �����ϵ��� �Ѵ�. by uamyd 20110424 */

	return 1;

} //----- End of fimd_exeStatRptMsg -----//

/* hjjung_20100823 */
//------------------------------------------------------------------------------
// RLEG ���� �뺸�Ǵ� ��� ���� �޽����� ó���Ѵ�.(CPS)
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
// GUI client�� �����ؿ� ��� ȣ��Ǿ� client table�� �߰��Ѵ�.
//------------------------------------------------------------------------------
int fimd_exeNewConn (int fd)
{
	int	i, bindPortNum;
	char	cliAddr[32];
   	int     txLen;
   	SockLibMsgType  txSockMsg;
   	IxpcQMsgType rxIxpcMsg;

	// �����ؿ� GUI client�� ip_adress�� � binding port�� ������ ������ ������
	//	������� socklib���� �����ϴ� clientSockFdTbl�� �˻��Ѵ�.
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
	
	// cliTbl���� ����ִ� index�� ã�´�.
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

	// cliTbl�� ������ �����Ѵ�.
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
			// client table���� �����Ѵ�.
			memset ((void*)&cliTbl[i], 0, sizeof(FimdClientContext));
			return -1;
		}

		memset(&rxIxpcMsg, 0, sizeof(IxpcQMsgType));

		if(*sound_flag == 0)
			fimd_DactAlmEvent2Client ();
		else
			fimd_ActAlmEvent2Client ();

	}

	// GUI�� ���ο� Connection�� �̷� ������ alarm_event noti�� �ش�. 
	fimd_broadcastAlmEvent2Client ();
	fimd_checkCurrentAlarm ();		
	return 1;

} //----- End of fimd_exeNewConn -----//

//------------------------------------------------------------------------------
// GUI client�� ���ӵ� connection�� ������ ��� ȣ��Ǿ� client table���� �����Ѵ�.
//------------------------------------------------------------------------------
int fimd_exeDisconn (int fd)
{
	int	cliIndex;
	

	// ������ ������ client�� �˻��Ѵ�.
	//
	if ((cliIndex = fimd_getCliIndex (fd)) < 0) {
		sprintf(trcBuf,"[fimd_exeDisconn] not found fd(%d) in cliTbl\n", fd);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}
	
	sprintf(trcBuf,"[fimd_exeDisconn] disconnected client; cliAddr=%s, fd=%d\n", cliTbl[cliIndex].cliAddr, fd);
	trclib_writeLog(FL,trcBuf);
	trclib_writeLogErr (FL,trcBuf);

	// client table���� �����Ѵ�.
	memset ((void*)&cliTbl[cliIndex], 0, sizeof(FimdClientContext));

	return 1;

} //----- End of fimd_exeDisconn -----//

//------------------------------------------------------------------------------
// GUI�κ��� �䱸 �޽����� ������ ���
// - GUI�� ó�� �⵿�Ǿ� ���� ��ü���� �����ϱ� ���� initial �ð��� �ʿ��ϹǷ�
//	�ʱ⿡ ���ӵǾ� ���� FIMD_CONFIG_REQUEST�� ������ �� �ý��ۿ� ���� sfdb�� ������
//	�ѹ��� �����ϰ� clint�� state�� INITIAL state�� �����Ѵ�.
// - GUI���� initial�� �Ϸ�Ǿ� INIT_COMPLETE�� ������ client�� state�� ACTIVE�� �����Ͽ�
//	�ֱ������� �� �ý��ۿ� ���� sfdb ������ ���۵� �� �ֵ��� �Ѵ�.
// - ��, client�� state�� ACTIVE�� �ƴϸ� sfdb ������ �ֱ������� �������� �ʴ´�.
//------------------------------------------------------------------------------
int fimd_exeRxSockMsg (int sockFd, SockLibMsgType *rxSockMsg)
{
	int	cliIndex;

	// data port�� �ƴ� �𿡼� ���� �޽����� ������.
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
		// GUI�� ó�� �⵿�Ǿ� ���� ��ü���� �����ϴ� �� initial ������ �ʿ��� ������
		//	��û�� ���, �� �ý��ۿ� ���� ������ �����ð� ������ �ΰ� �ѹ��� �����Ѵ�.
		// - �ð� ������ �ΰ� �����ؾ� �ϹǷ� thread�� �����Ͽ� �����Ѵ�.
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

	/* zombie ������ , Nonblocking*/
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

	if(cur_tMS.tm_min != 7) return -1; //�Ž� 7�п� ������û
	if(cur_tMS.tm_hour == prevWorkTime) return -1;

	prevWorkTime = cur_tMS.tm_hour;

	// sysconfig�� WORK_TIME �� altibase error log ��û�� ������.
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

