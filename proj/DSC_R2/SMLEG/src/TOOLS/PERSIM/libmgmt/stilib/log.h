/*****************************************************************************
 @(#) $Id: log.h,v 1.1.1.1 2011/04/19 14:13:42 june Exp $
 -----------------------------------------------------------------------------
 Copyright (C) 2002 SoftTeleware, Inc. <http://www.softteleware.com>
 All Rights Reserved.

 Last Modified $Date: 2011/04/19 14:13:42 $ by $Author: june $
 *****************************************************************************/

#ifndef _EMS_LOG_H
#define _EMS_LOG_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_0		0
#define LOG_1		1
#define LOG_2		2
#define LOG_3		3
#define LOG_4		4
#define LOG_5		0
#define LOG_6		6
#define LOG_7		7
#define LOG_8		8
#define LOG_9		9

#define THR_0		0
#define THR_1		1
#define THR_2		2
#define THR_3		3
#define THR_4		4
#define THR_5		0
#define THR_6		6
#define THR_7		7
#define THR_8		8
#define THR_9		9

#define LOG_CONSOLE	1
#define LOG_FILE	2
#define LOG_GLOBAL	4				/* for later */

#define ERR_SYS_PANIC	0
#define ERR_USR_PANIC	1
#define ERR_SYS_NORMAL	2
#define ERR_USR_NORMAL	3

#define MAX_LOG_SIZE		10485760		/* 10 Mbyte */	
#define MIN_LOG_SIZE		1024			/* 1 Kbyte 	*/
#define MID_LOG_SIZE		2097152			/* 2 MByte	*/
#define DEFAULT_LOG_SIZE	1048576			/* 1 MByte 	*/

#define EXT_LOG_NUM			30
#define MAX_LOG_NUM			10		
#define MIN_LOG_NUM			1
#define DEFAULT_LOG_NUM		5

#define MAX_LOG			10
#define MAX_FNUM		10000	

#define MODE_PATH			0
#define MODE_NO_PATH		1

typedef struct {
	char fname[BUFSIZ];		/* Fulli Path		*/
	char procs[BUFSIZ];
	char sys[BUFSIZ];
	char date[BUFSIZ];
	char path[BUFSIZ];		/* Log Root Path	*/
	int stdio;
	int level;
	int fnum;
	int fsize;
	int mode;
	char err_fname[BUFSIZ];
	int err_level;
} log_t;

extern log_t _log[MAX_LOG];

extern char* fmttm(char*,char*);
extern int dump(char*,int);

extern int initlog(int idx, char *path, char *sys, char *procs, int lev, int out, int mode);
extern int logset(int idx, int maxfn, int maxfs);
extern int setlogf(int idx, int maxfn, int maxfs);
extern int setloglevel(int idx, int level);
extern int getloglevel(int idx);
extern int lprintf(int idx, int level,char* fmt,...);
extern int logprn(int idx, int level,int tid,char* fmt,...);
extern int logdump(int idx,unsigned char*,int);
extern int check_date(int);
extern int check_fsize(int);

#ifdef __cplusplus
}
#endif

#endif
