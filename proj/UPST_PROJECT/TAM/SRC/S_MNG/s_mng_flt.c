/**A.1*  FILE INCLUSION *******************************************************/
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <define.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <dirent.h>			/* DIR */

#include "s_mng_flt.h"
#include "s_mng_init.h"

#include "path.h"
#include "commdef.h"		/* FILE_XXX */
#include "filter.h"
#include "db_api.h"
#include "db_define.h"
#include "filedb.h"			/* st_keepalive */
#include "loglib.h"

/** B.1* DEFINITION OF NEW CONSTANTS **********************/
/** C.1* DEFINITION OF NEW TYPES **************************/
/** D.1* DECLARATION OF VARIABLES *************************/
int		gInfoAccessHour;
int		gInfoAccessMin;

int		gCTNInfoHour;
int		gCTNInfoMin;

/** D.2* DECLARATION OF EXTERN VARIABLES *************************/
extern st_AlmLevel_List stNtafAlarm[MAX_NTAF_NUM];
extern st_Flt_Info		*flt_info;
extern int 				gdCNTOnOff;
extern int				gdSysNo;

/** E.1* DEFINITION OF FUNCTIONS **************************/
/** E.2* DEFINITION OF FUNCTIONS **************************/

int dInitSysConfig(void)
{
    FILE *fa;
    char szBuf[1024], szType[64], szTmp[64], szInfo[64];
    int  i,dRet, isReadCTNCnt = -1;

    fa = fopen(FILE_SYS_CONFIG, "r");
    if(fa == NULL){
        log_print(LOGN_CRI, "%s:%s:%d][LOAD SYSTEM CONFIG : %s FILE OPEN FAIL (%s)", __FILE__, __FUNCTION__, __LINE__, FILE_SYS_CONFIG, strerror(errno));
        return -1;
    }

    i = 0;
    while(fgets(szBuf, 1024, fa) != NULL){
        if(szBuf[0] != '#'){
            log_print(LOGN_WARN,"FAILED IN dInitSysConfig() : %s File [%d] row format error", FILE_SYS_CONFIG, i);
        }

        i++;

        if(szBuf[1] == '#') continue;
        else if(szBuf[1] == 'E') break;
        else if(szBuf[1] == '@'){

			if( (dRet = sscanf(&szBuf[2], "%s %s %s", szType, szTmp, szInfo)) > 0 ){
				switch(dRet){
					case 2:
						if( strcmp(szType,"MS-CTN") == 0 ){
							if( strcmp( szTmp, "ON" ) == 0 || strcmp( szTmp, "on" ) == 0 ){
								gdCNTOnOff = _MSCTN_COUNT_ON;
							}else{
								gdCNTOnOff = _MSCTN_COUNT_OFF;
							}   
							log_print(LOGN_CRI, "LOAD Model-Customer Count %s", gdCNTOnOff == _MSCTN_COUNT_ON? "ON":"OFF");	
							isReadCTNCnt++;
						}
						break;
					case 3:
						if(strcmp(szType, "SYS") == 0){
							if(strcmp(szTmp, "NO") == 0) 	 gdSysNo = atoi(szInfo);

						}else if(strcmp(szType, "INFOACCESS") == 0){
							if(strcmp(szTmp, "HOUR") == 0) 	  gInfoAccessHour	= atoi(szInfo);
							else if(strcmp(szTmp, "MIN") == 0) gInfoAccessMin	= atoi(szInfo);

						}else if(strcmp(szType, "CTN_INFO") == 0){
							if(strcmp(szTmp, "HOUR") == 0) 	  gCTNInfoHour	= atoi(szInfo);
							else if(strcmp(szTmp, "MIN") == 0) gCTNInfoMin	= atoi(szInfo);
						}
					default:
						break;
				}
			}
        }
    }/* while */

    fclose(fa);
	if( isReadCTNCnt < 0 ){
		log_print(LOGN_CRI,"%s:%s:%d]MS-CTN COUNT is NOT FOUND @%s", __FILE__, __FUNCTION__, __LINE__, FILE_SYS_CONFIG);
		return -2;
	}
	return 0;

} /* end of dInitSysConfig */

