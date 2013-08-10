/**
 * Include headers
 */
#include <ctype.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/msg.h>
#include <sys/time.h>

// TOP
#include "common_stg.h"
#include "commdef.h"
#include "procid.h"

// LIB headers
#include "mems.h"
#include "cifo.h"
#include "nifo.h"
#include "gifo.h"
#include "loglib.h"
#include "utillib.h"

// TAF_headers
#include "mmdblist_ftp.h"

// TOOLS
#include "tools.h"

// .
#include "ftp_svc.h"

/**
 * Define constants
 */
typedef enum
{
    CMDPORT,
    CMDRETR,
    CMDSTOR,
    CMDUSER,
    MAX_CMDS
} FTPCMD;

#define	SP					' '
#define PORT_FTPDATA		20
#define PORT_FTPSIG			21

/**
 * Declare variables
 */
string_short cmds[MAX_CMDS] = {
    { "RETR", CMDRETR },
    { "STOR", CMDSTOR },
    { "PORT", CMDPORT },
    { "USER", CMDUSER }
};
int					gACALLCnt;
extern stMEMSINFO	*pstMEMSINFO;
extern stCIFO		*gpCIFO;
extern int			g_lCurTime;


/**
 *	Implement func.
 */
int dGetCALLProcID(unsigned int uiClientIP)
{
	return SEQ_PROC_A_CALL + ( uiClientIP % gACALLCnt );
}

int dSvcProcess( TCP_INFO *pTCPINFO, Capture_Header_Msg *pCAPHEAD, U8 *pDATA )
{
	int				dRet;

	switch( pTCPINFO->cTcpFlag ) {
		case DEF_TCP_START:
			dRet = dSessStartMessage( pTCPINFO );
			if( dRet < 0 )
				log_print( LOGN_INFO, "ERROR IN dSessStartMessage dRet:%d", dRet ); 

			break;
		case DEF_TCP_END:
			dRet = dSessStopMessage( pTCPINFO );
			if( dRet < 0 ) 
                log_print( LOGN_INFO, "ERROR IN dSessStopMessage dRet:%d", dRet );	

			break;
		case DEF_TCP_DATA:
			dRet = dFTPMessage( pTCPINFO, pCAPHEAD, pDATA );
			if( dRet < 0 )
                log_print( LOGN_INFO, "ERROR IN dFTPMessage dRet:%d", dRet );

			break;
		default:

			break;
	}

	return 0;
}

int CmdParse(SESS_DATA *pstTcp, char *szData, int uDataSize, UINT uiTime, UINT uiMTime )
{
    int 			i, dRet;
    char			szDest[16], *str;
    int    	 		ip[4];
    int     		port[2];
	SESS_DATA		stMMDB, *pstMMDB;
             
    for ( i=0; i<MAX_CMDS; i++ )
    {
        if ( strncmp( cmds[i].szValue, szData, strlen(cmds[i].szValue) )==0 )
        {
            log_print( LOGN_INFO, "[PARSE] CMD [%s]", cmds[i].szValue );
            switch( cmds[i].usValue )
			{
				case CMDPORT:
					str = strchr( szData, ' ' );
					if ( str!=NULL )
					{
						if ( pstTcp->stDataKey.uiSrcIP!=0 && pstTcp->stDataKey.usSrcPort!=0 )
						{
							strcpy( szDest, util_cvtipaddr(NULL,  pstTcp->stDataKey.uiDestIP ) );
							dRet = dSetMMDB( &pstTcp->stDataKey, &pstMMDB );
							if( dRet >= 0 ) {
								log_print( LOGN_CRI, "[SIGNAL][DATA] INVALID FTP DATA SESS SRC[%s:%u] DEST[%s:%u]",
										util_cvtipaddr(NULL, pstTcp->stDataKey.uiSrcIP), 
										pstTcp->stDataKey.usSrcPort, 
										szDest, 
										pstTcp->stDataKey.usDestPort );
								dTermTCP( &pstTcp->stDataKey );
							}
						}

						str++;
						sscanf( str, "%d,%d,%d,%d,%d,%d",                                                  
								&ip[0], &ip[1], &ip[2], &ip[3], &port[0], &port[1] );                      
						pstTcp->stDataKey.uiSrcIP = ((ip[0]*256+ip[1])*256+ip[2])*256+ip[3];                          
						pstTcp->stDataKey.usSrcPort = port[0]*256+port[1];                                            
						pstTcp->stDataKey.uiDestIP = pstTcp->stKey.uiDestIP;
						pstTcp->stDataKey.usDestPort = 20;
						pstTcp->stDataKey.uiReserved = 0;

						memset( &stMMDB, 0, sizeof( SESS_DATA ) );
						memcpy( &stMMDB.stKey, &pstTcp->stDataKey, SESS_KEY_SIZE );
						dRet = dInsertMMDB( &stMMDB, &pstMMDB );
						if ( dRet < 0 )
							return -1;
						else {
							strcpy( szDest, util_cvtipaddr(NULL, stMMDB.stKey.uiDestIP) );

							log_print( LOGN_INFO, "CREATE DATA TCP SUCCESS SRC [%s:%u] DEST[%s:%u] %u %u",
            									util_cvtipaddr(NULL, stMMDB.stKey.uiSrcIP), stMMDB.stKey.usSrcPort, 
												szDest, stMMDB.stKey.usDestPort,
												stMMDB.stKey.uiSrcIP, stMMDB.stKey.uiDestIP);
						}

						pstMMDB->stSignalKey = pstTcp->stKey;
					}
					break;                                                                                     
				case CMDSTOR:
				case CMDRETR:                                                                                  
				case CMDUSER:                                                                                  
					break;                                                                                     
				default:                                                                                       
					break;                                                                                     

			}                                                                                              
            return cmds[i].usValue;                                                                         
        }                                                                                                  
    }

	log_print( LOGN_INFO, "[PARSE] UNKNOWN COMMAND : %s", szData );
    return 0;                                                                                              
}

