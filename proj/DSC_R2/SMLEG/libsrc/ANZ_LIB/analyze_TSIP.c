/* **************************************************
 * $Id: 
 *
 * Copyright (c) 2006 by uPresto Inc., KOREA
 * All rights reserved.
 * ************************************************** */

/**
 @file		: ptt_tokenized_sip.c
 @brief		: tokenized SIP 분석을 위한 함수들. 
 			  해당 데이터들은 Canada Bell Mobility사의 데이터를 기준으로
			  작성되었다. 
			  개발 기준 : Canada SIP packet, 
			              SIP Mobility(TM) Stack Tokenization Protocol 
						  Version : 1.3(April 13, 2004)
						  Copyright of the paper : Core Mobility, Inc.
 @date		: 2006.05.08. Initial
 @author	: syhan@upresto.com
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <utillib.h>

#include "Analyze_Ext_Abs.h"
#include <define.h>

#define MAX_LINE_SIZE	128 // SIP Header 각 라인의 최대 size
#define MAX_TMPBUF_SIZE	64  // 임시로 사용할 버퍼의 size

#define MAX_OFFSET_SIZE  2  // OFFSET 정보의 크기 .. 
#define MAX_VERINFO_SIZE 5  // version info 크기 

unsigned char g_TokSipVer;



/**
 @brief     ParseTokSIP 함수
 
 Tokenized SIP 프로토콜을 분석한다. \n
 Tokenized 되어있는 부분을 조합해 SIP 메시지를 생성한다. \n
 분석이 완료되면, 

 SIP/SDP 를 분석하여 각 SIP 정보 구조체를 채운다. 	
 
 @return  분석 성공 : 0, 실패 : < 0
 */

int ParseTokSIP( char *data, int size, char *szSip )
{
	char tmpbuf[64];

	char szVerInfo[6];
	int  iLastOffset = 0;   //< Tokenized 에서 지정하는 마지막 offset
	int  idx = 0;
	char *tok_start;
	st_tok_table  stTok;
	char szHdrField[MAX_TOKHDR_SIZE];
	int toklen=0;
	int bufidx=0;
	unsigned char ucSDPFlag = 0x00;

	//char szSip[MAX_SIP_SIZE];  // 분석해서 normal sip 를 저장할 버퍼 
	//
	//
	//dAppLog( LOG_INFO, "[analib:%s] START [%ld.%d]-----------------------------------------------", 
	//__FUNCTION__, 
	//	pstPkt->tvCapTime.tv_sec, pstPkt->tvCapTime.tv_usec );

	//< Tokenized SIP의 경우 맨 처음(버전정보)이  0x80 이상으로 시작한다. 
	if( (unsigned char)data[0] < 0x80 )
	{
#ifdef DEBUG_DETAIL
		dAppLog( LOG_WARN, "[analib:%s] it's not Toknized SIP packet", __FUNCTION__  );
#endif
		log_hexa( data, size );
		return -1;
	}

	//< Get Tokenized SIP Version Infomation
	//< 해당 버전 정보는 SIP 메시지의 처음 5바이트이나, 이에 대한 정보는 없다. 
	memcpy( szVerInfo, data, MAX_VERINFO_SIZE ); szVerInfo[MAX_VERINFO_SIZE] = 0;

	if( memchr( szVerInfo+1, 0xFD04, 2) != NULL ) // SIP Version 2
		g_TokSipVer = 2;
	else
		g_TokSipVer = 1;

	idx += MAX_VERINFO_SIZE;

	//< Get Last String Offset : Offset과 맵핑되는 String의 사이즈 
	//< 각 string 은 00 로 구분된다. 
	memcpy( tmpbuf, data+5, MAX_OFFSET_SIZE); tmpbuf[MAX_OFFSET_SIZE] = 0;
	iLastOffset = sGetOffset( tmpbuf );
	idx += MAX_OFFSET_SIZE;

	//< Get offset table
	memset( &stTok, 0x00, sizeof(st_tok_table) );
	tok_start = data+idx;  // Version Info: 5 + LastOffset:2  = 7

	// Get first Offset 
	stTok.iOffset[stTok.iCnt] = sGetOffset( data+idx );
	idx += 2;
	sprintf( stTok.szString[stTok.iCnt], tok_start+stTok.iOffset[stTok.iCnt] );
	stTok.iCnt++;

	while( (idx-MAX_VERINFO_SIZE-MAX_OFFSET_SIZE) < stTok.iOffset[0] )
	{
		stTok.iOffset[stTok.iCnt] = sGetOffset( data+idx );
		idx+=2;
		
		sprintf( stTok.szString[stTok.iCnt], tok_start+stTok.iOffset[stTok.iCnt] );
#ifdef DEBUG_DETAIL
dAppLog( LOG_INFO, "[analib:%s] [DIC:%d - %s] %s", __FUNCTION__, stTok.iCnt, stTok.szString[stTok.iCnt], tok_start+stTok.iOffset[stTok.iCnt] );
#endif
		stTok.iCnt++;
	} // end of while
#if 0
	for( i = 0; i < stTok.iCnt; i++ )
	{
		/*
		if( i == stTok.iCnt-1 )
			stTok.iStrLen[i] = iLastOffset - stTok.iOffset[stTok.iCnt-1];
		else
			stTok.iStrLen[i] = stTok.iOffset[i+1] - stTok.iOffset[i];
			*/

		//memcpy( stTok.szString[i], tok_start+stTok.iOffset[i], stTok.iStrLen[i] );
		sprintf( stTok.szString[i], tok_start+stTok.iOffset[i] );

	} // end of for
#endif

#if 0	
	dAppLog( LOG_INFO, "[analib:%s] START [%ld.%d]-----------------------------------------------", 
		__FUNCTION__, 
		pstPkt->tvCapTime.tv_sec, pstPkt->tvCapTime.tv_usec );
#endif


	// set index
	idx = iLastOffset+MAX_VERINFO_SIZE+MAX_OFFSET_SIZE;

	szSip[0]=0;
	while( idx < size )	
	{
		switch( (unsigned char)data[idx])
		{
			case 0x00: 		
				//idx += 1;
#ifdef DEBUG_DETAIL
				dAppLog( LOG_DEBUG, "[analib:%s] Undefined field[0x%02X]", __FUNCTION__, data[idx] );
#endif
				idx++;
				break;
			case 0x08:
				toklen = iGetTokField( data+idx, size-idx, szHdrField, 2 );
				bufidx += iMake_KnownHeader( &stTok, szHdrField, toklen, szSip+bufidx );
				break;
			case 0x09:
				toklen = iGetTokField( data+idx, size-idx, szHdrField, 2 );
				bufidx += iMake_UnknownHeader( &stTok, szHdrField, toklen, szSip+bufidx );
				break;

			case 0x60:  //Generic Request Start-Line
				if( data[idx+1] == 0x01 )
					toklen = iGetTokField( data+idx, size-idx, szHdrField, 4 ); 	
				else
					toklen = iGetTokField( data+idx, size-idx, szHdrField, 3 );

				bufidx += iMake_RequestStartLine( &stTok, szHdrField, toklen, szSip );
				break;

			case 0x61:	//Generic Response Start-Line
				toklen = iGetTokField( data+idx, size-idx, szHdrField, 4 );
				bufidx += iMake_ResponseStartLine( szHdrField, toklen, szSip );
				break;

			case 0x62:	//INVITE Fixed-Format Start-Line
				toklen = iGetTokField( data+idx, size-idx, szHdrField, 2 );
				bufidx += iMake_INVITE( &stTok, szHdrField, toklen, szSip+bufidx );
				break;

			case 0x63:	//VIA header Fixed-Format
				toklen = iGetTokField( data+idx, size-idx, szHdrField, 4 );
				bufidx += iMake_VIA( &stTok, szHdrField, toklen, szSip+bufidx );
				break;

			case 0x64:	//To header fixed-format
			case 0x65:  //From header fixed-format
				toklen = iGetTokField( data+idx, size-idx, szHdrField, 2 );
				bufidx += iMake_FromTo( &stTok, szHdrField, toklen, szSip+bufidx );
				break;
			
			case 0x66:	//CSeq header
				toklen = iGetTokField( data+idx, size-idx, szHdrField, 6 );
				bufidx += iMake_CSeq( szHdrField, toklen, szSip+bufidx );
				break;
	
			case 0x67:	//Client-style Call-ID Fixed-format		
				toklen = iGetTokField( data+idx, size-idx, szHdrField, 6 );
				bufidx += iMake_CliCallID( &stTok, szHdrField, toklen, szSip+bufidx );
				break;

			case 0x68:	//Server-style Call-ID Fixed-format
				toklen = iGetTokField( data+idx, size-idx, szHdrField, 3 );
				bufidx += iMake_SrvCallID( &stTok, szHdrField, toklen, szSip+bufidx );
				break;

			case 0x69:	//Contact Header
				toklen = iGetTokField( data+idx, size-idx, szHdrField, 2 );
				bufidx += iMake_FromTo( &stTok, szHdrField, toklen, szSip+bufidx );
				break;

			case 0x6A:	//Proxy-Authenticate Fixed-Format
#ifdef DEBUG_DETAIL
				dAppLog( LOG_INFO, "[analib:%s] MUST DO Proxy-Authenticate Fixed-Format", __FUNCTION__ );
#endif
				toklen = iGetTokField( data+idx, size-idx, szHdrField, 4 );
				break;

			case 0x6B:	//Proxy-Authorization No-URI Fixed-Format
				toklen = iGetTokField( data+idx, size-idx, szHdrField, 21 );
				bufidx += iMake_ProxyAuthorization( &stTok, szHdrField, toklen, szSip+bufidx );
				break;

			case 0x6C:	//Record-Route Header
				toklen = iGetTokField( data+idx, size-idx, szHdrField, 2 );
				bufidx += iMake_FromTo( &stTok, szHdrField, toklen, szSip+bufidx );
				break;

			case 0x6D:	//Route Header
				toklen = iGetTokField( data+idx, size-idx, szHdrField, 2 );
				bufidx += iMake_FromTo( &stTok, szHdrField, toklen, szSip+bufidx );
				break;

			case 0x6F:  //Proxy-Authorization header With URI
				toklen = iGetTokField( data+idx, size-idx, szHdrField, 22 );
				bufidx += iMake_ProxyAuthorization( &stTok, szHdrField, toklen, szSip+bufidx );
				break;

			case 0x70:	//Generic Content-Start	
				toklen = iGetTokField( data+idx, size-idx, szHdrField, 1 );
#ifdef DEBUG_DETAIL
				dAppLog( LOG_DEBUG, "[analib:%s] MUST DO Generic Content-Start[0x%02X]", 
						__FUNCTION__, (unsigned char)data[idx] );
#endif
				break;

			case 0x71:	//Fixed format Basic SDP
				toklen = iGetSDPTokField( data+idx, size-idx, szHdrField, 2 );
#ifdef DEBUG_DETAIL
				dAppLog( LOG_DEBUG, "[analib:%s] MUST DO Fixed format Basic SDP[0x%02X]", 
						__FUNCTION__, (unsigned char)data[idx] );
#endif
				break;

			case 0x72:	//Content-Type application/sdp-Fixed Format 2
				ucSDPFlag = 0x01;
				szSip[bufidx++] =0x0D;
				szSip[bufidx++] = 0x0A;
				toklen = iGetSDPTokField( data+idx, size-idx, szHdrField, 0 );
				bufidx += iMake_MultiPartSDPStart( &stTok, szHdrField, toklen, szSip+bufidx );
				break;

			case 0x73:	
				toklen = iGetSDPTokField( data+idx, size-idx, szHdrField, 2 );
#ifdef DEBUG_DETAIL
				dAppLog( LOG_DEBUG, "[analib:%s] MUST DO Fixed format multi-part SDP Time [0x%02X]", 
						__FUNCTION__, (unsigned char)data[idx] );
#endif
				break;

			case 0x74:	
				toklen = iGetSDPTokField( data+idx, size-idx, szHdrField, 2 );
#ifdef DEBUG_DETAIL
				dAppLog( LOG_DEBUG, "[analib:%s] MUST DO Fixed format multi-part SDP Extra [0x%02X]", 
						__FUNCTION__, (unsigned char)data[idx] );
#endif
				break;

			case 0x75:
				toklen = iGetSDPTokField( data+idx, size-idx, szHdrField, 0 );
				bufidx += iMake_MultiPartSDPMedia( &stTok, szHdrField, toklen, szSip+bufidx );
				break;

			case 0xE9: // expires
				toklen = iGetTokField( data+idx, size-idx, szHdrField, 4 );
				bufidx += iMake_CommonlyUsedHeader( &stTok, szHdrField, toklen, szSip+bufidx );
				break;

			case 0xFB: // Timestamp
				toklen = iGetTokField( data+idx, size-idx, szHdrField, 2 );
				bufidx += iMake_CommonlyUsedHeader( &stTok, szHdrField, toklen, szSip+bufidx );
				break;

			default:
				// check commonly-used header starts, 0..31
				//if( ((unsigned char)data[idx] >= 0xE0) && ((unsigned char)data[idx] <= 0xFF) )
				if( (unsigned char)data[idx] >= 0xE0 )
				{
					toklen = iGetTokField( data+idx, size-idx, szHdrField, 0 );
					bufidx += iMake_CommonlyUsedHeader( &stTok, szHdrField, toklen, szSip+bufidx );
				}
				else
				{
#ifdef DEBUG_DETAIL
					dAppLog( LOG_DEBUG, "[analib:%s] Unknown token[0x%02x]", 
							__FUNCTION__, (unsigned char)data[idx] );
#endif
					toklen = 0;
				}
				break;
		} //end of switch

		if( toklen == 0 )
			idx++;
		else
		{
			idx+= toklen;
			//log_hexa( szHdrField, toklen );
		}

	} // end of while

	// SDP 를 사용하지 않은 SIP 이므로 헤더 구분자를 만들어 준다. 
	if( ucSDPFlag == 0 )
	{
		szSip[bufidx++] = 0x0D;
		szSip[bufidx++] = 0x0A;
	}
	else
	{
		if( szSip[bufidx-2] == 0x0D && szSip[bufidx-1] == 0x0A )
		{
			bufidx -= 2;
		}
	}

	szSip[bufidx] = 0x00;

	if( idx != size )
	{
#ifdef DEBUG_DETAIL
		dAppLog( LOG_WARN, "[analib:%s] Remain field information!! plz.. check this field[idx:%d size:%d]",
				__FUNCTION__, idx, size );
#endif
		log_hexa( data, size );
	}
#ifdef DEBUG_DETAIL
	dAppLog( LOG_INFO, "[analib:%s]\n\n%s\n\n", __FUNCTION__, szSip );
#endif


	return 0;
}

