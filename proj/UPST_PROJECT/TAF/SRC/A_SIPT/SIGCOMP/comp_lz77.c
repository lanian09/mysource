#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>
#include <glib.h>

#include "udvm_state.h"
#include "sha1.h"


/* LZ77 compression */

#define COMP_BUFF_SIZE              (8192+8)
#define BYTECODE_PT                 (128+8)
#define STATIC_PT                   (256+8)
#define DYNAMIC_PT                  (512+8)
#define SIP_STATIC_PT               (2048+8)

#define LZ77CODE_LEN                47
static const guint8 lz77code[LZ77CODE_LEN] =
{
    0x1f, 0xa0, 0xa9, 0x06, 0x00, 0x00, 0x8b, 0x00, 0x0f, 0x86,
    0x03, 0x89, 0x8d, 0x89, 0x15, 0x88, 0x88, 0x00, 0x01, 0x1c,
    0x04, 0x20, 0x0d, 0x13, 0x50, 0x51, 0x22, 0x22, 0x50, 0x51,
    0x16, 0xf5, 0x23, 0x00, 0x00, 0xbf, 0xc0, 0x86, 0xa0, 0x8b,
    0x06, 0xfb, 0xe5, 0x07, 0xdf, 0xe5, 0xe6
};


static const int state_length = 8128;
static const int state_address = 64;
static const int state_instruction = 139;
static const int minimum_access_length = 6;
static const int byte_copy_left = 512;
static const int byte_copy_right = 8192;
static const int decompressed_pointer = 512;


guint8 * read_lz77_comp_state(gint cid, gint *new);


