#ifndef _ANALYZE_EXT_ABS_HEADER_ETHERNET
#define _ANALYZE_EXT_ABS_HEADER_ETHERNET

#include <VJ.h>
#include <comm_typedef.h>
#include <ipaf_svc.h>
#include <packet_def.h>

#pragma pack(1)

typedef unsigned char   *PUCHAR;
typedef unsigned int	DWORD;
typedef unsigned short  WORD;
typedef int 			BOOL;
typedef char            TCHAR;
typedef char            *PTCHAR;

#define DEF_DWORD_SIZE	sizeof(DWORD)
#define DEF_LONGINT_SIZE	sizeof(long int)


#define TRUE  1
#define FALSE 0 

#define MAX_PATH		260

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

#define DEF_PROTO_TCP		6
#define DEF_PROTO_UDP		17

typedef struct _ANALYZE_INFO_GRE {
    BOOL    bGRE;
    UCHAR   ucGREFlag;
	WORD    wGREHeaderSize;
	DWORD   dwGREKey;
	DWORD   dwSeqNum;
} ANALYZE_INFO_GRE, *PANALYZE_INFO_GRE;

typedef struct _ANALYZE_INFO_PPP {
    BOOL    bPPP;                   /* ppp check */
    BOOL    bSimpleIP;              /* ppp check */
	WORD    wProtocol;				/* 0x21 : IP, 2D : VJC, 2F : VJUC */
    DWORD   dwPPPSize;
	int	    nPPPState;
	int		n7EPosition;			/* first 7E position */
    char    c7ECount;  
    UCHAR   ucCode;
    UCHAR   ucID;
    DWORD   dwSrcIP;                /* IPCP 0x03인 경우  IP address */
    char    szMsgType[100];
} ANALYZE_INFO_PPP, *PANALYZE_INFO_PPP;

typedef struct _ANALYZE_INFO_MAC {
    BOOL    bMAC;
    UCHAR   strDestMACAddr[6];       /* Destination MAC Address */
	UCHAR   strSrcMACAddr[6];        /* Source MAC Address */ 
} ANALYZE_INFO_MAC, *PANALYZE_INFO_MAC;

typedef struct _ANALYZE_INFO_IP {
    BOOL    bIPHeader;
	BOOL	bChecksumErr;			/* IP header checksum */	
    UCHAR   ucProtocol;             /* Protocol */
    UCHAR   IPVersion;
    UCHAR   Timelive;
    UCHAR   Type;                   /* IGMF, EGP, OSPF, RIP->COMMAND */
	WORD	wLength;				/* IP total length */
	WORD	wIdent;					/* Identification */
	WORD	wIPFrag;				/* IP Fragmentation */
    DWORD   dwSrcIP;                /* Source IP address */
    DWORD   dwDestIP;               /* Destination IP address */
 } ANALYZE_INFO_IP, *PANALYZE_INFO_IP;

typedef struct _ANALYZE_INFO_UDPTCP {
    int     dUDPTCP;                /* UDP : 1 ,TCP : 2 */
	BOOL	bChecksumErr;			/* UDP/TCP checksum */
    UCHAR   nControlType;           /* TCP Control : SYN, ACK, RST, FIN, PSH에 대한것 */
    WORD   	wSrcPort;              	/* Source Port No. */
	WORD   	wDestPort;	            /* Destination Port No. */
    DWORD   seq;                    /* TCP Session Info */
	DWORD   ack;                    /* TCP Session Info */
	DWORD   data;                   /* TCP Session Info - not use */
	DWORD   window;                 /* TCP Session Info */
	WORD	wLength;				/* UDPTCP Length */
    WORD    wDataLen;          		/* UDPTCP DataLen */
	WORD    wGetCheck;              /* Http Get - 0: no GET, other: GET */
} ANALYZE_INFO_UDPTCP, *PANALYZE_INFO_UDPTCP;

