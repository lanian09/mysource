#ifndef _TRACE_FUNC_H_
#define _TRACE_FUNC_H_

/**
 *	Include headers
 */
#include "trace_comm.h"

/**
 *	Declare functions
 */
extern void Init_Trace_Array( pst_TraceFile pstTraceFile);
extern int dMallocIdx(pst_TraceFile pstTraceFile, time_t time, time_t mtime, int len, char *szData);
extern int CompTime(int time_a, int mtime_a, int time_b, int mtime_b);
extern int PushTrace(pst_TraceFile pstTraceFile, time_t time, time_t mtime, int len, char *szData);
extern int FlushData(pst_TraceFile pstTraceFile);
extern void writepacket(pst_TraceFile pstTraceFile, int len, char *data);
extern int PopTrace(pst_TraceFile pstTraceFile);
extern pst_Trace GetTrace(pst_TraceFile pstTraceFile, int idx);

#endif	/* _TRACE_FUNC_H_ */
