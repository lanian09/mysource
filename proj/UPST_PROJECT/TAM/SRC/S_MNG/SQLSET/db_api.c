/**A.1* FILE INCLUSION ********************************************************/
#include <stdio.h>			/*	sprintf(3)	*/
#include <mysql/mysql.h>	/*	MYSQL, MYSQL_ROW, MYSQL_RES structure, mysql_query, mysql_store_result, mysql_fetch_row, mysql_free_result	*/
#include <string.h>			/*	strncpy(3)	*/
#include <stdlib.h>			/*	atoll(3), atoi(3)	*/
#include <linux/limits.h>	/*	PATH_MAX	*/
#include <sys/types.h>		/*	open(2)		*/
#include <sys/stat.h>		/*	open(2)		*/
#include <fcntl.h>			/*	open(2)		*/
#include <unistd.h>			/*	write(2), close(2)	*/
#include <errno.h>

#if 0
#include "mmcdef.h"			/*  st_UserAdd */
#include "msg_struct.h"		/*  IDX_XX_STATISTICS */
#endif
#include "commdef.h"		/* BUF_SIZE */
#include "db_define.h"
#include "db_struct.h"		/* MAX_CONDMSG_CNT */
#include "filter.h"			/* st_xxx */
#include "msgdef.h"			/* IDX_MAXIMUM_STATISTICS */
#include "mmcdef.h"			/* MAX_USER */
#include "db_api.h"
#include "loglib.h"

/**B.1*  Definition of New Constants *********************/
/**B.2*  Definition of New Type  **************************/
/**C.1*  Declaration of Variables  ************************/
/**D.1*  Definition of Functions  *************************/
/**D.2*  Definition of Functions  *************************/

int dTruncateTbl(MYSQL *pstMySQL, char *sTblName)
{
	char		szBuf[MAX_SQLQUERY_SIZE];

	sprintf(szBuf, "TRUNCATE TABLE %s", sTblName);

	if(mysql_query(pstMySQL, szBuf) != 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_query(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			szBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
		return -1;
	}

	return 0;
}

int dGetMMCUserCount(MYSQL *pstMySQL, int *dCnt)
{
	int			dResult;
	char		szBuf[MAX_SQLQUERY_SIZE];
	MYSQL_ROW	stRow;
	MYSQL_RES	*pstRst;

	sprintf(szBuf, "SELECT COUNT(*) FROM SYS_USER_INFO");

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

	dResult	= 0;
	while( (stRow = mysql_fetch_row(pstRst)) != NULL)
	{
		dResult	= atoi(*stRow);
		if(dResult >= MAX_USER)
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: SYS_USER_INFO MAX_USER[%d] OVER[%d] [%s]", __FILE__, __FUNCTION__, __LINE__,
				MAX_USER, dResult, szBuf);
			break;
		}
	}
	mysql_free_result(pstRst);
	*dCnt = dResult;

	return 0;
}

int dInsertEquip(MYSQL *pstMySQL, st_Info_Equip *stData)
{
	char		szBuf[MAX_SQLQUERY_SIZE];

	sprintf(szBuf, "INSERT INTO TB_MEQUIPMC VALUES (%u,%u,%hu,'%s','%s','%s', %hu)",
		stData->uEquipIP,stData->uNetmask,stData->cEquipType, stData->szEquipTypeName, stData->szEquipName, stData->szDesc, stData->cMon1MinFlag);

	if(mysql_query(pstMySQL, szBuf) != 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_query(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			szBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
		if(mysql_errno(pstMySQL) == DB_DUPLICATED_ENTRY)
			return -1;
		else
			return -2;
	}

	return 0;
}

int dGetFltEquipCount(MYSQL *pstMySQL, int *dCount)
{
	int			dResult;
	char		szBuf[MAX_SQLQUERY_SIZE];
	MYSQL_ROW	stRow;
	MYSQL_RES	*pstRst;

	sprintf(szBuf,
		"SELECT "
			"COUNT(*) "
		"FROM "
			"TB_MEQUIPMC "
		"WHERE "
			"EquipType IN (6,7,8,9)");

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

	dResult	= 0;
	while( (stRow = mysql_fetch_row(pstRst)) != NULL)
		dResult	= atoi(*stRow);

	mysql_free_result(pstRst);
	*dCount = dResult;

	return 0;

}

int dGetFltRoamEquipCount(MYSQL * pstMySQL, int * dCount)
{
	int			dResult;
	char		szBuf[MAX_SQLQUERY_SIZE];
	MYSQL_ROW	stRow;
	MYSQL_RES	*pstRst;

	sprintf(szBuf,
		"SELECT "
			"COUNT(*) "
		"FROM "
			"TB_MEQUIPMC "
		"WHERE "
			"EquipType=%d OR EquipType=%d OR EquipType>=%d", LNS_SYSTYPE, SYSTEM_TYPE_ROAMAAA, ROAM_JAPAN);

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

	dResult	= 0;
	while( (stRow = mysql_fetch_row(pstRst)) != NULL)
		dResult	= atoi(*stRow);

	mysql_free_result(pstRst);
	*dCount = dResult;

	return 0;

}


int dGetInfoAccessCount(MYSQL *pstMySQL, unsigned char cSysType, int *dCount)
{
	int			dResult;
	char		szBuf[MAX_SQLQUERY_SIZE];
	MYSQL_ROW	stRow;
	MYSQL_RES	*pstRst;

	switch(cSysType)
	{
		case SYSTEM_TYPE_PCF:
			sprintf(szBuf,
				"SELECT "
					"COUNT(*) "
				"FROM "
				"("
					"SELECT "
						"BRANCHID,PCFTYPE,PCFIP "
					"FROM "
						"TB_MACCESSC "
					"GROUP BY "
						"BRANCHID,PCFTYPE,PCFIP"
				") PCFTABLE");
			break;
		case SYSTEM_TYPE_BSC:
			sprintf(szBuf,
				"SELECT "
					"COUNT(*) "
				"FROM "
				"("
					"SELECT "
						"BRANCHID,BSCID,SYSID "
					"FROM "
						"TB_MACCESSC "
					"WHERE "
						"PCFTYPE != 1 "
					"GROUP BY "
						"BRANCHID,BSCID,SYSID"
				") BSCTABLE");
			break;
		case SYSTEM_TYPE_BTS:
			sprintf(szBuf,
				"SELECT "
					"COUNT(*) "
				"FROM "
				"("
					"SELECT "
						"BRANCHID,BSCID,BTSID,SYSID "
					"FROM "
						"TB_MACCESSC "
					"WHERE "
						"PCFTYPE != 1 "
					"GROUP BY "
						"BRANCHID,BSCID,BTSID,SYSID"
				") BTSTABLE");
			break;
		default:
			log_print(LOGN_CRI, "F=%s:%s.%d: UNAVAILABLE cSysType[%hu]", __FILE__, __FUNCTION__, __LINE__, cSysType);
			return -1;
	}

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

	dResult		= 0;
	while( (stRow = mysql_fetch_row(pstRst)) != NULL)
		dResult = atoi(*stRow);

	mysql_free_result(pstRst);
	*dCount = dResult;

	return 0;
}

int dGetInfoAccessBTS(MYSQL *pstMySQL, st_WatchBTSList *pstData)
{
	int			i;
	char		szBuf[MAX_SQLQUERY_SIZE];
	MYSQL_ROW	stRow;
	MYSQL_RES	*pstRst;

	sprintf(szBuf,
		"SELECT "
			"IFNULL(BRANCHID,0),IFNULL(BSCID,0),IFNULL(BTSID,0),IFNULL(SYSID,0) "
		"FROM "
			"TB_MACCESSC "
		"WHERE "
			"PCFTYPE != 1 "
		"GROUP BY "
			"BRANCHID,BSCID,BTSID,SYSID");

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
		if(i >= MAX_MON_BTS_CNT)
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: TB_MACCESSC BTS COUNT[%d] is over MAX_MON_BTS_CNT[%d] [%s]", __FILE__, __FUNCTION__, __LINE__,
				i, MAX_MON_BTS_CNT, szBuf);
			break;
		}
		pstData->stWatchBTS[i].ucOffice		= (unsigned char)atoi(stRow[0]);
		pstData->stWatchBTS[i].ucBSCID		= (unsigned char)atoi(stRow[1]);
		pstData->stWatchBTS[i].usBTSID		= (unsigned short)atoi(stRow[2]);
		pstData->stWatchBTS[i].ucSYSID		= (unsigned char)atoi(stRow[3]);
		i++;
	}
	mysql_free_result(pstRst);
	pstData->dCount = i;

	return 0;
}

int dGetInfoAccessBSC(MYSQL *pstMySQL, st_WatchBSCList *pstData)
{
	int			i;
	char		szBuf[MAX_SQLQUERY_SIZE];
	MYSQL_ROW	stRow;
	MYSQL_RES	*pstRst;

	sprintf(szBuf,
		"SELECT "
			"IFNULL(BRANCHID,0),IFNULL(BSCID,0),IFNULL(SYSID,0) "
		"FROM "
			"TB_MACCESSC "
		"WHERE "
			"PCFTYPE != 1 "
		"GROUP BY "
			"BRANCHID,BSCID,SYSID");

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
		if(i >= MAX_MON_BSC_CNT)
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: TB_MACCESSC BSC COUNT[%d] is over MAX_MON_BSC_CNT[%d] [%s]", __FILE__, __FUNCTION__, __LINE__,
				i, MAX_MON_BSC_CNT, szBuf);
			break;
		}
		pstData->stWatchBSC[i].ucOffice		= (unsigned char)atoi(stRow[0]);
		pstData->stWatchBSC[i].ucBSCID		= (unsigned char)atoi(stRow[1]);
		pstData->stWatchBSC[i].ucSYSID		= (unsigned char)atoi(stRow[2]);
		i++;
	}
	mysql_free_result(pstRst);
	pstData->dCount = i;

	return 0;
}

int dGetInfoAccessPCF(MYSQL *pstMySQL, st_WatchPCFList *pstData)
{
	int			i;
	char		szBuf[MAX_SQLQUERY_SIZE];
	MYSQL_ROW	stRow;
	MYSQL_RES	*pstRst;

	sprintf(szBuf,
		"SELECT "
			"IFNULL(BRANCHID,0),IFNULL(PCFTYPE,0),IFNULL(PCFBIP,0) "
		"FROM "
			"TB_MACCESSC "
		"GROUP BY "
			"BRANCHID,PCFTYPE,PCFBIP");

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
		if(i >= MAX_MON_PCF_CNT)
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: TB_MACCESSC PCF COUNT[%d] is over MAX_MON_PCF_CNT[%d] [%s]", __FILE__, __FUNCTION__, __LINE__,
				i, MAX_MON_PCF_CNT, szBuf);
			break;
		}
		pstData->stWatchPCF[i].ucOffice		= (unsigned char)atoi(stRow[0]);
		pstData->stWatchPCF[i].ucPCFType	= (unsigned char)atoi(stRow[1]);
		pstData->stWatchPCF[i].uiIP			= (unsigned int)atoi(stRow[2]);
		i++;
	}
	mysql_free_result(pstRst);
	pstData->dCount = i;

	return 0;
}