int iGetTokField( char *data, int datalen,  char *tokfield, int deflen )
{
	int len=0;

	//< 맨 처음데이터(header 구분 데이터)가 헤더구분용 토큰이 아니면 리턴.  
	if( 0 > iIsHdrToken(data[len]) )
	{
#ifdef DEBUG_DETAIL
		dAppLog( LOG_INFO, "[analib:%s] Unknown Header field [0x%02x]", __FUNCTION__, data[len] );
#endif
		tokfield[len]=0;
		return len;
	}

	memcpy( tokfield, data, deflen );
	len += deflen;

	while( datalen > len )
	{
		//< 다음번 헤더구분 토큰이 나올때까지를 한 토큰 필드로 본다. 
		if( (len > 0) && len >= deflen && iIsHdrToken(data[len]) )
		{
			tokfield[len]=0;
			return len;
		}
		else
		{
			if( len > MAX_TOKHDR_SIZE )
			{
				tokfield[0] = 0;
				return 0;
			}

			tokfield[len] = data[len];
			len++;
		} 
	}

	return len;

}

int iIsHdrToken( const unsigned char tok )
{
	if( (tok == 0x08)							//< header start using known header
		|| (tok == 0x09)						//< header start using unknown header(any string)
		|| (tok >= 0x60 && tok <= 0x7F)		//< fixed-format headers & content
		|| (tok >= 0xE0) )	//< commonly-used header start, 0.. 31
		return 1;
	else
		return 0;
}


short sGetOffset( char *hex )
{
	int len=0;
	char tmpbuf[2];

	tmpbuf[0] = hex[1];
	tmpbuf[1] = hex[0];

	memcpy( &len, tmpbuf, 2 );

	return len;
}

int iConvert_Array2INT32( char *hex )
{
	int num = 0;
	char tmpbuf[4];

	tmpbuf[0] = hex[3];
	tmpbuf[1] = hex[2];
	tmpbuf[2] = hex[1];
	tmpbuf[3] = hex[0];

	memcpy( &num, tmpbuf, 4 );

	return num;
}

short iConvert_Array2INT16( char *hex )
{
	int num = 0;
	char tmpbuf[2];

	tmpbuf[0] = hex[1];
	tmpbuf[1] = hex[0];

	memcpy( &num, tmpbuf, 2 );

	return num;
}




/**
 @brief     Generic Request Start-Line 생성  \n
			field : 추철한 토큰  ( 60 0ㅇ 00 00 A6 ) \n
			len : field 의 길이  \n
			sipbuf : 분석한  normal Request Start-Line 을 저장한 버퍼  \n\n
	

 @return   성공 : 0, 실패 : < 0
 */
