#include "stmd_proto.h"

extern  char    trcBuf[4096], trcTmp[1024];
extern  int     trcLogFlag ;
extern  MYSQL   sql, *conn;
extern  int     sysCnt;
extern  STM_CommStatisticInfo    StatisticSystemInfo[SYSCONF_MAX_ASSO_SYS_NUM];
extern int trcLogId;
extern 	short	delTIME[STMD_PERIOD_TYPE_NUM];

extern RuleSetList        g_stSCERule[MAX_SCE_NUM];

int stmd_RULESETUpdate(int time_type, char *table_type, MYSQL_ROW inrow)
{
    char        query[4096];
    char        starttime[32],endtime[32],seltbl[32] = {0,}; 
    MYSQL_RES   *res1;
    MYSQL_ROW   row1;
	char		record_source[32]={0,}, rule_set_id[10]={0,}, rule_set_name[64]={0,}; 
	int			pkg_id; 

	memset (seltbl, 0x00, sizeof(seltbl));
	sprintf(record_source, "%s", inrow[0]);

	pkg_id = atoi(inrow[1]);
	sprintf(rule_set_id, "%s", inrow[2]);
	sprintf(rule_set_name, "%s", inrow[3]);
    
    sprintf(starttime, "%s", get_select_time2(time_type));
    sprintf(endtime, "%s", get_period_end_time2(time_type));    
    switch(time_type)
    {
    case STMD_HOUR:
        strcpy(seltbl, STM_STATISTIC_5MINUTE_RULESET_TBL_NAME);
        break;
    case STMD_DAY:
        strcpy(seltbl, STM_STATISTIC_HOUR_RULESET_TBL_NAME);
        break;
    case STMD_WEEK:
        strcpy(seltbl, STM_STATISTIC_DAY_RULESET_TBL_NAME);
        break;
    case STMD_MONTH:
        strcpy(seltbl, STM_STATISTIC_DAY_RULESET_TBL_NAME);
        break;
    default:
        strcpy(seltbl, STM_STATISTIC_HOUR_RULESET_TBL_NAME);
        break;
    }

    if (time_type == STMD_HOUR || time_type == STMD_DAY){
		sprintf(query, "SELECT "
                " IFNULL(SUM(session),0), "
                " IFNULL(SUM(upstream_volume),0), "
                " IFNULL(SUM(downstream_volume),0), "
                " IFNULL(SUM(block_cnt),0), "
                " IFNULL(SUM(redirect_cnt),0), "
                " IFNULL(SUM(stat_cnt),0) "
                " FROM %s "
				" WHERE (record_source = '%s' and pkg_id = %d and "
					"rule_set_id = '%s' and rule_set_name = '%s' and "
					"stat_date > '%s' and stat_date <= '%s') "
				" Group by record_source, pkg_id, rule_set_id, rule_set_name",
                seltbl, record_source, pkg_id, rule_set_id, rule_set_name, starttime, endtime);
    }else{
		sprintf(query, "SELECT "
                " IFNULL(SUM(session),0), "
                " IFNULL(SUM(upstream_volume),0), "
                " IFNULL(SUM(downstream_volume),0), "
                " IFNULL(SUM(block_cnt),0), "
                " IFNULL(SUM(redirect_cnt),0), "
                " IFNULL(SUM(stat_cnt),0) "
                " FROM %s "
				" WHERE (record_source = '%s' and pkg_id = %d and "
					"rule_set_id = '%s' and rule_set_name = '%s' and "
					"stat_date >= '%s' and stat_date < '%s') "
					"Group by record_source, pkg_id, rule_set_id, rule_set_name",
                seltbl, record_source, pkg_id, rule_set_id, rule_set_name, starttime, endtime);
    } 
    if ( trcLogFlag == TRCLEVEL_SQL ) {
		logPrint(trcLogId,FL,"RULE Update : query=%s\n", query);
	}

	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; query=%s; err=%s\n", query, mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
    res1 = mysql_store_result(conn);
    row1 = mysql_fetch_row(res1); 

    if( row1 == NULL )
    {
        sprintf(trcBuf, "[%d]%s RULE Update select error: %s\n", time_type, seltbl, query);
        trclib_writeLogErr (FL, trcBuf);
		mysql_free_result(res1);
        return -1;
    }
    mysql_free_result(res1);
    
    // 데이타를 update한다.
	sprintf(query, "UPDATE %s SET "
                " session = %s, "
                " upstream_volume = %s, "
                " downstream_volume = %s, "
                " block_cnt = %s, "
                " redirect_cnt = %s, "
                " stat_cnt = %s "
				" WHERE (record_source = '%s' and "
					" pkg_id = %d and rule_set_id = '%s' and rule_set_name = '%s' and "
					"stat_date = '%s')",
                table_type, row1[0], row1[1], row1[2], row1[3], row1[4], row1[5],
				record_source, pkg_id, rule_set_id, rule_set_name, get_select_time(time_type));
    
    if ( trcLogFlag == TRCLEVEL_SQL ) 
    {
        sprintf(trcBuf, "update query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
        
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; query : %s; err=%s\n", query, mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
//        mysql_free_result(res1); // 10.07
        return -1;
    }

    return 1;
}

int stmd_RULESETInsert(int time_type, char *table_type, MYSQL_ROW inrow)
{
    char        query[4096];
    char    	weektype[32];

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
        sprintf(weektype, "%s", get_insert_week());
    }

    sprintf(query, "INSERT INTO %s VALUES "
	 				"('%s','%s','%s','%s','%s',"
					 "'%s','%s','%s','%s','%s',"
					 "'%s','%s')", table_type, \
					inrow[0],inrow[1],inrow[2],inrow[3],inrow[4],inrow[5],\
					inrow[6],inrow[7],inrow[8],inrow[9],get_select_time(time_type),weektype);
                                    
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; query : %s;  err=%s\n", query, mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
//	logPrint(trcLogId,FL, "RULE Update Insert : query=%s\n", query);

    return 1;
}

