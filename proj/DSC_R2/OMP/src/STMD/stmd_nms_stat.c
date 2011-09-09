#include "stmd_proto.h"

extern  char    trcBuf[4096], trcTmp[1024];
extern  int     trcFlag, trcLogFlag, trcLogId;
extern  short   maskITEM  [STMD_MASK_ITEM_NUM][STMD_PERIOD_TYPE_NUM];
extern  MYSQL   sql, *conn;
extern  char    strInf_Sys[5][10];
extern  int     sysCnt;
extern  STM_CommStatisticInfo    StatisticSystemInfo[SYSCONF_MAX_ASSO_SYS_NUM];
extern  int    max_sts_count;
extern  short   printTIME [STMD_PERIOD_TYPE_NUM];

int stmd_nms_LoadSelect(int time_type, char *table_type, char *nms_time, char *nms_end_time)
{
    char        condBuf[4096], tmpBuf[1024];
    char        avr_mem[10], max_mem[10], avr_cpu[10], max_cpu[10];
    int         sts_code;
    char        str_time[10];
    int         select_cnt = 0;

    char        query[4096];
    MYSQL_RES   *result;
    MYSQL_ROW   row;

    sts_code = STSCODE_STM_PERIODIC_LOAD_HOUR;
    strcpy(str_time, STMD_STR_HOUR);

    sprintf(condBuf,"    %s %s\n    S%04d LOAD %s PERIODIC STATISTICS MESSAGE\n",
        "OMP", commlib_printTStamp(), sts_code, str_time);

    sprintf(tmpBuf, "      PERIOD = %s -- %s\n\n", nms_time, nms_end_time);
    strcat(condBuf,tmpBuf);

    sprintf(query, "SELECT * FROM %s where (stat_date = '%s')", table_type, nms_time);

    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf(trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }

    sprintf(tmpBuf, "      SYSTEM    AVR_MEM(%%)  MAX_MEM(%%)  AVR_CPU(%%)  MAX_CPU(%%)\n");
    strcat(condBuf, tmpBuf);
    sprintf(tmpBuf, "      ------------------------------------------------------\n");
    strcat(condBuf, tmpBuf);
    result = mysql_store_result(conn);
    while((row = mysql_fetch_row(result)) != NULL) {
        strcpy(avr_mem, row[3]);
        sprintf(avr_mem, "%d.%d", atoi(avr_mem)/10, atoi(avr_mem)%10); 
        strcpy(max_mem, row[4]);
        sprintf(max_mem, "%d.%d", atoi(max_mem)/10, atoi(max_mem)%10); 
        strcpy(avr_cpu, row[5]);
        sprintf(avr_cpu, "%d.%d", atoi(avr_cpu)/10, atoi(avr_cpu)%10); 
        strcpy(max_cpu, row[6]);
        sprintf(max_cpu, "%d.%d", atoi(max_cpu)/10, atoi(max_cpu)%10);

        sprintf(tmpBuf, "      %-6s  %10s  %10s  %10s  %10s\n",
            row[0], avr_mem, max_mem, avr_cpu, max_cpu);
        strcat(condBuf, tmpBuf);
        select_cnt++;
    }
    mysql_free_result(result);

    if (select_cnt == 0) {
        sprintf(tmpBuf, "      %-6s  %10s  %10s  %10s  %10s\n",
            SYSCONF_SYSTYPE_MPA, "0", "0", "0", "0");
        strcat(condBuf, tmpBuf);
        sprintf(tmpBuf, "      %-6s  %10s  %10s  %10s  %10s\n",
            SYSCONF_SYSTYPE_MPB, "0", "0", "0", "0");
        strcat(condBuf, tmpBuf);
    }

    sprintf(tmpBuf, "    COMPLETED\n\n\n");
    strcat(condBuf, tmpBuf);

    stmd_txMsg2Nmsib(condBuf, (sts_code - STSCODE_TO_MSGID_STATISTICS), 1, 1);

    return 1;/* cjs */
}

