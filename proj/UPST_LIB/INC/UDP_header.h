#ifndef _PROTOCOL_UDP_HEADER_ETHERNET
#define _PROTOCOL_UDP_HEADER_ETHERNET

/* UDP Header, RFC793 */
typedef	struct	_UDP_RHDR {
	UCHAR	Source[2];
	UCHAR	Destination[2];
	UCHAR	Length[2];
	UCHAR	Checksum[2];
	UCHAR	Data[1];
} UDP_RHDR, *PUDP_RHDR;

/* NETBIOS Header over UDP/TCP : 2001.02.27 by joonhk */
typedef struct _NETBIOS_HDR {
	UCHAR NAME_TRN_ID[2];
	UCHAR CODE_FLAG[2]; 
	UCHAR QDCOUNT[2];
	UCHAR ANCOUNT[2];
	UCHAR NSCOUNT[2];
	UCHAR ARCOUNT[2];
} NETBIOS_HDR, *PNETBIOS_HDR;

/* NETBIOS - Name Service , NODE STATISTICS : 2001.02.28 by joonhk */
typedef struct _NODE_STAT {
	UCHAR UNIT_ID[6];			
	UCHAR JUMPERS;				
	UCHAR TEST_RESULT;			
	UCHAR VERSION_NUM[2];		
	UCHAR PERIODofSTAT[2];		
	UCHAR NUMofCRC[2];	
	UCHAR NUM_ALIGNMENT_ERR[2];	
	UCHAR NUMofCOLLISION[2];	
	UCHAR NUM_SEND_ABORT[2];
	UCHAR NUM_GOOD_SEND[4];		
	UCHAR NUM_GOOD_RECEIVE[4]; 
	UCHAR NUM_RETRANSMIT[2];	
	UCHAR NUM_NO_RES_COND[2];	
	UCHAR NUM_FREE_COMMAND_BLOCK[2];		
	UCHAR TOTAL_NUM_COMMAND_BLOCK[2];	
	UCHAR MAX_TOTAL_NUM_COMMAND_BLOCK[2];
	UCHAR NUM_PENDING_SESSION[2];	
	UCHAR MAX_NUM_PENDING_SESSION[2];	
	UCHAR MAX_TOTAL_SESSION_POSSIBLE[2];	
	UCHAR SESSION_DATA_PACKET_SIZE[2];
} NODE_STAT, *PNODE_STAT;

/* NBT Datagram : 2001.03.05 by joonhk */
typedef struct _NBT_D {
	UCHAR MSGTYPE;
	UCHAR FLAGS;
	UCHAR DGM_ID[2];
	UCHAR SOURCE_IP_ADDRESS[4];
	UCHAR SOURCE_PORT[2];
} NBT_D, *PNBT_D;

#define MAX_ATTR_FORMAT 63

