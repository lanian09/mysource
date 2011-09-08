#include "stmd_proto.h"

extern  char    trcBuf[4096], trcTmp[1024];
extern  int     trcLogFlag ;
extern  MYSQL   sql, *conn;
extern  int     sysCnt;
extern  STM_CommStatisticInfo    StatisticSystemInfo[SYSCONF_MAX_ASSO_SYS_NUM];
extern int trcLogId;
extern short 	delTIME[STMD_PERIOD_TYPE_NUM];

//int   stmd_LoadUpdate(int time_type, char *table_type, char *system_name)
int stmd_LURUpdate(int time_type, char *table_type, MYSQL_ROW inrow)
{
    char        query[4096];
    char        starttime[32],endtime[32],seltbl[32]; // by jjinri 2009.04.20 ,sysname[12];
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;
	char		record_source[32] = {0,}, link_id[10] = {0,};

	memset (seltbl, 0x00, sizeof(seltbl));
//    sprintf(sysname, "%s", inrow[1]);
	sprintf(record_source, "%s", inrow[0]);
	sprintf(link_id, "%s", inrow[1]);
//	sprintf(glbl_usg_cnt_id, "%s", inrow[2]);
    
    sprintf(starttime, "%s", get_select_time2(time_type));
    sprintf(endtime, "%s", get_period_end_time2(time_type));    
    switch(time_type)
    {
    case STMD_HOUR:
        strcpy(seltbl, STM_STATISTIC_5MINUTE_LUR_TBL_NAME);
        break;
    case STMD_DAY:
        strcpy(seltbl, STM_STATISTIC_HOUR_LUR_TBL_NAME);
        break;
    case STMD_WEEK:
        strcpy(seltbl, STM_STATISTIC_DAY_LUR_TBL_NAME);
        break;
    case STMD_MONTH:
        strcpy(seltbl, STM_STATISTIC_DAY_LUR_TBL_NAME);
        break;
    default:
        strcpy(seltbl, STM_STATISTIC_HOUR_LUR_TBL_NAME);
        break;
    }

    if (time_type == STMD_HOUR || time_type == STMD_DAY){
		sprintf(query, "SELECT "
                " IFNULL(SUM(upstream_volume),0), "
                " IFNULL(SUM(downstream_volume),0), "
                " IFNULL(SUM(stat_cnt),0) "
                " FROM %s "
				" WHERE (record_source = '%s' and "
				" 		link_id = %s and stat_date > '%s' and stat_date <= '%s') "
				" Group by record_source,link_id ",
                seltbl, record_source, link_id, starttime, endtime);
    }else{
	sprintf(query, "SELECT "
                " IFNULL(SUM(upstream_volume),0), "
                " IFNULL(SUM(downstream_volume),0), "
                " IFNULL(SUM(stat_cnt),0) "
                " FROM %s "
				" WHERE (record_source = '%s' and "
				" 		link_id = %s and stat_date >= '%s' and stat_date < '%s')"
				" Group by record_source,link_id ",
                seltbl, record_source, link_id, starttime, endtime);
    } 
//	logPrint(trcLogId,FL,"LUR Update : query=%s\n", query);

	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; query=%s; err=%s\n", query, mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
    result1 = mysql_store_result(conn);
    row1 = mysql_fetch_row(result1); 

    if( row1[0] == NULL )
    {
        sprintf(trcBuf, "[%d]%s Lur 5MIN select error: %s\n", time_type, seltbl, query);
        trclib_writeLogErr (FL, trcBuf);
		mysql_free_result(result1);
        return -1;
    }
    
    // 데이타를 update한다.
    sprintf(query, "UPDATE %s SET " 
                    "upstream_volume = %s,"
                    "downstream_volume = %s,"
                    "stat_cnt = %s "
                    " WHERE (stat_date = '%s' AND record_source= '%s' AND link_id = %s )", 
                    table_type,
                    row1[0],row1[1], row1[2],
                    get_select_time(time_type), record_source, link_id);
    
    if ( trcLogFlag == TRCLEVEL_SQL ) 
    {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
        
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; query : %s; err=%s\n", query, mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        mysql_free_result(result1);
        return -1;
    }
         
    mysql_free_result(result1);

    return 1;
}

