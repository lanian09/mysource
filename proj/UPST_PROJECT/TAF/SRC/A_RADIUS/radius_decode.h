#ifndef _RADIUS_DECODE_H_
#define _RADIUS_DECODE_H_

/**
 * Include headers
 */
// LIB headers
#include "typedef.h"

#include "radius_func.h"

/**
 * Define constants
 */
#define MAX_BSMSC_SIZE	13


/**
 * Define structures
 */
/* LGT BMT NEW STRUCTURE 2006.04.11 */
typedef struct _st_Radius_
{
    UCHAR Code;
    UCHAR Identifier;
    UCHAR Length[2];
    UCHAR Authenticator[16];
    UCHAR Attributes[1];
} st_Radius, *pst_Radius;

/**
 * Declare functions
 */
extern void print_timegap(struct timeval *before, struct timeval *after, char *funcname, int linenum);
extern int dump_DebugString(char *debug_str, char *s, int len);
extern inline int dAnalyze_RADIUS( UCHAR *pBuf, pst_ACCInfo pstAccReq, INFO_ETH *pINFOETH );
extern inline int dAnalyze_RADIUS_ATTRIB( UCHAR *pBuf, USHORT usDataLen, pst_ACCInfo pstAccInfo );

#endif	/* _RADIUS_DECODE_H_ */