int dGetFltEquipList(MYSQL *pstMySQL, st_WatchEquipList *pstData)
{
	int			i;
	char		szBuf[MAX_SQLQUERY_SIZE];
	MYSQL_ROW	stRow;
	MYSQL_RES	*pstRst;

	sprintf(szBuf,
		"SELECT "
			"EquipIp,EquipType,Mon1MinFlag "
		"FROM "
			"TB_MEQUIPMC "
		"WHERE "
			"EquipType IN (6,7,8,9) "
		"ORDER BY "
			"EquipIp");

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

	i		= pstData->dCount;
	while( (stRow = mysql_fetch_row(pstRst)) != NULL)
	{
		if(i >= (MAX_MON_PDSN_CNT+MAX_MON_AAA_CNT+MAX_MON_HSS_CNT+MAX_MON_LNS_CNT))
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: TB_MEQUIPMC COUNT[%d] is over %d [%s]", __FILE__, __FUNCTION__, __LINE__,
				i, (MAX_MON_PDSN_CNT+MAX_MON_AAA_CNT+MAX_MON_HSS_CNT+MAX_MON_LNS_CNT), szBuf);
			mysql_free_result(pstRst);
			return -3;
		}
		pstData->stWatchEquip[i].uiIP	= (unsigned int)atoi(stRow[0]);
		pstData->stWatchEquip[i].dType	= (int)atoi(stRow[1]);
		pstData->stWatchEquip[i].dMon1MinFlag	= (int)atoi(stRow[2]);
		i++;
	}
	mysql_free_result(pstRst);
	pstData->dCount	= i;

	return 0;
}

int dGetFltRoamEquipList(MYSQL *pstMySQL, st_LoamEquipList *pstData)
{
	int			i;
	char		szBuf[MAX_SQLQUERY_SIZE];
	MYSQL_ROW	stRow;
	MYSQL_RES	*pstRst;

	sprintf(szBuf,
		"SELECT "
			"EquipIp,EquipType,NETMASK,MON1MINFLAG "
		"FROM "
			"TB_MEQUIPMC "
		"WHERE "
			"EquipType=%d OR EquipType=%d  OR EquipType>=%d "
		"ORDER BY "
			"EquipIp", LNS_SYSTYPE, SYSTEM_TYPE_ROAMAAA, ROAM_JAPAN);

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

	i		= pstData->dCount;
	while( (stRow = mysql_fetch_row(pstRst)) != NULL)
	{
		if(i >= MAX_ROAM_EQUIP_CNT)
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: TB_MEQUIPMC ROAM COUNT[%d] is over MAX_ROAM_EQUIP_CNT[%d] [%s]", __FILE__, __FUNCTION__, __LINE__,
				i, MAX_ROAM_EQUIP_CNT, szBuf);
			mysql_free_result(pstRst);
			return -3;
		}
		pstData->stRoamEquip[i].uiIP		= (unsigned int)atoi(stRow[0]);
		pstData->stRoamEquip[i].dType		= (int)atoi(stRow[1]);
		pstData->stRoamEquip[i].uiNetMask	= (unsigned int)atoi(stRow[2]);
		pstData->stRoamEquip[i].dMon1MinFlag	= (int)atoi(stRow[3]);
		i++;
	}
	mysql_free_result(pstRst);
	pstData->dCount	= i;

	return 0;
}

int dGetWatchServiceList(MYSQL *pstMySQL, st_WatchServiceList *pstData)
{
	int			i;
	char		szBuf[MAX_SQLQUERY_SIZE];
	MYSQL_ROW	stRow;
	MYSQL_RES	*pstRst;

	sprintf(szBuf,
		"SELECT "
			"IP,IFNULL(L4,0),IFNULL(L7,0) "
		"FROM "
			"FLT_SVR "
		"WHERE "
			"SYSTYPE=%d "
		"ORDER BY "
			"L4,L7,IP", SYSTEM_TYPE_SERVICE);

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
		if(i >= MAX_MON_SVC_CNT)
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: FLT_SVC MAX_MON_SVC_CNT[%d] OVER[%d] [%s]", __FILE__, __FUNCTION__, __LINE__,
				MAX_MON_SVC_CNT, i, szBuf);
			break;
		}
		pstData->stWatchService[i].uiIP			= (unsigned int)atoi(stRow[0]);
		pstData->stWatchService[i].dSvcL4Type	= (int)atoi(stRow[1]);
		pstData->stWatchService[i].dSvcL7Type	= (int)atoi(stRow[2]);
		i++;
	}
	mysql_free_result(pstRst);
	pstData->dCount = i;

	return 0;
}

int dGetSvcInfo(MYSQL *pstMySQL, pst_SvcMmc pstData, int *dCnt, char *sWhere)
{
	int			i;
	char		szBuf[MAX_SQLQUERY_SIZE];
	MYSQL_ROW	stRow;
	MYSQL_RES	*pstRst;

	if(sWhere)
	{
		sprintf(szBuf,
			"SELECT "
				"IP,PORT,FLAG,IFNULL(SYSTYPE,0),IFNULL(L4,0),IFNULL(L7,0),IFNULL(APPCODE,0),IFNULL(`DESC`,'-') "
			"FROM "
				"FLT_SVR "
			"%s"
			"ORDER BY "
				"L4,L7,IP,PORT,FLAG", sWhere);
	}
	else
	{
		sprintf(szBuf,
			"SELECT "
				"IP,PORT,FLAG,IFNULL(SYSTYPE,0),IFNULL(L4,0),IFNULL(L7,0),IFNULL(APPCODE,0),IFNULL(`DESC`,'-') "
			"FROM "
				"FLT_SVR "
			"ORDER BY "
				"L4,L7,IP,PORT,FLAG");
	}

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
		if(i >= MAX_SVR_CNT)
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: FLT_SVC MAX_SVR_CNT[%d] OVER[%d] [%s]", __FILE__, __FUNCTION__, __LINE__,
				MAX_SVR_CNT, i, szBuf);
			break;
		}

		pstData[i].uSvcIP		= (unsigned int)atoi(stRow[0]);
		pstData[i].huPort		= (unsigned short)atoi(stRow[1]);
		pstData[i].cFlag		= (unsigned short)atoi(stRow[2]);
		pstData[i].cSysType		= (unsigned short)atoi(stRow[3]);

		pstData[i].huL4Code		= (unsigned short)atoi(stRow[4]);
		pstData[i].huL7Code		= (unsigned short)atoi(stRow[5]);
		pstData[i].huAppCode	= (unsigned short)atoi(stRow[6]);

		if(strlen(stRow[7]) < MAX_SDESC)
			strncpy(pstData[i].szDesc, stRow[7], MAX_SDESC);
		else
		{
			strncpy(pstData[i].szDesc, stRow[7], MAX_SDESC-1);
			pstData[i].szDesc[MAX_SDESC-1] = 0x00;
		}
		i++;
	}
	mysql_free_result(pstRst);
	*dCnt = i;

	return 0;
}

int dGetSCTPInfo(MYSQL *pstMySQL, st_SCTP_DB *pstData, int *dCnt, char *sWhere)
{
	int			i;
	char		szBuf[MAX_SQLQUERY_SIZE];
	MYSQL_ROW	stRow;
	MYSQL_RES	*pstRst;

	if(sWhere)
	{
		sprintf(szBuf,
			"SELECT "
				"IP,IFNULL(SYSTYPE,0),IFNULL(DIRECTION,0),IFNULL(GROUPID,0),IFNULL(`DESC`,'-') "
			"FROM "
				"FLT_SCTP "
			"%s"
			"ORDER BY "
				"IP", sWhere);
	}
	else
	{
		sprintf(szBuf,
			"SELECT "
				"IP,IFNULL(SYSTYPE,0),IFNULL(DIRECTION,0),IFNULL(GROUPID,0),IFNULL(`DESC`,'-') "
			"FROM "
				"FLT_SCTP "
			"ORDER BY "
				"IP");
	}

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
		if(i >= MAX_MON_SVC_CNT)
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: FLT_SVC MAX_MON_SVC_CNT[%d] OVER[%d] [%s]", __FILE__, __FUNCTION__, __LINE__,
				MAX_MON_SVC_CNT, i, szBuf);
			break;
		}

		pstData[i].uSCTPIP		= (unsigned int)atoi(stRow[0]);
		pstData[i].cSysType		= (unsigned short)atoi(stRow[1]);
		pstData[i].cDirection		= (unsigned short)atoi(stRow[2]);
		pstData[i].huGroupID		= (unsigned short)atoi(stRow[3]);

		if(strlen(stRow[4]) < MAX_SDESC)
			strncpy(pstData[i].szDesc, stRow[4], MAX_SDESC);
		else
		{
			strncpy(pstData[i].szDesc, stRow[4], MAX_SDESC-1);
			pstData[i].szDesc[MAX_SDESC-1] = 0x00;
		}
		i++;
	}
	mysql_free_result(pstRst);
	*dCnt = i;

	return 0;
}


int dInsertSvcInfo(MYSQL *pstMySQL, st_SvcInfo *stData)
{
	char		szBuf[MAX_SQLQUERY_SIZE];

	sprintf(szBuf, "INSERT INTO FLT_SVR VALUES (%u,%hu,%hu,%hu,%hu,%hu,%hu,'%s')",
		stData->uSvcIP, stData->huPort, stData->cFlag, stData->cSysType, stData->huL4Code, stData->huL7Code, stData->huAppCode, stData->szDesc);

	if(mysql_query(pstMySQL, szBuf) != 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_query(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			szBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
		if(mysql_errno(pstMySQL) == DB_DUPLICATED_ENTRY)
			return -1;
		else
			return -2;
	}

	return 0;
}

int dInsertSCTPInfo(MYSQL *pstMySQL, st_SCTP_DB *stData)
{
	char		szBuf[MAX_SQLQUERY_SIZE];

	sprintf(szBuf, "INSERT INTO FLT_SCTP VALUES (%u,%hu,%hu,%hu,'%s')",
		stData->uSCTPIP, stData->cSysType, stData->cDirection, stData->huGroupID, stData->szDesc);

	if(mysql_query(pstMySQL, szBuf) != 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_query(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			szBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
		if(mysql_errno(pstMySQL) == DB_DUPLICATED_ENTRY)
			return -1;
		else
			return -2;
	}

	return 0;
}


int dGetSvcInfoCount(MYSQL *pstMySQL, int *dCnt)
{
	int			dResult;
	char		szBuf[MAX_SQLQUERY_SIZE];
	MYSQL_ROW	stRow;
	MYSQL_RES	*pstRst;

	sprintf(szBuf, "SELECT COUNT(*) FROM FLT_SVR");

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

	dResult	= 0;
	while( (stRow = mysql_fetch_row(pstRst)) != NULL)
	{
		dResult	= atoi(*stRow);
		if(dResult >= MAX_SVR_CNT)
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: FLT_SVC MAX_SVR_CNT[%d] OVER[%d] [%s]", __FILE__, __FUNCTION__, __LINE__,
				MAX_SVR_CNT, dResult, szBuf);
			break;
		}
	}
	mysql_free_result(pstRst);
	*dCnt = dResult;

	return 0;
}

