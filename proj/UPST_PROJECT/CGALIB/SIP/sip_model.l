/* HTTP Parsing Test */

%{
	#include <stdio.h>
	#include <string.h>

	#include "common_stg.h"
	#include "sip.h"

	char			*pModel;
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
STR8		[^\r\n]*

%s			MODEL

%%


LGT\-client\/{STR8}MIM[ \t]*[0-9]+\.[0-9]+[ \t]*	{
														BEGIN MODEL;
													}

<MODEL>{STR7}										{
                                            			len = (yyleng > MAX_MODEL_LEN) ? MAX_MODEL_LEN : yyleng;
                                            			memcpy(pModel, yytext, len);
                                            			pModel[len] = 0x00;
                                            			BEGIN 0;
														return 0;
													}

(\r\n)												{	}

[ \t\n]												{	}

.													{	}


%%

int sip_modelwrap(void)
{
	return 1;
}

int sip_model(char *sp, int slen, char *model)
{
	pModel = model;

	sip_model_scan_bytes(sp, slen);

	sip_modellex();

	sip_model_delete_buffer(yy_current_buffer);
	yy_init = 1;

	return 0;
}

#ifdef TEST
int main(void)
{
	int iRet = -1;
	char szBuf[BUFSIZ];
	char model[1024];

	char *sp;
	int	slen;

	model[0] = 0;

	pModel = model;

//	sprintf(szBuf, "LGT-client/IM1.0 LGT");
//	sprintf(szBuf, "LGT-client/VT1.5 + MIM 1.0 LU1400");
//	sprintf(szBuf, "LGT-client/VT1.5 + MIM 1.0 LU1600");
//	sprintf(szBuf, "LGT-client/MIM1.0 SPH-W6450");
//	sprintf(szBuf, "LGT-client/VT1.5 + MIM 1.0 SPH-W6450");
	sprintf(szBuf, "IM-serv");

	sp = szBuf;
	slen = strlen(szBuf);

	sip_model_scan_bytes(sp, slen);
	
	iRet = sip_modellex();

	printf("RETURN [%d]\n", iRet);
	printf("Model: [%s]\n", model);

	return 0;
}
#endif
