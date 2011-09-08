#include "stmd_proto.h"

extern  char    trcBuf[4096], trcTmp[1024];
extern  int     trcLogFlag ;
extern  MYSQL   sql, *conn;
extern 	short	delTIME[STMD_PERIOD_TYPE_NUM];

int stmd_SmsUpdate(int time_type, char *table_type, MYSQL_ROW inrow)
{
    char        query[4096*2];
    char        starttime[32],endtime[32],seltbl[32],sysname[12], smsc[20];
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

	memset (seltbl, 0x00, sizeof(seltbl));
	sprintf(sysname, "%s", inrow[0]);
	sprintf(smsc, "%s", inrow[1]);
    sprintf(starttime, "%s", get_select_time2(time_type));
    sprintf(endtime, "%s", get_period_end_time2(time_type));    

    switch(time_type)
    {
		case STMD_HOUR:
			strcpy(seltbl, STM_STATISTIC_5MINUTE_SMS_TBL_NAME);
			break;
		case STMD_DAY:
			strcpy(seltbl, STM_STATISTIC_HOUR_SMS_TBL_NAME);
			break;
		case STMD_WEEK:
			strcpy(seltbl, STM_STATISTIC_DAY_SMS_TBL_NAME);
			break;
		case STMD_MONTH:
			strcpy(seltbl, STM_STATISTIC_DAY_SMS_TBL_NAME);
			break;
		default:
			strcpy(seltbl, STM_STATISTIC_HOUR_SMS_TBL_NAME);
			break;
    }

    if (time_type == STMD_HOUR || time_type == STMD_DAY){
    	sprintf(query, "SELECT "
                " IFNULL(SUM(req),0), "
                " IFNULL(SUM(succ),0), "
                " IFNULL(SUM(fail),0), "
                " IFNULL(SUM(smpp_err),0), "
                " IFNULL(SUM(svr_err),0), "
                " IFNULL(SUM(smsc_err),0), "
                " IFNULL(SUM(etc_err),0), "
                " IFNULL(SUM(stat_cnt),0) "
                " FROM %s WHERE system_name = '%s' and smsc_ip = '%s' "
				" and (stat_date > '%s' and stat_date <= '%s')",
                seltbl, sysname, smsc, starttime, endtime);
    }else{
	sprintf(query, "SELECT "
                " IFNULL(SUM(req),0), "
                " IFNULL(SUM(succ),0), "
                " IFNULL(SUM(fail),0), "
                " IFNULL(SUM(smpp_err),0), "
                " IFNULL(SUM(svr_err),0), "
                " IFNULL(SUM(smsc_err),0), "
                " IFNULL(SUM(etc_err),0), "
                " IFNULL(SUM(stat_cnt),0) "
                " FROM %s WHERE system_name = '%s' and smsc_ip = '%s' "
				" and (stat_date >= '%s' and stat_date < '%s')",
                seltbl, sysname, smsc, starttime, endtime);
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
        sprintf(trcBuf, "[%d]%s Sms select error\n%s\n", time_type, seltbl, query);
        trclib_writeLogErr (FL, trcBuf);
		mysql_free_result(result1); // MEM if NULL 10.07
        return -1;
    }
    
    // 데이타를 update한다.
    sprintf(query, "UPDATE %s SET " 
                    " req = %s, succ = %s, fail = %s, smpp_err = %s, "
                    " svr_err = %s, smsc_err = %s, etc_err = %s, "
                    " stat_cnt = %s "
                    " WHERE system_name = '%s' and smsc_ip = '%s' and (stat_date = '%s')", 
                    table_type,
                    row1[0],row1[1], row1[2],row1[3],
                    row1[4],row1[5], row1[6],
					row1[7], sysname, smsc, get_select_time(time_type));
    
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

int stmd_SmsInsert(int time_type, char *table_type, MYSQL_ROW inrow)
{
    char        query[4096];
    char    weektype[32];


///////////////////////////
    memset(weektype,0x00,sizeof(weektype));
    if(time_type == STMD_WEEK)
    {
        strcpy(weektype,"Mon");
    }
    else if(time_type == STMD_MONTH)
    {
        strcpy(weektype,get_insert_week2());
    }
    else
    {
        strcpy(weektype,get_insert_week());
    }

    sprintf(query, "INSERT INTO %s VALUES ("
                                    " '%s', '%s', "
									" '%s', '%s', '%s', '%s', "
									" '%s', '%s', '%s', "
									" '%s', '%s', '%s' )", 
                                    table_type,
                                    inrow[0], inrow[1], 
									inrow[2], inrow[3], inrow[4], inrow[5], 
									inrow[6], inrow[7], inrow[8], 
									inrow[9], get_select_time(time_type), weektype);
                                    
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n%s\n", mysql_error(conn), query);
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }

    return 1;
}

