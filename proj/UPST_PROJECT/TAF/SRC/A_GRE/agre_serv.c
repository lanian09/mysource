/**
 * Include headers
 */
// TOP
#include "common_stg.h"
#include "commdef.h"			/* Capture_Header_Msg */

// LIB
#include "loglib.h"

// TAF headers
#include "Analyze_Ext_Abs.h"	/* INFO_PPP */
#include "mmdb_psess.h"			/* T_PSESS_BUF */
#include "arp_head.h"			/* DEF_LCP_~ + a */
#include "PPP_header.h"			/* PPP_~ */

// .
#include "agre_serv.h"

/**
 * Declare variables
 */
INFO_PPP            g_PPP;
T_PSESS_BUF         SESS_BUF;

/**
 *	Declare external func.
 */
/* ANZ_LIB */
extern int Analyze_PPP(PUCHAR pBuf, WORD wSize, INFO_PPP *pInfo, struct slcompress *pComp);
extern int dump_DebugString(char *debug_str, char *s, int len);

/**
 *	Implement func.
 */
int MergePPPData( PSESS_DATA *pPSessData, unsigned char *pData, int dSize, INFO_ETH *pstEth, Capture_Header_Msg *pstCAP )
{
    int             ret;
    int             dStartPos, dNextStartPos, d7EOmit, d7EDup;
    unsigned char   *pBuffer;
    T_PPP_INFO      *pstPPPInfo;
    struct slcompress   *m_comp;

    if( pstCAP->bRtxType == DEF_FROM_CLIENT) {
        pstPPPInfo  = &pPSessData->stETH1;
        pBuffer     = pPSessData->SESS_BUF.stPPPBuf.szBuf1;
        m_comp      = &pPSessData->SESS_BUF.stPPPBuf.m_comp1;
    }                               
    else if( pstCAP->bRtxType == DEF_FROM_SERVER ) {
        pstPPPInfo  = &pPSessData->stETH2;
        pBuffer     = pPSessData->SESS_BUF.stPPPBuf.szBuf2;
        m_comp      = &pPSessData->SESS_BUF.stPPPBuf.m_comp2; 
    }                       
    else {                  
        log_print( LOGN_CRI, "Unknown UpDownType[%d]", pstCAP->bRtxType );
        return -1;
    }                       
                            
    pPSessData->SESS_BUF.stPPPBuf.LastUpdateTime = pstCAP->curtime;
                            
    /* BUFFER VALIDATION CHECK */
    if( pstPPPInfo->siLength < 0 || pstPPPInfo->siLength > MAX_PPP_SIZE
        || pBuffer == 0 || (pBuffer+pstPPPInfo->siLength) == 0 ) {
        /* BUFFER ERROR */
        pstPPPInfo->siLength    = 0;
        pstPPPInfo->dPPPState   = X;
        pstPPPInfo->bMrgState   = 0;

        return -2;
    }

    /* GRE seq check */
    if( pstEth->stGRE.dwSeqNum > 0 && pstPPPInfo->uiSeq > 0 && (pstPPPInfo->uiSeq+1) != pstEth->stGRE.dwSeqNum ) {
        log_print( LOGN_INFO, "GRE seq error, Key[%u] UpDown[%d] last_seq[%u] cur_seq[%u]",
                          pPSessData->key.uiKey, pstCAP->bRtxType, pstPPPInfo->uiSeq, pstEth->stGRE.dwSeqNum );
    }

    pstPPPInfo->uiSeq = pstEth->stGRE.dwSeqNum;

    /* CHECK 7E STATE & WRITE COMPLETE PPP PACKET */
    dStartPos = 0;

	while( dStartPos < dSize )
    {
        ret = Get7EState( pData + dStartPos, dSize - dStartPos, &pstPPPInfo->dPPPState, &dNextStartPos, &d7EOmit, &d7EDup );
        if( ret < 0 ) {
            pstPPPInfo->bMrgState = 1;
            log_print( LOGN_WARN, "ERROR IN %s", __FUNCTION__ );

            return -3;
        }
        else if( d7EOmit ) {
            pBuffer[0]              = 0x7E;
            pstPPPInfo->siLength    = 1;
        }

        switch( pstPPPInfo->dPPPState )
        {
            case F_DATA_F:

                ret = CopyPacket( pBuffer, &pstPPPInfo->siLength, pData+dStartPos+d7EDup, dNextStartPos-d7EDup );
                if( ret < 0 ) {
                    pstPPPInfo->bMrgState = 1;      // abnormal
                    log_print( LOGN_INFO, "F_DATA_F OrgSize[%d] InSize[%d] 7EDup[%d] NextPos[%d]",
                                       pstPPPInfo->siLength, dSize, d7EDup, dNextStartPos );
                }
                else {
                    if( pstPPPInfo->bMrgState == 0 )
                        ProcPPP( pPSessData, pBuffer, pstPPPInfo->siLength, m_comp, pstEth, pstCAP );
                    else
                        log_print( LOGN_DEBUG, "Packet Discard b/c MergeState[%d] LastSeq[%u]",
                                            pstPPPInfo->bMrgState, pstPPPInfo->uiSeq );
                }

                pstPPPInfo->siLength = 0;
                pstPPPInfo->dPPPState = X;
                pstPPPInfo->bMrgState = 0;

                break;

            case F_DATA:
            case F:
                ret = CopyPacket( pBuffer, &pstPPPInfo->siLength, pData+dStartPos+d7EDup, dNextStartPos-d7EDup );
                if( ret < 0 ) {
                    pstPPPInfo->bMrgState = 1;      // abnormal
                    log_print( LOGN_INFO, "F_DATA_F OrgSize[%d] InSize[%d] 7EDup[%d] NextPos[%d]",
                                        pstPPPInfo->siLength, dSize, d7EDup, dNextStartPos );
                }
                break;

            default:
                log_print( LOGN_WARN, "Unknown PPPState[%u]", pstPPPInfo->dPPPState );
                break;
        }

        dStartPos += dNextStartPos;
    }

    return 0;
}