typedef struct _ANALYZE_INFO_A11 {
    BOOL    bA11;
	UCHAR	ucMsg;					/* A11 Message Type */
	UCHAR	ucCode;					/* A11 reply Code */
	UCHAR	ucLifetime[2];			/* Lifetime */
    DWORD   dwKey;                  /* UDP A11 SSE - GRE Key */
	DWORD	dwServiceOption;		/* UDP A11 VOSE - Service Option */
									/*           15 : IS-95A         */
									/*           25 : IS-95B         */
									/*           33 : 1x             */
									/*           59 : 1xEV-DO        */
    char    szMDN[16];          	/* UDP A11 SSE - MDN */
	char	szBsMscId[12];			/* UDP A11 VOSE - BS/MSC ID */
	DWORD	dwAirlinkType;			/* UDP A11 VOSE - Airlink Record Type        */
									/*            1 : Connection Setup (-> 0x01) */
									/*            2 : Active Start     (-> 0x02) */
									/*            3 : Active Stop      (-> 0x04) */
									/*            4 : SDB              (-> 0x08) */
	DWORD	dwForwardMux;			/* UDP A11 VOSE - Forward Mux Option */
	DWORD	dwReverseMux;			/* UDP A11 VOSE - Reverse Mux Option */
	DWORD	dwHomeAgentAddr;
	DWORD	dwUpdateReason;			/* UDP A11 VOSE - Update Reason */
} ANALYZE_INFO_A11, *PANALYZE_INFO_A11;

typedef struct _ANALYZE_INFO_MIP {
    BOOL    bMIP;
	UCHAR	ucMsg;					/* MIP Message Type */
	UCHAR	ucCode;					/* MIP Code */
	UCHAR	ucLifetime[2];			/* Lifetime */
    DWORD   dwSrcIP;                /* Source IP address */
} ANALYZE_INFO_MIP, *PANALYZE_INFO_MIP;

typedef struct _ANALYZE_INFO_RADIUS {
    BOOL    bRADIUS;
	UCHAR	ucCode;					/* RADIUS Code */
	UCHAR	ucId;					
	UCHAR	ucSubsRet;
	UCHAR	ucSubsFailCode;
	DWORD	dwFramedIPAddr;
	UCHAR	szCallStation[16];
	DWORD	dwAcctType;
	DWORD	dwSessStartTime;
} ANALYZE_INFO_RADIUS, *PANALYZE_INFO_RADIUS;

typedef struct _ANALYZE_INFO_WAP10 {
    // for WAP
	BOOL	bWAP;
	UCHAR	WtpPDUType;
	UCHAR	WspPDUType;
    int     wtp_num;            // WTP's count(only concatenated WTP)
    UCHAR	Wtp_type;			// Not use
    WORD    wTid;
    UCHAR	rid;
    UCHAR	gtr_ttr;
    UCHAR	tcl;
    char    szPSN[12];
    char    szMaxGrp[64];       // Max group  - Not use
    char    szNumGrp[64];       // Num group  - Not use
    UCHAR	*pFragData;
    WORD    wFragDataSize;
    WORD    wWtpSize;
    UCHAR   WSPType;          // Connect : 0x01, Disconnect : 0x02 - Not use
	UCHAR	AbortType;		  // 0: provider, 1:user
	UCHAR	AbortReason;	  // WTP Abort Reason .. 

	UCHAR	szURI[1024];  // 1001
	UCHAR	szContentType[128];
	UCHAR	szUserAgent[128]; // 101
	UINT	uiContentLen;
	UINT	uiDataLen;		// Real Content-length
	USHORT	usRetCode;		// Status Code
	UINT	uiDate;

} ANALYZE_INFO_WAP10, *PANALYZE_INFO_WAP10;	

typedef struct _ANALYZE_INFO_802_3 {
    BOOL    b802_3;
	UCHAR	DSAP;					
	UCHAR	SSAP;					
} ANALYZE_INFO_802_3, *PANALYZE_INFO_802_3;

typedef struct _ANALYZE_INFO {
    int     nVer;                   /* 0x1000 : Serial, 0x1001 : Datalink, 0x1002 : Ethernet */
    WORD    wFrameType;             /* IP(0x0800), ARP(0x0806), RARP(0x0835), IEEE802.3(<=0x05DC) */
    DWORD   dwMsgType;              /* Message Type : BPDU, IEEE802.3, ICMP */
	DWORD   dwLength;               /* Packet Length */ 

	int		dLevel;
	int		dPaddingSize;

    ANALYZE_INFO_MAC    	stMAC;
    ANALYZE_INFO_802_3  	st802_3;
    ANALYZE_INFO_IP     	stIP;
    ANALYZE_INFO_UDPTCP 	stUDPTCP;
    ANALYZE_INFO_RADIUS		stRADIUS;

} ANALYZE_INFO, *PANALYZE_INFO;

