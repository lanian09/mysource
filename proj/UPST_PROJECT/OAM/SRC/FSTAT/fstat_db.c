/**
 *	Include headers
 */

/* SYS HEADER */
#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
/* LIB HEADER */
#include "loglib.h"
#include "dblib.h"
/* PRO HEADER */
#include "commdef.h"
#include "timesec.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */
#include "fstat_db.h"

/**
	Declare variables
*/
extern MYSQL		stMySQL;


/*****************************************************
  INSERT LOAD DATA
 ****************************************************/

int dInsert_STAT_LOADData(st_LOAD *pstData, int dTotCount)
{
	int			i, dRet, dCount;
	time_t		tInsertTbl;

	dCount		= dTotCount;
	tInsertTbl	= pstData->uiStatTime;

	log_print(LOGN_INFO, LH"dCount[%d] tInsertTbl[%ld]", LT, dCount, tInsertTbl);
	for(i = 0; i < dCount; i++)
	{
		if( (dRet = dInsertLOADStatNew(&stMySQL, dCount, &pstData[i])) < 0)
		{
			switch(dRet)
			{
				case -1:
					log_print(LOGN_CRI, LH"ERR[%d] -> STAT_LOAD_5MIN NOT EXIST", LT, dRet);
					if( (dRet = dCreate_LOAD_Table(FIVE_MIN_PERIOD)) < 0)
					{
						log_print(LOGN_CRI, LH"ERROR in dCreate_LOAD_Table(FIVE_MIN_PERIOD) dRet[%d]", LT, dRet);
						break;
					}
					i--;
					continue;
				case -2:
					log_print(LOGN_CRI, LH"MYSQL IS NOT CONNECTED", LT);
					exit(-9);
				default:
					log_print(LOGN_WARN, LH"ERROR IN dInsertLOADStatNew(i[%d]) dRet[%d]", LT, i, dRet);
					InsertErrorPrint_LOAD(LOGN_WARN, &pstData[i]);
					continue;
			}
		}
		else
			log_print(LOGN_INFO,LH"SUCCESS IN dInsertLOADStatNew(i[%d]) dRet[%d]", LT, i, dRet);
	}

	if( (tInsertTbl % SEC_OF_HOUR) == 0)
	{
		if( (dRet = dInsertLOADStatPeriod(&stMySQL, tInsertTbl, ONE_HOUR_PERIOD)) < 0)
		{
			switch(dRet)
			{
				case -1:
					log_print(LOGN_CRI, LH"ERR[%d] -> STAT_LOAD_1HOUR NOT EXIST", LT, dRet);
					if( (dRet = dCreate_LOAD_Table(ONE_HOUR_PERIOD)) < 0)
					{
						log_print(LOGN_CRI, LH"ERROR in dCreate_LOAD_Table(ONE_HOUR_PERIOD) dRet[%d]", LT, dRet);
						break;
					}

					if( (dRet = dInsertLOADStatPeriod(&stMySQL, tInsertTbl, ONE_HOUR_PERIOD)) < 0)
					{
						log_print(LOGN_CRI, LH"ERROR in dInsertLOADStatPeriod(ONE_HOUR_PERIOD) dRet[%d]", LT, dRet);
						break;
					}
					break;
				case -2:
					log_print(LOGN_CRI, LH"MYSQL IS NOT CONNECTED", LT);
					exit(-9);
				default:
					log_print(LOGN_WARN, LH"ERROR IN dCreate_LOAD_Table(tInsertTbl[%lu], ONE_HOUR_PERIOD) dRet[%d]", LT, tInsertTbl, dRet);
					break;
			}
		}
		else
			log_print(LOGN_INFO,LH"SUCCESS IN dInsertLOADStatPeriod(tInsertTbl[%lu], ONE_HOUR_PERIOD) dRet[%d]", LT, tInsertTbl, dRet);
	}

	if( ((tInsertTbl+(SEC_OF_HOUR*9)) % SEC_OF_DAY) == 0)
	{
		if( (dRet = dInsertLOADStatPeriod(&stMySQL, tInsertTbl, ONE_DAY_PERIOD)) < 0)
		{
			switch(dRet)
			{
				case -1:
					log_print(LOGN_CRI, LH"ERR[%d] -> STAT_LOAD_1DAY NOT EXIST", LT, dRet);
					if( (dRet = dCreate_LOAD_Table(ONE_DAY_PERIOD)) < 0)
					{
						log_print(LOGN_CRI, LH"ERROR IN dCreate_LOAD_Table(ONE_DAY_PERIOD) dRet[%d]", LT, dRet);
						break;
					}

					if( (dRet = dInsertLOADStatPeriod(&stMySQL, tInsertTbl, ONE_DAY_PERIOD)) < 0)
					{
						log_print(LOGN_CRI, LH"ERROR in dInsertLOADStatPeriod(ONE_DAY_PERIOD) dRet[%d]", LT, dRet);
						break;
					}
					break;
				case -2:
					log_print(LOGN_CRI, LH"MYSQL IS NOT CONNECTED", LT);
					exit(-9);
				default:
					log_print(LOGN_WARN, LH"ERROR IN dCreate_LOAD_Table(tInsertTbl[%lu], ONE_DAY_PERIOD) dRet[%d]", LT,
						tInsertTbl, dRet);
					break;
			}
		}
		else
		{
			log_print(LOGN_INFO,LH"SUCCESS IN dInsertLOADStatPeriod(tInsertTbl[%lu], ONE_DAY_PERIOD) dRet[%d]", LT,
				tInsertTbl, dRet);
		}
	}

	return 0;
}

void Insert_STAT_LOADData(st_LOAD *pstData, int dTotCount)
{
	dInsert_STAT_LOADData(pstData, dTotCount);
}

