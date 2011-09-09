/* HTTP Parsing Test */

%{
	#include <stdio.h>
	#include <string.h>

	#include "http.h"

	char	*to;
	int		len;
%}

STR		[^\r\n \t]+

%s		TO

%%

TO[ \t]*:[ \t]*							{
											BEGIN TO;
										}

<TO>{STR}								{
											len = (yyleng > MAX_MIN_LEN) ? MAX_MIN_LEN : yyleng;
											memcpy(to, yytext, len);
											to[len] = 0x00;	
											BEGIN 0;
											return 0;
										}

\r                                      {   }
\n                                      {   }
.										{	}


%%

int mms_towrap(void)
{
	return 1;
}

int mms_to(char *sp, int slen, char *szTo)
{
	to = szTo;

	mms_to_scan_bytes(sp, slen);

	mms_tolex();

	mms_to_delete_buffer(yy_current_buffer);
	yy_init = 1;

	return 0;
}

#ifdef TEST
int main(void)
{
	int iRet = -1;
	char szBuf[1024*10];
	char szTo[BUFSIZ];

	char *sp;
	int	slen;

	sprintf(szBuf, "audio/k3g");

	to = szTo;

	sp = szBuf;
	slen = strlen(szBuf);

	mms_to_scan_bytes(sp, slen);
	
	iRet = mms_tolex();
	printf("RETURN=%d\n", iRet);
	printf("FROM=%s\n", szTo);
	return 0;
}
#endif