int stmd_nms_FaultSelect(int time_type, char *table_type, char *flt_table_type, char *nms_time, char *nms_end_time)
{
    char        condBuf[4096], tmpBuf[1024];
    int         sts_code, i;
    char        str_time[10];
    int         select_cnt = 0;
    char                sysTypeName[16];

    char        query[4096];
    MYSQL_RES   *result;
    MYSQL_ROW   row;

    sts_code = STSCODE_STM_PERIODIC_FAULT_HOUR;
    strcpy(str_time, STMD_STR_HOUR);

    sprintf(condBuf,"    %s %s\n    S%04d FAULT %s PERIODIC STATISTICS MESSAGE\n",
        "OMP", commlib_printTStamp(), sts_code, str_time);

    sprintf(tmpBuf, "      PERIOD = %s -- %s\n\n", nms_time, nms_end_time);
    strcat(condBuf,tmpBuf);

    sprintf(tmpBuf, "      TYPE    SYSTEM      ITEM       MINOR  MAJOR  CRITICAL\n");
    strcat(condBuf, tmpBuf);
    sprintf(tmpBuf, "      =====================================================\n");
    strcat(condBuf, tmpBuf);

    for (i = 0; i < sysCnt; i++) {
        sprintf(query, "SELECT * FROM %s where (stat_date = '%s') AND system_name = '%s' ",
            table_type, nms_time, StatisticSystemInfo[i].sysName);

        select_cnt = 0;
        if ( trcLogFlag == TRCLEVEL_SQL ) {
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }

		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);
            return -1;
        }

        result = mysql_store_result(conn);
        while((row = mysql_fetch_row(result)) != NULL) {                    
            sprintf(tmpBuf, "      %-6s  %-10s  CPU        %5s  %5s  %8s\n", row[0], row[1], row[2], row[3], row[4]);
            strcat(condBuf, tmpBuf);
            sprintf(tmpBuf, "      %-6s  %-10s  MEM        %5s  %5s  %8s\n", row[0], row[1], row[5], row[6], row[7]);
            strcat(condBuf, tmpBuf);
            sprintf(tmpBuf, "      %-6s  %-10s  DISK       %5s  %5s  %8s\n", row[0], row[1], row[8], row[9], row[10]);
            strcat(condBuf, tmpBuf);
            sprintf(tmpBuf, "      %-6s  %-10s  LAN        %5s  %5s  %8s\n", row[0], row[1], row[11], row[12], row[13]);
            strcat(condBuf, tmpBuf);
            sprintf(tmpBuf, "      %-6s  %-10s  PROC       %5s  %5s  %8s\n", row[0], row[1], row[14], row[15], row[16]);
            strcat(condBuf, tmpBuf);
            select_cnt++;
        }
        mysql_free_result(result);

        if (select_cnt == 0) {
            if(strcasecmp(StatisticSystemInfo[i].sysType, SYSCONF_SYSTYPE_OMP))
                    strcpy(sysTypeName, "MP");
            else
                    strcpy(sysTypeName, StatisticSystemInfo[i].sysType);
                    
            sprintf(tmpBuf, "      %-6s  %-10s  CPU        %5s  %5s  %8s\n", 
                sysTypeName, StatisticSystemInfo[i].sysName, "0", "0", "0");
            strcat(condBuf, tmpBuf);
            sprintf(tmpBuf, "      %-6s  %-10s  MEM        %5s  %5s  %8s\n", 
                sysTypeName, StatisticSystemInfo[i].sysName, "0", "0", "0");
            strcat(condBuf, tmpBuf);
            sprintf(tmpBuf, "      %-6s  %-10s  DISK       %5s  %5s  %8s\n", 
                sysTypeName, StatisticSystemInfo[i].sysName, "0", "0", "0");
            strcat(condBuf, tmpBuf);
            sprintf(tmpBuf, "      %-6s  %-10s  LAN        %5s  %5s  %8s\n", 
                sysTypeName, StatisticSystemInfo[i].sysName, "0", "0", "0");
            strcat(condBuf, tmpBuf);
            sprintf(tmpBuf, "      %-6s  %-10s  PROC       %5s  %5s  %8s\n", 
                sysTypeName, StatisticSystemInfo[i].sysName, "0", "0", "0");
            strcat(condBuf, tmpBuf);

        }
    }

    select_cnt=0;
    for (i = 0; i < sysCnt; i++) {
        //jean if (strcasecmp(StatisticSystemInfo[i].sysType, SYSCONF_SYSTYPE_SMS))
        if (strcasecmp(StatisticSystemInfo[i].sysType, SYSCONF_SYSTYPE_BSD))
            continue;

        strcpy(sysTypeName, "MP");
        
        sprintf(query, "SELECT * FROM %s where (stat_date = '%s') AND system_name = '%s' ",
            flt_table_type, nms_time, StatisticSystemInfo[i].sysName);
        if ( trcLogFlag == TRCLEVEL_SQL ) {
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }

		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);
            return -1;
        }

        result = mysql_store_result(conn);
        while((row = mysql_fetch_row(result)) != NULL) {
            sprintf(tmpBuf, "      %-6s  %-10s  H/W        %5s  %5s  %8s\n", sysTypeName, row[1], row[2], row[3], row[4]);
            strcat(condBuf, tmpBuf);
            select_cnt++;
        }
        mysql_free_result(result);

        if (select_cnt == 0) {
            sprintf(tmpBuf, "      %-6s  %-10s  %-9s  %5s  %5s  %8s\n", 
                sysTypeName, StatisticSystemInfo[i].sysName, "H/W", "0", "0", "0");
            strcat(condBuf, tmpBuf);
        }
    }

    sprintf(tmpBuf, "    COMPLETED\n\n\n");
    strcat(condBuf, tmpBuf);

    stmd_txMsg2Nmsib(condBuf, (sts_code - STSCODE_TO_MSGID_STATISTICS), 1, 2);

    return 1;
}


