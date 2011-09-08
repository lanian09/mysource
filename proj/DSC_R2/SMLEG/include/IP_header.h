#ifndef _PROTOCOL_IP_HEADER_ETHERNET
#define _PROTOCOL_IP_HEADER_ETHERNET

#pragma pack(1)

#define PROTO_BPDU      10001
#define PROTO_802_3     10002
/* IPv4 Header */
typedef struct _IP_RHDR {
	UCHAR VerLen;
	UCHAR Service;
	UCHAR Length[2];
	UCHAR Ident[2];
	UCHAR Flagoff[2];
	UCHAR Timelive;
	UCHAR Protocol;
	UCHAR CheckSum[2];
	UCHAR Source[4];
	UCHAR Destination[4];
	UCHAR Data[1];
} IP_RHDR, *PIP_RHDR;

/* IPv6 Header */
typedef struct _IPNG_RHDR {
	UCHAR VerPrio;
	UCHAR FlowLabel[3];
	UCHAR Length[2];
	UCHAR NextHdr;
	UCHAR HopLimit;
	UCHAR Source[16];
	UCHAR Destination[16];
	UCHAR Data[1];
} IPNG_RHDR, *PIPNG_RHDR;

/* ARP HEADER : 2001.03.20 by hyon */
typedef struct _ARP {
	UCHAR HardType[2];
	UCHAR ProtType[2];
	UCHAR HardSize;
	UCHAR ProtSize;
	UCHAR Op[2];
	UCHAR SenderEthernetAddr[6];
	UCHAR SenderIPAddr[4];
	UCHAR TargetEthernetAddr[6];
	UCHAR TargetIPAddr[4];
} ARP, *PARP;

/* IGMP HEADER : 2001.03.20 by hyon */
typedef	struct	_IGMP {
	UCHAR	VerType;
	UCHAR	Unused;
	UCHAR	Checksum[2];
	UCHAR	GroupAddress[4];
} IGMP, *PIGMP;

/* EGP HEADER : 2001.03.20 by hyon */
typedef struct _EGP_HDR {
	UCHAR Version;		
	UCHAR Type;	
	UCHAR Code;
	UCHAR Status;				
	UCHAR Checksum[2];		
	UCHAR AutoSystem[2];	
	UCHAR Sequence[2];
} EGP_HDR, *PEGP_HDR;

/* OSPFIGP HEADER : 2001.03.20 by hyon */
typedef struct _OSPFIGP {
	UCHAR  Version;
	UCHAR  Type;
	UCHAR  Length[2];
	UCHAR  RouterID[4];
	UCHAR  AreaID[4];
	UCHAR  Checksum[2];
	UCHAR  AuType[2];
	UCHAR  Authentication1[4];
	UCHAR  Authentication2[4];
	UCHAR  Data[1];
} OSPFIGP, *POSPFIGP;

/* ICMP HEADER : 2001.03.21 by hyon */
typedef struct _ICMP_RHDR {
	UCHAR	Type;
	UCHAR	Code;
	UCHAR	Checksum[2];
	UCHAR   Data[1];
} ICMP_RHDR, *PICMP_RHDR;

/* ICMP TYPE0 : 2001.03.21 by hyon */
typedef struct _ICMP_TYPE0 {
	UCHAR	Identifier[2];
	UCHAR	SequenceNumber[2];
	UCHAR   Data[1];
} ICMP_TYPE0, *PICMP_TYPE0;


/* ICMP TYPE3 : 2001.03.21 by hyon */
typedef struct _ICMP_TYPE3 {
	UCHAR	unused[4];
/*	ICMP_IP	ip;  */
	UCHAR   Data[1];
} ICMP_TYPE3, *PICMP_TYPE3;


/* GRE (General Routing Encapsulation) */
typedef struct _GRE {
	UCHAR  FlagRecur;
	UCHAR  FlagVer;
	UCHAR  Protocol[2];
	UCHAR  Key[4];
	UCHAR  Sequence[4];
    UCHAR  Acknowledge[4];
	UCHAR  Data[1];
} GRE, *PGRE;


#pragma pack(0)
#endif

