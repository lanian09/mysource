#ifndef DNS_H
#define DNS_H

typedef struct dns {
	unsigned short		id;			/* Identification		*/
#ifdef Linux
	unsigned short		rcode:4,
						cd:1,
						ad:1,
						z:1,
						ra:1,
						rd:1,
						tc:1,
						aa:1,
						opcode:4,
						qr:1;
#endif
#ifdef SunOS
	unsigned short		qr:1,		/* Query & Response		*/
						opcode:4,
						aa:1,		/* Authoritative		*/
						tc:1,		/* Truncated			*/
						rd:1,		/* Recursion Desired	*/
						ra:1,		/* Recursion Available	*/
						z:1,		/* 						*/
						ad:1,		/* 						*/
						cd:1,		/* 						*/
						rcode:4;	/* Return Code			*/
#endif
	unsigned short		qdcount;	/* #RRs in the questions section		*/
	unsigned short		ancount;	/* #RRs in the answer section 			*/
	unsigned short		nscount;	/* #RRs in the authority section 		*/
	unsigned short		arcount;	/* #RRs in the additional data section 	*/
} dns_t;

/* RFC 2136 : Dynamic Updates in the Domain Name System 
 */
typedef struct dnsupd {
	unsigned short		id;			/* Identification		*/
#ifdef Linux
	unsigned short		rcode:1,
						z:4,
						opcde:7,
						qr:4;
#endif
#ifdef SunOS
	unsigned short		qr:1,		/* Query & Response		*/
						opcode:4,
						z:7,
						rcode:4;	/* Return Code			*/
#endif

	unsigned short		zocount;	/* #RRs in the zone selection 			*/
	unsigned short		prcount;	/* #RRs in the prerequsite section 		*/
	unsigned short		upcount;	/* #RRs in the update section 			*/
	unsigned short		adcount;	/* #RRs in the additional data section 	*/
} dnsupd_t;

#define QR_QUERY		0
#define QR_RESPONSE		1

#define OP_QUERY		0		/* Standard Query : RFC 1035 		*/
#define OP_IQUERY		1		/* Inverse Query : RFC 1035 		*/
#define OP_STATUS		2		/* Serverv Status Query : RFC 1035 	*/
#define OP_NOTIFY		3		/* Notify : RFC 1996 				*/
#define OP_UPDATE		5		/* Update : RFC 2136 				*/

#define RC_NO_ERROR				0	
#define RC_FORMAT_ERROR			1
#define RC_SERVER_FAILURE		2
#define RC_NAME_ERROR			3
#define RC_NOT_IMPLEMENTED		4
#define RC_REFUSED				5
#define RC_YX_DOMAIN			6
#define RC_YX_RRSET				7
#define RC_NX_RRSET				8
#define RC_NOT_AUTH				9
#define RC_NOT_ZONE				10
#define RC_BAD_VERS				16
#define RC_BAD_KEY				17
#define RC_BAD_TIME				18
#define RC_BAD_MODE				19
#define RC_BAD_NAME				10
#define RC_BAD_ALG				21

/* TYPE */
#define	DNS_TYPE_A			1
#define	DNS_TYPE_NS			2
#define	DNS_TYPE_MD			3
#define	DNS_TYPE_MF			4
#define	DNS_TYPE_CNAME		5
#define	DNS_TYPE_SOA		6
#define	DNS_TYPE_MB			7
#define	DNS_TYPE_MG			8
#define	DNS_TYPE_MR			9
#define	DNS_TYPE_NULL		10
#define	DNS_TYPE_WKS		11
#define	DNS_TYPE_PTR		12
#define	DNS_TYPE_HINFO		13
#define	DNS_TYPE_MINFO		14
#define	DNS_TYPE_MX			15
#define	DNS_TYPE_TXT		16

/* QTYPE */
#define DNS_QTYPE_AXFR		252
#define DNS_QTYPE_MAILB		253
#define DNS_QTYPE_MAILA		254
#define DNS_QTYPE_ALL 		255

/* CLASS */
#define DNS_CLASS_IN		1
#define DNS_CLASS_CS		2
#define DNS_CLASS_CH		3
#define DNS_CLASS_HS		4

#endif
