/**
	@file		lib_load.c
	@author
	@version
	@date		2011-07-18
	@brief		lib_load.c
*/

/**
	Include headers
*/
/* SYS HEADER */
#include <unistd.h>
#include <dlfcn.h>
/* LIB HEADER */
#include "config.h"
/* PRO HEADER */
#include "mmcdef.h"
#include "path.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */
#include "lib_load.h"
#include "cmd_load.h"

/**
	Define constants
*/
#define SO_LIB_PATH					START_PATH"/BIN/libsmsprn.so"
#define DEFAULT_MMC_FUNC			"P_default_function"

/**
	Declare variables
*/
static void			*lib_handle = NULL;

extern LIB_TBL		lib_tbl[MAX_LIB_TABLE];		/*< mmcd_main.h */

/**
 * Delcare func.
 */
char *(*ErrMess_Prt)(int);
void (*set_cont_flag)(int);
int (*get_cont_flag)(void);

/**
 *	Implement func.
 */
void close_lib_tbl()
{
	if (lib_handle == NULL)
		return;
	dlclose(lib_handle);
}

int init_lib_tbl()
{
	int	i;

	/* SO_LIB_PATH -> /usr1/app/bin/libsmsprn.so */
	if ((lib_handle = dlopen(SO_LIB_PATH, RTLD_NOW)) == NULL) {
		return -1;
	}

	/* lib table 초기화 */
	for (i = 0;i < MAX_LIB_TABLE;i++) {
		lib_tbl[i].mmc_res  = NULL;
		lib_tbl[i].com_name = NULL;
		lib_tbl[i].block = 0;
	}

	/* 특수 함수 로딩 */
	if ((ErrMess_Prt = (char *(*)())dlsym(lib_handle, "MH_ErrMess")) == NULL)
		return -2;
	if ((set_cont_flag = (void (*)())dlsym(lib_handle, "set_cont_flag")) == NULL)
		return -3;
	if ((get_cont_flag = (int (*)())dlsym(lib_handle, "get_cont_flag")) == NULL)
		return -4;

	return 1;
}


int set_lib_entry(int idx, char *func_name)
{
	void (*mmc_res) (char *, void *, short * , short * );

	/* Shared Object 설정 */

	/* 전용 함수 검색후 실패하면 Default 함수로 설정 */
	if ((mmc_res = (void (*)())dlsym(lib_handle, func_name)) == NULL) {
		if (lib_tbl[idx].mmc_res == NULL) {
			if ((mmc_res = (void (*)())dlsym(lib_handle, DEFAULT_MMC_FUNC)) == NULL) {
				dlclose(lib_handle);
				return 0;
			}
			/* DEFAULT FUNCTION LOADED */
			lib_tbl[idx].mmc_res = mmc_res;
		}
	} else {
		/* Its own print lib loaded */
		lib_tbl[idx].mmc_res = mmc_res;
	}
	return 1;
}