int ReplyParse(SESS_DATA *pstTcp, char *szData, int uDataSize, UINT uiTime, UINT uiMTime )
{
	int		respcode;
	
    if (isdigit(szData[0]) && isdigit(szData[1]) && isdigit(szData[2]) && szData[3]==SP ) {   
        sscanf( szData, "%d ", &respcode );
        switch( respcode ) {   
            case 230:
                pstTcp->stFTPSTAT.uiFTPLogonDuration = (uiTime * 1000000 + uiMTime) - 
							(pstTcp->stFTPSTAT.uiTcpSynTime * 1000000 
							+ pstTcp->stFTPSTAT.uiTcpSynMTime);
                break;
			default:
				break;
        }
    }

	return 1;
}

int dFTPMessage( TCP_INFO *pTCPINFO, Capture_Header_Msg  *pCAPHEAD, U8 *pDATA )
{
	int				dRet;
	int				dRtxFlag;
	time_t			tNowTime;

	/* SESS_DATA		stMMDB; */
	PSESS_DATA		pstMMDB;
	SESS_KEY		stKey;

	st_TimeVal		stAckTime;

	dRtxFlag = pCAPHEAD->bRtxType;

	/* SETTING MMDB KEY */

	stKey.uiSrcIP		= pTCPINFO->uiCliIP; 
	stKey.usSrcPort		= pTCPINFO->usCliPort;
	stKey.uiDestIP		= pTCPINFO->uiSrvIP;
	stKey.usDestPort	= pTCPINFO->usSrvPort; 
	stKey.uiReserved 	= 0;

	time(&tNowTime);

	if( stKey.usSrcPort != PORT_FTPSIG && stKey.usDestPort != PORT_FTPSIG 
		&& stKey.usSrcPort != PORT_FTPDATA && stKey.usDestPort != PORT_FTPDATA ) {
		log_print( LOGN_DEBUG, "[DROP] NOT FTP DATA IP:%s PORT:%u RTX:%d",
						util_cvtipaddr(NULL, stKey.uiSrcIP), stKey.usSrcPort, dRtxFlag );	
		return 1;
	}

	log_print( LOGN_DEBUG, "IP:%s PORT:%u RTX:%d %u %u %u %u",
						util_cvtipaddr(NULL, stKey.uiSrcIP), stKey.usSrcPort, dRtxFlag,
						pTCPINFO->uiIPTotUpPktCnt, pTCPINFO->uiIPTotDnPktCnt,
						pTCPINFO->uiIPTotUpPktSize, pTCPINFO->uiIPTotDnPktSize);	

	stAckTime.tv_sec 	= pTCPINFO->uiAckTime;
	stAckTime.tv_usec	= pTCPINFO->uiAckMTime;

	
	/* A_TCP BUG PROTECTION CODE - DROP PKT WHEN TIME TOO LATE *
	if ( g_lCurTime > pstIPTCPHeader->tCapTime.tv_sec+600 ) {
		log_print( LOGN_DEBUG, "[FTP] DROP PKT TOO OLD TIME[%d]", pstIPTCPHeader->tCapTime.tv_sec );
		return 0;
	}

	if ( pstIPTCPHeader->tCapTime.tv_sec > pstAckTime->tv_sec ) {
		*pstAckTime = pstIPTCPHeader->tCapTime;
	}
	*/

	//szTCPData = &pstMsgQ->szBody[DEF_PACKHDR_SIZE];

	log_print( LOGN_INFO, "[DATATIME] [%d.%d]", pTCPINFO->uiCapTime, pTCPINFO->uiCapMTime );
	log_print( LOGN_INFO, "[ACKTIME] [%ld.%ld]", stAckTime.tv_sec, stAckTime.tv_usec );

	/* HANDLE SIGNAL */
	if( (dRet = dSetMMDB( &stKey, &pstMMDB ) ) < 0 ) {
		/***** SESSION도 없고, 시작 메시지도 아닌경우, 해당 사항이 없으므로 폐기되는 경우 */
		log_print( LOGN_CRI, "CANNOT FIND TCP SESSION INFO [%s][%d]",
						  util_cvtipaddr(NULL, stKey.uiSrcIP), stKey.usSrcPort );

		return -1;
	}

	pstMMDB->stFTPSTAT.uiLastUpdateTime = pTCPINFO->uiCapTime;

	if ( stKey.usDestPort == 21 ) {
		if( dRtxFlag == DEF_FROM_SERVER ) {
			ReplyParse( pstMMDB, (char*)pDATA, pTCPINFO->uiDataSize, pTCPINFO->uiCapTime, pTCPINFO->uiCapMTime );
			log_print( LOGN_INFO, "[SERVER] PARSE END" );
		}
		else if( dRtxFlag == DEF_FROM_CLIENT ) {
			CmdParse( pstMMDB, (char*)pDATA, pTCPINFO->uiDataSize, pTCPINFO->uiCapTime, pTCPINFO->uiCapMTime );
			log_print( LOGN_INFO, "[CLIENT] PARSE END" );
		}
	}
	else {
		switch( dRtxFlag ) {
			case DEF_FROM_SERVER:
				pstMMDB->stFTPSTAT.uiDownBytes 	+= pTCPINFO->uiIPTotDnPktSize;
				pstMMDB->stFTPSTAT.uiDownFrames += pTCPINFO->uiIPTotDnPktCnt;

				pstMMDB->stFTPSTAT.uiUpBytes    += pTCPINFO->uiIPTotUpPktSize;
				pstMMDB->stFTPSTAT.uiUpFrames   += pTCPINFO->uiIPTotUpPktCnt;

				pstMMDB->stFTPSTAT.uiDownRetranBytes	+= pTCPINFO->uiIPTotDnRetransSize;

				if ( pstMMDB->stFTPSTAT.uiFTPDownFirstSeq==0 )
					pstMMDB->stFTPSTAT.uiFTPDownFirstSeq = pTCPINFO->uiSeqNum;

				pstMMDB->stFTPSTAT.uiFTPDownLastSeq = pTCPINFO->uiSeqNum;

				log_print( LOGN_INFO, "[SERVER-DATA]" );

				break;

			case DEF_FROM_CLIENT:
				pstMMDB->stFTPSTAT.uiDownBytes  += pTCPINFO->uiIPTotDnPktSize;
				pstMMDB->stFTPSTAT.uiDownFrames += pTCPINFO->uiIPTotDnPktCnt;

				pstMMDB->stFTPSTAT.uiUpBytes 	+= pTCPINFO->uiIPTotUpPktSize;
				pstMMDB->stFTPSTAT.uiUpFrames 	+= pTCPINFO->uiIPTotUpPktCnt;

				pstMMDB->stFTPSTAT.uiUpRetranBytes	+= pTCPINFO->uiIPTotUpRetransSize;
				
				if ( pstMMDB->stFTPSTAT.uiFTPUpFirstSeq==0 )
					pstMMDB->stFTPSTAT.uiFTPUpFirstSeq = pTCPINFO->uiSeqNum;

				pstMMDB->stFTPSTAT.uiFTPUpLastSeq = pTCPINFO->uiSeqNum;

				log_print( LOGN_INFO, "[CLIENT_DATA]" );

				break;
		}
	}

	return 0;
}

