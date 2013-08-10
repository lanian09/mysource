/**		file  stMEMSINFO_Prt.c
 *      - typedef를 위한 functions
 *
 *     Copyright (c) 2006~ by Upresto Inc, Korea
 *     All rights reserved
 *
 *		$Id: debug_func.c,v 1.1.1.1 2011/04/19 14:13:42 june Exp $
 * 
 *     @Author      $Author: june $
 *     @version     $Revision: 1.1.1.1 $
 *     @date        $Date: 2011/04/19 14:13:42 $
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
	FPRINTF(LOG_LEVEL,"_%s : pthis = %p\n",pcPrtPrefixStr, pthis);
	/* U32 	uiType */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiType = %u\n",pcPrtPrefixStr,(U32)(pthis->uiType));
	/* U32 uiShmKey */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiShmKey = %u\n",pcPrtPrefixStr,(U32)(pthis->uiShmKey));
	/* U32 	uiSemFlag */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiSemFlag = %u\n",pcPrtPrefixStr,(U32)(pthis->uiSemFlag));
	/* U32	uiSemKey */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiSemKey = %u\n",pcPrtPrefixStr,(U32)(pthis->uiSemKey));
	/* S32	iSemID */
	FPRINTF(LOG_LEVEL,"_%s : pthis->iSemID = %d\n",pcPrtPrefixStr,(pthis->iSemID));
	/* U32 uiTotMemSize */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiTotMemSize = %u\n",pcPrtPrefixStr,(U32)(pthis->uiTotMemSize));
	/* U32 uiHeadRoomSize */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiHeadRoomSize = %u\n",pcPrtPrefixStr,(U32)(pthis->uiHeadRoomSize));
	/* U32 uiMemNodeHdrSize */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiMemNodeHdrSize = %u\n",pcPrtPrefixStr,(U32)(pthis->uiMemNodeHdrSize));
	/* U32 uiMemNodeBodySize */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiMemNodeBodySize = %u\n",pcPrtPrefixStr,(U32)(pthis->uiMemNodeBodySize));
	/* U32 uiMemNodeAllocedCnt */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiMemNodeAllocedCnt = %u\n",pcPrtPrefixStr,(U32)(pthis->uiMemNodeAllocedCnt));
	/* U32 uiMemNodeTotCnt */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiMemNodeTotCnt = %u\n",pcPrtPrefixStr,(U32)(pthis->uiMemNodeTotCnt));
	/* OFFSET offsetHeadRoom */
	FPRINTF(LOG_LEVEL,"_%s : pthis->offsetHeadRoom = %ld\n",pcPrtPrefixStr,(pthis->offsetHeadRoom));
	/* OFFSET offsetNodeStart */
	FPRINTF(LOG_LEVEL,"_%s : pthis->offsetNodeStart = %ld\n",pcPrtPrefixStr,(pthis->offsetNodeStart));
	/* OFFSET offsetFreeList */
	FPRINTF(LOG_LEVEL,"_%s : pthis->offsetFreeList = %ld\n",pcPrtPrefixStr,(pthis->offsetFreeList));
	/* OFFSET offsetNodeEnd */
	FPRINTF(LOG_LEVEL,"_%s : pthis->offsetNodeEnd = %ld\n",pcPrtPrefixStr,(pthis->offsetNodeEnd));
	/* U64	createCnt */
	{    S8 STG_PrintPre[1024]; U8 *_stg_s; _stg_s = (U8 *) &pthis->createCnt; sprintf(STG_PrintPre, "0x%02x%02x%02x%02x%02x%02x%02x%02x" ,_stg_s[7] ,_stg_s[6] ,_stg_s[5] ,_stg_s[4] ,_stg_s[3] ,_stg_s[2] ,_stg_s[1] ,_stg_s[0] ); 
		FPRINTF(LOG_LEVEL,"_%s : pthis->createCnt = %s\n",pcPrtPrefixStr,STG_PrintPre);
	}
	/* U64	delCnt */
	{    S8 STG_PrintPre[1024]; U8 *_stg_s; _stg_s = (U8 *) &pthis->delCnt; sprintf(STG_PrintPre, "0x%02x%02x%02x%02x%02x%02x%02x%02x" ,_stg_s[7] ,_stg_s[6] ,_stg_s[5] ,_stg_s[4] ,_stg_s[3] ,_stg_s[2] ,_stg_s[1] ,_stg_s[0] ); 
		FPRINTF(LOG_LEVEL,"_%s : pthis->delCnt = %s\n",pcPrtPrefixStr,STG_PrintPre);
	}
}
/** stMEMSNODEHDR_Prt function.
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
void stMEMSNODEHDR_Prt(S8 *pcPrtPrefixStr, stMEMSNODEHDR *pthis){
	FPRINTF(LOG_LEVEL,"_%s : pthis = %p\n",pcPrtPrefixStr, pthis);
	/* U32	uiID */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiID = %u\n",pcPrtPrefixStr,(U32)(pthis->uiID));
	/* STIME	TimeSec */
	{    S8 STG_PrintPre[1024]; S8 tmp[1024]; if(pthis->TimeSec){ strftime(STG_PrintPre, 1024, "%Y%m%d%H%M%S", localtime((time_t *)&pthis->TimeSec)); sprintf(tmp, "\t%u", pthis->TimeSec); strcat(STG_PrintPre, tmp); } else { sprintf(STG_PrintPre,"0\t0"); } 
		FPRINTF(LOG_LEVEL,"_%s : pthis->TimeSec = %s\n",pcPrtPrefixStr,STG_PrintPre);
	}
	/* U8 	ucIsFree */
	FPRINTF(LOG_LEVEL,"_%s : pthis->ucIsFree = %d\n",pcPrtPrefixStr,(pthis->ucIsFree));
	/* S8	DebugStr[MEMS_MAX_DEBUG_STR] */
	pthis->DebugStr[MEMS_MAX_DEBUG_STR - 1] = 0;
	FPRINTF(LOG_LEVEL,"_%s : pthis->DebugStr = %s\n" ,pcPrtPrefixStr,pthis->DebugStr);
	/* S8	DelStr[MEMS_MAX_DEBUG_STR] */
	pthis->DelStr[MEMS_MAX_DEBUG_STR - 1] = 0;
	FPRINTF(LOG_LEVEL,"_%s : pthis->DelStr = %s\n" ,pcPrtPrefixStr,pthis->DelStr);
}
