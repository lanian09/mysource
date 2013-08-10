/**A.1*  File Inclusion *******************************************************/                       
#include <stdio.h>     /* memset */
#include <string.h>    /* memset */
#include <stdlib.h>    /* memset */
#include <errno.h>     /* errno  */

#include "filelib.h"

/**B.1* DEFINITION OF NEW CONSTANTS *******************************************/                       
/**B.2* DEFINITION OF NEW TYPE ************************************************/                       
/**C.1* DECLARATION OF VARIABLES **********************************************/                       
/**C.2* DECLARATION OF FUNCTIONS **********************************************/

/* 특정 Message Queue Key 값 */
int get_que_key(char *szFileName, char *szBlockName) 
{
	int     dScanCount, dKey;
	char    szBuf[BUFLEN], szBlock[PRC_NAME_LEN], szCmd[BUFLEN], szVerYn[BUFLEN], szQueKey[BUFLEN];
	FILE    *fp;

	if( (fp = fopen(szFileName, "r")) == NULL)
	{
		return E_FILE_GENERIC;
	}

	while(fgets(szBuf, BUFLEN, fp) != NULL)
	{
		/*  from Source to Target: sscanf   */
		if(szBuf[0] != '#')
		{
			fclose(fp);
			return E_FILE_TYPE;
		}
		else if(szBuf[1] == '#')
			continue;
		else if(szBuf[1] == 'E')
			break;
		else if(szBuf[1] == '@')
		{
			if((dScanCount = sscanf(&szBuf[2], "%s %s %s %s", szBlock, szCmd, szVerYn, szQueKey)) == 4) 
			{
				if(!strcmp(szBlockName, szBlock))
				{
					dKey = strtol(szQueKey,0,0);
	                fclose(fp);
					return dKey;
				}
			}
			else 
			{
			    fclose(fp);
				return E_FILE_FUNC;
			}
		}
	}

	fclose(fp);
	return E_FILE_EXIST;
}

/* 특정 Shared Memory key값 */
int get_shm_key(char *szFileName, char *szShmName)
{
	int dScanCount, dShmKey;
	char szBuf[BUFLEN], szShmKey[BUFLEN], szShm[BUFLEN]; 
	FILE *fp;

	if((fp = fopen(szFileName, "r")) == NULL)
		return E_FILE_GENERIC;
	while(fgets(szBuf, BUFLEN, fp) != NULL)
	{
		if(szBuf[0] != '#')
		{
			fclose(fp);
			return E_FILE_TYPE;
		}
		else if(szBuf[1] == '#')
			continue;
		else if(szBuf[1] == 'E')
			break;
		else if(szBuf[1] == '@')
		{
			if((dScanCount = sscanf(&szBuf[2], "%s", szShmKey)) == 1)
			{
				if(!strcmp(szShmName, szShm))
				{
					dShmKey = strtol(szShmKey,0,0);
					fclose(fp);
					return dShmKey;
				}
			}
			else
			{
				fclose(fp);
				return E_FILE_FUNC;
			}
		}
	}
	fclose(fp);
	return E_FILE_EXIST; 
}

/* 특정 Semaphore key값 */
int get_sema_key(char *szFileName, char *szSemName)
{
	int dScanCount, dSemKey;
	char szBuf[BUFLEN], szSemKey[BUFLEN], szSem[BUFLEN]; 
	FILE *fp;

	if((fp = fopen(szFileName, "r")) == NULL)
		return E_FILE_GENERIC;
	while(fgets(szBuf, BUFLEN, fp) != NULL)
	{
		if(szBuf[0] != '#')
		{
			fclose(fp);
			return E_FILE_TYPE;
		}
		else if(szBuf[1] == '#')
			continue;
		else if(szBuf[1] == 'E')
			break;
		else if(szBuf[1] == '@')
		{
			if((dScanCount = sscanf(&szBuf[2], "%s", szSemKey)) == 1)
			{
				if(!strcmp(szSemName, szSem))
				{
					dSemKey = strtol(szSemKey,0,0);
					fclose(fp);
					return dSemKey;
				}
			}
			else
			{
				fclose(fp);
				return E_FILE_FUNC;
			}
		}
	}
	fclose(fp);
	return E_FILE_EXIST; 
}

