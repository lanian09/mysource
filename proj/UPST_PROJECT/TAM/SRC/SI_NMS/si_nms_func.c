/*******************************************************************************
			DQMS Project

	Author   : Jae Seung Lee
	Section  : SI_NMS
	SCCS ID  : @(#)si_nms_func.c	1.1
	Date     : 01/21/05
	Revision History :
		'05. 01. 21		Initial
		'08. 01. 07		Update By LSH for review

	Description :

	Copyright (c) uPRESTO 2005
*******************************************************************************/

/** A.1 * File Include ************************************/
#include <stdio.h>			/*	fopen(3), fgets(3), fclose(3), sprintf(3)	*/
#include <string.h>			/*	strlen(3), strerror(3)		*/
#include <stdlib.h>			/*	atoi(3), atof(3)			*/
#include <errno.h>			/*	errno(3)					*/
#include <ctype.h>			/*	isspace(3)					*/
#include <sys/stat.h>		/*	fchmod(2)					*/
#include <mysql/mysql.h>	/*	MYSQL						*/
#include <unistd.h>			/*	usleep()					*/

// LIB
#include "mems.h"
#include "gifo.h"
#include "cifo.h"

// OAM
#include "msgdef.h"			/* pst_MsgQ */
#include "path.h"
#include "filedb.h"			/* MAX_NTAF_COUNT */
#include "loglib.h"
#include "almstat.h"		/* LOCTYPE_XXXX */

// DQMS
#include "procid.h"
#include "timesec.h"

// .
#include "si_nms_comm.h"
#include "si_nms_func.h"

/** B.1* DEFINITION OF NEW CONSTANTS **********************/
// Move to si_nms_func.h

/** C.1* DEFINITION OF NEW TYPES **************************/
enum {
	FLAG_NEED_RCV_OK = 0,
	FLAG_NO_NEED_RCV_OK,
	FLAG_NEED_MAX_RCV
};

typedef struct _st_HoldFList
{
	int		dSfd;
	int		dFlag;						/*	FLAG_NEED_RCV_OK, FLAG_NO_NEED_RCV_OK	*/
	time_t	tInitialTime;				/*	initial time							*/
	char	sFileName[FILE_NAME_LEN];	/*	file name								*/
} st_HoldFList;

typedef struct _st_LoadStat
{
	char	cSysType;
	char	cSysID;
	float	fCPUAvg;
	float	fCPUMax;
	float	fCPUMin;
	float	fMEMAvg;
	float	fMEMMax;
	float	fMEMMin;
	float	fQueueAvg;
	float	fQueueMax;
	float	fQueueMin;
	float	fNifoAvg;
	float	fNifoMax;
	float	fNifoMin;
	float	fDisk1Avg;
	float	fDisk1Max;
	float	fDisk1Min;
	float	fDisk2Avg;
	float	fDisk2Max;
	float	fDisk2Min;
	float	fDisk3Avg;
	float	fDisk3Max;
	float	fDisk3Min;
	float	fDisk4Avg;
	float	fDisk4Max;
	float	fDisk4Min;
} st_LoadStat;

#define	FAULTTYPE		40
typedef struct _st_FaultStat
{
	char	cSysType;
	char	cSysID;
	char	sFaultType[FAULTTYPE];
	int		dCritical;
	int		dMajor;
	int		dMinor;
	int		dStop;
	int		dNormal;
} st_FaultStat;

typedef struct _st_TrafficStat
{
	char		cTAFID;

	long long	llThruFrames;
	long long	llThruBytes;
	long long	llTotFrames;
	long long	llTotBytes;

	long long	llIPFrames;
	long long	llIPBytes;
	long long	llUDPFrames;
	long long	llUDPBytes;
	long long	llTCPFrames;
	long long	llTCPBytes;
	long long	llSCTPFrames;
	long long	llSCTPBytes;

	long long	llIPErrorFrames;
	long long	llIPErrorBytes;
	long long	llUTCPFrames;
	long long	llUTCPBytes;
	long long	llFailDataFrames;
	long long	llFailDataBytes;
	long long	llFilterOutFrames;
	long long	llFilterOutBytes;
} st_TrafficStat;


/** D.1* DECLARATION OF VARIABLES *************************/
st_HoldFList	hold_list[FILE_NUM_5MIN+FILE_NUM_HOUR];
pst_NTAM		fidb;

/** D.2* DECLARATION OF VARIABLES *************************/
extern st_NMSPortInfo		gstNMSPortInfo;							/*	SRC/SI_NMS/si_nms_main.c	*/
extern int					gMyNEID;								/*	SRC/SI_NMS/si_nms_init.c	*/
extern st_OidInfo			gstOidInfo[IDX_MAXIMUM_STATISTICS];		/*	SRC/SI_NMS/si_nms_init.c	*/
extern char					*gsLocSys;								/*	SRC/SI_NMS/si_nms_init.c	*/
extern int					gdSysNo;								/*	SRC/SI_NMS/si_nms_init.c	*/

extern stMEMSINFO			*pMEMSINFO;
extern stCIFO				*pCIFO;

/** E.1* DEFINITION OF FUNCTIONS **************************/
// Move to si_nms_func.h

/** E.2* DEFINITION OF FUNCTIONS **************************/


int dGetIPAddr(char *conf_file, char *primary_addr, int dMaxLen)
{
	FILE	*fp;
	size_t	len_str;
	char	szStr[257];

	primary_addr[0] = '\0';

	if( (fp = fopen(conf_file, "r")) == NULL)
	{
		log_print(LOGN_CRI, LH"FAILED fopen(%s) - errno[%d:%s]", LT, conf_file, errno, strerror(errno));
		return -1;
	}

	while(fgets(szStr, 256, fp) != NULL)
	{
		/*	START: remove a white space in szStr	*/
		len_str = strlen(szStr);
		while(isspace(szStr[len_str-1]))
			szStr[--len_str] = 0x00;
		/*	END: Writer: Han-jin Park, Date: 2009.05.12	*/

		if(len_str > dMaxLen)
		{
			log_print(LOGN_CRI, LH"Check %s contents - address is too long[MAX SIZE: %u bytes]", LT, conf_file, dMaxLen);
			continue;
		}

		if( (szStr[0] == '#') || (szStr[0] == '/'))
			continue;

		if(dValidIP(szStr) < 0)
			continue;

		sprintf(primary_addr, "%s", szStr);
		if(primary_addr[0] == 0x00)
		{
			fclose(fp);
			log_print(LOGN_CRI, LH"ERROR %s file", LT, conf_file);
			return -1;
		}
		else
			break;
	}
	fclose(fp);

	return 0;
}

int dValidIP(char *IPaddr)
{
	int		d1, d2, d3, d4;

	if( (sscanf(IPaddr, "%d.%d.%d.%d", &d1, &d2, &d3, &d4) != 4) ||
			((d1 < 0) || (d1 > 255) || (d2 < 0) || (d2 > 255) || (d3 < 0) || (d3 > 255) || (d4 < 0) || (d4 > 255)))
	{
		log_print(LOGN_WARN, LH"IP address(%s) is not valid", LT, IPaddr);
		return -1;
	}

	return 0;
}

int dIsReceivedMessage(pst_MsgQ pstMsgQ)
{
	OFFSET		offset;

	offset = gifo_read(pMEMSINFO, pCIFO, SEQ_PROC_SI_NMS);
	if(offset <= 0)
	{
		usleep(0);
		return -1;		
	}
	pstMsgQ = (pst_MsgQ)nifo_get_value(pMEMSINFO, DEF_MSGQ_NUM, offset);

	return 0;
}