int
comp_lz77(int cid, char *msg, int msg_len, char *out_buff)
{
    guint16 stat_len = 0, dyna_len = 0, dyna_temp_len = 0;
    guint16 pt, stat_pt = 0, dyna_pt = 0, dyna_temp_pt = 0;
    guint16 sip_len = 0, sip_temp_len = 0;
    guint16 sip_pt = 0, sip_temp_pt = 0;
    int cur=0;
    guint16 len;
    int out_len = 0;
    guint16 decomp_pt;
    int new, offset=0, tbit = 0;
//	int state_id_len = 0; 

    guint8 *state_buff, *sha1buff;
    unsigned char sha1_digest_buf[20];
    sha1_context ctx;




    /* find state item */
    state_buff = read_lz77_comp_state(cid, &new);

#ifdef KILS_DEBUG
    printf("new = %d\n", new);
#endif

    /*   A SigComp message takes one of two forms depending on whether it
     *  accesses a state item at the receiving endpoint.  The two variants of
     *  a SigComp message are given in Figure 3.  (The T-bit controls the
     *  format of the returned feedback item and is defined in Section 7.1.)
     *
     *   0   1   2   3   4   5   6   7       0   1   2   3   4   5   6   7
     * +---+---+---+---+---+---+---+---+   +---+---+---+---+---+---+---+---+
     * | 1   1   1   1   1 | T |  len  |   | 1   1   1   1   1 | T |   0   |
     * +---+---+---+---+---+---+---+---+   +---+---+---+---+---+---+---+---+
     * |                               |   |                               |
     * :    returned feedback item     :   :    returned feedback item     :
     * |                               |   |                               |
     * +---+---+---+---+---+---+---+---+   +---+---+---+---+---+---+---+---+
     * |                               |   |           code_len            |
     * :   partial state identifier    :   +---+---+---+---+---+---+---+---+
     * |                               |   |   code_len    |  destination  |
     * +---+---+---+---+---+---+---+---+   +---+---+---+---+---+---+---+---+
     * |                               |   |                               |
     * :   remaining SigComp message   :   :    uploaded UDVM bytecode     :
     * |                               |   |                               |
     * +---+---+---+---+---+---+---+---+   +---+---+---+---+---+---+---+---+
     *                                     |                               |
     *                                     :   remaining SigComp message   :
     *                                     |                               |
     *                                     +---+---+---+---+---+---+---+---+
     *
     */

    if (new) {
        out_buff[offset] = 0xf8; /* state id len -> 6 */
        out_buff[offset] += tbit<<2;
        offset++;

        if (tbit) {
            /* feedback */
        }


        /* bytecode */
        out_buff[offset] = LZ77CODE_LEN >> 4;
        offset++;
        out_buff[offset] = (LZ77CODE_LEN & 0xf) << 4;
        out_buff[offset] += 0x01;
        offset++;
        memcpy(&out_buff[offset], lz77code, LZ77CODE_LEN);
        offset+=LZ77CODE_LEN;

    }
    else {
        out_buff[offset] = 0xf9;
        out_buff[offset] += tbit<<2;
        offset++;

        if (tbit) {
            /* feedback */
        }

        /* generate partial state identifier */
        sha1buff = g_malloc(8+state_length);
        memset(&sha1buff[0], 0x00, 8+state_length);
        memcpy(&sha1buff[0], &state_buff[0], 8);
        memcpy(&sha1buff[8], &state_buff[8+state_address], state_length);

        memset(sha1_digest_buf, 0x00, sizeof(sha1_digest_buf));
        sha1_starts(&ctx);
        sha1_update(&ctx, (guint8 *)sha1buff, 8+state_length);
        sha1_finish(&ctx, sha1_digest_buf);

#ifdef KILS_DEBUG
        printf("Partial State Identifier ============================\n");
        debugdump(sha1_digest_buf, minimum_access_length);
#endif


        /* copy partial state identifier */
        memcpy(&out_buff[offset], &sha1_digest_buf[0], minimum_access_length);
        offset+=minimum_access_length;

    }








#if 0
    decomp_pt = state_buff[8+68]<<8;
    decomp_pt += state_buff[8+69];
#else
    decomp_pt = decompressed_pointer;
#endif


    for (cur=0; cur<msg_len; cur++) {

#ifdef KILS_DEBUG
        printf("\n\n");
#endif

        /* 
         * search from static dictionary 
         */
        stat_len = 0;
        for (stat_pt = 0; stat_pt<256; stat_pt++) {
            if (msg[cur] == state_buff[STATIC_PT+stat_pt]) {
                for (stat_len = 1;
                     msg[cur+stat_len] == state_buff[STATIC_PT+stat_pt+stat_len];
                     stat_len++) {
                    if (msg[cur+stat_len] == 0x00) break;
                }
                break;
            }
        }

#ifdef KILS_DEBUG
        if (stat_len)
            printf("%c found in stat_dic: 0x%02x (%d)\n", msg[cur], stat_pt, stat_len);
#endif




        /* 
         * search from dynamic dictionary 
         */
        dyna_temp_len = 0;
        dyna_len = 0;
        for (dyna_temp_pt = 0; state_buff[DYNAMIC_PT+dyna_temp_pt]; dyna_temp_pt++) {
            if (msg[cur] == state_buff[DYNAMIC_PT+dyna_temp_pt]) {
                for (dyna_temp_len = 1;
                     msg[cur+dyna_temp_len] == state_buff[DYNAMIC_PT+dyna_temp_pt+dyna_temp_len];
                     dyna_temp_len++) {
                    if (msg[cur+dyna_temp_len] == 0x00) break;
                }
                if (dyna_len < dyna_temp_len) {
                    dyna_len = dyna_temp_len;
                    dyna_pt = dyna_temp_pt;
                }
            }
        }

#ifdef KILS_DEBUG
        if (dyna_len)
            printf("%c found in dyna_dic: 0x%02x (%d)\n", msg[cur], dyna_pt, dyna_len);
#endif




        /* 
         * search from SIP/SDP dictionary 
         */ 
        sip_temp_len = 0;
        sip_len = 0;
        for (sip_temp_pt = 0; state_buff[SIP_STATIC_PT+sip_temp_pt]; sip_temp_pt++) {
            if (msg[cur] == state_buff[SIP_STATIC_PT+sip_temp_pt]) {
                for (sip_temp_len = 1;
                     msg[cur+sip_temp_len] == state_buff[SIP_STATIC_PT+sip_temp_pt+sip_temp_len];
                     sip_temp_len++) {
                    if (msg[cur+sip_temp_len] == 0x00) break;
                }
                if (sip_len < sip_temp_len) {
                    sip_len = sip_temp_len;
                    sip_pt = sip_temp_pt;
                }
            }
        }
 
#ifdef KILS_DEBUG
        if (sip_len)
            printf("%c found in sip_dic: 0x%02x (%d)\n", msg[cur], sip_pt, sip_len);
#endif



        /* compressing */


        if (dyna_len > stat_len) {
            if (sip_len > dyna_len) {
                len = sip_len;
                pt = sip_pt + 0x800;
            }
            else {
                len = dyna_len;
                pt = dyna_pt + 0x200;
            }
        }
        else {
            if (sip_len > stat_len) {
                len = sip_len;
                pt = sip_pt + 0x800;
            }
            else {
                len = stat_len;
                pt = stat_pt + 0x100;
            }
        }

#ifdef KILS_DEBUG
        printf("Result 0x%04x (%d)\n", pt, len);
#endif



        /* update dynamic dictionary */
        memcpy(&state_buff[8+decomp_pt], &msg[cur], len);
        decomp_pt += len;



        /* next message */
        cur += (len-1);


        /* generate SigComp message */
        len = ntohs(len);
        pt = ntohs(pt);
        memcpy(&out_buff[offset+out_len], &pt, 2);
        memcpy(&out_buff[offset+out_len+2], &len, 2);
        out_len += 4;
    }

    /* update decompressed_pointer */
    state_buff[8+68] = (decomp_pt & 0xff00) >> 8;
    state_buff[8+69] = (decomp_pt & 0x00ff);


    return (offset+out_len);
}



