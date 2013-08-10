/***** A.1 * File Include *******************************/

/* SYS HEADER */
#include <stdio.h>     /* FGETS(3), FOPEN(3) */
#include <errno.h>     /* ERRNO */
#include <string.h>	   /* STRCAT(3), MEMCPY(3), MEMSET(3), STRSTR(3) */
#include <sys/types.h> /* MSGGET(), MKDIR(2) */
#include <sys/ipc.h>   /* MSGGET() */
#include <sys/stat.h>  /* MKDIR(2) */
#include <fcntl.h>     /* MKDIR(2) */
#include <unistd.h>    /* MKDIR(2) */
/* LIB HEADER */
#include "clisto.h"		/* OFFSET */
#include "commdef.h"
#include "loglib.h"
#include "utillib.h"
/* PRO HEADER */
#include "path.h"
#include "msgdef.h"
#include "procid.h"
#include "mmcdef.h"
#include "sockio.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
#include "almstat.h"
/* LOC HEADER */
#include "cond_func.h"			/* cvt_ipaddr() */
#include "cond_sock.h"			/* dSendMessage() */
#include "cond_ipc.h"			/* dMsgsnd(), dMsgrcv(), dGetNode() */
#include "cond_mem.h"

/**D.2*  Definition of Functions  ( External ) ************/
extern st_MSG_BUF gstMsgBuf[MAX_FDSET2];
extern st_ConTbl  stConTbl[MAX_FDSET2+1];

extern fd_set 	gstWritefds;
extern fd_set	gstReadfds;

extern int 		gdNumWfds;

int dSend_FSTAT(st_MsgQ *pstMsg)
{
	pst_MsgQSub	pstMsgSub;

	pstMsgSub			= (pst_MsgQSub)pstMsg;

	pstMsgSub->usType 	= DEF_SVC;
	pstMsgSub->usSvcID	= SID_SVC;
	pstMsgSub->usMsgID	= MID_STAT_FAULT;

	if( dMsgsnd2( SEQ_PROC_FSTAT ) < 0 ){
		return -1;
	}

	log_print(LOGN_DEBUG, LH"[SUCCESS] MSGSND FSTAT=%d, TYPE=%d,SID=%d,MID=%d,BODY=%d,TAFID=%d,TAMID=%d",
		LT, SEQ_PROC_FSTAT,
		pstMsgSub->usType, pstMsgSub->usSvcID, pstMsgSub->usMsgID, pstMsg->usBodyLen, pstMsg->ucNTAFID, pstMsg->ucNTAMID);

	return 0;
}

int dSend_SI_NMS(char *psData, size_t szStrLen)
{
	int			dRet;
	pst_MsgQ	pstMsgQ;
	pst_MsgQSub	pstMsgQSub;
	U8		   *pNODE;

	if( (dRet = dGetNode(&pNODE, &pstMsgQ)) < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN dGetNode(COND)",LT);
		return -1;
	}

	pstMsgQSub			= (pst_MsgQSub)&pstMsgQ->llMType;
	pstMsgQSub->usType	= DEF_SVC;
	pstMsgQSub->usSvcID	= SID_SVC;
	pstMsgQSub->usMsgID	= MID_LOG_CONSOLE;

	pstMsgQ->usBodyLen	= szStrLen;
	pstMsgQ->ucProID	= SEQ_PROC_COND;
	pstMsgQ->dMsgQID	= 0;

	memcpy(pstMsgQ->szBody, psData, szStrLen);

	if( (dRet = dMsgsnd( SEQ_PROC_SI_NMS, pNODE )) < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN dMsgsnd(to SI_NMS)"EH,LT,ET);
		return -2;
	}
	log_print(LOGN_DEBUG, "[SUCCESS] MSGSND SI_NMS=%d, TYPE[%d] SID[%d] MID[%d] BODY[%d] TAFID[%d] TAMID[%d]",
		SEQ_PROC_SI_NMS, pstMsgQSub->usType, pstMsgQSub->usSvcID, pstMsgQSub->usMsgID, pstMsgQ->usBodyLen, pstMsgQ->ucNTAFID, pstMsgQ->ucNTAMID);

	return 0;
}

