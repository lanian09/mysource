/*******************************************************************************
			DQMS Project

	Author   : Jae Seung Lee
	Section  : SI_NMS
	SCCS ID  : @(#)si_nms_init.c	1.1
	Date     : 01/21/05
	Revision History :
        '05. 01. 21		Initial
		'08. 01. 07		Update By LSH for review
		'08. 01. 14		Add By LSH for IUPS NTAM

	Description :

	Copyright (c) uPRESTO 2005
*******************************************************************************/

/** A.1 * File Include ************************************/
#include <stdio.h>			/*	fopen(3), fgets(3), fclose(3)		*/
#include <string.h>			/*	memset(3), strerror(3), strcmp(3)	*/
#include <errno.h>			/*	errno(3)	*/
#include <signal.h>			/*	signal(2)	*/
#include <stdlib.h>			/*	atoi(3)		*/
#include <mysql/mysql.h>	/*	MYSQL		*/
#include <ctype.h>			/*	isspace(3)	*/
#include <arpa/inet.h>		/*	inet_addr(3)	*/

// LIB
#include "mems.h"
#include "gifo.h"
#include "cifo.h"
#include "loglib.h"
#include "ipclib.h"
#include "dblib.h"

// OAM
#include "path.h"
#include "procid.h"
#include "filedb.h"			/* st_NTAM */
#include "sshmid.h"

// PROJECT
#include "msgdef.h"

// .
#include "si_nms_comm.h"
#include "si_nms_init.h"

/** B.1* DEFINITION OF NEW CONSTANTS **********************/
// Move to si_nms_init.h

/** C.1* DEFINITION OF NEW TYPES **************************/

/** D.1* DECLARATION OF VARIABLES *************************/
int						dFinishSig;
int						gMyNEID;
st_OidInfo				gstOidInfo[IDX_MAXIMUM_STATISTICS];		/*	SRC/SI_NMS/si_nms.h			*/
char					gsLocSys[MAX_LOCSYS_SIZE];				/*	SRC/SI_NMS/si_nms.h			*/
int						gdSysNo;

/** D.2* DECLARATION OF VARIABLES *************************/
extern int				gJiSTOPFlag;							/* SRC/SI_NMS/si_nms_main.c	*/
extern st_NMSIPList		gstNMSIPList;							/* SRC/SI_NMS/si_nms_main.c	*/
extern st_NMSPortInfo	gstNMSPortInfo;							/* SRC/SI_NMS/si_nms_main.c	*/
extern MYSQL			stMySQL;								/* SRC/SI_NMS/si_nms_main.c	*/

pst_WNTAM    			stWNTAM;								/* To init fidb */
extern pst_NTAM			fidb;	

/** E.1* DEFINITION OF FUNCTIONS **************************/
// Move to si_nms_init.h

/** E.2* DEFINITION OF FUNCTIONS **************************/
extern int dGetIPAddr(char *conf_file, char *primary_addr, int dMaxLen);			/*	SRC/SI_NMS/si_nms_func.c	*/