#define DEF_ANALYZE_INFO_SIZE		sizeof(ANALYZE_INFO)

/* GREA11_DECODE_RESULT */
typedef struct _GREA11_RESULT_INFO {
    int     nVer;                   /* 0x1000 : Serial, 0x1001 : Datalink, 0x1002 : Ethernet */
    WORD    wFrameType;             /* IP(0x0800), ARP(0x0806), RARP(0x0835), IEEE802.3(<=0x05DC) */
    DWORD   dwMsgType;              /* Message Type : BPDU, IEEE802.3, ICMP */
	DWORD   dwLength;               /* Packet Length */ 
	int		dLevel;

    BOOL    bGRE;
    UCHAR   ucGREFlag;
	WORD    wGREHeaderSize;
	DWORD   dwGREKey;
	DWORD   dwSeqNum;
	BOOL	bA11;
	UCHAR	Type;
	UCHAR	Code;
	UCHAR	Status;

    ANALYZE_INFO_IP     stIP;
    ANALYZE_INFO_UDPTCP stUDPTCP;
} GREA11_RESULT_INFO, *PGREA11_RESULT_INFO;

/*
 * Analyze Http Library 
 */

typedef struct _STAT_URL {
    int     nVer;                   /* 0x1000 : Serial, 0x1001 : Datalink, 0x1002 : Ethernet */
    WORD    wFrameType;             /* IP(0x0800), ARP(0x0806), RARP(0x0835), IEEE802.3(<=0x05DC) */
    DWORD   dwLength;               /* Packet Length */

	char	szDomainName[128];
	char	szUserAgent[128];
	char	szPhoneParam[128];
	char	szCPData[128];
	char	szURL1[128];
	char	szURL2[128];
	char	szURL3[128];
    short   sType;
    short   sDepth;

    int     dLevel;
	int		dPaddingSize;
	int		dRequestCount;

    ANALYZE_INFO_MAC    stMAC;
    ANALYZE_INFO_IP     stIP;
    ANALYZE_INFO_PPP    stPPP;
    ANALYZE_INFO_UDPTCP stUDPTCP;

} STAT_URL, *PSTAT_URL;

/*
 * Analyze PPP Library
 */

typedef struct _PPP_INFO {
	BOOL	bVJCIP;
	int		datalen;
	int 	dLevel;
	int		dConnection;
    ANALYZE_INFO_IP     	stIP;
    ANALYZE_INFO_PPP    	stPPP;
    ANALYZE_INFO_MIP 		stMIP;
	unsigned char uBuffer[2048];
} PPP_INFO, *PPPP_INFO;

/* Pseudo Header for evaluating TCP/UDP Checksum */
typedef	struct	_PSU_RHDR {
	UCHAR	Source[4];
	UCHAR	Destination[4];
	UCHAR	Zero;
	UCHAR	Protocol;
	UCHAR	Length[2];
} PSU_RHDR, *PPSU_RHDR;


typedef struct _INFO_ETH {
    DWORD   dwLength;        /**** Packet Size ******/
    WORD    wFrameType;  /**** Eth(protocol) or 802.3(FrameLen) *****/
    WORD    wPaddingSize;    /***** 802.3 프레임일 경우 패딩 길이 *******/
    ANALYZE_INFO_MAC    stMAC;
    ANALYZE_INFO_802_3  st802_3;
    ANALYZE_INFO_IP     stIP;
    ANALYZE_INFO_UDPTCP stUDPTCP;
    ANALYZE_INFO_GRE    stGRE;
    PUCHAR  pData;
} INFO_ETH;




/* breif sipinfo structor
 *
 *   SIP/SDP ºÐ¼®A≫ A§CN ±¸A¶A¼
 *    */

//@{  SIP METHODS
#define SIP_RESPONSE    0x00
#define SIP_UNKNOWN     0x01
#define SIP_ACK         0x02
#define SIP_BYE         0x03
#define SIP_CANCEL      0x04
#define SIP_INFO        0x05
#define SIP_INVITE      0x06
#define SIP_MESSAGE     0x07
#define SIP_NOTIFY      0x08
#define SIP_OPTIONS     0x09
#define SIP_PRACK       0x0A
#define SIP_PUBLISH     0x0B
#define SIP_REFER       0x0C
#define SIP_REGISTER    0x0D
#define SIP_SUBSCRIBE   0x0E
#define SIP_UPDATE      0x0F
//@}

