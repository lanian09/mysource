/*******************************************************************************
			DQMS Project

	Author   : Jae Seung Lee
	Section  : SI_SVCMON
	SCCS ID  : @(#)si_svcmon_init.c	1.1
	Date     : 01/21/05
	Revision History :
        '05. 01. 21		Initial
		'08. 01. 07		Update By LSH for review
		'08. 01. 14		Add By LSH for IUPS NTAM

	Description :

	Copyright (c) uPRESTO 2005
*******************************************************************************/

/***** A.1 * File Include *******************************/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/ipc.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/msg.h>
#include "path.h"
#include "ipclib.h"
#include "sshmid.h"
#include "nsocklib.h"
#include "loglib.h"
#include "filedb.h"
#include "memg.h"
#include "clist_memg.h"
#include "si_svcmon_init.h"


/** B.1* DEFINITION OF NEW CONSTANTS **********************/
/** C.1* DEFINITION OF NEW TYPES **************************/
/** D.1* DECLARATION OF VARIABLES *************************/
extern int					dMyQid;

extern int					JiSTOPFlag;
extern int					Finishflag;

extern st_SubSysInfoList	gstSubList;

pst_WNTAM         fidb;
extern stMEMGINFO			*pMEMGINFO;
extern st_Send_Info			gstSendInfo;
extern st_ClientInfo       stSock[MAX_RECORD];
/** E.1* DEFINITION OF FUNCTIONS **************************/
/** E.2* DEFINITION OF FUNCTIONS **************************/

int dInitProc(void)
{
	int		dRet;

	SetUpSignal();
	
	memset(&gstSubList, 0x00, DEF_SUBSYSINFOLIST_SIZE);
	if( (dRet = dReadSubSysInfoFile(&gstSubList)) < 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN dReadSubSysInfoFile() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		return -1;
	}

	if((pMEMGINFO =  cmem_init(0, SIDB_NODE_SIZE, SIDB_NODE_CNT)) == NULL) {
		log_print(LOGN_CRI, "F=%s:%s.%d cmem_init NULL", __FILE__, __FUNCTION__, __LINE__);
		return -2;
	}
#if 0
	if((dMyQid = msgget(S_MSGQ_SI_SVCMON, 0666|IPC_CREAT)) < 0) {
		log_print(LOGN_CRI, "F=%s:%s.%d msgget S_MSGQ_SI_SVCMON error=%d:%s",
				__FILE__, __FUNCTION__, __LINE__, errno, strerror(errno));
		return -3;
	}
#endif
	dReadRemainFile();

	return 0;
} /* end of dInitProc */

void SetUpSignal(void)
{
	JiSTOPFlag = 1;

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

	log_print(LOGN_DEBUG, "SetUpSignal : SIGNAL HANDLER WAS INSTALLED[%d]", JiSTOPFlag);
} /* end of SetUpSignal */

void UserControlledSignal(int sign)
{
	JiSTOPFlag	= 0;
	Finishflag	= sign;
} /* end of UserControlledSignal */

void FinishProgram(void)
{
	int	i;
	log_print(LOGN_CRI, "FinishProgram : PROGRAM IS NORMALLY TERMINATED, Cause = %d", Finishflag);
    for(i = 0; i< MAX_RECORD; i++)
    {
		if(stSock[i].dSfd > 0) close(stSock[i].dSfd);
    }
#ifndef DNMS_CONN_DISABLE
	fidb->stNTAM.cInterlock[SI_DNMS_INTERLOCK] = CRITICAL;
#endif
	dWriteRemainFile();
	exit(0);
} /* end of FinishProgram */

void IgnoreSignal(int sign)
{
	if(sign != SIGALRM)
		log_print(LOGN_CRI, "IgnoreSignal : UNWANTED SIGNAL IS RECEIVED, signal = %d", sign);

	signal(sign, IgnoreSignal);
} /* end of IgnoreSignal */

int dReadSubSysInfoFile(pst_SubSysInfoList pstList)
{
	int		dCnt, dLine;
	FILE	*fa;
	char	szBuf[1024];
	char	szInfo_1[64], szInfo_2[64], szInfo_3[64];

	fa = fopen(FILE_SVCMON_INFO, "r");
	if(fa == NULL) {
		log_print(LOGN_CRI,"dReadSubSysInfoFile : FILE=%s OPEN FAIL (%s)", FILE_SVCMON_INFO, strerror(errno));
		return -1;
	}

	dCnt = 0; dLine = 0;
	while(fgets(szBuf,1024,fa) != NULL)
	{
		if(szBuf[0] != '#') {
			log_print(LOGN_CRI,"dReadSubSysInfoFile : File=%s:%d ROW FORMAT ERR", FILE_SVCMON_INFO, dCnt+1);
			fclose(fa);
			return -2;
		}

		if(szBuf[1] == '#')
			continue;
		else if(szBuf[1] == 'E')
			break;
		else if(szBuf[1] == '@') {
			if(sscanf(&szBuf[2],"%s %s %s", szInfo_1, szInfo_2, szInfo_3) == 3) {
				if(dCnt >= MAX_SUBSYS_NUM) {
					log_print(LOGN_CRI, "SUB MAX DISCARD i=%d INFO1=%s INFO2=%s INFO3=%s", dLine, szInfo_1, szInfo_2, szInfo_3);
					break;
				}
				pstList->stInfo[dCnt].dNo = atoi(szInfo_1);
				pstList->stInfo[dCnt].uiIP =  ntohl(inet_addr(szInfo_2));
				pstList->stInfo[dCnt].dFlag = atoi(szInfo_3);
				log_print(LOGN_CRI, "CIAPP INFO POS=%d NO=%d IP=%u:%s FLAG=%d",
					dCnt, pstList->stInfo[dCnt].dNo, pstList->stInfo[dCnt].uiIP, szInfo_2, pstList->stInfo[dCnt].dFlag);
				dCnt++;
			}
		}
		dLine++;
	}

	fclose(fa);

	pstList->dCount = dCnt;
	if(pstList->dCount != 1) {
		log_print(LOGN_CRI, "READ CIAPP INFO NO DATA : CNT=%d", pstList->dCount);
		return -3;
	}

	log_print(LOGN_CRI, "READ CIAPP INFO DATA : CNT=%d", pstList->dCount);

	return 0;
}

