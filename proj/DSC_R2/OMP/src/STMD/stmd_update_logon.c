#include "stmd_proto.h"

extern  char    trcBuf[4096], trcTmp[1024];
extern  int     trcLogFlag ;
extern  MYSQL   sql, *conn;
extern short	delTIME[STMD_PERIOD_TYPE_NUM];

int stmd_LogOnUpdate(int time_type, char *table_type, MYSQL_ROW inrow)
{
    char        query[4096*2];
    char        starttime[32],endtime[32],seltbl[32],sysname[12];
	int         sm_ch_id, log_mod;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

	memset (seltbl, 0x00, sizeof(seltbl));
	sprintf(sysname, "%s", inrow[0]);
	sm_ch_id = atoi(inrow[1]);
	log_mod  = atoi(inrow[2]);
    sprintf(starttime, "%s", get_select_time2(time_type));
    sprintf(endtime, "%s", get_period_end_time2(time_type));    

    switch(time_type)
    {
		case STMD_HOUR:
			strcpy(seltbl, STM_STATISTIC_5MINUTE_LOGON_TBL_NAME);
			break;
		case STMD_DAY:
			strcpy(seltbl, STM_STATISTIC_HOUR_LOGON_TBL_NAME);
			break;
		case STMD_WEEK:
			strcpy(seltbl, STM_STATISTIC_DAY_LOGON_TBL_NAME);
			break;
		case STMD_MONTH:
			strcpy(seltbl, STM_STATISTIC_DAY_LOGON_TBL_NAME);
			break;
		default:
			strcpy(seltbl, STM_STATISTIC_HOUR_LOGON_TBL_NAME);
			break;
    }

    if (time_type == STMD_HOUR || time_type == STMD_DAY){
    	sprintf(query, "SELECT "
                " IFNULL(SUM(log_req),0), "
                " IFNULL(SUM(log_succ),0), "
                " IFNULL(SUM(log_fail),0), "
                " IFNULL(SUM(HBIT_0),0), "
                " IFNULL(SUM(HBIT_1),0), "
                " IFNULL(SUM(HBIT_2),0), "
                " IFNULL(SUM(HBIT_3),0), "
                " IFNULL(SUM(HBIT_4),0), "
                " IFNULL(SUM(HBIT_5),0), "
                " IFNULL(SUM(HBIT_6),0), "
                " IFNULL(SUM(HBIT_7),0), "
                " IFNULL(SUM(HBIT_8),0), "
                " IFNULL(SUM(HBIT_9),0), "
                " IFNULL(SUM(HBIT_10),0), "
                " IFNULL(SUM(HBIT_11),0), "
                " IFNULL(SUM(HBIT_12),0), "
                " IFNULL(SUM(HBIT_13),0), "
                " IFNULL(SUM(HBIT_14),0), "
                " IFNULL(SUM(HBIT_15),0), "
                " IFNULL(SUM(HBIT_16),0), "
                " IFNULL(SUM(HBIT_17),0), "
                " IFNULL(SUM(HBIT_18),0), "
                " IFNULL(SUM(HBIT_19),0), "
                " IFNULL(SUM(HBIT_20),0), "
                " IFNULL(SUM(HBIT_21),0), "
                " IFNULL(SUM(HBIT_22),0), "
                " IFNULL(SUM(HBIT_23),0), "
                " IFNULL(SUM(HBIT_24),0), "
                " IFNULL(SUM(HBIT_25),0), "
                " IFNULL(SUM(HBIT_26),0), "
                " IFNULL(SUM(HBIT_27),0), "
                " IFNULL(SUM(HBIT_28),0), "
                " IFNULL(SUM(HBIT_29),0), "
                " IFNULL(SUM(HBIT_30),0), "
                " IFNULL(SUM(HBIT_31),0), "
                " IFNULL(SUM(SM_INT_ERR),0), "
                " IFNULL(SUM(OP_ERR),0), "
                " IFNULL(SUM(OP_TIMEOUT),0), "
                " IFNULL(SUM(ETC_FAIL),0), "
                " IFNULL(SUM(API_REQ_ERR),0), "
                " IFNULL(SUM(API_TIMEOUT),0), "
                " IFNULL(SUM(stat_cnt),0) "
                " FROM %s WHERE system_name = '%s' "
				" and sm_ch_id = %d and log_mod = %d "
				" and (stat_date > '%s' and stat_date <= '%s')",
                seltbl, sysname, sm_ch_id, log_mod, starttime, endtime);
    }else{
	sprintf(query, "SELECT "
                " IFNULL(SUM(log_req),0), "
                " IFNULL(SUM(log_succ),0), "
                " IFNULL(SUM(log_fail),0), "
                " IFNULL(SUM(HBIT_0),0), "
                " IFNULL(SUM(HBIT_1),0), "
                " IFNULL(SUM(HBIT_2),0), "
                " IFNULL(SUM(HBIT_3),0), "
                " IFNULL(SUM(HBIT_4),0), "
                " IFNULL(SUM(HBIT_5),0), "
                " IFNULL(SUM(HBIT_6),0), "
                " IFNULL(SUM(HBIT_7),0), "
                " IFNULL(SUM(HBIT_8),0), "
                " IFNULL(SUM(HBIT_9),0), "
                " IFNULL(SUM(HBIT_10),0), "
                " IFNULL(SUM(HBIT_11),0), "
                " IFNULL(SUM(HBIT_12),0), "
                " IFNULL(SUM(HBIT_13),0), "
                " IFNULL(SUM(HBIT_14),0), "
                " IFNULL(SUM(HBIT_15),0), "
                " IFNULL(SUM(HBIT_16),0), "
                " IFNULL(SUM(HBIT_17),0), "
                " IFNULL(SUM(HBIT_18),0), "
                " IFNULL(SUM(HBIT_19),0), "
                " IFNULL(SUM(HBIT_20),0), "
                " IFNULL(SUM(HBIT_21),0), "
                " IFNULL(SUM(HBIT_22),0), "
                " IFNULL(SUM(HBIT_23),0), "
                " IFNULL(SUM(HBIT_24),0), "
                " IFNULL(SUM(HBIT_25),0), "
                " IFNULL(SUM(HBIT_26),0), "
                " IFNULL(SUM(HBIT_27),0), "
                " IFNULL(SUM(HBIT_28),0), "
                " IFNULL(SUM(HBIT_29),0), "
                " IFNULL(SUM(HBIT_30),0), "
                " IFNULL(SUM(HBIT_31),0), "
                " IFNULL(SUM(SM_INT_ERR),0), "
                " IFNULL(SUM(OP_ERR),0), "
                " IFNULL(SUM(OP_TIMEOUT),0), "
                " IFNULL(SUM(ETC_FAIL),0), "
                " IFNULL(SUM(API_REQ_ERR),0), "
                " IFNULL(SUM(API_TIMEOUT),0), "
                " IFNULL(SUM(stat_cnt),0) "
                " FROM %s WHERE system_name = '%s' "
				" and sm_ch_id = %d and log_mod = %d "
				" and (stat_date >= '%s' and stat_date < '%s')",
                seltbl, sysname, sm_ch_id, log_mod, starttime, endtime);
    } 

	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
    result1 = mysql_store_result(conn);
    row1 = mysql_fetch_row(result1); 

    if( row1 == NULL )
    {
        sprintf(trcBuf, "[%d]%s LogOn select error\n%s\n", time_type, seltbl, query);
        trclib_writeLogErr (FL, trcBuf);
		mysql_free_result(result1); // MEM if NULL 10.07
        return -1;
    }

	sprintf(query, "UPDATE %s SET log_req = %s, log_succ = %s, log_fail = %s, succ_rate = %.1f," //uamyd 20110515 succrate_added
			" HBIT_0 = %s, HBIT_1 = %s, HBIT_2 = %s, HBIT_3 = %s, HBIT_4 = %s, "
			" HBIT_5 = %s, HBIT_6 = %s, HBIT_7 = %s, HBIT_8 = %s, HBIT_9 = %s, "
			" HBIT_10 = %s, HBIT_11 = %s, HBIT_12 = %s, HBIT_13 = %s, HBIT_14 = %s, "
			" HBIT_15 = %s, HBIT_16 = %s, HBIT_17 = %s, HBIT_18 = %s, HBIT_19 = %s, "
			" HBIT_20 = %s, HBIT_21 = %s, HBIT_22 = %s, HBIT_23 = %s, HBIT_24 = %s, "
			" HBIT_25 = %s, HBIT_26 = %s, HBIT_27 = %s, HBIT_28 = %s, HBIT_29 = %s, "
			" HBIT_30 = %s, HBIT_31 = %s, " 
			" SM_INT_ERR = %s, OP_ERR = %s, OP_TIMEOUT = %s, ETC_FAIL = %s, "
			" API_REQ_ERR = %s, API_TIMEOUT = %s, "
			" stat_cnt = %s "
			" WHERE system_name = '%s' and stat_date = '%s' and sm_ch_id = %d and log_mod = %d",
			table_type, 
			row1[0],  row1[1],  row1[2], 
			atoi(row1[0])==0?0.0:(((float)atoi(row1[1])/(float)atoi(row1[0]))*100), //uamyd 20110515 succrate_added
			row1[3], row1[4],  row1[5],  row1[6],  row1[7], 
			row1[8],  row1[9],  row1[10], row1[11], row1[12], 
			row1[13], row1[14], row1[15], row1[16], row1[17], 
			row1[18], row1[19], row1[20], row1[21], row1[22], 
			row1[23], row1[24], row1[25], row1[26], row1[27], 
			row1[28], row1[29], row1[30], row1[31], row1[32], 
			row1[33], row1[34], row1[35], row1[36], 
			row1[37], row1[38], row1[39], row1[40], 
			row1[41],
			sysname, get_select_time(time_type), sm_ch_id, log_mod);


    if ( trcLogFlag == TRCLEVEL_SQL ) 
    {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
        
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        mysql_free_result(result1);
        return -1;
    }

    mysql_free_result(result1);

    return 1;
}