// msgflag
#define MSG_REQUEST     0x01    //< INVITE, BYE, REQUEST ..
#define MSG_RESPONSE    0x02    //< OK and other fail status-code(2xx ~ 6xx )
#define MSG_OTHER       0x04    //< ACK, Trying(1xx), provisional messages, continueing to process some request;

#define PTT_INVITE_REQ      0x01;
#define PTT_INVITE_FAIL_RES 0x02;
#define PTT_INVITE_SUCC_RES 0x04;
#define PTT_ACK             0x08;
#define PTT_CANCEL          0x10;
#define PTT_MEDIA_SESS      0x20;
#define PTT_BYE_REQ         0x40;
#define PTT_BYE_RES         0x80;

#define MAX_VIA_CNT     4
#define MAX_MEDIA_CNT   4
#define MAX_TO_SIZE     32
#define MAX_FROM_SIZE   32
#define MAX_CALLID_SIZE 64
#define MAX_CSEQ_SIZE   32
#define MAX_CONTENTTYPE_SIZE  32
#define MAX_MEDIA_SIZE  16
#define MAX_VERSION_SIZE    4
#define MAX_VIA_SIZE    32
#define MAX_RURI_SIZE   64
#define MAX_REASON_SIZE 64
#define MAX_METHOD_SIZE 16
#define MAX_MEDTYPE_SIZE 16
#define MAX_MEDPROTO_SIZE 16

typedef struct _st_sipinfo
{
    int             bSIP;           // 1: SIP , 0: Not SIP
    unsigned char   ucMethod;
    unsigned char   ucSDPFlag;   // SDP °¡ AOA¸¸e 1 .. Media °u·A A¤º¸°¡ ºÐ¼® μE.
    char            szVersion[MAX_VERSION_SIZE];                //< SIP Version : 4

    unsigned short  usViaCnt;                                   //< CN SIP ¿¡ AO´A via ¼o
    unsigned short  usExpires;              // Expire A¤º¸
    unsigned int    uiCSeq;   // CSequence ¸Þ½AAoAC ¼øA÷¹øE￡ , ´UAI CallID ¿¡¼­¸¸ ≫c¿eμE. Ac½Aμμ ±¸ºÐ ¿eμμ..

    char            szVia[MAX_VIA_CNT][MAX_VIA_SIZE]; // Via A¤º¸μe : 4 * 32 = 128
    char            szTo[MAX_TO_SIZE];   // Callee : 32
    char            szFrom[MAX_FROM_SIZE]; // Caller : 32
    char            szContact[MAX_FROM_SIZE]; // Caller : 32

    char            szCSeq[MAX_CSEQ_SIZE];              /* 32 */
    char            szContentType[MAX_CONTENTTYPE_SIZE];  // ¸Þ½AAo ³≫ºIAC  MIME Type : 32
    char            szCallID[MAX_CALLID_SIZE];  // μI Agent °￡AC ´eE­ ID : 64

    unsigned short  usContentLen;
    unsigned short  usMaxForwards;
    unsigned int    uiTimestamp;

    char            szMedType[MAX_MEDTYPE_SIZE];  // Media Type (Audio, Video, ..from SDP ) : 16
    char            szMedProto[MAX_MEDPROTO_SIZE]; // RTP/AVP from SDP : 16

	unsigned short	usAudioPort;
	unsigned short	usVideoPort;
	unsigned int	uiReserved;

    unsigned int    uiMedIP;  // RTP IP .. from SDP
    unsigned short  usMedPort;  // RTP Port  .. from SDP
    unsigned short  usMedFormat;  // Media Format .. ex..) 97 .. from SDP

    char            szRequestURI[MAX_RURI_SIZE]; // Request URI : 64

    char            szReason[MAX_REASON_SIZE];  // Reason : 64

    unsigned short  usStatusCode;
    unsigned char   ucMsgFlag;                  // 1: REQUEST, 2:RESPONSE, 4: Other information
    unsigned char   ucSDPDescType;              // SDP Description Type
    unsigned char   ucReserved[4];
} st_sipinfo, *pst_sipinfo;
#define SIPINFO_SIZE    sizeof(st_sipinfo)

