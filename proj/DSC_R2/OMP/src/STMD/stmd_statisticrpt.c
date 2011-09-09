/* 기존에 통계는 통계 수집 시작 시간을 insert time으로 삼았지만...*/
/* 운영자 요청으로 통계 수집 완료 시간을 insert time으로 한다. */
/* by helca 2007.05.22*/
#include "stmd_proto.h"

extern  char    trcBuf[4096], trcTmp[1024];
extern  char    mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN];
extern  int     fimdQid, trcLogFlag;
extern  int    max_sts_count;
extern  MYSQL   sql, *conn;
extern  int trcLogId;
extern PDSN_LIST		g_stPdsn;
extern SVC_ALM			g_stSvcAlm;
extern SVC_VAL			g_stSvcVal;
extern short   delTIME [STMD_PERIOD_TYPE_NUM];

int		LastSceMaxCpu[2][3];
int		LastSceAvgCpu[2][3];
int		LastSceMaxMem[2];
int		LastSceAvgMem[2];


int update_account (IxpcQMsgType *rxIxpcMsg)
{	
#if 0 //쓰이지 않음. noted by uamyd 20110424
	LEG_STAT   stLeg;
    char    query[4096];
//// 	char	sysName[2][10] = {"SCMA","SCMB"};
////	int		i, j;
//	int		j;

	struct in_addr pdsn_ip ;
	char    PDSN[24] = {0,};
	int		exist_stat[2] = {0, 0};

	int count;

	memset(exist_stat, 0x00, sizeof(exist_stat));
	memset(&stLeg, 0x00, sizeof(stLeg));

    // 현재의 데이타를 삽입한다.
    memcpy((void *)&stLeg, rxIxpcMsg->body, sizeof(LEG_STAT));


	/**< stLeg.nCount == ?? 얼마냐? nCount가 PDSN IP 갯수입니다.  
	  **/
	for( count=0; count < ntohl(stLeg.nCount); count++ )
	{	
		pdsn_ip.s_addr = stLeg.stPDSNStat[count].uiPDSN_IP;	
		sprintf(PDSN, "%s",inet_ntoa(pdsn_ip));

			sprintf(query, "UPDATE %s SET rx_cnt = %d, start = %d, interim = %d, disconnect = %d, "
					" stop = %d, start_logon_cnt = %d, int_logon_cnt = %d, disc_logon_cnt = %d, "
					" logout_cnt = %d "
					" where system_name = '%s' and pdsn_ip = '%s' and stat_date = '%s'",
					STM_STATISTIC_5MINUTE_LEG_TBL_NAME, 
					ntohl(stLeg.stPDSNStat[count].uiPDSN_RecvCnt),
					ntohl(stLeg.stPDSNStat[count].uiPDSN_StartCnt),
					ntohl(stLeg.stPDSNStat[count].uiPDSN_InterimCnt),
					ntohl(stLeg.stPDSNStat[count].uiPDSN_DiscReqCnt),
					ntohl(stLeg.stPDSNStat[count].uiPDSN_StopCnt),
					ntohl(stLeg.stPDSNStat[count].uiLogOn_StartCnt),
					ntohl(stLeg.stPDSNStat[count].uiLogOn_InterimCnt),
					ntohl(stLeg.stPDSNStat[count].uiLogOn_DiscReqCnt),
					ntohl(stLeg.stPDSNStat[count].uiLogOn_StopCnt),
					stLeg.szSysName,
					PDSN, get_insert_time());

		// SVC_ALM 을 위한 logon / logout 현재 값 저장 .
		if( !strncasecmp(stLeg.szSysName, "SCMA", 4) )
		{
			// 1 을 CURRENT define으로 0 을 PREV define으로 수정, 1은 EXIST로 0은 NOT EXIST로 수정 필요.
			g_stSvcVal.logon_A[1] += ntohl(stLeg.stPDSNStat[count].uiLogOn_StartCnt) + 
								ntohl(stLeg.stPDSNStat[count].uiLogOn_InterimCnt) + 
								ntohl(stLeg.stPDSNStat[count].uiLogOn_DiscReqCnt);
			g_stSvcVal.logout_A[1] += ntohl(stLeg.stPDSNStat[count].uiLogOn_StopCnt); 
			exist_stat[0] = 1;
		}
		else
		{
			g_stSvcVal.logon_B[1] += ntohl(stLeg.stPDSNStat[count].uiLogOn_StartCnt) + 
								ntohl(stLeg.stPDSNStat[count].uiLogOn_InterimCnt) +
								ntohl(stLeg.stPDSNStat[count].uiLogOn_DiscReqCnt);
			g_stSvcVal.logout_B[1] += ntohl(stLeg.stPDSNStat[count].uiLogOn_StopCnt); 
			exist_stat[1] = 1;
		}


		if (stmd_mysql_query (query) < 0) {
			sprintf(trcBuf, "query fail: query:%s, err = %s\n", query, mysql_error(conn));
			trclib_writeErr (FL,trcBuf);
			return -1;
		}
		logPrint(trcLogId, FL, "ACCOUNT UPDATE : %s <-- %s\n",query, stLeg.szSysName);

	}

	// SCMA, SCMB 의 현재 통계가 존재하지 않는 경우, 현재 값은 0

	if( !strncmp(stLeg.szSysName, "SCMA", 4) )
	{
		// exist_stat의 [0] 은 SCMA 로 , [1]은 SCMB 로 DEFINE 필요 , 
		if( exist_stat[0] == 0 ) // SCMA not exist
		{
			g_stSvcVal.logon_A[1] = 0;
			g_stSvcVal.logout_A[1] = 0;
		}
	}
	else
	{
		if( exist_stat[1] == 0 ) // SCMB not exist
		{
			g_stSvcVal.logon_B[1] = 0;
			g_stSvcVal.logout_B[1] = 0;
		}
	}

	if ( trcLogFlag == TRCLEVEL_SQL ) {
		sprintf( trcBuf, "query = %s\n", query);
		trclib_writeLog(FL, trcBuf);
	}
#endif 
	return 0;
}


int stmd_hdlRlegStatisticRpt(IxpcQMsgType *rxIxpcMsg)
{
	PLEG_TOT_STAT pstLegTot;
	PLEG_STAT     pstLeg;
    char          *sysname, query[4096], PDSN[24] = {0,};
	int		      j, count, exist_stat[2] = {0,0};

	struct in_addr pdsn_ip ;

	pstLegTot = (PLEG_TOT_STAT)&rxIxpcMsg->body;
	pstLeg    = &pstLegTot->stAcct;
	sysname   = &pstLegTot->szSysName[0]; 

	memset(exist_stat, 0x00, sizeof(exist_stat));

    // leg_loginout 테이블을 지운다. 
	/*
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
            STM_STATISTIC_5MINUTE_LEG_TBL_NAME, get_delete_time(STMD_1HOUR_OFFSET*7));
			*/
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
            STM_STATISTIC_5MINUTE_LEG_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_MIN]));
    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf( trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "leg_5minute mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
    }

	/** RLEG A, B 양쪽에서 data를 받게됨. 
	for( i	= 0; i < 2; i++ )
	{
		for( j = 0; j < g_stPdsn.pdsnCnt; j++ )
		{
			sprintf(query, "Insert into leg_5minute_statistics values "
					" ( '%s', '%s', 0, 0, 0, 0, 0, 0, 0, 1, '%s', '%s' ) ",
					sysName[i], g_stPdsn.stItem[j].ip, get_insert_time(), get_insert_week());

			if (stmd_mysql_query (query) < 0) {
				sprintf(trcBuf, "query fail: query:%s, err = %s\n", query, mysql_error(conn));
				trclib_writeErr (FL,trcBuf);
				return -1;
			}
		}
	}
	*/

	/**< Default 0를 insert 하는 부분.**/
	for( j = 0; j < g_stPdsn.pdsnCnt; j++ )		/**< pdsn cnt 는 2?? **/
	{
		sprintf(query, "Insert into leg_5minute_statistics values "
				" ( '%s', '%s', 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, '%s', '%s' ) ",
				sysname, g_stPdsn.stItem[j].ip, get_insert_time(), get_insert_week());

		if (stmd_mysql_query (query) < 0) {
			sprintf(trcBuf, "query fail: query:%s, err = %s\n", query, mysql_error(conn));
			trclib_writeErr (FL,trcBuf);
			return -1;
		}

	}

	/**< stLeg.nCount == ?? 얼마냐? nCount가 PDSN IP 갯수입니다.  
	  **/
