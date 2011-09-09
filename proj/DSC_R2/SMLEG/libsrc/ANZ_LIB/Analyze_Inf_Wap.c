/**********************************************************
                 KTF IPAS Project

   Section  : IPAS Project
   SCCS ID  : ANZ_LIB
   Date     : 2006.07.05.
   Revision History :
        '2006.07.05.   Initial

   Description:
        IPAF ETHERNET Analyzing Library

   Copyright (c) uPresto 2006
***********************************************************/

/**A.1*  File Inclusion ***********************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>
#include <stdint.h>

#include <Num_Proto.h>
#include <VJ.h>
#include <Analyze_Ext_Abs.h>
#include <utillib.h>
#include <PPP_header.h>
#include <Ethernet_header.h>
#include <IP_header.h>
#include <TCP_header.h>
#include <UDP_header.h>
#include <IEEE_header.h>
#include <Errcode.h>

#include "Analyze_Inf_Wap.h"

/**B.1*  Definition of New Constants **********************/
/**B.2*  Definition of New Type  **************************/
/**C.1*  Declaration of Variables  ************************/
/**D.1*  Definition of Functions  *************************/

// yhshin
#define TOUSHORT(x) (USHORT)(*(x)<<8|*(x+1))
#define TOULONG(x)  (ULONG)(*(x)<<24|*(x+1)<<16|*(x+2)<<8|*(x+3))
//#define TOUSHORT(x)   ntohs((USHORT)(x))
////#define TOULONG(x)    ntohl((ULONG)(x))
//

#define LOUCHAR(w)	((UCHAR)((USHORT)(w) & 0xff))
#define HIUCHAR(w)  ((UCHAR)((USHORT)(w) >> 8))


/**D.2*  Definition of Functions  *************************/
BOOL AnalyzeIP_UDP_WAP(PANALYZE_INFO_WAP10 pInfo, PUCHAR pucBuffer, WORD wSize, USHORT usAppCode )
{   // 536
	pInfo->bWAP = (BOOL)TRUE;

    switch (usAppCode)
    {   
        case APP_WAP_CON_SESSION:
            pInfo->wtp_num = 0; 
            if (pucBuffer[0] == 0)  // concatenated PDU
                AnalyzeIP_UDP_WAP_CONCAT_WTP( pInfo, pucBuffer, wSize );
            else                    // non-concatenated PDU
                AnalyzeIP_UDP_WAP_WTP_PDU(pInfo, pucBuffer, wSize );   // 532
            break;
#if 0
        case APP_WAP_NON_CON_SESSION:  // 현재 분석 안함. 
            //AnalyzeIP_UDP_WAP_WSP(pInfo, pucBuffer+1, wStart, wSize-1 );
            break;
#endif
        
        default:
			pInfo->bWAP = FALSE;
            break;
    }
    
    return TRUE;
}

void AnalyzeIP_UDP_WAP_CONCAT_WTP(PANALYZE_INFO_WAP10 pInfo, PUCHAR pucBuffer, WORD wSize) 
{
    UCHAR *pData; 
    int   nSize;
    WORD  wLen;

    WTP_CONCATE_HDR *con_hdr = (WTP_CONCATE_HDR *)(pucBuffer + 1);
    nSize = (int)wSize;
    nSize--;
    while (nSize > 0)
    {
        if (con_hdr->short_len.flag)
            wLen = (unsigned short)(con_hdr->long_len.len_1 << 8 | con_hdr->long_len.len_2) & 0x7FFFF;
        else    
            wLen = con_hdr->short_len.len;

        pData = (con_hdr->short_len.flag)? con_hdr->long_len.data : con_hdr->short_len.data;

        nSize -= 1 + (int)con_hdr->short_len.flag;

        AnalyzeIP_UDP_WAP_WTP_PDU(pInfo, pData, wLen);
        nSize -= wLen;
        con_hdr = (WTP_CONCATE_HDR *)(pData + wLen);
    } // end of while

}

void AnalyzeIP_UDP_WAP_WTP_PDU(PANALYZE_INFO_WAP10 pInfo, PUCHAR pucBuffer, WORD wSize ) 
{
    WTP_HDR *wtp_hdr;
    UCHAR type, *pData = NULL; 
    WORD wTPI_size = 0, wWTPHDR_size = 0;

    wtp_hdr = (WTP_HDR *)pucBuffer;

    type = wtp_hdr->octet1.invoke_result.pdu_type;
    switch (type)
    {
        case 0x01:  // Invoke
        case 0x04:  // Abort
        case 0x05:  // Segmented Invoke
        case 0x06:  // Segmented Result
            wWTPHDR_size = 4;
            break;  
        case 0x02:  // Result
        case 0x03:  // Ack  
            wWTPHDR_size = 3;
            break;  
        case 0x07:  // Negative Ack
            wWTPHDR_size = wtp_hdr->octet4.negative_ack.num + 4;
            break;  
    }

    //strcat( pInfo->szMsgType, _get_type_string(type));
    //pInfo->Wtp_type[pInfo->wtp_num] = type; 
    //pInfo->Wtp_type = type; 
	pInfo->WtpPDUType = type;
    //pInfo->wWtpSize[pInfo->wtp_num] = wSize;
    pInfo->wWtpSize = wSize;

    AnalyzeIP_UDP_WAP_WTP_OCTET1(pInfo, wtp_hdr, type);
    wSize--;


    AnalyzeIP_UDP_WAP_WTP_OCTET2_3(pInfo, wtp_hdr );
    wSize -= 2;

    pData = AnalyzeIP_UDP_WAP_WTP_OCTET4(pInfo, wtp_hdr, &wSize, type);

    if (wtp_hdr->octet1.invoke_result.con)
    {
        wTPI_size = AnalyzeIP_UDP_WAP_WTP_TPI(pInfo, pData, wSize);
        wSize -= wTPI_size;

    }


    if (wSize)
    {
        switch (type)
        {
            case 0x05:  // Segmented Invoke
            case 0x06:  // Segmented Result
				/*
                if (pTree)
                    InsertTreeItem(inf, " - Segmented WSP data - ", hRoot, wStart, wSize, TRUE);
				*/

                pInfo->pFragData = pData + wTPI_size;
                pInfo->wFragDataSize = wSize;
                break;
            case 0x01:  // Invoke
            case 0x02:  // Result
            case 0x03:  // Ack
            case 0x04:  // Abort
            case 0x07:  // Negative Ack
                if (pInfo->bWAP)
                {
                    pInfo->pFragData = pData + wTPI_size;
                    pInfo->wFragDataSize = wSize;
                }
                AnalyzeIP_UDP_WAP_WSP(pInfo, pData + wTPI_size, wSize);
                break;

            default:
				pInfo->bWAP = FALSE;
                break;

        }
    }

    pInfo->wtp_num++;
}

