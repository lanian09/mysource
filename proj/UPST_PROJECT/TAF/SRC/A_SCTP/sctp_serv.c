/******************************************************************************* 
        @file   sctp_serv.c
 *      - A_SCTP 프로세스를 초기화 하는 함수들
 *
 *      Copyright (c) 2006~ by Upresto Inc. Korea
 *      All rights reserved
 *
 *      $Id: sctp_serv.c,v 1.2 2011/09/06 02:07:44 dcham Exp $
 *
 *      @Author     $Author: dcham $
 *      @version    $Revision: 1.2 $
 *      @date       $Date: 2011/09/06 02:07:44 $
 *      @ref        sctp_serv.c
 *
 *      @section    Intro(소개)
 *      - A_SCTP 메인 서비스 함수들.
 *
 *      @section    Requirement
 *
*******************************************************************************/

/* INCLUDE ********************************************************************/

/* SYS HEADER */
#include <ctype.h>
/* LIB HEADER */
#include "mems.h"
#include "cifo.h"
#include "gifo.h"
#include "nifo.h"
#include "loglib.h"
/* PRO HEADER */
#include "procid.h"
#include "common_stg.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */
#include "sctpstack.h"
#include "sctp_serv.h"
#include "sctp_func.h"

/* VARIABLES ******************************************************************/
char *szChunkTypeName[] = {
"SCTP_DATA",
"SCTP_INIT",
"SCTP_INIT_ACK",
"SCTP_SACK",
"SCTP_HEART",
"SCTP_HEART_ACK",
"SCTP_ABORT",
"SCTP_SHUTDOWN",                
"SCTP_SHUTDOWN_ACK",                
"SCTP_ERROR",                           
"SCTP_COOKIE_ECHO",
"SCTP_COOKIE_ACK",                              
"SCTP_ECNE",                                        
"SCTP_CWR",                                             
"SCTP_SHUTDOWN_COM"                                         
};

/* VARIABLES ( External ) *****************************************************/
extern stMEMSINFO		*gpMEMSINFO;
extern stCIFO			*gpCIFO;
extern OFFSET			gdOffset;
/* FUNCTION *******************************************************************/
#define WIDTH   16
int dump_DebugString(char *debug_str, char *s, int len)
{
	char buf[BUFSIZ],lbuf[BUFSIZ],rbuf[BUFSIZ];
	unsigned char *p;
	int line,i;

	log_print(LOGN_DEBUG,"### %s",debug_str);
	p =(unsigned char *) s;
	for(line = 1; len > 0; len -= WIDTH,line++) {
		memset(lbuf,0,BUFSIZ);
		memset(rbuf,0,BUFSIZ);

		for(i = 0; i < WIDTH && len > i; i++,p++) {
			sprintf(buf,"%02x ",(unsigned char) *p);
			strcat(lbuf,buf);
			sprintf(buf,"%c",(!iscntrl(*p) && *p <= 0x7f) ? *p : '.');
			strcat(rbuf,buf);
		}
		log_print(LOGN_DEBUG,"%04x: %-*s    %s",line - 1,WIDTH * 3,lbuf,rbuf);
	}
	return line;
}

