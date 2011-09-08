#include "samd.h"
#include "check_sys_msg.h"

int     g_dStopFlag = 0;
long	newSumOfPIDs;
long	oldSumOfPIDs;

int		alarm_flag;
int		appCnt;
int		auxilaryCnt;
int		daemonCnt;
//// 07.21 jjinri int		duiaValue;
int		ixpcQID;
int		prev_alarm_flag;
int		queCNT;
int		samdQID;
int		statistic_cnt;

unsigned int	pingInterval;

char	iv_home[64];
char	l_sysconf[256];
char	R_sysconf[256];
char	systemModel[5];
char	mySysName[COMM_MAX_NAME_LEN];
char	myAppName[COMM_MAX_NAME_LEN];
char	sysLabel[COMM_MAX_NAME_LEN];
char	trcBuf[TRCBUF_LEN];
char	trcBuf1[TRCBUF_LEN];
char	trcTmp[TRCTMP_LEN];
char	l_sysconf2[256];

SAMD_ProcessInfo	ProcessInfo[SYSCONF_MAX_APPL_NUM];
SAMD_DaemonInfo		DaemonInfo[SYSCONF_MAX_DAEMON_NUM];	// daemon process를 처리하기 위해
SAMD_AuxilaryInfo	AuxilaryInfo[SYSCONF_MAX_AUXILARY_MSGQ_NUM];

SFM_SysCommMsgType		*loc_sadb;
STM_LoadMPStatMsgType	system_statistic;
#if 0
SFM_SysIFBond			bond[MAX_BOND_NUM];
#endif
SFM_SysSts				sysSts;
SFM_L3PD				*l3pd;

extern int			trcFlag;
extern int			trcLogFlag;
extern T_keepalive	*keepalive;

extern void	QClear (void); // by helca
extern void	no_Transmitted_udr_check(void);

extern int	link_test(void); // by helca
extern int	InitSys (void);
extern int	check_my_run_status(char *procname);
//extern int  findCaseIndex(char *syntax, char *idex);
//extern int  findWord(char *buf, char *index);
extern pid_t getCM_AdapPid(void);
extern int check_disk_status(void);

void handleChildProcess(int sign);
extern void thread_ping_test (void);

#ifdef __LINUX__
void bond_Result(void);
#endif

void link_info(void);

#ifdef __HPLOG__
void fan_info(void);
void power_info(void);
#endif

#if defined(__LINUX__) || defined(__HPLOG__)
void cpu_sts_info(void);
#endif
int getFanPwrInfo(void);
int getFanPwrInfo2(void);

int smNetStatus();
int mysqlLiveCheck();
int tt_liveCheck();

void *thread_SM_MainCheck(void *arg);
void smShellExe();

int smLiveCheck();    /* SM process live check : sjjeon */
int cmLiveCheck();    /* CM process live check : sjjeon */

#define HW_LINK_COUNT 16
/*******************************************************************************

*******************************************************************************/
int my_system(const char *cmd)
{
	FILE *p;

	if ((p = popen(cmd, "w")) == NULL)
		return (-1);
	return (pclose(p));
}


/*******************************************************************************
    
*******************************************************************************/

void *thr_mmc_proc(void *arg);
void *thr_dup_proc(void *arg);

