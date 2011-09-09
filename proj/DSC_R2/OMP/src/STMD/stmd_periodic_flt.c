#include "stmd_proto.h"

extern  char    trcBuf[4096], trcTmp[1024];
extern  int     trcLogFlag;
extern  int     sysCnt;

extern  short   maskITEM  [STMD_MASK_ITEM_NUM][STMD_PERIOD_TYPE_NUM];
extern  STM_CommStatisticInfo    StatisticSystemInfo[SYSCONF_MAX_ASSO_SYS_NUM];
extern  MYSQL       sql, *conn;

extern  short   printTIME [STMD_PERIOD_TYPE_NUM];

int stmd_FltSelect(int time_type, char *table_type1, char *table_type2)
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
    int         i,j;
    char        fileName[60];
    FILE        *fp;

    char title1[4][16]={"CPU","MEM","HW","PROC"};
//    char *title1[]={"HW","PROC","NET_NMS","CONN_NTP","CONN_NMS"};
//    char *title2[]={"DUP_HB","DUP_OOS", "RATE_WAP1", "RATE_WAP2","RATE_HTTP","RATE_UAWAP","RATE_AAA","RATE_VODS","RATE_ANAAA",
//                    "RATE_VT", "RATE_RADIUS", "NET_UAWAP","NET_AAA","CONN_UAWAP","CONN_NTP"};
    int row_index;
    char SysName[5][8];
    int realSysCnt =0;
    int realItemCnt =0;

    for(i=0; i<sysCnt; i++ ){
        sprintf(SysName[i], "%s", StatisticSystemInfo[i].sysName);
        realSysCnt++;
    }
    sprintf(SysName[realSysCnt], "TAPA");realSysCnt++;
    sprintf(SysName[realSysCnt], "TAPB");realSysCnt++;
    sprintf(SysName[realSysCnt], "SCEA");realSysCnt++;
    sprintf(SysName[realSysCnt], "SCEB");realSysCnt++;

    
    now = time(0);
    if(time_type != STMD_WEEK)
        now = now - (printTIME[time_type]*60) - STAT_OFFSET_UNIT;
    else 
        now = now - (printTIME[time_type]*60); 

    getFilePath (path, tm, &now);
    makeDirectory (time_type,path,tm);

    sprintf(fileName, "%s", path );
    makeFileName ( fileName, STMD_FAULT, time_type, tm );
    
    if ( ( fp = fopen(fileName, APPEND ) ) == NULL ){
        sprintf(trcBuf, "%s Open Fail\n", fileName);
        trclib_writeLog(FL, trcBuf);
        return -1;
    }
    switch (time_type) {
        case    STMD_HOUR :
            sts_code = STSCODE_STM_PERIODIC_FAULT_HOUR;
            sprintf(str_time, "%s", STMD_STR_HOUR); 
            break;
        case    STMD_DAY :
            sts_code = STSCODE_STM_PERIODIC_FAULT_DAY;
            sprintf(str_time, "%s", STMD_STR_DAY); 
            break;
        case    STMD_WEEK :
            sts_code = STSCODE_STM_PERIODIC_FAULT_WEEK;
            sprintf(str_time, "%s", STMD_STR_WEEK); 
            break;
        case    STMD_MONTH :
            sts_code = STSCODE_STM_PERIODIC_FAULT_MONTH;
            sprintf(str_time, "%s", STMD_STR_MONTH); 
            break;
    }
    sprintf(condBuf,"    %s %s\n    S%04d FAULT PERIODIC STATISTICS MESSAGE\n",
        //"BSDM", // 현재는 OMP로 고정하지만 실질적인 시스템 이름으로 변경 필요 hslim_oper
        "DSCM", // 현재는 OMP로 고정하지만 실질적인 시스템 이름으로 변경 필요 hslim_oper
        commlib_printTStamp(), // 현재시각 time stamp (년,월,일,시,분,초,요일)
        sts_code);

    sprintf(tmpBuf, "    PEROID = %s\n    MEASURETIME = %s  -  %s\n\n",
        str_time,
        get_period_start_time(time_type), get_period_end_time(time_type));
    strcat(condBuf,tmpBuf);

    sprintf(tmpBuf, "    ===============================================\n");
    strcat(condBuf, tmpBuf);
    sprintf(tmpBuf, "    SYSTEM  ITEM           MINOR    MAJOR  CRITICAL\n");
    strcat(condBuf, tmpBuf);
    sprintf(tmpBuf, "    ===============================================\n");
    strcat(condBuf, tmpBuf);

    for (i = 0; i < realSysCnt; i++) 
	{
        //if(!strcasecmp(SysName[i], "BSDM"))
        if(!strcasecmp(SysName[i], "DSCM"))
            realItemCnt=4;
//            realItemCnt=5;
        else if(!strcasecmp(SysName[i], "TAPA") || !strcasecmp(SysName[i], "TAPB") )
            realItemCnt=3;
        else if(!strcasecmp(SysName[i], "SCEA") || !strcasecmp(SysName[i], "SCEB") )
            realItemCnt=3;
        else
            realItemCnt=4;

		sprintf(query, "SELECT"
            " SUM(cpu_min_cnt),SUM(cpu_maj_cnt),SUM(cpu_cri_cnt), "
			" SUM(mem_min_cnt),SUM(mem_maj_cnt),SUM(mem_cri_cnt),"
            " SUM(etc_hw_min_cnt),SUM(etc_hw_maj_cnt),SUM(etc_hw_cri_cnt),"
            " SUM(proc_min_cnt),SUM(proc_maj_cnt),SUM(proc_cri_cnt) "
            " from %s "
            " where (stat_date = '%s') AND system_name = '%s' ",
            table_type1, get_period_select_time(time_type), SysName[i]);

        if ( trcLogFlag == TRCLEVEL_SQL ) 
		{
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

        select_cnt = 0;
		//   시간대의 통계가 생성이 안되어 있는 경우가 발생 가능함. 
        while((row = mysql_fetch_row(result)) != NULL && row[0] != NULL) 
		{
            row_index = 0;
            for(j=0;j<realItemCnt;j++)
			{
                if (j==0)
				{
		    		sprintf(tmpBuf, "%3s %-6s  %-12s %7s %8s %9s\n", 
                        "", SysName[i], title1[j], row[row_index], row[row_index+1], row[row_index+2]);
                    strcat(condBuf, tmpBuf);
				} 
				else 
				{
		    		sprintf(tmpBuf, "%3s %-6s  %-12s %7s %8s %9s\n", 
                        "", "", title1[j], row[row_index], row[row_index+1], row[row_index+2]);
                    strcat(condBuf, tmpBuf);
				}
                row_index += 3;
            }
            select_cnt++;
        }
        mysql_free_result(result);

        if (select_cnt == 0) 
		{
            for(j=0;j<realItemCnt;j++)
			{
                if (j==0)
				{
		    		sprintf(tmpBuf, "%3s %-6s  %-12s %7s %8s %9s\n", 
                        "", SysName[i], title1[j], "0", "0", "0");
                    strcat(condBuf, tmpBuf);
				} 
				else 
				{
		    		sprintf(tmpBuf, "%3s %-6s  %-12s %7s %8s %9s\n", 
                        "", "", title1[j], "0", "0", "0");
                    strcat(condBuf, tmpBuf);
				}
            }

        }
        
        //if ( !strcasecmp(SysName[i], "BSDA") || !strcasecmp(SysName[i], "BSDB") ){
        //if ( !strcasecmp(SysName[i], "DSCA") || !strcasecmp(SysName[i], "DSCB") )
        if ( !strcasecmp(SysName[i], "SCMA") || !strcasecmp(SysName[i], "SCMB") )
		{
#if 0 // jjinri
            sprintf(query, "SELECT"
                " IFNULL(SUM(dup_hb_min_cnt), 0), IFNULL(SUM(dup_hb_maj_cnt), 0), IFNULL(SUM(dup_hb_cri_cnt), 0),  "
                " IFNULL(SUM(dup_oos_min_cnt), 0), IFNULL(SUM(dup_oos_maj_cnt), 0), IFNULL(SUM(dup_oos_cri_cnt), 0),  "
                " IFNULL(SUM(succ_wap1ana_min_cnt), 0), IFNULL(SUM(succ_wap1ana_maj_cnt), 0), IFNULL(SUM(succ_wap1ana_cri_cnt), 0),"
                " IFNULL(SUM(succ_wap2ana_min_cnt), 0), IFNULL(SUM(succ_wap2ana_maj_cnt), 0), IFNULL(SUM(succ_wap2ana_cri_cnt), 0),  "
                " IFNULL(SUM(succ_httpana_min_cnt), 0), IFNULL(SUM(succ_httpana_maj_cnt), 0), IFNULL(SUM(succ_httpana_cri_cnt), 0),  "
                " IFNULL(SUM(succ_uawap_min_cnt), 0), IFNULL(SUM(succ_uawap_maj_cnt), 0), IFNULL(SUM(succ_uawap_cri_cnt), 0),"
                " IFNULL(SUM(succ_aaa_min_cnt), 0), IFNULL(SUM(succ_aaa_maj_cnt), 0), IFNULL(SUM(succ_aaa_cri_cnt), 0),  "
                " IFNULL(SUM(succ_vods_min_cnt), 0), IFNULL(SUM(succ_vods_maj_cnt), 0), IFNULL(SUM(succ_vods_cri_cnt), 0),  "
                " IFNULL(SUM(succ_anaaa_min_cnt), 0), IFNULL(SUM(succ_anaaa_maj_cnt), 0), IFNULL(SUM(succ_anaaa_cri_cnt), 0),  "
               	" IFNULL(SUM(succ_vt_min_cnt), 0), IFNULL(SUM(succ_vt_maj_cnt), 0), IFNULL(SUM(succ_vt_cri_cnt), 0),  " 
	        	" IFNULL(SUM(succ_radius_min_cnt), 0), IFNULL(SUM(succ_radius_maj_cnt), 0), IFNULL(SUM(succ_radius_cri_cnt), 0),  "	
				" IFNULL(SUM(net_uawap_min_cnt), 0), IFNULL(SUM(net_uawap_maj_cnt), 0), IFNULL(SUM(net_uawap_cri_cnt), 0),"
                " IFNULL(SUM(net_aaa_min_cnt), 0), IFNULL(SUM(net_aaa_maj_cnt), 0), IFNULL(SUM(net_aaa_cri_cnt), 0),  "
                " IFNULL(SUM(sess_uawap_min_cnt), 0), IFNULL(SUM(sess_uawap_maj_cnt), 0), IFNULL(SUM(sess_uawap_cri_cnt), 0), "
                " IFNULL(SUM(sess_ntp_min_cnt), 0), IFNULL(SUM(sess_ntp_maj_cnt), 0), IFNULL(SUM(sess_ntp_cri_cnt), 0)  "
                " from %s "
                " where (stat_date = '%s') AND system_name = '%s' ",
                table_type2, get_period_select_time(time_type), SysName[i]);

            if ( trcLogFlag == TRCLEVEL_SQL ) 
			{
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
            select_cnt=0;
            while((row = mysql_fetch_row(result)) != NULL) 
			{
                row_index = 0;
                for(j=0;j<15;j++)
				{
		    		sprintf(tmpBuf, "%3s %-6s  %-12s %7s %8s %9s\n", 
                        "", "", title2[j], row[row_index], row[row_index+1], row[row_index+2]);
                    strcat(condBuf, tmpBuf);
                    row_index += 3;
				}
                select_cnt++;
            }

            mysql_free_result(result);

            if (select_cnt == 0) 
			{
                for(j=0;j<15;j++)
				{
		    		sprintf(tmpBuf, "%3s %-6s  %-12s %7s %8s %9s\n", 
                        "", "", title2[j], "0", "0", "0");
                    strcat(condBuf, tmpBuf);
				}
            }
#endif
        }
        sprintf(tmpBuf, "    ===============================================\n");
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

int doPeriodicHourFlt()
{
    if ( maskITEM[STMD_FAULT][STMD_HOUR] == MASK)
        return -1;

    if ( stmd_FltSelect(STMD_HOUR, STM_STATISTIC_HOUR_FAULT_TBL_NAME, STM_STATISTIC_HOUR_BSD_FLT_TBL_NAME) < 0 ) 
    {
        sprintf(trcBuf, "FAULT HOUR Select Error\n");
        trclib_writeLogErr(FL, trcBuf);
        return -1;
    }

    return 1;
}

int doPeriodicDayFlt()
{
    if ( maskITEM[STMD_FAULT][STMD_DAY] == MASK)
        return -1;

    if ( stmd_FltSelect(STMD_DAY, STM_STATISTIC_DAY_FAULT_TBL_NAME, STM_STATISTIC_DAY_BSD_FLT_TBL_NAME) < 0 ) 
    {
        sprintf(trcBuf, "FAULT DAY Select Error\n");
        trclib_writeLogErr(FL, trcBuf);
        return -1;
    }

    return 1;
}

int doPeriodicWeekFlt()
{
    if ( maskITEM[STMD_FAULT][STMD_WEEK] == MASK)
        return -1;

    if ( stmd_FltSelect(STMD_WEEK, STM_STATISTIC_WEEK_FAULT_TBL_NAME, STM_STATISTIC_WEEK_BSD_FLT_TBL_NAME) < 0 ) 
    {
        sprintf(trcBuf, "FAULT WEEK Select Error\n");
        trclib_writeLogErr(FL, trcBuf);
        return -1;
    }

    return 1;
}

int doPeriodicMonthFlt()
{
    if ( maskITEM[STMD_FAULT][STMD_MONTH] == MASK)
        return -1;

    if ( stmd_FltSelect(STMD_MONTH, STM_STATISTIC_MONTH_FAULT_TBL_NAME, STM_STATISTIC_MONTH_BSD_FLT_TBL_NAME) < 0 ) 
    {
        sprintf(trcBuf, "FAULT MONTH Select Error\n");
        trclib_writeLogErr(FL, trcBuf);
        return -1;
    }

    return 1;
}
