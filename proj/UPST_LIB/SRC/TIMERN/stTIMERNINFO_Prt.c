/**		file  stTIMERNINFO_Prt.c
 *      - typedef를 위한 functions
 *
 *     Copyright (c) 2006~ by Upresto Inc, Korea
 *     All rights reserved
 *
 *		$Id: stTIMERNINFO_Prt.c,v 1.1.1.1 2011/08/19 00:53:28 uamyd Exp $
 * 
 *     @Author      $Author: uamyd $
 *     @version     $Revision: 1.1.1.1 $
 *     @date        $Date: 2011/08/19 00:53:28 $
 *     @warning     $type???로 정의된 것들만 사용가능
 *     @ref         stTIMERNINFO_Prt.c
 *     @todo        library를 만들기 위한 Makefile을 만들자
 *
 *     @section     Intro(소개)
 *      - typedef를 위한 functions
 *
 *     @section     Requirement
 *      @li 규칙에 틀린 곳을 찾아주세요.
 *
 **/


#include "timerN.h"


/** stTIMERNINFO_Prt function.
 *
 *  Printing Function
 *
 * @param *pcPrtPrefixStr 	: Print Prefix String
 * @param *pthis 		: Print변수 Pointer
 *
 *  @return 	void
 *  @see    	timerN.h
 *
 *  @exception  규칙에 틀린 곳을 찾아주세요.
 *  @note 		structg.pl로 만들어진 자동 코드 
 **/
void stTIMERNINFO_Prt(S8 *pcPrtPrefixStr, stTIMERNINFO *pthis){
	FPRINTF(LOG_LEVEL,"_%s : pthis = %p",pcPrtPrefixStr, pthis);
	/* void  *pstHASHGINFO   */
	FPRINTF(LOG_LEVEL,"_%s : pthis->pstHASHGINFO   = %p",pcPrtPrefixStr,(pthis->pstHASHGINFO  ));
	/* U32 uiMaxNodeCnt */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiMaxNodeCnt = %u",pcPrtPrefixStr,(U32)(pthis->uiMaxNodeCnt));
	/* U32 uiNodeCnt */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiNodeCnt = %u",pcPrtPrefixStr,(U32)(pthis->uiNodeCnt));
	/* U32 uiArgMaxSize */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiArgMaxSize = %u",pcPrtPrefixStr,(U32)(pthis->uiArgMaxSize));
	/* U32	uiTimerNIdIndex */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiTimerNIdIndex = %u",pcPrtPrefixStr,(U32)(pthis->uiTimerNIdIndex));
	/* STIME	uiCurrentTime */
	{    S8 STG_PrintPre[1024]; S8 tmp[1024]; time_t ttCallTime = pthis->uiCurrentTime; if(pthis->uiCurrentTime){ strftime(STG_PrintPre, 1024, "%Y%m%d%H%M%S", localtime((time_t *)&ttCallTime)); sprintf(tmp, "\t%u", pthis->uiCurrentTime); strcat(STG_PrintPre, tmp); } else { sprintf(STG_PrintPre,"0\t0"); } 
		FPRINTF(LOG_LEVEL,"_%s : pthis->uiCurrentTime = %s",pcPrtPrefixStr,STG_PrintPre);
	}
}
