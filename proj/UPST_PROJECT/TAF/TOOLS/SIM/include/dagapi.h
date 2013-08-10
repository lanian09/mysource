/*
 * Copyright (c) 2004-2005 Endace Technology Ltd, Hamilton, New Zealand.
 * All rights reserved.
 *
 * This source code is proprietary to Endace Technology Limited and no part
 * of it may be redistributed, published or disclosed except as outlined in
 * the written contract supplied with this product.
 *
 * $Id: dagapi.h,v 1.1.1.1 2011/08/29 05:56:42 dcham Exp $
 */

#ifndef DAGAPI_H
#define DAGAPI_H

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

/* DAG headers. */
#include "dagnew.h"
#include "dagreg.h"


/*****************************************************************************/
/* Macros and constants                                                      */
/*****************************************************************************/

/* GPP record type defines */
#define TYPE_LEGACY		0
#define TYPE_HDLC_POS		1
#define TYPE_ETH		2
#define TYPE_ATM		3
#define TYPE_AAL5		4
#define TYPE_MC_HDLC		5
#define TYPE_MC_RAW		6
#define TYPE_MC_ATM		7
#define TYPE_MC_RAW_CHANNEL	8
#define TYPE_MC_AAL5		9
#define TYPE_COLOR_HDLC_POS	10
#define TYPE_COLOR_ETH		11

#define TYPE_MIN	1	// sanity checking
#define TYPE_MAX	11	// sanity checking

#define dag_record_size   16
#define DAGF_NONBLOCK     0x01	// Deprecated, use dag_set_stream_poll

#define MAX_INTERFACES		16

#define PBM_TX_DEFAULT_SIZE	16


/*****************************************************************************/
/* Platform-specific headers and macros                                      */
/*****************************************************************************/

#if defined(__FreeBSD__) || defined(__linux__) || (defined(__APPLE__) && defined(__ppc__))

#include <inttypes.h>
#include <sys/time.h>

#elif defined(__SVR4) && defined(__sun)

#define TIMEVAL_TO_TIMESPEC(tv, ts)							\
	do {													\
		(ts)->tv_sec = (tv)->tv_sec;						\
		(ts)->tv_nsec = (tv)->tv_usec * 1000;				\
	} while (0)
	
#define TIMESPEC_TO_TIMEVAL(tv, ts)							\
	do {													\
		(tv)->tv_sec = (ts)->tv_sec;						\
		(tv)->tv_usec = (ts)->tv_nsec / 1000;				\
	} while (0)

#define timeradd(tvp, uvp, vvp)								\
	do {													\
		(vvp)->tv_sec = (tvp)->tv_sec + (uvp)->tv_sec;		\
		(vvp)->tv_usec = (tvp)->tv_usec + (uvp)->tv_usec;	\
		if ((vvp)->tv_usec >= 1000000) {					\
			(vvp)->tv_sec++;								\
        	(vvp)->tv_usec -= 1000000;						\
        }													\
	} while (0)	
			
#elif defined(_WIN32)

/* Nothing here, but it allows the error below to catch unsupported platforms. */

#else
#error Compiling on an unsupported platform - please contact <support@endace.com> for assistance.
#endif /* Platform-specific code. */


/*****************************************************************************/
/* Data structures                                                           */
/*****************************************************************************/

/* GPP Type 1 */
typedef struct pos_rec {
	uint32_t                hdlc;
	uint8_t           pload[1];
} pos_rec_t;

/* GPP Type 2 */
typedef struct eth_rec {
	uint8_t           offset;
	uint8_t           pad;
	uint8_t           dst[6];
	uint8_t           src[6];
	uint16_t          etype;
	uint8_t           pload[1];
} eth_rec_t;

/* GPP Type 3 */
typedef struct atm_rec {
	uint32_t          header;
	uint8_t           pload[1];
} atm_rec_t;

/* GPP Type 4 */
typedef struct aal5_rec {
	uint32_t          header;
	uint8_t           pload[1];
} aal5_rec_t;

/* GPP Type 5  */
typedef struct mc_hdlc_rec {
	uint32_t		  mc_header;
	uint8_t           pload[1];
} mc_hdlc_rec_t;

/* GPP Type 6  */
typedef struct mc_raw_rec {
	uint32_t		  mc_header;
	uint8_t           pload[1];
} mc_raw_rec_t;

/* GPP Type 7  */
typedef struct mc_atm_rec {
	uint32_t		  mc_header;
	uint8_t           pload[1];
} mc_atm_rec_t;

/* GPP Type 8  */
typedef struct mc_raw_channel_rec {
	uint32_t		  mc_header;
	uint8_t           pload[1];
} mc_raw_channel_rec_t;

/* GPP Type 9  */
typedef struct mc_aal5_rec {
	uint32_t		  mc_header;
	uint8_t           pload[1];
} mc_aal5_rec_t;


