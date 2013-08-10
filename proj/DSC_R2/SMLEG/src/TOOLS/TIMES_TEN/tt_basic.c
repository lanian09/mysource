/*******************************************/
/* Title : Timesten C  program...          */
/*         TT library                      */
/* Date  : 2007.08.06  juno                */
/*******************************************/
#ifdef WIN32
#include <windows.h>
#else
#include <sqlunix.h>
#endif
#include <sql.h>
#include <sqlext.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include "tt_basic.h"

SQLHENV     hEnv  = SQL_NULL_HENV;

/*-----------------------------------------------
    Error Message
-----------------------------------------------*/
void CheckReturnCode (SQLRETURN rc, char* msg, SQLHDBC hDbc, SQLHSTMT hStmt, char *filename, int lineno) 
{
	#define MSG_LNG 512 
	
	char		str[1000];
	SQLCHAR szSqlState[MSG_LNG];                 /* SQL state string */
	SQLINTEGER pfNativeError;                    /* Native error code */
	SQLCHAR szErrorMsg[MSG_LNG];                 /* Error msg text buffer pointer */
	SQLSMALLINT pcbErrorMsg;                     /* Error msg text Available bytes */
	SQLRETURN ret = SQL_SUCCESS;                 
	if (rc != SQL_SUCCESS && rc != SQL_NO_DATA_FOUND ) {
		/* Now see why the error/warning occurred */
		while (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
			ret = SQLError(hEnv, hDbc, hStmt, szSqlState, &pfNativeError, szErrorMsg, MSG_LNG, &pcbErrorMsg);
			switch (ret) { 
				case SQL_SUCCESS:
					if ( pfNativeError == 2207 )
					{
						/* Error (Create table) */
						return;
					}
					else
						fprintf(stderr, "*** TimesTen Error/Warning = %ld\n*** %s\n *** ODBC Error/Warning = %s\n", pfNativeError, szErrorMsg, szSqlState);
					break;
				case SQL_SUCCESS_WITH_INFO:
					fprintf(stderr, "*** Call to SQLError failed with return code of SQL_SUCCESS_WITH_INFO.\n *** Need to increase size of message buffer.\n");
					break;
				case SQL_INVALID_HANDLE:
					fprintf(stderr, "*** Call to SQLError failed with return code of SQL_INVALID_HANDLE.\n");
					break;
				case SQL_ERROR:
					fprintf(stderr, "*** Call to SQLError failed with return code of SQL_ERROR.\n");
					break;
				case SQL_NO_DATA_FOUND:
					break;
			} /* switch */
		} /* while */
		if (rc != SQL_SUCCESS_WITH_INFO) { 
			/* It's not just a warning */
			fprintf(stderr, "*** ERROR in %s, line %d:  %s\n", filename, lineno, msg);
		}
	}
}

/*-----------------------------------------------
    timesten connetion
-----------------------------------------------*/
int TT_Connect( SQLHDBC *hDbc, SQLCHAR* g_connStr, int autocommit_opt )
{
	char		str[1000];
	SQLRETURN	rc = SQL_SUCCESS;                          			  /* General return code for the API */
	SQLCHAR		szConnOut[255];                                       /* Buffer for completed connection string */
	SQLSMALLINT	cbConnOut;                                            /* number of bytes returned in szConnOut */
	
	rc = SQLAllocEnv(&hEnv);
	if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
	{
		memset ( str , '\0', 1000);
		strcpy ( str , "Unable to allocate an evironment handle\n");
		CheckReturnCode(rc, str, NULL, NULL, __FILE__, __LINE__);
		LOG_Write(str, FAILURE);
		return FAILURE;
	}
	
#ifndef TPSONLY
	fprintf(stdout, (char *)"Allocate an environment handel complete....\n");
	fflush(stdout);
#endif
	
	rc = SQLAllocConnect(hEnv, hDbc);
	if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
	{
		memset ( str , '\0', 1000);
		strcpy ( str , "Unable to allocate an connection handle\n");
		CheckReturnCode(rc, str, *hDbc, NULL, __FILE__, __LINE__);
		LOG_Write(str, FAILURE);
		return FAILURE;
	}

#ifndef TPSONLY
	fprintf(stdout, (char *)"Allocate a connection handle complete....\n");
	fflush(stdout);
#endif
	
	rc = TT_Autocommit( *hDbc, autocommit_opt );						/* commit control... */
	if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
	{
		return FAILURE;
	}

	rc = SQLDriverConnect(*hDbc, NULL, g_connStr, SQL_NTS, szConnOut, 255, &cbConnOut, SQL_DRIVER_NOPROMPT);
	if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
	{
		memset ( str , '\0', 1000);
		strcpy ( str , "Error in connecting to the driver\n");
		CheckReturnCode(rc, str, *hDbc, NULL, __FILE__, __LINE__);
		LOG_Write(str, FAILURE);
		return FAILURE;
	}

#ifndef TPSONLY
	fprintf(stdout, (char *)"Allocate a connect to driver complete....\n");
	fflush(stdout);
	/*fprintf(stdout, (char *)"Connection Successful....\n"); */
	/*fflush(stdout); */
#endif

	return SUCCESS;
}

