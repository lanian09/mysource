#include "stmd_proto.h"

extern  OnDemandList    onDEMAND[MAX_ONDEMAND_NUM];
extern  char            trcBuf[4096], trcTmp[1024];
extern  int             trcFlag, trcLogFlag, trcLogId;
extern  int     		sysCnt;
extern  MYSQL       	sql, *conn;
extern  int    			max_sts_count;
char    ondemandBuf[4096*8], ondemandTmp[4096], ondemandHead[1024], ondemandInit[1024], ondemandErr[1024];
char	tmp[5][512];
//int		ruleSet[5];
extern  STM_CommStatisticInfo   StatisticSystemInfo[SYSCONF_MAX_ASSO_SYS_NUM];
extern SCE_t	g_stSCE[2];
extern int			SCE_CNT;

extern char	rsFname[32];
extern RuleSetList g_stSCERule[MAX_SCE_NUM];
extern int g_ruleIdBuf[MAX_RULE_NUM];
extern int g_ruleItemCnt;

extern RuleEntryList g_stSCEEntry[MAX_SCE_NUM];
extern int g_ruleEntryBuf[MAX_ENTRY_NUM];
extern char g_ruleEntryName[MAX_ENTRY_NUM];
extern int g_ruleEntryCnt;


extern PDSN_LIST         g_stPdsn;
extern SMSC_LIST	     g_stSmsc[2];

int doOnDemandJob() {

    int i;

    for ( i = 0; i < MAX_ONDEMAND_NUM ; i++) {
        if ( (onDEMAND[i].statisticsType != NOT_REGISTERED) && 
            (!strcasecmp(onDEMAND[i].measureTime, get_insert_time3())) ) { // 분까지 select (now-300)/300*300

            switch ( onDEMAND[i].statisticsType) {
                case  STMD_FAULT:
                    sprintf(trcBuf, "FLT DEMAND START: %ld\n", time(0));
                    trclib_writeLogErr(FL, trcBuf);
                    doOnDemandFault(i);
                    sprintf(trcBuf, "FLT DEMAND END: %ld\n", time(0));
                    trclib_writeLogErr(FL, trcBuf);
                    break;

                case  STMD_LOAD:
                    sprintf(trcBuf, "LOAD DEMAND START: %ld\n", time(0));
                    trclib_writeLogErr(FL, trcBuf);
                    doOnDemandLoad(i);
                    sprintf(trcBuf, "LOAD DEMAND END: %ld\n", time(0));
                    trclib_writeLogErr(FL, trcBuf);
                    break;

                case  STMD_LINK:
                    sprintf(trcBuf, "LINK DEMAND START: %ld\n", time(0));
                    trclib_writeLogErr(FL, trcBuf);
                    doOnDemandLink(i);
                    sprintf(trcBuf, "LINK DEMAND END: %ld\n", time(0));
                    trclib_writeLogErr(FL, trcBuf);
                    break;

                case  STMD_LEG:
                    sprintf(trcBuf, "LEG DEMAND START: %ld\n", time(0));
                    trclib_writeLogErr(FL, trcBuf);
                    doOnDemandLeg(i);
                    sprintf(trcBuf, "LEG DEMAND END: %ld\n", time(0));
                    trclib_writeLogErr(FL, trcBuf);
                    break;

                case  STMD_LOGON:
                    sprintf(trcBuf, "LOGON DEMAND START: %ld\n", time(0));
                    trclib_writeLogErr(FL, trcBuf);
                    doOnDemandLogon(i);
                    sprintf(trcBuf, "LOGON DEMAND END: %ld\n", time(0));
                    trclib_writeLogErr(FL, trcBuf);
                    break;

                case  STMD_FLOW:
                    sprintf(trcBuf, "FLOW DEMAND START: %ld\n", time(0));
                    trclib_writeLogErr(FL, trcBuf);
                    doOnDemandFlow(i);
                    sprintf(trcBuf, "FLOW DEMAND END: %ld\n", time(0));
                    trclib_writeLogErr(FL, trcBuf);
                    break;

                case  STMD_RULE_SET:
                    sprintf(trcBuf, "RULESET DEMAND start: %ld\n", time(0));
                    trclib_writeLogErr(FL, trcBuf);
                    doOnDemandRuleSet(i);
                    sprintf(trcBuf, "RULESET DEMAND END: %ld\n", time(0));
                    trclib_writeLogErr(FL, trcBuf);
                    break;

                case  STMD_RULE_ENT:
                    sprintf(trcBuf, "ruleent DEMAND start: %ld\n", time(0));
                    trclib_writeLogErr(FL, trcBuf);
                    doOnDemandRuleEnt(i);
                    sprintf(trcBuf, "ruleent DEMAND end: %ld\n", time(0));
                    trclib_writeLogErr(FL, trcBuf);
                    break;

                case  STMD_SMS:
                    sprintf(trcBuf, "sms DEMAND start: %ld\n", time(0));
                    trclib_writeLogErr(FL, trcBuf);
                    doOnDemandSms(i);
                    sprintf(trcBuf, "sms DEMAND END: %ld\n", time(0));
                    trclib_writeLogErr(FL, trcBuf);
                    break;

#ifdef DELAY
                case  STMD_DEL:
                    sprintf(trcBuf, "del DEMAND2 start: %ld\n", time(0));
                    trclib_writeLogErr(FL, trcBuf);
                    doOnDemandDelay2(i);
                    sprintf(trcBuf, "del DEMAND2 END: %ld\n", time(0));
                    trclib_writeLogErr(FL, trcBuf);
                    break;
#else
                case  STMD_DEL:
                    sprintf(trcBuf, "del DEMAND start: %ld\n", time(0));
                    trclib_writeLogErr(FL, trcBuf);
                    doOnDemandDelay(i);
                    sprintf(trcBuf, "del DEMAND END: %ld\n", time(0));
                    trclib_writeLogErr(FL, trcBuf);
                    break;
#endif

                default:
                    sprintf(trcBuf, "unexpected onDEMAND statisticsType = %d\n", onDEMAND[i].statisticsType);
                    trclib_writeLogErr(FL, trcBuf);
                    break;
            }
        }
    }
	return 0;
}

int doOnDemandLink(int list)
{
    char        query[4096] = {0,};
    char        cmdName[64] = {0,};
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int         i,j;
	char title[5][16] = {"SYSTEM", "ITEM", "UpThruput(Mbps)", "DnThruput(Mbps)", "Total(Mbps)"};
	char title1[5][16] = {"", "", "UpBytes(MBytes)", "DnBytes(MBytes)", "Total(MBytes)"};
    int row_index;
    char SysName[2][8];
    int realSysCnt =0;
    int realItemCnt =0, row_cnt = 0, index;
	char linkName[2][10] = {"Link 1", "Link 2"};


    for(i=0; i<SCE_CNT; i++ ){
        sprintf(SysName[i], "%s", g_stSCE[i].sce_name);
        realSysCnt++;
    }

    memset (ondemandBuf, 0x00, sizeof(ondemandBuf));
    memset (ondemandErr, 0x00, sizeof(ondemandErr));

    onDEMAND[list].Txcount = onDEMAND[list].Txcount + 1;

    sprintf(ondemandBuf, "    PERIOD = %d\n    MEASURETIME = %s(>)  -  (<=)%s CNT = %d\n",
        onDEMAND[list].period, 
        get_ondemand_time(0 - ((onDEMAND[list].period-STAT_UNIT) * 60)),
        (char *)get_current_time(),
        onDEMAND[list].Txcount);
    sprintf(ondemandErr, "    PERIOD = %d\n    MEASURETIME = (>)%s  -  (<=)%s CNT = %d\n",
        onDEMAND[list].period, 
        get_ondemand_time(0 - ((onDEMAND[list].period-STAT_UNIT) * 60)),
        (char *)get_current_time(),
        onDEMAND[list].Txcount);
    sprintf(cmdName, "%s", "stat-link");

    sprintf(ondemandTmp, "    =======================================================================================\n");
    strcat(ondemandBuf, ondemandTmp);
    sprintf(ondemandTmp, "%2s %4s %-10s %-12s %-12s %-12s\n","",title[0],title[1],title[2],title[3],title[4]);
    strcat(ondemandBuf, ondemandTmp);
    sprintf(ondemandTmp, "%2s %4s %-10s %-12s %-12s %-12s\n","",title1[0],title1[1],title1[2],title1[3],title1[4]);
    strcat(ondemandBuf, ondemandTmp);
    sprintf(ondemandTmp, "    =======================================================================================\n");
    strcat(ondemandBuf, ondemandTmp);

	realItemCnt = 2;

    for ( i=0 ; i < realSysCnt; i++) 
    {
        if ( strcasecmp(onDEMAND[list].svcName, "ALL") ) {
            if(strcasecmp(onDEMAND[list].svcName, SysName[i]))
                continue;
        }
                    
	//char title[5][16] = {"SYSTEM", "ITEM", "UpThruput(Mbps)", "DnThruput(Mbps)", "Total(Mbps)"};
	//char title1[5][16] = {"", "", "UpBytes(MBytes)", "DnBytes(MBytes)", "Total(MBytes)"};
        sprintf(query, "SELECT link_id, "
            " IFNULL(ROUND(SUM(upstream_volume)*8/1024/(%d*60), 3),0), "
			" IFNULL(ROUND(SUM(downstream_volume)*8/1024/(%d*60), 3),0), "
			" ROUND((IFNULL(SUM(upstream_volume),0) + IFNULL(SUM(downstream_volume),0))*8/1024/(%d*60), 3), "
            " IFNULL(ROUND(SUM(upstream_volume)/1024, 3),0), "
			" IFNULL(ROUND(SUM(downstream_volume)/1024, 3),0), "
			" ROUND((IFNULL(SUM(upstream_volume),0) + IFNULL(SUM(downstream_volume),0))/1024, 3) "
            " from %s "
            " where record_source = '%s' AND stat_date > '%s' AND stat_date <= '%s' "
			" group by record_source, link_id order by record_source desc, stat_date, link_id",
			onDEMAND[list].period, onDEMAND[list].period, onDEMAND[list].period, 
			STM_STATISTIC_5MINUTE_LUR_TBL_NAME, SysName[i],
			get_ondemand_time(0 - ((onDEMAND[list].period-STAT_UNIT) * 60)), 
			get_current_time() );

        if ( trcLogFlag == TRCLEVEL_SQL ) {
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }

		logPrint(trcLogId,FL,"ondemand query: %s\n", query);

		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);

            onDEMAND[list].statisticsType = NOT_REGISTERED;
            sprintf(ondemandTmp, "    RESULT = FAIL\n    FAIL REASON = DB SELECT FAIL\n");
            strcat(ondemandErr, ondemandTmp);
            stmd_ondemand_txMMLResult (list, ondemandErr, -1, 0, 0, cmdName, 0, 1);
            return -1;
        }

		row_cnt = 0;
        result = mysql_store_result(conn);
        while( (row = mysql_fetch_row(result)) != NULL) 
		{
            row_index = 1;
			index = atoi(row[0]);

			if( row_cnt == 0 )
			{
				sprintf(ondemandTmp, "%2s %4s %-10s %-12s %-12s %-12s\n",
						"", SysName[i], linkName[index-1], row[row_index], row[row_index+1], row[row_index +2]);
				strcat(ondemandBuf, ondemandTmp);

				sprintf(ondemandTmp, "%2s %4s %-10s %-12s %-12s %-12s\n",
						"", "", "", row[row_index+3], row[row_index+4], row[row_index+5]);
				strcat(ondemandBuf, ondemandTmp);
			} 
			else 
			{
				sprintf(ondemandTmp, "%2s %4s %-10s %-12s %-12s %-12s\n",
						"", "", linkName[index-1], row[row_index], row[row_index+1], row[row_index+2]);
				strcat(ondemandBuf, ondemandTmp);
				sprintf(ondemandTmp, "%2s %4s %-10s %-12s %-12s %-12s\n",
						"", "", "", row[row_index+3], row[row_index+4], row[row_index+5]);
				strcat(ondemandBuf, ondemandTmp);
			}
			row_cnt++;
        }

        mysql_free_result(result);
		if( row_cnt == 0 )
		{
			for( j = 0 ; j < realItemCnt; j++ )
			{
				sprintf(ondemandTmp, "%2s %4s %-10s %-12s %-12s %-12s\n",
						"", SysName[i], linkName[j], "0.000", "0.000", "0.000");
				strcat(ondemandBuf, ondemandTmp);
				sprintf(ondemandTmp, "%2s %4s %-10s %-12s %-12s %-12s\n",
						"", "", "", "0.000", "0.000", "0.000");
				strcat(ondemandBuf, ondemandTmp);
			} 
			//		sprintf(ondemandTmp, " NO DATA " );
		}

		sprintf(ondemandTmp, "    =======================================================================================\n");
		strcat(ondemandBuf, ondemandTmp);
    }


    if ( onDEMAND[list].count == -1) 
	{
        stmd_ondemand_txMMLResult (list, ondemandBuf, 0, 0, 0, cmdName, 0, 1);
        onDEMAND[list].statisticsType = NOT_REGISTERED;
    } 
	else if ( onDEMAND[list].Txcount == onDEMAND[list].count) 
	{
        stmd_ondemand_txMMLResult (list, ondemandBuf, 0, 0, 0, cmdName, 0, 1);
        onDEMAND[list].statisticsType = NOT_REGISTERED;
    } 
	else 
	{
        stmd_ondemand_txMMLResult (list, ondemandBuf, 0, 1, (onDEMAND[list].period*60)+10, cmdName, 0, 1);
        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(onDEMAND[list].period*60));
    }
	return 0;
}


