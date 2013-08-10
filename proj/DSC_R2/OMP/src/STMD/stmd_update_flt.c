#include "stmd_proto.h"

extern  char    trcBuf[4096], trcTmp[1024];
extern  int     trcLogFlag; 
extern  MYSQL   sql, *conn;
extern  int     sysCnt;
extern  STM_CommStatisticInfo    StatisticSystemInfo[SYSCONF_MAX_ASSO_SYS_NUM];
extern short 	delTIME[STMD_PERIOD_TYPE_NUM];

//int   stmd_FltUpdate(int time_type, char *table_type, char *system_name)
int stmd_FltUpdate(int time_type, char *table_type, MYSQL_ROW inrow)
{
    char        query[4096];
    char        starttime[32],endtime[32],seltbl[32],sysname[12];
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

	memset (seltbl, 0x00, sizeof(seltbl));

    memset(sysname,0,sizeof(sysname));
    memset(starttime, 0, sizeof(starttime));
    memset(endtime, 0, sizeof(endtime));
    memset(seltbl, 0, sizeof(seltbl));

    sprintf(sysname, "%s", inrow[1]);
    
    sprintf(starttime, "%s", get_select_time2(time_type));
    sprintf(endtime, "%s", get_period_end_time2(time_type));    

    switch(time_type)
    {
    case STMD_HOUR:
        strcpy(seltbl, STM_STATISTIC_5MINUTE_FAULT_TBL_NAME);
        break;
    case STMD_DAY:
        strcpy(seltbl, STM_STATISTIC_HOUR_FAULT_TBL_NAME);
        break;
    case STMD_WEEK:
        strcpy(seltbl, STM_STATISTIC_DAY_FAULT_TBL_NAME);
        break;
    case STMD_MONTH:
        strcpy(seltbl, STM_STATISTIC_DAY_FAULT_TBL_NAME);
        break;
    default:
        strcpy(seltbl, STM_STATISTIC_HOUR_FAULT_TBL_NAME);
        break;
    }

//fprintf(stderr, "[%s:%d]#1\n", __FILE__, __LINE__);
   
    if ( time_type == STMD_HOUR || time_type == STMD_DAY) { 
    	sprintf(query, "SELECT "
                " SUM(cpu_min_cnt), SUM(cpu_maj_cnt), SUM(cpu_cri_cnt)," 
                " SUM(mem_min_cnt), SUM(mem_maj_cnt), SUM(mem_cri_cnt)," 
                " SUM(etc_hw_min_cnt), SUM(etc_hw_maj_cnt), SUM(etc_hw_cri_cnt)," 
                " SUM(proc_min_cnt), SUM(proc_maj_cnt), SUM(proc_cri_cnt)," 
   //             " SUM(net_nms_min_cnt), SUM(net_nms_maj_cnt), SUM(net_nms_cri_cnt)," 
   //             " SUM(sess_ntp_min_cnt), SUM(sess_ntp_maj_cnt), SUM(sess_ntp_cri_cnt)," 
   //             " SUM(sess_nms_min_cnt), SUM(sess_nms_maj_cnt), SUM(sess_nms_cri_cnt)," 
                " SUM(stat_cnt) "
                " FROM %s WHERE (system_name = '%s' and stat_date > '%s' and stat_date <= '%s')",
                seltbl, sysname, starttime, endtime);
    }else {
	sprintf(query, "SELECT "
                " SUM(cpu_min_cnt), SUM(cpu_maj_cnt), SUM(cpu_cri_cnt),"
                " SUM(mem_min_cnt), SUM(mem_maj_cnt), SUM(mem_cri_cnt),"
                " SUM(etc_hw_min_cnt), SUM(etc_hw_maj_cnt), SUM(etc_hw_cri_cnt),"
                " SUM(proc_min_cnt), SUM(proc_maj_cnt), SUM(proc_cri_cnt),"
   //             " SUM(net_nms_min_cnt), SUM(net_nms_maj_cnt), SUM(net_nms_cri_cnt),"
   //             " SUM(sess_ntp_min_cnt), SUM(sess_ntp_maj_cnt), SUM(sess_ntp_cri_cnt),"
   //             " SUM(sess_nms_min_cnt), SUM(sess_nms_maj_cnt), SUM(sess_nms_cri_cnt),"
                " SUM(stat_cnt) "
                " FROM %s WHERE (system_name = '%s' and stat_date >= '%s' and stat_date < '%s')",
                seltbl, sysname, starttime, endtime);

    }
//fprintf(stderr, "[%s:%d]%s\n", __FILE__, __LINE__, query);

	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
    result1 = mysql_store_result(conn);
    row1 = mysql_fetch_row(result1);   
 
    if (row1 == NULL )
    {
        sprintf(trcBuf, "[%d]%s Fault select error\n", time_type, seltbl);
        trclib_writeLogErr (FL, trcBuf);
		mysql_free_result(result1);
        return -1;
    }

//fprintf(stderr, "[%s:%d]#2\n", __FILE__, __LINE__);
    
    // 데이타를 update한다.
    sprintf(query, "UPDATE %s SET " 
                    "cpu_min_cnt = %s, cpu_maj_cnt = %s, cpu_cri_cnt = %s, "
                    "mem_min_cnt = %s, mem_maj_cnt = %s, mem_cri_cnt = %s, "
                    "etc_hw_min_cnt = %s, etc_hw_maj_cnt = %s, etc_hw_cri_cnt = %s, "
                    "proc_min_cnt = %s, proc_maj_cnt = %s, proc_cri_cnt = %s, "
   //                 "net_nms_min_cnt = %s, net_nms_maj_cnt = %s, net_nms_cri_cnt = %s, "
   //                 "sess_ntp_min_cnt = %s, sess_ntp_maj_cnt = %s, sess_ntp_cri_cnt = %s, "
   //                 "sess_nms_min_cnt = %s, sess_nms_maj_cnt = %s, sess_nms_cri_cnt = %s, "
                    "stat_cnt = %s "
                    " WHERE (stat_date = '%s' AND system_name='%s')", 
                    table_type,
                    row1[0], row1[1], row1[2],
                    row1[3], row1[4], row1[5],
                    row1[6], row1[7], row1[8],
                    row1[9], row1[10], row1[11],
   //                 row1[12], row1[13], row1[14],
   //                 row1[15], row1[16], row1[17],
   //                 row1[18], row1[19], row1[20],
                    row1[12],
                    get_select_time(time_type), sysname);
//fprintf(stderr, "[%s:%d]%s\n", __FILE__, __LINE__, query);
    
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

int stmd_FltInsert(int time_type, char *table_type, MYSQL_ROW inrow)
{
    char        query[4096];
    char    	weektype[32];


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

    sprintf(query, "INSERT INTO %s VALUES ("
                                    "'%s', '%s',"
                                    " '%s', '%s', '%s',"
                                    " '%s', '%s', '%s',"
                                    " '%s', '%s', '%s',"
                                    " '%s', '%s', '%s',"
   //                                 " '%s', '%s', '%s',"
   //                                 " '%s', '%s', '%s',"
   //                                 " '%s', '%s', '%s',"
                                    " '%s', '%s', '%s' )", 
                                    table_type,
                                    inrow[0], inrow[1],
                                    inrow[2], inrow[3], inrow[4],
                                    inrow[5], inrow[6], inrow[7],
                                    inrow[8], inrow[9], inrow[10],
                                    inrow[11], inrow[12], inrow[13],
   //                                 inrow[14], inrow[15], inrow[16],
   //                                 inrow[17], inrow[18], inrow[19],
   //                                 inrow[20], inrow[21], inrow[22], inrow[23], 
									 inrow[14],get_select_time(time_type), weektype);
                                    
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; query: %s; err=%s\n", mysql_error(conn), query);
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
////////////////////////

    return 1;
}

#if 0
int stmd_bsdFltUpdate(int time_type, char *table_type, MYSQL_ROW inrow)
{
    char        query[4096];
    char        starttime[32],endtime[32],seltbl[32],sysname[12];
//  STM_CommonFltStatisticInfo  FltStatisticInfo[3];
    MYSQL_RES   *result1, *result2;
    MYSQL_ROW   row1, row2;
    int         update_cnt = 0;

    memset(sysname,0,sizeof(sysname));
    memset(starttime, 0, sizeof(starttime));
    memset(endtime, 0, sizeof(endtime));
    memset(seltbl, 0, sizeof(seltbl));

    sprintf(sysname, "%s", inrow[1]);
    
    sprintf(starttime, "%s", get_select_time2(time_type));
    sprintf(endtime, "%s", get_period_end_time2(time_type));    

    switch(time_type)
    {
    case STMD_HOUR:
        strcpy(seltbl, STM_STATISTIC_5MINUTE_BSD_FLT_TBL_NAME);
        break;
    case STMD_DAY:
        strcpy(seltbl, STM_STATISTIC_HOUR_BSD_FLT_TBL_NAME);
        break;
    case STMD_WEEK:
        strcpy(seltbl, STM_STATISTIC_DAY_BSD_FLT_TBL_NAME);
        break;
    case STMD_MONTH:
        strcpy(seltbl, STM_STATISTIC_DAY_BSD_FLT_TBL_NAME);
        break;
    default:
        strcpy(seltbl, STM_STATISTIC_HOUR_BSD_FLT_TBL_NAME);
        break;
    }

    if ( time_type == STMD_HOUR || time_type == STMD_DAY) {
    	sprintf(query, "SELECT "
                " SUM(dup_hb_min_cnt), SUM(dup_hb_maj_cnt), SUM(dup_hb_cri_cnt)," 
                " SUM(dup_oos_min_cnt), SUM(dup_oos_maj_cnt), SUM(dup_oos_cri_cnt)," 
                " SUM(succ_wap1ana_min_cnt), SUM(succ_wap1ana_maj_cnt), SUM(succ_wap1ana_cri_cnt)," 
                " SUM(succ_wap2ana_min_cnt), SUM(succ_wap2ana_maj_cnt), SUM(succ_wap2ana_cri_cnt)," 
                " SUM(succ_httpana_min_cnt), SUM(succ_httpana_maj_cnt), SUM(succ_httpana_cri_cnt)," 
                " SUM(succ_uawap_min_cnt), SUM(succ_uawap_maj_cnt), SUM(succ_uawap_cri_cnt)," 
                " SUM(succ_aaa_min_cnt), SUM(succ_aaa_maj_cnt), SUM(succ_aaa_cri_cnt),"
                " SUM(succ_vods_min_cnt), SUM(succ_vods_maj_cnt), SUM(succ_vods_cri_cnt)," 
                " SUM(succ_anaaa_min_cnt), SUM(succ_anaaa_maj_cnt), SUM(succ_anaaa_cri_cnt)," 
               	" SUM(succ_vt_min_cnt), SUM(succ_vt_maj_cnt), SUM(succ_vt_cri_cnt)," 
		" SUM(succ_radius_min_cnt), SUM(succ_radius_maj_cnt), SUM(succ_radius_cri_cnt),"	
		" SUM(net_pdsn_min_cnt), SUM(net_pdsn_maj_cnt), SUM(net_pdsn_cri_cnt)," 
                " SUM(net_uawap_min_cnt), SUM(net_uawap_maj_cnt), SUM(net_uawap_cri_cnt)," 
                " SUM(net_aaa_min_cnt), SUM(net_aaa_maj_cnt), SUM(net_aaa_cri_cnt)," 
                " SUM(sess_ntp_min_cnt), SUM(sess_ntp_maj_cnt), SUM(sess_ntp_cri_cnt)," 
                " SUM(sess_uawap_min_cnt), SUM(sess_uawap_maj_cnt), SUM(sess_uawap_cri_cnt)," 
                " SUM(stat_cnt) "
                " FROM %s WHERE (system_name = '%s' and stat_date > '%s' and stat_date <= '%s')",
                seltbl, sysname, starttime, endtime);
    }else{
	sprintf(query, "SELECT "
                " SUM(dup_hb_min_cnt), SUM(dup_hb_maj_cnt), SUM(dup_hb_cri_cnt),"
                " SUM(dup_oos_min_cnt), SUM(dup_oos_maj_cnt), SUM(dup_oos_cri_cnt),"
                " SUM(succ_wap1ana_min_cnt), SUM(succ_wap1ana_maj_cnt), SUM(succ_wap1ana_cri_cnt),"
                " SUM(succ_wap2ana_min_cnt), SUM(succ_wap2ana_maj_cnt), SUM(succ_wap2ana_cri_cnt),"
                " SUM(succ_httpana_min_cnt), SUM(succ_httpana_maj_cnt), SUM(succ_httpana_cri_cnt),"
                " SUM(succ_uawap_min_cnt), SUM(succ_uawap_maj_cnt), SUM(succ_uawap_cri_cnt),"
                " SUM(succ_aaa_min_cnt), SUM(succ_aaa_maj_cnt), SUM(succ_aaa_cri_cnt),"
                " SUM(succ_vods_min_cnt), SUM(succ_vods_maj_cnt), SUM(succ_vods_cri_cnt),"
                " SUM(succ_anaaa_min_cnt), SUM(succ_anaaa_maj_cnt), SUM(succ_anaaa_cri_cnt),"
                " SUM(succ_vt_min_cnt), SUM(succ_vt_maj_cnt), SUM(succ_vt_cri_cnt),"
                " SUM(succ_radius_min_cnt), SUM(succ_radius_maj_cnt), SUM(succ_radius_cri_cnt),"
                " SUM(net_pdsn_min_cnt), SUM(net_pdsn_maj_cnt), SUM(net_pdsn_cri_cnt),"
                " SUM(net_uawap_min_cnt), SUM(net_uawap_maj_cnt), SUM(net_uawap_cri_cnt),"
                " SUM(net_aaa_min_cnt), SUM(net_aaa_maj_cnt), SUM(net_aaa_cri_cnt),"
                " SUM(sess_ntp_min_cnt), SUM(sess_ntp_maj_cnt), SUM(sess_ntp_cri_cnt),"
                " SUM(sess_uawap_min_cnt), SUM(sess_uawap_maj_cnt), SUM(sess_uawap_cri_cnt),"
                " SUM(stat_cnt) "
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

    if (row1[0] == NULL )
    {
        sprintf(trcBuf, "[%d]%s Fault select error\n%s\n", time_type, seltbl, query);
        trclib_writeLogErr (FL, trcBuf);
        return -1;
    }


    
    // 데이타를 update한다.
    sprintf(query, "UPDATE %s SET " 
                    " dup_hb_min_cnt=%s, dup_hb_maj_cnt=%s, dup_hb_cri_cnt=%s," 
                    " dup_oos_min_cnt=%s, dup_oos_maj_cnt=%s, dup_oos_cri_cnt=%s," 
                    " succ_wap1ana_min_cnt=%s, succ_wap1ana_maj_cnt=%s, succ_wap1ana_cri_cnt=%s," 
                    " succ_wap2ana_min_cnt=%s, succ_wap2ana_maj_cnt=%s, succ_wap2ana_cri_cnt=%s," 
                    " succ_httpana_min_cnt=%s, succ_httpana_maj_cnt=%s, succ_httpana_cri_cnt=%s," 
                    " succ_uawap_min_cnt=%s, succ_uawap_maj_cnt=%s, succ_uawap_cri_cnt=%s," 
                    " succ_aaa_min_cnt=%s, succ_aaa_maj_cnt=%s, succ_aaa_cri_cnt=%s," 
                    " succ_vods_min_cnt=%s, succ_vods_maj_cnt=%s, succ_vods_cri_cnt=%s,"
                    " succ_anaaa_min_cnt=%s, succ_anaaa_maj_cnt=%s, succ_anaaa_cri_cnt=%s,"
                    " succ_vt_min_cnt=%s, succ_vt_maj_cnt=%s, succ_vt_cri_cnt=%s," 
		    " succ_radius_min_cnt=%s, succ_radius_maj_cnt=%s, succ_radius_cri_cnt=%s," 
		    " net_pdsn_min_cnt=%s, net_pdsn_maj_cnt=%s, net_pdsn_cri_cnt=%s," 
                    " net_uawap_min_cnt=%s, net_uawap_maj_cnt=%s, net_uawap_cri_cnt=%s," 
                    " net_aaa_min_cnt=%s, net_aaa_maj_cnt=%s, net_aaa_cri_cnt=%s," 
                    " sess_ntp_min_cnt=%s, sess_ntp_maj_cnt=%s, sess_ntp_cri_cnt=%s," 
                    " sess_uawap_min_cnt=%s, sess_uawap_maj_cnt=%s, sess_uawap_cri_cnt=%s," 
                    " stat_cnt=%s "
                    " WHERE (stat_date = '%s' AND system_name='%s')", 
                    table_type,
                    row1[0], row1[1], row1[2],
                    row1[3], row1[4], row1[5],
                    row1[6], row1[7], row1[8],
                    row1[9], row1[10], row1[11],
                    row1[12], row1[13], row1[14],
                    row1[15], row1[16], row1[17],
                    row1[18], row1[19], row1[20],
                    row1[21], row1[22], row1[23],
                    row1[24], row1[25], row1[26],
                    row1[27], row1[28], row1[29],
                    row1[30], row1[31], row1[32],
                    row1[33], row1[34], row1[35],
                    row1[36], row1[37], row1[38],
                    row1[39], row1[40], row1[41],
                    row1[42], row1[43], row1[44], 
		    row1[45], row1[46], row1[47], 
		    row1[48], get_select_time(time_type), sysname);
    
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

int stmd_bsdFltInsert(int time_type, char *table_type, MYSQL_ROW inrow)
{
    char        query[4096];
//  STM_CommonFltStatisticInfo  FltStatisticInfo[3];
//  MYSQL_RES   *result1, *result2;
//  MYSQL_ROW   row1, row2;
    int         insert_cnt = 0;
    char    weektype[32];


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

    sprintf(query, "INSERT INTO %s VALUES ("
                   "'%s', '%s',"
                   " '%s', '%s', '%s',"
                   " '%s', '%s', '%s',"
                   " '%s', '%s', '%s',"
                   " '%s', '%s', '%s',"
                   " '%s', '%s', '%s',"
                   " '%s', '%s', '%s',"
                   " '%s', '%s', '%s',"
                   " '%s', '%s', '%s'," 
	           " '%s', '%s', '%s',"
                   " '%s', '%s', '%s',"
                   " '%s', '%s', '%s',"
                   " '%s', '%s', '%s',"
                   " '%s', '%s', '%s',"
                   " '%s', '%s', '%s'," 
		   " '%s', '%s', '%s',"
                   " '%s', '%s', '%s',"
                   " '%s', '%s', '%s' )", 
                   table_type,
                   inrow[0], inrow[1],
                   inrow[2], inrow[3], inrow[4],
                   inrow[5], inrow[6], inrow[7],
                   inrow[8], inrow[9], inrow[10],
                   inrow[11], inrow[12], inrow[13],
                   inrow[14], inrow[15], inrow[16],
                   inrow[17], inrow[18], inrow[19],
                   inrow[20], inrow[21], inrow[22],
                   inrow[23], inrow[24], inrow[25],
                   inrow[26], inrow[27], inrow[28],
                   inrow[29], inrow[30], inrow[31],
                   inrow[32], inrow[33], inrow[34],
                   inrow[35], inrow[36], inrow[37],
                   inrow[38], inrow[39], inrow[40],
                   inrow[41], inrow[42], inrow[43],
                   inrow[44], inrow[45], inrow[46], 
		   inrow[47], inrow[48], inrow[49], 
		   inrow[50], get_select_time(time_type), weektype);
                                    
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n%s\n", mysql_error(conn), query);
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
////////////////////////

    return 1;
}
#endif

int stmd_exeUpdateHourFlt()
{
    char        query[4096],insert_time[32];
    int         ret_select;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

    // 하루 전의 1시간 데이타를 지운다.
#if 0
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_HOUR_FAULT_TBL_NAME, get_delete_time(STMD_1HOUR_WEEK_OFFSET));
#endif
	/*
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_HOUR_FAULT_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*3));
		*/
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_HOUR_FAULT_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_HOUR]));
    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "FltStatistic 1 Hour mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
		return -1;
    }

    // 하루 전의 1시간 데이타를 지운다.
