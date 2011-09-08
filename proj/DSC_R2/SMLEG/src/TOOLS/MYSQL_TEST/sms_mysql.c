#include <stdio.h>
#include <string.h>

#include <time.h>
#include <mysql.h>

#define SMS_DB		"mysql"				// Mysql Database 명 
#define SMS_TBL		"sms_history"		// Mysql Table 명 
#define HOST_IP		"211.254.95.210"	// Mysql 접속 IP
#define USER_NAME	"root"				// Mysql 접속 계정 명 
#define PASSWD		"mysql"				// Mysql 접속 패스워드 

MYSQL	sql, *conn;		// Mysql 핸들 변수 

// RDRANA -> SMPP 간 SMS INFORMATION STRUCTURE (MSG_Q BODY)
typedef struct __st_sms_info__ {
#define SMS_MAX_SUBSID_LEN          64
#define SMS_MAX_MSG_LEN             160
    unsigned short  sendFlag;
    unsigned short  pkgID;
    unsigned int    blkTm;
    unsigned char   subsID[SMS_MAX_SUBSID_LEN];
    unsigned char   smsMsg[SMS_MAX_MSG_LEN];
} SMS_INFO;

////////////////////////////////////////////////////////////////////
// SMS HISTORY MANAGE STRUCTURE (DB TABLE SCHEMA)
typedef struct __st_sms_his__ {
    unsigned int    delivTm;
    unsigned short  delivSts;
    unsigned int    reportTm;
    unsigned short  reportSts;
	unsigned int	cid;
    SMS_INFO        info;
} SMS_HIS;

unsigned short	delivAckSts = 3;

#define SELECT_SMS	"Select SUBS_ID, unix_timestamp(sysdate()) - MAX(DELIV_TIME) Duration From sms_history Where SUBS_ID = '%s' Group by SUBS_ID;" 
#define MAX_DURATION	10
#define DELIV			1
#define REPORT			2

void sample_data(SMS_HIS *his);

int main(void)
{
	int ret = 0;

	SMS_HIS		sms_his, sms_his2;

	MYSQL_RES	*result;
	MYSQL_ROW	row;

	unsigned int	duration = 0;
	char subscriber[SMS_MAX_SUBSID_LEN];

	memset(&sms_his, 0x00, sizeof(SMS_HIS));
	sample_data(&sms_his);

	ret = Connect_DB();
	if ( ret < 0 )
	{
		printf("BAD GUY!! mysql connection fail..err=%s\n", mysql_error(&sql));
		return -1;
	}
	else
		printf("NICE GUY!! mysql connection success..\n");

	// Got Block RDR From RDRANA : Subscribers ID = '01012345678' and Special Pkg ID = 100

	// Select SUBS_ID, MAX(DELIV_TIME) From sms_history Where SUBS_ID = '01012345678';
	ret = Select_DB(sms_his.info.subsID);
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
			sms_his2.cid = 1;
			sms_his2.delivSts = 5;
			ret = Insert_DB(sms_his2);
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
	ret = Update_DB(sms_his, DELIV);
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
	ret = Update_DB(sms_his, REPORT);
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
#if 0
int keepaliveDB(void) 
{ 
    MYSQL_RES *result; 
    MYSQL_ROW row; 

    char query1[128] = {0,};
    int alive1 = 0;

    sprintf(query1, "select 1 from information_schema.TABLES where TABLE_SCHEMA='DSCM' and TABLE_NAME='rdr_lur_5minute_statistics'")

    mysql_query(conn, query1);
    result = mysql_store_result(&sql) ;
    row = mysql_fetch_row(result);
    alive1 = atoi(row[0]);
    mysql_free_result(result);

    if( alive1 != 1 )
    {
        sprintf(trcBuf, "local mysql server gone away : %d\n",alive1);
        trclib_writeErr(FL, trcBuf);
		return -1;
    }
    mysql_free_result(result);

    return 1;
}
#endif

int Connect_DB()
{
	mysql_init (&sql);
	if ( (conn = mysql_real_connect (&sql, HOST_IP, USER_NAME, PASSWD, SMS_DB, 0, 0, 0)) == NULL )
		return -1;

	return 0;
}

int Select_DB(char *subs_id)
{
	char query[1024] = {0,};

	snprintf(query, sizeof(query), SELECT_SMS, subs_id);

	if ( mysql_query(conn, query) != 0 )
	{
		printf("query fail: query=%s, err=%s\n", query, mysql_error(conn));
		return -1;
	}

	return 0;
}

int Insert_DB(SMS_HIS his)
{
	char query[1024] = {0,};

	snprintf(query, sizeof(query), 
			"Insert Into sms_history Values "
				"(%d, %d, '%s', %d, '%s', "
				"from_unixtime(%d), %d, %d, %d, %d);",
				his.info.sendFlag, his.info.pkgID, his.info.subsID, his.info.blkTm, his.info.smsMsg, \
				his.delivTm, his.delivSts, his.reportTm, his.reportSts, his.cid);
	
	if ( mysql_query(conn, query) != 0 )
	{
		printf("query fail: query=%s, err=%s\n", query, mysql_error(conn));
		return -1;
	}
	return 0;
}


int Update_DB(SMS_HIS his, int update_type)
{
	char query[1024] = {0,};

	if ( update_type == DELIV )
	{
		snprintf(query, sizeof(query), \
			"Update sms_history Set DELIV_STS = %d "
			"Where SUBS_ID = %s And SERIAL_NUM = %d;", \
			his.delivSts, his.info.subsID, his.cid);
	}
	else if ( update_type == REPORT )
	{
		snprintf(query, sizeof(query), \
			"Update sms_history Set REPORT_TIME = %d, REPORT_STS = %d "
			"Where SUBS_ID = %s And SERIAL_NUM = %d;", \
			his.reportTm, his.reportSts, his.info.subsID, his.cid);
	}

	if ( mysql_query(conn, query) != 0 )
	{
		printf("query fail: query=%s, err=%s\n", query, mysql_error(conn));
		return -1;
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
	his->reportTm = 0;
	his->reportSts = 0;
	his->cid = 0;
}

