#include "stmd_proto.h"

extern  char    trcBuf[4096], trcTmp[1024];
extern  int     trcLogFlag ;
extern  MYSQL   sql, *conn;
extern  int     sysCnt;
extern  STM_CommStatisticInfo    StatisticSystemInfo[SYSCONF_MAX_ASSO_SYS_NUM];
extern int trcLogId;

int stmd_TRUpdate(int time_type, char *table_type, MYSQL_ROW inrow)
{
    char        query[4096];
    char        starttime[32],endtime[32],seltbl[32]; // by jjinri 2009.04.20 ,sysname[12];
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;
	char		record_source[32]={0,}, package_id[10]={0,};
	char		protocol_id[10]={0,}, service_id[10]={0,};

	memset (seltbl, 0x00, sizeof(seltbl));
	sprintf(record_source, "%s", inrow[0]);
	sprintf(package_id, "%s", inrow[1]);
	sprintf(service_id, "%s", inrow[2]);
	sprintf(protocol_id, "%s", inrow[3]);
    
    sprintf(starttime, "%s", get_select_time2(time_type));
    sprintf(endtime, "%s", get_period_end_time2(time_type));    
    switch(time_type)
    {
    case STMD_HOUR:
        strcpy(seltbl, STM_STATISTIC_5MINUTE_TR_TBL_NAME);
        break;
    case STMD_DAY:
        strcpy(seltbl, STM_STATISTIC_HOUR_TR_TBL_NAME);
        break;
    case STMD_WEEK:
        strcpy(seltbl, STM_STATISTIC_DAY_TR_TBL_NAME);
        break;
    case STMD_MONTH:
        strcpy(seltbl, STM_STATISTIC_DAY_TR_TBL_NAME);
        break;
    default:
        strcpy(seltbl, STM_STATISTIC_HOUR_TR_TBL_NAME);
        break;
    }

    if (time_type == STMD_HOUR || time_type == STMD_DAY){
		sprintf(query, "SELECT "
                " IFNULL(SUM(upstream_volume),0), "
                " IFNULL(SUM(downstream_volume),0), "
                " IFNULL(SUM(stat_cnt),0) "
                " FROM %s "
				" WHERE (record_source = '%s' and "
				"package_id = %s and "
				"service_id = %s and protocol_id = %s and "
				"stat_date > '%s' and stat_date <= '%s')"
				" Group by record_source,package_id,service_id,protocol_id",\
                seltbl, record_source, package_id, service_id, protocol_id, \
				starttime, endtime);
    }else{
		sprintf(query, "SELECT "
                " IFNULL(SUM(upstream_volume),0), "
                " IFNULL(SUM(downstream_volume),0), "
                " IFNULL(SUM(stat_cnt),0) "
                " FROM %s "
				" WHERE (record_source = '%s' and "
					"package_id = %s and service_id = %s and protocol_id = %s and "
					"stat_date >= '%s' and stat_date < '%s') "
				" Group by record_source,package_id,service_id,protocol_id",
                seltbl, record_source, package_id, service_id, protocol_id, \
				starttime, endtime);
    } 
//	logPrint(trcLogId,FL,"TR Update : query=%s\n", query);

	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; query=%s; err=%s\n", query, mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
    result1 = mysql_store_result(conn);
    row1 = mysql_fetch_row(result1); 

    if( row1 == NULL )
    {
        sprintf(trcBuf, "[%d]%s TR 5MIN select error: %s\n", time_type, seltbl, query);
        trclib_writeLogErr (FL, trcBuf);
		mysql_free_result(result1);
        return -1;
    }
    
    // ����Ÿ�� update�Ѵ�.
    sprintf(query, "UPDATE %s SET " 
                    	"upstream_volume = %s,"
                    	"downstream_volume = %s,"
                    	"stat_cnt = %s "
				" WHERE (stat_date = '%s' and record_source = '%s' and "
					"package_id = %s and service_id = %s and protocol_id = %s) ",\
                    table_type, row1[0],row1[1], row1[2],\
                    get_select_time(time_type), record_source, package_id,  \
					service_id, protocol_id);
    
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

int stmd_TRInsert(int time_type, char *table_type, MYSQL_ROW inrow)
{
    char        query[4096];
    char    	weektype[32];

///////////////////////////
    memset(weektype,0,sizeof(weektype));
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

    sprintf(query, "INSERT INTO %s VALUES "
	 				"('%s','%s','%s','%s','%s',"
					 "'%s','%s','%s','%s')", table_type, \
					inrow[0],inrow[1],inrow[2],inrow[3],inrow[4], \
					inrow[5],inrow[6],get_select_time(time_type),weektype);
                                    
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; query : %s;  err=%s\n", query, mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
//	logPrint(trcLogId,FL, "TR Update Insert : query=%s\n", query);

    return 1;
}

int stmd_exeUpdateHourTR(t_start)
{
    char    query[4096],insert_time[32];
    int     ret_select;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
            STM_STATISTIC_HOUR_TR_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*3)); // 3�� 
    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "TR Statistic(Hour) mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
    }