int dSend_SI_NMS_Alarm(st_almsts *psData)
{
	int			dRet;
	pst_MsgQ	pstMsgQ;
	pst_MsgQSub	pstMsgQSub;
	U8		   *pNODE;

	if( (dRet = dGetNode(&pNODE, &pstMsgQ)) < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN dGetNode(COND)",LT);
		return -1;
	}
	pstMsgQSub			= (pst_MsgQSub)&pstMsgQ->llMType;
	pstMsgQSub->usType	= DEF_SVC;
	pstMsgQSub->usSvcID	= SID_SVC;
	pstMsgQSub->usMsgID	= MID_LOG_ALARM;

	pstMsgQ->usBodyLen	= DEF_ALMSTS_SIZE;
	pstMsgQ->ucProID	= SEQ_PROC_COND;
	pstMsgQ->dMsgQID	= 0;

	memcpy(pstMsgQ->szBody, psData, pstMsgQ->usBodyLen);

	if( (dRet = dMsgsnd( SEQ_PROC_SI_NMS, pNODE )) < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN dMsgsnd(to SI_NMS)"EH,LT,ET);
		return -2;
	}
	
	log_print(LOGN_DEBUG, "[SUCCESS] Alarm MSGSND SI_NMS=%d, TYPE[%d] SID[%d] MID[%d] BODY[%d] TAFID[%d] TAMID[%d]",
		SEQ_PROC_SI_NMS,
		pstMsgQSub->usType, pstMsgQSub->usSvcID, pstMsgQSub->usMsgID, pstMsgQ->usBodyLen, pstMsgQ->ucNTAFID, pstMsgQ->ucNTAMID);

	return 0;
}

/********************************************************************
	Function Name :

	Parameter(s):

	Function:

	Return:

	History:
		Created By TUNDRA in 02.12.11

*********************************************************************/
int dMergeBuffer(int dIdx, int dSfd, char* szTcpRmsg, int dRecvLen)
{
    long long llMagicNo = MAGIC_NUMBER;
    int     dPktSize;
    int     dRet;
    st_MngPkt    stRpkt;

    if( (dRecvLen + gstMsgBuf[dIdx].dWidx) < MAX_MNG_PKT_BUFSIZE )
    {
        memcpy( &gstMsgBuf[dIdx].szBuf[gstMsgBuf[dIdx].dWidx], &szTcpRmsg[0], dRecvLen );

        gstMsgBuf[dIdx].dWidx += dRecvLen;

        if( gstMsgBuf[dIdx].dWidx < 8 )
            return 0;

        if( *(long long*)&gstMsgBuf[dIdx].szBuf[0] == llMagicNo )
        {
            if( gstMsgBuf[dIdx].dWidx > MNG_PKT_HEAD_SIZE )
            {
                stRpkt.head = *(pst_MngHead)&gstMsgBuf[dIdx].szBuf[0];
                dPktSize = stRpkt.head.usBodyLen + MNG_PKT_HEAD_SIZE;

                if( dPktSize <= gstMsgBuf[dIdx].dWidx )
                {
                    memcpy( &stRpkt.data[0], &gstMsgBuf[dIdx].szBuf[MNG_PKT_HEAD_SIZE], stRpkt.head.usBodyLen );

                    dRet = dRcvPkt_Handle( dIdx, dSfd, stRpkt );
					gstMsgBuf[dIdx].dWidx -= dPktSize ;
					if( dRet < 0 )
						return -1;

                    if( gstMsgBuf[dIdx].dWidx > 0 )
                    {
                        memcpy( &gstMsgBuf[dIdx].szBuf[0], &gstMsgBuf[dIdx].szBuf[dPktSize],
								 gstMsgBuf[dIdx].dWidx  );

                        while( gstMsgBuf[dIdx].dWidx > MNG_PKT_HEAD_SIZE )
                        {
                            if( *(long long*)&gstMsgBuf[dIdx].szBuf[0] == llMagicNo )
                            {
                                stRpkt.head = *(pst_MngHead)&gstMsgBuf[dIdx].szBuf[0];

                                dPktSize = stRpkt.head.usBodyLen;

                                if( (dPktSize+MNG_PKT_HEAD_SIZE) <= gstMsgBuf[dIdx].dWidx )
                                {
                                    memcpy( &stRpkt.data[0], &gstMsgBuf[dIdx].szBuf[MNG_PKT_HEAD_SIZE], dPktSize );

                                    gstMsgBuf[dIdx].dWidx -= MNG_PKT_HEAD_SIZE+dPktSize;

                                    dRet = dRcvPkt_Handle( dIdx, dSfd, stRpkt );

									if( dRet < 0 )
										return -1;


                                    if( gstMsgBuf[dIdx].dWidx > 0 )
                                    {
                                        memcpy( &gstMsgBuf[dIdx].szBuf[0], &gstMsgBuf[dIdx].szBuf[MNG_PKT_HEAD_SIZE+dPktSize], gstMsgBuf[dIdx].dWidx );
                                    }
                                }
                                else
                                {
                                    break;
                                }
                            }
                            else
                            {
                    			log_print(LOGN_DEBUG,"$$$$$$$$$$$$$$$$$ fail dBufinIdx[%d]\n", gstMsgBuf[dIdx].dWidx);
								return -1;
                            }
                        }
                    }
                    else
                    {
                        gstMsgBuf[dIdx].dWidx = 0;
                    }

                    return 1;
                }
            }
        }
        else
        {
            log_print(LOGN_DEBUG,"LOST FRAME\n");
            gstMsgBuf[dIdx].dWidx = 0;
            return -1;
        }
    }
    else
    {
        log_print(LOGN_DEBUG,"BUFFER OVERFLOW INVOKE\n");
        return -2;
    }

    return 0;
}



