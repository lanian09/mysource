/**********************************************************
                 ABLEX Main-Memory DBMS

   Author   : SangWoo Lee 
   Section  :
   SCCS ID  : %W%
   Date     : %G%
   Revision History : 
        '99.  9. 21     Initial
		'01.  7. 23		Revised for TPK
		'04.  6.		miheeh : Count_XXXX() Ãß°¡

   Description:
		MMDB V2.1        

   Copyright (c) ABLEX 1999, 2000, and 2001
***********************************************************/

/**A.1*  File Inclusion ***********************************/
#include "mmdb_psess.h"

/**B.1*  Definition of New Constants *********************/
/**B.2*  Definition of New Type  **************************/
/**C.1*  Declaration of Variables  ************************/
/**D.1*  Definition of Functions  *************************/
/**D.2*  Definition of Functions  *************************/

int Insert_PSESS( PPSESS_DATA disp )
{
	PSESS_KEY *P1;
	long long *P2;
	int	ret = 0;

	P1 = (PSESS_KEY*)disp;
	P2 = (long long *)disp;

	//P( semid_psess ); 
	ret = avl_insert_psess(P1, &P2[JMM_PSESS_KEY_LEN], &psess_tbl->root);
	if( ret == 0 )
		psess_tbl->used ++;
	//V( semid_psess );
	
	return ret;
}

PPSESS_DATA Search_PSESS( PPSESS_KEY key )
{
	PPSESS_DATA pData;

	pData = (PPSESS_DATA)avl_search_psess(psess_tbl->root, key);

	return pData;
}

int Delete_PSESS( PSESS_KEY *key )
{
	int ret;

	//P( semid_psess );
	ret = avl_delete_psess( &psess_tbl->root, key );
	if( ret == 0 )
		psess_tbl->used --;
	//V( semid_psess );  

	return ret;
}

PPSESS_DATA Select_PSESS( PPSESS_KEY first_key, PPSESS_KEY last_key )
{
	PPSESS_DATA pData;

    pData = (PPSESS_DATA)avl_select_psess( psess_tbl->root, first_key, last_key );

	return pData;
}

int Update_PSESS( PPSESS_DATA input )
{
	int ret = -1;
	PPSESS_DATA	disp;
	PSESS_KEY *P1;
    long long *P2;

	P1 = (PSESS_KEY *)input;
    P2 = (long long *)input;

	//P( semid_psess );
	disp = (PPSESS_DATA)avl_search_psess(psess_tbl->root, P1);
	if( disp != 0 )
    	ret = avl_update_psess( (PSESS_TYPE *)disp, &P2[JMM_PSESS_KEY_LEN]);
	//V( semid_psess );

	return ret;
}

int Count_PSESS()
{
	return psess_tbl->used;
}

int GetCount_PSESS(void)
{
	return Count_PSESS();
}

