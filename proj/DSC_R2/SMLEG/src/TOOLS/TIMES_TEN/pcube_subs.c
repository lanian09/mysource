/*******************************************/
/* Title : Timesten SAMPLE SOURCE API.     */
/* Date  : 2007.09.10  juno.               */
/*******************************************/
#include <sqlunix.h>
#include <sql.h>
#include <sqlext.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include "tt_basic.h"

#define  SUCCESS       0
#define  FAILURE      -1

/* -----------------------------------------------
	 Parameter Prototype
----------------------------------------------- */

FIELDINFO NAME_INDEX_FieldList = { SQL_C_LONG,      SQL_INTEGER,    0,  0 };        /* C1   INTEGER  NOT NULL   */
FIELDINFO NAME_FieldList = { SQL_C_CHAR,      SQL_CHAR,       64, 0 };        /* C2   CHAR(10) NULL       */
FIELDINFO NAME_HASH_CODE_FieldList = { SQL_C_LONG,      SQL_INTEGER,    0,  0 };        /* C3   INTEGER  NOT NULL   */
FIELDINFO DOMAIN_INDEX_FieldList = { SQL_C_LONG,      SQL_INTEGER,    0, 0 };        /* C4   INTEGER NOT NULL       */
FIELDINFO STATE_FieldList = { SQL_C_CHAR,      SQL_CHAR,       2048, 0 };        /* C5   CHAR(10) NULL       */
FIELDINFO STATE_TIMESTAMP_FieldList = { SQL_C_TIMESTAMP, SQL_DATE,       0, 0 };        /* C6   CHAR(10) NULL       */
FIELDINFO LAST_UPDATE_FieldList = { SQL_C_TIMESTAMP, SQL_DATE,       0, 0 };        /* C7   CHAR(10) NULL       */
FIELDINFO PARENT_FieldList = { SQL_C_LONG,      SQL_INTEGER,    0,  0 };        /* C8   INTEGER  NOT NULL   */

//FIELDINFO IP_FieldList = { SQL_C_LONG,      SQL_INTEGER,    0,  0 };        /* C1   INTEGER  NOT NULL   */
FIELDINFO IP_FieldList = { SQL_C_CHAR,      SQL_CHAR,       64, 0 };        /* C2   CHAR(10) NULL       */
FIELDINFO VAL_FieldList = { SQL_C_LONG,      SQL_INTEGER,       0, 0 };        /* C2   INTEGER NOT NULL       */
FIELDINFO CNT_FieldList = { SQL_C_LONG,      SQL_INTEGER,       0, 0 };        /* C2   INTEGER NOT NULL       */


SQLINTEGER      NAME_INDEX;
SQLCHAR         NAME[64+1];
SQLINTEGER      NAME_HASH_CODE;
SQLCHAR         DOMAIN_INDEX;
SQLCHAR         STATE[2048+1];
TIMESTAMP_STRUCT STATE_TIMESTAMP;
TIMESTAMP_STRUCT LAST_UPDATE;
SQLINTEGER      PARENT;
//SQLINTEGER		IP;
SQLCHAR         IP[64+1];
SQLINTEGER		VAL;
SQLINTEGER		CNT;

/* ------------
	PIPE
------------ */
char buffer[10];
int **fd_p;
int **fd_c;



void Usage();

int  Select(SQLHDBC hDbc, long start, long end, char *imsi);
void MainProcess( void *arg_rec, int pro_cnt, int order, char  *imsi );


