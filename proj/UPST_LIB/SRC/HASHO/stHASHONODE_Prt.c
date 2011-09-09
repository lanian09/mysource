/**		file  stHASHONODE_Prt.c
 *      - typedef�� ���� functions
 *
 *     Copyright (c) 2006~ by Upresto Inc, Korea
 *     All rights reserved
 *
 *		$Id: stHASHONODE_Prt.c,v 1.1.1.1 2011/08/19 00:53:28 uamyd Exp $
 * 
 *     @Author      $Author: uamyd $
 *     @version     $Revision: 1.1.1.1 $
 *     @date        $Date: 2011/08/19 00:53:28 $
 *     @warning     $type???�� ���ǵ� �͵鸸 ��밡��
 *     @ref         stHASHONODE_Prt.c
 *     @todo        library�� ����� ���� Makefile�� ������
 *
 *     @section     Intro(�Ұ�)
 *      - typedef�� ���� functions
 *
 *     @section     Requirement
 *      @li ��Ģ�� Ʋ�� ���� ã���ּ���.
 *
 **/


#include "hasho.h"


/** stHASHONODE_Prt function.
 *
 *  Printing Function
 *
 * @param *pcPrtPrefixStr 	: Print Prefix String
 * @param *pthis 		: Print���� Pointer
 *
 *  @return 	void
 *  @see    	hasho.h
 *
 *  @exception  ��Ģ�� Ʋ�� ���� ã���ּ���.
 *  @note 		structg.pl�� ������� �ڵ� �ڵ� 
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
