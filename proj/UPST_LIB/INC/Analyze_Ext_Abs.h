#ifndef _ANALYZE_EXT_ABS_HEADER_ETHERNET
#define _ANALYZE_EXT_ABS_HEADER_ETHERNET

#include "typedef.h"
#include "VJ.h"
#include "define.h"
#include <UDP_header.h>

#define TRUE  1
#define FALSE 0 

#define MAX_PATH	260


/****** Added By KIW in 2004.3.20 ******/
#define		ICMP3_HDR_LEN			8	/**** ICMP type 3의 header len ****/
#define		ICMP3_ATT_IPDATA_LEN	8   /**** ICMP type 3 에서 attach 되는 IP datagram len *****/

// added by Mihee
#define	F_DATA_F		(3)
#define	F_DATA			(2)
#define	F				(1)
#define	X				(0)
#define	DATA_O			(-1)
#define DATA_F			(-2)
#define	DATA_F_DATA		(-3)
#define	DATA_F_F		(-4)
#define	DATA_F_F_DATA	(-5)
#define	F_F				(-6)
#define	F_F_DATA		(-7)

#define anz_offset(infoptr, ptr)  (long) ((unsigned char *) (ptr==NULL?0:(unsigned char *)ptr-(unsigned char *)infoptr))

typedef struct _ANALYZE_INFO_MAC {
    UCHAR   strDestMACAddr[6];       /* Destination MAC Address */
	UCHAR   strSrcMACAddr[6];        /* Source MAC Address */ 
} ANALYZE_INFO_MAC, *PANALYZE_INFO_MAC;


typedef struct _ANALYZE_INFO_IP {
    BOOL    bIPHeader;
    BOOL    bChecksumErr;
    WORD    wTotalLength;
    WORD    wIPHeaderLen;
    UCHAR   IPVersion;
	union {
		UCHAR	TOS;        /* type of service */
		struct {
			UCHAR	TOS_Reserved:1;
			UCHAR	TOS_Monetary:1;			/* RFC 1349 */
			UCHAR	TOS_Reliability:1;
			UCHAR	TOS_Throughput:1;
			UCHAR	TOS_Delay:1;
			UCHAR	TOS_Precedence:3;
		};
	};
    UCHAR   Timelive;
    UCHAR   ucProtocol;             /* Protocol */
    UCHAR   Type;                   /* IGMF, EGP, OSPF, RIP->COMMAND */
	UCHAR	ucOption;				/* Option type */
	USHORT	usIdent;				/* Identification */
	USHORT	usIPFrag;				/* IP Fragmentation */
    DWORD   dwSrcIP;                /* Source IP address */
    DWORD   dwDestIP;               /* Destination IP address */
} ANALYZE_INFO_IP, *PANALYZE_INFO_IP;




typedef struct _ANALYZE_INFO_UDPTCP {
    int     dUDPTCP;                /* UDP : 1 ,TCP : 2 */
    BOOL    bChecksumErr;
    UCHAR   nControlType;           /* TCP Control : SYN, ACK, RST, FIN, PSH에 대한것 */
    WORD    wDataLen;
    WORD    wSrcPort;               /* Source Port No. */
    WORD    wDestPort;              /* Destination Port No. */
    DWORD   seq;                    /* TCP Session Info */
    DWORD   ack;                    /* TCP Session Info */
    DWORD   window;                 /* TCP Session Info */
	WORD	mss;				/* Bit Value */
	//DWORD	dwOption;				/* Bit Value */
	WORD	wHeaderLen;
} ANALYZE_INFO_UDPTCP, *PANALYZE_INFO_UDPTCP;

/***** Defined By KIW in 2004.3.13 ****/
/*** Src, Dest port Added by KIW in 2004.3.20 ****/
typedef struct _ANALYZE_INFO_ICMP {
    UCHAR   Type;
    UCHAR   Code;
    UCHAR   ucProtocol;             /* Protocol */
	UCHAR	ucReserved1;
    BOOL   	bChecksumErr;
    DWORD   dwSrcIP;         		/* Source IP address */
    DWORD   dwDestIP;              	/* Destination IP address */
	WORD    wSrcPort;				/* Source Port No. */
	WORD    wDestPort;				/* Destination Port No. */
	UCHAR   ucReserved_2[4];	
} ANALYZE_INFO_ICMP, *PANALYZE_INFO_ICMP;


