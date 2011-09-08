#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <arpa/inet.h>

#include <loglib.h>

FILE		*g_fdUserDefLog;
FILE		*g_fdUserDefDegLog;

int			g_nProcIdx;
char		g_szLogFilePath[1024];
pid_t		g_nPid;
char		g_szProcName[64];
st_LogLevel	*g_stLogLevel;
int			g_nLogLevel;

FILE		*g_fdUserDefBcon;
char		g_szBconFilePath[1024];

//int Init_logbeacon( char *szLogFilePath )
int bcon_init( char *szLogFilePath )
{
	struct stat stStatInfo;
	DIR		 *stDirInfo;

	strcpy( g_szBconFilePath, szLogFilePath);
	g_szBconFilePath[1023] = '\0';

	if( stat(g_szBconFilePath, &stStatInfo) != 0 ) {
		stDirInfo =  opendir( g_szBconFilePath ) ;

		if( stDirInfo == (DIR*)NULL )
			mkdir( g_szBconFilePath, 0777 );
		else
			closedir(stDirInfo);
	}

	return 0;
}

//int log_beacon( unsigned char *szBuf, int size, int nFlag, int nRetCode, int nLogLevel )
int bcon_write( unsigned char *szBuf, int size, int nFlag, int nRetCode, int nLogLevel )
{
	int				i;
	char			szDirPath[80], szMesgPath[80], szFilePath[80], szFile1[80], szFile2[80];
	struct stat		stStatInfo;
	time_t			tCurTime;
	int				nCurMTime;
	struct tm  		tmChkTime;
	struct timeval	tvTime;

	FILE			*fdMesgFile;
	DIR				*stDirInfo;

	st_BeaconHdr	stBconHdr;

	if( gettimeofday(&tvTime, NULL) == -1 )
	{
		tCurTime = 0;
		nCurMTime = 0;
	}
	else
	{
		tCurTime = tvTime.tv_sec;
		nCurMTime = tvTime.tv_usec;
	}
	
	localtime_r( &tCurTime, &tmChkTime );

	switch( nLogLevel )
	{
		case 1:
			sprintf(szFilePath, "%s/DEBUG/PKTLOG", g_szBconFilePath );
			break;
		case 2:
			sprintf(szFilePath, "%s/DEBUG/MSGLOG", g_szBconFilePath );
			break;
		default:
			return -1;
	}
	
	sprintf(szMesgPath, "%s.0", szFilePath );

	if (stat(szMesgPath, &stStatInfo) == 0)
	{
		if( stStatInfo.st_size > BEACON_FILE_SIZE )
		{
			if( g_fdUserDefBcon != NULL )
   				fclose( g_fdUserDefBcon );
			g_fdUserDefBcon = NULL;
			
			for( i=MAX_DEBUG_FILE-1; i > 0; i-- )
			{
				sprintf( szFile2, "%s.%d", szFilePath, i );
				sprintf( szFile1, "%s.%d", szFilePath, i-1 );

				remove( szFile2 );
				rename( szFile1, szFile2 );
			}
			unlink( szMesgPath );	
		
			if( (fdMesgFile = fopen(szMesgPath, "a+")) == NULL )
				return -1;
			g_fdUserDefBcon = fdMesgFile;
	
			if( fwrite( (char *)FVERSION, 24, 1, g_fdUserDefBcon ) != 1 )
				return -1;
		}
	}
	else
	{
		if( g_fdUserDefBcon != NULL )
   			fclose( g_fdUserDefBcon );
		g_fdUserDefBcon = NULL;

		sprintf(szDirPath, "%s/DEBUG", g_szBconFilePath );

		stDirInfo =  opendir( szDirPath ) ;
		if( stDirInfo == (DIR*)NULL )
			mkdir( szDirPath, 0777 );
		else
			closedir(stDirInfo);

		if( (fdMesgFile = fopen(szMesgPath, "a+")) == NULL )
			return -1;
		g_fdUserDefBcon = fdMesgFile;

		if( fwrite( (char *)FVERSION, 24, 1, g_fdUserDefBcon ) != 1 )
			return -1;
	}

	if( g_fdUserDefBcon == NULL )
	{
		if ((fdMesgFile = fopen(szMesgPath, "a+")) == NULL)
			return -1;
		g_fdUserDefBcon = fdMesgFile;
	}

	stBconHdr.Info.cHeaderLen = 16;
	stBconHdr.Info.cFlag = (char)nLogLevel;
	stBconHdr.Info.sRetCode = htons((short)nRetCode);
	stBconHdr.tCurTime = htonl(tCurTime);
	stBconHdr.nCurMTime = htonl(nCurMTime);
	stBconHdr.nSize = htonl(size);

	// packet header write	
	if( fwrite( &stBconHdr, (int)stBconHdr.Info.cHeaderLen, 1, g_fdUserDefBcon) != 1 )
	{
		return -2;
	}

	// packet body write
	if( fwrite( szBuf, size, 1, g_fdUserDefBcon ) != 1 )
	{
		return -2;
	}

	fflush( g_fdUserDefBcon );

	return 0;
}

