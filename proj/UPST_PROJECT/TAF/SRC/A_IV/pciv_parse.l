/* Full Viewer Parsing Test */

%{
	#include <stdio.h>
	#include <string.h>
	#include <typedef.h>
	#include <common_stg.h>
	#include <pciv_func.h> 
	int		*pCMD;
	int		*pLast;
%}

%%

^GetServerList 							{ 
											*pLast = yyleng + 1;
											*pCMD = eGetServerList;
											return 0;
										}

^GetServerInfo 							{ 
											*pLast = yyleng + 1;
											*pCMD = eGetServerInfo;
											return 0;
										}

^GetBrokerMessage 						{ 
											*pLast = yyleng + 1;
											*pCMD = eGetBrokerMessage;
											return 0;
										}

^Open 									{ 
											*pLast = yyleng + 1;
											*pCMD = eOpen;
											return 0;
										}

^CheckAndVersion 						{ 
											*pLast = yyleng + 1;
											*pCMD = eCheckAndVersion;
											return 0;
										}

^IsOpened 								{ 
											*pLast = yyleng + 1;
											*pCMD = eIsOpened;
											return 0;
										}

^Navigate 								{ 
											*pLast = yyleng + 1;
											*pCMD = eNavigate;
											return 0;
										}

^GetCurrentUrl 							{ 
											*pLast = yyleng + 1;
											*pCMD = eGetCurrentUrl;
											return 0;
										}

^GetFullSize 							{ 
											*pLast = yyleng + 1;
											*pCMD = eGetFullSize;
											return 0;
										}

^GetTitle 								{ 
											*pLast = yyleng + 1;
											*pCMD = eGetTitle;
											return 0;
										}

^WaitCompleted							{ 
											*pLast = yyleng + 1;
											*pCMD = eWaitCompleted;
											return 0;
										}

^Close									{ 
											*pLast = yyleng + 1;
											*pCMD = eClose;
											return 0;
										}

^Click									{ 
											*pLast = yyleng + 1;
											*pCMD = eClick;
											return 0;
										}

^IsCompleted							{ 
											*pLast = yyleng + 1;
											*pCMD = eIsCompleted;
											return 0;
										}

^GetRectImage							{ 
											*pLast = yyleng + 1;
											*pCMD = eGetRectImage;
											return 0;
										}

^GetObjectImage							{ 
											*pLast = yyleng + 1;
											*pCMD = eGetObjectImage;
											return 0;
										}

^SetInputData							{ 
											*pLast = yyleng + 1;
											*pCMD = eSetInputData;
											return 0;
										}

^GetInputData							{ 
											*pLast = yyleng + 1;
											*pCMD = eGetInputData;
											return 0;
										}

^GetFullImage							{ 
											*pLast = yyleng + 1;
											*pCMD = eGetFullImage;
											return 0;
										}

^GetThumbnailImage						{ 
											*pLast = yyleng + 1;
											*pCMD = eGetThumbnailImage;
											return 0;
										}

^GetAnchorList							{ 
											*pLast = yyleng + 1;
											*pCMD = eGetAnchorList;
											return 0;
										}

^GetObjectList							{ 
											*pLast = yyleng + 1;
											*pCMD = eGetObjectList;
											return 0;
										}

^GoBack									{ 
											*pLast = yyleng + 1;
											*pCMD = eGoBack;
											return 0;
										}

^GoForward								{ 
											*pLast = yyleng + 1;
											*pCMD = eGoForward;
											return 0;
										}

^Refresh								{ 
											*pLast = yyleng + 1;
											*pCMD = eRefresh;
											return 0;
										}

^Stop									{ 
											*pLast = yyleng + 1;
											*pCMD = eStop;
											return 0;
										}

^MouseOver								{ 
											*pLast = yyleng + 1;
											*pCMD = eMouseOver;
											return 0;
										}

^GetLoginItems							{ 
											*pLast = yyleng + 1;
											*pCMD = eGetLoginItems;
											return 0;
										}

^SetLoginItems							{ 
											*pLast = yyleng + 1;
											*pCMD = eSetLoginItems;
											return 0;
										}

^GetTicket								{ 
											*pLast = yyleng + 1;
											*pCMD = eGetTicket;
											return 0;
										}

^InitTicket								{ 
											*pLast = yyleng + 1;
											*pCMD = eInitTicket;
											return 0;
										}

^GetCookies								{ 
											*pLast = yyleng + 1;
											*pCMD = eGetCookies;
											return 0;
										}

^ObjectClick							{ 
											*pLast = yyleng + 1;
											*pCMD = eObjectClick;
											return 0;
										}

^ExecuteSnapshot						{ 
											*pLast = yyleng + 1;
											*pCMD = eExecuteSnapshot;
											return 0;
										}

^GetActiveWindow						{ 
											*pLast = yyleng + 1;
											*pCMD = eGetActiveWindow;
											return 0;
										}

^SetActiveWindow						{ 
											*pLast = yyleng + 1;
											*pCMD = eSetActiveWindow;
											return 0;
										}

