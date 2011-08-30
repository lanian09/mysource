#include <stdio.h>

int main()
{
	int dRet;
	MYSQL_ROW row;
	MYSQL_RES rst;
	char *query = "Select TRID, USERNAME, PASSWD FROM SYS_USER_TEST";
	dRet = db_select(DBCONN, query, &rst);
	if( dRet < 0 ){
		//어쩌구
	}

	while( ( row = mysql_fetch_row(rst) ) != NULL ){
		//구조체에 담는다. atoll, atoi, atoll (row[0])부터...
	}

	mysql_free_result(&rst);
	return;
	
}
