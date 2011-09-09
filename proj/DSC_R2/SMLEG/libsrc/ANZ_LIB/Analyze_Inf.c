/**********************************************************
                 KTF IPAS Project

   Author   : MI HEE HONG
   Section  : IPAS Project
   SCCS ID  : ANZ_LIB
   Date     : 06/07/04
   Revision History :
        '03.    1. 15   Initial
		'04.   06. 07	Update By LSH For Padding Data Size

   Description:
        IPAF ETHERNET Analyzing Library

   Copyright (c) ABLEX 2003, and 2004
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

/**B.1*  Definition of New Constants **********************/
/**B.2*  Definition of New Type  **************************/
/**C.1*  Declaration of Variables  ************************/
/**D.1*  Definition of Functions  *************************/

// yhshin
#define TOUSHORT(x) (USHORT)(*(x)<<8|*(x+1))
#define TOULONG(x)  (ULONG)(*(x)<<24|*(x+1)<<16|*(x+2)<<8|*(x+3))
//

#define LOUCHAR(w)	((UCHAR)((USHORT)(w) & 0xff))
#define HIUCHAR(w)  ((UCHAR)((USHORT)(w) >> 8))

unsigned short CheckSum( void *addr, long count);
int Analyze_Info_IP_UDP(PUCHAR pBuf, DWORD dwSize, ANALYZE_INFO *pInfo, PSU_RHDR *ppseudo);
int Analyze_Info_IP_TCP(PUCHAR pBuf, DWORD dwSize, ANALYZE_INFO *pInfo, PSU_RHDR *ppseudo);
int Analyze_Info_IP(PUCHAR pBuf, DWORD dwSize, ANALYZE_INFO *pInfo, BOOL bEthernet, struct slcompress *pComp);
int Init_Analyze_Info(ANALYZE_INFO *pInfo);
int AnalyzeEth_Info(PUCHAR pBuf, DWORD dwSize, ANALYZE_INFO *pInfo, struct slcompress *pComp);

// jjinr
int Analyze_IP( PUCHAR pBuf, DWORD dwSize, pst_IPTCPHeader pstIPTCP);
int Analyze_UDP(PUCHAR pBuf, DWORD dwSize, pst_IPTCPHeader pstIPTCP);

/**D.2*  Definition of Functions  *************************/
int AnalyzeEth_Info(PUCHAR pBuf, DWORD dwSize, ANALYZE_INFO *pInfo, struct slcompress *pComp)
{
	int             dResult = 1;
	PETHERNET_HDR   pEthFrame;

	pEthFrame = (PETHERNET_HDR)pBuf;

	/* Packet Type [ethernet] */
	pInfo->nVer = 0x1002;

	/* Packet length */
	pInfo->dwLength = dwSize;

	/* ethernet length check */
	if( dwSize < 60 || dwSize > 1518 )
		return ERR_200101; 
	
	pInfo->dLevel = NUM_MAC;

	/* MAC address *
	memcpy(pInfo->stMAC.strDestMACAddr, pEthFrame->Destination, 6);
	memcpy(pInfo->stMAC.strSrcMACAddr, pEthFrame->Source, 6);
	*/

	/* mac type or ieee802.3 length field */
	pInfo->wFrameType = TOUSHORT(pEthFrame->Protocol);
//	printf("pInfo->wFrameType: %d.  \n", TOUSHORT(pEthFrame->Protocol));

	/* IP HEADER init */
	pInfo->stIP.bIPHeader = FALSE;
	
//	fprintf (stderr, "pInfo->wFrameType : %02x%02x\n", pEthFrame->Protocol[0], pEthFrame->Protocol[1]);
//	fprintf (stderr, "pInfo->wFrameType : %x. %02x\n", pInfo->wFrameType, pEthFrame->Data[0]);
//	return -1;


#if 0
	/* in case of the mac type of 0x0800, that is, ip datagram,
	   analysizing the packet in more detail.  */
	if (pInfo->wFrameType <= 0x05DC)
	{
		/* 802.3 */
		pInfo->dLevel = NUM_802_3;

		/* length check */
		if( pInfo->wFrameType > dwSize-14 )
			return ERR_200101;
		
		pInfo->st802_3.b802_3 = TRUE;
		pInfo->st802_3.DSAP = pEthFrame->Data[0];
		pInfo->st802_3.SSAP = pEthFrame->Data[1];

		if (pEthFrame->Data[0] == 0x42 && pEthFrame->Data[1] == 0x42)
			pInfo->dLevel = NUM_802_3_BPDU;
		else if (pEthFrame->Data[0] == 0x06 && pEthFrame->Data[1] == 0x06) {
			dResult = Analyze_Info_IP(
				&pEthFrame->Data[2], dwSize - (DEF_ETHERNET_HDR_SIZE-1) - 2, pInfo, TRUE, pComp);
		}
		else if (pEthFrame->Data[0] == 0xF0 && pEthFrame->Data[1] == 0xF0)
			pInfo->dLevel = NUM_802_3_NETBEUI;
	}
	else if (pInfo->wFrameType == PROTO_IP)
	{
		pInfo->dLevel = NUM_DIX;
		dResult = Analyze_Info_IP(
			pEthFrame->Data, dwSize - (DEF_ETHERNET_HDR_SIZE-1), pInfo, TRUE, pComp);
	}
	else if (pInfo->wFrameType == PROTO_ARP)
		pInfo->dLevel = NUM_DIX;
	else if (pInfo->wFrameType == PROTO_RARP)
		pInfo->dLevel = NUM_DIX;
#else
	
	pInfo->dLevel = NUM_DIX;
	dResult = Analyze_Info_IP(
			pEthFrame->Data, dwSize - (DEF_ETHERNET_HDR_SIZE-1), pInfo, TRUE, pComp);
#endif
	return dResult;
}

