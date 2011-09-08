#include "stmd_proto.h"

extern  char    trcBuf[4096], trcTmp[1024];
extern  int     trcLogFlag ;
extern  MYSQL   sql, *conn;
extern  int     sysCnt;
extern  STM_CommStatisticInfo    StatisticSystemInfo[SYSCONF_MAX_ASSO_SYS_NUM];
extern int trcLogId;

/* 2009.04.22 jjinri */
int stmd_MALURUpdate(int time_type, char *table_type, MYSQL_ROW inrow)
{
    char        query[4096];
    char        starttime[32],endtime[32],seltbl[32]; // by jjinri 2009.04.20 ,sysname[12];
    MYSQL_RES   *result1, *result2;
    MYSQL_ROW   row1, row2;
    int         update_cnt = 0;
	char		record_source[32]={0,}, attack_id[10]={0,}, subscriber_id[64]={0,}; 
	char		attack_ip[32]={0,}, other_ip[32]={0,}, port_number[10]={0,}; 
	char		attack_type[10]={0,}, attack_name[10] = {0,}, side[4]={0,}, ip_protocol[10]={0,}; 

	memset (seltbl, 0x00, sizeof(seltbl));
	sprintf(record_source, "%s", inrow[0]);
	sprintf(attack_id, "%s", inrow[1]);
	sprintf(subscriber_id, "%s", inrow[2]);
	sprintf(attack_ip, "%s", inrow[3]);
	sprintf(other_ip, "%s", inrow[4]);
	sprintf(port_number, "%s", inrow[5]);
	sprintf(attack_type, "%s", inrow[6]);
	sprintf(attack_name, "%s", inrow[7]);
	sprintf(side, "%s", inrow[8]);
	sprintf(ip_protocol, "%s", inrow[9]);
    
    sprintf(starttime, "%s", get_select_time2(time_type));
    sprintf(endtime, "%s", get_period_end_time2(time_type));    
    switch(time_type)
    {
    case STMD_HOUR:
        strcpy(seltbl, STM_STATISTIC_5MINUTE_MALUR_TBL_NAME);
        break;
    case STMD_DAY:
        strcpy(seltbl, STM_STATISTIC_HOUR_MALUR_TBL_NAME);
        break;
    case STMD_WEEK:
        strcpy(seltbl, STM_STATISTIC_DAY_MALUR_TBL_NAME);
        break;
    case STMD_MONTH:
        strcpy(seltbl, STM_STATISTIC_DAY_MALUR_TBL_NAME);
        break;
    default:
        strcpy(seltbl, STM_STATISTIC_HOUR_MALUR_TBL_NAME);
        break;
    }

    if (time_type == STMD_HOUR || time_type == STMD_DAY){
		sprintf(query, "SELECT "
                " IFNULL(SUM(attacks),0), "
                " IFNULL(SUM(malicious_sessions),0), "
                " IFNULL(SUM(stat_cnt),0) "
                " FROM %s "
				" WHERE (record_source = '%s' and "
					"attack_id = %s and subscriber_id = '%s' and "
					"attack_ip = %s and other_ip = %s and "
					"port_number = %s and attack_type = %s and "
					"attack_name = '%s' and side = %s and ip_protocol = %s and "
					"stat_date > '%s' and stat_date <= '%s')",
				" Group by record_source,attack_id,subscriber_id,attack_ip,other_ip,port_number,"
				" attack_type,attack_name,side,ip_protocol",
                seltbl, record_source, attack_id, subscriber_id, \
				attack_ip, other_ip, port_number, attack_type, \
				attack_name, side, ip_protocol,\
				starttime, endtime);
    }else{
		sprintf(query, "SELECT "
                " IFNULL(SUM(attacks),0), "
                " IFNULL(SUM(malicious_sessions),0), "
                " IFNULL(SUM(stat_cnt),0) "
                " FROM %s "
				" WHERE (record_source = '%s' and "
					"attack_id = %s and subscriber_id = '%s' and "
					"attack_ip = %s and other_ip = %s and "
					"port_number = %s and attack_type = %s and "
					"attack_name = '%s' and side = %s and ip_protocol = %s and "
					"stat_date >= '%s' and stat_date < '%s')"
				" Group by record_source,attack_id,subscriber_id,attack_ip,other_ip,port_number,"
				" attack_type,attack_name,side,ip_protocol",
                seltbl, record_source, attack_id, subscriber_id, \
				attack_ip, other_ip, port_number, attack_type, \
				attack_name, side, ip_protocol,\
				starttime, endtime);
    } 
//	logPrint(trcLogId,FL,"MALUR Update : query=%s\n", query);

	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; query=%s; err=%s\n", query, mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
    result1 = mysql_store_result(conn);
    row1 = mysql_fetch_row(result1); 

    if( row1 == NULL )
    {
        sprintf(trcBuf, "[%d]%s MALUR select error: %s\n", time_type, seltbl, query);
        trclib_writeLogErr (FL, trcBuf);
		mysql_free_result(result1);
        return -1;
    }
    
    // 데이타를 update한다.
    sprintf(query, "UPDATE %s SET " 
                    	"attacks = %s,"
                    	"malicious_sessions = %s,"
                    	"stat_cnt = %s "
				" WHERE (stat_date = '%s' and record_source = '%s' and "
						"attack_id = %s and subscriber_id = '%s' and "
						"attack_ip = %s and other_ip = %s and port_number = %s and "
						"attack_type = %s and attack_name = '%s' and side = %s and ip_protocol = %s )",\
                    table_type, row1[0],row1[1], row1[2],\
                    get_select_time(time_type), record_source, attack_id, subscriber_id, \
					attack_ip, other_ip, port_number, attack_type, attack_name, side, ip_protocol);
    
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

int stmd_MALURInsert(int time_type, char *table_type, MYSQL_ROW inrow)
{
    char        query[4096];
    int         insert_cnt = 0;
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

    sprintf(query, "INSERT INTO %s VALUES "
	 				"('%s','%s','%s','%s','%s',"
					 "'%s','%s','%s','%s','%s',"
					 "'%s','%s','%s','%s','%s')", table_type, \
					inrow[0],inrow[1],inrow[2],inrow[3],inrow[4], \
					inrow[5],inrow[6],inrow[7],inrow[8],inrow[9], \
					inrow[10],inrow[11],inrow[12],get_select_time(time_type),weektype);
                                    
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; query : %s;  err=%s\n", query, mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
//	logPrint(trcLogId,FL, "MALUR Update Insert : query=%s\n", query);

    return 1;
}

int stmd_exeUpdateHourMALUR(t_start)
{
    char    query[4096],insert_time[32];
    int     ret_select, i;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
            STM_STATISTIC_HOUR_MALUR_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*3)); // 3달 
    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "MALUR Statistic(Hour) mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
    }