int iMake_RequestStartLine( st_tok_table *pstTok, char *field, int len, char *sipbuf )
{
	int idx=0, bufidx=0;
	st_bitfield stBF;
	unsigned short usParamCnt;

	// Invalid check 
	if( field[idx++] != 0x60 || len < 3 )
	{
#ifdef DEBUG_DETAIL
		dAppLog( LOG_DEBUG, "[analib:%s] FAIL IN iMake_RequestStartLine(not indicator: 0x%02x / default len: %d)", 
			__FUNCTION__, field[0], len );
		log_hexa( field, len );
#endif
	}

	bufidx += iGetMethod( field[idx++], sipbuf+bufidx );
	if( field[1] == SIP_UNKNOWN && bufidx == 0 )
	{
#ifdef DEBUG_DETAIL
		dAppLog( LOG_WARN, "[analib:%s] UNKNOWN SIP METHOD[0x%02X]", __FUNCTION__, field[1] );
#endif
		bufidx += iGetUnknownMethod( field[idx++], sipbuf );
	}

	sipbuf[bufidx++] = ' ';
	SetBitField( &stBF, field[idx++] );


	// sip scheme 
	if( stBF.bb == 0 )
	{
		sprintf( sipbuf+bufidx, "sip:" );
		bufidx = strlen(sipbuf);

		// two user parts(f=1) only sip sheme
		if( stBF.f == 1 )
		{
			bufidx += iGetString( sipbuf+bufidx, field[idx++], 0, pstTok );
		}
		
		// user part( or null if no user part). Implies '@'
		if( field[idx] != 0 )
		{
			bufidx += iGetString( sipbuf+bufidx, field[idx++], 0, pstTok );
			sipbuf[bufidx++] = '@';
		}
		else
		{
			idx++;
		}

		// host part
		bufidx += iGetString( sipbuf+bufidx, field[idx++], 0, pstTok );
		
		// specified port(c=1)
		if( stBF.c == 1 )
		{
			sprintf( sipbuf+bufidx, ":%d", iConvert_Array2INT16(field+idx) );
			idx += 2;
		}
	}
	// tel scheme
	else if( stBF.bb == 1 )
	{
		sprintf( sipbuf+bufidx, "tel:" );
		bufidx = strlen(sipbuf);

		// destination pone number string
		bufidx += iGetString( sipbuf+bufidx, field[idx++], 0, pstTok );
	}
	else if( stBF.bb == 3 )
	{
#ifdef DEBUG_DETAIL
		dAppLog( LOG_DEBUG, "[analib:%s] MUST CHECK.... other scheme ", __FUNCTION__ );
		log_hexa( field, len );
#endif
		bufidx += iGetString( sipbuf+bufidx, field[idx++], 0, pstTok );
		sipbuf[bufidx++] = ':';	
		bufidx += iGetString( sipbuf+bufidx, field[idx++], 0, pstTok );
	}

	// If flags indicates params (d=1)
	if( stBF.d == 1 )
	{
		usParamCnt = (unsigned short)field[idx++];
		
		int i;
		for( i = 0; i < usParamCnt; i++ )
		{
			if( idx > len )
			{
#ifdef DEBUG_DETAIL
				dAppLog( LOG_WARN, "[analib:%s] Warning.. check field length to parse Parameters[idx:%d/len:%d]", 
						__FUNCTION__,  idx, len );
#endif
				sipbuf[0] = 0;
				return 0;
			}

			bufidx += iGetParamString( sipbuf+bufidx, field+idx, pstTok );
			idx += 3;
		} // end of for
	}


	sprintf( sipbuf+bufidx, " SIP/2.0" );
	bufidx = strlen(sipbuf);

	sipbuf[bufidx++] = 0x0D;
	sipbuf[bufidx++] = 0x0A;
	sipbuf[bufidx] = 0x00;

	if( len != idx )
	{
#ifdef DEBUG_DETAIL
		dAppLog( LOG_WARN, "[analib:%s] check field & analyzing..len[%d] idx[%d] [%s]", __FUNCTION__ , len, idx, sipbuf );
		log_hexa( field, len );
#endif
	}

#ifdef DETAIL
	dAppLog( LOG_INFO, "[analib:%s]\n%s",__FUNCTION__,  sipbuf );
#endif

	return strlen(sipbuf);
}

void SetBitField( st_bitfield *pstBF, const unsigned char field )
{
	// set 'a'
	if( (field & 0x01) == 0x01 )
		pstBF->a = 0x01;
	else 
		pstBF->a = 0x00;

	// set 'bb'
	if( (field & 0x06) == 0x00 )
		pstBF->bb = 0x00;
	else if( (field & 0x06) == 0x02 )
		pstBF->bb = 0x01;
	else if( (field & 0x06) == 0x04 )
		pstBF->bb = 0x02;
	else 
		pstBF->bb = 0x03;

	// set 'c'
	if( (field & 0x08) == 0x08 )
		pstBF->c = 0x01;
	else 
		pstBF->c = 0x00;

	// set 'd'
	if( (field & 0x10) == 0x10 )
		pstBF->d = 0x01;
	else
		pstBF->d = 0x00;

	// set 'e'
	if( (field & 0x20) == 0x20 )
		pstBF->e = 0x01;
	else
		pstBF->e = 0x00;

	// set 'f'
	if( (field & 0x40) == 0x40 )
		pstBF->f = 0x01;
	else
		pstBF->f = 0x00;

	return;
}

void SetParamBitField( st_bitfield_param *pstPBF, const unsigned char field )
{
	// set 'aa'
	if( (field & 0x03) == 0x00 )
		pstPBF->aa = 0x00;
	else if( (field & 0x06) == 0x02 )
		pstPBF->aa = 0x01;
	else if( (field & 0x06) == 0x04 )
		pstPBF->aa = 0x02;
	else 
		pstPBF->aa = 0x03;

	// set 'b'
	if( (field & 0x04) == 0x04 )
		pstPBF->b = 0x01;
	else
		pstPBF->b = 0x00;

	// set 'c'
	if( (field & 0x08) == 0x08 )
		pstPBF->c = 0x01;
	else
		pstPBF->c = 0x00;
}

int iGetUnknownMethod( const unsigned char method,  char *buf )
{
	return 0;
}

int iGetMethod( const unsigned char method,  char *buf )
{
	switch( (int)method )
	{
		case SIP_RESPONSE:
			sprintf( buf, "%s", "RESPONSE" );
			break;
	
		case SIP_UNKNOWN:
			// i've never seen unknown method data
			// so, this flow is an assumption according to Core Mobility paper. 
			// surely, this flow must be tested and verificated... written by syhan :)
			//buf[0] = 0;
			sprintf( buf, "%s", "UNKNOWN" );
			break;

		case SIP_ACK:
			sprintf( buf, "%s", "ACK" );
			break;
		
		case SIP_BYE:
			sprintf( buf, "%s", "BYE" );
			break;

		case SIP_CANCEL:
			sprintf( buf, "%s", "CANCEL" );
			break;

		case SIP_INFO:
			sprintf( buf, "%s", "INFO" );
			break;

		case SIP_INVITE:
			sprintf( buf, "%s", "INVITE" );
			break;

		case SIP_MESSAGE:
			sprintf( buf, "%s", "MESSAGE" );
			break;

		case SIP_NOTIFY:
			sprintf( buf, "%s", "NOTIFY" );
			break;
	
		case SIP_OPTIONS:
			sprintf( buf, "%s", "OPTIONS" );
			break;

		case SIP_PRACK:
			sprintf( buf, "%s", "PRACK" );
			break;
			
		case SIP_PUBLISH:
			sprintf( buf, "%s", "PUBLISH" );
			break;
		
		case SIP_REFER:
			sprintf( buf, "%s", "PRFER" );
			break;

		case SIP_REGISTER:
			sprintf( buf, "%s", "REGISTER" );
			break;

		case SIP_SUBSCRIBE:
			sprintf( buf, "%s", "SUBSCRIBE" );
			break;

		case SIP_UPDATE:
			sprintf( buf, "%s", "UPDATE" );
			break;

		default:
			dAppLog( LOG_DEBUG, "[analib:%s] Unknown Method[0x%02x]", __FUNCTION__, method );
			break;
	} // end of switch 

	return strlen(buf);
}

int iMake_ResponseStartLine( char *field, int len, char *sipbuf )
{
	int idx = 0;
	int sipIdx = 0;
	unsigned short usStatusCode;
	char tmpbuf[3];
	

    // Invalid check
    if( field[idx++] != 0x61 || len < 4 )  // 4 is default size
    {
#ifdef DEBUG_DETAIL
        dAppLog( LOG_DEBUG, "[analib:%s] FAIL ...not indicator: 0x%02x / default len: %d",
            __FUNCTION__, field[0], len );
        log_hexa( field, len );
#endif
    }
	

    log_hexa( field, len );
	/*
	sipIdx += iGetMethod( field[idx++], sipbuf+sipIdx );
	sipbuf[sipIdx++] = ' ';
	*/

	sipIdx += sprintf( sipbuf+sipIdx, "SIP/2.0 " );


	tmpbuf[0] = field[idx+1];
	tmpbuf[1] = field[idx];
	tmpbuf[2] = 0;
	idx += 2;
	

	memcpy( &usStatusCode, tmpbuf, 2 );
	sipIdx += sprintf( sipbuf+sipIdx, "%d ", usStatusCode );
	//sipIdx += 2;
	//idx++;

	if( field[idx]== 0x01 ) // low range
	{
		sipIdx += iGet_SipDic_ver1( sipbuf+sipIdx, (unsigned char)field[++idx],  0 );
		idx += 1;
	}
	else if( field[idx] == 0x02 )  // high range
	{
		sipIdx += iGet_SipDic_ver1( sipbuf+sipIdx, 256+(unsigned char)field[++idx], 0 );
		idx += 1;
	}
	else 
	{
		sipIdx += iGet_SipDic_ver1( sipbuf+sipIdx, (unsigned char)field[idx], 1 );
		idx++;
	}

	sipbuf[sipIdx++] = 0x0D;
	sipbuf[sipIdx++] = 0x0A;
	sipbuf[sipIdx] = 0;

    if( len != idx )
	{
#ifdef DEBUG_DETAIL
		dAppLog( LOG_WARN, "[analib:%s] check field & analyzing..len[%d] idx[%d]", __FUNCTION__ , len, idx);
		log_hexa( field, len );
#endif
	}

	return strlen(sipbuf);

}

