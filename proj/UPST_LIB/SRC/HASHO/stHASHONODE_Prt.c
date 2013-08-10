/**		file  stHASHONODE_Prt.c
 *      - typedef를 위한 functions
 *
 *     Copyright (c) 2006~ by Upresto Inc, Korea
 *     All rights reserved
 *
 *		$Id: stHASHONODE_Prt.c,v 1.1.1.1 2011/08/19 00:53:28 uamyd Exp $
 * 
 *     @Author      $Author: uamyd $
 *     @version     $Revision: 1.1.1.1 $
 *     @date        $Date: 2011/08/19 00:53:28 $
 *     @warning     $type???로 정의된 것들만 사용가능
 *     @ref         stHASHONODE_Prt.c
 *     @todo        library를 만들기 위한 Makefile을 만들자
 *
 *     @section     Intro(소개)
 *      - typedef를 위한 functions
 *
 *     @section     Requirement
 *      @li 규칙에 틀린 곳을 찾아주세요.
 *
 **/


#include "hasho.h"


/** stHASHONODE_Prt function.
 *
 *  Printing Function
 *
 * @param *pcPrtPrefixStr 	: Print Prefix String
 * @param *pthis 		: Print변수 Pointer
 *
 *  @return 	void
 *  @see    	hasho.h
 *
 *  @exception  규칙에 틀린 곳을 찾아주세요.
 *  @note 		structg.pl로 만들어진 자동 코드 
 **/
void stHASHONODE_Prt(S8 *pcPrtPrefixStr, stHASHONODE *pthis){
	FPRINTF(LOG_LEVEL,"_%s : pthis = %p",pcPrtPrefixStr, pthis);
	/* OFFSET offset_next */
	FPRINTF(LOG_LEVEL,"_%s : pthis->offset_next = %ld",pcPrtPrefixStr,(pthis->offset_next));
	/* OFFSET offset_prev */
	FPRINTF(LOG_LEVEL,"_%s : pthis->offset_prev = %ld",pcPrtPrefixStr,(pthis->offset_prev));
	/* OFFSET offset_Key */
	FPRINTF(LOG_LEVEL,"_%s : pthis->offset_Key = %ld",pcPrtPrefixStr,(pthis->offset_Key));
	/* OFFSET offset_Data */
	FPRINTF(LOG_LEVEL,"_%s : pthis->offset_Data = %ld",pcPrtPrefixStr,(pthis->offset_Data));
}