/*******************************************************************************
 ANALYZE SCTP HEADER & SWITCH BY TYPE
*******************************************************************************/
int dAnalyzeSCTP(Capture_Header_Msg *pCAPHEAD, INFO_ETH *pINFOETH, UCHAR *pNODE)
{
	int					dRet;
	UINT				uiPaddingLen;
	UINT				uiSysInfo;
	UCHAR				ucChunkCnt = 0;
	UINT				uiCheckSum;
	USHORT				usSCTPDataLen;
	USHORT				usPosition = 0;
	USHORT				usTempSys;
    U8                  *pucSCTPData;
	U8					*pucETHData;
	UCHAR				szIP1[4];
	UCHAR				szIP2[4];
	UCHAR				szSysInfo[4];

	st_SCTPHeader		stSCTPHeader;
	st_SCTPChunkHeader	stSCTPChunkHeader;
	pst_SCTPCOMMON		pstSCTPCOMMON;
	pst_SCTPDATA		pstSCTPData;

	ASSO_DATA			stMMDB;
	PASSO_DATA			pstMMDB;

#if 0	
	pst_SMsgQSub 		pstMsgQSub;

	pstMsgQSub = (pst_SMsgQSub)&pstMsgQ->llMType;

	if( pstMsgQSub->usType == DEF_SVC ) {
		if( (pstMsgQSub->usSvcID != SID_SESS_INFO) || (pstMsgQSub->usMsgID != MID_SESS_DATA) ) {
			log_print( LOGN_CRI, "INVALID SVCID:%u,%u MSGID:%u,%u", 
					 SID_SESS_INFO, pstMsgQSub->usSvcID, MID_SESS_DATA, pstMsgQSub->usMsgID );
			return -1;
		}
	}
	else {
		log_print( LOGN_CRI, "INVALID TYPE:%u,%u", DEF_SVC, pstMsgQSub->usType );
		return -2;
	}
#endif

	if( pCAPHEAD->bRtxType==DEF_FROM_CLIENT) {
		usTempSys = (pINFOETH->stUDPTCP.seq);
        memcpy( &szSysInfo[0], &usTempSys, 2 );

        usTempSys = (pINFOETH->stUDPTCP.ack);
        memcpy( &szSysInfo[2], &usTempSys, 2 );
	} else {
		usTempSys = (pINFOETH->stUDPTCP.ack);
        memcpy( &szSysInfo[0], &usTempSys, 2 );

        usTempSys = (pINFOETH->stUDPTCP.seq);
        memcpy( &szSysInfo[2], &usTempSys, 2 );
	}
	//uiSysInfo = atoi(szSysInfo);
	memcpy( &uiSysInfo, szSysInfo, 4 );

	memcpy( &szIP1[0], &pINFOETH->stIP.dwSrcIP, 4 );
	memcpy( &szIP2[0], &pINFOETH->stIP.dwDestIP, 4 );

	/* SET SCTP DATA TOTAL LENGTH & POSITION */
	usSCTPDataLen = pINFOETH->stIP.wTotalLength - pINFOETH->stIP.wIPHeaderLen;

	/* VALIDATION CHECK FOR SCTP PACKET PROTOCOL IN IP HEADER */
	if( pINFOETH->stIP.ucProtocol != 0x84 ) {
		log_print( LOGN_CRI, "[ERROR] INVALID SCTP PROTOCOL:%u", pINFOETH->stIP.ucProtocol );
		return -2;
	}

	/* GET SCTP DATA POSITION */
//	pucSCTPData = &pstMsgQ->szBody[DEF_PACKHDR_SIZE + pINFOETH->stIP.usIPHeaderLen + 14];
	pucSCTPData = (UCHAR *)nifo_get_value(gpMEMSINFO, ETH_DATA_NUM, gdOffset) + pINFOETH->stIP.wIPHeaderLen + 14;
	pucETHData	= (UCHAR *)nifo_get_value(gpMEMSINFO, ETH_DATA_NUM, gdOffset);

	/* GET SCTP COMMON HEADER */
	pstSCTPCOMMON = (pst_SCTPCOMMON)pucSCTPData;

	if( pCAPHEAD->bRtxType==DEF_FROM_CLIENT) {
        stSCTPHeader.usSrcPort 	= TOUSHORT(pstSCTPCOMMON->SrcPort);
        stSCTPHeader.usDestPort = TOUSHORT(pstSCTPCOMMON->DestPort);
    } else {
        stSCTPHeader.usSrcPort 	= TOUSHORT(pstSCTPCOMMON->DestPort);
        stSCTPHeader.usDestPort = TOUSHORT(pstSCTPCOMMON->SrcPort);
    }
	stSCTPHeader.uiVerifiTag 	= TOULONG(pstSCTPCOMMON->VerifiTag);
	stSCTPHeader.uiCheckSum 	= TOULONG(pstSCTPCOMMON->CheckSum);

	/* SET PKTINFO */
	pINFOETH->stUDPTCP.wSrcPort		= TOUSHORT(pstSCTPCOMMON->SrcPort);
	pINFOETH->stUDPTCP.wDestPort 	= TOUSHORT(pstSCTPCOMMON->DestPort);

	log_print( LOGN_DEBUG, "## NEW PACKET[%s] : S:%3u.%3u.%3u.%3u:%5u D:%3u.%3u.%3u.%3u:%5u VERI:0x%08x SYS:%u,%u,%u LEN:%u",
						(pCAPHEAD->bRtxType==DEF_FROM_CLIENT)?"UP":"DN",
						szIP1[3], szIP1[2], szIP1[1], szIP1[0], stSCTPHeader.usSrcPort, 
						szIP2[3], szIP2[2], szIP2[1], szIP2[0], stSCTPHeader.usDestPort, 
						stSCTPHeader.uiVerifiTag, pINFOETH->stUDPTCP.seq, pINFOETH->stUDPTCP.ack, uiSysInfo, usSCTPDataLen );

//	dump_DebugString("HEXA", pucSCTPData, usSCTPDataLen);

	/* CHECK FOR SHECKSUM */
	memset( &pstSCTPCOMMON->CheckSum[0], 0x00, 4 );
	
    uiCheckSum = uiCrc32( pucSCTPData, usSCTPDataLen, ~0L );
    if( uiCheckSum != stSCTPHeader.uiCheckSum ) {
        log_print( LOGN_DEBUG, "CRC ERROR CAL:0x%04x PKT:0x%04x", uiCheckSum, stSCTPHeader.uiCheckSum );
        return -3;
    }

	usPosition  += DEF_SCTPHEADER_SIZE;
    pucSCTPData += usPosition;

	/* CHECK ASSOCIATION IN MMDB */
	if(  pCAPHEAD->bRtxType == DEF_FROM_CLIENT) {
		stMMDB.stKey.usSrcPort  = stSCTPHeader.usSrcPort;
    	stMMDB.stKey.usDestPort = stSCTPHeader.usDestPort;
	}
	else {
		stMMDB.stKey.usDestPort	= stSCTPHeader.usSrcPort;
		stMMDB.stKey.usSrcPort	= stSCTPHeader.usDestPort;
	}

	stMMDB.stKey.uiSysInfo	= uiSysInfo;
	
	dRet = dSetMMDB( &stMMDB.stKey, &pstMMDB );
	if( dRet < 0 ) {
		log_print( LOGN_DEBUG, "[%s][%s.%d] FAIL IN dSetMMDB dRet:%d", __FILE__, __FUNCTION__, __LINE__, dRet );

		/* INSERT NEW ASSOCIATION */
		dRet = dInsertMMDB( &stMMDB, &pstMMDB );
		if( dRet < 0 ) {
			log_print( LOGN_CRI, "[%s][%s.%d] FAIL IN dInsertMMDB dRet:%d", __FILE__, __FUNCTION__, __LINE__, dRet );
			return -4;
		}
		else {
			log_print( LOGN_INFO, "NEW ASSOCIATION INSERTED SP:%5u DP:%5u SY:%u",
							   stMMDB.stKey.usSrcPort, stMMDB.stKey.usDestPort, stMMDB.stKey.uiSysInfo );
 
			/* INITIALIZE MMDB */
			InitASSODATA( pstMMDB );

			/* SET CREATE TIME & UPDATE TIME */
			pstMMDB->stSessTime.tv_sec  	= pCAPHEAD->curtime;
            pstMMDB->stSessTime.tv_usec 	= pCAPHEAD->ucurtime;
            pstMMDB->stUpdateTime.tv_sec    = pCAPHEAD->curtime;
            pstMMDB->stUpdateTime.tv_usec   = pCAPHEAD->ucurtime;
		}
	}
	else {
		log_print( LOGN_INFO, "CURRENT ASSOCIATION SP:%5u DP:%5u SYS:%3u",
			  			   stMMDB.stKey.usSrcPort, stMMDB.stKey.usDestPort, stMMDB.stKey.uiSysInfo );

		/* CHECK FOR VERIFICATION TAG */
		switch( pCAPHEAD->bRtxType ) 
		{
		case DEF_FROM_CLIENT:	/* UP */
			if( pstMMDB->uiReqVerifTag == 0 )
				pstMMDB->uiReqVerifTag = stSCTPHeader.uiVerifiTag;
			else if( pstMMDB->uiReqVerifTag != stSCTPHeader.uiVerifiTag ) {
				log_print( LOGN_DEBUG, "INVALID VERIFICATION TAG MEM:0x%08x CUR:0x%08x",
                                  	pstMMDB->uiReqVerifTag, stSCTPHeader.uiVerifiTag );
				pstMMDB->uiReqVerifTag = stSCTPHeader.uiVerifiTag;
            }

			break;
		case DEF_FROM_SERVER:	/* DOWN */
			if( pstMMDB->uiResVerifTag == 0 )
				pstMMDB->uiResVerifTag = stSCTPHeader.uiVerifiTag;
			else if( pstMMDB->uiResVerifTag != stSCTPHeader.uiVerifiTag ) {
				log_print( LOGN_DEBUG, "INVALID VERIFICATION TAG MEM:0x%08x CUR:0x%08x",
                                  	pstMMDB->uiResVerifTag, stSCTPHeader.uiVerifiTag );
				pstMMDB->uiResVerifTag = stSCTPHeader.uiVerifiTag;
            }
	
			break;
		default:
			log_print( LOGN_CRI, "NEVER HAPPENED CASE FUNC:%s.%d", __FUNCTION__, __LINE__ );
			return -7;

			break;
		}

		/* SET UPDATE TIME */
		pstMMDB->stUpdateTime.tv_sec 	= pCAPHEAD->curtime;
		pstMMDB->stUpdateTime.tv_usec	= pCAPHEAD->ucurtime;
	}

	/* LOOPING IN SCTP DATA */
	while( (usSCTPDataLen - usPosition ) > 0 ) {

		/* GET SCTP CHUNK HEADER  */
    	pstSCTPData = (pst_SCTPDATA)pucSCTPData;
    	usPosition 	+= DEF_SCTPCHUNKHEADER_SIZE;
    	pucSCTPData += DEF_SCTPCHUNKHEADER_SIZE;

    	stSCTPChunkHeader.ucType       = pstSCTPData->Type;
    	stSCTPChunkHeader.ucFlag       = pstSCTPData->Flag;
    	stSCTPChunkHeader.usChunkLen   = TOUSHORT(pstSCTPData->ChunkLen);

		ucChunkCnt++;

		log_print( LOGN_DEBUG, "## NEW CHUNK[%3u] TYPE:%s LEN:%u",
							ucChunkCnt, szChunkTypeName[stSCTPChunkHeader.ucType], stSCTPChunkHeader.usChunkLen );

		/* FOR CHECK CHUNK TYPE INFO */
		if( ((stSCTPChunkHeader.ucType && DEF_CHECK_CHUNKTYPE) == DEF_CHUNK_SKIP1 ) ||
			((stSCTPChunkHeader.ucType && DEF_CHECK_CHUNKTYPE) == DEF_CHUNK_SKIP2 ) ) {

			/* MUST SKIP THIS CHUNK */
			log_print( LOGN_WARN, "SKIP THIS CHUNK TYPE:0x%02x", stSCTPChunkHeader.ucType );

			/* ALL CHUNK MUST 4 BYTE */
			if( (stSCTPChunkHeader.usChunkLen%4) == 0 )
            	uiPaddingLen = 0;
        	else
            	uiPaddingLen = (4 - (stSCTPChunkHeader.usChunkLen%4));

			usPosition 	+= (stSCTPChunkHeader.usChunkLen - DEF_SCTPCHUNKHEADER_SIZE + uiPaddingLen);
			pucSCTPData += (stSCTPChunkHeader.usChunkLen - DEF_SCTPCHUNKHEADER_SIZE + uiPaddingLen);

			continue;
		}

		/* SWITCH BY CHUNK TYPE */
    	switch( stSCTPChunkHeader.ucType )
		{
		case DEF_SCTP_DATA:
			dRet = dAnalyzeDATA( pstMMDB, pucSCTPData, pucETHData, &stSCTPChunkHeader, pINFOETH, pCAPHEAD );
			if( dRet < 0 )
				log_print( LOGN_DEBUG, "FAIL IN dAnalyzeDATA dRet:%d", dRet );

			break;

		case DEF_SCTP_SACK:
			dRet = dAnalyzeSACK( pstMMDB, pucSCTPData, &stSCTPChunkHeader, pINFOETH, pCAPHEAD );
			if( dRet < 0 )
                log_print( LOGN_DEBUG, "FAIL IN dAnalyzeSACK dRet:%d", dRet );

            break;

		case DEF_SCTP_INIT:
		case DEF_SCTP_INIT_ACK:
			dRet = dAnalyzeINIT( pstMMDB, pucSCTPData, &stSCTPChunkHeader );
			if( dRet < 0 )
				log_print( LOGN_DEBUG, "FAIL IN dAnalyzeINIT dRet:%d", dRet );
			else if( dRet == 0 ) {

				/* NOT CONTAIN IP PARAMETER IN SCTP DATA */
				if( stSCTPChunkHeader.ucType == DEF_SCTP_INIT )
					pstMMDB->uiSrcIPFst = pINFOETH->stIP.dwSrcIP; 
				else
					pstMMDB->uiDestIPFst = pINFOETH->stIP.dwDestIP;
			}

			break;

		case DEF_SCTP_HEART:
		case DEF_SCTP_HEART_ACK:
			dRet = dAnalyzeHEART( pstMMDB, pucSCTPData, &stSCTPChunkHeader );
			if( dRet < 0 )
                log_print( LOGN_DEBUG, "FAIL IN dAnalyzeHEART dRet:%d", dRet );
			

			break;

		case DEF_SCTP_ABORT:
			dRet = dAnalyzeABORT( pstMMDB, pucSCTPData, &stSCTPChunkHeader );
			if( dRet < 0 )
				log_print( LOGN_DEBUG, "FAIL IN dAnalyzeABORT dRet:%d", dRet );

			break;

		/* SET CLOSE STATUS BY CHUNK TYPE */
		case DEF_SCTP_SHUTDOWN:
		case DEF_SCTP_SHUTDOWN_ACK:
		case DEF_SCTP_SHUTDOWN_COM:
			dRet = dAnalyzeSHUTDOWN( pstMMDB, pucSCTPData, &stSCTPChunkHeader, pCAPHEAD ); 
			if( dRet < 0 )
				log_print( LOGN_DEBUG, "FAIL IN dAnalyzeSHUTDOWN dRet:%d", dRet );

            break;

		case DEF_SCTP_ERROR:
			dRet = dAnalyzeERROR( pstMMDB, pucSCTPData, &stSCTPChunkHeader );
			if( dRet < 0 )
				log_print( LOGN_DEBUG, "FAIL IN dAnalyzeERROR dRet:%d", dRet );

			break;

		case DEF_SCTP_COOKIE_ECHO:
		case DEF_SCTP_COOKIE_ACK:
			dRet = dAnalyzeCOOKIE( pstMMDB, pucSCTPData, &stSCTPChunkHeader );
			if( dRet < 0 )
				log_print( LOGN_DEBUG, "FAIL IN dAnalyzeCOOKIE dRet:%d", dRet );

			break;

		case DEF_SCTP_ECNE:
			log_print( LOGN_DEBUG, "RESERVED FOR EXPLICIT CONGESTION NOTIFICATION ECHO" );

			break;

		case DEF_SCTP_CWR:
			log_print( LOGN_DEBUG, "RESERVED FOR CONGESTION WINDOW REDUCED" );

			break;

		default:
			log_print( LOGN_CRI, "INVALID SCTP TYPE:%u", stSCTPChunkHeader.ucType );
			break;
		}

		/* ALL CHUNK MUST 4 BYTE */
		if( (stSCTPChunkHeader.usChunkLen%4) == 0 )
			uiPaddingLen = 0;
		else
			uiPaddingLen = (4 - (stSCTPChunkHeader.usChunkLen%4));

        usPosition 	+= (stSCTPChunkHeader.usChunkLen - DEF_SCTPCHUNKHEADER_SIZE + uiPaddingLen);
        pucSCTPData += (stSCTPChunkHeader.usChunkLen - DEF_SCTPCHUNKHEADER_SIZE + uiPaddingLen);
	}

	if( pstMMDB->uiSrcIPFst == 0 ) {
		/* CASE : CREATE ASSOCIATION WITHOUT INIT MESSAGES */
		pstMMDB->uiSrcIPFst 	= pINFOETH->stIP.dwSrcIP;
		pstMMDB->uiDestIPFst 	= pINFOETH->stIP.dwDestIP;
	}

	nifo_node_delete(gpMEMSINFO, pNODE);

	return 1;
}


