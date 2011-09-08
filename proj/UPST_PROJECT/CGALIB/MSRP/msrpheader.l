/* HTTP Parsing Test */

%{
	#include <stdio.h>
	#include <string.h>

	#include "common_stg.h"
	#include "msrp.h"

	int				len;
	unsigned short *pusMethod;
	char *pszTID;
%}

URL			[0-9a-zA-Z\xA1-\xFE/`:;._&%?=-]+
STR1		[0-9a-zA-Z\xA1-\xFE:;/._&%?=-]+		
STR2		[ 0-9a-zA-Z\xA1-\xFE:;/._&%?=-]+		
STR3		[^\r\n \t;]+
STR4		[^\r\n \t]+
STR5		[^>\r\n \t;]+
STR6		[^"\r\n \t]+
STR7		[^\r\n]+

%s			START
%s			METHOD

%%

^[ \t]*MSRP[ \t]+						{
											BEGIN START;		
										}

<START>{STR4}							{
											len = (yyleng > MSRP_TID_LEN) ? MSRP_TID_LEN : yyleng;
											memcpy(pszTID, yytext, len);
											pszTID[len] = 0x00;
											BEGIN 0;
											BEGIN METHOD;	
										}

<METHOD>[ \t]+SEND						{
											*pusMethod = MSRP_METHOD_SEND;
											BEGIN 0;
											return 0;
										}

<METHOD>[ \t]+REPORT					{
											*pusMethod = MSRP_METHOD_REPORT;
											BEGIN 0;
											return 0;
										}

<METHOD>[ \t]+AUTH						{
											*pusMethod = MSRP_METHOD_AUTH;
											BEGIN 0;
											return 0;
										}

<METHOD>[ \t]+[0-9]+					{
											*pusMethod = MSRP_METHOD_RESPONSE;
											BEGIN 0;
											return 0;
										}

(\r\n)									{	
											return 0;
										}

[ \t\n]									{	}

.										{	}


%%

int msrpheaderwrap(void)
{
	return 1;
}

int msrpheader(char *sp, int slen, unsigned short *usMethod, char *szTID)
{
	pusMethod = usMethod;
	pszTID = szTID;

	msrpheader_scan_bytes(sp, slen);

	msrpheaderlex();

	msrpheader_delete_buffer(yy_current_buffer);
	yy_init = 1;

	return 0;
}

#ifdef TEST
int main(void)
{
	int		iRet = -1;
	char	szBuf[BUFSIZ];
	unsigned short	usMethod;
	char	szTID[BUFSIZ];

	char *sp;
	int	slen;

	usMethod = 0;
	memset(szTID, 0x00, 9);

	pusMethod = &usMethod;
	pszTID = szTID;

	sprintf(szBuf, "MSRP 784e6b58 SEND\r\nTo-Path: msrps://192.168.151.65:19200/ims-789-29-0-ims;tcp msrps://192.168.151.65:19200/ims-789-29-1-ims;tcp\r\nFrom-Path: msrp://211.246.101.60:7000/4e4f6b55;tcp\r\nMessage-ID: 784f6b77\r\n-------784e6b58$\r\n");
	sp = szBuf;
	slen = strlen(szBuf);

	msrpheader_scan_bytes(sp, slen);
	
	iRet = msrpheaderlex();
	printf("RETURN [%d]\n", iRet);
	printf("Method: [%d]\n", usMethod);
	printf("TID: [%s]\n", szTID);

	return 0;
}
#endif
