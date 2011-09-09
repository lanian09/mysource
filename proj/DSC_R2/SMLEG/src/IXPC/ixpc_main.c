#include "ixpc_proto.h"

int g_dStopFlag = 0;

IXPC_MsgQRoutTable	msgQRoutTbl[SYSCONF_MAX_APPL_NUM];
IXPC_SockRoutTable	sockRoutTbl[SYSCONF_MAX_ASSO_SYS_NUM];
IxpcConSts			*ixpcCON;
int		ixpcQid, ixpcPortNum;
char	mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN];
time_t	currentTime, lastPingTestTime;
char	trcBuf[4096], trcTmp[1024];

SFM_SysCommMsgType   *loc_sadb;
char sysLabel[COMM_MAX_NAME_LEN];

extern T_keepalive     *keepalive;
#define KEEPALIVE_CHECK_TIME    30


void checkKeepAlive(void);

/*------------------------------------------------------------------------------
* - ixpc는 application process들이 시스템 내부 및 다른 시스템에 있는 다른 application으로
*	메시지를 전송할 수 있도록 메시지를 routing해주는 기능을 담당한다.
* - 다른 시스템으로 routing을 위해 다른 시스템에 있는 ixpc와 tcp로 접속한다.
* - 자신의 message queue에 들어온 메시지의 destination 정보를 확인하여 시스템 내부에
*	있는 application으로 routing하는 경우 해당 application의 message queue에 메시지를
*	기록한다.
* - 다른 시스템에 있는 application으로 routing하는 경우에는 해당 시스템으로 연결된
*	socket fd로 전달하여, remote에 있는 ixpc가 해당 application의 message queue를
*	통해 전달할 수 있도록 한다.
* - ixpc간 접속은 초기기동시 자신이 bind port를 열고 remote ixpc로 접속한다.
*	접속되지 않는 경우 일정 시간마다 재접속을 시도한다.
* - 자신과 remote의 ixpc 모두 서로서로 연결하므로, 자신이 remote로 접속한 connection과
*	remote에서 접속해온 connection이 각각 존재한다. 단, 자신이 접속한 fd는 송신용으로,
*	remote에서 접속해온 fd는 수신용으로 사용된다.
* - 어떠한 원인에서 든 routing되지 못한 메시지는 즉시 폐기되며, error log를 남긴다. 
* - 일정기간 동안 송신할 메시지가 없으면 connection check를 위해 ixpc간 정해진
*	dummy 메시지를 보낸다. 정상적인 상태에서는 서로서로 송신 port에 대해 check 메시지를
*	보내므로 수신 port에 일정기간 이상 수신된 메시지가 없으면 remote ixpc가 hangup
*	되었다고 판단하고 송수신 port를 모두 강제로 끊는다.
* - remote ixpc 상태와 무관하게 LAN Cable의 이상 등으로 인한 통신장애를 감지하기 위해
*	주기적으로 ping test를 하고 접속되지 않으면 송수신 port를 모두 강제로 끊는다.
* - 메시지큐로 통신하는 경우 모든 메시지에 long형의 mtype이 반드시 포함되는데, 시스템마다
*	long이 4 또는 8byte로 다르다. 이를 고려하지 않고 그대로 socket을 통해 보내면 메시지가
*	제대로 읽혀지지 않는 문제가 발생한다. 이를 위해 다른 시스템으로 routing하는 경우
*	메시지큐에서 읽은 데이터중 long형인 mtype을 떼어내 socket header에 4byte로 변환하여
*	넣어서 보내고, 수신측에서 다시 떼어내 자신에 맞는 long형으로 변환하여 메시지큐로
*	보낸다.
------------------------------------------------------------------------------*/
int main (int ac, char *av[])
{
	GeneralQMsgType	rxGenQMsg;
	SockLibMsgType	rxSockMsg;
	int		ret, actFd, check_Index;
	time_t	prev;

	if((check_Index = check_my_run_status("IXPC")) <0 )
		exit(0);


	if (ixpc_initial() < 0) {
		fprintf(stderr,">>>>>> ixpc_initial fail\n");
		return -1;
	}

	/* clear previous queue messages
	*/
	//while (msgrcv(ixpcQid, &rxGenQMsg, sizeof(rxGenQMsg), 0, IPC_NOWAIT) > 0);

	prev = currentTime;
	while (!g_dStopFlag)
	{
		/* remote 시스템들과 연결된 socket port들을 확인하여 메시지를 처리한다.
		*/
		ret = socklib_action ((char*)&rxSockMsg, &actFd);

//if (ret != 0)
//printf ("OMP HERE0 ret=%d\n", ret);

		switch (ret)
		{
			case SOCKLIB_NEW_CONNECTION:
				ixpc_newConnEvent (actFd); /* remote ixpc가 접속해온 경우 */
				break;

			case SOCKLIB_CLIENT_MSG_RECEIVED:
				ixpc_recvEventRxPort (actFd, &rxSockMsg); /* remote ixpc로부터 data를 수신한 경우 */
				break;

			case SOCKLIB_SERVER_MSG_RECEIVED:
				ixpc_recvEventTxPort (actFd); /* remote로 접속한 port로 data가 들어온 경우 */
				break;

			case SOCKLIB_CLIENT_DISCONNECTED:
				ixpc_disconnEventRxPort (actFd); /* remote ixpc가 접속해온 fd가 끊어진 경우 */
				break;

			case SOCKLIB_SERVER_DISCONNECTED:
				ixpc_disconnEventTxPort (actFd); /* remote ixpc로 접속한 fd가(송신port) 끊어진 경우 */
				break;

			case SOCKLIB_NO_EVENT:
				break;

			default:
				break;
		} /* end of socklib_action */

		/* 자신의 msgQ에 들어오는 메시지를 처리한다.
		*/
		if (msgrcv(ixpcQid, &rxGenQMsg, sizeof(rxGenQMsg), 0, IPC_NOWAIT) < 0) {
			if (errno != ENOMSG) {
				sprintf(trcBuf,"[ixpc_main] msgrcv fail; err=%d(%s)\n", errno, strerror(errno));
				return -1;
			}
		} else {
			ixpc_exeRxQMsg (&rxGenQMsg);
		}

		/*
		* - 주기적(1초)으로 sockRoutTbl을 확인하여 접속되지 못한 곳으로 재접속을 시도한다.
		* - currentTime을 update한다.
		* - 일정시간 동안 송신할 메시지가 없으면 connection check 메시지를 보낸다.
		* - 일정시간 동안 수신된 메시지가 없으면 송수신 port 모두 강제 해제한다.
		* - 주기적으로 ping test하고 접속실패시 송수신 port 모두 강제 해제한다.
		*/
		currentTime = time(0);
		if (prev == currentTime)
			continue;

		prev = currentTime;
		keepalivelib_increase ();
		ixpc_checkConnections ();

		/*
		   samd process check
	    */
        checkKeepAlive();

	} /* end of while(1) */

	FinishProgram();

	return 1;

} /** End of main **/


