/**		file  stMEMSNODEHDR_Prt.c
 *      - typedef�� ���� functions
 *
 *     Copyright (c) 2006~ by Upresto Inc, Korea
 *     All rights reserved
 *
 *		$Id: stMEMSNODEHDR_Prt.c,v 1.1.1.1 2011/08/19 00:53:28 uamyd Exp $
 * 
 *     @Author      $Author: uamyd $
 *     @version     $Revision: 1.1.1.1 $
 *     @date        $Date: 2011/08/19 00:53:28 $
 *     @warning     $type???�� ���ǵ� �͵鸸 ��밡��
 *     @ref         stMEMSNODEHDR_Prt.c
 *     @todo        library�� ����� ���� Makefile�� ������
 *
 *     @section     Intro(�Ұ�)
 *      - typedef�� ���� functions
 *
 *     @section     Requirement
 *      @li ��Ģ�� Ʋ�� ���� ã���ּ���.
 *
 **/


#include "mems.h"


/** stMEMSNODEHDR_Prt function.
 *
 *  Printing Function
 *
 * @param *pcPrtPrefixStr 	: Print Prefix String
 * @param *pthis 		: Print���� Pointer
 *
 *  @return 	void
 *  @see    	mems.h
 *
 *  @exception  ��Ģ�� Ʋ�� ���� ã���ּ���.
 *  @note 		structg.pl�� ������� �ڵ� �ڵ� 
 **/
void stMEMSNODEHDR_Prt(S8 *pcPrtPrefixStr, stMEMSNODEHDR *pthis){
	FPRINTF(LOG_LEVEL,"_%s : pthis = %p",pcPrtPrefixStr, pthis);
	/* U32		uiID */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiID = %u",pcPrtPrefixStr,(U32)(pthis->uiID));
	/* U32		uiZoneID */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiZoneID = %u",pcPrtPrefixStr,(U32)(pthis->uiZoneID));
	/* STIME	TimeSec */
	{    S8 STG_PrintPre[1024]; S8 tmp[1024]; time_t ttCallTime = pthis->TimeSec; if(pthis->TimeSec){ strftime(STG_PrintPre, 1024, "%Y%m%d%H%M%S", localtime((time_t *)&ttCallTime)); sprintf(tmp, "\t%u", pthis->TimeSec); strcat(STG_PrintPre, tmp); } else { sprintf(STG_PrintPre,"0\t0"); } 
		FPRINTF(LOG_LEVEL,"_%s : pthis->TimeSec = %s",pcPrtPrefixStr,STG_PrintPre);
	}
	/* U8 	ucIsFree */
	FPRINTF(LOG_LEVEL,"_%s : pthis->ucIsFree = %d",pcPrtPrefixStr,(pthis->ucIsFree));
	/* S8		DebugStr[MEMS_MAX_DEBUG_STR] */
	pthis->DebugStr[MEMS_MAX_DEBUG_STR - 1] = 0;
	FPRINTF(LOG_LEVEL,"_%s : pthis->DebugStr = %s" ,pcPrtPrefixStr,pthis->DebugStr);
	/* S8		DelStr[MEMS_MAX_DEBUG_STR] */
	pthis->DelStr[MEMS_MAX_DEBUG_STR - 1] = 0;
	FPRINTF(LOG_LEVEL,"_%s : pthis->DelStr = %s" ,pcPrtPrefixStr,pthis->DelStr);
}
