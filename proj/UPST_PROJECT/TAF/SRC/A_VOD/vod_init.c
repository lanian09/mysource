/*******************************************************************************		
 *		@file	vod_init.c
 * 		- A_VOD 프로세스를 초기화 하는 함수들
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: vod_init.c,v 1.2 2011/09/06 12:46:39 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.2 $
 * 		@date		$Date: 2011/09/06 12:46:39 $
 * 		@ref		vod_init.c vod_maic.c
 *
 * 		@section	Intro(소개)
 * 		- A_VOD 프로세스를 초기화 하는 함수들
 *
 * 		@section	Requirement
 *
*******************************************************************************/


/* INCLUDE ********************************************************************/
#include <stdio.h>
#include <errno.h>
#include <signal.h>

// TOP
#include "commdef.h"
#include "procid.h"
#include "sshmid.h"
#include "common_stg.h"
#include "path.h"

// LIB
#include "mems.h"
#include "cifo.h"
#include "gifo.h"
#include "nifo.h"
#include "loglib.h"
#include "filelib.h"

// .
#include "vod_sess.h"
#include "vod_init.h"

/**
 *	Declare var.
 */
extern stCIFO		*gpCIFO;
extern S32			giFinishSignal;
extern S32			giStopFlag;

int					gACALLCnt = 0;

/**
 *	Implement func.
 */

/******************************************************************************* 
 *	dInitVOD function.
 *  @return			S32
 *  @see			vod_init.c vod_main.c
*******************************************************************************/
S32 dInitVOD( stMEMSINFO **pMEMSINFO, stHASHGINFO **pLVODHASH, stHASHGINFO **pMENUTITLE, 
			  stTIMERNINFO **pTIMERNINFO, stHASHOINFO **pstVODSESSHASH, stHASHOINFO **pstRTCPSESSHASH )
{
	/* Setup Signal */
	SetUpSignal();

	if((*pMEMSINFO = nifo_init_zone((U8*)"A_VOD", SEQ_PROC_A_VOD, FILE_NIFO_ZONE)) == NULL) {
        log_print(LOGN_CRI, LH"FAILED IN nifo_init NULL", LT);
        return -1;
    }
    
    if((gpCIFO = gifo_init_group(FILE_CIFO_CONF, FILE_GIFO_CONF)) == NULL ){
        log_print(LOGN_CRI, LH"FAILED IN gifo_init_group, cifo=%s, gifo=%s",
            LT, FILE_CIFO_CONF, FILE_GIFO_CONF);
        return -2;
    }

	gACALLCnt = get_block_num(FILE_MC_INIT, "A_CALL");
	log_print(LOGN_INFO, "INIT., A_CALL ProcCount[%d]", gACALLCnt);
	if((*pLVODHASH = hashg_init(0, TAG_KEY_LVOD_CONF_SIZE, 0, LVOD_CONF_SIZE, CONF_CNT)) == NULL) {
		log_print(LOGN_CRI, LH"hashg_init LVODHASH NULL", LT);
		return -3;	
	}

	if((*pTIMERNINFO = timerN_init(VOD_MENU_TITLE_HASH_SIZE, MENU_TITLE_KEY_SIZE)) == NULL) {
		log_print(LOGN_CRI, LH"timerN_init NULL", LT);
		return -4;
	}

	if((*pMENUTITLE = hashg_init(0, MENU_TITLE_KEY_SIZE, 0, MENU_TITLE_DATA_SIZE, VOD_MENU_TITLE_HASH_SIZE)) == NULL) {
		log_print(LOGN_CRI, LH"hasho_init MENU_TITLE NULL", LT);
		return -5;	
	}

	if((*pstVODSESSHASH = hasho_init(S_SSHM_VODSESS, DEF_VODSESSKEY_SIZE, DEF_VODSESSKEY_SIZE, DEF_VODSESS_SIZE, DEF_VODSESS_CNT, 0)) == NULL) {
		log_print(LOGN_CRI, LH"hasho_init MENU_TITLE NULL", LT);
		return -6;
	}

	if((*pstRTCPSESSHASH = hasho_init(S_SSHM_RTCPSESS, DEF_RTCPSESSKEY_SIZE, DEF_RTCPSESSKEY_SIZE, DEF_RTCPSESS_SIZE, DEF_RTCPSESS_CNT, 0)) == NULL) {
		log_print(LOGN_CRI, LH"hasho_init MENU_TITLE NULL", LT);
		return -7;
	}

	hashg_set_MaxNodeCnt(*pMENUTITLE,VOD_MENU_TITLE_HASH_SIZE);

	ReadData_VOD(*pLVODHASH);

	return 0;
}


