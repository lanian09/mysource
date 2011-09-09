/* File Viewer Parsing Test */

%{
	#include <stdio.h>
	#include <string.h>
	#include "fv.h"
 
	int *type;
%}

STR			[^\r\n \t]+
NUM			[0-9]+

%%

\/DocView 								{ 
											*type = APP_FV_DOC;
											return 0;
										}

\/PageView 								{ 
											*type = APP_FV_PAGE;
											return 0;
										}

\/SaveView 								{ 
											*type = APP_FV_SAVE;
											return 0;
										}

(\r\n)									{	}

[ \t\n]									{	}

.										{	}


%%

int fv_typewrap(void)
{
	return 1;
}

int fv_type(char *sp, int slen, int *pdType)
{
	type = pdType;
	*type = APP_ETC;

	fv_type_scan_bytes(sp, slen);

	fv_typelex();

	fv_type_delete_buffer(yy_current_buffer);
	yy_init = 1;

	return 0;
}

#ifdef TEST
int main(void)
{
	int iRet = -1;
	char szBuf[1024*10];
	int dType;

	char *sp;
	int	slen;

	type = &dType;
	*type = APP_ETC;

	sprintf(szBuf, "http://211.43.200.141/ocView");

	sp = szBuf;
	slen = strlen(szBuf);

	fv_type_scan_bytes(sp, slen);
	
	iRet = fv_typelex();

	printf("RETURN [%d]\n", iRet);
	printf("TYPE: [%d]\n", dType);
	return 0;
}
#endif