int stmd_LogOnInsert(int time_type, char *table_type, MYSQL_ROW inrow)
{
    char        query[4096];
    char    weektype[32];


///////////////////////////
    memset(weektype,0,sizeof(weektype));
    if(time_type == STMD_WEEK)
    {
        sprintf(weektype,"Mon");
    }
    else if(time_type == STMD_MONTH)
    {
        sprintf(weektype, "%s", get_insert_week2());
    }
    else
    {
        sprintf (weektype, "%s", get_insert_week());
    }

	sprintf(query, "INSERT INTO %s VALUES ("
					" '%s', '%s', '%s', '%s', '%s', '%s', '%.1f', " //uamyd 20110515 succrate_added
					" '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', "
					" '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', "
					" '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', "
					" '%s', '%s', "
					" '%s', '%s', '%s', '%s', '%s', '%s', " 
					" '%s', '%s', '%s' )", 
					table_type, 
					inrow[0],  inrow[1],  inrow[2],  inrow[3],  inrow[4], inrow[5], 
					atoi(inrow[3])==0?0.0:(((float)atoi(inrow[4])/(float)atoi(inrow[3]))*100), //uamyd 20110515 succrate_added
					inrow[7],  inrow[8],  inrow[9],  inrow[10], inrow[11], 
					inrow[12], inrow[13], inrow[14], inrow[15], inrow[16], 
					inrow[17], inrow[18], inrow[19], inrow[20], inrow[21], 
					inrow[22], inrow[23], inrow[24], inrow[25], inrow[26], 
					inrow[27], inrow[28], inrow[29], inrow[30], inrow[31], 
					inrow[32], inrow[33], inrow[34], inrow[35],   inrow[36], 
					inrow[37], inrow[38], inrow[39], inrow[40], 
					inrow[41], inrow[42], 
					inrow[43], inrow[44], inrow[45], get_select_time(time_type), weektype);
                                    
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n%s\n", mysql_error(conn), query);
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }

    return 1;
}