#if 0
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_HOUR_BSD_FLT_TBL_NAME, get_delete_time(STMD_1HOUR_WEEK_OFFSET));
#endif

#if 0 // jjinri
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_HOUR_BSD_FLT_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*3));
    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "BsdFltStatistic 1 Hour mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
    }
#endif
    
    /*현재 시간보다 5분전 시간을 구한다음
    5분 table에서 정보를 select 하고
    row 만큼 loop -> upate / insert */
    strcpy(insert_time, get_insert_time());
    if(strcasecmp(insert_time,"") == 0)
    {
        sprintf(trcBuf, "FAULT Statistic(Hour) preview 5minute time get fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
    sprintf(query, "SELECT * FROM %s where (stat_date = '%s')",
                STM_STATISTIC_5MINUTE_FAULT_TBL_NAME, insert_time);
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }

    result1 = mysql_store_result(conn);
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
        sprintf(query, "SELECT * FROM %s where (system_name = '%s' and stat_date = '%s')",
                    STM_STATISTIC_HOUR_FAULT_TBL_NAME, row1[1], get_select_time(STMD_HOUR));
        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
            if ( stmd_FltUpdate(STMD_HOUR, STM_STATISTIC_HOUR_FAULT_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "FAULT HOUR Update error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_FltInsert(STMD_HOUR, STM_STATISTIC_HOUR_FAULT_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "FAULT HOUR Insert error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else 
        {
            sprintf(trcBuf,"stmd_exeUpdateHourFlt Update/Insert Fail = %d\n",ret_select);
            trclib_writeLogErr (FL,trcBuf);
            mysql_free_result(result1);
            return -1;
        }
    }
    mysql_free_result(result1);

#if 0 
    sprintf(query, "SELECT * FROM %s where (stat_date = '%s')",
                STM_STATISTIC_5MINUTE_BSD_FLT_TBL_NAME, insert_time);
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    result1 = mysql_store_result(conn);
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
        sprintf(query, "SELECT * FROM %s where (system_name = '%s' and stat_date = '%s')",
                    STM_STATISTIC_HOUR_BSD_FLT_TBL_NAME, row1[1], get_select_time(STMD_HOUR));
        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
            if ( stmd_bsdFltUpdate(STMD_HOUR, STM_STATISTIC_HOUR_BSD_FLT_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "BSD FAULT HOUR Update error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_bsdFltInsert(STMD_HOUR, STM_STATISTIC_HOUR_BSD_FLT_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "BSD FAULT HOUR Insert error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else 
        {
            sprintf(trcBuf,"stmd_exeUpdateHourBsdFlt Update/Insert Fail = %d\n",ret_select);
            trclib_writeLogErr (FL,trcBuf);
            mysql_free_result(result1);
            return -1;
        }
    }
    mysql_free_result(result1);
#endif 
    return 1;
    
}

int stmd_exeUpdateDayFlt()
{
    char        query[4096],insert_time[32];
    int         ret_select;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

    // 하루 전의 1시간 데이타를 지운다.
#if 0
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_DAY_FAULT_TBL_NAME, get_delete_time(STMD_1DAY_WEEK_OFFSET));
#endif
	/*
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_DAY_FAULT_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*3));
		*/
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_DAY_FAULT_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_DAY]));
    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "FltStatistic 1 Day mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
		return -1;
    }

    // 하루 전의 1시간 데이타를 지운다.
