/* HTTP Parsing Test */

%{
	#include <stdio.h>
	#include <string.h>

	#include "common_stg.h"
	#include "msrp.h"

	int				len;
	unsigned short *pusMethod, *pusSuccessReport, *pusFailureReport, *pusResCode, *pusEndFlag;
	char *pszContentType, *pszToPath, *pszFromPath, *pszMSGID, *pszTID, *pSURI;
%}

URL			[0-9a-zA-Z\xA1-\xFE/`:;._&%?=-]+
STR1		[0-9a-zA-Z\xA1-\xFE:;/._&%?=-]+		
STR2		[ 0-9a-zA-Z\xA1-\xFE:;/._&%?=-]+		
STR3		[^\r\n \t;]+
STR4		[^\r\n \t]+
STR5		[^>\r\n \t;]+
STR6		[^"\r\n \t]+
STR7		[^\r\n]+
STR8		[^\#\$\+\r\n \t]+

%s			METHOD
%s			SUCC
%s			FAIL
%s			CONTENTTYPE
%s			TOPATH
%s			FROMPATH
%s			MSGID
%s			STATUS
%s			END
%s			TID
%s			SURI

%%

^[ \t]*MSRP[ \t]+						{
											BEGIN TID;		
										}

<TID>{STR4}								{
											len = (yyleng > MSRP_TID_LEN) ? MSRP_TID_LEN : yyleng;
											memcpy(pszTID, yytext, len);
											pszTID[len] = 0x00;
											BEGIN 0;
											BEGIN METHOD;	
										}

<METHOD>[ \t]+SEND						{
											*pusMethod = MSRP_METHOD_SEND;
											BEGIN 0;
										}

<METHOD>[ \t]+REPORT					{
											*pusMethod = MSRP_METHOD_REPORT;
											BEGIN 0;
										}

<METHOD>[ \t]+AUTH						{
											*pusMethod = MSRP_METHOD_AUTH;
											BEGIN 0;
										}

<METHOD>[ \t]+[0-9]+					{
											*pusMethod = MSRP_METHOD_RESPONSE;
											*pusResCode = atoi(yytext);
											BEGIN 0;
										}

Success\-Report[ \t]*:[ \t]*				{ 
											BEGIN SUCC;
										}

<SUCC>yes								{
											*pusSuccessReport = MSRP_FLAG_YES;
											BEGIN 0;
										}

<SUCC>no								{
											*pusSuccessReport = MSRP_FLAG_NO;
											BEGIN 0;
										}

Failure\-Report[ \t]*:[ \t]*				{ 
											BEGIN FAIL;
										}

<FAIL>yes								{
											*pusFailureReport = MSRP_FLAG_YES;
											BEGIN 0;
										}

<FAIL>no								{
											*pusFailureReport = MSRP_FLAG_NO;
											BEGIN 0;
										}

<FAIL>partial							{
											*pusFailureReport = MSRP_FLAG_PARTIAL;
											BEGIN 0;
										}

Content\-Type[ \t]*:[ \t]*				{ 
											BEGIN CONTENTTYPE;
										}

<CONTENTTYPE>{STR4}						{
											len = (yyleng > MSRP_CONTENTTYPE_LEN) ? MSRP_CONTENTTYPE_LEN : yyleng;
											memcpy(pszContentType, yytext, len);
											pszContentType[len] = 0x00;
											BEGIN 0;
										}

To\-Path[ \t]*:[ \t]*					{ 
											BEGIN TOPATH;
										}

<TOPATH>{STR4}							{
											len = (yyleng > MSRP_PATH_LEN) ? MSRP_PATH_LEN : yyleng;
											memcpy(pszToPath, yytext, len);
											pszToPath[len] = 0x00;
											BEGIN 0;
										}

From\-Path[ \t]*:[ \t]*					{
											BEGIN FROMPATH;
										}

<FROMPATH>{STR4}						{
											len = (yyleng > MSRP_PATH_LEN) ? MSRP_PATH_LEN : yyleng;
											memcpy(pszFromPath, yytext, len);
											pszFromPath[len] = 0x00;
											BEGIN 0;
										}

Message\-ID[ \t]*:[ \t]*					{
											BEGIN MSGID;
										}

<MSGID>{STR4}							{
											len = (yyleng > MSRP_MSGID_LEN) ? MSRP_MSGID_LEN : yyleng;
											memcpy(pszMSGID, yytext, len);
											pszMSGID[len] = 0x00;
											BEGIN 0;
										}

Status[ \t]*:[ \t]*[0-9]+[ \t]+			{
											BEGIN STATUS;
										}

<STATUS>[0-9]+							{
											*pusResCode = atoi(yytext);
											BEGIN 0;
										}

\-\-\-\-\-\-\-{STR8}					{
											BEGIN END;
										}

<END>\#									{
											*pusEndFlag = MSRP_ENDFLAG_ABORT;
											BEGIN 0;
										}

<END>\$									{
											*pusEndFlag = MSRP_ENDFLAG_END;
											BEGIN 0;
										}

<END>\+									{
											*pusEndFlag = MSRP_ENDFLAG_CONTINUE;
											BEGIN 0;

										}

Sender\-URI[ \t]*:[ \t]*				{
											BEGIN SURI;
										}

<SURI>{STR7}							{
											len = (yyleng > MSRP_SURI_LEN) ? MSRP_SURI_LEN : yyleng;
											memcpy(pSURI, yytext, len);
											pSURI[len] = 0x00;
											BEGIN 0;
										}

(\r\n)									{	}

[ \t\n]									{	}

.										{	}


%%

int msrpwrap(void)
{
	return 1;
}

int msrp(char *sp, int slen, unsigned short *usMethod, char *szContentType, unsigned short *usSuccessReport, unsigned short *usFailureReport, char *szToPath, char *szFromPath, char *szMSGID, unsigned short *usResCode, unsigned short *usEndFlag, char *szTID, char *suri)
{
	pusMethod = usMethod;
	pszContentType = szContentType;
	pusSuccessReport = usSuccessReport;
	pusFailureReport = usFailureReport;
	pszToPath = szToPath;
	pszFromPath = szFromPath;
	pszMSGID = szMSGID;
	pusResCode = usResCode;
	pusEndFlag = usEndFlag;
	pszTID = szTID;
	pSURI = suri;

	msrp_scan_bytes(sp, slen);

	msrplex();

	msrp_delete_buffer(yy_current_buffer);
	yy_init = 1;

	return 0;
}

#ifdef TEST
int main(void)
{
	int		iRet = -1;
	char	szBuf[BUFSIZ];

	unsigned short usMethod;
	char szContentType[BUFSIZ];
	unsigned short usSuccessReport;
	unsigned short usFailureReport;
	unsigned short usEndFlag;
	char szToPath[BUFSIZ];
	char szFromPath[BUFSIZ];
	char szMSGID[BUFSIZ];
	char szTID[BUFSIZ];
	unsigned short usResCode;
	char suri[BUFSIZ];

	char *sp;
	int	slen;

	usMethod = 0;
	szContentType[0] = 0x00;
	usSuccessReport = 0;
	usFailureReport = 0;
	szToPath[0] = 0x00;
	szFromPath[0] = 0x00;
	szMSGID[0] = 0x00;
	usResCode = 0;
	usEndFlag = 0;
	szTID[0] = 0x00;
	suri[0] = 0x00;

	pusMethod = &usMethod;
	pszContentType = szContentType;
	pusSuccessReport = &usSuccessReport;
	pusFailureReport = &usFailureReport;
	pszToPath = szToPath;
	pszFromPath = szFromPath;
	pszMSGID = szMSGID;
	pusResCode = &usResCode;
	pusEndFlag = &usEndFlag;
	pszTID = szTID;
	pSURI = suri;

//	sprintf(szBuf, "MSRP 784e6b58 SEND\r\nTo-Path: msrps://192.168.151.65:19200/ims-789-29-0-ims;tcp msrps://192.168.151.65:19200/ims-789-29-1-ims;tcp\r\nFrom-Path: msrp://211.246.101.60:7000/4e4f6b55;tcp\r\nMessage-ID: 784f6b77\r\nByte-Range: 1-317/317\r\nContent-Type: text/plain\r\nStatus: 000 200 OK\r\n-------784e6b58$\r\n");

//	sprintf(szBuf, "MSRP 784e6b58 200 OK\r\nTo-Path: msrps://192.168.151.65:19200/ims-789-29-0-ims;tcp msrps://192.168.151.65:19200/ims-789-29-1-ims;tcp\r\nFrom-Path: msrp://211.246.101.60:7000/4e4f6b55;tcp\r\nMessage-ID: 784f6b77\r\nContent-Type: text/plain\r\n-------784e6b58$\r\n");

	sprintf(szBuf, "MSRP 30eb3ca269042a721a19 REPORT\r\nTo-Path: msrp://10.137.170.227:7019/1c01057403383c7a649f8;tcp\r\nFrom-Path: msrp://211.115.124.253:19344/ims-11687-19-0-ims;tcp msrp://211.115.124.253:19343/ims-11688-19-1-ims;tcp\r\nMessage-ID : 1b0105740338366037c14\r\nByte-Range: 1-45684/45684\r\nStatus: 000 200 OK\r\nContent-Type: application/msrp-relay-report\r\n");

	sp = szBuf;
	slen = strlen(szBuf);

printf("len=%d data=%.*s", slen, slen, sp);

	msrp_scan_bytes(sp, slen);
	
	iRet = msrplex();
	printf("RETURN [%d]\n", iRet);
	printf("Method: [%d]\n", usMethod);
	printf("ContentType: [%s]\n", szContentType);
	printf("SuccessReport: [%d]\n", usSuccessReport);
	printf("FailureReport: [%d]\n", usFailureReport);
	printf("ToPath: [%s]\n", szToPath);
	printf("FromPath: [%s]\n", szFromPath);
	printf("MSGID: [%s]\n", szMSGID);
	printf("ResCode: [%d]\n", usResCode);
	printf("EndFlag: [%d]\n", usEndFlag);
	printf("TID: [%s]\n", szTID);
	printf("SURI: [%s]\n", suri);

	return 0;
}
#endif
