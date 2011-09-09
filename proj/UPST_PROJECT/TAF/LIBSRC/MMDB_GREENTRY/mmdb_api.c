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
#include "mmdb_greentry.h"

/**B.1*  Definition of New Constants *********************/
/**B.2*  Definition of New Type  **************************/
/**C.1*  Declaration of Variables  ************************/
/**D.1*  Definition of Functions  *************************/
/**D.2*  Definition of Functions  *************************/

int Insert_GREENTRY( PGREENTRY_DATA disp )
{
	GREENTRY_KEY *P1;
	long long *P2;
	int	ret = 0;

	P1 = (GREENTRY_KEY*)disp;
	P2 = (long long *)disp;

	//P( semid_greentry ); 
	ret = avl_insert_greentry(P1, &P2[JMM_GREENTRY_KEY_LEN], &greentry_tbl->root);
	if( ret == 0 )
		greentry_tbl->used ++;
	//V( semid_greentry );
	
	return ret;
}

PGREENTRY_DATA Search_GREENTRY( PGREENTRY_KEY key )
{
	PGREENTRY_DATA pData;

	pData = (PGREENTRY_DATA)avl_search_greentry(greentry_tbl->root, key);

	return pData;
}

int Delete_GREENTRY( GREENTRY_KEY *key )
{
	int ret;

	//P( semid_greentry );
	ret = avl_delete_greentry( &greentry_tbl->root, key );
	if( ret == 0 )
		greentry_tbl->used --;
	//V( semid_greentry );  

	return ret;
}

PGREENTRY_DATA Select_GREENTRY( PGREENTRY_KEY first_key, PGREENTRY_KEY last_key )
{
	PGREENTRY_DATA pData;

    pData = (PGREENTRY_DATA)avl_select_greentry( greentry_tbl->root, first_key, last_key );

	return pData;
}

int Update_GREENTRY( PGREENTRY_DATA input )
{
	int ret = -1;
	PGREENTRY_DATA	disp;
	GREENTRY_KEY *P1;
    long long *P2;

	P1 = (GREENTRY_KEY *)input;
    P2 = (long long *)input;

	//P( semid_greentry );
	disp = (PGREENTRY_DATA)avl_search_greentry(greentry_tbl->root, P1);
	if( disp != 0 )
    	ret = avl_update_greentry( (GREENTRY_TYPE *)disp, &P2[JMM_GREENTRY_KEY_LEN]);
	//V( semid_greentry );

	return ret;
}

int Count_GREENTRY()
{
	return greentry_tbl->used;
}

int GetCount_GREENTRY(void)
{
	return Count_GREENTRY();
}

