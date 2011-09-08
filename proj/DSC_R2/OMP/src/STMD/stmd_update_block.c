#include "stmd_proto.h"

extern  char    trcBuf[4096], trcTmp[1024];
extern  int     trcLogFlag ;
extern  MYSQL   sql, *conn;
extern  int     sysCnt;
extern  STM_CommStatisticInfo    StatisticSystemInfo[SYSCONF_MAX_ASSO_SYS_NUM];
extern int trcLogId;
extern short 	delTIME[STMD_PERIOD_TYPE_NUM];

extern RuleSetList        g_stSCERule[MAX_SCE_NUM];

int stmd_BLOCKUpdate(int time_type, char *table_type, MYSQL_ROW inrow)
{
    char        query[4096];
    char        starttime[32],endtime[32],seltbl[32]; 
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;
	char		record_source[32]={0,}, subscriber_id[32]={0,}, package_id[10]={0,};  
	char		service_id[10]={0,}, protocol_id[10]={0,}, initiating_side[2] = {0,}; 
	char		block_reason[4] = {0,}, redirected[2] = {0,};

	memset (seltbl, 0x00, sizeof(seltbl));

	sprintf(record_source, "%s", inrow[0]);
	sprintf(subscriber_id, "%s", inrow[1]);
	sprintf(package_id, "%s", inrow[2]);
	sprintf(service_id, "%s", inrow[3]);
	sprintf(protocol_id, "%s", inrow[4]);
	sprintf(initiating_side, "%s", inrow[5]);
	sprintf(block_reason, "%s", inrow[6]);
	sprintf(redirected, "%s", inrow[8]);
    
    sprintf(starttime, "%s", get_select_time2(time_type));
    sprintf(endtime, "%s", get_period_end_time2(time_type));    
    switch(time_type)
    {
    case STMD_HOUR:
        strcpy(seltbl, STM_STATISTIC_5MINUTE_BLOCK_TBL_NAME);
        break;
    case STMD_DAY:
        strcpy(seltbl, STM_STATISTIC_HOUR_BLOCK_TBL_NAME);
        break;
    case STMD_WEEK:
        strcpy(seltbl, STM_STATISTIC_DAY_BLOCK_TBL_NAME);
        break;
    case STMD_MONTH:
        strcpy(seltbl, STM_STATISTIC_DAY_BLOCK_TBL_NAME);
        break;
    default:
        strcpy(seltbl, STM_STATISTIC_HOUR_BLOCK_TBL_NAME);
        break;
    }

    if (time_type == STMD_HOUR || time_type == STMD_DAY){
		sprintf(query, "SELECT "
                " IFNULL(SUM(block_rdr_cnt),0), "
                " IFNULL(SUM(stat_cnt),0) "
                " FROM %s "
				" WHERE (record_source = '%s' and "
					"subscriber_id = '%s' and package_id = %s and "
					"service_id = %s and protocol_id  = %s and "
					"initiating_side = %s and block_reason = %s and redirected = %s and "
					"stat_date > '%s' and stat_date <= '%s') "\
				" Group by record_source,subscriber_id,package_id,service_id,protocol_id,"\
				" 	initiating_side,block_reason,redirected",\
                seltbl, record_source, subscriber_id, package_id, service_id, \
				protocol_id, initiating_side, block_reason, redirected, starttime, endtime);
    }else{
		sprintf(query, "SELECT "
                " IFNULL(SUM(block_rdr_cnt),0), "
                " IFNULL(SUM(stat_cnt),0) "
                " FROM %s "
				" WHERE (record_source = '%s' and "
					"subscriber_id = '%s' and package_id = %s and "
					"service_id = %s and protocol_id  = %s and "
					"initiating_side = %s and block_reason = %s and redirected = %s and "
					"stat_date >= '%s' and stat_date < '%s') "\
				" Group by record_source,subscriber_id,package_id,service_id,protocol_id,"\
				" 	initiating_side,block_reason,redirected",\
                seltbl, record_source, subscriber_id, package_id, service_id, \
				protocol_id, initiating_side, block_reason, redirected, starttime, endtime);
    } 
//	logPrint(trcLogId,FL,"BLOCK Update : query=%s\n", query);

	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; query=%s; err=%s\n", query, mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
    result1 = mysql_store_result(conn);
    row1 = mysql_fetch_row(result1); 

    if( row1== NULL )
    {
        sprintf(trcBuf, "[%d]%s BLOCK 5MIN select is NULL: %s\n", time_type, seltbl, query);
        trclib_writeLogErr (FL, trcBuf);
		mysql_free_result(result1);
        return -1;
    }
    
    // 데이타를 update한다.
	sprintf(query, "UPDATE %s SET "
                " block_rdr_cnt = %s, "
                " stat_cnt = %s "
				" WHERE (record_source = '%s' and "
					"subscriber_id = '%s' and package_id = %s and "
					"service_id = %s and protocol_id  = %s and "
					"initiating_side = %s and block_reason = %s and redirected = %s and "
					"stat_date = '%s')",\
                table_type, row1[0], row1[1], record_source, subscriber_id, package_id, service_id, \
				protocol_id, initiating_side, block_reason, redirected, get_select_time(time_type));
    
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

int stmd_BLOCKInsert(int time_type, char *table_type, MYSQL_ROW inrow)
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
        sprintf(weektype, "%s", get_insert_week());
    }

    sprintf(query, "INSERT INTO %s VALUES "
	 				"('%s','%s','%s','%s','%s',"
					 "'%s','%s','%s','%s','%s',"
					 "'%s','%s')", table_type, \
					inrow[0],inrow[1],inrow[2],inrow[3],inrow[4], 
					inrow[5],inrow[6],inrow[7],inrow[8],inrow[9], 
					get_select_time(time_type),weektype);
                                    
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; query : %s;  err=%s\n", query, mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
//	logPrint(trcLogId,FL, "BLOCK Update Insert : query=%s\n", query);

    return 1;
}

