/* HTTP Parsing Test */

%{
	#include "common_stg.h"
	#include "sip.h"

	int				*pCTYPE;
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


chat									{
											*pCTYPE = SIP_CTYPE_CHAT;
											return 0;
										}

file									{
											*pCTYPE = SIP_CTYPE_FILE;
											return 0;
										}

(\r\n)									{	}

[ \t\n]									{	}

.										{	}


%%

int sip_ctypewrap(void)
{
	return 1;
}

int sip_ctype(char *sp, int slen, int *ctype)
{
	pCTYPE = ctype;

	sip_ctype_scan_bytes(sp, slen);

	sip_ctypelex();

	sip_ctype_delete_buffer(yy_current_buffer);
	yy_init = 1;

	return 0;
}

#ifdef TEST
int main(void)
{
	int iRet = -1;
	char szBuf[BUFSIZ];
	int dCType = 0;

	char *sp;
	int	slen;

	pCTYPE = &dCType;

//	sprintf(szBuf, "<sip:01057402162@lgt.co.kr");
//	sprintf(szBuf, "<sip:0107402162@lgt.co.kr");
//	sprintf(szBuf, "<sip:010740216@lgt.co.kr");
//	sprintf(szBuf, "<sip:010.57402162@lgt.co.kr");
	sprintf(szBuf, "<sip:default_01057403609@lgt.co.kr>");
//	sprintf(szBuf, "<sip:cf=38-2944-0-0-@172.17.7.25:5070;session=adhoc>;isfocus");
//	sprintf(szBuf, "sip:adhoc@lgt.co.kr");

	sp = szBuf;
	slen = strlen(szBuf);

	sip_ctype_scan_bytes(sp, slen);
	
	iRet = sip_ctypelex();

	printf("RETURN [%d]\n", iRet);
	printf("RANGE: [%d]\n", dCType);

	return 0;
}
#endif
