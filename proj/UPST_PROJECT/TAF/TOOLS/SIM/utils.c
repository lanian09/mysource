/*
 * Copyright (c) 2002-2005 Endace Technology Ltd, Hamilton, New Zealand.
 * All rights reserved.
 *
 * This source code is proprietary to Endace Technology Limited and no part
 * of it may be redistributed, published or disclosed except as outlined in
 * the written contract supplied with this product.
 *
 * $Id: utils.c,v 1.1.1.1 2011/08/29 05:56:42 dcham Exp $
 *
 * Helper functions.
 */

/* File header. */
#include "utils.h"

/* C Standard Library headers. */
#include <time.h>
#include <stdlib.h>

/*
 * Swap byte ordering of uint64_t on a big endian
 * machine.
 */
uint64_t
swapll(uint64_t ull)
{
#if (BYTE_ORDER == BIG_ENDIAN)
	return ((ull & 0xff00000000000000LL) >> 56) |
	    ((ull & 0x00ff000000000000LL) >> 40) |
	    ((ull & 0x0000ff0000000000LL) >> 24) |
	    ((ull & 0x000000ff00000000LL) >> 8) |
	    ((ull & 0x00000000ff000000LL) << 8) |
	    ((ull & 0x0000000000ff0000LL) << 24) |
	    ((ull & 0x000000000000ff00LL) << 40) |
	    ((ull & 0x00000000000000ffLL) << 56);
#else
	return ull;
#endif
}

/*
 * This routine implements a POSIX compliant way to get the seconds
 * offset from UTC. Unfortunately, struct tm's do not need to implement
 * some of the older fields any more, such as tm_gmtoff.
 */
int
gmtoff(void)
{
	time_t          now;
	struct tm       loc,
	                utc;
	int             tdiff = 0;

	now = time(NULL);
	loc = *localtime(&now);
	utc = *gmtime(&now);

	tdiff = ((loc.tm_hour * 60 + loc.tm_min) * 60 + loc.tm_sec) -
	    ((utc.tm_hour * 60 + utc.tm_min) * 60 + utc.tm_sec);
	if((loc.tm_yday > utc.tm_yday) || (loc.tm_year > utc.tm_year))
		tdiff += 86400;
	else if((loc.tm_yday < utc.tm_yday) || (loc.tm_year < utc.tm_year))
		tdiff -= 86400;
	return tdiff;
}

/*
*	$Log $
*/

