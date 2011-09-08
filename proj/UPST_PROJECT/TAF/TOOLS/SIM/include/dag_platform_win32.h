/*
 * Copyright (c) 2005 Endace Technology Ltd, Hamilton, New Zealand.
 * All rights reserved.
 *
 * This source code is proprietary to Endace Technology Limited and no part
 * of it may be redistributed, published or disclosed except as outlined in
 * the written contract supplied with this product.
 *
 * $Id: dag_platform_win32.h,v 1.1.1.1 2011/08/29 05:56:42 dcham Exp $
 */

#ifndef DAG_PLATFORM_WIN32_H
#define DAG_PLATFORM_WIN32_H

#if defined(_WIN32)

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif /* HAVE_CONFIG_H */

//#include <windows.h>
#include <getopt.h>
#include <io.h>
#include <sys/stat.h>
#include <ipchecksum.h>
#include <strmisc.h>
#include <strtok_r.h>
#include <timeofday.h>
#include <wintypedefs.h>
#include <ethertype.h>
#include <stdbool.h>

extern char* optarg;
extern int optind;
extern int opterr;

/* definition of IP header version 4 as per RFC 791 */
typedef struct
{ 
  u_char ip_ver_hl;      /* version and header length */ 
  u_char ip_tos;         /* type of service */ 
  short ip_len;          /* total length */ 
  u_short ip_id;         /* identification */ 
  short ip_off;          /* fragment offset field */ 
  u_char ip_ttl;         /* time to live */ 
  u_char ip_p;           /* protocol */ 
  u_short ip_cksum;      /* checksum */ 
  struct in_addr ip_src; /* source address */ 
  struct in_addr ip_dst; /* destination address */ 
  u_char ip_hl;          /* header length */
  u_char ip_v;           /* version */ 

} iphdr;

typedef struct
{
  unsigned short source;
  unsigned short dest;
  unsigned short len;
  unsigned short check;

} udphdr;

struct sockaddr_un {
  unsigned short sun_family;              /* address family AF_LOCAL/AF_UNIX */
  char	         sun_path[108]; /* 108 bytes of socket address     */
};


#ifndef PRIu64
#define PRIu64 "I64u"
#endif /* PRIu64 */

#ifndef PRId64
#define PRId64 "I64d"
#endif /* PRId64 */

#ifndef PRIx64
#define PRIx64 "I64x"
#endif /* PRIx64 */

#define __inline__

#ifndef INLINE 
#define INLINE __inline
#endif /* INLINE */

#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif /* STDOUT_FILENO */


/* Readline headers. */
#ifndef HAVE_READLINE
#define HAVE_READLINE 0
#endif /* HAVE_READLINE */


/* Byteswap code. */
#if defined(BYTESWAP)
#include <byteswap.h>
#else
ULONGLONG bswap_64(ULONGLONG x);
UINT32 bswap_32(UINT x);


#endif /* BYTESWAP */

/* Check IP checksum (for IP packets). */
#define IN_CHKSUM(IP) ip_sum_calc_win32((uint8_t *)(IP))
#define STDIN_FILENO (fileno(stdin))


/* Routines. */
uint16_t ip_sum_calc_win32(uint8_t* buff);
void* reallocf(void *ptr, size_t size);


#endif /* _WIN32 */

#endif /* DAG_PLATFORM_WIN32_H */ 