#define MAX_TOK_CNT     32  // Tokenized SIP Offset Count
#define MAX_STRING_SIZE 64
#define MAX_TOKHDR_SIZE 64  // Tokenized ￠®IiE  ￠®Ii¡E￠cIAIAIAC CE￠®Iia A￠®¡×I¡E￠c¡§u
//#define MAX_SIP_SIZE    2048

typedef struct _st_tok_table
{
    int  iCnt;
    int  iOffset[MAX_TOK_CNT];
    char szString[MAX_TOK_CNT][MAX_STRING_SIZE];
    //int  iStrLen[MAX_TOK_CNT];
} st_tok_table, *pst_tok_table;

#define TOK_TABLE_SIZE  sizeof(st_tok_table)

typedef struct _st_bitfield
{
    unsigned char a;
    unsigned char bb;
    unsigned char c;
    unsigned char d;
    unsigned char e;
    unsigned char f;
} st_bitfield;

#define MAX_BITFIELD_SIZE   sizeof( st_bitfield )

typedef struct _st_bitfield_param
{
    unsigned char aa;
    unsigned char b;
    unsigned char c;
} st_bitfield_param;

#define MAX_BITFIELD_PARAM_SIZE sizeof( st_bitfield_param )

// Bitfield definitions for Field Inclusions flag, SDP-Start token 0x72
typedef struct _st_bitfield_sdp
{
    unsigned char a;
    unsigned char b;
    unsigned char c;
    unsigned char d;
    unsigned char e;
    unsigned char f;
    unsigned char g;
    unsigned char h;

    unsigned char j;
    unsigned char k;
    unsigned char m;
    unsigned char n;
    unsigned char p;
    unsigned char q;
    unsigned char r;
    unsigned char s;
} st_bitfield_sdp;
#define MAX_BITFIELD_SDP_SIZE sizeof( st_bitfield_sdp )

/*function prototype define... */
//jjinri
int Analyze_IP( PUCHAR pBuf, DWORD dwSize, pst_IPTCPHeader pstIPTCP);
int Analyze_UDP(PUCHAR pBuf, DWORD dwSize, pst_IPTCPHeader pstIPTCP);

int AnalyzeSerial_Info(PUCHAR pBuf, DWORD dwSize, ANALYZE_INFO *pInfo, struct slcompress *pComp);
int AnalyzeEth_Info(PUCHAR pBuf, DWORD dwSize, ANALYZE_INFO *pInfo, struct slcompress *pComp);
int Init_Analyze_Info(ANALYZE_INFO *pInfo);
int Print_Analyze_Info2(ANALYZE_INFO *pInfo);
int Print_Analyze_Info3(ANALYZE_INFO *pInfo, char *szMsg);
int Print_A11_MSGTYPE(ANALYZE_INFO *pInfo, char *szMsg);
int Print_MIP_MSGTYPE(ANALYZE_INFO *pInfo, char *szMsg);
int Print_Analyze_Inf_Error(int dType);

int Analyze_Info_IP(PUCHAR pBuf, DWORD dwSize, ANALYZE_INFO *pInfo, BOOL bEthernet, struct slcompress *pComp);
int Analyze_Info_IP_TCP(PUCHAR pBuf, DWORD dwSize, ANALYZE_INFO *pInfo, PSU_RHDR *ppseudo);
int Analyze_Info_IP_UDP(PUCHAR pBuf, DWORD dwSize, ANALYZE_INFO *pInfo, PSU_RHDR *ppseudo);
int Analyze_Info_ARP(PUCHAR pBuf, DWORD dwSize, ANALYZE_INFO *pInfo, int dType);
int Analyze_Info_IP_IGMP(PUCHAR pBuf, DWORD dwSize, ANALYZE_INFO *pInfo, int dVersion);
int Analyze_Info_IP_ICMP(PUCHAR pBuf, DWORD dwSize, ANALYZE_INFO *pInfo);
int Analyze_Info_IP_EGP(PUCHAR pBuf, DWORD dwSize, ANALYZE_INFO *pInfo);
int Analyze_Info_IP_OSPFIGP(PUCHAR pBuf, DWORD dwSize, ANALYZE_INFO *pInfo);
int Analyze_Info_IP_UDP_RIP(PUCHAR pBuf, DWORD dwSize, ANALYZE_INFO *pInfo);
int Analyze_Info_IP_UDP_RADIUS(PUCHAR pBuf, DWORD dwSize, ANALYZE_INFO *pInfo);
int Analyze_Info_IP_UDP_RADIUS_Attr(PUCHAR pBuf, WORD wSize, ANALYZE_INFO *pInfo);
int Analyze_Info_PPP(WORD Protocol, PUCHAR pBuf, DWORD dwSize, ANALYZE_INFO *pInfo, struct slcompress *pComp);
int IsCompletePPPPacket(PUCHAR pBuf, DWORD wSize);
void GetPPP7EState(PUCHAR pBuf, DWORD wSize, int *nPPPState, int *n7EPosition);

