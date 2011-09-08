#ifndef _DIAMETER_FUNC_H_
#define _DIAMETER_FUNC_H_

/**
 *	Include headers
 */

// DQMS
#include "capdef.h"

// LIB
#include "typedef.h"


/**
 * Declare functions
 */
S64 GetGapTime(STIME endtime, MTIME endmtime, STIME starttime, MTIME startmtime);
int dump(char *s,int len);
S32 dGetCALLProcID(U32 uiClientIP);
int dSendTransLog(Capture_Header_Msg *pCAPHEAD, HKey_Trans *pstTransKey, HData_Trans *pstTransData);
int cb_timeout_transaction(HKey_Trans *pstTransKey);
HData_Trans *pCreateTransaction(Capture_Header_Msg *pCAPHEAD, INFO_ETH *pINFOETH, HKey_Trans *pstTransKey);
int dCheck_TraceInfo( HData_Trans *pstSessData, UCHAR *pData, Capture_Header_Msg *pstCAP );
int dProcDIAMETER_Trans( Capture_Header_Msg *pCAPHEAD, INFO_ETH *pINFOETH, UCHAR *pDATA, UCHAR *pETHDATA, st_DiameterInfo *pstDIAMETERINFO);



#endif	/* _DIAMETER_FUNC_H_ */