/*******************************************************************************
 * return value
 *  -1  error
 *   0  partial PPP
 *   1  7e + data + 7e  : completed
 *   2  7e + data + 7e 7e ~  : continue
*******************************************************************************/
int Get7EState( unsigned char *pBuf, short siSize, char *nPPPState, int *nNextPos, int *n7EOmit, int *n7EDup)
{
    int     i;
    int     dRet = 0;

    *n7EOmit    = 0;
    *n7EDup     = 0;
    *nNextPos   = siSize;

    i = 0;

    while( i < siSize )
    {
        if( pBuf[i] == 0x7E ) {
            switch( *nPPPState )
            {
                case X:
                    *nPPPState = F;
                    break;

                case F_DATA:
                    *nPPPState = F_DATA_F;
                    dRet = 1;
                    break;

                case F_DATA_F:
                    *nNextPos = i;
                    dRet = 2;
                    break;

                case F:
                    *n7EDup = *n7EDup + 1;
                    break;

                default:
                    // error case....
                    dRet = -1;
                    break;
            }
        }
        else {
			switch( *nPPPState )
            {
                case F:
                    *nPPPState = F_DATA;
                    break;

                case F_DATA:
                    *nPPPState = F_DATA;
                    break;

                case X:
                    *n7EOmit = 1;
                    *nPPPState = F_DATA;
                    break;

                case F_DATA_F:
                    *nNextPos = i;
                    dRet = 2;
                    break;

                default:
                    // error case....
                    dRet = -1;
                    break;
            }
        }

        if( dRet < 0 || dRet > 1 )
            break;

        i++;
    }

    return dRet;
}

