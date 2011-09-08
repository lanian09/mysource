/**		@file	mlog_init.c
 * 		- M_LOG 프로세스를 초기화 하는 함수들
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: mlog_init.c,v 1.2 2011/08/31 13:44:16 dcham Exp $
 *
 * 		@Author		$Author: dcham $
 * 		@version	$Revision: 1.2 $
 * 		@date		$Date: 2011/08/31 13:44:16 $
 * 		@ref		mlog_init.c mlog_maic.c
 *
 * 		@section	Intro(소개)
 * 		- M_LOG 프로세스를 초기화 하는 함수들
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
#include "path.h"
#include "rppi_def.h"
#include "mems.h"
#include "hasho.h"
#include "loglib.h"
#include "mlog_init.h"

extern stHASHOINFO   *pDEFECTINFO;
extern char			gsLocCode[MAX_LOCCODE_LEN];
extern S32 			gdWEBFlag;				/* 외부인터넷로그 생성 플래그 */
extern S32          gdSysNo;
extern S32          giStopFlag;
extern char         gsLocCode[MAX_LOCCODE_LEN];
extern S32          giFinishSignal;

/*
extern S32			giFinishSignal;
extern S32			giStopFlag;

extern S32			dMyQID;
extern S32			dSIDBQID;

extern U64			nifo_create;
extern U64			nifo_del;

extern S32			gdSysNo;
extern char			gsLocCode[MAX_LOCCODE_LEN];
*/

/** dInitMLOG function.
 *
 *  dInitMLOG Function
 *
 *  @return			S32
 *  @see			mlog_init.c mlog_main.c
 *
 **/
extern int dCloseFileMng(pFILEMNG pstFILEMNG);

S32 dInitMLOG(stMEMSINFO **pMEMSINFO)
{
	/* Setup Signal */
	SetUpSignal();

#ifdef ENABLE_ANALYZE_DIAMETER /* added by uamyd 20100929. hasho 에서 threshold 값을 읽어 오도록 해야함 */
	/* Defect Code 관리 Hash Table */
    if((pDEFECTINFO = hasho_init( 0, sizeof(unsigned int), sizeof(unsigned int),
        HDATA_DEFECT_SIZE, HASH_DEFECTINFO_CNT, 0)) == NULL) {
        log_print(LOGN_CRI, "[%s][%s.%d] hasho_init NULL", __FILE__, __FUNCTION__, __LINE__);
        return -10;
    }

#endif /* ENABLE_ANALYZE_DIAMETER */

	/*
	dRet = dProRemainDataFile();
	if(dRet < 0)
		log_print(LOGN_CRI, "ProRemainDataFile Err=[%d]", dRet);
	*/

	return 0;
}

S32 dGetSYSCFG()
{
	FILE *fa;
	char szBuf[1024], szType[64], szSubType[64], szInfo[64];
	int i, isFound = 0, dRet = 0;


	fa = fopen(FILE_SYS_CONFIG, "r");
	if(fa == NULL) {
		log_print(LOGN_CRI, "%s:%s:%d][LOAD SYSTEM CONFIG : %s FILE OPEN FAIL (%s)", __FILE__, __FUNCTION__, __LINE__, FILE_SYS_CONFIG, strerror(errno));
		return -1;
	}

	i = 0;
	while(fgets(szBuf, 1024, fa) != NULL)
	{
		if(szBuf[0] != '#') 
		{
			log_print(LOGN_WARN,"FAILED IN dGetSYSCFG() : %s File [%d] row format error", FILE_SYS_CONFIG, i);
		}

		i++;

		if(szBuf[1] == '#') continue;
		else if(szBuf[1] == 'E') break;
		else if(szBuf[1] == '@')
		{
			if( (dRet = sscanf(&szBuf[2], "%s %s %s", szType, szSubType, szInfo)) > 0 ) 
			{
				switch(dRet){
					case 2:
						if(strcmp(szType, "LOC-CODE") == 0){
							strncpy(gsLocCode, szSubType, MAX_LOCCODE_LEN);
							log_print(LOGN_CRI, "LOAD LOCATION CODE : [%s]", gsLocCode);
							isFound |= IS_FOUND_LOCCODE;
						} else if(strcmp(szType, "WEBLOG") == 0){
							if( strcmp( szSubType, "ON" ) == 0 || strcmp( szSubType, "on" ) == 0 ) {
								gdWEBFlag = DEF_WEBLOG_ON;
							} else {
								gdWEBFlag = DEF_WEBLOG_OFF;
							}
							log_print(LOGN_CRI, "LOAD WEBLOG FLAG : [%d]", gdWEBFlag);
							isFound |= IS_FOUND_WEBLOG;
						} 
						break;
					case 3:
						if((strcmp(szType, "SYS") == 0) && (strcmp(szSubType, "NO") == 0)){
							gdSysNo= atoi(szInfo);
							log_print(LOGN_CRI, "LOAD SYSNO : [%d]", gdSysNo);
							isFound |= IS_FOUND_SYSNO;
						}
						break;
					default:
						break;
				}
			}
		}
	}/* while */

	fclose(fa);

	if( (isFound & IS_FOUND_LOCCODE) != IS_FOUND_LOCCODE){
		log_print(LOGN_CRI,"%s:%s:%d][LOC-CODE is NOT FOUND at File=%s", __FILE__, __FUNCTION__, __LINE__, FILE_SYS_CONFIG);
		return -2;
	}

	if( (isFound & IS_FOUND_SYSNO) != IS_FOUND_SYSNO ){
		log_print(LOGN_CRI,"%s:%s:%d][SYS NO(SYSNO) is NOT FOUND at File=%s", __FILE__, __FUNCTION__, __LINE__, FILE_SYS_CONFIG);
		return -3;
	}


	return 0;
}

