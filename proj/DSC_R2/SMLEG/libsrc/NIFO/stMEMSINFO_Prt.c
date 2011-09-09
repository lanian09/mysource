/**		file  stMEMSINFO_Prt.c
 *      - typedef�� ���� functions
 *
 *     Copyright (c) 2006~ by Upresto Inc, Korea
 *     All rights reserved
 *
 *		$Id: stMEMSINFO_Prt.c,v 1.1 2011/04/26 09:08:46 jjinri Exp $
 * 
 *     @Author      $Author: jjinri $
 *     @version     $Revision: 1.1 $
 *     @date        $Date: 2011/04/26 09:08:46 $
 *     @warning     $type???�� ���ǵ� �͵鸸 ��밡��
 *     @ref         stMEMSINFO_Prt.c
 *     @todo        library�� ����� ���� Makefile�� ������
 *
 *     @section     Intro(�Ұ�)
 *      - typedef�� ���� functions
 *
 *     @section     Requirement
 *      @li ��Ģ�� Ʋ�� ���� ã���ּ���.
 *
 **/


#include "mems.h"


/** stMEMSINFO_Prt function.
 *
 *  Printing Function
 *
 * @param *pcPrtPrefixStr 	: Print Prefix String
 * @param *pthis 		: Print���� Pointer
 *
 *  @return 	void
 *  @see    	mems.h
 *
 *  @exception  ��Ģ�� Ʋ�� ���� ã���ּ���.
 *  @note 		structg.pl�� ������� �ڵ� �ڵ� 
 **/
void stMEMSINFO_Prt(S8 *pcPrtPrefixStr, stMEMSINFO *pthis){
	FPRINTF(LOG_LEVEL,"_%s : pthis = %p",pcPrtPrefixStr, pthis);
	/* U32 	uiType */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiType = %u",pcPrtPrefixStr,(U32)(pthis->uiType));
	/* U32 			uiShmKey */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiShmKey = %u",pcPrtPrefixStr,(U32)(pthis->uiShmKey));
	/* OFFSET			uiTotMemSize */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiTotMemSize = %ld",pcPrtPrefixStr,(pthis->uiTotMemSize));
	/* U32				uiHeadRoomSize */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiHeadRoomSize = %u",pcPrtPrefixStr,(U32)(pthis->uiHeadRoomSize));
	/* U32				uiMemNodeHdrSize */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiMemNodeHdrSize = %u",pcPrtPrefixStr,(U32)(pthis->uiMemNodeHdrSize));
	/* OFFSET  		offsetHeadRoom */
	FPRINTF(LOG_LEVEL,"_%s : pthis->offsetHeadRoom = %ld",pcPrtPrefixStr,(pthis->offsetHeadRoom));
	/* U32				uiZoneCnt */
	FPRINTF(LOG_LEVEL,"_%s : pthis->uiZoneCnt = %u",pcPrtPrefixStr,(U32)(pthis->uiZoneCnt));
	/* st_MEMSZONE		stMEMSZONE[MAX_MEMSZONE_CNT] */
	{ int iIndex;
		for(iIndex = 0;iIndex < MAX_MEMSZONE_CNT;iIndex++){
			st_MEMSZONE_Prt(pcPrtPrefixStr, &(pthis->stMEMSZONE[iIndex]));
		}
	}
}