#if 0
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_DAY_BSD_FLT_TBL_NAME, get_delete_time(STMD_1DAY_WEEK_OFFSET));
#endif

#if 0 // jjinri
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_DAY_BSD_FLT_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*3));
    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "BsdFltStatistic 1 Day mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
    }
#endif
    
    /*현재 시간보다 5분전 시간을 구한다음
    5분 table에서 정보를 select 하고
    row 만큼 loop -> upate / insert */
    strcpy(insert_time, get_insert_time());
    if(strcasecmp(insert_time,"") == 0)
    {
        sprintf(trcBuf, "FAULT Statistic(Day) preview 5minute time get fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
    sprintf(query, "SELECT * FROM %s where (stat_date = '%s')",
                STM_STATISTIC_5MINUTE_FAULT_TBL_NAME, insert_time);
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }

    result1 = mysql_store_result(conn);
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
        sprintf(query, "SELECT * FROM %s where (system_name = '%s' and stat_date = '%s')",
                    STM_STATISTIC_DAY_FAULT_TBL_NAME, row1[1], get_select_time(STMD_DAY));
        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
            if ( stmd_FltUpdate(STMD_DAY, STM_STATISTIC_DAY_FAULT_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "FAULT DAY Update error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_FltInsert(STMD_DAY, STM_STATISTIC_DAY_FAULT_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "FAULT DAY Insert error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else 
        {
            sprintf(trcBuf,"stmd_exeUpdateDayFlt Update/Insert Fail = %d\n",ret_select);
            trclib_writeLogErr (FL,trcBuf);
            mysql_free_result(result1);
            return -1;
        }
    }
    mysql_free_result(result1);
    
#if 0
    sprintf(query, "SELECT * FROM %s where (stat_date = '%s')",
                STM_STATISTIC_5MINUTE_BSD_FLT_TBL_NAME, insert_time);
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    result1 = mysql_store_result(conn);
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
        sprintf(query, "SELECT * FROM %s where (system_name = '%s' and stat_date = '%s')",
                    STM_STATISTIC_DAY_BSD_FLT_TBL_NAME, row1[1], get_select_time(STMD_DAY));
        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
            if ( stmd_bsdFltUpdate(STMD_DAY, STM_STATISTIC_DAY_BSD_FLT_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "BSD FAULT DAY Update error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_bsdFltInsert(STMD_DAY, STM_STATISTIC_DAY_BSD_FLT_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "BSD FAULT DAY Insert error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else 
        {
            sprintf(trcBuf,"stmd_exeUpdateDayBsdFlt Update/Insert Fail = %d\n",ret_select);
            trclib_writeLogErr (FL,trcBuf);
            mysql_free_result(result1);
            return -1;
        }
    }
    mysql_free_result(result1);
#endif
    
    return 1;
    
}


int stmd_exeUpdateWeekFlt()
{
    char        query[4096],insert_time[32];
    int         ret_select;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

    // 하루 전의 1시간 데이타를 지운다.
#if 0
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_WEEK_FAULT_TBL_NAME, get_delete_time(STMD_1WEEK_WEEK_OFFSET));
#endif
	/*
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_WEEK_FAULT_TBL_NAME, get_delete_time(STMD_1WEEK_OFFSET*3));
		*/
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_WEEK_FAULT_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_WEEK]));
    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "FltStatistic 1 Week mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
		return -1;
    }

    // 하루 전의 1시간 데이타를 지운다.
