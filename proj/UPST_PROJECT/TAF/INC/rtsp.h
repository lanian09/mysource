#ifndef _RTSP_H_
#define _RTSP_H_

#include "common_stg.h"

extern int rtsp_reqhdr(char *sp, int slen, char *szMAN);
extern int rtsp_requrl(char *sp, int slen, int *pdTrackID);
extern int rtsp_reshdr(char *sp, int slen, unsigned int *puiSession, unsigned short *pusRTI, unsigned short *pusPort1, unsigned short *pusPort2);
extern int rtsp_resbody(char *sp, int slen, int *pdMinRange, int *pdMinMRange, int *pdMaxRange, int *pdMaxMRange, int *pdVideoTrackID, int *pdAudioTrackID);

#endif	/* _RTSP_H_ */