int dSaveFileList(char *FileName, int dSfd, int dPeriod)
{
	int		i;

	if(dPeriod == STAT_PERIOD_5MIN)
	{
		for(i = 0; i < FILE_NUM_5MIN; i++)
		{
			if(hold_list[i].dFlag == 0)
			{
				if(dSfd > 0)
				{
					hold_list[i].dSfd 	= dSfd;
					hold_list[i].dFlag	= FLAG_NEED_RCV_OK;
				}
				else
				{
					hold_list[i].dSfd 	= 0;
					hold_list[i].dFlag	= FLAG_NO_NEED_RCV_OK;
				}
				hold_list[i].tInitialTime = time(NULL);
				strcpy(hold_list[i].sFileName, FileName);

				log_print(LOGN_INFO, LH"Insert FileName[%s] dSfd[%d] Index[%d]", LT, FileName, dSfd, i);
				break;
			}
		}

		if(i == FILE_NUM_5MIN)
			log_print(LOGN_CRI, LH"Can't insert FileName[%s]", LT, FileName);
	}
	else
	{
		for(i = FILE_NUM_5MIN; i < (FILE_NUM_5MIN+FILE_NUM_HOUR); i++)
		{
			if(hold_list[i].dFlag == 0)
			{
				if(dSfd > 0)
				{
					hold_list[i].dSfd 	= dSfd;
					hold_list[i].dFlag 	= FLAG_NEED_RCV_OK;
				}
				else
				{
					hold_list[i].dSfd 	= 0;
					hold_list[i].dFlag	= FLAG_NO_NEED_RCV_OK;
				}
				strcpy(hold_list[i].sFileName, FileName);
				hold_list[i].tInitialTime = time(NULL);

				log_print(LOGN_INFO, LH"Insert FileName[%s] dSfd[%d] Index[%d]", LT, FileName, dSfd, i);
				break;
			}
		}

		if(i == (FILE_NUM_5MIN+FILE_NUM_HOUR))
			log_print(LOGN_DEBUG, LH"Can't insert FileName[%s]", LT, FileName);
	}
	DisplayFileList();

	return 1;
}

void DisplayFileList(void)
{
	int		i;

	for(i = 0; i < (FILE_NUM_5MIN+FILE_NUM_HOUR); i++)
	{
		if(strlen(hold_list[i].sFileName) > 0)
			log_print(LOGN_INFO, "[%02d] dSfd[%d] FileName[%s] dFlag[%d]", i, hold_list[i].dSfd, hold_list[i].sFileName, hold_list[i].dFlag);
	}
	log_print(LOGN_INFO, "______________________________________________________");
}

