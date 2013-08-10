/*******************************************************************************
			DQMS Project

	Author   : Park Si Woo
	Section  : ALMD
	SCCS ID  : @(#)almd_mem.c	1.7
	Date     : 09/24/03
	Revision History :
        '01.  7. 21     Initial
        '03.  1. 15     Initial

	Description :

	Copyright (c) uPRESTO 2005
*******************************************************************************/

/***** A.1 * File Include *******************************/

/* SYS HEADER */
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>
/* LIB HEADER */
#include "loglib.h"
#include "ipclib.h"
#include "utillib.h"
#include "filedb.h"
/* PRO HEADER */
#include "procid.h"
#include "sockio.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
#include "almstat.h"
/* LOC HEADER */
#include "almd_func.h"
#include "almd_mem.h"

extern st_MSG_BUF    gstMsgBuf[MAX_FDSET2];

extern int errno;
extern int gdIndex;

extern int vdLogLevel;      // LOG WRITE LEVEL
extern int mY_HOST_ID;
extern int gdSINMSQid;
#ifdef _DEBUG_
unsigned int gdCnt[32];
time_t		tdCurTime[32], tdOldTime[32];
#endif

int log_print(int dIndex, char *fmt, ...);


int dMergeBuffer( int dIdx, int dSfd, char* szTcpRmsg, int dRecvLen )
{
    long long llMagicNo = MAGIC_NUMBER;
    int     dPktSize;
    int     dRet;
    st_MngPkt    stRpkt;


    if( (dRecvLen + gstMsgBuf[dIdx].dWidx) < MAX_MNG_PKT_BUFSIZE ) {
        memcpy( &gstMsgBuf[dIdx].szBuf[gstMsgBuf[dIdx].dWidx], &szTcpRmsg[0], dRecvLen );

        gstMsgBuf[dIdx].dWidx += dRecvLen;

        if( gstMsgBuf[dIdx].dWidx < 8 )
            return 0;

        if( *(long long*)&gstMsgBuf[dIdx].szBuf[0] == llMagicNo ) {
            if( gstMsgBuf[dIdx].dWidx > MNG_PKT_HEAD_SIZE ) {

                stRpkt.head = *(pst_MngHead)&gstMsgBuf[dIdx].szBuf[0];
                dPktSize = stRpkt.head.usBodyLen + MNG_PKT_HEAD_SIZE;

                if( dPktSize <= gstMsgBuf[dIdx].dWidx ) {
                    memcpy( &stRpkt.data[0], &gstMsgBuf[dIdx].szBuf[MNG_PKT_HEAD_SIZE], stRpkt.head.usBodyLen );

                    dRet = dRcvPkt_Handle( dIdx, dSfd, stRpkt );

					gstMsgBuf[dIdx].dWidx -= dPktSize ;

					if( dRet < 0 )
						return -1;

                    if( gstMsgBuf[dIdx].dWidx > 0 ) {
                        memcpy( &gstMsgBuf[dIdx].szBuf[0], &gstMsgBuf[dIdx].szBuf[dPktSize],
								 gstMsgBuf[dIdx].dWidx  );

                        while( gstMsgBuf[dIdx].dWidx > MNG_PKT_HEAD_SIZE ) {
                            if( *(long long*)&gstMsgBuf[dIdx].szBuf[0] == llMagicNo ) {
                                stRpkt.head = *(pst_MngHead)&gstMsgBuf[dIdx].szBuf[0];

                                dPktSize = stRpkt.head.usBodyLen;

                                if( (dPktSize+MNG_PKT_HEAD_SIZE) <= gstMsgBuf[dIdx].dWidx ) {
                                    memcpy( &stRpkt.data[0], &gstMsgBuf[dIdx].szBuf[MNG_PKT_HEAD_SIZE], dPktSize );

                                    gstMsgBuf[dIdx].dWidx -= MNG_PKT_HEAD_SIZE+dPktSize;

                                    dRet = dRcvPkt_Handle( dIdx, dSfd, stRpkt );
									if( dRet < 0 )
										return -1;

                                    if( gstMsgBuf[dIdx].dWidx > 0 ) {
                                        memcpy( &gstMsgBuf[dIdx].szBuf[0], &gstMsgBuf[dIdx].szBuf[MNG_PKT_HEAD_SIZE+dPktSize], gstMsgBuf[dIdx].dWidx );
                                    }
                                }
                                else {
                                    break;
                                }
                            }
                            else {
                    			log_print(LOGN_CRI, "[ERROR] INVALID MAGIC NUMBER FAIL : dBufinIdx[%d]",
												 gstMsgBuf[dIdx].dWidx );
								return -1;
                            }
                        }
                    }
                    else {
                        gstMsgBuf[dIdx].dWidx = 0;
                    }

                    return 1;
                }
            }
        }
        else {
            log_print( LOGN_CRI, "[ERROR] INVALID MAGIC NUMBER FAIL");
            gstMsgBuf[dIdx].dWidx = 0;
            return -1;
        }
    }
    else {
        log_print( LOGN_CRI, "[ERROR] BUFFER OVERFLOW INVOKE RECVLEN[%d] WIDX[%d] MAX_BUF_SZ[%d]",
						  dRecvLen, gstMsgBuf[dIdx].dWidx, MAX_MNG_PKT_BUFSIZE );
        return -2;
    }

    return 0;
}


