#include "stmd_proto.h"

extern  char    trcBuf[4096], trcTmp[1024];
extern  int     trcLogFlag ;
extern  MYSQL   sql, *conn;
extern  int     sysCnt;
extern  STM_CommStatisticInfo    StatisticSystemInfo[SYSCONF_MAX_ASSO_SYS_NUM];
extern short delTIME[STMD_PERIOD_TYPE_NUM];

//int   stmd_LoadUpdate(int time_type, char *table_type, char *system_name)
int stmd_LoadUpdate(int time_type, char *table_type, MYSQL_ROW inrow)
{
    char        query[4096];
    char        starttime[32],endtime[32],seltbl[32],sysname[12];
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

	memset (seltbl, 0x00, sizeof(seltbl));
    sprintf(sysname, "%s", inrow[1]);
    
    sprintf(starttime, "%s", get_select_time2(time_type));
    sprintf(endtime, "%s", get_period_end_time2(time_type));    
    switch(time_type)
    {
    case STMD_HOUR:
        strcpy(seltbl, STM_STATISTIC_5MINUTE_LOAD_TBL_NAME);
        break;
    case STMD_DAY:
        strcpy(seltbl, STM_STATISTIC_HOUR_LOAD_TBL_NAME);
        break;
    case STMD_WEEK:
        strcpy(seltbl, STM_STATISTIC_DAY_LOAD_TBL_NAME);
        break;
    case STMD_MONTH:
        strcpy(seltbl, STM_STATISTIC_DAY_LOAD_TBL_NAME);
        break;
    default:
        strcpy(seltbl, STM_STATISTIC_HOUR_LOAD_TBL_NAME);
        break;
    }

    if (time_type == STMD_HOUR || time_type == STMD_DAY){
    	sprintf(query, "SELECT "
                " IFNULL(AVG(avr_cpu0),0), IFNULL(MAX(max_cpu0),0)," 
                " IFNULL(AVG(avr_cpu1),0), IFNULL(MAX(max_cpu1),0)," 
                " IFNULL(AVG(avr_cpu2),0), IFNULL(MAX(max_cpu2),0),"
                " IFNULL(AVG(avr_cpu3),0), IFNULL(MAX(max_cpu3),0)," 
                " IFNULL(AVG(avr_memory),0), IFNULL(MAX(max_memory),0)," 
                " IFNULL(AVG(avr_disk),0), IFNULL(MAX(max_disk),0)," 
                " IFNULL(AVG(avr_msgQ),0), IFNULL(MAX(max_msgQ),0)," 
//                " IFNULL(AVG(avr_sess),0), IFNULL(MAX(max_sess),0)," 
                " IFNULL(SUM(stat_cnt),0) "
                " FROM %s WHERE (system_name = '%s' and stat_date > '%s' and stat_date <= '%s')",
                seltbl, sysname, starttime, endtime);
    }else{
	sprintf(query, "SELECT "
                " IFNULL(AVG(avr_cpu0),0), IFNULL(MAX(max_cpu0),0),"
                " IFNULL(AVG(avr_cpu1),0), IFNULL(MAX(max_cpu1),0),"
                " IFNULL(AVG(avr_cpu2),0), IFNULL(MAX(max_cpu2),0),"
                " IFNULL(AVG(avr_cpu3),0), IFNULL(MAX(max_cpu3),0),"
                " IFNULL(AVG(avr_memory),0), IFNULL(MAX(max_memory),0),"
                " IFNULL(AVG(avr_disk),0), IFNULL(MAX(max_disk),0),"
                " IFNULL(AVG(avr_msgQ),0), IFNULL(MAX(max_msgQ),0),"
//                " IFNULL(AVG(avr_sess),0), IFNULL(MAX(max_sess),0),"
                " IFNULL(SUM(stat_cnt),0) "
                " FROM %s WHERE (system_name = '%s' and stat_date >= '%s' and stat_date < '%s')",
                seltbl, sysname, starttime, endtime);
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
        sprintf(trcBuf, "[%d]%s Load select error\n%s\n", time_type, seltbl, query);
        trclib_writeLogErr (FL, trcBuf);
		mysql_free_result(result1); // MEM if NULL 10.07
        return -1;
    }
    
    // 데이타를 update한다.
    sprintf(query, "UPDATE %s SET " 
                    "avr_cpu0 = %s, max_cpu0 = %s, "
                    "avr_cpu1 = %s, max_cpu1 = %s, "
                    "avr_cpu2 = %s, max_cpu2 = %s, "
                    "avr_cpu3 = %s, max_cpu3 = %s,"
                    "avr_memory = %s, max_memory = %s, "
                    "avr_disk = %s, max_disk = %s, "
                    "avr_msgQ = %s, max_msgQ = %s, "
//                    "avr_sess = %s, max_sess = %s, "
                    "stat_cnt = %s "
                    " WHERE (stat_date = '%s' AND system_name='%s')", 
                    table_type,
                    row1[0],row1[1],
                    row1[2],row1[3],
                    row1[4],row1[5],
                    row1[6],row1[7],
                    row1[8],row1[9],
                    row1[10],row1[11],
                    row1[12],row1[13],
//                    row1[14],row1[15],
                    row1[14],
                    get_select_time(time_type), sysname);
    
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

//int   stmd_LoadInsert(int time_type, char *table_type, char *system_name)
int stmd_LoadInsert(int time_type, char *table_type, MYSQL_ROW inrow)
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
        sprintf(weektype, "%s", get_insert_week());
    }

    sprintf(query, "INSERT INTO %s VALUES ("
                                    "'%s', '%s',"
                                    " '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s',"
                                    " '%s', '%s',"
                                    " '%s', '%s',"
                                    " '%s', '%s',"
//                                    " '%s', '%s',"
                                    " '%s', '%s', '%s' )", 
                                    table_type,
                                    inrow[0], inrow[1],
                                    inrow[2], inrow[3], inrow[4], inrow[5], inrow[6], inrow[7], inrow[8], inrow[9],
                                    inrow[10], inrow[11],
                                    inrow[12], inrow[13],
                                    inrow[14], inrow[15],
//                                    inrow[16], inrow[17],
                                    inrow[16], get_select_time(time_type), weektype);
                                    
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n%s\n", mysql_error(conn), query);
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
////////////////////////

