#include "stmd_proto.h"
#include "sfm_msgtypes.h"

#include <time.h>

int     stmdQid, ixpcQid, condQid, fimdQid, nmsifQid, rlegQid;
char    mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN];
char    trcBuf[4096*4], trcTmp[1024];

int     workFLAG = 0; // job을 수행하는지의 flag
int     rxStatMsgCnt = 0; // 받은 통계 메시지의 개수 
int     max_sts_count;
short   printTIME [STMD_PERIOD_TYPE_NUM];
short   delTIME [STMD_PERIOD_TYPE_NUM];
short   maskITEM  [STMD_MASK_ITEM_NUM][STMD_PERIOD_TYPE_NUM];
//char    updateJOB [MAX_CRONJOB_NUM];
MYSQL   sql, *conn;
MYSQL   sql_Rm, *conn_Rm; // For RDR mysql connection by jjinri 2009.04.18
extern unsigned int mysql_timeout;

OnDemandList    onDEMAND[MAX_ONDEMAND_NUM]; // ondemand 통계 명령을 저장하는 변수
CronList        cronJOB[MAX_CRONJOB_NUM]; // schedule 통계 명령을 저장하는 변수

SFM_sfdb        *sfdb;
SFM_SysCommMsgType   *loc_sadb;

RuleSetList	g_stSCERule[MAX_SCE_NUM];
int g_ruleIdBuf[MAX_RULE_NUM];
int	g_ruleItemCnt;

RuleEntryList g_stSCEEntry[MAX_SCE_NUM];
int g_ruleEntryBuf[MAX_ENTRY_NUM];
char g_ruleEntryName[MAX_ENTRY_NUM][128];
int	g_ruleEntryCnt;
extern char       rsFname[32];
extern char			ver[8];

PDSN_LIST         g_stPdsn[2];
int              g_PdsnCnt;
SMSC_LIST         g_stSmsc[MAX_SMSC_NUM];
int              g_SmscCnt;

SVC_ALM          g_stSvcAlm; 
SVC_VAL          g_stSvcVal;

extern int initSmsc(void);


extern  int  trcFlag, trcLogFlag, trcLogId, trcErrLogId;

StmdMmcHdlrVector   mmcHdlrVector[STMD_MAX_MMC_HANDLER] =
{
	{"stat-load",           	stmd_mmc_stat_load},
	{"stat-fault",          	stmd_mmc_stat_fault},
	{"stat-link",          		stmd_mmc_stat_link},
	{"stat-rule-set",    		stmd_mmc_stat_rule_set},
	{"stat-rule-ent",    		stmd_mmc_stat_rule_ent},
	{"stat-account",           	stmd_mmc_stat_leg},
	{"stat-logon",           	stmd_mmc_stat_logon},
	{"stat-flow",           	stmd_mmc_stat_flow},	// 2010.08.23
	{"stat-sms",          		stmd_mmc_stat_sms},
#ifdef DELAY
	{"stat-delay",          	stmd_mmc_stat_delay2},
#else
	{"stat-delay",          	stmd_mmc_stat_delay},
#endif
	{"set-svc-alm",     		stmd_mmc_set_svc_alm},
	{"dis-svc-alm",     		stmd_mmc_dis_svc_alm},
	{"add-pdsn-info",     		stmd_mmc_add_pdsn_info},
	{"dis-pdsn-info",       	stmd_mmc_dis_pdsn_info},
	{"del-pdsn-info",       	stmd_mmc_del_pdsn_info},
	{"chg-pdsn-info",       	stmd_mmc_chg_pdsn_info},
	{"canc-exe-cmd",        	stmd_mmc_canc_exe_cmd},
	{"mask-stat-item",      	stmd_mmc_mask_stat_item},
	{"umask-stat-item",     	stmd_mmc_umask_stat_item},
	{"dis-stat-mask",     		stmd_mmc_dis_stat_mask},
};
int     numMmcHdlr  = 33;
/* // stmd 부가 기능 
	{"add-stat-schd",     		stmd_mmc_add_stat_schd},
	{"del-stat-schd",       	stmd_mmc_del_stat_schd},
	{"dis-stat-schd",       	stmd_mmc_dis_stat_schd},
	{"dis-stat-his",        	stmd_mmc_dis_stat_his},
	{"dis-stat-info",       	stmd_mmc_dis_stat_info},
	{"canc-exe-cmd",        	stmd_mmc_canc_exe_cmd},
	{"dis-stat-nms",        	stmd_mmc_dis_stat_nms},
	{"dis-stat-ptime",      	stmd_mmc_dis_stat_ptime},
	{"chg-stat-ptime",      	stmd_mmc_chg_stat_ptime},
*/

// cronJOB 
char    strITEM[STMD_MASK_ITEM_NUM][14] =
{
	STMD_STR_FAULT,
	STMD_STR_LOAD,
	STMD_STR_LINK,
	STMD_STR_LEG,
	STMD_STR_LOGON,
	STMD_STR_FLOW,		// 2010.08.23
	STMD_STR_RULE_SET,
	STMD_STR_RULE_ENT,
#ifdef DELAY
	STMD_STR_SMS
#else
	STMD_STR_SMS,
	STMD_STR_DEL
#endif
};


int     txStaticCnt = 0; /* 통계 요구 메시지를 보내야 할 프로세스 갯수 */
int     sysCnt  = 0; /* 통계 관련 시스템에 관한 정보 */
STM_CommStatisticInfo   StatisticSystemInfo[SYSCONF_MAX_ASSO_SYS_NUM];
STMD_StatisticProcessInfo   txStaticsProcInfo[SYSCONF_MAX_ASSO_SYS_NUM*SYSCONF_MAX_APPL_NUM];

int stmd_SelectINI_Values(void);
extern int initPdsn(void);

SCE_t	g_stSCE[2];
int		SCE_CNT;

void sendCondTest(void);

int main(void)
{
    int     ret, check_Index;
    int     min, sec;
    GeneralQMsgType rxGenQMsg;
	time_t t_now, t_start; // by jjinri 2009.04.18
	int aliveLo = -1; 
	int aliveRm = -1; 
	int i;
//	int dup = DUAL_STANDBY_MODE;

#if 1
    if((check_Index = check_my_run_status("STMD")) < 0)
        exit(0);
#endif

    //trcLogFlag = TRCLEVEL_SQL;
    trcLogFlag = 0;

    if ( InitSys() < 0 )
        exit(1);

    // clear previous messages
    while (msgrcv(stmdQid, &rxGenQMsg, sizeof(GeneralQMsgType), 0, IPC_NOWAIT) > 0);

	logPrint(trcLogId, FL, "STMD PROCESS START VER[%s]\n", ver);

    while(1) 
    {
        keepalivelib_increase();

        while ((ret = msgrcv(stmdQid, &rxGenQMsg, sizeof(GeneralQMsgType), 0, IPC_NOWAIT)) > 0) {
			keepalivelib_increase(); 
			aliveLo = keepaliveLoDB();
			if( aliveLo == -1 )
			{
				if( conn != NULL )
				{
					mysql_close(conn);
					conn = NULL;
				}

				aliveLo = connectLoDB(); // dead == Local
				if( aliveLo == 0 )
				{
					sprintf(trcBuf, "[stmd_main] >>> mysql re connection success.\n"); 
					trclib_writeLog(FL, trcBuf);
				}
				else
				{
					sprintf(trcBuf, "[stmd_main] >>> mysql re connection fail.\n"); 
					trclib_writeLogErr(FL, trcBuf);
					continue;
				}
			}
            stmd_exeRxQMsg (&rxGenQMsg);
		    memset(&rxGenQMsg, 0, sizeof(GeneralQMsgType));  
		}
		if ((ret < 0) && (errno == EINVAL || errno == EFAULT)) {
            sprintf(trcBuf,"[stmd_main] >>> msgrcv fail; err=%d(%s)\n", errno, strerror(errno));
            trclib_writeLogErr (FL,trcBuf);
            //return -1;
			continue;
        }

		aliveLo = 1; 
		aliveRm = 1;
        // 5분의 배수일때 통계 요구 메시지를 보내고, 일을 시작한다.
        switch(isTimeToWork ())
        {
        	case 0:
   		   	case 1:

				aliveLo = keepaliveLoDB();
				if( aliveLo == -1 )
				{
					if( conn != NULL )
					{
						mysql_close(conn);
						conn = NULL;
					}

					aliveLo = connectLoDB(); // dead == Local
					if( aliveLo == 0 )
					{
						sprintf(trcBuf, "[stmd_main] >>> mysql re connection success.\n"); 
						trclib_writeLog(FL, trcBuf);
					}
					else
					{
						sprintf(trcBuf, "[stmd_main] >>> mysql re connection fail.\n"); 
						trclib_writeLogErr(FL, trcBuf);
						continue;
					}
#if 0 	// 2010. 10. 05 jjinri : 시스템 명령 제거 
					aliveLo = mysqlLiveCheck();
					if( aliveLo != 0 )
					{
						sprintf(trcBuf,"[stmd_main] >>> mysqld is down.\n");
						trclib_writeLogErr (FL,trcBuf);
						system("/etc/init.d/mysql start");
						sleep(2);
						aliveLo = mysqlLiveCheck();
						if( aliveLo == 0 )
						{
							sprintf(trcBuf, "[stmd_main] >>> mysqld is alive.\n"); 
							trclib_writeLogErr(FL, trcBuf);
							aliveLo = connectLoDB(); // dead == Local
							if( aliveLo == 0 )
							{
								sprintf(trcBuf, "[stmd_main] >>> mysql re connection success.\n"); 
								trclib_writeLogErr(FL, trcBuf);
							}
							else
							{
								sprintf(trcBuf, "[stmd_main] >>> mysql re connection fail.\n"); 
								trclib_writeLogErr(FL, trcBuf);
								break;
							}
						}
						else
						{
							sprintf(trcBuf,"[stmd_main] >>> mysqld is not yet alive. see you 5minute after.\n");
							trclib_writeLogErr (FL,trcBuf);
							break;
						}
					}
#endif
				}
				else
				{
					sprintf(trcBuf,"[stmd_main] >>> local mysql connection alive.\n");
					trclib_writeLog(FL,trcBuf);
				}

				if( (sfdb->sys[1].commInfo.systemDup.myStatus == 1 && sfdb->sys[2].commInfo.systemDup.myStatus != 1) ||
						(sfdb->sys[1].commInfo.systemDup.myStatus != 1 && sfdb->sys[2].commInfo.systemDup.myStatus == 1) )
				{
					sprintf(trcBuf,">>> Active Standby Mode...\n");
					trclib_writeLog(FL,trcBuf);
					SYS_MODE = ACT_STANDBY_MODE; 
				}
				else
				{
					sprintf(trcBuf,">>> Dual Standby Mode...Keepalive...Not Available service.\n");
					trclib_writeLogErr (FL,trcBuf);
					SYS_MODE = DUAL_STANDBY_MODE;
				}

				aliveRm = keepaliveRmDB();
				if( aliveRm == -1 )
				{
					sprintf(trcBuf,"[stmd_main] >>> Remote mysqld is down.\n");
					trclib_writeLogErr (FL,trcBuf);
					
					if( conn_Rm != NULL )
						mysql_close(conn_Rm);

					aliveRm = connectRmDB(); // dead == Local
					if( aliveRm == 0 )
					{
						sprintf(trcBuf, "[stmd_main] >>> mysql remote re connection success.\n"); 
						trclib_writeLog(FL, trcBuf);
					}
					else
					{
						sprintf(trcBuf, "[stmd_main] >>> mysql remote re connection fail.\n"); 
						trclib_writeLogErr(FL, trcBuf);
					}
				}
				else
				{
					sprintf(trcBuf,"[stmd_main] >>> remote mysql connection alive.\n");
					trclib_writeLog(FL,trcBuf);
				}

				stmd_exeTxStatisticReqMsg(&min, &sec);
				t_now = time(0);
				// 현재 시간의 데이터를 가져온다 .
				t_start = (t_now/STAT_OFFSET_UNIT)*STAT_OFFSET_UNIT;
				if( SYS_MODE == ACT_STANDBY_MODE )
				{
					logPrint(trcLogId, FL, "stmd_LoadStatisticRDReport Start =%ld\n",time(0)); // 2010. 12. 22
					stmd_LoadStatisticRDReport(t_start);
					logPrint(trcLogId, FL, "stmd_LoadStatisticRDReport End =%ld\n",time(0)); // 2010. 12. 22
				}
				else
				{
					logPrint(trcLogId, FL, "stmd_InitStatisticRDReport Start =%ld\n",time(0)); 
					ret = stmd_InitStatisticRDReport(t_start);
					if( ret < 0 ) 
					{
						logPrint(trcErrLogId, FL, "stmd_InitStatisticRDReport Fail, [ret:%d], tWhen=%d\n", ret,time(0)); 
						continue;
					}
					logPrint(trcLogId, FL, "stmd_InitStatisticRDReport End =%ld\n",time(0)); 
				}

				for ( i = 0; i < MAX_ONDEMAND_NUM ; i++){
					if( strlen(onDEMAND[i].measureTime) )
					logPrint(trcLogId, FL, "ONDEMAND %d : measure time : %s\n", i, onDEMAND[i].measureTime); 
				}

            	break;
        }
        
        // 조건이 맞으면 일을 한다.
		t_now = time(0);
        if ( readyToWork (min, sec) > 0 )
        {
			aliveLo = keepaliveLoDB();
			if( aliveLo == -1 )
			{
				if( conn != NULL )
				{
					mysql_close(conn);
					conn = NULL;
				}

				aliveLo = connectLoDB(); // dead == Local
				if( aliveLo == 0 )
				{
					sprintf(trcBuf, "[stmd_main] >>> mysql re connection success.\n"); 
					trclib_writeLog(FL, trcBuf);
				}
				else
				{
					sprintf(trcBuf, "[stmd_main] >>> mysql re connection fail.\n"); 
					trclib_writeLogErr(FL, trcBuf);
					continue;
				}
			}

			logPrint(trcLogId, FL, "doOnDemandJob start : %d \n", time(0));	  
			doOnDemandJob(); // ondemand 통계
			keepalivelib_increase(); 
			commlib_microSleep(1000);
			logPrint(trcLogId, FL, "doOnDemandJob end : %d\n", time(0));

			stmd_msg_receiver();
			checkSvcAlm();

/* 2010.10.11 ret < 0 continue; */
			logPrint(trcLogId, FL, "hour update start: %d\n", time(0)); // 2010. 12. 22
			ret = stmd_exeUpdateHourArea(); 
			if( ret < 0 )
			{
				logPrint(trcErrLogId, FL, "hour update fail: %d\n", ret);
				continue;
			}
			logPrint(trcLogId, FL, "hour update end: %d\n", time(0)); // 2010. 12. 22
			keepalivelib_increase(); 
			commlib_microSleep(1000);

			logPrint(trcLogId, FL, "day update start: %d\n", time(0)); // 2010. 12. 22
			ret = stmd_exeUpdateDayArea();
			if( ret < 0 )
			{
				logPrint(trcErrLogId, FL, "day update fail: %d\n", ret);
				continue;
			}
			logPrint(trcLogId, FL, "day update end: %d\n", time(0)); // 2010. 12. 22
			keepalivelib_increase(); 
			commlib_microSleep(1000);
		
			logPrint(trcLogId, FL, "week update start: %d\n", time(0)); // 2010. 12. 22
			ret = stmd_exeUpdateWeekArea();
			if( ret < 0 )
			{
				logPrint(trcErrLogId, FL, "week update fail: %d\n", ret);
				continue;
			}
			logPrint(trcLogId, FL, "week update end: %d\n", time(0)); // 2010. 12. 22
			keepalivelib_increase(); 
			commlib_microSleep(1000);
		
			logPrint(trcLogId, FL, "month update start: %d\n", time(0)); // 2010. 12. 22
			ret = stmd_exeUpdateMonthArea();
			if( ret < 0 )
			{
				logPrint(trcLogId, FL, "month update fail: %d\n", ret);
				continue;
			}
			logPrint(trcLogId, FL, "month update end: %d\n", time(0)); // 2010. 12. 22
			keepalivelib_increase(); 
			commlib_microSleep(1000);

			logPrint(trcLogId, FL, "doPeriodicJob start, %d\n", time(0));	 
			doPeriodicJob(); // 주기적 통계
			keepalivelib_increase(); 
			commlib_microSleep(1000);
			logPrint(trcLogId, FL, "doPeriodicJob end. %d\n", time(0));


			logPrint(trcLogId, FL, "doCronJob start, %d\n", time(0));
			doCronJob(); // 스케쥴 통계(무한 통계)
			keepalivelib_increase(); 
			logPrint(trcLogId, FL, "doCronJob end, %d\n", time(0));	    	
			
			workFLAG = 2;
		}
		commlib_microSleep(1000);
       
    }


}

