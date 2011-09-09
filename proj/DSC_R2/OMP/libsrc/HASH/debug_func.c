#include "memg.h"
#include "hasho.h"

/** stHASHONODE_Prt function.
 *
 *  Printing Function
 *
 * @param *pcPrtPrefixStr   : Print Prefix String
 * @param *pthis        : Print변수 Pointer
 *
 *  @return     void
 *  @see        hasho.h
 *
 *  @exception  규칙에 틀린 곳을 찾아주세요.
 *  @note       structg.pl로 만들어진 자동 코드 
 **/
void stHASHONODE_Prt(S8 *pcPrtPrefixStr, stHASHONODE *pthis) {
	FPRINTF(LOG_LEVEL,"_%s : pthis = %p\n", pcPrtPrefixStr, pthis);
	/* OFFSET offset_next */
	FPRINTF(LOG_LEVEL,"_%s : pthis->offset_next = %ld\n",pcPrtPrefixStr, (pthis->offset_next));
	/* OFFSET offset_prev */
	FPRINTF(LOG_LEVEL,"_%s : pthis->offset_prev = %ld\n",pcPrtPrefixStr, (pthis->offset_prev));
	/* OFFSET offset_Key */
	FPRINTF(LOG_LEVEL,"_%s : pthis->offset_Key = %ld\n",pcPrtPrefixStr, (pthis->offset_Key));
	/* OFFSET offset_Data */
	FPRINTF(LOG_LEVEL,"_%s : pthis->offset_Data = %ld\n",pcPrtPrefixStr, (pthis->offset_Data));
}

/** stHASHOINFO_Prt function.
 *
 *  Printing Function
 *
 * @param *pcPrtPrefixStr   : Print Prefix String
 * @param *pthis        : Print변수 Pointer
 *
 *  @return     void
 *  @see        hasho.h
 *
 *  @exception  규칙에 틀린 곳을 찾아주세요.
 *  @note       structg.pl로 만들어진 자동 코드 
 **/
void stHASHOINFO_Prt(S8 *pcPrtPrefixStr, stHASHOINFO *pthis) {
	FPRINTF(LOG_LEVEL,"_%s : pthis = %p\n",pcPrtPrefixStr, pthis);
	/* U32  version */
	FPRINTF(LOG_LEVEL,"_%s : pthis->version = %u\n",pcPrtPrefixStr,(U32)(pthis->version));
	/* OFFSET offset_psthashnode   */
	FPRINTF(LOG_LEVEL,"_%s : pthis->offset_psthashnode = %ld\n",pcPrtPrefixStr,(pthis->offset_psthashnode  ));
	/* U16 usKeyLen */
	FPRINTF(LOG_LEVEL,"_%s : pthis->usKeyLen = %d\n", pcPrtPrefixStr, (pthis->usKeyLen));
	/* U16 usDataLen */
	FPRINTF(LOG_LEVEL,"_%s : pthis->usDataLen = %d\n", pcPrtPrefixStr, (pthis->usDataLen));
	/* U32 uiHashSize */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiHashSize = %u\n", pcPrtPrefixStr, (U32)(pthis->uiHashSize));
	/* OFFSET   offset_memginfo */
	FPRINTF(LOG_LEVEL,"_%s : pthis->offset_memginfo = %ld\n",pcPrtPrefixStr, (pthis->offset_memginfo));
}
/** stMEMGINFO_Prt function.
 *
 *  Printing Function
 *
 * @param *pcPrtPrefixStr   : Print Prefix String
 * @param *pthis        : Print변수 Pointer
 *
 *  @return     void
 *  @see        memg.h
 *
 *  @exception  규칙에 틀린 곳을 찾아주세요.
 *  @note       structg.pl로 만들어진 자동 코드 
 **/
void stMEMGINFO_Prt(S8 *pcPrtPrefixStr, stMEMGINFO *pthis) {
	FPRINTF(LOG_LEVEL,"_%s : pthis = %p\n",pcPrtPrefixStr, pthis);
	/* U32 uiType */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiType = %u\n",pcPrtPrefixStr,(U32)(pthis->uiType));
	/* U32 uiShmKey */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiShmKey = %u\n",pcPrtPrefixStr,(U32)(pthis->uiShmKey));
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
}

/** stMEMGNODEHDR_Prt function.
 *
 *  Printing Function
 *
 * @param *pcPrtPrefixStr   : Print Prefix String
 * @param *pthis        : Print변수 Pointer
 *
 *  @return     void
 *  @see        memg.h
 *
 *  @exception  규칙에 틀린 곳을 찾아주세요.
 *  @note       structg.pl로 만들어진 자동 코드 
 **/
void stMEMGNODEHDR_Prt(S8 *pcPrtPrefixStr, stMEMGNODEHDR *pthis) {
	FPRINTF(LOG_LEVEL,"_%s : pthis = %p\n",pcPrtPrefixStr, pthis);
	/* U32  uiID */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiID = %u\n",pcPrtPrefixStr,(U32)(pthis->uiID));
	/* STIME    TimeSec */
	{    S8 STG_PrintPre[1024]; S8 tmp[1024]; if(pthis->TimeSec){ strftime(STG_PrintPre, 1024, "%Y%m%d%H%M%S", localtime((time_t *)&pthis->TimeSec)); sprintf(tmp, "\t%u", pthis->TimeSec); strcat(STG_PrintPre, tmp); } else { sprintf(STG_PrintPre,"0\t0"); } 
		FPRINTF(LOG_LEVEL,"_%s : pthis->TimeSec = %s\n",pcPrtPrefixStr,STG_PrintPre);
	}
	/* U8   ucIsFree */
	FPRINTF(LOG_LEVEL,"_%s : pthis->ucIsFree = %d\n",pcPrtPrefixStr,(pthis->ucIsFree));
	/* S8   DebugStr[MEMG_MAX_DEBUG_STR] */
	pthis->DebugStr[MEMG_MAX_DEBUG_STR - 1] = 0;
	FPRINTF(LOG_LEVEL,"_%s : pthis->DebugStr = %s\n" ,pcPrtPrefixStr,pthis->DebugStr);
}