int doOnDemandLogon(int list)
{
    char        query[4096];
    char        cmdName[64];
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int         i, j, row_index, row_cnt = 0, realSysCnt = 2;
	float       rate; //uamyd 20110515 succrate_added

	char title[7][16]={"SYSTEM", "SM_CH_ID", "LOG_MOD", "LOG_REQ", "LOG_SUCC","LOG_FAIL","SUCC_RATE"}; //uamyd 20110515 succrate_added
	char title1[5][16]={"", "HBIT_0","HBIT_1","HBIT_2", "HBIT_3"};
	char title2[5][16]={"", "HBIT_4","HBIT_5","HBIT_6", "HBIT_7"};
	char title3[5][16]={"", "HBIT_8","HBIT_9","HBIT_10", "HBIT_11"};
	char title4[5][16]={"", "HBIT_12","HBIT_13","HBIT_14", "HBIT_15"};
	char title5[5][16]={"", "HBIT_16","HBIT_17","HBIT_18", "HBIT_19"};
	char title6[5][16]={"", "HBIT_20","HBIT_21","HBIT_22", "HBIT_23"};
	char title7[5][16]={"", "HBIT_24","HBIT_25","HBIT_26", "HBIT_27"};
	char title8[5][16]={"", "HBIT_28","HBIT_29","HBIT_30", "HBIT_31"};
	char title10[5][16]={"", "SM_INT_ERR","OP_ERR","OP_TIMEOUT", "ETC_FAIL"};
	char title11[3][16]={"", "API_REQ_ERR","API_TIMEOUT"};
    char SysName[2][8] = {"SCMA", "SCMB"};

    memset (ondemandBuf, 0x00, sizeof(ondemandBuf));
    memset (ondemandErr, 0x00, sizeof(ondemandErr));

    onDEMAND[list].Txcount = onDEMAND[list].Txcount + 1;
    sprintf(ondemandBuf, "    PERIOD = %d(MIN)\n    MEASURETIME = %s(>)  - (>=)%s CNT = %d\n",
        onDEMAND[list].period, 
        get_ondemand_time(0 - ((onDEMAND[list].period-STAT_UNIT) * 60)),
        (char *)get_current_time(),
        onDEMAND[list].Txcount);
    sprintf(ondemandErr, "    PERIOD = %d(MIN)\n    MEASURETIME = %s(>)  -  (>=)%s CNT =%d\n",
        onDEMAND[list].period, 
        get_ondemand_time(0 - ((onDEMAND[list].period-STAT_UNIT) * 60)),
        (char *)get_current_time(),
        onDEMAND[list].Txcount);
    sprintf (cmdName, "%s", "stat-logon");

    sprintf(ondemandTmp, "    ====================================================================\n");
	strcat(ondemandBuf, ondemandTmp);
	sprintf(ondemandTmp, "%3s %6s %-12s %-12s\n", "",title[0], title[1], title[2]);
	strcat(ondemandBuf, ondemandTmp);
	sprintf(ondemandTmp, "%3s %6s %-12s %-12s %-12s %-12s\n", "","",title[3], title[4], title[5], title[6]); //uamyd 20110515 succrate_added
	strcat(ondemandBuf, ondemandTmp);
	sprintf(ondemandTmp, "%3s %6s %-12s %-12s %-12s %-12s\n", "","", title1[1], title1[2],title1[3],title1[4]);
	strcat(ondemandBuf, ondemandTmp);
	sprintf(ondemandTmp, "%3s %6s %-12s %-12s %-12s %-12s\n", "","", title2[1], title2[2],title2[3],title2[4]);
	strcat(ondemandBuf, ondemandTmp);
	sprintf(ondemandTmp, "%3s %6s %-12s %-12s %-12s %-12s\n", "","", title3[1], title3[2],title3[3],title3[4]);
	strcat(ondemandBuf, ondemandTmp);
	sprintf(ondemandTmp, "%3s %6s %-12s %-12s %-12s %-12s\n", "","", title4[1], title4[2],title4[3],title4[4]);
	strcat(ondemandBuf, ondemandTmp);
	sprintf(ondemandTmp, "%3s %6s %-12s %-12s %-12s %-12s\n", "","", title5[1], title5[2],title5[3],title5[4]);
	strcat(ondemandBuf, ondemandTmp);
	sprintf(ondemandTmp, "%3s %6s %-12s %-12s %-12s %-12s\n", "","", title6[1], title6[2],title6[3],title6[4]);
	strcat(ondemandBuf, ondemandTmp);
	sprintf(ondemandTmp, "%3s %6s %-12s %-12s %-12s %-12s\n", "","", title7[1], title7[2],title7[3],title7[4]);
	strcat(ondemandBuf, ondemandTmp);
	sprintf(ondemandTmp, "%3s %6s %-12s %-12s %-12s %-12s\n", "","", title8[1], title8[2],title8[3],title8[4]);
	strcat(ondemandBuf, ondemandTmp);
	sprintf(ondemandTmp, "%3s %6s %-12s %-12s %-12s %-12s\n", "","", title10[1], title10[2],title10[3],title10[4]);
	strcat(ondemandBuf, ondemandTmp);
	sprintf(ondemandTmp, "%3s %6s %-12s %-12s \n", "","", title11[1], title11[2]);
	strcat(ondemandBuf, ondemandTmp);
    sprintf(ondemandTmp, "    ====================================================================\n");
    strcat(ondemandBuf, ondemandTmp);

    for ( i=0 ; i < realSysCnt; i++) 
    {
        if ( strcasecmp(onDEMAND[list].svcName, "ALL") ) {
            if(strcasecmp(onDEMAND[list].svcName, SysName[i]))
                continue;
        }

		sprintf(query, "SELECT system_name, sm_ch_id, log_mod, "
				" ifnull(sum(log_req), 0), ifnull(sum(log_succ), 0), ifnull(sum(log_fail), 0), "
				" ifnull(sum(HBIT_0), 0), ifnull(sum(HBIT_1), 0), ifnull(sum(HBIT_2), 0), "
				" ifnull(sum(HBIT_3), 0),  "
				" ifnull(sum(HBIT_4), 0), ifnull(sum(HBIT_5), 0), ifnull(sum(HBIT_6), 0), "
				" ifnull(sum(HBIT_7), 0),  "
				" ifnull(sum(HBIT_8), 0), ifnull(sum(HBIT_9), 0), ifnull(sum(HBIT_10), 0), "
				" ifnull(sum(HBIT_11), 0),  "
				" ifnull(sum(HBIT_12), 0), ifnull(sum(HBIT_13), 0), ifnull(sum(HBIT_14), 0), "
				" ifnull(sum(HBIT_15), 0),  "
				" ifnull(sum(HBIT_16), 0), ifnull(sum(HBIT_17), 0), ifnull(sum(HBIT_18), 0), "
				" ifnull(sum(HBIT_19), 0),  "
				" ifnull(sum(HBIT_20), 0), ifnull(sum(HBIT_21), 0), ifnull(sum(HBIT_22), 0), "
				" ifnull(sum(HBIT_23), 0),  "
				" ifnull(sum(HBIT_24), 0), ifnull(sum(HBIT_25), 0), ifnull(sum(HBIT_26), 0), "
				" ifnull(sum(HBIT_27), 0),  "
				" ifnull(sum(HBIT_28), 0), ifnull(sum(HBIT_29), 0), ifnull(sum(HBIT_30), 0), "
				" ifnull(sum(HBIT_31), 0),  "
				" ifnull(sum(sm_int_err),0), ifnull(sum(op_err),0), ifnull(sum(op_timeout),0), "
				" ifnull(sum(etc_fail),0), "
				" ifnull(sum(api_req_err),0), ifnull(sum(api_timeout),0) "
				" from %s "
				" where system_name = '%s' AND stat_date > '%s' AND stat_date <= '%s' "
				" group by system_name, sm_ch_id, log_mod ",
            STM_STATISTIC_5MINUTE_LOGON_TBL_NAME, SysName[i],
            get_ondemand_time(0 - ((onDEMAND[list].period-STAT_UNIT) * 60)), (char *)get_current_time()
        );

        if ( trcLogFlag == TRCLEVEL_SQL ) 
        {
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }
		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail;query:%s, err=%s\n", query, mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);
            sprintf(ondemandTmp, "    RESULT = FAIL\n    FAIL REASON = DB SELECT FAIL\n");

            onDEMAND[list].statisticsType = NOT_REGISTERED;
            strcat(ondemandErr, ondemandTmp);
            stmd_ondemand_txMMLResult (list, ondemandErr, -1, 0, 0, cmdName, 0, 1);
            return -1;
        }

		row_cnt = 0;
		result = mysql_store_result(conn);
		while( (row = mysql_fetch_row(result)) != NULL)
		{                                                                                                         
			row_index = 1;
			if( row_cnt == 0 )
			{
				sprintf(ondemandTmp, "%3s %6s SM_CH%7s %12s\n",
                        "",SysName[i], row[row_index], !atoi(row[row_index+1])?"LOG_ON":"LOG_OUT");
                strcat(ondemandBuf, ondemandTmp);
			}
			else
			{
				sprintf(ondemandTmp, "%3s %6s SM_CH%7s %12s\n",
                        "","", row[row_index], !atoi(row[row_index+1])?"LOG_ON":"LOG_OUT");
                strcat(ondemandBuf, ondemandTmp);
			}

			rate = (((float)atoi(row[row_index+3])/(float)atoi(row[row_index+2]))*100);//uamyd 20110515 succrate_added
			/* common */
			sprintf(ondemandTmp, "%3s %6s %-12s %-12s %-12s %-12.1f\n",
					"", "", row[row_index+2],row[row_index+3],row[row_index+4],
					atoi(row[row_index+2])==0?0.0:rate); //uamyd 20110515 succrate_added
			strcat(ondemandBuf, ondemandTmp);

			for( j=5; j<34; j+=4 ){
				//j=5,9,13,17,21,25,29,33, HBIT0 ~ HBIT31
				sprintf(ondemandTmp, "%3s %6s %-12s %-12s %-12s %-12s\n",
						"", "", row[row_index+j],row[row_index+j+1],row[row_index+j+2],row[row_index+j+3]);
				strcat(ondemandBuf, ondemandTmp );
			}

			sprintf(ondemandTmp, "%3s %6s %-12s %-12s %-12s %-12s\n",
					"", "", row[row_index+37],row[row_index+38],row[row_index+39],row[row_index+40]);
			strcat(ondemandBuf, ondemandTmp);
			sprintf(ondemandTmp, "%3s %6s %-12s %-12s \n",
					"", "", row[row_index+41],row[row_index+42]);
			strcat(ondemandBuf, ondemandTmp);
			/*************************** common */

			row_cnt++;
		}

		mysql_free_result(result);
		if( row_cnt == 0 )
		{
			sprintf(ondemandTmp, "%3s %6s %-12s %-12s\n", "",SysName[i],"-","-");
			strcat(ondemandBuf, ondemandTmp);
			sprintf(ondemandTmp, "%3s %6s %-12s %-12s %-12s %-12s\n", "", "", "0","0","0","0"); //uamyd 20110515 succrate_added
			strcat(ondemandBuf, ondemandTmp);

			/* 그냥 9회 반복해서 msg make */
			for( j=0; j<9;j++){
				sprintf(ondemandTmp, "%3s %6s %-12s %-12s %-12s %-12s\n", "", "", "0","0","0","0");
				strcat(ondemandBuf, ondemandTmp );
			}

			sprintf(ondemandTmp, "%3s %6s %-12s %-12s\n", 
					"", "", "0","0");
			strcat(ondemandBuf, ondemandTmp);
		}
		sprintf(ondemandTmp, "    ====================================================================\n");
		strcat(ondemandBuf, ondemandTmp);                                                                         

	}    

    if ( onDEMAND[list].count == -1) 
    {
        stmd_ondemand_txMMLResult (list, ondemandBuf, 0, 0, 0, cmdName, 0, 1);
        onDEMAND[list].statisticsType = NOT_REGISTERED;
    } 
    else if ( onDEMAND[list].Txcount == onDEMAND[list].count) 
    {
        stmd_ondemand_txMMLResult (list, ondemandBuf, 0, 0, 0, cmdName, 0, 1);
        onDEMAND[list].statisticsType = NOT_REGISTERED;
    } 
    else 
    {
        stmd_ondemand_txMMLResult (list, ondemandBuf, 0, 1, (onDEMAND[list].period*60)+10, cmdName, 0, 1);
        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(onDEMAND[list].period*60));
    }

    return 1;
}