/** SetUpSignal function.
 *
 *  SetUpSignal Function
 *
 *  @return			void
 *  @see			mlog_init.c mlog_main.c
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
 *  @see			mlog_init.c mlog_main.c
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
 *  @see			mlog_init.c mlog_main.c
 *
 **/
void FinishProgram(pFILEMNG	pstFILEMNG)
{
#ifdef MEM_TEST
    log_print(LOGN_CRI, "############### CREATE CNT[%llu] DELETE CNT[%llu]", nifo_create, nifo_del);
#endif
	dCloseFileMng(pstFILEMNG);
	log_print(LOGN_CRI, "############### PROGRAM IS NORMALLY TERMINATED, Cause = %d", giFinishSignal);
}

/** IgnoreSignal function.
 *
 *  IgnoreSignal Function
 *
 *	@param	isign	:	signal
 *
 *  @return			void
 *  @see			mlog_init.c mlog_main.c
 *
 **/
void IgnoreSignal(S32 isign)
{
	if (isign != SIGALRM)
		log_print(LOGN_CRI, "############# UNWANTED SIGNAL IS RECEIVED, signal = %d", isign);

	signal(isign, IgnoreSignal);
}

/*
 * $Log: mlog_init.c,v $
 * Revision 1.2  2011/08/31 13:44:16  dcham
 * *** empty log message ***
 *
 * Revision 1.1  2011/08/31 13:13:41  dcham
 * M_LOG added
 *
 * Revision 1.8  2011/02/15 07:41:14  jsyoon
 * *** empty log message ***
 *
 * Revision 1.7  2011/02/15 02:54:55  jsyoon
 * *** empty log message ***
 *
 * Revision 1.6  2011/01/11 04:09:17  uamyd
 * modified
 *
 * Revision 1.4  2010/09/29 06:50:05  uamyd
 * added lastFailReason to LOG_DIAMETER
 *
 * Revision 1.3  2010/09/27 12:17:12  dqms
 * TAM_DB protocol value set method changed
 *
 * Revision 1.2  2010/09/17 08:45:03  dqms
 * added code to apply with file-readed-system no
 *
 * Revision 1.1.1.1  2010/08/23 01:13:11  uamyd
 * DQMS With TOTMON, 2nd-import
 *
 * Revision 1.5  2009/10/19 08:51:30  pkg
 * nifo_init => nifo_init_tam 변경
 *
 * Revision 1.4  2009/06/29 08:59:28  dark264sh
 * *** empty log message ***
 *
 * Revision 1.3  2009/06/20 13:22:33  dark264sh
 * *** empty log message ***
 *
 * Revision 1.2  2009/06/12 01:12:49  dark264sh
 * M_LOG file name 전송 처리
 *
 * Revision 1.1  2009/06/10 23:52:59  dark264sh
 * *** empty log message ***
 *
 */
