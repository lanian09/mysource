/**
	@file		mmcd_db.h
	@author
	@version
	@date		2011-07-20
	@brief		mmcd_db.c 헤더파일, MMCD에서 사용하는 DB관련 함수들
*/

#ifndef __MMCD_DB_H__
#define __MMCD_DB_H__


/**
	Include headers
*/
#include <time.h>
#include <mysql/mysql.h>

#include "mmcdef.h"
#include "define.h"

#define MAX_SQLQUERY_SIZE		20480

typedef struct _st_sys_mmcd_msg
{
	char            szUserName[MAX_USER_NAME];
	unsigned int    uiTime;
	unsigned int    uiUserBIP;
	unsigned short  usSeq;
	unsigned short  usResult;
	unsigned char   szCommand[MAX_MMC_COMMAND_LEN];
	unsigned char   szMessage[MAX_MMCD_MSG_SIZE];
} st_SysMMCDMsg, *pst_SysMMCDMsg;

/**
  Declare functions
 */
extern int dUser_Login_Update(MYSQL *pstMySQL, char *sUserName, time_t tLoginTime);
extern int dGetUserInfo(MYSQL *pstMySQL, st_UserAdd *pstData, int *dCnt);
extern int dInsert_MMCDMsg(MYSQL *pstMySQL, st_SysMMCDMsg *pstData);
extern int dCreate_MMCDMsg(MYSQL *pstMySQL);
extern int dCommonDDL(MYSQL *pstMySQL, char *psCallFn, const char *psQuery);

#endif	/* __MMCD_DB_H__ */
