// Analyze_Inf2.c 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>
#include <stdint.h>
#include <ctype.h>

#include "Analyze_Ext_Abs.h"
#include "PPP_header.h"
#include "Ethernet_header.h"
#include "IP_header.h"
#include "TCP_header.h"
#include "commdef.h"

#include "Errcode.h"
#include "loglib.h"

int Analyze_A11_CVSE(PUCHAR pBuf, WORD wSize, INFO_A11 *pInfo);
int Analyze_A11_NVSE(PUCHAR pBuf, WORD wSize, INFO_A11 *pInfo);
int dump_DebugString(char *debug_str, char *s, int len);

static void *p;


#define WIDTH   16
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

/*************************************************
func   : AnalyzeEth
def    : AnalyzeEth main function

MAC Frame을 분석하여 종류에 상관 없이 IP Frame으로 
뽑아 낸다.

*************************************************/
int Analyze_Eth(PUCHAR pBuf, WORD wSize, INFO_ETH *pInfo)
{
	int	dResult = 1;
	PETHERNET_HDR   pEthFrame;

	pEthFrame = (PETHERNET_HDR)pBuf;
	p = (void *)pBuf;

	/* ethernet length check */
	if(wSize < MIN_ETHERNET_SIZE || wSize > MAX_ETHERNET_SIZE)
		return ERR_200101;

	/* MAC address */
	memcpy(pInfo->stMAC.strDestMACAddr, pEthFrame->Destination, 6);
	memcpy(pInfo->stMAC.strSrcMACAddr, pEthFrame->Source, 6);

	pInfo->wDataLength = 0;

	/* mac type or ieee802.3 length field */
	pInfo->wFrameType = TOUSHORT(pEthFrame->Protocol);

	/* in case of the mac type of 0x0800, that is, ip datagram,
	   analysizing the packet in more detail.  */
	if (pInfo->wFrameType <= 0x05DC)
	{
		/* 802.3 */
		/* length check, wFrameType is length */	
#if 0
		if(pInfo->wFrameType < (MIN_ETHERNET_SIZE - (sizeof(ETHERNET_HDR)-1)) )
			pInfo->wPaddingSize = MIN_ETHERNET_SIZE - (sizeof(ETHERNET_HDR)-1) - pInfo->wFrameType;

		if(wSize != (sizeof(ETHERNET_HDR)-1) + pInfo->wFrameType + pInfo->wPaddingSize)
			return ERR_200102;
#endif
		// from RP anzlib (temporary code)
        pInfo->dPaddingSize = wSize - (sizeof(ETHERNET_HDR)-1) - pInfo->wFrameType;

        if( pInfo->dPaddingSize < 0 )
            return ERR_200102;

		pInfo->st802_3.b802_3 = TRUE;
		pInfo->st802_3.DSAP = pEthFrame->Data[0];
		pInfo->st802_3.SSAP = pEthFrame->Data[1];

		/* BPDU (Bridge Protocol Data Unit) */
		if (pEthFrame->Data[0] == 0x42 && pEthFrame->Data[1] == 0x42)
		{	/* BPDU (Bridge Protocol Data Unit) */
		}
		else if (pEthFrame->Data[0] == 0x06 && pEthFrame->Data[1] == 0x06)
		{
			dResult = Analyze_IP(&pEthFrame->Data[2], wSize-(sizeof(ETHERNET_HDR)-1)-2, pInfo);
		}
		else if (pEthFrame->Data[0] == 0xF0 && pEthFrame->Data[1] == 0xF0)
		{ 	/* NetBios */
		}
	} /***** end of 802.3 *******/
	else if (pInfo->wFrameType == PROTO_IP)
	{
		dResult = Analyze_IP(pEthFrame->Data, wSize-(sizeof(ETHERNET_HDR)-1), pInfo);
	}
	else if (pInfo->wFrameType == PROTO_ARP)
	{
	}
	else if (pInfo->wFrameType == PROTO_RARP)
	{
	}
	else if (pInfo->wFrameType == PPP_LCP)
	{
	}
	else if (pInfo->wFrameType == PPP_PAP)
	{
	}
	else if (pInfo->wFrameType == PPP_IPCP)
	{
	}
	else if (pInfo->wFrameType == PPP_CCP1
			|| pInfo->wFrameType == PPP_CCP2)
	{
	}
	else if (pInfo->wFrameType == PPP_CHAP)
	{
	}

	return dResult;
} /**** end of Analyze_Eth *****/

/*************************************************
func   : AnalyzeIPOptions()
def    : Analyze a IP Option protocol
 *************************************************/
int AnayzeIPOptions(PUCHAR pBuffer, WORD wSize, INFO_ETH *pInfo)
{
	PIP_OPTION opt = (PIP_OPTION)pBuffer;

	pInfo->stIP.ucOption = opt->Type_Type;

	return TRUE;
}

/*************************************************
func   : Analyze_IP()
def    : Analyze a IP protocol

IP  datagram의 상위 프로토콜 구분
ICMP | TCP | 
 *************************************************/
int Analyze_IP(PUCHAR pucBuffer, WORD wSize, INFO_ETH *pInfo)
{
	int			dRet;
	PIP_RHDR pIP = (PIP_RHDR)pucBuffer;
	WORD wIPHeaderLen, wTotalIPLen, wIPDataLen;
	WORD wPad;
	UCHAR PSEUDO[12];
	short sLength;

	if (pIP->Version != 4)
		return ERR_300103;

	wTotalIPLen = TOUSHORT(pIP->Length);

	if (wSize < wTotalIPLen)
		return ERR_300101;

	wIPHeaderLen = pIP->IHL << 2;
	pInfo->stIP.wTotalLength = wTotalIPLen;

	if((wIPHeaderLen < 20) || (wTotalIPLen < wIPHeaderLen))
		return ERR_300102;

	pInfo->stIP.bIPHeader = TRUE;

	wIPDataLen = wTotalIPLen - wIPHeaderLen;

	pInfo->stIP.Timelive = pIP->Timelive;

	/* TOS */
	pInfo->stIP.TOS = pIP->TOS;

	/* version (4 bits) */
	pInfo->stIP.IPVersion = pIP->Version;


	/* header length (4 bits) */
	pInfo->stIP.wIPHeaderLen = wIPHeaderLen;

	pInfo->stIP.ucProtocol = pIP->Protocol;

	/* IP header checksum */
	if(Checksum(pIP, wIPHeaderLen) != 0)
	{
		pInfo->stIP.bChecksumErr = 1;
		return 0;
	}

	pInfo->stIP.dwSrcIP = TOULONG(pIP->Source);
	pInfo->stIP.dwDestIP = TOULONG(pIP->Destination);


	/* IP Options */
	if (wIPHeaderLen > 20)
		AnayzeIPOptions(pIP->Options, wIPHeaderLen-20, pInfo);

#if 0
	/* 
	 * AQUA2에서는 fragment이면서 
	 */

	/* IP Flags */
	if( (pInfo->stIP.ucProtocol == 17) && 
		((pIP->Flagoff[0] & 0x40) == 0) &&
		(TOUSHORT(pIP->Flagoff) & 0x1fff)    )  
		return 0;
#endif

	/* IP Flags */
	pInfo->stIP.usIdent = TOUSHORT(pIP->Ident);
	pInfo->stIP.usIPFrag = TOUSHORT(pIP->Flagoff);

	if (pInfo->stIP.ucProtocol == 6 || pInfo->stIP.ucProtocol == 17)
	{
		memcpy(PSEUDO, pIP->Source, 4);
		memcpy(PSEUDO+4, pIP->Destination, 4);
		PSEUDO[8] = 0;
		PSEUDO[9] = pIP->Protocol;
		sLength = htons(wIPDataLen);
		memcpy(PSEUDO+10, &sLength, 2);
	}

	wPad = 0;

	if (wSize > wTotalIPLen)
		wPad = wSize - wTotalIPLen;

    if (wTotalIPLen > wSize)    /* wrong packet */
    	wIPDataLen = wSize - wIPHeaderLen;
    else
    	wIPDataLen = wTotalIPLen - wIPHeaderLen;

    switch (pInfo->stIP.ucProtocol)
    {
    	case 1:     /* ICMP */
			dRet = AnalyzeIP_ICMP(pucBuffer+wIPHeaderLen, wIPDataLen, pInfo);
    		break;
		case 4:		/* IP (IP over IP) */
			dRet = Analyze_IP(pucBuffer+wIPHeaderLen, wIPDataLen, pInfo);
			break;
    	case 6:     /* TCP */
			dRet = AnalyzeIP_TCP(pucBuffer+wIPHeaderLen, wIPDataLen, pInfo, (char*)PSEUDO);
    		break;
		case 17:    /* UDP */
			if((pInfo->stIP.usIPFrag & 0x1FFF) == 0) {
				dRet = Analyze_IP_UDP(pucBuffer+wIPHeaderLen, wIPDataLen, pInfo);
			}
			else {
				dRet = 0;
			}
			break;
		case 47:	/* GRE */
			dRet = Analyze_IP_GRE( pucBuffer+wIPHeaderLen, wIPDataLen, pInfo );
			break;
    	default:    // other IP protocol
    		dRet = 0;
    		break;
    }

	return dRet;
}

