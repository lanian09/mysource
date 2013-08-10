#include "stmd_proto.h"

extern  char    trcBuf[4096], trcTmp[1024];
extern  int     trcLogFlag ;
extern  int     trcLogId;
extern  MYSQL   sql, *conn;
extern	short	delTIME[STMD_PERIOD_TYPE_NUM];

typedef struct  _leg_stat {
  char system_name[11];
  char pdsn_ip[19];
  int  rx_cnt;
  int  start;
  int  interim;
  int  disconnect;
  int  stop;
  int  start_logon_cnt;
  int  int_logon_cnt;
  int  disc_logon_cnt;
  int  logout_cnt;
  int  stat_cnt;
} LEG_STAT_lo, *PLEG_STAT_lo;

int stmd_LegUpdate(int time_type, char *table_type, MYSQL_ROW inrow)
{
    char        query[4096*2];
    char        starttime[32],endtime[32],seltbl[32],sysname[12], pdsn[20];
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

	memset (seltbl, 0x00, sizeof(seltbl));
	sprintf(sysname, "%s", inrow[0]);
	sprintf(pdsn, "%s", inrow[1]);
    sprintf(starttime, "%s", get_select_time2(time_type));
    sprintf(endtime, "%s", get_period_end_time2(time_type));    

    switch(time_type)
    {
		case STMD_HOUR:
			strcpy(seltbl, STM_STATISTIC_5MINUTE_LEG_TBL_NAME);
			break;
		case STMD_DAY:
			strcpy(seltbl, STM_STATISTIC_HOUR_LEG_TBL_NAME);
			break;
		case STMD_WEEK:
			strcpy(seltbl, STM_STATISTIC_DAY_LEG_TBL_NAME);
			break;
		case STMD_MONTH:
			strcpy(seltbl, STM_STATISTIC_DAY_LEG_TBL_NAME);
			break;
		default:
			strcpy(seltbl, STM_STATISTIC_HOUR_LEG_TBL_NAME);
			break;
    }

    if (time_type == STMD_HOUR || time_type == STMD_DAY){
    	sprintf(query, "SELECT "
                " IFNULL(SUM(rx_cnt),0), "
                " IFNULL(SUM(start),0), "
                " IFNULL(SUM(interim),0), "
                " IFNULL(SUM(disconnect),0), "
                " IFNULL(SUM(stop),0), "
                " IFNULL(SUM(start_logon_cnt),0), "
                " IFNULL(SUM(int_logon_cnt),0), "
                " IFNULL(SUM(disc_logon_cnt),0), "
                " IFNULL(SUM(logout_cnt),0), "
                " IFNULL(SUM(stat_cnt),0) "
                " FROM %s WHERE system_name = '%s' and pdsn_ip = '%s' "
				" and (stat_date > '%s' and stat_date <= '%s')",
                seltbl, sysname, pdsn, starttime, endtime);
    }else{
	sprintf(query, "SELECT "
                " IFNULL(SUM(rx_cnt),0), "
                " IFNULL(SUM(start),0), "
                " IFNULL(SUM(interim),0), "
                " IFNULL(SUM(disconnect),0), "
                " IFNULL(SUM(stop),0), "
                " IFNULL(SUM(start_logon_cnt),0), "
                " IFNULL(SUM(int_logon_cnt),0), "
                " IFNULL(SUM(disc_logon_cnt),0), "
                " IFNULL(SUM(logout_cnt),0), "
                " IFNULL(SUM(stat_cnt),0) "
                " FROM %s WHERE system_name = '%s' and pdsn_ip = '%s' "
				" and (stat_date >= '%s' and stat_date < '%s')",
                seltbl, sysname, pdsn, starttime, endtime);
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
        sprintf(trcBuf, "[%d]%s Leg select error\n%s\n", time_type, seltbl, query);
        trclib_writeLogErr (FL, trcBuf);
		mysql_free_result(result1); // MEM if NULL 10.07
        return -1;
    }
    
    // µ•¿Ã≈∏∏¶ update«—¥Ÿ.
    sprintf(query, "UPDATE %s SET " 
                    "rx_cnt = %s, start = %s, interim = %s, disconnect = %s, stop = %s, "
                    "start_logon_cnt = %s, int_logon_cnt = %s, disc_logon_cnt = %s, logout_cnt = %s, "
                    "stat_cnt = %s "
                    " WHERE system_name = '%s' and pdsn_ip = '%s' and (stat_date = '%s')", 
                    table_type,
                    row1[0],row1[1], row1[2], row1[3], row1[4],
                    row1[5],row1[6], row1[7], row1[8],
					row1[9], sysname, pdsn, get_select_time(time_type));
    
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

int stmd_LegInsert(int time_type, char *table_type, MYSQL_ROW inrow)
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
                                    " '%s', '%s', "
									" '%s', '%s', '%s', '%s', '%s', "
									" '%s', '%s', '%s', '%s', "
									" '%s', '%s', '%s' )", 
                                    table_type,
                                    inrow[0], inrow[1], 
									inrow[2], inrow[3], inrow[4], inrow[5], inrow[6], 
									inrow[7], inrow[8], inrow[9], inrow[10],
									inrow[11], get_select_time(time_type), weektype);
                                    
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n%s\n", mysql_error(conn), query);
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }

    return 1;
}

