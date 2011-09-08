#include "stmd_proto.h"

extern  char    trcBuf[4096], trcTmp[1024];
extern  int     trcLogFlag, trcLogId;
extern  int     sysCnt;

extern  short   maskITEM  [STMD_MASK_ITEM_NUM][STMD_PERIOD_TYPE_NUM];
extern  STM_CommStatisticInfo    StatisticSystemInfo[SYSCONF_MAX_ASSO_SYS_NUM];
extern  MYSQL       sql, *conn;

extern  short   printTIME [STMD_PERIOD_TYPE_NUM];

int stmd_SmsSelect(int time_type, char *table_type)
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

    int         i,j;
    char title[5][16]={"SYSTEM", "SMSC_IP", "REQ", "SUCC","FAIL"};
	char title1[6][16]={"", "", "SMPP_ERR","SVR_ERR","SMSC_ERR","ETC_ERR"};
    int row_index;
    char SysName[2][8] = {"SCMA", "SCMB"};
    int realSysCnt =2;
    int realItemCnt[2] = {0, 0};
	char smsc_ip[2][16];
	int	exist = 0;

    now = time(0);
    if(time_type != STMD_WEEK)
        now = now - (printTIME[time_type]*60) - STAT_OFFSET_UNIT;
    else
        now = now - (printTIME[time_type]*60);

    getFilePath (path, tm, &now); // ~/LOG/STAT 까지 만든다.
    makeDirectory (time_type,path,tm);

    sprintf(fileName,"%s", path );
    makeFileName ( fileName, STMD_SMS, time_type, tm );

    if ( ( fp = fopen(fileName, APPEND ) ) == NULL ){
        sprintf(trcBuf, "%s Open Fail\n", fileName);
        trclib_writeLog(FL, trcBuf);
        return -1;
    }

    switch (time_type) {
        case    STMD_HOUR :
            sts_code = STSCODE_STM_PERIODIC_SMS_HOUR;
            sprintf(str_time, "%s", STMD_STR_HOUR);
            break;
        case    STMD_DAY :
            sts_code = STSCODE_STM_PERIODIC_SMS_DAY;
            sprintf(str_time, "%s", STMD_STR_DAY);
            break;
        case    STMD_WEEK :
            sts_code = STSCODE_STM_PERIODIC_SMS_WEEK;
            sprintf (str_time, "%s", STMD_STR_WEEK);
            break;
        case    STMD_MONTH :
            sts_code = STSCODE_STM_PERIODIC_SMS_MONTH;
            sprintf (str_time, "%s", STMD_STR_MONTH);
            break;
    }

    sprintf(condBuf,"    %s %s\n    S%04d SMS PERIODIC STATISTICS MESSAGE\n",
        "SCM", // 현재는 OMP로 고정하지만 실질적인 시스템 이름으로 변경 필요 hslim_oper
        commlib_printTStamp(), // 현재시각 time stamp (년,월,일,시,분,초,요일)
        sts_code);

    sprintf(tmpBuf, "    PERIOD = %s\n    MEASURETIME = %s  -  %s\n\n",
        str_time,
        get_period_start_time(time_type), get_period_end_time(time_type));

    strcat(condBuf,tmpBuf);

    sprintf(tmpBuf, "    ====================================================================================\n");
    strcat(condBuf, tmpBuf);
    sprintf(tmpBuf, "%3s %8s %18s %12s %12s %12s\n","",title[0],title[1],title[2],title[3],title[4] );
    strcat(condBuf, tmpBuf);
    sprintf(tmpBuf, "%3s %8s %18s %12s %12s %12s %12s\n", "","", "", title1[2],title1[3],title1[4],title1[5]);
    strcat(condBuf, tmpBuf);
    sprintf(tmpBuf, "    ====================================================================================\n");
    strcat(condBuf, tmpBuf);

	memset(smsc_ip, 0x00, sizeof(smsc_ip));
    for(i = 0; i < realSysCnt; i++)
	{
	   sprintf(query, "SELECT code, count(*) from ip_code_tbl where type = 2 group by code");

		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);
            if(fp != NULL) fclose(fp);
            return -1;
	   }
	   result = mysql_store_result(conn);
	   row = mysql_fetch_row(result);
	   if( row == NULL )
	   {
		   exist = 0;
		   realItemCnt[i] = 0;
		   mysql_free_result(result);
		   continue;
	   }
	   else
	   {
		   exist = 1;
		   realItemCnt[i] = atoi(row[1]);
		   sprintf(smsc_ip[i], "%s", row[0]);
		   mysql_free_result(result);
	   }
	   
	   sprintf(query, "SELECT system_name, smsc_ip, "
			   " IFNULL(SUM(req), 0), IFNULL(SUM(succ), 0), IFNULL(SUM(fail), 0), "
			   " IFNULL(SUM(smpp_err), 0), " 
			   " IFNULL(SUM(svr_err), 0), IFNULL(SUM(smsc_err), 0), " 
			   " IFNULL(SUM(etc_err), 0) "
			   " from %s "
			   " where system_name = '%s' AND (stat_date = '%s') "
			   " group by system_name, smsc_ip ",
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

		logPrint(trcLogId, FL, "SMS Period query : %s\n", query);

        result = mysql_store_result(conn);

		select_cnt = 0;
        while((row = mysql_fetch_row(result)) != NULL) 
		{
            row_index = 2;
			if (select_cnt==0)
			{
				sprintf(tmpBuf, "%3s %8s %18s %12s %12s %12s\n",
						"", row[0], row[1], row[row_index],row[row_index+1],row[row_index+2]);
				strcat(condBuf, tmpBuf);
				sprintf(tmpBuf, "%3s %8s %18s %12s %12s %12s %12s\n",
						"", "", "", row[row_index+3],row[row_index+4],row[row_index+5],row[row_index+6]);
				strcat(condBuf, tmpBuf);
			} 
			else 
			{
				sprintf(tmpBuf, "%3s %8s %18s %12s %12s %12s\n",
						"", "", row[1], row[row_index],row[row_index+1],row[row_index+2]);
				strcat(condBuf, tmpBuf);
				sprintf(tmpBuf, "%3s %8s %18s %12s %12s %12s %12s\n",
						"", "", "", row[row_index+3], row[row_index+4],row[row_index+5],row[row_index+6] );
				strcat(condBuf, tmpBuf);
			}

    		sprintf(tmpBuf, "    ====================================================================================\n");
            strcat(condBuf, tmpBuf);

            select_cnt++;
        }
        mysql_free_result(result);

        if (select_cnt == 0) 
		{
			for( j = 0; j < realItemCnt[i]; j++ )
			{
				if (j==0)
				{
					sprintf(tmpBuf, "%3s %8s %18s %12s %12s %12s\n",
							"", SysName[i], smsc_ip[j], "0", "0", "0");
					strcat(condBuf, tmpBuf);
					sprintf(tmpBuf, "%3s %8s %18s %12s %12s %12s %12s\n",
							"", "", "", "0", "0", "0","0" );
					strcat(condBuf, tmpBuf);
				}
				else 
				{
					sprintf(tmpBuf, "%3s %8s %18s %12s %12s %12s\n",
							"", "", smsc_ip[j], "0", "0", "0");
					strcat(condBuf, tmpBuf);
					sprintf(tmpBuf, "%3s %8s %18s %12s %12s %12s %12s\n",
							"", "", "", "0", "0", "0","0" );
					strcat(condBuf, tmpBuf);
				}
			}
    		sprintf(tmpBuf, "    ====================================================================================\n");
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

int doPeriodicHourSms()
{
    if ( maskITEM[STMD_SMS][STMD_HOUR] == MASK)
	{
		logPrint(trcLogId, FL, "SMS HOUR Period Masking...TT\n");
        return -1;
	}

    if ( stmd_SmsSelect(STMD_HOUR, STM_STATISTIC_HOUR_SMS_TBL_NAME) < 0 ) {
        sprintf(trcBuf, "SMS HOUR Select Error\n");
        trclib_writeLogErr(FL, trcBuf);
        return -1;
    }

    return 1;
}

int doPeriodicDaySms()
{
    if ( maskITEM[STMD_SMS][STMD_DAY] == MASK)
	{
		logPrint(trcLogId, FL, "SMS DAY Period Masking...TT\n");
        return -1;
	}

    if ( stmd_SmsSelect(STMD_DAY, STM_STATISTIC_DAY_SMS_TBL_NAME) < 0 ) {
        sprintf(trcBuf, "SMS DAY Select Error\n");
        trclib_writeLogErr(FL, trcBuf);
        return -1;
    }

    return 1;
}

int doPeriodicWeekSms()
{
    if ( maskITEM[STMD_SMS][STMD_WEEK] == MASK)
	{
		logPrint(trcLogId, FL, "SMS WEEK Period Masking...TT\n");
        return -1;
	}

    if ( stmd_SmsSelect(STMD_WEEK, STM_STATISTIC_WEEK_SMS_TBL_NAME) < 0 ) {
        sprintf(trcBuf, "SMS WEEK Select Error\n");
        trclib_writeLogErr(FL, trcBuf);
        return -1;
    }

    return 1;
}

int doPeriodicMonthSms()
{
    if ( maskITEM[STMD_SMS][STMD_MONTH] == MASK)
	{
		logPrint(trcLogId, FL, "SMS MONTH Period Masking...TT\n");
        return -1;
	}

    if ( stmd_SmsSelect(STMD_MONTH, STM_STATISTIC_MONTH_SMS_TBL_NAME) < 0 ) {
        sprintf(trcBuf, "SMS MONTH Select Error\n");
        trclib_writeLogErr(FL, trcBuf);
        return -1;
    }

    return 1;
}