/*************************************************
func   : AnalyzeIP_TCP()
def    : Analyze a TCP protocol

TCP Header 구성 

 *************************************************/
int AnalyzeIP_TCP(PUCHAR pucBuffer, WORD wSize, INFO_ETH *pInfo, char *pseudo)
{
	WORD wTCPHeaderLen, wTCPDataLen;
	UCHAR   uBuf[2000];

	PTCP_RHDR pTCP = (PTCP_RHDR)pucBuffer;
	wTCPHeaderLen = pTCP->Offset << 2;

	/* TCP check */
	pInfo->stUDPTCP.dUDPTCP = 2;

	/* TCP checksum */
	memcpy(uBuf, pseudo, 12);
	memcpy(uBuf+12, pTCP, wSize);
	if(Checksum(uBuf, 12+wSize) != 0)
	{
		pInfo->stUDPTCP.bChecksumErr = 1;
		return 0;
	}

	/* Control Type */
	pInfo->stUDPTCP.nControlType = pTCP->CB;

	/* Source Port */
	pInfo->stUDPTCP.wSrcPort = TOUSHORT(pTCP->Source);

	/* Destination Port */
	pInfo->stUDPTCP.wDestPort = TOUSHORT(pTCP->Destination);


	if (wSize < wTCPHeaderLen)
		return ERR_370102;

	wTCPDataLen = wSize - wTCPHeaderLen;

	/* for TCP session trace */
	pInfo->stUDPTCP.seq = TOULONG(pTCP->Seq);
	pInfo->stUDPTCP.ack = TOULONG(pTCP->Ack);
	pInfo->stUDPTCP.window = TOUSHORT(pTCP->Window);
	pInfo->stUDPTCP.mss = 0;

	/* Thanks Tundra 2003.01.02. */
	if(wTCPHeaderLen > 20)
		AnalyzeIP_TCP_Option(pTCP->Options, wTCPHeaderLen - 20, pInfo);

	pInfo->stUDPTCP.wHeaderLen 	= wTCPHeaderLen;
	pInfo->stUDPTCP.wDataLen 	= wTCPDataLen;

	pInfo->wDataLength = wTCPDataLen;

	if(wTCPDataLen)
	{
/**
		memcpy(pInfo->szData, pucBuffer + wTCPHeaderLen, wTCPDataLen);
**/
		pInfo->offset = anz_offset(p, (pucBuffer + wTCPHeaderLen));
	}


	return TRUE;
}

/*************************************************
func   : AnalyzeIP_TCP_Option()
def    : Analyze a TCP Option protocol

TCP Header 구성

 *************************************************/
void AnalyzeIP_TCP_Option(PUCHAR pucBuffer, WORD wSize, INFO_ETH *pInfo)
{
	WORD i = 0;
	UCHAR kind, len;

	while( i < wSize )
	{
		kind = pucBuffer[i];
		switch( kind )
		{
			case 0: // end of option
				return;

			case 1: // No operation
				i++;
				break;

			case 2: // Maximum Segment Size (4bytes)
				if( i+4 > wSize ) // Overflow check
					return;
				len = pucBuffer[i+1];
				if( len != 4 ) // Length check
					return;
				pInfo->stUDPTCP.mss = ntohs(*(WORD *)(pucBuffer+i+2));
				i += len;
				break;

			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
			case 11:
			case 12:
			case 13:
			case 14:
			case 15:
			case 18:
			case 19:
				// Options not to analyze.
				// Checking length for the analysis of next option.
				if( i+2 > wSize )
					return;
				len = pucBuffer[i+1];
				if( len <= 0 ) // legnth check
					return;
				i += len;
				break;

			default:
				// Options that cannot be analyzed in current version.
				return;
		}
	}
}

/*************************************************
func   : Analyze_IP_UDP()
def    : Analyze a UDP protocol

UDP Header 구성 

 *************************************************/ 
#if 0
int Analyze_IP_UDP( PUCHAR pBuf, WORD wSize, INFO_ETH *pInfo )
{
	int         dResult = 1;
	PUDP_RHDR   pUDP = (PUDP_RHDR)pBuf;
	WORD      	wUDPHeaderLen, wUDPDataLen;

	UCHAR		uBuf[2000];
	PSU_RHDR	pseudo;

	/* UDP packet size check */
	wUDPHeaderLen = sizeof(UDP_RHDR)-1;
	if( wSize < wUDPHeaderLen)
		return ERR_360101;

	/* UDP check */
	pInfo->stUDPTCP.dUDPTCP = 1;

	/* Source Port */
	pInfo->stUDPTCP.wSrcPort = TOUSHORT(pUDP->Source);

	/* Destination Port */
	pInfo->stUDPTCP.wDestPort = TOUSHORT(pUDP->Destination);

	wUDPDataLen = wSize - wUDPHeaderLen;
	pInfo->stUDPTCP.wDataLen = wUDPDataLen;

	/* UDP checksum */
	memcpy( pseudo.Source, &pInfo->stIP.dwSrcIP, 4 );
	memcpy( pseudo.Destination, &pInfo->stIP.dwDestIP, 4);
	pseudo.Zero = 0x0;
	pseudo.Protocol = pInfo->stIP.ucProtocol;
	memcpy(pseudo.Length, &wSize, 2);

	memcpy( uBuf, &pseudo, 12 );
	memcpy( uBuf+12, pUDP, wSize );
	if( Checksum(uBuf, 12+wSize) != 0 )
		pInfo->stUDPTCP.bChecksumErr = 1;

	pInfo->stUDPTCP.wDataLen = wUDPDataLen;
	pInfo->wDataLength = wUDPDataLen;

	if(wUDPDataLen)
	{
		memcpy(pInfo->szData, pBuf + wUDPHeaderLen, wUDPDataLen);
	}


	//memcpy (pInfo->szData, pUDP->Data, 1);

	return dResult;
} /**** Analyze_IP_UDP ******/
#endif
int Analyze_IP_UDP( PUCHAR pBuf, WORD wSize, INFO_ETH *pInfo )
{
    int         dResult = 1;
    PUDP_RHDR   pUDP = (PUDP_RHDR)pBuf;
    WORD        wUDPHeaderLen, wUDPDataLen;
    DWORD       temp;

    UCHAR       uBuf[DATAMAXSIZE+12];
    PSU_RHDR    pseudo;
    //char      PSEUDO[12];
    //USHORT        sLength;

    wUDPHeaderLen = sizeof(UDP_RHDR)-1;
	pInfo->offset = anz_offset(p, (pBuf + wUDPHeaderLen));

    /* UDP check */
    pInfo->stUDPTCP.dUDPTCP = 1;

    /* Source Port */
    pInfo->stUDPTCP.wSrcPort = TOUSHORT(pUDP->Source);

    /* Destination Port */
    pInfo->stUDPTCP.wDestPort = TOUSHORT(pUDP->Destination);


    /* UDP packet size check */
    if( wSize < wUDPHeaderLen)
        return ERR_360101;

    wUDPDataLen = wSize - wUDPHeaderLen;
    pInfo->stUDPTCP.wDataLen = wUDPDataLen;

    /* UDP checksum */
	/* If UDP checksum field is zero in IPv4, UDP checksuming is disabled. */
	if( pUDP->Checksum[0] != 0 || pUDP->Checksum[1] != 0 )
	{
		temp = htonl(pInfo->stIP.dwSrcIP);
		memcpy( pseudo.Source, &temp, 4 );
		temp = htonl(pInfo->stIP.dwDestIP);
		memcpy( pseudo.Destination, &temp, 4);
		pseudo.Zero = 0x0;
		pseudo.Protocol = pInfo->stIP.ucProtocol;
		pseudo.Length[0] = HIUCHAR(wSize);
		pseudo.Length[1] = LOUCHAR(wSize);
		memcpy( uBuf, &pseudo, 12 );
		memcpy( uBuf+12, pUDP, wSize );

		if( Checksum(uBuf, 12+wSize) != 0 )
		{
			pInfo->stUDPTCP.bChecksumErr = 1;
			return 0;
		}
	}

    pInfo->stUDPTCP.bChecksumErr = 0;
	pInfo->stUDPTCP.wHeaderLen = wUDPHeaderLen;
    pInfo->stUDPTCP.wDataLen = wUDPDataLen;
    pInfo->wDataLength = wUDPDataLen;

#if 0
    if(wUDPDataLen)
    {
/*
        memcpy(pInfo->szData, pBuf + wUDPHeaderLen, wUDPDataLen);
*/
		pInfo->offset = anz_offset(p, (pBuf + wUDPHeaderLen));
    }
#endif

    return dResult;
}