/*****************************************************
  INSERT FAULT DATA
 ****************************************************/
int dInsert_STAT_FAULTData(st_FAULT *pstData, int dTotCount)
{
	int			i, dRet, dCount;
	time_t		tInsertTbl;

	dCount		= dTotCount;
	tInsertTbl	= pstData->uiStatTime;

	log_print(LOGN_INFO, LH"dCount[%d] tInsertTbl[%lu]", LT, dCount, tInsertTbl);
	for(i = 0; i < dCount; i++)
	{
		if( (dRet = dInsertFAULTStatNew(&stMySQL, dCount, &pstData[i])) < 0)
		{
			switch(dRet)
			{
				case -1:
					log_print(LOGN_CRI, LH"ERR[%d] -> STAT_FAULT_5MIN NOT EXIST", LT, dRet);
					if( (dRet = dCreate_FAULT_Table(FIVE_MIN_PERIOD)) < 0)
					{
						log_print(LOGN_CRI, LH"ERROR IN dCreate_FAULT_Table(FIVE_MIN_PERIOD) dRet[%d]", LT, dRet);
						break;
					}
					i--;
					continue;
				case -2:
					log_print(LOGN_CRI, LH"MYSQL IS NOT CONNECTED", LT);
					exit(-9);
				default:
					log_print(LOGN_WARN, LH"ERROR IN dInsertFAULTStatNew(i[%d]) dRet[%d]", LT, i, dRet);
					InsertErrorPrint_FAULT(LOGN_WARN, &pstData[i]);
					continue;
			}
		}
		else
			log_print(LOGN_INFO,LH"SUCCESS IN dInsertFAULTStatNew(i[%d]) dRet[%d]", LT, i, dRet);
	}

	if( (tInsertTbl % SEC_OF_HOUR) == 0)
	{
		if( (dRet = dInsertFAULTStatPeriod(&stMySQL, tInsertTbl, ONE_HOUR_PERIOD)) < 0)
		{
			switch(dRet)
			{
				case -1:
					log_print(LOGN_CRI, LH"ERR[%d] -> STAT_FAULT_1HOUR NOT EXIST", LT, dRet);
					if( (dRet = dCreate_FAULT_Table(ONE_HOUR_PERIOD)) < 0)
					{
						log_print(LOGN_CRI, LH"ERROR IN dCreate_FAULT_Table(ONE_HOUR_PERIOD) dRet[%d]", LT, dRet);
						break;
					}

					if( (dRet = dInsertFAULTStatPeriod(&stMySQL, tInsertTbl, ONE_HOUR_PERIOD)) < 0)
					{
						log_print(LOGN_CRI, LH"ERROR IN dInsertFAULTStatPeriod(ONE_HOUR_PERIOD) dRet[%d]", LT, dRet);
						break;
					}
					break;
				case -2:
					log_print(LOGN_CRI, LH"MySQL is not connected", LT);
					exit(-9);
				default:
					log_print(LOGN_WARN, LH"ERROR IN dInsertFAULTStatPeriod(tInsertTbl[%lu], ONE_HOUR_PERIOD) dRet[%d]", LT,
						tInsertTbl, dRet);
					break;
			}
		}
		else
		{
			log_print(LOGN_INFO,LH"SUCCESS IN dInsertFAULTStatPeriod(tInsertTbl[%lu], ONE_HOUR_PERIOD) dRet[%d]", LT,
				tInsertTbl, dRet);
		}
	}

	if( ((tInsertTbl+(SEC_OF_HOUR*9)) % SEC_OF_DAY) == 0)
	{
		if( (dRet = dInsertFAULTStatPeriod(&stMySQL, tInsertTbl, ONE_DAY_PERIOD)) < 0)
		{
			switch(dRet)
			{
				case -1:
					log_print(LOGN_CRI, LH"ERR[%d] -> STAT_FAULT_1HOUR NOT EXIST", LT, dRet);
					if( (dRet = dCreate_FAULT_Table(ONE_DAY_PERIOD)) < 0)
					{
						log_print(LOGN_CRI, LH"ERROR IN dCreate_FAULT_Table(ONE_DAY_PERIOD), dRet[%d]", LT, dRet);
						break;
					}

					if( (dRet = dInsertFAULTStatPeriod(&stMySQL, tInsertTbl, ONE_DAY_PERIOD)) < 0)
					{
						log_print(LOGN_CRI, LH"ERROR IN dInsertFAULTStatPeriod(ONE_DAY_PERIOD) dRet[%d]", LT, dRet);
						break;
					}
					break;
				case -2:
					log_print(LOGN_CRI, LH"MYSQL IS NOT CONNECTED", LT);
					exit(-9);
				default:
					log_print(LOGN_WARN, LH"ERROR IN dInsertFAULTStatPeriod(tInsertTbl[%lu], ONE_DAY_PERIOD) dRet[%d]", LT,
						tInsertTbl, dRet);
					break;
			}
		}
		else
		{
			log_print(LOGN_INFO,LH"SUCCESS IN dInsertFAULTStatPeriod(tInsertTbl[%lu], ONE_DAY_PERIOD) dRet[%d]", LT,
				tInsertTbl, dRet);
		}
	}

	return 0;
}

void Insert_STAT_FAULTData(st_FAULT *pstData, int dTotCount)
{
	dInsert_STAT_FAULTData(pstData, dTotCount);
}

