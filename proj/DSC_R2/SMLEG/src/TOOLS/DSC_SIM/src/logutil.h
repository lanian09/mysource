#ifndef _LOGUTIL_H_
#define _LOGUTIL_H_

#include <time.h>
#include <define.h>


/* log level struct */
/* extern st_LogLevel     *gLogLevel; */
extern int     vdLogLevel;

/* DEFINE LOG LEVEL */
#define LOG_NOPRINT         0
#define LOG_CRI             1
#define LOG_WARN            2
#define LOG_DEBUG           3
#define LOG_INFO            4

#define LOG_TYPE_DEBUG  	1
#define LOG_TYPE_WRITE  	2

/* LOG FILE SIZE */
#define LOG_FILE_SIZE   	10000000

#define MAX_LOGPATH_SIZE    1024

/* LOG FILE NUMBER */
#define DEF_DEBUG_NUM   	10


#define LOG_FILETYPE_APPLOG 1
#define LOG_FILETYPE_BEACON 2

#define FVERSION            "1003                    "

extern void InitAppLog(int pid, int procidx, char *logfilepath, char *proc_name);
extern int dAppLog(int dIndex, char *fmt, ...);
extern int dAppHexa(int dIndex, unsigned char *szLog, int dSize);
extern int dAppDump(char *s,int len);

#endif
