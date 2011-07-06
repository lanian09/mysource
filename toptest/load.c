
/*******************************************************************************
							ABLEX IPAS Project

	Author	: tundra
	Section	: IPAS (NTAF) CHSMD

	Revision History	:
 		$Log: chsmd_load.c,v $
 		Revision 1.1.1.1  2006/09/26 02:03:25  ntas
 		Intialize GNGI_WNTAF_SRC Backup
 		
 		Revision 1.1  2002/01/30 17:20:31  swdev4
 		Initial revision

	Copyright (c) ABLEX 2002
*******************************************************************************/


/**A.1*  File Inclusion *******************************************************/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include <sys/sysinfo.h>

#include <ipaf_shm.h>
#include <ipaf_shm2.h>
#include <almstat.h>
#include <utillib.h>


/**B.1* Definition of New Constants *******************************************/

/**C.1*  Declaration of Variables  ********************************************/
extern st_NTAF		*fidb;
extern T_Keepalive	*keepalive;

extern	char	cpu_oldstat;
extern	char	mem_oldstat;
extern	char	disk_oldstat[2];
extern  int		link_oldstat[MAX_NTAF_LINK];

int linkfailcnt[3];


/**D.1*  Declaration of Function  *********************************************/
long percentages(int cnt, int *out, register long *new, register long *old, long *diffs);

int Read_CpuLoad( unsigned long *sys, unsigned long *user, unsigned long *nice, unsigned long *idle, unsigned long *total );

int mem_compute();
int cpu_compute();
int dCheckLoad(int dType, int dLoadNo, char *szCurrload);
extern void Send_AlmMsg( char loctype, char invtype, short invno, char almstatus, char oldalm);

/**D.2*  Declaration of Function  *********************************************/

/*******************************************************************************
 * MAKE CPU OUTPUT STATUS VALUE USING CPU LOAD VALUE
*******************************************************************************/
int cpu_compute()
{
	double		fVal;			
	char		szLoad[6];

	unsigned long total, user, sys, idle, nice;

    Read_CpuLoad( &sys, &user, &nice, &idle, &total );

   	dAppLog(LOG_INFO," USER [%6.2f] SYSTEM[%6.2f] NICE[%6.2f] IDLE[%6.2f]",
    (double)user/(double)total*100,
    (double)sys/(double)total*100,
    (double)nice/(double)total*100,
    (double)idle/(double)total*100);

	sprintf( szLoad, "%6.2f", 100.0 - (double)idle/(double)total*100 );
	fVal = 100.0 - (double)idle/(double)total*100;

	//dAppLog(LOG_INFO," LOAD [%f]",fVal);
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

	//dCheckLoad(1, 0, szLoad);

	//dAppLog(LOG_DEBUG,"[CPU] [%lld] [%lld]", fidb->cpusts.llCur, fidb->cpusts.lMax );

	//dAppLog(LOG_INFO,"CPU_COMPUTE  MPCPU = [%s] TOT/IDL[%d][%d]  OLDSTAT = [%x]", szLoad, total, idle, cpu_oldstat);

	return 1;
}

/*******************************************************************************
 * COMPUTE MEMORY STATUS 
*******************************************************************************/
#if 0
int mem_compute()
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
        dAppLog(LOG_DEBUG,"FILE OPEN ERROR [%s]\n", szPath);
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
		dAppLog(LOG_CRI,"FAILED IN mem_compute() :%d",dRet);
		return dRet;
	}

	fidb->memsts.lMax = dTotal;		
	fidb->memsts.llCur = dTotal - dFree - dBuff - dCach;

	sprintf(szLoad,"%6.2f", (((double)fidb->memsts.llCur)*100.0)/((double)fidb->memsts.lMax));
	dCheckLoad(2, 0, szLoad);
#ifdef DEBUG
	dAppLog(LOG_DEBUG,"MEM_COMPUTE   TOT[%d]/CUR[%d]", dTotal, fidb->memsts.llCur);
#endif

	return 0;
}
#endif


