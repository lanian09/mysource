#ifndef IPP_H
#define IPP_H

/*
 * Author : hwang-hae-yeun <hyhwang@softteleware.com>
 *   Copyright 2004 hwang-hae-yeun
 *
 *   This program is NOT-free software; You can redistribute it
 *   and/or modify it under the terms of the Hwang's General Public
 *   License (HGPL) as published by the hwang senior engineer.
 *
 * Changes :
 *   - 2004/09/17 albamc <albamc@gmail.com> :
 *      - support multiple ippool lists :
 *      PDSN CU requires a multiple number of IP pools for each DU.
 */

/* Enable IPC? (using shared memory) */

/* IP pool size: 16 < nbit < 32
	example)
	nbit=16 : FFFF (65536) present bitmap 65536/8 = 8192 byte need.
*/
#define IPPSZ	65536
#define NBITS	(sizeof(int)*8)

#define IPMSZ	((IPPSZ+NBITS-1)/NBITS)

#define __howmany(__x, __y) (((__x)+((__y)-1))/(__y))

#define IPPZERO(p)		bzero((p), sizeof (*(p)))
#define IPPSET(n,p)		((p)->map[(n)/NBITS] |= (1ul<<((n)%NBITS)))
#define IPPCLR(n,p) 	((p)->map[(n)/NBITS] &= ~(1ul<<((n)%NBITS)))
#define IPPISSET(n,p)	(((p)->map[(n)/NBITS] & (1ul<<((n)%NBITS))) != 0l)

#define IPP_ROUND	0x00000001	/* round-robin or not 		*/
#define IPP_SEQ		0x00000002	/* sequence allocation or not	*/
#define IPP_SHARE	0x00000004	/* shared memory use or not 	*/
#define IPP_PRESHARE 	0x00000008 	/* pre-shared memory use or not */

#define IPP_KEY		0x20000L

typedef struct ipph {
	unsigned int 	net;
	unsigned int 	mask;
	unsigned int  	nbit;
	unsigned int  	total;
	unsigned int  	used;
	unsigned int  	free;
	unsigned int 	isip;	/* include	*/	
	unsigned int 	ieip;
	unsigned int 	xsip;	/* exclude */
	unsigned int 	xeip;

	int  			cur;
	int  			end;
	unsigned int 	map[IPMSZ];
} ipp_t;

#define MAXIPP		10

typedef struct ipplist {
	int 		nipp;
	int 		cipp;
	ipp_t 		ipp[MAXIPP];
} ipplist_t;

typedef struct _ipp {
	unsigned int addr;
	int index;
}_ipp_t;

#define MAXIPPLIST 	16

extern int ipptype; /* size: 8kbyte x MAXIPP    */
/* already shared memory allocated. */
extern ipplist_t* shmpre_ipplist;
extern int ippadd(char* netaddr, int nbit,char* xsaddr,char* xeaddr);
extern int ippaddn(int index, char* netaddr, int nbit,char* xsaddr,char* xeaddr);
extern int ippdel(char* netaddr, int nbit);
extern unsigned int ipalloc();
extern int ipallocn(char *s,int index);
//extern _ipp_t *ipallocn(int index);
extern void ipfree(unsigned int addr);
extern void ippflush();
extern int ippused();
extern int ippusedn(int);
extern int ipptotal();
extern int ipptotaln(int);
extern int ipset(unsigned int);
extern int ippfree();

#endif