void sendCondTest(void)
{
	FILE *fp = NULL;
	char buf[4096] = {0,};
	int snd_cnt = 1;
	char condBuf[4096], tmpBuf[1024];

	fp = fopen("/DSC/LOG/STAT/2009/06/23/21/RULEENT_HOUR", "r");

	while( fgets(buf, 4096, fp) )
	{
		memset(tmpBuf, 0x00, sizeof(tmpBuf));
		memset(condBuf, 0x00, sizeof(condBuf));

		snprintf(tmpBuf, strlen(buf), "%s\n", buf); 
		strcat(condBuf, tmpBuf);

		sprintf(trcBuf,"[test GUI] MESSAGE\n%s",condBuf);
		trclib_writeLogErr (FL,trcBuf);                                                    

		if(strlen(condBuf) > 3000) {
			sprintf(trcBuf,"CONSOLE TEST SEND [%dth]\n",snd_cnt);
			trclib_writeLogErr (FL,trcBuf);
			stmd_txMsg2Cond (condBuf, (STSCODE_STM_PERIODIC_RULEENT_HOUR - STSCODE_TO_MSGID_STATISTICS), 1, snd_cnt++);
			memset(condBuf, 0x00, sizeof(condBuf));
		}
	}

	fclose(fp);
}

int checkSvcAlm(void)
{
	double logonRateA = 0, logoutRateA = 0, trafficRateA = 0;
	double logonRateB = 0, logoutRateB = 0, trafficRateB = 0;
	
	// logon Check SCM A
	if( (g_stSvcVal.logon_A[0] >= g_stSvcAlm.logon_min || 
			g_stSvcVal.logon_A[1] >= g_stSvcAlm.logon_min ) && 
			g_stSvcVal.logon_A[0] != g_stSvcVal.logon_A[1] )
	{
		if( g_stSvcVal.logon_A[0] != 0 )
			logonRateA = (double)(g_stSvcVal.logon_A[1] - g_stSvcVal.logon_A[0])*100/g_stSvcVal.logon_A[0];
		else
			logonRateA = 100;

		// rate 계산값은 부호를 가짐. 
		if( logonRateA >= g_stSvcAlm.logon_rate || logonRateA <= -g_stSvcAlm.logon_rate) // RATE 를 넘어섬...
		{
			makeAlmMessage("SCMA", "LOGON", logonRateA, g_stSvcAlm.logon_rate,
					g_stSvcVal.logon_A[0], g_stSvcVal.logon_A[1], g_stSvcAlm.logon_min);
		}
	}

	// logout Check SCM A
	if( (g_stSvcVal.logout_A[0] >= g_stSvcAlm.logout_min || 
			g_stSvcVal.logout_A[1] >= g_stSvcAlm.logout_min) &&
			g_stSvcVal.logout_A[0] != g_stSvcVal.logout_A[1] )
	{
		if( g_stSvcVal.logout_A[0] != 0 )
			logoutRateA = (double)(g_stSvcVal.logout_A[1] - g_stSvcVal.logout_A[0])*100/g_stSvcVal.logout_A[0];
		else
			logoutRateA = 100;

		// rate 계산값은 부호를 가짐. 
		if( logoutRateA >= g_stSvcAlm.logout_rate || logoutRateA <= -g_stSvcAlm.logout_rate) // RATE 를 넘어섬...
		{
			makeAlmMessage("SCMA", "LOGOUT", logoutRateA, g_stSvcAlm.logout_rate, 
					g_stSvcVal.logout_A[0],g_stSvcVal.logout_A[1], g_stSvcAlm.logout_min);
		}
	}

	// throughput Check  SCE A
	if( (g_stSvcVal.traffic_A[0] >= g_stSvcAlm.traffic_min || 
			g_stSvcVal.traffic_A[1] >= g_stSvcAlm.traffic_min) &&
			g_stSvcVal.traffic_A[0] != g_stSvcVal.traffic_A[1] )
	{
		if( g_stSvcVal.traffic_A[0] != 0 )
			trafficRateA = (double)(g_stSvcVal.traffic_A[1] - g_stSvcVal.traffic_A[0])*100/g_stSvcVal.traffic_A[0];
		else
			trafficRateA = 100;

		// RATE 를 넘어섬...
		if( trafficRateA >= g_stSvcAlm.traffic_rate || trafficRateA <= -g_stSvcAlm.traffic_rate ) 
		{
			makeAlmMessage("SCEA", "THROUGHPUT", trafficRateA, g_stSvcAlm.traffic_rate,
					g_stSvcVal.traffic_A[0],g_stSvcVal.traffic_A[1], g_stSvcAlm.traffic_min);
		}
	}

	// logon Check SCM B
	if( (g_stSvcVal.logon_B[0] >= g_stSvcAlm.logon_min || 
			g_stSvcVal.logon_B[1] >= g_stSvcAlm.logon_min ) && 
			g_stSvcVal.logon_B[0] != g_stSvcVal.logon_B[1] )
	{
		if( g_stSvcVal.logon_B[0] != 0 )
			logonRateB = (double)(g_stSvcVal.logon_B[1] - g_stSvcVal.logon_B[0])*100/g_stSvcVal.logon_B[0];
		else
			logonRateB = 100;

		// rate 계산값은 부호를 가짐. 
		if( logonRateB >= g_stSvcAlm.logon_rate || logonRateB <= -g_stSvcAlm.logon_rate) // RATE 를 넘어섬...
		{
			makeAlmMessage("SCMB", "LOGON", logonRateB, g_stSvcAlm.logon_rate,
					g_stSvcVal.logon_B[0], g_stSvcVal.logon_B[1], g_stSvcAlm.logon_min);
		}
	}

	// logout Check SCM B
	if( (g_stSvcVal.logout_B[0] >= g_stSvcAlm.logout_min || 
			g_stSvcVal.logout_B[1] >= g_stSvcAlm.logout_min) &&
			g_stSvcVal.logout_B[0] != g_stSvcVal.logout_B[1] )
	{
		if( g_stSvcVal.logout_B[0] != 0 )
			logoutRateB = (double)(g_stSvcVal.logout_B[1] - g_stSvcVal.logout_B[0])*100/g_stSvcVal.logout_B[0];
		else
			logoutRateB = 100;

		// rate 계산값은 부호를 가짐. 
		if( logoutRateB >= g_stSvcAlm.logout_rate || logoutRateB <= -g_stSvcAlm.logout_rate) // RATE 를 넘어섬...
		{
			makeAlmMessage("SCMB", "LOGOUT", logoutRateB, g_stSvcAlm.logout_rate, 
					g_stSvcVal.logout_B[0],g_stSvcVal.logout_B[1], g_stSvcAlm.logout_min);
		}
	}

	// throughput Check  SCE B
	if( (g_stSvcVal.traffic_B[0] >= g_stSvcAlm.traffic_min || 
			g_stSvcVal.traffic_B[1] >= g_stSvcAlm.traffic_min) &&
			g_stSvcVal.traffic_B[0] != g_stSvcVal.traffic_B[1] )
	{
		if( g_stSvcVal.traffic_B[0] != 0 )
			trafficRateB = (double)(g_stSvcVal.traffic_B[1] - g_stSvcVal.traffic_B[0])*100/g_stSvcVal.traffic_B[0];
		else
			trafficRateB = 100;

		// RATE 를 넘어섬...
		if( trafficRateB >= g_stSvcAlm.traffic_rate || trafficRateB <= -g_stSvcAlm.traffic_rate ) 
		{
			makeAlmMessage("SCEB", "THROUGHPUT", trafficRateB, g_stSvcAlm.traffic_rate,
					g_stSvcVal.traffic_B[0],g_stSvcVal.traffic_B[1], g_stSvcAlm.traffic_min);
		}
	}

	// 알람 체크 이후 현재 값을 이전 값으로 셋팅시켜 준다. 
	g_stSvcVal.logon_A[0] = g_stSvcVal.logon_A[1];
	g_stSvcVal.logout_A[0] = g_stSvcVal.logout_A[1];
	g_stSvcVal.traffic_A[0] = g_stSvcVal.traffic_A[1];

	g_stSvcVal.logon_B[0] = g_stSvcVal.logon_B[1];
	g_stSvcVal.logout_B[0] = g_stSvcVal.logout_B[1];
	g_stSvcVal.traffic_B[0] = g_stSvcVal.traffic_B[1];

	return 0;
}

//------------------------------------------------------------------------------
int makeAlmMessage (char *system, char *svcname, double rate, int RATE, int prev, int curr, int MIN )
{
	int     almCode;
	char    condBuf[4096], tmpBuf[1024];

	almCode = ALMCODE_SFM_SVC_OVER_INFO;

	sprintf(condBuf,"    %s %s\n%3s S%04d SERVICE RATE OVER ALARM %s\n",
			mySysName,   // system_name
			commlib_printTStamp(),  // CoAc½A°￠ time stamp (³a,¿u,AI,½A,ºÐ,AE,¿aAI)
			"*", almCode, "OCCURED");

	sprintf(tmpBuf,"      SYSTEM  = %s\n      "
			"SEVICE = %s\n      OVERRATE  = %4.2lf%%(>= %d%%)\n      "
			"PREV = %d\n      CURRENT  = %d(MIN = %d)\n      "
			"COMPLETED\n\n\n",
			system, svcname, rate, RATE, prev, curr, MIN);

	strcat (condBuf, tmpBuf);

	stmd_txAlmMsg2Cond( condBuf, MTYPE_ALARM_REPORT, almCode);

	return 0;
}