#if 0
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_WEEK_BSD_FLT_TBL_NAME, get_delete_time(STMD_1WEEK_WEEK_OFFSET));
#endif

#if 0 // jjinri
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_WEEK_BSD_FLT_TBL_NAME, get_delete_time(STMD_1WEEK_OFFSET*3));
    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "BsdFltStatistic 1 Week mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
    }
#endif
    
    /*현재 시간보다 5분전 시간을 구한다음
    5분 table에서 정보를 select 하고
    row 만큼 loop -> upate / insert */
    strcpy(insert_time, get_insert_time());
    if(strcasecmp(insert_time,"") == 0)
    {
        sprintf(trcBuf, "FAULT Statistic(Week) preview 5minute time get fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
    sprintf(query, "SELECT * FROM %s where (stat_date = '%s')",
                STM_STATISTIC_5MINUTE_FAULT_TBL_NAME, insert_time);
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }

    result1 = mysql_store_result(conn);
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
        sprintf(query, "SELECT * FROM %s where (system_name = '%s' and stat_date = '%s')",
                    STM_STATISTIC_WEEK_FAULT_TBL_NAME, row1[1], get_select_time(STMD_WEEK));
        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
            if ( stmd_FltUpdate(STMD_WEEK, STM_STATISTIC_WEEK_FAULT_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "FAULT WEEK Update error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_FltInsert(STMD_WEEK, STM_STATISTIC_WEEK_FAULT_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "FAULT WEEK Insert error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else 
        {
            sprintf(trcBuf,"stmd_exeUpdateWeekFlt Update/Insert Fail = %d\n",ret_select);
            trclib_writeLogErr (FL,trcBuf);
            mysql_free_result(result1);
            return -1;
        }
    }
    mysql_free_result(result1);
    
#if 0
    sprintf(query, "SELECT * FROM %s where (stat_date = '%s')",
                STM_STATISTIC_5MINUTE_BSD_FLT_TBL_NAME, insert_time);
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    result1 = mysql_store_result(conn);
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
        sprintf(query, "SELECT * FROM %s where (system_name = '%s' and stat_date = '%s')",
                    STM_STATISTIC_WEEK_BSD_FLT_TBL_NAME, row1[1], get_select_time(STMD_WEEK));
        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
            if ( stmd_bsdFltUpdate(STMD_WEEK, STM_STATISTIC_WEEK_BSD_FLT_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "BSD FAULT WEEK Update error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_bsdFltInsert(STMD_WEEK, STM_STATISTIC_WEEK_BSD_FLT_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "BSD FAULT WEEK Insert error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else 
        {
            sprintf(trcBuf,"stmd_exeUpdateWeekBsdFlt Update/Insert Fail = %d\n",ret_select);
            trclib_writeLogErr (FL,trcBuf);
            mysql_free_result(result1);
            return -1;
        }
    }
    mysql_free_result(result1);
#endif
    
    return 1;
    
}

int stmd_exeUpdateMonthFlt()
{
    char        query[4096],insert_time[32];
    int         ret_select;
    MYSQL_RES   *result1;
    MYSQL_ROW   row1;

    // 하루 전의 1시간 데이타를 지운다.
#if 0
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_MONTH_FAULT_TBL_NAME, get_delete_time(STMD_1MON_MONTH_OFFSET));
#endif
	/*
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_MONTH_FAULT_TBL_NAME, get_delete_time(STMD_1MON_OFFSET*3));
		*/
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_MONTH_FAULT_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_MONTH]));
    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "FltStatistic 1 Month mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
		return -1;
    }

    // 하루 전의 1시간 데이타를 지운다.
