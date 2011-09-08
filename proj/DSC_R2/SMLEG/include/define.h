/**********************************************************
 *           ABLEX DEFINITION
 *
 *           Author   : LEE SANG HO
 *           Section  : IPAS Project
 *           SCCS ID  : %W%
 *           Date     : %G%
 *           Revision History :
 *                      '03.  1.  15 Initial
 *           Description:
 *
 *            Copyright (c) ABLEX 2001
 ************************************************************/

#ifndef _DEFINE____________H
#define _DEFINE____________H

#include <string.h>

#define DEF_PREPAID_CONF_FILE       "/DSC/NEW/DATA/PPS.conf"
#define DEF_ICMP_CONF_FILE       	"/DSC/NEW/DATA/ICMP.conf"
#define DEF_BCAST_CONF_FILE       	"/DSC/NEW/DATA/BCAST.conf"
#define DEF_URLMATCH_FILE           "/DSC/NEW/DATA/UDR_CATEGORY.conf"
#define DEF_UDR_DUMPCONF_FILE       "/DSC/NEW/DATA/UDR_DUMP.conf"
#define DEF_UDR_TXCCONF_FILE        "/DSC/NEW/DATA/UDR_TXC.conf"
#define DEF_CDR_SVCTYPE_FILE        "/DSC/NEW/DATA/UDR_CDRINFO.conf"
#define DEF_DATA_SVCTYPE_FILE       "/DSC/NEW/DATA/SERVICE_TYPE.conf"
#define DEF_DATA_SVCOPT_FILE        "/DSC/NEW/DATA/SVC_OPT.conf"
#define UDRGEN_DUMP_PREFIX          "/DSC/LOG/UDR"

/* 20080718 PDSN TYPE INFO */
#define DEF_PDSNTYPEINFO_FILE		"/DSC/NEW/DATA/PDSN_TYPE.conf"

#define DEF_INIT_FILE 	START_PATH"/NEW/DATA/INIT_IPAF.dat"
#define DEF_VER_FILE 	START_PATH"/NEW/DATA/SW_VER.dat"
#define DEF_SVC_TIMEOUT	START_PATH"/NEW/DATA/SVC_TIMEOUT.dat"

#define DEF_MACS_TIMEOUT_FILE	START_PATH"/NEW/DATA/MACS_TIMEOUT.dat"
#define DEF_WICGS_TIMEOUT_FILE	START_PATH"/NEW/DATA/WICGS_TIMEOUT.dat"

#define MAX_MIN_SIZE        17      /* Calling Station ID */
#define MAX_MIN_LEN			16		/* MAX_MIN_SIZE-1, should be 8's multiple number */
#define MAX_MSISDN_SIZE     17
#define MAX_MODEL_SIZE      11      /* Phone Model Name */

#define MAX_IPAF_MULTI      2       /* IPAF Muliple Count : Example IWF -> 2, PDSN -> 1 */
#define MAX_IPAF_PAIR       2       /* IPAF Pair Count  : Allways 2 */

#define MAX_CATESVC_COUNT   21      /* Max Category Service Count */
#define MAX_WATCH_COUNT     10      /* Max Watch Count */


#define MAX_IMSI_SIZE       17      /* MAX DSCP IMSI Size */
#define MAX_WINSVCNAME_SIZE 6       /* MAX DSCP WIN Service Name Size */
#define MAX_CELLINFO_SIZE   17      /* MAX DSCP CELLINFO Size */

#define MAX_FILENAME_SIZE   256     /* File Name Size */

#define MAX_CON_SIZE		64		/* MAX Category content size */
#define MAX_CON_COUNT		100		/* MAX Category content count */

/* DEFINE LOG LEVEL */
#define LOG_NOPRINT         0
#define LOG_CRI             1
#define LOG_WARN            2
#define LOG_DEBUG           3
#define LOG_INFO            4

#define LOG_TYPE_DEBUG  1
#define LOG_TYPE_WRITE  2

#define TCP_FIN     0x01
#define TCP_SYN     0x02
#define TCP_RST     0x04
#define TCP_PSH     0x08
#define TCP_ACK     0x10
#define TCP_SYNACK  0x12

/* ADD BY HWH */
#define DEF_FILLTEROUT_OFF		0x00
#define DEF_FILLTEROUT_ON		0x01
#define DEF_FILLTEROUT_RPASS	0x02
#define DEF_QUD_SND_TIME	5

#define DEF_FLAG_OFF			0x00
#define DEF_FLAG_ON				0x01

#define DEF_CALL_CLEAR_NOREMAIN	1000

#define MAX_AAA_SOCK    1
//#define MAX_UDR_COUNT   3
#define MAX_UDR_COUNT   2

#define SYS_BSDA    "DSCA"
#define SYS_BSDB    "DSCB"

#define MAX_RADIUS_PKTSIZE     5000
#define TRACE_IMSI      1
#define TRACE_MSISDN    2

#define MAX_TRACE_LEN   32
#define MAX_TRCMSG_SIZE 2700

/* TRACE INFO : challa(20060627) */ 
#define DEF_INDEX_INIT         100
#define MAX_INDEX_VALUE        60000

#endif
