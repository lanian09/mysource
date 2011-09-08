/**
 * Include headers
 */
#include <unistd.h>

// LIB
#include "typedef.h"
#include "loglib.h"
#include "utillib.h"	/* util_cvtipaddr func. */
#include "hasho.h"

// TAF
#include "arp_head.h"
#include "mmdb_psess.h"
#include "mmdb_greentry.h"

// .
#include "arp_func.h"
#include "arp_serv.h"

/**
 * Declare variables
 */
INFO_A11            		g_A11;
INFO_PPP    				g_PPP;
extern T_PSESS_BUF  		SESS_BUF;
extern stHASHOINFO			*pCALLHASH;
extern void 				*hGREEntrySess;

/**
 *	Implement func.
 */
int ProcA11( unsigned char *pAppData, INFO_ETH *pstEth, Capture_Header_Msg *pstCAP )
{
    int     	ret=0, i;
	int			dHeaderLen;
	U8			szIPAddr[32];
	U8			szIPAddr1[32];
	U8			ucCheckRetry = 0;
	//st_GREEntry_Key stKey;
	//st_GREEntry_Data *pstData;
	//st_GREEntry_Key		stGREEntryKey;
	//st_GREEntry_Data	stGREEntryData, *pstGREEntryData;
	GREENTRY_KEY	stKey;
	GREENTRY_DATA	*pstData, stGREEntryData, *pstGREEntryData;
    PSESS_DATA  PSessData, *pPSessData;

    memset( &g_A11, 0x00, sizeof(INFO_A11) );

	dHeaderLen = 14 + pstEth->stIP.wIPHeaderLen + pstEth->stUDPTCP.wHeaderLen;

    if( pstEth->stUDPTCP.wDataLen > 0 ) {
        ret = Analyze_A11( pAppData+dHeaderLen, pstEth->stUDPTCP.wDataLen, &g_A11 );
        if( ret < 0 ) {
            log_print( LOGN_WARN, "ERROR[%d] Analyze_A11()", ret );
            return -1;
        }
		else {
			//Print_INFO_A11( &g_A11 ); 
		}
    }
	else {
		log_print( LOGN_WARN, "%s.%d INVALID SIZE:%d", __FUNCTION__, __LINE__, pstEth->stUDPTCP.wDataLen );
		return -2;
	}

    if( !g_A11.bA11 ) {
		log_print( LOGN_WARN, "Not A11 Src[%u,%u] Dest[%u,%u] MSG[0X%02X]",
				pstEth->stIP.dwSrcIP, pstEth->stUDPTCP.wSrcPort, pstEth->stIP.dwDestIP, pstEth->stUDPTCP.wDestPort, g_A11.ucMsg ); 

        return 1;
    }

    memset( &PSessData, 0x00, sizeof(PSESS_DATA) );

    /* CHECK DIRECTION */
	if( pstCAP->bRtxType == DEF_FROM_CLIENT )
		PSessData.key.uiServingPCF = htonl(pstEth->stIP.dwSrcIP);
	else if( pstCAP->bRtxType == DEF_FROM_SERVER )
		PSessData.key.uiServingPCF = htonl(pstEth->stIP.dwDestIP);
	else {
		log_print( LOGN_CRI, "INVALID DIRECTION :%d", pstCAP->bRtxType );
		return 1;
	}

/*
    if( g_A11.ucMsg == DEF_A11_REG_REQ || g_A11.ucMsg == DEF_A11_REG_ACK )
		PSessData.key.uiServingPCF = htonl(pstEth->stIP.dwSrcIP);
    else if( g_A11.ucMsg == DEF_A11_REG_REP || g_A11.ucMsg == DEF_A11_REG_UP )
		PSessData.key.uiServingPCF = htonl(pstEth->stIP.dwDestIP);
    else {
        log_print( LOGN_DEBUG, "Unknown A11 Msg[%d]", g_A11.ucMsg );
        return 1;
    }
*/

    PSessData.key.uiKey = g_A11.dwKey;

	/* REGISTRATION REQUEST, LIFETIME > 0, CONNECTION SETUP */
log_print( LOGN_DEBUG, "A11_MSG, I[%15s] PCF[%15s] PD[%15s] K[%10u] MSG[%2u] LFT[%4u] AK[0x%02x]",
g_A11.szMDN, util_cvtipaddr(szIPAddr, ntohl(PSessData.key.uiServingPCF)), 
pstCAP->bRtxType == DEF_FROM_CLIENT ? util_cvtipaddr(szIPAddr1, pstEth->stIP.dwDestIP) : util_cvtipaddr(szIPAddr1, pstEth->stIP.dwSrcIP),
PSessData.key.uiKey, g_A11.ucMsg, g_A11.usLifetime, g_A11.ucAirlinkType );

	/* CHG FOR TEST LDH */
	//if( g_A11.ucMsg == DEF_A11_REG_REQ && g_A11.usLifetime > 0 && g_A11.ucAirlinkType & CONN_SETUP ) {   
	if( g_A11.ucMsg == DEF_A11_REG_REQ && g_A11.usLifetime > 0 && g_A11.ucAirlinkType & ACTIVE_START ) {
		/* NEW A10 SESSION */
        ret = CreateA10( &PSessData, pstEth, pstCAP, &ucCheckRetry );
        if( ret < 0 )
            return -3;
    }

#if 0
    log_print(LOGN_INFO, "KEY ENTRY CNT = [%u]", g_A11.ucKeyEntryCnt);
    for( i = 0; i < g_A11.ucKeyEntryCnt; i++ ) {
        stGREEntryKey.uiPCFIP = pPSessData->key.uiServingPCF;
        stGREEntryKey.uiGREKey = g_A11.dwKeyEntry[i];
        stGREEntryData.uiMainGREKey = pPSessData->key.uiKey;
        pstGREEntryData = Search_Data(hGREEntrySess, &stGREEntryKey);
        if( pstGREEntryData == NULL ) {
            ret = Insert_Data(hGREEntrySess, &stGREEntryKey, &stGREEntryData, time(0));
            if( ret < 0 ) {
                log_print( LOGN_WARN, "[FAIL:%d] Insert_Data(hGREEntrySess)", ret );
            }
			else {
				log_print( LOGN_INFO, "GRE KEY ENTRY INSERTED. PCFIP[%u] KEY[%u] MAINKEY[%u]",
						stGREEntryKey.uiPCFIP, stGREEntryKey.uiGREKey, stGREEntryData.uiMainGREKey);
			}
        }
        else {
            log_print( LOGN_INFO, "GRE KEY ENTRY ALREADY EXIST. PCFIP[%u] KEY[%u]",
                    stGREEntryKey.uiPCFIP, stGREEntryKey.uiGREKey);
        }
    }
	Print_GREEntry();
#endif
    pPSessData = Search_PSESS( &PSessData.key );
	if( pPSessData == NULL ) {
#if 0
		log_print( LOGN_DEBUG, "PSESS NOT EXISTS, I[%15s] PCF[%15s] K[%10u] MSG[%2u] LFT[%4u] AK[0x%02x]",
				g_A11.szMDN, util_cvtipaddr(szIPAddr, PSessData.key.uiServingPCF), PSessData.key.uiKey, 
				g_A11.ucMsg, g_A11.usLifetime, g_A11.ucAirlinkType );
		return -4;
#endif
#if 0
		stKey.uiPCFIP = PSessData.key.uiServingPCF;
		stKey.uiGREKey = PSessData.key.uiKey;
		pstData = (st_GREEntry_Data *)Search_Data(hGREEntrySess, &stKey);
		if( pstData == NULL ) {
			log_print( LOGN_DEBUG, "PSESS NOT EXISTS, I[%15s] PCF[%15s] K[%10u] MSG[%2u] LFT[%4u] AK[0x%02x]",
					g_A11.szMDN, util_cvtipaddr(szIPAddr, PSessData.key.uiServingPCF), PSessData.key.uiKey, 
					g_A11.ucMsg, g_A11.usLifetime, g_A11.ucAirlinkType );
			return -4;
		}
		PSessData.key.uiKey = pstData->uiMainGREKey;
		pPSessData = Search_PSESS( &PSessData.key ); 
		if( pPSessData == NULL ) {
			log_print( LOGN_DEBUG, "PSESS NOT EXISTS, I[%15s] PCF[%15s] K[%10u] MSG[%2u] LFT[%4u] AK[0x%02x]",
					g_A11.szMDN, util_cvtipaddr(szIPAddr, PSessData.key.uiServingPCF), PSessData.key.uiKey, 
					g_A11.ucMsg, g_A11.usLifetime, g_A11.ucAirlinkType );
			return -4;
		}
#endif
		stKey.uiPCFIP = PSessData.key.uiServingPCF;
		stKey.uiGREKey = PSessData.key.uiKey;
		pstData = Search_GREENTRY(&stKey);
		if( pstData == NULL ) {
			log_print( LOGN_DEBUG, "    PSESS NOT EXISTS, I[%15s] PCF[%15s] K[%10u] MSG[%2u] LFT[%4u] AK[0x%02x]",
					g_A11.szMDN, util_cvtipaddr(szIPAddr, PSessData.key.uiServingPCF), PSessData.key.uiKey, 
					g_A11.ucMsg, g_A11.usLifetime, g_A11.ucAirlinkType );
			return -4;
		}
		PSessData.key.uiKey = pstData->uiMainGREKey;
		pPSessData = Search_PSESS( &PSessData.key ); 
		if( pPSessData == NULL ) {
			log_print( LOGN_DEBUG, "    PSESS NOT EXISTS, I[%15s] PCF[%15s] K[%10u] MSG[%2u] LFT[%4u] AK[0x%02x]",
					g_A11.szMDN, util_cvtipaddr(szIPAddr, PSessData.key.uiServingPCF), PSessData.key.uiKey, 
					g_A11.ucMsg, g_A11.usLifetime, g_A11.ucAirlinkType );
			return -4;
		}
		else {
			log_print( LOGN_DEBUG, "    PROCA11:[REG:%2d] LFT[%4d] I[%15s] CT[%10u.%06d] PCF[%10u] PD[%10u] K[%10u] AK[0x%02x] CD[0X%02X] UP[%u] STOP[%u] CALL[%u]",
					g_A11.ucMsg, g_A11.usLifetime, g_A11.szMDN, pPSessData->CreateTime, pPSessData->CreateMTime,
					PSessData.key.uiServingPCF, PSessData.uiNASIP, PSessData.key.uiKey, g_A11.ucAirlinkType, g_A11.ucCode, g_A11.dwUpdateReason,
				    PSessData.uiStopFlag, PSessData.uiCallType);
			/*
			   if( g_A11.ucCode == 1 || g_A11.ucCode == 3 || g_A11.ucCode == 201 )
			   log_beacon( pAppData, pstCAP->datalen, 1, 1, 1 ); 
			 */
			if( g_A11.szSublink[0] == 0x01 && g_A11.szSublink[19] == 0x02 && pPSessData->uiSvcOption == DEF_1X_NUM )
				bcon_write( pAppData, pstCAP->datalen, 1, 1, 1 );
		}
	}
	else {
		log_print( LOGN_DEBUG, "    PROCA11:[REG:%2d] LFT[%4d] I[%15s] CT[%10u.%06d] PCF[%10u] PD[%10u] K[%10u] AK[0x%02x] CD[0X%02X] UP[%u] STOP[%u] CALL[%u]",
                            g_A11.ucMsg, g_A11.usLifetime, g_A11.szMDN, pPSessData->CreateTime, pPSessData->CreateMTime,
                            PSessData.key.uiServingPCF, PSessData.uiNASIP, PSessData.key.uiKey, g_A11.ucAirlinkType, g_A11.ucCode, g_A11.dwUpdateReason, 
			   				PSessData.uiStopFlag, PSessData.uiCallType);
/*
		if( g_A11.ucCode == 1 || g_A11.ucCode == 3 || g_A11.ucCode == 201 )
			log_beacon( pAppData, pstCAP->datalen, 1, 1, 1 ); 
*/
		if( g_A11.szSublink[0] == 0x01 && g_A11.szSublink[19] == 0x02 && pPSessData->uiSvcOption == DEF_1X_NUM )
			bcon_write( pAppData, pstCAP->datalen, 1, 1, 1 );
	}

    for( i = 0; i < g_A11.ucKeyEntryCnt; i++ ) {
        stGREEntryData.key.uiPCFIP = pPSessData->key.uiServingPCF;
        stGREEntryData.key.uiGREKey = g_A11.dwKeyEntry[i];
        stGREEntryData.uiMainGREKey = pPSessData->key.uiKey;
		pstGREEntryData = Search_GREENTRY(&stGREEntryData.key);
		if( pstGREEntryData == NULL ) {
			ret = Insert_GREENTRY(&stGREEntryData);
            if( ret < 0 ) {
                log_print( LOGN_WARN, "[FAIL:%d] Insert_Data(hGREEntrySess)", ret );
            }
			else {
				log_print( LOGN_INFO, "GRE KEY ENTRY INSERTED. PCFIP[%u] KEY[%u] MAINKEY[%u]",
						stGREEntryData.key.uiPCFIP, stGREEntryData.key.uiGREKey, stGREEntryData.uiMainGREKey);
			}
		}
        else {
            log_print( LOGN_INFO, "GRE KEY ENTRY ALREADY EXIST. PCFIP[%u] KEY[%u]",
                    stGREEntryData.key.uiPCFIP, stGREEntryData.key.uiGREKey);
        }
	}
	//Print_GREEntry();

	/* CHECK TRACE INFO */
	ret = dCheck_TraceInfo( pPSessData, pAppData, pstCAP );
	if( ret < 0 )
		log_print( LOGN_INFO, "[%s.%d] ERROR IN dCheck_TraceInfo dRet:%d", __FUNCTION__, __LINE__, ret );

    pPSessData->LastUpdateTime  = pstCAP->curtime;
    pPSessData->LastUpdateMTime = pstCAP->ucurtime;

    if( pPSessData->uiSeq == 0xffffffff )
        pPSessData->uiSeq = 0;
    else
        pPSessData->uiSeq ++;

	switch( g_A11.ucMsg )
    {
        case DEF_A11_REG_REQ:   /* REGI REQUEST */
            pPSessData->ucRegiReqCount++;

            pPSessData->RegA11.usLifetime = g_A11.usLifetime;
            if( g_A11.usLifetime > 0 ) {
                if( g_A11.ucAirlinkType & CONN_SETUP)
                    pPSessData->RegA11.usA11Flag |= 0x0001;
                else {
                    pPSessData->RegA11.usA11Flag &= 0xf0ff;    /* 2번째 초기화를 위한 */
                    pPSessData->RegA11.usA11Flag |= 0x0100;
                }
            }
            else {
                pPSessData->RegA11.usA11Flag |= 0x0010;
                pPSessData->RPTTime = pstCAP->curtime;
                pPSessData->RPTMTime = pstCAP->ucurtime;
            }

            if( g_A11.ucAirlinkType & ACTIVE_START ) {
                if( pPSessData->RegA11.ucAirlink & ACTIVE_STOP ) {
					/* 기존에 ACTIVE STOP인데, ACTIVE START가 전송 -> IDLE 정보 UPDATE */
                    pPSessData->uiIdleDuration += GetUpTime( pPSessData->RPTTime, pPSessData->RPTMTime,
                                                             pstCAP->curtime, pstCAP->ucurtime );

                    pPSessData->RPTTime     = pstCAP->curtime;
                    pPSessData->RPTMTime    = pstCAP->ucurtime;
                }

                pPSessData->ucAStartCount ++;
            }

            if( g_A11.ucAirlinkType & ACTIVE_STOP ) {
                if( pPSessData->RegA11.ucAirlink & ACTIVE_START ) {
					/* 기존에 ACTIVE START인데, ACTIVE STOP이 전송 -> ACTIVE 정보 UPDATE */
                    pPSessData->uiActiveDuration += GetUpTime( pPSessData->RPTTime, pPSessData->RPTMTime,
                                                               pstCAP->curtime, pstCAP->ucurtime );
                    pPSessData->RPTTime     = pstCAP->curtime;
                    pPSessData->RPTMTime    = pstCAP->ucurtime;
                }
                pPSessData->ucAStopCount ++;
            }

            pPSessData->RegA11.ucAirlink = g_A11.ucAirlinkType;
            pPSessData->ucAppType 		 = g_A11.ucApplicationType;

			if( g_A11.szSublink[0] == 0x01 && g_A11.szSublink[19] == 0x02 ) {
        		for(i=0; i<6; i++)
            		sprintf( &pPSessData->ucBSMSC[i*2], "%02X", g_A11.szSublink[i+31] );

        		memcpy( &pPSessData->stBSMSC, &g_A11.szSublink[31], 6 );
    		}

    		pPSessData->ucBSMSC[DEF_BSMSD_LENGTH-1] = 0x00;

            if( g_A11.dwServiceOption > 0 ) {
                if( pPSessData->uiSvcOption > 0 && pPSessData->uiSvcOption != g_A11.dwServiceOption ) {
					/* SVCOPTION 변경에 대한 정보 UPDATE */
                    log_print( LOGN_INFO, "SvcOption changed, [%u-%u-%u]=>[%u-%u-%u]",
                                       pPSessData->uiSvcOption, pPSessData->uiFMux, pPSessData->uiRMux,
                                       g_A11.dwServiceOption, g_A11.dwForwardMux, g_A11.dwReverseMux );

                    pPSessData->ucSvcOptChange ++;
                }
                pPSessData->uiSvcOption     = g_A11.dwServiceOption;

				switch( pPSessData->uiSvcOption ) {
                	case DEF_1X_NUM:
                    	sprintf( pPSessData->szNetOption, "%s", "1X" );
                       	break;

                    case DEF_EVDO_NUM:
                    	sprintf( pPSessData->szNetOption, "%s", "EV-DO" );
                        break;

                    case DEF_IS95A_NUM:
                        sprintf( pPSessData->szNetOption, "%s", "IS-95A" );
                        break;

                    case DEF_IS95B_NUM:
                        sprintf( pPSessData->szNetOption, "%s", "IS-95B" );
                        break;

                    default:
                        sprintf( pPSessData->szNetOption, "%s", "ETC" );
                        break;
                }

                pPSessData->uiFMux          = g_A11.dwForwardMux;
                pPSessData->uiRMux          = g_A11.dwReverseMux;
            }

			/* BSMSC 정보 변경에 대한 UPDATE *
			if( g_A11.szBsMscId[0] != 0x00 ) {
                if( memcmp(pPSessData->ucBSMSC, g_A11.szBsMscId, DEF_BSMSD_LENGTH ) != 0 )
                    pPSessData->ucBSMSCChgCount ++;
                memcpy( pPSessData->ucBSMSC, g_A11.szBsMscId, DEF_BSMSD_LENGTH );
            }
			*/

            pPSessData->RegA11.ucFlag          |= DEF_A11_REQUEST;
            pPSessData->RegA11.StartTime       = pstCAP->curtime;
            pPSessData->RegA11.StartMTime      = pstCAP->ucurtime;
			pPSessData->RegA11.uiSrcIP         = htonl(pstEth->stIP.dwSrcIP);
            pPSessData->RegA11.uiDestIP        = htonl(pstEth->stIP.dwDestIP);
            pPSessData->RegA11.ucReqMsgCode    = g_A11.ucMsg;
            pPSessData->RegA11.ucReqCount++;

            if( pPSessData->RegA11.ucFlag == DEF_A11_PAIR ) {
                Report_SIGLog( NUM_UDP_A11, pPSessData->RegA11.ucReqMsgCode, pPSessData );
                memset(&pPSessData->RegA11, 0x00, DEF_A11INFO_SIZE );
            }

			/* LIFETIME > 0, CONNECTION SETUP */
			/* CHG FOR TEST LDH */
            /*if( g_A11.usLifetime > 0 && g_A11.ucAirlinkType & CONN_SETUP ) {  */

			if( g_A11.usLifetime > 0 && g_A11.ucAirlinkType & ACTIVE_START && ucCheckRetry != 1 ) {

#if 1 /* INYOUNG */
				/*
				 * DORMANT 상태가 되었는지를 확인하여 발신과 착신을 구분
				 */
				if(pPSessData->uiCallType >= DEF_CALL_RECALL) {
                	log_print( LOGN_DEBUG, "    PROCA11:RP_RE_CREATE I[%15s] CT[%u.%06d] PCF[%10u] PD[%10u] K[%10u] LFT[%4u] AK[0x%02x] STOP[%u] CALL[%u]",
                                   pPSessData->szMIN, pPSessData->CreateTime, pPSessData->CreateMTime,
                                   pPSessData->key.uiServingPCF, pPSessData->uiNASIP, pPSessData->key.uiKey,
								   pPSessData->RegA11.usLifetime, g_A11.ucAirlinkType, pPSessData->uiStopFlag, pPSessData->uiCallType );

					/* SEND START MESSAGE */
					Report_SIGLog( START_RP_SIG_RECALL_NUM, pPSessData->RegA11.ucReqMsgCode, pPSessData );
				} else {
#endif
            		/* NEW A10 SESSION */
                	log_print( LOGN_DEBUG, "    PROCA11:RP_CREATE I[%15s] CT[%u.%06d] PCF[%10u] PD[%10u] K[%10u] LFT[%4u] AK[0x%02x] STOP[%u] CALL[%u]",
                                   pPSessData->szMIN, pPSessData->CreateTime, pPSessData->CreateMTime,
                                   pPSessData->key.uiServingPCF, pPSessData->uiNASIP, pPSessData->key.uiKey,
								   pPSessData->RegA11.usLifetime, g_A11.ucAirlinkType, pPSessData->uiStopFlag, pPSessData->uiCallType );

					/* SEND START MESSAGE */
					Report_SIGLog( START_CALL_NUM, pPSessData->RegA11.ucReqMsgCode, pPSessData );
#if 1 /* INYOUNG */
				}
#endif
            }

            break;

        case DEF_A11_REG_REP: /* REGI REPLY */
            pPSessData->RegA11.usLifetime = g_A11.usLifetime;

            if( g_A11.ucCode == DEF_A11_REG_ACCT ) {
                pPSessData->ucRegiSuccCount++;
				pPSessData->RegA11.ucRegiReply = g_A11.ucCode;

                if( g_A11.usLifetime > 0 && (pPSessData->RegA11.usA11Flag & 0xfff3) == 0x0001 ) {
                    pPSessData->uiRPUpTime = GetUpTime( pPSessData->CreateTime, pPSessData->CreateMTime,
                                                        pstCAP->curtime, pstCAP->ucurtime );
                    pPSessData->RegA11.usA11Flag |= 0x0002;
                }
                else if( g_A11.usLifetime == 0 )
                    pPSessData->RegA11.usA11Flag |= 0x0020;
                else
                    pPSessData->RegA11.usA11Flag |= 0x0200;
            }
            else {
				pPSessData->RegA11.ucRegiReply = g_A11.ucCode;

                if( g_A11.usLifetime > 0 && (pPSessData->RegA11.usA11Flag & 0xfff3) == 0x0001 )
                    pPSessData->RegA11.usA11Flag |= 0x0004;
                else if( g_A11.usLifetime == 0 )    // ???
                    pPSessData->RegA11.usA11Flag |= 0x0040;
                else if( (pPSessData->RegA11.usA11Flag & 0x0700) == 0x0100 )
                    pPSessData->RegA11.usA11Flag |= 0x0400;
                else
                    log_print( LOGN_DEBUG, "Unknown A11Flag[0x%04x] MIN[%s] CreateT[%u.%d] RegiCnt[%d] ucFlag[%d]",
                                        pPSessData->RegA11.usA11Flag, pPSessData->szMIN, pPSessData->CreateTime, pPSessData->CreateMTime,
                                        pPSessData->ucRegiReqCount, pPSessData->RegA11.ucFlag );
            }

            pPSessData->RegA11.ucFlag          |= DEF_A11_RESPONSE;
            pPSessData->RegA11.EndTime         = pstCAP->curtime;
            pPSessData->RegA11.EndMTime        = pstCAP->ucurtime;
            pPSessData->RegA11.ucRepMsgCode    = g_A11.ucMsg;

			if( g_A11.AlwaysOn != 0 )
				pPSessData->ucAlwaysOn		= g_A11.AlwaysOn;

            if( g_A11.ucCode != DEF_A11_REG_ACCT )
                pPSessData->RegA11.ucRejCount++;

            if( pPSessData->RegA11.ucFlag == DEF_A11_PAIR )
                Report_SIGLog( NUM_UDP_A11, pPSessData->RegA11.ucReqMsgCode, pPSessData );

            //if( g_A11.usLifetime > 0 && g_A11.ucCode > DEF_A11_REG_ACCT ) {
			if( g_A11.usLifetime > 0 ) {
				/* CONTINUE SESSION */
				if( pPSessData->RegA11.ucAirlink == ACTIVE_STOP ) {
					/* ONLY STOP */
					pPSessData->uiStopFlag = 1;
				}

				log_print( LOGN_DEBUG, "    PROCA11:RP_CONT I[%15s] CT[%10u.%06d] PCF[%10u] K[%10u] LFT[%4u] AK[0x%02x] STOP[%u] CALL[%u]",
									pPSessData->szMIN, pPSessData->CreateTime, pPSessData->CreateMTime,
									pPSessData->key.uiServingPCF, pPSessData->key.uiKey, pPSessData->RegA11.usLifetime, pPSessData->RegA11.ucAirlink, 
									pPSessData->uiStopFlag, pPSessData->uiCallType);
            }
            else if( (g_A11.usLifetime == 0) && (pPSessData->RegA11.ucAirlink & ACTIVE_STOP) ) {
				/* released */
                ProcPPPSess( pPSessData );
				
				/* 1. 착신호 상태와 일반호 상태를 구분 */
				if( pPSessData->uiCallType >= DEF_CALL_RECALL) {
                	log_print( LOGN_DEBUG, 
						"    PROCA11:RP_RE_STOP I[%s] CT[%10u.%06d] PCF[%10u] PD[%10u] K[%10u] LIFETIME[%4u] AIRLINK[0x%02x] STOP[%u] CALL[%u]",
                        		pPSessData->szMIN, pPSessData->CreateTime, pPSessData->CreateMTime,
                        		pPSessData->key.uiServingPCF, pPSessData->uiNASIP, pPSessData->key.uiKey, 
								pPSessData->RegA11.usLifetime, pPSessData->RegA11.ucAirlink, pPSessData->uiStopFlag, pPSessData->uiCallType);

					Report_SIGLog( STOP_RP_RECALL_NUM, pPSessData->RegA11.ucReqMsgCode, pPSessData );

				} else {
                	log_print( LOGN_DEBUG, 
						"    PROCA11:RP_STOP I[%s] CT[%10u.%06d] PCF[%10u] PD[%10u] K[%10u] LIFETIME[%4u] AIRLINK[0x%02x] STOP[%u] CALL[%u]",
                        		pPSessData->szMIN, pPSessData->CreateTime, pPSessData->CreateMTime,
                        		pPSessData->key.uiServingPCF, pPSessData->uiNASIP, pPSessData->key.uiKey, 
								pPSessData->RegA11.usLifetime, pPSessData->RegA11.ucAirlink, pPSessData->uiStopFlag, pPSessData->uiCallType);

					Report_SIGLog( STOP_CALL_NUM, pPSessData->RegA11.ucReqMsgCode, pPSessData );
				}

				/* DELETE SESSION */
                ret = RemoveA10( pPSessData );
            }

			if( pPSessData->RegA11.ucFlag == DEF_A11_PAIR )
				memset(&pPSessData->RegA11, 0x00, DEF_A11INFO_SIZE );

            break;

		case DEF_A11_REG_UP:    /* REGI UPDATE */
            pPSessData->ucUpdateReqCount++;
            pPSessData->UpA11.uiUpdateReason = g_A11.dwUpdateReason;
            pPSessData->UpA11.usA11Flag |= 0x1000;

            // Signal           
            pPSessData->UpA11.ucFlag          |= DEF_A11_REQUEST;
            pPSessData->UpA11.StartTime       = pstCAP->curtime;
            pPSessData->UpA11.StartMTime      = pstCAP->ucurtime;
            pPSessData->UpA11.uiSrcIP         = htonl(pstEth->stIP.dwSrcIP);
            pPSessData->UpA11.uiDestIP        = htonl(pstEth->stIP.dwDestIP);
            pPSessData->UpA11.ucReqMsgCode    = g_A11.ucMsg;
            pPSessData->UpA11.ucReqCount++;

            if( pPSessData->UpA11.ucFlag == DEF_A11_PAIR ) {
                Report_SIGLog( NUM_UDP_A11, pPSessData->UpA11.ucReqMsgCode, pPSessData );
                memset(&pPSessData->UpA11, 0x00, DEF_A11INFO_SIZE );
            }

            break;

        case DEF_A11_REG_ACK:   /* regi ack */
            if( g_A11.ucCode == DEF_A11_REG_ACCT ) {
                pPSessData->ucUpdateAckCount++;
                pPSessData->UpA11.usA11Flag |= 0x2000;

				pPSessData->UpA11.ucRegiReply = g_A11.ucCode;
            }
            else {
                pPSessData->UpA11.usA11Flag |= 0xc000;
				pPSessData->UpA11.ucRegiReply = g_A11.ucCode;
            }

            pPSessData->UpA11.ucFlag          |= DEF_A11_RESPONSE;
            pPSessData->UpA11.EndTime         = pstCAP->curtime;
            pPSessData->UpA11.EndMTime        = pstCAP->ucurtime;
            pPSessData->UpA11.ucRepMsgCode    = g_A11.ucMsg;

            if( g_A11.ucCode != DEF_A11_REG_ACCT )
                pPSessData->UpA11.ucRejCount++;

            if( pPSessData->UpA11.ucFlag == DEF_A11_PAIR ) {
                Report_SIGLog( NUM_UDP_A11, pPSessData->UpA11.ucReqMsgCode, pPSessData );
                memset(&pPSessData->UpA11, 0x00, sizeof(T_SESS) );
            }

            break;

		case DEF_A11_SESS_UP:
			pPSessData->SessA11.ucFlag          |= DEF_A11_REQUEST;
			pPSessData->SessA11.StartTime       = pstCAP->curtime;
            pPSessData->SessA11.StartMTime      = pstCAP->ucurtime;
            pPSessData->SessA11.uiSrcIP         = htonl(pstEth->stIP.dwSrcIP);
            pPSessData->SessA11.uiDestIP        = htonl(pstEth->stIP.dwDestIP);
            pPSessData->SessA11.ucReqMsgCode    = g_A11.ucMsg;
            pPSessData->SessA11.ucReqCount++;

			if( pPSessData->SessA11.ucFlag == DEF_A11_PAIR ) {
                Report_SIGLog( NUM_UDP_A11, pPSessData->SessA11.ucReqMsgCode, pPSessData );
                memset(&pPSessData->SessA11, 0x00, DEF_A11INFO_SIZE );
            }

			break;

		case DEF_A11_SESS_ACK:
			pPSessData->SessA11.ucRegiReply 		= g_A11.ucCode;

            pPSessData->SessA11.ucFlag          |= DEF_A11_RESPONSE;
            pPSessData->SessA11.EndTime         = pstCAP->curtime;
            pPSessData->SessA11.EndMTime        = pstCAP->ucurtime;
            pPSessData->SessA11.ucRepMsgCode    = g_A11.ucMsg;

            if( g_A11.ucCode != DEF_A11_REG_ACCT )
                pPSessData->SessA11.ucRejCount++;

            if( pPSessData->SessA11.ucFlag == DEF_A11_PAIR ) {
                Report_SIGLog( NUM_UDP_A11, pPSessData->SessA11.ucReqMsgCode, pPSessData );
                memset(&pPSessData->SessA11, 0x00, DEF_A11INFO_SIZE );
            }

			break;

        default:
            log_print( LOGN_WARN, "Unknown A11 Msg[0X%02x]", g_A11.ucMsg );
            break;
    }

    return 0;
}