int stmd_exeUpdateHourBLOCK(void)
{
    char    query[4096],insert_time[32];
    int     ret_select;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

	/*
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
            STM_STATISTIC_HOUR_BLOCK_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*3)); // 3달 
			*/
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
            STM_STATISTIC_HOUR_BLOCK_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_HOUR]));
    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
	
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "BLOCK Statistic(Hour) mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
		return -1;
    }

///////////////////
    /*현재 시간을 구한다음 5분 table에서 정보를 select 하고
    row 만큼 loop -> upate / insert */
    sprintf(insert_time, "%s", get_insert_time());
    if(strcasecmp(insert_time,"") == 0)
    {
        sprintf(trcBuf, "BLOCK Statistic(Hour) 5minute time get fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
	/*
    sprintf(query, "SELECT record_source,subscriber_id,package_id,service_id,protocol_id, "\
					"initiating_side,block_reason,sum(block_rdr_cnt),redirected,sum(stat_cnt), "\
					"stat_date, max(stat_week) "\
					"FROM %s where ( stat_date = '%s') "\
					"Group by record_source,subscriber_id,package_id,service_id,protocol_id,"\
					"initiating_side,block_reason,redirected",\
                STM_STATISTIC_5MINUTE_BLOCK_TBL_NAME, insert_time);
				*/
    sprintf(query, "SELECT * FROM %s where ( stat_date = '%s') ",\
                STM_STATISTIC_5MINUTE_BLOCK_TBL_NAME, insert_time);
	
//	logPrint(trcLogId, FL,"BLOCK Hour Update start query : %s\n", query);

	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    result1 = mysql_store_result(conn);
    
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
        sprintf(query, "SELECT * FROM %s where (record_source = '%s' and "
						"subscriber_id = '%s' and package_id = %s and "
						"service_id = %s and protocol_id = %s and "
						"initiating_side = %s and block_reason = %s and "
						"redirected = %s and stat_date = '%s')", \
                    STM_STATISTIC_HOUR_BLOCK_TBL_NAME, row1[0], row1[1], row1[2], \
					row1[3], row1[4], row1[5], row1[6], row1[8], get_select_time(STMD_HOUR));

//		logPrint(trcLogId, FL,"BLOCK Hour Update find query : %s\n", query);

        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
            if ( stmd_BLOCKUpdate(STMD_HOUR, STM_STATISTIC_HOUR_BLOCK_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "BLOCK HOUR Update error: %s\n", query);
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_BLOCKInsert(STMD_HOUR, STM_STATISTIC_HOUR_BLOCK_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "BLOCK HOUR Insert error: %s\n", query);
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else 
        {
            sprintf(trcBuf,"stmd_exeUpdateHourBLOCK Update/Insert Fail = %d:query=:%s\n",ret_select,query);
            trclib_writeLogErr (FL,trcBuf);
            mysql_free_result(result1);
            return -1;
        }
    }
    
    mysql_free_result(result1);

    return 1;
}

int stmd_exeUpdateDayBLOCK()
{
    char    query[4096],insert_time[32];
    int     ret_select;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

	/*
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_DAY_BLOCK_TBL_NAME, get_delete_time(STMD_1MON_OFFSET)); // 1년
		*/
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_DAY_BLOCK_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_DAY]));
    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "BLOCK Day Statistic mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
		return -1;
    }