/*************************************************
func   : BCD()
def    : BCD
param  : UCHAR ucData
return : ucHigh | ucLow
 *************************************************/ 
UCHAR BCD2(UCHAR ucData)
{
	UCHAR   ucHigh;
	UCHAR   ucLow;

	ucHigh = (ucData << 4) & 0xf0;
	ucLow = (ucData >> 4) & 0x0f;

	return (ucHigh | ucLow);
}


/*************************************************
func   : Analyze_DataLink()
def    : Analyze DataLink protocol - HDLC, PPP
 *************************************************/
int Analyze_DataLink(PUCHAR pBuf, DWORD dwSize, INFO_ETH *pInfo )
{ 
	int             dResult = 1;
	PRA_HDR         pRA;
	PDATALINK_HDR   pDataLink = (PDATALINK_HDR)pBuf;

	if (dwSize < 5)    /* 5bytes (Header + FCS) */
		return ERR_000201;

	if (pDataLink->Address == 0x0F)
	{
		/* HDLC */
		pInfo->wFrameType = TOUSHORT(pDataLink->Protocol);

		if (pInfo->wFrameType == PROTO_IP)
			dResult = Analyze_IP(pDataLink->Data, dwSize - 6, pInfo);
	}
	else if (pDataLink->Address == 0xFF)
	{
		/* PPP Frame Format */
		pInfo->wFrameType = TOUSHORT(pDataLink->Protocol);

		if (pInfo->wFrameType == PPP_IP)
			dResult = Analyze_IP(pDataLink->Data, dwSize-6, pInfo);
#if 0
		else if (pInfo->wFrameType == PPP_CCP1 || pInfo->wFrameType == PPP_CCP2)
			dResult = Analyze_PPP_CCP(pDataLink->Data, dwSize-6, pInfo);

		else if (pInfo->wFrameType == PPP_CHAP)
			dResult = Analyze_PPP_CHAP(pDataLink->Data, dwSize-6, pInfo);

		else if (pInfo->wFrameType == PPP_IPCP)
			dResult = Analyze_PPP_IPCP(pDataLink->Data, dwSize-6, pInfo);

		else if (pInfo->wFrameType == PPP_LCP)
			dResult = Analyze_PPP_LCP(pDataLink->Data, dwSize-6, pInfo);

		else if (pInfo->wFrameType == PPP_PAP)
			dResult = Analyze_PPP_PAP(pDataLink->Data, dwSize-6, pInfo);
#endif
	}
	else
	{
		/* RA Header */
		pRA = (PRA_HDR)pBuf;
#if 0
		/* Add Upper Layer Protocol... */
		dResult = Analyze_PPP(0x880B, pRA->Data, dwSize - 3 - 2, pInfo, pComp);
#endif
	}

	return dResult;
}

/*************************************************
func   : AnalyzeURL_IP_TCP_HTTP()
def    : Analyze a HTTP protocol

method를 구분하고,
스트링을 분석해서 host과 content 길이를 확인한다.

 *************************************************/
int Analyze_HTTP(PUCHAR pBuf, WORD wSize, INFO_HTTP *pInfo)
{
	int dRet = 1;
	int i=0, j;
	char str[128];

	pInfo->ucMethod = 0;

	if( wSize < 7 )
		return 0;

	memset( str, 0, 128 );
	if( strncasecmp((char*)pBuf, "HTTP/1.", 7) == 0 ) 
	{	/* response */
		pInfo->ucMethod = 3;

		while( pBuf[i] != ' ' )
			i++;

		while( pBuf[i] == ' ' )
			i++;

		j = i;
		while( i < wSize )
		{
			if( pBuf[i] == ' ' )
			{
				if( (i-j) == 0 || (i-j) > 3 )
					return E_HTTPParcing; 

				memcpy( str, pBuf+j, i-j );
				pInfo->wRetCode = atoi(str);
				break;
			}
			i++;
		}
	} /**** end of HTTP/1. ******/
	else if( strncasecmp((char*)pBuf, "GET ", 4) == 0 ) 
	{	/* request */
		pInfo->ucMethod = 1;

		while( pBuf[i] != ' ' )
			i++;

		while( pBuf[i] == ' ' )
			i++;

		j = i;
		while( i < wSize )
		{
			if( pBuf[i] == ' ' )
			{
				if( (i-j) == 0 )
					return E_HTTPParcing; 
				memcpy( pInfo->szURL, pBuf+j, (i-j < 128 ? i-j : 127) );
				break;
			}
			i++;
		}
	} /***** end of GET ******/
	else if( strncasecmp((char*)pBuf, "POST", 4) == 0 
			|| strncasecmp((char*)pBuf, "OPTIONS", 7) == 0
			|| strncasecmp((char*)pBuf, "HEAD", 4) == 0
			|| strncasecmp((char*)pBuf, "PUT", 3) == 0
			|| strncasecmp((char*)pBuf, "DELETE", 6) == 0
			|| strncasecmp((char*)pBuf, "TRACE", 5) == 0
			|| strncasecmp((char*)pBuf, "PATCH", 5) == 0
			|| strncasecmp((char*)pBuf, "COPY", 4) == 0
			|| strncasecmp((char*)pBuf, "MOVE", 4) == 0
			|| strncasecmp((char*)pBuf, "LINK", 4) == 0
			|| strncasecmp((char*)pBuf, "UNLINK", 6) == 0 )
	{
		pInfo->ucMethod = 2;
		return 0;
	}
	else
		return 0;

	while( i < wSize )
	{
		if( pBuf[i] == 0x0D && pBuf[i+1] == 0x0A )
		{
			if( pBuf[i+2] == 0x0D && pBuf[i+3] == 0x0A )
				break;

			i += 2;
			str[0] = 0;
			if( strncasecmp((char*)(pBuf+i), "Host: ", 6) == 0 )
			{
				i += 6;
				Analyze_HTTP_String( pBuf+i, wSize-i, str, &j );
				sprintf( (char*)pInfo->szHost, "%s", str );
				i += j;
			}
			else if( strncasecmp((char*)(pBuf+i), "Content-Length: ", 16 ) == 0 )
			{
				i += 16;
				Analyze_HTTP_String( pBuf+i, wSize-i, str, &j );
				//if( j > 5 )
				//	return E_HTTPParcing; 

				pInfo->wContentLen = atoi(str);
				i += j;
			}
			/*
			   else if( strncasecmp(pBuf+i, "Content-Type: ", 14) == 0 )
			   {
			   Analyze_HTTP_String( pBuf+i+14, wSize-(i+16), str, &j );
			   }
			   */
		}
		else 
			i++;

	} /***** end of while  i < wSize, Host이름과 content 길이 구분  ******/

	return dRet;

} /***** end of Analyze_HTTP ********/

/*************************************************
func   : AnalyzeURL_STRING()
def    : Analyze a HTTP protocol
 *************************************************/
int Analyze_HTTP_String( PUCHAR pBuf, WORD wSize, char *str, int *iLen )
{
	int dRet = 1;
	int i=0;

	while( i < wSize )
	{
		if( pBuf[i] == 0x0D && pBuf[i+1] == 0x0A )
		{
			memcpy( str, pBuf, (i < 128 ? i : 127) );
			break;
		}
		i++;
	}
	*iLen = i; 

	return dRet;
} 

/*****************************************************************
  Modified By KIW in 2004.3.15
  2004.3.20 :: Src|Dest port analyze Added By KIW
 *****************************************************************/

int AnalyzeIP_ICMP(PUCHAR pucBuffer, WORD wSize, INFO_ETH *pInfo)
{
	PICMP_RHDR pICMP = (PICMP_RHDR)pucBuffer;
	PIP_RHDR 	pIPHdr;
	PTCP_RHDR	pTCPHdr;	

	WORD wIPHeaderLen;


	pInfo->stICMP.Type = pICMP->Type;

	/**** No need to analyze ***********/
	if(pICMP->Type != 3)
		return 0;

	pInfo->stICMP.Code = pICMP->Code;

	pIPHdr = (PIP_RHDR)(pICMP->Data + 4);

	pInfo->stICMP.ucProtocol = pIPHdr->Protocol;

	/* IP header checksum Added by dark264 */
	if(Checksum(pICMP, wSize) != 0)
	{
		pInfo->stICMP.bChecksumErr = 1;
		return 0;
	}

	pInfo->stICMP.dwSrcIP = TOULONG(pIPHdr->Source);
	pInfo->stICMP.dwDestIP = TOULONG(pIPHdr->Destination);

	wIPHeaderLen = pIPHdr->IHL << 2;

	/***** Data to Analyze is poor ********/
	if(wSize < (ICMP3_HDR_LEN+wIPHeaderLen+ICMP3_ATT_IPDATA_LEN))        
		return -1;		

	pTCPHdr	= (PTCP_RHDR)(pIPHdr+wIPHeaderLen);

	/* Source Port */
	pInfo->stICMP.wSrcPort = TOUSHORT(pTCPHdr->Source);

	/* Destination Port */
	pInfo->stICMP.wDestPort = TOUSHORT(pTCPHdr->Destination);

	return 1;

} /**** end of AnalyzeIP_ICMP_Data *****/