/*******************************************************************************

*******************************************************************************/
void InitPSESS( PSESS_DATA *pSESS )
{
	memset( &pSESS->RegA11, 0x00, DEF_A11INFO_SIZE );
	memset( &pSESS->UpA11, 0x00, DEF_A11INFO_SIZE );
	memset( &pSESS->SessA11, 0x00, DEF_A11INFO_SIZE );

	memset( &pSESS->UpLCP, 0x00, DEF_TSESS_SIZE );
	memset( &pSESS->DownLCP, 0x00, DEF_TSESS_SIZE );
	memset( &pSESS->UpLCPEcho, 0x00, DEF_TSESS_SIZE );
    memset( &pSESS->DownLCPEcho, 0x00, DEF_TSESS_SIZE );
	memset( &pSESS->CHAPAP, 0x00, DEF_TSESS_SIZE );
	memset( &pSESS->UpIPCP, 0x00, DEF_TSESS_SIZE );
	memset( &pSESS->DownIPCP, 0x00, DEF_TSESS_SIZE );
	memset( &pSESS->LCPTerm, 0x00, DEF_TSESS_SIZE );

}

/*******************************************************************************
	RETURN : 1  -> REG REQ RETRY
			 0  -> CREATE NEW A10 SESSION
			-1  -> PPP BUFFER ERROR
			-2  -> FAIL INSERT SESSION
*******************************************************************************/
int CreateA10( PSESS_DATA *pPSessData, INFO_ETH *pstEth, Capture_Header_Msg *pstCAP, U8 *pucRetry )
{
	int         		i, ret;
	PSESS_DATA  		*pPSessData1;
//	st_GREEntry_Key		stGREEntryKey;
//	st_GREEntry_Data	stGREEntryData, *pstGREEntryData;
	st_CallHashKey		stCallKey, *pstCallKey;
	st_CallHashData		stCallData, *pstCallData;
	stHASHONODE			*pHASHNODE;

	/* DELETE IF EXISTS */
	pPSessData1 = Search_PSESS( &pPSessData->key );
	if( pPSessData1 != NULL ) {
		log_print( LOGN_DEBUG, "    PSESS ALREADY EXISTS, SrvPCF[%u] Key[%u] CreateTime[%ld] IMSI[%s] A11Flag[0x%04x] STOP[%u] CALL[%u]",
				pPSessData1->key.uiServingPCF, pPSessData1->key.uiKey, pPSessData1->CreateTime, pPSessData1->szMIN,
				pPSessData1->RegA11.usA11Flag, pPSessData1->uiStopFlag, pPSessData1->uiCallType );

		/*
		 * usA11Flag
		 * 0x0001 -> DEF_A11_REG_REQ, usLifetime > 0, ucAirlinkType & CONN_SETUP.
		 * 0x0010 -> DEF_A11_REG_REQ, usLifetime = 0.
		 * 0x0100 -> DEF_A11_REG_REQ, usLifetime > 0, ucAirlinkType X CONN_SETUP.
		 * 0x0002 -> DEF_A11_REG_REP, g_A11.ucCode == 0x00(REGI ACCEPT), usLifetime > 0, usA11Flag & 0xfff3 == 0x0001.
		 * 0x0020 -> DEF_A11_REG_REP, g_A11.ucCode == 0x00(REGI ACCEPT), usLifetime = 0.
		 * 0x0200 -> DEF_A11_REG_REP, g_A11.ucCode == 0x00(REGI ACCEPT), usLifetime > 0, usA11Flag & 0xfff3 != 0x0001. 
		 * 0x0003 -> A10이 정상적으로 이루어졌는지 여부에 대한 확인
		 * 0x0004 -> DEF_A11_REG_REP, g_A11.ucCode != DEF_A11_REG_ACCT, usLifetime > 0, usA11Flag & 0xfff3 == 0x0001.
		 * 0x0040 -> DEF_A11_REG_REP, g_A11.ucCode != DEF_A11_REG_ACCT, usLifetime = 0.
		 * 0x0400 -> DEF_A11_REG_REP, g_A11.ucCode != DEF_A11_REG_ACCT, usLifetime > 0, usA11Flag & 0x0700 == 0x0100.
		 * 0x0008 -> RETRY
		 * 0x1000 -> DEF_A11_REG_UP
		 * 0x2000 -> DEF_A11_REG_ACK, g_A11.ucCode == DEF_A11_REG_ACCT
		 * 0x4000 -> DEF_A11_SESS_UP
		 * 0x8000 -> DEF_A11_SESS_ACK 
		 * 0xc000 -> DEF_A11_REG_ACK, g_A11.ucCode != DEF_A11_REG_ACCT 
		 * 
		 */

		/*
		 * 1. IF ( DORMANT 상태로 TIME WAIT 시간 안에 시그널이 들어왔음 )
		 * 	  -> 이전호에 연결하고 시작 시그널을 보내지 않는다.
		 * 2. IF ( TIME WAIT 이후 착신대기 상태에 시그널이 들어왔음 )
  		 * 	  -> 새로운 호로 업데이트하고 시작 시그널을 보낸다. 
		 */

		/* DO NOT SEND START MESSAGE */
		if(pPSessData1->uiStopFlag == 1) {
			*pucRetry = 1;
		}

		if( (pPSessData1->RegA11.usA11Flag & 0x0003) == 0x0001 && strcmp(pPSessData1->szMIN, g_A11.szMDN) == 0 ) {
			/* REGISTRATION NOT ACCEPTED & RETRY */
			log_print( LOGN_CRI, "PSESS regi retry MIN[%s] A11Flag[0x%04x] PPPFlag[0x%04x]",
					pPSessData1->szMIN, pPSessData1->RegA11.usA11Flag, pPSessData1->usPPPFlag );

			pPSessData1->RegA11.usA11Flag |= 0x0008;
			return 1;
		}

		/* LAST REPORT *
		   ProcPPPSess( pPSessData1 );
		//Report_RP( MID_RP_DROP, pPSessData1 );

		 * DELETE PREVIOUS SESSION *
		 ret = RemoveA10( pPSessData1 );
		 */

		if( pPSessData1->uiStopFlag == 2) {

			pPSessData1->CreateTime = pstCAP->curtime;
			pPSessData1->CreateMTime = pstCAP->ucurtime;

			InitPSESS( pPSessData1 );
		}

		/* 시그널을 받았음을 표시 */
		if( pPSessData1->uiCallType == DEF_CALL_RECALL)
			pPSessData1->uiCallType = DEF_CALL_RECALL_1;

		pPSessData = pPSessData1;
	}
	else {
		pstCallKey 	= &stCallKey;
		pstCallData = &stCallData;

		/* CHECK IMSI CALL SESSION */
		stCallKey.llIMSI = atoll(g_A11.szMDN);

		if( (pHASHNODE = hashs_find( pCALLHASH, (U8 *)pstCallKey )) == NULL ) {

			stCallData.uiPCFIP 				= pPSessData->key.uiServingPCF;
			stCallData.stCreateTime.tv_sec 	= pstCAP->curtime;
			stCallData.stCreateTime.tv_usec = pstCAP->ucurtime;

			if( (pHASHNODE = hashs_add( pCALLHASH, (U8 *)pstCallKey, (U8 *)pstCallData )) == NULL )
				log_print( LOGN_CRI, "[%s.%d] ERROR IN hashs_add!!", __FUNCTION__, __LINE__ );
			else
				log_print( LOGN_DEBUG, "    ADD IMSI_SESS:%s IP:%u", g_A11.szMDN, stCallData.uiPCFIP );
		}
		else {
			pstCallData = (pst_CallHashData)nifo_ptr(pCALLHASH, pHASHNODE->offset_Data);

			/* SET NEW PCF IP ADDRESS */
			if( pstCallData->uiPCFIP != pPSessData->key.uiServingPCF ) {
				log_print( LOGN_WARN, "CHANHE IMSI:%s PCF IP %u -> %u", g_A11.szMDN, pstCallData->uiPCFIP, pPSessData->key.uiServingPCF );
				pstCallData->uiPCFIP = pPSessData->key.uiServingPCF;
			}
		}

		pPSessData->CreateTime	= pstCallData->stCreateTime.tv_sec;
		pPSessData->CreateMTime	= pstCallData->stCreateTime.tv_usec;

		pPSessData->uiCallType = DEF_CALL_NORMAL;	/* 착신 호 구분은 새로 시작하는 세션에 한해서 초기화한다 */

		InitPSESS( pPSessData );
	}

	pPSessData->uiStopFlag		= 0;
	pPSessData->usPPPFlag 		= 0;
	pPSessData->ucPPPSetupCount = 0;

	memset( &pPSessData->UpLCP,     0x00, DEF_TSESS_SIZE );
	memset( &pPSessData->DownLCP,   0x00, DEF_TSESS_SIZE );
	memset( &pPSessData->UpLCPEcho,	0x00, DEF_TSESS_SIZE );
	memset( &pPSessData->DownLCPEcho,0x00, DEF_TSESS_SIZE );
	memset( &pPSessData->LCPTerm,   0x00, DEF_TSESS_SIZE );
	memset( &pPSessData->UpIPCP,    0x00, DEF_TSESS_SIZE );
	memset( &pPSessData->DownIPCP,  0x00, DEF_TSESS_SIZE );
	memset( &pPSessData->CHAPAP,    0x00, DEF_TSESS_SIZE );

	sprintf( pPSessData->szMIN, "%s", g_A11.szMDN );
	if( pPSessData->szMIN[5] == '0' )
		memcpy( pPSessData->szTraceMIN, &pPSessData->szMIN[5], MAX_MIN_SIZE );
	else {
		pPSessData->szTraceMIN[0] = '0';
		memcpy( &pPSessData->szTraceMIN[1], &pPSessData->szMIN[5], MAX_MIN_SIZE-1 );
	} 

	/*
	   memcpy( pPSessData->ucBSMSC, g_A11.szBsMscId, DEF_BSMSD_LENGTH );
	 */
	if( g_A11.szSublink[0] == 0x01 && g_A11.szSublink[19] == 0x02 ) {
		for(i=0; i<6; i++)
			sprintf( &pPSessData->ucBSMSC[i*2], "%02X", g_A11.szSublink[i+31] );

		memcpy( &pPSessData->stBSMSC, &g_A11.szSublink[31], 6 );
	}

	pPSessData->ucBSMSC[DEF_BSMSD_LENGTH-1] = 0x00;

	pPSessData->ucAppType   = g_A11.ucApplicationType;
	pPSessData->uiSvcOption = g_A11.dwServiceOption;

	switch( pPSessData->uiSvcOption ) {
		case DEF_1X_NUM:
			sprintf( pPSessData->szNetOption, "%s", "1X" );
			break;

		case DEF_EVDO_NUM:
			sprintf( pPSessData->szNetOption, "%s", "EV-DO" );
			break;

		case DEF_IS95A_NUM:
			sprintf( pPSessData->szNetOption, "%s", "IS-95A" );
			break;

		case DEF_IS95B_NUM:
			sprintf( pPSessData->szNetOption, "%s", "IS-95B" );
			break;

		default:
			sprintf( pPSessData->szNetOption, "%s", "ETC" );
			break;
	}

	pPSessData->uiFMux      = g_A11.dwForwardMux;
	pPSessData->uiRMux      = g_A11.dwReverseMux;
	pPSessData->uiHomeAgent = TOULONG(g_A11.ucHomeAgentAddr);
	pPSessData->uiNASIP		= htonl(pstEth->stIP.dwDestIP);

	pPSessData->RegA11.ucAirlink   = g_A11.ucAirlinkType;
	pPSessData->RPTTime     = pstCAP->curtime;
	pPSessData->RPTMTime    = pstCAP->ucurtime;

	pPSessData->LastUpdateTime  = pstCAP->curtime;
	pPSessData->LastUpdateMTime = pstCAP->ucurtime;

	pPSessData->SESS_BUF.stPPPBuf.cFlag           = 1;
	pPSessData->SESS_BUF.stPPPBuf.LastUpdateTime  = pstCAP->curtime;
	pPSessData->SESS_BUF.stPPPBuf.uiServingPCF    = pPSessData->key.uiServingPCF;
	pPSessData->SESS_BUF.stPPPBuf.uiKey           = pPSessData->key.uiKey;

	memset( &pPSessData->stETH1, 0x00, sizeof(T_PPP_INFO) );
	memset( &pPSessData->stETH2, 0x00, sizeof(T_PPP_INFO) );

	if( pPSessData1 == NULL ) {
		ret = Insert_PSESS( pPSessData );
		if( ret < 0 ) {
			log_print( LOGN_WARN, "[FAIL:%d] Insert_PSESS()", ret );
			return -2;
		}
	}

#if 0
	log_print(LOGN_INFO, "KEY ENTRY CNT = [%u]", g_A11.ucKeyEntryCnt);
	for( i = 0; i < g_A11.ucKeyEntryCnt; i++ ) {
		stGREEntryKey.uiPCFIP = pPSessData->key.uiServingPCF;
		stGREEntryKey.uiGREKey = g_A11.dwKeyEntry[i];
		stGREEntryData.uiMainGREKey = pPSessData->key.uiKey;
		pstGREEntryData = Search_Data(hGREEntrySess, &stGREEntryKey);
		if( pstGREEntryData == NULL ) {
			ret = Insert_Data(hGREEntrySess, &stGREEntryKey, &stGREEntryData, time(0));
			if( ret < 0 ) {
				log_print( LOGN_WARN, "[FAIL:%d] Insert_Data(hGREEntrySess)", ret );
			}
			log_print( LOGN_INFO, "GRE KEY ENTRY INSERTED. PCFIP[%u] KEY[%u] MAINKEY[%u]",
					stGREEntryKey.uiPCFIP, stGREEntryKey.uiGREKey, stGREEntryData.uiMainGREKey);
		}
		else {
			log_print( LOGN_INFO, "GRE KEY ENTRY ALREADY EXIST. PCFIP[%u] KEY[%u]",
					stGREEntryKey.uiPCFIP, stGREEntryKey.uiGREKey);
		}
	}
#endif

	return 0;
}


