/******************************************************************************* 
        @file   asso_api.c
 *      - A_SCTP 프로세스를 초기화 하는 함수들
 *
 *      Copyright (c) 2006~ by Upresto Inc. Korea
 *      All rights reserved
 *
 *      $Id: asso_api.c,v 1.1.1.1 2011/08/29 05:56:42 dcham Exp $
 *
 *      @Author     $Author: dcham $
 *      @version    $Revision: 1.1.1.1 $
 *      @date       $Date: 2011/08/29 05:56:42 $
 *      @ref        asso_api.c
 *
 *      @section    Intro(소개)
 *      - ASSOCIATION 관련 MMDB
 *
 *      @section    Requirement
 *
*******************************************************************************/

/**A.1*  File Inclusion *******************************************************/
#include <stdio.h>
#include <sctpstack.h>

/**B.1*  Definition of New Constants ******************************************/

/**B.2*  Definition of New Type  **********************************************/

/**C.1*  Declaration of Variables  ********************************************/

/**D.1*  Definition of Functions  *********************************************/

/**D.2*  Definition of Functions  *********************************************/

/*******************************************************************************

*******************************************************************************/
void Init_SessDB()
{
	int		i;
	
	for(i=0; i < MAX_ASSO_RECORD-1; i++)
		pstAssoTbl->tuple[i].right = i+1;		

	pstAssoTbl->tuple[i].right = -1;

    pstAssoTbl->free = 0;
    pstAssoTbl->root = -1;
    pstAssoTbl->uiAssoCount = 0;
}


/*******************************************************************************

*******************************************************************************/
int Insert_SESS(PASSO_DATA pstSrc)
{
	int			ret = 0;
	ASSO_KEY	*P;
	long long 	*P2;

	P = (ASSO_KEY *)pstSrc;
	P2 = (long long *)pstSrc;
	ret = avl_insert_sess(P, &P2[MAX_ASSO_KEY_LEN], &pstAssoTbl->root);

	return ret;
}


/*******************************************************************************

*******************************************************************************/
PASSO_DATA Search_SESS(PASSO_KEY pstKey)
{
	PASSO_DATA pstData;

	pstData = (PASSO_DATA)avl_search_sess(pstAssoTbl->root, pstKey);

	return pstData;
}


/*******************************************************************************

*******************************************************************************/
int Delete_SESS(PASSO_KEY pstKey)
{
	int ret;
	
	ret = avl_delete_sess(&pstAssoTbl->root, pstKey);

	return ret;
}


/*******************************************************************************

*******************************************************************************/
PASSO_DATA Select_SESS(PASSO_KEY first_key, PASSO_KEY last_key)
{
	PASSO_DATA pstData;

    pstData = (PASSO_DATA)avl_select_sess(pstAssoTbl->root, first_key, last_key);

	return pstData;
}


/*******************************************************************************

*******************************************************************************/
int Update_SESS(PASSO_DATA disp, PASSO_DATA input)
{
	int ret;
    long long *P2;
    P2 = (long long *)input;

    ret = avl_update_sess((ASSO_TYPE *)disp, &P2[MAX_ASSO_KEY_LEN]);

	return ret;
}


/*******************************************************************************

*******************************************************************************/
PASSO_DATA pstSelectMMDB(PASSO_KEY pstFKey, PASSO_KEY pstLKey)
{
    PASSO_DATA      pstData;

    pstData = (PASSO_DATA)avl_select_sess(pstAssoTbl->root, pstFKey, pstLKey);

    return pstData;
}

/*
* $Log: asso_api.c,v $
* Revision 1.1.1.1  2011/08/29 05:56:42  dcham
* NEW OAM SYSTEM
*
* Revision 1.1  2011/08/05 02:38:57  uamyd
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