BOOL DecodePPP(PUCHAR *ppDest, DWORD *pdwDestSize, PUCHAR pSrc, DWORD dwSrcSize);
int Analyze_Info_PPP_CCP(PUCHAR pBuf, DWORD dwSize, ANALYZE_INFO *pInfo);
int Analyze_Info_PPP_CHAP(PUCHAR pBuf, DWORD dwSize, ANALYZE_INFO *pInfo);
int Analyze_Info_PPP_IPCP(PUCHAR pBuf, DWORD dwSize, ANALYZE_INFO *pInfo);
int Analyze_Info_PPP_LCP(PUCHAR pBuf, DWORD dwSize, ANALYZE_INFO *pInfo);
int Analyze_Info_PPP_PAP(PUCHAR pBuf, DWORD dwSize, ANALYZE_INFO *pInfo);
int Analyze_Info_IP_GRE(PUCHAR pBuf, DWORD dwSize, ANALYZE_INFO *pInfo, struct slcompress *pComp);
int Analyze_Info_IP_UDP_A11(PUCHAR pBuf, DWORD dwSize, ANALYZE_INFO *pInfo);
int Analyze_Info_IP_UDP_A11_Extension(PUCHAR pBuf, DWORD dwSize, ANALYZE_INFO *pInfo);
int Analyze_Info_IP_UDP_A11_MHAE(PUCHAR pBuf, WORD wSize, ANALYZE_INFO *pInfo);
int Analyze_Info_IP_UDP_A11_VOSE(PUCHAR pBuf, WORD wSize, ANALYZE_INFO *pInfo);
int Analyze_Info_IP_UDP_A11_SSE(PUCHAR pBuf, WORD wSize, ANALYZE_INFO *pInfo);
int Analyze_Info_IP_UDP_A11_RUAE(PUCHAR pBuf, WORD wSize, ANALYZE_INFO *pInfo);
UCHAR BCD(UCHAR ucData);
int Analyze_Info_IP_UDP_MIP(PUCHAR pBuf, DWORD dwSize, ANALYZE_INFO *pInfo);
int Analyze_Info_DataLink(PUCHAR pBuf, DWORD dwSize, ANALYZE_INFO *pInfo, struct slcompress *pComp);
int AnalyzeURL_IP_TCP_HTTP(PUCHAR pBuf, DWORD wSize, ANALYZE_INFO *pInfo);
int AnalyzeURL_STRING(char *str, ANALYZE_INFO *pInfo);

int AnalyzePPP_Info(PUCHAR pBuf, DWORD dwSize, PPP_INFO *pInfo, struct slcompress *pComp);
int AnalyzePPP_Info_PPP_CCP(PUCHAR pBuf, DWORD dwSize, PPP_INFO *pInfo);
int AnalyzePPP_Info_PPP_CHAP(PUCHAR pBuf, DWORD dwSize, PPP_INFO *pInfo);
int AnalyzePPP_Info_PPP_LCP(PUCHAR pBuf, DWORD dwSize, PPP_INFO *pInfo);
int AnalyzePPP_Info_PPP_PAP(PUCHAR pBuf, DWORD dwSize, PPP_INFO *pInfo);
int AnalyzePPP_Info_PPP_IPCP(PUCHAR pBuf, DWORD dwSize, PPP_INFO *pInfo);
int AnalyzePPP_Info_IP(PUCHAR pBuf, DWORD dwSize, PPP_INFO *pInfo, BOOL bEthernet, struct slcompress *pComp);
int AnalyzePPP_Info_IP_ICMP(PUCHAR pBuf, DWORD dwSize, PPP_INFO *pInfo);
int AnalyzePPP_Info_IP_UDP(PUCHAR pBuf, DWORD dwSize, PPP_INFO *pInfo);
int AnalyzePPP_Info_IP_UDP_MIP(PUCHAR pBuf, DWORD dwSize, PPP_INFO *pInfo);