/*******************************************************************************

*******************************************************************************/
int RemoveA10( PSESS_DATA *pPSessData )
{
	int ret;
	st_CallHashKey		stCallKey;
	pst_CallHashData	pstCallData;
	//st_GREEntry_Key		stFirstKey, stLastKey, *pstKey;
	GREENTRY_KEY		stFirstKey, stLastKey;
	GREENTRY_DATA		*pData;
	stHASHONODE         *pHASHNODE;

	// clear PSESS_BUF 
	if( pPSessData->SESS_BUF.stPPPBuf.cFlag == 1
			&& pPSessData->SESS_BUF.stPPPBuf.uiServingPCF == pPSessData->key.uiServingPCF
			&& pPSessData->SESS_BUF.stPPPBuf.uiKey == pPSessData->key.uiKey )
	{
		pPSessData->SESS_BUF.stPPPBuf.cFlag = 0;
	}
	else {
		log_print( LOGN_CRI, "RemoveA10: [PSESS_BUF not exist] IMSI[%s] CreateT[%u.%d] Key[%u]",
				pPSessData->szMIN, pPSessData->CreateTime, pPSessData->CreateMTime, pPSessData->key.uiKey );
	}

	/* DEL HASH CALL SESSION */
	stCallKey.llIMSI = atoll(pPSessData->szMIN);
	if( (pHASHNODE = hashs_find( pCALLHASH, (U8 *)&stCallKey )) != NULL ) {
		pstCallData = (pst_CallHashData)nifo_ptr(pCALLHASH, pHASHNODE->offset_Data);

		if( pstCallData->uiPCFIP == pPSessData->key.uiServingPCF ) {
			log_print( LOGN_DEBUG, "DEL IMSI_SESS:%s IP:%u", pPSessData->szMIN, pstCallData->uiPCFIP ); 
			hashs_del( pCALLHASH, (U8 *)&stCallKey );
		}
	}

	// delte PSESS
	ret = Delete_PSESS( &pPSessData->key );
	if( ret < 0 ) {
		log_print( LOGN_CRI, "RemoveA10: [FAIL:%d Delete_PSESS] IMSI[%s] CreateT[%u.%d] Key[%u]",
				ret, pPSessData->szMIN, pPSessData->CreateTime, pPSessData->CreateMTime, pPSessData->key.uiKey );
		return -1;
	}

	stFirstKey.uiPCFIP = pPSessData->key.uiServingPCF;
	stFirstKey.uiGREKey = 0;
	stLastKey.uiPCFIP = pPSessData->key.uiServingPCF;
	stLastKey.uiGREKey = 0xFFFFFFFF;
#if 0
	while( (pstKey = Select_Key(hGREEntrySess, &stFirstKey, &stLastKey)) != NULL ) {
		ret = Delete_Data(hGREEntrySess, pstKey);
		if( ret < 0 ) {
			log_print(LOGN_WARN, "Select_Key SUCCESS, but Delete_Data FAILED. RET[%d]", ret);
		}

		stFirstKey = *pstKey;
	}
#endif
	while( (pData = Select_GREENTRY(&stFirstKey, &stLastKey)) ) {
		ret = Delete_GREENTRY(&pData->key);
		if( ret < 0 ) {
			log_print(LOGN_WARN, "Select_GREENTRY SUCCESS, but Delete_Data FAILED. RET[%d]", ret);
		}

		stFirstKey = pData->key;
	}

	return 0;
}


