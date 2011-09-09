/* ACCEPT_CONTACT */

%{
	#include <stdio.h>
	#include <string.h>
	
	#include "common_stg.h"
	#include "sip.h"

	int				*pSERVICE;
	int				len;
%}

%%


lgt-mim								{
										*pSERVICE = DEF_PLATFORM_IM;
										return 0;
									}

(\r\n)								{	}

[ \t\n]								{	}

.									{	}


%%

int sip_servicewrap(void)
{
	return 1;
}

int sip_service(char *sp, int slen, int *dSvcType)
{
	pSERVICE = dSvcType;

	sip_service_scan_bytes(sp, slen);

	sip_servicelex();

	sip_service_delete_buffer(yy_current_buffer);
	yy_init = 1;

	return 0;
}

#ifdef TEST
int main(void)
{
	int iRet = -1;
	char szBuf[BUFSIZ];
	int dSvcType = 0;

	char *sp;
	int	slen;

	pSERVICE = &dSvcType;

//	sprintf(szBuf, "*;lgt-mim");
	sprintf(szBuf, "Accept-Contact: *;description=\"VT\";explicit;require");

	sp = szBuf;
	slen = strlen(szBuf);

	sip_service_scan_bytes(sp, slen);
	
	sip_servicelex();

	printf("SERVICE [%d]\n", dSvcType);

	return 0;
}
#endif