/********************************************************************
    Function Name : dRcvPkt_Handle

    Parameter(s):
        int         dSsfd ; (input) send sockfd
        st_MyPKT    stMyPkt ; (input) recv Packet

    Function:

    Return:
        Success : 0
        Failure :   -1 :  General Fail


    History:
        Created By TUNDRA in 02.12.11

*********************************************************************/

int dRcvPkt_Handle(int dIdx, int dSsfd, st_MngPkt stMyPkt)
{
    switch( stMyPkt.head.ucSvcID )
    {
    case 0x01:


		break;

    case 0x02:
    case 0x03:

        break;

    case 0x22:


        break;

    case 0x24:
        if( stMyPkt.head.ucMsgID == 0x01 )
        {
        }

		return 0;
        break;

    default:
        log_print(LOGN_DEBUG,"except packet recv\n");
        return -1;
        break;

    }


    return 0;

}


int dIsRcvedMessage(st_MsgQ *pstMsg)
{
	int dRet;

	if( (dRet = dMsgrcv(&pstMsg)) < 0 ){
		if( dRet != -1 ){
			log_print(LOGN_CRI, LH"FAILED IN dMsgrcv(COND)",LT);
		}
		return NO_MSG;
	}
	return 1;
}

int dMake_Dir( struct tm *Tm, char *szLogPath )
{
    int     dRet;
    char    szLogFilePath[128];

    //sprintf( szLogFilePath, "%s/%02d", DEF_COND_LOGN_PATH, Tm->tm_year-100 );
	sprintf( szLogFilePath, "%s", DEF_HIS_PATH );

	/*
	* MAKE MONTH PATH
	*/
	sprintf( szLogFilePath, "%s/%02d", szLogFilePath, Tm->tm_mon+1 );

    dRet = mkdir( szLogFilePath, 0755 );
    if( dRet < 0 )
    {
        if( errno != EEXIST )
        {
            log_print(LOGN_CRI,"[ERROR] MAKE MON DIRECTORY [%d] [%s]", errno, strerror(errno) );

            return -1;
        }
    }

	/*
	* MAKE DAY PATH
	*/
	sprintf( szLogFilePath, "%s/%02d", szLogFilePath, Tm->tm_mday );

    dRet = mkdir( szLogFilePath, 0755 );
    if( dRet < 0 )
    {
        if( errno != EEXIST )
        {
            log_print(LOGN_CRI,"[ERROR] MAKE MON DIRECTORY [%d] [%s]", errno, strerror(errno) );

            return -1;
        }
    }

	/*
	* MAKE LAST LOG PATH
	*/
	sprintf( szLogFilePath, "%s/%s", szLogFilePath, szLogPath );

    dRet = mkdir( szLogFilePath, 0755 );
    if( dRet < 0 )
    {
        if( errno != EEXIST )
        {
            log_print(LOGN_CRI,"[ERROR] MAKE YEAR DIRECTORY [%d] [%s]", errno, strerror(errno) );

            return -1;
        }
    }

    return 1;
}