int doOnDemandFlow(int list)
{
    char        query[4096];
    char        cmdName[64];
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int         i;
	char title[4][16]={"SYSTEM", "AVG_FLOW", "MIN_FLOW","MAX_FLOW"};
    int row_index  = 0;
    char SysName[2][8] = {"SCEA", "SCEB"};
    int realSysCnt =2;
	int	row_cnt = 0;

    memset (ondemandBuf, 0x00, sizeof(ondemandBuf));
    memset (ondemandErr, 0x00, sizeof(ondemandErr));

    onDEMAND[list].Txcount = onDEMAND[list].Txcount + 1;
    sprintf(ondemandBuf, "    PERIOD = %d(MIN)\n    MEASURETIME = %s(>)  - (>=)%s CNT = %d\n",
        onDEMAND[list].period, 
        get_ondemand_time(0 - ((onDEMAND[list].period-STAT_UNIT) * 60)),
        (char *)get_current_time(),
        onDEMAND[list].Txcount);
    sprintf(ondemandErr, "    PERIOD = %d(MIN)\n    MEASURETIME = %s(>)  -  (>=)%s CNT =%d\n",
        onDEMAND[list].period, 
        get_ondemand_time(0 - ((onDEMAND[list].period-STAT_UNIT) * 60)),
        (char *)get_current_time(),
        onDEMAND[list].Txcount);
    sprintf (cmdName, "%s", "stat-flow");

    sprintf(ondemandTmp, "    ====================================================================\n");
	strcat(ondemandBuf, ondemandTmp);
	sprintf(ondemandTmp, "%3s %6s %-12s %-12s %-12s\n", "",title[0], title[1], title[2],title[3]);
	strcat(ondemandBuf, ondemandTmp);
    sprintf(ondemandTmp, "    ====================================================================\n");
    strcat(ondemandBuf, ondemandTmp);

    for ( i=0 ; i < realSysCnt; i++) 
    {
        if ( strcasecmp(onDEMAND[list].svcName, "ALL") ) {
            if(strcasecmp(onDEMAND[list].svcName, SysName[i]))
                continue;
        }

		sprintf(query, "SELECT system_name, "
				" ROUND(IFNULL(AVG(avg_flow),0),0), IFNULL(MIN(min_flow),0), IFNULL(MAX(max_flow),0) "
				" from %s "
				" where system_name = '%s' AND stat_date > '%s' AND stat_date <= '%s' "
				" group by system_name ",
            STM_STATISTIC_5MINUTE_FLOW_TBL_NAME, SysName[i],
            get_ondemand_time(0 - ((onDEMAND[list].period-STAT_UNIT) * 60)), (char *)get_current_time()
			);

		// dcham debugging 0506
		sprintf(trcBuf,"%s\n",query);
		trclib_writeLogErr (FL,trcBuf);

		if ( trcLogFlag == TRCLEVEL_SQL ) 
        {
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }
		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail;query:%s, err=%s\n", query, mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);
            sprintf(ondemandTmp, "    RESULT = FAIL\n    FAIL REASON = DB SELECT FAIL\n");

            onDEMAND[list].statisticsType = NOT_REGISTERED;
            strcat(ondemandErr, ondemandTmp);
            stmd_ondemand_txMMLResult (list, ondemandErr, -1, 0, 0, cmdName, 0, 1);
            return -1;
        }

		row_cnt = 0;
		result = mysql_store_result(conn);
		while( (row = mysql_fetch_row(result)) != NULL)
		{                                                                                                         
			row_index = 1;
			if( row_cnt == 0 )
			{
				sprintf(ondemandTmp, "%3s %6s %-12s %-12s %-12s\n",
						"",SysName[i],row[row_index],row[row_index+1],row[row_index+2]);
				strcat(ondemandBuf, ondemandTmp);
			}
			else
			{
				sprintf(ondemandTmp, "%3s %6s %-12s %-12s %-12s\n",
						"","",row[row_index],row[row_index+1],row[row_index+2]);
				strcat(ondemandBuf, ondemandTmp);
			}
			row_cnt++;
		}

		mysql_free_result(result);
		if( row_cnt == 0 )
		{
			sprintf(ondemandTmp, "%3s %6s %-12s %-12s %-12s\n",
					"",SysName[i],"0","0","0");
			strcat(ondemandBuf, ondemandTmp);
		}
		sprintf(ondemandTmp, "    ====================================================================\n");
		strcat(ondemandBuf, ondemandTmp);                                                                         

	}    

    if ( onDEMAND[list].count == -1) 
    {
        stmd_ondemand_txMMLResult (list, ondemandBuf, 0, 0, 0, cmdName, 0, 1);
        onDEMAND[list].statisticsType = NOT_REGISTERED;
    } 
    else if ( onDEMAND[list].Txcount == onDEMAND[list].count) 
    {
        stmd_ondemand_txMMLResult (list, ondemandBuf, 0, 0, 0, cmdName, 0, 1);
        onDEMAND[list].statisticsType = NOT_REGISTERED;
    } 
    else 
    {
        stmd_ondemand_txMMLResult (list, ondemandBuf, 0, 1, (onDEMAND[list].period*60)+10, cmdName, 0, 1);
        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(onDEMAND[list].period*60));
    }

    return 1;
}


#ifdef DELAY
int doOnDemandDelay2(int list)
{
    char        query[4096];
    char        cmdName[64];
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int         i;
	char title[4][16]={"SYSTEM", "MIN_MSEC", "MAX_MSEC", "AVG_MSEC"};
    int row_index;
    char SysName[2][8] = {"SCE", "SCEB"};
    int realSysCnt =1;
	int	row_cnt = 0;

    memset (ondemandBuf, 0x00, sizeof(ondemandBuf));
    memset (ondemandErr, 0x00, sizeof(ondemandErr));

    onDEMAND[list].Txcount = onDEMAND[list].Txcount + 1;
    sprintf(ondemandBuf, "    PERIOD = %d(HOUR)\n    MEASURETIME = %s:00:00\n",
        onDEMAND[list].period+1, 
        get_ondemand_delay());
    sprintf (cmdName, "%s", "stat-delay");

    sprintf(ondemandTmp, "    ===============================================================\n");
	strcat(ondemandBuf, ondemandTmp);
	sprintf(ondemandTmp, "%3s %8s %-12s %-12s %-12s\n","",title[0],title[1],title[2],title[3]);
	strcat(ondemandBuf, ondemandTmp);
    sprintf(ondemandTmp, "    ===============================================================\n");
    strcat(ondemandBuf, ondemandTmp);

    for ( i=0 ; i < realSysCnt; i++) 
    {
        if ( strcasecmp(onDEMAND[list].svcName, "ALL") ) {
            if(strcasecmp(onDEMAND[list].svcName, SysName[i]))
                continue;
        }

        sprintf(query, "SELECT system_name, "
				" ROUND(IFNULL(MIN(min_usec)*1000,0), 3), ROUND(IFNULL(MAX(max_usec)*1000,0), 3), "
				" ROUND(IFNULL(AVG(avg_usec)*1000,0), 3) "
				" from %s "
				" where system_name = '%s' AND stat_date = '%s' "
				" group by system_name ",
            STM_STATISTIC_HOUR_DELAY_TBL_NAME, SysName[i],
            get_ondemand_delay()
        );

        if ( trcLogFlag == TRCLEVEL_SQL ) 
        {
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }

		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);
            sprintf(ondemandTmp, "    RESULT = FAIL\n    FAIL REASON = DB SELECT FAIL\n");

            onDEMAND[list].statisticsType = NOT_REGISTERED;
            strcat(ondemandErr, ondemandTmp);
            stmd_ondemand_txMMLResult (list, ondemandErr, -1, 0, 0, cmdName, 0, 1);
            return -1;
        }

		row_cnt = 0;
		result = mysql_store_result(conn);
		while( (row = mysql_fetch_row(result)) != NULL)
		{                                                                                                         
			row_index = 1;
			if( row_cnt == 0 )
			{
				sprintf(ondemandTmp, "%3s %8s %12s %12s %12s\n",
						"", row[0], row[row_index],row[row_index+1],row[row_index+2]);
				strcat(ondemandBuf, ondemandTmp);
			}
			else
			{
				sprintf(ondemandTmp, "%3s %8s %12s %12s %12s\n",
						"", "", row[row_index],row[row_index+1],row[row_index+2]);
				strcat(ondemandBuf, ondemandTmp);
			}
			row_cnt++;
		}

		mysql_free_result(result);

		if( row_cnt == 0 )
		{
			sprintf(ondemandTmp, "%3s %8s %12s %12s %12s\n",
					"", SysName[0], "0.000","0.000","0.000");
			strcat(ondemandBuf, ondemandTmp);
	//		sprintf(ondemandTmp, "	NO DATA " );
		}

    	sprintf(ondemandTmp, "    ===============================================================\n");
		strcat(ondemandBuf, ondemandTmp);                                                                         
	}    

    if ( onDEMAND[list].count == -1) 
    {
        stmd_ondemand_txMMLResult (list, ondemandBuf, 0, 0, 0, cmdName, 0, 1);
        onDEMAND[list].statisticsType = NOT_REGISTERED;
    } 
    else if ( onDEMAND[list].Txcount == onDEMAND[list].count) 
    {
        stmd_ondemand_txMMLResult (list, ondemandBuf, 0, 0, 0, cmdName, 0, 1);
        onDEMAND[list].statisticsType = NOT_REGISTERED;
    } 
    else 
    {
        stmd_ondemand_txMMLResult (list, ondemandBuf, 0, 1, (onDEMAND[list].period*60)+10, cmdName, 0, 1);
        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(onDEMAND[list].period*60));
    }

    return 1;
}
#endif


