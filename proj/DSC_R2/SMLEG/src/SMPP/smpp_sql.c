#include <stdio.h>
#include <string.h>

#include <time.h>
#include <mysql.h>
#include <smpp.h>

#define SMS_DB		"DSCM"				// Mysql Database 명 
#define HOST_IP		"211.254.95.231"	// Mysql 접속 IP
#define USER_NAME	"root"				// Mysql 접속 계정 명 
#define PASSWD		"mysql"				// Mysql 접속 패스워드 
//#define SMS_TBL		"sms_history"		// Mysql Table 명 

MYSQL	sql, *conn;		// Mysql 핸들 변수 
extern 	SMS_DB_INFO sms_db;
char    sysLabel[COMM_MAX_NAME_LEN], mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN];

unsigned short	delivAckSts = 3;

#define SELECT_SMS	"Select SUBS_ID, unix_timestamp(sysdate()) - unix_timestamp(MAX(DELIV_TIME)) Duration From sms_history Where SUBS_ID = '%s' Group by SUBS_ID;" 
//#define SELECT_SMS	"Select SUBS_ID, unix_timestamp(MAX(DELIV_TIME)) Duration From sms_history Where SUBS_ID = '%s' Group by SUBS_ID;" 
#define MAX_DURATION	10
#define DELIV			1
#define REPORT			2

void sample_data(SMS_HIS *his);

int keepaliveDB(void) 
{ 
    MYSQL_RES *result; 
    MYSQL_ROW row; 

    char query1[128] = {0,};
    int alive1 = 0;

    sprintf(query1, "select 1 from information_schema.TABLES where TABLE_SCHEMA='DSCM' and TABLE_NAME='rdr_lur_5minute_statistics'");

    mysql_query(conn, query1);
    result = mysql_store_result(&sql) ;
    row = mysql_fetch_row(result);
    alive1 = atoi(row[0]);
    mysql_free_result(result);

    if( alive1 != 1 )
    {
        printf("local mysql server gone away : %d\n",alive1);
		return -1;
    }
    mysql_free_result(result);

    return 1;
}

int smpp_connectDB(void)
{
	unsigned int mysql_timeout = 5;
	const int _RETRY = 5;
	int i;

	for( i = 0; i < _RETRY; i++ )
	{
		if( conn != NULL )
			mysql_close(conn);

		if (mysql_init (&sql) == NULL){
			dAppLog(LOG_DEBUG, "[%d] th mysql init failed", i);
		}
		mysql_options(&sql, MYSQL_OPT_CONNECT_TIMEOUT, &mysql_timeout);
		if ( (conn = mysql_real_connect (&sql, sms_db.ipaddr, sms_db.user, sms_db.passwd, sms_db.name, 0, 0, 0)) == NULL ) {
			dAppLog(LOG_DEBUG, "[%d] th mysql connect failed", i);
		}
		else
			return 0;
	}
	if( i == _RETRY )
	{
		dAppLog(LOG_DEBUG, "mysql connect all retry failed");
		return -1;
	}

	return 0;
}

int smpp_selectDB(char *subs_id)
{
	char query[1024] = {0,};

	snprintf(query, sizeof(query), SELECT_SMS, subs_id);

	if( conn != NULL ) {
		if ( mysql_query(conn, query) != 0 ) {
			dAppLog (LOG_DEBUG, "query fail: query=%s, err=%s", query, mysql_error(conn));

#if 0
			/* retry db connection */
			if ( (conn = mysql_real_connect (&sql, sms_db.ipaddr, sms_db.user, sms_db.passwd, sms_db.name, 0, 0, 0)) == NULL ) {
				dAppLog (LOG_DEBUG, "retry connection fail: conn=%d, err=%s", conn, mysql_error(conn));
				return -1;
			}
			/* retry db operation */
#endif
			/* retry db connection */
			if( smpp_connectDB() < 0 )
				return -1;
			else
				dAppLog (LOG_DEBUG, "retry connect DB success");

			if ( mysql_query(conn, query) != 0 ) {
				dAppLog (LOG_DEBUG, "retry select query fail: query=%s, err=%s", query, mysql_error(conn));
				return -1;
			}
		}
	}
	else {

		dAppLog (LOG_DEBUG, "Lost mysql Connection Handler");

		if( smpp_connectDB() < 0 )
			return -1;
		else
			dAppLog (LOG_DEBUG, "retry connect DB success");

		if ( mysql_query(conn, query) != 0 ) {
			dAppLog (LOG_DEBUG, "retry select query fail: query=%s, err=%s", query, mysql_error(conn));
			return -1;
		}
	}

	return 0;
}