typedef struct _INFO_HTTP {
	UCHAR	ucMethod; /* Request:GET/POST... Reply , 1:GET , 3:HTTP/1.* , 2 : else*/
	WORD	wRetCode;
	WORD	wContentLen;
	UCHAR	szURL[128];
	UCHAR	szHost[64];
} INFO_HTTP;




typedef struct _ANALYZE_INFO_802_3 {
    BOOL    b802_3;
	UCHAR	DSAP;					
	UCHAR	SSAP;					
} ANALYZE_INFO_802_3, *PANALYZE_INFO_802_3;

#define MAX_FLOW_ID		15
#define MAX_GRE_ENTRY	6
typedef struct _INFO_A11 {
    BOOL    bA11;
    UCHAR   ucMsg;                  /* A11 Message Type */
    UCHAR   ucCode;                 /* A11 reply Code */
    USHORT  usLifetime;             /* Lifetime */
    DWORD   dwKey;                  /* UDP A11 SSE - GRE Key */
    DWORD   dwServiceOption;        /* UDP A11 VOSE - Service Option */
                                    /*           15 : IS-95A         */
                                    /*           25 : IS-95B         */
                                    /*           33 : 1x             */
                                    /*           59 : 1xEV-DO        */
    UCHAR   szMDN[20];              /* UDP A11 SSE - MDN */
    UCHAR   szBsMscId[12];          /* UDP A11 VOSE - BS/MSC ID */
    UCHAR   ucESN[8];
    DWORD   dwForwardMux;           /* UDP A11 VOSE - Forward Mux Option */
    DWORD   dwReverseMux;           /* UDP A11 VOSE - Reverse Mux Option */
    UCHAR   ucHomeAgentAddr[4];
    DWORD   dwUpdateReason;         /* UDP A11 VOSE - Update Reason */
    UCHAR   ucAirlinkType;          /* UDP A11 VOSE - Airlink Record Type        */
                                    /*            1 : Connection Setup (-> 0x01) */
                                    /*            2 : Active Start     (-> 0x02) */
                                    /*            3 : Active Stop      (-> 0x04) */
                                    /*            4 : SDB              (-> 0x08) */
    UCHAR   ucApplicationType;      /* Application Type                                   */
                                    /* UDP A11 VOSE - Applicaton Type                     */
                                    /*          1 : Account          (-> 0x01)            */
                                    /*          2 : MEI(Mobility Event Indicator (->0x02) */
	UCHAR	szSublink[37];
	UCHAR	FFlowID[MAX_FLOW_ID];
	UCHAR	FFlowIDCnt;
	UCHAR	RFlowID[MAX_FLOW_ID];
	UCHAR	RFlowIDCnt;
	UCHAR	AlwaysOn;
	UCHAR	ucKeyEntryCnt;
	UCHAR	szReserved[9];
	DWORD	dwKeyEntry[MAX_GRE_ENTRY];	/* UDP A11 NVSE - GRE Key Entry */
} INFO_A11;

typedef struct _ANALYZE_INFO_GRE {
    BOOL    bGRE;
    UCHAR   ucGREFlag;
    WORD    wGREHeaderSize;
    WORD    wDataSize;
    DWORD   dwGREKey;
    DWORD   dwSeqNum;
} ANALYZE_INFO_GRE, *PANALYZE_INFO_GRE;

typedef struct _ANALYZE_INFO_PPP {
    BOOL    bPPP;                   /* ppp check */
    BOOL    bFCSError;
    BOOL    bSimpleIP;              /* ppp check */
    WORD    wProtocol;              /* 0x21 : IP, 2D : VJC, 2F : VJUC */
    WORD    wPPPSize;
    int     nPPPState;
    //int       n7EPosition;            /* first 7E position */
    //char    c7ECount;
    UCHAR   ucCode;
    UCHAR   ucID;
    UCHAR   ucIPAddr[4];            /* IPCP 0x03인 경우  IP address */
    UCHAR   szUserName[32];
    UCHAR   szFailureMsg[64];
} ANALYZE_INFO_PPP, *PANALYZE_INFO_PPP;

