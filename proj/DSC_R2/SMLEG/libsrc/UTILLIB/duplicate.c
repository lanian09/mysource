#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
//#include <signal.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <unistd.h>

#include <comm_typedef.h>
#include <utillib.h>
#include <define.h>
#include <ipaf_define.h>
#include <ipaf_shm.h>

int dONLY_TCP_Duplicate_Message( UINT *uiLower, UINT *uiHigher, UINT uiCurID )
{
	dAppLog(LOG_INFO, "CURID[%d] HIGH[%d] LOW[%d]", uiCurID, *uiHigher, *uiLower);
	if( *uiLower == 0 && *uiHigher == 0 ) {
		*uiHigher = uiCurID;
		return NON_DUP_ID;
	}

	if ( *uiLower < uiCurID ) {
		return NON_DUP_ID;
	} else if( *uiLower == uiCurID ) {
		return DEF_DUP_ID;
	} else if( *uiHigher == uiCurID ) {
		return DEF_DUP_ID;
	} else if( uiCurID > *uiLower && uiCurID < *uiHigher ) {
		*uiLower = uiCurID;
	} else {
		if( uiCurID > *uiHigher ) {
			*uiLower = *uiHigher;
			*uiHigher = uiCurID;
		}
	}

	return NON_DUP_ID;
}

int dTCPUDP_Duplicate_Message( UINT uiCompID[], UINT uiCompOffset[], UINT uiCurID, UINT uiCurOffset )
{
	if( uiCompID[0] == 0 && uiCompID[1] == 0 ) {
		uiCompID[1] = uiCurID;
		uiCompOffset[1] = uiCurOffset;
		return NON_DUP_ID;
	}

	dAppLog(LOG_INFO, "CURRENT[%d][%d] HIGH[%d][%d] LOW[%d][%d]", uiCurID, uiCurOffset, uiCompID[1], uiCompOffset[1], 
					uiCompID[0], uiCompOffset[0] );

	/* TCP, UDP 중복 판단을 위해 IDENTIFICATION과 FRAGMENT OFFSET 값을 비교한다. */
	if( uiCompID[0] > uiCurID ) {
		return DEF_DUP_ID;
	} else if( uiCompID[0] == uiCurID ) { 
		if( uiCompOffset[0] >= uiCurOffset ) {
			return DEF_DUP_ID;
		} else {
			if( uiCompID[1] == uiCurID && uiCompOffset[1] == uiCurOffset ) {
				return DEF_DUP_ID;
			} else if( uiCompID[1] == uiCurID && uiCompOffset[1] > uiCurOffset ) {
				uiCompID[0] = uiCurID;
        		uiCompOffset[0] = uiCurOffset;	
			}
		}
	} else if( uiCompID[1] == uiCurID ) {
		if( uiCompOffset[1] == uiCurOffset ) {
			return DEF_DUP_ID;
		} else if( uiCompOffset[1] > uiCurOffset ) {
			uiCompID[0] = uiCurID;
			uiCompOffset[0] = uiCurOffset;
		} else {
			uiCompID[0] = uiCompID[1];
            uiCompOffset[0] = uiCompOffset[1];

            uiCompID[1] = uiCurID;
            uiCompOffset[1] = uiCurOffset;	
		}
	} else if( uiCurID > uiCompID[0] && uiCurID < uiCompID[1] ) {
		uiCompID[0] = uiCurID;
		uiCompOffset[0] = uiCurOffset;
	} else {
		if( uiCurID > uiCompID[1] ) {
			uiCompID[0] = uiCompID[1];
			uiCompOffset[0] = uiCompOffset[1];

			uiCompID[1] = uiCurID;
			uiCompOffset[1] = uiCurOffset;
		}
	}

	return NON_DUP_ID;
}


/*******************************************************************************
 *
*******************************************************************************/
void dInitDupList( pst_DupList pstList )
{
	int		i;

	memset( pstList, 0x00, sizeof(st_DupList) );

	for( i=0; i<MAX_DUP_NODE; i++ ) {
		if( i < (MAX_DUP_NODE-1) )
            pstList->stNode[i].usNext = i+1;
        else
            pstList->stNode[i].usNext = 0;

        if( (MAX_DUP_NODE-1-i) > 0 )
            pstList->stNode[MAX_DUP_NODE-1-i].usPrev = MAX_DUP_NODE-2-i;
        else
            pstList->stNode[MAX_DUP_NODE-1-i].usPrev = MAX_DUP_NODE-1;

	}
}

/*******************************************************************************
 *
*******************************************************************************/
int dSearchDupList( pst_DupList pstList, unsigned short usIdentification, unsigned short usFragmentOffset )
{
	unsigned short	usCount, usIndex;

	usCount = pstList->usCurrentCount;
	usIndex = pstList->usLastIndex;

	while( usCount > 0 ) {
		if(usIdentification == pstList->stNode[usIndex].usIdentification ) {
            if( usFragmentOffset == pstList->stNode[usIndex].usFragmentOffset ) {
                dAppLog( LOG_INFO, "##### DUPLICATE PACKET IDEN:%5d FRAG:%5d INDEX:%02d",
                        			usIdentification, usFragmentOffset, usIndex );
                return DEF_DUP_ID;
            }
        }

        usIndex = pstList->stNode[usIndex].usPrev;
        usCount--;
	}

	/*
    * DEL FIRST & INSERT NEW
    */
    if( pstList->usFirstIndex == pstList->stNode[pstList->usLastIndex].usNext ) {
        /* LIST FULL */
        pstList->usFirstIndex = pstList->stNode[pstList->usFirstIndex].usNext;
        pstList->usLastIndex = pstList->stNode[pstList->usLastIndex].usNext;

        pstList->stNode[pstList->usLastIndex].usIdentification = usIdentification;
        pstList->stNode[pstList->usLastIndex].usFragmentOffset = usFragmentOffset;
    }
    else {
        pstList->usLastIndex = pstList->stNode[pstList->usLastIndex].usNext;

        pstList->stNode[pstList->usLastIndex].usIdentification = usIdentification;
        pstList->stNode[pstList->usLastIndex].usFragmentOffset = usFragmentOffset;

        pstList->usCurrentCount++;
    }

	return 0;
}


/*******************************************************************************
 *
*******************************************************************************/
int dSearchTCPDupList( pst_DupList pstList, unsigned short usIdentification )
{
    unsigned short  usCount, usIndex;

    usCount = pstList->usCurrentCount;
    usIndex = pstList->usLastIndex;

    while( usCount > 0 ) {
        if(usIdentification == pstList->stNode[usIndex].usIdentification ) {
        	dAppLog( LOG_INFO, "##### DUPLICATE PACKET IDEN:%5d INDEX:%02d",
                               usIdentification, usIndex );
        	return DEF_DUP_ID;
        }

        usIndex = pstList->stNode[usIndex].usPrev;
        usCount--;
    }

    /*
    * DEL FIRST & INSERT NEW
    */
    if( pstList->usFirstIndex == pstList->stNode[pstList->usLastIndex].usNext ) {
        /* LIST FULL */
        pstList->usFirstIndex = pstList->stNode[pstList->usFirstIndex].usNext;
        pstList->usLastIndex = pstList->stNode[pstList->usLastIndex].usNext;

        pstList->stNode[pstList->usLastIndex].usIdentification = usIdentification;
    }
    else {
        pstList->usLastIndex = pstList->stNode[pstList->usLastIndex].usNext;

        pstList->stNode[pstList->usLastIndex].usIdentification = usIdentification;

        pstList->usCurrentCount++;
    }

    return 0;
}

