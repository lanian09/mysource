// Analyze_PPP.c 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>
#include <stdint.h>

#include <typedef.h>
#include <PPP_header.h>
#include <Errcode.h>
#include <Analyze_Ext_Abs.h>

#define TOUSHORT(x) (USHORT)(*(x)<<8|*(x+1))
#define TOULONG(x)  (ULONG)(*(x)<<24|*(x+1)<<16|*(x+2)<<8|*(x+3))

#define	LOUCHAR(w)	((UCHAR)((USHORT)(w) & 0xff))
#define	HIUCHAR(w)	((UCHAR)((USHORT)(w) >> 8))

static unsigned short fcstab[256] = {
      0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
      0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
      0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
      0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
      0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
      0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
      0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
      0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
      0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
      0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
      0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
      0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
      0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
      0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
      0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
      0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
      0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
      0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
      0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
      0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
      0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
      0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
      0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
      0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
      0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
      0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
      0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
      0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
      0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
      0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
      0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
      0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};

void GetPPP7EState(PUCHAR pBuf, WORD wSize, int *nPPPState, int *n7EPosition);
BOOL DecodePPP(PUCHAR *ppDest, WORD *pwDestSize, PUCHAR pSrc, WORD wSrcSize);
int CheckFCS(PUCHAR pOrgData, WORD wOrgSize);
int Analyze_PPP_LCP(PUCHAR pBuf, WORD wSize, ANALYZE_INFO_PPP *pInfo);
int Analyze_PPP_PAP(PUCHAR pBuf, WORD wSize, ANALYZE_INFO_PPP *pInfo);
int Analyze_PPP_IPCP(PUCHAR pBuf, WORD wSize, ANALYZE_INFO_PPP *pInfo);
int Analyze_PPP_CHAP(PUCHAR pBuf, WORD wSize, ANALYZE_INFO_PPP *pInfo);
int Analyze_PPP_CCP(PUCHAR pBuf, WORD wSize, ANALYZE_INFO_PPP *pInfo);



