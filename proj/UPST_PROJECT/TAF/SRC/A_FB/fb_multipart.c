/**     @file   fb_multipart.c
 *      - FB Service Processing
 *
 *      Copyright (c) 2006~ by Upresto Inc. Korea
 *      All rights reserved
 *
 *       $Id: fb_multipart.c,v 1.2 2011/09/04 09:56:04 dhkim Exp $
 *
 *      @Author     $Author: dhkim $
 *      @version    $Revision: 1.2 $
 *      @date       $Date: 2011/09/04 09:56:04 $
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

#include <ctype.h>

// LIB
#include "typedef.h"
#include "loglib.h"

// PROJECT
#include "common_stg.h"

// TAF
#include "http.h"

// .
#include "fb_multipart.h"

S32 dGetEnd(U8 *out, U8 *in, S32 len)
{
	int			i;
	int			state = END_STATE_EMPTY;

	for(i = 0; i < len; i++)
	{
		switch(state)
		{
		case END_STATE_EMPTY:
			switch(in[i])
			{
			case 0x0D:
				state = END_STATE_0D;
				break;
			default:
				state = END_STATE_EMPTY;
				break;
			}
			break;
		case END_STATE_0D:
			switch(in[i])
			{
			case 0x0A:
				state = END_STATE_0D0A;
				break;
			case 0x0D:
				state = END_STATE_0D;
				break;
			default:
				state = END_STATE_EMPTY;
				break;
			}
			break;
		case END_STATE_0D0A:
			switch(in[i])
			{
			case 0x0D:
				state = END_STATE_0D0A0D;
				break;
			default:
				state = END_STATE_EMPTY;
				break;
			}
			break;
		case END_STATE_0D0A0D:
			switch(in[i])
			{
			case 0x0A:
				state = END_STATE_0D0A0D0A;
				break;
			case 0x0D:
				state = END_STATE_0D;
				break;
			default:
				state = END_STATE_EMPTY;
				break;
			}
			break;
		default:
			log_print(LOGN_CRI, "[%s][%s.%d] INVALID STATUS [%d]", 
					__FILE__, __FUNCTION__, __LINE__, state);
			return 0;
		}

		if(out != NULL) {
			out[i] = in[i];
		}

		if(state == END_STATE_0D0A0D0A) {
			return (i + 1);
		}
	}
	
	return 0;
}

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
S32 dGetMultiPart(U8 *out, S32 *outlen, S32 *zip, U8 *indata, S32 inlen, U8 *key, S32 keylen)
{
	S32		state = 0;
	S32		totlen = *outlen;
	S32		offset = 0;
	S32		i, header_len = 0, body_len = 0;

	S32		dChunked = 0;
	U8		szCType[MAX_TEMP_CONTENT_SIZE];
	U8		szTemp[MAX_TEMP_CONTENT_SIZE];
	U8		szHostName[MAX_HOSTNAME_SIZE];
	S32		ctype;
	S32		ctypelen = 0;
	S32		hostlen = 0;
	S32		clen = 0;
	S32		pcnt = 0;

	U8		*in;

	in = &indata[keylen];
	inlen = inlen - keylen;
	*outlen = 0;
	keylen -= 2;

	if(keylen < 0) return -1;

	for(i = 0; i < inlen; i++)
	{
		if(state >= keylen) {
			/* parsing */
    		szCType[0] = 0x00;
			ctypelen = 0;
			*zip = 0;
			ctype = 0;

			/* find header */
			header_len = dGetEnd(NULL, out, offset);

			if(httphdrinfo((char*)out, header_len, (char*)szHostName, &hostlen, (char*)szCType, &ctypelen, &clen, &dChunked, zip, &pcnt) != 0) {
				/* 분석 실패 */
				offset = 0;
				state = 0;
				continue;
			} 
			else if(httpctype((char*)szCType, ctypelen, &ctype, &hostlen, (char*)szTemp) == 0) {
				if(ctype != 0) {
					body_len = dGetEnd(out, &out[header_len], offset - header_len);
					*outlen = body_len - 4;
					out[*outlen] = 0x00;		
					return i;
				} 
				else {
					offset = 0;
					state = 0;
					i = i + 2;
					continue;
				}
			}
		}

		if(offset >= totlen) return -2;

		if(key[state] == tolower(in[i])) state++;
		else state = 0;

		out[offset++] = in[i];	
	}

	return 0;
}

/**
 *  $Log: fb_multipart.c,v $
 *  Revision 1.2  2011/09/04 09:56:04  dhkim
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *  NEW OAM SYSTEM
 *
 *  Revision 1.3  2011/08/17 07:16:34  dcham
 *  *** empty log message ***
 *
 *  Revision 1.2  2011/08/08 11:05:41  uamyd
 *  modified block added
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
 *  Revision 1.3  2009/03/02 09:09:45  dark264sh
 *  DOWNLOAD VOD : 1x 단말에서 Content-Length, Transfer-Encoding Chunked, Multi Part 없이 Packet-Counter만 있는 경우 처리
 *
 *  Revision 1.2  2008/07/02 07:38:18  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.1  2008/06/22 10:18:02  dark264sh
 *  A_FB chunked, multipart, gzip, deflate, min 처리
 *
 */
          
