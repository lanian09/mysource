/*******************************************************************************
            DQMS Project

    Author   : Jae Seung Lee
    Section  : SI_SVC
    SCCS ID  : @(#)si_svc_main.c    1.1
    Date     : 01/21/05
    Revision History :
        '05. 01. 21     Initial
        '08. 01. 07     Update By LSH for review
        '08. 01. 14     Add By LSH for IUPS NTAM        

    Description :   

    Copyright (c) uPRESTO 2005
*******************************************************************************/
    
/***** A.1 * File Include *******************************/
#include <stdio.h>

// DQMS
#include "capdef.h"

// LIB
#include "loglib.h"

// .
#include "trace_link.h"

/** B.1* DEFINITION OF NEW CONSTANTS **********************/
/** C.1* DEFINITION OF NEW TYPES **************************/
/** D.1* DECLARATION OF VARIABLES *************************/
/** E.1* DEFINITION OF FUNCTIONS **************************/
/** E.2* DEFINITION OF FUNCTIONS **************************/
extern int FileCheck(st_TraceFile *pstTraceFile, st_TraceMsgHdr *pstTraceMsg);



void Init_Trace_Array( pst_TraceFile pstTraceFile)
{
	int i;

	pstTraceFile->freehead = 0;
	pstTraceFile->freetail = MAX_PKT_CNT-1;

	pstTraceFile->usedhead = 0;
	pstTraceFile->usedtail = 0;

	pstTraceFile->dUsedCnt = 0;
	pstTraceFile->dFreeCnt = MAX_PKT_CNT-1;

	pstTraceFile->createtime = 0;
	pstTraceFile->fp = NULL;

	for( i = 0; i < MAX_PKT_CNT ; i++ )
	{
		pstTraceFile->stTraceList[i].dPrev = i - 1;	 /* freehead´ÂPrev Index°¡ -1 */
		pstTraceFile->stTraceList[i].dNext = i + 1;

		pstTraceFile->stTraceList[i].dMyIdx = i;

		pstTraceFile->stTraceList[i].time= 0;
		pstTraceFile->stTraceList[i].mtime= 0;

		memset( pstTraceFile->stTraceList[i].data, 0, MAX_TRACEBUF );

		pstTraceFile->stTraceList[i].cUsed = -1;
	}
	pstTraceFile->stTraceList[0].dPrev = 0;	 /* freehead´ÂPrev Index°¡ -1 */
	pstTraceFile->stTraceList[MAX_PKT_CNT-1].dNext = 0;

	pstTraceFile->stTraceList[pstTraceFile->freetail].dNext = 0;	/* tail: Next Index°¡ -1 */
}

int dMallocIdx(pst_TraceFile pstTraceFile, time_t time, time_t mtime, int len, char *szData)
{
	int freeIndex = pstTraceFile->freetail;

	if ( pstTraceFile->stTraceList[freeIndex].dPrev == 0 )
	{
		return 0;
	}

	/* Free Linker °ª d¸® */
	pstTraceFile->freetail = pstTraceFile->stTraceList[freeIndex].dPrev;
	pstTraceFile->stTraceList[pstTraceFile->freetail].dNext = 0;

	/* »õî¥ÀÅ¸¦ 'Ç Ã±â */
	pstTraceFile->stTraceList[freeIndex].dNext = 0;
	pstTraceFile->stTraceList[freeIndex].dPrev = 0;

	pstTraceFile->stTraceList[freeIndex].time = time;
	pstTraceFile->stTraceList[freeIndex].mtime = mtime;

	pstTraceFile->stTraceList[freeIndex].len = len;
	memcpy( pstTraceFile->stTraceList[freeIndex].data, szData, len );

	pstTraceFile->stTraceList[freeIndex].cUsed = 1;

	++pstTraceFile->dUsedCnt;
	--pstTraceFile->dFreeCnt;

	return freeIndex;
}

int CompTime(int time_a, int mtime_a, int time_b, int mtime_b)
{
	if( time_a > time_b )
		return 1;
	else if ( time_a < time_b )
		return -1;
	else
	{
		if ( mtime_a > mtime_b )
			return 1;
		else if ( mtime_a < mtime_b )
			return -1;
	}

	return 0;
}

