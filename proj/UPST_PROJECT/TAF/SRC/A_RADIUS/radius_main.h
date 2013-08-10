#ifndef _RADIUS_MAIN_H_
#define _RADIUS_MAIN_H_

/**
 * Include headers
 */
#include <unistd.h>

// LIB
#include "common_stg.h"
#include "mems.h"
#include "cifo.h"
#include "nifo.h"
#include "gifo.h"
#include "loglib.h"
#include "verlib.h"

// DQMS
#include "procid.h"
#include "sshmid.h"
//#include "commdef.h"	/* Capture_Header_Msg */
#include "capdef.h"		/* Capture_Header_Msg */

// OAM
#include "path.h"

// TAF
#include "filter.h"
#include "Analyze_Ext_Abs.h"

#include "radius_comm.h"

/**
 * Declare variables
 */
stMEMSINFO		*pstMEMSINFO;
stCIFO			*gpCIFO;
stHASHOINFO		*pstHASHOINFO;
stTIMERNINFO	*pstTIMERNINFO;

st_TraceList	*pstTRACE;		/* TRACE */

S32				gACALLCnt = 0;

S32				giFinishSignal;
S32				giStopFlag;

S32				gTIMER_TRANS;

st_ippool 		stIPPool;

/**
 * Declare functions
 */
extern S32 dInitRADIUSProc(stMEMSINFO **pstMEMSINFO, stHASHOINFO **pstHASHOINFO, stTIMERNINFO **pstTIMERNINFO);
extern inline int dAnalyze_RADIUS( PUCHAR pBuf, pst_ACCInfo pstAccReq, INFO_ETH *pINFOETH );
extern int dProcRADIUS_Trans( Capture_Header_Msg *pCAPHEAD, INFO_ETH *pINFOETH, UCHAR *pDATA, st_ACCInfo *pstACCINFO);
extern void FinishProgram(void);


#endif	/* _RADIUS_MAIN_H_ */
