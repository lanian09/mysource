/**		file  stHASHGNODE_Prt.c
 *      - typedef를 위한 functions
 *
 *     Copyright (c) 2006~ by Upresto Inc, Korea
 *     All rights reserved
 *
 *		$Id: stHASHGNODE_Prt.c,v 1.1.1.1 2011/08/19 00:53:28 uamyd Exp $
 * 
 *     @Author      $Author: uamyd $
 *     @version     $Revision: 1.1.1.1 $
 *     @date        $Date: 2011/08/19 00:53:28 $
 *     @warning     $type???로 정의된 것들만 사용가능
 *     @ref         stHASHGNODE_Prt.c
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


/** stHASHGNODE_Prt function.
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
void stHASHGNODE_Prt(S8 *pcPrtPrefixStr, stHASHGNODE *pthis){
	FPRINTF(LOG_LEVEL,"_%s : pthis = %p",pcPrtPrefixStr, pthis);
	/* U8 *pstHASHGINFO */
	FPRINTF(LOG_LEVEL,"_%s : pthis->pstHASHGINFO = %p",pcPrtPrefixStr,(pthis->pstHASHGINFO));
	/* struct _st_hashgnode *next   */
	FPRINTF(LOG_LEVEL,"_%s : pthis->next = %p",pcPrtPrefixStr,(pthis->next));
	/* struct _st_hashgnode **prev */
	FPRINTF(LOG_LEVEL,"_%s : pthis->prev = %p",pcPrtPrefixStr,(pthis->prev));
	/* U8 *pstKey */
	FPRINTF(LOG_LEVEL,"_%s : pthis->pstKey = %p",pcPrtPrefixStr,(pthis->pstKey));
	/* U8 *pstData */
	FPRINTF(LOG_LEVEL,"_%s : pthis->pstData = %p",pcPrtPrefixStr,(pthis->pstData));
}
