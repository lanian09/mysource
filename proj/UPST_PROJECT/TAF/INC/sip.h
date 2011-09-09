#ifndef _SIP_H_
#define _SIP_H_

#include "common_stg.h"

extern int sip_trans_key(char *sp, int slen, char *callID, char *fromTag, int *seqNo, int *msgType, int *seqType);
extern int sip(char *sp, int slen, char *uri, char *from, char *to, unsigned int *sessid, unsigned short *audioport, unsigned short *videoport, char *useragent, char *auth_nonce, char *username, char *audioproto, char *videoproto, int *msgtype, int *event, char *contact, char *accept, unsigned short *msgport);
extern int sip_clientip(char *sp, int slen, char *ip);
extern int sipheader(char *sp, int slen, int *isYes);
extern int sip_contentlen(char *sp, int slen, int *contentlen);
extern int sip_model(char *sp, int slen, char *model);
extern int sip_min(char *sp, int slen, char *min, int *vendor);
extern int sip_invite(char *sp, int slen, int *range);
extern int sip_ctype(char *sp, int slen, int *ctype);
extern int sip_service(char *sp, int slen, int *dSvcType);

#endif	/* _SIP_H_ */
