/**A.1*	FILE INCLUSION ********************************************************/

#include <stdio.h>
#include <ctype.h>
#include <string.h>

// LIB
#include "typedef.h"
#include "define.h"
#include "loglib.h"
#include "utillib.h"

// PROJECT
#include "common_stg.h"

// .
#include "radius_decode.h"
#include "radius_func.h"

/**B.2*	DEFINITION OF NEW TYPE ************************************************/
/**C.1*	DECLARATION OF VARIABLES **********************************************/

void print_timegap(struct timeval *before, struct timeval *after, char *funcname, int linenum)
{
    time_t diff = (after->tv_sec*1000000+after->tv_usec) - (before->tv_sec*1000000+before->tv_usec);
    log_print(LOGN_CRI, "FUNC[%s %d]: BEFORE[%ld.%06ld] AFTER[%ld.%06ld] DIFF[%ld]",
            funcname, linenum, before->tv_sec, before->tv_usec, after->tv_sec, after->tv_usec, diff);
}

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

*******************************************************************************/
inline int dAnalyze_RADIUS( UCHAR *pBuf, pst_ACCInfo pstAccReq, INFO_ETH *pINFOETH )
{
	int			dRet;
	int			dHeadLen;	/* IP Header Len + TCP Header Len */
	USHORT		usDataLen;
	pst_Radius	pstRadius;
	int			dPaddingSize;
	
	/* ADD BY YOON 2008.09.23 */
	dPaddingSize = pINFOETH->stIP.wTotalLength
					- pINFOETH->stUDPTCP.wDataLen - pINFOETH->stIP.wIPHeaderLen - pINFOETH->stUDPTCP.wHeaderLen;

	log_print( LOGN_DEBUG, "dPaddingSize:%d wTotalLength:%d wIPHeaderLen:%d wHeaderLen:%d", 
							dPaddingSize, pINFOETH->stIP.wTotalLength, pINFOETH->stIP.wIPHeaderLen, pINFOETH->stUDPTCP.wHeaderLen);

	dHeadLen = pINFOETH->stIP.wTotalLength - (pINFOETH->stUDPTCP.wDataLen + dPaddingSize);

#ifdef PACKET_DUMP
	dump_DebugString("RADIUS", pBuf+14+dHeadLen, pINFOETH->stUDPTCP.wDataLen);
#endif

	pstRadius 			= (pst_Radius)(pBuf+14+dHeadLen);
	pstAccReq->ucCode 	= pstRadius->Code;
	pstAccReq->ucID		= pstRadius->Identifier;

	usDataLen = TOUSHORT(pstRadius->Length);
	/* ADD BY YOON 2008.09.23 */
	if( pINFOETH->stUDPTCP.wDataLen < usDataLen ) {
		log_print( LOGN_DEBUG, "INVALID LENGTH RAD:%d dHeadLen:%d HEAD:%d", usDataLen, dHeadLen, pINFOETH->stUDPTCP.wDataLen);
		return -1;
	}

	memcpy( &pstAccReq->szAuthen[0], &pstRadius->Authenticator[0], 16 );
	pstAccReq->szAuthen[16] = 0x00;

	if( usDataLen > 20 ) {

		dRet = dAnalyze_RADIUS_ATTRIB( pstRadius->Attributes, usDataLen-20, pstAccReq );
		if( dRet < 0 ) {
			log_print( LOGN_INFO, "INVALID ATTRIBUTE LENGTH INFO" );
			return -2;
		}
	}

	return 1;
}