/*************************************************
func   : Analyze_L2TP_PPP()
def    : Analyze L2TP PPP protocol 
*************************************************/
int Analyze_L2TP_PPP(PUCHAR pBuf, WORD wSize, INFO_PPP *pInfo, struct slcompress *pComp)
{
    int         dResult = 1;
//	int         n7EPos;
    WORD       wPPPSize;
    WORD        wProtocol;
    WORD        wProtocolSize;
    WORD        wHeaderSize = 0;
    PUCHAR      pTemp = NULL;
    PUCHAR      pData;
    PUCHAR      pProtocol;
    PUCHAR      pStart;

    if(wSize < 1)
        return ERR_010101;

	/* PPP status check 
    GetPPP7EState( pBuf, wSize, &pInfo->stPPP.nPPPState, &n7EPos );
    if( pInfo->stPPP.nPPPState != F_DATA_F )    / 7e + data + 7e /
        return ERR_010101;
	* REMOVED BY LDH FOR L2TP PPP */

    /* PPP Header *
    DecodePPP(&pTemp, &wPPPSize, pBuf, wSize);
	* REMOVED BY LDH FOR L2TP PPP */

	/* FOR L2TP PPP */
	pTemp 		= pBuf;
	wPPPSize 	= wSize;

	/*
    if( CheckFCS(pTemp, wPPPSize) != 1 )
        pInfo->stPPP.bFCSError = 1;
	* REMOVED BY LDH FOR L2TP PPP */

    // PPP check
    pInfo->stPPP.bPPP = TRUE;

    // PPP Size
    pInfo->stPPP.wPPPSize = wPPPSize;

	/*
    if( pTemp[1] == 0xff && pTemp[2] == 0x03) {
	* MODIFIED BY LDH FOR L2TP PPP */
	if( pTemp[0] == 0xff && pTemp[1] == 0x03) {
        pProtocol = pTemp+2;
        wHeaderSize = 2;
    }
    else {
//		pProtocol = pTemp+1;
		pProtocol = pTemp;
        wHeaderSize = 0;
    }

    /* PROTOCOL */
    if (pProtocol[0] == 0x21 || pProtocol[0] == 0x2D || pProtocol[0] == 0x2F) {
		/* IP, VJC TCPIP, VJUC TCPIP */
        wProtocol 		= pProtocol[0];
        wProtocolSize 	= 1;
        pStart 			= pProtocol+1;
    }
    else {
        wProtocol 		= (pProtocol[0] << 8) + pProtocol[1];
        wProtocolSize 	= 2;
        pStart 			= pProtocol+2;
    }

	wHeaderSize += wProtocolSize;
    pData 		= pProtocol+wProtocolSize;

    pInfo->stPPP.wProtocol = wProtocol;

    /* PPP Data */
    switch( wProtocol )
	{
		/* LCP (Link Control Protocol) */
		case PPP_LCP:
//			if( wPPPSize <= (wHeaderSize+3) )
//				return ERR_010104;
//			dResult = Analyze_PPP_LCP(pData, wPPPSize-wHeaderSize-3, &pInfo->stPPP);
			dResult = Analyze_PPP_LCP(pData, wPPPSize-wHeaderSize, &pInfo->stPPP);
			break;

			/* PAP (Password Authentication Protocol) */
		case PPP_PAP:
//			if( wPPPSize <= (wHeaderSize+3) )
//				return ERR_010106;
//			dResult = Analyze_PPP_PAP(pData, wPPPSize-wHeaderSize-3, &pInfo->stPPP);
			dResult = Analyze_PPP_PAP(pData, wPPPSize-wHeaderSize, &pInfo->stPPP);
			break;

			/* IPCP (Internet Protocol Control Protocol */
		case PPP_IPCP:
//			if( wPPPSize <= (wHeaderSize+3) )
//				return ERR_010107;
//			dResult = Analyze_PPP_IPCP(pData, wPPPSize-wHeaderSize-3, &pInfo->stPPP);
			dResult = Analyze_PPP_IPCP(pData, wPPPSize-wHeaderSize, &pInfo->stPPP);
			break;

			/* CHAP (Challenge Handshake Authentication Protocol */
		case PPP_CHAP:
//			if( wPPPSize <= (wHeaderSize+3) )
//			return ERR_010108;
//			dResult = Analyze_PPP_CHAP(pData, wPPPSize-wHeaderSize-3, &pInfo->stPPP);
			dResult = Analyze_PPP_CHAP(pData, wPPPSize-wHeaderSize, &pInfo->stPPP);
			break;

		case PPP_CCP1:
		case PPP_CCP2:
//			if( wPPPSize <= (wHeaderSize+3) )
//				return ERR_010109;
//			dResult = Analyze_PPP_CCP(pData, wPPPSize-wHeaderSize-3, &pInfo->stPPP);
			dResult = Analyze_PPP_CCP(pData, wPPPSize-wHeaderSize, &pInfo->stPPP);
			break;

		case PPP_IP:
			/* DO NOTHING */
		case PPP_VJUCTCPIP:
			/* DO NOTHING */
		case PPP_VJCTCPIP:
			/* DO NOTHING */
			break;

		default:
			break;
	}

    return dResult;
}