int smpp_insertDB(SMS_HIS *his)
{
	char query[1024] = {0,};
	//char query[1024];
	// mySysName
	// smc_tbl->smc_info[0].ip_addr
	// his->info.pkgID
	// his->info.sPBit
	// his->info.sHBit
	// his->info.subsID
	//
	/* from_unixtime : for unsigned int -> yyyy-mm-dd hh:mm:ss convert */

	dAppLog(LOG_DEBUG, ">>> [DB INSERT] SMSC SERVER IP:%s",  smc_tbl->smc_info[0].ip_addr);

	if( his->delivSts == 0 )
	{
		snprintf(query, sizeof(query), \
			"Insert Into sms_history Values "
				"('%s', '%s', %d, '%02d %02d', '%s', %d, '%s', "
				" from_unixtime(%d), %d, '%s', %d, %d, %d, "
				" %d, %d, %d, %d, %d );", \
				mySysName, smc_tbl->smc_info[0].ip_addr, his->info.pkgID, his->info.sPBit, his->info.sHBit, his->info.subsID, \
				his->info.blkTm, his->info.smsMsg, \
				his->delivTm, his->delivSts, his->reportTm, his->reportSts, his->cid, 
				1,1,0,0,0,0);
	}
	else if( his->delivSts >= 1 && his->delivSts <= 12 ) // SMSC_ERR FAIL
	{
		snprintf(query, sizeof(query), \
			"Insert Into sms_history Values "
				"('%s', '%s', %d, '%02d %02d', '%s', %d, '%s', "
				" from_unixtime(%d), %d, '%s', %d, %d, %d, "
				" %d, %d, %d, %d, %d );", \
				mySysName, smc_tbl->smc_info[0].ip_addr, his->info.pkgID, his->info.sPBit, his->info.sHBit, his->info.subsID, \
				his->info.blkTm, his->info.smsMsg, \
				his->delivTm, his->delivSts, his->reportTm, his->reportSts, his->cid, 
				1,0,0,0,1,0);
	}
	else if( his->delivSts == SMPP_ERR_NORMAL )
	{
		snprintf(query, sizeof(query), \
			"Insert Into sms_history Values "
				"('%s', '%s', %d, '%02d %02d', '%s', %d, '%s', "
				" from_unixtime(%d), %d, '%s', %d, %d, %d, "
				" %d, %d, %d, %d, %d );", \
				mySysName, smc_tbl->smc_info[0].ip_addr, his->info.pkgID, his->info.sPBit, his->info.sHBit, his->info.subsID, \
				his->info.blkTm, his->info.smsMsg, \
				his->delivTm, his->delivSts, his->reportTm, his->reportSts, his->cid, 
				1,0,1,0,0,0);
	}
	else if( his->delivSts == SMPP_ERR_WRITE )
	{
		snprintf(query, sizeof(query), \
			"Insert Into sms_history Values "
				"('%s', '%s', %d, '%02d %02d', '%s', %d, '%s', "
				" from_unixtime(%d), %d, '%s', %d, %d, %d, "
				" %d, %d, %d, %d, %d );", \
				mySysName, smc_tbl->smc_info[0].ip_addr, his->info.pkgID, his->info.sPBit, his->info.sHBit, his->info.subsID, \
				his->info.blkTm, his->info.smsMsg, \
				his->delivTm, his->delivSts, his->reportTm, his->reportSts, his->cid, 
				1,0,0,1,0,0);
	}
	else
	{
		snprintf(query, sizeof(query), \
			"Insert Into sms_history Values "
				"('%s', '%s', %d, '%02d %02d', '%s', %d, '%s', "
				" from_unixtime(%d), %d, '%s', %d, %d, %d, "
				" %d, %d, %d, %d, %d );", \
				mySysName, smc_tbl->smc_info[0].ip_addr, his->info.pkgID, his->info.sPBit, his->info.sHBit, his->info.subsID, \
				his->info.blkTm, his->info.smsMsg, \
				his->delivTm, his->delivSts, his->reportTm, his->reportSts, his->cid, 
				1,0,0,0,0,1);
	}
	/*
	snprintf(query, sizeof(query), \
			"Insert Into sms_history Values "
				"('%s', '%s', %d, '%02d %02d', '%s', %d, '%s', "
				"from_unixtime(%d), %d, '%s', %d, %d);", \
				mySysName, smc_tbl->smc_info[0].ip_addr, his->info.pkgID, his->info.sPBit, his->info.sHBit, his->info.subsID, \
				his->info.blkTm, his->info.smsMsg, \
				his->delivTm, his->delivSts, his->reportTm, his->reportSts, his->cid);
	*/
	
	if( conn != NULL ) {

		if ( mysql_query(conn, query) != 0 ) {
			dAppLog (LOG_DEBUG, "query fail: query=%s, err=%s", query, mysql_error(conn));

			/* retry db connection */
			if( smpp_connectDB() < 0 )
				return -1;
			else
				dAppLog (LOG_DEBUG, "retry connect DB success");

#if 0
			/* retry db connection */
			if ( (conn = mysql_real_connect (&sql, sms_db.ipaddr, sms_db.user, sms_db.passwd, sms_db.name, 0, 0, 0)) == NULL ) {
				dAppLog (LOG_DEBUG, "retry connection fail: conn=%d, err=%s", conn, mysql_error(conn));
				return -1;
			}
#endif
			/* retry db operation */
			if ( mysql_query(conn, query) != 0 ) {
				dAppLog (LOG_DEBUG, "retry insert query fail: query=%s, err=%s", query, mysql_error(conn));
				return -1;
			}
		}
	}
	else {

		dAppLog (LOG_DEBUG, "Lost mysql Connection Handler");

		/* retry db operation */
		if( smpp_connectDB() < 0 )
			return -1;
		else
			dAppLog (LOG_DEBUG, "retry connect DB success");

		if ( mysql_query(conn, query) != 0 ) {
			dAppLog (LOG_DEBUG, "retry insert query fail: query=%s, err=%s", query, mysql_error(conn));
			return -1;
		}
	}
	return 0;
}