#if 0 //jean
    // 5분 데이타를 select
    sprintf(query, "SELECT * FROM %s where (stat_date = '%s' AND system_name='%s')",
        STM_STATISTIC_5MINUTE_LOAD_TBL_NAME, get_insert_time(), system_name);
    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }

	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    result2 = mysql_store_result(conn);
    while( (row2 = mysql_fetch_row(result2)) != NULL ) {
        strcpy(LoadStatisticInfo[1].comm_info.sysType, row2[0]);
        strcpy(LoadStatisticInfo[1].comm_info.sysName, row2[1]);
        LoadStatisticInfo[1].average_mem    = atof(row2[2]);
        LoadStatisticInfo[1].max_mem        = atof(row2[3]);
        LoadStatisticInfo[1].average_cpu0   = atof(row2[4]);
        LoadStatisticInfo[1].max_cpu0       = atof(row2[5]);
        LoadStatisticInfo[1].average_cpu1   = atof(row2[6]);
        LoadStatisticInfo[1].max_cpu1       = atof(row2[7]);
        LoadStatisticInfo[1].average_cpu2   = atof(row2[8]);
        LoadStatisticInfo[1].max_cpu2       = atof(row2[9]);
        LoadStatisticInfo[1].average_cpu3   = atof(row2[10]);
        LoadStatisticInfo[1].max_cpu3       = atof(row2[11]);
        LoadStatisticInfo[1].stat_cnt       = atof(row2[12]);

        if ( time_type == STMD_WEEK ) {
            sprintf(query, "INSERT INTO %s VALUES('%s', '%s', %d, %d, %d, %d, %d,\
                %d, %d, %d, %d, %d, %d, '%s', '%s' )", table_type, 
                row2[0], row2[1],
                LoadStatisticInfo[1].average_mem, LoadStatisticInfo[1].max_mem,
                LoadStatisticInfo[1].average_cpu0, LoadStatisticInfo[1].max_cpu0,
                LoadStatisticInfo[1].average_cpu1, LoadStatisticInfo[1].max_cpu1,
                LoadStatisticInfo[1].average_cpu2, LoadStatisticInfo[1].max_cpu2,
                LoadStatisticInfo[1].average_cpu3, LoadStatisticInfo[1].max_cpu3,
                LoadStatisticInfo[1].stat_cnt,
                get_select_time(time_type), "Sun");
        } else {
            sprintf(query, "INSERT INTO %s VALUES('%s', '%s', %d, %d, %d, %d, %d,\
                %d, %d, %d, %d, %d, %d, '%s', '%s' )", table_type, 
                row2[0], row2[1],
                LoadStatisticInfo[1].average_mem, LoadStatisticInfo[1].max_mem,
                LoadStatisticInfo[1].average_cpu0, LoadStatisticInfo[1].max_cpu0,
                LoadStatisticInfo[1].average_cpu1, LoadStatisticInfo[1].max_cpu1,
                LoadStatisticInfo[1].average_cpu2, LoadStatisticInfo[1].max_cpu2,
                LoadStatisticInfo[1].average_cpu3, LoadStatisticInfo[1].max_cpu3,
                LoadStatisticInfo[1].stat_cnt,
                get_select_time(time_type), get_insert_week());
        }
        if ( trcLogFlag == TRCLEVEL_SQL ) {
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }

		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);
            mysql_free_result(result2);
            return -1;
        }
        insert_cnt++;
    }
    sprintf(trcBuf, "insert_cnt = %d\n", insert_cnt);
    trclib_writeLog(FL, trcBuf);

    mysql_free_result(result2);
