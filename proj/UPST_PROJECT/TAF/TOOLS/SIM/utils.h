/*
 * Copyright (c) 2002,2003 Endace Technology Ltd, Hamilton, New Zealand.
 * All rights reserved.
 *
 * This source code is proprietary to Endace Technology Limited and no part
 * of it may be redistributed, published or disclosed except as outlined in
 * the written contract supplied with this product.
 *
 * $Id: utils.h,v 1.1.1.1 2011/08/29 05:56:42 dcham Exp $
 *
 * Helper functions.
 */

/* Endace headers. */
#include "dagutil.h"

/* C Standard Library headers. */
#ifndef _WIN32
#include <inttypes.h>
#else /* _WIN32 */
#include <wintypedefs.h>
#endif /* _WIN32 */
/*
 * Swap byte ordering of uint64_t on a big endian
 * machine.
 */
uint64_t swapll(uint64_t ull);

/*
 * This routine implements a POSIX compliant way to get the seconds
 * offset from UTC. Unfortunately, struct tm's do not need to implement
 * some of the older fields any more, such as tm_gmtoff.
 */
int gmtoff(void);

/*
 * Default snaplength
 */
#define DEFAULT_SNAPLEN		2040

/*
 * ATM snaplength
 */
#define ATM_SNAPLEN		48

/*
 * Size of ATM payload 
 */
#define ATM_SLEN(h)		ATM_SNAPLEN
#define ATM_WLEN(h)		ATM_SNAPLEN

/*
 * Size of Ethernet payload
 */
#define ETHERNET_WLEN(h)	(ntohs((h)->wlen) - (fcs_bits >> 3))
#define ETHERNET_SLEN(h) 	dagutil_min(ETHERNET_WLEN(h), ntohs((h)->rlen) - dag_record_size - 2)

/*
 * Size of HDLC payload
 */
#define HDLC_WLEN(h)		(ntohs((h)->wlen) - (fcs_bits >> 3))
#define HDLC_SLEN(h)		dagutil_min(HDLC_WLEN(h), ntohs((h)->rlen) - dag_record_size)
