/**		file  stHASHOINFO_Prt.c
 *      - typedef를 위한 functions
 *
 *     Copyright (c) 2006~ by Upresto Inc, Korea
 *     All rights reserved
 *
 *		$Id: stHASHOINFO_Prt.c,v 1.1.1.1 2011/08/19 00:53:28 uamyd Exp $
 * 
 *     @Author      $Author: uamyd $
 *     @version     $Revision: 1.1.1.1 $
 *     @date        $Date: 2011/08/19 00:53:28 $
 *     @warning     $type???로 정의된 것들만 사용가능
 *     @ref         stHASHOINFO_Prt.c
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


/** stHASHOINFO_Prt function.
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
void stHASHOINFO_Prt(S8 *pcPrtPrefixStr, stHASHOINFO *pthis){
	FPRINTF(LOG_LEVEL,"_%s : pthis = %p",pcPrtPrefixStr, pthis);
	/* U32	version */
	FPRINTF(LOG_LEVEL,"_%s : pthis->version = %u",pcPrtPrefixStr,(U32)(pthis->version));
	/* OFFSET offset_psthashnode   */
	FPRINTF(LOG_LEVEL,"_%s : pthis->offset_psthashnode   = %ld",pcPrtPrefixStr,(pthis->offset_psthashnode  ));
	/* U16 usKeyLen */
	FPRINTF(LOG_LEVEL,"_%s : pthis->usKeyLen = %d",pcPrtPrefixStr,(pthis->usKeyLen));
	/* U16 usDataLen */
	FPRINTF(LOG_LEVEL,"_%s : pthis->usDataLen = %d",pcPrtPrefixStr,(pthis->usDataLen));
	/* U32 uiHashSize */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiHashSize = %u",pcPrtPrefixStr,(U32)(pthis->uiHashSize));
	/* OFFSET	offset_memginfo */
	FPRINTF(LOG_LEVEL,"_%s : pthis->offset_memginfo = %ld",pcPrtPrefixStr,(pthis->offset_memginfo));
}
