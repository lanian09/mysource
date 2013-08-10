/*
 * Copyright (c) 2005 Endace Technology Ltd, Hamilton, New Zealand.
 * All rights reserved.
 *
 * This source code is proprietary to Endace Technology Limited and no part
 * of it may be redistributed, published or disclosed except as outlined in
 * the written contract supplied with this product.
 *
 * $Id: dag_platform_freebsd.h,v 1.1.1.1 2011/08/29 05:56:42 dcham Exp $
 */

#ifndef DAG_PLATFORM_FREEBSD_H
#define DAG_PLATFORM_FREEBSD_H

#if defined(__FreeBSD__)

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif /* HAVE_CONFIG_H */

/* CAUTION: FreeBSD is much pickier about header file ordering than Linux! */

/* POSIX headers. */
#include <sys/types.h>
#include <sys/socket.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/un.h>


/* C Standard Library headers. */
#include <inttypes.h>
#include <stdbool.h>


#ifndef PRIu64
#define PRIu64 "llu"
#endif /* PRIu64 */

#ifndef PRId64
#define PRId64 "lld"
#endif /* PRId64 */

#ifndef PRIx64
#define PRIx64 "llx"
#endif /* PRIx64 */

#ifndef INLINE
#define INLINE inline
#endif /* INLINE */


/* Readline header default. */
#ifndef HAVE_READLINE
#define HAVE_READLINE 1
#endif /* HAVE_READLINE */


/* Byteswap code. */
#if defined(BYTESWAP)
#include <byteswap.h>
#else
#include <machine/endian.h>
#if defined(__byte_swap_int)
#define bswap_64(x) __bswap64(x)
#else
#define bswap_64(x)                               \
    (__extension__                                \
     ({ union { __extension__ uint64_t __ll;      \
     uint32_t __l[2]; } __w, __r;                 \
     __w.__ll = (x);                              \
     __r.__l[0] = __byte_swap_long (__w.__l[1]);  \
     __r.__l[1] = __byte_swap_long (__w.__l[0]);  \
     __r.__ll; }))
#endif /* __byte_swap_int */
#endif /* BYTESWAP */

/* Check IP checksum (for IP packets). */
#include <machine/in_cksum.h>
#define IN_CHKSUM(IP) in_cksum_hdr((struct ip *)IP)

#endif /* __FreeBSD__ */

#endif /* DAG_PLATFORM_FREEBSD_H */ 
