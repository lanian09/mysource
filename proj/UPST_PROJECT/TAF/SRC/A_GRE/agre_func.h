#ifndef _AGRE_FUNC_H_
#define _AGRE_FUNC_H_

/**
 *	Include headers
 */
// TOP
#include "capdef.h"

// LIB
#include "typedef.h"

// TAF
#include "mmdb_psess.h"		/* PSESS_DATA */

/**
 * Declare functions
 */
extern int Init_A11_PSESS();
extern int Init_TraceShm();
extern int Init_GREEntry_Shm(void);
extern int dCheck_TraceInfo( PSESS_DATA *pstSessData, unsigned char *pData, Capture_Header_Msg *pstCAP );
extern unsigned int GetUpTime( time_t StartTime, int MStartTime, time_t EndTime, int MEndTime );
extern int Report_SIGLog( UCHAR ucProtoType, UCHAR ucMsgType, PSESS_DATA *pPSessData );

#endif	/* _AGRE_FUNC_H_ */