char *getUpdateHourLegQuery(char *query, MYSQL_ROW row)
{
	switch(atoi(row[12])){
		case 0 : //update query
			sprintf(query, "UPDATE %s SET " 
				"rx_cnt = %s, start = %s, interim = %s, disconnect = %s, stop = %s, "
				"start_logon_cnt = %s, int_logon_cnt = %s, disc_logon_cnt = %s, logout_cnt = %s, "
				"stat_cnt = %s "
				" WHERE system_name = '%s' and pdsn_ip = '%s' and (stat_date = '%s')", 
				STM_STATISTIC_HOUR_LEG_TBL_NAME,
				row[2], row[3], row[4], row[5],row[6], 
				row[7], row[8], row[9], row[10],
				row[11],
				row[0], row[1], get_select_time(STMD_HOUR));
			break;
		case 1 : //insert query
			sprintf(query, "INSERT INTO %s VALUES ("
				" '%s', '%s', "
				" '%s', '%s', '%s', '%s', '%s', "
				" '%s', '%s', '%s', '%s', "
				" '%s', '%s', '%s' )", 
				STM_STATISTIC_HOUR_LEG_TBL_NAME,
				row[0], row[1], 
				row[2], row[3], row[4], row[5], row[6], 
				row[7], row[8], row[9], row[10],
				row[11], get_select_time(STMD_HOUR), get_insert_week());
			break;
	}
	return query;

}

char *getUpdateHourLegSelectQuery(char *query, char *t_5min)
{
	sprintf(query,"SELECT F.system_name, F.pdsn_ip,"  // 0,1
		"(F.rx_cnt+IFNULL(H.rx_cnt,0)) rx_cnt,"		// 2
		"(F.start+IFNULL(H.start,0)) `start`,"		// 3
		"(F.interim+IFNULL(H.interim,0)) interim,"  // 4
		"(F.disconnect+IFNULL(H.disconnect,0)) disconnect," //5
		"(F.stop+IFNULL(H.stop,0)) `stop`,"					//6
		"(F.start_logon_cnt+IFNULL(H.start_logon_cnt,0)) start_logon_cnt," //7
		"(F.int_logon_cnt+IFNULL(H.int_logon_cnt,0)) int_logon_cnt,"	   //8
		"(F.disc_logon_cnt+IFNULL(H.disc_logon_cnt,0)) disc_logon_cnt,"    //9
		"(F.logout_cnt+IFNULL(H.logout_cnt,0)) logout_cnt,"				   //10
		"(F.stat_cnt+IFNULL(H.stat_cnt,0)) stat_cnt,"					   //11
		"IF(IFNULL(H.pdsn_ip,1)=1,1,0) AS ins "						   //12 - 0=update, 1=insert
		"FROM %s AS F LEFT JOIN ( SELECT * FROM %s WHERE stat_date='%s' ) AS H "
		"ON F.system_name = H.system_name AND F.pdsn_ip = H.pdsn_ip "
		"WHERE F.stat_date = '%s'",
		STM_STATISTIC_5MINUTE_LEG_TBL_NAME,
		STM_STATISTIC_HOUR_LEG_TBL_NAME,
		get_select_time(STMD_HOUR), 
		t_5min);
	return query;
}

