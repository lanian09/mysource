/**A.1*  File Inclusion *******************************************************/

/* SYS HEADER */
#include <stdio.h>
#include <errno.h>
#include <string.h>		/* memset, strstr, strlen */
#include <stdlib.h>		/* atol */
/* LIB HEADER */
#include "filedb.h"		/* pst_NTAF, pst_keepalive */
#include "loglib.h"
/* PRO HEADER */
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
#include "almstat.h"	/* INVTYPE* */
/* LOC HEADER */
#include "chsmd_msg.h"	/* Send_AlmMsg(), SetFIDBValue() */
#include "chsmd_load.h"

/**B.1* Definition of New Constants *******************************************/
/**C.1*  Declaration of Variables  ********************************************/
st_linkdev 	stLinkDev[MAX_NTAF_LINK];

int 		linkfailcnt[MAX_NTAF_LINK];
char	    nifo_oldstat;

/**C.2*  Declaration of Variables  ********************************************/
extern pst_NTAF		     fidb;
extern pst_keepalive_taf keepalive;

extern char	cpu_oldstat;
extern char	mem_oldstat;
extern char	disk_oldstat[MAX_NTAF_DISK_COUNT];
extern char	queue_oldstat;
extern char	link_oldstat[MAX_NTAF_LINK];

/**D.1*  Declaration of Function  *********************************************/
/**D.2*  Declaration of Function  *********************************************/

/*******************************************************************************
 * MAKE CPU OUTPUT STATUS VALUE USING CPU LOAD VALUE
*******************************************************************************/
int cpu_compute(void)
{
	double		fVal;
	char		szLoad[6];

	unsigned long long total, user, sys, idle, nice;

    Read_CpuLoad( &sys, &user, &nice, &idle, &total );

   	log_print(LOGN_INFO," USER [%6.2f] SYSTEM[%6.2f] NICE[%6.2f] IDLE[%6.2f]",
    (double)user/(double)total*100,
    (double)sys/(double)total*100,
    (double)nice/(double)total*100,
    (double)idle/(double)total*100);

	sprintf( szLoad, "%6.2f", 100.0 - (double)idle/(double)total*100 );
	fVal = 100.0 - (double)idle/(double)total*100;

	log_print(LOGN_INFO," LOAD [%f]",fVal);
	if( fVal < 0 )
	{
		fidb->cpusts.lMax = 1000;
		fidb->cpusts.llCur = 0;
	}
	else
	{
    	fidb->cpusts.lMax = total;
    	fidb->cpusts.llCur = total - idle;
	}

	dCheckLoad(INVTYPE_CPU, 0, szLoad);

	log_print(LOGN_DEBUG,"[CPU] [%lld] [%lld]", fidb->cpusts.llCur, fidb->cpusts.lMax );

	log_print(LOGN_INFO,"CPU_COMPUTE  MPCPU = [%s] TOT/IDL[%llu][%llu]  OLDSTAT = [%x]", szLoad, total, idle, cpu_oldstat);

	return 1;
}

/*******************************************************************************
 * COMPUTE MEMORY STATUS
*******************************************************************************/
int mem_compute(void)
{
	char	szPath[20] = "/proc/meminfo";
	char	szName[20];
	char	buffer[2048];
	char    szLoad[6];

	int		dTotal, dFree, dBuff, dCach;
	int		dRet;

	FILE *fp;

    fp = fopen( szPath, "r" );

    if( fp == NULL )
    {
#ifdef DEBUG
        log_print(LOGN_DEBUG,"FILE OPEN ERROR [%s]\n", szPath);
#endif
        return -1;
    }

	dRet = 0;
	while( fgets( buffer, 2048, fp ) != NULL )
	{
		if( sscanf( buffer, "%s", szName ) == 1 )
		{
            if( strcasecmp(szName, "MemTotal:") == 0 )
            {

                if( sscanf( buffer, "%*s %d %*s", &dTotal ) != 1 )
                {
					dRet = -1;
					break;
                }
                continue;
            }
            else if( strcasecmp(szName, "MemFree:") == 0 )
            {
                if( sscanf( buffer, "%*s %d %*s", &dFree) != 1 )
                {
					dRet = -2;
					break;
                }
                continue;

            }
            else if( strcasecmp(szName, "Buffers:") == 0 )
            {
                if( sscanf( buffer, "%*s %d %*s", &dBuff) != 1 )
                {
					dRet = -3;
					break;
                }
                continue;

            }
            else if( strcasecmp(szName, "Cached:") == 0 )
            {
                if( sscanf( buffer, "%*s %d %*s", &dCach) != 1 )
                {
					dRet = -4;
					break;
                }
                continue;

            }

		}

	}

    fclose(fp);

	if( dRet < 0 )
	{
		log_print(LOGN_CRI,"FAILED IN mem_compute() :%d",dRet);
		return dRet;
	}

	fidb->memsts.lMax = dTotal;
	fidb->memsts.llCur = dTotal - dFree - dBuff - dCach;

	sprintf(szLoad,"%6.2f", (((double)fidb->memsts.llCur)*100.0)/((double)fidb->memsts.lMax));
	dCheckLoad(INVTYPE_MEMORY, 0, szLoad);
#ifdef DEBUG
	log_print(LOGN_DEBUG,"MEM_COMPUTE   TOT[%d]/CUR[%lld]", dTotal, fidb->memsts.llCur);
#endif

	return 0;
}

