/*******************************************************************************
				KTF IPAF Project

   Author	: joonhk
   Section	: 
   SCCS ID	: %W%
   Date		: %G%
   Revision History :
   	'03.  1.  6	Initial


   Description :

   Copyright (c) Infravalley 2003
*******************************************************************************/

/**A.1*	FILE INCLUSION ********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include <ipaf_svc.h>
#include <ipaf_shm.h>
#include <ipaf_define.h>
#include <ipaf_names.h>
#include <eth_capd.h>
#include <shmutil.h>
#include <utillib.h>

/**B.1*	DEFINITION OF NEW CONSTANTS *******************************************/

/**B.2*	DEFINITION OF NEW TYPE ************************************************/

/**C.1*	DECLARATION OF VARIABLES **********************************************/
extern	T_CAPBUF		*shm_capbuf;
extern	T_CAP			*shm_cap;

/**C.2*	DECLARATION OF FUNCTIONS **********************************************/

/*******************************************************************************
 * Shared Memory 읽는 함수 
*******************************************************************************/
int Read_m_shm( int dIndex, T_CAP *P, unsigned char *puBuffer )
{
	T_CAPHDR	*stHdr;

	int	dTemplength 	= 0;
	int dByteToRead		= 0;

	long lWritePos		= P->WritePos;
	long *lReadPos		= &P->ReadPos[dIndex];
	long *lWriteEndPos	= &P->WriteEndPos;

//	p( semid_capindex );

	/*
	if( lWritePos != *lReadPos )
	dAppLog( LOG_INFO, "[Read_m_shm][INFO] W_POS[%d] R_POS[%d] W_EPOS[%d]",
				lWritePos, *lReadPos, *lWriteEndPos );	
	*/

	if( ( lWritePos > MRG_MEM_SIZE ) || ( *lReadPos > MRG_MEM_SIZE ) ) {
		
		dAppLog( LOG_CRI, "[Read_m_shm][0] BUF ERROR : W[P:%d] R[P:%d] ",  
			P->WritePos, P->ReadPos[dIndex] );

	//	v( semid_capindex );

		return Err_POINTER;
	}

	while(1)
	{
		if( lWritePos == *lReadPos ) {
		//	v( semid_capindex );
			return Err_BUF_EMPTY;
		}
		else if( lWritePos < *lReadPos ) {
			if( *lWriteEndPos == *lReadPos ) {
                *lReadPos = 0;
                lReadPos    = &P->ReadPos[dIndex];
                lWriteEndPos = &P->WriteEndPos;

                continue;
            }
            else if( *lWriteEndPos < *lReadPos ) {
				dAppLog( LOG_CRI, "[Read_m_shm][1] READPOS[%d] GREATER THAN WRITEENDPOS[%d]",
					*lReadPos, *lWriteEndPos ); 

			//	v( semid_capindex );
                return Err_CRITICAL;
            }
            else { /*** *lWriteEndPos > *lReadPos */
                dByteToRead = *lWriteEndPos - *lReadPos;
            }

            break;
		}

		dByteToRead = lWritePos - *lReadPos;

		break;	
	}

	/* 
	* READ A PACKET HEADER 
	*/
	stHdr = (T_CAPHDR *) &shm_capbuf->Mem[*lReadPos];
	dTemplength = sizeof( T_CAPHDR ) + stHdr->datalen;

	/*
	dAppLog( LOG_INFO, "[Read_m_shm][INFO] TMP_LEN[%d] TO_READ[%d]",
                dTemplength, dByteToRead );
	*/

	/* 
	* ERROR CHECK - 2002-07-08 : 읽어야 할 데이타의 Size에 대한 체크
	*/
	if( ( dTemplength < 0 ) || ( dTemplength > MAX_BUF_SIZE ) ) {

		dAppLog( LOG_CRI, "[Read_m_shm][2] LENGTH ERROR : LEN[%d] HDR[%d] DATA[%d]", 
			dTemplength, sizeof( T_CAPHDR ) , stHdr->datalen );

	//	v( semid_capindex );
		return Err_POINTER;
	}

	/* 
	* ERROR CHECK 
	*/
	if( lWritePos > *lReadPos ) {
        if( lWritePos < (*lReadPos + dTemplength) ) {

            dAppLog( LOG_CRI, "[Read_m_shm][3] READ BUFFER ERROR : Wp[%d] Rp[%d] datalen[%d]",
                        lWritePos, *lReadPos, dTemplength);

            *lReadPos = lWritePos;

		//	v( semid_capindex );
            return Err_POINTER;
        }
    }
    else {
        if( *lWriteEndPos < (*lReadPos + dTemplength) ) {

            dAppLog( LOG_CRI, "[Read_m_shm][4] READ BUFFER ERROR : WEndp[%d] Rp[%d] datalen[%d]",
                        *lWriteEndPos, *lReadPos, dTemplength);

            *lReadPos = *lWriteEndPos;
	
		//	v( semid_capindex );
            return Err_POINTER;
        }
    }

	/*** READ A PACKET */
	memcpy( puBuffer, &shm_capbuf->Mem[*lReadPos], dTemplength );
	
	/*** UPDATE A READ POSITION */
	*lReadPos += dTemplength;
	dByteToRead -= dTemplength;

/*
	dAppLog( LOG_CRI, "[Read_m_shm][INFO][%d] W_POS[%d] R_POS[%d] W_EPOS[%d] TO_READ[%d]",
                dTemplength, lWritePos, *lReadPos, *lWriteEndPos, dByteToRead );
*/

//	v( semid_capindex );

	return dByteToRead;
}