typedef struct _ANALYZE_INFO_MIP {
    BOOL    bMIP;
    UCHAR   ucMsg;                  /* MIP Message Type */
    UCHAR   ucCode;                 /* MIP Code */
    UCHAR   ucLifetime[2];          /* Lifetime */
    UCHAR   ucSrcIP[4];             /* Source IP address */
} ANALYZE_INFO_MIP, *PANALYZE_INFO_MIP;

typedef struct _PPP_INFO {
    BOOL    bVJCIP;
    int     datalen;
    int     dLevel;
    int     dConnection;
    ANALYZE_INFO_IP         stIP;
    ANALYZE_INFO_PPP        stPPP;
    ANALYZE_INFO_MIP        stMIP;
    unsigned char uBuffer[2048];
} PPP_INFO, *PPPP_INFO;

#define DATAMAXSIZE		1500
typedef struct _INFO_ETH {
	WORD    			wDataLength;    /* szData Size */
    WORD    			wFrameType;  	/* Eth(protocol) or 802.3(FrameLen) */
    //WORD    			wPaddingSize;   /* 802.3 프레임일 경우 패딩 길이 */
    int	    			dPaddingSize;   /* 802.3 프레임일 경우 패딩 길이 */
	USHORT				usL4Code;
	USHORT				usL7Code;
	USHORT				usAppCode;
	USHORT				usRpPiFlag;
	USHORT				usSysType;
    ANALYZE_INFO_MAC    stMAC;				/*MAC Header*/
    ANALYZE_INFO_802_3  st802_3;
    ANALYZE_INFO_IP     stIP;						/* IP */
    ANALYZE_INFO_UDPTCP stUDPTCP;		/* PORT */
    ANALYZE_INFO_ICMP   stICMP;
	ANALYZE_INFO_GRE    stGRE;
	ANALYZE_INFO_MIP    stMIP;
	PUCHAR  			pAppData;
    long			  	offset;
} INFO_ETH;

typedef struct _INFO_PPP {
    ANALYZE_INFO_PPP        stPPP;
    INFO_ETH    stETH;
} INFO_PPP;

#define DEF_INFOPPP_SIZE 	sizeof(INFO_PPP)

typedef struct _ANALYZE_INFO_RADIUS {
    int					dRADIUS;    /* Msg Type: 1 for RADIUS, 2 for A14, 3 for DNS */
	UCHAR   ucCallingStation[16];
	DWORD   dwId;                                   /* Id for RADIUS, TID for A14 */
	UCHAR   ucCode;                                 /* RADIUS Code */
	UCHAR   ucRet;
	UCHAR   ucFailCode;
	UCHAR   ucWSType;                               /* Win Service Type */
	DWORD   dwWinCallId;
	UCHAR   ucCST;                                  /* Call Subscriber Type */
} ANALYZE_INFO_RADIUS, *PANALYZE_INFO_RADIUS;

typedef struct _INFO_RADIUS {
	BOOL    bRADIUS;
	DWORD   dwId;
	UCHAR   ucCSID[20];
	UCHAR   ucCode;
	UCHAR   ucSubsRet;
	UCHAR   ucADR;
	UCHAR   ucWSType;
	UCHAR   ucCST;
	DWORD   dwWinCallId;
	DWORD   dwFramedIP;
	DWORD   dwAcctType;
	UCHAR   ucCorelationID[8];
	UCHAR   szMDN[16];
} INFO_RADIUS;

typedef struct _ANALYZE_INFO_RTCP_BILL_KTF 
{
	DWORD dwSSRCsender;         // SSRC of sender
	DWORD dwRecvBytes;
	DWORD dwRecvPack;
	DWORD dwSequence;
	DWORD dwLastSequence;
	DWORD dwTimestamp;
	DWORD dwLastTimestamp;
	UCHAR ucBillStr[5];
} ANALYZE_INFO_RTCP_BILL_KTF, *PANALYZE_INFO_RTCP_BILL_KTF;

