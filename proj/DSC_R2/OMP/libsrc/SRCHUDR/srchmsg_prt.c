/*
*  _  _   _  ____ ____    _   __     __ _    _    _    _____  __
* | || \ | ||  __|  _ \  / \  \ \   / // \  | |  | |  |  _\ \/ /
* | ||  \| || |__| |_) |/ _ \  \ \ / // _ \ | |  | |  | |__\  /
* | || \   ||  __|    // ___ \  \   // ___ \| |__| |__|  __ | |
* |_||_|\__||_|  |_|\_\_/   \_\  \_//_/   \_\____|____|____||_|
*
* Copyright 2004 Infravalley, Inc. All Rights Reserved
*
* ------------------------------------------------------------------------------
* MODULE NAME : srchmsg_prt.c
* DESCRIPTION :
* REVISION    : DATE       VER NAME                   DESCRIPTION
*               2004/06/08 1.0 poopee                 Created
* COMMENTS    :
*
*
* ------------------------------------------------------------------------------
*/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// 07.22 jjinri #include "srchmsg.h"

extern int		errno;


// int print_aaa_msg(st_AAAREQ *aaa_msg, SRCH_INFO *pSRCHINFO);
char *str_time(time_t t);
void print_hex_buf(char *buffer, int len, char *result);
// 07.22 jjinri void conv_id(INT64 lltmp, char *str);
// int OctetPrint( UCHAR *szBuf, UCHAR *szBSID );

void print_hexa_dump(unsigned char *pDumpMsg, 
                unsigned int uiDumpLen)
{
                
    unsigned int   uiCnt, uiNextCnt=1;
    
    printf("Hexa Start\n");
    for(uiCnt=1; uiCnt<=uiDumpLen; uiCnt++)
    {
        printf("[%02x]", *(pDumpMsg+(uiCnt-1)));

        if (uiCnt==15*uiNextCnt) {
            printf("\n");
            uiNextCnt++;
        }

    }

    printf("\n");
    printf("Hexa End\n");

}




#if 0 // 07.22 jjinri
/*------------------------------------------------------------------------------
* FUNCTIONS   :
* SYNOPSIS    :
* PARAMETERS  :
* RETURNS     :
* REMARKS     : 
* ----------------------------------------------------------------------------*/
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
        sprintf(&szBSID[i*2], "%02x", szBuf[i]);

    return 0;
}


