/**********************************************************
                 ABLEX TigerPools Project

   Author   : Park Si Woo
   Section  : KTF Project
   SCCS ID  : @(#)lwrite.c	1.20
   Date     : 9/9/03
   Revision History :
        '01.  7. 21     Initial
		'02. 11. 14		Update LOG By Lee Sang Ho

   Description:

   Copyright (c) ABLEX 2001
***********************************************************/

#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <ctype.h>

#include <logutil.h>

FILE	*user_def_log;
FILE	*user_def_deb_log;
FILE	*dtaf_log;
int		last_tafno = 0;

int g_procidx;
char g_logfilepath[1024];
pid_t g_pid;
char g_procname[64];

int     vdLogLevel = 4;
//int     vdLogLevel = 1;


int log_close()
{
	if(user_def_log != NULL)
		fclose(user_def_log);

	if(user_def_deb_log != NULL)
		fclose(user_def_deb_log);

	user_def_log = NULL;
	user_def_deb_log = NULL;

	return 1;
}

int dDebugFileCheck(int strsize)
{
	FILE    	*fptr;
	char		debug[DEF_DEBUG_NUM][80];
	char    	dir_path[80], mesg_path[80];
	struct stat stat_log;
	DIR			*dirp;
	int			i;

	if(g_logfilepath == NULL)
        sprintf(g_logfilepath, "%s", LOG_PATH);

	sprintf(mesg_path, "%s/DEBUG/debuglog.0", g_logfilepath);
	if(stat(mesg_path, &stat_log) == 0) 
	{
		if( (stat_log.st_size + strsize ) > LOG_FILE_SIZE )
		{
           	log_close();

			for(i = 0; i < DEF_DEBUG_NUM; i++)
				sprintf(debug[i], "%s/DEBUG/debuglog.%d", 
				g_logfilepath, i);

			for(i = DEF_DEBUG_NUM - 1; i > 1; i--)
			{
				remove(debug[i]);
				rename(debug[i-1], debug[i]);
			}
            remove(debug[i]);
            rename(mesg_path, debug[i]);

            unlink(mesg_path);
		}
	} 
	else 
	{
		dirp = opendir(g_logfilepath);

		if( dirp == (DIR*)NULL )
			mkdir(g_logfilepath, 0777 );
		else
			closedir(dirp);

		sprintf(dir_path, "%s/DEBUG", g_logfilepath);
		mkdir(dir_path, 0777);

		user_def_deb_log = NULL;
	}

	if( user_def_deb_log == NULL )
    {
        if ((fptr = fopen(mesg_path, "a+")) == NULL)
            return -1;
        user_def_deb_log = fptr;
    }

	return 0;
}

int log_debug(char *fmt, ...)
{
    time_t      nowtime;
    struct  tm  check_time;
    int       strsize;
    char        msg[30720];
    va_list     ap;

    va_start( ap, fmt );
    vsprintf( msg, fmt, ap );

    strsize = strlen( msg );
    if(dDebugFileCheck(strsize) < 0)
        return -1;

    time(&nowtime);
    localtime_r(&nowtime, &check_time);

    fprintf (user_def_deb_log, "[%02d/%02d %02d:%02d:%02d] ",
        check_time.tm_mon+1, check_time.tm_mday,
        check_time.tm_hour, check_time.tm_min, check_time.tm_sec );

    vfprintf (user_def_deb_log, fmt, ap );
    fprintf( user_def_deb_log, "\n");
    fflush( user_def_deb_log );

    va_end( ap );

    return 1;
}


int log_write(char *fmt, ...)
{
	char    	dir_path[80], mesg_path[80];
	struct stat 	stat_log;
	time_t  	nowtime;
	struct  tm  	check_time, log_time;
	FILE		*fptr;
	char		deb_tmp[30720];
	va_list 	ap;
	DIR			*dirp;

	time(&nowtime);
	localtime_r(&nowtime, &check_time);

	if(g_logfilepath == NULL)
        sprintf(g_logfilepath, "%s", LOG_PATH);

	sprintf(mesg_path, "%s/%02d%02d/%02d", 
		g_logfilepath, check_time.tm_mon + 1, check_time.tm_mday, check_time.tm_hour);

	if (stat(mesg_path, &stat_log) == 0) 
	{
		localtime_r(&stat_log.st_atime, &log_time);
		if (check_time.tm_year != log_time.tm_year) 
		{
			unlink (mesg_path);
			log_close();
	
			user_def_log = NULL;
		}
	} 
	else 
	{
		log_close();
		dirp = opendir( g_logfilepath );
		if( dirp == (DIR*)NULL )
			mkdir( g_logfilepath, 0777 );
		else
			closedir( dirp );

		sprintf(dir_path, "%s/%02d%02d", 
			g_logfilepath, check_time.tm_mon+1, check_time.tm_mday);
		mkdir(dir_path, 0777);
		user_def_log = NULL;
	}

	if( user_def_log == NULL )
	{
		if ((fptr = fopen(mesg_path, "a+")) == NULL)
			return -1;
		user_def_log = fptr;
	}

	va_start( ap, fmt );
	fprintf (user_def_log, "[%d][%02d:%02d:%02d] ", 
		g_pid, check_time.tm_hour, check_time.tm_min, check_time.tm_sec );

	vfprintf (user_def_log, fmt, ap );
	vsprintf (deb_tmp, fmt, ap );
	fprintf( user_def_log,"\n");

	fflush( user_def_log );

	log_debug(deb_tmp);

	va_end( ap );

	return 1;
}

