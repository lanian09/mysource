/* RTSP Parsing Test */

%{
	#include <stdio.h>
	#include <string.h>

	#include "rtsp.h"

	unsigned int *session;
	unsigned short *rti;
	unsigned short *porta;
	unsigned short *portb;
%}

STR			[^\r\n \t]+
NUM			[0-9]+

%s			SESSION
%s			RTI
%s			CPORTa
%s			CPORTb
%s			CPORT

%%

Session[ \t]*:[ \t]* 					{ 
											BEGIN SESSION;
										}

<SESSION>{NUM}							{
											*session = atoll(yytext);
											BEGIN 0;
										}

RTI[ \t]*=[ \t]* 						{ 
											BEGIN RTI;
										}

<RTI>{NUM}								{
											*rti = atoi(yytext);
											BEGIN 0;
										}

client_port[ \t]*=[ \t]* 				{ 
											BEGIN CPORTa;
										}

<CPORTa>{NUM}							{
											*porta = atoi(yytext);
											BEGIN 0;
											BEGIN CPORT;
										}

<CPORT>[ \t]*-[ \t]*					{
											BEGIN 0;
											BEGIN CPORTb;
										}


<CPORTb>{NUM}							{
											*portb = atoi(yytext);
											BEGIN 0;
										}

(\r\n)									{	}

[ \t\n]									{	}

.										{	}


%%

int rtsp_reshdrwrap(void)
{
	return 1;
}

int rtsp_reshdr(char *sp, int slen, unsigned int *puiSession, unsigned short *pusRTI, unsigned short *pusPort1, unsigned short *pusPort2)
{
	session = puiSession;
	rti = pusRTI;
	porta = pusPort1;
	portb = pusPort2;

	rtsp_reshdr_scan_bytes(sp, slen);

	rtsp_reshdrlex();

	rtsp_reshdr_delete_buffer(yy_current_buffer);
	yy_init = 1;

	return 0;
}

#ifdef TEST
int main(void)
{
	int iRet = -1;
	char szBuf[1024*10];
	unsigned int uiSession;
	unsigned short usRTI;
	unsigned short usPort1;
	unsigned short usPort2;

	char *sp;
	int	slen;

	session = &uiSession;
	rti = &usRTI;
	porta = &usPort1;
	portb = &usPort2;

	sprintf(szBuf, 
		"RTSP/1.0 200 OK\r\n" 
        "CSeq: 4\r\n" 
        "Last-Modified: Mon, 15 Jan 2007 02:54:05 GMT\r\n" 
        "Date: Tue, 21 Aug 2007 06:26:37 GMT\r\n" 
        "Session: 2600869822\r\n" 
		"Transport: RTP/AVP;unicast;client_port=6976-6977;server_port=12900-12901;"
		"source=192.168.235.10;destination=10.163.30.216;ssrc=fa577bbe\r\n"
        "Server: XenonStreamer/4.1.9.3194 (MAN=VID,VER=0200,BUF=05,RTI=020)\r\n" 
        "Range: npt=0.0000-\r\n" 
        "RTP-Info: url=rtsp://211.43.200.141/3966/070278924_WAP.k3g/trackID=101;seq=1;rtptime=0,"
		"url=rtsp://211.43.200.141/3966/070278924_WAP.k3g/trackID=201;seq=1;rtptime=0\r\n" 
        "RTCP-Interval: 1000\r\n\r\n"); 

	sp = szBuf;
	slen = strlen(szBuf);

	rtsp_reshdr_scan_bytes(sp, slen);
	
	iRet = rtsp_reshdrlex();

	printf("RETURN [%d]\n", iRet);
	printf("Session: [%u]\n", uiSession);
	printf("RTI: [%d]\n", dRTI);
	printf("Port1: [%d]\n", usPort1);
	printf("Port2: [%d]\n", usPort2);
	return 0;
}
#endif
