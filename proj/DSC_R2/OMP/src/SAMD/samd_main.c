#include "samd.h"

#include <sys/signal.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <pthread.h>

int     g_dStopFlag = 0;
int		samdQID, ixpcQID, mcdmQID, queCNT;
long	oldSumOfPIDs, newSumOfPIDs;
char	*iv_home, l_sysconf[256];
char	mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN], sysLabel[COMM_MAX_NAME_LEN];
char	l3pdFileName[256], sceFileName[256], szl2swFileName[256];
int		statistic_cnt=0;
pthread_mutex_t  mutex = PTHREAD_MUTEX_INITIALIZER; // ������ �ʱ�ȭ

SAMD_ProcessInfo                ProcessInfo[SYSCONF_MAX_APPL_NUM]; 
SFM_SysCommMsgType              *loc_sadb;
STM_LoadOMPStatMsgType          system_statistic;
SFM_sfdb       	*sfdb;

SFM_L3PD    *l3pd;
extern int	trcFlag, trcLogFlag;
extern      T_keepalive	*keepalive;
extern      SFM_SCE	*g_pstSCEInfo;

void handleChildProcess (int sign);
int mysqlLiveCheck();    /* mysql live check : sjjeon */
void etc_block_test();
int cmd2write(char *cmd); /* cmd  ����� write �Ѵ�. */
int my_system(const char *cmd);

int g_preSts;
/*******************************************************************************

*******************************************************************************/
int my_system(const char *cmd)
{
	FILE *p;

	if ((p = popen(cmd, "w")) == NULL)
		return (-1);
	return (pclose(p));
}


#if 0
void *thr_mmc_proc(void *arg);
void get_rxQ_read_thread(void)
{
    pthread_attr_t  thrAttr;
    pthread_t       thrId;
	int status;

    pthread_attr_init(&thrAttr);                                                   
    pthread_attr_setscope(&thrAttr, PTHREAD_SCOPE_SYSTEM);                         
    pthread_attr_setdetachstate(&thrAttr, PTHREAD_CREATE_DETACHED);                
    if(pthread_create(&thrId, &thrAttr, thr_mmc_proc, NULL)) {                    
        sprintf (trcBuf, "[%s] pthread_create fail\n",__FUNCTION__);                     
        trclib_writeLogErr (FL,trcBuf);                                            
    }
	pthread_join(thrId, (void *) &status);
    pthread_attr_destroy(&thrAttr);
	return;

}
#endif

void *thr_ping_check(void *arg);

void ping_check_thread(void)
{
    pthread_attr_t  thrAttr;
    pthread_t       thrId;
	int status;

    pthread_attr_init(&thrAttr);                                                   
    pthread_attr_setscope(&thrAttr, PTHREAD_SCOPE_SYSTEM);                         
    pthread_attr_setdetachstate(&thrAttr, PTHREAD_CREATE_DETACHED);                
    if(pthread_create(&thrId, &thrAttr, thr_ping_check, NULL)) {                    
        sprintf (trcBuf, "[%s] pthread_create fail\n",__FUNCTION__);                     
        trclib_writeLogErr (FL,trcBuf);                                            
    }
	pthread_join(thrId, (void *) &status);
    pthread_attr_destroy(&thrAttr);
	return;

}
#if 0
// MMC thread ó��
void *thr_mmc_proc(void *arg)
{ 
	int 		rxQmsgCnt;
	while (1)
	{
		//ping_test();
		rxQmsgCnt = HandleRxMsg();
		if (rxQmsgCnt < 0) return NULL;
		if (rxQmsgCnt == 0) commlib_microSleep(1000);
	}
}
#endif
void *thr_ping_check(void *arg)
{
	while(1)
	{
		ping_test();
		sleep(2);
	}
}
/*
	main function 
 */
