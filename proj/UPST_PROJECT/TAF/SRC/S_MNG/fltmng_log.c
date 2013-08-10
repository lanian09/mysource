/** A.1 * File Include *************************************************************/
#include <common_stg.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>

/*
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#include <taf_svc.h>
#include <utillib.h>
#include <init_shm.h>
#include <mmcid.h>
#include <taf_mmcd.h>
#include <dqms_define.h>
*/

#include <filter.h> 		/* st_AlmLevel_List */
#include "loglib.h"

/** B.1 * Definition of New Constants **********************************************/
/** B.1 * Definition of New Type ***************************************************/

/** C.1 * Declaration of Variables *************************************************/
/** C.2 * Declaration of External Variables ****************************************/
extern st_Flt_Info		*flt_info;
/** D.1 * Definition of Functions **************************************************/

void print_AlmLevel(st_AlmLevel_List *pstAlmLevelList)
{
	int i;

	log_print(LOGN_WARN, "############ ALARM LEVEL #############");
	log_print(LOGN_WARN, "ALARM COUNT [%d]",  pstAlmLevelList->dCount);
	for(i = 0; i < pstAlmLevelList->dCount; i++)
	{
		log_print(LOGN_WARN, "ALARM LEVEL[%d].sCriticalLevel = [%d]", i, pstAlmLevelList->stAlmLevel[i].sCriticalLevel);
		log_print(LOGN_WARN, "ALARM LEVEL[%d].sMajorLevel = [%d]", i, pstAlmLevelList->stAlmLevel[i].sMajorLevel);
		log_print(LOGN_WARN, "ALARM LEVEL[%d].sMinorLevel = [%d]", i, pstAlmLevelList->stAlmLevel[i].sMinorLevel);
	}
}

void print_FltCommon(st_Flt_Common *pstFltCommon)
{
	log_print(LOGN_WARN, "########### FLT_COMMON ############");
	log_print(LOGN_WARN, "FLT_COMON usCheckInterval = [%u]",
		pstFltCommon->usCheckInterval);
	log_print(LOGN_WARN, "FLT_COMON usRepeatCnt = [%u]",
		pstFltCommon->usRepeatCnt);
	log_print(LOGN_WARN, "FLT_COMON usTCPLongLast = [%u]",
		pstFltCommon->usTCPLongLast);
	log_print(LOGN_WARN, "FLT_COMON bOnOffState = [%u]",
		pstFltCommon->bOnOffState);
}



