/**A.1 * File Include *********************************************************/

/* SYS HEADER */
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <stdlib.h>
/* LIB HEADER */
#include "clisto.h"		/* U8 */
#include "filedb.h"
#include "loglib.h"
#include "utillib.h"
/* PRO HEADER */
#include "msgdef.h"
#include "procid.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
#include "almstat.h"
/* LOC HEADER */
#include "chsmd_func.h"		/* dMsgsnd(), dGetNode() */
#include "chsmd_ntp.h"

/**B.1*  Definition of New Constants ******************************************/
/**B.2*  Definition of New Type  **********************************************/
/**C.1*  Declaration of Variables  ********************************************/

unsigned char ucStatus;
unsigned char ucDStatus;
unsigned char ucStatusFlag;

st_NTP_STS   stNTPSTS;
st_NTP_STS   oldNTPSTS;

extern pst_NTAM fidb;
extern int gdIndex;
extern int gdSysNo;



int CheckNTPStS(void)
{
	char 	szCommand[126], szRetMsg[100][1024], szBuf[2048];
	char    value1[100][64], value2[100][64], value3[100][64], value4[100][64];
	char    value5[100][64], value6[100][64], value7[100][64], value8[100][64];
	char    value9[100][64], value10[100][64];
	char    *pNeedle;
	int		i, readcnt, retlen, idx;
	FILE	*fp;

	i		= 0;
	readcnt	= 0;
	retlen	= 0;
	idx		= 0;

	log_print(LOGN_INFO, " # CHECK NTP STATUS ##########################");
	memset(szRetMsg, 0x00, sizeof(szRetMsg) );
	sprintf(szCommand, "/usr/sbin/ntpq -p");

	if( (fp = popen(szCommand, "r")) == NULL)
	{
		log_print(LOGN_CRI, LH"FAILED IN popen(szCommand=%s)"EH, LT, szCommand, ET);
		return -1;
	}

	while(fgets(szBuf, 2048, fp) != NULL)
	{
		if(i < 2)
		{
			pNeedle = strstr(szBuf, "refused");
			if(pNeedle != NULL)
			{
				break;
			}
			i++;
			continue;
		}

		sprintf(&szRetMsg[i-2][0], "%s", szBuf);

		i++;
	}
   	pclose(fp);

	memset(&stNTPSTS, 0x00, sizeof(stNTPSTS));
	if(i == 0)
	{
		sprintf(&szRetMsg[0][0], "ntpq: read: Connection refused");
		retlen = strlen(szRetMsg[0]);
		log_print(LOGN_CRI, "FAILED IN command[%s]:[%s]", szCommand, szRetMsg[0]);
	}
	else
	{
		readcnt = i-2;
		for(i = 0; i < readcnt; i++)
		{
			sscanf(&szRetMsg[i][0], "%s %s %s %s %s %s %s %s %s %s",
				value1[i], value2[i], value3[i], value4[i], value5[i], 
				value6[i], value7[i], value8[i], value9[i], value10[i]) ;
			log_print(LOGN_INFO, " [%d][%s][%s][when:%s][poll:%s][reach:%s]", 
				i, value1[i], value2[i], value5[i], value6[i], value7[i]);
		}

		for(i = 0; i < readcnt; i++)
		{
			if( (value1[i][0] == '*') || (value1[i][0] == '+'))
			{
				stNTPSTS.stNTP[i].ucSymF = value1[i][0];
				sprintf(stNTPSTS.stNTP[i].szIP, "%s", &value1[i][1]);
			}
			else
			{
				stNTPSTS.stNTP[i].ucSymF = 0x00;
				sprintf(stNTPSTS.stNTP[i].szIP , "%s", &value1[i][0]);
			}

			stNTPSTS.stNTP[i].usST		= atoi(value3[i]);
			stNTPSTS.stNTP[i].ucTF		= value4[i][0];
			stNTPSTS.stNTP[i].usWhen	= atoi(value5[i]);
			stNTPSTS.stNTP[i].usPoll	= atoi(value6[i]);
			stNTPSTS.stNTP[i].usReach	= atoi(value7[i]);

			sscanf(value8[i], "%f", &stNTPSTS.stNTP[i].fDelay);
			sscanf(value9[i], "%f", &stNTPSTS.stNTP[i].fOffset);
			sscanf(value10[i], "%f", &stNTPSTS.stNTP[i].fJitter);
		}
		stNTPSTS.dNtpCnt = readcnt ;
	}
	CompareNTPSTS(stNTPSTS, &oldNTPSTS);

	return 0;
}

