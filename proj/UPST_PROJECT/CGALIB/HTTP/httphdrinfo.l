/* HTTP Parsing Test */

%{
	#include <stdio.h>
	#include <string.h>

	#include "http.h"

	char *ucHost;
	char *ucCType;
	int *pCLen;
	int	*pChunked;
	int *pCTypeLen;
	int *pHostLen;
	int *pCEnco;
	int *pPktCnt;
%}

URL			[0-9a-zA-Z\xA1-\xFE/`:;._&%?=-]+
STR1		[0-9a-zA-Z\xA1-\xFE:;/._&%?=-]+		
STR2		[ 0-9a-zA-Z\xA1-\xFE:;/._&%?=-]+		
STR3		[^\r\n \t;]+
STR4		[^\r\n \t]+
STR5		[^\r\n]+
STR6		[^\r\n \t-]+
STR7		[0-9]+

%s			HOST
%s			CTYPE
%s			CLEN
%s			CENCO
%s			PCNT

%%

Host[ \t]*:[ \t]* 						{ 
											BEGIN HOST;
										}

<HOST>{STR4}							{
											*pHostLen = (yyleng > MAX_HOSTNAME_LEN) ? MAX_HOSTNAME_LEN : yyleng;
											memcpy(ucHost, yytext, *pHostLen);
											ucHost[*pHostLen] = 0x00;
											BEGIN 0;
										}

Content-Type[ \t]*:[ \t]* 				{ 
											BEGIN CTYPE;
										}


<CTYPE>{STR5}							{
											*pCTypeLen = (yyleng > MAX_TEMP_CONTENT_LEN) ? MAX_TEMP_CONTENT_LEN : yyleng;
											memcpy(ucCType, yytext, *pCTypeLen);
											ucCType[*pCTypeLen] = 0x00;
											BEGIN 0;
										}

Content-Length[ \t]*:[ \t]* 			{ 
											BEGIN CLEN;
										}

<CLEN>{STR4}							{
											*pCLen = atoi(yytext);
											BEGIN 0;
										}

Transfer-Encoding[ \t]*:[ \t]*chunked	{
											*pChunked = 1;	
										}

Content-Encoding[ \t]*:[ \t]*			{
											BEGIN CENCO;
										}

<CENCO>gzip								{
											*pCEnco = DEF_ZIP_GZIP;
											BEGIN 0;
										}

<CENCO>deflate							{
											*pCEnco = DEF_ZIP_DEFLATE;
											BEGIN 0;
										}

Packet-Counter[ \t]*:[ \t]*{STR6}-{STR6}-{STR6}-	{
											BEGIN PCNT;
										}

<PCNT>{STR7}							{
											*pPktCnt = atoi(yytext);
											BEGIN 0;
										}

(\r\n)									{	}

[ \t\n]									{	}

.										{	}


%%

int httphdrinfowrap(void)
{
	return 1;
}

int httphdrinfo(char *sp, int slen, char *szHost, int *pdHostLen, char *szCType, int *pdCTypeLen, int *pdCLen, int *pdChunked, int *pdCEnco, int *pdPktCnt)
{
	ucHost = szHost;
	ucCType = szCType;
	pCLen = pdCLen;
	pChunked = pdChunked;	
	pCTypeLen = pdCTypeLen;
	pHostLen = pdHostLen;
	pCEnco = pdCEnco;
	pPktCnt = pdPktCnt;

	httphdrinfo_scan_bytes(sp, slen);

	httphdrinfolex();

	httphdrinfo_delete_buffer(yy_current_buffer);
	yy_init = 1;

	return 0;
}

#ifdef TEST
int main(void)
{
	int iRet = -1;
	char szBuf[1024*10];
	char szHost[1024];
	char szCType[1024];
	int	dCTypeLen = 0;
	int	dCLen = 0;
	int dChunked = 0;
	int dHostLen = 0;
	int dCEnco = 0;
	int	dPktCnt = 0;

	char *sp;
	int	slen;

	ucHost = szHost;
	ucCType = szCType;
	pCLen = &dCLen;
	pChunked = &dChunked;	
	pCTypeLen = &dCTypeLen;
	pHostLen = &dHostLen;	
	pCEnco = &dCEnco;
	pPktCnt = &dPktCnt;

	sprintf(szBuf, "GET http://mvod.magicn.com/KUN/00/00/00/72/11/721141_1.asp?menuid=721141 HTTP/1.1\r\nHost :61.23.23.11\r\nContent-Type:text/html; charset=euc-kr\r\nContent-Length : 127\r\nPacket-Counter: 401002612404-LIC4015325-4016157331-1480202-6401-205--0--\r\n");

	sp = szBuf;
	slen = strlen(szBuf);

	httphdrinfo_scan_bytes(sp, slen);
	
	iRet = httphdrinfolex();
	printf("RETURN [%d]\n", iRet);
	printf("HOST: [%s][%d]\n", ucHost, dHostLen);
	printf("Content-Type: [%s][%d]\n", ucCType, dCTypeLen);
	printf("Content-Length: [%d]\n", dCLen);
	printf("Packet-Counter: [%d]\n", dPktCnt);
	return 0;
}
#endif