int dGetSCTPInfoCount(MYSQL *pstMySQL, int *dCnt)
{
	int			dResult;
	char		szBuf[MAX_SQLQUERY_SIZE];
	MYSQL_ROW	stRow;
	MYSQL_RES	*pstRst;

	sprintf(szBuf, "SELECT COUNT(*) FROM FLT_SCTP");

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

	dResult	= 0;
	if( (stRow = mysql_fetch_row(pstRst)) != NULL)
	{
		dResult	= atoi(*stRow);
		if(dResult >= MAX_SCTP_COUNT)
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: FLT_SCTP MAX_SCTP_COUNT[%d] OVER[%d] [%s]", __FILE__, __FUNCTION__, __LINE__,
				MAX_SCTP_COUNT, dResult, szBuf);
		}
	}
	mysql_free_result(pstRst);
	*dCnt = dResult;

	return 0;
}

int dGetCountSVC(MYSQL *pstMySQL, unsigned int uiIP, unsigned short usPort, unsigned short huFlag)
{
	char		szBuf[MAX_SQLQUERY_SIZE];
	int			dResult;
	MYSQL_ROW	stRow;
	MYSQL_RES	*pstRst;

	sprintf(szBuf, "SELECT COUNT(*) FROM FLT_SVR WHERE IP = %u AND PORT = %hu AND FLAG = %hu", uiIP, usPort, huFlag);

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

	dResult	= 0;
	if( (stRow = mysql_fetch_row(pstRst)) != NULL)
		dResult	= atoi(*stRow);

	mysql_free_result(pstRst);

	return dResult;
}

