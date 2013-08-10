
/*
 * Copyright (c) 2004-2005 Endace Technology Ltd, Hamilton, New Zealand.
 * All rights reserved.
 *
 * This source code is proprietary to Endace Technology Limited and no part
 * of it may be redistributed, published or disclosed except as outlined in
 * the written contract supplied with this product.
 *
 * $Id: dag_romutil.h,v 1.1.1.1 2011/08/29 05:56:42 dcham Exp $
 */

#ifndef DAG_ROMUTIL_H
#define DAG_ROMUTIL_H

/* DAG headers. */
#include "dag_platform.h"


/* ---------------------------------------------------------*/
/* CFI (Common Flash Interface) Definitions                 */

/* CFI Vendor Identifiers */
#define CFI_VENDOR_GENERIC      0x0000
#define CFI_VENDOR_INTEL_EXT    0x0001
#define CFI_VENDOR_AMD_EXT      0x0002
#define CFI_VENDOR_INTEL        0x0003
#define CFI_VENDOR_AMD          0x0004
#define CFI_VENDOR_MITSUBISHI   0x0100
#define CFI_VENDOR_MITSUBISHI_EXT   0x0101
#define CFI_VENDOR_SST          0x0102

/* CFI Device Interface Codes */
#define CFI_DEV_X8      0x0000  /* async 8-bit only */
#define CFI_DEV_X16     0x0001  /* async 16-bit only */
#define CFI_DEV_X8_X16  0x0002  /* async 8 and 16 bit via BYTE# */
#define CFI_DEV_X32     0x0003  /* async 32-bit only */
#define CFI_DEV_X16_X32 0x0004  /* async 16 and 32 bit via WORD# */

/* CFI Query Addresses */
/* 8-bit device with 8-bit accesses */
#define CFI_8_Q             0x10
#define CFI_8_R             0x11
#define CFI_8_Y             0x12
#define CFI_8_VENDOR_LO     0x13
#define CFI_8_VENDOR_HI     0x14
#define CFI_8_SIZE          0x27
#define CFI_8_DEVICE_LO     0x28
#define CFI_8_DEVICE_HI     0x29

/* 16-bit device with 8-bit accesses */
#define CFI_16_8_Q          0x20
#define CFI_16_8_R          0x22
#define CFI_16_8_Y          0x24
#define CFI_16_8_VENDOR_LO  0x26
#define CFI_16_8_VENDOR_HI  0x28
#define CFI_16_8_SIZE       0x4e
#define CFI_16_8_DEVICE_LO  0x50
#define CFI_16_8_DEVICE_HI  0x52

/* CFI Address for address sensitive devices (e.g AMD) */
/* CFI commands are written to this address */
#define CFI_BASE_ADDR           0x00
#define CFI_QUERY_ADDR          0x55    /* Needed for AMD */

/* CFI Commands */
#define CFI_CMD_WRITE_WORD      0x10    /* Write single word */
#define CFI_CMD_BLOCK_ERASE     0x20    /* Erase a single block */
#define CFI_CMD_BLOCK_Q_ERASE   0x28    /* Erase blocks using queuing */
#define CFI_CMD_WRITE_BYTE      0x40    /* Write single byte */
#define CFI_CMD_WRITE_BUFFER    0xE8    /* Write buffer */
#define CFI_CMD_CLEAR_STATUS    0x50    /* Clear status register */
#define CFI_CMD_READ_STATUS     0x70    /* Read status register */
#define CFI_CMD_QUERY           0x98    /* Enter CFI Query mode */
#define CFI_CMD_ERASE_SUSPEND   0xB0    /* Suspend the current erase operation */
#define CFI_CMD_RESUME          0xD0    /* Resume a suspended operation */
#define CFI_CMD_CONFIRM         0xD0    /* Confirm end of operation (same as resume) */
#define CFI_CMD_RESET           0xF0    /* AMD? reset to Read Array mode */
#define CFI_CMD_READ_ARRAY      0xFF    /* Enter Read Array mode (default) */

