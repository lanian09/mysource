/**A.1* FILE INCLUSION ********************************************************/
#include <stdio.h>          /*  fopen(3), fgets(3)  */
#include <stdlib.h>			/*	atoll(3), atoi(3)	*/
#include <errno.h>          /*  errno(3)    */
#include <string.h>         /*  strncpy(3), strlen(3), strerror(3), strcmp(3)   */

#include "dblib.h"

/**B.1*	DEFINITION OF NEW CONSTANTS *********************/
/**B.2*	DEFINITION OF NEW TYPE  **************************/
/**C.1*	DECLARATION OF VARIABLES  ************************/
/**D.1*	DEFINITION OF FUNCTIONS  *************************/
int db_conn(MYSQL *pstMySQL, char *szIP, char *szName, char *szPass, char *szAlias)
{
#ifdef MYSQL_RECONNECT
	my_bool		cMyReconn;
#endif	// END: #ifdef MYSQL_RECONNECT
#ifdef MYSQLCHARSET_EUCKR
	char		sMyLocale[BUFSIZ];
#endif	// END: #ifdef MYSQLCHARSET_EUCKR

	mysql_init(pstMySQL);

#ifdef MYSQL_RECONNECT
	cMyReconn = 1;
	if( (mysql_options(pstMySQL, MYSQL_OPT_RECONNECT, &cMyReconn)) != 0) {

		return E_DB_RECONNECT;
	}
#endif	// END: #ifdef MYSQL_RECONNECT

#ifdef MYSQLCHARSET_EUCKR
	strncpy(sMyLocale, "euckr", BUFSIZ);
	if( (mysql_options(pstMySQL, MYSQL_SET_CHARSET_NAME, sMyLocale)) != 0) {
		return E_DB_CHARSET_NAME;
	}
#endif	// END: #ifdef MYSQLCHARSET_EUCKR

	if( (mysql_real_connect(pstMySQL, szIP, szName, szPass, szAlias, 0, NULL, 0)) == NULL) {
		return E_DB_REAL_CONNECT;
	}
return 0;
}

void db_disconn(MYSQL *pstMySQL)
{
    mysql_close(pstMySQL);
}

int db_check_alive(MYSQL *pstMySQL)
{
	if(pstMySQL == NULL) {
		//dAppLog(LOG_CRI, "F=%s:%s.%d: pstMySQL is NULL", __FILE__, __FUNCTION__, __LINE__);
		return E_DB_NULL_OBJECT;
	}

	if( (mysql_ping(pstMySQL)) != 0) {
		//dAppLog(LOG_CRI, "F=%s:%s.%d: [%d] [%s]", __FILE__, __FUNCTION__, __LINE__, mysql_errno(pstMySQL), mysql_error(pstMySQL));
		return E_DB_PING;
	}

	return DB_SUCCESS;
}


