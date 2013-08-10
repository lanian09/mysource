/**     @file   fb_chunked.c
 *      - FB Service Processing
 *
 *      Copyright (c) 2006~ by Upresto Inc. Korea
 *      All rights reserved
 *
 *       $Id: fb_chunked.c,v 1.2 2011/09/04 09:56:04 dhkim Exp $
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

// LIB
#include "typedef.h"
#include "commdef.h"

// PROJECT
#include "common_stg.h"

#include "fb_chunked.h"

unsigned int uiConvDecFromHexa(char *szIn, int len)
{
	unsigned int    i;
	unsigned int    tmp = 0, rst = 0;

	if(len > 8) return 0;

	for(i = 0; i < len; i++)
	{
		rst = rst << 4;
		DECFROMHEXA(tmp, szIn[i]);
		rst += tmp;
	}

	return rst;
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
S32 dGetChunked(U8 *out, S32 *outlen, U8 *in, S32 inlen)
{
	S32		start = 0, flag = 0;
	S32		totlen = *outlen;
	S32		offset = 0, tmp_offset = 0;
	S32		i;
	U8		szTmp[BUFSIZ];
	S32		tmp_state = END_STATE_EMPTY;

	*outlen = 0;

	for(i = 0; i < inlen; i++)
	{
		if((flag == 1) && (start == 0)) start -= 2;

//dAppLog(LOG_INFO, "i=%d value=%02X tmp_state=%d start=%d len=%d", i, in[i], tmp_state, start, offset);

		if(start == 0) {
			switch(tmp_state)
			{
			case END_STATE_EMPTY:
				switch(in[i])
				{
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
				case 'a':
				case 'A':
				case 'b':
				case 'B':
				case 'c':
				case 'C':
				case 'd':
				case 'D':
				case 'e':
				case 'E':
				case 'f':
				case 'F':
					tmp_state = END_STATE_30;
					szTmp[tmp_offset++] = in[i];
					break;
				default:
					return -1;	
				}	
				break;
			case END_STATE_30:
				switch(in[i])
				{
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
				case 'a':
				case 'A':
				case 'b':
				case 'B':
				case 'c':
				case 'C':
				case 'd':
				case 'D':
				case 'e':
				case 'E':
				case 'f':
				case 'F':
					tmp_state = END_STATE_30;
					szTmp[tmp_offset++] = in[i];
					break;
				case 0x0D:
					tmp_state = END_STATE_0D;
					break;
				default:
					return -2;	
				}	
				break;
			case END_STATE_0D:
				switch(in[i])
				{
				case 0x0A:
					szTmp[tmp_offset] = 0x00;
					start = uiConvDecFromHexa((char*)szTmp, tmp_offset);
					tmp_offset = 0;
					tmp_state = END_STATE_EMPTY;
					if(flag == 0) flag = 1;
					break;
				default:
					return -3;
				}
				break;
			default:
				return -4;
			}
		}
		else if(start > 0) {
			if(offset >= totlen) return -10;
			out[offset++] = in[i];
			start--;
		}
		else {
			flag = 0;
			start++;
		}
	}

	*outlen = offset;
	out[offset] = 0x00;		

	return 0;
}

/**
 *  $Log: fb_chunked.c,v $
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
 *  Revision 1.1  2008/06/22 10:18:02  dark264sh
 *  A_FB chunked, multipart, gzip, deflate, min 처리
 *
 */
          