#ifdef DEBUG
logPrint(trcLogId, FL, " @@@ received PDSN Cnt=%d\n", ntohl(pstLeg->uiCount));
#endif
	for( count=0; count < ntohl(pstLeg->uiCount); count++ )
	{	
		pdsn_ip.s_addr = pstLeg->stPDSNStat[count].uiPDSN_IP;	
		sprintf(PDSN, "%s",inet_ntoa(pdsn_ip));

		// 비교문이 필요없는데.. 바꿀 필요있음.
		/** 굳이 비교해서 update 할 필요없음. 
		if( !strncasecmp(stLeg.szSysName, "SCMA", 4) )
		{
			sprintf(query, "UPDATE %s SET rx_cnt = %d, start = %d, interim = %d, "
					" stop = %d, start_logon_cnt = %d, int_logon_cnt = %d, "
					" logout_cnt = %d "
					" where system_name = 'SCMA' and pdsn_ip = '%s' and stat_date = '%s'",
					STM_STATISTIC_5MINUTE_LEG_TBL_NAME, 
					ntohl(stLeg.stPDSNStat[count].uiPDSN_RecvCnt),
					ntohl(stLeg.stPDSNStat[count].uiPDSN_StartCnt),
					ntohl(stLeg.stPDSNStat[count].uiPDSN_InterimCnt),
					ntohl(stLeg.stPDSNStat[count].uiPDSN_StopCnt),
					ntohl(stLeg.stPDSNStat[count].uiLogOn_StartCnt),
					ntohl(stLeg.stPDSNStat[count].uiLogOn_InterimCnt),
					ntohl(stLeg.stPDSNStat[count].uiLogOn_StopCnt),
					PDSN, get_insert_time());
		}
		else
		{
			sprintf(query, "UPDATE %s SET rx_cnt = %d, start = %d, interim = %d, "
					" stop = %d, start_logon_cnt = %d, int_logon_cnt = %d, "
					" logout_cnt = %d "
					" where system_name = 'SCMB' and pdsn_ip = '%s' and stat_date = '%s'",
					STM_STATISTIC_5MINUTE_LEG_TBL_NAME, 
					ntohl(stLeg.stPDSNStat[count].uiPDSN_RecvCnt),
					ntohl(stLeg.stPDSNStat[count].uiPDSN_StartCnt),
					ntohl(stLeg.stPDSNStat[count].uiPDSN_InterimCnt),
					ntohl(stLeg.stPDSNStat[count].uiPDSN_StopCnt),
					ntohl(stLeg.stPDSNStat[count].uiLogOn_StartCnt),
					ntohl(stLeg.stPDSNStat[count].uiLogOn_InterimCnt),
					ntohl(stLeg.stPDSNStat[count].uiLogOn_StopCnt),
					PDSN, get_insert_time());
		}
		*/
		sprintf(query, "UPDATE %s SET rx_cnt = %d, start = %d, interim = %d, disconnect = %d, "
				" stop = %d, start_logon_cnt = %d, int_logon_cnt = %d, disc_logon_cnt = %d, "
				" logout_cnt = %d "
				" where system_name = '%s' and pdsn_ip = '%s' and stat_date = '%s'",
				STM_STATISTIC_5MINUTE_LEG_TBL_NAME, 
				ntohl(pstLeg->stPDSNStat[count].uiPDSN_RecvCnt),
				ntohl(pstLeg->stPDSNStat[count].uiPDSN_StartCnt),
				ntohl(pstLeg->stPDSNStat[count].uiPDSN_InterimCnt),
				ntohl(pstLeg->stPDSNStat[count].uiPDSN_DiscReqCnt),
				ntohl(pstLeg->stPDSNStat[count].uiPDSN_StopCnt),
				ntohl(pstLeg->stPDSNStat[count].uiLogOn_StartCnt),
				ntohl(pstLeg->stPDSNStat[count].uiLogOn_InterimCnt),
				ntohl(pstLeg->stPDSNStat[count].uiLogOn_DiscReqCnt),
				ntohl(pstLeg->stPDSNStat[count].uiLogOn_StopCnt),
				sysname,
				PDSN, get_insert_time());

		// SVC_ALM 을 위한 logon / logout 현재 값 저장 .
		if( !strncasecmp(sysname, "SCMA", 4) )
		{
			g_stSvcVal.logon_A[DEF_CURRENT] += ntohl(pstLeg->stPDSNStat[count].uiLogOn_StartCnt) + 
								ntohl(pstLeg->stPDSNStat[count].uiLogOn_InterimCnt) +
								ntohl(pstLeg->stPDSNStat[count].uiLogOn_DiscReqCnt);
			g_stSvcVal.logout_A[DEF_CURRENT] += ntohl(pstLeg->stPDSNStat[count].uiLogOn_StopCnt); 
			exist_stat[DEF_SYS_SCMA] = DEF_EXIST;
		}
		else
		{
			g_stSvcVal.logon_B[DEF_CURRENT] += ntohl(pstLeg->stPDSNStat[count].uiLogOn_StartCnt) + 
								ntohl(pstLeg->stPDSNStat[count].uiLogOn_InterimCnt) +
								ntohl(pstLeg->stPDSNStat[count].uiLogOn_DiscReqCnt);
			g_stSvcVal.logout_B[DEF_CURRENT] += ntohl(pstLeg->stPDSNStat[count].uiLogOn_StopCnt); 
			exist_stat[DEF_SYS_SCMB] = DEF_EXIST;
		}


		if (stmd_mysql_query (query) < 0) {
			sprintf(trcBuf, "query fail: query:%s, err = %s\n", query, mysql_error(conn));
			trclib_writeErr (FL,trcBuf);
			return -2;
		}
#ifdef DEBUG
		logPrint(trcLogId, FL, " UPDATE ACCOUNT INFO >>> %s,%s,%s\n"
				               " @@@ rx=%d start=%d interim=%d discon=%d stop =%d "
							   " logon(start=%d interim=%d discon=%d logout=%d\n)n",
							   sysname, PDSN, get_insert_time(),
				ntohl(pstLeg->stPDSNStat[count].uiPDSN_RecvCnt),
				ntohl(pstLeg->stPDSNStat[count].uiPDSN_StartCnt),
				ntohl(pstLeg->stPDSNStat[count].uiPDSN_InterimCnt),
				ntohl(pstLeg->stPDSNStat[count].uiPDSN_DiscReqCnt),
				ntohl(pstLeg->stPDSNStat[count].uiPDSN_StopCnt),
				ntohl(pstLeg->stPDSNStat[count].uiLogOn_StartCnt),
				ntohl(pstLeg->stPDSNStat[count].uiLogOn_InterimCnt),
				ntohl(pstLeg->stPDSNStat[count].uiLogOn_DiscReqCnt),
				ntohl(pstLeg->stPDSNStat[count].uiLogOn_StopCnt));

							   /*
		@@@ SCMB,199.168.101.200, stat_date='2011-05-05 00:00:00'
		@@@ rx=1020 start=3049 interim=3388 discon=383 stop=3837  logon(start=3838 interim=3838 discon=8383 logout=3884)
													 */

#endif
	}

	// SCMA, SCMB 의 현재 통계가 존재하지 않는 경우, 현재 값은 0

	if( !strncmp(sysname, "SCMA", 4) )
	{
		if( exist_stat[DEF_SYS_SCMA] == DEF_NOT_EXIST ) // SCMA not exist
		{
			g_stSvcVal.logon_A[DEF_CURRENT] = 0;
			g_stSvcVal.logout_A[DEF_CURRENT] = 0;
		}
	}
	else
	{
		if( exist_stat[DEF_SYS_SCMB] == DEF_NOT_EXIST ) // SCMB not exist
		{
			g_stSvcVal.logon_B[DEF_CURRENT] = 0;
			g_stSvcVal.logout_B[DEF_CURRENT] = 0;
		}
	}

	// logon A + B 현재 값 
//	g_stSvcVal.logonSum[1] = g_stSvcVal.logon_A[1] + g_stSvcVal.logon_B[1];
//	g_stSvcVal.logoutSum[1] = g_stSvcVal.logout_A[1] + g_stSvcVal.logout_B[1];

	if ( trcLogFlag == TRCLEVEL_SQL ) {
		sprintf( trcBuf, "query = %s\n", query);
		trclib_writeLog(FL, trcBuf);
	}

    return 1;
}

#if 0 // 사용되지 않음 added by uamyd 20110424
int stmd_hdlRlegStatisticRpt_OLD(IxpcQMsgType *rxIxpcMsg)
{
	LEG_STAT   stLeg;
    char    query[4096];
//// 	char	sysName[2][10] = {"SCMA","SCMB"};
////	int		i, j;
	int		j;

	struct in_addr pdsn_ip ;
	char    PDSN[24] = {0,};
	int		exist_stat[2] = {0, 0};

	int count;

	memset(exist_stat, 0x00, sizeof(exist_stat));
    // 현재의 데이타를 삽입한다.
	memset(&stLeg, 0x00, sizeof(stLeg));
    memcpy((void *)&stLeg, rxIxpcMsg->body, sizeof(LEG_STAT));

    // leg_loginout 테이블을 지운다. 
	/*
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
            STM_STATISTIC_5MINUTE_LEG_TBL_NAME, get_delete_time(STMD_1HOUR_OFFSET*7));
			*/
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
            STM_STATISTIC_5MINUTE_LEG_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_MIN]));
    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf( trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "leg_5minute mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
    }

	/** RLEG A, B 양쪽에서 data를 받게됨. 
	for( i	= 0; i < 2; i++ )
	{
		for( j = 0; j < g_stPdsn.pdsnCnt; j++ )
		{
			sprintf(query, "Insert into leg_5minute_statistics values "
					" ( '%s', '%s', 0, 0, 0, 0, 0, 0, 0, 1, '%s', '%s' ) ",
					sysName[i], g_stPdsn.stItem[j].ip, get_insert_time(), get_insert_week());

			if (stmd_mysql_query (query) < 0) {
				sprintf(trcBuf, "query fail: query:%s, err = %s\n", query, mysql_error(conn));
				trclib_writeErr (FL,trcBuf);
				return -1;
			}
		}
	}
	*/
	/**< stLeg.szSysName이 SCMA인 경우 Default 0를 insert 하는 부분.**/
	if( !strcmp(stLeg.szSysName, "SCMA" ) )
	{
		for( j = 0; j < g_stPdsn.pdsnCnt; j++ )		/**< pdsn cnt 는 2?? **/
		{
			sprintf(query, "Insert into leg_5minute_statistics values "
					" ( 'SCMA', '%s', 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, '%s', '%s' ) ",
					g_stPdsn.stItem[j].ip, get_insert_time(), get_insert_week());

			if (stmd_mysql_query (query) < 0) {
				sprintf(trcBuf, "query fail: query:%s, err = %s\n", query, mysql_error(conn));
				trclib_writeErr (FL,trcBuf);
				return -1;
			}
			logPrint(trcLogId, FL, "ACCOUNT INSERT[%d/%d] : %s <-- %s\n",j, g_stPdsn.pdsnCnt, query, stLeg.szSysName);
		}

		/**< 여기서 함수 만들어서 분리 **/
//		update_account (rxIxpcMsg);
	}
	else
	{
		for( j = 0; j < g_stPdsn.pdsnCnt; j++ )
		{
			sprintf(query, "Insert into leg_5minute_statistics values "
					" ( 'SCMB', '%s', 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, '%s', '%s' ) ",
					g_stPdsn.stItem[j].ip, get_insert_time(), get_insert_week());

			if (stmd_mysql_query (query) < 0) {
				sprintf(trcBuf, "query fail: query:%s, err = %s\n", query, mysql_error(conn));
				trclib_writeErr (FL,trcBuf);
				return -1;
			}
			logPrint(trcLogId, FL, "ACCOUNT INSERT[%d/%d] : %s <-- %s\n",j, g_stPdsn.pdsnCnt, query, stLeg.szSysName);
		}
//		update_account (rxIxpcMsg);
	}



