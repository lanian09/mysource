#include "stmd_proto.h"

extern  char    trcBuf[4096], trcTmp[1024];
extern  int     trcLogFlag ;
extern  MYSQL   sql, *conn;
extern  int     sysCnt;
extern  STM_CommStatisticInfo    StatisticSystemInfo[SYSCONF_MAX_ASSO_SYS_NUM];
extern int trcLogId;

extern RuleSetList        g_stSCERule[MAX_SCE_NUM];

/* 2009.04.22 jjinri */
int stmd_PURUpdate(int time_type, char *table_type, MYSQL_ROW inrow)
{
    char        query[4096] = {0,};
    char        starttime[32],endtime[32],seltbl[32]; // by jjinri 2009.04.20 ,sysname[12];
    MYSQL_RES   *result1, *result2;
    MYSQL_ROW   row1, row2;
    int         update_cnt = 0;
	char		record_source[32]={0,}, pkg_usg_cnt_id[10]={0,}, glbl_usg_cnt_id[10]={0,}; 

	memset (seltbl, 0x00, sizeof(seltbl));
	sprintf(record_source, "%s", inrow[0]);
	sprintf(pkg_usg_cnt_id, "%s", inrow[1]);
	sprintf(glbl_usg_cnt_id, "%s", inrow[2]);
    
    sprintf(starttime, "%s", get_select_time2(time_type));
    sprintf(endtime, "%s", get_period_end_time2(time_type));    
    switch(time_type)
    {
    case STMD_HOUR:
        strcpy(seltbl, STM_STATISTIC_5MINUTE_PUR_TBL_NAME);
        break;
    case STMD_DAY:
        strcpy(seltbl, STM_STATISTIC_HOUR_PUR_TBL_NAME);
        break;
    case STMD_WEEK:
        strcpy(seltbl, STM_STATISTIC_DAY_PUR_TBL_NAME);
        break;
    case STMD_MONTH:
        strcpy(seltbl, STM_STATISTIC_DAY_PUR_TBL_NAME);
        break;
    default:
        strcpy(seltbl, STM_STATISTIC_HOUR_PUR_TBL_NAME);
        break;
    }

    if (time_type == STMD_HOUR || time_type == STMD_DAY){
		sprintf(query, "SELECT "
                " IFNULL(SUM(upstream_volume),0), "
                " IFNULL(SUM(downstream_volume),0), "
                " IFNULL(SUM(sessions),0), "
                " IFNULL(SUM(seconds),0), "
                " IFNULL(SUM(concurrent_sessions),0), "
                " IFNULL(SUM(active_subscribers),0), "
                " IFNULL(SUM(total_active_subscribers),0), "
                " IFNULL(SUM(stat_cnt),0) "
                " FROM %s "
				" WHERE (record_source = '%s' and "
					"pkg_usg_cnt_id = %s and glbl_usg_cnt_id = %s and "
					"stat_date > '%s' and stat_date <= '%s')"
				" Group by record_source,pkg_usg_cnt_id,glbl_usg_cnt_id",
                seltbl, record_source, pkg_usg_cnt_id, glbl_usg_cnt_id, 
				starttime, endtime);
    }else{
		sprintf(query, "SELECT "
                " IFNULL(SUM(upstream_volume),0), "
                " IFNULL(SUM(downstream_volume),0), "
                " IFNULL(SUM(sessions),0), "
                " IFNULL(SUM(seconds),0), "
                " IFNULL(SUM(concurrent_sessions),0), "
                " IFNULL(SUM(active_subscribers),0), "
                " IFNULL(SUM(total_active_subscribers),0), "
                " IFNULL(SUM(stat_cnt),0) "
                " FROM %s "
				" WHERE (record_source = '%s' and "
					"pkg_usg_cnt_id = %s and glbl_usg_cnt_id = %s and "
					"stat_date >= '%s' and stat_date < '%s')"
				" Group by record_source,pkg_usg_cnt_id,glbl_usg_cnt_id",
                seltbl, record_source, pkg_usg_cnt_id, glbl_usg_cnt_id, 
				starttime, endtime);
    } 
//	logPrint(trcLogId,FL,"PUR Update SELECT : query=%s\n", query);

	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; query=%s; err=%s\n", query, mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
    result1 = mysql_store_result(conn);
    row1 = mysql_fetch_row(result1); 

    if( row1 == NULL )
    {
        sprintf(trcBuf, "[%d]%s PUR 5MIN select error: %s\n", time_type, seltbl, query);
        trclib_writeLogErr (FL, trcBuf);
		mysql_free_result(result1);
        return -1;
    }
    
	memset(query, 0x00, sizeof(query));
    // 데이타를 update한다.
    sprintf(query, "UPDATE %s SET " 
                    	"upstream_volume = %s,"
                    	"downstream_volume = %s,"
                    	"sessions = %s,"
                    	"seconds = %s,"
                    	"concurrent_sessions = %s,"
                    	"active_subscribers = %s,"
                    	"total_active_subscribers = %s,"
                    	"stat_cnt = %s "
				" WHERE (stat_date = '%s' and record_source = '%s' and "
						"pkg_usg_cnt_id = %s and glbl_usg_cnt_id = %s) ",\
                    table_type, row1[0],row1[1], row1[2],row1[3],row1[4],row1[5],row1[6],row1[7],
                    get_select_time(time_type), record_source, pkg_usg_cnt_id, glbl_usg_cnt_id);
    
//	logPrint(trcLogId,FL,"PUR Update UPDATE : query=%s\n", query);

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
int stmd_PURInsert(int time_type, char *table_type, MYSQL_ROW inrow)
{
    char        query[4096] = {0,};
    int         insert_cnt = 0;
    char    weektype[32] = {0,};


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
					 "'%s','%s','%s','%s','%s',"
					 "'%s','%s','%s')", table_type, \
					inrow[0],inrow[1],inrow[2],inrow[3],inrow[4], \
					inrow[5],inrow[6],inrow[7],inrow[8],inrow[9], \
					inrow[10],get_select_time(time_type),weektype);
                                    
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; query : %s;  err=%s\n", query, mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
//	logPrint(trcLogId,FL, "PUR Update Insert : query=%s\n", query);

    return 1;
}