/*******************************************************************************

*******************************************************************************/
int dAnalyzeDATA( PASSO_DATA pstMMDB, UCHAR *pucData, UCHAR *pcuETHData, pst_SCTPChunkHeader pstChunkHeader, INFO_ETH *pINFOETH, Capture_Header_Msg *pCAPHEAD )
{
	int				dRet;
	int				dSeqProcID;
	UINT			uiStackCount;
	UINT			uiStackIndex;
	UINT			uiTSN, uiProtoID;
	USHORT			usStreamID, usStreamSeq;
	USHORT			usWriteLen;

	pst_DATA		pstDataHeader;

	UCHAR			ucFragStatus = 0;

	OFFSET			offset;
	UCHAR			*pucBuffer;
	UCHAR 			*pucDataNode;


	pstDataHeader = (pst_DATA)pucData;
	uiTSN		= TOULONG(pstDataHeader->TSN);
	usStreamID	= TOUSHORT(pstDataHeader->STREAMID);
	usStreamSeq	= TOUSHORT(pstDataHeader->STREAMSEQ);
	uiProtoID	= TOULONG(pstDataHeader->PROTOID);

	log_print( LOGN_INFO, "DATA CHUNK : TSN:%10u STMID:%5u STMSEQ:%5u PROTOID:%u",
					   uiTSN, usStreamID, usStreamSeq, uiProtoID );

	/* INSERT SCTP CHUNK DATA INTO NIFO & MMDB */
	if( pCAPHEAD->bRtxType == DEF_FROM_CLIENT) {
		uiStackCount = pstMMDB->uiReqCount;
		uiStackIndex = pstMMDB->uiReqLast;
	}
	else {
		uiStackCount = pstMMDB->uiResCount;
		uiStackIndex = pstMMDB->uiResLast;
	}

	/* CHECK CHUNK FLAG : ORDERED INFO */
	if( (pstChunkHeader->ucFlag & DEF_NONORDER) == DEF_NONORDER ) {
		/* NONORDERED STREAM */
	}

	/* CHECK CHUNK FLAG : FRAGMENATION INFO */
	ucFragStatus = (pstChunkHeader->ucFlag & DEF_ONE_PACKET); 

	log_print( LOGN_INFO, "[%s] ResCount:%u ResIndex:%u ReqCount:%u ReqIndex:%u",
					   (pCAPHEAD->bRtxType == DEF_FROM_CLIENT)?"UP":"DN",
                       pstMMDB->uiResCount, pstMMDB->uiResFirst,
                       pstMMDB->uiReqCount, pstMMDB->uiReqFirst );

	pucDataNode = nifo_node_alloc(gpMEMSINFO);
	if(pucDataNode == NULL) {
		log_print( LOGN_CRI, "[ERROR] FAIL INi FUN:%s.%d nifo_node_alloc!!", __FUNCTION__, __LINE__ );
		return -1;
	}
	offset = nifo_offset(gpMEMSINFO, pucDataNode);

	/* COPY PACKET INFO DATA IN NEW DATANODE NIFO */
	usWriteLen = (pstChunkHeader->usChunkLen - DEF_SCTPCHUNKHEADER_SIZE - DEF_DATA_SIZE);
	pINFOETH->stUDPTCP.wDataLen	= usWriteLen;
	
	/* ADD Capture_Header_Msg */
	pucBuffer = nifo_tlv_alloc(gpMEMSINFO, pucDataNode, CAP_HEADER_NUM, CAP_HRD_LEN, DEF_MEMSET_OFF);
	if( pucBuffer == 0 ) {
		log_print( LOGN_CRI, "[ERROR] FAIL IN FUNC:%s.%d nifo_tlv_alloc!!", __FUNCTION__, __LINE__ );
		nifo_node_delete(gpMEMSINFO, pucDataNode);
		return -2;
	}
	memcpy( pucBuffer, pCAPHEAD, CAP_HRD_LEN );
	
	/* ADD INFO_ETH */
	pucBuffer = nifo_tlv_alloc(gpMEMSINFO, pucDataNode, INFO_ETH_NUM, INFO_ETH_SIZE, DEF_MEMSET_ON);
	if( pucBuffer == 0 ) {
		log_print( LOGN_CRI, "[ERROR] FAIL IN FUNC:%s.%d nifo_tlv_alloc!!", __FUNCTION__, __LINE__ );
		nifo_node_delete(gpMEMSINFO, pucDataNode);
		return -3;
	}
	memcpy( pucBuffer, pINFOETH, INFO_ETH_SIZE );

	/* ADD SCTP DATA */
	pucBuffer = nifo_tlv_alloc(gpMEMSINFO, pucDataNode, SCTP_DATA_NUM, 1500, DEF_MEMSET_ON);
	if( pucBuffer == 0 ) {
		log_print( LOGN_CRI, "[ERROR] FAIL IN FUNC:%s.%d nifo_tlv_alloc!!", __FUNCTION__, __LINE__ );
		nifo_node_delete(gpMEMSINFO, pucDataNode);
		return -4;
	}
	memcpy( pucBuffer, pucData+DEF_DATA_SIZE, usWriteLen);

	pucBuffer = nifo_tlv_alloc(gpMEMSINFO, pucDataNode, ETH_DATA_NUM, pCAPHEAD->datalen, DEF_MEMSET_ON);
	if( pucBuffer == 0 ) {
        log_print( LOGN_CRI, "[ERROR] FAIL IN FUNC:%s.%d nifo_tlv_alloc!!", __FUNCTION__, __LINE__ );
        nifo_node_delete(gpMEMSINFO, pucDataNode);
        return -4;
    }
	memcpy( pucBuffer, pcuETHData, pCAPHEAD->datalen ); 

	/* DECISION FOR SEND QID : BY PORT & PROTOCOL ID */
	if( pstMMDB->stKey.usDestPort == DEF_DIAMETER_PORT || pstMMDB->stKey.usSrcPort == DEF_DIAMETER_PORT)
		dSeqProcID = SEQ_PROC_A_DIAMETER;
	else {
		log_print( LOGN_WARN, "CANNOT SEND SCTP DATA DSTPORT:%u PORTO:%u", pstMMDB->stKey.usDestPort, uiProtoID );
		nifo_node_delete(gpMEMSINFO, pucDataNode);
		return -5;
	}
	
	/* SEND SCTP DATA NODE */
	dRet = gifo_write( gpMEMSINFO, gpCIFO, SEQ_PROC_A_SCTP, dSeqProcID, nifo_offset(gpMEMSINFO, pucDataNode) );
	if( dRet < 0 ) {
		log_print( LOGN_CRI, "[ERROR] FAIL IN FUNC:%s.%d gifo_write dRet:%d", __FUNCTION__, __LINE__, dRet );
		nifo_node_delete(gpMEMSINFO, pucDataNode);
	}
	else {
		log_print( LOGN_INFO, "SEND DATA to SeqProcID = %d", dSeqProcID );
	}

#if 0
	/* MAKE MESSAGE QUEUE */
	memset( &stMsgQ, 0x00, DEF_MSGQ_SIZE );

	pstMsgQSub = (pst_SMsgQSub)&stMsgQ.llMType;
    pstMsgQSub->usType 	= DEF_SVC;
    pstMsgQSub->usSvcID = SID_SESS_INFO;
	pstMsgQSub->usMsgID = MID_DATA_DIAMETER;

	dMakeNID( SEQ_PROC_A_SCTP, &stMsgQ.llNID );

	stMsgQ.ucProID		= SEQ_PROC_A_SCTP;
	stMsgQ.dMsgQID		= gdMyQID;

	stMsgQ.usBodyLen	= DEF_PACKHDR_SIZE + pstPKTINFO->stTCPHeader.usDataLen;

	memcpy( &stMsgQ.szBody[0], pstPKTINFO, DEF_PACKHDR_SIZE );
	memcpy( &stMsgQ.szBody[DEF_PACKHDR_SIZE], pucData+DEF_DATA_SIZE, pstPKTINFO->stTCPHeader.usDataLen ); 

	/* SEND MESSAGE QUEUE */
	dRet = dSendMsg( dSendQID, &stMsgQ );
	if( dRet < 0 ) {
		log_print( LOGN_INFO, "[%s][%s.%d] FAIL IN dSendMsg dRet:%d",
						   __FILE__, __FUNCTION__, __LINE__, dRet );
		return -2;
	}
#endif	

	return 1;


/* FOR TEST : NOT CHECK SACK
	if( uiStackCount == 0 ) {
		//INSERT IN MMDB STACK LIST
		stStackKey.uiTSN		= uiTSN;
		stStackKey.uiReserved	= 0;

		dRet = dGetStack( pstMMDB, pstPKTINFO->stTCPHeader.usRtxType, &stStackKey, &pstStackNode );
		if( dRet < 0 ) {
			log_print( LOGN_CRI, "[ERRROR] FAIL IN dGetStack FUN:%S.%d dRet:%d", __FUNCTION__, __LINE__, dRet );
			return -5;
		}

		//SET STACK NODE INFO
		pstStackNode->stDataTime.tv_sec		= pstPKTINFO->tCapTime.tv_sec;
		pstStackNode->stDataTime.tv_usec	= pstPKTINFO->tCapTime.tv_usec;
		pstStackNode->usStreamID         	= usStreamID;
        pstStackNode->usStreamSeq        	= usStreamSeq;
		pstStackNode->ucFragStatus			= ucFragStatus;
		pstStackNode->dSendQID				= dSendQID;
	}
	else {
		//SEARCH STACK IN MMDB STACK LIST
		for( i=0; i<uiStackCount; i++ ) {

			dRet = dSetStackIndex( uiStackIndex, &pstStackNode );
			if( dRet < 0 ) {
				log_print( LOGN_CRI, "[ERROR] FAIL IN FUN:%s.%d dSetStackIndex dRet:%d",
								  __FUNCTION__, __LINE__, dRet );
				return -6;
			}
			else
				log_print( LOGN_INFO, "[INFO] STACK_TSN:%u CURR_TSN:%u", 
								   pstStackNode->stSKey.uiTSN, uiTSN );

			if( pstStackNode->stSKey.uiTSN < uiTSN ) {
	
				//INSERT NEXT NODE
				dRet = dGetStackOnly( &pstNewStackNode );
				if( dRet < 0 ) {
					log_print( LOGN_CRI, "[ERROR] FAIL IN FUN:%s.%d dGetStackOnly dRet:%d",
									  __FUNCTION__, __LINE__, dRet );
					return -7;
				}
				else {
					//INITILIZE STACK NODE
					InitStackNode( pstNewStackNode );
				}

				pstNewStackNode->stSKey.uiTSN 		= uiTSN;
				pstNewStackNode->stSKey.uiReserved 	= 0;

				dRet = dAddStackNext( pstMMDB, pstPKTINFO->stPKTCAP.ucRtxType, pstStackNode, pstNewStackNode );
				if( dRet < 0 ) {
					log_print( LOGN_CRI, "[ERROR] FAIL IN FUN:%s.%d dAddStackNext dRe:%d",
									  __FUNCTION__, __LINE__, dRet );
					return -8;
				}

				// INPUT NEW NODE DATA INFO
				pstNewStackNode->stDataTime.tv_sec	= pstPKTINFO->stPKTCAP.lCapTime;
        		pstNewStackNode->stDataTime.tv_usec	= pstPKTINFO->stPKTCAP.lCapMTime;
				pstNewStackNode->uiChunkOffset		= dDataOffset;
				pstNewStackNode->usStreamID			= usStreamID;
				pstNewStackNode->usStreamSeq		= usStreamSeq;
				pstNewStackNode->ucFragStatus		= ucFragStatus;
				pstNewStackNode->dSendQID			= dSendQID;
					
				break;
			}
			else if( pstStackNode->stSKey.uiTSN == uiTSN ) {
				
				//RETRANSMISSION
				if( pstStackNode->ucRetranInfo == 1 ) {

					//MOVE PREV NODE BECAUSE CURRENT NODE IS RETRASMISSION NODE
					uiStackIndex = pstStackNode->uiStackPrev;
				}
				else {

					//SET RETRANSMISSION INFO IN NODE
					pstStackNode->ucRetranInfo = 1;

					dRet = dGetStackOnly( &pstNewStackNode );
					if( dRet < 0 ) {
                        log_print( LOGN_CRI, "[ERROR] FAIL IN FUN:%s.%d dGetStackOnly dRet:%d",
                                          __FUNCTION__, __LINE__, dRet );
                        return -9;
                    }
					else {
						//INITILIZE STACK NODE
                    	InitStackNode( pstNewStackNode );
					}

					pstNewStackNode->stSKey.uiTSN       = uiTSN;
                    pstNewStackNode->stSKey.uiReserved  = 0;

                    dRet = dAddStackPrev( pstMMDB, pstPKTINFO->stPKTCAP.ucRtxType, pstStackNode, pstNewStackNode );
                    if( dRet < 0 ) {
                        log_print( LOGN_CRI, "[ERROR] FAIL IN FUN:%s.%d dAddStackNext dRe:%d",
                                          __FUNCTION__, __LINE__, dRet );
                        return -10;
                    }

					//INPUT NEW NODE DATA INFO
					pstNewStackNode->stDataTime.tv_sec	= pstPKTINFO->stPKTCAP.lCapTime;
        			pstNewStackNode->stDataTime.tv_usec	= pstPKTINFO->stPKTCAP.lCapMTime;
                    pstNewStackNode->uiChunkOffset  	= dDataOffset;
					pstNewStackNode->usStreamID         = usStreamID;
                	pstNewStackNode->usStreamSeq        = usStreamSeq;
					pstNewStackNode->ucFragStatus       = ucFragStatus;
					pstNewStackNode->dSendQID           = dSendQID;

					break;
				}
			}
			else { //pstStackNode->stSKey.uiTSN > uiTSN
			
				//MOVE PREV 
				uiStackIndex = pstStackNode->uiStackPrev;
				if( uiStackIndex == 0 ) {
					
					//CURRENT STACK NODE IS FIRST & INSERT PREV STACK NODE
					dRet = dGetStackOnly( &pstNewStackNode );
                	if( dRet < 0 ) {
                    	log_print( LOGN_CRI, "[ERROR] FAIL IN FUN:%s.%d dGetStackOnly dRet:%d",
                                      	  __FUNCTION__, __LINE__, dRet );
                    	return -11;
                	}
					else {
						//INITILIZE STACK NODE
                    	InitStackNode( pstNewStackNode );
					}

                	pstNewStackNode->stSKey.uiTSN       = uiTSN;
                	pstNewStackNode->stSKey.uiReserved  = 0;

                	dRet = dAddStackPrev( pstMMDB, pstPKTINFO->stPKTCAP.ucRtxType, pstStackNode, pstNewStackNode );
                	if( dRet < 0 ) {
                    	log_print( LOGN_CRI, "[ERROR] FAIL IN FUN:%s.%d dAddStackNext dRe:%d",
                                      	  __FUNCTION__, __LINE__, dRet );
                    	return -12;
                	}

					//INPUT NEW NODE DATA INFO
					pstNewStackNode->stDataTime.tv_sec	= pstPKTINFO->stPKTCAP.lCapTime;
        			pstNewStackNode->stDataTime.tv_usec	= pstPKTINFO->stPKTCAP.lCapMTime;
                	pstNewStackNode->uiChunkOffset  	= dDataOffset;
					pstNewStackNode->usStreamID         = usStreamID;
                    pstNewStackNode->usStreamSeq        = usStreamSeq;
					pstNewStackNode->ucFragStatus       = ucFragStatus;
					pstNewStackNode->dSendQID           = dSendQID;

					break;
				}
			}
		}
	}

	return 0;
*/
}


