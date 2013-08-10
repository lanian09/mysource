#ifndef PACKET_DEF_H
#define PACKET_DEF_H

#include <sys/time.h>
#include <unistd.h>

#pragma pack(1)
typedef struct _st_IPHeader_
{
	UINT    dSrcIP;                 // Source IP address    	: 4 
    UINT    dDestIP;                // Destination IP addr  	: 4 
	USHORT  usIPHeaderLen;          // IP Header Len     		: 2 
	USHORT  usTotLen;               // Pack len          		: 2 
	UCHAR   ucTimelive;             // Time To live      		: 1 
	UCHAR	ucProtocol;				// Protocol					: 1 
	USHORT	usIdentification;		// Identification
	USHORT	usAckHeaderLen;
	UINT	uiIPFrag;				//Frag Offset				: 4
	UCHAR	szReserved[2];
} st_IPHeader, *pst_IPHeader;

typedef struct _st_TCPHeader_
{
	UINT    dSeq;                   // TCP Session Info     		: 4 
    UINT    dAck;                   // TCP Session Info     		: 4 
	UINT    dWindow;                // TCP Session Info     		: 4 
	USHORT  usSrcPort;              // Source Port No.      		: 2 
    USHORT  usDestPort;             // Destination Port No. 		: 2 
	USHORT	usRtxType;				// RX, TX Type UP:2, DOWN:1 	: 2 
	USHORT	usTCPHeaderLen;			// TCP Header Length			: 2
	USHORT  usDataLen;              // TCP Data Len      			: 2
	UCHAR   ucControlType;          // TCP Control : SYN, ACK, RST, FIN, PSH에 대한것 : 1
	UCHAR	ucIPFrag;				// IP Fragmentation
} st_TCPHeader, *pst_TCPHeader;

typedef struct _st_IPTCPHeader_
{
//	struct timeval  tCapTime;       // Pack Cap Time     : 8
	st_IPHeader		stIPHeader;
	st_TCPHeader	stTCPHeader;

} st_IPTCPHeader, *pst_IPTCPHeader;

#define DEF_IPHDR_SIZE      sizeof(st_IPHeader)
#define DEF_TCPHDR_SIZE     sizeof(st_TCPHeader)
#define DEF_PACKHDR_SIZE    sizeof(st_IPTCPHeader)

/* 2005.04.11 FOR IP FRAGMENTATION */
#define DEF_CHECK_IPFRAG	0x2000
#define DEF_CHECK_OFFSET	0x1FFF

#define DEF_NON_IPFLAG		0x00
#define DEF_IPFRAG_START	0x01
#define DEF_IPFRAG_MID		0x02
#define DEF_IPFRAG_END		0x03
#define DEF_DONT_FRAG       0x4000  // 060728,POOPEE

#pragma pack()
#endif
