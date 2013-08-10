#ifndef _VJ
#define _VJ

#include <sys/types.h>
#include <stdint.h>

#pragma pack(1)

//#define IPPROTO_TCP	0

//yhshin
typedef unsigned char u_int8_t;
typedef unsigned short u_int16_t;
#if 0
typedef unsigned short u_short;
typedef unsigned char u_char;
typedef unsigned int uint32_t;
#endif

typedef unsigned int tcp_seq;

/* Internet address.  */
struct in_addr_x
  {
    uint32_t s_addr_x;
  };

/*
 * TCP header.
 * Per RFC 793, September, 1981.
 */

/*
 * Structure of an internet header, naked of options.
 */

// yshin
//#define __BYTE_ORDER __LITTLE_ENDIAN

struct my_ip
  {
//#if __BYTE_ORDER == __LITTLE_ENDIAN
#if LINUX
    unsigned char ip_hl:4;       /* header length */
    unsigned char ip_v:4;        /* version */
#else
    unsigned char ip_v:4;        /* version */
    unsigned char ip_hl:4;       /* header length */
#endif

/*
 *#if __BYTE_ORDER == __BIG_ENDIAN
 *   unsigned int ip_v:4;       //  version
 *   unsigned int ip_hl:4;      //  header length
 *#endif
 */

    u_int8_t ip_tos;        /* type of service */
    u_short ip_len;         /* total length */
    u_short ip_id;          /* identification */
    u_short ip_off;         /* fragment offset field */

#define IP_RF 0x8000            /* reserved fragment flag */
#define IP_DF 0x4000            /* dont fragment flag */
#define IP_MF 0x2000            /* more fragments flag */
#define IP_OFFMASK 0x1fff       /* mask for fragmenting bits */

    u_int8_t ip_ttl;        /* time to live */
    u_int8_t ip_p;          /* protocol */
    u_short ip_sum;         /* checksum */

    struct in_addr_x ip_src, ip_dst;  /* source and dest address */
  };


#if 1
struct tcphdr
  {
    u_int16_t th_sport;     /* source port */
    u_int16_t th_dport;     /* destination port */
    tcp_seq th_seq;     /* sequence number */
    tcp_seq th_ack;     /* acknowledgement number */

//#if __BYTE_ORDER == __LITTLE_ENDIAN
#if LINUX
    u_int8_t th_x2:4;       /* (unused) */
    u_int8_t th_off:4;      /* data offset */
#else
    u_int8_t th_off:4;      /* data offset */
    u_int8_t th_x2:4;       /* (unused) */
#endif

/*
#if __BYTE_ORDER == __BIG_ENDIAN
    u_int8_t th_off:4;      // data offset
    u_int8_t th_x2:4;       // (unused)
#endif
*/

    u_int8_t th_flags;
#define TH_FIN  0x01
#define TH_SYN  0x02
#define TH_RST  0x04
#define TH_PUSH 0x08
#define TH_ACK  0x10
#define TH_URG  0x20
    u_int16_t th_win;       /* window */
    u_int16_t th_sum;       /* checksum */
    u_int16_t th_urp;       /* urgent pointer */
};
#endif

/*
A.  Sample Implementation

    The following is a sample implementation of the protocol described in
    this document.

    Since many people who might have the deal with this code are familiar
    with the Berkeley Unix kernel and its coding style (affectionately known
    as kernel normal form), this code was done in that style.  It uses the
    Berkeley `subroutines' (actually, macros and/or inline assembler
    expansions) for converting to/from network byte order and
    copying/comparing strings of bytes.  These routines are briefly
    described in sec. A.5 for anyone not familiar with them.

    This code has been run on all the machines listed in the table on page
    24.  Thus, the author hopes there are no byte order or alignment
    problems (although there are embedded assumptions about alignment that
    are valid for Berkeley Unix but may not be true for other IP
    implementations --- see the comments mentioning alignment in
    sl_compress_tcp and sl_decompress_tcp).

    There was some attempt to make this code efficient.  Unfortunately, that
    may have made portions of it incomprehensible.  The author apologizes
    for any frustration this engenders.  (In honesty, my C style is known to
    be obscure and claims of `efficiency' are simply a convenient excuse.)

    This sample code and a complete Berkeley Unix implementation is
    available in machine readable form via anonymous ftp from Internet host
    ftp.ee.lbl.gov (128.3.254.68), file cslip.tar.Z. This is a compressed
    Unix tar file.  It must be ftped in binary mode.

    All of the code in this appendix is covered by the following copyright:
*/