int dSendToOMPFLTInfo( int dSocketFd, int dConTblIdx )
{
	FILE		*fp;

	int			i;
	int			dRet;
	int			dBufLength = 0;
	time_t		tNow;

	struct tm	*stYesterdayTM;
	struct tm 	*stTodayTM;

	char		szYesterdayPath[256];
	char		szYesterdayFile[256];
	char		szTodayPath[256];
	char		szTodayFile[256];

	char		szTmpBuf[256];
	char		szBuffer[MAX_MNGPKT_BODY_SIZE];		/* MAX_MNGPKT_BODY_SIZE : 3584 */

	tNow = time( &tNow );
	stTodayTM = localtime( &tNow );

	/*
    * SET TODAY COND FILE PATH
    */
    sprintf( szTodayPath, "%s%02d/%02d/%s",
            DEF_HIS_PATH, stTodayTM->tm_mon+1, stTodayTM->tm_mday, DEF_FLT_LOG_PATH );

	tNow = tNow - 86400;
	stYesterdayTM = localtime( &tNow );

	/*
	* SET YESTERDAY COND FILE PATH
	*/
	sprintf( szYesterdayPath, "%s%02d/%02d/%s",
			DEF_HIS_PATH, stYesterdayTM->tm_mon+1, stYesterdayTM->tm_mday, DEF_FLT_LOG_PATH );

	/*
	* SEND YESTERDAY FLT INFOMATION
	*/

	for( i=0; i<24; i++ ) {
		sprintf( szYesterdayFile, "%sFLT_RESULT_%02d.dat", szYesterdayPath, i );

		fp = fopen( szYesterdayFile, "r" );
		if( fp == NULL ) {
			if( errno == ENOENT ) {
				log_print(LOGN_DEBUG, "THIS FILE [%s] NOT EXIST",
						szYesterdayFile );
				continue;
			}
			else {
				log_print(LOGN_DEBUG, "THIS FILE [%s] NOT EXIST [%d] [%s]",
						szYesterdayFile, errno, strerror(errno) );
				continue;
			}
		}

		while( fgets( szTmpBuf, 256, fp ) != NULL ) {
			/*
            * CHECK SEND WHEN COMPLETED DATA
            */
			if( strlen( szTmpBuf ) == 1 ) {
				sprintf( &szBuffer[dBufLength], "%s", szTmpBuf );
				dBufLength += strlen( szTmpBuf );

				/*
				* CHECK SEND BUFFER MAX LENGTH
				*/
				if( dBufLength > 3000 ) {
					dRet = dSendFLTDATA( dSocketFd, dConTblIdx, dBufLength, (char *)&szBuffer[0] );
					if( dRet < 0 ) {
						log_print( LOGN_CRI, "ERROR IN dSendFLTDATA() : SOCKET CLOSED");

						return -1;
					}

					dBufLength = 0;
				}
			}
			else {
				sprintf( &szBuffer[dBufLength], "%s", szTmpBuf );
				dBufLength += strlen( szTmpBuf );
			}
		} /* WHILE LOOP END */

		fclose( fp );
	}

	/*
	* SEND TODAY FLT INFORMATION
	*/
	for( i=0; i<24; i++ ) {
        sprintf( szTodayFile, "%sFLT_RESULT_%02d.dat", szTodayPath, i );

        fp = fopen( szTodayFile, "r" );
        if( fp == NULL ) {
            if( errno == ENOENT ) {
                log_print(LOGN_DEBUG, "THIS FILE [%s] NOT EXIST",
                        szTodayFile );
                continue;
            }
            else {
                log_print(LOGN_DEBUG, "THIS FILE [%s] NOT EXIST [%d] [%s]",
                        szTodayFile, errno, strerror(errno) );
                continue;
            }
        }

        while( fgets( szTmpBuf, 256, fp ) != NULL ) {
			/*
			* CHECK SEND WHEN COMPLETED DATA
			*/
            if( strlen( szTmpBuf ) == 1 ) {
                sprintf( &szBuffer[dBufLength], "%s", szTmpBuf );
                dBufLength += strlen( szTmpBuf );

                /*
                * CHECK SEND BUFFER MAX LENGTH
                */
                if( dBufLength > 3000 ) {
                    dRet = dSendFLTDATA( dSocketFd, dConTblIdx, dBufLength, (char *)&szBuffer[0] );
                    if( dRet < 0 ) {
                        log_print( LOGN_CRI, "ERROR IN dSendFLTDATA() : SOCKET CLOSED");

						return -1;
                    }

                    dBufLength = 0;
                }
            }
            else {
                sprintf( &szBuffer[dBufLength], "%s", szTmpBuf );
                dBufLength += strlen( szTmpBuf );
            }
        } /* WHILE LOOP END */

		fclose( fp );
    }

	dRet = dSendFLTDATA( dSocketFd, dConTblIdx, dBufLength, (char *)&szBuffer[0] );
	if( dRet < 0 ) {
		log_print( LOGN_CRI, "ERROR IN dSendFLTDATA() : SOCKET CLOSED");

		return -1;
	}

	return 1;
}

