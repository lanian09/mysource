/*******************************************************************************
                 NTAS Project 

   Author   : Hwang Woo-Hyoung
   Section  : NTAS Project
   SCCS ID  : @(#)tcpif_msg.c (V1.0)
   Date     : 2/15/04
   Revision History :
        '03.    01. 15. initial

   Description:

   Copyright (c) ABLEX 2004
*******************************************************************************/

/**
 *	Include headers
 */
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <sys/select.h>
#include <unistd.h>
#include <sys/time.h>
#include <netinet/in.h>

// TOP
#include "common_stg.h"
#include "commdef.h"
#include "msgdef.h"
#include "filter.h"
#include "sockio.h"

// LIB
#include "loglib.h"
#include "utillib.h"
#include "nsocklib.h"

// .
#include "ci_log_util.h"
#include "ci_log_sock.h"
#include "ci_log_func.h"

/**
 *	Declare var.
 */
extern st_Flt_Info		*flt_info;
extern int				g_dLogInStatus;   /* 0->log-out, 1->login*/
extern int				g_dServerSocket;
extern fd_set			g_readset;
extern fd_set			g_writeset;
extern int				g_dNumOfSocket;
extern st_ClientInfo	g_stClientInfo;



/**
 *	Implement func.
 */

/** dCheckLogType function. 
 *
 *	Checking LOG Type
 *
 *  @return     int
 *
 *  @exception  
 *  @note       Nothing
 **/
int dCheckLogType(int type, char *pdata)
{
	int 				dRet = 1;
	LOG_HTTP_TRANS		*pLOGHTTP;
	LOG_PAGE_TRANS		*pLOGPAGE;
	LOG_TCP_SESS 		*pLOGTCP;

	switch(type)
	{
		case LOG_HTTP_TRANS_DEF_NUM:
			pLOGHTTP = (LOG_HTTP_TRANS *)pdata;
			switch(pLOGHTTP->usSvcL4Type) 
			{
				case L4_WAP20:
				case L4_FB:
					dRet = 0;
					break;
				default:
					dRet = 1;
			}
			break;
		case LOG_PAGE_TRANS_DEF_NUM:
			pLOGPAGE = (LOG_PAGE_TRANS *)pdata;
			switch(pLOGPAGE->LastSvcL4Type) 
			{
				case L4_FV_FB:
				case L4_FV_EMS:
				case L4_FV_IV:
				case L4_EMS:
				case L4_P_EMS:
				case L4_EMS_NO:
				case L4_DN_2G:
				case L4_DN_2G_NODN:
				case L4_DN_VOD:
				case L4_DN_VOD_NODN:
				case L4_DN_JAVA:
				case L4_OMA_DN:
				case L4_OMA_DN_2G:
				case L4_OMA_DN_VOD:
				case L4_OMA_DN_WIPI:
				case L4_WIPI:
				case L4_WIPI_ONLINE:
				case L4_MMS_UP:
				case L4_MMS_UP_NODN:
				case L4_MMS_DN:
				case L4_MMS_DN_NODN:
				case L4_MMS_NEW:
				case L4_TODAY:
				case L4_WIDGET:
					dRet = 0;
					break;
				default:
					dRet = 1;
			}
			break;
		case LOG_TCP_SESS_DEF_NUM:
			pLOGTCP = (LOG_TCP_SESS *)pdata;
			switch(pLOGTCP->usSvcL4Type) 
			{
				case L4_DNS:
					dRet = 0;
					break;
				default:
					dRet = 1;
			}
			break;
		default: 
			log_print( LOGN_CRI, "### IMPOSSIBLE TYPE: %d", type);
	}

	return dRet;
}