void get_rxQ_read_thread(void)
{
    pthread_attr_t  thrAttr;
    pthread_t       thrId;
    char trcBuf[1024] = {0x00,};
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

// MMC thread 
void *thr_mmc_proc(void *arg)
{
#ifdef _USE_MMC_THREAD_
    int         rxQmsgCnt;
    while (1)
	{
		rxQmsgCnt = HandleRxMsg();
		if (rxQmsgCnt < 0) continue;
		if (rxQmsgCnt == 0) commlib_microSleep(1000);
	}
#endif
	return (void*)0;
}

void dup_check_thread(void)
{
    pthread_attr_t  thrAttr;
    pthread_t       thrId;
    char trcBuf[1024] = {0x00,};
    int status;

    pthread_attr_init(&thrAttr);
    pthread_attr_setscope(&thrAttr, PTHREAD_SCOPE_SYSTEM);
    pthread_attr_setdetachstate(&thrAttr, PTHREAD_CREATE_DETACHED);
    if(pthread_create(&thrId, &thrAttr, thr_dup_proc, NULL)) {
        sprintf (trcBuf, "[%s] pthread_create fail\n",__FUNCTION__);
        trclib_writeLogErr (FL,trcBuf);
    }
    pthread_join(thrId, (void *) &status);
    pthread_attr_destroy(&thrAttr);
    return;

}


void *thr_dup_proc(void *arg)
{
    while (1)
    {
		/*	이중화 절체를 위한 Process상태 수집	*/
		checkSdmdForPrcSts();
        sleep(1);
    }

}

//#define _USE_MMC_THREAD_
// 
int main(int argc, char *argv[])
{
	int				now100MS, prev100MS, prevHalfSecs, nowHalfSecs;	/*	100milli-secsond 단위 시각	*/
	struct timeval	now, prev;
    int         rxQmsgCnt;

#if 1
	int check_Index;
	if( (check_Index = check_my_run_status("SAMD")) < 0)
		exit(0);
#endif 

	if(InitSys() < 0)
		exit(1);

	// yhshin
	//freopen ("/dev/null", "w", stdout);

	{
		/*	clear previous messages	*/
		char	tmpBuff[65536];
		while(msgrcv(samdQID, tmpBuff, sizeof(tmpBuff), 0, IPC_NOWAIT) > 0);
	}
	get_system_information(INIT_FLOW);	/*	맨 처음 값은 정확한 데이타가 아니므로 버려야 한다	*/

	gettimeofday(&now, NULL);
	prev.tv_sec		= now.tv_sec;
	prev.tv_usec	= now.tv_usec;
	prev100MS		= (now.tv_sec * 10) + (now.tv_usec/100000);
	prevHalfSecs	= (now.tv_sec * 10) + ( ((now.tv_usec - 500000L) < 0) ? 0 : 5);

	//SetUpSignal();

	smShellExe(); /*SM 쉘 실행..*/

    //ping test
    thread_ping_test ();

	// mmc process
	// thread -> main : sjjeon 20090921
	//get_rxQ_read_thread();

	// dup
	dup_check_thread();

	while(!g_dStopFlag)
	{
		QClear();
#ifndef _USE_MMC_THREAD_
		// 20090921 sjjeon
		// kill-prc 문제로 thread에서 처리하던 것을 main에서 처리함.
        rxQmsgCnt = HandleRxMsg();                                                  
		//if (rxQmsgCnt < 0) continue;                                                
		//if (rxQmsgCnt == 0) commlib_microSleep(1000);
#endif
		commlib_microSleep(1000);

		gettimeofday(&now, NULL);
		now100MS	= (now.tv_sec * 10) + (now.tv_usec/100000);
		nowHalfSecs	= (now.tv_sec * 10) + (((now.tv_usec - 500000L) < 0) ? 0 : 5);
		// 100MS 단위 시간이 바뀌지 않으면 아무것도 안한다.
		if (prev100MS == now100MS)
			continue;

		//----------------------------------------------------------------------
		/*	100MS 단위 시간으로 작업해야 할 일들	*/
		prev100MS = now100MS;

		// jobs by a second
		if (prev.tv_sec == now.tv_sec)
			continue;

		//----------------------------------------------------------------------
		/* 초 단위 시간으로 작업해야 할 일들	*/
		prev.tv_sec  = now.tv_sec;
		prev.tv_usec = now.tv_usec;

		keepalivelib_increase();

		/*	프로세스 상태 수집 및 auto_restart	*/
		checkProcessStatus();

		/*	watch-dog (keepalive) 기능 수행		*/
		checkKeepAlive();

		/*	CPU, Memory 점유율 정보 수집			*/
		if(nowHalfSecs > prevHalfSecs)
		{
			prevHalfSecs = nowHalfSecs;
			get_system_information(NORMAL_FLOW);
			commlib_microSleep(20000);
		}

		/*	disk 사용율 정보 수집	*/
		get_diskUsage();

		/*  disk h/w 정보 up/down 상태 */
		if(prev.tv_sec%2) {
			check_disk_status();
			dCheckHW_SysMsg();	// by june, 20100312
		}

		get_queUsage();		//2006.06.22 by helca

		/*	collect the status of NTP Daemon & remote NTP Server	*/
		CheckNTPStS();

		/*	1초이상 주기로 작업할 일들						*/
		/*	1초마다 system message queue full 감시	*/

		if(now.tv_sec % 5 == 0)
		{
			link_info(); /* sun solaris link-info : sjjeon*/

		#if defined(__HPLOG__) || defined(__LINUX__)
			/* To check links' connection status is included in bond_Result() */
			//			link_info();
			/* -------------------------------------------------------------- */
			cpu_sts_info();
		#endif

			/* SCM H/W FAN/POWER 상태정보 : sjjeon*/
			//getFanPwrInfo();
			//getFanPwrInfo2();

			/*	SAMD에서 관리하는 cpu, memory, disk, lan, process 상태를 전송한다.	* */
			report_sadb2FIMD();

//			if(loc_sadb->loc_system_dup.myLocalDupStatus == 1)
//				no_Transmitted_udr_check();
//			else if(loc_sadb->loc_system_dup.myLocalDupStatus != 1)
//			{
//				alarm_flag		= 0;
//				prev_alarm_flag	= 0;
//			}
		}


	#if 0
		/*	DATA, BIN directory sync to the other.	*/
		if(now.tv_sec%10 == 0)	// because of CPU usage, comment out
			sync_files_to_remote();
	#endif
	} //-- end of while(1) --//

	//checkProcessStatus();
	//FinishProgram();

	return 0;
}

#ifdef __LINUX__
void bond_Result(void)
{
	#define MII_STRING		"MII Status"
	#define SLAVE_STRING	"Slave Interface"

	char			bondfile[128], buff[128];
	char			*result, *ethname, *pSlash;
	int				i, index, status, bondStatus[MAX_BOND_NUM], linkStatus[SFM_HW_MAX_LINK_NUM];
	struct stat		bondstat;
	FILE			*fp;

	/* bonding/bond0 file's content
		Bonding Mode: adaptive load balancing
		Primary Slave: None
		Currently Active Slave: eth2
		MII Status: up
		MII Polling Interval (ms): 80
		Up Delay (ms): 0
		Down Delay (ms): 0

		Slave Interface: eth0
		MII Status: up
		Link Failure Count: 4
		Permanent HW addr: 00:16:35:80:ff:c9

		Slave Interface: eth2
		MII Status: up
		Link Failure Count: 4
		Permanent HW addr: 00:11:0a:5b:b7:ca
	*/

	for(i = 0; i < MAX_BOND_NUM; i++)
		bondStatus[i] = 1;

	for(i = 0; i < SFM_HW_MAX_LINK_NUM; i++)
		linkStatus[i] = 1;

	for(i = 0; i < MAX_BOND_NUM; i++)
	{
		sprintf(bondfile, "/proc/net/bonding/bond%d", i);
		if(stat(bondfile, &bondstat) < 0)
		{
			sprintf(trcBuf, "[bond_Result]Bonding file not exist - %s\n", bondfile);
			trclib_writeLogErr(FL, trcBuf);
			// exception handle
			continue;
		}

		fp = fopen(bondfile, "r");
		if(!fp)
		{
			// except handle
			sprintf(trcBuf, "[bond_Result]Bonding file open error - %s, %s\n",
			strerror(errno), bondfile);
			trclib_writeLogErr(FL, trcBuf);
			continue;
		}

		index = -1; //
		while(fgets(buff, sizeof(buff), fp))
		{
			if(!strncasecmp(buff, MII_STRING, strlen(MII_STRING)))
			{
				result = strchr(buff, ':');
				result++;

				while(isspace(*(result))) result++;

				if(!strncasecmp(result, "up", 2))
					status = SFM_LAN_CONNECTED; // UP
				else
					status = SFM_LAN_DISCONNECTED; // DOWN

				if(index < 0)
				{ // bond status -> MII Status: up
					pSlash = strrchr(bondfile, '/');
					pSlash++;
					strcpy(loc_sadb->IF_bond[i].bondName, pSlash);
					bondStatus[i] = status;
				}
				else
				{         // Slave(eth) status -> MII Status: up
					linkStatus[index] = status;
					sprintf(loc_sadb->sysSts.linkSts[index].StsName, "link%d", index);
				}
			}
			else if(!strncasecmp(buff, SLAVE_STRING, strlen(SLAVE_STRING)))
			{
				ethname = strchr(buff, ':');
				ethname++;

				while(isspace(*(ethname))) ethname++;

				if(!strncasecmp(ethname, "eth", 3))
					index = atoi(ethname+3);
			}
		}
		fclose(fp);
	}

	for(i = 0; i < MAX_BOND_NUM; i++)
		loc_sadb->IF_bond[i].status = bondStatus[i];

	for(i = 0; i < SFM_HW_MAX_LINK_NUM; i++)
		loc_sadb->sysSts.linkSts[i].status = linkStatus[i];
}
#endif




#if 1 
/* SUN Solaris 10, HardWard Link Info : sjjeon*/
/*
link_test()를 link_info로 통합하여 데이터 전송함.
sun solaris 10 에서는 "dladm show-dev"를 통하여 link 정보를 획득.
 
- link 정보는 아래와 같다. 8,9 번항목은 따로 추가 함..
- 순서 : 내용
  0~7 : link정보 (e1000g0 ~ e1000g7)
  8 :  mysql
  9 :  timetens 
  10 : sm process
  11 : cm process
*/
void link_info(void)
{
	int rv;
//  int i;
//	FILE *fp = NULL;
//	char buff[512],buff2[512];//, *token;
//	charcmd[32];
//	char statFILE[] = "/DSC/NEW/STATUS/HW_LINK_STS";
//  gSCMCheck ++;

	/* mysql live check....: sjjeon*/
	rv = mysqlLiveCheck();
	if(rv ==0){
		loc_sadb->sysHW[get_hwinfo_index ("mysql")].status = 0; // running
	}
	else{
		loc_sadb->sysHW[get_hwinfo_index ("mysql")].status = 1; // dead
	}


	/* timetens live check : sjjeon */
	rv = tt_liveCheck();
	if(rv ==0){
		loc_sadb->sysHW[get_hwinfo_index ("timesten")].status = 0; // running
	}
	else{
		loc_sadb->sysHW[get_hwinfo_index ("timesten")].status = 1; // dead
	}


	return;
}


#else
void link_info(void)
{
    char    *env, target[32], command[256], fname[256], lineBuf[1024];
    char    *pc;
    FILE    *fp;
    int     eth_line, eth_flag, index, i;

    if ((env = getenv(IV_HOME)) == NULL)
        return;

    sprintf (fname, "%s/TMP/%s_eth_sts", env, mySysName);
    sprintf (command, "/sbin/ifconfig -a > %s", fname );
    my_system (command);

    if ((fp = fopen (fname, "r")) == NULL) {
        fprintf (stderr, "[link_test] fopen fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno));
        return;
    }

    eth_flag = 0;
    eth_line = 0;

    for(i = 0; i < SFM_HW_MAX_LINK_NUM; i++)
        loc_sadb->sysSts.linkSts[i].status = 1;

    while (fgets (lineBuf, sizeof(lineBuf), fp) != NULL) {
        // eth0      Link encap:Ethernet  HWaddr 00:16:35:80:FF:C9
        //           inet addr:192.203.140.161  Bcast:192.203.140.255  Mask:255.255.255.0
        //            UP BROADCAST RUNNING SLAVE MULTICAST  MTU:1500  Metric:1

        if(!strncasecmp(lineBuf, "eth", 3)){
            eth_flag = 1;
            eth_line = 0;
            index = lineBuf[3] - '0';
        }

        if(eth_flag && eth_line == 2){
            pc = lineBuf;
            while(isspace(*pc)) pc++;
            if(!strncasecmp(pc, "UP", 2))
				loc_sadb->sysSts.linkSts[index].status = 0;
            else
				loc_sadb->sysSts.linkSts[index].status = 1;
            sprintf(loc_sadb->sysSts.linkSts[index].StsName, "link%d", index);
        }
        eth_line++;
    }

    fclose(fp);
    unlink(fname);
}
#endif

/*
   mysql live check
    0 : running
    1 : not running
*/
int mysqlLiveCheck()    /* mysql live check : sjjeon */
{
    const int _BUFSIZ = 512;
    char cmd[100], out_file[64];
    char buf[_BUFSIZ], trcBuf[256];
    FILE *fp=NULL;
	int	rv = 0, retry_cnt=0;
    //int i=0;

	bzero(cmd, sizeof(cmd));
	bzero(out_file, sizeof(out_file));
    memset(buf,0,sizeof(buf));

	sprintf(out_file, "/etc/MysqlLiveCheck");
	sprintf(cmd , "/etc/init.d/mysql status > %s", out_file);

RETRY_CHECK:

    // retry 실패..
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
			//printf("mysql running.. OK\n");
			rv =0;
			goto FINISH;
		}
    }

	rv=1;
	//fprintf(stderr,"mysql not running.. OK\n");
	sprintf(trcBuf, "[%s] mysql not running.\n",__FUNCTION__ );
	trclib_writeLogErr(FL, trcBuf);

FINISH:
    if(fp) fclose(fp);
    return rv;
}