int smpp_insertDB2(SMS_HIS *his)
{
	char query[1024] = {0,};
	//char query[1024];
	// mySysName
	// smc_tbl->smc_info[0].ip_addr
	// his->info.pkgID
	// his->info.sPBit
	// his->info.sHBit
	// his->info.subsID
	//
	/* from_unixtime : for unsigned int -> yyyy-mm-dd hh:mm:ss convert */
	his->delivSts = SMPP_ERR_WRITE;
	snprintf(query, sizeof(query), \
			"Insert Into sms_history Values "
				"('%s', '%s', %d, '%02d %02d', '%s', %d, '%s', "
				"from_unixtime(%d), %d, '%s', %d, %d);", \
				mySysName, smc_tbl->smc_info[0].ip_addr, his->info.pkgID, his->info.sPBit, his->info.sHBit, his->info.subsID, \
				his->info.blkTm, his->info.smsMsg, \
				his->delivTm, his->delivSts, his->reportTm, his->reportSts, his->cid);

	if( conn != NULL ) {
		if ( mysql_query(conn, query) != 0 ) {
			dAppLog (LOG_DEBUG, "query fail: query=%s, err=%s", query, mysql_error(conn));

			if( smpp_connectDB() < 0 )
				return -1;
			else
				dAppLog (LOG_DEBUG, "retry connect DB success");
#if 0
			/* retry db connection */
			if ( (conn = mysql_real_connect (&sql, sms_db.ipaddr, sms_db.user, sms_db.passwd, sms_db.name, 0, 0, 0)) == NULL ) {
				dAppLog (LOG_DEBUG, "retry connection fail: conn=%d, err=%s", conn, mysql_error(conn));
				return -1;
			}
			/* retry db operation */
#endif
			if ( mysql_query(conn, query) != 0 ) {
				dAppLog (LOG_DEBUG, "retry insert query fail: query=%s, err=%s", query, mysql_error(conn));
				return -1;
			}
		}
	}
	else {

		dAppLog (LOG_DEBUG, "Lost mysql Connection Handler");

#if 0
		/* retry db connection */
		if ( (conn = mysql_real_connect (&sql, sms_db.ipaddr, sms_db.user, sms_db.passwd, sms_db.name, 0, 0, 0)) == NULL ) {
			dAppLog (LOG_DEBUG, "retry connection fail: conn=%d, err=%s", conn, mysql_error(conn));
			return -1;
		}
		/* retry db operation */
#endif
		/* retry db connection */
		if( smpp_connectDB() < 0 )
			return -1;
		else
			dAppLog (LOG_DEBUG, "retry connect DB success");

		if ( mysql_query(conn, query) != 0 ) {
			dAppLog (LOG_DEBUG, "retry insert query fail: query=%s, err=%s", query, mysql_error(conn));
			return -1;
		}

	}
	return 0;
}

