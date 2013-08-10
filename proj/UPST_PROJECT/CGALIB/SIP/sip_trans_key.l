/* HTTP Parsing Test */

%{

	#include <stdio.h>
	#include <string.h>
	#include <unistd.h>

	#include "sip.h"

	char	*pCallID;
	char	*pFromTag;
	int		*pSeqNo;
	int		*pMsgType;
	int		*pSeqType;
	int		len;
%}

URL			[0-9a-zA-Z\xA1-\xFE/`:;._&%?=-]+
STR1		[0-9a-zA-Z\xA1-\xFE:;/._&%?=-]+		
STR2		[ 0-9a-zA-Z\xA1-\xFE:;/._&%?=-]+		
STR3		[^\r\n \t;]+
STR4		[^\r\n \t]+

%s			CALLID
%s			FROM
%s			TAG
%s			SEQNO
%s			SEQTYPE
%s			RESP

%%

^[ \t]*ACK[ \t]+{STR4}[ \t]+SIP\/[0-9]+\.[0-9]+								{
											*pMsgType = SIP_MSG_ACK;	
										}

^[ \t]*BYE[ \t]+{STR4}[ \t]+SIP\/[0-9]+\.[0-9]+								{
											*pMsgType = SIP_MSG_BYE;	
										}

^[ \t]*CANCEL[ \t]+{STR4}[ \t]+SIP\/[0-9]+\.[0-9]+							{
											*pMsgType = SIP_MSG_CANCEL;	
										}

^[ \t]*INFO[ \t]+{STR4}[ \t]+SIP\/[0-9]+\.[0-9]+								{
											*pMsgType = SIP_MSG_INFO;	
										}

^[ \t]*INVITE[ \t]+{STR4}[ \t]+SIP\/[0-9]+\.[0-9]+							{
											*pMsgType = SIP_MSG_INVITE;	
										}

^[ \t]*MESSAGE[ \t]+{STR4}[ \t]+SIP\/[0-9]+\.[0-9]+							{
											*pMsgType = SIP_MSG_MESSAGE;	
										}

^[ \t]*NOTIFY[ \t]+{STR4}[ \t]+SIP\/[0-9]+\.[0-9]+							{
											*pMsgType = SIP_MSG_NOTIFY;	
										}

^[ \t]*OPTIONS[ \t]+{STR4}[ \t]+SIP\/[0-9]+\.[0-9]+							{
											*pMsgType = SIP_MSG_OPTIONS;	
										}

^[ \t]*PRACK[ \t]+{STR4}[ \t]+SIP\/[0-9]+\.[0-9]+							{
											*pMsgType = SIP_MSG_PRACK;	
										}

^[ \t]*PUBLISH[ \t]+{STR4}[ \t]+SIP\/[0-9]+\.[0-9]+							{
											*pMsgType = SIP_MSG_PUBLISH;	
										}

^[ \t]*REFER[ \t]+{STR4}[ \t]+SIP\/[0-9]+\.[0-9]+							{
											*pMsgType = SIP_MSG_REFER;	
										}

^[ \t]*REGISTER[ \t]+{STR4}[ \t]+SIP\/[0-9]+\.[0-9]+							{
											*pMsgType = SIP_MSG_REGISTER;	
										}

^[ \t]*SUBSCRIBE[ \t]+{STR4}[ \t]+SIP\/[0-9]+\.[0-9]+						{
											*pMsgType = SIP_MSG_SUBSCRIBE;	
										}

^[ \t]*UPDATE[ \t]+{STR4}[ \t]+SIP\/[0-9]+\.[0-9]+							{
											*pMsgType = SIP_MSG_UPDATE;	
										}

Call-ID[ \t]*:[ \t]* 					{ 
											BEGIN CALLID;
										}

<CALLID>{STR4}							{
											len = (yyleng > SIP_CALLID_LEN) ? SIP_CALLID_LEN : yyleng;
											memcpy(pCallID, yytext, len);
											pCallID[len] = 0x00;
											BEGIN 0;
										}

From[ \t]*:[ \t]*						{ 
											BEGIN FROM;
										}

<FROM>tag[ \t]*=[ \t]*					{
											BEGIN 0;
											BEGIN TAG;
										}

<TAG>{STR3}								{
											len = (yyleng > SIP_TAG_LEN) ? SIP_TAG_LEN : yyleng;
											memcpy(pFromTag, yytext, len);
											pFromTag[len] = 0x00;
											BEGIN 0;
										}


CSeq[ \t]*:[ \t]*						{ 
											BEGIN SEQNO;
										}

<SEQNO>[0-9]+							{
											*pSeqNo = atoi(yytext);
											BEGIN 0;
											BEGIN SEQTYPE;
										}

<SEQTYPE>ACK							{
											*pSeqType = SIP_MSG_INVITE;
											BEGIN 0;
										}

<SEQTYPE>BYE							{
											*pSeqType = SIP_MSG_BYE;
											BEGIN 0;
										}

<SEQTYPE>CANCEL							{
											*pSeqType = SIP_MSG_CANCEL;
											BEGIN 0;
										}

<SEQTYPE>INFO							{
											*pSeqType = SIP_MSG_INFO;
											BEGIN 0;
										}

<SEQTYPE>INVITE							{
											*pSeqType = SIP_MSG_INVITE;
											BEGIN 0;
										}

<SEQTYPE>MESSAGE						{
											*pSeqType = SIP_MSG_MESSAGE;
											BEGIN 0;
										}

<SEQTYPE>NOTIFY							{
											*pSeqType = SIP_MSG_NOTIFY;
											BEGIN 0;
										}

<SEQTYPE>OPTIONS						{
											*pSeqType = SIP_MSG_OPTIONS;
											BEGIN 0;
										}

<SEQTYPE>PRACK							{
											*pSeqType = SIP_MSG_PRACK;
											BEGIN 0;
										}

<SEQTYPE>PUBLISH						{
											*pSeqType = SIP_MSG_PUBLISH;
											BEGIN 0;
										}

<SEQTYPE>REFER							{
											*pSeqType = SIP_MSG_REFER;
											BEGIN 0;
										}

<SEQTYPE>REGISTER						{
											*pSeqType = SIP_MSG_REGISTER;
											BEGIN 0;
										}

<SEQTYPE>SUBSCRIBE						{
											*pSeqType = SIP_MSG_SUBSCRIBE;
											BEGIN 0;
										}

<SEQTYPE>UPDATE							{
											*pSeqType = SIP_MSG_UPDATE;
											BEGIN 0;
										}

SIP\/[0-9]+\.[0-9]+[ \t]+				{
											BEGIN RESP;
										}

<RESP>[0-9]+							{
											*pMsgType = atoi(yytext) * 10;
											BEGIN 0;
										}

(\r\n)									{	}

[ \t\n]									{	}

.										{	}


%%

int sip_trans_keywrap(void)
{
	return 1;
}

int sip_trans_key(char *sp, int slen, char *callID, char *fromTag, int *seqNo, int *msgType, int *seqType)
{
	pCallID = callID;
	pFromTag = fromTag;
	pSeqNo = seqNo;
	pMsgType = msgType;
	pSeqType = seqType;

	sip_trans_key_scan_bytes(sp, slen);

	sip_trans_keylex();

	sip_trans_key_delete_buffer(yy_current_buffer);
	yy_init = 1;

	return 0;
}

#ifdef TEST
int main(void)
{
	int iRet = -1;
	char szBuf[1024*10];
	char szCallID[1024];
	char szFromTag[1024];
	int	dSeqNo = 0;
	int	dMsgType = 0;
	int	dSeqType = 0;

	char *sp;
	int	slen;

	szCallID[0] = 0x00;
	szFromTag[0] = 0x00;

	pCallID = szCallID;
	pFromTag = szFromTag;
	pSeqNo = &dSeqNo;
	pMsgType = &dMsgType;
	pSeqType = &dSeqType;

//	sprintf(szBuf, "INVITE tel:15720003003 SIP/2.0\r\nCall-ID: YKgLAKTzcGoAABPEO_ZlQQ@211.246.101.65\r\nContent-Length:472\r\nCSeq: 1 INVITE\r\nFrom: MyAgent_2 <SIP:0161001002@ktf.com>;tag=LAKTzcGoAABPEO_ZlQQ\r\n");
//	sprintf(szBuf, "INVITE tel:15720003003 SIP/2.0\r\nCall-ID: YKgLAKTzcGoAABPEO_ZlQQ@211.246.101.65\r\nContent-Length:472\r\nCSeq: 1 INVITE\r\nFrom: MyAgent_2 <SIP:0161001002@ktf.com>;tag=YKgLAKTzcGoAABPEO_ZlQQ123\r\nTo: 0161001002@ktf.com;tag=KKYKgLAKTzcGoAABPEO_ZlQQKK\r\n");

#if 0
	sprintf(szBuf, 
		"SIP/2.0 100 Trying\r\nCall-ID: YKgLAKTzcGoAABPEO_ZlQQ@211.246.101.65\r\nContent-Length:472\r\n"
//			"Cseq: 1542 ACK\r\n"
//			"Cseq: 1542 BYE\r\n"
//			"Cseq: 1542 CANCEL\r\n"
//			"Cseq: 1542 INFO\r\n"
//			"Cseq: 1542 INVITE\r\n"
//			"Cseq: 1542 MESSAGE\r\n"
//			"Cseq: 1542 NOTIFY\r\n"
//			"Cseq: 1542 OPTIONS\r\n"
//			"Cseq: 1542 PRACK\r\n"
//			"Cseq: 1542 PUBLISH\r\n"
//			"Cseq: 1542 REFER\r\n"
//			"Cseq: 1542 REGISTER\r\n"
//			"Cseq: 1542 SUBSCRIBE\r\n"
			"Cseq: 1542 UPDATE\r\n"
		"From: MyAgent_2 <SIP:0161001002@ktf.com>;tag=YKgLAKTzcGoAABPEO_ZlQQ123\r\n"
		"To: 0161001002@ktf.com;tag=KKYKgLAKTzcGoAABPEO_ZlQQKK\r\n");
#endif

#if 1
	sprintf(szBuf, 
			"INVITE tel:+821029172403 SIP/2.0\r\n"
			"To: tel:+821029172403\r\n"
			"Via: SIP/2.0/TCP 10.36.2.1:5070;branch=z9hG4bK747c831d_2.2.2.23.19954.1.,SIP/2.0/TCP scim1.ktfims.com:5060;received=221.148.244.14;branch=z9hG4bK--1l4ihyosbnnx7-1w3z6frfexf\r\n"
//			"Cseq: 1542 ACK\r\n"
//			"Cseq: 1542 BYE\r\n"
//			"Cseq: 1542 CANCEL\r\n"
//			"Cseq: 1542 INFO\r\n"
//			"Cseq: 1542 INVITE\r\n"
//			"Cseq: 1542 MESSAGE\r\n"
//			"Cseq: 1542 NOTIFY\r\n"
//			"Cseq: 1542 OPTIONS\r\n"
//			"Cseq: 1542 PRACK\r\n"
//			"Cseq: 1542 PUBLISH\r\n"
//			"Cseq: 1542 REFER\r\n"
//			"Cseq: 1542 REGISTER\r\n"
//			"Cseq: 1542 SUBSCRIBE\r\n"
			"Cseq: 1542 UPDATE\r\n"
			"From: tel:+821029172404;tag=23-4639-1-cf\r\n"
			"Route: <sip:IM@scim1.ktfims.com:5060;lr;term>,<sip:2.2.1.23.19954.1.141@10.36.2.1:5070;lr;OdiPsi=IM>;ruri=\"tel:+821029172403\"\r\n"
			"Call-ID: 347890160-ri-23-4639-1IMN2\r\n"
			"Contact: <sip:cf=23-4639-0-0-@221.148.244.147:5070;session=adhoc>;isfocus\r\n"
			"Subject: UID=3c825d55\r\n"
			"Supported: timer\r\n"
			"Referred-By: tel:+821029172404\r\n"
			"Content-Type: application/sdp\r\n"
			"Max-Forwards: 66\r\n"
			"Record-Route: <sip:2.2.2.23.19954.1.@10.36.2.1:5070;lr>,<sip:scim1.ktfims.com:5060;transport=tcp;lr>,<sip:172.31.38.201:5060;opmnuid=591533691;asid=D-1bck77iqumwsm-16ab7u6e\r\n"
			"Content-Length: 178\r\n"
			"Accept-Contact: *;+u.ktf.imchat\r\n"
			"Session-Expires: 360;refresher=uas\r\n"
			"P-Charging-Vector: icid-value=jgag.pcscf1.ktfims.com-27-20330-1192015583-7cd35022;orig-ioi=\"Type 3KTF\";orig-ioi=\"Type 3KTF\"\r\n"
			"P-Asserted-Identity: tel:+821029172404\r\n"
			"P-Charging-Function-Addresses: ccf=\"aaa://cdf.ktfims.com\";ccf=\"aaa://cdf.ktfims.com\"\r\n"
			"User-Agent: IM-serv\r\n"
			"\r\n"
			"v=0\r\n"
			"o=- 0 0 IN IP4 221.148.244.171\r\n"
			"s=-\r\n"
			"c=IN IP4 221.148.244.171\r\n"
			"t=0 0\r\n"
			"m=message 19203 TCP/MSRP *\r\n"
			"a=accept-types:*\r\n"
			"a=path:msrps://221.148.244.171:19203/ims-8-17-1-ims;tcp\r\n\r\n"
			);
#endif

	sp = szBuf;
	slen = strlen(szBuf);

	sip_trans_key_scan_bytes(sp, slen);
	
	iRet = sip_trans_keylex();
	printf("RETURN [%d]\n", iRet);
	printf("CALLID: [%s]\n", szCallID);
	printf("FROM TAG: [%s]\n", szFromTag);
	printf("SEQ NO: [%d]\n", dSeqNo);
	printf("MSG TYPE: [%d]\n", dMsgType);
	printf("SEQ TYPE: [%d]\n", dSeqType);

	return 0;
}
#endif