/*******************************************************************************
 *  ReadData_VOD function.
 *  @return        	VOID 
 *  @see
*******************************************************************************/
void ReadData_VOD(stHASHGINFO *pLVODHASH)
{
	U32		uiCount;
	int 	start = 0;
	FILE 	*fp;
	char 	buf[BUFSIZ];
	char 	*bufcmp;
	char 	parse[BUFSIZ];
	LVOD_CONF	LVODCONF;

	fp = fopen(FILE_PATH_FILTER, "r");
	if(fp == NULL) {
		log_print(LOGN_CRI, LH"OPEN ERROR[%s][%s]", LT, FILE_PATH_FILTER, strerror(errno));
		exit(0);
	}

	while(fgets(buf, BUFSIZ, fp)){
		switch(buf[0]){
		case '(':
			if(start != 0) {
				log_print(LOGN_CRI, LH"FILE Format Error(%s)", LT, parse);
				parse[0] = 0x00;
			}
			start = 1;
			sprintf(parse, "%s", buf);
			break;	
		case ')':
			if(start == 1) {
				sprintf(parse, "%s%s", parse, buf);

				bufcmp = &buf[2];
				if( ! strncmp(bufcmp,"LVOD",strlen("LVOD")) ){
					if(pLVODHASH != NULL) {
						memset(&LVODCONF, 0x00, LVOD_CONF_SIZE);
						LVOD_CONF_GRASP_LEX(parse, strlen(parse), (char*)(U8 *)&LVODCONF);
						hashg_add(pLVODHASH, (U8 *)&LVODCONF, (U8 *)&LVODCONF);
						// hasho_print_all("AAA",pLVODHASH);
					}
				}
				start = 0;
			} else {
				log_print(LOGN_CRI, LH"FILE Format Error(%s)", LT, buf);
			}
			break;
		default:
			if(start == 1) {
				sprintf(parse, "%s%s", parse, buf);
			}
		}
	}

	fclose(fp);
	uiCount = hashg_get_occupied_node_count(pLVODHASH);
	log_print(LOGN_CRI, "READDATA VOD HASHTABLE CNT=%u MAX=%u USE=%.lf%%, NODE ALLOC=%u MAX=%u, AvgLinked=%.02lf",
		uiCount, pLVODHASH->uiHashSize, ((double)uiCount / (double)pLVODHASH->uiHashSize) * 100,
		pLVODHASH->uiLinkedCnt, pLVODHASH->uiHashSize * 10, (double)pLVODHASH->uiLinkedCnt / (double)uiCount);
}



/******************************************************************************* 
 *	SetUpSignal function.
 *  @return			void
 *  @see			vod_init.c vod_main.c
*******************************************************************************/
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



/******************************************************************************* 
 *	UserControlledSignal function.
 *	@param	isign	:	signal
 *  @return			void
 *  @see			vod_init.c vod_main.c
*******************************************************************************/
void UserControlledSignal(S32 isign)
{
	giStopFlag = 0;
	giFinishSignal = isign;
	log_print(LOGN_CRI, "User Controlled Signal Req = %d", isign);
}



/******************************************************************************* 
 *	FinishProgram function.
 *  @return			void
 *  @see			vod_init.c vod_main.c
*******************************************************************************/
void FinishProgram(void)
{
	log_print(LOGN_CRI, "PROGRAM IS NORMALLY TERMINATED, Cause = %d", giFinishSignal);
	exit(0);
}



