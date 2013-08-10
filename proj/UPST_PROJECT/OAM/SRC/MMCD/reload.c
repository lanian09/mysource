/**
	@file		reload.c
	@author
	@version
	@date		2011-07-14
	@brief		reload.c
*/

/**
 *	Include headers
 */

/* SYS HEADER */
#include <stdio.h>
/* LIB HEADER */
#include "commdef.h"
#include "config.h"
#include "loglib.h"
/* PRO HEADER */
#include "path.h"
#include "mmcdef.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */
#include "para_load.h"
#include "cmd_user.h"
#include "reload.h"

/**
 *	Declare variables
 */
extern int			myBlockCode;			/*< mmcd_main.h */

/**
 *	Declare extern func.
 */
extern void close_lib_tbl();
extern int load_cmd_tbl(FILE *fp);
extern int load_para_tbl(void);
extern int init_lib_tbl();

/**
 *	Implement func.
 */
int Init_tbl()
{
	FILE *fp;
	int	 res;
	PARA_TBL *para_tbl;

	res = load_para_tbl();
	if( res <= 0 )
	{
		log_print(LOGN_CRI,"FAILURE IN LOADING PARA-TBL dRet = %d",res);
		return -1;
	}

	/* Shared Lib Loading */
	/* lib_load.c */
	res = init_lib_tbl();
	if( res <= 0 )
	{
		log_print(LOGN_CRI,"FAILURE IN LOADING LIB-TBL dRet = %d", res);
		return -2;
	}

	/* para_load.c */
	if ((para_tbl = get_para_tbl("MMCD")) == NULL)
	{
		 log_print(LOGN_CRI,"NOT FOUND MMCD CODE");
		 return -3;
	 }

	 myBlockCode = para_tbl->code;

    if ((fp = fopen(COMMAND_FILE_PATH, "r")) == NULL) {
        return eLOAD_CMD_FILE;
    }

	/* cmd_load.c */
	res = load_cmd_tbl(fp);
	fclose(fp);

	if (res <= 0)
	{
		log_print(LOGN_CRI,"FAILURE IN LOADING CMD-TBL : RET[%d]", res);
	}

	return res;
}

int Rebuild_tbl()
{
	close_lib_tbl();
	return Init_tbl();
}
