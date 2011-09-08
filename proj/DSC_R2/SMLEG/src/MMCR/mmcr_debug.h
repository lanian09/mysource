#ifndef MMCR_DEBUG_H
#define MMCR_DEBUG_H

#ifdef	MMDB_DEBUG

#define DEBUG_PRINT(a)   fprintf(stderr, "DEBUG[%s:%d] %s\n", __FILE__, __LINE__, a)
#define DEBUG_PRINT_A(a)   fprintf(stderr, "DEBUG[%s:%d] %s %s\n", __FILE__, __LINE__, #a, a)
#define DEBUG_PRINT_I(a)   fprintf(stderr, "DEBUG[%s:%d] %s %d\n", __FILE__, __LINE__, #a, a)
#define DEBUG_IP        fprintf(stderr, "DEBUG-IP[%s:%d] free(%d), root(%d), count(%d)\n", __FILE__, __LINE__, \
                        destip_tbl->free, destip_tbl->root, destip_tbl->uiCount)
#define DEBUG_PORT        fprintf(stderr, "DEBUG-DEST[%s:%d] free(%d), root(%d), count(%d)\n", __FILE__, __LINE__, \
                        destport_tbl->free, destport_tbl->root, destport_tbl->uiCount)
#define DEBUG_MEM(a, b) \
{ \
    int i; \
    char    tbuf[b]; \
 \
    memcpy(tbuf, &(a), (b)); \
    fprintf(stderr, "DEBUG-MEM[%s:%d]%s -> (",  __FILE__, __LINE__, #a); \
    for(i = 0; i < b; i++){ \
        fprintf(stderr, "%.02x|", tbuf[i]); \
    } \
    fprintf(stderr, "]\n"); \
}

#else

#define DEBUG_PRINT(a)
#define DEBUG_PRINT_A(a)
#define DEBUG_PRINT_I(a)
#define DEBUG_IP
#define DEBUG_PORT
#define DEBUG_MEM(a,b)

#endif /* MMDB_DEBUG */

#endif /* MMCR_DEBUG_H */
