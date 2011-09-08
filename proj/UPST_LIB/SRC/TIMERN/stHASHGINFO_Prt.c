/**		file  stHASHGINFO_Prt.c
 *      - typedef�� ���� functions
 *
 *     Copyright (c) 2006~ by Upresto Inc, Korea
 *     All rights reserved
 *
 *		$Id: stHASHGINFO_Prt.c,v 1.1.1.1 2011/08/19 00:53:28 uamyd Exp $
 * 
 *     @Author      $Author: uamyd $
 *     @version     $Revision: 1.1.1.1 $
 *     @date        $Date: 2011/08/19 00:53:28 $
 *     @warning     $type???�� ���ǵ� �͵鸸 ��밡��
 *     @ref         stHASHGINFO_Prt.c
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


/** stHASHGINFO_Prt function.
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