USHORT Checksum(void *addr, WORD count)
{
	// Checksum
	// Compute Internet Checksum for "count" bytes
	//      beginning at location "addr".

	register long sum = 0;
	unsigned short  checksum;

	while( count > 1 )
	{
		// This is the inner loop
		sum += *(unsigned short *) addr;
		addr = (unsigned short *) addr+1;

		count -= 2;
	}

	// Add left-over byte, if any
	if( count > 0 )
		sum += *(unsigned char *) addr;

	// Fold 32-bit sum to 16 bits
	while( sum >> 16 )
	{
		sum = (sum & 0xffff) + (sum >> 16);
	}
	checksum = (unsigned short)~sum;

	return checksum;
}


DWORD SET_BIT(int dPosition)
{
	DWORD data;

	data = data >> (dPosition - 1);
	data = 0x01;
	data = data << (dPosition - 1);

	return data;
}

/*************************************************
 * func   : Analyze_IP_UDP_RADIUS()
 * def    : Analyze RADIUS protocol
 * *************************************************/
int Analyze_RADIUS( PUCHAR pBuf, WORD wSize, INFO_RADIUS *pInfo )
{
	PRADIUS pRadius = (PRADIUS)pBuf;
	WORD wLength = TOUSHORT(pRadius->Length);

	if( wSize != wLength )
		return ERR_Wrong_Size;

	switch( pRadius->Code )
	{
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 124:
		case 125:
		case 126:
		case 127:
			pInfo->bRADIUS = 1;
			break;
		default:
			return 0;
	}

	pInfo->ucCode = pRadius->Code;
	pInfo->dwId = pRadius->Identifier;

	if( wLength > sizeof(RADIUS)-1 )
	{
		Analyze_RADIUS_Attr(pRadius->Attributes, wLength-(sizeof(RADIUS)-1), pInfo);
	}

	return 1;
}

/*************************************************
 * func   : Analyze_IP_UDP_RADIUS_Attr()
 * *************************************************/
int Analyze_RADIUS_Attr(PUCHAR pBuf, WORD wSize, INFO_RADIUS *pInfo)
{
	WORD i = 0;
	PRADIUS_Attr    pAttr;
	PRADIUS_Vendor  pVendor;


	while( i < wSize )
	{
		pAttr = (PRADIUS_Attr)(pBuf+i);
		switch( pAttr->Type )
		{
			case 26:        /* Vendor Specific */
				pVendor = (PRADIUS_Vendor)pAttr->Data;
				switch(pVendor->Type)
				{
					case 44:        // Corelation ID
						memcpy(pInfo->ucCorelationID, pVendor->Data, 8 );
						break;
					case 210:       // Subsinfo Result
						pInfo->ucSubsRet = pVendor->Data[0];    // 0: unsuccessful, 1: successful
						break;
					case 211:       // ADR 0xd3
						pInfo->ucADR = pVendor->Data[0];                // 1 ~ 9: reason
						break;
					case 197:       // WSType
						pInfo->ucWSType = pVendor->Data[0];
						break;
					case 218:       // CST
						pInfo->ucCST = pVendor->Data[0];
						break;
					case 205:       // WinCall ID
						if( pVendor->Length == 6 )
							pInfo->dwWinCallId = TOULONG(pVendor->Data);
						break;
					case 222:       // MSISDN
						if( pVendor->Length < 18 )
							memcpy(pInfo->szMDN, pVendor->Data, pVendor->Length-2 );
						break;
				}

			case 8:         /* Framed IP Address */
				if( pAttr->Length == 6 )
					pInfo->dwFramedIP = TOULONG(pAttr->Data);
				break;
			case 31:        /* Calling Station Id */
				if( pAttr->Length <= 22 )
					memcpy( pInfo->ucCSID, pAttr->Data, pAttr->Length-2 );
				break;
			case 40:        /* Account Status Type */
				if( pAttr->Length == 6 )
					pInfo->dwAcctType = TOULONG(pAttr->Data);
				break;
			default:
				break;
		} /* end of switch */

		if( pAttr->Length <= 0 || pAttr->Length > wSize )
			return ERR_Invalid_Length;

		i += pAttr->Length;
	}

	return 1;
}

/*************************************************
func   : Analyze_IP_GRE()
def    : Analyze GRE protocol 
*************************************************/
int Analyze_IP_GRE(PUCHAR pBuf, WORD wSize, INFO_ETH *pInfo )
{
    int     dResult = 1;
    PGRE    pGRE;
    WORD    wGREHeaderSize;
    short   dGREDataSize;
    WORD    Protocol;
    UCHAR   ucGREFlag;
    UCHAR   ucFlagVer;
    UCHAR   FlagSeq, FlagKey, FlagAck;

    pGRE = (PGRE)pBuf;
    ucGREFlag = pGRE->FlagRecur;
    ucFlagVer = pGRE->FlagVer;
    FlagSeq = ucGREFlag & 0x10;
    FlagKey = ucGREFlag & 0x20;
    FlagAck = ucFlagVer & 0x80;

    /* GRE Check */
    pInfo->stGRE.bGRE = TRUE;
    pInfo->stGRE.ucGREFlag = pGRE->FlagRecur;

    wGREHeaderSize = 4 + (FlagKey ? 4 : 0) + (FlagSeq ? 4 : 0) + (FlagAck ? 4 : 0);
    if( wGREHeaderSize > wSize )
        return ERR_GRE_HeaderLenError;
    pInfo->stGRE.wGREHeaderSize = wGREHeaderSize;

    Protocol = TOUSHORT(pGRE->Protocol);
    dGREDataSize = wSize - wGREHeaderSize;
    if( dGREDataSize < 0 )
        return ERR_GRE_DataLenError;

    pInfo->stGRE.wDataSize = dGREDataSize;

    if( FlagSeq > 0 && FlagKey > 0 )
    {
        /* Key */
        pInfo->stGRE.dwGREKey = TOULONG(pGRE->Key);

        /* Sequence Number */
        pInfo->stGRE.dwSeqNum = TOULONG(pGRE->Sequence);

        /* Data */
        pInfo->pAppData = FlagAck ? pGRE->Data : pGRE->Acknowledge;
    }
    else if (FlagSeq == 0 && FlagKey > 0)
    {
        /* Key */
        pInfo->stGRE.dwGREKey = TOULONG(pGRE->Key);
        pInfo->pAppData = FlagAck ? pGRE->Acknowledge : pGRE->Sequence;
    }
	else if (FlagSeq > 0 && FlagKey == 0)
    {
        /* Sequence Number */
        pInfo->stGRE.dwSeqNum = TOULONG(pGRE->Key);
        pInfo->pAppData = FlagAck ? pGRE->Acknowledge : pGRE->Sequence;
    }
    else
    {
        pInfo->pAppData = FlagAck ? pGRE->Sequence : pGRE->Key;
    }

#if 0
    if (Protocol == 0x880B || Protocol == 0x8881)
        {
        if (wSize < wGREHeaderSize)
            return ERR_350101;

        dResult = Analyze_PPP(Protocol, pData, wSize-wGREHeaderSize, pInfo, pComp);
        }
#endif

    return dResult;
}