/*-----------------------------------------------
    timesten allocstmt
-----------------------------------------------*/
int TT_AllocStmt( SQLHDBC hDbc, SQLHSTMT *hStmt )
{
	char		str[1000];
	SQLRETURN	rc = SQL_SUCCESS;                          			  /* General return code for the API */

	rc = SQLAllocStmt(hDbc, hStmt);
	if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
	{
		memset ( str , '\0', 1000);
		strcpy ( str , "Unable to allocate a statement handle\n");
		CheckReturnCode(rc, str, hDbc, hStmt, __FILE__, __LINE__);
		LOG_Write(str, FAILURE);
		return FAILURE;
	}

#ifndef TPSONLY
	fprintf(stdout, (char *)"Allocate a statement handle complete....\n");
	fflush(stdout);
#endif
	
	return SUCCESS;
}

/*-----------------------------------------------
	timesten FreeStmt
-----------------------------------------------*/
int TT_FreeStmt( SQLHSTMT hStmt, SQLUSMALLINT OPTION )
{
	char		str[1000];
	SQLRETURN	rc = SQL_SUCCESS;                          /* General return code for the API */

	rc = SQLFreeStmt(hStmt, OPTION);
	if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
	{
		memset ( str , '\0', 1000);
		sprintf ( str , "Unable to free the statement handle(%d)\n", rc);
		CheckReturnCode(rc, str, NULL, hStmt, __FILE__, __LINE__);
		LOG_Write(str, FAILURE);
		return FAILURE;
	}
	
#ifndef TPSONLY
	if ( OPTION == SQL_DROP )
	{
		fprintf(stdout, (char *)"Free statement complete...\n");
		fflush(stdout);
	}
#endif
	return SUCCESS;
}

/*-----------------------------------------------
	timesten disconnetion
-----------------------------------------------*/
int TT_DisConnect(SQLHDBC hDbc )
{
	char		str[1000];
	SQLRETURN   rc = SQL_SUCCESS;                          /* General return code for the API */

	if (hDbc != SQL_NULL_HDBC)
	{
		rc = SQLDisconnect(hDbc);
		if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
		{
			memset ( str , '\0', 1000);
			strcpy ( str , "Error during disconnect\n");
			CheckReturnCode(rc, str, hDbc, NULL, __FILE__, __LINE__);
			LOG_Write(str, FAILURE);
			return FAILURE;
		}        
#ifndef TPSONLY
		fprintf(stdout, (char *)"Disconnect from TimesTen complete...\n");
		fflush(stdout);
#endif
		
		rc = SQLFreeConnect(hDbc);
		if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
		{
			memset ( str , '\0', 1000);
			strcpy ( str , "Error during free connect handler...\n");
			CheckReturnCode(rc, str, hDbc, NULL, __FILE__, __LINE__);
			LOG_Write(str, FAILURE);
			return FAILURE;
		}        
#ifndef TPSONLY
		fprintf(stdout, (char *)"Free the connection handle completed...\n");
		fflush(stdout);
#endif
	}
	
	if (hEnv != SQL_NULL_HENV)
	{
		rc = SQLFreeEnv(hEnv);
		if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
		{
			memset ( str , '\0', 1000);
			strcpy ( str , "Error during free environment handler...\n");
			CheckReturnCode(rc, str, NULL, NULL, __FILE__, __LINE__);
			LOG_Write(str, FAILURE);
			return FAILURE;
		}        
#ifndef TPSONLY
		fprintf(stdout, (char *)"Free the environment handle complete...\n");
		fflush(stdout);
#endif
	}

	memset ( str , '\0', 1000);
	strcpy ( str , "DisConnected...");
	LOG_Write(str, SUCCESS);

	LOG_End();
	
	return SUCCESS;
}