int stmd_exeUpdateHourPUR(t_start)
{
    char    query[4096],insert_time[32];
    int     ret_select, i;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

	memset(query, 0x00, sizeof(query));
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
            STM_STATISTIC_HOUR_PUR_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*3)); // 3달 
    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "PUR Statistic(Hour) mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
    }
//	logPrint(trcLogId,FL, "PUR Hour Delete : query=%s\n", query);


///////////////////
    /*현재 시간을 구한다음 5분 table에서 정보를 select 하고
    row 만큼 loop -> upate / insert */
    strcpy(insert_time, get_insert_time());
    if(strcasecmp(insert_time,"") == 0)
    {
        sprintf(trcBuf, "PUR Statistic(Hour) 5minute time get fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
	memset(query, 0x00, sizeof(query));

	/*
    sprintf(query, "SELECT record_source,pkg_usg_cnt_id,glbl_usg_cnt_id,sum(upstream_volume), "\
					"sum(downstream_volume), sum(sessions), sum(seconds), sum(concurrent_sessions), "\
					"sum(active_subscribers), sum(total_active_subscribers), sum(stat_cnt), "\
					"stat_date, max(stat_week) "\
					"FROM %s "\
					"where ( stat_date = '%s') "\
					"Group by record_source,pkg_usg_cnt_id,glbl_usg_cnt_id",\
                STM_STATISTIC_5MINUTE_PUR_TBL_NAME, insert_time);
				*/
    sprintf(query, "SELECT * FROM %s where ( stat_date = '%s') ", \
                STM_STATISTIC_5MINUTE_PUR_TBL_NAME, insert_time);
	
//	logPrint(trcLogId, FL,"PUR Hour Update start query : %s\n", query);

	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    result1 = mysql_store_result(conn);
    
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
		memset(query, 0x00, sizeof(query));
        sprintf(query, "SELECT * FROM %s where (record_source = '%s' and "
						" pkg_usg_cnt_id = %s and glbl_usg_cnt_id = %s and "
						" stat_date = '%s')", \
                    STM_STATISTIC_HOUR_PUR_TBL_NAME, row1[0], row1[1], row1[2], \
					get_select_time(STMD_HOUR));

        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
            if ( stmd_PURUpdate(STMD_HOUR, STM_STATISTIC_HOUR_PUR_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "PUR HOUR Update error: %s\n", query);
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_PURInsert(STMD_HOUR, STM_STATISTIC_HOUR_PUR_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "PUR HOUR Insert error: %s\n", query);
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else 
        {
            sprintf(trcBuf,"stmd_exeUpdateHourPUR Update/Insert Fail = %d:query=:%s\n",ret_select,query);
            trclib_writeLogErr (FL,trcBuf);
            mysql_free_result(result1);
            return -1;
        }
    }
    
    mysql_free_result(result1);

    return 1;
}

int stmd_exeUpdateDayPUR()
{
    char    query[4096] = {0,},insert_time[32] = {0,};
    int     ret_select, i;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

	memset(query, 0x00, sizeof(query));
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_DAY_PUR_TBL_NAME, get_delete_time(STMD_1MON_OFFSET)); // 1년
    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "PUR Day Statistic mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
    }