int dSendFLTDATA(int dSocketFd, int dConTblIdx, int MsgLen, char *szMsg)
{
	int				dRet;

	st_MngPkt  		stSpkt;
	fd_set  		stWd;

	memset( &stSpkt, 0x00, sizeof( stSpkt ) );
    stSpkt.head.llMagicNumber = MAGIC_NUMBER;
    stSpkt.head.llIndex = 1;
    stSpkt.head.usResult = 0;

    stSpkt.head.usTotPage = 0;
    stSpkt.head.usCurPage = 0;
    stSpkt.head.usBodyLen = MsgLen;
    stSpkt.head.usTotLen = MsgLen + MNG_PKT_HEAD_SIZE;

	/*
	* SEND COND DATA FLAG
	*/
	stSpkt.head.ucBinFlag = 2;

    memcpy( &stSpkt.data[0], &szMsg[0], MsgLen );
    stSpkt.data[MsgLen] = 0x00;

	/*
	* CASE : NON BLOCKED SOCKET FD
	*/
	if( stConTbl[dConTblIdx].dSfd > 0 && stConTbl[dConTblIdx].cSockBlock == 0x00 ) {
		dRet = dSendMessage( dConTblIdx, dSocketFd, stSpkt.head.usTotLen, (char*)&stSpkt );
		if( dRet < 0 ) {
			log_print(LOGN_CRI,"SEND MESSAGE FAIL IDX[%d] SFD[%d] IP[%s]",
					dConTblIdx, dSocketFd, cvt_ipaddr(stConTbl[dConTblIdx].uiCliIP) );

			close(dSocketFd);
        	stConTbl[dConTblIdx].dSfd = -1;
        	stConTbl[dConTblIdx].uiCliIP = 0;
        	stConTbl[dConTblIdx].cSockBlock = 0x00;

        	stConTbl[dConTblIdx].Reserv[0] = 0;

			FD_CLR( dSocketFd, &gstReadfds );

        	memcpy((char*)&stWd, (char*)&gstWritefds, sizeof(fd_set));

        	if( gdNumWfds >0 && FD_ISSET( dSocketFd, &stWd ) )
        	{
            	log_print(LOGN_CRI,"SENDFAIL WRITE FDSET CLEAR");
            	FD_CLR( dSocketFd, &gstWritefds );
            	gdNumWfds--;
        	}

			return -1;
		}
		else {
			log_print( LOGN_INFO, "MESSAGE SEND OK : usBodyLen[%d] usTotLen[%d]",
					stSpkt.head.usBodyLen, stSpkt.head.usTotLen );
		}
	}
	else if( stConTbl[dConTblIdx].dSfd > 0 && stConTbl[dConTblIdx].cSockBlock == 0x01 ) {
		AppendNonSendMsg( dConTblIdx, stSpkt.head.usTotLen, (char*)&stSpkt );
	}

	return 1;
}