/*******************************************************************************

*******************************************************************************/
int dSndTraceMsgProc(char *pTraceHdr, char *pPacket, int TraceHdrLen, int PacketLen)
{
	int					dRet;
	UCHAR				ucSvcID, ucMsgID;
	UCHAR       		szBody[MAX_MSGBODY_SIZE];

	st_NTAFTHeader		*pstHeader;

	INT64				llMagicNum;

	if( g_dLogInStatus == DEF_LOGOUT ) {
		if( dCheckSock() < 0 ) {
			log_print( LOGN_CRI, "dSndMsgProc : failed in dCheckSock" );
			return -1;
		}
	}

	ucSvcID 	= SID_LOG;
	ucMsgID 	= st_TraceMsgHdr_DEF_NUM;

	llMagicNum 	= MAGIC_NUMBER;

	/* st_NTAFHeader */
	pstHeader = (st_NTAFTHeader *)&szBody[0];

	pstHeader->ucSvcID 			= ucSvcID;
	pstHeader->ucMsgID 			= ucMsgID;

	pstHeader->ucNTAFID 		= flt_info->stTmfInfo.usTmfID;
	pstHeader->llMagicNumber 	= llMagicNum;
	pstHeader->usTotlLen 		= NTAFT_HEADER_LEN + TraceHdrLen + PacketLen;
	pstHeader->usBodyLen 		= TraceHdrLen + PacketLen;
	pstHeader->llIndex 			= 0;

	memcpy(&szBody[NTAFT_HEADER_LEN], pTraceHdr, TraceHdrLen);
	memcpy(&szBody[NTAFT_HEADER_LEN + TraceHdrLen], pPacket, PacketLen);

	log_print( LOGN_INFO, "[MAGIC]:[%lld] [SYSNO]:[%d] [SID]:[%s:%d] [MID]:[%s:%d] [LEN]:[%d]", 
			pstHeader->llMagicNumber, pstHeader->ucNTAFID, 
			PrintSID(ucSvcID), ucSvcID, PrintMID(ucMsgID), ucMsgID, pstHeader->usTotlLen);

	dRet = writeSocket(g_dServerSocket, (char*)&szBody[0], pstHeader->usTotlLen);
	if( dRet < 0 ) {
		log_print( LOGN_WARN, "dSndMsgProc : Failed in writeSocket" );
		return -1;
	}

	return 0;
}

/*******************************************************************************

*******************************************************************************/
int dSndMsgProc(int type, int size, char *data)
{
	int					dRet;
	UCHAR				ucSvcID, ucMsgID;
	UCHAR       		szBody[MAX_MSGBODY_SIZE];

	st_NTAFTHeader		*pstHeader;

	INT64				llMagicNum;

	if( g_dLogInStatus == DEF_LOGOUT ) {
		if( dCheckSock() < 0 ) {
			log_print( LOGN_CRI, "dSndMsgProc : failed in dCheckSock" );
			return -1;
		}
	}

	if(type==LOG_PISIGNAL_DEF_NUM) 
		type = LOG_SIGNAL_DEF_NUM;

	ucSvcID 	= SID_LOG;
	ucMsgID 	= type;

	llMagicNum 	= MAGIC_NUMBER;

	/* st_NTAFHeader */
	pstHeader = (st_NTAFTHeader *)&szBody[0];

	pstHeader->ucSvcID 			= ucSvcID;
	pstHeader->ucMsgID 			= ucMsgID;

	pstHeader->ucNTAFID 		= flt_info->stTmfInfo.usTmfID;
	pstHeader->llMagicNumber 	= llMagicNum;
	pstHeader->usTotlLen 		= NTAFT_HEADER_LEN + size;
	pstHeader->usBodyLen 		= size;
	pstHeader->llIndex 			=0;

	memcpy(&szBody[NTAFT_HEADER_LEN], data, size);

	log_print( LOGN_INFO, "[MAGIC]:[%lld] [SYSNO]:[%d] [SID]:[%s:%d] [MID]:[%s:%d] [LEN]:[%d]", 
			pstHeader->llMagicNumber, pstHeader->ucNTAFID, 
			PrintSID(ucSvcID), ucSvcID, PrintMID(ucMsgID), ucMsgID, pstHeader->usTotlLen);

	dRet = writeSocket(g_dServerSocket, (char*)&szBody[0], pstHeader->usTotlLen);
	if( dRet < 0 ) {
		log_print( LOGN_WARN, "dSndMsgProc : Failed in writeSocket" );
		return -1;
	}

	return 0;
}


