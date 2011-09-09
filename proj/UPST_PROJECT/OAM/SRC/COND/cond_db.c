/***** A.1 * File Include *******************************/

/* SYS HEADER */
#include <stdio.h>
#include <string.h> /* STRLEN(3), MEMSET(3) */
#include <stdlib.h> /* EXIT(3) */
/* LIB HEADER */
#include "loglib.h" /* log_print */
#include "dblib.h"  /* MYSQL, st_ConnInfo */
/* PRO HEADER */
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
#include "almstat.h"
/* LOC HEADER */
#include "cond_db.h"

extern MYSQL stMySQL;

void vErrPrint(char* szAlm, char* szMsg, unsigned char TAMID, unsigned char TAFID)
{
	pst_almsts palm;
	palm = (pst_almsts)szAlm;

	log_print(LOGN_DEBUG,"######## ERROR COND DATA PRINT #########");
	log_print(LOGN_DEBUG,"CREATE TIME :%ld ",palm->tWhen);
	log_print(LOGN_DEBUG,"SYSTEM TYPE :%u ",palm->ucSysType);
	log_print(LOGN_DEBUG,"NTAM ID     :%u ",TAMID);
	log_print(LOGN_DEBUG,"NTAF ID     :%u ",TAFID);
	log_print(LOGN_DEBUG,"LOC TYPE    :%hu ",palm->ucLocType);
	log_print(LOGN_DEBUG,"INV TYPE    :%u ",palm->ucInvType);
	log_print(LOGN_DEBUG,"INV NO      :%u ",palm->ucInvNo);
	log_print(LOGN_DEBUG,"IP ADDRESS  :%u ",palm->uiIPAddr);
	log_print(LOGN_DEBUG,"MESSAGE     :%s",szMsg);
	log_print(LOGN_DEBUG,"######## END OF ERROR DATA #############");
	return;
}

void getCreateQuery(char *szQuery)
{
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

	return;
}

void getInsertQuery(char *szQuery, char* szAlm, char* szMsg, unsigned char TAMID, unsigned char TAFID)
{
	pst_almsts palm;
	
	palm = (pst_almsts)szAlm;
	
	sprintf(szQuery, "INSERT INTO SYS_COND_MSG VALUES (%u, %d, %d, %d, %d, %d, %d, %u, \'%s\')",
		(unsigned int)palm->tWhen, palm->ucSysType, TAMID, TAFID, 
		palm->ucLocType, palm->ucInvType, palm->ucInvNo, palm->uiIPAddr,
		szMsg);

	return;
}


int dInsert_CONDResult(char* szAlm, char* szMsg, int dLen, unsigned char ucTAMID, unsigned char ucTAFID)
{
	int		   dRet;
	char       szIQuery[1024], szCQuery[1024];
	st_almsts *palm;

	//memset( szIQuery, 0x00, 1024 );
	//memset( szCQuery, 0x00, 1024 );

	palm     = (st_almsts*)szAlm;
	getInsertQuery(szIQuery, szAlm, szMsg, ucTAMID, ucTAFID);

	if( (dRet = db_insert(&stMySQL, szIQuery)) < 0 ){
		switch( db_errno(&stMySQL) ){
			case E_DB_NOT_CONNECT:
				log_print(LOGN_WARN, LH"FAILED IN connect DB",LT);
				break;
			case E_DB_TABLE_NOT_EXIST:
				getCreateQuery(szCQuery);
				if( (dRet = db_create(&stMySQL, szCQuery)) == 0 ){
					if( (dRet = db_insert(&stMySQL, szIQuery)) < 0 ){
						log_print(LOGN_CRI, LH"ERROR IN db_insert(SYS_COND_MSG), dRet=%d\nQuery=%s",
							LT, dRet,szIQuery);
						return dRet;
					}
					log_print(LOGN_CRI, LH"SUCCESS IN db_created(SYS_COND_MSG)",LT);
				}
				log_print(LOGN_CRI, LH"ERROR IN db_create(SYS_COND_MSG), dRet=%d",
					LT, dRet);
				break;
			case E_DB_DUPLICATED_ENTRY:
				log_print(LOGN_WARN, LH"FAILED IN db_insert(DUPLICATED)",LT);
				vErrPrint(szAlm, szMsg, ucTAMID, ucTAFID);
				return 0;
				break;
			default:
				log_print(LOGN_CRI, LH"FAILED IN db_insert(), dRet=%d", LT, dRet);
				vErrPrint(szAlm, szMsg, ucTAMID, ucTAFID);
				break;
			
		}
		return dRet;
	}

	log_print(LOGN_DEBUG,"SUCCESS INSERT COND MSG TO DB\nQuery=%s",szIQuery);
	return 0;
}
