#include "scem.h"

#include <sys/signal.h>
#include <sys/msg.h>
#include <sys/wait.h>

int		scemQID, ixpcQID, mcdmQID;
char	*iv_home, l_sysconf[256];
char    trcBuf[4096], trcTmp[1024];
char	mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN], sysLabel[COMM_MAX_NAME_LEN];
char	sceFileName[256];

SFM_sfdb       	*sfdb;

extern int	trcFlag, trcLogFlag;
extern T_keepalive		*keepalive;
extern SFM_SCE			*g_pstSCEInfo;

void handleChildProcess (int sign);
int report_SCE2FIMD (void);
int report_SceFlow_Samd2Stmd (int flow_num, int sce_idx);
extern int getShell_SCE_status_snmp(int sysType);
extern int getShell_SNMP_SceFlow(int sysType);

/*******************************************************************************

*******************************************************************************/
void UserControlledSignal(int sign)
{
	sprintf (trcBuf, "[%s] PROGRAM IS NORMALLY TERMINATED, signal=%d\n",__FUNCTION__, sign); trclib_writeLogErr (FL,trcBuf);
    exit(0);
}

/*******************************************************************************
    
*******************************************************************************/
void IgnoreSignal(int sign)
{
    if (sign != SIGALRM){
		sprintf (trcBuf, "[%s] UNWANTED SIGNAL IS RECEIVED, signal = %d\n",__FUNCTION__, sign); trclib_writeLogErr (FL,trcBuf);
	}
    signal(sign, IgnoreSignal);
}

/*******************************************************************************
    
*******************************************************************************/

void SetUpSignal()
{       

	/* WANTED SIGNALS   */
	signal(SIGTERM, UserControlledSignal);
	signal(SIGINT,  UserControlledSignal);
	signal(SIGQUIT, UserControlledSignal);

	/* UNWANTED SIGNALS */
	signal(SIGHUP,  IgnoreSignal);
	signal(SIGALRM, IgnoreSignal);
	signal(SIGPIPE, IgnoreSignal);
	signal(SIGPOLL, IgnoreSignal);
	signal(SIGPROF, IgnoreSignal);
	signal(SIGUSR1, IgnoreSignal);
	signal(SIGUSR2, IgnoreSignal);
	signal(SIGVTALRM, IgnoreSignal);
} 

int main(void)
{
	int		check_Index, i;

	setbuf(stdout,NULL);

	if((check_Index = check_my_run_status("SCEM")) < 0)
		exit(0);	

	if (InitSys() < 0 )
		exit(1);
	
	SetUpSignal ();

	{ // clear previous messages
		char	tmpBuff[65536];
		while (msgrcv (scemQID, tmpBuff, sizeof(tmpBuff), 0, IPC_NOWAIT) > 0);
	}

	while (1)
	{
		/*cisco sce device 정보 수집*/
		for(i=0; i<MAX_SCE_CNT; i++){
			getShell_SCE_status_snmp(i);
			/*cisco sce flow info 정보 수집, stmd 로 전송*/
			report_SceFlow_Samd2Stmd(getShell_SNMP_SceFlow(i), i);
		}
		keepalivelib_increase();
		
		/*report to fimd*/
		report_SCE2FIMD();
		sleep(2);
	} //-- end of while(1) --//
}

void handleChildProcess (int sign)
{
    int status;
	signal(SIGCHLD, (void *)handleChildProcess);
	signal(SIGCLD,  (void *)handleChildProcess);
    /* 좀비 때문에 **/
    while (wait3 (&status, WNOHANG, (struct rusage *)0) > 0);
}