/*
char attribute_format[MAX_ATTR_FORMAT][30] = {
	"User Name",
	"User Password",
	"CHAP Password",
	"NAS IP Address",
	"NAS Port",
	"Service Type",
	"Framed Protocol",
	"Framed IP Address",
	"Framed IP Netmask",
	"Framed Routing",		
	"Filter Id",
	"Framed MTU",
	"Framed Compression",
	"Login IP Host",
	"Login Service",
	"Login TCP Port",
	"(unassigned)",
	"Reply Message",
	"Callback Number",
	"Callback Id",			
	"(unassigned)",
	"Framed Route",
	"Framed IPX Network",
	"State",
	"Class",
	"Vendor Specific",
	"Session Timeout",
	"Idel Timeout",
	"Termination Action",
	"Called Station Id",	
	"Calling Station Id",
	"NAS Identifier",
	"Proxy State",
	"Login LAT Service",
	"Login LAT Node",
	"Login LAT Group",
	"Framed AppleTalk Link",
	"Framed AppleTalk Network",
	"Framed AppleTalk Zone",
	"Acct Status Type",		
	"Acct Delay Time",
	"Acct Input Octets",
	"Acct Output Octets",
	"Acct Session Id",
	"Acct Authentic",
	"Acct Session Time",
	"Acct Input Packets",
	"Acct Output Packets",
	"Acct Terminate Cause",
	"Acct Multi Session Id",
	"Acct Link Count",
	"",
	"",
	"",
	"Event Time Stamp",
	"",
	"",
	"",
	"",
	"CHAP challenge",
	"NAS Port Type",
	"Port Limit",
	"Login LAT Port"
};

#define MAX_VEND_SPECIFIC 217
char vendor_specific[MAX_VEND_SPECIFIC][60] = {
	"Security Status",
	"Security Level",
	"Pre-shared secret",
	"Reverse Tunnel Specification",
	"Differenciated Services Class Option Attribute",
	"Accounting Container",
	"Home Agent Attribute",
	"Key Attribute",
	"Serving PCF",
	"MS/BSC ID",			

	"User Zone",
	"Forward Mux Option",
	"Reverse Mux Option",
	"Forward Fundamental Rate",
	"Reverse Fundamental Rate",
	"Service Option",
	"Forward Traffic Type",
	"Reverse Traffic Type",
	"Fundamental Frame Size",
	"Forward Fundamental RC",	

	"Reverse Fundamental RC",
	"IP Technology",
	"Compulsory Tunnel Indicator",
	"Release Indicator",
	"Bad PPP Frame Count",
	"",
	"",
	"",
	"",
	"Number of Active Transitions",	

	"SDB Octet Count(Terminating)",
	"SDB Octet Count(Originating)",
	"Number of SDBs(Terminating)",
	"Number of SDBs(Originating)",
	"Alternate Billing Identifier",
	"IP Quality of Service(QOS)",
	"Interconnection IP Network Provider ID",
	"Interconnecting IP Network Service Quality of Service",
	"Airlink Quality of Service",
	"Airlink Record Type",	

	"R-P Session ID",
	"Airlink Sequence Number",
	"Number of PPP-bytes received",
	"Correlation ID",
	"Mobile Originated/Mobile Terminated Indicator",
	"",
	"",
	"",
	"",
	"",								

	"", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",	

	"", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",		

	"", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",		

	"",
	"",
	"Packet Session Start Time",
	"PPP Session Start Time",
	"Accumulated Active Time",
	"Price Plan",
	"Win Service Type",
	"Accumulated Output Octets",
	"Accumulated Input Octets",
	"VPN Gateway IP Address",	

	"VPN Protocol",
	"PPP Up Time",
	"BS/MSC ID(Session Start)",
	"BS/MSC ID(Session Stop)",
	"Win Call ID",
	"SVC Type",
	"USBL Time",			

	"USBL Packet",					
	"ReAccess Flag",				
	"SubsInfo Result",			
	"ADR",				
	"VPN",			
	"AUTH",		
	"WIN1",	
	"WIN2",	
	"WIN3",		
	"MN Table ID"			
};
***/

/* BOOTP : 2001.03.21 by hyon */
typedef struct _BOOTP {
	UCHAR OPCode;
	UCHAR HWType;
	UCHAR HWLength;
	UCHAR HOPCount;
	UCHAR TransactionID[4];
	UCHAR NumOfSec[2];
	UCHAR Flag[2];
	UCHAR ClientIP[4];
	UCHAR YourIP[4];
	UCHAR ServerIP[4];
	UCHAR GatewayIP[4];
	UCHAR ClientHWAddr[16];
	UCHAR ServerName[64];
	UCHAR BootFileName[128];
} BOOTP, *PBOOTP;


/* RIP (UDP port = 520) : 2001.03.21 by hyon */
typedef struct _RIP {
	UCHAR Command;
	UCHAR Version;
	UCHAR Reserved1[2];
	UCHAR Data[1];
} RIP, *PRIP;

/* RADIUS : 2001.03.21 by hyon */
typedef struct _RADIUS {
	UCHAR Code;
	UCHAR Identifier;
	UCHAR Length[2];
	UCHAR Authenticator[16];
	UCHAR Attributes[1];
} RADIUS, *PRADIUS;

typedef struct _RADIUS_Attr {
	UCHAR Type;
	UCHAR Length;
	UCHAR Data[1];
} RADIUS_Attr, *PRADIUS_Attr;

typedef struct _RADIUS_Vendor {
	UCHAR VendorID[4];
	UCHAR Type;
	UCHAR Length;
	UCHAR Data[1];
} RADIUS_Vendor, *PRADIUS_Vendor;

/* A14 */
typedef struct _A14 {
	UCHAR MsgType;
	UCHAR Length[2];
	UCHAR TID[4];
	UCHAR Element[1];
} A14, *PA14;

