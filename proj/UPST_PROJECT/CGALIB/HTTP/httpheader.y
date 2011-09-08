/* HTTP Parsing Test */
/*
 * YACC은 파싱에 성공을 하면 return 0
 * 실패하는 경우 return 양수 
 * 파싱에 성공 여부를 알아야 하기 때문에 return 값으로 상태값을 받으려면 안된다.
 */
%{
	#include <stdio.h>
	#include "http.h"

	void httpheadererror(char *);
	int	httpheaderlex(void);
	int httpheader_scan_bytes(char *sp, int slen);
	void ReleasehttpheaderBuffer(void);
	char *pucLogUrl;
	int	*pdLogLen;
	int	*pdRes;
	int *pdRst;
	int *pUrlType;
%}

%token LFCR
%token URL1 URL2 URL3 URL4
%token GET POST HEAD PUT DELETE TRACE CONNECT RESULT
%token DESCRIBE ANNOUNCE GET_PARAMETER OPTIONS PAUSE PLAY
%token RECORD REDIRECT SETUP SET_PARAMETER TEARDOWN
%token HTTP RTSP ERROR
%token RESP RESCODE

%%

program:
		| program response 
		| program request
		| program end
		;

request:
		GET URL1 HTTP LFCR				{ *pdRst = METHOD_GET; *pUrlType = URL_TYPE_HOST_PORT; return 0; }
		| GET URL2 HTTP LFCR			{ *pdRst = METHOD_GET; *pUrlType = URL_TYPE_HOST_NOPORT; return 0; }
		| GET URL3 HTTP LFCR			{ *pdRst = METHOD_GET; *pUrlType = URL_TYPE_NOHOST_NOPORT; return 0; }
		| POST URL1 HTTP LFCR			{ *pdRst = METHOD_POST; *pUrlType = URL_TYPE_HOST_PORT; return 0; }
		| POST URL2 HTTP LFCR			{ *pdRst = METHOD_POST; *pUrlType = URL_TYPE_HOST_NOPORT; return 0; }
		| POST URL3 HTTP LFCR			{ *pdRst = METHOD_POST; *pUrlType = URL_TYPE_NOHOST_NOPORT; return 0; }
		| HEAD URL1 HTTP LFCR			{ *pdRst = METHOD_HEAD; *pUrlType = URL_TYPE_HOST_PORT; return 0; }
		| HEAD URL2 HTTP LFCR			{ *pdRst = METHOD_HEAD; *pUrlType = URL_TYPE_HOST_NOPORT; return 0; }
		| HEAD URL3 HTTP LFCR			{ *pdRst = METHOD_HEAD; *pUrlType = URL_TYPE_NOHOST_NOPORT; return 0; }
		| OPTIONS URL1 HTTP LFCR		{ *pdRst = METHOD_OPTIONS; *pUrlType = URL_TYPE_HOST_PORT; return 0; }
		| OPTIONS URL2 HTTP LFCR		{ *pdRst = METHOD_OPTIONS; *pUrlType = URL_TYPE_HOST_NOPORT; return 0; }
		| OPTIONS URL3 HTTP LFCR		{ *pdRst = METHOD_OPTIONS; *pUrlType = URL_TYPE_NOHOST_NOPORT; return 0; }
		| PUT URL1 HTTP LFCR			{ *pdRst = METHOD_PUT; *pUrlType = URL_TYPE_HOST_PORT; return 0; }
		| PUT URL2 HTTP LFCR			{ *pdRst = METHOD_PUT; *pUrlType = URL_TYPE_HOST_NOPORT; return 0; }
		| PUT URL3 HTTP LFCR			{ *pdRst = METHOD_PUT; *pUrlType = URL_TYPE_NOHOST_NOPORT; return 0; }
		| DELETE URL1 HTTP LFCR			{ *pdRst = METHOD_DELETE; *pUrlType = URL_TYPE_HOST_PORT; return 0; }
		| DELETE URL2 HTTP LFCR			{ *pdRst = METHOD_DELETE; *pUrlType = URL_TYPE_HOST_NOPORT; return 0; }
		| DELETE URL3 HTTP LFCR			{ *pdRst = METHOD_DELETE; *pUrlType = URL_TYPE_NOHOST_NOPORT; return 0; }
		| TRACE URL1 HTTP LFCR			{ *pdRst = METHOD_TRACE; *pUrlType = URL_TYPE_HOST_PORT; return 0; }
		| TRACE URL2 HTTP LFCR			{ *pdRst = METHOD_TRACE; *pUrlType = URL_TYPE_HOST_NOPORT; return 0; }
		| TRACE URL3 HTTP LFCR			{ *pdRst = METHOD_TRACE; *pUrlType = URL_TYPE_NOHOST_NOPORT; return 0; }
		| CONNECT URL4 HTTP LFCR		{ *pdRst = METHOD_CONNECT; *pUrlType = URL_TYPE_CONNECT; return 0; }
		| RESULT LFCR					{ *pdRst = METHOD_RESULT; *pUrlType = URL_TYPE_NOHOST_NOPORT; return 0; }
		| DESCRIBE URL1 RTSP LFCR		{ *pdRst = METHOD_DESCRIBE; *pUrlType = URL_TYPE_HOST_PORT; return 0; }
		| DESCRIBE URL2 RTSP LFCR		{ *pdRst = METHOD_DESCRIBE; *pUrlType = URL_TYPE_HOST_NOPORT; return 0; }
		| DESCRIBE URL3 RTSP LFCR		{ *pdRst = METHOD_DESCRIBE; *pUrlType = URL_TYPE_NOHOST_NOPORT; return 0; }
		| ANNOUNCE URL1 RTSP LFCR		{ *pdRst = METHOD_ANNOUNCE; *pUrlType = URL_TYPE_HOST_PORT; return 0; }
		| ANNOUNCE URL2 RTSP LFCR		{ *pdRst = METHOD_ANNOUNCE; *pUrlType = URL_TYPE_HOST_NOPORT; return 0; }
		| ANNOUNCE URL3 RTSP LFCR		{ *pdRst = METHOD_ANNOUNCE; *pUrlType = URL_TYPE_NOHOST_NOPORT; return 0; }
		| GET_PARAMETER URL1 RTSP LFCR		{ *pdRst = METHOD_GET_PARAMETER; *pUrlType = URL_TYPE_HOST_PORT; return 0; }
		| GET_PARAMETER URL2 RTSP LFCR		{ *pdRst = METHOD_GET_PARAMETER; *pUrlType = URL_TYPE_HOST_NOPORT; return 0; }
		| GET_PARAMETER URL3 RTSP LFCR		{ *pdRst = METHOD_GET_PARAMETER; *pUrlType = URL_TYPE_NOHOST_NOPORT; return 0; }
		| OPTIONS URL1 RTSP LFCR		{ *pdRst = METHOD_OPTIONS; *pUrlType = URL_TYPE_HOST_PORT; return 0; }
		| OPTIONS URL2 RTSP LFCR		{ *pdRst = METHOD_OPTIONS; *pUrlType = URL_TYPE_HOST_NOPORT; return 0; }
		| OPTIONS URL3 RTSP LFCR		{ *pdRst = METHOD_OPTIONS; *pUrlType = URL_TYPE_NOHOST_NOPORT; return 0; }
		| PAUSE URL1 RTSP LFCR		{ *pdRst = METHOD_PAUSE; *pUrlType = URL_TYPE_HOST_PORT; return 0; }
		| PAUSE URL2 RTSP LFCR		{ *pdRst = METHOD_PAUSE; *pUrlType = URL_TYPE_HOST_NOPORT; return 0; }
		| PAUSE URL3 RTSP LFCR		{ *pdRst = METHOD_PAUSE; *pUrlType = URL_TYPE_NOHOST_NOPORT; return 0; }
		| PLAY URL1 RTSP LFCR		{ *pdRst = METHOD_PLAY; *pUrlType = URL_TYPE_HOST_PORT; return 0; }
		| PLAY URL2 RTSP LFCR		{ *pdRst = METHOD_PLAY; *pUrlType = URL_TYPE_HOST_NOPORT; return 0; }
		| PLAY URL3 RTSP LFCR		{ *pdRst = METHOD_PLAY; *pUrlType = URL_TYPE_NOHOST_NOPORT; return 0; }
		| RECORD URL1 RTSP LFCR		{ *pdRst = METHOD_RECORD; *pUrlType = URL_TYPE_HOST_PORT; return 0; }
		| RECORD URL2 RTSP LFCR		{ *pdRst = METHOD_RECORD; *pUrlType = URL_TYPE_HOST_NOPORT; return 0; }
		| RECORD URL3 RTSP LFCR		{ *pdRst = METHOD_RECORD; *pUrlType = URL_TYPE_NOHOST_NOPORT; return 0; }
		| REDIRECT URL1 RTSP LFCR		{ *pdRst = METHOD_REDIRECT; *pUrlType = URL_TYPE_HOST_PORT; return 0; }
		| REDIRECT URL2 RTSP LFCR		{ *pdRst = METHOD_REDIRECT; *pUrlType = URL_TYPE_HOST_NOPORT; return 0; }
		| REDIRECT URL3 RTSP LFCR		{ *pdRst = METHOD_REDIRECT; *pUrlType = URL_TYPE_NOHOST_NOPORT; return 0; }
		| SETUP URL1 RTSP LFCR		{ *pdRst = METHOD_SETUP; *pUrlType = URL_TYPE_HOST_PORT; return 0; }
		| SETUP URL2 RTSP LFCR		{ *pdRst = METHOD_SETUP; *pUrlType = URL_TYPE_HOST_NOPORT; return 0; }
		| SETUP URL3 RTSP LFCR		{ *pdRst = METHOD_SETUP; *pUrlType = URL_TYPE_NOHOST_NOPORT; return 0; }
		| SET_PARAMETER URL1 RTSP LFCR		{ *pdRst = METHOD_SET_PARAMETER; *pUrlType = URL_TYPE_HOST_PORT; return 0; }
		| SET_PARAMETER URL2 RTSP LFCR		{ *pdRst = METHOD_SET_PARAMETER; *pUrlType = URL_TYPE_HOST_NOPORT; return 0; }
		| SET_PARAMETER URL3 RTSP LFCR		{ *pdRst = METHOD_SET_PARAMETER; *pUrlType = URL_TYPE_NOHOST_NOPORT; return 0; }
		| TEARDOWN URL1 RTSP LFCR		{ *pdRst = METHOD_TEARDOWN; *pUrlType = URL_TYPE_HOST_PORT; return 0; }
		| TEARDOWN URL2 RTSP LFCR		{ *pdRst = METHOD_TEARDOWN; *pUrlType = URL_TYPE_HOST_NOPORT; return 0; }
		| TEARDOWN URL3 RTSP LFCR		{ *pdRst = METHOD_TEARDOWN; *pUrlType = URL_TYPE_NOHOST_NOPORT; return 0; }
		;

