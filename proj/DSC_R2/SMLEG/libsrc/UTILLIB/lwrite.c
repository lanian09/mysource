/**A.1*  File Inclusion ***********************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <netinet/in.h>

#include <utillib.h>
#include <define.h>

#include <sys/timeb.h>
#include <ipaf_shm.h>
#include <comm_loglevel.h>
#include <sys/shm.h>

/**B.1*  Definition of New Constants *********************/
#define	FVERSION	"1003                    "
#define LOG_FILE_SIZE       20000000
#define BEACON_FILE_SIZE    10000000
#define PROC_NAME_LENGTH    16
#define MAX_DEBUG_FILE      10

#define SYSCONF_FILE_NAME   "/DSC/NEW/DATA/sysconfig"

/**B.2*  Definition of New Type  **************************/

/* Beacon File Header Format */
typedef struct {
	union {
		struct _Info {
			char	cHeaderLen;	/* 16 */		
			char	cFlag;		/* 1: Packet log, 2: Msg log */
			short	sRetCode;	/* 0:Success, others: ErrorCode */
		} Info;
		long	lPacketNo;
	};   
	long  	lSize;		/* body size */
	time_t	curtime;	/* current time */
	long	ucurtime;	/* current micro time */
} st_BeaconHdr;

/**C.1*  Declaration of Variables  ************************/	
		   
extern int errno;

pid_t 	g_procidx;
char	g_procname[16];
char	g_MPName[64];
int 	g_MPLogMode = 0;
char	g_logfilepath[1024];
char	g_beaconfilepath[1024];
char	g_errorfilepath[1024];
int     vdLogLevel = 4;

char	g_cdrfilepath[1024];

char	szLogBuf[1024];

int		g_iExt = 0;
int		g_logno = 0;
time_t	g_prevtime = 0;

FILE    *user_def_log = NULL;
FILE    *user_def_deb_log;
FILE    *user_def_deb_log2;
FILE	*user_def_beacon;
FILE	*fpError;

FILE	*user_def_cdr = NULL;

extern char logfilepath[1024];
extern int proc_idx;
extern int pid;

extern  T_GENINFO           *gen_info;
int conflib_getNthTokenInFileSection ( char *fname, char *section, char *keyword, int n, char *string );


stLogLevel      *gLogLvl;
unsigned char   *gMyLogLvl;

typedef struct{
    int     iLogLevel;
    char    szProcName[32];
} st_ProcInfo_t, *pst_ProcInfo_t;

st_ProcInfo_t gst_ProcInfo[LOGLVL_APPINX_NUM] = {
//    {LOGLVL_APPINX_UAWAPANA,    "UAWAPANA"},
//    {LOGLVL_APPINX_AAAIF,       "AAAIF"},
//    {LOGLVL_APPINX_UDRGEN,      "UDRGEN"},
    {LOGLVL_APPINX_CAPD,        "CAPD"},
    {LOGLVL_APPINX_PANA,        "PANA"},
//    {LOGLVL_APPINX_CDR,         "CDR"},
    {LOGLVL_APPINX_TRCDR0,       "RLEG0"},
    {LOGLVL_APPINX_TRCDR1,       "RLEG1"},
    {LOGLVL_APPINX_TRCDR2,       "RLEG2"},
    {LOGLVL_APPINX_TRCDR3,       "RLEG3"},
    {LOGLVL_APPINX_TRCDR4,       "RLEG4"},
//    {LOGLVL_APPINX_WAP1ANA,     "WAP1ANA"},
//    {LOGLVL_APPINX_WAP2ANA,     "WAP2ANA"},
//    {LOGLVL_APPINX_HTTPANA,     "HTTPANA"},
//    {LOGLVL_APPINX_SDMD,        "SDMD"},
//	{LOGLVL_APPINX_VODSANA, 	"VODSANA"},
//	{LOGLVL_APPINX_WIPINWANA,	"WIPINWANA"},
//	{LOGLVL_APPINX_JAVANWANA, 	"JAVANWANA"},
//	{LOGLVL_APPINX_REANA, 		"REANA"},
//	{LOGLVL_APPINX_VTANA, 		"VTANA"},
//	{LOGLVL_APPINX_LOGM, 		"LOGM"},
    {LOGLVL_APPINX_RANA,    	"RANA"},
//    {LOGLVL_APPINX_FBANA,    	"FBANA"},
//    {LOGLVL_APPINX_OZCDR,    	"OZCDR"},
//    {LOGLVL_APPINX_MEMD,    	"MEMD"},
//    {LOGLVL_APPINX_WVANA,    	"WVANA"},
	{LOGLVL_APPINX_RDRANA,		"RDRANA"}, 	// ADD by jjinri 2009.04.24
	{LOGLVL_APPINX_SMPP,		"SMPP"}, 	/* by june, 2009.04.26 */
	{LOGLVL_APPINX_RDRCAPD,		"RDRCAPD"},	/* by yhshin, 2009.04.26 */
	{LOGLVL_APPINX_MMCR,		"MMCR"},	/* by yhshin, 2009.04.26 */
	{LOGLVL_APPINX_IXPC,		"IXPC"},	/* by sjjeon, 2009.05.15 */
	{LOGLVL_APPINX_SAMD,		"SAMD"} 	/* by sjjeon, 2009.05.15 */
};