/*******************************************************************************
 해당 TCP SESSION 생성
*******************************************************************************/
int dSessStartMessage( TCP_INFO *pTCPINFO )
{
	int					dRet;
	SESS_DATA			stSrc;
	PSESS_DATA      	pstMMDB;
    SESS_KEY        	stKey;
	char				szDest[16];

	/* SETTING MMDB KEY */
	stKey.uiSrcIP 		= pTCPINFO->uiCliIP;
	stKey.usSrcPort		= pTCPINFO->usCliPort;
	stKey.uiDestIP		= pTCPINFO->uiSrvIP;
	stKey.usDestPort	= pTCPINFO->usSrvPort;
	stKey.uiReserved	= 0;

	strcpy( szDest, util_cvtipaddr(NULL,  stKey.uiDestIP ) );

	dRet = dSetMMDB( &stKey, &pstMMDB );
	if ( pTCPINFO->usSrvPort == 21 ) {
		if( dRet >= 0 ) {
			/* FTP SIGNAL에서 TCP KEY 중복이 발생하면 기존 키는 정리 후에 새로 키 생성 */
			log_print( LOGN_CRI, "[SIGNAL][START] TCP KEY DUPLICATE SRC [%s:%u] DEST[%s:%u]",
							  util_cvtipaddr(NULL, stKey.uiSrcIP), stKey.usSrcPort, szDest, stKey.usDestPort );
			/* added by uamyd 2007.10.17 : If EXIST session when recieved 'START', then running free old session */
			pstMMDB->dTimeOutTime=1;
			dTermTCP( &stKey );
		}

		memset( &stSrc, 0, sizeof(SESS_DATA) );
		stSrc.stKey = stKey;
		dRet = dInsertMMDB( &stSrc, &pstMMDB );
		if ( dRet < 0) {
			log_print( LOGN_CRI, "[SIGNAL][START] CREATE FAIL [%d] SRC [%s:%u] DEST[%s:%u]",
							  dRet, util_cvtipaddr(NULL, stKey.uiSrcIP), stKey.usSrcPort, szDest, stKey.usDestPort );
			return dRet;
		}

		strcpy( szDest, util_cvtipaddr(NULL, stKey.uiDestIP) );

		log_print( LOGN_INFO, "CREATE TCP SUCCESS SRC [%s:%u] DEST[%s:%u] %u.%u %u",
					            util_cvtipaddr(NULL, stKey.uiSrcIP), stKey.usSrcPort, szDest, stKey.usDestPort,
								pTCPINFO->uiCapTime, pTCPINFO->uiCapMTime, pTCPINFO->uiAckTime);

		/* FILL TCP INFO */
		pstMMDB->stFTPSTAT.uiTcpSynTime		= pTCPINFO->uiCapTime;
		pstMMDB->stFTPSTAT.uiTcpSynMTime	= pTCPINFO->uiCapMTime;

		pstMMDB->stFTPSTAT.uiServerIP		= pTCPINFO->uiSrvIP;
		pstMMDB->stFTPSTAT.uiClientIP		= pTCPINFO->uiCliIP;

		pstMMDB->stFTPSTAT.usPlatformType	= dGetPlatformType(pTCPINFO->usL4Code, pTCPINFO->usL7Code);
		pstMMDB->stFTPSTAT.usSvcL4Type		= pTCPINFO->usL4Code;
		pstMMDB->stFTPSTAT.usSvcL7Type		= pTCPINFO->usL7Code;
	}
	else {
		if ( dRet<0 ) {
			log_print( LOGN_DEBUG, "[DATA][START] NOT FOUND TCP SRC[%s:%hu] DEST[%s:%hu]",
					util_cvtipaddr(NULL, stKey.uiSrcIP), stKey.usSrcPort, szDest, stKey.usDestPort );
			return dRet;
		}

		pstMMDB->stFTPSTAT.uiTcpSynTime     = pTCPINFO->uiCapTime;
        pstMMDB->stFTPSTAT.uiTcpSynMTime    = pTCPINFO->uiCapMTime;

        pstMMDB->stFTPSTAT.uiServerIP       = pTCPINFO->uiSrvIP;
		pstMMDB->stFTPSTAT.uiClientIP       = pTCPINFO->uiCliIP;
		pstMMDB->stFTPSTAT.usPlatformType   = dGetPlatformType(pTCPINFO->usL4Code, pTCPINFO->usL7Code);
        pstMMDB->stFTPSTAT.usSvcL4Type      = pTCPINFO->usL4Code;
        pstMMDB->stFTPSTAT.usSvcL7Type      = pTCPINFO->usL7Code;

		/* FILL TCP INFO */
		pstMMDB->stFTPSTAT.uiFTPSynTime			= pTCPINFO->uiCapTime;
        pstMMDB->stFTPSTAT.uiFTPSynMTime		= pTCPINFO->uiCapMTime;

		pstMMDB->stFTPSTAT.uiDownBytes  += pTCPINFO->uiIPTotDnPktSize;
        pstMMDB->stFTPSTAT.uiDownFrames += pTCPINFO->uiIPTotDnPktCnt;

        pstMMDB->stFTPSTAT.uiUpBytes    += pTCPINFO->uiIPTotUpPktSize;
        pstMMDB->stFTPSTAT.uiUpFrames   += pTCPINFO->uiIPTotUpPktCnt;

		log_print( LOGN_INFO, "START %u %u %u %u", pTCPINFO->uiIPTotUpPktCnt, pTCPINFO->uiIPTotDnPktCnt,
										  pTCPINFO->uiIPTotUpPktSize, pTCPINFO->uiIPTotDnPktSize );
	}

	/* FILL CALL INFO */
	pstMMDB->stFTPSTAT.uiLastUpdateTime			= pTCPINFO->uiCapTime;

	return 0;
}