int ProcPPP( PSESS_DATA *pPSessData, unsigned char *pBuf, short siSize, struct slcompress *m_comp, INFO_ETH *pstEth, Capture_Header_Msg *pstCAP )
{
    int ret;

    //pPSessData->OpTime = time(NULL);
    pPSessData->LastUpdateTime = pstCAP->curtime;
    pPSessData->LastUpdateMTime = pstCAP->ucurtime;

    memset( &g_PPP, 0x00, DEF_INFOPPP_SIZE );

    ret = Analyze_PPP( pBuf, siSize, &g_PPP, m_comp );
    if( ret < 0 ) {
        log_print( LOGN_CRI, "[ERROR:%d] Analyze_PPP() MIN[%s] Key[%u] UpDown[%d] Seq[%u]",
                          ret, pPSessData->szMIN, pPSessData->key.uiKey, pstCAP->bRtxType, pstEth->stGRE.dwSeqNum );

		dump_DebugString( "ERR PPP", pBuf, siSize );

        if( pstCAP->bRtxType == DEF_FROM_CLIENT )
            pPSessData->uiUpAnaErrFrames++;
        else
            pPSessData->uiDownAnaErrFrames++;

        return -1;
    }
    else
        log_print( LOGN_INFO, "MIN[%s] Key[%u] PPP Size[%d] Protocol[0x%04x] Code[0x%02x]",
                       pPSessData->szMIN, pPSessData->key.uiKey, siSize, g_PPP.stPPP.wProtocol, g_PPP.stPPP.ucCode );

    if( g_PPP.stPPP.bFCSError ) {
        log_print( LOGN_DEBUG, "FCS ERROR, MIN[%s] Key[%u] UpDown[%d] Seq[%u]",
                            pPSessData->szMIN, pPSessData->key.uiKey, pstCAP->bRtxType, pstEth->stGRE.dwSeqNum );

        if( pstCAP->bRtxType == DEF_FROM_CLIENT ) {
            pPSessData->uiUpFCSErrFrames++;
            pPSessData->uiUpFCSErrBytes += siSize;
        }
        else {
            pPSessData->uiDownFCSErrFrames++;
            pPSessData->uiDownFCSErrBytes += siSize;
        }

        return -1;
    }

    if( pstCAP->bRtxType == DEF_FROM_CLIENT ) {
        pPSessData->uiUpPPPFrames += 1;
        pPSessData->uiUpPPPBytes += siSize;
    }
    else {
        pPSessData->uiDownPPPFrames += 1;
        pPSessData->uiDownPPPBytes += siSize;
    }

	/*
	 DEF_LCP_CONF_REQ UP : pPSessData->usPPPFlag |= 0x0001 
	 DEF_LCP_CONF_REQ DN : pPSessData->usPPPFlag |= 0x0004
	 DEF_LCP_CONF_ACK DN : pPSessData->usPPPFlag |= 0x0008  
	 DEF_LCP_CONF_ACK UP : pPSessData->usPPPFlag |= 0x0002

	 DEF_LCP_TERM_REQ UP : pPSessData->usPPPFlag |= 0x0010
	 DEF_LCP_TERM_REQ DN : pPSessData->usPPPFlag |= 0x0040 
	 DEF_LCP_TERM_ACK DN : pPSessData->usPPPFlag |= 0x0080
	 DEF_LCP_TERM_ACK UP : pPSessData->usPPPFlag |= 0x0020

	 CHAP : pPSessData->usPPPFlag |= 0x0100
	 DEF_CHAP_CHAL : pPSessData->usPPPFlag |= 0x0200
	 DEF_CHAP_RESP : pPSessData->usPPPFlag |= 0x0400
	 DEF_CHAP_SUCC : pPSessData->usPPPFlag |= 0x0800

	 DEF_IPCP_CONF_REQ UP : pPSessData->usPPPFlag |= 0x1000
	 DEF_IPCP_CONF_REQ DN : pPSessData->usPPPFlag |= 0x4000
	 DEF_IPCP_CONF_ACK DN : pPSessData->usPPPFlag |= 0x8000
	 DEF_IPCP_CONF_ACK UP :	pPSessData->usPPPFlag |= 0x2000
	*/

	switch( g_PPP.stPPP.wProtocol )
    {
        case PPP_LCP:   /* LCP */

            if( g_PPP.stPPP.ucCode == DEF_LCP_CONF_REQ ) {          /* CONFIG-REQ */

                if( pstCAP->bRtxType == DEF_FROM_CLIENT ) {
					pPSessData->ucUpLCPReqCount++;

					if( pPSessData->UpLCP.StartTime == 0 ) {
						pPSessData->UpLCPStartTime  	= pstCAP->curtime;
                        pPSessData->UpLCPStartMTime 	= pstCAP->ucurtime;

						pPSessData->UpLCP.StartTime     = pstCAP->curtime;
                        pPSessData->UpLCP.StartMTime    = pstCAP->ucurtime;
					}

                    pPSessData->usPPPFlag |= 0x0001;

                    pPSessData->UpLCP.ucFlag        |= 0x01;
                    pPSessData->UpLCP.uiID          = g_PPP.stPPP.ucID;
                    pPSessData->UpLCP.ucReqMsgCode  = g_PPP.stPPP.ucCode;
                    pPSessData->UpLCP.ucReqCount++;

                    /* TO HANDLE WHEN PACKET IS INVERTED */
					/* 아래의 경우는 발생하지 않음. ID를 비교하기 때문에 PACKET이 역전되는 경우는 처리 못함 */
                    if( pPSessData->UpLCP.ucFlag == 0x03 ) {
                        Report_SIGLog( NUM_UP_LCP, pPSessData->UpLCP.ucReqMsgCode, pPSessData);
                        memset( &pPSessData->UpLCP, 0x00, DEF_TSESS_SIZE );
                    }
                }
				else {   /* DOWN */
					pPSessData->ucDownLCPReqCount++;
					if( pPSessData->DownLCP.StartTime == 0 ) {
						pPSessData->DownLCPStartTime    = pstCAP->curtime;
                        pPSessData->DownLCPStartMTime   = pstCAP->ucurtime;

						pPSessData->DownLCP.StartTime 	= pstCAP->curtime;
                        pPSessData->DownLCP.StartMTime 	= pstCAP->ucurtime;
					}

                    pPSessData->usPPPFlag |= 0x0004;

                    pPSessData->DownLCP.ucFlag 			|= 0x01;
                    pPSessData->DownLCP.uiID 			= g_PPP.stPPP.ucID;
                    pPSessData->DownLCP.ucReqMsgCode 	= g_PPP.stPPP.ucCode;
                    pPSessData->DownLCP.ucReqCount++;

                    /* TO HANDLE WHEN PACKET IS INVERED */
					/* 아래의 경우는 발생하지 않음. ID를 비교하기 때문에 PACKET이 역전되는 경우는 처리 못함 */
                    if( pPSessData->DownLCP.ucFlag == 0x03 ) {
                        Report_SIGLog( NUM_DOWN_LCP, pPSessData->DownLCP.ucReqMsgCode, pPSessData );
						memset( &pPSessData->DownLCP, 0x00, DEF_TSESS_SIZE );
                    }
                }
            }
            else if( g_PPP.stPPP.ucCode == DEF_LCP_CONF_ACK ) { /* CONFIG-ACK */
                if( pstCAP->bRtxType == DEF_FROM_CLIENT ) {
					/* CHECK ID 2009.09.06 BY LDH */
					if( pPSessData->DownLCP.uiID != g_PPP.stPPP.ucID ) {
						log_print( LOGN_DEBUG, "NOT MATCHED F_CLI ID I:%s REQ:%u RES:%u", pPSessData->szMIN, pPSessData->DownLCP.uiID, g_PPP.stPPP.ucID );
						break;
					}

                    pPSessData->DownLCP.EndTime 		= pstCAP->curtime;
                    pPSessData->DownLCP.EndMTime 		= pstCAP->ucurtime;

                    pPSessData->DownLCP.ucFlag 			|= 0x02;
                    pPSessData->DownLCP.uiID 			= g_PPP.stPPP.ucID;
                    pPSessData->DownLCP.ucRepMsgCode 	= g_PPP.stPPP.ucCode;

					log_print(LOGN_INFO, "DN_LCP DownLCP.ucFlag:0x%02X UpLCP.ucFlag:0x%02X", pPSessData->DownLCP.ucFlag, pPSessData->UpLCP.ucFlag );

					pPSessData->usPPPFlag |= 0x0008;

					if( pPSessData->DownLCP.ucFlag == 0x03 ) {
                        Report_SIGLog( NUM_DOWN_LCP, pPSessData->DownLCP.ucReqMsgCode, pPSessData );
                        memset( &pPSessData->DownLCP, 0x00, DEF_TSESS_SIZE );
                    }
                }
				else { /* DOWN */
					/* CHECK ID 2009.09.06 BY LDH */
                    if( pPSessData->UpLCP.uiID != g_PPP.stPPP.ucID ) {
                        log_print( LOGN_DEBUG, "NOT MATCHED F_SVR ID I:%s REQ:%u RES:%u", pPSessData->szMIN, pPSessData->UpLCP.uiID, g_PPP.stPPP.ucID );
                        break;
                    }

                    pPSessData->UpLCP.EndTime 		= pstCAP->curtime;
                    pPSessData->UpLCP.EndMTime 		= pstCAP->ucurtime;

                    pPSessData->UpLCP.ucFlag 		|= 0x02;
                    pPSessData->UpLCP.uiID 			= g_PPP.stPPP.ucID;
                    pPSessData->UpLCP.ucRepMsgCode 	= g_PPP.stPPP.ucCode;

					log_print(LOGN_INFO, "UP_LCP DownLCP.ucFlag:0x%02X UpLCP.ucFlag:0x%02X", pPSessData->DownLCP.ucFlag, pPSessData->UpLCP.ucFlag );

					pPSessData->usPPPFlag |= 0x0002;

					if( pPSessData->UpLCP.ucFlag == 0x03 ) {
                        Report_SIGLog( NUM_UP_LCP, pPSessData->UpLCP.ucReqMsgCode, pPSessData );
                        memset( &pPSessData->UpLCP, 0x00, DEF_TSESS_SIZE );
                    }
                }

                if( pPSessData->PPPSetupTime == 0 && (pPSessData->usPPPFlag & 0x000f) == 0x000f) {
                    if( pPSessData->UpLCPStartTime == pPSessData->DownLCPStartTime ) {
                        if( pPSessData->UpLCPStartMTime < pPSessData->DownLCPStartMTime )
                            pPSessData->uiLCPDuration = GetUpTime( pPSessData->UpLCPStartTime, pPSessData->UpLCPStartMTime,
                                                            pstCAP->curtime, pstCAP->ucurtime);
                        else
                            pPSessData->uiLCPDuration = GetUpTime( pPSessData->DownLCPStartTime, pPSessData->DownLCPStartMTime,
                                                            pstCAP->curtime, pstCAP->ucurtime);
                    }
                    else if( pPSessData->UpLCPStartTime < pPSessData->DownLCPStartTime )
                        pPSessData->uiLCPDuration = GetUpTime( pPSessData->UpLCPStartTime, pPSessData->UpLCPStartMTime,
                                                        pstCAP->curtime, pstCAP->ucurtime);
                    else
                        pPSessData->uiLCPDuration = GetUpTime( pPSessData->DownLCPStartTime, pPSessData->DownLCPStartMTime,
                                                        pstCAP->curtime, pstCAP->ucurtime);
                }
            }
			else if( g_PPP.stPPP.ucCode == DEF_LCP_TERM_REQ ) { /* TERM-REQ */
				if( pPSessData->LCPTerm.StartTime == 0 ) {
                	pPSessData->PPPTermTime 		= pstCAP->curtime;
                	pPSessData->PPPTermMTime 		= pstCAP->ucurtime;

					pPSessData->LCPTerm.StartTime 	= pstCAP->curtime;
                    pPSessData->LCPTerm.StartMTime 	=  pstCAP->ucurtime;
				}

                pPSessData->LCPTerm.ucFlag 			|= 0x01;
                pPSessData->LCPTerm.ucReqMsgCode 	= g_PPP.stPPP.ucCode;
                pPSessData->LCPTerm.uiID 			= g_PPP.stPPP.ucID;
                pPSessData->LCPTerm.ucReqCount++;

                if( pstCAP->bRtxType == DEF_FROM_CLIENT ) {
                    pPSessData->usPPPFlag |= 0x0010;

                    if( pPSessData->LCPTerm.ucFlag == 0x03 ) {
                        Report_SIGLog( NUM_UP_LCP, pPSessData->LCPTerm.ucReqMsgCode, pPSessData );
                        memset( &pPSessData->LCPTerm, 0x00, DEF_TSESS_SIZE );
                    }
                }
                else { /* DOWN */
                    pPSessData->usPPPFlag |= 0x0040;

                    if( pPSessData->LCPTerm.ucFlag == 0x03 ) {
                        Report_SIGLog( NUM_DOWN_LCP, pPSessData->LCPTerm.ucReqMsgCode, pPSessData );
                        memset( &pPSessData->LCPTerm, 0x00, DEF_TSESS_SIZE );
                    }
                }
            }
			else if( g_PPP.stPPP.ucCode == DEF_LCP_TERM_ACK ) { /* TERM-ACK */

				/* CHECK ID 2009.09.06 BY LDH */
                if( pPSessData->LCPTerm.uiID != g_PPP.stPPP.ucID ) {
                    log_print( LOGN_INFO, "NOT MATCHED ID REQ:%u RES:%u", pPSessData->LCPTerm.uiID, g_PPP.stPPP.ucID );
                    break;
                }

                pPSessData->LCPTerm.EndTime 		= pstCAP->curtime;
                pPSessData->LCPTerm.EndMTime 		= pstCAP->ucurtime;

                pPSessData->LCPTerm.ucFlag 			|= 0x02;
                pPSessData->LCPTerm.ucRepMsgCode 	= g_PPP.stPPP.ucCode;
                pPSessData->LCPTerm.uiID 			= g_PPP.stPPP.ucID;

                if( pstCAP->bRtxType == DEF_FROM_CLIENT ) {
                    pPSessData->usPPPFlag |= 0x0080;

                    if( pPSessData->LCPTerm.ucFlag == 0x03 ) {
                        Report_SIGLog( NUM_DOWN_LCP, pPSessData->LCPTerm.ucReqMsgCode, pPSessData );
                        memset( &pPSessData->LCPTerm, 0x00, DEF_TSESS_SIZE );
                    }
                }
                else {
                    pPSessData->usPPPFlag |= 0x0020;

                    if( pPSessData->LCPTerm.ucFlag == 0x03 ) {
                        Report_SIGLog( NUM_UP_LCP, pPSessData->LCPTerm.ucReqMsgCode, pPSessData );
                        memset( &pPSessData->LCPTerm, 0x00, DEF_TSESS_SIZE );
                    }
                }
            }
            else if( g_PPP.stPPP.ucCode == DEF_LCP_CONF_NAK ) { /* NAK */
                if( pstCAP->bRtxType == DEF_FROM_CLIENT ) {
					pPSessData->DownLCP.ucNakCount++;

					/* CHECK ID 2009.09.06 BY LDH */
                	if( pPSessData->DownLCP.uiID != g_PPP.stPPP.ucID ) {
                    	log_print( LOGN_INFO, "NOT MATCHED ID REQ:%u RES:%u", pPSessData->DownLCP.uiID, g_PPP.stPPP.ucID );
                    	break;
                	}
					pPSessData->DownLCP.ucRepMsgCode = g_PPP.stPPP.ucCode;
                }
                else {
					pPSessData->UpLCP.ucNakCount++;

					/* CHECK ID 2009.09.06 BY LDH */
                    if( pPSessData->UpLCP.uiID != g_PPP.stPPP.ucID ) {
                        log_print( LOGN_INFO, "NOT MATCHED ID REQ:%u RES:%u", pPSessData->UpLCP.uiID, g_PPP.stPPP.ucID );
                        break;
                    }
					pPSessData->UpLCP.ucRepMsgCode = g_PPP.stPPP.ucCode;
                }
            }
            else if( g_PPP.stPPP.ucCode == DEF_LCP_CONF_REJ ) { /* REJECT */
                if( pstCAP->bRtxType == DEF_FROM_CLIENT ) {
					pPSessData->DownLCP.ucRejCount++;

					/* CHECK ID 2009.09.06 BY LDH */
                    if( pPSessData->DownLCP.uiID != g_PPP.stPPP.ucID ) {
                        log_print( LOGN_INFO, "NOT MATCHED ID REQ:%u RES:%u", pPSessData->DownLCP.uiID, g_PPP.stPPP.ucID );
                        break;
                    }
					pPSessData->DownLCP.ucRepMsgCode = g_PPP.stPPP.ucCode;
                }
                else {
					pPSessData->UpLCP.ucRejCount++;

					/* CHECK ID 2009.09.06 BY LDH */
                    if( pPSessData->UpLCP.uiID != g_PPP.stPPP.ucID ) {
                        log_print( LOGN_INFO, "NOT MATCHED ID REQ:%u RES:%u", pPSessData->UpLCP.uiID, g_PPP.stPPP.ucID );
                        break;
                    }
					pPSessData->UpLCP.ucRepMsgCode = g_PPP.stPPP.ucCode;
                }
            }
			else if( g_PPP.stPPP.ucCode == DEF_LCP_ECHO_REQ ) {
				if( pstCAP->bRtxType == DEF_FROM_CLIENT ) {

					pPSessData->ucUpLCPReqCount++;

                    if( pPSessData->UpLCPEcho.StartTime == 0 ) {
                        pPSessData->UpLCPEcho.StartTime     = pstCAP->curtime;
                        pPSessData->UpLCPEcho.StartMTime    = pstCAP->ucurtime;
                    }

                    pPSessData->UpLCPEcho.ucFlag        |= 0x01;
                    pPSessData->UpLCPEcho.uiID          = g_PPP.stPPP.ucID;
                    pPSessData->UpLCPEcho.ucReqMsgCode  = g_PPP.stPPP.ucCode;
                    pPSessData->UpLCPEcho.ucReqCount++;

                    /* TO HANDLE WHEN PACKET IS INVERTED */
                    /* 아래의 경우는 발생하지 않음. ID를 비교하기 때문에 PACKET이 역전되는 경우는 처리 못함 */
                    if( pPSessData->UpLCPEcho.ucFlag == 0x03 ) {
                        Report_SIGLog( NUM_UP_LCP, pPSessData->UpLCPEcho.ucReqMsgCode, pPSessData);
                        memset( &pPSessData->UpLCPEcho, 0x00, DEF_TSESS_SIZE );
                    }
				}
				else {
					pPSessData->ucDownLCPReqCount++;
                    if( pPSessData->DownLCPEcho.StartTime == 0 ) {
                        pPSessData->DownLCPEcho.StartTime   = pstCAP->curtime;
                        pPSessData->DownLCPEcho.StartMTime  = pstCAP->ucurtime;
                    }

                    pPSessData->DownLCPEcho.ucFlag          |= 0x01;
                    pPSessData->DownLCPEcho.uiID            = g_PPP.stPPP.ucID;
                    pPSessData->DownLCPEcho.ucReqMsgCode    = g_PPP.stPPP.ucCode;
                    pPSessData->DownLCPEcho.ucReqCount++;

                    /* TO HANDLE WHEN PACKET IS INVERED */
                    /* 아래의 경우는 발생하지 않음. ID를 비교하기 때문에 PACKET이 역전되는 경우는 처리 못함 */
                    if( pPSessData->DownLCPEcho.ucFlag == 0x03 ) {
                        Report_SIGLog( NUM_DOWN_LCP, pPSessData->DownLCPEcho.ucReqMsgCode, pPSessData );
                        memset( &pPSessData->DownLCPEcho, 0x00, DEF_TSESS_SIZE );
                    }
				}
			}
			else if( g_PPP.stPPP.ucCode == DEF_LCP_ECHO_REP ) {
				if( pstCAP->bRtxType == DEF_FROM_CLIENT ) {
                    /* CHECK ID 2009.09.06 BY LDH */
                    if( pPSessData->DownLCPEcho.uiID != g_PPP.stPPP.ucID ) {
                        log_print( LOGN_DEBUG, "NOT MATCHED F_CLI ID I:%s REQ:%u RES:%u", pPSessData->szMIN, pPSessData->DownLCPEcho.uiID, g_PPP.stPPP.ucID );
                        break;
                    }

                    pPSessData->DownLCPEcho.EndTime         = pstCAP->curtime;
                    pPSessData->DownLCPEcho.EndMTime        = pstCAP->ucurtime;

                    pPSessData->DownLCPEcho.ucFlag          |= 0x02;
                    pPSessData->DownLCPEcho.uiID            = g_PPP.stPPP.ucID;
                    pPSessData->DownLCPEcho.ucRepMsgCode    = g_PPP.stPPP.ucCode;

                    if( pPSessData->DownLCPEcho.ucFlag == 0x03 ) {
                        Report_SIGLog( NUM_DOWN_LCP, pPSessData->DownLCPEcho.ucReqMsgCode, pPSessData );
                        memset( &pPSessData->DownLCPEcho, 0x00, DEF_TSESS_SIZE );
                    }
                }
                else { /* DOWN */
                    /* CHECK ID 2009.09.06 BY LDH */
                    if( pPSessData->UpLCPEcho.uiID != g_PPP.stPPP.ucID ) {
                        log_print( LOGN_DEBUG, "NOT MATCHED F_SVR ID I:%s REQ:%u RES:%u", pPSessData->szMIN, pPSessData->UpLCPEcho.uiID, g_PPP.stPPP.ucID );
                        break;
                    }

                    pPSessData->UpLCPEcho.EndTime       = pstCAP->curtime;
                    pPSessData->UpLCPEcho.EndMTime      = pstCAP->ucurtime;

                    pPSessData->UpLCPEcho.ucFlag        |= 0x02;
                    pPSessData->UpLCPEcho.uiID          = g_PPP.stPPP.ucID;
                    pPSessData->UpLCPEcho.ucRepMsgCode  = g_PPP.stPPP.ucCode;

                    if( pPSessData->UpLCPEcho.ucFlag == 0x03 ) {
                        Report_SIGLog( NUM_UP_LCP, pPSessData->UpLCPEcho.ucReqMsgCode, pPSessData );
                        memset( &pPSessData->UpLCPEcho, 0x00, DEF_TSESS_SIZE );
                    }
                }
			}
			else if( g_PPP.stPPP.ucCode == DEF_LCP_PROT_REJ || g_PPP.stPPP.ucCode == DEF_LCP_DISC_REQ )
                /* DISCARD-REQUEST */ ;
            else {
                log_print( LOGN_INFO, "Unknown LCP Code[%d]", g_PPP.stPPP.ucCode );
            }

            break;

        case PPP_CHAP:  /* CHAP */

            pPSessData->usPPPFlag |= 0x0100;
            if( g_PPP.stPPP.ucCode == DEF_CHAP_CHAL ) { /* CHALLENGE */
                pPSessData->usPPPFlag |= 0x0200;

				if( pPSessData->CHAPAP.StartTime == 0 ) {
					pPSessData->CHAPAP.StartTime    = pstCAP->curtime;
                    pPSessData->CHAPAP.StartMTime   = pstCAP->ucurtime;

					pPSessData->AuthReqTime     = pstCAP->curtime;
                    pPSessData->AuthReqMTime    = pstCAP->ucurtime;
				}

                pPSessData->CHAPAP.ucFlag       |= 0x01;
                pPSessData->CHAPAP.uiID         = g_PPP.stPPP.ucID;
                pPSessData->CHAPAP.ucReqMsgCode = g_PPP.stPPP.ucCode;
                pPSessData->CHAPAP.ucReqCount++;

                if( pPSessData->CHAPAP.ucFlag == 0x03 ) {
                    Report_SIGLog( NUM_CHAP, pPSessData->CHAPAP.ucReqMsgCode, pPSessData );
                    memset( &pPSessData->CHAPAP, 0x00, DEF_TSESS_SIZE );
                }
            }
            else if( g_PPP.stPPP.ucCode == DEF_CHAP_RESP ) {    /* RESPONSE */
				/* CHECK ID 2009.09.06 BY LDH */
                if( pPSessData->CHAPAP.uiID != g_PPP.stPPP.ucID ) {
                    log_print( LOGN_INFO, "NOT MATCHED ID REQ:%u RES:%u", pPSessData->CHAPAP.uiID, g_PPP.stPPP.ucID );
                    break;
                }

                pPSessData->usPPPFlag |= 0x0400;

                pPSessData->CHAPAP.ucRepMsgCode = g_PPP.stPPP.ucCode;
                pPSessData->CHAPAP.uiID         = g_PPP.stPPP.ucID;

                pPSessData->ucAuthResultCode    = g_PPP.stPPP.ucCode;
                sprintf( pPSessData->szAuthUserName, "%s", g_PPP.stPPP.szUserName );

                pPSessData->CHAPResTime     = pstCAP->curtime;
                pPSessData->CHAPResMTime    = pstCAP->ucurtime;
            }
			else if( g_PPP.stPPP.ucCode == DEF_CHAP_SUCC ) {    /* SUCCESS */
				/* CHECK ID 2009.09.06 BY LDH */
                if( pPSessData->CHAPAP.uiID != g_PPP.stPPP.ucID ) {
                    log_print( LOGN_INFO, "NOT MATCHED ID REQ:%u RES:%u", pPSessData->CHAPAP.uiID, g_PPP.stPPP.ucID );
                    break;
                }

                pPSessData->usPPPFlag |= 0x0800;

                pPSessData->CHAPAP.EndTime  	= pstCAP->curtime;
                pPSessData->CHAPAP.EndMTime 	= pstCAP->ucurtime;

                pPSessData->CHAPAP.ucFlag       |= 0x02;
                pPSessData->CHAPAP.uiID         = g_PPP.stPPP.ucID;
                pPSessData->CHAPAP.ucRepMsgCode = g_PPP.stPPP.ucCode;

                if( pPSessData->CHAPAP.ucFlag == 0x03 ) {
                    Report_SIGLog( NUM_CHAP, pPSessData->CHAPAP.ucReqMsgCode, pPSessData );
                    memset( &pPSessData->CHAPAP, 0x00, DEF_TSESS_SIZE );
                }

                pPSessData->ucAuthResultCode    =  g_PPP.stPPP.ucCode;

                pPSessData->AuthEndTime     	= pstCAP->curtime;
                pPSessData->AuthEndMTime    	= pstCAP->ucurtime;
            }
            else if( g_PPP.stPPP.ucCode == DEF_CHAP_FAIL ) {    /* FAILURE */
				/* CHECK ID 2009.09.06 BY LDH */
                if( pPSessData->CHAPAP.uiID != g_PPP.stPPP.ucID ) {
                    log_print( LOGN_INFO, "NOT MATCHED ID REQ:%u RES:%u", pPSessData->CHAPAP.uiID, g_PPP.stPPP.ucID );
                    break;
                }

                pPSessData->CHAPAP.EndTime 		= pstCAP->curtime;
                pPSessData->CHAPAP.EndMTime 	= pstCAP->ucurtime;

                pPSessData->CHAPAP.ucFlag 		|= 0x02;
                pPSessData->CHAPAP.uiID	 		= g_PPP.stPPP.ucID;
                pPSessData->CHAPAP.ucRepMsgCode = g_PPP.stPPP.ucCode;

                if( pPSessData->CHAPAP.ucFlag == 0x03 ) {
                    Report_SIGLog( NUM_CHAP, pPSessData->CHAPAP.ucReqMsgCode, pPSessData );
                    memset( &pPSessData->CHAPAP, 0x00, DEF_TSESS_SIZE );
                }

                pPSessData->ucAuthResultCode =  g_PPP.stPPP.ucCode;

                pPSessData->AuthEndTime = pstCAP->curtime;
                pPSessData->AuthEndMTime = pstCAP->ucurtime;
            }
            break;

        case PPP_PAP:   /* PAP */
            if( g_PPP.stPPP.ucCode == DEF_PAP_AUTH_REQ ) {  /* PAP REQUEST */
                pPSessData->usPPPFlag |= 0x0200;

				if( pPSessData->CHAPAP.StartTime == 0 ) {
					pPSessData->CHAPAP.StartTime = pstCAP->curtime;
                    pPSessData->CHAPAP.StartMTime = pstCAP->ucurtime;

					pPSessData->AuthReqTime = pstCAP->curtime;
                    pPSessData->AuthReqMTime = pstCAP->ucurtime;
				}

                sprintf( pPSessData->szAuthUserName, "%s", g_PPP.stPPP.szUserName );

                pPSessData->CHAPAP.ucFlag 		|= 0x01;
                pPSessData->CHAPAP.uiID 		= g_PPP.stPPP.ucID;
                pPSessData->CHAPAP.ucReqMsgCode = g_PPP.stPPP.ucCode;

                // TO HANDLE WHEN PACKET IS INVERTED
                if( pPSessData->CHAPAP.ucFlag == 0x03 ) {
                    Report_SIGLog( NUM_PAP, pPSessData->CHAPAP.ucReqMsgCode, pPSessData );
                    memset( &pPSessData->CHAPAP, 0x00, DEF_TSESS_SIZE );
                }
            }
			else if( g_PPP.stPPP.ucCode == DEF_PAP_AUTH_ACK ) {   /* PAP SUCCESS ACK */
                pPSessData->usPPPFlag |= 0x0800;

                pPSessData->CHAPAP.EndTime 		= pstCAP->curtime;
                pPSessData->CHAPAP.EndMTime 	= pstCAP->ucurtime;

                pPSessData->CHAPAP.ucFlag 		|= 0x02;
                pPSessData->CHAPAP.uiID 		= g_PPP.stPPP.ucID;
                pPSessData->CHAPAP.ucRepMsgCode = g_PPP.stPPP.ucCode;

                // TO HANDLE WHEN PACKET IS INVERTED
                if( pPSessData->CHAPAP.ucFlag == 0x03 ) {
                    Report_SIGLog( NUM_PAP, pPSessData->CHAPAP.ucReqMsgCode, pPSessData );
                    memset( &pPSessData->CHAPAP, 0x00, DEF_TSESS_SIZE );
                }

                pPSessData->ucAuthResultCode 	= g_PPP.stPPP.ucCode;

                pPSessData->AuthEndTime 		= pstCAP->curtime;
                pPSessData->AuthEndMTime 		= pstCAP->ucurtime;
            }
            else if( g_PPP.stPPP.ucCode == DEF_PAP_AUTH_NAK ) {    /* PAP FAIL ACK */
                pPSessData->CHAPAP.EndTime 		= pstCAP->curtime;
                pPSessData->CHAPAP.EndMTime 	= pstCAP->ucurtime;

                pPSessData->CHAPAP.ucFlag 		|= 0x02;
                pPSessData->CHAPAP.uiID 		= g_PPP.stPPP.ucID;
                pPSessData->CHAPAP.ucRepMsgCode = g_PPP.stPPP.ucCode;

                // TO HANDLE WHEN PACKET IS INVERTED
                if( pPSessData->CHAPAP.ucFlag == 0x03 ) {
                    Report_SIGLog( NUM_PAP, pPSessData->CHAPAP.ucReqMsgCode, pPSessData );
                    memset( &pPSessData->CHAPAP, 0x00, DEF_TSESS_SIZE );
                }

                pPSessData->ucAuthResultCode 	= g_PPP.stPPP.ucCode;

                pPSessData->AuthEndTime 		= pstCAP->curtime;
                pPSessData->AuthEndMTime 		= pstCAP->ucurtime;
            }

			break;

        case PPP_IPCP:  /* IPCP */
            if( g_PPP.stPPP.ucCode == DEF_IPCP_CONF_REQ ) {     /* CONFIG-REQ */

				if( pPSessData->IPCPStartTime == 0 ) {
					pPSessData->IPCPStartTime   	= pstCAP->curtime;
                    pPSessData->IPCPStartMTime  	= pstCAP->ucurtime;
				}

                if( pstCAP->bRtxType == DEF_FROM_CLIENT ) {
                    pPSessData->ucUpIPCPReqCount++;
                    pPSessData->usPPPFlag |= 0x1000;

					if( pPSessData->UpIPCP.StartTime == 0 ) {
                    	pPSessData->UpIPCP.StartTime    = pstCAP->curtime;
                    	pPSessData->UpIPCP.StartMTime   = pstCAP->ucurtime;
                	}

                    pPSessData->UpIPCP.ucFlag       |= 0x01;
                    pPSessData->UpIPCP.uiID         = g_PPP.stPPP.ucID;
                    pPSessData->UpIPCP.ucReqMsgCode = g_PPP.stPPP.ucCode;
                    pPSessData->UpIPCP.ucReqCount++;

                    if( pPSessData->UpIPCP.ucFlag == 0x03 ) {
                        Report_SIGLog( NUM_UP_IPCP, pPSessData->UpIPCP.ucReqMsgCode, pPSessData );
                        memset( &pPSessData->UpIPCP, 0x00, DEF_TSESS_SIZE );
                    }
                }
                else { /* DOWN */
                    pPSessData->ucDownIPCPReqCount++;
					pPSessData->usPPPFlag |= 0x4000;

					if( pPSessData->DownIPCP.StartTime == 0 ) {
                        pPSessData->DownIPCP.StartTime 	= pstCAP->curtime;
                        pPSessData->DownIPCP.StartMTime = pstCAP->ucurtime;
                    }

					pPSessData->DownIPCP.ucFlag         |= 0x01;
                    pPSessData->DownIPCP.uiID           = g_PPP.stPPP.ucID;
                    pPSessData->DownIPCP.ucReqMsgCode   = g_PPP.stPPP.ucCode;
                    pPSessData->DownIPCP.ucReqCount++;

                    if( pPSessData->DownIPCP.ucFlag == 0x03 ) {
                        Report_SIGLog( NUM_DOWN_IPCP, pPSessData->DownIPCP.ucReqMsgCode, pPSessData );
                        memset( &pPSessData->DownIPCP, 0x00, DEF_TSESS_SIZE );
                    }
                }
            }
            else if( g_PPP.stPPP.ucCode == DEF_IPCP_CONF_ACK ) {    /* CONFIG-ACK */
                if( pstCAP->bRtxType == DEF_FROM_CLIENT ) {

					/* CHECK ID 2009.09.06 BY LDH */
                    if( pPSessData->DownIPCP.uiID != g_PPP.stPPP.ucID ) {
                        log_print( LOGN_INFO, "NOT MATCHED ID REQ:%u RES:%u", pPSessData->DownIPCP.uiID, g_PPP.stPPP.ucID );
                        break;
                    }

                    pPSessData->DownIPCP.EndTime 	= pstCAP->curtime;
                    pPSessData->DownIPCP.EndMTime 	= pstCAP->ucurtime;

                    pPSessData->DownIPCP.ucFlag         |= 0x02;
                    pPSessData->DownIPCP.uiID           = g_PPP.stPPP.ucID;
                    pPSessData->DownIPCP.ucRepMsgCode   = g_PPP.stPPP.ucCode;

					pPSessData->usPPPFlag |= 0x8000;

                    if( pPSessData->DownIPCP.ucFlag == 0x03 ) {
                        Report_SIGLog( NUM_DOWN_IPCP, pPSessData->DownIPCP.ucReqMsgCode, pPSessData );
                        memset( &pPSessData->DownIPCP, 0x00, DEF_TSESS_SIZE );
                    }
                }
                else {
					
					/* CHECK ID 2009.09.06 BY LDH */
                    if( pPSessData->UpIPCP.uiID != g_PPP.stPPP.ucID ) {
                        log_print( LOGN_INFO, "NOT MATCHED ID REQ:%u RES:%u", pPSessData->UpIPCP.uiID, g_PPP.stPPP.ucID );
                        break;
                    }

                    pPSessData->uiIPAddr = TOULONG(g_PPP.stPPP.ucIPAddr);

                    pPSessData->UpIPCP.EndTime  = pstCAP->curtime;
                    pPSessData->UpIPCP.EndMTime = pstCAP->ucurtime;

                    pPSessData->UpIPCP.ucFlag       |= 0x02;
                    pPSessData->UpIPCP.uiID         = g_PPP.stPPP.ucID;
                    pPSessData->UpIPCP.ucRepMsgCode = g_PPP.stPPP.ucCode;

					pPSessData->usPPPFlag |= 0x2000;

                    if( pPSessData->UpIPCP.ucFlag == 0x03 ) {
                        Report_SIGLog( NUM_UP_IPCP, pPSessData->UpIPCP.ucReqMsgCode, pPSessData );
                        memset( &pPSessData->UpIPCP, 0x00, DEF_TSESS_SIZE );
                    }
                }

				if( (pPSessData->usPPPFlag & 0xf000) == 0xf000 ) {
                    if( pPSessData->PPPSetupTime == 0 ) {
                        pPSessData->PPPSetupTime    = pstCAP->curtime;
                        pPSessData->PPPSetupMTime   = pstCAP->ucurtime;

                        pPSessData->uiIPCPDuration = GetUpTime( pPSessData->IPCPStartTime, pPSessData->IPCPStartMTime,
                                                            pstCAP->curtime, pstCAP->ucurtime);
                    }
                    else {
                        log_print( LOGN_DEBUG, "IPCP_ACK dup, CreateTime[%d] MIN[%s]",
                                            pPSessData->CreateTime, pPSessData->szMIN );
                    }

                    pPSessData->ucPPPSetupCount ++;
                }
            }
            else if( g_PPP.stPPP.ucCode == DEF_IPCP_CONF_NAK ) {    /* CONFIGURE-NAK */
                if( pstCAP->bRtxType == DEF_FROM_CLIENT ) {
					pPSessData->DownIPCP.ucNakCount++;

					/* CHECK ID 2009.09.06 BY LDH */
                    if( pPSessData->DownIPCP.uiID != g_PPP.stPPP.ucID ) {
                        log_print( LOGN_INFO, "NOT MATCHED ID REQ:%u RES:%u", pPSessData->DownIPCP.uiID, g_PPP.stPPP.ucID );
                        break;
                    }
                    pPSessData->DownIPCP.ucRepMsgCode   = g_PPP.stPPP.ucCode;
                }
                else {
					pPSessData->UpIPCP.ucNakCount++;
			
					/* CHECK ID 2009.09.06 BY LDH */
                    if( pPSessData->UpIPCP.uiID != g_PPP.stPPP.ucID ) {
                        log_print( LOGN_INFO, "NOT MATCHED ID REQ:%u RES:%u", pPSessData->UpIPCP.uiID, g_PPP.stPPP.ucID );
                        break;
                    }
                    pPSessData->UpIPCP.ucRepMsgCode = g_PPP.stPPP.ucCode;
                }
            }
            else if( g_PPP.stPPP.ucCode == DEF_IPCP_CONF_REJ ) {    /* CONFIGURE-REJ*/
                if( pstCAP->bRtxType == DEF_FROM_CLIENT ) {
					pPSessData->DownIPCP.ucRejCount++;

					/* CHECK ID 2009.09.06 BY LDH */
                    if( pPSessData->DownIPCP.uiID != g_PPP.stPPP.ucID ) {
                        log_print( LOGN_INFO, "NOT MATCHED ID REQ:%u RES:%u", pPSessData->DownIPCP.uiID, g_PPP.stPPP.ucID );
                        break;
                    }
                    pPSessData->DownIPCP.ucRepMsgCode   = g_PPP.stPPP.ucCode;
                }
                else {
					pPSessData->UpIPCP.ucRejCount++;
	
					/* CHECK ID 2009.09.06 BY LDH */
                    if( pPSessData->UpIPCP.uiID != g_PPP.stPPP.ucID ) {
                        log_print( LOGN_INFO, "NOT MATCHED ID REQ:%u RES:%u", pPSessData->UpIPCP.uiID, g_PPP.stPPP.ucID );
                        break;
                    }
                    pPSessData->UpIPCP.ucRepMsgCode = g_PPP.stPPP.ucCode;
                }
            }
            else {
                log_print( LOGN_DEBUG, "Unknown IPCP Code[%d]", g_PPP.stPPP.ucCode );
            }

            break;

		case PPP_IP:
        case PPP_VJCTCPIP:
        case PPP_VJUCTCPIP:
		case PPP_CD:
        case PPP_CCP1:
        case PPP_CCP2:
        case PPP_LQR:
            log_print( LOGN_INFO, "Not Interesting PPP[0x%04x]", g_PPP.stPPP.wProtocol );
            break;

        default:
            log_print( LOGN_DEBUG, "Unknown PPP[0x%04x] MIN[%s] CreateT[%u.%d]",
                               g_PPP.stPPP.wProtocol, pPSessData->szMIN, pPSessData->CreateTime, pPSessData->CreateMTime );
            break;
    }

    return 0;
}

int CopyPacket( unsigned char *pDestBuf, short *psiLen, unsigned char *pSrcBuf, short siSize )
{
    if( *psiLen + siSize > MAX_PPP_SIZE )
        return -1;

    memcpy( pDestBuf+*psiLen, pSrcBuf, siSize );
    *psiLen += siSize;

    return 0;
}