#endif

    return 1;
}

int stmd_exeUpdateHourLoad()
{
    char    query[4096],insert_time[32];
    int     ret_select;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

    // 하루 전의 1시간 데이타를 지운다.
#if 0
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_HOUR_LOAD_TBL_NAME, get_delete_time(STMD_1HOUR_WEEK_OFFSET));
#endif
	/*
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
            STM_STATISTIC_HOUR_LOAD_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*3)); // 3달 
			*/
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
            STM_STATISTIC_HOUR_LOAD_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_HOUR]));
    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "LoadStatistic 1 Hour mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
        // 에러 처리를 어떻게 해야 할지??? hslim_oper
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
    
    sprintf(query, "SELECT * FROM %s where ( stat_date = '%s')",
                STM_STATISTIC_5MINUTE_LOAD_TBL_NAME, insert_time);
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    result1 = mysql_store_result(conn);
    
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
        sprintf(query, "SELECT * FROM %s where (system_name = '%s' and stat_date = '%s')",
                    STM_STATISTIC_HOUR_LOAD_TBL_NAME, row1[1], get_select_time(STMD_HOUR));
        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
            if ( stmd_LoadUpdate(STMD_HOUR, STM_STATISTIC_HOUR_LOAD_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "LOAD HOUR Update error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_LoadInsert(STMD_HOUR, STM_STATISTIC_HOUR_LOAD_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "LOAD HOUR Insert error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else 
        {
            sprintf(trcBuf,"stmd_exeUpdateHourLoad Update/Insert Fail = %d\n",ret_select);
            trclib_writeLogErr (FL,trcBuf);
            mysql_free_result(result1);
            return -1;
        }
    }
    
    mysql_free_result(result1);

///////////////////

#if 0 //jean
    for (i = 0; i < sysCnt; i++) {

        sprintf(query, "SELECT * FROM %s where (stat_date = '%s' AND system_name='%s')",
            STM_STATISTIC_HOUR_LOAD_TBL_NAME, get_select_time(STMD_HOUR), StatisticSystemInfo[i].sysName);
        if ( trcLogFlag == TRCLEVEL_SQL ) {
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }

        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 ) { // update
            if ( stmd_LoadUpdate(STMD_HOUR, STM_STATISTIC_HOUR_LOAD_TBL_NAME, StatisticSystemInfo[i].sysName) < 0 ) {
                sprintf(trcBuf, "LOAD HOUR Update error\n");
                trclib_writeLogErr (FL,trcBuf);
            }
        } else if ( ret_select == 0 ) { // insert 
            if ( stmd_LoadInsert(STMD_HOUR, STM_STATISTIC_HOUR_LOAD_TBL_NAME, StatisticSystemInfo[i].sysName) < 0 ) {
                sprintf(trcBuf, "LOAD HOUR Insert error\n");
                trclib_writeLogErr (FL,trcBuf);
            }
        } else {
            return -1;
        }
    }
#endif

    return 1;
}

int stmd_exeUpdateDayLoad()
{
    char    query[4096],insert_time[32];
    int     ret_select;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

    // 하루 전의 1시간 데이타를 지운다.
#if 0
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_DAY_LOAD_TBL_NAME, get_delete_time(STMD_1DAY_WEEK_OFFSET));
#endif
	/*
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_DAY_LOAD_TBL_NAME, get_delete_time(STMD_1MON_OFFSET)); // 1년
		*/
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_DAY_LOAD_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_DAY]));
    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "LoadStatistic 1 Day mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
		return -1;
        // 에러 처리를 어떻게 해야 할지??? hslim_oper
    }