int dMakeOIDFile(MYSQL *pstMySQL, st_atQueryInfo *pstAtQueryInfo, char *sFileName)
{
	int				i, j, dCount, dRet;
	char			sFullPath[PATH_MAX], sQuery[1024], sTableName[MAX_BUF_SIZE], sWriteBuf[MAX_BUF_SIZE], sOidTime[MAX_BUF_SIZE];
	struct tm		*ntm;
	st_LoadStat		stLoadStat[EQUIP_MAX_COUNT];
	st_FaultStat	stFaultStat[EQUIP_MAX_COUNT];
	st_TrafficStat	stTrafficStat[EQUIP_MAX_COUNT];
	FILE			*ofp;
	size_t			szWriteLen;
	MYSQL_ROW		stRow;
	MYSQL_RES		*pstRst;

	ntm = (struct tm*)stTmNow();
	memset(sFullPath, 0x00, PATH_MAX);
	switch(pstAtQueryInfo->cPeriod)
	{
		case STAT_PERIOD_5MIN:
#ifdef _ENABLE_CONFIGURATION_LOADED_FILENAME_ /* changed by uamdy 20101029 : NMS linked file-naming rule change, PN_CR_VERSION */
			sprintf(sFileName, "%s-M-%04d%02d%02d%02d%02d", gsLocSys, ntm->tm_year+1900, ntm->tm_mon+1, ntm->tm_mday, ntm->tm_hour, dGetABSMinute(ntm->tm_min));
#else
			sprintf(sFileName, "70DQM01-%s%d-M-%04d%02d%02d%02d%02d", gsLocSys, gdSysNo, ntm->tm_year+1900, ntm->tm_mon+1, ntm->tm_mday, ntm->tm_hour, dGetABSMinute(ntm->tm_min));
#endif /* _ENABLE_CONFIGURATION_LOADED_FILENAME_ */
			sprintf(sFullPath, "%s/%s/5MIN/%s", START_PATH, NMS_STATISTICS_DIR, sFileName);
			break;
		case STAT_PERIOD_HOUR:
#ifdef _ENABLE_CONFIGURATION_LOADED_FILENAME_ /* changed by uamdy 20101029 : NMS linked file-naming rule change, PN_CR_VERSION */
			sprintf(sFileName, "%s-A-%04d%02d%02d%02d", gsLocSys, ntm->tm_year+1900, ntm->tm_mon+1, ntm->tm_mday, ntm->tm_hour);
#else
			sprintf(sFileName, "70DQM01-%s%d-A-%04d%02d%02d%02d", gsLocSys, gdSysNo, ntm->tm_year+1900, ntm->tm_mon+1, ntm->tm_mday, ntm->tm_hour);
#endif /* _ENABLE_CONFIGURATION_LOADED_FILENAME_ */
			sprintf(sFullPath, "%s/%s/HOUR/%s", START_PATH, NMS_STATISTICS_DIR, sFileName);
			break;
		default:
			log_print(LOGN_CRI, LH"Unknown period[%d]", LT, pstAtQueryInfo->cPeriod);
			return -1;
	}

	/*	~/LOG/NMS_DIR/5MIN/ or ~/.../1HOUR/ 폴더에 통계 전송화일을 생성한다.	*/
	if( (ofp = fopen(sFullPath, "w")) == NULL)
	{
		log_print(LOGN_CRI, LH"FAILED IN fopen(%s) [errno:%d-%s]", LT, sFullPath, errno, strerror(errno));
		return -2;
	}
	else if( (dRet = chmod(sFullPath, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)) == -1)
		log_print(LOGN_CRI, LH"FAILED IN chmod(%s) [errno:%d-%s]", LT, sFullPath, errno, strerror(errno));


	memset(sOidTime, 0x00, MAX_BUF_SIZE);
	if( (dRet = dGetOidTime(pstAtQueryInfo->cPeriod, sOidTime)) < 0)
		log_print(LOGN_CRI, LH"ERROR IN dGetOidTime() dRet[%d]", LT, dRet);

	for(i = 0; i < IDX_MAXIMUM_STATISTICS; i++)
	{
		switch(i)
		{
			case IDX_LOAD_STATISTICS:
				memset(sTableName, 0x00, MAX_BUF_SIZE);
				sprintf(sTableName, "%s", gstOidInfo[i].sTableName);
				sprintf(sQuery,
					"SELECT "
						"SYSTEMTYPE,SYSTEMID,"
						"IFNULL(TRUNCATE(AVG(CPUAVG)/100,2),0),IFNULL(TRUNCATE(MAX(CPUMAX)/100,2),0),IFNULL(TRUNCATE(MIN(CPUMIN)/100,2),0),"
						"IFNULL(TRUNCATE(AVG(MEMAVG)/100,2),0),IFNULL(TRUNCATE(MAX(MEMMAX)/100,2),0),IFNULL(TRUNCATE(MIN(MEMMIN)/100,2),0),"
						"IFNULL(TRUNCATE(AVG(QUEAVG)/100,2),0),IFNULL(TRUNCATE(MAX(QUEMAX)/100,2),0),IFNULL(TRUNCATE(MIN(QUEMIN)/100,2),0),"
						"IFNULL(TRUNCATE(AVG(NIFOAVG)/100,2),0),IFNULL(TRUNCATE(MAX(NIFOMAX)/100,2),0),IFNULL(TRUNCATE(MIN(NIFOMIN)/100,2),0),"
						"IFNULL(TRUNCATE(AVG(DISK1AVG)/100,2),0),IFNULL(TRUNCATE(MAX(DISK1MAX)/100,2),0),IFNULL(TRUNCATE(MIN(DISK1MIN)/100,2),0),"
						"IFNULL(TRUNCATE(AVG(DISK2AVG)/100,2),0),IFNULL(TRUNCATE(MAX(DISK2MAX)/100,2),0),IFNULL(TRUNCATE(MIN(DISK2MIN)/100,2),0),"
						"IFNULL(TRUNCATE(AVG(DISK3AVG)/100,2),0),IFNULL(TRUNCATE(MAX(DISK3MAX)/100,2),0),IFNULL(TRUNCATE(MIN(DISK3MIN)/100,2),0),"
						"IFNULL(TRUNCATE(AVG(DISK4AVG)/100,2),0),IFNULL(TRUNCATE(MAX(DISK4MAX)/100,2),0),IFNULL(TRUNCATE(MIN(DISK4MIN)/100,2),0) "
					"FROM "
						"%s "
					"WHERE "
						"STATTIME>=%lu AND STATTIME<%lu "
					"GROUP BY "
						"SYSTEMTYPE, SYSTEMID "
					"ORDER BY "
						"SYSTEMTYPE, SYSTEMID",
					sTableName, pstAtQueryInfo->tStartTime, pstAtQueryInfo->tEndTime);

				if(mysql_query(pstMySQL, sQuery) != 0)
				{
					log_print(LOGN_CRI, LH"FAILED IN mysql_query(%s) mysql_errno[%d-%s]", LT,
						sQuery, mysql_errno(pstMySQL), mysql_error(pstMySQL));
					fclose(ofp);
					return -3;
				}

				if( (pstRst = mysql_store_result(pstMySQL)) == NULL)
				{
					log_print(LOGN_CRI, LH"FAILED IN mysql_store_result(%s) mysql_errno[%d-%s]", LT,
						sQuery, mysql_errno(pstMySQL), mysql_error(pstMySQL));
					fclose(ofp);
					return -4;
				}

				j	= 0;
				memset(stLoadStat, 0x00, sizeof(st_LoadStat)*EQUIP_MAX_COUNT);
				while( (stRow = mysql_fetch_row(pstRst)) != NULL)
				{
					if(j > EQUIP_MAX_COUNT)
					{
						log_print(LOGN_CRI, LH"Count[%d] is over EQUIP_MAX_COUNT[%d]", LT, j, EQUIP_MAX_COUNT);
						fclose(ofp);
						return -5;
					}

					stLoadStat[j].cSysType		= (char)atoi(stRow[0]);
					stLoadStat[j].cSysID		= (char)atoi(stRow[1]);
					stLoadStat[j].fCPUAvg		= (float)atof(stRow[2]);
					stLoadStat[j].fCPUMax		= (float)atof(stRow[3]);
					stLoadStat[j].fCPUMin		= (float)atof(stRow[4]);
					stLoadStat[j].fMEMAvg		= (float)atof(stRow[5]);
					stLoadStat[j].fMEMMax		= (float)atof(stRow[6]);
					stLoadStat[j].fMEMMin		= (float)atof(stRow[7]);
					stLoadStat[j].fQueueAvg		= (float)atof(stRow[8]);
					stLoadStat[j].fQueueMax		= (float)atof(stRow[9]);
					stLoadStat[j].fQueueMin		= (float)atof(stRow[10]);
					stLoadStat[j].fNifoAvg		= (float)atof(stRow[11]);
					stLoadStat[j].fNifoMax		= (float)atof(stRow[12]);
					stLoadStat[j].fNifoMin		= (float)atof(stRow[13]);
					stLoadStat[j].fDisk1Avg		= (float)atof(stRow[14]);
					stLoadStat[j].fDisk1Max		= (float)atof(stRow[15]);
					stLoadStat[j].fDisk1Min		= (float)atof(stRow[16]);
					stLoadStat[j].fDisk2Avg		= (float)atof(stRow[17]);
					stLoadStat[j].fDisk2Max		= (float)atof(stRow[18]);
					stLoadStat[j].fDisk2Min		= (float)atof(stRow[19]);
					stLoadStat[j].fDisk3Avg		= (float)atof(stRow[20]);
					stLoadStat[j].fDisk3Max		= (float)atof(stRow[21]);
					stLoadStat[j].fDisk3Min		= (float)atof(stRow[22]);
					stLoadStat[j].fDisk4Avg		= (float)atof(stRow[23]);
					stLoadStat[j].fDisk4Max		= (float)atof(stRow[24]);
					stLoadStat[j].fDisk4Min		= (float)atof(stRow[25]);
					j++;
				}
				mysql_free_result(pstRst);
				dCount	= j;

				for(j = 0; j < dCount; j++)
				{
					memset(sWriteBuf, 0x00, MAX_BUF_SIZE);
					szWriteLen = 0;

					sprintf(&sWriteBuf[szWriteLen], "REPLACE\n");
					szWriteLen = strlen(sWriteBuf);

					sprintf(&sWriteBuf[szWriteLen], "%d:I:%d:\n", gMyNEID, gstOidInfo[i].dObjectID);
					szWriteLen = strlen(sWriteBuf);

					sprintf(&sWriteBuf[szWriteLen], "%d:I:%u:\n", gstOidInfo[i].dSidFirstNum, stLoadStat[j].cSysType);
					szWriteLen = strlen(sWriteBuf);
					sprintf(&sWriteBuf[szWriteLen], "%d:I:%u:\n", gstOidInfo[i].dSidFirstNum+1, stLoadStat[j].cSysID);
					szWriteLen = strlen(sWriteBuf);
					sprintf(&sWriteBuf[szWriteLen], "%d:I:%.2f:\n", gstOidInfo[i].dSidFirstNum+2, stLoadStat[j].fCPUAvg);
					szWriteLen = strlen(sWriteBuf);
					sprintf(&sWriteBuf[szWriteLen], "%d:I:%.2f:\n", gstOidInfo[i].dSidFirstNum+3, stLoadStat[j].fCPUMax);
					szWriteLen = strlen(sWriteBuf);
					sprintf(&sWriteBuf[szWriteLen], "%d:I:%.2f:\n", gstOidInfo[i].dSidFirstNum+4, stLoadStat[j].fCPUMin);
					szWriteLen = strlen(sWriteBuf);
					sprintf(&sWriteBuf[szWriteLen], "%d:I:%.2f:\n", gstOidInfo[i].dSidFirstNum+5, stLoadStat[j].fMEMAvg);
					szWriteLen = strlen(sWriteBuf);
					sprintf(&sWriteBuf[szWriteLen], "%d:I:%.2f:\n", gstOidInfo[i].dSidFirstNum+6, stLoadStat[j].fMEMMax);
					szWriteLen = strlen(sWriteBuf);
					sprintf(&sWriteBuf[szWriteLen], "%d:I:%.2f:\n", gstOidInfo[i].dSidFirstNum+7, stLoadStat[j].fMEMMin);
					szWriteLen = strlen(sWriteBuf);
					sprintf(&sWriteBuf[szWriteLen], "%d:I:%.2f:\n", gstOidInfo[i].dSidFirstNum+8, stLoadStat[j].fQueueAvg);
					szWriteLen = strlen(sWriteBuf);
					sprintf(&sWriteBuf[szWriteLen], "%d:I:%.2f:\n", gstOidInfo[i].dSidFirstNum+9, stLoadStat[j].fQueueMax);
					szWriteLen = strlen(sWriteBuf);
					sprintf(&sWriteBuf[szWriteLen], "%d:I:%.2f:\n", gstOidInfo[i].dSidFirstNum+10, stLoadStat[j].fQueueMin);
					szWriteLen = strlen(sWriteBuf);
					sprintf(&sWriteBuf[szWriteLen], "%d:I:%.2f:\n", gstOidInfo[i].dSidFirstNum+11, stLoadStat[j].fNifoAvg);
					szWriteLen = strlen(sWriteBuf);
					sprintf(&sWriteBuf[szWriteLen], "%d:I:%.2f:\n", gstOidInfo[i].dSidFirstNum+12, stLoadStat[j].fNifoMax);
					szWriteLen = strlen(sWriteBuf);
					sprintf(&sWriteBuf[szWriteLen], "%d:I:%.2f:\n", gstOidInfo[i].dSidFirstNum+13, stLoadStat[j].fNifoMin);
					szWriteLen = strlen(sWriteBuf);
					sprintf(&sWriteBuf[szWriteLen], "%d:I:%.2f:\n", gstOidInfo[i].dSidFirstNum+14, stLoadStat[j].fDisk1Avg);
					szWriteLen = strlen(sWriteBuf);
					sprintf(&sWriteBuf[szWriteLen], "%d:I:%.2f:\n", gstOidInfo[i].dSidFirstNum+15, stLoadStat[j].fDisk1Max);
					szWriteLen = strlen(sWriteBuf);
					sprintf(&sWriteBuf[szWriteLen], "%d:I:%.2f:\n", gstOidInfo[i].dSidFirstNum+16, stLoadStat[j].fDisk1Min);
					szWriteLen = strlen(sWriteBuf);
					sprintf(&sWriteBuf[szWriteLen], "%d:I:%.2f:\n", gstOidInfo[i].dSidFirstNum+17, stLoadStat[j].fDisk2Avg);
					szWriteLen = strlen(sWriteBuf);
					sprintf(&sWriteBuf[szWriteLen], "%d:I:%.2f:\n", gstOidInfo[i].dSidFirstNum+18, stLoadStat[j].fDisk2Max);
					szWriteLen = strlen(sWriteBuf);
					sprintf(&sWriteBuf[szWriteLen], "%d:I:%.2f:\n", gstOidInfo[i].dSidFirstNum+19, stLoadStat[j].fDisk2Min);
					szWriteLen = strlen(sWriteBuf);
					sprintf(&sWriteBuf[szWriteLen], "%d:I:%.2f:\n", gstOidInfo[i].dSidFirstNum+20, stLoadStat[j].fDisk3Avg);
					szWriteLen = strlen(sWriteBuf);
					sprintf(&sWriteBuf[szWriteLen], "%d:I:%.2f:\n", gstOidInfo[i].dSidFirstNum+21, stLoadStat[j].fDisk3Max);
					szWriteLen = strlen(sWriteBuf);
					sprintf(&sWriteBuf[szWriteLen], "%d:I:%.2f:\n", gstOidInfo[i].dSidFirstNum+22, stLoadStat[j].fDisk3Min);
					szWriteLen = strlen(sWriteBuf);
					sprintf(&sWriteBuf[szWriteLen], "%d:I:%.2f:\n", gstOidInfo[i].dSidFirstNum+23, stLoadStat[j].fDisk4Avg);
					szWriteLen = strlen(sWriteBuf);
					sprintf(&sWriteBuf[szWriteLen], "%d:I:%.2f:\n", gstOidInfo[i].dSidFirstNum+24, stLoadStat[j].fDisk4Max);
					szWriteLen = strlen(sWriteBuf);
					sprintf(&sWriteBuf[szWriteLen], "%d:I:%.2f:\n", gstOidInfo[i].dSidFirstNum+25, stLoadStat[j].fDisk4Min);
					szWriteLen = strlen(sWriteBuf);

					sprintf(&sWriteBuf[szWriteLen], "10043:C:%s:\n", sOidTime);

					fprintf(ofp, "%s\n", sWriteBuf);
					fflush(ofp);
				}
				break;
			case IDX_FAULT_STATISTICS:
				memset(sTableName, 0x00, MAX_BUF_SIZE);
				sprintf(sTableName, "%s", gstOidInfo[i].sTableName);
				sprintf(sQuery,
					"SELECT "
						"SYSTEMTYPE,SYSTEMID,FAULTTYPE,IFNULL(SUM(CRI),0),IFNULL(SUM(MAJ),0),IFNULL(SUM(MIN),0),IFNULL(SUM(STOP),0),IFNULL(SUM(NORMAL),0) "
					"FROM "
						"%s "
					"WHERE "
						"STATTIME>=%lu AND STATTIME<%lu "
					"GROUP BY "
						"SYSTEMTYPE,SYSTEMID,FAULTTYPE "
					"ORDER BY "
						"SYSTEMTYPE,SYSTEMID",
					sTableName, pstAtQueryInfo->tStartTime, pstAtQueryInfo->tEndTime);

				if(mysql_query(pstMySQL, sQuery) != 0)
				{
					log_print(LOGN_CRI, LH"FAILED IN mysql_query(%s) mysql_errno[%d-%s]", LT,
						sQuery, mysql_errno(pstMySQL), mysql_error(pstMySQL));
					fclose(ofp);
					return -6;
				}

				if( (pstRst = mysql_store_result(pstMySQL)) == NULL)
				{
					log_print(LOGN_CRI, LH"FAILED IN mysql_store_result(%s) mysql_errno[%d-%s]", LT,
						sQuery, mysql_errno(pstMySQL), mysql_error(pstMySQL));
					fclose(ofp);
					return -7;
				}

				j	= 0;
				memset(stFaultStat, 0x00, sizeof(st_FaultStat)*EQUIP_MAX_COUNT);
				while( (stRow = mysql_fetch_row(pstRst)) != NULL)
				{
					if(j > EQUIP_MAX_COUNT)
					{
						log_print(LOGN_CRI, LH"Count[%d] is over EQUIP_MAX_COUNT[%d]", LT, j, EQUIP_MAX_COUNT);
						fclose(ofp);
						return -8;
					}

					stFaultStat[j].cSysType		= (char)atoi(stRow[0]);
					stFaultStat[j].cSysID		= (char)atoi(stRow[1]);
					sprintf(stFaultStat[j].sFaultType, "%s", stRow[2]);
					stFaultStat[j].dCritical	= atoi(stRow[3]);
					stFaultStat[j].dMajor		= atoi(stRow[4]);
					stFaultStat[j].dMinor		= atoi(stRow[5]);
					stFaultStat[j].dStop		= atoi(stRow[6]);
					stFaultStat[j].dNormal		= atoi(stRow[7]);
					j++;
				}
				mysql_free_result(pstRst);
				dCount	= j;

				for(j = 0; j < dCount; j++)
				{
					memset(sWriteBuf, 0x00, MAX_BUF_SIZE);
					szWriteLen = 0;

					sprintf(&sWriteBuf[szWriteLen], "REPLACE\n");
					szWriteLen = strlen(sWriteBuf);

					sprintf(&sWriteBuf[szWriteLen], "%d:I:%d:\n", gMyNEID, gstOidInfo[i].dObjectID);
					szWriteLen = strlen(sWriteBuf);

					sprintf(&sWriteBuf[szWriteLen], "%d:I:%u:\n", gstOidInfo[IDX_LOAD_STATISTICS].dSidFirstNum, stFaultStat[j].cSysType);
					szWriteLen = strlen(sWriteBuf);

					sprintf(&sWriteBuf[szWriteLen], "%d:I:%u:\n", gstOidInfo[IDX_LOAD_STATISTICS].dSidFirstNum+1, stFaultStat[j].cSysID);
					szWriteLen = strlen(sWriteBuf);

					sprintf(&sWriteBuf[szWriteLen], "%d:I:%s:\n", gstOidInfo[i].dSidFirstNum, stFaultStat[j].sFaultType);
					szWriteLen = strlen(sWriteBuf);

					sprintf(&sWriteBuf[szWriteLen], "%d:I:%d:\n", gstOidInfo[i].dSidFirstNum+1, stFaultStat[j].dCritical);
					szWriteLen = strlen(sWriteBuf);

					sprintf(&sWriteBuf[szWriteLen], "%d:I:%d:\n", gstOidInfo[i].dSidFirstNum+2, stFaultStat[j].dMajor);
					szWriteLen = strlen(sWriteBuf);

					sprintf(&sWriteBuf[szWriteLen], "%d:I:%d:\n", gstOidInfo[i].dSidFirstNum+3, stFaultStat[j].dMinor);
					szWriteLen = strlen(sWriteBuf);

					sprintf(&sWriteBuf[szWriteLen], "%d:I:%d:\n", gstOidInfo[i].dSidFirstNum+4, stFaultStat[j].dStop);
					szWriteLen = strlen(sWriteBuf);

					sprintf(&sWriteBuf[szWriteLen], "%d:I:%d:\n", gstOidInfo[i].dSidFirstNum+5, stFaultStat[j].dNormal);
					szWriteLen = strlen(sWriteBuf);

					/* 통계 출력 파라미터 종료 표시 => 10043:C:xxxx: */
					sprintf(&sWriteBuf[szWriteLen], "10043:C:%s:\n", sOidTime);

					fprintf(ofp, "%s\n", sWriteBuf);
					fflush(ofp);
				}
				break;
			case IDX_TRAFFIC_STATISTICS:
				memset(sTableName, 0x00, MAX_BUF_SIZE);
				sprintf(sTableName, "%s", gstOidInfo[i].sTableName);
				sprintf(sQuery,
					"SELECT "
						"TAFID,IFNULL(SUM(THRU_FRAMES),0),IFNULL(SUM(THRU_BYTES),0),IFNULL(SUM(TOT_FRAMES),0),IFNULL(SUM(TOT_BYTES),0),"
						"IFNULL(SUM(IP_FRAMES),0),IFNULL(SUM(IP_BYTES),0),IFNULL(SUM(UDP_FRAMES),0),IFNULL(SUM(UDP_BYTES),0),"
						"IFNULL(SUM(TCP_FRAMES),0),IFNULL(SUM(TCP_BYTES),0),IFNULL(SUM(SCTP_FRAMES),0),IFNULL(SUM(SCTP_BYTES),0),"
						"IFNULL(SUM(IPERROR_FRAMES),0),IFNULL(SUM(IPERROR_BYTES),0),IFNULL(SUM(UTCP_FRAMES),0),IFNULL(SUM(UTCP_BYTES),0),"
						"IFNULL(SUM(FAILDATA_FRAMES),0),IFNULL(SUM(FAILDATA_BYTES),0),IFNULL(SUM(FILTEROUT_FRAMES),0),IFNULL(SUM(FILTEROUT_BYTES),0) "
					"FROM "
						"%s "
					"WHERE "
						"STATTIME>=%lu AND STATTIME<%lu "
					"GROUP BY "
						"TAFID "
					"ORDER BY "
						"TAFID",
					sTableName, pstAtQueryInfo->tStartTime, pstAtQueryInfo->tEndTime);

				if(mysql_query(pstMySQL, sQuery) != 0)
				{
					log_print(LOGN_CRI, LH"FAILED IN mysql_query(%s) mysql_errno[%d-%s]", LT,
						sQuery, mysql_errno(pstMySQL), mysql_error(pstMySQL));
					fclose(ofp);
					return -9;
				}

				if( (pstRst = mysql_store_result(pstMySQL)) == NULL)
				{
					log_print(LOGN_CRI, LH"FAILED IN mysql_store_result(%s) mysql_errno[%d-%s]", LT,
						sQuery, mysql_errno(pstMySQL), mysql_error(pstMySQL));
					fclose(ofp);
					return -10;
				}

				j	= 0;
				memset(stTrafficStat, 0x00, sizeof(st_TrafficStat)*EQUIP_MAX_COUNT);
				while( (stRow = mysql_fetch_row(pstRst)) != NULL)
				{
					if(j > EQUIP_MAX_COUNT)
					{
						log_print(LOGN_CRI, LH"Count[%d] is over EQUIP_MAX_COUNT[%d]", LT, j, EQUIP_MAX_COUNT);
						fclose(ofp);
						return -11;
					}

					stTrafficStat[j].cTAFID				= (char)atoi(stRow[0]);

					stTrafficStat[j].llThruFrames		= (long long)atoll(stRow[1]);
					stTrafficStat[j].llThruBytes		= (long long)atoll(stRow[2]);
					stTrafficStat[j].llTotFrames		= (long long)atoll(stRow[3]);
					stTrafficStat[j].llTotBytes			= (long long)atoll(stRow[4]);

					stTrafficStat[j].llIPFrames			= (long long)atoll(stRow[5]);
					stTrafficStat[j].llIPBytes			= (long long)atoll(stRow[6]);
					stTrafficStat[j].llUDPFrames		= (long long)atoll(stRow[7]);
					stTrafficStat[j].llUDPBytes			= (long long)atoll(stRow[8]);

					stTrafficStat[j].llTCPFrames		= (long long)atoll(stRow[9]);
					stTrafficStat[j].llTCPBytes			= (long long)atoll(stRow[10]);
					stTrafficStat[j].llSCTPFrames		= (long long)atoll(stRow[11]);
					stTrafficStat[j].llSCTPBytes		= (long long)atoll(stRow[12]);

					stTrafficStat[j].llIPErrorFrames	= (long long)atoll(stRow[13]);
					stTrafficStat[j].llIPErrorBytes		= (long long)atoll(stRow[14]);
					stTrafficStat[j].llUTCPFrames		= (long long)atoll(stRow[15]);
					stTrafficStat[j].llUTCPBytes		= (long long)atoll(stRow[16]);

					stTrafficStat[j].llFailDataFrames	= (long long)atoll(stRow[17]);
					stTrafficStat[j].llFailDataBytes	= (long long)atoll(stRow[18]);
					stTrafficStat[j].llFilterOutFrames	= (long long)atoll(stRow[19]);
					stTrafficStat[j].llFilterOutBytes	= (long long)atoll(stRow[20]);
					j++;
				}
				mysql_free_result(pstRst);
				dCount	= j;

				for(j = 0; j < dCount; j++)
				{
					memset(sWriteBuf, 0x00, MAX_BUF_SIZE);
					szWriteLen = 0;

					sprintf(&sWriteBuf[szWriteLen], "REPLACE\n");
					szWriteLen = strlen(sWriteBuf);

					sprintf(&sWriteBuf[szWriteLen], "%d:I:%d:\n", gMyNEID, gstOidInfo[i].dObjectID);
					szWriteLen = strlen(sWriteBuf);

					sprintf(&sWriteBuf[szWriteLen], "%d:I:%u:\n", gstOidInfo[IDX_LOAD_STATISTICS].dSidFirstNum+1, stTrafficStat[j].cTAFID);
					szWriteLen = strlen(sWriteBuf);
					sprintf(&sWriteBuf[szWriteLen], "%d:I:%lld:\n", gstOidInfo[i].dSidFirstNum, stTrafficStat[j].llThruFrames);
					szWriteLen = strlen(sWriteBuf);
					sprintf(&sWriteBuf[szWriteLen], "%d:I:%lld:\n", gstOidInfo[i].dSidFirstNum+1, stTrafficStat[j].llThruBytes);
					szWriteLen = strlen(sWriteBuf);
					sprintf(&sWriteBuf[szWriteLen], "%d:I:%lld:\n", gstOidInfo[i].dSidFirstNum+2, stTrafficStat[j].llTotFrames);
					szWriteLen = strlen(sWriteBuf);
					sprintf(&sWriteBuf[szWriteLen], "%d:I:%lld:\n", gstOidInfo[i].dSidFirstNum+3, stTrafficStat[j].llTotBytes);
					szWriteLen = strlen(sWriteBuf);
					sprintf(&sWriteBuf[szWriteLen], "%d:I:%lld:\n", gstOidInfo[i].dSidFirstNum+4, stTrafficStat[j].llIPFrames);
					szWriteLen = strlen(sWriteBuf);
					sprintf(&sWriteBuf[szWriteLen], "%d:I:%lld:\n", gstOidInfo[i].dSidFirstNum+5, stTrafficStat[j].llIPBytes);
					szWriteLen = strlen(sWriteBuf);
					sprintf(&sWriteBuf[szWriteLen], "%d:I:%lld:\n", gstOidInfo[i].dSidFirstNum+6, stTrafficStat[j].llUDPFrames);
					szWriteLen = strlen(sWriteBuf);
					sprintf(&sWriteBuf[szWriteLen], "%d:I:%lld:\n", gstOidInfo[i].dSidFirstNum+7, stTrafficStat[j].llUDPBytes);
					szWriteLen = strlen(sWriteBuf);
					sprintf(&sWriteBuf[szWriteLen], "%d:I:%lld:\n", gstOidInfo[i].dSidFirstNum+8, stTrafficStat[j].llTCPFrames);
					szWriteLen = strlen(sWriteBuf);
					sprintf(&sWriteBuf[szWriteLen], "%d:I:%lld:\n", gstOidInfo[i].dSidFirstNum+9, stTrafficStat[j].llTCPBytes);
					szWriteLen = strlen(sWriteBuf);
					sprintf(&sWriteBuf[szWriteLen], "%d:I:%lld:\n", gstOidInfo[i].dSidFirstNum+11, stTrafficStat[j].llSCTPFrames);
					szWriteLen = strlen(sWriteBuf);
					sprintf(&sWriteBuf[szWriteLen], "%d:I:%lld:\n", gstOidInfo[i].dSidFirstNum+12, stTrafficStat[j].llSCTPBytes);
					szWriteLen = strlen(sWriteBuf);
					sprintf(&sWriteBuf[szWriteLen], "%d:I:%lld:\n", gstOidInfo[i].dSidFirstNum+13, stTrafficStat[j].llIPErrorFrames);
					szWriteLen = strlen(sWriteBuf);
					sprintf(&sWriteBuf[szWriteLen], "%d:I:%lld:\n", gstOidInfo[i].dSidFirstNum+14, stTrafficStat[j].llIPErrorBytes);
					szWriteLen = strlen(sWriteBuf);
					sprintf(&sWriteBuf[szWriteLen], "%d:I:%lld:\n", gstOidInfo[i].dSidFirstNum+15, stTrafficStat[j].llUTCPFrames);
					szWriteLen = strlen(sWriteBuf);
					sprintf(&sWriteBuf[szWriteLen], "%d:I:%lld:\n", gstOidInfo[i].dSidFirstNum+16, stTrafficStat[j].llUTCPBytes);
					szWriteLen = strlen(sWriteBuf);
					sprintf(&sWriteBuf[szWriteLen], "%d:I:%lld:\n", gstOidInfo[i].dSidFirstNum+17, stTrafficStat[j].llFailDataFrames);
					szWriteLen = strlen(sWriteBuf);
					sprintf(&sWriteBuf[szWriteLen], "%d:I:%lld:\n", gstOidInfo[i].dSidFirstNum+18, stTrafficStat[j].llFailDataBytes);
					szWriteLen = strlen(sWriteBuf);
					sprintf(&sWriteBuf[szWriteLen], "%d:I:%lld:\n", gstOidInfo[i].dSidFirstNum+19, stTrafficStat[j].llFilterOutFrames);
					szWriteLen = strlen(sWriteBuf);
					sprintf(&sWriteBuf[szWriteLen], "%d:I:%lld:\n", gstOidInfo[i].dSidFirstNum+20, stTrafficStat[j].llFilterOutBytes);
					szWriteLen = strlen(sWriteBuf);
					/* 통계 출력 파라미터 종료 표시 => 10043:C:xxxx: */
					sprintf(&sWriteBuf[szWriteLen], "10043:C:%s:\n", sOidTime);
					fprintf(ofp, "%s\n", sWriteBuf);
					fflush(ofp);
				}
				break;
			default:
				log_print(LOGN_CRI, LH"Unknown Statistics[%d]", LT, i);
				fclose(ofp);
				return -12;
		}
	}
	fclose(ofp);

	return 0;
}

