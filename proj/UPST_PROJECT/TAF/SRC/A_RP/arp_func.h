#ifndef _ARP_FUNC_H_
#define _ARP_FUNC_H_

/**
 *	Include headers
 */
// TOP
#include "capdef.h"

// LIB
#include "typedef.h"
#include "hasho.h"
#include "Analyze_Ext_Abs.h"

// TAF
#include "mmdb_psess.h"

/**
 *	Declare functions
 */
extern stHASHONODE *hashs_find(stHASHOINFO *pstHASHOINFO, U8 *pKey);
extern stHASHONODE *hashs_add(stHASHOINFO *pstHASHOINFO, U8 *pKey, U8 *pData);
extern void hashs_del(stHASHOINFO *pstHASHOINFO, U8 *pKey);
extern int dCheck_TraceInfo( PSESS_DATA *pstSessData, unsigned char *pData, Capture_Header_Msg *pstCAP );
extern unsigned int GetUpTime( time_t StartTime, int MStartTime, time_t EndTime, int MEndTime );
extern int Report_SIGLog( UCHAR ucProto, UCHAR ucMsgType, PSESS_DATA *pPSessData );
extern S32 dGetGREProcID( UINT uiClientIP );
extern void Print_INFO_A11( INFO_A11 *pINFOA11 );



#endif /* _ARP_FUNC_H_ */
