#ifndef _LOGLIB_H_
#define _LOGLIB_H_

#include <time.h>
#include <errno.h>
#include <sys/types.h>

#include "config.h" /* MAX_SW_COUNT */

/* DEFINE LOG LEVEL */
#define LOGN_NOPRINT		0
#define LOGN_CRI			1
#define LOGN_WARN			2
#define LOGN_DEBUG			3
#define LOGN_INFO			4

#define LOG_TYPE_DEBUG  	1
#define LOG_TYPE_WRITE  	2

/* LOG FILE SIZE */
#define LOG_FILE_SIZE   	20000000

#define MAX_LOGPATH_SIZE	1024

/* LOG FILE NUMBER */
#define DEF_DEBUG_NUM   	50


#define LOG_FILETYPE_APPLOG 1
#define LOG_FILETYPE_BEACON 2

#define FVERSION			"1003					"

#define LH "%s.%d:%s "
#define LT __FILE__,__LINE__,__FUNCTION__
#define EH ", errno=%d:%s"
#define ET errno,strerror(errno)

typedef struct _st_LogLevel
{
//	unsigned short  usSysType[MAX_SW_COUNT];
	unsigned short  usLogLevel[MAX_SW_COUNT];   /* SEQ_PROC_## NUMBER */
} st_LogLevel, *pst_LogLevel;

typedef struct _st_LogLevel_List
{
	int dCount;
	char reserved[4];
	st_LogLevel  stLogLevel;
} st_LogLevel_List, *pst_LogLevel_List;

/* log level struct */
extern st_LogLevel	 *g_stLogLevel;

/* Beacon File Header Format */
typedef struct {
	union {
		struct _Info {
			char	cHeaderLen;		/* 16 */
			char	cFlag;			/* 1: Packet log, 2: Msg log */
			short   sRetCode;		/* 0:Success, others: ErrorCode */
		} Info;
		int	 lPacketNo;
	};
	int	 nSize;						/* body size */
	time_t  tCurTime;				/* current time */
	int	 nCurMTime;					/* current micro time */
} st_BeaconHdr;

#define BEACON_FILE_SIZE	10000000
#define MAX_DEBUG_FILE		10

/* function prototype */
extern int bcon_init( char *szLogFilePath );
extern int bcon_write( unsigned char *szBuf, int size, int nFlag, int nRetCode, int nLogLevel );

/* 가능한 사용하지 마시오. => log_print 만 사용해서 작성하도록 해주세요.  */
/* CAUTION */ int log_debug(char *fmt, ...);
/* CAUTION */ int log_write(char *fmt, ...);

extern int log_hexa( unsigned char *szLog, int nSize );
//extern int log_print(int dIndex, char *fmt, ...);
extern int log_print(int dIndex, char *fmt, ...)
		__attribute__((format(printf,2,3)));
extern int log_init(key_t kShmKey, pid_t nPid, int nProcIdx, char *szLogFilePath, char *szProcName);
extern int log_close();

#endif
