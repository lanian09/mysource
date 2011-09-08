/* 기존에 통계는 통계 수집 시작 시간을 insert time으로 삼았지만...*/
/* 운영자 요청으로 통계 수집 완료 시간을 insert time으로 한다. */
/* by helca 2007.05.22*/

#include "stmd_proto.h"

extern  CronList    	cronJOB[MAX_CRONJOB_NUM];
extern  char        	trcBuf[4096], trcTmp[1024];
extern  int         	trcFlag, trcLogFlag, trcLogId;
extern  int         	sysCnt;
extern  char        	strITEM[STMD_MASK_ITEM_NUM][14];
extern  MYSQL       	sql, *conn;
extern  STM_CommStatisticInfo   StatisticSystemInfo[SYSCONF_MAX_ASSO_SYS_NUM];
extern	char	rsFname[32];

char    cronBuf[4096*4], cronHead1[4096], cronHead2[4096], cronTmp[4096];
extern SCE_t                g_stSCE[2];
extern int					SCE_CNT;

extern	RuleSetList g_stSCERule[MAX_RULE_NUM];
extern	int			g_ruleIdBuf[MAX_RULE_NUM];
extern	int			g_ruleItemCnt;

extern	RuleEntryList g_stSCEEntry[MAX_ENTRY_NUM];

extern PDSN_LIST         g_stPdsn;

int doCronJob() {
    int i, ret = 0;

    for ( i = 0; i < MAX_CRONJOB_NUM ; i++) 
    {
        // schedule 통계가 현재 시간보다 적으면 죽었다 살아난 것으로 판단하고 수행한다.
        if ( (cronJOB[i].statisticsType != NOT_REGISTERED) && 
            ((strcasecmp(cronJOB[i].measureTime, get_insert_time())) <= 0 ) ) 
        {
            if(trcLogId) {
                sprintf(trcBuf, " doCronJob[%s], cronTime:%s\n", 
                        strITEM[cronJOB[i].statisticsType], cronJOB[i].measureTime);
                trclib_writeLog(FL, trcBuf);
            }
            keepalivelib_increase();

            switch ( cronJOB[i].statisticsType) {
                case  STMD_FAULT:
					logPrint(trcLogId, FL, "doCronFault start\n"); // 2010. 12. 22
                    doCronFault(i);
					logPrint(trcLogId, FL, "doCronFault end\n"); // 2010. 12. 22
	            	keepalivelib_increase();		                    
		    		break;

                case  STMD_LOAD:
					logPrint(trcLogId, FL, "doCronLoad start\n"); // 2010. 12. 22
                    doCronLoad(i);
					logPrint(trcLogId, FL, "doCronLoad end\n"); // 2010. 12. 22
                    keepalivelib_increase(); 
		    		break;

                case  STMD_LINK:
					logPrint(trcLogId, FL, "doCronLink start\n"); // 2010. 12. 22
                    doCronLink(i);
					logPrint(trcLogId, FL, "doCronLink end\n"); // 2010. 12. 22
                    keepalivelib_increase(); 
		    		break;

                case  STMD_RULE_SET:
					logPrint(trcLogId, FL, "doCronRuleSet start\n"); // 2010. 12. 22
                    doCronRuleSet(i);
					logPrint(trcLogId, FL, "doCronRuleSet end\n"); // 2010. 12. 22
                    keepalivelib_increase(); 
		    		break;

                case  STMD_RULE_ENT:
					logPrint(trcLogId, FL, "doCronRuleEnt start\n"); // 2010. 12. 22
                    doCronRuleEnt(i);
					logPrint(trcLogId, FL, "doCronRuleEnt end\n"); // 2010. 12. 22
                    keepalivelib_increase(); 
		    		break;

                case  STMD_LEG:
					logPrint(trcLogId, FL, "doCronLeg start\n"); // 2010. 12. 22
                    doCronLeg(i);
					logPrint(trcLogId, FL, "doCronLeg end\n"); // 2010. 12. 22
                    keepalivelib_increase(); 
		    		break;

                case  STMD_LOGON:
					logPrint(trcLogId, FL, "doCronLogon start\n"); // 2010. 12. 22
                    doCronLogon(i);
					logPrint(trcLogId, FL, "doCronLogon end\n"); // 2010. 12. 22
                    keepalivelib_increase(); 
		    		break;

                case  STMD_FLOW:
					logPrint(trcLogId, FL, "doCronFlow start\n"); // 2010. 12. 22
                    ret = doCronFlow(i);
					logPrint(trcLogId, FL, "doCronFlow end\n"); // 2010. 12. 22
                    keepalivelib_increase(); 
		    		break;

                case  STMD_SMS:
                    doCronSms(i);
                    keepalivelib_increase(); 
#ifdef DELAY
		    		send_query_info (STAT_PERIOD_5MIN,\
                            get_ondemand_time2(0 - ((cronJOB[i].period-STAT_UNIT) * 60)),\
                            cronJOB[i].measureTime);
#else
					/*
		    		send_query_info (STAT_PERIOD_5MIN,
                            get_ondemand_time2(0 - ((cronJOB[i].period-STAT_UNIT) * 60)),
                            cronJOB[i].measureTime);
					*/
#endif
		    		break;


                case  STMD_DEL:
#ifdef DELAY
                    sprintf(trcBuf, "DELAY CRON statisticsType PASS OK\n");
                    trclib_writeLogErr(FL, trcBuf);
#else
                    doCronDel(i);
                    keepalivelib_increase(); 
		    		send_query_info (STAT_PERIOD_5MIN,\
                            get_ondemand_time2(0 - ((cronJOB[i].period-STAT_UNIT) * 60)),\
                            cronJOB[i].measureTime);

#endif
		    		break;

				default:
                    sprintf(trcBuf, "unexpected CRON statisticsType = %d\n", cronJOB[i].statisticsType);
                    trclib_writeLogErr(FL, trcBuf);
                    break;
            }
        }
    }
	return 0;
}

int doCronFlow(int list)
{
    char        query[4096] = {0,};
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int         i;
    int 		row_index; 
    char 		sysName[2][8] = {"SCEA","SCEB"};
    int 		realSysCnt =2;
    int 		row_cnt = 0;
	char title[4][16]={"SYSTEM", "AVG_FLOW", "MIN_FLOW","MAX_FLOW"};

    sprintf(cronBuf, "    %s %s\n    S%04d FLOW schedule statistics message\n",
        "SCM", 
        commlib_printTStamp(), // 현재시각 time stamp (년,월,일,시,분,초,요일)
        STSCODE_STM_SCHEDULE_FLOW);

    sprintf(cronTmp, "    PERIOD = %d(min)\n    MEASURETIME = %s  -  %s\n",
        cronJOB[list].period,
        get_ondemand_time(0 - ((cronJOB[list].period-STAT_UNIT) * 60)),
        get_current_time() 
    );
    strcat(cronBuf, cronTmp);

    sprintf(cronTmp, "    ======================================================================================\n");      
	strcat(cronBuf, cronTmp);
	sprintf(cronTmp, "%3s %8s %12s %12s %12s\n","",title[0],title[1],title[2],title[3] );
	strcat(cronBuf, cronTmp);
    sprintf(cronTmp, "    ======================================================================================\n");      
    strcat(cronBuf, cronTmp);                

    for(i = 0; i < realSysCnt; i++){
        if ( strcasecmp(cronJOB[list].sysName, "all") ) {
            if( strncmp(cronJOB[list].sysName, sysName[i], strlen(cronJOB[list].sysName)) )
                continue;
        }

		sprintf(query, "select system_name,  "
				" round(ifnull(avg(avg_flow),0),0), ifnull(min(min_flow),0), ifnull(max(max_flow),0) "
				" from %s "
                " where system_name = '%s' and stat_date >= '%s' and stat_date <= '%s'"
				" group by system_name ",
				STM_STATISTIC_5MINUTE_FLOW_TBL_NAME, sysName[i],
        		get_current_time(), get_current_time() 
//				get_ondemand_time2(0 - ((cronJOB[list].period-STAT_UNIT) * 60)), 
//				cronJOB[list].measureTime
        );
        
        if ( trcLogFlag == TRCLEVEL_SQL ){
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }
        if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);
            cronJOB[list].statisticsType = NOT_REGISTERED;
            return -1;
        }

		row_cnt = 0;
        result = mysql_store_result(conn);
        while( (row = mysql_fetch_row(result)) != NULL)
		{
			row_index = 1;
			sprintf(cronTmp, "%3s %8s %12s %12s %12s\n",
					"", row[0], row[row_index],row[row_index+1],row[row_index+2]);
			strcat(cronBuf, cronTmp);
			row_cnt++;
		}
		if( row_cnt == 0 )
		{
			sprintf(cronTmp, "%3s %8s %12s %12s %12s\n",
					"", sysName[i], "0","0","0");
			strcat(cronBuf, cronTmp);
		}
    	sprintf(cronTmp, "    ======================================================================================\n");      
		strcat(cronBuf, cronTmp);
        mysql_free_result(result);
    }
    sprintf(cronTmp, "    COMPLETED\n\n\n");
    strcat(cronBuf, cronTmp);
    stmd_cron_txMsg2Cond (cronBuf, (STSCODE_STM_SCHEDULE_FLOW - STSCODE_TO_MSGID_STATISTICS), 0, 1);
    sprintf((char *)cronJOB[list].measureTime, "%s", get_ondemand_time2(cronJOB[list].period*60));
        
	return 0;
}