int dTermTCP( PSESS_KEY pstKey )
{
	int				dRet;
	int				dSeqProcID;
	st_TimeVal		stTime;
	PSESS_DATA		pstMMDB, pstFTPDATA, pstFTPSIG;
	char			szDest[16];

	U8				*pstNode;
	LOG_FTP			*pstFTPLog;

	dRet = dSetMMDB( pstKey, &pstMMDB );
	if( dRet < 0 ) {
		log_print( LOGN_DEBUG, "NO TCP SESSION IN dTermTCP [%s][%d]",
				util_cvtipaddr(NULL, pstKey->uiSrcIP), pstKey->usSrcPort );

		return -1;
	}

	gettimeofday( &stTime, NULL );

	strcpy( szDest, util_cvtipaddr(NULL,  pstKey->uiDestIP ) );

	if ( pstKey->usDestPort == 21 ) {
		if ( pstMMDB->stDataKey.uiSrcIP != 0 && pstMMDB->dTimeOutTime==0 ) {
			log_print( LOGN_INFO, "[SIGNAL][STOPWAIT] WAIT DATA-TCP OUT SRC[%s:%u] DEST[%s:%u] SYN[%u]",
					util_cvtipaddr(NULL, pstKey->uiSrcIP), pstKey->usSrcPort, szDest, pstKey->usDestPort,
					pstMMDB->stFTPSTAT.uiTcpSynTime );

			/* changed by uamyd 2007.10.07 : 10 -> 3 */
			pstMMDB->dTimeOutTime = g_lCurTime+3;

			return 0;
		}

		log_print( LOGN_INFO, "[SIGNAL][STOP] SRC [%s:%u] DEST[%s:%u] SYN[%u]",
				util_cvtipaddr(NULL, pstKey->uiSrcIP), pstKey->usSrcPort, szDest, pstKey->usDestPort,
				pstMMDB->stFTPSTAT.uiTcpSynTime );
	}
	else {
		log_print( LOGN_INFO, "[DATA][STOP] SRC [%s:%u] DEST[%s:%u] SYN[%u]",
				util_cvtipaddr(NULL, pstKey->uiSrcIP), pstKey->usSrcPort, szDest, pstKey->usDestPort,
				pstMMDB->stFTPSTAT.uiFTPSynTime );

		pstFTPDATA = pstMMDB;

		dRet = dSetMMDB( &pstFTPDATA->stSignalKey, &pstFTPSIG );
		if( dRet < 0 ) {
			log_print( LOGN_DEBUG, "[DATA][STOP] NOT FOUND SIGNAL SESS [%s][%d]",
					util_cvtipaddr(NULL, pstFTPDATA->stSignalKey.uiSrcIP), pstFTPDATA->stSignalKey.usSrcPort );

			return -1;
		}

		if ( memcmp( &pstFTPSIG->stDataKey, &pstFTPDATA->stKey, sizeof(SESS_KEY) )!=0 ) {
			log_print( LOGN_DEBUG, "[DATA][STOP] SIGNAL & DATA KEY MISMATCH [%s][%d]",
					util_cvtipaddr(NULL, pstFTPSIG->stKey.uiSrcIP), pstFTPSIG->stKey.usSrcPort );
		}
		else {
			if( (pstNode = nifo_node_alloc( pstMEMSINFO )) == NULL ) {
                log_print(LOGN_CRI, "[%s.%d] nifo_node_alloc NULL", __FUNCTION__, __LINE__);
                return -1;
            }

            if( (pstFTPLog = (LOG_FTP *)nifo_tlv_alloc(pstMEMSINFO, pstNode, LOG_FTP_DEF_NUM, LOG_FTP_SIZE, DEF_MEMSET_OFF)) == NULL ) {
                log_print(LOGN_CRI, "[%s.%d] nifo_tlv_alloc [%d] NULL", __FUNCTION__, __LINE__, LOG_FTP_DEF_NUM );
				nifo_node_delete(pstMEMSINFO, pstNode);
                return -2;
            }

			memset( pstFTPLog, 0x00, LOG_FTP_SIZE );

			CopyLOGFTP( pstFTPLog, &pstFTPSIG->stFTPSTAT, &pstFTPDATA->stFTPSTAT );

			pstFTPLog->uiEndDuration = (pstFTPDATA->stFTPSTAT.uiEndTime*1000000+pstFTPDATA->stFTPSTAT.uiEndMTime)
                                                 -(pstFTPDATA->stFTPSTAT.uiFTPFinTime*1000000+pstFTPDATA->stFTPSTAT.uiFTPFinMTime);
			pstFTPLog->uiFTPTimeDuration = (pstFTPDATA->stFTPSTAT.uiFTPFinTime*1000000
                                                    + pstFTPDATA->stFTPSTAT.uiFTPFinMTime)
                                                    - (pstFTPDATA->stFTPSTAT.uiFTPSynTime*1000000
                                                    + pstFTPDATA->stFTPSTAT.uiFTPSynMTime);

			/*
			pstFTPLog->OpStartTime		= pstFTPDATA->stFTPSTAT.uiTcpSynTime;
			pstFTPLog->OpStartMTime		= pstFTPDATA->stFTPSTAT.uiTcpSynMTime;
			pstFTPLog->OpEndTime		= pstFTPDATA->stFTPSTAT.uiEndTime;
			pstFTPLog->OpEndMTime		= pstFTPDATA->stFTPSTAT.uiEndMTime;
			*/

			log_print(LOGN_CRI, "dTermTCP][FTPFIN=%u %u][FTPSYN=%u %u][FTPTIMEDUR=%u", 
							pstFTPLog->uiFTPFinTime, pstFTPLog->uiFTPFinMTime,
							pstFTPLog->uiFTPSynTime, pstFTPLog->uiFTPSynMTime,
							pstFTPLog->uiFTPTimeDuration);

			memset( &pstFTPSIG->stDataKey, 0, sizeof(SESS_KEY) );

			if ( pstFTPSIG->dTimeOutTime>0 ) {
				dRet = dFreeMMDB( pstFTPSIG );
				if( dRet < 0 ) {
					log_print( LOGN_CRI, "ERROR IN dFreeMMDB RET[%d]", dRet );
        			nifo_node_delete(pstMEMSINFO, pstNode);
					return -2;
				}
			}

//			LOG_FTP_Prt("PRINT LOG_FTP", pstFTPLog );

			dSeqProcID = dGetCALLProcID(pstFTPLog->uiClientIP);
    		if( (dRet = gifo_write( pstMEMSINFO, gpCIFO, SEQ_PROC_A_FTP, dSeqProcID, nifo_offset(pstMEMSINFO, pstNode) )) < 0 ) {
        		nifo_node_delete(pstMEMSINFO, pstNode);
        		log_print(LOGN_CRI, "SEND DATA SeqProcID[%d]", dSeqProcID );
    		}
		}
	}

	dRet = dFreeMMDB( pstMMDB );
	if( dRet < 0 ) 
	{
		log_print( LOGN_CRI, "ERROR IN dFreeMMDB RET[%d]", dRet );
		return -2;
	}

	return 1;
}