/*******************************************************************************

*******************************************************************************/
int dCheckNProc_Event()
{
    struct timeval  timeout;
    fd_set          fd_read_tmp;
    fd_set          fd_write_tmp;
	int				dRet;

	timeout.tv_sec 	= 0;
    timeout.tv_usec = 1;

	memcpy( &fd_read_tmp, &g_readset, sizeof(fd_set) );
    memcpy( &fd_write_tmp, &g_writeset, sizeof(fd_set) );
		
	if( (dRet = select(g_dNumOfSocket, &fd_read_tmp, &fd_write_tmp, (fd_set *)0, &timeout)) > 0 ) {

		if( FD_ISSET(g_dServerSocket, &fd_write_tmp) ) /* 재전송 처리*/
			write_proc( g_dServerSocket );

		if( FD_ISSET(g_dServerSocket, &fd_read_tmp) ) {

       		dRet = dRecvPacket( g_dServerSocket );
           	if( dRet < 0 ) {

           		FD_CLR(g_dServerSocket, &g_readset);
				FD_CLR(g_dServerSocket, &g_writeset);
           		close(g_dServerSocket);
				
				g_stClientInfo.dLastFlag	= 0;
				g_stClientInfo.szBuf[0] 	= 0x00;
				g_stClientInfo.dBufSize 	= 0;
				g_stClientInfo.dFront 		= 0;
				g_stClientInfo.dRear 		= 0;
				g_stClientInfo.szWBuf[0] 	= 0x00;
				g_dNumOfSocket 				= 0;
/*
               	g_stClientInfo.dType 		= 0;
				g_stClientInfo.szBuf[0] 	= 0x00;
				g_stClientInfo.dBufSize 	= 0;
				g_stClientInfo.front 		= 0;
				g_stClientInfo.rear 		= 0;
				g_stClientInfo.outbuff[0] 	= 0x00;
				g_dNumOfSocket 				= 0;
*/
               	log_print( LOGN_DEBUG, "dCheckNProc_Event : Failed in dRecvPacket" ); 

				g_dLogInStatus = DEF_LOGOUT;
	
				return -1;
			}
		}
	}
	else if( dRet == 0 ) 
   		return 0;
	else {
   		log_print( LOGN_CRI, "dCheckNProc_Event : FAILED IN SELECT, [%d:%s]", 
							errno, strerror(errno) );
		return -1;
	}

	return 0;

}


/*******************************************************************************

*******************************************************************************/
int dCheckSock()
{
   int     dLoop = 0;
   int     dRet = 0;

    do {
        dRet = dInit_CILOG_Client(&g_dServerSocket);
        if( dRet < 0 ) {
            close(g_dServerSocket);
            usleep(0);
            dLoop++;
        }
        else {
            g_dNumOfSocket = g_dServerSocket + 1;
            g_dLogInStatus = DEF_CONNECT;

            g_stClientInfo.szBuf[0] 	= 0x00;
            g_stClientInfo.dBufSize 	= 0;
            g_stClientInfo.dFront 		= 0;
            g_stClientInfo.dRear 		= 0;
            g_stClientInfo.szWBuf[0] 	= 0x00;

            return 0;
        }

    } while( dLoop < IPC_RETRY_CNT );

    log_print( LOGN_CRI, "dCheckSock : FAILED IN dInit_CILOG_Client UNTIL IPC_RETRY_CNT" );

	g_dLogInStatus = DEF_LOGOUT;

    return -1;

}
