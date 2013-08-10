/**		file  stHASHOINFO_Prt.c
 *      - typedef�� ���� functions
 *
 *     Copyright (c) 2006~ by Upresto Inc, Korea
 *     All rights reserved
 *
 *		$Id: stHASHOINFO_Prt.c,v 1.1.1.1 2011/08/19 00:53:28 uamyd Exp $
 * 
 *     @Author      $Author: uamyd $
 *     @version     $Revision: 1.1.1.1 $
 *     @date        $Date: 2011/08/19 00:53:28 $
 *     @warning     $type???�� ���ǵ� �͵鸸 ��밡��
 *     @ref         stHASHOINFO_Prt.c
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


/** stHASHOINFO_Prt function.
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
