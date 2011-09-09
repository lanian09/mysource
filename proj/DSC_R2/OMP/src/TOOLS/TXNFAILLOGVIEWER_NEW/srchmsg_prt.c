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

#include "srchmsg.h"

extern int		errno;


int print_txnlog_msg(st_DbFetchInfo_t *pDbFetch, 
                     st_LogParse_t    *pstLogParse, 
                     FILE *OutFd, st_SrchInfo_t  *pSrchInfo,
                     int iFailReason);

char *str_time(time_t t);
void print_hex_buf(char *buffer, int len, char *result);
void conv_id(INT64 lltmp, char *str);
int OctetPrint( UCHAR *szBuf, UCHAR *szBSID );

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
int print_txnlog_msg(st_DbFetchInfo_t *pDbFetch, 
                     st_LogParse_t    *pstLogParse, 
                     FILE *OutFd, st_SrchInfo_t  *pSrchInfo,
                     int iFailReason)
{
	int			len, i;
	char		buffer[1024*20], tmp_buf[128];
	char        szAcctSessID[8+1];
	char        szCorrllID[8+1];
    UCHAR       szBSID[32];
	struct in_addr	inaddr; 
    char        szFailReason[16];
    
    //print_hexa_dump(aaa_msg, sizeof(st_AAAREQ));
	len = 0;
	memset( buffer, 0x00, sizeof(buffer) );
    memset( szFailReason, 0x00, 16);

	/* timestamp */
	if (pSrchInfo->iViewType == VIEWTYPE_NORM)
	{
		sprintf(buffer,"-------------------------------------------------------\n");
		len = strlen(buffer);
		sprintf(&buffer[len],"[%s]\n",str_time(pDbFetch->iLogTime));
	}
	len = strlen(buffer);
    
    if (pSrchInfo->iViewType == VIEWTYPE_NORM)
    {
        sprintf(&buffer[len],"- - - - - - - - - - - - - - - - - - - - - - - - - - - -\n");
        len = strlen(buffer);
    }
    
    /* LogId */
    if (pSrchInfo->iViewType == VIEWTYPE_NORM)
        sprintf(&buffer[len],"LOG-ID                             = [%d]\n", 
                pDbFetch->iLogId );
    else
        sprintf(&buffer[len],"%d; ", pDbFetch->iLogId);
    len = strlen(buffer);
    
    /* LogTime */
    if (pSrchInfo->iViewType == VIEWTYPE_NORM)
        sprintf(&buffer[len],"LOG-TIME                           = [%s]\n", 
                str_time(pDbFetch->iLogTime) );
    else
        sprintf(&buffer[len],"%s; ", str_time(pDbFetch->iLogTime));
    len = strlen(buffer);
    
    /* FLAG */
    if (pSrchInfo->iViewType == VIEWTYPE_NORM)
        sprintf(&buffer[len],"FLAG                               = [%d]\n", 
                pDbFetch->iFlag);
    else
        sprintf(&buffer[len],"%d; ", pDbFetch->iFlag);
    len = strlen(buffer);
    
    switch(iFailReason)
    {
        case ERR_DECODE:
            snprintf(szFailReason, 16, "%s", "DECODE");
            break;
        
        case ERR_IPPOOL:
            snprintf(szFailReason, 16, "%s", "IPPOOL");
            break;

        case ERR_SESSION:
            snprintf(szFailReason, 16, "%s", "SESSION");
            break;
        
        case ERR_SEND:
            snprintf(szFailReason, 16, "%s", "SENDFAIL");
            break;
        
        case ERR_REDIRECT:
            snprintf(szFailReason, 16, "%s", "REDIRECT");
            break;
        
        default :
            snprintf(szFailReason, 16, "%s/%d", "UNKNOWN", iFailReason);
            break;

    }
    if (pSrchInfo->iViewType == VIEWTYPE_NORM)
        sprintf(&buffer[len],"FAIL-REASON                        = [%s]\n", 
                szFailReason);
    else
        sprintf(&buffer[len],"%s; ", szFailReason);
    len = strlen(buffer);
    
    
    
    /* Start Print Log Sub Info */
    /* MIN */
    if (pSrchInfo->iViewType == VIEWTYPE_NORM)
        sprintf(&buffer[len],"MIN-NUMBER                         = [%s]\n", 
                pstLogParse->szMin);
    else
        sprintf(&buffer[len],"%s; ", pstLogParse->szMin);
    len = strlen(buffer);
    
    /* Src IP */
    inaddr.s_addr = pstLogParse->uiSrcIp;
    if (pSrchInfo->iViewType == VIEWTYPE_NORM)
        sprintf(&buffer[len],"TERMINAL-IP-ADDRESS                = [%s]\n", 
                inet_ntoa(inaddr));
    else
        sprintf(&buffer[len],"%s; ", inet_ntoa(inaddr));
    len = strlen(buffer);
    
    /* Src Port */
    if (pSrchInfo->iViewType == VIEWTYPE_NORM)
        sprintf(&buffer[len],"TERMINAL-PORT                      = [%d]\n", 
                pstLogParse->uiSrcPort);
    else
        sprintf(&buffer[len],"%d; ", pstLogParse->uiSrcPort);
    len = strlen(buffer);
    
    /* SUB-NO */
    if (pSrchInfo->iViewType == VIEWTYPE_NORM)
        sprintf(&buffer[len],"SUB-NUMBER                         = [%s]\n", 
                pstLogParse->szSubNo);
    else
        sprintf(&buffer[len],"%s; ", pstLogParse->szSubNo);
    len = strlen(buffer);
    
    /* Request-Time */
    if (pSrchInfo->iViewType == VIEWTYPE_NORM)
        sprintf(&buffer[len],"REQUEST-TIME                       = [%s]\n",
                str_time(pstLogParse->tReqTime));
    else
        sprintf(&buffer[len],"%s; ", str_time(pstLogParse->tReqTime));
    len = strlen(buffer);
    
    /* Response-Time */
    if (pSrchInfo->iViewType == VIEWTYPE_NORM)
        sprintf(&buffer[len],"RESPONSE-TIME                      = [%s]\n",
                str_time(pstLogParse->tResTime));
    else
        sprintf(&buffer[len],"%s; ", str_time(pstLogParse->tResTime));
    len = strlen(buffer);
    
    /* WAP Request Size */
    if (pSrchInfo->iViewType == VIEWTYPE_NORM)
        sprintf(&buffer[len],"WAP-REQUEST-SIZE                   = [%d]\n",
                pstLogParse->iWapReqSize);
    else
        sprintf(&buffer[len],"%d; ", pstLogParse->iWapReqSize);
    len = strlen(buffer);
    
    /* WAP Response Size */
    if (pSrchInfo->iViewType == VIEWTYPE_NORM)
        sprintf(&buffer[len],"WAP-RESPONSE-SIZE                  = [%d]\n",
                pstLogParse->iWapResSize);
    else
        sprintf(&buffer[len],"%d; ", pstLogParse->iWapResSize);
    len = strlen(buffer);
    
    /* Contents Length */
    if (pSrchInfo->iViewType == VIEWTYPE_NORM)
        sprintf(&buffer[len],"CONTENTS-LENGTH                    = [%d]\n",
                pstLogParse->iContentsLen);
    else
        sprintf(&buffer[len],"%d; ", pstLogParse->iContentsLen);
    len = strlen(buffer);
    
    /* ResultCode */
    if (pSrchInfo->iViewType == VIEWTYPE_NORM)
        sprintf(&buffer[len],"RESULT-CODE                        = [%d]\n",
                pstLogParse->uiResultCode);
    else
        sprintf(&buffer[len],"%d; ", pstLogParse->uiResultCode);
    len = strlen(buffer);
    
    
    /* Status */
    if (pSrchInfo->iViewType == VIEWTYPE_NORM)
        sprintf(&buffer[len],"STATUS                             = [%s]\n",
                pstLogParse->szStatus);
    else
        sprintf(&buffer[len],"%s; ", pstLogParse->szStatus);
    len = strlen(buffer);

    /* URL */
    if (pSrchInfo->iViewType == VIEWTYPE_NORM)
        sprintf(&buffer[len],"URL                                = [%s]\n",
                pstLogParse->szURL);
    else
        sprintf(&buffer[len],"%s; ", pstLogParse->szURL);
    len = strlen(buffer);

    /* Method Type */
    if (pSrchInfo->iViewType == VIEWTYPE_NORM)
        sprintf(&buffer[len],"METHOD-TYPE                        = [%s]\n",
                pstLogParse->szMethodType);
    else
        sprintf(&buffer[len],"%s; ", pstLogParse->szMethodType);
    len = strlen(buffer);
    
    /* Method Type */
    if (pSrchInfo->iViewType == VIEWTYPE_NORM)
        sprintf(&buffer[len],"USER-AGENT                         = [%s]\n",
                pstLogParse->szUserAgent);
    else
        sprintf(&buffer[len],"%s; ", pstLogParse->szUserAgent);
    len = strlen(buffer);
    
    /* Txn Log */
    if (pSrchInfo->iViewType == VIEWTYPE_NORM)
        sprintf(&buffer[len],"TXN-LOG                            = [%s]\n", 
                pDbFetch->sTLOG);
    else
        sprintf(&buffer[len],"%s;\n", pDbFetch->sTLOG);
    len = strlen(buffer);

	
	buffer[len] = 0;

#if 1
	if (fprintf(OutFd,"%s",buffer) < 0)
	{
		fprintf(stderr,"ERROR: file write error(%s)!!!\n",strerror(errno));
		exit(1);
	}
#else
	fprintf(stderr,"%s\n",buffer);
#endif

	return 0;
}



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
