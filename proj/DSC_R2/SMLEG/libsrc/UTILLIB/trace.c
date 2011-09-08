/**********************************************************
                 KTF IPAS Project

   Author   : LEE SANG HO
   Section  : IPAS Project
   SCCS ID  : %W%
   Date     : %G%
   Revision History :
		'03.	1. 15	Initial

   Description:
		Make Send MSG

   Copyright (c) ABLEX 2003
***********************************************************/

/**A.1*  File Inclusion ***********************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <sys/sysinfo.h>
#include <ipam_shm.h>
#include <ipam_sesssvc.h>
#include <ipaf_svc.h>
#include <ipam_ipaf.h>
#include <ipam_sys.h>
#include <ipam_error.h>
#include <ipam_sys.h>
#include <arpa/inet.h>
#include <ipam_sessif_new.h>
#include <udrgen_define.h>

#include <comm_msgtypes.h>

/**B.1*  Definition of New Constants *********************/
/**B.2*  Definition of New Type  **************************/

#define  MAX_DATE_LEN     256
#define  MAX_URL_LEN      240


/**C.1*  Declaration of Variables  ************************/

char    crmtime_str[MAX_DATE_LEN];
extern  st_AAAFlgInfo  stAAAFlg;

/**D.1*  Definition of Functions  *************************/
char 	*cvt_ipaddr(UINT uiIP);
char    *crTime(time_t when);

void 	CvtBinToHexa     (int dLen, unsigned char *szSrc, unsigned char *szTrg);
void    MakeStringTime   (int dType, time_t stTime, char *szBuf);
void    MakeACCTrcInfo   ( pst_ACCInfo pstACC , char *szBuf, int *pdOffset, char dRcvSnd_Flag);
void    MakeTraceUDR     ( pst_UDRInfo pstUDRInfo , char *szBuf, int *pdLen );
void    conv_id          (INT64 lltmp, char *str);
int     OctetPrint       ( UCHAR *szBuf, UCHAR *szBSID );
int     dMakeURLTrace    ( char *szURL, char *szBody ); 
int		OctetPrint2( UCHAR *szBuf, char *szSubnet );	// 080107, poopee, SUBNET


/**D.2*  Definition of Functions  *************************/

/***********************************************************
     Function Name :   dMakeACCInfo
     Parameter(s)  :
     Function      :   Make trace infomation of st_ACCInfo
     return        :
     History       :
***********************************************************/

char *crTime(time_t when)
{

    memset (crmtime_str, 0, 255);

	strftime(crmtime_str, 80, "%Y-%m-%d %T %a", localtime((time_t *)&when));
			crmtime_str[21] = toupper(crmtime_str[21]);
	    	crmtime_str[22] = toupper(crmtime_str[22]);

    return crmtime_str;
}


char *cvt_ipaddr(UINT uiIP)
{

    struct in_addr inaddr;

    inaddr.s_addr = uiIP;

    return (inet_ntoa(inaddr)); 
}


void CvtBinToHexa(int dLen, unsigned char *szSrc, unsigned char *szTrg)
{
    int      i;
    int      dOffset = 0;

   for(i=0; i<dLen; i++)
   {
       sprintf( (char*)&szTrg[dOffset], "%02X", szSrc[i] );
       dOffset += 2;
   }

   szTrg[dOffset] = 0x00;
   return;
}



void MakeStringTime(int dType, time_t dTime, char *szBuf)
{
    struct tm stTime;
    /*
    if(dType == 1)
    	dTime -= 3600*9;
    */
    if(dTime < 1)
	    //strcpy(szBuf, "NO DATETIME");
	    strcpy(szBuf, "    ");
    else
    {
        localtime_r(&dTime, &stTime);
        sprintf(szBuf, "%04d-%02d-%02d %02d:%02d:%02d",
	            stTime.tm_year + 1900, stTime.tm_mon + 1, stTime.tm_mday,
	            stTime.tm_hour, stTime.tm_min, stTime.tm_sec);
    }

    return;
}




void conv_id(INT64 lltmp, char *str)
{
    INT64   tmp1, tmp2, bitmask;
    int i;

    for (i=0; i<8; i++)
    {
        tmp1 = lltmp >> i*8;
        bitmask = 0x000000ff;
        tmp2 = (tmp1 & bitmask);
        str[8-i-1] = (char) tmp2;
    }
    str[8] = 0;
}


int OctetPrint( UCHAR *szBuf, UCHAR *szBSID )
{
	int   i;

	for( i=0; i<12; i++ )
	{
		//if (i !=0 && (i%3) == 0) sprintf(&szBSID[i],"|");
		sprintf(&szBSID[i*2], "%02x", szBuf[i]);
	}

	return 0;
}

	
	
	