#if 0
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_MONTH_BSD_FLT_TBL_NAME, get_delete_time(STMD_1MON_MONTH_OFFSET));
#endif

#if 0 // jjinri
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_MONTH_BSD_FLT_TBL_NAME, get_delete_time(STMD_1MON_OFFSET*3));
    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "BsdFltStatistic 1 Month mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
    }
#endif
    
    /*현재 시간보다 5분전 시간을 구한다음
    5분 table에서 정보를 select 하고
    row 만큼 loop -> upate / insert */
    strcpy(insert_time, get_insert_time());
    if(strcasecmp(insert_time,"") == 0)
    {
        sprintf(trcBuf, "FAULT Statistic(Month) preview 5minute time get fail\n");
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
    sprintf(query, "SELECT * FROM %s where (stat_date = '%s')",
                STM_STATISTIC_5MINUTE_FAULT_TBL_NAME, insert_time);
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }

    result1 = mysql_store_result(conn);
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
        sprintf(query, "SELECT * FROM %s where (system_name = '%s' and stat_date = '%s')",
                    STM_STATISTIC_MONTH_FAULT_TBL_NAME, row1[1], get_select_time(STMD_MONTH));
        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        {
            if ( stmd_FltUpdate(STMD_MONTH, STM_STATISTIC_MONTH_FAULT_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "FAULT MONTH Update error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_FltInsert(STMD_MONTH, STM_STATISTIC_MONTH_FAULT_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "FAULT MONTH Insert error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else 
        {
            sprintf(trcBuf,"stmd_exeUpdateMonthFlt Update/Insert Fail = %d\n",ret_select);
            trclib_writeLogErr (FL,trcBuf);
            mysql_free_result(result1);
            return -1;
        }
    }
    mysql_free_result(result1);
    
#if 0
    sprintf(query, "SELECT * FROM %s where (stat_date = '%s')",
                STM_STATISTIC_5MINUTE_BSD_FLT_TBL_NAME, insert_time);
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    result1 = mysql_store_result(conn);
    while( (row1 = mysql_fetch_row(result1)) != NULL ) 
    {
        sprintf(query, "SELECT * FROM %s where (system_name = '%s' and stat_date = '%s')",
                    STM_STATISTIC_MONTH_BSD_FLT_TBL_NAME, row1[1], get_select_time(STMD_MONTH));
        ret_select = stmd_mysql_select_query(query);
        if ( ret_select == 1 )  // update
        { 
            if ( stmd_bsdFltUpdate(STMD_MONTH, STM_STATISTIC_MONTH_BSD_FLT_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "BSD FAULT MONTH Update error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else if ( ret_select == 0 ) // insert 
        { 
            if ( stmd_bsdFltInsert(STMD_MONTH, STM_STATISTIC_MONTH_BSD_FLT_TBL_NAME, row1) < 0 ) 
            {
                sprintf(trcBuf, "BSD FAULT MONTH Insert error\n");
                trclib_writeLogErr (FL,trcBuf);
                continue;
            }
        } 
        else 
        {
            sprintf(trcBuf,"stmd_exeUpdateMonthBsdFlt Update/Insert Fail = %d\n",ret_select);
            trclib_writeLogErr (FL,trcBuf);
            mysql_free_result(result1);
            return -1;
        }
    }
    mysql_free_result(result1);
#endif
    
    return 1;
    
}
