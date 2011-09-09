/* RTSP Parsing Test */

%{
	#include <stdio.h>
	#include <string.h>

	#include "rtsp.h"

	unsigned char *man;
	int	len;
%}

STR			[^\r\n ,\t]+
NUM			[0-9]+

%s			MAN

%%

MAN[ \t]*=[ \t]* 						{ 
											BEGIN MAN;
										}

<MAN>{STR}								{
											len = (yyleng > MAX_MSMAN_LEN) ? MAX_MSMAN_LEN : yyleng;
											memcpy(man, yytext, len);
											man[len] = 0x00;
											BEGIN 0;
											return 0;
										}

(\r\n)									{	}

[ \t\n]									{	}

.										{	}


%%

int rtsp_reqhdrwrap(void)
{
	return 1;
}

int rtsp_reqhdr(char *sp, int slen, char *szMAN)
{
	man = szMAN;

	rtsp_reqhdr_scan_bytes(sp, slen);

	rtsp_reqhdrlex();

	rtsp_reqhdr_delete_buffer(yy_current_buffer);
	yy_init = 1;

	return 0;
}

#ifdef TEST
int main(void)
{
	int iRet = -1;
	char szBuf[1024*10];
	char szMAN[MAX_MSMAN_SIZE];

	char *sp;
	int	slen;

	man = szMAN;

	sprintf(szBuf, 
        "DESCRIBE rtsp://211.43.200.141/3966/070278924_WAP.k3g?PID=409000175118&CID=70278924&BUY_CHANNEL=001 RTSP/1.0\r\n" 
        "CSeq: 0\r\n" 
        "User-Agent: ClickMovie(MAN=LG,CTN=0001073509912,VCT=03,ACT=01,SCT=01,CLR=08,LCD=1,VER=0100)\r\n" 
        "Accept: application/sdp\r\n" 
        "User-Network: Qualcomm\r\n" 
        "DeviceInfo: MSM6XX0\r\n" 
		);


	sp = szBuf;
	slen = strlen(szBuf);

	rtsp_reqhdr_scan_bytes(sp, slen);
	
	iRet = rtsp_reqhdrlex();

	printf("RETURN [%d]\n", iRet);
	printf("MAN: [%s]\n", szMAN);
	return 0;
}
#endif