typedef struct _ANALYZE_INFO_RTCP_BILL_LGT 
{
	DWORD dwSSRCsender;         // SSRC of sender
	DWORD dwRecvDataSize;
	UCHAR ucBillStr[5];
} ANALYZE_INFO_RTCP_BILL_LGT, *PANALYZE_INFO_RTCP_BILL_LGT;

typedef struct _ANALYZE_INFO_RTCP_REPORT 
{
	DWORD   dwSSRCsource;          // SSRC of the source
	UCHAR   ucFracLoss;               // fraction lost
	DWORD   dwPackLoss;         // cumulative number of packets lost
	DWORD   dwHighestSeq;         // extended highest sequence number received
	DWORD   dwJitter;            // interarrival jitter
	DWORD   dwLastSR;           // last SR (LSR)
	DWORD   dwDelayLastSR;          // delay since last SR (DLSR)
} ANALYZE_INFO_RTCP_REPORT, *PANALYZE_INFO_RTCP_REPORT;

typedef struct _ANALYZE_INFO_RTCP_RRSR 
{
	DWORD dwSSRCsender;         // SSRC of sender

	INT64 llNTPTimestamp;  // NTP timestamp
	DWORD dwRTPTimestamp;  // RTP timestamp
	DWORD dwPktCnt;   // sender's packet count
	DWORD dwPktBytes;   // sender's octet count

	ANALYZE_INFO_RTCP_REPORT    report_block;
} ANALYZE_INFO_RTCP_RRSR, *PANALYZE_INFO_RTCP_RRSR;

typedef struct _ANALYZE_INFO_RTCP 
{
	BOOL  bType;
	UCHAR ucCount;
	UCHAR ucVer;
	WORD  wLength;

	ANALYZE_INFO_RTCP_RRSR  stRRSR;
	ANALYZE_INFO_RTCP_BILL_LGT  stBill_Lgt;
	ANALYZE_INFO_RTCP_BILL_KTF  stBill_Ktf;
} ANALYZE_INFO_RTCP, *PANALYZE_INFO_RTCP;

#define RTCP_HDRSIZE (sizeof(ANALYZE_INFO_RTCP)-sizeof(ANALYZE_INFO_RTCP_REPORT))

typedef struct _ANALYZE_INFO_RTP 
{
	UCHAR ucCSRCCount;
	UCHAR ucExtension;
	UCHAR ucVersion;
	UCHAR ucPayloadType;
	UCHAR ucMarker;
	WORD  wSequence;
	DWORD dwTimestamp;
	DWORD dwSSRC;
} ANALYZE_INFO_RTP, *PANALYZE_INFO_RTP;
	
typedef struct _RTP 
{
	UCHAR CSRCCount:4;
	UCHAR Extension:1;
	UCHAR Padding:1;
	UCHAR Version:2;
	UCHAR PayloadType:7;
	UCHAR Marker:1;
	UCHAR Sequence[2];
	UCHAR Timestamp[4];
	UCHAR SSRC[4];
	UCHAR CSRC[1];
} RTP, *PRTP;

typedef struct _RTCP 
{
	UCHAR Count:5;
	UCHAR Pad:1;
	UCHAR Ver:2;
	UCHAR Type;
	UCHAR Length[2];
	UCHAR Data[1];
} RTCP, *PRTCP;

typedef struct _RTCP_SR 
{
	UCHAR SSRC[4];          // SSRC of sender

	UCHAR NTPTimestamp[8];  // NTP timestamp
	UCHAR RTPTimestamp[4];  // RTP timestamp
	UCHAR PacketCount[4];   // sender's packet count
	UCHAR PacketBytes[4];   // sender's octet count

	// report block
	UCHAR Data[1];
} RTCP_SR, *PRTCP_SR;

typedef struct _RTCP_RR 
{
	UCHAR SSRC[4];          // SSRC of packet sender

	// report block
	UCHAR Data[1];
} RTCP_RR, *PRTCP_RR;