int main()
{
	system_info 	system_infomation;
	int		check_Index;
	struct timeval	now, prev;
	unsigned int	now100MS, prev100MS; // 100milli-secsond ���� �ð�

	setbuf(stdout,NULL);

	if((check_Index = check_my_run_status("SAMD")) < 0)
		exit(0);	
	
	if (InitSys() < 0 )
		exit(1);
	
	{ // clear previous messages
		char	tmpBuff[65536];
		while (msgrcv (samdQID, tmpBuff, sizeof(tmpBuff), 0, IPC_NOWAIT) > 0);
	}

	get_system_information(&system_infomation, INIT_FLOW); //�� ó�� ���� ��Ȯ�� ����Ÿ�� �ƴϹǷ� ������ �Ѵ� 

	gettimeofday (&now, NULL);
	prev.tv_sec  = now.tv_sec;
	prev.tv_usec = now.tv_usec;
	prev100MS = (now.tv_sec * 10) + (now.tv_usec/100000);

	//get_rxQ_read_thread();
	ping_check_thread();

	while (!g_dStopFlag)
	{
		//thread���� ���� �ǹ̰�... ��� �������� �� 
		HandleRxMsg();
		commlib_microSleep(1000);

		gettimeofday (&now, NULL);
		now100MS = (now.tv_sec * 10) + (now.tv_usec/100000);

		// 100MS ���� �ð��� �ٲ��� ������ �ƹ��͵� ���Ѵ�.
		if (prev100MS == now100MS)
			continue;

		// 100MS ���� �ð����� �۾��ؾ� �� �ϵ�
		prev100MS = now100MS;

		if (prev.tv_sec == now.tv_sec)
			continue;

		// �� ���� �ð����� �۾��ؾ� �� �ϵ�
		prev.tv_sec  = now.tv_sec;
		prev.tv_usec = now.tv_usec;

		keepalivelib_increase();

		//handleChildProcess ();

		/* ���μ��� ���� ���� �� auto_restart */
		checkProcessStatus();

		/* watch-dog (keepalive) ��� ���� */
		checkKeepAlive();


		/* CPU, Memory ������ ���� ���� */
		get_system_information(&system_infomation, NORMAL_FLOW);
		commlib_microSleep(20000);

		/* disk ����� ���� ����
		*/
		get_diskUsage ();
		get_queUsage  ();


		if (g_dStopFlag) {
				
		}
		/* samd���� �����ϴ� cpu, memory, disk, lan, process ���¸� �����Ѵ�. */
		CheckNTPStS(); 
		report_sadb2FIMD ();

		/* 5�ʸ��� system message queue full ����
		*/
		if (now.tv_sec%5==1) {
			QClear();
		}

		/* 5�ʸ��� mysql live check : sjjeon
		*/
		if (now.tv_sec%5==0) {
			etc_block_test();
		}

	} //-- end of while(1) --//

	checkProcessStatus();
	FinishProgram();
	return 0;
}

void handleChildProcess (int sign)
{
    int status;
	signal(SIGCHLD, (void *)handleChildProcess);
	signal(SIGCLD,  (void *)handleChildProcess);
    /* ���� ������ **/
    while (wait3 (&status, WNOHANG, (struct rusage *)0) > 0);
}

/* ��Ÿ block check : sjjeon*/
void etc_block_test()
{
	int rv;
	char checkBlock[]="mysql";	

    /* mysql live check....: sjjeon*/                                               
    rv = mysqlLiveCheck();
    strcpy(loc_sadb->sysSts.linkSts[0].StsName, checkBlock);
    if(rv ==0)
		loc_sadb->sysSts.linkSts[0].status = 0;  // mysql running
	else{
		loc_sadb->sysSts.linkSts[0].status = 1;  // mysql not running
		sprintf(trcBuf,"[%s] mysql not running...\n", __FUNCTION__);
		trclib_writeLog (FL,trcBuf);
	}
}

