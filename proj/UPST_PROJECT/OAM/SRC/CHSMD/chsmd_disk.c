/***** A.1 * File Include *******************************/

/* SYS HEADER */
#include <stdio.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/vfs.h>
#include <errno.h>
/* LIB HEADER */
#include "filedb.h"
#include "loglib.h"
/* PRO HEADER */
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */
#include "chsmd_disk.h"

extern pst_NTAM fidb;

int dGetDF(st_SoldfList *stSoldfList)
{
	int				i, dRet;
	struct statfs	buf;

	dRet	= 0;
	for(i = 0; i < stSoldfList->dCount; i++) {

		memset(&buf, sizeof(struct statfs), 0);
		if( (dRet = statfs(stSoldfList->stSoldf[i].szMountp, &buf)) < 0) {

			if(errno != ENOENT) {

				log_print(LOGN_CRI, LH"STATFS(%s) ERROR -> dRet=%d"EH, 
					LT, stSoldfList->stSoldf[i].szMountp, dRet, ET);
				return -1;
			}
			stSoldfList->stSoldf[i].llUsed		= 0;
			stSoldfList->stSoldf[i].llTotal		= 0;
			stSoldfList->stSoldf[i].dPercent	= 0;

		} else {
			stSoldfList->stSoldf[i].llUsed		= buf.f_blocks - buf.f_bavail;
			stSoldfList->stSoldf[i].llTotal		= buf.f_blocks;
			stSoldfList->stSoldf[i].dPercent	= 100*buf.f_bavail/buf.f_blocks;
			log_print(LOGN_INFO, "DISK IDX=%d USED=%lld TOTAL=%lld PERCENTAGE=%d",
					i, stSoldfList->stSoldf[i].llUsed, stSoldfList->stSoldf[i].llTotal, stSoldfList->stSoldf[i].dPercent);
		}
	}

	return 0;
}

int df_compute(st_df_total *stTotal, st_SoldfList *pstSolDfList)
{
	int		i;

	dInitializeData(stTotal);
	dGetDF(pstSolDfList);

	stTotal->dIndex = pstSolDfList->dCount;
	for(i = 0; i < stTotal->dIndex; i++) {

		stTotal->stData[i].dUsed		= pstSolDfList->stSoldf[i].llUsed;
		stTotal->stData[i].dSize		= pstSolDfList->stSoldf[i].llTotal;
		stTotal->stData[i].dAvail		= pstSolDfList->stSoldf[i].llTotal - pstSolDfList->stSoldf[i].llUsed;
		stTotal->stData[i].dUsedPercent	= (double)(100.0 - pstSolDfList->stSoldf[i].dPercent);

		fidb->disksts[i].llCur	= stTotal->stData[i].dUsed;
		fidb->disksts[i].lMax	= stTotal->stData[i].dSize;

		stTotal->dTotSize	+= stTotal->stData[i].dSize;
		stTotal->dTotUsed	+= stTotal->stData[i].dUsed;
		stTotal->dTotAvail	+= stTotal->stData[i].dAvail;

		sprintf(stTotal->stData[i].szName, "%s", pstSolDfList->stSoldf[i].szMountp);
		log_print(LOGN_INFO, "DISK IDX=%d PARTITION_NAME=%s CURR=%d TOTAL=%d",
				i, pstSolDfList->stSoldf[i].szMountp, stTotal->stData[i].dUsed, stTotal->stData[i].dSize);
	}

	return 1;
}

int dInitializeData( st_df_total *stTotal )
{
    int i = 0;

    stTotal->dIndex = 0;
    stTotal->dTotSize = 0;
    stTotal->dTotUsed = 0;
    stTotal->dTotAvail = 0;

    for( i=0 ; i < MAX_DF_DATA; i++ ) {

        memset( stTotal->stData[i].szName, 0x00, 50 );
        stTotal->stData[i].dSize = 0;
        stTotal->stData[i].dUsed = 0;
        stTotal->stData[i].dAvail = 0;
        stTotal->stData[i].dUsedPercent = 0;
    }

    return 1;
}