/*
 * Copyright (c) 1989 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are
 * permitted provided that the above copyright notice and this
 * paragraph are duplicated in all such forms and that any
 * documentation, advertising materials, and other materials
 * related to such distribution and use acknowledge that the
 * software was developed by the University of California,
 * Berkeley.  The name of the University may not be used to
 * endorse or promote products derived from this software
 * without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE
 * IMPLIED WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

/*
A.1  Definitions and State Data
*/

#define MAX_STATES 16   /* must be >2 and <255 */
#define MAX_HDR 128     /* max TCP+IP hdr length (by protocol def) */

/* packet types */
#define TYPE_IP 0x40
#define TYPE_UNCOMPRESSED_TCP 0x70
#define TYPE_COMPRESSED_TCP 0x80
#define TYPE_ERROR 0x00 /* this is not a type that ever appears on  */
                        /* the wire.  The receive framer uses it to */
                        /* tell the decompressor there was a packet */
                        /* transmission error.                      */
/*
 * Bits in first octet of compressed packet
 */

/* flag bits for what changed in a packet */

#define NEW_C  0x40
#define NEW_I  0x20
#define TCP_PUSH_BIT 0x10

#define NEW_S  0x08
#define NEW_A  0x04
#define NEW_W  0x02
#define NEW_U  0x01

/* reserved, special-case values of above */
#define SPECIAL_I (NEW_S|NEW_W|NEW_U)        /* echoed interactive traffic */
#define SPECIAL_D (NEW_S|NEW_A|NEW_W|NEW_U)  /* unidirectional data        */
#define SPECIALS_MASK (NEW_S|NEW_A|NEW_W|NEW_U)


/*
 * "state" data for each active tcp conversation on the wire.  This is
 * basically a copy of the entire IP/TCP header from the last packet together
 * with a small identifier the transmit & receive ends of the line use to
 * locate saved header.
 */

struct cstate {
    struct cstate *cs_next;  /* next most recently used cstate (xmit only) */
    u_short cs_hlen;         /* size of hdr (receive only) */
    u_char cs_id;            /* connection # associated with this state */
    u_char cs_filler;
    union {
        char hdr[MAX_HDR];
        struct my_ip csu_ip;      /* ip/tcp hdr from most recent packet */
    } slcs_u;
};
#define cs_ip slcs_u.csu_ip

#define cs_hdr slcs_u.csu_hdr

/*
 * all the state data for one serial line (we need one of these per line).
 */

struct slcompress {
    struct cstate *last_cs;            /* most recently used tstate */
    u_char last_recv;                  /* last rcvd conn. id */
    u_char last_xmit;                  /* last sent conn. id */
    u_short flags;
    struct cstate tstate[MAX_STATES];  /* xmit connection states */
    struct cstate rstate[MAX_STATES];  /* receive connection states */

	// by sykim
	u_char cnt;
	u_char reserved[7];
};

/* flag values */
#define SLF_TOSS 1       /* tossing rcvd frames because of input err */

/*
 * The following macros are used to encode and decode numbers.  They all
 * assume that `cp' points to a buffer where the next byte encoded (decoded)
 * is to be stored (retrieved).  Since the decode routines do arithmetic,
 * they have to convert from and to network byte order.
 */

/*
 * ENCODE encodes a number that is known to be non-zero.  ENCODEZ checks for
 * zero (zero has to be encoded in the long, 3 byte form).
 */

#define ENCODE(n) { \
    if ((u_short)(n) >= 256) \
        { \
        *cp++ = 0; \
        cp[1] = (n); \
        cp[0] = (n) >> 8; \
        cp += 2; \
        } \
    else \
        { \
        *cp++ = (n); \
        } \
}

#define ENCODEZ(n) { \
    if ((u_short)(n) >= 256 || (u_short)(n) == 0) \
        { \
        *cp++ = 0; \
        cp[1] = (n); \
        cp[0] = (n) >> 8; \
        cp += 2; \
        } \
    else \
        { \
        *cp++ = (n); \
        } \
}