//------------------------------------------------------------------------------
// SERVICE ALARM 발생 메시지를 COND로 전송한다. 
//------------------------------------------------------------------------------
int stmd_txAlmMsg2Cond (char *buff, long mtype, int msgId)
{           
	int             txLen;         
	GeneralQMsgType txGenQMsg;
	IxpcQMsgType    *txIxpcMsg;

	txGenQMsg.mtype = mtype; 

	txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
	memset ((void*)&txIxpcMsg->head, 0, sizeof(txIxpcMsg->head));                          

	strcpy (txIxpcMsg->head.srcSysName, mySysName);                                        
	strcpy (txIxpcMsg->head.srcAppName, myAppName);
	strcpy (txIxpcMsg->head.dstSysName, mySysName);                                        
	strcpy (txIxpcMsg->head.dstAppName, "COND");
	txIxpcMsg->head.msgId   = msgId; 
	txIxpcMsg->head.segFlag = 0;                                                           
	txIxpcMsg->head.seqNo   = 1;

	strcpy (txIxpcMsg->body, buff);
	txIxpcMsg->head.bodyLen = strlen(buff);
	txLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;                             

	if (msgsnd(condQid, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0) {                       
		sprintf(trcBuf,"[stmd_txMsg2Cond] msgsnd fail to COND; err=%d(%s)\n%s", 
				errno, strerror(errno), buff);
		trclib_writeLogErr (FL,trcBuf);                                                    
		return -1;
	} else {                                                                               
		if (trcFlag || trcLogFlag) {
			sprintf(trcBuf,"[stmd_txMsg2Cond] send to COND\n%s", buff);                    
			trclib_writeLog (FL,trcBuf);                                                   
		}                                                                                  
	}                                                                                      
	return 1;                                                                              

} //----- End of stmd_txMsg2Cond -----//     

int connectDB(MYSQL *sql, int type) 
{
	int RETRY = 5;
	int i;

	if( stmd_mysql_init(type) < 0 ){
		sprintf(trcBuf, "mysql re connect fail : connect : ERR[%s]\n",mysql_error(sql));
		trclib_writeErr(FL, trcBuf);
		for( i = 0 ;i < RETRY; i++ ){
			if( stmd_mysql_init(type) < 0 ){
				sprintf(trcBuf, "mysql re connect fail. Try [%d]th, err=%s\n",i+1, mysql_error(sql));
				trclib_writeErr(FL, trcBuf);
				sleep(1);
				keepalivelib_increase();
			} else {
				break;
			}
		}
		if( i == RETRY )
		{
			sprintf(trcBuf, "mysql re connect fail : connect : ERR[%s]\n",mysql_error(sql));
			trclib_writeErr(FL, trcBuf);
			sprintf(trcBuf,"stmd terminate.\n");  trclib_writeLogErr (FL,trcBuf);
			exit(-1);
		}
		else
		{
			sprintf(trcBuf, "mysql re connect success Try[%d]\n",i+1);
			trclib_writeErr(FL, trcBuf);
		}
	}

	return 0;
}

// 2010. 10. 05 jjinri : RETRY 추가 
int connectLoDB(void)
{
	return connectDB(&sql, LOCAL_DB);
}

int connectRmDB(void) 
{

	if( (sfdb->sys[1].commInfo.systemDup.myStatus == 1 && sfdb->sys[2].commInfo.systemDup.myStatus != 1) ||
			(sfdb->sys[1].commInfo.systemDup.myStatus != 1 && sfdb->sys[2].commInfo.systemDup.myStatus == 1) )
	{
		sprintf(trcBuf,">>> Active Standby Mode...\n");
		trclib_writeLogErr(FL,trcBuf);
		SYS_MODE = ACT_STANDBY_MODE; 
	}
	else
	{
		sprintf(trcBuf,">>> Dual Standby Mode...Keepalive...Not Available service.\n");
		trclib_writeLogErr (FL,trcBuf);
		SYS_MODE = DUAL_STANDBY_MODE;
		return -1;
	}


	return connectDB(&sql_Rm, REMOTE_DB);
}


int keepaliveLoDB(void)
{
    MYSQL_RES *result;
    MYSQL_ROW row;

    char query[128] = {0,};
    int alive = 0; 

    sprintf(query, "select 1 from information_schema.TABLES where TABLE_SCHEMA='DSCM' and TABLE_NAME='INI_VALUES'");

	if( conn == NULL )
		return -1;
	else
	{
		if( stmd_mysql_query (query) < 0 ){
			return -1;
		}

		result = mysql_store_result(&sql) ;
		if( result == NULL )
			return -1;
		row = mysql_fetch_row(result);
		if( row == NULL )
		{
			mysql_free_result(result); // MEM if NULL 10.07
			return -1;
		}
		else
		{
			alive = atoi(row[0]);
			mysql_free_result(result);

			if( alive != 1 )
			{
				sprintf(trcBuf, "mysql server gone away : %d\n",alive);
				trclib_writeErr(FL, trcBuf);
				return -1;
			}
		}
	}
	return alive;
}

int keepaliveRmDB(void)
{
    MYSQL_RES *result;
    MYSQL_ROW row;
    char query[128] = {0,};
	int alive = 0;

    sprintf(query, "select 1 from information_schema.TABLES where TABLE_SCHEMA='mysql' and TABLE_NAME='db'");

	if( conn_Rm == NULL )
	{
		sprintf(trcBuf, "conn_Rm handler is NULL\n");
		trclib_writeLogErr(FL, trcBuf);
	}
	else
	{
		sprintf(trcBuf, "conn_Rm handler is Not NULL\n");
		trclib_writeLogErr(FL, trcBuf);

		mysql_close(conn_Rm);
		conn_Rm = NULL;
	}

	if( 0 != connectDB(&sql_Rm, REMOTE_DB)){
		sprintf(trcBuf, "Retry Remote Connection All Fail.\n");
		trclib_writeLogErr(FL, trcBuf);
		return -1;
	}

	if (stmd_mysql_query_with_conn (conn_Rm, query) < 0) {
		sprintf(trcBuf, "Remote Query Fail.\n");
		trclib_writeLogErr(FL, trcBuf);
	}

	result = mysql_store_result(&sql_Rm) ;
	if( result == NULL )
	{
		sprintf(trcBuf, "result handler is NULL. \n");
		trclib_writeLogErr(FL, trcBuf);
		return -1;
	}
	row = mysql_fetch_row(result);
	if( row == NULL )
	{
		mysql_free_result(result); // MEM if NULL 10.07
		sprintf(trcBuf, "row fetch is NULL. \n");
		trclib_writeLogErr(FL, trcBuf);
		return -1;
	}
	else
	{
		alive = atoi(row[0]);
		mysql_free_result(result);
		if( alive != 1 )
		{
			sprintf(trcBuf, "Remote mysql server gone away : %d\n",alive);
			trclib_writeLogErr(FL, trcBuf);
			return -1;
		}
	}

    return alive;
}

int stmd_exeRxQMsg (GeneralQMsgType *rxGenQMsg)
{
    int     ret = 1;
//	IxpcQMsgType *ixpc = (IxpcQMsgType*)rxGenQMsg->body;


//	printf("%s : %s\n", ixpc->head.srcSysName, ixpc->head.srcAppName);

    switch (rxGenQMsg->mtype) {
        case MTYPE_SETPRINT:
            ret = trclib_exeSetPrintMsg ((TrcLibSetPrintMsgType*)rxGenQMsg);
            break;
        case MTYPE_STATUS_REPORT:
        case MTYPE_STATISTICS_REPORT:
            ret = stmd_exeStatMsg ((IxpcQMsgType*)rxGenQMsg->body);
            rxStatMsgCnt++;
            break;
        case MTYPE_MMC_REQUEST:
            ret = stmd_exeMMCMsg ((IxpcQMsgType*)rxGenQMsg->body);
            break;
        default:
            sprintf(trcBuf, "unexpected mtype[%ld]\n", rxGenQMsg->mtype);
            trclib_writeErr(FL, trcBuf);
            return -1;
    }

    return 1;
}

//------------------------------------------------------------------------------
// MMCD에서 명령어를 수신한 경우
//------------------------------------------------------------------------------
int stmd_exeMMCMsg (IxpcQMsgType *rxIxpcMsg)
{
    
    MMLReqMsgType       *mmlReqMsg;
    StmdMmcHdlrVector   *mmcHdlr;

    mmlReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
    if ((mmcHdlr = (StmdMmcHdlrVector*) bsearch (
            mmlReqMsg->head.cmdName,
            mmcHdlrVector,
            numMmcHdlr,
            sizeof(StmdMmcHdlrVector),
            stmd_mmcHdlrVector_bsrchCmp)) == NULL) 
    {
        sprintf(trcBuf,"[stmd_exeMMCMsg] received unknown mml_cmd(:%s:)\n", mmlReqMsg->head.cmdName);
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
//fprintf(stderr, "stmd_exeMMCMsg::cmdName = %s\n", mmcHdlr->cmdName);
    if (trcLogFlag) {
        sprintf(trcBuf, "cmdName = %s\n", mmcHdlr->cmdName);
        trclib_writeLog(FL, trcBuf);
    }
    // 처리 function을 호출한다.
    (int)(*(mmcHdlr->func)) (rxIxpcMsg);

    return 1;
}

int stmd_send_logon_usage(IxpcQMsgType *rxIxpcMsg)
{
	PLEG_TOT_STAT	pstLegTot;
	PLOGON_STAT     pstLogOn;
	STAT_LOGON_RATE stLogonStat[2];

	int             sm_ch_id, log_mod;
	unsigned int    request, success;
	float           rate;

	pstLegTot = (PLEG_TOT_STAT)&rxIxpcMsg->body;

	/* calculate usage */
	for( log_mod = 0; log_mod < LOG_MOD_CNT; log_mod++ ){
		for( sm_ch_id = 0, request = 0, success = 0; sm_ch_id < MAX_RLEG_CNT; sm_ch_id++ ){
			
			pstLogOn = &pstLegTot->stLogon[sm_ch_id][log_mod];
			request  += (unsigned int) ntohl( pstLogOn->uiLogOn_Request );
			success  += (unsigned int) ntohl( pstLogOn->uiLogOn_Success );
		}
		if( !request || !success ){
			rate = 0.0;
		} else {
			rate = ( (float) success/ (float) request ) * 100;
		}

		stLogonStat[log_mod].rate    = (short )rate;
		stLogonStat[log_mod].request = request;
		stLogonStat[log_mod].success = success;
#ifdef DEBUG
		logPrint(trcLogId, FL, " Log%s=%d Stat : rate=%d request=%d success=%d\n",
				!log_mod?"On ":"Out", log_mod, 
				stLogonStat[log_mod].rate, stLogonStat[log_mod].request, stLogonStat[log_mod].success);
#endif
		sprintf( stLogonStat[log_mod].szSysName, "%s", pstLegTot->szSysName );
		sprintf( stLogonStat[log_mod].stat_date, "%s", get_insert_time() );

	}
	stmd_txMsg2FIMD((char*)&stLogonStat); /* 단 한 번 보낸다. */
	return 0;
}

// 해당 MP로부터 통계 메시지를 받은 경우
int stmd_exeStatMsg (IxpcQMsgType *rxIxpcMsg)
{
	int    ret;
    time_t now;
    now = time(0);
 
    if (trcLogFlag) {
    	sprintf(trcBuf, "rxIxpcMsg->head.msgId = %d\n", rxIxpcMsg->head.msgId);
    	trclib_writeLog(FL, trcBuf);
    }

    switch (rxIxpcMsg->head.msgId) {
		case  MSGID_LOAD_STATISTICS_REPORT:
			logPrint(trcLogId, FL, "Load Statistics Response\n");
			stmd_hdlLoadStatisticRpt(rxIxpcMsg);    //100
			logPrint(trcLogId, FL, "Load Statistics End\n");
			break;
			
		case  MSGID_FAULT_STATISTICS_REPORT:        //101
			logPrint(trcLogId, FL, "Fault Statistics Response\n");
			stmd_hdlFltStatisticRpt(rxIxpcMsg);
			logPrint(trcLogId, FL, "Fault Statistics End\n");
			break;

		case MSGID_LEG_STATISTICS_REPORT:			//114
			logPrint(trcLogId, FL, "RLEG leg/logon Response:%d, <-- %s-%s\n",
					rxIxpcMsg->head.msgId, rxIxpcMsg->head.srcSysName, rxIxpcMsg->head.srcAppName);
			ret = stmd_hdlRlegStatisticRpt(rxIxpcMsg);
			if( ret < 0 ){
				logPrint(trcLogId, FL, "Failed leg 5minute statistics, Don't notification 2FIMD\n");
			}

			/* 앞으로 LOGON 통계는 별도로 보고 받지 않고, LEG 통계에 포함되어서 함께 올라옴. by uamyd 20110424 */
			ret = stmd_hdlLogonStatisticRpt(rxIxpcMsg);    //100
			if( ret < 0 ){
				logPrint(trcLogId, FL, "Failed logon 5minute statistics, Don't notification 2FIMD\n");
				break;
			}

			stmd_send_logon_usage(rxIxpcMsg); //2FIMD
			break;

		/** FLOW REPORT 2010.08.23 */
		case MSGID_SYS_SCEFLOW_STATUS_REPORT:
			logPrint(trcLogId, FL, "SCE flow Response:%d, <-- %s\n", rxIxpcMsg->head.msgId, rxIxpcMsg->head.srcSysName);
			stmd_hdlFlowStatisticRpt(rxIxpcMsg);    //100
//			logPrint(trcLogId, FL, "SCE flow End\n");
			break;

		default:
			sprintf(trcBuf,"unexpected statistic_msgid(%d)\n", rxIxpcMsg->head.msgId);
			trclib_writeErr(FL, trcBuf);
			break;
    }

    return 1;
}

int isTimeToWork ()
{
    time_t      cur_time;
    struct tm  *cur_tMS;

    cur_time  = time (0);
    cur_tMS = (struct tm*)localtime((time_t*)&cur_time);

    // 현재의 분이 5로 나누어 떨어지면 할 일을 한다.  workFlag가 0일때만
    if ( (cur_tMS->tm_min%STAT_UNIT) == 0 && (cur_tMS->tm_sec > 5) && (cur_tMS->tm_sec < 45) && workFLAG == 0 )
//    if ( (cur_tMS->tm_sec < 45) && workFLAG == 0 ) // for test
    {
        workFLAG = 1;
        if(cur_tMS->tm_hour == 0 && cur_tMS->tm_min == 0)
        {
            return 0;
        }
        else
        {
	    	logPrint(trcLogId, FL, "start time: %04d-%02d-%02d %02d:%02d:%02d\n",
                    cur_tMS->tm_year + 1900, 
                    cur_tMS->tm_mon +1, 
                    cur_tMS->tm_mday,
                    cur_tMS->tm_hour, 
                    cur_tMS->tm_min, 
                    cur_tMS->tm_sec );
            return 1;
        }
    } 
    else if ( (cur_tMS->tm_min%STAT_UNIT) != 0 )
    {
        workFLAG = 0;
        return -1;
    } 
    else 
    {
        return -1;
    }

    return -1;

}

// 통계 요구 메시지를 보내는 함수
void stmd_exeTxStatisticReqMsg(int *min, int *sec)
{
    GeneralQMsgType     txGenQMsg;
    IxpcQMsgType        *txIxpcMsg;
    int                 i, txLen;
    time_t      cur_time;
    struct tm   *cur_tMS;

    txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;

    memset((void *)&txIxpcMsg->head, 0, sizeof(txIxpcMsg->head));

    txGenQMsg.mtype = MTYPE_STATISTICS_REQUEST;

    strcpy (txIxpcMsg->head.srcSysName, mySysName);
    strcpy (txIxpcMsg->head.srcAppName, myAppName);

    for ( i = 0; i < txStaticCnt ; i++) 
	{
		strcpy (txIxpcMsg->head.dstSysName, txStaticsProcInfo[i].sysName ); strcpy (txIxpcMsg->head.dstAppName, txStaticsProcInfo[i].prcName);
		txLen = sizeof(txIxpcMsg->head);

		sprintf(trcBuf, "[ReqMsg] SysName = %s, AppName = %s\n", txIxpcMsg->head.dstSysName, txIxpcMsg->head.dstAppName);
		trclib_writeLog(FL, trcBuf);
		logPrint(trcLogId, FL, "%s", trcBuf); // 2010. 12. 22

		if ( msgsnd(ixpcQid, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0 ) 
		{
			sprintf(trcBuf, "Statistics Req Error[%d] = %s\n", errno, strerror(errno));
			trclib_writeErr (FL,trcBuf);
		}
		commlib_microSleep(1000);
    }

    cur_time  = time ( (time_t *)0);
    cur_tMS = (struct tm*)localtime ( &cur_time );
    *min = cur_tMS->tm_min; // 현재 시간을 기록
    *sec = cur_tMS->tm_sec;

    // 수신한 통계 메시지를 초기화 한다.
    rxStatMsgCnt = 0;
}

int readyToWork (int min, int sec)
{
    time_t      cur_time;
    struct tm   *cur_tMS;

    if ( workFLAG == 1 )
    {
        cur_time = time ( (time_t *)0);
//        localtime_r(&cur_time, &cur_tMS);
        cur_tMS = localtime(&cur_time);
        
        /* 통계 요청후 25sec(WAIT_FOR_STAT_MSG) 후에 작업시작 */
        //if ( ((min == cur_tMS.tm_min) && (cur_tMS.tm_sec > WAIT_FOR_STAT_MSG+sec)) ) 
        if ( ((min >= cur_tMS->tm_min) && (cur_tMS->tm_sec > WAIT_FOR_STAT_MSG+sec)) ) 
        {
	    	logPrint(trcLogId, FL, "readyToWork cur_time= %04d-%02d-%02d %02d:%02d:%02d\n",
                    cur_tMS->tm_year + 1900, 
                    cur_tMS->tm_mon +1, 
                    cur_tMS->tm_mday,
                    cur_tMS->tm_hour, 
                    cur_tMS->tm_min, 
                    cur_tMS->tm_sec  );
            return 1;
        } 
        else 
        {
            return -1;
        }
    } else {
        return -1;
    }
}
long long htonll(long long val)
{
    /*long long res;
    unsigned int *in , *out = (unsigned int *)(&res);
    
    out[0] = htonl(in[0]);
    out[1] = htonl(in[1]);
    
    return res;*/
    
    
    long long res;
    int * out = (int *)(&res);
    int * in = (int *)(&val);
    out[0] = htonl(in[0]);
    out[1] = htonl(in[1]);
    
    return res;
}

long long ntohll(long long val)
{
    long long res;
    int * out = (int *)(&res);
    int * in = (int *)(&val);
    int tmp;
    out[0] = ntohl(in[0]);
    out[1] = ntohl(in[1]);
    
    tmp=out[0];
    out[0]=out[1];
    out[1]=tmp;
    
    return res;
}

void clear_g_stRule(void)
{
	int i,j;

	for( i = 0; i < SCE_CNT; i++ )
	{
		for( j = 0; j < MAX_RULE_NUM; j++ )
		{
			g_stSCERule[i].stRule[j].eFlag = 0;
			g_stSCERule[i].stRule[j].unblk = 0;
			g_stSCERule[i].stRule[j].blk = 0;
			g_stSCERule[i].stRule[j].red = 0;
			g_stSCERule[i].stRule[j].tot = 0;
			g_stSCERule[i].stRule[j].uThru = 0;
			g_stSCERule[i].stRule[j].dThru = 0;
			g_stSCERule[i].stRule[j].tThru = 0;
			g_stSCERule[i].stRule[j].uByte = 0;
			g_stSCERule[i].stRule[j].dByte = 0;
			g_stSCERule[i].stRule[j].tByte = 0;
		}
	}
}

void clear_g_stEntry(void)
{
	int i,j;

	for( i = 0; i < SCE_CNT; i++ )
	{
		for( j = 0; j < MAX_ENTRY_NUM; j++ )
		{
			g_stSCEEntry[i].stEntry[j].eFlag = 0;
			g_stSCEEntry[i].stEntry[j].unblk = 0;
			g_stSCEEntry[i].stEntry[j].blk = 0;
			g_stSCEEntry[i].stEntry[j].red = 0;
			g_stSCEEntry[i].stEntry[j].tot = 0;
			g_stSCEEntry[i].stEntry[j].uThru = 0;
			g_stSCEEntry[i].stEntry[j].dThru = 0;
			g_stSCEEntry[i].stEntry[j].tThru = 0;
			g_stSCEEntry[i].stEntry[j].uByte = 0;
			g_stSCEEntry[i].stEntry[j].dByte = 0;
			g_stSCEEntry[i].stEntry[j].tByte = 0;
		}
	}
}

int stmd_InitStatisticRDReport(time_t t_start)
{
    int ret = 0;
    time_t start = t_start;

	// 전체 트래픽 통계 
	ret = Insert_default_lur();
	if( ret >= 0 )
		logPrint(trcLogId, FL, "LUR table default success : %ld\n", time(0));
	else
		return -1;

	stmd_msg_receiver();

	// RuleSet 통계 
    ret = Insert_default_ruleset();
    if( ret >= 0 )
	    logPrint(trcLogId, FL, "RULESET table default success : %ld\n", time(0));
	else 
		return -2;

	stmd_msg_receiver();

	// Rule Entry 통계 
    ret = Insert_default_ruleent();
    if( ret >= 0 )
	    logPrint(trcLogId, FL, "RULE Entry table default success : %ld\n", time(0));
	else 
		return -3;

	stmd_msg_receiver();

	// FLOW 통계 
    ret = stmd_LoadStatisticFLOW(start);
    if( ret >= 0 )
	    logPrint(trcLogId, FL, "FLOW table insert success : %ld\n", time(0));
	else 
		return -4;

	stmd_msg_receiver();
	/**
	// FLOW 통계 
    ret = Insert_default_flow();
    if( ret >= 0 )
	    logPrint(trcLogId, FL, "FLOW table default success : %ld\n", time(0));
	else 
		return -5;
	stmd_msg_receiver();
	*/

	// Sms 통계 
    ret = Insert_default_sms();
    if( ret >= 0 )
	    logPrint(trcLogId, FL, "SMS table default success : %ld\n", time(0));
	else
		return -6;

    ret = stmd_LoadStatisticDELAY(start);
    if( ret >= 0 )
	    logPrint(trcLogId, FL, "Delay table insert success : %ld\n", start);
	else
		return -7;

    return ret;
}


int stmd_LoadStatisticRDReport(time_t t_start)
{
    int ret = 0;
    time_t start = t_start;

	// sync INI_VALUES 테이블 
	ret = stmd_SelectINI_Values();

	// sync RULE_SET_LIST.conf 파일 
    memset(&g_stSCERule[0], 0x00, sizeof(RuleSetList)*MAX_SCE_NUM);
    memset(g_ruleIdBuf,0x00, sizeof(int)*MAX_RULE_NUM);
    g_ruleItemCnt = readRuleConfFile(rsFname);
	logPrint(trcLogId, FL, "RULE SET LIST CNT[%d]\n", g_ruleItemCnt); // 2010. 12. 22

	// FLOW 통계 
	logPrint(trcLogId, FL, "FLOW 5minute start: %ld\n", time(0));  // 2010. 12.23
    ret = stmd_LoadStatisticFLOW(start);
	logPrint(trcLogId, FL, "FLOW 5minute end ret[%d]: %ld\n", ret, time(0));  // 2010. 12.23

	stmd_msg_receiver();
	logPrint(trcLogId, FL, "stmd_msg_receiver succ\n");  // 2010. 12.23

	initSmsc();
	// Sms 통계 
	logPrint(trcLogId, FL, "SMS 5minute start: %ld\n", time(0));  // 2010. 12.23
    ret = stmd_LoadStatisticSMS(start);
	logPrint(trcLogId, FL, "SMS 5minute end ret[%d]: %ld\n", ret, time(0));  // 2010. 12.23

	stmd_msg_receiver();
	logPrint(trcLogId, FL, "stmd_msg_receiver succ\n");  // 2010. 12.23

	// 전체 트래픽 통계 
	logPrint(trcLogId, FL, "LUR 5minute start: %ld\n", time(0));  // 2010. 12.23
	ret = stmd_LoadStatisticLUR(start);
	logPrint(trcLogId, FL, "LUR 5minute end ret[%d]: %ld\n", ret, time(0));  // 2010. 12.23

	stmd_msg_receiver();
	logPrint(trcLogId, FL, "stmd_msg_receiver succ\n");  // 2010. 12.23

	logPrint(trcLogId, FL, "BLOCK 5minute start: %ld\n", time(0));  // 2010. 12.23
    ret = stmd_LoadStatisticBLOCK(start);
	logPrint(trcLogId, FL, "BLOCK 5minute end ret[%d]: %ld\n", ret, time(0));  // 2010. 12.23
	stmd_msg_receiver();
	logPrint(trcLogId, FL, "stmd_msg_receiver succ\n");  // 2010. 12.23

	// TR + BLOCK = RULESET and RULE SET ENTRY
	logPrint(trcLogId, FL, "TR 5minute start: %ld\n", time(0));  // 2010. 12.23
	ret = stmd_LoadStatisticTR(start);
	logPrint(trcLogId, FL, "TR 5minute end ret[%d]: %ld\n", ret, time(0));  // 2010. 12.23

	stmd_msg_receiver();
	logPrint(trcLogId, FL, "stmd_msg_receiver succ\n");  // 2010. 12.23
	clear_g_stRule();

	// RuleSet 통계 
	logPrint(trcLogId, FL, "RULESET 5minute start: %ld\n", time(0));  // 2010. 12.23
    ret = stmd_LoadStatisticRULESET(start);
	logPrint(trcLogId, FL, "RULESET 5minute end ret[%d]: %ld\n", ret, time(0));  // 2010. 12.23

	stmd_msg_receiver();
	logPrint(trcLogId, FL, "stmd_msg_receiver succ\n");  // 2010. 12.23
	clear_g_stEntry();

	// Rule Entry 통계 
	logPrint(trcLogId, FL, "RULEENTRY 5minute start: %ld\n", time(0));  // 2010. 12.23
    ret = stmd_LoadStatisticRULEENTRY(start);
	logPrint(trcLogId, FL, "RULEENTRY 5minute end ret[%d]: %ld\n", ret, time(0));  // 2010. 12.23

    return ret;
}

int stmd_SelectINI_Values(void)
{
	char query[1024] = {0,}, query_rm[1024] = {0,};

	MYSQL_RES *res, *res_rm;
	MYSQL_ROW row, row_rm;
	int	ErrCode = 0;
	int ompINICnt = 0, cmINICnt = 0;
	int i, entry = 0, cnt = 0;

	snprintf(query, 1024, "select count(*) from INI_VALUES ");

	if( conn == NULL )
		return -1;

	if (stmd_mysql_query (query) < 0) {
		sprintf(trcBuf, "select fail query:%s, err = %s\n", query, mysql_error(conn));
		trclib_writeErr (FL,trcBuf);
		return -1;
	}
	res = mysql_store_result(conn);
	row = mysql_fetch_row(res);

	if( row != NULL )
		ompINICnt = atoi(row[0]);

	mysql_free_result(res);

	snprintf(query_rm, 1024, "select count(*) from INI_VALUES ");

	if( conn_Rm != NULL )
	{
		if (stmd_mysql_query_with_conn (conn_Rm, query) < 0) {
			ErrCode = mysql_errno(conn_Rm);
			if (ErrCode == 1053) {
				sprintf(trcBuf, "Mysql Connect Fail : query:%s, err = %s\n", query, mysql_error(conn_Rm));
				trclib_writeErr (FL,trcBuf);
			}
			sprintf(trcBuf, "select fail Remote: query:%s, err = %s\n", query, mysql_error(conn_Rm));
			return -1;
		}
		res_rm = mysql_store_result(conn_Rm);
		row_rm = mysql_fetch_row(res_rm);

		if( row_rm != NULL )
			cmINICnt = atoi(row_rm[0]);

		mysql_free_result(res_rm);
	}
	else
		return -1;

	if( ompINICnt == 0 && cmINICnt == 0 )
	{
		sprintf(trcBuf, "Critical Not Exist INI_VALUES table DSC, DSCM Check Table\n");
		trclib_writeErr (FL,trcBuf);
		return -1;
	}

	if( ompINICnt == cmINICnt && g_ruleEntryCnt != 0 ) // CM과 DSCM의 INI_VALUES table 일치 
	{
// 2010. 12. 22
		logPrint(trcLogId, FL, "INI VALUES EXACT CNT[%d]\n", ompINICnt);
		for( i = 0; i < SCE_CNT; i++ )
			logPrint(trcLogId, FL, "SCE[%d] RULESET CNT[%d]\n", i, g_stSCERule[i].ruleSetCnt);
		return 0;
	}
	else // 일치 하지 않는 경우, DSCM의 INI_VALUES를 지우고 CM의 값을 집어 넣는다. 
	{
		logPrint(trcLogId, FL, "INI VALUES MISMATCH OMP[%d] MP[%d]\n", ompINICnt, cmINICnt);

		if( conn != NULL && conn_Rm != NULL )
		{
        	memset(query, 0x00, sizeof(query));
        	snprintf(query, 1024, "delete from INI_VALUES");
			if (stmd_mysql_query (query) < 0) {
				sprintf(trcBuf, "delete fail Remote : err = %s\n", mysql_error(conn));
				trclib_writeErr (FL,trcBuf);
				return -1;
			}

			memset(query_rm, 0x00, sizeof(query_rm));
			snprintf(query_rm, 1024, "select * from INI_VALUES ");
			if (stmd_mysql_query_with_conn (conn_Rm, query_rm) < 0) {
				sprintf(trcBuf, "select fail Remote : err = %s\n", mysql_error(conn_Rm));
				trclib_writeErr (FL,trcBuf);
				return -1;
			}

			res_rm = mysql_store_result(conn_Rm);
			while( ( row_rm = mysql_fetch_row(res_rm)) != NULL )
			{
				memset(query, 0x00, sizeof(query));
				snprintf(query, 1024, "INSERT INTO INI_VALUES values ('%s', '%s', '%s', '%s', '%s')",\
										row_rm[0], row_rm[1], row_rm[2], row_rm[3], row_rm[4]);
				if (stmd_mysql_query (query) < 0) {
					sprintf(trcBuf, "query fail: query:%s, err = %s\n", query, mysql_error(conn));
					trclib_writeErr (FL,trcBuf);
					continue;
				}
			}
			mysql_free_result(res_rm);
		}

		// ENTRY 구조체 다시  sync 맞추기 
		memset(query,0x00,sizeof(query));

		sprintf(query,"SELECT distinct value_key, VALUE FROM INI_VALUES WHERE value_type=1 "
		" ORDER BY VALUE, se_ip DESC ");

		if (stmd_mysql_query (query) < 0) {
			sprintf(trcBuf, "query fail: query:%s, err = %s\n", query, mysql_error(conn));
			trclib_writeErr (FL,trcBuf);
			return -1;
		}
		res = mysql_store_result(conn);                                                        

		for( i = 0; i < SCE_CNT; i++ )
			g_stSCEEntry[i].ruleEntryCnt = 0;

		cnt =  0;
		while( (row = mysql_fetch_row(res)) != NULL)
		{
			entry = atoi(row[1]);
			for(i = 0; i < SCE_CNT; i++ )
			{
				g_stSCEEntry[i].stEntry[cnt].real = 1;
				g_stSCEEntry[i].stEntry[cnt].eFlag = 0;
				g_stSCEEntry[i].stEntry[cnt].eId = entry;
				strcpy(g_stSCEEntry[i].stEntry[cnt].eName,row[0]);
				g_stSCEEntry[i].ruleEntryCnt++;
				g_ruleEntryBuf[cnt] = entry;
				strcpy(g_ruleEntryName[cnt],row[0]); 
				g_ruleEntryCnt++;
					
			}
			cnt++;
		}

		mysql_free_result(res);
	}
	return 0;
}

int	Insert_default_ruleset(void)
{
	int i, j, ret = 0;
	char	query[2048] = {0,};

	for (i=0; i < SCE_CNT; i++) 
	{
		logPrint(trcLogId, FL, "default ruleset Cnt: %d\n",g_stSCERule[i].ruleSetCnt );
		
		for( j = 0; j < MAX_RULE_NUM; j++ )
		{
			if( g_stSCERule[i].stRule[j].real == 1 )
			{
				sprintf(query, "INSERT INTO %s VALUES ( "
						" '%s', " // sce_ip
						"  %d, "  // pkg_id = 20, 4999
						" '%s', " // rule_set_id = 00 00, 01 01
						" '%s', " // rule_set_name = 'Default Package'
						"  0, " // session
						"  0, " // upstream_volume
						"  0, " // downstream_volume
						"  0, " // block_cnt
						"  0, " // redirect_cnt
						"  1 , " // stat_cnt
						" '%s', " // stat_date
						" '%s' )" // stat_week 
						, STM_STATISTIC_5MINUTE_RULESET_TBL_NAME
						, g_stSCE[i].sce_name // sce_ip
						, g_stSCERule[i].stRule[j].rId
						, g_stSCERule[i].stRule[j].phBit
						, g_stSCERule[i].stRule[j].rName
						, get_insert_time()
						, get_insert_week());

				if (stmd_mysql_query (query) < 0) {
					sprintf(trcBuf, "insert default ruleset query:%s, err = %s\n", query, mysql_error(conn));
					trclib_writeErr (FL,trcBuf);
					ret = -1; // insert 실패  
					continue;
				}
				else
					ret = 1; // insert 성공
			}
		}
		keepalivelib_increase();                                                           
	}                           

	return ret;
}

int	Insert_default_ruleent(void)
{
	int i, j, ret = 0;
	char	query[2048] = {0,};


	for (i=0; i < SCE_CNT; i++) {

		logPrint(trcLogId, FL, "default ruleent Cnt : %d\n",g_stSCEEntry[i].ruleEntryCnt );

		for (j=0; j < MAX_ENTRY_NUM; j++) {

			if( g_stSCEEntry[i].stEntry[j].real == 1 )
			{
				sprintf(query, "INSERT INTO %s VALUES ( "
						" '%s', " // sce_ip
						"  %d, "  // service_id = 
						" '%s', " // rule_entry_name
						" 0, " // session
						" 0, " // upstream_volume
						"  0, " // downstream_volume
						"  0, " // block_cnt
						"  0, " // redirect_cnt
						"  1, " // stat_cnt
						" '%s', " // stat_date
						" '%s' )" // stat_week 
						, STM_STATISTIC_5MINUTE_RULEENT_TBL_NAME
						, g_stSCE[i].sce_name // sce_ip --> system name
						, g_stSCEEntry[i].stEntry[j].eId // service_id
						, g_stSCEEntry[i].stEntry[j].eName // service_ name
						, get_insert_time() // stat_date
						, get_insert_week() ); // stat_week

				if (stmd_mysql_query (query) < 0) {
					sprintf(trcBuf, "insert default ruleent fail query:%s, err = %s\n", query, mysql_error(conn));     
					trclib_writeErr (FL,trcBuf);                                               
					ret = -1; // insert ½CÆÐ                                              
					continue;                                                                  
				}                                                                              
			}
		}                                                                                  
		stmd_msg_receiver();                                                               
		keepalivelib_increase();                                                           
	}                           

	return ret;
}

/** Flow 5minute Init 2010.08.23 */
int Insert_default_flow(void)
{
	int 	ret = 0;
	char	query[2048] = {0,};

	/** SYSNAME, AVG, MIN, MAX, 1, 시간, 주 2010.08.23 */
	sprintf(query, "Insert into flow_5minute_statistics values "
			" ( '%s', 0, 0, 0, 1, '%s', '%s' ) ",
			"SCEA", get_insert_time(), get_insert_week());

	if (stmd_mysql_query (query) < 0) {
		sprintf(trcBuf, "query fail: query:%s, err = %s\n", query, mysql_error(conn));
		trclib_writeErr (FL,trcBuf);
		ret = -1;
	}

	sprintf(query, "Insert into flow_5minute_statistics values "
			" ( '%s', 0, 0, 0, 1, '%s', '%s' ) ",
			"SCEB", get_insert_time(), get_insert_week());

	if (stmd_mysql_query (query) < 0) {
		sprintf(trcBuf, "query fail: query:%s, err = %s\n", query, mysql_error(conn));
		trclib_writeErr (FL,trcBuf);
		ret = -1;
	}

	return ret;
}

int Insert_default_sms(void)
{
	int i, j, ret = 0;
	char	query[2048] = {0,};

	for(i = 0;  i < 2; i++ )
	{
		for( j = 0; j < g_stSmsc[i].smscCnt; j++ )
		{
			// SMSC 5min default insert 
			if( i == 0 )
			{
				sprintf(query, "Insert into sms_5minute_statistics values "
						" ( '%s', '%s', 0, 0, 0, 0, 0, 0, 0, 1, '%s', '%s' ) ",
						"SCMA", g_stSmsc[i].stItem[j].ip, get_insert_time(), get_insert_week());

				if (stmd_mysql_query (query) < 0) {
					sprintf(trcBuf, "query fail: query:%s, err = %s\n", query, mysql_error(conn));
					trclib_writeErr (FL,trcBuf);
					ret = -1;
					continue;
				}
			}
			else
			{
				sprintf(query, "Insert into sms_5minute_statistics values "
						" ( '%s', '%s', 0, 0, 0, 0, 0, 0, 0, 1, '%s', '%s' ) ",
						"SCMB", g_stSmsc[i].stItem[j].ip, get_insert_time(), get_insert_week());

				if (stmd_mysql_query (query) < 0) {
					sprintf(trcBuf, "query fail: query:%s, err = %s\n", query, mysql_error(conn));
					trclib_writeErr (FL,trcBuf);
					ret = -1;
					continue;
				}
			}
		}
	}

	return ret;
}

int	Insert_default_lur(void)
{
	int i, j, ret = 0;
	char	query[2048] = {0,};

	for(i = 0; i < SCE_CNT; i++ )
	{
		for(j = 0; j < 2; j++ )
		{
			memset(query, 0x00, sizeof(query));
			snprintf(query,sizeof(query), "INSERT INTO %s VALUES ('%s',%d,0,0,1,'%s','%s')",  
					STM_STATISTIC_5MINUTE_LUR_TBL_NAME,
					g_stSCE[i].sce_name, j+1, get_insert_time(), get_insert_week());

			if (stmd_mysql_query (query) < 0) {
				sprintf(trcBuf, "insert fail Link Default query=%s : err = %s\n",query, mysql_error(conn));
				trclib_writeErr (FL,trcBuf);
				ret = -1;
			}
		}
	}
	return ret;
}

int stmd_LoadStatisticLUR(time_t t_start)
{
    int     i;
    char    query[2048] = {0,};
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int         ret_flag;
	int 		loop = 0;
	int exist_stat[2] = {0,};

    memset(query, 0x00, sizeof(query));
    sprintf(query, "DELETE FROM %s WHERE stat_date < '%s'", \
			STM_STATISTIC_5MINUTE_LUR_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_MIN]));
    if (stmd_mysql_query (query) < 0) {
		sprintf(trcBuf, "rdr_lur 5minute delete fail :query:%s, err = %s\n",query, mysql_error(conn));
		trclib_writeErr (FL,trcBuf);
		return -1;
    }

	ret_flag = Insert_default_lur();
	if( ret_flag < 0 )
	{
		sprintf(trcBuf, "rdr_lur 5minute default insert fail. query:%s, err = %s\n",
				query, mysql_error(conn));
		trclib_writeErr (FL,trcBuf);
	}
    memset(query, 0x00, sizeof(query));
	/*
    snprintf(query, sizeof(query), SELECT_RPT_LUR, RPT_LUR, get_insert_time3(),
			get_insert_time());
	*/
	snprintf(query, sizeof(query), SELECT_RPT_LUR, RPT_LUR, t_start-300, t_start);

	if(conn_Rm == NULL ) 
		return -1;

	if (stmd_mysql_query_with_conn (conn_Rm, query) < 0) {
		sprintf(trcBuf, "select fail Remote : err = %s\n", mysql_error(conn_Rm));
		trclib_writeErr (FL,trcBuf);
        return -2;
    }

    result = mysql_store_result(conn_Rm);
    while( ( row = mysql_fetch_row(result)) != NULL )
    {
		stmd_msg_receiver();
        keepalivelib_increase();

		for(i = 0; i < SCE_CNT; i++ )
		{
			if( g_stSCE[i].sce_id == atoi(row[0]) )
				break;
		}
		memset(query, 0x00, sizeof(query));
		sprintf(query, "UPDATE %s SET upstream_volume = %s, downstream_volume = %s "
				" where stat_date = '%s' and record_source = '%s' and link_id = %d ",
				STM_STATISTIC_5MINUTE_LUR_TBL_NAME, 
				row[2], row[3], get_insert_time(), g_stSCE[i].sce_name, atoi(row[1]));

		loop++;

		if (stmd_mysql_query (query) < 0) {
			sprintf(trcBuf, "5minute Insert/Update fail Local : query:%s;err = %s\n", query,
					mysql_error(conn));
			trclib_writeErr (FL,trcBuf);
            ret_flag = -1; // insert 실패  
        }
        else
            ret_flag = 1; // insert 성공
		
		// Get Current Throughput Value
		if( !strncasecmp(g_stSCE[i].sce_name, "SCEA", 4) )
		{
//			g_stSvcVal.traffic_A[1] += (double)(atoi(row[2])+atoi(row[3]))*8/1024/300 ;
			g_stSvcVal.traffic_A[1] += (atoi(row[2])+atoi(row[3])) ;
			exist_stat[0] = 1;
		}
		else
		{
//			g_stSvcVal.traffic_B[1] += (double)(atoi(row[2])+atoi(row[3]))*8/1024/300  ;
			g_stSvcVal.traffic_B[1] += atoi(row[2])+atoi(row[3]);
			exist_stat[1] = 1;
		}

    }
	mysql_free_result(result);

//	g_stSvcVal.trafficSum[1] = g_stSvcVal.traffic_A[1] + g_stSvcVal.traffic_B[1] ;
	// SCEA, SCEB 의 현재 통계 값이 없으면 현재값으로 0 을 셋팅 
	if( exist_stat[0] == 0 )
		g_stSvcVal.traffic_A[1] = 0;
	else
		g_stSvcVal.traffic_A[1] = g_stSvcVal.traffic_A[1]*8/1024/300;


	if( exist_stat[1] == 0 )
		g_stSvcVal.traffic_B[1] = 0;
	else
		g_stSvcVal.traffic_B[1] = g_stSvcVal.traffic_B[1]*8/1024/300;

    if( ret_flag == 1)
		logPrint(trcLogId, FL, "rdr_lur insert success : %ld, loop : %d\n", time(0), loop);

    return ret_flag;
}

int stmd_LoadStatisticTR(time_t t_start)
{
    int     i;
    char    query[2048] = {0,};
    char    query_format[4096] = {0,};
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int         ret_flag;
	int			sce_id = 0, loop = 0;
	char	start_time[24] = {0,};
	char	SCEIP[32] = {0,}, SCENAME[10] = {0,};

    sprintf(query, "DELETE FROM %s WHERE stat_date < '%s'", \
				STM_STATISTIC_5MINUTE_TR_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_MIN]));

	if (stmd_mysql_query (query) < 0) {
		sprintf(trcBuf, "rdr_tr 5minute delete fail :query:%s, err = %s\n",query, mysql_error(conn));
		trclib_writeErr (FL,trcBuf);
		return -1;
    }