//int   stmd_LoadInsert(int time_type, char *table_type, char *system_name)
int stmd_LURInsert(int time_type, char *table_type, MYSQL_ROW inrow)
{
    char        query[4096];
    char    weektype[32];


///////////////////////////
    memset(weektype,0,sizeof(weektype));
    if(time_type == STMD_WEEK)
    {
        sprintf (weektype,"Mon");
    }
    else if(time_type == STMD_MONTH)
    {
        sprintf (weektype, "%s", get_insert_week2());
    }
    else
    {
        sprintf (weektype, "%s", get_insert_week());
    }

    sprintf(query, "INSERT INTO %s VALUES "
	 				"('%s','%s','%s','%s','%s','%s','%s')", table_type,
					inrow[0],inrow[1],inrow[2],inrow[3],inrow[4],get_select_time(time_type),weektype);
                                    
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; query : %s;  err=%s\n", query, mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
//	logPrint(trcLogId,FL, "LUR Update Insert : query=%s\n", query);

    return 1;
}

int stmd_exeUpdateHourLUR(void)
{
    char    query[4096],insert_time[32];
    int     ret_select;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

	/*
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
            STM_STATISTIC_HOUR_LUR_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*3)); // 3달 
			*/
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
            STM_STATISTIC_HOUR_LUR_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_HOUR]));
    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "LoadStatistic 1 Hour mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
		return -1;
    }

///////////////////
    /*현재 시간을 구한다음 5분 table에서 정보를 select 하고
    row 만큼 loop -> upate / insert */
    sprintf (insert_time, "%s", get_insert_time());
    if(strcasecmp(insert_time,"") == 0)
    {
        sprintf(trcBuf, "Load Statistic(Hour) 5minute time get fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
	/*
    sprintf(query, "SELECT record_source,link_id,glbl_usg_cnt_id,sum(upstream_volume), "\
					"sum(downstream_volume), sum(stat_cnt), stat_date, max(stat_week) "\
					"FROM %s where ( stat_date = '%s') "\
					"GROUP BY record_source, link_id, glbl_usg_cnt_id",
                STM_STATISTIC_5MINUTE_LUR_TBL_NAME, insert_time);
				*/
    sprintf(query, "SELECT * FROM %s where ( stat_date = '%s') ",\
                STM_STATISTIC_5MINUTE_LUR_TBL_NAME, insert_time);
	
//	logPrint(trcLogId, FL,"LUR Hour Update start query : %s\n", query);

	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    result1 = mysql_store_result(conn);
    
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
        sprintf(query, "SELECT * FROM %s where (record_source = '%s' and "
						"link_id = %s and stat_date = '%s')",
                    STM_STATISTIC_HOUR_LUR_TBL_NAME, row1[0], row1[1], get_select_time(STMD_HOUR));

//		logPrint(trcLogId, FL,"LUR Hour Update start query : %s\n", query);

        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
            if ( stmd_LURUpdate(STMD_HOUR, STM_STATISTIC_HOUR_LUR_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "LUR HOUR Update error: %s\n", query);
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_LURInsert(STMD_HOUR, STM_STATISTIC_HOUR_LUR_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "LUR HOUR Insert error: %s\n", query);
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else 
        {
            sprintf(trcBuf,"stmd_exeUpdateHourLUR Update/Insert Fail = %d\n",ret_select);
            trclib_writeLogErr (FL,trcBuf);
            mysql_free_result(result1);
            return -1;
        }
    }
    
    mysql_free_result(result1);

    return 1;
}

int stmd_exeUpdateDayLUR()
{
    char    query[4096],insert_time[32];
    int     ret_select;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

	/*
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_DAY_LUR_TBL_NAME, get_delete_time(STMD_1MON_OFFSET)); // 1년
		*/
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_DAY_LUR_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_DAY]));
    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "LUR Day Statistic mysql_delete fail\n");
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
        sprintf(trcBuf, "LUR Statistic(Day) preview 5minute time get fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }

	/*
    sprintf(query, "SELECT record_source,link_id,glbl_usg_cnt_id,sum(upstream_volume), "\
					"sum(downstream_volume), sum(stat_cnt), stat_date, max(stat_week) "\
					"FROM %s where ( stat_date = '%s') "\
					"GROUP BY record_source, link_id, glbl_usg_cnt_id",
                STM_STATISTIC_5MINUTE_LUR_TBL_NAME, insert_time);
				*/
    sprintf(query, "SELECT * FROM %s where ( stat_date = '%s') ",\
                STM_STATISTIC_5MINUTE_LUR_TBL_NAME, insert_time);
	
//	logPrint(trcLogId, FL,"LUR Day Update start query : %s\n", query);

	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    result1 = mysql_store_result(conn);
    
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
        sprintf(query, "SELECT * FROM %s where (record_source = '%s' and "
						"link_id = %s and stat_date = '%s')",
                    STM_STATISTIC_DAY_LUR_TBL_NAME, row1[0], row1[1], get_select_time(STMD_DAY));
		/*
        sprintf(query, "SELECT * FROM %s where (record_source = '%s' and "
						"link_id = %s and glbl_usg_cnt_id = %s and stat_date = '%s')",
                    STM_STATISTIC_DAY_LUR_TBL_NAME, row1[0], row1[1], row1[2], get_select_time(STMD_DAY));
		*/

//	logPrint(trcLogId, FL,"LUR Day Update start query : %s\n", query);

        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
            if ( stmd_LURUpdate(STMD_DAY, STM_STATISTIC_DAY_LUR_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "LUR DAY Update error: %s\n", query);
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_LURInsert(STMD_DAY, STM_STATISTIC_DAY_LUR_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "LUR DAY Insert error: %s\n", query);
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else 
        {
            sprintf(trcBuf,"stmd_exeUpdateDayLUR Update/Insert Fail = %d\n",ret_select);
            trclib_writeLogErr (FL,trcBuf);
            mysql_free_result(result1);
            return -1;
        }
    }
    
    mysql_free_result(result1);

    return 1;
	
}

