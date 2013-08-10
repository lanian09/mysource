#ifndef _FTP_SVC_H_
#define _FTP_SVC_H_

/**
 *	Include headers
 */
// TOP
#include "capdef.h"

// LIB
#include "typedef.h"
#include "common_stg.h"

// TAF
#include "mmdblist_ftp.h"

/**
 * Declare functions
 */
extern int dGetCALLProcID(unsigned int uiClientIP);
extern int dSvcProcess( TCP_INFO *pTCPINFO, Capture_Header_Msg *pCAPHEAD, U8 *pDATA );
extern int CmdParse(SESS_DATA *pstTcp, char *szData, int uDataSize, UINT uiTime, UINT uiMTime );
extern int ReplyParse(SESS_DATA *pstTcp, char *szData, int uDataSize, UINT uiTime, UINT uiMTime );
extern int dFTPMessage( TCP_INFO *pTCPINFO, Capture_Header_Msg  *pCAPHEAD, U8 *pDATA );
extern int dSessStartMessage( TCP_INFO *pTCPINFO );
extern int dTermTCP( PSESS_KEY pstKey );
extern int dSessStopMessage( TCP_INFO *pTCPINFO );
extern int dSend_FTPLOG( LOG_FTP *pstLog );
extern void CopyLOGFTP( LOG_FTP *pLOG_FTP, LOG_FTP_INFO *pLOG_SIG, LOG_FTP_INFO *pLOG_DATA );

#endif	/* _FTP_SVC_H_ */