///////////////////
    /*현재 시간을 구한다음  
    5분 table에서 정보를 select 하고
    row 만큼 loop -> upate / insert */
    sprintf(insert_time, "%s", get_insert_time());
    if(strcasecmp(insert_time,"") == 0)
    {
        sprintf(trcBuf, "Load Statistic(Day) preview 5minute time get fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
    sprintf(query, "SELECT * FROM %s where (stat_date = '%s')",
                STM_STATISTIC_5MINUTE_LOAD_TBL_NAME, insert_time);
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    result1 = mysql_store_result(conn);
    
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
        sprintf(query, "SELECT * FROM %s where (system_name = '%s' and stat_date = '%s')",
                    STM_STATISTIC_DAY_LOAD_TBL_NAME, row1[1], get_select_time(STMD_DAY));
        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
            if ( stmd_LoadUpdate(STMD_DAY, STM_STATISTIC_DAY_LOAD_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "LOAD DAY Update error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_LoadInsert(STMD_DAY, STM_STATISTIC_DAY_LOAD_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "LOAD DAY Insert error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else 
        {
            sprintf(trcBuf,"stmd_exeUpdateDayLoad Update/Insert Fail = %d\n",ret_select);
            trclib_writeLogErr (FL,trcBuf);
            mysql_free_result(result1);
            return -1;
        }
    }
    
    mysql_free_result(result1);

///////////////////

    return 1;
}


int stmd_exeUpdateWeekLoad()
{
    char    query[4096],insert_time[32];
    int     ret_select;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

    // 한달 전의 한주 데이타를 지운다.
#if 0
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_WEEK_LOAD_TBL_NAME, get_delete_time(STMD_1WEEK_OFFSET));
#endif
	/*
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_WEEK_LOAD_TBL_NAME, get_delete_time(STMD_1MON_OFFSET));
		*/
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_WEEK_LOAD_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_WEEK]));
    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "LoadStatistic 1 Week mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
		return -1;
        // 에러 처리를 어떻게 해야 할지??? hslim_oper
    }

///////////////////
    /*현재 시간보다 5분전 시간을 구한다음
    5분 table에서 정보를 select 하고
    row 만큼 loop -> upate / insert */
    sprintf (insert_time, "%s",  get_insert_time());
    if(strcasecmp(insert_time,"") == 0)
    {
        sprintf(trcBuf, "Load Statistic(Week) preview 5minute time get fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
    sprintf(query, "SELECT * FROM %s where (stat_date = '%s')",
                STM_STATISTIC_5MINUTE_LOAD_TBL_NAME, insert_time);
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    result1 = mysql_store_result(conn);
    
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
        sprintf(query, "SELECT * FROM %s where (system_name = '%s' and stat_date = '%s')",
                    STM_STATISTIC_WEEK_LOAD_TBL_NAME, row1[1], get_select_time(STMD_WEEK));
        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
            if ( stmd_LoadUpdate(STMD_WEEK, STM_STATISTIC_WEEK_LOAD_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "LOAD WEEK Update error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_LoadInsert(STMD_WEEK, STM_STATISTIC_WEEK_LOAD_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "LOAD WEEK Insert error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else 
        {
            sprintf(trcBuf,"stmd_exeUpdateWeekLoad Update/Insert Fail = %d\n",ret_select);
            trclib_writeLogErr (FL,trcBuf);
            mysql_free_result(result1);
            return -1;
        }
    }
    
    mysql_free_result(result1);

///////////////////

    return 1;
}



int stmd_exeUpdateMonthLoad()
{
    char    query[4096],insert_time[32];
    int     ret_select;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

    // 1년 전의 한달 데이타를 지운다.
	/*
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_MONTH_LOAD_TBL_NAME, get_delete_time(STMD_1MON_OFFSET));
		*/
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_MONTH_LOAD_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_MONTH]));
    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "LoadStatistic 1 Month mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
		return -1;
        // 에러 처리를 어떻게 해야 할지??? hslim_oper
    }

///////////////////
    /*현재 시간보다 5분전 시간을 구한다음
    5분 table에서 정보를 select 하고
    row 만큼 loop -> upate / insert */
    sprintf(insert_time, "%s", get_insert_time());
    if(strcasecmp(insert_time,"") == 0)
    {
        sprintf(trcBuf, "Load Statistic(Month) preview 5minute time get fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
    sprintf(query, "SELECT * FROM %s where (stat_date = '%s')",
                STM_STATISTIC_5MINUTE_LOAD_TBL_NAME, insert_time);
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    result1 = mysql_store_result(conn);
    
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
        sprintf(query, "SELECT * FROM %s where (system_name = '%s' and stat_date = '%s')",
                    STM_STATISTIC_MONTH_LOAD_TBL_NAME, row1[1], get_select_time(STMD_MONTH));
        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
            if ( stmd_LoadUpdate(STMD_MONTH, STM_STATISTIC_MONTH_LOAD_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "LOAD MONTH Update error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_LoadInsert(STMD_MONTH, STM_STATISTIC_MONTH_LOAD_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "LOAD MONTH Insert error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else 
        {
            sprintf(trcBuf,"stmd_exeUpdateMonthLoad Update/Insert Fail = %d\n",ret_select);
            trclib_writeLogErr (FL,trcBuf);
            mysql_free_result(result1);
            return -1;
        }
    }
    
    mysql_free_result(result1);

///////////////////

    return 1;
}
