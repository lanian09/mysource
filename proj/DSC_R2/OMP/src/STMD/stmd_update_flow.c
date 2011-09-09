#include "stmd_proto.h"

extern  char    trcBuf[4096], trcTmp[1024];
extern  int     trcLogFlag ;
extern  MYSQL   sql, *conn;
extern 	short	delTIME[STMD_PERIOD_TYPE_NUM];

int stmd_FlowUpdate(int time_type, char *table_type, MYSQL_ROW inrow)
{
    char        query[4096*2];
    char        starttime[32],endtime[32],seltbl[32],sysname[12];
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

	memset (seltbl, 0x00, sizeof(seltbl));
	sprintf(sysname, "%s", inrow[0]);
    sprintf(starttime, "%s", get_select_time2(time_type));
    sprintf(endtime, "%s", get_period_end_time2(time_type));    

    switch(time_type)
    {
		case STMD_HOUR:
			strcpy(seltbl, STM_STATISTIC_5MINUTE_FLOW_TBL_NAME);
			break;
		case STMD_DAY:
			strcpy(seltbl, STM_STATISTIC_HOUR_FLOW_TBL_NAME);
			break;
		case STMD_WEEK:
			strcpy(seltbl, STM_STATISTIC_DAY_FLOW_TBL_NAME);
			break;
		case STMD_MONTH:
			strcpy(seltbl, STM_STATISTIC_DAY_FLOW_TBL_NAME);
			break;
		default:
			strcpy(seltbl, STM_STATISTIC_HOUR_FLOW_TBL_NAME);
			break;
    }

    if (time_type == STMD_HOUR || time_type == STMD_DAY) {
    	sprintf(query, "SELECT "
                " IFNULL(AVG(avg_flow),0), "
                " IFNULL(MIN(min_flow),0), "
                " IFNULL(MAX(max_flow),0), "
                " IFNULL(SUM(stat_cnt),0) "
                " FROM %s WHERE system_name = '%s' "
				" and (stat_date > '%s' and stat_date <= '%s')",
                seltbl, sysname, starttime, endtime);
    }else{
	sprintf(query, "SELECT "
                " IFNULL(AVG(avg_flow),0), "
                " IFNULL(MIN(min_flow),0), "
                " IFNULL(MAX(max_flow),0), "
                " IFNULL(SUM(stat_cnt),0) "
                " FROM %s WHERE system_name = '%s' "
				" and (stat_date >= '%s' and stat_date < '%s')",
                seltbl, sysname, starttime, endtime);
    } 

	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s:%s\n", mysql_error(conn), query);
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
    result1 = mysql_store_result(conn);
    row1 = mysql_fetch_row(result1); 

    if( row1 == NULL )
    {
        sprintf(trcBuf, "[%d]%s Flow select error\n%s\n", time_type, seltbl, query);
        trclib_writeLogErr (FL, trcBuf);
		mysql_free_result(result1); 
        return -1;
    }
    
    // ����Ÿ�� update�Ѵ�.
    sprintf(query, "UPDATE %s SET " 
                    " avg_flow = %s, min_flow = %s, max_flow = %s, stat_cnt = %s "
                    " WHERE system_name = '%s' and (stat_date = '%s')", 
                    table_type,
                    row1[0], row1[1], row1[2], row1[3],
					sysname, get_select_time(time_type));
    
    if ( trcLogFlag == TRCLEVEL_SQL ) 
    {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
        
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s:%s\n", mysql_error(conn), query);
        trclib_writeLogErr (FL,trcBuf);
        mysql_free_result(result1);
        return -1;
    }

    mysql_free_result(result1);

    return 1;
}

int stmd_FlowInsert(int time_type, char *table_type, MYSQL_ROW inrow)
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
        sprintf(weektype,"%s", get_insert_week2());
    }
    else
    {
        sprintf(weektype,"%s", get_insert_week());
    }

    sprintf(query, "INSERT INTO %s VALUES ("
                                    " '%s', "
									" '%s', '%s', '%s', '%s', "
									" '%s', '%s' )", 
                                    table_type,
                                    inrow[0], 
									inrow[1], inrow[2], inrow[3], inrow[4], 
									get_select_time(time_type), weektype );
                                    
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n%s\n", mysql_error(conn), query);
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }

    return 1;
}

int stmd_exeUpdateHourFlow()
{
    char    query[4096];
	char	insert_time[32];
    int     ret_select;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
            STM_STATISTIC_HOUR_FLOW_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_HOUR]));

    if ( trcLogFlag == TRCLEVEL_SQL ) 
	{
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }

    if (stmd_mysql_query (query) < 0) 
	{
        sprintf(trcBuf, "FlowStatistic 1 Hour mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
		return -1;
    }

    /*���� �ð��� ���Ѵ��� 5�� table���� ������ select �ϰ�
    row ��ŭ loop -> upate / insert */
    sprintf(insert_time, "%s", get_insert_time());
    if(strcasecmp(insert_time,"") == 0)
    {
        sprintf(trcBuf, "Flow Statistic(Hour) 5minute time get fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
    sprintf(query, "SELECT * FROM %s where ( stat_date = '%s')",
                STM_STATISTIC_5MINUTE_FLOW_TBL_NAME, insert_time);

	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    result1 = mysql_store_result(conn);
    
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
        sprintf(query, "SELECT * FROM %s where (system_name = '%s' and stat_date = '%s')",
                    STM_STATISTIC_HOUR_FLOW_TBL_NAME, row1[0], get_select_time(STMD_HOUR));

        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
            if ( stmd_FlowUpdate(STMD_HOUR, STM_STATISTIC_HOUR_FLOW_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "FLOW HOUR Update error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_FlowInsert(STMD_HOUR, STM_STATISTIC_HOUR_FLOW_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "FLOW HOUR Insert error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else 
        {
            sprintf(trcBuf,"stmd_exeUpdateHourFlow Update/Insert Fail = %d\n", ret_select);
            trclib_writeLogErr (FL,trcBuf);
            mysql_free_result(result1);
            return -1;
        }
    }
    
    mysql_free_result(result1);

    return 1;
}

int stmd_exeUpdateDayFlow()
{
    char    query[4096],insert_time[32];
    int     ret_select;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_DAY_FLOW_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_DAY]));

    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }

    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "Flow 1 Day mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
		return -1;
    }

