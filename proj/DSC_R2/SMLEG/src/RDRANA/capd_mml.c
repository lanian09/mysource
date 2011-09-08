#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "capd_mml.h"
#include "comm_smpp_if.h"
#include <mysql.h>
#include "comm_session.h"

char	resBuf[4096], resHead[4096], resTmp[1024];
extern unsigned char           ruleSetList[MAX_PKG_ID_NUMBER];
extern RuleSetList				g_stRule[MAX_RULE_SET_LIST];
extern RuleEntryList			g_stEntry[MAX_RULE_ENTRY_LIST];
int MakeRuleSetList(char *fname);

int Send_CondTrcMsg_BLOCK(RDR_BLOCK stBlock, int traceType, char *ip);
int Send_CondTrcMsg_TR(RDR_TR stTR, int traceType, char *ip);

extern int connectDB(void);
extern int sms_selectDB(void);
extern int writeSmsFile(void);
extern int readSmsFile(void);

char *str_time(time_t t);
 
/* DEL : by june, 2010-10-03
 * DESC: TRACE �� ���Ǵ� SESSION ���� ���� ��Ʈ �ּ� ó��
 */
#if 0
extern char * find_imsi_rad_sess (rad_sess_key *pkey);
#endif
extern SMS_INFO stSms;

extern MYSQL	sql, *conn;
#define MAX_SCE_NUM 2
SCE_LIST		g_stSce[MAX_SCE_NUM];

int rdrana_mmcHdlrVector_bsrchCmp (const void *a, const void *b)
{
    return (strcasecmp ((char*)a, ((RDRMmcHdlrVector*)b)->cmdName));
} 

int rdrana_mmcHdlrVector_qsortCmp (const void *a, const void *b)
{
    return (strcasecmp (((RDRMmcHdlrVector*)a)->cmdName, ((RDRMmcHdlrVector*)b)->cmdName));
} 



