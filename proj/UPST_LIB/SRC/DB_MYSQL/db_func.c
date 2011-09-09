/**A.1* FILE INCLUSION ********************************************************/
#include <stdio.h>          /*  fopen(3), fgets(3)  */
#include <stdlib.h>			/*	atoll(3), atoi(3)	*/
#include <errno.h>          /*  errno(3)    */
#include <string.h>         /*  strncpy(3), strlen(3), strerror(3), strcmp(3)   */

#include "dblib.h"

/**B.1*	DEFINITION OF NEW CONSTANTS *********************/
/**B.2*	DEFINITION OF NEW TYPE  **************************/
/**C.1*	DECLARATION OF VARIABLES  ************************/
/**D.1*  Definition of Functions  *************************/

int db_errno(MYSQL *pstMySQL)
{
	return mysql_errno(pstMySQL);
}

char* db_error(MYSQL *pstMySQL)
{
	return (char*)mysql_error(pstMySQL);
}

int db_trunc_tbl(MYSQL *pstMySQL, char *sTblName)
{
	char		query[MAX_SQL_QUERY_SIZE];

	sprintf(query, "TRUNCATE TABLE %s", sTblName);

	if( mysql_query( pstMySQL, query ) != 0 ){
		return E_DB_QUERY;
	}

	return 0;
}

void db_free(MYSQL_RES **pstRst)
{
	mysql_free_result(*pstRst);
}

int db_select(MYSQL *pstMySQL, char *query, MYSQL_RES **pstRst)
{
	if( mysql_query( pstMySQL, query ) != 0 ){
		return E_DB_QUERY;
	}

	if( (*pstRst = mysql_store_result(pstMySQL)) == NULL ){
		return E_DB_RESULT;
	}
	
	return mysql_num_rows(*pstRst);
}

int db_count(MYSQL *pstMySQL, char *query)
{
	int        dRet;
	MYSQL_ROW  row;
	MYSQL_RES *pstRst;
	
	if( (dRet = db_select(pstMySQL,query,&pstRst)) <= 0 ){
		return dRet;
	}

	if( dRet > 1 ){
		return E_DB_ROW_OVER;
	}

	while( (row = mysql_fetch_row(pstRst)) != NULL ){
		dRet = (int) atoi(row[0]);
	}

	mysql_free_result(pstRst);
	return dRet;
}

int db_insert(MYSQL *pstMySQL, char *query)
{
	if( mysql_query( pstMySQL, query ) != 0 ){
		return E_DB_QUERY;
	}
	return DB_SUCCESS;
}

int db_delete(MYSQL *pstMySQL, char *query)
{
	if( mysql_query( pstMySQL, query ) != 0 ){
		return E_DB_QUERY;
	}
	return DB_SUCCESS;
}

int db_update(MYSQL *pstMySQL, char *query)
{
	if( mysql_query( pstMySQL, query ) != 0 ){
		return E_DB_QUERY;
	}
	return DB_SUCCESS;
}

int db_create(MYSQL *pstMySQL, char *query)
{
	if( mysql_query( pstMySQL, query ) != 0 ){
		return E_DB_QUERY;
	}
	return DB_SUCCESS;
}
