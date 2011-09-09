#ifndef __DBLIB_H__
#define __DBLIB_H__

#include <mysql/mysql.h>	/*	MYSQL, MYSQL_ROW, MYSQL_RES structure, mysql_query, mysql_store_result, mysql_fetch_row, mysql_free_result	*/

/* MYSQL ERR CODE */
#define E_DB_TABLE_NOT_EXIST      1146
#define E_DB_NOT_CONNECT          2002
#define E_DB_DUPLICATED_ENTRY     1062

/* ERROR CODE */
#define E_DB_NULL_OBJECT  -1
#define E_DB_RECONNECT    -2
#define E_DB_CHARSET_NAME -3
#define E_DB_REAL_CONNECT -4
#define E_DB_PING         -5
#define E_DB_QUERY        -6
#define E_DB_RESULT       -7
#define E_DB_ROW_OVER     -8

/**
 *	PERIOD
 *	@brief	FSTAT, S_MNG 에서 쿼리를 수행할 DB 테이블을 결정할 때 사용하는 비교값
 *	@see	FSTAT/fstat_db.c
 */
enum {
    FIVE_MIN_PERIOD = 0,
    ONE_HOUR_PERIOD,
    ONE_DAY_PERIOD
};

#define DB_SUCCESS        0

#define MAX_SQL_QUERY_SIZE 20480

extern int  db_conn(MYSQL *pstMySQL, char *szIP, char *szName, char *szPass, char *szAlias);
extern void db_disconn(MYSQL *pstMySQL);
extern int  db_check_alive(MYSQL *pstMySQL);


/* func.c */
extern int  db_trunc_tbl(MYSQL *pstMySQL,char*);
extern void db_free(MYSQL_RES**);
extern int  db_select(MYSQL *pstMySQL, char*, MYSQL_RES**);
extern int  db_count(MYSQL *pstMySQL, char*);
extern int  db_insert(MYSQL *pstMySQL, char*);
extern int  db_delete(MYSQL *pstMySQL, char*);
extern int  db_update(MYSQL *pstMySQL, char*);
extern int  db_create(MYSQL *pstMySQL, char*);

/* func.c +error */
extern int   db_errno(MYSQL *pstMySQL);
extern char *db_error(MYSQL *pstMySQL);


#endif /* __DBLIB_H__ */