/* BSD_T1.0.0  add 20060627(challa) ---> */
/***********************************************************
     Function Name :   dMakeACCTrcInfo
     Parameter(s)  :
     Function      :   Make st_ACCInfo Trace 
     return        :
     History       :
***********************************************************/
void MakeACCTrcInfo( pst_ACCInfo pstACC , char *szBuf, int *pdOffset, char dRcvSnd_Flag)
{
    int   		dOffset = 0;
	UINT		uiIPTmp = 0;
    char  		*szLogMsg = NULL;
	char		szAuth[128];
	char 		szDate[MAX_DATE_LEN];
    char        szAcctSessID[8+1];
    char        szCorrllID[8+1];
	UCHAR       szBSID[32];
	char		szSubnet[MAX_SUBNET_SIZE+3];	// 080107, poopee, SUBNET
	struct      in_addr  stAddr;
	
    memset(szAuth, 0x00, 128);
    memset(szDate, 0x00, MAX_DATE_LEN);
    memset(&stAddr, 0x00, sizeof(struct in_addr));

	szLogMsg = szBuf;

    if(dRcvSnd_Flag == DEF_RCV_FLAG)
    {           
	    sprintf(&szLogMsg[dOffset], "CODE=[%3d]  ID=[%3d]  LEN=[%d]",
	    		pstACC->ucCode, pstACC->ucID, pstACC->uiRADIUSLen);
	    dOffset = strlen(szLogMsg);

	    CvtBinToHexa(16, pstACC->szAuthen, szAuth);
   	    sprintf(&szLogMsg[dOffset],  "\nAUTHENTICATOR                          = [0x%s]", szAuth);
	    dOffset = strlen(szLogMsg);
    }

    if( pstACC->ucTimeStampF > 0x00 )
    {
        MakeStringTime(1, pstACC->uiTimeStamp, szDate);
        sprintf (&szLogMsg[dOffset], "\n[5535/200] TIMESTAMP                   = [%s]", szDate);
        dOffset = strlen(szLogMsg);	
    }

    if( pstACC->ucUDRSeqF > 0x00 )
    {
        sprintf (&szLogMsg[dOffset], "\n[5535/201] UDR-SEQUENCE                = [%d]", pstACC->uiUDRSeq );
	    dOffset = strlen(szLogMsg);

    }

	if( pstACC->ucCallStatIDF > 0x00 )
	{
    	sprintf(&szLogMsg[dOffset],  "\n[      31] CALLING-STATION-ID          = [%s]", pstACC->szMIN );
    	dOffset = strlen(szLogMsg);
	}
    
	if( pstACC->ucESNF > 0x00 )
	{
		// 070514,poopee, Vendor-ID Ãß°¡ 
        if( pstACC->szESN[0] != 0 )
			/*
		    sprintf(&szLogMsg[dOffset],  "\n[5535/ 52] ESN                         = [%s]", pstACC->szESN);
			*/
		    sprintf(&szLogMsg[dOffset],  "\n[5535/ 52] ESN                         = [********]");
        else
            sprintf(&szLogMsg[dOffset],  "\n[5535/ 52] ESN                         = [        ]" );
		dOffset = strlen(szLogMsg);
	}

	if( pstACC->ucFramedIPF > 0x00 )
	{
		uiIPTmp = htonl(pstACC->uiFramedIP);
    	sprintf(&szLogMsg[dOffset],  "\n[       8] FRAMED-IP-ADDRESS           = [%s]", cvt_ipaddr(uiIPTmp) );
    	dOffset = strlen(szLogMsg);
	}

	if( pstACC->ucUserNameF > 0x00 )
	{
    	sprintf(&szLogMsg[dOffset],  "\n[       1] USER-NAME                   = [%s]", pstACC->szUserName );
    	dOffset = strlen(szLogMsg);
	}

	if( pstACC->ucAcctSessIDF > 0x00 )
	{
        conv_id ( pstACC->llAcctSessID, szAcctSessID);
    	sprintf(&szLogMsg[dOffset],  "\n[      44] ACCT-SESSION-ID             = [0x%s]", szAcctSessID );
    	dOffset = strlen(szLogMsg);
    }

	if( pstACC->ucCorrelationIDF > 0x00 )
	{
        conv_id ( pstACC->llCorrelID, szCorrllID);
    	sprintf(&szLogMsg[dOffset],  "\n[5535/ 44] CORRELATION-ID              = [0x%s]", szCorrllID );
    	dOffset = strlen(szLogMsg);
	}

	if( pstACC->ucSessContinueF > 0x00 )
	{
		sprintf(&szLogMsg[dOffset],  "\n[5535/ 48] SESSION-CONT                = [%d]", pstACC->uiSessContinue );
    	dOffset = strlen(szLogMsg);
	}

	if( pstACC->ucBeginningSessF > 0x00 )
	{
		sprintf(&szLogMsg[dOffset],  "\n[5535/ 51] BEGINNING-SESSION           = [%d]", pstACC->uiBeginnigSess);
    	dOffset = strlen(szLogMsg);
    }
		
	if( pstACC->ucHAIPF > 0x00 )
	{
		stAddr.s_addr = pstACC->uiHAIP;
		sprintf(&szLogMsg[dOffset],  "\n[5535/  7] HA-IP-ADDR                  = [%s]", inet_ntoa(stAddr) );
    	dOffset = strlen(szLogMsg);
	}

	if( pstACC->ucNASIPF > 0x00) 
	{
		uiIPTmp = htonl(pstACC->uiNASIP);
		sprintf(&szLogMsg[dOffset],  "\n[       4] NAS-IP-ADDRESS              = [%s]", cvt_ipaddr(uiIPTmp) );
    	dOffset = strlen(szLogMsg);
	}

	if( pstACC->ucPCFIPF > 0x00 )
	{
		uiIPTmp = htonl(pstACC->uiPCFIP);
		sprintf(&szLogMsg[dOffset],  "\n[5535/  9] PCF-IP-ADDR                 = [%s]", cvt_ipaddr(uiIPTmp) );
    	dOffset = strlen(szLogMsg);
	}


	if( pstACC->ucBSIDF > 0x00 )
	{
		OctetPrint( (UCHAR *)pstACC->szBSID, szBSID );
    	sprintf(&szLogMsg[dOffset],  "\n[5535/ 10] BSID                        = [%s]", szBSID );

		/*
        if( pstACC->szBSID[0] != 0 )
    	    sprintf(&szLogMsg[dOffset],  "\n[ 26/ 10] BSID                         = [%s]", pstACC->szBSID );
        else
    	    sprintf(&szLogMsg[dOffset],  "\n[ 26/ 10] BSID                         = [            ]" );
		*/

    	dOffset = strlen(szLogMsg);
	}


	if( pstACC->ucUserIDF > 0x00 )
	{
		sprintf(&szLogMsg[dOffset],  "\n[5535/ 11] USER-ID                     = [%d]", pstACC->dUserID );
    	dOffset = strlen(szLogMsg);
	}

	if( pstACC->ucFwdFCHMuxF > 0x00 )
	{
		sprintf(&szLogMsg[dOffset],  "\n[5535/ 12] F_FCH_MUX                   = [%d]", pstACC->dFwdFCHMux );
    	dOffset = strlen(szLogMsg);
	}

	if( pstACC->ucRevFCHMuxF > 0x00 )
	{
		sprintf(&szLogMsg[dOffset],  "\n[5535/ 13] R_FCH_MUX                   = [%d]", pstACC->dRevFCHMux );
    	dOffset = strlen(szLogMsg);
	}

	if( pstACC->ucSvcOptF > 0x00 )
	{
		sprintf(&szLogMsg[dOffset],  "\n[5535/ 16] SERVICE-OPTION              = [%d]", pstACC->dSvcOpt );
    	dOffset = strlen(szLogMsg);
	}

	if( pstACC->ucFwdTrafTypeF > 0x00 )
	{
		sprintf(&szLogMsg[dOffset],  "\n[5535/ 17] FTYPE                       = [%d]", pstACC->dFwdTrafType );
    	dOffset = strlen(szLogMsg);
	}

	if( pstACC->ucRevTrafTypeF > 0x00 )
	{
		sprintf(&szLogMsg[dOffset],  "\n[5535/ 18] RTYPE                       = [%d]", pstACC->dRevTrafType );
    	dOffset = strlen(szLogMsg);
	}

	if( pstACC->ucFCHSizeF > 0x00 )
	{
		sprintf(&szLogMsg[dOffset],  "\n[5535/ 19] FFSIZE                      = [%d]", pstACC->dFCHSize );
    	dOffset = strlen(szLogMsg);
	}

	if( pstACC->ucFwdFCHRCF > 0x00 )
	{
		sprintf(&szLogMsg[dOffset],  "\n[5535/ 20] FRC                         = [%d]", pstACC->dFwdFCHRC );
    	dOffset = strlen(szLogMsg);
	}

	if( pstACC->ucRevFCHRCF > 0x00 )
	{
		sprintf(&szLogMsg[dOffset],  "\n[5535/ 21] RRC                         = [%d]", pstACC->dRevFCHRC );
    	dOffset = strlen(szLogMsg);
	}

	if( pstACC->ucIPTechF > 0x00 )
	{
		sprintf(&szLogMsg[dOffset],  "\n[5535/ 22] IP-TECH                     = [%d]", pstACC->dIPTech );
    	dOffset = strlen(szLogMsg);
	}

	if( pstACC->ucCompTunnelIndF > 0x00 )
	{
		sprintf(&szLogMsg[dOffset],  "\n[5535/ 23] COMP-FLAG                   = [%d]", pstACC->dCompTunnelInd );
    	dOffset = strlen(szLogMsg);
	}

	if( pstACC->ucReleaseIndF > 0x00 )
	{
		sprintf(&szLogMsg[dOffset],  "\n[5535/ 24] REASON-IND                  = [%d]", pstACC->dReleaseInd );
    	dOffset = strlen(szLogMsg);
	}

	if( pstACC->ucDCCHSizeF > 0x00 )
	{
		sprintf(&szLogMsg[dOffset],  "\n[5535/ 50] DFSIZE                      = [%d]", pstACC->dDCCHSize );
    	dOffset = strlen(szLogMsg);
	}

	if( pstACC->ucAlwaysOnF > 0x00 )
	{
		sprintf(&szLogMsg[dOffset],  "\n[5535/ 78] ALWAYS-ON                   = [%d]", pstACC->dAlwaysOn );
    	dOffset = strlen(szLogMsg);
	}

	if( pstACC->ucAcctOutOctF > 0x00 )
	{
		sprintf(&szLogMsg[dOffset],  "\n[      43] ACCT-OUTPUT-OCTETS          = [%d]", pstACC->dAcctOutOct );
    	dOffset = strlen(szLogMsg);
	}

	if( pstACC->ucAcctInOctF > 0x00 )
	{
		sprintf(&szLogMsg[dOffset],  "\n[      42] ACCT-INPUT-OCTETS           = [%d]", pstACC->dAcctInOct );
    	dOffset = strlen(szLogMsg);
	}

	if( pstACC->ucBadPPPFrameCntF > 0x00 )
	{
		sprintf(&szLogMsg[dOffset],  "\n[5535/ 25] BAD-FRAME-COUNT             = [%d]", pstACC->dBadPPPFrameCnt );
    	dOffset = strlen(szLogMsg);
	}


	if( pstACC->ucEventTimeF > 0x00 )
	{
		MakeStringTime(1, pstACC->uiEventTime, szDate);
    	sprintf(&szLogMsg[dOffset],  "\n[      55] EVENT-TIMESTAMP             = [%s]", szDate);
    	dOffset = strlen(szLogMsg);
	}

	if( pstACC->ucActTimeF > 0x00 )
	{
		sprintf(&szLogMsg[dOffset],  "\n[5535/ 49] ACTIVE-TIME                 = [%d]", pstACC->uiActTime );
    	dOffset = strlen(szLogMsg);
	}

	if( pstACC->ucNumActF > 0x00 )
	{
		sprintf(&szLogMsg[dOffset],  "\n[5535/ 30] NUM-ACTIVE                  = [%d]", pstACC->dNumAct );
    	dOffset = strlen(szLogMsg);
	}

	if( pstACC->ucTermSDBOctCntF > 0x00 )
	{
		sprintf(&szLogMsg[dOffset],  "\n[5535/ 31] SDB-INPUT-OCTETS            = [%d]", pstACC->dTermSDBOctCnt );
    	dOffset = strlen(szLogMsg);
	}

	if( pstACC->ucOrgSDBOctCntF > 0x00 )
	{
		sprintf(&szLogMsg[dOffset],  "\n[5535/ 32] SDB-OUTPUT-OCTETS           = [%d]", pstACC->dOrgSDBOctCnt );
    	dOffset = strlen(szLogMsg);
	}

	if( pstACC->ucTermNumSDBF > 0x00 )
	{
		sprintf(&szLogMsg[dOffset],  "\n[5535/ 33] NUMSDB-INPUT                = [%d]", pstACC->dTermNumSDB );
    	dOffset = strlen(szLogMsg);
	}

	if( pstACC->ucOrgNumSDBF > 0x00 )
	{
		sprintf(&szLogMsg[dOffset],  "\n[5535/ 34] NUMSDB-OUTPUT               = [%d]", pstACC->dOrgNumSDB );
    	dOffset = strlen(szLogMsg);
	}

	if( pstACC->ucRcvHDLCOctF > 0x00 )
	{
		sprintf(&szLogMsg[dOffset],  "\n[5535/ 43] NUM-BYTES-RECEIVED-TOTAL    = [%d]", pstACC->dRcvHDLCOct );
    	dOffset = strlen(szLogMsg);
	}

	if( pstACC->ucInMIPSigCntF > 0x00 )
	{
		sprintf(&szLogMsg[dOffset],  "\n[5535/ 46] MIP-SIGNAL-INBOUND-COUNT    = [%d]", pstACC->dInMIPSigCnt );
    	dOffset = strlen(szLogMsg);
	}

	if( pstACC->ucOutMIPSigCntF > 0x00 )
	{
		sprintf(&szLogMsg[dOffset],  "\n[5535/ 47] MIP-SIGNAL-OUTBOUND-COUNT   = [%d]", pstACC->dOutMIPSigCnt );
    	dOffset = strlen(szLogMsg);
	}

	if( pstACC->ucIPQoSF > 0x00 )
	{
		sprintf(&szLogMsg[dOffset],  "\n[5535/ 36] IP-QOS                      = [%d]", pstACC->dIPQoS );
    	dOffset = strlen(szLogMsg);
	}

	if( pstACC->ucAirQoSF > 0x00 )
	{
		sprintf(&szLogMsg[dOffset],  "\n[5535/ 39] AIR-PRIORITY                = [%d]", pstACC->dAirQoS );
    	dOffset = strlen(szLogMsg);
	}

	if( pstACC->ucAcctInPktF > 0x00 )
	{
		sprintf(&szLogMsg[dOffset],  "\n[      47] ACCT-INPUT-PACKETS          = [%d]", pstACC->dAcctInPkt );
    	dOffset = strlen(szLogMsg);
	}

	if( pstACC->ucAcctOutPktF > 0x00 )
	{
		sprintf(&szLogMsg[dOffset],  "\n[      48] ACCT-OUTPUT-PACKETS         = [%d]", pstACC->dAcctOutPkt );
    	dOffset = strlen(szLogMsg);
	}

	if( pstACC->ucRPConnectIDF > 0x00 )
	{
		//sprintf(&szLogMsg[dOffset],  "\n[ 26/ 41] R-P-CONNECTION-ID            = [0x%02x]", pstACC->dRPConnectID );
		sprintf(&szLogMsg[dOffset],  "\n[5535/ 41] R-P-CONNECTION-ID           = [%d]", pstACC->dRPConnectID );
    	dOffset = strlen(szLogMsg);
	}

	if( pstACC->ucAcctAuthF > 0x00 )
	{
		sprintf(&szLogMsg[dOffset],  "\n[      45] ACCT-AUTHENTIC              = [%d]", pstACC->dAcctAuth );
    	dOffset = strlen(szLogMsg);
	}

	if( pstACC->ucAcctSessTimeF > 0x00 )
	{
		sprintf(&szLogMsg[dOffset],  "\n[      46] ACCT-SESSION-TIME           = [%d]", pstACC->uiAcctSessTime );
    	dOffset = strlen(szLogMsg);
	}

	if( pstACC->ucAcctTermCauseF > 0x00 )
	{
		sprintf(&szLogMsg[dOffset],  "\n[      49] ACCT-TERMINATE-CAUSE        = [%d]", pstACC->dAcctTermCause );
    	dOffset = strlen(szLogMsg);
	}

	if( pstACC->ucAcctStatTypeF > 0x00 )
	{
		sprintf(&szLogMsg[dOffset],  "\n[      40] ACCT-STATUS-TYPE            = [%d]", pstACC->dAcctStatType );
    	dOffset = strlen(szLogMsg);
	}

	if( pstACC->ucNASPortTypeF > 0x00 )
	{
		sprintf(&szLogMsg[dOffset],  "\n[      61] NAS-PORT-TYPE               = [%d]", pstACC->dNASPortType );
    	dOffset = strlen(szLogMsg);
	}

	if( pstACC->ucNASPortF > 0x00 )
	{
		sprintf(&szLogMsg[dOffset],  "\n[       5] NAS-PORT                    = [%d]", pstACC->dNASPort);
    	dOffset = strlen(szLogMsg);
	}

	if( pstACC->ucNASPortIDF > 0x00 )
	{
		sprintf(&szLogMsg[dOffset],  "\n[      87] NAS-PORT-ID                 = [%s]", pstACC->szNASPortID);
    	dOffset = strlen(szLogMsg);
	}

	if( pstACC->ucSvcTypeF > 0x00 )
	{
		sprintf(&szLogMsg[dOffset],  "\n[       6] SERVICE-TYPE                = [%d]", pstACC->dSvcType);
    	dOffset = strlen(szLogMsg);
	}

	if( pstACC->ucAcctDelayTimeF > 0x00 )
	{
		sprintf(&szLogMsg[dOffset],  "\n[      41] ACCT-DELAY-TIME             = [%d]", pstACC->uiAcctDelayTime);
    	dOffset = strlen(szLogMsg);
	}

	/* Modified by kmyang 2007.05.10 */
	if( pstACC->ucC23BITF > 0x00 )
	{
		sprintf(&szLogMsg[dOffset],  "\n[6964/  1] C23-BIT            	       = [%d]", pstACC->uiC23BIT );
    	dOffset = strlen(szLogMsg);
	}

	/* 080220, poopee, HBIT */
	if( pstACC->ucHBITF > 0x00 )
	{
		sprintf(&szLogMsg[dOffset],  "\n[6964/  1] H-BIT                       = [%d]", pstACC->uiHBIT);
		dOffset = strlen(szLogMsg);
	}

	// 080107, poopee, SUBNET
	if( pstACC->ucSubnetF > 0x00 )
	{
		OctetPrint2( (UCHAR *)pstACC->szSubnet, szSubnet );
		sprintf(&szLogMsg[dOffset],  "\n[5535/108] SUBNET                      = [%s]", szSubnet);
    	dOffset = strlen(szLogMsg);
	}

    szLogMsg[dOffset] = 0x00;


    *pdOffset += dOffset;

    return;
}