int dGetABSMinute(int r_min)
{
	static int	ret_min = 0;

	if(!(r_min%5))
		ret_min = r_min;
	else
		ret_min = r_min - (r_min%5);

	return ret_min;
}

struct tm *stTmNow(void)
{
	struct tm	*stTm;
	time_t		tNow;

	tNow	= time(NULL);
	stTm	= localtime(&tNow);

	return stTm;
}

int dGetOidTime(int dPeriodic, char *sOidTime)
{
	time_t		tClock;
	struct tm	*stTmNow;

	time(&tClock);
	tClock = ( (tClock/SEC_OF_5MIN)*SEC_OF_5MIN);
	stTmNow = localtime(&tClock);

	if( (dPeriodic == STAT_PERIOD_5MIN) || (dPeriodic == STAT_PERIOD_HOUR))
		sprintf(sOidTime, "%04d%02d%02d%02d%02d%02d", stTmNow->tm_year+1900, stTmNow->tm_mon+1, stTmNow->tm_mday, stTmNow->tm_hour, stTmNow->tm_min, stTmNow->tm_sec);
	else
		sprintf(sOidTime, "%04d%02d%02d", stTmNow->tm_year+1900, stTmNow->tm_mon+1, stTmNow->tm_mday);

	return 0;
}

int dResetFileList(char *sFileName, int dSfd)
{
	int		k, rst_cnt;

	rst_cnt	= 0;
	for(k = 0; k < (FILE_NUM_5MIN+FILE_NUM_HOUR); k++)
	{
		if(!strcmp(sFileName, hold_list[k].sFileName) && (hold_list[k].dSfd==dSfd))
		{
			hold_list[k].dSfd	= 0;
			hold_list[k].dFlag	= 0;
			hold_list[k].tInitialTime	= 0;
			memset(&hold_list[k].sFileName, 0x00, FILE_NAME_LEN);
			log_print(LOGN_CRI, LH"reset FileName[%s] dSfd[%d]", LT, sFileName, dSfd);

			return 0;
		}
	}
	log_print(LOGN_CRI, LH"Not reset FileName[%s] dSfd[%d]", LT, sFileName, dSfd);
	DisplayFileList();

	return -1;
}

