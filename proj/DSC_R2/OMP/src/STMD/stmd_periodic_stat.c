#include "stmd_proto.h"

extern  char    trcBuf[4096];
extern  short 	printTIME [STMD_PERIOD_TYPE_NUM];
extern  int 	trcLogId;

int doPeriodicJob()
{
	time_t      now;
//    	struct  tm  tms;
    	struct  tm  *tms;

    	now = time(0);
//    	localtime_r ((time_t *)&now, &tms);
    	tms = localtime ((time_t *)&now);
    
    	// 매시 print time에 출력한다. // 5분 
    	if (  tms->tm_min >= (int)printTIME[STMD_HOUR]-5 && tms->tm_min < (int)printTIME[STMD_HOUR]) 
		{
        	logPrint(trcLogId, FL, "doPeriodicHourly start:printTIME->%d\n",(int)printTIME[STMD_HOUR]);
        	doPeriodicHourly();
			logPrint(trcLogId, FL, "doPeriodicHourly end:printTIME->%d\n",(int)printTIME[STMD_HOUR]);
    	}

    	// 자정에 print time에 출력한다. 
    	if (tms->tm_hour == 0 )
		{
			if ( (int )printTIME[STMD_HOUR] <= tms->tm_min && tms->tm_min < (int )printTIME[STMD_DAY] ) 
			{	
				logPrint(trcLogId, FL, "doPeriodicDayly start:printDAY->%d\n",(int)printTIME[STMD_DAY]);
        		doPeriodicDaily();
				logPrint(trcLogId, FL, "doPeriodicDayly end:printDAY->%d\n",(int)printTIME[STMD_DAY]);
			}
		}

    	// 월요일 자정 print time에 출력한다.
    	if (tms->tm_wday == 1 && tms->tm_hour == 0) 
		{
			if( (int )printTIME[STMD_DAY] <= tms->tm_min && tms->tm_min < (int )printTIME[STMD_WEEK] ) 
			{
				logPrint(trcLogId, FL, "doPeriodicWeekly start:printWEEK->%d\n",(int)printTIME[STMD_WEEK]);
				doPeriodicWeekly();
				logPrint(trcLogId, FL, "doPeriodicWeekly end:printWEEK->%d\n",(int)printTIME[STMD_WEEK]);
			}
		}

    	// 1일이고, 시간이 자정이고 print time에 출력한다.
    	if (tms->tm_mday == 1 && tms->tm_hour == 0) 
		{
			if ( (int )printTIME[STMD_WEEK] <= tms->tm_min && tms->tm_min < (int )printTIME[STMD_MONTH]) 
			{
				logPrint(trcLogId, FL, "doPeriodicMonthly start:printMONTH->%d\n",(int)printTIME[STMD_MONTH]);
				doPeriodicMonthly();
				logPrint(trcLogId, FL, "doPeriodicMonthly end:printMONTH->%d\n",(int)printTIME[STMD_MONTH]);
			}	
		}
    	return 1;
}