/************ DIAMETER DECODEING *******************/
int dAnalyze_Diameter_AvpHeader(PUCHAR pBuf, pst_AVPHdr avpHdr)
{
	int 		offset;


//	dump_DebugString("AVP DATA", pBuf, 100);

	offset = 0;
	avpHdr->uiAvpCode = ntohl(*((UINT*)(pBuf+offset)));
	offset += 4;
	avpHdr->flags.ucVendor = *((UCHAR*)(pBuf+offset)) >> 7;
	avpHdr->flags.ucMandatory = *((UCHAR*)(pBuf+offset)) >> 6;
	avpHdr->flags.ucSecurity = *((UCHAR*)(pBuf+offset)) >> 5;
	avpHdr->uiLength = ntohl(*((UINT*)(pBuf+offset))) & 0x00ffffff;
	offset += 4;


	if (avpHdr->flags.ucVendor)    
	{
		avpHdr->uiVenderId = ntohl(*((UINT*)(pBuf+offset)));
		offset += 4;
	}

	avpHdr->value = (char*)(pBuf+offset);

	switch (avpHdr->uiAvpCode)
	{
		case PUBLIC_IDENTITY:
		case USER_AUTHORIZATION_TYPE:
		case SESSION_ID:
		case ORIGIN_HOST:
		case ORIGIN_REALM:
		case DESTINATION_HOST:
		case DESTINATION_REALM:
		case AUTH_APPLICATION_ID:
		case CC_REQUEST_NUMBER:
		case CC_REQUEST_TYPE:
		case SUBSCRIPTION_ID_TYPE:
		case SUBSCRIPTION_ID_DATA:
		case ORIGIN_STATE_ID:
		case PROXY_HOST:
		case PROXY_STATE:
		case BEARER_OPERATION:
		case GCID:
		case ICID:
		case INSERVICE:
		case PCC_RULE_STATUS:
		case CHARGING_SERVER:
		case CAUSE_CODE:
		case ACCOUNTING_RECORD_TYPE:
		case RE_AUTH_REQUEST_TYPE:
		case CALLED_STATION_ID:
		case SUBSCRIPTION_ID:
		case PROXY_INFO:
		case ACCESS_NETWORK_CHARGING_IDENTIFIER_GX:
		case CHARGING_RULE_INSTALL:
		case CHARGING_RULE_DEFINITION:
		case IMS_INFORMATION:
		case SERVICE_INFORMATION:
		case CHARGING_RULE_REPORT:
		case RESULT_CODE:						/* 268 */
		case EXPERIMENTAL_RESULT_CODE:			/* 297 */
			return offset;
		default: 
			return avpHdr->uiLength + (avpHdr->uiLength % 4 == 0  ? 0 : (4 - avpHdr->uiLength % 4 ));  // 4bye padding
	}
}


int dAnalyze_Diameter( PUCHAR pBuf, int dDataLen, pst_DiameterInfo pstDiameterInfo)
{
	int offset = 0;
	int avpoffset = 0;
	int nextOffset = 0;
	int avpHdrLen = 0;


	st_DiameterHdr 	*hdr = &pstDiameterInfo->stDiameterHdr;
	st_AVPHdr 		*avpHdr = &pstDiameterInfo->stAVPHdr;
	st_AVPHdr 		stAVPHdr;
	UCHAR 			*pAVPBuf;

	hdr->ucVersion = *((UCHAR*)(pBuf+offset));
	if (hdr->ucVersion != 1) 
		return -1;

	hdr->uiLength = ntohl(*((UINT*)(pBuf+offset))) & 0x00ffffff;
	offset += 4;
	hdr->flags.bRequest = *((UCHAR*)(pBuf+offset)) >> 7;
	hdr->flags.bProxiable = *((UCHAR*)(pBuf+offset)) >> 6;	
	hdr->flags.bError = *((UCHAR*)(pBuf+offset)) >> 5;	
	hdr->flags.reserved = *((UCHAR*)(pBuf+offset)) >> 4;

	hdr->uiCmdCode = ntohl(*((UINT*)(pBuf+offset))) & 0x00ffffff;
	offset += 4;
	hdr->uiAppID = ntohl(*((UINT*)(pBuf+offset)));
	offset += 4;
	hdr->uiHopByHopID = ntohl(*((UINT*)(pBuf+offset)));
	offset += 4;
	hdr->uiEndToEndID = ntohl(*((UINT*)(pBuf+offset)));
	offset += 4;

	if (hdr->uiLength <= DIAMETER_HDR_SIZE) 
		return -2;

	while( (offset < hdr->uiLength) && (offset < dDataLen) )
	{
		nextOffset = dAnalyze_Diameter_AvpHeader(pBuf+offset, avpHdr);
		if (nextOffset == 0) 
			break;
		log_print(LOGN_DEBUG, "offset:%d next:%d AVP code:%d len:%d",
				offset, nextOffset, avpHdr->uiAvpCode, avpHdr->uiLength);

//		offset += avpHdr->uiLength;
		offset += avpHdr->uiLength + (avpHdr->uiLength % 4 == 0  ? 0 : (4 - avpHdr->uiLength % 4 ));

		if(avpHdr->flags.ucVendor) {
			avpHdrLen = 12;
		} else {
			avpHdrLen = 8;
		}

		switch(avpHdr->uiAvpCode)
		{
			case PUBLIC_IDENTITY:
				memcpy(pstDiameterInfo->szPublicID, avpHdr->value, min(MAX_HOST_REALM_LEN, avpHdr->uiLength - avpHdrLen));
				break;
			case USER_AUTHORIZATION_TYPE:
				pstDiameterInfo->ucUserAuthorizationType = ntohl(*((UCHAR*)(avpHdr->value)));
				break;
			case SESSION_ID:
				memcpy(pstDiameterInfo->szSessionID, avpHdr->value, min(MAX_SESSIONID_LEN, avpHdr->uiLength - avpHdrLen));
				break;
			case ORIGIN_HOST:
				memcpy(pstDiameterInfo->szOrgHost, avpHdr->value, min(MAX_HOST_REALM_LEN, avpHdr->uiLength - avpHdrLen));
				break;
			case ORIGIN_REALM:
				memcpy(pstDiameterInfo->szOrgRealm, avpHdr->value, min(MAX_HOST_REALM_LEN, avpHdr->uiLength - avpHdrLen));
				break;
			case DESTINATION_HOST:
				memcpy(pstDiameterInfo->szDestHost, avpHdr->value, min(MAX_HOST_REALM_LEN, avpHdr->uiLength - avpHdrLen));
				break;
			case DESTINATION_REALM:
				memcpy(pstDiameterInfo->szDestRealm, avpHdr->value, min(MAX_HOST_REALM_LEN, avpHdr->uiLength - avpHdrLen));
				break;
			case AUTH_APPLICATION_ID:
				pstDiameterInfo->uiApplicationID = ntohl(*((UINT*)(avpHdr->value)));
				break;
			case CC_REQUEST_NUMBER:
				pstDiameterInfo->uiCCReqNum = ntohl(*((UINT*)(avpHdr->value)));
				break;
			case CC_REQUEST_TYPE:
				pstDiameterInfo->ucCCReqType = ntohl(*((UINT*)(avpHdr->value)));
				break;
			case SUBSCRIPTION_ID_TYPE:
				pstDiameterInfo->ucSubscriptionType = ntohl(*((UINT*)(avpHdr->value)));
				break;
			case SUBSCRIPTION_ID_DATA:
				memcpy(pstDiameterInfo->szSubscriptionData, avpHdr->value, min(MAX_SUBSCRIPTION_LEN, avpHdr->uiLength - avpHdrLen));
				break;
			case ORIGIN_STATE_ID:
				pstDiameterInfo->uiOrgStateId = ntohl(*((UINT*)(avpHdr->value)));
				break;
			case PROXY_HOST:
				memcpy(pstDiameterInfo->szProxyHost, avpHdr->value, min(MAX_PROXYHOST_LEN, avpHdr->uiLength - avpHdrLen));
				break;
			case PROXY_STATE:
				memcpy(pstDiameterInfo->szProxyState, avpHdr->value, min(MAX_PROXYSTATE_LEN, avpHdr->uiLength - avpHdrLen));
				break;
			case BEARER_OPERATION:
				pstDiameterInfo->ucBearerOP = ntohl(*((UINT*)(avpHdr->value)));
				break;
			case GCID:
				memcpy(pstDiameterInfo->szGCID, avpHdr->value, min(MAX_GCID_LEN, avpHdr->uiLength - avpHdrLen));
				break;
			case ICID:
				memcpy(pstDiameterInfo->szICID, avpHdr->value, min(MAX_ICID_LEN, avpHdr->uiLength - avpHdrLen));
				break;
			case INSERVICE:
				memcpy(pstDiameterInfo->szInService, avpHdr->value, min(MAX_INSERVICE_LEN, avpHdr->uiLength - avpHdrLen));
				break;
			case PCC_RULE_STATUS:
				pstDiameterInfo->ucPccStatus = ntohl(*((UCHAR*)(avpHdr->value)));
				break;
			case CHARGING_SERVER:
				pstDiameterInfo->uiChargingServer = ntohl(*((UINT*)(avpHdr->value)));
				break;
			case CAUSE_CODE_CDN:
				pstDiameterInfo->iCauseCode = ntohl(*((INT*)(avpHdr->value)));
				break;
			case ACCOUNTING_RECORD_TYPE:
				pstDiameterInfo->ucAccRecordType = ntohl(*((UINT*)(avpHdr->value)));
				break; 
			case RE_AUTH_REQUEST_TYPE:
				pstDiameterInfo->ucReAuthReqType = ntohl(*((UINT*)(avpHdr->value)));
				break; 
			case CALLED_STATION_ID:
				memcpy(pstDiameterInfo->szCalledStationID, avpHdr->value, min(MAX_CALLED_STATION_ID_LEN, avpHdr->uiLength - avpHdrLen));
				break; 
			case RESULT_CODE:
				pstDiameterInfo->uiResultCode = ntohl(*((UINT*)(avpHdr->value)));
				break;
			case EXPERIMENTAL_RESULT: 	/* 297 */
			case USER_IDENTITY:
				avpoffset = 0;
				pAVPBuf = (UCHAR*)avpHdr->value;
				while(avpoffset < avpHdr->uiLength) {
					log_print(LOGN_DEBUG, "avpoffset:%d len:%d", avpoffset, avpHdr->uiLength);

					stAVPHdr.uiAvpCode = ntohl(*((UINT*)(pAVPBuf+avpoffset)));
					avpoffset += 4;
					stAVPHdr.flags.ucVendor = *((UCHAR*)(pAVPBuf+avpoffset)) >> 7;
					stAVPHdr.flags.ucMandatory = *((UCHAR*)(pAVPBuf+avpoffset)) >> 6;
					stAVPHdr.flags.ucSecurity = *((UCHAR*)(pAVPBuf+avpoffset)) >> 5;
					stAVPHdr.uiLength = ntohl(*((UINT*)(pAVPBuf+avpoffset))) & 0x00ffffff;
					avpoffset += 4;

					if (stAVPHdr.flags.ucVendor) {
						stAVPHdr.uiVenderId = ntohl(*((UINT*)(pAVPBuf+avpoffset)));
						avpoffset += 4;
						avpHdrLen = 12;
					} else {
						avpHdrLen = 8;
					}

					if (stAVPHdr.uiAvpCode == EXPERIMENTAL_RESULT_CODE) { 	/* 298 */
						pstDiameterInfo->uiExpResultCode = ntohl(*((UINT*)(pAVPBuf+avpoffset)));
					}
					if (stAVPHdr.uiAvpCode == PUBLIC_IDENTITY) { 	/* 601 */
						memcpy(pstDiameterInfo->szPublicID, pAVPBuf+avpoffset, min(MAX_HOST_REALM_LEN, stAVPHdr.uiLength - avpHdrLen));
					}
					avpoffset += 
						((avpHdr->uiLength + (avpHdr->uiLength % 4 == 0  ? 0 : (4 - avpHdr->uiLength % 4 ))) - avpoffset);
				}
				break;
			case EXCEPTIONAL_CASE:
				/* 0 */
				return offset;
				break;
			default:
				log_print(LOGN_INFO, "NOT SUPPORT AVP CODE: %d OFFSET: %d", avpHdr->uiAvpCode, offset);
				break;	
		}
	}

	return offset;
}


