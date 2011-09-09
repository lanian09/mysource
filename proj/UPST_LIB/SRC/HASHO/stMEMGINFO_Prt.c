/**		file  stMEMGINFO_Prt.c
 *      - typedef�� ���� functions
 *
 *     Copyright (c) 2006~ by Upresto Inc, Korea
 *     All rights reserved
 *
 *		$Id: stMEMGINFO_Prt.c,v 1.1.1.1 2011/08/19 00:53:28 uamyd Exp $
 * 
 *     @Author      $Author: uamyd $
 *     @version     $Revision: 1.1.1.1 $
 *     @date        $Date: 2011/08/19 00:53:28 $
 *     @warning     $type???�� ���ǵ� �͵鸸 ��밡��
 *     @ref         stMEMGINFO_Prt.c
 *     @todo        library�� ����� ���� Makefile�� ������
 *
 *     @section     Intro(�Ұ�)
 *      - typedef�� ���� functions
 *
 *     @section     Requirement
 *      @li ��Ģ�� Ʋ�� ���� ã���ּ���.
 *
 **/


#include "memg.h"


/** stMEMGINFO_Prt function.
 *
 *  Printing Function
 *
 * @param *pcPrtPrefixStr 	: Print Prefix String
 * @param *pthis 		: Print���� Pointer
 *
 *  @return 	void
 *  @see    	memg.h
 *
 *  @exception  ��Ģ�� Ʋ�� ���� ã���ּ���.
 *  @note 		structg.pl�� ������� �ڵ� �ڵ� 
 **/
void stMEMGINFO_Prt(S8 *pcPrtPrefixStr, stMEMGINFO *pthis){
	FPRINTF(LOG_LEVEL,"_%s : pthis = %p",pcPrtPrefixStr, pthis);
	/* U32 uiType */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiType = %u",pcPrtPrefixStr,(U32)(pthis->uiType));
	/* U32 uiShmKey */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiShmKey = %u",pcPrtPrefixStr,(U32)(pthis->uiShmKey));
	/* U32 uiTotMemSize */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiTotMemSize = %u",pcPrtPrefixStr,(U32)(pthis->uiTotMemSize));
	/* U32 uiHeadRoomSize */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiHeadRoomSize = %u",pcPrtPrefixStr,(U32)(pthis->uiHeadRoomSize));
	/* U32 uiMemNodeHdrSize */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiMemNodeHdrSize = %u",pcPrtPrefixStr,(U32)(pthis->uiMemNodeHdrSize));
	/* U32 uiMemNodeBodySize */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiMemNodeBodySize = %u",pcPrtPrefixStr,(U32)(pthis->uiMemNodeBodySize));
	/* U32 uiMemNodeAllocedCnt */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiMemNodeAllocedCnt = %u",pcPrtPrefixStr,(U32)(pthis->uiMemNodeAllocedCnt));
	/* U32 uiMemNodeTotCnt */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiMemNodeTotCnt = %u",pcPrtPrefixStr,(U32)(pthis->uiMemNodeTotCnt));
	/* OFFSET offsetHeadRoom */
	FPRINTF(LOG_LEVEL,"_%s : pthis->offsetHeadRoom = %ld",pcPrtPrefixStr,(pthis->offsetHeadRoom));
	/* OFFSET offsetNodeStart */
	FPRINTF(LOG_LEVEL,"_%s : pthis->offsetNodeStart = %ld",pcPrtPrefixStr,(pthis->offsetNodeStart));
	/* OFFSET offsetFreeList */
	FPRINTF(LOG_LEVEL,"_%s : pthis->offsetFreeList = %ld",pcPrtPrefixStr,(pthis->offsetFreeList));
	/* OFFSET offsetNodeEnd */
	FPRINTF(LOG_LEVEL,"_%s : pthis->offsetNodeEnd = %ld",pcPrtPrefixStr,(pthis->offsetNodeEnd));
}
