#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ethernet.h>


#define UCHAR	unsigned char
#define SHORT	short
#define UINT	unsigned int
#define PBTABLE_PATH        "/DSC/NEW/DATA/RULESET_LIST.conf"

#define MAX_PBIT_CNT                        100
#define MAX_HBIT_CNT                        100

typedef struct _st_Pkginfo_ {
	UCHAR       ucUsedFlag;
	UCHAR       ucSMSFlag;
	UCHAR       ucReserved[2];
	SHORT       sPkgNo;
	SHORT       sRePkgNo;
} ST_PKG_INFO, *PST_PKG_INFO;

#define DEF_ST_PKG_INFO_SIZE    sizeof(ST_PKG_INFO)

typedef struct _st_PBTable_List_ {
	UINT            dCount;
	ST_PKG_INFO     stPBTable[MAX_PBIT_CNT][MAX_HBIT_CNT];      /* PBit, HBit Table */
} ST_PBTABLE_LIST, *PST_PBTABLE_LIST;

#define DEF_ST_PBTABLE_SIZE     sizeof(ST_PBTABLE_LIST) 

ST_PBTABLE_LIST		stPBTableList;

int dLoadPBTable(void);
void LogPBTable(void);

int main(int argc, char *args[] )
{
	char cmd1[256] = {0,};
	char buf[1024] = {0,};
	char *filename1 = "result1.txt";
	FILE *fp = NULL, *fp2 = NULL;
	int	pageM = 1, pageN = 30, len=0, cnt=0, RuleId =0;
	char IMSI[16]={0,}, IP[20]={0,}, PKG[6]={0,};
	char lineBuf[100]={0,};
	int lineCnt=0,colCnt = 0, p=0, i=0, j=0;
	int Pbit=0, Hbit=0;
	unsigned int	frameIP;
	struct in_addr client ; 


	if( argc < 2 )
		sprintf(cmd1, "sudo -u pcube /SM/pcube/sm/server/bin/p3db --dis-sess-page-info %d %d > %s",pageM,pageN, filename1);
	else
	{
		sprintf(cmd1, "sudo -u pcube /SM/pcube/sm/server/bin/p3db --dis-sess-page-info %d %d > %s",atoi(args[1]),atoi(args[2]), filename1);
		printf("sudo -u pcube /SM/pcube/sm/server/bin/p3db --dis-sess-page-info %d %d > %s\n",atoi(args[1]),atoi(args[2]), filename1);
	}

	system(cmd1);

	if( (fp = fopen(filename1, "r")) != NULL )
	{
		while( fgets(buf, sizeof(buf), fp) != NULL)
		{
			if(strstr(buf, "Command terminated successfully"))
				break;
			else
			{
				for(i = 0, p=0; i < strlen(buf); i++ )
				{
					if( buf[i] == ' ' || buf[i] == '\n')
					{
						++lineCnt;
						strncpy(PKG, buf+p, i-p);
						RuleId=atoi(PKG);
						p = i+1;
						colCnt=0;
						for(j = 0; j < stPBTableList.dCount; j++)
						{
							if( stPBTableList.stPBTable[Pbit][Hbit].sPkgNo == RuleId )
							{                                                                                 
								break;
							}
							else
							{
								Pbit++;
								Hbit++;
							}
						}
						printf("PKG[%s] : Pbit[%d], Hbit[%d]\n",PKG,Pbit,Hbit);
					}
					else if( buf[i] == ',' )
					{
						if( colCnt == 0 )
						{
							strncpy(IMSI, buf+p, i-p);
							p = i+1;
							colCnt++;
							printf("%d: IMSI[%s]\t",lineCnt, IMSI);
						}
						else if( colCnt == 1 )
						{
							strncpy(IP, buf+p, i-p);
							frameIP = atoi(IP);
							client.s_addr = frameIP; 
							strcpy(IP,inet_ntoa(client)); 
							p = i+1;
							colCnt++;
							printf("IP[%s]\t", IP);
						}
					}
					else
					{
						continue;
					}
				}
			}
		}
	}
	else
	{
		printf("%s file open fail.\n", filename1);
	}

	fclose(fp);


	printf("...........................\n");

	return 0;

}

void LogPBTable(void)
{
	int i, j;

	//  dAppLog(level, "##### PBTable LIST TOT[%u]####", stPBTableList.dCount);
	printf ("###### PBTable LIST TOT[%u]#####", stPBTableList.dCount);

	for( i=0; i<MAX_PBIT_CNT; i++) {
		for( j=0; j<MAX_HBIT_CNT ; j++) {
			if (stPBTableList.stPBTable[i][j].ucUsedFlag) {
				printf ("%d\t%d\t%d\t%d\t%d", i,j,
						stPBTableList.stPBTable[i][j].sPkgNo,
						stPBTableList.stPBTable[i][j].sRePkgNo,
						stPBTableList.stPBTable[i][j].ucSMSFlag );
			}
		}
	}
}


int dLoadPBTable(void)
{
	FILE                *fa;
	char                szBuffer[1024];
	int i, j;

	unsigned short              usNo, usPBit, usHBit;
	ST_PKG_INFO         stPKGInfo;

	memset(&stPBTableList, 0, DEF_ST_PBTABLE_SIZE);
	for( i=0; i<MAX_PBIT_CNT; i++) {
		for( j=0; j<MAX_HBIT_CNT ; j++) {
			stPBTableList.stPBTable[i][j].sPkgNo = -1;
			stPBTableList.stPBTable[i][j].sRePkgNo = -1;
		}
	}

	if( (fa=fopen(PBTABLE_PATH, "r")) == NULL) {
		printf ("[dLoadPBTable]: %s FILE NOT FOUND", PBTABLE_PATH);
		return -1;
	}

	fseek(fa, 0, SEEK_SET);
	while( fgets( szBuffer, 1024, fa ) != NULL ) {
		if(szBuffer[0] == '#')
			continue;

		if( sscanf( &szBuffer[0], "%hd %hd %hd %hd %hd %c"
					, &usNo
					, &usPBit
					, &usHBit
					, &stPKGInfo.sPkgNo
					, &stPKGInfo.sRePkgNo
					, &stPKGInfo.ucSMSFlag) == 6 )
		{
			if( (usPBit<=MAX_PBIT_CNT) && (usHBit<=MAX_HBIT_CNT) ) {
				stPBTableList.dCount++;
				stPKGInfo.ucUsedFlag = 1;
				stPBTableList.stPBTable[usPBit][usHBit] = stPKGInfo;

			} else {
				printf ("[CAUTION] VALUE OUT OF RANGE EXCEPTION PBIT[%d] HBIT[%d]", usPBit,usHBit);
				continue;
			}

		} else {
			printf ("[dLoadPBTable: %s FILE FORMAT ERROR",PBTABLE_PATH);
			fclose(fa);
			return -1;
		}
	}
	fclose(fa);
	LogPBTable();                                                                      

	return 0;      
}