/*************************************************
func   : Analyze_PPP()
def    : Analyze PPP protocol 
*************************************************/ 
int Analyze_PPP(PUCHAR pBuf, WORD wSize, INFO_PPP *pInfo, struct slcompress *pComp)
{
	int         dResult = 1;
	int      	n7EPos;
    WORD       wPPPSize;
    WORD        wProtocol;
	WORD        wProtocolSize;
	WORD        wHeaderSize = 0;
	PUCHAR		pTemp = NULL;
	PUCHAR      pData;
	PUCHAR      pProtocol;
	PUCHAR      pStart;

	if(wSize < 1)
		return ERR_010101;

	/***
	if(wSize > 1800)
		return ERR_010103;
	***/

	/*** counting for 0x7e
    pInfo->stPPP.c7ECount = 0;
	for (i = 0; i < wSize; i++)
		if (pBuf[i] == 0x7e)
			pInfo->stPPP.c7ECount++;
	***/

	// PPP status check 
	GetPPP7EState( pBuf, wSize, &pInfo->stPPP.nPPPState, &n7EPos );
    if( pInfo->stPPP.nPPPState != F_DATA_F )	/* 7e + data + 7e */
	  	return ERR_010101;

    //pInfo->stPPP.c7ECount = 2;

    /* PPP Header */
   	DecodePPP(&pTemp, &wPPPSize, pBuf, wSize);

	if( CheckFCS(pTemp, wPPPSize) != 1 )
		pInfo->stPPP.bFCSError = 1;

    // PPP check
    pInfo->stPPP.bPPP = TRUE;

    // PPP Size
    pInfo->stPPP.wPPPSize = wPPPSize;
	
	if( pTemp[1] == 0xff && pTemp[2] == 0x03)
	{
		pProtocol = pTemp+3;
		wHeaderSize = 3;
	}
	else
	{
		pProtocol = pTemp+1;
		wHeaderSize = 1;
	}

	// Protocol
	if (pProtocol[0] == 0x21		// IP		
		|| pProtocol[0] == 0x2D		// VJC TCPIP
		|| pProtocol[0] == 0x2F)	// VJUC TCPIP
	{
		wProtocol = pProtocol[0];
		wProtocolSize = 1;
		pStart = pProtocol+1;
	}
	else
	{
		wProtocol = (pProtocol[0] << 8) + pProtocol[1];
		wProtocolSize = 2;
		pStart = pProtocol+2;
	}

	wHeaderSize += wProtocolSize;
	pData = pProtocol+wProtocolSize;

	pInfo->stPPP.wProtocol = wProtocol;

	/* PPP Data */
	switch( wProtocol )
	{
		/* LCP (Link Control Protocol) */
		case PPP_LCP:
			if( wPPPSize <= (wHeaderSize+3) )
				return ERR_010104;
			dResult = Analyze_PPP_LCP(pData, wPPPSize-wHeaderSize-3, &pInfo->stPPP);
		break;

		/* PAP (Password Authentication Protocol) */
		case PPP_PAP:
			if( wPPPSize <= (wHeaderSize+3) )
				return ERR_010106;
			dResult = Analyze_PPP_PAP(pData, wPPPSize-wHeaderSize-3, &pInfo->stPPP);
			break;

		/* IPCP (Internet Protocol Control Protocol */
		case PPP_IPCP:
			if( wPPPSize <= (wHeaderSize+3) )
				return ERR_010107;
			dResult = Analyze_PPP_IPCP(pData, wPPPSize-wHeaderSize-3, &pInfo->stPPP);
			break;

		/* CHAP (Challenge Handshake Authentication Protocol */
		case PPP_CHAP:
			if( wPPPSize <= (wHeaderSize+3) )
				return ERR_010108;
			dResult = Analyze_PPP_CHAP(pData, wPPPSize-wHeaderSize-3, &pInfo->stPPP);
			break;

		case PPP_CCP1:
		case PPP_CCP2:
			if( wPPPSize <= (wHeaderSize+3) )
				return ERR_010109;
			dResult = Analyze_PPP_CCP(pData, wPPPSize-wHeaderSize-3, &pInfo->stPPP);
			break;

		/* IP (Internet Protocol) */
		case PPP_IP:
			/*
			if( wPPPSize <= (wHeaderSize+3) )
				return ERR_010110;
			dResult = Analyze_IP(pData, wPPPSize-wHeaderSize-3, &pInfo->stETH);
			break;
			*/

		case PPP_VJUCTCPIP:
			/*
			if( wPPPSize <= (wProtocolSize+4) )
				return ERR_010111;

			if (pComp == 0)
				return ERR_010102;

			sl_uncompress_tcp(pStart, wPPPSize-wProtocolSize-4, TYPE_UNCOMPRESSED_TCP, pComp, &nVJlen);

			if( pStart == 0 )
				return ERR_010102;

			pIP = (PIP_RHDR)pStart;
			pIP->Protocol = 0x06;
			dResult = Analyze_IP(pStart, wPPPSize-wProtocolSize-4, &pInfo->stETH);
			break;
			*/

		case PPP_VJCTCPIP:
			/*
			if( wPPPSize <= (wProtocolSize+4) )
				return ERR_010112;

			memset(uBuffer, 0x00, sizeof(uBuffer));
			memcpy(uBuffer+128, pStart, wPPPSize-wProtocolSize-4);
			pStart = sl_uncompress_tcp(uBuffer+128, wPPPSize-wProtocolSize-4, TYPE_COMPRESSED_TCP, pComp, &nVJlen);

			if( pStart == 0 )
				return ERR_010105;

			pIP = (PIP_RHDR)pStart;
			wTempSize = TOUSHORT(pIP->Length);
			pIP->Protocol = 0x06;

			dResult = Analyze_IP(pStart, wTempSize, &pInfo->stETH);
			//pInfo->stETH.pData = pBuf+wHeaderSize+wProtocolSize;
			pInfo->stETH.pAppData = pData+nVJlen;
			*/
			break;

		default:
			break;
		}

    return dResult;
}