int dDeleteSvc(MYSQL *pstMySQL, unsigned int uiIP, unsigned short usPort, unsigned short huFlag)
{
	char		szBuf[MAX_SQLQUERY_SIZE];

	sprintf(szBuf, "DELETE FROM FLT_SVR WHERE IP = %u AND PORT = %hu AND FLAG = %hu", uiIP, usPort, huFlag);

	if(mysql_query(pstMySQL, szBuf) != 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_query(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			szBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
		return -1;
	}

	return 0;
}

int dGetCountSCTPIP(MYSQL *pstMySQL, unsigned int uiIP)
{
	char		szBuf[MAX_SQLQUERY_SIZE];
	int			dResult;
	MYSQL_ROW	stRow;
	MYSQL_RES	*pstRst;

	sprintf(szBuf, "SELECT COUNT(*) FROM FLT_SCTP WHERE IP = %u", uiIP);

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

	dResult	= 0;
	if( (stRow = mysql_fetch_row(pstRst)) != NULL)
		dResult	= atoi(*stRow);

	mysql_free_result(pstRst);

	return dResult;
}

int dDeleteSCTP(MYSQL *pstMySQL, unsigned int uiIP)
{
	char		szBuf[MAX_SQLQUERY_SIZE];

	sprintf(szBuf, "DELETE FROM FLT_SCTP WHERE IP = %u", uiIP);

	if(mysql_query(pstMySQL, szBuf) != 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_query(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			szBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
		return -1;
	}

	return 0;
}

int dGetEquipSe(MYSQL *pstMySQL, st_EQUIP_SE *pstData, int *dCnt, char *szWhere)
{
	int			i;
	char		szBuf[MAX_SQLQUERY_SIZE];
	MYSQL_ROW	stRow;
	MYSQL_RES	*pstRst;
	
	sprintf(szBuf,
	"SELECT IP, SysNo, SysType, IFNULL(Name,'-'), IFNULL('Description','-') "
	"FROM   TB_EQUIP_SE"
	"%s"
	, szWhere);

	if( mysql_query(pstMySQL, szBuf) != 0 ){
		log_print(LOGN_CRI,"%s:%s:%d] FAILED IN mysql_query(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, 
			szBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
		return -1;
	}

	if( (pstRst = mysql_store_result(pstMySQL)) == NULL ){
		log_print(LOGN_CRI, "%s:%s:%d] FAILED IN mysql_store_result(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			szBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
		return -2;
	}

	i = 0;
	while( (stRow = mysql_fetch_row(pstRst) ) != NULL ){
		if( i > MAX_MNIP_COUNT ){
			log_print(LOGN_CRI,"%s:%s:%d] EQUIP_SE COUNT (%d) OVER(%d) [query=%s]", __FILE__, __FUNCTION__, __LINE__, i, MAX_MNIP_COUNT, szBuf );
			break;
		}	
		pstData[i].uiIP = (int)atoi(stRow[0]);
		pstData[i].dSysNo = (unsigned int)atoi(stRow[1]);
		pstData[i].chSysType = (unsigned char)atoi(stRow[2]);
		strncpy( pstData[i].szName, stRow[3], MAX_PDSNNAME );
		pstData[i].szName[MAX_PDSNNAME] = 0x00;
		strncpy( pstData[i].szDesc, stRow[4], MAX_SDESC);
		pstData[i].szDesc[MAX_SDESC] = 0x00;
	
		i++;
    }
    mysql_free_result(pstRst);
    *dCnt = i;

    return 0;
}

int dGetNAS(MYSQL *pstMySQL, st_NAS_db  *pstData, int *dCnt, char *sWhere)
{
	int			i;
	char		szBuf[MAX_SQLQUERY_SIZE];
	MYSQL_ROW	stRow;
	MYSQL_RES	*pstRst;

	if(sWhere){
		sprintf(szBuf,
			"SELECT "
				"IP, NETMASK, FLAG, IFNULL(SYSTYPE,'-'), IFNULL(`DESC`,'-') "
	        "FROM "
	        	"FLT_CLT"
	        "%s",
	        sWhere);
	}else{
		sprintf(szBuf,
			"SELECT "
				"IP, NETMASK, FLAG, IFNULL(SYSTYPE,'-'), IFNULL(`DESC`,'-') "
	        "FROM "
	        	"FLT_CLT");
	}

	if(mysql_query(pstMySQL, szBuf) != 0){
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_query(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			szBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
		return -1;
	}

	if( (pstRst = mysql_store_result(pstMySQL)) == NULL){
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_store_result(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			szBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
		return -2;
	}

	i		= 0;
	while( (stRow = mysql_fetch_row(pstRst)) != NULL){
		if(i >= MAX_MNIP_COUNT){
			log_print(LOGN_CRI, "F=%s:%s.%d: FLT_CLT MAX_MNIP_COUNT[%d] OVER[%d] [%s]", __FILE__, __FUNCTION__, __LINE__,
				MAX_MNIP_COUNT, i, szBuf);
			break;
		}

		pstData[i].uMNIP		= (unsigned int)atoi(stRow[0]);
		pstData[i].usNetMask	= (unsigned short)atoi(stRow[1]);
		pstData[i].cFlag		= (unsigned char)atoi(stRow[2]);
		pstData[i].cSysType		= (unsigned char)atoi(stRow[3]);

		if(strlen(stRow[4]) < MAX_SDESC)
			strncpy(pstData[i].szDesc, stRow[4], MAX_SDESC);
		else{
			strncpy(pstData[i].szDesc, stRow[4], MAX_SDESC-1);
			pstData[i].szDesc[MAX_SDESC-1] = 0x00;
		}
		i++;
	}
	mysql_free_result(pstRst);
	*dCnt = i;

	return 0;
}

int dInsertMNIP(MYSQL *pstMySQL, st_NAS_db *stData)
{
	char		szBuf[MAX_SQLQUERY_SIZE];

	sprintf(szBuf, "INSERT INTO FLT_CLT VALUES (%u,%hu,%hu,%hu,'%s')",
		stData->uMNIP, stData->usNetMask, stData->cFlag, stData->cSysType, stData->szDesc);

	if(mysql_query(pstMySQL, szBuf) != 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_query(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			szBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
		if(mysql_errno(pstMySQL) == DB_DUPLICATED_ENTRY)
			return -1;
		else
			return -2;
	}

	return 0;
}

int dGetMNIPCount(MYSQL *pstMySQL, int *dCnt)
{
	int			dResult;
	char		szBuf[MAX_SQLQUERY_SIZE];
	MYSQL_ROW	stRow;
	MYSQL_RES	*pstRst;

	sprintf(szBuf, "SELECT COUNT(*) FROM FLT_CLT");

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

	dResult	= 0;
	while( (stRow = mysql_fetch_row(pstRst)) != NULL)
	{
		dResult	= atoi(*stRow);
		if(dResult >= MAX_MNIP_COUNT)
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: FLT_IPPOOL MAX_MNIP_COUNT[%d] OVER[%d] [%s]", __FILE__, __FUNCTION__, __LINE__, MAX_MNIP_COUNT, dResult, szBuf);
			break;
		}
	}
	mysql_free_result(pstRst);
	*dCnt = dResult;

	return 0;
}

int dGetCountMNIP(MYSQL *pstMySQL, unsigned int uMNIP, unsigned short huNetmask, unsigned short huFlag)
{
	char		szBuf[MAX_SQLQUERY_SIZE];
	int			dResult;
	MYSQL_ROW	stRow;
	MYSQL_RES	*pstRst;

	sprintf(szBuf, "SELECT COUNT(*) FROM FLT_CLT WHERE IP=%u AND NETMASK=%hu AND FLAG=%hu", uMNIP, huNetmask, huFlag);

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

	dResult	= 0;
	if( (stRow = mysql_fetch_row(pstRst)) != NULL)
		dResult	= atoi(*stRow);

	mysql_free_result(pstRst);

	return dResult;
}


int dDeleteMNIP(MYSQL *pstMySQL, unsigned int uMNIP, unsigned short huNetmask, unsigned short huFlag)
{
	char		szBuf[MAX_SQLQUERY_SIZE];

	sprintf(szBuf, "DELETE FROM FLT_CLT WHERE IP=%u AND NETMASK=%hu AND FLAG=%hu", uMNIP, huNetmask, huFlag);

	if(mysql_query(pstMySQL, szBuf) != 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_query(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			szBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
		return -1;
	}

	return 0;
}

int dGetEquipInfo(MYSQL *pstMySQL, st_InfoEquip_MMC *pstData, int *dCnt)
{
	int			i;
	size_t		szDesc;
	char		szBuf[MAX_SQLQUERY_SIZE];
	MYSQL_ROW	stRow;
	MYSQL_RES	*pstRst;

	sprintf(szBuf,
		"SELECT "
			"EquipIp,IFNULL(EquipType,0),IFNULL(EquipTypeName,0),IFNULL(EquipName,'-'),IFNULL(Description,'-'),IFNULL(NETMASK,0),IFNULL(Mon1MinFlag,0) "
		"FROM "
			"TB_MEQUIPMC "
		"ORDER BY "
			"EquipIp, EquipType");

	if(mysql_query(pstMySQL, szBuf) != 0){
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_query(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			szBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
		return -1;
	}

	if( (pstRst = mysql_store_result(pstMySQL)) == NULL){
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_store_result(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			szBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
		return -2;
	}

	i		= 0;
	szDesc	= sizeof(pstData[0].szDesc);
	while( (stRow = mysql_fetch_row(pstRst)) != NULL){

		if(i >= MAX_EQUIP_INFO){
			log_print(LOGN_CRI, "F=%s:%s.%d: TB_MEQUIPMC MAX_EQUIP_INFO[%d] OVER[%d] [%s]", __FILE__, __FUNCTION__, __LINE__,
				MAX_EQUIP_INFO, i, szBuf);
			break;
		}

		pstData[i].uEquipIP		= (unsigned int)atoi(stRow[0]);
		pstData[i].cEquipType	= (unsigned char)atoi(stRow[1]);

		if(strlen(stRow[2]) < DEF_EQUIPNAME)
			strncpy(pstData[i].szEquipTypeName, stRow[2], DEF_EQUIPNAME);
		else{
			strncpy(pstData[i].szEquipTypeName, stRow[2], DEF_EQUIPNAME-1);
			pstData[i].szEquipTypeName[DEF_EQUIPNAME-1] = 0x00;
		}

		if(strlen(stRow[3]) < DEF_EQUIPNAME)
			strncpy(pstData[i].szEquipName, stRow[3], DEF_EQUIPNAME);
		else{
			strncpy(pstData[i].szEquipName, stRow[3], DEF_EQUIPNAME-1);
			pstData[i].szEquipName[DEF_EQUIPNAME-1] = 0x00;
		}

		if(strlen(stRow[4]) < MAX_SDESC)
			strncpy(pstData[i].szDesc, stRow[4], MAX_SDESC);
		else{
			strncpy(pstData[i].szDesc, stRow[4], MAX_SDESC-1);
			pstData[i].szDesc[szDesc-1] = 0x00;
		}

		pstData[i].uNetmask	    = (unsigned char)atoi(stRow[5]);
		pstData[i].cMon1MinFlag = (char)atoi(stRow[6]);
		i++;
	}
	mysql_free_result(pstRst);
	*dCnt = i;

	return 0;
}

int dGetInfoEquipCount(MYSQL *pstMySQL, int *dCnt)
{
	int			dResult;
	char		szBuf[MAX_SQLQUERY_SIZE];
	MYSQL_ROW	stRow;
	MYSQL_RES	*pstRst;

	sprintf(szBuf, "SELECT COUNT(*) FROM TB_MEQUIPMC");

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

	dResult	= 0;
	while( (stRow = mysql_fetch_row(pstRst)) != NULL)
		dResult	= atoi(*stRow);

	mysql_free_result(pstRst);
	*dCnt = dResult;

	return 0;
}

int dDeleteEquip(MYSQL *pstMySQL, unsigned int uiIP)
{
	char		szBuf[MAX_SQLQUERY_SIZE];

	sprintf(szBuf, "DELETE FROM TB_MEQUIPMC WHERE EquipIp=%u", uiIP);

	if(mysql_query(pstMySQL, szBuf) != 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_query(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			szBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
		return -1;
	}

	return 0;
}

int dGetWatchInfoMonThreshold(MYSQL *pstMySQL, st_WatchThresholdList *pstData)
{
	int			i;
	char		szBuf[MAX_SQLQUERY_SIZE];
	MYSQL_ROW	stRow;
	MYSQL_RES	*pstRst;
//dcham 20110616
	sprintf(szBuf,
		"SELECT "
			"BranchID,SvcType,AlarmType,SvcIP,IFNULL(StartTime,0),IFNULL(GapTime,0),IFNULL(DayRate,0),IFNULL(NightRate,0),"
			"IFNULL(DayMinTrial,0),IFNULL(NightMinTrial,0),IFNULL(PeakTrial,0) "
		"FROM "
			"INFO_MON_THRESHOLD "
		"ORDER BY "
			"BranchID,SvcType,AlarmType,SvcIP");

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
		if(i >= MAX_MON_THRESHOLD_COUNT)
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: INFO_MON_THRESHOLD MAX_MON_THRESHOLD_COUNT[%d] OVER[%d] [%s]", __FILE__, __FUNCTION__, __LINE__,
				MAX_MON_THRESHOLD_COUNT, i, szBuf);
			break;
		}

		pstData->stWatchThreshold[i].ucOffice				= (unsigned char)atoi(stRow[0]);
		pstData->stWatchThreshold[i].ucSysType				= (unsigned char)atoi(stRow[1]);
		pstData->stWatchThreshold[i].ucAlarmType			= (unsigned char)atoi(stRow[2]);
		pstData->stWatchThreshold[i].uiIP       			= (unsigned int)atoi(stRow[3]); //dcham 20110616
		pstData->stWatchThreshold[i].ucStartTime			= (unsigned char)atol(stRow[4]);

		pstData->stWatchThreshold[i].ucRange				= (unsigned char)atoi(stRow[5]);
		pstData->stWatchThreshold[i].ucDayTimeRate			= (unsigned char)atoi(stRow[6]);
		pstData->stWatchThreshold[i].ucNightTimeRate		= (unsigned char)atoi(stRow[7]);
		pstData->stWatchThreshold[i].uiDayTimeMinTrial		= (unsigned int)atol(stRow[8]);
		pstData->stWatchThreshold[i].uiNightTimeMinTrial	= (unsigned int)atol(stRow[9]);
		pstData->stWatchThreshold[i].uiPeakTrial			= (unsigned int)atol(stRow[10]);
		i++;
	}
	mysql_free_result(pstRst);
	pstData->dCount = i;

	return 0;

}

int dGetMONThres(MYSQL *pstMySQL, st_MON_ThresMMC *pstData, int *dCnt, char *sWhere)
{
	int			i;
	char		szBuf[MAX_SQLQUERY_SIZE];
	MYSQL_ROW	stRow;
	MYSQL_RES	*pstRst;
//20110616 dcham 
	if(sWhere)
	{
		sprintf(szBuf,
			"SELECT "
				"BranchID,SvcType,AlarmType,SvcIP, IFNULL(StartTime,0),IFNULL(GapTime,0),IFNULL(DayRate,0),IFNULL(NightRate,0),"
				"IFNULL(DayMinTrial,0),IFNULL(NightMinTrial,0),IFNULL(PeakTrial,0),IFNULL(DESCRIPTION,'-') "
			"FROM "
				"INFO_MON_THRESHOLD "
			"%s"
			"ORDER BY "
				"BranchID,SvcType,AlarmType,SvcIP", sWhere);
	}
	else
	{
		sprintf(szBuf,
			"SELECT "
				"BranchID,SvcType,AlarmType,SvcIP,IFNULL(StartTime,0),IFNULL(GapTime,0),IFNULL(DayRate,0),IFNULL(NightRate,0),"
				"IFNULL(DayMinTrial,0),IFNULL(NightMinTrial,0),IFNULL(PeakTrial,0),IFNULL(DESCRIPTION,'-') "
			"FROM "
				"INFO_MON_THRESHOLD "
			"ORDER BY "
				"BranchID,SvcType,AlarmType,SvcIP");
	}

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
		if(i >= MAX_MON_THRESHOLD_COUNT)
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: INFO_MON_THRESHOLD MAX_MON_THRESHOLD_COUNT[%d] OVER[%d] [%s]", __FILE__, __FUNCTION__, __LINE__,
				MAX_MON_THRESHOLD_COUNT, i, szBuf);
			break;
		}
//dcham 20110616
		pstData[i].huBranchID		= (unsigned short)atoi(stRow[0]);
		pstData[i].cSvcType			= (unsigned char)atoi(stRow[1]);
		pstData[i].cAlarmType		= (unsigned char)atoi(stRow[2]);
		pstData[i].dSvcIP           = (unsigned int)atoi(stRow[3]);
		pstData[i].cStartHour		= (unsigned char)atol(stRow[4]);

		pstData[i].cDayRange		= (unsigned char)atoi(stRow[5]);
		pstData[i].huDayRate		= (unsigned short)atoi(stRow[6]);
		pstData[i].huNightRate		= (unsigned short)atoi(stRow[7]);
		pstData[i].uDayMinTrial		= (unsigned int)atol(stRow[8]);
		pstData[i].uNigthMinTrial	= (unsigned int)atol(stRow[9]);
		pstData[i].uPeakTrial		= (unsigned int)atol(stRow[10]);

		if(strlen(stRow[11]) < MAX_SDESC)
			strncpy(pstData[i].szDesc, stRow[11], MAX_SDESC);
		else
		{
			strncpy(pstData[i].szDesc, stRow[11], MAX_SDESC-1);
			pstData[i].szDesc[MAX_SDESC-1] = 0x00;
		}
		i++;
	}
	mysql_free_result(pstRst);
	*dCnt = i;

	return 0;
}

int dGetWatchInfoDefectThreshold(MYSQL *pstMySQL, st_DefectThresholdList *pstData)
{
	int			i;
	char		szBuf[MAX_SQLQUERY_SIZE];
	MYSQL_ROW	stRow;
	MYSQL_RES	*pstRst;

	sprintf(szBuf,
		"SELECT "
			"SvcType,IFNULL(TCPSetupTime,0),IFNULL(ResponseTime,0),IFNULL(UpThroughput,0),IFNULL(DnThroughput,0),IFNULL(UpRetransCount,0),"
			"IFNULL(DnRetransCount,0),IFNULL(UpJitter,0),IFNULL(DnJitter,0),IFNULL(UpPacketLoss,0),IFNULL(DnPacketLoss,0) "
		"FROM "
			"INFO_DEFECT_THRESHOLD "
		"ORDER BY "
			"SvcType");

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
		if(i >= MAX_DEFECT_THRES)
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: INFO_DEFFECT_THRESHOLD MAX_DEFECT_THRES[%d] OVER[%d] [%s]", __FILE__, __FUNCTION__, __LINE__,
				MAX_DEFECT_THRES, i, szBuf);
			break;
		}

		pstData->stDefectThreshold[i].uiSvcType			= (unsigned int)atoi(stRow[0]);
		pstData->stDefectThreshold[i].uiTCPSetupTime	= (unsigned int)atoi(stRow[1]);
		pstData->stDefectThreshold[i].uiResponseTime	= (unsigned int)atoi(stRow[2]);
		pstData->stDefectThreshold[i].uiUpThroughput	= (unsigned int)atoi(stRow[3]);
		pstData->stDefectThreshold[i].uiDnThroughput	= (unsigned int)atoi(stRow[4]);
		pstData->stDefectThreshold[i].uiUpRetransCnt	= (unsigned int)atoi(stRow[5]);
		pstData->stDefectThreshold[i].uiDnRetransCnt	= (unsigned int)atoi(stRow[6]);
		pstData->stDefectThreshold[i].UpJitter			= (unsigned int)atoi(stRow[7]);
		pstData->stDefectThreshold[i].DnJitter			= (unsigned int)atoi(stRow[8]);
		pstData->stDefectThreshold[i].UpPacketLoss		= (unsigned int)atoi(stRow[9]);
		pstData->stDefectThreshold[i].DnPacketLoss		= (unsigned int)atoi(stRow[10]);
		i++;
	}
	mysql_free_result(pstRst);
	pstData->dCount = i;

	return 0;
}

int dGetThres(MYSQL *pstMySQL, st_ThresMMC *pstData, int *dCnt, char *sWhere)
{
	int			i;
	char		szBuf[MAX_SQLQUERY_SIZE];
	MYSQL_ROW	stRow;
	MYSQL_RES	*pstRst;

	if(sWhere)
	{
		sprintf(szBuf,
			"SELECT "
				"SvcType,IFNULL(TCPSetupTime,0),IFNULL(ResponseTime,0),IFNULL(UpThroughput,0),IFNULL(DnThroughput,0),IFNULL(UpRetransCount,0),"
				"IFNULL(DnRetransCount,0),IFNULL(UpJitter,0),IFNULL(DnJitter,0),IFNULL(UpPacketLoss,0),IFNULL(DnPacketLoss,0),IFNULL(Description,'-') "
			"FROM "
				"INFO_DEFECT_THRESHOLD "
			"%s"
			"ORDER BY "
				"SvcType", sWhere);
	}
	else
	{
		sprintf(szBuf,
			"SELECT "
				"SvcType,IFNULL(TCPSetupTime,0),IFNULL(ResponseTime,0),IFNULL(UpThroughput,0),IFNULL(DnThroughput,0),IFNULL(UpRetransCount,0),"
				"IFNULL(DnRetransCount,0),IFNULL(UpJitter,0),IFNULL(DnJitter,0),IFNULL(UpPacketLoss,0),IFNULL(DnPacketLoss,0),IFNULL(Description,'-') "
			"FROM "
				"INFO_DEFECT_THRESHOLD "
			"ORDER BY "
				"SvcType");
	}

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
		if(i >= MAX_DEFECT_THRES)
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: INFO_DEFFECT_THRESHOLD MAX_DEFECT_THRES[%d] OVER[%d] [%s]", __FILE__, __FUNCTION__, __LINE__,
				MAX_DEFECT_THRES, i, szBuf);
			break;
		}

		pstData[i].cSvcType			= (unsigned char)atoi(stRow[0]);
		pstData[i].uTCPSetupTime	= (unsigned int)atoi(stRow[1]);
		pstData[i].uResponseTime	= (unsigned int)atoi(stRow[2]);
		pstData[i].uUpThroughput	= (unsigned int)atoi(stRow[3]);
		pstData[i].uDnThroughput	= (unsigned int)atoi(stRow[4]);
		pstData[i].uUpRetransCount	= (unsigned int)atoi(stRow[5]);
		pstData[i].uDnRetransCount	= (unsigned int)atoi(stRow[6]);
		pstData[i].uUpJitter		= (unsigned int)atoi(stRow[7]);
		pstData[i].uDnJitter		= (unsigned int)atoi(stRow[8]);
		pstData[i].uUpPacketLoss	= (unsigned int)atoi(stRow[9]);
		pstData[i].uDnPacketLoss	= (unsigned int)atoi(stRow[10]);

		if(strlen(stRow[11]) < MAX_SDESC)
			strncpy(pstData[i].szDesc, stRow[11], MAX_SDESC);
		else
		{
			strncpy(pstData[i].szDesc, stRow[11], MAX_SDESC-1);
			pstData[i].szDesc[MAX_SDESC-1] = 0x00;
		}
		i++;
	}
	mysql_free_result(pstRst);
	*dCnt = i;

	return 0;
}