int doCronLogon(int list)
{
    char        query[4096] = {0,};
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int         i, j, row_index, realSysCnt = 2, row_cnt = 0;
    char 		sysName[2][8] = {"SCMA","SCMB"};
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

    sprintf(cronBuf, "    %s %s\n    S%04d LOGON schedule statistics message\n",
        "SCM", 
        commlib_printTStamp(), // 현재시각 time stamp (년,월,일,시,분,초,요일)
        STSCODE_STM_SCHEDULE_LOGON);

    sprintf(cronTmp, "    PERIOD = %d(min)\n    MEASURETIME = %s  -  %s\n",
        cronJOB[list].period,
        get_ondemand_time(0 - ((cronJOB[list].period-STAT_UNIT) * 60)),
        get_current_time() 
    );
    strcat(cronBuf, cronTmp);

    sprintf(cronTmp, "    ======================================================================================\n");      
	strcat(cronBuf, cronTmp);
	sprintf(cronTmp, "%3s %8s %12s %12s\n","",title[0],title[1],title[2]);
	strcat(cronBuf, cronTmp);
	sprintf(cronTmp, "%3s %8s %12s %12s %12s %12s\n","","", title[3],title[4],title[5],title[6]); //uamyd 20110515 succrate_added
	strcat(cronBuf, cronTmp);
	sprintf(cronTmp, "%3s %8s %12s %12s %12s %12s\n", "","", title1[1], title1[2],title1[3],title1[4]);
	strcat(cronBuf, cronTmp);
	sprintf(cronTmp, "%3s %8s %12s %12s %12s %12s\n", "","", title2[1], title2[2],title2[3],title2[4]);
	strcat(cronBuf, cronTmp);
	sprintf(cronTmp, "%3s %8s %12s %12s %12s %12s\n", "","", title3[1], title3[2],title3[3],title3[4]);
	strcat(cronBuf, cronTmp);
	sprintf(cronTmp, "%3s %8s %12s %12s %12s %12s\n", "","", title4[1], title4[2],title4[3],title4[4]);
	strcat(cronBuf, cronTmp);
	sprintf(cronTmp, "%3s %8s %12s %12s %12s %12s\n", "","", title5[1], title5[2],title5[3],title5[4]);
	strcat(cronBuf, cronTmp);
	sprintf(cronTmp, "%3s %8s %12s %12s %12s %12s\n", "","", title6[1], title6[2],title6[3],title6[4]);
	strcat(cronBuf, cronTmp);
	sprintf(cronTmp, "%3s %8s %12s %12s %12s %12s\n", "","", title7[1], title7[2],title7[3],title7[4]);
	strcat(cronBuf, cronTmp);
	sprintf(cronTmp, "%3s %8s %12s %12s %12s %12s\n", "","", title8[1], title8[2],title8[3],title8[4]);
	strcat(cronBuf, cronTmp);
	sprintf(cronTmp, "%3s %8s %12s %12s %12s %12s\n", "","", title10[1], title10[2],title10[3],title10[4]);
	strcat(cronBuf, cronTmp);
	sprintf(cronTmp, "%3s %8s %12s %12s\n", "","", title11[1], title11[2]);
	strcat(cronBuf, cronTmp);
    sprintf(cronTmp, "    ======================================================================================\n");      
    strcat(cronBuf, cronTmp);                

    for(i = 0; i < realSysCnt; i++){
        if ( strcasecmp(cronJOB[list].sysName, "all") ) {
            if( strncmp(cronJOB[list].sysName, sysName[i], strlen(cronJOB[list].sysName)) )
                continue;
        }

		sprintf(query, "select system_name, sm_ch_id, log_mod, "
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
                " where system_name = '%s' and stat_date >= '%s' and stat_date <= '%s'"
				" group by system_name,sm_ch_id, log_mod",
				STM_STATISTIC_5MINUTE_LOGON_TBL_NAME, sysName[i],
        		get_current_time(), get_current_time() 
//				get_ondemand_time2(0 - ((cronJOB[list].period-STAT_UNIT) * 60)), 
//				cronJOB[list].measureTime
        );
        
        if ( trcLogFlag == TRCLEVEL_SQL ){
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }
		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);
            cronJOB[list].statisticsType = NOT_REGISTERED;
            return -1;
        }

		row_cnt = 0;
        result = mysql_store_result(conn);
        while( (row = mysql_fetch_row(result)) != NULL)
		{
			row_index = 1;
			if( row_cnt == 0 ){
			
				sprintf(cronTmp, "%3s %8s SM_CH%7s %12s\n",
						"", row[0], row[row_index], !atoi(row[row_index+1])?"LOG_ON":"LOG_OUT");
				strcat(cronBuf, cronTmp);
			} else {
				sprintf(cronTmp, "%3s %8s SM_CH%7s %12s\n",
						"", "", row[row_index],!atoi(row[row_index+1])?"LOG_ON":"LOG_OUT");
				strcat(cronBuf, cronTmp);
			}
			sprintf(cronTmp, "%3s %8s %12s %12s %12s %12.1f\n", 
					"", "", row[row_index+2],row[row_index+3],row[row_index+4],
					atoi(row[row_index+2])==0?0.0:(((float)atoi(row[row_index+3])/(float)atoi(row[row_index+2]))*100)); //uamyd 20110515 succrate_added
			strcat(cronBuf, cronTmp );

			for( j = 5; j<34; j+=4 ){
				//j=5,9,13,17,21,25,29,33
				sprintf(cronTmp, "%3s %8s %12s %12s %12s %12s\n", 
						"", "", row[row_index+j],row[row_index+j+1],row[row_index+j+2],row[row_index+j+3]);
				strcat(cronBuf, cronTmp );
			}
			sprintf(cronTmp, "%3s %8s %12s %12s %12s %12s\n", 
					"", "", row[row_index+37],row[row_index+38],row[row_index+39],row[row_index+40]);
			strcat(cronBuf, cronTmp);
			sprintf(cronTmp, "%3s %8s %12s %12s\n", 
					"", "", row[row_index+41],row[row_index+42]);
			strcat(cronBuf, cronTmp);
			row_cnt++;
		}

		if( row_cnt == 0 )
		{
			sprintf(cronTmp, "%3s %8s %12s %12s\n", "", sysName[i], "-","-");
			strcat(cronBuf, cronTmp);
			sprintf(cronTmp, "%3s %8s %12s %12s %12s %12s\n",
					"", "", "0","0","0","0"); //uamyd 20110515 succrate_added
			strcat(cronBuf, cronTmp);

			for(j=0;j<9;j++){
				sprintf(cronTmp, "%3s %8s %12s %12s %12s %12s\n", 
						"", "", "0","0","0","0");
				strcat(cronBuf, cronTmp );
			}
			sprintf(cronTmp, "%3s %8s %12s %12s\n", 
					"", "", "0","0");
			strcat(cronBuf, cronTmp);
		}
    	sprintf(cronTmp, "    ======================================================================================\n");      
		strcat(cronBuf, cronTmp);
        mysql_free_result(result);
    }
    sprintf(cronTmp, "    COMPLETED\n\n\n");
    strcat(cronBuf, cronTmp);
    stmd_cron_txMsg2Cond (cronBuf, (STSCODE_STM_SCHEDULE_LOGON - STSCODE_TO_MSGID_STATISTICS), 0, 1);
    sprintf((char *)cronJOB[list].measureTime, "%s", get_ondemand_time2(cronJOB[list].period*60));
        
	return 0;
}