// DNS (Domain Name System) : 2002.11.08 by sykim
typedef struct _DNS {
	UCHAR Ident[2];		// used to match request/replay pakcets
	UCHAR Param[2];		// QR(1) Opcode(4) AA(1) TC(1) RD(1) RA(1) 0(3) Rcode(4)
	UCHAR QuestNum[2];	// Total Questions
	UCHAR AnsNum[2];	// Total Answer RRs
	UCHAR AuthNum[2];	// Total Authority RRs
	UCHAR AddNum[2];	// Total Additional RRs
	UCHAR Data[1];		// Questions[], AnswerRRs[], AuthorityRRs[], AdditionalRRs[]
} DNS, *PDNS;

/* A11-Registration Request */
typedef struct _A11_REG_REQ {
	UCHAR  Type;			/* 0x01 */
	UCHAR  Flags;			/* S=0 B=0 D=0 M=0 G=1 V=0 T=1 Res=1 */
	UCHAR  Lifetime[2];
	UCHAR  HomeAddress[4];	/* 0; */
	UCHAR  HomeAgent[4];
	UCHAR  CareofAddress[4];
	UCHAR  Identification[8];
	UCHAR  Extension[1];
} A11_REG_REQ, *PA11_REG_REQ;

/* A11-Registration Reply */
typedef struct _A11_REG_REP {
	UCHAR  Type;			/* 0x03 */
	UCHAR  Code;
	UCHAR  Lifetime[2];
	UCHAR  HomeAddress[4];	/* 0 */
	UCHAR  HomeAgent[4];
	UCHAR  Identification[8];
	UCHAR  Extension[1];
} A11_REG_REP, *PA11_REG_REP;

/* A11-Registration Update */
typedef struct _A11_REG_UPD {
	UCHAR  Type;			/* 0x14 */
	UCHAR  Reserved[3];
	UCHAR  HomeAddress[4];	/* 0 */
	UCHAR  HomeAgent[4];
	UCHAR  Identification[8];
	UCHAR  Extension[1];
} A11_REG_UPD, *PA11_REG_UPD;

/* A11-Registration Acknowledge */
typedef struct _A11_REG_ACK {
	UCHAR  Type;			/* 0x15 */
	UCHAR  Reserved[2];
	UCHAR  Status;
	UCHAR  HomeAddress[4];
	UCHAR  CareofAddress[4];
	UCHAR  Identification[8];
	UCHAR  Extension[1];
} A11_REG_ACK, *PA11_REG_ACK;

/* A11-Keep Alive */
typedef struct _A11_KEEPALIVE {
	UCHAR Type;
	UCHAR Code;
	UCHAR Reserved[2];
	UCHAR Address[4];
	UCHAR Extension[1];
} A11_KEEPALIVE, *PA11_KEEPALIVE;

/* A11-Repair */
typedef struct _A11_REPAIR {
	UCHAR Type;
	UCHAR Code;
	UCHAR Reserved[2];
	UCHAR Address[4];
	UCHAR LifeTime[4];
	UCHAR GREKey[4];
	UCHAR SPI[4];
	UCHAR Extension[1];
} A11_REPAIR, *PA11_REPAIR;

/* A11-Session Specific Extension */
typedef struct _A11_SSE {
	UCHAR  Identifier;		/* 0x39 */
	UCHAR  Length;
	UCHAR  Protocol[2];	/* 0x880B (PPP) */
	UCHAR  Key[4];
	UCHAR  Reserved[2];
	UCHAR  MNConnectionId[2];
	UCHAR  MNIDType[2];	/* 0x0006 */
	UCHAR  MNIDLength;
	UCHAR  Identity[9];
} A11_SSE, *PA11_SSE;

/* A11-Vendor/Organization Specific Extension */
typedef struct _A11_CVSE {
	UCHAR  Identifier;		/* 0x39(0x26) */
	UCHAR  Reserved;
	UCHAR  Length[2];
	UCHAR  Cdma2000VenderID[4];
	UCHAR  ApplicationType;
	UCHAR  ApplicationSubType;		/* 0x01 */
	UCHAR  ApplicationData[1];
} A11_CVSE, *PA11_CVSE;