#if 0
	//20110210 by dcham, STMD Hang Up 문제로 RDRANA로 이동
	/* RPT_TR delete 05.25 */
	sprintf(query, "DELETE FROM %s WHERE end_time < %ld", \
			RPT_TR, (t_start) - STMD_1HOUR_OFFSET*delTIME[STMD_MIN]);

	logPrint(trcLogId, FL, "RPT_TR DEL REMOTE QUERY : %s\n", query); // 2010. 12. 22

	if(conn_Rm == NULL )
	{
		sprintf(trcBuf, "RPT_TR conn_Rm is NULL\n"); // 2010. 12. 22
		trclib_writeErr (FL,trcBuf);				// 2010. 12. 22
		return -1;
	}
	else
	{
		sprintf(trcBuf, "RPT_TR conn_Rm is NOT NULL\n"); // 2010. 12. 22
		trclib_writeErr (FL,trcBuf);				// 2010. 12. 22
	}

	if (stmd_mysql_query_with_conn (conn_Rm, query) < 0) {
		sprintf(trcBuf, "RPT_TR table delete fail :query:%s, err = %s\n",query, mysql_error(conn_Rm));
		trclib_writeErr (FL,trcBuf);
	}
#endif

    memset(query, 0x00, sizeof(query));

/*
    sprintf(query, "SELECT RECORD_SOURCE, PACKAGE_ID, SERVICE_ID, SUM(SAMPLE_SIZE), "
                        " sum(UPSTREAM_VOLUME*SAMPLE_SIZE), sum(DOWNSTREAM_VOLUME*SAMPLE_SIZE) "
                    "FROM %s "
                    "WHERE time_stamp >= '%s' and time_stamp < '%s' "
                    "GROUP BY RECORD_SOURCE, PACKAGE_ID, SERVICE_ID ", RPT_TR, 
					get_insert_time3(), get_insert_time() );
*/
	 sprintf(query, "SELECT RECORD_SOURCE, PACKAGE_ID, SERVICE_ID, SUM(SAMPLE_SIZE), "
                        " sum(UPSTREAM_VOLUME*SAMPLE_SIZE), sum(DOWNSTREAM_VOLUME*SAMPLE_SIZE) "
                    "FROM %s "
                    "WHERE end_time >= %ld and end_time < %ld "
                    "GROUP BY RECORD_SOURCE, PACKAGE_ID, SERVICE_ID ", 
					RPT_TR, t_start-300, t_start );

	if(conn_Rm == NULL )
	{
		sprintf(trcBuf, "RPT_TR conn_Rm is NULL\n"); // 2010. 12. 22
		trclib_writeErr (FL,trcBuf);				// 2010. 12. 22
		return -1;
	}

	if (stmd_mysql_query_with_conn (conn_Rm, query) < 0) {
		sprintf(trcBuf, "select fail Remote : err = %s\n", mysql_error(conn_Rm));
		trclib_writeErr (FL,trcBuf);
        return -2;
    }

	strcpy(start_time, get_insert_time());

    result = mysql_store_result(conn_Rm);
    while( ( row = mysql_fetch_row(result)) != NULL )
    {
		stmd_msg_receiver();
        keepalivelib_increase();

        memset(query, 0x00, sizeof(query));
        memset(query_format, 0x00, sizeof(query_format));

		sce_id = atoi(row[0]);
		for(i = 0; i < SCE_CNT; i++)
		{
			if( sce_id == g_stSCE[i].sce_id )
			{
				memset(SCEIP, 0x00, sizeof(SCEIP));
				strcpy(SCEIP, g_stSCE[i].sce_ip);
				memset(SCENAME, 0x00, sizeof(SCENAME));
				strcpy(SCENAME, g_stSCE[i].sce_name);
				break;
			}
		}

    	memset(query, 0x00, sizeof(query));
        sprintf(query, "INSERT INTO %s VALUES ", STM_STATISTIC_5MINUTE_TR_TBL_NAME);
		sprintf(query_format, "('%s',%s,%s,%s,%s, "
								"%s, 1,'%s','%s')",\
								SCENAME,row[1],row[2],row[3],row[4],\
								row[5], start_time, get_insert_week());

        sprintf(query, "%s %s", query, query_format);
		loop++;

		if (stmd_mysql_query (query) < 0) {
			sprintf(trcBuf, "insert tr 1 fail Local : err = %s\n", mysql_error(conn));
			trclib_writeErr (FL,trcBuf);
            ret_flag = -1; // insert 실패  
            continue;
        }
        else
            ret_flag = 1; // insert 성공
    }
	
	mysql_free_result(result);

    if( ret_flag == 1)
		logPrint(trcLogId, FL, "rdr_tr insert success : %ld, %d\n", time(0), loop);

    return ret_flag;
}