/*************************************************
func   : IsCompletePPPPacket()
def    : - PPP protocol 
*************************************************/ 
int IsCompletePPPPacket(PUCHAR pBuf, DWORD wSize)
{
	DWORD       i = 0;
	BOOL       	bCheck = TRUE;

	int			dFirst = 0;
	int			dLast  = 0;
	int			dCount = 0;
	int			nState = 0;

	/* First */
	while ((i < wSize) && bCheck)
		{
		if (pBuf[i] == 0x7e)
			{
			switch (dFirst)
				{
				case 0:
					dFirst = 1;
					break;

				case 1:
					nState = 0;
					i--;
					bCheck = FALSE;
					break;

				case 2:
					dFirst = 0;
					dCount += 100;
					bCheck = FALSE;
					break;

				case 3:
					dFirst = 4;
					bCheck = FALSE;
					break;
				}
			}
		else
			{
			switch (dFirst)
				{
				case 0:
					dFirst = 3;
					break;

				case 1:
					dFirst = 2;
					break;
				}
			}

		i++;
		}

	/* Last */
    while (i < wSize)
        {
		if (pBuf[i] == 0x7e)
			{
            switch (dLast)
                {
                case 0:
                    dLast = 10;
                    break;

                case 20:
                    dLast = 0;
					dCount += 100;
                    break;
                }
            }
        else
            {
            switch (dLast)
                {
                case 10:
                    dLast = 20;
                    break;
                }
            }

        i++;
        }

	nState = dCount + dLast + dFirst;

	/* x				0 */
	/* 7e				1 */
	/* 7e + data		2 */
	/* data				3 */
	/* data + 7e		4 */

	/* nState return value form ( X Y Z )	  */

	/* X : '7e + data + 7e' Count */ 
	/* Y : Last State 			  */ 
	/* Z : First State 			  */ 

	/* example */
	/* 7e 								0 0 1 */
	/* 7e + data						0 0 2 */
	/* 7e + data + 7e					1 0 0 */
	/* 7e + data + 7e + 7e  			1 1 0 */
	/* 7e + data + 7e + 7e + data 		1 2 0 */

	/* data  							0 0 3 */
	/* data + 7e 						0 0 4 */
	/* data + 7e + 7e 					0 1 4 */
	/* data + 7e + 7e + data			0 2 4 */
	/* data + 7e + 7e + data + 7e		1 0 4 */
	/* data + 7e + 7e + data + 7e + 7e	1 1 4 */

	/* 7e + 7e 							0 1 1 */
	/* 7e + 7e + data					0 2 1 */
	/* 7e + 7e + data + 7e				1 0 1 */
	/* 7e + 7e + data + 7e + 7e  		1 1 1 */
	/* 7e + 7e + data + 7e + 7e + data 	1 2 1 */
 
	return nState;
}

/*
 * Get 7E state and first 7E position
 */