int dReadNTAFName(st_NTafName *pstNTafName)
{
	FILE	*fp;
	char	sFullPath[PATH_MAX], sTMF[16], sIP[MAX_IPADDR_SIZE], sAlias[16], sBuf[MAX_BUF_SIZE];
	int		dType, dSystemNo, dActiveFlag, dCount, dRet;

	sprintf(sFullPath, "%s%s", DATA_PATH, "SUBSYS_INFO.dat");
	if( (fp = fopen(sFullPath, "r")) == NULL)
	{
		log_print(LOGN_CRI, LH"FAILED IN fopen(%s) errno[%d-%s]", LT,
			sFullPath, errno, strerror(errno));
        return -1;
	}
	else
		log_print(LOGN_DEBUG, LH"SUCCESS IN fopen(%s)", LT, "SUBSYS_INFO.dat");

	dCount	= 0;
	while(fgets(sBuf, MAX_BUF_SIZE, fp) != NULL)
	{
		if(dCount > MAX_NTAF_COUNT)
		{
			log_print(LOGN_CRI, LH"dCount[%d] is over MAX_NTAF_COUNT[%d]", LT, dCount, MAX_NTAF_COUNT);
			fclose(fp);
			return -2;
		}

        if(sBuf[0] != '#')
		{
            log_print(LOGN_CRI, LH"File(%s) row format error", LT, "SUBSYS_INFO.dat");
            fclose(fp);
            return -3;
        }

        if(sBuf[1] == '#')
            continue;
        else if(sBuf[1] == 'E')
            break;
        else if(sBuf[1] == '@')
		{
			if( (dRet = sscanf(&sBuf[2], "%s %d %d %s %d %s", sTMF, &dType, &dSystemNo, sIP, &dActiveFlag, sAlias)) == 6)
			{
				if(strcmp(sTMF, "TMF") == 0)
				{
					pstNTafName[dCount].dSysNo	= dSystemNo;
					sprintf(pstNTafName[dCount].sNTAFName, "%s", sAlias);
					dCount++;
				}
			}
        }
	}
	fclose(fp);

	return dCount;
}

