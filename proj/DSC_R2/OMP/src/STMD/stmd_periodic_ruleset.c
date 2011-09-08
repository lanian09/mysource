#include "stmd_proto.h"

extern  char    trcBuf[4096*4], trcTmp[4096];
extern  int     trcLogFlag, trcLogId;
extern  int     sysCnt;

extern  short   maskITEM  [STMD_MASK_ITEM_NUM][STMD_PERIOD_TYPE_NUM];
extern  STM_CommStatisticInfo    StatisticSystemInfo[SYSCONF_MAX_ASSO_SYS_NUM];
extern  MYSQL       sql, *conn;
extern SCE_t    	g_stSCE[2];
extern int          SCE_CNT;

extern  short   printTIME [STMD_PERIOD_TYPE_NUM];
extern char rsFname[32];
extern RuleSetList	g_stSCERule[MAX_SCE_NUM];
extern	int			g_ruleIdBuf[MAX_RULE_NUM];
extern	int			g_ruleItemCnt;


int stmd_RuleSetSelect(int time_type, char *table_type)
{
    char        condBuf[4096], tmpBuf[1024];
    int         sts_code;
    char        str_time[10];
	int			snd_cnt = 0;

    char        query[4096], query_head[4096]; 
    MYSQL_RES   *result;
    MYSQL_ROW   row;

    time_t      now;
    char        tm[DIR_NUM][8];
    char        path[60];
    char        fileName[60];
    FILE        *fp;

    int         i,j;
	char title[4][16] = {"SYSTEM", "RULE SET", "Session", "Block"};
	char title1[5][16] = {"", "", "UpStream(Mbps)", "DnStream(Mbps)", "Total(Mbps)"};
	char title2[5][16] = {"", "", "UpByte(MBytes)", "DnByte(MBytes)", "Total(MBytes)"};

    int row_index;
    char SysName[2][8];
    int realSysCnt =0;
    int realItemCnt =0, row_cnt=0, index;

    for(i=0; i<SCE_CNT; i++ )
	{
        sprintf(SysName[i], "%s", g_stSCE[i].sce_name);
        realSysCnt++;
    }

    now = time(0);
    if(time_type != STMD_WEEK)
        now = now - (printTIME[time_type]*60) - STAT_OFFSET_UNIT;
    else
        now = now - (printTIME[time_type]*60);

    getFilePath (path, tm, &now); // ~/LOG/STAT 까지 만든다.
    makeDirectory (time_type,path,tm);

    sprintf (fileName, "%s", path );
    makeFileName ( fileName, STMD_RULE_SET, time_type, tm );

    if ( ( fp = fopen(fileName, APPEND ) ) == NULL )
	{
        sprintf(trcBuf, "%s Open Fail\n", fileName);
        trclib_writeLog(FL, trcBuf);
        return -1;
    }

	//char title[4][16] = {"SYSTEM", "RULE SET", "Session", "Block"};
	//char title1[5][16] = {"", "", "UpStream(Mbps)", "DnStream(Mbps)", "Total(Mbps)"};
	//char title2[5][16] = {"", "", "UpByte(MBytes)", "DnByte(MBytes)", "Total(MBytes)"};
    switch (time_type) {
        case    STMD_HOUR :
            sts_code = STSCODE_STM_PERIODIC_RULESET_HOUR;
            sprintf(str_time, "%s", STMD_STR_HOUR);
			sprintf(query_head, "SELECT pkg_id, rule_set_id, rule_set_name, "
					" IFNULL(SUM(session),0), "
					" IFNULL(SUM(block_cnt),0), "
					" IFNULL(ROUND(SUM(upstream_volume)*8/1024/1024/3600,3),0), "
					" IFNULL(ROUND(SUM(downstream_volume)*8/1024/1024/3600,3),0), "
					" ROUND((IFNULL(SUM(upstream_volume),0) + IFNULL(SUM(downstream_volume),0))*8/1024/1024/3600, 3), "
					" IFNULL(ROUND(SUM(upstream_volume)/1024/1024,3),0), "
					" IFNULL(ROUND(SUM(downstream_volume)/1024/1024,3),0), "
					" ROUND((IFNULL(SUM(upstream_volume),0) + IFNULL(SUM(downstream_volume),0))/1024/1024, 3) "
                	" from %s ", table_type);
            break;
        case    STMD_DAY :
            sts_code = STSCODE_STM_PERIODIC_RULESET_DAY;
            sprintf(str_time, "%s", STMD_STR_DAY);
			sprintf(query_head, "SELECT pkg_id, rule_set_id, rule_set_name, "
					" IFNULL(SUM(session),0), "
					" IFNULL(SUM(block_cnt),0), "
					" IFNULL(ROUND(SUM(upstream_volume)*8/1024/1024/86400,3),0), "
					" IFNULL(ROUND(SUM(downstream_volume)*8/1024/1024/86400,3),0), "
					" ROUND((IFNULL(SUM(upstream_volume),0) + IFNULL(SUM(downstream_volume),0))*8/1024/1024/86400, 3), "
					" IFNULL(ROUND(SUM(upstream_volume)/1024/1024,3),0), "
					" IFNULL(ROUND(SUM(downstream_volume)/1024/1024,3),0), "
					" ROUND((IFNULL(SUM(upstream_volume),0) + IFNULL(SUM(downstream_volume),0))/1024/1024, 3) "
                	" from %s ", table_type);
            break;
        case    STMD_WEEK :
            sts_code = STSCODE_STM_PERIODIC_RULESET_WEEK;
            sprintf(str_time, "%s", STMD_STR_WEEK);
			sprintf(query_head, "SELECT pkg_id, rule_set_id, rule_set_name, "
					" IFNULL(SUM(session),0), "
					" IFNULL(SUM(block_cnt),0), "
					" ROUND(IFNULL(SUM(upstream_volume)*8/1024/1024/(86400*7),0),3), "
					" ROUND(IFNULL(SUM(downstream_volume)*8/1024/1024/(86400*7),0),3),  "
					" ROUND((IFNULL(SUM(upstream_volume),0) + IFNULL(SUM(downstream_volume),0))*8/1024/1024/(86400*7), 3), "
					" ROUND(IFNULL(SUM(upstream_volume)/1024/1024,0),3), "
					" ROUND(IFNULL(SUM(downstream_volume)/1024/1024,0),3), "
					" ROUND( (IFNULL(SUM(upstream_volume),0) + IFNULL(SUM(downstream_volume),0))/1024/1024, 3)  "
                	" from %s ", table_type);
            break;
        case    STMD_MONTH :
            sts_code = STSCODE_STM_PERIODIC_RULESET_MONTH;
            sprintf(str_time, "%s", STMD_STR_MONTH);
			sprintf(query_head, "SELECT pkg_id, rule_set_id, rule_set_name, "
					" IFNULL(SUM(session),0), "
					" IFNULL(SUM(block_cnt),0), "
					" ROUND(IFNULL(SUM(upstream_volume)*8/1024/1024/(86400*30),0),3), "
					" ROUND(IFNULL(SUM(downstream_volume)*8/1024/1024/(86400*30),0),3),  "
					" ROUND((IFNULL(SUM(upstream_volume),0) + IFNULL(SUM(downstream_volume),0))*8/1024/1024/(86400*30), 3), "
					" ROUND(IFNULL(SUM(upstream_volume)/1024/1024,0),3), "
					" ROUND(IFNULL(SUM(downstream_volume)/1024/1024,0),3), "
					" ROUND( (IFNULL(SUM(upstream_volume),0) + IFNULL(SUM(downstream_volume),0))/1024/1024, 3)  "
                	" from %s ", table_type);
            break;
    }

    sprintf(condBuf,"    %s %s\n    S%04d RULE SET PERIODIC STATISTICS MESSAGE\n",
        "SCE", // 현재는 OMP로 고정하지만 실질적인 시스템 이름으로 변경 필요 hslim_oper
        commlib_printTStamp(), // 현재시각 time stamp (년,월,일,시,분,초,요일)
        sts_code);

    sprintf(tmpBuf, "    PERIOD = %s\n    MEASURETIME = %s  -  %s\n\n",
        str_time,
        get_period_start_time(time_type), get_period_end_time(time_type));

    strcat(condBuf,tmpBuf);

	sprintf(tmpBuf, "    =======================================================================================\n");
	strcat(condBuf, tmpBuf);
	sprintf(tmpBuf, "%3s %6s %-28s %15s %15s \n","",title[0],title[1],title[2],title[3]);
	strcat(condBuf, tmpBuf);
	sprintf(tmpBuf, "%3s %6s %-28s %15s %15s %15s\n","","","",title1[2],title1[3],title1[4]);
	strcat(condBuf, tmpBuf);
	sprintf(tmpBuf, "%3s %6s %-28s %15s %15s %15s\n","","","",title2[2],title2[3],title2[4]);
	strcat(condBuf, tmpBuf);
	sprintf(tmpBuf, "    =======================================================================================\n");
	strcat(condBuf, tmpBuf);

    for(i = 0; i < realSysCnt; i++)
	{
        sprintf(query, "%s  "
                " where record_source = '%s' AND stat_date = '%s' "
				" group by pkg_id, rule_set_id, rule_set_name "
				" order by pkg_id, rule_set_id, rule_set_name ", query_head,
            	SysName[i], get_period_select_time(time_type));


		realItemCnt = g_stSCERule[i].ruleSetCnt;

		logPrint(trcLogId, FL, "periodic query: %s \n",query);

        if ( trcLogFlag == TRCLEVEL_SQL ) 
		{
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }
		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; query:%s, err=%s\n", query,mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);
            if(fp != NULL) fclose(fp);
            return -1;
        }

        row_cnt = 0;                                                                                              
        result = mysql_store_result(conn);
        while( (row = mysql_fetch_row(result)) != NULL)
        {
            row_index = 3;
			if( row_cnt == 0 )
			{
				sprintf(tmpBuf, "%3s %6s %-28s %15s %15s\n",
						"",SysName[i], row[2], row[row_index],row[row_index+1]);
				strcat(condBuf, tmpBuf);
				sprintf(tmpBuf, "%3s %6s %-28s %15s %15s %15s\n",
						"","", "",row[row_index+2],row[row_index+3],row[row_index+4]);
				strcat(condBuf, tmpBuf);
				sprintf(tmpBuf, "%3s %6s %-28s %15s %15s %15s\n",
						"","", "",row[row_index+5],row[row_index+6],row[row_index+7]);
				strcat(condBuf, tmpBuf);
			}
			else
			{
				sprintf(tmpBuf, "%3s %6s %-28s %15s %15s \n",
						"","", row[2],row[row_index],row[row_index+1]);
				strcat(condBuf, tmpBuf);
				sprintf(tmpBuf, "%3s %6s %-28s %15s %15s %15s\n",
						"","", "",row[row_index+2],row[row_index+3],row[row_index+4]);
				strcat(condBuf, tmpBuf);
				sprintf(tmpBuf, "%3s %6s %-28s %15s %15s %15s\n",
						"","", "",row[row_index+5],row[row_index+6],row[row_index+7]);
				strcat(condBuf, tmpBuf);
			}
			if(strlen(condBuf) > 3000) {
	            strcat(condBuf, "\nCONTINUE\n");
				stmd_txMsg2Cond (condBuf, (sts_code - STSCODE_TO_MSGID_STATISTICS), 1, snd_cnt);
				fprintf (fp, "%s",condBuf);
				memset(condBuf, 0x00, sizeof(condBuf));
			}
			row_cnt++;
        }
        mysql_free_result(result);

        if( row_cnt == 0 ) // no query result set
        {	
            for( j = 0; j < realItemCnt; j++ )
            {
                index = g_ruleIdBuf[j];
				if( j == 0 )
				{
					sprintf(tmpBuf, "%3s %6s %-28s %15s %15s \n",
							"", SysName[i], g_stSCERule[i].stRule[index].rName, "0", "0");
					strcat(condBuf, tmpBuf);
					sprintf(tmpBuf, "%3s %6s %-28s %15s %15s %15s\n",
							"", "", "", "0.000", "0.000", "0.000");
					strcat(condBuf, tmpBuf);                                                             
					sprintf(tmpBuf, "%3s %6s %-28s %15s %15s %15s\n",
							"", "", "", "0.000","0.000","0.000");
					strcat(condBuf, tmpBuf);
					sprintf(tmpBuf, "    --------------------------------------------------------------------------------------\n");
					strcat(condBuf, tmpBuf);
				}
				else
				{
					sprintf(tmpBuf, "%3s %6s %-28s %15s %15s \n",
							"", "", g_stSCERule[i].stRule[index].rName, "0","0");
					strcat(condBuf, tmpBuf);
					sprintf(tmpBuf, "%3s %6s %-28s %15s %15s %15s\n",
							"", "", "", "0.000","0.000","0.000");
					strcat(condBuf, tmpBuf);                                                             
					sprintf(tmpBuf, "%3s %6s %-28s %15s %15s %15s\n",
							"", "", "", "0.000","0.000","0.000");
					strcat(condBuf, tmpBuf);
					sprintf(tmpBuf, "    ---------------------------------------------------------------------------------------\n");
				}
				if(strlen(condBuf) > 3000) {
					strcat(condBuf, "\nCONTINUE\n");
					stmd_cron_txMsg2Cond (condBuf, (sts_code - STSCODE_TO_MSGID_STATISTICS), 1, snd_cnt);
					fprintf (fp, "%s",condBuf);
					memset(condBuf, 0x00, sizeof(condBuf));
				}
            }
        }
		sprintf(tmpBuf, "    =======================================================================================\n");
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

int doPeriodicHourRuleSet()
{
    if ( maskITEM[STMD_RULE_SET][STMD_HOUR] == MASK)
	{
		logPrint(trcLogId, FL, "Rule Set HOUR Period Masking...TT\n");
        return -1;
	}

    if ( stmd_RuleSetSelect(STMD_HOUR, STM_STATISTIC_HOUR_RULESET_TBL_NAME) < 0 ) {
        sprintf(trcBuf, "Rule Set HOUR Select Error\n");
        trclib_writeLogErr(FL, trcBuf);
        return -1;
    }

//fprintf(stderr, "ppp rule set hour\n");
    return 1;
}

int doPeriodicDayRuleSet()
{
    if ( maskITEM[STMD_RULE_SET][STMD_DAY] == MASK)
	{
		logPrint(trcLogId, FL, "Rule Set DAY Period Masking...TT\n");
        return -1;
	}

    if ( stmd_RuleSetSelect(STMD_DAY, STM_STATISTIC_DAY_RULESET_TBL_NAME) < 0 ) {
        sprintf(trcBuf, "Rule Set DAY Select Error\n");
        trclib_writeLogErr(FL, trcBuf);
        return -1;
    }

    return 1;
}

int doPeriodicWeekRuleSet()
{
    if ( maskITEM[STMD_RULE_SET][STMD_WEEK] == MASK)
	{
		logPrint(trcLogId, FL, "Rule Set WEEK Period Masking...TT\n");
        return -1;
	}

    if ( stmd_RuleSetSelect(STMD_WEEK, STM_STATISTIC_WEEK_RULESET_TBL_NAME) < 0 ) {
        sprintf(trcBuf, "Rule Set WEEK Select Error\n");
        trclib_writeLogErr(FL, trcBuf);
        return -1;
    }

    return 1;
}

int doPeriodicMonthRuleSet()
{
    if ( maskITEM[STMD_RULE_SET][STMD_MONTH] == MASK)
	{
		logPrint(trcLogId, FL, "Rule Set MONTH Period Masking...TT\n");
        return -1;
	}

    if ( stmd_RuleSetSelect(STMD_MONTH, STM_STATISTIC_MONTH_RULESET_TBL_NAME) < 0 ) {
        sprintf(trcBuf, "Rule Set MONTH Select Error\n");
        trclib_writeLogErr(FL, trcBuf);
        return -1;
    }

    return 1;
}