typedef struct _RTCP_REPORT 
{
	UCHAR SSRC[4];          // SSRC of the source
	UCHAR FL;               // fraction lost
	UCHAR CNPL[3];          // cumulative number of packets lost
	UCHAR EHSNR[4];         // extended highest sequence number received
	UCHAR IJ[4];            // interarrival jitter
	UCHAR LSR[4];           // last SR (LSR)
	UCHAR DLSR[4];          // delay since last SR (DLSR)
} RTCP_REPORT, *PRTCP_REPORT;

typedef struct _RTCP_BILL_LGT 
{ // type = 204 (APP specific)
	UCHAR SSRC[4];

	// billing report (LGT specific)
	UCHAR BillStr[4];
	UCHAR DataSize[4];
} RTCP_BILL_LGT, *PRTCP_BILL_LGT;

typedef struct _RTCP_BILL_KTF 
{ // type = 204 (APP specific)
	UCHAR SSRC[4];

	// billing report (KTF specific)
	UCHAR BillStr[4];
	UCHAR SN[4];
	UCHAR LastSN[4];
	UCHAR Timestamp[4];
	UCHAR LastTS[4];
	UCHAR BytesReceived[4];
	UCHAR PackReceived[4];
} RTCP_BILL_KTF, *PRTCP_BILL_KTF;

#define INFO_ETH_SIZE	sizeof(INFO_ETH)
#define INFO_LEN		(INFO_ETH_SIZE - DATAMAXSIZE)


/* function prototype define... */
USHORT Checksum( void *addr, WORD count );
int Analyze_Eth(PUCHAR pBuf, WORD wSize, INFO_ETH *pInfo );
int Analyze_IP(PUCHAR pBuf, WORD wSize, INFO_ETH *pInfo);
int AnayzeIPOptions(PUCHAR pBuffer, WORD wSize, INFO_ETH *pInfo);
void AnalyzeIP_TCP_Option(PUCHAR pucBuffer, WORD wSize, INFO_ETH *pInfo);
int AnalyzeIP_TCP(PUCHAR pucBuffer, WORD wSize, INFO_ETH *pInfo, char *pseudo);
int AnalyzeIP_ICMP(PUCHAR pucBuffer, WORD wSize, INFO_ETH *pInfo);
int Analyze_IP_UDP( PUCHAR pBuf, WORD wSize, INFO_ETH *pInfo );

UCHAR BCD2(UCHAR ucData);
int Analyze_DataLink(PUCHAR pBuf, DWORD dwSize, INFO_ETH *pInfo );
int Analyze_HTTP(PUCHAR pBuf, WORD wSize, INFO_HTTP *pInfo);
int Analyze_HTTP_String(PUCHAR pBuf, WORD wSize, char *str, int *iLength );
DWORD SET_BIT(int dPosition);

int Analyze_RADIUS_Attr(PUCHAR pBuf, WORD wSize, INFO_RADIUS *pInfo);
int Analyze_RADIUS( PUCHAR pBuf, WORD wSize, INFO_RADIUS *pInfo );

int Analyze_IP_GRE(PUCHAR pBuf, WORD wSize, INFO_ETH *pInfo );

int Analyze_A11(PUCHAR pBuf, WORD wSize, INFO_A11 *pInfo);
int Analyze_A11_Extension(PUCHAR pBuf, WORD wSize, INFO_A11 *pInfo);
int Analyze_A11_MHAE(PUCHAR pBuf, WORD wSize, INFO_A11 *pInfo);
int Analyze_A11_VOSE(PUCHAR pBuf, WORD wSize, INFO_A11 *pInfo);
int Analyze_A11_SSE(PUCHAR pBuf, WORD wSize, INFO_A11 *pInfo);
int Analyze_A11_RUAE(PUCHAR pBuf, WORD wSize, INFO_A11 *pInfo);

