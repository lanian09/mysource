/* **************************************************
 * $Id: analyze_SIP.c,v 1.1.1.1 2011/04/19 14:13:42 june Exp $
 *
 * Copyright (c) 2006 by uPresto Inc., KOREA
 * All rights reserved.
 * ************************************************** */

/**
 @file		: ptt_sip.c
 @brief		: SIP �м��� ���� �Լ���. 
 @date		: 2006.04.19. Initial
 @author	: syhan@upresto.com
 */

#include <string.h>
#include <stdlib.h>
#include <utillib.h>

#include "Analyze_Ext_Abs.h"
#include <define.h>

#define MAX_LINE_SIZE	128 // SIP Header �� ������ �ִ� size
#define MAX_TMPBUF_SIZE	64  // �ӽ÷� ����� ������ size
#define _FALSE			0
#define _TRUE			1
#define MAX_SIP_SIZE 6144

/**
 @brief     ParseSIP �Լ�
 
 SIP ���������� �м��Ѵ�. \n
 SIP Header�� Body�� �м��ϰ� Body�� ������ SDP�� ��� 
 SDP �м� ���� �����͸� �Ѱ��ش�. \n\n

 SIP/SDP �� �м��Ͽ� �� SIP ���� ����ü�� ä���. 	
 
 @return   void
 */

void Parse_PTTSIP( char *szData, int iDataSize, st_sipinfo *pstSIP )
{
	if( (unsigned char)szData[0] >= 0x80 )
	{
		/*
        dAppLog ( LOG_DEBUG, "[PARSE] TOKENIZED SIP - SKIP" );
        ParseTokSIP( szData, iDataSize, szSip );
        Parse_SIP( szSip, strlen(szSip), pstSIP );
        */

        /* 2008.09.16 MODIFIED BY LDH */
        log_hexa( szData, iDataSize );
        pstSIP->bSIP = 0;
	}
	else
	{
#ifdef DEBUG_DETAIL
		dAppLog( LOG_INFO, "[analib:%s] [#### NOT TOKENIZED SIP ####]", __FUNCTION__ );
#endif
		Parse_SIP( szData, iDataSize, pstSIP );
	}

	return;
}

