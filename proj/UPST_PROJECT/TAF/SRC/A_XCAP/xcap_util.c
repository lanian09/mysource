/**     @file   xcap_util.c
 *      - XCAP Service Processing
 *
 *      Copyright (c) 2006~ by Upresto Inc. Korea
 *      All rights reserved
 *
 *       $Id: xcap_util.c,v 1.2 2011/09/06 12:46:42 hhbaek Exp $
 *
 *      @Author     $Author: hhbaek $
 *      @version    $Revision: 1.2 $
 *      @date       $Date: 2011/09/06 12:46:42 $
 *      @ref        xcap_util.c
 *      @todo       Nothing
 *
 *      @section    Intro(소개)
 *      - XCAP Service Processing
 *
 *      @section    Requirement
 *       @li Nothing
 *
 **/

/**
 *	Include headers
 */
// TOP
#include "common_stg.h"

// .
#include "xcap_util.h"

/**
 *	Implement funcs
 */
char *PrintTYPE(S32 type)
{
	switch(type)
	{
	case CAP_HEADER_NUM:			return "CAP_HEADER";
	case INFO_ETH_NUM:				return "INFO_ETH";
	case TCP_INFO_DEF_NUM:			return "TCP_INFO";
	case LOG_HTTP_TRANS_DEF_NUM:	return "LOG_HTTP_TRANS";
	case HTTP_REQ_HDR_NUM:			return "HTTP_REQ_HDR";
	case HTTP_REQ_BODY_NUM:			return "HTTP_REQ_BODY";
	case HTTP_RES_HDR_NUM:			return "HTTP_RES_HDR";
	case HTTP_RES_BODY_NUM:			return "HTTP_RES_BODY";
	case TEXT_INFO_DEF_NUM:			return "TEXT_INFO";
	default:						return "UNKNOWN";
	}
}

char *PrintMsgType(S32 msgtype)
{

	if(msgtype >= 100)				return "RESP";

	switch(msgtype)
	{
	case METHOD_GET:				return "GET";
	case METHOD_POST:				return "POST";
	case METHOD_HEAD:				return "HEAD";
	case METHOD_PUT:				return "PUT";
	case METHOD_OPTIONS:			return "OPTIONS";
	case METHOD_DELETE:				return "DELETE";
	case METHOD_TRACE:				return "TRACE";
	default:						return "UNKNOWN";
	}
}

char *PrintResStr(S32 msgtype)
{
	switch(msgtype)
	{
	case 100: return "Continue";
	case 101: return "Switching protocols";
	case 200: return "Ok";
	case 201: return "Created";
	case 202: return "Accepted";
	case 203: return "Non-authoritive information";
	case 204: return "No content";
	case 205: return "Reset content";
	case 206: return "Partial content";
	case 226: return "IM used";
	case 300: return "Multiple choices";
	case 301: return "Moved permanently";
	case 302: return "Moved temporarily";
	case 303: return "See other";
	case 304: return "Not modified";
	case 305: return "Use proxy";
	case 400: return "Bad request";
	case 401: return "Unauthorized";
	case 402: return "Payment required";
	case 403: return "Forbidden";
	case 404: return "Not found";
	case 405: return "Method not allowed";
	case 406: return "Not acceptable";
	case 407: return "Proxy authentication required";
	case 408: return "Request timeout";
	case 409: return "Conflict";
	case 410: return "Gone";
	case 411: return "Length required";
	case 412: return "Precondition failed";
	case 413: return "Request entity too large";
	case 414: return "Request URI too large";
	case 415: return "Unsupported media type";
	case 426: return "Upgrade Required";
	case 500: return "Internal server error";
	case 501: return "Not implemented";
	case 502: return "Bad gateway";
	case 503: return "Service unavailable";
	case 504: return "Gateway timeout";
	case 505: return "HTTP version not supported";
	default: return "UNKNOWN";
	}
}

/**
 *  $Log: xcap_util.c,v $
 *  Revision 1.2  2011/09/06 12:46:42  hhbaek
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *  NEW OAM SYSTEM
 *
 *  Revision 1.3  2011/08/17 07:28:12  dcham
 *  *** empty log message ***
 *
 *  Revision 1.2  2011/08/08 11:05:44  uamyd
 *  modified block added
 *
 *  Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 *  init DQMS2
 *
 *  Revision 1.2  2011/01/11 04:09:11  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2010/08/23 01:13:01  uamyd
 *  DQMS With TOTMON, 2nd-import
 *
 *  Revision 1.1.1.1  2009/05/26 02:14:30  dqms
 *  Init TAF_RPPI
 *
 *  Revision 1.1  2008/09/18 07:24:37  dark264sh
 *  IM 서비스 추가 (SIP, XCAP, MSRP)
 *
 *  Revision 1.1.1.1  2007/12/27 08:17:38  uamyd
 *  import
 *
 *  Revision 1.1.1.1  2007/10/22 07:50:44  jsyoon
 *  AQUA3 Project = AQUA2 + IMS Protocol + Packet Dump
 *
 *  Revision 1.3  2007/04/25 11:08:21  dark264sh
 *  NFO_ETH, Capture_Header_Msg 삭제, st_PKT_INFO 추가에 따른 변경
 *
 *  Revision 1.2  2007/03/14 09:27:46  dark264sh
 *  remove prefix
 *
 *  Revision 1.1  2007/03/07 10:33:41  dark264sh
 *  *** empty log message ***
 *
 */
          