WORD AnalyzeIP_UDP_RTCP(USHORT usAppCode, ANALYZE_INFO_RTCP *info_rtcp, PUCHAR pucBuffer, WORD wSize);
BOOL AnalyzeIP_UDP_RTCP_SR(ANALYZE_INFO_RTCP_RRSR *rtcp_rrsr, int nCount, PUCHAR pucBuffer, WORD wSize);
BOOL AnalyzeIP_UDP_RTCP_RR(ANALYZE_INFO_RTCP_RRSR *rtcp_rrsr, int nCount, PUCHAR pucBuffer, WORD wSize);
BOOL AnalyzeIP_UDP_RTCP_BILL_LGT(ANALYZE_INFO_RTCP_BILL_LGT *rtcp_bill, PUCHAR pucBuffer, WORD wSize);
BOOL AnalyzeIP_UDP_RTCP_BILL_KTF(ANALYZE_INFO_RTCP_BILL_KTF *rtcp_bill, PUCHAR pucBuffer, WORD wSize);
BOOL AnalyzeIP_UDP_RTP(ANALYZE_INFO_RTP *info_rtp, PUCHAR pucBuffer, WORD wSize);


typedef struct _st_Diameter_hdr_flag
{
	UCHAR           bRequest;
	UCHAR           bProxiable;
	UCHAR           bError;
	UCHAR           bTrans;
	UCHAR			reserved;	
} st_Diameter_hdr_flag, *pst_Diameter_hdr_flag;

typedef struct _st_DiameterHdr
{
	UCHAR 			ucVersion;
	UINT 			uiLength;
	st_Diameter_hdr_flag flags;
	UINT			uiCmdCode;
	UINT			uiAppID;
	UINT			uiHopByHopID;
	UINT			uiEndToEndID;
} st_DiameterHdr, *pst_DiameterHdr;

#define DIAMETER_HDR_SIZE sizeof(st_DiameterHdr)


typedef struct _st_AVP_hdr_flag
{
	UCHAR			ucVendor;
	UCHAR			ucMandatory;
	UCHAR			ucSecurity;	
}st_AVP_hdr_flag, *pst_AVP_hdr_flag;

typedef struct _st_AVPHdr
{
	UINT			uiAvpCode;
	st_AVP_hdr_flag flags;
	UCHAR			ucFlags;
	UINT			uiLength;
	UINT			uiVenderId;
	char*			value;
} st_AVPHdr, *pst_AVPHdr;

typedef struct _st_DiameterInfo
{
	struct timeval  stCreateTime;
	st_DiameterHdr	stDiameterHdr;
	st_AVPHdr		stAVPHdr;

	UCHAR			szSessionID[MAX_SESSIONID_SIZE];
	UCHAR			szOrgHost[MAX_HOST_REALM_SIZE];
	UCHAR 			szOrgRealm[MAX_HOST_REALM_SIZE];
	UCHAR			szDestHost[MAX_HOST_REALM_SIZE];
	UCHAR			szDestRealm[MAX_HOST_REALM_SIZE];
	UCHAR			szPublicID[MAX_HOST_REALM_SIZE];
	UCHAR			szUserID[MAX_HOST_REALM_SIZE];
	UINT			uiApplicationID;
	UINT			uiAuthSessionState;
	UCHAR			ucCCReqType;
	UINT			uiCCReqNum;
	UINT			uiOrgStateId;
	UCHAR			ucSubscriptionType;
	UCHAR 			ucUserAuthorizationType;
	UCHAR			szSubscriptionData[MAX_SUBSCRIPTION_SIZE];
	UCHAR			szCalledStationID[MAX_CALLED_STATION_ID_SIZE];
	UCHAR			szProxyHost[MAX_PROXYHOST_SIZE];
	UCHAR			szProxyState[MAX_PROXYSTATE_SIZE];
	UCHAR			szInService[MAX_INSERVICE_SIZE];
	UINT			uiChargingServer;
	UCHAR			szGCID[MAX_GCID_SIZE];
	UCHAR			szICID[MAX_ICID_SIZE];
	UCHAR			ucPccStatus;	
	UCHAR			ucBearerOP;	
	UCHAR			szErrMSG[MAX_ERR_MSG_SIZE];
	UINT			uiResultCode;
	UINT			uiExpResultCode;
	INT				iCauseCode;
	UCHAR			ucAccRecordType;
	UCHAR			ucReAuthReqType;
} st_DiameterInfo, *pst_DiameterInfo;

