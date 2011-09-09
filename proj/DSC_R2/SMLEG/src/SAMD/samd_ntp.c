#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "samd.h"

/*============ extern ==============*/
/* shared memory address pointer */
extern SFM_SysCommMsgType *loc_sadb;
extern int		trcLogId, trcErrLogId;
extern char		trcBuf[4096];
extern char		mySysName[COMM_MAX_NAME_LEN];

/*============ global ==============*/
st_NTP_STS   stNTPSTS;		/* buffer for new NTP values */
st_NTP_STS   oldNTPSTS;		/* buffer for current NTP values */
unsigned char ucRmtNTPSvrStatus; /* Remote NTP Server's status */
unsigned char ucLclNTPDmnStatus; /* Local NTP Deamon's status */
unsigned int uiNTPDEADCNT = 0;

int CheckNTPStS()
{
	char szCommand[126];
	char szBuf[512];
	int i=0;
	int  readcnt=0;
	FILE *fp=NULL;
	int retlen=0;
	int  idx=0;
    static int daemonDown = 0;
    int     channelCount = 0;

	char  szRetMsg[10][1024];
	char  value1[10][64];
	char  value2[10][64];
	char  value3[10][64];
	char  value4[10][64];
	char  value5[10][64];
	char  value6[10][64];
	char  value7[10][64];
	char  value8[10][64];
	char  value9[10][64];
	char  value10[10][64];

	char  outfile[80];
	memset(szRetMsg, 0x00, sizeof(szRetMsg) );

	retlen = 0;

    /*
    env = getenv(IV_HOME);
    if(env)
	*/
#ifdef _FILE_OPEN_ 
	sprintf(outfile, "/tmp/ntp_check.txt");
	sprintf(szCommand, "/usr/sbin/ntpq -p > %s",outfile);
	my_system (szCommand);
	fp = fopen(outfile, "r");
#else
	sprintf(szCommand, "/usr/sbin/ntpq -p");
	fp = popen( szCommand, "r");
#endif

	if (fp == NULL) {
		sprintf(trcBuf, "[%s] f(p)open ntpq error!\n", __FUNCTION__);
		trclib_writeLogErr (FL,trcBuf);
		return 0;
	}

   	i = 0;
   	while( fgets(szBuf, sizeof(szBuf), fp ) != NULL )
   	{
       	if( i<2 )
       	{
           	i++;
           	continue;
       	}
       	sprintf( szRetMsg[i-2], "%s", szBuf);
       	i++;
   	}

#ifdef _FILE_OPEN_
 	if(fp) fclose(fp);
#else
 	if(fp) pclose(fp);
#endif

    channelCount = i;
	memset( &stNTPSTS, 0x00, sizeof(stNTPSTS));
   	if( channelCount == 0 )
   	{
       	//sprintf( &szRetMsg[0][0], "ntpq: read: Connection refused");
		sprintf(trcBuf, "[CheckNTPStS] ntpq error - %d.%s\n", errno, strerror(errno));
		trclib_writeLogErr (FL,trcBuf);
        daemonDown++;
   	}
	else
	{
		idx = 0;

		readcnt = channelCount-2;

		for( i=0; i<readcnt; i++)
		{
			sscanf( szRetMsg[i], "%s %s %s %s %s %s %s %s %s %s",
				value1[i], value2[i], value3[i], value4[i],
				value5[i], value6[i], value7[i], value8[i],
				value9[i], value10[i] ) ;
		}

		for( i=0 ; i < readcnt ; i++ )
		{
			if( value1[i][0] == '*' || value1[i][0] == '+' )
			{
				stNTPSTS.stNTP[i].ucSymF = value1[i][0];
				sprintf( stNTPSTS.stNTP[i].szIP, "%s", &value1[i][1] );
			}
			else
			{
				stNTPSTS.stNTP[i].ucSymF = 0x00;
				sprintf(stNTPSTS.stNTP[i].szIP, "%s", &value1[i][0]);
			}

			stNTPSTS.stNTP[i].usST = atoi( value3[i] );
			stNTPSTS.stNTP[i].ucTF = value4[i][0];
			stNTPSTS.stNTP[i].usWhen = atoi( value5[i] );
			stNTPSTS.stNTP[i].usPoll = atoi( value6[i] );
			stNTPSTS.stNTP[i].usReach = atoi( value7[i] );

			sscanf( value8[i], "%f", &stNTPSTS.stNTP[i].fDelay );
			sscanf( value9[i], "%f", &stNTPSTS.stNTP[i].fOffset );
			sscanf( value10[i], "%f", &stNTPSTS.stNTP[i].fJitter );
		}

		stNTPSTS.dNtpCnt = readcnt ;

#ifdef DEBUG
		for( i=0; i<stNTPSTS.dNtpCnt ; i++ )
		{
			fprintf(stderr, "IDX[%2d] SYMBOL : %c\t", i, stNTPSTS.stNTP[i].ucSymF);
			fprintf(stderr, "IDX[%2d] REMOTE : %s\t", i, stNTPSTS.stNTP[i].szIP);
			fprintf(stderr, "IDX[%2d] ST     : %d\t", i, stNTPSTS.stNTP[i].usST);
			fprintf(stderr, "IDX[%2d] T      : %c\t", i, stNTPSTS.stNTP[i].ucTF);
			fprintf(stderr, "IDX[%2d] WHEN   : %d\n", i, stNTPSTS.stNTP[i].usWhen);
			fprintf(stderr, "IDX[%2d] POLL   : %d\t", i, stNTPSTS.stNTP[i].usPoll);
			fprintf(stderr, "IDX[%2d] REACH  : %d\t", i, stNTPSTS.stNTP[i].usReach);
			fprintf(stderr, "IDX[%2d] DELAY  : %.3f\t", i, stNTPSTS.stNTP[i].fDelay);
			fprintf(stderr, "IDX[%2d] OFFSET : %.3f\t", i, stNTPSTS.stNTP[i].fOffset);
			fprintf(stderr, "IDX[%2d] JITTER : %.3f\n", i, stNTPSTS.stNTP[i].fJitter);
		}
#endif
	}
    if((daemonDown > NTP_DAEMON_DOWN_COUNT && channelCount == 0) ||
        channelCount > 0){
        CompareNTPSTS( stNTPSTS, &oldNTPSTS );
        daemonDown = 0;
    }


	return 0;
}

