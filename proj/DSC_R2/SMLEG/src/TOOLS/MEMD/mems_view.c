#include "ipaf_define.h"
#include "commlib.h"
#include "define.h"
#include "ipaf_names.h"
#include "mems.h"
#include "nifo.h"
#include "utillib.h"

stMEMSINFO 			*pstMEMSINFO;

int dPrintNIFOZONEStatus(void);

int main(void)
{
	time_t 	tLast = 0, tCur = 0;

	if( (pstMEMSINFO = nifo_init_zone("MEMS", SEQ_PROC_MEM, DEF_NIFO_ZONE_CONF_FILE)) == NULL )
	{
		dAppLog( LOG_CRI, "ERROR nifo_init_zone NULL");
		return -1;
	}

	tLast = tCur = time(0);
	while(1)
	{
		tCur = time(0);

		if( tCur != tLast )
		{
			dPrintNIFOZONEStatus();
			tLast = tCur;
			usleep(1);
		}
	}
	return 0;
}

int dPrintNIFOZONEStatus(void)
{
	int					i;
	float				fPercent;
	unsigned int		uiCurNIFO, uiMaxNIFO;
	unsigned int		uiAllocCnt = 0, uiTotCnt = 0;
	for(i=0;i<pstMEMSINFO->uiZoneCnt;i++)
	{
		uiCurNIFO   = mems_alloced_cnt(pstMEMSINFO, i);
		uiMaxNIFO   = pstMEMSINFO->stMEMSZONE[i].uiMemNodeTotCnt;

		fPercent = ((float)uiCurNIFO/(float)uiMaxNIFO) * 100.0;
		//dAppLog(LOG_CRI, "ZoneID[%d] fPercent[%f] uiCurNIFO[%lld] uiMaxNIFO[%lld]",
		//		i, fPercent, uiCurNIFO, uiMaxNIFO);
		fprintf(stderr, "ZoneID[%d] fPercent[%f] uiCurNIFO[%u] uiMaxNIFO[%u]\n",
				i, fPercent, uiCurNIFO, uiMaxNIFO);
		uiAllocCnt += uiCurNIFO;
		uiTotCnt += uiMaxNIFO;
	}

	fPercent = ((float)uiAllocCnt/(float)uiTotCnt) * 100.0;
	//dAppLog(LOG_CRI, "NIFO_ALL fPercent[%f] CurNIFO[%lld] MaxNIFO[%lld]",
//			fPercent, uiAllocCnt, uiTotCnt);
	fprintf(stderr, "NIFO_ALL fPercent[%f] CurNIFO[%u] MaxNIFO[%u]\n\n",
			fPercent, uiAllocCnt, uiTotCnt);
	return 0;
}