void AnalyzeIP_UDP_WAP_WTP_OCTET1(PANALYZE_INFO_WAP10 pInfo, WTP_HDR *wtp_hdr, UCHAR type)
{   
    //UCHAR gtr_ttr = 0xff;
    
    pInfo->bWAP = FALSE;
    switch (type)
    {   
        case 0x01:
        case 0x02:
        case 0x05:
        case 0x06: 
            //pInfo->gtr_ttr[pInfo->wtp_num] = 
            pInfo->gtr_ttr = 
                (UCHAR)((wtp_hdr->octet1.invoke_result.gtr << 1) | wtp_hdr->octet1.invoke_result.ttr);
            pInfo->rid = wtp_hdr->octet1.invoke_result.rid;
            //pInfo->rid[pInfo->wtp_num] = wtp_hdr->octet1.invoke_result.rid;
            if (type == 0x01 || type == 0x02)   // invoke or result
            {   
				// 00 Not last packet
				// 01 Last packet of message
				// 10 Last packet of packet group
				// 11 Segmentation and Re-assembly NOT supported.
				// The default setting should be GTR=1 and TTR=1, 
				// that is, WTP segmentation and re-assembly not supported.
                //if (pInfo->gtr_ttr[pInfo->wtp_num] == 0 || pInfo->gtr_ttr[pInfo->wtp_num] == 2)
                //if (pInfo->gtr_ttr == 0 || pInfo->gtr_ttr == 2)
                if (pInfo->gtr_ttr == 0 || pInfo->gtr_ttr == 3 || pInfo->gtr_ttr == 1 )
                    pInfo->bWAP = TRUE;
            }
            //else    // segmented
            //{
            //    pInfo->bWtpFrag = TRUE;
            //}
            break;
        
        case 0x03: // ACK
            //pInfo->rid[pInfo->wtp_num] = wtp_hdr->octet1.invoke_result.rid;
            pInfo->rid = wtp_hdr->octet1.invoke_result.rid;
			pInfo->bWAP = TRUE;
            break;

		case 0x04:  // ABORT
			pInfo->AbortType = wtp_hdr->octet1.abort.abort_type;
        	pInfo->bWAP = TRUE;
        case 0x07:
            //pInfo->rid[pInfo->wtp_num] = wtp_hdr->octet1.invoke_result.rid;
            pInfo->rid = wtp_hdr->octet1.invoke_result.rid;
			pInfo->bWAP = TRUE;
            break;
        
        default:
            break;
    } /* end of switch */
}

void AnalyzeIP_UDP_WAP_WTP_OCTET2_3(PANALYZE_INFO_WAP10 pInfo, WTP_HDR *wtp_hdr)
{
    pInfo->wTid = TOUSHORT(wtp_hdr->tid) & 0x7FFF; 
}

UCHAR *AnalyzeIP_UDP_WAP_WTP_OCTET4(PANALYZE_INFO_WAP10 pInfo, WTP_HDR *wtp_hdr, WORD *wSize, UCHAR type)
{
    UCHAR *pData;
    int i, num;	
	char szMsg[128];

    switch (type)
    {
        case 1: // Invoke
            /// beacon display...
            //pInfo->tcl[pInfo->wtp_num] = wtp_hdr->octet4.invoke.tcl;
            pInfo->tcl = wtp_hdr->octet4.invoke.tcl;
			if( pInfo->tcl > 0x02 )
			{
				pInfo->bWAP = FALSE;
				(*wSize) = 0;
			}

            (*wSize)--;
            pData = wtp_hdr->octet4.invoke.data;
            break;

        case 2: // Result
        case 3: // Ack
            pData = wtp_hdr->octet4.result_ack.data;
			//(*wSize) = 0;
            break;

        case 4: // Abort
            (*wSize)--;
            pData = wtp_hdr->octet4.abort.data;
			pInfo->AbortReason = wtp_hdr->octet4.abort.reason;
            break;

        case 5: // Segmented Invoke
        case 6: // Segmented Result
            strcat(pInfo->szPSN, szMsg);
            (*wSize)--;
            pData = wtp_hdr->octet4.segment.data;
            break;

        case 7: // Negative Ack
            num = (int)wtp_hdr->octet4.negative_ack.num;
            for (i = 0; i < num; i++)
            {
                sprintf(szMsg, "%u", wtp_hdr->octet4.negative_ack.data[i]);
                if (strlen(pInfo->szPSN) > 0)
                    strcat(pInfo->szPSN, ",");
                strcat(pInfo->szPSN, szMsg);
            }

            (*wSize) -= (WORD)(num+1);
            pData = (wtp_hdr->octet4.segment.data + i);
            break;

        default:
            pData = wtp_hdr->octet4.result_ack.data;
            break;
    }

    return pData;
}