/*******************************************************************************

*******************************************************************************/
int dOperationSACK( PASSO_DATA pstMMDB, Capture_Header_Msg *pCAPHEAD, UINT uiSackTSN )
{
	int			dRet, i;	
	UINT        uiStackCount, uiStackIndex;
	UCHAR		ucDelRTX;

	pSTACK_LIST 		pstStackNode;
	
	if( pCAPHEAD->bRtxType == DEF_FROM_CLIENT) {
        uiStackCount = pstMMDB->uiResCount;
        uiStackIndex = pstMMDB->uiResFirst;

		ucDelRTX = DEF_FROM_SERVER;
    }
    else {
        uiStackCount = pstMMDB->uiReqCount;
        uiStackIndex = pstMMDB->uiReqFirst;

		ucDelRTX = DEF_FROM_CLIENT;
    }

	log_print( LOGN_INFO, "[%s] RSCNT:%u RSIDX:%u RQCNT:%u RQIDX:%u",
					   (pCAPHEAD->bRtxType==DEF_FROM_CLIENT)?"UP":"DN",
					   pstMMDB->uiResCount, pstMMDB->uiResFirst,
					   pstMMDB->uiReqCount, pstMMDB->uiReqFirst );

	/* FOR TEST : NOT CHECK SACK */
	return 0;

	/* SACK OPERATION !! */
    for( i=0; i<uiStackCount; i++ ) {

        dRet = dSetStackIndex( uiStackIndex, &pstStackNode );
        if( dRet < 0 ) {
            log_print( LOGN_CRI, "[ERROR] FAIL IN FUN:%s.%d dSetStackIndex dRet:%d", __FUNCTION__, __LINE__, dRet );
            return -1;
        }
        else
            log_print( LOGN_INFO, "[INFO] STACK_TSN:%u CUMUL_TSN:%u", pstStackNode->stSKey.uiTSN, uiSackTSN );

        if( pstStackNode->stSKey.uiTSN <= uiSackTSN ) {

			/* SET SACK TIME */
			pstStackNode->stSackTime.tv_sec		= pCAPHEAD->curtime;
			pstStackNode->stSackTime.tv_usec	= pCAPHEAD->ucurtime;
		
			/* SEND CURRENT STACK NODE TO SERVICE BLOCK *
			pucNode = nifo_ptr( gpMEMSINFO, pstStackNode->uiChunkOffset );

			* SEND MESSAGE QUEUE FOR OFFSET *
			dRet = nifo_msg_write( gpMEMSINFO, pstStackNode->dSendQID, pucNode );
			if( dRet < 0 ) {
				log_print( LOGN_CRI, "[ERROR] FAIL IN FUNC:%s.%d nifo_msg_write dRet:%d",
								  __FUNCTION__, __LINE__, dRet );
			}
			else {
				log_print( LOGN_INFO, "SEND DATA  : TSN:%10u QID:%d", 
								   pstStackNode->stSKey.uiTSN, pstStackNode->dSendQID );
			}
			*/
	
			uiStackIndex = pstStackNode->uiStackNext;

			dRet = dDelStack( pstMMDB, ucDelRTX, pstStackNode );
			if( dRet < 0 ) {
				log_print( LOGN_DEBUG, "[ERROR] FAIL IN FUN%s.%d dSetStackIndex dRet:%d",
                              __FUNCTION__, __LINE__, dRet );
			}

			continue;
        }
        else {
			/* STACK NODE NOT YET RECEIVED SACK */
			log_print( LOGN_INFO, "NOY YET RECIEVED SACK TSN:%u SACL%u CNT:%u CUR:%u", 
					pstStackNode->stSKey.uiTSN, uiSackTSN, uiStackCount, i+1 );
			break;
        }
    }

	return 0;
}