int iMake_CSeq( char *field, int len, char *sipbuf )
{
	int idx=0, bufidx=0;
	unsigned int uiCSeq;

	// Invalid check
	if( field[idx++] != 0x66 || len < 6 )
	{
#ifdef DEBUG_DETAIL
		dAppLog( LOG_DEBUG, "[analib:%s] not indicator: 0x%02x / default len: %d", 
				__FUNCTION__, field[0], len );
		log_hexa( field, len );
#endif
	}

	uiCSeq = iConvert_Array2INT32( field+idx );
	idx += 4;

	sprintf( sipbuf+bufidx, "CSeq: %u ", uiCSeq );
	bufidx += strlen(sipbuf);

	if( len == 6 && field[0] != 0x01 )
	{
		bufidx += iGetMethod( field[idx++], sipbuf+bufidx );
	}
	else   // UNKNOWN METHOD 인경우 ... 아직 보지 못했음.. 추후 추가 필요. 
	{
		dAppLog( LOG_DEBUG, "[analib:%s] Unknown Method", __FUNCTION__ );
		log_hexa( field, len );
	}
	
	sipbuf[bufidx++] = 0x0D;
	sipbuf[bufidx++] = 0x0A;
	sipbuf[bufidx] = 0;

    if( len != idx )
	{
#ifdef DEBUG_DETAIL
		dAppLog( LOG_WARN, "[analib:%s] check field & analyzing..len[%d] idx[%d]", 
				__FUNCTION__ , len, idx);
		log_hexa( field, len );
#endif
	}

	
	return strlen(sipbuf);
}

int iMake_CliCallID( st_tok_table *pstTok, char *field, int len, char *sipbuf )
{
	int idx=0, bufidx=0;
	
	// Invalid check
	if( field[idx++] != 0x67 || len < 6 )
	{
#ifdef DEBUG_DETAIL
		dAppLog( LOG_DEBUG, "[FAIL:%s] not indicator: 0x%02x / default len: %d", 
				__FUNCTION__, field[0], len );
		log_hexa( field, len );
#endif
	}
	
	sprintf( sipbuf+bufidx, "Call-ID: %02X%02X%02X%02X@%s%c%c", 
		(unsigned char)field[idx], (unsigned char)field[idx+1], 
		(unsigned char)field[idx+2], (unsigned char)field[idx+3],
		pstTok->szString[(unsigned char)field[5]-0x80], 0x0D, 0x0A );

	/*
	if( len != idx )
	{
		dAppLog( LOG_WARN, "[analib:%s] check field & analyzing..len[%d] idx[%d]",
			   __FUNCTION__ , len, idx);
		log_hexa( field, len );
	}
	*/

	return strlen(sipbuf);
}

int iMake_SrvCallID( st_tok_table *pstTok, char *field, int len, char *sipbuf )
{
	int idx=0, bufidx=0;

	// Invalid check
	if( field[idx++] != 0x68 || len < 3 )
	{
		dAppLog( LOG_DEBUG, "[analib:%s] FAIL not indicator: 0x%02x / default len: %d", 
				__FUNCTION__, field[0], len );
		log_hexa( field, len );
	}

	sprintf( sipbuf+bufidx, "Call-ID: %s@%s%c%c", 
			pstTok->szString[(unsigned char)field[1]-0x80], 
			pstTok->szString[(unsigned char)field[2]-0x80],
			0x0D, 0x0A );
	/*
	if( len != idx )
	{
		dAppLog( LOG_WARN, "[analib:%s] check field & analyzing..len[%d] idx[%d]",
			   __FUNCTION__ , len, idx);
		log_hexa( field, len );
	}
	*/

	return strlen(sipbuf);
			
}

int iMake_ProxyAuthorization( st_tok_table *pstTok, unsigned char *field, int len, char *sipbuf )
{
	int idx=0, bufidx=0;

	// Invalid check
	if( (field[0] != 0x6B && field[0] != 0x6F) && len < 21 )
	{
		dAppLog( LOG_DEBUG, "[analib:%s] FAIL... not indicator: 0x%02x / default len: %d", 
				__FUNCTION__, field[0], len );
		log_hexa( field, len );
		return 0;
	}
	idx++;

	sprintf( sipbuf+bufidx, "Proxy-Authorizaton: Disgest Username=\"%s@",
		pstTok->szString[field[idx++]-0x80] );
	bufidx = strlen(sipbuf);
	bufidx += iGet_LocDir_ver2( sipbuf+bufidx, field[idx++] );

	// realm
	sprintf( sipbuf+bufidx, "\",realm=\"" );
	bufidx = strlen(sipbuf);
	bufidx += iGet_LocDir_ver2( sipbuf+bufidx, field[idx++] );
	sipbuf[bufidx++] = 0x22;

	// nonce
	sipbuf[bufidx++] = ',';
	sprintf( sipbuf+bufidx, "nonce=\"%s\"",  pstTok->szString[field[idx++]-0x80] );
	bufidx = strlen( sipbuf );

	// uri ... if 0x6F.. 
	if( field[0] == 0x6F )
	{
		sipbuf[bufidx++] = ',';
		sprintf( sipbuf+bufidx, "uri=\"%s\"", pstTok->szString[field[idx++]-0x80] );
		bufidx = strlen(sipbuf);
	}
	
	//response
	sipbuf[bufidx++] = ',';
	sprintf( sipbuf+bufidx, "response=\"" );
	bufidx = strlen(sipbuf);
	
	for( ;idx < len; idx++ )
	{
		sprintf( sipbuf+bufidx, "%02X", field[idx] );
		bufidx += 2;
	} // end of for
	
	sipbuf[bufidx++] = 0x22;  // "
	sipbuf[bufidx++] =  0x0D;
	sipbuf[bufidx++] = 0x0A;
	sipbuf[bufidx] = 0;


	if( len != idx )
	{
		dAppLog( LOG_WARN, "[analib:%s] check field & analyzing..len[%d] idx[%d]",
			   __FUNCTION__ , len, idx);
		log_hexa( field, len );
	}

	return strlen(sipbuf);
}


int iMake_INVITE( st_tok_table *pstTok, unsigned char *field, int len, char *sipbuf )
{
	int idx=0, bufidx=0;
	st_bitfield stBF;
	st_bitfield_param stPBF;

	char tmpbuf[64];

	// Invalid check
	if( field[idx++] != 0x62 || len < 2 )
	{
		dAppLog( LOG_DEBUG, "[analib:%s] not indicator: 0x%02x / default len: %d", 
				__FUNCTION__, field[0], len );
		log_hexa( field, len );
	}

	SetBitField( &stBF, field[idx++] );

	if( stBF.bb == 0x00 )
		sprintf( tmpbuf, "sip:" );
	else if( stBF.bb == 0x01 )
		sprintf( tmpbuf, "tel:" );
	else if( stBF.bb == 0x02 )  // Reserved 
		tmpbuf[0] = 0;
	else 		// scheme is provided by a string token
		tmpbuf[0] = 0;


	sprintf( sipbuf+bufidx, "INVITE %s%s", tmpbuf, pstTok->szString[field[idx++]-0x80] );
	bufidx = strlen(sipbuf);

	if( stBF.f == 0x01 )
	{
		bufidx += iGet_LocDir_ver2( sipbuf+bufidx, field[idx++] );
		sipbuf[bufidx++] = '@';
		sprintf( sipbuf+bufidx, "%s", pstTok->szString[field[idx++]-0x80] );
		bufidx = strlen(sipbuf);
	}

	// make parameters...
	if( field[idx] >= 0x40 && field[idx] <= 0x4F )
	{
		SetParamBitField( &stPBF, field[idx++] );

		if( stPBF.aa == 0x00 )
			sipbuf[bufidx++] = ';';   // paramater 구분자 
		else if( stPBF.aa == 0x01 )
			sipbuf[bufidx++] = ',';
		else if( stPBF.aa == 0x02 )
			sipbuf[bufidx++] = '&';  // 확인 요망.. 경우에 따라 '?' 가 올 수 있음. 
		else // reserved
#ifdef DEBUG_DETAIL
			dAppLog( LOG_DEBUG, "[analib:%s] it's reserved parameter set... aa is 0x%02x", 
					__FUNCTION__, stPBF.aa );
#endif

		bufidx += iGet_SipDic_ver1( sipbuf+bufidx, field[idx++], 1 );  // 1: search by direct token
		sipbuf[bufidx++] = '=';

		if( stPBF.b == 0x01 )
		{
			sipbuf[bufidx++] = '<';
			bufidx += iGet_LocDir_ver2( sipbuf+bufidx, field[idx] );
			sipbuf[bufidx++] = '>';
		}
		else
		{
			bufidx += iGet_LocDir_ver2( sipbuf+bufidx, field[idx] );
		}
	}

	if( g_TokSipVer == 2 ) // Version 2
		sprintf( sipbuf+bufidx, " SIP/2.0" );
	bufidx = strlen(sipbuf);


	sipbuf[bufidx++] = 0x0D;
	sipbuf[bufidx++] = 0x0A;
	sipbuf[bufidx] = 0;


	if( len != idx )
	{
		dAppLog( LOG_WARN, "[analib:%s] check field & analyzing..len[%d] idx[%d]",
			   __FUNCTION__ , len, idx);
		log_hexa( field, len );
	}

	return strlen(sipbuf);
}