/* GET MESSAGE QUEUE INFO */
int get_que_info(char *szFileName, st_QUEUE_INFO *pQueInfo )
{
	int dScanCount = 0;
	char szBuf[BUFLEN], szQueName[BUFLEN], szQueKey[BUFLEN], szNifo[BUFLEN]; 
	FILE *fp;

	pQueInfo->dQueCnt = 0;
	pQueInfo->dNifoQueCnt = 0;

	if((fp = fopen(szFileName, "r")) == NULL)
		return E_FILE_GENERIC;
	while(fgets(szBuf, BUFLEN, fp) != NULL)
	{
		if(szBuf[0] != '#')
		{
			fclose(fp);
			return E_FILE_TYPE;
		}
		else if(szBuf[1] == '#')
			continue;
		else if(szBuf[1] == 'E')
			break;
		else if(szBuf[1] == '@')
		{
			if((dScanCount = sscanf(&szBuf[2], "%*s %*s %*s %s %s %s", szQueName, szQueKey, szNifo)) == 3)
			{
				//MESSAGE QUEUE NAME
				sprintf(*((pQueInfo->sQueName)+pQueInfo->dQueCnt), "%s", szQueName); 
				//MESSAGE QUEUE KEY
				*((pQueInfo->dQueList)+pQueInfo->dQueCnt) = strtol(szQueKey,0,0); 
				//MESSAGE QUEUE COUNT
				pQueInfo->dQueCnt++; 
				//NIFO MESSAGE QUEUE INFO
				if(!strcasecmp(szNifo,"Y"))
				{
					sprintf(*((pQueInfo->sNifoQueName)+pQueInfo->dNifoQueCnt), "%s", szQueName);
					*((pQueInfo->dNifoQueList)+pQueInfo->dNifoQueCnt) = strtol(szQueKey,0,0);
					pQueInfo->dNifoQueCnt++;
				}
			}
			else
			{
			    fclose(fp);
				return E_FILE_FUNC;
			}
		}
	}
	fclose(fp);
	return 1;
}

/* GET SHARED MEMORY INFO */
int get_shm_info(char *szFileName, st_SHM_INFO *pShmInfo )
{
	int dScanCount = 0;
	char szBuf[BUFLEN], szShmName[BUFLEN], szShmKey[BUFLEN]; 
	FILE *fp;

	pShmInfo->dShmCnt = 0;

	if((fp = fopen(szFileName, "r")) == NULL)
		return E_FILE_GENERIC;
	while(fgets(szBuf, BUFLEN, fp) != NULL)
	{
		if(szBuf[0] != '#')
		{
			fclose(fp);
			return E_FILE_TYPE;
		}
		else if(szBuf[1] == '#')
			continue;
		else if(szBuf[1] == 'E')
			break;
		else if(szBuf[1] == '@')
		{
			if((dScanCount = sscanf(&szBuf[2], "%s %s", szShmName, szShmKey)) == 2)
			{
				//SHARED MEMORY NAME
				sprintf(*((pShmInfo->sShmName)+pShmInfo->dShmCnt), "%s", szShmName); 
				//SHARED MEMORY KEY
				*((pShmInfo->dShmList)+pShmInfo->dShmCnt) = strtol(szShmKey,0,0); 
				//SHARED MEMORY COUNT
				pShmInfo->dShmCnt++; 
			}
			else
			{
			    fclose(fp);
				return E_FILE_FUNC;
			}
		}
	}
	fclose(fp);
	return 1;
}

/* GET SEMAPHORE INFO */
int get_sema_info(char *szFileName, st_SEM_INFO *pSemInfo )
{
	int dScanCount = 0;
	char szBuf[BUFLEN], szSemName[BUFLEN], szSemKey[BUFLEN]; 
	FILE *fp;

	pSemInfo->dSemCnt = 0;

	if((fp = fopen(szFileName, "r")) == NULL)
		return E_FILE_GENERIC;
	while(fgets(szBuf, BUFLEN, fp) != NULL)
	{
		if(szBuf[0] != '#')
		{
			fclose(fp);
			return E_FILE_TYPE;
		}
		else if(szBuf[1] == '#')
			continue;
		else if(szBuf[1] == 'E')
			break;
		else if(szBuf[1] == '@')
		{
			if((dScanCount = sscanf(&szBuf[2], "%s %s", szSemName, szSemKey)) == 2)
			{
				//SEMA NAME
				sprintf(*((pSemInfo->sSemName)+pSemInfo->dSemCnt), "%s", szSemName); 
				//SEMA KEY
				*((pSemInfo->dSemList)+pSemInfo->dSemCnt) = strtol(szSemKey,0,0); 
				//SEMA COUNT
				pSemInfo->dSemCnt++;
			}
			else
			{
			    fclose(fp);
				return E_FILE_FUNC;
			}
		}
	}
	fclose(fp);
	return 1;
}