int log_close()
{
	if(g_fdUserDefLog != NULL)
		fclose(g_fdUserDefLog);

	if(g_fdUserDefDegLog != NULL)
		fclose(g_fdUserDefDegLog);

	g_fdUserDefLog = NULL;
	g_fdUserDefDegLog = NULL;

	return 1;
}

int check_debugfile(int nStrSize)
{
	int			i;
	FILE		*fdMesgFile;
	char		szDebugPath[DEF_DEBUG_NUM][80];
	char		szDirPath[80], szMesgPath[80];
	struct stat stStatInfo;
	DIR			*stDirInfo;

	sprintf(szMesgPath, "%s/DEBUG/debuglog.0", g_szLogFilePath);
	if(stat(szMesgPath, &stStatInfo) == 0)
	{
		if( (stStatInfo.st_size + nStrSize ) > LOG_FILE_SIZE )
		{
		   	log_close();

			for(i = 0; i < DEF_DEBUG_NUM; i++)
				sprintf(szDebugPath[i], "%s/DEBUG/debuglog.%d",
				g_szLogFilePath, i);

			for(i = DEF_DEBUG_NUM - 1; i > 1; i--)
			{
				remove(szDebugPath[i]);
				rename(szDebugPath[i-1], szDebugPath[i]);
			}
			remove(szDebugPath[i]);
			rename(szMesgPath, szDebugPath[i]);

			unlink(szMesgPath);
		}
	}
	else
	{
		stDirInfo = opendir(g_szLogFilePath);

		if( stDirInfo == (DIR*)NULL )
			mkdir(g_szLogFilePath, 0777 );
		else
			closedir(stDirInfo);

		sprintf(szDirPath, "%s/DEBUG", g_szLogFilePath);
		mkdir(szDirPath, 0777);

		g_fdUserDefDegLog = NULL;
	}

	if( g_fdUserDefDegLog == NULL )
	{
		if ((fdMesgFile = fopen(szMesgPath, "a+")) == NULL)
			return -1;
		g_fdUserDefDegLog = fdMesgFile;
	}

	return 0;
}

int log_debug(char *fmt, ...)
{
	time_t		tNowTime;
	struct tm	tmChkTime;
	int			nStrSize;
	char		szMsg[30720];
	va_list		ap;

	va_start( ap, fmt );
	vsprintf( szMsg, fmt, ap );

	nStrSize = strlen( szMsg );
	if(check_debugfile(nStrSize) < 0)
		return -1;

	time(&tNowTime);
	localtime_r(&tNowTime, &tmChkTime);

	fprintf (g_fdUserDefDegLog, "[%02d/%02d %02d:%02d:%02d] ",
		tmChkTime.tm_mon+1, tmChkTime.tm_mday,
		tmChkTime.tm_hour, tmChkTime.tm_min, tmChkTime.tm_sec );

	vfprintf (g_fdUserDefDegLog, fmt, ap );
	fprintf( g_fdUserDefDegLog, "\n");
	fflush( g_fdUserDefDegLog );

	va_end( ap );

	return 1;
}

int log_write(char *fmt, ...)
{
	char		szDirPath[80], szMesgPath[80];
	struct stat stStatInfo;
	time_t  	tNowTime;
	struct  tm  tmChkTime, tmLogTime;
	FILE		*fdMesgFile;
	char		deb_tmp[30720];
	va_list 	ap;
	DIR			*stDirInfo;

	time(&tNowTime);
	localtime_r(&tNowTime, &tmChkTime);

	sprintf(szMesgPath, "%s/%02d%02d/%02d",
		g_szLogFilePath, tmChkTime.tm_mon + 1, tmChkTime.tm_mday, tmChkTime.tm_hour);

	if (stat(szMesgPath, &stStatInfo) == 0)
	{
		localtime_r(&stStatInfo.st_atime, &tmLogTime);
		if (tmChkTime.tm_year != tmLogTime.tm_year)
		{
			unlink (szMesgPath);
			log_close();

			g_fdUserDefLog = NULL;
		}
	}
	else
	{
		log_close();
		stDirInfo = opendir( g_szLogFilePath );
		if( stDirInfo == (DIR*)NULL )
			mkdir( g_szLogFilePath, 0777 );
		else
			closedir( stDirInfo );

		sprintf(szDirPath, "%s/%02d%02d",
			g_szLogFilePath, tmChkTime.tm_mon+1, tmChkTime.tm_mday);
		mkdir(szDirPath, 0777);
		g_fdUserDefLog = NULL;
	}

	if( g_fdUserDefLog == NULL )
	{
		if ((fdMesgFile = fopen(szMesgPath, "a+")) == NULL)
			return -1;
		g_fdUserDefLog = fdMesgFile;
	}

	va_start( ap, fmt );
	fprintf (g_fdUserDefLog, "[%d][%02d:%02d:%02d] ",
		g_nPid, tmChkTime.tm_hour, tmChkTime.tm_min, tmChkTime.tm_sec );

	vfprintf (g_fdUserDefLog, fmt, ap );
	vsprintf (deb_tmp, fmt, ap );
	fprintf( g_fdUserDefLog,"\n");

	fflush( g_fdUserDefLog );

	log_debug(deb_tmp);

	va_end( ap );

	return 1;
}

