/* HTTP Parsing Test */

%{
	#include <stdio.h>
	#include <string.h>

	#include "http.h"

	int		*dn;
%}

PORT	[0-9:]*

%%

http:\/\/[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+{PORT}\/\?		{
															*dn = 1;
												}

\r                                      {   }
\n                                      {   }
.										{	}


%%

int mms_to_dnwrap(void)
{
	return 1;
}

int mms_to_dn(char *sp, int slen, int *pddn)
{
	dn = pddn;

	mms_to_dn_scan_bytes(sp, slen);

	mms_to_dnlex();

	mms_to_dn_delete_buffer(yy_current_buffer);
	yy_init = 1;

	return 0;
}

#ifdef TEST
int main(void)
{
	int iRet = -1;
	char szBuf[1024*10];
	int	ddn = 0;

	char *sp;
	int	slen;

	sprintf(szBuf, "http://192.168.239.135/?");

	dn = &ddn;

	sp = szBuf;
	slen = strlen(szBuf);

	mms_to_dn_scan_bytes(sp, slen);
	
	iRet = mms_to_dnlex();
	printf("RETURN=%d\n", iRet);
	printf("dn=%d\n", ddn);
	return 0;
}
#endif