int doOnDemandDelay(int list)
{
    char        query[4096];
    char        cmdName[64];
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int         i;
	char title[4][16]={"SYSTEM", "MIN_MSEC", "MAX_MSEC", "AVG_MSEC"};
    int row_index;
    char SysName[2][8] = {"SCE", "SCEB"};
    int realSysCnt =1;
	int	row_cnt = 0;

    memset (ondemandBuf, 0x00, sizeof(ondemandBuf));
    memset (ondemandErr, 0x00, sizeof(ondemandErr));

    onDEMAND[list].Txcount = onDEMAND[list].Txcount + 1;
    sprintf(ondemandBuf, "    PERIOD = %d(MIN)\n    MEASURETIME = %s(>)  - (>=)%s CNT = %d\n",
        onDEMAND[list].period, 
        get_ondemand_time(0 - ((onDEMAND[list].period-STAT_UNIT) * 60)),
        get_current_time(),
        onDEMAND[list].Txcount);
    sprintf(ondemandErr, "    PERIOD = %d(MIN)\n    MEASURETIME = %s(>)  -  (>=)%s CNT =%d\n",
        onDEMAND[list].period, 
        get_ondemand_time(0 - ((onDEMAND[list].period-STAT_UNIT) * 60)),
        get_current_time(),
        onDEMAND[list].Txcount);
    sprintf (cmdName, "%s", "stat-delay");

    sprintf(ondemandTmp, "    ===============================================================\n");
	strcat(ondemandBuf, ondemandTmp);
	sprintf(ondemandTmp, "%3s %8s %-12s %-12s %-12s\n","",title[0],title[1],title[2],title[3]);
	strcat(ondemandBuf, ondemandTmp);
    sprintf(ondemandTmp, "    ===============================================================\n");
    strcat(ondemandBuf, ondemandTmp);

    for ( i=0 ; i < realSysCnt; i++) 
    {
        if ( strcasecmp(onDEMAND[list].svcName, "ALL") ) {
            if(strcasecmp(onDEMAND[list].svcName, SysName[i]))
                continue;
        }

        sprintf(query, "SELECT system_name, "
				" ROUND(IFNULL(MIN(min_usec)*1000,0), 3), ROUND(IFNULL(MAX(max_usec)*1000,0), 3), "
				" ROUND(IFNULL(AVG(avg_usec)*1000,0), 3) "
				" from %s "
				" where system_name = '%s' AND stat_date > '%s' AND stat_date <= '%s' "
				" group by system_name ",
            STM_STATISTIC_5MINUTE_DELAY_TBL_NAME, SysName[i],
            get_ondemand_time(0 - ((onDEMAND[list].period-STAT_UNIT) * 60)), get_current_time()
        );

//printf("######################jean %s\n", query);
        if ( trcLogFlag == TRCLEVEL_SQL ) 
        {
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }
		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);
            sprintf(ondemandTmp, "    RESULT = FAIL\n    FAIL REASON = DB SELECT FAIL\n");

            onDEMAND[list].statisticsType = NOT_REGISTERED;
            strcat(ondemandErr, ondemandTmp);
            stmd_ondemand_txMMLResult (list, ondemandErr, -1, 0, 0, cmdName, 0, 1);
            return -1;
        }

		row_cnt = 0;
		result = mysql_store_result(conn);
		while( (row = mysql_fetch_row(result)) != NULL)
		{                                                                                                         
			row_index = 1;
			if( row_cnt == 0 )
			{
				sprintf(ondemandTmp, "%3s %8s %12s %12s %12s\n",
						"", row[0], row[row_index],row[row_index+1],row[row_index+2]);
				strcat(ondemandBuf, ondemandTmp);
			}
			else
			{
				sprintf(ondemandTmp, "%3s %8s %12s %12s %12s\n",
						"", "", row[row_index],row[row_index+1],row[row_index+2]);
				strcat(ondemandBuf, ondemandTmp);
			}
			row_cnt++;
		}

		mysql_free_result(result);

		if( row_cnt == 0 )
		{
			sprintf(ondemandTmp, "%3s %8s %12s %12s %12s\n",
					"", SysName[0], "0.000","0.000","0.000");
			strcat(ondemandBuf, ondemandTmp);
	//		sprintf(ondemandTmp, "	NO DATA " );
		}

    	sprintf(ondemandTmp, "    ===============================================================\n");
		strcat(ondemandBuf, ondemandTmp);                                                                         
	}    

    if ( onDEMAND[list].count == -1) 
    {
        stmd_ondemand_txMMLResult (list, ondemandBuf, 0, 0, 0, cmdName, 0, 1);
        onDEMAND[list].statisticsType = NOT_REGISTERED;
    } 
    else if ( onDEMAND[list].Txcount == onDEMAND[list].count) 
    {
        stmd_ondemand_txMMLResult (list, ondemandBuf, 0, 0, 0, cmdName, 0, 1);
        onDEMAND[list].statisticsType = NOT_REGISTERED;
    } 
    else 
    {
        stmd_ondemand_txMMLResult (list, ondemandBuf, 0, 1, (onDEMAND[list].period*60)+10, cmdName, 0, 1);
        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(onDEMAND[list].period*60));
    }

    return 1;
}


int doOnDemandSms(int list)
{
    char        query[4096];
    char        cmdName[64];
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int         i,j;
	char title[5][16]={"SYSTEM", "ITEM", "REQ", "SUCC","FAIL"};
	char title1[6][16]={"", "", "SMPP_ERR","SVR_ERR","SMSC_ERR","ETC_ERR"};
    int row_index;
    char SysName[2][8] = {"SCMA", "SCMB"};
    int realSysCnt =2;
    int realItemCnt =0;
	int	exist = 0, row_cnt = 0;

    memset (ondemandBuf, 0x00, sizeof(ondemandBuf));
    memset (ondemandErr, 0x00, sizeof(ondemandErr));

    onDEMAND[list].Txcount = onDEMAND[list].Txcount + 1;
    sprintf(ondemandBuf, "    PERIOD = %d(MIN)\n    MEASURETIME = %s(>)  - (>=)%s CNT = %d\n",
        onDEMAND[list].period, 
        get_ondemand_time(0 - ((onDEMAND[list].period-STAT_UNIT) * 60)),
        get_current_time(),
        onDEMAND[list].Txcount);
    sprintf(ondemandErr, "    PERIOD = %d(MIN)\n    MEASURETIME = %s(>)  -  (>=)%s CNT =%d\n",
        onDEMAND[list].period, 
        get_ondemand_time(0 - ((onDEMAND[list].period-STAT_UNIT) * 60)),
        get_current_time(),
        onDEMAND[list].Txcount);
    sprintf(cmdName, "%s", "stat-sms");

    sprintf(ondemandTmp, "    ================================================================\n");
	strcat(ondemandBuf, ondemandTmp);
	sprintf(ondemandTmp, "%3s %6s %-18s %-12s %-12s %-12s\n","",title[0],title[1],title[2],title[3],title[4]);
	strcat(ondemandBuf, ondemandTmp);
	sprintf(ondemandTmp, "%3s %6s %-18s %-12s %-12s %-12s %-12s\n", "","", "", title1[2],title1[3],title1[4],title1[5]);
	strcat(ondemandBuf, ondemandTmp);
    sprintf(ondemandTmp, "    ================================================================\n");
    strcat(ondemandBuf, ondemandTmp);

    for ( i=0 ; i < realSysCnt; i++) 
    {
        if ( strcasecmp(onDEMAND[list].svcName, "ALL") ) {
            if(strcasecmp(onDEMAND[list].svcName, SysName[i]))
                continue;
        }

		// realItemCnt 는 PDSN 개수가 되어야 한다? 
		sprintf(query, "SELECT count(*) from ip_code_tbl where type = 2");

		if (stmd_mysql_query (query) < 0) {
			sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}
		result = mysql_store_result(conn);
		row = mysql_fetch_row(result);
		if( row == NULL )
		{
			exist = 0;
			realItemCnt = 0;
			mysql_free_result(result);
			continue;
		}
		else
		{
			exist = 1;
			if(row[0] == NULL)
			{
				mysql_free_result(result);
				continue;
			}
			else
			{
				realItemCnt = atoi(row[0]);
			}
		}
		mysql_free_result(result); // MEM MUST Leak 10.07

        sprintf(query, "SELECT system_name, smsc_ip, "
				" IFNULL(SUM(req), 0), IFNULL(SUM(succ), 0), IFNULL(SUM(fail), 0), "
				" IFNULL(SUM(smpp_err), 0), IFNULL(SUM(svr_err), 0), "
				" IFNULL(SUM(smsc_err),0), IFNULL(SUM(etc_err), 0) "
				" from %s "
				" where system_name = '%s' AND stat_date > '%s' AND stat_date <= '%s' "
				" group by system_name, smsc_ip ",
            STM_STATISTIC_5MINUTE_SMS_TBL_NAME, SysName[i],
            get_ondemand_time(0 - ((onDEMAND[list].period-STAT_UNIT) * 60)), get_current_time()
        );

//printf("######################jean %s\n", query);
        if ( trcLogFlag == TRCLEVEL_SQL ) 
        {
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }
		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);
            sprintf(ondemandTmp, "    RESULT = FAIL\n    FAIL REASON = DB SELECT FAIL\n");

            onDEMAND[list].statisticsType = NOT_REGISTERED;
            strcat(ondemandErr, ondemandTmp);
            stmd_ondemand_txMMLResult (list, ondemandErr, -1, 0, 0, cmdName, 0, 1);
            return -1;
        }

		row_cnt = 0;
		result = mysql_store_result(conn);
		while( (row = mysql_fetch_row(result)) != NULL)
		{                                                                                                         
			row_index = 2;
			if( row_cnt == 0 )
			{
				sprintf(ondemandTmp, "%3s %6s %-18s %-12s %-12s %-12s\n",
						"", SysName[i], row[1], row[row_index],row[row_index+1],row[row_index+2]);
				strcat(ondemandBuf, ondemandTmp);
				sprintf(ondemandTmp, "%3s %6s %-18s %-12s %-12s %-12s %-12s\n",
						"", "", "", row[row_index+3],row[row_index+4],row[row_index+5],row[row_index+6]);
				strcat(ondemandBuf, ondemandTmp);
			}
			else
			{
				sprintf(ondemandTmp, "%3s %6s %-18s %-12s %-12s %-12s\n",
						"", "", row[1], row[row_index],row[row_index+1],row[row_index+2]);
				strcat(ondemandBuf, ondemandTmp);
				sprintf(ondemandTmp, "%3s %6s %-18s %-12s %-12s %-12s %-12s\n",
						"", "", "", row[row_index+3],row[row_index+4],row[row_index+5],row[row_index+6]);
				strcat(ondemandBuf, ondemandTmp);
			}
			row_cnt++;
		}

		mysql_free_result(result);

		if( row_cnt == 0 )
		{
			for( j = 0 ; j < realItemCnt; j++ )
			{
				sprintf(ondemandTmp, "%3s %6s %-18s %-12s %-12s %-12s\n",
						"", SysName[i], g_stSmsc[i].stItem[j].ip, "0","0","0");
				strcat(ondemandBuf, ondemandTmp);
				sprintf(ondemandTmp, "%3s %6s %-18s %-12s %-12s %-12s %-12s\n",
						"", "", "", "0","0","0","0");
				strcat(ondemandBuf, ondemandTmp);
			}
		}

    	sprintf(ondemandTmp, "    ================================================================\n");
		strcat(ondemandBuf, ondemandTmp);                                                                         
	}    


    if ( onDEMAND[list].count == -1) 
    {
        stmd_ondemand_txMMLResult (list, ondemandBuf, 0, 0, 0, cmdName, 0, 1);
        onDEMAND[list].statisticsType = NOT_REGISTERED;
    } 
    else if ( onDEMAND[list].Txcount == onDEMAND[list].count) 
    {
        stmd_ondemand_txMMLResult (list, ondemandBuf, 0, 0, 0, cmdName, 0, 1);
        onDEMAND[list].statisticsType = NOT_REGISTERED;
    } 
    else 
    {
        stmd_ondemand_txMMLResult (list, ondemandBuf, 0, 1, (onDEMAND[list].period*60)+10, cmdName, 0, 1);
        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(onDEMAND[list].period*60));
    }

    return 1;
}


