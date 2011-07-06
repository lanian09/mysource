#include <stdio.h>

typedef union _un_Status {
    unsigned char   ucSts;
    struct {
#if 0
        unsigned char   init:2;     /* 0: not initial, 1:initial; */
        unsigned char   stop:2;     /* 0: alive, 1:stop(passive) */
        unsigned char   mask:2;     /* 0: unmask, 1:mask */
        unsigned char   sts:2;      /* 0 : Normal 1 : Warning 2 : Critical */
#endif
        unsigned char   sts:2;      /* 0 : Normal 1 : Warning 2 : Critical */
        unsigned char   mask:2;     /* 0: unmask, 1:mask */
        unsigned char   stop:2;     /* 0: alive, 1:stop(passive) */
        unsigned char   init:2;     /* 0: not initial, 1:initial; */
    } stSts;
}

int main()
{
        printf("1");
        return 0;
}