/*
    timesten live check
    0 : running
    1 : not running
*/
int tt_liveCheck()
{
    const int _BUFSIZ = 512;
    char cmd[100], out_file[64];
    char buf[_BUFSIZ];
    char para[3][16];
    FILE *fp=NULL;
    int rv=1, retry_cnt=0;

	sprintf(out_file, "/etc/TimestenLiveCheck");
	sprintf(cmd , "/DSC/NEW/STATUS/timesten.sh > %s", out_file);

    bzero(buf,sizeof(buf));

RETRY_CHECK:

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
        sprintf(trcBuf, "[%s] fopen fail..\n",__FUNCTION__);
        trclib_writeLogErr (FL,trcBuf);
        retry_cnt++;
		goto RETRY_CHECK;
    }

    while (fgets(buf,_BUFSIZ, fp) != NULL)
    {
        /* run example: 
			/SM/pcube/lib/tt/TimesTen/pcubesm22/bin/ttStatus > grep "Daemon"
		   para[0] para[1] para[2]
		   Daemon pid 12997 port 17001 instance pcubesm22 
		 */
        if(strstr(buf,"Daemon")!=NULL) {
            sscanf(buf, "%s %s %s", para[0], para[1], para[2]);
			//fprintf(stderr, "==> %s %s \n", para[1], para[2]);
            if(atoi(para[2])>0) rv=0;
            goto FINISH;
        }
    }
	rv = 1;

FINISH:
	if(fp) fclose(fp);
	return rv;
}
/*End of tt_liveCheck*/


#if defined(__LINUX__) || defined(__HPLOG__)
#ifdef __HPLOG__
void cpu_sts_info(void)
{
	int     index;
	char	*env, command[256], fname[256], lineBuf[1024], token[10][15];
	FILE	*fp;

	if( (env = getenv(IV_HOME)) == NULL)
		return;

	sprintf(fname, "%s/TMP/%s_CPU_test", env, mySysName);
	sprintf(command, "/sbin/hplog -t > %s", fname );
	system(command);

	if( (fp = fopen (fname, "r")) == NULL)
	{
		sprintf(trcBuf, "%s: fopen fail[%s]; err=%d(%s)\n", __FUNCTION__, fname, errno, strerror(errno));
		trclib_writeLogErr(FL, trcBuf);
		return;
	}

	/*	1  Maxim 1617   CPU (1)         Nominal  105F/ 41C 176F/ 80C	*/
	while(fgets(lineBuf, sizeof(lineBuf), fp) != NULL)
	{
		index	= 0;
		memset(token, 0x00, sizeof(token));
		sscanf(lineBuf, "%s %s %s %s %s %s %s %s %s %s", token[0], token[1], token[2], token[3], token[4], token[5], token[6], token[7], token[8], token[9]);

		if(!strncasecmp(token[3], "CPU", 3))
		{
			index	= token[4][1] - ('0' + 1);
			if(!strncmp(token[5], "Nominal", 7))
				loc_sadb->sysSts.cpuSts[index].status	= 0;	//	NORMAL
			else
				loc_sadb->sysSts.cpuSts[index].status	= 1;	//	ABNORMAL

			sprintf(loc_sadb->sysSts.cpuSts[index].StsName,"cpu%d",index);
		}
	}
	fclose(fp);

	unlink(fname);
}
#else
void cpu_sts_info(void)
{
	int		i, index;
	char    *env, target[32], command[256], fname[256], lineBuf[1024], token[5][15];
	FILE    *fp;

	index	= 0
	if( (env = getenv(IV_HOME)) == NULL)
		return;

	sprintf(fname, "%s/TMP/%s_CPU_test", env, mySysName);
	sprintf(command, "grep processor /proc/cpuinfo > %s", fname );
	my_system(command);
//printf("cpu_sts_info call\n");
	if( (fp = fopen(fname, "r")) == NULL)
	{
		sprintf(trcBuf, "%s: fopen fail[%s]; err=%d(%s)\n", __FUNCTION__, fname, errno, strerror(errno));
		trclib_writeLogErr(FL, trcBuf);
		return;
	}

	for(i = 0; i < SFM_HW_MAX_CPU_NUM; i++)
		loc_sadb->sysSts.cpuSts[i].status = 1; // ABNORMAL

	while(fgets(lineBuf, sizeof(lineBuf), fp) != NULL)
	{
		memset(token, 0x00, sizeof(token));
		sscanf(lineBuf, "%s %s %s %s %s ", token[0], token[1], token[2], token[3], token[4]);

		if(!strcasecmp(token[0], "processor"))
		{
		//	fprintf(stderr, "lineBuf: %s\n", lineBuf);
			index	= atoi(token[2]);
			sprintf(loc_sadb->sysSts.cpuSts[index].StsName, "cpu%d", index);
			loc_sadb->sysSts.cpuSts[index].status = 0; // NORMAL
		}
		else
			continue;
	}
	fclose(fp);

	unlink(fname);
}
#endif
#endif


