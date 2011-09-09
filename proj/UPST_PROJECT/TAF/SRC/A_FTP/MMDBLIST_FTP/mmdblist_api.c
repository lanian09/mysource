/**A.1*  File Inclusion ***********************************/
#include <common_stg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/types.h>

//#include <utillib.h>
#include <mmdblist_ftp.h>
#include "loglib.h"

/**B.1*  Definition of New Constants *********************/

/**B.2*  Definition of New Type  **************************/

/**C.1*  Declaration of Variables  ************************/

/**D.1*  Definition of Functions  *************************/

/**D.2*  Definition of Functions  *************************/
int dInsertMMDB(PSESS_DATA pstSrc, PSESS_DATA *ppstDst)
{
	int				dRet = 0;
	PSESS_DATA		pstCheck;

	dRet = Insert_SESS(pstSrc);
	if(dRet < 0)	/* RESOURCE ERROR */
	{
		log_print(LOGN_WARN,"MMDB RESOURCE ERROR INSERT[%d][%u:%u][%u:%u]", 
			dRet, 
			pstSrc->stKey.uiSrcIP, pstSrc->stKey.usSrcPort, 
			pstSrc->stKey.uiDestIP, pstSrc->stKey.usDestPort );
		return -2;
	}
	else if(dRet > 0)   /* EXIST DATA */
    {
 		log_print(LOGN_WARN,"MMDB ALEADY EXIST INSERT[%d][%u:%u][%u:%u]",
			dRet,
			pstSrc->stKey.uiSrcIP, pstSrc->stKey.usSrcPort, 
			pstSrc->stKey.uiDestIP, pstSrc->stKey.usDestPort );
    	return -100;
    }

	pstCheck = Search_SESS(&pstSrc->stKey);
	if(pstCheck == NULL)
	{
		log_print(LOGN_WARN,"MMDB NOT EXIST[%u:%u][%u:%u]", 
			pstSrc->stKey.uiSrcIP, pstSrc->stKey.usSrcPort, 
			pstSrc->stKey.uiDestIP, pstSrc->stKey.usDestPort );
		return -3;
	}

	/* Current Session Count + 1 */
	pstSessTbl->uiSessCount++;

	*ppstDst = pstCheck;

	return 0;
}

int dSetMMDB(PSESS_KEY pstKey, PSESS_DATA *pstMMDB)
{
	/* MSG DB INSERT */
	*pstMMDB = Search_SESS(pstKey);
	if(*pstMMDB == NULL)
	{
		return -1;
	}

	return 0;
}

int dFreeMMDB(PSESS_DATA pstMMDB)
{
	int				dRet;

	/* MMDB KEY Validation */
	if(	(pstMMDB->stKey.uiSrcIP == 0 && pstMMDB->stKey.usSrcPort == 0 
				&& pstMMDB->stKey.uiDestIP == 0 && pstMMDB->stKey.usDestPort == 0
				&& pstMMDB->stKey.uiReserved == 0 ) 
			|| ( pstMMDB->stKey.uiSrcIP == 0xFFFFFFFF && pstMMDB->stKey.usSrcPort == 0xFFFF 
				&& pstMMDB->stKey.uiDestIP == 0xFFFFFFFF && pstMMDB->stKey.usDestPort == 0xFFFF 
				&& pstMMDB->stKey.uiReserved == 0xFFFFFFFF) )
		return -2;

	/* MMDB Session DELETE Flow */
	dRet = Delete_SESS(&pstMMDB->stKey);
	if(dRet <  0)
	{
		log_print(LOGN_WARN,"MMDB_DELETE ERROR[%d][%u:%u][%u:%u]", 
				dRet,
				pstMMDB->stKey.uiSrcIP, pstMMDB->stKey.usSrcPort, 
				pstMMDB->stKey.uiDestIP, pstMMDB->stKey.usDestPort );
		return -1;
	}
	else if(dRet == 1)
	{
		log_print(LOGN_WARN,"MMDB_DELETE NOT FOUND[%d][%u:%u][%u:%u]",
				dRet,
				pstMMDB->stKey.uiSrcIP, pstMMDB->stKey.usSrcPort, 
				pstMMDB->stKey.uiDestIP, pstMMDB->stKey.usDestPort );
		return 1;
	}

	/* Current Session Count - 1 */
	pstSessTbl->uiSessCount--;


	return 0;	
}