///////////////////
    /*현재 시간을 구한다음 5분 table에서 정보를 select 하고
    row 만큼 loop -> upate / insert */
    sprintf (insert_time, "%s",get_insert_time());
    if(strcasecmp(insert_time,"") == 0)
    {
        sprintf(trcBuf, "BLOCK Statistic(Day) 5minute time get fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
	/*
    sprintf(query, "SELECT record_source,subscriber_id,package_id,service_id,protocol_id, "\
					"initiating_side,block_reason,sum(block_rdr_cnt),redirected,sum(stat_cnt), "\
					"stat_date, max(stat_week) "\
					"FROM %s where ( stat_date = '%s') "\
					"Group by record_source,subscriber_id,package_id,service_id,protocol_id,"\
					"initiating_side,block_reason,redirected",\
                STM_STATISTIC_5MINUTE_BLOCK_TBL_NAME, insert_time);
				*/
    sprintf(query, "SELECT * FROM %s where ( stat_date = '%s') ",\
                STM_STATISTIC_5MINUTE_BLOCK_TBL_NAME, insert_time);
	
//	logPrint(trcLogId, FL,"BLOCK Day Update start query : %s\n", query);

	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    result1 = mysql_store_result(conn);
    
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
        sprintf(query, "SELECT * FROM %s where (record_source = '%s' and "
						"subscriber_id = '%s' and package_id = %s and "
						"service_id = %s and protocol_id = %s and "
						"initiating_side = %s and block_reason = %s and "
						"redirected = %s and stat_date = '%s')", \
                    STM_STATISTIC_DAY_BLOCK_TBL_NAME, row1[0], row1[1], row1[2], \
					row1[3], row1[4], row1[5], row1[6], row1[8], get_select_time(STMD_DAY));

        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
            if ( stmd_BLOCKUpdate(STMD_DAY, STM_STATISTIC_DAY_BLOCK_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "BLOCK DAY Update error: %s\n", query);
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_BLOCKInsert(STMD_DAY, STM_STATISTIC_DAY_BLOCK_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "BLOCK DAY Insert error: %s\n", query);
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else 
        {
            sprintf(trcBuf,"stmd_exeUpdateDay Update/Insert Fail = %d\n",ret_select);
            trclib_writeLogErr (FL,trcBuf);
            mysql_free_result(result1);
            return -1;
        }
    }
    
    mysql_free_result(result1);

    return 1;
	
}

int stmd_exeUpdateWeekBLOCK()
{
    char    query[4096],insert_time[32];
    int     ret_select;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

	/*
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_WEEK_BLOCK_TBL_NAME, get_delete_time(STMD_1MON_OFFSET));
		*/
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_WEEK_BLOCK_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_WEEK]));
    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "BLOCK WEEK Statistic 1 Week mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
		return -1;
    }