int Init_Analyze_Info(ANALYZE_INFO *pInfo)
{
	int dResult = 1;

	pInfo->nVer = 0;
	pInfo->wFrameType = 0;
	pInfo->dwMsgType = 0;
	pInfo->dwLength = 0;

	pInfo->dPaddingSize = 0;
	pInfo->dLevel = 0;

	/* stMAC */
	pInfo->stMAC.bMAC = FALSE;
	memset(pInfo->stMAC.strDestMACAddr, 0x00, sizeof(pInfo->stMAC.strDestMACAddr));
	memset(pInfo->stMAC.strSrcMACAddr, 0x00, sizeof(pInfo->stMAC.strSrcMACAddr));

	/* st802_3 */
	pInfo->st802_3.b802_3 = FALSE;
	pInfo->st802_3.DSAP = 0;
	pInfo->st802_3.SSAP = 0;

	/* stIP */
	pInfo->stIP.bIPHeader = FALSE;
	pInfo->stIP.ucProtocol = 0;
	pInfo->stIP.IPVersion = 0;
	pInfo->stIP.Timelive = 0;
	pInfo->stIP.Type = 0;
	pInfo->stIP.wLength = 0;
	pInfo->stIP.wIdent = 0;
	pInfo->stIP.wIPFrag = 0;
	pInfo->stIP.dwSrcIP = 0;
	pInfo->stIP.dwDestIP = 0;
	//memset(pInfo->stIP.ucDestIP, 0x00, 4);

	/* stUDPTCP */
	pInfo->stUDPTCP.dUDPTCP = 0;
	pInfo->stUDPTCP.nControlType = 0;
	pInfo->stUDPTCP.wSrcPort = 0;
	pInfo->stUDPTCP.wDestPort = 0;
	pInfo->stUDPTCP.seq = 0;
	pInfo->stUDPTCP.ack = 0;
	pInfo->stUDPTCP.data = 0;
	pInfo->stUDPTCP.window = 0;
	//pInfo->stUDPTCP.ucCode = 0;

	return dResult;
}