int doOnDemandLeg(int list)
{
    char        query[4096];
    char        cmdName[64];
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int         i,j;
	char title[7][16]={"SYSTEM", "PDSN_IP", "RX_CNT", "START","INTERIM","DISCONNECT","STOP"};
	char title1[6][16]={"", "", "START_LOGON","INTERIM_LOGON","DISCONN_LOGON","LOG_OUT"};
    int row_index;
    char SysName[2][8] = {"SCMA", "SCMB"};
    int realSysCnt =2;
    int realItemCnt =0;
	int	exist = 0, row_cnt = 0;

    memset (ondemandBuf, 0x00, sizeof(ondemandBuf));
    memset (ondemandErr, 0x00, sizeof(ondemandErr));

    onDEMAND[list].Txcount = onDEMAND[list].Txcount + 1;
    sprintf(ondemandBuf, "    PERIOD = %d(MIN)\n    MEASURETIME = %s(>)  - (>=)%s CNT = %d\n",
        onDEMAND[list].period, 
        get_ondemand_time(0 - ((onDEMAND[list].period-STAT_UNIT) * 60)),
        get_current_time(),
        onDEMAND[list].Txcount);
    sprintf(ondemandErr, "    PERIOD = %d(MIN)\n    MEASURETIME = %s(>)  -  (>=)%s CNT =%d\n",
        onDEMAND[list].period, 
        get_ondemand_time(0 - ((onDEMAND[list].period-STAT_UNIT) * 60)),
        get_current_time(),
        onDEMAND[list].Txcount);
    sprintf(cmdName, "%s", "stat-account");

    sprintf(ondemandTmp, "    ====================================================================\n");
	strcat(ondemandBuf, ondemandTmp);
	sprintf(ondemandTmp, "%3s %6s %-18s %-12s %-12s %-12s %-12s %-12s\n","",title[0],title[1],title[2],title[3],title[4],title[5],title[6] );
	strcat(ondemandBuf, ondemandTmp);
	sprintf(ondemandTmp, "%3s %6s %-18s %-12s %-12s %-12s %-12s\n", "","", "", title1[2],title1[3],title1[4],title1[5]);
	strcat(ondemandBuf, ondemandTmp);
    sprintf(ondemandTmp, "    ====================================================================\n");
    strcat(ondemandBuf, ondemandTmp);

    for ( i=0 ; i < realSysCnt; i++) 
    {
        if ( strcasecmp(onDEMAND[list].svcName, "ALL") ) {
            if(strcasecmp(onDEMAND[list].svcName, SysName[i]))
                continue;
        }

		sprintf(query, "SELECT  count(*) from ip_code_tbl where type = 1; ");

		if (stmd_mysql_query (query) < 0) {
			sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}
		result = mysql_store_result(conn);
		row = mysql_fetch_row(result);
		if( row == NULL )
		{
			exist = 0;
			realItemCnt = 0;
			mysql_free_result(result);
		}
		else
		{
			exist = 1;
			realItemCnt = atoi(row[0]);
			mysql_free_result(result);
		}

        sprintf(query, "SELECT system_name, pdsn_ip, "
				" IFNULL(SUM(rx_cnt), 0), IFNULL(SUM(start), 0), IFNULL(SUM(interim), 0), "
				" IFNULL(SUM(disconnect), 0), IFNULL(SUM(stop), 0), "
				" IFNULL(SUM(start_logon_cnt), 0), IFNULL(SUM(int_logon_cnt), 0), "
				" IFNULL(SUM(disc_logon_cnt), 0), IFNULL(SUM(logout_cnt), 0) "
				" from %s "
				" where system_name = '%s' AND stat_date > '%s' AND stat_date <= '%s' "
				" group by system_name, pdsn_ip ",
            STM_STATISTIC_5MINUTE_LEG_TBL_NAME, SysName[i],
            get_ondemand_time(0 - ((onDEMAND[list].period-STAT_UNIT) * 60)), get_current_time()
        );

//printf("######################jean %s\n", query);
        if ( trcLogFlag == TRCLEVEL_SQL ) 
        {
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }
		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);
            sprintf(ondemandTmp, "    RESULT = FAIL\n    FAIL REASON = DB SELECT FAIL\n");

            onDEMAND[list].statisticsType = NOT_REGISTERED;
            strcat(ondemandErr, ondemandTmp);
            stmd_ondemand_txMMLResult (list, ondemandErr, -1, 0, 0, cmdName, 0, 1);
            return -1;
        }

		row_cnt = 0;
		result = mysql_store_result(conn);
		while( (row = mysql_fetch_row(result)) != NULL)
		{                                                                                                         
			row_index = 2;
			if( row_cnt == 0 )
			{
				sprintf(ondemandTmp, "%3s %6s %-18s %-12s %-12s %-12s %-12s %-12s\n",
						"", SysName[i], row[1], row[row_index],row[row_index+1],row[row_index+2],row[row_index+3],row[row_index+4]);
				strcat(ondemandBuf, ondemandTmp);
				sprintf(ondemandTmp, "%3s %6s %-18s %-12s %-12s %-12s %-12s\n",
						"", "", "", row[row_index+5],row[row_index+6],row[row_index+7],row[row_index+8]);
				strcat(ondemandBuf, ondemandTmp);
			}
			else
			{
				sprintf(ondemandTmp, "%3s %6s %-18s %-12s %-12s %-12s %-12s %-12s\n",
						"", "", row[1], row[row_index],row[row_index+1],row[row_index+2],row[row_index+3],row[row_index+4]);
				strcat(ondemandBuf, ondemandTmp);
				sprintf(ondemandTmp, "%3s %6s %-18s %-12s %-12s %-12s %-12s\n",
						"", "", "", row[row_index+5],row[row_index+6],row[row_index+7],row[row_index+8]);
				strcat(ondemandBuf, ondemandTmp);
			}
			row_cnt++;
		}

		mysql_free_result(result);
		if( row_cnt == 0 )
		{
			for(j=0;j<realItemCnt;j++)
			{
				if( j == 0 )
				{
					sprintf(ondemandTmp, "%3s %6s %-18s %-12s %-12s %-12s %-12s %-12s\n",
							"", SysName[i], g_stPdsn.stItem[j].ip, "0","0","0","0","0");
					strcat(ondemandBuf, ondemandTmp);
					sprintf(ondemandTmp, "%3s %6s %-18s %-12s %-12s %-12s %-12s\n",
							"", "", "", "0","0","0","0");
					strcat(ondemandBuf, ondemandTmp);
				}
				else
				{
					sprintf(ondemandTmp, "%3s %6s %-18s %-12s %-12s %-12s %-12s %-12s\n",
							"", "", g_stPdsn.stItem[j].ip, "0","0","0","0","0");
					strcat(ondemandBuf, ondemandTmp);
					sprintf(ondemandTmp, "%3s %6s %-18s %-12s %-12s %-12s %-12s\n",
							"", "", "", "0","0","0","0");
					strcat(ondemandBuf, ondemandTmp);
				}
			}
		}
		sprintf(ondemandTmp, "    =======================================================================================\n");   
		strcat(ondemandBuf, ondemandTmp);                                                                         

	}    

    if ( onDEMAND[list].count == -1) 
    {
        stmd_ondemand_txMMLResult (list, ondemandBuf, 0, 0, 0, cmdName, 0, 1);
        onDEMAND[list].statisticsType = NOT_REGISTERED;
    } 
    else if ( onDEMAND[list].Txcount == onDEMAND[list].count) 
    {
        stmd_ondemand_txMMLResult (list, ondemandBuf, 0, 0, 0, cmdName, 0, 1);
        onDEMAND[list].statisticsType = NOT_REGISTERED;
    } 
    else 
    {
        stmd_ondemand_txMMLResult (list, ondemandBuf, 0, 1, (onDEMAND[list].period*60)+10, cmdName, 0, 1);
        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(onDEMAND[list].period*60));
    }

    return 1;
}


int doOnDemandLoad(int list)
{
    char        query[4096];
    char        cmdName[64];
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    char        itemRes[8][10];
    int         i,j;
//    char *title[]={"CPU","MEM","DISK","QUEUE","SESS"};
    char title[4][16]={"CPU","MEM","DISK","QUEUE"};
    int row_index;
    char SysName[5][8];
    int realSysCnt =0;
    int realItemCnt =0;

    for(i=0; i<sysCnt; i++ ){
        sprintf(SysName[i], "%s", StatisticSystemInfo[i].sysName);
        realSysCnt++;
    }
/* 10.13 TAP REMOVE
    sprintf(SysName[realSysCnt], "TAPA");realSysCnt++;
    sprintf(SysName[realSysCnt], "TAPB");realSysCnt++;
*/
	sprintf(SysName[realSysCnt], "SCEA");realSysCnt++;
	sprintf(SysName[realSysCnt], "SCEB");realSysCnt++;

    memset (ondemandBuf, 0x00, sizeof(ondemandBuf));
    memset (ondemandErr, 0x00, sizeof(ondemandErr));

    onDEMAND[list].Txcount = onDEMAND[list].Txcount + 1;
    sprintf(ondemandBuf, "    PERIOD = %d(MIN)\n    MEASURETIME = %s(>)  - (>=)%s CNT = %d\n",
        onDEMAND[list].period, 
        get_ondemand_time(0 - ((onDEMAND[list].period-STAT_UNIT) * 60)),
        get_current_time(),
        onDEMAND[list].Txcount);
    sprintf(ondemandErr, "    PERIOD = %d(MIN)\n    MEASURETIME = %s(>)  -  (>=)%s CNT =%d\n",
        onDEMAND[list].period, 
        get_ondemand_time(0 - ((onDEMAND[list].period-STAT_UNIT) * 60)),
        get_current_time(),
        onDEMAND[list].Txcount);
    sprintf(cmdName, "%s", "stat-load");

    sprintf(ondemandTmp, "    ==================================\n");
    strcat(ondemandBuf, ondemandTmp);
    sprintf(ondemandTmp, "    SYSTEM    ITEM    AVG(%%)    MAX(%%)\n");
    strcat(ondemandBuf, ondemandTmp);
    sprintf(ondemandTmp, "    ==================================\n");
    strcat(ondemandBuf, ondemandTmp);

    for ( i=0 ; i < realSysCnt; i++) 
    {
        if ( strcasecmp(onDEMAND[list].svcName, "ALL") ) {
            if(strcasecmp(onDEMAND[list].svcName, SysName[i]))
                continue;
        }

        //if(!strcasecmp(SysName[i], "BSDM"))
        if(!strcasecmp(SysName[i], "DSCM"))
            realItemCnt=4;
/* 10.13 TAP REMOVE
        else if(!strcasecmp(SysName[i], "TAPA") || !strcasecmp(SysName[i], "TAPB") )
			realItemCnt = 2;
*/
		else if(!strcasecmp(SysName[i], "SCEA") || !strcasecmp(SysName[i], "SCEB"))
            realItemCnt=3;
        else
            realItemCnt=4;
//            realItemCnt=5;

        sprintf(query, "SELECT "
                " AVG(avr_cpu0), MAX(max_cpu0)," 
                " AVG(avr_memory), MAX(max_memory)," 
                " AVG(avr_disk), MAX(max_disk)," 
                " AVG(avr_msgQ), MAX(max_msgQ) " 
//                " AVG(avr_sess), MAX(max_sess)" 
                " from %s "
                " where system_name = '%s' AND stat_date > '%s' AND stat_date <= '%s'",
            STM_STATISTIC_5MINUTE_LOAD_TBL_NAME, SysName[i],
            get_ondemand_time(0 - ((onDEMAND[list].period-STAT_UNIT) * 60)), get_current_time()
        );

//printf("######################jean %s\n", query);
        if ( trcLogFlag == TRCLEVEL_SQL ) 
        {
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }
		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);
            sprintf(ondemandTmp, "    RESULT = FAIL\n    FAIL REASON = DB SELECT FAIL\n");

            onDEMAND[list].statisticsType = NOT_REGISTERED;
            strcat(ondemandErr, ondemandTmp);
            stmd_ondemand_txMMLResult (list, ondemandErr, -1, 0, 0, cmdName, 0, 1);
            return -1;
        }

        result = mysql_store_result(conn);
        row = mysql_fetch_row(result);
        if (row[0] == NULL) 
        { //위의 select구문은 row pointer는 NULL로 오지 않는다. 각 객체가 NULL로 온다.
            for(j=0;j<realItemCnt;j++){
                if (j==0){
                    sprintf(ondemandTmp, "%3s %-9s %-5s %8s %9s\n", 
                        "", SysName[i], title[j], "0.0", "0.0");
                    strcat(ondemandBuf, ondemandTmp);
                } else {
                    sprintf(ondemandTmp, "%3s %-9s %-5s %8s %9s\n", 
                        "", "", title[j], "0.0", "0.0");
                    strcat(ondemandBuf, ondemandTmp);
                }
            }
            sprintf(ondemandTmp, "    ==================================\n");
            strcat(ondemandBuf, ondemandTmp);

            sprintf(trcBuf, "Ondemand Load select empty set\n");
            trclib_writeLogErr (FL, trcBuf);
        } 
        else 
        {
			for(j=0;j< 8;j++){ // SESS는 그대로 출력 하자. add by helca 080930
				sprintf(itemRes[j], "%s",  row[j]);
				sprintf(itemRes[j], "%d.%d", atoi(itemRes[j])/10, atoi(itemRes[j])%10); 
			}

			row_index = 0;
			for(j=0;j<realItemCnt;j++){
				if (j==0){
					sprintf(ondemandTmp, "%3s %-9s %-5s %8s %9s\n", 
							"", SysName[i], title[j], itemRes[row_index],itemRes[row_index+1]);
					strcat(ondemandBuf, ondemandTmp);
				} else {
					sprintf(ondemandTmp, "%3s %-9s %-5s %8s %9s\n", 
							"", "", title[j], itemRes[row_index],itemRes[row_index+1]);
					strcat(ondemandBuf, ondemandTmp);
				}

				row_index += 2;
			}
			sprintf(ondemandTmp, "    ==================================\n");
			strcat(ondemandBuf, ondemandTmp);

		}
		mysql_free_result(result);
    }

    if ( onDEMAND[list].count == -1) 
    {
        stmd_ondemand_txMMLResult (list, ondemandBuf, 0, 0, 0, cmdName, 0, 1);
        onDEMAND[list].statisticsType = NOT_REGISTERED;
    } 
    else if ( onDEMAND[list].Txcount == onDEMAND[list].count) 
    {
        stmd_ondemand_txMMLResult (list, ondemandBuf, 0, 0, 0, cmdName, 0, 1);
        onDEMAND[list].statisticsType = NOT_REGISTERED;
    } 
    else 
    {
        stmd_ondemand_txMMLResult (list, ondemandBuf, 0, 1, (onDEMAND[list].period*60)+10, cmdName, 0, 1);
        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(onDEMAND[list].period*60));
    }


    return 1;
}