/*-----------------------------------------------
	timesten prepare
-----------------------------------------------*/
int TT_Prepare(SQLHSTMT hStmt, SQLCHAR* g_szSql )
{
	char		str[1000];
	SQLRETURN   rc = SQL_SUCCESS;    
	
	rc = SQLPrepare(hStmt, g_szSql, SQL_NTS);
	if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
	{
		memset ( str , '\0', 1000);
		strcpy ( str , "Error PrePare...\n");
		CheckReturnCode(rc, str, NULL, hStmt, __FILE__, __LINE__);
		LOG_Write(str, FAILURE);
		return FAILURE;
	}
	return SUCCESS;
}

/*-----------------------------------------------
	timesten execute
-----------------------------------------------*/
int TT_Execute(SQLHSTMT hStmt )
{
	char		str[1000];
	SQLRETURN   rc = SQL_SUCCESS;   

	rc = SQLExecute(hStmt);
	if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
	{
		memset ( str , '\0', 1000);
		strcpy ( str , "Error Execute...\n");
		CheckReturnCode(rc, str, NULL, hStmt, __FILE__, __LINE__);
		LOG_Write(str, FAILURE);
		return FAILURE;
	}
	return SUCCESS;
}

/*-----------------------------------------------
	timesten bind parameter
-----------------------------------------------*/
int TT_BindParam(SQLHSTMT hStmt, int order, FIELDINFO field, SQLPOINTER context )
{
	char		str[1000];
	SQLRETURN   rc = SQL_SUCCESS;   
	SQLLEN		context_len;
	
	rc = SQLBindParameter( hStmt, order, SQL_PARAM_INPUT,
													field.ctype, field.sqltype,
													field.precision, field.scale,
													context, 0, NULL);
	
	if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
	{
		memset ( str , '\0', 1000);
		strcpy ( str , "Error BindParam...\n");
		CheckReturnCode(rc, str, NULL, hStmt, __FILE__, __LINE__);
		LOG_Write(str, FAILURE);
		return FAILURE;
	}
	return SUCCESS;

}

/*-----------------------------------------------
	timesten NumResultCols
-----------------------------------------------*/
int TT_NumResultCols (SQLHSTMT hStmt, SQLSMALLINT *numCols )
{
	char		str[1000];
	SQLRETURN   rc = SQL_SUCCESS;

	rc = SQLNumResultCols(hStmt, numCols);
	if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
	{
		memset ( str , '\0', 1000);
		strcpy ( str , "Error using numresultcols...\n");
		CheckReturnCode(rc, str, NULL, hStmt, __FILE__, __LINE__);
		LOG_Write(str, FAILURE);
		return FAILURE;
	}

	return SUCCESS;
}

/*-----------------------------------------------
	timesten BindCol
-----------------------------------------------*/
int	TT_BindCol( SQLHSTMT hStmt, SQLSMALLINT col, SQLSMALLINT coltype,
								SQLPOINTER colValue, SQLINTEGER BufferLength, SQLLEN *outlen )
{
	char		str[1000];
	SQLRETURN   rc = SQL_SUCCESS;

	rc = SQLBindCol(hStmt, col, coltype, colValue, BufferLength, outlen);
	if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
	{
		memset ( str , '\0', 1000);
		strcpy ( str , "Error using SQLBindCol...\n");
		CheckReturnCode(rc, str, NULL, hStmt, __FILE__, __LINE__);
		LOG_Write(str, FAILURE);
		return FAILURE;
	}
	
	return SUCCESS;
}

