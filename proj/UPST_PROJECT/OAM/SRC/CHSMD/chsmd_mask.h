#ifndef __CHSMD_MASK_H__
#define __CHSMD_MASK_H__

enum {
    EN_SUBCH        = 0x01,
    EN_ICH          = 0x02,
    EN_DIRECTOR     = 0x04,
    EN_SWITCH       = 0x08,
    EN_DIRECTOR_ALL = 0x10,
    EN_SWITCH_ALL   = 0x20,
    EN_CH_ALL       = ( EN_SUBCH | EN_ICH | EN_DIRECTOR_ALL | EN_SWITCH_ALL )
} en_channel_type;

extern int dWriteMaskInfo(int rcv_type);
extern int dReadMaskInfo(int rcv_type);

#endif /* __CHSMD_MASK_H__ */
