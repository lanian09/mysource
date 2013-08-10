#ifndef __RPPI_FUNC_H__
#define __RPPI_FUNC_H__

#include "typedef.h"
#include "common_stg.h"
#include "rppi_def.h"	/* HData_RPPI */

#define GET_ISRECALL(a) ((a == INIT_RECALL_CALLSTART) ? 1 : 0)

extern S32 dCallStartInfo(S32 type, U8 *data);
extern S32 dCallStopInfo(S32 type, U8 *data);
extern S32 dSignalInfo(U8 *data);
extern S32 dPAGESessInfo(U8 *data);
extern S32 dVODSessInfo(U8 *data);
extern S32 dHTTPSessInfo(U8 *data);
extern S32 dTCPSessInfo(U8 *data);
extern S32 dSIPSessInfo(U8 *data);
extern S32 dMSRPSessInfo(U8 *data);
extern S32 dVTSessInfo(U8 *data);
extern S32 dIVSessInfo(U8 *data);
extern S32 dIMSessInfo(U8 *data);
extern S32 dNotiSigInfo(U8 *data);
extern S32 dFTPSessInfo(U8 *data);
extern S32 dDIALUPSessInfo(U8 *data);
extern S32 dStartServiceInfo(U8 *data);
extern S32 dDNSSessInfo(U8 *data);
extern S32 dINETSessInfo(U8 *data);
extern S32 dIHTTPSessInfo(U8 *data);
extern S32 dITCPSessInfo(U8 *data);

extern S32 dUpdateRPPI(LOG_RPPI *pstRPPILog, LOG_SIGNAL *pstSIGNAL, HData_RPPI *pstRPPIHash);
extern S32 dDelRPPI(HData_RPPI *pstRPPIHash, LOG_RPPI *pstRPPILog);
extern LOG_RPPI_ERR *dCreateErrLog(LOG_RPPI *pstRPPILog);
extern LOG_RPPI *pFindRPPILog(S32 isReCall, HData_RPPI *pstRPPIHash, U32 uiAccStartTime, U32 uiAccStartMTime);
extern void dUpdateCommonLog(LOG_COMMON *pLOG, LOG_RPPI *pstRPPILog);
extern S32 dProcCallStop(HData_RPPI *pstRPPIHash, LOG_RPPI *pstRPPILog, S32 isFirstServ);
extern S32 dGetA11UpdateCode( U16 usCallStatus, U16 usUpdateReason );
extern void vCheckFirst(HData_RPPI *pHASH, LOG_RPPI *pLOG, U8 *str, U32 SetupTime, U32 SetupMTime);
// recall
extern S32 dReCallSignalInfo(S32 type, U8 *data);
extern S32 dUpdateReCallRPPI(LOG_RPPI *pstRPPILog, LOG_SIGNAL *pstSIGNAL, HData_RPPI *pstRPPIHash);


#endif /* __RPPI_FUNC_H__ */