/*************************************************
func   : Analyze_A11()
def    : Analyze A11 protocol
*************************************************/
int Analyze_A11(PUCHAR pBuf, WORD wSize, INFO_A11 *pInfo)
{
    int     dResult = 1;
    PA11_REG_REQ    pRegReq;
    PA11_REG_REP    pRegRep;
    PA11_REG_UPD    pRegUpd;
    PA11_REG_ACK    pRegAck;

    INFO_A11 *pA11 = pInfo;

    pA11->ucMsg = pBuf[0];

    switch (pBuf[0])
    {
        /* A11-Registration Request */
        case 0x01:
            if (wSize < sizeof(A11_REG_REQ)-1)
                return ERR_450102;

            pA11->bA11 = TRUE;
            pRegReq = (PA11_REG_REQ) pBuf;

            pA11->usLifetime = TOUSHORT(pRegReq->Lifetime);
            memcpy( pA11->ucHomeAgentAddr, pRegReq->HomeAgent, 4);

            dResult = Analyze_A11_Extension(pRegReq->Extension, wSize-24, pA11);

            break;

        /* A11-Registration Reply */
        case 0x03:
            if(wSize < sizeof(A11_REG_REP)-1)
                return ERR_450103;

            pA11->bA11 = TRUE;
            pRegRep = (PA11_REG_REP)pBuf;

            pA11->usLifetime = TOUSHORT(pRegRep->Lifetime);
            pA11->ucCode = pRegRep->Code;
            memcpy( pA11->ucHomeAgentAddr, pRegRep->HomeAgent, 4);

            dResult = Analyze_A11_Extension(pRegRep->Extension, wSize-20, pInfo);

            break;

        /* A11-Registration Update */
        case 0x14:
        /* A11-Session Update */
		case 0x16:
            if(wSize < sizeof(A11_REG_UPD)-1)
                return ERR_450104;

            pA11->bA11 = TRUE;
            pRegUpd = (PA11_REG_UPD)pBuf;
            memcpy( pA11->ucHomeAgentAddr, pRegUpd->HomeAgent, 4);

            dResult = Analyze_A11_Extension(pRegUpd->Extension, wSize-20, pInfo);
            break;

        /* Registration Acknowledge */
        case 0x15:
        /* A11-Session Update Acknowledge */
        case 0x17:
            if(wSize < sizeof(A11_REG_ACK)-1)
                return ERR_450105;

            pA11->bA11 = TRUE;
            pRegAck = (PA11_REG_ACK)pBuf;
            pA11->ucCode = pRegAck->Status;

            dResult = Analyze_A11_Extension(pRegAck->Extension, wSize-20, pInfo);
            break;

        /* RP KeepAlive Message */
        case 0xF0:
            pA11->bA11 = TRUE;
            break;

        /* RP Repair Message */
        case 0xF1:
            pA11->bA11 = TRUE;
            break;

        default:
            return 0;
    }

    return dResult;
}

/*************************************************
func   : Analyze_A11_Extension()
def    : Analyze A11 protocol
*************************************************/
int Analyze_A11_Extension(PUCHAR pBuf, WORD wSize, INFO_A11 *pInfo)
{
    DWORD i = 0;
    WORD wLength;

    while(i < wSize)
    {
        switch(pBuf[i])
        {
            case 0x20:  /* Mobile-Home Authentication Extension */
                wLength = pBuf[i+1];
                if( (wLength > wSize) || (wLength <= 0) )
                    return ERR_Invalid_Length;

                Analyze_A11_MHAE(pBuf+i, wLength, pInfo);
                i += (wLength + 2);
                break;

            case 0x26:  /* Critical Vendor/Organization Specific Extension */
                wLength = TOUSHORT(pBuf+i+2);
                if( (wLength > wSize) || (wLength <= 0) )
                    return ERR_Invalid_Length;

                Analyze_A11_CVSE(pBuf+i, wLength, pInfo);
                i += (wLength + 4);
                break;

            case 0x27:  /* Session Specific Extension */
                wLength = pBuf[i+1];
                if( (wLength > wSize) || (wLength <= 0) )
                    return ERR_Invalid_Length;

                Analyze_A11_SSE(pBuf+i, wLength, pInfo);
                i += (wLength + 2);
                break;

            case 0x28:  /* Registration Update Authentication Extension */
                wLength = pBuf[i+1];
                if( (wLength > wSize) || (wLength <= 0) )
                    return ERR_Invalid_Length;

                Analyze_A11_RUAE(pBuf+i, wLength, pInfo);
                i += (wLength + 2);
                break;

			case 0x86:	/* Normal Vendor/Organization Specific Extension */
				wLength = pBuf[i+1];
				if( (wLength > wSize) || (wLength <= 0) )
                    return ERR_Invalid_Length;

				Analyze_A11_NVSE(pBuf+i, wLength, pInfo);
                i += (wLength + 2);
				break;

            default:
                return 0;
        }
    }

    return 1;
}

/*************************************************
func   : Analyze_IP_UDP_A11_MHAE()
def    : Analyze A11 protocol - Mobile-Home Authentication Extension
*************************************************/
int Analyze_A11_MHAE(PUCHAR pBuf, WORD wSize, INFO_A11 *pInfo)
{
//  PA11_MHAE pMHAE = (PA11_MHAE)pBuf;
    return 1;
}

