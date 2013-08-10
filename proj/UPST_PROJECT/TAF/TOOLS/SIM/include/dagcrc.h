/*
 * Copyright (c) 2004-2005 Endace Technology Ltd, Hamilton, New Zealand.
 * All rights reserved.
 *
 * This source code is proprietary to Endace Technology Limited and no part
 * of it may be redistributed, published or disclosed except as outlined in
 * the written contract supplied with this product.
 *
 * $Id: dagcrc.h,v 1.1.1.1 2011/08/29 05:56:42 dcham Exp $
 */

#ifndef DAGCRC_H
#define DAGCRC_H

#ifndef _WIN32
/* C Standard Library headers. */
#include <inttypes.h>
#else /* _WIN32 */
#include <wintypedefs.h>
#endif /* _WIN32 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/* Ethernet 32-bit CRC. */
void dagcrc_make_ethernet_table_r(void);
uint32_t dagcrc_ethernet_crc32_r(uint32_t crc, const uint8_t* buf, int len);


/* PPP 16 and 32-bit CRCs. */
#define PPPINITFCS16 0xffff  /* Initial FCS value */
#define PPPGOODFCS16 0xf0b8  /* Good final FCS value */

#define PPPINITFCS32 0xffffffff   /* Initial FCS value */
#define PPPGOODFCS32 0xdebb20e3   /* Good final FCS value */

void dagcrc_make_ppp_fcs16_table(void);
uint16_t dagcrc_ppp_fcs16(uint16_t fcs, const uint8_t* cp, int len);
uint32_t dagcrc_ppp_fcs32(uint32_t fcs, const uint8_t* cp, int len);


/* ATM AAL5 32-bit CRC. */
#define CRC_INIT   0xffffffffL
#define CRC_PASS   0xC704DD7BL

void dagcrc_make_aal5_table(void);
uint32_t dagcrc_aal5_crc(uint32_t crc_accum, const char *data_blk_ptr, int data_blk_size);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DAGCRC_H */
