/**		file  st_MEMSZONE_Prt.c
 *      - typedef�� ���� functions
 *
 *     Copyright (c) 2006~ by Upresto Inc, Korea
 *     All rights reserved
 *
 *		$Id: st_MEMSZONE_Prt.c,v 1.1 2011/04/26 09:08:46 jjinri Exp $
 * 
 *     @Author      $Author: jjinri $
 *     @version     $Revision: 1.1 $
 *     @date        $Date: 2011/04/26 09:08:46 $
 *     @warning     $type???�� ���ǵ� �͵鸸 ��밡��
 *     @ref         st_MEMSZONE_Prt.c
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


/** st_MEMSZONE_Prt function.
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
void st_MEMSZONE_Prt(S8 *pcPrtPrefixStr, st_MEMSZONE *pthis){
	FPRINTF(LOG_LEVEL,"_%s : pthis = %p",pcPrtPrefixStr, pthis);
	/* U32 	uiSemFlag */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiSemFlag = %u",pcPrtPrefixStr,(U32)(pthis->uiSemFlag));
	/* U32		uiSemKey */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiSemKey = %u",pcPrtPrefixStr,(U32)(pthis->uiSemKey));
	/* S32		iSemID */
	FPRINTF(LOG_LEVEL,"_%s : pthis->iSemID = %d",pcPrtPrefixStr,(pthis->iSemID));
	/* U32		uiZoneID */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiZoneID = %u",pcPrtPrefixStr,(U32)(pthis->uiZoneID));
	/* U32		uiMemNodeBodySize */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiMemNodeBodySize = %u",pcPrtPrefixStr,(U32)(pthis->uiMemNodeBodySize));
	/* U32		uiMemNodeAllocedCnt */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiMemNodeAllocedCnt = %u",pcPrtPrefixStr,(U32)(pthis->uiMemNodeAllocedCnt));
	/* U32		uiMemNodeTotCnt */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiMemNodeTotCnt = %u",pcPrtPrefixStr,(U32)(pthis->uiMemNodeTotCnt));
	/* OFFSET  offsetNodeStart */
	FPRINTF(LOG_LEVEL,"_%s : pthis->offsetNodeStart = %ld",pcPrtPrefixStr,(pthis->offsetNodeStart));
	/* OFFSET  offsetFreeList */
	FPRINTF(LOG_LEVEL,"_%s : pthis->offsetFreeList = %ld",pcPrtPrefixStr,(pthis->offsetFreeList));
	/* OFFSET  offsetNodeEnd */
	FPRINTF(LOG_LEVEL,"_%s : pthis->offsetNodeEnd = %ld",pcPrtPrefixStr,(pthis->offsetNodeEnd));
	/* U64		createCnt */
	FPRINTF(LOG_LEVEL,"_%s : pthis->createCnt = %llu",pcPrtPrefixStr,(pthis->createCnt));
	/* U64		delCnt */
	FPRINTF(LOG_LEVEL,"_%s : pthis->delCnt = %llu",pcPrtPrefixStr,(pthis->delCnt));
}
