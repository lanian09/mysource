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

#include <string.h>
#include <stdlib.h>

#include <utillib.h>

#include "Analyze_Ext_Abs.h"
#include <define.h>

void SetBitField_SDP( st_bitfield_sdp *pstSDP, const unsigned char field, int flag )
{
	if( flag == 0 ) // field inclusion flags
	{
		// set a
		if( (field & 0x01) == 0x01 )
			pstSDP->a = 1;
		else
			pstSDP->a = 0;

		// set b
		if( (field & 0x02) == 0x02 )
			pstSDP->b = 1;
		else
			pstSDP->b = 0;

		// set c
		if( (field & 0x04) == 0x04 )
			pstSDP->c = 1;
		else
			pstSDP->c = 0;
	
		// set d
		if( (field & 0x08) == 0x08 )
			pstSDP->d = 1;
		else
			pstSDP->d = 0;
	
		// set e
		if( (field & 0x10) == 0x10 )
			pstSDP->e = 1;
		else
			pstSDP->e = 0;
	
		// set f
		if( (field & 0x20) == 0x20 )
			pstSDP->f = 1;
		else
			pstSDP->f = 0;

		// set g
		if( (field & 0x40) == 0x40 )
			pstSDP->g = 1;
		else
			pstSDP->g = 0;
	
		// set h
		if( (field & 0x80) == 0x80 )
			pstSDP->h = 1;
		else
			pstSDP->h = 0;
	}
	else
	{
		// set j 
		if( (field & 0x01) == 0x01 )
			pstSDP->j = 1;
		else
			pstSDP->j = 0;

		// set k 
		if( (field & 0x02) == 0x02 )
			pstSDP->k = 1;
		else
			pstSDP->k = 0;

		// set m
		if( (field & 0x04) == 0x04 )
			pstSDP->m = 1;
		else
			pstSDP->m = 0;
	
		// set n
		if( (field & 0x08) == 0x08 )
			pstSDP->n = 1;
		else
			pstSDP->n = 0;
	
		// set p
		if( (field & 0x10) == 0x10 )
			pstSDP->p = 1;
		else
			pstSDP->p = 0;
	
		// set q 
		if( (field & 0x20) == 0x20 )
			pstSDP->q = 1;
		else
			pstSDP->q = 0;

		// set r
		if( (field & 0x40) == 0x40 )
			pstSDP->r = 1;
		else
			pstSDP->r = 0;
	
		// set s 
		if( (field & 0x80) == 0x80 )
			pstSDP->s = 1;
		else
			pstSDP->s = 0;
	}
	
	return;
}

int iIsSDPHdrToken( const unsigned char tok )
{
	    if( tok >= 0x71 && tok <= 0x75 )
	        return 1;
	    else    
	        return 0;
}

int iGetSDPTokField( char *data, int datalen, char *tokfield, int deflen )
{
	int len=0;

	if( 0 > iIsSDPHdrToken(data[len]) )
	{
		dAppLog( LOG_INFO, "[%s]Unknown Header field [0x%02x]", __FUNCTION__, data[len] );
		tokfield[len]=0;
		return len;
	}

	memcpy( tokfield, data, deflen );
	len += deflen;

	while( datalen > len )
	{
		if( (len > 0) && len >= deflen && iIsSDPHdrToken(data[len]) )
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

int iGetParamString( char *buf, char *field, st_tok_table *pstTok )
{
	st_bitfield_param stBF;
	int idx=0;

	SetParamBitField( &stBF, field[0] );

	if( stBF.aa == 0 )
		buf[idx++] = ';';
	else if( stBF.aa == 1 )
		buf[idx++] = ',';
	else if( stBF.aa == 2 )
		buf[idx++] = '?';   // 확인 필요 '&' 올 수도 있음.   
	else   // stBF.aa == 3 (0x11) is reserved..
	{
		dAppLog( LOG_DEBUG, "[%s] check parameter field ... [0x%02X]", __FUNCTION__, field[0] );
		buf[0] = 0;
		return 0;
	}

	idx += iGetString( buf+idx, field[1], 0, pstTok );

	if( field[2] != 0 )
	{
		buf[idx++] = '=';
		idx += iGetString( buf+idx, field[2], 0, pstTok );
	}

	return strlen(buf);

}