int smpp_updateDB(SMS_HIS *his, int update_type)
{
	char query[1024] = {0,};

	if (update_type == SMPP_DELIVER_ACK_MSG)
	{
		if( his->delivSts == 0 ) return 0;
		//if( his->delivSts != 0 )
		//{
			snprintf(query, sizeof(query), \
					"Update sms_history Set DELIV_STS = %d, SUCC = 0, SMPP_ERR = 0, SVR_ERR = 0, "
					"SMSC_ERR = 1, ETC_ERR = 0 " 
					"Where SUBS_ID = '%s' And SERIAL_NUM = %d And REPORT_TIME = '';", \
					his->delivSts, his->info.subsID, his->cid);
		//}
	}
	else if (update_type == SMPP_REPORT_MSG)
	{
		snprintf(query, sizeof(query), \
			"Update sms_history Set REPORT_TIME = '%s', REPORT_STS = %d "
			"Where SUBS_ID = '%s' And SERIAL_NUM  = %d And REPORT_TIME = '';", \
			his->reportTm, his->reportSts, his->info.subsID, his->cid);
	}

	dAppLog(LOG_DEBUG, "query info: query=%s, err=%s", query, mysql_error(conn));

	if( conn != NULL ) {
		if ( mysql_query(conn, query) != 0 ) {
			dAppLog(LOG_DEBUG, "query fail: query=%s, err=%s", query, mysql_error(conn));

#if 0
			/* retry db connection */
			if ( (conn = mysql_real_connect (&sql, sms_db.ipaddr, sms_db.user, sms_db.passwd, sms_db.name, 0, 0, 0)) == NULL ) {
				dAppLog (LOG_DEBUG, "retry connection fail: conn=%d, err=%s", conn, mysql_error(conn));
				return -1;
			}
			/* retry db operation */
#endif
			if( smpp_connectDB() < 0 )
				return -1;
			else
				dAppLog (LOG_DEBUG, "retry connect DB success");

			if ( mysql_query(conn, query) != 0 ) {
				dAppLog (LOG_DEBUG, "retry update query fail: query=%s, err=%s", query, mysql_error(conn));
				return -1;
			}
		}
	}
	else {

		dAppLog (LOG_DEBUG, "Lost mysql Connection Handler");

		/* retry db connection */
		if( smpp_connectDB() < 0 )
			return -1;
		else
			dAppLog (LOG_DEBUG, "retry connect DB success");

		if ( mysql_query(conn, query) != 0 ) {
			dAppLog (LOG_DEBUG, "retry insert query fail: query=%s, err=%s", query, mysql_error(conn));
			return -1;
		}

	}
	return 0;
}

				
void sample_data(SMS_HIS *his)
{
	his->info.sendFlag = 0;
	his->info.pkgID = 100;
	his->info.blkTm = time(0);
	strcpy(his->info.subsID, "01012345678");
	his->info.subsID[strlen(his->info.subsID)] = 0;
	strcpy(his->info.smsMsg, "nice guy. you are a bad boy!!!");
	his->info.smsMsg[strlen(his->info.smsMsg)] = 0;

	his->delivTm = his->info.blkTm + 15;
	his->delivSts = 0;
	his->reportSts = 0;
	memset(his->reportTm, 0x00, sizeof(char)*SMPP_MSG_DELIVER_TIME_LEN);
}

