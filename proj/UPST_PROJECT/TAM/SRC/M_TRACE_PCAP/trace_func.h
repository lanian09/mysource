#ifndef _TRACE_FUNC_H_
#define _TRACE_FUNC_H_

/**
 * Include headers
 */
// DQMS
#include "common_stg.h"			/* st_TraceMsgHdr */

// .
#include "trace_comm.h"			/* st_TraceFile */

/**
 *	Define constants
 */
#define MAX_PKTCNTPERFILE			1000000
#define TRACEDIR					START_PATH"/TRACELOG/"

/**
 *	Declare functions
 */
extern int dTraceProc(const char* szData);
extern int FileCheck(st_TraceFile *pstTraceFile, st_TraceMsgHdr *pstTraceMsg);
extern int Check_TraceInfo();
extern int Check_ServTraceInfo();
extern void StopTrace(pst_TraceFile pstTraceFile);
extern void StartTrace(st_TraceInfo *sourceInfo);
extern void StartServerTrace(st_TraceInfo *sourceInfo);
extern void DisTrace(int level);
extern void DisServerTrace(int level);
extern S32 dMakeIRMHash();
extern S32 dCvtfromIRMtoIMSI(U8 *szIMSI);
extern S32 dCvtfromIRMtoMDN(U8 *szIMSI);

#endif	/* _TRACE_FUNC_H_ */