int iMake_VIA( st_tok_table *pstTok, unsigned char *field, int len, char *sipbuf )
{
	int idx=0, bufidx=0;

	// Invalid check
	if( field[idx++] != 0x63 || len < 4 )
	{
		dAppLog( LOG_DEBUG, "[analib:%s] FAIL... not indicator: 0x%02x / default len: %d", 
				__FUNCTION__, field[0], len );
		log_hexa( field, len );
	}

	sprintf( sipbuf+bufidx, "Via: SIP/2.0/UDP %s", 
					pstTok->szString[(unsigned char)field[idx++]-0x80] );
	bufidx = strlen(sipbuf);

	if( field[idx] != 0 )  // if null, there is no branch parameter. 
	{
		sprintf( sipbuf+bufidx, ";branch=z9hG4bK%s",  
				pstTok->szString[(unsigned char)field[idx++]-0x80]);
		bufidx = strlen(sipbuf);
	}

	if( field[idx++] != 0 )  // if null, there is no tok parameter.
	{
		sprintf( sipbuf+bufidx, ";tok=" );
		bufidx = strlen(sipbuf);
		bufidx += iGet_LocDir_ver2( sipbuf+bufidx, field[idx] );
	}
	
	sipbuf[bufidx++] = 0x0D;
	sipbuf[bufidx++] = 0x0A;
	sipbuf[bufidx] = 0;

	if( len != idx )
	{
		dAppLog( LOG_WARN, "[analib:%s] check field & analyzing..len[%d] idx[%d]",
			   __FUNCTION__ , len, idx);
		log_hexa( field, len );
	}

	return strlen(sipbuf);
}

int iMake_FromTo( st_tok_table *pstTok, unsigned char *field, int len, char *sipbuf )
{
	int idx=0, bufidx=0, iURICnt=0;
	st_bitfield stBF;
	st_bitfield_param stParamBF;

	// Invalid check
	if( ( field[idx] != 0x6C && field[idx] != 0x64 && field[idx] != 0x65 && field[idx] != 0x69 && field[idx] != 0x6D ) && len < 2 )
	{
		dAppLog( LOG_DEBUG, "[analib:%s] not indicator: 0x%02x / default len: %d", 
				__FUNCTION__, field[0], len );
		log_hexa( field, len );
	}

	if( field[idx] == 0x64 ) // To
	{
		sprintf( sipbuf+bufidx, "To: " );
	}
	else if( field[idx] == 0x65 ) // from
	{
		sprintf( sipbuf+bufidx, "From: " );
	}
	else if( field[idx] == 0x6D ) //Route
	{
		sprintf( sipbuf+bufidx, "Route: " );
	}
	else if(field[idx] == 0x69 ) // Contact
	{
		sprintf( sipbuf+bufidx, "Contact: " );
	}
	else if( field[idx] == 0x6C ) // Record-Route 
	{
		sprintf( sipbuf+bufidx, "Record-Route: ");
	}

	bufidx = strlen(sipbuf);
	idx++;

	SetBitField( &stBF, field[idx++] );


	// if flag indicate a display-name is present
	if( stBF.a == 1 )
	{
		dAppLog( LOG_WARN, "[analib:%s] display-name here!! [%s]", __FUNCTION__, field );
	}

	sipbuf[bufidx++] = '<';
	// sip scheme 
	if( stBF.bb == 0 )
	{
		sprintf( sipbuf+bufidx, "sip:" );
		bufidx = strlen(sipbuf);

		// two user parts(f=1) only sip sheme
		if( stBF.f == 1 )
		{
			bufidx += iGetString( sipbuf+bufidx, field[idx++], 0, pstTok );
		}
		
		if( field[idx] != 0 )
		{
			bufidx += iGetString( sipbuf+bufidx, field[idx++], 0, pstTok );
			sipbuf[bufidx++] = '@';
		}
		else
		{
			idx++;
		}

		bufidx += iGetString( sipbuf+bufidx, field[idx++], 0, pstTok );
		
		// specified port(c=1)
		if( stBF.c == 1 )
		{
			sprintf( sipbuf+bufidx, ":%d", iConvert_Array2INT16(field+idx) );
			idx += 2;
		}
	}
	// tel scheme
	else if( stBF.bb == 1 )
	{
		sprintf( sipbuf+bufidx, "tel:" );
		bufidx = strlen(sipbuf);

		bufidx += iGetString( sipbuf+bufidx, field[idx++], 0, pstTok );
	}
	else if( stBF.bb == 3 )
	{
		bufidx += iGetString( sipbuf+bufidx, field[idx++], 0, pstTok );
		sipbuf[bufidx++] = ':';	
		bufidx += iGetString( sipbuf+bufidx, field[idx++], 0, pstTok );
	}

	sipbuf[bufidx++] = '>';
	

	if( len > idx )
	{
		if( stBF.d == 1 )
		{
			iURICnt =(unsigned int)field[idx++];
		}

		sipbuf[bufidx++] = ';';
		if( stBF.e == 1 )
		{
			sprintf( sipbuf+bufidx, "tok=" );
			bufidx = strlen(sipbuf);
			bufidx += iGetString( sipbuf+bufidx, field[idx++], 0, pstTok );

			if( idx < len )
				sipbuf[bufidx++] = ';';
		}

		unsigned char ucPBF=0, uclvalue=0;

		while( idx < len )
		{
			if( field[idx] == 0x00 )
			{
			}
			else if( field[idx] == 0x01 )
			{
				bufidx += iGet_SipDic_ver1( sipbuf+bufidx, field[++idx], 0 );
				idx++;
			}
			else if( field[idx] == 0x02 )
			{
				bufidx += iGet_SipDic_ver1( sipbuf+bufidx, field[++idx]+255, 0 );
				idx++;
			}
			else if( field[idx] == 0x03 )
			{
				if( g_TokSipVer == 2 )
					bufidx += iGet_LocDir_ver2( sipbuf+bufidx, field[++idx] );
				else
					bufidx += iGet_LocDir_ver1( sipbuf+bufidx, field[++idx] );

				idx++;
			}
			else if( field[idx] == 0x04 )
			{
			}
			else if( field[idx] == 0x05 )
			{
				idx++;
				sprintf( sipbuf+bufidx, "%d", iConvert_Array2INT16( field+idx) );
				bufidx = strlen(sipbuf);
				idx += 2;
			}
			else if( field[idx] == 0x06 )
			{
				idx++;
				sprintf( sipbuf+bufidx, "%d", iConvert_Array2INT32( field+idx ) );
				bufidx = strlen(sipbuf);
				idx += 4;
			}
			else if( field[idx] >= 0x40 && field[idx] <= 0x4F )
			{
				SetParamBitField( &stParamBF, field[idx++] );
				ucPBF = 0x01;  // set Parameter 
				continue;

			}
			else if( field[idx] >= 0x80 && field[idx] <= 0x9F )
			{
				bufidx += iGetString( sipbuf+bufidx, field[idx++], 0, pstTok );
			}
			else if( field[idx] >= 0xA0 && field[idx] <= 0xBF )
			{
				bufidx += iGetString( sipbuf+bufidx, field[idx++], 0, pstTok );
			}
			else if( field[idx] >= 0xC0 && field[idx] <= 0xDF )
			{
				bufidx += iGetString( sipbuf+bufidx, field[idx++], 0, pstTok );
			}
			else
			{
				dAppLog( LOG_DEBUG, "[analib:%s] Not defined token [0x%02X]",
						__FUNCTION__, (unsigned char) field[idx] );
				idx ++;
			}

			if( ucPBF == 0x01 && uclvalue == 0x00 )
			{
				sipbuf[bufidx++] = '=';
				uclvalue = 0x01;

				if( stParamBF.b == 1 )
					sipbuf[bufidx++] = 0x22;
			}
			else if( ucPBF == 0x01 && uclvalue == 0x01 )
			{
				ucPBF = uclvalue = 0x00;

				if( stParamBF.b == 1 )
					sipbuf[bufidx++] = 0x22;

				if( idx < len )
					sipbuf[bufidx++] = ';';
			}
			else
			{
				idx++;
			}

		} // endof while 
	} // end of if  ... for parse parameters

	sipbuf[bufidx++] = 0x0D;
	sipbuf[bufidx++] = 0x0A;
	sipbuf[bufidx] = 0;

#ifdef DEBUG_DETAIL
	dAppLog( LOG_INFO,"[analib:%s]\n%s" , __FUNCTION__, sipbuf );
#endif

	if( idx != len )
	{
		dAppLog( LOG_WARN, "[analib:%s] Remain field information!! plz.. check this field", __FUNCTION__ );
		log_hexa( field, len );
	}

	return strlen(sipbuf);
}