#if 1

	/**< stLeg.nCount == ?? 얼마냐? nCount가 PDSN IP 갯수입니다.  
	  **/
	for( count=0; count < ntohl(stLeg.nCount); count++ )
	{	
		pdsn_ip.s_addr = stLeg.stPDSNStat[count].uiPDSN_IP;	
		sprintf(PDSN, "%s",inet_ntoa(pdsn_ip));

		// 비교문이 필요없는데.. 바꿀 필요있음.
		/** 굳이 비교해서 update 할 필요없음. 
		if( !strncasecmp(stLeg.szSysName, "SCMA", 4) )
		{
			sprintf(query, "UPDATE %s SET rx_cnt = %d, start = %d, interim = %d, "
					" stop = %d, start_logon_cnt = %d, int_logon_cnt = %d, "
					" logout_cnt = %d "
					" where system_name = 'SCMA' and pdsn_ip = '%s' and stat_date = '%s'",
					STM_STATISTIC_5MINUTE_LEG_TBL_NAME, 
					ntohl(stLeg.stPDSNStat[count].uiPDSN_RecvCnt),
					ntohl(stLeg.stPDSNStat[count].uiPDSN_StartCnt),
					ntohl(stLeg.stPDSNStat[count].uiPDSN_InterimCnt),
					ntohl(stLeg.stPDSNStat[count].uiPDSN_StopCnt),
					ntohl(stLeg.stPDSNStat[count].uiLogOn_StartCnt),
					ntohl(stLeg.stPDSNStat[count].uiLogOn_InterimCnt),
					ntohl(stLeg.stPDSNStat[count].uiLogOn_StopCnt),
					PDSN, get_insert_time());
		}
		else
		{
			sprintf(query, "UPDATE %s SET rx_cnt = %d, start = %d, interim = %d, "
					" stop = %d, start_logon_cnt = %d, int_logon_cnt = %d, "
					" logout_cnt = %d "
					" where system_name = 'SCMB' and pdsn_ip = '%s' and stat_date = '%s'",
					STM_STATISTIC_5MINUTE_LEG_TBL_NAME, 
					ntohl(stLeg.stPDSNStat[count].uiPDSN_RecvCnt),
					ntohl(stLeg.stPDSNStat[count].uiPDSN_StartCnt),
					ntohl(stLeg.stPDSNStat[count].uiPDSN_InterimCnt),
					ntohl(stLeg.stPDSNStat[count].uiPDSN_StopCnt),
					ntohl(stLeg.stPDSNStat[count].uiLogOn_StartCnt),
					ntohl(stLeg.stPDSNStat[count].uiLogOn_InterimCnt),
					ntohl(stLeg.stPDSNStat[count].uiLogOn_StopCnt),
					PDSN, get_insert_time());
		}
		*/
		sprintf(query, "UPDATE %s SET rx_cnt = %d, start = %d, interim = %d, disconnect = %d, "
				" stop = %d, start_logon_cnt = %d, int_logon_cnt = %d, disc_logon_cnt = %d, "
				" logout_cnt = %d "
				" where system_name = '%s' and pdsn_ip = '%s' and stat_date = '%s'",
				STM_STATISTIC_5MINUTE_LEG_TBL_NAME, 
				ntohl(stLeg.stPDSNStat[count].uiPDSN_RecvCnt),
				ntohl(stLeg.stPDSNStat[count].uiPDSN_StartCnt),
				ntohl(stLeg.stPDSNStat[count].uiPDSN_InterimCnt),
				ntohl(stLeg.stPDSNStat[count].uiPDSN_DiscReqCnt),
				ntohl(stLeg.stPDSNStat[count].uiPDSN_StopCnt),
				ntohl(stLeg.stPDSNStat[count].uiLogOn_StartCnt),
				ntohl(stLeg.stPDSNStat[count].uiLogOn_InterimCnt),
				ntohl(stLeg.stPDSNStat[count].uiLogOn_DiscReqCnt),
				ntohl(stLeg.stPDSNStat[count].uiLogOn_StopCnt),
				stLeg.szSysName,
				PDSN, get_insert_time());

		// SVC_ALM 을 위한 logon / logout 현재 값 저장 .
		if( !strncasecmp(stLeg.szSysName, "SCMA", 4) )
		{
			g_stSvcVal.logon_A[DEF_CURRENT] += ntohl(stLeg.stPDSNStat[count].uiLogOn_StartCnt) + 
								ntohl(stLeg.stPDSNStat[count].uiLogOn_InterimCnt) +
								ntohl(stLeg.stPDSNStat[count].uiLogOn_DiscReqCnt);
			g_stSvcVal.logout_A[DEF_CURRENT] += ntohl(stLeg.stPDSNStat[count].uiLogOn_StopCnt); 
			exist_stat[DEF_SYS_SCMA] = DEF_EXIST;
		}
		else
		{
			g_stSvcVal.logon_B[DEF_CURRENT] += ntohl(stLeg.stPDSNStat[count].uiLogOn_StartCnt) + 
								ntohl(stLeg.stPDSNStat[count].uiLogOn_InterimCnt) +
								ntohl(stLeg.stPDSNStat[count].uiLogOn_DiscReqCnt);
			g_stSvcVal.logout_B[DEF_CURRENT] += ntohl(stLeg.stPDSNStat[count].uiLogOn_StopCnt); 
			exist_stat[DEF_SYS_SCMB] = DEF_EXIST;
		}


		if (stmd_mysql_query (query) < 0) {
			sprintf(trcBuf, "query fail: query:%s, err = %s\n", query, mysql_error(conn));
			trclib_writeErr (FL,trcBuf);
			return -1;
		}
		logPrint(trcLogId, FL, "ACCOUNT UPDATE[%d/%d] : %s <-- %s\n",count,ntohl(stLeg.nCount), query, stLeg.szSysName);

	}

	// SCMA, SCMB 의 현재 통계가 존재하지 않는 경우, 현재 값은 0

	if( !strncmp(stLeg.szSysName, "SCMA", 4) )
	{
		if( exist_stat[DEF_SYS_SCMA] == DEF_NOT_EXIST ) // SCMA not exist
		{
			g_stSvcVal.logon_A[DEF_CURRENT] = 0;
			g_stSvcVal.logout_A[DEF_CURRENT] = 0;
		}
	}
	else
	{
		if( exist_stat[DEF_SYS_SCMB] == DEF_NOT_EXIST ) // SCMB not exist
		{
			g_stSvcVal.logon_B[DEF_CURRENT] = 0;
			g_stSvcVal.logout_B[DEF_CURRENT] = 0;
		}
	}

	// logon A + B 현재 값 
//	g_stSvcVal.logonSum[1] = g_stSvcVal.logon_A[1] + g_stSvcVal.logon_B[1];
//	g_stSvcVal.logoutSum[1] = g_stSvcVal.logout_A[1] + g_stSvcVal.logout_B[1];

	if ( trcLogFlag == TRCLEVEL_SQL ) {
		sprintf( trcBuf, "query = %s\n", query);
		trclib_writeLog(FL, trcBuf);
	}
#endif

    return 1;
}
#endif

int stmd_hdlLogonStatisticRpt(IxpcQMsgType *rxIxpcMsg)
{
	PLEG_TOT_STAT pstLegTot;
	PLOGON_STAT   pstLogOn;
    char         *sysname, query[4096];
	int           log_mod, sm_ch_id;
	float         rate; //uamyd 20110515 succrate_added

    // leg_loginout 테이블을 지운다. 
	/*
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
            STM_STATISTIC_5MINUTE_LOGON_TBL_NAME, get_delete_time(STMD_1HOUR_OFFSET*7));
	*/
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
            STM_STATISTIC_5MINUTE_LOGON_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_MIN]));
    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf( trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "log_leg mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
		return -1;
    }

	pstLegTot = (PLEG_TOT_STAT)&rxIxpcMsg->body;
	sysname   = &pstLegTot->szSysName[0];
	
	// RLEG A, B에서 받는 걸로 변경. 
	for( sm_ch_id = 0; sm_ch_id < MAX_RLEG_CNT; sm_ch_id++ ){
		for( log_mod = 0; log_mod < LOG_MOD_CNT; log_mod++ ){ 
			sprintf(query, "insert into %s values ( '%s', %d, %d, %u, %u, %u, %d,"
					"%u, %u, %u, %u, %u, %u, %u, %u, %u, %u,"
					"%u, %u, %u, %u, %u, %u, %u, %u, %u, %u,"
					"%u, %u, %u, %u, %u, %u, %u, %u, %u, %u,"
					"%u, %u, "
					"%u, %u, %u, %u, %u, %u,"
					"%d, '%s', '%s' )", 
					STM_STATISTIC_5MINUTE_LOGON_TBL_NAME, sysname, sm_ch_id, log_mod, 0,0,0,0, 
					0,0,0,0,0,0,0,0,0,0,
					0,0,0,0,0,0,0,0,0,0,
					0,0,0,0,0,0,0,0,0,0,
					0,0,
					0,0,0,0,0,0, 
					1, get_insert_time(), get_insert_week());

			if (stmd_mysql_query (query) < 0) {
				sprintf(trcBuf, "insert logon_5minute mysql_query fail:%s\n", query);
				trclib_writeLogErr (FL,trcBuf);
				return -(200+sm_ch_id*10+log_mod);
			}

			pstLogOn  = &pstLegTot->stLogon[sm_ch_id][log_mod];
			//rate      = (((float)ntohl(pstLogOn->uiLogOn_Success)/(float)ntohl(pstLogOn->uiLogOn_Request))*100);
			if(ntohl(pstLogOn->uiLogOn_Request) ==0 )
				rate  = 0;
			else 
				rate  = (((float)ntohl(pstLogOn->uiLogOn_Success)/(float)ntohl(pstLogOn->uiLogOn_Request))*100);

			sprintf(query, "UPDATE %s SET log_req = %d, log_succ = %d, log_fail = %d, succ_rate = %.1f,"
					" HBIT_0 = %d, HBIT_1 = %d, HBIT_2 = %d, HBIT_3 = %d, HBIT_4 = %d ,"
					" HBIT_5 = %d, HBIT_6 = %d, HBIT_7 = %d, HBIT_8 = %d, HBIT_9 = %d ,"
					" HBIT_10 = %d, HBIT_11 = %d, HBIT_12 = %d, HBIT_13 = %d, HBIT_14 = %d ,"
					" HBIT_15 = %d, HBIT_16 = %d, HBIT_17 = %d, HBIT_18 = %d, HBIT_19 = %d ,"
					" HBIT_20 = %d, HBIT_21 = %d, HBIT_22 = %d, HBIT_23 = %d, HBIT_24 = %d ,"
					" HBIT_25 = %d, HBIT_26 = %d, HBIT_27 = %d, HBIT_28 = %d, HBIT_29 = %d ,"
					" HBIT_30 = %d, HBIT_31 = %d, "
					" SM_INT_ERR = %d, OP_ERR = %d, OP_TIMEOUT = %d, ETC_FAIL = %d, "
					" API_REQ_ERR = %d, API_TIMEOUT = %d "
					" where system_name = '%s' and stat_date = '%s' and sm_ch_id=%d and log_mod=%d",
					STM_STATISTIC_5MINUTE_LOGON_TBL_NAME, 
					ntohl(pstLogOn->uiLogOn_Request),
					ntohl(pstLogOn->uiLogOn_Success),
					ntohl(pstLogOn->uiLogOn_Fail),
					//!ntohl(pstLogOn->uiLogOn_Request)?0:(int)rate,
					rate,
					ntohl(pstLogOn->uiLogOn_HBIT[0]),
					ntohl(pstLogOn->uiLogOn_HBIT[1]),
					ntohl(pstLogOn->uiLogOn_HBIT[2]),
					ntohl(pstLogOn->uiLogOn_HBIT[3]),
					ntohl(pstLogOn->uiLogOn_HBIT[4]),
					ntohl(pstLogOn->uiLogOn_HBIT[5]),
					ntohl(pstLogOn->uiLogOn_HBIT[6]),
					ntohl(pstLogOn->uiLogOn_HBIT[7]),
					ntohl(pstLogOn->uiLogOn_HBIT[8]),
					ntohl(pstLogOn->uiLogOn_HBIT[9]),
					ntohl(pstLogOn->uiLogOn_HBIT[10]),
					ntohl(pstLogOn->uiLogOn_HBIT[11]),
					ntohl(pstLogOn->uiLogOn_HBIT[12]),
					ntohl(pstLogOn->uiLogOn_HBIT[13]),
					ntohl(pstLogOn->uiLogOn_HBIT[14]),
					ntohl(pstLogOn->uiLogOn_HBIT[15]),
					ntohl(pstLogOn->uiLogOn_HBIT[16]),
					ntohl(pstLogOn->uiLogOn_HBIT[17]),
					ntohl(pstLogOn->uiLogOn_HBIT[18]),
					ntohl(pstLogOn->uiLogOn_HBIT[19]),
					ntohl(pstLogOn->uiLogOn_HBIT[20]),
					ntohl(pstLogOn->uiLogOn_HBIT[21]),
					ntohl(pstLogOn->uiLogOn_HBIT[22]),
					ntohl(pstLogOn->uiLogOn_HBIT[23]),
					ntohl(pstLogOn->uiLogOn_HBIT[24]),
					ntohl(pstLogOn->uiLogOn_HBIT[25]),
					ntohl(pstLogOn->uiLogOn_HBIT[26]),
					ntohl(pstLogOn->uiLogOn_HBIT[27]),
					ntohl(pstLogOn->uiLogOn_HBIT[28]),
					ntohl(pstLogOn->uiLogOn_HBIT[29]),
					ntohl(pstLogOn->uiLogOn_HBIT[30]),
					ntohl(pstLogOn->uiLogOn_HBIT[31]),
					ntohl(pstLogOn->uiLogOn_Reason1),
					ntohl(pstLogOn->uiLogOn_Reason2),
					ntohl(pstLogOn->uiLogOn_Reason3),
					ntohl(pstLogOn->uiLogOn_Reason4),
					ntohl(pstLogOn->uiLogOn_APIReqErr),
					ntohl(pstLogOn->uiLogOn_APITimeout),
					sysname,
					get_insert_time(),sm_ch_id, log_mod);

			if (stmd_mysql_query (query) < 0) {
				sprintf(trcBuf, "update logon_5minute mysql_query fail:%s\n", query);
				trclib_writeLogErr (FL,trcBuf);
				return -(300+sm_ch_id*10+log_mod);
			}
#ifdef DEBUG
			logPrint(trcLogId, FL, " @@@ UPDATE LOGON QUERY :: %s\n", query);
#endif
		}
	}

	logPrint(trcLogId, FL, "LOGON UPDATE :  <-- %s\n", sysname);

	if ( trcLogFlag == TRCLEVEL_SQL ) {
		sprintf( trcBuf, "query = %s\n", query);
		trclib_writeLog(FL, trcBuf);
	}

    return 1;
}