int get_hwinfo_index (char *hwname) 
{
	int i;
	for (i=0; i<SFM_MAX_HPUX_HW_COM; i++) {
		if (!strcasecmp(loc_sadb->sysHW[i].StsName, hwname)) {
			return i;
		}
	}

	return -1;
}
 
void set_hwinfo_sts (int idx, char status) 
{
	loc_sadb->sysHW[idx].status = status;
}

/*
    SM live check 
    0 : running
    1 : not running
	-1: error
*/
int smLiveCheck()
{
    const int       _BUFSIZ = 512;
    char            buf[_BUFSIZ];
    int             rv=0,i=0,retry_cnt=0;
    char            logbuf[256];

    FILE            *fp=NULL;
    char            cmd[256], outfile[80];

    bzero(cmd, sizeof(cmd));
    bzero(outfile, sizeof(outfile));


_RETRY_:
	rv = 1; // run fail

	// RETRY FAIL
	if(retry_cnt>3){
		set_hwinfo_sts (i, SFM_HW_STATUS_DEAD);
		sprintf(logbuf, "%s: retry fail.[%d].\n", __FUNCTION__,retry_cnt);
		trclib_writeLogErr(FL, logbuf);
		goto GO_OUT;
	}

	sprintf (outfile, "/tmp/smlivecheck.txt");
    /* SM Live check 쉘을 기동하여 결과를 읽어온다. */
    sprintf (cmd,  "/usr/local/bin/sudo -u pcube /SM/pcube/sm/server/bin/p3sm --sm-status 2>&1 > %s", outfile);
	my_system(cmd);

	if ((i = get_hwinfo_index ("sm")) < 0) {
        sprintf(logbuf, "%s: not found sm process name.\n", __FUNCTION__);
        trclib_writeLogErr(FL, logbuf);
       	goto GO_OUT;
	}

	if ((fp = fopen (outfile, "r")) == NULL) {
		sprintf(logbuf, "%s: cmd popen fail[%s]. err=%d(%s)\n", __FUNCTION__, cmd, errno, strerror(errno));
		trclib_writeLogErr(FL, logbuf);
		retry_cnt++;
		goto _RETRY_;
	}

    bzero(buf, _BUFSIZ);                                                               
                                                                                       
    while (fgets(buf,_BUFSIZ, fp) != NULL)                                             
    {                                                                                  
        if(strstr(buf,"Active") || strstr(buf,"Standby")) {                                                    
            rv = 0; // running                                                         
            break;
        }
	} /*End of while*/                                                                 

GO_OUT:
	set_hwinfo_sts (i, rv);
    if(fp) fclose(fp);                                                                 
    return rv;        
}
/*End of smLiveCheck*/


/*
sjjeon
SM 쉘 구동..
*/
void smShellExe()
{
    pthread_attr_t  thrAttr;
    pthread_t   thrId;
    int ret;
	char			logBuf[256];
    
    pthread_attr_init ( &thrAttr);
    pthread_attr_setscope ( &thrAttr, PTHREAD_SCOPE_SYSTEM);
    pthread_attr_setdetachstate ( &thrAttr, PTHREAD_CREATE_DETACHED);
    
    if ((ret = pthread_create (&thrId, &thrAttr, thread_SM_MainCheck, NULL)) != 0) {
        sprintf(logBuf,"[%s] pthread_create fail\n",__FUNCTION__ );
        trclib_writeLogErr (FL,logBuf);
    }

}

/* SM status check main*/
void *thread_SM_MainCheck(void *arg)
{
	int rv; 

	while(1)
	{
		smLiveCheck(); // sm live check


		/* cm live check : sjjeon */
		rv = cmLiveCheck();
		if(rv ==0)
			loc_sadb->sysHW[get_hwinfo_index ("cm")].status = 0; // running
		else
			loc_sadb->sysHW[get_hwinfo_index ("cm")].status = 1; // not running


		smNetStatus(); // sm connect, sync status
		sleep(1);
	}

	return NULL;
}

/*
   by sjjeon 
   SM Net Status (Sync, Connection)
 */
