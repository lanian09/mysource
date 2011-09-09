/**		@file	o_svcmon_init.c
 * 		- O_SVCMON 프로세스를 초기화 하는 함수들
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: o_svcmon_init.c,v 1.4 2011/09/05 12:52:56 uamyd Exp $
 *
 * 		@Author		$Author: uamyd $
 * 		@version	$Revision: 1.4 $
 * 		@date		$Date: 2011/09/05 12:52:56 $
 * 		@ref		o_svcmon_init.c o_svcmon_maic.c
 *
 * 		@section	Intro(소개)
 * 		- O_SVCMON 프로세스를 초기화 하는 함수들
 *
 * 		@section	Requirement
 *
 **/

#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "typedef.h"
#include "loglib.h"
#include "ipclib.h"

#include "path.h"
#include "sshmid.h"

#include "watch_filter.h"
#include "filter.h"
#include "svcmon.h"

#include "o_svcmon_init.h"
#include "o_svcmon_conf.h"

extern S32			giFinishSignal;
extern S32			giStopFlag;

extern S32			dMyQID;
extern S32			dMONDQID;
extern S32			dALMDQID;
extern S32			dMSVCMONQID;
extern S32			dCONDQID;

extern st_WatchFilter	*gWatchFilter;
extern st_MonTotal		*gMonTotal;
extern st_MonTotal_1Min *gMonTotal1Min;

/** dInitMLOG function.
 *
 *  dInitMLOG Function
 *
 *  @return			S32
 *  @see			o_svcmon_init.c o_svcmon_main.c
 *
 **/
S32 dInitOSVCMON(stHASHOINFO **pDefHash, stHASHOINFO **pAMonHash, stHASHOINFO **pSMonHash, stHASHOINFO **pThresHash, stHASHOINFO **pNasIPHash)
{
	S32				dRet;

	/* Setup Signal */
	SetUpSignal();

	if( (dRet = shm_init(S_SSHM_WATCH_FILTER, sizeof(st_WatchFilter), (void**)&gWatchFilter)) < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN shm_init(WATCH_FILTER=0x%x)"EH, LT, S_SSHM_WATCH_FILTER, ET);
		return -1; 
	}

	if( (dRet = shm_init(S_SSHM_MON_TOTAL, sizeof(st_MonTotal), (void**)&gMonTotal)) < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN shm_init(MON_TOTAL=0x%x)"EH, LT, S_SSHM_MON_TOTAL, ET);
		return -2; 
	}

	if( (dRet = shm_init(S_SSHM_MON_TOTAL_1MIN, sizeof(st_MonTotal_1Min), (void**)&gMonTotal1Min)) < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN shm_init(MON_TOTAL_1MIN=0x%x)"EH, LT, S_SSHM_MON_TOTAL_1MIN, ET);
		return -3; 
	}

	if ((dRet = dGetSYSCFG()) < 0) {
		log_print(LOGN_CRI, LH"dGetSYSCFG Fail. RET[%d]", LT, dRet);
		return -4;
	}

	if((*pAMonHash = hasho_init(0, DEF_SVCMONHASH_KEY_SIZE, DEF_SVCMONHASH_KEY_SIZE, DEF_SVCMONHASH_DATA_SIZE, MAX_SVCMONHASH_SIZE, 0)) == NULL) {
		log_print(LOGN_CRI, LH"hasho_init NULL", LT);
		return -5;
	}

	if((*pSMonHash = hasho_init(0, DEF_SVCMONHASH_KEY_SIZE, DEF_SVCMONHASH_KEY_SIZE, DEF_SVCMONHASH_DATA_SIZE, MAX_SVCMONHASH_SIZE, 0)) == NULL) {
		log_print(LOGN_CRI, LH"hasho_init NULL", LT);
		return -6;
	}

	if((*pThresHash = hasho_init(0, DEF_THRESHASH_KEY_SIZE, DEF_THRESHASH_KEY_SIZE, DEF_THRESHASH_DATA_SIZE, MAX_THRESHASH_SIZE, 0)) == NULL) {
		log_print(LOGN_CRI, LH"hasho_init NULL", LT);
		return -7;
	}

	if((*pDefHash = hasho_init(0, DEF_DEFHASH_KEY_SIZE, DEF_DEFHASH_KEY_SIZE, DEF_DEFHASH_DATA_SIZE, MAX_DEFHASH_SIZE, 0)) == NULL) {
		log_print(LOGN_CRI, LH"hasho_init NULL", LT);
		return -8;
	}

	if((dRet = dReadDefData(*pDefHash)) < 0) {
		log_print(LOGN_CRI, LH"dReadDefData dRet=%d", LT, dRet);
		return -9;
	}

	if((*pNasIPHash = hasho_init(0, DEF_NASIPHASH_KEY_SIZE, DEF_NASIPHASH_KEY_SIZE, DEF_NASIPHASH_DATA_SIZE, MAX_NASIPHASH_SIZE, 0)) == NULL) {
		log_print(LOGN_CRI, LH"hasho_init NULL", LT);
		return -10;
	}

	return 0;
}