int PushTrace(pst_TraceFile pstTraceFile, time_t time, time_t mtime, int len, char *szData)
{
	int idx, comp, newidx;
	pst_Trace pstTrace;
	pst_Trace pstNextTrace;
	pst_Trace pstNewTrace;
	pst_Trace pstWriteTrace;

	if ( pstTraceFile->usedhead == 0 || pstTraceFile->usedtail == 0 ) /* First Trace Creation */
	{
		idx = pstTraceFile->usedhead = pstTraceFile->usedtail 
			= dMallocIdx(pstTraceFile, time, mtime, len, szData);
		if ( idx==0 )
		{
			return -1;
		}
	}
	else
	{
		idx = pstTraceFile->usedtail;

		while ( idx>0 )
		{
			pstTrace = &pstTraceFile->stTraceList[idx];

			comp = CompTime( time, mtime, pstTrace->time, pstTrace->mtime );

			if ( comp<0 )		/* if time for new packet is smaller than time of compared idx, continue; */
				idx = pstTrace->dPrev;
			else if ( comp > 0 )
			{
				/* Trace Exist */
				if ( pstTrace->cUsed<0 ) 
				{
					/* Wrong Reference */
					log_print(LOGN_CRI, "dUpdate_Trace: WRONG REFERENCE" );
					return -1;
				}

				newidx = dMallocIdx(pstTraceFile, time, mtime, len, szData);
				if ( newidx==0 )
				{
					pstWriteTrace = &pstTraceFile->stTraceList[pstTraceFile->usedhead];
					writepacket( pstTraceFile, pstWriteTrace->len, pstWriteTrace->data );	/* Oldest Packet Write */
					PopTrace( pstTraceFile );
					newidx = dMallocIdx(pstTraceFile, time, mtime, len, szData);
				}

				pstNewTrace = &pstTraceFile->stTraceList[newidx];

				if ( idx == pstTraceFile->usedtail )
				{
					/* because nextidex do not exist, need not to set trace  */

					/* newidx Setting */
					pstNewTrace->dPrev = idx;

					pstTraceFile->usedtail = newidx;
				}
				else
				{
					/* nextidx setting */
					pstNextTrace = &pstTraceFile->stTraceList[pstTrace->dNext];
					pstNextTrace->dPrev = newidx;

					/* newidx Setting */
					pstNewTrace->dNext = pstTrace->dNext;
					pstNewTrace->dPrev = idx;
				}

				/* previdx setting */
				pstTrace->dNext = newidx;

				break;
			}
			else
			{
				log_print(LOGN_WARN, "PACEKT SAME TIME NEW PKT[%ld.%ld] OLD PKT[%ld.%ld]", 
					time, mtime, pstTrace->time, pstTrace->mtime );

				return -1;
			}
		}

		if ( idx==0 ) /* usedheadº¸´ٵµ À: ½ð£ */
		{
			pstTrace = &pstTraceFile->stTraceList[pstTraceFile->usedhead];

			newidx = dMallocIdx(pstTraceFile, time, mtime, len, szData);
			if ( newidx==0 )
				writepacket( pstTraceFile, len, szData );	/* Oldest Packet Write */
			else
			{
				/* nextidx setting */
				pstTrace->dPrev = newidx;

				/* newidx setting */
				pstNewTrace = &pstTraceFile->stTraceList[newidx];
				pstNewTrace->dNext = pstTraceFile->usedhead;
				pstTraceFile->usedhead = newidx;		/* 20060807 */
			}
		}
	}

	return idx;
}

int FlushData(pst_TraceFile pstTraceFile)
{
	int idx;
	pst_Trace	pstHeadTrace;

    while ( pstTraceFile->usedhead>0 )
	{
		pstHeadTrace = GetTrace( pstTraceFile, pstTraceFile->usedhead );
		writepacket( pstTraceFile, pstHeadTrace->len, pstHeadTrace->data );
		PopTrace( pstTraceFile );	/* Pop Out head idx */
	}

	return idx;
}