int CompareNTPSTS(st_NTP_STS stCurr, pst_NTP_STS pstOld)
{
	int i=0;
	unsigned short usMaxVal;
	unsigned char ucTStatus=NTP_CRITICAL;
	int NntpCnt=0;

	if( stCurr.dNtpCnt == 0 ) /* Local ntpd(NTP Daemon) down */
	{
		if( pstOld->dNtpCnt == 0 )
		{
			return 0;
		}
		else
		{
			for(i=0; i<pstOld->dNtpCnt; i++ )
			{
				if( pstOld->stNTP[i].ucSymF == '*')
				{
					sprintf(trcBuf, "NTP SVR DOWN idx[%d] %s\n", i, pstOld->stNTP[i].szIP);
					trclib_writeLogErr(FL, trcBuf);
				}
				else if( pstOld->stNTP[i].ucSymF == '+' )
				{
					sprintf(trcBuf, "NORMAL NTP SVR DOWN idx[%d] %s\n", i, pstOld->stNTP[i].szIP);
					trclib_writeLogErr(FL, trcBuf);
				}
				else
				{
					sprintf(trcBuf, "NORMAL NTP SVR DOWN idx[%d] %s\n", i, pstOld->stNTP[i].szIP);
					trclib_writeLogErr(FL, trcBuf);
				}
			}

			if( ucLclNTPDmnStatus != NTP_CRITICAL )
			{
				//sprintf(trcBuf, "#### NTP DAEMON[/sbin/init.d/xntpd] DOWN\n");
				sprintf(trcBuf, "#### NTP DAEMON[/etc/init.d/xntpd] DOWN\n");
				trclib_writeLogErr(FL, trcBuf);

				if( ucLclNTPDmnStatus == NTP_NORMAL )
				{
					// Local ntpd(NTP Daemon) shut down
			        loc_sadb->hwNTP[0] = NTP_CRITICAL; /* set critical alarm */
				}
				ucLclNTPDmnStatus = NTP_CRITICAL;

				// Remote NTP Server connection error
				if( ucRmtNTPSvrStatus == NTP_NORMAL )
				{
					sprintf(trcBuf, "#### NTP CHANNEL NORMAL->ABNORMAL\n");
					trclib_writeLogErr(FL, trcBuf);

					// CHANNEL ALARM INVOKE
			        loc_sadb->hwNTP[1] = NTP_CRITICAL ;

				}
				ucRmtNTPSvrStatus = NTP_CRITICAL;
			}
			memcpy( pstOld, &stCurr, sizeof(st_NTP_STS));
		}

	}
	else if( pstOld->dNtpCnt == 0 )
	{
		memcpy( pstOld, &stCurr, sizeof(st_NTP_STS));
		for( i=0; i<pstOld->dNtpCnt; i++ )
		{

			if( pstOld->stNTP[i].ucSymF == '*')
			{
				sprintf(trcBuf, " NTP SVR ALIVE idx[%d] %s\n", i, pstOld->stNTP[i].szIP);
				trclib_writeLogErr(FL, trcBuf);
			}
			else if( pstOld->stNTP[i].ucSymF == '+' )
			{
				sprintf(trcBuf, " NTP SVR ALIVE idx[%d] %s\n", i, pstOld->stNTP[i].szIP);
				trclib_writeLogErr(FL, trcBuf);
			}
			else
			{
				sprintf(trcBuf, "NTP SVR DEAD idx[%d] %s\n", i, pstOld->stNTP[i].szIP);
				trclib_writeLogErr(FL, trcBuf);
			}
		}

		if( ucLclNTPDmnStatus != NTP_NORMAL )
		{
			//sprintf(trcBuf, "#### NTP DAEMON[/sbin/init.d/xntpd] START\n");
			sprintf(trcBuf, "#### NTP DAEMON[/etc/init.d/xntpd] START\n");
			trclib_writeLogErr(FL, trcBuf);

			if( ucLclNTPDmnStatus == NTP_CRITICAL )
			{
				// ALARM INVOKE
			    loc_sadb->hwNTP[0] = NTP_NORMAL ;

			}

			if( ucLclNTPDmnStatus == NTP_INITIAL )
			{
				loc_sadb->hwNTP[0] = NTP_NORMAL;
			}
			ucLclNTPDmnStatus = NTP_NORMAL;
		}
		memcpy( pstOld, &stCurr, sizeof(st_NTP_STS));
	}
	else
	{
		memcpy( pstOld, &stCurr, sizeof(st_NTP_STS));

		for( i=0; i<pstOld->dNtpCnt; i++ )
		{
			if( pstOld->stNTP[i].ucSymF == '*' || pstOld->stNTP[i].ucSymF == '+' )
			{
				usMaxVal = pstOld->stNTP[i].usPoll ;

				if( usMaxVal < pstOld->stNTP[i].usReach )
					usMaxVal = pstOld->stNTP[i].usReach;

				if( pstOld->stNTP[i].usWhen > usMaxVal )
				{
					sprintf(trcBuf, "## ABNORMAL NTP CHANNEL idx[%d] [%c] %s\n",
							i, pstOld->stNTP[i].ucSymF, pstOld->stNTP[i].szIP);
					trclib_writeLogErr(FL, trcBuf);
					ucTStatus = NTP_NORMAL;
				}
				else
				{
					ucTStatus = NTP_NORMAL;
					NntpCnt++;
					if(ucRmtNTPSvrStatus != NTP_NORMAL){
						sprintf(trcBuf, "## NORMAL NTP CHANNEL idx[%d] [%c] %s\n",
								i, pstOld->stNTP[i].ucSymF, pstOld->stNTP[i].szIP);
						trclib_writeLogErr(FL, trcBuf);
					}
				}
			}
		}

		if( NntpCnt == 0 )
		{
			if( ucTStatus != ucRmtNTPSvrStatus )
			{
				if( ucRmtNTPSvrStatus == NTP_CRITICAL )
				{
			    	loc_sadb->hwNTP[1] = NTP_NORMAL ;

					sprintf(trcBuf, "NTP CHANNEL CHANGE ABNORMAL-->NORMAL");
					trclib_writeLogErr(FL, trcBuf);

					ucRmtNTPSvrStatus = NTP_NORMAL;
					uiNTPDEADCNT = 0;
				}
				else if( ucRmtNTPSvrStatus == NTP_NORMAL )
				{
					uiNTPDEADCNT++;
					sprintf(trcBuf, "NTPDEADCNT INCREASE[%d]\n", uiNTPDEADCNT );
					trclib_writeLogErr(FL, trcBuf);

					if( loc_sadb->hwNTP[1] < 0x80 && uiNTPDEADCNT >= 12  )
					{
    					loc_sadb->hwNTP[1] &= 0x80 ;
			    		loc_sadb->hwNTP[1] |= NTP_CRITICAL ;

						sprintf(trcBuf, "NTP CHANNEL CHANGE NORMAL-->ABNORMAL NTPDEADCNT[%d]\n", uiNTPDEADCNT);
						trclib_writeLogErr(FL, trcBuf);
						ucRmtNTPSvrStatus  = NTP_CRITICAL;
					}

				}
				else  if( ucRmtNTPSvrStatus == NTP_INITIAL )
				{
					loc_sadb->hwNTP[1] &= 0x80;
					loc_sadb->hwNTP[1] |= ucTStatus ;
					ucRmtNTPSvrStatus = ucTStatus;
				}
			}
		}
		else
		{
			if( ucRmtNTPSvrStatus != NTP_NORMAL )
			{
    			loc_sadb->hwNTP[1] &= 0x80 ;
			   	loc_sadb->hwNTP[1] |= NTP_NORMAL ;

				sprintf(trcBuf, "## NTP CHANNEL CHANGE ABNORMAL-->NORMAL\n");
				trclib_writeLogErr(FL, trcBuf);
				ucRmtNTPSvrStatus = NTP_NORMAL;
				uiNTPDEADCNT = 0;
			}
			else
			{
    			loc_sadb->hwNTP[1] &= 0x80 ;
			   	loc_sadb->hwNTP[1] |= NTP_NORMAL ;
				ucRmtNTPSvrStatus = NTP_NORMAL;
				uiNTPDEADCNT = 0;
			}
		}
	}
	return 0;
}