/*------------------------------------------------------------------------------
* FUNCTIONS   :
* SYNOPSIS    :
* PARAMETERS  :
* RETURNS     :
* REMARKS     : PW_ACCOUNTING_REQUEST, PW_ACCOUNTING_RESPONSE
* ----------------------------------------------------------------------------*/
int print_aaa_msg(st_AAAREQ *aaa_msg, SRCH_INFO *pSRCHINFO)
{
	int			len, i;
	char		buffer[1024*20], tmp_buf[128];
	char        szAcctSessID[8+1];
	char        szCorrllID[8+1];
    UCHAR       szBSID[32];
	struct in_addr	inaddr; 

    
	len = 0;
	memset( buffer, 0x00, sizeof(buffer) );

	/* timestamp */
	if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
	{
		sprintf(buffer,"-------------------------------------------------------\n");
		len = strlen(buffer);
		sprintf(&buffer[len],"[%s]\n",str_time(aaa_msg->stInfo.uiTimeStamp));
	}
	len = strlen(buffer);

    /** NORM : log format **/
	if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
	{
			sprintf(&buffer[len],"[UDRGEN->AAAIF] ACCOUNTING-REQUEST\n");
	}
	len = strlen(buffer);
	
	/* RADIUS header */

    for (i=0; (i==0) || (i<aaa_msg->dUDRCount); i++)
    {
        if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
        {
            sprintf(&buffer[len],"- - - - - - - - - - - - - - - - - - - - - - - - - - - -\n");
            len = strlen(buffer);
        }

        /* TimeStamp */
        if( aaa_msg->stInfo.ucTimeStampF > 0) 
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"TIME-STAMP                         = [%s]\n", 
                        str_time(aaa_msg->stInfo.uiTimeStamp) );
            else
                sprintf(&buffer[len],"%s< ", str_time(aaa_msg->stInfo.uiTimeStamp ));
            len = strlen(buffer);
        }
        else if(pSRCHINFO->viewtype == VIEWTYPE_LINE) 
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }


        /* UDR-SEQ */	
        if (aaa_msg->stInfo.ucUDRSeqF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"UDR-SEQUENCE                       = [%d]\n",aaa_msg->stInfo.uiUDRSeq);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stInfo.uiUDRSeq);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }


        /* MSID */
        if (aaa_msg->stInfo.ucCallStatIDF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"MSID                               = [%s]\n",aaa_msg->stInfo.szMIN);
            else
                sprintf(&buffer[len],"%s< ",aaa_msg->stInfo.szMIN);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }

        /* MDN */
        if (aaa_msg->stInfo.ucMDNF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"MDN                                = [%s]\n",aaa_msg->stInfo.szMDN);
            else
                sprintf(&buffer[len],"%s< ",aaa_msg->stInfo.szMDN);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }


        /* ESN */
        if (aaa_msg->stInfo.ucESNF > 0)
        {
            sprintf ((char*)aaa_msg->stInfo.szESN, "********");
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"ESN                                = [%s]\n",aaa_msg->stInfo.szESN);
            else
                sprintf(&buffer[len],"%s< ",aaa_msg->stInfo.szESN);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }

        /* Framed-IP-Address */
        if (aaa_msg->stInfo.ucFramedIPF > 0)
        {
            
            inaddr.s_addr = ntohl(aaa_msg->stInfo.uiFramedIP);
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"SOURCE-IP-ADDRESS                  = [%s]\n",inet_ntoa(inaddr));
            else
                sprintf(&buffer[len],"%s< ",inet_ntoa(inaddr));
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {	
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }

        /* USER-NAME */
        if (aaa_msg->stInfo.ucUserNameF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"NETWORK-ACCESS-INDENTIFIER         = [%s]\n",aaa_msg->stInfo.szUserName);
            else
                sprintf(&buffer[len],"%s< ",aaa_msg->stInfo.szUserName);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }


        /* Accounting-Session-ID */
        if (aaa_msg->stInfo.ucAcctSessIDF > 0)
        {
            conv_id ( aaa_msg->stInfo.llAcctSessID, szAcctSessID);

            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"ACCT-SESSION-ID                    = [0x%s]\n", szAcctSessID);
            else
                sprintf(&buffer[len],"0x%s< " , szAcctSessID);
            len = strlen(buffer);

        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }


        /* Correlation-ID */
        if (aaa_msg->stInfo.ucCorrelationIDF > 0)
        {
            conv_id ( aaa_msg->stInfo.llCorrelID, szCorrllID);

            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"CORRELATION-ID                     = [0x%s]\n", szCorrllID);
            else
                sprintf(&buffer[len],"0x%s< " , szCorrllID);
            len = strlen(buffer);

            /*
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"CORRELATION-ID                     = [%lld]\n",aaa_msg->stInfo.llCorrelID);
            else
                sprintf(&buffer[len],"%lld, ",aaa_msg->stInfo.llCorrelID);
            len = strlen(buffer);
            */
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }

        /* SESSION-CONTINUE */ 
        if (aaa_msg->stInfo.ucSessContinueF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"SESSION-CONTINUE                   = [%d]\n",aaa_msg->stInfo.uiSessContinue);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stInfo.uiSessContinue);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }

        /* BEGINNING-SESSION */
        if (aaa_msg->stInfo.ucBeginningSessF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"BEGINNING-SESSION                  = [%d]\n",aaa_msg->stInfo.uiBeginnigSess);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stInfo.uiBeginnigSess);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }

        /* HA-IP-Address */
        if (aaa_msg->stInfo.ucHAIPF > 0)
        {
            inaddr.s_addr = ntohl(aaa_msg->stInfo.uiHAIP);
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"HOME-AGENT                         = [%s]\n",inet_ntoa(inaddr));
            else
                sprintf(&buffer[len],"%s< ",inet_ntoa(inaddr));
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }


        /* NAS-IP-Address */
        if (aaa_msg->stInfo.ucNASIPF > 0)
        {
            inaddr.s_addr = ntohl(aaa_msg->stInfo.uiNASIP);
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"PDSN-ADDRESS                       = [%s]\n",inet_ntoa(inaddr));
            else
                sprintf(&buffer[len],"%s< ",inet_ntoa(inaddr));
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }


        /* Serving-PCF */
        if (aaa_msg->stInfo.ucPCFIPF > 0)
        {
            inaddr.s_addr = ntohl(aaa_msg->stInfo.uiPCFIP);
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"SERVING-PCF                        = [%s]\n",inet_ntoa(inaddr));
            else
                sprintf(&buffer[len],"%s< ",inet_ntoa(inaddr));
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }

        /* BSID */
        if (aaa_msg->stInfo.ucBSIDF > 0)
        {
            OctetPrint( (UCHAR *)aaa_msg->stInfo.szBSID, szBSID );

            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"BSID                               = [%s]\n", szBSID);
            else
                sprintf(&buffer[len],"%s< ", szBSID);

            len = strlen(buffer);
                
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }


        /* USER-ID */
        if (aaa_msg->stInfo.ucUserIDF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"USER-ZONE                          = [%d]\n",aaa_msg->stInfo.dUserID);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stInfo.dUserID);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }

        /* FCH_MUX-OPTION */
        if (aaa_msg->stInfo.ucFwdFCHMuxF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"FORWARD-FCH-MUX-OPTION             = [%d]\n",aaa_msg->stInfo.dFwdFCHMux);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stInfo.dFwdFCHMux);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }

        /* REVERSE-FCH-MUX-OPTION */
        if (aaa_msg->stInfo.ucRevFCHMuxF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"REVERSE-FCH-MUX-OPTION             = [%d]\n",aaa_msg->stInfo.dRevFCHMux);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stInfo.dRevFCHMux);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }

        /* Service-Option */
        if (aaa_msg->stInfo.ucSvcOptF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"SERVICE-OPTION                     = [%d]\n",aaa_msg->stInfo.dSvcOpt);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stInfo.dSvcOpt);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }

        /* FORWARD-TRAFFIC-TYPE */
        if (aaa_msg->stInfo.ucFwdTrafTypeF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"FORWARD-TRAFFIC-TYPE               = [%d]\n",aaa_msg->stInfo.dFwdTrafType);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stInfo.dFwdTrafType);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }


        /* REVERSE-TRAFFIC-TYPE */
        if (aaa_msg->stInfo.ucRevTrafTypeF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"REVERSE-TRAFFIC-TYPE               = [%d]\n",aaa_msg->stInfo.dRevTrafType);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stInfo.dRevTrafType);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }

        /* FCH-FRAME-SIZE */
        if (aaa_msg->stInfo.ucFCHSizeF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"FCH-FRAME-SIZE                     = [%d]\n",aaa_msg->stInfo.dFCHSize);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stInfo.dFCHSize);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }


        /* FORWARD-FCH-RC */
        if (aaa_msg->stInfo.ucFwdFCHRCF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"FORWARD-FCH-RC                     = [%d]\n",aaa_msg->stInfo.dFwdFCHRC);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stInfo.dFwdFCHRC);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }


        /* REVERSE-FCH-RC */
        if (aaa_msg->stInfo.ucRevFCHRCF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"REVERSE-FCH-RC                     = [%d]\n",aaa_msg->stInfo.dRevFCHRC);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stInfo.dRevFCHRC);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }


        /* IP-Technology */
        if (aaa_msg->stInfo.ucIPTechF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"IP-TECHNOLOGY                      = [%d]\n",aaa_msg->stInfo.dIPTech);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stInfo.dIPTech);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }


        /* COMPULSORY-TUNNEL-INDICATOR */
        if (aaa_msg->stInfo.ucCompTunnelIndF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"COMPULSORY-TUNNEL-INDICATOR        = [%d]\n",aaa_msg->stInfo.dCompTunnelInd);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stInfo.dCompTunnelInd);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }


        /* Release-Indicator */
        if (aaa_msg->stInfo.ucReleaseIndF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"RELEASE-INDICATOR                  = [%d]\n",aaa_msg->stInfo.dReleaseInd);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stInfo.dReleaseInd);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }


        /* DCCH-FRAME-SIZE */
        if (aaa_msg->stInfo.ucDCCHSizeF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"DCCH-FRAME-SIZE                    = [%d]\n",aaa_msg->stInfo.dDCCHSize);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stInfo.dDCCHSize);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }

        /*ALWAYS-ON */
        if (aaa_msg->stInfo.ucAlwaysOnF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"ALWAYS-ON                          = [%d]\n",aaa_msg->stInfo.dAlwaysOn);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stInfo.dAlwaysOn);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }

        /* DATA-OCTET-COUNT-TERMINATING */
        if (aaa_msg->stInfo.ucAcctOutOctF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"DATA-OCTET-COUNT-TERMINATING       = [%d]\n",aaa_msg->stInfo.dAcctOutOct);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stInfo.dAcctOutOct);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }

        /* DATA-OCTET-COUNT-ORIGINATING */
        if (aaa_msg->stInfo.ucAcctInOctF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"DATA-OCTET-COUNT-ORIGINATING       = [%d]\n",aaa_msg->stInfo.dAcctInOct);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stInfo.dAcctInOct);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }

        /* BAD-PPP-FRAME-COUNT */
        if (aaa_msg->stInfo.ucBadPPPFrameCntF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"BAD-PPP-FRAME-COUNT                = [%d]\n",aaa_msg->stInfo.dBadPPPFrameCnt);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stInfo.dBadPPPFrameCnt);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }


        /* Event-Time */
        if (aaa_msg->stInfo.ucEventTimeF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"EVENT-TIME                         = [%s]\n",
                    str_time(aaa_msg->stInfo.uiEventTime));
            else
                sprintf(&buffer[len],"%s< ",str_time(aaa_msg->stInfo.uiEventTime));
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }

        /* ACTIVE-TIME */
        if (aaa_msg->stInfo.ucActTimeF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"ACTIVE-TIME                        = [%d]\n",
                        aaa_msg->stInfo.uiActTime);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stInfo.uiActTime );
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }

        /* NUMBER-ACTIVE-TRANSITIONS */
        if (aaa_msg->stInfo.ucNumActF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"NUMBER-ACTIVE-TRANSITIONS          = [%d]\n",aaa_msg->stInfo.dNumAct);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stInfo.dNumAct);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }

        /* SDB-OCTET-COUNT-TERMINATING */
        if (aaa_msg->stInfo.ucTermSDBOctCntF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"SDB-OCTET-COUNT-TERMINATING        = [%d]\n",aaa_msg->stInfo.dTermSDBOctCnt);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stInfo.dTermSDBOctCnt);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }

        /* SDB-OCTET-COUNT-ORIGINATING */
        if (aaa_msg->stInfo.ucOrgNumSDBF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"SDB-OCTET-COUNT-ORIGINATING        = [%d]\n",aaa_msg->stInfo.dOrgSDBOctCnt);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stInfo.dOrgSDBOctCnt);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }


        /* NUMBER-OF-SDBS-TERMINATING */
        if (aaa_msg->stInfo.ucTermNumSDBF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"NUMBER-OF-SDBS-TERMINATING         = [%d]\n",aaa_msg->stInfo.dTermNumSDB);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stInfo.dTermNumSDB);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }


        /* NUMBER-OF-SDBS-ORIGINATING */
        if (aaa_msg->stInfo.ucOrgNumSDBF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"NUMBER-OF-SDBS-ORIGINATING         = [%d]\n",aaa_msg->stInfo.dOrgNumSDB);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stInfo.dOrgNumSDB);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }


        /* NUMBER-OF-HDLC-LAYER-OCTET-RECV */
        if (aaa_msg->stInfo.ucRcvHDLCOctF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"NUMBER-OF-HDLC-LAYER-OCTET-RECV    = [%d]\n",aaa_msg->stInfo.dRcvHDLCOct);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stInfo.dRcvHDLCOct);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }


        /* INBOUND-MIP-SIGNALING-OCTET-COUNT */
        if (aaa_msg->stInfo.ucInMIPSigCntF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"INBOUND-MIP-SIGNALING-OCTET-COUNT  = [%d]\n",aaa_msg->stInfo.dInMIPSigCnt);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stInfo.dInMIPSigCnt);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }

        /* OUTBOUND-MIP-SIGNALING-OCTET-COUNT */
        if (aaa_msg->stInfo.ucOutMIPSigCntF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"OUTBOUND-MIP-SIGNALING-OCTET-COUNT = [%d]\n",aaa_msg->stInfo.dOutMIPSigCnt);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stInfo.dOutMIPSigCnt);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }

        /* IP-QOS */
        if (aaa_msg->stInfo.ucIPQoSF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"IP-QOS                             = [%d]\n",aaa_msg->stInfo.dIPQoS);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stInfo.dIPQoS);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }

        /* Airlink-QoS */
        if (aaa_msg->stInfo.ucAirQoSF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"AIRLINK-QOS                        = [%d]\n",aaa_msg->stInfo.dAirQoS);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stInfo.dAirQoS);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }

        /* ACCT-INPUT-PACKETS */
        if (aaa_msg->stInfo.ucAcctInPktF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"ACCT-INPUT-PACKETS                 = [%d]\n",aaa_msg->stInfo.dAcctInPkt);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stInfo.dAcctInPkt);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }

        /* ACCT-OUTPUT-PACKETS */
        if (aaa_msg->stInfo.ucAcctOutPktF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"ACCT-OUTPUT-PACKETS                = [%d]\n",aaa_msg->stInfo.dAcctOutPkt);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stInfo.dAcctOutPkt);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }


        /* R-P-CONNECTION-ID */
        if (aaa_msg->stInfo.ucRPConnectIDF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"R-P-CONNECTION-ID                  = [%d]\n",aaa_msg->stInfo.dRPConnectID);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stInfo.dRPConnectID);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }


        /* ACCT-AUTHENTIC */
        if (aaa_msg->stInfo.ucAcctAuthF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"ACCT-AUTHENTIC                     = [%d]\n",aaa_msg->stInfo.dAcctAuth);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stInfo.dAcctAuth);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }


        /* Acct-Session-Time */
        if (aaa_msg->stInfo.ucAcctSessTimeF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"ACCT-SESSION-TIME                  = [%d]\n",aaa_msg->stInfo.uiAcctSessTime);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stInfo.uiAcctSessTime);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }

        /* ACCT-TERMINATE-CAUSE */
        if (aaa_msg->stInfo.ucAcctTermCauseF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"ACCT-TERMINATE-CAUSE               = [%d]\n",aaa_msg->stInfo.dAcctTermCause);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stInfo.dAcctTermCause);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }


        /* Acct-Status-Type */
        if (aaa_msg->stInfo.ucAcctStatTypeF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"ACCT-STATUS-TYPE                   = [%d]\n",aaa_msg->stInfo.dAcctStatType);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stInfo.dAcctStatType);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }

        /* NAS-PORT-TYPE */
        if (aaa_msg->stInfo.ucNASPortTypeF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"NAS-PORT-TYPE                      = [%d]\n",aaa_msg->stInfo.dNASPortType);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stInfo.dNASPortType);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }


        /* NAS-PORT */
        if (aaa_msg->stInfo.ucNASPortF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"NAS-PORT                           = [%d]\n",aaa_msg->stInfo.dNASPort);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stInfo.dNASPort);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }


        /* NAS-PORT-ID */
        if (aaa_msg->stInfo.ucNASPortIDF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"NAS-PORT-ID                        = [%s]\n",aaa_msg->stInfo.szNASPortID);
            else
                sprintf(&buffer[len],"%s< ",aaa_msg->stInfo.szNASPortID);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }


        if (aaa_msg->stInfo.ucSvcTypeF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"SERVICE-TYPE                       = [%d]\n",aaa_msg->stInfo.dSvcType);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stInfo.dSvcType);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }

        if (aaa_msg->stInfo.ucAcctDelayTimeF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"ACCT-DELAY-TIME                    = [%d]\n",aaa_msg->stInfo.uiAcctDelayTime);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stInfo.uiAcctDelayTime);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }

        if (aaa_msg->stInfo.ucC23BITF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
            {
	        sprintf(&buffer[len],"C23BIT                             = [%d]\n",aaa_msg->stInfo.uiC23BIT);
            }
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stInfo.uiC23BIT);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }



        /* Retry-Flag */
        /*
        if (aaa_msg->stInfo.ucRetryF > 0)
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"RETRY-FLAG                         = [%d]\n",aaa_msg->stInfo.uiRetryFlg);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stInfo.uiRetryFlg);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"< ");
            len = strlen(buffer);
        }
        */

        /* UDRInfo Print Start */
        if( aaa_msg->stUDRInfo[i].ucDataSvcTypeF > 0x00 )
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
            {
                sprintf(&buffer[len],"- - - - - - - - - - - - - - - - - - - - - - - - - - - -\n");
                len = strlen(buffer);
                sprintf(&buffer[len],"DATA-SERVICE-TYPE                  = [%d]\n",
                        aaa_msg->stUDRInfo[i].dDataSvcType);
            }
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stUDRInfo[i].dDataSvcType);
            len = strlen(buffer);
        }
        else if (pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"<");
            len = strlen(buffer);
        }


        if( aaa_msg->stUDRInfo[i].ucTranIDF > 0x00 )
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"TRANSACTION-ID                     = [%d]\n",aaa_msg->stUDRInfo[i].uiTranID);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stUDRInfo[i].uiTranID);
            len = strlen(buffer);
        }
        else if( pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"<");
            len = strlen(buffer);
        }


        if( aaa_msg->stUDRInfo[i].ucReqTimeF > 0x00 )
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"REQUEST-TIME                       = [%s]\n",
                    str_time(aaa_msg->stUDRInfo[i].tReqTime ));
            else
                sprintf(&buffer[len],"%s< ",str_time(aaa_msg->stUDRInfo[i].tReqTime ));
            len = strlen(buffer);
        }
        else if(pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"<");
            len = strlen(buffer);
        }


        if( aaa_msg->stUDRInfo[i].ucResTimeF > 0x00 )
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"RESPONSE-TIME                      = [%s]\n",
                    str_time(aaa_msg->stUDRInfo[i].tResTime ));
            else
                sprintf(&buffer[len],"%s< ",str_time(aaa_msg->stUDRInfo[i].tResTime ));
            len = strlen(buffer);
        }
        else if( pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"<");
            len = strlen(buffer);
        }


        if( aaa_msg->stUDRInfo[i].ucSessionTimeF > 0x00 )
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"SESSION-TIME                       = [%d]\n",aaa_msg->stUDRInfo[i].tSessionTime);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stUDRInfo[i].tSessionTime);
            len = strlen(buffer);
        }
        else if( pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"<");
            len = strlen(buffer);
        }


        if( aaa_msg->stUDRInfo[i].ucDestIPF > 0x00 )
        {
            inaddr.s_addr = ntohl(aaa_msg->stUDRInfo[i].uiDestIP);
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"SERVER-IP-ADDRESS                  = [%s]\n", inet_ntoa(inaddr) );
            else
                sprintf(&buffer[len],"%s< ", inet_ntoa(inaddr));
            len = strlen(buffer);
        } 
        else if( pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"<");
            len = strlen(buffer);
        }


        if( aaa_msg->stUDRInfo[i].ucDestPortF > 0x00 )
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"SERVER-PORT                        = [%d]\n",aaa_msg->stUDRInfo[i].dDestPort);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stUDRInfo[i].dDestPort);
            len = strlen(buffer);
        }
        else if(  pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"<");
            len = strlen(buffer);
        }


        if( aaa_msg->stUDRInfo[i].ucSrcPortF > 0x00 )
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"TERMINAL-PORT                      = [%d]\n",aaa_msg->stUDRInfo[i].dSrcPort);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stUDRInfo[i].dSrcPort);
            len = strlen(buffer);
        }
        else if( pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"<");
            len = strlen(buffer);
        }



        if( aaa_msg->stUDRInfo[i].ucURLF > 0x00 )
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"URL                                = [%s]\n",aaa_msg->stUDRInfo[i].szURL);
            else
                sprintf(&buffer[len],"%s< ",aaa_msg->stUDRInfo[i].szURL);
            len = strlen(buffer);
        }
        else if( pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"<");
            len = strlen(buffer);
        }
            

        if( aaa_msg->stUDRInfo[i].ucCTypeF > 0x00 )
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"C_TYPE                             = [%d]\n",aaa_msg->stUDRInfo[i].dCType);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stUDRInfo[i].dCType);
            len = strlen(buffer);
        }
        else if( pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"<");
            len = strlen(buffer);
        }

		/*
        if( aaa_msg->stUDRInfo[i].ucAppIDF > 0x00 )
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"APPLICATION-ID                     = [%d]\n",aaa_msg->stUDRInfo[i].dAppID);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stUDRInfo[i].dAppID);
            len = strlen(buffer);
        }
        else if( pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"<");
            len = strlen(buffer);
        }



        if( aaa_msg->stUDRInfo[i].ucContentCodeF > 0x00 )
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"CONTENT-CODE                       = [%d]\n",aaa_msg->stUDRInfo[i].dContentCode);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stUDRInfo[i].dContentCode);
            len = strlen(buffer);
        }
        else if( pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"<");
            len = strlen(buffer);
        }
		*/
        
		if( aaa_msg->stUDRInfo[i].ucAppIDF > 0x00 )
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"APPLICATION-ID                     = [%s]\n",aaa_msg->stUDRInfo[i].szAppID);
            else
                sprintf(&buffer[len],"%s< ",aaa_msg->stUDRInfo[i].szAppID);
            len = strlen(buffer);
        }
        else if( pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"<");
            len = strlen(buffer);
        }



        if( aaa_msg->stUDRInfo[i].ucContentCodeF > 0x00 )
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"CONTENT-CODE                       = [%s]\n",aaa_msg->stUDRInfo[i].szContentCode);
            else
                sprintf(&buffer[len],"%s< ",aaa_msg->stUDRInfo[i].szContentCode);
            len = strlen(buffer);
        }
        else if( pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"<");
            len = strlen(buffer);
        }


        if( aaa_msg->stUDRInfo[i].ucMethodTypeF > 0x00 )
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"METHOD-TYPE                        = [%d]\n",aaa_msg->stUDRInfo[i].dMethodType);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stUDRInfo[i].dMethodType);
            len = strlen(buffer);
        }
        else if(pSRCHINFO->viewtype == VIEWTYPE_LINE) 
        {
            sprintf(&buffer[len],"<");
            len = strlen(buffer);
        }


        if( aaa_msg->stUDRInfo[i].ucResultCodeF > 0x00 )
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"RESULT-CODE                        = [%d]\n",aaa_msg->stUDRInfo[i].dResultCode);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stUDRInfo[i].dResultCode);
            len = strlen(buffer);
        }
        else if(pSRCHINFO->viewtype == VIEWTYPE_LINE) 
        {
            sprintf(&buffer[len],"<");
            len = strlen(buffer);
        }


        if( aaa_msg->stUDRInfo[i].ucIPUpSizeF > 0x00 )
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"IP-LAYER-UPLOAD-SIZE               = [%d]\n",aaa_msg->stUDRInfo[i].dIPUpSize);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stUDRInfo[i].dIPUpSize);
            len = strlen(buffer);
        }
        else if(pSRCHINFO->viewtype == VIEWTYPE_LINE) 
        {
            sprintf(&buffer[len],"<");
            len = strlen(buffer);
        }


        if( aaa_msg->stUDRInfo[i].ucIPDownSizeF > 0x00 )
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"IP-LAYER-DOWMLOAD-SIZE             = [%d]\n",aaa_msg->stUDRInfo[i].dIPDownSize);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stUDRInfo[i].dIPDownSize);
            len = strlen(buffer);
        }
        else if(pSRCHINFO->viewtype == VIEWTYPE_LINE) 
        {
            sprintf(&buffer[len],"<");
            len = strlen(buffer);
        }

        if( aaa_msg->stUDRInfo[i].ucRetransInSizeF > 0x00 )
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"TCP-LAYER-RETRANS-INPUT-SIZE       = [%d]\n",aaa_msg->stUDRInfo[i].dRetransInSize);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stUDRInfo[i].dRetransInSize);
            len = strlen(buffer);
        }
        else if(pSRCHINFO->viewtype == VIEWTYPE_LINE) 
        {
            sprintf(&buffer[len],"<");
            len = strlen(buffer);
        }


        if( aaa_msg->stUDRInfo[i].ucRetransOutSizeF > 0x00 )
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"TCP-LAYER-RETRANS-OUTPUT-SIZE      = [%d]\n",
                        aaa_msg->stUDRInfo[i].dRetransOutSize);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stUDRInfo[i].dRetransOutSize);
            len = strlen(buffer);
        }
        else if(pSRCHINFO->viewtype == VIEWTYPE_LINE) 
        {
            sprintf(&buffer[len],"<");
            len = strlen(buffer);
        }

		
		if( aaa_msg->stUDRInfo[i].ucCPCodeF > 0x00 )
		{
			if (pSRCHINFO->viewtype == VIEWTYPE_NORM )
				sprintf(&buffer[len],"CP-CODE                            = [%d]\n",
						aaa_msg->stUDRInfo[i].dCPCode);
			else
				sprintf(&buffer[len],"%d< ",aaa_msg->stUDRInfo[i].dCPCode);
			len = strlen(buffer);
		}
		else if(pSRCHINFO->viewtype == VIEWTYPE_LINE)
		{
			sprintf(&buffer[len],"<");
			len = strlen(buffer);
		}

		if( aaa_msg->stUDRInfo[i].ucPhoneNumF > 0x00 )
		{
			if (pSRCHINFO->viewtype == VIEWTYPE_NORM )
				sprintf(&buffer[len],"PHONE-NUMBER                       = [%s]\n",
						aaa_msg->stUDRInfo[i].szPhoneNum);
			else
				sprintf(&buffer[len],"%s< ",aaa_msg->stUDRInfo[i].szPhoneNum);
			len = strlen(buffer);
		}
		else if(pSRCHINFO->viewtype == VIEWTYPE_LINE)
		{
			sprintf(&buffer[len],"<");
			len = strlen(buffer);
		}


	   if( aaa_msg->stUDRInfo[i].ucUseCountF > 0x00 )
	   {
		   if (pSRCHINFO->viewtype == VIEWTYPE_NORM )
			   sprintf(&buffer[len],"USE-COUNT                          = [%d]\n",
					   aaa_msg->stUDRInfo[i].dUseCount);
		   else
			   sprintf(&buffer[len],"%d< ",aaa_msg->stUDRInfo[i].dUseCount);
		   len = strlen(buffer);
	   }
	   else if(pSRCHINFO->viewtype == VIEWTYPE_LINE)
	   {
		   sprintf(&buffer[len],"<");
		   len = strlen(buffer);
	   }
		
		if( aaa_msg->stUDRInfo[i].ucUseTimeF > 0x00 )
		{
			if (pSRCHINFO->viewtype == VIEWTYPE_NORM )
				sprintf(&buffer[len],"USE-TIME                           = [%d]\n",
						aaa_msg->stUDRInfo[i].dUseTime);
			else
				sprintf(&buffer[len],"%d< ",aaa_msg->stUDRInfo[i].dUseTime);
			len = strlen(buffer);
		}
		else if(pSRCHINFO->viewtype == VIEWTYPE_LINE)
		{
				sprintf(&buffer[len],"<");
				len = strlen(buffer);
		}
		
		if( aaa_msg->stUDRInfo[i].ucTotalSizeF > 0x00 )
		{
			if (pSRCHINFO->viewtype == VIEWTYPE_NORM )
				sprintf(&buffer[len],"TOTAL-SIZE                         = [%d]\n",
						aaa_msg->stUDRInfo[i].dTotalSize);
			else
				sprintf(&buffer[len],"%d< ",aaa_msg->stUDRInfo[i].dTotalSize);
			len = strlen(buffer);
		}
		else if(pSRCHINFO->viewtype == VIEWTYPE_LINE)
		{
			sprintf(&buffer[len],"<");
			len = strlen(buffer);
																																						}


		if( aaa_msg->stUDRInfo[i].ucTotalTimeF > 0x00 )
		{
			if (pSRCHINFO->viewtype == VIEWTYPE_NORM )
				sprintf(&buffer[len],"TOTAL-TIME                         = [%d]\n",
						aaa_msg->stUDRInfo[i].dTotalTime);
			else
				sprintf(&buffer[len],"%d< ",aaa_msg->stUDRInfo[i].dTotalTime);
			len = strlen(buffer);
		}
		else if(pSRCHINFO->viewtype == VIEWTYPE_LINE)
		{
			sprintf(&buffer[len],"<");
			len = strlen(buffer);
		}

		if( aaa_msg->stUDRInfo[i].ucBillcomCountF > 0x00 )
		{
			if (pSRCHINFO->viewtype == VIEWTYPE_NORM )
				sprintf(&buffer[len],"BILLCOM-HEADER-COUNT               = [%d]\n",
						aaa_msg->stUDRInfo[i].dBillcomCount);
			else
				sprintf(&buffer[len],"%d< ",aaa_msg->stUDRInfo[i].dBillcomCount);
			len = strlen(buffer);
		}
		else if(pSRCHINFO->viewtype == VIEWTYPE_LINE)
		{
			sprintf(&buffer[len],"<");
			len = strlen(buffer);
		}

		if( aaa_msg->stUDRInfo[i].ucGWCountF > 0x00 )
		{
			if (pSRCHINFO->viewtype == VIEWTYPE_NORM )
				sprintf(&buffer[len],"GATEWAY-HEADER-COUNT               = [%d]\n",
						aaa_msg->stUDRInfo[i].dGWCount);
			else
				sprintf(&buffer[len],"%d< ",aaa_msg->stUDRInfo[i].dGWCount);
			len = strlen(buffer);
		}
		else if(pSRCHINFO->viewtype == VIEWTYPE_LINE)
		{
			sprintf(&buffer[len],"<");
			len = strlen(buffer);
		}

		if( aaa_msg->stUDRInfo[i].ucModelF > 0x00 )
		{
			if (pSRCHINFO->viewtype == VIEWTYPE_NORM )
				sprintf(&buffer[len],"HANDSET-MODEL                      = [%s]\n",
						aaa_msg->stUDRInfo[i].szModel);
			else
				sprintf(&buffer[len],"%s< ",aaa_msg->stUDRInfo[i].szModel);
			len = strlen(buffer);
		}
		else if(pSRCHINFO->viewtype == VIEWTYPE_LINE)
		{
			sprintf(&buffer[len],"<");
			len = strlen(buffer);
		}
	
        if( aaa_msg->stUDRInfo[i].ucAudioInputIPSizeF > 0x00 )
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM )
                sprintf(&buffer[len],"AUDIO-UPLOAD-SIZE                  = [%d]\n",
                        aaa_msg->stUDRInfo[i].dAudioInputIPSize);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stUDRInfo[i].dAudioInputIPSize);
            len = strlen(buffer);
        }
        else if(pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"<");
            len = strlen(buffer);
        }
 
                if( aaa_msg->stUDRInfo[i].ucAudioOutputIPSizeF > 0x00 )
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM )
                sprintf(&buffer[len],"AUDIO-DOWNLOAD-SIZE                = [%d]\n",
                        aaa_msg->stUDRInfo[i].dAudioOutputIPSize);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stUDRInfo[i].dAudioOutputIPSize);
            len = strlen(buffer);
        }
        else if(pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"<");
            len = strlen(buffer);
        }

	                if( aaa_msg->stUDRInfo[i].ucVideoInputIPSizeF > 0x00 )
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM )
                sprintf(&buffer[len],"VIDEO-UPLOAD-SIZE                  = [%d]\n",
                        aaa_msg->stUDRInfo[i].dVideoInputIPSize);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stUDRInfo[i].dVideoInputIPSize);
            len = strlen(buffer);
        }
        else if(pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"<");
            len = strlen(buffer);
        }
 
                if( aaa_msg->stUDRInfo[i].ucVideoOutputIPSizeF > 0x00 )
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM )
                sprintf(&buffer[len],"VIDEO-DOWNLOAD-SIZE                = [%d]\n",
                        aaa_msg->stUDRInfo[i].dVideoOutputIPSize);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stUDRInfo[i].dVideoOutputIPSize);
            len = strlen(buffer);
        }
        else if(pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"<");
            len = strlen(buffer);
        }

        if( aaa_msg->stUDRInfo[i].ucContentLenF > 0x00 )
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"TRANSACTION-CONTENT-LENGTH         = [%d]\n",
                        aaa_msg->stUDRInfo[i].dContentLen);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stUDRInfo[i].dContentLen);
            len = strlen(buffer);
        }
        else if(pSRCHINFO->viewtype == VIEWTYPE_LINE) 
        {
            sprintf(&buffer[len],"<");
            len = strlen(buffer);
        }


        if( aaa_msg->stUDRInfo[i].ucTranCompleteF > 0x00 )
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"TRANSACTION-COMPLETENESS           = [%d]\n",
                        aaa_msg->stUDRInfo[i].dTranComplete);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stUDRInfo[i].dTranComplete);
            len = strlen(buffer);
        }
        else if(pSRCHINFO->viewtype == VIEWTYPE_LINE) 
        {
            sprintf(&buffer[len],"<");
            len = strlen(buffer);
        }


        if( aaa_msg->stUDRInfo[i].ucTranTermReasonF > 0x00 )
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"TRANSACTION-TERMINATION-REASON     = [%d]\n",
                        aaa_msg->stUDRInfo[i].dTranTermReason);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->stUDRInfo[i].dTranTermReason);
            len = strlen(buffer);
        }
        else if(pSRCHINFO->viewtype == VIEWTYPE_LINE) 
        {
            sprintf(&buffer[len],"<");
            len = strlen(buffer);
        }


        if( aaa_msg->stUDRInfo[i].ucUserAgentF > 0x00 )
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"USER-AGENT  	                 = [%s]\n",
                        aaa_msg->stUDRInfo[i].szUserAgent);
            else
                sprintf(&buffer[len],"%s< ",aaa_msg->stUDRInfo[i].szUserAgent);
            len = strlen(buffer);
        }
        else if(pSRCHINFO->viewtype == VIEWTYPE_LINE) 
        {
            sprintf(&buffer[len],"<");
            len = strlen(buffer);
        }
        
		if( aaa_msg->stUDRInfo[i].ucPacketCntF > 0x00 )
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM)
                sprintf(&buffer[len],"DOWNLOAD-INFO                      = [%s]\n",
                        aaa_msg->stUDRInfo[i].szPacketCnt);
            else
                sprintf(&buffer[len],"%s< ",aaa_msg->stUDRInfo[i].szPacketCnt);
            len = strlen(buffer);
        }
        else if(pSRCHINFO->viewtype == VIEWTYPE_LINE) 
        {
            sprintf(&buffer[len],"<");
            len = strlen(buffer);
        }


	if( aaa_msg->dReserved > 0x00 )
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM )
        sprintf(&buffer[len],"SEND-OPT                           = [%d]\n",
                        aaa_msg->dReserved);
            else
                sprintf(&buffer[len],"%d< ",aaa_msg->dReserved);
            len = strlen(buffer);
        }
        else if(pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"<");
            len = strlen(buffer);
        }
 
    if( aaa_msg->stUDRInfo[i].ucCallerMinF > 0x00 )
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM )
        sprintf(&buffer[len],"CALLER-MIN                         = [%s]\n",
                        aaa_msg->stUDRInfo[i].szCallerMin);
            else
                sprintf(&buffer[len],"%s< ",aaa_msg->stUDRInfo[i].szCallerMin);
            len = strlen(buffer);
        }
        else if(pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"<");
            len = strlen(buffer);
        }

        if( aaa_msg->stUDRInfo[i].ucCalledMinF > 0x00 )
        {
            if (pSRCHINFO->viewtype == VIEWTYPE_NORM )
        sprintf(&buffer[len],"CALLEE-MIN                         = [%s]",
                        aaa_msg->stUDRInfo[i].szCalledMin);
            else
                sprintf(&buffer[len],"%s< ",aaa_msg->stUDRInfo[i].szCalledMin);
            len = strlen(buffer);
        }
        else if(pSRCHINFO->viewtype == VIEWTYPE_LINE)
        {
            sprintf(&buffer[len],"<");
            len = strlen(buffer);
        }
        
        sprintf(&buffer[len],"\n");
        len = strlen(buffer);

	}

	
	buffer[len] = 0;

