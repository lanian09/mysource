%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <ems.h>
	
    int             *pRet;
    int             len;
    char            *pszTransactionID;
    unsigned short  *pusClientType;
    unsigned short  *pusClientPlatform;
    char            *pszClientVersion;
    char 			*pszCommand;
    char            *pszSmtpServer;
    char            *pszPop3Server;
    unsigned short  *pusSmtpSsl;
    unsigned short  *pusPop3Ssl;
    unsigned int    *puiPeriodTime;
    unsigned short  *pusParam;
    unsigned short  *pusImageRecv;

%}

VALUE        [^&]+

%s      TRANSACTIONID
%s      CLIENTTYPE
%s      CLIENTPLATFORM
%s      CLIENTVERSION
%s      COMMAND
%s      SMTPSERVER
%s      POP3SERVER
%s      SMTPSSL
%s      POP3SSL
%s      PERIODTIME
%s      PARAM
%s      IMAGERECV

%%

transactionid[ \t]*=[ \t]*  {
                                BEGIN TRANSACTIONID;
                            }

clienttype[ \t]*=[ \t]*  	{
                                BEGIN CLIENTTYPE;
                            }

clientplatform[ \t]*=[ \t]* {
                                BEGIN CLIENTPLATFORM;
                            }

clientversion[ \t]*=[ \t]*  {
                                BEGIN CLIENTVERSION;
                            }
command[ \t]*=[ \t]*		{  
								BEGIN COMMAND;
							}
smtpserver[ \t]*=[ \t]*		{ 
								BEGIN SMTPSERVER;
							}
pop3server[ \t]*=[ \t]*		{ 
								BEGIN POP3SERVER;
							}
smtpssl[ \t]*=[ \t]*  		{   
								BEGIN SMTPSSL;
							}
pop3ssl[ \t]*=[ \t]*		{      
								BEGIN POP3SSL;
							}
periodtime[ \t]*=[ \t]*  	{    
								BEGIN PERIODTIME;
							}
param[ \t]*=[ \t]*			{      
								BEGIN PARAM;
							}
imagerecv[ \t]*=[ \t]*		{    
								BEGIN IMAGERECV;
							}

<TRANSACTIONID>{VALUE}   	{
                                len = (yyleng > MAX_TRANSID_LEN) ? MAX_TRANSID_LEN : yyleng;
                                memcpy(pszTransactionID, yytext, len);
                                pszTransactionID[len] = 0x00;
                                *pRet=1;
                                BEGIN 0;
							}
<CLIENTTYPE>{VALUE}			{
								*pusClientType = atoi(yytext);
                                *pRet=1;
								BEGIN 0;
							}
<CLIENTPLATFORM>{VALUE}		{
								*pusClientPlatform = atoi(yytext);
                                *pRet=1;
								BEGIN 0;
							}
<CLIENTVERSION>{VALUE}		{
                                len = (yyleng > MAX_CLIVER_LEN) ? MAX_CLIVER_LEN : yyleng;
                                memcpy(pszClientVersion, yytext, len);
                                pszClientVersion[len] = 0x00;
                                *pRet=1;
                                BEGIN 0;
							}
<COMMAND>{VALUE}			{
                                len = (yyleng > MAX_COMMAND_LEN) ? MAX_COMMAND_LEN : yyleng;
                                memcpy(pszCommand, yytext, len);
                                pszCommand[len] = 0x00;
                                *pRet=1;
                                BEGIN 0;
							}
<SMTPSERVER>{VALUE}			{
                                len = (yyleng > MAX_SMTPSVR_LEN) ? MAX_SMTPSVR_LEN : yyleng;
                                memcpy(pszSmtpServer, yytext, len);
                                pszSmtpServer[len] = 0x00;
                                *pRet=1;
                                BEGIN 0;
							}
<POP3SERVER>{VALUE}			{
                                len = (yyleng > MAX_POP3SVR_LEN) ? MAX_POP3SVR_LEN : yyleng;
                                memcpy(pszPop3Server, yytext, len);
                                pszPop3Server[len] = 0x00;
                                *pRet=1;
                                BEGIN 0;
							}
<POP3SSL>{VALUE}			{
								if (strstr(yytext, "yes") != NULL) {
									*pusPop3Ssl = 1;
								} else {
									*pusPop3Ssl = 2;
								}
								BEGIN 0;
							}
<SMTPSSL>{VALUE}			{
								if (strstr(yytext, "yes") != NULL) {
									*pusSmtpSsl = 1;
								} else {
									*pusSmtpSsl = 2;
								}
								BEGIN 0;
							}