int dRcvPkt_Handle( int dIdx, int dSsfd, st_MngPkt stMyPkt )
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
		return -1;
	}
	return 0;
}

int dSend_FSTAT(pst_MsgQ pstMsg, char *data)
{
	unsigned char   *pNODE;
	pst_MsgQSub		pstMsgSub;
	int				dRet;

	if( (dRet = dGetNode(&pNODE, &pstMsg)) < 0 ){
		log_print(LOGN_CRI,LH" FAILED IN dGetNode(QMON), errno=%d:%s",
				LT,errno,strerror(errno));
		return -1;
	}

	pstMsgSub			= (pst_MsgQSub)pstMsg;
	pstMsgSub->usType	= DEF_SVC;
	pstMsgSub->usSvcID	= SID_SVC;
	pstMsgSub->usMsgID	= MID_STAT_LOAD;

	if(pstMsg->ucNTAFID != 0)
		pstMsg->usBodyLen = sizeof(st_NTAF);
	else
		pstMsg->usBodyLen = sizeof(st_NTAM);

	pstMsg->dMsgQID		= 0;
	pstMsg->ucProID		= SEQ_PROC_ALMD;
	pstMsg->usRetCode	= 0;
	pstMsg->llIndex		= gdIndex++;

	memcpy(pstMsg->szBody, data, pstMsg->usBodyLen);

	if( (dRet = dMsgsnd(SEQ_PROC_FSTAT, pNODE)) < 0 ){
		log_print(LOGN_CRI, LH" FAILED IN dMsgsnd(FSTAT)",LT);
		return -1;
	}

    log_print(LOGN_DEBUG,"[SUCCESS] MSGSND FSTAT TYPE=%d,SID=%d,MID=%d,BODY=%d,TAFID=%d,TAMID=%d",
        pstMsgSub->usType, pstMsgSub->usSvcID, pstMsgSub->usMsgID,
        pstMsg->usBodyLen, pstMsg->ucNTAFID, pstMsg->ucNTAMID );

    return 0;
}