/*********************************************************** 
     Function Name :   dMakeUDRInfo
     Parameter(s)  :
     Function      :   Make trace UDR Info 
	 return 	   :
	 History       :
***********************************************************/
void MakeTraceUDR( pst_UDRInfo pstUDRInfo, char *szBuf, int *pdLen )
{
	time_t		stTime;
    int   		dOffset = 0;
	UINT		uiIPTmp = 0;
    char  		*szLogMsg = NULL;
	char		szAuth[40];
	char        szDate[MAX_DATE_LEN];

	struct 		in_addr  stAddr;
    
    memset(&stTime, 0x00, sizeof(time_t));
    memset(szAuth, 0x00, 40);
    memset(szDate, 0x00, MAX_DATE_LEN);
    memset(&stAddr, 0x00, sizeof(struct in_addr));


	szLogMsg = szBuf;

	dOffset += sprintf( &szLogMsg[dOffset], 
		"\n-------------------------------------");

    if( stAAAFlg.ucDataSvcTypeF > 0x00 && pstUDRInfo->ucDataSvcTypeF > 0x00 )
    {
	    dOffset += sprintf (&szLogMsg[dOffset], "\n[5535/203] DATA-SERVICE-TYPE           = [%d]", pstUDRInfo->dDataSvcType );
    }

    if( stAAAFlg.ucTransIDF > 0x00 && pstUDRInfo->ucTranIDF > 0x00 )
    {
	    dOffset += sprintf (&szLogMsg[dOffset], "\n[5535/204] TRANSACTION-ID              = [%d]", pstUDRInfo->uiTranID );
    }

    if( stAAAFlg.ucReqTimeF > 0x00 && pstUDRInfo->ucReqTimeF > 0x00 )
    {
        MakeStringTime(1, pstUDRInfo->tReqTime, szDate);
	    dOffset += sprintf (&szLogMsg[dOffset], "\n[5535/205] REQUEST-TIME                = [%s]", szDate );
    }

    if( stAAAFlg.ucResTimeF > 0x00 && pstUDRInfo->ucResTimeF > 0x00 )
    {
        MakeStringTime(1, pstUDRInfo->tResTime, szDate);
	    dOffset += sprintf (&szLogMsg[dOffset], "\n[5535/206] RESPONSE-TIME               = [%s]", szDate );
    }

    if( stAAAFlg.ucSessTimeF > 0x00 && pstUDRInfo->ucSessionTimeF > 0x00 )
    {
	    dOffset += sprintf (&szLogMsg[dOffset], "\n[5535/207] SESSION-TIME                = [%d]", (int)pstUDRInfo->tSessionTime );
    }

    if( stAAAFlg.ucDestIPF > 0x00 && pstUDRInfo->ucDestIPF > 0x00 )
    {
		uiIPTmp = htonl(pstUDRInfo->uiDestIP);
	    dOffset += sprintf (&szLogMsg[dOffset], "\n[5535/208] SERVER-IP-ADDRESS           = [%s]", cvt_ipaddr(uiIPTmp) );
    }

    if( stAAAFlg.ucDestPortF > 0x00 && pstUDRInfo->ucDestPortF > 0x00 )
    {
    	dOffset += sprintf (&szLogMsg[dOffset], "\n[5535/209] SERVER-PORT                 = [%d]", pstUDRInfo->dDestPort );
    }
    
    if( stAAAFlg.ucSrcPortF > 0x00 && pstUDRInfo->ucSrcPortF > 0x00 )
    {
	    dOffset += sprintf (&szLogMsg[dOffset], "\n[5535/210] TERMINAL-PORT               = [%d]", pstUDRInfo->dSrcPort );
    }

#if 0  /** for URL LEN=720 **/
    if( stAAAFlg.ucURLF > 0x00 && pstUDRInfo->ucURLF > 0x00 )
    {
	    dOffset += sprintf (&szLogMsg[dOffset], "\n[ 26/211] URL                          = [%s]", pstUDRInfo->szURL );
    }

#else

    if( stAAAFlg.ucURLF > 0x00 && pstUDRInfo->ucURLF > 0x00 )
    {
	    dOffset += dMakeURLTrace( pstUDRInfo->szURL, &szLogMsg[dOffset] );
    }
#endif
	

    if( stAAAFlg.ucCTypeF > 0x00 && pstUDRInfo->ucCTypeF > 0x00 )
    {
	    dOffset += sprintf (&szLogMsg[dOffset], "\n[5535/212] DOWNLOAD-TYPE               = [%d]", pstUDRInfo->dCType );
    }

    if( stAAAFlg.ucAppIDF > 0x00 && pstUDRInfo->ucAppIDF > 0x00 )
    {
	    dOffset += sprintf (&szLogMsg[dOffset], "\n[5535/213] APPLICATION-ID              = [%s]", pstUDRInfo->szAppID );
    }

    if( stAAAFlg.ucCntCodeF > 0x00 && pstUDRInfo->ucContentCodeF > 0x00 )
    {
	    dOffset += sprintf (&szLogMsg[dOffset], "\n[5535/214] CONTENT-ID                  = [%s]", pstUDRInfo->szContentCode );
    }

    if( stAAAFlg.ucMethTypeF > 0x00 && pstUDRInfo->ucMethodTypeF > 0x00 )
    {
	    dOffset += sprintf (&szLogMsg[dOffset], "\n[5535/215] METHOD-TYPE                 = [%d]", pstUDRInfo->dMethodType );
    }

    if( stAAAFlg.ucResultCodeF > 0x00 && pstUDRInfo->ucResultCodeF > 0x00 )
    {
	    dOffset += sprintf (&szLogMsg[dOffset], "\n[5535/216] RESULT-CODE                 = [%d]", pstUDRInfo->dResultCode );
    }

    if( stAAAFlg.ucIPUpSizeF > 0x00 && pstUDRInfo->ucIPUpSizeF > 0x00 )
    {
	    dOffset += sprintf (&szLogMsg[dOffset], "\n[5535/217] IP-LAYER-UPLOAD-SIZE        = [%d]", pstUDRInfo->dIPUpSize );
    }

    if( stAAAFlg.ucIPDownSizeF > 0x00 && pstUDRInfo->ucIPDownSizeF > 0x00 )
    {
	    dOffset += sprintf (&szLogMsg[dOffset], "\n[5535/218] IP-LAYER-DOWMLOAD-SIZE      = [%d]", pstUDRInfo->dIPDownSize );
    }

    if( stAAAFlg.ucReInputSizeF > 0x00 && pstUDRInfo->ucRetransInSizeF > 0x00 )
    {
	    dOffset += sprintf (&szLogMsg[dOffset], "\n[5535/219] TCP-RETRANS-INPUT-SIZE      = [%d]", pstUDRInfo->dRetransInSize );
    }

    if( stAAAFlg.ucReOutputSizeF > 0x00 && pstUDRInfo->ucRetransOutSizeF > 0x00 )
    {
	    dOffset += sprintf (&szLogMsg[dOffset], "\n[5535/220] TCP-RETRANS-OUTPUT-SIZE     = [%d]", pstUDRInfo->dRetransOutSize );
    }

/* R1.3.0 20061127 add(challa) ---> */
	if( stAAAFlg.ucUseCntF > 0x00 && pstUDRInfo->ucUseCountF > 0x00 )
	{
	    dOffset += sprintf (&szLogMsg[dOffset], "\n[5535/230] USE-COUNT                   = [%d]", pstUDRInfo->dUseCount );
	}

	if( stAAAFlg.ucUseTimeF > 0x00 && pstUDRInfo->ucUseTimeF > 0x00 )
	{
	    dOffset += sprintf (&szLogMsg[dOffset], "\n[5535/231] USE-TIME                    = [%d]", pstUDRInfo->dUseTime );
	}

	if( stAAAFlg.ucTotSizeF > 0x00 && pstUDRInfo->ucTotalSizeF > 0x00 )
	{
	    dOffset += sprintf (&szLogMsg[dOffset], "\n[5535/232] TOTAL-SIZE                  = [%d]", pstUDRInfo->dTotalSize );
	}

	if( stAAAFlg.ucTotTimeF > 0x00 && pstUDRInfo->ucTotalTimeF > 0x00 )
	{
	    dOffset += sprintf (&szLogMsg[dOffset], "\n[5535/233] TOTAL-TIME                  = [%d]", pstUDRInfo->dTotalTime );
	}
/* <--- */

	/* 20070104 Add(challa) NEW UDR ---> */
	if( stAAAFlg.ucAudioInputIPSizeF > 0x00 && pstUDRInfo->ucAudioInputIPSizeF > 0x00 )
	{
	    dOffset += sprintf (&szLogMsg[dOffset], "\n[5535/238] AUDIO-UPLOAD-SIZE           = [%d]", pstUDRInfo->dAudioInputIPSize );
	}

	if( stAAAFlg.ucAudioOutputIPSizeF > 0x00 && pstUDRInfo->ucAudioOutputIPSizeF > 0x00 )
	{
	    dOffset += sprintf (&szLogMsg[dOffset], "\n[5535/239] AUDIO-DOWNLOAD-SIZE         = [%d]", pstUDRInfo->dAudioOutputIPSize );
	}

	if( stAAAFlg.ucVideoInputIPSizeF > 0x00 && pstUDRInfo->ucVideoInputIPSizeF > 0x00 )
	{
	    dOffset += sprintf (&szLogMsg[dOffset], "\n[5535/240] VIDEO-UPLOAD-SIZE           = [%d]", pstUDRInfo->dVideoInputIPSize );
	}

	if( stAAAFlg.ucVideoOutputIPSizeF > 0x00 && pstUDRInfo->ucVideoOutputIPSizeF > 0x00 )
	{
	    dOffset += sprintf (&szLogMsg[dOffset], "\n[5535/241] VIDEO-DOWNLOAD-SIZE         = [%d]", pstUDRInfo->dVideoOutputIPSize );
	}
	/* <--- */


    if( stAAAFlg.ucCntLenF > 0x00 && pstUDRInfo->ucContentLenF > 0x00 )
    {
	    dOffset += sprintf (&szLogMsg[dOffset], "\n[5535/221] TRANSACTION-CONTENT-LENGTH  = [%d]", pstUDRInfo->dContentLen );
    }

    if( stAAAFlg.ucTransCompleteF > 0x00 && pstUDRInfo->ucTranCompleteF > 0x00 )
    {
	    dOffset += sprintf (&szLogMsg[dOffset], "\n[5535/222] TRANSACTION-COMPLETENESS    = [%d]", pstUDRInfo->dTranComplete );
    }

    if( stAAAFlg.ucTransTermReasonF > 0x00 && pstUDRInfo->ucTranTermReasonF > 0x00 )
    {
	    dOffset += sprintf (&szLogMsg[dOffset], "\n[5535/223] UDR-GENERATION-REASON       = [%d]", pstUDRInfo->dTranTermReason );
    }

    if( stAAAFlg.ucUserAgentF > 0x00 && pstUDRInfo->ucUserAgentF > 0x00 )
    {
	    dOffset += sprintf (&szLogMsg[dOffset], "\n[5535/224] USER-AGENT                  = [%s]", pstUDRInfo->szUserAgent );
    }

/* R1.3.0 20061127 add(challa) ---> */
	if( stAAAFlg.ucPktCntF > 0x00 && pstUDRInfo->ucPacketCntF > 0x00 )
	{
	    dOffset += sprintf (&szLogMsg[dOffset], "\n[5535/237] DOWNLOAD-INFO               = [%s]", pstUDRInfo->szPacketCnt );
    }
/* <--- */

/* R2.2.0 20070510 add(kmyang) ---> */
	if( stAAAFlg.ucCalledMinF > 0x00 && pstUDRInfo->ucCalledMinF > 0x00 )
	{
	    dOffset += sprintf (&szLogMsg[dOffset], "\n[5535/251] CALLEE-MIN                  = [%s]", pstUDRInfo->szCalledMin);
    }
	if( stAAAFlg.ucCallerMinF > 0x00 && pstUDRInfo->ucCallerMinF > 0x00 )
	{
	    dOffset += sprintf (&szLogMsg[dOffset], "\n[5535/252] CALLER-MIN                  = [%s]", pstUDRInfo->szCallerMin);
    }
/* <--- */

    szLogMsg[dOffset] = 0x00;

    *pdLen += dOffset;

    return;
}



