/**********************************************************
                 KTF IPAS Project

   Author   : Park Si Woo
   Section  : Infravalley Develpment
   SCCS ID  : %W%
   Date     : %G%
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


#include <define.h>
#include <ipam_define.h>
#include <ipam_svc.h>
#include <ipam_sessif_new.h>

#include <ipam_names.h>

#include <MD5/md5.h>
#include <ipam_error.h>
#include <ipam_sesssvc.h>

/**B.1*  Definition of New Constants *********************/
#define VENDOR_3GPP2 5535

/**B.2*  Definition of New Type  **************************/


/**C.1*  Declaration of Variables  ************************/



extern int errno;

/*
extern ULONG CVT_ULONG( ULONG value );
extern LONG CVT_LONG( LONG value );
extern INT CVT_INT( INT value );
extern SHORT CVT_SHORT( SHORT value );
extern INT64 CVT_INT64( INT64 value );
*/

int PrnRadiusAttr( int dType, int dVSType, int dValLen, unsigned char *szAttr );
int ParsingRadiusAttr( INT *dType, INT *dVSType, UINT *uiVendorID, UCHAR *szAttrVal, INT *dValLen, UCHAR *pszBuf, INT *dBufSize );

int ParsingRadius( UCHAR *pszBuf, INT dBufSize, unsigned int ulFromIP, pst_ACCInfo pstRDPkt , int *dADR , int dNASType)
{
	int 	dType, dVSType, dValLen; 
	UCHAR	ucTemp;
	short   sTemp;
	short	sWSTypeF=0;
	short	sCSTypeF=0;
	short	sSubsCapaF=0;
	UCHAR	szAttrVal[128];
	int 	dIdx=0;
	int		dTempSize=0;
	int 	 dTempOrgSize=0;
	int		dRet;
	int     i;
	int    	dTempVal=0;
	int		dMandCnt=0;
	int    	dBothChk=0;
	int		dBothStopChk=0;
	int		dPDSNChk=0;
	int		dPDSNStart=0;
	int		dPDSNStop=0;
	int 	dWSTypeChk=0;
	UINT	uiVendorTemp=0;		
	UINT	uiVendorID=0;		
	char	szTempMIN[17];
	

	pst_RADIUS_HEAD  pstRds = (st_RADIUS_HEAD*)&pszBuf[0];

	memset( pstRDPkt, 0x00, sizeof(st_ACCInfo) );

	pstRds->usLength = CVT_SHORT(pstRds->usLength);

	dTempOrgSize = pstRds->usLength - sizeof(st_RADIUS_HEAD);
	dTempSize = dTempOrgSize;

	pstRDPkt->ucCode = pstRds->ucCode;
	pstRDPkt->ucID = pstRds->ucID;
	pstRDPkt->uiRADIUSLen = pstRds->usLength;
	memcpy( &pstRDPkt->szAuthen[0], &pstRds->szAuth[0], 16 );

	if( pstRds->usLength != dBufSize )
	{
		fprintf(stderr,"ERROR: header and socket message length mismatch(%d/%d)!!!\n",pstRds->usLength,dBufSize);
		*dADR = -20;
		return -1;
	}

	dIdx += sizeof(st_RADIUS_HEAD) ;

	while(dTempSize > 4)
	{
		dType = 0;
		dVSType = 0;

		dRet = ParsingRadiusAttr( &dType, &dVSType, &uiVendorID, &szAttrVal[0], &dValLen, &pszBuf[dIdx], &dTempSize ); 
		if( dRet < 0 )
		{
			PrnRadiusAttr( dType, dVSType, dValLen, &szAttrVal[0] );
			break;
		}

		PrnRadiusAttr( dType, dVSType, dValLen, &szAttrVal[0] );

		if( uiVendorID == 10415 )
			uiVendorTemp = 10415;

		dIdx += dTempSize;
		dTempOrgSize -= dTempSize;
		dTempSize = dTempOrgSize;

		switch( dType )
		{
			case 26:
				switch( dVSType )
				{
				case 1:
					// 3GPP-IMSI 
                    if( dValLen > 0 && dValLen < MAX_MIN_SIZE)
                    {
                        sTemp = strlen((char *)&pstRDPkt->szMIN[0]);
                        memcpy( &pstRDPkt->szMSISDN[0], &pstRDPkt->szMIN[0], sTemp);
                        pstRDPkt->szMSISDN[sTemp] = 0x00;
                        pstRDPkt->ucMSISDNF = 1;

                        memcpy( &pstRDPkt->szMIN[0], &szAttrVal[0], dValLen );
                        pstRDPkt->szMIN[dValLen] = 0x00;
                        pstRDPkt->ucCallStatIDF = 1;
                    }
                    else
                    {
                        *dADR = ERROR_INVALID_PARAMETER_VALUE;
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
                        return -1;
                    }

                    dPDSNChk++;
                    break;

				case 2:	//3GPP-Charging-id
					if( dValLen == sizeof(INT))
                    {
                        memcpy( &pstRDPkt->d3GChargeID, &szAttrVal[0], dValLen );
                        pstRDPkt->d3GChargeID = CVT_INT(pstRDPkt->d3GChargeID);
                        pstRDPkt->uc3GChargeIDF = 0x01;
                        dPDSNChk++;
					}
					else
					{
                        *dADR = ERROR_INVALID_PARAMETER_VALUE;
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
                        return -1;
					}

					break;

                case 3:
                    // 3GPP-PDP Type
                    if( dValLen == sizeof(INT))
                    {
                        memcpy( &pstRDPkt->d3GPDPType, &szAttrVal[0], dValLen );
                        pstRDPkt->d3GPDPType = CVT_INT(pstRDPkt->d3GPDPType);
                        pstRDPkt->uc3GPDPTypeF = 0x01;
                        dPDSNChk++;
                    }
                    else
                    {
                        *dADR = ERROR_INVALID_PARAMETER_VALUE;
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
                        return -1;
                    }

                    break;

                case 4: // 3GPP-CG-Address

                    if( dValLen == sizeof(INT))
                    {
                        memcpy( &pstRDPkt->ui3GCGAddr, &szAttrVal[0], dValLen );
                        pstRDPkt->uc3GCGAddrF = 0x01;
                        dPDSNChk++;
                    }
                    else
                    {
                        *dADR = ERROR_INVALID_PARAMETER_VALUE;
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
                        return -1;
                    }

                    break;

                case 5:
                    // 3GPP-GPRS-Negotiated-QoS-Profile
                    if( dValLen > 0 && dValLen < MAX_3GNEGOQOS_SIZE)
                    {
                        memcpy( &pstRDPkt->sz3GGPRSNegoQos[0], &szAttrVal[0], dValLen );
                        pstRDPkt->sz3GGPRSNegoQos[dValLen] = 0x00;
                        pstRDPkt->uc3GGPRSNegoQosF = 1;
                        dPDSNChk++;
                    }
                    else
                    {
                        *dADR = ERROR_INVALID_PARAMETER_VALUE;
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
                        return -1;
                    }

                    break;

                case 6:
                    // 3GPP-SGSN-Address IP ADDRESS
                    if( dValLen == sizeof(UINT) )
                    {
                        memcpy( &pstRDPkt->uiPCFIP, &szAttrVal[0], dValLen );
                        pstRDPkt->ucPCFIPF = 0x01;
                        dPDSNChk++;
                    }
                    else
                    {
                        *dADR = ERROR_INVALID_PARAMETER_VALUE;
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
                        return -1;
                    }

                    break;

				case 7:

					// HA IP ADDRESS, HOME Agent Attribute 
					// M Condition : PDSN [ Start, Stop ] 

					// 3GPP-GGSN-Address

					if(dNASType == DEF_NAS_GGSN) {
						if(dValLen == sizeof(UINT)) {
							memcpy(&pstRDPkt->ui3GGGSNAddr, &szAttrVal[0], dValLen);
							pstRDPkt->uc3GGGSNAddrF = 0x01;
							dPDSNChk++;
						} else {
							*dADR = ERROR_INVALID_PARAMETER_VALUE;
							fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
							return -1;
						}
					} else {
	
						if( dValLen == sizeof(UINT) ) {
							memcpy( &pstRDPkt->uiHAIP, &szAttrVal[0], dValLen );
							pstRDPkt->ucHAIPF = 0x01;
					
							dPDSNChk++;
						} else {
							
							*dADR = ERROR_INVALID_PARAMETER_VALUE;
							fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
							return -1;
						}
					}

					break;
				case 8:
					// 3GPP-IMSI-MCC-MNC
					if( dValLen > 0 && dValLen <= MAX_3GMCCMNC_SIZE) {
						memcpy( &pstRDPkt->sz3GIMSIMCCMNC[0], &szAttrVal[0], dValLen );
						pstRDPkt->sz3GIMSIMCCMNC[dValLen] = 0x00;
						pstRDPkt->uc3GIMSIMCCMNCF = 0x01;
						dPDSNChk++;
					} else {
						*dADR = ERROR_INVALID_PARAMETER_VALUE;
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
						return -1;
					}
					break;

				case 9:

					// PCF IP ADDRESS , Serving PCF 
					// M Condition : PDSN [ Start, Stop ]

					// 3GPP-GGSN-MCC-MNC

					if(dNASType == DEF_NAS_GGSN) {
						if( dValLen > 0 && dValLen < MAX_3GMCCMNC_SIZE) {
							memcpy( &pstRDPkt->sz3GGGSNMCCMNC[0], &szAttrVal[0], dValLen );
							pstRDPkt->sz3GGGSNMCCMNC[dValLen] = 0x00;
							pstRDPkt->uc3GGGSNMCCMNCF = 0x01;
							dPDSNChk++;

						} else {
							*dADR = ERROR_INVALID_PARAMETER_VALUE;
							fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
							return -1;
						}

					} else {
						if( dValLen == sizeof(UINT) ) {
							memcpy( &pstRDPkt->uiPCFIP, &szAttrVal[0], dValLen );
							pstRDPkt->ucPCFIPF = 0x01;

							dPDSNChk++;
						} else {
							fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
							*dADR = ERROR_INVALID_PARAMETER_VALUE;
							return -1;
						}
					}

					break;

				case 10:

					// BS/MSC Address (PPP Start) 
					// M Condition : PDSN [ Start,Stop ]

					// 3GPP-NSAPI
					if(dNASType == DEF_NAS_GGSN) {
						if( dValLen == 1 ) {
							pstRDPkt->uc3GNSAPI = szAttrVal[0];
							pstRDPkt->uc3GNSAPIF = 0x01;
							dPDSNChk++;
						} else if( dValLen == 2 ) {
							memcpy(&sTemp, &szAttrVal[0], 2 );
							sTemp = CVT_SHORT(sTemp);
							pstRDPkt->uc3GNSAPI = sTemp;
							pstRDPkt->uc3GNSAPIF = 0x01;
							dPDSNChk++;
						} else if( dValLen == 4 ) {
							memcpy( &dTempVal,  &szAttrVal[0], dValLen );
							dTempVal = CVT_INT(dTempVal);
							pstRDPkt->uc3GNSAPI = dTempVal;
							pstRDPkt->uc3GNSAPIF = 0x01;
							dPDSNChk++;
						} else {
							*dADR = ERROR_INVALID_PARAMETER_VALUE;
							fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
							return -1;
						}

					} else {
						if( dValLen > 0 && dValLen <= MAX_BSMSC_SIZE ) 
						{
							memcpy( &pstRDPkt->szPPPStart[0], &szAttrVal[0], dValLen );
							pstRDPkt->szPPPStart[dValLen] = 0x00;
							pstRDPkt->ucPPPSLen = dValLen;
							pstRDPkt->ucPPPSetupF = 0x01;

							dPDSNChk++;
						}
						else
						{
							*dADR = ERROR_INVALID_PARAMETER_VALUE;
							fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
							return -1;
						}
					}

					break;
				case 11:
					//3GPP-Session-Stop-Indicator
					if( dValLen == 1 ) {
						pstRDPkt->uc3GSessStopIndi = szAttrVal[0];
						pstRDPkt->uc3GSessStopIndiF = 0x01;
						dPDSNChk++;
					} else if( dValLen == 2 ) {
						memcpy(&sTemp, &szAttrVal[0], 2 );
						sTemp = CVT_SHORT(sTemp);
						pstRDPkt->uc3GSessStopIndi = sTemp;
						pstRDPkt->uc3GSessStopIndiF = 0x01;
						dPDSNChk++;
					} else if( dValLen == 4 ) {
						memcpy( &dTempVal,  &szAttrVal[0], dValLen );
						dTempVal = CVT_INT(dTempVal);
						pstRDPkt->uc3GSessStopIndi = dTempVal;
						pstRDPkt->uc3GSessStopIndiF = 0x01;
						dPDSNChk++;
					} else {
						*dADR = ERROR_INVALID_PARAMETER_VALUE;
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
						return -1;
					}

					break;
				case 12:

				 	//	Forward MUX
					// M Condition : PDSN [Start,Stop]

					// 3GPP-Selection-Mode
					if(dNASType == DEF_NAS_GGSN) {
						if( dValLen == 1 ) {
							pstRDPkt->uc3GSelectMode = szAttrVal[0];
							pstRDPkt->uc3GSelectModeF = 0x01;
							dPDSNChk++;
						} else {
							*dADR = ERROR_INVALID_PARAMETER_VALUE;
							fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
							return -1;
						}
					} else {	
						if( dValLen == sizeof(INT) )
						{
							memcpy( &pstRDPkt->dFMUX, &szAttrVal[0], dValLen );
							pstRDPkt->dFMUX = CVT_INT( pstRDPkt->dFMUX );
							pstRDPkt->ucFMUXF = 0x01;

							dPDSNChk++;
						}
						else
						{
							*dADR = ERROR_INVALID_PARAMETER_VALUE;
							fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
							return -1;
						}
					}

					break;

				case 13:

					// Reverse MUX 
					// M Condition : PDSN [ Start,Stop ] 

					//3GPP-Charging-Characteristics
					
					if(dNASType == DEF_NAS_GGSN) {

						if( dValLen > 0 && dValLen < MAX_3GCHARGE_SIZE) {

							memcpy( &pstRDPkt->sz3GChargChar[0], &szAttrVal[0], dValLen );
							pstRDPkt->sz3GChargChar[dValLen] = 0x00;
							pstRDPkt->uc3GChargCharF = 0x01;
							dPDSNChk++;
						} else {
							*dADR = ERROR_INVALID_PARAMETER_VALUE;
							fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
							return -1;
						}

					} else {
						if( dValLen == sizeof(INT) ) {
							memcpy( &pstRDPkt->dRMUX, &szAttrVal[0], dValLen );
							pstRDPkt->dRMUX = CVT_INT( pstRDPkt->dRMUX );
							pstRDPkt->ucRMUXF = 0x01;

							dPDSNChk++;

						} else {
							*dADR = ERROR_INVALID_PARAMETER_VALUE;
							fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
							return -1;
						}
					}

					break;

				case 16:

					// Service Option 
					// M Condition : PDSN [Start, Stop] , IWF [Start, Stop]

					if( dValLen == sizeof(INT) )
					{
						memcpy( &pstRDPkt->dSvcOption, &szAttrVal[0], dValLen );
						pstRDPkt->dSvcOption = CVT_INT(pstRDPkt->dSvcOption);
						pstRDPkt->ucSvcOptF = 0x01;

						dBothChk++;
					}
					else
					{
						*dADR = ERROR_INVALID_PARAMETER_VALUE;
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
						return -1;
					}

					break;
				/* R2.3.3 add 2005.1005 (lander) ---> */
				case 18:
					// SGSN MCC-MNC
					if (dValLen > 0 && dValLen < MAX_SGSN_MCCMNC_SIZE)
					{
						memcpy (pstRDPkt->szSGSNMccMnc, &szAttrVal[0], dValLen);
						pstRDPkt->ucSGSNMccMncF = 0x01;
					}
					else
					{
						*dADR = ERROR_INVALID_PARAMETER_VALUE;
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
						return -1;
					}
					break;
				/* <--- */

				case 22:

					// IP Technology 
					// M Condition : PDSN [Start, Stop]
	
					if( dValLen == sizeof(INT) )
					{
						memcpy( &pstRDPkt->dIPTech, &szAttrVal[0], dValLen );
						pstRDPkt->dIPTech = CVT_INT( pstRDPkt->dIPTech );

						pstRDPkt->ucIPTechF = 0x01;

						dPDSNChk++;
					}
					else
					{
						*dADR = ERROR_INVALID_PARAMETER_VALUE;
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
						return -1;
					}

					break;


				case 24:
					// Release Indicator 
					// M Condition : PDSN [Stop] , IWF [Stop]
					
					if( dValLen == sizeof(INT) )	
					{ 
						memcpy( &pstRDPkt->dRelIndi, &szAttrVal[0], dValLen );
						pstRDPkt->dRelIndi = CVT_INT( pstRDPkt->dRelIndi );
						pstRDPkt->ucReleaseIndiF = 0x01;

						dBothStopChk++;
					}
					else
					{
						*dADR = ERROR_INVALID_PARAMETER_VALUE;
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
						return -1;
					}

					break;

				case 30:

					// Number Active 
					// M Condition : PDSN [Stop], IWF [Stop]
					
					if( dValLen == sizeof(INT) )
					{
						memcpy( &pstRDPkt->dNumActive, &szAttrVal[0], dValLen );
						pstRDPkt->dNumActive = CVT_INT( pstRDPkt->dNumActive );
						pstRDPkt->ucNumAF = 0x01;
			
						dBothStopChk++;

					}
					else
					{
						*dADR = ERROR_INVALID_PARAMETER_VALUE;
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
						return -1;
					}
					
					break;

				case 36:

					// IP Qos 
					// M Condition : PDSN [Start,Stop]

					if( dValLen == sizeof(INT) )
					{
						memcpy( &pstRDPkt->dIPQoS, &szAttrVal[0], dValLen );
						pstRDPkt->dIPQoS = CVT_INT( pstRDPkt->dIPQoS );
						pstRDPkt->ucIPQoSF = 0x01;

						dPDSNChk++;

					}
					else
					{
						*dADR = ERROR_INVALID_PARAMETER_VALUE;
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
						return -1;
					}

					break;

				case 39:

					// Air Qos 
					// M Condition : PDSN [Start, Stop]

					if( dValLen == sizeof(INT) )
					{
						memcpy( &pstRDPkt->dAirQoS, &szAttrVal[0], dValLen );
						pstRDPkt->dAirQoS = CVT_INT( pstRDPkt->dAirQoS );

						pstRDPkt->ucAirQoSF = 0x01;

						dPDSNChk++;

					}
					else
					{
						*dADR = ERROR_INVALID_PARAMETER_VALUE;
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
						return -1;
					}

					break;


				case 44:
					// Correlation ID 
					// M Condition : PDSN [Start, Stop], IWF [Start, Stop]

					if( dValLen == sizeof(INT64) )
					{
						memcpy(&pstRDPkt->llCorrelID, &szAttrVal[0], dValLen );
						pstRDPkt->llCorrelID = CVT_INT64(pstRDPkt->llCorrelID);
						pstRDPkt->ucCorrelationIDF = 0x01;

						dBothChk++;
					}
					else
					{
						*dADR = ERROR_INVALID_PARAMETER_VALUE;
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
						return -1;
					}

					break;
				/* R2.4.0 2006.0202(challa) add -----> */
				case 48:
					// Session Continue
					if(dValLen == sizeof(char))
					{
						memcpy(&ucTemp, &szAttrVal[0], dValLen);
						pstRDPkt->uiSessionContinue 	= ucTemp;
						pstRDPkt->ucSessionContinueF 	= 0x01;
					}
					else if (dValLen == sizeof(SHORT))
					{
						memcpy(&sTemp, &szAttrVal[0], dValLen);
						pstRDPkt->uiSessionContinue 	= CVT_SHORT(sTemp);
						pstRDPkt->ucSessionContinueF 	= 0x01;
					}
					else if (dValLen == sizeof(INT))	 
					{
						memcpy (&dTempVal, &szAttrVal[0], dValLen);
						dTempVal 						= CVT_INT (dTempVal);
						pstRDPkt->uiSessionContinue 	= dTempVal;
						pstRDPkt->ucSessionContinueF 	= 0x01;
					}
					else
					{
					 	fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
                        *dADR = ERROR_INVALID_PARAMETER_VALUE;
					}

					break;

				case 51:
					// Beginning Session 
					if(dValLen == sizeof(char))
					{
						memcpy(&ucTemp, &szAttrVal[0], dValLen);
						pstRDPkt->uiBeginningSession	= ucTemp;
						pstRDPkt->ucBeginningSessionF 	= 0x01;
					}
					else if (dValLen == sizeof(SHORT))
					{
						memcpy(&sTemp, &szAttrVal[0], dValLen);
						pstRDPkt->uiBeginningSession 	= CVT_SHORT(sTemp);
						pstRDPkt->ucBeginningSessionF 	= 0x01;
					}
					else if (dValLen == sizeof(INT))	 
					{
						memcpy (&dTempVal, &szAttrVal[0], dValLen);
						dTempVal 						= CVT_INT (dTempVal);
						pstRDPkt->uiBeginningSession	= dTempVal;
						pstRDPkt->ucBeginningSessionF 	= 0x01;
					}
					else
					{
					 	fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
                        *dADR = ERROR_INVALID_PARAMETER_VALUE;
					}
				/* <------  */

				/* R2.4.0 add 2006.0124 (lander) ---> 
				case 48:
					// Session Continue
					if(dValLen == sizeof(char))
					{
						memcpy(&ucTemp, &szAttrVal[0], dValLen);
						pstRDPkt->uiSessionContinue = ucTemp;
						pstRDPkt->ucSessionContinueF = 0x01;
					}
					else if (dValLen == sizeof(SHORT))
					{
						memcpy(&sTemp, &szAttrVal[0], dValLen);
						pstRDPkt->uiSessionContinue = CVT_SHORT(sTemp);
						pstRDPkt->ucSessionContinueF = 0x01;
					}
					else 
					{
						memcpy(&pstRDPkt->uiSessionContinue, &szAttrVal[0], dValLen);
						pstRDPkt->uiSessionContinue = CVT_INT(pstRDPkt->uiSessionContinue);
						pstRDPkt->ucSessionContinueF = 0x01;
					}
					break;
				case 51:
					// Beginning Session
					if (dValLen == sizeof(char))
					{
						memcpy(&ucTemp, &szAttrVal[0], dValLen);
						pstRDPkt->uiBeginningSession = ucTemp;
						pstRDPkt->ucBeginningSessionF = 0x01;
					}
					else if (dValLen == sizeof(SHORT))
					{
						memcpy(&sTemp, &szAttrVal[0], dValLen);
						pstRDPkt->uiBeginningSession = CVT_SHORT(sTemp);
						pstRDPkt->ucBeginningSessionF = 0x01;
					}
					else
					{
						memcpy(&pstRDPkt->uiBeginningSession, &szAttrVal[0], dValLen);
						pstRDPkt->uiBeginningSession = CVT_SHORT(sTemp);
						pstRDPkt->uiBeginningSession = CVT_INT(pstRDPkt->uiBeginningSession);

						pstRDPkt->ucBeginningSessionF = 0x01;
					}
					break;
				-----> */

				case 52:

					// ESN 
					// M Condition : PDSN [Start, Stop]

					if( dValLen > 0 && dValLen <= MAX_ESN_SIZE )
					{
						memcpy( &pstRDPkt->szESN, &szAttrVal[0], dValLen );
						pstRDPkt->ucESNF = 0x01;

						dPDSNChk++;
					}
					else
					{
						*dADR = ERROR_INVALID_PARAMETER_VALUE;
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
						return -1;
					}

					break;
				/* R2.3.0 Add 2004.1222 (lander) ---> */
				case 64:
					if (dValLen == sizeof(UINT))
					{
						memcpy (&pstRDPkt->uiCoA, &szAttrVal[0], dValLen);
						pstRDPkt->ucCoAF = 0x01;
					}
					else
					{
						*dADR = ERROR_INVALID_PARAMETER_VALUE;	
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
					}

					break;
				/* <--- */

				/* 20040329,poopee */
				case 78:    // Always-On
					if (dValLen == sizeof(INT))
					{
						memcpy(&pstRDPkt->dAlwaysOn,&szAttrVal[0],dValLen);
						pstRDPkt->ucAlwaysOnF = 0x01;
					}
					else
					{
						*dADR = ERROR_INVALID_PARAMETER_VALUE;
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
						return -1;
					}
					break;

				/* R2.3.0 Add 2004.1222 (lander) ---> */
                case 161:   /* IPSec Mode */
                    /* 0:None, 1:AH, 2:ESP, 3:AH+ESP */
                    if (uiVendorID != VENDOR_3GPP2)
                    {
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
                        *dADR = ERROR_INVALID_PARAMETER_VALUE;
                    }

                    if (dValLen == 1)
                    {
                        pstRDPkt->ucIpSecMode = szAttrVal[0];
                        pstRDPkt->ucIpSecModeF = 0x01;
                    }
                    else if (dValLen == 2)
                    {
                        memcpy (&sTemp, &szAttrVal[0], 2);
                        sTemp = CVT_SHORT(sTemp);
                        pstRDPkt->ucIpSecMode = sTemp;
                        pstRDPkt->ucIpSecModeF = 0x01;
                    }
                    else if (dValLen == sizeof(INT))
                    {
                        memcpy (&dTempVal, &szAttrVal[0], dValLen);
                        dTempVal = CVT_INT (dTempVal);
                        pstRDPkt->ucIpSecMode = dTempVal;
                        pstRDPkt->ucIpSecModeF = 0x01;
                    }
                    else
                    {
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
                        *dADR = ERROR_INVALID_PARAMETER_VALUE;
                    }
                    break;

                case 164:	// ESSID
                    if (dValLen > 0 && dValLen <= MAX_ESSID_SIZE)
                    {
                        memcpy (&pstRDPkt->szESSID[0], &szAttrVal[0], dValLen);
                        pstRDPkt->ucESSIDF = 0x01;
                    }
                    else
                    {
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
                        *dADR = ERROR_INVALID_PARAMETER_VALUE;
                    }
                    break;

                case 165:   /* MS Interface Indicator*/
                    /* 0:None, 1: WLAN, 2:Bluetooth, 3:Wibro */
                    if (uiVendorID != VENDOR_3GPP2)
                    {
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
                        *dADR = ERROR_INVALID_PARAMETER_VALUE;
                    }

                    if (dValLen == 1)
                    {
                        pstRDPkt->ucMsInterfaceInd = szAttrVal[0];
                        pstRDPkt->ucMsInterfaceIndF = 0x01;
                    }
                    else if (dValLen == sizeof(SHORT))
                    {
                        memcpy (&sTemp, &szAttrVal[0], sizeof(SHORT));
                        sTemp = CVT_SHORT(sTemp);
                        pstRDPkt->ucMsInterfaceInd = sTemp;
                        pstRDPkt->ucMsInterfaceIndF = 0x01;
                    }
                    else if (dValLen == sizeof(INT))
                    {
                        memcpy (&dTempVal, &szAttrVal[0], sizeof(INT));
                        dTempVal = CVT_INT (dTempVal);
                        pstRDPkt->ucMsInterfaceInd = dTempVal;
                        pstRDPkt->ucMsInterfaceIndF = 0x01;
                    }
                    else
                    {
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
                        *dADR = ERROR_INVALID_PARAMETER_VALUE;
                    }
                    break;
				/* <--- */

				case 180:

					//  NAS-TYPE 
					// M Condition : PDSN [Start, Stop], IWF [Start, Stop]

					if( dValLen == 1 )
					{
						pstRDPkt->ucNASType = szAttrVal[0];
					}
					else if( dValLen == 2 )
					{
						memcpy( &sTemp, &szAttrVal[0], 2 );
						sTemp = CVT_SHORT( sTemp );

						pstRDPkt->ucNASType = sTemp;

					}
					else if( dValLen == sizeof(INT) )
					{
						memcpy( &dTempVal, &szAttrVal[0], dValLen );
						dTempVal = CVT_INT(dTempVal);
	
						pstRDPkt->ucNASType = dTempVal;
					}
					else
					{
						*dADR = ERROR_INVALID_PARAMETER_VALUE;
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
						return -1;
					}


					if( pstRDPkt->ucNASType > 11 )
					{
						*dADR = ERROR_INVALID_PARAMETER_VALUE;
						fprintf(stderr,"ERROR: invalid parameter value(%d/%d=%d)!!!\n",dType,dVSType,pstRDPkt->ucNASType);
						return -1;
					}

					dBothChk++;
					pstRDPkt->ucNASTypeF = 0x01;


					break;

				case 193:

					// Packet-Session Start Time
					// M Condition : PDSN [Start, Stop], IWF [Start, Stop]

					if( dValLen == sizeof(time_t) )
					{	
						memcpy( &pstRDPkt->dPktSessSTime, &szAttrVal[0], dValLen );
						pstRDPkt->dPktSessSTime = CVT_INT( pstRDPkt->dPktSessSTime );
						pstRDPkt->ucPktSessStartTimeF = 0x01;
						dBothChk++;

					}
					else
					{
						*dADR = ERROR_INVALID_PARAMETER_VALUE;
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
						return -1;
					}

					break;

				case 194:

					// PPP Start Time 
					// M Condition : PDSN [Stop]
					
					if( dValLen == sizeof(time_t) )
					{
						memcpy( &pstRDPkt->dPPPSTime, &szAttrVal[0], dValLen );
						pstRDPkt->dPPPSTime = CVT_INT( pstRDPkt->dPPPSTime );

						pstRDPkt->ucPPPSTimeF = 0x01;

						dPDSNStop++;

					}
					else
					{
						*dADR = ERROR_INVALID_PARAMETER_VALUE;
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
						return -1;
					}

					break;

				case 196:

					// Price Plan 
					// M Condition : PDSN [ WSType =Áö´É¸Á, Start,Stop ]	
					
					if( dValLen == 1 )
					{
						ucTemp = szAttrVal[0];
						pstRDPkt->ucPricePlan = ucTemp ;
						pstRDPkt->ucPlanF = 0x01;
					}
					else if( dValLen == 2 )
					{
						memcpy( &sTemp, &szAttrVal[0], 2 );
						sTemp = CVT_SHORT(sTemp);
						pstRDPkt->ucPricePlan = sTemp ;
						pstRDPkt->ucPlanF = 0x01;
					}
					else if( dValLen == 4 )
					{
						memcpy( &dTempVal, &szAttrVal[0], dValLen );
						dTempVal = CVT_INT(dTempVal);

						pstRDPkt->ucPricePlan = dTempVal;
						pstRDPkt->ucPlanF = 0x01;
					}
					else
					{
						*dADR = ERROR_INVALID_PARAMETER_VALUE;
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
						return -1;
					}


					break;


				case 197:

					// WSTYPE 
					// M Condition : PDSN [Start,Stop], IWF [Start, Stop]

					if( dValLen == 1 )
					{
						ucTemp = szAttrVal[0];
						pstRDPkt->ucWSType = ucTemp ;
					}
					else if( dValLen == 2 )
					{
						memcpy( &sTemp, &szAttrVal[0], 2 );
						sTemp = CVT_SHORT(sTemp);
						pstRDPkt->ucWSType = sTemp ;
					}
					else if( dValLen == 4 )
					{
						memcpy( &dTempVal, &szAttrVal[0], dValLen );
						dTempVal = CVT_INT(dTempVal );
						pstRDPkt->ucWSType = dTempVal;
					}
					else
					{
						*dADR = ERROR_INVALID_PARAMETER_VALUE;
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
						return -1;
					}

					dBothChk++;
					sWSTypeF = 1;
					pstRDPkt->ucWSTypeF = 0x01;

					break;

			
				case 202:

					// PPP Negotiation Duration 
					// M Condition : PDSN [Stop] , IWF [Stop]

					if( dValLen == 1 )
					{
						pstRDPkt->ucPPPNego = szAttrVal[0];
					}
					else if( dValLen == 2 )
					{
						memcpy(&sTemp, &szAttrVal[0], 2 );
						sTemp = CVT_SHORT(sTemp);
						pstRDPkt->ucPPPNego = sTemp;
					}
					else if( dValLen == 4 )
					{
						memcpy( &dTempVal,  &szAttrVal[0], dValLen );
						dTempVal = CVT_INT(dTempVal);
						pstRDPkt->ucPPPNego = dTempVal;
					}
					else
					{
						*dADR = ERROR_INVALID_PARAMETER_VALUE;
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
						return -1;
					}

					dBothStopChk++;
					pstRDPkt->ucPPPNegoDurationF = 0x01;

					break;
			
				case 203:
					// BS/MSC Address (Session Start) 
					// M Condition : PDSN [Start, Stop]
			
					if( dValLen > 0 && dValLen <= MAX_BSMSC_SIZE )
					{
						memcpy( &pstRDPkt->szSessStart[0],  &szAttrVal[0], dValLen );
						pstRDPkt->szSessStart[dValLen] = 0x00;
						pstRDPkt->ucSessSLen = dValLen;	
						pstRDPkt->ucSessionStartF = 0x01;
						
						dPDSNChk++;

					}
					else
					{
						*dADR = ERROR_INVALID_PARAMETER_VALUE;
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
						return -1;
					}

					break;
			
				case 204:

					// BS/MSC Address (Session Stop) 
					// M Condition : PDSN [Start, Stop]

					if( dValLen > 0 && dValLen <= MAX_BSMSC_SIZE )
					{
						memcpy( &pstRDPkt->szSessStop[0],  &szAttrVal[0], dValLen );
						pstRDPkt->szSessStop[dValLen] = 0x00;
						pstRDPkt->ucSessTLen = dValLen;
						pstRDPkt->ucSessionStopF = 0x01;

						dPDSNChk++;

					}
					else
					{
						*dADR = ERROR_INVALID_PARAMETER_VALUE;
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
						return -1;
					}

					break;

				case 205:
					// Win_Call_ID
					// M Condition : PDSN [WSType=Áö´É¸Á, Start, Stop]
					
					if( dValLen == sizeof(INT) )
					{
						memcpy( &pstRDPkt->dWinID, &szAttrVal[0], dValLen );
						pstRDPkt->dWinID = CVT_INT( pstRDPkt->dWinID );	
						if( pstRDPkt->dWinID == 0 )
						{
							*dADR = ERROR_INVALID_PARAMETER_VALUE;
							fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
							return -1;
						}

						pstRDPkt->ucWinCallIDF = 0x01;
						dWSTypeChk++;
					}
					else
					{
						*dADR = ERROR_INVALID_PARAMETER_VALUE;
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
						return -1;
					}
	
					break;	
			
				case 211:

					// ADR 
					// M Condition : Qud, AAA response

					if( dValLen == 1 )
					{
						pstRDPkt->dADR = szAttrVal[0];
					}
					else if( dValLen == 2 )
					{
						memcpy( &sTemp, &szAttrVal[0], 2 );
						sTemp = CVT_SHORT(sTemp);
						pstRDPkt->dADR = sTemp;
					}
					else if( dValLen == 4 )
					{
						memcpy( &pstRDPkt->dADR, &szAttrVal[0], dValLen );
						pstRDPkt->dADR = CVT_INT(pstRDPkt->dADR) ;
					}
					else
					{
						*dADR = ERROR_INVALID_PARAMETER_VALUE;
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
						return -1;
					}

					pstRDPkt->ucADRF = 0x01;

					break;

				/* 20040329,poopee */
				case 213:	// AUTH
					if (dValLen == 1)
						pstRDPkt->ucAuth = szAttrVal[0];
					else if (dValLen == 2)
					{
						memcpy(&sTemp,&szAttrVal[0],dValLen);
						sTemp = CVT_SHORT(sTemp);
						pstRDPkt->ucAuth = sTemp;
					}
					else if (dValLen == 4)
					{
						memcpy(&sTemp,&szAttrVal[0],dValLen);
						sTemp = CVT_INT(sTemp);
						pstRDPkt->ucAuth = sTemp;
					}
					else
					{	
						*dADR = ERROR_INVALID_PARAMETER_VALUE;
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
						return -1;
					}
	
					pstRDPkt->ucAuthF = 0x01;
					break;

				case 216:
					// WIN_SVC Option
					// M Condition : PDSN [WSType=Áö´É¸Á, Start, Stop]
					
					if( dValLen > 0 && dValLen <= MAX_WINSVC_SIZE )
					{
						memcpy( &pstRDPkt->szWinSvc[0], &szAttrVal[0], dValLen );
						pstRDPkt->ucWINSVCF = 0x01;

						dWSTypeChk++;
					}
					else
					{
						*dADR = ERROR_INVALID_PARAMETER_VALUE;
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
						return -1;
					}

					break;

				case 218:

					// Calling Subscriber Type 
					// M Condition : PDSN [Start, Stop], IWF [Start, Stop]
				
					if( dValLen == 1 )
					{
						pstRDPkt->ucCallSubType = szAttrVal[0];

					}
					else if( dValLen == 2 )
					{
						memcpy( &sTemp, &szAttrVal[0], 2 );
						sTemp = CVT_SHORT( sTemp );
						pstRDPkt->ucCallSubType = sTemp ;
					}
					else if( dValLen == 4 )
					{
						memcpy( &dTempVal, &szAttrVal[0], dValLen );
						dTempVal = CVT_INT(dTempVal);
						pstRDPkt->ucCallSubType = dTempVal;
					}
					else
					{
						*dADR = ERROR_INVALID_PARAMETER_VALUE;
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
						return -1;
					}

					dBothChk++;
					sCSTypeF = 1;	
					pstRDPkt->ucCallingSubsTypeF = 0x01;

					break;

				case 219:

					// Subscriber[Terminal] Capaility 
					// M Condition : PDSN [Start, Stop], IWF [Start, Stop]

					if( dValLen == 1 )
					{
						pstRDPkt->ucSubsCapa = szAttrVal[0];

					}
					else if( dValLen == 2 )
					{
						memcpy( &sTemp, &szAttrVal[0], 2 );
						sTemp = CVT_SHORT(sTemp);
						pstRDPkt->ucSubsCapa = sTemp;

					}
					else if( dValLen == 4 )
					{
						memcpy( &dTempVal, &szAttrVal[0], dValLen );
						dTempVal = CVT_INT(dTempVal) ;
						pstRDPkt->ucSubsCapa = dTempVal;
					}
                    else
                    {
                        *dADR = ERROR_INVALID_PARAMETER_VALUE;
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
                        return -1;
                    }

					sSubsCapaF = 1;
					pstRDPkt->ucTerminalCapaF = 0x01;

					dBothChk++;

					break;

				/* 20040329,poopee */
				case 220:	// Mobile-Type
					// 1: SIP, 2: MIPS, 3: MIPD	
					if (dValLen == 1)
						pstRDPkt->ucMobileType == szAttrVal[0];
					else if (dValLen == 2)
					{
						memcpy(&sTemp,&szAttrVal[0],dValLen);
						sTemp = CVT_SHORT(sTemp);
						pstRDPkt->ucMobileType = sTemp;
					}	
					else if (dValLen == 4)
					{
						memcpy(&sTemp,&szAttrVal[0],dValLen);
						sTemp = CVT_INT(sTemp);
						pstRDPkt->ucMobileType = sTemp;
					}	
                    else
                    {
                        *dADR = ERROR_INVALID_PARAMETER_VALUE;
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
                        return -1;
					}

					pstRDPkt->ucMobileTypeF = 0x01;
					break;

				case 222:
					// MSISDN 
					// M Condition : PDSN [Start, Stop], IWF [Start, Stop]
					
					if( dValLen > 0 && dValLen <= MAX_MSISDN_SIZE )
					{
						memcpy( &pstRDPkt->szMSISDN[0], &szAttrVal[0], dValLen );
						pstRDPkt->ucMSILen = dValLen;	
						pstRDPkt->ucMSISDNF = 0x01;

						dBothChk++;

					}
					else
					{
                        *dADR = ERROR_INVALID_PARAMETER_VALUE;
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
                        return -1;
					}
	
					break;

				/* 20040329,poopee */
				case 223:	// AAA-On
					if (dValLen == 1)
						pstRDPkt->ucAaaOn = szAttrVal[0];
					else if (dValLen == 2)
					{
						memcpy(&sTemp,&szAttrVal[0],dValLen);
						sTemp = CVT_SHORT(sTemp);
						pstRDPkt->ucAaaOn = sTemp;
					}	
					else if (dValLen == 4)
					{
						memcpy(&sTemp,&szAttrVal[0],dValLen);
						sTemp = CVT_INT(sTemp);
						pstRDPkt->ucAaaOn = sTemp;
					}	
					else
					{
                        *dADR = ERROR_INVALID_PARAMETER_VALUE;
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
                        return -1;
					}
				
					pstRDPkt->ucAaaOnF = 0x01;
					break;
					
				/* lander */
				case 224:	// Access-IN-Sub
					if (uiVendorID != VENDOR_3GPP2)
					{
						*dADR = ERROR_INVALID_PARAMETER_VALUE;
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
						return -1;
					}

					if (dValLen == 1) // case KTF Standard
					{
						pstRDPkt->ucAccessINSub = szAttrVal[0];
					} 
					else if (dValLen == 2)
					{
						memcpy (&sTemp, &szAttrVal[0], dValLen);
						sTemp = CVT_SHORT (sTemp);
						pstRDPkt->ucAccessINSub = sTemp;
					}
					else if (dValLen == 4)
					{
						memcpy (&dTempVal, &szAttrVal[0], dValLen);
						dTempVal = CVT_INT (dTempVal);
						pstRDPkt->ucAccessINSub = dTempVal;
					}
					else 
					{
						*dADR = ERROR_INVALID_PARAMETER_VALUE;
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
						return -1;
					}
					pstRDPkt->ucAccessINSubF = 0x01;

					if (pstRDPkt->ucAccessINSub != 1)
					{
						*dADR = ERROR_INVALID_PARAMETER_VALUE;
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
						return -1;
					}
				
					break;

				case 245:
					// DSCP IP ADDRESS
					// M Condition : Optional 

					if( dValLen == sizeof(UINT) )
					{
						memcpy( &pstRDPkt->uiDSCPIP, &szAttrVal[0], dValLen );
						pstRDPkt->ucDSCPIPAddrF = 0x01;

					}
					else
					{
						*dADR = ERROR_INVALID_PARAMETER_VALUE;
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
						return -1;
					}

					break;

				case 246:
					// IN_Packet_Period
					// M Condition : PDSN [WSType=Áö´É¸Á, Start] 

					if( dValLen == sizeof(INT) )
					{

						memcpy( &pstRDPkt->dPPeriod, &szAttrVal[0], dValLen );
						pstRDPkt->dPPeriod = CVT_INT(pstRDPkt->dPPeriod);
						pstRDPkt->ucINPktPeriodF = 0x01;

						dWSTypeChk++;
					}
					else
					{
                        *dADR = ERROR_INVALID_PARAMETER_VALUE;
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
                        return -1;
					}

					break;

				case 247:
					// IN_TIME_PERIOD
					// M Condition : PDSN [WSType=Áö´É¸Á, Start]

					if( dValLen == sizeof(INT) )
					{
						memcpy( &pstRDPkt->dTPeriod, &szAttrVal[0], dValLen );
						pstRDPkt->dTPeriod = CVT_INT( pstRDPkt->dTPeriod );
						pstRDPkt->ucINTimePeriodF = 0x01;

						dWSTypeChk++;

					}
					else
					{
                        *dADR = ERROR_INVALID_PARAMETER_VALUE;
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
                        return -1;
					}

					break;

				case 248:
					// Total Time Duration 
					// M Condition : PDSN [Stop], IWF [Stop]
					
					if( dValLen == sizeof(INT) )
					{
						memcpy( &pstRDPkt->dTotalTime, &szAttrVal[0], dValLen );
						pstRDPkt->dTotalTime = CVT_INT(pstRDPkt->dTotalTime);
						pstRDPkt->ucTotalTimeDurationF = 0x01;
						
						dBothStopChk++;

					}
					else
					{
                        *dADR = ERROR_INVALID_PARAMETER_VALUE;
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
                        return -1;
					}

					break;

				case 252:
					// Retry Flag 
					// M Condition : PDSN [Start, Stop], IWF [Start, Stop]
					if( dValLen == 1 )
					{
						pstRDPkt->dRetryFlag = szAttrVal[0];
					}
					else if( dValLen == 2 )
					{
						memcpy( &sTemp, &szAttrVal[0], 2 );
						sTemp = CVT_SHORT( sTemp );
						pstRDPkt->dRetryFlag = sTemp;
					}
					else if( dValLen == 4 )
					{
						memcpy( &dTempVal, &szAttrVal[0], dValLen );
						pstRDPkt->dRetryFlag = CVT_INT( dTempVal );
					}
					else
					{

                        *dADR = ERROR_INVALID_PARAMETER_VALUE;
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
                        return -1;
					}

					pstRDPkt->sRetryF= 1;
					dBothChk++;

					break;
	
				case 253:
		
					//  AN Release Indicator 
					// M Condition : PDSN [Stop], IWF [Stop]
	
					if( dValLen == 1 )
					{
						pstRDPkt->dANRelIndi = szAttrVal[0];
					}
					else if( dValLen == 2 )
					{
						memcpy( &sTemp, &szAttrVal[0], 2 );
						sTemp = CVT_SHORT( sTemp );
						pstRDPkt->dANRelIndi = sTemp ;
						pstRDPkt->ucANReleaseIndiF = 0x01;
					}
					else if( dValLen == 4 )
					{
						memcpy( &dTempVal, &szAttrVal[0], dValLen );
						dTempVal = CVT_INT( dTempVal);
						pstRDPkt->dANRelIndi = dTempVal;
						pstRDPkt->ucANReleaseIndiF = 0x01;
					}
					else
					{
                        *dADR = ERROR_INVALID_PARAMETER_VALUE;
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
                        return -1;
					}

					dBothStopChk++;

					break;

				default :

#if 0
					*dADR = ERROR_INVALID_PARAMETER_VALUE;
					fprintf(stderr,"ERROR: invalid parameter type(%d/%d)!!!\n",dType,dVSType);
					return -1;
#endif
					/* R2.3.0 Change 2005.0113 (lander) ---> */
					/* IWF SEND VPN PARA */
					fprintf(stderr,"ERROR: invalid parameter type(%d/%d)!!!\n",dType,dVSType);
					break;
				}

				break;

			case 1:

				// User Name 
				// M Condition : PDSN [Start, Stop], IWF [Start, Stop]

				if( dValLen > 0 && dValLen <= MAX_USERNAME_SIZE )
				{
					memcpy( &pstRDPkt->szUserName, &szAttrVal[0], dValLen );
					pstRDPkt->szUserName[dValLen] = 0x00;
					pstRDPkt->ucUserLen = dValLen;
					pstRDPkt->ucUserNameF = 0x01;
					
					dBothChk++;

				}
				else
				{
                    *dADR = ERROR_INVALID_PARAMETER_VALUE;
					fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
                    return -1;
				}

				break;

			case 4:

				// NAS-IP-Address 
				// M Condition : PDSN [Start, Stop], IWF [Start, Stop]	

				if( dValLen == sizeof(UINT) )
				{	
					memcpy( &pstRDPkt->uiNASIP, &szAttrVal[0], dValLen );
					pstRDPkt->ucNASIPAddrF  = 0x01;

					dBothChk++;

				}
				else
				{
                    *dADR = ERROR_INVALID_PARAMETER_VALUE;
					fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
                    return -1;
				}

				break;

			case 8:

				// Frame-IP-Address 
				// M Condition : PDSN [Start, Stop], IWF [Start, Stop]

				if( dValLen == sizeof(UINT) )
				{
					memcpy( &pstRDPkt->uiUserIP, &szAttrVal[0], dValLen );
					pstRDPkt->ucFramedIPAddrF = 0x01;
					dBothChk++;

				}
				else
				{
                    *dADR = ERROR_INVALID_PARAMETER_VALUE;
					fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
                    return -1;
				}


				break;	

			case 30:
				//Called-Station-Id
				if(dValLen > 0 && dValLen < MAX_3GCALLEDID_SIZE) {
					memcpy(&pstRDPkt->sz3GCalledStatID[0], &szAttrVal[0], dValLen );
					pstRDPkt->sz3GCalledStatID[dValLen] = 0x00;
					pstRDPkt->uc3GCalledStatIDF = 0x01;
				} else {
					*dADR = ERROR_INVALID_PARAMETER_VALUE;	
					fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
					return -1;
				}
				break;
			
			case 31:
				/* Calling Station ID */
				
				// M Condition : PDSN [Start, Stop], IWF [Start, Stop]

				if( dValLen > 0 && dValLen < MAX_MIN_SIZE )
				{

					memcpy( &pstRDPkt->szMIN[0], &szAttrVal[0], dValLen );
					pstRDPkt->szMIN[dValLen] = 0x00;
					pstRDPkt->ucCallStatIDF = 0x01;

					dBothChk++;

				}
				else
				{
					*dADR = ERROR_INVALID_PARAMETER_VALUE;	
					fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
					return -1;
				}
					
				break;

			case 40:

				// Acct-Status-Type 
				// M Condition : PDSN [Start, Stop], IWF [Start, Stop]

				if( dValLen == sizeof(INT) )
				{
					memcpy( &pstRDPkt->dAStatType, &szAttrVal[0], dValLen );

					pstRDPkt->dAStatType = CVT_INT(pstRDPkt->dAStatType);

					if( pstRDPkt->dAStatType > 8 )
					{
						*dADR = ERROR_INVALID_PARAMETER_VALUE;	
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
						return -1;
					}

					pstRDPkt->ucAcctStatTypeF = 0x01;

					dBothChk++;

				}
				else
				{
					*dADR = ERROR_INVALID_PARAMETER_VALUE;	
					fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
					return -1;
				}

		
				break;

			case 42 :
				// Acct-Input-Octets 
				// M Condition : PDSN [Stop], IWF [Stop]

				if( dValLen == sizeof(INT) )
				{	
					memcpy( &pstRDPkt->dAcctInOctes, &szAttrVal[0], dValLen );
					pstRDPkt->dAcctInOctes = CVT_INT( pstRDPkt->dAcctInOctes );
					pstRDPkt->ucAcctInputOctetsF = 0x01;

					dBothStopChk++;

				}
				else
				{
					*dADR = ERROR_INVALID_PARAMETER_VALUE;	
					fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
					return -1;
				}

				break;

			case 43 :

				// Acct-Output-Octets 
				// M Condition : PDSN [Stop], IWF [Stop]

				if( dValLen == sizeof(INT) )
				{
					memcpy( &pstRDPkt->dAcctOutOctes, &szAttrVal[0], dValLen );
					pstRDPkt->dAcctOutOctes = CVT_INT( pstRDPkt->dAcctOutOctes );

					pstRDPkt->ucAcctOutputOctetsF = 0x01;

					dBothStopChk++;

				}
				else
				{
					*dADR = ERROR_INVALID_PARAMETER_VALUE;	
					fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
					return -1;
				}

				break;


			case 44:

				// Accounting Session ID 
				// M Condition : PDSN [Start, Stop], IWF [Start, Stop]

				if(dNASType == DEF_NAS_GGSN) {
					if(dValLen > 0 && dValLen < MAX_3GACCID_SIZE)
					{
						memcpy( pstRDPkt->szAcctSessID, szAttrVal, dValLen );
                        pstRDPkt->szAcctSessID[dValLen] = 0x00;
                        pstRDPkt->AcctSessIDLen = dValLen;
                        pstRDPkt->llAcntSessID = strtoul((const char*)&szAttrVal[0], NULL, 16 );
                        pstRDPkt->ucAcctSessionIDF  = 0x01;

					} else {
						*dADR = ERROR_INVALID_PARAMETER_VALUE;	
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
						return -1;
					}
				} else {
					if( dValLen == sizeof(INT64) )
					{
						memcpy( &pstRDPkt->llAcntSessID, &szAttrVal[0], dValLen );
						/*  8byte revers ordering */	
						pstRDPkt->llAcntSessID = CVT_INT64( pstRDPkt->llAcntSessID);
						pstRDPkt->ucAcctSessionIDF = 0x01;

						dBothChk++;
					}
					else
					{
						*dADR = ERROR_INVALID_PARAMETER_VALUE;	
						fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
						return -1;
					}
				}

				break;
			
			case 46:
				//  Acct-Session-Time 
				// M Condition : PDSN [Stop]
				
				if( dValLen == sizeof(time_t) )
				{
					memcpy( &pstRDPkt->dAcctSessTime, &szAttrVal[0], dValLen );
					pstRDPkt->dAcctSessTime = CVT_INT( pstRDPkt->dAcctSessTime );
					pstRDPkt->ucAcctSessTimeF = 0x01;

					dPDSNStop++;

				}
				else
				{
					*dADR = ERROR_INVALID_PARAMETER_VALUE;	
					fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
					return -1;
				}

				break;

			/* R2.3.3 add 2005.1005 (lander) ---> */
			case 50 :
				if (dValLen > 0 && dValLen < MAX_ACCT_MULTI_SESSID_SIZE)
				{
					memcpy(pstRDPkt->szAcctMultiSessID, &szAttrVal[0], dValLen);
					pstRDPkt->ucAcctMultiSessIdF = 0x01;
				}
				else
				{
					*dADR = ERROR_INVALID_PARAMETER_VALUE;	
					fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
					return -1;
				}
				break;
			/* <--- */

			case 55:

				// Event Time 
				// M Condition : PDSN [Stop], IWF [Stop]

				if( dValLen == sizeof(time_t) )
				{
					memcpy( &pstRDPkt->dEventTime, &szAttrVal[0], dValLen );
					pstRDPkt->dEventTime = CVT_INT( pstRDPkt->dEventTime );
					pstRDPkt->ucEventTimeF = 0x01;

					dBothStopChk++;

				}
				else
				{
					*dADR = ERROR_INVALID_PARAMETER_VALUE;	
					fprintf(stderr,"ERROR: invalid parameter length(%d/%d=%d)!!!\n",dType,dVSType,dValLen);
					return -1;
				}
	
				break;

			default :
#if 0
				*dADR = ERROR_INVALID_PARAMETER_VALUE;
				fprintf(stderr,"ERROR: invalid parameter type(%d/%d)!!!\n",dType,dVSType);
				return -1;
#endif
				fprintf(stderr,"ERROR: invalid parameter type(%d/%d)!!!\n",dType,dVSType);
				break;

		}
		
	}

	
	if( dIdx != dBufSize || dTempSize != 0 )
	{
		*dADR = ERROR_INVALID_PARAMETER_VALUE;
		fprintf(stderr,"ERROR: message size mismatch(%d/%d/%d)!!!\n",dIdx,dBufSize,dTempSize);
		return -1;
	}


	return dRet;
}