int sendWatchdogMsg2COND (int procIdx)
{
	int				txLen;
	char			tmpBuf[256];
	GeneralQMsgType	txGenQMsg;
	IxpcQMsgType	*txIxpcMsg;
	char			logbuf[256];

	txGenQMsg.mtype = MTYPE_STATUS_REPORT;

	txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
	memset ((void*)&txIxpcMsg->head, 0, sizeof(txIxpcMsg->head));

	strcpy (txIxpcMsg->head.srcSysName, mySysName);
	strcpy (txIxpcMsg->head.srcAppName, myAppName);
	strcpy (txIxpcMsg->head.dstSysName, "DSCM");
	strcpy (txIxpcMsg->head.dstAppName, "COND");
	txIxpcMsg->head.msgId   = MSGID_WATCHDOG_STATUS_REPORT; // TTIB에서만 임시로 사용하는 값이다. COND에서는 사용하지 않는
	txIxpcMsg->head.segFlag = 0;
	txIxpcMsg->head.seqNo   = 1;

	sprintf(txIxpcMsg->body, "    %s %s\n    S%04d WATCH-DOG FUNCTION STATUS REPORT\n",
			sysLabel, commlib_printTStamp(), STSCODE_SFM_WATCHDOG_REPORT);

	sprintf(tmpBuf, "      SYSTEM = %s\n", mySysName);
	strcat (txIxpcMsg->body, tmpBuf);
	sprintf(tmpBuf, "      PROC   = SAMD\n");
	strcat (txIxpcMsg->body, tmpBuf);
	sprintf(tmpBuf, "      INFORM = WATCH-DOG_FUNCTION_EVENT_DETECTED\n");
	strcat (txIxpcMsg->body, tmpBuf);
	strcat (txIxpcMsg->body, "      COMPLETED\n\n\n");

	txIxpcMsg->head.bodyLen = strlen(txIxpcMsg->body) +1;
	txLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;

	if (msgsnd(ixpcQid, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0) {
		sprintf(logbuf,"[sendWatchdogMsg2COND] msgsnd fail to COND; err=%d(%s)\n", errno, strerror(errno));
		trclib_writeLogErr (FL,logbuf);
		return -1;
	} else {
		sprintf(logbuf,"[sendWatchdogMsg2COND] send to COND\n");
		trclib_writeLog (FL,logbuf);
	}

	return 1;
}



int report_sadb2FIMD(void)
{
	int					txLen;
	GeneralQMsgType		txGenQMsg;
	IxpcQMsgType		*txIxpcMsg;
	char 				logbuf[256];

	txIxpcMsg	= (IxpcQMsgType*)txGenQMsg.body;
	memset( (void*)&txIxpcMsg->head, 0x00, sizeof(txIxpcMsg->head));

	txGenQMsg.mtype	= MTYPE_STATUS_REPORT;

	strcpy(txIxpcMsg->head.srcSysName, mySysName);
	strcpy(txIxpcMsg->head.srcAppName, myAppName);
	strcpy(txIxpcMsg->head.dstSysName, "DSCM");
	strcpy(txIxpcMsg->head.dstAppName, "FIMD");

	txIxpcMsg->head.msgId	= MSGID_SYS_COMM_STATUS_REPORT;
	txIxpcMsg->head.bodyLen	= sizeof(SFM_SysCommMsgType) ;
	txLen					= sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;


	memcpy( (void*)txIxpcMsg->body, loc_sadb, txIxpcMsg->head.bodyLen);

	if(msgsnd(ixpcQid, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0)
	{
		sprintf(logbuf, "[%s] msgsnd(report_sadb2FIMD) fail; err=%d(%s)\n", __FUNCTION__, errno, strerror(errno));
		trclib_writeLogErr(FL, logbuf);
		return -1;
	}

	return 1;
}	/** End of report_sadb2FIMD **/



void checkKeepAlive(void)
{       
    int     i;

    for(i = 0; i < loc_sadb->processCount; i++)
	{   
		if(!strcasecmp(loc_sadb->loc_process_sts[i].processName, "SAMD")) {

			if(loc_sadb->loc_process_sts[i].status != SFM_STATUS_ALIVE)
				continue;

			if(keepalive->cnt[i] == keepalive->oldcnt[i])
				(keepalive->retry[i])++;
			else
				keepalive->retry[i] = 0;

			//if(!strcasecmp(loc_sadb->loc_process_sts[i].processName, "samd"))
			//	fprintf(stderr,"keepalive check cnt : %d, retry : %d\n",keepalive->cnt[i] , keepalive->retry[i]);

			keepalive->oldcnt[i] = keepalive->cnt[i];
			if(keepalive->retry[i] < KEEPALIVE_CHECK_TIME)
				continue;

			sprintf(trcBuf,"[%s] watchdog kill; proc= %s\n", __FUNCTION__, loc_sadb->loc_process_sts[i].processName);
			trclib_writeLogErr(FL,trcBuf);

			kill(loc_sadb->loc_process_sts[i].pid, SIGKILL);

			keepalive->retry[i] = -10;                                                                                

			sendWatchdogMsg2COND(i);                                                                                  

			// OMP-FIMD¿¡¼­ Aa¾O¸Þ½AAo¸| ¸¸μe¼o AOμμ·I CI±a A§CØ CA·I¼¼½º ≫oAAA¤º¸¸|                                  
			//  ´U½ACN¹ø AÐ¾i ProcessInfo¿¡ settingCN EA Ai½A reportCN´U.                                             
			//                                                                                                        
			//getProcessStatus();
//			report_sadb2FIMD();

#if 0	
			/* SAMD Watch-dog 작업시.. IXPC bind fail 현상으로 IXPC에서 SAMD를 Watch-dog 하지
			   않는다. crontab에서 samd의 process check 기능으로 IXPC watch-dog 기능을 대신한다.
			   SAMD가 죽었을 경우.. Keepalive check로 SAMD kill 하는 기능은 유지한다.
			 */
			int pid = 0, ret;
			if( (pid = fork()) < 0)                                                                                   
			{                                                                                                         
				sprintf(trcBuf,"[%s] stat fail=%d(%s)\n", __FUNCTION__, errno, strerror(errno));                      
				trclib_writeLogErr (FL,trcBuf);                                                                       
				return;                                                                                            
			}                                                                                                         

			if(!pid)                                                                                                  
			{                                                                                                         
				/*  child process´A execl·I ±aμ¿ ½AA² EA A¾·a   */                                                    
				freopen("/dev/null", "w", stdout);                                                                    
				if( (ret = execl("SAMD", "SAMD", NULL)) < 0)             
				{                                                                                                     
					;
				}                                                                                                     
				//return; // child process를 내려야 하는데...why ?
				exit(0);
			} 

#endif 
		} else {
			continue;
		}
	}                                                                                                             
                                                                                                                  
    return;                                                                                                       
}