void GetAlarmStr(char *sAlias, unsigned char ucLocType, unsigned char ucInvType, char *psBuf)
{
	size_t	szStrLen;

	szStrLen	= 0;
	sprintf(psBuf+szStrLen, "%s ", sAlias);
	szStrLen = strlen(psBuf);

	switch(ucLocType)
	{
		case LOCTYPE_PHSC:
			switch(ucInvType)
			{
				case INVTYPE_POWER:
					sprintf(psBuf+szStrLen, "%s ", "POWER Alarm");
					szStrLen = strlen(psBuf);
					break;
				//case INVTYPE_ETH_INF:
				case INVTYPE_LINK:
					sprintf(psBuf+szStrLen, "%s ", "NIC Alarm");
					szStrLen = strlen(psBuf);
					break;
				case INVTYPE_FAN:
					sprintf(psBuf+szStrLen, "%s ", "FAN Alarm");
					szStrLen = strlen(psBuf);
					break;
				//case INVTYPE_DISKARRY:
				case INVTYPE_DISKARRAY:
					sprintf(psBuf+szStrLen, "%s ", "DISKARRAY Alarm");
					szStrLen = strlen(psBuf);
					break;
				//case INVTYPE_BDF_MIRROR:
				case INVTYPE_PORT_MIRROR:
					sprintf(psBuf+szStrLen, "%s ", "Direct Mirror Alarm");
					szStrLen = strlen(psBuf);
					break;
				case INVTYPE_PORT_SWITCH:
					sprintf(psBuf+szStrLen, "%s ", "Switch Port Alarm");
					szStrLen = strlen(psBuf);
					break;
				case INVTYPE_POWER_DIRECTOR:
					sprintf(psBuf+szStrLen, "%s ", "Director Power Alarm");
					szStrLen = strlen(psBuf);
					break;
				default:
					sprintf(psBuf+szStrLen, "%s ", "Unknown Alarm");
					szStrLen = strlen(psBuf);
					break;
			}
			break;
		case LOCTYPE_LOAD:
			switch(ucInvType)
			{
				case INVTYPE_CPU:
					sprintf(psBuf+szStrLen, "%s ", "CPU Load");
					szStrLen = strlen(psBuf);
					break;
				case INVTYPE_MEMORY:
					sprintf(psBuf+szStrLen, "%s ", "Memory Load");
					szStrLen = strlen(psBuf);
					break;
				case INVTYPE_DISK:
					sprintf(psBuf+szStrLen, "%s ", "Disk Load");
					szStrLen = strlen(psBuf);
					break;
				case INVTYPE_QUEUE:
					sprintf(psBuf+szStrLen, "%s ", "Queue Load");
					szStrLen = strlen(psBuf);
					break;
				case INVTYPE_SESSION:
					sprintf(psBuf+szStrLen, "%s ", "Session Load");
					szStrLen = strlen(psBuf);
					break;
				case INVTYPE_DBSTATUS:
					sprintf(psBuf+szStrLen, "%s ", "MySQL Status");
					szStrLen = strlen(psBuf);
					break;
				default:
					sprintf(psBuf+szStrLen, "%s ", "Unknown Alarm");
					szStrLen = strlen(psBuf);
					break;
			}
			break;
		case LOCTYPE_PROCESS:
			switch(ucInvType)
			{
				case INVTYPE_USERPROC:
#if 0	// 쓰지 않는 알람 타입
				case INVTYPE_USERTRCE:
#endif
					sprintf(psBuf+szStrLen, "%s ", "SW Process Alarm");
					szStrLen = strlen(psBuf);
					break;
				default:
					sprintf(psBuf+szStrLen, "%s ", "Unknown Alarm");
					szStrLen = strlen(psBuf);
					break;
			}
			break;
		case LOCTYPE_CHNL:
			switch(ucInvType)
			{
#if 0	// 쓰지 않는 알람 타입
				case INVTYPE_NTAF:
					sprintf(psBuf+szStrLen, "%s ", "NTAF Channel Alarm");
					szStrLen = strlen(psBuf);
					break;
				case INVTYPE_NTAM:
					sprintf(psBuf+szStrLen, "%s ", "NTAM Channel Alarm");
					szStrLen = strlen(psBuf);
					break;
				case INVTYPE_DHUB:
					sprintf(psBuf+szStrLen, "%s ", "DHUB Channel Alarm");
					szStrLen = strlen(psBuf);
					break;
				case INVTYPE_DSCP:
					sprintf(psBuf+szStrLen, "%s ", "DSCP Channel Alarm");
					szStrLen = strlen(psBuf);
					break;
				case INVTYPE_IDR:
					sprintf(psBuf+szStrLen, "%s ", "IDR Channel Alarm");
					szStrLen = strlen(psBuf);
					break;
				case INVTYPE_NOIDR:
					sprintf(psBuf+szStrLen, "%s ", "NOIDR Channel Alarm");
					szStrLen = strlen(psBuf);
					break;
				case INVTYPE_DB_LINK:
					sprintf(psBuf+szStrLen, "%s ", "DB Link Channel Alarm");
					szStrLen = strlen(psBuf);
					break;
#endif
				case INVTYPE_NMS:
					sprintf(psBuf+szStrLen, "%s ", "NMS Channel Alarm");
					szStrLen = strlen(psBuf);
					break;
				default:
					sprintf(psBuf+szStrLen, "%s ", "Unknown Alarm");
					szStrLen = strlen(psBuf);
					break;
			}
			break;
#if 0	// 쓰지 않는 알람 타입
		case LOCTYPE_NETLINK:
			switch(ucInvType)
			{
				case INVTYPE_PINGDSCP:
					sprintf(psBuf+szStrLen, "%s ", "DSCP Ping Alarm");
					szStrLen = strlen(psBuf);
					break;
				case INVTYPE_PINGPDSN:
					sprintf(psBuf+szStrLen, "%s ", "PDSN Ping Alarm");
					szStrLen = strlen(psBuf);
					break;
				case INVTYPE_PINGIWF:
					sprintf(psBuf+szStrLen, "%s ", "GIWF Ping Alarm");
					szStrLen = strlen(psBuf);
					break;
				case INVTYPE_PINGAAA:
					sprintf(psBuf+szStrLen, "%s ", "AAA Ping Alarm");
					szStrLen = strlen(psBuf);
					break;
				case INVTYPE_PINGGGSN:
					sprintf(psBuf+szStrLen, "%s ", "GGSN Ping Alarm");
					szStrLen = strlen(psBuf);
					break;
				case INVTYPE_PINGIPC:
					sprintf(psBuf+szStrLen, "%s ", "IPC Ping Alarm");
					szStrLen = strlen(psBuf);
					break;
				case INVTYPE_PINGWAG:
					sprintf(psBuf+szStrLen, "%s ", "WAG Ping Alarm");
					szStrLen = strlen(psBuf);
					break;
				default:
					sprintf(psBuf+szStrLen, "%s ", "Unknown Alarm");
					szStrLen = strlen(psBuf);
					break;
			}
			break;
#endif
		/* added by dcham 20110616 */
		case LOCTYPE_SVC:
			switch(ucInvType)
			{
				case INVTYPE_RECALL:                                
					sprintf(psBuf+szStrLen, "%s ", "RECALL Alarm");
					szStrLen = strlen(psBuf);
					break;
				case INVTYPE_AAA:                                
					sprintf(psBuf+szStrLen, "%s ", "AAA Alarm");
					szStrLen = strlen(psBuf);
					break;
				case INVTYPE_HSS:                                
					sprintf(psBuf+szStrLen, "%s ", "HSS Alarm");
					szStrLen = strlen(psBuf);
					break;
				case INVTYPE_LNS:                                
					sprintf(psBuf+szStrLen, "%s ", "LNS Alarm");
					szStrLen = strlen(psBuf);
					break;
				case INVTYPE_MENU:                                
					sprintf(psBuf+szStrLen, "%s ", "MENU Alarm");
					szStrLen = strlen(psBuf);
					break;
				case INVTYPE_DN:                                
					sprintf(psBuf+szStrLen, "%s ", "DN Alarm");
					szStrLen = strlen(psBuf);
					break;
				case INVTYPE_STREAM:                                
					sprintf(psBuf+szStrLen, "%s ", "STREAM Alarm");
					szStrLen = strlen(psBuf);
					break;
				case INVTYPE_MMS:                                
					sprintf(psBuf+szStrLen, "%s ", "MMS Alarm");
					szStrLen = strlen(psBuf);
					break;
				case INVTYPE_WIDGET:                                
					sprintf(psBuf+szStrLen, "%s ", "WIDGET Alarm");
					szStrLen = strlen(psBuf);
					break;
				case INVTYPE_PHONE:                                
					sprintf(psBuf+szStrLen, "%s ", "PHONE Alarm");
					szStrLen = strlen(psBuf);
					break;
				case INVTYPE_EMS:                                
					sprintf(psBuf+szStrLen, "%s ", "EMS Alarm");
					szStrLen = strlen(psBuf);
					break;
				case INVTYPE_BANK:                                
					sprintf(psBuf+szStrLen, "%s ", "BANK Alarm");
					szStrLen = strlen(psBuf);
					break;
				case INVTYPE_FV:                                
					sprintf(psBuf+szStrLen, "%s ", "FV Alarm");
					szStrLen = strlen(psBuf);
					break;
				case INVTYPE_IM:                                
					sprintf(psBuf+szStrLen, "%s ", "IM Alarm");
					szStrLen = strlen(psBuf);
					break;
				case INVTYPE_VT:                                
					sprintf(psBuf+szStrLen, "%s ", "VT Alarm");
					szStrLen = strlen(psBuf);
					break;
				case INVTYPE_ETC:                                
					sprintf(psBuf+szStrLen, "%s ", "ETC Alarm");
					szStrLen = strlen(psBuf);
					break;
				case INVTYPE_CORP:                                
					sprintf(psBuf+szStrLen, "%s ", "CORP Alarm");
					szStrLen = strlen(psBuf);
					break;
				case INVTYPE_REGI:                                
					sprintf(psBuf+szStrLen, "%s ", "REGI Alarm");
					szStrLen = strlen(psBuf);
					break;
				case INVTYPE_INET:                                
					sprintf(psBuf+szStrLen, "%s ", "INET Alarm");
					szStrLen = strlen(psBuf);
					break;
				case INVTYPE_RECVCALL:                                
					sprintf(psBuf+szStrLen, "%s ", "RECVCALL Alarm");
					szStrLen = strlen(psBuf);
					break;
				case INVTYPE_IM_RECV:                                
					sprintf(psBuf+szStrLen, "%s ", "IM RECV Alarm");
					szStrLen = strlen(psBuf);
					break;
				case INVTYPE_VT_RECV:
					sprintf(psBuf+szStrLen, "%s ", "VT RECV Alarm");
					szStrLen = strlen(psBuf);
					break;
				default:
					sprintf(psBuf+szStrLen, "%s ", "Unknown Alarm");
					szStrLen = strlen(psBuf);
					break;
			}
			break;
		default:
			sprintf(psBuf+szStrLen, "%s ", "Unknown Alarm");
			szStrLen = strlen(psBuf);
			break;
	}
}

