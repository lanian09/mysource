
#include "stmd_proto.h"

extern  int     sysCnt;
extern  int     nmsifQid;
extern  int     stmdQid;
extern  STM_CommStatisticInfo   StatisticSystemInfo[SYSCONF_MAX_ASSO_SYS_NUM];
extern  char    trcBuf[4096], trcTmp[1024];
extern  OnDemandList    onDEMAND[MAX_ONDEMAND_NUM];
extern  CronList    cronJOB[MAX_CRONJOB_NUM];
extern  short   printTIME [STMD_PERIOD_TYPE_NUM];
extern  MYSQL   sql, *conn;
extern  MYSQL   sql_Rm, *conn_Rm;

int stmd_mmcHdlrVector_qsortCmp (const void *a, const void *b)
{
    return (strcasecmp (((StmdMmcHdlrVector*)a)->cmdName, ((StmdMmcHdlrVector*)b)->cmdName));
}

int stmd_mmcHdlrVector_bsrchCmp (const void *a, const void *b)
{
    return (strcasecmp ((char*)a, ((StmdMmcHdlrVector*)b)->cmdName));
}

int get_system_information(char *sysName, char *groupName, char *sysType)
{
    int     i;

    for( i = 0; i < sysCnt ; i++) {
        if (!strcasecmp(StatisticSystemInfo[i].sysName, sysName)) {
            strcpy(groupName, StatisticSystemInfo[i].sysGroup);
            strcpy(sysType, StatisticSystemInfo[i].sysType);
            break;
        }
    }
    if ( i == sysCnt)
        return -1;
    else
        return 1;
}

/**
    ondemand, cron, periodic 통계 출력의 PERIOD를 출력하기 위해 기능 추가
**/
static  char    currentStamp[32];
char *get_current_time() 
{
    time_t      now;
    struct tm   *pLocalTime;

    now = time(0);
    now = (now/STAT_OFFSET_UNIT) *STAT_OFFSET_UNIT; // 5분단위로 시간을 관리하기 위해서

    if ((pLocalTime = (struct tm*)localtime((time_t*)&now)) == NULL) {
        strcpy(currentStamp, "");
    } else {
        strftime (currentStamp, 32, "%Y-%m-%d %H:%M", pLocalTime);
    }
    return (currentStamp);
}

static  char    periodstarttimeStamp[32];
char *get_period_start_time(int time_type)
{
    time_t      now;
    struct tm   *pLocalTime;
    now = time(0);
    now = now- (printTIME[time_type]*60) - STAT_OFFSET_UNIT;

    if ((pLocalTime = (struct tm*)localtime((time_t*)&now)) == NULL) {
        strcpy(periodstarttimeStamp, "");
    } else {
        switch (time_type) {
            case    STMD_HOUR:
                strftime (periodstarttimeStamp, 32, "%Y-%m-%d %H:00", pLocalTime);
                break;
            case    STMD_DAY:
                strftime (periodstarttimeStamp, 32, "%Y-%m-%d 00:00", pLocalTime);
                break;
            case    STMD_WEEK:
                //월 요일에 주 통계를 저장한다
                now = time(0);
                now = now - (7 * STMD_1HOUR_OFFSET);// 1주일 전의 시간을 구한다
                pLocalTime = (struct tm*)localtime((time_t*)&now);
                strftime (periodstarttimeStamp, 32, "%Y-%m-%d 00:00", pLocalTime);
                break;
            case    STMD_MONTH:
                strftime (periodstarttimeStamp, 32, "%Y-%m-01 00:00", pLocalTime);
                break;
        }
    }
    return (periodstarttimeStamp);
}

static  char    periodendtimeStamp[32];
char *get_period_end_time(int time_type)
{
    time_t      now;
    struct tm   *pLocalTime;

    now = time(0);

    if ((pLocalTime = (struct tm*)localtime((time_t*)&now)) == NULL) {
        strcpy(periodendtimeStamp, "");
    } else {
        switch (time_type) {
            case    STMD_HOUR:
                strftime (periodendtimeStamp, 32, "%Y-%m-%d %H:00", pLocalTime);
                break;
            case    STMD_DAY:
                strftime (periodendtimeStamp, 32, "%Y-%m-%d 00:00", pLocalTime);
                break;
            case    STMD_WEEK:
                // 파일에 저장할때 일주일전 ~ 현재 까지 의 시간을 구하기 위함중
                // 현재 까지의 시간을 구한다
                strftime (periodendtimeStamp, 32, "%Y-%m-%d 00:00", pLocalTime);
                break;
            case    STMD_MONTH:
                strftime (periodendtimeStamp, 32, "%Y-%m-01 00:00", pLocalTime);
                break;
        }
    }
    return (periodendtimeStamp);
}
/** End ondemand, cron, periodic 통계 출력의 PERIOD를 출력하기 위해 기능 추가End **/