WORD AnalyzeIP_UDP_WAP_WTP_TPI(PANALYZE_INFO_WAP10 pInfo, PUCHAR pucBuffer, WORD wSize)
{
    TPI_HDR *tpi_hdr;
    WORD wTPI_size = 0;
    UCHAR type, con = TRUE, len, *tpi_data;
    char szMsg[256];
    int i;

    tpi_hdr = (TPI_HDR *)pucBuffer;

    while (con)
    {
        con = tpi_hdr->long_tpi.con;
        type = tpi_hdr->long_tpi.type;

        if (type)
        {
            len = tpi_hdr->long_tpi.len;
            tpi_data = tpi_hdr->long_tpi.data;
            wTPI_size += 2 + len;
        }
        else
        {
            len = tpi_hdr->short_tpi.len;
            tpi_data = tpi_hdr->short_tpi.data;
            wTPI_size += 1 + len;
        }

        switch (tpi_hdr->long_tpi.id)
        {
            case 0x02:
                if (tpi_data[0] == 0x04)            /// Max group
                {
                    switch (len - 1)
                    {
                        case 1:
                            sprintf(pInfo->szMaxGrp, "%u", tpi_data[1]);
                            break;

                        case 2:
                            sprintf(pInfo->szMaxGrp, "%u", TOUSHORT(tpi_data + 1));
                            break;

                        case 4:
                            sprintf(pInfo->szMaxGrp, "%ld", TOULONG(tpi_data + 1));
                            break;

                        default:
                            strcat(pInfo->szMaxGrp, "0x");
                            for (i = 1; i < (int)len; i++)
                            {
                                sprintf(szMsg, "%02X", tpi_data[i]);
                                strcat(pInfo->szMaxGrp, szMsg);
                            }
                            break;
                    }
                }
                else if (tpi_data[0] == 0x07)       /// Num group
                {
                    switch (len - 1)
                    {
                        case 1:
                            sprintf(pInfo->szNumGrp, "%u", tpi_data[1]);
                            break;

                        case 2:
                            sprintf(pInfo->szNumGrp, "%u", TOUSHORT(tpi_data + 1));
                            break;

                        case 4:
                            sprintf(pInfo->szNumGrp, "%ld", TOULONG(tpi_data + 1));
                            break;

                        default:
                            strcat(pInfo->szNumGrp, "0x");
                            for (i = 1; i < (int)len; i++)
                            {
                                sprintf(szMsg, "%02X", tpi_data[i]);
                                strcat(pInfo->szNumGrp, szMsg);
                            }
                            break;
                        }
                }
                break;

            case 0x03:  /// PSN
                for (i = 0; i < (int)len; i++)
                {
                    if (strlen(pInfo->szPSN) > 0)
                        strcat(pInfo->szPSN, ",");

                    sprintf(szMsg, "%u", tpi_data[i]);
                    strcat(pInfo->szPSN, szMsg);
                }
                break;

            default:
                break;
        }
    }

    tpi_hdr = (TPI_HDR *)(tpi_data + len);

    return wTPI_size;
}

WORD AnalyzeIP_UDP_WAP_WSP(PANALYZE_INFO_WAP10 pInfo, PUCHAR pucBuffer, WORD wSize)
{
    UCHAR type, *pTemp;
    //char szMsg[128];

    
    pTemp = pucBuffer;
    //while ((int)wSize > 0)
    //{   
        type = pInfo->WspPDUType = pTemp[0];
        pTemp++, wSize--;


        switch (type)
        {
            case 0x01:  // Connect
                pTemp = AnalyzeIP_UDP_WAP_WSP_Connect(pInfo, pTemp, &wSize);
                pInfo->WSPType = 0x01;
                break;
            case 0x02:  // Connect Reply
                pTemp = AnalyzeIP_UDP_WAP_WSP_ConReply(pInfo, pTemp, &wSize);
                break;
            case 0x03:  // Redirect
                pTemp = AnalyzeIP_UDP_WAP_WSP_Redirect(pInfo, pTemp, &wSize);
                break;
            case 0x04:  // Reply
                pTemp = AnalyzeIP_UDP_WAP_WSP_Reply(pInfo, pTemp, &wSize);
                break;
            case 0x05:  // Disconnect
                pTemp = AnalyzeIP_UDP_WAP_WSP_Disconnect(pInfo, pTemp, &wSize);
                pInfo->WSPType = 0x02;
                break;
            case 0x06:  // Push
            case 0x07:  // Push
                pTemp = AnalyzeIP_UDP_WAP_WSP_Push(pInfo, pTemp, &wSize);
                break;  
            case 0x08:  // Suspend
                pTemp = AnalyzeIP_UDP_WAP_WSP_Suspend(pInfo, pTemp, &wSize);
                break;  
            case 0x09:  // Resume
                pTemp = AnalyzeIP_UDP_WAP_WSP_Resume(pInfo, pTemp, &wSize);
                break;  
            case 0x40:  // Get
                pTemp = AnalyzeIP_UDP_WAP_WSP_Get(pInfo, pTemp, &wSize);
                break;  
            case 0x60:  // Post
                pTemp = AnalyzeIP_UDP_WAP_WSP_Post(pInfo, pTemp, &wSize);
                break;  
            case 0x80:  // Fragmented PDU
                pTemp = AnalyzeIP_UDP_WAP_WSP_FragPDU(pInfo, pTemp, &wSize);
                break;
            default:
                wSize = 0;
                break;
        }
    //}
    
    return TRUE;
}