int dCheck_Channel(int hdFlag, unsigned int uiIP)
{
	if(fidb->cInterlock[SI_NMS_INTERLOCK] != MASK)
	{
		if(hdFlag == 0)
		{    /* DisConnect */
			log_print(LOGN_CRI, LH"DEAD CHANNEL CONNECT-FLAG=%d OLD_STATUS=%d IP=%u", LT,
				hdFlag, fidb->cInterlock[SI_NMS_INTERLOCK], uiIP);
			fidb->cInterlock[SI_NMS_INTERLOCK] = CRITICAL;
		}
		else if(hdFlag == 1)
		{
			log_print(LOGN_CRI, LH"ALIVE CHANNEL CONNECT-FLAG=%d OLD_STATUS=%d IP=%u", LT,
				hdFlag, fidb->cInterlock[SI_NMS_INTERLOCK], uiIP);
			fidb->cInterlock[SI_NMS_INTERLOCK] = NORMAL;
		}
		else
		{
			log_print(LOGN_CRI, LH"UNKOWN CHANNEL CONNECT-FLAG=%d OLD_STATUS=%d IP=%u", LT,
				hdFlag, fidb->cInterlock[SI_NMS_INTERLOCK], uiIP);
		}
	}
	else
	{
		log_print(LOGN_CRI, LH"MASK CHANNEL CONNECT-FLAG=%d OLD_STATUS=%d IP=%u", LT,
			hdFlag, fidb->cInterlock[SI_NMS_INTERLOCK], uiIP);
	}

	return 0;
}