/** SetUpSignal function.
 *
 *  SetUpSignal Function
 *
 *  @return			void
 *  @see			o_svcmon_init.c o_svcmon_main.c
 *
 **/
void SetUpSignal(void)
{
	giStopFlag = 1;

	/* WANTED SIGNALS */
	signal(SIGTERM, UserControlledSignal);
	signal(SIGINT, UserControlledSignal);
	signal(SIGQUIT, UserControlledSignal);

	/* UNWANTED SIGNALS */
	signal(SIGHUP, IgnoreSignal);
	signal(SIGALRM, IgnoreSignal);
	signal(SIGPIPE, IgnoreSignal);
	signal(SIGPOLL, IgnoreSignal);
	signal(SIGPROF, IgnoreSignal);
	signal(SIGUSR1, IgnoreSignal);
	signal(SIGUSR2, IgnoreSignal);
	signal(SIGVTALRM, IgnoreSignal);
	signal(SIGCLD, IgnoreSignal);
}

/** UserControlledSignal function.
 *
 *  UserControlledSignal Function
 *
 *	@param	isign	:	signal
 *
 *  @return			void
 *  @see			o_svcmon_init.c o_svcmon_main.c
 *
 **/
void UserControlledSignal(S32 isign)
{
	giStopFlag = 0;
	giFinishSignal = isign;
	log_print(LOGN_CRI, "############## User Controlled Signal Req = %d", isign);
}

/** FinishProgram function.
 *
 *  FinishProgram Function
 *
 *  @return			void
 *  @see			o_svcmon_init.c o_svcmon_main.c
 *
 **/
void FinishProgram(void)
{
	log_print(LOGN_CRI, "############### PROGRAM IS NORMALLY TERMINATED, Cause = %d", giFinishSignal);
}

/** IgnoreSignal function.
 *
 *  IgnoreSignal Function
 *
 *	@param	isign	:	signal
 *
 *  @return			void
 *  @see			o_svcmon_init.c o_svcmon_main.c
 *
 **/
void IgnoreSignal(S32 isign)
{
	if (isign != SIGALRM)
		log_print(LOGN_CRI, "############# UNWANTED SIGNAL IS RECEIVED, signal = %d", isign);

	signal(isign, IgnoreSignal);
}