int  Select(SQLHDBC hDbc, long start, long end, char *imsi)
{
	char	str[1000];
	char	lsql_select[4096] = {0,};				

	SQLHSTMT    hStmt = SQL_NULL_HSTMT;
	SQLRETURN   rc = SQL_SUCCESS;
	SQLLEN		outlen[10];
	int     	idx, tps;
	long		row_num;
	int			rows_fetched;
	int			select_type = 0;

	if ( TT_AllocStmt(hDbc, &hStmt) != SUCCESS )  
        return FAILURE;

	/*
	sprintf(lsql_select, " SELECT rows %d to %d"
							" NAME_INDEX, NAME, NAME_HASH_CODE, DOMAIN_INDEX, STATE, STATE_TIMESTAMP, "
							" LAST_UPDATE, PARENT "
						" FROM PCUBE.SUBS " ,start,end);
						*/
	if( end > start )
	{
		sprintf(lsql_select, " select rows %d to %d a.name, b.ip, a.last_update, c.val "
				" from pcube.subs a, pcube.mappings_ip b, pcube.tunables c "
				" where a.name_index=b.name_index AND a.name_index=c.name_index AND c.tunable_index=0 "
				"order by a.last_update desc, a.name, b.ip, c.val;",start,end);
		/*
		sprintf(lsql_select, " select rows %d to %d a.name, a.last_update, c.val "
				" from pcube.subs a, pcube.tunables c "
				" where a.name_index=c.name_index AND c.tunable_index=0 "
				"order by a.last_update desc, a.name, c.val;",start,end);
				*/
		/*
		sprintf(lsql_select, " select rows %d to %d name,last_update from pcube.subs" ,start,end);
		*/

		TT_BindCol(hStmt, ++idx, NAME_FieldList.ctype, NAME,  sizeof(NAME),  &outlen[0] );
		TT_BindCol(hStmt, ++idx, IP_FieldList.ctype, IP,   sizeof(IP),  &outlen[1] );
		TT_BindCol(hStmt, ++idx, LAST_UPDATE_FieldList.ctype, &LAST_UPDATE,   sizeof(LAST_UPDATE),  &outlen[2] );
		TT_BindCol(hStmt, ++idx, VAL_FieldList.ctype, &VAL,   sizeof(VAL),  &outlen[3] );

		select_type = 1;
	}
	else if( start == 1 && end == 1 )
	{
		sprintf(lsql_select, "select count(*) from pcube.subs a, pcube.mappings_ip b, pcube.tunables c "
			"where a.name_index=b.name_index AND a.name_index=c.name_index AND c.tunable_index=0;"); 
		TT_BindCol(hStmt, ++idx, CNT_FieldList.ctype, &CNT,   sizeof(CNT),  &outlen[0] );

		select_type = 2;
	}
	else if( start == 0 && end == 0 && strlen(imsi) == 15 ) // subs NAME_INDEX, LAST_UPDATE 
	{
		sprintf(lsql_select, "select name_index, last_update from subs where name = '%s';", imsi); 
		TT_BindCol(hStmt, ++idx, NAME_INDEX_FieldList.ctype, &NAME_INDEX,   sizeof(NAME_INDEX),  &outlen[0] );
		TT_BindCol(hStmt, ++idx, LAST_UPDATE_FieldList.ctype, &LAST_UPDATE,   sizeof(LAST_UPDATE),  &outlen[1] );

		select_type = 3;
	}
	else if( start == 0 && end == 0 && strlen(imsi) < 15 ) // mappings_ip IP
	{
		sprintf(lsql_select, "select b.ip from subs a, mappings_ip b "
			"where b.name_index = %s and a.name_index=b.name_index;", imsi); 
////		TT_BindCol(hStmt, ++idx, IP_FieldList.ctype, &IP,   sizeof(IP),  &outlen[0] );
		TT_BindCol(hStmt, ++idx, IP_FieldList.ctype, IP,   sizeof(IP),  &outlen[0] );

		select_type = 4;
	}
	else if( start == 1 && end == 0 ) // tunables pkgId
	{
		sprintf(lsql_select, "select b.val from subs a, tunables b "
			"where b.name_index = %s and a.name_index=b.name_index;", imsi); 
		TT_BindCol(hStmt, ++idx, VAL_FieldList.ctype, &VAL,   sizeof(VAL),  &outlen[0] );

		select_type = 5;
	}

	if ( TT_Prepare(hStmt, (SQLCHAR *)lsql_select) != SUCCESS )
	{
		printf("prepare fail\n");
		TT_FreeStmt( hStmt, SQL_DROP);
        return FAILURE;
	}

	/*----- Binding Param... ----------------*/
//	TT_BindParam(hStmt, ++idx, NAME_INDEX_FieldList, &NAME );

    idx = 0;
	/*----- Binding Col... ----------------*/
	/*
	TT_BindCol(hStmt, ++idx, NAME_INDEX_FieldList.ctype, &NAME_INDEX,   sizeof(NAME_INDEX),  &outlen[0] );
	TT_BindCol(hStmt, ++idx, NAME_FieldList.ctype, NAME,  sizeof(NAME),  &outlen[1] );
	TT_BindCol(hStmt, ++idx, NAME_HASH_CODE_FieldList.ctype, &NAME_HASH_CODE,  sizeof(NAME_HASH_CODE),  &outlen[2] );
	TT_BindCol(hStmt, ++idx, DOMAIN_INDEX_FieldList.ctype, &DOMAIN_INDEX,   sizeof(DOMAIN_INDEX),  &outlen[3] );
	TT_BindCol(hStmt, ++idx, STATE_FieldList.ctype, STATE,   sizeof(STATE),  &outlen[4] );
	TT_BindCol(hStmt, ++idx, STATE_TIMESTAMP_FieldList.ctype, &STATE_TIMESTAMP,   sizeof(STATE_TIMESTAMP),  &outlen[5] );
	TT_BindCol(hStmt, ++idx, LAST_UPDATE_FieldList.ctype, &LAST_UPDATE,   sizeof(LAST_UPDATE),  &outlen[6] );
	TT_BindCol(hStmt, ++idx, PARENT_FieldList.ctype, &PARENT,   sizeof(PARENT),  &outlen[7] );
	*/

//	TT_BindCol(hStmt, ++idx, NAME_FieldList.ctype, NAME,  sizeof(NAME),  &outlen[0] );
//	TT_BindCol(hStmt, ++idx, IP_FieldList.ctype, &IP,   sizeof(IP),  &outlen[1] );
//	TT_BindCol(hStmt, ++idx, VAL_FieldList.ctype, &VAL,   sizeof(VAL),  &outlen[2] );


	/*
	memset ( str, '\0', 1000 );
    sprintf(str, "Selecting Data...%ld ~ %ld", start, end);
	*/
#ifndef TPSONLY
	fprintf(stdout, str);
	fflush(stdout);
	fprintf(stdout, "\n");
	fflush(stdout);
#endif

//	LOG_SetStart(str);				/* check time~ START!!! */
	/*
	for(row_num=start; row_num<=end; row_num++)
	{
	*/
        rows_fetched = 0;
		if ( (rc = TT_Execute(hStmt)) != SUCCESS )
		{
			TT_Commit(hDbc, SQL_ROLLBACK);
			TT_FreeStmt( hStmt, SQL_DROP);
			return FAILURE;
		}
		while ((rc = TT_Fetch(hStmt)) == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
		{
/*
printf(" C1[%d] - C2[%s] - C3[%d] - C4[%d] - C5[%s] - "
						, NAME_INDEX, NAME, NAME_HASH_CODE, DOMAIN_INDEX, STATE  );
	*/
			if( select_type == 1 )
			{
				//printf("%s,%d,%d ", NAME, IP, VAL  );
				printf("%s,%s,%02d-%02d-%02d %02d:%02d:%02d,%d ", NAME, IP, LAST_UPDATE.year, LAST_UPDATE.month,
						LAST_UPDATE.day, LAST_UPDATE.hour, LAST_UPDATE.minute, LAST_UPDATE.second, VAL );
				/*
				printf("%s,%02d-%02d-%02d %02d:%02d:%02d,%d ", NAME, LAST_UPDATE.year, LAST_UPDATE.month,
						LAST_UPDATE.day, LAST_UPDATE.hour, LAST_UPDATE.minute, LAST_UPDATE.second, VAL );
						*/
			}
			else if( select_type == 2 )
			{
				printf("%d", CNT);
			}
			else if( select_type == 3 )
			{
				printf("%d,%02d-%02d-%02d %02d:%02d:%02d ", NAME_INDEX, LAST_UPDATE.year, LAST_UPDATE.month,
						LAST_UPDATE.day, LAST_UPDATE.hour, LAST_UPDATE.minute, LAST_UPDATE.second );
			}
			else if( select_type == 4 )
			{
////				printf("%d", IP);
				printf("%s", IP);
			}
			else if( select_type == 5 )
			{
				printf("%d", VAL);
			}
            rows_fetched++;
		}
		if (rc != SQL_NO_DATA_FOUND)
		{
			TT_Commit(hDbc, SQL_ROLLBACK);
			TT_FreeStmt( hStmt, SQL_DROP);
			return FAILURE;
		}
		rc = TT_FreeStmt(hStmt, SQL_CLOSE);
		/*
        if (  rows_fetched != 1  )
        {   
			fprintf( stdout, "row(%d) --------------> Fail : the row does not exist\n", row_num);
			fflush(stdout);
			TT_Commit(hDbc, SQL_ROLLBACK);
			TT_FreeStmt( hStmt, SQL_DROP);
			return FAILURE;
        }
		*/
//	}
	fflush(stdout);
	TT_Commit(hDbc, SQL_COMMIT);
//	tps = LOG_SetEnd(str, end-start+1);				/* check time~ END!!! */
	printf("\nCommand terminated successfully\n");
	if ( TT_FreeStmt( hStmt, SQL_DROP ) != SUCCESS )
        return FAILURE;

    return tps;
}

/*---------------------------------------------------*/
/*                                                   */
/*              T E S T  M A I N                     */
/*                                                   */
/*---------------------------------------------------*/
void Usage()
{
   	fprintf(stderr, "Parameter error!!  \n");
   	fprintf(stderr, "Usage :  ttbmtc + [-d DSN name] + [-u UID] + [-p PASSWORD] + [-o oraclePWD] \n");
	fprintf(stderr, "                + Act[i,s,u,d] + Start_row + End_row + Process_num\n");
   	fflush(stderr);
}

int main(int argc, char *argv[])
{   
	ARGUMENT arg;
	int	status, i, proc_cnt, ret, client_err = 0;
	int total_tps=0;
	FILE *fp;
	char line[10];
	char	imsi[16] = {0,};
		 

	memset ( arg.ConnString, '\0', 100);
	memset ( arg.job_gb, '\0', 1);
	arg.start_row = 0;
	arg.end_row = 0;

//	if ( argc < 7 || (argc-5) % 2 != 0 || argv[1][0] != '-')
	if ( argc < 7 || argv[1][0] != '-')
	{
		Usage();
		exit(-1);
	}

	for (i=1 ; i<argc ; i++)
	{
		if ( memcmp ( argv[i], "-d" , 2 ) == 0 && i != argc - 1 ) {
			strcpy(arg.ConnString,"DSN=");
			strcat(arg.ConnString,argv[i+1]);
			i++;
		}
		else if ( memcmp ( argv[i], "-u" , 2 ) == 0 && i != argc - 1 ) {
			strcat(arg.ConnString,";UID=");
			strcat(arg.ConnString,argv[i+1]);
			i++;
		}
		else if ( memcmp ( argv[i], "-p" , 2 ) == 0 && i != argc - 1 ) {
			strcat(arg.ConnString,";PWD=");
			strcat(arg.ConnString,argv[i+1]);
			i++;
		}
		else if ( memcmp ( argv[i], "-o" , 2 ) == 0 && i != argc - 1 ) {
			strcat(arg.ConnString,";oraclePWD=");
			strcat(arg.ConnString,argv[i+1]);
			i++;
		}
		else if ( memcmp ( argv[i], "-" , 1 ) != 0 )
		{
			strcpy ( arg.job_gb, argv[i]);
			arg.start_row = (long)atoi(argv[i+1]);
			arg.end_row = (long)atoi(argv[i+2]);
			proc_cnt = atoi(argv[i+3]);
			if( argc > 11 )
			{
				strcpy(imsi, argv[i+4]);
				i=i+5;
			}
			else
				i=i+4;
		}
	}

	fp = popen("ls | grep log | wc -l", "r");
	fgets(line, 10, fp);
	for(i=0;i<10;i++)
	{
		if ( memcmp (line+i, " ", 1 ) != 0 )
			break;
	}
	if ( memcmp (line+i, "0", 1 ) == 0 )
		system("mkdir log");
	status = pclose(fp);

	int	*pid = (int*)malloc(proc_cnt * sizeof(int));

	fd_p =(int**)malloc(sizeof(int*)*proc_cnt);
	fd_c =(int**)malloc(sizeof(int*)*proc_cnt);
	for ( i=0;i<proc_cnt;i++)
	{
		fd_p[i]=(int*)malloc(sizeof(int)*2);
		fd_c[i]=(int*)malloc(sizeof(int)*2);
		status = pipe(fd_p[i]);
		if(status == -1)
			puts("pipe() error");
		status = pipe(fd_c[i]);
		if(status == -1)
			fprintf(stderr, "pipe() error");
	}

	ret = fork();
	for ( i=0;i<proc_cnt;i++){
		if(ret>0){
			pid[i]=ret;
			if ( i != proc_cnt-1)
				ret = fork();
		}
		else if ( ret == 0 ) {
			MainProcess ( &arg, proc_cnt, i ,imsi);
//			MainProcess ( &arg, proc_cnt, i );
			break;
		}
		else{
			fprintf(stderr, "fork Error\n");
			fflush(stderr);
			exit(1);
		}
	}

	if ( ret > 0 )
	{
		for ( i=0;i<proc_cnt;i++)
		{
			read(fd_c[i][0], buffer, 5);
			if ( memcmp ( buffer, "error", 5) == 0 )
				client_err = 1;
		}
		if ( client_err == 0 )
		{
			for ( i=0;i<proc_cnt;i++)
				write(fd_p[i][1], "start", 5);
			for ( i=0;i<proc_cnt;i++)
			{
				read(fd_c[i][0], buffer, 10);
				if ( memcmp ( buffer, "error", 5) == 0 )
					client_err = 1;
				total_tps += atoi(buffer);
			}
		}
		else
			for ( i=0;i<proc_cnt;i++)
				write(fd_p[i][1], "error", 5);

		for ( i=0 ; i<proc_cnt; i++ )
			while ( !waitpid(pid[i], &status, 0) )
				{};
		/*
		if( total_tps >= 0 && client_err == 0 )
			fprintf(stdout, "\n     -->  TOTAL TPS :%8d\n", total_tps);
			*/
		for ( i=0 ; i<proc_cnt; i++ ) 
		{
			close( fd_p[i][0] );
			close( fd_p[i][1] );
			close( fd_c[i][0] );
			close( fd_c[i][1] );
		}
	}
	for ( i=0;i<proc_cnt;i++) 
	{
		free(fd_p[i]);
		free(fd_c[i]);
	}
	free(fd_p);
	free(fd_c);
	free(pid);
}

void MainProcess( void *arg_rec, int pro_cnt, int order, char *imsi )
{
	SQLHDBC     hDbc  = SQL_NULL_HDBC;
	char		str[1000];
	char		tps_c[10];
	long start, end, tps;
	ARGUMENT    *arg = (ARGUMENT*)arg_rec;

	start = arg->start_row + ((arg->end_row - arg->start_row+1)/pro_cnt)*order;	
	if ( pro_cnt-1 == order )
		end = arg->end_row;
	else
		end = arg->start_row + ((arg->end_row-arg->start_row+1)/pro_cnt)*(order+1)-1;


	memset ( str , '\0', 1000);
	memcpy ( str , arg->job_gb, 1);
	LOG_SetFilename(str);

	if ( TT_Connect( &hDbc, (SQLCHAR*)arg[0].ConnString, SQL_AUTOCOMMIT_OFF ) != SUCCESS )		
	{
		write(fd_c[order][1], "error", 5);
		return;
	}
	memset ( str , '\0', 1000);
	strcpy ( str , "Connected...");
	LOG_Write(str, SUCCESS);

#ifndef TPSONLY
	fprintf (stdout, "TimesTen Connected...\n");
#endif

//	Create(hDbc);

	write(fd_c[order][1], "ready", 5);
	read(fd_p[order][0], buffer, 5);
	if ( memcmp( buffer, "error", 5 ) == 0 )
		if ( TT_DisConnect(hDbc) != SUCCESS )           
			return;

	switch (arg->job_gb[0])
	{
		case 'I':
		case 'i':
//			tps = Insert(hDbc, start, end);
			break;
	
		case 'S':
		case 's':
			tps = Select(hDbc, start, end, imsi);
			break;
		
		case 'U':
		case 'u':
//			tps = Update(hDbc, start, end);
			break;
		
		case 'D':
		case 'd':
//			tps = Delete(hDbc, start, end);
			break;
	
		default :
			fprintf ( stdout, "Argument Error!!  \n");
			fprintf ( stdout, "parameter Error!![%s]\n", arg->job_gb);
			write(fd_c[order][1], "error", 5);
			return;
	}

	if ( TT_DisConnect(hDbc) != SUCCESS )           
	{
		write(fd_c[order][1], "error", 5);
		return;
	}
		
	memset(tps_c, 0x00, 10);
	sprintf(tps_c, "%d", tps);
	write(fd_c[order][1], tps_c, strlen(tps_c));

	return;
}