int stmd_exeUpdateHourLeg()
{
    char    query[4096],insert_time[32];
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
            STM_STATISTIC_HOUR_LEG_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_HOUR]));

    if ( trcLogFlag == TRCLEVEL_SQL ) 
	{
        sprintf(trcBuf, "delete query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }

	// ø¿∑°µ» Ω√∞£ – ≈Î∞Ë ªË¡¶
    if (stmd_mysql_query (query) < 0) 
	{
        sprintf(trcBuf, "LegStatistic 1 Hour mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
		return -1;
    }

	//left join ¿ª ªÁøÎ«ÿº≠ 5∫– ≈Î∞ËøÕ Ω√∞£ ≈Î∞Ë∏¶ «—π¯ø° ¡∂»∏
	// πŸ∑Œ ∞·∞˙∏¶ row ∏∂¥Ÿ update or insert∏¶ ºˆ«‡
    sprintf(insert_time, "%s", get_insert_time());
    if(strcasecmp(insert_time,"") == 0)
    {
        sprintf(trcBuf, "Leg Statistic(Hour) 5minute time get fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -2;
    }

	getUpdateHourLegSelectQuery(query, insert_time);
    if ( trcLogFlag == TRCLEVEL_SQL ) 
	{
        sprintf(trcBuf, "select query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -3;
    }

    result1 = mysql_store_result(conn);
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
		getUpdateHourLegQuery(query, row1);
		if ( trcLogFlag == TRCLEVEL_SQL ) 
		{
			sprintf(trcBuf, "update query = %s\n", query);
			trclib_writeLog(FL, trcBuf);
		}
		if (stmd_mysql_query (query) < 0) {
			sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
			trclib_writeLogErr (FL,trcBuf);
			return -4;
		}
    }
    
    mysql_free_result(result1);

    return 1;
}
#if 0
int stmd_exeUpdateHourLeg_OLD()
{
    char    query[4096],insert_time[32];
    int     ret_select;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

	/*
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
            STM_STATISTIC_HOUR_LEG_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*3)); // 3¥ﬁ 
			*/
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
            STM_STATISTIC_HOUR_LEG_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_HOUR]));

    if ( trcLogFlag == TRCLEVEL_SQL ) 
	{
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }

logPrint(trcLogId, FL, "stmd_exeUpdateHourLeg(DELETE QUERY) start = %d\n", time(0));
    if (stmd_mysql_query (query) < 0) 
	{
        sprintf(trcBuf, "LegStatistic 1 Hour mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
		return -1;
    }

    /*«ˆ¿Á Ω√∞£¿ª ±∏«—¥Ÿ¿Ω 5∫– tableø°º≠ ¡§∫∏∏¶ select «œ∞Ì
    row ∏∏≈≠ loop -> upate / insert */
    sprintf(insert_time, "%s", get_insert_time());
    if(strcasecmp(insert_time,"") == 0)
    {
        sprintf(trcBuf, "Leg Statistic(Hour) 5minute time get fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
    sprintf(query, "SELECT * FROM %s where ( stat_date = '%s')",
                STM_STATISTIC_5MINUTE_LEG_TBL_NAME, insert_time);

logPrint(trcLogId, FL, "stmd_exeUpdateHourLeg(SELECT QUERY) start = %d\n", time(0));
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    result1 = mysql_store_result(conn);
    
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
        sprintf(query, "SELECT * FROM %s where (system_name = '%s' and "
						" pdsn_ip = '%s' and stat_date = '%s')",
                    STM_STATISTIC_HOUR_LEG_TBL_NAME, row1[0], row1[1], get_select_time(STMD_HOUR));

logPrint(trcLogId, FL, "stmd_exeUpdateHourLeg(SELECT(each) QUERY) start = %d\n", time(0));
        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
logPrint(trcLogId, FL, "stmd_exeUpdateHourLeg(UPDATE(each) QUERY) start = %d\n", time(0));
            if ( stmd_LegUpdate(STMD_HOUR, STM_STATISTIC_HOUR_LEG_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "LEG HOUR Update error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
logPrint(trcLogId, FL, "stmd_exeUpdateHourLeg(INSERT(each) QUERY) start = %d\n", time(0));
            if ( stmd_LegInsert(STMD_HOUR, STM_STATISTIC_HOUR_LEG_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "LEG HOUR Insert error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else 
        {
            sprintf(trcBuf,"stmd_exeUpdateHourLeg Update/Insert Fail = %d\n",ret_select);
            trclib_writeLogErr (FL,trcBuf);
            mysql_free_result(result1);
            return -1;
        }
    }
    
    mysql_free_result(result1);

    return 1;
}
#endif

int stmd_exeUpdateDayLeg()
{
    char    query[4096],insert_time[32];
    int     ret_select;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

	/*
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_DAY_LEG_TBL_NAME, get_delete_time(STMD_1MON_OFFSET)); // 1≥‚
		*/
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_DAY_LEG_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_DAY]));

    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }

    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "Leg 1 Day mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
		return -1;
    }

///////////////////
    /*«ˆ¿Á Ω√∞£¿ª ±∏«—¥Ÿ¿Ω  
    5∫– tableø°º≠ ¡§∫∏∏¶ select «œ∞Ì
    row ∏∏≈≠ loop -> upate / insert */
    sprintf(insert_time, "%s", get_insert_time());
    if(strcasecmp(insert_time,"") == 0)
    {
        sprintf(trcBuf, "Leg Statistic(Day) preview 5minute time get fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
    sprintf(query, "SELECT * FROM %s where (stat_date = '%s')",
                STM_STATISTIC_5MINUTE_LEG_TBL_NAME, insert_time);
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    result1 = mysql_store_result(conn);
    
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
        sprintf(query, "SELECT * FROM %s where (system_name = '%s' and "
						" pdsn_ip = '%s' and stat_date = '%s')",
                    STM_STATISTIC_DAY_LEG_TBL_NAME, row1[0], row1[1], get_select_time(STMD_DAY));

        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
            if ( stmd_LegUpdate(STMD_DAY, STM_STATISTIC_DAY_LEG_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "LEG DAY Update error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_LegInsert(STMD_DAY, STM_STATISTIC_DAY_LEG_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "LEG DAY Insert error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else 
        {
            sprintf(trcBuf,"stmd_exeUpdateDayLeg Update/Insert Fail = %d\n",ret_select);
            trclib_writeLogErr (FL,trcBuf);
            mysql_free_result(result1);
            return -1;
        }
    }
    
    mysql_free_result(result1);

///////////////////

    return 1;
}


int stmd_exeUpdateWeekLeg()
{
    char    query[4096],insert_time[32];
    int     ret_select;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

	/*
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_WEEK_LEG_TBL_NAME, get_delete_time(STMD_1MON_OFFSET));
		*/
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_WEEK_LEG_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_WEEK]));

    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "LegStatistic 1 Week mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
		return -1;
    }

///////////////////
    /*«ˆ¿Á Ω√∞£∫∏¥Ÿ 5∫–¿¸ Ω√∞£¿ª ±∏«—¥Ÿ¿Ω
    5∫– tableø°º≠ ¡§∫∏∏¶ select «œ∞Ì
    row ∏∏≈≠ loop -> upate / insert */
    sprintf (insert_time, "%s", get_insert_time());
    if(strcasecmp(insert_time,"") == 0)
    {
        sprintf(trcBuf, "Leg Statistic(Week) preview 5minute time get fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
    sprintf(query, "SELECT * FROM %s where (stat_date = '%s')",
                STM_STATISTIC_5MINUTE_LEG_TBL_NAME, insert_time);
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    result1 = mysql_store_result(conn);
    
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
        sprintf(query, "SELECT * FROM %s where (system_name = '%s' and "
						" pdsn_ip = '%s' and stat_date = '%s')",
                    STM_STATISTIC_WEEK_LEG_TBL_NAME, row1[0], row1[1], get_select_time(STMD_WEEK));
        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
            if ( stmd_LegUpdate(STMD_WEEK, STM_STATISTIC_WEEK_LEG_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "LEG WEEK Update error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_LegInsert(STMD_WEEK, STM_STATISTIC_WEEK_LEG_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "LEG WEEK Insert error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else 
        {
            sprintf(trcBuf,"stmd_exeUpdateWeekLeg Update/Insert Fail = %d\n",ret_select);
            trclib_writeLogErr (FL,trcBuf);
            mysql_free_result(result1);
            return -1;
        }
    }
    
    mysql_free_result(result1);

///////////////////

    return 1;
}



int stmd_exeUpdateMonthLeg()
{
    char    query[4096],insert_time[32];
    int     ret_select;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

    // 1≥‚ ¿¸¿« «—¥ﬁ µ•¿Ã≈∏∏¶ ¡ˆøÓ¥Ÿ.
	/*
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_MONTH_LEG_TBL_NAME, get_delete_time(STMD_1MON_OFFSET));
		*/
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_MONTH_LEG_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_MONTH]));

    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "LegStatistic 1 Month mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
		return -1;
    }

