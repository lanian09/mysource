#ifndef _FB_MULTIPART_H_ 
#define _FB_MULTIPART_H_ 

extern S32 dGetEnd(U8 *out, U8 *in, S32 len);
extern S32 dGetMultiPart(U8 *out, S32 *outlen, S32 *zip, U8 *indata, S32 inlen, U8 *key, S32 keylen);

#endif /* _FB_MULTIPART_H_ */