int getMMCMsg( IxpcQMsgType *rxIxpcMsg )
{
	MMLReqMsgType       *mmlReqMsg;
	RDRMmcHdlrVector    *mmcHdlr;

	mmlReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
	if ((mmcHdlr = (RDRMmcHdlrVector*) bsearch (
			mmlReqMsg->head.cmdName,
			mmcHdlrVector,
			numMmcHdlr,
			sizeof(RDRMmcHdlrVector),
			rdrana_mmcHdlrVector_bsrchCmp)) == NULL)
	{
		dAppLog(LOG_CRI,"[getMMCMsg] received unknown mml_cmd(:%s:)", mmlReqMsg->head.cmdName);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	(int)(*(mmcHdlr->func)) (rxIxpcMsg);

	return 1;
}

int txMMCResult (IxpcQMsgType *rxIxpcMsg, char *resBuf, char resCode, char contFlag)
{                   
    GeneralQMsgType txGenQMsg;
    IxpcQMsgType    *txIxpcMsg;
    MMLResMsgType   *txResMsg;
    MMLReqMsgType   *rxReqMsg;
    int             txLen;
    char    totalBuf[1024];
                
    sprintf( totalBuf, "    SYSTEM = %s\n    APP = %s\n    Success Rule Set Sync.\n", mySysName,myAppName );
                
    txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
    memset ((void*)&txIxpcMsg->head, 0, sizeof(txIxpcMsg->head));
            
    txResMsg = (MMLResMsgType*)txIxpcMsg->body;

    txGenQMsg.mtype = MTYPE_MMC_RESPONSE;
    
    strcpy (txIxpcMsg->head.srcSysName, rxIxpcMsg->head.dstSysName);
    strcpy (txIxpcMsg->head.srcAppName, rxIxpcMsg->head.dstAppName);
    strcpy (txIxpcMsg->head.dstSysName, rxIxpcMsg->head.srcSysName);
    strcpy (txIxpcMsg->head.dstAppName, rxIxpcMsg->head.srcAppName);
    txIxpcMsg->head.segFlag = 0;
    txIxpcMsg->head.seqNo = 1;
            
    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
    txResMsg->head.mmcdJobNo = rxReqMsg->head.mmcdJobNo;
    if(contFlag)
    {       
        txResMsg->head.extendTime = htons(5);
    }
    txResMsg->head.resCode = resCode;
    txResMsg->head.contFlag = contFlag;
    strcpy(txResMsg->head.cmdName, rxReqMsg->head.cmdName);
    strcat( totalBuf,resBuf );
    strcpy(txResMsg->body, totalBuf);

    txIxpcMsg->head.bodyLen = sizeof(txResMsg->head) + strlen(txResMsg->body);
    txLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;

    if (msgsnd(dIxpcQid, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0) {
        sprintf(trcBuf, "msgsnd error = %s, cmd = %s\n", strerror(errno), txResMsg->head.cmdName);
        trclib_writeLogErr(FL,trcBuf);
        return -1;
    }
    return 1;
}

int rdrana_mmc_sync_sms_msg (IxpcQMsgType *rxIxpcMsg)
{

	if( connectDB() < 0 )
	{
		dAppLog(LOG_INFO, "connect sms_msg table fail");
	}
	else
	{
		if( sms_selectDB() < 0 )
		{
			dAppLog(LOG_INFO, "select sms_msg data null");
		}
		else
		{
			writeSmsFile();
			dAppLog(LOG_INFO, "sync sms msg() Success Read...");
		}
	}

	return 0;
}


int rdrana_mmc_sync_rule_file2 (IxpcQMsgType *rxIxpcMsg)
{
	char resBuf[256] = {0,};
	char fname[64] = {0,};
	int ret;

	memset(fname, 0x00, sizeof(fname));
	sprintf(fname, "%s", PBTABLE_PATH);
	ret = MakeRuleSetList(fname);
	if( ret < 0 )
	{
		dAppLog(LOG_CRI, "MakeRuleSetList() fail...");
		return -1;
	}
	dAppLog(LOG_CRI, "MakeRuleSetList() Success Read...");

	txMMCResult(rxIxpcMsg, resBuf, 0, 0);
	
	return 0;
}

int RetryDBConn(void)
{
	unsigned int mysql_timeout = 5;
	if( conn == NULL )
	{
		mysql_init(&sql);
		mysql_options(&sql, MYSQL_OPT_CONNECT_TIMEOUT, &mysql_timeout);
		if ((conn = mysql_real_connect (&sql, "localhost", "root", "mysql", "mysql", 0, 0, 0)) == NULL)
		{
			dAppLog( LOG_CRI, "mysql connection fail");
			return -1;
		}
		else
		{
			dAppLog(LOG_WARN, "Reconnect Select Query Success.");
			return 0;
		}
	}
	else
	{
		mysql_close(conn);
		mysql_init(&sql);
		mysql_options(&sql, MYSQL_OPT_CONNECT_TIMEOUT, &mysql_timeout);
		if ((conn = mysql_real_connect (&sql, "localhost", "root", "mysql", "mysql", 0, 0, 0)) == NULL)
		{
			dAppLog( LOG_CRI, "mysql connection fail");
			return -1;
		}
		else
		{
			dAppLog(LOG_WARN, "Reconnect Select Query Success.");
			return 0;
		}
	}
}

int Send_CondTrcMsg_TR(RDR_TR stTR, int traceType, char *ip)
{
	GeneralQMsgType txGenQMsg;
	IxpcQMsgType	*txIxpcMsg = NULL;
	char 			szTrcMsg[4096], szTemp[256];
	int				dMsgLen, dTxLen;
	struct tm 		*pTime;
	struct timeval	stTmval;
	char			sztimebuf[128];
	int				i, ret = 0;
/* DEL : by june, 2010-10-03
	rad_sess_key stRadKey;
	char *radIMSI;
 */

	MYSQL_RES *result;
	MYSQL_ROW row;
	char			query[128] = {0,};

	memset(&stTmval, 0x00, sizeof(struct timeval));
	memset(&sztimebuf, 0x00, sizeof(sztimebuf));
	memset(&txGenQMsg, 0x00, sizeof(txGenQMsg));
	memset(szTrcMsg, 0x00, 4096);

	txGenQMsg.mtype = MTYPE_TRC_CONSOLE;
	txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
	memset((void*)&txIxpcMsg->head, 0x00, sizeof(txIxpcMsg->head));

	strcpy(txIxpcMsg->head.srcSysName, mySysName);
	strcpy(txIxpcMsg->head.srcAppName, myAppName);
	strcpy(txIxpcMsg->head.dstSysName, "DSCM");
	strcpy(txIxpcMsg->head.dstAppName, "COND");

	gettimeofday(&stTmval, NULL );
	sprintf( sztimebuf,"[%s.%06ld]", str_time(stTmval.tv_sec), stTmval.tv_usec);

	// TR RDR Msg
	/* TODO : �ؿ� szTrcMsg IMSI �����ִ� �κ� ���� �Ǿ�� ��. */
	if( traceType == TYPE_IMSI )
		snprintf(szTrcMsg, 4096, "IMSI:%s\n", stTR.ucSubscriberID);
	else
		snprintf(szTrcMsg, 4096, "IP:%s\n", ip);


	snprintf(szTrcMsg, 4096, "%s%s\t%s\nS7000 CALL TRACE INFORMATION(RDRANA: TRANSACTION RDR )\n------------------------------------------------------------\n",
							szTrcMsg, mySysName, sztimebuf);

	if( traceType == 3 ) // Server IP �̸� IMSI �� �� �����ش�. 
	{
		snprintf(szTrcMsg, 4096, "%s%s", szTrcMsg, "\tIMSI:");
		if( !strcmp(stTR.ucSubscriberID, "N/A") )
		{
/* DEL : by june, 2010-10-03
 * DESC: TRACE �� ���Ǵ� SESSION ���� ���� ��Ʈ �ּ� ó��
 */
#if 0
			snprintf(szTrcMsg, 4096, "%s%s\n", szTrcMsg, "Unknown Subscriber");
			stRadKey.mobIP = stTR.uiClientIP;
			radIMSI = find_imsi_rad_sess(&stRadKey);
			if( radIMSI == NULL )
				snprintf(szTrcMsg, 4096, "%s%s\n", szTrcMsg, "Unknown Subscriber");
			else
				snprintf(szTrcMsg, 4096, "%s%s\n", szTrcMsg, radIMSI);
#else
			snprintf(szTrcMsg, 4096, "%s%s\n", szTrcMsg, "Unknown Subscriber");
#endif
		}
		else
			snprintf(szTrcMsg, 4096, "%s%s\n", szTrcMsg, stTR.ucSubscriberID);
	}
	snprintf(szTrcMsg, 4096, "%s%s", szTrcMsg, "\tSCE:");
	for( i = 0; i < MAX_SCE_NUM; i++ )
	{
		if( !strcmp(g_stSce[i].sce_ip, stTR.szSceIp) )
		{
			snprintf(szTrcMsg, 4096, "%s%s\n", szTrcMsg, g_stSce[i].sce_name);
			break;
		}
	}
	if( i == MAX_SCE_NUM ) // sce_ip �� ��ġ�Ǵ� ���� ���� ���� ���� ����. 
	{
		// SCE NAME
		sprintf(query, "SELECT value_key FROM INI_VALUES WHERE value_type = 5 "
				" AND se_ip = '%s'", stTR.szSceIp);

		if (mysql_query (conn, query) != 0)
		{
			dAppLog(LOG_WARN, "TR RDR: Select Query Fail.");
			ret = RetryDBConn();
			if( ret == 0 ) // success connection
			{
				if (mysql_query (conn, query) != 0)
					ret = -1;
				else
					ret = 0;
			}
		}
		else
			ret = 0; // SUCCESS

		if( ret == 0 )
		{
			result = mysql_store_result(&sql);
			if( result == NULL )
				dAppLog(LOG_WARN, "TR RDR: mysql_store_result fail.");
			else 
			{
				row = mysql_fetch_row(result);
				if(row == NULL )
					dAppLog(LOG_WARN, "TR RDR: mysql_fetch_row fail.");
				else
					snprintf(szTrcMsg, 4096, "%s%s\n", szTrcMsg, row[0]);
			}
			mysql_free_result(result);
		}
	}

	snprintf(szTrcMsg, 4096, "%s%s", szTrcMsg, "\tTime:");
	// TIME
	if((pTime = (struct tm*)localtime((time_t*)&stTR.uiTime)) == NULL)
		strcpy(szTemp, " ");
	else
		strftime(szTemp, 32, "%Y-%m-%d %H:%M:%S", pTime);

	snprintf(szTrcMsg, 4096, "%s%s\n", szTrcMsg, szTemp);

	// PKG ID
	snprintf(szTrcMsg, 4096, "%s%s", szTrcMsg, "\tRule Set ID:");
	snprintf(szTrcMsg, 4096, "%sPbit=%02d,Hbit=%02d\n", szTrcMsg, g_stRule[stTR.usPackageID].pBit,
			g_stRule[stTR.usPackageID].hBit);

	// Service ID
	snprintf(szTrcMsg, 4096, "%s%s", szTrcMsg, "\tRule Set Entry :");
	if( g_stEntry[stTR.dServiceID].entName[0] != 0x00 )
	{
		dAppLog(LOG_DEBUG, "Known ENTRY -> %d : %s", stTR.dServiceID, g_stEntry[stTR.dServiceID].entName);
		snprintf(szTrcMsg, 4096, "%s%s(%d)\n", szTrcMsg, g_stEntry[stTR.dServiceID].entName,stTR.dServiceID);
	}
	else
	{
		if( conn != NULL )
		{
			// Service Entry NAME
			sprintf(query, "SELECT DISTINCT value_key FROM INI_VALUES WHERE value_type = 1 "
					" AND value = %d", stTR.dServiceID);

			if (mysql_query (conn, query) != 0)
			{
				dAppLog(LOG_WARN, "TR Select Query Fail.");
				if( conn == NULL )
				{
					mysql_init(&sql);
					if ((conn = mysql_real_connect (&sql, "localhost", "root", "mysql", "mysql", 0, 0, 0)) == NULL)
						dAppLog( LOG_CRI, "mysql connection fail");

					if( mysql_query( conn,query) != 0 )
						dAppLog(LOG_WARN, "Reconnect Select Query Fail.");
					else
						dAppLog(LOG_WARN, "Reconnect Select Query Success.");
				}
				else
				{
					mysql_close(conn);
					mysql_init(&sql);
					if ((conn = mysql_real_connect (&sql, "localhost", "root", "mysql", "mysql", 0, 0, 0)) == NULL)
						dAppLog( LOG_CRI, "mysql connection fail");

					if( mysql_query( conn,query) != 0 )
						dAppLog(LOG_WARN, "Reconnect Select Query Fail.");
					else
						dAppLog(LOG_WARN, "Reconnect Select Query Success.");
				}
			}

			result = mysql_store_result(&sql);
			if( result == NULL )
				dAppLog(LOG_WARN, "mysql_store_result fail.");
			else
			{
				row = mysql_fetch_row(result);
				if(row == NULL )
					dAppLog(LOG_WARN, "mysql_fetch_row fail.");
				else
				{
					if( row[0] != NULL )
					{
						dAppLog(LOG_CRI, "Select ENTRY -> %d : %s", stTR.dServiceID, row[0]);
						snprintf(szTrcMsg, 4096, "%s%s(%d)\n", szTrcMsg, row[0],stTR.dServiceID);
						sprintf(g_stEntry[stTR.dServiceID].entName, "%s", row[0]);
					}
					else
						dAppLog(LOG_CRI, "Select Entry NULL...");
				}
			}
			mysql_free_result(result);
		}
	}

	// SERVER IP
	snprintf(szTrcMsg, 4096, "%s%s", szTrcMsg, "\tServer IP:");
	snprintf(szTrcMsg, 4096, "%s%s\n", szTrcMsg, stTR.szSvrIP);

	snprintf(szTrcMsg, 4096, "%s%s", szTrcMsg, "\tServer PORT:");
	snprintf(szTrcMsg, 4096, "%s%d\n", szTrcMsg, stTR.usServerPort);

	snprintf(szTrcMsg, 4096, "%s%s", szTrcMsg, "\tClient IP:");
	snprintf(szTrcMsg, 4096, "%s%s\n", szTrcMsg, stTR.szCliIP);

	snprintf(szTrcMsg, 4096, "%s%s", szTrcMsg, "\tClient PORT:");
	snprintf(szTrcMsg, 4096, "%s%d\n", szTrcMsg, stTR.usClientPort);


	dMsgLen = strlen(szTrcMsg);
	szTrcMsg[dMsgLen] = 0;
	dAppLog(LOG_DEBUG, "Len:%d\tMsg:%s", dMsgLen, szTrcMsg);

	txIxpcMsg->head.bodyLen = dMsgLen;
	memcpy(txIxpcMsg->body, szTrcMsg, dMsgLen);

	dTxLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;

	if( msgsnd(dIxpcQid, (void*)&txGenQMsg, dTxLen, IPC_NOWAIT) < 0 )
	{
		dAppLog(LOG_CRI, "[FAIL] SEND TO TRACE MSGQ[IXPC] ERROR %d(%s)", errno, strerror(errno));
		return -1;
	}
	else
	{
		dAppLog(LOG_DEBUG, "[SUCC] SEND TO TRACE MSGQ");
	}
	return 0;
}

int Send_CondTrcMsg_BLOCK(RDR_BLOCK stBlock, int traceType, char *ip)
{
	GeneralQMsgType txGenQMsg;
	IxpcQMsgType	*txIxpcMsg = NULL;
	char 			szTrcMsg[4096], szTemp[256];
	int				dMsgLen, dTxLen;
	struct tm 		*pTime;
	struct timeval	stTmval;
	char			sztimebuf[128];
	char			blkReason;
	int				ret = 0, i = 0;

	MYSQL_RES *result;
	MYSQL_ROW row;
	char			query[128] = {0,};

	memset(&stTmval, 0x00, sizeof(struct timeval));
	memset(&sztimebuf, 0x00, sizeof(sztimebuf));
	memset(&txGenQMsg, 0x00, sizeof(txGenQMsg));
	memset(szTrcMsg, 0x00, 4096);

	txGenQMsg.mtype = MTYPE_TRC_CONSOLE;
	txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
	memset((void*)&txIxpcMsg->head, 0x00, sizeof(txIxpcMsg->head));

	strcpy(txIxpcMsg->head.srcSysName, mySysName);
	strcpy(txIxpcMsg->head.srcAppName, myAppName);
	strcpy(txIxpcMsg->head.dstSysName, "DSCM");
	strcpy(txIxpcMsg->head.dstAppName, "COND");

	gettimeofday(&stTmval, NULL );
	sprintf( sztimebuf,"[%s.%06ld]", str_time(stTmval.tv_sec), stTmval.tv_usec);

	// Block RDR Msg
	/* TODO : �ؿ� szTrcMsg IMSI �����ִ� �κ� ���� �Ǿ�� ��. */
	if( traceType == TYPE_IMSI )
		snprintf(szTrcMsg, 4096, "IMSI:%s\n", stBlock.ucSubscriberID);
	else
		snprintf(szTrcMsg, 4096, "IP:%s\n", ip);

	if( traceType == 3 ) // Server IP �̸� IMSI �� �� �����ش�. 
	{
		snprintf(szTrcMsg, 4096, "%s%s", szTrcMsg, "\tIMSI:");
		snprintf(szTrcMsg, 4096, "%s%s\n", szTrcMsg, stBlock.ucSubscriberID);
	}

	snprintf(szTrcMsg, 4096, "%s%s\t%s\nS7000 CALL TRACE INFORMATION(RDRANA: Block RDR )\n------------------------------------------------------------\n",\
							szTrcMsg, mySysName, sztimebuf);
	snprintf(szTrcMsg, 4096, "%s%s", szTrcMsg, "\tSCE:");
	for( i = 0; i < MAX_SCE_NUM; i++ )
	{
		if( !strcmp(g_stSce[i].sce_ip, stBlock.szSceIp) )
		{
			snprintf(szTrcMsg, 4096, "%s%s\n", szTrcMsg, g_stSce[i].sce_name);
			break;
		}
	}
	if( i == MAX_SCE_NUM ) // sce_ip �� ��ġ�Ǵ� ���� ���� ���� ���� ����. 
	{
		if (mysql_query (conn, query) != 0)
		{
			dAppLog(LOG_WARN, "BLOCK RDR: Select Query Fail.");
			ret = RetryDBConn();
			if( ret == 0 )
			{
				if (mysql_query (conn, query) != 0)
					ret = 0; // success query
				else
					ret = -1; // fail query
			}
			else
				ret = -1; // fail reconnection

		}
		else
			ret = 0; // success query

		if( ret == 0 ) // success query
		{
			result = mysql_store_result(&sql);
			if( result == NULL )
				dAppLog(LOG_WARN, "BLOCK RDR: mysql_store_result fail.");
			else 
			{
				row = mysql_fetch_row(result);
				if(row == NULL )
					dAppLog(LOG_WARN, "BLOCK RDR: mysql_fetch_row fail.");
				else
					snprintf(szTrcMsg, 4096, "%s%s\n", szTrcMsg, row[0]);
			}
			mysql_free_result(result);
		}
	}

	snprintf(szTrcMsg, 4096, "%s%s", szTrcMsg, "\tTime:");
	// TIME
	if((pTime = (struct tm*)localtime((time_t*)&stBlock.uiTime)) == NULL)
		strcpy(szTemp, " ");
	else
		strftime(szTemp, 32, "%Y-%m-%d %H:%M:%S", pTime);

	snprintf(szTrcMsg, 4096, "%s%s\n", szTrcMsg, szTemp);

	// PKG ID
	snprintf(szTrcMsg, 4096, "%s%s", szTrcMsg, "\tRule Set ID:");
	snprintf(szTrcMsg, 4096, "%sPbit=%02d,Hbit=%02d\n", szTrcMsg, g_stRule[stBlock.usPackageID].pBit,
			g_stRule[stBlock.usPackageID].hBit);

	// Service ID
	snprintf(szTrcMsg, 4096, "%s%s", szTrcMsg, "\tRule Set Entry :");
	if( g_stEntry[stBlock.dServiceID].entName[0] != '\0' )
	{
		dAppLog(LOG_DEBUG, "Known Block ENTRY -> %d : %s", stBlock.dServiceID, 
				g_stEntry[stBlock.dServiceID].entName);
		snprintf(szTrcMsg, 4096, "%s%s(%d)\n", szTrcMsg, g_stEntry[stBlock.dServiceID].entName,
				stBlock.dServiceID);
	}
	else
	{
		if( conn != NULL )
		{
			// Entry Name
			sprintf(query, "SELECT DISTINCT value_key FROM INI_VALUES WHERE value_type = 1 "
					" AND value = %d", stBlock.dServiceID);

			if (mysql_query (conn, query) != 0)
			{
				dAppLog(LOG_WARN, "BLOCK RDR: Entry Query Fail.");
				ret = RetryDBConn();
				if( ret == 0 ) // reconnection success
				{
					if (mysql_query (conn, query) != 0)
						ret = 0; // success query 
					else
						ret = -1; // fail query
				}
				else
					ret = -1; // reconnection fail
			}
			else
				ret = 0; // success query

			if( ret == 0 ) // success query
			{
				result = mysql_store_result(&sql);
				if( result == NULL )
					dAppLog(LOG_WARN, "BLOCK RDR: mysql_store_result fail.");
				else
				{
					row = mysql_fetch_row(result);
					if(row == NULL )
						dAppLog(LOG_WARN, "BLOCK RDR: mysql_fetch_row fail.");
					else
					{
						snprintf(szTrcMsg, 4096, "%s%s(%d)\n", szTrcMsg, row[0], stBlock.dServiceID);
						sprintf(g_stEntry[stBlock.dServiceID].entName, "%s", row[0]);
					}
				}
				mysql_free_result(result);
			}
		}
	}


	// Block Reason
	snprintf(szTrcMsg, 4096, "%s%s", szTrcMsg, "\tBlock Reason:");
	snprintf(szTrcMsg, 4096, "%s%d(", szTrcMsg, stBlock.ucBlkReason);
	blkReason = (stBlock.ucBlkReason & 0x01000000) >> 6;

	/* TODO trace Block ���� ���� �׸� ����  */
	switch( blkReason )
	{
		case 0 : snprintf(szTrcMsg, 4096, "%s%s", szTrcMsg, "The action of the effective rule is block.)\n");
			break;
		case 1 : snprintf(szTrcMsg, 4096, "%s%s", szTrcMsg, "The concurrent session limit of the effective rule was reached)\n");
			break;
		default : snprintf(szTrcMsg, 4096, "%s%s", szTrcMsg, "not known reason)\n");
			 break;
	}

	snprintf(szTrcMsg, 4096, "%s------------------------------------------------------------\n", szTrcMsg);

	dMsgLen = strlen(szTrcMsg);
	szTrcMsg[dMsgLen] = 0;
	dAppLog(LOG_DEBUG, "Len:%d\tMsg:%s", dMsgLen, szTrcMsg);

	txIxpcMsg->head.bodyLen = dMsgLen;
	memcpy(txIxpcMsg->body, szTrcMsg, dMsgLen);

	dTxLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;

	if( msgsnd(dIxpcQid, (void*)&txGenQMsg, dTxLen, IPC_NOWAIT) < 0 )
	{
		dAppLog(LOG_CRI, "[FAIL] SEND TO TRACE MSGQ[IXPC] ERROR %d(%s)", errno, strerror(errno));
		return -1;
	}
	else
	{
		dAppLog(LOG_DEBUG, "[SUCC] SEND TO TRACE MSGQ");
	}
	return 0;
}

char *str_time(time_t t)
{
	static char mtime_str[81];

	strftime(mtime_str,80,"%Y-%m-%d %T",localtime(&t));
	return (char*)mtime_str;
}

//---------------------------------------------------------------------------
// MML Command �� ���� ���μ����� �Ϸ�� �� result message �� MMLResMsgType ��
// �޽��� �������� ����� MMCD�� ixpcQid�� �̿��� msgsnd �Ѵ�. (�޽���ť ���)
// @param rxIxpcMsg		: request ���� �޽��� (mmc command message)
// @param buff			: MML Command ��� �޽��� ���� ����
// @param resCode		: ��ɾ� ó�� ��� -> success(0), fail(-1)
// @param contFlag		: ������ �޽��� ���� ǥ�� -> last(0), not_last(1)
// @param extendTime	: not_last �� ��� ���� �޽������� timer ����ð�(��)
//			-> mmcd���� extendTime �ð���ŭ timer�� �����Ų��. 
// @param segFlag		: �� �޽����� �ʹ� ��� �ѹ��� ������ ���� �� ����
// @param seqNo			: sequence number (segment�� ��� �Ϸù�ȣ)
//
// @return 1	����
// @return -1	����
//---------------------------------------------------------------------------
int rdr_txMMLResult (IxpcQMsgType *rxIxpcMsg, char *buff, char resCode,
            char contFlag, unsigned short extendTime, char segFlag, char seqNo)
{
    int     txLen;
    GeneralQMsgType txGenQMsg;
    IxpcQMsgType    *txIxpcMsg;
    MMLResMsgType   *txMmlResMsg;
    MMLReqMsgType   *rxMmlReqMsg;

    txGenQMsg.mtype = MTYPE_MMC_RESPONSE;

    txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
    memset ((void*)&txIxpcMsg->head, 0, sizeof(txIxpcMsg->head));

    txMmlResMsg = (MMLResMsgType*)txIxpcMsg->body;
    rxMmlReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

    // ixpc routing header
    strcpy (txIxpcMsg->head.srcSysName, mySysName);
    strcpy (txIxpcMsg->head.srcAppName, myAppName);
//    strcpy (txIxpcMsg->head.dstSysName, rxIxpcMsg->head.srcSysName);
//    strcpy (txIxpcMsg->head.dstAppName, rxIxpcMsg->head.srcAppName);
    strcpy (txIxpcMsg->head.dstSysName, "DSCM");
    strcpy (txIxpcMsg->head.dstAppName, "MMCD");
    txIxpcMsg->head.segFlag = segFlag;
    txIxpcMsg->head.seqNo   = seqNo;

    // mml result header
    txMmlResMsg->head.mmcdJobNo  = rxMmlReqMsg->head.mmcdJobNo;
    txMmlResMsg->head.extendTime = extendTime;
    txMmlResMsg->head.resCode    = resCode;
    txMmlResMsg->head.contFlag   = contFlag;
    strcpy (txMmlResMsg->head.cmdName, rxMmlReqMsg->head.cmdName);

    // result message
    strcpy (txMmlResMsg->body, buff);

    txIxpcMsg->head.bodyLen = sizeof(txMmlResMsg->head) + strlen(buff) + 1;
    txLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;

    if (msgsnd(dIxpcQid, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0) {
        sprintf(trcBuf,"[rdr_txMMLResult] msgsnd fail to MMCD; err=%d(%s)\n%s",
                errno, strerror(errno), buff);
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    } else {
        if (trcFlag || trcLogFlag) {
            sprintf(trcBuf,"[rdr_txMMLResult] send to MMCD\n%s", buff);
            trclib_writeLog (FL,trcBuf);
        }
    }
    return 1;

} 