/*******************************************************************************
 해당 TCP SESSION에 대한 모든 NODE 정리
*******************************************************************************/
int dSessStopMessage( TCP_INFO *pTCPINFO )
{
	int					dRet;
    SESS_KEY        	stKey;
	PSESS_DATA			pstMMDB;
	char				szDest[16];

	/* SETTING MMDB KEY */
	stKey.uiSrcIP       = pTCPINFO->uiCliIP;
    stKey.usSrcPort     = pTCPINFO->usCliPort;
    stKey.uiDestIP      = pTCPINFO->uiSrvIP;
    stKey.usDestPort    = pTCPINFO->usSrvPort;
    stKey.uiReserved = 0;

	dRet = dSetMMDB( &stKey, &pstMMDB );
	if( dRet < 0 ) {
		log_print( LOGN_WARN, "CANNOT FIND TCP SESSION IN Retransmission [IP]:[%s] [%u]",
							util_cvtipaddr(NULL, stKey.uiSrcIP), 
							stKey.usSrcPort );
		return 0;
	}

	pstMMDB->stFTPSTAT.uiLastUpdateTime = pTCPINFO->uiCapTime;

	if( stKey.usDestPort == 20 ) {
		pstMMDB->stFTPSTAT.uiDownBytes  += pTCPINFO->uiIPTotDnPktSize;
        pstMMDB->stFTPSTAT.uiDownFrames += pTCPINFO->uiIPTotDnPktCnt;

        pstMMDB->stFTPSTAT.uiUpBytes    += pTCPINFO->uiIPTotUpPktSize;
        pstMMDB->stFTPSTAT.uiUpFrames   += pTCPINFO->uiIPTotUpPktCnt;

        log_print( LOGN_INFO, "STOP %u %u %u %u", pTCPINFO->uiIPTotUpPktCnt, pTCPINFO->uiIPTotDnPktCnt,
                                                pTCPINFO->uiIPTotUpPktSize, pTCPINFO->uiIPTotDnPktSize );
		if( pTCPINFO->usL4FailCode == TCP_NOERR_FIN_E1 || pTCPINFO->usL4FailCode == TCP_NOERR_FIN_E2 ) {
			pstMMDB->stFTPSTAT.uiFTPFinTime     = pTCPINFO->uiCapTime;
        	pstMMDB->stFTPSTAT.uiFTPFinMTime    = pTCPINFO->uiCapMTime;

        	pstMMDB->stFTPSTAT.usCliFTPStatus   = pTCPINFO->ucTcpClientStatus;
        	pstMMDB->stFTPSTAT.usSvrFTPStatus   = pTCPINFO->ucTcpServerStatus;
		}
	}
		
	pstMMDB->stFTPSTAT.uiEndTime 	= pTCPINFO->uiCapTime;
	pstMMDB->stFTPSTAT.uiEndMTime 	= pTCPINFO->uiCapMTime;

	pstMMDB->stFTPSTAT.usCliFTPStatus   = pTCPINFO->ucTcpClientStatus;
    pstMMDB->stFTPSTAT.usSvrFTPStatus   = pTCPINFO->ucTcpServerStatus;
	

	strcpy( szDest, util_cvtipaddr(NULL,  stKey.uiDestIP ) );

	log_print( LOGN_INFO, "[STOP] TCP SRC [%s:%u] DEST[%s:%u] SYN [%u] %u.%u",
			util_cvtipaddr(NULL, stKey.uiSrcIP), stKey.usSrcPort, szDest, stKey.usDestPort, 
			pstMMDB->stFTPSTAT.uiTcpSynTime, pTCPINFO->uiCapTime, pTCPINFO->uiCapMTime );

	dRet = dTermTCP( &stKey );

	return dRet;
}