/*******************************************************************************
 SACK에서의 처리와 동일하지만, 특정 TSN에 대해서만 처리.
*******************************************************************************/
int dOperationSACKGap( PASSO_DATA pstMMDB, Capture_Header_Msg *pCAPHEAD, UINT uiSackTSN )
{
    int         dRet, i;
    UINT        uiStackCount, uiStackIndex;
    UCHAR       ucDelRTX;

    pSTACK_LIST pstStackNode;


    if( pCAPHEAD->bRtxType == DEF_FROM_CLIENT) {
        uiStackCount = pstMMDB->uiResCount;
        uiStackIndex = pstMMDB->uiResFirst;

        ucDelRTX = DEF_FROM_SERVER;
    }
    else {
        uiStackCount = pstMMDB->uiReqCount;
        uiStackIndex = pstMMDB->uiReqFirst;

        ucDelRTX = DEF_FROM_CLIENT;
    }

    log_print( LOGN_INFO, "RTX:%u ResCount:%u ResIndex:%u ReqCount:%u ReqIndex:%u",
                       pCAPHEAD->bRtxType,
                       pstMMDB->uiResCount, pstMMDB->uiResFirst,
                       pstMMDB->uiReqCount, pstMMDB->uiReqFirst );

    /* SACK OPERATION !! */
    for( i=0; i<uiStackCount; i++ ) {

        dRet = dSetStackIndex( uiStackIndex, &pstStackNode );
        if( dRet < 0 ) {
            log_print( LOGN_CRI, "[ERROR] FAIL IN FUN:%s.%d dSetStackIndex dRet:%d", __FUNCTION__, __LINE__, dRet );
            return -1;
        }
        else
            log_print( LOGN_INFO, "[INFO] STACK_TSN:%u CUMUL_TSN:%u", pstStackNode->stSKey.uiTSN, uiSackTSN );

        if( pstStackNode->stSKey.uiTSN == uiSackTSN ) {

            /* SET SACK TIME */
            pstStackNode->stSackTime.tv_sec     = pCAPHEAD->curtime;
            pstStackNode->stSackTime.tv_usec    = pCAPHEAD->ucurtime;

            /* SEND CURRENT STACK NODE TO SERVICE BLOCK *
            pucNode = nifo_ptr( gpMEMSINFO, pstStackNode->uiChunkOffset );

            dRet = nifo_msg_write( gpMEMSINFO, , pucNode );
			if( dRet < 0 ) {
                log_print( LOGN_CRI, "[ERROR] FAIL IN FUNC:%s.%d nifo_msg_write dRet:%d",
                                  __FUNCTION__, __LINE__, dRet );
            }
            */

            uiStackIndex = pstStackNode->uiStackNext;

            dRet = dDelStack( pstMMDB, ucDelRTX, pstStackNode );
            if( dRet < 0 ) {
                log_print( LOGN_DEBUG, "[ERROR] FAIL IN FUN%s.%d dSetStackIndex dRet:%d",
                              __FUNCTION__, __LINE__, dRet );
            }

			break;
        }
        else
			uiStackIndex = pstStackNode->uiStackNext;
    }

    return 0;
}


