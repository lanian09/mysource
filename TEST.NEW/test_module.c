/*******************************************************************************
			DQMS Project

	Author   :
	Section  : QMON
	SCCS ID  : @(#)qmon_restart.c	1.1
	Date     :
	Revision History :

	Description :

	Copyright (c) uPRESTO 2005
*******************************************************************************/

/***** A.1 * File Include *******************************/
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <dqms_defines.h>
#include <define.h>
#include <tam_names.h>
#include <tam_define.h>
#include <tam_svc.h>
#include <tam_etc.h>
#include <tam_shm.h>
#include <almstat.h>
#include <common_stg.h>
#include <sys/msg.h>


#define MY_START_PATH "/home/uamyd/dqms/TAM_APP/SRC/TEST.NEW"
#define MY_MODEL_NAME_LEN	17
#define MY_MIN_LEN			16

typedef struct _st_Model{
	U8  szModel[MY_MODEL_NAME_LEN];
	U32	uiCount;
} st_Model;

typedef struct _st_Model_Cnt{
	st_Model stModel[1000];
	U32 uiModelCnt;
} st_Model_Cnt;

int dRecordModelCnt(st_Model_Cnt *pstMC, char *pszModel, int len)
{
	int i, isMatched, *pdCnt;
	st_Model *pstModel;

	printf("RecordModelCount:insert Model=[%s:%d]", pszModel,len);
	printf("\n");

	pstModel = &pstMC->stModel[0];
	pdCnt      = &pstMC->uiModelCnt;

	for( i = 0, isMatched = 0; i < *pdCnt; i++ ){
		if( strncmp( (pstModel+i)->szModel, pszModel, len ) == 0 ){
			(pstModel+i)->uiCount++;
			isMatched = 1;
			printf("insert model=%s, cnt=%d", pszModel, (pstModel+i)->uiCount);
			printf("\n");
		}
	}

	if( isMatched != 1 ){
			memcpy(&(pstModel+*pdCnt)->szModel[0], pszModel, len);
			(pstModel+*pdCnt)->szModel[len] = 0x00;
			(pstModel+*pdCnt)->uiCount++;
			(*pdCnt)++;
			printf("added model count %d -> %d", *pdCnt-1, *pdCnt);
			printf("\n");
	}
	return 0;
}

void printRecordStruct(st_Model_Cnt *pstMC)
{
	int i;
	st_Model *pstModel;
	FILE	*fp;
	char szTable[] = "TB_MODEL_COUNTING";

	
	if( (fp= fopen("./result.txt", "w")) == NULL ){
		printf("FAILED IN fopen");
		printf("\n");
		return;
	}

	printf("==========================================");
	printf("\n");
	printf("Print Record Table");
	printf("\n");
	fprintf(fp,"delete from model_counting_table(alias);\n");

	printf("==========================================");
	printf("\n");
	

	printf("model total count = %d", pstMC->uiModelCnt);
	printf("\n");
	

	printf("No\tModel\tCount");
	printf("\n");
	printf("==========================================");

	printf("\n");
	for( i = 0; i< pstMC->uiModelCnt; i++ ){
		pstModel = &pstMC->stModel[i];
		printf("%d\t%s\t%d", i+1, pstModel->szModel, pstModel->uiCount);
		printf("\n");
		fprintf(fp,"insert into %s values ('%s','%s',%d);\n", szTable, "20100930", pstModel->szModel, pstModel->uiCount);
		
	}
	
	printf("==========================================");
	printf("\n");
	printf("Complete");
	printf("\n");
	fprintf(fp,"commit;\n");
	if( fp != NULL )
		fclose(fp);
	return;
}

int dInit_CTNInfo(void)
{
	FILE		*fp;
	time_t		tCurr;
	struct tm	stCurr;
	int			dRet,i;
	char		szBuf[1024], sYYYYMMDD[9], szFilePath[PATH_MAX], szModel[MY_MODEL_NAME_LEN], szMIN[MY_MIN_LEN], szIMSI[MY_MIN_LEN];
	st_Model_Cnt stModelCnt, *pstModelCnt;
	

	/* date calculate */
	tCurr = time(NULL);
	tCurr -= SEC_OF_DAY;
	if(localtime_r( (const time_t*)&tCurr, &stCurr) == NULL){
		printf("%s:%s:%d] FAILED IN localtime_r(tCurr[%lu]) errno[%d-%s]", __FILE__, __FUNCTION__, __LINE__,
                tCurr, errno, strerror(errno));
		printf("\n");
		return -1;
	}
	
	if( (dRet = strftime(sYYYYMMDD, 9, "%Y%m%d", &stCurr)) != 8)
    {
        printf("%s:%s:%d] FAILED IN strftime(sYYYYMMDD[%s]) errno[%d-%s]", __FILE__, __FUNCTION__, __LINE__,
                sYYYYMMDD, errno, strerror(errno));
		printf("\n");
        return -2;
    }

	/* set data file name */
	sprintf(szFilePath, "%s/DQMS1_FUSR_ID0000_T%s000000.DAT",MY_START_PATH, sYYYYMMDD);
printf("fileFullPath=%s\n", szFilePath);

	if( (fp = fopen(szFilePath, "r")) == NULL ){
		printf("%s:%s:%d] FAILED IN fopen(szFilePath[%s]) errno[%d-%s]", __FILE__, __FUNCTION__, __LINE__, 
				szFilePath, errno, strerror(errno));
		printf("\n");
		return -3;
	}

	memset(&stModelCnt, 0x00, sizeof(st_Model_Cnt));
	pstModelCnt = &stModelCnt;

	i = 0;
	while(fgets(szBuf, 1024, fp) != NULL){

		if( ( dRet = sscanf(szBuf, "%s %s %s", szMIN, szIMSI, szModel) )  == 3 ){
#if 0
			printf("read(%d) MIN=%s(%d), IMSI=%s(%d), MODEL=%s(%d)", 
					i, szMIN, (int)strlen(szMIN), szIMSI, (int)strlen(szIMSI), szModel, (int)strlen(szModel));
			printf("\n");
#endif

			dRecordModelCnt(pstModelCnt, &szModel[0], strlen(szModel));
			
		}

		
	}

	if( fp != NULL ){
		if( (dRet = fclose(fp)) != 0 ){
			printf("%s:%s:%d] FAILED IN fclose(%s) errno[%d-%s]", __FILE__, __FUNCTION__, __LINE__,
            		szFilePath, errno, strerror(errno));
			printf("\n");
			return -4;
		}
	}

	printRecordStruct(pstModelCnt);
	
	return 0;
}
