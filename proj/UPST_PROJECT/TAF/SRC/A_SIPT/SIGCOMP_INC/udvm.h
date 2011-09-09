/* udvm.h
 * Routines making up the Univerasl Decompressor Virtual Machine (UDVM) used for
 * Signaling Compression (SigComp) dissection.
 * Copyright 2004, Anders Broman <anders.broman@ericsson.com>
 *
 * $Id: udvm.h,v 1.1.1.1 2011/08/29 05:56:42 dcham Exp $
 *
 * Ethereal - Network traffic analyzer
 * By Gerald Combs <gerald@ethereal.com>
 * Copyright 1998 Gerald Combs
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 * References:
 * http://www.ietf.org/rfc/rfc3320.txt?number=3320
 * http://www.ietf.org/rfc/rfc3321.txt?number=3321
 * Useful links :
 * http://www.ietf.org/internet-drafts/draft-ietf-rohc-sigcomp-impl-guide-02.txt
 * http://www.ietf.org/internet-drafts/draft-ietf-rohc-sigcomp-sip-01.txt
 */

#ifndef SIGCOMP_UDVM_H
#define SIGCOMP_UDVM_H

#define UDVM_MEMORY_SIZE						65536


#if 0
extern tvbuff_t* decompress_sigcomp_message(tvbuff_t *bytecode_tvb, tvbuff_t *message_tvb, packet_info *pinfo,
						   proto_tree *tree, gint destination, gint print_flags, gint hf_id);
#endif

extern int decompress_sigcomp_message(char *bytecode_tvb, int btlen, char *message_tvb, int mtlen, int destination, int hf_id, unsigned char *in_buff, unsigned char *out_buff, int *out_len);


extern void sigcomp_init_protocol(void);
extern int dissect_sigcomp(char *tvb, int tlen, char *out_buff, int *out_len);

/* example: extern const value_string q931_cause_location_vals[]; */
#endif 
/* SIGCOMP_UDVM_H */