int dInitLogLvl(void)
{
	FILE	*fp;
	int		i, dCount;
	char	sBuf[1024], sType1[64], sType2[64], sType3[64];

	i	= 0;
	if( (fp = fopen(FILE_LOG_LEVEL, "r")) == NULL)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN fopen(%s) [errno:%d-%s]", 
			__FILE__, __FUNCTION__, __LINE__, FILE_LOG_LEVEL, errno, strerror(errno));
		return -1;
	}

	while(fgets(sBuf, 1024, fp) != NULL)
	{
		if(sBuf[0] != '#')
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN FILE(%s) LINE(%d) FORMAT", 
				__FILE__, __FUNCTION__, __LINE__, FILE_LOG_LEVEL, i);
			fclose(fp);
			return -2;
		}

		switch(sBuf[1])
		{
			case '#':
				continue;
			case 'E':
				goto end_dInitLogLvl;
			case '@':
				if(sscanf(&sBuf[2], "%s %s %s", sType1, sType2, sType3) == 3)
				{
					if(strcmp(sType1, "LOGLVL") == 0)
					{
						dCount = atoi(sType2);
						if(dCount > MAX_SW_COUNT || dCount < 0) {
							log_print(LOGN_CRI, "F=%s:%s.%d INVALID COUNT=%d", __FILE__, __FUNCTION__, __LINE__, dCount);
							dCount = MAX_SW_COUNT;
						}
						for( i = 0; i < MAX_SW_COUNT; i++ ){
							g_stLogLevel->usLogLevel[i] = atoi(sType3);
							log_print(LOGN_CRI, "F=%s:%s.%d: LOG LEVEL INIT >> IDX=%d LEVEL=%d", 
								__FILE__, __FUNCTION__, __LINE__, i, g_stLogLevel->usLogLevel[i]);
						}
					}
				}
				break;
			default:
				log_print(LOGN_CRI, "F=%s:%s.%d: sBuf[1][%c] is not delimeter in a file[%s]", 
					__FILE__, __FUNCTION__, __LINE__, sBuf[1], FILE_LOG_LEVEL);
				break;
		}
	}
end_dInitLogLvl:
	fclose(fp);

	return 0;
}

int dRead_FLT_Tmf(st_NtafIP_db *stNtafIP)
{
	char	sBuf[1024], sInfo1[64], sInfo2[64], sInfo3[64], sInfo4[64], sInfo5[64];
	int		i, htohIP, dNtafCnt;
	FILE	*fp;

	if( (fp = fopen(DATA_PATH"SUBSYS_INFO.dat", "r")) == NULL)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN fopen(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, "SUBSYS_INFO.dat", errno, strerror(errno));
		return -1;
	}

	dNtafCnt	= 0;

	for(i = 0; i < MAX_NTAF_NUM; i++)
	{
		stNtafIP->sNo[i]	= 0;
		stNtafIP->uiIP[i]	= 0;
	}

	while(fgets(sBuf, 1024, fp) != NULL)
	{
		if(sBuf[0] != '#')
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN FILE(%s) LINE(%d) FORMAT", __FILE__, __FUNCTION__, __LINE__, "LOGLVL.dat", i);
			fclose(fp);
			return -2;
		}

		switch(sBuf[1])
		{
			case '#':
				continue;
			case 'E':
				goto end_dRead_FLT_Tmf;
			case '@':
				if(sscanf(&sBuf[2], "%s %s %s %s %s", sInfo1, sInfo2, sInfo3, sInfo4, sInfo5) == 5)
				{
					if(strcmp(sInfo1,"TMF") == 0)
					{
						stNtafIP->sNo[dNtafCnt]		= atoi(sInfo3);
						htohIP						= inet_addr(sInfo4);
						stNtafIP->uiIP[dNtafCnt]	= ntohl(htohIP);
						log_print(LOGN_CRI, "F=%s:%s.%d: i=%d TYPE=%s NO=%d IP=%s:%d F=%s", __FILE__, __FUNCTION__, __LINE__,
							dNtafCnt, sInfo2, stNtafIP->sNo[dNtafCnt], sInfo4, stNtafIP->uiIP[dNtafCnt], sInfo5);
						dNtafCnt++;
					}
				}
				break;
			default:
				log_print(LOGN_CRI, "F=%s:%s.%d: sBuf[1][%c] is not delimeter in a file[%s]", __FILE__, __FUNCTION__, __LINE__, sBuf[1], "SUBSYS_INFO.dat");
				break;
		}
	}
