/*******************************************************************************
			DQMS Project

	Author   : Jae Seung Lee
	Section  : SI_SVC
	SCCS ID  : @(#)si_svc_init.c	1.1
	Date     : 01/21/05
	Revision History :
        '05. 01. 21		Initial
		'08. 01. 07		Update By LSH for review
		'08. 01. 14		Add By LSH for IUPS NTAM		

	Description :

	Copyright (c) uPRESTO 2005
*******************************************************************************/

/***** A.1 * File Include *******************************/
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/ipc.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <unistd.h>			/* close() */

// DQMS
#include "path.h"
#include "sshmid.h"
#include "procid.h"

// LIB
#include "loglib.h"
#include "ipclib.h"
#include "cifo.h"
#include "gifo.h"

// OAM
#include "filedb.h"			/* pst_WNTAM */
#include "almstat.h"		/* CRITICAL */

// .
#include "si_db_init.h"

/** B.1* DEFINITION OF NEW CONSTANTS **********************/
/** C.1* DEFINITION OF NEW TYPES **************************/
/** D.1* DECLARATION OF VARIABLES *************************/
pst_WNTAM    				stWNTAM;								/* To init fidb */
extern pst_NTAM				fidb;

extern int					JiSTOPFlag;
extern int					Finishflag;

extern st_SubSysInfoList	gstSubList;

extern stMEMGINFO			*pMEMGINFO;
extern st_Send_Info			gstSendInfo;
extern st_ClientInfo       stSock[MAX_RECORD];

/** E.1* DEFINITION OF FUNCTIONS **************************/
// Move to si_db_init.h

/** E.2* DEFINITION OF FUNCTIONS **************************/
extern stMEMGINFO *cmem_init(S32 uiShmKey, U32 uiBodySize, U32 uiCnt);
extern U8 *cmem_alloc(stMEMGINFO *pMEMGINFO);
extern void cmem_link_prev(stMEMGINFO *pMEMGINFO, U8 *pHead, U8 *pNew);
extern void cmem_delete(stMEMGINFO *pMEMGINFO, U8 *pDel);

int dInitProc(stMEMSINFO **pMEMSINFO, stCIFO **pCIFO)
{
    int         dRet;

    SetUpSignal();

	memset(&gstSubList, 0x00, DEF_SUBSYSINFOLIST_SIZE);
	dRet = dReadSubSysInfoFile(&gstSubList);
	if(dRet < 0) {
		log_print(LOGN_CRI, LH"dInitInfo : FAILED IN dRead_FLT_Tmf dRet[%d]", LT, dRet);
		return -1;
	}

	if((pMEMGINFO =  cmem_init(0, SIDB_NODE_SIZE, SIDB_NODE_CNT)) == NULL) {
		log_print(LOGN_CRI, LH"cmem_init NULL", LT);
		return -2;
	}

	// GIFO 를 위한 NIFO ZONE 설정
	*pMEMSINFO = nifo_init_zone((U8*)"SI_DB", SEQ_PROC_SI_DB, FILE_NIFO_ZONE);
	if(*pMEMSINFO == NULL)
	{
		log_print(LOGN_CRI, LH"FAILED IN nifo_init_zone, NULL", LT);
		return -3;
	}
	
	// GIFO 그룹 설정
	*pCIFO = gifo_init_group(FILE_CIFO_CONF, FILE_GIFO_CONF);
	if(*pCIFO == NULL)
	{
		log_print(LOGN_CRI, LH"FAILED IN gifo_init_group, RET=NULL, cifo=%s, gifo=%s", LT,
					FILE_CIFO_CONF, FILE_GIFO_CONF);
		return -4;
	}
	
	dReadRemainFile();

    return 0;
} /* end of dInitProc */

void SetUpSignal()
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

    return;
} /* end of SetUpSignal */

void UserControlledSignal(int sign)
{
    JiSTOPFlag = 0;
    Finishflag = sign;
} /* end of UserControlledSignal */