int stmd_exeUpdateHourRULESET()
{
    char    query[4096],insert_time[32];
    int     ret_select;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

	/*
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
            STM_STATISTIC_HOUR_RULESET_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*3)); // 3달 
			*/
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
            STM_STATISTIC_HOUR_RULESET_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_HOUR]));

    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "RULESET Statistic(Hour) mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
		return -1;
    }

///////////////////
    /*현재 시간을 구한다음 5분 table에서 정보를 select 하고
    row 만큼 loop -> upate / insert */
    sprintf (insert_time, "%s", get_insert_time());
    if(strcasecmp(insert_time,"") == 0)
    {
        sprintf(trcBuf, "RULESET Statistic(Hour) 5minute time get fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
	// sce / pkg_id / phbit / rule_set_id
    sprintf(query, "SELECT * FROM %s where ( stat_date = '%s') ",\
                STM_STATISTIC_5MINUTE_RULESET_TBL_NAME, insert_time);
	
//	logPrint(trcLogId, FL,"RULE Hour Update start query : %s\n", query);

	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    result1 = mysql_store_result(conn);
    
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
		memset(query, 0x00, sizeof(query));
        sprintf(query, "SELECT * FROM %s where (record_source = '%s' and pkg_id = %s and "
						" rule_set_id = '%s' and rule_set_name = '%s' and "
						" stat_date = '%s')", 
						STM_STATISTIC_HOUR_RULESET_TBL_NAME, 
						row1[0], row1[1], row1[2], row1[3], get_select_time(STMD_HOUR));


        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
            if ( stmd_RULESETUpdate(STMD_HOUR, STM_STATISTIC_HOUR_RULESET_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "RULE HOUR Update error: %s\n", query);
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_RULESETInsert(STMD_HOUR, STM_STATISTIC_HOUR_RULESET_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "RULE HOUR Insert error: %s\n", query);
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else 
        {
            sprintf(trcBuf,"stmd_exeUpdateHourRULESET Update/Insert Fail = %d:query=:%s\n",ret_select,query);
            trclib_writeLogErr (FL,trcBuf);
            mysql_free_result(result1);
            return -1;
        }
    }
    
    mysql_free_result(result1);

    return 1;
}

int stmd_exeUpdateDayRULESET()
{
    char    query[4096],insert_time[32];
    int     ret_select;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

	/*
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_DAY_RULESET_TBL_NAME, get_delete_time(STMD_1MON_OFFSET)); // 1년
		*/
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_DAY_RULESET_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_DAY]));

    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "RULESET Day Statistic mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
		return -1;
    }

///////////////////
    /*현재 시간을 구한다음 5분 table에서 정보를 select 하고
    row 만큼 loop -> upate / insert */
    sprintf(insert_time, "%s", get_insert_time());
    if(strcasecmp(insert_time,"") == 0)
    {
        sprintf(trcBuf, "RULESET Statistic(Day) 5minute time get fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
    sprintf(query, "SELECT * FROM %s where ( stat_date = '%s') ",\
                STM_STATISTIC_5MINUTE_RULESET_TBL_NAME, insert_time);
	
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    result1 = mysql_store_result(conn);
    
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
        sprintf(query, "SELECT * FROM %s where (record_source = '%s' and pkg_id = %s and "
						" rule_set_id = '%s' and rule_set_name = '%s' and "
						" stat_date = '%s')", 
						STM_STATISTIC_DAY_RULESET_TBL_NAME, 
						row1[0], row1[1], row1[2], row1[3], get_select_time(STMD_DAY));

//	logPrint(trcLogId, FL,"RULESET Day Update start query : %s\n", query);

        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
            if ( stmd_RULESETUpdate(STMD_DAY, STM_STATISTIC_DAY_RULESET_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "RULESET DAY Update error: %s\n", query);
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_RULESETInsert(STMD_DAY, STM_STATISTIC_DAY_RULESET_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "RULESET DAY Insert error: %s\n", query);
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else 
        {
            sprintf(trcBuf,"RULESET DAY Update/Insert Fail = %d\n",ret_select);
            trclib_writeLogErr (FL,trcBuf);
            mysql_free_result(result1);
            return -1;
        }
    }
    
    mysql_free_result(result1);

    return 1;
	
}

int stmd_exeUpdateWeekRULESET()
{
    char    query[4096],insert_time[32];
    int     ret_select;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

	/*
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_WEEK_RULESET_TBL_NAME, get_delete_time(STMD_1MON_OFFSET));
		*/
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_WEEK_RULESET_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_WEEK]));

    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "RULESET WEEK Statistic 1 Week mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
		return -1;
    }