int CompareNTPSTS(st_NTP_STS stCurr, pst_NTP_STS pstOld)
{
	int i=0;
	unsigned short usMaxVal;
	unsigned char ucTStatus=CRITICAL;

	log_print(LOGN_INFO," ># COMPARE NTP STATUS START # [DAEMON:%d:%s][CHANNEL:%d:%s]",
        fidb->hwNTP[0],
        (fidb->hwNTP[0]==NORMAL)?"NORMAL":\
        (fidb->hwNTP[0]==CRITICAL)?"CRITICAL":\
        (fidb->hwNTP[0]==NOT_EQUIP)?"NOT_EQUIP":"UNKOWN",
        fidb->hwNTP[1],
        (fidb->hwNTP[1]==NORMAL)?"NORMAL":\
        (fidb->hwNTP[1]==CRITICAL)?"CRITICAL":\
        (fidb->hwNTP[1]==NOT_EQUIP)?"NOT_EQUIP":"UNKOWN");

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
					log_print(LOGN_CRI,"NTP SVR DOWN idx[%d] %s", i, pstOld->stNTP[i].szIP );
				}
				else if( pstOld->stNTP[i].ucSymF == '+' )
				{
					log_print(LOGN_CRI,"NORMAL NTP SVR DOWN idx[%d] %s", i, pstOld->stNTP[i].szIP);
				}
				else
				{
					log_print(LOGN_CRI,"NORMAL NTP SVR DOWN idx[%d] %s", i, pstOld->stNTP[i].szIP );
				}
			}



			if( ucDStatus != CRITICAL )
			{
				log_print(LOGN_CRI, "#### NTP DAEMON[/etc/init.d/ntpd] DOWN");

				/*
				* ALARM INVOKE
				*/
				if( ucDStatus == NORMAL )
				{
					/*
					* DAEMON ALARM INVOKE
					*/
					SetNTPSTS( 1, CRITICAL, ucDStatus  );
				}

				ucDStatus = CRITICAL;

				/*
				* DAEMON ALARM INVOKE
				*/
				if( ucStatus == NORMAL )
				{
					log_print(LOGN_CRI, "#### NTP CHANNEL NORMAL->ABNORMAL");

                    /*
					* DAEMON ALARM INVOKE
					*/
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
				log_print(LOGN_DEBUG, " NTP SVR ALIVE idx[%d][%c] %s", i, pstOld->stNTP[i].ucSymF,pstOld->stNTP[i].szIP );
			}
			else if( pstOld->stNTP[i].ucSymF == '+' )
			{
				log_print(LOGN_DEBUG, " NTP SVR ALIVE(NOT PEER) idx[%d][%c] %s",
				i, pstOld->stNTP[i].ucSymF,pstOld->stNTP[i].szIP );
			}
			else
			{
				log_print(LOGN_DEBUG, "NTP SVR ALIVE idx[%d] %s", i, pstOld->stNTP[i].szIP );
			}
		}


		if( ucDStatus != NORMAL )
		{
			log_print(LOGN_DEBUG,"#### NTP DAEMON[/etc/init.d/ntpd] START");

			/*
			* ALARM INVOKE
			*/
			if( ucDStatus == CRITICAL )
			{
                /*
				* DAEMON ALARM INVOKE
				*/
				SetNTPSTS( 1, NORMAL, ucDStatus  );
			}

			fidb->hwNTP[0] &= 0x80;
			fidb->hwNTP[0] |= NORMAL;

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

/*
				if( usMaxVal < pstOld->stNTP[i].usReach )
					usMaxVal = pstOld->stNTP[i].usReach;
*/

				if( pstOld->stNTP[i].usWhen > usMaxVal )
                {
                    log_print(LOGN_WARN,"NTP CHANNEL idx[%d] [%c] %s[WHEN:%hu][POLL:%hu][REACH:%hu]",
                        i, pstOld->stNTP[i].ucSymF, pstOld->stNTP[i].szIP,
                        pstOld->stNTP[i].usWhen,pstOld->stNTP[i].usPoll,pstOld->stNTP[i].usReach);
                }
                else
                {
                    ucTStatus = NORMAL;
                    log_print(LOGN_DEBUG,"NTP CHANNEL idx[%d] [%c] %s[WHEN:%hu][POLL:%hu][REACH:%hu]",
                        i, pstOld->stNTP[i].ucSymF, pstOld->stNTP[i].szIP,
                        pstOld->stNTP[i].usWhen,pstOld->stNTP[i].usPoll,pstOld->stNTP[i].usReach);
                }
			}
		}

		if( fidb->hwNTP[0] == 0x00 )
		{
			log_print(LOGN_DEBUG,"NTP STATUS CHANGE BY OTHER PROCESS ");
			fidb->hwNTP[0] &= 0x80;
			fidb->hwNTP[0] |= NORMAL;
		}

		if( ucTStatus == CRITICAL && ucDStatus == NORMAL ){
            ucStatusFlag++;
            log_print(LOGN_WARN,"> OCCURED NTP CHANNEL CRITICAL :CURRENT COUNT[%d]",ucStatusFlag);
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
				SetNTPSTS( 2, NORMAL, ucStatus  );

				log_print(LOGN_DEBUG,"NTP CHANNEL CHANGE ABNORMAL->NORMAL");
			}
			else if( ucStatus == NORMAL )
			{
				if( ucStatusFlag > 3 ){
					ucStatusFlag = 0;
					SetNTPSTS( 2, CRITICAL, ucStatus  );

					log_print(LOGN_CRI,"NTP CHANNEL CHANGE NORMAL-->ABNORMAL");
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
				fidb->hwNTP[1] &= 0x80;
			    fidb->hwNTP[1] |= ucTStatus ;
			}

			ucStatus = ucTStatus;
		}

		if( fidb->hwNTP[1] == 0x00 )
		{
			log_print(LOGN_DEBUG,"NTP STATUS CHANGE BY OTHER PROCESS ");
			fidb->hwNTP[1] &= 0x80;
			fidb->hwNTP[1] |= ucStatus;
		}
	}

	log_print(LOGN_INFO,"  # SUCCESS COMPARE NTP STATUS # DAEMON:%d:%s][CHANNEL:%d:%s]",
        fidb->hwNTP[0],
        (fidb->hwNTP[0]==NORMAL)?"NORMAL":\
        (fidb->hwNTP[0]==CRITICAL)?"CRITICAL":\
        (fidb->hwNTP[0]==NOT_EQUIP)?"NOT_EQUIP":"UNKOWN",
        fidb->hwNTP[1],
        (fidb->hwNTP[1]==NORMAL)?"NORMAL":\
        (fidb->hwNTP[1]==CRITICAL)?"CRITICAL":\
        (fidb->hwNTP[1]==NOT_EQUIP)?"NOT_EQUIP":"UNKOWN");

	return 0;
}