int doCronDel(int list)
{
    char        query[4096];
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int         i,j;
    int 		row_index;
    char 		sysName[2][8] = {"SCE","SCMB"};
    int 		realSysCnt =1;
    int 		row_cnt = 0;
	char title[4][16]={"SYSTEM", "MIN_msec", "MAX_msec","AVG_msec"};

    sprintf(cronBuf, "    %s %s\n    S%04d DELAY  SCHEDULE STATISTICS MESSAGE\n",
        "SCE", 
        commlib_printTStamp(), // 현재시각 time stamp (년,월,일,시,분,초,요일)
        STSCODE_STM_SCHEDULE_DEL);

    sprintf(cronTmp, "    PERIOD = %d(min)\n    MEASURETIME = %s  -  %s\n",
        cronJOB[list].period,
        get_ondemand_time(0 - ((cronJOB[list].period-STAT_UNIT) * 60)),
        get_current_time() 
    );
    strcat(cronBuf, cronTmp);

    sprintf(cronTmp, "    ======================================================================================\n");      
	strcat(cronBuf, cronTmp);
	sprintf(cronTmp, "%3s %8s %12s %12s %12s\n","",title[0],title[1],title[2],title[3]);
	strcat(cronBuf, cronTmp);
    sprintf(cronTmp, "    ======================================================================================\n");      
    strcat(cronBuf, cronTmp);                

    for(i = 0; i < realSysCnt; i++){
        if ( strcasecmp(cronJOB[list].sysName, "all") ) {
            if( strncmp(cronJOB[list].sysName, sysName[i], strlen(cronJOB[list].sysName)) )
                continue;
        }

		sprintf(query, "select system_name, "
				" ifnull(ROUND(MIN(min_usec*1000),3), 0), ifnull(ROUND(MAX(max_usec*1000),3), 0), "
				" ifnull(ROUND(AVG(avg_usec*1000),3), 0) "
				" from %s "
                " where system_name = '%s' and stat_date >= '%s' and stat_date <= '%s'"
				" group by system_name ",
				STM_STATISTIC_5MINUTE_DELAY_TBL_NAME, sysName[i],
        		get_current_time(), get_current_time() 
//				get_ondemand_time2(0 - ((cronJOB[list].period-STAT_UNIT) * 60)), 
//				cronJOB[list].measureTime
        );
        
        if ( trcLogFlag == TRCLEVEL_SQL ) {
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }
		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);
            cronJOB[list].statisticsType = NOT_REGISTERED;
            return -1;
        }

		row_cnt = 0;
        result = mysql_store_result(conn);
        while( (row = mysql_fetch_row(result)) != NULL)
		{
			row_index = 1;
			if (row_cnt==0)
			{
				sprintf(cronTmp, "%3s %8s %-12s %-12s %-12s\n",
						"", row[0], row[row_index],row[row_index+1],row[row_index+2] );
				strcat(cronBuf, cronTmp );
			}
			else
			{
				sprintf(cronTmp, "%3s %8s %-12s %-12s %-12s\n",
						"", row[0], row[row_index],row[row_index+1],row[row_index+2]);
				strcat(cronBuf, cronTmp);
			}
			row_cnt++;
		}
		if( row_cnt == 0 )
		{
			if( j == 0 )
			{
				sprintf(cronTmp, "%3s %8s %-12s %-12s %-12s\n",
						"", sysName[i], "0.000", "0.000", "0.000");
				strcat(cronBuf, cronTmp);
			}
			else
			{
				sprintf(cronTmp, "%3s %8s %-12s %-12s %-12s\n",
						"", sysName[i], "0.000", "0.000", "0.000");
				strcat(cronBuf, cronTmp);
			}
		}
    	sprintf(cronTmp, "    ======================================================================================\n");      
		strcat(cronBuf, cronTmp);
        mysql_free_result(result);
    }
    sprintf(cronTmp, "    completed\n\n\n");
    strcat(cronBuf, cronTmp);
    stmd_cron_txMsg2Cond (cronBuf, (STSCODE_STM_SCHEDULE_DEL - STSCODE_TO_MSGID_STATISTICS), 0, 1);
    sprintf((char *)cronJOB[list].measureTime, "%s", get_ondemand_time2(cronJOB[list].period*60));

	return 0;
}

int doCronSms(int list)
{
    char        query[4096];
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int         i,j;
    int 		row_index;
    char 		sysName[2][8] = {"SCMA","SCMB"};
    int 		realSysCnt =2;
    int 		realItemCnt =2, row_cnt = 0;
	char title[5][16]={"SYSTEM", "SMSC_IP", "REQ", "SUCC","FAIL"};
	char title1[6][16]={"", "", "SMPP_ERR","SVR_ERR","SMSC_ERR","ETC_ERR"};

    sprintf(cronBuf, "    %s %s\n    S%04d SMS  SCHEDULE STATISTICS MESSAGE\n",
        "SCE", 
        commlib_printTStamp(), // 현재시각 time stamp (년,월,일,시,분,초,요일)
        STSCODE_STM_SCHEDULE_SMS);

    sprintf(cronTmp, "    PERIOD = %d(min)\n    MEASURETIME = %s  -  %s\n",
        cronJOB[list].period,
        get_ondemand_time(0 - ((cronJOB[list].period-STAT_UNIT) * 60)),
        get_current_time() 
    );
    strcat(cronBuf, cronTmp);

    sprintf(cronTmp, "    ======================================================================================\n");      
	strcat(cronBuf, cronTmp);
	sprintf(cronTmp, "%3s %8s %-18s %12s %12s %12s\n","",title[0],title[1],title[2],title[3],title[4]);
	strcat(cronBuf, cronTmp);
	sprintf(cronTmp, "%3s %8s %-18s %12s %12s %12s %12s\n", "","", "", title1[2],title1[3],title1[4],title1[5]);
	strcat(cronBuf, cronTmp);
    sprintf(cronTmp, "    ======================================================================================\n");      
    strcat(cronBuf, cronTmp);                

    for(i = 0; i < realSysCnt; i++){
        if ( strcasecmp(cronJOB[list].sysName, "all") ) {
            if( strncmp(cronJOB[list].sysName, sysName[i], strlen(cronJOB[list].sysName)) )
                continue;
        }

		// realItemCnt : smsc 개수 
		sprintf(query, "select count(*) from  ip_code_tbl where type = 2");

		if (stmd_mysql_query (query) < 0) {
			sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}
		result = mysql_store_result(conn);
		row = mysql_fetch_row(result);
		if( row == NULL )
		{
			realItemCnt = 0;
			mysql_free_result(result); // MEM if NULL 10.07
			continue;
			//mysql_free_result(result);                                                                             
		}
		else
		{
			if( row[0] == NULL )
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

		sprintf(query, "select system_name, smsc_ip, "
				" ifnull(sum(req), 0), ifnull(sum(succ), 0), ifnull(sum(fail), 0), "
				" ifnull(sum(smpp_err), 0), "
				" ifnull(sum(svr_err), 0), ifnull(sum(smsc_err), 0), "
				" ifnull(sum(etc_err), 0) "
				" from %s "
                " where system_name = '%s' and stat_date >= '%s' and stat_date <= '%s'"
				" group by system_name, smsc_ip ",
				STM_STATISTIC_5MINUTE_SMS_TBL_NAME, sysName[i],
        		get_current_time(), get_current_time() 
//				get_ondemand_time2(0 - ((cronJOB[list].period-STAT_UNIT) * 60)), 
//				cronJOB[list].measureTime
        );
        
        if ( trcLogFlag == TRCLEVEL_SQL ) {
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }
		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);
            cronJOB[list].statisticsType = NOT_REGISTERED;
            return -1;
        }

		row_cnt = 0;
        result = mysql_store_result(conn);
        while( (row = mysql_fetch_row(result)) != NULL)
		{
			row_index = 2;
			if (row_cnt==0)
			{
				sprintf(cronTmp, "%3s %8s %-18s %-12s %-12s %-12s\n",
						"", row[0], row[1], row[row_index],row[row_index+1],row[row_index+2] );
				strcat(cronBuf, cronTmp );
				sprintf(cronTmp, "%3s %8s %-18s %-12s %-12s %-12s %-12s\n",
						"", "", "", row[row_index+3],row[row_index+4],row[row_index+5],row[row_index+6]);
				strcat(cronBuf, cronTmp);
			}
			else
			{
				sprintf(cronTmp, "%3s %8s %-18s %-12s %-12s %-12s\n",
						"", "", row[1], row[row_index],row[row_index+1],row[row_index+2]);
				strcat(cronBuf, cronTmp);
				sprintf(cronTmp, "%3s %8s %-18s %-12s %-12s %-12s %-12s\n",
						"", "", "", row[row_index+3],row[row_index+4],row[row_index+5],row[row_index+6] );
				strcat(cronBuf, cronTmp);
			}
			row_cnt++;
		}
		if( row_cnt == 0 )
		{
			for( j = 0; j < realItemCnt; j++ )
			{
				if( j == 0 )
				{
					sprintf(cronTmp, "%3s %8s %-18s %-12s %-12s %-12s\n",
							"", sysName[i], "", "0", "0", "0");
					strcat(cronBuf, cronTmp);
					sprintf(cronTmp, "%3s %8s %-18s %-12s %-12s %-12s %-12s\n",
							"", "", "", "0", "0", "0","0" );
					strcat(cronBuf, cronTmp);
				}
				else
				{
					sprintf(cronTmp, "%3s %8s %-18s %-12s %-12s %-12s\n",
							"", "", "", "0", "0", "0");
					strcat(cronBuf, cronTmp);
					sprintf(cronTmp, "%3s %8s %-18s %-12s %-12s %-12s %-12s\n",
							"", "", "", "0", "0", "0","0" );
					strcat(cronBuf, cronTmp);
				}
			}
		}
    	sprintf(cronTmp, "    ======================================================================================\n");      
		strcat(cronBuf, cronTmp);
        mysql_free_result(result);
    }
    sprintf(cronTmp, "    completed\n\n\n");
    strcat(cronBuf, cronTmp);
    stmd_cron_txMsg2Cond (cronBuf, (STSCODE_STM_SCHEDULE_SMS - STSCODE_TO_MSGID_STATISTICS), 0, 1);
    sprintf((char *)cronJOB[list].measureTime, "%s", get_ondemand_time2(cronJOB[list].period*60));

	return 0;        
}