USHORT CheckSum( void *addr, long count);


/* analyze_SIP.c */
void Parse_SIP( char *szData, int iDataSize, st_sipinfo *pstSIP );
void Parse_PTTSIPHeader( pst_sipinfo pstSIP, char *szData, char *start, char *end);
unsigned char ucGet_Method( char *method );
void Parse_PTTSIP( char *szData, int iDataSize, st_sipinfo *pstSIP );


/* analyze_SDP.c */
extern void ParseSDP( char *szData, int iDataLen, st_sipinfo *pstSIP );
extern void ParseSDPLine( char *szData,  char *start, char *end, st_sipinfo *pstSIP );
char *pGetLineEnd(char *line_start);

int ParseTokSIP( char *data, int size, char *szSip );
int iGetTokField( char *data, int datalen,  char *tokfield, int deflen );
int iIsHdrToken( const unsigned char tok );
short sGetOffset( char *hex );
int iMake_RequestStartLine( st_tok_table *pstTok, char *field, int len, char *sip );
int iGetMethod( const unsigned char method,  char *buf );
int iGetUnknownMethod( const unsigned char method,  char *buf );
int iMake_ResponseStartLine( char *field, int len, char *sipbuf );
short iConvert_Array2INT16( char *hex );
void SetBitField( st_bitfield *pstBF, const unsigned char field );
int iConvert_Array2INT32( char *hex );
int iMake_CSeq( char *field, int len, char *sipbuf );
int iMake_CliCallID( st_tok_table *pstTok, char *field, int len, char *sipbuf );
int iMake_SrvCallID( st_tok_table *pstTok, char *field, int len, char *sipbuf );
int iMake_ProxyAuthorization( st_tok_table *pstTok, unsigned char *field, int len, char *sipbuf );
int iMake_INVITE( st_tok_table *pstTok, unsigned char *field, int len, char *sipbuf );
void SetParamBitField( st_bitfield_param *pstPBF, const unsigned char field );
 int iMake_VIA( st_tok_table *pstTok, unsigned char *field, int len, char *sipbuf );
int iMake_FromTo( st_tok_table *pstTok, unsigned char *field, int len, char *sipbuf );

int iMake_KnownHeader( st_tok_table *pstTok, unsigned char *field, int len, char *sipbuf );
int iMake_UnknownHeader( st_tok_table *pstTok, unsigned char *field, int len, char *sipbuf );
int iMake_CommonlyUsedHeader( st_tok_table *pstTok, unsigned char *field, int len, char *sipbuf );
int iGetString( char *buf, const unsigned char code, short index, st_tok_table *pstTok );
int iMake_MultiPartSDPStart( st_tok_table *pstTok, unsigned char *field, int len, char *sipbuf );
int iMake_MultiPartSDPMedia( st_tok_table *pstTok, unsigned char *field, int len, char *sipbuf );

// analyze_TSIP_DIC.c
int iGet_LocDir_ver1( char *buf, const unsigned char code );
int iGet_LocDir_ver2( char *buf, const unsigned char code );
int iGet_StatusString( char *buf, const short code );
int iGet_HeaderNumber( char *buf, const unsigned char number );
//extern int iGet_SipDic_ver1( char* buf, unsigned short index, unsigned char flag );
int iGet_SipDic_ver1( char* buf, unsigned char i, unsigned char flag );
int iGetParamString( char *buf, char *field, st_tok_table *pstTok );

// analyze_TSIP_UTIL.c
void SetBitField_SDP( st_bitfield_sdp *pstSDP, const unsigned char field, int flag );
int iGetSDPTokField( char *data, int datalen, char *tokfield, int deflen );
int iIsSDPHdrToken( const unsigned char tok );
unsigned int GetUpTime( time_t StartTime, int MStartTime, time_t EndTime, int MEndTime );

#pragma pack(0)
#endif