#if 0 //사용되지 않음. 로직이 변경됨 by uamyd 20110424
int stmd_hdlLogonStatisticRpt_OLD(IxpcQMsgType *rxIxpcMsg)
{
	LOGON_STAT   stLogOn;
    char    query[4096];
//	char SysName[2][10] = {"SCMA", "SCMB"};


    // leg_loginout 테이블을 지운다. 
	/*
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
            STM_STATISTIC_5MINUTE_LOGON_TBL_NAME, get_delete_time(STMD_1HOUR_OFFSET*7));
	*/
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
            STM_STATISTIC_5MINUTE_LOGON_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_MIN]));
    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf( trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "log_leg mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
    }
	memset(&stLogOn, 0x00, sizeof(stLogOn));

    // 현재의 데이타를 삽입한다.
    memcpy((void *)&stLogOn, rxIxpcMsg->body, sizeof(LOGON_STAT));
	
	if( !strncmp(stLogOn.szSysName, "SCMA", 4) )
	{
		// RLEG A, B에서 받는 걸로 변경. 
		sprintf(query, "insert into %s values ( 'SCMA', %u, %u, %u,"
				"%u, %u, %u, %u, %u, %u, %u, %u, %u, %u,"
				"%u, %u, %u, %u, %u, %u, %u, %u, %u, %u,"
				"%u, %u, %u, %u, %u, %u, %u, %u, %u, %u,"
				"%u, %u, "
				"%u, %u, %u, %u, %u, %u,"
				"%d, '%s', '%s' )", 
				STM_STATISTIC_5MINUTE_LOGON_TBL_NAME, 0,0,0, 
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,
				0,0,0,0,0,0, 
				1, get_insert_time(), get_insert_week());

	}
	else if( !strncmp(stLogOn.szSysName, "SCMB", 4) )
	{
		sprintf(query, "insert into %s values ( 'SCMB', %u, %u, %u,"
				"%u, %u, %u, %u, %u, %u, %u, %u, %u, %u,"
				"%u, %u, %u, %u, %u, %u, %u, %u, %u, %u,"
				"%u, %u, %u, %u, %u, %u, %u, %u, %u, %u,"
				"%u, %u, "
				"%u, %u, %u, %u, %u, %u,"
				"%d, '%s', '%s' )", 
				STM_STATISTIC_5MINUTE_LOGON_TBL_NAME, 0,0,0, 
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,
				0,0,0,0,0,0, 
				1, get_insert_time(), get_insert_week());
	}

	if (stmd_mysql_query (query) < 0) {
		sprintf(trcBuf, "insert logon_5minute mysql_query fail:%s\n", query);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	sprintf(query, "UPDATE %s SET log_req = %d, log_succ = %d, log_fail = %d, "
			" HBIT_0 = %d, HBIT_1 = %d, HBIT_2 = %d, HBIT_3 = %d, HBIT_4 = %d ,"
			" HBIT_5 = %d, HBIT_6 = %d, HBIT_7 = %d, HBIT_8 = %d, HBIT_9 = %d ,"
			" HBIT_10 = %d, HBIT_11 = %d, HBIT_12 = %d, HBIT_13 = %d, HBIT_14 = %d ,"
			" HBIT_15 = %d, HBIT_16 = %d, HBIT_17 = %d, HBIT_18 = %d, HBIT_19 = %d ,"
			" HBIT_20 = %d, HBIT_21 = %d, HBIT_22 = %d, HBIT_23 = %d, HBIT_24 = %d ,"
			" HBIT_25 = %d, HBIT_26 = %d, HBIT_27 = %d, HBIT_28 = %d, HBIT_29 = %d ,"
			" HBIT_30 = %d, HBIT_31 = %d, "
			" SM_INT_ERR = %d, OP_ERR = %d, OP_TIMEOUT = %d, ETC_FAIL = %d, "
			" API_REQ_ERR = %d, API_TIMEOUT = %d "
			" where system_name = '%s' and stat_date = '%s'",
			STM_STATISTIC_5MINUTE_LOGON_TBL_NAME, 
			ntohl(stLogOn.uiLogOn_Request),
			ntohl(stLogOn.uiLogOn_Success),
			ntohl(stLogOn.uiLogOn_Fail),
			ntohl(stLogOn.uiLogOn_HBIT[0]),
			ntohl(stLogOn.uiLogOn_HBIT[1]),
			ntohl(stLogOn.uiLogOn_HBIT[2]),
			ntohl(stLogOn.uiLogOn_HBIT[3]),
			ntohl(stLogOn.uiLogOn_HBIT[4]),
			ntohl(stLogOn.uiLogOn_HBIT[5]),
			ntohl(stLogOn.uiLogOn_HBIT[6]),
			ntohl(stLogOn.uiLogOn_HBIT[7]),
			ntohl(stLogOn.uiLogOn_HBIT[8]),
			ntohl(stLogOn.uiLogOn_HBIT[9]),
			ntohl(stLogOn.uiLogOn_HBIT[10]),
			ntohl(stLogOn.uiLogOn_HBIT[11]),
			ntohl(stLogOn.uiLogOn_HBIT[12]),
			ntohl(stLogOn.uiLogOn_HBIT[13]),
			ntohl(stLogOn.uiLogOn_HBIT[14]),
			ntohl(stLogOn.uiLogOn_HBIT[15]),
			ntohl(stLogOn.uiLogOn_HBIT[16]),
			ntohl(stLogOn.uiLogOn_HBIT[17]),
			ntohl(stLogOn.uiLogOn_HBIT[18]),
			ntohl(stLogOn.uiLogOn_HBIT[19]),
			ntohl(stLogOn.uiLogOn_HBIT[20]),
			ntohl(stLogOn.uiLogOn_HBIT[21]),
			ntohl(stLogOn.uiLogOn_HBIT[22]),
			ntohl(stLogOn.uiLogOn_HBIT[23]),
			ntohl(stLogOn.uiLogOn_HBIT[24]),
			ntohl(stLogOn.uiLogOn_HBIT[25]),
			ntohl(stLogOn.uiLogOn_HBIT[26]),
			ntohl(stLogOn.uiLogOn_HBIT[27]),
			ntohl(stLogOn.uiLogOn_HBIT[28]),
			ntohl(stLogOn.uiLogOn_HBIT[29]),
			ntohl(stLogOn.uiLogOn_HBIT[30]),
			ntohl(stLogOn.uiLogOn_HBIT[31]),
			ntohl(stLogOn.uiLogOn_Reason1),
			ntohl(stLogOn.uiLogOn_Reason2),
			ntohl(stLogOn.uiLogOn_Reason3),
			ntohl(stLogOn.uiLogOn_Reason4),
			ntohl(stLogOn.uiLogOn_APIReqErr),
			ntohl(stLogOn.uiLogOn_APITimeout),
			stLogOn.szSysName,
			get_insert_time());

	if (stmd_mysql_query (query) < 0) {
		sprintf(trcBuf, "update logon_5minute mysql_query fail:%s\n", query);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	trclib_writeLog(FL, trcBuf);
	logPrint(trcLogId, FL, "LOGON UPDATE : %s <-- %s\n", query, stLogOn.szSysName);
	

	if ( trcLogFlag == TRCLEVEL_SQL ) {
		sprintf( trcBuf, "query = %s\n", query);
		trclib_writeLog(FL, trcBuf);
	}

    return 1;
}
#endif

/** 2010.08.23 FLOW */
int stmd_hdlFlowStatisticRpt(IxpcQMsgType *rxIxpcMsg)
{
	SCE_FLOW_INFO   stFlow;
    char    query[4096];
//	char SysName[2][10] = {"SCMA", "SCMB"};

	memset(&stFlow, 0x00, sizeof(stFlow));

    // 현재의 데이타를 삽입한다.
    memcpy((void *)&stFlow, rxIxpcMsg->body, sizeof(SCE_FLOW_INFO));
	
	// 초기화 Insert 
	// SYSNAME, FLOW_NUM
	sprintf(query, "insert into %s values ( '%s', %u, from_unixtime(%u), %u )", 
			STM_REPORT_IMSI_FLOW_TBL_NAME, 
			stFlow.szSysName, ntohl(stFlow.tGetTime), ntohl(stFlow.tGetTime), 0);

	if (stmd_mysql_query (query) < 0) {
		sprintf(trcBuf, "insert flow report imsi mysql_query fail:%s\n", query);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	// REPORT DATA update
	sprintf(query, "UPDATE %s SET FlowNum = %d "
			" WHERE system_name = '%s' and ReportTime = %u ",
			STM_REPORT_IMSI_FLOW_TBL_NAME, 
			ntohl(stFlow.uiFlowNum),
			stFlow.szSysName,
			ntohl(stFlow.tGetTime));
			

	if (stmd_mysql_query (query) < 0) {
		sprintf(trcBuf, "update flow report imsi mysql_query fail:%s\n", query);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	if ( trcLogFlag == TRCLEVEL_SQL ) {
		sprintf( trcBuf, "query = %s\n", query);
		trclib_writeLog(FL, trcBuf);
	}

    return 1;
}

int stmd_hdlLoadStatisticRpt(IxpcQMsgType *rxIxpcMsg)
{
    STM_LoadOMPStatMsgType    LoadOmpStat;
    STM_LoadMPStatMsgType    LoadMpStat;

    int     i, j;
    char    query[4096];
    char    groupName[COMM_MAX_NAME_LEN];
    char    sysType[COMM_MAX_NAME_LEN];
    char    sceName[COMM_MAX_NAME_LEN];
	int		avgSceMem[2], maxSceMem[2];
	int 	sumSceMem[2];

    if ( get_system_information(rxIxpcMsg->head.srcSysName, groupName, sysType) < 0) {
        sprintf(trcBuf, "LoadStatistic unknown Processor = %s\n", rxIxpcMsg->head.srcSysName);
        trclib_writeErr(FL, trcBuf);
        return -1;
    }
    
    //if(!strcasecmp(sysType, SYSCONF_SYSTYPE_SMS))
    if(!strcasecmp(sysType, SYSCONF_SYSTYPE_BSD))
        strcpy(sysType, "MP");

	/*
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_5MINUTE_LOAD_TBL_NAME, get_delete_time(STMD_1HOUR_OFFSET*7)); // 7일 
	*/
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
        STM_STATISTIC_5MINUTE_LOAD_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_MIN])); 
    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf( trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "LoadStatistic 5M mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
    }

    
    if( !strcasecmp(sysType,"MP") )
    {
        // 현재의 데이타를 삽입한다.
        memcpy((void *)&LoadMpStat, rxIxpcMsg->body, sizeof(STM_LoadMPStatMsgType));

        sprintf(query, "INSERT INTO %s VALUES ( "
				"'%s', '%s', %d, %d, %d, "
				" %d, %d, %d, %d, %d,"
            	" %d, %d, %d, %d, %d, "
				" %d, "
            	" %d, '%s', '%s' )", 
            STM_STATISTIC_5MINUTE_LOAD_TBL_NAME, sysType, rxIxpcMsg->head.srcSysName,
            ntohl(LoadMpStat.comminfo.average_cpu[0]),
            ntohl(LoadMpStat.comminfo.max_cpu[0]),
            ntohl(LoadMpStat.comminfo.average_cpu[1]),
            ntohl(LoadMpStat.comminfo.max_cpu[1]),
            ntohl(LoadMpStat.comminfo.average_cpu[2]),
            ntohl(LoadMpStat.comminfo.max_cpu[2]),
            ntohl(LoadMpStat.comminfo.average_cpu[3]),
            ntohl(LoadMpStat.comminfo.max_cpu[3]), 

            ntohl(LoadMpStat.comminfo.avg_mem),
            ntohl(LoadMpStat.comminfo.max_mem),

            ntohl(LoadMpStat.avg_disk),
            ntohl(LoadMpStat.max_disk),
            ntohl(LoadMpStat.avg_msgQ),
            ntohl(LoadMpStat.max_msgQ),
//            ntohl(LoadMpStat.avg_sess),
//            ntohl(LoadMpStat.max_sess),

            1, get_insert_time(), get_insert_week()); // stat_date는 현재 시간 time(0)

        if ( trcLogFlag == TRCLEVEL_SQL ) {
            sprintf( trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }

        if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf, "LoadStatistic 5M mysql_query fail:%s\n", query);
            trclib_writeLogErr (FL,trcBuf);
            return -1;
        }

//		sprintf( trcBuf, "query = %s\n", query);
//		trclib_writeLogErr(FL, trcBuf);
    }
    else
    {
        // 현재의 데이타를 삽입한다.
        memcpy((void *)&LoadOmpStat, rxIxpcMsg->body, sizeof(STM_LoadOMPStatMsgType));

        sprintf(query, "INSERT INTO %s VALUES ('%s', '%s',"
            " %d, %d, %d, %d, %d, "
			" %d, %d, %d, %d, %d,"
            " %d, %d, %d, %d, "
            " %d, '%s', '%s' )", 
            STM_STATISTIC_5MINUTE_LOAD_TBL_NAME, sysType, rxIxpcMsg->head.srcSysName,

            LoadOmpStat.ompInfo.comminfo.average_cpu[0],
            LoadOmpStat.ompInfo.comminfo.max_cpu[0],
            LoadOmpStat.ompInfo.comminfo.average_cpu[1],
            LoadOmpStat.ompInfo.comminfo.max_cpu[1],
            LoadOmpStat.ompInfo.comminfo.average_cpu[2],
            LoadOmpStat.ompInfo.comminfo.max_cpu[2],
            LoadOmpStat.ompInfo.comminfo.average_cpu[3],
            LoadOmpStat.ompInfo.comminfo.max_cpu[3],

            LoadOmpStat.ompInfo.comminfo.avg_mem,
            LoadOmpStat.ompInfo.comminfo.max_mem,

            LoadOmpStat.ompInfo.avg_disk,
            LoadOmpStat.ompInfo.max_disk,
            LoadOmpStat.ompInfo.avg_msgQ,
            LoadOmpStat.ompInfo.max_msgQ,
//            LoadOmpStat.ompInfo.avg_sess,
//            LoadOmpStat.ompInfo.max_sess,

            1, get_insert_time(), get_insert_week());

        if ( trcLogFlag == TRCLEVEL_SQL ) {
            sprintf( trcBuf, "query = %s\n", query);
            trclib_writeLog(FL, trcBuf);
        }

        if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf, "LoadStatistic 5M mysql_query fail: %s\n", query);
            trclib_writeLogErr (FL,trcBuf);
            return -1;
        }

		sprintf( trcBuf, "query = %s\n", query);
		trclib_writeLog(FL, trcBuf);
