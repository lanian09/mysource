/**     @file   fb_uncomp.c
 *      - FB Service Processing
 *
 *      Copyright (c) 2006~ by Upresto Inc. Korea
 *      All rights reserved
 *
 *       $Id: fb_uncomp.c,v 1.2 2011/09/04 09:56:05 dhkim Exp $
 *
 *      @Author     $Author: dhkim $
 *      @version    $Revision: 1.2 $
 *      @date       $Date: 2011/09/04 09:56:05 $
 *      @ref        fb_multipart.c
 *      @todo       library가 만들어지지 않은 상태, library 생성 이후 함수 대치
 *
 *      @section    Intro(소개)
 *      - FB Service Processing
 *
 *      @section    Requirement
 *       @li library 생성 이후 함수 대치
 *
 **/

#include <zlib.h>

// LIB
#include "typedef.h"

// .
#include "fb_uncomp.h"

/** dSend_FB_Data function.
 *
 *  dSend_FB_Data Function
 *
 *  @param  *pMEMSINFO : New Interface 관리 구조체
 *  @param  *pNode : 전송하고자 하는 Node
 *  @param  dSndMsgQ : send the msg to the next process
 *
 *  @return         S32  SUCCESS: 0, FAIL: -1(NIFO NODE 생성 실패) -2(TLV NODE 생성 실패) -3(메시지 전송 실패)
 *  @see            fb_msgq.c
 *
 **/
S32 dUncompress(U8 *out, S32 *outlen, U8 *in, S32 inlen)
{
	int				wbits = MAX_WBITS;
	int				err;
	z_stream		zstrm;
	z_streamp		strm = &zstrm;

	U8				*next;
	S32				complen;
	S32				inits_done = 0;

	strm->zalloc = Z_NULL;
	strm->zfree = Z_NULL;
	strm->opaque = Z_NULL;

	next = in;
	complen = inlen;

	if((in[0] == 0x1F) && (in[1] == 0x8B)) {
		Bytef *c = in + 2;
		Bytef flags = 0;

		if(*c == Z_DEFLATED) {
			c++;	
		} else {
			return -100;
		}

		flags = *c;

		/* Skip past the MTIME, XFL, and OS fields. */
		c += 7;

		if(flags & (1 << 2)) {
			int xsize = (int)(*c | (*(c + 1) << 8));
			c += xsize;
		}

		if(flags & (1 << 3)) {
			/* A null terminated filename */

			while(*c != '\0') {
				c++;
				if(c >= (in + inlen)) return -201;
			}

			c++;
		}

		if(flags & (1 << 4)) {
			/* A null terminated comment */

			while (*c != '\0') {
				c++;
				if(c >= (in + inlen)) return -202;
			}

			c++;
		}

		if(c >= (in + inlen)) return -200;

		next = c;

		complen -= (c - in);
	}

	strm->next_in = next;
	strm->avail_in = complen;

	strm->next_out = out;
	strm->avail_out = *outlen;

	err = inflateInit2(strm, wbits);
	if(err != Z_OK) return err;

	while(1) {

		err = inflate(strm, Z_SYNC_FLUSH);
		if(err == Z_OK || err == Z_STREAM_END) {
			*outlen = (*outlen) - strm->avail_out;
			err = Z_OK;
			break;
		} 

		else if(err == Z_DATA_ERROR && inits_done < 1) {
			wbits = -MAX_WBITS;

			inflateReset(strm);

			strm->next_in = next;
			strm->avail_in = complen;

			inflateEnd(strm);

			strm->next_out = out;
			strm->avail_out = *outlen;
			
			err = inflateInit2(strm, wbits);

			inits_done++;
			if(err != Z_OK) return err;
		}
		else {
			inflateEnd(strm);
			return err;
		}
	}

	inflateEnd(strm);
	return err;
}

/**
 *  $Log: fb_uncomp.c,v $
 *  Revision 1.2  2011/09/04 09:56:05  dhkim
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *  NEW OAM SYSTEM
 *
 *  Revision 1.2  2011/08/17 07:16:34  dcham
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 *  init DQMS2
 *
 *  Revision 1.2  2011/01/11 04:09:06  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2010/08/23 01:12:59  uamyd
 *  DQMS With TOTMON, 2nd-import
 *
 *  Revision 1.1.1.1  2009/05/26 02:14:15  dqms
 *  Init TAF_RPPI
 *
 *  Revision 1.2  2008/07/14 07:20:37  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.1  2008/06/22 10:18:02  dark264sh
 *  A_FB chunked, multipart, gzip, deflate, min 처리
 *
 */
          
