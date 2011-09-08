/* HTTP Parsing Test */

%{
	#include <stdio.h>
	#include <string.h>
	
	#include "common_stg.h"
	#include "sip.h"

	int 			*pisYes;
%}

URL			[0-9a-zA-Z\xA1-\xFE/`:;._&%?=-]+
STR1		[0-9a-zA-Z\xA1-\xFE:;/._&%?=-]+		
STR2		[ 0-9a-zA-Z\xA1-\xFE:;/._&%?=-]+		
STR3		[^\r\n \t;]+
STR4		[^\r\n \t]+
STR5		[^>\r\n \t;]+
STR6		[^"\r\n \t]+
STR7		[^\r\n]+

%s			METHOD
%s			URI
%s			RST
%s			CODE

%%

^[ \t]*ACK[ \t]+						{
											BEGIN METHOD;		
										}

^[ \t]*BYE[ \t]+						{
											BEGIN METHOD;		
										}

^[ \t]*CANCEL[ \t]+						{
											BEGIN METHOD;		
										}

^[ \t]*INFO[ \t]+						{
											BEGIN METHOD;		
										}

^[ \t]*INVITE[ \t]+						{
											BEGIN METHOD;		
										}

^[ \t]*MESSAGE[ \t]+					{
											BEGIN METHOD;		
										}

^[ \t]*NOTIFY[ \t]+						{
											BEGIN METHOD;		
										}

^[ \t]*OPTIONS[ \t]+					{
											BEGIN METHOD;		
										}

^[ \t]*PRACK[ \t]+						{
											BEGIN METHOD;		
										}

^[ \t]*PUBLISH[ \t]+					{
											BEGIN METHOD;		
										}

^[ \t]*REFER[ \t]+						{
											BEGIN METHOD;		
										}

^[ \t]*REGISTER[ \t]+					{
											BEGIN METHOD;		
										}

^[ \t]*SUBSCRIBE[ \t]+					{
											BEGIN METHOD;		
										}

^[ \t]*UPDATE[ \t]+						{
											BEGIN METHOD;		
										}

<METHOD>{STR4}[ \t]+					{
											BEGIN 0;
											BEGIN URI;
										}

<URI>SIP\/[0-9]+\.[0-9]+				{
											*pisYes = YES;
											BEGIN 0;
											return 0;
										}

^[ \t]*SIP\/[0-9]+\.[0-9]+[ \t]+		{
											BEGIN RST;		
										}

<RST>[0-9]+[ \t]+						{
											BEGIN 0;
											BEGIN CODE;
										}

<CODE>{STR4}							{
											*pisYes = YES;
											BEGIN 0;
											return 0;
										}

(\r\n)									{	
											*pisYes = NO;
											return 0;
										}

[ \t\n]									{	}

.										{	}


%%

int sipheaderwrap(void)
{
	return 1;
}

int sipheader(char *sp, int slen, int *isYes)
{
	pisYes = isYes;

	sipheader_scan_bytes(sp, slen);

	sipheaderlex();

	sipheader_delete_buffer(yy_current_buffer);
	yy_init = 1;

	return 0;
}

#ifdef TEST
int main(void)
{
	int		iRet = -1;
	char	szBuf[BUFSIZ];
	int		isYes;

	char *sp;
	int	slen;

	isYes = NO;

	pisYes = &isYes;

//	sprintf(szBuf, "MSRP 784e6b58 SEND\r\nTo-Path: msrps://192.168.151.65:19200/ims-789-29-0-ims;tcp msrps://192.168.151.65:19200/ims-789-29-1-ims;tcp\r\nFrom-Path: msrp://211.246.101.60:7000/4e4f6b55;tcp\r\nMessage-ID: 784f6b77\r\n-------784e6b58$\r\n");
	sprintf(szBuf, "dsfsdk\r\n\r\nINVITE tel:15720003003 SIP/2.0\r\nCall-ID: YKgLAKTzcGoAABPEO_ZlQQ@211.246.101.65\r\nContent-Length:472\r\nCSeq: 1 INVITE\r\nFrom: MyAgent_2 <SIP:0161001002@ktf.com>;tag=LAKTzcGoAABPEO_ZlQQ\r\n");
//	sprintf(szBuf, "INVITE tel:15720003003 SIP/2.0\r\nCall-ID: YKgLAKTzcGoAABPEO_ZlQQ@211.246.101.65\r\nContent-Length:472\r\nCSeq: 1 INVITE\r\nFrom: MyAgent_2 <SIP:0161003124@ktf.com>;tag=YKgLAKTzcGoAABPEO_ZlQQ123\r\nTO: sips:0161001002@ktf.com;tag=KKYKgLAKTzcGoAABPEO_ZlQQKK\r\n");
//	sprintf(szBuf, "SIP/2.0 100 Trying\r\nCall-ID: YKgLAKTzcGoAABPEO_ZlQQ@211.246.101.65\r\nContent-Length:472\r\nCSeq: 1 INVITE\r\nFrom: MyAgent_2 <SIP:0161001002@ktf.com>;tag=YKgLAKTzcGoAABPEO_ZlQQ123\r\nTO: sip:0161001002@ktf.com;tag=KKYKgLAKTzcGoAABPEO_ZlQQKK\r\n");
//	sprintf(szBuf, "INVITE tel:15720003003 SIP/2.0\r\nCall-ID: YKgLAKTzcGoAABPEO_ZlQQ@211.246.101.65\r\nContent-Length:472\r\nCSeq: 1 INVITE\r\nFrom: MyAgent_2 <SIP:0161003124@ktf.com>;tag=YKgLAKTzcGoAABPEO_ZlQQ123\r\nTO: sips:0161001002@ktf.com;tag=KKYKgLAKTzcGoAABPEO_ZlQQKK\r\nUser-Agent: Poc-client/OMA1.0\r\n\r\no=- 1170740904 1170740904 IN IP4 211.246.101.65\r\nm=audio 40000 RTP/AVP 106 8 4 18 0 101\r\nm=video 40002 RTP/AVP 107 34 31\r\n\r\n");

	sp = szBuf;
	slen = strlen(szBuf);

	sipheader_scan_bytes(sp, slen);
	
	iRet = sipheaderlex();
	printf("RETURN [%d]\n", iRet);
	printf("isYes: [%d]\n", isYes);

	return 0;
}
#endif
