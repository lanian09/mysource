/* HTTP Parsing Test */

%{
	#include <stdio.h>
	#include <string.h>

	#include "http.h"

	int		*dn;
%}

PORT	[0-9:]*

%%

http:\/\/[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+{PORT}\/download\.dn\?		{
															*dn = 1;
												}

\r                                      {   }
\n                                      {   }
.										{	}


%%

int a2g_dnwrap(void)
{
	return 1;
}

int a2g_dn(char *sp, int slen, int *pddn)
{
	dn = pddn;

	a2g_dn_scan_bytes(sp, slen);

	a2g_dnlex();

	a2g_dn_delete_buffer(yy_current_buffer);
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

	sprintf(szBuf, "http://192.168.239.135/veri.dn?");

	dn = &ddn;

	sp = szBuf;
	slen = strlen(szBuf);

	a2g_dn_scan_bytes(sp, slen);
	
	iRet = a2g_dnlex();
	printf("RETURN=%d\n", iRet);
	printf("dn=%d\n", ddn);
	return 0;
}
#endif