void Parse_SIP( 
		char *szData,			//< ��Ŷ ������ 
		int iDataSize, 			//< ��Ŷ ������ 
		st_sipinfo *pstSIP
	)
{
	char *line_start, *line_end, *header_start, *header_end;
	int	iEnd, iHSize;
	//char szSip[MAX_SIP_SIZE];
	//int ret=0;

	// SIP�� ����� �ٵ� ���̿� CRLF�� �����Ѵ�. SIP�� Header/Body ������. 
	const char szDOUBLECRLF[5] = { 0x0D, 0x0A, 0x0D, 0x0A, 0 }; 

	pstSIP->bSIP = 1;

	/*
	if( (unsigned int)szData[0] >= 0x80 )
	{
		memset( szSip, 0x00, MAX_SIP_SIZE );
		ret = ParseTokSIP( szData, iDataSize,  szSip );
		if( ret < 0 )
		{
			//pstSIP->bSIP = 0;
			dAppLog( LOG_INFO, "[analib:%s] TokSip Parse Error", __FUNCTION__ );
			return;
		}

		memcpy( &szData[0], szSip, strlen(szSip) );
		iDataSize = strlen(szSip);
	}
	*/	


	line_start = header_start = szData; 
	header_end = strstr( szData, szDOUBLECRLF );
	if( header_end == 0)
	{
#ifdef DEBUG_DETAIL
		dAppLog( LOG_WARN, "[analib:%s] Abnormal SIP", __FUNCTION__ );
		dAppLog( LOG_WARN, "\n%s", szData );
#endif
		//log_hexa( szData, iDataSize );

		//pstSIP->bSIP = 0;
		//return;
		szData[strlen(szData)] = 0;
		header_end = header_start+strlen(szData);
	}

	iHSize = header_end - header_start;
	iEnd = _FALSE;

	do
	{
		line_end = pGetLineEnd( line_start );
		if( line_end == NULL )
		{
			iEnd = _TRUE;
			line_end = header_end;
		}

		Parse_PTTSIPHeader( pstSIP, szData, line_start, line_end );

		line_start = line_end + 2;
	} while( iEnd == _FALSE && line_start < header_end );

	// SDP�� ���� ��� SDP �м��� �Ѵ�. 
	//if( pstSIP->ucSDPFlag == _TRUE && iDataSize > header_end )
	//if( pstSIP->ucSDPFlag )
	//if( pstSIP->ucMethod == SIP_INVITE 
	//	&& ((pstSIP->ucMsgFlag == MSG_REQUEST) || (pstSIP->ucMsgFlag == MSG_RESPONSE && pstSIP->usStatusCode == 200))  )
	if( pstSIP->ucMethod == SIP_INVITE || (pstSIP->ucMsgFlag == MSG_RESPONSE && pstSIP->usStatusCode == 200) )
	{
		if( iDataSize > iHSize + 4 )
			ParseSDP( header_end+4, pstSIP->usContentLen, pstSIP );
	}

#if 0
	dAppLog( LOG_INFO, "[analib:%s] --------------------------- ", __FUNCTION__ );
	dAppLog( LOG_INFO, "[analib:%s] Method[0x%02x] From[%s] To[%s] Ver[%s] Via[Cnt:%d 1:%s, 2:%s, 3:%s] Expires[%d] Call-ID[%s] CSeq[%ld] ContentType[%s] ContentLength[%d] MaxForwards[%d] TimeStamp[%d] RequestURI[%s] StatusCode[%d] Reason[%s] SDP[Media Type:%s, Proto:%s, IP:%d/%d, Format:%d ] MsgFlag[0x%02X]", 
			__FUNCTION__,
			pstSIP->ucMethod, pstSIP->szFrom, pstSIP->szTo, pstSIP->szVersion, pstSIP->usViaCnt, 
			pstSIP->szVia[0], pstSIP->szVia[1], pstSIP->szVia[2], pstSIP->usExpires, 
			pstSIP->szCallID, pstSIP->uiCSeq, pstSIP->szContentType, pstSIP->usContentLen, 
			pstSIP->usMaxForwards, pstSIP->uiTimestamp, pstSIP->szRequestURI, 
			pstSIP->usStatusCode, pstSIP->szReason,
			pstSIP->szMedType, pstSIP->szMedProto, pstSIP->uiMedIP, pstSIP->usMedPort, pstSIP->usMedFormat,
			pstSIP->ucMsgFlag );
#endif

	return;

}

/**
 @brief		Parse_PTTSIPHeader

 SIP ����� �� ������ �м��Ͽ� st_sipinfo ����ü��
 �м� ������ �ִ´�. \n
 ���δ��� �м��� �ǽ��Ѵ�. 

 @return void 
 */