int Init_logdebug( pid_t proc_idx, char *proc_name, char *logfilepath )
{
    struct stat	filestat;
	DIR			*dirp;
    int         iShmId = 0;
    int         key = 0;
    char        tmpGet[32];
    int         i = 0;
    int         iFlag = 0;

    memset(tmpGet, 0x00, 32);
	g_procidx = proc_idx;
	strcpy( g_procname, proc_name );
	g_procname[15] = '\0';
	sprintf( g_logfilepath, "%s/%s", logfilepath, proc_name);
	g_logfilepath[1023] = '\0';

	if( stat(g_logfilepath, &filestat) != 0 )
	{
        dirp =  opendir( g_logfilepath ) ;

        if( dirp == (DIR*)NULL )
            mkdir( g_logfilepath, 0777 );
        else
            closedir(dirp);
	}

    /* LogLevel Setting */
    if(conflib_getNthTokenInFileSection(SYSCONF_FILE_NAME,
				"SHARED_MEMORY_KEY", "SHM_LOG_LEVEL", 1, tmpGet) < 0)
    {
        return(-1);
    }
    key = strtol(tmpGet, 0, 0);
    if((iShmId = (int)shmget(key, sizeof(stLogLevel) 
                ,0666|IPC_CREAT))<0)
    {
		printf ("error -2: %d.%s\n", errno, strerror(errno));
        return(-2);
    }
    gLogLvl = (stLogLevel *) shmat (iShmId,0,0);
    if( gLogLvl < (stLogLevel *)0 )
    {
        return(-3);
    }

	for( i = 0; i < LOGLVL_APPINX_NUM; i++ )
    {
        if(!strcmp(gst_ProcInfo[i].szProcName, proc_name))
        {  
            gMyLogLvl = &gLogLvl->loglevel[gst_ProcInfo[i].iLogLevel][LOGLVL_KIND_APP];
			
            iFlag = 1;
            break;
        }
		//i++;
    }

    if(iFlag != 1)
        return -4;
    
	return 0;
}

int Init_MPAppLog(pid_t proc_idx, char *proc_name, int procnum, char *logfilepath)
{
	struct stat filestat;
	DIR         *dirp;
	int         iShmId = 0;
	int         key = 0;
	char        tmpGet[32];
	int         i = 0;
	int         iFlag = 0;

	memset(tmpGet, 0x00, 32);
	g_procidx = proc_idx;
	strcpy( g_procname, proc_name );
	g_procname[15] = '\0';
	sprintf( g_logfilepath, "%s/%s", logfilepath, proc_name);
	g_logfilepath[1023] = '\0';
	g_MPLogMode = 1;
	if( procnum == 0 ) {
		sprintf(g_MPName, "%s", proc_name);
	}
	else {
		sprintf(g_MPName, "%s%d", proc_name, procnum);
	}

	if( stat(g_logfilepath, &filestat) != 0 )
	{
		dirp =  opendir( g_logfilepath ) ;

		if( dirp == (DIR*)NULL )
			mkdir( g_logfilepath, 0777 );
		else
			closedir(dirp);
	}

	/* LogLevel Setting */
	if(conflib_getNthTokenInFileSection(SYSCONF_FILE_NAME,
				"SHARED_MEMORY_KEY", "SHM_LOG_LEVEL", 1, tmpGet) < 0)
	{
		return(-1);
	}
	key = strtol(tmpGet, 0, 0);
	if((iShmId = (int)shmget(key, sizeof(stLogLevel)
					,0666|IPC_CREAT))<0)
	{
		return(-2);
	}
	gLogLvl = (stLogLevel *) shmat (iShmId,0,0);
	if( gLogLvl < (stLogLevel *)0 )
	{
		return(-3);
	}

	//Set_ProcInfo();
	while(gst_ProcInfo[i].szProcName[0] != '\0')
	{
		if(!strcmp(gst_ProcInfo[i].szProcName, proc_name))
		{
			gMyLogLvl = &gLogLvl->loglevel[gst_ProcInfo[i].iLogLevel][LOGLVL_KIND_APP];
			iFlag = 1;
			break;
		}
		i++;
	}

	if(iFlag != 1)
		return -4;

	return 0;
}

int Init_logbeacon( char *logfilepath )
{
    struct stat	filestat;
	DIR			*dirp;

	strcpy( g_beaconfilepath, logfilepath);
	g_beaconfilepath[1023] = '\0';

	if( stat(g_beaconfilepath, &filestat) != 0 )
	{
        dirp =  opendir( g_beaconfilepath ) ;

        if( dirp == (DIR*)NULL )
            mkdir( g_beaconfilepath, 0777 );
        else
            closedir(dirp);
	}
	g_iExt = 0;

	return 0;
}

int Init_logerror( char *logfilepath )
{
	struct stat	filestat;
	DIR			*pDir;

	strcpy( g_errorfilepath, logfilepath );

	if( stat(g_errorfilepath, &filestat) != 0 )
	{
		pDir = opendir( g_errorfilepath );
		if( pDir == (DIR *)NULL )
			mkdir( g_errorfilepath, 0777 );
		else
			closedir( pDir );
	}

	return 0;	
}


int Init_logcdr( char *logfilepath )
{
    struct stat filestat;
    DIR         *dirp;

    sprintf( g_cdrfilepath, "%s", logfilepath);
    g_cdrfilepath[1023] = '\0';

    if( stat(g_cdrfilepath, &filestat) != 0 )
    {
        dirp =  opendir( g_cdrfilepath ) ;

        if( dirp == (DIR*)NULL )
            mkdir( g_cdrfilepath, 0777 );
        else
            closedir(dirp);
    }

    return 0;
}

