/**		file  stTIMERNINFO_Prt.c
 *      - typedef�� ���� functions
 *
 *     Copyright (c) 2006~ by Upresto Inc, Korea
 *     All rights reserved
 *
 *		$Id: stTIMERNINFO_Prt.c,v 1.1.1.1 2011/08/19 00:53:28 uamyd Exp $
 * 
 *     @Author      $Author: uamyd $
 *     @version     $Revision: 1.1.1.1 $
 *     @date        $Date: 2011/08/19 00:53:28 $
 *     @warning     $type???�� ���ǵ� �͵鸸 ��밡��
 *     @ref         stTIMERNINFO_Prt.c
 *     @todo        library�� ����� ���� Makefile�� ������
 *
 *     @section     Intro(�Ұ�)
 *      - typedef�� ���� functions
 *
 *     @section     Requirement
 *      @li ��Ģ�� Ʋ�� ���� ã���ּ���.
 *
 **/


#include "timerN.h"


/** stTIMERNINFO_Prt function.
 *
 *  Printing Function
 *
 * @param *pcPrtPrefixStr 	: Print Prefix String
 * @param *pthis 		: Print���� Pointer
 *
 *  @return 	void
 *  @see    	timerN.h
 *
 *  @exception  ��Ģ�� Ʋ�� ���� ã���ּ���.
 *  @note 		structg.pl�� ������� �ڵ� �ڵ� 
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
