/**A.1*  File Inclusion *******************************************************/

/* SYS HEADER */
#include <stdio.h>
#include <errno.h>
#include <mntent.h>
#include <sys/vfs.h>
/* LIB HEADER */
/* PRO HEADER */
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */
#include "chsmd_disk.h"	/* st_SoldfList, st_Soldf */

/**C.1*  Declaration of Variables  ********************************************/
/**C.2*  Declaration of Variables  ********************************************/
/**D.2*  Definition of Functions  *********************************************/
/*******************************************************************************
 * GET DISK USAGE INFORMATION
*******************************************************************************/
int dGetDF(st_SoldfList *stSoldfList)
{
	int				i, dRet;
	struct statfs	buf;

	dRet	= 0;
	for(i = 0; i < stSoldfList->dCount; i++)
	{
		if( (dRet = statfs(stSoldfList->stSoldf[i].szMountp, &buf)) < 0)
		{
			if(errno != ENOENT)
				return -1;

			stSoldfList->stSoldf[i].llUsed		= 0;
			stSoldfList->stSoldf[i].llTotal		= 0;
			stSoldfList->stSoldf[i].dPercent	= 0;
		}
		else
		{
			stSoldfList->stSoldf[i].llUsed		= buf.f_blocks - buf.f_bavail;
			stSoldfList->stSoldf[i].llTotal		= buf.f_blocks;
			stSoldfList->stSoldf[i].dPercent	= 100*buf.f_bavail/buf.f_blocks;
		}
	}

	return 0;
}