int Analyze_Info_IP(
		PUCHAR pBuf, DWORD dwSize, ANALYZE_INFO *pInfo, BOOL bEthernet, struct slcompress *pComp)
{
	int     dResult = 1;
	DWORD   wIPHeaderLen;
	DWORD   wTotalIPLen;
	DWORD   wIPDataLen;
	PIP_RHDR pIP = (PIP_RHDR)pBuf;
	PSU_RHDR pseudo;

	/* IP version check */
	pInfo->stIP.IPVersion = pIP->VerLen & 0xf0;

	if( pInfo->stIP.IPVersion == 0x40 )
		pInfo->stIP.bIPHeader = 1;	/* IPv4 */
	else if( pInfo->stIP.IPVersion == 0x60 )
	{
		pInfo->stIP.bIPHeader = 2;	/* IPv6, not supported IP version */
		return ERR_300201;
	}
   	else 
		return ERR_300103;			/* unknown IP version */

	/*** BY LEE 2004.03.22 ***/
	pInfo->stIP.IPVersion = pIP->VerLen;

	/* IP total length check */
	wTotalIPLen = TOUSHORT(pIP->Length);

	/* Update By LSH for Padding Size */
	pInfo->dPaddingSize = dwSize - wTotalIPLen;
	if(pInfo->dPaddingSize < 0)
		return ERR_300102;

	pInfo->stIP.wLength = wTotalIPLen;

	/* IP header length check */	
	wIPHeaderLen = (pIP->VerLen & 0x0f) * 4;
	if( wIPHeaderLen < 20 || wIPHeaderLen > wTotalIPLen )
		return ERR_300101;

	/* 2005.04.11 FOR IP FRAG */
	pInfo->stIP.wIdent = TOUSHORT(pIP->Ident);
	pInfo->stIP.wIPFrag = TOUSHORT(pIP->Flagoff);

	switch (pInfo->dLevel)
		{
		case NUM_DIX :
			pInfo->dLevel = NUM_IP;
			break;

		case NUM_802_3 :
			pInfo->dLevel = NUM_802_3_IP;
			break;

		case NUM_GRE_PPP :
			pInfo->dLevel = NUM_GRE_PPP_IP;
			break;

		case NUM_PPP :
			pInfo->dLevel = NUM_PPP_IP;
			break;
		}	

	/* IP header checksum */	
	if (CheckSum(pIP, wIPHeaderLen) == 0)
		pInfo->stIP.bChecksumErr = 0;
	else
		pInfo->stIP.bChecksumErr = 1;

	/* TimeToLive */
	pInfo->stIP.Timelive = pIP->Timelive;
	
	/* Protocol */
	pInfo->stIP.ucProtocol = pIP->Protocol;

	/* Src IP Addr */
	pInfo->stIP.dwSrcIP = TOULONG(pIP->Source);

	/* Dest IP Addr */
	pInfo->stIP.dwDestIP = TOULONG(pIP->Destination);
		
	wIPDataLen = wTotalIPLen - wIPHeaderLen;
	if (wIPDataLen > 0)
	{
		memcpy( pseudo.Source, pIP->Source, 4);		
		memcpy( pseudo.Destination, pIP->Destination, 4);		
		pseudo.Zero		= 0x0;
		pseudo.Protocol = pIP->Protocol;		
		pseudo.Length[0] = HIUCHAR(wIPDataLen);
		pseudo.Length[1] = LOUCHAR(wIPDataLen);

		switch (pInfo->stIP.ucProtocol)
		{
		case 1:		/* ICMP : MODIFIED BY LDH 2006.02.08 */
			//dResult = ERR_310101;
			dResult = 0;
			break;
		case 6:		/* TCP */
			dResult = Analyze_Info_IP_TCP(pIP->Data, wIPDataLen, pInfo, &pseudo);
			break;

		case 17:	/* UDP */
			dResult = Analyze_Info_IP_UDP(pIP->Data, wIPDataLen, pInfo, &pseudo);
			break;

		default:	/* other IP protocol */
			break;
		}
	}
	
	return dResult;
}