int dSelectThres(MYSQL *pstMySQL, st_ThresMMC *pstData)
{
	int			i;
	char		szBuf[MAX_SQLQUERY_SIZE];
	MYSQL_ROW	stRow;
	MYSQL_RES	*pstRst;

	sprintf(szBuf,
		"SELECT "
			"TCPSetupTime,ResponseTime,UpThroughput,DnThroughput,UpRetransCount,"
			"DnRetransCount,UpJitter,DnJitter,UpPacketLoss,DnPacketLoss,Description "
		"FROM "
			"INFO_DEFECT_THRESHOLD "
		"WHERE "
			"SvcType=%hu", pstData->cSvcType);

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
		if(i >= 1)
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: INFO_DEFECT_THRESHOLD SvcType[%hu] IS DUPLICATED.[%s]", __FILE__, __FUNCTION__, __LINE__,
				pstData->cSvcType, szBuf);
			mysql_free_result(pstRst);
			return -3;
		}

		pstData->uTCPSetupTime		= (unsigned int)atoi(stRow[0]);
		pstData->uResponseTime		= (unsigned int)atoi(stRow[1]);
		pstData->uUpThroughput		= (unsigned int)atoi(stRow[2]);
		pstData->uDnThroughput		= (unsigned int)atoi(stRow[3]);
		pstData->uUpRetransCount	= (unsigned int)atoi(stRow[4]);
		pstData->uDnRetransCount	= (unsigned int)atoi(stRow[5]);
		pstData->uUpJitter			= (unsigned int)atoi(stRow[6]);
		pstData->uDnJitter			= (unsigned int)atoi(stRow[7]);
		pstData->uUpPacketLoss		= (unsigned int)atoi(stRow[8]);
		pstData->uDnPacketLoss		= (unsigned int)atoi(stRow[9]);

		if(strlen(stRow[10]) < MAX_SDESC)
			strncpy(pstData->szDesc, stRow[10], MAX_SDESC);
		else
		{
			strncpy(pstData->szDesc, stRow[10], MAX_SDESC-1);
			pstData->szDesc[MAX_SDESC-1] = 0x00;
		}
		i++;
	}
	mysql_free_result(pstRst);

	if(i == 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: INFO_DEFECT_THRESHOLD SvcType[%hu] IS NOT FOUND.[%s]", __FILE__, __FUNCTION__, __LINE__,
			pstData->cSvcType, szBuf);
		return -4;
	}

	return 0;
}

int dChgThres(MYSQL *pstMySQL, st_ThresMMC *pstData)
{
	char		szBuf[MAX_SQLQUERY_SIZE];

	sprintf(szBuf,
		"UPDATE INFO_DEFECT_THRESHOLD "
			"SET TCPSetupTime=%u,ResponseTime=%u,UpThroughput=%u,DnThroughput=%u,UpRetransCount=%u,"
			"DnRetransCount=%u,UpJitter=%u,DnJitter=%u,UpPacketLoss=%u,DnPacketLoss=%u,Description='%s' "
		"WHERE "
			"SvcType=%hu",
		pstData->uTCPSetupTime, pstData->uResponseTime, pstData->uUpThroughput, pstData->uDnThroughput,
		pstData->uUpRetransCount, pstData->uDnRetransCount, pstData->uUpJitter, pstData->uDnJitter,
		pstData->uUpPacketLoss, pstData->uDnPacketLoss, pstData->szDesc, pstData->cSvcType);

	if(mysql_query(pstMySQL, szBuf) != 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_query(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			szBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
		return -1;
	}

	return 0;
}

int dSelectMONThres(MYSQL *pstMySQL, st_MON_ThresMMC *pstData)
{
	int			i;
	char		szBuf[MAX_SQLQUERY_SIZE];
	MYSQL_ROW	stRow;
	MYSQL_RES	*pstRst;
//20110616 dcham
	sprintf(szBuf,
		"SELECT "
			"StartTime,GapTime,DayRate,NightRate,DayMinTrial,NightMinTrial,PeakTrial,Description "
		"FROM "
			"INFO_MON_THRESHOLD "
		"WHERE "
			"BranchID=%hu AND SvcType=%hu AND AlarmType=%hu AND SvcIP=%u", pstData->huBranchID, pstData->cSvcType, pstData->cAlarmType,pstData->dSvcIP);

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
		if(i >= 1)
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: INFO_MON_THRESHOLD BranchID[%hu] SvcType[%hu] AlarmType[%hu] SvcIP[%u] IS DUPLICATED.[%s]", __FILE__, __FUNCTION__, __LINE__,
				pstData->huBranchID, pstData->cSvcType, pstData->cAlarmType ,pstData->dSvcIP, szBuf);
			mysql_free_result(pstRst);
			return -3;
		}

		pstData->cStartHour		= (unsigned char)atoi(stRow[0]);
		pstData->cDayRange		= (unsigned char)atoi(stRow[1])+pstData->cStartHour;
		pstData->huDayRate		= (unsigned short)atoi(stRow[2]);
		pstData->huNightRate	= (unsigned short)atoi(stRow[3]);
		pstData->uDayMinTrial	= (unsigned int)atoi(stRow[4]);
		pstData->uNigthMinTrial	= (unsigned int)atoi(stRow[5]);
		pstData->uPeakTrial		= (unsigned int)atoi(stRow[6]);

		if(strlen(stRow[7]) < MAX_SDESC)
			strncpy(pstData->szDesc, stRow[7], MAX_SDESC);
		else
		{
			strncpy(pstData->szDesc, stRow[7], MAX_SDESC-1);
			pstData->szDesc[MAX_SDESC-1] = 0x00;
		}
		i++;
	}
	mysql_free_result(pstRst);

	if(i == 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: INFO_MON_THRESHOLD BranchID[%hu] SvcType[%hu] AlarmType[%hu] SvcIP[%u] IS NOT FOUND.[%s]", __FILE__, __FUNCTION__, __LINE__,
			pstData->huBranchID, pstData->cSvcType, pstData->cAlarmType, pstData->dSvcIP, szBuf);
		return -4;
	}

	return 0;
}