^CheckRectImage							{ 
											*pLast = yyleng + 1;
											*pCMD = eCheckRectImage;
											return 0;
										}

^GetChunkTrain							{ 
											*pLast = yyleng + 1;
											*pCMD = eGetChunkTrain;
											return 0;
										}

^GetAllheaders							{ 
											*pLast = yyleng + 1;
											*pCMD = eGetAllheaders;
											return 0;
										}

^GetFontSize							{ 
											*pLast = yyleng + 1;
											*pCMD = eGetFontSize;
											return 0;
										}

^SetFontSize							{ 
											*pLast = yyleng + 1;
											*pCMD = eSetFontSize;
											return 0;
										}

^GetFlvUrl								{ 
											*pLast = yyleng + 1;
											*pCMD = eGetFlvUrl;
											return 0;
										}

^SetSelectData							{ 
											*pLast = yyleng + 1;
											*pCMD = eSetSelectData;
											return 0;
										}

^SetPopupWindow							{ 
											*pLast = yyleng + 1;
											*pCMD = eSetPopupWindow;
											return 0;
										}

^Exit									{ 
											*pLast = yyleng + 1;
											*pCMD = eExit;
											return 0;
										}

^IsChangedDocument						{ 
											*pLast = yyleng + 1;
											*pCMD = eIsChangedDocument;
											return 0;
										}

^GetExViewURL							{ 
											*pLast = yyleng + 1;
											*pCMD = eGetExViewURL;
											return 0;
										}

^CheckRectImageEx						{ 
											*pLast = yyleng + 1;
											*pCMD = eCheckRectImageEx;
											return 0;
										}

^SetLCDSize								{ 
											*pLast = yyleng + 1;
											*pCMD = eSetLCDSize;
											return 0;
										}

^GetText								{ 
											*pLast = yyleng + 1;
											*pCMD = eGetText;
											return 0;
										}

^MouseDrag								{ 
											*pLast = yyleng + 1;
											*pCMD = eMouseDrag;
											return 0;
										}

^RegionCheck							{ 
											*pLast = yyleng + 1;
											*pCMD = eRegionCheck;
											return 0;
										}

^MouseOverEx							{ 
											*pLast = yyleng + 1;
											*pCMD = eMouseOverEx;
											return 0;
										}

^ClickEx								{ 
											*pLast = yyleng + 1;
											*pCMD = eClickEx;
											return 0;
										}

^OpenEx									{ 
											*pLast = yyleng + 1;
											*pCMD = eOpenEx;
											return 0;
										}

^SetTicketDuration						{ 
											*pLast = yyleng + 1;
											*pCMD = eSetTicketDuration;
											return 0;
										}

^SendXCData								{ 
											*pLast = yyleng + 1;
											*pCMD = eSendXCData;
											return 0;
										}

^GetChunkTrainForce						{ 
											*pLast = yyleng + 1;
											*pCMD = eGetChunkTrainForce;
											return 0;
										}

^EvAppWithURL							{ 
											*pLast = yyleng + 1;
											*pCMD = eEvAppWithURL;
											return 0;
										}

^EvChangeSize							{ 
											*pLast = yyleng + 1;
											*pCMD = eEvChangeSize;
											return 0;
										}

^EvChangeURL							{ 
											*pLast = yyleng + 1;
											*pCMD = eEvChangeURL;
											return 0;
										}

^EvAlert								{ 
											*pLast = yyleng + 1;
											*pCMD = eEvAlert;
											return 0;
										}

^EvPopupWindow							{ 
											*pLast = yyleng + 1;
											*pCMD = eEvPopupWindow;
											return 0;
										}

^EvXCExecute							{ 
											*pLast = yyleng + 1;
											*pCMD = eEvXCExecute;
											return 0;
										}

^ThisIsJustPing							{ 
											*pLast = yyleng + 1;
											*pCMD = eThisIsJustPing;
											return 0;
										}

(\r\n)									{	}

[ \t\n]									{	}

.										{	}


%%

int pciv_parsewrap(void)
{
	return 1;
}

int pciv_parse(char *sp, int slen, int *cmd, int *last)
{
	pCMD = cmd;
	pLast = last;	

	pciv_parse_scan_bytes(sp, slen);

	pciv_parselex();

	pciv_parse_delete_buffer(yy_current_buffer);
	yy_init = 1;

	return 0;
}

#ifdef TEST
int main(void)
{
	int iRet = -1;
	char szBuf[1024*10];
	int	cmd = 0;
	int last = 0;

	char *sp;
	int	slen;

	pCMD = &cmd;
	pLast = &last;

	sprintf(szBuf, "ClickEx,  dddd");

	sp = szBuf;
	slen = strlen(szBuf);

	pciv_parse_scan_bytes(sp, slen);
	
	iRet = pciv_parselex();

	printf("RETURN [%d]\n", iRet);
	printf("ORG=%s CMD=%d LAST=%d\n", szBuf, cmd, last);
	return 0;
}
#endif