int doCronLeg(int list)
{
    char        query[4096];
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int         i,j;
    int 		row_index;
    char 		sysName[2][8] = {"SCMA","SCMB"};
    int 		realSysCnt =2;
    int 		realItemCnt =2, row_cnt = 0;
	char title[7][16]={"SYSTEM", "PDSN_IP", "RX_CNT", "START","INTERIM","DISCONNECT","STOP"};
	char title1[6][16]={"", "", "START_LOGON","INTERIM_LOGON","DISCONN_LOGON","LOG_OUT"};

    sprintf(cronBuf, "    %s %s\n    S%04d ACCOUNT SCHEDULE STATISTICS MESSAGE\n",
        "SCM", 
        commlib_printTStamp(), // 현재시각 time stamp (년,월,일,시,분,초,요일)
        STSCODE_STM_SCHEDULE_LEG);

    sprintf(cronTmp, "    PERIOD = %d(min)\n    MEASURETIME = %s  -  %s\n",
        cronJOB[list].period,
        get_ondemand_time(0 - ((cronJOB[list].period-STAT_UNIT) * 60)),
        get_current_time() 
    );
    strcat(cronBuf, cronTmp);

    sprintf(cronTmp, "    ======================================================================================\n");      
	strcat(cronBuf, cronTmp);
	sprintf(cronTmp, "%3s %8s %-18s %-12s %-12s %12s %12s %12s\n","",title[0],title[1],title[2],title[3],title[4],title[5],title[6] );
	strcat(cronBuf, cronTmp);
	sprintf(cronTmp, "%3s %8s %-18s %12s %12s %12s %12s\n", "","", "", title1[2],title1[3],title1[4],title1[5]);
	strcat(cronBuf, cronTmp);
    sprintf(cronTmp, "    ======================================================================================\n");      
    strcat(cronBuf, cronTmp);                

    for(i = 0; i < realSysCnt; i++){
        if ( strcasecmp(cronJOB[list].sysName, "all") ) {
            if( strncmp(cronJOB[list].sysName, sysName[i], strlen(cronJOB[list].sysName)) )
                continue;
        }

		// realItemCnt : pdsn 개수 
		sprintf(query, "select count(*) from ip_code_tbl where type = 1;");

		if (stmd_mysql_query (query) < 0) {
			sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}
		result = mysql_store_result(conn);
		row = mysql_fetch_row(result);
		if( row == NULL )
		{
			realItemCnt = 0;
			mysql_free_result(result); // MEM if NULL 10.07
			continue;
			//mysql_free_result(result);
		}
		else
		{
			if( row[1] == NULL )
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

		sprintf(query, "select system_name, pdsn_ip, "
				" ifnull(sum(rx_cnt), 0), ifnull(sum(start), 0), ifnull(sum(interim), 0), "
				" ifnull(sum(disconnect), 0), ifnull(sum(stop), 0), "
				" ifnull(sum(start_logon_cnt), 0), ifnull(sum(int_logon_cnt), 0), "
				" ifnull(sum(disc_logon_cnt), 0), ifnull(sum(logout_cnt), 0) "
				" from %s "
                " where system_name = '%s' and stat_date >= '%s' and stat_date <= '%s'"
				" group by system_name, pdsn_ip ",
				STM_STATISTIC_5MINUTE_LEG_TBL_NAME, sysName[i],
        		get_current_time(), get_current_time() 
//				get_ondemand_time2(0 - ((cronJOB[list].period-STAT_UNIT) * 60)), 
//				cronJOB[list].measureTime
        );
        
        if ( trcLogFlag == TRCLEVEL_SQL ) {
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }
		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);
            cronJOB[list].statisticsType = NOT_REGISTERED;
            return -1;
        }

		row_cnt = 0;
        result = mysql_store_result(conn);
        while( (row = mysql_fetch_row(result)) != NULL)
		{
			row_index = 2;
			if (row_cnt==0)
			{
				sprintf(cronTmp, "%3s %8s %18s %-12s %-12s %-12s %-12s %-12s\n",
						"", row[0], row[1], row[row_index],row[row_index+1],row[row_index+2],row[row_index+3],row[row_index+4] );
				strcat(cronBuf, cronTmp );
				sprintf(cronTmp, "%3s %8s %18s %-12s %-12s %-12s %-12s\n",
						"", "", "", row[row_index+5],row[row_index+6],row[row_index+7],row[row_index+8]);
				strcat(cronBuf, cronTmp);
			}
			else
			{
				sprintf(cronTmp, "%3s %8s %18s %-12s %-12s %-12s %-12s %-12s\n",
						"", "", row[1], row[row_index],row[row_index+1],row[row_index+2],row[row_index+3],row[row_index+4]);
				strcat(cronBuf, cronTmp);
				sprintf(cronTmp, "%3s %8s %18s %-12s %-12s %-12s %-12s\n",
						"", "", "", row[row_index+5],row[row_index+6],row[row_index+7],row[row_index+8] );
				strcat(cronBuf, cronTmp);
			}
			row_cnt++;
		}
		if( row_cnt == 0 )
		{
			for( j = 0; j < realItemCnt; j++ )
			{
				if( j == 0 )
				{
					sprintf(cronTmp, "%3s %8s %-18s %-12s %-12s %-12s %-12s %-12s\n",
							"", sysName[i], g_stPdsn.stItem[j].ip, "0", "0", "0", "0", "0");
					strcat(cronBuf, cronTmp);
					sprintf(cronTmp, "%3s %8s %-18s %-12s %-12s %-12s %-12s\n",
							"", "", "", "0", "0", "0", "0" );
					strcat(cronBuf, cronTmp);
				}
				else
				{
					sprintf(cronTmp, "%3s %8s %-18s %-12s %-12s %-12s %-12s %-12s\n",
							"", "", g_stPdsn.stItem[j].ip, "0", "0", "0", "0", "0");
					strcat(cronBuf, cronTmp);
					sprintf(cronTmp, "%3s %8s %-18s %-12s %-12s %-12s %-12s\n",
							"", "", "", "0", "0", "0", "0" );
					strcat(cronBuf, cronTmp);
				}
			}
		}
    	sprintf(cronTmp, "    ======================================================================================\n");      
		strcat(cronBuf, cronTmp);
        mysql_free_result(result);
    }
    sprintf(cronTmp, "    completed\n\n\n");
    strcat(cronBuf, cronTmp);
    stmd_cron_txMsg2Cond (cronBuf, (STSCODE_STM_SCHEDULE_LEG - STSCODE_TO_MSGID_STATISTICS), 0, 1);
    sprintf((char *)cronJOB[list].measureTime, "%s", get_ondemand_time2(cronJOB[list].period*60));
        
	return 0;
}