static  char    periodendtimeStamp2[32];
char *get_period_end_time2(int time_type)
{
    int         wday;
    time_t      now;
    struct tm   *pLocalTime;

    if(time_type== STMD_HOUR)
    {
        now = time(0)+ 60*60;
    }
    else if(time_type== STMD_DAY)
    {
        now = time(0)+ (60*60*24);
    }
    else if(time_type== STMD_WEEK)
    {
        now = time(0);
        //now = now-STAT_OFFSET_UNIT;
    }
    else if(time_type== STMD_MONTH)
    {
        //jean now = time(0)+ (60*60*24*30);
        now = time(0)+ (60*60*24*31);
    }

    if ((pLocalTime = (struct tm*)localtime((time_t*)&now)) == NULL) 
    {
        strcpy(periodendtimeStamp2, "");
    } 
    else 
    {
        switch (time_type) {
            case    STMD_HOUR:
                strftime (periodendtimeStamp2, 32, "%Y-%m-%d %H:00", pLocalTime);
                break;
            case    STMD_DAY:
                strftime (periodendtimeStamp2, 32, "%Y-%m-%d 00:00", pLocalTime);
                break;
            case    STMD_WEEK:
                // 파일에 저장할때 일주일전 ~ 현재 까지 의 시간을 구하기 위함중
                // 현재 까지의 시간을 구한다
                if(!pLocalTime->tm_wday)
                    wday = 7;
                else
                    wday = pLocalTime->tm_wday;

                now = now - (wday * STMD_1HOUR_OFFSET) + STMD_1HOUR_OFFSET + STMD_1HOUR_WEEK_OFFSET;
                pLocalTime = (struct tm*)localtime((time_t*)&now);
                strftime (periodendtimeStamp2, 32, "%Y-%m-%d 00:00:00", pLocalTime);
                break;
            case    STMD_MONTH:
                strftime (periodendtimeStamp2, 32, "%Y-%m-01 00:00", pLocalTime);
                break;
        }
    }
    return (periodendtimeStamp2);
}
/** End ondemand, cron, periodic 통계 출력의 PERIOD를 출력하기 위해 기능 추가End **/

static  char    timeStamp[32];
char *get_delete_time(int del_offset) 
{
    time_t      now;
    struct tm   *pLocalTime;

    now = time(0);
    
    now = now - del_offset - STAT_OFFSET_UNIT; /* 지울 시간 만큼 뺀다 */

    if ((pLocalTime = (struct tm*)localtime((time_t*)&now)) == NULL) {
        strcpy(timeStamp, "");
    } else {
        strftime (timeStamp, 32, "%Y-%m-%d %H:%M", pLocalTime);
    }
    return (timeStamp);
}

static  char    inserttimeStamp[32];
char *get_insert_time4(time_t t_start)
{
    time_t      now;
    struct tm   *pLocalTime;

    now = t_start;
    
    // 어떤 통계든 시작점은 stat_date에 간주한다.
    //now = ((now-STAT_OFFSET_UNIT)/STAT_OFFSET_UNIT) *STAT_OFFSET_UNIT; // 5분단위로 시간을 관리하기 위해서

    if ((pLocalTime = (struct tm*)localtime((time_t*)&now)) == NULL) {
        strcpy(inserttimeStamp, "");
    } else {
        strftime (inserttimeStamp, 32, "%Y-%m-%d %H:%M:00", pLocalTime);
    }
    return (inserttimeStamp);
}

static  char    inserttimeStamp[32];
char *get_insert_time()
{
    time_t      now;
    struct tm   *pLocalTime;

    now = (time(0)/STAT_OFFSET_UNIT)*STAT_OFFSET_UNIT;
    
    // 어떤 통계든 시작점은 stat_date에 간주한다.
    //now = ((now-STAT_OFFSET_UNIT)/STAT_OFFSET_UNIT) *STAT_OFFSET_UNIT; // 5분단위로 시간을 관리하기 위해서

    if ((pLocalTime = (struct tm*)localtime((time_t*)&now)) == NULL) {
        strcpy(inserttimeStamp, "");
    } else {
        strftime (inserttimeStamp, 32, "%Y-%m-%d %H:%M", pLocalTime);
    }
    return (inserttimeStamp);
}

static  char    inserttimeStamp3[32];
char *get_insert_time3()
{
    time_t      now;
    struct tm   *pLocalTime;

    now = time(0);
    
    // 어떤 통계든 시작점은 stat_date에 간주한다.
    now = ((now-STAT_OFFSET_UNIT)/STAT_OFFSET_UNIT) *STAT_OFFSET_UNIT; // 5분단위로 시간을 관리하기 위해서

    if ((pLocalTime = (struct tm*)localtime((time_t*)&now)) == NULL) {
        strcpy(inserttimeStamp3, "");
    } else {
        strftime (inserttimeStamp3, 32, "%Y-%m-%d %H:%M", pLocalTime);
    }
    return (inserttimeStamp3);
}