int log_write_cdr( char *fmt, ... )
{
    char        dir_path[80], mesg_path[80];
    struct stat stat_log;
    time_t      nowtime;
    struct tm   check_time, log_time;
    FILE        *fptr;
    va_list     ap;
    DIR         *dirp;
    char        logmsg[4096];

    time(&nowtime);
    localtime_r(&nowtime, &check_time);

    sprintf(mesg_path, "%s/%02d%02d/%02d",
        g_cdrfilepath, check_time.tm_mon + 1, check_time.tm_mday,
        check_time.tm_hour);

    if( stat(mesg_path, &stat_log) == 0 )
    {
        localtime_r(&stat_log.st_atime, &log_time);
        if( check_time.tm_year != log_time.tm_year )
        {
            if( user_def_cdr != NULL )
                fclose( user_def_cdr );

            user_def_cdr = NULL;
            unlink(mesg_path);
        }
    }
	else
    {
        if( user_def_cdr != NULL )
            fclose( user_def_cdr );

        user_def_cdr = NULL;

        dirp =  opendir( g_logfilepath ) ;
        if( dirp == (DIR*)NULL )
            mkdir( g_cdrfilepath, 0777 );
        else
            closedir(dirp);

        sprintf(dir_path, "%s/%02d%02d",
            g_cdrfilepath, check_time.tm_mon+1, check_time.tm_mday);
        mkdir(dir_path, 0777);
    }


    if( user_def_cdr == NULL )
    {
        if ((fptr = fopen(mesg_path, "a+")) == NULL)
            return -1;

        user_def_cdr = fptr;
    }

    va_start( ap, fmt );

    fprintf (user_def_cdr, "[%02d/%02d %02d:%02d:%02d] ",
        check_time.tm_mon+1, check_time.tm_mday,
        check_time.tm_hour, check_time.tm_min, check_time.tm_sec);

    vfprintf (user_def_cdr, fmt, ap );
    vsprintf (logmsg, fmt, ap );
    fprintf( user_def_cdr,"\n");

    fflush( user_def_cdr );

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

	sprintf( sztime, "%02d%02d%02d", check_time.tm_hour, check_time.tm_min, check_time.tm_sec );

	return 1;

}


int log_write( char *fmt, ... )
{
#ifdef LOGDEBUG
    char        dir_path[80], mesg_path[80];
    struct stat stat_log;
    time_t      nowtime;
    struct tm   check_time, log_time;
    FILE        *fptr;
    va_list     ap;
	DIR			*dirp;
	char		logmsg[4096];

    time(&nowtime);
    localtime_r(&nowtime, &check_time);

    sprintf(mesg_path, "%s/%02d%02d/%02d",
        g_logfilepath, check_time.tm_mon + 1, check_time.tm_mday,
		check_time.tm_hour);

    if( stat(mesg_path, &stat_log) == 0 )
    {
        localtime_r(&stat_log.st_atime, &log_time);
        if( check_time.tm_year != log_time.tm_year )
        {
			if( user_def_log != NULL )
    			fclose( user_def_log );

            user_def_log = NULL;
            unlink(mesg_path);
        }
    }
    else
    {
		if( user_def_log != NULL )
    		fclose( user_def_log );
        
		user_def_log = NULL;

        dirp =  opendir( g_logfilepath ) ;
        if( dirp == (DIR*)NULL )
            mkdir( g_logfilepath, 0777 );
        else
            closedir(dirp);

        sprintf(dir_path, "%s/%02d%02d", 
			g_logfilepath, check_time.tm_mon+1, check_time.tm_mday);
        mkdir(dir_path, 0777);
    }


    if( user_def_log == NULL )
    {
        if ((fptr = fopen(mesg_path, "a+")) == NULL)
            return -1;

        user_def_log = fptr;
    }

    va_start( ap, fmt );

    fprintf (user_def_log, "[%02d/%02d %02d:%02d:%02d][%s:%u] ",
		check_time.tm_mon+1, check_time.tm_mday,
		check_time.tm_hour, check_time.tm_min, check_time.tm_sec,
        g_procname, g_procidx); 

    vfprintf (user_def_log, fmt, ap );
    vsprintf (logmsg, fmt, ap );
    fprintf( user_def_log,"\n");

    fflush( user_def_log );

    log_debug( logmsg );

    va_end( ap );
#endif

    return 1;
}