/*******************************************************************************
 * COMPUTE QUEUE STATUS
*******************************************************************************/
int queue_compute(void)
{
	long long	llCurVal;
	float		fPercent;
	char		szLoad[6];

	if(fidb->quests.lMax != 0)
	{
		llCurVal = ( (float)fidb->quests.llCur/(float)fidb->quests.lMax)*100.0;
		fPercent = ( (float)fidb->quests.llCur/(float)fidb->quests.lMax)*100.0;

		if(fPercent>100)
		{
			fidb->quests.llCur = fidb->quests.lMax;
			llCurVal = ((float)fidb->quests.llCur/(float)fidb->quests.lMax)*100.0;
			fPercent = ((float)fidb->quests.llCur/(float)fidb->quests.lMax)*100.0;
		}
		sprintf(szLoad, "%6.2f", fPercent);
		dCheckLoad(INVTYPE_QUEUE, 0, szLoad);

		log_print(LOGN_DEBUG, "QUE_COMPUTE   LOAD[%6.2f] CUR[%lld] MAX[%lld]", fPercent, fidb->quests.llCur, fidb->quests.lMax);
	}
	else
		log_print(LOGN_WARN, "QUE_COMPUTE FAIL : lMax ZERO(%lld) llCur=%lld", fidb->quests.lMax, fidb->quests.llCur);

	return 0;
}

/*******************************************************************************
 * COMPUTE NIFO STATUS
*******************************************************************************/
int nifo_compute(void)
{
	long long	llCurVal;
	float		fPercent;
	char		szLoad[6];

	if(fidb->nifosts.lMax != 0)
	{
		llCurVal = ( (float)fidb->nifosts.llCur/(float)fidb->nifosts.lMax)*100.0;
		fPercent = ( (float)fidb->nifosts.llCur/(float)fidb->nifosts.lMax)*100.0;

		if(fPercent>100)
		{
			fidb->nifosts.llCur = fidb->nifosts.lMax;
			llCurVal = ((float)fidb->nifosts.llCur/(float)fidb->nifosts.lMax)*100.0;
			fPercent = ((float)fidb->nifosts.llCur/(float)fidb->nifosts.lMax)*100.0;
		}
		sprintf(szLoad, "%6.2f", fPercent);
		dCheckLoad(INVTYPE_NIFO, 0, szLoad);

		log_print(LOGN_DEBUG, "F=%s:%s.%d: NIFO LOAD[%6.2f] CUR[%lld] MAX[%lld]", __FILE__, __FUNCTION__, __LINE__,
			fPercent, fidb->nifosts.llCur, fidb->nifosts.lMax);
	}
	else
		log_print(LOGN_WARN, "NIFO_COMPUTE FAIL : lMax ZERO(%lld) llCur=%lld", fidb->nifosts.lMax, fidb->nifosts.llCur);

	return 0;
}


