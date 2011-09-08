/* Full Viewer Parsing Test */

%{
	#include <stdio.h>
	#include <string.h>
	#include "fv.h"
 
	char	*ucMin;
	int		len;
%}

STR			[^\r\n \t]+
NUM			[0-9]{10,}

%s			USERAGENT
%s			MIN
%%

User-Agent[ \t]*:[ \t]* 				{ 
											BEGIN USERAGENT;
										}

<USERAGENT>[ \t]*;[ \t]* 				{ 
											BEGIN MIN;
										}

<MIN>{NUM} 								{ 
											len = (yyleng > MAX_MIN_LEN) ? MAX_MIN_LEN : yyleng;
											memcpy(ucMin, yytext, len);
											ucMin[len] = 0x00;
											BEGIN 0;
											return 0;
										}

(\r\n)									{	}

[ \t\n]									{	}

.										{	}


%%

int fb_minwrap(void)
{
	return 1;
}

int fb_min(char *sp, int slen, char *min)
{
	ucMin = min;

	fb_min_scan_bytes(sp, slen);

	fb_minlex();

	fb_min_delete_buffer(yy_current_buffer);
	yy_init = 1;

	return 0;
}

#ifdef TEST
int main(void)
{
	int iRet = -1;
	char szBuf[1024*10];
	char min[BUFSIZ];

	char *sp;
	int	slen;

	ucMin = min;
	min[0] = 0x00;

	sprintf(szBuf, "user-agent: mozilla/4.0 (compatible; msie 6.0; windows nt 5.0);4500001080803836");

	sp = szBuf;
	slen = strlen(szBuf);

	fb_min_scan_bytes(sp, slen);
	
	iRet = fb_minlex();

	printf("RETURN [%d]\n", iRet);
	printf("MIN: [%s]\n", min);
	return 0;
}
#endif