/******************************************************************************* 
 * 	IgnoreSignal function.
 *	@param	isign	:	signal
 *  @return			void
 *  @see			vod_init.c vod_main.c
*******************************************************************************/
void IgnoreSignal(S32 isign)
{
	if (isign != SIGALRM)
	{
		log_print(LOGN_CRI, "UNWANTED SIGNAL IS RECEIVED, signal = %d", isign);
	}

	signal(isign, IgnoreSignal);
}

/*
 * $Log: vod_init.c,v $
 * Revision 1.2  2011/09/06 12:46:39  hhbaek
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * NEW OAM SYSTEM
 *
 * Revision 1.4  2011/08/21 09:07:53  hhbaek
 * Commit TAF/SRC/ *
 *
 * Revision 1.3  2011/08/17 07:26:30  dcham
 * *** empty log message ***
 *
 * Revision 1.2  2011/08/08 11:05:43  uamyd
 * modified block added
 *
 * Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 * init DQMS2
 *
 * Revision 1.4  2011/05/09 15:23:29  jsyoon
 * *** empty log message ***
 *
 * Revision 1.3  2011/01/11 04:09:10  uamyd
 * modified
 *
 * Revision 1.1.1.1  2010/08/23 01:12:59  uamyd
 * DQMS With TOTMON, 2nd-import
 *
 * Revision 1.2  2009/06/29 13:26:26  dark264sh
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2009/05/26 02:14:28  dqms
 * Init TAF_RPPI
 *
 * Revision 1.1.1.1  2008/06/09 08:17:16  jsyoon
 * WATAS3 PROJECT START
 *
 * Revision 1.3  2007/09/04 11:28:10  watas
 * *** empty log message ***
 *
 * Revision 1.2  2007/09/03 09:35:05  watas
 * *** empty log message ***
 *
 * Revision 1.1  2007/08/21 12:54:39  dark264sh
 * no message
 *
 * Revision 1.10  2006/11/28 12:58:27  cjlee
 * doxygen
 *
 * Revision 1.9  2006/11/17 03:37:42  cjlee
 * 내용  : Set_FirstUrlList 적용
 *     hash안에 메뉴명을 미리 넣어두어야 할 것들
 *
 * Revision 1.8  2006/11/16 06:02:09  cjlee
 * *** empty log message ***
 *
 * Revision 1.7  2006/11/16 05:18:52  cjlee
 * 내용  : MENU관련 변경
 * 	- INC/ *.h  : init함수의 extern 선언 변경
 * 	- BODY.stc
 * 	   parsing rule 변경
 * 	   url="  http:// 이런 식도 처리 가능
 * 	   <a ...  href=.. 등도 처리 가능
 * 	   hashg를 이용한 add 추가 (href에 대해서)
 * 	   관련 함수 추가
 * 	- aqua.pstg
 * 	   MENUTITLE관련 structure및 deifne 추가
 * 	- A_BREW , A_MEKUN , A_WIPI , A_VOD
 * 	  *init , *main : MENU관련 처리 추가
 * 	  URL을 받은후에 hash에서 비교를 하여 적당한 메뉴명을 넣는다.
 *
 * Revision 1.6  2006/11/08 07:32:04  shlee
 * CONF관련 hasho -> hashg로 변경 및 CONF_CNT 101 CONF_PREA_CNT 811로 변경
 *
 * Revision 1.5  2006/11/07 08:53:15  shlee
 * hasho_init function
 *
 * Revision 1.4  2006/10/20 10:03:10  dark264sh
 * *** empty log message ***
 *
 * Revision 1.3  2006/10/18 08:53:31  dark264sh
 * nifo debug 코드 추가
 *
 * Revision 1.2  2006/10/13 07:03:51  dark264sh
 * filter PATH define으로 변경
 *
 * Revision 1.1  2006/10/12 15:16:41  shlee
 * INIT
 *
 */