/*******************************************************************************
 * GET CPU ROAD VLAUE FROM /pro/stat FILE
*******************************************************************************/
int Read_CpuLoad(unsigned long long *system, unsigned long long *user, unsigned long long *nice, unsigned long long *idle, unsigned long long *total)
{
    char path[20] = "/proc/stat" ;
    char buff[1024];
	long long cpustate[4];
	static long long oldstate[4];
	static long long newstate[4];
	static long long diffstate[4];

    FILE *fp;

    fp = fopen( path, "r" );

    if( fp == NULL )
    {
        log_print(LOGN_DEBUG,"FILE OPEN ERROR [%s]\n", path);
        return -1;
    }

    memset( buff, 0x00, sizeof(char)*1024);

    fgets( buff, 1024, fp );

    fclose(fp);

    sscanf( buff, "%s %llu %llu %llu %llu", path, &newstate[0], &newstate[1], &newstate[2], &newstate[3] );


	/***************************************************************************
	 * CONVERT CPU STATUS VALUE
	***************************************************************************/
	percentages( 4, cpustate, newstate, oldstate, diffstate );

	*user = cpustate[0];
	*nice = cpustate[1];
	*system = cpustate[2];
	*idle = cpustate[3];

    *total = *user + *nice + *system + *idle ;

    return 1;
}


/*******************************************************************************
 * CONVERT STATUS VALUE
*******************************************************************************/
long long percentages(int cnt, long long *out, register long long *new, register long long *old, long long *diffs)
{
    register int i;
    register long long change;
    register long long total_change;
    register long long *dp;
    long long half_total;

    /* initialization */
    total_change = 0;
    dp = diffs;

    /***************************************************************************
	 * calculate changes for each state and the overall change
	***************************************************************************/
    for (i = 0; i < cnt; i++)
    {
    	if ((change = *new - *old) < 0)
    	{
        	/* this only happens when the counter wraps */
        	change = (long long)
        	((long long)*new-(long long)*old);
    	}
    	total_change += (*dp++ = change);
    	*old++ = *new++;
    }

    /***************************************************************************
	 * avoid divide by zero potential
	***************************************************************************/
    if (total_change == 0)
    {
    	total_change = 1;
    }

    /***************************************************************************
	 * calculate percentages based on overall change, rounding up
	***************************************************************************/
    half_total = total_change / 2l;
    for (i = 0; i < cnt; i++)
    {
    	*out++ = (long long)((*diffs++ * 1000 + half_total) / total_change);
    }

    /* return the total in case the caller wants to use it */
    return(total_change);
}

char cDecideCurrentStatus(unsigned long luCurLoad, unsigned long luCriticalValue, unsigned long luMajorValue, unsigned long luMinorValue)
{
	char	cCurStatus;

	if( (luCriticalValue == 0) && (luMajorValue == 0) && (luMinorValue == 0))
        cCurStatus = NORMAL;
	else
	{
		if(luCurLoad < luMinorValue)
			cCurStatus = NORMAL;
		else if(luCurLoad < luMajorValue)
			cCurStatus = MINOR;
		else if(luCurLoad < luCriticalValue)
			cCurStatus = MAJOR;
		else
			cCurStatus = CRITICAL;
	}

	return cCurStatus;
}

