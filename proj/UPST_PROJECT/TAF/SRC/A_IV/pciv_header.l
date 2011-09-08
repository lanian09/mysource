/* Full Viewer Parsing Test */

%{
	#include <stdio.h>
	#include <string.h>
	#include <typedef.h>
	#include <common_stg.h>
	#include <pciv_func.h> 
	int		*pVer;
%}

%%

^ZACO0100 								{ 
											*pVer = 100;
											return 0;
										}

^ZACO0101 								{ 
											*pVer = 101;
											return 0;
										}

^ZACO0102 								{ 
											*pVer = 102;
											return 0;
										}

(\r\n)									{	}

[ \t\n]									{	}

.										{	}


%%

int pciv_headerwrap(void)
{
	return 1;
}

int pciv_header(char *sp, int slen, int *version)
{
	pVer = version;

	pciv_header_scan_bytes(sp, slen);

	pciv_headerlex();

	pciv_header_delete_buffer(yy_current_buffer);
	yy_init = 1;

	return 0;
}

#ifdef TEST
int main(void)
{
	int iRet = -1;
	char szBuf[1024*10];
	int	version = 0;

	char *sp;
	int	slen;

	pVer = &version;

	sprintf(szBuf, "ZACO0101");

	sp = szBuf;
	slen = strlen(szBuf);

	pciv_header_scan_bytes(sp, slen);
	
	iRet = pciv_headerlex();

	printf("RETURN [%d]\n", iRet);
	printf("ORG=%s VER=%d\n", szBuf, version);
	return 0;
}
#endif