end_dRead_FLT_Tmf:
	log_print(LOGN_CRI, "F=%s:%s.%d: NtafCnt [%d] d_NtafCnt [%d]", 
		__FILE__, __FUNCTION__, __LINE__, stNtafIP->sNo[dNtafCnt-1], dNtafCnt);
	fclose(fp);

	return stNtafIP->sNo[dNtafCnt-1];
}

/*	START: GI NTAF에게만 SCTP filter 정보를 보내주기 위하여	*/
int	dRead_FLT_GINTAF_Tmf(st_Sub_GI_NTAF_List *stGINtafIP)
{
	FILE	*fp;
	int		dGINtafCnt, htohIP;
	char	sBuf[1024], sInfo1[64], sInfo2[64], sInfo3[64], sInfo4[64], sInfo5[64];

	if( (fp = fopen(DATA_PATH"SUBSYS_INFO.dat", "r")) == NULL)
	{
		log_print(LOGN_CRI,"F=%s:%s.%d: FAILED IN fopen(%s) - errno{%d: %s]", __FILE__, __FUNCTION__, __LINE__,
			"SUBSYS_INFO.dat", errno, strerror(errno));
		return -1;
	}

	dGINtafCnt	= 0;
	memset(stGINtafIP, 0x00, sizeof(st_Sub_GI_NTAF_List));

	while(fgets(sBuf, 1024, fp) != NULL)
	{
		if(sBuf[0] != '#')
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: %s File [%d] row format error", __FILE__, __FUNCTION__, __LINE__, "SUBSYS_INFO.dat", dGINtafCnt);
			fclose(fp);
			return -2;
		}

		switch(sBuf[1])
		{
			case '#':
				continue;
			case 'E':
				goto end_dRead_FLT_GINTAF_Tmf;
			case '@':
				if(sscanf(&sBuf[2], "%s %s %s %s %s", sInfo1, sInfo2, sInfo3, sInfo4, sInfo5) == 5)
				{
					if( (strcmp(sInfo1, "TMF") == 0) && (strcmp(sInfo2, "1") == 0))
					{
						stGINtafIP->usSysType[dGINtafCnt]	= atoi(sInfo2);
						stGINtafIP->usSysNo[dGINtafCnt]		= atoi(sInfo3);
						htohIP								= inet_addr(sInfo4);
						stGINtafIP->uiIP[dGINtafCnt]		= ntohl(htohIP);
						log_print(LOGN_CRI, "F=%s:%s.%d: i=%d TYPE=%hu NO=%hu IP=%s:%d F=%s", __FILE__, __FUNCTION__, __LINE__, dGINtafCnt,
							stGINtafIP->usSysType[dGINtafCnt], stGINtafIP->usSysNo[dGINtafCnt], sInfo4, stGINtafIP->uiIP[dGINtafCnt], sInfo5);
						dGINtafCnt++;
					}
				}
				break;
			default:
				log_print(LOGN_CRI, "F=%s:%s.%d: sBuf[1][%c] is not delimeter in a file[%s]", __FILE__, __FUNCTION__, __LINE__, sBuf[1], "SUBSYS_INFO.dat");
				break;
		}
	}
end_dRead_FLT_GINTAF_Tmf:
	log_print(LOGN_CRI, "F=%s:%s.%d: d_NtafCnt [%d]", __FILE__, __FUNCTION__, __LINE__, dGINtafCnt);
	fclose(fp);

	return dGINtafCnt;
}
/*	END: Writer: Han-jin Park, Date: 2009.05.11	*/