///////////////////
    /*���� �ð��� ���Ѵ��� 5�� table���� ������ select �ϰ�
    row ��ŭ loop -> upate / insert */
    sprintf(insert_time, "%s", get_insert_time());
    if(strcasecmp(insert_time,"") == 0)
    {
        sprintf(trcBuf, "TR Statistic(Hour) 5minute time get fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
	/*
    sprintf(query, "SELECT record_source,package_id,service_id,protocol_id,sum(upstream_volume),"\
					"sum(downstream_volume),sum(stat_cnt),stat_date,max(stat_date) "\
					"FROM %s where ( stat_date = '%s') " \
					"Group by record_source,package_id,service_id,protocol_id,stat_date",\
                STM_STATISTIC_5MINUTE_TR_TBL_NAME, insert_time);
				*/
    sprintf(query, "SELECT * FROM %s where ( stat_date = '%s') ", \
                STM_STATISTIC_5MINUTE_TR_TBL_NAME, insert_time);
	
//	logPrint(trcLogId, FL,"TR Hour Update start query : %s\n", query);

	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    result1 = mysql_store_result(conn);
    
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
        sprintf(query, "SELECT * FROM %s where (record_source = '%s' and "
						"package_id = %s and service_id = %s and protocol_id = %s and "
						"stat_date = '%s')", \
                    STM_STATISTIC_HOUR_TR_TBL_NAME, row1[0], row1[1], row1[2], \
					row1[3], get_select_time(STMD_HOUR));
        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
            if ( stmd_TRUpdate(STMD_HOUR, STM_STATISTIC_HOUR_TR_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "TR HOUR Update error: %s\n", query);
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_TRInsert(STMD_HOUR, STM_STATISTIC_HOUR_TR_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "TR HOUR Insert error: %s\n", query);
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else 
        {
            sprintf(trcBuf,"stmd_exeUpdateHourTR Update/Insert Fail = %d:query=:%s\n",ret_select,query);
            trclib_writeLogErr (FL,trcBuf);
            mysql_free_result(result1);
            return -1;
        }
    }
    
    mysql_free_result(result1);

    return 1;
}

int stmd_exeUpdateDayTR()
{
    char    query[4096],insert_time[32];
    int     ret_select;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_DAY_TR_TBL_NAME, get_delete_time(STMD_1MON_OFFSET)); // 1��
    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "TR Day Statistic mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
    }

