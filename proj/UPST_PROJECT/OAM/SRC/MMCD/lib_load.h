/**
	@file		lib_load.h
	@author
	@version
	@date		2011-07-18
	@brief		lib_load.c 헤더파일
*/
#ifndef __LIB_LOAD_H__
#define __LIB_LOAD_H__

/**
	Declare functions
*/
extern int init_lib_tbl(void);
extern void close_lib_tbl();
extern int set_lib_entry(int idx, char *func_name);

#endif /* __LIB_LOAD_H__ */