///////////////////
    /*현재 시간을 구한다음 5분 table에서 정보를 select 하고
    row 만큼 loop -> upate / insert */
    sprintf (insert_time, "%s", get_insert_time());
    if(strcasecmp(insert_time,"") == 0)
    {
        sprintf(trcBuf, "RULESET Statistic(Week) 5minute time get fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
    sprintf(query, "SELECT * FROM %s where ( stat_date = '%s') ",\
                STM_STATISTIC_5MINUTE_RULESET_TBL_NAME, insert_time);
	
//	logPrint(trcLogId, FL,"RULE Week Update start query : %s\n", query);

	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    result1 = mysql_store_result(conn);
    
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
        sprintf(query, "SELECT * FROM %s where (record_source = '%s' and pkg_id = %s and "
						" rule_set_id = '%s' and rule_set_name = '%s' and "
						" stat_date = '%s')", 
						STM_STATISTIC_WEEK_RULESET_TBL_NAME, 
						row1[0], row1[1], row1[2], row1[3], get_select_time(STMD_WEEK));

//	logPrint(trcLogId, FL,"RULE Week Update start query : %s\n", query);

        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
            if ( stmd_RULESETUpdate(STMD_WEEK, STM_STATISTIC_WEEK_RULESET_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "RULESET WEEK Update error: %s\n", query);
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_RULESETInsert(STMD_WEEK, STM_STATISTIC_WEEK_RULESET_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "RULESET WEEK Insert error: %s\n", query);
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else 
        {
            sprintf(trcBuf,"stmd_exeUpdateWeekRULESET Update/Insert Fail = %d\n",ret_select);
            trclib_writeLogErr (FL,trcBuf);
            mysql_free_result(result1);
            return -1;
        }
    }
    
    mysql_free_result(result1);

///////////////////

    return 1;
}

int stmd_exeUpdateMonthRULESET()
{
    char    query[4096],insert_time[32];
    int     ret_select;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

    // 1년 전의 한달 데이타를 지운다.
	/*
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_MONTH_RULESET_TBL_NAME, get_delete_time(STMD_1MON_OFFSET));
		*/
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_MONTH_RULESET_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_MONTH]));

    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "RULESET Month Statistic  mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
		return -1;
    }

///////////////////
    /*현재 시간을 구한다음 5분 table에서 정보를 select 하고
    row 만큼 loop -> upate / insert */
    sprintf (insert_time, "%s", get_insert_time());
    if(strcasecmp(insert_time,"") == 0)
    {
        sprintf(trcBuf, "RULESET Statistic(Week) 5minute time get fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
    sprintf(query, "SELECT * FROM %s where ( stat_date = '%s') ",\
                STM_STATISTIC_5MINUTE_RULESET_TBL_NAME, insert_time);
	
//	logPrint(trcLogId, FL,"RULE Month Update start query : %s\n", query);

	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    result1 = mysql_store_result(conn);
    
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
        sprintf(query, "SELECT * FROM %s where (record_source = '%s' and pkg_id = %s and "
						" rule_set_id = '%s' and rule_set_name = '%s' and "
						" stat_date = '%s')", 
						STM_STATISTIC_MONTH_RULESET_TBL_NAME, 
						row1[0], row1[1], row1[2], row1[3], get_select_time(STMD_MONTH));
		
//	logPrint(trcLogId, FL,"RULESET Month Update start query : %s\n", query);

        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
            if ( stmd_RULESETUpdate(STMD_MONTH, STM_STATISTIC_MONTH_RULESET_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "RULESET MONTH Update error: %s\n", query);
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_RULESETInsert(STMD_MONTH, STM_STATISTIC_MONTH_RULESET_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "RULESET MONTH Insert error: %s\n", query);
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else 
        {
            sprintf(trcBuf,"stmd_exeUpdateMonthRULESET Update/Insert Fail = %d\n",ret_select);
            trclib_writeLogErr (FL,trcBuf);
            mysql_free_result(result1);
            return -1;
        }
    }

    mysql_free_result(result1);

///////////////////

    return 1;
}