int create_date_time( char *szdate, char *sztime )
{

	time_t nowtime;
	struct tm check_time;

	time(&nowtime);
	localtime_r( &nowtime, &check_time );

	sprintf( szdate, "%04d%02d%02d",
		  check_time.tm_year+1900, check_time.tm_mon+1, check_time.tm_mday );

	sprintf( sztime, "%02d%02d%02d", 
		check_time.tm_hour, check_time.tm_min, check_time.tm_sec );

	return 1;

}


int log_hexa( unsigned char *szLog, int dSize )
{
	int 		i=0;
	time_t  	nowtime;
	struct  tm  check_time;

	time(&nowtime);
	localtime_r(&nowtime, &check_time);

	if(dDebugFileCheck(dSize) < 0)
        return -1;

	fprintf (user_def_deb_log, "[%02d/%02d %02d:%02d:%02d] ", 
		check_time.tm_mon+1, check_time.tm_mday, 
		check_time.tm_hour, check_time.tm_min, check_time.tm_sec );

    for( i=0; i<dSize ; i++ )
    {
        if( i%16 == 0 )
        {
            fprintf( user_def_deb_log, "\n");
            fprintf( user_def_deb_log, "%04d", i);
            fflush( user_def_deb_log );
        }

        if(i%4==0)
            fprintf( user_def_deb_log, " |");

        fprintf( user_def_deb_log, " %02X", szLog[i]);
    }

    fprintf( user_def_deb_log, "\n");
    fflush( user_def_deb_log );

	return 1;
}

int dAppDebug(int dType, int dLevel,  char *szMsg, struct tm *pstCheckTime, char *szName)
{
	int			dLen;
	time_t  	nowtime;
	char		szLevel[80];
	struct  tm  check_time;

	struct timeval	tvTime;

	dLen = strlen(szMsg);
	if(dDebugFileCheck(dLen) < 0)
		return -1;
	
	time(&nowtime);
	localtime_r(&nowtime, &check_time);

	if(dLevel == LOG_INFO)
		strcpy(szLevel, "INF");
	else if(dLevel == LOG_DEBUG)
		strcpy(szLevel, "DBG");
	else if(dLevel == LOG_WARN)
		strcpy(szLevel, "WRN");
	else if(dLevel == LOG_CRI)
		strcpy(szLevel, "CRI");
	else
		strcpy(szLevel, "UNK");

	gettimeofday(&tvTime, NULL);
#if defined(MICRO_LOG)
    fprintf (user_def_deb_log,
    "[%s:%d][%02d:%02d:%02d.%06ld][%s] [%s]\n",
    g_procname, g_pid,
    check_time.tm_hour, check_time.tm_min, 
	check_time.tm_sec, tvTime.tv_usec,
    szLevel, szMsg);
#endif

#if !defined(MICRO_LOG)
	fprintf (user_def_deb_log, 
	"[%s:%d][%02d/%02d %02d:%02d:%02d][%s] [%s]\n", 
	g_procname, g_pid,
	check_time.tm_mon+1, check_time.tm_mday, 
	check_time.tm_hour, check_time.tm_min, check_time.tm_sec, 
	szLevel, szMsg);
#endif

    fflush( user_def_deb_log );

	return 0;
}

