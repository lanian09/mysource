#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>
#include <glib.h>

#include "udvm_state.h"
#include "sha1.h"

#ifdef _WIN32
#include "pint.h"
#endif


/* LZSS compression */

#define COMP_BUFF_SIZE              (8192+8)
#define BYTECODE_PT                 (128+8)
#if 0
#define DYNAMIC_PT                  (210+8)
#define SIP_STATIC_PT               (1024+8)
#endif
#define DYNAMIC_PT                  (256+8)
#define SIP_STATIC_PT               (884+8)
#define SIP_STATIC_LEN              0x0d8c      // 3468

#define LZSSCODE_LEN                82
static const guint8 lzsscode[LZSSCODE_LEN] =
{
// 128
    0x1f, 0xa0, 0xcc, 0x06, 0x00, 0xad, 0x8c, 0xa3, 0x74, 0x00, // 10: STATE-ACCESS (static_id, 6, 0, 3468, 884, 0)
// 138
    0x0f, 0x86, 0x04, 0x88, 0xb1, 0x00, 0x00, 0x88,             // 8: MULTILOAD (64, 4, 256, 4352, 0, 256)
// 146
    0x1e, 0x20, 0x31, 0x02, 0x09, 0x00, 0xa0, 0xff, 0x8e, 0x04, 0x8c, 0xbf, 0xff, 0x01, // 14: INPUT-HUFFMAN
// 160
    0x17, 0x50, 0x8d, 0x0f, 0x23, 0x06,                         // 6: COMPARE
// 166
    0x22, 0x21, 0x01,                                           // 3: OUTPUT
// 169
    0x13, 0x21, 0x01, 0x23,                                     // 4: COPY-LITERAL
// 173
    0x16, 0xe5,                                                 // 2: JUMP
// 175
    0x1d, 0x04, 0x22, 0xe8,                                     // 4: INPUT-BITS
// 179
    0x06, 0x11, 0x03,                                           // 3: ADD
// 182
    0x0e, 0x24, 0x63,                                           // 3: LOAD
// 185
    0x14, 0x50, 0x51, 0x23,                                     // 4: COPY-OFFSET
// 189
    0x22, 0x52, 0x51,                                           // 3: OUTPUT
// 192
    0x16, 0x9f, 0xd2,                                           // 3: JUMP
// 195
    0x23, 0x00, 0x00, 0xb0, 0xc0, 0x86, 0xa0, 0x89, 0x06,       // 9: END-MESSAGE
// 204
    0xfb, 0xe5, 0x07, 0xdf, 0xe5, 0xe6                          // 6: static id
};

static const int state_length = 4288;
static const int state_address = 64;
static const int state_instruction = 137;
static const int minimum_access_length = 6;
static const int byte_copy_left = 256;
static const int byte_copy_right = 4352;
static const int input_bit_order = 0;
static const int decompressed_pointer = 256;


guint8 * read_lzss_comp_state(gint cid, gint *new);
int init_lzss_comp_state(guint8 *comp_buff);
int reinit_sip_static_dictionary(guint8 *comp_buff);


int
comp_lzss(int cid, char *msg, int msg_len, char *out_buff)
{
    guint16 dyna_len = 0, dyna_temp_len = 0;
    guint16 pt, dyna_pt = 0, dyna_temp_pt = 0;
    guint16 sip_len = 0, sip_temp_len = 0;
    guint16 sip_pt = 0, sip_temp_pt = 0;
    int cur=0;
    int len, comp_len, out_len = 0;
    guint16 decomp_pt;
    int new, offset=0, tbit = 0;
//  int state_id_len = 0;
    guint len_bits=0, offset_bits=0, remain_bits=0, comp_bits=0, msg_bits=0, endian_bits=0;
    gint remain_len=0, lzss_offset=0;
   

    guint8 *comp_buff, *sha1buff;
    unsigned char sha1_digest_buf[20];
    sha1_context ctx;

    int print_flag = 0;




    /* find state item */
    comp_buff = read_lzss_comp_state(cid, &new);

#ifdef KILS_DEBUG
    if (print_flag)
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
        out_buff[offset] = LZSSCODE_LEN >> 4;
        offset++;
        out_buff[offset] = (LZSSCODE_LEN & 0xf) << 4;
        out_buff[offset] += 0x01;
        offset++;
        memcpy(&out_buff[offset], lzsscode, LZSSCODE_LEN);
        offset+=LZSSCODE_LEN;

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
        memcpy(&sha1buff[0], &comp_buff[0], 8);
        memcpy(&sha1buff[8], &comp_buff[8+state_address], state_length);

        memset(sha1_digest_buf, 0x00, sizeof(sha1_digest_buf));
        sha1_starts(&ctx);
        sha1_update(&ctx, (guint8 *)sha1buff, 8+state_length);
        sha1_finish(&ctx, sha1_digest_buf);

        if (print_flag) {
#ifdef KILS_DEBUG
            printf("Partial State Identifier ============================\n");
            debugdump(sha1_digest_buf, minimum_access_length);
            debugdump((guint8 *)sha1buff, 8+state_length);
#endif
        }
		


        /* copy partial state identifier */
        memcpy(&out_buff[offset], &sha1_digest_buf[0], minimum_access_length);
        offset+=minimum_access_length;

        /* reinit SIP static dictionary */
        reinit_sip_static_dictionary(comp_buff);
    }



    