/*-----------------------------------------------
	timesten SQLDirect      
-----------------------------------------------*/
int TT_ExecDirect( SQLHSTMT hStmt, char* query )
{
	char		str[1000];
	SQLRETURN   rc = SQL_SUCCESS;

	rc = SQLExecDirect( hStmt, (SQLCHAR*)query, SQL_NTS );
	if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO) {
		memset ( str , '\0', 1000);
		strcpy ( str , "Error using SQLExecDirect...\n");
		CheckReturnCode(rc, str, NULL, hStmt, __FILE__, __LINE__);
		/* Exception : Create Table tb_test1 */
		if ( memcmp(query, "CREATE", 6) != 0 )	
		{
			LOG_Write(str, FAILURE);
			return FAILURE;
		}
	}
	return SUCCESS;
}

/*-----------------------------------------------
	timesten SQLFetch      
-----------------------------------------------*/
int TT_Fetch(SQLHSTMT hStmt )
{
	return SQLFetch(hStmt);	
}

/*-----------------------------------------------
	timesten prepare
-----------------------------------------------*/
int TT_RowCount(SQLHSTMT hStmt, SQLLEN *row_count )
{
	char		str[1000];
	SQLRETURN   rc = SQL_SUCCESS;    
	
	rc = SQLRowCount(hStmt, row_count);
	if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
	{
		memset ( str , '\0', 1000);
		strcpy ( str , "Error Rowcount...\n");
		CheckReturnCode(rc, str, NULL, hStmt, __FILE__, __LINE__);
		LOG_Write(str, FAILURE);
		return FAILURE;
	}
	return rc;
}

/*-----------------------------------------------
	timesten Set Autocommit    
-----------------------------------------------*/
int TT_Autocommit(SQLHDBC hDbc, int on_off )
{
	char		str[1000];
	SQLRETURN   rc = SQL_SUCCESS;

	rc = SQLSetConnectOption( hDbc, SQL_AUTOCOMMIT, on_off);
	if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
	{
		memset ( str , '\0', 1000);
		strcpy ( str , "Error Autocommit...\n");
		CheckReturnCode(rc, str, hDbc, NULL, __FILE__, __LINE__);
		LOG_Write(str, FAILURE);
		return FAILURE;
	}

	return SUCCESS;
}

/*-----------------------------------------------
	timesten Set Commit/Rollback    
-----------------------------------------------*/
int TT_Commit(SQLHDBC hDbc, int commit_opt )
{
	char				str[1000];
	SQLRETURN   rc = SQL_SUCCESS;

	rc = SQLTransact(hEnv, hDbc, commit_opt);
	if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
	{
		memset ( str , '\0', 1000);
		strcpy ( str , "Error Commit/Rollback...\n");
		CheckReturnCode(rc, str, hDbc, NULL, __FILE__, __LINE__);
		LOG_Write(str, FAILURE);
		return FAILURE;
	}

	return SUCCESS;
}

/*-----------------------------------------------
    Set Log Filename
-----------------------------------------------*/
void LOG_SetFilename(char* act)
{
	int pid_t;
	char filename[30];

	pid_t = getpid();

	memset ( filename, '\0', 30 );
	sprintf ( filename, "log/BMT_%1.1s%d.log", act, pid_t);

	fpout = fopen( filename , "w" );
	memset ( comment, '\0', 100 );
	memcpy ( comment, "Logging start... ", 26 );
	LOG_Write( comment, SUCCESS );
}

