#include "stmd_proto.h"

extern  char    trcBuf[4096], trcTmp[1024];
extern  int     trcLogFlag, trcLogId;
extern  int     sysCnt;

extern  short   maskITEM  [STMD_MASK_ITEM_NUM][STMD_PERIOD_TYPE_NUM];
extern  STM_CommStatisticInfo    StatisticSystemInfo[SYSCONF_MAX_ASSO_SYS_NUM];
extern  MYSQL       sql, *conn;

extern  short   printTIME [STMD_PERIOD_TYPE_NUM];

int stmd_LoadSelect(int time_type, char *table_type)
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

    char        itemRes[8][10];
    int         i,j;
//    char *title[]={"CPU","MEM","DISK","QUEUE","SESS"};
    char *title[]={"CPU","MEM","HW","QUEUE"};
    int row_index;
    char SysName[5][8];
    int realSysCnt =0;
    int realItemCnt =0;

    for(i=0; i<sysCnt; i++ ){
        sprintf(SysName[i], "%s", StatisticSystemInfo[i].sysName);
        realSysCnt++;
    }
/* 10.13 TAP REMOVE
    sprintf(SysName[realSysCnt], "TAPA");realSysCnt++;
    sprintf(SysName[realSysCnt], "TAPB");realSysCnt++;
*/
    sprintf(SysName[realSysCnt], "SCEA");realSysCnt++;
    sprintf(SysName[realSysCnt], "SCEB");realSysCnt++;


    now = time(0);
    if(time_type != STMD_WEEK)
        now = now - (printTIME[time_type]*60) - STAT_OFFSET_UNIT;
    else
        now = now - (printTIME[time_type]*60);

    getFilePath (path, tm, &now); // ~/LOG/STAT 까지 만든다.
    makeDirectory (time_type,path,tm);

    sprintf(fileName, "%s", path );
    makeFileName ( fileName, STMD_LOAD, time_type, tm );

    if ( ( fp = fopen(fileName, APPEND ) ) == NULL ){
        sprintf(trcBuf, "%s Open Fail\n", fileName);
        trclib_writeLog(FL, trcBuf);
        return -1;
    }


    switch (time_type) {
        case    STMD_HOUR :
            sts_code = STSCODE_STM_PERIODIC_LOAD_HOUR;
            sprintf(str_time, "%s", STMD_STR_HOUR);
            break;
        case    STMD_DAY :
            sts_code = STSCODE_STM_PERIODIC_LOAD_DAY;
            sprintf(str_time, "%s", STMD_STR_DAY);
            break;
        case    STMD_WEEK :
            sts_code = STSCODE_STM_PERIODIC_LOAD_WEEK;
            sprintf(str_time, "%s", STMD_STR_WEEK);
            break;
        case    STMD_MONTH :
            sts_code = STSCODE_STM_PERIODIC_LOAD_MONTH;
            sprintf(str_time, "%s", STMD_STR_MONTH);
            break;
    }

    sprintf(condBuf,"    %s %s\n    S%04d LOAD PERIODIC STATISTICS MESSAGE\n",
        //"BSDM", // 현재는 OMP로 고정하지만 실질적인 시스템 이름으로 변경 필요 hslim_oper
        "DSCM", // 현재는 OMP로 고정하지만 실질적인 시스템 이름으로 변경 필요 hslim_oper
        commlib_printTStamp(), // 현재시각 time stamp (년,월,일,시,분,초,요일)
        sts_code);

    sprintf(tmpBuf, "    PERIOD = %s\n    MEASURETIME = %s  -  %s\n\n",
        str_time,
        get_period_start_time(time_type), get_period_end_time(time_type));
    strcat(condBuf,tmpBuf);

    sprintf(tmpBuf, "    ==================================\n");
    strcat(condBuf, tmpBuf);
    sprintf(tmpBuf, "    SYSTEM    ITEM    AVG(%%)    MAX(%%)\n");
    strcat(condBuf, tmpBuf);
    sprintf(tmpBuf, "    ==================================\n");
    strcat(condBuf, tmpBuf);

    for(i = 0; i < realSysCnt; i++){

        //if(!strcasecmp(SysName[i], "BSDM"))
        if(!strcasecmp(SysName[i], "DSCM"))
            realItemCnt=4;
/* 10.13 TAP REMOVE
        else if(!strcasecmp(SysName[i], "TAPA") || !strcasecmp(SysName[i], "TAPB"))
            realItemCnt=3;
*/
		else if(!strcasecmp(SysName[i], "SCEA") || !strcasecmp(SysName[i], "SCEB"))
            realItemCnt=3;
        else
            realItemCnt=4;
//            realItemCnt=5;

        sprintf(query, "SELECT "
                " IFNULL(AVG(avr_cpu0), 0), IFNULL(MAX(max_cpu0), 0)," 
                " IFNULL(AVG(avr_memory), 0), IFNULL(MAX(max_memory), 0)," 
                " IFNULL(AVG(avr_disk), 0), IFNULL(MAX(max_disk), 0)," 
                " IFNULL(AVG(avr_msgQ), 0), IFNULL(MAX(max_msgQ), 0) " 
           //     " IFNULL(AVG(avr_sess), 0), IFNULL(MAX(max_sess), 0)" 
                " from %s "
                " where system_name = '%s' AND (stat_date = '%s')",
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

        result = mysql_store_result(conn);
        while((row = mysql_fetch_row(result)) != NULL) 
		{
//            for(j=0; j < 10; j++)
            for(j=0; j < 8; j++)
			{ // SESS는 그댜로 출력 하자. add by helca 080930
                strcpy(itemRes[j], row[j]);
				sprintf(itemRes[j], "%d.%d", atoi(itemRes[j])/10, atoi(itemRes[j])%10); 
				/** session delete 0608
				if(j<8)
					sprintf(itemRes[j], "%d.%d", atoi(itemRes[j])/10, atoi(itemRes[j])%10); 
				else
					sprintf(itemRes[j], "%d", atoi(itemRes[j]));
				*/
            }

            row_index = 0;
            for(j=0; j < realItemCnt; j++)
			{
                if (j==0)
				{
                    sprintf(tmpBuf, "%3s %-9s %-5s %8s %9s\n", 
                        "", SysName[i], title[j], itemRes[row_index],itemRes[row_index+1]);
                    strcat(condBuf, tmpBuf);
                } 
				else 
				{
                    sprintf(tmpBuf, "%3s %-9s %-5s %8s %9s\n", 
                        "", "", title[j], itemRes[row_index],itemRes[row_index+1]);
                    strcat(condBuf, tmpBuf);
                }

                row_index += 2;
            }
            sprintf(tmpBuf, "    ==================================\n");
            strcat(condBuf, tmpBuf);

            select_cnt++;
        }
        mysql_free_result(result);

        if (select_cnt == 0) 
		{
            for(j=0; j < realItemCnt; j++)
			{
                if (j==0)
				{
                    sprintf(tmpBuf, "%3s %-9s %-5s %8s %9s\n", 
                        "", SysName[i], title[j], "0.0", "0.0");
                    strcat(condBuf, tmpBuf);
                }
				else 
				{
                    sprintf(tmpBuf, "%3s %-9s %-5s %8s %9s\n", 
                        "", "", title[j], "0.0", "0.0");
                    strcat(condBuf, tmpBuf);
                }
            }
            sprintf(tmpBuf, "    ==================================\n");
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

int doPeriodicHourLoad()
{
    if ( maskITEM[STMD_LOAD][STMD_HOUR] == MASK)
	{
		logPrint(trcLogId, FL, "LOAD HOUR Period Masking...TT\n");
        return -1;
	}

    if ( stmd_LoadSelect(STMD_HOUR, STM_STATISTIC_HOUR_LOAD_TBL_NAME) < 0 ) {
        sprintf(trcBuf, "LOAD HOUR Select Error\n");
        trclib_writeLogErr(FL, trcBuf);
        return -1;
    }

    return 1;
}

int doPeriodicDayLoad()
{
    if ( maskITEM[STMD_LOAD][STMD_DAY] == MASK)
	{
		logPrint(trcLogId, FL, "LOAD DAY Period Masking...TT\n");
        return -1;
	}

    if ( stmd_LoadSelect(STMD_DAY, STM_STATISTIC_DAY_LOAD_TBL_NAME) < 0 ) {
        sprintf(trcBuf, "LOAD DAY Select Error\n");
        trclib_writeLogErr(FL, trcBuf);
        return -1;
    }

    return 1;
}

int doPeriodicWeekLoad()
{
    if ( maskITEM[STMD_LOAD][STMD_WEEK] == MASK)
	{
		logPrint(trcLogId, FL, "LOAD WEEK Period Masking...TT\n");
        return -1;
	}

    if ( stmd_LoadSelect(STMD_WEEK, STM_STATISTIC_WEEK_LOAD_TBL_NAME) < 0 ) {
        sprintf(trcBuf, "LOAD WEEK Select Error\n");
        trclib_writeLogErr(FL, trcBuf);
        return -1;
    }

    return 1;
}

int doPeriodicMonthLoad()
{
    if ( maskITEM[STMD_LOAD][STMD_MONTH] == MASK)
	{
		logPrint(trcLogId, FL, "LOAD MONTH Period Masking...TT\n");
        return -1;
	}

    if ( stmd_LoadSelect(STMD_MONTH, STM_STATISTIC_MONTH_LOAD_TBL_NAME) < 0 ) {
        sprintf(trcBuf, "LOAD MONTH Select Error\n");
        trclib_writeLogErr(FL, trcBuf);
        return -1;
    }

    return 1;
}