/*******************************************************************************
 * CHECK LOAD OF CPU, MEMORY, DISK
*******************************************************************************/
int dCheckLoad(int dType, int dLoadNo, char *sCurLoad)
{
	unsigned long	luCurLoad, luCriticalValue, luMajorValue, luMinorValue;
	char			cCurStatus;

	luCriticalValue	= 0;
	luMajorValue	= 0;
	luMinorValue	= 0;
	luCurLoad		= atol(sCurLoad);

	switch(dType)
	{
		case INVTYPE_CPU:
			luCriticalValue	= keepalive->cpu.critical;
			luMajorValue	= keepalive->cpu.major;
			luMinorValue	= keepalive->cpu.minor;

			cCurStatus = cDecideCurrentStatus(luCurLoad, luCriticalValue, luMajorValue, luMinorValue);

			if(cpu_oldstat != cCurStatus)
			{
				if(cCurStatus == NORMAL)
				{
					fidb->cpu = NORMAL;
					log_print(LOGN_INFO, "CPU ALARM RELEASE [%x]", cCurStatus);
				}
				else
				{
					fidb->cpu = cCurStatus;
#ifdef DEBUG
					log_print(LOGN_INFO, "CPU ALARM INVOKE [%x]", cCurStatus);
#endif
				}
				Send_AlmMsg(LOCTYPE_LOAD, dType, 0, cCurStatus, cpu_oldstat);
			}
			cpu_oldstat = cCurStatus;
			break;
		case INVTYPE_MEMORY:
			luCriticalValue	= keepalive->mem.critical;
			luMajorValue	= keepalive->mem.major;
			luMinorValue	= keepalive->mem.minor;

			cCurStatus = cDecideCurrentStatus(luCurLoad, luCriticalValue, luMajorValue, luMinorValue);
			if(mem_oldstat != cCurStatus)
			{
				if(cCurStatus == NORMAL)
				{
					fidb->mem = NORMAL;
#ifdef DEBUG
					log_print(LOGN_INFO, "MEM ALARM RELEASE [%x]", cCurStatus);
#endif
				}
				else
				{
					fidb->mem = cCurStatus;
#ifdef DEBUG
					log_print(LOGN_INFO, "MEM ALARM INVOKE [%x]", cCurStatus);
#endif
				}
				Send_AlmMsg(LOCTYPE_LOAD, dType, 0, cCurStatus, mem_oldstat);
			}
			mem_oldstat = cCurStatus;
			break;
		case INVTYPE_DISK:
			luCriticalValue	= keepalive->disk.critical;
			luMajorValue	= keepalive->disk.major;
			luMinorValue	= keepalive->disk.minor;

			cCurStatus = cDecideCurrentStatus(luCurLoad, luCriticalValue, luMajorValue, luMinorValue);
			if((char)disk_oldstat[dLoadNo] != cCurStatus)
			{
				if(cCurStatus == NORMAL)
				{
					fidb->disk[dLoadNo] = NORMAL;
#ifdef DEBUG
					log_print(LOGN_INFO, "DISK ALARM RELEASE [%x] DISK[%d]", cCurStatus, dLoadNo);
#endif
				}
				else
				{
					fidb->disk[dLoadNo] = cCurStatus;
#ifdef DEBUG
					log_print(LOGN_INFO, "DISK ALARM INVOKE [%x] DISK[%d]", cCurStatus, dLoadNo);
#endif
				}
				Send_AlmMsg(LOCTYPE_LOAD, dType, dLoadNo, cCurStatus, (char)disk_oldstat[dLoadNo]);
			}
			disk_oldstat[dLoadNo] = cCurStatus;
			break;
		case INVTYPE_QUEUE:
			luCriticalValue	= keepalive->queue.critical;
			luMajorValue	= keepalive->queue.major;
			luMinorValue	= keepalive->queue.minor;

			cCurStatus = cDecideCurrentStatus(luCurLoad, luCriticalValue, luMajorValue, luMinorValue);
			if(queue_oldstat != cCurStatus)
			{
				if(cCurStatus == NORMAL)
				{
					fidb->queue = NORMAL;
#ifdef DEBUG
					log_print(LOGN_INFO, "QUEUE ALARM RELEASE [%x]", cCurStatus);
#endif
				}
				else
				{
					fidb->queue = cCurStatus;
#ifdef DEBUG
					log_print(LOGN_INFO, "QUEUE ALARM INVOKE [%x]", cCurStatus);
#endif
				}
				Send_AlmMsg(LOCTYPE_LOAD, dType, dLoadNo, cCurStatus, queue_oldstat);
			}
			queue_oldstat = cCurStatus;
			break;
		case INVTYPE_NIFO:
			luCriticalValue	= keepalive->nifo.critical;
			luMajorValue	= keepalive->nifo.major;
			luMinorValue	= keepalive->nifo.minor;

			cCurStatus = cDecideCurrentStatus(luCurLoad, luCriticalValue, luMajorValue, luMinorValue);
			if(nifo_oldstat != cCurStatus)
			{
				if(cCurStatus == NORMAL)
				{
					fidb->nifo = NORMAL;
#ifdef DEBUG
					log_print(LOGN_INFO, "NIFO ALARM RELEASE [%x]", cCurStatus);
#endif
				}
				else
				{
					fidb->nifo = cCurStatus;
#ifdef DEBUG
					log_print(LOGN_INFO, "NIFO ALARM INVOKE [%x]", cCurStatus);
#endif
				}
				Send_AlmMsg(LOCTYPE_LOAD, dType, dLoadNo, cCurStatus, nifo_oldstat);
			}
			nifo_oldstat = cCurStatus;
			break;
		default:
	        log_print(LOGN_CRI, "F=%s:%s.%d: dType[%d] dLoadNo[%d] sCurLoad[%s]", __FILE__, __FUNCTION__, __LINE__,
				dType, dLoadNo, sCurLoad);
			return -1;
	}

	return 0;
}