int readRuleConfFile(char *fname)
{
	FILE *fp = NULL;
	int No = 0, Pbit = 0, Hbit = 0, PkgNo = 0, RedNo = 0, SmsOnOff = 0, i = 0, j = 0;
	int cnt = 0, index = 0;
	char buf[128] = {0,};
	char query[1024] = {0,};
	char sceIp[20] = {0,};
	int *ruleIdBuf = g_ruleIdBuf;
	RuleSetList *pstSCERule = &g_stSCERule[0];

	MYSQL_RES	*res;
	MYSQL_ROW	row;

	if( (fp = fopen(fname, "r")) != NULL )
	{
		while( (fgets(buf, sizeof(buf), fp)) != NULL )
		{
			if (buf[0] == '#' )
				continue;
			else
			{
				sscanf( buf, "%d %d %d %d %d %d", &No, &Pbit, &Hbit, &PkgNo, &RedNo, &SmsOnOff );
				ruleIdBuf[cnt] = PkgNo; // RULESET_LIST.conf 파일에 등록된 Rule Id 
				for(i = 0; i < SCE_CNT; i++)
				{
					pstSCERule[i].stRule[PkgNo].rId = PkgNo;
                    sprintf(pstSCERule[i].stRule[PkgNo].phBit, "%02d %02d", Pbit, Hbit);
				}
				cnt++;
			}
		}

		sprintf(query, "select se_ip, value_key, value from INI_VALUES "
						" where value_type=2 group by se_ip,value_key,value order by value");

		if( conn == NULL )
		{
			fclose(fp);
			return -1;
		}

		if (stmd_mysql_query (query) < 0) {
			sprintf(trcBuf, "select fail :query:%s, err = %s\n",query, mysql_error(conn));
			trclib_writeErr (FL,trcBuf);
			fclose(fp);
			return -1;
		}

    	res = mysql_store_result(conn);
    	while( ( row = mysql_fetch_row(res)) != NULL )
    	{
			memset(sceIp,0x00,sizeof(sceIp));
			strcpy(sceIp, row[0]);
			index = atoi(row[2]);
			for( i = 0; i < cnt; i++ )
			{
				if( g_ruleIdBuf[i] == index )
				{
					for( j = 0; j < SCE_CNT; j++ )
					{
						if(!strcmp(sceIp, g_stSCE[j].sce_ip))
						{
							strcpy(pstSCERule[j].stRule[index].rName, row[1]);
							strcpy(pstSCERule[j].sce_ip, sceIp);
							pstSCERule[j].stRule[index].real = 1;
							pstSCERule[j].ruleSetCnt++;
							break;
						}
					}
					break;
				}
			}
		}
		mysql_free_result(res);
	}

	fclose(fp);

	// 2010. 12. 22
	for( i = 0; i < SCE_CNT; i++)
		logPrint(trcLogId, FL, "SCE[%d] RuleSetCnt:%d\n", i, pstSCERule[i].ruleSetCnt);

	return cnt;
}