int dChgMONThres(MYSQL *pstMySQL, st_MON_ThresMMC *pstData)
{
	char		szBuf[MAX_SQLQUERY_SIZE];
//dcham 20110616
	sprintf(szBuf,
		"UPDATE INFO_MON_THRESHOLD "
			"SET StartTime=%hu,GapTime=%hu,DayRate=%hu,NightRate=%hu,DayMinTrial=%u,NightMinTrial=%u,PeakTrial=%u,Description='%s' "
		"WHERE "
			"BranchID=%hu AND SvcType=%hu AND AlarmType=%hu AND SvcIP=%u",
		pstData->cStartHour, pstData->cDayRange, pstData->huDayRate, pstData->huNightRate, pstData->uDayMinTrial, pstData->uNigthMinTrial,
		pstData->uPeakTrial, pstData->szDesc, pstData->huBranchID, pstData->cSvcType, pstData->cAlarmType, pstData->dSvcIP);

	if(mysql_query(pstMySQL, szBuf) != 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_query(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			szBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
		return -1;
	}

	return 0;
}

int dIsAdminID(MYSQL *pstMySQL, char *sUserName)
{
	int			dResult;
	char		szBuf[MAX_SQLQUERY_SIZE];
	MYSQL_ROW	stRow;
	MYSQL_RES	*pstRst;

	sprintf(szBuf,
		"SELECT "
			"COUNT(*) "
		"FROM "
			"SYS_USER_INFO "
		"WHERE "
			"USERNAME='%s' AND USERLEVEL=0", sUserName);

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

	dResult	= 0;
	while( (stRow = mysql_fetch_row(pstRst)) != NULL)
		dResult	= atoi(*stRow);

	mysql_free_result(pstRst);

	return dResult;
}

int dGetCmdHistoryCount(MYSQL *pstMySQL, char *sUserName, time_t tStartTime, time_t tEndTime, int dIsAdmin, int *dCnt)
{
	int			dResult;
	char		szBuf[MAX_SQLQUERY_SIZE];
	MYSQL_ROW	stRow;
	MYSQL_RES	*pstRst;

	if(dIsAdmin)
	{
		sprintf(szBuf,
			"SELECT "
				"COUNT(*) "
			"FROM "
			"( "
				"SELECT "
					"USERNAME,USERIP,TIME,COMMAND "
				"FROM "
					"SYS_MMCD_MSG "
				"WHERE "
					"(TIME >= %lu) AND (TIME <= %lu) "
				"GROUP BY "
					"USERNAME,USERIP,TIME,COMMAND "
			") CMD_HIS", tStartTime, tEndTime);
	}
	else
	{
		sprintf(szBuf,
			"SELECT "
				"COUNT(*) "
			"FROM "
			"( "
				"SELECT "
					"USERNAME,USERIP,TIME,COMMAND "
				"FROM "
					"SYS_MMCD_MSG "
				"WHERE "
					"(TIME >= %lu) AND (TIME <= %lu) AND (USERNAME='%s') "
				"GROUP BY "
					"USERNAME,USERIP,TIME,COMMAND "
			") CMD_HIS", tStartTime, tEndTime, sUserName);
	}

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

	dResult	= 0;
	while( (stRow = mysql_fetch_row(pstRst)) != NULL)
		dResult	= atoi(*stRow);

	mysql_free_result(pstRst);
	*dCnt = dResult;

	return 0;
}

int dGetCommandList(MYSQL *pstMySQL, char *sUserName, st_Cmd *pstData, time_t tStartTime, time_t tEndTime, int dIsAdmin, int dLoopCnt)
{
	int			i;
	size_t		szUserNameLen, szCommandLen;
	char		szBuf[MAX_SQLQUERY_SIZE];
	MYSQL_ROW	stRow;
	MYSQL_RES	*pstRst;

	if(dIsAdmin)
	{
		sprintf(szBuf,
			"SELECT "
				"HIS_TABLE.USERNAME,HIS_TABLE.TIME,IFNULL(HIS_TABLE.USERIP,0),HIS_TABLE.COMMAND "
			"FROM "
				"("
				"SELECT "
					"@ROWNUM := @ROWNUM + 1 AS ROWNUM, SYS_MMCD_MSG.USERNAME AS USERNAME, SYS_MMCD_MSG.USERIP AS USERIP, SYS_MMCD_MSG.TIME AS TIME, SYS_MMCD_MSG.COMMAND AS COMMAND "
				"FROM "
					"SYS_MMCD_MSG, "
					"(SELECT @ROWNUM := 0) R "
				"WHERE "
					"((TIME >= %lu) AND (TIME <= %lu)) "
				"GROUP BY "
					"SYS_MMCD_MSG.USERNAME,SYS_MMCD_MSG.USERIP,SYS_MMCD_MSG.TIME,SYS_MMCD_MSG.COMMAND "
				"ORDER BY "
					"SYS_MMCD_MSG.TIME "
				") HIS_TABLE "
			"WHERE "
				"HIS_TABLE.ROWNUM > %d", tStartTime, tEndTime, MAX_CMD_CNT*(dLoopCnt-1));
	}
	else
	{
		sprintf(szBuf,
			"SELECT "
				"HIS_TABLE.USERNAME,HIS_TABLE.TIME,IFNULL(HIS_TABLE.USERIP,0),HIS_TABLE.COMMAND "
			"FROM "
				"("
				"SELECT "
					"@ROWNUM := @ROWNUM + 1 AS ROWNUM, SYS_MMCD_MSG.USERNAME AS USERNAME, SYS_MMCD_MSG.USERIP AS USERIP, SYS_MMCD_MSG.TIME AS TIME, SYS_MMCD_MSG.COMMAND AS COMMAND "
				"FROM "
					"SYS_MMCD_MSG, "
					"(SELECT @ROWNUM := 0) R "
				"WHERE "
					"((TIME >= %lu) AND (TIME <= %lu)) AND (USERNAME='%s') "
				"GROUP BY "
					"SYS_MMCD_MSG.USERNAME,SYS_MMCD_MSG.USERIP,SYS_MMCD_MSG.TIME,SYS_MMCD_MSG.COMMAND "
				"ORDER BY "
					"SYS_MMCD_MSG.TIME "
				") HIS_TABLE "
			"WHERE "
				"HIS_TABLE.ROWNUM > %d", tStartTime, tEndTime, sUserName, MAX_CMD_CNT*(dLoopCnt-1));
	}

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
	szUserNameLen	= sizeof(pstData[0].szUserName);
	szCommandLen	= sizeof(pstData[0].szCommand);
	while( (stRow = mysql_fetch_row(pstRst)) != NULL)
	{
		if(i >= MAX_CMD_CNT)
		{
			log_print(LOGN_DEBUG, "F=%s:%s.%d: SYS_MMCD_MSG MAX_CMD_CNT[%d] OVER[%d] [%s]", __FILE__, __FUNCTION__, __LINE__,
				MAX_CMD_CNT, i, szBuf);
			break;
		}

		if(strlen(stRow[0]) < szUserNameLen)
			strncpy(pstData[i].szUserName, stRow[0], szUserNameLen);
		else
		{
			strncpy(pstData[i].szUserName, stRow[0], szUserNameLen-1);
			pstData[i].szUserName[szUserNameLen-1] = 0x00;
		}

		pstData[i].uiTime		= (unsigned int)atoi(stRow[1]);
		pstData[i].uiUserBIP	= (unsigned int)atoi(stRow[2]);

		if(strlen(stRow[3]) < szCommandLen)
			strncpy(pstData[i].szCommand, stRow[3], szCommandLen);
		else
		{
			strncpy(pstData[i].szCommand, stRow[3], szCommandLen-1);
			pstData[i].szCommand[szCommandLen-1] = 0x00;
		}
		i++;
	}
	mysql_free_result(pstRst);

	if(i)
		return i;
	else
		return 0;
}

int dGetCnt(MYSQL *pstMySQL, char *sCountTableSqlStmt, int *dCnt)
{
	char		sBuf[MAX_SQLQUERY_SIZE], sTbl[128];
	size_t		szTbl;
	MYSQL_ROW	stRow;
	MYSQL_RES	*pstRst;

	sprintf(sBuf, "%s", sCountTableSqlStmt);
	if(mysql_query(pstMySQL, sBuf) != 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_query(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			sBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
		return -1;
	}

	if( (pstRst = mysql_store_result(pstMySQL)) == NULL)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_store_result(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			sBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
		return -2;
	}

	szTbl	= sizeof(sTbl);
	while( (stRow = mysql_fetch_row(pstRst)) != NULL)
	{
		if(strlen(stRow[0]) < szTbl)
			strncpy(sTbl, stRow[0], szTbl);
		else
		{
			strncpy(sTbl, stRow[0], szTbl-1);
			sTbl[szTbl-1] = 0x00;
		}
	}
	mysql_free_result(pstRst);

	sprintf(sBuf, "SELECT COUNT(*) FROM %s", sTbl);

	if(mysql_query(pstMySQL, sBuf) != 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_query(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			sBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
		return -3;
	}

	if( (pstRst = mysql_store_result(pstMySQL)) == NULL)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_store_result(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			sBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
		return -4;
	}

	if( (stRow = mysql_fetch_row(pstRst)) != NULL)
		*dCnt	= atoi(*stRow);

	mysql_free_result(pstRst);

	return 0;
}

int dAddAdminInfo(MYSQL *pstMySQL, st_User_Add *stData)
{
	char		szBuf[MAX_SQLQUERY_SIZE];

	sprintf(szBuf,
		"INSERT INTO SYS_USER_INFO VALUES ('%s','%s',%d,%d,%d,%u,'%s','%s',%d)",
		stData->szUserName, stData->szPassword, stData->sSLevel, 0, 0, stData->uConnectIP,
		stData->szLocalName, stData->szContact, stData->uCreateTime);

	if(mysql_query(pstMySQL, szBuf) != 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_query(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			szBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
		if(mysql_errno(pstMySQL) == DB_DUPLICATED_ENTRY)
			return -1;
		else
			return -2;
	}

	return 0;
}

int dSelectAdminInfo(MYSQL *pstMySQL, st_User_Add *pstData)
{
	int			dCount;
	char		szBuf[MAX_SQLQUERY_SIZE];
	MYSQL_ROW	stRow;
	MYSQL_RES	*pstRst;

	sprintf(szBuf, "SELECT COUNT(*) FROM SYS_USER_INFO WHERE USERNAME='%s'", pstData->szUserName);

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

	dCount	= 0;
	while( (stRow = mysql_fetch_row(pstRst)) != NULL)
		dCount		= (unsigned int)atoi(stRow[0]);

	mysql_free_result(pstRst);

	return dCount;
}

int dDeleteAdminInfo(MYSQL *pstMySQL, st_User_Add *pstData)
{
	char		szBuf[MAX_SQLQUERY_SIZE];

	sprintf(szBuf, "DELETE FROM SYS_USER_INFO WHERE USERNAME='%s'", pstData->szUserName);

	if(mysql_query(pstMySQL, szBuf) != 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_query(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			szBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
		return -1;
	}

	return 0;
}

int dUpdateAdminInfo(MYSQL *pstMySQL, st_User_Add *pstData)
{
	char	szBuf[MAX_SQLQUERY_SIZE];

	sprintf(szBuf,
		"UPDATE "
			"SYS_USER_INFO "
		"SET "
			"PASSWORD='%s', USERLEVEL=%d "
		"WHERE "
			"USERNAME = '%s'",
	    pstData->szPassword, pstData->sSLevel, pstData->szUserName);

	if(mysql_query(pstMySQL, szBuf) != 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_query(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			szBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
		return -1;
	}

	return 0;
}

int dGetSysStat(MYSQL *pstMySQL, st_SYS_STAT *pstData, char *sQuery, int dType)
{
	int			i;
	char		sTemp[2][DEF_TABLENAME_LEN];
	char		sBuf[MAX_SQLQUERY_SIZE];
	MYSQL_ROW	stRow;
	MYSQL_RES	*pstRst;

	sprintf(sBuf, "%s" ,sQuery);
	log_print(LOGN_DEBUG, "F=%s:%s.%d: sBuf[%s]", __FILE__, __FUNCTION__, __LINE__, sBuf);

	if(mysql_query(pstMySQL, sBuf) != 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_query(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			sBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
		return -1;
	}

	if( (pstRst = mysql_store_result(pstMySQL)) == NULL)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_store_result(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			sBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
		return -2;
	}

	if(dType == 0)	/*	DEF_STAT_FAULT	*/
	{
		for(i = 0; ((i < 2) && ((stRow = mysql_fetch_row(pstRst)) != NULL)); i++)
		{
			if(strlen(stRow[0]) < DEF_TABLENAME_LEN)
				strncpy(sTemp[i], stRow[0], DEF_TABLENAME_LEN);
			else
			{
				strncpy(sTemp[i], stRow[0], DEF_TABLENAME_LEN-1);
				sTemp[i][DEF_TABLENAME_LEN-1] = 0x00;
			}

			if(strstr(sTemp[i], "SOFT") != NULL)
				pstData[i].usType	= 1;
			else
				pstData[i].usType	= 0;

			pstData[i].usStatType	= dType;
			pstData[i].uiCri		= (unsigned int)atoi(stRow[1]);
			pstData[i].uiMaj		= (unsigned int)atoi(stRow[2]);
			pstData[i].uiMin		= (unsigned int)atoi(stRow[3]);
			pstData[i].uiStop		= (unsigned int)atoi(stRow[4]);
			pstData[i].uiCnt		= (unsigned int)atoi(stRow[5]);
		}
	}
	else if(dType == 1)	/*	DEF_STAT_LOAD	*/
	{
		if( (stRow = mysql_fetch_row(pstRst)) != NULL)
		{
			for(i = 0; i < 9; i++)
			{
				pstData[i].uiAvg	= (unsigned int)atoi(stRow[(i*3)]);
				pstData[i].uiMax	= (unsigned int)atoi(stRow[(i*3)+1]);
				pstData[i].uiMin	= (unsigned int)atoi(stRow[(i*3)+2]);
			}
		}
	}
	else
		log_print(LOGN_CRI, "F=%s:%s.%d: dType[%d] is an unknown type", __FILE__, __FUNCTION__, __LINE__, dType);

	mysql_free_result(pstRst);

	return i;
}

int dGetTrafficStat(MYSQL *pstMySQL, st_traffic_stat *pstData, char *szQuery)
{
	int			i;
	char		szBuf[MAX_SQLQUERY_SIZE];
	MYSQL_ROW	stRow;
	MYSQL_RES	*pstRst;

	sprintf(szBuf, "%s" ,szQuery);
	log_print(LOGN_CRI, "F=%s:%s.%d: szBuf[%s]", __FILE__, __FUNCTION__, __LINE__, szBuf);

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

	i = 0;
	if( (stRow = mysql_fetch_row(pstRst)) != NULL)
	{
		pstData->uThruStatFrames	= (unsigned int)atoi(stRow[0]);
		pstData->ulThruStatBytes	= (unsigned long)atol(stRow[1]);

		pstData->uTotStatFrames		= (unsigned int)atoi(stRow[2]);
		pstData->ulTotStatBytes		= (unsigned long)atol(stRow[3]);

		pstData->uIPStatFrames		= (unsigned int)atoi(stRow[4]);
		pstData->ulIPStatBytes		= (unsigned long)atol(stRow[5]);

		pstData->uUDPStatFrames		= (unsigned int)atoi(stRow[6]);
		pstData->ulUDPStatBytes		= (unsigned long)atol(stRow[7]);

		pstData->uTCPStatFrames		= (unsigned int)atoi(stRow[8]);
		pstData->ulTCPStatBytes		= (unsigned long)atol(stRow[9]);

		pstData->uSCTPStatFrames	= (unsigned int)atoi(stRow[10]);
		pstData->ulSCTPStatBytes	= (unsigned long)atol(stRow[11]);

		pstData->uETCStatFrames		= (unsigned int)atoi(stRow[12]);
		pstData->ulETCStatBytes		= (unsigned long)atol(stRow[13]);

		pstData->uIPErrorFrames		= (unsigned int)atoi(stRow[14]);
		pstData->ulIPErrorBytes		= (unsigned long)atol(stRow[15]);

		pstData->uUTCPErrorFrames	= (unsigned int)atoi(stRow[16]);
		pstData->ulUTCPErrorBytes	= (unsigned long)atol(stRow[17]);

		pstData->uFailDataFrames	= (unsigned int)atoi(stRow[18]);
		pstData->ulFailDataBytes	= (unsigned long)atol(stRow[19]);

		pstData->uFilterOutFrames	= (unsigned int)atoi(stRow[20]);
		pstData->ulFilterOutBytes	= (unsigned long)atol(stRow[21]);

		i++;
	}
	mysql_free_result(pstRst);

	if(!i)
		return -3;
	else
		return 0;
}

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

int	dInsertThres(MYSQL *pstMySQL, st_Thres *pstData)
{
	char	szBuf[MAX_SQLQUERY_SIZE];

	sprintf(szBuf,
		"INSERT INTO INFO_DEFECT_THRESHOLD VALUES (%hu,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,'%s')",
		pstData->cSvcType, pstData->uTCPSetupTime, pstData->uResponseTime, pstData->uUpThroughput, pstData->uDnThroughput,
		pstData->uUpRetransCount, pstData->uDnRetransCount, pstData->uUpJitter, pstData->uDnJitter, pstData->uUpPacketLoss,
		pstData->uDnPacketLoss, pstData->szDesc);

	if(mysql_query(pstMySQL, szBuf) != 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_query(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			szBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
		return -1;
	}

	return 0;
}

int dGet_CONDMsg(MYSQL *pstMySQL, st_SysCONDMsg *pstData, time_t tSTime, time_t tETime, int *dCnt)
{
	int			i;
	char		szBuf[MAX_SQLQUERY_SIZE];
	size_t		MessageLen;
	MYSQL_ROW	stRow;
	MYSQL_RES	*pstRst;

	sprintf(szBuf,
		"SELECT "
			"TIME, SYSTYPE, NTAMID, NTAFID, LOCTYPE, INVTYPE, INVNO,  IFNULL(IPADDR,0), IFNULL(MESSAGE,'-') "
		"FROM "
			"SYS_COND_MSG "
		"WHERE "
			"(TIME >= %lu) AND (TIME <= %lu)",
		tSTime, tETime);

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
	MessageLen	= sizeof(pstData[0].szMessage);
	while( (stRow = mysql_fetch_row(pstRst)) != NULL)
	{
		if(i >= MAX_CONDMSG_CNT)
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: SYS_COND_MSG MAX_CONDMSG_CNT[%d] OVER[%d] [%s]", __FILE__, __FUNCTION__, __LINE__,
				MAX_CONDMSG_CNT, i, szBuf);
			break;
		}

		pstData[i].uiTime		= (unsigned int)atoi(stRow[0]);
		pstData[i].ucSysType	= (unsigned char)atoi(stRow[1]);
		pstData[i].ucNTAMID		= (unsigned char)atoi(stRow[2]);
		pstData[i].ucNTAFID		= (unsigned char)atoi(stRow[3]);
		pstData[i].ucLocType	= (unsigned char)atoi(stRow[4]);
		pstData[i].ucInvType	= (unsigned char)atoi(stRow[5]);
		pstData[i].ucInvNo		= (unsigned char)atoi(stRow[6]);
		pstData[i].uiIPAddr		= (unsigned int)atoi(stRow[7]);

		if(strlen(stRow[8]) < MessageLen)
			strncpy(pstData[i].szMessage, stRow[8], MessageLen);
		else
		{
			strncpy(pstData[i].szMessage, stRow[8], MessageLen-1);
			pstData[i].szMessage[MessageLen-1] = 0x00;
		}
		i++;
	}
	mysql_free_result(pstRst);
	*dCnt = i;

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

int dSyncInfoEquipTAMDB(MYSQL *pstMySQL, char *sFilePath, char *sFileName)
{
	int			i, fd;
	char		sBuf[MAX_SQLQUERY_SIZE], sLineBuf[BUF_SIZE], sFullFileName[PATH_MAX];
	ssize_t		sszWrLen, szLineLen;
	MYSQL_ROW	stRow;
	MYSQL_RES	*pstRst;

	sprintf(sBuf,
		"SELECT "
			"EquipIp,IFNULL(EquipType,0),IFNULL(EquipTypeName,0),IFNULL(EquipName,'-'),IFNULL(Description,'-') "
		"FROM "
			"TB_MEQUIPMC "
		"WHERE "
			"EquipType<%d "
		"ORDER BY "
			"EquipIp, EquipType", ROAM_JAPAN);

	if(mysql_query(pstMySQL, sBuf) != 0){
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_query(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			sBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
		return -1;
	}

	if( (pstRst = mysql_store_result(pstMySQL)) == NULL){
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_store_result(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			sBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
		return -2;
	}

	memset(sFullFileName, 0x00, PATH_MAX);
	sprintf(sFullFileName, "%s/%s", sFilePath, sFileName);
	if( (fd = open(sFullFileName, O_CREAT|O_WRONLY|O_TRUNC, 0666)) == -1){
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN open(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			sFullFileName, errno, strerror(errno));
		return -3;
	}

	sprintf(sLineBuf, "DELETE FROM TB_MEQUIPMC;\n");
	szLineLen = strlen(sLineBuf);
	if( (sszWrLen = write(fd, sLineBuf, szLineLen)) == -1){
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN write() [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, errno, strerror(errno));
		return -4;
	}

	if(sszWrLen != szLineLen){
		log_print(LOGN_CRI, "F=%s:%s.%d: Didn't complete to write a file[%s] sszWrLen[%lu] szLineLen[%lu]", __FILE__, __FUNCTION__, __LINE__,
			sFileName, sszWrLen, szLineLen);
		return -5;
	}

	i		= 0;
	while( (stRow = mysql_fetch_row(pstRst)) != NULL){

		memset(sLineBuf, 0x00, BUF_SIZE);
		if(i >= MAX_EQUIP_INFO){
			log_print(LOGN_CRI, "F=%s:%s.%d: TB_MEQUIPMC MAX_EQUIP_INFO[%d] OVER[%d] [%s]", __FILE__, __FUNCTION__, __LINE__,
				MAX_EQUIP_INFO, i, sBuf);
			break;
		}

		sprintf(sLineBuf,
			"INSERT INTO TB_MEQUIPMC "
			"(EQUIPIP,EQUIPTYPE,EQUIPTYPENAME,EQUIPNAME,DESCRIPTION) "
			"VALUES(%s,%s,'%s','%s','%s');\n",
			stRow[0], stRow[1], stRow[2], stRow[3], stRow[4]);

		szLineLen = strlen(sLineBuf);
		if( (sszWrLen = write(fd, sLineBuf, szLineLen)) == -1){
			log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN write() [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, errno, strerror(errno));
			return -6;
		}

		if(sszWrLen != szLineLen){
			log_print(LOGN_CRI, "F=%s:%s.%d: Didn't complete to write a file[%s] sszWrLen[%lu] szLineLen[%lu]", __FILE__, __FUNCTION__, __LINE__,
				sFileName, sszWrLen, szLineLen);
			return -7;
		}
		i++;
	}
	mysql_free_result(pstRst);

	sprintf(sLineBuf, "Commit work;\n");
	szLineLen = strlen(sLineBuf);
	if( (sszWrLen = write(fd, sLineBuf, szLineLen)) == -1){
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN write() [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, errno, strerror(errno));
		return -8;
	}

	if(sszWrLen != szLineLen){
		log_print(LOGN_CRI, "F=%s:%s.%d: Didn't complete to write a file[%s] sszWrLen[%lu] szLineLen[%lu]", __FILE__, __FUNCTION__, __LINE__,
			sFileName, sszWrLen, szLineLen);
		return -9;
	}

	if(close(fd) == -1){
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN close() [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, errno, strerror(errno));
		return -10;
	}

	return 0;
}

int dCheckStatistics(MYSQL *pstMySQL, time_t tStatTime)
{
	int			i, dResult;
	char		sBuf[MAX_SQLQUERY_SIZE];
	MYSQL_ROW	stRow;
	MYSQL_RES	*pstRst;

	for(i = 0; i < IDX_MAXIMUM_STATISTICS; i++)
	{
		switch(i)
		{
			case IDX_LOAD_STATISTICS:
				sprintf(sBuf, "SELECT COUNT(*) FROM STAT_LOAD_5MIN WHERE STATTIME=%lu", tStatTime);
				break;
			case IDX_FAULT_STATISTICS:
				sprintf(sBuf, "SELECT COUNT(*) FROM STAT_FAULT_5MIN WHERE STATTIME=%lu", tStatTime);
				break;
			case IDX_TRAFFIC_STATISTICS:
				sprintf(sBuf, "SELECT COUNT(*) FROM STAT_TRAFFIC_5MIN WHERE STATTIME=%lu", tStatTime);
				break;
		}

		if(mysql_query(pstMySQL, sBuf) != 0)
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_query(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
				sBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
			return -1;
		}

		if( (pstRst = mysql_store_result(pstMySQL)) == NULL)
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_store_result(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
				sBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
			return -2;
		}

		dResult	= 0;
		while( (stRow = mysql_fetch_row(pstRst)) != NULL)
			dResult	= atoi(*stRow);

		mysql_free_result(pstRst);

		if(dResult == 0)
		{
			switch(i)
			{
				case IDX_LOAD_STATISTICS:
					sprintf(sBuf, "STAT_LOAD_5MIN");
					break;
				case IDX_FAULT_STATISTICS:
					sprintf(sBuf, "STAT_FAULT_5MIN");
					break;
				case IDX_TRAFFIC_STATISTICS:
					sprintf(sBuf, "STAT_TRAFFIC_5MIN");
					break;
			}
			log_print(LOGN_CRI, "F=%s:%s.%d: COUNT[%d] IN TABLE[%s]", __FILE__, __FUNCTION__, __LINE__, dResult, sBuf);
			return -1;
		}
	}

	return 0;
}

int dCheckHourStat(MYSQL *pstMySQL, time_t tStart, time_t tEnd)
{
	int			i, dResult;
	char		sBuf[MAX_SQLQUERY_SIZE];
	MYSQL_ROW	stRow;
	MYSQL_RES	*pstRst;

	for(i = 0; i < IDX_MAXIMUM_STATISTICS; i++)
	{
		switch(i)
		{
			case IDX_LOAD_STATISTICS:
				sprintf(sBuf, "SELECT COUNT(*) FROM STAT_LOAD_5MIN WHERE STATTIME>=%lu AND STATTIME<=%lu", tStart, tEnd);
				break;
			case IDX_FAULT_STATISTICS:
				sprintf(sBuf, "SELECT COUNT(*) FROM STAT_FAULT_5MIN WHERE STATTIME>=%lu AND STATTIME<=%lu", tStart, tEnd);
				break;
			case IDX_TRAFFIC_STATISTICS:
				sprintf(sBuf, "SELECT COUNT(*) FROM STAT_TRAFFIC_5MIN WHERE STATTIME>=%lu AND STATTIME<=%lu", tStart, tEnd);
				break;
		}

		if(mysql_query(pstMySQL, sBuf) != 0)
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_query(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
				sBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
			return -1;
		}

		if( (pstRst = mysql_store_result(pstMySQL)) == NULL)
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_store_result(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
				sBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
			return -2;
		}

		dResult	= 0;
		if( (stRow = mysql_fetch_row(pstRst)) != NULL)
			dResult	= atoi(*stRow);

		mysql_free_result(pstRst);

		if(dResult == 0)
		{
			switch(i)
			{
				case IDX_LOAD_STATISTICS:
					sprintf(sBuf, "STAT_LOAD_5MIN");
					break;
				case IDX_FAULT_STATISTICS:
					sprintf(sBuf, "STAT_FAULT_5MIN");
					break;
				case IDX_TRAFFIC_STATISTICS:
					sprintf(sBuf, "STAT_TRAFFIC_5MIN");
					break;
			}
			log_print(LOGN_CRI, "F=%s:%s.%d: COUNT[%d] IN TABLE[%s]", __FILE__, __FUNCTION__, __LINE__, dResult, sBuf);
			return -1;
		}
	}

	return 0;
}

int dInsert_CONDMsg(MYSQL *pstMySQL, st_SysCONDMsg *pstData)
{
	char	szBuf[MAX_SQLQUERY_SIZE];

	sprintf(szBuf, "INSERT INTO SYS_COND_MSG VALUES (%u, %d, %d, %d, %d, %d, %d, %u, \'%s\')",
        pstData->uiTime, pstData->ucSysType, pstData->ucNTAMID, pstData->ucNTAFID, pstData->ucLocType,
        pstData->ucInvType, pstData->ucInvNo, pstData->uiIPAddr, pstData->szMessage);

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

int dCreate_CONDMsg(MYSQL *pstMySQL)
{
	char    szQuery[MAX_STMT_SIZE*4], sFn[MAX_FILENAME_SIZE];
	int     dRet;

	sprintf(szQuery,
		"CREATE TABLE `SYS_COND_MSG` ("
			"`TIME` bigint(20) default NULL,"
			"`SYSTYPE` int(11) default NULL,"
			"`NTAMID` int(11) default NULL,"
			"`NTAFID` int(11) default NULL,"
			"`LOCTYPE` int(11) default NULL,"
			"`INVTYPE` int(11) default NULL,"
			"`INVNO` int(11) default NULL,"
			"`IPADDR` int(10) unsigned default NULL,"
			"`MESSAGE` text"
		") ENGINE=InnoDB DEFAULT CHARSET=euckr");

	sprintf(sFn, "%s", __FUNCTION__);
	if( (dRet = dCommonDDL(pstMySQL, sFn, (const char*)szQuery)) < 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN dCommonDDL() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		return -1;
	}

	return 0;
}

/* added by dcham 2011.06.26 */
int dInsertMONThres(MYSQL *pstMySQL, st_MON_ThresMMC *pstData)
{
	char        szBuf[MAX_SQLQUERY_SIZE];

	sprintf(szBuf,
			"INSERT INTO INFO_MON_THRESHOLD VALUES"
			"(%hu,%hu,%hu,%u,%hu,%hu,%hu,%hu,%u,%u,%u,'%s')",
			pstData->huBranchID, pstData->cSvcType, pstData->cAlarmType,pstData->dSvcIP,pstData->cStartHour,pstData->cDayRange,pstData->huDayRate,
			pstData->huNightRate, pstData->uDayMinTrial, pstData->uNigthMinTrial,pstData->uPeakTrial, pstData->szDesc);

	if(mysql_query(pstMySQL, szBuf) != 0)
	{
		switch(mysql_errno(pstMySQL))
		{
			case DB_DUPLICATED_ENTRY:
				log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_query(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
						szBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
				return -1;
			case DB_TABLE_NOT_EXIST:
				log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_query(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
						szBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
				return -2;
			case DB_NOT_CONNECT:
				log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_query(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
						szBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
				return -3;
			default:
				log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_query(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
						szBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
				return -4;
		}
	}
	return 0;
}