int ParsingRadiusAttr( INT *dType, INT *dVSType, UINT *uiVendorID, UCHAR *szAttrVal, INT *dValLen, UCHAR *pszBuf, INT *dBufSize )
{
	
	INT		Length;
	UCHAR	ucLen, ucVSLen;
	UCHAR	ucType, ucVSType;
	UCHAR	Value[128];
	INT		dValue;
	int 	dIdx=0;
	UINT	dVenderID;

    if( *dBufSize <= 3 )
	{

        return -1;
	}

    ucType = pszBuf[dIdx];
    dIdx += 1;
    *dType = ucType;

    ucLen = pszBuf[dIdx];
    dIdx += 1;

    Length = ucLen;
    Length -= 2;

    if( Length <= 0 )
	{
        return -1;
	}

    if( Length > (*dBufSize-2) )
	{
        return -1;
	}

    if( ucType == 26  )
    {
        /*      Vender Specific Type        */
        if( (*dBufSize - dIdx ) < 7 )
		{
            return -1;
		}

        memcpy( &dVenderID, &pszBuf[dIdx], 4 );

		*uiVendorID = CVT_INT(dVenderID);
        dIdx += 4;

        ucVSType = pszBuf[dIdx];
        dIdx += 1;

        *dVSType = ucVSType;

        ucVSLen = pszBuf[dIdx];
        dIdx += 1;

        Length = ucVSLen;
        Length -= 2;

        if( Length <= 0 )
		{
            return -1;
		}

        if( (*dBufSize - dIdx ) < Length )
		{
            return -1;
		}

        memcpy( &szAttrVal[0], &pszBuf[dIdx], Length );
        szAttrVal[Length] = 0x00;
        dIdx += Length;
        *dValLen = Length;
    }
    else
    {
        /*   Normal Value Attribute         */
        if( (*dBufSize - dIdx ) < Length )
		{
            return -1;
		}

        memcpy( &szAttrVal[0], &pszBuf[dIdx], Length );
        szAttrVal[Length] = 0x00;
        dIdx += Length;
        *dValLen = Length;
    }

	*dBufSize = dIdx;

	return 0; 
} 


int PrnRadiusAttr( int dType, int dVSType, int dValLen, unsigned char *szAttr )
{

	char   TempMsg[2048];
	int    dTempLen=0;
	int    i;

	sprintf(&TempMsg[dTempLen], "TYPE[%3d] VSTYPE[%3d] LEN[%2d] VALUE :", dType, dVSType, dValLen );
	dTempLen = strlen(TempMsg);

	for( i=0; i<dValLen; i++ )
	{
		sprintf(&TempMsg[dTempLen], " %02X", szAttr[i] ); 
		dTempLen += 3;
	}

	TempMsg[dTempLen] = 0x00;
	
	return 0;
}

