/* HTTP Parsing Test */

%{
	#include <stdio.h>
	#include <string.h>

	#include "common_stg.h"
	#include "msrp.h"

	char			*pMIN;
	int				*pVENDOR;
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
										}

lgt										{
											*pVENDOR = IM_VENDOR_LGT;
										}

ktf										{
											*pVENDOR = IM_VENDOR_KTF;
										}

skt										{
											*pVENDOR = IM_VENDOR_SKT;
										}

(\r\n)									{	}

[ \t\n]									{	}

.										{	}


%%

int msrp_minwrap(void)
{
	return 1;
}

int msrp_min(char *sp, int slen, char *min, int *vendor)
{
	pMIN = min;
	pVENDOR = vendor;

	msrp_min_scan_bytes(sp, slen);

	msrp_minlex();

	msrp_min_delete_buffer(yy_current_buffer);
	yy_init = 1;

	return 0;
}

#ifdef TEST
int main(void)
{
	int iRet = -1;
	char szBuf[BUFSIZ];
	char min[1024];
	int	dVendor = 0;

	char *sp;
	int	slen;

	min[0] = 0;

	pMIN = min;
	pVENDOR = &dVendor;

//	sprintf(szBuf, "<sip:01057402162@lgt.co.kr");
//	sprintf(szBuf, "<sip:01057402162@ktf.co.kr");
	sprintf(szBuf, "<sip:01057402162@skt.co.kr");
//	sprintf(szBuf, "<sip:0107402162@lgt.co.kr");
//	sprintf(szBuf, "<sip:010740216@lgt.co.kr");
//	sprintf(szBuf, "<sip:010.57402162@lgt.co.kr");
//	sprintf(szBuf, "<sip:default_01057403609@lgt.co.kr>");

	sp = szBuf;
	slen = strlen(szBuf);

	msrp_min_scan_bytes(sp, slen);
	
	iRet = msrp_minlex();

	printf("RETURN [%d]\n", iRet);
	printf("MIN: [%s]\n", min);
	printf("VENDOR: [%d]\n", dVendor);

	return 0;
}
#endif