/*******************************************************************************

*******************************************************************************/
int dLinkStatus(char *sNetStr)
{
	FILE		*fp = NULL;
	char		*sPnt, sBuf[256], sCommand[1024];
	int			dRet;
	size_t		szStrLen;

	dRet = 0;
	memset(sCommand, 0x00, 1024);
	sprintf(sCommand, "/sbin/ethtool %s", sNetStr);

	if( (fp = popen((const char*)sCommand, "r")) == NULL)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN popen(sCommand[%s]) errno[%d-%s]", 
			__FILE__, __FUNCTION__, __LINE__, sCommand, errno, strerror(errno));
		return -1;
	}

	while(fgets(sBuf, 256, fp) != NULL)
	{
		if( (sPnt = strstr(sBuf, "Speed")) != NULL)
		{
            if( (strncmp(sPnt, "Speed: 1000Mb/s", strlen("Speed: 1000Mb/s"))==0) ||
				(strncmp(sPnt, "Speed: 100Mb/s", strlen("Speed: 100Mb/s"))==0))
                dRet += 1;
			else
				log_print(LOGN_INFO, "F=%s:%s.%d: Speed[%s] [%s]", __FILE__, __FUNCTION__, __LINE__, sPnt, sNetStr);
		}

        if( (sPnt = strstr(sBuf, "Duplex")) != NULL)
			dRet += 10;
#if 0
			szStrLen = strlen("Duplex: Full");
            if(strncmp(sPnt, "Duplex: Full", szStrLen) == 0)
                dRet += 10;
			else
				log_print(LOGN_INFO, "Duplex[%s] [%s]", sPnt, sNetStr);
#endif

        if( (sPnt = strstr(sBuf, "Link")) != NULL)
        {
			szStrLen = strlen("Link detected: yes");
            if(strncmp(sPnt, "Link detected: yes", szStrLen) == 0)
                dRet += 100;
			else
				log_print(LOGN_INFO, "F=%s:%s.%d: Link[%s] [%s]", __FILE__, __FUNCTION__, __LINE__, sPnt, sNetStr);
        }
	}
	log_print(LOGN_INFO, "F=%s:%s.%d: LINK RESULT [%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
	pclose(fp);

	return dRet;
}


/*******************************************************************************
 *
*******************************************************************************/
int dLinkCheck(void)
{
	short	i;
	int		dRet;
	char	sEthStr[16];

	for(i = 0; i < MAX_NTAF_LINK; i++)
	{
		if(fidb->link[i] == NOT_EQUIP)
		{
			log_print(LOGN_DEBUG, "F=%s:%s.%d: ETH%d IS NOT EQUIP", __FILE__, __FUNCTION__, __LINE__, i);
			continue;
		}

		if( !strlen(stLinkDev[i].szDevName) ){
			continue;
		}

		sprintf(sEthStr, "%s", stLinkDev[i].szDevName);
		if( (dRet = dLinkStatus(sEthStr)) < 0)
		{
			log_print(LOGN_DEBUG, "F=%s:%s.%d: ERROR IN dLinkStatus(eth%d)", __FILE__, __FUNCTION__, __LINE__, i);
			continue;
		}

		if(dRet == 111)
		{
			log_print(LOGN_DEBUG, "F=%s:%s.%d: ETH%d STATUS IS UP", __FILE__, __FUNCTION__, __LINE__, i);
			linkfailcnt[i] = 0;
			SetFIDBValue(&fidb->link[i], NORMAL);

			if( (link_oldstat[i]!=NORMAL)&&(fidb->link[i]<MASK))
				Send_AlmMsg(LOCTYPE_PHSC, INVTYPE_LINK, i , (char)NORMAL, (char)link_oldstat[i]);

			SetFIDBValue((unsigned char*)&link_oldstat[i], NORMAL);
		}
		else
		{
			log_print(LOGN_DEBUG, "F=%s:%s.%d: ETH%d STATUS IS DOWN", __FILE__, __FUNCTION__, __LINE__, i);

			linkfailcnt[i]++;
			if(linkfailcnt[i] >= 2)
			{
				SetFIDBValue(&fidb->link[i], CRITICAL);

				if( (link_oldstat[i]!=CRITICAL) && (fidb->link[i]<MASK))
					Send_AlmMsg(LOCTYPE_PHSC, INVTYPE_LINK, i , (char)CRITICAL, (char)link_oldstat[i]);

				SetFIDBValue((unsigned char*)&link_oldstat[i-1], CRITICAL);
			}
		}
	}

	return 0;
}


