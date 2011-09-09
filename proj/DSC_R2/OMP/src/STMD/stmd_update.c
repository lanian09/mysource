#include "stmd_proto.h"

//extern char updateJOB [MAX_CRONJOB_NUM];
extern  int     trcLogId;

// by jjinri 2009. 04.18 int stmd_exeUpdateHourArea(void)
int stmd_exeUpdateHourArea(void) // modify by jjinri 2009.04.18
{
	int	ret = 0;
#ifdef DELAY
	time_t      cur_time;
	struct tm  *cur_tMS;
#endif

	logPrint(trcLogId, FL, "stmd_exeUpdateHourArea start\n");

	stmd_msg_receiver();

/* 2010.10.11 ret < 0 : return minus; */
	ret = stmd_exeUpdateHourLoad();
	if( ret < 0 )
		return -1;
   	keepalivelib_increase(); 
   	commlib_microSleep(1000); 

	stmd_msg_receiver();
    	
	ret = stmd_exeUpdateHourFlt();
	if( ret < 0 )
		return -2;
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

	ret = stmd_exeUpdateHourLUR();
	if( ret < 0 )
		return -3;
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

	ret = stmd_exeUpdateHourBLOCK();
	if( ret < 0 )
		return -4;
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

	ret = stmd_exeUpdateHourLogOn();	 		/**< LogOn */
	if( ret < 0 )
		return -5;
   	keepalivelib_increase(); 
   	commlib_microSleep(1000); 

	stmd_msg_receiver();

	ret = stmd_exeUpdateHourLeg();
	if( ret < 0 )
		return -6;
   	keepalivelib_increase(); 
   	commlib_microSleep(1000); 

	stmd_msg_receiver();

	ret = stmd_exeUpdateHourFlow();	 		/**2010.08.23  */
	if( ret < 0 )
		return -7;
   	keepalivelib_increase(); 
   	commlib_microSleep(1000); 

	stmd_msg_receiver();

	ret = stmd_exeUpdateHourRULESET();
	if( ret < 0 )
		return -8;
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

	stmd_msg_receiver();

	ret = stmd_exeUpdateHourRULEENT();
	if( ret < 0 )
		return -9;

   	keepalivelib_increase(); 
	commlib_microSleep(1000);

	stmd_msg_receiver();

	ret = stmd_exeUpdateHourSms();
	if( ret < 0 )
		return -10;
   	keepalivelib_increase(); 
   	commlib_microSleep(1000); 

#ifdef DELAY
	cur_time = (time(0)/STAT_OFFSET_UNIT)*STAT_OFFSET_UNIT;
	cur_tMS = (struct tm*)localtime((time_t*)&cur_time);

	if( (cur_tMS->tm_min % STAT_MIN_SEC) == 0 )
	{
		ret = stmd_exeUpdateHourDelay();
		if( ret < 0 )
			return -12;
	}
#else
	ret = stmd_exeUpdateHourDelay();
	if( ret < 0 )
		return -11;
#endif
   	keepalivelib_increase(); 
   	commlib_microSleep(1000); 
	/** 06.28 DELAY UPDATE */

	stmd_msg_receiver();

#if 0
	// -- add by jjinri 2009.04.21
	stmd_exeUpdateHourTR(t_start);
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

	// -- add by jjinri 2009.04.22
	stmd_exeUpdateHourPUR(t_start);
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

	// add by jjinri 2009.04.22 --
	stmd_exeUpdateHourMALUR(t_start);
   	keepalivelib_increase(); 
	commlib_microSleep(1000);


	// -- add by jjinri 2009.05.01
	stmd_exeUpdateHourHTTP(t_start);
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

	// rdr_rulecnt jjinri 2009.05.08
	stmd_exeUpdateHourRULE(t_start);
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
#endif

	logPrint(trcLogId, FL, "stmd_exeUpdateHourArea end\n");	
	return 1;
}