response:
		RESP RESCODE					{ *pdRst = METHOD_RESPONSE; *pUrlType = 0; return 0; }
		;

end:
		ERROR							{ *pdRst = -1; *pUrlType = 0; return 1; }
		;

		
%%

void httpheadererror(char *s)
{
	*pdRst = -1;
	pucLogUrl[0] = 0x00;
	*pdRes = 0;
	*pdLogLen = 0;
	*pUrlType = 0;
}

int httpheader(char *sp, int slen, char *szLogUrl, int *pdLogUrlLen, int *pdUrlType, int *pdResCode, int *pdRet)
{
	int		dRet;
	pucLogUrl = szLogUrl;
	pdLogLen = pdLogUrlLen;
	pdRes = pdResCode;
	pdRst = pdRet;
	pUrlType = pdUrlType;

	httpheader_scan_bytes(sp, slen);
	
	dRet = httpheaderparse();

	ReleasehttpheaderBuffer();

	return dRet;
}

#ifdef TEST
S32 GetURL(U8 *inUrl, S32 urlLen, S32 urlType, U8 *hostName, S32 hostNameLen, U8 *outUrl)
{
    int     i;
    int     flag = 0;
    int     startOffset, portLen = 0;
    int     newUrlLen = 0;
    int     tmpLen = 0;
    int     copyLen = 0;

    switch(urlType)
    {
    case URL_TYPE_HOST_PORT:
        for(i = 7; i < urlLen; i++)
        {
            if((flag == 0) && (inUrl[i] == ':')) {
                flag = 1;
                startOffset = i;
            } else if(flag == 1) {
                if((inUrl[i] >= '0') && (inUrl[i] <= '9')) {
                    portLen++;
                } else {
                    memcpy(outUrl, inUrl, startOffset);
                    memcpy(&outUrl[startOffset], &inUrl[startOffset+portLen+1], urlLen - (startOffset+portLen+1)); 
                    newUrlLen = urlLen - (portLen + 1);
                    outUrl[newUrlLen] = 0x00;
                    break;
                }
            }

            if(inUrl[i] == '/') break;
        }
        break;
    case URL_TYPE_HOST_NOPORT:
        memcpy(outUrl, inUrl, urlLen);
        newUrlLen = urlLen;
        outUrl[newUrlLen] = 0x00;   
        break;
    case URL_TYPE_NOHOST_NOPORT:
        if(hostNameLen > 0) {
            sprintf(outUrl, "http://%s", hostName);
            tmpLen = hostNameLen + 7;
            if(urlLen + tmpLen > MAX_URL_LEN) {
                copyLen = urlLen - (urlLen + tmpLen - MAX_URL_LEN);
                newUrlLen = MAX_URL_LEN;
            } else {
                copyLen = urlLen;
                newUrlLen = urlLen + tmpLen;
            }

            memcpy(&outUrl[tmpLen], inUrl, copyLen);
            outUrl[newUrlLen] = 0x00;
		}
        break;
    default:
        printf("[%s][%s.%d] UNKNOWN URLTYPE[%d]", __FILE__, __FUNCTION__, __LINE__, urlType);
        newUrlLen = 0;
        break;
    }

    return newUrlLen;
}