///////////////////
    /*���� �ð��� ���Ѵ���  
    5�� table���� ������ select �ϰ�
    row ��ŭ loop -> upate / insert */
    sprintf(insert_time, "%s", get_insert_time()); 
	if(strcasecmp(insert_time,"") == 0)
    {
        sprintf(trcBuf, "Flow Statistic(Day) preview 5minute time get fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
    sprintf(query, "SELECT * FROM %s where (stat_date = '%s')",
                STM_STATISTIC_5MINUTE_FLOW_TBL_NAME, insert_time);
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }

    result1 = mysql_store_result(conn);
    
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
        sprintf(query, "SELECT * FROM %s where (system_name = '%s' and stat_date = '%s')",
                    STM_STATISTIC_DAY_FLOW_TBL_NAME, row1[0], get_select_time(STMD_DAY));

        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
            if ( stmd_FlowUpdate(STMD_DAY, STM_STATISTIC_DAY_FLOW_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "DEL DAY Update error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_FlowInsert(STMD_DAY, STM_STATISTIC_DAY_FLOW_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "DEL DAY Insert error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else 
        {
            sprintf(trcBuf,"stmd_exeUpdateDayFlow Update/Insert Fail = %d\n",ret_select);
            trclib_writeLogErr (FL,trcBuf);
            mysql_free_result(result1);
            return -1;
        }
    }
    
    mysql_free_result(result1);

///////////////////

    return 1;
}


int stmd_exeUpdateWeekFlow()
{
    char    query[4096],insert_time[32];
    int     ret_select;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_WEEK_FLOW_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_WEEK]));

    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "FlowStatistic 1 Week mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
		return -1;
    }

///////////////////
    /*���� �ð����� 5���� �ð��� ���Ѵ���
    5�� table���� ������ select �ϰ�
    row ��ŭ loop -> upate / insert */

    sprintf(insert_time, "%s", get_insert_time());
    if(strcasecmp(insert_time,"") == 0)
    {
        sprintf(trcBuf, "Flow Statistic(Week) preview 5minute time get fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
    sprintf(query, "SELECT * FROM %s where (stat_date = '%s')",
                STM_STATISTIC_5MINUTE_FLOW_TBL_NAME, insert_time);
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    result1 = mysql_store_result(conn);
    
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
        sprintf(query, "SELECT * FROM %s where (system_name = '%s' and stat_date = '%s')",
                    STM_STATISTIC_WEEK_FLOW_TBL_NAME, row1[0], get_select_time(STMD_WEEK));

        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
            if ( stmd_FlowUpdate(STMD_WEEK, STM_STATISTIC_WEEK_FLOW_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "FLOW WEEK Update error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_FlowInsert(STMD_WEEK, STM_STATISTIC_WEEK_FLOW_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "FLOW WEEK Insert error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else 
        {
            sprintf(trcBuf,"stmd_exeUpdateWeekFlow Update/Insert Fail = %d\n",ret_select);
            trclib_writeLogErr (FL,trcBuf);
            mysql_free_result(result1);
            return -1;
        }
    }
    
    mysql_free_result(result1);

///////////////////

    return 1;
}

int stmd_exeUpdateMonthFlow()
{
    char    query[4096],insert_time[32];
    int     ret_select;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

    // 1�� ���� �Ѵ� ����Ÿ�� �����.
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_MONTH_FLOW_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_MONTH]));

    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "FlowStatistic 1 Month mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
		return -1;
    }

    sprintf(insert_time, "%s", get_insert_time());
    if(strcasecmp(insert_time,"") == 0)
    {
        sprintf(trcBuf, "Flow Statistic(Month) preview 5minute time get fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
    sprintf(query, "SELECT * FROM %s where (stat_date = '%s')",
                STM_STATISTIC_5MINUTE_FLOW_TBL_NAME, insert_time);
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }

    result1 = mysql_store_result(conn);
    
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
        sprintf(query, "SELECT * FROM %s where (system_name = '%s' and stat_date = '%s')",
                    STM_STATISTIC_MONTH_FLOW_TBL_NAME, row1[0], get_select_time(STMD_MONTH));

        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
            if ( stmd_FlowUpdate(STMD_MONTH, STM_STATISTIC_MONTH_FLOW_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "DEL MONTH Update error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_FlowInsert(STMD_MONTH, STM_STATISTIC_MONTH_FLOW_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "DEL MONTH Insert error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else 
        {
            sprintf(trcBuf,"stmd_exeUpdateMonthFlow Update/Insert Fail = %d\n",ret_select);
            trclib_writeLogErr (FL,trcBuf);
            mysql_free_result(result1);
            return -1;
        }
    }
    
    mysql_free_result(result1);

    return 1;
}
