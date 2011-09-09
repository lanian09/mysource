/**
	@file		mmcd_util.h
	@author
	@version
	@date		2011-07-14
	@brief		mmcd_util.c 헤더파일
*/

#ifndef __MMCD_UTIL_H__
#define __MMCD_UTIL_H__

/**
	Declare functions
*/
extern int send_com_data(int osfd, char *sBuf, int endFlag, mml_msg *ml, int dResult, int dConTblIdx);
extern void CheckClient(int dReloadFlag);
extern int dGetNMSsfd(void);
extern int dGetMaxFds(void);
extern int cCheckTrendCmd(char cType, char *sCmdStr);
extern char *mtime();

#endif	/* __MMCD_UTIL_H__ */