int main(void)
{
	int iRet = -1;
	char szBuf[1024*10];
	char ucLogUrl[1024*2];
	char outUrl[1024];
	int	dLogLen;
	int	dRes;
	int dRst;
	int dUrlType;

	char hostName[1024];
	int hostNameLen = 0;

	char *sp;
	int	slen;

	pucLogUrl = ucLogUrl;
	pucUrl = ucUrl;
	pdRes = &dRes;
	pdRst = &dRst;
	pUrlType = &dUrlType;

	sprintf(hostName, "mvod12.magicn.com");
	hostNameLen = strlen(hostName);

	//sprintf(szBuf, "GET http://mvod.magicn.com:80/KUN/00/00/00/72/11/721141_1.asp?menuid=721141 HTTP/1.1\r\n");
	//sprintf(szBuf, "GET http://mvod.magicn.com:/KUN/00/00/00/72/11/721141_1.asp?menuid=721141 HTTP/1.1\r\n");
	//sprintf(szBuf, "GET http://mvod.magicn.com/KUN/00/00/00/72/11/721141_1.asp?menuid=721141 HTTP/1.1\r\n");
	//sprintf(szBuf, "GET /KUN/00/00/00/72/11/721141_1.asp?menuid=721141 HTTP/1.1\r\n");
	//sprintf(szBuf, "HTTP/1.1 200 OK\r\n");
	//sprintf(szBuf, "TTP/1.1 200 OK\r\n");
	//sprintf(szBuf, "CONNECT ent.wooribank.com:443 HTTP/1.1\r\n");
	sprintf(szBuf, "GET http://ktfwipidcinfo.magicn.com:8081/comm/adult_info.asp?prevTitle=스피드맞고2&prevUrl=http%3A%2F%2Fktfwipidcmagicncom%2Fpage%2F34584asp%3Fhistoryhttp%3A%2F%2Fktfwipidcmagicncom%2Fpage%2Fdefaultasp%608%A8%3Bhttp%3A%2F%2Fktfwipidc-AQUAddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddAAAAAAAAAAAAAAAAAAAAAAAA-DDD=11111111111111122222222222222223333333333333334444444-D HTTP/1.1\r\n");
//	sprintf(szBuf, "GET http:///guide.asp HTTP/1.1\r\n");

	sp = szBuf;
	slen = strlen(szBuf);

	httpheader_scan_bytes(sp, slen);
	
	iRet = httpheaderparse();
	printf("RETURN [%d] RST[%d]\n", iRet, dRst);
	printf("LOGURL: TYPE[%d]LEN[%d][%s]\n", dUrlType, dLogLen, ucLogUrl);
	printf("RESCODE: [%d]\n", dRes);
	
//	dLen = GetURL(ucUrl, dLen, dUrlType, hostName, hostNameLen, outUrl);
//	printf("SIZE[%d] HOSTNAME[%s]\n", hostNameLen, hostName);
//	printf("SIZE[%d] URL[%s]\n", dLen, outUrl);
	return 0;
}
#endif
