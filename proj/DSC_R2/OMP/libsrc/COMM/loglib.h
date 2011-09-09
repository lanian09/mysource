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

#define	LOGLIB_MAX_OPEN_FILE	16		/* �� ���μ������� �ִ�� open�� �� �ִ� log file ���� */
#define	LOGLIB_MAX_FILE_SIZE	1024000	/* LOGLIB_MODE_LIMIT_SIZE�� ��� ���� ũ�� ���� (byte����) */
#define	LOGLIB_MAX_LOG_SUFFIX	10		/* LOGLIB_MODE_LIMIT_SIZE�� ��� �α� ������ ����� ���� ������ ���� */

#define	LOGLIB_MODE_LIMIT_SIZE		0x00000001	/* ȭ�� ũ�⸦ ���� �ϴ� ��� */
#define	LOGLIB_MODE_DAILY			0x00000002	/* ���� ���ο� ȭ���� �����ϴ� ��� */
#define	LOGLIB_MODE_HOURLY			0x00000004	/* �Ž� ���ο� ȭ���� �����ϴ� ��� */
#define	LOGLIB_MODE_ONE_DIR			0x00000008	/* YYYY.mm.dd �����ϴ� ��� */
#define	LOGLIB_MODE_7DAYS			0x00010000	/* 1������ ���� ���� �ڵ� ����  */
#define	LOGLIB_FLUSH_IMMEDIATE		0x00020000	/* �Ź� fflush �Ѵ�. */
#define	LOGLIB_FNAME_LNUM			0x00040000	/* �ҽ� �����̸��� line_number ��� */
#define	LOGLIB_TIME_STAMP			0x00080000	/* �ð�(time_stamp) ��� */

#define TRCBUF_LEN  COMM_MAX_IPCBUF_LEN
#define TRCTMP_LEN  1024

typedef struct {
	char	fname[64];	/* file name */
	FILE 	*fp;		/* file pointer */
	int		mode;
	time_t	lastTime;	/* LOGLIB_MODE_DAILY, LOGLIB_MODE_HOURLY�� ��� ������ ��� �ð� */
} LogInformationTable;


extern int errno;

extern int loglib_openLog (char*, int);
extern int loglib_closeLog (int);
extern int logPrint (int, char*, int, char*, ...);
extern int loglib_checkLimitSize (int);
extern int loglib_checkDate (int);
extern int loglib_checkTimeHour (int);


#endif /*__LOGLIB_H__*/