int iMake_KnownHeader( st_tok_table *pstTok, unsigned char *field, int len, char *sipbuf )
{
	int idx=0, bufidx=0;

	// Invalid check
	if( (field[idx++] != 0x08 || field[idx] != 0x65 ) && len < 2 )
	{
		dAppLog( LOG_DEBUG, "[analib:%s] FAIL... not indicator: 0x%02x / default len: %d", 
				__FUNCTION__, field[0], len );
		log_hexa( field, len );
	}

	bufidx += iGet_HeaderNumber( sipbuf+bufidx, field[idx++] );
	sipbuf[bufidx++] = ':';
	sipbuf[bufidx++] = ' ';
	
	if( field[idx++] == 0x03 ) // Local strings 0...255 
	{
		bufidx += iGet_LocDir_ver2( sipbuf+bufidx, field[idx++] );
	}

	sipbuf[bufidx++] = 0x0D;
	sipbuf[bufidx++] = 0x0A;
	sipbuf[bufidx] = 0;

	if( idx != len )
	{
		dAppLog( LOG_WARN, "[analib:%s] Remain field information!! plz.. check this field", __FUNCTION__ );
		log_hexa( field, len );
	}


	return strlen(sipbuf);
}

int iMake_CommonlyUsedHeader( st_tok_table *pstTok, unsigned char *field, int len, char *sipbuf )
{
	int idx=0, bufidx=0;

	// Invalid check
	//if( (field[idx++] != 0x08 || field[idx] != 0x65 ) && len < 2 )
	if( ((unsigned char)field[idx] < 0xE0) && len < 2 )
	{
		dAppLog( LOG_DEBUG, "[analib:%s] FAIL not indicator: 0x%02x / default len: %d", 
				__FUNCTION__, field[0], len );
		log_hexa( field, len );
	}


	bufidx += iGet_HeaderNumber( sipbuf+bufidx, field[idx++] );

	if( field[idx] == 0x00 )
	{
		if ( bufidx>=9 &&  strncmp(sipbuf, "Supported", 9 ) == 0 )
		{
			sipbuf[bufidx++] = ':';
			sipbuf[bufidx++] = ' ';
		}

		idx++;
	}
	else if( field[idx] == 0x01 )
	{
		sipbuf[bufidx++] = '=';
		sipbuf[bufidx++] = ' ';
		bufidx += iGet_SipDic_ver1( sipbuf+bufidx, field[++idx], 0 );
		idx++;
	}
	else if( field[idx] == 0x02 )
	{
		sipbuf[bufidx++] = ':';
		sipbuf[bufidx++] = ' ';
		bufidx += iGet_SipDic_ver1( sipbuf+bufidx, field[++idx]+255, 0 );
		idx++;
	}
	else if( field[idx] == 0x03 )
	{
		sipbuf[bufidx++] = ':';
		sipbuf[bufidx++] = ' ';
		if( g_TokSipVer == 2 )
			bufidx += iGet_LocDir_ver2( sipbuf+bufidx, field[++idx] );
		else
			bufidx += iGet_LocDir_ver1( sipbuf+bufidx, field[++idx] );

		idx++;
	}
	else if( field[idx] == 0x04 )
	{
		idx++;
	}
	else if( field[idx] == 0x05 )
	{
		sipbuf[bufidx++] = ':';
		sipbuf[bufidx++] = ' ';
		idx++;
		sprintf( sipbuf+bufidx, "%u", iConvert_Array2INT16( field+idx) );
		bufidx = strlen(sipbuf);
		idx += 2;
	}
	else if( field[idx] == 0x06 )
	{
		sipbuf[bufidx++] = ':';
		sipbuf[bufidx++] = ' ';
		idx++;
		sprintf( sipbuf+bufidx, "%u", iConvert_Array2INT32( field+idx ) );
		bufidx = strlen(sipbuf);
		idx += 4;
	}
	else if( field[idx] >= 0x40 && field[idx] <= 0x4F )
	{
	}
	else if( field[idx] >= 0x80 && field[idx] <= 0x9F )
	{
		sipbuf[bufidx++] = ':';
		sipbuf[bufidx++] = ' ';
		bufidx += iGetString( sipbuf+bufidx, field[idx++], 0, pstTok );
	}
	else if( field[idx] >= 0xA0 && field[idx] <= 0xBF )
	{
		sipbuf[bufidx++] = ':';
		sipbuf[bufidx++] = ' ';
		bufidx += iGetString( sipbuf+bufidx, field[idx++], 0, pstTok );
	}
	else if( field[idx] >= 0xC0 && field[idx] <= 0xDF )
	{
		sipbuf[bufidx++] = ':';
		sipbuf[bufidx++] = ' ';
		bufidx += iGetString( sipbuf+bufidx, field[idx++], 0, pstTok );
	}
	else
	{
		dAppLog( LOG_DEBUG, "[analib:%s] Not defined token [0x%02X]",
				__FUNCTION__, (unsigned char) field[idx] );
		idx ++;
	}

	sipbuf[bufidx++] = 0x0D;
	sipbuf[bufidx++] = 0x0A;
	sipbuf[bufidx] = 0;
	
	if( idx != len )
	{
		dAppLog( LOG_WARN, "[analib:%s] Remain field information!! plz.. check this field[idx:%d len:%d]", __FUNCTION__, idx, len );
		log_hexa( field, len );
	}

#ifdef DETAIL
	dAppLog( LOG_INFO, "[analib:%s]\n%s", __FUNCTION__, sipbuf );
#endif


	return strlen(sipbuf);
}

int iGetString( char *buf, const unsigned char code, short index, st_tok_table *pstTok )
{
	int iStrGrpNo = 0;   // String Group Number
	int ret;

	iStrGrpNo = code & 0xF0;

	//if( code >= 0xE0 && code <= 0xFF )
	if( code >= 0xE0 )
	{
		iGet_HeaderNumber( buf, code );
	}
	else if( code >= 0xC0 && code <= 0xDF && index == 0 )
	{
		iGet_SipDic_ver1( buf, (short)code, 1 );
	}
	else if( code >= 0xA0 && code <= 0xBF )
	{
		if (g_TokSipVer == 2 )
			iGet_LocDir_ver2( buf, code );
		else
			iGet_LocDir_ver1( buf, code );
	}
	else if( code >= 0x80 && code <= 0x9F )
	{
		ret = sprintf( buf, pstTok->szString[code-0x80] );
	}
	else if( code == 0 && index <= 511 ) // Sip directory string index lange 0~511
	{
		iGet_SipDic_ver1( buf, index, 0 );
	}
	else
	{
		dAppLog( LOG_DEBUG, "[analib:%s] Unknown string code[0x%02X]", __FUNCTION__, (unsigned char)code );
	}

	return strlen(buf);
}

