#ifndef COMM_LOGLEVEL_H
#define COMM_LOGLEVEL_H

#include "define.h"

#pragma pack(1)
/* General Macro */
#define LOGLVL_MAX_APP_NUM      30  /* The max number of applications */
#define LOGLVL_KIND             2   /* the number of kinds of log     */
#define LOGLVL_KIND_MSG         0   /* Message - kind of log          */
#define LOGLVL_KIND_APP         1   /* Message - kind of log          */

/* ADD BY YOON 2008.09.22 */
#define LOGLVL_APPINX_NUM           LOGLVL_MAX_APP_NUM  // modi by jjinri 2009.05.05
#define LOGLVL_TYPE_NUM             5

//#define LOGLVL_APPINX_UAWAPANA      0
//#define LOGLVL_APPINX_AAAIF         1
//#define LOGLVL_APPINX_UDRGEN        2
#define LOGLVL_APPINX_CAPD          0
#define LOGLVL_APPINX_PANA          1
//#define LOGLVL_APPINX_CDR           5
#define LOGLVL_APPINX_TRCDR0         2
#define LOGLVL_APPINX_TRCDR1         3
#define LOGLVL_APPINX_TRCDR2         4
#define LOGLVL_APPINX_TRCDR3         5
#define LOGLVL_APPINX_TRCDR4         6
//#define LOGLVL_APPINX_WAP1ANA       7
//#define LOGLVL_APPINX_WAP2ANA       8
//#define LOGLVL_APPINX_HTTPANA       9
//#define LOGLVL_APPINX_SDMD          10
//#define LOGLVL_APPINX_VODSANA       11
//#define LOGLVL_APPINX_WIPINWANA		12
//#define LOGLVL_APPINX_JAVANWANA     13
//#define LOGLVL_APPINX_LOGM			14
//#define LOGLVL_APPINX_REANA			15
//#define LOGLVL_APPINX_FBANA			16
//#define LOGLVL_APPINX_VTANA			17
#define LOGLVL_APPINX_RANA			7
//#define LOGLVL_APPINX_OZCDR			19
//#define LOGLVL_APPINX_MEMD			20
//#define LOGLVL_APPINX_WVANA			21
//#define LOGLVL_APPINX_CDR2			22
//#define LOGLVL_APPINX_PTOPANA		23
#define LOGLVL_APPINX_RDRANA		8 // ADD By jjinri 2009.04.24
#define LOGLVL_APPINX_SMPP			9 /* by june, 2009.04.26 */
#define LOGLVL_APPINX_RDRCAPD		10 /* by june, 2009.04.26 */
#define LOGLVL_APPINX_MMCR			11 /* by jjinri, 2009.05.05 */
#define LOGLVL_APPINX_IXPC			12 /* by sjjeon, 2009.05.15 */
#define LOGLVL_APPINX_SAMD			13 /* by sjjeon, 2009.05.15 */

// for test.. it's a temporary variable for testing  
//#define	LOGLVL_APPINX_WAP1TEST		11


typedef struct _stLogLevel{
    unsigned char loglevel [LOGLVL_APPINX_NUM][LOGLVL_KIND];
}stLogLevel;

#pragma pack(0)

#endif /* COMM_LOGLEVEL_H */