int stmd_exeUpdateHourLogOn()
{
    char    query[4096],insert_time[32];
    int     ret_select;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

	/*
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
            STM_STATISTIC_HOUR_LOGON_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*3)); // 3달 
			*/
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
            STM_STATISTIC_HOUR_LOGON_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_HOUR]));

    if ( trcLogFlag == TRCLEVEL_SQL ) 
	{
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }

    if (stmd_mysql_query (query) < 0) 
	{
        sprintf(trcBuf, "LogOnStatistic 1 Hour mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
		return -1;
    }

    /*현재 시간을 구한다음 5분 table에서 정보를 select 하고
    row 만큼 loop -> upate / insert */
    sprintf (insert_time, "%s", get_insert_time());
    if(strcasecmp(insert_time,"") == 0)
    {
        sprintf(trcBuf, "LogOn Statistic(Hour) 5minute time get fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
    sprintf(query, "SELECT * FROM %s where ( stat_date = '%s')",
                STM_STATISTIC_5MINUTE_LOGON_TBL_NAME, insert_time);

	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    result1 = mysql_store_result(conn);
    
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
        sprintf(query, "SELECT * FROM %s where (system_name = '%s' and "
						" stat_date = '%s' and sm_ch_id = %s and log_mod = %s )",
                    STM_STATISTIC_HOUR_LOGON_TBL_NAME, row1[0], get_select_time(STMD_HOUR), row1[1], row1[2]);

        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
            if ( stmd_LogOnUpdate(STMD_HOUR, STM_STATISTIC_HOUR_LOGON_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "LOGON HOUR Update error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_LogOnInsert(STMD_HOUR, STM_STATISTIC_HOUR_LOGON_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "LOGON HOUR Insert error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else 
        {
            sprintf(trcBuf,"stmd_exeUpdateHourLogOn Update/Insert Fail = %d\n",ret_select);
            trclib_writeLogErr (FL,trcBuf);
            mysql_free_result(result1);
            return -1;
        }
    }
    
    mysql_free_result(result1);

    return 1;
}

int stmd_exeUpdateDayLogOn()
{
    char    query[4096],insert_time[32];
    int     ret_select;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

	/*
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_DAY_LOGON_TBL_NAME, get_delete_time(STMD_1MON_OFFSET)); // 1년
		*/
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_DAY_LOGON_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_DAY]));

    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }

    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "LogOn 1 Day mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
		return -1;
    }