///////////////////
    /*���� �ð��� ���Ѵ��� 5�� table���� ������ select �ϰ�
    row ��ŭ loop -> upate / insert */
    sprintf (insert_time, "%s", get_insert_time());
    if(strcasecmp(insert_time,"") == 0)
    {
        sprintf(trcBuf, "TR Statistic(Day) 5minute time get fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
	/*
    sprintf(query, "SELECT record_source,package_id,service_id,protocol_id,sum(upstream_volume),"\
					"sum(downstream_volume),sum(stat_cnt),stat_date,max(stat_date) "\
					"FROM %s where ( stat_date = '%s') " \
					"Group by record_source,package_id,service_id,protocol_id,stat_date",\
                STM_STATISTIC_5MINUTE_TR_TBL_NAME, insert_time);
				*/
    sprintf(query, "SELECT * FROM %s where ( stat_date = '%s') ", \
                STM_STATISTIC_5MINUTE_TR_TBL_NAME, insert_time);
	
//	logPrint(trcLogId, FL,"TR Day Update start query : %s\n", query);

	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    result1 = mysql_store_result(conn);
    
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
        sprintf(query, "SELECT * FROM %s where (record_source = '%s' and "
						"package_id = %s and service_id = %s and protocol_id = %s and "
						"stat_date = '%s')", \
                    STM_STATISTIC_DAY_TR_TBL_NAME, row1[0], row1[1], row1[2], \
					row1[3], get_select_time(STMD_DAY));

        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
            if ( stmd_TRUpdate(STMD_DAY, STM_STATISTIC_DAY_TR_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "TR DAY Update error: %s\n", query);
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_TRInsert(STMD_DAY, STM_STATISTIC_DAY_TR_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "TR DAY Insert error: %s\n", query);
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

int stmd_exeUpdateWeekTR()
{
    char    query[4096],insert_time[32];
    int     ret_select;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_WEEK_TR_TBL_NAME, get_delete_time(STMD_1MON_OFFSET));
    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "TR WEEK Statistic 1 Week mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
    }

///////////////////
    /*���� �ð��� ���Ѵ��� 5�� table���� ������ select �ϰ�
    row ��ŭ loop -> upate / insert */
    sprintf (insert_time, "%s", get_insert_time());
    if(strcasecmp(insert_time,"") == 0)
    {
        sprintf(trcBuf, "TR Statistic(Week) 5minute time get fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
	/*
    sprintf(query, "SELECT record_source,package_id,service_id,protocol_id,sum(upstream_volume),"\
					"sum(downstream_volume),sum(stat_cnt),stat_date,max(stat_date) "\
					"FROM %s where ( stat_date = '%s') " \
					"Group by record_source,package_id,service_id,protocol_id,stat_date",\
                STM_STATISTIC_5MINUTE_TR_TBL_NAME, insert_time);
				*/
    sprintf(query, "SELECT * FROM %s where ( stat_date = '%s') ", \
                STM_STATISTIC_5MINUTE_TR_TBL_NAME, insert_time);
	
//	logPrint(trcLogId, FL,"TR Week Update start query : %s\n", query);

	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    result1 = mysql_store_result(conn);
    
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
        sprintf(query, "SELECT * FROM %s where (record_source = '%s' and "
						"package_id = %s and service_id = %s and protocol_id = %s and "
						"stat_date = '%s')", \
                    STM_STATISTIC_WEEK_TR_TBL_NAME, row1[0], row1[1], row1[2], \
					row1[3], get_select_time(STMD_WEEK));

        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
            if ( stmd_TRUpdate(STMD_WEEK, STM_STATISTIC_WEEK_TR_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "TR WEEK Update error: %s\n", query);
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_TRInsert(STMD_WEEK, STM_STATISTIC_WEEK_TR_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "TR WEEK Insert error: %s\n", query);
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

int stmd_exeUpdateMonthTR()
{
    char    query[4096],insert_time[32];
    int     ret_select;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

    // 1�� ���� �Ѵ� ����Ÿ�� �����.
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_MONTH_TR_TBL_NAME, get_delete_time(STMD_1MON_OFFSET));
    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "TR Month Statistic  mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
    }

///////////////////
    /*���� �ð��� ���Ѵ��� 5�� table���� ������ select �ϰ�
    row ��ŭ loop -> upate / insert */
    sprintf(insert_time, "%s", get_insert_time());
    if(strcasecmp(insert_time,"") == 0)
    {
        sprintf(trcBuf, "TR Statistic(Week) 5minute time get fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
	/*
    sprintf(query, "SELECT record_source,package_id,service_id,protocol_id,sum(upstream_volume),"\
					"sum(downstream_volume),sum(stat_cnt),stat_date,max(stat_date) "\
					"FROM %s where ( stat_date = '%s') " \
					"Group by record_source,package_id,service_id,protocol_id,stat_date",\
                STM_STATISTIC_5MINUTE_TR_TBL_NAME, insert_time);
				*/
    sprintf(query, "SELECT * FROM %s where ( stat_date = '%s') ", \
                STM_STATISTIC_5MINUTE_TR_TBL_NAME, insert_time);
	
//	logPrint(trcLogId, FL,"TR Month Update start query : %s\n", query);

	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    result1 = mysql_store_result(conn);
    
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
        sprintf(query, "SELECT * FROM %s where (record_source = '%s' and "
						"package_id = %s and service_id = %s and protocol_id = %s and "
						"stat_date = '%s')", \
                    STM_STATISTIC_MONTH_TR_TBL_NAME, row1[0], row1[1], row1[2], \
					row1[3], get_select_time(STMD_MONTH));

        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
            if ( stmd_TRUpdate(STMD_MONTH, STM_STATISTIC_MONTH_TR_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "TR MONTH Update error: %s\n", query);
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_TRInsert(STMD_MONTH, STM_STATISTIC_MONTH_TR_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "TR MONTH Insert error: %s\n", query);
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
