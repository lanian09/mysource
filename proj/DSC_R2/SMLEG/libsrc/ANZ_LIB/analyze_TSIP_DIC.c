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

/**
 @brief     ParseTokSIP 함수
 
 Tokenized SIP 프로토콜을 분석한다. \n
 Tokenized 되어있는 부분을 조합해 SIP 메시지를 생성한다. \n
 분석이 완료되면, 

 SIP/SDP 를 분석하여 각 SIP 정보 구조체를 채운다. 	
 
 @return  분석 성공 : 0, 실패 : < 0
 */

int iGet_LocDir_ver1( char *buf, const unsigned char code )
{
	int len = 0;

	switch( (int)code )
	{
		case 0xA0:
			// Local string A0 is always reserved for used as the protocol version string.
			sprintf( buf, "%s", "1/com.sprintpcs/1" );
			break;

		case 0xA1: 
			sprintf( buf, "%s", "sprintpcs.com" );
			break;
		
		case 0xA2:
			sprintf( buf, "%s", "ptt.sprintpcs.com" );
			break;

		case 0xA3:
			sprintf( buf, "%s", "sip:sprintpcs.com" );
			break;

		case 0xA4:
			sprintf( buf, "%s", "sip:ptt.sprintpcs.com" );
			break;
		
		case 0xA5:
		case 0xA6:
		case 0xA7:
		case 0xA8:
		case 0xA9:
		case 0xAA:
			// reserved 
			break;
		
		case 0xAB:
			sprintf( buf, "%s", "ptt_tf" );
			break;
	
		case 0xAC:
			sprintf( buf, "%s", "ptt_ti" );
			break;

		case 0xAD:
			sprintf( buf, "%s", "ptt_tc" );
			break;

		case 0xAE:
			sprintf( buf, "%s", "ptt_tp" );
			break;

		case 0xAF:
			sprintf( buf, "%s", "ptt_fd" );
            break;
	
		case 0xB0:
			sprintf( buf, "%s", "mdn" );
            break;

		case 0xB1:
			sprintf( buf, "%s", "App-ID" );
            break;

		case 0xB2:
			sprintf( buf, "%s", "PTTI" );
            break;

		case 0xB3:
			sprintf( buf, "%s", "PTTG" );
            break;

		case 0xB4:
			sprintf( buf, "%s", "CM" );
            break;

		case 0xB5:
			sprintf( buf, "%s", "Winphoria" );
            break;

		case 0xB6:
			//sprintf( buf, "%s", "97 EVRC/4000" );
			sprintf( buf, "%s", "97 EVRC" );
			break;

		case 0xB7:
			sprintf( buf, "%s", "0 97" );
			break;

		case 0xB8:
		case 0xB9:
		case 0xBA:
		case 0xBB:
		case 0xBC:
		case 0xBD:
		case 0xBE:
		case 0xBF:
			// reserved
			break;

		default:
			dAppLog( LOG_DEBUG, "[%s] Undefine Location Direct Token code[0x%02x]", 
				__FUNCTION__, code );
	} // end of switch

	len = strlen(buf);

	return len;
}



