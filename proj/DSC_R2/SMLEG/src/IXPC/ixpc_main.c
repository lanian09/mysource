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
* - ixpc�� application process���� �ý��� ���� �� �ٸ� �ý��ۿ� �ִ� �ٸ� application����
*	�޽����� ������ �� �ֵ��� �޽����� routing���ִ� ����� ����Ѵ�.
* - �ٸ� �ý������� routing�� ���� �ٸ� �ý��ۿ� �ִ� ixpc�� tcp�� �����Ѵ�.
* - �ڽ��� message queue�� ���� �޽����� destination ������ Ȯ���Ͽ� �ý��� ���ο�
*	�ִ� application���� routing�ϴ� ��� �ش� application�� message queue�� �޽�����
*	����Ѵ�.
* - �ٸ� �ý��ۿ� �ִ� application���� routing�ϴ� ��쿡�� �ش� �ý������� �����
*	socket fd�� �����Ͽ�, remote�� �ִ� ixpc�� �ش� application�� message queue��
*	���� ������ �� �ֵ��� �Ѵ�.
* - ixpc�� ������ �ʱ�⵿�� �ڽ��� bind port�� ���� remote ixpc�� �����Ѵ�.
*	���ӵ��� �ʴ� ��� ���� �ð����� �������� �õ��Ѵ�.
* - �ڽŰ� remote�� ixpc ��� ���μ��� �����ϹǷ�, �ڽ��� remote�� ������ connection��
*	remote���� �����ؿ� connection�� ���� �����Ѵ�. ��, �ڽ��� ������ fd�� �۽ſ�����,
*	remote���� �����ؿ� fd�� ���ſ����� ���ȴ�.
* - ��� ���ο��� �� routing���� ���� �޽����� ��� ���Ǹ�, error log�� �����. 
* - �����Ⱓ ���� �۽��� �޽����� ������ connection check�� ���� ixpc�� ������
*	dummy �޽����� ������. �������� ���¿����� ���μ��� �۽� port�� ���� check �޽�����
*	�����Ƿ� ���� port�� �����Ⱓ �̻� ���ŵ� �޽����� ������ remote ixpc�� hangup
*	�Ǿ��ٰ� �Ǵ��ϰ� �ۼ��� port�� ��� ������ ���´�.
* - remote ixpc ���¿� �����ϰ� LAN Cable�� �̻� ������ ���� �����ָ� �����ϱ� ����
*	�ֱ������� ping test�� �ϰ� ���ӵ��� ������ �ۼ��� port�� ��� ������ ���´�.
* - �޽���ť�� ����ϴ� ��� ��� �޽����� long���� mtype�� �ݵ�� ���ԵǴµ�, �ý��۸���
*	long�� 4 �Ǵ� 8byte�� �ٸ���. �̸� ������� �ʰ� �״�� socket�� ���� ������ �޽�����
*	����� �������� �ʴ� ������ �߻��Ѵ�. �̸� ���� �ٸ� �ý������� routing�ϴ� ���
*	�޽���ť���� ���� �������� long���� mtype�� ��� socket header�� 4byte�� ��ȯ�Ͽ�
*	�־ ������, ���������� �ٽ� ��� �ڽſ� �´� long������ ��ȯ�Ͽ� �޽���ť��
*	������.
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
		/* remote �ý��۵�� ����� socket port���� Ȯ���Ͽ� �޽����� ó���Ѵ�.
		*/
		ret = socklib_action ((char*)&rxSockMsg, &actFd);

//if (ret != 0)
//printf ("OMP HERE0 ret=%d\n", ret);

		switch (ret)
		{
			case SOCKLIB_NEW_CONNECTION:
				ixpc_newConnEvent (actFd); /* remote ixpc�� �����ؿ� ��� */
				break;

			case SOCKLIB_CLIENT_MSG_RECEIVED:
				ixpc_recvEventRxPort (actFd, &rxSockMsg); /* remote ixpc�κ��� data�� ������ ��� */
				break;

			case SOCKLIB_SERVER_MSG_RECEIVED:
				ixpc_recvEventTxPort (actFd); /* remote�� ������ port�� data�� ���� ��� */
				break;

			case SOCKLIB_CLIENT_DISCONNECTED:
				ixpc_disconnEventRxPort (actFd); /* remote ixpc�� �����ؿ� fd�� ������ ��� */
				break;

			case SOCKLIB_SERVER_DISCONNECTED:
				ixpc_disconnEventTxPort (actFd); /* remote ixpc�� ������ fd��(�۽�port) ������ ��� */
				break;

			case SOCKLIB_NO_EVENT:
				break;

			default:
				break;
		} /* end of socklib_action */

		/* �ڽ��� msgQ�� ������ �޽����� ó���Ѵ�.
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
		* - �ֱ���(1��)���� sockRoutTbl�� Ȯ���Ͽ� ���ӵ��� ���� ������ �������� �õ��Ѵ�.
		* - currentTime�� update�Ѵ�.
		* - �����ð� ���� �۽��� �޽����� ������ connection check �޽����� ������.
		* - �����ð� ���� ���ŵ� �޽����� ������ �ۼ��� port ��� ���� �����Ѵ�.
		* - �ֱ������� ping test�ϰ� ���ӽ��н� �ۼ��� port ��� ���� �����Ѵ�.
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
	txIxpcMsg->head.msgId   = MSGID_WATCHDOG_STATUS_REPORT; // TTIB������ �ӽ÷� ����ϴ� ���̴�. COND������ ������� �ʴ�
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

			// OMP-FIMD�������� Aa��O������AAo��| ������e��o AO��졤I CI��a A��C�� CA��I�������� ��oAAA��������|                                  
			//  ��U��ACN���� A����i ProcessInfo���� settingCN EA Ai��A reportCN��U.                                             
			//                                                                                                        
			//getProcessStatus();
//			report_sadb2FIMD();

#if 0	
			/* SAMD Watch-dog �۾���.. IXPC bind fail �������� IXPC���� SAMD�� Watch-dog ����
			   �ʴ´�. crontab���� samd�� process check ������� IXPC watch-dog ����� ����Ѵ�.
			   SAMD�� �׾��� ���.. Keepalive check�� SAMD kill �ϴ� ����� �����Ѵ�.
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
				/*  child process��A execl��I ��a�좯 ��AA�� EA A����a   */                                                    
				freopen("/dev/null", "w", stdout);                                                                    
				if( (ret = execl("SAMD", "SAMD", NULL)) < 0)             
				{                                                                                                     
					;
				}                                                                                                     
				//return; // child process�� ������ �ϴµ�...why ?
				exit(0);
			} 

#endif 
		} else {
			continue;
		}
	}                                                                                                             
                                                                                                                  
    return;                                                                                                       
}