int doPeriodicHourly()
{
    sprintf(trcBuf, "doPeriodicHourly\n");
    trclib_writeLog(FL, trcBuf);
#if 1    
	sprintf(trcBuf, "ENT HOUR START : %ld\n", time(0));
	trclib_writeLogErr(FL, trcBuf);
    doPeriodicHourRuleEnt();
	sprintf(trcBuf, "ENT HOUR END : %ld\n", time(0));
	trclib_writeLogErr(FL, trcBuf);
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

	stmd_msg_receiver();
#endif

	sprintf(trcBuf, "LOAD HOUR START : %ld\n", time(0));
	trclib_writeLogErr(FL, trcBuf);
    doPeriodicHourLoad();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
	sprintf(trcBuf, "LOAD HOUR STOP : %ld\n", time(0));
	trclib_writeLogErr(FL, trcBuf);

	stmd_msg_receiver();
    
	sprintf(trcBuf, "FLT HOUR START : %ld\n", time(0));
	trclib_writeLogErr(FL, trcBuf);
	doPeriodicHourFlt();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
	sprintf(trcBuf, "FLT HOUR STOP : %ld\n", time(0));
	trclib_writeLogErr(FL, trcBuf);

	sprintf(trcBuf, "LINK HOUR START : %ld\n", time(0));
	trclib_writeLogErr(FL, trcBuf);
    doPeriodicHourLink();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
	sprintf(trcBuf, "LINK HOUR STOP : %ld\n", time(0));
	trclib_writeLogErr(FL, trcBuf);


	sprintf(trcBuf, "RULESET HOUR START : %ld\n", time(0));
	trclib_writeLogErr(FL, trcBuf);
    doPeriodicHourRuleSet();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
	sprintf(trcBuf, "RULESET HOUR STOP : %ld\n", time(0));
	trclib_writeLogErr(FL, trcBuf);

	stmd_msg_receiver();
	
	sprintf(trcBuf, "LEG HOUR START : %ld\n", time(0));
	trclib_writeLogErr(FL, trcBuf);
    doPeriodicHourLeg();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
	sprintf(trcBuf, "LEG HOUR STOP : %ld\n", time(0));
	trclib_writeLogErr(FL, trcBuf);

	stmd_msg_receiver();

	sprintf(trcBuf, "LOGON HOUR START : %ld\n", time(0));
	trclib_writeLogErr(FL, trcBuf);
    doPeriodicHourLogon();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
	sprintf(trcBuf, "LOGON HOUR STOP : %ld\n", time(0));
	trclib_writeLogErr(FL, trcBuf);

	stmd_msg_receiver();

	sprintf(trcBuf, "FLOW HOUR START : %ld\n", time(0));
	trclib_writeLogErr(FL, trcBuf);
    doPeriodicHourFlow();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
	sprintf(trcBuf, "FLOW HOUR STOP : %ld\n", time(0));
	trclib_writeLogErr(FL, trcBuf);

	stmd_msg_receiver();

	sprintf(trcBuf, "SMS HOUR START : %ld\n", time(0));
	trclib_writeLogErr(FL, trcBuf);
    doPeriodicHourSms();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
	sprintf(trcBuf, "SMS HOUR STOP : %ld\n", time(0));
	trclib_writeLogErr(FL, trcBuf);

	sprintf(trcBuf, "DEL HOUR START : %ld\n", time(0));
	trclib_writeLogErr(FL, trcBuf);
    doPeriodicHourDelay();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
	sprintf(trcBuf, "DEL HOUR END : %ld\n", time(0));
	trclib_writeLogErr(FL, trcBuf);

#if 0	
	doPeriodicHourRuleCnt();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

	doPeriodicHourRuleThru();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

	doPeriodicHourRuleUser();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
    
	doPeriodicHourAaa();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
    
	doPeriodicHourUawap();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
    
	doPeriodicHourIpaf();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
    
	doPeriodicHourSvcTr();
   	stmd_msg_receiver();	
	keepalivelib_increase(); 
	commlib_microSleep(1000);

    doPeriodicHourRadius();
	stmd_msg_receiver();
	keepalivelib_increase(); 
    commlib_microSleep(1000);
    
	doPeriodicHourSvcTtr();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
    
	doPeriodicHourUdr();
	stmd_msg_receiver();
 	keepalivelib_increase();   
	commlib_microSleep(1000);
    
	doPeriodicHourCdr();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
    
	doPeriodicHourSvcTypeUdr();
	stmd_msg_receiver();
    keepalivelib_increase(); 
    commlib_microSleep(1000);
    
	doPeriodicHourVt();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
    
	doPeriodicHourCdr2();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
    
	doPeriodicHourFailUdr();
	stmd_msg_receiver();
   	keepalivelib_increase(); 
	commlib_microSleep(1000); 
#endif // by jjinri 2009.04.18
    
	send_query_info (STAT_PERIOD_HOUR, \
                get_period_end_time (STMD_HOUR), \
                get_period_end_time (STMD_HOUR));

    return 1;
}

