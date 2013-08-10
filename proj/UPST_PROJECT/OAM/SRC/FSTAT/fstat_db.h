/**
	@file		fstat_db.h
	@author
	@version
	@date		2011-07-25
	@brief		fstat_db.c 헤더파일
*/

#ifndef __FSTAT_DB_H__
#define __FSTAT_DB_H__

/**
	Include headers
*/
#include "fstat_init.h"

/**
 *	Define constants
 */
#define MAX_FUNCNAME_SIZE			256
#define MAX_STRPERIOD_SIZE			128
#define MAX_STRTABLE_SIZE			128

/**
	Declare functions
*/
extern int dInsert_STAT_LOADData(st_LOAD *pstData, int dTotCount);
extern void Insert_STAT_LOADData(st_LOAD *pstData, int dTotCount);
extern int dInsert_STAT_FAULTData(st_FAULT *pstData, int dTotCount);
extern void Insert_STAT_FAULTData(st_FAULT *pstData, int dTotCount);
extern int dInsert_STAT_TRAFFICData(st_TRAFFIC *pstData, int dTotCount);
extern void Insert_STAT_TRAFFICData(st_TRAFFIC *pstData, int dTotCount);
extern int dCreate_LOAD_Table(char cPeriod);
extern int dCreate_FAULT_Table(char cPeriod);
extern int dCreate_TRAFFIC_Table(char cPeriod);
extern void InsertErrorPrint_LOAD(int dLevel, st_LOAD *Data);
extern void InsertErrorPrint_FAULT(int dLevel,st_FAULT *Data);
extern void InsertErrorPrint_TRAFFIC(int dLevel, st_TRAFFIC *Data);
extern int dInsertLOADStatNew(MYSQL *pstMySQL, int dCount, st_LOAD *pstData);
extern int dInsertLOADStatPeriod(MYSQL *pstMySQL, time_t tInsertTbl, char cPeriod);
extern int dInsertFAULTStatNew(MYSQL *pstMySQL, int dCount, st_FAULT *pstData);
extern int dInsertFAULTStatPeriod(MYSQL *pstMySQL, time_t tInsertTbl, char cPeriod);
extern int dInsertTRAFFICStatNew(MYSQL *pstMySQL, st_TRAFFIC *pstData);
extern int dInsertTRAFFICStatPeriod(MYSQL *pstMySQL, time_t tInsertTbl, int dTAFID, char cPeriod);
extern int dCommonDDL(MYSQL *pstMySQL, char *psCallFn, const char *psQuery);

#endif	/* __FSTAT_DB_H__ */
