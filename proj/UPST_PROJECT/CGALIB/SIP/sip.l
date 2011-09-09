/* HTTP Parsing Test */

%{
	#include <stdio.h>
	#include <string.h>

	#include "sip.h"

	char			*pURI;
	char			*pFrom;
	char			*pTo;
	unsigned int	*pSessID;
	unsigned short	*pAudioPort;
	unsigned short	*pVideoPort;
	unsigned short	*pMsrpPort;
	char			*pUserAgent;
	char			*pAuth_nonce;
	char			*pUserName;
	char			*pAudioProto;
	char			*pVideoProto;
	int				*pMsgType;
	int				*pEvent;
	char			*pContact;
	char			*pAccept;
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
STR8		[^\r\n;]+

%s			URI
%s			FROM
%s			FROMID
%s			TO
%s			TOID
%s			USERNAME
%s			SESSID
%s			AUDIO_VIDEO
%s			AUDIO_PORT
%s			AUDIO_PROTO
%s			AUDIO_PROTO_SUB
%s			VIDEO_PORT
%s			VIDEO_PROTO
%s			VIDEO_PROTO_SUB
%s			USERAGENT
%s			AUTH
%s			NONCE
%s			CONTACT
%s			ACCEPT_CONTACT
%s			MESSAGE_PORT

%%

^[ \t]*ACK[ \t]+						{
											*pMsgType = SIP_MSG_ACK;
											BEGIN	URI;		
										}

^[ \t]*BYE[ \t]+						{
											*pMsgType = SIP_MSG_BYE;
											BEGIN	URI;		
										}

^[ \t]*CANCEL[ \t]+						{
											*pMsgType = SIP_MSG_CANCEL;
											BEGIN	URI;		
										}

^[ \t]*INFO[ \t]+						{
											*pMsgType = SIP_MSG_INFO;
											BEGIN	URI;		
										}

^[ \t]*INVITE[ \t]+						{
											*pMsgType = SIP_MSG_INVITE;
											BEGIN	URI;		
										}

^[ \t]*MESSAGE[ \t]+					{
											*pMsgType = SIP_MSG_MESSAGE;
											BEGIN	URI;		
										}

^[ \t]*NOTIFY[ \t]+						{
											*pMsgType = SIP_MSG_NOTIFY;
											BEGIN	URI;		
										}

^[ \t]*OPTIONS[ \t]+					{
											*pMsgType = SIP_MSG_OPTIONS;
											BEGIN	URI;		
										}

^[ \t]*PRACK[ \t]+						{
											*pMsgType = SIP_MSG_PRACK;
											BEGIN	URI;		
										}

^[ \t]*PUBLISH[ \t]+					{
											*pMsgType = SIP_MSG_PUBLISH;
											BEGIN	URI;		
										}

^[ \t]*REFER[ \t]+						{
											*pMsgType = SIP_MSG_REFER;
											BEGIN	URI;		
										}

^[ \t]*REGISTER[ \t]+					{
											*pMsgType = SIP_MSG_REGISTER;
											BEGIN	URI;		
										}

^[ \t]*SUBSCRIBE[ \t]+					{
											*pMsgType = SIP_MSG_SUBSCRIBE;
											BEGIN	URI;		
										}

^[ \t]*UPDATE[ \t]+						{
											*pMsgType = SIP_MSG_UPDATE;
											BEGIN	URI;		
										}

<URI>{STR4}								{
											if(pURI[0] == 0x00) {
												len = (yyleng > SIP_URI_LEN) ? SIP_URI_LEN : yyleng;
												memcpy(pURI, yytext, len);
												pURI[len] = 0x00;
											}
											BEGIN 0;
										}

From[ \t]*:[ \t]*						{ 
											BEGIN FROM;
										}

<FROM>{STR7}							{
											len = (yyleng > SIP_FROM_LEN) ? SIP_FROM_LEN : yyleng;
											memcpy(pFrom, yytext, len);
											pFrom[len] = 0x00;
											BEGIN 0;
										}

To[ \t]*:[ \t]*							{ 
											BEGIN TO;
										}

<TO>{STR7}								{
											len = (yyleng > SIP_TO_LEN) ? SIP_TO_LEN : yyleng;
											memcpy(pTo, yytext, len);
											pTo[len] = 0x00;
											BEGIN 0;
										}

username[ \t]*=[ \t]*					{
											BEGIN USERNAME;
										}

<USERNAME>{STR4}						{
											len = (yyleng > SIP_USERNAME_LEN) ? SIP_USERNAME_LEN : yyleng;
											memcpy(pUserName, yytext, len);
											pUserName[len] = 0x00;
											BEGIN 0;
											BEGIN SESSID;
										}

<SESSID>[ \t]+{STR4}					{
											*pSessID = atoi(yytext);
											BEGIN 0;

										}

m[ \t]*=[ \t]*							{
											BEGIN AUDIO_VIDEO;
										}

<AUDIO_VIDEO>message[ \t]+				{
											BEGIN 0;
											BEGIN MESSAGE_PORT;	
										}

<MESSAGE_PORT>{STR4}					{
											*pMsrpPort = atoi(yytext);
											BEGIN 0;
										}

<AUDIO_VIDEO>audio[ \t]+				{
											BEGIN 0;
											BEGIN AUDIO_PORT;	
										}

<AUDIO_PORT>{STR4}						{
											*pAudioPort = atoi(yytext);
											BEGIN 0;
											BEGIN AUDIO_PROTO;
										}

<AUDIO_PROTO>[ \t]+						{
											BEGIN 0;
											BEGIN AUDIO_PROTO_SUB;
										}

<AUDIO_PROTO_SUB>{STR4}					{
											len = (yyleng > SIP_PROTO_LEN) ? SIP_PROTO_LEN : yyleng;
											memcpy(pAudioProto, yytext, len);
											pAudioProto[len] = 0x00;
											BEGIN 0;
										}

<AUDIO_VIDEO>video[ \t]+				{
											BEGIN 0;
											BEGIN VIDEO_PORT;	
										}

<VIDEO_PORT>{STR4}						{
											*pVideoPort = atoi(yytext);
											BEGIN 0;
											BEGIN VIDEO_PROTO;
										}

<VIDEO_PROTO>[ \t]+						{
											BEGIN 0;
											BEGIN VIDEO_PROTO_SUB;
										}

<VIDEO_PROTO_SUB>{STR4}					{
											len = (yyleng > SIP_PROTO_LEN) ? SIP_PROTO_LEN : yyleng;
											memcpy(pVideoProto, yytext, len);
											pVideoProto[len] = 0x00;
											BEGIN 0;
										}

User-Agent[ \t]*:[ \t]*					{
											BEGIN USERAGENT;
										}

<USERAGENT>{STR7}						{
											len = (yyleng > SIP_USERAGENT_LEN) ? SIP_USERAGENT_LEN : yyleng;
											memcpy(pUserAgent, yytext, len);
											pUserAgent[len] = 0x00;
											BEGIN 0;
										}

Authorization[ \t]*:[ \t]*				{
											BEGIN AUTH;
										}

WWW-Authenticate[ \t]*:[ \t]*			{
											BEGIN AUTH;
										}

<AUTH>{STR7}nonce[ \t]*=[ \t]*\"		{
											BEGIN 0;
											BEGIN NONCE;
										}