static  char    inserttimeStamp2[32];
char *get_insert_time2(int time_gap)
{
    time_t      now;
    struct tm   *pLocalTime;
    int         time_offset = time_gap * 60;

    now = time(0);
    // 어떤 통계든 시작점은 stat_date에 간주한다.
    now = ((now-time_offset)/time_offset)*time_offset; // 5분단위로 시간을 관리하기 위해서

    if ((pLocalTime = (struct tm*)localtime((time_t*)&now)) == NULL) {
        strcpy(inserttimeStamp2, "");
    } else {
        strftime (inserttimeStamp2, 32, "%Y-%m-%d %H:%M", pLocalTime);
    }
    return (inserttimeStamp2);
}
static  char    ondemandtimeDelay[32];
char *get_ondemand_delay()
{
	time_t      now;
	struct tm   *pLocalTime;

	now = time(0);

	now = (now/STMD_5MIN_OFFSET) *STMD_5MIN_OFFSET;
	if ((pLocalTime = (struct tm*)localtime((time_t*)&now)) == NULL) {
		strcpy(ondemandtimeDelay, "");
	} else {
		strftime (ondemandtimeDelay, 32, "%Y-%m-%d %H", pLocalTime);
	}
	return (ondemandtimeDelay);
}

static  char    ondemandtimeStamp[32];
char *get_ondemand_time(int period)
{
    time_t      now;
    struct tm   *pLocalTime;

    now = time(0);
    now = now + period - STAT_OFFSET_UNIT; // 취합할 시간을 구하기 위해

    now = (now/STAT_OFFSET_UNIT) *STAT_OFFSET_UNIT; // 5분단위로 시간을 관리하기 위해서

    if ((pLocalTime = (struct tm*)localtime((time_t*)&now)) == NULL) {
        strcpy(ondemandtimeStamp, "");
    } else {
        strftime (ondemandtimeStamp, 32, "%Y-%m-%d %H:%M", pLocalTime);
    }
    return (ondemandtimeStamp);
}

static  char    ondemandtimeStamp2[32];
char *get_ondemand_time2(int period)
{
    time_t      now;
    struct tm   *pLocalTime;

    now = time(0);
    now = now + period; // 취합할 시간을 구하기 위해

    now = (now/STAT_OFFSET_UNIT) *STAT_OFFSET_UNIT; // 5분단위로 시간을 관리하기 위해서

    if ((pLocalTime = (struct tm*)localtime((time_t*)&now)) == NULL) {
        strcpy(ondemandtimeStamp2, "");
    } else {
        strftime (ondemandtimeStamp2, 32, "%Y-%m-%d %H:%M", pLocalTime);
    }
    return (ondemandtimeStamp2);
}


#if 0
char *get_on_demand_time(int diff)
{
    time_t      now;
    struct tm   *pLocalTime;

    now = time(0);
    now = now + diff;
    //now = now - STAT_OFFSET_UNIT; // 취합할 시간을 구하기 위해

    now = (now/STAT_OFFSET_UNIT) *STAT_OFFSET_UNIT; // 5분단위로 시간을 관리하기 위해서

    if ((pLocalTime = (struct tm*)localtime((time_t*)&now)) == NULL) {
        strcpy(ondemandtimeStamp, "");
    } else {
        strftime (ondemandtimeStamp, 32, "%Y-%m-%d %H:%M", pLocalTime);
    }
    return (ondemandtimeStamp);
}
#endif

int get_diff_time(char *pTime)
{
    time_t    now;
    struct tm *locTime;
    char      hourBuf[3], minBuf[3];
    int       tmBuf, locBuf;

    memset(hourBuf, 0x00, 3);
    memset(minBuf, 0x00, 3);
// 2009.04.23 jjinri note : pTime이 200904231340이면 
//    memcpy(hourBuf, pTime, 2); // 2009.04.23 jjinri note : hourBuf에 20이 들어가는데 이상함.. 
//    memcpy(minBuf, pTime+2, 2); // 2009.04.23 jjinri note : hourBuf에 20이 들어가는데 이상함..
    memcpy(hourBuf, pTime+8, 2); // 2009.04.23 jjinri note : hourBuf에 20이 들어가는데 이상함.. 
    memcpy(minBuf, pTime+10, 2); // 2009.04.23 jjinri note : hourBuf에 20이 들어가는데 이상함..

    tmBuf = (atoi(hourBuf))*3600 + (atoi(minBuf))*60;

    now = time(0);
    locTime = localtime((time_t *)&now);
    locBuf = locTime->tm_hour*3600 + locTime->tm_min*60;

    return (tmBuf-locBuf);
}

static char  ConvertTimeStr[32];
char *get_convert_time(long long ptime)
{
    time_t      now;
    struct tm   *pLocalTime;

    now = (time_t)ptime;
    if ((pLocalTime = (struct tm*)localtime((time_t*)&now)) == NULL) 
        strcpy(ConvertTimeStr, "");
    else
        strftime(ConvertTimeStr, 32, "%Y-%m-%d %H:%M", pLocalTime);

    return ConvertTimeStr;
}

static char  nowTimeStr[32];
char *get_now_time()
{
    time_t      now;
    struct tm   *pLocalTime;

    now = time(0);
    if ((pLocalTime = (struct tm*)localtime((time_t*)&now)) == NULL) 
        strcpy(nowTimeStr, "");
    else
        strftime(nowTimeStr, 32, "%Y-%m-%d %H:%M", pLocalTime);

    return nowTimeStr;
}