int Analyze_Info_IP_TCP(PUCHAR pBuf, DWORD dwSize, ANALYZE_INFO *pInfo, PSU_RHDR *ppseudo)
{
    int         dResult = 1;
    DWORD       wTCPHeaderLen;
    DWORD       wTCPDataLen;
    PTCP_RHDR   pTCP = (PTCP_RHDR)pBuf;
    UCHAR       uBuffer[2000];

	/* TCP check */
    pInfo->stUDPTCP.dUDPTCP = 2;

	/* length check */
    wTCPHeaderLen = pTCP->Offset*4;
	if( wTCPHeaderLen < 20 )
	   	return ERR_370102;

	if( dwSize < wTCPHeaderLen)
		return ERR_370101;

	switch (pInfo->dLevel)
	{
	case NUM_IP:
		pInfo->dLevel = NUM_TCP;
		break;

	case NUM_PPP_IP:
		pInfo->dLevel = NUM_PPP_TCP;
		break;

	case NUM_802_3_IP:
		pInfo->dLevel = NUM_802_3_TCP;
		break;
	}

	memcpy(uBuffer, ppseudo, 12);
	memcpy(uBuffer+12, pTCP, dwSize);
	if (CheckSum(uBuffer, 12+dwSize) == 0)
		pInfo->stUDPTCP.bChecksumErr = 0;
	else
		pInfo->stUDPTCP.bChecksumErr = 1;

	wTCPDataLen = dwSize - wTCPHeaderLen;

    /* Control Type */
    pInfo->stUDPTCP.nControlType = pTCP->Flags;

	/* Source Port */
	pInfo->stUDPTCP.wSrcPort = TOUSHORT(pTCP->Source);
		
	/* Destination Port */
	pInfo->stUDPTCP.wDestPort = TOUSHORT(pTCP->Destination);
		
	/* TCP length */
	pInfo->stUDPTCP.wLength = (unsigned short)dwSize;
	pInfo->stUDPTCP.wDataLen = wTCPDataLen;

	/* for TCP session trace */
	pInfo->stUDPTCP.seq = TOULONG(pTCP->Seq);
    pInfo->stUDPTCP.ack = TOULONG(pTCP->Ack);
    pInfo->stUDPTCP.window = TOUSHORT(pTCP->Window);

    return dResult;
}

int Analyze_Info_IP_UDP(PUCHAR pBuf, DWORD dwSize, ANALYZE_INFO *pInfo, PSU_RHDR *ppseudo)
{
    int         dResult = 1;
	PUDP_RHDR   pUDP = (PUDP_RHDR)pBuf;
    DWORD       wUDPHeaderLen;
    DWORD       wUDPDataLen;
	UCHAR		uBuffer[2000];

    /* UDP check */
    pInfo->stUDPTCP.dUDPTCP = 1;

    /* Source Port */
    pInfo->stUDPTCP.wSrcPort = TOUSHORT(pUDP->Source);
		
	/* Destination Port */
	pInfo->stUDPTCP.wDestPort = TOUSHORT(pUDP->Destination);
    
	/* UDP length */
	wUDPHeaderLen = DEF_UDP_RHDR_SIZE-1;

	if (dwSize < wUDPHeaderLen)
	{
		return ERR_360101;
	}
    
	wUDPDataLen = dwSize - wUDPHeaderLen;
	
	pInfo->stUDPTCP.wLength = (unsigned short)dwSize;
	pInfo->stUDPTCP.wDataLen = wUDPDataLen;

	switch (pInfo->dLevel)
	{
	case NUM_IP:
		pInfo->dLevel = NUM_UDP;
		break;

	case NUM_PPP_IP:
		pInfo->dLevel = NUM_PPP_UDP;
		break;

	case NUM_802_3_IP:
		pInfo->dLevel = NUM_802_3_UDP;
		break;
	}

	/* 20041019 NOT CHECK UDP CHECKSUM FOR 0X00 0X00 */
	if( pUDP->Checksum[0] == 0x00 && pUDP->Checksum[1] == 0x00 ) {
		pInfo->stUDPTCP.bChecksumErr = 0;

		return dResult;
	}

	/* 20041019 CHECKSUM VALUE 0XFFFF */
	if( pUDP->Checksum[0] == 0xFF && pUDP->Checksum[1] == 0xFF ) {
		pUDP->Checksum[0] = 0x00;
		pUDP->Checksum[1] = 0x00;
	}

	memcpy(uBuffer, ppseudo, 12);
	memcpy(uBuffer+12, pUDP, dwSize);
	if (CheckSum(uBuffer, 12+dwSize) == 0)
		pInfo->stUDPTCP.bChecksumErr = 0;
	else {
		pInfo->stUDPTCP.bChecksumErr = 1;
	}

	return dResult;
}

