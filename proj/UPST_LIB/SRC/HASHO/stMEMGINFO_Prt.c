/**		file  stMEMGINFO_Prt.c
 *      - typedef를 위한 functions
 *
 *     Copyright (c) 2006~ by Upresto Inc, Korea
 *     All rights reserved
 *
 *		$Id: stMEMGINFO_Prt.c,v 1.1.1.1 2011/08/19 00:53:28 uamyd Exp $
 * 
 *     @Author      $Author: uamyd $
 *     @version     $Revision: 1.1.1.1 $
 *     @date        $Date: 2011/08/19 00:53:28 $
 *     @warning     $type???로 정의된 것들만 사용가능
 *     @ref         stMEMGINFO_Prt.c
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


/** stMEMGINFO_Prt function.
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
