/**		file  stHASHGNODE_Prt.c
 *      - typedef�� ���� functions
 *
 *     Copyright (c) 2006~ by Upresto Inc, Korea
 *     All rights reserved
 *
 *		$Id: stHASHGNODE_Prt.c,v 1.1.1.1 2011/08/19 00:53:28 uamyd Exp $
 * 
 *     @Author      $Author: uamyd $
 *     @version     $Revision: 1.1.1.1 $
 *     @date        $Date: 2011/08/19 00:53:28 $
 *     @warning     $type???�� ���ǵ� �͵鸸 ��밡��
 *     @ref         stHASHGNODE_Prt.c
 *     @todo        library�� ����� ���� Makefile�� ������
 *
 *     @section     Intro(�Ұ�)
 *      - typedef�� ���� functions
 *
 *     @section     Requirement
 *      @li ��Ģ�� Ʋ�� ���� ã���ּ���.
 *
 **/


#include "hashg.h"


/** stHASHGNODE_Prt function.
 *
 *  Printing Function
 *
 * @param *pcPrtPrefixStr 	: Print Prefix String
 * @param *pthis 		: Print���� Pointer
 *
 *  @return 	void
 *  @see    	hashg.h
 *
 *  @exception  ��Ģ�� Ʋ�� ���� ã���ּ���.
 *  @note 		structg.pl�� ������� �ڵ� �ڵ� 
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
