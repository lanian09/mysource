%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <ems.h>
	
    int             *pRet;
    int             len;
    char            *pszTransactionID;
    char            *pszResult;

%}

XMLVALUE        [^<>\n*]+

%s      TRANSACTIONID
%s      RESULT

%%

\<[ \t]*result[ \t]*\>			{
                                	BEGIN RESULT;
								}

\<[ \t]*transactionid[ \t]*\>  	{
									BEGIN TRANSACTIONID;
                            	}

<TRANSACTIONID>{XMLVALUE}		{
									len = (yyleng > MAX_TRANSID_LEN) ? MAX_TRANSID_LEN : yyleng;
									memcpy(pszTransactionID, yytext, len);
									pszTransactionID[len] = 0x00;
									*pRet=1;
									BEGIN 0;
								}

<RESULT>{XMLVALUE}				{
									len = (yyleng > MAX_APPFAILCODE_LEN) ? MAX_APPFAILCODE_LEN : yyleng;
									memcpy(pszResult, yytext, len);
									pszResult[len] = 0x00;
									*pRet=1;
									BEGIN 0;
								}

\r								{ }
\n								{ }
.								{ }

%%

int emsrespbodywrap(void)
{
    return 1;
}

int emsrespbody(char *sp, int slen, char *szTransactionID, char *szResult)
{
    int     dFlag=0;

    pRet                = &dFlag;
    pszTransactionID    = szTransactionID;
    pszResult			= szResult;	

#if 0
    printf("TRANSACTIONID[%s], RESULT[%s]\n", szTransactionID, szResult);
#endif

    emsrespbody_scan_bytes(sp, slen);
    emsrespbodylex();

    emsrespbody_delete_buffer(yy_current_buffer);
    yy_init = 1;

    return *pRet;
}

#ifdef  TEST
int main(int argc, char *argv[])
{
    int     		dFlag = 0;
    char    		szTransactionID[MAX_TRANSID_SIZE];
	char			szResult[MAX_APPFAILCODE_SIZE];

    char    *sp;
    int     slen;

    pRet                = &dFlag;
    pszTransactionID    = szTransactionID;
    pszResult			= szResult;	

    if (argc < 2) {
        printf("%s ", argv[0]);
        return(1);
    }

    sp = strdup(argv[1]);
    slen = strlen(sp);

    emsrespbody_scan_bytes(sp, slen);
    emsrespbodylex();
	
	printf("\nSUCCFLAG=[%d]\n", *pRet);
    printf("transactionid=[%s]\n", szTransactionID);
    printf("result=[%s]\n", szResult);

    return 0;
}
#endif