#define DEF_DIAMETERINFO_SIZE 	sizeof(st_DiameterInfo)

int dAnalyze_Diameter( PUCHAR pBuf, int dDataLen, pst_DiameterInfo pstDiameterInfo );

/* L2TP DEFINITION */

#define DEF_L2TP_PORT 	1701

#define pntohs(p)	((unsigned short)                               \
					((unsigned short)*((const unsigned char *)(p)+0)<<8 | \
					(unsigned short)*((const unsigned char *)(p)+1)<<0))

#define pntohl(p)   ((unsigned int)*((const unsigned char *)(p)+0)<<24|  \
					(unsigned int)*((const unsigned char *)(p)+1)<<16|  \
					(unsigned int)*((const unsigned char *)(p)+2)<<8|   \
					(unsigned int)*((const unsigned char *)(p)+3)<<0)

/* L2TP HEADER */
#define CONTROL_BIT(msg_info)	(msg_info & 0x8000)			/* Type bit control = 1 data = 0 */
#define LENGTH_BIT(msg_info)	(msg_info & 0x4000)			/* Length bit = 1  */
#define RESERVE_BITS(msg_info)	(msg_info &	0x37F8)			/* Reserved bit - usused */
#define SEQUENCE_BIT(msg_info)	(msg_info & 0x0800)			/* SEQUENCE bit = 1 Ns and Nr fields */
#define OFFSET_BIT(msg_info)	(msg_info & 0x0200)			/* Offset */
#define PRIORITY_BIT(msg_info)	(msg_info & 0x0100)			/* Priority */
#define L2TP_VERSION(msg_info)	(msg_info & 0x000f)			/* Version of l2tp */

/* L2TP AVP HEADER */
#define MANDATORY_BIT(msg_info) (msg_info & 0x8000)			/* Mandatory = 1 */
#define HIDDEN_BIT(msg_info) 	(msg_info & 0x4000)			/* Hidden = 1 */
#define AVP_LENGTH(msg_info) 	(msg_info & 0x03ff)			/* AVP Length */
#define FRAMING_SYNC(msg_info)  (msg_info & 0x0001)			/* SYNC Framing Type */
#define FRAMING_ASYNC(msg_info) (msg_info & 0x0002)			/* ASYNC Framing Type */
#define BEARER_DIGITAL(msg_info) (msg_info & 0x0001)		/* Digital Bearer Type */
#define BEARER_ANALOG(msg_info) (msg_info & 0x0002)			/* Analog Bearer Type */
#define CIRCUIT_STATUS_BIT(msg_info) (msg_info & 0x0001)    /* Circuit Status */
#define CIRCUIT_TYPE_BIT(msg_info)	(msg_info & 0x0001)      /* Circuit Condition */

#define MAX_HOSTNAME_SIZE				41
#define MAX_HOSTNAME_LEN				(MAX_HOSTNAME_SIZE - 1)
#define MAX_CHAP_SIZE 					128
#define MAX_CHAP_LEN 					(MAX_CHAP_SIZE - 1)
#define MAX_LCPCONFREQ_SIZE				128
#define MAX_LCPCONFREQ_LEN 				(MAX_CHAP_SIZE - 1)

