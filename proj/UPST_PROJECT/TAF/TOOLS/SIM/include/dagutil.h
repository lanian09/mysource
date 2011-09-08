/*
 * Copyright (c) 2002-2005 Endace Technology Ltd, Hamilton, New Zealand.
 * All rights reserved.
 *
 * This source code is proprietary to Endace Technology Limited and no part
 * of it may be redistributed, published or disclosed except as outlined in
 * the written contract supplied with this product.
 *
 * $Id: dagutil.h,v 1.1.1.1 2011/08/29 05:56:42 dcham Exp $
 */

#ifndef DAGUTIL_H
#define DAGUTIL_H

/* DAG headers. */
#include "dag_platform.h"
#include "dagnew.h"
#include "dagreg.h"
#include "dagpci.h"

/* Macros defined on some platforms but not others. */
#ifndef LINE_MAX
#define LINE_MAX 1024
#endif /* LINE_MAX */

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif /* PATH_MAX */

#ifndef O_LARGEFILE
#define O_LARGEFILE 0
#endif /* O_LARGEFILE */

#ifndef O_DIRECT
#define O_DIRECT 0
#endif /* O_DIRECT */

#ifndef UINT8_MAX
#define UINT8_MAX 255
#endif /* UINT8_MAX */

#ifndef UINT16_MAX
#define UINT16_MAX 65535
#endif /* UINT16_MAX */


/* SI units. */
#ifndef ONE_KIBI
#define ONE_KIBI (1024)
#endif /* ONE_KIBI */

#ifndef ONE_MEBI
#define ONE_MEBI (1048576)
#endif /* ONE_MEBI */

#ifndef ONE_GIBI
#define ONE_GIBI (1073741824)
#endif /* ONE_GIBI */

#ifndef ONE_TEBI
#define ONE_TEBI (1099511627776ULL)
#endif /* ONE_TEBI */


/* Useful cross-platform macros. */
#ifndef dagutil_min
#define dagutil_min(X,Y) ((X)>(Y))?(Y):(X)
#endif /* dagutil_min */

#ifndef dagutil_max
#define dagutil_max(X,Y) ((X)>(Y))?(X):(Y)
#endif /* dagutil_max */

#ifndef MIN
#define MIN(X,Y) ((X)>(Y))?(Y):(X) 
#endif /* MIN */

#ifndef MAX
#define MAX(X,Y) ((X)>(Y))?(X):(Y)
#endif /* MAX */


/* Useful bit constant macros. */
enum
{
	BIT0  = (1<<0),
	BIT1  = (1<<1),
	BIT2  = (1<<2),
	BIT3  = (1<<3),
	BIT4  = (1<<4),
	BIT5  = (1<<5),
	BIT6  = (1<<6),
	BIT7  = (1<<7),
	BIT8  = (1<<8),
	BIT9  = (1<<9),
	BIT10 = (1<<10),
	BIT11 = (1<<11),
	BIT12 = (1<<12),
	BIT13 = (1<<13),
	BIT14 = (1<<14),
	BIT15 = (1<<15),
	BIT16 = (1<<16),
	BIT17 = (1<<17),
	BIT18 = (1<<18),
	BIT19 = (1<<19),
	BIT20 = (1<<20),
	BIT21 = (1<<21),
	BIT22 = (1<<22),
	BIT23 = (1<<23),
	BIT24 = (1<<24),
	BIT25 = (1<<25),
	BIT26 = (1<<26),
	BIT27 = (1<<27),
	BIT28 = (1<<28),
	BIT29 = (1<<29),
	BIT30 = (1<<30),
	BIT31 = (1<<31)
};


typedef struct pbm_offsets
{
	uint32_t globalstatus;  /*  Global status. */
	uint32_t streambase;    /*  Offset of first stream. */
	uint32_t streamsize;    /*  Size of each stream. */
	uint32_t streamstatus;  /*  Control / Status. */
	uint32_t mem_addr;      /*  Mem hole base address. */
	uint32_t mem_size;      /*  Mem hole size. */
	uint32_t record_ptr;    /*  Record pointer. */
	uint32_t limit_ptr;     /*  Limit pointer. */
	uint32_t safetynet_cnt; /*  At limit event pointer. */
	uint32_t drop_cnt;      /*  Drop counter. */
	
} pbm_offsets_t;


/* 
 * This part controls fan and thermal information
 */
enum
{
	LM63_TEMP_LOC   = 0x00,
	LM63_TEMP_REM_H = 0x01,
	LM63_CONF   = 0x03,
	LM63_LOC_HIGH = 0x05,
	LM63_REM_HIGH = 0x07,
	
	LM63_ALERT_MASK = 0x16,
	