int iMake_UnknownHeader( st_tok_table *pstTok, unsigned char *field, int len, char *sipbuf )
{
	int idx=0, bufidx=0, rvalue=0;

	// Invalid check
	if( field[idx++] != 0x09 && len < 2 )
	{
		dAppLog( LOG_DEBUG, "[analib:%s] FAIL.. not indicator: 0x%02x / default len: %d", 
				__FUNCTION__, field[0], len );
		log_hexa( field, len );
	}


	while( idx < len )
	{

		if( field[idx] == 0x00 )
		{
			dAppLog( LOG_DEBUG, "[analib:%s] Unknown Field.. please check this message.. field[0x%02x]", 
					__FUNCTION__ , field[idx]);
			idx++;
		}
		else if( field[idx] == 0x01 )
		{
			bufidx += iGet_SipDic_ver1( sipbuf+bufidx, field[++idx], 0 );
			idx++;
		}
		else if( field[idx] == 0x02 )
		{
			bufidx += iGet_SipDic_ver1( sipbuf+bufidx, field[++idx]+255, 0 );
			idx++;
		}
		else if( field[idx] == 0x03 )
		{
			if( g_TokSipVer == 2 )
				bufidx += iGet_LocDir_ver2( sipbuf+bufidx, field[++idx] );
			else
				bufidx += iGet_LocDir_ver1( sipbuf+bufidx, field[++idx] );

			idx++;
		}
		else if( field[idx] == 0x04 )
		{
			dAppLog( LOG_DEBUG, "[analib:%s] Indexed Message String", __FUNCTION__ );
			log_hexa( field, len );
			idx ++;
		}
		else if( field[idx] == 0x05 )
		{
			idx++;
			sprintf( sipbuf+bufidx, "%u", iConvert_Array2INT16( field+idx) );
			bufidx = strlen(sipbuf);
			idx += 2;
		}
		else if( field[idx] == 0x06 )
		{
			idx++;
			sprintf( sipbuf+bufidx, "%u", iConvert_Array2INT32( field+idx ) );
			bufidx = strlen(sipbuf);
			idx += 4;
		}
		else if( field[idx] >= 0x40 && field[idx] <= 0x4F )
		{
			dAppLog( LOG_DEBUG, "[analib:%s] Unknown Field.. please check this message.. field[0x%02x]", 
					__FUNCTION__ , field[idx]);
			idx++;
		}
		else if( field[idx] >= 0x80 && field[idx] <= 0x9F )
		{
			bufidx += iGetString( sipbuf+bufidx, field[idx++], 0, pstTok );
		}
		else if( field[idx] >= 0xA0 && field[idx] <= 0xBF )
		{
			bufidx += iGetString( sipbuf+bufidx, field[idx++], 0, pstTok );
		}
		else if( field[idx] >= 0xC0 && field[idx] <= 0xDF )
		{
			bufidx += iGetString( sipbuf+bufidx, field[idx++], 0, pstTok );
		}
		//else if( field[idx] >= 0xE0 && field[idx] <= 0xFF )
		else if( field[idx] >= 0xE0 )
		{
			bufidx += iGetString( sipbuf+bufidx, field[idx++], 0, pstTok );
		}
		else
		{
			dAppLog( LOG_DEBUG, "[analib:%s] Not defined token [0x%02X]",
					__FUNCTION__, (unsigned char) field[idx] );
			idx ++;
		}

		if( rvalue == 0 )
		{
			sipbuf[bufidx++] = ':';
			sipbuf[bufidx++] = ' ';
			rvalue = 1;
		}

	} // end of while

	sipbuf[bufidx++] = 0x0D;
	sipbuf[bufidx++] = 0x0A;
	sipbuf[bufidx] = 0x00;
	
	if( idx != len )
	{
		dAppLog( LOG_WARN, "[analib:%s] Remain field information!! plz.. check this field", __FUNCTION__ );
		log_hexa( field, len );
	}

#ifdef DETAIL
	dAppLog( LOG_INFO, "[analib:%s]\n%s", __FUNCTION__,  sipbuf );
#endif


	return strlen(sipbuf);
}


int iMake_MultiPartSDPStart( st_tok_table *pstTok, unsigned char *field, int len, char *sipbuf )
{
	int idx=0, bufidx=0;
	st_bitfield_sdp stBF;		// Bitfield for Field Inclusions and Default flag, SDP-Start Token
	char szOrigName[32];		// for Origin username
	unsigned int uiOrigSessID;	// for Origin Session Id 16~32bits
	unsigned short usSDPVer;	// SDP Version

	// Invalid check
	if( field[idx++] != 0x72 && len < 3 )
	{
		dAppLog( LOG_DEBUG, "[analib:%s] FAIL... not indicator: 0x%02x / default len: %d", 
				__FUNCTION__, field[0], len );
		log_hexa( field, len );
	}

	SetBitField_SDP( &stBF, field[idx++], 0 );
	SetBitField_SDP( &stBF, field[idx++], 1 );

	// set Origin Name
	iGetString( szOrigName, field[idx++], 0, pstTok );

	// set Origin Session ID
	if( field[idx++] == 0x05 )
	{
		uiOrigSessID = iConvert_Array2INT16( field+idx );
		idx += 2;
	}
	else 
	{
		uiOrigSessID = iConvert_Array2INT32( field+idx );
		idx += 4;
	}

	/* **  Make 'v' field ** */
	if( stBF.j == 0 )
	{
		sprintf( sipbuf+bufidx, "v=0%c%c%c", 0x0D, 0x0A, 0x00 );
		bufidx = strlen(sipbuf);
		usSDPVer = 0;
	}
	else
	{
		dAppLog( LOG_WARN, "[analib:%s] SDP Version is not 0", __FUNCTION__ );
		log_hexa( field, len );

		return -1;
	}
	

	/* **  Make 'o' field ** */
	// o field address type is "IN IP4"
	if( stBF.k == 0 )
	{
		sprintf( sipbuf+bufidx, "o=%s %d %d IN IP4 ", szOrigName, uiOrigSessID, usSDPVer );
		bufidx = strlen(sipbuf);
	}
	else 
	// o field address type is in a string
	{
		sprintf( sipbuf+bufidx, "o=%s %d %d ", szOrigName, uiOrigSessID, usSDPVer );
		bufidx = strlen(sipbuf);
	
		// Get network type and address type 	
		bufidx += iGetString( sipbuf+bufidx, field[idx++], 0, pstTok );
	}

	//Get Orign IP
	bufidx += iGetString( sipbuf+bufidx, field[idx++], 0, pstTok );
	sipbuf[bufidx++] = 0x0D;
	sipbuf[bufidx++] = 0x0A;

	/* ** Make 's' field ** */
	sprintf( sipbuf+bufidx, "s=" ); 
	bufidx += 2;
	bufidx += iGetString( sipbuf+bufidx, field[idx++], 0, pstTok );
	sipbuf[bufidx++] = 0x0D;
	sipbuf[bufidx++] = 0x0A;


	/* ** Make 'i' field ** */
	if( stBF.a == 1 )
	{
		sprintf( sipbuf+bufidx, "i=" );
		bufidx += 2;

		bufidx += iGetString( sipbuf+bufidx, field[idx++], 0, pstTok );
		sipbuf[bufidx++] = 0x0D;
		sipbuf[bufidx++] = 0x0A;
	}

	/* ** Make 'u' field ** */
	if( stBF.b == 1 )
	{
		dAppLog( LOG_DEBUG, "[analib:%s] check u field", __FUNCTION__ );
		idx ++;
	}

	/* ** Make 'e' field ** */
	if( stBF.c == 1 )
	{
		sprintf( sipbuf+bufidx, "e=" );
		bufidx += 2;

		bufidx += iGetString( sipbuf+bufidx, field[idx++], 0, pstTok );
		sipbuf[bufidx++] = 0x0D;
		sipbuf[bufidx++] = 0x0A;
	}

	/* ** Make 'p' field ** */
	if( stBF.d == 1 )
	{
		sprintf( sipbuf+bufidx, "p=" );
		bufidx += 2;

		bufidx += iGetString( sipbuf+bufidx, field[idx++], 0, pstTok );
		sipbuf[bufidx++] = 0x0D;
		sipbuf[bufidx++] = 0x0A;
	}

	/* ** Make 'c' field ** */
	if( stBF.e == 1 )
	{
		sprintf( sipbuf+bufidx, "c=" );
		bufidx += 2;

		if( stBF.m == 0 )
		{
			sprintf( sipbuf+bufidx, "IN IP4 " );
			bufidx = strlen(sipbuf);
		}
		else
		{
			bufidx += iGetString( sipbuf+bufidx, field[idx++], 0, pstTok );
			sipbuf[bufidx++] = ' ';
		}

		bufidx += iGetString( sipbuf+bufidx, field[idx++], 0, pstTok );
		sipbuf[bufidx++] = 0x0D;
		sipbuf[bufidx++] = 0x0A;
	}

	/* ** Make 'b' field ** */
	if( stBF.f == 1 )
	{
		sprintf( sipbuf+bufidx, "b=" ); 
		bufidx += 2;
	
		// get bandwidth type
		bufidx += iGetString( sipbuf+bufidx, field[idx++], 0, pstTok );
		
		// bandwidth value
		if( field[idx++] == 0x05 )
		{
			sprintf( sipbuf+bufidx, "%d%c%c", iConvert_Array2INT16( field+idx ), 0x0D, 0x0A );
			idx += 2;
		}
		else if( field[idx++] == 0x06 )
		{
			sprintf( sipbuf+bufidx, "%d%c%c", iConvert_Array2INT32( field+idx ), 0x0D, 0x0A );
			idx += 4;
		}

		bufidx = strlen(sipbuf);
		dAppLog( LOG_DEBUG, "[analib:%s] SDP bandwidth field...[%s]", __FUNCTION__, sipbuf);
		log_hexa( field, len );
	}

	/* ** Make 't' field ** */
	if( stBF.g == 1 )
	{
		if( stBF.n == 0 )
		{
			sprintf( sipbuf+bufidx, "t=0 0%c%c", 0x0D, 0x0A );
			bufidx = strlen(sipbuf);
		}
		else
		{
			sprintf( sipbuf+bufidx, "t=" );
			bufidx += 2;
		
			int start=0, end=1;
			for( start = 0; start <= end; start++ )
			{
				if( field[idx] == 0x05 )
				{
					idx++;
					sprintf( sipbuf+bufidx, "%d", iConvert_Array2INT16( field+idx ) );
					idx += 2;

				}
				else if( field[idx] == 0x06 )
				{
					idx++;
					sprintf( sipbuf+bufidx, "%d", iConvert_Array2INT32( field+idx ) );
					idx += 4;
				}
				else
				{
					bufidx += iGetString( sipbuf+bufidx, field[idx++], 0, pstTok );
				}

				bufidx = strlen(sipbuf);
				if( start == 0 )
					sipbuf[bufidx++] = ' ';
			} // end of for
		}

		sipbuf[bufidx++] = 0x0D;
		sipbuf[bufidx++] = 0x0A;

	}

	sipbuf[bufidx] = 0x00;

	
	if( idx != len )
	{
		dAppLog( LOG_WARN, "[analib:%s] Remain field information!! plz.. check this field", __FUNCTION__ );
		log_hexa( field, len );
	}

#ifdef DETAIL
	dAppLog( LOG_INFO, "[analib:%s]\n%s\n", __FUNCTION__, sipbuf );
#endif


	return strlen(sipbuf);
}