int doCronLink(int list)
{
    char        query[4096];
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int         i,j;
    int row_index;
    char sysName[2][8];
    int realSysCnt =0;
    int realItemCnt =0, row_cnt = 0;
	char title[5][16] = {"SYSTEM", "LINK", "UPSTREAM(Mbps)", "DNTREAM(Mbps)", "TOTAL(Mbps)"};
	char title1[5][16] = {"", "", "UPBYTE(Mbytes)", "DNBYTE(Mbytes)", "TOTAL(Mbytes)"};
	char	linkName[2][10] = {"LINK 1" , "LINK 2"};

    for(i=0; i < SCE_CNT; i++ )
    {
        sprintf(sysName[i],"%s", g_stSCE[i].sce_name);
        realSysCnt++;
    }

    sprintf(cronBuf, "    %s %s\n    S%04d LINK THRUPUT SCHEDULE STATISTICS MESSAGE\n",
        "SCE", 
        commlib_printTStamp(), // 현재시각 time stamp (년,월,일,시,분,초,요일)
        STSCODE_STM_SCHEDULE_LINK);

    sprintf(cronTmp, "    PERIOD = %d(MIN)\n    MEASURETIME = %s  -  %s\n",
        cronJOB[list].period,
        get_ondemand_time(0 - ((cronJOB[list].period-STAT_UNIT) * 60)),
        get_current_time() 
    );
    strcat(cronBuf, cronTmp);

    sprintf(cronTmp, "    ======================================================================================\n");      
    strcat(cronBuf, cronTmp);                                                              
    sprintf(cronTmp, "%3s %9s %-10s %-15s %-15s %-15s\n","",title[0],title[1],title[2],title[3],title[4]);          
    strcat(cronBuf, cronTmp);                                                              
    sprintf(cronTmp, "%3s %9s %-10s %-15s %-15s %-15s\n","","","",title1[2],title1[3],title1[4]);          
    strcat(cronBuf, cronTmp);                                                              
    sprintf(cronTmp, "    ======================================================================================\n");      
    strcat(cronBuf, cronTmp);                

    for(i = 0; i < realSysCnt; i++){
        if ( strcasecmp(cronJOB[list].sysName, "ALL") ) {
            if( strncmp(cronJOB[list].sysName, sysName[i], strlen(cronJOB[list].sysName)) )
                continue;
        }

		realItemCnt = 2;

        sprintf(query, "SELECT link_id,"
				" ROUND(SUM(upstream_volume)*8/1024/300,3), ROUND(SUM(downstream_volume)*8/1024/300,3), "
				" ROUND(SUM(upstream_volume+downstream_volume)*8/1024/300,3), "
				" ROUND(SUM(upstream_volume)/1024,3), ROUND(SUM(downstream_volume)/1024,3), "
				" ROUND(SUM(upstream_volume+downstream_volume)/1024,3) "
                " from %s "
                " where record_source = '%s' AND stat_date >= '%s' AND stat_date <= '%s'"
				" group by link_id order by stat_date, link_id",
            STM_STATISTIC_5MINUTE_LUR_TBL_NAME, sysName[i],
        	get_current_time(), get_current_time() 
//			get_ondemand_time2(0 - ((cronJOB[list].period-STAT_UNIT) * 60)), 
//			cronJOB[list].measureTime
        );
        
        if ( trcLogFlag == TRCLEVEL_SQL ) {
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }
		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);
            cronJOB[list].statisticsType = NOT_REGISTERED;
            return -1;
        }

//   4        10    5        9        10

		row_cnt = 0;
        result = mysql_store_result(conn);
        while( (row = mysql_fetch_row(result)) != NULL)
		{
			row_index = 1;
			if( row_cnt == 0 )
			{
				sprintf(cronTmp, "%3s %9s %-10s %-15s %-15s %-15s\n",
						"", sysName[i], linkName[row_cnt], row[row_index], row[row_index+1], row[row_index+2]);
				strcat(cronBuf, cronTmp);
				sprintf(cronTmp, "%3s %9s %-10s %-15s %-15s %-15s\n",
						"", "", "", row[row_index+3], row[row_index+4], row[row_index+5]);
				strcat(cronBuf, cronTmp);
			}
			else
			{
				sprintf(cronTmp, "%3s %9s %-10s %-15s %-15s %-15s\n",
						"", "", linkName[row_cnt], row[row_index], row[row_index+1], row[row_index+2]);
				strcat(cronBuf, cronTmp);
				sprintf(cronTmp, "%3s %9s %-10s %-15s %-15s %-15s\n",
						"", "", "", row[row_index+3], row[row_index+4], row[row_index+5]);
				strcat(cronBuf, cronTmp);
			}
			row_cnt++;
		}
		if( row_cnt == 0 )
		{
			for( j = 0; j < realItemCnt; j++ )
			{
				if( j == 0 )
				{
					sprintf(cronTmp, "%3s %9s %-10s %-15.3f %-15.3f %-15.3f\n",
							"", sysName[i], linkName[j], 0.0,0.0,0.0);
					strcat(cronBuf, cronTmp);
					sprintf(cronTmp, "%3s %9s %-10s %-15.3f %-15.3f %-15.3f\n",
							"", "", "", 0.0,0.0,0.0);
					strcat(cronBuf, cronTmp);
				}
				else
				{
					sprintf(cronTmp, "%3s %9s %-10s %-15.3f %-15.3f %-15.3f\n",
							"", "", linkName[j], 0.0,0.0,0.0);
					strcat(cronBuf, cronTmp);
					sprintf(cronTmp, "%3s %9s %-10s %-15.3f %-15.3f %-15.3f\n",
							"", "", "", 0.0,0.0,0.0);
					strcat(cronBuf, cronTmp);
				}
			}
		}
    	sprintf(cronTmp, "    ======================================================================================\n");      
		strcat(cronBuf, cronTmp);
        mysql_free_result(result);
    }
    sprintf(cronTmp, "    COMPLETED\n\n\n");
    strcat(cronBuf, cronTmp);
    stmd_cron_txMsg2Cond (cronBuf, (STSCODE_STM_SCHEDULE_LINK - STSCODE_TO_MSGID_STATISTICS), 0, 1);
    sprintf((char *)cronJOB[list].measureTime, "%s", get_ondemand_time2(cronJOB[list].period*60));
        
	return 0;
}

int doCronRuleEnt(int list)
{
    char        query[4096];
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int         i;
    int row_index;
    char SysName[2][8];
    int realSysCnt =0;
    int realItemCnt =0, row_cnt = 0, snd_cnt =1;
	char title[5][16] = {"SYSTEM", "ENTRY", "Session", "Block_Cnt"};
	char title1[5][16] = {"", "", "UpStream(Mbps)", "DnStream(Mbps)", "Total(Mbps)"};
	char title2[5][16] = {"", "", "UpByte(MBytes)", "DnByte(MBytes)", "Total(MBytes)"};

    sprintf(cronBuf, "    %s %s\n    S%04d RULE ENTRY SCHEDULE STATISTICS MESSAGE\n",
        "SCE", 
        commlib_printTStamp(), // 현재시각 time stamp (년,월,일,시,분,초,요일)
        STSCODE_STM_SCHEDULE_RULEENT);

    sprintf(cronTmp, "    PERIOD = %d(MIN)\n    MEASURETIME = %s  -  %s\n",
        cronJOB[list].period,
        get_ondemand_time(0 - ((cronJOB[list].period-STAT_UNIT) * 60)),
        get_current_time() 
    );
    strcat(cronBuf, cronTmp);

    sprintf(cronTmp, "    ======================================================================================\n");      
    strcat(cronBuf, cronTmp);                                                              
    sprintf(cronTmp, "%3s %6s %-38s %-12s %-12s\n","",title[0],title[1],title[2],title[3]);
    strcat(cronBuf, cronTmp);                                                              
    sprintf(cronTmp, "%3s %6s %-38s %-12s %-12s %-12s\n","","","",title1[2],title1[3],title1[4]);          
    strcat(cronBuf, cronTmp);                                                              
    sprintf(cronTmp, "%3s %6s %-38s %-12s %-12s %-12s\n","","","",title2[2],title2[3],title2[4]);          
    strcat(cronBuf, cronTmp);                                                              
    sprintf(cronTmp, "    ======================================================================================\n");      
    strcat(cronBuf, cronTmp);                

    for(i=0; i < SCE_CNT; i++ )
    {
        sprintf(SysName[i], "%s", g_stSCE[i].sce_name);
        realSysCnt++;
    }

	snd_cnt = 0;
    for(i = 0; i < realSysCnt; i++){
        if ( strcasecmp(cronJOB[list].sysName, "ALL") ) {
            if( strncmp(cronJOB[list].sysName, SysName[i], strlen(cronJOB[list].sysName)) )
                continue;
        }

		realItemCnt = g_stSCEEntry[i].ruleEntryCnt;

        sprintf(query, "SELECT rule_ent_id, rule_ent_name, "
				" SUM(session), SUM(block_cnt), "
				" ROUND(SUM(upstream_volume)*8/1024/1024/300,3), ROUND(SUM(downstream_volume)*8/1024/1024/300,3), "
				" ROUND(SUM(upstream_volume+downstream_volume)*8/1024/1024/300,3), "
				" ROUND(SUM(upstream_volume)/1024/1024,3), ROUND(SUM(downstream_volume)/1024/1024,3), "
				" ROUND(SUM(upstream_volume+downstream_volume)/1024/1024,3) "
                " from %s "
                " where record_source = '%s' AND stat_date >= '%s' AND stat_date <= '%s'"
				" group by rule_ent_id order by stat_date, rule_ent_id",
            STM_STATISTIC_5MINUTE_RULEENT_TBL_NAME, SysName[i],
        	get_current_time(), get_current_time() 
//			get_ondemand_time2(0 - ((cronJOB[list].period-STAT_UNIT) * 60)), 
//			cronJOB[list].measureTime
        );
        
        if ( trcLogFlag == TRCLEVEL_SQL ) {
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }
		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);
            cronJOB[list].statisticsType = NOT_REGISTERED;
            return -1;
        }