static  char    insertweekStamp[32];
char *get_insert_week()
{
    time_t      now;
    struct tm   *pLocalTime;

    now = time(0);
    now = now-STAT_OFFSET_UNIT;

    if ((pLocalTime = (struct tm*)localtime((time_t*)&now)) == NULL) {
        strcpy(insertweekStamp, "");
    } else {
        strftime (insertweekStamp, 32, "%a", pLocalTime);
    }
    return (insertweekStamp);
}

static  char    insertweekStamp2[32];
char *get_insert_week2()
{
    time_t      now;
    struct tm   *pLocalTime;

    now = time(0);
    pLocalTime = (struct tm*)localtime((time_t*)&now);
    
    /* month first day */
    now = now - (pLocalTime->tm_mday)*3600*23;

    if ((pLocalTime = (struct tm*)localtime((time_t*)&now)) == NULL) {
        strcpy(insertweekStamp2, "");
    } else {
        strftime (insertweekStamp2, 32, "%a", pLocalTime);
    }
    return (insertweekStamp2);
}

static  char    selecttimeStamp[32];
char *get_select_time(int time_type)
{
    time_t      now;
    struct tm   *pLocalTime;
    int         wday;
   
    if (time_type == STMD_HOUR){ 
        now = time(0) + (60*60);
        now = now-(STAT_OFFSET_UNIT);// now - 300 5분 단위로 만들어 준다
    } else{ // DAY,WEEK,MONTH
		now = time(0);
        now = now-(STAT_OFFSET_UNIT);
    }

    if ((pLocalTime = (struct tm*)localtime((time_t*)&now)) == NULL) {
        strcpy(selecttimeStamp, "");
    } else {
        switch (time_type) {
            case    STMD_MIN:
                strftime (selecttimeStamp, 32, "%Y-%m-%d %H:%M", pLocalTime);
                break;
            case    STMD_HOUR:
                strftime (selecttimeStamp, 32, "%Y-%m-%d %H", pLocalTime);
                break;
            case    STMD_DAY:
                strftime (selecttimeStamp, 32, "%Y-%m-%d ", pLocalTime);
                break;
            case    STMD_WEEK:
                // 월요일의 시간이 구해진다
                if(!pLocalTime->tm_wday)
                    wday = 7;
                else
                    wday = pLocalTime->tm_wday;
                now = now - (wday * STMD_1HOUR_OFFSET) + STMD_1HOUR_OFFSET;
                pLocalTime = (struct tm*)localtime((time_t*)&now);
                strftime (selecttimeStamp, 32, "%Y-%m-%d 00:00:00", pLocalTime);
                break;
            case    STMD_MONTH:
                strftime (selecttimeStamp, 32, "%Y-%m-01 00:00:00", pLocalTime);
                break;
        }
    }
    return (selecttimeStamp);
}

char *time_to_string(time_t time)
{
    
    struct tm   *pLocalTime;
    
    if ((pLocalTime = (struct tm*)localtime((time_t*)&time)) == NULL) {
        strcpy(selecttimeStamp, "");
    } else {
        strftime (selecttimeStamp, 32, "%Y-%m-%d %H:%M", pLocalTime);
    }
    return (selecttimeStamp);
}

static  char    selecttimeStamp2[32];
char *get_select_time2(int time_type)
{
    time_t      now;
    struct tm   *pLocalTime;
    int         wday;

    now = time(0);
    
    now = now-(STAT_OFFSET_UNIT);// now - 300 5분 단위로 만들어 준다

    if ((pLocalTime = (struct tm*)localtime((time_t*)&now)) == NULL) {
        strcpy(selecttimeStamp2, "");
    } else {
        switch (time_type) {
            case    STMD_MIN:
                strftime (selecttimeStamp2, 32, "%Y-%m-%d %H:%M:00", pLocalTime);
                break;
            case    STMD_HOUR:
                strftime (selecttimeStamp2, 32, "%Y-%m-%d %H:00", pLocalTime);
                break;
            case    STMD_DAY:
                strftime (selecttimeStamp2, 32, "%Y-%m-%d 00:00", pLocalTime);
                break;
            case    STMD_WEEK:
                // 월요일의 시간이 구해진다
                if(!pLocalTime->tm_wday)
                    wday = 7;
                else
                    wday = pLocalTime->tm_wday;
//printf("get_select_time2 wday %d %d\n",  pLocalTime->tm_wday, wday);
                now = now - (wday * STMD_1HOUR_OFFSET) + STMD_1HOUR_OFFSET;
                pLocalTime = (struct tm*)localtime((time_t*)&now);
                strftime (selecttimeStamp2, 32, "%Y-%m-%d 00:00:00", pLocalTime);
                break;
            case    STMD_MONTH:
                strftime (selecttimeStamp2, 32, "%Y-%m-01 00:00:00", pLocalTime);
                break;
        }
    }
    return (selecttimeStamp2);
}