/*************************************************
func   : Analyze_IP_UDP_A11_CVSE()
def    : Analyze A11 protocol - Critical Vendor/Organization Specific Extension
*************************************************/
int Analyze_A11_CVSE(PUCHAR pBuf, WORD wSize, INFO_A11 *pInfo)
{
    PA11_CVSE pVOSE = (PA11_CVSE)pBuf;
    DWORD i = 0;
    PUCHAR pData;
    DWORD dwDataLen;
    DWORD dwType;

    INFO_A11 *pA11 = pInfo;

    /* Application Type - 1: Accounting
        SubType - 1: RADIUS, 2:
       Application Type - 242(0xf2) : Network Event Indicator
        SubType - 1: PDSN Release, 2: AAA Release
    */

    if ( (pVOSE->ApplicationType == 1 || pVOSE->ApplicationType == 242 || pVOSE->ApplicationType == 2)
        && (pVOSE->ApplicationSubType == 1 || pVOSE->ApplicationSubType == 2)) {
        pData = pVOSE->ApplicationData;

        if( pVOSE->ApplicationType == 242 )
            pInfo->ucApplicationType |= 0x04;
        else
            pInfo->ucApplicationType |= pVOSE->ApplicationType;

        dwDataLen = wSize - 6;
        while (i < dwDataLen) {
            if (pData[i] == 26) {		// Vendor-Specific(26)
                    if( pData[i+1] != pData[i+7]+6 )
                        return ERR_Invalid_Length;

                    switch (pData[i+6]) {
                        case 10:    // BS/MSC ID(PPP Setup)
                            //memcpy(pA11->szBsMscId, pData+i+8, 12);
                            break;
                        case 12:    // Forward Mux Option
                            pA11->dwForwardMux = TOULONG(pData+i+8);
                            break;
                        case 13:    // Reverse Mux Option
                            pA11->dwReverseMux = TOULONG(pData+i+8);
                            break;
                        case 16:    // Service Option
                            pA11->dwServiceOption = TOULONG(pData+i+8);
                            break;
						case 40:    // Airlink Record Type
                            dwType = TOULONG(pData+i+8);
                            switch (dwType) {
                                case 1:
                                    pA11->ucAirlinkType |= 0x01;    /* SETUP */
                                    break;
                                case 2:
                                    pA11->ucAirlinkType |= 0x02;    /* START */
                                    break;
                                case 3:
                                    pA11->ucAirlinkType |= 0x04;    /* STOP */
                                    break;
                                case 4:
                                    pA11->ucAirlinkType |= 0x08;    /* SDB */
                                    break;
                            }
                            break;
                        case 52:    // ESN
                            memcpy(pA11->ucESN, pData+i+8, 8);
                            break;
						case 108:	//SUBLINK
							memcpy(pA11->szSublink, pData+i+8, 37);
							break;
						/*
                        case 178:   // Update Reason
                            pA11->dwUpdateReason = TOULONG(pData+i+8);
                            break;
						*/
                    }
            }

            if( pData[i+1] <= 0 )
                return ERR_Invalid_Length;

            i += pData[i+1];
        }
    }

    return 1;
}

/*************************************************
func   : Analyze_IP_UDP_A11_NVSE()
def    : Analyze A11 protocol - Normal Vendor/Organization Specific Extension
*************************************************/
int Analyze_A11_NVSE(PUCHAR pBuf, WORD wSize, INFO_A11 *pInfo)
{
	PA11_NVSE pNVSE = (PA11_NVSE)pBuf;
	PUCHAR pData;
	DWORD dwDataLen, dwAppLen, i = 0;
	pA11_NVSE_FFLOW		pFFlow;
	pA11_NVSE_RFLOW		pRFlow;
	pA11_NVSE_GENTRY	pGEntry;
	INFO_A11 *pA11 = pInfo;

	pData = pNVSE->ApplicationData;

	dwDataLen = wSize - 8;

	switch( pNVSE->ApplicationType ) {
		case 0x07:
			switch( pNVSE->ApplicationSubType ) {
				case 0x01:	/* */
					memcpy( &pA11->dwUpdateReason, pData, 1 );
					break;
				default:
					break;
			}
			break;
		case 0x08:	/* SESSION PARAMETER */
			switch( pNVSE->ApplicationSubType ) {
				case 0x03:	/* QoS Mode */

					break;
				case 0x02:	/* Always-On */
					pA11->AlwaysOn = 1; 
					break;
				default:

					break;
			}

			break;
		case 0x09:	/* SERVICE OPTION */
			switch( pNVSE->ApplicationSubType ) {
				case 0x01:	/* SERVICE OPTION VALUE */
					pA11->dwServiceOption = TOUSHORT(pData);
					break;
				default:

					break;
			}
			break;

		case 0x0B:	/* PCF ENABLEED FEATURES */
			switch( pNVSE->ApplicationSubType ) {
				case 0x01:  /* SHORT DATA INDICATION SUPPORTED */

					break;
				default:

					break;
			}
			break;

		case 0x0C:	/* ADDITIONAL SESSION INFORMATION */
			switch( pNVSE->ApplicationSubType )
			{
				case 0x01:
					dwAppLen = pData[0];
					while( i+sizeof(A11_NVSE_GENTRY) <= dwAppLen ) {
						pGEntry = (pA11_NVSE_GENTRY)&pData[i+1];
						if( TOUSHORT(pGEntry->GREProtoType) == 0x8881 && pA11->ucKeyEntryCnt < MAX_GRE_ENTRY )
							pA11->dwKeyEntry[pA11->ucKeyEntryCnt++] = TOUINT(pGEntry->GREKey);
						i += sizeof(A11_NVSE_GENTRY);
					}
					break;

				default:
					break;
			}
			break;

		case 0x0D:
			switch( pNVSE->ApplicationSubType )
			{
				case 0x01:	/* FORWARD QOS INFORMATION */
					pFFlow = (pA11_NVSE_FFLOW)(pData);
					if( pA11->FFlowIDCnt < MAX_FLOW_ID ) {
						pA11->FFlowID[pA11->FFlowIDCnt] = pFFlow->FENTRY.ID;
						pA11->FFlowIDCnt++;
					}
					break;

				case 0x02:	/* REVERSE QOS INFORMATION */
					pRFlow = (pA11_NVSE_RFLOW)(pData);
					if( pA11->RFlowIDCnt < MAX_FLOW_ID ) {
						pA11->RFlowID[pA11->RFlowIDCnt] = pRFlow->FENTRY.ID;
						pA11->RFlowIDCnt++;
					}
					break;

				default:
					break;
			}
			break;

		default:
			break;
	}

	return 1;
}


/*************************************************
func   : Analyze_IP_UDP_A11_SSE()
def    : Analyze A11 protocol - Session Specific Extension
*************************************************/
int Analyze_A11_SSE(PUCHAR pBuf, WORD wSize, INFO_A11 *pInfo)
{
    int     i;
    TCHAR   szMsg[1024];
    TCHAR   szTemp[1024];
    PA11_SSE    pSSE = (PA11_SSE)pBuf;

    pInfo->dwKey = TOULONG(pSSE->Key);

    sprintf(szMsg , "%1x", (pSSE->Identity[0] >> 4) & 0x0f);

    if( pSSE->MNIDLength < 1 || pSSE->MNIDLength > wSize )
        return -1;

    for (i = 1; i < pSSE->MNIDLength; i++) {
        sprintf(szTemp, "%02x", BCD2(pSSE->Identity[i]));
        strcat(szMsg, szTemp);
    }

    i = strlen(szMsg);
    if (szMsg[i-1] == 'f')
        szMsg[i-1] = 0x00;      /* remove last 'f' */

    sprintf((char*)pInfo->szMDN, "%s", szMsg);

    return 1;
}

/*************************************************
func   : Analyze_IP_UDP_A11_RUAE()
def    : Analyze A11 protocol - Registration Update Authentication Extension
*************************************************/
int Analyze_A11_RUAE(PUCHAR pBuf, WORD wSize, INFO_A11 *pInfo)
{
//  PA11_RUAE pRUAE = (PA11_RUAE)pBuf;
    return 1;
}