void Parse_PTTSIPHeader (
		pst_sipinfo pstSIP, 		//< SIP Information ����ü
		char *szData, 				//< SIP ��Ŷ
		char *start, char *end 		//< szData �� �м� �� �κ��� ���� �� ������		
	)
{
	const char szFIELD_DELIM[3] = { ':', ' ', 0 };  //RFC-3261 7.3.1
	//const unsigned char ucFIELD_TOKEN = ';';  unused
	//const unsigned char ucPARAM_TOKEN = '=';  unused
	const unsigned char ucSPACE = ' ';

	char szLine[MAX_LINE_SIZE];
	int iSize;
	char *parse_start, *parse_end;
	int len, i;
	char *quoted_start, *quoted_end, *field_start;

	char tmpbuf[MAX_LINE_SIZE];


	if( (end - start) > MAX_LINE_SIZE )
		iSize = MAX_LINE_SIZE;
	else
		iSize = end-start;


	memcpy( szLine, start, iSize );
	szLine[iSize]=0;
	char	szMethod[128];

	if( NULL == strstr( szLine, szFIELD_DELIM ) )  
	{
		// SIP header�� �� ù ���� field delimitor�� "field-namea: "�� ����. 
		// Request-Line Ȥ�� Status-Line [rfc-3261]
		// Request-Line = Method SP Request-URI SP SIP-Version CRLF
		// Status-Line = SIP-Version SP Status-Code SP Reason-Phrase CRLF
		// (SP : Space)

		// Get first token, 
		// Request-Line : SIP METHOD,  Status-Line : SIP-Version
		parse_start = (char *)szLine;
		parse_end = strchr( parse_start, ucSPACE );
		if( NULL == parse_end )
		{
#ifdef DEBUG_DETAIL
			dAppLog( LOG_DEBUG, "[%s] Abnormal Request/Status Header Line[%s]", __FUNCTION__, szLine );
#endif
			pstSIP->bSIP = 0;
			//pstSIP->bSIP = 1;
			return;
		}
		
		len = parse_end - parse_start;

		if( 0 == strncasecmp( parse_start, "SIP/", 4 ) )
		{
			// Status-Line �� ��� SIP-Version�� �´�. 
			len = len - 4;
			memcpy( pstSIP->szVersion, parse_start+4, len );
			pstSIP->szVersion[len] = 0;
			
			pstSIP->ucMethod = SIP_RESPONSE;
			pstSIP->ucMsgFlag = MSG_RESPONSE;

		}
		else
		{
			// Request-Line
			if ( pstSIP->ucMethod == 0x00 )
			{
				memcpy( szMethod, parse_start, len );
				szMethod[len] = 0;

				
				pstSIP->ucMethod = ucGet_Method( szMethod );

				pstSIP->ucMsgFlag = MSG_REQUEST;
			}
		}

		parse_start = parse_end + 1; parse_end = 0;

		// Get second Token ... 
		// Request Line : Request-URI, Status-Line : Status-Code;
		parse_end = strchr( parse_start, ucSPACE );
		if( NULL == parse_end )
		{
#ifdef DEBUG_DETAIL
			dAppLog( LOG_DEBUG, "[analib:%s] Abnormal Request/Status Header Line[%s]", __FUNCTION__, szLine );
#endif
			pstSIP->bSIP = 1;
			//pstSIP->bSIP = 0;
			return;
		}

		if( 0 == strncasecmp( parse_start, "tel:", 4 ) 
			|| 0 == strncasecmp( parse_start, "sip:", 4 )
			|| 0 == strncasecmp( parse_start, "sips:", 5 ) )
		{
			//Request-URI
			field_start = strchr( parse_start, ':' );
			if( NULL != field_start )
				parse_start = field_start+1;
			else
				len = parse_end - parse_start;

			if( (len=parse_end-parse_start) > MAX_RURI_SIZE-1 )
			{
				len = MAX_RURI_SIZE-1;
			}
			
			memcpy( pstSIP->szRequestURI, parse_start, len );
			pstSIP->szRequestURI[len] = 0;
		}	
		else 
		{
			//Status-Line: Status-Code
			if( len > 3 )
			{
#ifdef DEBUG_DETAIL
				dAppLog( LOG_DEBUG, "[analib:%s] Status-Code length error[%s]", __FUNCTION__, szLine );
#endif
				//len = 3;
			}

			memcpy( tmpbuf, parse_start, len );
			tmpbuf[len] = 0;
			pstSIP->usStatusCode = (unsigned short)atoi(tmpbuf);
		}
		parse_start = parse_end + 1; 

		// Get third Token ....����° ��ū�� Request/Status-Line�� ������. 
		// Request Line : SIP-Version, Status-Line : Reason-Phrase
		len = strlen(parse_start);
		if( 0 == strncasecmp( parse_start, "SIP/", 4 ) ) 
		{
			//Request-Line�� ��� 3��° �ʵ忡 SIP/Version 
			len = len - 4;
			memcpy( pstSIP->szVersion, parse_start+4, len );
			pstSIP->szVersion[len] = 0;
		}
		else
		{
			//Status-Line�� ��� Status-Code�� ���� Reason
			if( len > MAX_REASON_SIZE )
			{
#ifdef DEBUG_DETAIL
				dAppLog( LOG_DEBUG, "[analib:%s] Status-Code's Reason-Phrase is length error[%s]", __FUNCTION__, szLine );
#endif
				len = MAX_REASON_SIZE;
			}

			memcpy( pstSIP->szReason, parse_start, len );
			pstSIP->szReason[len] = 0;
		}

	}
	else
	{
		// SIP header �� field-name : field-value [rfc-3261] �� ������ ���ε�. 
		// normal form ���İ� field-name �� ���� �� compact form �� ��� �м��Ѵ�.
		
		parse_start = (char *)szLine;
#if 0
		if( 0 == strncasecmp( parse_start, "Via: ", 5 )
		 || 0 == strncasecmp( parse_start, "v: ", 3) )
		{
			if( pstSIP->usViaCnt > MAX_VIA_CNT )
			{
#ifdef DEBUG_DETAIL
				dAppLog( LOG_DEBUG, "[analib:%s] Over VIA Count [%s]", __FUNCTION__, szLine  );
#endif
				return;
			}
			
			if( ':' == szLine[1] )
				parse_start = szLine+3;
			else 
				parse_start = szLine+5;

			parse_end = strchr( parse_start, ';' );
			if( NULL == parse_end )
			{
				len = strlen(parse_start);
				parse_end = end;
			}
			else
				len = parse_end - parse_start;

			if( len > MAX_VIA_SIZE )
				len = MAX_VIA_SIZE;

			memcpy( pstSIP->szVia[pstSIP->usViaCnt], parse_start, len );
			pstSIP->szVia[pstSIP->usViaCnt][len] = 0;
			
			parse_start = parse_end + 1;
			
			while( parse_start <= end )
			{
				parse_end = strchr( parse_start, ';' );
				if( NULL == parse_end )
				{
					len = strlen(parse_start);
					parse_end = end;
				}
				else
					len = parse_end - parse_start;

				//Get other data.. (maddr, ttl, received, branch..- RFC-3261 )
				if( 0 == strncasecmp( parse_start, "branch=", 7 ) )
				{
#ifdef DEBUG_DETAIL
					dAppLog( LOG_INFO, "[analib:%s] branch[%s]", __FUNCTION__, parse_start );
#endif
				}
				else if( 0 == strncasecmp( parse_start, "maddr=", 6 ) )
				{
#ifdef DEBUG_DETAIL
					dAppLog( LOG_INFO, "[analib:%s] maddr[%s]", __FUNCTION__, parse_start );
#endif
				}
				else if( 0 == strncasecmp( parse_start, "ttl=", 4 ) )
				{
#ifdef DEBUG_DETAIL
					dAppLog( LOG_INFO, "[analib:%s] ttl[%s]", __FUNCTION__, parse_start );
#endif
				}
				else if( 0 == strncasecmp( parse_start, "received=", 9) )
				{
#ifdef DEBUG_DETAIL
					dAppLog( LOG_INFO, "[analib:%s] received[%s]", __FUNCTION__, parse_start );
#endif
				}
				else if( 0 == strncasecmp( parse_start, "tok=", 4) )
				{
#ifdef DEBUG_DETAIL
					dAppLog( LOG_INFO, "[analib:%s] tok[%s]", __FUNCTION__, parse_start );
#endif
				}
				else
				{
#ifdef DEBUG_DETAIL
					dAppLog( LOG_INFO, "[analib:%s] Unknown parameter[%s]", __FUNCTION__, parse_start );
#endif
				}

				parse_start = parse_end + 1;
			} // end of while
		
			pstSIP->usViaCnt++;
		}
#endif
		if( 0 == strncasecmp( szLine, "Max-Forwards: ", 14 ) )
		{
			parse_start = szLine+14;
			parse_end = end;
			len = strlen(parse_start);

			memcpy( tmpbuf, parse_start, len );
			tmpbuf[len] = 0;
			
			pstSIP->usMaxForwards = (unsigned short)atoi(tmpbuf);
		}
		else if( 0 == strncasecmp( szLine, "To: ", 4 )
			  || 0 == strncasecmp( szLine, "t: ", 3 ) )
		{
			if( ':' == szLine[1] )
				parse_start = szLine+3;
			else
				parse_start = szLine+4;

			
			parse_end = strchr( parse_start, ';' );
			if( NULL == parse_end )
			{
				len = end - parse_start;
				parse_end = end;
			}
			else
			{
				len = parse_end - parse_start;
			}
			
			quoted_start = strchr( parse_start, '<' );
			quoted_end = strchr( parse_start, '>' );
			if( quoted_start && quoted_end )
			{
				field_start = strchr( quoted_start, ':' ); //< tel:, sip:, sips: 
				if( NULL == field_start )
					parse_start = quoted_start+1;
				else
					parse_start = field_start+1;

				len = quoted_end - parse_start;
			}
			else
			{
#ifdef DEBUG_DETAIL
				dAppLog( LOG_DEBUG, "[analib:%s] abnormal To field, not find quoted information[%s]", __FUNCTION__, szLine );
#endif
				//pstSIP->bSIP = 0;
				return;
			}
			
			if( len > MAX_TO_SIZE )
				len = MAX_TO_SIZE;

			memcpy( pstSIP->szTo, parse_start, len );
			pstSIP->szTo[len] = 0;

			parse_start = parse_end+1;
			while( parse_start <= end )
			{
				parse_end = strchr( parse_start, ';' );
				if( NULL == parse_end )
				{
					len = end - parse_start;
					parse_end = end;
				}
				else
					len = parse_end - parse_start;

				if( 0 == strncasecmp( parse_start, "tag=", 4 ) )
				{
#ifdef DEBUG_DETAIL
					dAppLog( LOG_INFO, "[analib:%s] tag[%s]", __FUNCTION__, parse_start );
#endif
				}
				else
				{
#ifdef DEBUG_DETAIL
					dAppLog( LOG_INFO, "[analib:%s] Unknown parameter[%s]", __FUNCTION__, parse_start );
#endif
				}

				parse_start = parse_end + 1;
			} // end of while 
		}
		else if( 0 == strncasecmp( szLine, "From: ", 6 )
			  || 0 == strncasecmp( szLine, "f: ", 3 ) )
		{
			if( ':' == szLine[1] )
				parse_start = szLine+3;
			else
				parse_start = szLine+6;

			parse_end = strchr( parse_start, ';' );
			if( NULL == parse_end )
			{
				len = end - parse_start;
				parse_end = end;
			}
			else
			{
				len = parse_end - parse_start;
			}

			// Get display name
			// Display name in from field will be parsed.  not parse currently.
			
		
			// Get from data
			quoted_start = strchr( parse_start, '<' );
			quoted_end = strchr( parse_start, '>' );
			if( quoted_start && quoted_end )
			{
				field_start = strchr( quoted_start, ':' ); 
				if( NULL == field_start )
					parse_start = quoted_start+1;
				else
					parse_start = field_start+1;

				len = quoted_end - parse_start;
			}
			else
			{
#ifdef DEBUG_DETAIL
				dAppLog( LOG_DEBUG, "[analib:%s] abnormal To field, not find quoted information[%s]", __FUNCTION__, szLine );
#endif
				//pstSIP->bSIP = 0;
				return;
			}
			
			if( len > MAX_FROM_SIZE )
				len = MAX_FROM_SIZE;

			memcpy( pstSIP->szFrom, parse_start, len );
			pstSIP->szFrom[len] = 0;

			parse_start = parse_end + 1;

			while( parse_start <= end )
			{
				parse_end = strchr( parse_start, ';' );
				if( NULL == parse_end )
				{
					len = strlen(parse_start);
					parse_end = end;
				}
				else
					len = parse_end - parse_start;

				if( 0 == strncasecmp( parse_start, "tag=", 4 ) )
				{
#ifdef DEBUG_DETAIL
					dAppLog( LOG_INFO, "[analib:%s] tag[%s]", __FUNCTION__, parse_start );
#endif
				}
				else if( 0 == strncasecmp( parse_start, "mdn=", 4 ) )
				{
#ifdef DEBUG_DETAIL
					dAppLog( LOG_INFO, "[analib:%s] mdn[%s]", __FUNCTION__, parse_start );
#endif
				}
				else
				{
					dAppLog( LOG_INFO, "[analib:%s] Unknown parameter[%s]", __FUNCTION__, parse_start );
					//pstSIP->bSIP = 0;
				}

				parse_start = parse_end+1;
			} // end of while
		}
		else if( 0 == strncasecmp( szLine, "Contact: ", 9 ) 
					|| 0 == strncasecmp( szLine, "m: ", 3 ) )
        {
			if( ':' == szLine[1] )
				parse_start = szLine+3;
			else
				parse_start = szLine+9;
            
            parse_end = strchr( parse_start, ';' );
            if( NULL == parse_end )
            {
                len = end - parse_start;
                parse_end = end;
            }
            else
            {
                len = parse_end - parse_start;
            }   
                
            // Get Contact Info
            // Get from data
            quoted_start = strchr( parse_start, '<' );
            quoted_end = strchr( parse_start, '>' );
            if( quoted_start && quoted_end )
            {
                field_start = strchr( quoted_start, ':' );
                if( NULL == field_start )
                    parse_start = quoted_start+1;
                else
                    parse_start = field_start+1;

                len = quoted_end - parse_start;
            } else {   
#ifdef DEBUG_DETAIL
                dAppLog( LOG_DEBUG, "[analib:%s] abnormal To field, not find quoted information[%s]", __FUNCTION__, szLine );
#endif              
                //pstSIP->bSIP = 0;
                return;
            }
                
            if( len > MAX_FROM_SIZE )
                len = MAX_FROM_SIZE;
                
            memcpy( pstSIP->szContact, parse_start, len );
            pstSIP->szContact[len] = 0;
            
            parse_start = parse_end + 1;

            while( parse_start <= end )
            {
                parse_end = strchr( parse_start, ';' );
                if( NULL == parse_end )
                {
                    len = strlen(parse_start);
                    parse_end = end;
                }
                else
                    len = parse_end - parse_start;

                if( 0 == strncasecmp( parse_start, "tag=", 4 ) )
                {
#ifdef DEBUG_DETAIL
                    dAppLog( LOG_INFO, "[analib:%s] tag[%s]", __FUNCTION__, parse_start );
#endif
                }
                else if( 0 == strncasecmp( parse_start, "description=", 12 ) )
                {
#ifdef DEBUG_DETAIL
                    dAppLog( LOG_INFO, "[analib:%s] description[%s]", __FUNCTION__, parse_start );
#endif      
                }
                else
                {
                    dAppLog( LOG_INFO, "[analib:%s] Unknown parameter[%s]", __FUNCTION__, parse_start );
                    //pstSIP->bSIP = 0;
                }
            
                parse_start = parse_end+1;
            } // end of while
        }       
		else if( 0 == strncasecmp( szLine, "Expires: ", 9 ) )
		{
			parse_start = szLine+9;
			parse_end = end;
			len = strlen(parse_start);

			memcpy( tmpbuf, parse_start, len );
			tmpbuf[len]=0;
			
			pstSIP->usExpires = (unsigned short)atoi(tmpbuf);
		}	
		else if( 0 == strncasecmp( szLine, "Call-ID: ", 9 )
			  || 0 == strncasecmp( szLine, "i: ", 3 ) )
		{
			if( ':' == szLine[1] )
				parse_start = szLine+3;
			else
				parse_start = szLine+9;

			parse_end = end;

			if( (len = strlen( parse_start)) > MAX_CALLID_SIZE )
				len = MAX_CALLID_SIZE;
		
			memcpy( pstSIP->szCallID, parse_start, len );
			pstSIP->szCallID[len] = 0;
			
			for ( i = 0 ; i < len ; i++ )
			{
				if( pstSIP->szCallID[i] == '@' ) {
					pstSIP->szCallID[i] = 0x00;
					break;
				}
			}
		}
		else if( 0 == strncasecmp( szLine, "CSeq: ", 6 ) )
		{
			parse_start = szLine+6;
			parse_end = strchr( parse_start, ucSPACE );
			if( NULL == parse_end )
				parse_end = end;
			len = parse_end - parse_start;

			if( len > MAX_CSEQ_SIZE )
				len = MAX_CSEQ_SIZE;

			// Get CSeq
			memcpy( pstSIP->szCSeq, parse_start, len );
			pstSIP->szCSeq[len] = 0;
			//memcpy( tmpbuf, parse_start, len ); tmpbuf[len] = 0;
			//pstSIP->uiCSeq = atoi( tmpbuf );
			pstSIP->uiCSeq = (unsigned int)atoi( pstSIP->szCSeq);



			// Get CSeq Method
			if( parse_end < end )
			{
				parse_start = parse_end+1;
				parse_end = end;
				len = strlen(parse_start);

				memcpy( tmpbuf, parse_start, len );
				tmpbuf[len] = 0;

				if( 0 == pstSIP->ucMethod  && MSG_REQUEST !=  pstSIP->ucMsgFlag )
				{
					//pstSIP->ucMethod = ucGet_Method(tmpbuf);
					memcpy( szMethod, tmpbuf, len );
					szMethod[len]=0;
				}
#ifdef DEBUG_DETAIL
				else
					dAppLog( LOG_DEBUG, "[analib:%s] Method has already setted [method: 0x%02d]", __FUNCTION__,  pstSIP->ucMethod );
#endif
			}
		}
		else if( 0 == strncasecmp( szLine, "Content-Length: ", 16 )
			  || 0 == strncasecmp(szLine, "l: ", 3 ) )
		{
			if( szLine[1] == ':' )
				parse_start = szLine+3;
			else 
				parse_start = szLine+16;

			parse_end = end;
			len = strlen(parse_start);

			memcpy( tmpbuf, parse_start, len ); 
			tmpbuf[len] = 0;
			
			pstSIP->usContentLen= (unsigned short)atoi(tmpbuf);
		}
		else if( 0 == strncasecmp( szLine, "Content-Type: ", 14 )
			  || 0 == strncasecmp( szLine, "c: ", 3) )
		{
			if( ':' == szLine[1] )
				parse_start = szLine+3;
			else
				parse_start = szLine+14;

			parse_end = end;
			if( (len = strlen(parse_start)) > MAX_CONTENTTYPE_SIZE )
				len = MAX_CONTENTTYPE_SIZE;

			memcpy( pstSIP->szContentType, parse_start, len );
			pstSIP->szContentType[len] = 0;
#if 0	
			// INVITE �� CONTENT TYPE �� ������ ���� ��찡 �ִ�. 
			if( 0 != strstr( parse_start, "sdp" ) )
			{
				pstSIP->ucSDPFlag = 0x01;
			}
#endif
		}
		else if( 0 == strncasecmp( szLine, "Timestamp: ", 11 ) )
		{
			parse_start = szLine+11;
			parse_end = end;
			len = strlen(parse_start);
	
			memcpy( tmpbuf, parse_start, len );
			tmpbuf[len] = 0;

			pstSIP->uiTimestamp = (unsigned int)atoi(tmpbuf);
		}
		else
		{
#ifdef DEBUG_DETAIL
			dAppLog( LOG_DEBUG, "[analib:%s] Not analyze header field[%s]", __FUNCTION__, szLine );
#endif
		}
	} // end if if
}


