#include "cond_proto.h"
#include "stm_msgtypes.h"

int	condQid, ixpcQid, nmsifQid;
char	mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN];
char	trcBuf[4096], trcTmp[1024];
int	almLogId, stsLogId;
/*** TRACE 메시지  */
int	traceId;
/*** TRACE 메시지  */
long	prev_mtype;
char	prev_body[MAX_GEN_QMSG_LEN];

InhMsgTbl *inhMsg;
extern int      trcFlag, trcLogFlag;
extern ClientSockFdContext      clientSockFdTbl[SOCKLIB_MAX_CLIENT_CNT];

GeneralQMsgType 	prev_rxGenQMsg;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main (int ac, char *av[])
{
	int		ret, actFd, loopCnt=0;
	int		tickPerSec=100;
	SockLibMsgType	rxSockMsg;
	GeneralQMsgType	rxGenQMsg;
    	
#if 0
	if((check_Index = check_my_run_status("COND")) < 0)
    		exit(0);
#endif

	if (cond_initial() < 0) {
		fprintf(stderr,">>>>>> cond_initial fail\n");
		return -1;
	}

	// clear previous messages
	while (msgrcv(condQid, &rxGenQMsg, sizeof(rxGenQMsg), 0, IPC_NOWAIT) > 0);

	//trcLogFlag = 1;

	memset(&rxGenQMsg, 0, sizeof(GeneralQMsgType));
	memset(&prev_rxGenQMsg, 0, sizeof(GeneralQMsgType));	
	
	while (1)
	{
		while ((ret = msgrcv(condQid, &rxGenQMsg, sizeof(rxGenQMsg), 0, IPC_NOWAIT)) > 0) {
			cond_exeRxQMsg (&rxGenQMsg);
			memset(&rxGenQMsg, 0, sizeof(GeneralQMsgType));
		}
		
		if ((ret < 0) && (errno == EINVAL || errno == EFAULT)) {
			sprintf(trcBuf,"[cond_main] >>> msgrcv fail; err=%d(%s)\n", errno, strerror(errno));
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
				cond_exeRxSockMsg (actFd, &rxSockMsg);
				break;

			case SOCKLIB_NEW_CONNECTION:
				cond_exeNewConn (actFd);
				if (strlen((const char*)&prev_rxGenQMsg) > 0 )	
					cond_exeConsoleMsg (&prev_rxGenQMsg);		
				break;

			case SOCKLIB_CLIENT_DISCONNECTED:
				cond_exeDisconn (actFd);
				break;
		}

		if (++loopCnt%tickPerSec == 0) {
			keepalivelib_increase();
		}

	} //-- end of while(1) --//

	return 0;
	
} //----- End of main -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int cond_exeRxQMsg (GeneralQMsgType *rxGenQMsg)
{
	int	ret=1;

	switch (rxGenQMsg->mtype) {
		case MTYPE_SETPRINT:
			ret = trclib_exeSetPrintMsg ((TrcLibSetPrintMsgType*)rxGenQMsg);
			break;
		case MTYPE_ALARM_REPORT:
		case MTYPE_STATUS_REPORT:
		case MTYPE_DB_DISCONNECT:			// DB disconnection event
		case MTYPE_STAT_REPORT_SHORT_TERM: // 단기 통계
		case MTYPE_STAT_REPORT_LONG_TERM:  // 중장기 통계
		case MTYPE_UAWAP_TRANSLOG:         // uawap trans log file write	
		case MTYPE_TRC_CONSOLE:			
		case MTYPE_QUEUE_CLEAR_REPORT:
			ret = cond_exeConsoleMsg (rxGenQMsg);
			break;
		case MTYPE_NO_TRANSMITTED_ACT:  /*OMP에서는 요청하지 않는것 같다. sjjeon*/
			ret = cond_exeConsoleMsg (rxGenQMsg);
			memset(&prev_rxGenQMsg, 0x00, sizeof(GeneralQMsgType));
			memcpy(&prev_rxGenQMsg,rxGenQMsg, sizeof(GeneralQMsgType));	
			break;	
		case MTYPE_NO_TRANSMITTED_DACT: /*OMP에서는 요청하지 않는것 같다. sjjeon*/
			ret = cond_exeConsoleMsg (rxGenQMsg);
			break;

		case MTYPE_MMC_REQUEST:
			ret = cond_exeMMCMsg ((IxpcQMsgType*)rxGenQMsg->body);
			break;

		// MMC 처리 메시지는 MMCD에서 직접 로그를 남기므로 바로 printer로 출력만 한다.
		//
		case MTYPE_MMC_RESPONSE:
			break;

		default:
			sprintf(trcBuf,"[cond_exeRxQMsg] received unknown mtype(%ld)\n", rxGenQMsg->mtype);
			trclib_writeLogErr (FL,trcBuf);
			return -1;
	}

	return 1;

} //----- End of cond_exeRxQMsg -----//