int stmd_LoadStatisticRULESET(time_t t_start)
{
    int     i;
    char    query[4098] = {0,};
    char    query_format[4096] = {0,}; 
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int         ret_flag;
	int			rule_id = 0;
	char	SCENAME[32] = {0,}, start_time[32]= {0,};

	/*
    sprintf(query, "DELETE FROM %s WHERE stat_date < '%s'", 
			STM_STATISTIC_5MINUTE_RULESET_TBL_NAME, get_delete_time(STMD_1HOUR_OFFSET*7));
			*/
    sprintf(query, "DELETE FROM %s WHERE stat_date < '%s'", 
			STM_STATISTIC_5MINUTE_RULESET_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_MIN]));
    if (stmd_mysql_query (query) < 0) {
		sprintf(trcBuf, "rdr_ruleset 5minute delete fail :query:%s, err = %s\n",query, mysql_error(conn));
		trclib_writeErr (FL,trcBuf);
		return -1;
    }

    memset(query, 0x00, sizeof(query));

	strcpy(start_time, get_insert_time());

	sprintf (query, " SELECT aa.record_source,aa.package_id, aa.rule_set_name, IFNULL(bb.sess,0), IFNULL(bb.up,0), IFNULL(bb.down,0), IFNULL(aa.blk,0), IFNULL(aa.red,0) "
					" FROM (SELECT record_source, package_id, VALUE_KEY rule_set_name,  blk,  red "
					" FROM ( "
					" SELECT record_source, package_id, SUM(block_rdr_cnt) blk, 0 red "
					" FROM rdr_block_5minute_statistics "
					" WHERE stat_date = '%s' "
					" GROUP BY record_source, package_id ) a , (  "
					" SELECT DISTINCT value_key, VALUE FROM INI_VALUES WHERE value_type = 2 ) b "
					" WHERE a.package_id = b.value) aa "
					" LEFT JOIN "
					" (SELECT record_source, package_id, VALUE_KEY rule_set_name, sess, up, down "
					" FROM (  "
					" SELECT record_source, package_id, SUM(SESSION) sess, SUM(upstream_volume) up,  "
					" SUM(downstream_volume) down, stat_date, stat_week "
					" FROM rdr_tr_5minute_statistics  "
					" WHERE stat_date = '%s' "
					" GROUP BY record_source, package_id  "
					" )a , "
					" ( SELECT DISTINCT value_key, VALUE FROM INI_VALUES WHERE value_type = 2  "
					" ) b "
					" WHERE a.package_id = b.value) bb "
					" ON aa.record_source = bb.record_source AND aa.package_id = bb.package_id ", start_time, start_time);

	logPrint(trcLogId, FL, "QUERY : %s\n", query);

	if( conn == NULL )
		return -1;

	if (stmd_mysql_query(query) < 0) {
		sprintf(trcBuf, "select fail Local : err = %s\n", mysql_error(conn));
		trclib_writeErr (FL,trcBuf);
        return -2;
    }

    result = mysql_store_result(conn);
    while( ( row = mysql_fetch_row(result)) != NULL )
    {
		stmd_msg_receiver();
        keepalivelib_increase();
		
        memset(query, 0x00, sizeof(query));
        memset(query_format, 0x00, sizeof(query_format));

		sprintf(SCENAME,"%s",row[0]);
		rule_id = atoi(row[1]);
		for( i = 0; i < SCE_CNT; i++ )
		{
			if(!strcmp(g_stSCE[i].sce_name, SCENAME))
			{
				g_stSCERule[i].stRule[rule_id].eFlag = 1;
				break;
			}
		}

        memset(query, 0x00, sizeof(query));
	
		if( i != 2 )
		{
			sprintf(query, "INSERT INTO %s VALUES ( "
					" '%s', " // sce_ip
					"  %s, "  // pkg_id = 20, 4999
					" '%s', " // rule_set_id = 00 00, 01 01
					" '%s', " // rule_set_name = 'Default Package'
					"  %s, " // session
					"  %s, " // upstream_volume
					"  %s, " // downstream_volume
					"  %s, " // block_cnt
					"  %s, " // redirect_cnt
					"  1 , " // stat_cnt
					" '%s', " // stat_date
					" '%s' )" // stat_week 
					, STM_STATISTIC_5MINUTE_RULESET_TBL_NAME
					, g_stSCE[i].sce_name // sce_ip
					, row[1] // pkg_id
					, g_stSCERule[i].stRule[rule_id].phBit // rule_set_id , row[2] // rule_set_name
					, g_stSCERule[i].stRule[rule_id].rName // rule_set_name
					, row[3] // session
					, row[4] // upstream_volume
					, row[5] // downstream_volume
					, row[6] // block_cnt
					, row[7] // redirect_cnt
					, start_time
					, get_insert_week());

			if (stmd_mysql_query (query) < 0) {
				sprintf(trcBuf, "insert fail Local query:%s, err = %s\n", query, mysql_error(conn));
				trclib_writeErr (FL,trcBuf);
				ret_flag = -1; // insert 실패  
				continue;
			}
			else
				ret_flag = 1; // insert 성공
		}

	} // end-of-while fetch  
	mysql_free_result(result);

    if( ret_flag == 1)
		logPrint(trcLogId, FL, "rdr_ruleset insert from [ rdr_tr + rdr_block ] success : %ld\n", time(0));

    return ret_flag;
}

int stmd_LoadStatisticDELAY(time_t t_start)
{
    char    query[4096] = {0,};
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int     ret_flag = 0;
	char	start_time[32] = {0,}, end_time[32] = {0,};

	/*
    sprintf(query, "DELETE FROM %s WHERE stat_date < '%s'", 
			STM_STATISTIC_5MINUTE_DELAY_TBL_NAME, get_delete_time(STMD_1HOUR_OFFSET*7));
			*/
    sprintf(query, "DELETE FROM %s WHERE stat_date < '%s'", 
			STM_STATISTIC_5MINUTE_DELAY_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_MIN]));
    if (stmd_mysql_query (query) < 0) {
		sprintf(trcBuf, "delay 5minute delete fail :query:%s, err = %s\n",query, mysql_error(conn));
		trclib_writeErr (FL,trcBuf);
		return -1;
	}

	strcpy(start_time, get_insert_time3());
	strcpy(end_time, get_insert_time());

	sprintf(query, "SELECT count(*) from %s where stat_date = '%s'", 
			STM_STATISTIC_5MINUTE_DELAY_TBL_NAME, get_insert_time());

	if (stmd_mysql_query (query) < 0) {
		sprintf(trcBuf, "select fail Remote : err = %s\n", mysql_error(conn));
		trclib_writeErr (FL,trcBuf);
        return -2;
    }

    result = mysql_store_result(conn);
    row = mysql_fetch_row(result);

	if( row == NULL )
		ret_flag = 0;
	else if ( row[0] != NULL )
		ret_flag = atoi(row[0]);
	else
		ret_flag = 0;

	if( ret_flag == 0 )
	{
		memset(query, 0x00, sizeof(query));

		sprintf(query, "INSERT INTO %s VALUES ('SCE','%s','%s',0,0,0,0) ",
				STM_STATISTIC_5MINUTE_DELAY_TBL_NAME ,get_insert_time(), get_insert_time());

		if (stmd_mysql_query (query) < 0) {
			sprintf(trcBuf, "insert fail Local query:%s, err = %s\n", query, mysql_error(conn));
			trclib_writeErr (FL,trcBuf);
		}
		else
			ret_flag = 1; // insert 성공
	} 
	mysql_free_result(result);


    if( ret_flag == 1)
		logPrint(trcLogId, FL, "sms_5min insert from sms_history success : %ld\n", time(0));

    return ret_flag;
}

int stmd_LoadStatisticLeg(time_t t_start)
{
    int     i,j;
    char    query[4096] = {0,};
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int     ret_flag;

	/*
    sprintf(query, "DELETE FROM %s WHERE stat_date < '%s'", 
			STM_STATISTIC_5MINUTE_LEG_TBL_NAME, get_delete_time(STMD_1HOUR_OFFSET*7));
			*/
    sprintf(query, "DELETE FROM %s WHERE stat_date < '%s'", 
			STM_STATISTIC_5MINUTE_LEG_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_MIN]));
    if (stmd_mysql_query (query) < 0) {
		sprintf(trcBuf, "leg 5minute delete fail :query:%s, err = %s\n",query, mysql_error(conn));
		return -1;
    }

	sprintf(query, " select count(*) from leg_5minute_statistics where stat_date = '%s' ", get_insert_time());
	if (stmd_mysql_query (query) < 0) {
		sprintf(trcBuf, "insert fail Local query:%s, err = %s\n", query, mysql_error(conn));
		trclib_writeErr (FL,trcBuf);
	}

    result = mysql_store_result(conn);
    row = mysql_fetch_row(result);
	if( row == NULL )
	{	
		mysql_free_result(result); // MEM if NULL 10.07
		return -1;
	}
	else
	{
		if(atoi(row[0]) == 0 )
		{
			for( i  = 0; i < 2; i++ )
			{
				for( j = 0; j < g_stPdsn[i].pdsnCnt; j++ )
				{
					sprintf(query, "Insert into leg_5minute_statistics values "
							" ( '%s', '%s', 0, 0, 0, 0, 0, 0, 0, 1, '%s', '%s' ) ",
							g_stPdsn[i].pdsnName, g_stPdsn[i].stItem[j].ip, get_insert_time(), get_insert_week());

					if (stmd_mysql_query (query) < 0) {
						sprintf(trcBuf, "query fail: query:%s, err = %s\n", query, mysql_error(conn));
						trclib_writeErr (FL,trcBuf);
						mysql_free_result(result); // MEM if fail 10.07
						return -1;
					}
				}
			}
		}
	}
	mysql_free_result(result);

    return ret_flag;
}

#if 0
int stmd_LoadStatisticLogon(time_t t_start)
{
    char    query[4096] = {0,};
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int     ret_flag;

	sprintf(trcBuf, "logon 5minute insert start:%ld\n", time(0));
	trclib_writeLogErr (FL,trcBuf);

	/*
    sprintf(query, "DELETE FROM %s WHERE stat_date < '%s'", 
			STM_STATISTIC_5MINUTE_LOGON_TBL_NAME, get_delete_time(STMD_1HOUR_OFFSET*7));
			*/
    sprintf(query, "DELETE FROM %s WHERE stat_date < '%s'", 
			STM_STATISTIC_5MINUTE_LOGON_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_MIN]));

    if (stmd_mysql_query (query) < 0) {
		sprintf(trcBuf, "logon 5minute delete fail :query:%s, err = %s\n",query, mysql_error(conn));
		trclib_writeErr (FL,trcBuf);
    }

	sprintf(query, " select count(*) from logon_5minute_statistics where stat_date = '%s' ", get_insert_time());
	if (stmd_mysql_query (query) < 0) {
		sprintf(trcBuf, "insert fail Local query:%s, err = %s\n", query, mysql_error(conn));
		trclib_writeErr (FL,trcBuf);
	}

    result = mysql_store_result(conn);
    row = mysql_fetch_row(result);
	if(row == NULL )
	{
		mysql_free_result(result); // MEM if NULL 10.07
		return -1;
	}
	else
	{
		if(atoi(row[0]) == 0 )
		{
			sprintf(query, "insert into %s values ( '%s', %u, %u, %u,"
					"%u, %u, %u, %u,"
					"%u, %u, %u, %u, %u,"
					"%d, '%s', '%s' )",
					STM_STATISTIC_5MINUTE_LOGON_TBL_NAME, "SCMA", 0,0,0, 0,0,0,0 ,0,0,0,0,0, 1,
					get_insert_time(), get_insert_week());

			if (stmd_mysql_query (query) < 0) {
				sprintf(trcBuf, "insert logon_5minute mysql_query fail:%s\n", query);
				trclib_writeLogErr (FL,trcBuf);
				mysql_free_result(result); // MEM if fail 10.07
				return -1;
			}
			sprintf(query, "insert into %s values ( '%s', %u, %u, %u,"
					"%u, %u, %u, %u,"
					"%u, %u, %u, %u, %u,"
					"%d, '%s', '%s' )",
					STM_STATISTIC_5MINUTE_LOGON_TBL_NAME, "SCMB", 0,0,0, 0,0,0,0 ,0,0,0,0,0, 1,
					get_insert_time(), get_insert_week());

			if (stmd_mysql_query (query) < 0) {
				sprintf(trcBuf, "insert logon_5minute mysql_query fail:%s\n", query);
				trclib_writeLogErr (FL,trcBuf);
				mysql_free_result(result); // MEM if fail 10.07
				return -1;
			}
		}
	}
	mysql_free_result(result);

	sprintf(trcBuf, "logon 5minute insert end:%ld\n", time(0));
	trclib_writeLogErr (FL,trcBuf);

    return ret_flag;
}
#endif