static  char    prevMintimeStamp[32];
char *get_sel_prev_min_time()
{
    time_t      now;
    struct tm   *pLocalTime;

    // STAT_OFFSET_UNIT  == 300
    now = time(0)-STAT_OFFSET_UNIT;
    now = ((now-STAT_OFFSET_UNIT)/STAT_OFFSET_UNIT) *STAT_OFFSET_UNIT; 

    if ((pLocalTime = (struct tm*)localtime((time_t*)&now)) == NULL) {
        strcpy(prevMintimeStamp, "");
    } else {
        strftime (prevMintimeStamp, 32, "%Y-%m-%d %H:%M", pLocalTime);
    }
    return (prevMintimeStamp);
}

static  char    tosubsselecttimeStamp[32];
char *get_tosubs_select_time()
{
    time_t      now;
    struct tm   *pLocalTime;

    now = time(0);
    now = now-STAT_OFFSET_UNIT-STMD_1HOUR_OFFSET;// now - 300 - 1시간 5분단위로 하루전 시간을 만든다 

    if ((pLocalTime = (struct tm*)localtime((time_t*)&now)) == NULL)
        strcpy(tosubsselecttimeStamp, "");
    else 
        strftime (tosubsselecttimeStamp, 32, "%Y-%m-%d ", pLocalTime);

    return (tosubsselecttimeStamp);
}

static  char    periodselecttimeStamp[32];
char *get_period_select_time(int time_type)
{
    time_t      now;
    struct tm   *pLocalTime;

    now = time(0);
    if( (time_type == STMD_DAY) || (time_type == STMD_MONTH))    
        now = now- (printTIME[time_type]*120) - STAT_OFFSET_UNIT;

    if ((pLocalTime = (struct tm*)localtime((time_t*)&now)) == NULL) {
        strcpy(periodselecttimeStamp, "");
    } else {
        switch (time_type) {
            case    STMD_HOUR:
                strftime (periodselecttimeStamp, 32, "%Y-%m-%d %H", pLocalTime);
                break;
            case    STMD_DAY:
                strftime (periodselecttimeStamp, 32, "%Y-%m-%d ", pLocalTime);
                break;
            case    STMD_WEEK:
                //now = now - (pLocalTime->tm_wday * STMD_1HOUR_OFFSET);
                now = time(0);
                now = now - (7 * STMD_1HOUR_OFFSET);
                pLocalTime = (struct tm*)localtime((time_t*)&now);
                strftime (periodselecttimeStamp, 32, "%Y-%m-%d 00:00:00", pLocalTime);
                break;
            case    STMD_MONTH:
                strftime (periodselecttimeStamp, 32, "%Y-%m-01 00:00:00", pLocalTime);
                break;
        }
    }
    return (periodselecttimeStamp);
}

int stmd_mysql_init (int type)
{
	mysql_timeout = KEEPALIVE_DEADLINE;

	switch(type){
		case LOCAL_DB:
			mysql_init (&sql);
			mysql_options(&sql, MYSQL_OPT_CONNECT_TIMEOUT, (char*)&mysql_timeout);

			if (( conn = mysql_real_connect (&sql, "localhost", "root", "mysql", STM_STATISTIC_DB_NAME, 0, 0, 0)) == NULL) {
				sprintf(trcBuf,">>> mysql_real_connect fail; err=%d:%s\n", mysql_errno(&sql), mysql_error(&sql));
				trclib_writeLogErr (FL,trcBuf);
				return -1;
			}
			break;
		case REMOTE_DB:
			mysql_init (&sql_Rm);
			mysql_options(&sql_Rm, MYSQL_OPT_CONNECT_TIMEOUT, (char*)&mysql_timeout);

			if (( conn_Rm = mysql_real_connect (&sql_Rm, DSC_IP, "root", "mysql", CM_DB_NAME, 0, 0, 0)) == NULL) {
				sprintf(trcBuf,">>> mysql_real_connect fail; err=%d:%s\n", mysql_errno(&sql_Rm), mysql_error(&sql_Rm));
				trclib_writeLogErr (FL,trcBuf);
				return -2;
			}
			break;
	}

    return 0;

} //----- End of stmd_mysql_init -----//

int stmd_mysql_query_with_conn (MYSQL *con, char *query)
{
    if( conn_Rm == NULL )
    {
        sprintf(trcBuf,">>> mysql remote  connect fail;\n");
        trclib_writeLogErr (FL,trcBuf);
        sprintf(trcBuf,">>> mysql_remote_query fail; query=%s\n", query); trclib_writeLogErr (FL,trcBuf);
		return -1;
    }
	
    if (mysql_query (conn_Rm, query) != 0) {
        sprintf(trcBuf,">>> mysql_remote_query fail; err=%d:%s\n", mysql_errno(conn_Rm), mysql_error(conn_Rm));
        trclib_writeLogErr (FL,trcBuf);
        sprintf(trcBuf,">>> mysql_remote_query fail; query=%s\n", query); trclib_writeLogErr (FL,trcBuf);
		return -2;
	}
	return 0;
}