#define  CONTROL_MESSAGE  				0
#define  RESULT_ERROR_CODE 				1
#define  L2TP_PROTOCOL_VERSION  		2
#define  FRAMING_CAPABILITIES 			3
#define  BEARER_CAPABILITIES 			4
#define  TIE_BREAKER 					5
#define  FIRMWARE_REVISION 				6
#define  HOST_NAME 						7
#define  VENDOR_NAME 					8
#define  ASSIGNED_TUNNEL_ID 			9
#define  RECEIVE_WINDOW_SIZE 			10
#define  CHALLENGE 						11
#define  CAUSE_CODE_CDN 				12
#define  CHALLENGE_RESPONSE 			13
#define  ASSIGNED_SESSION 				14
#define  CALL_SERIAL_NUMBER 			15
#define  MINIMUM_BPS 					16
#define  MAXIMUM_BPS 					17
#define  BEARER_TYPE 					18
#define  FRAMING_TYPE 					19
#define  CALLED_NUMBER 					21
#define  CALLING_NUMBER 				22
#define  SUB_ADDRESS 					23
#define  TX_CONNECT_SPEED 				24
#define  PHYSICAL_CHANNEL 				25
#define  INITIAL_RECEIVED_LCP_CONFREQ 	26
#define  LAST_SENT_LCP_CONFREQ 			27
#define  LAST_RECEIVED_LCP_CONFREQ 		28
#define  PROXY_AUTHEN_TYPE 				29
#define  PROXY_AUTHEN_NAME 				30
#define  PROXY_AUTHEN_CHALLENGE 		31
#define  PROXY_AUTHEN_ID 				32
#define  PROXY_AUTHEN_RESPONSE 			33
#define  CALL_STATUS_AVPS 				34
#define  ACCM 							35
#define  RANDOM_VECTOR 					36
#define  PRIVATE_GROUP_ID 				37
#define  RX_CONNECT_SPEED 				38
#define  SEQUENCING_REQUIRED 			39

typedef struct _st_L2TP_INFO {
	USHORT 	usPacketType;
	USHORT 	usHeaderSize; 								/* L2TP Header Size */
	USHORT 	usLength;									/* L2TP Payload Size */
	USHORT 	usTunnelID;
	USHORT 	usSessionID;
	USHORT 	usNs;
	USHORT 	usNr;
	USHORT 	usOffsetSize;
	
	UCHAR 	ucSendLCPFlag;
	UCHAR 	ucRecvLCPFlag;
	UCHAR 	ucCHAPChalFlag;
	UCHAR 	ucCHAPRespFlag;
	UCHAR 	ucAUTHID;									/* CHAP ID */

	USHORT 	usMessageType;								/* CONTROL_MESSAGE */
	USHORT 	usProtocolVer;								/* L2TP_PROTOCOL_VERSION */
	USHORT 	usResultCode;								/* RESULT_ERROR_CODE */
	USHORT 	usErrorCode;								/* RESULT_ERROR_CODE */
	USHORT 	usFirmwareRevision;							/* FIRMWARE_REVISION */

	UCHAR 	szHostName[MAX_HOSTNAME_SIZE];				/* HOST_NAME */
	UCHAR 	szVendorName[MAX_HOSTNAME_SIZE];			/* VENDOR_NAME */
	USHORT 	usAssignedTunnelID;							/* ASSIGNED_TUNNEL_ID */
	USHORT 	usAssignedSessionID;						/* ASSIGNED_SESSION */
	USHORT 	usReceiveWindowSize;						/* RECEIVE_WINDOW_SIZE */
	UCHAR 	szChallenge[MAX_CHAP_SIZE];					/* CHALLENGE */
	UCHAR 	szChallengeResp[MAX_CHAP_SIZE];				/* CHALLENGE_RESPONSE */

	UCHAR 	szCalledNumber[MAX_IMSI_SIZE];				/* ICRQ, OCRQ */
	UCHAR 	szCallingNumber[MAX_IMSI_SIZE];				/* ICRQ */

	UINT 	uiCallSerialNumber; 						/* CALL_SERIAL_NUMBER */
	UINT 	uiTxConnectSpeed;							/* ICCN */
	UINT 	uiRxConnectSpeed;							/* ICCN */

	UCHAR 	szSentLCPConfReq[MAX_LCPCONFREQ_SIZE];		/* ICCN, LAST_SENT_LCP_CONFREQ */
	UCHAR 	szRecvLCPConfReq[MAX_LCPCONFREQ_SIZE];		/* ICCN, LAST_RECEIVED_LCP_CONFREQ */

	USHORT 	usProxyAuthenType; 							/* 1: Text 2: CHAP 3: PAP 4: No AUTH 5: MSCHAPv1, PROXY_AUTHEN_TYPE */
	UCHAR 	szProxyAuthenName[MAX_HOST_REALM_SIZE];		/* PROXY_AUTHEN_NAME */
} st_L2TP_INFO;

#define  DEF_L2TPINFO_SIZE 	sizeof(st_L2TP_INFO)

#endif