int dRead_FLT_AlmLvl_NTAF(st_AlmLevel_List *stAlm, int dNo)
{
	int		dSysNo, dCri, dMaj, dMin, dIDX = 0;
	FILE	*fp;
	char	szBuf[1024], szType[16];

	if( (fp = fopen(DATA_PATH"AlmClsLoad_NTAF","r")) == NULL)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN fopen(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			"AlmClsLoad_NTAF", errno, strerror(errno));
		return -1;
	}

	while(fgets(szBuf, 1024, fp) != NULL)
	{
		if(szBuf[0] != '#')
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN FILE(%s) FORMAT", __FILE__, __FUNCTION__, __LINE__, "AlmClsLoad_NTAF");
			fclose(fp);
			return -2;
		}

		if(szBuf[1] == '#')
			continue;
		else if(szBuf[1] == 'E')
			break;
		else if(szBuf[1] == '@')
		{
			if(sscanf(&szBuf[2],"%d %s %d %d %d", &dSysNo, szType, &dCri, &dMaj, &dMin) == 5)
			{
				if(dSysNo == dNo)
				{
					if(strcmp(szType, "CPU") == 0)
						dIDX = CPU_LOC;
					else if(strcmp(szType, "MEM") == 0)
						dIDX = MEM_LOC;
					else if(strcmp(szType, "DISK") == 0)
						dIDX = DISK_LOC;
					else if(strcmp(szType, "QUE") == 0)
						dIDX = QUE_LOC;
					else if(strcmp(szType, "NIFO") == 0)
						dIDX = NIFO_LOC;
					else if(strcmp(szType, "TRAFFIC") == 0)
						dIDX = TAF_TRAFFIC_LOC;
				}
				else
					continue;

				strcpy(stAlm->stAlmLevel[dIDX].szTypeName, szType);
				stAlm->stAlmLevel[dIDX].sCriticalLevel	= dCri;
				stAlm->stAlmLevel[dIDX].sMajorLevel		= dMaj;
				stAlm->stAlmLevel[dIDX].sMinorLevel		= dMin;

				log_print(LOGN_DEBUG, "F=%s:%s.%d: NTAF=%d ALARM >>> T=%s CRI=%d MAJ=%d MIN=%d", __FILE__, __FUNCTION__, __LINE__,
					dNo, stAlm->stAlmLevel[dIDX].szTypeName, stAlm->stAlmLevel[dIDX].sCriticalLevel,
					stAlm->stAlmLevel[dIDX].sMajorLevel, stAlm->stAlmLevel[dIDX].sMinorLevel);
			}
		}
	}
	fclose(fp);

	return 0;
}

int dInit_Ntaf_Alarm(void)
{
	FILE	*fp;
	char	sBuf[1024], sType[16];
	int		dSysNo, dCri, dMaj, dMin, dIDX;

	if( (fp = fopen(FILE_TAF_LOAD_DATA, "r")) == NULL)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN fopen(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			"AlmClsLoad_NTAF", errno, strerror(errno));
		return -1;
	}

	while(fgets(sBuf, 1024, fp) != NULL)
	{
		if(sBuf[0] != '#')
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: File(=%s) row format error", __FILE__, __FUNCTION__, __LINE__, FILE_TAF_LOAD_DATA);
			fclose(fp);
			return -2;
		}

		switch(sBuf[1])
		{
			case '#':
				continue;
			case 'E':
				goto end_dInit_Ntaf_Alarm;
			case '@':
				if(sscanf(&sBuf[2], "%d %s %d %d %d", &dSysNo, sType, &dCri, &dMaj, &dMin) == 5)
				{
					if(strcmp(sType,"CPU") == 0)
						dIDX = CPU_LOC;
					else if(strcmp(sType, "MEM") == 0)
						dIDX = MEM_LOC;
					else if(strcmp(sType, "DISK") == 0)
						dIDX = DISK_LOC;
					else if(strcmp(sType, "QUE") == 0)
						dIDX = QUE_LOC;
					else if(strcmp(sType, "NIFO") == 0)
						dIDX = NIFO_LOC;
					else if(strcmp(sType, "TRAFFIC") == 0)
						dIDX = TAF_TRAFFIC_LOC;

					stNtafAlarm[dSysNo-1].dSysNo	= dSysNo;
					stNtafAlarm[dSysNo-1].dCount	= ALM_CNT-1;
					strcpy(stNtafAlarm[dSysNo-1].stAlmLevel[dIDX].szTypeName, sType);
					stNtafAlarm[dSysNo-1].stAlmLevel[dIDX].sCriticalLevel	= dCri;
					stNtafAlarm[dSysNo-1].stAlmLevel[dIDX].sMajorLevel		= dMaj;
					stNtafAlarm[dSysNo-1].stAlmLevel[dIDX].sMinorLevel		= dMin;

					log_print(LOGN_CRI, "NTAF=%d ALARM LEVEL >>> T=%s CRI=%d MAJ=%d MIN=%d",
						dSysNo, stNtafAlarm[dSysNo-1].stAlmLevel[dIDX].szTypeName,
						stNtafAlarm[dSysNo-1].stAlmLevel[dIDX].sCriticalLevel, stNtafAlarm[dSysNo-1].stAlmLevel[dIDX].sMajorLevel,
						stNtafAlarm[dSysNo-1].stAlmLevel[dIDX].sMinorLevel);
				}
				break;
			default:
				log_print(LOGN_CRI, "F=%s:%s.%d: sBuf[1][%c] is not delimeter in a file(=%s)", 
					__FILE__, __FUNCTION__, __LINE__, sBuf[1], FILE_TAF_LOAD_DATA);
				break;
		}
	}