//   4        10    5        9        10

		row_cnt = 0;
        result = mysql_store_result(conn);
        while( (row = mysql_fetch_row(result)) != NULL)
		{
			row_index = 2;
			if( row_cnt == 0 )
			{
				sprintf(cronTmp, "%3s %6s %-38s %-12s %-12s\n",
						"", SysName[i], row[1], row[row_index], row[row_index+1]);
				strcat(cronBuf, cronTmp);
				sprintf(cronTmp, "%3s %6s %-38s %-12s %-12s %-12s\n",
						"", "", "", row[row_index+2], row[row_index+3], row[row_index+4]);
				strcat(cronBuf, cronTmp);
				sprintf(cronTmp, "%3s %6s %-38s %-12s %-12s %-12s\n",
						"", "", "", row[row_index+5], row[row_index+6], row[row_index+7]);
				strcat(cronBuf, cronTmp);
			}
			else
			{
				sprintf(cronTmp, "%3s %6s %-38s %-12s %-12s\n",
						"", "", row[1], row[row_index], row[row_index+1]);
				strcat(cronBuf, cronTmp);
				sprintf(cronTmp, "%3s %6s %-38s %-12s %-12s %-12s\n",
						"", "", "", row[row_index+2], row[row_index+3], row[row_index+4]);
				strcat(cronBuf, cronTmp);
				sprintf(cronTmp, "%3s %6s %-38s %-12s %-12s %-12s\n",
						"", "", "", row[row_index+5], row[row_index+6], row[row_index+7]);
				strcat(cronBuf, cronTmp);
			}
			if(strlen(cronBuf) > 3000) { 
				stmd_cron_txMsg2Cond (cronBuf, (STSCODE_STM_SCHEDULE_RULEENT - STSCODE_TO_MSGID_STATISTICS), 1, snd_cnt++);
				memset(cronBuf, 0x00, sizeof(cronBuf));
//				sprintf(cronBuf, "");    
			}
			row_cnt++;
		}
        mysql_free_result(result);

		if( row_cnt == 0 )
			sprintf(cronTmp, "   NO DATA " );

    	sprintf(cronTmp, "    ======================================================================================\n");      
		strcat(cronBuf, cronTmp);

    }
    sprintf(cronTmp, "    COMPLETED\n\n\n");
    strcat(cronBuf, cronTmp);
    stmd_cron_txMsg2Cond (cronBuf, (STSCODE_STM_SCHEDULE_RULEENT - STSCODE_TO_MSGID_STATISTICS), 0, snd_cnt);
    sprintf((char *)cronJOB[list].measureTime, "%s", get_ondemand_time2(cronJOB[list].period*60));
        
	return 0;
}

int doCronRuleSet(int list)
{
    char        query[4096] = {0,};
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int         i,j, snd_cnt = 1;
    int row_index, index;
    char SysName[2][8];
    int realSysCnt =0;
    int realItemCnt =0, row_cnt = 0;
	char title[5][16] = {"SYSTEM", "RULE", "Session", "Block_Cnt"};
	char title1[5][16] = {"", "", "UpStream(Mbps)", "DnStream(Mbps)", "Total(Mbps)"};
	char title2[5][16] = {"", "", "UpByte(MBytes)", "DnByte(MBytes)", "Total(MBytes)"};

    sprintf(cronBuf, "    %s %s\n    S%04d RULESET SCHEDULE STATISTICS MESSAGE\n",
        "SCE", 
        commlib_printTStamp(), // 현재시각 time stamp (년,월,일,시,분,초,요일)
        STSCODE_STM_SCHEDULE_RULESET);

    sprintf(cronTmp, "    PERIOD = %d(MIN)\n    MEASURETIME = %s  -  %s\n",
        cronJOB[list].period,
        get_ondemand_time(0 - ((cronJOB[list].period-STAT_UNIT) * 60)),
        get_current_time() 
    );
    strcat(cronBuf, cronTmp);

    sprintf(cronTmp, "    ======================================================================================\n");      
    strcat(cronBuf, cronTmp);                                                              
    sprintf(cronTmp, "%3s %6s %-38s %-12s %-12s\n","",title[0],title[1],title[2],title[3]);
    strcat(cronBuf, cronTmp);                                                              
    sprintf(cronTmp, "%3s %6s %-38s %-12s %-12s %-12s\n","","","",title1[2],title1[3],title1[4]);          
    strcat(cronBuf, cronTmp);                                                              
    sprintf(cronTmp, "%3s %6s %-38s %-12s %-12s %-12s\n","","","",title2[2],title2[3],title2[4]);          
    strcat(cronBuf, cronTmp);                                                              
    sprintf(cronTmp, "    ======================================================================================\n");      
    strcat(cronBuf, cronTmp);                

    for(i=0; i < SCE_CNT; i++ )
    {
        sprintf(SysName[i], "%s", g_stSCE[i].sce_name);
        realSysCnt++;
    }

    for(i = 0; i < realSysCnt; i++){
        if ( strcasecmp(cronJOB[list].sysName, "ALL") ) {
            if( strncmp(cronJOB[list].sysName, SysName[i], strlen(cronJOB[list].sysName)) )
                continue;
        }

		realItemCnt = g_ruleItemCnt;

        sprintf(query, "SELECT rule_set_id, rule_set_name, "
				" SUM(session), SUM(block_cnt), "
				" ROUND(SUM(upstream_volume)*8/1024/1024/300,3), ROUND(SUM(downstream_volume)*8/1024/1024/300,3), "
				" ROUND(SUM(upstream_volume+downstream_volume)*8/1024/1024/300,3), "
				" ROUND(SUM(upstream_volume)/1024/1024,3), ROUND(SUM(downstream_volume)/1024/1024,3), "
				" ROUND(SUM(upstream_volume+downstream_volume)/1024/1024,3) "
                " from %s "
                " where record_source = '%s' AND stat_date >= '%s' AND stat_date <= '%s'"
				" group by rule_set_id order by stat_date, rule_set_id",
            STM_STATISTIC_5MINUTE_RULESET_TBL_NAME, SysName[i],
        	get_current_time(), get_current_time() 
//			get_ondemand_time2(0 - ((cronJOB[list].period-STAT_UNIT) * 60)), 
//			cronJOB[list].measureTime
        );
        
        if ( trcLogFlag == TRCLEVEL_SQL ) {
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }
		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);
            cronJOB[list].statisticsType = NOT_REGISTERED;
            return -1;
        }
