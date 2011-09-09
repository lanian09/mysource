/**
	@file		help_proc.h
	@author
	@version
	@date		2011-07-14
	@brief		help_proc.c 헤더파일
*/
#ifndef __HELP_PROC_H__
#define __HELP_PROC_H__

#include "cmd_load.h"

/**
	Declare functions
*/
extern int append_help(int i, char *outstr, int *slen, int *lline);
extern int cmd_description(int slen, COM_TBL *cmd, char *outstr);
extern void help_proc(char *outstr);

#endif /* __HELP_PROC_H__ */
