/* udvm_state.h
 * Routines making up the State handler of the Univerasl Decompressor Virtual Machine (UDVM) 
 * used for Signaling Compression (SigComp) dissection.
 * Copyright 2004, Anders Broman <anders.broman@ericsson.com>
 *
 * $Id: udvm_state.h,v 1.1.1.1 2011/08/29 05:56:42 dcham Exp $
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
 * http://www.ietf.org/internet-drafts/draft-ietf-rohc-sigcomp-impl-guide-03.txt
 * http://www.ietf.org/internet-drafts/draft-ietf-rohc-sigcomp-sip-01.txt
 */

#ifndef SIGCOMP_UDVM_STATE_H
#define SIGCOMP_UDVM_STATE_H

struct state_item {
    gint    compartment_id;
	gchar   partial_state_str[64];
    guchar  *state_buff;
};

struct compart_item {
	gint    compartment_id;
	guchar  *comp_buff;
	guchar  *feedback_buff;
};

struct local_item {
	gint    local_id;
	gchar   partial_state_str[64];
	guchar  *state_buff;
	guchar  *feedback_buff;
};


extern const guint16 sip_sdp_state_length;
extern const guint8 sip_sdp_state_identifier[];
extern const guint8 sip_sdp_static_dictionary_for_sigcomp[];


extern void sigcomp_init_udvm(void);
extern int udvm_state_pre_create(guint8*, guint8*, guint16);
extern void udvm_state_create(gint, gint);
extern void udvm_state_free(guint8 buff[],guint16 p_id_start,guint16 p_id_length);
extern int udvm_state_access(char *, guint8 *, guint16, guint16, guint16, guint16 *, guint16 *, guint16, gboolean, gint);
extern int udvm_state_access2(guint8*, gchar *, guint16, guint16 *, guint16 *);
extern void udvm_state_print(void);
extern guint8* comp_state_access(gint);
extern void comp_state_create(gint, guint8*);
extern void udvm_state_release(gint compart_id, gint flag);

#endif 
/* SIGCOMP_UDVM_STATE_H */
