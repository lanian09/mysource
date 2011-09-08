/**		file  stMEMGNODEHDR_Prt.c
 *      - typedef를 위한 functions
 *
 *     Copyright (c) 2006~ by Upresto Inc, Korea
 *     All rights reserved
 *
 *		$Id: stMEMGNODEHDR_Prt.c,v 1.1.1.1 2011/08/19 00:53:28 uamyd Exp $
 * 
 *     @Author      $Author: uamyd $
 *     @version     $Revision: 1.1.1.1 $
 *     @date        $Date: 2011/08/19 00:53:28 $
 *     @warning     $type???로 정의된 것들만 사용가능
 *     @ref         stMEMGNODEHDR_Prt.c
 *     @todo        library를 만들기 위한 Makefile을 만들자
 *
 *     @section     Intro(소개)
 *      - typedef를 위한 functions
 *
 *     @section     Requirement
 *      @li 규칙에 틀린 곳을 찾아주세요.
 *
 **/


#include "memg.h"


/** stMEMGNODEHDR_Prt function.
 *
 *  Printing Function
 *
 * @param *pcPrtPrefixStr 	: Print Prefix String
 * @param *pthis 		: Print변수 Pointer
 *
 *  @return 	void
 *  @see    	memg.h
 *
 *  @exception  규칙에 틀린 곳을 찾아주세요.
 *  @note 		structg.pl로 만들어진 자동 코드 
 **/
void stMEMGNODEHDR_Prt(S8 *pcPrtPrefixStr, stMEMGNODEHDR *pthis){
	FPRINTF(LOG_LEVEL,"_%s : pthis = %p",pcPrtPrefixStr, pthis);
	/* U32	uiID */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiID = %u",pcPrtPrefixStr,(U32)(pthis->uiID));
	/* STIME	TimeSec */
	{    S8 STG_PrintPre[1024]; S8 tmp[1024]; time_t ttCallTime = pthis->TimeSec; if(pthis->TimeSec){ strftime(STG_PrintPre, 1024, "%Y%m%d%H%M%S", localtime((time_t *)&ttCallTime)); sprintf(tmp, "\t%u", pthis->TimeSec); strcat(STG_PrintPre, tmp); } else { sprintf(STG_PrintPre,"0\t0"); } 
		FPRINTF(LOG_LEVEL,"_%s : pthis->TimeSec = %s",pcPrtPrefixStr,STG_PrintPre);
	}
	/* U8	ucIsFree */
	FPRINTF(LOG_LEVEL,"_%s : pthis->ucIsFree = %d",pcPrtPrefixStr,(pthis->ucIsFree));
	/* S8	DebugStr[MEMG_MAX_DEBUG_STR] */
	pthis->DebugStr[MEMG_MAX_DEBUG_STR - 1] = 0;
	FPRINTF(LOG_LEVEL,"_%s : pthis->DebugStr = %s" ,pcPrtPrefixStr,pthis->DebugStr);
}
