#ifndef _HTTP_H_
#define _HTTP_H_

#include "common_stg.h"

extern int httpheader(char *sp, int slen, char *szLogUrl, int *pdLogUrlLen, int *pdUrlType, int *pdResCode, int *pdRet);
extern int httphdrinfo(char *sp, int slen, char *szHost, int *pdHostLen, char *szCType, int *pdCTypeLen, int *pdCLen, int *pdChunked, int *pdCEnco, int *pdPktCnt);
extern int httpctype(char *sp, int slen, int *CType, int *multilen, char *multi);
extern int httphost(char *sp, int slen, char *szHost, int *pHostLen);
extern int ssl(char *sp, int slen, char *szMin, char *szCPName, char *szSvcCode, char *szCompatible, int *pCompLen, int *pBaseID, int *pNID, int *pSID);
extern int mms_from(char *sp, int slen, char *szFrom);
extern int mms_to(char *sp, int slen, char *szTo);
extern int mms_to_dn(char *sp, int slen, int *pddn);
extern int mms_from_dn(char *sp, int slen, int *pddn);
extern int a2g_dn(char *sp, int slen, int *pddn);
extern int vod_dn(char *sp, int slen, int *pddn);
extern int httpctypeinfo(char *sp, int slen, char *szCType, int *pCTypeLen);

#endif	/* _HTTP_H_ */