///////////////////
    /*«ˆ¿Á Ω√∞£∫∏¥Ÿ 5∫–¿¸ Ω√∞£¿ª ±∏«—¥Ÿ¿Ω
    5∫– tableø°º≠ ¡§∫∏∏¶ select «œ∞Ì
    row ∏∏≈≠ loop -> upate / insert */
    sprintf (insert_time, "%s", get_insert_time());
    if(strcasecmp(insert_time,"") == 0)
    {
        sprintf(trcBuf, "Leg Statistic(Month) preview 5minute time get fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
    sprintf(query, "SELECT * FROM %s where (stat_date = '%s')",
                STM_STATISTIC_5MINUTE_LEG_TBL_NAME, insert_time);
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    result1 = mysql_store_result(conn);
    
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
        sprintf(query, "SELECT * FROM %s where (system_name = '%s' and "
						" pdsn_ip = '%s' and stat_date = '%s')",
                    STM_STATISTIC_MONTH_LEG_TBL_NAME, row1[0], row1[1], get_select_time(STMD_MONTH));

        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
            if ( stmd_LegUpdate(STMD_MONTH, STM_STATISTIC_MONTH_LEG_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "LEG MONTH Update error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_LegInsert(STMD_MONTH, STM_STATISTIC_MONTH_LEG_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "LEG MONTH Insert error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else 
        {
            sprintf(trcBuf,"stmd_exeUpdateMonthLeg Update/Insert Fail = %d\n",ret_select);
            trclib_writeLogErr (FL,trcBuf);
            mysql_free_result(result1);
            return -1;
        }
    }
    
    mysql_free_result(result1);

    return 1;
}