end_dInit_Ntaf_Alarm:
	fclose(fp);

	return 0;
} /* end of dResd_FLT_AlmLv_NTAF */

int dLogWrite(st_LogLevel_List	*pstLogLevelList)
{
	char	sConfDir[1024], sConfFile[80];
	FILE	*fp;
	DIR		*dirp;

	memset(sConfFile, 0x00, 80);
	memset(sConfDir, 0x00, 1024);

	strcpy(sConfDir, DATA_PATH);
	strcpy(sConfFile, FILE_LOG_LEVEL);

	log_print(LOGN_INFO, "F=%s:%s.%d: CONFPATH=%s] MSGPATH=%s]", __FILE__, __FUNCTION__, __LINE__, sConfDir, sConfFile);

	if( (dirp = opendir(sConfDir)) == (DIR*)NULL)
		mkdir(sConfDir, 0777);
	else
		closedir(dirp);

	if( (fp = fopen(sConfFile, "w")) == NULL)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN fopen(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, sConfFile, errno, strerror(errno));
		return -1;
	}
	fprintf(fp, "## LOGLVL ## Process Count ## LEVEL \n#@ LOGLVL  %d %d  \n#E  \n",
		pstLogLevelList->dCount, pstLogLevelList->stLogLevel.usLogLevel[0]);
	fflush(fp);
	fclose(fp);

	return 0;
}

