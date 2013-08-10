#ifndef __PPSCONF_DEFINE_HEADER_FILE__
#define __PPSCONF_DEFINE_HEADER_FILE__

#pragma pack(1)
/* struct for PrePaid Conf */
#define PPS_FLAG_ON     1
#define PPS_FLAG_OFF    0
typedef struct _st_PPS_CONF{
    unsigned int   uiInterimInterval;
    unsigned short usMaxCnt;
    unsigned short usOnOff;
    unsigned short usFilterOut;
    unsigned short usSvcType;
	unsigned short usBroadFilterOut;		/* 2007.10.11 FOR BROADCATING */
	unsigned short usBroadSvcType;			/* 2007.10.11 FOR BROADCATING */
} st_PPS_CONF, *pst_PPS_CONF;

#pragma pack()
#endif