/*************************************************
func   : Analyze_L2TP()
def    : Analyze L2TP protocol
*************************************************/
int dAnalyze_L2TP(PUCHAR pBuf, int wSize, st_L2TP_INFO *pInfo)
{
	int 	dResult=1, idx=2;
	
	L2TP_AVP_HDR 	*pL2TPAvpHdr;
	st_L2TP_INFO 	*pstL2TPInfo = pInfo;

	USHORT 	control;
	USHORT 	avp_vendor_id;
	USHORT 	avp_type;
	USHORT 	avp_len;

#ifdef __DEBUG__
	dump_DebugString("L2TP HDR", pBuf, wSize);
#endif

	control = pntohs(&pBuf[0]);

	if(CONTROL_BIT(control)) {
		pstL2TPInfo->usPacketType = 1;
	}
	if (LENGTH_BIT(control)) {
		pstL2TPInfo->usLength = pntohs(&pBuf[idx]);
		idx += 2;
	} else {	// Not exist length field
		pstL2TPInfo->usLength = 0;
	}
	// TunnelID(2) + SessionID(2)
	pstL2TPInfo->usTunnelID = pntohs(&pBuf[idx]);
	idx += 2; 	
	pstL2TPInfo->usSessionID = pntohs(&pBuf[idx]);
	idx += 2; 	

	if (SEQUENCE_BIT(control)) {
		pstL2TPInfo->usNr = pntohs(&pBuf[idx]);
		idx += 2; 	
		pstL2TPInfo->usNs = pntohs(&pBuf[idx]);
		idx += 2; 	
	} else {
		pstL2TPInfo->usNr = 0;
		pstL2TPInfo->usNs = 0;
	}

	if (OFFSET_BIT(control)) {
		// OFFSET BIT가 있으면 offset size 만큼 Padding후에 AVP가 존재한다.
		pstL2TPInfo->usOffsetSize = pntohs(&pBuf[idx]);
		idx += 2;
		idx += pstL2TPInfo->usOffsetSize;
	} else {
		pstL2TPInfo->usOffsetSize = 0;
	}

	pstL2TPInfo->usHeaderSize = idx;

#ifdef __DEBUG__
	log_print(LOGN_DEBUG, "CODE: %02x IDX: %d TYPE: %d HDRSIZE: %d LEN: %d TID: %d SID: %d NS: %d NR: %d OFFSETSIZE: %d",
			control, idx, pstL2TPInfo->usPacketType, pstL2TPInfo->usHeaderSize, pstL2TPInfo->usLength,
			pstL2TPInfo->usTunnelID, pstL2TPInfo->usSessionID,
			pstL2TPInfo->usNs, pstL2TPInfo->usNr, pstL2TPInfo->usOffsetSize);
#endif

	if (pstL2TPInfo->usLength==12 || pstL2TPInfo->usLength==0) {			/* 12: ZLB Acknowledge 0: PPP */
		pstL2TPInfo->usMessageType = 0;
		return dResult;
	}

	while (idx < pstL2TPInfo->usLength ) {

		pL2TPAvpHdr = (L2TP_AVP_HDR *)&pBuf[idx];
		avp_vendor_id = ntohs(pL2TPAvpHdr->usVendorID);
		avp_len = pL2TPAvpHdr->ucLength;
		avp_type = ntohs(pL2TPAvpHdr->usAVPType);

#ifdef __DEBUG__
		log_print(LOGN_DEBUG, "AVP HDR CONTROL: %02x] TYPE: %d LEN: %d VENDOR: %d ", 
				pL2TPAvpHdr->ucControl, avp_type, avp_len, avp_vendor_id);
#endif

		if(avp_len < 6) { 	/* AVP Length must be <= 6 */
			log_print(LOGN_WARN, "ERROR AVP HDR] LEN: %d TYPE: %d VENDOR: %d", avp_len, avp_type, avp_vendor_id);
			return -2;
		} else {
			idx += avp_len;
		}

		switch (avp_type) {
			case CONTROL_MESSAGE:  					/* 0 */
				pstL2TPInfo->usMessageType = pntohs(&pL2TPAvpHdr->AVPValue[0]);
				break;
			case RESULT_ERROR_CODE:  				/* 1 */
				pstL2TPInfo->usResultCode = pntohs(&pL2TPAvpHdr->AVPValue[0]);
				pstL2TPInfo->usErrorCode = pntohs(&pL2TPAvpHdr->AVPValue[2]);
				break;
			case L2TP_PROTOCOL_VERSION: 					/* 2 */
				pstL2TPInfo->usProtocolVer = pntohs(&pL2TPAvpHdr->AVPValue[0]);
				break;
			case FRAMING_CAPABILITIES:				/* 3 */
			case BEARER_CAPABILITIES: 				/* 4 */
			case TIE_BREAKER: 						/* 5 */
				break;
			case FIRMWARE_REVISION: 				/* 6 */
				pstL2TPInfo->usFirmwareRevision = pntohs(&pL2TPAvpHdr->AVPValue[0]);
				break;
			case HOST_NAME:							/* 7 */
				memcpy(pstL2TPInfo->szHostName, &pL2TPAvpHdr->AVPValue[0], avp_len-6);
				pstL2TPInfo->szHostName[avp_len-6] = 0x00;
				break;
			case VENDOR_NAME:						/* 8 */
				memcpy(pstL2TPInfo->szVendorName, &pL2TPAvpHdr->AVPValue[0], avp_len-6);
				pstL2TPInfo->szVendorName[avp_len-6] = 0x00;
				break;
			case ASSIGNED_TUNNEL_ID:				/* 9 */
				pstL2TPInfo->usAssignedTunnelID = pntohs(&pL2TPAvpHdr->AVPValue[0]);
				break;
			case RECEIVE_WINDOW_SIZE:				/* 10 */
				pstL2TPInfo->usReceiveWindowSize = pntohs(&pL2TPAvpHdr->AVPValue[0]);
				break;
			case CHALLENGE:							/* 11 */
				memcpy(pstL2TPInfo->szChallenge, &pL2TPAvpHdr->AVPValue[0], avp_len-6);
				pstL2TPInfo->szChallenge[avp_len-6] = 0x00;
				break;
			case CAUSE_CODE:						/* 12 */
				break;
			case CHALLENGE_RESPONSE:				/* 13 */
				memcpy(pstL2TPInfo->szChallengeResp, &pL2TPAvpHdr->AVPValue[0], avp_len-6);
				pstL2TPInfo->szChallengeResp[avp_len-6] = 0x00;
				break;
			case ASSIGNED_SESSION:					/* 14 */
				pstL2TPInfo->usAssignedSessionID = pntohs(&pL2TPAvpHdr->AVPValue[0]);
				break;
			case CALL_SERIAL_NUMBER:				/* 15 */
				pstL2TPInfo->uiCallSerialNumber = pntohl(&pL2TPAvpHdr->AVPValue[0]);
				break;
			case MINIMUM_BPS:						/* 16 */
			case MAXIMUM_BPS:						/* 17 */
			case BEARER_TYPE:						/* 18 */
			case FRAMING_TYPE:						/* 19 */
				break;
			case CALLED_NUMBER:						/* 21 */
				memcpy(pstL2TPInfo->szCalledNumber, &pL2TPAvpHdr->AVPValue[0], avp_len-6);
				pstL2TPInfo->szCalledNumber[avp_len-6] = 0x00;
				break;
			case CALLING_NUMBER:					/* 22 */
				memcpy(pstL2TPInfo->szCallingNumber, &pL2TPAvpHdr->AVPValue[0], avp_len-6);
				pstL2TPInfo->szCallingNumber[avp_len-6] = 0x00;
				break;
			case SUB_ADDRESS:						/* 23 */
			case TX_CONNECT_SPEED:					/* 24 */
				pstL2TPInfo->uiTxConnectSpeed = pntohl(&pL2TPAvpHdr->AVPValue[0]);
				break;
			case PHYSICAL_CHANNEL:					/* 25 */
			case INITIAL_RECEIVED_LCP_CONFREQ:		/* 26 */
				break;
			case LAST_SENT_LCP_CONFREQ:				/* 27 */
				memcpy(pstL2TPInfo->szSentLCPConfReq, &pL2TPAvpHdr->AVPValue[0], avp_len-6);
				pstL2TPInfo->szSentLCPConfReq[avp_len-6] = 0x00;
				pstL2TPInfo->ucSendLCPFlag = 1;
				break;
			case LAST_RECEIVED_LCP_CONFREQ:			/* 28 */
				memcpy(pstL2TPInfo->szRecvLCPConfReq, &pL2TPAvpHdr->AVPValue[0], avp_len-6);
				pstL2TPInfo->szRecvLCPConfReq[avp_len-6] = 0x00;
				pstL2TPInfo->ucRecvLCPFlag = 1;
				break;
			case PROXY_AUTHEN_TYPE:					/* 29 */
				pstL2TPInfo->usProxyAuthenType = pntohs(&pL2TPAvpHdr->AVPValue[0]);
				break;
			case PROXY_AUTHEN_NAME:					/* 30 */
				memcpy(pstL2TPInfo->szProxyAuthenName, &pL2TPAvpHdr->AVPValue[0], avp_len-6);
				pstL2TPInfo->szProxyAuthenName[avp_len-6] = 0x00;
				break;
			case PROXY_AUTHEN_CHALLENGE:			/* 31 */
				pstL2TPInfo->ucCHAPChalFlag = 1;
				break;
			case PROXY_AUTHEN_ID:					/* 32 */
				pstL2TPInfo->ucAUTHID = pL2TPAvpHdr->AVPValue[1];
				break;
			case PROXY_AUTHEN_RESPONSE:				/* 33 */
				pstL2TPInfo->ucCHAPRespFlag = 1;
				break;
			case CALL_STATUS_AVPS:					/* 34 */
			case ACCM:								/* 35 */
			case RANDOM_VECTOR:						/* 36 */
			case PRIVATE_GROUP_ID:					/* 37 */
				break;
			case RX_CONNECT_SPEED:					/* 38 */
				pstL2TPInfo->uiRxConnectSpeed = pntohl(&pL2TPAvpHdr->AVPValue[0]);
				break;
			case SEQUENCING_REQUIRED:				/* 39 */
				break;
			default:
				log_print(LOGN_WARN, "DECODE] UNKNOWN AVP TYPE: %d LEN: %d VENDOR: %d", avp_type, avp_len, avp_vendor_id);
				break;
		}
	}

	return dResult;
}


