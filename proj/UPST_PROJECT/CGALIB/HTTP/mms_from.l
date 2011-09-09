/* HTTP Parsing Test */

%{
	#include <stdio.h>
	#include <string.h>

	#include "http.h"

	char	*from;
	int		len;
%}

STR		[^\r\n \t]+

%s		FROM

%%

FROM[ \t]*:[ \t]*						{
											BEGIN FROM;
										}

<FROM>{STR}								{
											len = (yyleng > MAX_MIN_LEN) ? MAX_MIN_LEN : yyleng;
											memcpy(from, yytext, len);
											from[len] = 0x00;	
											BEGIN 0;
											return 0;
										}

\r                                      {   }
\n                                      {   }
.										{	}


%%

int mms_fromwrap(void)
{
	return 1;
}

int mms_from(char *sp, int slen, char *szFrom)
{
	from = szFrom;

	mms_from_scan_bytes(sp, slen);

	mms_fromlex();

	mms_from_delete_buffer(yy_current_buffer);
	yy_init = 1;

	return 0;
}

#ifdef TEST
int main(void)
{
	int iRet = -1;
	char szBuf[1024*10];
	char szFrom[BUFSIZ];

	char *sp;
	int	slen;

	sprintf(szBuf, "audio/k3g");

	from = szFrom;

	sp = szBuf;
	slen = strlen(szBuf);

	mms_from_scan_bytes(sp, slen);
	
	iRet = mms_fromlex();
	printf("RETURN=%d\n", iRet);
	printf("FROM=%s\n", szFrom);
	return 0;
}
#endif
