/* Full Viewer Parsing Test */

%{
	#include <stdio.h>
	#include <string.h>

	#include "page.h"
 
	int		*pJS;
%}

STR			[^\r\n \t]+
NUM			[0-9]{10,}

%s			USERAGENT
%s			MIN
%%

\.js 									{ 
											*pJS = 1;
										}

\.jsp 									{ 
											*pJS = 0;
										}

(\r\n)									{	}

[ \t\n]									{	}

.										{	}


%%

int page_jswrap(void)
{
	return 1;
}

int page_js(char *sp, int slen, int *js)
{
	pJS = js;

	page_js_scan_bytes(sp, slen);

	page_jslex();

	page_js_delete_buffer(yy_current_buffer);
	yy_init = 1;

	return 0;
}

#ifdef TEST
int main(void)
{
	int iRet = -1;
	int	js = 0;
	char	szBuf[BUFSIZ];

	char *sp;
	int	slen;

	pJS = &js;

//	sprintf(szBuf, "get http://www.naver.com/js/www.js?20080415");
//	sprintf(szBuf, "get http://www.naver.com/js/www.jsp?20080415");
//	sprintf(szBuf, "get http://www.naver.com/js/www.js");
	sprintf(szBuf, "get http://www.naver.com/js/www.jsp");

	sp = szBuf;
	slen = strlen(szBuf);

	page_js_scan_bytes(sp, slen);
	
	iRet = page_jslex();

	printf("RETURN [%d]\n", iRet);
	printf("js: [%d]\n", js);
	return 0;
}
#endif