int smNetStatus()
{

	const int       _BUFSIZ = 512;
	char            buf[_BUFSIZ], logbuf[256];
	FILE            *pp=NULL;
	char			cmd[128], temp[6][10];
	int             i=0, retry_cnt=0, rv, fail_check_cnt=0;
	unsigned char	isActive;
	char            outfile[80];
	int				SMA_CONN_STAT, SMA_SYNC_STAT, SMB_CONN_STAT, SMB_SYNC_STAT;

	// 초기화 
	bzero(buf, sizeof(buf));
	bzero(cmd, sizeof(cmd));
	bzero(logbuf, sizeof(logbuf));
	bzero(temp, sizeof(temp));

	// 초기화 fail 설정.
	SMA_CONN_STAT=SMA_SYNC_STAT=SMB_CONN_STAT=SMB_SYNC_STAT=1;

RETRY_:
	sprintf (outfile, "/tmp/smconnlivecheck.txt");
	sprintf(cmd, "/DSC/NEW/STATUS/sm-conn-sync-sts.sh | grep SCE | awk '{print $2,$6,$8}' 2>&1 > %s", outfile);
	my_system (cmd);

	// initialize
	if(pp) fclose(pp); //re-try 했을때...
	pp = NULL;
	fail_check_cnt=0;

	// RETRY 시도수가 3회 이상이면 실패로 간주한다. 
	if(retry_cnt>=3){
		sprintf(logbuf, "[%s]  SM status check fail. re-try=%d.(cmd:%s)\n",
				__FUNCTION__,retry_cnt, cmd);
		trclib_writeLogErr(FL, logbuf);

		rv = -1;
		goto OUT;
	}


	// fopen으로 쉘 명령어 수행
	pp = fopen(outfile, "r");
	if(pp == NULL){
		sprintf(logbuf, "[%s] System fopen fail[%s]. err=%d(%s)\n",__FUNCTION__, cmd, errno, strerror(errno));
		trclib_writeLogErr(FL, logbuf);
		retry_cnt++;
		goto RETRY_;
	}

	while(fgets(buf,_BUFSIZ,pp) !=NULL)
	{
		if(strstr(buf,"SCE") != NULL){
			sscanf(buf, "%s %s %s", temp[i*3+0],temp[i*3+1],temp[i*3+2]);
			//fprintf(stderr, "READ : %s %s %s\n", temp[i*3+0],temp[i*3+1],temp[i*3+2]);
			i++;
		}
	}

	// 정보를 얻어오는 횟수가 적으면 재시도 한다. (2 count)
	if(i!=2){
		sprintf(logbuf, "[%s]  SM status check fail(i is not 2 count). re-try=%d.(cmd:%s)\n",
				__FUNCTION__,retry_cnt, cmd);
		trclib_writeLogErr(FL, logbuf);

		retry_cnt++;
		goto RETRY_;
	}

	/*
	 **** 결 과 ****
	 SCEA up done
	 SCEB up done

	 또는 

	 SCEA down not-done
	 SCEB down not-done
	 *************

	 SCEA_CONN=12, SCEA_SYNC=13, SCEB_CONN=14, SCEB_SYNC=15;
	 ..
	 .down도 normal로 처리 단, SM이 죽어 있을 경우는 조회가 되지 않는다. (Major)
	 */
	isActive = loc_sadb->loc_system_dup.myLocalDupStatus;

	if(!strncasecmp(temp[0], "SCEA", 4))
	{
		if((isActive == 1 /*Active*/) && (!strncasecmp(temp[1],"up",2))){
			//loc_sadb->sysHW[get_hwinfo_index ("SMCONN_A")].status = SFM_HW_STATUS_ALIVE;
			SMA_CONN_STAT = SFM_HW_STATUS_ALIVE;
		} else if((isActive == 2) && (!strncasecmp(temp[1],"down",4))){
			//	loc_sadb->sysHW[get_hwinfo_index ("SMCONN_A")].status = SFM_HW_STATUS_ALIVE;
			SMA_CONN_STAT = SFM_HW_STATUS_ALIVE;
		} else {
			//loc_sadb->sysHW[get_hwinfo_index ("SMCONN_A")].status = SFM_HW_STATUS_DEAD;
			SMA_CONN_STAT = SFM_HW_STATUS_DEAD;
			sprintf(logbuf, "[%s]  SMCONN_A DEAD.(don't check status.)\n", __FUNCTION__);
			trclib_writeLogErr(FL, logbuf);
			fail_check_cnt++;
		}

		if((isActive == 1 /*Active*/) && (!strncasecmp(temp[2],"done",4))){
			//loc_sadb->sysHW[get_hwinfo_index ("SMSYNC_A")].status = SFM_HW_STATUS_ALIVE;
			SMA_SYNC_STAT = SFM_HW_STATUS_ALIVE;
		} 
		else if((isActive == 2) && (!strncasecmp(temp[2],"not-done",8))){
			//loc_sadb->sysHW[get_hwinfo_index ("SMSYNC_A")].status = SFM_HW_STATUS_ALIVE;
			SMA_SYNC_STAT = SFM_HW_STATUS_ALIVE;
		} else {
			//loc_sadb->sysHW[get_hwinfo_index ("SMSYNC_A")].status = SFM_HW_STATUS_DEAD;
			SMA_SYNC_STAT = SFM_HW_STATUS_DEAD;
			sprintf(logbuf, "[%s]  SMSYNC_A DEAD.(don't check status.)\n", __FUNCTION__);
			trclib_writeLogErr(FL, logbuf);
			fail_check_cnt++;
		}
	}else{
		sprintf(logbuf, "[%s]  SM_A CONN/SYNC DEAD.\n", __FUNCTION__);
		trclib_writeLogErr(FL, logbuf);
		fail_check_cnt++;

		//loc_sadb->sysHW[get_hwinfo_index ("SMCONN_A")].status = SFM_HW_STATUS_DEAD;
		//loc_sadb->sysHW[get_hwinfo_index ("SMSYNC_A")].status = SFM_HW_STATUS_DEAD;
		SMA_CONN_STAT = SFM_HW_STATUS_DEAD;
		SMA_SYNC_STAT = SFM_HW_STATUS_DEAD;
	}

	//	fprintf(stderr,"\n(name:conn_sts:sync_sts)(%s:%d:%d)\n",
	//			 loc_sadb->sysSts.linkSts[SCEA_CONN].StsName,
	//			 loc_sadb->sysSts.linkSts[SCEA_CONN].status,
	//			 loc_sadb->sysSts.linkSts[SCEA_SYNC].status);

	if(!strncasecmp(temp[3], "SCEB",4))
	{

		if((isActive == 1 /*Active*/) && (!strncasecmp(temp[4],"up",2))){
			//loc_sadb->sysHW[get_hwinfo_index ("SMCONN_B")].status = SFM_HW_STATUS_ALIVE;
			SMB_CONN_STAT = SFM_HW_STATUS_ALIVE;
		} else if((isActive == 2) && (!strncasecmp(temp[4],"down",4))){
			//loc_sadb->sysHW[get_hwinfo_index ("SMCONN_B")].status = SFM_HW_STATUS_ALIVE;
			SMB_CONN_STAT = SFM_HW_STATUS_ALIVE;
		} else {
			//loc_sadb->sysHW[get_hwinfo_index ("SMCONN_B")].status = SFM_HW_STATUS_DEAD;
			SMB_CONN_STAT = SFM_HW_STATUS_DEAD;
			sprintf(logbuf, "[%s]  SMCONN_B DEAD.(don't check status.)\n", __FUNCTION__);
			trclib_writeLogErr(FL, logbuf);
			fail_check_cnt++;
		}

		if((isActive == 1 /*Active*/) && (!strncasecmp(temp[5],"done",4))){
			//loc_sadb->sysHW[get_hwinfo_index ("SMSYNC_B")].status = SFM_HW_STATUS_ALIVE;
			SMB_SYNC_STAT = SFM_HW_STATUS_ALIVE;
		} else if((isActive == 2) && (!strncasecmp(temp[5],"not-done",8))){
			//loc_sadb->sysHW[get_hwinfo_index ("SMSYNC_B")].status = SFM_HW_STATUS_ALIVE;
			SMB_SYNC_STAT = SFM_HW_STATUS_ALIVE;
		} else {
			//loc_sadb->sysHW[get_hwinfo_index ("SMSYNC_B")].status = SFM_HW_STATUS_DEAD;
			SMB_SYNC_STAT = SFM_HW_STATUS_DEAD;
			sprintf(logbuf, "[%s]  SMSYNC_B DEAD.(don't check status.)\n", __FUNCTION__);
			trclib_writeLogErr(FL, logbuf);
			fail_check_cnt++;
		}
	}else{
		sprintf(logbuf, "[%s]  SM_B CONN/SYNC DEAD.\n", __FUNCTION__);
		trclib_writeLogErr(FL, logbuf);
		fail_check_cnt++;
		//loc_sadb->sysHW[get_hwinfo_index ("SMCONN_B")].status = SFM_HW_STATUS_DEAD;
		//loc_sadb->sysHW[get_hwinfo_index ("SMSYNC_B")].status = SFM_HW_STATUS_DEAD;
		SMB_CONN_STAT = SFM_HW_STATUS_DEAD;
		SMB_SYNC_STAT = SFM_HW_STATUS_DEAD;
	}

	if(fail_check_cnt!=0){
		int j;
		// fail check error count가 1개라도 나오면 re-try.
		sprintf(logbuf, "[%s] fail check error.(count : %d) \n", __FUNCTION__, fail_check_cnt);
		trclib_writeLogErr(FL, logbuf);
		retry_cnt++;

		for(j=0; j<6; j++){
			sprintf(logbuf, "[%s] INFO : %s,%s,%s \t%s,%s,%s\n", 
					__FUNCTION__, temp[0],temp[1],temp[2],temp[3],temp[4],temp[5]);
			trclib_writeLogErr(FL, logbuf);
			bzero(temp, sizeof(temp));
		}
		goto RETRY_;
	}

	//	fprintf(stderr,"\n(name:conn_sts:sync_sts)(%s:%d:%d)\n",
	//			 loc_sadb->sysSts.linkSts[SCEB_CONN].StsName,
	//			 loc_sadb->sysSts.linkSts[SCEB_CONN].status,
	//			 loc_sadb->sysSts.linkSts[SCEB_SYNC].status);
	rv=0;

OUT:
	// 마지막에 상태 반영한다.
	loc_sadb->sysHW[get_hwinfo_index ("SMCONN_A")].status = SMA_CONN_STAT;
	loc_sadb->sysHW[get_hwinfo_index ("SMSYNC_A")].status = SMA_SYNC_STAT;
	loc_sadb->sysHW[get_hwinfo_index ("SMCONN_B")].status = SMB_CONN_STAT;
	loc_sadb->sysHW[get_hwinfo_index ("SMSYNC_B")].status = SMB_SYNC_STAT;

	if(pp) fclose(pp);
	return rv;

}
/* End of smNetStatus() */