void FinishProgram()
{
	int i;
    log_print(LOGN_CRI, 
	"FinishProgram : PROGRAM IS NORMALLY TERMINATED, Cause = %d", Finishflag);
    for(i = 0; i< MAX_RECORD; i++)
    {
        if(stSock[i].dSfd > 0) close(stSock[i].dSfd);
    }
	fidb->cInterlock[SI_DB_INTERLOCK] = CRITICAL;
	dWriteRemainFile();
    exit(0);	
} /* end of FinishProgram */

void IgnoreSignal(int sign)
{
    if (sign != SIGALRM)
        log_print(LOGN_CRI, 
		LH"IgnoreSignal : UNWANTED SIGNAL IS RECEIVED, signal = %d", LT, sign);
    signal(sign, IgnoreSignal);
} /* end of IgnoreSignal */

int dReadSubSysInfoFile(pst_SubSysInfoList pstList)
{
	int		dCnt, dLine;
	FILE	*fa;
	char	szBuf[1024];
	char	szInfo_1[64], szInfo_2[64], szInfo_3[64];

	fa = fopen(FILE_CIAPP_INFO, "r");
	if(fa == NULL) {
		log_print(LOGN_CRI, LH"dReadSubSysInfoFile : FILE=%s OPEN FAIL (%s)", LT, FILE_CIAPP_INFO, strerror(errno));
		return -1;
	}

	dCnt = 0; dLine = 0;
	while(fgets(szBuf,1024,fa) != NULL)
	{
		if(szBuf[0] != '#') {
			log_print(LOGN_CRI, LH"dReadSubSysInfoFile : File=%s:%d ROW FORMAT ERR", LT, FILE_CIAPP_INFO, dCnt+1);
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
					log_print(LOGN_CRI, LH"SUB MAX DISCARD i=%d INFO1=%s INFO2=%s INFO3=%s", LT, dLine, szInfo_1, szInfo_2, szInfo_3);
					break;
				}
				pstList->stInfo[dCnt].dNo = atoi(szInfo_1);
				pstList->stInfo[dCnt].uiIP =  ntohl(inet_addr(szInfo_2));
				pstList->stInfo[dCnt].dFlag = atoi(szInfo_3);
				log_print(LOGN_CRI, LH"CIAPP INFO POS=%d NO=%d IP=%u:%s FLAG=%d", LT,
					dCnt, pstList->stInfo[dCnt].dNo, pstList->stInfo[dCnt].uiIP, szInfo_2, pstList->stInfo[dCnt].dFlag);
				dCnt++;
			}
		}
		dLine++;
	}

	fclose(fa);

	pstList->dCount = dCnt;
	if(pstList->dCount != 1) {
		log_print(LOGN_CRI, LH"READ CIAPP INFO NO DATA : CNT=%d", LT, pstList->dCount);
		return -3;
	}

	log_print(LOGN_CRI, LH"READ CIAPP INFO DATA : CNT=%d", LT, pstList->dCount);

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

    fa = fopen(FILE_CIAPP_REMAIN, "r");
    if(fa == NULL) {
        log_print(LOGN_CRI, LH"dReadRemainFile : FILE=%s OPEN FAIL (%s)", LT, FILE_CIAPP_REMAIN, strerror(errno));
        return -1;
    }
    
    dCnt = 0; dLine = 0;
    while(fgets(szBuf,1024,fa) != NULL)
    {
        if(szBuf[0] != '#') {
            log_print(LOGN_CRI, LH"dReadRemainFile : File=%s:%d ROW FORMAT ERR", LT, FILE_CIAPP_REMAIN, dCnt+1);
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
					log_print(LOGN_CRI, LH"cmem_alloc NULL", LT);
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
				
                log_print(LOGN_CRI, LH"CIAPP_REMAIN POS=%d date=%s filename=%s name=%s", LT,
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

    fa = fopen(FILE_CIAPP_REMAIN, "w");
    if(fa == NULL) {
        log_print(LOGN_CRI, LH"dReadRemainFile : FILE=%s OPEN FAIL (%s)", LT, FILE_CIAPP_REMAIN, strerror(errno));
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
