/* HTTP Parsing Test */

%{
	#include <stdio.h>
	#include <string.h>

	#include "http.h"

	int	*pCType;
	int	*pMultiLen;
	char *ucMulti;
%}

STR9	[^\r\n \t\"]+

%s		MULTIPART
%s		BOUNDARY

%%

text\/html								{
											*pCType = 1;
										}

text\/x-html							{
											*pCType = 1;
										}

text\/vnd.wap.wml						{
											*pCType = 1;
										}

application\/xhtml+wml					{
											*pCType = 1;
										}

application\/x\-www\-form\-urlencoded	{
											*pCType = 1;
										}

text\/css								{
											*pCType = 1;
										}

audio\/k3g								{
											*pCType = 2;
										}

audio\/ak3g								{
											*pCType = 2;
										}

video\/k3g								{
											*pCType = 2;
										}

video\/ak3g								{
											*pCType = 2;
										}

multipart\/related						{
											*pCType = 3;
											BEGIN MULTIPART;
										}

<MULTIPART>boundary[ \t]*=[ \t]*\"		{
											BEGIN 0;
											BEGIN BOUNDARY;
										}

<BOUNDARY>{STR9}						{
											*pMultiLen = (yyleng > (MAX_MULTIPART_LEN - 4)) ? (MAX_MULTIPART_LEN - 4) : yyleng;
											ucMulti[0] = 0x2D;
											ucMulti[1] = 0x2D;
											memcpy(&ucMulti[2], yytext, *pMultiLen);
											(*pMultiLen) += 2;
											ucMulti[(*pMultiLen)++] = 0x2D;
											ucMulti[(*pMultiLen)++] = 0x2D;
											ucMulti[*pMultiLen] = 0x00;
											BEGIN 0;
										}

application\/sdp						{
											*pCType = 4;
										}

text\/xml								{
											*pCType = 5;
										}

\r                                      {   }
\n                                      {   }
.										{	}


%%

int httpctypewrap(void)
{
	return 1;
}

int httpctype(char *sp, int slen, int *CType, int *multilen, char *multi)
{
	pCType = CType;
	pMultiLen = multilen;
	ucMulti = multi;

	httpctype_scan_bytes(sp, slen);

	httpctypelex();

	httpctype_delete_buffer(yy_current_buffer);
	yy_init = 1;

	return 0;
}

#ifdef TEST
int main(void)
{
	int iRet = -1;
	char szBuf[1024*10];
	int	ctype = 0;
	int	multilen = 0;
	char szMulti[MAX_MULTIPART_SIZE];

	char *sp;
	int	slen;

	sprintf(szBuf, "application/x-www-form-urlencoded");

	pCType = &ctype;
	pMultiLen = &multilen;
	ucMulti = szMulti;
	ucMulti[0] = 0x00;

	sp = szBuf;
	slen = strlen(szBuf);

	httpctype_scan_bytes(sp, slen);
	
	iRet = httpctypelex();
	printf("RETURN [%d]\n", iRet);
	printf("Content-Type: [%d]\n", ctype);
	printf("Multi Len: [%d]\n", multilen);
	printf("MultiPart: [%s]\n", szMulti);
	return 0;
}
#endif