int dSend_FTPLOG( LOG_FTP *pstLog )
{
	int			dRet;
	int			dSeqProcID;
	U8			*pstNode;

	if( (pstNode = nifo_node_alloc( pstMEMSINFO )) == NULL ) {
		log_print(LOGN_CRI, "[%s.%d] nifo_node_alloc NULL", __FUNCTION__, __LINE__);
        return -1;
	}

	if( (pstLog = (LOG_FTP *)nifo_tlv_alloc(pstMEMSINFO, pstNode, LOG_FTP_DEF_NUM, LOG_FTP_SIZE, DEF_MEMSET_OFF)) == NULL ) {
		log_print(LOGN_CRI, "[%s.%d] nifo_tlv_alloc [%d] NULL", __FUNCTION__, __LINE__, LOG_FTP_DEF_NUM );
       	nifo_node_delete(pstMEMSINFO, pstNode);
		return -2;
	}

	dSeqProcID = dGetCALLProcID(pstLog->uiClientIP);
	if( (dRet = gifo_write( pstMEMSINFO, gpCIFO, SEQ_PROC_A_FTP, dSeqProcID, nifo_offset(pstMEMSINFO, pstNode) )) < 0 ) {
		nifo_node_delete(pstMEMSINFO, pstNode);
		log_print(LOGN_CRI, "SEND DATA SeqProcID[%d]", dSeqProcID );
	}

	return 1;
}