/*******************************************************************************

*******************************************************************************/
int ProcPPPSess( PSESS_DATA *pPSessData )
{
	/* LCP */
	if( pPSessData->usPPPFlag & 0x000F ) {
		if( pPSessData->UpLCP.ucFlag > 0 )
			Report_SIGLog( NUM_UP_LCP, pPSessData->UpLCP.ucReqMsgCode, pPSessData );

		if( pPSessData->DownLCP.ucFlag > 0 )
			Report_SIGLog( NUM_DOWN_LCP, pPSessData->DownLCP.ucReqMsgCode, pPSessData );
	}

	/* LCP TERM */
	switch( pPSessData->usPPPFlag & 0x00F0 ) {
		case 0x0010:
			if( pPSessData->LCPTerm.ucFlag > 0 )
                Report_SIGLog( NUM_UP_LCP, pPSessData->LCPTerm.ucReqMsgCode, pPSessData );

			break;

		case 0x0040:
			if( pPSessData->LCPTerm.ucFlag > 0 )
                Report_SIGLog( NUM_DOWN_LCP, pPSessData->LCPTerm.ucReqMsgCode, pPSessData );

			break;

		default:
			log_print( LOGN_DEBUG, "ProcPPPSess: Unknown PPPFlag[0x%04x]", pPSessData->usPPPFlag );
			break;
	}

	/* CHAP / PAP */
	if( pPSessData->usPPPFlag & 0x0F00 ) {
		if( pPSessData->CHAPAP.ucFlag > 0 )
        	Report_SIGLog( NUM_CHAP, pPSessData->CHAPAP.ucReqMsgCode, pPSessData );
	}

	/* IPCP */
	if( pPSessData->usPPPFlag & 0xF000 ) {
		if( pPSessData->UpIPCP.ucFlag > 0 )
        	Report_SIGLog( NUM_UP_IPCP, pPSessData->UpIPCP.ucReqMsgCode, pPSessData );

		if( pPSessData->DownIPCP.ucFlag > 0 )
        	Report_SIGLog( NUM_DOWN_IPCP, pPSessData->DownIPCP.ucReqMsgCode, pPSessData );
	}

	/* LCP ECHO */
	if( pPSessData->UpLCPEcho.ucFlag > 0 )
		Report_SIGLog( NUM_UP_LCP, pPSessData->UpLCPEcho.ucReqMsgCode, pPSessData );

	if( pPSessData->DownLCPEcho.ucFlag > 0 )
		Report_SIGLog( NUM_DOWN_LCP, pPSessData->DownLCPEcho.ucReqMsgCode, pPSessData );



	return 0;
}