#if 0
    decomp_pt = comp_buff[8+70]<<8;
    decomp_pt += comp_buff[8+71];
#else
    decomp_pt = decompressed_pointer;
#endif


    for (cur=0; cur<msg_len; cur++) {

        /* 
         * search from dynamic dictionary 
         */
        dyna_temp_len = 0;
        dyna_len = 0;
        for (dyna_temp_pt = DYNAMIC_PT; comp_buff[dyna_temp_pt]; dyna_temp_pt++) {
            if (msg[cur] == comp_buff[dyna_temp_pt]) {
                for (dyna_temp_len = 1;
                     msg[cur+dyna_temp_len] == comp_buff[dyna_temp_pt+dyna_temp_len];
                     dyna_temp_len++) {
                    if (msg[cur+dyna_temp_len] == 0x00) break;
                    if (dyna_temp_len == 18) break;     /* length must be 3 ~ 18 */
                }
                if (dyna_len < dyna_temp_len) {
                    dyna_len = dyna_temp_len;
                    dyna_pt = dyna_temp_pt;
                }
            }
        }

#ifdef KILS_DEBUG
        if (0)
            if (dyna_len)
                printf("[%c:0x%x] found in dyna_dic: %d (%d)\n", msg[cur], msg[cur], (dyna_pt-DYNAMIC_PT), dyna_len);
#endif





        /* 
         * search from SIP/SDP dictionary 
         */ 
        sip_temp_len = 0;
        sip_len = 0;
        for (sip_temp_pt = SIP_STATIC_PT; comp_buff[sip_temp_pt]; sip_temp_pt++) {
            if (msg[cur] == comp_buff[sip_temp_pt]) {
                for (sip_temp_len = 1;
                     msg[cur+sip_temp_len] == comp_buff[sip_temp_pt+sip_temp_len];
                     sip_temp_len++) {
                    if (msg[cur+sip_temp_len] == 0x00) break;
                    if (sip_temp_len == 18) break;     /* length must be 3 ~ 18 */
                }
                if (sip_len < sip_temp_len) {
                    sip_len = sip_temp_len;
                    sip_pt = sip_temp_pt;
                }
            }
        }

#ifdef KILS_DEBUG
        if (0)
            if (sip_len)
                printf("[%c:0x%x] found in sip_dic: 0x%02x (%d)\n", msg[cur], msg[cur], (sip_pt-SIP_STATIC_PT), sip_len);
#endif



        /* compressing */



        /* literal case */
        if (dyna_len < 3 && sip_len < 3) {
#ifdef KILS_DEBUG
            if (0)
                printf("literal: 0x%x ASCII[%c]\n", msg[cur], msg[cur]);
#endif
            msg_bits = msg[cur];
            len = 1;
        }
        /* offset/length pair case */
        else {
            if (sip_len > dyna_len) {
                len = sip_len;
                pt = sip_pt-8;
            }
            else {
                len = dyna_len;
                pt = dyna_pt-8;
            }

            if (decomp_pt > pt)
                lzss_offset = decomp_pt - pt;
            else 
                lzss_offset = (byte_copy_right - pt) + (decomp_pt - byte_copy_left);


#ifdef KILS_DEBUG
            if (0)
                printf("offset/length: cur(%u)-found(%u) : (%d - %d)\n", decomp_pt, pt, lzss_offset, len);
#endif
        }







        /* update dynamic dictionary */
        memcpy(&comp_buff[8+decomp_pt], &msg[cur], len);
        decomp_pt += len;



        /* next message */
        cur += (len-1);



        /* 
         * generate SigComp message 
         */

        //lzss_get_bits(&comp_bits, &remain_bits, &remain_len);

        if (remain_len) {
            if (len>=3) {
                /* offset/length */
                len_bits = len - 3;
                offset_bits = lzss_offset - 1;

                comp_bits = 0x80000000;         /* bit on */
                comp_bits = comp_bits | ((offset_bits << 20) >> 1);
                comp_bits = comp_bits | ((len_bits << 28) >> 13);

                comp_bits = comp_bits >> remain_len;
                comp_bits = comp_bits | remain_bits;

                comp_len    = (remain_len+17) / 8;
                remain_len  = (remain_len+17) % 8;
                remain_bits = comp_bits << (comp_len*8);
            }
            else {
                /* literal */
                comp_bits = 0x00000000;         /* bit off */
                comp_bits = comp_bits | (msg_bits << 23);

                comp_bits = comp_bits >> remain_len;
                comp_bits = comp_bits | remain_bits;

                comp_len    = (remain_len+9) / 8;
                remain_len  = (remain_len+9) % 8;
                remain_bits = comp_bits << (comp_len*8);
            }
        }
        else {
            if (len>=3) {    
                /* offset/length */
                len_bits = len - 3;
                offset_bits = lzss_offset - 1;

                comp_bits = 0x80000000;         /* bit on */
                comp_bits = comp_bits | ((offset_bits << 20) >> 1);
                comp_bits = comp_bits | ((len_bits << 28) >> 13);

                comp_len    = (remain_len+17) / 8;
                remain_len  = (remain_len+17) % 8;
                remain_bits = comp_bits << (comp_len*8);
            }
            else {          
                /* literal */
                comp_bits = 0x00000000;         /* bit off */
                comp_bits = comp_bits | (msg_bits << 23);

                comp_len    = (remain_len+9) / 8;
                remain_len  = (remain_len+9) % 8;
                remain_bits = comp_bits << (comp_len*8);
            }
        }

#ifdef KILS_DEBUG
        if (0)
            printf("remain_len(%d) comp_len(%d)\n", remain_len, comp_len);
#endif


#ifdef _WIN32
        endian_bits = pntohl(&comp_bits);
#else
        endian_bits = comp_bits;
#endif
        memcpy(&out_buff[offset+out_len], &endian_bits, comp_len);
        out_len += comp_len;
    }

    /* update decompressed_pointer */
    comp_buff[8+70] = (decomp_pt & 0xff00) >> 8;
    comp_buff[8+71] = (decomp_pt & 0x00ff);



    if (remain_len) {
#ifdef _WIN32
        endian_bits = pntohl(&remain_bits);
#else
        endian_bits = remain_bits;
#endif
        memcpy(&out_buff[offset+out_len], &endian_bits, 1);
        out_len += 1;
    }