int get_nifo_useyn(char *szFileName, char *szBlockName, char *szNifoYn)
{
	int     dScanCount;
	char    szBuf[BUFLEN], szBlock[PRC_NAME_LEN], szCmd[BUFLEN], szVerYn[BUFLEN], szQueKey[BUFLEN], szNifo[BUFLEN];
	FILE *fp;

	if( (fp = fopen(szFileName, "r")) == NULL)
	{
		return E_FILE_GENERIC;
	}

	while(fgets(szBuf, BUFLEN, fp) != NULL)
	{
		/*  from Source to Target: sscanf   */
		if(szBuf[0] != '#')
		{
			fclose(fp);
			return E_FILE_TYPE;
		}
		else if(szBuf[1] == '#')
			continue;
		else if(szBuf[1] == 'E')
			break;
		else if(szBuf[1] == '@')
		{
			if((dScanCount = sscanf(&szBuf[2], "%s %s %s %s %s", szBlock, szCmd, szVerYn, szQueKey, szNifo)) == 4) 
			{
				if(!strcmp(szBlockName, szBlock)) {
					strcpy(szNifoYn, szNifo);
					fclose(fp);
					return 1;
				}
			}
			else 
			{
				fclose(fp);
				return E_FILE_FUNC;
			}
		}
	}

	fclose(fp);
	return E_FILE_EXIST;
}

int get_proc_seq(char *szFileName, char *szBlockName)
{
	int     dScanCount, dSeq;
	char    szBuf[BUFLEN], szBlock[PRC_NAME_LEN], szCmd[BUFLEN], szVerYn[BUFLEN], szQueKey[BUFLEN], szNifoYn[BUFLEN], szProcSeq[BUFLEN];
	FILE    *fp;

	if( (fp = fopen(szFileName, "r")) == NULL)
	{
		return E_FILE_GENERIC;
	}

	while(fgets(szBuf, BUFLEN, fp) != NULL)
	{
		/*  from Source to Target: sscanf   */
		if(szBuf[0] != '#')
		{
			fclose(fp);
			return E_FILE_TYPE;
		}
		else if(szBuf[1] == '#')
			continue;
		else if(szBuf[1] == 'E')
			break;
		else if(szBuf[1] == '@')
		{
			if((dScanCount = sscanf(&szBuf[2], "%s %s %s %s %s %s", szBlock, szCmd, szVerYn, szQueKey, szNifoYn, szProcSeq)) == 4) 
			{
				if(!strcmp(szBlockName, szBlock)) {
					dSeq = atoi(szProcSeq);
					fclose(fp);
					return dSeq;
				}

			}
			else 
			{
				fclose(fp);
				return E_FILE_FUNC;
			}
		}
	}

	fclose(fp);
	return E_FILE_EXIST;
}

int get_block_num(char *szFileName, char *szBlockName)
{
	int     rdCnt, dProcLen;
	char    szBuf[BUFLEN], szBlock[PRC_NAME_LEN];
	FILE    *fp;

	dProcLen = strlen(szBlockName);
	if( (fp = fopen(szFileName, "r")) == NULL) {
		return E_FILE_GENERIC;
	}

	rdCnt = 0;
	while(fgets(szBuf, BUFLEN, fp) != NULL) {
		/*  from Source to Target: sscanf   */
		if(szBuf[0] != '#') {
			fclose(fp);
			return E_FILE_TYPE;

		} else if(szBuf[1] == '@') {
			if( sscanf(&szBuf[2], "%s %*s %*s %*s %*s %*s", szBlock) == 1 ) {
				if(!strncmp(szBlockName, szBlock, dProcLen)) {
					rdCnt++;
				}
			}

		} else if(szBuf[1] == '#') {
			continue;
		} else if(szBuf[1] == 'E') {
			break;
		} else {
			fclose(fp);
			return E_FILE_TYPE;
		}
	}

	fclose(fp);
	return rdCnt;

}
