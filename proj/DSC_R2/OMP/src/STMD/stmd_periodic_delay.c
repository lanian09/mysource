#include "stmd_proto.h"

extern  char    trcBuf[4096], trcTmp[1024];
extern  int     trcLogFlag, trcLogId;
extern  int     sysCnt;

extern  short   maskITEM  [STMD_MASK_ITEM_NUM][STMD_PERIOD_TYPE_NUM];
extern  STM_CommStatisticInfo    StatisticSystemInfo[SYSCONF_MAX_ASSO_SYS_NUM];
extern  MYSQL       sql, *conn;

extern  short   printTIME [STMD_PERIOD_TYPE_NUM];

int stmd_DelaySelect(int time_type, char *table_type)
{
    char        condBuf[4096], tmpBuf[1024];
    int         sts_code;
    char        str_time[10];
    int         select_cnt = 0;

    char        query[4096];
    MYSQL_RES   *result;
    MYSQL_ROW   row;

    time_t      now;
    char        tm[DIR_NUM][8];
    char        path[60];
    char        fileName[60];
    FILE        *fp;
	char		*SysName = "SCE";
    int         i;
    char title[4][16]={"SYSTEM", "MIN_msec", "MAX_msec","AVG_msec"};
    int row_index;
    int realSysCnt =1;
    int realItemCnt =1;

    now = time(0);
    if(time_type != STMD_WEEK)
        now = now - (printTIME[time_type]*60) - STAT_OFFSET_UNIT;
    else
        now = now - (printTIME[time_type]*60);

    getFilePath (path, tm, &now); // ~/LOG/STAT 까지 만든다.
    makeDirectory (time_type,path,tm);

    sprintf (fileName, "%s", path );
    makeFileName ( fileName, STMD_DEL, time_type, tm );

    if ( ( fp = fopen(fileName, APPEND ) ) == NULL ){
        sprintf(trcBuf, "%s Open Fail\n", fileName);
        trclib_writeLog(FL, trcBuf);
        return -1;
    }

    switch (time_type) {
        case    STMD_HOUR :
            sts_code = STSCODE_STM_PERIODIC_DEL_HOUR;
            sprintf(str_time, "%s", STMD_STR_HOUR);
            break;
        case    STMD_DAY :
            sts_code = STSCODE_STM_PERIODIC_DEL_DAY;
            sprintf(str_time, "%s", STMD_STR_DAY);
            break;
        case    STMD_WEEK :
            sts_code = STSCODE_STM_PERIODIC_DEL_WEEK;
            sprintf(str_time, "%s", STMD_STR_WEEK);
            break;
        case    STMD_MONTH :
            sts_code = STSCODE_STM_PERIODIC_DEL_MONTH;
            sprintf(str_time, "%s", STMD_STR_MONTH);
            break;
    }

    sprintf(condBuf,"    %s %s\n    S%04d DELAY PERIODIC STATISTICS MESSAGE\n",
        "SCE", // 현재는 OMP로 고정하지만 실질적인 시스템 이름으로 변경 필요 hslim_oper
        commlib_printTStamp(), // 현재시각 time stamp (년,월,일,시,분,초,요일)
        sts_code);

    sprintf(tmpBuf, "    PERIOD = %s\n    MEASURETIME = %s  -  %s\n\n",
        str_time,
        (char *)get_period_start_time(time_type), (char *)get_period_end_time(time_type));
    strcat(condBuf,tmpBuf);

    sprintf(tmpBuf, "    ====================================================\n");
    strcat(condBuf, tmpBuf);
    sprintf(tmpBuf, "%3s %8s %12s %12s %12s\n","",title[0],title[1],title[2],title[3] );
    strcat(condBuf, tmpBuf);
    sprintf(tmpBuf, "    ====================================================\n");
    strcat(condBuf, tmpBuf);

    for(i = 0; i < realSysCnt; i++)
	{
		realItemCnt = 1;

	   	sprintf(query, "SELECT system_name, "
			   " ROUND(IFNULL(MIN(min_usec*1000),0), 3), ROUND(IFNULL(MAX(max_usec*1000),0), 3), "
			   " ROUND(IFNULL(AVG(avg_usec*1000),0), 3) "
			   " from %s "
			   " where system_name = '%s' AND (stat_date = '%s') "
			   " group by system_name ",
			   table_type, SysName, (char *)get_period_select_time(time_type));

        if ( trcLogFlag == TRCLEVEL_SQL ) {
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }
		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);
            if(fp != NULL) fclose(fp);
            return -1;
        }

        result = mysql_store_result(conn);
        while((row = mysql_fetch_row(result)) != NULL) 
		{
			row_index = 1;
			if (select_cnt == 0)
			{
				sprintf(tmpBuf, "%3s %8s %12s %12s %12s\n",
						"",SysName,row[row_index],row[row_index+1],row[row_index+2]);
				strcat(condBuf, tmpBuf);
			} 
			else 
			{
				sprintf(tmpBuf, "%3s %8s %12s %12s %12s\n",
						"","",row[row_index],row[row_index+1],row[row_index+2]);
				strcat(condBuf, tmpBuf);
			}

            select_cnt++;
        }
        mysql_free_result(result);

        if (select_cnt == 0) 
		{
			sprintf(tmpBuf, "%3s %8s %12s %12s %12s\n",
					"", SysName, "0.000","0.000","0.000");
			strcat(condBuf, tmpBuf);
        } 
    	sprintf(tmpBuf, "    ====================================================\n");
		strcat(condBuf, tmpBuf);
    }

    sprintf(tmpBuf, "    COMPLETED\n\n\n");
    strcat(condBuf, tmpBuf);

    if(fp != NULL) 
	{
        fprintf (fp, "%s",condBuf);
        fclose(fp);
    }
    
    stmd_txMsg2Cond(condBuf, (sts_code - STSCODE_TO_MSGID_STATISTICS), 0, 1);
    
    return 1;
}