///////////////////
    /*현재 시간을 구한다음  
    5분 table에서 정보를 select 하고
    row 만큼 loop -> upate / insert */
    sprintf (insert_time, "%s", get_insert_time());
    if(strcasecmp(insert_time,"") == 0)
    {
        sprintf(trcBuf, "LogOn Statistic(Day) preview 5minute time get fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
    sprintf(query, "SELECT * FROM %s where (stat_date = '%s')",
                STM_STATISTIC_5MINUTE_LOGON_TBL_NAME, insert_time);
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    result1 = mysql_store_result(conn);
    
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
        sprintf(query, "SELECT * FROM %s where (system_name = '%s' and "
						" stat_date = '%s' and sm_ch_id = %s and log_mod = %s )",
                    STM_STATISTIC_DAY_LOGON_TBL_NAME, row1[0], get_select_time(STMD_DAY), row1[1], row1[2]);

        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
            if ( stmd_LogOnUpdate(STMD_DAY, STM_STATISTIC_DAY_LOGON_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "LOGON DAY Update error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_LogOnInsert(STMD_DAY, STM_STATISTIC_DAY_LOGON_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "LOGON DAY Insert error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else 
        {
            sprintf(trcBuf,"stmd_exeUpdateDayLogOn Update/Insert Fail = %d\n",ret_select);
            trclib_writeLogErr (FL,trcBuf);
            mysql_free_result(result1);
            return -1;
        }
    }
    
    mysql_free_result(result1);

///////////////////

    return 1;
}


int stmd_exeUpdateWeekLogOn()
{
    char    query[4096],insert_time[32];
    int     ret_select;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

	/*
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_WEEK_LOGON_TBL_NAME, get_delete_time(STMD_1MON_OFFSET));
		*/
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_WEEK_LOGON_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_WEEK]));

    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "LogOnStatistic 1 Week mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
		return -1;
    }

