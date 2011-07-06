/**********************************************************
                 ABLEX IPAS Project

   Author   : Park Si Woo
   Section  : IPAS (NTAM) CHSMD
   SCCS ID  : %W%
   Date     : %G%
   Revision History :
        '01.  7. 21     Initial
        '03.  1. 15     Initial


   Description:

   Copyright (c) ABLEX 2001
***********************************************************/

/**A.1*  File Inclusion ***********************************/
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#include <sys/ipc.h>
#include <sys/msg.h>

#include <ipaf_shm.h>
#include <ipaf_shm2.h>
#include <ipaf_define.h>
#include <ipaf_svc.h>
#include <wntas_define.h>
#include <ipaf_names.h>
#include <ipaf_etc.h>
#include <almstat.h>
#include <errno.h>

/**B.1*  Definition of New Constants ******************************************/



/**B.2*  Definition of New Type  **********************************************/

/**C.1*  Declaration of Variables  ********************************************/

extern st_NTAF      *fidb;
extern int          dNIDIndex;	/* NID Index */
extern int          gdSysNo;    /* SYSTEM NO - NTAF */


unsigned char ucStatus;	/* CHANNEL, fidb->hwntp[1] */
unsigned char ucDStatus; /* DAEMON, fidb->hwntp[0] */
unsigned char ucStatusFlag; /* Old Channel Value, if occred CRITICAL, +1, then equal 3 or bigger than 2 NORMAL */

int SetNTPSTS( int dType, unsigned char  ucCur, unsigned char ucOld  );
int CompareNTPSTS(st_NTP_STS stCurr, pst_NTP_STS pstOld );

st_NTP_STS   stNTPSTS;	
st_NTP_STS   oldNTPSTS;

