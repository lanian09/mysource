/* HTTP Parsing Test */

%{
	#include <stdio.h>
	#include <string.h>

	#include "http.h"

	char *ucCType;
	int	*ctypeLen;
%}

STR			[^=&;: \t\r\n]+

%%

{STR}									{
											*ctypeLen = (yyleng > MAX_CONTENTSTYPE_SIZE) ? MAX_CONTENTSTYPE_SIZE : yyleng;
											memcpy(ucCType, &yytext[0], *ctypeLen);
											ucCType[*ctypeLen] = 0x00;
											return 0;
										}	
\r                                      {   }
\n                                      {   }
.										{	}


%%

int httpctypeinfowrap(void)
{
	return 1;
}

int httpctypeinfo(char *sp, int slen, char *szCType, int *pCTypeLen)
{
	ucCType = szCType;
	ctypeLen = pCTypeLen;

	httpctypeinfo_scan_bytes(sp, slen);

	httpctypeinfolex();

	httpctypeinfo_delete_buffer(yy_current_buffer);
	yy_init = 1;

	return 0;
}

#ifdef TEST
int main(void)
{
	int iRet = -1;
	char szBuf[1024*10];
	char szCType[1024];
	int		dLen;

	char *sp;
	int	slen;

	ucCType = szCType;
	ctypeLen = &dLen;

	sprintf(szBuf, "text/javascript; charset=utf-8");

	sp = szBuf;
	slen = strlen(szBuf);

	httpctypeinfo_scan_bytes(sp, slen);
	
	iRet = httpctypeinfolex();
	printf("RETURN [%d]\n", iRet);
	printf("CTYPE: [%s]\n", ucCType);
	return 0;
}
#endif