void GetPPP7EState(PUCHAR pBuf, WORD wSize, int *nPPPState, int *n7EPosition)
{
	DWORD i=0;

	// initial
	int nState = X;
	int	nFirst = -1;

	while (i < wSize)
	{
		if (pBuf[i] == 0x7E)
		{
			switch (nState)
			{
				case DATA_F:
					nState = DATA_F_F;
					break;

				case DATA_O:
					nState = DATA_F;
					nFirst = i;
					break;
	
				case X:
					nState = F;
					nFirst = i;
					break;

				case F:
					nState = F_F;
					break;

				case F_DATA:
					nState = F_DATA_F;
					break;
			}
		}
		else
		{
			switch (nState)
			{
				case F_F:
					nState = F_F_DATA;
					break;

				case DATA_F_F:
					nState = DATA_F_F_DATA;
					break;

				case DATA_F:
					nState = DATA_F_DATA;
					break;

				case X:
					nState = DATA_O;
					break;
			
				case F:
					nState = F_DATA;
					break;
			}
		}
		i++;
	}
	
	// return
	*nPPPState = nState;
	*n7EPosition = nFirst; 	
}

/*************************************************
func   : DecodePPP()
def    : Analyze PPP protocol 
*************************************************/ 
BOOL DecodePPP(PUCHAR *ppDest, WORD *pwDestSize, PUCHAR pSrc, WORD wSrcSize)
{
	static UCHAR    Buffer[65535];
	BOOL            bStuffing = FALSE;
	DWORD           i;
    DWORD           j;

	i = j = 0;

	while (i < wSrcSize)
	{
		if (pSrc[i] == 0x7d && (i+1) < wSrcSize)
		{
			Buffer[j] = pSrc[i+1] ^ 0x20;
			bStuffing = TRUE;
			i += 2;
			j++;
		}
		else
		{
			Buffer[j] = pSrc[i];
			i++;
			j++;
		}
	}

	*ppDest = Buffer;
	*pwDestSize = j;

	return bStuffing;
}

/*************************************************
func   : Analyze_PPP_CCP()
def    : Analyze PPP - CCP protocol 
documnet : RFC 1962
*************************************************/ 
int Analyze_PPP_CCP(PUCHAR pBuf, WORD wSize, ANALYZE_INFO_PPP *pInfo)
{
	int                 dResult = 1;
    int                 i;  
	PPPP_DATA_HEADER    pHeader;
/*
	static struct _data_type {
		UCHAR Type;
		char  Desc[40];
	} data_type[15] =
		{
			{ 0x00, "OUI"                  },
			{ 0x01, "Predictor type 1"     },
			{ 0x02, "Predictor type 2"     },
			{ 0x03, "Puddle Jumper"        },
			{ 0x10, "Hewlett-Packard PPC"  },
			{ 0x11, "Stac Electronics LZS" },		// 1974
			{ 0x12, "Microsoft PPC"        },		// 2118
			{ 0x13, "Gandalf FZA"          },
			{ 0x14, "V.42bis compression"  },
			{ 0x15, "BSD Compress"         },		// 1977
			{ 0x17, "LZS-DCP"              },		// 1967
			{ 0x18, "MVRCA (Magnalink)"    },		// 1975
			{ 0x19, "DCE"                  },		// 1976
			{ 0x1a, "Deflate"              },		// 1979
			{ 0xff, "Reserved"             }
		};
*/
	/* Code */
	static struct _ccp_code {
		UCHAR Code;
		char  Desc[30];
	} ccp_code[9] =
		{
			{ 0x01, "Configure-request" },
			{ 0x02, "Configure-ack"     },
			{ 0x03, "Configure-nak"     },
			{ 0x04, "Configure-reject"  },
			{ 0x05, "Terminate-request" },
			{ 0x06, "Terminate-ack"     },
			{ 0x07, "Code-reject"       },
			{ 0x0e, "Reset-request"     },
			{ 0x0f, "Reset-ack"         }
		};

    if (wSize < sizeof(PPP_DATA_HEADER)-1) 
        return ERR_020101;

    pHeader = (PPPP_DATA_HEADER)pBuf;

	for (i = 0; i < 9; i++)
		if (pHeader->Code == ccp_code[i].Code)
			break;

	if(i < 9)
	{
		pInfo->ucCode = pHeader->Code;
		pInfo->ucID = pHeader->ID;
	}
    return dResult;
}

