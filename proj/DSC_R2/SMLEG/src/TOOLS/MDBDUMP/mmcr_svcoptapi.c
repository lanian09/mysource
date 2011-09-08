/**********************************************************
                 ABLEX Main-Memory DBMS

   Author   : SangWoo Lee 
   Section  :
   SCCS ID  : %W%
   Date     : %G%
   Revision History : 
        '99.  9. 21     Initial
		'01.  7. 23		Revised for TPK
		'04.  4. 20		Insert By LSH for MMDB Record Count

   Description:
        

   Copyright (c) ABLEX 1999, 2000, 2001, and 2004
***********************************************************/

/**A.1*  File Inclusion ***********************************/

#include "mmdb_svcopt.h"

/**B.1*  Definition of New Constants *********************/
/**B.2*  Definition of New Type  **************************/
/**C.1*  Declaration of Variables  ************************/
/**D.1*  Definition of Functions  *************************/
/**D.2*  Definition of Functions  *************************/

int Insert_SVCOPT( PSVCOPT_DATA disp )
{
	SVCOPT_KEY* P;
	long long *P2;
	int	ret = 0;


	P = (SVCOPT_KEY*)disp;
	P2 = (long long *)disp;

	p( semid_svcopt );
	ret = avl_insert_svcopt(P, &P2[JMM_SVCOPT_KEY_LEN], &svcopt_tbl->root);
	if(ret == 0)	// Insert Success = 0
		svcopt_tbl->uiCount++;
	v( semid_svcopt );
	
	return ret;
}

PSVCOPT_DATA Search_SVCOPT( PSVCOPT_KEY key )
{
	PSVCOPT_DATA pData;

	p( semid_svcopt );
	pData = (PSVCOPT_DATA)avl_search_svcopt(svcopt_tbl->root, key);
	v( semid_svcopt );

	return pData;
}

int Delete_SVCOPT( SVCOPT_KEY *key )
{
	int ret;

	p( semid_svcopt );	
	ret = avl_delete_svcopt(&svcopt_tbl->root, key);
	if(ret == 0)	// Delete Success = 0
		svcopt_tbl->uiCount--;
	v( semid_svcopt );

	return ret;
}

PSVCOPT_DATA Select_SVCOPT( PSVCOPT_KEY first_key, PSVCOPT_KEY last_key )
{
	PSVCOPT_DATA pData;

	p( semid_svcopt );	
    pData = (PSVCOPT_DATA)avl_select_svcopt(svcopt_tbl->root, first_key, last_key);
	v( semid_svcopt );

	return pData;
}

int Update_SVCOPT( PSVCOPT_DATA disp, PSVCOPT_DATA input )
{
	int ret;
    long long *P2;
    P2 = (long long *)input;

	p( semid_svcopt );
    ret = avl_update_svcopt((SVCOPT_TYPE *)disp, &P2[JMM_SVCOPT_KEY_LEN]);
	v( semid_svcopt );

	return ret;
}