int iMake_MultiPartSDPMedia( st_tok_table *pstTok, unsigned char *field, int len, char *sipbuf )
{
	int idx=0, bufidx=0;
	st_bitfield_sdp stBF;		// Bitfield for Field Inclusions and Default flag, SDP-Start Token
	int iAttrCnt;				// Number of media attribute lines

	int i;

	// Invalid check
	if( field[idx++] != 0x75 && len < 4 )
	{
		dAppLog( LOG_DEBUG, "[analib:%s] FAIL..not indicator: 0x%02x / default len: %d", 
				__FUNCTION__, field[0], len );
		log_hexa( field, len );
		return 0;
	}

	SetBitField_SDP( &stBF, field[idx++], 0 );

	sprintf( sipbuf+bufidx, "m=" );
	bufidx += 2;

	if( field[idx] == 0x01 )
		bufidx += iGetString( sipbuf+bufidx, 0, (short)field[++idx], pstTok );
	else if( field[idx] == 0x02 )
		bufidx += iGetString( sipbuf+bufidx, 0, ((short)field[++idx]+255), pstTok );
	else if( field[idx] == 0x03 )
		bufidx += iGetString( sipbuf+bufidx, field[++idx], 0, pstTok );
	else 
	{
		bufidx += iGetString( sipbuf+bufidx, field[idx], 0, pstTok );
	}

	idx++;

	sprintf( sipbuf+bufidx, " %d", iConvert_Array2INT16( field+idx ) );
	bufidx = strlen(sipbuf);
	idx += 2;

	/* If field indication flags indicate that the number of media ports is specified.
	 * bitfield b=1
	 */
	if( stBF.b == 1 )
	{
		dAppLog( LOG_DEBUG, "[analib:%s] SDP media .. the number of media ports... ", __FUNCTION__ );
		sprintf( sipbuf+bufidx, " %d", iConvert_Array2INT16( field+idx ) );
		bufidx = strlen(sipbuf);
		idx += 2;
	}

	// Always included... Media transport
	sipbuf[bufidx++] = ' ';
	bufidx += iGetString( sipbuf+bufidx, field[idx++], 0, pstTok );

	// If field inclusion flags indicate more than one formats(d=1). otherwise, there is  one format.
	// Number of format lines 
	if( stBF.d == 1 )
	{
		dAppLog( LOG_DEBUG, "[analib:%s] SDP Number of format lines...", __FUNCTION__ );
		sprintf( sipbuf+bufidx, "/%d", (unsigned short)field[idx++] );
		bufidx += strlen(sipbuf);
	}

	// the following entry is repeated one more times, based on the number of ports( 1 or specified)
	// Media format
	sipbuf[bufidx++] = ' ';
	bufidx += iGetString( sipbuf+bufidx, field[idx++], 0, pstTok );

	// if field inclusion flags indicate an i= field
	// bitfield a=1
	// Information text
	if( stBF.a == 1 )
	{

		dAppLog( LOG_DEBUG, "[analib:%s] SDP information text..", __FUNCTION__ );

		sprintf( sipbuf+bufidx, "%c%ci=", 0x0D, 0x0A );
		bufidx += strlen(sipbuf);

		if( field[idx] == 0x01 )
			bufidx += iGetString( sipbuf+bufidx, 0, (short)field[++idx], pstTok );
		else if( field[idx] == 0x02 )
			bufidx += iGetString( sipbuf+bufidx, 0, ((short)field[++idx]+255), pstTok );
		else if( field[idx] == 0x03 )
			bufidx += iGetString( sipbuf+bufidx, field[++idx], 0, pstTok );
		else 
		{
			bufidx += iGetString( sipbuf+bufidx, field[idx], 0, pstTok );
		}
		idx++;
	}

	// if field inclusion flags indicate a 'c=' field and address type is flagged as mon IP4
	// field e=1 and c=1
	if( stBF.e == 1 )
	{
		sprintf( sipbuf+bufidx, "%c%cc=", 0x0D, 0x0A );
		bufidx = strlen(sipbuf);

		if( stBF.c == 0 )
		{
			sprintf( sipbuf+bufidx, "IN IP4" );
			bufidx = strlen(sipbuf);
		}
		else   // c field address type is in a string
		{
			bufidx += iGetString( sipbuf+bufidx, field[idx++], 0, pstTok );
		}

		// Connection IP address
		bufidx += iGetString( sipbuf+bufidx, field[idx++], 0, pstTok );

		sipbuf[bufidx++] = 0x0D;
		sipbuf[bufidx++] = 0x0A;
	}

	// if field inclusion flags indicate a 'b=' field (f=1)
	// Bandwidth.. string and value, using 16 or 32bits
	if( stBF.f == 1 )
	{
		sprintf( sipbuf+bufidx, "b=" ); 
		bufidx += 2;
	
		// get bandwidth type
		bufidx += iGetString( sipbuf+bufidx, field[idx++], 0, pstTok );
		
		// bandwidth value
		if( field[idx++] == 0x05 )
		{
			sprintf( sipbuf+bufidx, "%d%c%c", iConvert_Array2INT16( field+idx ), 0x0D, 0x0A );
			idx += 2;
		}
		else if( field[idx++] == 0x06 )
		{
			sprintf( sipbuf+bufidx, "%d%c%c", iConvert_Array2INT32( field+idx ), 0x0D, 0x0A );
			idx += 4;
		}

		bufidx = strlen(sipbuf);
		dAppLog( LOG_DEBUG, "[analib:%s] SDP bandwidth field...[%s]", __FUNCTION__, sipbuf);
		log_hexa( field, len );
	}

	// if field inclusion flags indicate a 'k=' field(g=1)
	// about Encryption ... method and value
	if( stBF.g == 1 )
	{

		dAppLog( LOG_DEBUG, "[analib:%s] SDP Encryption.", __FUNCTION__ );

		sprintf( sipbuf+bufidx, "%c%ck=", 0x0D, 0x0A );
		bufidx += strlen(sipbuf);

		for( i=0; i < 2; i++ )
		{
			if( field[idx] != 0x00 && i == 1 )
			{
				sipbuf[bufidx++] = ':';
			}

			if( field[idx] == 0x01 )
				bufidx += iGetString( sipbuf+bufidx, 0, (short)field[++idx], pstTok );
			else if( field[idx] == 0x02 )
				bufidx += iGetString( sipbuf+bufidx, 0, ((short)field[++idx]+255), pstTok );
			else if( field[idx] == 0x03 )
				bufidx += iGetString( sipbuf+bufidx, field[++idx], 0, pstTok );
			else 
			{
				bufidx += iGetString( sipbuf+bufidx, field[idx], 0, pstTok );
			}
			idx++;
		} // end of for
	}

	// if field inclusion flags indicate are one moe media attribute lines (h=1)
	// if field inclusion flags indicate session attributes 
	// ane the number of session attributes is >0, the next two fields are repeated( in pairs)
	if( stBF.h == 1 )
	{
		iAttrCnt = (unsigned short) field[idx++];
		sprintf( sipbuf+bufidx, "%c%ca=", 0x0D, 0x0A );
		bufidx = strlen(sipbuf);

		// Get each Attribute line
		for( i=0; i < iAttrCnt; i++ )
		{
			int j;
			for( j=0; j<2; j++ )
			{
				if( field[idx] == 0x00 )  // null
				{
					if( j%2 == 0 ) //attributes name
					{
						dAppLog( LOG_DEBUG, "[analib:%s] Attribute name is null", __FUNCTION__ );
						log_hexa( field, len );
						break;
					}
				}
				else
				{
					if( j%2 != 0 )
						sipbuf[bufidx++] = ':';

					if( field[idx] == 0x01 )
						bufidx += iGetString( sipbuf+bufidx, 0, (short)field[++idx], pstTok );
					else if( field[idx] == 0x02 )
						bufidx += iGetString( sipbuf+bufidx, 0, ((short)field[++idx]+255), pstTok );
					else if( field[idx] == 0x03 )
						bufidx += iGetString( sipbuf+bufidx, field[++idx], 0, pstTok );
					else 
					{
						bufidx += iGetString( sipbuf+bufidx, field[idx], 0, pstTok );
					}
				}
				idx++;
			} // end of for j
		} // end of for i
	}

	sipbuf[bufidx++] = 0x0D;
	sipbuf[bufidx++] = 0x0A;
	sipbuf[bufidx] = 0x00;

	if( idx != len )
	{
		dAppLog( LOG_WARN, "[analib:%s] Remain field information!! plz.. check this field[idx:%d/len:%d]",
				__FUNCTION__, idx, len );
		log_hexa( field, len );
	}

	return strlen(sipbuf);
}


/* ************************************************** 
 * $Log
 * ************************************************** */

