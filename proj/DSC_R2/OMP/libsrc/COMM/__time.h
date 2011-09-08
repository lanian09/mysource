#ifndef __TIME_H
#define __TIME_H

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>
#include <sys/time.h>

/*	====================================== 
 *
 *	NTP(Network Time Protocol)  : RFC 1059
 *
 *	======================================
 */

/*
 * Calendar arithmetic - contributed by G. Healton
 */
#define YEAR_BREAK 500      /* years < this are tm_year values:
                 * Break < AnyFourDigitYear && Break >
                 * Anytm_yearYear */

#define YEAR_PIVOT 98       /* 97/98: years < this are year 2000+
                 * FYI: official UNIX pivot year is
                 * 68/69 */

/*
 * Number of Days since 1 BC Gregorian to 1 January of given year
 */
#define julian0(year)   (((year) * 365 ) + ((year) > 0 ? (((year) + 3) \
                / 4 - ((year - 1) / 100) + ((year - 1) / \
                400)) : 0))

/*
 * Number of days since start of NTP time to 1 January of given year
 */
#define ntp0(year)  (julian0(year) - julian0(1900))

/*
 * Number of days since start of UNIX time to 1 January of given year
 */
#define unix0(year) (julian0(year) - julian0(1970))

/*
 * LEAP YEAR test for full 4-digit years (e.g, 1999, 2010)
 */
#define isleap_4(y) ((y) % 4 == 0 && !((y) % 100 == 0 && !(y % \
                400 == 0)))

/*
 * LEAP YEAR test for tm_year (struct tm) years (e.g, 99, 110)
 */
#define isleap_tm(y)    ((y) % 4 == 0 && !((y) % 100 == 0 && !(((y) \
                + 1900) % 400 == 0)))

/*
 * to convert simple two-digit years to tm_year style years:
 *
 *  if (year < YEAR_PIVOT)
 *      year += 100;
 *
 * to convert either two-digit OR tm_year years to four-digit years:
 *
 *  if (year < YEAR_PIVOT)
 *      year += 100;
 *
 *  if (year < YEAR_BREAK)
 *      year += 1900;
 */

/*
 * Operations for jitter calculations (these use doubles).
 *
 * Note that we carefully separate the jitter component from the
 * dispersion component (frequency error plus precision). The frequency
 * error component is computed as CLOCK_PHI times the difference between
 * the epoch of the time measurement and the reference time. The
 * precision componen is computed as the square root of the mean of the
 * squares of a zero-mean, uniform distribution of unit maximum
 * amplitude. Whether this makes statistical sense may be arguable.
 */
#define SQUARE(x) 	((x) * (x))
#define SQRT(x) 	(sqrt(x))
#define DIFF(x, y) 	(SQUARE((x) - (y)))
#define LOGTOD(a)   ((a) < 0 ? 1. / (1L << -(a)) : \
                	1L << (int)(a)) /* log2 to double */
#define UNIVAR(x)   (SQUARE(.28867513 * LOGTOD(x))) /* std uniform distr */
#define ULOGTOD(a)  (1L << (int)(a)) /* ulog2 to double */
#define MAXDISPERSE 16. /* max dispersion (square) */
#define MINDISPERSE .01 /* min dispersion */
#define MAXDISTANCE 1.  /* max root distance */

#define JAN_1970    	2208988800  /* 1970 - 1900 in seconds (0x83aa7e80) */

#define NTP_PORT    	123
#define NTP_OLDVERSION	1
#define NTP_VERSION		3
#define NTP_MINDPOLL	6	/* log2 default min poll interval (64 s) */
#define	NTP_MAXDPOLL	10	/* log2 default max poll interval (~17 m) */
#define	NTP_MINPOLL		4	/* log2 min poll interval (16 s) */
#define	NTP_MAXPOLL		17	/* log2 max poll interval (~4.5 h) */

/*
 * Values for peer.leap, sys_leap
 */
#define LEAP_NOWARNING  0x0 /* normal, no leap second warning */
#define LEAP_ADDSECOND  0x1 /* last minute of day has 61 seconds */
#define LEAP_DELSECOND  0x2 /* last minute of day has 59 seconds */
#define LEAP_NOTINSYNC  0x3 /* overload, clock is free running */

/*
 * Values for peer.mode
 */
#define MODE_UNSPEC 	0   /* unspecified (probably old NTP version) */
#define MODE_ACTIVE 	1   /* symmetric active */
#define MODE_PASSIVE    2   /* symmetric passive */
#define MODE_CLIENT 	3   /* client mode */
#define MODE_SERVER 	4   /* server mode */
#define MODE_BROADCAST  5   /* broadcast mode */
#define MODE_CONTROL    6   /* control mode packet */
#define MODE_PRIVATE    7   /* implementation defined function */
#define MODE_BCLIENT    8   /* broadcast client mode */

typedef struct ntp {
#ifdef  Linux
	unsigned char	li:2,		/* Leap Indicator 				*/
					vn:3,		/* Version Number				*/
					mode:3;
#endif

#ifdef SunOS
	unsigned char	mode:3,
					vn:3,		/* Version Number				*/
					li:2;		/* Leap Indicator 				*/
#endif	
	unsigned char 	stra;		/* Startum						*/
	unsigned char 	poll;		/* Poll							*/
	unsigned char 	prec;		/* Precision					*/
	unsigned int  	dist;		/* Synchronizing Distance		*/
	unsigned int  	drift;		/* Estimated Drift Rate 		*/
	unsigned int  	clkid;		/* Refernced Clock Identifier 	*/
	unsigned int  	reftm[2];	/* Referenced Timestamp 		*/
	unsigned int  	orgtm[2];	/* Originate Timestamp 			*/
	unsigned int  	rcvtm[2];	/* Received Timestamp 			*/
	unsigned int  	xmttm[2];	/* Transmit Timestamp 			*/
} ntp_t;

extern char* 	get_time (time_t t,char *buf);
extern int 		msleep(int tv_sec,int tv_usec);
extern char* 	makefmttm (struct tm *tmst, char *fmt, char *s);
extern int 		gettmst (struct tm* tmst, char *fmt, char *s);
extern char* 	fmttm(char *fmt,char *s);
extern char* 	gfmttm(time_t* tmsp,char* fmt,char *s);
extern time_t	rfmttm(char *fmt,char *s);
extern int 		settimeout(struct timeval *timeout,int sec,int usec);

#ifdef __cplusplus
}
#endif

#endif