<NONCE>{STR6}							{
											len = (yyleng > SIP_NONCE_LEN) ? SIP_NONCE_LEN : yyleng;
											memcpy(pAuth_nonce, yytext, len);
											pAuth_nonce[len] = 0x00;
											BEGIN 0;
										}

^[ \t]*event[ \t]*:[ \t]*presence		{
											*pEvent = SIP_EVENT_PRES;
										}

^[ \t]*event[ \t]*:[ \t]*conference		{
											*pEvent = SIP_EVENT_CONF;
										}

^[ \t]*contact[ \t]*:[ \t]*				{
											BEGIN CONTACT;
										}

<CONTACT>{STR7}							{
											len = (yyleng > SIP_CONTACT_LEN) ? SIP_CONTACT_LEN : yyleng;
											memcpy(pContact, yytext, len);
											pContact[len] = 0x00;
											BEGIN 0;
										}

^[ \t]*accept-contact[ \t]*:[ \t]*		{
											BEGIN ACCEPT_CONTACT;
										}

<ACCEPT_CONTACT>{STR7}					{
											len = (yyleng > SIP_CONTACT_LEN) ? SIP_CONTACT_LEN : yyleng;
											memcpy(pAccept, yytext, len);
											pAccept[len] = 0x00;
											BEGIN 0;
										}

(\r\n)									{	}

[ \t\n]									{	}

.										{	}


%%

int sipwrap(void)
{
	return 1;
}

int sip(char *sp, int slen, char *uri, char *from, char *to, unsigned int *sessid, unsigned short *audioport, unsigned short *videoport, char *useragent, char *auth_nonce, char *username, char *audioproto, char *videoproto, int *msgtype, int *event, char *contact, char *accept, unsigned short *msgport)
{
	pURI = uri;
	pFrom = from;
	pTo = to;
	pSessID = sessid;
	pAudioPort = audioport;
	pVideoPort = videoport;
	pMsrpPort = msgport;
	pUserAgent = useragent;
	pAuth_nonce = auth_nonce;
	pUserName = username;
	pAudioProto = audioproto;
	pVideoProto = videoproto;
	pMsgType = msgtype;
	pEvent = event;
	pContact = contact;
	pAccept = accept;

	sip_scan_bytes(sp, slen);

	siplex();

	sip_delete_buffer(yy_current_buffer);
	yy_init = 1;

	return 0;
}