#if 1
	/* clear dynamic dictionary */
	memset(&comp_buff[DYNAMIC_PT], 0x00, 4096);
	memset(&comp_buff[8+70],       0x00, 2);
#endif


    return (offset+out_len);
}



int 
init_lzss_comp_state(guint8 *comp_buff)
{
    int i;

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
     * :       circular buffer         :
     * |                               |
     * |    (dynamic dictionary)       |
     * |                               |
     * |    (SIP static dictionary)    |
     * |                               |
     * +---+---+---+---+---+---+---+---+ 
     *
     */

    /* memset - state_length (8128)*/
    comp_buff[0] = state_length >> 8;              // 0x1f   
    comp_buff[1] = state_length & 0xff;            // 0xc0;

    /* memset - state_address (64)*/
    comp_buff[2] = state_address >> 8;             // 0x00;   
    comp_buff[3] = state_address & 0xff;           // 0x40;

    /* memset - state_instruction (139)*/
    comp_buff[4] = state_instruction >> 8;         // 0x00;   
    comp_buff[5] = state_instruction & 0xff;       // 0x8b;

    /* memset - minimum_access_length (6)*/
    comp_buff[6] = minimum_access_length >> 8;     // 0x00;   
    comp_buff[7] = minimum_access_length & 0xff;   // 0x06;

    /* memset - byte_copy_left */
    comp_buff[8+64] = byte_copy_left >> 8;         // 0x02;
    comp_buff[8+65] = byte_copy_left & 0xff;       // 0x00;

    /* memset - byte_copy_right */
    comp_buff[8+66] = byte_copy_right >> 8;        // 0x20;
    comp_buff[8+67] = byte_copy_right & 0xff;      // 0x00;

    /* memset - input_bit_order */
    comp_buff[8+68] = input_bit_order >> 8;        // 0x00;
    comp_buff[8+69] = input_bit_order & 0xff;      // 0x00;

    /* memset - decompressed_pointer */
    comp_buff[8+70] = decompressed_pointer >> 8;   // 0x02;
    comp_buff[8+71] = decompressed_pointer & 0xff; // 0x00;

    /* memset - bytecode */
    memcpy(&comp_buff[BYTECODE_PT], lzsscode, LZSSCODE_LEN);


    /* memset - SIP/SDP static dictionary */
    //for (i=0; i<0x12e4; i++) {
    for (i=0; i< SIP_STATIC_LEN; i++) {
        comp_buff[SIP_STATIC_PT+i] = sip_sdp_static_dictionary_for_sigcomp[i];
    }

    return 1;
}

int
reinit_sip_static_dictionary(guint8* comp_buff)
{
    int i;

    /* memset - SIP/SDP static dictionary */
    //for (i=0; i<0x12e4; i++) {
    for (i=0; i< SIP_STATIC_LEN; i++) {
        comp_buff[SIP_STATIC_PT+i] = sip_sdp_static_dictionary_for_sigcomp[i];
    }
	
	return 0;
}

guint8 *
read_lzss_comp_state(gint cid, gint *new)
{
    struct compart_item *pcompart;
//    guint16 i;
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


        pcompart = g_malloc0(sizeof(struct compart_item));

        pcompart->compartment_id = cid;
        pcompart->comp_buff = g_malloc0(COMP_BUFF_SIZE);

        /* init comp_buff */
        init_lzss_comp_state(pcompart->comp_buff);

        comp_state_create(cid, (guint8*)pcompart);
    }

    return pcompart->comp_buff;
}