/*
   mysql live check                                                                   
    0 : running
    1 : not running
*/
#if 1
int mysqlLiveCheck()    /* mysql live check : sjjeon */
{
	const int _BUFSIZ = 512;
	char cmd[100], out_file[64];
	char buf[_BUFSIZ];
	FILE *fp=NULL;
	int rv = 0, retry_cnt=0;

	bzero(cmd, sizeof(cmd));
	bzero(out_file, sizeof(out_file));
	memset(buf,0,sizeof(buf));

	sprintf(out_file, "/etc/MysqlLiveCheck");
	sprintf(cmd , "/etc/init.d/mysql status > %s", out_file);

RETRY_CHECK:

	// retry ����..
	if(retry_cnt>3){
		sprintf(trcBuf, "[%s] re-try fail.\n", __FUNCTION__);
		trclib_writeLogErr(FL, trcBuf);
		rv =1;
		goto FINISH;
	}

	// cmd 
	my_system(cmd);

	if ((fp = fopen(out_file, "r")) == NULL)
	{
		sprintf(trcBuf, "[%s] fopen error!\n",__FUNCTION__);
		trclib_writeLogErr (FL,trcBuf);
		retry_cnt++;
		goto RETRY_CHECK;                                                     
	}                                                                         

	while (fgets(buf,_BUFSIZ, fp) != NULL)                                    
	{                                                                         
		if(strstr(buf,"SUCCESS")!=NULL)                                       
		{                                                                     
			rv =0;                                                            
			goto FINISH;                       
		}                                                                     
	}                                                                         

	rv=1;                                                                     
	sprintf(trcBuf, "[%s] mysql not running.\n",__FUNCTION__ );               
	trclib_writeLogErr(FL, trcBuf);                                           

FINISH:                                                                       
	if(fp) fclose(fp);                                                        
	return rv;                                                                
}

#else
int mysqlLiveCheck()    /* mysql live check : sjjeon */
{
    const int _BUFSIZ = 512;
    char cmd[]="/etc/init.d/mysql status";
    char buf[_BUFSIZ];
    FILE *ptr=NULL;

    memset(buf,0,sizeof(buf));

    if ((ptr = popen(cmd, "r")) == NULL)
    {
        sprintf(trcBuf, "[%s] popen mysql error!\n",__FUNCTION__);
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }

    while (fgets(buf,_BUFSIZ, ptr) != NULL)
    {
        if(strstr(buf,"SUCCESS")) {
            /* popen buffer clear ......���������� ���� ����...*/
            while(fgets(buf, _BUFSIZ, ptr) != NULL) {}
            if(ptr)pclose(ptr);
            return 0;
        }
    }
    if(ptr) pclose(ptr);
    return 1;
} 
#endif
/*
by sjjeon
fimd�� status�� üũ �ϴ� �Լ�...
*/
void chkStatus_Test(char *file, char *function, int line)
{
    time_t      tm; 
    struct tm   tm_MS;
    char        tStamp[32];
    char        buff[2048];

    tm  = time ( (time_t *)0);
    localtime_r ( &tm, &tm_MS );
    memset(buff,0,sizeof(buff));

    sprintf(tStamp,"%04d-%02d-%02d %02d:%02d:00",
            (tm_MS.tm_year+1900),(tm_MS.tm_mon+1),tm_MS.tm_mday,tm_MS.tm_hour,(tm_MS.tm_min-6));

    if(g_preSts != g_pstSCEInfo->SCEDev[0].cpuInfo[0].mask){
        g_preSts = g_pstSCEInfo->SCEDev[0].cpuInfo[0].mask;
    }   
}

/*
	by sjjeon
	cmd ������ Write �Ѵ�.
	ex) "ls -alrt > ls.txt"

	The following function can be used in a  multithreaded  pro-
	cess  in  place  of  the  most  common  usage  of the Unsafe
	system(3C) function
 */
int cmd2write(char *cmd)
{
	FILE *p= NULL;

	if ((p = popen(cmd, "w")) == NULL){
		sprintf(trcBuf,"[%s] popen error. \n", __FUNCTION__);
		trclib_writeLog (FL,trcBuf);
		return (-1);
	}
	return (pclose(p));

}