int log_hexa( unsigned char *szLog, int nSize )
{
	int 		i=0;
	time_t  	tNowTime;
	struct  tm  tmChkTime;

	time(&tNowTime);
	localtime_r(&tNowTime, &tmChkTime);

	if(check_debugfile(nSize) < 0)
		return -1;

	fprintf (g_fdUserDefDegLog, "[%02d/%02d %02d:%02d:%02d] ",
		tmChkTime.tm_mon+1, tmChkTime.tm_mday,
		tmChkTime.tm_hour, tmChkTime.tm_min, tmChkTime.tm_sec );

	for( i=0; i<nSize ; i++ )
	{
		if( i%16 == 0 )
		{
			fprintf( g_fdUserDefDegLog, "\n");
			fprintf( g_fdUserDefDegLog, "%04d", i);
			fflush( g_fdUserDefDegLog );
		}

		if(i%4==0)
			fprintf( g_fdUserDefDegLog, " |");

		fprintf( g_fdUserDefDegLog, " %02X", szLog[i]);
	}

	fprintf( g_fdUserDefDegLog, "\n");
	fflush( g_fdUserDefDegLog );

	return 1;
}

int print_debug(int dType, int dLevel,  char *szMsg, struct tm *pstCheckTime, char *szName)
{
	int			nLen;
	time_t  	tNowTime;
	char		szLevel[80];
	struct  tm  tmChkTime;

	struct timeval	tvTime;

	nLen = strlen(szMsg);
	if(check_debugfile(nLen) < 0)
		return -1;

	time(&tNowTime);
	localtime_r(&tNowTime, &tmChkTime);

	if(dLevel == LOGN_INFO)
		strcpy(szLevel, "INF");
	else if(dLevel == LOGN_DEBUG)
		strcpy(szLevel, "DBG");
	else if(dLevel == LOGN_WARN)
		strcpy(szLevel, "WRN");
	else if(dLevel == LOGN_CRI)
		strcpy(szLevel, "CRI");
	else
		strcpy(szLevel, "UNK");

	gettimeofday(&tvTime, NULL);

#if defined(MICRO_LOG)
	fprintf (g_fdUserDefDegLog,
	"[%s:%d][%02d:%02d:%02d.%06ld][%s] [%s]\n",
	g_szProcName, g_nPid,
	tmChkTime.tm_hour, tmChkTime.tm_min,
	tmChkTime.tm_sec, tvTime.tv_usec,
	szLevel, szMsg);
#else
	fprintf (g_fdUserDefDegLog,
	"[%s:%d][%02d/%02d %02d:%02d:%02d][%s] [%s]\n",
	g_szProcName, g_nPid,
	tmChkTime.tm_mon+1, tmChkTime.tm_mday,
	tmChkTime.tm_hour, tmChkTime.tm_min, tmChkTime.tm_sec,
	szLevel, szMsg);
#endif

	fflush( g_fdUserDefDegLog );

	return 0;
}

