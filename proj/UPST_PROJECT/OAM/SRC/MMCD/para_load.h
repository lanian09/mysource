/**
	@file		para_load.h
	@author
	@version
	@date		2011-07-14
	@brief		para_load.c 헤더파일
*/

#ifndef __PARA_LOAD_H__
#define __PARA_LOAD_H__

#define ENUM_LIST_PATH			START_PATH"/DATA/MMC_ENUM"
#define MAX_PARA_SYMBOL         2048

typedef struct {
	char        *para_name;
	char        *para_help;     /* Parameter에 대한 상세 설명 */
	long long   code;
} PARA_TBL;

/**
	Declare functions
*/
extern void itom(int idx, char *res);
extern int para_cmp_sort(const void *a, const void *b);
extern int para_cmp(const void *a, const void *b);
extern PARA_TBL *get_para_tbl(char *str);
extern int get_para_idx(char *str);
extern int load_para_tbl(void);

#endif /* __PARA_LOAD_H__ */