// 2010. 12. 11
		logPrint(trcLogId, FL, "doRuleSet Query:%s\n", query);

		row_cnt = 0;
        result = mysql_store_result(conn);
        while( (row = mysql_fetch_row(result)) != NULL)
		{
			row_index = 2;
			if( row_cnt == 0 )
			{
				sprintf(cronTmp, "%3s %6s %-38s %-12s %-12s\n",
						"", SysName[i], row[1], row[row_index], row[row_index+1]);
				strcat(cronBuf, cronTmp);
				sprintf(cronTmp, "%3s %6s %-38s %-12s %-12s %-12s\n",
						"", "", "", row[row_index+2], row[row_index+3], row[row_index+4]);
				strcat(cronBuf, cronTmp);
				sprintf(cronTmp, "%3s %6s %-38s %-12s %-12s %-12s\n",
						"", "", "", row[row_index+5], row[row_index+6], row[row_index+7]);
				strcat(cronBuf, cronTmp);
				sprintf(cronTmp, "    --------------------------------------------------------------------------------------\n");      
				strcat(cronBuf, cronTmp);
			}
			else
			{
				sprintf(cronTmp, "%3s %6s %-38s %-12s %-12s\n",
						"", "", row[1], row[row_index], row[row_index+1]);
				strcat(cronBuf, cronTmp);
				sprintf(cronTmp, "%3s %6s %-38s %-12s %-12s %-12s\n",
						"", "", "", row[row_index+2], row[row_index+3], row[row_index+4]);
				strcat(cronBuf, cronTmp);
				sprintf(cronTmp, "%3s %6s %-38s %-12s %-12s %-12s\n",
						"", "", "", row[row_index+5], row[row_index+6], row[row_index+7]);
				strcat(cronBuf, cronTmp);
				sprintf(cronTmp, "    --------------------------------------------------------------------------------------\n");      
				strcat(cronBuf, cronTmp);
			}
// 2010. 12. 22
//			logPrint(trcLogId, FL, "CRONBUF [%d]th [LEN:%d:%s] \n", row_cnt, strlen(cronBuf), cronBuf);
			if(strlen(cronBuf) > 3000) { 
// 2010. 12. 22
				logPrint(trcLogId, FL, "BUF > 3000 SEND COND[LEN:%d]\n%s\n", strlen(cronBuf), cronBuf);
				stmd_cron_txMsg2Cond (cronBuf, (STSCODE_STM_SCHEDULE_RULEENT - STSCODE_TO_MSGID_STATISTICS), 1, snd_cnt++);
				memset(cronBuf, 0x00, sizeof(cronBuf));
			}
			row_cnt++;
		}
		if( row_cnt == 0 )
		{
// 2010. 12. 22
			logPrint(trcLogId, FL, "RULE SET ROW CNT ZERO\n");
			for( j = 0; j < realItemCnt; j++ )
			{
				index = g_ruleIdBuf[j];
				if( g_stSCERule[i].stRule[index].real == 1 )
				{
					if( j == 0 )
					{
						sprintf(cronTmp, "%3s %6s %-38s %-12d %-12d\n",
								"", SysName[i], g_stSCERule[i].stRule[index].rName, 0,0);
						strcat(cronBuf, cronTmp);
						sprintf(cronTmp, "%3s %6s %-38s %-12s %-12s %-12s\n",
								"", "", "", "0.000","0.000","0.000");
						strcat(cronBuf, cronTmp);
						sprintf(cronTmp, "%3s %6s %-38s %-12s %-12s %-12s\n",
								"", "", "", "0.000","0.000","0.000");
						strcat(cronBuf, cronTmp);
						sprintf(cronTmp, "    --------------------------------------------------------------------------------------\n");      
						strcat(cronBuf, cronTmp);
					}
					else
					{
						sprintf(cronTmp, "%3s %6s %-38s %-12d %-12d\n",
								"", "", g_stSCERule[i].stRule[index].rName, 0,0);
						strcat(cronBuf, cronTmp);
						sprintf(cronTmp, "%3s %6s %-38s %-12s %-12s %-12s\n",
								"", "", "", "0.000","0.000","0.000");
						strcat(cronBuf, cronTmp);
						sprintf(cronTmp, "%3s %6s %-38s %-12s %-12s %-12s\n",
								"", "", "", "0.000","0.000","0.000");
						strcat(cronBuf, cronTmp);
						sprintf(cronTmp, "    --------------------------------------------------------------------------------------\n");      
					}
				}
				// 2010. 12. 22
				if(strlen(cronBuf) > 3000) { 
					// 2010. 12. 22
					logPrint(trcLogId, FL, "[ZERO]BUF > 3000 SEND COND[LEN:%d]\n%s\n", strlen(cronBuf), cronBuf);
					stmd_cron_txMsg2Cond (cronBuf, (STSCODE_STM_SCHEDULE_RULEENT - STSCODE_TO_MSGID_STATISTICS), 1, snd_cnt++);
					memset(cronBuf, 0x00, sizeof(cronBuf));
					// 2010. 12. 22
					//logPrint(trcLogId, FL, "CRONBUF [%d]th [LEN:%d:%s] \n", row_cnt, strlen(cronBuf), cronBuf);
				}
			}
		}
    	sprintf(cronTmp, "    ======================================================================================\n");      
		strcat(cronBuf, cronTmp);
        mysql_free_result(result);
    }
    sprintf(cronTmp, "    COMPLETED\n\n\n");
    strcat(cronBuf, cronTmp);
// 2010. 12. 22
	logPrint(trcLogId, FL, "LAST RULESET SEND COND LEN[%d]\n%s\n", strlen(cronBuf), cronBuf);
    stmd_cron_txMsg2Cond (cronBuf, (STSCODE_STM_SCHEDULE_RULESET - STSCODE_TO_MSGID_STATISTICS), 0, snd_cnt);
    sprintf((char *)cronJOB[list].measureTime, "%s", get_ondemand_time2(cronJOB[list].period*60));
        
	return 0;
}

int doCronLoad(int list)
{
    char        query[4096];
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    char        itemRes[10][10];
    int         i,j;
    char *title[]={"CPU","MEM","DISK","MSGQ"};
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

    sprintf(cronBuf, "    %s %s\n    S%04d LOAD SCHEDULE STATISTICS MESSAGE\n",
        "OMP", 
        commlib_printTStamp(), // 현재시각 time stamp (년,월,일,시,분,초,요일)
        STSCODE_STM_SCHEDULE_LOAD);

    sprintf(cronTmp, "    PERIOD = %d(MIN)\n    MEASURETIME = %s  -  %s\n",
        cronJOB[list].period,
        get_ondemand_time(0 - ((cronJOB[list].period-STAT_UNIT) * 60)),
        get_current_time() 
    );
    strcat(cronBuf, cronTmp);

    sprintf(cronTmp, "    ==================================\n");
    strcat(cronBuf, cronTmp);
    sprintf(cronTmp, "    SYSTEM    ITEM    AVG(%%)    MAX(%%)\n");
    strcat(cronBuf, cronTmp);
    sprintf(cronTmp, "    ==================================\n");
    strcat(cronBuf, cronTmp);

    for(i = 0; i < realSysCnt; i++){
        if ( strcasecmp(cronJOB[list].sysName, "ALL") ) {
            if( strncmp(cronJOB[list].sysName, SysName[i], strlen(cronJOB[list].sysName)) )
                continue;
        }

        if(!strcasecmp(SysName[i], "DSCM"))
            realItemCnt=4;
/* 10.13 TAP REMOVE
        else if(!strcasecmp(SysName[i], "TAPA") || !strcasecmp(SysName[i], "TAPB"))
            realItemCnt=2;
*/
        else if(!strcasecmp(SysName[i], "SCEA") || !strcasecmp(SysName[i], "SCEB"))
            realItemCnt=3;
        else
            realItemCnt=4;



        /*sprintf(query, "SELECT "
                " AVG(avr_cpu0), MAX(max_cpu0)," 
                " AVG(avr_cpu1), MAX(max_cpu1)," 
                " AVG(avr_cpu2), MAX(max_cpu2),"
                " AVG(avr_cpu3), MAX(max_cpu3)," 
                " AVG(avr_memory), MAX(max_memory)," 
                " AVG(avr_disk), MAX(max_disk)," 
                " AVG(avr_msgQ), MAX(max_msgQ)," 
                " AVG(avr_sess), MAX(max_sess)" */
        sprintf(query, "SELECT "
                " AVG(avr_cpu0), MAX(max_cpu0)," 
                " AVG(avr_memory), MAX(max_memory)," 
                " AVG(avr_disk), MAX(max_disk)," 
                " AVG(avr_msgQ), MAX(max_msgQ) " 
//                " AVG(avr_sess), MAX(max_sess)" 
                " from %s "
                " where system_name = '%s' AND stat_date >= '%s' AND stat_date <= '%s'",
            STM_STATISTIC_5MINUTE_LOAD_TBL_NAME, SysName[i],
        	get_current_time(), get_current_time() 
//			get_ondemand_time2(0 - ((cronJOB[list].period-STAT_UNIT) * 60)), 
//			cronJOB[list].measureTime
        );
        
        if ( trcLogFlag == TRCLEVEL_SQL ) {
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }
		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);
            cronJOB[list].statisticsType = NOT_REGISTERED;
            return -1;
        }

