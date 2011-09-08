/* HTTP Parsing Test */

%{
	#include <stdio.h>
	#include <string.h>

	#include "common_stg.h"
	#include "xcap.h"

	char			*pMIN;
	int				len;
%}

URL			[0-9a-zA-Z\xA1-\xFE/`:;._&%?=-]+
STR1		[0-9a-zA-Z\xA1-\xFE:;/._&%?=-]+		
STR2		[ 0-9a-zA-Z\xA1-\xFE:;/._&%?=-]+		
STR3		[^\r\n \t;]+
STR4		[^\r\n \t]+
STR5		[^>\r\n \t;]+
STR6		[^"\r\n \t]+
STR7		[^\r\n]+
STR8		[^\r\n]*

%s			MIN

%%


sip:[0-9]{10,11}						{
                                       		len = (yyleng - 4 > MAX_MIN_LEN) ? MAX_MIN_LEN : yyleng - 4;
                                       		memcpy(pMIN, &yytext[4], len);
                                       		pMIN[len] = 0x00;
											return 0;
										}

(\r\n)									{	}

[ \t\n]									{	}

.										{	}


%%

int xcap_minwrap(void)
{
	return 1;
}

int xcap_min(char *sp, int slen, char *min)
{
	pMIN = min;

	xcap_min_scan_bytes(sp, slen);

	xcap_minlex();

	xcap_min_delete_buffer(yy_current_buffer);
	yy_init = 1;

	return 0;
}

#ifdef TEST
int main(void)
{
	int iRet = -1;
	char szBuf[BUFSIZ];
	char min[1024];

	char *sp;
	int	slen;

	min[0] = 0;

	pMIN = min;

//	sprintf(szBuf, "<sip:01057402162@lgt.co.kr");
//	sprintf(szBuf, "<sip:0107402162@lgt.co.kr");
//	sprintf(szBuf, "<sip:010740216@lgt.co.kr");
//	sprintf(szBuf, "<sip:010.57402162@lgt.co.kr");
	sprintf(szBuf, "http://172.17.7.29:9520/resource-lists/users/sip:01057408224@lgt.co.kr/buddy_lists.xml");

	sp = szBuf;
	slen = strlen(szBuf);

	xcap_min_scan_bytes(sp, slen);
	
	iRet = xcap_minlex();

	printf("RETURN [%d]\n", iRet);
	printf("MIN: [%s]\n", min);

	return 0;
}
#endif