/* 10.13 TAP REMOVE
        for(i =0; i< 2; i++){
            if (i ==0)
                strcpy(pdName, "TAPA");
            else
                strcpy(pdName, "TAPB");

            sprintf(query, "INSERT INTO %s VALUES ('%s', '%s',"
                " %d, %d, %d, %d, %d, %d, %d, %d,"
                " %d, %d,"
                " %d, %d, %d, %d, "
                " %d, '%s', '%s' )", 
                STM_STATISTIC_5MINUTE_LOAD_TBL_NAME, "TAP", pdName,

                LoadOmpStat.pdInfo[i].average_cpu[0],
                LoadOmpStat.pdInfo[i].max_cpu[0],
                LoadOmpStat.pdInfo[i].average_cpu[1],
                LoadOmpStat.pdInfo[i].max_cpu[1],
                LoadOmpStat.pdInfo[i].average_cpu[2],
                LoadOmpStat.pdInfo[i].max_cpu[2],
                LoadOmpStat.pdInfo[i].average_cpu[3],
                LoadOmpStat.pdInfo[i].max_cpu[3],

                LoadOmpStat.pdInfo[i].avg_mem,
                LoadOmpStat.pdInfo[i].max_mem,

                0, 0, 0, 0, 

                1, get_insert_time(), get_insert_week());

            if ( trcLogFlag == TRCLEVEL_SQL ) {
                sprintf( trcBuf, "query = %s\n", query);
                trclib_writeLog(FL, trcBuf);
            }

            if (stmd_mysql_query (query) < 0) {
                sprintf(trcBuf, "LoadStatistic 5M mysql_query fail:%s\n", query);
                trclib_writeLogErr (FL,trcBuf);
                return -1;
            }

//			sprintf( trcBuf, "query = %s\n", query);
//			trclib_writeLogErr(FL, trcBuf);

        } //end for
*/
        for(i =0; i< 2; i++){
            if (i ==0)
			{
				// sce A의 avg Mem, max Mem을 구하라.  3개 
				sumSceMem[i] = 0; maxSceMem[i] = 0;
				for(j = 0; j < 3; j++)
				{
					sumSceMem[i] += LoadOmpStat.sceInfo[i].avg_mem[j];
					if(LoadOmpStat.sceInfo[i].max_mem[j] > maxSceMem[0])
						LastSceMaxMem[0] = maxSceMem[0] = LoadOmpStat.sceInfo[i].max_mem[j];
				}
				LastSceAvgMem[0] = avgSceMem[0] = sumSceMem[i] / 3;
                strcpy(sceName, "SCEA");


				/* ABNORMAL CPU, MEMORY DEBUG LOG */
#if 0
				if(LoadOmpStat.sceInfo[i].max_cpu[0] > 1000 ) 
					LoadOmpStat.sceInfo[i].max_cpu[0] = LastSceMaxCpu[0][0];
				if(LoadOmpStat.sceInfo[i].max_cpu[1] > 1000 ) 
					LoadOmpStat.sceInfo[i].max_cpu[1] = LastSceMaxCpu[0][1];
				if(LoadOmpStat.sceInfo[i].max_cpu[2] > 1000 ) 
					LoadOmpStat.sceInfo[i].max_cpu[2] = LastSceMaxCpu[0][2];
				if(LoadOmpStat.sceInfo[i].avg_cpu[0] > 1000 ) 
					LoadOmpStat.sceInfo[i].avg_cpu[0] = LastSceAvgCpu[0][0];
				if(LoadOmpStat.sceInfo[i].avg_cpu[1] > 1000 ) 
					LoadOmpStat.sceInfo[i].avg_cpu[1] = LastSceAvgCpu[0][1];
				if(LoadOmpStat.sceInfo[i].avg_cpu[2] > 1000 ) 
					LoadOmpStat.sceInfo[i].avg_cpu[2] = LastSceAvgCpu[0][2];
#endif


				if(LoadOmpStat.sceInfo[i].max_mem[0] > 1000 || LoadOmpStat.sceInfo[i].max_cpu[0]>1000 ||
						LoadOmpStat.sceInfo[i].max_mem[1] > 1000 || LoadOmpStat.sceInfo[i].max_cpu[1]>1000 ||
						LoadOmpStat.sceInfo[i].max_mem[2] > 1000 || LoadOmpStat.sceInfo[i].max_cpu[2]>1000)
				{
					sprintf( trcBuf, "SCEA:AVG_CPU0:%d,MAX_CPU0:%d,AVG_CPU1:%d,MAX_CPU1:%d\n"
											"AVG_CPU2:%d,MAX_CPU2:%d,AVG_MEM0:%d,MAX_MEM0:%d\n"
											"AVG_MEM1:%d,MAX_MEM1:%d,AVG_MEM2:%d,MAX_MEM2:%d\n"
											"SUMSCEAMEM:%d,MAXSCEMEM:%d\n", 
											LoadOmpStat.sceInfo[i].avg_cpu[0],
											LoadOmpStat.sceInfo[i].max_cpu[0],
											LoadOmpStat.sceInfo[i].avg_cpu[1],
											LoadOmpStat.sceInfo[i].max_cpu[1],
											LoadOmpStat.sceInfo[i].avg_cpu[2],
											LoadOmpStat.sceInfo[i].max_cpu[2],
											LoadOmpStat.sceInfo[i].avg_mem[0],
											LoadOmpStat.sceInfo[i].max_mem[0],
											LoadOmpStat.sceInfo[i].avg_mem[1],
											LoadOmpStat.sceInfo[i].max_mem[1],
											LoadOmpStat.sceInfo[i].avg_mem[2],
											LoadOmpStat.sceInfo[i].max_mem[2],
											sumSceMem[0],
											maxSceMem[0] );
					trclib_writeLogErr (FL,trcBuf);
				}
			}
            else
			{
				// sce B의 avg Mem, max Mem을 구하라. 
				sumSceMem[i] = 0; maxSceMem[i] = 0;
				for(j = 0; j < 3; j++)
				{
					sumSceMem[i] += LoadOmpStat.sceInfo[i].avg_mem[j];
					if(LoadOmpStat.sceInfo[i].max_mem[j] > maxSceMem[1])
						LastSceMaxMem[0] = maxSceMem[1] = LoadOmpStat.sceInfo[i].max_mem[j];
				}
				LastSceAvgMem[1] = avgSceMem[1] = sumSceMem[i] / 3;
                strcpy(sceName, "SCEB");
#if 0
				/* ABNORMAL CPU, MEMORY DEBUG LOG */
				if(LoadOmpStat.sceInfo[i].max_mem[0] > 1000 ) 
					LoadOmpStat.sceInfo[i].max_mem[0] = LastSceMaxMem[1][0];
				if(LoadOmpStat.sceInfo[i].max_mem[1] > 1000 ) 
					LoadOmpStat.sceInfo[i].max_mem[1] = LastSceMaxMem[1][1];
				if(LoadOmpStat.sceInfo[i].max_mem[2] > 1000 ) 
					LoadOmpStat.sceInfo[i].max_mem[2] = LastSceMaxMem[1][2];
#endif
						
					
				if(LoadOmpStat.sceInfo[i].max_mem[0] > 1000 || LoadOmpStat.sceInfo[i].max_cpu[0]>1000 ||
						LoadOmpStat.sceInfo[i].max_mem[1] > 1000 || LoadOmpStat.sceInfo[i].max_cpu[1]>1000 ||
						LoadOmpStat.sceInfo[i].max_mem[2] > 1000 || LoadOmpStat.sceInfo[i].max_cpu[2]>1000)
				{
					sprintf( trcBuf, "SCEB:AVG_CPU0:%d,MAX_CPU0:%d,AVG_CPU1:%d,MAX_CPU1:%d\n"
											"AVG_CPU2:%d,MAX_CPU2:%d,AVG_MEM0:%d,MAX_MEM0:%d\n"
											"AVG_MEM1:%d,MAX_MEM1:%d,AVG_MEM2:%d,MAX_MEM2:%d\n"
											"SUMSCEAMEM:%d,MAXSCEMEM:%d\n", 
											LoadOmpStat.sceInfo[i].avg_cpu[0],
											LoadOmpStat.sceInfo[i].max_cpu[0],
											LoadOmpStat.sceInfo[i].avg_cpu[1],
											LoadOmpStat.sceInfo[i].max_cpu[1],
											LoadOmpStat.sceInfo[i].avg_cpu[2],
											LoadOmpStat.sceInfo[i].max_cpu[2],
											LoadOmpStat.sceInfo[i].avg_mem[0],
											LoadOmpStat.sceInfo[i].max_mem[0],
											LoadOmpStat.sceInfo[i].avg_mem[1],
											LoadOmpStat.sceInfo[i].max_mem[1],
											LoadOmpStat.sceInfo[i].avg_mem[2],
											LoadOmpStat.sceInfo[i].max_mem[2],
											sumSceMem[1],
											maxSceMem[1] );
					trclib_writeLogErr (FL,trcBuf);
				}
			}


            sprintf(query, "INSERT INTO %s VALUES ('%s', '%s',"
                " %d, %d, %d, %d, %d, %d, %d, %d,"
                " %d, %d,"
                " %d, %d, %d, %d, "
                " %d, '%s', '%s' )", 
                STM_STATISTIC_5MINUTE_LOAD_TBL_NAME, "SCE", sceName,

                LoadOmpStat.sceInfo[i].avg_cpu[0],
                LoadOmpStat.sceInfo[i].max_cpu[0],
                LoadOmpStat.sceInfo[i].avg_cpu[1],
                LoadOmpStat.sceInfo[i].max_cpu[1],
                LoadOmpStat.sceInfo[i].avg_cpu[2],
                LoadOmpStat.sceInfo[i].max_cpu[2],
//                LoadOmpStat.sceInfo[i].avg_cpu[3],
//                LoadOmpStat.sceInfo[i].max_cpu[3],
				0, // sce 는 cpu가 3개 , 4번째는 0값으로 셋팅
				0, // sce 는 cpu가 3개 , 4번째는 0값으로 셋팅

//                LoadOmpStat.sceInfo[i].avg_mem[0],
//                LoadOmpStat.sceInfo[i].max_mem[0],
				avgSceMem[i], // sce 는 mem가 3개 , 칼럼은 1개
				maxSceMem[i], // sce 는 mem가 3개 , 칼럼은 1개

                LoadOmpStat.sceInfo[i].avg_disk,
                LoadOmpStat.sceInfo[i].max_disk,

                0, 0, 

                1, get_insert_time(), get_insert_week());

//			logPrint(trcLogId, FL, "SCE INSERT : %s\n",query);

            if ( trcLogFlag == TRCLEVEL_SQL ) {
                sprintf( trcBuf, "query = %s\n", query);
                trclib_writeLog(FL, trcBuf);
            }

            if (stmd_mysql_query (query) < 0) {
                sprintf(trcBuf, "LoadStatistic 5M mysql_query fail:%s\n", query);
                trclib_writeLogErr (FL,trcBuf);
                return -1;
            }

//			sprintf( trcBuf, "query = %s\n", query);
//			trclib_writeLogErr(FL, trcBuf);

        } //end for
    } // end if_else

    return 1;
}