int dInitProc(stMEMSINFO **pMEMSINFO, stCIFO **pCIFO)
{
	int		dRet;

	SetUpSignal();

	/*
	 * GIFO 를 사용하기 위한 설정
	 */
	*pMEMSINFO = nifo_init_zone((U8*)"SI_NMS", SEQ_PROC_SI_NMS, FILE_NIFO_ZONE);
	if(*pMEMSINFO == NULL)
	{
		log_print(LOGN_CRI, LH"FAILED IN nifo_init_zone, NULL", LT);
		return -1;
	}

	*pCIFO = gifo_init_group(FILE_CIFO_CONF, FILE_GIFO_CONF);
	if(*pCIFO == NULL)
	{
		log_print(LOGN_CRI, LH"FAILED IN gifo_init_group, RET=NULL, cifo=%s, gifo=%s", LT,
					FILE_CIFO_CONF, FILE_GIFO_CONF);
		return -2;
	}

	if( (dRet = dGetSYSCFG()) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dGetSYSCFG() dRet[%d]", LT, dRet);
		return -4;
	}

	memset(&gstNMSPortInfo, 0x00, DEF_NMSPORTINFO_SIZE);
	if( (dRet = dReadNMSPortInfoFile(&gstNMSPortInfo)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dReadNMSPortInfoFile() dRet[%d]", LT, dRet);
		return -5;
	}

	if( (dRet = dReadNMSIPInfoFile(&gstNMSIPList)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dReadNMSIPInfoFile() dRet[%d]", LT, dRet);
		return -6;
	}

	if( (dRet = dReadNMSOidInfoFile()) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dReadNMSOidInfoFile() dRet[%d]", LT, dRet);
		return -7;
	}

	if( (dRet = dGetIPAddr(FILE_SUP_IP_CONF, gstNMSPortInfo.ipaddr[0], sizeof(gstNMSPortInfo.ipaddr[0]))) < 0)
	{
		log_print(LOGN_CRI, LH"FAILED dGetIPAddr(\"%s\")", LT, FILE_SUP_IP_CONF);
		return -8;
	}
	else
		log_print(LOGN_DEBUG, LH"gstNMSPortInfo.ipaddr[0][%s] SUCCESS!", LT, gstNMSPortInfo.ipaddr[0]);

	return 0;
} /* end of dInitProc */

void SetUpSignal(void)
{
	gJiSTOPFlag = 1;

    /* WANTED SIGNALS   */
    signal(SIGTERM, UserControlledSignal);
    signal(SIGINT,  UserControlledSignal);
    signal(SIGQUIT, UserControlledSignal);

    /* UNWANTED SIGNALS */
    signal(SIGHUP,  IgnoreSignal);
    signal(SIGALRM, IgnoreSignal);
    signal(SIGPIPE, IgnoreSignal);
    signal(SIGPOLL, IgnoreSignal);
    signal(SIGPROF, IgnoreSignal);
    signal(SIGUSR1, IgnoreSignal);
    signal(SIGUSR2, IgnoreSignal);
    signal(SIGVTALRM, IgnoreSignal);
    signal(SIGCLD, SIG_IGN);

	log_print(LOGN_DEBUG, LH"SIGNAL HANDLER WAS INSTALLED! gJiSTOPFlag[%d]", LT, gJiSTOPFlag);
} /* end of SetUpSignal */

void UserControlledSignal(int sign)
{
	gJiSTOPFlag	= 0;
	dFinishSig	= sign;
} /* end of UserControlledSignal */

void FinishProgram(void)
{
	log_print(LOGN_CRI, "FinishProgram : PROGRAM IS NORMALLY TERMINATED, Cause = %d", dFinishSig);

	//dDisConnectMySQL(&stMySQL);
	db_disconn(&stMySQL);
}

void IgnoreSignal(int sign)
{
	if(sign != SIGALRM)
		log_print(LOGN_CRI, "IgnoreSignal : UNWANTED SIGNAL IS RECEIVED, signal = %d", sign);

	signal(sign, IgnoreSignal);
} /* end of IgnoreSignal */

int dReadNMSPortInfoFile(st_NMSPortInfo *pstData)
{
	int		dCnt, dLine;
	FILE	*fp;
	char	szBuf[1024], szInfo_1[64], szInfo_2[64];

	if( (fp = fopen(FILE_NMSPORT, "r")) == NULL)
	{
		log_print(LOGN_CRI, LH"FAILED IN fopen(%s) [errno:%d-%s]", LT,
			FILE_NMSPORT, errno, strerror(errno));
		return -1;
	}

	dCnt	= 0;
	dLine	= 0;
	while(fgets(szBuf,1024,fp) != NULL)
	{
		if(szBuf[0] != '#')
		{
			log_print(LOGN_CRI, LH"ERROR IN FILE(%s) LINE(%d) FORMAT", LT, FILE_NMSPORT, dCnt+1);
			fclose(fp);
			return -2;
		}

		if(szBuf[1] == '#')
			continue;
		else if(szBuf[1] == 'E')
			break;
		else if(szBuf[1] == '@')
		{
			if(sscanf(&szBuf[2],"%s %s", szInfo_1, szInfo_2) == 2)
			{
				if(dCnt > PORT_IDX_MAX)
				{
					log_print(LOGN_CRI, "SUB MAX DISCARD i=%d INFO1=%s INFO2=%s", dLine, szInfo_1, szInfo_2);
					break;
				}

				if(strcmp(szInfo_1, "NMS_ALM") == 0)
				{
					pstData->port[PORT_IDX_ALM]	= atoi(szInfo_2);
					log_print(LOGN_DEBUG, LH"%s[%d]", LT, szInfo_1, pstData->port[PORT_IDX_ALM]);
				}
				else if(strcmp(szInfo_1, "NMS_CONS") == 0)
				{
					pstData->port[PORT_IDX_CONS]	= atoi(szInfo_2);
					log_print(LOGN_DEBUG, LH"%s[%d]", LT, szInfo_1, pstData->port[PORT_IDX_CONS]);
				}
				else if(strcmp(szInfo_1, "NMS_CONF") == 0)
				{
					pstData->port[PORT_IDX_CONF]	= atoi(szInfo_2);
					log_print(LOGN_DEBUG, LH"%s[%d]", LT, szInfo_1, pstData->port[PORT_IDX_CONF]);
				}
				else if(strcmp(szInfo_1, "NMS_MMC") == 0)
				{
					pstData->port[PORT_IDX_MMC]	= atoi(szInfo_2);
					log_print(LOGN_DEBUG, LH"%s[%d]", LT, szInfo_1, pstData->port[PORT_IDX_MMC]);
				}
				else if(strcmp(szInfo_1, "NMS_STAT") == 0)
				{
					pstData->port[PORT_IDX_STAT]	= atoi(szInfo_2);
					log_print(LOGN_DEBUG, LH"%s[%d]", LT, szInfo_1, pstData->port[PORT_IDX_STAT]);
				}
				else if(strcmp(szInfo_1, "NMS_NEID") == 0)
				{
					gMyNEID	= atoi(szInfo_2);
					log_print(LOGN_DEBUG, LH"%s[%d]", LT, szInfo_1, gMyNEID);
				}
				else
					log_print(LOGN_CRI, LH"Unknown delimiter[%s]", LT, szInfo_1);

				dCnt++;
			}
		}
		dLine++;
	}
	fclose(fp);

	if( (dCnt != PORT_IDX_MAX+1) && (pstData->port[PORT_IDX_ALM]) && (pstData->port[PORT_IDX_CONS]) &&
		(pstData->port[PORT_IDX_CONF]) && (pstData->port[PORT_IDX_MMC]) && (pstData->port[PORT_IDX_STAT]))
	{
		log_print(LOGN_CRI, LH"ERROR IN FILE[%s]", LT, FILE_NMSPORT);
		return -3;
	}

	return 0;
}

int dReadNMSIPInfoFile(st_NMSIPList *pstData)
{
	int		dCnt, dLine;
	FILE	*fp;
	char	szBuf[1024], szInfo_1[64], szInfo_2[64];

	if( (fp = fopen(FILE_NMSIP, "r")) == NULL)
	{
		log_print(LOGN_CRI, LH"FAILED IN fopen(%s) [errno:%d-%s]", LT,
			FILE_NMSIP, errno, strerror(errno));
		return -1;
	}

	dCnt	= 0;
	dLine	= 0;
	while(fgets(szBuf, 1024, fp) != NULL)
	{
		if(szBuf[0] != '#')
		{
			log_print(LOGN_CRI, LH"ERROR IN FILE(%s) LINE(%d) FORMAT", LT, FILE_NMSIP, dCnt+1);
			fclose(fp);
			return -2;
		}

		if(szBuf[1] == '#')
			continue;
		else if(szBuf[1] == 'E')
			break;
		else if(szBuf[1] == '@')
		{
			if(sscanf(&szBuf[2], "%s %s", szInfo_1, szInfo_2) == 2)
			{
				if(dCnt > MAX_STATISTICS_CONN)
				{
					log_print(LOGN_CRI, "SUB MAX DISCARD i=%d INFO1=%s INFO2=%s", dLine, szInfo_1, szInfo_2);
					break;
				}

				if(strcmp(szInfo_1, "5MIN") == 0)
				{
					pstData->stNMSIPInfo[dCnt].cType	= STAT_PERIOD_5MIN;
					pstData->stNMSIPInfo[dCnt].uIP		= ntohl(inet_addr(szInfo_2));
					log_print(LOGN_DEBUG, LH"STAT_PERIOD_5MIN cType[0x%02X] uIP[%u]", LT,
						pstData->stNMSIPInfo[dCnt].cType, pstData->stNMSIPInfo[dCnt].uIP);
				}
				else if(strcmp(szInfo_1, "1HOUR") == 0)
				{
					pstData->stNMSIPInfo[dCnt].cType	= STAT_PERIOD_HOUR;
					pstData->stNMSIPInfo[dCnt].uIP		= ntohl(inet_addr(szInfo_2));
					log_print(LOGN_DEBUG, LH"STAT_PERIOD_HOUR cType[0x%02X] uIP[%u]", LT,
						pstData->stNMSIPInfo[dCnt].cType, pstData->stNMSIPInfo[dCnt].uIP);
				}
				else
					log_print(LOGN_CRI, LH"Unknown delimiter[%s]", LT, szInfo_1);

				dCnt++;
			}
		}
		dLine++;
	}
	fclose(fp);
	pstData->dCount	= dCnt;

	return 0;
}

int dGetSYSCFG(void)
{
	FILE	*fa;
	char	sBuf[1024], sType[64], sTemp[64], sInfo[64];
	int		i, dRet, dIsFoundLocSys, dIsFoundSysNo;

	dRet	= 0;
	if( (fa = fopen(FILE_SYS_CONFIG,"r"))== NULL)
	{
		log_print(LOGN_CRI, LH"ERROR IN fopen(%s) errno[%d:%s]", LT,
			FILE_SYS_CONFIG, errno, strerror(errno));
		return -1;
	}

	i				= 0;
	dIsFoundLocSys	= 0;
	dIsFoundSysNo	= 0;
	while( fgets(sBuf, 1024, fa) != NULL)
	{
		if(sBuf[0] != '#')
		{
			log_print(LOGN_WARN, LH"File [%s:%d] row format error", LT,
				FILE_SYS_CONFIG, i);
		}

		i++;
		if(sBuf[1] == '#') continue;
		else if(sBuf[1] == 'E') break;
		else if(sBuf[1] == '@'){
			if( (dRet = sscanf(&sBuf[2], "%s %s %s", sType, sTemp, sInfo)) > 0)
			{
				switch(dRet)
				{
					case 2:
						if(strcmp(sType, "LOC-CODE") == 0)
						{
							strncpy(gsLocSys, sTemp, MAX_LOCSYS_SIZE);
							log_print(LOGN_DEBUG, LH"gsLocSys[%s]", LT, gsLocSys);
							dIsFoundLocSys	= 1;
						}
						break;
					case 3:
						if(strcmp(sType, "SYS") == 0)
						{
							if(strcmp(sTemp, "NO") == 0)
							{
								gdSysNo = atoi(sInfo);
								log_print(LOGN_DEBUG, LH"gdSysNo[%d]", LT, gdSysNo);
								dIsFoundSysNo	= 1;
							}
						}
						break;
					default:
						break;
				}
			}
		}
	}
	fclose(fa);

	if(!dIsFoundLocSys)
	{
		log_print(LOGN_CRI, LH"LOC-CODE is NOT FOUND. Check File[%s]", LT, FILE_SYS_CONFIG);
		return -2;
	}

	if(!dIsFoundSysNo)
	{
		log_print(LOGN_CRI, LH"SYS NO is NOT FOUND. Check File[%s]", LT, FILE_SYS_CONFIG);
		return -3;
	}

	return 0;
}

#if 0
int dInit_MsgQId(void)
{
	if( (gdMyQId = msgget(S_MSGQ_SI_NMS, 0666 | IPC_CREAT)) < 0)
	{
		log_print(LOGN_CRI, LH"FAILED IN msgget(S_MSGQ_SI_NMS[0x%04X]) [errno:%d-%s]", LT,
			S_MSGQ_SI_NMS, errno, strerror(errno));

		return -1;
	}

	if( (gdAlmdQId = msgget(S_MSGQ_ALMD, 0666 | IPC_CREAT)) < 0)
	{
		log_print(LOGN_CRI, LH"FAILED IN msgget(S_MSGQ_ALMD[0x%04X]) [errno:%d-%s]", LT,
			S_MSGQ_ALMD, errno, strerror(errno));

		return -2;
	}

	if( (gdCondQId = msgget(S_MSGQ_COND, 0666 | IPC_CREAT)) < 0)
	{
		log_print(LOGN_CRI, LH"FAILED IN msgget(S_MSGQ_COND[0x%04X]) [errno:%d-%s]", LT,
			S_MSGQ_COND, errno, strerror(errno));

		return -3;
	}

	if( (gdMmcdQId = msgget(S_MSGQ_MMCD, 0666 | IPC_CREAT)) < 0)
	{
		log_print(LOGN_CRI, LH"FAILED IN msgget(S_MSGQ_MMCD[0x%04X]) [errno:%d-%s]", LT,
			S_MSGQ_MMCD, errno, strerror(errno));

		return -4;
	}

	return 0;
}
#endif

int dReadNMSOidInfoFile(void)
{
	FILE	*fp;
	char	sTableName[40], sFullPath[PATH_MAX], sBuf[MAX_BUF_SIZE];
	int		dOid, dSectionNumber, dRet;
	size_t	szLength;

	sprintf(sFullPath, "%s", FILE_NMSOID);
	if( (fp = fopen(sFullPath, "r")) == NULL)
	{
		log_print(LOGN_CRI, LH"FAILED IN fopen(%s) errno[%d-%s]", LT,
			sFullPath, errno, strerror(errno));
		return -1;
	}

	while(fgets(sBuf, MAX_BUF_SIZE, fp) != NULL)
	{
		szLength = strlen(sBuf);
		while(isspace(sBuf[szLength - 1]))
			sBuf[--szLength] = 0x00;

		if(sBuf[0] == '#')
			continue;

		if( (dRet = sscanf(sBuf, "%s %d %d", sTableName, &dSectionNumber, &dOid)) == 3)
		{
			if(strstr(sTableName, "LOAD") != NULL)
			{
				sprintf(gstOidInfo[IDX_LOAD_STATISTICS].sTableName, "%s", sTableName);
				gstOidInfo[IDX_LOAD_STATISTICS].dSidFirstNum	= dSectionNumber;
				gstOidInfo[IDX_LOAD_STATISTICS].dObjectID		= dOid;
			}
			else if(strstr(sTableName, "FAULT") != NULL)
			{
				sprintf(gstOidInfo[IDX_FAULT_STATISTICS].sTableName, "%s", sTableName);
				gstOidInfo[IDX_FAULT_STATISTICS].dSidFirstNum	= dSectionNumber;
				gstOidInfo[IDX_FAULT_STATISTICS].dObjectID		= dOid;
			}
			else if(strstr(sTableName, "TRAFFIC") != NULL)
			{
				sprintf(gstOidInfo[IDX_TRAFFIC_STATISTICS].sTableName, "%s", sTableName);
				gstOidInfo[IDX_TRAFFIC_STATISTICS].dSidFirstNum	= dSectionNumber;
				gstOidInfo[IDX_TRAFFIC_STATISTICS].dObjectID	= dOid;
			}
			else
			{
				log_print(LOGN_CRI, LH"Unknown TableName[%s]", LT, sTableName);
				fclose(fp);
				return -2;
			}
		}
		else
		{
			log_print(LOGN_CRI, LH"FAILED IN sscanf(%s) dRet[%d]", LT, sBuf, dRet);
			fclose(fp);
			return -3;
		}
	}
	fclose(fp);

	return 0;
}

int dInitFidb(void)
{
    int     dRet;

    if( (dRet = shm_init(S_SSHM_FIDB, DEF_WNTAM_SIZE, (void**)&stWNTAM)) < 0 ){
        log_print(LOGN_CRI,LH"FAILED IN shm_init(S_SSHM_FIDB[0x%04X], dRet=%d)"EH,
        LT,S_SSHM_FIDB,dRet,ET);
        return -6;
    }

    fidb     = &stWNTAM->stNTAM;
    //director = &stWNTAM->stDirectTOT;
    //swch     = &stWNTAM->stSwitchTOT;

	/**
	 * 공유메모리 초기화 부분이 빠져있음
	 */

    return 0;
}
