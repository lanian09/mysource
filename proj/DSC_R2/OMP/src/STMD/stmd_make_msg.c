#include "stmd_proto.h"

extern  char    trcBuf[4096], trcTmp[1024];
extern  int     trcFlag, trcLogFlag;
extern  int     sysCnt;
extern  STM_CommStatisticInfo   StatisticSystemInfo[SYSCONF_MAX_ASSO_SYS_NUM];
extern  MYSQL   sql, *conn;
extern  SFM_sfdb        *sfdb;

extern  OnDemandList    onDEMAND[MAX_ONDEMAND_NUM];
extern  CronList        cronJOB[MAX_CRONJOB_NUM];

/*
#define ONDEMANDJOB     0
#define CRONJOB         1
#define PERIODIC        2
*/

int makeMsgDb(int type, int list, int code, char* tbl_type, char* msgBuf, char* startTime, char* endTime, char* cmdName, IxpcQMsgType *rxIxpcMsg)
{
    char        query[4096], sysName[32];
    char        tmpBuf[1024],sndBuf[4096], tmpTime[32], stime[32],etime[32];
    MYSQL_RES   *result, *result1,*result2;
    MYSQL_ROW   row, row1, row2;
    int         rntCnt=0;
    char        seq=0;
    int         rowcnt=0, selcnt=0;
    
    memset(tmpTime, 0, sizeof(tmpTime));
    memset(stime, 0, sizeof(stime));
    memset(etime, 0, sizeof(etime));
    sprintf(stime,"%s:00",startTime);
    sprintf(etime,"%s:00",endTime);
    
    sprintf(query, "SELECT * FROM %s where (stat_date = '%s' or stat_date = '%s')",
                        tbl_type, startTime, endTime);
	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n", mysql_error(conn));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
    
    result = mysql_store_result(conn);
    rowcnt = mysql_num_rows(result);
//printf("makeMsgDb-> rowcnt=%d\n", rowcnt);
    if(rowcnt == 0)
    {
        mysql_free_result(result);
        sprintf(trcBuf,"DB make Messge Peroid is null : %s\n",query);
        trclib_writeLogErr (FL,trcBuf);
        return -2;
    }
    
    memset(sndBuf, 0, sizeof(sndBuf));
    strcpy(sndBuf, msgBuf);
    memset(msgBuf, 0, sizeof(msgBuf));
    
    /* Make Header */
    sprintf (tmpBuf,"\n\n  %7s %7s %7s %7s %7s %7s %7s %7s",
                "SYSTEM", "SELECT", "INSERT", "DELETE", "UPDATE", "NOT_FND","AL_EXIST","ACC_FAIL" );
    strcat(sndBuf, tmpBuf);  
    sprintf (tmpBuf, "\n  %s",
        "==================================================================");
    strcat(sndBuf, tmpBuf); 
        
    //result = mysql_store_result(conn);
    while( (row = mysql_fetch_row(result)) != NULL ) 
    {
//printf("DB date = %s\n", row[9]);
        if(tmpTime[0]== 0)
        {
            if(!strcasecmp(stime, row[9]))
            {
                sprintf(tmpTime, "%s", stime);
            }
            else if(!strcasecmp(etime, row[9]))
            {
                sprintf(tmpTime, "%s", etime);
            }
        }
        
        if( strcasecmp(tmpTime, row[9]) )
        {
            continue;
        }
        
        sprintf(sysName, "%s", row[0]);
        
        if(type == MMCJOB)
        {
            // 5분 데이타를 select
        sprintf(query, "SELECT "
                        "IFNULL(select_cnt,0), IFNULL(insert_cnt,0), IFNULL(delete_cnt,0), IFNULL(update_cnt,0), "
                        "IFNULL(notfound,0), IFNULL(alr_exist,0), IFNULL(acc_fail,0) "
                        " FROM %s WHERE (sys = '%s' and stat_date >= '%s' and stat_date <= '%s')", 
                        tbl_type,
                        sysName, startTime, endTime);
        }
        else
        {
        // 5분 데이타를 select
        sprintf(query, "SELECT "
                        "IFNULL(SUM(select_cnt),0), IFNULL(SUM(insert_cnt),0), IFNULL(SUM(delete_cnt),0), IFNULL(SUM(update_cnt),0), "
                        "IFNULL(SUM(notfound),0), IFNULL(SUM(alr_exist),0), IFNULL(SUM(acc_fail),0) "
                        " FROM %s WHERE (sys = '%s' and stat_date >= '%s' and stat_date <= '%s')", 
                        tbl_type,
                        sysName, startTime, endTime);
        }
/*printf("query :: %s\n",query); */
		if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf,">>> mysql_query fail; err=%s\n%s\n", mysql_error(conn), query);
            trclib_writeLogErr (FL,trcBuf);
            mysql_free_result(result);

            return -1;
        }
        
        result1 = mysql_store_result(conn);
        //row1 = mysql_fetch_row(result1);
        while( (row1 = mysql_fetch_row(result1)) != NULL ) 
        {
            sprintf(tmpBuf,"\n %7s %7s %7s %7s %7s %7s %7s %7s",
                        sysName, row1[0],row1[1],row1[2],row1[3],row1[4],row1[5],row1[6] );
            if(sndBuf[0]==0)
            {
                sprintf(sndBuf, "%s", tmpBuf);
            }
            else
            {
                strcat(sndBuf, tmpBuf);
            }
            
            // Size Check & Send 
            if(strlen(sndBuf) > 3000)
            {
                /* Periodic file Save*/
                if(type == PERIODIC)
                {
                    if(msgBuf[0] == 0)
                        sprintf(msgBuf, "%s", sndBuf);
                    else
                        strcat(msgBuf, sndBuf);
                }
                
                SendMsg(type, list, code, sndBuf, cmdName, 1, 1, seq++, rxIxpcMsg);
                memset(sndBuf, 0, sizeof(sndBuf));
            }
            selcnt++;
        }
        
        if (selcnt == 0) 
        {
            sprintf(trcBuf, "[%d]DB - makeMsgDb select ia null\n%s\n",type,query);
            trclib_writeLogErr (FL, trcBuf);
            mysql_free_result(result);
            mysql_free_result(result1);
printf("%s",trcBuf);
            return -2;
        }
        
        rntCnt++;
        mysql_free_result(result1); 
    }

    mysql_free_result(result);
    
    /* no Result Check */
    //if((type == ONDEMANDJOB || type == CRONJOB) && rntCnt == 0)
    if(rntCnt == 0)
    {
        sprintf(trcBuf,"DB make Messge is null : %s\n",query);
        trclib_writeLogErr (FL,trcBuf);
        
        return -2;
    }
    
    /* Total value */
    sprintf(query, "SELECT "
                    "IFNULL(SUM(select_cnt),0), IFNULL(SUM(insert_cnt),0), IFNULL(SUM(delete_cnt),0), IFNULL(SUM(update_cnt),0), "
                    "IFNULL(SUM(notfound),0), IFNULL(SUM(alr_exist),0), IFNULL(SUM(acc_fail),0) "                   
                    " FROM %s WHERE (stat_date >= '%s' and stat_date <= '%s')", 
                    tbl_type, startTime, endTime);

	if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf,">>> mysql_query fail; err=%s\n%s\n", mysql_error(conn), query);
        trclib_writeLogErr (FL,trcBuf);
        mysql_free_result(result);
        return 0;
    }
    
    result2 = mysql_store_result(conn);
    row2 = mysql_fetch_row(result2);
    if(row2[0] == NULL)
    {
        mysql_free_result(result2);
        return 0;
    }
    
    sprintf (tmpBuf,"\n  %s",
            "------------------------------------------------------------------");
    strcat(sndBuf, tmpBuf);
        
    sprintf(tmpBuf,"\n %7s %7s %7s %7s %7s %7s %7s %7s",
                    "TOT",row2[0],row2[1],row2[2],row2[3],row2[4],row2[5],row2[6] );
    strcat(sndBuf, tmpBuf); 
    
    sprintf (tmpBuf,"\n  %s \n",
            "==================================================================");
    strcat(sndBuf, tmpBuf);

    sprintf(tmpBuf, "    COMPLETED\n\n\n");
    strcat(sndBuf, tmpBuf);
    SendMsg(type, list, code, sndBuf, cmdName, 0,0, seq++, rxIxpcMsg);
    
    /* Periodic file Save*/
    if(type == PERIODIC)
    {
        if(msgBuf[0] == 0)
            strcpy(msgBuf, sndBuf);
        else
            strcat(msgBuf, sndBuf);
    }
    
    mysql_free_result(result);
    
    return 1;
}