typedef struct _A11_NVSE {
	UCHAR	Identifier;
	UCHAR	Length;
	UCHAR	Reserved[2];
	UCHAR	Cdma2000VenderID[4];
	UCHAR  	ApplicationType;
    UCHAR  	ApplicationSubType;      /* 0x01 */
    UCHAR  	ApplicationData[1];
} A11_NVSE, *PA11_NVSE;

/* A11-Mobile-Home Authentication Extension */
typedef struct _A11_MHAE {
	UCHAR  Identifier;		/* 0x32(0x20) */
	UCHAR  Length;			/* 0x14 */
	UCHAR  SPI[4];			/* 0x00000100 ~ 0xFFFFFFFF */
	UCHAR  Authenticator[16];
} A11_MHAE, *PA11_MHAE;

/* A11-Registration Update Authentication Extension */
typedef struct _A11_RUAE {
	UCHAR  Identifier;		/* 0x40(0x28) */
	UCHAR  Length;			/* 0x14 */
	UCHAR  SPI[4];			/* 0x00000100 ~ 0xFFFFFFFF */
	UCHAR  Authenticator[16];
} A11_RUAE, *PA11_RUAE;

typedef struct _A11_NVSE_FENTRY {
	UCHAR	Length;
	UCHAR	ID;
	UCHAR	FLAG;
	UCHAR	ReqQosLen;
	UCHAR	GranQosLen;
} A11_NVSE_FENTRY, *pA11_NVSE_FENTRY;

typedef struct _A11_NVSE_FFLOW {
	UCHAR	Length[2];
	UCHAR	SRID;
	UCHAR	FLAG;
	UCHAR	FFlowCnt;
	A11_NVSE_FENTRY	FENTRY;
} A11_NVSE_FFLOW, *pA11_NVSE_FFLOW;

typedef struct _A11_NVSE_RFLOW {
    UCHAR   Length[2];
    UCHAR   SRID;
    UCHAR   RFlowCnt;
    A11_NVSE_FENTRY FENTRY;
} A11_NVSE_RFLOW, *pA11_NVSE_RFLOW;

typedef struct _A11_NVSE_GENTRY {
	UCHAR	Length;
	UCHAR	SRID;
	UCHAR	ServiceOption[2];
	UCHAR	GREProtoType[2];
	UCHAR	GREKey[4];
	UCHAR	PCFIP[4];
} A11_NVSE_GENTRY, *pA11_NVSE_GENTRY;

/* MIP : Registration Request */
typedef struct _MIP_REG_REQ {
	UCHAR Type;		/* 1 */
	UCHAR Flag;
	UCHAR Lifetime[2];
	UCHAR HomeAddress[4];
	UCHAR HomeAgent[4];
	UCHAR CareofAddress[4];
	UCHAR Identification[8];
	UCHAR Extension[1];
} MIP_REG_REQ, *PMIP_REG_REQ;

/* MIP : Registration Reply */
typedef struct _MIP_REG_REP {
	UCHAR Type;		/* 3 */
	UCHAR Code;
	UCHAR Lifetime[2];
	UCHAR HomeAddress[4];
	UCHAR HomeAgent[4];
	UCHAR Identification[8];
	UCHAR Extension[1];
} MIP_REG_REP, *PMIP_REG_REP;

/* L2TP Control Message Header */
typedef struct _L2TP_MSG_HDR {
	USHORT 	usControl;
	USHORT 	usLength;
	USHORT 	usTunnelID;
	USHORT 	usSessionID;
	USHORT 	usNs;
	USHORT 	usNr;
	USHORT 	usOffsetSize;
	USHORT 	usReserved;
} L2TP_MSG_HDR, *PL2TP_MSG_HDR;

typedef struct _L2TP_MSG_HDR1 {
	USHORT 	usControl;
	USHORT 	usTunnelID;
	USHORT 	usSessionID;
	USHORT 	usNs;
	USHORT 	usNr;
	USHORT 	usOffsetSize;
} L2TP_MSG_HDR1, *PL2TP_MSG_HDR1;

typedef struct _L2TP_AVP_HDR {
	UCHAR 	ucControl;
	UCHAR 	ucLength;
	USHORT 	usVendorID;
	USHORT 	usAVPType;
	UCHAR 	AVPValue[1];
} L2TP_AVP_HDR, *PL2TP_AVP_HDR;



#endif