#ifdef TEST
int main(void)
{
	int iRet = -1;
	char szBuf[1024*10];
	char szURI[1024];
	char szFrom[1024];
	char szTo[1024];
	unsigned int SessID;
	unsigned short AudioPort;
	unsigned short VideoPort;
	char szUserAgent[1024];
	char szAuth_nonce[1024];
	char szUserName[1024];
	char szAudioProto[1024];
	char szVideoProto[1024];
	int	dMsgType = 0;
	int	dEvent = 0;
	char szContact[BUFSIZ];
	char szAccept[BUFSIZ];

	char *sp;
	int	slen;

	szURI[0] = 0x00;
	szFrom[0] = 0x00;
	szTo[0] = 0x00;
	szContact[0] = 0x00;
	szAccept[0] = 0x00;

	pURI = szURI;
	pFrom = szFrom;
	pTo = szTo;
	pSessID = &SessID;
	pAudioPort = &AudioPort;
	pVideoPort = &VideoPort;
	pUserAgent = szUserAgent;
	pAuth_nonce = szAuth_nonce;
	pUserName = szUserName;
	pAudioProto = szAudioProto;
	pVideoProto = szVideoProto;
	pMsgType = &dMsgType;
	pEvent = &dEvent;
	pContact = szContact;
	pAccept = szAccept;

//	sprintf(szBuf, "INVITE tel:15720003003 SIP/2.0\r\nCall-ID: YKgLAKTzcGoAABPEO_ZlQQ@211.246.101.65\r\nContent-Length:472\r\nCSeq: 1 INVITE\r\nFrom: MyAgent_2 <SIP:0161001002@ktf.com>;tag=LAKTzcGoAABPEO_ZlQQ\r\n");
//	sprintf(szBuf, "INVITE tel:15720003003 SIP/2.0\r\nCall-ID: YKgLAKTzcGoAABPEO_ZlQQ@211.246.101.65\r\nContent-Length:472\r\nCSeq: 1 INVITE\r\nFrom: MyAgent_2 <SIP:0161003124@ktf.com>;tag=YKgLAKTzcGoAABPEO_ZlQQ123\r\nTO: sips:0161001002@ktf.com;tag=KKYKgLAKTzcGoAABPEO_ZlQQKK\r\n");
//	sprintf(szBuf, "SIP/2.0 100 Trying\r\nCall-ID: YKgLAKTzcGoAABPEO_ZlQQ@211.246.101.65\r\nContent-Length:472\r\nCSeq: 1 INVITE\r\nFrom: MyAgent_2 <SIP:0161001002@ktf.com>;tag=YKgLAKTzcGoAABPEO_ZlQQ123\r\nTO: sip:0161001002@ktf.com;tag=KKYKgLAKTzcGoAABPEO_ZlQQKK\r\n");
	sprintf(szBuf, "INVITE tel:15720003003 SIP/2.0\r\nCall-ID: YKgLAKTzcGoAABPEO_ZlQQ@211.246.101.65\r\nContent-Length:472\r\nCSeq: 1 INVITE\r\nFrom: MyAgent_2 <SIP:0161003124@ktf.com>;tag=YKgLAKTzcGoAABPEO_ZlQQ123\r\nTO: sips:0161001002@ktf.com;tag=KKYKgLAKTzcGoAABPEO_ZlQQKK\r\nContact: <sip:01024441019@10.154.66.193:5060>;transport=udp;+g.oma.sip-im\r\nAccept-Contact: *;lgt-mim\r\nUser-Agent: Poc-client/OMA1.0\r\n\r\no=- 1170740904 1170740904 IN IP4 211.246.101.65\r\nm=audio 40000 RTP/AVP 106 8 4 18 0 101\r\nm=video 40002 RTP/AVP 107 34 31\r\n\r\n");
	sp = szBuf;
	slen = strlen(szBuf);

	sip_scan_bytes(sp, slen);
	
	iRet = siplex();
	printf("RETURN [%d]\n", iRet);
	printf("URI: [%s]\n", szURI);
	printf("FROM: [%s]\n", szFrom);
	printf("TO: [%s]\n", szTo);
	printf("SessID: [%u]\n", SessID);
	printf("AudioPort: [%hu]\n", AudioPort);
	printf("VideoPort: [%hu]\n", VideoPort);
	printf("UserAgent: [%s]\n", szUserAgent);
	printf("Auth_nonce: [%s]\n", szAuth_nonce);
	printf("UserName: [%s]\n", szUserName);
	printf("AudioProto [%s]\n", szAudioProto);
	printf("VideoProto [%s]\n", szVideoProto);
	printf("MSGTYPE [%d]\n", dMsgType);
	printf("EVENT [%d]\n", dEvent);
	printf("CONTACT [%s]\n", szContact);
	printf("ACCEPT-CONTACT [%s]\n", szAccept);

	return 0;
}
#endif