int log_debug( char *fmt, ... )
{
#ifdef LOGDEBUG
    char        dir_path[80], mesg_path[80];
    struct stat stat_log;
    time_t      nowtime;
    struct tm  	check_time;
    FILE        *fptr;
    char        debug1[80];
    char        debug2[80];
    char        debug3[80];
    char        debug4[80];
    va_list     ap;
	DIR			*dirp;

	if( g_MPLogMode == 1 )
    	sprintf(mesg_path, "%s/DEBUG/%s_debuglog.0", g_logfilepath, g_MPName);
	else
    	sprintf(mesg_path, "%s/DEBUG/debuglog.0", g_logfilepath);

	if(stat(mesg_path, &stat_log) == 0)
	{
    	if( (stat_log.st_size) > LOG_FILE_SIZE )
        {
            sprintf(debug1, "%s/DEBUG/debuglog.1", g_logfilepath );
            sprintf(debug2, "%s/DEBUG/debuglog.2", g_logfilepath );
            sprintf(debug3, "%s/DEBUG/debuglog.3", g_logfilepath );
            sprintf(debug4, "%s/DEBUG/debuglog.4", g_logfilepath );
            log_close();
            remove(debug4);
            rename(debug3, debug4);
            remove(debug3);
            rename(debug2, debug3);
            remove(debug2);
            rename(debug1, debug2);
            remove(debug1);
            rename(mesg_path, debug1);
            unlink(mesg_path);
        }
    }
    else
    {

        dirp =  opendir( g_logfilepath ) ;

        if( dirp == (DIR*)NULL )
            mkdir( g_logfilepath, 0777 );
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

    time(&nowtime);
    localtime_r(&nowtime, &check_time);

    fprintf (user_def_deb_log, "[%02d/%02d %02d:%02d:%02d][%s:%u] ",
        check_time.tm_mon+1, check_time.tm_mday,
        check_time.tm_hour, check_time.tm_min, check_time.tm_sec,
        g_procname, g_procidx);

    va_start( ap, fmt );
    vfprintf (user_def_deb_log, fmt, ap );
    fprintf( user_def_deb_log, "\n");
    fflush( user_def_deb_log );

    va_end( ap );
#endif
    return 1;
}

int log_close()
{

	if( user_def_log != NULL )
    	fclose( user_def_log );

	if( user_def_deb_log != NULL )
    	fclose( user_def_deb_log );

	if( user_def_beacon != NULL )
		fclose( user_def_beacon );

	user_def_beacon = NULL;
    user_def_log = NULL;
    user_def_deb_log = NULL;

    return 1;
}



int log_hexa( unsigned char *szLog, int dSize )
{
#ifdef LOGDEBUG
    char        dir_path[80], mesg_path[80];
    struct stat     stat_log;
    time_t      nowtime;
    struct  tm  check_time;
    FILE        *fptr;
    int     	k;
    char        debug1[80];
    char        debug2[80];
    char        debug3[80];
    char        debug4[80];
	DIR			*dirp;

    int i=0;

    sprintf(mesg_path, "%s/DEBUG/debuglog.0", g_logfilepath);

    if(stat(mesg_path, &stat_log) == 0)
    {
        if( (stat_log.st_size) > LOG_FILE_SIZE )
        {
            sprintf(debug1, "%s/DEBUG/debuglog.1", g_logfilepath );
            sprintf(debug2, "%s/DEBUG/debuglog.2", g_logfilepath );
            sprintf(debug3, "%s/DEBUG/debuglog.3", g_logfilepath );
            sprintf(debug4, "%s/DEBUG/debuglog.4", g_logfilepath );

			if( user_def_deb_log != NULL )
    			fclose( user_def_deb_log );
			user_def_deb_log = NULL;

            remove(debug4);
            rename(debug3, debug4);
            remove(debug3);
            rename(debug2, debug3);
            remove(debug2);
            rename(debug1, debug2);
            remove(debug1);
            rename(mesg_path, debug1);
            unlink(mesg_path);
        }
    }
    else
    {
		if( user_def_deb_log != NULL )
    		fclose( user_def_deb_log );
		user_def_deb_log = NULL;

        dirp =  opendir( g_logfilepath ) ;

        if( dirp == (DIR*)NULL )
        {
            mkdir( g_logfilepath, 0777 );
        }
        else
        {
            closedir(dirp);
        }

        sprintf(dir_path, "%s/DEBUG", g_logfilepath);
        mkdir(dir_path, 0777);
    }

    if( user_def_deb_log == NULL )
    {
        if ((fptr = fopen(mesg_path, "a+")) == NULL)
            return -1;

        user_def_deb_log = fptr;
    }

    time(&nowtime);
    localtime_r(&nowtime, &check_time);

    fprintf (user_def_deb_log, "[%02d/%02d %02d:%02d:%02d][%s:%u] ",
        check_time.tm_mon+1, check_time.tm_mday,
        check_time.tm_hour, check_time.tm_min, check_time.tm_sec,
        g_procname, g_procidx);

    fprintf( user_def_deb_log, "\n ##### PRINT HEXA CODE START ##### ");

    fflush( user_def_deb_log );
	k = 1;

    for( i=0; i<dSize ; i++ )
    {
        if( i%20 == 0 )
        {
            fprintf( user_def_deb_log, "\n");
			fprintf( user_def_deb_log, " %04d : ", i );
            fflush( user_def_deb_log );
        }

		if( i%((4*k)-1) == 0 && i != 0  )
		{
			k++;
        	fprintf( user_def_deb_log, " %02x|", szLog[i]);
		}
		else
		{
        	fprintf( user_def_deb_log, " %02x", szLog[i]);
		}

    }

    fprintf( user_def_deb_log, "\n ##### PRINT HEXA CODE END   ##### ");
    fflush( user_def_deb_log );
    fprintf( user_def_deb_log, "\n");
    fflush(user_def_deb_log);
#endif

    return 1;
}

int log_hexa2( unsigned char *szLog, int dSize )
{
#ifdef LOGDEBUG
    char        dir_path[80], mesg_path[80];
    struct stat     stat_log;
    time_t      nowtime;
    struct  tm  check_time;
    FILE        *fptr;
    int     	k;
    char        debug1[80];
    char        debug2[80];
    char        debug3[80];
    char        debug4[80];
	DIR			*dirp;
    int i=0;

    sprintf(mesg_path, "%s/DEBUG/debuglog.0", g_logfilepath);

    if(stat(mesg_path, &stat_log) == 0)
    {
        if( (stat_log.st_size) > LOG_FILE_SIZE )
        {
            sprintf(debug1, "%s/DEBUG/debuglog.1", g_logfilepath );
            sprintf(debug2, "%s/DEBUG/debuglog.2", g_logfilepath );
            sprintf(debug3, "%s/DEBUG/debuglog.3", g_logfilepath );
            sprintf(debug4, "%s/DEBUG/debuglog.4", g_logfilepath );

			if( user_def_deb_log != NULL )
    			fclose( user_def_deb_log );
			user_def_deb_log = NULL;

            remove(debug4);
            rename(debug3, debug4);
            remove(debug3);
            rename(debug2, debug3);
            remove(debug2);
            rename(debug1, debug2);
            remove(debug1);
            rename(mesg_path, debug1);
            unlink(mesg_path);
        }
    }
    else
    {
		if( user_def_deb_log != NULL )
    		fclose( user_def_deb_log );
		user_def_deb_log = NULL;

        dirp =  opendir( g_logfilepath ) ;

        if( dirp == (DIR*)NULL )
            mkdir( g_logfilepath, 0777 );
        else
            closedir(dirp);

        sprintf(dir_path, "%s/DEBUG", g_logfilepath);
        mkdir(dir_path, 0777);
    }

    if( user_def_deb_log == NULL )
    {
        if ((fptr = fopen(mesg_path, "a+")) == NULL)
            return -1;

        user_def_deb_log = fptr;
    }

    time(&nowtime);
    localtime_r(&nowtime, &check_time);

    fprintf (user_def_deb_log, "[%02d/%02d %02d:%02d:%02d][%s:%u] ",
        check_time.tm_mon+1, check_time.tm_mday,
        check_time.tm_hour, check_time.tm_min, check_time.tm_sec,
        g_procname, g_procidx);

	k = 1;
    for( i=0; i<dSize ; i++ )
    {
        if( i%16 == 0 )
        {
            fprintf( user_def_deb_log, "\n");
			fprintf( user_def_deb_log, " %04d : ", i );
            fflush( user_def_deb_log );
        }

		if( i%((4*k)-1) == 0 && i != 0  )
		{
			k++;
        	fprintf( user_def_deb_log, " %02x|", szLog[i]);
		}
		else
		{
        	fprintf( user_def_deb_log, " %02x", szLog[i]);
		}

    }

    fprintf( user_def_deb_log, "\n");
    fflush(user_def_deb_log);
#endif

 
	return 1;
}


int log_hexa3( unsigned char *szLog, int dSize )
{

#ifdef LOGDEBUG
    char        	dir_path[80], mesg_path[80];
	struct stat     stat_log;
	time_t      	nowtime;
	struct  tm  	check_time;
	struct  tm      log_time;
	FILE        	*fptr;
	int     		k;
	DIR				*dirp;
	int 			i=0;

	time(&nowtime);
	localtime_r(&nowtime, &check_time);

	sprintf(mesg_path, "%s/%02d%02d/%02d",
		g_logfilepath, check_time.tm_mon + 1, check_time.tm_mday,
		check_time.tm_hour);

	if (stat(mesg_path, &stat_log) == 0)
	{
		localtime_r(&stat_log.st_atime, &log_time);
		if (check_time.tm_year != log_time.tm_year)
		{
			log_close();	
			user_def_deb_log2 = NULL;
			unlink(mesg_path);
		}
	}
	else
	{
		log_close();	

		dirp =  opendir( g_logfilepath ) ;
		if( dirp == (DIR*)NULL )
			mkdir( g_logfilepath, 0777 );
		else
			closedir(dirp);

		sprintf(dir_path, "%s/%02d%02d", 
			g_logfilepath, check_time.tm_mon+1, check_time.tm_mday);
		mkdir(dir_path, 0777);
		user_def_deb_log2 = NULL;
	}

	if( user_def_deb_log2 == NULL )
    {
		if ((fptr = fopen(mesg_path, "a+")) == NULL)
			return -1;
		user_def_deb_log2 = fptr;
	}

	fprintf (user_def_deb_log2, "[%02d/%02d %02d:%02d:%02d][%s:%u] ",
		check_time.tm_mon+1, check_time.tm_mday,
		check_time.tm_hour, check_time.tm_min, check_time.tm_sec,
		g_procname, g_procidx);

	k = 1;
	for( i=0; i<dSize ; i++ )
	    {
		if( i%16 == 0 )
			{
			fprintf( user_def_deb_log2, "\n");
			fprintf( user_def_deb_log2, " %04d : ", i );
			fflush( user_def_deb_log2 );
			}

		if( i%((4*k)-1) == 0 && i != 0  )
			{
			k++;
			fprintf( user_def_deb_log2, " %02x|", szLog[i]);
			}
		else
			{
			fprintf( user_def_deb_log2, " %02x", szLog[i]);
			}

		}

	fprintf( user_def_deb_log2, "\n");
	fflush(user_def_deb_log2);
#endif

	return 1;
	}

int log_beacon( unsigned char *puBuf, int size, int iFlag, int iRetCode, int iLogLevel )
{
	int	i;
    char        	dir_path[80], mesg_path[80], file_path[80], szFile1[80], szFile2[80];
	struct stat     stat_log;
	time_t      	curtime;
	int				ucurtime;
	struct tm  		check_time;
	struct timeval	tval;

	FILE        	*fptr;
	DIR				*dirp;

	st_BeaconHdr	stHdr;

	if( gettimeofday(&tval, NULL) == -1 )
	{
		curtime = 0;
		ucurtime = 0;
	}
	else
	{
		curtime = tval.tv_sec;
		ucurtime = tval.tv_usec;
	}
	
	localtime_r( &curtime, &check_time );

	switch( iLogLevel )
	{
		case 1:
			sprintf(file_path, "%s/DEBUG/PKTLOG", g_beaconfilepath );
			break;
		case 2:
			sprintf(file_path, "%s/DEBUG/MSGLOG", g_beaconfilepath );
			break;
		default:
			return -1;
	}
	sprintf(mesg_path, "%s.0", file_path );

	if (stat(mesg_path, &stat_log) == 0)
	{
		if( stat_log.st_size > BEACON_FILE_SIZE )
		{
			if( user_def_beacon != NULL )
   				fclose( user_def_beacon );
			user_def_beacon = NULL;
			
			for( i=MAX_DEBUG_FILE-1; i > 0; i-- )
			{
				sprintf( szFile2, "%s.%d", file_path, i );
				sprintf( szFile1, "%s.%d", file_path, i-1 );

				remove( szFile2 );
				rename( szFile1, szFile2 );
			}
			unlink( mesg_path );	
		
			if( (fptr = fopen(mesg_path, "a+")) == NULL )
				return -1;
			user_def_beacon = fptr;
	
			if( fwrite( (char *)FVERSION, 24, 1, user_def_beacon ) != 1 )
				return -1;
		}
	}
	else
   	{
		if( user_def_beacon != NULL )
   			fclose( user_def_beacon );
		user_def_beacon = NULL;

		sprintf(dir_path, "%s/DEBUG", g_beaconfilepath );

		dirp =  opendir( dir_path ) ;
		if( dirp == (DIR*)NULL )
			mkdir( dir_path, 0777 );
		else
			closedir(dirp);

		if( (fptr = fopen(mesg_path, "a+")) == NULL )
			return -1;
		user_def_beacon = fptr;

		if( fwrite( (char *)FVERSION, 24, 1, user_def_beacon ) != 1 )
			return -1;
	}

	if( user_def_beacon == NULL )
    {
		if ((fptr = fopen(mesg_path, "a+")) == NULL)
			return -1;
		user_def_beacon = fptr;
	}

	stHdr.Info.cHeaderLen = 16;
	stHdr.Info.cFlag = (char)iLogLevel;
	stHdr.Info.sRetCode = htons((short)iRetCode);
	stHdr.curtime = htonl(curtime);
	stHdr.ucurtime = htonl(ucurtime);
	stHdr.lSize = htonl(size);

	// packet header write	
	if( fwrite( &stHdr, (int)stHdr.Info.cHeaderLen, 1, user_def_beacon) != 1 )
	{
		return -2;
	}

	// packet body write
	if( fwrite( puBuf, size, 1, user_def_beacon ) != 1 )
	{
		return -2;
	}

	fflush( user_def_beacon );

	return 0;
}

int log_message( unsigned char *puBuf, int size, int iFlag, int iRetCode, int iLogLevel )
{
    char        	dir_path[80], mesg_path[80];
	struct stat     stat_log;
	time_t      	curtime;
	int				ucurtime;
	struct  tm  	check_time, log_time;
	struct	timeval	tval;

	FILE        	*fptr;
	DIR				*dirp;

	st_BeaconHdr	stHdr;

	if( gettimeofday(&tval, NULL) == -1 )
	{
		curtime = 0;
		ucurtime = 0;
	}
	else
	{
		curtime = tval.tv_sec;
		ucurtime = tval.tv_usec;
	}

	localtime_r( &g_prevtime, &log_time );	
	localtime_r( &curtime, &check_time );
	
	if( log_time.tm_hour != check_time.tm_hour )
		g_iExt = 0;
	g_prevtime = curtime;

	while (1)
	{
		sprintf(mesg_path, "%s/%02d/%02d/MSG%02d%02d_%02d.%d",
			g_beaconfilepath, check_time.tm_mon + 1, check_time.tm_mday, 
			check_time.tm_mon+1, check_time.tm_mday, check_time.tm_hour, g_iExt);

		if( stat(mesg_path, &stat_log) == 0 )
		{
			localtime_r( &stat_log.st_atime, &log_time ); 
					
			if( check_time.tm_year != log_time.tm_year )
			{
				if( fpError != NULL )
    				fclose( fpError );
				fpError = NULL;
				
				unlink( mesg_path );
				continue;
			}
			else if( stat_log.st_size > BEACON_FILE_SIZE )
			{
				if( fpError != NULL )
    				fclose( fpError );
				fpError = NULL;
				
				g_iExt++;
				continue;
			}
		}
		else
    	{
			if( fpError != NULL )
    			fclose( fpError );
			fpError = NULL;

			sprintf(dir_path, "%s/%02d", 
				g_beaconfilepath, check_time.tm_mon+1);

			dirp =  opendir( dir_path ) ;
			if( dirp == (DIR*)NULL )
				mkdir( dir_path, 0777 );
			else
				closedir(dirp);

			sprintf(dir_path, "%s/%02d/%02d", g_beaconfilepath, check_time.tm_mon+1, check_time.tm_mday);

			dirp = opendir( dir_path );
			if( dirp == (DIR *)NULL )
			{
				mkdir(dir_path, 0777);
				g_iExt = 0;
		
				sprintf(mesg_path, "%s/%02d/%02d/MSG%02d%02d_%02d.%d",
					g_beaconfilepath, check_time.tm_mon + 1, check_time.tm_mday, 
					check_time.tm_mon+1, check_time.tm_mday, check_time.tm_hour, g_iExt);

				//continue;
			}
			else
				closedir( dirp );

			if( (fptr = fopen(mesg_path, "a+")) == NULL )
				return -1;
			fpError = fptr;

			if( fwrite( (char *)FVERSION, 24, 1, fpError ) != 1 )
				return -1;
		}

		break;
	}

	if( fpError == NULL )
    {
		if ((fptr = fopen(mesg_path, "a+")) == NULL)
			return -1;
		fpError = fptr;
	}

	stHdr.Info.cHeaderLen = 16;
	stHdr.Info.cFlag = (char)iLogLevel;
	stHdr.Info.sRetCode = htons((short)iRetCode);
	stHdr.curtime = htonl(curtime);
	stHdr.ucurtime = htonl(ucurtime);
	stHdr.lSize = htonl(size);

	// packet header write	
	if( fwrite( &stHdr, (int)stHdr.Info.cHeaderLen, 1, fpError) != 1 )
	{
		return -2;
	}

	// packet body write
	if( fwrite( puBuf, size, 1, fpError ) != 1 )
	{
		return -2;
	}

	fflush( fpError );

	return 0;
}

int log_packet( unsigned char *puBuf, int size, int iFlag, int iRetCode, int iLogLevel )
{
	char        	dir_path[80], mesg_path[80];
	struct stat     stat_log;
	time_t      	curtime;
	int				ucurtime;
	struct  tm  	check_time, log_time;
	struct	timeval	tval;

	FILE        	*fptr;
	DIR				*dirp;

	st_BeaconHdr	stHdr;

	if( gettimeofday(&tval, NULL) == -1 )
	{
		curtime = 0;
		ucurtime = 0;
	}
	else
	{
		curtime = tval.tv_sec;
		ucurtime = tval.tv_usec;
	}

	localtime_r( &g_prevtime, &log_time );	
	localtime_r( &curtime, &check_time );

	if( log_time.tm_hour != check_time.tm_hour )
		g_iExt = 0;
	g_prevtime = curtime;

	while (1)
	{
		sprintf(mesg_path, "%s/%02d/%02d/%c_PKT%02d%02d_%02d.%d",
				g_beaconfilepath, check_time.tm_mon + 1, check_time.tm_mday, 
				iFlag, check_time.tm_mon+1, check_time.tm_mday, check_time.tm_hour, g_iExt);

		if( stat(mesg_path, &stat_log) == 0 )
		{
			localtime_r( &stat_log.st_atime, &log_time ); 

			if( check_time.tm_year != log_time.tm_year )
			{
				if( fpError != NULL )
					fclose( fpError );
				fpError = NULL;

				unlink( mesg_path );
				continue;
			}
			else if( stat_log.st_size > BEACON_FILE_SIZE )
			{
				if( fpError != NULL )
					fclose( fpError );
				fpError = NULL;

				g_iExt++;
				continue;
			}
		}
		else
		{
			if( fpError != NULL )
				fclose( fpError );
			fpError = NULL;

			sprintf(dir_path, "%s/%02d", 
					g_beaconfilepath, check_time.tm_mon+1);

			dirp =  opendir( dir_path ) ;
			if( dirp == (DIR*)NULL )
				mkdir( dir_path, 0777 );
			else
				closedir(dirp);

			sprintf(dir_path, "%s/%02d/%02d", 
					g_beaconfilepath, check_time.tm_mon+1, check_time.tm_mday);

			dirp = opendir( dir_path );
			if( dirp == (DIR *)NULL )
			{
				mkdir(dir_path, 0777);
				g_iExt = 0;

				sprintf(mesg_path, "%s/%02d/%02d/%c_PKT%02d%02d_%02d.%d",
						g_beaconfilepath, check_time.tm_mon + 1, check_time.tm_mday, 
						iFlag, check_time.tm_mon+1, check_time.tm_mday, check_time.tm_hour, g_iExt);

				//continue;
			}
			else
				closedir( dirp );

			if( (fptr = fopen(mesg_path, "a+")) == NULL )
				return -1;
			fpError = fptr;

			if( fwrite( (char *)FVERSION, 24, 1, fpError ) != 1 )
				return -1;
		}

		break;
	}

	if( fpError == NULL )
	{
		if ((fptr = fopen(mesg_path, "a+")) == NULL)
			return -1;
		fpError = fptr;
	}

	stHdr.Info.cHeaderLen = 16;
	stHdr.Info.cFlag = (char)iLogLevel;
	stHdr.Info.sRetCode = htons((short)iRetCode);
	stHdr.curtime = htonl(curtime);
	stHdr.ucurtime = htonl(ucurtime);
	stHdr.lSize = htonl(size);

	// packet header write	
	if( fwrite( &stHdr, (int)stHdr.Info.cHeaderLen, 1, fpError) != 1 )
	{
		return -2;
	}

	// packet body write
	if( fwrite( puBuf, size, 1, fpError ) != 1 )
	{
		return -2;
	}

	fflush( fpError );

	fclose( fpError );
	fpError = NULL;

	return 0;
}

int dAppWrite(int dLevel, char *szMsg)
{
	char        dir_path[80], mesg_path[80], szLevel[80];
	struct stat stat_log;
	time_t      nowtime;
	struct  tm  check_time, log_time;
	FILE        *fptr;
	DIR         *dirp;

	time(&nowtime);
	localtime_r(&nowtime, &check_time);

	while(1)
	{
		if( g_MPLogMode == 1 ) {
			sprintf(mesg_path, "%s/%02d%02d/%s_%02d_%d",
					g_logfilepath, check_time.tm_mon + 1, check_time.tm_mday, 
					g_MPName, check_time.tm_hour, g_logno);
		}
		else {
			sprintf(mesg_path, "%s/%02d%02d/%02d_%d",
					g_logfilepath, check_time.tm_mon + 1, check_time.tm_mday, 
					check_time.tm_hour, g_logno);
		}

		if (stat(mesg_path, &stat_log) == 0)
		{
			localtime_r(&stat_log.st_atime, &log_time);
			if (check_time.tm_year != log_time.tm_year)
			{
				if( user_def_log != NULL )
					fclose( user_def_log );
				user_def_log = NULL;
				unlink(mesg_path);
			}
			else if( stat_log.st_size > LOG_FILE_SIZE )
			{
				if( user_def_log != NULL )
					fclose( user_def_log );
				user_def_log = NULL;

				g_logno++;

				continue;
			}
		}
		else
		{
			if( user_def_log != NULL )
			{
				fclose( user_def_log );
				user_def_log = NULL;

				if( g_logno > 0 )
				{
					g_logno = 0;	// new hour file
					continue;
				}
			}

			dirp = opendir( g_logfilepath );
			if( dirp == (DIR*)NULL )
				mkdir( g_logfilepath, 0777 );
			else
				closedir( dirp );

			sprintf(dir_path, "%s/%02d%02d", 
					g_logfilepath, check_time.tm_mon+1, check_time.tm_mday );
			dirp = opendir( dir_path );
			if( dirp == (DIR*)NULL )
				mkdir(dir_path, 0777);
			else
				closedir( dirp );
		}

		break;
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

	fprintf (user_def_log, "[%02d/%02d %02d:%02d:%02d][%s:%d] [%s]\n",
			check_time.tm_mon+1, check_time.tm_mday,
			check_time.tm_hour, check_time.tm_min, check_time.tm_sec,
			szLevel, (int)g_procidx, szMsg);

	fflush( user_def_log );
	/* MODIFY: BY JUNE 
	 * DESCIP: CRI LOG 는 DEBUG DIRECTORY 에 저장 안함.(성능 이슈)
	 */
	//dAppDebug(LOG_TYPE_WRITE, dLevel, szMsg, &check_time, szLevel);

	return 1;
}

int dDebugFileCheck(int strsize)
{
	FILE        *fptr;
	char        debug1[80];
	char        debug2[80];
	char        debug3[80];
	char        debug4[80];
	char        dir_path[80], mesg_path[80];
	struct stat stat_log;
	DIR         *dirp;

	if( g_MPLogMode == 1 )
		sprintf(mesg_path, "%s/DEBUG/%s_debuglog.0", g_logfilepath, g_MPName);
	else
		sprintf(mesg_path, "%s/DEBUG/debuglog.0", g_logfilepath);
	if(stat(mesg_path, &stat_log) == 0)
	{
		if( (stat_log.st_size + strsize ) > LOG_FILE_SIZE )
		{
			if( g_MPLogMode == 1 ) {
				sprintf(debug1, "%s/DEBUG/%s_debuglog.1", g_logfilepath, g_MPName);
				sprintf(debug2, "%s/DEBUG/%s_debuglog.2", g_logfilepath, g_MPName);
				sprintf(debug3, "%s/DEBUG/%s_debuglog.3", g_logfilepath, g_MPName);
				sprintf(debug4, "%s/DEBUG/%s_debuglog.4", g_logfilepath, g_MPName);
			}
			else {
				sprintf(debug1, "%s/DEBUG/debuglog.1", g_logfilepath );
				sprintf(debug2, "%s/DEBUG/debuglog.2", g_logfilepath );
				sprintf(debug3, "%s/DEBUG/debuglog.3", g_logfilepath );
				sprintf(debug4, "%s/DEBUG/debuglog.4", g_logfilepath );
			}

			log_close();

			remove(debug4);
			rename(debug3, debug4);
			remove(debug3);
			rename(debug2, debug3);
			remove(debug2);
			rename(debug1, debug2);
			remove(debug1);
			rename(mesg_path, debug1);
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

int dAppDebug(int dType, int dLevel,  char *szMsg, struct tm *pstCheckTime, char *szName)
{
	int             dLen;
	char            szLevel[80];
	struct  tm      check_time;
	struct  timeval stTime;

	dLen = strlen(szMsg);

	if(dDebugFileCheck(dLen) < 0)
		return -1;

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

	if(dType == LOG_TYPE_DEBUG)
	{
		gettimeofday(&stTime, NULL);
		localtime_r(&stTime.tv_sec, &check_time);

		fprintf (user_def_deb_log, "[%02d/%02d %02d:%02d:%02d.%06ld][%s:%d] [%s]\n",
				check_time.tm_mon+1, check_time.tm_mday,
				check_time.tm_hour, check_time.tm_min, check_time.tm_sec,
				stTime.tv_usec, szLevel, (int)g_procidx, szMsg);
	}
	else
	{
		fprintf (user_def_deb_log, "[%02d/%02d %02d:%02d:%02d][%s:%d] [%s]\n",
				pstCheckTime->tm_mon+1, pstCheckTime->tm_mday,
				pstCheckTime->tm_hour, pstCheckTime->tm_min, pstCheckTime->tm_sec,
				szName, (int)g_procidx, szMsg);
	}

	fflush( user_def_deb_log );

	return 0;
}

int dAppLog(int dIndex, char *fmt, ...)
{
	char            szMsg[30720];
	va_list         args;
	int 			dRet;

	if( gMyLogLvl == 0x00 )
		return 0;
 
	switch(dIndex)
	{
		case LOG_INFO :
		case LOG_DEBUG :
		case LOG_WARN :
			if(dIndex > *gMyLogLvl)
				return 0;

			va_start(args, fmt);
			vsprintf(szMsg, fmt, args);
			va_end(args);

			dAppDebug(LOG_TYPE_DEBUG, dIndex, szMsg, NULL, NULL);

			break;
		case LOG_CRI :
			if(dIndex > *gMyLogLvl)
				return 0;

			va_start(args, fmt);
			vsprintf(szMsg, fmt, args);
			va_end(args);

			dRet = dAppWrite(dIndex, szMsg);
			break;
		case LOG_NOPRINT :
		default :
			return -1;
	}

	return 0;
}

#if 0
int dMPAppLog(int dIndex, char *fmt, ...)
{       
	char            szMsg[30720];
	va_list         args;
	int             dRet;

	if( gMyLogLvl == 0x00 )
		return 0;

	switch(dIndex)
	{   
		case LOG_INFO :
		case LOG_DEBUG :
		case LOG_WARN :
			if(dIndex > *gMyLogLvl)
				return 0;

			va_start(args, fmt);
			vsprintf(szMsg, fmt, args);
			va_end(args);

			dAppDebug(LOG_TYPE_DEBUG, dIndex, szMsg, NULL, NULL);

			break;
		case LOG_CRI :
			if(dIndex > *gMyLogLvl)
				return 0;

			va_start(args, fmt);
			vsprintf(szMsg, fmt, args);
			va_end(args);

			dRet = dAppWrite(dIndex, szMsg);
			break;
		case LOG_NOPRINT :
		default :
			return -1;
	}

	return 0;
}
#endif