//int dAppWrite(int dLevel, char *szMsg)
int print_critical(int dLevel, char *szMsg)
{
	char		szDirPath[80], szMesgPath[80], szLevel[80];
	struct stat stStatInfo;
	time_t  	tNowTime;
	struct  tm  tmChkTime, tmLogTime;
	FILE		*fdMesgFile;
	DIR			*stDirInfo;

	time(&tNowTime);
	localtime_r(&tNowTime, &tmChkTime);

	sprintf(szMesgPath, "%s/%02d%02d/%02d",
		g_szLogFilePath, tmChkTime.tm_mon + 1, tmChkTime.tm_mday, tmChkTime.tm_hour);

	if (stat(szMesgPath, &stStatInfo) == 0)
	{
		localtime_r(&stStatInfo.st_atime, &tmLogTime);
		if (tmChkTime.tm_year != tmLogTime.tm_year)
		{
			unlink (szMesgPath);
			log_close();

			g_fdUserDefLog = NULL;
		}
	}
	else
	{
		log_close();
		stDirInfo = opendir( g_szLogFilePath );
		if( stDirInfo == (DIR*)NULL )
			mkdir( g_szLogFilePath, 0777 );
		else
			closedir( stDirInfo );

		sprintf(szDirPath, "%s/%02d%02d",
			g_szLogFilePath, tmChkTime.tm_mon+1, tmChkTime.tm_mday);

		mkdir(szDirPath, 0777);
		g_fdUserDefLog = NULL;
	}

	if( g_fdUserDefLog == NULL )
	{
		if ((fdMesgFile = fopen(szMesgPath, "a+")) == NULL)
			return -1;

		g_fdUserDefLog = fdMesgFile;
	}

	if(dLevel == LOGN_INFO)
		strcpy(szLevel, "INF");
	else if(dLevel == LOGN_DEBUG)
		strcpy(szLevel, "DBG");
	else if(dLevel == LOGN_WARN)
		strcpy(szLevel, "WRN");
	else if(dLevel == LOGN_CRI)
		strcpy(szLevel, "CRI");
	else
		strcpy(szLevel, "UNK");

	fprintf (g_fdUserDefLog,
		"[%s:%d][%02d/%02d %02d:%02d:%02d][%s] [%s]\n",
		g_szProcName, g_nPid,
		tmChkTime.tm_mon+1, tmChkTime.tm_mday,
		tmChkTime.tm_hour, tmChkTime.tm_min, tmChkTime.tm_sec,
		szLevel, szMsg);

	fflush( g_fdUserDefLog );

#if !defined(COMMERCIALLOG)
//	dAppDebug(LOG_TYPE_WRITE, dLevel, szMsg, &tmChkTime, szLevel);
	print_debug(LOG_TYPE_WRITE, dLevel, szMsg, &tmChkTime, szLevel);
#endif

	return 1;
}

//int debug_print(int dIndex, char *fmt, ...)
int log_print(int dIndex, char *fmt, ...)
{
	char			szMsg[30720];
	va_list		 args;

	g_nLogLevel = g_stLogLevel->usLogLevel[g_nProcIdx];

	switch(dIndex)
	{
		case LOGN_INFO :
		case LOGN_DEBUG :
		case LOGN_WARN :
			if(dIndex > g_nLogLevel)
				return 0;
			va_start(args, fmt);
			vsnprintf(szMsg, 30720, fmt, args);
			va_end(args);
			print_debug(LOG_TYPE_DEBUG, dIndex, szMsg, NULL, NULL);
			break;

		case LOGN_CRI :
			if(dIndex > g_nLogLevel)
				return 0;
			va_start(args, fmt);
			vsnprintf(szMsg, 30720, fmt, args);
			va_end(args);
			print_critical(dIndex, szMsg);
			break;

		case LOGN_NOPRINT :
		default :
			return -1;
	}
	return 0;
}

//int InitAppLog(pid_t nPid, int nProcIdx, char *szLogFilePath, char *szProcName)
int log_init(key_t kShmKey, pid_t nPid, int nProcIdx, char *szLogFilePath, char *szProcName)
{
	int		nShmID;

	g_nPid = nPid;
	g_nProcIdx = nProcIdx;
	sprintf(g_szLogFilePath, "%s", szLogFilePath);
	sprintf(g_szProcName, "%s", szProcName);

	if( (nShmID = shmget(kShmKey, sizeof(st_LogLevel), 0666|IPC_CREAT|IPC_EXCL)) < 0)
	{
		if(errno == EEXIST)
		{
			if( (nShmID = shmget(kShmKey, sizeof(st_LogLevel), 0666|IPC_CREAT)) < 0)
			{
				fprintf(stderr, "shmget ERROR[%d:%s]", errno, strerror(errno));
				return -errno;
			}

			if( (void *)(g_stLogLevel = (st_LogLevel *)shmat(nShmID, 0, 0)) == (void *)-1)
				return -errno;

		}
		else
			return -errno;

		g_stLogLevel->usLogLevel[g_nProcIdx] = LOGN_DEBUG;
	}
	else
	{
		if( (void *)(g_stLogLevel = (st_LogLevel*)shmat(nShmID, 0, 0)) == (void *)-1)
			return -errno;

		g_stLogLevel->usLogLevel[g_nProcIdx] = LOGN_DEBUG;
	}

	return 0;
}
