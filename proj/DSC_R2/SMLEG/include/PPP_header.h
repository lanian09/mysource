#ifndef _PROTO_PPP_HEADER
#define _PROTO_PPP_HEADER

#pragma pack(1)

#define PPP_IP        (0x0021)
#define PPP_VJCTCPIP  (0x002D)
#define PPP_VJUCTCPIP (0x002F)
#define PPP_CD        (0x00FD)
#define PPP_IPCP      (0x8021)
#define PPP_CCP1      (0x80FB)
#define PPP_CCP2      (0x80FD)
#define PPP_LCP       (0xC021)
#define PPP_PAP       (0xC023)
#define PPP_LQR       (0xC025)
#define PPP_CHAP      (0xC223)

#define MOBILE_LCP    1
#define MOBILE_IPCP   2
#define MOBILE_CHAP   3
#define MOBILE_ICMP   4
#define MOBILE_UDP    5
#define MOBILE_MIP    6
#define MOBILE_RADIUS 7
#define MOBILE_A11    8
#define MOBILE_PAP    9
#define MOBILE_CCP    10

typedef struct _PPP_DATA_HEADER {
	UCHAR Code;
	UCHAR ID;
	UCHAR Length[2];
	UCHAR Data[1];
} PPP_DATA_HEADER, *PPPP_DATA_HEADER;

typedef struct _IPCP_DATA {
	UCHAR Type;
	UCHAR Length;
	UCHAR Option[1];
} IPCP_DATA, *PIPCP_DATA;

typedef struct _LCP_DATA {
	UCHAR Type;
	UCHAR Length;
	UCHAR Option[1];
} LCP_DATA, *PLCP_DATA;

typedef struct _PAP_DATA {
	UCHAR Length;
	UCHAR Message[1];
} PAP_DATA, *PPAP_DATA;

typedef struct _CHAP_DATA {
	UCHAR Length;
	UCHAR String[1];
} CHAP_DATA, *PCHAP_DATA;

/* MIP : Agent Advertisement/Agent Solicitation */
typedef struct _MIP_AAAS {
	UCHAR NumAddrs;
	UCHAR AddrEntrySize;
	UCHAR Lifetime[2];
	UCHAR Data[1];
} MIP_AAAS, *PMIP_AAAS;

/* Mobility Agent Adertisement Extension */
typedef struct _MIP_MAAE {
	UCHAR Type;
	UCHAR Length;
	UCHAR SequenceNumber[2];
	UCHAR RegistrationLifetime[2];
	UCHAR Flag;
	UCHAR Reserved;
	UCHAR CareofAddress[4];
	UCHAR Data[1];
} MIP_MAAE, *PMIP_MAAE;

/* Prefix-length Extension */
typedef struct _MIP_PLE {
	UCHAR Type;		
	UCHAR Length;
	UCHAR PrefixLength;
	UCHAR Data[1];
} MIP_PLE, *PMIP_PLE;

/* Mobile IP Agent Adertisement Challenge Extension */
typedef struct _MIP_AACE {
	UCHAR Type;		
	UCHAR Length;
	UCHAR Challenge[1];
} MIP_AACE, *PMIP_AACE;

/* One-byte Padding Extension */
typedef struct _MIP_OPE {
	UCHAR Type;
} MIP_OPE, *PMIP_OPE;

#pragma pack(0)
#endif