//------------------------------------------------------------------------------
// FIMD,SAMD 등에서 통보되는 GUI console에 전달되어야 하는 메시지를 수신한 경우 호출되어,
//	GUI console에서 접속된 port들로 전달(broadcast)하고, mtype별로 각각 ALARM,STATUS
//	메시지 로그 directory에 logging한다.
//------------------------------------------------------------------------------
int cond_exeConsoleMsg (GeneralQMsgType *rxGenQMsg)
{
	int	logId=stsLogId;
   	char	condBuf[4096], tmpBuf[4096]; 	
	IxpcQMsgType	*rxIxpcMsg;
	//SockLibMsgType	txSockMsg;  sjjeon:사용하지 않는것같아 막는다.

	rxIxpcMsg = (IxpcQMsgType*)rxGenQMsg->body;
	// mtype별로 로그 파일을 결정한다.
	switch (rxGenQMsg->mtype) {
		case MTYPE_ALARM_REPORT:
			if (trcFlag || trcLogFlag) {
				sprintf(trcBuf,"[cond_exeConsoleMsg] rx ALARM MSG src(%s-%s)\n",
					rxIxpcMsg->head.srcSysName, rxIxpcMsg->head.srcAppName);
				trclib_writeLog (FL,trcBuf);
			}
			logId = almLogId;
			break;

		case MTYPE_STATUS_REPORT:
			if (trcFlag || trcLogFlag) {
				sprintf(trcBuf,"[cond_exeConsoleMsg] rx STS MSG src(%s-%s)\n",
					rxIxpcMsg->head.srcSysName, rxIxpcMsg->head.srcAppName);
				trclib_writeLog (FL,trcBuf);
			}
			logId = stsLogId;
			break;

		case MTYPE_DB_DISCONNECT:		
			if (trcFlag || trcLogFlag) {
				sprintf(trcBuf,"[cond_exeConsoleMsg] rx STS DB Disconnect src(%s-%s)\n",
					rxIxpcMsg->head.srcSysName, rxIxpcMsg->head.srcAppName);
				trclib_writeLog (FL,trcBuf);
			}
			logId = stsLogId;
			break;

		case MTYPE_STAT_REPORT_SHORT_TERM: // 단기 통계
			if (trcFlag || trcLogFlag) {
				sprintf(trcBuf,"[cond_exeConsoleMsg] rx SHORT STAT MSG src(%s-%s)\n",
					rxIxpcMsg->head.srcSysName, rxIxpcMsg->head.srcAppName);
				trclib_writeLog (FL,trcBuf);
			}
			logId = stsLogId;
			break;

		case MTYPE_STAT_REPORT_LONG_TERM:  // 중장기 통계
			if (trcFlag || trcLogFlag) {
				sprintf(trcBuf,"[cond_exeConsoleMsg] rx LONG STAT MSG src(%s-%s)\n",
					rxIxpcMsg->head.srcSysName, rxIxpcMsg->head.srcAppName);
				trclib_writeLog (FL,trcBuf);
			}
			logId = -1 ;
			break;
		case MTYPE_QUEUE_CLEAR_REPORT:
			if (trcFlag || trcLogFlag) {
				sprintf(trcBuf,"[cond_exeConsoleMsg] rx STS MSG src(%s-%s)\n",
					rxIxpcMsg->head.srcSysName, rxIxpcMsg->head.srcAppName);
				trclib_writeLog (FL,trcBuf);
			}
			logId = stsLogId;
			break;
		case MTYPE_UAWAP_TRANSLOG:
		case MTYPE_TRC_CONSOLE: // jjinri 2009.05.05
   			if (trcFlag || trcLogFlag) {
				sprintf(trcBuf,"[cond_exeConsoleMsg] rx TRACE MSG src(%s-%s)\n",
					rxIxpcMsg->head.srcSysName, rxIxpcMsg->head.srcAppName);
				trclib_writeLog (FL,trcBuf);
   			}
			sprintf(trcBuf,"[MTYPE_TRC_CONSOLE] srcApp: %s\n", rxIxpcMsg->head.srcAppName);
			trclib_writeLogErr (FL,trcBuf);
   	
			// by helca 12.19
			sprintf(tmpBuf,"    DSCM %s\n", commlib_printTStamp());
			strcpy(condBuf, rxIxpcMsg->body);
			strcat(tmpBuf, condBuf);
			strcat(tmpBuf, "      COMPLETED\n\n\n");
			strcpy(rxIxpcMsg->body, tmpBuf);
			rxIxpcMsg->head.bodyLen = strlen(tmpBuf);
//			logId = stsLogId ;
			logId = traceId ;
			break;
		
		default:
			break;
	}

	rxIxpcMsg->body[rxIxpcMsg->head.bodyLen] = 0;

	// 로그파일에 기록한다.
	// save 5minute statistics , not periodic 

	// 040813.hphlr.cjs
	//if(logId != NULL) 
	if(strlen(rxIxpcMsg->body)>4096) return 0;

	if(logId != -1)
		logPrint(logId,FL,"%s",rxIxpcMsg->body);

	// GUI console로 socket방식으로 전달하고, nmsif로 msgQ를 통해 전달한다.
	// - NMS로 모든 status,alarm,MMC 결과 메시지가 전달되어야 하는데, status,alarm
	//	메시지는 cond를 통해 전달되므로 여기서 전송하고, MMC 결과 메시지는 MMCD에서
	//	nmsif로 직접 전달한다.
	
	/* jean */
	if (rxGenQMsg->mtype != MTYPE_STAT_REPORT_SHORT_TERM) 
		cond_txConsoleMsg2GUI (rxGenQMsg);
	
	// send to nmsif if only alarm/status : 2006.08.17 by sdlee
	if (rxGenQMsg->mtype == MTYPE_ALARM_REPORT || rxGenQMsg->mtype == MTYPE_STATUS_REPORT)
		cond_txConsoleMsg2nmsif (rxGenQMsg);

	return 1;

} //----- End of cond_exeConsoleMsg -----//



