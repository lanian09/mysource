#ifndef __CAP_DEF_H__
#define __CAP_DEF_H__

typedef struct {
    unsigned char   bRtxType; /* 업다운 방향 */
    unsigned char   bEthIdx;  // ETH No.
    unsigned char   bReserve2;  // 17 - UDP(A11), 47 - GRE
    unsigned char   bOutOfOrder;    // 시간순이 약간 약간 안 맞는것
    long            curtime;
    long            ucurtime;
    int             datalen;
} Capture_Header_Msg;
#define CAP_HRD_LEN     sizeof(Capture_Header_Msg)
#define CAP_HDR_SIZE    CAP_HRD_LEN

/** 여기부터는  M_TRACE_CAPD 에서 사용하는 상수 , 구조체 */
#define MAX_MTU_SIZE    2048    /* WHEN 1520 IS ERROR OCCURED */

typedef struct
{
    unsigned int    pkno;
    unsigned int    size;
    unsigned int    ctime;
    unsigned int    cmtime;
} st_beacon_hdr, *pst_beacon_hdr;

#endif /* __CAP_DEF_H__ */