int dSend_Traffic_FSTAT(st_MsgQ *pstMsg)
{

	unsigned char   *pNODE;
    st_MsgQSub	*pstMsgSub;
    int			dRet;

	if( (dRet = dGetNode(&pNODE, &pstMsg)) < 0 ){
		log_print(LOGN_CRI,LH" FAILED IN dGetNode(QMON), errno=%d:%s",
				LT,errno,strerror(errno));
		return -1;
	}

    pstMsgSub			= (st_MsgQSub*)pstMsg;
    pstMsgSub->usType	= DEF_SVC;
    pstMsgSub->usSvcID	= SID_SVC;
    pstMsgSub->usMsgID	= MID_STAT_TRAFFIC;

	if( (pstMsg->ucNTAMID == 0) && (pstMsg->ucNTAFID != 0))
		pstMsg->usBodyLen = sizeof(st_NtafStatList);
	else
		log_print(LOGN_CRI, "F=%s:%s.%d: INVALID Traffic Info(ucNTAMID[%hu], ucNTAFID[%hu])", __FILE__, __FUNCTION__, __LINE__,
			pstMsg->ucNTAMID, pstMsg->ucNTAFID);

    pstMsg->dMsgQID		= 0;
	pstMsg->ucProID		= SEQ_PROC_ALMD;
	pstMsg->usRetCode	= 0;
    pstMsg->llIndex		= gdIndex++;

	if( (dRet = dMsgsnd(SEQ_PROC_FSTAT, pNODE)) < 0 ){
		log_print(LOGN_CRI, LH" FAILED IN dMsgsnd(CHSMD)",LT);
		return -1;
	}

    log_print(LOGN_DEBUG,"[SUCCESS] MSGSND FSTAT TYPE=%d, SID=%d, MID=%d, BODY=%d, TAFID=%d, TAMID=%d",
        pstMsgSub->usType, pstMsgSub->usSvcID, pstMsgSub->usMsgID, pstMsg->usBodyLen, pstMsg->ucNTAFID, pstMsg->ucNTAMID);

    return 0;
}


void Send_CondMess(int systype, int sysno, int loctype, char invtype, short invno, char almstatus, char oldalm, long long llLoadVal )
{
	int				dRet;
	char			levelstr[8];
	unsigned char *pNODE;
	st_Qentry       smesg;
	st_almsts       almsts;
	pst_MsgQ		pstSndMsg;
	pst_MsgQSub		pstSndSub;

	if( (dRet = dGetNode(&pNODE, &pstSndMsg)) < 0 ){
		log_print(LOGN_CRI,LH" FAILED IN dGetNode(QMON), errno=%d:%s",
				LT,errno,strerror(errno));
		exit(0);
	}

	smesg.mtype	= 0;
	levelstr[0]	= 0;

	memset(&smesg.mtext, 0x00, 4096);
	memset(&almsts, 0x00, sizeof(st_almsts));

	almsts.ucSysType	= systype;
	if(systype == SYSTYPE_TAM)
		almsts.ucSysNo	= mY_HOST_ID;
	else
		almsts.ucSysNo	= sysno+1;

	almsts.ucLocType		= loctype;
	almsts.ucInvType		= invtype;
	almsts.ucInvNo			= invno;
	almsts.ucAlmLevel		= almstatus;
	almsts.ucOldAlmLevel	= oldalm;
	almsts.llLoadVal		= llLoadVal;

	log_print(LOGN_DEBUG, "SYSTYPE:[0x%02x] SYSNO:[0x%02x] LOCTYPE:[0x%02x] INVTYPE:[0x%02x] INVNO:[0x%02x] ALM:[0x%02x] OLD:[0x%02x]",
		almsts.ucSysType, almsts.ucSysNo, almsts.ucLocType, almsts.ucInvType, almsts.ucInvNo, almsts.ucAlmLevel, almsts.ucOldAlmLevel );

	time(&almsts.tWhen);

	memcpy(smesg.mtext, &almsts, sizeof(st_almsts));

	pstSndSub = (pst_MsgQSub)&pstSndMsg->llMType;
	pstSndSub->usType	= DEF_SYS;
	pstSndSub->usSvcID	= SID_STATUS;
	pstSndSub->usMsgID	= MID_CONSOL;

	util_makenid(SEQ_PROC_ALMD, &pstSndMsg->llNID);

	if(systype == SYSTYPE_TAM)
		pstSndMsg->ucNTAFID	= 0;
	else if( systype == SYSTYPE_TAF )
		pstSndMsg->ucNTAFID	= sysno+1;

    pstSndMsg->ucProID = SEQ_PROC_ALMD;
    pstSndMsg->llIndex = gdIndex;
    gdIndex++;

    pstSndMsg->dMsgQID	= 0;
    pstSndMsg->usBodyLen	= sizeof(st_almsts);
    pstSndMsg->usRetCode	= 0;
    memcpy(pstSndMsg->szBody, smesg.mtext, pstSndMsg->usBodyLen);

	if( (dRet = dMsgsnd(SEQ_PROC_COND, pNODE)) < 0 ){
		log_print(LOGN_CRI, LH" FAILED IN dMsgsnd(COND)",LT);
		exit(0);
	}
}