int iGet_LocDir_ver2( char *buf, const unsigned char code )
{
	switch( (int)code )
	{
		case 0:
		case 0xA0:
			// Local string A0 is always reserved for used as the protocol version string.
			sprintf( buf, "%s", "1/com.sprintpcs/2" );
			break;
	
		case 1:
		case 0xA1: 
			sprintf( buf, "%s", "sprintpcs.com" );
			break;

		case 2:
		case 0xA2:
			sprintf( buf, "%s", "ptt.sprintpcs.com" );
			break;

		case 3:
		case 0xA3:
			sprintf( buf, "%s", "sip:sprintpcs.com" );
			break;

		case 4:
		case 0xA4:
			sprintf( buf, "%s", "sip:ptt.sprintpcs.com" );
			break;
		
		case 5:
		case 0xA5:
			sprintf( buf, "%s", "1x.bell.ca" );
			break;

		case 6:
		case 0xA6:
			sprintf( buf, "%s", "services5.1x.bell.ca" );
            break;
		
		case 7:
		case 0xA7:
			sprintf( buf, "%s", "sip:1x.bell.ca" );
            break;
		
		case 8:
		case 0xA8:
			sprintf( buf, "%s", "sip:services.1x.bell.ca" );
            break;
		
		case 9:
		case 0xA9:
		case 10:
		case 0xAA:
			// reserved 
			break;
		
		case 11:
		case 0xAB:
			sprintf( buf, "%s", "ptt_tf" );
			break;
	
		case 12:
		case 0xAC:
			sprintf( buf, "%s", "ptt_ti" );
			break;

		case 13:
		case 0xAD:
			sprintf( buf, "%s", "ptt_tc" );
			break;
	
		case 14:
		case 0xAE:
			sprintf( buf, "%s", "ptt_tp" );
			break;

		case 15:
		case 0xAF:
			sprintf( buf, "%s", "ptt_fd" );
            break;
	
		case 16:
		case 0xB0:
			sprintf( buf, "%s", "mdn" );
            break;

		case 17:
		case 0xB1:
			sprintf( buf, "%s", "App-ID" );
            break;

		case 18:
		case 0xB2:
			sprintf( buf, "%s", "PTTI" );
            break;

		case 19:
		case 0xB3:
			sprintf( buf, "%s", "PTTG" );
            break;

		case 20:
		case 0xB4:
			sprintf( buf, "%s", "CM" );
            break;

		case 21:
		case 0xB5:
			sprintf( buf, "%s", "Winphoria" );
            break;

		case 22:
		case 0xB6:
			//sprintf( buf, "%s", "97 EVRC/4000" );
			sprintf( buf, "%s", "97 EVRC" );
			break;

		case 23:
		case 0xB7:
			sprintf( buf, "%s", "0 97" );
			break;

		case 24:
		case 0xB8:
		case 25:
		case 0xB9:
		case 26:
		case 0xBA:
		case 27:
		case 0xBB:
		case 28:
		case 0xBC:
		case 29:
		case 0xBD:
		case 30:
		case 0xBE:
		case 31:
		case 0xBF:
			// reserved
			break;

		case 32:
			sprintf( buf, "%s", "%40" );
			break; 

		case 33:
			sprintf( buf, "%s", "mip-expire" );
			break; 

		case 34:
			sprintf( buf, "%s", "application/cpim-pidf+wbxml" );
			break; 

		case 35:
			sprintf( buf, "%s", "application/vnd.dynamicsoft.doapi+wbxml" );
			break; 

		case 36:
			sprintf( buf, "%s", "application/watcherinfo+wbxml" );
			break; 

		case 37:
			sprintf( buf, "%s", "application/notification_policy+wbxml" );
			break; 

		case 38:
			sprintf( buf, "%s", "buddylist" );
		    break; 

		case 39:
			sprintf( buf, "%s", "dm" );
			break; 

		case 40:
			sprintf( buf, "%s", "presence" );
			break; 

		case 41:
			sprintf( buf, "%s", "presence.winfo" );
			break; 

		case 42:
			sprintf( buf, "%s", "im" );
			break; 

		case 43:
			sprintf( buf, "%s", "wap" );
			break; 

		case 44:
			sprintf( buf, "%s", "wap.126" );
			break; 

		case 45:
			sprintf( buf, "%s", "0.5" );
			break; 

		case 46:
			sprintf( buf, "%s", "number" );
			break; 

		case 47:
			sprintf( buf, "%s", "total" );
			break; 

		default:
			dAppLog( LOG_DEBUG, "[%s] Undefine Location Direct Token code[0x%02x]", 
				__FUNCTION__, code );
	} // end of switch

	return strlen(buf);
}

int iGet_StatusString( char *buf, const short code )
{
	switch( code )
	{
		case 100:
			sprintf( buf, "%s", "Trying" );
			break;

		case 180: 
			sprintf( buf, "%s", "Ringing" );
            break;

		case 181:
			sprintf( buf, "%s", "Call Is Being Forwarded" );
            break;
	
		case 182:
			sprintf( buf, "%s", "Queued" );
            break;

		case 183:
			sprintf( buf, "%s", "Session Progress" );
            break;

		case 200:
			sprintf( buf, "%s", "OK" );
            break;

		case 300:
			sprintf( buf, "%s", "Multiple Choices" );
            break;

		case 301:
			sprintf( buf, "%s", "Moved Permanently" );
            break;

		case 302:
			sprintf( buf, "%s", "Moved Temporarily" );
            break;

		case 305:
			sprintf( buf, "%s", "Use Proxy" );
            break;

		case 380:
			sprintf( buf, "%s", "Alternative Service" );
            break;

		case 400:
			sprintf( buf, "%s", "Bad Request" );
            break;

		case 401:
			sprintf( buf, "%s", "Unauthorized" );
            break;

		case 402:
			sprintf( buf, "%s", "Payment required" );
            break;

		case 403:
			sprintf( buf, "%s", "Forbidden" );
            break;

		case 404:
			sprintf( buf, "%s", "Not Found" );
            break;

		case 405:
			sprintf( buf, "%s", "Method Not Allowed" );
            break;

		case 406:
			sprintf( buf, "%s", "Not Acceptable" );
            break;

		case 407:
			sprintf( buf, "%s", "Proxy Authentication Required" );
            break;

		case 408:
			sprintf( buf, "%s", "Request Timeout" );
            break;
		
		case 410:
			sprintf( buf, "%s", "Gone" );
            break;	

		case 413:
			sprintf( buf, "%s", "Request Entity Too Large" );
            break;

		case 414:
			sprintf( buf, "%s", "Request-URI Too Long" );
            break;

		case 415:
			sprintf( buf, "%s", "Unsupported Media Type" );
            break;

		case 416:
			sprintf( buf, "%s", "Unsupported URI Scheme" );
            break;

		case 420:
			sprintf( buf, "%s", "Bad Extension" );
            break;

		case 421:
			sprintf( buf, "%s", "Extension Required" );
            break;

		case 423:
			sprintf( buf, "%s", "Interval Too brief" );
            break;

		case 480:
			sprintf( buf, "%s", "Temporarily Unavailable" );
            break;

		case 481:
			sprintf( buf, "%s", "Call/Transaction Does Not Exist" );
            break;

		case 483:
			sprintf( buf, "%s", "Too many Hops" );
            break;

		case 484:
			sprintf( buf, "%s", "Address Incomplete" );
            break;

		case 485:
			sprintf( buf, "%s", "Ambiguous" );
            break;

		case 486:
			sprintf( buf, "%s", "Busy Here" );
            break;

		case 487:
			sprintf( buf, "%s", "Request Terminated" );
            break;

		case 488:
			sprintf( buf, "%s", "Not Acceptable Here" );
            break;

		case 491:
			sprintf( buf, "%s", "Request Pending" );
            break;

		case 493:
			sprintf( buf, "%s", "Undecipherable" );
            break;

		case 500:
			sprintf( buf, "%s", "Server Internal Error" );
            break;
		
		case 501:
			sprintf( buf, "%s", "Not Implemented" );
            break;

		case 502:
			sprintf( buf, "%s", "Bad Gateway" );
            break;

		case 503:
			sprintf( buf, "%s", "Service Unavailable" );
            break;

		case 504:
			sprintf( buf, "%s", "Server Time-out" );
            break;

		case 505:
			sprintf( buf, "%s", "Version Not Supported" );
            break;

		case 513:
			sprintf( buf, "%s", "Message Too Large" );
            break;

		case 600:
			sprintf( buf, "%s", "Busy Everywhere" );
            break;

		case 603:
			sprintf( buf, "%s", "Decline" );
            break;

		case 604:
			sprintf( buf, "%s", "Does Not Exist Anywhere" );
            break;

		case 606:
			sprintf( buf, "%s", "Not Acceptable" );
            break;

		default:
			dAppLog( LOG_DEBUG, "[%s] Unknown status code[%d]", 
				__FUNCTION__, code );
	} // end of switch

	return strlen( buf );
}