/*******************************************************************************
 * GET CPU ROAD VLAUE FROM /pro/stat FILE
*******************************************************************************/
int Read_CpuLoad( unsigned long *system, unsigned long *user, unsigned long *nice, unsigned long *idle, unsigned long *total )
{
    char path[20] = "/proc/stat" ;
    char buff[1024];
	int  cpustate[4];
	static long oldstate[4];
	static long newstate[4];
	static long diffstate[4];

    FILE *fp;

    fp = fopen( path, "r" );

    if( fp == NULL )
    {
        dAppLog(LOG_DEBUG,"FILE OPEN ERROR [%s]\n", path);
        return -1;
    }

    memset( buff, 0x00, sizeof(char)*1024);

    fgets( buff, 1024, fp );

    fclose(fp);

    sscanf( buff, "%s %lu %lu %lu %lu", path, &newstate[0], &newstate[1], &newstate[2], &newstate[3] );


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
long percentages(int cnt, int *out, register long *new, register long *old, long *diffs)
{
    register int i;
    register long change;
    register long total_change;
    register long *dp;
    long half_total;

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
        	change = (int)
        	((unsigned long)*new-(unsigned long)*old);
    	}
	dAppLog(LOG_INFO,"[%d]change[%lu] new[%lu] old[%lu] total[%lu]",i+1,change, *new, *old,total_change);
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
char res[100];
int len=0;
    for (i = 0; i < cnt; i++)
    {
	memset(res,0x00,100);
	sprintf(res,"[%d]out[%lu]",i+1,*out);
	len = strlen(res);
	
    	*out++ = (int)((*diffs++ * 1000 + half_total) / total_change);

	sprintf(&res[len]," diff[%lu] half[%lu] total[%lu]", *diffs, half_total,total_change);

	dAppLog(LOG_INFO,"%s",res);
	
    }

    /* return the total in case the caller wants to use it */
    return(total_change);
}