///////////////////
    /*현재 시간보다 5분전 시간을 구한다음
    5분 table에서 정보를 select 하고
    row 만큼 loop -> upate / insert */
    sprintf (insert_time, "%s", get_insert_time());
    if(strcasecmp(insert_time,"") == 0)
    {
        sprintf(trcBuf, "LogOn Statistic(Week) preview 5minute time get fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
    sprintf(query, "SELECT * FROM %s where (stat_date = '%s')",
                STM_STATISTIC_5MINUTE_LOGON_TBL_NAME, insert_time);
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    result1 = mysql_store_result(conn);
    
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
        sprintf(query, "SELECT * FROM %s where (system_name = '%s' and "
						" stat_date = '%s' and sm_ch_id = %s and log_mod = %s )",
                    STM_STATISTIC_WEEK_LOGON_TBL_NAME, row1[0], get_select_time(STMD_WEEK), row1[1], row1[2]);
        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
            if ( stmd_LogOnUpdate(STMD_WEEK, STM_STATISTIC_WEEK_LOGON_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "LOGON WEEK Update error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_LogOnInsert(STMD_WEEK, STM_STATISTIC_WEEK_LOGON_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "LOGON WEEK Insert error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else 
        {
            sprintf(trcBuf,"stmd_exeUpdateWeekLogOn Update/Insert Fail = %d\n",ret_select);
            trclib_writeLogErr (FL,trcBuf);
            mysql_free_result(result1);
            return -1;
        }
    }
    
    mysql_free_result(result1);

///////////////////

    return 1;
}



int stmd_exeUpdateMonthLogOn()
{
    char    query[4096],insert_time[32];
    int     ret_select;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

    // 1년 전의 한달 데이타를 지운다.
	/*
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_MONTH_LOGON_TBL_NAME, get_delete_time(STMD_1MON_OFFSET));
		*/
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_MONTH_LOGON_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_MONTH]));

    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "LogOnStatistic 1 Month mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
		return -1;
    }

///////////////////
    /*현재 시간보다 5분전 시간을 구한다음
    5분 table에서 정보를 select 하고
    row 만큼 loop -> upate / insert */
    sprintf (insert_time, "%s",get_insert_time());
    if(strcasecmp(insert_time,"") == 0)
    {
        sprintf(trcBuf, "LogOn Statistic(Month) preview 5minute time get fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
    sprintf(query, "SELECT * FROM %s where (stat_date = '%s')",
                STM_STATISTIC_5MINUTE_LOGON_TBL_NAME, insert_time);
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    result1 = mysql_store_result(conn);
    
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
        sprintf(query, "SELECT * FROM %s where (system_name = '%s' and "
						" stat_date = '%s' and sm_ch_id = %s and log_mod = %s )",
                    STM_STATISTIC_MONTH_LOGON_TBL_NAME, row1[0], get_select_time(STMD_MONTH), row1[1], row1[2]);

        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
            if ( stmd_LogOnUpdate(STMD_MONTH, STM_STATISTIC_MONTH_LOGON_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "LOGON MONTH Update error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_LogOnInsert(STMD_MONTH, STM_STATISTIC_MONTH_LOGON_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "LOGON MONTH Insert error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else 
        {
            sprintf(trcBuf,"stmd_exeUpdateMonthLogOn Update/Insert Fail = %d\n",ret_select);
            trclib_writeLogErr (FL,trcBuf);
            mysql_free_result(result1);
            return -1;
        }
    }
    
    mysql_free_result(result1);

    return 1;
}