int dInsert_STAT_TRAFFICData(st_TRAFFIC *pstData, int dTotCount)
{
	int			dRet, dTAFID;
	time_t		tInsertTbl;

	dTAFID		= pstData->usTAFID;
	tInsertTbl	= pstData->uiStatTime;

	log_print(LOGN_INFO, LH"tInsertTbl[%lu]", LT, tInsertTbl);
	if( (dRet = dInsertTRAFFICStatNew(&stMySQL, pstData)) < 0)
	{
		switch(dRet)
		{
			case -1:
				log_print(LOGN_CRI, LH"ERR[%d] -> STAT_TRAFFIC_5MIN NOT EXIST", LT, dRet);
				if( (dRet = dCreate_TRAFFIC_Table(FIVE_MIN_PERIOD)) < 0)
				{
					log_print(LOGN_CRI, LH"ERROR IN dCreate_TRAFFIC_Table(FIVE_MIN_PERIOD) dRet[%d]", LT, dRet);
					return dRet;
				}

				if( (dRet = dInsertTRAFFICStatNew(&stMySQL, pstData)) < 0)
				{
					log_print(LOGN_CRI, LH"ERROR IN dInsertTRAFFICStatNew(FIVE_MIN_PERIOD) dRet[%d]", LT, dRet);
					return dRet;
				}
				break;
			case -2:
				log_print(LOGN_CRI, LH"MYSQL IS NOT CONNECTED", LT);
				exit(-9);
			default:
				log_print(LOGN_WARN, LH"ERROR IN dInsertTRAFFICStatNew(dTAFID[%d]) dRet[%d]", LT, dTAFID, dRet);
				InsertErrorPrint_TRAFFIC(LOGN_WARN, pstData);
				break;
		}
	}
	else
		log_print(LOGN_INFO,LH"SUCCESS IN dInsertTRAFFICStatNew(dTAFID[%d]) dRet[%d]", LT, dTAFID, dRet);

	if( (tInsertTbl % SEC_OF_HOUR) == 0)
	{
		if( (dRet = dInsertTRAFFICStatPeriod(&stMySQL, tInsertTbl, dTAFID, ONE_HOUR_PERIOD)) < 0)
		{
			switch(dRet)
			{
				case -1:
					log_print(LOGN_CRI, LH"ERR[%d] -> STAT_TRAFFIC_1HOUR NOT EXIST", LT, dRet);
					if( (dRet = dCreate_TRAFFIC_Table(ONE_HOUR_PERIOD)) < 0)
					{
						log_print(LOGN_CRI, LH"ERROR IN dCreate_TRAFFIC_Table(ONE_HOUR_PERIOD) dRet[%d]", LT, dRet);
						return dRet;
					}

					if( (dRet = dInsertTRAFFICStatPeriod(&stMySQL, tInsertTbl, dTAFID, ONE_HOUR_PERIOD)) < 0)
					{
						log_print(LOGN_CRI, LH"ERROR IN dInsertTRAFFICStatPeriod(tInsertTbl[%lu], dTAFID[%d], ONE_HOUR_PERIOD) dRet[%d]", LT,
							tInsertTbl, dTAFID, dRet);
						return dRet;
					}
					break;
				case -2:
					log_print(LOGN_CRI, LH"MYSQL IS NOT CONNECTED", LT);
					exit(-9);
				default:
					log_print(LOGN_WARN, LH"ERROR IN dInsertTRAFFICStatPeriod(tInsertTbl[%lu], dTAFID[%d], ONE_HOUR_PERIOD) dRet[%d]", LT,
						tInsertTbl, dTAFID, dRet);
					break;
			}
		}
		else
		{
			log_print(LOGN_INFO,LH"SUCCESS IN dInsertTRAFFICStatPeriod(tInsertTbl[%lu], dTAFID[%d], ONE_HOUR_PERIOD) dRet[%d]", LT,
				tInsertTbl, dTAFID, dRet);
		}
	}

	if( ((tInsertTbl+(SEC_OF_HOUR*9)) % SEC_OF_DAY) == 0)
	{
		if( (dRet = dInsertTRAFFICStatPeriod(&stMySQL, tInsertTbl, dTAFID, ONE_DAY_PERIOD)) < 0)
		{
			switch(dRet)
			{
				case -1:
					log_print(LOGN_CRI, LH"ERR[%d] -> STAT_TRAFFIC_1DAY NOT EXIST", LT, dRet);
					if( (dRet = dCreate_TRAFFIC_Table(ONE_DAY_PERIOD)) < 0)
					{
						log_print(LOGN_CRI, LH"ERROR IN dCreate_TRAFFIC_Table(ONE_DAY_PERIOD) dRet[%d]", LT, dRet);
						return dRet;
					}

					if( (dRet = dInsertTRAFFICStatPeriod(&stMySQL, tInsertTbl, dTAFID, ONE_DAY_PERIOD)) < 0)
					{
						log_print(LOGN_CRI, LH"ERROR IN dInsertTRAFFICStatPeriod(tInsertTbl[%lu], dTAFID[%d], ONE_DAY_PERIOD) dRet[%d]", LT,
							tInsertTbl, dTAFID, dRet);
						return dRet;
					}
					break;
				case -2:
					log_print(LOGN_CRI, LH"MYSQL IS NOT CONNECTED", LT);
					exit(-9);
				default:
					log_print(LOGN_WARN, LH"ERROR IN dInsertTRAFFICStatPeriod(tInsertTbl[%lu], dTAFID[%d], ONE_DAY_PERIOD) dRet[%d]", LT,
						tInsertTbl, dTAFID, dRet);
					break;
			}
		}
		else
		{
			log_print(LOGN_INFO,LH"SUCCESS IN dInsertTRAFFICStatPeriod(tInsertTbl[%lu], dTAFID[%d], ONE_DAY_PERIOD) dRet[%d]", LT,
				tInsertTbl, dTAFID, dRet);
		}
	}

	return 0;
}

void Insert_STAT_TRAFFICData(st_TRAFFIC *pstData, int dTotCount)
{
	dInsert_STAT_TRAFFICData(pstData, dTotCount);
}

/*****************************************************
  CREATE LOAD TABLE
 ****************************************************/