/*******************************************************************************

*******************************************************************************/
int dAnalyzeSACK( PASSO_DATA pstMMDB, UCHAR *pucData, pst_SCTPChunkHeader pstChunkHeader, INFO_ETH *pINFOETH, Capture_Header_Msg *pCAPHEAD)
{
	int			dRet, i, j;
	USHORT		usOffset = 0;
	UINT		uiCumulTSN, uiRwnd;
	USHORT		usNoGapAck, usNoDupl;

	UINT		uiDuplTSN;
	USHORT		usGapAckStart, usGapAckEnd;

	pst_SACK	pstSACKHeader;


	/* GET SACK HEADER MENDANTORY */
	pstSACKHeader = (pst_SACK)pucData;
	uiCumulTSN	= TOULONG(pstSACKHeader->CumulTSN);
	uiRwnd		= TOULONG(pstSACKHeader->RWND);
	usNoGapAck	= TOUSHORT(pstSACKHeader->NoGapAck);
	usNoDupl	= TOUSHORT(pstSACKHeader->NoDupl);

	log_print( LOGN_INFO, "SACK CHUNK : TSN:%10u RWND:%10u NoGap:%5u NoDupl:%5u", uiCumulTSN, uiRwnd, usNoGapAck, usNoDupl );

	usOffset += DEF_SACK_SIZE;

	for( i=0; i<usNoGapAck; i++ ) {
		memcpy( &usGapAckStart, &pucData[usOffset], DEF_USHORT_SIZE );
		usOffset += DEF_USHORT_SIZE;

		memcpy( &usGapAckEnd, &pucData[usOffset], DEF_USHORT_SIZE );
		usOffset += DEF_USHORT_SIZE;

		log_print( LOGN_INFO, "[INFO] GAP BLOCK INFO ST:%u ED:%u", usGapAckStart, usGapAckEnd );

		/* WHAT OPERATION !! */
		for( j=0; j<(usGapAckEnd-usGapAckStart+1); j++ ) {
			dRet = dOperationSACKGap( pstMMDB, pCAPHEAD, uiCumulTSN+usGapAckStart+j );
			if( dRet < 0 )
				log_print( LOGN_DEBUG, "[ERROR][%d] FAIL IN FUNC:%s.%d dOperationSACK dRet:%d", j, __FUNCTION__, __LINE__, dRet );	
		} 
	}

	for( i=0; i<usNoDupl; i++ ) {
		memcpy( &uiDuplTSN, &pucData[usOffset], DEF_UINT_SIZE );
		usOffset += DEF_UINT_SIZE;

		/* WHAT OPERATION !! */


	}

	dRet = dOperationSACK( pstMMDB, pCAPHEAD, uiCumulTSN );
	if( dRet < 0 ) {
		log_print( LOGN_DEBUG, "[ERROR] FAIL IN FUNC:%s.%d dOperationSACK dRet:%d", __FUNCTION__, __LINE__, dRet );
		return -1;
	} 

	return 0;
}


