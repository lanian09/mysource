#include <stdio.h>
#include <unistd.h>
#include <mysql/mysql.h>

MYSQL *conn, sql;

int db_init()
{
	unsigned int timeout = 3;

	mysql_init( &sql );

	mysql_options( &sql, MYSQL_OPT_CONNECT_TIMEOUT,  (char*)&timeout );

	conn = mysql_real_connect( &sql, "localhost", "root", "mysql", "DSCM", 0,0,0 );
	if( conn == NULL ){
		printf("DB connection failed\n");
		return -1;
	}

	return 0;
}

int db_close()
{
	mysql_close(conn);
	return 0;
}

int db_run()
{
	char 	query[4096] = {0,};
	time_t	curr;

	sprintf(query, "delete from rdr_ruleent_5minute_statistics where stat_date < '2011-08-25'");
	time(&curr);

	if( mysql_query(conn, query) != 0 ){
		printf("error query running. time=%d\n", curr);
		return -1;
	}
	return 0;
	
}

int main()
{

	int err_cnt = 0;

	if( db_init() < 0 ){
		return -1;
	}

	while(err_cnt < 3){
		if( db_run() < 0 ){
			err_cnt++;
			printf("error occured count = %d\n", err_cnt);
		}
		usleep(1);
	}
	printf("end of process\n");

	db_close();
	return 0;
}
