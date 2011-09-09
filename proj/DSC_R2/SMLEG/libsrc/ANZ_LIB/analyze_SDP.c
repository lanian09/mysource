/* **************************************************
 * $Id: analyze_SDP.c,v 1.1.1.1 2011/04/19 14:13:42 june Exp $
 *
 * Copyright (c) 2006 by uPresto Inc., KOREA
 * All rights reserved.
 * ************************************************** */

/**
 @file		: ptt_sdp.c
 @brief		: SDP 분석을 위한 함수들. A_VOD 의 SDP 분석 모듈을 포팅
 @date		: 2006.04.19. Initial
 @author	: syhan@upresto.com
 */

#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include <utillib.h>
#include "Analyze_Ext_Abs.h"
#include <define.h>

#define SDP_DESC_SESSION    1
#define SDP_DESC_TIME       2
#define SDP_DESC_MEDIA      3

/**
 @brief     SDP(Session Description Protocol) 분석 함수 
 
 SDP 프로토콜을 분석한다. \n
 SDP 는 RTSP/SIP의 바디 부분에 포함되어져 있다. \n

 SDP 를 분석하여 각 SIP 정보 구조체를 채운다. 	\n
 해당 함수는 A_VOD 의 SDP 분석 부분을 수정한 것이다. \n
 
 @return   void
 */
void ParseSDP( char *szData, int iDataLen, st_sipinfo *pstSIP )
{
	int isend;
	char *line_end, *line_start;
	char *sdp_end = szData+iDataLen;

	// SIP 에서 Content-Type, Content-Length 정보 없는 경우 있음.  
	if( iDataLen == 0 )
	{
		sdp_end = szData+strlen(szData);
	}

	line_start = szData;

	isend = 0;

	do
	{
		line_end = pGetLineEnd(line_start);
		if( line_end == NULL )
		{
			isend = 1;
			line_end = sdp_end;
		}

		ParseSDPLine( szData, line_start, line_end, pstSIP );
		line_start = line_end+2;
	} while( isend == 0 && line_start < sdp_end );
}



/**
 @brief     SDP(Session Description Protocol) 각 라인 분석
 
 SDP 프로토콜의 각 라인을 분석한다. \n
 SDP 는 RTSP/SIP의 바디 부분에 포함되어져 있다. \n

 SDP 를 분석하여 각 SIP 정보 구조체를 채운다. 	
 
 @return   void
 */
void ParseSDPLine( char *szData,  char *start, char *end, st_sipinfo *pstSIP )
{
	int dLineLen = end - start;
	char *p;
	char tmpbuf[128];
	int len;

	if( dLineLen < 2 )
	{
		if( dLineLen > 0 )
		{
			dAppLog(LOG_CRI, "ParsePTTSDPLine(%s:%d): SDP line[%s]",
					__FILE__, __LINE__, start);
		}
		return;
	}

	if( strncmp(start, "v=", 2) == 0 )
	{
		// Session description
		pstSIP->ucSDPDescType = SDP_DESC_SESSION;
		start += 2;
#ifdef DEBUG_DETAIL
		dAppLog( LOG_INFO, "SDP v[%s]", start );
#endif

	}
	else if( strncmp(start, "t=", 2) == 0 )
	{
		// Time description
		pstSIP->ucSDPDescType = SDP_DESC_TIME;
#ifdef DEBUG_DETAIL
		dAppLog( LOG_INFO, "SDP t[%s]", start );
#endif
		start += 2;
	}
	else if( strncmp(start, "m=", 2) == 0 )
	{
		// Media description
		char *space;
		int len=0;

		pstSIP->ucSDPDescType = SDP_DESC_MEDIA;

		// get media type
		start += 2;
		space = strchr(start, ' ');
		if( space != NULL )
		{
			if( (len = space -start) > MAX_MEDIA_SIZE ) 
				len = MAX_MEDTYPE_SIZE;

			memcpy( pstSIP->szMedType, start, len);
			pstSIP->szMedType[len] = 0;
			//pstSIP->usMediaCnt++;
		}

		// get port
		start += len+1;
		space = strchr( start, ' ' );
		if( space != NULL )
		{
			len = space - start;
			memcpy( tmpbuf, start, len );
			tmpbuf[len] = 0;

/* ADDED BY LYH 07.05.02 */
			if( strncasecmp( pstSIP->szMedType, "audio", 5 ) == 0 ) {
				pstSIP->usAudioPort = atoi(tmpbuf);
			} else if ( strncasecmp( pstSIP->szMedType, "video", 5 ) == 0 ) {
				pstSIP->usVideoPort = atoi(tmpbuf);
			}
		}

		// get media protocol
		space = strchr( start, ' ' );
		start = space+1;
		space = strchr( start, ' ' );
		if( space != NULL )
		{
			len = space -start; 
			memcpy( pstSIP->szMedProto, start, len );
			pstSIP->szMedProto[len] = 0;
		}

		//get Media format
		start += len+1;
		len = end - start;
		if( len > 0 )
		{
			memcpy( tmpbuf, start, len );
			pstSIP->usMedFormat = atoi(tmpbuf);
		}

	}
	else if( strncmp(start, "a=", 2) == 0 )
	{
		start += 2;

		if( pstSIP->ucSDPDescType == SDP_DESC_SESSION ) // session attribute
		{
			// get TimeRange
			if( strncmp(start, "range:npt=", 6) == 0 )
			{
				start += 6;
				// pstSIP->dTimeRange = dGetTimeRange(start); Not Use SIP
			}
		}
		else if( pstSIP->ucSDPDescType == SDP_DESC_MEDIA ) // media attribute
		{
			// get trackID
			if(	strncmp(start, "control:trackID=", 16) == 0 )
			{
				start += 16;
				//pstSIP->sTrackID[pstSIP->usMediaCnt-1] = atoi(start); Not Use SIP
			}
		}
	}
	else if( strncmp( start, "c=", 2 ) == 0 )
	{
		if( (p = strstr( start, "IP4" )) != 0 )
		{	
			len = end-(p+4);
			memcpy( tmpbuf, p+4, len );
			tmpbuf[len] = 0;
			pstSIP->uiMedIP = ntohl(inet_addr(tmpbuf));
		}
	}
	else
	{
		if( *(start+1) != '=' )
		{	
			dAppLog(LOG_CRI, "ParsePTTSDPLine(%s:%d): SDP line[%s]",
					__FILE__, __LINE__, start);
			return;
		}
	}
}

char *pGetLineEnd(char *line_start)
{
	char szCRLF[3] = { 0x0D, 0x0A, 0 };

	return strstr(line_start, szCRLF);
}



/* ************************************************** 
 * $Log
 * ************************************************** */