int dAppWrite(int dLevel, char *szMsg)
{
	char    	dir_path[80], mesg_path[80], szLevel[80];
	struct stat stat_log;
	time_t  	nowtime;
	struct  tm  check_time, log_time;
	FILE		*fptr;
	DIR			*dirp;

	time(&nowtime);
	localtime_r(&nowtime, &check_time);

	if(g_logfilepath == NULL)
        sprintf(g_logfilepath, "%s", LOG_PATH);

	sprintf(mesg_path, "%s/%02d%02d/%02d", 
		g_logfilepath, check_time.tm_mon + 1, check_time.tm_mday, check_time.tm_hour);

	if (stat(mesg_path, &stat_log) == 0) 
	{
		localtime_r(&stat_log.st_atime, &log_time);
		if (check_time.tm_year != log_time.tm_year) 
		{
			unlink (mesg_path);
			log_close();
	
			user_def_log = NULL;
		}
	} 
	else 
	{
		log_close();
		dirp = opendir( g_logfilepath );
		if( dirp == (DIR*)NULL )
			mkdir( g_logfilepath, 0777 );
		else
			closedir( dirp );

		sprintf(dir_path, "%s/%02d%02d", 
			g_logfilepath, check_time.tm_mon+1, check_time.tm_mday);
		mkdir(dir_path, 0777);
		user_def_log = NULL;
	}

	if( user_def_log == NULL )
	{
		if ((fptr = fopen(mesg_path, "a+")) == NULL)
			return -1;
		user_def_log = fptr;
	}

	if(dLevel == LOG_INFO)
		strcpy(szLevel, "INF");
	else if(dLevel == LOG_DEBUG)
		strcpy(szLevel, "DBG");
	else if(dLevel == LOG_WARN)
		strcpy(szLevel, "WRN");
	else if(dLevel == LOG_CRI)
		strcpy(szLevel, "CRI");
	else
		strcpy(szLevel, "UNK");

	fprintf (user_def_log, 
		"[%s:%d][%02d/%02d %02d:%02d:%02d][%s] [%s]\n", 
		g_procname, g_pid,
		check_time.tm_mon+1, check_time.tm_mday, 
		check_time.tm_hour, check_time.tm_min, check_time.tm_sec, 
		szLevel, szMsg);

	fflush( user_def_log );
#if !defined(COMMERCIALLOG)
	dAppDebug(LOG_TYPE_WRITE, dLevel, szMsg, &check_time, szLevel);
#endif

	return 1;
}



int dAppLog(int dIndex, char *fmt, ...)
{
    char            szMsg[30720];
    va_list         args;

#if 0
    va_start(args, fmt);
//    vsprintf(szMsg, fmt, args);
    vsnprintf(szMsg, 30720, fmt, args);
	va_end(args);
#endif

/*
	vdLogLevel = gLogLevel->stLogLevel[g_procidx].usLogLevel;
*/

	switch(dIndex)
	{
	case LOG_INFO :
	case LOG_DEBUG :
	case LOG_WARN :
		if(dIndex > vdLogLevel)
			return 0;
    	va_start(args, fmt);
    	vsnprintf(szMsg, 30720, fmt, args);
		va_end(args);
		dAppDebug(LOG_TYPE_DEBUG, dIndex, szMsg, NULL, NULL);
		break;
	case LOG_CRI :
		if(dIndex > vdLogLevel)
			return 0;
    	va_start(args, fmt);
    	vsnprintf(szMsg, 30720, fmt, args);
		va_end(args);
		dAppWrite(dIndex, szMsg);
		break;
	case LOG_NOPRINT :
	default :
		return -1;
	}


    return 0;
}

void InitAppLog(pid_t pid, int procidx, char *logfilepath, char *proc_name)
{
	g_pid = pid;
	g_procidx = procidx;
	sprintf(g_logfilepath, "%s", logfilepath);
	sprintf(g_procname, "%s", proc_name);
}




#define WIDTH   16
int
dAppDump(char *s,int len)
{
    char buf[BUFSIZ],lbuf[BUFSIZ],rbuf[BUFSIZ];
    unsigned char *p;
    int line,i;

    p =(unsigned char *) s;
    for(line = 1; len > 0; len -= WIDTH,line++) {
        memset(lbuf,0,BUFSIZ);
        memset(rbuf,0,BUFSIZ);

        for(i = 0; i < WIDTH && len > i; i++,p++) {
            sprintf(buf,"%02x ",(unsigned char) *p);
            strcat(lbuf,buf);
            sprintf(buf,"%c",(!iscntrl(*p) && *p <= 0x7f) ? *p : '.');
            strcat(rbuf,buf);
        }
        dAppLog(LOG_DEBUG, "%04x: %-*s    %s",line - 1,WIDTH * 3,lbuf,rbuf);
    }
    if(!(len%16)) dAppLog(LOG_DEBUG, "\n");

    return line;
}