/*************************************************
func   : Analyze_PPP_CHAP()
def    : Analyze PPP - CHAP protocol 
*************************************************/ 
int Analyze_PPP_CHAP(PUCHAR pBuf, WORD wSize, ANALYZE_INFO_PPP *pInfo)
{
	int                 dResult = 1;
    int                 i;
	PPPP_DATA_HEADER    pHeader;
	WORD				Length;
	short				DataLength;
	UCHAR				TempLen;
	
	/* Code */
	static struct _chap_code {
		UCHAR Code;
		char  Desc[10];
	} chap_code[4] =
		{
			{ 0x01, "Challenge" },
			{ 0x02, "Response"  },
			{ 0x03, "Success"   },
			{ 0x04, "Failure"   }
		};

    if (wSize < sizeof(PPP_DATA_HEADER)-1) 
        return ERR_030101;

	pHeader = (PPPP_DATA_HEADER)pBuf;

	for (i = 0; i < 4; i++)
		if (pHeader->Code == chap_code[i].Code)
			break;

	if (i < 4)
	{
		pInfo->ucCode = pHeader->Code;
		pInfo->ucID = pHeader->ID;
	}
	else
		return 0;

	Length = (pHeader->Length[0] << 8) + pHeader->Length[1];

	if(Length > wSize || Length < 4 )
		return ERR_Invalid_Length;

	/* CHAP Data */
	DataLength = (short)Length - 4;

	switch( pHeader->Code )
	{
		case 0x01:      /* Chanllenge */
			break;
		case 0x02:      /* Response */
			TempLen = (UCHAR)pHeader->Data[0];

			if( (TempLen+5) >= Length )
				return ERR_Invalid_Length;
			/*
			if( (Length-TempLen-5) < 0 )
				return ERR_Invalid_Length;
			*/

			memcpy( pInfo->szUserName, pHeader->Data+TempLen+1, Length-TempLen-5 );
			pInfo->szUserName[Length-TempLen-5] = '\0';
			break;
		case 0x03:      /* Success */
			break;
		case 0x04:       /* Failure */
			memcpy( pInfo->szFailureMsg, pHeader->Data, Length-4 );
			break;
	}

    return dResult;
}

/*************************************************
func   : Analyze_PPP_IPCP()
def    : Analyze PPP - IPCP protocol 
*************************************************/ 
int Analyze_PPP_IPCP(PUCHAR pBuf, WORD wSize, ANALYZE_INFO_PPP *pInfo)
{
    int                 dResult = 1;
    int                 i;
	WORD				Length;
	short				DataLength;
	PPPP_DATA_HEADER    pHeader;
	int					TempLen;

	// Code 
	static struct _ipcp_code {
		UCHAR Code;
		char  Desc[20];
	} ipcp_code[7] =
		{
			{ 0x01, "Configure-request" },
			{ 0x02, "Configure-ack"     },
			{ 0x03, "Configure-nak"     },
			{ 0x04, "Configure-reject"  },
			{ 0x05, "Terminate-request" },
			{ 0x06, "Terminate-ack"     },
			{ 0x07, "Code-reject"       }
		};

/**
	static struct _data_type {
		UCHAR Type;
		char  Desc[30];
	} data_type[8] =
		{
			{ 0x01, "IP Address"                    },
			{ 0x02, "IP Compression Protocol"       },
			{ 0x03, "IP Address"                    },
			{ 0x04, "Mobile-IPv4"                   },
			{ 0x81, "Primary DNS Server Address"    },
			{ 0x82, "Primary NBNS Server Address"   },
			{ 0x83, "Secondary DNS Server Address"  },
			{ 0x84, "Secondary NBNS Server Address" }
		};
**/
  
    if (wSize < sizeof(PPP_DATA_HEADER)-1) 
        return ERR_040101;

    pHeader = (PPPP_DATA_HEADER)pBuf;

	for (i = 0; i < 7; i++)
		if (pHeader->Code == ipcp_code[i].Code)
			break;

	if (i < 7)
	{
		pInfo->ucCode = pHeader->Code;
		pInfo->ucID = pHeader->ID;
	}
	else 
		return 0;

	/* Length */
	Length = (pHeader->Length[0] << 8) + pHeader->Length[1];

    if (Length > wSize)
        return ERR_Invalid_Length;

	/* IPCP Data */
	DataLength = (short)Length - 4;
	i = 0;
	while (DataLength > 0)
	{
		/* Length */
        TempLen = *(pHeader->Data+i+1);

        if ( (TempLen > Length) || (TempLen <= 0) )
			return ERR_Invalid_Length;

		/* Option */
		switch (*(pHeader->Data+i))
		{
			case 0x03:
				if( pHeader->Code == 0x02 )
        			pInfo->bSimpleIP = TRUE;

				memcpy( pInfo->ucIPAddr, pHeader->Data+i+2, 4);
				break;

			case 0x81:
			case 0x82:
			case 0x83:
			case 0x84:
			case 0x01:
			case 0x02:
			case 0x04:
			default:
				break;
		}

		/* update index */
		if( *(pHeader->Data+i+1) <= 0 )
			return ERR_Invalid_Length;

		DataLength -= *(pHeader->Data+i+1);
		if( DataLength < 0 )
			return ERR_Invalid_Length;

		i += *(pHeader->Data+i+1);
	}

	return dResult;
}