int doOnDemandRuleEnt(int list)
{
    char        query[4096];
    char        cmdName[64];
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int         i,j;
	char title[4][16] = {"SYSTEM", "ITEM", "Session", "Block"};
	char title1[5][16] = {"", "", "UpStream(Mbps)", "DnStream(Mbps)", "Total(Mbps)"};
	char title2[5][16] = {"", "", "UpByte(MBytes)", "DnByte(MBytes)", "Total(MBytes)"};
    int row_index;
    char SysName[2][8];
    char SysIp[2][16];
    int realSysCnt =0;
    int realItemCnt =0, row_cnt = 0;


    memset (ondemandBuf, 0x00, sizeof(ondemandBuf));
    memset (ondemandErr, 0x00, sizeof(ondemandErr));

    onDEMAND[list].Txcount = onDEMAND[list].Txcount + 1;

    sprintf(ondemandBuf, "    PERIOD = %d\n    MEASURETIME = %s(>)  -  (<=)%s CNT = %d\n",
        onDEMAND[list].period, 
        get_ondemand_time(0 - ((onDEMAND[list].period-STAT_UNIT) * 60)),
        get_current_time(),
        onDEMAND[list].Txcount);
    sprintf(ondemandErr, "    PERIOD = %d\n    MEASURETIME = (>)%s  -  (<=)%s CNT = %d\n",
        onDEMAND[list].period, 
        get_ondemand_time(0 - ((onDEMAND[list].period-STAT_UNIT) * 60)),
        get_current_time(),
        onDEMAND[list].Txcount);
    sprintf (cmdName, "stat-rule-ent");

    sprintf(ondemandTmp, "    =======================================================================================\n");
	strcat(ondemandBuf, ondemandTmp);
	sprintf(ondemandTmp, "%3s %6s %-28s %15s %15s \n","",title[0],title[1],title[2],title[3]);
	strcat(ondemandBuf, ondemandTmp);
	sprintf(ondemandTmp, "%3s %6s %-28s %15s %15s %15s\n","","","",title1[2],title1[3],title1[4]);
	strcat(ondemandBuf, ondemandTmp);
	sprintf(ondemandTmp, "%3s %6s %-28s %15s %15s %15s\n","","","",title2[2],title2[3],title2[4]);
	strcat(ondemandBuf, ondemandTmp);
    sprintf(ondemandTmp, "    =======================================================================================\n");
    strcat(ondemandBuf, ondemandTmp);

    for(i=0; i<SCE_CNT; i++ ){
        sprintf (SysName[i], "%s", g_stSCE[i].sce_name);
		sprintf (SysIp[i], "%s", g_stSCE[i].sce_ip);
        realSysCnt++;
    }

	double  upbyte = 0; 
	double  dnbyte = 0; 
	double  sumbyte = 0; 
	int		snd_cnt = 1;

    for ( i=0 ; i < realSysCnt; i++) 
    {
        if ( strcasecmp(onDEMAND[list].svcName, "ALL") ) {
            if(strcasecmp(onDEMAND[list].svcName, SysName[i]))
                continue;
        }

		/*
		sprintf(query, " select  count(*) from %s where where record_source = '%s' "
						" AND stat_date > '%s' AND stat_date <= '%s'"
						" 
						*/

		realItemCnt = g_stSCEEntry[i].ruleEntryCnt;
                    
        sprintf(query, "SELECT rule_ent_id, rule_ent_name, "
				" IFNULL(SUM(session),0), IFNULL(SUM(block_cnt),0), "
				" IFNULL(SUM(upstream_volume),0), "
				" IFNULL(SUM(downstream_volume),0), "
				" IFNULL(SUM(upstream_volume),0) + IFNULL(SUM(downstream_volume),0) "
				" from %s "
				" where record_source = '%s' AND stat_date > '%s' AND stat_date <= '%s' "
				" group by rule_ent_id order by rule_ent_id",
		   STM_STATISTIC_5MINUTE_RULEENT_TBL_NAME, SysName[i],
			get_ondemand_time(0 - ((onDEMAND[list].period-STAT_UNIT) * 60)), 
			get_current_time() );

        if ( trcLogFlag == TRCLEVEL_SQL ) {
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }

		logPrint(trcLogId,FL,"RULE ENT ondemand query: %s\n", query);

		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; queyr=%s, err=%s\n",query, mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);

            onDEMAND[list].statisticsType = NOT_REGISTERED;
            sprintf(ondemandTmp, "    RESULT = FAIL\n    FAIL REASON = DB SELECT FAIL\n");
            strcat(ondemandErr, ondemandTmp);
            stmd_ondemand_txMMLResult (list, ondemandErr, -1, 0, 0, cmdName, 0, 1);
            return -1;
        }

		row_cnt = 0;
        result = mysql_store_result(conn);
        while( (row = mysql_fetch_row(result)) != NULL) 
		{
			row_index = 2;
			upbyte = atof(row[row_index+2]);
			dnbyte = atof(row[row_index+3]);
			sumbyte = atof(row[row_index+4]);

			if( row_cnt == 0 )
			{
				sprintf(ondemandTmp, "%3s %6s %-28s %15s %15s \n",
						"", SysName[i], row[1], row[row_index], row[row_index+1]);
				strcat(ondemandBuf, ondemandTmp);
			} else {
				sprintf(ondemandTmp, "%3s %6s %-28s %15s %15s \n",
						"", "", row[1], row[row_index], row[row_index+1]);
				strcat(ondemandBuf, ondemandTmp);
			}

			sprintf(ondemandTmp, "%3s %6s %-28s %15.03f %15.03f %15.03f\n",
					"", "", "", /*upstream_volume*/ upbyte/(double)1024/(double)1024/(onDEMAND[list].period*60), 
					/*downstream_volume */ dnbyte/(double)1024/(double)1024/(double)(onDEMAND[list].period*60),
					/*total*/ sumbyte/(double)1024/(double)1024/(double)(onDEMAND[list].period*60));
			strcat(ondemandBuf, ondemandTmp);
			sprintf(ondemandTmp, "%3s %6s %-28s %15.03f %15.03f %15.03f\n",
					"", "", "", /*upstream_volume*/ upbyte/(double)1024/(double)1024,
					/*downstream_volume */ dnbyte/(double)1024/(double)1024,
					/*total*/ sumbyte/(double)1024/(double)1024);

			strcat(ondemandBuf, ondemandTmp);
			sprintf(ondemandTmp, "    --------------------------------------------------------------------------------------\n");
			strcat(ondemandBuf, ondemandTmp);

            if (strlen (ondemandBuf) > 3000) {
                    stmd_ondemand_txMMLResult (list, ondemandBuf, 0, 1,\
                            (onDEMAND[list].period*60)+10, cmdName, 1, snd_cnt++);
                    memset(ondemandBuf, 0, sizeof(ondemandBuf));
			}
			row_cnt++;
		}
        mysql_free_result(result);
		
		// ?? 나오면 안됨. !!
		if( row_cnt == 0 )
		{
			for( j = 0; j < realItemCnt; j++ )
			{
				if( j == 0 )
				{
					sprintf(ondemandTmp, "%3s %6s %28s %15s %15s \n",
							"", SysName[i], g_stSCEEntry[i].stEntry[j].eName, "0","0");
//fprintf(stderr,"RULE-ENTRY-NAME : %s \n", g_stSCEEntry[i].stEntry[j].eName );
					strcat(ondemandBuf, ondemandTmp);
					sprintf(ondemandTmp, "%3s %6s %-28s %15s %15s %15s\n",
							"", "", "", "0.000","0.000","0.000");
					strcat(ondemandBuf, ondemandTmp);
					sprintf(ondemandTmp, "%3s %6s %-28s %15s %15s %15s\n",
							"", "", "", "0.000","0.000","0.000");
					strcat(ondemandBuf, ondemandTmp);
					sprintf(ondemandTmp, "    --------------------------------------------------------------------------------------\n");
					strcat(ondemandBuf, ondemandTmp);
				} 
				else 
				{
					sprintf(ondemandTmp, "%3s %6s %-28s %15s %15s \n",
							"", "", g_stSCEEntry[i].stEntry[j].eName, "0","0");
//fprintf(stderr,"RULE-ENTRY-NAME : %s \n", g_stSCEEntry[i].stEntry[j].eName );

					strcat(ondemandBuf, ondemandTmp);
					sprintf(ondemandTmp, "%3s %6s %-28s %15s %15s %15s\n",
							"", "", "", "0.000","0.000","0.000");
					strcat(ondemandBuf, ondemandTmp);
					sprintf(ondemandTmp, "%3s %6s %-28s %15s %15s %15s\n",
							"", "", "", "0.000","0.000","0.000");
					strcat(ondemandBuf, ondemandTmp);
					sprintf(ondemandTmp, "    --------------------------------------------------------------------------------------\n");
					strcat(ondemandBuf, ondemandTmp);
				}
				// 2010. 12. 22
				if (strlen (ondemandBuf) > 3000) {
					stmd_ondemand_txMMLResult (list, ondemandBuf, 0, 1,\
							(onDEMAND[list].period*60)+10, cmdName, 1, snd_cnt++);
					memset(ondemandBuf, 0, sizeof(ondemandBuf));
				}
			}
		}
    
        sprintf(ondemandTmp, "    =======================================================================================\n");
        strcat(ondemandBuf, ondemandTmp);

		if( row_cnt == 0 )
			sprintf(ondemandTmp, " NO DATA " );
    
        sprintf(ondemandTmp, "    =======================================================================================\n");
        strcat(ondemandBuf, ondemandTmp);
    }

    if ( onDEMAND[list].count == -1) 
	{
        stmd_ondemand_txMMLResult (list, ondemandBuf, 0, 0, 0, cmdName, 0, snd_cnt);
        onDEMAND[list].statisticsType = NOT_REGISTERED;
    } 
	else if ( onDEMAND[list].Txcount == onDEMAND[list].count) 
	{
        stmd_ondemand_txMMLResult (list, ondemandBuf, 0, 0, 0, cmdName, 0, snd_cnt);
        onDEMAND[list].statisticsType = NOT_REGISTERED;
    } 
	else 
	{
        stmd_ondemand_txMMLResult (list, ondemandBuf, 0, 1, (onDEMAND[list].period*60)+10, cmdName, 0, snd_cnt);
        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(onDEMAND[list].period*60));
    }
	return 0;
}


