/* HTTP Parsing Test */

%{
	#include <stdio.h>
	#include <string.h>

	#include "http.h"

	char *ucHost;
	int	*hostLen;
%}

URL1		[^\r\n \t]+
STR1		[0-9a-zA-Z\xA1-\xFE:;/._&%?=-]+		
STR2		[ 0-9a-zA-Z\xA1-\xFE:;/._&%?=-]+		
HOST2		[^\r\n \t/]+
HOST1		[0-9a-zA-Z\xA1-\xFE:;/._&%?=-]+		

%%

http:\/\/{HOST2}						{
											*hostLen = (yyleng-7 > MAX_HOSTNAME_LEN) ? MAX_HOSTNAME_LEN : yyleng - 7;
											memcpy(ucHost, &yytext[7], *hostLen);
											ucHost[*hostLen] = 0x00;
										}	
\r                                      {   }
\n                                      {   }
.										{	}


%%

int httphostwrap(void)
{
	return 1;
}

int httphost(char *sp, int slen, char *szHost, int *pHostLen)
{
	ucHost = szHost;
	hostLen = pHostLen;

	httphost_scan_bytes(sp, slen);

	httphostlex();

	httphost_delete_buffer(yy_current_buffer);
	yy_init = 1;

	return 0;
}

#ifdef TEST
int main(void)
{
	int iRet = -1;
	char szBuf[1024*10];
	char szHost[1024];
	int		dLen;

	char *sp;
	int	slen;

	ucHost = szHost;
	hostLen = &dLen;

	//sprintf(szBuf, "GET http://mvod.magicn.com:80/KUN/00/00/00/72/11/721141_1.asp?menuid=721141 HTTP/1.1\r\nHost: magicn.com\r\nContent-Type: text/html\r\nContent-Length: 127\r\n");
	sprintf(szBuf, "GET http://mvod.magicn.com: HTTP/1.1\r\nHost: magicn.com\r\nContent-Type: text/html\r\nContent-Length: 127\r\n");

	sp = szBuf;
	slen = strlen(szBuf);

	httphost_scan_bytes(sp, slen);
	
	iRet = httphostlex();
	printf("RETURN [%d]\n", iRet);
	printf("HOST: [%s]\n", ucHost);
	return 0;
}
#endif