// by jjinri 2009.04.18 int stmd_exeUpdateDayArea(void)
int stmd_exeUpdateDayArea(void) // modify by jjinri 2009.04.18
{
	int ret = 0;
#ifdef DELAY
////	time_t      cur_time;
////	struct tm  *cur_tMS;
#endif

	logPrint(trcLogId, FL, "stmd_exeUpdateDayArea start\n");

	ret = stmd_exeUpdateDayLoad();
	if( ret < 0 )
		return -1;
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
	
	ret = stmd_exeUpdateDayFlt();
	if( ret < 0 )
		return -2;
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

	ret = stmd_exeUpdateDayLUR();
	if( ret < 0 )
		return -3;
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

	ret = stmd_exeUpdateDayBLOCK();
	if( ret < 0 )
		return -4;
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

	ret = stmd_exeUpdateDayLeg();
	if( ret < 0 )
		return -5;
   	keepalivelib_increase(); 
   	commlib_microSleep(1000); 

	ret = stmd_exeUpdateDayLogOn();
	if( ret < 0 )
		return -6;
   	keepalivelib_increase(); 
   	commlib_microSleep(1000); 

	stmd_msg_receiver();

	ret = stmd_exeUpdateDayFlow();	 		/**2010.08.23  */
	if( ret < 0 )
		return -7;
   	keepalivelib_increase(); 
   	commlib_microSleep(1000); 

	stmd_msg_receiver();

	// rdr_ruleset jjinri 2009.06.01
	ret = stmd_exeUpdateDayRULESET();
	if( ret < 0 )
		return -8;
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

	stmd_msg_receiver();

	ret = stmd_exeUpdateDayRULEENT();
	if( ret < 0 )
		return -9;
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
	
	stmd_msg_receiver();

	ret = stmd_exeUpdateDaySms();
	if( ret < 0 )
		return -10;
   	keepalivelib_increase(); 
   	commlib_microSleep(1000); 

#ifdef DELAY
	/*
	cur_time = (time(0)/STAT_OFFSET_UNIT)*STAT_OFFSET_UNIT;
	cur_tMS = (struct tm*)localtime((time_t*)&cur_time);

	if( (cur_tMS->tm_min % STAT_MIN_SEC) == 0 )
	*/
		ret = stmd_exeUpdateDayDelay();
		if( ret < 0 )
			return -12;
#else
	ret = stmd_exeUpdateDayDelay();
	if( ret < 0 )
		return -11;
#endif

   	keepalivelib_increase(); 
   	commlib_microSleep(1000); 

	stmd_msg_receiver();

#if 0
	// add by jjinri 2009.04.18 --
	stmd_exeUpdateDayTR(t_start);
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

	// add by jjinri 2009.04.21 --
	stmd_exeUpdateDayPUR(t_start);
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
	
	// -- add by jjinri 2009.04.22
	stmd_exeUpdateDayMALUR(t_start);
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

	// add by jjinri 2009.04.28 --
	stmd_exeUpdateDayHTTP(t_start);
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

	// rdr_rulecnt jjinri 2009.05.08
	stmd_exeUpdateDayRULE(t_start);
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
#endif

	logPrint(trcLogId, FL, "stmd_exeUpdateDayArea end\n");	
	
	return 1;

}

// by jjinri 2009.04.18 int stmd_exeUpdateWeekArea(void)
int stmd_exeUpdateWeekArea(void) // modify by jjinri 2009.04.18
{
	int		ret = 0;

#ifdef DELAY
////	time_t      cur_time;
////	struct tm  *cur_tMS;
#endif

	logPrint(trcLogId, FL, "stmd_exeUpdateWeekArea start\n");

   	ret = stmd_exeUpdateWeekLoad();
	if( ret < 0 )
		return -1;
  	keepalivelib_increase();  
	commlib_microSleep(1000); 

	stmd_msg_receiver();

   	ret = stmd_exeUpdateWeekFlt();
	if( ret < 0 )
		return -2;
   	keepalivelib_increase();	
	commlib_microSleep(1000);

	stmd_msg_receiver();
    
   	ret = stmd_exeUpdateWeekLUR();
	if( ret < 0 )
		return -3;
  	keepalivelib_increase();  
	commlib_microSleep(1000); 

	stmd_msg_receiver();

	ret = stmd_exeUpdateWeekBLOCK();
	if( ret < 0 )
		return -4;
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

	stmd_msg_receiver();
	
   	ret = stmd_exeUpdateWeekLeg();
	if( ret < 0 )
		return -5;
  	keepalivelib_increase();  
	commlib_microSleep(1000); 
	
	stmd_msg_receiver();

	ret = stmd_exeUpdateWeekLogOn();
	if( ret < 0 )
		return -6;
   	keepalivelib_increase(); 
   	commlib_microSleep(1000); 

	stmd_msg_receiver();

	ret = stmd_exeUpdateWeekFlow();	 		/**2010.08.23  */
	if( ret < 0 )
		return -7;
   	keepalivelib_increase(); 
   	commlib_microSleep(1000); 

	stmd_msg_receiver();

	// rdr_ruleset jjinri 2009.06.01
	ret = stmd_exeUpdateWeekRULESET();
	if( ret < 0 )
		return -8;
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

	stmd_msg_receiver();

	ret = stmd_exeUpdateWeekRULEENT();
	if( ret < 0 )
		return -9;
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

	stmd_msg_receiver();

	ret = stmd_exeUpdateWeekSms();
	if( ret < 0 )
		return -10;
   	keepalivelib_increase(); 
   	commlib_microSleep(1000); 

#ifdef DELAY
	/*
	cur_time = (time(0)/STAT_OFFSET_UNIT)*STAT_OFFSET_UNIT;
	cur_tMS = (struct tm*)localtime((time_t*)&cur_time);

	if( (cur_tMS->tm_min % STAT_MIN_SEC) == 0 )
	*/
		ret = stmd_exeUpdateWeekDelay();
		if( ret < 0 )
			return -12;
#else
	ret = stmd_exeUpdateWeekDelay();
	if( ret < 0 )
		return -11;
#endif

   	keepalivelib_increase(); 
   	commlib_microSleep(1000); 

	stmd_msg_receiver();

#if 0
	// add by jjinri 2009.04.18 --
   	stmd_exeUpdateWeekTR(t_start);
  	keepalivelib_increase();  
	commlib_microSleep(1000); 

	// add by jjinri 2009.04.21 --
	stmd_exeUpdateWeekPUR(t_start);
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

	// add by jjinri 2009.04.21 --
	stmd_exeUpdateWeekMALUR(t_start);
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

	// add by jjinri 2009.04.28 --
	stmd_exeUpdateWeekHTTP(t_start);
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

	// rdr_rulecnt jjinri 2009.05.08
	stmd_exeUpdateWeekRULE(t_start);
   	keepalivelib_increase(); 
	commlib_microSleep(1000);
#endif

	logPrint(trcLogId, FL, "stmd_exeUpdateWeekArea end\n"); 
	return 1;

}