UCHAR *AnalyzeIP_UDP_WAP_WSP_Connect(PANALYZE_INFO_WAP10 pInfo, PUCHAR pucBuffer, WORD *wSize)
{   
    UCHAR 	*pTemp = pucBuffer;
    UCHAR 	ver;
    WORD 	wCaplen=0, wHdrlen=0;
    int 	nCaplen_size, nHdrlen_size; // variable length field size
    //char szMsg[1024];

    ver = pTemp[0];
    pTemp++;
    (*wSize)--;

    nCaplen_size = GetVariableLengthWORD(pTemp, &wCaplen);
    pTemp += nCaplen_size;
    (*wSize) -= nCaplen_size;

    nHdrlen_size = GetVariableLengthWORD(pTemp, &wHdrlen);
    pTemp += nHdrlen_size;
    (*wSize) -= nHdrlen_size;

    /*
    if (pTree)
    {
        _insert_version(szMsg, ver, inf, hRoot, wStart, 1);
        _insert_cap_length(szMsg, wCaplen, inf, hRoot, wStart, nCaplen_size);
        _insert_hdr_length(szMsg, wHdrlen, inf, hRoot, wStart, nHdrlen_size);
        _insert_capabilities(szMsg, wCaplen, inf, hRoot, wStart, pTemp);
        _insert_header(szMsg, wHdrlen, inf, hRoot, wStart, pTemp+wCaplen);
    }
    */

    AnalyzeWSP_Cap( pInfo, pTemp, wCaplen);
	(*wSize) -= wCaplen;
	pTemp += wCaplen;
    AnalyzeWSP_Headers( pInfo, pTemp, wHdrlen);

    (*wSize) -= wHdrlen;
    pTemp += wHdrlen;
    //(*wSize) -= (wCaplen + wHdrlen);
    //pTemp += (wCaplen + wHdrlen);

    return pTemp;
}

UCHAR *AnalyzeIP_UDP_WAP_WSP_ConReply(PANALYZE_INFO_WAP10 pInfo, PUCHAR pucBuffer, WORD *wSize) 
{
    PUCHAR pTemp = pucBuffer;
    DWORD dwId=0;
    WORD wCaplen=0, wHdrlen=0;
    int nIdlen_size, nCaplen_size, nHdrlen_size;    // variable length field size
    //char szMsg[1024];

    nIdlen_size = GetVariableLengthDWORD(pTemp, &dwId);
    pTemp += nIdlen_size; (*wSize) -= nIdlen_size;

    nCaplen_size = GetVariableLengthWORD(pTemp, &wCaplen);
    pTemp += nCaplen_size; (*wSize) -= nCaplen_size;

    nHdrlen_size = GetVariableLengthWORD(pTemp, &wHdrlen);
    pTemp += nHdrlen_size; (*wSize) -= nHdrlen_size;
    
    AnalyzeWSP_Cap( pInfo, pTemp, nHdrlen_size );
    AnalyzeWSP_Headers( pInfo, pTemp, nCaplen_size );

    (*wSize) -= (wCaplen + wHdrlen);
    pTemp += (wCaplen + wHdrlen);

    return pTemp;
}

UCHAR *AnalyzeIP_UDP_WAP_WSP_Redirect(PANALYZE_INFO_WAP10 pInfo, PUCHAR pucBuffer, WORD *wSize)
{
    PUCHAR pTemp = pucBuffer;
    UCHAR Addr[128];
	UCHAR flag, type, Addrlen;
    BOOL bType, bPort;
    WORD Port_num;
    //char szMsg[128];

    flag = pTemp[0];
    pTemp++, (*wSize)--;

    bType = pTemp[0] & 0x80;
    bPort = pTemp[0] & 0x40;
    Addrlen = pTemp[0] & 0x3F;
    pTemp++, (*wSize)--;

    if (bType)
    {
        type = pTemp[0];
        pTemp++; (*wSize)--;
    }

    if (bPort)
    {
        Port_num = TOUSHORT(pTemp);
        pTemp += 2; (*wSize) -= 2;
    }

    //Addr = new BYTE[Addrlen];
    memcpy(Addr, pTemp, Addrlen);
	Addr[Addrlen]=0;

    pTemp += Addrlen; (*wSize) -= Addrlen;
    //delete []Addr;

    return pTemp;
}

UCHAR *AnalyzeIP_UDP_WAP_WSP_Disconnect(PANALYZE_INFO_WAP10 pInfo, PUCHAR pucBuffer, WORD *wSize)
{
    UCHAR *pTemp = pucBuffer;
    DWORD dwId;
    int nIdlen_size;
    //char szMsg[128];

    nIdlen_size = GetVariableLengthDWORD(pTemp, &dwId);
    pTemp += nIdlen_size;
	(*wSize) -= nIdlen_size;

    /*
    if (pTree)
        _insert_session_id(szMsg, dwId, inf, hSub1, wStart, nIdlen_size);
    */

    return pTemp;
}

UCHAR *AnalyzeIP_UDP_WAP_WSP_Reply(PANALYZE_INFO_WAP10 pInfo, PUCHAR pucBuffer, WORD *wSize)
{
    UCHAR *pTemp = pucBuffer;
    //UCHAR status;
    WORD wHdrlen=0;
    int nHdrlen_size;
	int nContent_size;
    //char szMsg[128];

    pInfo->usRetCode = (unsigned short)pTemp[0];
    pTemp++;
	(*wSize)--;

    nHdrlen_size = GetVariableLengthWORD(pTemp, &wHdrlen);
    pTemp += nHdrlen_size;
	(*wSize) -= nHdrlen_size;


    /*
    if (pTree)
    {
        sprintf(szMsg, " Status: %u (%s) ", status, _get_status_string(status));
        InsertTreeItem(inf, szMsg, hSub1, wStart, 1, TRUE);

        _insert_hdr_length(szMsg, wHdrlen, inf, hSub1, wStart, nHdrlen_size);

        //// content, header, data...
        nContent_size = _insert_content(szMsg, pTemp, inf, hSub1, wStart);
        wSize -= nContent_size, pTemp += nContent_size, wHdrlen -= nContent_size;
        _insert_header(szMsg, wHdrlen, inf, hSub1, wStart, pTemp);
        wSize -= wHdrlen, pTemp += wHdrlen;
        _insert_data(szMsg, wSize, inf, hSub1, wStart, pTemp);
        pTemp += wSize;
    }
    else
	*/
	
	nContent_size = _insert_content(pInfo, pTemp);
	//*wSize -= nContent_size;
	pTemp += nContent_size; 
	//wHdrlen -= nContent_size;
	AnalyzeWSP_Headers( pInfo, pTemp, wHdrlen );
		
    //pTemp += *wSize;
	pTemp += wHdrlen;

    (*wSize) -= wHdrlen;
	pInfo->uiDataLen = (*wSize);
	(*wSize) -= pInfo->uiDataLen;

    return pTemp;
}