int stmd_exeUpdateWeekLUR()
{
    char    query[4096],insert_time[32];
    int     ret_select;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

	/*
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_WEEK_LUR_TBL_NAME, get_delete_time(STMD_1MON_OFFSET));
		*/
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_WEEK_LUR_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_WEEK]));
    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "LUR WEEK Statistic 1 Week mysql_delete fail\n");
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
        sprintf(trcBuf, "LUR Statistic(Week) preview 5minute time get fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
	/*
    sprintf(query, "SELECT record_source,link_id,glbl_usg_cnt_id,sum(upstream_volume), "\
					"sum(downstream_volume), sum(stat_cnt), stat_date, max(stat_week) "\
					"FROM %s where ( stat_date = '%s') "\
					"GROUP BY record_source, link_id, glbl_usg_cnt_id",
                STM_STATISTIC_5MINUTE_LUR_TBL_NAME, insert_time);
				*/
    sprintf(query, "SELECT * FROM %s where ( stat_date = '%s') ",\
                STM_STATISTIC_5MINUTE_LUR_TBL_NAME, insert_time);

	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    result1 = mysql_store_result(conn);

    while( (row1 = mysql_fetch_row(result1)) != NULL )
    {
        sprintf(query, "SELECT * FROM %s where (record_source = '%s' and "
						"link_id = %s and stat_date = '%s')",
                    STM_STATISTIC_WEEK_LUR_TBL_NAME, row1[0], row1[1], get_select_time(STMD_WEEK));
		/*
        sprintf(query, "SELECT * FROM %s where (record_source = '%s' and "
                        "link_id = %s and glbl_usg_cnt_id = %s and stat_date = '%s')",
                    STM_STATISTIC_WEEK_LUR_TBL_NAME, row1[0], row1[1], row1[2], get_select_time(STMD_WEEK));
		*/

//	logPrint(trcLogId, FL,"LUR Week Update start query : %s\n", query);

        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        {
            if ( stmd_LURUpdate(STMD_WEEK, STM_STATISTIC_WEEK_LUR_TBL_NAME, row1) < 0 )
            {
                sprintf(trcBuf, "LUR WEEK Update error: %s\n", query);
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        }
        else if ( ret_select == 0 ) // insert 
        {
            if ( stmd_LURInsert(STMD_WEEK, STM_STATISTIC_WEEK_LUR_TBL_NAME, row1) < 0 )
            {
                sprintf(trcBuf, "LUR WEEK Insert error: %s\n", query);
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        }
        else
        {
            sprintf(trcBuf,"stmd_exeUpdateWeekLUR Update/Insert Fail = %d\n",ret_select);
            trclib_writeLogErr (FL,trcBuf);
            mysql_free_result(result1);
            return -1;
        }
    }
    
    mysql_free_result(result1);

///////////////////

    return 1;
}

int stmd_exeUpdateMonthLUR()
{
    char    query[4096],insert_time[32];
    int     ret_select;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

    // 1년 전의 한달 데이타를 지운다.
	/*
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_MONTH_LUR_TBL_NAME, get_delete_time(STMD_1MON_OFFSET));
		*/
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_MONTH_LUR_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_MONTH]));
    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "LUR Month Statistic  mysql_delete fail\n");
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
        sprintf(trcBuf, "LUR Statistic(Month) preview 5minute time get fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
	/*
    sprintf(query, "SELECT record_source,link_id,glbl_usg_cnt_id,sum(upstream_volume), "\
					"sum(downstream_volume), sum(stat_cnt), stat_date, max(stat_week) "\
					"FROM %s where ( stat_date = '%s') "\
					"GROUP BY record_source, link_id, glbl_usg_cnt_id",
                STM_STATISTIC_5MINUTE_LUR_TBL_NAME, insert_time);
				*/
    sprintf(query, "SELECT * FROM %s where ( stat_date = '%s') ",\
                STM_STATISTIC_5MINUTE_LUR_TBL_NAME, insert_time);

	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    result1 = mysql_store_result(conn);
    
    while( (row1 = mysql_fetch_row(result1)) != NULL )
    {
		/*
        sprintf(query, "SELECT * FROM %s where (record_source = '%s' and "
                        "link_id = %s and glbl_usg_cnt_id = %s and stat_date = '%s')",
                    STM_STATISTIC_MONTH_LUR_TBL_NAME, row1[0], row1[1], row1[2], get_select_time(STMD_MONTH));
		*/
        sprintf(query, "SELECT * FROM %s where (record_source = '%s' and "
						"link_id = %s and stat_date = '%s')",
                    STM_STATISTIC_MONTH_LUR_TBL_NAME, row1[0], row1[1], get_select_time(STMD_MONTH));

//		logPrint(trcLogId, FL,"LUR Month Update start query : %s\n", query);

        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        {
            if ( stmd_LURUpdate(STMD_MONTH, STM_STATISTIC_MONTH_LUR_TBL_NAME, row1) < 0 )
            {
                sprintf(trcBuf, "LUR MONTH Update error: %s\n", query);
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        }
        else if ( ret_select == 0 ) // insert 
        {
            if ( stmd_LURInsert(STMD_MONTH, STM_STATISTIC_MONTH_LUR_TBL_NAME, row1) < 0 )
            {
                sprintf(trcBuf, "LUR MONTH Insert error: %s\n", query);
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        }
        else
        {
            sprintf(trcBuf,"stmd_exeUpdateMonthLUR Update/Insert Fail = %d\n",ret_select);
            trclib_writeLogErr (FL,trcBuf);
            mysql_free_result(result1);
            return -1;
        }
    }

    mysql_free_result(result1);

///////////////////

    return 1;
}