//------------------------------------------------------------------------------
// MMCD에서 명령어를 수신한 경우
//------------------------------------------------------------------------------
int cond_exeMMCMsg (IxpcQMsgType *rxIxpcMsg)
{
	IxpcQMsgType 	rxIxpcMsgtmp;
	MMLReqMsgType	*mmlReqMsg;
	pthread_attr_t	thrAttr;
	pthread_t	thrId;

	memcpy(&rxIxpcMsgtmp, rxIxpcMsg, sizeof(IxpcQMsgType));
	mmlReqMsg = (MMLReqMsgType*)rxIxpcMsgtmp.body;

	if (!strcasecmp (mmlReqMsg->head.cmdName, "dis-msg-his") ||
		!strcasecmp (mmlReqMsg->head.cmdName, "dis-cmd-his")) {
		// 로그를 검색하고 결과를 보내는데 시간이 오래 걸리므로 thread를 생성해 수행한다.
		//
		pthread_attr_init (&thrAttr);
		pthread_attr_setscope (&thrAttr, PTHREAD_SCOPE_SYSTEM);
		pthread_attr_setdetachstate (&thrAttr, PTHREAD_CREATE_DETACHED);
		if (pthread_create (&thrId, &thrAttr, cond_mmc_srch_log_his, (void*)&rxIxpcMsgtmp)) {
			sprintf(trcBuf,"[cond_exeMMCMsg] pthread_create fail(cond_mmc_srch_log_his)\n");
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}
	} else if( !strcasecmp (mmlReqMsg->head.cmdName, "alw-msg") ){
		cond_mmc_alw_msg((void*)&rxIxpcMsgtmp);
	} else if( !strcasecmp (mmlReqMsg->head.cmdName, "dis-inh-msg") ){
		cond_mmc_dis_inh_msg((void*)&rxIxpcMsgtmp);
	} else if( !strcasecmp (mmlReqMsg->head.cmdName, "inh-msg") ){
		cond_mmc_inh_msg((void*)&rxIxpcMsgtmp);
	} else {
		sprintf(trcBuf,"[cond_exeMMCMsg] received unknown mml_cmd(%s)\n", mmlReqMsg->head.cmdName);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	return 1;

} //----- End of cond_exeMMCMsg -----//



//------------------------------------------------------------------------------
// GUI로부터 요구 메시지를 수신한 경우
//------------------------------------------------------------------------------
int cond_exeRxSockMsg (int sockFd, SockLibMsgType *rxSockMsg)
{
	return 1;

} //----- End of cond_exeRxSockMsg -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int cond_exeNewConn (int fd)
{
	int     i;
	char    cliAddr[32];

	// 접속해온 GUI client의 ip_adress를 얻기위해 socklib에서 관리하는
	//  clientSockFdTbl을 검색한다.
	//
	for (i=0; i<SOCKLIB_MAX_CLIENT_CNT; i++) {
		if (clientSockFdTbl[i].fd == fd)
			break;
	}
	if (i >= SOCKLIB_MAX_CLIENT_CNT) {
		sprintf(trcBuf,"[cond_exeNewConn] not found fd[%d] in clientSockFdTbl\n", fd);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}
	sprintf (cliAddr, "%s", inet_ntoa(clientSockFdTbl[i].cliAddr.sin_addr));

	sprintf(trcBuf,"[cond_exeNewConn] connected new client; cliAddr=%s, fd=%d\n", cliAddr, fd);
	trclib_writeLogErr (FL,trcBuf);

	return 1;

} //----- End of cond_exeNewConn -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int cond_exeDisconn (int fd)
{

	sprintf(trcBuf,"[cond_exeDisconn] disconnected client; fd=%d\n", fd);
	trclib_writeLogErr (FL,trcBuf);

	return 1;

} //----- Ena of cond_exeDisconn -----//