int dCreate_LOAD_Table(char cPeriod)
{
	char    szQuery[MAX_STMT_SIZE*4], sFn[MAX_FUNCNAME_SIZE], sPeriod[MAX_STRPERIOD_SIZE];
	int     dRet;

	switch(cPeriod)
	{
		case FIVE_MIN_PERIOD:
			sprintf(sPeriod, "%s", "5MIN");
			break;
		case ONE_HOUR_PERIOD:
			sprintf(sPeriod, "%s", "1HOUR");
			break;
		case ONE_DAY_PERIOD:
			sprintf(sPeriod, "%s", "1DAY");
			break;
		default:
			log_print(LOGN_CRI, LH"unknown cPeriod[%d]", LT, cPeriod);
			return -1;
	}

	sprintf(szQuery,
		"CREATE TABLE `STAT_LOAD_%s` ("
			"`STATTIME` bigint(20) NOT NULL default '0',"
			"`SYSTEMTYPE` int(11) NOT NULL default '0',"
			"`SYSTEMID` int(11) NOT NULL default '0',"
			"`CPUAVG` int(11) default NULL,"
			"`CPUMAX` int(11) default NULL,"
			"`CPUMIN` int(11) default NULL,"
			"`MEMAVG` int(11) default NULL,"
			"`MEMMAX` int(11) default NULL,"
			"`MEMMIN` int(11) default NULL,"
			"`QUEAVG` int(11) default NULL,"
			"`QUEMAX` int(11) default NULL,"
			"`QUEMIN` int(11) default NULL,"
			"`NIFOAVG` int(11) default NULL,"
			"`NIFOMAX` int(11) default NULL,"
			"`NIFOMIN` int(11) default NULL,"
			"`TRAFFICAVG` int(11) default NULL,"
			"`TRAFFICMAX` int(11) default NULL,"
			"`TRAFFICMIN` int(11) default NULL,"
			"`DISK1AVG` int(11) default NULL,"
			"`DISK1MAX` int(11) default NULL,"
			"`DISK1MIN` int(11) default NULL,"
			"`DISK2AVG` int(11) default NULL,"
			"`DISK2MAX` int(11) default NULL,"
			"`DISK2MIN` int(11) default NULL,"
			"`DISK3AVG` int(11) default NULL,"
			"`DISK3MAX` int(11) default NULL,"
			"`DISK3MIN` int(11) default NULL,"
			"`DISK4AVG` int(11) default NULL,"
			"`DISK4MAX` int(11) default NULL,"
			"`DISK4MIN` int(11) default NULL,"
			"`COUNT` int(11) default NULL,"
			"PRIMARY KEY (`STATTIME`,`SYSTEMTYPE`,`SYSTEMID`)"
		") ENGINE=InnoDB DEFAULT CHARSET=euckr", sPeriod);

	sprintf(sFn, "%s", __FUNCTION__);
	if( (dRet = dCommonDDL(&stMySQL, sFn, (const char*)szQuery)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dCommonDDL() dRet[%d]", LT, dRet);
		return -2;
	}

	return 0;
}

/*****************************************************
  CREATE FAULT TABLE
 ****************************************************/
int dCreate_FAULT_Table(char cPeriod)
{
	char    szQuery[MAX_STMT_SIZE*4], sFn[MAX_FUNCNAME_SIZE], sPeriod[MAX_STRPERIOD_SIZE];
	int     dRet;

	switch(cPeriod)
	{
		case FIVE_MIN_PERIOD:
			sprintf(sPeriod, "%s", "5MIN");
			break;
		case ONE_HOUR_PERIOD:
			sprintf(sPeriod, "%s", "1HOUR");
			break;
		case ONE_DAY_PERIOD:
			sprintf(sPeriod, "%s", "1DAY");
			break;
		default:
			log_print(LOGN_CRI, LH"unknown cPeriod[%d]", LT, cPeriod);
			return -1;
	}

	sprintf(szQuery,
		"CREATE TABLE `STAT_FAULT_%s` ("
			"`STATTIME` bigint(20) NOT NULL default '0',"
			"`SYSTEMTYPE` int(11) NOT NULL default '0',"
			"`SYSTEMID` int(11) NOT NULL default '0',"
			"`FAULTTYPE` varchar(8) NOT NULL default '',"
			"`CRI` int(11) default NULL,"
			"`MAJ` int(11) default NULL,"
			"`MIN` int(11) default NULL,"
			"`STOP` int(11) default NULL,"
			"`NORMAL` int(11) default NULL,"
			"PRIMARY KEY  (`STATTIME`,`SYSTEMTYPE`,`SYSTEMID`,`FAULTTYPE`)"
		") ENGINE=InnoDB DEFAULT CHARSET=euckr", sPeriod);

	sprintf(sFn, "%s", __FUNCTION__);
	if( (dRet = dCommonDDL(&stMySQL, sFn, (const char*)szQuery)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dCommonDDL() dRet[%d]", LT, dRet);
		return -2;
	}

	return 0;
}

/*****************************************************
  CREATE TRAFFIC TABLE
 ****************************************************/
int dCreate_TRAFFIC_Table(char cPeriod)
{
	char    szQuery[MAX_STMT_SIZE*4], sFn[MAX_FUNCNAME_SIZE], sPeriod[MAX_STRPERIOD_SIZE];
	int     dRet;

	switch(cPeriod)
	{
		case FIVE_MIN_PERIOD:
			sprintf(sPeriod, "%s", "5MIN");
			break;
		case ONE_HOUR_PERIOD:
			sprintf(sPeriod, "%s", "1HOUR");
			break;
		case ONE_DAY_PERIOD:
			sprintf(sPeriod, "%s", "1DAY");
			break;
		default:
			log_print(LOGN_CRI, LH"unknown cPeriod[%d]", LT, cPeriod);
			return -1;
	}

	sprintf(szQuery,
		"CREATE TABLE `STAT_TRAFFIC_%s` ("
			"`STATTIME` bigint(20) NOT NULL default '0',"
			"`TAFID` int(11) NOT NULL default '0',"
			"`THRU_FRAMES` bigint(10) unsigned default NULL,"
			"`THRU_BYTES` bigint(20) unsigned default NULL,"
			"`TOT_FRAMES` bigint(10) unsigned default NULL,"
			"`TOT_BYTES` bigint(20) unsigned default NULL,"
			"`IP_FRAMES` bigint(10) unsigned default NULL,"
			"`IP_BYTES` bigint(20) unsigned default NULL,"
			"`UDP_FRAMES` bigint(10) unsigned default NULL,"
			"`UDP_BYTES` bigint(20) unsigned default NULL,"
			"`TCP_FRAMES` bigint(10) unsigned default NULL,"
			"`TCP_BYTES` bigint(20) unsigned default NULL,"
			"`SCTP_FRAMES` bigint(10) unsigned default NULL,"
			"`SCTP_BYTES` bigint(20) unsigned default NULL,"
			"`ETC_FRAMES` bigint(10) unsigned default NULL,"
			"`ETC_BYTES` bigint(20) unsigned default NULL,"
			"`IPERROR_FRAMES` bigint(10) unsigned default NULL,"
			"`IPERROR_BYTES` bigint(20) unsigned default NULL,"
			"`UTCP_FRAMES` bigint(10) unsigned default NULL,"
			"`UTCP_BYTES` bigint(20) unsigned default NULL,"
			"`FAILDATA_FRAMES` bigint(10) unsigned default NULL,"
			"`FAILDATA_BYTES` bigint(20) unsigned default NULL,"
			"`FILTEROUT_FRAMES` bigint(10) unsigned default NULL,"
			"`FILTEROUT_BYTES` bigint(20) unsigned default NULL,"
			"PRIMARY KEY  (`STATTIME`,`TAFID`)"
		") ENGINE=InnoDB DEFAULT CHARSET=euckr", sPeriod);

	sprintf(sFn, "%s", __FUNCTION__);
	if( (dRet = dCommonDDL(&stMySQL, sFn, (const char*)szQuery)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dCommonDDL() dRet[%d]", LT, dRet);
		return -2;
	}

	return 0;
}


void InsertErrorPrint_LOAD(int dLevel, st_LOAD *Data)
{
	log_print(dLevel, "########### STAT_LOAD ERR ############");
	log_print(dLevel, "[StatTime    ] : [%u]", Data->uiStatTime);
	log_print(dLevel, "[SystemType  ] : [%hu]", Data->usSystemType);
	log_print(dLevel, "[SystemID    ] : [%hu]", Data->usSystemID);
	log_print(dLevel, "[CPUAVG      ] : [%u]", Data->usCPUAVG);
	log_print(dLevel, "[CPUMAX      ] : [%u]", Data->usCPUMAX);
	log_print(dLevel, "[CPUMIN      ] : [%u]", Data->usCPUMIN);
	log_print(dLevel, "[MEMAVG      ] : [%u]", Data->usMEMAVG);
	log_print(dLevel, "[MEMMAX      ] : [%u]", Data->usMEMMAX);
	log_print(dLevel, "[MEMMIN      ] : [%u]", Data->usMEMMIN);
	log_print(dLevel, "[QUEAVG      ] : [%u]", Data->usQUEAVG);
	log_print(dLevel, "[QUEMAX      ] : [%u]", Data->usQUEMAX);
	log_print(dLevel, "[QUEMIN      ] : [%u]", Data->usQUEMIN);
	log_print(dLevel, "[NIFOAVG     ] : [%u]", Data->usNIFOAVG);
	log_print(dLevel, "[NIFOMAX     ] : [%u]", Data->usNIFOMAX);
	log_print(dLevel, "[NIFOMIN     ] : [%u]", Data->usNIFOMIN);
	log_print(dLevel, "[TRAFFICAVG  ] : [%u]", Data->uTrafficAVG);
	log_print(dLevel, "[TRAFFICMAX  ] : [%u]", Data->uTrafficMAX);
	log_print(dLevel, "[TRAFFICMIN  ] : [%u]", Data->uTrafficMIN);
	log_print(dLevel, "[DISK1AVG    ] : [%u]", Data->usDISK1AVG);
	log_print(dLevel, "[DISK1MAX    ] : [%u]", Data->usDISK1MAX);
	log_print(dLevel, "[DISK1MIN    ] : [%u]", Data->usDISK1MIN);
	log_print(dLevel, "[DISK2AVG    ] : [%u]", Data->usDISK2AVG);
	log_print(dLevel, "[DISK2MAX    ] : [%u]", Data->usDISK2MAX);
	log_print(dLevel, "[DISK2MIN    ] : [%u]", Data->usDISK2MIN);
	log_print(dLevel, "[DISK3AVG    ] : [%u]", Data->usDISK3AVG);
	log_print(dLevel, "[DISK3MAX    ] : [%u]", Data->usDISK3MAX);
	log_print(dLevel, "[DISK3MIN    ] : [%u]", Data->usDISK3MIN);
	log_print(dLevel, "[DISK4AVG    ] : [%u]", Data->usDISK4AVG);
	log_print(dLevel, "[DISK4MAX    ] : [%u]", Data->usDISK4MAX);
	log_print(dLevel, "[DISK4MIN    ] : [%u]", Data->usDISK4MIN);
	log_print(dLevel, "[COUNT       ] : [%hu]", Data->usCount);
}

void InsertErrorPrint_FAULT(int dLevel,st_FAULT *Data)
{
    log_print( dLevel, "########### STAT_FAULT ERR ############");
    log_print( dLevel, "[StatTime        ] : [%u]",Data->uiStatTime);
    log_print( dLevel, "[SystemType       ] : [%d]",Data->usSystemType);
    log_print( dLevel, "[SystemID     ] : [%d]",Data->usSystemID);
    log_print( dLevel, "[FAILTYPE     ] : [%s]",Data->szType);
    log_print( dLevel, "[CRITICAL     ] : [%u]",Data->uiCRI);
    log_print( dLevel, "[MAJOR     ] : [%u]",Data->uiMAJ);
    log_print( dLevel, "[MINOR     ] : [%u]",Data->uiMIN);
    log_print( dLevel, "[STOP     ] : [%u]",Data->uiSTOP);
    log_print( dLevel, "[NORMAL     ] : [%u]",Data->uiNORMAL);
}

void InsertErrorPrint_TRAFFIC(int dLevel, st_TRAFFIC *Data)
{
	log_print( dLevel, "########### STAT_FAULT ERR ############");
	log_print( dLevel, "[StatTime          ] : [%lu]",Data->uiStatTime);
	log_print( dLevel, "[TAFID             ] : [%hu]",Data->usTAFID);

	log_print( dLevel, "[ThruStat.uiFrames ] : [%u]",Data->ThruStat.uiFrames);
	log_print( dLevel, "[ThruStat.ulBytes	 ] : [%lu]",Data->ThruStat.ulBytes);

	log_print( dLevel, "[TotStat.uiFrames	 ] : [%u]",Data->TotStat.uiFrames);
	log_print( dLevel, "[TotStat.ulBytes	 ] : [%lu]",Data->TotStat.ulBytes);

	log_print( dLevel, "[IPStat.uiFrames	 ] : [%u]",Data->IPStat.uiFrames);
	log_print( dLevel, "[IPStat.ulBytes	 ] : [%lu]",Data->IPStat.ulBytes);

	log_print( dLevel, "[UDPStat.uiFrames	 ] : [%u]",Data->UDPStat.uiFrames);
	log_print( dLevel, "[UDPStat.ulBytes	 ] : [%lu]",Data->UDPStat.ulBytes);

	log_print( dLevel, "[TCPStat.uiFrames	 ] : [%u]",Data->TCPStat.uiFrames);
	log_print( dLevel, "[TCPStat.ulBytes	 ] : [%lu]",Data->TCPStat.ulBytes);

	log_print( dLevel, "[SCTPStat.uiFrames ] : [%u]",Data->SCTPStat.uiFrames);
	log_print( dLevel, "[SCTPStat.ulBytes	 ] : [%lu]",Data->SCTPStat.ulBytes);

	log_print( dLevel, "[IPError.uiFrames	 ] : [%u]",Data->IPError.uiFrames);
	log_print( dLevel, "[IPError.ulBytes	 ] : [%lu]",Data->IPError.ulBytes);

	log_print( dLevel, "[UTCPError.uiFrames] : [%u]",Data->UTCPError.uiFrames);
	log_print( dLevel, "[UTCPError.ulBytes ] : [%lu]",Data->UTCPError.ulBytes);

	log_print( dLevel, "[FailData.uiFrames ] : [%u]",Data->FailData.uiFrames);
	log_print( dLevel, "[FailData.ulBytes	 ] : [%lu]",Data->FailData.ulBytes);

	log_print( dLevel, "[FilterOut.uiFrames] : [%u]",Data->FilterOut.uiFrames);
	log_print( dLevel, "[FilterOut.ulBytes ] : [%lu]",Data->FilterOut.ulBytes);
}


















int dInsertLOADStatNew(MYSQL *pstMySQL, int dCount, st_LOAD *pstData)
{
	char	szBuf[MAX_SQLQUERY_SIZE];

	sprintf(szBuf,
		"INSERT INTO STAT_LOAD_5MIN VALUES "
			"(%u,%hu,%hu,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%hu)",
		pstData->uiStatTime, pstData->usSystemType, pstData->usSystemID,
		pstData->usCPUAVG, pstData->usCPUMAX, pstData->usCPUMIN,
		pstData->usMEMAVG, pstData->usMEMMAX, pstData->usMEMMIN,
		pstData->usQUEAVG, pstData->usQUEMAX, pstData->usQUEMIN,
		pstData->usNIFOAVG, pstData->usNIFOMAX, pstData->usNIFOMIN,
		pstData->uTrafficAVG, pstData->uTrafficMAX, pstData->uTrafficMIN,
		pstData->usDISK1AVG, pstData->usDISK1MAX, pstData->usDISK1MIN,
		pstData->usDISK2AVG, pstData->usDISK2MAX, pstData->usDISK2MIN,
		pstData->usDISK3AVG, pstData->usDISK3MAX, pstData->usDISK3MIN,
		pstData->usDISK4AVG, pstData->usDISK4MAX, pstData->usDISK4MIN,
		pstData->usCount);

	if(mysql_query(pstMySQL, szBuf) != 0)
	{
		log_print(LOGN_CRI, LH"FAILED IN mysql_query(%s) [errno:%d-%s]", LT,
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

int dInsertLOADStatPeriod(MYSQL *pstMySQL, time_t tInsertTbl, char cPeriod)
{
	char	szBuf[MAX_SQLQUERY_SIZE], sFromTable[MAX_STRTABLE_SIZE], sToTable[MAX_STRTABLE_SIZE];
	int		dPeriod;

	switch(cPeriod)
	{
		case ONE_HOUR_PERIOD:
			sprintf(sFromTable, "%s", "STAT_LOAD_5MIN");
			sprintf(sToTable, "%s", "STAT_LOAD_1HOUR");
			dPeriod = SEC_OF_HOUR;
			break;
		case ONE_DAY_PERIOD:
			sprintf(sFromTable, "%s", "STAT_LOAD_1HOUR");
			sprintf(sToTable, "%s", "STAT_LOAD_1DAY");
			dPeriod = SEC_OF_DAY;
			break;
		default:
			log_print(LOGN_CRI, LH"unknown cPeriod[%d]", LT, cPeriod);
			return -4;
	}

	sprintf(szBuf,
		"INSERT INTO %s "
		"("
			"STATTIME,SYSTEMTYPE,SYSTEMID,"
			"CPUAVG,CPUMAX,CPUMIN,"
			"MEMAVG,MEMMAX,MEMMIN,"
			"QUEAVG,QUEMAX,QUEMIN,"
			"NIFOAVG,NIFOMAX,NIFOMIN,"
			"TRAFFICAVG,TRAFFICMAX,TRAFFICMIN,"
			"DISK1AVG,DISK1MAX,DISK1MIN,"
			"DISK2AVG,DISK2MAX,DISK2MIN,"
			"DISK3AVG,DISK3MAX,DISK3MIN,"
			"DISK4AVG,DISK4MAX,DISK4MIN"
		") "
		"SELECT "
			"'%lu',SYSTEMTYPE,SYSTEMID,"
			"TRUNCATE(AVG(CPUAVG),0),TRUNCATE(AVG(CPUMAX),0),TRUNCATE(AVG(CPUMIN),0),"
			"TRUNCATE(AVG(MEMAVG),0),TRUNCATE(AVG(MEMMAX),0),TRUNCATE(AVG(MEMMIN),0),"
			"TRUNCATE(AVG(QUEAVG),0),TRUNCATE(AVG(QUEMAX),0),TRUNCATE(AVG(QUEMIN),0),"
			"TRUNCATE(AVG(NIFOAVG),0),TRUNCATE(AVG(NIFOMAX),0),TRUNCATE(AVG(NIFOMIN),0),"
			"TRUNCATE(AVG(TRAFFICAVG),0),TRUNCATE(AVG(TRAFFICMAX),0),TRUNCATE(AVG(TRAFFICMIN),0),"
			"TRUNCATE(AVG(DISK1AVG),0),TRUNCATE(AVG(DISK1MAX),0),TRUNCATE(AVG(DISK1MIN),0),"
			"TRUNCATE(AVG(DISK2AVG),0),TRUNCATE(AVG(DISK2MAX),0),TRUNCATE(AVG(DISK2MIN),0),"
			"TRUNCATE(AVG(DISK3AVG),0),TRUNCATE(AVG(DISK3MAX),0),TRUNCATE(AVG(DISK3MIN),0),"
			"TRUNCATE(AVG(DISK4AVG),0),TRUNCATE(AVG(DISK4MAX),0),TRUNCATE(AVG(DISK4MIN),0) "
		"FROM "
			"%s "
		"WHERE "
			"STATTIME>=('%lu'- '%d') AND STATTIME<'%lu' "
		"GROUP BY "
			"SYSTEMTYPE,SYSTEMID", sToTable, tInsertTbl, sFromTable, tInsertTbl, dPeriod, tInsertTbl);

	if(mysql_query(pstMySQL, szBuf) != 0)
	{
		log_print(LOGN_CRI, LH"FAILED IN mysql_query(%s) [errno:%d-%s]", LT,
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

int dInsertFAULTStatNew(MYSQL *pstMySQL, int dCount, st_FAULT *pstData)
{
	char	szBuf[MAX_SQLQUERY_SIZE];

	sprintf(szBuf,
		"INSERT INTO STAT_FAULT_5MIN VALUES "
			"(%u,%hu,%hu,'%s',%u,%u,%u,%u,%u)",
		pstData->uiStatTime, pstData->usSystemType, pstData->usSystemID, pstData->szType,
		pstData->uiCRI, pstData->uiMAJ, pstData->uiMIN, pstData->uiSTOP, pstData->uiNORMAL);

	if(mysql_query(pstMySQL, szBuf) != 0)
	{
		log_print(LOGN_CRI, LH"FAILED IN mysql_query(%s) [errno:%d-%s]", LT,
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

int dInsertFAULTStatPeriod(MYSQL *pstMySQL, time_t tInsertTbl, char cPeriod)
{
	char	szBuf[MAX_SQLQUERY_SIZE], sFromTable[MAX_STRTABLE_SIZE], sToTable[MAX_STRTABLE_SIZE];
	int		dPeriod;

	switch(cPeriod)
	{
		case ONE_HOUR_PERIOD:
			sprintf(sFromTable, "%s", "STAT_FAULT_5MIN");
			sprintf(sToTable, "%s", "STAT_FAULT_1HOUR");
			dPeriod = SEC_OF_HOUR;
			break;
		case ONE_DAY_PERIOD:
			sprintf(sFromTable, "%s", "STAT_FAULT_1HOUR");
			sprintf(sToTable, "%s", "STAT_FAULT_1DAY");
			dPeriod = SEC_OF_DAY;
			break;
		default:
			log_print(LOGN_CRI, LH"unknown cPeriod[%d]", LT, cPeriod);
			return -4;
	}

	sprintf(szBuf,
		"INSERT INTO %s "
		"("
			"STATTIME,SYSTEMTYPE,SYSTEMID,FAULTTYPE,"
			"CRI,MAJ,MIN,STOP,NORMAL"
		") "
		"SELECT "
			"'%lu',SYSTEMTYPE,SYSTEMID,FAULTTYPE,"
			"SUM(CRI),SUM(MAJ),SUM(MIN),SUM(STOP),SUM(NORMAL) "
		"FROM "
			"%s "
		"WHERE "
			"STATTIME>=('%lu'- '%d') AND STATTIME<'%lu' "
		"GROUP BY "
			"SYSTEMTYPE,SYSTEMID,FAULTTYPE",
		sToTable, tInsertTbl, sFromTable, tInsertTbl, dPeriod, tInsertTbl);

	if(mysql_query(pstMySQL, szBuf) != 0)
	{
		log_print(LOGN_CRI, LH"FAILED IN mysql_query(%s) [errno:%d-%s]", LT,
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

int dInsertTRAFFICStatNew(MYSQL *pstMySQL, st_TRAFFIC *pstData)
{
	char	szBuf[MAX_SQLQUERY_SIZE];

	sprintf(szBuf,
		"INSERT INTO STAT_TRAFFIC_5MIN VALUES "
			"(%lu,%hu,%u,%lu,%u,%lu,%u,%lu,%u,%lu,%u,%lu,%u,%lu,%u,%lu,%u,%lu,%u,%lu,%u,%lu,%u,%lu)",
		pstData->uiStatTime, pstData->usTAFID, pstData->ThruStat.uiFrames, pstData->ThruStat.ulBytes, pstData->TotStat.uiFrames, pstData->TotStat.ulBytes,
		pstData->IPStat.uiFrames, pstData->IPStat.ulBytes, pstData->UDPStat.uiFrames, pstData->UDPStat.ulBytes, pstData->TCPStat.uiFrames, pstData->TCPStat.ulBytes,
		pstData->SCTPStat.uiFrames, pstData->SCTPStat.ulBytes, pstData->ETCStat.uiFrames, pstData->ETCStat.ulBytes, pstData->IPError.uiFrames, pstData->IPError.ulBytes,
		pstData->UTCPError.uiFrames, pstData->UTCPError.ulBytes, pstData->FailData.uiFrames, pstData->FailData.ulBytes,
		pstData->FilterOut.uiFrames, pstData->FilterOut.ulBytes);

	if(mysql_query(pstMySQL, szBuf) != 0)
	{
		log_print(LOGN_CRI, LH"FAILED IN mysql_query(%s) [errno:%d-%s]", LT,
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

int dInsertTRAFFICStatPeriod(MYSQL *pstMySQL, time_t tInsertTbl, int dTAFID, char cPeriod)
{
	char	szBuf[MAX_SQLQUERY_SIZE], sFromTable[MAX_STRTABLE_SIZE], sToTable[MAX_STRTABLE_SIZE];
	int		dPeriod;

	switch(cPeriod)
	{
		case ONE_HOUR_PERIOD:
			sprintf(sFromTable, "%s", "STAT_TRAFFIC_5MIN");
			sprintf(sToTable, "%s", "STAT_TRAFFIC_1HOUR");
			dPeriod = SEC_OF_HOUR;
			break;
		case ONE_DAY_PERIOD:
			sprintf(sFromTable, "%s", "STAT_TRAFFIC_1HOUR");
			sprintf(sToTable, "%s", "STAT_TRAFFIC_1DAY");
			dPeriod = SEC_OF_DAY;
			break;
		default:
			log_print(LOGN_CRI, LH"unknown cPeriod[%d]", LT, cPeriod);
			return -4;
	}

	sprintf(szBuf,
		"INSERT INTO %s "
		"("
			"STATTIME,TAFID,"
			"THRU_FRAMES,THRU_BYTES,"
			"TOT_FRAMES,TOT_BYTES,"
			"IP_FRAMES,IP_BYTES,"
			"UDP_FRAMES,UDP_BYTES,"
			"TCP_FRAMES,TCP_BYTES,"
			"SCTP_FRAMES,SCTP_BYTES,"
			"ETC_FRAMES,ETC_BYTES,"
			"IPERROR_FRAMES,IPERROR_BYTES,"
			"UTCP_FRAMES,UTCP_BYTES,"
			"FAILDATA_FRAMES,FAILDATA_BYTES,"
			"FILTEROUT_FRAMES,FILTEROUT_BYTES "
		") "
		"SELECT "
			"'%lu',TAFID,"
			"SUM(THRU_FRAMES),SUM(THRU_BYTES),"
			"SUM(TOT_FRAMES),SUM(TOT_BYTES),"
			"SUM(IP_FRAMES),SUM(IP_BYTES),"
			"SUM(UDP_FRAMES),SUM(UDP_BYTES),"
			"SUM(TCP_FRAMES),SUM(TCP_BYTES),"
			"SUM(SCTP_FRAMES),SUM(SCTP_BYTES),"
			"SUM(ETC_FRAMES),SUM(ETC_BYTES),"
			"SUM(IPERROR_FRAMES),SUM(IPERROR_BYTES),"
			"SUM(UTCP_FRAMES),SUM(UTCP_BYTES),"
			"SUM(FAILDATA_FRAMES),SUM(FAILDATA_BYTES),"
			"SUM(FILTEROUT_FRAMES),SUM(FILTEROUT_BYTES) "
		"FROM "
			"%s "
		"WHERE "
			"STATTIME>=('%lu'- '%d') AND STATTIME<'%lu' AND TAFID='%d' "
		"GROUP BY "
			"TAFID",
		sToTable, tInsertTbl, sFromTable, tInsertTbl, dPeriod, tInsertTbl, dTAFID);

	if(mysql_query(pstMySQL, szBuf) != 0)
	{
		log_print(LOGN_CRI, LH"FAILED IN mysql_query(%s) [errno:%d-%s]", LT,
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

int dCommonDDL(MYSQL *pstMySQL, char *psCallFn, const char *psQuery)
{
	if(mysql_query(pstMySQL, psQuery) != 0)
	{
		log_print(LOGN_CRI, LH"FAILED IN mysql_query(%s) [errno:%d-%s]", LT,
			psQuery, mysql_errno(pstMySQL), mysql_error(pstMySQL));
		return -1;
	}

	return 0;
}