/*
 * DECODEL takes the (compressed) change at byte cp and adds it to the
 * current value of packet field 'f' (which must be a 4-byte (long) integer
 * in network byte order).  DECODES does the same for a 2-byte (short) field.
 * DECODEU takes the change at cp and stuffs it into the (short) field f.
 * 'cp' is updated to point to the next field in the compressed header.
 */

#define DECODEL(f) { \
    if (*cp == 0) \
        { \
        (f) = htonl(ntohl(f) + ((cp[1] << 8) | cp[2])); \
        cp += 3; \
        } \
    else \
        { \
        (f) = htonl(ntohl(f) + (unsigned long)*cp++); \
        } \
}

#define DECODES(f) { \
    if (*cp == 0) \
        { \
        (f) = htons(ntohs(f) + ((cp[1] << 8) | cp[2])); \
        cp += 3; \
        } \
    else \
        { \
        (f) = htons(ntohs(f) + (unsigned long)*cp++); \
        } \
}

#define DECODEU(f) { \
    if (*cp == 0) \
        { \
        (f) = htons((cp[1] << 8) | cp[2]); \
        cp += 3; \
        } \
    else \
        { \
        (f) = htons((unsigned long)*cp++); \
        } \
}

/*
A.5  Berkeley Unix dependencies

    Note:  The following is of interest only if you are trying to bring the
    sample code up on a system that is not derived from 4BSD (Berkeley
    Unix).

    The code uses the normal Berkeley Unix header files (from
    /usr/include/netinet) for definitions of the structure of IP and TCP
    headers.  The structure tags tend to follow the protocol RFCs closely
    and should be obvious even if you do not have access to a 4BSD
    system./48/

    ----------------------------
    48. In the event they are not obvious, the header files (and all the
    Berkeley networking code) can be anonymous ftp'd from host


    The macro BCOPY(src, dst, amt) is invoked to copy amt bytes from src to
    dst.  In BSD, it translates into a call to bcopy.  If you have the
    misfortune to be running System-V Unix, it can be translated into a call
    to memcpy.  The macro OVBCOPY(src, dst, amt) is used to copy when src
    and dst overlap (i.e., when doing the 4-byte alignment copy).  In the
    BSD kernel, it translates into a call to ovbcopy.  Since AT&T botched
    the definition of memcpy, this should probably translate into a copy
    loop under System-V.

    The macro BCMP(src, dst, amt) is invoked to compare amt bytes of src and
    dst for equality.  In BSD, it translates into a call to bcmp.  In
    System-V, it can be translated into a call to memcmp or you can write a
    routine to do the compare.  The routine should return zero if all bytes
    of src and dst are equal and non-zero otherwise.

    The routine ntohl(dat) converts (4 byte) long dat from network byte
    order to host byte order.  On a reasonable cpu this can be the no-op
    macro:

        #define ntohl(dat) (dat)

    On a Vax or IBM PC (or anything with Intel byte order), you will have to
    define a macro or routine to rearrange bytes.

    The routine ntohs(dat) is like ntohl but converts (2 byte) shorts
    instead of longs.  The routines htonl(dat) and htons(dat) do the inverse
    transform (host to network byte order) for longs and shorts.

    A struct mbuf is used in the call to sl_compress_tcp because that
    routine needs to modify both the start address and length if the
    incoming packet is compressed.  In BSD, an mbuf is the kernel's buffer
    management structure.  If other systems, the following definition should
    be sufficient:
*/

        struct mbuf {
            u_char  *m_off; /* pointer to start of data */
            int     m_len;  /* length of data */
        };

        #define mtod(m, t) ((t)(m->m_off))
/*
    ----------------------------
    ucbarpa.berkeley.edu, files pub/4.3/tcp.tar and pub/4.3/inet.tar.
*/

void List_sl_compress_init(struct slcompress *comp);
u_char *List_sl_uncompress_tcp(u_char *bufp, int len, unsigned int type, struct slcompress *comp);
void sl_compress_init(struct slcompress *comp);
u_char *sl_uncompress_tcp(u_char *bufp, int len, unsigned int type, struct slcompress *comp);

#pragma pack(0)
#endif

