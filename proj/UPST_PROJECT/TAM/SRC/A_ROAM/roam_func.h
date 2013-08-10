#ifndef _ROAM_FUNC_H_
#define _ROAM_FUNC_H_

/**
 *	INCLUDE HEADER FILES
 */
// DQMS
#include "common_stg.h"			/* LOG_COMMON, LOG_RPPI .. */

// LIB
#include "typedef.h"

// TAM
#include "rppi_def.h"

/**
 *	DECLARE FUNCTIONS
 */
extern void vCheckFirst(HData_RPPI *pHASH, LOG_RPPI *pLOG, U8 *str, U32 SetupTime, U32 SetupMTime);
extern S32 dPAGESessInfo(U8 *data);
extern S32 dVODSessInfo(U8 *data);
extern S32 dHTTPSessInfo(U8 *data);
extern S32 dTCPSessInfo(U8 *data);
extern S32 dSIPSessInfo(U8 *data);
extern S32 dMSRPSessInfo(U8 *data);
extern S32 dVTSessInfo(U8 *data);
extern S32 dIMSessInfo(U8 *data);
extern S32 dDIALUPSessInfo(U8 *data);
extern S32 dNotiSigInfo(U8 *data);
extern S32 dStartServiceInfo(U8 *data);
extern S32 dDNSSessInfo(U8 *data);
extern HData *pCallStartInfo(U8 *data);
extern S32 dAccessInfo(U8 *data);
extern S32 dAccountInfo(U8 *data);
extern S32 dLcpInfo(U8 *data);
extern S32 dIpcpInfo(U8 *data);
extern S32 dAuthInfo(U8 *data);
extern S32 dOtherPPPInfo(U8 *data);
extern S32 dL2TPInfo(U8 *data);
extern S32 dDelRPPI(HData_RPPI *pstRPPIHash, LOG_RPPI *pstRPPILog);
extern void dUpdateCommonLog(LOG_COMMON *pLOG, LOG_RPPI *pstRPPILog);
extern LOG_RPPI_ERR *dCreateErrLog(LOG_RPPI *pstRPPILog);
extern HData *pFindCallWithTime(HData_RPPI *pstRPPIHash, U32 uiAccStartTime, U32 uiAccStartMTime);
extern HData *pFindCallWithCorrID(HData_RPPI *pstRPPIHash, U64 ulCorrelationID);
extern S32 dProcCallStop(HData_RPPI *pstRPPIHash, LOG_RPPI *pstRPPILog, S32 isFirstServ);
extern S32 dFTPSessInfo(U8 *data);

#endif	/* _ROAM_FUNC_H_ */