/*******************************************************************************

*******************************************************************************/
int dAnalyzeINIT( PASSO_DATA pstMMDB, UCHAR *pucData, pst_SCTPChunkHeader pstChunkHeader )
{
	int			dRet;
	USHORT		usOffset = 0;
	USHORT		usParaType, usParaLen, usIPCnt = 0;
	USHORT		usChunkLen;
	pst_INIT	pstINITHeader;

	pstINITHeader = (pst_INIT)pucData;

	log_print( LOGN_INFO, "INIT_%d TAG:0x%08lu RW:%lu OUT:%hu IN:%hu TSN:%lu",
			pstChunkHeader->ucType,
			TOULONG(pstINITHeader->INITTag), TOULONG(pstINITHeader->RWND),
			TOUSHORT(pstINITHeader->OutStream), TOUSHORT(pstINITHeader->InStream),
			TOULONG(pstINITHeader->InitTSN) );

	if( pstChunkHeader->ucType == DEF_SCTP_INIT ) {
		pstMMDB->uiResVerifTag 	= TOULONG(pstINITHeader->INITTag);
		pstMMDB->uiReqInitTSN 	= TOULONG(pstINITHeader->InitTSN);

		/* SETUP STATUS INFO */
		pstMMDB->ucSetupStatus	= (pstMMDB->ucSetupStatus | DEF_INIT );
	}
	else {
		pstMMDB->uiReqVerifTag 	= TOULONG(pstINITHeader->INITTag);
        pstMMDB->uiResInitTSN 	= TOULONG(pstINITHeader->InitTSN);

		/* SETUP STATUS INFO */
		pstMMDB->ucSetupStatus	= (pstMMDB->ucSetupStatus | DEF_INIT_ACK );
	}

	/* 현재 데이터는 CHUNK HEADER 부분이 제외된 부분이기 때문에 */
	usChunkLen = pstChunkHeader->usChunkLen;
	usChunkLen -= DEF_SCTPCHUNKHEADER_SIZE;

	usOffset += DEF_INIT_SIZE;

	while ( usChunkLen <= usOffset ) {
		
		usParaType = pucData[usOffset];
		usOffset += DEF_USHORT_SIZE;

		dRet = dCheckOptionParameter( usParaType );
		if( dRet < 0 ) {
			/* CASE : SKIP PARAMETER */
			usParaLen 	= pucData[usOffset];
        	usOffset 	+= usParaLen;
 
			continue;
		}

		switch( usParaType ) 
		{
		case DEF_PARA_IP4:
			usParaLen = pucData[usOffset];
			usOffset += DEF_USHORT_SIZE;

			if( pstChunkHeader->ucType == DEF_SCTP_INIT ) {
				if( usIPCnt == 0 )
					memcpy( &pstMMDB->uiSrcIPFst, &pucData[usOffset], DEF_UINT_SIZE );
				else
					memcpy( &pstMMDB->uiSrcIPSnd, &pucData[usOffset], DEF_UINT_SIZE );
				
				usIPCnt++;
			}
			else {
				if( usIPCnt == 0 )
                    memcpy( &pstMMDB->uiDestIPFst, &pucData[usOffset], DEF_UINT_SIZE );
                else
                    memcpy( &pstMMDB->uiDestIPSnd, &pucData[usOffset], DEF_UINT_SIZE );
                
                usIPCnt++;
			}

			usOffset += (usParaLen - DEF_USHORT_SIZE*2); 

			break;

		default:
			log_print( LOGN_DEBUG, "INIT_%u PARAMETER:%u", pstChunkHeader->ucType, usParaType );
			usParaLen = pucData[usOffset];
        	usOffset += (usParaLen - DEF_USHORT_SIZE) ;

			break;
		}
	}

	return (int)usIPCnt;
}


/*******************************************************************************

*******************************************************************************/
int dAnalyzeSHUTDOWN( PASSO_DATA pstMMDB, UCHAR *pucData, pst_SCTPChunkHeader pstChunkHeader, Capture_Header_Msg *pCAPHEAD )
{
	int			dRet;
	UINT		uiCumulTSN;

	if( pstChunkHeader->ucType == DEF_SCTP_SHUTDOWN ) {
		pstMMDB->ucSetupStatus = (pstMMDB->ucCloseStatus | DEF_SHUTDOWN);

		memcpy( &uiCumulTSN, pucData, DEF_UINT_SIZE );

		/* FOLLOW SACK ROUTINE */
		dRet = dOperationSACK( pstMMDB, pCAPHEAD, uiCumulTSN );
		if( dRet < 0 ) {
        	log_print( LOGN_DEBUG, "[ERROR] FAIL IN FUNC:%s.%d dOperationSACK dRet:%d", __FUNCTION__, __LINE__, dRet );
        	return -1;
    	}
	}
	else if( pstChunkHeader->ucType == DEF_SCTP_SHUTDOWN_COM ) {
		pstMMDB->ucSetupStatus = (pstMMDB->ucCloseStatus | DEF_SHUTDOWN_COM);

		/* CLOSE ASSOCIATION */
		dRet = dCloseAssociation( pstMMDB );
		if( dRet < 0 )
			log_print( LOGN_DEBUG, "NERVER OCCURRED CASE!!" );	

	}
	else if( pstChunkHeader->ucType == DEF_SCTP_SHUTDOWN_ACK ) {
		pstMMDB->ucSetupStatus = (pstMMDB->ucCloseStatus | DEF_SHUTDOWN_ACK);
	}

	return 0;
}

/*******************************************************************************

*******************************************************************************/
int dAnalyzeHEART( PASSO_DATA pstMMDB, UCHAR *pucData, pst_SCTPChunkHeader pstChunkHeader )
{
	int			dRet;
	USHORT		usParaType;
	USHORT		usParaLen;
	USHORT		usOffset = 0;

	usParaType = pucData[usOffset];
	usOffset += DEF_USHORT_SIZE;

	dRet = dCheckOptionParameter( usParaType );
    if( dRet < 0 ) {
        /* CASE : SKIP PARAMETER */
        usParaLen   = pucData[usOffset];
        usOffset    += usParaLen;
    }

	if( usParaType == DEF_PARA_HEARTINFO ) {
		usParaLen = pucData[usOffset];

		log_print( LOGN_INFO, "HEARBEAT_%u HEARTBEAT INFO!!", pstChunkHeader->ucType );
	}

	return 0;
}

/*******************************************************************************

*******************************************************************************/
int dAnalyzeCOOKIE( PASSO_DATA pstMMDB, UCHAR *pucData, pst_SCTPChunkHeader pstChunkHeader )
{
	if( pstChunkHeader->ucType == DEF_SCTP_COOKIE_ECHO ) {
		pstMMDB->ucSetupStatus = (pstMMDB->ucSetupStatus | DEF_COOKIE_ECHO );
	}
	else if( pstChunkHeader->ucType == DEF_SCTP_COOKIE_ACK ) {
		pstMMDB->ucSetupStatus = (pstMMDB->ucSetupStatus | DEF_COOKIE_ACK );
	}

	return 0;
}


/*******************************************************************************

*******************************************************************************/
int dAnalyzeABORT( PASSO_DATA pstMMDB, UCHAR *pucData, pst_SCTPChunkHeader pstChunkHeader)
{
	int			dRet;
    USHORT      usChunkDataLen;

	usChunkDataLen = (pstChunkHeader->usChunkLen - DEF_SCTPCHUNKHEADER_SIZE);

	/* CHECK ERROR CAUSE CODE */
	dRet = dCheckErrorCode( pstMMDB, pucData, usChunkDataLen );
    if( dRet < 0 ) {
        log_print( LOGN_CRI, "[ERROR] FAIL IN FUNC:%s.%d dCheckErrorCode dRet:%d",
                          __FUNCTION__, __LINE__, dRet );
    }

	/* CLOSE ASSOCIATION */
    dRet = dCloseAssociation( pstMMDB );
    if( dRet < 0 ) 
        log_print( LOGN_DEBUG, "NERVER OCCURRED CASE!!" );
	
	return 0;
}


/*******************************************************************************

*******************************************************************************/
int dAnalyzeERROR( PASSO_DATA pstMMDB, UCHAR *pucData, pst_SCTPChunkHeader pstChunkHeader )
{
	int			dRet;
	USHORT		usChunkDataLen;

	usChunkDataLen = (pstChunkHeader->usChunkLen - DEF_SCTPCHUNKHEADER_SIZE);

	/* CHECK ERROR CAUSE CODE */
	dRet = dCheckErrorCode( pstMMDB, pucData, usChunkDataLen );
	if( dRet < 0 ) {
		log_print( LOGN_CRI, "[ERROR] FAIL IN FUNC:%s.%d dCheckErrorCode dRet:%d", __FUNCTION__, __LINE__, dRet );
	}

	return 0;
}