int doPeriodicDaily()
{
    sprintf(trcBuf, "doPeriodicDaily\n");
    trclib_writeLog(FL, trcBuf);

    doPeriodicDayLoad();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
    
	doPeriodicDayFlt();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

    doPeriodicDayLink();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

    doPeriodicDayLeg();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

	stmd_msg_receiver();

    doPeriodicDayLogon();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

	stmd_msg_receiver();

    doPeriodicDayFlow();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

    doPeriodicDayRuleSet();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

	stmd_msg_receiver();

#if 1
    doPeriodicDayRuleEnt();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
#endif

	stmd_msg_receiver();

    doPeriodicDaySms();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

    doPeriodicDayDelay();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

#if 0
    
	doPeriodicDayAaa();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
    
	doPeriodicDayIpaf();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
    
	doPeriodicDayUawap();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
    
	doPeriodicDaySvcTr();
	stmd_msg_receiver();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
    
	doPeriodicDayRadius();
	stmd_msg_receiver();
    keepalivelib_increase(); 
    commlib_microSleep(1000);
    
	doPeriodicDaySvcTtr();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
    
	doPeriodicDayUdr();
	stmd_msg_receiver();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
    
	doPeriodicDayCdr();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
    
	doPeriodicDaySvcTypeUdr();
	stmd_msg_receiver();
    keepalivelib_increase(); 
    commlib_microSleep(1000);
    
	doPeriodicDayVt();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
    
	doPeriodicDayCdr2();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
    
	doPeriodicDayFailUdr();
	stmd_msg_receiver();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
#endif // by jjinri 2009.04.18
    
	return 1;
}

int doPeriodicWeekly()
{
   	sprintf(trcBuf, "doPeriodicWeekly\n");
   	trclib_writeLog(FL, trcBuf);

    doPeriodicWeekLoad();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
    
	doPeriodicWeekFlt();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

    doPeriodicWeekLink();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

    doPeriodicWeekLeg();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

    doPeriodicWeekLogon();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

	stmd_msg_receiver();

    doPeriodicWeekFlow();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

	stmd_msg_receiver();

    doPeriodicWeekRuleSet();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

	stmd_msg_receiver();

    doPeriodicWeekRuleEnt();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

	stmd_msg_receiver();

    doPeriodicWeekSms();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

    doPeriodicWeekDelay();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

#if 0
	doPeriodicWeekAaa();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
    
	doPeriodicWeekIpaf();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
    
	doPeriodicWeekUawap();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
    
	doPeriodicWeekSvcTr();
   	stmd_msg_receiver();	
	keepalivelib_increase(); 
	commlib_microSleep(1000);
    
	doPeriodicWeekRadius();
	stmd_msg_receiver();
   	keepalivelib_increase(); 
   	commlib_microSleep(1000);
    
	doPeriodicWeekSvcTtr();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
    
	doPeriodicWeekUdr();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
    
	doPeriodicWeekCdr();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
    
	doPeriodicWeekSvcTypeUdr();
	stmd_msg_receiver();
   	keepalivelib_increase(); 
   	commlib_microSleep(1000);
    
	doPeriodicWeekVt();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
    
	doPeriodicWeekCdr2();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
    
	doPeriodicWeekFailUdr();
	stmd_msg_receiver();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
#endif // by jjinri 2009.04.18
    
	return 1;
}

int doPeriodicMonthly()
{
   	sprintf(trcBuf, "doPeriodicMonthly\n");
   	trclib_writeLog(FL, trcBuf);

    doPeriodicMonthLoad();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
    
	doPeriodicMonthFlt();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

    doPeriodicMonthLink();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

    doPeriodicMonthLeg();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

    doPeriodicMonthLogon();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

    doPeriodicMonthFlow();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

	stmd_msg_receiver();

	stmd_msg_receiver();

    doPeriodicMonthRuleSet();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

	stmd_msg_receiver();

    doPeriodicMonthRuleEnt();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

	stmd_msg_receiver();

    doPeriodicMonthSms();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

    doPeriodicMonthDelay();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

#if 0
	doPeriodicMonthAaa();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
    
	doPeriodicMonthIpaf();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
    
	doPeriodicMonthUawap();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
    
	doPeriodicMonthSvcTr();
   	stmd_msg_receiver();	
	keepalivelib_increase(); 
	commlib_microSleep(1000);
    
	doPeriodicMonthRadius();
	stmd_msg_receiver();
   	keepalivelib_increase(); 
   	commlib_microSleep(1000);
    
	doPeriodicMonthSvcTtr();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
    
	doPeriodicMonthUdr();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
    
	doPeriodicMonthCdr();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
    
	doPeriodicMonthSvcTypeUdr();
	stmd_msg_receiver();
   	keepalivelib_increase(); 
   	commlib_microSleep(1000);
    
	doPeriodicMonthVt();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
    
	doPeriodicMonthCdr2();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
    
	doPeriodicMonthFailUdr();
	stmd_msg_receiver();
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
#endif // by jjinri 2009.04.18
    
	return 1;
}

