/**		file  stHASHGINFO_Prt.c
 *      - typedef를 위한 functions
 *
 *     Copyright (c) 2006~ by Upresto Inc, Korea
 *     All rights reserved
 *
 *		$Id: stHASHGINFO_Prt.c,v 1.1.1.1 2011/08/19 00:53:28 uamyd Exp $
 * 
 *     @Author      $Author: uamyd $
 *     @version     $Revision: 1.1.1.1 $
 *     @date        $Date: 2011/08/19 00:53:28 $
 *     @warning     $type???로 정의된 것들만 사용가능
 *     @ref         stHASHGINFO_Prt.c
 *     @todo        library를 만들기 위한 Makefile을 만들자
 *
 *     @section     Intro(소개)
 *      - typedef를 위한 functions
 *
 *     @section     Requirement
 *      @li 규칙에 틀린 곳을 찾아주세요.
 *
 **/


#include "hashg.h"


/** stHASHGINFO_Prt function.
 *
 *  Printing Function
 *
 * @param *pcPrtPrefixStr 	: Print Prefix String
 * @param *pthis 		: Print변수 Pointer
 *
 *  @return 	void
 *  @see    	hashg.h
 *
 *  @exception  규칙에 틀린 곳을 찾아주세요.
 *  @note 		structg.pl로 만들어진 자동 코드 
 **/
void stHASHGINFO_Prt(S8 *pcPrtPrefixStr, stHASHGINFO *pthis){
	FPRINTF(LOG_LEVEL,"_%s : pthis = %p",pcPrtPrefixStr, pthis);
	/* stHASHGNODE *psthashnode   */
	FPRINTF(LOG_LEVEL,"_%s : pthis->psthashnode   = %p",pcPrtPrefixStr,(pthis->psthashnode  ));
	/* U16 usKeyLen */
	FPRINTF(LOG_LEVEL,"_%s : pthis->usKeyLen = %d",pcPrtPrefixStr,(pthis->usKeyLen));
	/* U16 usDataLen */
	FPRINTF(LOG_LEVEL,"_%s : pthis->usDataLen = %d",pcPrtPrefixStr,(pthis->usDataLen));
	/* U32 uiHashSize */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiHashSize = %u",pcPrtPrefixStr,(U32)(pthis->uiHashSize));
	/* U32	MaxNodeCnt */
	FPRINTF(LOG_LEVEL,"_%s : pthis->MaxNodeCnt = %u",pcPrtPrefixStr,(U32)(pthis->MaxNodeCnt));
	/* U32	uiLinkedCnt */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiLinkedCnt = %u",pcPrtPrefixStr,(U32)(pthis->uiLinkedCnt));
}