/*******************************************************************************
 * Shared Memory에 쓰는 함수
*******************************************************************************/
/* To reduce a memory copy */
int Write_m_shm( T_CAP *P, int datalen, T_CAPHDR *caphdr, void *data)
{
	long	ReadPos = 0;
	long	*lWritePos = &P->WritePos;
	long	*lWriteEndPos = &P->WriteEndPos;
	int dLength = datalen + CAP_HDR_SIZE;

	p( semid_capindex );

	/*** ERROR CHECK */
	if( (*lWritePos > MRG_MEM_SIZE) ) {
		dAppLog( LOG_CRI, "[Write_m_shm][0] BUF ERROR : W[P:%d]", P->WritePos );

		v( semid_capindex );
		return Err_POINTER;
	}

	/*** ERROR CHECK */
	if( ( dLength < 0 ) || ( dLength > MAX_BUF_SIZE ) ) {
		dAppLog( LOG_CRI, "[Write_m_shm][1] LENGTH ERROR : LEN[%d]", dLength );

		v( semid_capindex );
		return Err_POINTER;
	}

	dGetLastReadPos( &ReadPos, P );	

	/*
	dAppLog( LOG_INFO, "[Write_m_shm][INFO] W_POS[%d] R_POS[%d] W_EPOS[%d] DATALEN[%d]",
                *lWritePos, ReadPos, *lWriteEndPos, dLength );
	*/

	while(1) {
		if( (*lWritePos < ReadPos) && (*lWritePos+dLength >= ReadPos) ) {
			v( semid_capindex );
			return Err_BUF_FULL;
		}

		if( (*lWritePos + dLength) > MRG_MEM_SIZE ) {
			*lWriteEndPos = *lWritePos;
			*lWritePos = 0;
			lWritePos = &P->WritePos;
			lWriteEndPos = &P->WriteEndPos;

			continue;
		} 
		
		break;
	}

	/*
	dAppLog( LOG_INFO, "[Write_m_shm][2] W_POS[%d] R_POS[%d] W_EPOS[%d]",
                *lWritePos, ReadPos, *lWriteEndPos );
	*/

    /*** WRITE A PACKET */
    memcpy( &shm_capbuf->Mem[*lWritePos], caphdr, CAP_HDR_SIZE);
    memcpy( &shm_capbuf->Mem[*lWritePos+CAP_HDR_SIZE], data, datalen);

    /*** WritePos UPDATE */
    *lWritePos += dLength;

/*
	dAppLog( LOG_CRI, "[Write_m_shm][INFO][%d] W_POS[%d] R_POS[%d] W_EPOS[%d]",
                dLength, *lWritePos, ReadPos, *lWriteEndPos );
*/

	v( semid_capindex );

	return 1;
}