//   4        10    5        9        10
        result = mysql_store_result(conn);
        row = mysql_fetch_row(result);
        if (row[0] == NULL) {
            
            for(j=0;j<realItemCnt;j++){
                if (j==0){
                    sprintf(cronTmp, "%3s %-9s %-5s %8s %9s\n", 
                        "", SysName[i], title[j], "0.0", "0.0");
                    strcat(cronBuf, cronTmp);
                } else {
                    sprintf(cronTmp, "%3s %-9s %-5s %8s %9s\n", 
                        "", "", title[j], "0.0", "0.0");
                    strcat(cronBuf, cronTmp);
                }
            }
            sprintf(cronTmp, "    ==================================\n");
            strcat(cronBuf, cronTmp);

            sprintf(trcBuf, "cron Load select empty set\n");
			trclib_writeLog (FL, trcBuf);
        } else {

			for(j=0;j<8;j++){ // SESS는 그대로 출력 하자. add by helca 080930
				sprintf(itemRes[j], "%s", row[j]);
				sprintf(itemRes[j], "%d.%d", atoi(itemRes[j])/10, atoi(itemRes[j])%10); 
            }

            row_index = 0;
            for(j=0;j<realItemCnt;j++){
                if (j==0){
                    sprintf(cronTmp, "%3s %-9s %-5s %8s %9s\n", 
                        "", SysName[i], title[j], itemRes[row_index],itemRes[row_index+1]);
                    strcat(cronBuf, cronTmp);
                } else {
                    sprintf(cronTmp, "%3s %-9s %-5s %8s %9s\n", 
                        "", "", title[j], itemRes[row_index],itemRes[row_index+1]);
                    strcat(cronBuf, cronTmp);
                }

                row_index += 2;
            }
            sprintf(cronTmp, "    ==================================\n");
            strcat(cronBuf, cronTmp);
        }
        mysql_free_result(result);
    }
    sprintf(cronTmp, "    COMPLETED\n\n\n");
    strcat(cronBuf, cronTmp);
    stmd_cron_txMsg2Cond (cronBuf, (STSCODE_STM_SCHEDULE_LOAD - STSCODE_TO_MSGID_STATISTICS), 0, 1);
    sprintf((char *)cronJOB[list].measureTime, "%s", get_ondemand_time2(cronJOB[list].period*60));
        
	return 0;
}

int doCronFault(int list)
{
    char        query[4096];
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    int         i,j;
    char *title1[]={"CPU","MEM","HW","PROC"};
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
    sprintf(SysName[realSysCnt], "SCEA");realSysCnt++;
    sprintf(SysName[realSysCnt], "SCEB");realSysCnt++;


    sprintf(cronBuf, "    %s %s\n    S%04d FAULT SCHEDULE STATISTICS MESSAGE\n",
        "OMP", 
        commlib_printTStamp(), // 현재시각 time stamp (년,월,일,시,분,초,요일)
        STSCODE_STM_SCHEDULE_FAULT);

    sprintf(cronTmp, "    PERIOD = %d(MIN)\n    MEASURETIME = %s  -  %s\n",
        cronJOB[list].period,
        get_ondemand_time(0 - ((cronJOB[list].period-STAT_UNIT) * 60)),
        get_current_time()
    );
    strcat(cronBuf, cronTmp);

    sprintf(cronTmp, "    ===============================================\n");
    strcat(cronBuf, cronTmp);
    sprintf(cronTmp, "    SYSTEM  ITEM           MINOR    MAJOR  CRITICAL\n");
    strcat(cronBuf, cronTmp);
    sprintf(cronTmp, "    ===============================================\n");
    strcat(cronBuf, cronTmp);

    for ( i=0 ; i < realSysCnt; i++) 
    {
        if ( strcasecmp(cronJOB[list].sysName, "ALL") ) {
            if( strncmp(cronJOB[list].sysName, SysName[i], strlen(cronJOB[list].sysName)) )
                continue;
        }

        if(!strcasecmp(SysName[i], "DSCM"))
            realItemCnt=4;
        else if(!strcasecmp(SysName[i], "TAPA") || !strcasecmp(SysName[i], "TAPB")) 
            realItemCnt=3;
        else if( !strcasecmp(SysName[i], "SCEA") || !strcasecmp(SysName[i], "SCEB"))
            realItemCnt=3;
        else
            realItemCnt=4;

        
        sprintf(query, "SELECT"
            " SUM(cpu_min_cnt),SUM(cpu_maj_cnt),SUM(cpu_cri_cnt),"
            " SUM(mem_min_cnt),SUM(mem_maj_cnt),SUM(mem_cri_cnt),"
            " SUM(etc_hw_min_cnt),SUM(etc_hw_maj_cnt),SUM(etc_hw_cri_cnt),"
//            " SUM(cpu_min_cnt),SUM(mem_min_cnt),SUM(etc_hw_min_cnt),"
//            " SUM(cpu_maj_cnt),SUM(mem_maj_cnt),SUM(etc_hw_maj_cnt),"
//            " SUM(cpu_cri_cnt),SUM(mem_cri_cnt),SUM(etc_hw_cri_cnt),"
            " SUM(proc_min_cnt), SUM(proc_maj_cnt), SUM(proc_cri_cnt)"
//            " SUM(net_nms_min_cnt), SUM(net_nms_maj_cnt), SUM(net_nms_cri_cnt),"
//            " SUM(sess_ntp_min_cnt), SUM(sess_ntp_maj_cnt), SUM(sess_ntp_cri_cnt),"
//            " SUM(sess_nms_min_cnt), SUM(sess_nms_maj_cnt), SUM(sess_nms_cri_cnt) "
            " from %s "
            "where system_name = '%s' AND stat_date >= '%s' AND stat_date <= '%s'",
            STM_STATISTIC_5MINUTE_FAULT_TBL_NAME, SysName[i],
        	get_current_time(), get_current_time() 
//			get_ondemand_time2(0 - ((cronJOB[list].period-STAT_UNIT) * 60)), 
//			cronJOB[list].measureTime
        );
        if ( trcLogFlag == TRCLEVEL_SQL ) {
            sprintf(trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }
		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
            trclib_writeLogErr (FL,trcBuf);
            cronJOB[list].statisticsType = NOT_REGISTERED;
            return -1;
        }

        result = mysql_store_result(conn);
        row = mysql_fetch_row(result);
//   4     6          12       8        9        10
        if (row[0] == NULL) 
        {
            for(j=0;j<realItemCnt;j++){
                if (j==0){
                    sprintf(cronTmp, "%3s %-6s  %-12s %7s %8s %9s\n", 
                        "", SysName[i], title1[j], "0", "0", "0");
                    strcat(cronBuf, cronTmp);
                } else {
                    sprintf(cronTmp, "%3s %-6s  %-12s %7s %8s %9s\n", 
                        "", "", title1[j], "0", "0", "0");
                    strcat(cronBuf, cronTmp);
                }
            }

            sprintf(trcBuf, "[%s] cron Fault select empty set\n",SysName[i]);
            trclib_writeLog (FL, trcBuf);
        } 
        else 
        {
            row_index = 0;
            for(j=0;j<realItemCnt;j++){
                if (j==0){
                    sprintf(cronTmp, "%3s %-6s  %-12s %7s %8s %9s\n", 
                        "", SysName[i], title1[j], row[row_index], row[row_index+1], row[row_index+2]);
                    strcat(cronBuf, cronTmp);
                } else {
                    sprintf(cronTmp, "%3s %-6s  %-12s %7s %8s %9s\n", 
                        "", "", title1[j], row[row_index], row[row_index+1], row[row_index+2]);
                    strcat(cronBuf, cronTmp);
                }

                row_index += 3;
            }

        }
        mysql_free_result(result);
    }


    sprintf(cronTmp, "    COMPLETED\n\n\n");
    strcat(cronBuf, cronTmp);

    stmd_cron_txMsg2Cond (cronBuf, (STSCODE_STM_SCHEDULE_FAULT - STSCODE_TO_MSGID_STATISTICS), 0, 1);
    sprintf((char *)cronJOB[list].measureTime, "%s", get_ondemand_time2(cronJOB[list].period*60));

	return 0;
}
