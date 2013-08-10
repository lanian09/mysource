#ifndef _ARP_SERV_H_
#define _ARP_SERV_H_

/**
 *	Include headers
 */
// TOP
#include "capdef.h"

// LIB
#include "typedef.h"
#include "Analyze_Ext_Abs.h"

// TAF
#include "mmdb_psess.h"


/**
 * Declare functions
 */
extern int ProcA11( unsigned char *pAppData, INFO_ETH *pstEth, Capture_Header_Msg *pstCAP );
extern void InitPSESS( PSESS_DATA *pSESS );
extern int CreateA10( PSESS_DATA *pPSessData, INFO_ETH *pstEth, Capture_Header_Msg *pstCAP, U8 *pucRetry );
extern int RemoveA10( PSESS_DATA *pPSessData );
extern int ProcPPPSess( PSESS_DATA *pPSessData );
extern int Print_GREEntry(void);

#endif	/* _ARP_SERV_H_ */