int doPeriodicHourDelay()
{
    if ( maskITEM[STMD_DEL][STMD_HOUR] == MASK)
	{
		logPrint(trcLogId, FL, "DELAY HOUR Period Masking...TT\n");
        return -1;
	}

    if ( stmd_DelaySelect(STMD_HOUR, STM_STATISTIC_HOUR_DELAY_TBL_NAME) < 0 ) {
        sprintf(trcBuf, "DELAY HOUR Select Error\n");
        trclib_writeLogErr(FL, trcBuf);
        return -1;
    }

    return 1;
}

int doPeriodicDayDelay()
{
    if ( maskITEM[STMD_DEL][STMD_DAY] == MASK)
	{
		logPrint(trcLogId, FL, "DELAY DAY Period Masking...TT\n");
        return -1;
	}

    if ( stmd_DelaySelect(STMD_DAY, STM_STATISTIC_DAY_DELAY_TBL_NAME) < 0 ) {
        sprintf(trcBuf, "DELAY DAY Select Error\n");
        trclib_writeLogErr(FL, trcBuf);
        return -1;
    }

    return 1;
}

int doPeriodicWeekDelay()
{
    if ( maskITEM[STMD_DEL][STMD_WEEK] == MASK)
	{
		logPrint(trcLogId, FL, "DELAY WEEK Period Masking...TT\n");
        return -1;
	}

    if ( stmd_DelaySelect(STMD_WEEK, STM_STATISTIC_WEEK_DELAY_TBL_NAME) < 0 ) {
        sprintf(trcBuf, "DELAY WEEK Select Error\n");
        trclib_writeLogErr(FL, trcBuf);
        return -1;
    }

    return 1;
}

int doPeriodicMonthDelay()
{
    if ( maskITEM[STMD_DEL][STMD_MONTH] == MASK)
	{
		logPrint(trcLogId, FL, "DELAY MONTH Period Masking...TT\n");
        return -1;
	}

    if ( stmd_DelaySelect(STMD_MONTH, STM_STATISTIC_MONTH_DELAY_TBL_NAME) < 0 ) {
        sprintf(trcBuf, "DELAY MONTH Select Error\n");
        trclib_writeLogErr(FL, trcBuf);
        return -1;
    }

    return 1;
}
