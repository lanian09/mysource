/******************************************************************************* 
        @file   asso_alloc.c
 *      - A_SCTP 프로세스를 초기화 하는 함수들
 *
 *      Copyright (c) 2006~ by Upresto Inc. Korea
 *      All rights reserved
 *
 *      $Id: asso_alloc.c,v 1.1.1.1 2011/08/29 05:56:42 dcham Exp $
 *
 *      @Author     $Author: dcham $
 *      @version    $Revision: 1.1.1.1 $
 *      @date       $Date: 2011/08/29 05:56:42 $
 *      @ref        asso_alloc.c
 *
 *      @section    Intro(소개)
 *      - ASSOCIATION 관련 MMDB 
 *
 *      @section    Requirement
 *
*******************************************************************************/

/**A.1*  File Inclusion *******************************************************/
#include <sctpstack.h>

/**B.1*  Definition of New Constants ******************************************/

/**B.2*  Definition of New Type  **********************************************/

/**C.1*  Declaration of Variables  ********************************************/

/**D.1*  Definition of Functions  *********************************************/

/**D.2*  Definition of Functions  *********************************************/

/*******************************************************************************

*******************************************************************************/
int sess_alloc()
{
    ASSO_TYPE 	*pNode;
	int 		dIndex;

	pNode = &pstAssoTbl->tuple[pstAssoTbl->free];
	if( pNode->right == -1 )
		return -1;

	dIndex = pstAssoTbl->free;
	pstAssoTbl->free = pNode->right;

	return(dIndex);
}


/*******************************************************************************

*******************************************************************************/
void sess_dealloc(int dIndex)
{
	ASSO_TYPE	*pNode;
	/* 시스템 Recovery용 */
	ASSO_TYPE	*dealloc = &pstAssoTbl->tuple[dIndex];

	memset( &dealloc->key, 0x00, ASSO_KEY_SIZE );

	/* 시스템 노드 free시킴 */
    pNode = &pstAssoTbl->tuple[pstAssoTbl->free];
	dealloc->right = pNode->right;
    pNode->right = dIndex;
}

/*
* $Log: asso_alloc.c,v $
* Revision 1.1.1.1  2011/08/29 05:56:42  dcham
* NEW OAM SYSTEM
*
* Revision 1.1  2011/08/05 02:38:56  uamyd
* A_SCTP modified
*
* Revision 1.2  2011/01/11 04:09:05  uamyd
* modified
*
* Revision 1.1.1.1  2010/08/23 01:13:04  uamyd
* DQMS With TOTMON, 2nd-import
*
* Revision 1.1  2009/05/27 07:42:36  dqms
* Makefile
*
* Revision 1.1  2009/05/13 11:42:49  upst_cvs
* NEW
*
* Revision 1.1  2008/01/11 12:09:02  pkg
* import two-step by uamyd
*
* Revision 1.2  2007/04/29 13:09:34  doit1972
* CVS LOG 정보 추가
*
*/
