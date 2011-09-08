/* HTTP Parsing Test */

%{
	#include "sip.h"

	char			*pIP;
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


%%


[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+			{
											len = yyleng;
											memcpy(pIP, yytext, len);
											pIP[len] = 0x00;
										}

(\r\n)									{	}

[ \t\n]									{	}

.										{	}


%%

int sip_clientipwrap(void)
{
	return 1;
}

int sip_clientip(char *sp, int slen, char *ip)
{
	pIP = ip;

	sip_clientip_scan_bytes(sp, slen);

	sip_clientiplex();

	sip_clientip_delete_buffer(yy_current_buffer);
	yy_init = 1;

	return 0;
}

#ifdef TEST
int main(void)
{
	int iRet = -1;
	char szBuf[1024*10];
	char szIP[1024];

    struct in_addr  stAddr;
	unsigned int	ip = 0;

	char *sp;
	int	slen;

	szIP[0] = 0x00;

	pIP = szIP;

//	sprintf(szBuf, "0fc23aac@172.23.37.230823930623103");
//	sprintf(szBuf, "0fc23aac@172.23.37.230");
	sprintf(szBuf, "758608a8@172.23.37.188");
//	sprintf(szBuf, "758608a8@172.23.37188");
	sp = szBuf;
	slen = strlen(szBuf);

	sip_clientip_scan_bytes(sp, slen);
	
	iRet = sip_clientiplex();

    if(inet_pton(AF_INET, szIP, (void *)&stAddr) > 0) {
		ip = htonl(stAddr.s_addr);
	}

	printf("RETURN [%d]\n", iRet);
	printf("IP: [%s][%u]\n", szIP, ip);

	return 0;
}
#endif
