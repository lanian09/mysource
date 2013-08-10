#define  SUCCESS		0
#define  FAILURE		-1

typedef struct
{
	int     ctype;
	int     sqltype;
	unsigned long     precision;
	int     scale;
} FIELDINFO;

typedef struct
{
    char  ConnString[100];
    char  job_gb[1];
    long  start_row;
    long  end_row;
} ARGUMENT;

char   comment[100];
FILE*  fpout;
struct timespec ntime_start, ntime_end, ntime_now;
struct tm *t;

void CheckReturnCode (SQLRETURN rc, char* msg, SQLHDBC hDbc, SQLHSTMT hStmt, char *filename, int lineno);
int TT_ExecDirect( SQLHSTMT hStmt, char* query );
int TT_Connect( SQLHDBC *hDbc, SQLCHAR* g_connStr, int autocommit_opt );
int TT_AllocStmt( SQLHDBC hDbc, SQLHSTMT *hStmt );
int TT_FreeStmt( SQLHSTMT hStmt, SQLUSMALLINT OPTION );
int TT_DisConnect(SQLHDBC hDbc );
int TT_Prepare(SQLHSTMT hStmt, SQLCHAR* g_szSql );
int TT_Execute(SQLHSTMT hStmt );
int TT_BindParam(SQLHSTMT hStmt, int order, FIELDINFO field, SQLPOINTER context );
int TT_NumResultCols (SQLHSTMT hStmt, SQLSMALLINT *numCols );
int TT_BindCol( SQLHSTMT hStmt, SQLSMALLINT col, SQLSMALLINT coltype,
		                SQLPOINTER colValue, SQLINTEGER BufferLength, SQLLEN *outlen );
int TT_Fetch(SQLHSTMT hStmt );
int TT_RowCount(SQLHSTMT hStmt, SQLLEN *row_count );
int TT_Autocommit(SQLHDBC hDbc, int on_off );
int TT_Commit(SQLHDBC hDbc, int commit_opt );
void LOG_SetFilename(char* act);
void LOG_Write(char* str, int opt);
void LOG_End();
void LOG_SetStart(char* Prog_nm);
float LOG_SetEnd( char* Prog_nm, long rows);
void LOG_SetFetchcount( long count);