/* CFI Status Register Values */
#define CFI_STATUS_BLOCK_LOCKED     0x02
#define CFI_STATUS_PROGRAM_SUSPENDED    0x04
#define CFI_STATUS_VPP_LOW          0x08
#define CFI_STATUS_PROGRAM_ERROR    0x10
#define CFI_STATUS_ERASE_ERROR      0x20
#define CFI_STATUS_ERASE_SUSPENDED  0x40
#define CFI_STATUS_READY            0x80

/* Intel StrataFlash specifics */
#define STRATA_BUFFER_SIZE      0x20

/* Processor regions and offsets */
#define BOOT_REGION   1
#define BOOT_START    0x00000000
#define BOOT_SIZE     0x00020000  /* 64Kb */
#define KERNEL_REGION 2
#define KERNEL_START  0x00020000
#define KERNEL_SIZE   0x00080000  /* 900Kb */
#define FS_REGION     3
#define FS_START      0x00100000
#define FS_SIZE       0x00200000	/* 2Mb */

/* Register offsets from ROM base */
#define DAGROM_DATA 0x0000      /* Data register offset */
#define DAGROM_KEY  0x0004      /* Key register offset */
#define DAGROM_ADDR 0x0008      /* Address register offset */

#define DAGROM_SIZE     (rp->size)
#define DAGROM_SECTOR   (rp->sector)
#define DAGROM_BSTART   (rp->bstart)
#define DAGROM_BSIZE    (rp->bsize)
#define DAGROM_TSTART   (rp->tstart)
#define DAGROM_TSIZE    (rp->tsize)
#define DAGROM_XSTART   (rp->pstart)    /* processor boot image start */
#define DAGROM_XSIZE    (rp->psize)     /* processor boot image size */

#define DAGERRBUF_SIZE  4096
#define DAGSERIAL_SIZE  128
#define DAGSERIAL_HEADER_SIZE 8             /* magic 4 byte key, SIWD key */
#define DAGSERIAL_MAGIC_MARKER_VAL 0xABCD   /* the magic marker value */
#define DAGSERIAL_MAGIC_MARKER_SIZE 4       /* our magic byte marker */
#define DAGSERIAL_KEY_SIZE 4                /* key size */

/* Important: OFFSET's are from the start of a SECTOR */
#define DAGSERIAL_KEY_OFFSET DAGROM_SECTOR-(DAGSERIAL_SIZE)-(DAGSERIAL_KEY_SIZE)
#define DAGSERIAL_PACKAGE_SIZE DAGSERIAL_SIZE + DAGSERIAL_HEADER_SIZE
#define DAGSERIAL_PACKAGE_OFFSET DAGROM_SECTOR-(DAGSERIAL_PACKAGE_SIZE)
#define DAGSERIAL_OFFSET DAGROM_SECTOR-DAGSERIAL_SIZE

/* Can remove this macro once the code is stable */
#define MUX(X,Y) X,Y

typedef struct romtab romtab_t;


typedef unsigned int (*romreadf_t) (romtab_t *rp, uint32_t saddr);
typedef void (*romwritef_t) (romtab_t *rp, uint32_t saddr, unsigned int val);
typedef int (*erasef_t) (romtab_t *rp, uint32_t saddr, uint32_t eaddr);
typedef int (*programf_t) (romtab_t *rp, u_char* bp, uint32_t ibuflen, uint32_t saddr, uint32_t eaddr);

/* ---------------------------------------------------------*/
/* "Internal" data structures                               */

