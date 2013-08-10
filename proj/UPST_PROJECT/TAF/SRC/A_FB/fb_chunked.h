#ifndef _FB_CHUNKED_H_ 
#define _FB_CHUNKED_H_ 

extern unsigned int uiConvDecFromHexa(char *szIn, int len);
extern S32 dGetChunked(U8 *out, S32 *outlen, U8 *in, S32 inlen);

#endif /* _FB_CHUNKED_H_ */