int iGet_SipDic_ver1( char* buf, unsigned char i, unsigned char flag )
{
	int index= 0;

	if( flag == 0x01 )
		index = 1000+i;
	else 
		index = i;

	switch( index )
	{
		case 0:
		case 1192: 
			sprintf( buf, "%s", "tok" );
			break;

		case 1:
		case 1193:
			sprintf( buf, "%s", "tok-redir" );
            break;
		
		case 2:
		case 1194:
			sprintf( buf, "%s", "sip" );
            break;

		case 3:
		case 1195:
			sprintf( buf, "%s", "SIP" );
            break;
	
		case 4:
		case 1196:
			sprintf( buf, "%s", "SIP/2.0" );
            break;

		case 5:
		case 1197:
			sprintf( buf, "%s", "SIP/2.0/TCP" );
            break;

		case 6:
		case 1198:
			sprintf( buf, "%s", "SIP/2.0/UDP" );
            break;

		case 7:
		case 1199:
			sprintf( buf, "%s", "FALSE" );
            break;

		case 8:
		case 1200:
			sprintf( buf, "%s", "False" );
            break;

		case 9:
		case 1201:
			sprintf( buf, "%s", "false" );
			break;
		
		case 10:
		case 1202:
			sprintf( buf, "%s", "TRUE" );
            break;

		case 11:
		case 1203:
			sprintf( buf, "%s", "True" );
            break;

		case 12:
		case 1204:
			sprintf( buf, "%s", "true" );
            break;

		case 13:
		case 1205:
			sprintf( buf, "%s", "OK" );
            break;

		case 14:
		case 1206:
			sprintf( buf, "%s", "Ok" );
            break;

		case 15:
		case 1207:
			sprintf( buf, "%s", "application/sdp" );
            break;

		case 16:
		case 1208:
			sprintf( buf, "%s", "text/plain" );
            break;

		case 17:
		case 1209:
			sprintf( buf, "%s", "lr" );
            break;

		case 18:
		case 1210:
			sprintf( buf, "%s", "maddr" );
            break;

		case 19:
		case 1211:
			sprintf( buf, "%s", "RTP/AVP" );
            break;

		case 20:
		case 1212:
			sprintf( buf, "%s", "expires" );
            break;

		case 21:
		case 1213:
			sprintf( buf, "%s", "mandatory" );
            break;

		case 22:
		case 1214:
			sprintf( buf, "%s", "optional" );
            break;

		case 23:
		case 1215:
			sprintf( buf, "%s", "cnonce" );
            break;

		case 24:
		case 1216:
			sprintf( buf, "%s", "nc" );
            break;

		case 25:
		case 1217:
			sprintf( buf, "%s", "nextnonce" );
            break;

		case 26:
		case 1218:
			sprintf( buf, "%s", "rspauth" );
            break;

		case 27:
		case 1219:
			sprintf( buf, "%s", "domain" );
            break;

		case 28:
		case 1220:
			sprintf( buf, "%s", "none" );
            break;

		case 29:
		case 1221:
			sprintf( buf, "%s", "e2e" );
            break;

		case 30:
		case 1222:
			sprintf( buf, "%s", "comp" );
            break;

		case 31:
		case 1223:
			sprintf( buf, "%s", "sigcomp" );
            break;

		case 32:
			sprintf( buf, "%s", "0 0" );
            break;
	
		case 33:
			sprintf( buf, "%s", "0.0.0.0" );
            break;

		case 34:
			sprintf( buf, "%s", "100rel" );
            break;

		case 35:
			sprintf( buf, "%s", "3600" );
            break;
		
		case 36:
			sprintf( buf, "%s", "Accepted" );
            break;

		case 37:
			sprintf( buf, "%s", "ack" );
            break;

		case 38:
			sprintf( buf, "%s", "active" );
            break;

		case 39:
			sprintf( buf, "%s", "Address Incomplete" );
            break;

		case 40:
			sprintf( buf, "%s", "AKAv" );
            break;

		case 41:
			sprintf( buf, "%s", "AKAv1-MD5" );
            break;

		case 42:
			sprintf( buf, "%s", "alert" );
            break;

		case 43:
			sprintf( buf, "%s", "alg" );
            break;

		case 44:
			sprintf( buf, "%s", "algorithm" );
            break;

		case 45:
			sprintf( buf, "%s", "Alternative Service" );
            break;

		case 46:
			sprintf( buf, "%s", "Ambiguous" );
            break;

		case 47:
			sprintf( buf, "%s", "AMR" );
            break;
		
		case 48:
			sprintf( buf, "%s", "Anonymous" );
            break;

		case 49:
			sprintf( buf, "%s", "app" );
            break;
	
		case 50:
			sprintf( buf, "%s", "application" );
            break;

		case 51:
			sprintf( buf, "%s", "application/cpim-pidf+xml" );
            break;

		case 52:
			sprintf( buf, "%s", "application/mime" );
            break;		

		case 53:
			sprintf( buf, "%s", "application/vnd.wap.coc" );
            break;

		case 54:
			sprintf( buf, "%s", "application/vnd.wap.sic" );
            break;

		case 55:
			sprintf( buf, "%s", "application/vnd.wap.slc" );
            break;

		case 56:
			sprintf( buf, "%s", "Apr" );
            break;

		case 57:
			sprintf( buf, "%s", "AS" );
            break;

		case 58:
			sprintf( buf, "%s", "audio" );
            break;

		case 59:
			sprintf( buf, "%s", "Aug" );
            break;

		case 60:
			sprintf( buf, "%s", "auth" );
            break;

		case 61:
			sprintf( buf, "%s", "auth-int" );
            break;

		case 62:
			sprintf( buf, "%s", "auts" );
            break;
	
		case 63:
			sprintf( buf, "%s", "Bad Event" );
            break;

		case 64:
			sprintf( buf, "%s", "Bad Extension" );
			break;

		case 65:
			sprintf( buf, "%s", "Bad Gateway" );
            break;

		case 66:
			sprintf( buf, "%s", "Bad Request" );
            break;

		case 67:
			sprintf( buf, "%s", "base64" );
            break;

		case 68:
			sprintf( buf, "%s", "branch" );
            break;
		
		case 69:
			sprintf( buf, "%s", "Busy Everywhere" );
            break;
		
		case 70:
			sprintf( buf, "%s", "Busy Here" );
            break;

		case 71:
			sprintf( buf, "%s", "Call Is Being Forwarded" );
            break;

		case 72:
			sprintf( buf, "%s", "Call/Transaction Does Not Exist" );
            break;

		case 73:
			sprintf( buf, "%s", "card" );
            break;

		case 74:
			sprintf( buf, "%s", "cat" );
            break;

		case 75:
			sprintf( buf, "%s", "cause" );
            break;

		case 76:
			sprintf( buf, "%s", "channels" );
            break;

		case 77:
			sprintf( buf, "%s", "charset" );
            break;

		case 78:
			sprintf( buf, "%s", "clear" );
            break;

		case 79:
			sprintf( buf, "%s", "conf" );
            break;

		case 80:
			sprintf( buf, "%s", "control" );
            break;

		case 81:
			sprintf( buf, "%s", "crc" );
            break;

		case 82:
			sprintf( buf, "%s", "critical" );
            break;

		case 83:
			sprintf( buf, "%s", "CT" );
            break;

		case 84:
			sprintf( buf, "%s", "curr" );
            break;

		case 85:
			sprintf( buf, "%s", "data" );
            break;

		case 86:
			sprintf( buf, "%s", "deactivated" );
            break;

		case 87:
			sprintf( buf, "%s", "Dec" );
            break;

		case 88:
			sprintf( buf, "%s", "Decline" );
            break;

		case 89:
			sprintf( buf, "%s", "des" );
            break;

		case 90:
			sprintf( buf, "%s", "Dialog Terminated" );
            break;

		case 91:
			sprintf( buf, "%s", "Digest" );
            break;

		case 92:
			sprintf( buf, "%s", "digest" );
            break;

		case 93:
			sprintf( buf, "%s", "disgest-integrity" );
            break;

		case 94:
			sprintf( buf, "%s", "Does Not Exist Anywhere" );
            break;

		case 95:
			sprintf( buf, "%s", "duration" );
            break;

		case 96:
			sprintf( buf, "%s", "emergency" );
            break;

		case 97:
			sprintf( buf, "%s", "events" );
            break;

		case 98:
			sprintf( buf, "%s", "Extension Required" );
            break;

		case 99:
			sprintf( buf, "%s", "failure" );
            break;

		case 100:
			sprintf( buf, "%s", "Feb" );
            break;

		case 101:
			sprintf( buf, "%s", "fmtp" );
            break;

		case 102:
			sprintf( buf, "%s", "Forbidden" );
            break;

		case 103:
			sprintf( buf, "%s", "framerate" );
            break;

		case 104:
			sprintf( buf, "%s", "Fri" );
            break;

		case 105:
			sprintf( buf, "%s", "from-tag" );
            break;

		case 106:
			sprintf( buf, "%s", "giveup" );
            break;

		case 107:
			sprintf( buf, "%s", "GMT" );
            break;

		case 108:
			sprintf( buf, "%s", "Gone" );
            break;

		case 109:
			sprintf( buf, "%s", "group" );
            break;

		case 110:
			sprintf( buf, "%s", "handling" );
            break;

		case 111:
			sprintf( buf, "%s", "header" );
            break;

		case 112:
			sprintf( buf, "%s", "http" );
            break;
		
		case 113:
			sprintf( buf, "%s", "icon" );
            break;

		case 114:
			sprintf( buf, "%s", "id" );
            break;

		case 115:
			sprintf( buf, "%s", "im" );
            break;
	
		case 116:
			sprintf( buf, "%s", "image" );
            break;

		case 117:
			sprintf( buf, "%s", "IN IP4" );
            break;

		case 118:
			sprintf( buf, "%s", "IN IP6" );
            break;

		case 119:
			sprintf( buf, "%s", "inactive" );
            break;
		
		case 120:
			sprintf( buf, "%s", "info" );
            break;

		case 121:
			sprintf( buf, "%s", "interleaving" );
            break;

		case 122:
			sprintf( buf, "%s", "Interval Too Brief" );
            break;

		case 123:
			sprintf( buf, "%s", "ip" );
            break;
		
		case 124:
			sprintf( buf, "%s", "ipsec-3gpp" );
            break;

		case 125:
			sprintf( buf, "%s", "ipsec-ike" );
            break;

		case 126:
			sprintf( buf, "%s", "ipsec-man" );
            break;

		case 127:
			sprintf( buf, "%s", "Jan" );
            break;
		
		case 128:
			sprintf( buf, "%s", "Jul" );
            break;

		case 129:
			sprintf( buf, "%s", "Jun" );
            break;
		
		case 130:
			sprintf( buf, "%s", "key-mgmt" );
            break;

		case 131:
			sprintf( buf, "%s", "key-mgmt:mikey" );
            break;

		case 132:
			sprintf( buf, "%s", "keywds" );
            break;

		case 133:
			sprintf( buf, "%s", "lang" );
            break;

		case 134:
			sprintf( buf, "%s", "local" );
            break;

		case 135:
			sprintf( buf, "%s", "Loop Detected" );
            break;

		case 136:
			sprintf( buf, "%s", "mailto" );
            break;

		case 137:
			sprintf( buf, "%s", "Mar" );
            break;
		
		case 138:
			sprintf( buf, "%s", "maxptime" );
            break;

		case 139:
			sprintf( buf, "%s", "May" );
            break;

		case 140:
			sprintf( buf, "%s", "MD5" );
            break;

		case 141:
			sprintf( buf, "%s", "MD5-sess" );
            break;

		case 142:
			sprintf( buf, "%s", "message" );
            break;

		case 143:
			sprintf( buf, "%s", "Message Too Large" );
            break;

		case 144:
			sprintf( buf, "%s", "message/cpim" );
            break;

		case 145:
			sprintf( buf, "%s", "message/sip" );
            break;

		case 146:	
			sprintf( buf, "%s", "message/sipfrag" );
            break;

		case 147:
			sprintf( buf, "%s", "method" );
            break;
		
		case 148:
			sprintf( buf, "%s", "Method Not Allowed" );
            break;

		case 149:
			sprintf( buf, "%s", "mid" );
            break;

		case 150:
			sprintf( buf, "%s", "mode-change-neighbor" );
            break;

		case 151:
			sprintf( buf, "%s", "mode-change-period" );
            break;	

		case 152:
			sprintf( buf, "%s", "mode-set" );
            break;
		
		case 153:
			sprintf( buf, "%s", "Mon" );
            break;

		case 154:
			sprintf( buf, "%s", "Moved Permanently" );
            break;

		case 155:
			sprintf( buf, "%s", "Moved Temporarily" );
            break;

		case 156:
			sprintf( buf, "%s", "multipart" );
            break;

		case 157:
			sprintf( buf, "%s", "multipart/mixed" );
            break;
		
		case 158:
			sprintf( buf, "%s", "multipart/signed" );
            break;

		case 159:
			sprintf( buf, "%s", "Multiple Choices" );
            break;

		case 160:
			sprintf( buf, "%s", "nack" );
            break;

		case 161:
			sprintf( buf, "%s", "nonce" );
            break;

		case 162:
			sprintf( buf, "%s", "non-urgent" );
            break;

		case 163:
			sprintf( buf, "%s", "noresource" );
            break;

		case 164:
			sprintf( buf, "%s", "normal" );
            break;

		case 165:
			sprintf( buf, "%s", "Not Acceptable" );
            break;

		case 166:
			sprintf( buf, "%s", "Not Acceptable Here" );
            break;

		case 167:
			sprintf( buf, "%s", "Not Found" );
            break;

		case 168:
			sprintf( buf, "%s", "Not Implemented" );
            break;

		case 169:
			sprintf( buf, "%s", "Nov" );
            break;

		case 170:
			sprintf( buf, "%s", "Oct" );
            break;

		case 171:
			sprintf( buf, "%s", "octet-align" );
            break;

		case 172:
			sprintf( buf, "%s", "opaque" );
            break;

		case 173:
			sprintf( buf, "%s", "orient:landscape" );
            break;

		case 174:
			sprintf( buf, "%s", "orient:portrait" );
            break;

		case 175:
			sprintf( buf, "%s", "orient:seascape" );
            break;

		case 176:
			sprintf( buf, "%s", "path" );
            break;

		case 177:
			sprintf( buf, "%s", "Payment Required" );
            break;

		case 178:
			sprintf( buf, "%s", "pending" );
            break;

		case 179:
			sprintf( buf, "%s", "phone" );
            break;

		case 180:
			sprintf( buf, "%s", "pli" );
            break;

		case 181:
			sprintf( buf, "%s", "precondition" );
            break;

		case 182:
			sprintf( buf, "%s", "Precondition Failure" );
            break;

		case 183:
			sprintf( buf, "%s", "pref" );
            break;

		case 184:
			sprintf( buf, "%s", "pres" );
            break;
			
		case 185:
			sprintf( buf, "%s", "privacy" );
            break;

		case 186:
			sprintf( buf, "%s", "probation" );
            break;

		case 187:
			sprintf( buf, "%s", "prompt" );
            break;

		case 188:
			sprintf( buf, "%s", "Provide Referror Identity" );
            break;

		case 189:
			sprintf( buf, "%s", "Proxy Authentication Required" );
            break;

		case 190:
			sprintf( buf, "%s", "ptime" );
            break;

		case 191:
			sprintf( buf, "%s", "purpose" );
            break;

		case 192:
			sprintf( buf, "%s", "q" );
            break;

		case 193:
			sprintf( buf, "%s", "Q.850" );
            break;

		case 194:
			sprintf( buf, "%s", "qop" );
            break;

		case 195:
			sprintf( buf, "%s", "qos" );
            break;

		case 196:
			sprintf( buf, "%s", "quality" );
            break;

		case 197:
			sprintf( buf, "%s", "Queued" );
            break;

		case 198:
			sprintf( buf, "%s", "rate" );
            break;
		
		case 199:
			sprintf( buf, "%s", "realm" );
            break;
		
		case 200:
			sprintf( buf, "%s", "reason" );
            break;

		case 201:
			sprintf( buf, "%s", "received" );
            break;

		case 202:
			sprintf( buf, "%s", "recv" );
            break;

		case 203:
			sprintf( buf, "%s", "recvonly" );
            break;

		case 204:
			sprintf( buf, "%s", "refer" );
            break;

		case 205:
			sprintf( buf, "%s", "refresher" );
            break;

		case 206:
			sprintf( buf, "%s", "rejected" );
            break;

		case 207:
			sprintf( buf, "%s", "remote" );
            break;

		case 208:
			sprintf( buf, "%s", "render" );
            break;

		case 209:
			sprintf( buf, "%s", "replaces" );
            break;

		case 210:
			sprintf( buf, "%s", "Request Entity Too Large" );
            break;

		case 211:
			sprintf( buf, "%s", "Request Pending" );
            break;

		case 212:
			sprintf( buf, "%s", "Request Terminated" );
            break;

		case 213:
			sprintf( buf, "%s", "Request Timeout" );
            break;

		case 214:
			sprintf( buf, "%s", "Request-URI Too Long" );
            break;

		case 215:
			sprintf( buf, "%s", "required" );
            break;
		
		case 216:
			sprintf( buf, "%s", "response" );
            break;

		case 217:
			sprintf( buf, "%s", "retry-after" );
            break;
		
		case 218:
			sprintf( buf, "%s", "Ringing" );
            break;

		case 219:
			sprintf( buf, "%s", "robust-sorting" );
            break;

		case 220:
			sprintf( buf, "%s", "rpsi" );
            break;

		case 221:
			sprintf( buf, "%s", "rtcp-fb" );
            break;

		case 222:
			sprintf( buf, "%s", "RTP/AVPF" );
            break;

		case 223:
			sprintf( buf, "%s", "RTP/SAVP" );
            break;

		case 224:
			sprintf( buf, "%s", "rtpmap" );
            break;

		case 225:
			sprintf( buf, "%s", "Sat" );
            break;
	
		case 226:
			sprintf( buf, "%s", "sctp" );
            break;

		case 227:
			sprintf( buf, "%s", "SCTP" );
            break;

		case 228:
			sprintf( buf, "%s", "sdp" );
            break;

		case 229:
			sprintf( buf, "%s", "sdplang" );
            break;

		case 230:
			sprintf( buf, "%s", "sec-agree" );
            break;

		case 231:
			sprintf( buf, "%s", "Security Agreement Required" );
            break;

		case 232:
			sprintf( buf, "%s", "send" );
            break;

		case 233:
			sprintf( buf, "%s", "sendonly" );
            break;

		case 234:
			sprintf( buf, "%s", "sendrecv" );
            break;

		case 235:	
			sprintf( buf, "%s", "Sep" );
            break;

		case 236:
			sprintf( buf, "%s", "Server Internal Error" );
            break;

		case 237:
			sprintf( buf, "%s", "Server Time-out" );
            break;

		case 238:
			sprintf( buf, "%s", "Service Unavailable" );
            break;

		case 239:
			sprintf( buf, "%s", "session" );
            break;
		
		case 240:
			sprintf( buf, "%s", "Session Interval Too Small" );
            break;

		case 241:
			sprintf( buf, "%s", "Session Progress" );
            break;
		
		case 242:
			sprintf( buf, "%s", "sipfrag" );
            break;

		case 243:
			sprintf( buf, "%s", "sips" );
            break;
		
		case 244:
			sprintf( buf, "%s", "sli" );
            break;
		
		case 245:
			sprintf( buf, "%s", "smime" );
            break;
		
		case 246:
			sprintf( buf, "%s", "stale" );
            break;

		case 247:
			sprintf( buf, "%s", "Sun" );
            break;

		case 248:
			sprintf( buf, "%s", "tag" );
            break;

		case 249:
			sprintf( buf, "%s", "TCP" );
            break;

		case 250:
			sprintf( buf, "%s", "tcp" );
            break;

		case 251:
			sprintf( buf, "%s", "tel" );
            break;

		case 252:
			sprintf( buf, "%s", "telephone-event" );
            break;

		case 253:
			sprintf( buf, "%s", "Temporarily Unavailable" );
            break;

		case 254:
			sprintf( buf, "%s", "terminated" );
            break;

		case 255:
			sprintf( buf, "%s", "text" );
            break;

		case 256:
			sprintf( buf, "%s", "Thu" );
            break;

		case 257:
			sprintf( buf, "%s", "timeout" );
            break;

		case 258:
			sprintf( buf, "%s", "timer" );
            break;

		case 259:
			sprintf( buf, "%s", "tls" );
            break;

		case 260:
			sprintf( buf, "%s", "TLS" );
            break;

		case 261:
			sprintf( buf, "%s", "tone" );
            break;

		case 262:	
			sprintf( buf, "%s", "Too Many Hops" );
            break;

		case 263:
			sprintf( buf, "%s", "tool" );
            break;

		case 264:
			sprintf( buf, "%s", "to-tag" );
            break;

		case 265:
			sprintf( buf, "%s", "transport" );
            break;

		case 266:
			sprintf( buf, "%s", "true" );
            break;

		case 267:
			sprintf( buf, "%s", "TRUE" );
            break;

		case 268:
			sprintf( buf, "%s", "True" );
            break;

		case 269:
			sprintf( buf, "%s", "Trying" );
            break;

		case 270:
			sprintf( buf, "%s", "ttl" );
            break;

		case 271:
			sprintf( buf, "%s", "ttr-int" );
            break;

		case 272:
			sprintf( buf, "%s", "Tue" );
            break;

		case 273:
			sprintf( buf, "%s", "type:broadcast" );
            break;

		case 274:
			sprintf( buf, "%s", "type:H.332" );
            break;

		case 275:
			sprintf( buf, "%s", "type:meeting" );
            break;

		case 276:
			sprintf( buf, "%s", "type:moderated" );
            break;

		case 277:
			sprintf( buf, "%s", "type:recvonly" );
            break;

		case 278:
			sprintf( buf, "%s", "type:test" );
            break;

		case 279:
			sprintf( buf, "%s", "uac" );
            break;

		case 280:
			sprintf( buf, "%s", "uas" );
            break;

		case 281:
			sprintf( buf, "%s", "UDP" );
            break;
			
		case 282:
			sprintf( buf, "%s", "udp" );
            break;

		case 283:
			sprintf( buf, "%s", "Unauthorized" );
            break;

		case 284:
			sprintf( buf, "%s", "Undecipherable" );
            break;

		case 285:
			sprintf( buf, "%s", "unknown" );
            break;

		case 286:
			sprintf( buf, "%s", "Unsupported Media Type" );
            break;

		case 287:
			sprintf( buf, "%s", "Unsupported URI Scheme" );
            break;

		case 288:
			sprintf( buf, "%s", "urgent" );
            break;

		case 289:
			sprintf( buf, "%s", "uri" );
            break;

		case 290:
			sprintf( buf, "%s", "Use Proxy" );
            break;

		case 291:
			sprintf( buf, "%s", "user" );
            break;
		
		case 292:
			sprintf( buf, "%s", "username" );
            break;

		case 293:
			sprintf( buf, "%s", "Version Not Supported" );
            break;

		case 294:
			sprintf( buf, "%s", "video" );
            break;

		case 295:
			sprintf( buf, "%s", "Wed" );
            break;

		case 296:
			sprintf( buf, "%s", "xml" );
            break;

		default:
			dAppLog( LOG_DEBUG, "[%s] Unknown directory index[%d]", 
				__FUNCTION__, flag==0x01?index-1000:index);

	} // end of switch

	return strlen(buf);
}