// by jjinri 2009.04.18 int stmd_exeUpdateMonthArea()
int stmd_exeUpdateMonthArea(void)
{
	int		ret = 0;
#ifdef DELAY
////	time_t      cur_time;
////	struct tm  *cur_tMS;
#endif

	logPrint(trcLogId, FL, "stmd_exeUpdateMonthArea start\n");

   	ret = stmd_exeUpdateMonthLoad();
	if( ret < 0 )
		return -1;
  	keepalivelib_increase();  
	commlib_microSleep(1000); 

	stmd_msg_receiver();

   	ret = stmd_exeUpdateMonthFlt();
	if( ret < 0 )
		return -2;
   	keepalivelib_increase();	
	commlib_microSleep(1000);

	stmd_msg_receiver();
    
   	ret = stmd_exeUpdateMonthLUR();
	if( ret < 0 )
		return -3;
  	keepalivelib_increase();  
	commlib_microSleep(1000); 

	stmd_msg_receiver();

	ret = stmd_exeUpdateMonthBLOCK();
	if( ret < 0 )
		return -4;
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

	stmd_msg_receiver();
	
   	ret = stmd_exeUpdateMonthLeg();
	if( ret < 0 )
		return -5;
  	keepalivelib_increase();  
	commlib_microSleep(1000); 

	stmd_msg_receiver();

	ret = stmd_exeUpdateMonthLogOn();
	if( ret < 0 )
		return -6;
   	keepalivelib_increase(); 
   	commlib_microSleep(1000); 

	stmd_msg_receiver();

	ret = stmd_exeUpdateMonthFlow();	 		/**2010.08.23  */
	if( ret < 0 )
		return -7;
   	keepalivelib_increase(); 
   	commlib_microSleep(1000); 

	stmd_msg_receiver();

	ret = stmd_exeUpdateMonthRULESET();
	if( ret < 0 )
		return -8;
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

	stmd_msg_receiver();

	ret = stmd_exeUpdateMonthRULEENT();
	if( ret < 0 )
		return -9;
   	keepalivelib_increase(); 
	commlib_microSleep(1000);

	stmd_msg_receiver();

	ret = stmd_exeUpdateMonthSms();
	if( ret < 0 )
		return -10;
   	keepalivelib_increase(); 
   	commlib_microSleep(1000); 

	stmd_msg_receiver();

#ifdef DELAY
	/*
	cur_time = (time(0)/STAT_OFFSET_UNIT)*STAT_OFFSET_UNIT;
	cur_tMS = (struct tm*)localtime((time_t*)&cur_time);

	if( (cur_tMS->tm_min % STAT_MIN_SEC) == 0 )
	*/
		ret = stmd_exeUpdateMonthDelay();
		if( ret < 0 )
			return -12;
#else
	ret = stmd_exeUpdateMonthDelay();
	if( ret < 0 )
		return -11;
#endif
   	keepalivelib_increase(); 
   	commlib_microSleep(1000); 

	stmd_msg_receiver();

	logPrint(trcLogId, FL, "stmd_exeUpdateMonthArea end\n");
    	
	return 1;

}
