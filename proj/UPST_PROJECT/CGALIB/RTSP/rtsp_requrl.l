/* RTSP Parsing Test */

%{
	#include <stdio.h>
	#include <string.h>

	#include "rtsp.h"

	int *trackid;
%}

STR			[^\r\n \t]+
NUM			[0-9]+

%s			TRACKID

%%

trackID[ \t]*=[ \t]* 					{ 
											BEGIN TRACKID;
										}

<TRACKID>{NUM}							{
											*trackid = atoi(yytext);
											BEGIN 0;
											return 0;
										}

(\r\n)									{	}

[ \t\n]									{	}

.										{	}


%%

int rtsp_requrlwrap(void)
{
	return 1;
}

int rtsp_requrl(char *sp, int slen, int *pdTrackID)
{
	trackid = pdTrackID;

	rtsp_requrl_scan_bytes(sp, slen);

	rtsp_requrllex();

	rtsp_requrl_delete_buffer(yy_current_buffer);
	yy_init = 1;

	return 0;
}

#ifdef TEST
int main(void)
{
	int iRet = -1;
	char szBuf[1024*10];
	int dTrackID;

	char *sp;
	int	slen;

	trackid = &dTrackID;

	sprintf(szBuf, "rtsp://211.43.200.141/3966/070278924_WAP.k3g/trackID=201");

	sp = szBuf;
	slen = strlen(szBuf);

	rtsp_requrl_scan_bytes(sp, slen);
	
	iRet = rtsp_requrllex();

	printf("RETURN [%d]\n", iRet);
	printf("TrackID: [%d]\n", dTrackID);
	return 0;
}
#endif