int stmd_mysql_query(char *query)
{
    if( conn == NULL )
    {
        sprintf(trcBuf,">>> mysql local connect fail;\n");
        trclib_writeLogErr (FL,trcBuf);
        sprintf(trcBuf,">>> mysql_local_query fail; query=%s\n", query); trclib_writeLogErr (FL,trcBuf);
		return -1;
		//sprintf(trcBuf,">>> mysql_query fail; stmd terminate.\n"); trclib_writeLogErr (FL,trcBuf);
		//exit(-1);
    }
	
    if (mysql_query (conn, query) != 0) {
        sprintf(trcBuf,">>> mysql_local_query fail; err=%d:%s\n", mysql_errno(conn), mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        sprintf(trcBuf,">>> mysql_local_query fail; query=%s\n", query); trclib_writeLogErr (FL,trcBuf);
		return -2;
		//sprintf(trcBuf,">>> mysql_query fail; stmd terminate.\n"); trclib_writeLogErr (FL,trcBuf);
		//exit(-2);
    }
    return 0;
}


/* Fail Udr은 Duplication err가 너무 많습니다..*/
int stmd_mysql_query_failUdr (char *query)
{
	if (stmd_mysql_query (query) < 0) {
        return -1;
    }
    return 1;
}

/* UDR Service Type 별 통계는 Duplication err가 너무 많습니다..*/
int stmd_mysql_query_udrsvctype (char *query)
{

	if (stmd_mysql_query (query) < 0) {
        return -1;
    }
    return 1;
}

/* UDR Duplication err 가 많아 잠시 로그를 막아 둡니다.*/
int stmd_mysql_query_udr (char *query)
{

	if (stmd_mysql_query (query) < 0) {
        return -1;
    }
    return 1;
}

int stmd_mysql_select_query (char *query)
{
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int         ret_flag;

	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; query:%s, err=%s\n", query, mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }

    ret_flag = 0; // select flag초기화
    result = mysql_store_result(conn);
    while( ( row = mysql_fetch_row(result)) != NULL ) {
        ret_flag = 1; //select 성공
        break;
    }
    mysql_free_result(result);

    return ret_flag;
}

int stmd_checkParaTimeValue (char *time, int *year, int *mon, int *day, int *hour, int *min)
{
    int     i;
    char    tmp[32];

    for (i=0; i<strlen(time); i++) {
        if (!isdigit(time[i]))
            return -1;
    }

    strncpy (tmp, &time[0], 4); tmp[4] = 0; *year = atoi(tmp);
    strncpy (tmp, &time[4], 2); tmp[2] = 0; *mon  = atoi(tmp);
    strncpy (tmp, &time[6], 2); tmp[2] = 0; *day  = atoi(tmp);
    strncpy (tmp, &time[8], 2); tmp[2] = 0; *hour = atoi(tmp);
    strncpy (tmp, &time[10],2); tmp[2] = 0; *min = atoi(tmp);

    if (*mon  < 1 || *mon  > 12) return -1;
    if (*day  < 1 || *day  > 31) return -1;
    if (*hour < 0 || *hour > 23) return -1;
    if (*min  < 0 || *min  > 59) return -1;

    return 1;
}

int checkFreeList (int type)
{
    int     i;

    if ( type == ONDEMANDJOB ){
        for (i=0; i< MAX_ONDEMAND_NUM; i++){
            if ( onDEMAND[i].statisticsType == NOT_REGISTERED ){
                return i;
            }
        }
    } else {
        for (i=0; i< MAX_CRONJOB_NUM; i++){
            if ( cronJOB[i].statisticsType == NOT_REGISTERED ){
                return i;
            }
        }
    }
    return -1;
}

static  char    nmsselecttimeStamp[32];
char *get_nms_select_time()
{
    time_t      now;
    struct tm   *pLocalTime;

    now = time(0);
    now = now- (55*60) - STAT_OFFSET_UNIT;

    if ((pLocalTime = (struct tm*)localtime((time_t*)&now)) == NULL) {
        strcpy(nmsselecttimeStamp, "");
    } else {
        strftime (nmsselecttimeStamp, 32, "%Y-%m-%d %H:00", pLocalTime);
    }
    return (nmsselecttimeStamp);
}

static  char    endselecttimeStamp[32];
char *get_select_endtime(int type, int year, int mon, int day, int hour, int min, int cnt)
{
    time_t      timev;
    struct tm   tm_src;
    struct tm   *pLocalTime;
    int         period;

    tm_src.tm_year = year - 1900;
    tm_src.tm_mon  = mon - 1;
    tm_src.tm_mday = day;
    tm_src.tm_hour = hour;
    tm_src.tm_min  = min;
    tm_src.tm_sec  = 0; 
    
    timev = mktime( &tm_src  );
    
    switch(type)
    {
    case STMD_MIN:
        period = 60*5*cnt;
        break;
    case STMD_HOUR:
        period = 3600*cnt;
        break;
    case STMD_DAY:
        period = 3600*24*cnt;
        break;
    case STMD_WEEK:
        period = 3600*24*cnt*7;
        break;
    case STMD_MONTH:
        period = 60*60*24*30*cnt;
        break;
    }
    
    timev = timev+period;   
    if ((pLocalTime = (struct tm*)localtime((time_t*)&timev)) == NULL) 
    {
        strcpy(endselecttimeStamp, "");
    }
    else
    {
        if(type == STMD_MIN ||type== STMD_HOUR)
        {
            strftime (endselecttimeStamp, 32, "%Y-%m-%d %H:%M", pLocalTime);
        }
        else if(type == STMD_DAY ||type== STMD_WEEK)
        {
            strftime (endselecttimeStamp, 32, "%Y-%m-%d 00:00", pLocalTime);
        }
        else if(type == STMD_MONTH)
        {
            strftime (endselecttimeStamp, 32, "%Y-%m-01 00:00", pLocalTime);
        }   
    }
    
    return (endselecttimeStamp);
}


int stmd_nms_checkParaTimeValue (char *time, int *year, int *mon, int *day, int *hour)
{
    int     i;
    char    tmp[32];

    for (i=0; i<strlen(time); i++) {
        if (!isdigit(time[i]))
            return -1;
    }

    strncpy (tmp, &time[0], 4); tmp[4] = 0; *year = atoi(tmp);
    strncpy (tmp, &time[4], 2); tmp[2] = 0; *mon  = atoi(tmp);
    strncpy (tmp, &time[6], 2); tmp[2] = 0; *day  = atoi(tmp);
    strncpy (tmp, &time[8], 2); tmp[2] = 0; *hour = atoi(tmp);

    if (*mon  < 1 || *mon  > 12) return -1;
    if (*day  < 1 || *day  > 31) return -1;
    if (*hour < 0 || *hour > 23) return -1;

    return 1;
}

/*
** 로그 directory를 가져온다.
*/
void getFilePath ( char *path, char tm[][8], time_t *tt )
{
    char    buf[30];
    int     i;
//    struct  tm tms;
    struct  tm *tms;
    char    *env;

    memset ( buf, 0x00, 30 );

    /*
    ** 1999/Jul/23/13/ 이런 형식으로, hourly인 경우
    ** $를 둔 이유는 뒤에서 token으로 사용하기 위해서 이다.
    */
//    localtime_r ( tt, &tms );
    tms = localtime ( tt );
//    strftime ( buf, 30, "%Y/%m/%d/%H/", &tms );
    strftime ( buf, 30, "%Y/%m/%d/%H/", tms );

    /* tm array에 각각 1999, JUL, 23, 22 를 적는다 */
    strcpy ( tm[0], strtok (buf, "/") );
    for ( i =1; i< DIR_NUM; i++ ){
        strcpy ( tm[i], strtok (NULL,"/") );
    }

    if ((env = getenv(IV_HOME)) == NULL) {
        sprintf(trcBuf,"[InitSys] not found %s environment name\n", IV_HOME);
        trclib_writeErr(FL, trcBuf);
    }
    sprintf ( path, "%s/%s",env, STATD_MAIN_PATH );
}

/*
** 위에서 가져온 이름으로 각각의 directory를 만든다
*/
void makeDirectory (int time_type,char *path, char tm[][8] )
{
    char    buf[50];
    char    *env;

    if ((env = getenv(IV_HOME)) == NULL) {
        sprintf(trcBuf,"[InitSys] not found %s environment name\n", IV_HOME);
        trclib_writeErr(FL, trcBuf);
    }

//최상위인 년 directory를만든다
    sprintf(buf, "%s", tm[0]);
    strcat(path, buf); 
    mkdir(path, DIRMODE); // ~/log/STAT/2005

    sprintf(buf, "/%s", tm[1]);
    strcat(path, buf); 
    mkdir(path, DIRMODE); // ~/log/STAT/2005/06

    if(time_type == STMD_MONTH || time_type == STMD_WEEK)
        return;

    sprintf(buf, "/%s", tm[2]);
    strcat(path, buf); 
    mkdir(path, DIRMODE); // ~/log/STAT/2005/06/20

    if(time_type == STMD_DAY)
        return;

    sprintf(buf, "/%s", tm[3]);
    strcat(path, buf); 
    mkdir(path, DIRMODE); // ~/log/STAT/2005/06/20/12
}
/*
** file 이름를 완성한다.
*/
void makeFileName ( char *fileName, int statType, int timeOpt , char tm[][8] )
{
    char    statChar[20];
    char    optChar [24];

	memset (statChar, 0x00, sizeof(statChar));

    switch ( statType ){
        case STMD_LOAD:
            sprintf(statChar, "LOAD_");
            break;

        case STMD_FAULT:
            sprintf(statChar, "FAULT_");
            break;

        case STMD_LINK:
            sprintf(statChar, "LINK_TOTAL_");
            break;

        case STMD_LEG:
            sprintf(statChar, "LEG_");
            break;

        case STMD_LOGON:
            sprintf(statChar, "LOGON_");
            break;

        case STMD_FLOW:
            sprintf(statChar, "FLOW_");
            break;

        case STMD_RULE_SET:
            sprintf(statChar, "RULESET_");
            break;

        case STMD_RULE_ENT:
            sprintf(statChar, "RULEENT_");
            break;

        case STMD_SMS:
            sprintf(statChar, "SMS_");
            break;

        case STMD_DEL:
            sprintf(statChar, "DELAY_");
            break;

#if 0
        case STMD_DB:
            sprintf(statChar, "DB_");
            break;
        case STMD_SCIB:
            sprintf(statChar, "SCIB_");
            break;
        case STMD_RCIF:
            sprintf(statChar, "RCIF_");
            break;
        case STMD_SCPIF:
            sprintf(statChar, "SCIF_");
            break;
        case STMD_WISE:
            sprintf(statChar, "WISE_");
            break;
        case STMD_OB:
            sprintf(statChar, "OB_");
            break;
#endif
#if 0
        case STMD_TRAN:
            // 2006.08.22 by sdlee
            //sprintf(statChar, "TRAN_");
            sprintf(statChar, "IP_");
            break;
        case STMD_AAA:
            strcpy(statChar, "AAA_");
            break;
        case STMD_UAWAP:
            strcpy(statChar, "UAWAP_");
            break;
        case STMD_SVC_TR:
            // 2006.08.22 by sdlee
            //strcpy(statChar, "SVC_TTR_");
            strcpy(statChar, "SVC_");
            break;
        case STMD_RADIUS:
            strcpy(statChar, "RADIUS_");
            break;
        case STMD_SVC_TTR:
            // 2006.08.22 by sdlee
            //strcpy(statChar, "SVC_TTR_");
            strcpy(statChar, "TCPUDP_");
            break;
        case STMD_UDR:
            strcpy(statChar, "UDR_");
            break;
        case STMD_TYPE_UDR:
            strcpy(statChar, "TYPE_UDR_");
            break;
        case STMD_CDR:
            strcpy(statChar, "CDR_");
            break;
        case STMD_AN_AAA:
            strcpy(statChar, "AN_AAA_");
            break;  
        case SVC_WAP1:
            strcpy(statChar, "WAP1_");
            break;
        case SVC_WAP2:
            strcpy(statChar, "WAP2_");
            break;
        case SVC_HTTP:
            strcpy(statChar, "HTTP_");
            break;
        case SVC_JAVA:
            strcpy(statChar, "JAVA_");
            break;
        case SVC_VODS:
            strcpy(statChar, "VODS_");
            break;
        case SVC_WIPI:
            strcpy(statChar, "WIPI_");
            break;
        case SVC_VT:
            strcpy(statChar, "VT_");
            break;    
		case STMD_CDR2:
            strcpy(statChar, "CDR2_");
            break;
		case STMD_FAIL_UDR:
            strcpy(statChar, "FAIL_UDR_");
            break;
#endif // by jjinri 2009.04.18
    }

    switch ( timeOpt ){
    case STMD_HOUR:
        sprintf ( optChar,"%sHOUR",statChar );//3
        break;

    case STMD_DAY:
        sprintf ( optChar,"%sDAY",statChar );//2
        break;

    case STMD_WEEK:
        sprintf ( optChar,"%sWEEK_%s",statChar, tm[DIR_NUM-2] );//2
        break;

    case STMD_MONTH:
        sprintf ( optChar,"%sMONTH",statChar );//1
        break;

    default :
            return;
    }

    strcat(fileName, "/");
    strcat(fileName, optChar);
    
}

// stmd msg Queue에 들어온 메세지를 처리

int stmd_msg_receiver() 
{

	GeneralQMsgType rxGenQMsg;
	int ret;

	while ((ret = msgrcv(stmdQid, &rxGenQMsg, sizeof(GeneralQMsgType), 0, IPC_NOWAIT)) > 0) {
    		stmd_exeRxQMsg (&rxGenQMsg);
    		keepalivelib_increase();
    		memset(&rxGenQMsg, 0, sizeof(GeneralQMsgType));
	}
	if ((ret < 0) && (errno == EINVAL || errno == EFAULT)) {
    		sprintf(trcBuf,"[stmd_main] >>> msgrcv fail; err=%d(%s)\n", errno, strerror(errno));
    		trclib_writeLogErr (FL,trcBuf);
    		return -1;
	}
	return 0;
}

int send_query_info (int mprd, char *stm, char *etm)
{
    int                 slen=0;
    GeneralQMsgType     smsg;
    StatQueryInfo       *qry;

    memset (&smsg, 0, sizeof (GeneralQMsgType));

    smsg.mtype  = MTYPE_STATISTICS_REPORT;

    qry         = (StatQueryInfo *)smsg.body;

    qry->period = mprd;
    strcpy (qry->stime, stm);
    strcpy (qry->etime, etm);

    slen = sizeof (StatQueryInfo) + 8;

    if (msgsnd (nmsifQid, &smsg, slen, IPC_NOWAIT) < 0) {
        sprintf (trcBuf, "fail msgsnd to nmsif (%s), mprd=%d, tm=(%s~%s)\n", 
                    strerror (errno), mprd, stm, etm);
        trclib_writeErr (FL, trcBuf);
        return -1;
    }

    return 1;

} /* End of send_query_info () */