int stmd_hdlFltStatisticRpt(IxpcQMsgType *rxIxpcMsg)
{
   STM_AlarmStatisticMsgType   AlarmStatistic;
    char    query[4096];
    int     cnt_val, i;
    char    sysType[COMM_MAX_NAME_LEN];
    char    pdName[COMM_MAX_NAME_LEN] = {0,}, sceName[COMM_MAX_NAME_LEN] = {0,};

    // 7일 전의 5분데이타를 지운다.
	/*
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
            STM_STATISTIC_5MINUTE_FAULT_TBL_NAME, get_delete_time(STMD_1HOUR_OFFSET*7));
			*/
    sprintf(query, "DELETE FROM %s WHERE (stat_date < '%s')",
            STM_STATISTIC_5MINUTE_FAULT_TBL_NAME, get_delete_time(STMD_1DAY_OFFSET*delTIME[STMD_MIN]));

    if ( trcLogFlag == TRCLEVEL_SQL ) {
        sprintf( trcBuf, "query = %s\n", query);
        trclib_writeLog(FL, trcBuf);
    }
    if (stmd_mysql_query (query) < 0) {
        sprintf(trcBuf, "FaultStatistic 5M mysql_delete fail\n");
        trclib_writeLogErr (FL,trcBuf);
    }

    // 현재의 데이타를 삽입한다.
    memcpy((void *)&AlarmStatistic, rxIxpcMsg->body, sizeof(STM_AlarmStatisticMsgType));
//    sprintf(trcBuf, "sys[0]:%s,sys[1]:%s,sys[2]:%s\n", AlarmStatistic.sys[0].sysType,
//		AlarmStatistic.sys[1].sysType,AlarmStatistic.sys[2].sysType);
//    trclib_writeLogErr(FL, trcBuf);
    
	for ( cnt_val = 0 ; cnt_val < AlarmStatistic.eqSysCnt ; cnt_val++ ) {
		if(!strcasecmp(AlarmStatistic.sys[cnt_val].sysType, SYSCONF_SYSTYPE_BSD))
			strcpy(sysType, "MP");
		else
			strcpy(sysType, AlarmStatistic.sys[cnt_val].sysType);

#if 0
        logPrint(trcLogId,FL, "sys[%s] mem_min[%d] mem_maj[%d] mem_cri[%d] eg_min[%d] eg_maj[%d] eg_cri[%d]\n", AlarmStatistic.sys[cnt_val].sysName,
				AlarmStatistic.sys[cnt_val].comm.mem[0], AlarmStatistic.sys[cnt_val].comm.mem[1],                                                              
	            AlarmStatistic.sys[cnt_val].comm.mem[2],
	            AlarmStatistic.sys[cnt_val].comm.logonSuccRate[0],
	            AlarmStatistic.sys[cnt_val].comm.logonSuccRate[1],
	            AlarmStatistic.sys[cnt_val].comm.logonSuccRate[2]
				);
#endif
#ifdef _TPS_
        /* TPS(tpsCall) 추가, added by dcham 20110525 */ 
        sprintf(query, "INSERT INTO %s VALUES ('%s', '%s',"
            " %d, %d, %d,"
            " %d, %d, %d,"
            " %d, %d, %d,"
            " %d, %d, %d,"
//            " %d, %d, %d,"
//            " %d, %d, %d,"
//            " %d, %d, %d,"
            " %d, '%s', '%s' )", 
            STM_STATISTIC_5MINUTE_FAULT_TBL_NAME, sysType, AlarmStatistic.sys[cnt_val].sysName,

            AlarmStatistic.sys[cnt_val].comm.cpu[0],
            AlarmStatistic.sys[cnt_val].comm.cpu[1],
            AlarmStatistic.sys[cnt_val].comm.cpu[2],

            AlarmStatistic.sys[cnt_val].comm.mem[0],
            AlarmStatistic.sys[cnt_val].comm.mem[1],
            AlarmStatistic.sys[cnt_val].comm.mem[2],

// by helca 09.13 cpu, mem를 제외한 H/W 장애는 etc_hw_cnt에 포함. (disk, lan, mirroring_port, mp_hw) 
            AlarmStatistic.sys[cnt_val].comm.disk[0]+ \
			AlarmStatistic.sys[cnt_val].comm.lan[0]+ \
			AlarmStatistic.sys[cnt_val].comm.optlan[0]+ \
			AlarmStatistic.sys[cnt_val].comm.mp_hw[0]+ \
			AlarmStatistic.sys[cnt_val].spec.u.bsd.scm_fault[0]+ \
			AlarmStatistic.sys[cnt_val].spec.u.bsd.net_uawap[0]+ \
			AlarmStatistic.sys[cnt_val].spec.u.bsd.net_aaa[0]+ \
			AlarmStatistic.sys[cnt_val].comm.logonSuccRate[0]+ \
			AlarmStatistic.sys[cnt_val].comm.logoutSuccRate[0]+ \
			AlarmStatistic.sys[cnt_val].comm.smCh[0][0]+ \
			AlarmStatistic.sys[cnt_val].comm.smCh[1][0]+ \
			AlarmStatistic.sys[cnt_val].comm.smCh[2][0]+ \
			AlarmStatistic.sys[cnt_val].comm.smCh[3][0]+ \
			AlarmStatistic.sys[cnt_val].comm.smCh[4][0]+ \
			AlarmStatistic.sys[cnt_val].comm.legSession[0]+ \
			AlarmStatistic.sys[cnt_val].comm.tpsCall[0],

            AlarmStatistic.sys[cnt_val].comm.disk[1]+ \
			AlarmStatistic.sys[cnt_val].comm.lan[1]+ \
			AlarmStatistic.sys[cnt_val].comm.optlan[1]+ \
			AlarmStatistic.sys[cnt_val].comm.mp_hw[1]+ \
			AlarmStatistic.sys[cnt_val].spec.u.bsd.scm_fault[1]+ \
			AlarmStatistic.sys[cnt_val].spec.u.bsd.net_uawap[1]+ \
			AlarmStatistic.sys[cnt_val].spec.u.bsd.net_aaa[1]+ \
			AlarmStatistic.sys[cnt_val].comm.logonSuccRate[1]+ \
			AlarmStatistic.sys[cnt_val].comm.logoutSuccRate[1]+ \
			AlarmStatistic.sys[cnt_val].comm.smCh[0][1]+ \
			AlarmStatistic.sys[cnt_val].comm.smCh[1][1]+ \
			AlarmStatistic.sys[cnt_val].comm.smCh[2][1]+ \
			AlarmStatistic.sys[cnt_val].comm.smCh[3][1]+ \
			AlarmStatistic.sys[cnt_val].comm.smCh[4][1]+ \
			AlarmStatistic.sys[cnt_val].comm.legSession[1]+ \
			AlarmStatistic.sys[cnt_val].comm.tpsCall[1],

            AlarmStatistic.sys[cnt_val].comm.disk[2]+ \
			AlarmStatistic.sys[cnt_val].comm.lan[2]+ \
			AlarmStatistic.sys[cnt_val].comm.optlan[2]+ \
			AlarmStatistic.sys[cnt_val].comm.mp_hw[2]+ \
			AlarmStatistic.sys[cnt_val].spec.u.bsd.scm_fault[2]+ \
			AlarmStatistic.sys[cnt_val].spec.u.bsd.net_uawap[2]+ \
			AlarmStatistic.sys[cnt_val].spec.u.bsd.net_aaa[2]+ \
			AlarmStatistic.sys[cnt_val].comm.logonSuccRate[2]+ \
			AlarmStatistic.sys[cnt_val].comm.logoutSuccRate[2]+ \
			AlarmStatistic.sys[cnt_val].comm.smCh[0][2]+ \
			AlarmStatistic.sys[cnt_val].comm.smCh[1][2]+ \
			AlarmStatistic.sys[cnt_val].comm.smCh[2][2]+ \
			AlarmStatistic.sys[cnt_val].comm.smCh[3][2]+ \
			AlarmStatistic.sys[cnt_val].comm.smCh[4][2]+ \
			AlarmStatistic.sys[cnt_val].comm.legSession[2]+ \
			AlarmStatistic.sys[cnt_val].comm.tpsCall[2],


            AlarmStatistic.sys[cnt_val].comm.proc[0],
            AlarmStatistic.sys[cnt_val].comm.proc[1],
            AlarmStatistic.sys[cnt_val].comm.proc[2],

//            AlarmStatistic.sys[cnt_val].comm.net_nms[0],
//            AlarmStatistic.sys[cnt_val].comm.net_nms[1],
//            AlarmStatistic.sys[cnt_val].comm.net_nms[2],

// disable jjinri 2009.04.22            AlarmStatistic.sys[cnt_val].comm.sess_ntp[0],
// disable jjinri 2009.04.22            AlarmStatistic.sys[cnt_val].comm.sess_ntp[1],
// disable jjinri 2009.04.22            AlarmStatistic.sys[cnt_val].comm.sess_ntp[2],
//            AlarmStatistic.sys[cnt_val].comm.sess_ntp[0][0], // modify jjinri
//            AlarmStatistic.sys[cnt_val].comm.sess_ntp[1][1], // modify jjinri
//            AlarmStatistic.sys[cnt_val].comm.sess_ntp[2][2], // modify jjinri

//            AlarmStatistic.sys[cnt_val].comm.sess_nms[0],
//            AlarmStatistic.sys[cnt_val].comm.sess_nms[1],
//            AlarmStatistic.sys[cnt_val].comm.sess_nms[2],

            1, get_insert_time(), get_insert_week());
#else
        sprintf(query, "INSERT INTO %s VALUES ('%s', '%s',"
            " %d, %d, %d,"
            " %d, %d, %d,"
            " %d, %d, %d,"
            " %d, %d, %d,"
//            " %d, %d, %d,"
//            " %d, %d, %d,"
//            " %d, %d, %d,"
            " %d, '%s', '%s' )", 
            STM_STATISTIC_5MINUTE_FAULT_TBL_NAME, sysType, AlarmStatistic.sys[cnt_val].sysName,

            AlarmStatistic.sys[cnt_val].comm.cpu[0],
            AlarmStatistic.sys[cnt_val].comm.cpu[1],
            AlarmStatistic.sys[cnt_val].comm.cpu[2],

            AlarmStatistic.sys[cnt_val].comm.mem[0],
            AlarmStatistic.sys[cnt_val].comm.mem[1],
            AlarmStatistic.sys[cnt_val].comm.mem[2],

// by helca 09.13 cpu, mem를 제외한 H/W 장애는 etc_hw_cnt에 포함. (disk, lan, mirroring_port, mp_hw) 
            AlarmStatistic.sys[cnt_val].comm.disk[0]+ \
			AlarmStatistic.sys[cnt_val].comm.lan[0]+ \
			AlarmStatistic.sys[cnt_val].comm.optlan[0]+ \
			AlarmStatistic.sys[cnt_val].comm.mp_hw[0]+ \
			AlarmStatistic.sys[cnt_val].spec.u.bsd.scm_fault[0]+ \
			AlarmStatistic.sys[cnt_val].spec.u.bsd.net_uawap[0]+ \
			AlarmStatistic.sys[cnt_val].spec.u.bsd.net_aaa[0]+ \
			AlarmStatistic.sys[cnt_val].comm.logonSuccRate[0]+ \
			AlarmStatistic.sys[cnt_val].comm.logoutSuccRate[0]+ \
			AlarmStatistic.sys[cnt_val].comm.smCh[0][0]+ \
			AlarmStatistic.sys[cnt_val].comm.smCh[1][0]+ \
			AlarmStatistic.sys[cnt_val].comm.smCh[2][0]+ \
			AlarmStatistic.sys[cnt_val].comm.smCh[3][0]+ \
			AlarmStatistic.sys[cnt_val].comm.smCh[4][0]+ \
			AlarmStatistic.sys[cnt_val].comm.legSession[0],

            AlarmStatistic.sys[cnt_val].comm.disk[1]+ \
			AlarmStatistic.sys[cnt_val].comm.lan[1]+ \
			AlarmStatistic.sys[cnt_val].comm.optlan[1]+ \
			AlarmStatistic.sys[cnt_val].comm.mp_hw[1]+ \
			AlarmStatistic.sys[cnt_val].spec.u.bsd.scm_fault[1]+ \
			AlarmStatistic.sys[cnt_val].spec.u.bsd.net_uawap[1]+ \
			AlarmStatistic.sys[cnt_val].spec.u.bsd.net_aaa[1]+ \
			AlarmStatistic.sys[cnt_val].comm.logonSuccRate[1]+ \
			AlarmStatistic.sys[cnt_val].comm.logoutSuccRate[1]+ \
			AlarmStatistic.sys[cnt_val].comm.smCh[0][1]+ \
			AlarmStatistic.sys[cnt_val].comm.smCh[1][1]+ \
			AlarmStatistic.sys[cnt_val].comm.smCh[2][1]+ \
			AlarmStatistic.sys[cnt_val].comm.smCh[3][1]+ \
			AlarmStatistic.sys[cnt_val].comm.smCh[4][1]+ \
			AlarmStatistic.sys[cnt_val].comm.legSession[1],

            AlarmStatistic.sys[cnt_val].comm.disk[2]+ \
			AlarmStatistic.sys[cnt_val].comm.lan[2]+ \
			AlarmStatistic.sys[cnt_val].comm.optlan[2]+ \
			AlarmStatistic.sys[cnt_val].comm.mp_hw[2]+ \
			AlarmStatistic.sys[cnt_val].spec.u.bsd.scm_fault[2]+ \
			AlarmStatistic.sys[cnt_val].spec.u.bsd.net_uawap[2]+ \
			AlarmStatistic.sys[cnt_val].spec.u.bsd.net_aaa[2]+ \
			AlarmStatistic.sys[cnt_val].comm.logonSuccRate[2]+ \
			AlarmStatistic.sys[cnt_val].comm.logoutSuccRate[2]+ \
			AlarmStatistic.sys[cnt_val].comm.smCh[0][2]+ \
			AlarmStatistic.sys[cnt_val].comm.smCh[1][2]+ \
			AlarmStatistic.sys[cnt_val].comm.smCh[2][2]+ \
			AlarmStatistic.sys[cnt_val].comm.smCh[3][2]+ \
			AlarmStatistic.sys[cnt_val].comm.smCh[4][2]+ \
			AlarmStatistic.sys[cnt_val].comm.legSession[2],

            AlarmStatistic.sys[cnt_val].comm.proc[0],
            AlarmStatistic.sys[cnt_val].comm.proc[1],
            AlarmStatistic.sys[cnt_val].comm.proc[2],

//            AlarmStatistic.sys[cnt_val].comm.net_nms[0],
//            AlarmStatistic.sys[cnt_val].comm.net_nms[1],
//            AlarmStatistic.sys[cnt_val].comm.net_nms[2],

// disable jjinri 2009.04.22            AlarmStatistic.sys[cnt_val].comm.sess_ntp[0],
// disable jjinri 2009.04.22            AlarmStatistic.sys[cnt_val].comm.sess_ntp[1],
// disable jjinri 2009.04.22            AlarmStatistic.sys[cnt_val].comm.sess_ntp[2],
//            AlarmStatistic.sys[cnt_val].comm.sess_ntp[0][0], // modify jjinri
//            AlarmStatistic.sys[cnt_val].comm.sess_ntp[1][1], // modify jjinri
//            AlarmStatistic.sys[cnt_val].comm.sess_ntp[2][2], // modify jjinri

//            AlarmStatistic.sys[cnt_val].comm.sess_nms[0],
//            AlarmStatistic.sys[cnt_val].comm.sess_nms[1],
//            AlarmStatistic.sys[cnt_val].comm.sess_nms[2],

            1, get_insert_time(), get_insert_week());
#endif

        if ( trcLogFlag == TRCLEVEL_SQL ) {
            logPrint( trcLogId, FL, "query = %s\n", query);
        }
		logPrint( trcLogId, FL, "QUERY = %s\n", query);
            
        if (stmd_mysql_query (query) < 0) {
            sprintf(trcBuf, "FaultStatistic 5M mysql_query insert fail: %s\n", query);
            trclib_writeLogErr (FL,trcBuf);
            return -1;
        }

        if (!strcasecmp(AlarmStatistic.sys[cnt_val].sysType, SYSCONF_SYSTYPE_BSD)) {
        } 
		else 
		{
			// Proving Device 
            for(i =0; i< 2; i++)
			{
                if (i ==0)
                    strcpy(pdName, "TAPA");
                else
                    strcpy(pdName, "TAPB");

                sprintf(query, "INSERT INTO %s VALUES ('%s', '%s',"
                    " %d, %d, %d,"
                    " %d, %d, %d,"
                    " %d, %d, %d,"
                    " %d, %d, %d,"
//                    " %d, %d, %d,"
//                    " %d, %d, %d,"
//                    " %d, %d, %d,"
                    " %d, '%s', '%s' )", 
                    STM_STATISTIC_5MINUTE_FAULT_TBL_NAME, "TAP", pdName,

                    AlarmStatistic.sys[cnt_val].comm.pdcpu[i][0],
                    AlarmStatistic.sys[cnt_val].comm.pdcpu[i][1],
                    AlarmStatistic.sys[cnt_val].comm.pdcpu[i][2],

                    AlarmStatistic.sys[cnt_val].comm.pdmem[i][0],
                    AlarmStatistic.sys[cnt_val].comm.pdmem[i][1],
                    AlarmStatistic.sys[cnt_val].comm.pdmem[i][2],

                    AlarmStatistic.sys[cnt_val].comm.pdfan[i][0]+AlarmStatistic.sys[cnt_val].comm.pdgiga[i][0]+AlarmStatistic.sys[cnt_val].comm.pdpower[i][0],
                    AlarmStatistic.sys[cnt_val].comm.pdfan[i][1]+AlarmStatistic.sys[cnt_val].comm.pdgiga[i][1]+AlarmStatistic.sys[cnt_val].comm.pdpower[i][1],
                    AlarmStatistic.sys[cnt_val].comm.pdfan[i][2]+AlarmStatistic.sys[cnt_val].comm.pdgiga[i][2]+AlarmStatistic.sys[cnt_val].comm.pdpower[i][2],

                    0, 0, 0,
//                    0, 0, 0,
//                    0, 0, 0,
//                    0, 0, 0,
                    1, get_insert_time(), get_insert_week());

                if ( trcLogFlag == TRCLEVEL_SQL ) {
                    sprintf( trcBuf, "query = %s\n", query);
                    trclib_writeLog(FL, trcBuf);
                }

                if (stmd_mysql_query (query) < 0) {
                    sprintf(trcBuf, "FaultStatistic 5M mysql_query insert fail: %s\n", query);
                    trclib_writeLogErr (FL,trcBuf);
                    return -1;
                }
				
            } //end for
			// SCE Cisco
            for(i =0; i< 2; i++){
                if (i ==0)
                    strcpy(sceName, "SCEA");
                else
                    strcpy(sceName, "SCEB");

                sprintf(query, "INSERT INTO %s VALUES ('%s', '%s',"
                    " %d, %d, %d,"
                    " %d, %d, %d,"
                    " %d, %d, %d,"
                    " %d, %d, %d,"
//                    " %d, %d, %d,"
//                    " %d, %d, %d,"
//                    " %d, %d, %d,"
                    " %d, '%s', '%s' )", 
                    STM_STATISTIC_5MINUTE_FAULT_TBL_NAME, "SCE", sceName,

                    AlarmStatistic.sys[cnt_val].comm.sceCpu[i][0],
                    AlarmStatistic.sys[cnt_val].comm.sceCpu[i][1],
                    AlarmStatistic.sys[cnt_val].comm.sceCpu[i][2],

                    AlarmStatistic.sys[cnt_val].comm.sceMem[i][0],
                    AlarmStatistic.sys[cnt_val].comm.sceMem[i][1],
                    AlarmStatistic.sys[cnt_val].comm.sceMem[i][2],

                    AlarmStatistic.sys[cnt_val].comm.sceDisk[i][0]+AlarmStatistic.sys[cnt_val].comm.scePwr[i][0] + \
                    AlarmStatistic.sys[cnt_val].comm.sceFan[i][0]+AlarmStatistic.sys[cnt_val].comm.sceTemp[i][0] + \
                    AlarmStatistic.sys[cnt_val].comm.sceVolt[i][0]+AlarmStatistic.sys[cnt_val].comm.scePort[i][0] + \
                    AlarmStatistic.sys[cnt_val].comm.sceRdr[i][0]+AlarmStatistic.sys[cnt_val].comm.sceStatus[i][0] + \
                    AlarmStatistic.sys[cnt_val].comm.sceRdrConn[i][0]+AlarmStatistic.sys[cnt_val].comm.sceUser[i][0], \

                    AlarmStatistic.sys[cnt_val].comm.sceDisk[i][1]+AlarmStatistic.sys[cnt_val].comm.scePwr[i][1] + \
                    AlarmStatistic.sys[cnt_val].comm.sceFan[i][1]+AlarmStatistic.sys[cnt_val].comm.sceTemp[i][1] + \
                    AlarmStatistic.sys[cnt_val].comm.sceVolt[i][1]+AlarmStatistic.sys[cnt_val].comm.scePort[i][1] + \
                    AlarmStatistic.sys[cnt_val].comm.sceRdr[i][1]+AlarmStatistic.sys[cnt_val].comm.sceStatus[i][1] + \
                    AlarmStatistic.sys[cnt_val].comm.sceRdrConn[i][1]+AlarmStatistic.sys[cnt_val].comm.sceUser[i][1], \

                    AlarmStatistic.sys[cnt_val].comm.sceDisk[i][2]+AlarmStatistic.sys[cnt_val].comm.scePwr[i][2] + \
                    AlarmStatistic.sys[cnt_val].comm.sceFan[i][2]+AlarmStatistic.sys[cnt_val].comm.sceTemp[i][2] + \
                    AlarmStatistic.sys[cnt_val].comm.sceVolt[i][2]+AlarmStatistic.sys[cnt_val].comm.scePort[i][2] + \
                    AlarmStatistic.sys[cnt_val].comm.sceRdr[i][2]+AlarmStatistic.sys[cnt_val].comm.sceStatus[i][2] + \
                    AlarmStatistic.sys[cnt_val].comm.sceRdrConn[i][2]+AlarmStatistic.sys[cnt_val].comm.sceUser[i][2], \

                    0, 0, 0,
//                    0, 0, 0,
//                    0, 0, 0,
//                    0, 0, 0,
                    1, get_insert_time(), get_insert_week());

                if ( trcLogFlag == TRCLEVEL_SQL ) {
                    sprintf( trcBuf, "query = %s\n", query);
                    trclib_writeLog(FL, trcBuf);
                }

                if (stmd_mysql_query (query) < 0) {
                    sprintf(trcBuf, "FaultStatistic 5M mysql_query insert fail: %s\n", query);
                    trclib_writeLogErr (FL,trcBuf);
                    return -1;
                }
            } //end for SCE 

        } // end if_else

    } // end for

    return 1;
}