int dReadRemainFile()
{
    int     dCnt, dLine;
    FILE    *fa;
    char    szBuf[1024];
    char    szInfo_1[1024], szInfo_2[1024], szInfo_3[1024];
	st_sidb_node		*pNODE;
	st_SI_DB	*pSIDB;

    fa = fopen(FILE_SVCMON_REMAIN, "r");
    if(fa == NULL) {
        log_print(LOGN_CRI, "dReadRemainFile : FILE=%s OPEN FAIL (%s)", FILE_SVCMON_REMAIN, strerror(errno));
        return -1;
    }
    
    dCnt = 0; dLine = 0;
    while(fgets(szBuf,1024,fa) != NULL)
    {
        if(szBuf[0] != '#') {
            log_print(LOGN_CRI, "dReadRemainFile : File=%s:%d ROW FORMAT ERR", FILE_SVCMON_REMAIN, dCnt+1);
            fclose(fa);
            return -2;
        }

        if(szBuf[1] == '#')
            continue;
        else if(szBuf[1] == 'E')
            break;
        else if(szBuf[1] == '@') {
            if(sscanf(&szBuf[2],"%s %s %s", szInfo_1, szInfo_2, szInfo_3) == 3) {
				if((pNODE = (st_sidb_node *)cmem_alloc(pMEMGINFO)) == NULL) {
					log_print(LOGN_CRI, "F=%s:%s.%d cmem_alloc NULL", __FILE__, __FUNCTION__, __LINE__);
            		fclose(fa);
            		return -3;
					
				}
				pSIDB = &pNODE->stSIDB;

				memcpy(pSIDB->date, szInfo_1, SIDB_DATE_LEN);
				pSIDB->date[SIDB_DATE_LEN] = 0x00;	

				memcpy(pSIDB->filename, szInfo_2, RNES_PKT_SIZE-1);
				pSIDB->filename[RNES_PKT_SIZE-1] = 0x00;

				memcpy(pSIDB->name, szInfo_3, RNES_PKT_SIZE-1);
				pSIDB->name[RNES_PKT_SIZE-1] = 0x00;

				if(gstSendInfo.offset_Data == 0) {
					gstSendInfo.offset_Data = cmem_offset(pMEMGINFO, (U8 *)pNODE);
				} else {
					cmem_link_prev(pMEMGINFO, cmem_ptr(pMEMGINFO, gstSendInfo.offset_Data), (U8 *)pNODE);
				}
				gstSendInfo.cnt++;
				
                log_print(LOGN_CRI, "CIAPP_REMAIN INFO POS=%d date=%s filename=%s name=%s",
                    dCnt, pSIDB->date, pSIDB->filename, pSIDB->name);
                dCnt++;
            }
        }
        dLine++;
    }

    fclose(fa);

    return 0;
}

int dWriteRemainFile()
{
    FILE    			*fa;
	st_sidb_node		*pDATA, *pHEAD, *pNEXT;
	st_SI_DB			*pSIDB;

    fa = fopen(FILE_SVCMON_REMAIN, "w");
    if(fa == NULL) {
        log_print(LOGN_CRI, "dReadRemainFile : FILE=%s OPEN FAIL (%s)", FILE_SVCMON_REMAIN, strerror(errno));
        return -1;
    }

	fprintf(fa, "##date\tfilename\tname\n");

	pHEAD = (st_sidb_node *)cmem_ptr(pMEMGINFO, gstSendInfo.offset_Data);
	pDATA = pHEAD;

	while(pDATA != NULL)
	{
		pSIDB = &pDATA->stSIDB;

		fprintf(fa, "#@%s\t%s\t%s\n", pSIDB->date, pSIDB->filename, pSIDB->name);

		pNEXT = cmem_entry(cmem_ptr(pMEMGINFO, ((st_sidb_node *)pDATA)->list.offset_next), st_sidb_node, list);
		if(pHEAD == pNEXT) pNEXT = NULL;
		pDATA = pNEXT;
	}

	if(gstSendInfo.offset_Data != 0)
		cmem_delete(pMEMGINFO, (U8 *)pHEAD);

    fclose(fa);

    return 0;
}

int Init_Fidb(void)
{
	int     dRet;
	if( (dRet = shm_init(S_SSHM_FIDB, DEF_WNTAM_SIZE, (void**)&fidb)) < 0 ){
		log_print(LOGN_CRI,"%s.%d:%s FAILED IN shm_init(S_SSHM_FIDB[0x%04X], dRet=%d), errno=%d:%s",
				__FILE__,__LINE__,__FUNCTION__,S_SSHM_FIDB,dRet,errno,strerror(errno));
		return -6;
	}
	return 1;
}

