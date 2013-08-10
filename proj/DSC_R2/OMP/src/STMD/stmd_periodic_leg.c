#include "stmd_proto.h"

extern  char    trcBuf[4096], trcTmp[1024];
extern  int     trcLogFlag, trcLogId;
extern  int     sysCnt;

extern  short   maskITEM  [STMD_MASK_ITEM_NUM][STMD_PERIOD_TYPE_NUM];
extern  STM_CommStatisticInfo    StatisticSystemInfo[SYSCONF_MAX_ASSO_SYS_NUM];
extern  MYSQL       sql, *conn;

extern  short   printTIME [STMD_PERIOD_TYPE_NUM];
extern PDSN_LIST         g_stPdsn;

int stmd_LegSelect(int time_type, char *table_type)
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
    char title[7][16]={"SYSTEM", "PDSN_IP", "RX_CNT", "START","INTERIM","DISCONNECT","STOP"};
	char title1[6][16]={"", "", "START_LOGON","INTERIM_LOGON","DISCONN_LOGON","LOG_OUT"};
    int row_index;
    char SysName[2][8] = {"SCMA", "SCMB"};
    int realSysCnt =2;
    int realItemCnt =0;
	int	exist = 0;

    int     snd_cnt = 1;
	

    now = time(0);
    if(time_type != STMD_WEEK)
        now = now - (printTIME[time_type]*60) - STAT_OFFSET_UNIT;
    else
        now = now - (printTIME[time_type]*60);

    getFilePath (path, tm, &now); // ~/LOG/STAT 까지 만든다.
    makeDirectory (time_type,path,tm);

    sprintf(fileName, "%s", path );
    makeFileName ( fileName, STMD_LEG, time_type, tm );
    if ( ( fp = fopen(fileName, APPEND ) ) == NULL ){
        sprintf(trcBuf, "%s Open Fail\n", fileName);
        trclib_writeLog(FL, trcBuf);
        return -1;
    }


    switch (time_type) {
        case    STMD_HOUR :
            sts_code = STSCODE_STM_PERIODIC_LEG_HOUR;
            sprintf(str_time, "%s", STMD_STR_HOUR);
            break;
        case    STMD_DAY :
            sts_code = STSCODE_STM_PERIODIC_LEG_DAY;
            sprintf(str_time, "%s", STMD_STR_DAY);
            break;
        case    STMD_WEEK :
            sts_code = STSCODE_STM_PERIODIC_LEG_WEEK;
            sprintf(str_time, "%s", STMD_STR_WEEK);
            break;
        case    STMD_MONTH :
            sts_code = STSCODE_STM_PERIODIC_LEG_MONTH;
            sprintf(str_time, "%s", STMD_STR_MONTH);
            break;
    }

    sprintf(condBuf,"    %s %s\n    S%04d ACCOUNT PERIODIC STATISTICS MESSAGE\n",
        "SCM", // 현재는 OMP로 고정하지만 실질적인 시스템 이름으로 변경 필요 hslim_oper
        commlib_printTStamp(), // 현재시각 time stamp (년,월,일,시,분,초,요일)
        sts_code);

    sprintf(tmpBuf, "    PERIOD = %s\n    MEASURETIME = %s  -  %s\n\n",
        str_time,
        get_period_start_time(time_type), get_period_end_time(time_type));
    strcat(condBuf,tmpBuf);

    sprintf(tmpBuf, "    =================================================================================\n");
    strcat(condBuf, tmpBuf);
    sprintf(tmpBuf, "%3s %8s %18s %12s %12s %12s %12s %12s\n","",title[0],title[1],title[2],title[3],title[4],title[5],title[6] );
    strcat(condBuf, tmpBuf);
    sprintf(tmpBuf, "%3s %8s %18s %12s %12s %12s %12s\n", "","", "", title1[2],title1[3],title1[4],title1[5]);
    strcat(condBuf, tmpBuf);
    sprintf(tmpBuf, "    =================================================================================\n");
    strcat(condBuf, tmpBuf);

    for(i = 0; i < realSysCnt; i++)
	{
	   sprintf(query, "SELECT count(*) from ip_code_tbl where type = 1");

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
		   realItemCnt = 0;
		   mysql_free_result(result);
		   continue;
	   }
	   else
	   {
		   exist = 1;
		   if(row[0] == NULL)
		   {
		   		mysql_free_result(result);
				continue;
		   }
		   else
		   {
			   realItemCnt = atoi(row[0]);
		   }
	   }
	   mysql_free_result(result); // MEM MUST Leak 10.07
	   
	   sprintf(query, "SELECT pdsn_ip, "
			   " IFNULL(SUM(rx_cnt), 0), IFNULL(SUM(start), 0), IFNULL(SUM(interim), 0), "
			   " IFNULL(SUM(disconnect),0), IFNULL(SUM(stop), 0), " 
			   " IFNULL(SUM(start_logon_cnt), 0), IFNULL(SUM(int_logon_cnt), 0), "
			   " IFNULL(SUM(disc_logon_cnt), 0), IFNULL(SUM(logout_cnt), 0) "
			   " from %s "
			   " where system_name = '%s' AND (stat_date = '%s') "
			   " group by pdsn_ip ",
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

		logPrint(trcLogId, FL, "ACCOUNT period query : %s\n",query);

		select_cnt = 0;
        result = mysql_store_result(conn);
        while((row = mysql_fetch_row(result)) != NULL) 
		{
			if(strlen(condBuf) > 3000) {
				strcat(condBuf, "\nCONTINUE\n");                                               
				stmd_txMsg2Cond (condBuf, (sts_code - STSCODE_TO_MSGID_STATISTICS), 1, snd_cnt++);
				fprintf (fp, "%s",condBuf);
				memset(condBuf, 0x00, sizeof(condBuf));
			}
			row_index = 1;
			if (select_cnt==0)
			{
				sprintf(tmpBuf, "%3s %8s %18s %12s %12s %12s %12s %12s\n",
						"", SysName[i], row[0], row[row_index],row[row_index+1],row[row_index+2],row[row_index+3],row[row_index+4] );
				strcat(condBuf, tmpBuf);
				sprintf(tmpBuf, "%3s %8s %18s %12s %12s %12s %12s\n",
						"", "", "", row[row_index+5],row[row_index+6],row[row_index+7],row[row_index+8]);
				strcat(condBuf, tmpBuf);
			} 
			else 
			{
				sprintf(tmpBuf, "%3s %8s %18s %12s %12s %12s %12s %12s\n",
						"", "", row[0], row[row_index],row[row_index+1],row[row_index+2],row[row_index+3],row[row_index+4]);
				strcat(condBuf, tmpBuf);
				sprintf(tmpBuf, "%3s %8s %18s %12s %12s %12s %12s\n",
						"", "", "", row[row_index+5],row[row_index+6],row[row_index+7],row[row_index+8]);
				strcat(condBuf, tmpBuf);
			}

			sprintf(tmpBuf, "    =================================================================================\n");
			strcat(condBuf, tmpBuf);

			select_cnt++;
		}
        mysql_free_result(result);

        if (select_cnt == 0) 
		{
			for(j=0; j < realItemCnt; j++ )
			{
				if (j==0)
				{
					sprintf(tmpBuf, "%3s %8s %18s %12s %12s %12s %12s %12s\n",
							"", SysName[i], g_stPdsn.stItem[j].ip, "0","0", "0", "0", "0");
					strcat(condBuf, tmpBuf);
					sprintf(tmpBuf, "%3s %8s %18s %12s %12s %12s %12s\n",
							"", "", "", "0", "0", "0", "0");
					strcat(condBuf, tmpBuf);
				}
				else 
				{
					sprintf(tmpBuf, "%3s %8s %18s %12s %12s %12s %12s %12s\n",
							"", "", g_stPdsn.stItem[j].ip, "0", "0", "0", "0", "0");
					strcat(condBuf, tmpBuf);
					sprintf(tmpBuf, "%3s %8s %18s %12s %12s %12s %12s\n",
							"", "", "", "0", "0", "0", "0");
					strcat(condBuf, tmpBuf);
				}

				if(strlen(condBuf) > 3000) {
					strcat(condBuf, "\nCONTINUE\n");                                               
					stmd_txMsg2Cond (condBuf, (sts_code - STSCODE_TO_MSGID_STATISTICS), 1, snd_cnt++);
					fprintf (fp, "%s",condBuf);
					memset(condBuf, 0x00, sizeof(condBuf));
				}
			}
    		sprintf(tmpBuf, "    =================================================================================\n");
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
    
    stmd_txMsg2Cond(condBuf, (sts_code - STSCODE_TO_MSGID_STATISTICS), 0, snd_cnt);
    
    return 1;
}

int doPeriodicHourLeg()
{
    if ( maskITEM[STMD_LEG][STMD_HOUR] == MASK)
	{
		logPrint(trcLogId, FL, "LEG HOUR Period Masking...TT\n");
        return -1;
	}

    if ( stmd_LegSelect(STMD_HOUR, STM_STATISTIC_HOUR_LEG_TBL_NAME) < 0 ) {
        sprintf(trcBuf, "LEG HOUR Select Error\n");
        trclib_writeLogErr(FL, trcBuf);
        return -1;
    }

    return 1;
}

int doPeriodicDayLeg()
{
    if ( maskITEM[STMD_LEG][STMD_DAY] == MASK)
	{
		logPrint(trcLogId, FL, "LEG DAY Period Masking...TT\n");
        return -1;
	}

    if ( stmd_LegSelect(STMD_DAY, STM_STATISTIC_DAY_LEG_TBL_NAME) < 0 ) {
        sprintf(trcBuf, "LEG DAY Select Error\n");
        trclib_writeLogErr(FL, trcBuf);
        return -1;
    }

    return 1;
}

int doPeriodicWeekLeg()
{
    if ( maskITEM[STMD_LEG][STMD_WEEK] == MASK)
	{
		logPrint(trcLogId, FL, "LEG WEEK Period Masking...TT\n");
        return -1;
	}

    if ( stmd_LegSelect(STMD_WEEK, STM_STATISTIC_WEEK_LEG_TBL_NAME) < 0 ) {
        sprintf(trcBuf, "LEG WEEK Select Error\n");
        trclib_writeLogErr(FL, trcBuf);
        return -1;
    }

    return 1;
}

int doPeriodicMonthLeg()
{
    if ( maskITEM[STMD_LEG][STMD_MONTH] == MASK)
	{
		logPrint(trcLogId, FL, "LEG MONTH Period Masking...TT\n");
        return -1;
	}

    if ( stmd_LegSelect(STMD_MONTH, STM_STATISTIC_MONTH_LEG_TBL_NAME) < 0 ) {
        sprintf(trcBuf, "LEG MONTH Select Error\n");
        trclib_writeLogErr(FL, trcBuf);
        return -1;
    }

    return 1;
}