/*******************************************************************************

*******************************************************************************/
inline int dAnalyze_RADIUS_ATTRIB( UCHAR *pBuf, USHORT usDataLen, pst_ACCInfo pstAccInfo )
{
	int			dOffset = 0;
	USHORT		usLength;
	char		szC23BIT[64];

	pstAccInfo->uiRADIUSLen = usDataLen;

	while( dOffset < usDataLen ) {
    
		switch( pBuf[dOffset] ) {
			case 1 :	/* USER NAME */
                usLength = pBuf[dOffset+1];
                if( usLength >= 3 && usLength < (MAX_USERNAME_SIZE + 2) ) {
                    memcpy( &pstAccInfo->szUserName[0], pBuf+dOffset+2, usLength-2 );
                    pstAccInfo->szUserName[usLength-2] = 0x00;
					pstAccInfo->ucUserNameF = DEF_FLAG_ON;
                }
                else
                    log_print( LOGN_INFO, "INVALID USER_NAME LEN:%d", usLength );

                break;

			case 2:		/* ACCESS : USER PASSWORD */
				usLength = pBuf[dOffset+1];
				if( (usLength-2) >= (MAX_USERPWD_SIZE-1) ) {
					memcpy( &pstAccInfo->stAccessInfo.szUserPasswd[0], pBuf+dOffset+2, MAX_USERPWD_SIZE-1 );
					pstAccInfo->stAccessInfo.szUserPasswd[MAX_USERPWD_SIZE-1] = 0x00;
					pstAccInfo->stAccessInfo.ucUserPasswdF = DEF_FLAG_ON;
				}
				else {
					memcpy( &pstAccInfo->stAccessInfo.szUserPasswd[0], pBuf+dOffset+2, usLength-2 );
					pstAccInfo->stAccessInfo.szUserPasswd[usLength-2] = 0x00;
					pstAccInfo->stAccessInfo.ucUserPasswdF = DEF_FLAG_ON;
				}
	
				break;

			case 3:		/* ACCESS : CHAP PASSWORD */
				usLength = pBuf[dOffset+1];
				if( (usLength-2) == (MAX_CHAPPWD_SIZE-1) ) {
					memcpy( &pstAccInfo->stAccessInfo.szCHAPPassed[0], pBuf+dOffset+2, MAX_CHAPPWD_SIZE-1 );
					pstAccInfo->stAccessInfo.szCHAPPassed[MAX_CHAPPWD_SIZE-1] = 0x00;
					pstAccInfo->stAccessInfo.ucCHAPPassedF = DEF_FLAG_ON;
				}
				else
					log_print( LOGN_INFO, "INVALID CHAP PASSWORD LEN:%d", usLength );

				break;

			case 4 :	/* NAS IP Address */ 
				usLength = pBuf[dOffset+1];
                if( usLength == 6 ) {
                    memcpy( &pstAccInfo->uiNASIP, pBuf+dOffset+2, usLength-2 );
                    pstAccInfo->uiNASIP = util_cvtuint( pstAccInfo->uiNASIP );
					pstAccInfo->ucNASIPF = DEF_FLAG_ON;
                }
                else
                    log_print( LOGN_INFO, "INVALID NAS IP ADDRESS LEN:%d", usLength );
                break;

			case 5 :    /* NAS PORT */
                usLength = pBuf[dOffset+1];
                if( usLength == 6 ) {
                    memcpy( &pstAccInfo->dNASPort, pBuf+dOffset+2, usLength-2 );
                    pstAccInfo->dNASPort = util_cvtint( pstAccInfo->dNASPort );
					pstAccInfo->ucNASPortF = DEF_FLAG_ON;
                }
                else
                    log_print( LOGN_INFO, "INVALID NAS PORT LEN:%d", usLength );
                break;

			case 6 :	/* Service Type */
				usLength = pBuf[dOffset+1];
                if( usLength == 6 ) {
                    memcpy( &pstAccInfo->dSvcType, pBuf+dOffset+2, usLength-2 );
                    pstAccInfo->dSvcType = util_cvtint( pstAccInfo->dSvcType );
					pstAccInfo->ucSvcTypeF = DEF_FLAG_ON;
                }
                else
                    log_print( LOGN_INFO, "INVALID SERVICE_TYPE LEN:%d", usLength );
				break;

			case 7:		/* ACCESS : FRAMED PROTOCOL */
				usLength = pBuf[dOffset+1];
				if( usLength == 6 ) {
					memcpy( &pstAccInfo->stAccessInfo.uiFramedProto, pBuf+dOffset+2, usLength-2 );
					pstAccInfo->stAccessInfo.uiFramedProto = util_cvtint(pstAccInfo->stAccessInfo.uiFramedProto);
					pstAccInfo->stAccessInfo.ucFramedProtoF = DEF_FLAG_ON;
				}
				else
					log_print( LOGN_INFO, "INVALID FRAMED PROTOCOL LEN:%d", usLength );

				break;
				
			case 8:     /* Framed IP */
                usLength = pBuf[dOffset+1];
                if( usLength == 6 ) {
                    memcpy( &pstAccInfo->uiFramedIP, pBuf+dOffset+2, usLength-2 );
                    pstAccInfo->uiFramedIP = util_cvtuint( pstAccInfo->uiFramedIP );
					pstAccInfo->ucFramedIPF = DEF_FLAG_ON;
                }
                else
                    log_print( LOGN_INFO, "INVALID FRAMED_IP LEN:%d", usLength );

                break;

			case 10:     /* ACCESS : FRAMED ROUTING */
                usLength = pBuf[dOffset+1];
                if( usLength == 6 ) {
                    memcpy( &pstAccInfo->stAccessInfo.uiFramedRout, pBuf+dOffset+2, usLength-2 );
                    pstAccInfo->stAccessInfo.uiFramedRout = util_cvtint(pstAccInfo->stAccessInfo.uiFramedRout);
                    pstAccInfo->stAccessInfo.ucFramedRoutF = DEF_FLAG_ON;
                }
                else
                    log_print( LOGN_INFO, "INVALID FRAMED ROUTING LEN:%d", usLength );

                break;

			case 12:     /* ACCESS : FRAMED MTU */
                usLength = pBuf[dOffset+1];
                if( usLength == 6 ) {
                    memcpy( &pstAccInfo->stAccessInfo.uiFramedMTU, pBuf+dOffset+2, usLength-2 );
                    pstAccInfo->stAccessInfo.uiFramedMTU = util_cvtint(pstAccInfo->stAccessInfo.uiFramedMTU);
                    pstAccInfo->stAccessInfo.ucFramedMTUF = DEF_FLAG_ON;
                }
                else
                    log_print( LOGN_INFO, "INVALID FRAMED MTU LEN:%d", usLength );

                break;

			case 13:     /* ACCESS : FRAMED COMPRESSION */
                usLength = pBuf[dOffset+1];
                if( usLength == 6 ) {
                    memcpy( &pstAccInfo->stAccessInfo.uiFramedComp, pBuf+dOffset+2, usLength-2 );
                    pstAccInfo->stAccessInfo.uiFramedComp = util_cvtint(pstAccInfo->stAccessInfo.uiFramedComp);
                    pstAccInfo->stAccessInfo.ucFramedCompF = DEF_FLAG_ON;
                }
                else
                    log_print( LOGN_INFO, "INVALID FRAMED COMPRESSION LEN:%d", usLength );

                break;

			case 27:     /* ACCESS : SESSION TIMEOUT */
                usLength = pBuf[dOffset+1];
                if( usLength == 6 ) {
                    memcpy( &pstAccInfo->stAccessInfo.uiSessTimeout, pBuf+dOffset+2, usLength-2 );
                    pstAccInfo->stAccessInfo.uiSessTimeout = util_cvtint(pstAccInfo->stAccessInfo.uiSessTimeout);
                    pstAccInfo->stAccessInfo.ucSessTimeoutF = DEF_FLAG_ON;
                }
                else
                    log_print( LOGN_INFO, "INVALID SESSION TIMEOUT LEN:%d", usLength );

                break;

			case 28:     /* ACCESS : IDLE TIMEOUT */
                usLength = pBuf[dOffset+1];
                if( usLength == 6 ) {
                    memcpy( &pstAccInfo->stAccessInfo.uiIdleTimeout, pBuf+dOffset+2, usLength-2 );
                    pstAccInfo->stAccessInfo.uiIdleTimeout = util_cvtint(pstAccInfo->stAccessInfo.uiIdleTimeout);
                    pstAccInfo->stAccessInfo.ucIdleTimeoutF = DEF_FLAG_ON;
                }
                else
                    log_print( LOGN_INFO, "INVALID SESSION TIMEOUT LEN:%d", usLength );
                    
                break;

			case 31:    /* Calling Station ID */
                usLength = pBuf[dOffset+1];
                if( usLength < (MAX_MIN_SIZE+2) ) {
                    memcpy( &pstAccInfo->szMIN[0], pBuf+dOffset+2, usLength-2 );
                    pstAccInfo->szMIN[usLength-2] = 0x00;
					pstAccInfo->ucCallStatIDF = DEF_FLAG_ON;
                }
                else
                    log_print( LOGN_INFO, "INVALID CALL_ST_ID LEN:%d", usLength );

                break;
			
			case 40:    /* Account Status Type */
                usLength = pBuf[dOffset+1];
                if( usLength == 6 ) {
                    memcpy( &pstAccInfo->dAcctStatType, pBuf+dOffset+2, usLength-2 );
                    pstAccInfo->dAcctStatType = util_cvtint( pstAccInfo->dAcctStatType );
					pstAccInfo->ucAcctStatTypeF = DEF_FLAG_ON;
#if 0
					if( pstAccInfo->dAcctStatType==1 )
						Set_StatRad(g_timeindex, STATRAD_ACCOUNTING, STATACCT_TOT_START);
					else if( pstAccInfo->dAcctStatType==2 )
						Set_StatRad(g_timeindex, STATRAD_ACCOUNTING, STATACCT_TOT_STOP);
					else if( pstAccInfo->dAcctStatType==3 )
						Set_StatRad(g_timeindex, STATRAD_ACCOUNTING, STATACCT_TOT_INTERIM);
#endif
                }
                else
                    log_print( LOGN_INFO, "INVALID ACC_ST_TYPE LEN:%d", usLength );

                break;
	
			case 41:	/* Account Delay Type */
				usLength = pBuf[dOffset+1];
                if( usLength == 6 ) {
                    memcpy( &pstAccInfo->uiAcctDelayTime, pBuf+dOffset+2, usLength-2 );
                    pstAccInfo->uiAcctDelayTime = util_cvtuint( pstAccInfo->uiAcctDelayTime );
					pstAccInfo->ucAcctDelayTimeF = DEF_FLAG_ON;
                }
                else
                    log_print( LOGN_INFO, "INVALID ACC_DELAY_TYPE LEN:%d", usLength );

                break;

			case 42: 	/* Account Input Octets(Originating) */
				usLength = pBuf[dOffset+1];
                if( usLength == 6 ) {
                    memcpy( &pstAccInfo->dAcctInOct, pBuf+dOffset+2, usLength-2 );
                    pstAccInfo->dAcctInOct = util_cvtuint( pstAccInfo->dAcctInOct );
					pstAccInfo->ucAcctInOctF = DEF_FLAG_ON;
                }
                else
                    log_print( LOGN_INFO, "INVALID ACC_INPUT_OCTETS LEN:%d", usLength );

                break;

			case 43: 	/* Account Ouput Octets(Terminating) */
				usLength = pBuf[dOffset+1];
                if( usLength == 6 ) {
                    memcpy( &pstAccInfo->dAcctOutOct, pBuf+dOffset+2, usLength-2 );
                    pstAccInfo->dAcctOutOct = util_cvtuint( pstAccInfo->dAcctOutOct );
					pstAccInfo->ucAcctOutOctF = DEF_FLAG_ON;
                }
                else
                    log_print( LOGN_INFO, "INVALID ACC_OUTPUT_OCTETS LEN:%d", usLength );

                break;


			case 44:	/* Account Session ID */
				usLength = pBuf[dOffset+1];
				if( usLength == 10 ) {
					memcpy( &pstAccInfo->llAcctSessID, pBuf+dOffset+2, usLength-2 );
					pstAccInfo->llAcctSessID = util_cvtint64( pstAccInfo->llAcctSessID );
					pstAccInfo->ucAcctSessIDF = DEF_FLAG_ON;
				}
				else
					log_print( LOGN_INFO, "INVALID ACC_SESS_ID LEN:%d", usLength );

				break;

			case 45:	/* Account Authentic */
				usLength = pBuf[dOffset+1];
                if( usLength == 6 ) {
                    memcpy( &pstAccInfo->dAcctAuth, pBuf+dOffset+2, usLength-2 );
                    pstAccInfo->dAcctAuth = util_cvtint( pstAccInfo->dAcctAuth );
					pstAccInfo->ucAcctAuthF = DEF_FLAG_ON;
                }
                else
                    log_print( LOGN_INFO, "INVALID ACC_AUTH LEN:%d", usLength );

                break;
			
			case 46:	/* Account Session Time */
				usLength = pBuf[dOffset+1];
                if( usLength == 6 ) {
                    memcpy( &pstAccInfo->uiAcctSessTime, pBuf+dOffset+2, usLength-2 );
                    pstAccInfo->uiAcctSessTime = util_cvtuint( pstAccInfo->uiAcctSessTime );
					pstAccInfo->ucAcctSessTimeF = DEF_FLAG_ON;
                }
                else
                    log_print( LOGN_INFO, "INVALID ACC_SESS_TIME LEN:%d", usLength );

                break;

			case 47:	/* Account Input Packets */
				usLength = pBuf[dOffset+1];
                if( usLength == 6 ) {
                    memcpy( &pstAccInfo->dAcctInPkt, pBuf+dOffset+2, usLength-2 );
                    pstAccInfo->dAcctInPkt = util_cvtint( pstAccInfo->dAcctInPkt );
					pstAccInfo->ucAcctInPktF = DEF_FLAG_ON;
                }
                else
                    log_print( LOGN_INFO, "INVALID ACC_INPUT_PACKETS LEN:%d", usLength );

                break;

			case 48:	/* Account Output Packets */
				usLength = pBuf[dOffset+1];
                if( usLength == 6 ) {
                    memcpy( &pstAccInfo->dAcctOutPkt, pBuf+dOffset+2, usLength-2 );
                    pstAccInfo->dAcctOutPkt = util_cvtint( pstAccInfo->dAcctOutPkt );
					pstAccInfo->ucAcctOutPktF = DEF_FLAG_ON;
                }
                else
                    log_print( LOGN_INFO, "INVALID ACC_OUTPUT_PACKETS LEN:%d", usLength );

                break;
            
			case 49:	/* Account Terminate Cause */
				usLength = pBuf[dOffset+1];
                if( usLength == 6 ) {
                    memcpy( &pstAccInfo->dAcctTermCause, pBuf+dOffset+2, usLength-2 );
                    pstAccInfo->dAcctTermCause = util_cvtint( pstAccInfo->dAcctTermCause );
					pstAccInfo->ucAcctTermCauseF = DEF_FLAG_ON;
                }
                else
                    log_print( LOGN_INFO, "INVALID ACC_TERM_CAUSE LEN:%d", usLength );

                break;


			case 55:	/* Event Time Stamp */
				usLength = pBuf[dOffset+1];
                if( usLength == 6 ) {
                    memcpy( &pstAccInfo->uiEventTime, pBuf+dOffset+2, usLength-2 );
					pstAccInfo->uiEventTime = util_cvtuint( pstAccInfo->uiEventTime );
					pstAccInfo->ucEventTimeF = DEF_FLAG_ON;
                }
				else
                    log_print( LOGN_INFO, "INVALID EVENT_TIME LEN:%d", usLength );

				break;
		
			case 61:	/* NAS Port Type */
				usLength = pBuf[dOffset+1];
                if( usLength == 6 ) {
                    memcpy( &pstAccInfo->dNASPortType, pBuf+dOffset+2, usLength-2 );
                    pstAccInfo->dNASPortType = util_cvtint( pstAccInfo->dNASPortType );
					pstAccInfo->ucNASPortTypeF = DEF_FLAG_ON;
                }
                else
                    log_print( LOGN_INFO, "INVALID NAS_PORT_TYPE LEN:%d", usLength );

                break;

			case 85:    /* Acct-Interim-Interval */
                usLength = pBuf[dOffset+1];
                if( usLength == 6 ) {
                    memcpy( &pstAccInfo->dAcctInterim, pBuf+dOffset+2, usLength-2 );
                    pstAccInfo->dAcctInterim = util_cvtuint( pstAccInfo->dAcctInterim );
                    pstAccInfo->ucAcctInterimF = DEF_FLAG_ON;
                }
                else
                    log_print( LOGN_INFO, "INVALID FRAMED_IP LEN:%d", usLength );
                break;
			
			case 87:	/* NAS Port ID */
				usLength = pBuf[dOffset+1];
                if( usLength >= 3 && usLength < (MAX_PORT_SIZE + 2) ) {
                    memcpy( &pstAccInfo->szNASPortID[0], pBuf+dOffset+2, usLength-2 );
                    pstAccInfo->szNASPortID[usLength-2] = 0x00;
					pstAccInfo->ucNASPortIDF = DEF_FLAG_ON;
                }
                else
                    log_print( LOGN_INFO, "INVALID NAS_PORT_ID LEN:%d", usLength );
                    
                break;
	

			case 26:	/* Vendor Specific */

				switch( pBuf[dOffset+6] ) {
					case 7:		/* Home Agent IP Address */
						usLength = pBuf[dOffset+7];
                        if( usLength == 6 ) {
                            memcpy( &pstAccInfo->uiHAIP, pBuf+dOffset+8, usLength-2 );
                            pstAccInfo->uiHAIP = util_cvtuint( pstAccInfo->uiHAIP );
							pstAccInfo->ucHAIPF = DEF_FLAG_ON;
                        }
                        else
                            log_print( LOGN_INFO, "INVALID HOME_AGENT_IP LEN:%d", usLength );

                        break;
			
					case 9:		/* PCF IP Address */
						usLength = pBuf[dOffset+7];
                        if( usLength == 6 ) {
                            memcpy( &pstAccInfo->uiPCFIP, pBuf+dOffset+8, usLength-2 );
                            pstAccInfo->uiPCFIP = util_cvtuint( pstAccInfo->uiPCFIP );
							pstAccInfo->ucPCFIPF = DEF_FLAG_ON;
                        }
                        else
                            log_print( LOGN_INFO, "INVALID PCF_IP LEN:%d", usLength );

                        break;	

					case 10:    /* BS/MSC ID */
                        usLength = pBuf[dOffset+7];
                        if( usLength == 14 ) {
                            memcpy( &pstAccInfo->szBSID[0], pBuf+dOffset+8, usLength-2 );
                            pstAccInfo->szBSID[usLength-2] = 0x00;
							pstAccInfo->ucBSIDF = DEF_FLAG_ON;
                        }
                        else
                            log_print( LOGN_INFO, "INVALID BS_MSC_ID LEN:%d", usLength );
   
                        break;

					case 11:	/* USER ZONE */
						usLength = pBuf[dOffset+7];
                        if( usLength == 6 ) {
                            memcpy( &pstAccInfo->dUserID, pBuf+dOffset+8, usLength-2 );
                            pstAccInfo->dUserID = util_cvtint( pstAccInfo->dUserID );
							pstAccInfo->ucUserIDF = DEF_FLAG_ON;
                        }
                        else
                            log_print( LOGN_INFO, "INVALID USER_ID LEN:%d", usLength );

                        break;
				
					case 12:    /* Forward Mux Option */
                        usLength = pBuf[dOffset+7];
                        if( usLength == 6 ) {
                            memcpy( &pstAccInfo->dFwdFCHMux, pBuf+dOffset+8, usLength-2 );
                            pstAccInfo->dFwdFCHMux = util_cvtint( pstAccInfo->dFwdFCHMux );
							pstAccInfo->ucFwdFCHMuxF = DEF_FLAG_ON;
                        }
                        else
                            log_print( LOGN_INFO, "INVALID FORWARD_MUX_OPTION LEN:%d", usLength );

                        break;
                
					case 13:	/* Reverse Mux Option */	
						usLength = pBuf[dOffset+7];
                        if( usLength == 6 ) {
                            memcpy( &pstAccInfo->dRevFCHMux, pBuf+dOffset+8, usLength-2 );
                            pstAccInfo->dRevFCHMux = util_cvtint( pstAccInfo->dRevFCHMux );
							pstAccInfo->ucRevFCHMuxF = DEF_FLAG_ON;
                        }
                        else    
                            log_print( LOGN_INFO, "INVALID REVERSE_MUX_OPTION LEN:%d", usLength );
                        
                        break;
		
					case 16:    /* Service Option */
                        usLength = pBuf[dOffset+7];
                        if( usLength == 6 ) {
                            memcpy( &pstAccInfo->dSvcOpt, pBuf+dOffset+8, usLength-2 );
                            pstAccInfo->dSvcOpt = util_cvtint( pstAccInfo->dSvcOpt );
							pstAccInfo->ucSvcOptF = DEF_FLAG_ON;
                        }
                        else
                            log_print( LOGN_INFO, "INVALID SVC_OPT LEN:%d", usLength );
                    
                        break;

					case 17:	/* Forward Traffic Type */
						usLength = pBuf[dOffset+7];
                        if( usLength == 6 ) {
                            memcpy( &pstAccInfo->dFwdTrafType, pBuf+dOffset+8, usLength-2 );
                            pstAccInfo->dFwdTrafType = util_cvtint( pstAccInfo->dFwdTrafType );
							pstAccInfo->ucFwdTrafTypeF = DEF_FLAG_ON;
                        }
                        else    
                            log_print( LOGN_INFO, "INVALID FORWARD_TRAFFIC_TYPE LEN:%d", usLength );

                        break;
				
					case 18:    /* Reverse Traffic Type */
                        usLength = pBuf[dOffset+7];
                        if( usLength == 6 ) {
                            memcpy( &pstAccInfo->dRevTrafType, pBuf+dOffset+8, usLength-2 );
                            pstAccInfo->dRevTrafType = util_cvtint( pstAccInfo->dRevTrafType );
							pstAccInfo->ucRevTrafTypeF = DEF_FLAG_ON;
                        }
                        else
                            log_print( LOGN_INFO, "INVALID REVERSE_TRAFFIC_TYPE LEN:%d", usLength );

                        break;  
			
					case 19:	/* Fundamental Frame Size */
						usLength = pBuf[dOffset+7];
                        if( usLength == 6 ) {
                            memcpy( &pstAccInfo->dFCHSize, pBuf+dOffset+8, usLength-2 );
                            pstAccInfo->dFCHSize = util_cvtint( pstAccInfo->dFCHSize );
							pstAccInfo->ucFCHSizeF = DEF_FLAG_ON;
                        }
                        else
                            log_print( LOGN_INFO, "INVALID FCH_FRAME_SIZE LEN:%d", usLength );

                        break;
					
					case 20:	/* Forward Fundamental RC */
						usLength = pBuf[dOffset+7];
                        if( usLength == 6 ) {
                            memcpy( &pstAccInfo->dFwdFCHRC, pBuf+dOffset+8, usLength-2 );
                            pstAccInfo->dFwdFCHRC = util_cvtint( pstAccInfo->dFwdFCHRC );
							pstAccInfo->ucFwdFCHRCF = DEF_FLAG_ON;
                        }
                        else
                            log_print( LOGN_INFO, "INVALID FORWARD_FCH_RC LEN:%d", usLength );
                        
                        break;
			
					case 21:	/* Reverse Fundamental RC */
						usLength = pBuf[dOffset+7];
                        if( usLength == 6 ) {
                            memcpy( &pstAccInfo->dRevFCHRC, pBuf+dOffset+8, usLength-2 );
                            pstAccInfo->dRevFCHRC = util_cvtint( pstAccInfo->dRevFCHRC );
							pstAccInfo->ucRevFCHRCF = DEF_FLAG_ON;
                        }
                        else
                            log_print( LOGN_INFO, "INVALID REVERSE_FCH_RC LEN:%d", usLength );

                        break;
		
					case 22:	/* IP Technology */
						usLength = pBuf[dOffset+7];
                        if( usLength == 6 ) {
                            memcpy( &pstAccInfo->dIPTech, pBuf+dOffset+8, usLength-2 );
                            pstAccInfo->dIPTech = util_cvtint( pstAccInfo->dIPTech );
							pstAccInfo->ucIPTechF = DEF_FLAG_ON;
                        }
                        else
                            log_print( LOGN_INFO, "INVALID IP_TECH LEN:%d", usLength );

                        break;
            
					case 23:	/* Compulsory Tunnel Indicator */	
						usLength = pBuf[dOffset+7];
                        if( usLength == 6 ) {
                            memcpy( &pstAccInfo->dCompTunnelInd, pBuf+dOffset+8, usLength-2 );
                            pstAccInfo->dCompTunnelInd = util_cvtint( pstAccInfo->dCompTunnelInd );
							pstAccInfo->ucCompTunnelIndF = DEF_FLAG_ON;
                        }
                        else
                            log_print( LOGN_INFO, "INVALID COM_TUNNEL_INDICATOR LEN:%d", usLength );

                        break;
		
					case 24:	/* Release Indicator */
						usLength = pBuf[dOffset+7];
                        if( usLength == 6 ) {
                            memcpy( &pstAccInfo->dReleaseInd, pBuf+dOffset+8, usLength-2 );
                            pstAccInfo->dReleaseInd = util_cvtint( pstAccInfo->dReleaseInd );
							pstAccInfo->ucReleaseIndF = DEF_FLAG_ON;
                        }
                        else
                            log_print( LOGN_INFO, "INVALID RELEASE_INDICATOR LEN:%d", usLength );

                        break;

					case 25:	/* Bad PPP Frame Count */
						usLength = pBuf[dOffset+7];
                        if( usLength == 6 ) {
                            memcpy( &pstAccInfo->dBadPPPFrameCnt, pBuf+dOffset+8, usLength-2 );
                            pstAccInfo->dBadPPPFrameCnt = util_cvtint( pstAccInfo->dBadPPPFrameCnt ); 
							pstAccInfo->ucBadPPPFrameCntF = DEF_FLAG_ON;
                        }
                        else    
                            log_print( LOGN_INFO, "INVALID BAD_PPP_FRAME_CNT LEN:%d", usLength );

                        break;
			
					case 30:	/* Number of Active Transitions */
						usLength = pBuf[dOffset+7];
                        if( usLength == 6 ) {
                            memcpy( &pstAccInfo->dNumAct, pBuf+dOffset+8, usLength-2 );
                            pstAccInfo->dNumAct = util_cvtint( pstAccInfo->dNumAct );
							pstAccInfo->ucNumActF = DEF_FLAG_ON;
                        }
                        else
                            log_print( LOGN_INFO, "INVALID NUM_OF_ACTIVE_TRANSITIONS LEN:%d", usLength );

                        break;
            
					case 31:	/* SDB Octet Count(Teminating) */
						usLength = pBuf[dOffset+7];
                        if( usLength == 6 ) {
                            memcpy( &pstAccInfo->dTermSDBOctCnt, pBuf+dOffset+8, usLength-2 );
                            pstAccInfo->dTermSDBOctCnt = util_cvtint( pstAccInfo->dTermSDBOctCnt );
							pstAccInfo->ucTermSDBOctCntF = DEF_FLAG_ON;
                        }
                        else
                            log_print( LOGN_INFO, "INVALID SDB_OCTET_CNT_TERM LEN:%d", usLength );
                        
                        break;
	
					case 32:    /* SDB Octet Count(Original) */
                        usLength = pBuf[dOffset+7];
                        if( usLength == 6 ) {
                            memcpy( &pstAccInfo->dOrgSDBOctCnt, pBuf+dOffset+8, usLength-2 );
                            pstAccInfo->dOrgSDBOctCnt = util_cvtint( pstAccInfo->dOrgSDBOctCnt );
							pstAccInfo->ucOrgSDBOctCntF = DEF_FLAG_ON;
                        }
                        else
                            log_print( LOGN_INFO, "INVALID SDB_OCTET_CNT_ORIG LEN:%d", usLength );

                        break;

					case 33:    /* Number of SDBs(Terminating) */
                        usLength = pBuf[dOffset+7];
                        if( usLength == 6 ) {
                            memcpy( &pstAccInfo->dTermNumSDB, pBuf+dOffset+8, usLength-2 );
                            pstAccInfo->dTermNumSDB = util_cvtint( pstAccInfo->dTermNumSDB );
							pstAccInfo->ucTermNumSDBF = DEF_FLAG_ON;
                        }
                        else
                            log_print( LOGN_INFO, "INVALID NUM_OF_SDBS_TERM LEN:%d", usLength );

                        break;
                    
					case 34:    /* Number of SDBs(Originating) */
                        usLength = pBuf[dOffset+7];
                        if( usLength == 6 ) {
                            memcpy( &pstAccInfo->dOrgNumSDB, pBuf+dOffset+8, usLength-2 );
                            pstAccInfo->dOrgNumSDB = util_cvtint( pstAccInfo->dOrgNumSDB );
							pstAccInfo->ucOrgNumSDBF = DEF_FLAG_ON;
                        }
                        else
                            log_print( LOGN_INFO, "INVALID NUM_OF_SDBS_ORIG LEN:%d", usLength );

                        break;

					case 36:	/* IP Quality of Service */
						usLength = pBuf[dOffset+7];
                        if( usLength == 6 ) {
                            memcpy( &pstAccInfo->dIPQoS, pBuf+dOffset+8, usLength-2 );
                            pstAccInfo->dIPQoS = util_cvtint( pstAccInfo->dIPQoS );
							pstAccInfo->ucIPQoSF = DEF_FLAG_ON;
                        }
                        else
                            log_print( LOGN_INFO, "INVALID IP_QOS LEN:%d", usLength );

                        break;

					case 39:	/* Airlink Quality of Service */
						usLength = pBuf[dOffset+7];
                        if( usLength == 6 ) {
                            memcpy( &pstAccInfo->dAirQoS, pBuf+dOffset+8, usLength-2 );
                            pstAccInfo->dAirQoS = util_cvtint( pstAccInfo->dAirQoS );
							pstAccInfo->ucAirQoSF = DEF_FLAG_ON;
                        }
                        else    
                            log_print( LOGN_INFO, "INVALID AIR_QOS LEN:%d", usLength );

                        break;

					case 41:	/* R-P Session ID */
						usLength = pBuf[dOffset+7];
                        if( usLength == 6 ) {
                            memcpy( &pstAccInfo->dRPConnectID, pBuf+dOffset+8, usLength-2 );
                            pstAccInfo->dRPConnectID = util_cvtint( pstAccInfo->dRPConnectID );
							pstAccInfo->ucRPConnectIDF = DEF_FLAG_ON;
                        }
                        else
                            log_print( LOGN_INFO, "INVALID RP_SESSION_ID LEN:%d", usLength );

                        break;
	
					case 43:	/* Number of HDLC Layer bytes received */
						usLength = pBuf[dOffset+7];
                        if( usLength == 6 ) {
                            memcpy( &pstAccInfo->dRcvHDLCOct, pBuf+dOffset+8, usLength-2 );
                            pstAccInfo->dRcvHDLCOct = util_cvtint( pstAccInfo->dRcvHDLCOct );
							pstAccInfo->ucRcvHDLCOctF = DEF_FLAG_ON;
                        }
                        else
                            log_print( LOGN_INFO, "INVALID NUM_OF_HDLC_BYTES LEN:%d", usLength );

                        break;
							

					case 44:	/* Correlation ID */
						usLength = pBuf[dOffset+7];
						if( usLength == 10 ) {
							memcpy( &pstAccInfo->llCorrelID, pBuf+dOffset+8, usLength-2 );
							pstAccInfo->llCorrelID = util_cvtint64( pstAccInfo->llCorrelID );
							pstAccInfo->ucCorrelationIDF = DEF_FLAG_ON;
						}
						else
                    		log_print( LOGN_INFO, "INVALID CORREL_ID LEN:%d", usLength );

						break;
					
					case 46:	/* In-Bound Mobile IP Signaling Octet Count */
						usLength = pBuf[dOffset+7];
                        if( usLength == 6 ) {
                            memcpy( &pstAccInfo->dInMIPSigCnt, pBuf+dOffset+8, usLength-2 );
                            pstAccInfo->dInMIPSigCnt = util_cvtint( pstAccInfo->dInMIPSigCnt );
							pstAccInfo->ucInMIPSigCntF = DEF_FLAG_ON;
                        }
                        else
                            log_print( LOGN_INFO, "INVALID IN_MOBILE_IP_SIGNAL_CNT LEN:%d", usLength );

                        break;

					case 47:    /* Out-Bound Mobile IP Signaling Octet Count */
                        usLength = pBuf[dOffset+7];
                        if( usLength == 6 ) {
                            memcpy( &pstAccInfo->dOutMIPSigCnt, pBuf+dOffset+8, usLength-2 );
                            pstAccInfo->dOutMIPSigCnt = util_cvtint( pstAccInfo->dOutMIPSigCnt );
							pstAccInfo->ucOutMIPSigCntF = DEF_FLAG_ON;
                        }
                        else
                            log_print( LOGN_INFO, "INVALID OUT_MOBILE_IP_SIGNAL_CNT LEN:%d", usLength );

                        break;
	
					case 48:    /* Session Continue */
                        usLength = pBuf[dOffset+7];
                        if( usLength == 6 ) {
                            memcpy( &pstAccInfo->uiSessContinue, pBuf+dOffset+8, usLength-2 );
                            pstAccInfo->uiSessContinue = util_cvtuint( pstAccInfo->uiSessContinue );
							pstAccInfo->ucSessContinueF = DEF_FLAG_ON;
#if 0
							if( pstAccInfo->uiSessContinue==1 )
								Set_StatRad(g_timeindex, STATRAD_ACCOUNTING, STATACCT_SESS_CONTINUE);
#endif
                        }
                        else
                            log_print( LOGN_INFO, "INVALID SESSION_CONTINUE LEN:%d", usLength );

                        break;
    
					case 49:	/* Active Time */
						usLength = pBuf[dOffset+7];
                        if( usLength == 6 ) {
                            memcpy( &pstAccInfo->uiActTime, pBuf+dOffset+8, usLength-2 );
                            pstAccInfo->uiActTime = util_cvtuint( pstAccInfo->uiActTime );
							pstAccInfo->ucActTimeF = DEF_FLAG_ON;
                        }
                        else
                            log_print( LOGN_INFO, "INVALID ACTIVE_TIME LEN:%d", usLength );

                        break;	

					case 50:	/* DCCH Frame Format */
						usLength = pBuf[dOffset+7];
                        if( usLength == 6 ) {
                            memcpy( &pstAccInfo->dDCCHSize, pBuf+dOffset+8, usLength-2 );
                            pstAccInfo->dDCCHSize = util_cvtint( pstAccInfo->dDCCHSize );
							pstAccInfo->ucDCCHSizeF = DEF_FLAG_ON;
                        }
                        else
                            log_print( LOGN_INFO, "INVALID DCCH_SIZE LEN:%d", usLength );
                    
                        break;

					case 51:	/* BEGINNING SESSION */
						usLength = pBuf[dOffset+7];
                        if( usLength == 6 ) {
                            memcpy( &pstAccInfo->uiBeginnigSess, pBuf+dOffset+8, usLength-2 );
                            pstAccInfo->uiBeginnigSess= util_cvtint( pstAccInfo->uiBeginnigSess);
                            pstAccInfo->ucBeginningSessF = DEF_FLAG_ON;
                        }
                        else
                            log_print( LOGN_INFO, "INVALID BEGINNING SESSION LEN:%d", usLength );

                        break;
		
					case 52:	/* ESN */
						usLength = pBuf[dOffset+7];
                        if( usLength < (MAX_ESN_SIZE+2)) {
                            memcpy( &pstAccInfo->szESN[0], pBuf+dOffset+8, usLength-2 );
                            pstAccInfo->szESN[usLength-2] = 0x00;
							pstAccInfo->ucESNF = DEF_FLAG_ON;
                        }
                        else
                            log_print( LOGN_INFO, "INVALID ESN LEN:%d", usLength );
                    
                        break;

					case 78:	/* Always On Indicator */
						usLength = pBuf[dOffset+7];
                        if( usLength == 6 ) {
                            memcpy( &pstAccInfo->dAlwaysOn, pBuf+dOffset+8, usLength-2 );
                            pstAccInfo->dAlwaysOn = util_cvtint( pstAccInfo->dAlwaysOn );
							pstAccInfo->ucAlwaysOnF = DEF_FLAG_ON;
                        }
                        else
                            log_print( LOGN_INFO, "INVALID ALWAYS_ON LEN:%d", usLength );
                    
                        break;
                
					case 1:	/* C23BIT */
						usLength = pBuf[dOffset+7];
						if( usLength-2 >= sizeof(szC23BIT) ) {
							log_print(LOGN_CRI, "C23BIT TOO LONG!! USLENGTH[%u] SIZEOF[%ld]", usLength, sizeof(szC23BIT));
							memcpy( szC23BIT, pBuf+dOffset+8, sizeof(szC23BIT)-1 );
							szC23BIT[sizeof(szC23BIT)-1] = 0;
						}
						else {
							memcpy( szC23BIT, pBuf+dOffset+8, usLength-2 );
							szC23BIT[usLength-2] = 0x00;
						}

						if( strncasecmp( szC23BIT, "vci_c23=", 8 ) == 0 )
						{
							pstAccInfo->uiC23BIT = atoi(&szC23BIT[8]);
							pstAccInfo->ucC23BITF = DEF_FLAG_ON;
							log_print( LOGN_DEBUG, "C23BIT CHECK [%u][%d]", pstAccInfo->uiC23BIT, pstAccInfo->ucC23BITF );
						}
						else if( strncasecmp( szC23BIT, "hbit=", 5 ) == 0 )
						{
							pstAccInfo->uiHBIT	= atoi(&szC23BIT[5]);
							pstAccInfo->ucHBITF	= DEF_FLAG_ON;
							log_print( LOGN_DEBUG, "HBIT CHECK [%u][%d]", pstAccInfo->uiHBIT, pstAccInfo->ucHBITF );
						}
						else if( strncasecmp( szC23BIT, "pbit=", 5 ) == 0 )
						{
							pstAccInfo->dPBIT	= atoi(&szC23BIT[5]);
							pstAccInfo->ucPBITF	= DEF_FLAG_ON;
							log_print( LOGN_DEBUG, "PBIT CHECK [%u][%d]", pstAccInfo->dPBIT, pstAccInfo->ucPBITF );
						}
						else if( strncasecmp( szC23BIT, "Cbit=", 5 ) == 0 )
						{
							pstAccInfo->dCBIT	= atoi(&szC23BIT[5]);
							pstAccInfo->ucCBITF	= DEF_FLAG_ON;
							log_print( LOGN_DEBUG, "CBIT CHECK [%u][%d]", pstAccInfo->dCBIT, pstAccInfo->ucCBITF );
						}

                        break;

					case 108:	/* SUBNET ATTRIBUTE */

						usLength = pBuf[dOffset+7];

						log_print( LOGN_DEBUG, "START SUBNET ATTRIBUTE LENGTH:%d", usLength );

						if( (usLength - 2) == MAX_SUBNET_SIZE ) {
							 memcpy( pstAccInfo->szSubnet, pBuf+dOffset+8, usLength-2 );
							 //pstAccInfo->szSubnet[usLength-2] = 0x00;
							 pstAccInfo->ucSubnetF = DEF_FLAG_ON;
						}
						else
							log_print( LOGN_INFO, "INVALID SUBNET ATTRIBUTE LENGTH:%u", usLength );

						break;

					default:

						break;
				}

				break;

			default:

				break;
		}

		if( pBuf[dOffset+1] <= 0 ) {
			log_print( LOGN_DEBUG, "INVALID ATTRIBUTE:%d LENGTH:%d", pBuf[dOffset], pBuf[dOffset+1] );
			return -1;
		}

		dOffset += pBuf[dOffset+1];
	}

	return 1;
}