/** FLOW 5분 통계 : FLOW REPORT IMSI 테이블에서 Group by */
int stmd_LoadStatisticFLOW(time_t t_start)
{
    char    query[4096] = {0,};
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int     ret_flag;
	char	start_time[32] = {0,}, end_time[32] = {0,};

	// IMSI TABLE Delete : flow 임시 테이블 삭제 
	sprintf(query, "DELETE FROM %s WHERE (StringTime < '%s')",
			STM_REPORT_IMSI_FLOW_TBL_NAME, get_delete_time(STMD_5MIN_OFFSET));

	if ( trcLogFlag == TRCLEVEL_SQL ) {
		sprintf( trcBuf, "query = %s\n", query);
		trclib_writeLog(FL, trcBuf);
	}
	if (stmd_mysql_query (query) < 0) {
		sprintf(trcBuf, "flow imsi table mysql_delete fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

    sprintf(query, "DELETE FROM %s WHERE stat_date < '%s'", 
			STM_STATISTIC_5MINUTE_FLOW_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_MIN]));
    if (stmd_mysql_query (query) < 0) {
		sprintf(trcBuf, "sms 5minute delete fail :query:%s, err = %s\n",query, mysql_error(conn));
		trclib_writeErr (FL,trcBuf);
		return -1;
    }

	strcpy(start_time, get_insert_time3());
	strcpy(end_time, get_insert_time());

	// FLOW 5min default insert 
	sprintf(query, "Insert into flow_5minute_statistics values "
			" ( '%s', 0, 0, 0, 1, '%s', '%s' ) ",
			"SCEA", get_insert_time(), get_insert_week());

	if (stmd_mysql_query (query) < 0) {
		sprintf(trcBuf, "query fail: query:%s, err = %s\n", query, mysql_error(conn));
		trclib_writeErr (FL,trcBuf);
		return -1;
	}

	sprintf(query, "Insert into flow_5minute_statistics values "
			" ( '%s', 0, 0, 0, 1, '%s', '%s' ) ",
			"SCEB", get_insert_time(), get_insert_week());

	if (stmd_mysql_query (query) < 0) {
		sprintf(trcBuf, "query fail: query:%s, err = %s\n", query, mysql_error(conn));
		trclib_writeErr (FL,trcBuf);
		return -1;
	}

	// FLOW REPORT GROUP BY
    memset(query, 0x00, sizeof(query));

	sprintf(query, "SELECT system_name, AVG(FlowNum), MIN(FlowNum), MAX(FlowNum) "
				" FROM flow_report_imsi "
				" WHERE StringTime >= '%s' AND StringTime < '%s' "
				" GROUP BY system_name", start_time, end_time);

	if (stmd_mysql_query (query) < 0) {
		sprintf(trcBuf, "select fail Remote : err = %s\n", mysql_error(conn));
		trclib_writeErr (FL,trcBuf);
        return -2;
    }

    result = mysql_store_result(conn);
    while( ( row = mysql_fetch_row(result)) != NULL )
    {
        memset(query, 0x00, sizeof(query));
		sprintf(query, "UPDATE  %s SET avg_flow = %s, min_flow = %s, max_flow = %s "
						" where system_name = '%s' and stat_date = '%s' " 
						, STM_STATISTIC_5MINUTE_FLOW_TBL_NAME
						, row[1], row[2], row[3],
						row[0], get_insert_time()); // stat_week

		if (stmd_mysql_query (query) < 0) {
			sprintf(trcBuf, "insert fail Local query:%s, err = %s\n", query, mysql_error(conn));
			trclib_writeErr (FL,trcBuf);
			ret_flag = -1; // insert 실패  
			continue;
		}
		else
			ret_flag = 1; // insert 성공

	} // end-of-while fetch  
	mysql_free_result(result);

    if( ret_flag == 1)
		logPrint(trcLogId, FL, "flow_5min insert from flow_report_imsi success : %ld\n", time(0));

    return ret_flag;
}

int stmd_LoadStatisticSMS(time_t t_start)
{
    int     i,j;
    char    query[4096] = {0,};
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int     ret_flag;
	char	start_time[32] = {0,}, end_time[32] = {0,};

	/*
    sprintf(query, "DELETE FROM %s WHERE stat_date < '%s'", 
			STM_STATISTIC_5MINUTE_SMS_TBL_NAME, get_delete_time(STMD_1HOUR_OFFSET*7));
			*/
    sprintf(query, "DELETE FROM %s WHERE stat_date < '%s'", 
			STM_STATISTIC_5MINUTE_SMS_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_MIN]));
    if (stmd_mysql_query (query) < 0) {
		sprintf(trcBuf, "sms 5minute delete fail :query:%s, err = %s\n",query, mysql_error(conn));
		trclib_writeErr (FL,trcBuf);
		return -1;
    }

    sprintf(query, "DELETE FROM %s WHERE deliv_time < '%s'", 
			SMS_HISTORY, get_delete_time(STMD_1HOUR_OFFSET*30));
    if (stmd_mysql_query (query) < 0) {
		sprintf(trcBuf, "sms 5minute delete fail :query:%s, err = %s\n",query, mysql_error(conn));
		trclib_writeErr (FL,trcBuf);
		return -1;
    }

	strcpy(start_time, get_insert_time3());
	strcpy(end_time, get_insert_time());

	for(i = 0;  i < 2; i++ )
	{
		for( j = 0; j < g_stSmsc[i].smscCnt; j++ )
		{
			// SMSC 5min default insert 
			if( i == 0 )
			{
				sprintf(query, "Insert into sms_5minute_statistics values "
						" ( '%s', '%s', 0, 0, 0, 0, 0, 0, 0, 1, '%s', '%s' ) ",
						"SCMA", g_stSmsc[i].stItem[j].ip, get_insert_time(), get_insert_week());

				if (stmd_mysql_query (query) < 0) {
					sprintf(trcBuf, "query fail: query:%s, err = %s\n", query, mysql_error(conn));
					trclib_writeErr (FL,trcBuf);
					return -1;
				}
			}
			else
			{
				sprintf(query, "Insert into sms_5minute_statistics values "
						" ( '%s', '%s', 0, 0, 0, 0, 0, 0, 0, 1, '%s', '%s' ) ",
						"SCMB", g_stSmsc[i].stItem[j].ip, get_insert_time(), get_insert_week());

				if (stmd_mysql_query (query) < 0) {
					sprintf(trcBuf, "query fail: query:%s, err = %s\n", query, mysql_error(conn));
					trclib_writeErr (FL,trcBuf);
					return -1;
				}
			}
		}
	}


	// sms Request First Insert
    memset(query, 0x00, sizeof(query));
	/** sms_history table  변경, req ,succ, fail별 카운트  칼럼 추가 
    sprintf(query, "SELECT aaaa.system_name, IFNULL(aaaa.smsc_ip,' '), IFNULL(aaaa.req,0), "
					" IFNULL(aaaa.succ,0), IFNULL((aaaa.req-aaaa.succ),0), "
					" IFNULL(aaaa.smsc_err,0), IFNULL(smpp_err,0), IFNULL(svr_err,0), "
					" IFNULL(aaaa.req-aaaa.succ-aaaa.smsc_err-smpp_err-svr_err,0) "
					" FROM "
					" (SELECT aaa.system_name system_name, aaa.smsc_ip smsc_ip, aaa.req req, "
					" aaa.succ succ, aaa.smsc_err smsc_err, smpp_err "
			 		" FROM "
			 		" (SELECT aa.system_name system_name, aa.smsc_ip smsc_ip, aa.req req, "
					" aa.succ succ, bb.smsc_err smsc_err "
			  		" FROM "
			  		" (SELECT a.system_name system_name, a.smsc_ip smsc_ip, a.req req, b.succ succ "
			   		" FROM  "
			   		" (SELECT system_name, smsc_ip, COUNT(*) req FROM sms_history "
					" WHERE deliv_time >= '%s' AND deliv_time < '%s' "
					" GROUP BY system_name, smsc_ip) a " 
			   		" LEFT JOIN "
			   		" (SELECT system_name, smsc_ip, COUNT(*) succ FROM sms_history "
					" WHERE deliv_time >= '%s' AND deliv_time < '%s' "
					" AND deliv_sts = 0 GROUP BY system_name, smsc_ip) b  "
			   		" ON a.system_name = b.system_name "
			   		" AND a.smsc_ip = b.smsc_ip "
			  		" ) aa LEFT JOIN "
			  		" (SELECT system_name, smsc_ip, COUNT(*) smsc_err  FROM sms_history "
					" WHERE deliv_time >= '%s' AND deliv_time < '%s' "
			   		" AND deliv_sts >= 1 AND deliv_sts <= 12 "
			   		" GROUP BY system_name, smsc_ip) bb "
			  		" ON aa.system_name = bb.system_name AND aa.smsc_ip = bb.smsc_ip) aaa "
			 		" LEFT JOIN "
			 		" (SELECT system_name, smsc_ip, COUNT(*) smpp_err  FROM sms_history WHERE "
					" deliv_time >= '%s' AND deliv_time < '%s' "
			  		" AND deliv_sts = 100 "
			  		" GROUP BY system_name, smsc_ip) bbb "
			 		" ON aaa.system_name = bbb.system_name AND aaa.smsc_ip = bbb.smsc_ip "
			 		" ) aaaa "
			 		" LEFT JOIN "
			 		" (SELECT system_name, smsc_ip, COUNT(*) svr_err  FROM sms_history "
					" WHERE deliv_time >= '%s' AND deliv_time < '%s' "
			  		" AND deliv_sts = 200 "
			  		" GROUP BY system_name, smsc_ip) bbbb "
			 		" ON aaaa.system_name = bbbb.system_name AND aaaa.smsc_ip = bbbb.smsc_ip ", 
					start_time, end_time, start_time, end_time, start_time, end_time, 
					start_time, end_time, start_time, end_time
		   );
	*/
	sprintf(query, "SELECT system_name, smsc_ip, SUM(req), SUM(succ), SUM(req)-SUM(succ), "
				" SUM(smpp_err), SUM(svr_err), SUM(SMSC_ERR), SUM(etc_err) "
				" FROM sms_history "
				" WHERE deliv_time >= '%s' AND deliv_time < '%s' "
				" GROUP BY system_name, smsc_ip ", start_time, end_time);

	if (stmd_mysql_query (query) < 0) {
		sprintf(trcBuf, "select fail Remote : err = %s\n", mysql_error(conn));
		trclib_writeErr (FL,trcBuf);
        return -2;
    }

    result = mysql_store_result(conn);
    while( ( row = mysql_fetch_row(result)) != NULL )
    {
#if 0
        memset(query, 0x00, sizeof(query));
		sprintf(query, "INSERT INTO %s VALUES ( "
						" '%s', " 	// system_name
						" '%s', "  	// smsc_ip
						"  %s, " 	// req 		count
						"  %s, " 	// succ 	count
						"  %s, " 	// fail 	count
						"  %s, " 	// smpp_err	count
						"  %s, " 	// svr_err	count
						"  %s, " 	// smsc_err	count
						"  %s, " 	// etc_err	count
						"  1, "		// stat_cnt
						" '%s', " 	// stat_date
						" '%s' )" 	// stat_week 
         				, STM_STATISTIC_5MINUTE_SMS_TBL_NAME
						, row[0], row[1], row[2], row[3], row[4]
						, row[5], row[6], row[7], row[8], get_insert_time(), get_insert_week()); // stat_week
#endif

        memset(query, 0x00, sizeof(query));
		sprintf(query, "UPDATE  %s SET req = %s, succ = %s, fail = %s, "
						" smpp_err = %s , svr_err =  %s, smsc_err = %s, etc_err = %s "
						" where system_name = '%s' and smsc_ip = '%s' and stat_date = '%s' " 
						, STM_STATISTIC_5MINUTE_SMS_TBL_NAME
						, row[2], row[3], row[4], row[5], row[6], row[7], row[8], 
						row[0], row[1], get_insert_time()); // stat_week

		if (stmd_mysql_query (query) < 0) {
			sprintf(trcBuf, "insert fail Local query:%s, err = %s\n", query, mysql_error(conn));
			trclib_writeErr (FL,trcBuf);
			ret_flag = -1; // insert 실패  
			continue;
		}
		else
			ret_flag = 1; // insert 성공

	} // end-of-while fetch  
	mysql_free_result(result);

    if( ret_flag == 1)
		logPrint(trcLogId, FL, "sms_5min insert from sms_history success : %ld\n", time(0));

    return ret_flag;
}

int stmd_LoadStatisticRULEENTRY(time_t t_start)
{
	int     i,j;
    char    query[2048] = {0,};
    char    query_format[4096] = {0,};
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int         ret_flag;
	int			entry_id = 0;
	char		start_time[32] = {0,};

	/*
    sprintf(query, "DELETE FROM %s WHERE stat_date < '%s'", 
			STM_STATISTIC_5MINUTE_RULEENT_TBL_NAME, get_delete_time(STMD_1HOUR_OFFSET*7));
			*/
    sprintf(query, "DELETE FROM %s WHERE stat_date < '%s'", 
			STM_STATISTIC_5MINUTE_RULEENT_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_MIN]));
    if (stmd_mysql_query (query) < 0) {
		sprintf(trcBuf, "rdr_ruleent 5minute delete fail :query:%s, err = %s\n",query, mysql_error(conn));
		trclib_writeErr (FL,trcBuf);
		return -1;
    }

	strcpy(start_time, get_insert_time());
    memset(query, 0x00, sizeof(query));

	// insert all ruleset entry
	// update tr
	// update block
	for (i=0; i < SCE_CNT; i++) {

		for (j=0; j < g_stSCEEntry[i].ruleEntryCnt; j++) {

			sprintf(query, "INSERT INTO %s VALUES ( "
					" '%s', " // sce_ip
					"  %d, "  // service_id = 
					" '%s', " // rule_entry_name
					" 0, " // session
					" 0, " // upstream_volume
					"  0, " // downstream_volume
					"  0, " // block_cnt
					"  0, " // redirect_cnt
					"  1, " // stat_cnt
					" '%s', " // stat_date
					" '%s' )" // stat_week 
					, STM_STATISTIC_5MINUTE_RULEENT_TBL_NAME
					, g_stSCE[i].sce_name // sce_ip --> system name
					, g_stSCEEntry[i].stEntry[j].eId // service_id
					, g_stSCEEntry[i].stEntry[j].eName // service_ name
					, start_time // stat_date
					, get_insert_week() ); // stat_week
			

			if (stmd_mysql_query (query) < 0) {
				sprintf(trcBuf, "insert fail Local query:%s, err = %s\n", query, mysql_error(conn));
				trclib_writeErr (FL,trcBuf);
				ret_flag = -1; // insert 실패  
				continue;
			}
		}
		stmd_msg_receiver();
        keepalivelib_increase();
	}

    memset(query, 0x00, sizeof(query));
    sprintf(query, " SELECT record_source, service_id, SUM(session) sess, SUM(upstream_volume) up, "
					" SUM(downstream_volume) down, stat_date, stat_week " 
					" FROM rdr_tr_5minute_statistics "
    				" WHERE stat_date = '%s' "
					" GROUP BY record_source, service_id " , start_time );

	if( conn == NULL )
		return -1;


	if (stmd_mysql_query (query) < 0) {
		sprintf(trcBuf, "select fail Remote : err = %s\n", mysql_error(conn));
		trclib_writeErr (FL,trcBuf);
        return -2;
    }

    result = mysql_store_result(conn);
    while( ( row = mysql_fetch_row(result)) != NULL )
    {
        memset(query, 0x00, sizeof(query));
        memset(query_format, 0x00, sizeof(query_format));

		entry_id = atoi(row[1]);
//		fprintf (stderr, "row[0]:sysname - %s\n", row[0]);

		sprintf(query, "UPDATE %s SET session = %s , upstream_volume = %s, downstream_volume = %s "
						" WHERE record_source = '%s' and rule_ent_id = %s and stat_date = '%s' "
         				, STM_STATISTIC_5MINUTE_RULEENT_TBL_NAME
						, row[2] // session
						, row[3] // upstream_volume
						, row[4] // downstream_volume
						, row[0] // record_source
						, row[1] // service_id
						, start_time );

		if (stmd_mysql_query (query) < 0) {
			sprintf(trcBuf, "insert fail Local query:%s, err = %s\n", query, mysql_error(conn));
			trclib_writeErr (FL,trcBuf);
			continue;
		}

		stmd_msg_receiver();
		keepalivelib_increase();

//		logPrint(trcLogId, FL, "UPDATE ENTRY QUERY : %s\n", query);

	} // end-of-while fetch  
	mysql_free_result(result);

	//3 block
	// rdr_block에서 데이터를 가져와서  rdr_ruleent 으로 옮길 것이다. 
	sprintf(query, " SELECT RECORD_SOURCE, SERVICE_ID, IFNULL(sum(BLOCK_RDR_CNT),0) blk, 0 red,stat_date,stat_week "
					" FROM %s "
					" WHERE stat_date = '%s' and subscriber_id <> '' "
					" GROUP BY RECORD_SOURCE, SERVICE_ID "
					, STM_STATISTIC_5MINUTE_BLOCK_TBL_NAME, start_time);

	if (stmd_mysql_query (query) < 0) {
		sprintf(trcBuf, "select fail :query:%s, err = %s\n",query, mysql_error(conn));
		trclib_writeLogErr (FL,trcBuf);
		return -2;
	}

	result = mysql_store_result(conn);
	while( ( row = mysql_fetch_row(result)) != NULL )
	{
		// tr 데이터가 있으므로 block 데이터는 update
		sprintf(query, "UPDATE %s SET block_cnt = %s , redirect_cnt = 0 "
						" WHERE record_source = '%s' and rule_ent_id = %s and stat_date = '%s' "
         				, STM_STATISTIC_5MINUTE_RULEENT_TBL_NAME
						, row[2] // block cnt
						, row[0] // record_source
						, row[1] // service_id
						, start_time );

		if (stmd_mysql_query (query) < 0) {
			sprintf(trcBuf, "update fail Local : query:%s, err = %s\n", query, mysql_error(conn));
			trclib_writeLogErr (FL,trcBuf);
			ret_flag = -1; // update 실패  
			continue;
		}

		stmd_msg_receiver();
		keepalivelib_increase();

	} // end-of-while
	mysql_free_result(result);

	return 0;
}