<PERIODTIME>{VALUE}			{
								*puiPeriodTime = atoi(yytext);
                                *pRet=1;
								BEGIN 0;
							}
<PARAM>{VALUE}				{
								if (strstr(yytext, "norefresh") != NULL) {
									*pusParam = 2;
								} else {
									*pusParam = 1;
								}
								BEGIN 0;
							}
<IMAGERECV>{VALUE}			{
								if (strstr(yytext, "yes") != NULL) {
									*pusImageRecv = 1;
								} else {
									*pusImageRecv = 2;
								}
								BEGIN 0;
							}
\r							{ }
\n							{ }
.							{ }

%%

int emsreqbodywrap(void)
{
    return 1;
}

int emsreqbody(char *sp, int slen, char *szTransactionID, unsigned short *usClientType, unsigned short *usClientPlatform, char *szClientVersion, char *szCommand, char *szSmtpServer, char *szPop3Server, unsigned short *usSmtpssl, unsigned short *usPop3ssl, unsigned int *uiPeriodTime, unsigned short *usParam, unsigned short *usImageRecv)
{
    int     dFlag=0;

    pRet                = &dFlag;
    pszTransactionID    = szTransactionID;
    pusClientType		= usClientType;	
    pusClientPlatform	= usClientPlatform;	
    pszClientVersion	= szClientVersion;	
    pszCommand			= szCommand;	
	pszSmtpServer		= szSmtpServer;
	pszPop3Server		= szPop3Server;
	pusSmtpSsl			= usSmtpssl;
	pusPop3Ssl			= usPop3ssl;
    puiPeriodTime		= uiPeriodTime;
    pusParam			= usParam;
    pusImageRecv		= usImageRecv;

#if 0
    printf("uiCmdType[%d], uiPeriod[%d], uiStartTime[%d], szIMSI[%s]\n", 
            *uiCmdType, *uiPeriod, *uiStartTime, szIMSI);
#endif

    emsreqbody_scan_bytes(sp, slen);
    emsreqbodylex();

    emsreqbody_delete_buffer(yy_current_buffer);
    yy_init = 1;

    return *pRet;
}

#ifdef  TEST
int main(int argc, char *argv[])
{
    int     		dFlag = 0;
    char    		szTransactionID[MAX_TRANSID_SIZE];
	unsigned short	usClientType;
	unsigned short	usClientPlatform;
	char			szClientVersion[MAX_CLIVER_SIZE];
	char			szCommand[MAX_COMMAND_SIZE];
	char			szsmtpServer[MAX_SMTPSVR_SIZE];
	char 			szpop3Server[MAX_POP3SVR_SIZE];
	unsigned short	usSmtpssl;
	unsigned short	usPop3ssl;
    unsigned int    uiPeriodTime;
    unsigned short  usParam;
    unsigned short  usImageRecv;


    char    *sp;
    int     slen;

    pRet                = &dFlag;
    pszTransactionID    = szTransactionID;
    pusClientType		= &usClientType;	
    pusClientPlatform	= &usClientPlatform;	
    pszClientVersion	= szClientVersion;	
    pszCommand			= szCommand;	
	pszSmtpServer		= szsmtpServer;
	pszPop3Server		= szpop3Server;
	pusSmtpSsl			= &usSmtpssl;
	pusPop3Ssl			= &usPop3ssl;
    puiPeriodTime		= &uiPeriodTime;
    pusParam			= &usParam;
    pusImageRecv		= &usImageRecv;

    if (argc < 2) {
        printf("%s transactionid=01080803836_-1659063011&clienttype=1&clientplatform=1&clientversion=1.0&command=acknowledgement&result=0&operation=newmaillist&\r\n", argv[0]);
        return(1);
    }

    sp = strdup(argv[1]);
    slen = strlen(sp);

    emsreqbody_scan_bytes(sp, slen);
    emsreqbodylex();
	
	printf("\nSUCCFLAG=[%d]\n", *pRet);
    printf("transactionid=[%s]\n", szTransactionID);
    printf("clienttype=[%d]\n", usClientType);
    printf("clientplatform=[%d]\n", usClientPlatform);
    printf("clientversion=[%s]\n", szClientVersion);
    printf("command=[%s]\n", szCommand);
    printf("smptserver=[%s]\n", szsmtpServer);
    printf("pop3server=[%s]\n", szpop3Server);
    printf("smtpssl=[%d]\n", usSmtpssl);
    printf("pop3ssl=[%d]\n", usPop3ssl);
    printf("PeriodTime=[%d]\n", uiPeriodTime);
    printf("Param=[%d]\n", usParam);
    printf("ImageRecv=[%d]\n", usImageRecv);

    return 0;
}
#endif
