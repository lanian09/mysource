/* HTTP Parsing Test */

%{
	#include <stdio.h>
	#include <string.h>

	#include "http.h"
	#include "httpheader.tab.h"

	void httpheadererror(char *);

	extern char *pucLogUrl;
	extern int	*pdLogLen;
	extern int	*pdRes;
%}

URL1		[^\r\n \t]+
URL2		[0-9a-zA-Z\xA1-\xFE/`:;._&%?=-]+
STR1		[0-9a-zA-Z\xA1-\xFE:;/._&%?=-]+		
STR2		[ 0-9a-zA-Z\xA1-\xFE:;/._&%?=-]+		
HOST2       [^\r\n \t:/]+
HOST3       [0-9a-zA-Z.:]+

%%

^[ \t]*GET								{
											return GET;
										}

^[ \t]*POST								{
											return POST;
										}

^[ \t]*HEAD								{
											return HEAD;
										}

^[ \t]*OPTIONS							{
											return OPTIONS;
										}

^[ \t]*PUT								{
											return PUT;
										}

^[ \t]*DELETE							{
											return DELETE;
										}

^[ \t]*TRACE							{
											return TRACE;
										}

^[ \t]*CONNECT							{
											return CONNECT;
										}

^[ \t]*RESULT							{
											return RESULT;
										}

^[ \t]*DESCRIBE							{
											return DESCRIBE;
										}

^[ \t]*ANNOUNCE							{
											return ANNOUNCE;
										}

^[ \t]*GET_PARAMETER					{
											return GET_PARAMETER;
										}

^[ \t]*PAUSE							{
											return PAUSE;
										}

^[ \t]*PLAY								{
											return PLAY;
										}

^[ \t]*RECORD							{
											return RECORD;
										}

^[ \t]*REDIRECT							{
											return REDIRECT;
										}

^[ \t]*SETUP							{
											return SETUP;
										}

^[ \t]*SET_PARAMETER					{
											return SET_PARAMETER;
										}

^[ \t]*TEARDOWN							{
											return TEARDOWN;
										}

http\:\/\/{HOST2}:[0-9]*{URL1}			{
											*pdLogLen = (yyleng > MAX_LOGURL_LEN) ? MAX_LOGURL_LEN : yyleng;
											memcpy(pucLogUrl, yytext, *pdLogLen);
											pucLogUrl[*pdLogLen] = 0x00;
											return URL1;
										}

http\:\/\/{HOST2}{URL1}					{
											*pdLogLen = (yyleng > MAX_LOGURL_LEN) ? MAX_LOGURL_LEN : yyleng;
											memcpy(pucLogUrl, yytext, *pdLogLen);
											pucLogUrl[*pdLogLen] = 0x00;
											return URL2;
										}

http\:\/\/{URL1}						{
											*pdLogLen = (yyleng > MAX_LOGURL_LEN) ? MAX_LOGURL_LEN : yyleng;
											memcpy(pucLogUrl, yytext, *pdLogLen);
											pucLogUrl[*pdLogLen] = 0x00;
											return URL2;
										}

\/{URL1}								{
											*pdLogLen = (yyleng > MAX_LOGURL_LEN) ? MAX_LOGURL_LEN : yyleng;
											memcpy(pucLogUrl, yytext, *pdLogLen);
											pucLogUrl[*pdLogLen] = 0x00;
											return URL3;
										}

\/										{
											*pdLogLen = (yyleng > MAX_LOGURL_LEN) ? MAX_LOGURL_LEN : yyleng;
											memcpy(pucLogUrl, yytext, *pdLogLen);
											pucLogUrl[*pdLogLen] = 0x00;
											return URL3;
										}

rtsp\:\/\/{HOST2}:[0-9]*{URL1}			{
											*pdLogLen = (yyleng > MAX_LOGURL_LEN) ? MAX_LOGURL_LEN : yyleng;
											memcpy(pucLogUrl, yytext, *pdLogLen);
											pucLogUrl[*pdLogLen] = 0x00;
											return URL1;
										}

rtsp\:\/\/{HOST2}{URL1}					{
											*pdLogLen = (yyleng > MAX_LOGURL_LEN) ? MAX_LOGURL_LEN : yyleng;
											memcpy(pucLogUrl, yytext, *pdLogLen);
											pucLogUrl[*pdLogLen] = 0x00;
											return URL2;
										}

(\r\n)									{
											return LFCR;
										}

[ \t\n]									;

^[ \t]*HTTP\/1\.[0-9]+					{
											return RESP;
										}

^[ \t]*RTSP\/1\.[0-9]+					{
											return RESP;
										}

HTTP\/1\.[0-9]+							{
											return HTTP;
										}

RTSP\/1\.[0-9]+							{
											return RTSP;
										}

[0-9]+									{
											*pdRes = atoi(yytext);
											return RESCODE;
										}

{HOST3}									{
											*pdLogLen = (yyleng > MAX_LOGURL_LEN) ? MAX_LOGURL_LEN : yyleng;
											memcpy(pucLogUrl, yytext, *pdLogLen);
											pucLogUrl[*pdLogLen] = 0x00;
											return URL4;
										}

.										return ERROR;


%%

int httpheaderwrap(void)
{
	return 1;
}

void ReleasehttpheaderBuffer(void)
{
 /*
  Preventing heap memory leak in Flex.  - For non-reentrant C scanner only.

  Leakage of Heap memory is known bug of Flex,
  We can solve this problem by forced release of Flex's buffer memory when yylex( ) is no more needed.

  Visit below web page for more information.
  http://lists.gnu.org/archive/html/help-flex/2002-06/msg00012.html
 */



 httpheader_delete_buffer(yy_current_buffer);
 yy_init = 1;
}