struct romtab
{
	unsigned int romid;
	uint16_t     manufacturer;
	uint16_t     device;
	uint8_t*     iom;           /* IO memory pointer */
	uint32_t     base;           /* I/O memory map base offset */
	uint32_t     rom_version;    /* ROM revision number */
	uint16_t     device_code;    /* To restrict matches to a particular DAG card. */
	char         ident[80];
	romreadf_t   romread;        /* ROM read function */
	romwritef_t  romwrite;       /* ROM write function */
	erasef_t     erase;          /* ROM erase function */
	programf_t   program;        /* ROM program function */
	unsigned int size;           /* size of the ROM in bytes */
	unsigned int sector;         /* sector size in bytes */
	unsigned int bstart;         /* Bottom half start */
	unsigned int bsize;          /* Bottom half size */
	unsigned int tstart;         /* Top half start */
	unsigned int tsize;          /* Top half size */
	unsigned int pstart;         /* Processor image start */
	unsigned int psize;          /* Processor image size */
	
};


#define INIT_ROMTAB(romid, devicecode, ident, erase, program, size, sector, bstart, bsize, tstart, tsize, xstart, xsize)  \
{                                      \
	/*romid:*/     (romid),            \
	/*manufacturer:*/   -1,            \
	/*device:*/         -1,            \
	/*iom:*/            NULL,          \
	/*base:*/           -1,            \
	/*romversion:*/    -1,             \
	/*devicecode:*/ (devicecode),      \
	/*ident:*/     ident,              \
	/*romread:*/   NULL,               \
	/*romwrite:*/  NULL,               \
	/*erase:*/      erase,             \
	/*program:*/    program,           \
	/*size :*/      (size),            \
	/*sector :*/    (sector),          \
	/*bstart :*/    (bstart),          \
	/*bsize :*/     (bsize),           \
	/*tstart :*/    (tstart),          \
	/*tsize : */    (tsize),           \
	/*pstart : */   (xstart),          \
	/*psize :*/ (xsize)                \
}


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/* Given a DAG file descriptor return its ROM descriptor */
romtab_t* dagrom_init(uint32_t dagfd, int rom_number, int hold_processor);
void dagrom_free(romtab_t* rt);

/* Erasure is by sector and starts at saddr and ends at eaddr */
int dagrom_erase(romtab_t* rp, uint32_t saddr, uint32_t eaddr);

/* ROM Read */
void dagrom_readsector(romtab_t* rp, u_char *bp, uint32_t saddr);
int dagrom_read(romtab_t* rp, u_char *bp, uint32_t saddr, uint32_t eaddr);

/* ROM Program */
int dagrom_program(romtab_t* rp, u_char* bp, uint32_t ibuflen, uint32_t saddr, uint32_t eaddr);

/* SWID functions */
int dagswid_checkkey(romtab_t* rp, uint32_t iswidkey);
int dagswid_readkey(romtab_t* rp, uint32_t* iswidkey);
int dagswid_write(romtab_t* rp, char* swidbuf, uint32_t iswidlen, uint32_t iswidkey);
int dagswid_read(romtab_t* rp, char* swidbuf, uint32_t iswidlen);
int dagswid_erase(romtab_t* rp);
int dagswid_checkkey(romtab_t* rp, uint32_t iswidkey);
int dagswid_isswid(romtab_t* rp);

/* ROM Verify */
int dagrom_verify(romtab_t* rp, u_char* ibuf, uint32_t ibuflen, u_char* obuf, uint32_t obuflen, uint32_t saddr, uint32_t eaddr);

unsigned dagrom_romreg(romtab_t* rp);
void dagrom_keywreg(romtab_t* rp, unsigned int val);

void dagrom_loadcurrent(uint32_t dagfd, romtab_t* rp);

/* Xilinx revision strings */
void dagrom_xrev(romtab_t* rp);

/* XScale functions */
void dagrom_pbihold(romtab_t* rp);
void dagrom_pbirelease(romtab_t* rp);

/* XScale functions */
uint32_t dagrom_procsize(uint32_t target_processor);
uint32_t dagrom_procstart(uint32_t target_processor);
const char* dagrom_procname(uint32_t target_processor);

/* Set fast CFI mode: 1 => on, 0 => off. */
void dagrom_set_fast_cfi_mode(int mode);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DAG_ROMUTIL_H */