UCHAR *AnalyzeIP_UDP_WAP_WSP_Get(PANALYZE_INFO_WAP10 pInfo, PUCHAR pucBuffer, WORD *wSize)
{
    UCHAR	*pTemp = pucBuffer;
    WORD 	wUri_len=0;
    int 	nUrilen_size=0;
	WORD	wHdrlen=0;

//	
dAppLog(LOG_CRI, "[TID:%d] 01: SIZE: %d",pInfo->wTid, (*wSize) );
    nUrilen_size = GetVariableLengthWORD(pTemp, &wUri_len);
    pTemp += nUrilen_size; 
	(*wSize) -= nUrilen_size;
//
dAppLog(LOG_CRI, "[TID:%d] 02: SIZE: %d nUrilen:%d", pInfo->wTid, (*wSize), nUrilen_size );

	if( wUri_len > 1024 )
	{
		memcpy( pInfo->szURI, pTemp, 1024 );
		pInfo->szURI[1024] = 0;
	}
	else
	{
		/* FOR INVALID wUri_len */
		if( wUri_len > (*wSize) ) {

			dAppLog( LOG_CRI, "INVALID wUri_len:%d wSize:%d", wUri_len, (*wSize) );

			wUri_len = (*wSize);
			memcpy( pInfo->szURI, pTemp, wUri_len );
			pInfo->szURI[wUri_len] = 0;

			pTemp += wUri_len;

			return pTemp;
		}
		else {
			memcpy( pInfo->szURI, pTemp, wUri_len );
			pInfo->szURI[wUri_len] = 0;
		}
	}

    pTemp += wUri_len;

    (*wSize) -= wUri_len ;
//
dAppLog(LOG_CRI, "[TID:%d] 03: SIZE: %d wUri_len:%d", pInfo->wTid, (*wSize), wUri_len);

	if( (*wSize) > 0 )
	{
		wHdrlen = (*wSize);
		AnalyzeWSP_Headers( pInfo, pTemp, wHdrlen );
		(*wSize) -= wHdrlen;

dAppLog(LOG_DEBUG, "[TID:%d] 04: SIZE: %d wHdrlen:%d", pInfo->wTid, (*wSize), wHdrlen);
	}
	else
	{
		dAppLog( LOG_DEBUG,"[%s] wSize:%d URILen:%d ",
		__FUNCTION__, (*wSize), wUri_len );
	}
	(*wSize) = 0;

    return pTemp;
}

UCHAR *AnalyzeIP_UDP_WAP_WSP_Post(PANALYZE_INFO_WAP10 pInfo, PUCHAR pucBuffer, WORD *wSize)
{
    UCHAR *pTemp = pucBuffer;
    WORD wUrilen=0, wHdrlen=0;
    int nUrilen_size=0, nHdrlen_size=0;
	int  nContent_size=0;

    nUrilen_size = GetVariableLengthWORD(pTemp, &wUrilen);
    pTemp += nUrilen_size;
    (*wSize) -= nUrilen_size;

    nHdrlen_size = GetVariableLengthWORD(pTemp, &wHdrlen);
    pTemp += nHdrlen_size;
    (*wSize) -= nHdrlen_size;

	if( wUrilen > 1024 )
	{
		memcpy( pInfo->szURI, pTemp, 1024-1 );
		pInfo->szURI[1024] = 0;
	}
	else
	{
		memcpy( pInfo->szURI, pTemp, wUrilen );
		pInfo->szURI[wUrilen] = 0;
	}

    pTemp += wUrilen;
	(*wSize) -= wUrilen;

	nContent_size = _insert_content( pInfo, pTemp );	
	pTemp += nContent_size;

	AnalyzeWSP_Headers( pInfo, pTemp, (wHdrlen-nContent_size) );
    pTemp += wHdrlen;

  	(*wSize) -= wHdrlen;
   	pInfo->uiDataLen = (*wSize);
   	(*wSize) -= pInfo->uiDataLen;

    return pTemp;
}


UCHAR *AnalyzeIP_UDP_WAP_WSP_Push(PANALYZE_INFO_WAP10 pInfo, PUCHAR pucBuffer, WORD *wSize)
{
    UCHAR *pTemp = pucBuffer;
    WORD wHdrlen;
    int nHdrlen_size;
    //char szMsg[128];

    nHdrlen_size = GetVariableLengthWORD(pTemp, &wHdrlen);
    pTemp += nHdrlen_size; (*wSize) -= nHdrlen_size;

    pTemp += (*wSize);
    (*wSize) = 0;

    return pTemp;
}


UCHAR *AnalyzeIP_UDP_WAP_WSP_Suspend(ANALYZE_INFO_WAP10 *pResult, PUCHAR pucBuffer, WORD *wSize)
{
    UCHAR *pTemp = pucBuffer;
    DWORD dwId;
    int nIdlen_size;
    //char szMsg[128];

    nIdlen_size = GetVariableLengthDWORD(pTemp, &dwId);
    pTemp += nIdlen_size; (*wSize) -= nIdlen_size;

    /*
    if (pTree)
        _insert_session_id(szMsg, dwId, inf, hSub1, wStart, nIdlen_size);
    */

    pTemp += (*wSize);
    (*wSize) = 0;

    return pTemp;
}