#if 0 /* by june */
int main (void)
{
	int ret = 0;

	SMS_HIS		sms_his, sms_his2;

	MYSQL_RES	*result;
	MYSQL_ROW	row;

	unsigned int	duration = 0;
	char subscriber[SMS_MAX_SUBSID_LEN];

	memset(&sms_his, 0x00, sizeof(SMS_HIS));
	sample_data(&sms_his);

	ret = smpp_connectDB();
	if ( ret < 0 ) {
		printf("BAD GUY!! mysql connection fail..err=%s\n", mysql_error(&sql));
		return -1;
	}
	else
		printf("NICE GUY!! mysql connection success..\n");

	// Got Block RDR From RDRANA : Subscribers ID = '01012345678' and Special Pkg ID = 100

	// Select SUBS_ID, MAX(DELIV_TIME) From sms_history Where SUBS_ID = '01012345678';
	ret = smpp_selectDB(sms_his.info.subsID);
	if ( ret < 0 )
	{
		// select query 가 실패. 가입자가 table에 있는지 없는지 모르는 상태 
		printf("BAD GUY!! select query fail..\n");
	}
	else
	{
		printf("NICE GUY!! select success...\n");
	}

	result = mysql_store_result(conn);
	row = mysql_fetch_row(result);
	if ( row == NULL ) // select 결과 가입자가 없음. 
	{
		printf("NICE GUY!! Not Exist Subscriber[%s] In Table. This is first Subs Info.\n", sms_his.info.subsID);
		// DELIVER() To SMS Server
		ret = Insert_DB(sms_his);
		if( ret < 0 )
		{
			printf("BAD GUY!! Insert query fail..\n");
		}
		else
			printf("NICE GUY!! Fisrt Insert OK...\n");
	}
	else // select 결과 가입자가 있음.
	{
		// row[0] : Select 문의 첫번째 칼럼 -> SUBS_ID 값을 담고 있다. 
		// row[1] : Select 문의 두번째 칼럼 -> 현재 시간 - DELIV_TIME 의 시간 차이를 갖고 있다. 
		strncpy(subscriber, row[0], strlen(row[0]));
		duration = (unsigned int)row[1];

		if ( duration > MAX_DURATION ) // duration이 지났다. 
		{
			// DELIVER() To SMS Server 
			memset(&sms_his2, 0x00, sizeof(SMS_HIS));
			sample_data(&sms_his2);
			ret = smpp_insertDB(sms_his2);
			if( ret < 0 )
			{
				printf("BAD GUY!!. Insert query fail..\n");
			}
			else
				printf("NICE GUY!!. Duration Over... Second Insert OK...\n");
		}
		else
		{
			printf("NICE GUY!! Duration Not Yet passed...Nothing To Do.\n");
		}
	}

	// Delive Ack Received..
	sms_his.delivSts = delivAckSts;
	ret = smpp_updateDB(sms_his, DELIV);
	if ( ret < 0 )
	{
		printf("BAD GUY!! Update deliv Ack fail....\n");
	}
	else
	{
		printf("NICE GUY!! Update deliv Ack succss...\n");
	}

	// Report Received.
	sms_his.reportTm = time(0);
	sms_his.reportSts = 4;
	ret = smpp_updateDB(sms_his, REPORT);
	if ( ret < 0 )
	{
		printf("BAD GUY!! Update Report fail....\n");
	}
	else
	{
		printf("NICE GUY!! Update Report succss...\n");
	}

	// Report Ack Send....
	printf("NICE GUY!! Bye Bye~~~\n");

	return 0;
}
#endif