#if 1
	if (fprintf(pSRCHINFO->fp,"%s",buffer) < 0)
	{
		fprintf(stderr,"ERROR: file(%s) write error(%s)!!!\n",pSRCHINFO->outfile,strerror(errno));
		fclose(pSRCHINFO->fp);
		fclose(pSRCHINFO->err_fp);	/* 20040923,poopee */
		exit(1);
	}
#else
	fprintf(stderr,"%s\n",buffer);
#endif

	return 0;
}
#endif



/*------------------------------------------------------------------------------
* FUNCTIONS   :
* SYNOPSIS    :
* PARAMETERS  :
* RETURNS     :
* REMARKS     :
* ----------------------------------------------------------------------------*/
char *str_time(time_t t)
{
	static char	mtime_str[81];

	strftime(mtime_str,80,"%Y-%m-%d %T",localtime(&t));
	return (char*)mtime_str;
}

/*------------------------------------------------------------------------------
* FUNCTIONS   :
* SYNOPSIS    :
* PARAMETERS  :
* RETURNS     :
* REMARKS     :
* ----------------------------------------------------------------------------*/
void print_hex_buf(char *buffer, int len, char *result)
{
	int			i;

	for (i=0; i<len; i++) sprintf(result+i*2,"%02X",0x000000ff & buffer[i]);
	result[len*2] = 0;
}
