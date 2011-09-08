/**********************************************************
                 ABLEX Main-Memory DBMS

   Author   : SangWoo Lee 
   Section  :
   SCCS ID  : %W%
   Date     : %G%
   Revision History : 
        '99.  9. 21     Initial
		'01.  7. 23		Revised for TPK
		'04.  4. 20		Inserted By LSH for MMDB Record Count

   Description:
        

   Copyright (c) ABLEX 1999, 2000, 2001, and 2004
***********************************************************/

/**A.1*  File Inclusion ***********************************/

#include "mmdb_destport.h"

/**B.1*  Definition of New Constants *********************/
/**B.2*  Definition of New Type  **************************/
/**C.1*  Declaration of Variables  ************************/
/**D.1*  Definition of Functions  *************************/
/**D.2*  Definition of Functions  *************************/

int Insert_DESTPORT(PDESTPORT_DATA disp)
{
	int ret;
	DESTPORT_KEY* P;
	long long *P2;

	P = (DESTPORT_KEY *)disp;
	P2 = (long long *)disp;

	p( semid_destport );
	ret = avl_insert_destport(P, &P2[JMM_DESTPORT_KEY_LEN], &destport_tbl->root);
	if(ret == 0)
		destport_tbl->uiCount++;
	v( semid_destport );
	return ret;
}

PDESTPORT_DATA Search_DESTPORT(PDESTPORT_KEY key)
{
	PDESTPORT_DATA pData;

	p( semid_destport );
	pData = (PDESTPORT_DATA)avl_search_destport(destport_tbl->root, key);
	v( semid_destport );

	return pData;
}

/*
PDESTPORT_DATA Search_DESTPORT_seq(short seq)
{
	PDESTPORT_DATA pData;

	p( semid_destport );
	pData = (PDESTPORT_DATA)avl_search_destport_seq(destport_tbl->root, seq);
	v( semid_destport );

	return pData;
}
*/

int Delete_DESTPORT(DESTPORT_KEY *key)
{
	int ret;

	p( semid_destport );
	ret = avl_delete_destport(&destport_tbl->root, key);
	if(ret == 0)
		destport_tbl->uiCount--;
	v( semid_destport );

	return ret;
}

PDESTPORT_DATA Select_DESTPORT(PDESTPORT_KEY first_key, PDESTPORT_KEY last_key)
{
	PDESTPORT_DATA pData;

	p( semid_destport );
    pData = (PDESTPORT_DATA)avl_select_destport(destport_tbl->root, first_key, last_key);
	v( semid_destport );

	return pData;
}

int Update_DESTPORT(PDESTPORT_DATA disp, PDESTPORT_DATA input)
{
	int ret;
    long long *P2;
    P2 = (long long *)input;

	p( semid_destport );
    ret = avl_update_destport((DESTPORT_TYPE *)disp, &P2[JMM_DESTPORT_KEY_LEN]);
	v( semid_destport );
	
	return ret;
}

/*
 * for filtering IP block
 * return <> 0 : found
 * 		  == 0 : not found
 */
PDESTPORT_DATA Filter_DESTPORT( PDESTPORT_KEY key )
{
	DESTPORT_KEY 	first_key, last_key;
	PDESTPORT_DATA	pData;
	unsigned int	uiIP, uiNetmask;

	memset( &first_key, 0x00, sizeof(DESTPORT_KEY) );
	
	first_key.ucProtocol = key->ucProtocol;
	first_key.usDestPort = key->usDestPort;

#ifdef INTEL
	first_key.uiDestIP = htonl(key->uiDestIP - 1);
#else
	first_key.uiDestIP = key->uiIP - 1;
#endif
	
	memset( &last_key, 0xff, sizeof(DESTPORT_KEY) );

	while(1)
	{
		pData = Select_DESTPORT( &first_key, &last_key );

		if( pData == 0 
			|| pData->key.ucProtocol != key->ucProtocol 
			|| pData->key.usDestPort != key->usDestPort)
		{
			/* does not exist */
			break;
		}
		memcpy( &first_key, &pData->key, sizeof(DESTPORT_KEY) );

#ifdef INTEL
		uiIP = htonl(pData->key.uiDestIP);
#else
		uiIP = pData->key.uiIP;
#endif
		uiNetmask = pData->uiNetmask;
	
		if( (key->uiDestIP & uiNetmask) == (uiIP & uiNetmask) )	/* found */
		{
			return pData;
		}
		else if( (key->uiDestIP & uiNetmask) < (uiIP & uiNetmask) )
		{
			break;
		}
	}

	return 0;
}