/*******************************************************************************

*******************************************************************************
int ProcPPPSess( PSESS_DATA *pPSessData )
{
    switch( (pPSessData->usPPPFlag & 0x000f) )
    {  
        case 0x0000:
        case 0x0003:
        case 0x000c:
        case 0x000f:
            break;
        case 0x0001:
        case 0x000d:
            if( pPSessData->UpLCP.ucFlag > 0 )
                Report_SIGLog( NUM_UP_LCP, pPSessData->UpLCP.ucReqMsgCode, pPSessData );
            else
                log_print( LOGN_DEBUG, "ProcPPPSess: Wrong PPPFlag[0x%04x]", pPSessData->usPPPFlag );
            break;
        case 0x0004:
        case 0x0007:
            if( pPSessData->DownLCP.ucFlag > 0 )
                Report_SIGLog( NUM_DOWN_LCP, pPSessData->DownLCP.ucReqMsgCode, pPSessData );
            else
                log_print( LOGN_DEBUG, "ProcPPPSess: Wrong PPPFlag[0x%04x]", pPSessData->usPPPFlag );
            break;
        case 0x0005:
            if( pPSessData->UpLCP.ucFlag > 0 )
                Report_SIGLog( NUM_UP_LCP, pPSessData->UpLCP.ucReqMsgCode, pPSessData );

            if( pPSessData->DownLCP.ucFlag > 0 )
                Report_SIGLog( NUM_DOWN_LCP, pPSessData->DownLCP.ucReqMsgCode, pPSessData );
            break;
        default:
            log_print( LOGN_DEBUG, "ProcPPPSess: Unknown PPPFlag[0x%04x]", pPSessData->usPPPFlag );
            break;
    }

	switch( (pPSessData->usPPPFlag & 0x00f0) )
    {
        case 0x0000:
        case 0x0030:
        case 0x00c0:
        case 0x00f0:
            break;
        case 0x0010:
            if( pPSessData->LCPTerm.ucFlag > 0 )
                Report_SIGLog( NUM_UP_LCP, pPSessData->LCPTerm.ucReqMsgCode, pPSessData );
            else
                log_print( LOGN_DEBUG, "ProcPPPSess: Wrong PPPFlag[0x%04x]", pPSessData->usPPPFlag );
            break;

        case 0x0040:
            if( pPSessData->LCPTerm.ucFlag > 0 )
                Report_SIGLog( NUM_DOWN_LCP, pPSessData->LCPTerm.ucReqMsgCode, pPSessData );
            else
                log_print( LOGN_DEBUG, "ProcPPPSess: Wrong PPPFlag[0x%04x]", pPSessData->usPPPFlag );
            break;

        default:
            log_print( LOGN_DEBUG, "ProcPPPSess: Unknown PPPFlag[0x%04x]", pPSessData->usPPPFlag );
    }

    switch( (pPSessData->usPPPFlag & 0x0f00) )
    { 
        case 0x0000:
        case 0x0f00:
            break;
        case 0x0300:
            if( pPSessData->CHAPAP.ucFlag > 0 )
                Report_SIGLog( NUM_CHAP, pPSessData->CHAPAP.ucReqMsgCode, pPSessData );
            else
                log_print( LOGN_DEBUG, "ProcPPPSess: Wrong PPPFlag[0x%04x]", pPSessData->usPPPFlag );
            break;
        default:
            log_print( LOGN_DEBUG, "ProcPPPSess: Unknown PPPFlag[0x%04x]", pPSessData->usPPPFlag );
    }

	switch( (pPSessData->usPPPFlag & 0xf000) )
    {
        case 0x0000:
        case 0x3000:
        case 0xc000:
        case 0xf000:
            break;
        case 0x1000:
        case 0xd000:
            if( pPSessData->UpIPCP.ucFlag > 0 )
                Report_SIGLog( NUM_UP_IPCP, pPSessData->UpIPCP.ucReqMsgCode, pPSessData );
            else
                log_print( LOGN_DEBUG, "ProcPPPSess: Wrong PPPFlag[0x%04x]", pPSessData->usPPPFlag );
            break;
        case 0x4000:
        case 0x7000:
            if( pPSessData->DownIPCP.ucFlag > 0 )
                Report_SIGLog( NUM_DOWN_IPCP, pPSessData->DownIPCP.ucReqMsgCode, pPSessData );
            else
                log_print( LOGN_DEBUG, "ProcPPPSess: Wrong PPPFlag[0x%04x]", pPSessData->usPPPFlag );
            break;
        case 0x5000:
            if( pPSessData->UpIPCP.ucFlag > 0 )
                Report_SIGLog( NUM_UP_IPCP, pPSessData->UpIPCP.ucReqMsgCode, pPSessData );

            if( pPSessData->DownIPCP.ucFlag > 0 )
                Report_SIGLog( NUM_DOWN_IPCP, pPSessData->DownIPCP.ucReqMsgCode, pPSessData );
            break;
        default:
            log_print( LOGN_DEBUG, "ProcPPPSess: Unknown PPPFlag[0x%04x]", pPSessData->usPPPFlag );
    }

    return 0;
}
*/


int Print_GREEntry(void)
{
#if 0
	st_GREEntry_Key stKey1, stKey2, *pKey;
	st_GREEntry_Data *pData;

	memset(&stKey1, 0, sizeof(stKey1));
	memset(&stKey2, 0xFF, sizeof(stKey2));

	while( (pKey = Select_Key(hGREEntrySess, &stKey1, &stKey2)) ) {
		pData = Search_Data(hGREEntrySess, pKey);
		if( pData ) {
			log_print(LOGN_CRI, "PCF[%u] GRE[%u] MAINGRE[%u]", pKey->uiPCFIP, pKey->uiGREKey, pData->uiMainGREKey);
		}
		else {
			log_print(LOGN_CRI, "ERROR!!!");
		}

		stKey1 = *pKey;
	}
#endif

	return 0;
}
