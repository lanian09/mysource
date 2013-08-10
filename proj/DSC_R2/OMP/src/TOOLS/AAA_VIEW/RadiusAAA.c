/**********************************************************
                 LG_BSD Project

   Author   : Park Si Woo
   Section  : Infravalley Develpment
   SCCS ID  : @(#)RadiusAAA.c	1.5
   Date     : 8/26/03
   Revision History :
    '03.  1. 6  Revised by Siwoo Park


   Description:

   Copyright (c) Infravalley 2003
***********************************************************/

/**A.1*  File Inclusion ***********************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <sys/msg.h>
#include <errno.h>

#include <ipaf_svc.h>
#include <ipam_sessif_new.h>
#include <udrgen_define.h>


/**B.1*  Definition of New Constants *********************/
#define VENDOR_3GPP2 	5535
#define MAX_URL_LEN     241

/**B.2*  Definition of New Type  **************************/


/**C.1*  Declaration of Variables  ************************/

extern int errno;

extern ULONG CVT_ULONG( ULONG value );
extern LONG CVT_LONG( LONG value );
extern INT CVT_INT( INT value );
extern UINT CVT_UINT( UINT value );
extern SHORT CVT_SHORT( SHORT value );
extern INT64 CVT_INT64( INT64 value );


int ParsingRadiusAAA( UCHAR *pszBuf, unsigned short dBufSize, unsigned int ulFromIP, 
                      pst_AAAREQ pstRDPkt , int UDRCount)
{
	int 	dType, dVSType, dValLen; 
	short   sTemp;
	int		dRet;
	int 	dIdx		= 0;
	int		dTempSize	= 0;
	int 	dTempOrgSize= 0;
	int    	dTempVal	= 0;
	int     dURLSIZE	= 0;
	UINT    uiVendorID	= 0;
	UCHAR	ucTemp;
	UCHAR	szAttrVal[241];

	/* UDR Count check */
    int     i=0;

    st_AAAREQ  	stAAA;
	st_UDRInfo  stUDRInfo;
	char	szDoatVal[16], *ptr;		// 080220, poopee, HBIT
	
	pst_RADIUS_HEAD  pstRds = (st_RADIUS_HEAD*)&pszBuf[0];

	//pstRds->usLength = CVT_SHORT(pstRds->usLength);

	dTempOrgSize                    = pstRds->usLength - sizeof(st_RADIUS_HEAD);
	dTempSize                       = dTempOrgSize;
	pstRDPkt->stInfo.ucCode         = pstRds->ucCode;
	pstRDPkt->stInfo.ucID           = pstRds->ucID;
	pstRDPkt->stInfo.uiRADIUSLen    = pstRds->usLength;
	memcpy( &pstRDPkt->stInfo.szAuthen[0], &pstRds->szAuth[0], 16 );

    pstRDPkt->dUDRCount = UDRCount; 


	if( pstRds->usLength != dBufSize )
		return -1;

	dIdx += sizeof(st_RADIUS_HEAD) ;

	while(dTempSize > 4)
	{
		dType   = 0;
		dVSType = 0;
		sTemp   = 0;	

        dRet = ParsingAttr( &dType, &dVSType, &szAttrVal[0], &dValLen, 
                            &pszBuf[dIdx], &dTempSize );

		/*
		fprintf(stderr,"---->Type[%d]dVSType[%d]ret[%d]attr_len[%d]<----\n", 
							dType,dVSType,dRet,dValLen);
		*/

        if( dRet < 0 )
        {
            fprintf(stderr,"[ERROR] INVALID_PARAMETER_VALUE");
            return -2;
        }

		dIdx 		   += dTempSize;
		dTempOrgSize   -= dTempSize;
		dTempSize 		= dTempOrgSize;

		switch( dType )
		{
			case 26:
				switch( dVSType )
				{
				case 200:
                    if( dValLen == sizeof(INT))
                    {
                        memcpy( &pstRDPkt->stInfo.uiTimeStamp, &szAttrVal[0], 4 ); 
                        //pstRDPkt->stInfo.uiTimeStamp = CVT_INT(pstRDPkt->stInfo.uiTimeStamp);
                        pstRDPkt->stInfo.ucTimeStampF = 0x01;
                    }
                    break;

                case 201:
                    if( dValLen == sizeof(INT))
                    {
                        memcpy( &pstRDPkt->stInfo.uiUDRSeq, &szAttrVal[0], dValLen );  
                        //pstRDPkt->stInfo.uiUDRSeq = CVT_INT(pstRDPkt->stInfo.uiUDRSeq);
                        pstRDPkt->stInfo.ucUDRSeqF = 0x01;
                    }
                    break;

				/* 20070206(challa) chagne EVDO:failCcde(0,1) ---> */
                case 202:
                    if( dValLen == sizeof(INT) )
                    {
						memcpy( &pstRDPkt->stInfo.dReserved, &szAttrVal[0], dValLen );
						pstRDPkt->stInfo.ucMDNF = 0x01;
                    }
                    break;
				/* <--- */

                case 52:
					/* ESN */
                    if( dValLen >0 && dValLen < MAX_MIN_SIZE )
                    {
						memcpy( &pstRDPkt->stInfo.szESN, &szAttrVal[0], dValLen );
						pstRDPkt->stInfo.szESN[dValLen] = 0x00;
						pstRDPkt->stInfo.ucESNF = 0x01;
                    }
                    break;


				case 44:
					// Correlation ID 
					if( dValLen == sizeof(INT64) )
					{
						memcpy(&pstRDPkt->stInfo.llCorrelID, &szAttrVal[0], dValLen );
						//pstRDPkt->stInfo.llCorrelID = CVT_INT64(pstRDPkt->stInfo.llCorrelID);
						pstRDPkt->stInfo.ucCorrelationIDF = 0x01;
					}
					break;

                case 48:
                    if( dValLen == sizeof(INT))
                    {
                        memcpy( &pstRDPkt->stInfo.uiSessContinue, &szAttrVal[0], dValLen );  
                        //pstRDPkt->stInfo.uiSessContinue = CVT_INT(pstRDPkt->stInfo.uiSessContinue);
                        pstRDPkt->stInfo.ucSessContinueF = 0x01;
                    }
                    break;

                case 51:
                    if( dValLen == sizeof(INT))
                    {
                        memcpy( &pstRDPkt->stInfo.uiBeginnigSess, &szAttrVal[0], dValLen );  
                        //pstRDPkt->stInfo.uiBeginnigSess = CVT_INT(pstRDPkt->stInfo.uiBeginnigSess);
                        pstRDPkt->stInfo.ucBeginningSessF = 0x01;
                    }
                    break;

				case 7:
					// HA IP ADDRESS, HOME Agent Attribute M Condition
					if( dValLen == sizeof(UINT) )
					{
						memcpy( &pstRDPkt->stInfo.uiHAIP, &szAttrVal[0], dValLen );
						pstRDPkt->stInfo.ucHAIPF = 0x01;
					}

					break;

				case 9:
					// PCF IP ADDRESS , Serving PCF 
					if( dValLen == sizeof(UINT) )
					{
						memcpy( &pstRDPkt->stInfo.uiPCFIP, &szAttrVal[0], dValLen );
						pstRDPkt->stInfo.ucPCFIPF = 0x01;
					}

					break;

                case 10:
                    if( dValLen > 0 && dValLen <= MAX_BSID_SIZE)
                    {
						memcpy( pstRDPkt->stInfo.szBSID, szAttrVal, dValLen);
						pstRDPkt->stInfo.szBSID[dValLen] = 0x00;
						pstRDPkt->stInfo.ucBSIDF = 0x01;

					}

					break;

				case 11:
					if( dValLen == sizeof(INT))
					{
						memcpy( &pstRDPkt->stInfo.dUserID, &szAttrVal[0], dValLen );
						//pstRDPkt->stInfo.dUserID = CVT_INT(pstRDPkt->stInfo.dUserID);
						pstRDPkt->stInfo.ucUserIDF = 0x01;
					}

					break;

				case 12:
				 	// Forward MUX M Condition
					if( dValLen == sizeof(INT) ) 
                    {
						memcpy( &pstRDPkt->stInfo.dFwdFCHMux, &szAttrVal[0], dValLen );
						//pstRDPkt->stInfo.dFwdFCHMux = CVT_INT( pstRDPkt->stInfo.dFwdFCHMux );
						pstRDPkt->stInfo.ucFwdFCHMuxF = 0x01;
					}
					break;

				case 13:
					// Reverse MUX M Condition
					if( dValLen == sizeof(INT) ) 
                    {
						memcpy( &pstRDPkt->stInfo.dRevFCHMux, &szAttrVal[0], dValLen );
						//pstRDPkt->stInfo.dRevFCHMux = CVT_INT( pstRDPkt->stInfo.dRevFCHMux );
                        pstRDPkt->stInfo.ucRevFCHMuxF = 0x01;
                    }
                    break;

				case 16:
					// Service Option 
					if( dValLen == sizeof(INT) ) 
                    {
						memcpy( &pstRDPkt->stInfo.dSvcOpt, &szAttrVal[0], dValLen );
						//pstRDPkt->stInfo.dSvcOpt = CVT_INT(pstRDPkt->stInfo.dSvcOpt);
						pstRDPkt->stInfo.ucSvcOptF = 0x01;
					}
					break;
	
				case 17:
					if( dValLen == sizeof(INT) ) 
                    {
						memcpy( &pstRDPkt->stInfo.dFwdTrafType, &szAttrVal[0], dValLen );
						//pstRDPkt->stInfo.dFwdTrafType   = CVT_INT(pstRDPkt->stInfo.dFwdTrafType);
						pstRDPkt->stInfo.ucFwdTrafTypeF = 0x01;

					}
					break;
	
				case 18:
					// 3GPP-SGSN-Address IP ADDRESS
					if( dValLen == sizeof(INT) )
					{
						memcpy( &pstRDPkt->stInfo.dRevTrafType, &szAttrVal[0], dValLen );
                        //pstRDPkt->stInfo.dRevTrafType   = CVT_INT(pstRDPkt->stInfo.dRevTrafType);
						pstRDPkt->stInfo.ucRevTrafTypeF = 0x01;
					}

					break;

				case 19:
				 	// Forward MUX M Condition
					if( dValLen == 1 )
					{
						ucTemp = szAttrVal[0];
						pstRDPkt->stInfo.dFCHSize = ucTemp ;
						pstRDPkt->stInfo.ucFCHSizeF = 0x01;
					}
					else if( dValLen == 2 )
					{
						memcpy( &sTemp, &szAttrVal[0], 2 );
						//sTemp = CVT_SHORT(sTemp);
						pstRDPkt->stInfo.dFCHSize = sTemp ;
						pstRDPkt->stInfo.ucFCHSizeF = 0x01;
					}
					else if( dValLen == 4 )
					{
						memcpy( &dTempVal, &szAttrVal[0], dValLen );
						//dTempVal = CVT_INT(dTempVal);

						pstRDPkt->stInfo.dFCHSize = dTempVal;
						pstRDPkt->stInfo.ucFCHSizeF = 0x01;
					}
					else
						return -1;
					break;

                case 20:
					if( dValLen == 1 )
					{
						ucTemp = szAttrVal[0];
						pstRDPkt->stInfo.dFwdFCHRC = ucTemp ;
						pstRDPkt->stInfo.ucFwdFCHRCF = 0x01;
					}
					else if( dValLen == 2 )
					{
						memcpy( &sTemp, &szAttrVal[0], 2 );
						//sTemp = CVT_SHORT(sTemp);
						pstRDPkt->stInfo.dFwdFCHRC = sTemp ;
						pstRDPkt->stInfo.ucFwdFCHRCF = 0x01;
					}
					else if( dValLen == 4 )
					{
						memcpy( &dTempVal, &szAttrVal[0], dValLen );
						//dTempVal = CVT_INT(dTempVal);

						pstRDPkt->stInfo.dFwdFCHRC = dTempVal;
						pstRDPkt->stInfo.ucFwdFCHRCF = 0x01;
					}
					else
						return -1;
                    break;

                case 21:
					if( dValLen == 1 )
					{
						ucTemp = szAttrVal[0];
						pstRDPkt->stInfo.dRevFCHRC = ucTemp ;
						pstRDPkt->stInfo.ucRevFCHRCF = 0x01;
					}
					else if( dValLen == 2 )
					{
						memcpy( &sTemp, &szAttrVal[0], 2 );
						//sTemp = CVT_SHORT(sTemp);
						pstRDPkt->stInfo.dRevFCHRC = sTemp ;
						pstRDPkt->stInfo.ucRevFCHRCF = 0x01;
					}
					else if( dValLen == 4 )
					{
						memcpy( &dTempVal, &szAttrVal[0], dValLen );
						//dTempVal = CVT_INT(dTempVal);

						pstRDPkt->stInfo.dRevFCHRC = dTempVal;
						pstRDPkt->stInfo.ucRevFCHRCF = 0x01;
					}
					else
						return -1;
                    break;

				case 22:
					if( dValLen == 1 )
					{
						ucTemp = szAttrVal[0];
						pstRDPkt->stInfo.dIPTech = ucTemp ;
						pstRDPkt->stInfo.ucIPTechF = 0x01;
					}
					else if( dValLen == 2 )
					{
						memcpy( &sTemp, &szAttrVal[0], 2 );
						//sTemp = CVT_SHORT(sTemp);
						pstRDPkt->stInfo.dIPTech = sTemp ;
						pstRDPkt->stInfo.ucIPTechF = 0x01;
					}
					else if( dValLen == 4 )
					{
						memcpy( &dTempVal, &szAttrVal[0], dValLen );
						//dTempVal = CVT_INT(dTempVal);

						pstRDPkt->stInfo.dIPTech = dTempVal;
						pstRDPkt->stInfo.ucIPTechF = 0x01;
					}
					else
						return -1;
					break;

                case 23:
					if( dValLen == 1 )
					{
						ucTemp = szAttrVal[0];
						pstRDPkt->stInfo.dCompTunnelInd = ucTemp ;
						pstRDPkt->stInfo.ucCompTunnelIndF = 0x01;
					}
					else if( dValLen == 2 )
					{
						memcpy( &sTemp, &szAttrVal[0], 2 );
						//sTemp = CVT_SHORT(sTemp);
						pstRDPkt->stInfo.dCompTunnelInd    = sTemp;
						pstRDPkt->stInfo.ucCompTunnelIndF  = 0x01;
					}
					else if( dValLen == 4 )
					{
						memcpy( &dTempVal, &szAttrVal[0], dValLen );
						//dTempVal = CVT_INT(dTempVal);

						pstRDPkt->stInfo.dCompTunnelInd   = dTempVal;
						pstRDPkt->stInfo.ucCompTunnelIndF = 0x01;
					}
					else
						return -1;
                    break;

				case 24:
					// Release Indicator 
					if( dValLen == sizeof(INT) )	
					{ 
						memcpy( &pstRDPkt->stInfo.dReleaseInd, &szAttrVal[0], dValLen );
						//pstRDPkt->stInfo.dReleaseInd = CVT_INT( pstRDPkt->stInfo.dReleaseInd );
						pstRDPkt->stInfo.ucReleaseIndF = 0x01;
					}

					break;

                case 50:
					if( dValLen == sizeof(INT) )	
					{ 
						memcpy( &pstRDPkt->stInfo.dDCCHSize, &szAttrVal[0], dValLen );
						//pstRDPkt->stInfo.dDCCHSize = CVT_INT( pstRDPkt->stInfo.dDCCHSize );
						pstRDPkt->stInfo.ucDCCHSizeF = 0x01;
					}
					break;

				case 78:	// Always-On
					if (dValLen == sizeof(INT))
					{
						memcpy(&pstRDPkt->stInfo.dAlwaysOn,&szAttrVal[0],dValLen);
						//pstRDPkt->stInfo.dAlwaysOn = CVT_INT(pstRDPkt->stInfo.dAlwaysOn);
					    pstRDPkt->stInfo.ucAlwaysOnF = 0x01;
					}

					break;

				case 25:	//Bad_PPP 
					if (dValLen == sizeof(INT))
					{
						memcpy(&pstRDPkt->stInfo.dBadPPPFrameCnt,&szAttrVal[0],dValLen);
						//pstRDPkt->stInfo.dBadPPPFrameCnt = CVT_INT(pstRDPkt->stInfo.dBadPPPFrameCnt);
					    pstRDPkt->stInfo.ucBadPPPFrameCntF = 0x01;
					}

					break;

                case 49:
					if (dValLen == sizeof(INT))
					{
						memcpy(&pstRDPkt->stInfo.uiActTime,&szAttrVal[0],dValLen);
						//pstRDPkt->stInfo.uiActTime = CVT_INT(pstRDPkt->stInfo.uiActTime);
					    pstRDPkt->stInfo.ucActTimeF = 0x01;
					}
                    break;


				case 30:
					// Number Active 
					if( dValLen == sizeof(INT) )
					{
						memcpy( &pstRDPkt->stInfo.dNumAct, &szAttrVal[0], dValLen );
						//pstRDPkt->stInfo.dNumAct = CVT_INT( pstRDPkt->stInfo.dNumAct );
						pstRDPkt->stInfo.ucNumActF = 0x01;

					}
					break;

				case 31:
					if( dValLen == sizeof(INT) )
					{
						memcpy( &pstRDPkt->stInfo.dTermSDBOctCnt, &szAttrVal[0], dValLen );
						//pstRDPkt->stInfo.dTermSDBOctCnt = CVT_INT( pstRDPkt->stInfo.dTermSDBOctCnt );
						pstRDPkt->stInfo.ucTermSDBOctCntF = 0x01;

					}

					break;

				case 32:
					if( dValLen == sizeof(INT) )
					{
						memcpy( &pstRDPkt->stInfo.dOrgSDBOctCnt, &szAttrVal[0], dValLen );
						//pstRDPkt->stInfo.dOrgSDBOctCnt = CVT_INT( pstRDPkt->stInfo.dOrgSDBOctCnt );
						pstRDPkt->stInfo.ucOrgSDBOctCntF = 0x01;

					}
                    break;

				case 33:
					if( dValLen == sizeof(INT) )
					{
						memcpy( &pstRDPkt->stInfo.dTermNumSDB, &szAttrVal[0], dValLen );
						//pstRDPkt->stInfo.dTermNumSDB = CVT_INT( pstRDPkt->stInfo.dTermNumSDB );
						pstRDPkt->stInfo.ucTermNumSDBF = 0x01;

					}
                    break;

				case 34:
					if( dValLen == sizeof(INT) )
					{
						memcpy( &pstRDPkt->stInfo.dOrgNumSDB, &szAttrVal[0], dValLen );
						//pstRDPkt->stInfo.dOrgNumSDB = CVT_INT( pstRDPkt->stInfo.dOrgNumSDB );
						pstRDPkt->stInfo.ucOrgNumSDBF = 0x01;

					}

					break;

                case 43:
					if( dValLen == sizeof(INT) )
					{
						memcpy( &pstRDPkt->stInfo.dRcvHDLCOct, &szAttrVal[0], dValLen );
						//pstRDPkt->stInfo.dRcvHDLCOct = CVT_INT( pstRDPkt->stInfo.dRcvHDLCOct );
						pstRDPkt->stInfo.ucRcvHDLCOctF = 0x01;

					}
					break;

                case 46:
					if( dValLen == sizeof(INT) )
					{
						memcpy( &pstRDPkt->stInfo.dInMIPSigCnt, &szAttrVal[0], dValLen );
						//pstRDPkt->stInfo.dInMIPSigCnt = CVT_INT( pstRDPkt->stInfo.dInMIPSigCnt );
						pstRDPkt->stInfo.ucInMIPSigCntF = 0x01;

					}

					break;

                case 47:
					if( dValLen == sizeof(INT) )
					{
						memcpy( &pstRDPkt->stInfo.dOutMIPSigCnt, &szAttrVal[0], dValLen );
						//pstRDPkt->stInfo.dOutMIPSigCnt = CVT_INT( pstRDPkt->stInfo.dOutMIPSigCnt );
						pstRDPkt->stInfo.ucOutMIPSigCntF = 0x01;

					}

					break;

               case 36:
					if( dValLen == sizeof(INT) )
					{
						memcpy( &pstRDPkt->stInfo.dIPQoS, &szAttrVal[0], dValLen );
						//pstRDPkt->stInfo.dIPQoS = CVT_INT( pstRDPkt->stInfo.dIPQoS );
						pstRDPkt->stInfo.ucIPQoSF = 0x01;

					}

					break;

               case 39:
					if( dValLen == sizeof(INT) )
					{
						memcpy( &pstRDPkt->stInfo.dAirQoS, &szAttrVal[0], dValLen );
						//pstRDPkt->stInfo.dAirQoS = CVT_INT( pstRDPkt->stInfo.dAirQoS );
						pstRDPkt->stInfo.ucAirQoSF = 0x01;

					}

					break;

               case 41:
					if( dValLen == sizeof(INT) )
					{
						memcpy( &pstRDPkt->stInfo.dRPConnectID, &szAttrVal[0], dValLen );
						//pstRDPkt->stInfo.dRPConnectID = CVT_INT( pstRDPkt->stInfo.dRPConnectID );
						pstRDPkt->stInfo.ucRPConnectIDF = 0x01;

					}
                    break;

				// 071203, poopee, SUBNET
                case 108:
                    if( dValLen > 0 && dValLen <= MAX_SUBNET_SIZE)
                    {
						memcpy( pstRDPkt->stInfo.szSubnet, szAttrVal, dValLen);
						pstRDPkt->stInfo.szSubnet[dValLen] = 0x00;
						pstRDPkt->stInfo.ucSubnetF = 0x01;
					}
					break;

                case 250:
					if( dValLen == sizeof(INT) )
					{
						memcpy( &pstRDPkt->stInfo.uiRetryFlg, &szAttrVal[0], dValLen );
						//pstRDPkt->stInfo.uiRetryFlg = CVT_INT( pstRDPkt->stInfo.uiRetryFlg );

                        pstRDPkt->stInfo.ucRetryF = 0x01;

					}
					break;
                
				case 1:
#if 0
					if( dValLen == sizeof(INT) )
#else
					if( dValLen > 0 )
#endif
					{
#if 1					/* 080220, poopee, HBIT */
						// "vci_c23=0", "hbit=0"
						ptr = strchr(szAttrVal,'=');	

						if (strncasecmp(szAttrVal,"vci_c23=",8) == 0)
						{
							pstRDPkt->stInfo.uiC23BIT = atoi(ptr+1);
							pstRDPkt->stInfo.ucC23BITF = 0x01;
						}
						else if (strncasecmp(szAttrVal,"hbit=",5) == 0)
						{
							pstRDPkt->stInfo.uiHBIT = atoi(ptr+1);
							pstRDPkt->stInfo.ucHBITF = 0x01;
						}
#else
						memcpy( &pstRDPkt->stInfo.uiC23BIT, &szAttrVal[0], dValLen );
						//pstRDPkt->stInfo.uiRetryFlg = CVT_INT( pstRDPkt->stInfo.uiRetryFlg );

                        pstRDPkt->stInfo.ucC23BITF = 0x01;
#endif
					}
					break;


				default :
					break;
				}

				break;

			case 31:
				if( dValLen > 0 && dValLen < MAX_MIN_SIZE )
				{
					memcpy( &pstRDPkt->stInfo.szMIN, &szAttrVal[0], dValLen );
					pstRDPkt->stInfo.szMIN[dValLen] = 0x00;
					pstRDPkt->stInfo.ucCallStatIDF = 0x01;
					
				}

				break;

			case 8:
				if( dValLen == sizeof(UINT) )
				{
					memcpy( &pstRDPkt->stInfo.uiFramedIP, &szAttrVal[0], dValLen );
					pstRDPkt->stInfo.ucFramedIPF = 0x01;
				}

				break;	

			case 1:
				if( dValLen > 0 && dValLen < MAX_USERNAME_SIZE )
				{
					memcpy( &pstRDPkt->stInfo.szUserName[0], &szAttrVal[0], dValLen );
					pstRDPkt->stInfo.szUserName[dValLen] = 0x00;
                    pstRDPkt->stInfo.ucUserLen           = dValLen;
					pstRDPkt->stInfo.ucUserNameF         = 0x01;
					
				}

				break;

			case 44:
				// Accounting Session ID M Condition
				if( dValLen == sizeof(INT64) )
				{
					memcpy( &pstRDPkt->stInfo.llAcctSessID, &szAttrVal[0], dValLen );
					/*  8byte revers ordering */	
					//pstRDPkt->stInfo.llAcctSessID = CVT_INT64( pstRDPkt->stInfo.llAcctSessID );
					pstRDPkt->stInfo.ucAcctSessIDF = 0x01;
				}
				break;
			
			case 4:
				// NAS-IP-Address 
				if( dValLen == sizeof(UINT) )
				{	
					memcpy( &pstRDPkt->stInfo.uiNASIP, &szAttrVal[0], dValLen );
					pstRDPkt->stInfo.ucNASIPF  = 0x01;
				}

				break;

			case 42 :
				if( dValLen == sizeof(INT) )
				{	
					memcpy( &pstRDPkt->stInfo.dAcctInOct, &szAttrVal[0], dValLen );
					//pstRDPkt->stInfo.dAcctInOct = CVT_INT( pstRDPkt->stInfo.dAcctInOct );
					pstRDPkt->stInfo.ucAcctInOctF = 0x01;
				}
				break;

			case 43 :
				if( dValLen == sizeof(INT) )
				{
					memcpy( &pstRDPkt->stInfo.dAcctOutOct, &szAttrVal[0], dValLen );
					//pstRDPkt->stInfo.dAcctOutOct = CVT_INT( pstRDPkt->stInfo.dAcctOutOct );
					pstRDPkt->stInfo.ucAcctOutOctF = 0x01;
				}
				break;

			case 55:
				if( dValLen == sizeof(time_t) )
				{
					memcpy( &pstRDPkt->stInfo.uiEventTime, &szAttrVal[0], dValLen );
					//pstRDPkt->stInfo.uiEventTime = CVT_INT( pstRDPkt->stInfo.uiEventTime );
					pstRDPkt->stInfo.ucEventTimeF = 0x01;
				}
	
				break;

            case 47:
				if( dValLen == sizeof(INT) )
				{
					memcpy( &pstRDPkt->stInfo.dAcctInPkt, &szAttrVal[0], dValLen );
					//pstRDPkt->stInfo.dAcctInPkt = CVT_INT( pstRDPkt->stInfo.dAcctInPkt );
					pstRDPkt->stInfo.ucAcctInPktF = 0x01;
				}
				break;


            case 48:
				if( dValLen == sizeof(INT) )
				{
					memcpy( &pstRDPkt->stInfo.dAcctOutPkt, &szAttrVal[0], dValLen );
					//pstRDPkt->stInfo.dAcctOutPkt = CVT_INT( pstRDPkt->stInfo.dAcctOutPkt );
					pstRDPkt->stInfo.ucAcctOutPktF = 0x01;
				}
				break;

            case 45:
				if( dValLen == sizeof(INT) )
				{
					memcpy( &pstRDPkt->stInfo.dAcctAuth, &szAttrVal[0], dValLen );
					//pstRDPkt->stInfo.dAcctAuth = CVT_INT( pstRDPkt->stInfo.dAcctAuth );
					pstRDPkt->stInfo.ucAcctAuthF = 0x01;
				}
				break;

            case 46:
				if( dValLen == sizeof(INT) )
				{
					memcpy( &pstRDPkt->stInfo.uiAcctSessTime, &szAttrVal[0], dValLen );
					//pstRDPkt->stInfo.uiAcctSessTime = CVT_INT( pstRDPkt->stInfo.uiAcctSessTime );
					pstRDPkt->stInfo.ucAcctSessTimeF = 0x01;
				}
				break;

            case 49:
				if( dValLen == sizeof(INT) )
				{
					memcpy( &pstRDPkt->stInfo.dAcctTermCause, &szAttrVal[0], dValLen );
					//pstRDPkt->stInfo.dAcctTermCause = CVT_INT( pstRDPkt->stInfo.dAcctTermCause );
					pstRDPkt->stInfo.ucAcctTermCauseF = 0x01;
				}
				break;

            case 40:
				if( dValLen == sizeof(INT) )
				{
					memcpy( &pstRDPkt->stInfo.dAcctStatType, &szAttrVal[0], dValLen );
					//pstRDPkt->stInfo.dAcctStatType = CVT_INT( pstRDPkt->stInfo.dAcctStatType );

					pstRDPkt->stInfo.ucAcctStatTypeF = 0x01;
				}
				break;

            case 61:
				if( dValLen == sizeof(INT) )
				{
					memcpy( &pstRDPkt->stInfo.dNASPortType, &szAttrVal[0], dValLen );
					//pstRDPkt->stInfo.dNASPortType = CVT_INT( pstRDPkt->stInfo.dNASPortType );
					pstRDPkt->stInfo.ucNASPortTypeF = 0x01;
				}
				break;

            case 5:
				if( dValLen == sizeof(INT) )
				{
					memcpy( &pstRDPkt->stInfo.dNASPort, &szAttrVal[0], dValLen );
					//pstRDPkt->stInfo.dNASPort = CVT_INT( pstRDPkt->stInfo.dNASPort );
					pstRDPkt->stInfo.ucNASPortF = 0x01;
				}
				break;

            case 87:
                if( dValLen > 0 && dValLen < MAX_PORT_SIZE)
                {
                    memcpy(pstRDPkt->stInfo.szNASPortID, szAttrVal, dValLen );
                    pstRDPkt->stInfo.szNASPortID[dValLen] = 0x00;
                    pstRDPkt->stInfo.ucNASPortIDF         = 0x01;
                }
				break;

            case 6:
				if( dValLen == sizeof(INT) )
				{
					memcpy( &pstRDPkt->stInfo.dSvcType, &szAttrVal[0], dValLen );
					//pstRDPkt->stInfo.dSvcType   = CVT_INT( pstRDPkt->stInfo.dSvcType );
					pstRDPkt->stInfo.ucSvcTypeF = 0x01;
				}
				break;

            case 41:
				if( dValLen == sizeof(INT) )
				{
					memcpy( &pstRDPkt->stInfo.uiAcctDelayTime, &szAttrVal[0], dValLen );
					//pstRDPkt->stInfo.uiAcctDelayTime   = CVT_INT( pstRDPkt->stInfo.uiAcctDelayTime );
					pstRDPkt->stInfo.ucAcctDelayTimeF  = 0x01;
				}
				break;

			default :
				break;
		}


        if( pstRDPkt->dUDRCount > 0 )
        {
		    switch( dType )
            {
            case 26:
			    switch( dVSType )
				{
                case 203:
					i++;			

					if( dValLen == sizeof(INT) )
					{
						memcpy( &pstRDPkt->stUDRInfo[i-1].dDataSvcType, &szAttrVal[0], dValLen );
 
                        pstRDPkt->stUDRInfo[i-1].ucDataSvcTypeF = 0x01; 


					}
					break;

                case 204:
					if( dValLen == sizeof(INT) )
					{
						memcpy( &pstRDPkt->stUDRInfo[i-1].uiTranID, &szAttrVal[0], dValLen );
                        pstRDPkt->stUDRInfo[i-1].ucTranIDF = 0x01; 

					}
					break;

                case 205:
					if( dValLen == sizeof(INT) )
					{
						memcpy( &pstRDPkt->stUDRInfo[i-1].tReqTime, &szAttrVal[0], dValLen );

						/*
						pstRDPkt->stUDRInfo[i-1].tReqTime = 
                                CVT_INT( pstRDPkt->stUDRInfo[i-1].tReqTime );
						*/

                        pstRDPkt->stUDRInfo[i-1].ucReqTimeF = 0x01; 
					}
					break;

                case 206:
					if( dValLen == sizeof(INT) )
					{
						memcpy( &pstRDPkt->stUDRInfo[i-1].tResTime, &szAttrVal[0], dValLen );

						/*
						pstRDPkt->stUDRInfo[i-1].tResTime = 
                                 CVT_INT( pstRDPkt->stUDRInfo[i-1].tResTime );
						*/

                        pstRDPkt->stUDRInfo[i-1].ucResTimeF = 0x01; 
					}
					break;

                case 207:
					if( dValLen == sizeof(INT) )
					{
						memcpy( &pstRDPkt->stUDRInfo[i-1].tSessionTime, &szAttrVal[0], dValLen );
						/*
						pstRDPkt->stUDRInfo[i-1].tSessionTime = 
                                CVT_INT( pstRDPkt->stUDRInfo[i-1].tSessionTime );
						*/

                        pstRDPkt->stUDRInfo[i-1].ucSessionTimeF = 0x01; 

						
					}
					break;

                case 208:
					if( dValLen == sizeof(UINT) )
                    {
						memcpy( &pstRDPkt->stUDRInfo[i-1].uiDestIP, &szAttrVal[0], dValLen );
                        pstRDPkt->stUDRInfo[i-1].ucDestIPF = 0x01; 

                    }

					break;

                case 209:
					if( dValLen == sizeof(INT) )
					{
						memcpy( &pstRDPkt->stUDRInfo[i-1].dDestPort, &szAttrVal[0], dValLen );
						/*
						pstRDPkt->stUDRInfo[i-1].dDestPort = 
                                CVT_INT( pstRDPkt->stUDRInfo[i-1].dDestPort );
						*/

                        pstRDPkt->stUDRInfo[i-1].ucDestPortF = 0x01; 

					}
					break;

                case 210:
					if( dValLen == sizeof(INT) )
					{
						memcpy( &pstRDPkt->stUDRInfo[i-1].dSrcPort, &szAttrVal[0], dValLen );
						/*
						pstRDPkt->stUDRInfo[i-1].dSrcPort = 
                                CVT_INT( pstRDPkt->stUDRInfo[i-1].dSrcPort );
						*/

                        pstRDPkt->stUDRInfo[i-1].ucSrcPortF = 0x01; 

					}
					break;


                case 225:
					if( dValLen > 0 && dValLen < MAX_URL_LEN  )
					{
                        memcpy( &pstRDPkt->stUDRInfo[i-1].szURL[0], &szAttrVal[0], dValLen); 
                        pstRDPkt->stUDRInfo[i-1].szURL[dValLen] = '\0';
                        pstRDPkt->stUDRInfo[i-1].ucURLF = 0x01; 
					}
					break;

                case 226:
					if( dValLen > 0 && dValLen < MAX_URL_LEN )
					{
						if( pstRDPkt->stUDRInfo[i-1].ucURLF == 0x01 )
						{
							strcat( pstRDPkt->stUDRInfo[i-1].szURL, szAttrVal );
							dURLSIZE =  strlen( pstRDPkt->stUDRInfo[i-1].szURL );
                        	pstRDPkt->stUDRInfo[i-1].szURL[dURLSIZE] = '\0';
						}
						
					}
					break;

				case 227:
					if( dValLen > 0 && dValLen < MAX_URL_LEN )
					{
						if( pstRDPkt->stUDRInfo[i-1].ucURLF == 0x01 )
						{
							strcat( pstRDPkt->stUDRInfo[i-1].szURL, szAttrVal );
							dURLSIZE =  strlen( pstRDPkt->stUDRInfo[i-1].szURL );
                        	pstRDPkt->stUDRInfo[i-1].szURL[dURLSIZE] = '\0';
						}
					}
					break;
					
						
                case 212:
					if( dValLen == sizeof(INT) )
					{
						memcpy( &pstRDPkt->stUDRInfo[i-1].dCType, &szAttrVal[0], dValLen );
						//pstRDPkt->stUDRInfo[i-1].dCType = CVT_INT( pstRDPkt->stUDRInfo[i-1].dCType );
                        pstRDPkt->stUDRInfo[i-1].ucCTypeF = 0x01; 

					}
					break;

				/* R1.3.0 add(challa) ---> */
                case 213:
					if( dValLen > 0 && dValLen < MAX_APPID_SIZE )
					{
						memcpy( &pstRDPkt->stUDRInfo[i-1].szAppID[0], &szAttrVal[0], dValLen );
						pstRDPkt->stUDRInfo[i-1].szAppID[dValLen] = 0x00;
						
                        pstRDPkt->stUDRInfo[i-1].ucAppIDF = 0x01; 

					}
					break;

                case 214:
					if( dValLen > 0 && dValLen < MAX_CNTS_SIZE )
					{
						memcpy( &pstRDPkt->stUDRInfo[i-1].szContentCode[0], &szAttrVal[0], dValLen );
						pstRDPkt->stUDRInfo[i-1].szContentCode[dValLen] = 0x00;

                        pstRDPkt->stUDRInfo[i-1].ucContentCodeF = 0x01; 

					}
					break;
				/* <--- */

                case 215:
					if( dValLen == sizeof(INT) )
					{
						memcpy( &pstRDPkt->stUDRInfo[i-1].dMethodType, &szAttrVal[0], dValLen );
						/*
						pstRDPkt->stUDRInfo[i-1].dMethodType = 
                                CVT_INT( pstRDPkt->stUDRInfo[i-1].dMethodType );
						*/
                        pstRDPkt->stUDRInfo[i-1].ucMethodTypeF = 0x01; 

					}
					break;

                case 216:
					if( dValLen == sizeof(INT) )
					{
						memcpy( &pstRDPkt->stUDRInfo[i-1].dResultCode, &szAttrVal[0], dValLen );
						/*
						pstRDPkt->stUDRInfo[i-1].dResultCode = 
                                 CVT_INT( pstRDPkt->stUDRInfo[i-1].dResultCode );
						*/
                        pstRDPkt->stUDRInfo[i-1].ucResultCodeF = 0x01; 

					}
					break;

                case 217:
					if( dValLen == sizeof(INT) )
					{
						memcpy( &pstRDPkt->stUDRInfo[i-1].dIPUpSize, &szAttrVal[0], dValLen );
						//pstRDPkt->stUDRInfo[i-1].dIPUpSize = CVT_INT( pstRDPkt->stUDRInfo[i-1].dIPUpSize );

                        pstRDPkt->stUDRInfo[i-1].ucIPUpSizeF = 0x01; 
					}
					break;

                case 218:
					if( dValLen == sizeof(INT) )
					{
						memcpy( &pstRDPkt->stUDRInfo[i-1].dIPDownSize, &szAttrVal[0], dValLen );
						/*
						pstRDPkt->stUDRInfo[i-1].dIPDownSize = 
                                CVT_INT( pstRDPkt->stUDRInfo[i-1].dIPDownSize );
						*/

                        pstRDPkt->stUDRInfo[i-1].ucIPDownSizeF = 0x01; 
					}
					break;

                case 219:
					if( dValLen == sizeof(INT) )
					{
						memcpy( &pstRDPkt->stUDRInfo[i-1].dRetransInSize, &szAttrVal[0], dValLen );
						/*
						pstRDPkt->stUDRInfo[i-1].dRetransInSize = 
                                CVT_INT( pstRDPkt->stUDRInfo[i-1].dRetransInSize );
						*/

                        pstRDPkt->stUDRInfo[i-1].ucRetransInSizeF = 0x01; 
					}
					break;

                case 220:
					if( dValLen == sizeof(INT) )
					{
						memcpy( &pstRDPkt->stUDRInfo[i-1].dRetransOutSize, &szAttrVal[0], dValLen );
						/*
						pstRDPkt->stUDRInfo[i-1].dRetransOutSize = 
                                CVT_INT( pstRDPkt->stUDRInfo[i-1].dRetransOutSize );
						*/

                        pstRDPkt->stUDRInfo[i-1].ucRetransOutSizeF = 0x01; 
					}
					break;

				/* R1.3.0 20061128 add(challa) ---> */
                case 230:
                    if( dValLen == sizeof(INT) )
                    {
                        memcpy(&pstRDPkt->stUDRInfo[i-1].dUseCount, &szAttrVal[0], dValLen );
                        pstRDPkt->stUDRInfo[i-1].ucUseCountF = 0x01;
                    }
                    break;

                case 231:
                    if( dValLen == sizeof(INT) )
                    {
                        memcpy(&pstRDPkt->stUDRInfo[i-1].dUseTime, &szAttrVal[0], dValLen );
                        pstRDPkt->stUDRInfo[i-1].ucUseTimeF = 0x01;
                    }
                    break;

                case 232:
                    if( dValLen == sizeof(INT) )
                    {
                        memcpy(&pstRDPkt->stUDRInfo[i-1].dTotalSize, &szAttrVal[0], dValLen );
                        pstRDPkt->stUDRInfo[i-1].ucTotalSizeF = 0x01;
                    }
                    break;

                case 233:
                    if( dValLen == sizeof(INT) )
                    {
                        memcpy(&pstRDPkt->stUDRInfo[i-1].dTotalTime, &szAttrVal[0], dValLen );
                        pstRDPkt->stUDRInfo[i-1].ucTotalTimeF = 0x01;
                    }
                    break;
				/* <--- */


				/* 20070104 Add: NEW UDR ---> */
                case 238:
                    if( dValLen == sizeof(INT) )
                    {
                        memcpy(&pstRDPkt->stUDRInfo[i-1].dAudioInputIPSize, &szAttrVal[0], dValLen );
                        pstRDPkt->stUDRInfo[i-1].ucAudioInputIPSizeF = 0x01;
                    }
                    break;

                case 239:
                    if( dValLen == sizeof(INT) )
                    {
                        memcpy(&pstRDPkt->stUDRInfo[i-1].dAudioOutputIPSize, &szAttrVal[0], dValLen );
                        pstRDPkt->stUDRInfo[i-1].ucAudioOutputIPSizeF = 0x01;
                    }
                    break;

                case 240:
                    if( dValLen == sizeof(INT) )
                    {
                        memcpy(&pstRDPkt->stUDRInfo[i-1].dVideoInputIPSize, &szAttrVal[0], dValLen );
                        pstRDPkt->stUDRInfo[i-1].ucVideoInputIPSizeF = 0x01;
                    }
                    break;

                case 241:
                    if( dValLen == sizeof(INT) )
                    {
                        memcpy(&pstRDPkt->stUDRInfo[i-1].dVideoOutputIPSize, &szAttrVal[0], dValLen );
                        pstRDPkt->stUDRInfo[i-1].ucVideoOutputIPSizeF = 0x01;
                    }
                    break;

				/* <--- */

                case 221:
					if( dValLen == sizeof(INT) )
					{
						memcpy( &pstRDPkt->stUDRInfo[i-1].dContentLen, &szAttrVal[0], dValLen );
						/*
						pstRDPkt->stUDRInfo[i-1].dContentLen = 
                                CVT_INT( pstRDPkt->stUDRInfo[i-1].dContentLen );
						*/

                        pstRDPkt->stUDRInfo[i-1].ucContentLenF = 0x01; 
					}
					break;

                case 222:
					if( dValLen == sizeof(INT) )
					{
						memcpy( &pstRDPkt->stUDRInfo[i-1].dTranComplete, &szAttrVal[0], dValLen );
						/*
						pstRDPkt->stUDRInfo[i-1].dTranComplete = 
                                CVT_INT( pstRDPkt->stUDRInfo[i-1].dTranComplete );
						*/

                        pstRDPkt->stUDRInfo[i-1].ucTranCompleteF = 0x01; 
					}
					break;

                case 223:
					if( dValLen == sizeof(INT) )
					{
						memcpy( &pstRDPkt->stUDRInfo[i-1].dTranTermReason, &szAttrVal[0], dValLen );
						/*
						pstRDPkt->stUDRInfo[i-1].dTranTermReason = 
                                CVT_INT( pstRDPkt->stUDRInfo[i-1].dTranTermReason );
						*/

                        pstRDPkt->stUDRInfo[i-1].ucTranTermReasonF = 0x01; 

					}
					break;

                case 224:
					if( dValLen >0 && dValLen < MAX_USERAGENT_SIZE )
					{
                        memcpy( &pstRDPkt->stUDRInfo[i-1].szUserAgent[0], &szAttrVal[0], dValLen); 
                        pstRDPkt->stUDRInfo[i-1].szUserAgent[dValLen] = 0x00;

                        pstRDPkt->stUDRInfo[i-1].ucUserAgentF = 0x01; 

					}
					
					break;

                /* R1.3.0 20061128 add(challa) ---> */
                case 237:
                    if( dValLen >0 && dValLen < DEF_PKTCNT_SIZE )
                    {
                        memcpy( &pstRDPkt->stUDRInfo[i-1].szPacketCnt[0], &szAttrVal[0], dValLen);
                        pstRDPkt->stUDRInfo[i-1].szPacketCnt[dValLen] = 0x00;
                        pstRDPkt->stUDRInfo[i-1].ucPacketCntF = 0x01;
                    }
                    break;
                /* <--- */
                
		/* R2.1.0 20070530 add(kmyang) ---> */
                case 251:
                    if( dValLen >0 && dValLen < MAX_MIN_SIZE-1)
                    {
                        memcpy( &pstRDPkt->stUDRInfo[i-1].szCalledMin[0], &szAttrVal[0], dValLen);
                        pstRDPkt->stUDRInfo[i-1].szCalledMin[dValLen] = 0x00;
                        pstRDPkt->stUDRInfo[i-1].ucCalledMinF = 0x01;
                    }
                    break;
                
				case 252:
                    if( dValLen >0 && dValLen < MAX_MIN_SIZE-1)
                    {
                        memcpy( &pstRDPkt->stUDRInfo[i-1].szCallerMin[0], &szAttrVal[0], dValLen);
                        pstRDPkt->stUDRInfo[i-1].szCallerMin[dValLen] = 0x00;
                        pstRDPkt->stUDRInfo[i-1].ucCallerMinF = 0x01;
                    }
                    break;
                /* <--- */

                default: 
                    break;
                }

                break;

            default:
                break;
            }
        }
	}

	if( dIdx != dBufSize || dTempSize != 0 )
	{
        fprintf(stderr," MISMATCH SIZE\n");
		return -1;
	}

	return dRet;		
}



