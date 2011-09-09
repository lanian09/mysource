%{
	#include <stdio.h>
	#include <stdlib.h>
	#include "re_define.h"
	
	#define  MAX_HOST_SIZE 	128

	int		*pRet;
	int		len;
	char	*pHostname;
%}

%s		GET
%s 		HOST
%s		REDIRECT
%s 		HEADER

%%
[ \t]*SELECT=OK[ \t]* 		{
								*pRet=1;
							}

(\r\n)                                  {   }
 
[ \t\n]                                 {   }
 
.                                       {   }

%%

int parsedatawrap(void)
{
	return 1;
}

int parsedata(char *sp, int slen)
{
	int			dFlag=0;

	pRet		= &dFlag;

#if 0
	printf("uiCmdType[%d], uiPeriod[%d], uiStartTime[%d], szIMSI[%s]\n", 
			*uiCmdType, *uiPeriod, *uiStartTime, szIMSI);
#endif

	parsedata_scan_bytes(sp, slen);
	parsedatalex();

	return *pRet;
}

#ifdef	TEST
int main(int argc, char *argv[])
{	
	int		dFlag = 0;

	char	*sp;
	int		slen;

	pRet		= &dFlag;

	if (argc < 2) {
		printf("%s GET /?post=name&OK=YES HTTP/1.0\n", argv[0]);
		return(1);
	}

	sp = strdup(argv[1]);
	slen = strlen(sp);

	parsedata_scan_bytes(sp, slen);
	parsedatalex();

	printf("ARGV=[%s] SUCCFLAG=[%d]\n", argv[1], *pRet);

	return 0;
}
#endif

