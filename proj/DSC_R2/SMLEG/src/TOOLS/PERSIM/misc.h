#ifndef _MISC_H
#define _MISC_H

#ifndef __KERNEL__
#define _LVM_H_INCLUDE
#include <stdio.h>
#endif
#include <linux/list.h>

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdarg.h>
#include <netinet/in.h>
#include <md5.h>

#define MAX_STATUS	65
#define CLR_NRML "\033[0m"
#define CLR_BOLD "\033[1m"
#define CLR_ULND "\033[4m"
#define CLR_BLNK "\033[5m"
#define CLR_INVR "\033[7m"
#define CLR_BLCK "\033[30m"
#define CLR_RED  "\033[31m"
#define CLR_GREN "\033[32m"
#define CLR_YLLW "\033[33m"
#define CLR_BLUE "\033[34m"
#define CLR_PRPL "\033[35m"
#define CLR_AQUA "\033[36m"

#define PRN_OK 		printf(CLR_AQUA "[  " CLR_GREN "OK" CLR_AQUA "  ]" CLR_NRML "\n")
#define PRN_FAIL 	printf(CLR_AQUA "[ " CLR_RED "FAIL" CLR_AQUA " ]" CLR_NRML "\n")
#define PRN_PASS 	printf(CLR_AQUA "[ " CLR_YLLW "PASS" CLR_AQUA " ]" CLR_NRML "\n")
#define PRN_SKIP 	printf(CLR_AQUA "[ " CLR_RED "SKIP" CLR_AQUA " ]" CLR_NRML "\n")

#define STATUS(t) do { \
    int i;													\
    printf(CLR_GREN "* " CLR_NRML);							\
    for(i=0;i<MAX_STATUS;i++) {								\
        if(i<strlen(t)) putchar(t[i]); else putchar(0x20);	\
    }														\
    printf("  "); fflush(stdout);							\
} while(0);

/* Macroes for conversion between host and network byte order */

#define hton8(x)  (x)
#define ntoh8(x)  (x)
#define hton16(x) htons(x)
#define ntoh16(x) ntohs(x)
#define hton32(x) htonl(x)
#define ntoh32(x) ntohl(x)

static __inline uint64_t
hton64(uint64_t q)
{
        register uint32_t u, l;
        u = q >> 32;
        l = (uint32_t) q;

        return htonl(u) | ((uint64_t)htonl(l) << 32);
}
#define ntoh64(_x)      hton64(_x)

#define IPADDR(d)    (d&0xff),((d>>8)&0xff),((d>>16)&0xff),((d>>24)&0xff)

#endif