#if 1
int dMakeURLTrace( char *szURL, char *szBody ) 
{
	int   dURLSIZE =0;
	int   dCount   =0;
    int   dDivide  =0;
    int   dOffset  =0;

	char  szBuf[241];
    char  *szTempMsg;

	szTempMsg = szBody;
	dURLSIZE  = strlen( szURL );

	dCount  = (dURLSIZE / MAX_URL_LEN);
	dDivide = (dURLSIZE % MAX_URL_LEN);

	switch( dCount )
	{
	case 0:
	    dOffset += sprintf (&szTempMsg[dOffset], "\n[5535/225] URL                         = [%s]", szURL );
		break;

	case 1:
		strncpy(szBuf, szURL, MAX_URL_LEN );
		szBuf[MAX_URL_LEN] = 0x00;
	    dOffset += sprintf (&szTempMsg[dOffset], "\n[5535/225] URL                         = [%s]", szBuf );

		if( dDivide > 0 )
		{
			memset(szBuf, 0x00, sizeof(szBuf));

			strncpy(szBuf, &szURL[MAX_URL_LEN], dDivide );
			szBuf[dDivide] = 0x00;
	    	dOffset += sprintf (&szTempMsg[dOffset], "\n[5535/226] URL                         = [%s]", szBuf );
		}
		break;

	case 2:
		strncpy(szBuf, szURL, MAX_URL_LEN );
		szBuf[MAX_URL_LEN] = 0x00;
	    dOffset += sprintf (&szTempMsg[dOffset], "\n[5535/225] URL                         = [%s]", szBuf );

		strncpy(szBuf, &szURL[MAX_URL_LEN], MAX_URL_LEN );
		szBuf[MAX_URL_LEN] = 0x00;
	    dOffset += sprintf (&szTempMsg[dOffset], "\n[5535/226] URL                         = [%s]", szBuf );

		if( dDivide > 0 )
		{
			memset(szBuf, 0x00, sizeof(szBuf));
			strncpy(szBuf, &szURL[480], dDivide );
			szBuf[dDivide] = 0x00;

	    	dOffset += sprintf (&szTempMsg[dOffset], "\n[5535/227] URL                         = [%s]", szBuf );
		}
		break;

	case 3:
		strncpy(szBuf, szURL, MAX_URL_LEN );
		szBuf[MAX_URL_LEN] = 0x00;
	    dOffset += sprintf (&szTempMsg[dOffset], "\n[5535/225] URL                         = [%s]", szBuf );

		strncpy(szBuf, &szURL[MAX_URL_LEN], MAX_URL_LEN );
		szBuf[MAX_URL_LEN] = 0x00;
	    dOffset += sprintf (&szTempMsg[dOffset], "\n[5535/226] URL                         = [%s]", szBuf );

		strncpy(szBuf, &szURL[480], MAX_URL_LEN );
		szBuf[MAX_URL_LEN] = 0x00;
	    dOffset += sprintf (&szTempMsg[dOffset], "\n[5535/227] URL                         = [%s]", szBuf );
		
		break;

	default:
		break;

	}

	return dOffset;
}

// 080107, poopee, SUBNET
int	OctetPrint2( UCHAR *szBuf, char *szSubnet )
{
	int	i;

	strcpy(szSubnet,"0x");
	for( i=0; i<MAX_SUBNET_SIZE; i++ )
		sprintf(&szSubnet[2+i*2], "%02X", szBuf[i]);

	return 0;
}
#endif