int dLinkStatus_G2( int dPath )
{
    FILE        *fp;
    int         dRet;
    char        szBuffer[32], szPath[128], szName[32], szValue[32];

	dRet	= 0;

    sprintf(szPath, "/proc/net/nicinfo/eth%d.info", dPath);

    fp = fopen(szPath, "r");
    if(fp == NULL) {
        log_print(LOGN_CRI,"MAIN  FILE OPEN ERROR ETH[%d]", dPath);
        return -1;
    }

    while( fgets( szBuffer, 1024, fp ) != NULL ) {
        if( sscanf( szBuffer, "%s %s", szName, szValue ) == 2 ) {
            if( strcmp( szName, "Link" ) == 0 ) {
                if( strcmp( szValue, "up" ) == 0 )
                    dRet = 3;
                else
                    dRet = 6;

                break;
            }
        }
    }

    fclose(fp);

    if( dRet == 3 || dRet == 6 ) {
        return dRet;
    }
    else
        return -1;
}


int dLinkCheck_G2()
{
    int     i;
    int     dRet;

    log_print(LOGN_DEBUG,"START G2 Link CHECK");

    for( i=0; i<MAX_NTAF_LINK-1; i++) {

        dRet = dLinkStatus_G2(i);
        if( dRet < 0 ) {
            log_print(LOGN_CRI,"MAIN  DREADSTATUS FUNCTION ERROR ETH[%d]", i);
            continue;
        }

        log_print(LOGN_DEBUG,"AFTER  I:[%d][%d][%p]", dRet, i, &i );

        if( dRet == CRITICAL ) {
            linkfailcnt[i]++;

            if( i == 0 ) {
                log_print(LOGN_DEBUG,"ETH0_STATUS DOWN");
            }
            else {
                if( linkfailcnt[i] >= 2 ) {
                    SetFIDBValue( &fidb->link[i-1], 0x06 );
                    log_print(LOGN_DEBUG,"ETH%d STATUS IS DOWN",i);

                    if( link_oldstat[i-1] != dRet && fidb->link[i-1] < MASK ) {
                        Send_AlmMsg( LOCTYPE_PHSC, INVTYPE_LINK, i-1 , 0x06, link_oldstat[i-1] );
                    }

                    SetFIDBValue( (unsigned char*)&link_oldstat[i-1], dRet );
                }
            }
        }
		else if( dRet == NORMAL ) {
            linkfailcnt[i] = 0;

            if( i== 0 ) {
                log_print(LOGN_DEBUG,"ETH0_STATUS UP");
            }
            else {
                SetFIDBValue( &fidb->link[i-1], 0x03 );
                log_print(LOGN_DEBUG,"ETH%d STATUS IS UP",i);

                if( link_oldstat[i-1] != dRet ) {
                    if( link_oldstat[i-1] != 0x00 && fidb->link[i-1] < MASK )
                        Send_AlmMsg( LOCTYPE_PHSC, INVTYPE_LINK, i-1 , 0x03, link_oldstat[i-1] );
                }

                SetFIDBValue( (unsigned char*)&link_oldstat[i-1], dRet );
            }
        }
    }

    return 1;
}

