/**A.1* FILE INCLUSION ********************************************************/
#include <errno.h>
#include <common_stg.h>

#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>		/*	isspace(3)	*/
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "loglib.h"
#include "tools.h"

/*
	trace_tbl의 해당 파일로부터 shared memory가 초기화 될 때 파일로부터 로딩한다.
	 - Writer: Han-jin Park
	 - DAte: 2008.09.19
*/
int dGetTraceTblList(char *sFileName, st_TraceList *pst_TraceList)
{
	FILE			*fp;
	size_t			szLen;
	time_t			tExpiredTime;

	char			sBuf[512], sAdminID[MAX_USER_NAME_LEN], sMIN[MAX_MIN_SIZE];
	int				i, dSysNo, dType;
	long long		llIMSI;
	unsigned int	uIP;
	unsigned short	usEstimatedTime;

	if( (fp = fopen(sFileName, "r")) == NULL)
	{
		if( (fp = fopen(sFileName, "w")) == NULL)
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN fopen(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, sFileName, errno, strerror(errno));
		}
		else
			fclose(fp);

		return -1;
	}

	i		= 0;
	sBuf[0]	= 0x00;
	while(fgets(sBuf, 512, fp) != NULL)
	{
		szLen = strlen(sBuf);
		while(isspace(sBuf[szLen-1]))
			sBuf[--szLen] = 0x00;

		if(sBuf[0] != '#')
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN FILE(%s) LINE(%d) FORMAT", __FILE__, __FUNCTION__, __LINE__, sFileName, i);
			fclose(fp);
			return -2;
		}

		if(sBuf[1] == '#')
		{
			sBuf[0] = 0x00;
			continue;
		}
		else if(sBuf[1] == 'E')
			break;
		else
		{
			if(sscanf(&sBuf[2], "%d %s %lld %u %d %lu %s %hu", &dSysNo, sMIN, &llIMSI, &uIP, &dType, &tExpiredTime, sAdminID, &usEstimatedTime) == 8)
			{
				if(i >= MAX_TRACE_CNT)
				{
					log_print(LOGN_CRI, "F=%s:%s.%d: trace list(File:%s) count is over maximum(%d).", __FILE__, __FUNCTION__, __LINE__, sFileName, MAX_TRACE_CNT);
					fclose(fp);
					return -3;
				}

				if( (usEstimatedTime <= 0) || (usEstimatedTime > 73))
				{
					log_print(LOGN_CRI, "F=%s:%s.%d: Estimated Time Range Error[%d]", __FILE__, __FUNCTION__, __LINE__, i);
					fclose(fp);
					return -4;
				}

				strncpy(pst_TraceList->stTraceInfo[i].stTraceID.szMIN, sMIN, MAX_MIN_SIZE);
				strncpy(pst_TraceList->stTraceInfo[i].adminID, sAdminID, MAX_USER_NAME_LEN);

				pst_TraceList->stTraceInfo[i].stTraceID.llIMSI	= llIMSI;
				pst_TraceList->stTraceInfo[i].stTraceID.uIP		= uIP;
				pst_TraceList->stTraceInfo[i].dType				= dType;
				pst_TraceList->stTraceInfo[i].tExpiredTime		= tExpiredTime;
				pst_TraceList->stTraceInfo[i].usEstimatedTime	= usEstimatedTime;

				log_print(LOGN_INFO, "F=%s:%s.%d: Count[%d] SysNo[%d] MIN[%.*s] IMSI[%lld] IP[%u] Type[%d] ExpiredTime[%lu] sAdminID[%.*s] EstimatedTime[%hu]", __FILE__, __FUNCTION__, __LINE__,
					i, dSysNo, MAX_MIN_SIZE, pst_TraceList->stTraceInfo[i].stTraceID.szMIN,
					pst_TraceList->stTraceInfo[i].stTraceID.llIMSI, pst_TraceList->stTraceInfo[i].stTraceID.uIP,
					pst_TraceList->stTraceInfo[i].dType, pst_TraceList->stTraceInfo[i].tExpiredTime,
					MAX_USER_NAME_LEN, pst_TraceList->stTraceInfo[i].adminID, pst_TraceList->stTraceInfo[i].usEstimatedTime);

				i++;
			}
			sBuf[0] = 0x00;
		}
	} // while-loop end
	fclose(fp);
	pst_TraceList->count	= i;
	pst_TraceList->dSysNo	= dSysNo;

	return 0;
}

/*
	trace_tbl의 값이 변할 때마다 파일로 저장하고, 해당 파일로부터 shared memory가 초기화 될 때,
	dGetTraceTblList()를 이용하여 파일로부터 로딩한다.
	 - Writer: Han-jin Park
	 - DAte: 2008.09.19
*/
int dWriteTraceTblList(char *sFileName, st_TraceList *pst_TraceList)
{
	int		i;
	FILE	*fp;

	if(pst_TraceList->count > MAX_TRACE_CNT)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: file trace list[%s] count error, COUNT:[%d]", __FILE__, __FUNCTION__, __LINE__, sFileName, pst_TraceList->count);
		return -1;
	}

	if( (fp = fopen(sFileName, "w")) == NULL)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN fopen(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, sFileName, errno, strerror(errno));
		return -2;
	}

	fprintf(fp, "## Trace List FILE\n");
	fprintf(fp, "## SysNo    MIN    IMSI    IP    Type    ExpiredTime    adminID    EstimatedTime\n");
	fprintf(fp, "## EstimatedTime: 0 ~ 24[Default: 3hours]\n");

	for(i = 0; i < pst_TraceList->count; i++)
	{
		if( (pst_TraceList->stTraceInfo[i].usEstimatedTime <= 0) || (pst_TraceList->stTraceInfo[i].usEstimatedTime > 72))
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN FILE(%s) LINE(%d) FORMAT", __FILE__, __FUNCTION__, __LINE__, sFileName, i);
			fclose(fp);
			return -3;
		}

		fprintf(fp, "#@ %d %s %lld %u %d %u %s %hu\n",
			pst_TraceList->dSysNo, (pst_TraceList->stTraceInfo[i].stTraceID.szMIN[0] == 0x00) ? (U8 *)"NULL" : pst_TraceList->stTraceInfo[i].stTraceID.szMIN,
			pst_TraceList->stTraceInfo[i].stTraceID.llIMSI, pst_TraceList->stTraceInfo[i].stTraceID.uIP,
			pst_TraceList->stTraceInfo[i].dType, pst_TraceList->stTraceInfo[i].tExpiredTime,
			(pst_TraceList->stTraceInfo[i].adminID[0] == 0x00) ? (U8 *)"NULL" : pst_TraceList->stTraceInfo[i].adminID,
			pst_TraceList->stTraceInfo[i].usEstimatedTime);
	}
	fprintf(fp, "#E END\n");
	fclose(fp);

	return 0;
}