/*******************************************************************************
 * CHECK LOAD OF CPU, MEMORY, DISK
*******************************************************************************/
int dCheckLoad(int dType, int dLoadNo, char *szCurrload)
{
	unsigned long	currload;
	char			currstat;
	unsigned long	critical_level=0;
	unsigned long	major_level=0;
	unsigned long	minor_level=0;

	currload = atol(szCurrload);

	if(dType == 1)
	{
		/***********************************************************************
		 * CASE : CPU
		***********************************************************************/
		critical_level = keepalive->cpu.critical;
		major_level = keepalive->cpu.major;
		minor_level = keepalive->cpu.minor;
	}
	else if(dType == 2)
	{
		/***********************************************************************
		 * CASE : MEMORY 
		***********************************************************************/
		critical_level = keepalive->mem.critical;
		major_level = keepalive->mem.major;
		minor_level = keepalive->mem.minor;
	}
	else if(dType == 3)
	{
		/***********************************************************************
		 * CASE : DISK
		***********************************************************************/
		critical_level = keepalive->disk.critical;
		major_level = keepalive->disk.major;
		minor_level = keepalive->disk.minor;
	}
	
	if( critical_level == 0 && major_level == 0 && minor_level == 0)
	{
        currstat = 0x03; /* NORMAL */
	}
	else if(currload >= critical_level)
	{
		currstat = 0x06; /* CRITICAL */
	}
	else if(currload >= major_level && currload < critical_level)
	{   
        currstat = 0x05; /* MAJOR */
    }
    else if(currload >= minor_level && currload < major_level)
    {   
        currstat = 0x04; /* MINOR */
    }
    else
    {   
        currstat = 0x03; /* NORMAL */
    }

	/***************************************************************************
	 * CASE : CPU
	***************************************************************************/
	if(dType == 1)
	{
		if(cpu_oldstat != currstat)
		{
			if( currstat == NORMAL )
			{
				fidb->cpu = NORMAL;
				dAppLog(LOG_INFO,"CPU ALARM RELEASE [%x]", currstat );
			}
			else 
			{
				fidb->cpu = currstat;
#ifdef DEBUG
				dAppLog(LOG_INFO,"CPU ALARM INVOKE [%x]", currstat );
#endif
			}

			Send_AlmMsg( LOCTYPE_LOAD, INVTYPE_CPU, 0, currstat, cpu_oldstat );
		}

		cpu_oldstat = currstat;

	}

	/***************************************************************************
	 * CASE : MEMORY
	***************************************************************************/
	if(dType == 2)
	{
		if(mem_oldstat != currstat)
		{
			if( currstat == NORMAL )
			{

				fidb->mem = NORMAL;
#ifdef DEBUG
			dAppLog(LOG_INFO,"MEM ALARM RELEASE [%x]", currstat );
#endif
			}
			else 
			{
				fidb->mem = currstat;
#ifdef DEBUG
			dAppLog(LOG_INFO,"MEM ALARM INVOKE [%x]", currstat );
#endif
			}

			Send_AlmMsg( LOCTYPE_LOAD, INVTYPE_MEMORY, 0, currstat, mem_oldstat );
		}

		mem_oldstat = currstat;
	}

	/***************************************************************************
	 * CASE : DISK
	***************************************************************************/
	if(dType == 3)
	{
		if(disk_oldstat[dLoadNo] != currstat)
		{
			if( currstat == NORMAL )
			{
				fidb->disk = NORMAL;
#ifdef DEBUG
			dAppLog(LOG_INFO,"DISK ALARM RELEASE [%x] DISK[%d]", currstat, dLoadNo );
#endif
			}
			else 
			{
				fidb->disk = currstat;
#ifdef DEBUG
			dAppLog(LOG_INFO,"DISK ALARM INVOKE [%x] DISK[%d]", currstat, dLoadNo );
#endif
			}

			Send_AlmMsg( LOCTYPE_LOAD, INVTYPE_DISK, dLoadNo, currstat, disk_oldstat[dLoadNo] );
		}

		disk_oldstat[dLoadNo] = currstat;

	}

	return 0;
}


/*******************************************************************************

*******************************************************************************/
#if 0
int dLinkStatus( int dPath )
{
	FILE		*fp;
	int			dRet;
	char		szBuffer[32];
	char		szPath[128];

	sprintf(szPath, "/proc/net/PRO_LAN_Adapters/eth%d/Link", dPath);

	fp = fopen(szPath, "r");
    if(fp == NULL)
    {
#ifdef DEBUG
		dAppLog(LOG_DEBUG,"MAIN  FILE OPEN ERROR ETH[%d]", dPath);
#endif
		return -1;
    }

    if(fgets(szBuffer, 1024, fp) == NULL)
    {

    	fclose(fp);
#ifdef DEBUG
	    dAppLog(LOG_DEBUG,"MAIN  FGETS FUNCTION FAIL");
#endif
	    return -1;
    }

    fclose(fp);

    szBuffer[strlen(szBuffer)-1] = 0x00;

    if(strcmp(szBuffer, "up") == 0)
    {
    	dRet = 3;
    }
    else
    {
        dRet = 6;
    }

    return dRet;
}


