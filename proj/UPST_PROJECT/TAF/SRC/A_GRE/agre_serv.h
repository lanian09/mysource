#ifndef _AGRE_SERV_H_
#define _AGRE_SERV_H_

/**
 *	Include headers
 */
// TOP
#include "capdef.h"

// TAF
#include "mmdb_psess.h"

/**
 * Declare functions
 */
extern int MergePPPData( PSESS_DATA *pPSessData, unsigned char *pData, int dSize, INFO_ETH *pstEth, Capture_Header_Msg *pstCAP );
extern int Get7EState( unsigned char *pBuf, short siSize, char *nPPPState, int *nNextPos, int *n7EOmit, int *n7EDup);
extern int ProcPPP( PSESS_DATA *pPSessData, unsigned char *pBuf, short siSize, struct slcompress *m_comp, INFO_ETH *pstEth, Capture_Header_Msg *pstCAP );
extern int CopyPacket( unsigned char *pDestBuf, short *psiLen, unsigned char *pSrcBuf, short siSize );



#endif	/* _AGRE_SERV_H_ */