/*******************************************************************************

	Revision History :

 		$Log: chsmd_load.c,v $
 		Revision 1.3  2011/09/05 05:05:49  dcham
 		*** empty log message ***
 		
 		Revision 1.2  2011/09/01 01:58:07  uamyd
 		log_print format checket applied
 		
 		Revision 1.1  2011/08/29 09:57:07  dcham
 		*** empty log message ***
 		
 		Revision 1.3  2011/08/05 08:45:07  uamyd
 		refactoring
 		
 		Revision 1.2  2011/07/28 01:47:27  uamyd
 		CHSMD TAF modified
 		
 		Revision 1.1  2011/07/27 10:01:26  uamyd
 		TAF CHSMD added
 		
 		Revision 1.23  2011/06/29 09:42:47  uamyd
 		source refactoring, hw alarm report corrected
 		
 		Revision 1.22  2011/04/18 10:01:20  uamyd
 		link check module modified
 		
 		Revision 1.21  2011/01/11 04:09:12  uamyd
 		modified
 		
 		Revision 1.1.1.1  2010/08/23 01:13:00  uamyd
 		DQMS With TOTMON, 2nd-import
 		
 		Revision 1.20  2009/10/21 08:39:57  pkg
 		실시간 traffic 감시 항목 추가에 따른 명령어 처리 및 메시지 처리 수정
 		
 		Revision 1.19  2009/09/13 14:17:19  pkg
 		no message

 		Revision 1.18  2009/09/13 13:43:46  pkg
 		no message

 		Revision 1.17  2009/09/13 11:10:21  pkg
 		no message

 		Revision 1.16  2009/09/13 11:09:14  pkg
 		no message

 		Revision 1.15  2009/09/13 11:01:25  pkg
 		NIFO 감시 추가

 		Revision 1.14  2009/08/25 14:03:04  pkg
 		CHSMD의 dLinkStatus함수 fflush 삭제

 		Revision 1.13  2009/08/14 06:30:25  dqms
 		no message

 		Revision 1.12  2009/08/13 07:47:12  dqms
 		no message

 		Revision 1.11  2009/07/08 06:45:08  hjpark
 		no message

 		Revision 1.10  2009/07/02 04:43:27  dqms
 		*** empty log message ***

 		Revision 1.9  2009/07/02 04:39:58  hjpark
 		no message

 		Revision 1.8  2009/07/02 04:34:09  hjpark
 		no message

 		Revision 1.7  2009/07/02 03:40:29  hjpark
 		no message

 		Revision 1.6  2009/07/02 03:39:16  hjpark
 		no message

 		Revision 1.5  2009/07/02 03:37:28  hjpark
 		no message

 		Revision 1.4  2009/07/02 03:32:51  hjpark
 		no message

 		Revision 1.3  2009/06/10 21:25:17  jsyoon
 		*** empty log message ***

 		Revision 1.2  2009/06/05 05:30:16  jsyoon
 		*** empty log message ***

 		Revision 1.1.1.1  2009/05/26 02:14:42  dqms
 		Init TAF_RPPI

 		Revision 1.4  2009-05-21 07:36:53  astone
 		no message

 		Revision 1.3  2009-05-21 06:47:12  astone
 		no message

 		Revision 1.2  2009-05-21 06:44:58  hjpark
 		no message

 		Revision 1.1.1.1  2009-05-19 10:34:23  hjpark
 		DQMS
 		 -Writer: Han-jin Park
 		 -Date: 2009.05.19

 		Revision 1.9  2008/11/19 02:48:44  hjpark
 		*** empty log message ***

 		Revision 1.8  2008/07/28 06:32:50  hjpark
 		CPU 로드 정보 100 % 오류 수정
 		 - Date: 2008.07.28
 		 - Writer: Han-jin Park

 		Revision 1.7  2008/07/28 06:31:28  hjpark
 		cpu 로드 정보 100 % 오류 수정
 		 - Date: 2008.07.28
 		 - Writer: Han-jin Park

 		Revision 1.6  2008/07/28 06:19:44  hjpark
 		no message

 		Revision 1.5  2008/05/06 06:41:36  uamyd
 		20080506
 		인수시험
 		적용소스

 		Revision 1.4  2008/04/29 02:33:17  uamyd
 		added Queue check module

 		Revision 1.3  2008/04/04 11:47:37  uamyd
 		modified check-cpu/mem/disk

 		Revision 1.2  2008/01/03 11:13:46  dark264sh
 		modify Makefile options (-g3 -Wall), modify warning code

 		Revision 1.1.1.1  2007/12/27 09:04:36  leich
 		WNTAS_ADD

 		Revision 1.1.1.1  2007/10/31 05:12:12  uamyd
 		WNTAS so lated initialized

 		Revision 1.1  2002/01/30 17:20:31  swdev4
 		Initial revision

*******************************************************************************/