///////////////////
    /*현재 시간을 구한다음 5분 table에서 정보를 select 하고
    row 만큼 loop -> upate / insert */
    sprintf(insert_time, "%s", get_insert_time());
    if(strcasecmp(insert_time,"") == 0)
    {
        sprintf(trcBuf, "BLOCK Statistic(Week) 5minute time get fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
	/*
    sprintf(query, "SELECT record_source,subscriber_id,package_id,service_id,protocol_id, "\
					"initiating_side,block_reason,sum(block_rdr_cnt),redirected,sum(stat_cnt), "\
					"stat_date, max(stat_week) "\
					"FROM %s where ( stat_date = '%s') "\
					"Group by record_source,subscriber_id,package_id,service_id,protocol_id,"\
					"initiating_side,block_reason,redirected",\
                STM_STATISTIC_5MINUTE_BLOCK_TBL_NAME, insert_time);
				*/
    sprintf(query, "SELECT * FROM %s where ( stat_date = '%s') ",\
                STM_STATISTIC_5MINUTE_BLOCK_TBL_NAME, insert_time);
	
//	logPrint(trcLogId, FL,"BLOCK Week Update start query : %s\n", query);

	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    result1 = mysql_store_result(conn);
    
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
        sprintf(query, "SELECT * FROM %s where (record_source = '%s' and "
						"subscriber_id = '%s' and package_id = %s and "
						"service_id = %s and protocol_id = %s and "
						"initiating_side = %s and block_reason = %s and "
						"redirected = %s and stat_date = '%s')", \
                    STM_STATISTIC_WEEK_BLOCK_TBL_NAME, row1[0], row1[1], row1[2], \
					row1[3], row1[4], row1[5], row1[6], row1[8], get_select_time(STMD_WEEK));

        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
            if ( stmd_BLOCKUpdate(STMD_WEEK, STM_STATISTIC_WEEK_BLOCK_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "BLOCK WEEK Update error: %s\n", query);
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_BLOCKInsert(STMD_WEEK, STM_STATISTIC_WEEK_BLOCK_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "BLOCK WEEK Insert error: %s\n", query);
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else 
        {
            sprintf(trcBuf,"stmd_exeUpdateWeek Update/Insert Fail = %d\n",ret_select);
            trclib_writeLogErr (FL,trcBuf);
            mysql_free_result(result1);
            return -1;
        }
    }
    
    mysql_free_result(result1);

///////////////////

    return 1;
}

int stmd_exeUpdateMonthBLOCK()
{
    char    query[4096],insert_time[32];
    int     ret_select;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

    // 1년 전의 한달 데이타를 지운다.
	/*
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_MONTH_BLOCK_TBL_NAME, get_delete_time(STMD_1MON_OFFSET));
		*/
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_MONTH_BLOCK_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_MIN]));
    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "BLOCK Month Statistic  mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
		return -1;
    }

///////////////////
    /*현재 시간을 구한다음 5분 table에서 정보를 select 하고
    row 만큼 loop -> upate / insert */
    sprintf(insert_time, "%s",get_insert_time());
    if(strcasecmp(insert_time,"") == 0)
    {
        sprintf(trcBuf, "BLOCK Statistic(Week) 5minute time get fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
   
	/*
    sprintf(query, "SELECT record_source,subscriber_id,package_id,service_id,protocol_id, "\
					"initiating_side,block_reason,sum(block_rdr_cnt),redirected,sum(stat_cnt), "\
					"stat_date, max(stat_week) "\
					"FROM %s where ( stat_date = '%s') "\
					"Group by record_source,subscriber_id,package_id,service_id,protocol_id,"\
					"initiating_side,block_reason,redirected",\
                STM_STATISTIC_5MINUTE_BLOCK_TBL_NAME, insert_time);
				*/
    sprintf(query, "SELECT * FROM %s where ( stat_date = '%s') ",\
                STM_STATISTIC_5MINUTE_BLOCK_TBL_NAME, insert_time);
	
//	logPrint(trcLogId, FL,"BLOCK Month Update start query : %s\n", query);

	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    result1 = mysql_store_result(conn);
    
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
        sprintf(query, "SELECT * FROM %s where (record_source = '%s' and "
						"subscriber_id = '%s' and package_id = %s and "
						"service_id = %s and protocol_id = %s and "
						"initiating_side = %s and block_reason = %s and "
						"redirected = %s and stat_date = '%s')", \
                    STM_STATISTIC_MONTH_BLOCK_TBL_NAME, row1[0], row1[1], row1[2], \
					row1[3], row1[4], row1[5], row1[6], row1[8], get_select_time(STMD_MONTH));

        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
            if ( stmd_BLOCKUpdate(STMD_MONTH, STM_STATISTIC_MONTH_BLOCK_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "BLOCK MONTH Update error: %s\n", query);
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_BLOCKInsert(STMD_MONTH, STM_STATISTIC_MONTH_BLOCK_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "BLOCK MONTH Insert error: %s\n", query);
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else 
        {
            sprintf(trcBuf,"stmd_exeUpdateMonth Update/Insert Fail = %d\n",ret_select);
            trclib_writeLogErr (FL,trcBuf);
            mysql_free_result(result1);
            return -1;
        }
    }

    mysql_free_result(result1);

///////////////////

    return 1;
}