/**
 @brief		ucGet_Method

 �ƱԸ�Ʈ�� ���� ���ڿ��� ���Ͽ� �� SIP Method�� �ڵ尪(��ü ���� ptt_define.h)��
 �����Ѵ�. 

 @return	�� method�� �ڵ尪.

 */
unsigned char ucGet_Method( char *method )
{
	unsigned char val;
	

	if( 0 == strncasecmp( method, "INFO", strlen(method) ) )
	{
		val = SIP_INFO;
	}
	else if( 0 == strncasecmp( method, "ACK", strlen(method)) )
	{
		val = SIP_ACK;
	}
	else if( 0 == strncasecmp( method, "BYE", strlen(method)) )
	{
		val = SIP_BYE;
	}
	else if( 0 == strncasecmp( method,  "CANCEL", strlen(method)) )
	{
		val = SIP_CANCEL;
	}
	else if( 0 == strncasecmp( method, "OPTIONS", strlen(method)) )
	{
		val = SIP_OPTIONS;
	}
	else if( 0 == strncasecmp( method, "INVITE", strlen(method)) )
	{
		val = SIP_INVITE;
	}
	else if( 0 == strncasecmp( method, "REGISTER", strlen(method)) )
	{
		val = SIP_REGISTER;
	}
	else if( 0 == strncasecmp( method, "SUBSCRIBE", strlen(method)) )
	{
		val = SIP_SUBSCRIBE;
	}
	else 
	{
#ifdef DEBUG_DETAIL
		dAppLog( LOG_DEBUG, "[analib:%s] Unknown first token Type[%s]", __FUNCTION__, method );
#endif
		val = SIP_UNKNOWN;
	}

	return val;
}



/* ************************************************** 
 * $Log
 * ************************************************** */