int report_SCE2FIMD (void)
{
	GeneralQMsgType     txGenQMsg;
	IxpcQMsgType        *txIxpcMsg;
	int                 txLen;
	txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
	memset ((void*)&txIxpcMsg->head, 0, sizeof(txIxpcMsg->head));

	txGenQMsg.mtype = MTYPE_STATUS_REPORT;

	strcpy (txIxpcMsg->head.srcSysName, mySysName);
	strcpy (txIxpcMsg->head.srcAppName, myAppName);
	strcpy (txIxpcMsg->head.dstSysName, "DSCM");
	strcpy (txIxpcMsg->head.dstAppName, "FIMD");

	txIxpcMsg->head.msgId   = MSGID_SYS_SCE_STATUS_REPORT;
	txIxpcMsg->head.bodyLen = sizeof(SFM_SCE);
	txLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;

	memcpy ((void*)txIxpcMsg->body, g_pstSCEInfo, sizeof(SFM_SCE));
	SFM_SCE_N2H ((SFM_SCE *)&txIxpcMsg->body);

	if (msgsnd(ixpcQID, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0) {
		sprintf(trcBuf,"[report_SCE2FIMD] msgsnd(report_SCEFIMD) fail; err=%d(%s)\n"
				, errno
				, strerror(errno));
		trclib_writeLogErr (FL,trcBuf);
		return -1;                                                                                                                                                   
	}                                                                                                                                                                
	else {                                                                                                                                                           
		/*      printf("report_SCE2FIMD::%s->%s::%s->%s\n"                                                                                                                   
				, mySysName                                                                                                                                          
				, myAppName                                                                                                                                          
				, txIxpcMsg->head.dstSysName                                                                                                                         
				, txIxpcMsg->head.dstAppName);                                                                                                                       
		 */                                                                                                                                                                   
	}                                                                                                                                                                
	return 1;                                                                                                                                                        

} /** End of report_SCE2FIMD **/ 

int report_SceFlow_Samd2Stmd (int flow_num, int sce_idx)
{
	GeneralQMsgType     txGenQMsg;
	IxpcQMsgType        *txIxpcMsg;
	int                 txLen;
	SCE_FLOW_INFO       stSceFlow;

	txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
	memset ((void*)&txIxpcMsg->head, 0, sizeof(txIxpcMsg->head));

	txGenQMsg.mtype = MTYPE_STATUS_REPORT;

	strcpy (txIxpcMsg->head.srcSysName, mySysName);
	strcpy (txIxpcMsg->head.srcAppName, myAppName);
	strcpy (txIxpcMsg->head.dstSysName, "DSCM");
	strcpy (txIxpcMsg->head.dstAppName, "STMD");

	txIxpcMsg->head.msgId   = MSGID_SYS_SCEFLOW_STATUS_REPORT;
	txIxpcMsg->head.bodyLen = sizeof(SCE_FLOW_INFO);
	txLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;

	// Make SCE FLOW DATA 
	memset (&stSceFlow, 0x00, sizeof(SCE_FLOW_INFO));
	if (sce_idx)
		sprintf(stSceFlow.szSysName, "SCEB");
	else
		sprintf(stSceFlow.szSysName, "SCEA");

	stSceFlow.tGetTime  = time((time_t *)0);
	stSceFlow.uiFlowNum = flow_num;

	memcpy ((void*)txIxpcMsg->body, &stSceFlow, sizeof(SCE_FLOW_INFO));
	SCE_FLOW_INFO_N2H((SCE_FLOW_INFO *)&txIxpcMsg->body);

	if (msgsnd(ixpcQID, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0) {
		sprintf(trcBuf,"[report_SceFlow_Samd2Stmd] msgsnd(SCE_FLOW_INFO) fail; err=%d(%s)\n"
				, errno
				, strerror(errno));
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}
	else {
		/*      printf("report_SceFlow_Samd2Stmd: %s->%s::%s->%s\n"
				, mySysName
				, myAppName
				, txIxpcMsg->head.dstSysName
				, txIxpcMsg->head.dstAppName);
		 */
	}
	return 1;

} /** End of report_SceFlow_Samd2Stmd**/
