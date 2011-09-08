/* HTTP Parsing Test */

%{
	#include "sip.h"

	int				*pContentLen;
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

%s			CONTENTLEN

%%


Content-Length[ \t]*:[ \t]*				{
											BEGIN CONTENTLEN;
										}

<CONTENTLEN>[0-9]+						{
											*pContentLen = atoi(yytext);
											BEGIN 0;
											return 0;
										}

(\r\n)									{	}

[ \t\n]									{	}

.										{	}


%%

int sip_contentlenwrap(void)
{
	return 1;
}

int sip_contentlen(char *sp, int slen, int *contentlen)
{
	pContentLen = contentlen;

	sip_contentlen_scan_bytes(sp, slen);

	sip_contentlenlex();

	sip_contentlen_delete_buffer(yy_current_buffer);
	yy_init = 1;

	return 0;
}

#ifdef TEST
int main(void)
{
	int iRet = -1;
	char szBuf[1024*10];
	int	contentlen;

	char *sp;
	int	slen;

	contentlen = 0;

	pContentLen = &contentlen;

//	sprintf(szBuf, "INVITE tel:15720003003 SIP/2.0\r\nCall-ID: YKgLAKTzcGoAABPEO_ZlQQ@211.246.101.65\r\nContent-Length:472\r\nCSeq: 1 INVITE\r\nFrom: MyAgent_2 <SIP:0161001002@ktf.com>;tag=LAKTzcGoAABPEO_ZlQQ\r\n");
//	sprintf(szBuf, "INVITE tel:15720003003 SIP/2.0\r\nCall-ID: YKgLAKTzcGoAABPEO_ZlQQ@211.246.101.65\r\nContent-Length:472\r\nCSeq: 1 INVITE\r\nFrom: MyAgent_2 <SIP:0161003124@ktf.com>;tag=YKgLAKTzcGoAABPEO_ZlQQ123\r\nTO: sips:0161001002@ktf.com;tag=KKYKgLAKTzcGoAABPEO_ZlQQKK\r\n");
	sprintf(szBuf, "SIP/2.0 100 Trying\r\nCall-ID: YKgLAKTzcGoAABPEO_ZlQQ@211.246.101.65\r\nCSeq: 1 INVITE\r\nFrom: MyAgent_2 <SIP:0161001002@ktf.com>;tag=YKgLAKTzcGoAABPEO_ZlQQ123\r\nTO: sip:0161001002@ktf.com;tag=KKYKgLAKTzcGoAABPEO_ZlQQKK\r\nContent-Length: 200\r\n");
//	sprintf(szBuf, "SIP/2.0 100 Trying\r\nCall-ID: YKgLAKTzcGoAABPEO_ZlQQ@211.246.101.65\r\nContent-Length:0\r\nCSeq: 1 INVITE\r\nFrom: MyAgent_2 <SIP:0161001002@ktf.com>;tag=YKgLAKTzcGoAABPEO_ZlQQ123\r\nTO: sip:0161001002@ktf.com;tag=KKYKgLAKTzcGoAABPEO_ZlQQKK\r\n");
//	sprintf(szBuf, "INVITE tel:15720003003 SIP/2.0\r\nCall-ID: YKgLAKTzcGoAABPEO_ZlQQ@211.246.101.65\r\nContent-Length:472\r\nCSeq: 1 INVITE\r\nFrom: MyAgent_2 <SIP:0161003124@ktf.com>;tag=YKgLAKTzcGoAABPEO_ZlQQ123\r\nTO: sips:0161001002@ktf.com;tag=KKYKgLAKTzcGoAABPEO_ZlQQKK\r\nUser-Agent: Poc-client/OMA1.0\r\n\r\no=- 1170740904 1170740904 IN IP4 211.246.101.65\r\nm=audio 40000 RTP/AVP 106 8 4 18 0 101\r\nm=video 40002 RTP/AVP 107 34 31\r\n\r\n");

	sp = szBuf;
	slen = strlen(szBuf);

	sip_contentlen_scan_bytes(sp, slen);
	
	iRet = sip_contentlenlex();

	printf("RETURN [%d]\n", iRet);
	printf("Content-Length: [%d]\n", contentlen);

	return 0;
}
#endif
