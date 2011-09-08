#ifndef _TRACE_COMM_H_
#define _TRACE_COMM_H_

/**
 *	Include headers
 */
// DQMS
#include "common_stg.h"			/* st_TraceFile */
#include "capdef.h"					/* st_beacon_hdr */

// TAM
#include "rppi_def.h"				/* DEF_IRMHASH_KEY_SIZE */

#define MAX_PKT_CNT					500
#define	MAX_TRACEBUF				sizeof(st_beacon_hdr)+MAX_MTU_SIZE

typedef struct _st_Trace
{
    int 		dPrev;
    int 		dNext;

	int			dMyIdx;
    time_t  	time;

	time_t  	mtime;
	int			len;

    char    	cUsed;
    char    	reserved[3];

	char		data[MAX_TRACEBUF];
} st_Trace, *pst_Trace;

typedef struct _TraceFile
{
	st_TraceInfo	stTraceInfo;
	char			szFilename[128];
	FILE*			fp;
	UINT			fcnt;	
	UINT			pktcnt;
	time_t			lastuptime;
	time_t			createtime;

	int				usedhead;
	int				usedtail;
	int				freehead;
	int				freetail;

	int				dUsedCnt;
	int				dFreeCnt;

	st_Trace 		stTraceList[MAX_PKT_CNT];
} st_TraceFile, *pst_TraceFile;

typedef struct _TraceFileList
{
	st_TraceFile 	stTraceFile[MAX_TRACE_CNT];
} st_TraceFileList, *pst_TraceFileList;

#endif	/* _TRACE_COMM_H_ */