/*
    CM live check
    0 : running
    1 : not running
	-1: error
*/
int cmLiveCheck()    /* CM process live check : sjjeon */
{
	int pid_t;
	pid_t=getCM_AdapPid();

	if(pid_t) return 0;
	return 1;
}

/*
	by sjjeon
	SUN T2000 서버 FAN/POWER 상태 체크 
 */

int getFanPwrInfo(void)
{
	int i=0,tmp_value[5],trc_len=0;
	const int _BUFSIZE = 512;
	const int _MAX_BUFSIZE = 4086; // 4kbyte trace buffer
	char  buf[_BUFSIZE], logBuf[1024], trc_log[_MAX_BUFSIZE];
	char  cmd[128], syntax[2][20];
	FILE  *fp=NULL;
	int  re_try =0, rv=0, fail_flag=0; 
	int open_cnt=0;


#ifdef _FILE_OPEN_
	char  outfile[80];
	bzero(cmd, sizeof(cmd));
	bzero(outfile, sizeof(outfile));
	sprintf (outfile, "/tmp/fanpwrcheck.txt");
	// SCMA/SCMB 의 FAN/POWER 상태 체크 
	if(!strncasecmp(mySysName,"SCMA",4)){
		//sprintf(cmd, "/DSC/NEW/STATUS/dis-scm-fan.sh 0 | awk '\{printf \"%%s %%s\\n\", $1, $2 }' | egrep '^PS|^FT0' 2>&1 > %s",outfile);
		sprintf(cmd, "/DSC/NEW/STATUS/dis-scm-fan.sh 0 | egrep '^PS|^FT0' > %s", outfile);
	}
	else{
		//sprintf(cmd, "/DSC/NEW/STATUS/dis-scm-fan.sh 1 | awk '\{printf \"%%s %%s\\n\", $1, $2 }' | egrep '^PS|^FT0' 2>&1 > %s",outfile);
		sprintf(cmd, "/DSC/NEW/STATUS/dis-scm-fan.sh 1 | egrep '^PS|^FT0' > %s", outfile);
	}
#else
	bzero(cmd, sizeof(cmd));
	if(!strncasecmp(mySysName,"SCMA",4)){
		sprintf(cmd, "/DSC/NEW/STATUS/dis-scm-fan.sh 0 | egrep '^PS|^FT0'");
	}
	else{
		sprintf(cmd, "/DSC/NEW/STATUS/dis-scm-fan.sh 1 | egrep '^PS|^FT0'");
	}
#endif /* End of _FILE_OPEN_*/

RETRY_CHECK:
	bzero(logBuf, sizeof(logBuf));
	bzero(trc_log, _MAX_BUFSIZE);
	bzero(buf, _BUFSIZE);
	bzero(syntax, sizeof(syntax));

	// 2회이상 재시도 실패면 error.
	if (re_try > 2){
		sprintf(logBuf, "[%s] re-try fail(%d)..\n", __FUNCTION__,re_try);
		trclib_writeLogErr(FL, logBuf);
		rv = -1;
		goto GO_OUT;
	}

	for(i=0;i<5;i++) tmp_value[i]=1;  /*default : 1 (down)*/
	/*
		FT0/FM0          OK                3525     --   1920
		FT0/FM1          OK                3466     --   1920
		FT0/FM2          OK                3525     --   1920
		PS0     OK              OFF         OFF       OFF       OFF        OFF
		PS1     OK              OFF         OFF       OFF       OFF        OFF
	*/
#ifdef _FILE_OPEN_
	my_system(cmd);
	fp = fopen(outfile,"r");
#else
	fp = popen(cmd,"r");
#endif
	if(fp == NULL){
		re_try++;
		sprintf(logBuf, "[%s] f(p)open error.(cmd:%s)\n", __FUNCTION__, cmd);
		trclib_writeLogErr(FL, logBuf);
		commlib_microSleep(100000); //retry sleep
		goto RETRY_CHECK;
	}

	open_cnt++;
	//sprintf(logBuf, "[%s] 0.open open_cnt:%d(fp:%p)\n", __FUNCTION__, open_cnt, fp);
	//trclib_writeLogErr(FL, logBuf);

	// initialize
	i=0; fail_flag=0, trc_len=0;
	trc_len += sprintf(trc_log,"[%s] ",__FUNCTION__);

	/*
	by sjjeon
	popen 하여 데이터를 읽어오는 동안 시간차로 인하여,  
	shared memory 영역에 있는 값은 반영되므로 값 설정전에 바로 초기화 한다.
	 */
	while(fgets(buf, _BUFSIZE, fp) != NULL)
	{
		//FAN 1
		sscanf(buf, "%s%s", syntax[0], syntax[1]);

		// error log를 위해 trace로 저장..
		// _BUFSIZE 이하인 data만 trace에 저장.
		if((trc_len<_MAX_BUFSIZE-_BUFSIZE) && (strlen(buf)<_BUFSIZE)){
			trc_len += sprintf(trc_log+trc_len,"%s",buf);
		}

		//if(strstr(syntax[0], "FM0")!=NULL){
		if(!strcmp(syntax[0], "FT0/FM0")!=NULL){
			if(!strcmp(syntax[1], "OK")){
				tmp_value[0]=0;
				i++; 	// OK : index 증가.
			}
			else{
				sprintf(logBuf, "[%s] FM0 fail..re-try :%d (buf:%s)\n", __FUNCTION__, re_try, buf);
				trclib_writeLogErr(FL, logBuf);
				fail_flag++; //fail flag 증가.
			}
		} 
		//FAN 2
		//else if(strstr(syntax[0], "FM1")!=NULL){
		else if(!strcmp(syntax[0], "FT0/FM1")!=NULL){
			if(!strcmp(syntax[1], "OK")){
				tmp_value[1]=0;
				i++; 	// OK : index 증가.
			}
			else{
				sprintf(logBuf, "[%s] FM1 fail..re-try :%d (buf:%s)\n", __FUNCTION__, re_try, buf);
				trclib_writeLogErr(FL, logBuf);
				fail_flag++; //fail flag 증가.
			}
		}
		//FAN 3
	//	else if(strstr(syntax[0], "FM2")!=NULL){
		else if(!strcmp(syntax[0], "FT0/FM2")!=NULL){
			if(!strcmp(syntax[1], "OK")){
				tmp_value[2]=0;
				i++; 	// OK : index 증가.
			}
			else{
				sprintf(logBuf, "[%s] FM2 fail..re-try :%d (buf:%s)\n", __FUNCTION__, re_try, buf);
				trclib_writeLogErr(FL, logBuf);
				fail_flag++; //fail flag 증가.
			}
		}
		//PWR 1
		//else if(strstr(syntax[0], "PS0")!=NULL){
		else if(!strcmp(syntax[0], "PS0")!=NULL){
			if(!strcmp(syntax[1], "OK")){
				tmp_value[3]=0;
				i++; 	// OK : index 증가.
			}
			else{
				sprintf(logBuf, "[%s] PS0 fail..re-try :%d (buf:%s)\n", __FUNCTION__, re_try, buf);
				trclib_writeLogErr(FL, logBuf);
				fail_flag++; //fail flag 증가.
			}
		}
		//PWR 2
		//else if(strstr(syntax[0], "PS1")!=NULL){
		else if(!strcmp(syntax[0], "PS1")!=NULL){
			if(!strcmp(syntax[1], "OK")){
				tmp_value[4]=0; 
				i++; 	// OK : index 증가.
			}
			else{
				sprintf(logBuf, "[%s] PS1 fail..re-try :%d (buf:%s)\n", __FUNCTION__, re_try, buf);
				trclib_writeLogErr(FL, logBuf);
				fail_flag++; //fail flag 증가.
			}
		}else{
			// buf log
			sprintf(logBuf, "[%s]invalid syntax.(i:%d, syntax: %s, result: %s\nbuf : %s\n", 
					__FUNCTION__,i, syntax[0], syntax[1], buf);
			trclib_writeLogErr(FL, logBuf);
		}

		bzero(syntax,sizeof(syntax)); //syntax 초기화 
		bzero(buf, _BUFSIZE); // buf 초기화.
	} /* End of while(buf) */

	// 다시한번 결과값 check
	if(fail_flag==0){
		int j=0;
		for(j=0;j<5;j++){
			if(tmp_value[j]!=0) // 0: OK
				fail_flag++;
		}
	}

	// count == 5가 아니면 재시도..
	// fail_flag 0 이 아니면 재시도..
	if(i!=5 || fail_flag > 0) {
		sprintf(logBuf, "[%s] fan/power check fail.[i:%d(succ:5), fail_flag :%d (normal:0)]\n", 
				__FUNCTION__, i, fail_flag);
		trclib_writeLogErr(FL, logBuf);
		re_try++;
#ifdef _FILE_OPEN_
		if(fp)fclose(fp);
#else
		if(fp){
			pclose(fp);
			--open_cnt;
			sprintf(logBuf, "[%s] 1.popen open_cnt:%d(fp:%p) retry:%d\n"
					, __FUNCTION__, open_cnt, fp, re_try);
			trclib_writeLogErr(FL, logBuf);
		}
#endif
		fp=NULL;

		trclib_writeLogErr(FL, trc_log);
		commlib_microSleep(100000); //retry sleep
		goto RETRY_CHECK;
	}
	rv =0;

GO_OUT:
#ifdef _FILE_OPEN_
	if(fp)fclose(fp);
#else
	if(fp) {
		pclose(fp);
		--open_cnt;
		//sprintf(logBuf, "[%s] 2.pclose open_cnt:%d(fp:%p)\n", __FUNCTION__, open_cnt, fp);
		//trclib_writeLogErr(FL, logBuf);
	}
#endif 
	fp=NULL;

	loc_sadb->sysHW[get_hwinfo_index ("FAN1")].status = tmp_value[0];
	loc_sadb->sysHW[get_hwinfo_index ("FAN2")].status = tmp_value[1];
	loc_sadb->sysHW[get_hwinfo_index ("FAN3")].status = tmp_value[2];
	loc_sadb->sysHW[get_hwinfo_index ("PWR1")].status = tmp_value[3];
	loc_sadb->sysHW[get_hwinfo_index ("PWR2")].status = tmp_value[4];

	//sprintf(logBuf, "[%s] close open_cnt:%d\n", __FUNCTION__, open_cnt);
	//trclib_writeLogErr(FL, logBuf);
	return rv;
}
/*end of getFanPwrInfo*/
#undef _FILE_OPEN_


