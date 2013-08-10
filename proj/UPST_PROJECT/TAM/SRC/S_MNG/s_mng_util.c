/**A.1*  File Inclusion *******************************************************/
#include <string.h>		/* MEMSET, MEMCPY */
#include <errno.h>		/* errno */
#include <stdlib.h>		/* ATOI */

#include "loglib.h"

time_t tGetTimeFromStr(char *szTime)
{
	int			i, dOffSet = 0;
	struct tm	stTime;
	time_t		tTime;
	char		szBuf[13];

	log_print(LOGN_INFO, "TIME [%s]", szTime);

	memset(&stTime, 0x00, sizeof(struct tm));
	memcpy(szBuf, szTime, 12);
	szBuf[12] = 0x00;

	for(i = 0; i < 12; i++)
	{
		if(szBuf[i] < 0x30 || szBuf[i] > 0x39)
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: IS NOT NUMBER TIME[%s] NAME[%s]", __FILE__, __FUNCTION__, __LINE__, szBuf, szTime);
			return -1;
		}
	}

	/* GET YEAR */
	szBuf[4] = 0x00;
	stTime.tm_year = atoi(szBuf) - 1900;
	dOffSet += 4;

	/* GET MON */
	memcpy(szBuf, &szTime[dOffSet], 2);
	szBuf[2] = 0x00;
	stTime.tm_mon = atoi(szBuf) - 1;
	dOffSet += 2;

	/* GET DAY */
	memcpy(szBuf, &szTime[dOffSet], 2);
	szBuf[2] = 0x00;
	stTime.tm_mday = atoi(szBuf);
	dOffSet += 2;

	/* GET HOUR */
	memcpy(szBuf, &szTime[dOffSet], 2);
	szBuf[2] = 0x00;
	stTime.tm_hour = atoi(szBuf);
	log_print(LOGN_INFO, "In util : szBuf [%s]", szBuf);
	dOffSet += 2;

	/* GET MIN */
	memcpy(szBuf, &szTime[dOffSet], 2);
	szBuf[2] = 0x00;
	stTime.tm_min = atoi(szBuf);

	stTime.tm_sec = 0;

	if((tTime = mktime(&stTime)) < 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: mktime ERROR RET[%d][%s]", __FILE__, __FUNCTION__, __LINE__, errno, strerror(errno));
		return -2;
	}

	return tTime ;
}
