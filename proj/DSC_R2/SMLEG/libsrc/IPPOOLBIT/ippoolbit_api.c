/*******************************************************************************
                KTF IPAF Project

   Author   : Lee Dong-Hwan
   Section  : IPPOOLBIT 
   SCCS ID  : @(#)ippoolbit_init.c 
   Date     : 11/07/08
   Revision History :
        '08.    11. 07. initial

   Description :

   Copyright (c) uPresto 2005
*******************************************************************************/


/** A. FILE INCLUSION *********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <comm_typedef.h>
#include <ippool_bitarray.h>


/** B. DEFINITION OF NEW CONSTANTS ********************************************/


/** C. DEFINITION OF NEW TYPES ************************************************/


/** D. DECLARATION OF VARIABLES ***********************************************/
int					dIppoolBit_ShmID;
pst_IPPOOLLIST		pstIPPOOLBIT = NULL;


/** E.1 DEFINITION OF FUNCTIONS ***********************************************/
int dInit_IPPOOLBIT( int dKey );
int dSetIPPOOLList( UINT uiIPAddress, UINT uiNetMask, UCHAR ucStatus, pst_IPPOOLLIST pstIPPOOL );


/** E.2 DEFINITION OF FUNCTIONS ***********************************************/

/*******************************************************************************

*******************************************************************************/
int dInit_IPPOOLBIT( int dKey )
{
	dIppoolBit_ShmID = shmget( dKey, DEF_IPPOOLBIT_SIZE, 0666|IPC_CREAT|IPC_EXCL);
	if( dIppoolBit_ShmID < 0 ) {
		if( errno == EEXIST ) {
			dIppoolBit_ShmID = shmget( dKey, DEF_IPPOOLBIT_SIZE, 0666|IPC_CREAT );
			if( dIppoolBit_ShmID < 0 )
				return -1; 

			pstIPPOOLBIT = (pst_IPPOOLLIST)shmat( dIppoolBit_ShmID, 0, 0 );
			if( pstIPPOOLBIT < 0 )
				return -2;
		}
		else
			return -3;
	}
	else {
		pstIPPOOLBIT = (pst_IPPOOLLIST)shmat( dIppoolBit_ShmID, 0, 0 );
		if( pstIPPOOLBIT < 0 )
			return -4;
	}

	return 1;
}

/*******************************************************************************

*******************************************************************************/
int dSetIPPOOLList( UINT uiIPAddress, UINT uiNetMask, UCHAR ucStatus, pst_IPPOOLLIST pstIPPOOL )
{
    UINT    i;
    UINT    uiTempIP, uiTempIP1, uiTempIP2, uiTempIP3;
	UINT	uiNet1, uiNet2;

    uiTempIP1 = 0;
    uiTempIP2 = 0xFFFFFFFF;
    uiTempIP3 = 0;

	uiTempIP 	= uiIPAddress;
	uiNet1		= uiNetMask;
	uiNet2 		= ~(uiNet1);

	uiTempIP1 = (uiTempIP & uiNet1);
	uiTempIP2 = (uiTempIP | uiNet2);

	/*
    uiTempIP = uiIPAddress;
    uiTempIP1 = ((uiTempIP>>(32-uiNetMask))<<(32-uiNetMask));

    uiTempIP = uiIPAddress;
    if( uiNetMask != 32 )
        uiTempIP2 = (((~((uiTempIP>>(32-uiNetMask))<<(32-uiNetMask))<<(uiNetMask))>>(uiNetMask)) | uiTempIP1);
    else
        uiTempIP2 = uiTempIP;
	*/

    if( ucStatus == 1 ) {
        for( i=uiTempIP1; i<=uiTempIP2; i++ )
            _IPPOOL_SET( i, pstIPPOOL );
    }
    else if( ucStatus == 0 ) {
        for( i=uiTempIP1; i<=uiTempIP2; i++ )
            _IPPOOL_CLR( i, pstIPPOOL );
    }
    else
        return -1;

    return 1;
}


/*******************************************************************************

*******************************************************************************
int dInit_IPPOOLList( unsigned char *szIPPoolFile, pst_IPPOOLLIST pstIPPOOL )
{
    FILE        *fa;

    UINT        uiPDSNIP, uiIPPool;
    UINT        uiNetMask;
    UCHAR       szBuffer[1024];
    UCHAR       szPDSNIP[32], szIPPool[32];

    memset( pstIPPOOL, 0x00, sizeof(st_IPPOOLLIST) );

    if( (fa = fopen( szIPPoolFile, "r")) == NULL )
        return -1;

    while( fgets( szBuffer, 1024, fa ) != NULL ) {
        if( szBuffer[0] == '@' ) {
            if( szBuffer[1] == 'S' )
                continue;
            if( szBuffer[1] == 'E' )
            break;
        }

        if( sscanf( &szBuffer[0], "%s %s %d", szPDSNIP, szIPPool, &uiNetMask ) == 3 ) {
            uiPDSNIP = CVT_UINT(inet_addr( szPDSNIP ));
            uiIPPool = CVT_UINT(inet_addr( szIPPool ));

            _IPPOOL_SET( uiPDSNIP, pstIPPOOL );
            SetIPPOOLList( uiIPPool, uiNetMask, 1, pstIPPOOL );
        }
    }

    fclose( fa );

    return 1;
}
*/