	LM63_PWM_RPM    = 0x4a,
	LM63_SPINUP = 0x4b,
	LM63_LOOKUP_T1  = 0x50,
	LM63_LOOKUP_P1  = 0x51,
	LM63_LOOKUP_T2  = 0x52,
	LM63_LOOKUP_P2  = 0x53,
	LM63_LOOKUP_T3  = 0x54,
	LM63_LOOKUP_P3  = 0x55,
	LM63_LOOKUP_T4  = 0x56,
	LM63_LOOKUP_P4  = 0x57,
	LM63_LOOKUP_T5  = 0x58,
	LM63_LOOKUP_P5  = 0x59,
	LM63_LOOKUP_T6  = 0x5a,
	LM63_LOOKUP_P6  = 0x5b,
	LM63_LOOKUP_T7  = 0x5c,
	LM63_LOOKUP_P7  = 0x5d,
	LM63_LOOKUP_T8  = 0x5e,
	LM63_LOOKUP_P8  = 0x5f,
	
	LM63_REM_TEMP_FILTER = 0xbf,
	
	LM63 = 0x98,
};


/* Consistent release version string. */
extern const char* const kDagReleaseVersion;


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*
 * WARNING: routines in the dagutil module are provided for convenience
 *          and to promote code reuse among the dag* tools.
 *          They are subject to change without notice.
 */


/* Miscellaneous routines. */
void dagutil_set_progname(const char* program_name);
const char* dagutil_get_progname(void);
void dagutil_set_signal_handler(void (*handler)(int));
void dagutil_set_timer_handler(void (*handler)(int), uint32_t seconds);

/* Output routines. */
void dagutil_set_verbosity(int v);
int dagutil_get_verbosity(void);
void dagutil_inc_verbosity(void);
void dagutil_dec_verbosity(void);
void dagutil_panic(const char* fmt, ...) __attribute__((noreturn, format (printf, 1, 2)));
void dagutil_error(const char* fmt, ...) __attribute__((format (printf, 1, 2)));
void dagutil_warning(const char* fmt, ...) __attribute__((format (printf, 1, 2)));
void dagutil_verbose(const char* fmt, ...) __attribute__((format (printf, 1, 2)));

/* Sleeping routines. */
void dagutil_microsleep(uint32_t usecs);
void dagutil_nanosleep(uint32_t nanoseconds);

/* I/O routines. */
void* dagutil_readfile(char *filename, char *flags, unsigned int* out_length);

/* Coprocessor detect. */
coprocessor_t dagutil_detect_coprocessor(volatile uint8_t* dagiom);
bool dagutil_coprocessor_programmed(volatile uint8_t* iom);

/* Routines that were duplicated in dagthree, dagfour, dagsix, dagseven. */
pbm_offsets_t* dagutil_get_pbm_offsets(void);
int dagutil_lockstream(int dagfd, int stream);
int dagutil_unlockstream(int dagfd, int stream);
int dagutil_lockallstreams(int dagfd);
void dagutil_unlockallstreams(int dagfd);
uint32_t dagutil_iomread(volatile uint8_t* iom, uint32_t addr);
void dagutil_iomwrite(volatile uint8_t* iom, uint32_t addr, uint32_t value);
void dagutil_pbmstatus(int dagfd, volatile uint8_t* dagiom, uint32_t pbm_base, daginf_t* daginf, int pbm_ver, unsigned int have_ipfilter);
int dagutil_pbmdefault(int dagfd, volatile uint8_t* dagiom, uint32_t pbm_base, daginf_t* daginf, int pbm_ver);
int dagutil_pbmconfigmem(int dagfd, volatile uint8_t* dagiom, uint32_t pbm_base, daginf_t* daginf, int pbm_ver, const char* cstr);
int dagutil_pbmconfigmemoverlap(int dagfd, volatile uint8_t* dagiom, uint32_t pbm_base, daginf_t* daginf, int pbm_ver);
void dagutil_reset_datapath(volatile uint8_t* iom);

/* SMB routines. */
uint32_t dagutil_smb_init(volatile uint8_t* iom, dag_reg_t result[DAG_REG_MAX_ENTRIES], unsigned int count);
void dagutil_smb_write(volatile uint8_t* iom, uint32_t smb_base, unsigned char device, unsigned char address, unsigned char value);
int dagutil_smb_read(volatile uint8_t* iom, uint32_t smb_base, unsigned char device, unsigned char address, unsigned char *value);

/* Setup temperature parameters on LM63 part.  You must ensure the card has this feature before calling this function. */
void dagutil_start_copro_fan(volatile uint8_t* iom, uint32_t smb_base);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DAGUTIL_H */ 