//	logPrint(trcLogId, FL,"PUR Day Delete start query : %s\n", query);


///////////////////
    /*현재 시간을 구한다음 5분 table에서 정보를 select 하고
    row 만큼 loop -> upate / insert */
    strcpy(insert_time, get_insert_time());
    if(strcasecmp(insert_time,"") == 0)
    {
        sprintf(trcBuf, "PUR Statistic(Day) 5minute time get fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
	memset(query, 0x00, sizeof(query));
	/*
    sprintf(query, "SELECT record_source,pkg_usg_cnt_id,glbl_usg_cnt_id,sum(upstream_volume), sum(downstream_volume), sum(sessions), sum(seconds), sum(concurrent_sessions), sum(active_subscribers), sum(total_active_subscribers), sum(stat_cnt), stat_date, max(stat_week) FROM %s where ( stat_date = '%s') Group by record_source,pkg_usg_cnt_id,glbl_usg_cnt_id",\
			STM_STATISTIC_5MINUTE_PUR_TBL_NAME, insert_time);
			*/
    sprintf(query, "SELECT * FROM %s where ( stat_date = '%s') ", \
                STM_STATISTIC_5MINUTE_PUR_TBL_NAME, insert_time);
	
//	logPrint(trcLogId, FL,"PUR Day Update start query : %s\n", query);

	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    result1 = mysql_store_result(conn);
    
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
		memset(query, 0x00, sizeof(query));
        sprintf(query, "SELECT * FROM %s where (record_source = '%s' and "
						" pkg_usg_cnt_id = %s and glbl_usg_cnt_id = %s and "
						" stat_date = '%s')", \
                    STM_STATISTIC_DAY_PUR_TBL_NAME, row1[0], row1[1], row1[2], \
					get_select_time(STMD_DAY));

        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
            if ( stmd_PURUpdate(STMD_DAY, STM_STATISTIC_DAY_PUR_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "PUR DAY Update error: %s\n", query);
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_PURInsert(STMD_DAY, STM_STATISTIC_DAY_PUR_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "PUR DAY Insert error: %s\n", query);
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

int stmd_exeUpdateWeekPUR()
{
    char    query[4096],insert_time[32];
    int     ret_select, i;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_WEEK_PUR_TBL_NAME, get_delete_time(STMD_1MON_OFFSET));
    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "PUR WEEK Statistic 1 Week mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
    }
//	logPrint(trcLogId, FL,"PUR Week Delete start query : %s\n", query);


///////////////////
    /*현재 시간을 구한다음 5분 table에서 정보를 select 하고
    row 만큼 loop -> upate / insert */
    strcpy(insert_time, get_insert_time());
    if(strcasecmp(insert_time,"") == 0)
    {
        sprintf(trcBuf, "PUR Statistic(Week) 5minute time get fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
	memset(query, 0x00, sizeof(query));
	/*
    sprintf(query, "SELECT record_source,pkg_usg_cnt_id,glbl_usg_cnt_id,sum(upstream_volume), sum(downstream_volume), sum(sessions), sum(seconds), sum(concurrent_sessions), sum(active_subscribers), sum(total_active_subscribers), sum(stat_cnt), stat_date, max(stat_week) FROM %s where ( stat_date = '%s') Group by record_source,pkg_usg_cnt_id,glbl_usg_cnt_id", STM_STATISTIC_5MINUTE_PUR_TBL_NAME, insert_time);
	*/
    sprintf(query, "SELECT * FROM %s where ( stat_date = '%s') ", \
                STM_STATISTIC_5MINUTE_PUR_TBL_NAME, insert_time);
	
//	logPrint(trcLogId, FL,"PUR Week Update start query : %s\n", query);

	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    result1 = mysql_store_result(conn);
    
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
		memset(query, 0x00, sizeof(query));
        sprintf(query, "SELECT * FROM %s where (record_source = '%s' and "
						" pkg_usg_cnt_id = %s and glbl_usg_cnt_id = %s and "
						" stat_date = '%s')", \
                    STM_STATISTIC_WEEK_PUR_TBL_NAME, row1[0], row1[1], row1[2], \
					get_select_time(STMD_WEEK));

        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
            if ( stmd_PURUpdate(STMD_WEEK, STM_STATISTIC_WEEK_PUR_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "PUR WEEK Update error: %s\n", query);
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_PURInsert(STMD_WEEK, STM_STATISTIC_WEEK_PUR_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "PUR WEEK Insert error: %s\n", query);
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

int stmd_exeUpdateMonthPUR()
{
    char    query[4096] = {0,},insert_time[32] ={0,};
    int     ret_select, i;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

    // 1년 전의 한달 데이타를 지운다.
	memset(query,0x00,sizeof(query));
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_MONTH_PUR_TBL_NAME, get_delete_time(STMD_1MON_OFFSET));
    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "PUR Month Statistic  mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
    }
//	logPrint(trcLogId, FL,"PUR Month Delete start query : %s\n", query);


///////////////////
    /*현재 시간을 구한다음 5분 table에서 정보를 select 하고
    row 만큼 loop -> upate / insert */
    strcpy(insert_time, get_insert_time());
    if(strcasecmp(insert_time,"") == 0)
    {
        sprintf(trcBuf, "PUR Statistic(Month) 5minute time get fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
	memset(query, 0x00, sizeof(query));
	/*
    sprintf(query, "SELECT record_source,pkg_usg_cnt_id,glbl_usg_cnt_id,sum(upstream_volume), sum(downstream_volume), sum(sessions), sum(seconds), sum(concurrent_sessions), sum(active_subscribers), sum(total_active_subscribers), sum(stat_cnt), stat_date, max(stat_week) FROM %s where ( stat_date = '%s') Group by record_source,pkg_usg_cnt_id,glbl_usg_cnt_id", \
			STM_STATISTIC_5MINUTE_PUR_TBL_NAME, insert_time);
			*/
    sprintf(query, "SELECT * FROM %s where ( stat_date = '%s') ", \
                STM_STATISTIC_5MINUTE_PUR_TBL_NAME, insert_time);
	
//	logPrint(trcLogId, FL,"PUR Month Update start query : %s\n", query);

	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    result1 = mysql_store_result(conn);
    
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
		memset(query, 0x00, sizeof(query));
        sprintf(query, "SELECT * FROM %s where (record_source = '%s' and "
						" pkg_usg_cnt_id = %s and glbl_usg_cnt_id = %s and "
						" stat_date = '%s')", \
                    STM_STATISTIC_MONTH_PUR_TBL_NAME, row1[0], row1[1], row1[2], \
					get_select_time(STMD_MONTH));

        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
            if ( stmd_PURUpdate(STMD_MONTH, STM_STATISTIC_MONTH_PUR_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "PUR MONTH Update error: %s\n", query);
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_PURInsert(STMD_MONTH, STM_STATISTIC_MONTH_PUR_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "PUR MONTH Insert error: %s\n", query);
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