int doOnDemandRuleSet(int list)
{
    char        query[4096];
    char        cmdName[64];
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int         i, j;
	char title[4][16] = {"SYSTEM", "RULE", "SESSION", "BLK_CNT"};
	char title1[5][16] = {"", "", "UP_Mbps", "DOWN_Mbps", "TOTAL_Mbps"};
	char title2[5][16] = {"", "", "UP_MBytes", "DOWN_MBytes", "TOTAL_MBytes"};
    int row_index;
    char SysName[2][8];
    char SysIp[2][16];
    int realSysCnt =0;
    int realItemCnt =0, row_cnt = 0;

	int		snd_cnt = 1;

    for(i=0; i<SCE_CNT; i++ ){
        sprintf(SysName[i], "%s", g_stSCE[i].sce_name);
		sprintf(SysIp[i], "%s", g_stSCE[i].sce_ip);
        realSysCnt++;
    }

    memset (ondemandBuf, 0x00, sizeof(ondemandBuf));
    memset (ondemandErr, 0x00, sizeof(ondemandErr));

    onDEMAND[list].Txcount = onDEMAND[list].Txcount + 1;

    sprintf(ondemandBuf, "    PERIOD = %d\n    MEASURETIME = %s(>)  -  (<=)%s CNT = %d\n",
        onDEMAND[list].period, 
        get_ondemand_time(0 - ((onDEMAND[list].period-STAT_UNIT) * 60)),
        get_current_time(),
        onDEMAND[list].Txcount);
    sprintf(ondemandErr, "    PERIOD = %d\n    MEASURETIME = (>)%s  -  (<=)%s CNT = %d\n",
        onDEMAND[list].period, 
        get_ondemand_time(0 - ((onDEMAND[list].period-STAT_UNIT) * 60)),
        get_current_time(),
        onDEMAND[list].Txcount);
    sprintf(cmdName, "%s", "stat-rule-set");

    sprintf(ondemandTmp, "    =======================================================================================\n");
	strcat(ondemandBuf, ondemandTmp);
	sprintf(ondemandTmp, "%3s %6s %-28s %15s %15s\n","",title[0],title[1],title[2],title[3]);
	strcat(ondemandBuf, ondemandTmp);
	sprintf(ondemandTmp, "%3s %6s %-28s %15s %15s %15s\n","","","",title1[2],title1[3],title1[4]);
	strcat(ondemandBuf, ondemandTmp);
	sprintf(ondemandTmp, "%3s %6s %-28s %15s %15s %15s\n","","","",title2[2],title2[3],title2[4]);
	strcat(ondemandBuf, ondemandTmp);
    sprintf(ondemandTmp, "    =======================================================================================\n");
    strcat(ondemandBuf, ondemandTmp);

//	realItemCnt = g_ruleItemCnt;

    for ( i=0 ; i < realSysCnt; i++) 
    {
        if ( strcasecmp(onDEMAND[list].svcName, "ALL") ) {
            if(strcasecmp(onDEMAND[list].svcName, SysName[i]))
                continue;
        }

		realItemCnt = g_stSCERule[i].ruleSetCnt;
                    
		//char title[4][16] = {"SYSTEM", "RULE", "SESSION", "BLK_CNT"};
		//char title1[5][16] = {"", "", "UP_Mbps", "DOWN_Mbps", "TOTAL_Mbps"};
		//char title2[5][16] = {"", "", "UP_MBytes", "DOWN_MBytes", "TOTAL_MBytes"};
        sprintf(query, "SELECT rule_set_id, rule_set_name, IFNULL(SUM(session),0), IFNULL(SUM(block_cnt),0), "
				" ROUND(IFNULL(SUM(upstream_volume)*8/1024/1024/(%d*60) ,0), 3), "
				" ROUND(IFNULL(SUM(downstream_volume)*8/1024/1024/(%d*60) ,0), 3), "
				" ROUND((IFNULL(SUM(upstream_volume),0) + IFNULL(SUM(downstream_volume),0))*8/1024/1024/(%d*60), 3), "
				" ROUND(IFNULL(SUM(upstream_volume)/1024/1024 ,0), 3), "
				" ROUND(IFNULL(SUM(downstream_volume)/1024/1024 ,0), 3), "
				" ROUND((IFNULL(SUM(upstream_volume),0) + IFNULL(SUM(downstream_volume),0))/1024/1024, 3) "
				" from %s "
				" where record_source = '%s' AND stat_date > '%s' AND stat_date <= '%s' "
				" group by rule_set_id, rule_set_name  order by rule_set_id",
				onDEMAND[list].period, onDEMAND[list].period, onDEMAND[list].period,
		   STM_STATISTIC_5MINUTE_RULESET_TBL_NAME, SysName[i],
			get_ondemand_time(0 - ((onDEMAND[list].period-STAT_UNIT) * 60)), 
			get_current_time() );


        if ( trcLogFlag == TRCLEVEL_SQL ) {
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }

		logPrint(trcLogId,FL,"RULE SET ondemand query: %s\n", query);

		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);

            onDEMAND[list].statisticsType = NOT_REGISTERED;
            sprintf(ondemandTmp, "    RESULT = FAIL\n    FAIL REASON = DB SELECT FAIL\n");
            strcat(ondemandErr, ondemandTmp);
            stmd_ondemand_txMMLResult (list, ondemandErr, -1, 0, 0, cmdName, 0, 1);
            return -1;
        }

		row_cnt = 0;
        result = mysql_store_result(conn);
        while( (row = mysql_fetch_row(result)) != NULL) 
		{
            row_index = 2; // start upstream_volume

			if( row_cnt == 0 )
			{
				sprintf(ondemandTmp, "%3s %6s %-28s %15s %15s \n",
						"", SysName[i], row[1], row[row_index], row[row_index+1]);
				strcat(ondemandBuf, ondemandTmp);
			} else {
				sprintf(ondemandTmp, "%3s %6s %-28s %15s %15s \n",
						"", "", row[1], row[row_index], row[row_index+1]);
				strcat(ondemandBuf, ondemandTmp);
			}

			sprintf(ondemandTmp, "%3s %6s %-28s %15s %15s %15s\n",
					"", "", "", row[row_index+2], row[row_index+3], row[row_index+4]);
			strcat(ondemandBuf, ondemandTmp);

			sprintf(ondemandTmp, "%3s %6s %-28s %15s %15s %15s\n",
					"", "", "", row[row_index+5], row[row_index+6], row[row_index+7]);
			strcat(ondemandBuf, ondemandTmp);


			sprintf(ondemandTmp, "    --------------------------------------------------------------------------------------\n");
			strcat(ondemandBuf, ondemandTmp);

            if (strlen (ondemandBuf) > 3000) {
                    stmd_ondemand_txMMLResult (list, ondemandBuf, 0, 1,\
                            (onDEMAND[list].period*60)+10, cmdName, 1, snd_cnt++);
                    memset(ondemandBuf, 0, sizeof(ondemandBuf));
			}
			row_cnt++;
        }
        mysql_free_result(result);


		if( row_cnt == 0 )
		{
			for( j = 0; j < realItemCnt; j++ )
			{
				if( j == 0 )
				{
					sprintf(ondemandTmp, "%3s %6s %-28s %15s %15s \n",
							"", SysName[i], g_stSCERule[i].stRule[g_ruleIdBuf[j]].rName, "0","0");
					strcat(ondemandBuf, ondemandTmp);
					sprintf(ondemandTmp, "%3s %6s %-28s %15s %15s %15s\n",
							"", "", "", "0.000","0.000","0.000");
					strcat(ondemandBuf, ondemandTmp);
					sprintf(ondemandTmp, "%3s %6s %-28s %15s %15s %15s\n",
							"", "", "", "0.000","0.000","0.000");
					strcat(ondemandBuf, ondemandTmp);
					sprintf(ondemandTmp, "    --------------------------------------------------------------------------------------\n");
					strcat(ondemandBuf, ondemandTmp);
				} 
				else 
				{
					sprintf(ondemandTmp, "%3s %6s %-28s %-15s %-15s \n",
							"", "", g_stSCERule[i].stRule[g_ruleIdBuf[j]].rName, "0","0");

					strcat(ondemandBuf, ondemandTmp);
					sprintf(ondemandTmp, "%3s %6s %-28s %-15s %-15s %-15s\n",
							"", "", "", "0.000","0.000","0.000");
					strcat(ondemandBuf, ondemandTmp);
					sprintf(ondemandTmp, "%3s %6s %-28s %-15s %-15s %-15s\n",
							"", "", "", "0.000","0.000","0.000");
					strcat(ondemandBuf, ondemandTmp);
					sprintf(ondemandTmp, "    --------------------------------------------------------------------------------------\n");
					strcat(ondemandBuf, ondemandTmp);
				}
// 2010. 12. 22 추가 
				if (strlen (ondemandBuf) > 3000) {
					stmd_ondemand_txMMLResult (list, ondemandBuf, 0, 1,\
							(onDEMAND[list].period*60)+10, cmdName, 1, snd_cnt++);
					memset(ondemandBuf, 0, sizeof(ondemandBuf));
				}
			}
		}
    
        sprintf(ondemandTmp, "    =======================================================================================\n");
        strcat(ondemandBuf, ondemandTmp);
    }

    if ( onDEMAND[list].count == -1) 
	{
        stmd_ondemand_txMMLResult (list, ondemandBuf, 0, 0, 0, cmdName, 0, snd_cnt);
        onDEMAND[list].statisticsType = NOT_REGISTERED;
    } 
	else if ( onDEMAND[list].Txcount == onDEMAND[list].count) 
	{
        stmd_ondemand_txMMLResult (list, ondemandBuf, 0, 0, 0, cmdName, 0, snd_cnt);
        onDEMAND[list].statisticsType = NOT_REGISTERED;
    } 
	else 
	{
        stmd_ondemand_txMMLResult (list, ondemandBuf, 0, 1, (onDEMAND[list].period*60)+10, cmdName, 0, snd_cnt);
        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(onDEMAND[list].period*60));
    }
	return 0;
}