int CheckNTPStS()
{
	char szCommand[126];

	char szBuf[2048];
	int i=0;
	int  readcnt=0;
	FILE *fp;
	int retlen=0;
	int  idx=0;
	int  j=0;
	char *pNeedle;
		
	char  szRetMsg[100][1024];
	char  value1[100][64];
	char  value2[100][64];
	char  value3[100][64];
	char  value4[100][64];
	char  value5[100][64];
	char  value6[100][64];
	char  value7[100][64];
	char  value8[100][64];
	char  value9[100][64];
	char  value10[100][64];

	dAppLog(LOG_INFO,"");
	dAppLog(LOG_INFO," # CHECK NTP STATUS START");


	memset(szRetMsg, 0x00, sizeof(szRetMsg) );
	
	retlen = 0;

	sprintf(szCommand, "/usr/sbin/ntpq -p");

	fp = popen( szCommand, "r");


   	i = 0;
   	while( fgets(szBuf, 2048, fp ) != NULL )
   	{
       	if( i<2 )
       	{
			pNeedle = strstr(szBuf,"refused");
			if( pNeedle != NULL ){
				break;
			}
			i++;
			continue;
       	}

       	sprintf( &szRetMsg[i-2][0], "%s", szBuf);

       	i++;
   	}

   	pclose(fp);

	memset( &stNTPSTS, 0x00, sizeof(stNTPSTS));
   	if( i == 0 )
   	{
       	sprintf( &szRetMsg[0][0], "ntpq: read: Connection refused");
       	retlen = strlen(szRetMsg[0]);
		dAppLog(LOG_CRI,"[%s]",szRetMsg[0]);
   	}
	else
	{
		idx = 0;

		readcnt = i-2;

		for( i=0; i<readcnt; i++)
		{
			sscanf( &szRetMsg[i][0], "%s %s %s %s %s %s %s %s %s %s",
				value1[i], value2[i], value3[i], value4[i],
				value5[i], value6[i], value7[i], value8[i],
				value9[i], value10[i] ) ;
			dAppLog(LOG_INFO," [%d][%s][%s][when:%s][poll:%s][reach:%s]",
				i,value1[i],value2[i],value5[i],value6[i],value7[i]);
		}

		for( i=0 ; i < readcnt ; i++ )
		{
			if( value1[i][0] == '*' || value1[i][0] == '+' )
			{
				stNTPSTS.stNTP[i].ucSymF = value1[i][0];
				sprintf(stNTPSTS.stNTP[i].szIP, "%s", &value1[i][1] );
			}
			else
			{
				stNTPSTS.stNTP[i].ucSymF = 0x00;
				sprintf(stNTPSTS.stNTP[i].szIP , "%s", &value1[i][0] );
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

/*
		for( i=0; i<stNTPSTS.dNtpCnt ; i++ )
		{
			dAppLog(LOG_DEBUG,"IDX[%2d] SYMBOL : %c", i, stNTPSTS.stNTP[i].ucSymF);
			dAppLog(LOG_DEBUG,"IDX[%2d] REMOTE : %s", i, stNTPSTS.stNTP[i].szIP);
			//dAppLog(LOG_DEBUG,"IDX[%2d] ST     : %d", i, stNTPSTS.stNTP[i].usST);
			//dAppLog(LOG_DEBUG,"IDX[%2d] T      : %c", i, stNTPSTS.stNTP[i].ucTF);
			dAppLog(LOG_DEBUG,"IDX[%2d] WHEN   : %d", i, stNTPSTS.stNTP[i].usWhen);
			dAppLog(LOG_DEBUG,"IDX[%2d] POLL   : %d", i, stNTPSTS.stNTP[i].usPoll);
			dAppLog(LOG_DEBUG,"IDX[%2d] REACH  : %d", i, stNTPSTS.stNTP[i].usReach);
			//dAppLog(LOG_DEBUG,"IDX[%2d] DELAY  : %.3f", i, stNTPSTS.stNTP[i].fDelay);
			//dAppLog(LOG_DEBUG,"IDX[%2d] OFFSET : %.3f", i, stNTPSTS.stNTP[i].fOffset);
			//dAppLog(LOG_DEBUG,"IDX[%2d] JITTER : %.3f", i, stNTPSTS.stNTP[i].fJitter);
		}
*/
			
	}

	CompareNTPSTS( stNTPSTS, &oldNTPSTS );

	return 0;

}

int CompareNTPSTS(st_NTP_STS stCurr, pst_NTP_STS pstOld)
{
	int i=0;
	int j=0;
	unsigned short usMaxVal;
	unsigned char ucTStatus=CRITICAL;

	dAppLog(LOG_INFO," ># COMPARE NTP STATUS START # [DAEMON:%d:%s][CHANNEL:%d:%s]",
		fidb->hwntp[0],
		(fidb->hwntp[0]==NORMAL)?"NORMAL":\
		(fidb->hwntp[0]==CRITICAL)?"CRITICAL":\
		(fidb->hwntp[0]==NOT_EQUIP)?"NOT_EQUIP":"UNKOWN",
		fidb->hwntp[1],
		(fidb->hwntp[1]==NORMAL)?"NORMAL":\
		(fidb->hwntp[1]==CRITICAL)?"CRITICAL":\
		(fidb->hwntp[1]==NOT_EQUIP)?"NOT_EQUIP":"UNKOWN");

	if( stCurr.dNtpCnt == 0 )
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
					dAppLog(LOG_CRI,"NTP SVR DOWN idx[%d] %s", i, pstOld->stNTP[i].szIP );
				}
				else if( pstOld->stNTP[i].ucSymF == '+' )
				{
					dAppLog(LOG_CRI,"NORMAL NTP SVR DOWN idx[%d] %s", i, pstOld->stNTP[i].szIP);
				}
				else
				{
					dAppLog(LOG_CRI,"NORMAL NTP SVR DOWN idx[%d] %s", i, pstOld->stNTP[i].szIP );
				}
			}	



			if( ucDStatus != CRITICAL )
			{
				dAppLog(LOG_CRI, "#### NTP DAEMON[/etc/init.d/ntpd] DOWN");

				/*
				* ALARM INVOKE 
				*/
				if( ucDStatus == NORMAL )
				{
					/*
					* DAEMON ALARM INVOKE
					*/
			        //fidb->hwntp[0] &= 0x80;
			        //fidb->hwntp[0] |= CRITICAL ;
	
					if( fidb->hwntp[0] < 0x80 )
						SetNTPSTS( 1, CRITICAL, ucDStatus  );
				}

				ucDStatus = CRITICAL;

				/*
				* DAEMON ALARM INVOKE 
				*/
				if( ucStatus == NORMAL )
				{
					dAppLog(LOG_CRI, "#### NTP CHANNEL NORMAL->ABNORMAL");

                    /* 
					* DAEMON ALARM INVOKE
					*/
                    //fidb->hwntp[1] &= 0x80;
                    //fidb->hwntp[1] |= CRITICAL ;

                    if( fidb->hwntp[1] < 0x80 )
						SetNTPSTS( 2, CRITICAL, ucStatus  );

					ucStatus = CRITICAL;
				}
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
				dAppLog(LOG_DEBUG, "NTP SVR ALIVE idx[%d][%c] %s", i, pstOld->stNTP[i].ucSymF,pstOld->stNTP[i].szIP );
			}
			else if( pstOld->stNTP[i].ucSymF == '+' )
			{
				dAppLog(LOG_DEBUG, "NTP SVR ALIVE(NOT PEER) idx[%d][%c] %s", 
					i, pstOld->stNTP[i].ucSymF,pstOld->stNTP[i].szIP );
			}
			else
			{
				dAppLog(LOG_DEBUG, "NTP SVR ALIVE idx[%d] %s", i, pstOld->stNTP[i].szIP );
			}
		}


		if( ucDStatus != NORMAL )
		{
			dAppLog(LOG_DEBUG,"#### NTP DAEMON[/etc/init.d/ntpd] START");

			/*
			* ALARM INVOKE 
			*/
			if( ucDStatus == CRITICAL )
			{
                /*
				* DAEMON ALARM INVOKE
				*/
                //fidb->hwntp[0] &= 0x80;
                //fidb->hwntp[0] |= NORMAL ;

                if( fidb->hwntp[0] < 0x80 )
					SetNTPSTS( 1, NORMAL, ucDStatus  );
			}

			fidb->hwntp[0] &= 0x80;
			fidb->hwntp[0] |= NORMAL;

			ucDStatus = NORMAL;
		}

	}
	else
	{
		/*
		* CHECK CHAGNED NTP STATUS
		*/

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
					dAppLog(LOG_WARN,"NTP CHANNEL idx[%d] [%c] %s[WHEN:%hu][POLL:%hu][REACH:%hu]", 
						i, pstOld->stNTP[i].ucSymF, pstOld->stNTP[i].szIP,
						pstOld->stNTP[i].usWhen,pstOld->stNTP[i].usPoll,pstOld->stNTP[i].usReach);
				}
				else
				{
					ucTStatus = NORMAL;
					dAppLog(LOG_DEBUG,"NTP CHANNEL idx[%d] [%c] %s[WHEN:%hu][POLL:%hu][REACH:%hu]", 
						i, pstOld->stNTP[i].ucSymF, pstOld->stNTP[i].szIP,
						pstOld->stNTP[i].usWhen,pstOld->stNTP[i].usPoll,pstOld->stNTP[i].usReach);
				}
			}
		}

		if( fidb->hwntp[0] == 0x00 )
		{
			dAppLog(LOG_DEBUG,"NTP STATUS CHANGE BY OTHER PROCESS ");
			fidb->hwntp[0] &= 0x80;
			fidb->hwntp[0] |= NORMAL;
		}

		if( ucTStatus == CRITICAL && ucDStatus == NORMAL ){
			ucStatusFlag++;
			dAppLog(LOG_WARN,"> OCCURED NTP CHANNEL CRITICAL :CURRENT COUNT[%d]",ucStatusFlag); 
			if( ucStatusFlag > 4 )
				ucStatusFlag--;
		}else ucStatusFlag = 0;

		if( ucTStatus != ucStatus )
		{
			
			if( ucStatus == CRITICAL )
			{
                /*
				*  DAEMON ALARM INVOKE
				*/
                //fidb->hwntp[1] &= 0x80;
                //fidb->hwntp[1] |= NORMAL ;

				/* CRITICAL -> NORMAL */
                if( fidb->hwntp[1] < 0x80 )
					SetNTPSTS( 2, NORMAL, ucStatus  );

				dAppLog(LOG_DEBUG,"NTP CHANNEL CHANGE ABNORMAL->NORMAL");
			}
			else if( ucStatus == NORMAL )
			{
                // fidb->hwntp[1] &= 0x80;
                //fidb->hwntp[1] |= CRITICAL ;
				if( ucStatusFlag > 3 ){
					ucStatusFlag = 3;

					/* NORMAL -> CRITICAL */
					if( fidb->hwntp[1] < 0x80 )
						SetNTPSTS( 2, CRITICAL, ucStatus  );

					dAppLog(LOG_CRI,"NTP CHANNEL CHANGE NORMAL-->ABNORMAL");
				}else{
					/* occured CRITICAL event, 
					 * BUT in this case, NOT CHANGE STATUS
					 * change only 'ucStatusFlag > 2'
					 * by uamyd0626 2007.03.16
					 */
					ucTStatus = NORMAL;
				}

			}
			else  if( ucStatus == 0x00 )
			{
				/* FIRST SETTING */
				fidb->hwntp[1] &= 0x80;
			    fidb->hwntp[1] |= ucTStatus ;
			} 

			ucStatus = ucTStatus;
		}

		if( fidb->hwntp[1] == 0x00 )
		{
			dAppLog(LOG_DEBUG,"NTP STATUS CHANGE BY OTHER PROCESS ");
			fidb->hwntp[1] &= 0x80;
			fidb->hwntp[1] |= ucStatus;
		}
	}
	
	dAppLog(LOG_INFO,"  # SUCCESS COMPARE NTP STATUS # [DAEMON:%d:%s][CHANNEL:%d:%s]",
		fidb->hwntp[0],
		(fidb->hwntp[0]==NORMAL)?"NORMAL":\
		(fidb->hwntp[0]==CRITICAL)?"CRITICAL":\
		(fidb->hwntp[0]==NOT_EQUIP)?"NOT_EQUIP":"UNKOWN",
		fidb->hwntp[1],
		(fidb->hwntp[1]==NORMAL)?"NORMAL":\
		(fidb->hwntp[1]==CRITICAL)?"CRITICAL":\
		(fidb->hwntp[1]==NOT_EQUIP)?"NOT_EQUIP":"UNKOWN");

	return 0;	
} 


