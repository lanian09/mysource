/**
	@file		mmcd_db.c
	@author
	@version
	@date		2011-07-20
	@brief		MMCD에서 사용하는 DB관련 함수들
*/

/**
	Include headers
*/

/* SYS HEADER */
#include <errno.h>
#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* LIB HEADER */
#include "config.h"
#include "loglib.h"
/* PRO HEADER */
#include "mmcdef.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */
#include "mmcd_db.h"

/**
	Define constants
*/
#define MAX_SQLQUERY_SIZE		20480
#define DEF_INIT_MYSQL			DATA_PATH"DQMS_MySQL.env"

#define DB_TABLE_NOT_EXIST		1146
#define DB_NOT_CONNECT			2002

#define MAX_STMT_SIZE			1024
#define MAX_FILENAME_SIZE		256

/**
 *	Implement func.
 */

/**
	@brief	dUser_Login_Update
	@param	pstMySQL
	@param	sUserName
	@param	tLoginTime
	@return int
*/
int dUser_Login_Update(MYSQL *pstMySQL, char *sUserName, time_t tLoginTime)
{
	char		szBuf[MAX_SQLQUERY_SIZE];

	sprintf(szBuf,
		"UPDATE SYS_USER_INFO "
			"SET LASTLOGINTIME = %lu "
		"WHERE "
			"USERNAME = '%s'",
		tLoginTime, sUserName);

	if(mysql_query(pstMySQL, szBuf) != 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_query(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			szBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
		return -1;
	}

	return 0;
}

/**
	@brief	dGetUserInfo
	@param	pstMySQL
	@param	pstData
	@param	dCnt
	@return	int
*/
int dGetUserInfo(MYSQL *pstMySQL, st_UserAdd *pstData, int *dCnt)
{
	int			i;
	char		szBuf[MAX_SQLQUERY_SIZE];
	MYSQL_ROW	stRow;
	MYSQL_RES	*pstRst;

	sprintf(szBuf,
		"SELECT "
			"USERNAME, PASSWORD, USERLEVEL, LOGIN, LASTLOGINTIME, CONNECTIP, LOCALNAME, CONTACT, CREATETIME "
		"FROM "
			"SYS_USER_INFO");

	if(mysql_query(pstMySQL, szBuf) != 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_query(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			szBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
		return -1;
	}

	if( (pstRst = mysql_store_result(pstMySQL)) == NULL)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_store_result(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			szBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
		return -2;
	}

	i		= 0;
	while( (stRow = mysql_fetch_row(pstRst)) != NULL)
	{
		if(i >= MAX_USER)
		{
			log_print(LOGN_DEBUG, "F=%s:%s.%d: SYS_USER_INFO MAX_USER[%d] OVER[%d] [%s]", __FILE__, __FUNCTION__, __LINE__,
				MAX_USER, i, szBuf);
			break;
		}

		if(strlen(stRow[0]) < USERINFO)
			strncpy(pstData[i].szUserName, stRow[0], USERINFO);
		else
		{
			strncpy(pstData[i].szUserName, stRow[0], USERINFO-1);
			pstData[i].szUserName[USERINFO-1] = 0x00;
		}

		if(strlen(stRow[1]) < USERINFO)
			strncpy(pstData[i].szPassword, stRow[1], USERINFO);
		else
		{
			strncpy(pstData[i].szPassword, stRow[1], USERINFO-1);
			pstData[i].szPassword[USERINFO-1] = 0x00;
		}

		pstData[i].sSLevel			= (short)atoi(stRow[2]);
		pstData[i].usLogin			= (unsigned short)atoi(stRow[3]);
		pstData[i].uLastLoginTime	= (unsigned int)atoi(stRow[4]);
		pstData[i].uConnectIP		= (unsigned int)atoi(stRow[5]);

		if(strlen(stRow[6]) < USERLN)
			strncpy(pstData[i].szLocalName, stRow[6], USERLN);
		else
		{
			strncpy(pstData[i].szLocalName, stRow[6], USERLN-1);
			pstData[i].szLocalName[USERLN-1] = 0x00;
		}

		if(strlen(stRow[7]) < USERLN)
			strncpy(pstData[i].szContact, stRow[7], USERLN);
		else
		{
			strncpy(pstData[i].szContact, stRow[7], USERLN-1);
			pstData[i].szContact[USERLN-1] = 0x00;
		}
		pstData[i].uCreateTime	= (unsigned int)atoi(stRow[8]);
		i++;
	}
	mysql_free_result(pstRst);
	*dCnt	= i;

	if(i >= MAX_USER)
		return i;
	else
		return 0;
}

int dInsert_MMCDMsg(MYSQL *pstMySQL, st_SysMMCDMsg *pstData)
{
	char	szBuf[MAX_SQLQUERY_SIZE];

	sprintf(szBuf, "INSERT INTO SYS_MMCD_MSG VALUES ('%s',%u,%u,%d,%d,'%s','%s')",
		pstData->szUserName, pstData->uiUserBIP, pstData->uiTime, pstData->usSeq,
		pstData->usResult, pstData->szCommand, pstData->szMessage);

	if(mysql_query(pstMySQL, szBuf) != 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_query(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			szBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));

		switch(mysql_errno(pstMySQL))
		{
			case DB_TABLE_NOT_EXIST:
				return -1;
			case DB_NOT_CONNECT:
				return -2;
			default:
				return -3;
		}
	}

	return 0;
}

int dCreate_MMCDMsg(MYSQL *pstMySQL)
{
	char    szQuery[MAX_STMT_SIZE*4], sFn[MAX_FILENAME_SIZE];
	int     dRet;

	sprintf(szQuery,
		"CREATE TABLE `SYS_MMCD_MSG` ("
			"`USERNAME` varchar(24) NOT NULL default '',"
			"`USERIP` int(10) unsigned default NULL,"
			"`TIME` bigint(20) NOT NULL default '0',"
			"`SEQ` int(11) NOT NULL default '0',"
			"`RESULT` int(11) default NULL,"
			"`COMMAND` varchar(255) default NULL,"
			"`MESSAGE` text,"
			"PRIMARY KEY  (`USERNAME`,`TIME`,`SEQ`)"
		") ENGINE=InnoDB DEFAULT CHARSET=euckr");

	sprintf(sFn, "%s", __FUNCTION__);
	if( (dRet = dCommonDDL(pstMySQL, sFn, (const char*)szQuery)) < 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN dCommonDDL() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		return -1;
	}

	return 0;
}

int dCommonDDL(MYSQL *pstMySQL, char *psCallFn, const char *psQuery)
{
	if(mysql_query(pstMySQL, psQuery) != 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_query(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			psQuery, mysql_errno(pstMySQL), mysql_error(pstMySQL));
		return -1;
	}

	return 0;
}