int getFanPwrInfo2(void)
{
	int BUF_ARR_MAX=10;
	int i=0,tmp_value[5],trc_len=0,j=0;
	//const int _BUFSIZE = 512;
	const int _BUFSIZE = 1024;
	char  abuf[BUF_ARR_MAX][_BUFSIZE], logBuf[1024];
	char  cmd[128], syntax[2][20];
	FILE  *fp=NULL;
	int  re_try =0, rv=0, fail_flag=0; 
	int open_cnt=0, index=0;

#ifdef _FILE_OPEN_
	char outfile[80];
	bzero(outfile, sizeof(outfile));
	sprintf (outfile, "/tmp/fanpwrcheck.txt");
	// SCMA/SCMB 의 FAN/POWER 상태 체크 
	if(!strncasecmp(mySysName,"SCMA",4)){
		//sprintf(cmd, "/DSC/NEW/STATUS/dis-scm-fan.sh 0 | awk '\{printf \"%%s %%s\\n\", $1, $2 }' | egrep '^PS|^FT0' 2>&1 > %s",outfile);
		sprintf(cmd, "/DSC/NEW/STATUS/dis-scm-fan.sh 0 | egrep '^PS|^FT0' > %s", outfile);
	}
	else{
		//sprintf(cmd, "/DSC/NEW/STATUS/dis-scm-fan.sh 1 | awk '\{printf \"%%s %%s\\n\", $1, $2 }' | egrep '^PS|^FT0' 2>&1 > %s",outfile);
		sprintf(cmd, "/DSC/NEW/STATUS/dis-scm-fan.sh 1 | egrep '^PS|^FT0' > %s", outfile);
	}
#else
	if(!strncasecmp(mySysName,"SCMA",4)){
		sprintf(cmd, "/DSC/NEW/STATUS/dis-scm-fan.sh 0 | egrep '^PS|^FT0'");
	}
	else{
		sprintf(cmd, "/DSC/NEW/STATUS/dis-scm-fan.sh 1 | egrep '^PS|^FT0'");
	}
#endif /* End of _FILE_OPEN_*/

RETRY_CHECK:
	bzero(logBuf, sizeof(logBuf));
	bzero(abuf, _BUFSIZE*BUF_ARR_MAX);
	bzero(syntax, sizeof(syntax));


	// 3회이상 재시도 실패면 error.
	if (re_try > 3){
		sprintf(logBuf, "[%s] re-try fail(%d)..\n", __FUNCTION__,re_try);
		trclib_writeLogErr(FL, logBuf);
		rv = -1;
		goto GO_OUT;
	}

	for(i=0;i<5;i++) tmp_value[i]=1;  /*default : 1 (down)*/
	/*
		FT0/FM0          OK                3525     --   1920
		FT0/FM1          OK                3466     --   1920
		FT0/FM2          OK                3525     --   1920
		PS0     OK              OFF         OFF       OFF       OFF        OFF
		PS1     OK              OFF         OFF       OFF       OFF        OFF
	*/
#ifdef _FILE_OPEN_
	my_system(cmd);
	fp = fopen(outfile,"r");
#else
	if(re_try>0)
		fflush(fp);
	fp = popen(cmd,"r");
#endif
	if(fp == NULL){
		re_try++;
		sprintf(logBuf, "[%s] f(p)open error.(cmd:%s)\n", __FUNCTION__, cmd);
		trclib_writeLogErr(FL, logBuf);
		commlib_microSleep(100000); //retry sleep
		goto RETRY_CHECK;
	}

	open_cnt++;
	//sprintf(logBuf, "[%s] start->> popen open_cnt:%d(fp:%p)\n", __FUNCTION__, open_cnt, fp);
	//trclib_writeLogErr(FL, logBuf);

	// initialize
	i=0; fail_flag=0, trc_len=0, index=0;

	/*
	by sjjeon
	popen 하여 데이터를 읽어오는 동안 시간차로 인하여,  
	shared memory 영역에 있는 값은 반영되므로 값 설정전에 바로 초기화 한다.
	 */
	while(fgets(abuf[i], _BUFSIZE, fp) != NULL)
	{
		i++;
		if(i>=BUF_ARR_MAX){
			char tmp[_BUFSIZE];
			while(fgets(tmp, _BUFSIZE, fp) != NULL);
			break;
		}
	}

#ifdef _FILE_OPEN_
	if(fp)fclose(fp);
#else
	if(fp)fflush(fp);
	if(fp)rv=pclose(fp);
	if(rv<0){
		int cnt=0;
		sprintf(logBuf, "[%s] pclose fail : rv=%d\n", __FUNCTION__, rv);
		trclib_writeLogErr(FL, logBuf);
		for(cnt=0;cnt<3;cnt++){
			if(fp)fflush(fp);
			if((rv=pclose(fp))<0){
				sprintf(logBuf, "[%s] retry: %d,pclose fail : rv=%d\n", __FUNCTION__,cnt+1, rv);
				commlib_microSleep(100000); //retry sleep
				continue;
			}
			break;
		}
	}

#endif
	--open_cnt;

	for(j=0; j<i;j++)
	{
		//FAN 1
		sscanf(abuf[j], "%s%s", syntax[0], syntax[1]);

		if(strstr(syntax[0], "FM0")!=NULL){
		//if(!strcmp(syntax[0], "FT0/FM0")!=NULL){
			if(!strcmp(syntax[1], "OK")){
				tmp_value[0]=0;
				index++; 	// OK : index 증가.
			}
			else{
				sprintf(logBuf, "[%s] FM0 fail..re-try :%d (abuf[%d]:%s)\n", 
						__FUNCTION__, re_try,j, abuf[j]);
				trclib_writeLogErr(FL, logBuf);
				fail_flag++; //fail flag 증가.
			}
		} 
		//FAN 2
		else if(strstr(syntax[0], "FM1")!=NULL){
		//else if(!strcmp(syntax[0], "FT0/FM1")!=NULL){
			if(!strcmp(syntax[1], "OK")){
				tmp_value[1]=0;
				index++; 	// OK : index 증가.
			}
			else{
				sprintf(logBuf, "[%s] FM1 fail..re-try :%d (abuf[%d]:%s)\n", 
						__FUNCTION__, re_try, j,abuf[j]);
				trclib_writeLogErr(FL, logBuf);
				fail_flag++; //fail flag 증가.
			}
		}
		//FAN 3
		else if(strstr(syntax[0], "FM2")!=NULL){
//		else if(!strcmp(syntax[0], "FT0/FM2")!=NULL){
			if(!strcmp(syntax[1], "OK")){
				tmp_value[2]=0;
				index++; 	// OK : index 증가.
			}
			else{
				sprintf(logBuf, "[%s] FM2 fail..re-try :%d (abuf[%d]:%s)\n", 
						__FUNCTION__, re_try,j, abuf[j]);
				trclib_writeLogErr(FL, logBuf);
				fail_flag++; //fail flag 증가.
			}
		}
		//PWR 1
		else if(!strcmp(syntax[0], "PS0")!=NULL){
			if(!strcmp(syntax[1], "OK")){
				tmp_value[3]=0;
				index++; 	// OK : index 증가.
			}
			else{
				sprintf(logBuf, "[%s] PS0 fail..re-try :%d (abuf[%d]:%s)\n", 
						__FUNCTION__, re_try,j, abuf[j]);
				trclib_writeLogErr(FL, logBuf);
				fail_flag++; //fail flag 증가.
			}
		}
		//PWR 2
		else if(!strcmp(syntax[0], "PS1")!=NULL){
			if(!strcmp(syntax[1], "OK")){
				tmp_value[4]=0; 
				index++; 	// OK : index 증가.
			}
			else{
				sprintf(logBuf, "[%s] PS1 fail..re-try :%d (abuf[%d]:%s)\n", 
						__FUNCTION__, re_try,j, abuf[j]);
				trclib_writeLogErr(FL, logBuf);
				fail_flag++; //fail flag 증가.
			}
		}else{
			// buf log
			sprintf(logBuf, "[%s]invalid syntax.(i:%d, syntax: %s, result: %s\n abuf[%d] : %s\n", 
					__FUNCTION__,i, syntax[0], syntax[1],j,abuf[j]);
			trclib_writeLogErr(FL, logBuf);
		}

		bzero(syntax,sizeof(syntax)); //syntax 초기화 
		bzero(abuf[j], _BUFSIZE); // buf 초기화.
	} /* End of for(buf) */

	// 다시한번 결과값 check
	if(fail_flag==0){
		int k=0;
		for(k=0;k<5;k++){
			if(tmp_value[k]!=0) // 0: OK
				fail_flag++;
		}
	}

	// count == 5가 아니면 재시도..
	// fail_flag 0 이 아니면 재시도..
	if(index!=5 || fail_flag > 0) {
		sprintf(logBuf, "[%s] fan/power check fail.[index:%d(succ:5), fail_flag :%d (normal:0)]\n", 
				__FUNCTION__, index, fail_flag);
		trclib_writeLogErr(FL, logBuf);
		re_try++;
		commlib_microSleep(100000); //retry sleep
		goto RETRY_CHECK;
	}
	rv =0;

GO_OUT:
	loc_sadb->sysHW[get_hwinfo_index ("FAN1")].status = tmp_value[0];
	loc_sadb->sysHW[get_hwinfo_index ("FAN2")].status = tmp_value[1];
	loc_sadb->sysHW[get_hwinfo_index ("FAN3")].status = tmp_value[2];
	loc_sadb->sysHW[get_hwinfo_index ("PWR1")].status = tmp_value[3];
	loc_sadb->sysHW[get_hwinfo_index ("PWR2")].status = tmp_value[4];

	//sprintf(logBuf, "[%s] end->> pclose open_cnt:%d\n", __FUNCTION__, open_cnt);
	//trclib_writeLogErr(FL, logBuf);
	return rv;
} /* End of getFanPwrInfo2 */