S32 dReadDefData(stHASHOINFO *pDefHash)
{
    int     dCnt, dLine, i;
    FILE    *fa;
	char	seps[] = ",";
	char	*token;
    char    szBuf[BUFSIZ];
    char    szDesc[BUFSIZ];
	st_DefHash_Key		stDefHashKey;
	st_DefHash_Key		*pKey = &stDefHashKey;
	st_DefHash_Data		stDefHashData;
	st_DefHash_Data		*pData = &stDefHashData;
	stHASHONODE			*pHASHNODE;

    fa = fopen(FILE_DEFECT_CODE, "r");
    if(fa == NULL) {
        log_print(LOGN_CRI, "dReadDefData : FILE=%s OPEN FAIL (%s)", FILE_DEFECT_CODE, strerror(errno));
        return -1;
    }
    
    dCnt = 0; dLine = 1;
    while(fgets(szBuf,1024,fa) != NULL)
    {
        if(szBuf[0] != '#') {
            log_print(LOGN_CRI, "dReadDefData : File=%s:%d ROW FORMAT ERR", FILE_DEFECT_CODE, dLine);
            fclose(fa);
            return -2;
        }

        if(szBuf[1] == '#')
            continue;
        else if(szBuf[1] == 'E')
            break;
        else if(szBuf[1] == '@') {

			i = 0;
			token = strtok(&szBuf[2], seps);
			while(token != NULL)
			{
				switch(i)
				{
				case 0:	pData->uiArrayIndex = atoi(token);		break;
				case 1:	pKey->uiDefectCode = atoi(token);		break;
				case 2:	strcpy(szDesc, token);	break;
				default:										break;
				}
				token = strtok(NULL, seps);
				i++;
			}

			log_print(LOGN_CRI, "DEFECT_CODE INFO POS=%d INDEX=%u DEFECTCODE=%u DESC=%.*s", 
					dLine, pData->uiArrayIndex, pKey->uiDefectCode, BUFSIZ, szDesc);

			if((pHASHNODE = hasho_add(pDefHash, (U8 *)pKey, (U8 *)pData)) == NULL) {
               	log_print(LOGN_CRI, LH"hasho_add NULL DEFECT_CODE INFO POS=%d INDEX=%u DEFECTCODE=%u DESC=%.*s",
                   	LT, dLine, pData->uiArrayIndex, pKey->uiDefectCode, BUFSIZ, szDesc);
				return -3;
			}

			dCnt++;
        }
        dLine++;
    }

    fclose(fa);

	return 0;
}





/*
 * $Log: o_svcmon_init.c,v $
 * Revision 1.4  2011/09/05 12:52:56  uamyd
 * modified
 *
 * Revision 1.3  2011/09/05 05:33:00  hhbaek
 * *** empty log message ***
 *
 * Revision 1.2  2011/08/31 16:08:08  dhkim
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * NEW OAM SYSTEM
 *
 * Revision 1.1  2011/08/23 08:58:48  dcham
 * *** empty log message ***
 *
 * Revision 1.11  2011/06/23 00:58:18  innaei
 * *** empty log message ***
 *
 * Revision 1.10  2011/04/20 06:20:41  innaei
 * *** empty log message ***
 *
 * Revision 1.9  2011/04/17 12:36:52  innaei
 * *** empty log message ***
 *
 * Revision 1.8  2011/01/11 04:09:17  uamyd
 * modified
 *
 * Revision 1.2  2010/11/14 10:22:44  jwkim96
 * STP 작업 내용 반영.
 *
 * Revision 1.1.1.1  2010/08/23 01:13:11  uamyd
 * DQMS With TOTMON, 2nd-import
 *
 * Revision 1.7  2010/03/04 06:00:53  dark264sh
 * ROAM NASIP NetMask 처리
 *
 * Revision 1.6  2009/08/22 07:41:08  pkg
 * *** empty log message ***
 *
 * Revision 1.5  2009/06/20 15:51:23  dark264sh
 * *** empty log message ***
 *
 * Revision 1.4  2009/06/20 10:51:39  dark264sh
 * *** empty log message ***
 *
 * Revision 1.3  2009/06/18 17:01:48  dark264sh
 * O_SVCMON Filter 처리
 *
 * Revision 1.2  2009/06/17 11:05:35  dark264sh
 * O_SVCMON, M_SVCMON MsgQ 사용 변경
 *
 * Revision 1.1  2009/06/15 08:06:04  dark264sh
 * O_SVCMON 기본 동작 처리
 *
 */
