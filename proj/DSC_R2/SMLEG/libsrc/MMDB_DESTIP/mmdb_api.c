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

#include "mmdb_destip.h"
#include <define.h>
#include <utillib.h>

/**B.1*  Definition of New Constants *********************/
/**B.2*  Definition of New Type  **************************/
/**C.1*  Declaration of Variables  ************************/
/**D.1*  Definition of Functions  *************************/
/**D.2*  Definition of Functions  *************************/

int Insert_DESTIP( PDESTIP_DATA disp )
{
	DESTIP_KEY* P;
	long long *P2;
	int	ret = 0;


	P = (DESTIP_KEY*)disp;
	P2 = (long long *)disp;

	p( semid_destip );
	ret = avl_insert_destip(P, &P2[JMM_DESTIP_KEY_LEN], &destip_tbl->root);
	if(ret == 0)	// Insert Success = 0
		destip_tbl->uiCount++;
	v( semid_destip );
	
	return ret;
}

PDESTIP_DATA Search_DESTIP( PDESTIP_KEY key )
{
	PDESTIP_DATA pData;

	p( semid_destip );
	pData = (PDESTIP_DATA)avl_search_destip(destip_tbl->root, key);
	v( semid_destip );

	return pData;
}

int Delete_DESTIP( DESTIP_KEY *key )
{
	int ret;

	p( semid_destip );	
	ret = avl_delete_destip(&destip_tbl->root, key);
	if(ret == 0)	// Delete Success = 0
		destip_tbl->uiCount--;
	v( semid_destip );

	return ret;
}

PDESTIP_DATA Select_DESTIP( PDESTIP_KEY first_key, PDESTIP_KEY last_key )
{
	PDESTIP_DATA pData;

	p( semid_destip );	
    pData = (PDESTIP_DATA)avl_select_destip(destip_tbl->root, first_key, last_key);
	v( semid_destip );

	return pData;
}

int Update_DESTIP( PDESTIP_DATA disp, PDESTIP_DATA input )
{
	int ret;
    long long *P2;
    P2 = (long long *)input;

	p( semid_destip );
    ret = avl_update_destip((DESTIP_TYPE *)disp, &P2[JMM_DESTIP_KEY_LEN]);
	v( semid_destip );

	return ret;
}

/*
 * for filtering IP block
 * return <> 0 : found
 * 		  == 0 : not found
 */
PDESTIP_DATA Filter_DESTIP( PDESTIP_KEY key )
{
    DESTIP_KEY 		first_key, last_key;
    PDESTIP_DATA	pData;
    unsigned int	uiIP, uiNetmask;

    /* by jwkim96 */
    //unsigned char szTempIP[4], szTempIP2[4], szTempIP3[4],szTempIP4[4];

    memcpy( &first_key, key, sizeof(DESTIP_KEY) );

#ifdef INTEL
    first_key.uiIP = htonl(key->uiIP - 1);
#else
    first_key.uiIP = key->uiIP - 1;
#endif

    memset( &last_key, 0xff, sizeof(DESTIP_KEY) );

    while(1)
    {
#if 0
        dAppLog(LOG_INFO, "PARAMETER IP STRING");
        log_hexa((unsigned char *)&key->uiIP, sizeof(key->uiIP));
#endif /* by jwkim96 */
        pData = Select_DESTIP( &first_key, &last_key );

        if( pData == 0 || pData->key.ucFlag != key->ucFlag )	/* does not exist */
        {
            break;
        }
        memcpy( &first_key, &pData->key, sizeof(DESTIP_KEY) );

#ifdef INTEL
        uiIP = htonl(pData->key.uiIP);
#else
        uiIP = pData->key.uiIP;
#endif
        uiNetmask = pData->uiNetmask;

#if 0
        memcpy(szTempIP, &key->uiIP, 4);
        memcpy(szTempIP2, &pData->key.uiIP, 4);
        memcpy(szTempIP3, &uiIP, 4);
        memcpy(szTempIP4, &uiNetmask, 4);
        dAppLog(LOG_INFO, "%s: IP[%8x %3u.%3u.%3u.%3u] IP2[%8x %3u.%3u.%3u.%3u] IP3[%8x %3u.%3u.%3u.%3u]",
                __FUNCTION__, key->uiIP, szTempIP[0], szTempIP[1], szTempIP[2], szTempIP[3],
                pData->key.uiIP, szTempIP2[0], szTempIP2[1], szTempIP2[2], szTempIP2[3],
                uiIP, szTempIP3[0], szTempIP3[1], szTempIP3[2], szTempIP3[3]);
                //uiNetmask, szTempIP4[0], szTempIP4[1], szTempIP4[2], szTempIP4[3]);
        log_hexa((unsigned char *)&uiIP, sizeof(uiIP));
        log_hexa((unsigned char *)&uiNetmask, sizeof(uiNetmask));
#endif /* by jwkim96 */

        if( (key->uiIP & uiNetmask) == (uiIP & uiNetmask) )	/* found */
        {
#if 0
            dAppLog(LOG_INFO, "%s: RETURN VALUE [%p]", __FUNCTION__, pData);
#endif /* by jwkim96 */
            return pData;
        }
        else if( (key->uiIP & uiNetmask) < (uiIP & uiNetmask) )
        {
            break;
        }
    }

#if 0
    dAppLog(LOG_INFO, "%s: RETURN VALUE [0]", __FUNCTION__);
#endif /* by jwkim96 */
    return 0;
}

