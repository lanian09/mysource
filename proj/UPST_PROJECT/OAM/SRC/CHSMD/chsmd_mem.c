/***** A.1 * File Include *******************************/

/* SYS HEADER */
#include <stdio.h>
#include <string.h>	/* STRCASECMP(3) */
/* LIB HEADER */
#include "filedb.h"
#include "loglib.h"
/* PRO HEADER */
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */
#include "chsmd_mem.h"

extern pst_NTAM fidb;

int mem_compute( st_mem_state *mem )
{
    char  szName[20], buffer[2048], szPath[20] = "/proc/meminfo";
    int   dRet, dTotal, dFree, dBuff, dCach;
    FILE *fp;

	fp = fopen( szPath, "r" );

	if( fp == NULL ) {
		log_print(LOGN_CRI,LH"FILE OPEN ERROR. FILE=%s"EH, LT,szPath,ET);
		return -1;
	}

	dRet = 0;
	while( fgets( buffer, 2048, fp ) != NULL ) {

		sscanf( buffer, "%s", szName );

		if( !strcasecmp(szName, "MemTotal:") ) {

			if( sscanf( buffer, "%*s %d %*s", &dTotal ) != 1 ) {
				dRet = -1;
				break;
			}

		} else if( !strcasecmp(szName, "MemFree:") ) {

			if( sscanf( buffer, "%*s %d %*s", &dFree) != 1 ) {
				dRet = -2;
				break;
			}

		} else if( !strcasecmp(szName, "Buffers:") ) {

			if( sscanf( buffer, "%*s %d %*s", &dBuff) != 1 ) {
				dRet = -3;
				break;
			}

		} else if( !strcasecmp(szName, "Cached:") ) {

			if( sscanf( buffer, "%*s %d %*s", &dCach) != 1 ) {
				dRet = -4;
				break;
			}
		}
		continue;

    }

    fclose(fp);

    if( dRet < 0 ) {
        log_print(LOGN_CRI,"FAILED IN mem_compute() :%d",dRet);
        return dRet;
    }

    mem->real_tot    = dTotal;
    mem->real_act    = dTotal - dFree - dBuff - dCach;
    mem->virtual_act = 0;
    mem->virtual_tot = 0;
    mem->free        = dFree;
    mem->use         = mem->real_act;

	fidb->memsts.llCur = mem->use;
	fidb->memsts.lMax  = mem->real_tot;

    return 1;
}

