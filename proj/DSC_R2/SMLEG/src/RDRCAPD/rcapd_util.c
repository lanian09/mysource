#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "rcapd.h"

int timeSub (struct timeval *res,
		const struct timeval *after, const struct timeval *before)
{
	long sec = after->tv_sec - before->tv_sec;
	long usec = after->tv_usec - before->tv_usec;

	if (usec < 0)
		usec += 1000000, --sec;

	res->tv_sec = sec;
	res->tv_usec = usec;

	return (sec < 0) ? (-1) : ((sec == 0 && usec == 0) ? 0 : 1);
} 