int iGet_UnknownMethod( const unsigned char method, char *buf )
{
	return 0;
}

int iGet_HeaderNumber( char *buf, const unsigned char number )
{
	switch((int) number )
	{
		case 1:
			sprintf( buf, "Accpet" );
			break;

		case 2:
			sprintf( buf, "Accept-Contact" );
			break;  

		case 3:
			sprintf( buf, "Accept-Encording" );
			break;

		case 4:
			sprintf( buf, "Accept-Language" );
			break;

		case 5:
			sprintf( buf, "Alter-Info" );
			break;

		case 6:
		case 224:  //E0
			sprintf( buf, "Allow" );
			break;

		case 7:
		case 225:
			sprintf( buf, "Allow-Events" );
			break;

		case 8:
			sprintf( buf, "Authenticate" );
			break;

		case 9:
			sprintf( buf, "Authentication-Info" );
			break;

		case 10:
		case 226:
			sprintf( buf, "Authorization" );
			break;

		case 11:
		case 227:
			sprintf( buf, "Call-ID" );
			break;

		case 12:
			sprintf( buf, "Call-Info" );
			break;

		case 13:
			sprintf( buf, "Conference" );
			break;

		case 14:
		case 228:
			sprintf( buf, "Contact" );
			break;

		case 15:
			sprintf( buf, "Content-Disposition" );
			break;

		case 16:
			sprintf( buf, "Content-Encoding" );
			break;

		case 17:
			sprintf( buf, "Content-Language" );
			break;

		case 18:
		case 229:
			sprintf( buf, "Content-Length" );
			break;

		case 19:
		case 230:
			sprintf( buf, "Content-Type" );
			break;

		case 20:
			sprintf( buf, "Content-Version" );
			break;

		case 21:
		case 231:
			sprintf( buf, "CSeq" );
			break;

		case 22:
			sprintf( buf, "Data" );
			break;

		case 23:
			sprintf( buf, "Encryption" );
			break;

		case 24:
			sprintf( buf, "Error-Info" );
			break;

		case 25:
		case 232:
			sprintf( buf, "Event" );
			break;

		case 26:
		case 233:
			sprintf( buf, "Expires" );
			break;

		case 27:
		case 234:
			sprintf( buf, "From" );
			break;

		case 28:
			sprintf( buf, "Hide" );
			break;

		case 29:
		case 235:
			sprintf( buf, "In-Reply-To" );
			break;

		case 30:
		case 236:
			sprintf( buf, "Max-Forwards" );
			break;

		case 31:
			sprintf( buf, "MIME-Version" );
			break;

		case 32:
			sprintf( buf, "Min-Expires" );
			break;

		case 33:
			sprintf( buf, "Min-SE" );
			break;

		case 34:
			sprintf( buf, "Organization" );
			break;

		case 35:
			sprintf( buf, "P-Access-Network-Info" );
			break;

		case 36:
			sprintf( buf, "P-Asserted-Identity" );
			break;

		case 37:
			sprintf( buf, "P-Associated-URI" );
			break;

		case 38:
			sprintf( buf, "P-Called-Party-ID" );
			break;

		case 39:
			sprintf( buf, "P-Charging-Function-Address" );
			break;

		case 40:
			sprintf( buf, "P-Charging-Vector" );
			break;

		case 41:
			sprintf( buf, "P-Media-Authorization" );
			break;

		case 42:
			sprintf( buf, "P-Preferred-Identity" );
			break;
		
		case 43:
			sprintf( buf, "P-Visited-Network-ID" );
			break;

		case 44:
			sprintf( buf, "Path" );
			break;

		case 45:
			sprintf( buf, "Priority" );
			break;

		case 46:
		case 237:
			sprintf( buf, "Privacy" );
			break;

		case 47:
		case 238:
			sprintf( buf, "Proxy-Authenticate" );
			break;

		case 48:
		case 239:
			sprintf( buf, "Proxy-Authorization" );
			break;

		case 49:
			sprintf( buf, "Proxy-Require" );
			break;

		case 50:
		case 240:
			sprintf( buf, "RAck" );
			break;

		case 51:
			sprintf( buf, "Reason" );
			break;

		case 52:
		case 241:
			sprintf( buf, "Recipient" );
			break;

		case 53:
		case 242:
			sprintf( buf, "record-Route" );
			break;

		case 54:
			sprintf( buf, "Referred-By" );
			break;

		case 55:
			sprintf( buf, "Refer-To" );
			break;

		case 56:
			sprintf( buf, "Reject-Contact" );
			break;

		case 57:
			sprintf( buf, "Replaces" );
			break;

		case 58:
		case 243:
			sprintf( buf, "Reply-To" );
			break;

		case 59:
			sprintf( buf, "Request-Disposition" );
			break;

		case 60:
		case 244:
			sprintf( buf, "Require" );
			break;

		case 61:
			sprintf( buf, "Require-Contact" );
			break;

		case 62:
			sprintf( buf, "Response-Key" );
			break;

		case 63:
			sprintf( buf, "Retry-After" );
			break;

		case 64:
		case 245:
			sprintf( buf, "Route" );
			break;

		case 65:
		case 246:
			sprintf( buf, "RSeq" );
			break;

		case 66:
		case 247:
			sprintf( buf, "Security-Client" );
			break;

		case 67:
		case 248:
			sprintf( buf, "Security-Server" );
			break;

		case 68:
		case 249:
			sprintf( buf, "Security-verify" );
			break;

		case 69:
			sprintf( buf, "Server" );
			break;

		case 70:
			sprintf(buf, "Service-Route" );
			break;

		case 71:
			sprintf( buf, "Session" );
			break;

		case 72:
			sprintf( buf, "Session-Expires" );
			break;

		case 73:
			sprintf( buf, "Subject" );
			break;

		case 74:
			sprintf( buf, "Subscription-Expires" );
			break;

		case 75:
			sprintf( buf, "Subscription-State" );
			break;

		case 76:
		case 250:
			sprintf( buf, "Supported" );
			break;

		case 77:
		case 251:
			sprintf( buf, "Timestamp" );
			break;

		case 78:
		case 252:
			sprintf( buf, "To" );
			break;

		case 79:
			sprintf( buf, "Unsupported" );
			break;

		case 80:
		case 253:
			sprintf( buf, "User-Agent" );
			break;

		case 81:
		case 254:
			sprintf( buf, "Via" );
			break;

		case 82:
			sprintf( buf, "Warning" );
			break;

		case 83:
		case 255:
			sprintf( buf, "WWW-Authenticate" );
			break;

		default:
			dAppLog( LOG_DEBUG, "[%s] Unknown header number [%d]", 
				__FUNCTION__, number );
			break;

	} // end of switch

	return strlen(buf);
}

/* ************************************************** 
 * $Log
 * ************************************************** */
