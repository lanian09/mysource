#ifndef _PROTOCOL_HEADER_ETHERNET
#define _PROTOCOL_HEADER_ETHERNET

typedef struct _ETHERNET_HDR {
	UCHAR Destination[6];
	UCHAR Source[6];
	UCHAR Protocol[2];
	UCHAR Data[1];
} ETHERNET_HDR, *PETHERNET_HDR;

/* without FCS(4octets) */
/*** Modified by jwkim96  2004.02.15 ***/
/*** MIN_ETHERNET_SIZE: 60 => 54 ***/
#define	MIN_ETHERNET_SIZE	54 
#define MAX_ETHERNET_SIZE	1514	

#define PROTO_IP      (0x0800)
#define PROTO_ARP     (0x0806)
#define PROTO_RARP    (0x0835)
#define PROTO_XNS     (0x0600)
#define PROTO_SNMP    (0x814C)
#define PROTO_OLD_IPX (0x8137)
#define PROTO_NOVELL  (0x8138)
#define PROTO_IPNG    (0x86DD)
#define PROTO_PPP     (0x880B)

typedef struct _DATALINK_HDR {
    UCHAR Address;
    UCHAR Control;
    UCHAR Protocol[2];
    UCHAR Data[1];
} DATALINK_HDR, *PDATALINK_HDR;

typedef struct _RA_HDR {
    UCHAR Address[2];
    UCHAR Control;
    UCHAR Data[1];
} RA_HDR, *PRA_HDR;

#endif