UCHAR *AnalyzeIP_UDP_WAP_WSP_Resume(PANALYZE_INFO_WAP10 pInfo, PUCHAR pucBuffer, WORD *wSize )
{
    UCHAR *pTemp = pucBuffer;
    DWORD dwId=0;
    WORD wCaplen=0;
    int nIdlen_size=0, nCaplen_size=0;
	WORD wHdrlen=0;
    //char szMsg[128];

    nIdlen_size = GetVariableLengthDWORD(pTemp, &dwId);
    pTemp += nIdlen_size; (*wSize) -= nIdlen_size;

    nCaplen_size = GetVariableLengthWORD(pTemp, &wCaplen);
    pTemp += nCaplen_size; (*wSize) -= nCaplen_size;

    /*
    if (pTree)
    {
        _insert_session_id(szMsg, dwId, inf, hSub1, wStart, nIdlen_size);
        _insert_cap_length(szMsg, wCaplen, inf, hSub1, wStart, nCaplen_size);
        _insert_capabilities(szMsg, wCaplen, inf, hSub1, wStart, pTemp);
        _insert_header(szMsg, wSize - wCaplen, inf, hSub1, wStart, pTemp+wCaplen);
    }
    */


    AnalyzeWSP_Cap( pInfo, pTemp, wCaplen);

	wHdrlen = (*wSize)-wCaplen;
    AnalyzeWSP_Headers( pInfo,pTemp, wHdrlen);

    pTemp += (*wSize);
    (*wSize) = 0;

    return pTemp;
}

UCHAR *AnalyzeIP_UDP_WAP_WSP_FragPDU(PANALYZE_INFO_WAP10 pInfo, PUCHAR pucBuffer, WORD *wSize)
{
    UCHAR *pTemp = pucBuffer;
    WORD wHdrlen=0;
    int nHdrlen_size;
    //char szMsg[1024];

    nHdrlen_size = GetVariableLengthWORD(pTemp, &wHdrlen);
    pTemp += nHdrlen_size; (*wSize) -= nHdrlen_size;

    /*
    if (pTree)
    {
        _insert_hdr_length(szMsg, wHdrlen, inf, hSub1, wStart, nHdrlen_size);
        _insert_header(szMsg, wHdrlen, inf, hSub1, wStart, pTemp);
        _insert_data(szMsg, wSize - wHdrlen, inf, hSub1, wStart, pTemp+wHdrlen);
    }
    */

	AnalyzeWSP_Headers( pInfo,pTemp, wHdrlen);

    pTemp += (*wSize);
    (*wSize) = 0;

    return pTemp;
}



UCHAR *AnalyzeWSP_Cap(PANALYZE_INFO_WAP10 pInfo, PUCHAR pucBuffer, int nSize)
{
    WORD wLen, wTemp;
    int nLen_size;
    //char szMsg[128];
    UCHAR id;


    while( nSize > 0 )
    {

        nLen_size = GetVariableLengthWORD(pucBuffer, &wLen);
        id = pucBuffer[nLen_size] & 0x7F;

        switch (id)
        {
            case 0x00:  // Client SDU Size
            case 0x01:	// Server SDU Size
            case 0x08:
            case 0x09:
                GetVariableLengthWORD(pucBuffer+1, &wTemp);
                break;

            case 0x02:	// Protocol Option
                break;

            case 0x03: // Method MOR
            case 0x04: // Push MOR
                break;

            case 0x05: //PDU Type
                break;

            case 0x06:  // Page Code
                break;

            case 0x07:
            default:
                break;
        }

        pucBuffer += wLen;
        nSize -= (nLen_size + wLen);
    } /* end of while */

    return pucBuffer;
}

