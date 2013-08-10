#include "stmd_proto.h"

extern  char    trcBuf[4096], trcTmp[1024];
extern  int     trcLogFlag, trcLogId;
extern  int     sysCnt;

extern  short   maskITEM  [STMD_MASK_ITEM_NUM][STMD_PERIOD_TYPE_NUM];
extern  STM_CommStatisticInfo    StatisticSystemInfo[SYSCONF_MAX_ASSO_SYS_NUM];
extern  MYSQL       sql, *conn;

extern  short   printTIME [STMD_PERIOD_TYPE_NUM];

int stmd_FlowSelect(int time_type, char *table_type)
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

    int         i;
    char title[4][16]={"SYSTEM", "AVG_FLOW", "MIN_FLOW","MAX_FLOW"};
    int row_index;
    char SysName[2][8] = {"SCEA","SCEB"};
    int realSysCnt =2;
    int realItemCnt =0;

    now = time(0);
    if(time_type != STMD_WEEK)
        now = now - (printTIME[time_type]*60) - STAT_OFFSET_UNIT;
    else
        now = now - (printTIME[time_type]*60);

    getFilePath (path, tm, &now); // ~/LOG/STAT ���� �����.
    makeDirectory (time_type,path,tm);

    sprintf(fileName, "%s", path );
    makeFileName ( fileName, STMD_FLOW, time_type, tm );

    if ( ( fp = fopen(fileName, APPEND ) ) == NULL ){
        sprintf(trcBuf, "%s Open Fail\n", fileName);
        trclib_writeLog(FL, trcBuf);
        return -1;
    }


    switch (time_type) {
        case    STMD_HOUR :
            sts_code = STSCODE_STM_PERIODIC_FLOW_HOUR;
            sprintf (str_time, "%s", STMD_STR_HOUR);
            break;
        case    STMD_DAY :
            sts_code = STSCODE_STM_PERIODIC_FLOW_DAY;
            sprintf (str_time, "%s", STMD_STR_DAY);
            break;
        case    STMD_WEEK :
            sts_code = STSCODE_STM_PERIODIC_FLOW_WEEK;
            sprintf (str_time, "%s", STMD_STR_WEEK);
            break;
        case    STMD_MONTH :
            sts_code = STSCODE_STM_PERIODIC_FLOW_MONTH;
            sprintf (str_time, "%s", STMD_STR_MONTH);
            break;
    }

    sprintf(condBuf,"    %s %s\n    S%04d FLOW PERIODIC STATISTICS MESSAGE\n",
        "SCM", // ����� OMP�� ���������� �������� �ý��� �̸����� ���� �ʿ� hslim_oper
        commlib_printTStamp(), // ����ð� time stamp (��,��,��,��,��,��,����)
        sts_code);

    sprintf(tmpBuf, "    PERIOD = %s\n    MEASURETIME = %s  -  %s\n\n",
        str_time,
        get_period_start_time(time_type), get_period_end_time(time_type));
    strcat(condBuf,tmpBuf);

    sprintf(tmpBuf, "    ====================================================================\n");
    strcat(condBuf, tmpBuf);
	sprintf(tmpBuf, "%3s %8s %12s %12s %12s\n","",title[0],title[1],title[2],title[3] );
	strcat(condBuf, tmpBuf);
    sprintf(tmpBuf, "    ====================================================================\n");
    strcat(condBuf, tmpBuf);

    for(i = 0; i < realSysCnt; i++)
	{
		realItemCnt = 1;

	   	sprintf(query, "SELECT system_name, "
				" round(ifnull(avg(avg_flow), 0),0), ifnull(min(min_flow), 0), ifnull(max(max_flow), 0) "
			   " from %s "
			   " where system_name = '%s' AND (stat_date = '%s') "
			   " group by system_name ",
			   table_type, SysName[i], get_period_select_time(time_type));

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

        select_cnt = 0;
        result = mysql_store_result(conn);
        while((row = mysql_fetch_row(result)) != NULL) 
		{
			row_index = 1;
			if (select_cnt == 0)
			{
				sprintf(tmpBuf, "%3s %8s %12s %12s %12s\n",
						"",SysName[i],row[row_index],row[row_index+1],row[row_index+2]);
				strcat(condBuf, tmpBuf);
			} 
			else 
			{
				sprintf(tmpBuf, "%3s %8s %12s %12s %12s\n",
						"","",row[row_index],row[row_index+1],row[row_index+2]);
				strcat(condBuf, tmpBuf);
			}

    		sprintf(tmpBuf, "    ====================================================================\n");
			strcat(condBuf, tmpBuf);
			select_cnt++;
        }
        mysql_free_result(result);

        if (select_cnt == 0) 
		{
			sprintf(tmpBuf, "%3s %8s %12s %12s %12s\n",
					"",SysName[i], "0","0","0");
			strcat(condBuf, tmpBuf);
    		sprintf(tmpBuf, "    ====================================================================\n");
            strcat(condBuf, tmpBuf);
        } 
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

int doPeriodicHourFlow()
{
    if ( maskITEM[STMD_FLOW][STMD_HOUR] == MASK)
	{
		logPrint(trcLogId, FL, "FLOW HOUR Period Masking...TT\n");
        return -1;
	}

    if ( stmd_FlowSelect(STMD_HOUR, STM_STATISTIC_HOUR_FLOW_TBL_NAME) < 0 ) {
        sprintf(trcBuf, "FLOW HOUR Select Error\n");
        trclib_writeLogErr(FL, trcBuf);
        return -1;
    }

    return 1;
}

int doPeriodicDayFlow()
{
    if ( maskITEM[STMD_FLOW][STMD_DAY] == MASK)
	{
		logPrint(trcLogId, FL, "FLOW DAY Period Masking...TT\n");
        return -1;
	}

    if ( stmd_FlowSelect(STMD_DAY, STM_STATISTIC_DAY_FLOW_TBL_NAME) < 0 ) {
        sprintf(trcBuf, "FLOW DAY Select Error\n");
        trclib_writeLogErr(FL, trcBuf);
        return -1;
    }

    return 1;
}

int doPeriodicWeekFlow()
{
    if ( maskITEM[STMD_FLOW][STMD_WEEK] == MASK)
	{
		logPrint(trcLogId, FL, "FLOW WEEK Period Masking...TT\n");
        return -1;
	}

    if ( stmd_FlowSelect(STMD_WEEK, STM_STATISTIC_WEEK_FLOW_TBL_NAME) < 0 ) {
        sprintf(trcBuf, "FLOW WEEK Select Error\n");
        trclib_writeLogErr(FL, trcBuf);
        return -1;
    }

    return 1;
}

int doPeriodicMonthFlow()
{
    if ( maskITEM[STMD_FLOW][STMD_MONTH] == MASK)
	{
		logPrint(trcLogId, FL, "FLOW MONTH Period Masking...TT\n");
        return -1;
	}

    if ( stmd_FlowSelect(STMD_MONTH, STM_STATISTIC_MONTH_FLOW_TBL_NAME) < 0 ) {
        sprintf(trcBuf, "FLOW MONTH Select Error\n");
        trclib_writeLogErr(FL, trcBuf);
        return -1;
    }

    return 1;
}
