#ifndef	__LOGLIB_H__
#define	__LOGLIB_H__

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <stdarg.h>
#include <string.h>
#include <sys/time.h>
#include <dirent.h>
#include <unistd.h>


#define	FL	__FILE__, __LINE__

#define	LOGLIB_MAX_OPEN_FILE	16		/* 한 프로세스에서 최대로 open할 수 있는 log file 갯수 */
#define	LOGLIB_MAX_FILE_SIZE	1024000	/* LOGLIB_MODE_LIMIT_SIZE의 경우 파일 크기 제한 (byte단위) */
#define	LOGLIB_MAX_LOG_SUFFIX	10		/* LOGLIB_MODE_LIMIT_SIZE의 경우 로그 파일을 몇개까지 만들 것인지 정의 */

#define	LOGLIB_MODE_LIMIT_SIZE		0x00000001	/* 화일 크기를 제한 하는 경우 */
#define	LOGLIB_MODE_DAILY			0x00000002	/* 매일 새로운 화일을 생성하는 경우 */
#define	LOGLIB_MODE_HOURLY			0x00000004	/* 매시 새로운 화일을 생성하는 경우 */
#define	LOGLIB_MODE_ONE_DIR			0x00000008	/* YYYY.mm.dd 생성하는 경우 */
#define	LOGLIB_MODE_7DAYS			0x00010000	/* 1주일이 지난 파일 자동 삭제  */
#define	LOGLIB_FLUSH_IMMEDIATE		0x00020000	/* 매번 fflush 한다. */
#define	LOGLIB_FNAME_LNUM			0x00040000	/* 소스 파일이름과 line_number 기록 */
#define	LOGLIB_TIME_STAMP			0x00080000	/* 시각(time_stamp) 기록 */

#define TRCBUF_LEN  COMM_MAX_IPCBUF_LEN
#define TRCTMP_LEN  1024

typedef struct {
	char	fname[64];	/* file name */
	FILE 	*fp;		/* file pointer */
	int		mode;
	time_t	lastTime;	/* LOGLIB_MODE_DAILY, LOGLIB_MODE_HOURLY인 경우 마지막 기록 시각 */
} LogInformationTable;


extern int errno;

extern int loglib_openLog (char*, int);
extern int loglib_closeLog (int);
extern int logPrint (int, char*, int, char*, ...);
extern int loglib_checkLimitSize (int);
extern int loglib_checkDate (int);
extern int loglib_checkTimeHour (int);


#endif /*__LOGLIB_H__*/
