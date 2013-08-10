/*
 * Copyright (c) 2005 Endace Technology Ltd, Hamilton, New Zealand.
 * All rights reserved.
 *
 * This source code is proprietary to Endace Technology Limited and no part
 * of it may be redistributed, published or disclosed except as outlined
 * in the written contract supplied with this product.
 *
 * $Id: dagname.h,v 1.1.1.1 2011/08/29 05:56:42 dcham Exp $
 */

#ifndef DAGNAME_H
#define DAGNAME_H

#ifndef _WIN32
# include	<inttypes.h>
#else
# include	<wintypedefs.h>
#endif /* _WIN32 */

char **dag_xrev_parse(uint8_t *xrev, uint32_t xlen, char *argv[5]);

/*
 * This routine will partially destroy the contents of xrev by
 * writing '\0' markers into the buffer and letting argv point
 * into the buffer.
 */
char *dag_xrev_name(uint8_t *xrev, uint32_t xlen);


#endif /* DAGNAME_H */