void writepacket(pst_TraceFile pstTraceFile, int len, char *data)
{
	short trial;

    if( fwrite( data, len, 1, pstTraceFile->fp ) != 1 )
	{
		log_print(LOGN_WARN, "WriteTraceData: FileWrite Error [%d] [%s]", errno, strerror(errno));
        for( trial=0; trial<5; trial++)
        {
            if ( pstTraceFile->fp!=NULL )
            {
				log_print(LOGN_CRI, "WriteTraceData: File Closed [%s]", pstTraceFile->szFilename);
                fclose(pstTraceFile->fp);
                pstTraceFile->fp = NULL;
            }

            /* When Fail File Write */
            if ( FileCheck(pstTraceFile, NULL)<0 )
                return;

    		if( fwrite( data, len, 1, pstTraceFile->fp ) == 1 )
                break;
			else
				log_print(LOGN_WARN, "WriteTraceData: FileWrite Error [%d] [%s]", errno, strerror(errno));
        }
    }
	
	if ( pstTraceFile->stTraceInfo.dType==TRC_TYPE_IMSI )
        log_print(LOGN_INFO, "[WRITE] IMSI [%s]", pstTraceFile->stTraceInfo.stTraceID.szMIN );
	else if ( pstTraceFile->stTraceInfo.dType==TRC_TYPE_MDN )
        log_print(LOGN_INFO, "[WRITE] MDN [%s]", pstTraceFile->stTraceInfo.stTraceID.szMIN );
	else if ( pstTraceFile->stTraceInfo.dType==TRC_TYPE_ROAM_IMSI )
        log_print(LOGN_INFO, "[WRITE] ROAM IMSI [%s]", pstTraceFile->stTraceInfo.stTraceID.szMIN );
	else if ( pstTraceFile->stTraceInfo.dType==TRC_TYPE_ROAM_MDN )
        log_print(LOGN_INFO, "[WRITE] ROAM MDN [%s]", pstTraceFile->stTraceInfo.stTraceID.szMIN );
    else
        log_print(LOGN_INFO, "[WRITE] IP [%u]", pstTraceFile->stTraceInfo.stTraceID.uIP );


	fflush( pstTraceFile->fp );

	return;
}

int PopTrace(pst_TraceFile pstTraceFile)
{
	pst_Trace pstTrace;
	pst_Trace pstPrevTrace;
	pst_Trace pstNextTrace;
	pst_Trace pstFreeTailTrace;

	int idx = pstTraceFile->usedhead;

	if ( pstTraceFile->usedhead == 0 || pstTraceFile->usedtail == 0 )
	{
		return -1;	/* No Data */
	}

	pstTrace = GetTrace( pstTraceFile, pstTraceFile->usedhead );
	pstPrevTrace = GetTrace(pstTraceFile, pstTrace->dPrev);
	pstNextTrace = GetTrace(pstTraceFile, pstTrace->dNext);

	if ( pstNextTrace!=NULL )
	{
		pstNextTrace->dPrev = 0;
		pstTraceFile->usedhead = pstTrace->dNext;
	}
	else /* else idx == usedhead idex == usedtail idx */
	{
		pstTraceFile->usedhead = 0;
		pstTraceFile->usedtail = 0;
	}

	if ( pstTraceFile->freetail > 0 )
	{
		pstTrace->dPrev = pstTraceFile->freetail;
		pstTrace->dNext = 0;
		pstFreeTailTrace = GetTrace( pstTraceFile, pstTraceFile->freetail );
		pstFreeTailTrace->dNext = idx;
	}
	else
	{
		pstTrace->dPrev = 0;
		pstTrace->dNext = 0;
	}

	pstTraceFile->freetail = idx;
	pstTraceFile->stTraceList[idx].time = 0;
	pstTraceFile->stTraceList[idx].mtime = 0;
	pstTraceFile->stTraceList[idx].cUsed = -1;

	--pstTraceFile->dUsedCnt;
	++pstTraceFile->dFreeCnt;

	return pstTraceFile->freetail;
}

pst_Trace GetTrace(pst_TraceFile pstTraceFile, int idx)
{
	if ( idx==0 || idx >=MAX_PKT_CNT )
		return NULL;

	return &pstTraceFile->stTraceList[idx]; 
}


