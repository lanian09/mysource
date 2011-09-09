#ifndef _FB_UNCOMP_H_ 
#define _FB_UNCOMP_H_ 

#define MAX_UNCOMP_SIZE		(1024 * 1024 * 10)

extern S32 dUncompress(U8 *out, S32 *outlen, U8 *in, S32 inlen);

#endif /* _FB_UNCOMP_H_ */