int doOnDemandFault(int list)
{
    char        query[4096] = {0,};
    char        cmdName[64] = {0,};
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int         i,j;
    char title1[4][16]={"CPU","MEM","HW","PROC"};
	/*
    char *title1[]={"HW","PROC","NET_NMS","CONN_NTP","CONN_NMS"};
    char *title2[]={"DUP_HB","DUP_OOS", "RATE_WAP1", "RATE_WAP2","RATE_HTTP","RATE_UAWAP","RATE_AAA", "RATE_VODS", "RATE_ANAAA",
                    "RATE_VT","RATE_RADIUS","NET_UAWAP","NET_AAA","CONN_UAWAP","CONN_NTP"};
	*/
    int row_index;
    char SysName[5][8];
    int realSysCnt =0;
    int realItemCnt =0;

    for(i=0; i<sysCnt; i++ ){
        sprintf(SysName[i], "%s", StatisticSystemInfo[i].sysName);
        realSysCnt++;
    }
    sprintf(SysName[realSysCnt], "TAPA");realSysCnt++;
    sprintf(SysName[realSysCnt], "TAPB");realSysCnt++;
	sprintf(SysName[realSysCnt], "SCEA");realSysCnt++; // jjinri add 2009.04.22 
	sprintf(SysName[realSysCnt], "SCEB");realSysCnt++; // jjinri add 2009.04.22 

    memset (ondemandBuf, 0x00, sizeof(ondemandBuf));
    memset (ondemandErr, 0x00, sizeof(ondemandErr));

    onDEMAND[list].Txcount = onDEMAND[list].Txcount + 1;

    sprintf(ondemandBuf, "    PERIOD = %d\n    MEASURETIME = (>)%s  -  (<=)%s CNT = %d\n",
        onDEMAND[list].period, 
        (char *)get_ondemand_time(0 - ((onDEMAND[list].period-STAT_UNIT) * 60)),
        (char *)get_current_time(),
        onDEMAND[list].Txcount);
    sprintf(ondemandErr, "    PERIOD = %d\n    MEASURETIME = (>)%s  -  (<=)%s CNT = %d\n",
        onDEMAND[list].period, 
        (char *)get_ondemand_time(0 - ((onDEMAND[list].period-STAT_UNIT) * 60)),
        (char *)get_current_time(),
        onDEMAND[list].Txcount);
    sprintf(cmdName, "stat-fault");

    sprintf(ondemandTmp, "    ===============================================\n");
    strcat(ondemandBuf, ondemandTmp);
    sprintf(ondemandTmp, "    SYSTEM  ITEM           MINOR    MAJOR  CRITICAL\n");
    strcat(ondemandBuf, ondemandTmp);
    sprintf(ondemandTmp, "    ===============================================\n");
    strcat(ondemandBuf, ondemandTmp);

    for ( i=0 ; i < realSysCnt; i++) 
    {
        if ( strcasecmp(onDEMAND[list].svcName, "ALL") ) {
            if(strcasecmp(onDEMAND[list].svcName, SysName[i]))
                continue;
        }

        //if(!strcasecmp(SysName[i], "BSDM"))
        if(!strcasecmp(SysName[i], "DSCM"))
            realItemCnt=4;
		/* 
        else if( !strcasecmp(SysName[i], "TAPA") || !strcasecmp(SysName[i], "TAPB") || \
				!strcasecmp(SysName[i], "SCEA") || !strcasecmp(SysName[i], "SCEB")) 
            realItemCnt=1;
		*/
        else if( !strcasecmp(SysName[i], "TAPA") || !strcasecmp(SysName[i], "TAPB"))
			realItemCnt = 3;
		else if(!strcasecmp(SysName[i], "SCEA") || !strcasecmp(SysName[i], "SCEB")) 
			realItemCnt = 3;
        else
            realItemCnt=4;
                    
        sprintf(query, "SELECT"
            " SUM(cpu_min_cnt),SUM(cpu_maj_cnt),SUM(cpu_cri_cnt),"
            " SUM(mem_min_cnt),SUM(mem_maj_cnt),SUM(mem_cri_cnt),"
            " SUM(etc_hw_min_cnt),SUM(etc_hw_maj_cnt),SUM(etc_hw_cri_cnt),"
            " SUM(proc_min_cnt),SUM(proc_maj_cnt),SUM(proc_cri_cnt) "
            " from %s "
            " where system_name = '%s' AND stat_date > '%s' AND stat_date <= '%s'",
            STM_STATISTIC_5MINUTE_FAULT_TBL_NAME, SysName[i],
            (char *)get_ondemand_time(0 - ((onDEMAND[list].period-STAT_UNIT) * 60)), (char *)get_current_time() 
        );
//printf("jean ondemand ########\n%s\n======end\n", query);
        if ( trcLogFlag == TRCLEVEL_SQL ) {
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }

		logPrint(trcLogId,FL,"ondemand query: %s\n", query);

		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);

            onDEMAND[list].statisticsType = NOT_REGISTERED;
            sprintf(ondemandTmp, "    RESULT = FAIL\n    FAIL REASON = DB SELECT FAIL\n");
            strcat(ondemandErr, ondemandTmp);
            stmd_ondemand_txMMLResult (list, ondemandErr, -1, 0, 0, cmdName, 0, 1);
            return -1;
        }

        result = mysql_store_result(conn);
        row = mysql_fetch_row(result);
        if (row[0] == NULL) {
            sprintf(trcBuf, "Ondemand Fault select empty set\n");
            trclib_writeLogErr (FL, trcBuf);

            for(j=0;j<realItemCnt;j++){
                if (j==0){
                    sprintf(ondemandTmp, "%3s %-6s  %-12s %7s %8s %9s\n", 
                        "", SysName[i], title1[j], "0", "0", "0");
                    strcat(ondemandBuf, ondemandTmp);
                } else {
                    sprintf(ondemandTmp, "%3s %-6s  %-12s %7s %8s %9s\n", 
                        "", "", title1[j], "0", "0", "0");
                    strcat(ondemandBuf, ondemandTmp);
                }
            }

        } else {
            row_index = 0;
            for(j=0;j<realItemCnt;j++){
                if (j==0){
                    sprintf(ondemandTmp, "%3s %-6s  %-12s %7s %8s %9s\n", 
                        "", SysName[i], title1[j], row[row_index], row[row_index+1], row[row_index+2]);
                    strcat(ondemandBuf, ondemandTmp);
                } else {
                    sprintf(ondemandTmp, "%3s %-6s  %-12s %7s %8s %9s\n", 
                        "", "", title1[j], row[row_index], row[row_index+1], row[row_index+2]);
                    strcat(ondemandBuf, ondemandTmp);
                }

                row_index += 3;
            }
        }
        mysql_free_result(result);

    
        if ( !strcasecmp(SysName[i], "SCMA") || !strcasecmp(SysName[i], "SCMB") ){
#if 0
            sprintf(query, "SELECT"
                " IFNULL(SUM(dup_hb_min_cnt), 0), IFNULL(SUM(dup_hb_maj_cnt), 0), IFNULL(SUM(dup_hb_cri_cnt), 0),  "
                " IFNULL(SUM(dup_oos_min_cnt), 0), IFNULL(SUM(dup_oos_maj_cnt), 0), IFNULL(SUM(dup_oos_cri_cnt), 0),  "
                " IFNULL(SUM(succ_wap1ana_min_cnt), 0), IFNULL(SUM(succ_wap1ana_maj_cnt), 0), IFNULL(SUM(succ_wap1ana_cri_cnt), 0),"
                " IFNULL(SUM(succ_wap2ana_min_cnt), 0), IFNULL(SUM(succ_wap2ana_maj_cnt), 0), IFNULL(SUM(succ_wap2ana_cri_cnt), 0),  "
                " IFNULL(SUM(succ_httpana_min_cnt), 0), IFNULL(SUM(succ_httpana_maj_cnt), 0), IFNULL(SUM(succ_httpana_cri_cnt), 0),  "
                " IFNULL(SUM(succ_uawap_min_cnt), 0), IFNULL(SUM(succ_uawap_maj_cnt), 0), IFNULL(SUM(succ_uawap_cri_cnt), 0), "
                " IFNULL(SUM(succ_aaa_min_cnt), 0), IFNULL(SUM(succ_aaa_maj_cnt), 0), IFNULL(SUM(succ_aaa_cri_cnt), 0),  "
	        	" IFNULL(SUM(succ_vods_min_cnt), 0), IFNULL(SUM(succ_vods_maj_cnt), 0), IFNULL(SUM(succ_vods_cri_cnt), 0),  "
                " IFNULL(SUM(succ_anaaa_min_cnt), 0), IFNULL(SUM(succ_anaaa_maj_cnt), 0), IFNULL(SUM(succ_anaaa_cri_cnt), 0),  "
                " IFNULL(SUM(succ_vt_min_cnt), 0), IFNULL(SUM(succ_vt_maj_cnt), 0), IFNULL(SUM(succ_vt_cri_cnt), 0),  " 
                " IFNULL(SUM(succ_radius_min_cnt), 0), IFNULL(SUM(succ_radius_maj_cnt), 0), IFNULL(SUM(succ_radius_cri_cnt), 0),  " 
				" IFNULL(SUM(net_uawap_min_cnt), 0), IFNULL(SUM(net_uawap_maj_cnt), 0), IFNULL(SUM(net_uawap_cri_cnt), 0),"
                " IFNULL(SUM(net_aaa_min_cnt), 0), IFNULL(SUM(net_aaa_maj_cnt), 0), IFNULL(SUM(net_aaa_cri_cnt), 0),  "
                " IFNULL(SUM(sess_uawap_min_cnt), 0), IFNULL(SUM(sess_uawap_maj_cnt), 0), IFNULL(SUM(sess_uawap_cri_cnt), 0), "
                " IFNULL(SUM(sess_ntp_min_cnt), 0), IFNULL(SUM(sess_ntp_maj_cnt), 0), IFNULL(SUM(sess_ntp_cri_cnt), 0)  "
                " from %s "
                "where system_name = '%s' AND stat_date > '%s' AND stat_date <= '%s'",
                STM_STATISTIC_5MINUTE_BSD_FLT_TBL_NAME, SysName[i],
                get_ondemand_time(0 - ((onDEMAND[list].period-STAT_UNIT) * 60)), 
                get_current_time()
            );

			logPrint(trcLogId,FL,"ondemand SCMA SCMB query: %s\n", query);

            if ( trcLogFlag == TRCLEVEL_SQL ) {
                sprintf(trcBuf, "query = %s\n", query);
                trclib_writeLog(FL, trcBuf);
            }
			if (stmd_mysql_query (query) < 0) {
                sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
                trclib_writeLogErr (FL,trcBuf);

                onDEMAND[list].statisticsType = NOT_REGISTERED;
                sprintf(ondemandTmp, "    RESULT = FAIL\n    FAIL REASON = DB(HW_FLT) SELECT FAIL\n");
                strcat(ondemandErr, ondemandTmp);
                stmd_ondemand_txMMLResult (list, ondemandErr, -1, 0, 0, cmdName, 0, 1);
                return -1;
            }

            result = mysql_store_result(conn);
            row = mysql_fetch_row(result);
            if (row[0] == NULL) {
                sprintf(trcBuf, "Ondemand Flt select empty set\n");
                trclib_writeLogErr (FL, trcBuf);

                for(j=0;j<15;j++){
                    sprintf(ondemandTmp, "%3s %-6s  %-12s %7s %8s %9s\n", 
                        "", "", title2[j], "0", "0", "0");
                    strcat(ondemandBuf, ondemandTmp);
                }

            } else {
                row_index = 0;
                for(j=0;j<15;j++){
                    sprintf(ondemandTmp, "%3s %-6s  %-12s %7s %8s %9s\n", 
                        "", "", title2[j], row[row_index], row[row_index+1], row[row_index+2]);
                    strcat(ondemandBuf, ondemandTmp);
                    row_index += 3;
                }
            }
            mysql_free_result(result);
#endif
        }
        sprintf(ondemandTmp, "    ===============================================\n");
        strcat(ondemandBuf, ondemandTmp);
    }

    if ( onDEMAND[list].count == -1) {
        stmd_ondemand_txMMLResult (list, ondemandBuf, 0, 0, 0, cmdName, 0, 1);
        onDEMAND[list].statisticsType = NOT_REGISTERED;
    } else if ( onDEMAND[list].Txcount == onDEMAND[list].count) {
        stmd_ondemand_txMMLResult (list, ondemandBuf, 0, 0, 0, cmdName, 0, 1);
        onDEMAND[list].statisticsType = NOT_REGISTERED;
    } else {
        stmd_ondemand_txMMLResult (list, ondemandBuf, 0, 1, (onDEMAND[list].period*60)+10, cmdName, 0, 1);
        sprintf((char *)onDEMAND[list].measureTime, "%s", get_ondemand_time(onDEMAND[list].period*60));
    }
	return 0;
}

int sendResultMsg(char* msgBuf, int list, char* cmd, int period,int conFlag)
{
    char sndBuf[4096];
    int i, linecnt=0, oldpos=0;
    
    for(i=0; i<strlen(msgBuf); i++)
    {
        if(msgBuf[i] == '\n')
        {
            //printf("line find=%d\n",linecnt++);
            linecnt++;
            if(linecnt == 60)
            {
                memset(sndBuf, 0, sizeof(sndBuf));
                strncpy(sndBuf, &msgBuf[oldpos], i-oldpos);
                /*printf("%s\n",sndBuf);*/
                stmd_ondemand_txMMLResult (list, sndBuf, 0, 1, 5, cmd, 0, 1);
                linecnt = 0;
                oldpos = i+1;
            }
        }
        
        if( (i+1) == strlen(msgBuf))
        {
            memset(sndBuf, 0, sizeof(sndBuf));
            strncpy(sndBuf, &msgBuf[oldpos], i-oldpos);
            stmd_ondemand_txMMLResult (list, sndBuf, 0, conFlag, period, cmd, 0, 1);
            /*printf("%s\n",sndBuf);
            printf("*** Last Message\n"); */
        }
    }
    
    return 1;
}