/*************************************************
func   : Analyze_PPP_LCP()
def    : Analyze PPP - LCP protocol 
*************************************************/ 
int Analyze_PPP_LCP(PUCHAR pBuf, WORD wSize, ANALYZE_INFO_PPP *pInfo)
{
	int                 dResult = 1;
    int                 i;
	PPPP_DATA_HEADER    pHeader;
	
	/* Code */
	static struct _lcp_code {
		UCHAR Code;
		char  Desc[20];
	} lcp_code[13] = 
		{
			{ 0x01, "Configure-request" },
			{ 0x02, "Configure-ack"     },
			{ 0x03, "Configure-nak"     },
			{ 0x04, "Configure-reject"  },
			{ 0x05, "Terminate-request" },
			{ 0x06, "Terminate-ack"     },
			{ 0x07, "Code-reject"       },
			{ 0x08, "Protocol-reject"   },
			{ 0x09, "Echo-request"      },
			{ 0x0a, "Echo-reply"        },
			{ 0x0b, "Discard-request"   },
			{ 0x0c, "Identification"    },
			{ 0x0d, "Time-Remaining"    }
		};
/*
	static struct _data_type {
		UCHAR Type;
		char  Desc[40];
	} data_type[14] =
		{
			{ 0x00, "Vendor Specific"                       },
			{ 0x01, "Maximum Receive Unit"                  },
			{ 0x02, "Async Control Character Map"           },
			{ 0x03, "Authentication Protocol"               },
			{ 0x04, "Quality Protocol"                      },
			{ 0x05, "Magic Number"                          },
			{ 0x06, "DEPRECATED(Quality-Protocol)"          },
			{ 0x07, "Protocol Field Compression"            },
			{ 0x08, "Address and Control Field Compression" },
			{ 0x09, "FCS Alternative"                       },
			{ 0x0a, "Self-Describing-Pad"                   },
			{ 0x0b, "..."                                   },
			{ 0x0d, "Call Back"                             },
			{ 0x1c, "Internationalization"                  }
		};
*/
    if (wSize < sizeof(PPP_DATA_HEADER)-1) 
        return ERR_050101;

    pHeader = (PPPP_DATA_HEADER)pBuf;

	for (i = 0; i < 13; i++)
		if (pHeader->Code == lcp_code[i].Code)
			break;

	if (i < 13)
	{
		pInfo->ucCode = pHeader->Code;
		pInfo->ucID = pHeader->ID;
	}
    return dResult;
}