int SetNTPSTS( int dType, unsigned char ucCur, unsigned char ucOld  )
{
    pst_MsgQ    pstMq;
    st_MsgQSub *stSubMq;
    st_almsts   stAlm;
    time_t      nowtime;
    int 		dRet;
	U8		   *pNODE;

	log_print(LOGN_INFO," ## SEND NTP STATUS #");

	if( dType == 1 ) {

		fidb->hwNTP[0] &= MASK;
		fidb->hwNTP[0] |= ucCur ;

		if( fidb->hwNTP[0] >= MASK )
			return 0;
	} else {

		fidb->hwNTP[1] &= MASK;
		fidb->hwNTP[1] |= ucCur ;
		if( fidb->hwNTP[1] >= MASK )
			return 0;
	}

    stAlm.ucSysType     = SYSTYPE_TAM;
    stAlm.ucSysNo       = gdSysNo;
    stAlm.ucLocType     = LOCTYPE_NTP;
    stAlm.ucInvType     = INVTYPE_NTPSVR;
    stAlm.ucInvNo       = dType-1;
    stAlm.ucAlmLevel    = ucCur;
    stAlm.ucOldAlmLevel = ucOld;
    time(&nowtime);
    stAlm.tWhen    = nowtime;
    stAlm.uiIPAddr = 0;

	if( (dRet = dGetNode(&pNODE, &pstMq)) < 0 ){
        log_print(LOGN_CRI, LH"FAILED IN dGetNode(CHSMD)",LT);
        return -1;
    }


    stSubMq = (st_MsgQSub*)&pstMq->llMType;
    stSubMq->usType  = DEF_SYS;
    stSubMq->usSvcID = SID_STATUS;
    stSubMq->usMsgID = MID_CONSOL;

    util_makenid( SEQ_PROC_CHSMD, &pstMq->llNID );

	pstMq->ucNTAFID = 0;
    pstMq->ucProID  = SEQ_PROC_CHSMD;
    pstMq->llIndex  = gdIndex;
	gdIndex++;

    pstMq->dMsgQID = 0;

    pstMq->usBodyLen = sizeof(st_almsts);
    pstMq->usRetCode = 0;

    memcpy( &pstMq->szBody[0], &stAlm, sizeof(st_almsts) );

	if( (dRet = dMsgsnd(SEQ_PROC_COND, pNODE)) < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN dMsgsnd(COND)"EH, LT, ET);
        return -2;
	}

    log_print(LOGN_DEBUG, LH"[SUCC] dMsgsnd COND=%d: BLEN=%d, PROID=%d, SID=%d, MID=%d, INVTYPE=%d, INVNO=%d, STATUS=%d",
		LT, SEQ_PROC_COND, pstMq->usBodyLen,
		pstMq->ucProID, stSubMq->usSvcID, stSubMq->usMsgID,
        stAlm.ucInvType, stAlm.ucInvNo, stAlm.ucAlmLevel );

    return 0;
}