int dAlmWrite_NTAM(st_keepalive *keepalive)
{
	char	sConfDir[1024], sConfFile[80]; /* G*/
	FILE	*fp;
	DIR		*dirp;

	memset(sConfDir, 0x00, 1024);
	memset(sConfFile, 0x00, 80);

	strcpy(sConfDir, DATA_PATH);
	sprintf(sConfFile, "%s/%s", DATA_PATH, "AlmClsLoad_NTAM");
	log_print(LOGN_INFO, "F=%s:%s.%d: CONF_PATH=%s] MSG_PATH=%s]", __FILE__, __FUNCTION__, __LINE__, sConfDir, sConfFile);

	if( (dirp = opendir(sConfDir)) == (DIR*)NULL)
		mkdir(sConfDir, 0777);
	else
		closedir(dirp);

	if( (fp = fopen(sConfFile, "w")) == NULL)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN fopen(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, sConfFile, errno, strerror(errno));
		return -1;
	}
	fprintf(fp, "#@ %s %d %d %d \n", "CPU", keepalive->stTAMLoad.cpu.usCritical, keepalive->stTAMLoad.cpu.usMajor, keepalive->stTAMLoad.cpu.usMinor);
	fprintf(fp, "#@ %s %d %d %d \n", "MEM", keepalive->stTAMLoad.mem.usCritical, keepalive->stTAMLoad.mem.usMajor, keepalive->stTAMLoad.mem.usMinor);
	fprintf(fp, "#@ %s %d %d %d \n", "DISK", keepalive->stTAMLoad.disk.usCritical, keepalive->stTAMLoad.disk.usMajor, keepalive->stTAMLoad.disk.usMinor);
	fprintf(fp, "#@ %s %d %d %d \n", "QUE", keepalive->stTAMLoad.que.usCritical, keepalive->stTAMLoad.que.usMajor, keepalive->stTAMLoad.que.usMinor);
	fprintf(fp, "#@ %s %d %d %d \n", "NIFO", keepalive->stTAMLoad.nifo.usCritical, keepalive->stTAMLoad.nifo.usMajor, keepalive->stTAMLoad.nifo.usMinor);
	fprintf(fp, "#@ %s %d %d %d \n", "SWCPU", keepalive->stSWCHLoad.cpu.usCritical, keepalive->stSWCHLoad.cpu.usMajor, keepalive->stSWCHLoad.cpu.usMinor);
	fprintf(fp, "#@ %s %d %d %d \n", "SWMEM", keepalive->stSWCHLoad.mem.usCritical, keepalive->stSWCHLoad.mem.usMajor, keepalive->stSWCHLoad.mem.usMinor);
	fprintf(fp, "#E  \n");
	fflush(fp);
	fclose(fp);

	log_print(LOGN_DEBUG,  "#@ CPU %d %d %d ", keepalive->stTAMLoad.cpu.usCritical, keepalive->stTAMLoad.cpu.usMajor, keepalive->stTAMLoad.cpu.usMinor);
	log_print(LOGN_DEBUG,  "#@ MEM %d %d %d ", keepalive->stTAMLoad.mem.usCritical, keepalive->stTAMLoad.mem.usMajor, keepalive->stTAMLoad.mem.usMinor);
	log_print(LOGN_DEBUG,  "#@ DISK %d %d %d ", keepalive->stTAMLoad.disk.usCritical, keepalive->stTAMLoad.disk.usMajor, keepalive->stTAMLoad.disk.usMinor);
	log_print(LOGN_DEBUG,  "#@ QUE %d %d %d ", keepalive->stTAMLoad.que.usCritical, keepalive->stTAMLoad.que.usMajor, keepalive->stTAMLoad.que.usMinor);
	log_print(LOGN_DEBUG,  "#@ NIFO %d %d %d ", keepalive->stTAMLoad.nifo.usCritical, keepalive->stTAMLoad.nifo.usMajor, keepalive->stTAMLoad.nifo.usMinor);
	log_print(LOGN_DEBUG,  "#@ SWCPU %d %d %d ", keepalive->stSWCHLoad.cpu.usCritical, keepalive->stSWCHLoad.cpu.usMajor, keepalive->stSWCHLoad.cpu.usMinor);
	log_print(LOGN_DEBUG,  "#@ SWMEM %d %d %d ", keepalive->stSWCHLoad.mem.usCritical, keepalive->stSWCHLoad.mem.usMajor, keepalive->stSWCHLoad.mem.usMinor);

	return 0;
}

int dAlmWrite_NTAF(void)
{
	int		i, j;
	char	mesg_path[80];
	FILE	*fp;

	memset(mesg_path, 0x00, 80);
	sprintf(mesg_path, "%s", DATA_PATH"AlmClsLoad_NTAF");
	if( (fp = fopen(mesg_path, "w+")) == NULL)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN fopen(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, mesg_path, errno, strerror(errno));
		return -1;
	}

	for(i = 0; i < MAX_NTAF_NUM; i++)
	{
		if(stNtafAlarm[i].dSysNo < 1)
			continue;

		for(j = 0; j < stNtafAlarm[i].dCount; j++)
		{
			fprintf (fp,  "#@  %d %s %u %u %u \n",
				stNtafAlarm[i].dSysNo, stNtafAlarm[i].stAlmLevel[j].szTypeName, stNtafAlarm[i].stAlmLevel[j].sCriticalLevel,
				stNtafAlarm[i].stAlmLevel[j].sMajorLevel, stNtafAlarm[i].stAlmLevel[j].sMinorLevel);
		}
	}
	fprintf(fp, "#E  \n");
	fflush(fp);
	fclose(fp);

	return 0;
}
