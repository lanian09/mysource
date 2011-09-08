#include <stdio.h>
#include <string.h>

#include <time.h>
#include <mysql.h>
#include "capd_global.h"
#include "comm_smpp_if.h"
#include "utillib.h"
#include "rdrheader.h"

MYSQL	sql, *conn;		// Mysql 핸들 변수 
extern 	SMS_DB_INFO sms_db;
char    sysLabel[COMM_MAX_NAME_LEN], mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN];
extern 	SMS_INFO stSms;

#define SELECT_SMS	"Select msg From sms_msg"

int keepaliveDB(void) 
{ 
    MYSQL_RES *result; 
    MYSQL_ROW row; 

    char query1[128] = {0,};
    int alive1 = 0;

    sprintf(query1, "select 1 from information_schema.TABLES where TABLE_SCHEMA='mysql' and TABLE_NAME='db'");

    mysql_query(conn, query1);
    result = mysql_store_result(&sql) ;
    row = mysql_fetch_row(result);
    alive1 = atoi(row[0]);
    mysql_free_result(result);

    if( alive1 != 1 )
    {
        printf("remote mysql server gone away : %d\n",alive1);
		return -1;
    }
    mysql_free_result(result);

    return 1;
}

int connectDB(void)
{
	if (mysql_init (&sql) == NULL){
		return -1;
	}
	if ( (conn = mysql_real_connect (&sql, sms_db.ipaddr, sms_db.user, sms_db.passwd, sms_db.name, 0, 0, 0)) == NULL )
		return -1;

	return 0;
}

int sms_selectDB()
{
	MYSQL_RES   *result;
	MYSQL_ROW   row;
	char query[1024] = {0,};

	snprintf(query, sizeof(query), SELECT_SMS);

	if ( mysql_query(conn, query) != 0 ) {
		dAppLog (LOG_DEBUG, "query fail: query=%s, err=%s", query, mysql_error(conn));
		
		/* retry db connection */
		if ( (conn = mysql_real_connect (&sql, sms_db.ipaddr, sms_db.user, sms_db.passwd, sms_db.name, 0, 0, 0)) == NULL ) {
			return -1;
		}
		/* retry db operation */
		if ( mysql_query(conn, query) != 0 ) {
			return -1;
		}
	}

	result = mysql_store_result(conn);
	row = mysql_fetch_row(result);
	if( row == NULL )
	{
		return -1;
	}
	else
	{
		memset(stSms.smsMsg, 0x00, sizeof(stSms.smsMsg));
		sprintf(stSms.smsMsg, "%s", row[0]);
		stSms.smsMsg[strlen(stSms.smsMsg)] = 0;
		dAppLog( LOG_INFO, "SMS MSG change. -> %s", stSms.smsMsg);
	}

	return 0;
}