int stmd_LoadStatisticBLOCK(time_t t_start)
{
    int     i,j,index;
    char    query[2048] = {0,};
    char    query_format[4096] = {0,};
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int         ret_flag;
	int			rule_id = 0;

	/*
    sprintf(query, "DELETE FROM %s WHERE stat_date < '%s'", \
			STM_STATISTIC_5MINUTE_BLOCK_TBL_NAME, get_delete_time(STMD_1HOUR_OFFSET*7));
			*/
    sprintf(query, "DELETE FROM %s WHERE stat_date < '%s'", \
			STM_STATISTIC_5MINUTE_BLOCK_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_MIN]));
    if (stmd_mysql_query (query) < 0) {
		sprintf(trcBuf, "rdr_block 5minute delete fail :query:%s, err = %s\n",query, mysql_error(conn));
		trclib_writeLogErr (FL,trcBuf);
		return -1;
    }
#if 0 
	//20110210 by dcham, STMD Hang Up 문제로 RDRANA로 이동
	/* RPT_BLOCK delete 추가 05.25 */
    sprintf(query, "DELETE FROM %s WHERE end_time < %ld", \
			RPT_BLOCK, (t_start) - STMD_1HOUR_OFFSET*7);

	if(conn_Rm == NULL )
	{
		sprintf(trcBuf, "RPT_BLOCK conn_Rm is NULL\n"); // 2010. 12. 22
		trclib_writeErr (FL,trcBuf);				// 2010. 12. 22
		return -1;
	}
	else
	{
		sprintf(trcBuf, "RPT_BLOCK conn_Rm is NOT NULL\n"); // 2010. 12. 22
		trclib_writeErr (FL,trcBuf);				// 2010. 12. 22
	}

	logPrint(trcLogId, FL, "RPT_BLOCK DEL QUERY : %s\n", query); // 2010. 12. 22

	if (stmd_mysql_query (query) < 0) {
		sprintf(trcBuf, "RPT_BLOCK table delete fail :query:%s, err = %s\n",query, mysql_error(conn_Rm));
		trclib_writeErr (FL,trcBuf);
	}
#endif

	memset(query, 0x00, sizeof(query));

    snprintf(query, sizeof(query), SELECT_RPT_BLOCK, RPT_BLOCK,t_start-300,t_start);

	if (stmd_mysql_query_with_conn (conn_Rm, query) < 0) {
		sprintf(trcBuf, "select fail Remote : err = %s\n", mysql_error(conn_Rm));
		trclib_writeLogErr (FL,trcBuf);
        return -2;
    }

    result = mysql_store_result(conn_Rm);
    while( ( row = mysql_fetch_row(result)) != NULL )
    {
		stmd_msg_receiver();
        keepalivelib_increase();

        memset(query, 0x00, sizeof(query));
        memset(query_format, 0x00, sizeof(query_format));

		rule_id = atoi(row[2]);

		for( i = 0; i < SCE_CNT; i++ )
		{
			if(!strcmp(row[0], g_stSCERule[i].sce_ip))
			{
				g_stSCERule[i].stRule[rule_id].eFlag = 1;
				break;
			}
		}

        sprintf(query, "INSERT INTO %s VALUES ", STM_STATISTIC_5MINUTE_BLOCK_TBL_NAME);
		//rec_src,subsid,pkgid,serviceid,protoid,initside,blkreason,blkcnt,redirected,statcnt,end,statdate,week
		sprintf(query_format, "('%s','%s','%s','%s','%s','%s','%s','%s','%s',1,'%s','%s')",
		                        g_stSCE[i].sce_name,row[1],row[2],row[3],row[4],row[5],row[6],row[7],row[8],
//		                        row[0],row[1],row[2],row[3],row[4],row[5],row[6],row[7],row[8],
								get_insert_time(),get_insert_week());
        sprintf(query, "%s %s", query, query_format);

		if (stmd_mysql_query (query) < 0) {
			sprintf(trcBuf, "insert fail Local : err = %s\n", mysql_error(conn));
			trclib_writeLogErr (FL,trcBuf);
            ret_flag = -1; // insert 실패  
            continue;
        }
        else
            ret_flag = 1; // insert 성공
    }
	mysql_free_result(result);

// not exist Rule check -> insert 0
	for( i = 0; i < SCE_CNT; i++ )
	{
		for( j = 0; j < g_ruleItemCnt; j++ ) 
		{
			index = g_ruleIdBuf[j];
			if( g_stSCERule[i].stRule[index].real == 1 && g_stSCERule[i].stRule[index].eFlag == 0 ) 
			{
				sprintf(query, "Insert into rdr_block_5minute_statistics values ( "
								"'%s','', %d,'','','','',0,'',1,'%s','%s')", 
								g_stSCE[i].sce_name,index, get_insert_time(),get_insert_week());

				if (stmd_mysql_query (query) < 0) {
					sprintf(trcBuf, "insert 0 rdr_block query:%s, err = %s\n", query, mysql_error(conn));
					trclib_writeLogErr (FL,trcBuf);
				}
			}
			else if( g_stSCERule[i].stRule[index].real == 1 && g_stSCERule[i].stRule[index].eFlag == 1 ) 
			{
				g_stSCERule[i].stRule[index].eFlag = 0;
			}
		}
	}

    return ret_flag;
}

#if 0
int stmd_LoadStatisticHTTP(time_t t_start)
{
    int     ret, i;
    char    query[2048] = {0,};
    char    query_format[4096] = {0,};
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int         ret_flag;
	int			sce_id = 0;

	if( conn == NULL )
		return -1;

    sprintf(query, "DELETE FROM %s WHERE stat_date < '%s'", \
			STM_STATISTIC_5MINUTE_HTTP_TBL_NAME, get_delete_time(STMD_1HOUR_OFFSET*7));

    if (stmd_mysql_query (query) < 0) {
		sprintf(trcBuf, "rdr_http 5minute delete fail :query:%s, err = %s\n",query, mysql_error(conn));
		trclib_writeErr (FL,trcBuf);
    }

	/* RPT_BLOCK delete 추가 05.25 */
    sprintf(query, "DELETE FROM %s WHERE end_time < %d", \
			RPT_HTTP, t_start - STMD_1HOUR_OFFSET*7);

	if( conn_Rm == NULL )
		return -1;

	if (stmd_mysql_query_with_conn (conn_Rm, query) < 0) {
		sprintf(trcBuf, "RPT_HTTP table delete fail :query:%s, err = %s\n",query, mysql_error(conn_Rm));
		trclib_writeErr (FL,trcBuf);
    }

    memset(query, 0x00, sizeof(query));

    snprintf(query, sizeof(query), SELECT_RPT_HTTP, RPT_HTTP, t_start-300, t_start);

//	logPrint(trcLogId, FL, "QUERY : %s\n", query);

	if (stmd_mysql_query_with_conn (conn_Rm, query) < 0) {
		sprintf(trcBuf, "select fail Remote : err = %s\n", mysql_error(conn_Rm));
		trclib_writeErr (FL,trcBuf);
        return -2;
    }

    result = mysql_store_result(conn_Rm);
    while( ( row = mysql_fetch_row(result)) != NULL )
    {
        memset(query, 0x00, sizeof(query));
        memset(query_format, 0x00, sizeof(query_format));

        sprintf(query, "INSERT INTO %s VALUES ", STM_STATISTIC_5MINUTE_HTTP_TBL_NAME);
		//rec_src,pkgid,serviceid,protoid,init_side,upstream_volume,downstream_volume,user_agent,url,stat_cnt,end,stat_date,stat_week
		sprintf(query_format, "('%s','%s','%s','%s','%s','%s','%s','%s','%s',1,'%s','%s')",
		                        row[0],row[1],row[2],row[3],row[4],row[5],row[6],row[7],row[8],get_insert_time(),get_insert_week());
        sprintf(query, "%s %s", query, query_format);
//		logPrint(trcLogId, FL, "QUERY : %s\n", query);

		if (stmd_mysql_query (query) < 0) {
			sprintf(trcBuf, "insert fail Local : err = %s\n", mysql_error(conn));
			trclib_writeErr (FL,trcBuf);
            ret_flag = -1; // insert 실패  
            continue;
        }
        else
            ret_flag = 1; // insert 성공
    }
	mysql_free_result(result);

    if( ret_flag == 1)
		logPrint(trcLogId, FL, "rdr_http insert success : %d\n", t_start);
    return ret_flag;
}
#endif

int mysqlLiveRmCheck()    /* mysql live check : sjjeon */ 
{ 
	const int _BUFSIZ = 512; 
	char cmd[256]={0,};
	char buf[_BUFSIZ]; 
	FILE *fp=NULL; 
	char *fname = "/DSC/DATA/mysqlstatus";
	int ret = 0;

	memset(buf,0,sizeof(buf)); 

	sprintf(cmd,"rsh -l root %s /etc/init.d/mysql status > %s", SCMA_PRI_IP,fname);

	system(cmd);

	if ((fp = fopen(fname,"r")) == NULL) {                                   
		sprintf(trcBuf,"fopen fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno));
		trclib_writeLogErr (FL, trcBuf);
		return -1;                                                                    
	}    

	while (fgets(buf,_BUFSIZ, fp) != NULL)                                              
	{                                                                                    
		if(strstr(buf,"SUCCESS")) {                                                   
			while(fgets(buf, _BUFSIZ, fp) != NULL) {}                                   
			fclose(fp);                                                                 
			ret = 1;                                                                    
			break;
		}                                                                                
	}                                                                                    
	fclose(fp);

	sprintf(cmd,"rsh -l root %s /etc/init.d/mysql status > %s", SCMB_PRI_IP,fname);

	system(cmd);

	if ((fp = fopen(fname,"r")) == NULL) {                                   
		sprintf(trcBuf,"fopen fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno));
		trclib_writeLogErr (FL, trcBuf);
		return ret;                                                                    
	}    

	while (fgets(buf,_BUFSIZ, fp) != NULL)                                              
	{                                                                                    
		if(strstr(buf,"SUCCESS")) {                                                   
			while(fgets(buf, _BUFSIZ, fp) != NULL) {}                                   
			fclose(fp);                                                                 
			ret += 2;
			break;                                                                    
		}                                                                                
	}                                                                                    
	fclose(fp);

	return ret; 
} 



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
			//printf("mysql running.. OK\n");                                                        
			/* popen buffer clear ......ÆAAICA´UAy Co≫o ¹æAo...*/                        
			while(fgets(buf, _BUFSIZ, ptr) != NULL) {}                                   
			pclose(ptr);                                                                 
			return 0;                                                                    
		}                                                                                
		//printf("%s\n",buf);                                                            
	}                                                                                    
	//printf("mysql not running.. OK\n");                                                    
	pclose(ptr);                                                                         
	return 1;                                                                            
} 


#if 0
StmdMmcHdlrVector   mmcHdlrVector[STMD_MAX_MMC_HANDLER] =
{
	{"stat-load",           stmd_mmc_stat_load},
	{"srch-stat-load",      stmd_mmc_srch_stat_load},
	{"stat-fault",          stmd_mmc_stat_fault},
	{"srch-stat-fault",     stmd_mmc_srch_stat_fault},
	{"stat-ip",             stmd_mmc_stat_tcpip},
	{"srch-stat-ip",        stmd_mmc_srch_stat_tcpip},
	{"stat-if",             stmd_mmc_stat_if},
	{"srch-stat-if",        stmd_mmc_srch_stat_if},
	{"stat-rad",            stmd_mmc_stat_rad},
	{"srch-stat-rad",       stmd_mmc_srch_stat_rad},
	{"stat-svc",            stmd_mmc_stat_txn},
	{"srch-stat-svc",       stmd_mmc_srch_stat_txn},
	{"stat-tcpudp",         stmd_mmc_stat_ttxn},
	{"srch-stat-tcpudp",    stmd_mmc_srch_stat_ttxn},
	{"stat-udr",            stmd_mmc_stat_udr},
	{"srch-stat-udr",       stmd_mmc_srch_stat_udr},
	{"stat-type-udr",       stmd_mmc_stat_type_udr},
	{"srch-stat-type-udr",  stmd_mmc_srch_stat_type_udr},
	{"stat-cdr",            stmd_mmc_stat_cdr},
	{"srch-stat-cdr",       stmd_mmc_srch_stat_cdr},
	{"stat-cdr2",           stmd_mmc_stat_cdr2},
	{"srch-stat-cdr2",      stmd_mmc_srch_stat_cdr2},
	{"add-stat-schd",       stmd_mmc_add_stat_schd},
	{"del-stat-schd",       stmd_mmc_del_stat_schd},
	{"dis-stat-schd",       stmd_mmc_dis_stat_schd},
	{"dis-stat-his",        stmd_mmc_dis_stat_his},
	{"dis-stat-info",       stmd_mmc_dis_stat_info},
	{"canc-exe-cmd",        stmd_mmc_canc_exe_cmd},
	{"dis-stat-nms",        stmd_mmc_dis_stat_nms},
	{"dis-stat-ptime",      stmd_mmc_dis_stat_ptime},
	{"chg-stat-ptime",      stmd_mmc_chg_stat_ptime},
	{"mask-stat-item",      stmd_mmc_mask_stat_item},
	{"umask-stat-item",     stmd_mmc_umask_stat_item}
};
int     numMmcHdlr  = 33;

char    strITEM[STMD_MASK_ITEM_NUM][14] =
{
	STMD_STR_FAULT,
	STMD_STR_LOAD,
	STMD_STR_TRAN,
	STMD_STR_AAA,
	STMD_STR_UAWAP,
	STMD_STR_SVC_TR,
	STMD_STR_RADIUS,
	STMD_STR_SVC_TTR,
	STMD_STR_UDR,
	STMD_STR_TYPE_UDR,
	STMD_STR_CDR,
	STMD_STR_CDR2, 
	STMD_STR_AN_AAA,
	STMD_STR_FAIL_UDR
};
#endif // by jjinri 2009.04.18