///////////////////
    /*현재 시간을 구한다음 5분 table에서 정보를 select 하고
    row 만큼 loop -> upate / insert */
    sprintf(insert_time, "%s", get_insert_time());
    if(strcasecmp(insert_time,"") == 0)
    {
        sprintf(trcBuf, "MALUR Statistic(Hour) 5minute time get fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
	/*
    sprintf(query, "SELECT record_source, attack_id, subscriber_id, attack_ip, other_ip, "\
						"port_number, attack_type, attack_name, side, ip_protocol, sum(attacks), "\
						"sum(malicious_sessions), sum(stat_cnt), stat_date, max(stat_week) "\
					"FROM %s where ( stat_date = '%s') "\
					"Group by record_source, attack_id, subscriber_id, attack_ip,other_ip,port_number,"\
					"attack_type,attack_name,side,ip_protocol",\
                STM_STATISTIC_5MINUTE_MALUR_TBL_NAME, insert_time);
				*/
    sprintf(query, "SELECT  * FROM %s where ( stat_date = '%s') ", \
                STM_STATISTIC_5MINUTE_MALUR_TBL_NAME, insert_time);
	
//	logPrint(trcLogId, FL,"MALUR Hour Update start query : %s\n", query);

	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    result1 = mysql_store_result(conn);
    
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
        sprintf(query, "SELECT * FROM %s where (record_source = '%s' and "
						"attack_id = %s and subscriber_id = '%s' and "
						"attack_ip = %s and other_ip = %s and "
						"port_number = %s and attack_type = %s and "
						"attack_name = '%s' and side = %s and ip_protocol = %s and "
						"stat_date = '%s')", \
                    STM_STATISTIC_HOUR_MALUR_TBL_NAME, row1[0], row1[1], row1[2], \
					row1[3],row1[4],row1[5],row1[6],row1[7],row1[8],row1[9], \
					get_select_time(STMD_HOUR));

        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
            if ( stmd_MALURUpdate(STMD_HOUR, STM_STATISTIC_HOUR_MALUR_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "MALUR HOUR Update error: %s\n", query);
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_MALURInsert(STMD_HOUR, STM_STATISTIC_HOUR_MALUR_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "MALUR HOUR Insert error: %s\n", query);
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else 
        {
            sprintf(trcBuf,"stmd_exeUpdateHourMALUR Update/Insert Fail = %d:query=:%s\n",ret_select,query);
            trclib_writeLogErr (FL,trcBuf);
            mysql_free_result(result1);
            return -1;
        }
    }
    
    mysql_free_result(result1);

    return 1;
}

int stmd_exeUpdateDayMALUR()
{
    char    query[4096],insert_time[32];
    int     ret_select, i;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_DAY_MALUR_TBL_NAME, get_delete_time(STMD_1MON_OFFSET)); // 1년
    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "MALUR Day Statistic mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
    }