/*******************************************************************************
 *
*******************************************************************************/
int dLinkCheck()
{
	int		i;
	int		dRet;


	for( i=0; i<MAX_NTAF_LINK; i++)
	{
		dRet = dLinkStatus(i);
		if( dRet < 0 )
		{
			//dAppLog(LOG_DEBUG,"MAIN  DREADSTATUS FUNCTION ERROR ETH[%d]", i);
		}

		if( dRet == 6 )
		{
			linkfailcnt[i]++;

			if( i == 0 )
			{
				dAppLog(LOG_DEBUG,"ETH0_STATUS DOWN");
			}
			else
			{
				if( linkfailcnt[i] >= 2 )
				{
					SetFIDBValue( &fidb->link[i-1], 0x06 );
					dAppLog(LOG_DEBUG,"ETH%d STATUS IS DOWN",i);

					if( link_oldstat[i-1] != dRet && fidb->link[i-1] < MASK_VALUE )           
					{
						Send_AlmMsg( LOCTYPE_PHSC, INVTYPE_ETH_INF, i-1 , 0x06, link_oldstat[i-1] );
					}                                    
					SetFIDBValue( &link_oldstat[i-1], dRet );
					SetFIDBValue( &fidb->link[i-1], dRet );
				}

			}
		}
		else if( dRet == 3 )
		{
			linkfailcnt[i] = 0;

			if( i== 0 )
			{
				dAppLog(LOG_DEBUG,"ETH0_STATUS UP");
			}
			else
			{
				SetFIDBValue( &fidb->link[i-1], 0x03 );
				dAppLog(LOG_DEBUG,"ETH%d STATUS IS UP",i);  
				                                    
				                                    
				if( link_oldstat[i-1] != dRet )          
				{
					if( link_oldstat[i-1] != 0x00 && fidb->link[i-1] < MASK_VALUE )
						Send_AlmMsg( LOCTYPE_PHSC, INVTYPE_ETH_INF, i-1 , 0x03, link_oldstat[i-1] );
				}                                   
				                                    
				SetFIDBValue( &link_oldstat[i-1], dRet );
				SetFIDBValue( &fidb->link[i-1], dRet );
			}
		}
	}

	return 1;
}


int dLinkStatus_G2( int dPath )
{
    FILE        *fp;
    int         dRet;
    char        szBuffer[32];
    char        szPath[128];
    char        szName[32];
    char        szValue[32];
    int         dResult;

    sprintf(szPath, "/proc/net/nicinfo/eth%d.info", dPath);

    fp = fopen(szPath, "r");
    if(fp == NULL) {
        dAppLog(LOG_CRI,"MAIN  FILE OPEN ERROR ETH[%d]", dPath);
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

    dAppLog(LOG_DEBUG,"START G2 Link CHECK");

    for( i=0; i<MAX_NTAF_LINK-1; i++) {

        dRet = dLinkStatus_G2(i);
        if( dRet < 0 ) {
            dAppLog(LOG_CRI,"MAIN  DREADSTATUS FUNCTION ERROR ETH[%d]", i);
            continue;
        }

        dAppLog(LOG_DEBUG,"AFTER  I:[%d][%d][%p]", dRet, i, &i );

        if( dRet == CRITICAL ) {
            linkfailcnt[i]++;

            if( i == 0 ) {
                dAppLog(LOG_DEBUG,"ETH0_STATUS DOWN");
            }
            else {
                if( linkfailcnt[i] >= 2 ) {
                    SetFIDBValue( &fidb->link[i-1], 0x06 );
                    dAppLog(LOG_DEBUG,"ETH%d STATUS IS DOWN",i);

                    if( link_oldstat[i-1] != dRet && fidb->link[i-1] < MASK_VALUE ) {
                        Send_AlmMsg( LOCTYPE_PHSC, INVTYPE_ETH_INF, i-1 , 0x06, link_oldstat[i-1] );
                    }

                    SetFIDBValue( &link_oldstat[i-1], dRet );
                }
            }
        }
		else if( dRet == NORMAL ) {
            linkfailcnt[i] = 0;

            if( i== 0 ) {
                dAppLog(LOG_DEBUG,"ETH0_STATUS UP");
            }
            else {
                SetFIDBValue( &fidb->link[i-1], 0x03 );
                dAppLog(LOG_DEBUG,"ETH%d STATUS IS UP",i);

                if( link_oldstat[i-1] != dRet ) {
                    if( link_oldstat[i-1] != 0x00 && fidb->link[i-1] < MASK_VALUE )
                        Send_AlmMsg( LOCTYPE_PHSC, INVTYPE_ETH_INF, i-1 , 0x03, link_oldstat[i-1] );
                }

                SetFIDBValue( &link_oldstat[i-1], dRet );
            }
        }
    }

    return 1;
}
#endif