void AnalyzeWSP_Headers(PANALYZE_INFO_WAP10 pInfo, PUCHAR pucBuffer, int nSize)
{
    int field_ret, value_ret, len;
    //WORD wTemp;
    //char szMsg[256], szTemp[256];
    const char *szFieldName;
    DWORD dwField, dwValue;
	char *szVal;
	//char tmpbuf[128];

    while (nSize > 0)
    {
        // Shift-sequence
        if ((pucBuffer[0] == 127) || (pucBuffer[0] > 0 && pucBuffer[0] < 32))
        {
            if (pucBuffer[0] == 127)  // Shift-delimiter
            {
                pucBuffer += 2;
                nSize -= 2;
            }
            else
            {
                pucBuffer++;
                nSize--;
            }
            continue;
        }

        // field name
        field_ret = _decode(pucBuffer, &dwField, &len);
        switch (field_ret)
        {
            case RET_IS_STRING:
                szFieldName = (char*)pucBuffer;
                break;
            case RET_IS_LONG_INT:
                //while (pucBuffer len)
                break;
            case RET_IS_UINTVAR:
            case RET_IS_SHORT_INT:
                //szFieldName = g_tFieldName.FindStr(dwField);
                break;
        }

        pucBuffer += len;
        nSize -= len;

		/* MODIFIED BY LDH 2006.10.12 */
		if( nSize <= 0 )
			break;

        // field value
        value_ret = _decode(pucBuffer, &dwValue, &len);
        switch (value_ret)
        {
            case RET_IS_LONG_INT:
            {
                len = *pucBuffer;
                DWORD wVal;
                //const char *szVal;
                switch (dwField)
                {
                    case FN_ACCEPT:             // Accept
                    {
                        szVal = pGetMediaType(*(pucBuffer+1)&0x7F);
                        if (szVal)
                        {
                            if (len > 1)
                            {
                                int param_len, param_ret;
                                param_ret = _decode(pucBuffer+2, &dwValue, &param_len);
                                switch (param_ret)
                                {
                                    case RET_IS_SHORT_INT:
                                        switch (dwValue)
                                        {
                                            case 0x03:      // Type
                                                param_ret = _decode(pucBuffer+3, &dwValue, &param_len);
                                                switch (param_ret)
                                                {
                                                    case RET_IS_LONG_INT:
                                                        dwValue = _get_guintvar(pucBuffer+3);
                                                        break;

                                                    case RET_IS_SHORT_INT:
                                                        break;
                                                }
                                                break;
                                        }
                                        break;
                                }
                            }
                        }
                        else
                        break;
                    }
                    case FN_ACCEPT_CHARSET_DEP: // Accept-Charset
                        wVal = _get_guintvar(pucBuffer+1);
                        szVal = pGetCharacterSet(wVal);
                        break;
                    case FN_ACCEPT_APPLICATION: // Accept-Application
                        wVal = _get_guintvar(pucBuffer+1);
                        szVal = pGetApplicationIds(wVal);
                        break;
					case FN_DATE:	// Date
						memcpy( &pInfo->uiDate, &pucBuffer[1], len ); 
						pInfo->uiDate = (unsigned int)ntohl(pInfo->uiDate);
						break;
					case FN_CONTENT_LENGTH:  // Content-Length
						if(dwValue)
						{
							pInfo->uiContentLen = dwValue;
						}
						else
						{
							memcpy( &pInfo->uiContentLen, &pucBuffer[1], len );
							if( len == 2 )  // short 
								pInfo->uiContentLen = (unsigned int)ntohs(pInfo->uiContentLen);
							else if( len == 4 ) // int
								pInfo->uiContentLen = (unsigned int)ntohl(pInfo->uiContentLen);
							else
								pInfo->uiContentLen = 0;
						}
						break;
                    default:
                        break;
                }
                pucBuffer += len+1;
                nSize -= (len+1);
                break;
            }
            case RET_IS_UINTVAR:
            {
                pucBuffer += len;
                nSize -= len;

                DWORD val;
                int len2;
                PUCHAR end = pucBuffer + dwValue;
                switch (_decode(pucBuffer, &val, &len2))
                {
                    case RET_IS_STRING:
                        break;
                }
                pucBuffer += len2;
                nSize -= len2;

                // parameters
                while (pucBuffer < end)
                {
                    // param name
                    switch (_decode(pucBuffer, &val, &len2))
                    {
                        case RET_IS_STRING:
                            //sprintf(szMsg, " Value: %s=", pucBuffer);
                            break;
                        default:
                            //strcpy(szMsg, " Value: ");
							break;
                    }

                    pucBuffer += len2;
                    nSize -= len2;

					/* MODIFIED BY LDH 2006.10.12 */
					if( nSize <= 0 )
						break;

                    // param value
                    int len3;
                    switch (_decode(pucBuffer, &val, &len3))
                    {
                        case RET_IS_STRING:
                            //sprintf(szTemp, "%s ", pucBuffer);
                            break;
                        case RET_IS_LONG_INT:
                            //sprintf(szTemp, "%u ", _longint(pucBuffer));
                            break;
                        case RET_IS_SHORT_INT:
                            //sprintf(szTemp, "%u ", val);
                            break;
                    }
                    pucBuffer += len3;
                    nSize -= len3;

					/* MODIFIED BY LDH 2006.10.12 */
					if( nSize <= 0 )
						break;
                }
                break;
            }
            case RET_IS_STRING:
            {
                len = (strlen((char *)pucBuffer))+1;

				if( (nSize > 0 ) && (nSize-len) < 0 )
				{
					len = nSize;
				}
	
				if( dwField == FN_USER_AGENT )
				{	
					memcpy( pInfo->szUserAgent, pucBuffer, len );
					pInfo->szUserAgent[len]=0;
				}
				else if( dwField == FN_SERVER )
				{
					// if you wanna get server info.. (header field type :38 )
				}

                pucBuffer += len;
                nSize -= len;
                break;
            }
            case RET_IS_SHORT_INT:
            {
                const char *szVal;
                switch (dwField)
                {
                    case FN_ACCEPT: // Accept
                    {
                        int len;
                        DWORD val;
                        switch (_decode(pucBuffer, &val, &len))
                        {
                            case RET_IS_SHORT_INT:
                                szVal = pGetMediaType(val);
                                break;
                            case RET_IS_LONG_INT:
                            {
                                int len2;
                                DWORD val2;
                                pucBuffer++;
                                PUCHAR end = pucBuffer + len;
                                while (pucBuffer < end)
                                {
                                    switch (_decode(pucBuffer, &val2, &len2))
                                    {
                                        case RET_IS_STRING:
                                        {
                                            break;
                                        }
                                    }
                                    pucBuffer += len2;
                                }
                                break;
                            }
                            default:
                                //sprintf(szMsg, " Value: 0x%02X ", *pucBuffer);
                                break;
                        }
                        break;
                    }
                    case FN_ACCEPT_CHARSET_DEP: // Accept-Charset
                        szVal = pGetCharacterSet((*pucBuffer)&0x7F);
                        break;
                    case FN_ACCEPT_LANGUAGE:    // Accept-Language
                        szVal = pGetLanguages((*pucBuffer)&0x7F);
                        break;
                    case FN_ACCEPT_APPLICATION: // Accept-Application
                        szVal = pGetApplicationIds((*pucBuffer)&0x7F);
                        break;
                    case FN_BEARER_INDICATION:  // Bearer-Indication
                        szVal = pGetBearerType((*pucBuffer)&0x7F);
						break;
                    default:
                        break;
                }
                pucBuffer++;
                nSize--;
                break;
            }
        }

    }
}


int GetVariableLengthWORD(UCHAR *pData, WORD *wLen)
{
    int i = 0;
    *wLen = 0;
    WORD wTemp = 0;

    for (; pData[i] & 0x80; i++)
    {
        wTemp = (WORD)(pData[i] & 0x7F);
        (*wLen) = ((*wLen) + wTemp) * 0x80;
    }
    (*wLen) += (WORD)pData[i] & 0x7F;

    return i+1;
}

