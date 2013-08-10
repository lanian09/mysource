/**		file  stTIMERNKEY_Prt.c
 *      - typedef�� ���� functions
 *
 *     Copyright (c) 2006~ by Upresto Inc, Korea
 *     All rights reserved
 *
 *		$Id: stTIMERNKEY_Prt.c,v 1.1.1.1 2011/08/19 00:53:28 uamyd Exp $
 * 
 *     @Author      $Author: uamyd $
 *     @version     $Revision: 1.1.1.1 $
 *     @date        $Date: 2011/08/19 00:53:28 $
 *     @warning     $type???�� ���ǵ� �͵鸸 ��밡��
 *     @ref         stTIMERNKEY_Prt.c
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


/** stTIMERNKEY_Prt function.
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
void stTIMERNKEY_Prt(S8 *pcPrtPrefixStr, stTIMERNKEY *pthis){
	FPRINTF(LOG_LEVEL,"_%s : pthis = %p",pcPrtPrefixStr, pthis);
	/* U32	uiTimerNIdIndex */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiTimerNIdIndex = %u",pcPrtPrefixStr,(U32)(pthis->uiTimerNIdIndex));
	/* STIME 	sTimeKey */
	{    S8 STG_PrintPre[1024]; S8 tmp[1024]; time_t ttCallTime = pthis->sTimeKey; if(pthis->sTimeKey){ strftime(STG_PrintPre, 1024, "%Y%m%d%H%M%S", localtime((time_t *)&ttCallTime)); sprintf(tmp, "\t%u", pthis->sTimeKey); strcat(STG_PrintPre, tmp); } else { sprintf(STG_PrintPre,"0\t0"); } 
		FPRINTF(LOG_LEVEL,"_%s : pthis->sTimeKey = %s",pcPrtPrefixStr,STG_PrintPre);
	}
}