/*************************************************
func   : Analyze_PPP_PAP()
def    : Analyze PPP - PAP protocol 
*************************************************/ 
int Analyze_PPP_PAP(PUCHAR pBuf, WORD wSize, ANALYZE_INFO_PPP *pInfo)
{
    int                 dResult = 1;
    int                 i;
	PPPP_DATA_HEADER    pHeader;
	WORD				Length;
	short				DataLength;
	short				usPAPSize;
	UCHAR				UserLength;
	
	/* Code */
	static struct _pap_code {
		UCHAR Code;
		char  Desc[30];
	} pap_code[3] =
		{
			{ 0x01, "Authenticate-request" },
			{ 0x02, "Authenticate-Ack"     },
			{ 0x03, "Authenticate-nak"     }
		};

	usPAPSize = sizeof(PPP_DATA_HEADER);
    
    if (wSize < (usPAPSize-1) ) 
        return ERR_060101;

	pHeader = (PPPP_DATA_HEADER)pBuf;

	for (i = 0; i < 3; i++)
		if (pHeader->Code == pap_code[i].Code)
			break;

	if (i < 3)
	{
		pInfo->ucCode = pHeader->Code;
		pInfo->ucID = pHeader->ID;
	}
	else
		return 0;

	Length = (pHeader->Length[0] << 8) + pHeader->Length[1];

	if(Length > wSize)
		return ERR_Invalid_Length;

	DataLength = (short)Length - 4;

	switch( pHeader->Code )
	{
		case 0x01:    /* Request */
			//UserLength = Length - 10;
			UserLength = pBuf[usPAPSize - 1];
 
			memcpy( pInfo->szUserName, pHeader->Data+1, UserLength );
			pInfo->szUserName[UserLength] = '\0';
			break;
		case 0x02:   /* Success Ack */
			break;
		case 0x03:   /* Failure Ack */
			break;
	}

    return dResult;
}

int CheckFCS(PUCHAR pOrgData, WORD wOrgSize)
{
    PUCHAR pData;
    WORD wSize;
    WORD wFCS = 0xffff;
	
	if (pOrgData == NULL || wOrgSize <= 4)
    	return -1;

    pData = pOrgData + 1;
    wSize = wOrgSize - 4;

    while (wSize--)
   		wFCS = (wFCS >> 8) ^ fcstab[(wFCS ^ *pData++) & 0xff];
    wFCS ^= 0xffff;

    pData = pOrgData + (wOrgSize - 3);
    if (((PUCHAR)&wFCS)[0] == pData[0] && ((PUCHAR)&wFCS)[1] == pData[1])
    	return 1;

    return 0;
}


#if 0
/*************************************************
func   : Analyze_IP_GRE()
def    : Analyze GRE protocol 
*************************************************/ 
int Analyze_IP_GRE(PUCHAR pBuf, WORD wSize, INFO_ETH *pInfo )
{
    int   	dResult = 1;
	PGRE 	pGRE;
	WORD  	wGREHeaderSize;
	short 	dGREDataSize;	
    WORD  	Protocol;
    UCHAR 	ucGREFlag;
    UCHAR 	ucFlagVer;
    UCHAR 	FlagSeq, FlagKey, FlagAck;

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

#endif

/*************************************************
func   : Analyze_IP_UDP_MIP()
def    : Analyze MIP protocol - Registration Request/Reply protocol 
*************************************************/ 
int Analyze_IP_UDP_MIP(PUCHAR pBuf, WORD wSize, INFO_ETH *pInfo)
{
    int     dResult = 1;
    PMIP_REG_REQ pReq;
    PMIP_REG_REP pRep;


    pInfo->stMIP.bMIP = TRUE;
    pInfo->stMIP.ucMsg = pBuf[0];

	if (pInfo->stMIP.ucMsg == 0x03)
    	pInfo->stMIP.ucCode = pBuf[1];

    pInfo->stMIP.ucLifetime[0] = pBuf[2];
    pInfo->stMIP.ucLifetime[1] = pBuf[3];

	if (pBuf[0] == 1)
	{
		pReq = (PMIP_REG_REQ)pBuf;
		memcpy( pInfo->stMIP.ucSrcIP, pReq->HomeAddress,4 );
		//pInfo->stMIP.dwSrcIP = TOULONG(pReq->HomeAddress); 
	}
	else if (pBuf[0] == 3)
	{
		pRep = (PMIP_REG_REP)pBuf;
		memcpy( pInfo->stMIP.ucSrcIP, pRep->HomeAddress, 4 );
		//pInfo->stMIP.dwSrcIP = TOULONG(pRep->HomeAddress); 
	}
	
	return dResult;
}