/*-----------------------------------------------
    Write comment,time
-----------------------------------------------*/
void LOG_Write(char* str, int opt)
{
	char buff[100];
	char buff2[100];
	
	clock_gettime(CLOCK_REALTIME, &ntime_now);
	t = localtime(&ntime_now.tv_sec);
	
	memset ( buff2, '\0', 100);	
	sprintf( buff2, "%d-%d-%d %-2.2d:%-2.2d:%-2.2d", 
					t->tm_year+1900, t->tm_mon+1, t->tm_mday,
					t->tm_hour, t->tm_min, t->tm_sec);
	
	memset ( buff , '\0' , 100 );
	strcpy ( buff , buff2);
	memset ( buff2, '\0', 100);	
	sprintf ( buff2 , "[%-7.7d] : ", ntime_now.tv_nsec/100 );
	strcat ( buff , buff2);
	strcat ( buff , str);
	memset ( buff2, '\0', 100);	
	sprintf ( buff2 , "\n" );
	strcat ( buff , buff2);

	fprintf ( fpout , buff );

	if ( opt == FAILURE )
		fclose(fpout);
}

/*-----------------------------------------------
    End Log
-----------------------------------------------*/
void LOG_End()
{
  memset ( comment, '\0', 100 );
  memcpy ( comment, "Logging end... \n", 24 );
  LOG_Write(comment, SUCCESS);

  fclose(fpout);
}

/*-----------------------------------------------
    Start TPS Check
-----------------------------------------------*/
void LOG_SetStart(char* Prog_nm)
{
  char buff[100];
  char buff2[100];

  

  clock_gettime(CLOCK_REALTIME, &ntime_start);
  t = localtime(&ntime_start.tv_sec);

  memset ( buff2, '\0', 100 );
  sprintf( buff2, "%d-%d-%d %-2.2d:%-2.2d:%-2.2d", 
		  t->tm_year+1900, t->tm_mon+1, t->tm_mday,
		  t->tm_hour, t->tm_min, t->tm_sec);
  memset ( buff , '\0', 100 );
  strcpy ( buff , buff2);
  memset ( buff2, '\0', 100 );
  sprintf ( buff2 , "[%-7.7d] : Check Time... %s Start\n", ntime_start.tv_nsec/100, Prog_nm );
  strcat ( buff , buff2 );

  fprintf ( fpout , buff );
}

/*-----------------------------------------------
    End TPS Check, Write TPS
-----------------------------------------------*/
float LOG_SetEnd( char* Prog_nm, long rows)
{
  char buff[100];
  char buff2[100];
  float tps;

  clock_gettime(CLOCK_REALTIME, &ntime_end);
  t = localtime(&ntime_end.tv_sec);

  memset ( buff2, '\0', 100 );
  sprintf( buff2, "%d-%d-%d %-2.2d:%-2.2d:%-2.2d", 
		  t->tm_year+1900, t->tm_mon+1, t->tm_mday,
		  t->tm_hour, t->tm_min, t->tm_sec);
  memset ( buff , '\0', 100 );
  strcpy ( buff , buff2);
  memset ( buff2, '\0', 100 );
  sprintf ( buff2 , "[%-7.7d] : Check Time... %s End\n", ntime_end.tv_nsec/100, Prog_nm );
  strcat ( buff , buff2 );

  fprintf ( fpout , buff );

  memset ( buff, '\0', 100 );
  sprintf ( buff , "     --> Total Time : %10.2f\n" , 
			 ( ((double)(ntime_end.tv_sec*10000)+((double)ntime_end.tv_nsec/100000.0)) -
			   ((double)(ntime_start.tv_sec*10000)+((double)ntime_start.tv_nsec/100000.0)) )/10000.0 );
  fprintf ( fpout , buff );
#ifndef TPSONLY
  fprintf(stdout, "%s", buff);
#endif

  tps =    ((double)rows / ( ((double)ntime_end.tv_sec+((double)ntime_end.tv_nsec/1000000000.0)) -
                             ((double)ntime_start.tv_sec+((double)ntime_start.tv_nsec/1000000000.0)) ) ) ;
  memset ( buff, '\0', 100 );
  sprintf ( buff , "     -->   TPS      : %10.2f\n" , tps);
  fprintf ( fpout , buff );
  fprintf ( stdout, "%s", buff);

  return tps;
}

void LOG_SetFetchcount( long count)
{
  char buff[100];

  memset ( buff, '\0', 100 );
  sprintf ( buff , "     -->Fetch count : %ld \n",count);
  fprintf ( fpout , buff );
  fprintf ( stdout, "%s", buff);
}
