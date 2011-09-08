#include <stdio.h>
#include <string.h>
#include <glib.h>

#include "sigcomp.h"
#include "udvm.h"
#include "udvm_state.h"

#define IPADDR(n)     (n>>24)&0xff, (n>>16)&0xff, (n>>8)&0xff, n&0xff


int
comp_sig(int cid, char *in, int ilen, char *out)
{
    int olen;
	char str[32];

	memcpy(str, in, 31);
	str[31] = 0;


    if (ilen <= 0) return -1;


#ifdef KILS_DEBUG
#endif
printf("\n\nCOMP >>>>\n");
	printf("[[%s]]\n", str);
//    olen = comp_lzss(cid, in, ilen, out);
#ifdef KILS_DEBUG
#endif
	udvm_state_print();
printf("### %d.%d.%d.%d (%d)\n", IPADDR(cid), cid);
    return olen;
}



/*
 * Return value:     > 0, success
 *                   = 0, plain message
 *                   = -1, TCP case, no action
 *                   else, error
 *
 */
int
decomp_sig(char *in, int ilen, char *out, int *olen)
{
	int local_id;
	char str[32];
    

    if (ilen <= 0) return -2;

	/* check TCP packet 
       if not, return -1 */


    *olen = 0;
	local_id = dissect_sigcomp(in, ilen, out, olen);

	memcpy(str, out, 31);
	str[31] = 0;

#ifdef KILS_DEBUG
printf("[[%s]]\n", str);
#endif

    return local_id;
}

int
decomp_confirm(int lid, int cid)
{
    udvm_state_create(lid, cid);
#ifdef KILS_DEBUG
	udvm_state_print();
printf("### %d.%d.%d.%d (%d)\n", IPADDR(cid), cid);
#endif
	return 0;
}

/*
 * Retrun value  :0 uncompressed packet
 *                1 format-1 (include state identifier)
 *                2 format-2 (include bytecode)
 */
int
sigcomp_type(char *buf)
{
    unsigned char c = *buf;
	char str[32];

#ifdef KILS_DEBUG
#endif
printf("\n\nDECOMP >>> \n");

    if ((c & 0xf8)==0xf8) {
        if (c & 0x03) {
            return 1;
        }
        else {
            return 2;
        }
    }

	
	memcpy(str, buf, 31);
	str[31] = 0;
#ifdef KILS_DEBUG
#endif
printf("[[%s]]\n", str);

    return 0;
}


/*
    flag: 1:compression
        : 0:decompression
*/
int
sigcomp_release(int cid, int flag)
{
    udvm_state_release(cid, flag);
	return 0;
}
