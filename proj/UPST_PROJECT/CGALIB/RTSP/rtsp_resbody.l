/* RTSP Parsing Test */

%{
	#include <stdio.h>
	#include <string.h>

	#include "rtsp.h"
 
	int *min_range;
	int *min_range_m;
	int *max_range;
	int *max_range_m;
	int *video;
	int *audio;
	int	len;
	int flag;
	int	range_flag;
%}

STR			[^\r\n \t]+
NUM			[0-9]+

%s			MIN_RANGEa
%s			MIN_RANGEb
%s			MIN_RANGEc
%s			MAX_RANGEa
%s			MAX_RANGEb
%s			MAX_RANGEc
%s			RANGE
%s			TRACKID

%%

a[ \t]*=[ \t]*range[ \t]*:[ \t]*npt[ \t]*=[ \t]*							{ 
																				if(range_flag == 0) {
																					BEGIN MIN_RANGEa;
																					range_flag = 1;
																				}
																			}

<MIN_RANGEa>{NUM}															{
																				*min_range = atoi(yytext);
																				BEGIN 0;
																				BEGIN MIN_RANGEb;
																			}

<MIN_RANGEb>.																{
																				BEGIN 0;
																				BEGIN MIN_RANGEc;
																			}

<MIN_RANGEc>{NUM}															{
															
																				*min_range_m = atoi(yytext);
																				BEGIN 0;
																				BEGIN RANGE;
																			}

<RANGE>[ \t]*-[ \t]*														{
																				BEGIN 0;
																				BEGIN MAX_RANGEa;
																			}

<MAX_RANGEa>{NUM}															{
																				*max_range = atoi(yytext);
																				BEGIN 0;
																				BEGIN MAX_RANGEb;
																			}

<MAX_RANGEb>.																{
																				BEGIN 0;
																				BEGIN MAX_RANGEc;
																			}

<MAX_RANGEc>{NUM}															{
																				*max_range_m = atoi(yytext);
																				BEGIN 0;
																			}

m[ \t]*=[ \t]*video 														{ 
																				flag = 1;
																			}

m[ \t]*=[ \t]*audio 														{ 
																				flag = 2;
																			}

a[ \t]*=[ \t]*control[ \t]*:[ \t]*trackID[ \t]*=[ \t]*						{
																				BEGIN TRACKID;
																			}

<TRACKID>{NUM}																{
																				if(flag == 1) {
																					*video = atoi(yytext);
																				} else if(flag == 2) {
																					*audio = atoi(yytext);
																				}
																				BEGIN 0;
																			}

(\r\n)																		{	}

[ \t\n]																		{	}

.																			{	}


%%

int rtsp_resbodywrap(void)
{
	return 1;
}

int rtsp_resbody(char *sp, int slen, int *pdMinRange, int *pdMinMRange, int *pdMaxRange, int *pdMaxMRange, int *pdVideoTrackID, int *pdAudioTrackID)
{
	min_range = pdMinRange;
	min_range_m = pdMinMRange;
	max_range = pdMaxRange;
	max_range_m = pdMaxMRange;
	video = pdVideoTrackID;
	audio = pdAudioTrackID;

	flag = 0;
	range_flag = 0;

	rtsp_resbody_scan_bytes(sp, slen);

	rtsp_resbodylex();

	rtsp_resbody_delete_buffer(yy_current_buffer);
	yy_init = 1;

	return 0;
}

#ifdef TEST
int main(void)
{
	int iRet = -1;
	char szBuf[1024*10];
	int dMaxRange;
	int dMaxMRange;
	int dMinRange;
	int dMinMRange;
	int dVideoTrackID;
	int dAudioTrackID;

	char *sp;
	int	slen;

	min_range = &dMinRange;
	min_range_m = &dMinMRange;
	max_range = &dMaxRange;
	max_range_m = &dMaxMRange;
	video = &dVideoTrackID;
	audio = &dAudioTrackID;

	flag = 0;
	range_flag = 0;

	sprintf(szBuf,
        "v=0\r\n" 
        "o=- 2600869822 1187677592 IN IP4 192.168.235.10\r\n" 
        "s=070278924_WAP.k3g\r\n"
        "u=<none>\r\n" 
        "e=<none>\r\n" 
        "c=IN IP4 0.0.0.0 \r\n"
        "t=0 0\r\n" 
        "a=control:rtsp://211.43.200.141/3966/070278924_WAP.k3g\r\n" 
        "a=range:npt=1.000056-152.230000\r\n"
        "a=X-ProvideLGTbillingdata\r\n" 
        "m=video 0 RTP/AVP 97\r\n"
        "b=AS:64\r\n" 
        "b=TIAS:63635\r\n" 
        "b=RS:480\r\n" 
        "b=RR:640\r\n" 
        "a=maxprate:5.00\r\n" 
        "a=X-initpredecbufperiod:90000\r\n" 
        "a=X-initpostdecbufperiod:450000\r\n" 
        "a=range:npt=0.000000-152.152000\r\n" 
        "a=rtpmap:97 MP4V-ES/30000\r\n" 
        "a=control:trackID=201\r\n"
        "a=fmtp:97 profile-level-id=8;framesize=176-144;"
	 "config=000001B008000001B24E4558545245414D494E4720434F52504F524154494F4E000001B509000001000000012000845D4C31F490584121463F\r\n" 
        "m=audio 0 RTP/AVP 110\r\n"
        "b=AS:16\r\n" 
        "b=TIAS:15942\r\n" 
        "b=RS:480\r\n" 
        "b=RR:398\r\n" 
        "a=maxprate:3.00\r\n" 
        "a=range:npt=0.000000-152.230000\r\n" 
        "a=rtpmap:110 MP4A-LATM/11025/1\r\n" 
        "a=control:trackID=101\r\n"
        "a=fmtp:110 profile-level-id=15; object=2; bitrate=15942; cpresent=0; config=42002A103FC0; SBR-enabled=0\r\n\r\n" 
		);

	sp = szBuf;
	slen = strlen(szBuf);

	rtsp_resbody_scan_bytes(sp, slen);
	
	iRet = rtsp_resbodylex();

	printf("RETURN [%d]\n", iRet);
	printf("MinRange: [%d]\n", dMinRange);
	printf("MinMRange: [%d]\n", dMinMRange);
	printf("MaxRange: [%d]\n", dMaxRange);
	printf("MaxMRange: [%d]\n", dMaxMRange);
	printf("VideoTrackID: [%d]\n", dVideoTrackID);
	printf("AudioTrackID: [%d]\n", dAudioTrackID);
	return 0;
}
#endif