int stmd_exeUpdateHourSms()
{
    char    query[4096],insert_time[32];
    int     ret_select;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

	/*
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
            STM_STATISTIC_HOUR_SMS_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*3)); // 3달 
			*/
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
            STM_STATISTIC_HOUR_SMS_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_HOUR]));

    if ( trcLogFlag == TRCLEVEL_SQL ) 
	{
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }

    if (stmd_mysql_query (query) < 0) 
	{
        sprintf(trcBuf, "SmsStatistic 1 Hour mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
		return -1;
    }

    /*현재 시간을 구한다음 5분 table에서 정보를 select 하고
    row 만큼 loop -> upate / insert */
    sprintf(insert_time, "%s", get_insert_time());
    if(strcasecmp(insert_time,"") == 0)
    {
        sprintf(trcBuf, "Sms Statistic(Hour) 5minute time get fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
    sprintf(query, "SELECT * FROM %s where ( stat_date = '%s')",
                STM_STATISTIC_5MINUTE_SMS_TBL_NAME, insert_time);

	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    result1 = mysql_store_result(conn);
    
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
        sprintf(query, "SELECT * FROM %s where (system_name = '%s' and "
						" smsc_ip = '%s' and stat_date = '%s')",
                    STM_STATISTIC_HOUR_SMS_TBL_NAME, row1[0], row1[1], get_select_time(STMD_HOUR));

        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
            if ( stmd_SmsUpdate(STMD_HOUR, STM_STATISTIC_HOUR_SMS_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "SMS HOUR Update error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_SmsInsert(STMD_HOUR, STM_STATISTIC_HOUR_SMS_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "SMS HOUR Insert error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else 
        {
            sprintf(trcBuf,"stmd_exeUpdateHourSms Update/Insert Fail = %d\n",ret_select);
            trclib_writeLogErr (FL,trcBuf);
            mysql_free_result(result1);
            return -1;
        }
    }
    
    mysql_free_result(result1);

    return 1;
}

int stmd_exeUpdateDaySms()
{
    char    query[4096],insert_time[32];
    int     ret_select;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

	/*
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_DAY_SMS_TBL_NAME, get_delete_time(STMD_1MON_OFFSET)); // 1년
		*/
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_DAY_SMS_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_DAY]));

    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }

    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "Sms 1 Day mysql_delete fail\n");
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
        sprintf(trcBuf, "Sms Statistic(Day) preview 5minute time get fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
    sprintf(query, "SELECT * FROM %s where (stat_date = '%s')",
                STM_STATISTIC_5MINUTE_SMS_TBL_NAME, insert_time);
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    result1 = mysql_store_result(conn);
    
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
        sprintf(query, "SELECT * FROM %s where (system_name = '%s' and "
						" smsc_ip = '%s' and stat_date = '%s')",
                    STM_STATISTIC_DAY_SMS_TBL_NAME, row1[0], row1[1], get_select_time(STMD_DAY));

        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
            if ( stmd_SmsUpdate(STMD_DAY, STM_STATISTIC_DAY_SMS_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "SMS DAY Update error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_SmsInsert(STMD_DAY, STM_STATISTIC_DAY_SMS_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "SMS DAY Insert error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else 
        {
            sprintf(trcBuf,"stmd_exeUpdateDaySms Update/Insert Fail = %d\n",ret_select);
            trclib_writeLogErr (FL,trcBuf);
            mysql_free_result(result1);
            return -1;
        }
    }
    
    mysql_free_result(result1);

///////////////////

    return 1;
}


int stmd_exeUpdateWeekSms()
{
    char    query[4096],insert_time[32];
    int     ret_select;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

	/*
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_WEEK_SMS_TBL_NAME, get_delete_time(STMD_1MON_OFFSET));
		*/
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_WEEK_SMS_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_WEEK]));

    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "SmsStatistic 1 Week mysql_delete fail\n");
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
        sprintf(trcBuf, "Sms Statistic(Week) preview 5minute time get fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
    sprintf(query, "SELECT * FROM %s where (stat_date = '%s')",
                STM_STATISTIC_5MINUTE_SMS_TBL_NAME, insert_time);
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    result1 = mysql_store_result(conn);
    
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
        sprintf(query, "SELECT * FROM %s where (system_name = '%s' and "
						" smsc_ip = '%s' and stat_date = '%s')",
                    STM_STATISTIC_WEEK_SMS_TBL_NAME, row1[0], row1[1], get_select_time(STMD_WEEK));

        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
            if ( stmd_SmsUpdate(STMD_WEEK, STM_STATISTIC_WEEK_SMS_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "SMS WEEK Update error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_SmsInsert(STMD_WEEK, STM_STATISTIC_WEEK_SMS_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "SMS WEEK Insert error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else 
        {
            sprintf(trcBuf,"stmd_exeUpdateWeekSms Update/Insert Fail = %d\n",ret_select);
            trclib_writeLogErr (FL,trcBuf);
            mysql_free_result(result1);
            return -1;
        }
    }
    
    mysql_free_result(result1);

///////////////////

    return 1;
}



int stmd_exeUpdateMonthSms()
{
    char    query[4096],insert_time[32];
    int     ret_select;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

    // 1년 전의 한달 데이타를 지운다.
	/*
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_MONTH_SMS_TBL_NAME, get_delete_time(STMD_1MON_OFFSET));
		*/
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_MONTH_SMS_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_MONTH]));

    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "SmsStatistic 1 Month mysql_delete fail\n");
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
        sprintf(trcBuf, "Sms Statistic(Month) preview 5minute time get fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
    sprintf(query, "SELECT * FROM %s where (stat_date = '%s')",
                STM_STATISTIC_5MINUTE_SMS_TBL_NAME, insert_time);
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    result1 = mysql_store_result(conn);
    
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
        sprintf(query, "SELECT * FROM %s where (system_name = '%s' and "
						" smsc_ip = '%s' and stat_date = '%s')",
                    STM_STATISTIC_MONTH_SMS_TBL_NAME, row1[0], row1[1], get_select_time(STMD_MONTH));

        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
            if ( stmd_SmsUpdate(STMD_MONTH, STM_STATISTIC_MONTH_SMS_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "SMS MONTH Update error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_SmsInsert(STMD_MONTH, STM_STATISTIC_MONTH_SMS_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "SMS MONTH Insert error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else 
        {
            sprintf(trcBuf,"stmd_exeUpdateMonthSms Update/Insert Fail = %d\n",ret_select);
            trclib_writeLogErr (FL,trcBuf);
            mysql_free_result(result1);
            return -1;
        }
    }
    
    mysql_free_result(result1);

    return 1;
}