guint8 *
read_lz77_comp_state(gint cid, gint *new)
{
    struct compart_item *pcompart;
    guint16 i;
    gint *compart_id;
   


    compart_id = g_malloc0(sizeof(struct compart_item));
    *compart_id = cid;


    pcompart = (struct compart_item *)comp_state_access(cid);
    if (pcompart) {
        *new = FALSE;

#ifdef KILS_DEBUG
        printf("found for compartment ID(%d)\n", cid);
#endif
    }
    else {
        *new = TRUE;

#ifdef KILS_DEBUG
        printf("not found, new compartment ID(%d)\n", cid);
#endif

        /*
         *  format of the state item for compressor
         *
         *   0   1   2   3   4   5   6   7  
         * +---+---+---+---+---+---+---+---+
         * | st_len|st_addr|st_ins |min_acl|
         * +---+---+---+---+---+---+---+---+ 
         * |  len  |  left | right |  ptr  |
         * +---+---+---+---+---+---+---+---+ 
         * |                               |
         * :       state item              |
         * |                               |
         * +---+---+---+---+---+---+---+---+ 
         * |                               |
         * :       bytecode                |
         * |                               |
         * +---+---+---+---+---+---+---+---+ 
         * |                               |
         * :       static dictionary       :
         * |                               |
         * +---+---+---+---+---+---+---+---+ 
         * |                               | 
         * :       circular buffer         :
         * |                               |
         * +---+---+---+---+---+---+---+---+ 
         *
         */

        pcompart = g_malloc0(sizeof(struct compart_item));

        pcompart->compartment_id = cid;
        pcompart->comp_buff = g_malloc0(COMP_BUFF_SIZE);

        /* memset - state_length (8128)*/
        pcompart->comp_buff[0] = state_length >> 8;              // 0x1f   
        pcompart->comp_buff[1] = state_length & 0xff;            // 0xc0;

        /* memset - state_address (64)*/
        pcompart->comp_buff[2] = state_address >> 8;             // 0x00;   
        pcompart->comp_buff[3] = state_address & 0xff;           // 0x40;

        /* memset - state_instruction (139)*/
        pcompart->comp_buff[4] = state_instruction >> 8;         // 0x00;   
        pcompart->comp_buff[5] = state_instruction & 0xff;       // 0x8b;

        /* memset - minimum_access_length (6)*/
        pcompart->comp_buff[6] = minimum_access_length >> 8;     // 0x00;   
        pcompart->comp_buff[7] = minimum_access_length & 0xff;   // 0x06;

        /* memset - byte_copy_left */
        pcompart->comp_buff[8+64] = byte_copy_left >> 8;         // 0x02;
        pcompart->comp_buff[8+65] = byte_copy_left & 0xff;       // 0x00;

        /* memset - byte_copy_right */
        pcompart->comp_buff[8+66] = byte_copy_right >> 8;        // 0x20;
        pcompart->comp_buff[8+67] = byte_copy_right & 0xff;      // 0x00;

        /* memset - decompressed_pointer */
        pcompart->comp_buff[8+68] = decompressed_pointer >> 8;   // 0x02;
        pcompart->comp_buff[8+69] = decompressed_pointer & 0xff; // 0x00;

        /* memset - bytecode */
        memcpy(&pcompart->comp_buff[BYTECODE_PT], lz77code, LZ77CODE_LEN);


        /* memset - static-dictionary */
        for(i=0; i<256; i++) {
            pcompart->comp_buff[STATIC_PT+i] = i;
        }

#if 1
        /* memset - SIP/SDP static dictionary */
        for (i=0; i<0x12e4; i++) {
            pcompart->comp_buff[SIP_STATIC_PT+i] = sip_sdp_static_dictionary_for_sigcomp[i];
        }
#endif


        comp_state_create(cid, (guint8*)pcompart);
    }

    return pcompart->comp_buff;
}
