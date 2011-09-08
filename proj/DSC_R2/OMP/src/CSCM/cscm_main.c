#include "cscm.h"

#include <sys/signal.h>
#include <sys/msg.h>
#include <sys/wait.h>

int		cscmQID, ixpcQID, mcdmQID, queCNT;
char	*iv_home, l_sysconf[256];
char    trcBuf[4096], trcTmp[1024];
char	mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN], sysLabel[COMM_MAX_NAME_LEN];
char	szl2swFileName[256];

extern int	trcFlag, trcLogFlag;
extern T_keepalive		*keepalive;
extern SFM_L2Dev		*g_pstL2Dev;
SFM_SysCommMsgType  *loc_sadb;

void handleChildProcess (int sign);
void ping_test(void);
extern int getShell_L2_status_snmp(int sysType);

#define     NIPADDR(d)              (d&0xff),((d>>8)&0xff),((d>>16)&0xff),((d>>24)&0xff)

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

int my_system(const char *cmd)
{
	FILE *p;

	if ((p = popen(cmd, "w")) == NULL)
		return (-1);
	return (pclose(p));
}

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
	if((check_Index = check_my_run_status("CSCM")) < 0)
		exit(0);	

	if (InitSys() < 0 )
		exit(1);
	
	SetUpSignal ();

	{ // clear previous messages
		char	tmpBuff[65536];
		while (msgrcv (cscmQID, tmpBuff, sizeof(tmpBuff), 0, IPC_NOWAIT) > 0);
	}

	while (1)
	{
		// L2 Switch 정보를 shell로 정보를 조회한다.
		for(i=0; i< 2; i++){
			getShell_L2_status_snmp(i);
		}

		// Send to FIMD
		report_L2SW2FIMD();

		keepalivelib_increase(); 
			
		sleep(2); 
	}

}

void handleChildProcess (int sign)
{
    int status;
	signal(SIGCHLD, (void *)handleChildProcess);
	signal(SIGCLD,  (void *)handleChildProcess);
    /* 좀비 때문에 **/
    while (wait3 (&status, WNOHANG, (struct rusage *)0) > 0);
}

int report_L2SW2FIMD (void)
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

	txIxpcMsg->head.msgId   = MSGID_SYS_L2_STATUS_REPORT;
	txIxpcMsg->head.bodyLen = sizeof(SFM_L2Dev);
	txLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;

	memcpy ((void*)txIxpcMsg->body, g_pstL2Dev, sizeof(SFM_L2Dev));
	SFM_L2DEV_H2N (((SFM_L2Dev *)&txIxpcMsg->body));

	if (msgsnd(ixpcQID, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0) {
		sprintf(trcBuf,"[report_L2SW2FIMD] msgsnd fail; err=%d(%s)\n"
				, errno
				, strerror(errno));
		trclib_writeLogErr (FL,trcBuf);
		//      printf("%s",trcBuf);
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

} /** End of report_L2SW2FIMD **/ 

void ping_test (void)
{
	int i,j, rv;
	char    ipaddr[16];
	char    result[16];

	for (i=0; loc_sadb->loc_lan_sts[i].target_IPaddress != 0; i++)
	{
		j = 0;
		sprintf (ipaddr, "%d.%d.%d.%d", NIPADDR(loc_sadb->loc_lan_sts[i].target_IPaddress));
		sprintf(result, "%s\n",(pingtest(ipaddr, 0, 1,500000)>0)? "alive":"die");
		//fprintf(stderr, "%s. %s\n", ipaddr, result);

		if (!memcmp (result, "alive", 5)) {
			loc_sadb->loc_lan_sts[i].status = SFM_LAN_CONNECTED;
			sprintf(trcBuf,"[%s] pingtest success.ip:%s (re-try:1)\n",__FUNCTION__,ipaddr );
			trclib_writeLogErr (FL,trcBuf);
		} else {

			sprintf(trcBuf,"[%s] pingtest fail.ip:%s (re-try:1)\n",__FUNCTION__,ipaddr );
			trclib_writeLogErr (FL,trcBuf);

			rv = SFM_LAN_DISCONNECTED;

			do {
				sleep(1);
				sprintf(result, "%s\n",(pingtest(ipaddr, 0, 1,500000)>0)? "alive":"die");                                                                            
				if (!memcmp (result, "alive", 5)) {
					rv = SFM_LAN_CONNECTED;
					break;
				}else{
					sprintf(trcBuf,"[%s] pingtest fail.ip:%s (re-try:%d)\n",__FUNCTION__,ipaddr,j+2 );
					trclib_writeLogErr (FL,trcBuf);
				}
				j++;
			} while(j < 2);

			loc_sadb->loc_lan_sts[i].status = rv;
		}
		// 만약 핑이 안될경우 ping test 를 두번한다 
	}
}
