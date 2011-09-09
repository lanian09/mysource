#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

char    resBuf[4096*16], resHead[4096], resTmp[4096];

#define MAX_RULE_SET_LIST   5000
int g_TotalRuleSetCnt;
typedef struct __RuleSetList__ {
	unsigned short  pBit;
	unsigned short  hBit;
	unsigned short  pkgNo;
} RuleSetList; 

RuleSetList         g_stRuleSetList[MAX_RULE_SET_LIST];

#define PBTABLE_PATH "/DSC/NEW/DATA/RULESET_LIST.conf"


int readRuleSet(void)
{   
	FILE *fp = NULL;
	int No = 0, Pbit = 0, Hbit = 0, PkgNo = 0, RedNo = 0, SmsOnOff = 0;
	char buf[128] = {0,};
	RuleSetList *pstRule = &g_stRuleSetList[0];

	if( (fp = fopen(PBTABLE_PATH, "r")) != NULL )
	{
		while( (fgets(buf, sizeof(buf), fp)) != NULL )
		{
			if (buf[0] == '#' )
				continue;
			else
			{
				sscanf( buf, "%d %d %d %d %d %d", &No, &Pbit, &Hbit, &PkgNo, &RedNo, &SmsOnOff );
				pstRule[PkgNo].pkgNo = PkgNo;
				pstRule[PkgNo].pBit = Pbit;
				pstRule[PkgNo].hBit = Hbit;
			}
		}
	}
	else
		return -1;

	fclose(fp);
	return 0;
}


int main(void)
{
	char *filename = "./test.txt";
	FILE *fp = NULL;
	int             ret;
	int             idx = 0;
	char            txBuf[4096*16]; 
	int             RuleId=0, Pbit=-1, Hbit=-1;
	int             Page = 0, PageM = 0, PageN = 0;
	char            IMSI[16]={0,},IP[20]={0,},PKGID[6]={0,}, LAST_UPDATE[20]={0,};
	int             totalCnt=0,lineCnt = 0, colCnt = 0, p = 0, i = 0, complete=0, find = 0;
	unsigned int    frameIP;
	struct in_addr client ;
	int             pageEnd = 0, alpha = 0;

	ret = readRuleSet();
	if( ret < 0 )
	{
		return -1;
	}

	memset(txBuf, 0, sizeof(txBuf));

	if( (fp = fopen(filename, "r")) != NULL)
	{
		sprintf(txBuf+idx, "\n    RESULT = SUCCESS   ");  idx = strlen(txBuf);
		sprintf(txBuf+idx, "\n    -------------------------------------------------------------------------"); idx = strlen(txBuf);
		sprintf(txBuf+idx, "\n    %15s %19s %12s %12s", "IMSI", "IP_ADDR", "RULESET_ID", "LAST_UPDATE"); idx = strlen (txBuf);
		sprintf(txBuf+idx, "\n    -------------------------------------------------------------------------"); idx = strlen(txBuf);

		while( fgets(resBuf, sizeof(resBuf), fp) != NULL )
		{
			if(strstr(resBuf, "Command terminated successfully"))
			{
				complete = 1;
				break;
			}
			else
			{
				sscanf();
				for(i = 0, p=0; i < strlen(resBuf); i++ )
				{
					if( colCnt > 2 && (resBuf[i] == ' ' || resBuf[i] == '\n') )
					{
						memset(PKGID,0,sizeof(PKGID));
						strncpy(PKGID, resBuf+p, i-p);
						RuleId = atoi(PKGID);
						p = i+1;
						colCnt=0;
						printf("PKG[%s]\n",PKGID);
						find = 0;
						if( g_stRuleSetList[RuleId].pkgNo == RuleId )
						{
							Pbit = g_stRuleSetList[RuleId].pBit;
							Hbit = g_stRuleSetList[RuleId].hBit;
						}
						else
						{
							Pbit = -1;
							Hbit = -1;
						}                                                                                         
						if( resBuf[i] == '\n')
							break;

						++lineCnt;
						sprintf(txBuf+idx, "\n    %15s %19s       %02d_%02d   %20s ", IMSI, IP, Pbit, Hbit, LAST_UPDATE);
						idx = strlen (txBuf);
					}
					else if( resBuf[i] == ',' )
					{
						if( colCnt == 0 )
						{
							memset(IMSI,0x00,sizeof(IMSI));
							strncpy(IMSI, resBuf+p, i-p);
							p = i+1;
							colCnt++;
							printf("%d: IMSI[%s]\t",lineCnt, IMSI);
						}
						else if( colCnt == 1 )
						{
							memset(IP,0x00,sizeof(IP));
							strncpy(IP, resBuf+p, i-p);
							frameIP = atoi(IP);
							client.s_addr = frameIP;
							strcpy(IP,inet_ntoa(client));                                                         
							p = i+1;
							colCnt++;
							printf("IP[%s]\t", IP);
						}
						else if( colCnt == 2 )
						{
							memset(LAST_UPDATE,0x00,sizeof(LAST_UPDATE));
							strncpy(LAST_UPDATE, resBuf+p, i-p);
							p = i+1;
							colCnt++;
							printf("LAST_UPDATE[%s]\t", LAST_UPDATE);
						}
					}
					else
					{
						continue;
					}
				}
			}
		}
		if( complete != 1)
		{
			memset(txBuf,0x00,sizeof(txBuf));
			sprintf(txBuf, "\n   RESULT = PARSING FAIL   ");
		}
		else
		{                                                                                                         
			sprintf(txBuf+idx, "\n    -------------------------------------------------------------------------"); idx = strlen(txBuf);
			sprintf(txBuf+idx, "\n    TOTAL = %d ", lineCnt);  idx = strlen(txBuf);                              
			sprintf(txBuf+idx, "\n    -------------------------------------------------------------------------\n"); idx = strlen(txBuf);
		}                                       

	}
	fclose(fp);

	printf("\n\n++++++++++++++++++++++\n");
	printf("%s\n", txBuf);

	return 0;
}