int dWriteLog( st_MsgQ stMesgQ )
{
	FILE		*fp_RST;

	int     dRet;
    time_t  tNow;
    struct tm *stTM;

	char	szLogFileName[128];
	char    szLogFilePath[256];
	char    szResultFile[256];
	char    szInputString[4096];

	char	*szTempString;
	char	szFirstStr[12];
	char	szLogPath[8];


	tNow = time(&tNow);
    stTM = localtime(&tNow);

	/*
	* HIS DIRECTORY CAHNGE
	*/

	szTempString = strstr( (char *)stMesgQ.szBody, "\n" );
	if( szTempString == NULL ) {
		;
	}

	sscanf( szTempString, "%s", szFirstStr );

	if( szFirstStr[0] == '*' || szFirstStr[0] == '#' ) {
		// CASE : ALARM
		sscanf( szTempString, "%*s %s", szFirstStr );

		if( szFirstStr[0] == 'A' ) {
			sprintf(szLogFilePath, "%s%02d/%02d/%s",
				DEF_HIS_PATH, stTM->tm_mon+1, stTM->tm_mday, DEF_FLT_LOG_PATH );

			sprintf( szLogFileName, "FLT_RESULT_%02d.dat", stTM->tm_hour );
			sprintf( szLogPath, "%s", DEF_FLT_LOG_PATH );
		}
	}
	else {
		// CASE : STAT OR TRACE
		if( szFirstStr[0] == 'S' ) {
			if( szFirstStr[1] == '5' ) {
				// CASE : STAT
                sprintf(szLogFilePath, "%s%02d/%02d/%s",
                    DEF_HIS_PATH, stTM->tm_mon+1, stTM->tm_mday, DEF_STAT_LOG_PATH );

                sprintf( szLogFileName, "STAT_RESULT_%02d.dat", stTM->tm_hour );
                sprintf( szLogPath, "%s", DEF_STAT_LOG_PATH );
			}
			else {
				// CASE : TRACE & ETC
                sprintf(szLogFilePath, "%s%02d/%02d/%s",
                    DEF_HIS_PATH, stTM->tm_mon+1, stTM->tm_mday, DEF_STS_LOG_PATH );

                sprintf( szLogFileName, "STS_RESULT_%02d.dat", stTM->tm_hour );
                sprintf( szLogPath, "%s", DEF_STS_LOG_PATH );
			}
		}
	}

	sprintf(szResultFile, "%s%s", szLogFilePath, szLogFileName );

	if( (fp_RST = fopen( szResultFile, "a" )) == NULL )
    {
        if( errno == ENOENT )
        {
            dRet = dMake_Dir( stTM, szLogPath );
            if( dRet < 0 )
            {
                log_print(LOGN_CRI,"[ERROR] MAKE COND RESULT FILE PATH [%d] [%s]", errno, strerror(errno));
                log_print(LOGN_CRI,"[ERROR] [%s]", szResultFile );
                return -1;
            }

            if( (fp_RST = fopen( szResultFile, "a" )) == NULL )
            {
                log_print(LOGN_CRI,"[ERROR] COND RESULT FILE OPEN ERROR2 [%d] [%s]", errno, strerror(errno));
                log_print(LOGN_CRI,"[ERROR] [%s]", szResultFile );
                return -1;
            }
        }
        else
        {
            log_print(LOGN_CRI,"[ERROR] COND RESULT FILE OPEN ERROR1 [%d] [%s]", errno, strerror(errno));
            log_print(LOGN_CRI,"[ERROR] [%s]", szResultFile );
            return -1;
        }
    }

	stMesgQ.szBody[stMesgQ.usBodyLen] = 0x00;

	sprintf(szInputString, "#C ");
	strcat(szInputString, (char*)stMesgQ.szBody );
	strcat(szInputString, "\n" );

	fprintf( fp_RST,"%s", szInputString );

    fclose( fp_RST );

	return 1;
}
