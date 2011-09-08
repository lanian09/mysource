/**		file  stMEMSINFO_Prt.c
 *      - typedef를 위한 functions
 *
 *     Copyright (c) 2006~ by Upresto Inc, Korea
 *     All rights reserved
 *
 *		$Id: stMEMSINFO_Prt.c,v 1.1 2011/04/26 09:08:46 jjinri Exp $
 * 
 *     @Author      $Author: jjinri $
 *     @version     $Revision: 1.1 $
 *     @date        $Date: 2011/04/26 09:08:46 $
 *     @warning     $type???로 정의된 것들만 사용가능
 *     @ref         stMEMSINFO_Prt.c
 *     @todo        library를 만들기 위한 Makefile을 만들자
 *
 *     @section     Intro(소개)
 *      - typedef를 위한 functions
 *
 *     @section     Requirement
 *      @li 규칙에 틀린 곳을 찾아주세요.
 *
 **/


#include "mems.h"


/** stMEMSINFO_Prt function.
 *
 *  Printing Function
 *
 * @param *pcPrtPrefixStr 	: Print Prefix String
 * @param *pthis 		: Print변수 Pointer
 *
 *  @return 	void
 *  @see    	mems.h
 *
 *  @exception  규칙에 틀린 곳을 찾아주세요.
 *  @note 		structg.pl로 만들어진 자동 코드 
 **/
void stMEMSINFO_Prt(S8 *pcPrtPrefixStr, stMEMSINFO *pthis){
	FPRINTF(LOG_LEVEL,"_%s : pthis = %p",pcPrtPrefixStr, pthis);
	/* U32 	uiType */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiType = %u",pcPrtPrefixStr,(U32)(pthis->uiType));
	/* U32 			uiShmKey */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiShmKey = %u",pcPrtPrefixStr,(U32)(pthis->uiShmKey));
	/* OFFSET			uiTotMemSize */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiTotMemSize = %ld",pcPrtPrefixStr,(pthis->uiTotMemSize));
	/* U32				uiHeadRoomSize */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiHeadRoomSize = %u",pcPrtPrefixStr,(U32)(pthis->uiHeadRoomSize));
	/* U32				uiMemNodeHdrSize */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiMemNodeHdrSize = %u",pcPrtPrefixStr,(U32)(pthis->uiMemNodeHdrSize));
	/* OFFSET  		offsetHeadRoom */
	FPRINTF(LOG_LEVEL,"_%s : pthis->offsetHeadRoom = %ld",pcPrtPrefixStr,(pthis->offsetHeadRoom));
	/* U32				uiZoneCnt */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiZoneCnt = %u",pcPrtPrefixStr,(U32)(pthis->uiZoneCnt));
	/* st_MEMSZONE		stMEMSZONE[MAX_MEMSZONE_CNT] */
	{ int iIndex;
		for(iIndex = 0;iIndex < MAX_MEMSZONE_CNT;iIndex++){
			st_MEMSZONE_Prt(pcPrtPrefixStr, &(pthis->stMEMSZONE[iIndex]));
		}
	}
}