int Analyze_IP( PUCHAR pBuf, DWORD dwSize, pst_IPTCPHeader pstIPTCP)
{
	DWORD   wIPHeaderLen;
	DWORD   wTotalIPLen, wPadSize = 0;
	DWORD   wIPDataLen = 0;
	PIP_RHDR pIP = (PIP_RHDR)pBuf;

	/* IP version check */

	if( (pIP->VerLen & 0xf0) == 0x40 )
	{
		/* IP total length check */
		wTotalIPLen = TOUSHORT(pIP->Length);

		/* Update By LSH for Padding Size */
		wPadSize = dwSize - wTotalIPLen;
		if( wPadSize < 0 )
			return ERR_300102;

		pstIPTCP->stIPHeader.usTotLen = wTotalIPLen;

		/* IP header length check */	
		pstIPTCP->stIPHeader.usIPHeaderLen = wIPHeaderLen = (pIP->VerLen & 0x0f) * 4;
		if( wIPHeaderLen < 20 || wIPHeaderLen > wTotalIPLen )
			return ERR_300101;

		/* 2005.04.11 FOR IP FRAG */
		pstIPTCP->stIPHeader.usIdentification = TOUSHORT(pIP->Ident);
		pstIPTCP->stIPHeader.uiIPFrag = TOUSHORT(pIP->Flagoff);

		/* Protocol */
		pstIPTCP->stIPHeader.ucProtocol = pIP->Protocol;

		/* Src IP Addr */
		pstIPTCP->stIPHeader.dSrcIP = TOULONG(pIP->Source);

		/* Dest IP Addr */
		pstIPTCP->stIPHeader.dDestIP = TOULONG(pIP->Destination);

		wIPDataLen = wTotalIPLen - wIPHeaderLen;
		if (wIPDataLen > 0)
			return pstIPTCP->stIPHeader.ucProtocol;
		else
			return -1;
	}
	else if( (pIP->VerLen & 0xf0) == 0x60 )
		return ERR_300201;
   	else 
		return ERR_300103;			/* unknown IP version */

}

int Analyze_UDP(PUCHAR pBuf, DWORD dwSize, pst_IPTCPHeader pstIPTCP)
{
    int         dResult = 1;
	PUDP_RHDR   pUDP = (PUDP_RHDR)pBuf;
    DWORD       wUDPHeaderLen;
    DWORD       wUDPDataLen;

    /* Source Port */
    pstIPTCP->stTCPHeader.usSrcPort = TOUSHORT(pUDP->Source);
		
	/* Destination Port */
	pstIPTCP->stTCPHeader.usDestPort = TOUSHORT(pUDP->Destination);
    
	/* UDP length */
	wUDPHeaderLen = DEF_UDP_RHDR_SIZE-1;

	if (dwSize < wUDPHeaderLen)
		return ERR_360101;
    
	wUDPDataLen = dwSize - wUDPHeaderLen;
	
	pstIPTCP->stTCPHeader.usTCPHeaderLen = wUDPHeaderLen;
	pstIPTCP->stTCPHeader.usDataLen = wUDPDataLen;

	return dResult;
}


unsigned short CheckSum( void *addr, long count) 
{
	register long sum = 0;
	unsigned short checksum;
 
	while( count > 1 )  
	{
		//  This is the inner loop 
		sum +=  *(unsigned short *) addr;
		addr = (unsigned short *) addr+1;

		count -= 2;
	}
 
	// Add left-over byte, if any 
	if( count > 0 )
		sum += * (unsigned char *) addr;
 
	// Fold 32-bit sum to 16 bits 
	while (sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);
 
	checksum = ~sum;

	return checksum;
}
