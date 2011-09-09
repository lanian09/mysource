#include "mmcd_proto.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <inttypes.h>
#include <netdb.h>



extern char		trcBuf[4096], trcTmp[1024];
extern char		mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN];


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int mmcd_bsrchCmp (const void *a, const void *b)
{
	return (strcasecmp ((char*)a, ((MmcdBuiltInCmdVector*)b)->cmdName));
} //----- End of mmcd_bsrchCmp -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int mmcd_qsortCmp (const void *a, const void *b)
{
	return (strcasecmp (((MmcdBuiltInCmdVector*)a)->cmdName, ((MmcdBuiltInCmdVector*)b)->cmdName));
} //----- End of mmcd_qsortCmp -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static char tmpTimeStamp[32];
char *mmcd_printTimeHHMMSS (time_t ttt)
{
	struct tm	*pLocalTime;
	if (ttt==0) {
		strcpy (tmpTimeStamp,"");
	} else if ((pLocalTime = (struct tm*)localtime((time_t*)&ttt)) == NULL) {
		strcpy (tmpTimeStamp,"");
	} else {
		strftime (tmpTimeStamp, 32, "%T", pLocalTime);
	}
	return ((char*)tmpTimeStamp);
} //----- End of mmcd_printTimeHHMMSS -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
char *mmcd_printTimeMMDDHHMMSS (time_t ttt)
{
	struct tm	*pLocalTime;
	if (ttt==0) {
		strcpy (tmpTimeStamp,"");
	} else if ((pLocalTime = (struct tm*)localtime((time_t*)&ttt)) == NULL) {
		strcpy (tmpTimeStamp,"");
	} else {
		strftime (tmpTimeStamp, 32, "%m-%d %T", pLocalTime);
	}
	return ((char*)tmpTimeStamp);
} //----- End of mmcd_printTimeMMDDHHMMSS -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
char *mmcd_printUserClass (char privilege)
{
	switch (privilege) {
		case MML_PRIVILEGE_SU:
			return (MML_PRIVILEGE_SU_STR);
		case MML_PRIVILEGE_NU:
			return (MML_PRIVILEGE_NU_STR);
		case MML_PRIVILEGE_GUEST:
			return (MML_PRIVILEGE_GUEST_STR);
	}
	return ("-");
} //----- End of mmcd_printUserClass -----//

char *mmcd_printUserFullClass (char privilege)
{
	switch (privilege) {
		case MML_PRIVILEGE_SU:
			return (MML_PRIVILEGE_SUPER_STR);
		case MML_PRIVILEGE_NU:
			return (MML_PRIVILEGE_NORMAL_STR);
		case MML_PRIVILEGE_GUEST:
			return (MML_PRIVILEGE_GUEST_STR);
	}
	return ("-");
} //----- End of mmcd_printUserClass -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void mmcd_dumpCmdTbl (MMLCmdContext *cmdTbl, MMLHelpContext *helpTbl)
{
	int	i,j,k;

	printf("\n========== MML_COMMANDS_TABLE ==========\n");
	for (i=0; i<MML_NUM_TP_CMD_TBL && strcmp(cmdTbl[i].cmdName, ""); i++) {
		printf("\ncmdName=%s, dst=%s-%s, cls=%d, paraCnt=%d\n",
				cmdTbl[i].cmdName, cmdTbl[i].dstSysName, cmdTbl[i].dstAppName,
				cmdTbl[i].privilege, cmdTbl[i].paraCnt);
		for (j=0; j<cmdTbl[i].paraCnt; j++) {
			printf("  pName=%s, pType=%s, mand=%d",
					cmdTbl[i].paraInfo[j].paraName,
					mmcd_printParaType(cmdTbl[i].paraInfo[j].paraType),
					cmdTbl[i].paraInfo[j].mandFlag);
			if (cmdTbl[i].paraInfo[j].paraType != MML_PTYPE_ENUM) {
				if (cmdTbl[i].paraInfo[j].paraType == MML_PTYPE_HEXA) {
					printf(", min=0x%x, max=0x%x\n",
							cmdTbl[i].paraInfo[j].minVal,
							cmdTbl[i].paraInfo[j].maxVal);
				} else {
					printf(", min=%d, max=%d\n",
							cmdTbl[i].paraInfo[j].minVal,
							cmdTbl[i].paraInfo[j].maxVal);
				}
			} else {
				for (k=0; k<MML_MAX_ENUM_ITEM && strcmp(cmdTbl[i].paraInfo[j].enumList[k].enumStr, ""); k++) {
					printf(", %s", cmdTbl[i].paraInfo[j].enumList[k].enumStr);
					if( strlen(cmdTbl[i].paraInfo[j].enumList[k].enumStr2)!=0 )
						printf("=%s", cmdTbl[i].paraInfo[j].enumList[k].enumStr2);
				}
				printf("\n");
			}
		}
		//printf("  slogan=\"%s\"\n", helpTbl[i].cmdSlogan);
		//printf("  help=\"%s\"\n", helpTbl[i].cmdHelp);
	}
	printf("\n========== MML_COMMANDS_COUNT : %d ==========\n", i);

	return;

} //----- End of mmcd_dumpCmdTbl -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
char *mmcd_printParaType (char pType)
{
	switch (pType) {
		case MML_PTYPE_DECIMAL: return("DECIMAL");
		case MML_PTYPE_HEXA:    return("HEXA");
		case MML_PTYPE_STRING:  return("STRING");
		case MML_PTYPE_ENUM:    return("ENUM");
		case MML_PTYPE_FIXSTR:  return("FIXSTR");
		case MML_PTYPE_FIXDEC:	return("FIXDEC");
		case MML_PTYPE_DECSTR:	return("DECSTR");
		default:                return("unknown");
	}
} //----- End of mmcd_printParaType -----//

//2009.07.16 by sjs
char* GetClientIPstr( unsigned int nSocketHandle )
{
	struct sockaddr_in sockAddr;
	memset( &sockAddr, 0, sizeof( sockAddr ) );

	int nSockAddrLen = sizeof(sockAddr);
	if( getpeername( nSocketHandle, ( struct sockaddr* )&sockAddr, &nSockAddrLen ) != -1 )
	{
		return inet_ntoa(sockAddr.sin_addr);
	}
	return NULL;
}

unsigned int GetClientIPhl( unsigned int nSocketHandle )
{
	struct sockaddr_in sockAddr;
	memset( &sockAddr, 0, sizeof( sockAddr ) );

	int nSockAddrLen = sizeof(sockAddr);
	if( getpeername( nSocketHandle, ( struct sockaddr* )&sockAddr, &nSockAddrLen ) != -1 )
	{
//		return ntohl( ( long )sockAddr.sin_addr );
	}
	return NULL;
}