int SetNTPSTS( int dType, unsigned char ucCur, unsigned char ucOld  )
{
    time_t nowtime;
    st_MsgQ stMq;
    st_MsgQSub *stSubMq;
    T_Alm_Status stAlm;
    int dQid;
    int dLen, dRet;

	dAppLog(LOG_INFO," >># SEND NTP STATUS START");

	if( dType == 1 )
	{
		fidb->hwntp[0] &= 0x80;
		fidb->hwntp[0] |= ucCur ;	

		if( fidb->hwntp[0] >= 0x80 ){
			return 0;
		}
	}
	else
	{
		fidb->hwntp[1] &= 0x80;
		fidb->hwntp[1] |= ucCur ;
		if( fidb->hwntp[1] >= 0x80 )
			return 0;

	}

    stAlm.ucSysType = SYSTYPE_NTAF;
    stAlm.ucSysNo = gdSysNo;
    stAlm.ucLocType = LOCTYPE_NTP;
    stAlm.ucInvType = INVTYPE_NTPSVR;
    stAlm.ucInvNo = dType-1;
    stAlm.ucAlmLevel = ucCur;
    stAlm.ucOldAlmLevel = ucOld;
    time(&nowtime);
    stAlm.tWhen = nowtime;
    stAlm.uiIPAddr = 0;


    dQid = msgget( S_MSGQ_CI_SVC, 0666);

    if( dQid < 0 )
    {
        dAppLog(LOG_CRI,"[FAIL:%d] CI_SVC[0x%X] msgget [%s]", errno, S_MSGQ_CI_SVC,strerror(errno));
        return -1;
    }


    memset( &stMq, 0x00, sizeof(st_MsgQ) );

    stSubMq = (st_MsgQSub*)&stMq.llMType;
    stSubMq->usType = DEF_SYS;
    stSubMq->usSvcID =  SID_STATUS;
    stSubMq->usMsgID = MID_CONSOL;

    dMakeNID( SEQ_PROC_CHSMD, &stMq.llNID );
	stMq.ucNTAMID = 0;
	stMq.ucNTAFID = stAlm.ucSysNo;
    stMq.ucProID = SEQ_PROC_CHSMD;
    stMq.llIndex = dNIDIndex;
	dNIDIndex++;

    stMq.dMsgQID = dQid;

    stMq.usBodyLen = sizeof(T_Alm_Status) + NTAFT_HEADER_LEN;
    stMq.usRetCode = 0;

	memset( &stMq.szMIN, 0x00, MAX_MIN_SIZE );
	memset( &stMq.szExtra, 0x00, MAX_EXTRA_SIZE );
    memcpy( &stMq.szBody[NTAFT_HEADER_LEN], &stAlm, sizeof(T_Alm_Status) );

    dLen = DEF_MSGHEAD_LEN + stMq.usBodyLen ;

    //dRet = msgsnd( dQid, &stMq, dLen, IPC_NOWAIT) ;
	dRet = 1;
    if( dRet < 0 )
    {
        dAppLog(LOG_CRI,"[FAIL] MSGSND CI_SVC[0x%X] [%d:%s]", S_MSGQ_CI_SVC,errno,strerror(errno) );
        return -1;
    }
    dAppLog(LOG_DEBUG,"[SUCC] MSGSND CI_SVC[0x%X]: SLEN[%d] BLEN[%d] PROID[%d] SID[%d] MID[%d] INVTYPE[%d] INVNO[%d] STATUS[%d]",
		S_MSGQ_CI_SVC, dLen, stMq.usBodyLen,
		stMq.ucProID, stSubMq->usSvcID, stSubMq->usMsgID,
        stAlm.ucInvType, stAlm.ucInvNo, stAlm.ucAlmLevel );

    return 0;
}


