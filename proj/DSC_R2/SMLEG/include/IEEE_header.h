#ifndef _PROTOCOL_IEEE_HEADER_ETHERNET
#define _PROTOCOL_IEEE_HEADER_ETHERNET

#pragma pack(1)

/* BPDU : 2001.02.12 by joonhk */
typedef struct _BPDU {
	UCHAR ProtocolID[2];
	UCHAR Version;
	UCHAR BPDUType;
	UCHAR Flags;
	UCHAR RootID[8];
	UCHAR RootPathCost[4];
	UCHAR BridgeID[8];
	UCHAR PortID[2];
	UCHAR MessageAge[2];
	UCHAR MaximumAge[2];
	UCHAR HelloTime[2];
	UCHAR ForwardDelay[2];
} BPDU, *PBPDU;

/* LLC HEADER : 2001.02.12 by joonhk */
typedef struct _LLC_HDR {
	UCHAR DSAP;
	UCHAR SSAP;
	UCHAR Control;
} LLC_HDR, *PLLC_HDR;

/* IPX : 2001.02.16 by joonhk */
typedef struct _IPX {
	UCHAR Checksum[2];
	UCHAR PacketLength[2];
	UCHAR TransportControl;
	UCHAR PacketType;
	UCHAR DestinationNetwork[4];
	UCHAR DestinationNode[6];
	UCHAR DestinationSocket[2];
	UCHAR SourceNetwork[4];
	UCHAR SourceNode[6];
	UCHAR SourceSocket[2];
} IPX, *PIPX;

/* NETBIOS on 802.2 : 2001.02.19 by joonhk */
typedef struct _NETBIOS_802 {
	UCHAR Length[2];
	UCHAR Deliminator[2];
	UCHAR Command;
	UCHAR Data1;
	UCHAR Data2[2];
	UCHAR XMITCor[2];
	UCHAR RSPCor[2];
} NETBIOS_802, *PNETBIOS_802;

typedef struct _NETBIOS_802_2C {
	UCHAR DestinationName[16];
	UCHAR SourceName[16];
} NETBIOS_802_2C, *PNETBIOS_802_2C;

typedef struct _NETBIOS_802_0E {
	UCHAR DestinationNum;
	UCHAR SourceNum;
} NETBIOS_802_0E, *PNETBIOS_802_0E;

/* SMB : 2001.02.19 by joonhk */
typedef struct _SMB {
	UCHAR Deliminator;
	UCHAR ID[3];
	UCHAR Command;
	UCHAR ErrorClass;
	UCHAR Reserved;
	UCHAR ErrorCode[2];
	UCHAR Flags;
	UCHAR Flags2[2];
	UCHAR Padding[12];
	UCHAR TreeID[2];		/* authenticated resource identifier / Tree ID */
	UCHAR ProcessID[2];		/* caller's process ID */
	UCHAR UserID[2];		/* unathenticated user ID */
	UCHAR MultiplexID[2];	/* Multiple ID */
	UCHAR WordCount;		/* count of 16-bit fields Word count */
	UCHAR ByteCount[2];		/* variable no of 16-bit fields byte count */
	UCHAR Count[2];			/* count of 8-bit fields that follow */
	UCHAR VariableNum[2];	/* variable number of 8-bit fields */
} SMB, *PSMB;

#pragma pack(0)
#endif