void doDisNTPSts(IxpcQMsgType *rxIxpcMsg)
{

	MMLReqMsgType   *rxReqMsg;
	FILE	*fp;
	int     i, bufLen;
	char    trcTmp[4096], rBuff[2048];
	char	szCommand[128];

	rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;


    sprintf(szCommand, "/usr/sbin/ntpq -p");
   	fp = popen( szCommand, "r");

	if( fp == NULL )
	{
		sprintf(trcBuf, "\n  RESULT = FAIL\n  REASON = POPEN FAILED\n\n");
		MMCResSnd(rxIxpcMsg, trcBuf, -1, 0);
		return;
	}

	bufLen = 0;
	sprintf(trcTmp, "    SYSTEM = %s\n    RESULT = SUCCESS\n", mySysName);
	strcat( trcTmp, "    ==============================================================================="
					 "\n     REMOTE           REFID           ST T WHEN POLL REACH   DELAY   OFFSET  JITTER"
					 "\n    -------------------------------------------------------------------------------\n");
	bufLen = strlen(trcTmp);

	i = 0;
   	while( fgets(rBuff, 2048, fp ) != NULL )
   	{
		if( i<2 )
		{
			i++;
			continue;
		}

		sprintf( &trcTmp[bufLen], "    ");
		bufLen += 4;
       	sprintf( &trcTmp[bufLen], "%s", rBuff);
		bufLen = strlen(trcTmp);

		i++;
		if( bufLen >= 4000 )
			break;
   	}

    pclose(fp);

	if( i == 0 )
	{
		sprintf( &trcTmp[0], "\n\n  ntpq: read: Connection refused");
	}
	else
	{
		bufLen = strlen(trcTmp);
		sprintf( &trcTmp[bufLen], "    ===============================================================================\n");
	}

    strncpy(trcBuf, trcTmp, sizeof(trcBuf));
	strcat(trcBuf, "\n");
	MMCResSnd(rxIxpcMsg, trcBuf, 0, 0);

	return;
}