void CopyLOGFTP( LOG_FTP *pLOG_FTP, LOG_FTP_INFO *pLOG_SIG, LOG_FTP_INFO *pLOG_DATA )
{
	pLOG_FTP->uiClientIP			= pLOG_DATA->uiClientIP;
	pLOG_FTP->usPlatformType		= pLOG_DATA->usPlatformType;
	pLOG_FTP->usSvcL4Type			= pLOG_DATA->usSvcL4Type;
	pLOG_FTP->usSvcL7Type			= pLOG_DATA->usSvcL7Type;
	pLOG_FTP->uiServerIP			= pLOG_DATA->uiServerIP;
	pLOG_FTP->uiFTPLogonDuration	= pLOG_SIG->uiFTPLogonDuration;
	pLOG_FTP->uiEndTime				= pLOG_DATA->uiEndTime;
	pLOG_FTP->uiEndMTime			= pLOG_DATA->uiEndMTime;
	pLOG_FTP->uiEndDuration			= pLOG_DATA->uiEndDuration;
	pLOG_FTP->uiTcpSynTime			= pLOG_SIG->uiTcpSynTime;
	pLOG_FTP->uiTcpSynMTime			= pLOG_SIG->uiTcpSynMTime;
	pLOG_FTP->uiTcpSynAckTime		= pLOG_SIG->uiTcpSynAckTime;
	pLOG_FTP->uiUpFrames			= pLOG_DATA->uiUpFrames;
	pLOG_FTP->uiDownFrames			= pLOG_DATA->uiDownFrames;
	pLOG_FTP->uiUpBytes				= pLOG_DATA->uiUpBytes;
	pLOG_FTP->uiDownBytes			= pLOG_DATA->uiDownBytes;
	pLOG_FTP->uiUpRetranBytes		= pLOG_DATA->uiUpRetranBytes;
	pLOG_FTP->uiDownRetranBytes		= pLOG_DATA->uiDownRetranBytes;
	pLOG_FTP->uiFTPSynTime			= pLOG_DATA->uiFTPSynTime;
	pLOG_FTP->uiFTPSynMTime			= pLOG_DATA->uiFTPSynMTime;
	pLOG_FTP->uiFTPFinTime			= pLOG_DATA->uiFTPFinTime;
	pLOG_FTP->uiFTPFinMTime			= pLOG_DATA->uiFTPFinMTime;
	pLOG_FTP->uiFTPUpFirstSeq		= pLOG_DATA->uiFTPUpFirstSeq;
	pLOG_FTP->uiFTPUpLastSeq		= pLOG_DATA->uiFTPUpLastSeq;
	pLOG_FTP->uiFTPDownFirstSeq		= pLOG_DATA->uiFTPDownFirstSeq;
	pLOG_FTP->uiFTPDownLastSeq		= pLOG_DATA->uiFTPDownLastSeq;
	pLOG_FTP->uiLastUpdateTime		= pLOG_DATA->uiLastUpdateTime;
	pLOG_FTP->uiFTPTimeDuration		= pLOG_DATA->uiFTPTimeDuration;
	pLOG_FTP->usCliFTPStatus		= pLOG_DATA->usCliFTPStatus;
	pLOG_FTP->usSvrFTPStatus		= pLOG_DATA->usSvrFTPStatus;
	pLOG_FTP->OpStartTime			= pLOG_SIG->uiTcpSynTime;
	pLOG_FTP->OpStartMTime			= pLOG_SIG->uiTcpSynMTime;
	pLOG_FTP->OpEndTime				= pLOG_DATA->uiEndTime;
	pLOG_FTP->OpEndMTime			= pLOG_DATA->uiEndMTime;
}