/*******************************************************************************

*******************************************************************************/
int dCheckErrorCode( PASSO_DATA pstMMDB, UCHAR *pucData, USHORT usChunkDataLen )
{
	USHORT 		usDataOffset;
	USHORT      usCauseCode, usCauseLen;

	pst_ERROR   pstERROR;

	while( usChunkDataLen <= usDataOffset ) {
        pstERROR = (pst_ERROR)pucData;
        usCauseCode = TOUSHORT(pstERROR->CAUSE);
        usCauseLen  = TOUSHORT(pstERROR->LENGTH);

        usDataOffset += DEF_ERROR_SIZE;

        /* MAY BE SET STAT */
        switch( usCauseCode )
        {
        case DEF_CAUSE_INVALID_STID:

            break;
        case DEF_CAUSE_MISS_MEN:

            break;
        case DEF_CAUSE_COOKIE_ERR:

            break;
        case DEF_CAUSE_OUT_RESOURCE:

            break;
        case DEF_CAUSE_UNRESOL_ADDR:

            break;
        case DEF_CAUSE_UNRECO_TYPE:

            break;
        case DEF_CAUSE_INVALID_MEN:

            break;
        case DEF_CAUSE_UNRECO_PARA:

            break;
        case DEF_CAUSE_NO_DATA:

            break;
        case DEF_CAUSE_SHTTING_DN:

            break;
        default:
            log_print( LOGN_CRI, "INVALID ERROR CODE FUNC:%s.%d CODE:%u",
                              __FUNCTION__, __LINE__, usCauseCode );

			return -1;
            break;
        }

        /* MOVE NEXT ERROR CAUSES */
        usDataOffset += (usCauseLen - DEF_ERROR_SIZE);
    }

	return 0;
}


/*******************************************************************************

*******************************************************************************/
int dCheckOptionParameter( USHORT usType )
{
	if( ((usType && DEF_CHECK_PARATYPE) == DEF_PARA_SKIP1) ||
		((usType && DEF_CHECK_PARATYPE) == DEF_PARA_SKIP1) ) {
		log_print( LOGN_INFO, "SKIP PARAMETER TYPE:0x%04x", usType );
		return -1;
	}

	return 0;
}

/*******************************************************************************

*******************************************************************************/
int dCloseAssociation( PASSO_DATA pstMMDB )
{
	int			dRet, i;
	UINT		uiStackCount, uiStackIndex;

	pSTACK_LIST	pstStackNode;


	/* DELETE REQ STACK NODE */
	uiStackCount = pstMMDB->uiReqCount;
	uiStackIndex = pstMMDB->uiReqLast;

	for( i=0; i<uiStackCount; i++ ) {
		dRet = dSetStackIndex( uiStackIndex, &pstStackNode );
		if( dRet < 0 ) {
			log_print( LOGN_CRI, "[ERROR] FAIL IN FUN:%s.%d dSetStackIndex dRet:%d",
							  __FUNCTION__, __LINE__, dRet );
			break;
		}

		uiStackIndex = pstStackNode->uiStackPrev;

		dRet = dDelStack( pstMMDB, DEF_FROM_CLIENT, pstStackNode );
		if( dRet < 0 ) {
			log_print( LOGN_CRI, "[ERROR] FAIL IN FUN:%s.%d dSetStackIndex dRet:%d", 
                              __FUNCTION__, __LINE__, dRet );
		}
	}

	/* DELETE RES STACK NODE */
	uiStackCount = pstMMDB->uiResCount;
    uiStackIndex = pstMMDB->uiResLast;

    for( i=0; i<uiStackCount; i++ ) {
        dRet = dSetStackIndex( uiStackIndex, &pstStackNode );
        if( dRet < 0 ) {
            log_print( LOGN_CRI, "[ERROR] FAIL IN FUN:%s.%d dSetStackIndex dRet:%d",
                              __FUNCTION__, __LINE__, dRet );
            break;
        }

        uiStackIndex = pstStackNode->uiStackPrev;

        dRet = dDelStack( pstMMDB, DEF_FROM_SERVER, pstStackNode );
        if( dRet < 0 ) {
            log_print( LOGN_CRI, "[ERROR] FAIL IN FUN:%s.%d dSetStackIndex dRet:%d",
                              __FUNCTION__, __LINE__, dRet );
        }
    }

	/* DELETE ASSOCIATION */
	dRet = dFreeMMDB( pstMMDB );
	if( dRet < 0 ) {
		log_print( LOGN_CRI, "[ERROR] FAIL IN FUN:%s.%d dFreeMMDB dRet:%d",
                              __FUNCTION__, __LINE__, dRet );
	}

	return 0;
}


/*
* $Log: sctp_serv.c,v $
* Revision 1.2  2011/09/06 02:07:44  dcham
* *** empty log message ***
*
* Revision 1.1.1.1  2011/08/29 05:56:42  dcham
* NEW OAM SYSTEM
*
* Revision 1.4  2011/08/25 07:25:47  uamyd
* nifo_msg_write api or log changed to gifo_write
*
* Revision 1.3  2011/08/17 07:24:32  dcham
* *** empty log message ***
*
* Revision 1.2  2011/08/05 02:38:56  uamyd
* A_SCTP modified
*
* Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
* init DQMS2
*
* Revision 1.8  2011/01/11 04:09:09  uamyd
* modified
*
* Revision 1.1.1.1  2010/08/23 01:12:59  uamyd
* DQMS With TOTMON, 2nd-import
*
* Revision 1.7  2009/08/19 14:04:02  pkg
* TRACE관련 오류 수정
*
* Revision 1.6  2009/08/18 19:10:26  pkg
* *** empty log message ***
*
* Revision 1.5  2009/08/18 14:15:13  pkg
* *** empty log message ***
*
* Revision 1.4  2009/08/12 12:34:56  dqms
* DIAMETER 디버그
*
* Revision 1.3  2009/08/11 10:03:23  jsyoon
* SCTP Key에 GroupID 적용
*
* Revision 1.2  2009/05/27 14:24:48  dqms
* *** empty log message ***
*
* Revision 1.1  2009/05/27 07:38:13  dqms
* *** empty log message ***
*
* Revision 1.2  2009/05/14 06:40:51  upst_cvs
* *** empty log message ***
*
* Revision 1.1  2009/05/13 11:38:41  upst_cvs
* NEW
*
* Revision 1.3  2008/03/19 04:56:45  doit1972
* *** empty log message ***
*
* Revision 1.2  2008/01/21 08:26:37  pkg
* *** empty log message ***
*
* Revision 1.1  2008/01/11 12:09:08  pkg
* import two-step by uamyd
*
* Revision 1.17  2007/06/08 12:06:19  dark264sh
* 생성한 nifo node 삭제하지 않는 문제 해결
*
* Revision 1.16  2007/06/08 05:12:19  doit1972
* NO SACK CHECK
*
* Revision 1.15  2007/06/01 15:42:56  doit1972
* LOG INFO
*
* Revision 1.14  2007/06/01 11:07:03  doit1972
* MODIFY LOG INFO
*
* Revision 1.13  2007/05/30 06:43:59  doit1972
* ADD LOG INFO
*
* Revision 1.12  2007/05/30 02:49:37  doit1972
* MODIFY NIFO NODE DELETE POSITION
*
* Revision 1.11  2007/05/29 08:17:20  doit1972
* MODIFY LOG INFO
*
* Revision 1.10  2007/05/14 07:46:10  doit1972
* ADD pstPKTINFO->stPKTCAP.dDataLen = usWriteLen
*
* Revision 1.9  2007/05/14 05:43:27  doit1972
* MODIFY SEND QID INFO
*
* Revision 1.8  2007/05/11 08:35:38  doit1972
* MODIFY CRC & SYSTEM INFO
*
* Revision 1.7  2007/05/10 11:18:05  doit1972
* ADD LOG
*
* Revision 1.6  2007/05/10 06:36:46  doit1972
* ADD SWITCH BY PROTOCOL
*
* Revision 1.5  2007/05/10 02:22:25  doit1972
* *** empty log message ***
*
* Revision 1.4  2007/05/07 05:43:21  doit1972
* MODIFY SRC, DEST DIRECTION INFO
*
* Revision 1.3  2007/05/04 12:34:18  doit1972
* ADD FUNCTIONS
*
* Revision 1.2  2007/05/04 03:18:50  doit1972
* NEW FUNCTION
*
* Revision 1.1  2007/05/04 00:43:18  doit1972
* NEW FILE
*
*/