float rate ( long suc, long tot )
{
    if ( tot <= 0 ) return 0.0;

    return (float)suc/tot*100;
}
/*
#define ONDEMANDJOB     0
#define CRONJOB         1
#define PERIODIC        2
*/

int SendMsg(int type, int list, int code, char* buf, char *cmdName,char contFlag, char segFlag, char seqNo, IxpcQMsgType *rxIxpcMsg)
{
    printf("type=%d\n",type);
    if(type == ONDEMANDJOB)
    {
        if(contFlag == 1)
        {
            stmd_ondemand_txMMLResult(list, buf, 0, contFlag, 5, cmdName, segFlag, seqNo );
        }
        else //contFlag == 0
        {
            if( onDEMAND[list].Txcount == onDEMAND[list].count) 
            {
                stmd_ondemand_txMMLResult (list, buf, 0, 0, 0, cmdName, 0, 1);
                onDEMAND[list].statisticsType = NOT_REGISTERED;
            } 
            else 
            {
                stmd_ondemand_txMMLResult (list, buf, 0, 1, (onDEMAND[list].period*60)+10, cmdName, 0, 1);
                sprintf((char *)onDEMAND[list].measureTime, "%s", (char *)get_ondemand_time(onDEMAND[list].period*60));
            }
        }   
        //stmd_ondemand_txMMLResult (list, ondemandBuf, 0, 0, 0, cmdName, 0, 1);
        //stmd_ondemand_txMMLResult (int list, char *resBuf, char resCode, char contFlag, unsigned short extendTime, char *cmdName, char segFlag, char seqNo)
    }
    else if(type == CRONJOB)
    {
        stmd_cron_txMsg2Cond(buf, code, segFlag,seqNo);
        //stmd_cron_txMsg2Cond (cronBuf, (STSCODE_STM_SCHEDULE_LOAD - STSCODE_TO_MSGID_STATISTICS), 0, 1);
        //stmd_cron_txMsg2Cond(char *buff, int msgId, char segFlag, char seqNo)
    }
    else if(type == PERIODIC)
    {
        stmd_txMsg2Cond(buf, code, segFlag,seqNo);
        //stmd_txMsg2Cond(condBuf, (sts_code - STSCODE_TO_MSGID_STATISTICS), 0, 1);
        //stmd_txMsg2Cond(char *buff, int msgId, char segFlag, char seqNo)
    }
    else if(type == MMCJOB)
    {
        if(contFlag == 1)
        {
            stmd_txMMLResult (rxIxpcMsg, buf, 0, contFlag, 5, segFlag, seqNo);
        }
        else
        {
            stmd_txMMLResult (rxIxpcMsg, buf, 0, contFlag, 0, segFlag, seqNo);
        }
    }
    
    return 1;
}