int ParsingAttr( INT *dType, INT *dVSType, UCHAR *szAttrVal, INT *dValLen, UCHAR *pszBuf, INT *dBufSize )
{
	INT		Length;
	UCHAR	ucLen, ucVSLen;
	UCHAR	ucType, ucVSType;
	UCHAR	Value[128];
	INT		dValue;
	int 	dIdx=0;
	UINT	dVenderID;

	if( *dBufSize <= 3 )
		return -7;

	ucType = pszBuf[dIdx];
	dIdx += 1;
	*dType = ucType;

	ucLen = pszBuf[dIdx];
	dIdx += 1;

	Length = ucLen;
	Length -= 2;

    if( Length <= 0 )
        return -2;

	if( Length > (*dBufSize-2) )
		return -3;

	if( ucType == 26  )
	{
		/*      Vender Specific Type 		*/
		//if( (*dBufSize - dIdx ) < 7 )
			//return -4;

		memcpy( &dVenderID, &pszBuf[dIdx], 4 );
		dIdx += 4;

		ucVSType = pszBuf[dIdx];
		dIdx += 1;

		*dVSType = ucVSType;

		ucVSLen = pszBuf[dIdx];
		dIdx += 1;

		Length = ucVSLen;
		Length -= 2;	

		/*
    	if( Length <= 0 )
        	return -8;
		*/

		if( (*dBufSize - dIdx ) < Length )
			return -5;

		memcpy( &szAttrVal[0], &pszBuf[dIdx], Length );
		szAttrVal[Length] = 0x00;
		dIdx += Length;
		*dValLen = Length;
	}
	else
	{
		/*   Normal Value Attribute 		*/
		if( (*dBufSize - dIdx ) < Length )
			return -6;

		memcpy( &szAttrVal[0], &pszBuf[dIdx], Length );
		szAttrVal[Length] = 0x00;
		dIdx += Length;
		*dValLen = Length;
	}	

	*dBufSize = dIdx;
	return 0; 
} 
