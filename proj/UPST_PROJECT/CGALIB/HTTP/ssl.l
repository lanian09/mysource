/* HTTP Parsing Test */

%{
	#include <stdio.h>
	#include <string.h>

	#include "http.h"

	char *ucMin;
	int *pdBaseID;
	int *pdNID;
	int *pdSID;
	char *ucCPName;
	char *ucSvcCode;
	char *ucCompatible;
	int len;
	int *pdCompLen;
	int tmpLen;
	int tmpPos;
%}

URL			[0-9a-zA-Z\xA1-\xFE/`:;._&%?=-]+
STR1		[0-9a-zA-Z\xA1-\xFE:;/._&%?=-]+		
STR2		[ 0-9a-zA-Z\xA1-\xFE:;/._&%?=-]+		
STR3		[^\r\n \t;]+
STR4		[^\r\n \t]+
NUM			[0-9]+

%s			PHONE
%s			BASEID
%s			NID
%s			SID
%s			CPNAME
%s			SVCCODE
%s			COMPATIBLE

%%

HTTP_PHONE_NUMBER[ \t]*:[ \t]* 			{ 
											BEGIN PHONE;
										}

<PHONE>{NUM}							{
											if((yytext[0] == '8') && (yytext[1] == '2')) {
												tmpPos = 2;
												tmpLen = yyleng - 2;
											} else {
												tmpPos = 0;
												tmpLen = yyleng;
											}
											len = (tmpLen > MAX_MIN_LEN) ? MAX_MIN_LEN : tmpLen;
											memcpy(ucMin, &yytext[tmpPos], len);
											ucMin[len] = 0x00;
											BEGIN 0;
										}

BASE_ID[ \t]*:[ \t]*					{
											BEGIN BASEID;
										}

<BASEID>{NUM}							{
											*pdBaseID = atoi(yytext);
											BEGIN 0;
										}

NID[ \t]*:[ \t]*						{
											BEGIN NID;
										}

<NID>{NUM}								{
											*pdNID = atoi(yytext);
											BEGIN 0;
										}

SID[ \t]*:[ \t]*						{
											BEGIN SID;
										}

<SID>{NUM}								{
											*pdSID = atoi(yytext);
											BEGIN 0;
										}

cpname[ \t]*=[ \t]* 					{ 
											BEGIN CPNAME;
										}

<CPNAME>{STR3}							{
											len = (yyleng > MAX_CPNAME_LEN) ? MAX_CPNAME_LEN : yyleng;
											memcpy(ucCPName, yytext, len);
											ucCPName[len] = 0x00;
											BEGIN 0;
										}

svccode[ \t]*=[ \t]* 					{ 
											BEGIN SVCCODE;
										}

<SVCCODE>{STR3}							{
											len = (yyleng > MAX_SERVICECODE_LEN) ? MAX_SERVICECODE_LEN : yyleng;
											memcpy(ucSvcCode, yytext, len);
											ucSvcCode[len] = 0x00;
											BEGIN 0;
										}

compatible[ \t]*;[ \t]* 				{ 
											BEGIN COMPATIBLE;
										}

<COMPATIBLE>{STR4}						{
											len = (yyleng > MAX_COMPATIBLE_LEN) ? MAX_COMPATIBLE_LEN : yyleng;
											memcpy(ucCompatible, yytext, len);
											ucCompatible[len] = 0x00;
											*pdCompLen = len;
											BEGIN 0;
										}

(\r\n)									{	}

[ \t\n]									{	}

.										{	}


%%

int sslwrap(void)
{
	return 1;
}

int ssl(char *sp, int slen, char *szMin, char *szCPName, char *szSvcCode, char *szCompatible, int *pCompLen, int *pBaseID, int *pNID, int *pSID)
{
	ucMin = szMin;
	pdBaseID = pBaseID;
	pdNID = pNID;
	pdSID = pSID;	
	ucCPName = szCPName;
	ucSvcCode = szSvcCode;
	ucCompatible = szCompatible;
	pdCompLen = pCompLen;

	ssl_scan_bytes(sp, slen);

	ssllex();

	ssl_delete_buffer(yy_current_buffer);
	yy_init = 1;

	return 0;
}

#ifdef TEST
int main(void)
{
	int iRet = -1;
	char szBuf[1024*10];
	char szMin[1024];
	char szCPName[1024];
	char szSvcCode[1024];
	char szCompatible[1024];
	int dBaseID;
	int dNID;
	int dSID;
	int dCompLen;

	char *sp;
	int	slen;

	ucMin = szMin;
	pdBaseID = &dBaseID;
	pdNID = &dNID;
	pdSID = &dSID;	
	ucCPName = szCPName;
	ucSvcCode = szSvcCode;
	ucCompatible = szCompatible;
	pdCompLen = &dCompLen;

	sprintf(szBuf, "RESULT\r\nCPData: cpname=woori;svccode=wooribanking000\r\nUser-Agent: MobileExplorer/1.2 (Mozilla/1.22; compatible; KUNF12;\r\nHTTP_PHONE_NUMBER: 820175258525\r\nHTTP_PHONE_SYSTEM_PARAMETER: BASE_ID:326, NID: 36, SID: 2189, BASE_LAT: 0, BASE_LONG: 0\r\n");

	sp = szBuf;
	slen = strlen(szBuf);

	ssl_scan_bytes(sp, slen);
	
	iRet = ssllex();

	printf("RETURN [%d]\n", iRet);
	printf("MIN: [%s]\n", szMin);
	printf("CPNAME: [%s]\n", szCPName);
	printf("SVCCODE: [%s]\n", szSvcCode);
	printf("COMPATIBLE: [%s]LEN[%d]\n", szCompatible, dCompLen);
	printf("BASEID: [%d]\n", dBaseID);
	printf("NID: [%d]\n", dNID);
	printf("SID: [%d]\n", dSID);
	return 0;
}
#endif