int GetVariableLengthDWORD(UCHAR *pData, DWORD *dwLen)
{
    int i = 0;
    *dwLen = 0;
    DWORD dwTemp = 0;

    for (; pData[i] & 0x80; i++)
    {
        dwTemp = (DWORD)(pData[i] & 0x7F);
        (*dwLen) = (*dwLen + dwTemp) * 0x80;
    }
    (*dwLen) += (DWORD)(pData[i] & 0x7F);

    return i+1;
}

int _decode(PUCHAR pVal, DWORD *dwVal, int *nLen)
{
    if (*pVal & 0x80)
    {
        *dwVal = (*pVal & 0x7F);
        *nLen = 1;
        return RET_IS_SHORT_INT;
    }

    if (*pVal < 31)
    {
        *nLen = *pVal + 1;
        return RET_IS_LONG_INT;
    }

    if (*pVal == 31)
    {
        (*dwVal) = _uintvar(pVal+1, nLen);
        (*nLen)++;
        return RET_IS_UINTVAR;
    }

    *nLen = strlen((char*)pVal) + 1;

    return RET_IS_STRING;
}

PUCHAR pGetMediaType(UCHAR id)
{
	int i;

	for( i=0; i < MAX_CONTENT_TYPE_CNT; i++ )
	{
		if( id == content_type[i].id )
			return (PUCHAR)content_type[i].media_type;
	}
	
	return (PUCHAR)0;
}

PUCHAR pGetCharacterSet(WORD id)
{
	int i;

	for( i=0; i < MAX_CHAR_SET_CNT; i++ )
	{
		if( id == character_set[i].id )
			return (PUCHAR)character_set[i].character_set;
	}

	return (PUCHAR)0;
}

PUCHAR pGetLanguages(UCHAR id)
{
	int i;

	for( i=0; i < MAX_LANGUAGES_CNT; i++ )
	{
		if( id == languages[i].id )
			return (PUCHAR)languages[i].languages;
	}

	return (PUCHAR)0;
}


PUCHAR pGetApplicationIds(UCHAR id)
{
	int i;

	for( i=0; i < MAX_APP_ID_CNT; i++ )
	{
		if( id == application_ids[i].id )
			return (PUCHAR)application_ids[i].application;
	}

	return (PUCHAR)0;
}


PUCHAR pGetBearerType(UCHAR id)
{
	int i;

	for( i=0; i < MAX_APP_ID_CNT; i++ )
	{
		if( id == bearer_type[i].id )
			return (PUCHAR)bearer_type[i].bearer_type;
	}

	return (PUCHAR)0;
}


DWORD _get_guintvar(PUCHAR tvb)
{
    DWORD value = 0;
    DWORD octet;
    PUCHAR end = tvb + tvb[0];
    tvb++;

    while (tvb <= end)
    {
        value <<= 8;
        octet = *tvb;
        value += octet;
        tvb++;
    }
    return value;
}

DWORD _uintvar(PUCHAR pVal, int *nLen)
{   
    DWORD value = 0;
    UCHAR cont = 0x80;
    *nLen = 0;
    while (cont)
    {   
        cont = *pVal & 0x80;
        value <<= 7;
        value += (*pVal & 0x7F);
        pVal++;
        *nLen++; 
    }   
    return value;
}   


int _insert_content(PANALYZE_INFO_WAP10 pInfo, UCHAR *pTemp)
{
    int 	nSize=0;
    DWORD 	dwVal=0;
	PUCHAR  szCType=0;

    switch (_decode(pTemp, &dwVal, &nSize))
    {
        case RET_IS_SHORT_INT:
        {
            szCType = pGetMediaType(dwVal & 0x7F);
            if (szCType)
				sprintf(pInfo->szContentType, szCType );
            else
				sprintf(pInfo->szContentType,"%d:undefined", dwVal );
            break;
        } 
        case RET_IS_STRING:
        {
            //sprintf(szMsg, " Content-Type: %s ", pTemp);
			sprintf(pInfo->szContentType, pTemp );
            break;
        }
        case RET_IS_LONG_INT:
        {
            //const char *szCType = g_tContentType.FindStr(pTemp[1]&0x7F);
			szCType = pGetMediaType(pTemp[1] & 0x7F );
			sprintf( pInfo->szContentType, szCType );
        
            if (pTemp[0] > 1)
            {
                PUCHAR pStart = pTemp+2;
                PUCHAR pEnd = pTemp + pTemp[0];
                const char *szParam;
                while (pStart <= pEnd)  // parameters
                {
                    if (*pStart & 0x80)
                    {
                        UCHAR id = *pStart & 0x7F;
                        switch (id)
                        {
                            case 0x01:
                            {
                                DWORD val = 0;
                                int len = 0;
                                switch (_decode(pStart+1, &val, &len))
                                {
                                    case RET_IS_SHORT_INT:
                                        szParam = pGetCharacterSet(val);
                                        if (szParam)
                                        {
											// if you wanna get character set
                                        }
										pStart += len;
										nSize += len;

                                        break;
                                                  
                                    case RET_IS_STRING:
										// if you wanna get character set
										pStart += len+1;
                                        nSize += len+1;
                                        break;  
                                }

								if( pStart > pEnd )
									return 0;
								       
                                break;  
                            }       
                            default:
								pStart += strlen((char *)pStart)+1;
                        		nSize += strlen((char *)pStart)+1;
                                break;  
                        }       
                    }       
                    else if (*pStart == 0 || *pStart >= 20)
                    {       
                        pStart += strlen((char *)pStart)+1;
                        nSize += strlen((char *)pStart)+1;
                    }       
                    else    
                    {
						pStart += strlen((char *)pStart)+1;
                        nSize += strlen((char *)pStart)+1;       
                    }       
                }       
            }       
        }       
    }       

    return nSize;
}