///////////////////
    /*현재 시간을 구한다음 5분 table에서 정보를 select 하고
    row 만큼 loop -> upate / insert */
    sprintf (insert_time, "%s", get_insert_time());
    if(strcasecmp(insert_time,"") == 0)
    {
        sprintf(trcBuf, "MALUR Statistic(Day) 5minute time get fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
	/*
    sprintf(query, "SELECT record_source, attack_id, subscriber_id, attack_ip, other_ip, "\
						"port_number, attack_type, attack_name, side, ip_protocol, sum(attacks), "\
						"sum(malicious_sessions), sum(stat_cnt), stat_date, max(stat_week) "\
					"FROM %s where ( stat_date = '%s') "\
					"Group by record_source, attack_id, subscriber_id, attack_ip,other_ip,port_number,"\
					"attack_type,attack_name,side,ip_protocol",\
                STM_STATISTIC_5MINUTE_MALUR_TBL_NAME, insert_time);
				*/
    sprintf(query, "SELECT  * FROM %s where ( stat_date = '%s') ", \
                STM_STATISTIC_5MINUTE_MALUR_TBL_NAME, insert_time);
	
//	logPrint(trcLogId, FL,"MALUR Day Update start query : %s\n", query);

	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    result1 = mysql_store_result(conn);
    
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
        sprintf(query, "SELECT * FROM %s where (record_source = '%s' and "
						"attack_id = %s and subscriber_id = '%s' and "
						"attack_ip = %s and other_ip = %s and "
						"port_number = %s and attack_type = %s and "
						"side = %s and ip_protocol = %s and "
						"stat_date = '%s')", \
                    STM_STATISTIC_DAY_MALUR_TBL_NAME, row1[0], row1[1], row1[2], \
					row1[3],row1[4],row1[5],row1[6],row1[7],row1[8],\
					get_select_time(STMD_DAY));

        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
            if ( stmd_MALURUpdate(STMD_DAY, STM_STATISTIC_DAY_MALUR_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "MALUR DAY Update error: %s\n", query);
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_MALURInsert(STMD_DAY, STM_STATISTIC_DAY_MALUR_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "MALUR DAY Insert error: %s\n", query);
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

int stmd_exeUpdateWeekMALUR()
{
    char    query[4096],insert_time[32];
    int     ret_select, i;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_WEEK_MALUR_TBL_NAME, get_delete_time(STMD_1MON_OFFSET));
    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "MALUR WEEK Statistic 1 Week mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
    }

///////////////////
    /*현재 시간을 구한다음 5분 table에서 정보를 select 하고
    row 만큼 loop -> upate / insert */
    sprintf (insert_time, "%s", get_insert_time());
    if(strcasecmp(insert_time,"") == 0)
    {
        sprintf(trcBuf, "MALUR Statistic(Week) 5minute time get fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
	/*
    sprintf(query, "SELECT record_source, attack_id, subscriber_id, attack_ip, other_ip, "\
						"port_number, attack_type, attack_name, side, ip_protocol, sum(attacks), "\
						"sum(malicious_sessions), sum(stat_cnt), stat_date, max(stat_week) "\
					"FROM %s where ( stat_date = '%s') "\
					"Group by record_source, attack_id, subscriber_id, attack_ip,other_ip,port_number,"\
					"attack_type,attack_name,side,ip_protocol",\
                STM_STATISTIC_5MINUTE_MALUR_TBL_NAME, insert_time);
				*/
    sprintf(query, "SELECT  * FROM %s where ( stat_date = '%s') ", \
                STM_STATISTIC_5MINUTE_MALUR_TBL_NAME, insert_time);
	
//	logPrint(trcLogId, FL,"MALUR Week Update start query : %s\n", query);

	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    result1 = mysql_store_result(conn);
    
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
        sprintf(query, "SELECT * FROM %s where (record_source = '%s' and "
						"attack_id = %s and subscriber_id = '%s' and "
						"attack_ip = %s and other_ip = %s and "
						"port_number = %s and attack_type = %s and "
						"side = %s and ip_protocol = %s and "
						"stat_date = '%s')", \
                    STM_STATISTIC_WEEK_MALUR_TBL_NAME, row1[0], row1[1], row1[2], \
					row1[3],row1[4],row1[5],row1[6],row1[7],row1[8],\
					get_select_time(STMD_WEEK));

        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
            if ( stmd_MALURUpdate(STMD_WEEK, STM_STATISTIC_WEEK_MALUR_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "MALUR WEEK Update error: %s\n", query);
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_MALURInsert(STMD_WEEK, STM_STATISTIC_WEEK_MALUR_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "MALUR WEEK Insert error: %s\n", query);
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

int stmd_exeUpdateMonthMALUR()
{
    char    query[4096],insert_time[32];
    int     ret_select, i;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

    // 1년 전의 한달 데이타를 지운다.
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_MONTH_MALUR_TBL_NAME, get_delete_time(STMD_1MON_OFFSET));
    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "MALUR Month Statistic  mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
    }

///////////////////
    /*현재 시간을 구한다음 5분 table에서 정보를 select 하고
    row 만큼 loop -> upate / insert */
    sprintf (insert_time, "%s", get_insert_time());
    if(strcasecmp(insert_time,"") == 0)
    {
        sprintf(trcBuf, "MALUR Statistic(Month) 5minute time get fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
	/*
    sprintf(query, "SELECT record_source, attack_id, subscriber_id, attack_ip, other_ip, "\
						"port_number, attack_type, attack_name, side, ip_protocol, sum(attacks), "\
						"sum(malicious_sessions), sum(stat_cnt), stat_date, max(stat_week) "\
					"FROM %s where ( stat_date = '%s') "\
					"Group by record_source, attack_id, subscriber_id, attack_ip,other_ip,port_number,"\
					"attack_type,attack_name,side,ip_protocol",\
                STM_STATISTIC_5MINUTE_MALUR_TBL_NAME, insert_time);
				*/
    sprintf(query, "SELECT  * FROM %s where ( stat_date = '%s') ", \
                STM_STATISTIC_5MINUTE_MALUR_TBL_NAME, insert_time);
	
//	logPrint(trcLogId, FL,"MALUR Month Update start query : %s\n", query);

	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    result1 = mysql_store_result(conn);
    
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
        sprintf(query, "SELECT * FROM %s where (record_source = '%s' and "
						"attack_id = %s and subscriber_id = '%s' and "
						"attack_ip = %s and other_ip = %s and "
						"port_number = %s and attack_type = %s and "
						"side = %s and ip_protocol = %s and "
						"stat_date = '%s')", \
                    STM_STATISTIC_MONTH_MALUR_TBL_NAME, row1[0], row1[1], row1[2], \
					row1[3],row1[4],row1[5],row1[6],row1[7],row1[8],\
					get_select_time(STMD_MONTH));

        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
            if ( stmd_MALURUpdate(STMD_MONTH, STM_STATISTIC_MONTH_MALUR_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "MALUR MONTH Update error: %s\n", query);
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_MALURInsert(STMD_MONTH, STM_STATISTIC_MONTH_MALUR_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "MALUR MONTH Insert error: %s\n", query);
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
