#include "stmd_proto.h"

extern  char    trcBuf[4096], trcTmp[1024];
extern  int     trcLogFlag, trcLogId;
extern  int     sysCnt;

extern  short   maskITEM  [STMD_MASK_ITEM_NUM][STMD_PERIOD_TYPE_NUM];
extern  STM_CommStatisticInfo    StatisticSystemInfo[SYSCONF_MAX_ASSO_SYS_NUM];
extern  MYSQL       sql, *conn;

extern  short   printTIME [STMD_PERIOD_TYPE_NUM];

int stmd_LogonSelect(int time_type, char *table_type)
{
    char        condBuf[4096], tmpBuf[1024];
    int         sts_code;
    char        str_time[10];
    int         select_cnt = 0, snd_cnt = 0;

    char        query[4096];
    MYSQL_RES   *result;
    MYSQL_ROW   row;

    time_t      now;
    char        tm[DIR_NUM][8];
    char        path[60];
    char        fileName[60];
    FILE        *fp;

    int         i, j, row_index, realSysCnt = 2, realItemCnt = 0;
    char title[7][16]={"SYSTEM", "SM_CH_ID", "LOG_MOD", "LOG_REQ", "LOG_SUCC","LOG_FAIL","SUCC_RATE"}; //uamyd 20110515 succrate_added
	char title1[5][16]={"", "HBIT_0","HBIT_1","HBIT_2", "HBIT_3"}; 
	char title2[5][16]={"", "HBIT_4","HBIT_5","HBIT_6", "HBIT_7"};
	char title3[5][16]={"", "HBIT_8","HBIT_9","HBIT_10", "HBIT_11"};
	char title4[5][16]={"", "HBIT_12","HBIT_13","HBIT_14", "HBIT_15"};
	char title5[5][16]={"", "HBIT_16","HBIT_17","HBIT_18", "HBIT_19"};
	char title6[5][16]={"", "HBIT_20","HBIT_21","HBIT_22", "HBIT_23"};
	char title7[5][16]={"", "HBIT_24","HBIT_25","HBIT_26", "HBIT_27"};
	char title8[5][16]={"", "HBIT_28","HBIT_29","HBIT_30", "HBIT_31"};
	char title10[5][16]={"", "SM_INT_ERR","OP_ERR","OP_TIMEOUT", "ETC_FAIL"};
	char title11[3][16]={"", "API_REQ_ERR","API_TIMEOUT"};
    char SysName[2][8] = {"SCMA","SCMB"};

    now = time(0);
    if(time_type != STMD_WEEK)
        now = now - (printTIME[time_type]*60) - STAT_OFFSET_UNIT;
    else
        now = now - (printTIME[time_type]*60);

    getFilePath (path, tm, &now); // ~/LOG/STAT 까지 만든다.
    makeDirectory (time_type,path,tm);

    sprintf(fileName, "%s", path );
    makeFileName ( fileName, STMD_LOGON, time_type, tm );

    if ( ( fp = fopen(fileName, APPEND ) ) == NULL ){
        sprintf(trcBuf, "%s Open Fail\n", fileName);
        trclib_writeLog(FL, trcBuf);
        return -1;
    }


    switch (time_type) {
        case    STMD_HOUR :
            sts_code = STSCODE_STM_PERIODIC_LOGON_HOUR;
            sprintf (str_time, "%s", STMD_STR_HOUR);
            break;
        case    STMD_DAY :
            sts_code = STSCODE_STM_PERIODIC_LOGON_DAY;
            sprintf (str_time, "%s", STMD_STR_DAY);
            break;
        case    STMD_WEEK :
            sts_code = STSCODE_STM_PERIODIC_LOGON_WEEK;
            sprintf (str_time, "%s", STMD_STR_WEEK);
            break;
        case    STMD_MONTH :
            sts_code = STSCODE_STM_PERIODIC_LOGON_MONTH;
            sprintf (str_time, "%s", STMD_STR_MONTH);
            break;
    }

    sprintf(condBuf,"    %s %s\n    S%04d LOGON PERIODIC STATISTICS MESSAGE\n",
        "SCM", // 현재는 OMP로 고정하지만 실질적인 시스템 이름으로 변경 필요 hslim_oper
        commlib_printTStamp(), // 현재시각 time stamp (년,월,일,시,분,초,요일)
        sts_code);

    sprintf(tmpBuf, "    PERIOD = %s\n    MEASURETIME = %s  -  %s\n\n",
        str_time,
        get_period_start_time(time_type), get_period_end_time(time_type));
    strcat(condBuf,tmpBuf);

    sprintf(tmpBuf, "    ====================================================================\n");
    strcat(condBuf, tmpBuf);
	sprintf(tmpBuf, "%3s %8s %12s %12s\n","",title[0],title[1],title[2] );
	strcat(condBuf, tmpBuf);
	sprintf(tmpBuf, "%3s %8s %12s %12s %12s %12s\n","","",title[3],title[4],title[5],title[6] ); //uamyd 20110515 succrate_added
	strcat(condBuf, tmpBuf);
	sprintf(tmpBuf, "%3s %8s %12s %12s %12s %12s\n", "","", title1[1], title1[2],title1[3],title1[4]);
	strcat(condBuf, tmpBuf);
	sprintf(tmpBuf, "%3s %8s %12s %12s %12s %12s\n", "","", title2[1], title2[2],title2[3],title2[4]);
	strcat(condBuf, tmpBuf);
	sprintf(tmpBuf, "%3s %8s %12s %12s %12s %12s\n", "","", title3[1], title3[2],title3[3],title3[4]);
	strcat(condBuf, tmpBuf);
	sprintf(tmpBuf, "%3s %8s %12s %12s %12s %12s\n", "","", title4[1], title4[2],title4[3],title4[4]);
	strcat(condBuf, tmpBuf);
	sprintf(tmpBuf, "%3s %8s %12s %12s %12s %12s\n", "","", title5[1], title5[2],title5[3],title5[4]);
	strcat(condBuf, tmpBuf);
	sprintf(tmpBuf, "%3s %8s %12s %12s %12s %12s\n", "","", title6[1], title6[2],title6[3],title6[4]);
	strcat(condBuf, tmpBuf);
	sprintf(tmpBuf, "%3s %8s %12s %12s %12s %12s\n", "","", title7[1], title7[2],title7[3],title7[4]);
	strcat(condBuf, tmpBuf);
	sprintf(tmpBuf, "%3s %8s %12s %12s %12s %12s\n", "","", title8[1], title8[2],title8[3],title8[4]);
	strcat(condBuf, tmpBuf);
	sprintf(tmpBuf, "%3s %8s %12s %12s %12s %12s\n", "","", title10[1], title10[2],title10[3],title10[4]);
	strcat(condBuf, tmpBuf);
	sprintf(tmpBuf, "%3s %8s %12s %12s\n", "","", title11[1], title11[2]);
	strcat(condBuf, tmpBuf);
    sprintf(tmpBuf, "    ====================================================================\n");
    strcat(condBuf, tmpBuf);

    for(i = 0; i < realSysCnt; i++)
	{
		realItemCnt = 1;

	   	sprintf(query, "SELECT system_name, sm_ch_id, log_mod, "
				" ifnull(sum(log_req), 0), ifnull(sum(log_succ), 0), ifnull(sum(log_fail), 0), "
				" ifnull(sum(HBIT_0), 0), ifnull(sum(HBIT_1), 0), ifnull(sum(HBIT_2), 0), "
				" ifnull(sum(HBIT_3), 0),  "
				" ifnull(sum(HBIT_4), 0), ifnull(sum(HBIT_5), 0), ifnull(sum(HBIT_6), 0), "
				" ifnull(sum(HBIT_7), 0),  "
				" ifnull(sum(HBIT_8), 0), ifnull(sum(HBIT_9), 0), ifnull(sum(HBIT_10), 0), "
				" ifnull(sum(HBIT_11), 0),  "
				" ifnull(sum(HBIT_12), 0), ifnull(sum(HBIT_13), 0), ifnull(sum(HBIT_14), 0), "
				" ifnull(sum(HBIT_15), 0),  "
				" ifnull(sum(HBIT_16), 0), ifnull(sum(HBIT_17), 0), ifnull(sum(HBIT_18), 0), "
				" ifnull(sum(HBIT_19), 0),  "
				" ifnull(sum(HBIT_20), 0), ifnull(sum(HBIT_21), 0), ifnull(sum(HBIT_22), 0), "
				" ifnull(sum(HBIT_23), 0),  "
				" ifnull(sum(HBIT_24), 0), ifnull(sum(HBIT_25), 0), ifnull(sum(HBIT_26), 0), "
				" ifnull(sum(HBIT_27), 0),  "
				" ifnull(sum(HBIT_28), 0), ifnull(sum(HBIT_29), 0), ifnull(sum(HBIT_30), 0), "
				" ifnull(sum(HBIT_31), 0),  "
				" ifnull(sum(sm_int_err),0), ifnull(sum(op_err),0), ifnull(sum(op_timeout),0), "
				" ifnull(sum(etc_fail),0), "
				" ifnull(sum(api_req_err),0), ifnull(sum(api_timeout),0) "
			   " from %s "
			   " where system_name = '%s' AND (stat_date = '%s') "
			   " group by system_name, sm_ch_id, log_mod ",
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

		if(strlen(condBuf)) {
			stmd_txMsg2Cond(condBuf, (sts_code - STSCODE_TO_MSGID_STATISTICS), 1, snd_cnt++);
			fprintf (fp, "%s",condBuf);
			memset(condBuf, 0x00, sizeof(condBuf));
		}

        while((row = mysql_fetch_row(result)) != NULL) 
		{
			row_index = 1;
			if (select_cnt == 0)
			{
				sprintf(tmpBuf, "%3s %8s SM_CH%7s %12s\n",
						"",SysName[i],row[row_index],!atoi(row[row_index+1])?"LOG_ON":"LOG_OUT");
				strcat(condBuf, tmpBuf);
			} 
			else 
			{
				sprintf(tmpBuf, "%3s %8s SM_CH%7s %12s\n",
						"","",row[row_index],!atoi(row[row_index+1])?"LOG_ON":"LOG_OUT");
				strcat(condBuf, tmpBuf);
			}

			sprintf(tmpBuf, "%3s %8s %12s %12s %12s %12.1f\n", //uamyd 20110515 succrate_added
					"", "", row[row_index+2],row[row_index+3],row[row_index+4],
					atoi(row[row_index+2])==0?0.0:(((float)atoi(row[row_index+3])/(float)atoi(row[row_index+2]))*100)); //uamyd 20110515 succrate_added
			strcat(condBuf, tmpBuf );

			for( j=5; j<34; j+=4 ){
				//j = 5,9,13,17,21,25,29,33
				sprintf(tmpBuf, "%3s %8s %12s %12s %12s %12s\n", 
						"", "", row[row_index+j],row[row_index+j+1],row[row_index+j+2],row[row_index+j+3]);
				strcat(condBuf, tmpBuf );
			}
			sprintf(tmpBuf, "%3s %8s %12s %12s %12s %12s\n", 
					"", "", row[row_index+37],row[row_index+38],row[row_index+39],row[row_index+40]);
			strcat(condBuf, tmpBuf);
			sprintf(tmpBuf, "%3s %8s %12s %12s\n",
					"", "", row[row_index+41],row[row_index+42]);
			strcat(condBuf, tmpBuf);

    		sprintf(tmpBuf, "    ====================================================================\n");
			strcat(condBuf, tmpBuf);
			select_cnt++;

			if(strlen(condBuf)) {
				stmd_txMsg2Cond(condBuf, (sts_code - STSCODE_TO_MSGID_STATISTICS), 1, snd_cnt++);
                fprintf (fp, "%s",condBuf);
                memset(condBuf, 0x00, sizeof(condBuf));
            }
        }
        mysql_free_result(result);

        if (select_cnt == 0) 
		{
			sprintf(tmpBuf, "%3s %8s %12s %12s\n", "",SysName[i], "-","-");
			strcat(condBuf, tmpBuf);
			sprintf(tmpBuf, "%3s %8s %12s %12s %12s %12s\n", "", "", "0","0","0","0"); //uamyd 20110515 succrate_added
			strcat(condBuf, tmpBuf);
			for(j=0;j<9;j++){
				sprintf(tmpBuf, "%3s %8s %12s %12s %12s %12s\n", "", "", "0","0","0","0");
				strcat(condBuf, tmpBuf );
			}
			sprintf(tmpBuf, "%3s %8s %12s %12s\n", 
					"", "", "0","0");
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
    
    stmd_txMsg2Cond(condBuf, (sts_code - STSCODE_TO_MSGID_STATISTICS), 0, snd_cnt);
    
    return 1;
}

int doPeriodicHourLogon()
{
    if ( maskITEM[STMD_LOGON][STMD_HOUR] == MASK)
	{
		logPrint(trcLogId, FL, "LOGON HOUR Period Masking...TT\n");
        return -1;
	}

    if ( stmd_LogonSelect(STMD_HOUR, STM_STATISTIC_HOUR_LOGON_TBL_NAME) < 0 ) {
        sprintf(trcBuf, "LOGON HOUR Select Error\n");
        trclib_writeLogErr(FL, trcBuf);
        return -1;
    }

    return 1;
}

int doPeriodicDayLogon()
{
    if ( maskITEM[STMD_LOGON][STMD_DAY] == MASK)
	{
		logPrint(trcLogId, FL, "LOGON DAY Period Masking...TT\n");
        return -1;
	}

    if ( stmd_LogonSelect(STMD_DAY, STM_STATISTIC_DAY_LOGON_TBL_NAME) < 0 ) {
        sprintf(trcBuf, "LOGON DAY Select Error\n");
        trclib_writeLogErr(FL, trcBuf);
        return -1;
    }

    return 1;
}

int doPeriodicWeekLogon()
{
    if ( maskITEM[STMD_LOGON][STMD_WEEK] == MASK)
	{
		logPrint(trcLogId, FL, "LOGON WEEK Period Masking...TT\n");
        return -1;
	}

    if ( stmd_LogonSelect(STMD_WEEK, STM_STATISTIC_WEEK_LOGON_TBL_NAME) < 0 ) {
        sprintf(trcBuf, "LOGON WEEK Select Error\n");
        trclib_writeLogErr(FL, trcBuf);
        return -1;
    }

    return 1;
}

int doPeriodicMonthLogon()
{
    if ( maskITEM[STMD_LOGON][STMD_MONTH] == MASK)
	{
		logPrint(trcLogId, FL, "LOGON MONTH Period Masking...TT\n");
        return -1;
	}

    if ( stmd_LogonSelect(STMD_MONTH, STM_STATISTIC_MONTH_LOGON_TBL_NAME) < 0 ) {
        sprintf(trcBuf, "LOGON MONTH Select Error\n");
        trclib_writeLogErr(FL, trcBuf);
        return -1;
    }

    return 1;
}