#if 0
int Write_m_shm( T_CAP *P, int dLength, unsigned char *puBuffer)
{
    long    ReadPos = 0;
    long    *lWritePos = &P->WritePos;
    long    *lWriteEndPos = &P->WriteEndPos;

    p( semid_capindex );

    /*** ERROR CHECK */
    if( (*lWritePos > MRG_MEM_SIZE) ) {
        dAppLog( LOG_CRI, "[Write_m_shm][0] BUF ERROR : W[P:%d]", P->WritePos );

        v( semid_capindex );
        return Err_POINTER;
    }

    /*** ERROR CHECK */
    if( ( dLength < 0 ) || ( dLength > MAX_BUF_SIZE ) ) {
        dAppLog( LOG_CRI, "[Write_m_shm][1] LENGTH ERROR : LEN[%d]", dLength );

        v( semid_capindex );
        return Err_POINTER;
    }

    dGetLastReadPos( &ReadPos, P );

    /*
    dAppLog( LOG_INFO, "[Write_m_shm][INFO] W_POS[%d] R_POS[%d] W_EPOS[%d] DATALEN[%d]",
                *lWritePos, ReadPos, *lWriteEndPos, dLength );
    */

    while(1) {
        if( (*lWritePos < ReadPos) && (*lWritePos+dLength >= ReadPos) ) {
            v( semid_capindex );
            return Err_BUF_FULL;
        }

        if( (*lWritePos + dLength) > MRG_MEM_SIZE ) {
            *lWriteEndPos = *lWritePos;
            *lWritePos = 0;
            lWritePos = &P->WritePos;
            lWriteEndPos = &P->WriteEndPos;

            continue;
        }

        break;
    }

    /*
    dAppLog( LOG_INFO, "[Write_m_shm][2] W_POS[%d] R_POS[%d] W_EPOS[%d]",
                *lWritePos, ReadPos, *lWriteEndPos );
    */

    /*** WRITE A PACKET */
    memcpy( &shm_capbuf->Mem[*lWritePos], puBuffer, dLength );

    /*** WritePos UPDATE */
    *lWritePos += dLength;

    /*
    dAppLog( LOG_INFO, "[Write_m_shm][INFO] W_POS[%d] R_POS[%d] W_EPOS[%d]",
                *lWritePos, ReadPos, *lWriteEndPos );
    */

    v( semid_capindex );

    return 1;
}
#endif

/*******************************************************************************

*******************************************************************************/
int dGetLastReadPos( long *LastReadPos, T_CAP *P )
{
	int			i;

	long		LeftMinPos = MRG_MEM_SIZE;
	long		RightMinPos = MRG_MEM_SIZE;
 
	long		WritePos;
	long		ReadPos;

	WritePos = P->WritePos;

	for( i=0; i<MRG_READER_COUNT; i++ ) {
		ReadPos = P->ReadPos[i];

		if( ReadPos < 0 )
			continue;

		if( ReadPos <= WritePos ) {
			if( ReadPos < LeftMinPos )
				LeftMinPos = ReadPos;
		}
		else {
			if( ReadPos < RightMinPos )
				RightMinPos = ReadPos;
		}
	}

	if( RightMinPos == MRG_MEM_SIZE )
		*LastReadPos = LeftMinPos;
	else
		*LastReadPos = RightMinPos;

	return 0;
}
