#ifndef _ITCP_FUNC_H_
#define _ITCP_FUNC_H_

/**
 *	Include headers
 */
// TOP
#include "common_stg.h"
#include "capdef.h"

// LIB
#include "typedef.h"
#include "mems.h"
#include "hasho.h"
#include "timerN.h"
#include "Analyze_Ext_Abs.h"

/**
 * Declare functions
 */
extern S32 dProcTcpSess(stMEMSINFO *pMEMSINFO, stHASHOINFO *pTCPHASH, stTIMERNINFO *pTIMER, Capture_Header_Msg *pCAPHEAD, INFO_ETH *pINFOETH, U8 *pNode);
extern TCP_SESS *pCreateSession(stMEMSINFO *pMEMSINFO, stHASHOINFO *pTCPHASH, stTIMERNINFO *pTIMER, TCP_SESS_KEY *pTCPSESSKEY, Capture_Header_Msg *pCAPHEAD, INFO_ETH *pINFOETH, U8 ucRtxType, U8 ucControl);
extern S32 dCloseSession(stMEMSINFO *pMEMSINFO, stHASHOINFO *pTCPHASH, TCP_SESS_KEY *pTCPSESSKEY, TCP_SESS *pTCPSESS);
extern S32 dCheckSessKeyList(stMEMSINFO *pMEMSINFO, TCP_SESS_KEY *pTCPSESSKEY, TCP_SESS *pTCPSESS);
extern S32 dCheckAck(stMEMSINFO *pMEMSINFO, TCP_SESS_KEY *pTCPSESSKEY, TCP_SESS *pTCPSESS, Capture_Header_Msg *pCAPHEAD, INFO_ETH *pINFOETH, U8 ucRtxType);
extern S32 dInsertTCPData(stMEMSINFO *pMEMSINFO, TCP_SESS_KEY *pTCPSESSKEY, TCP_SESS *pTCPSESS, Capture_Header_Msg *pCAPHEAD, INFO_ETH *pINFOETH, U8 *pNode, S32 *delFlag);

#endif	/* _ITCP_FUNC_H_ */