#ifdef __linux__
typedef struct flags {
    uint8_t           pad:2;
    uint8_t           dserror:1;
    uint8_t           rxerror:1;
    uint8_t           trunc:1;
    uint8_t           vlen:1;
    uint8_t           iface:2;
} flags_t;
#else
typedef struct flags {
    uint8_t           iface:2;
    uint8_t           vlen:1;
    uint8_t           trunc:1;
    uint8_t           rxerror:1;
    uint8_t           dserror:1;
    uint8_t           pad:2;
} flags_t;
#endif



/* GPP Global type */
typedef struct dag_record {
	uint64_t          ts;
	uint8_t           type;
	flags_t           flags;
	uint16_t          rlen;
	uint16_t          lctr;
	uint16_t          wlen;
	union {
		pos_rec_t       pos;
		eth_rec_t       eth;
		atm_rec_t       atm;
		aal5_rec_t      aal5;
		mc_hdlc_rec_t	mc_hdlc;
		mc_raw_rec_t	mc_raw;
		mc_atm_rec_t	mc_atm;
		mc_aal5_rec_t	mc_aal5;
        mc_raw_channel_rec_t mc_raw_channel;
	} rec;
} dag_record_t;


/*****************************************************************************/
/* Function declarations                                                     */
/*****************************************************************************/
int		dag_open(char *dagname);
int		dag_close(int dagfd);
int		dag_configure(int dagfd, char *params);
int 		dag_attach_stream(int dagfd, int stream_num, uint32_t flags, uint32_t extra_window_size);
int		dag_detach_stream(int dagfd, int stream_num);
int		dag_start_stream(int dagfd, int stream_num);
int		dag_stop_stream(int dagfd, int stream_num);
int		dag_get_stream_poll(int dagfd, int stream_num, uint32_t *mindata,
			struct timeval *maxwait, struct timeval *poll);
int		dag_set_stream_poll(int dagfd, int stream_num, uint32_t mindata,
			struct timeval *maxwait, struct timeval *poll);
int		dag_get_stream_buffer_size(int dagfd, int stream_num);
int		dag_get_stream_buffer_level(int dagfd, int stream_num);
int		dag_get_stream_last_buffer_level (int dagfd, int stream_num);
int		dag_rx_get_stream_count(int dagfd);
int		dag_tx_get_stream_count(int dagfd);

/* Block/Record transmit allocator - zero copy */
uint8_t		*dag_tx_get_stream_space(int dagfd, int stream_num, uint32_t size);

/* Block oriented transmit - zero copy */
uint8_t		*dag_tx_stream_commit_bytes(int dagfd, int stream_num, uint32_t size);

/* Block oriented transmit - COPIES DATA */
int 		dag_tx_stream_copy_bytes(int dagfd, int stream_num, uint8_t * orig, uint32_t size);

/* Record oriented receive interface - zero copy */
uint8_t 		*dag_rx_stream_next_record(int dagfd, int stream_num);

/* Record oriented rx/tx for inline forwarding */
uint8_t 		*dag_rx_stream_next_inline (int dagfd, int rx_stream_num, int tx_stream_num);

/* Traditional ringbuffer interface - zero copy. Replaces dag_offset */
uint8_t		*dag_advance_stream(int dagfd, int stream_num, uint8_t **bottom);


/*****************************************************************************/
/* Windows specific functions.                                               */
/*****************************************************************************/
#if defined(_WIN32)

HANDLE dag_gethandle(int dagfd);

#endif /* _WIN32 */

/* receives errno from the api */
int dag_get_last_error(void); 


/*****************************************************************************/
/* Deprecated Function declarations - provided for code compatibility only   */
/*****************************************************************************/
int		dag_start(int dagfd);
int		dag_stop(int dagfd);
void		*dag_mmap(int dagfd);
int		dag_offset (int dagfd, int* oldoffset, int flags);
void		dag_getpollparams(int *mindatap,
			struct timeval *maxwait, struct timeval *poll);
void		dag_setpollparams(int mindata,
			struct timeval *maxwait, struct timeval *poll);
int		dag_clone(int dagfd, int minor);

/* Subject to change */
daginf_t	*dag_info(int dagfd);
dag_reg_t	*dag_regs(int dagfd);
uint8_t		*dag_iom(int dagfd);

/* libpcap helper */
uint8_t      dag_linktype(int dagfd);

/* General helper function - here temporarily, will eventually be put in dagutil.c
 *
 * name is the user-supplied string specifying a DAG device and stream.
 *   e.g. "/dev/dag0:2", "dag0:2", "0:2", "/dev/dag0", "dag0", "0", ""
 *
 * The canonical version of the name ("/dev/dag0") and the stream number are 
 * returned in buffer (which has length buflen) and stream_number respectively.
 * Returns -1 and sets errno on failure (most likely because name couldn't be
 * matched against any of the above templates).
 */
int dag_parse_name(const char* name, char* buffer, int buflen, int* stream_number);

/* Buffer size for use with dag_parse_name(). */
#define DAGNAME_BUFSIZE 128

void* GetMmapRegion(int iDagFd, int iStream);


#endif /* DAGAPI_H */


