/**		@file	wipi_init.c
 * 		- A_WIPI 프로세스를 초기화 하는 함수들
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: wipi_init.c,v 1.3 2011/09/06 12:46:41 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.3 $
 * 		@date		$Date: 2011/09/06 12:46:41 $
 * 		@ref		wipi_init.c wipi_maic.c
 *
 * 		@section	Intro(소개)
 * 		- A_WIPI 프로세스를 초기화 하는 함수들
 *
 * 		@section	Requirement
 *
 **/

/**
 *	Include headers
 */

#include <stdio.h>
#include <errno.h>
#include <signal.h>

// TOP
#include "commdef.h"
#include "procid.h"
#include "common_stg.h"
#include "filter.h"	/* st_Flt_Info */
#include "path.h"
#include "sshmid.h"

// LIB
#include "mems.h"
#include "cifo.h"
#include "gifo.h"
#include "nifo.h"
#include "loglib.h"
#include "filelib.h"
#include "ipclib.h"

// .
#include "wipi_init.h"

/**
 *	Declare var.
 */
extern stCIFO		*gpCIFO;
extern S32			giFinishSignal;
extern S32			giStopFlag;

st_Flt_Info			*flt_info;
int					gACALLCnt = 0;

/**
 *	Implement func.
 */

/** dInitWIPI function.
 *
 *  dInitWIPI Function
 *
 *  @return			S32
 *  @see			wipi_init.c wipi_main.c
 *
 **/
S32 dInitWIPI(stMEMSINFO **pMEMSINFO, stHASHGINFO **pLWIPIHASH)
{
	/* Setup Signal */
	SetUpSignal();

	if((*pMEMSINFO = nifo_init_zone((U8*)"A_WIPI", SEQ_PROC_A_WIPI, FILE_NIFO_ZONE)) == NULL) {
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

	if( shm_init(S_SSHM_FLT_INFO, DEF_FLT_INFO_SIZE, (void**)&flt_info) < 0 ){
        log_print(LOGN_CRI, "FAILED IN shm_init(FLT_INFO=%d)", S_SSHM_FLT_INFO);
        return -3;
    }

	if((*pLWIPIHASH = hashg_init(0, TAG_KEY_LWIPI_CONF_SIZE, 0, LWIPI_CONF_SIZE, CONF_CNT)) == NULL) {
		log_print(LOGN_CRI, LH"hashg_init LWIPIHASH NULL", LT);
		return -4;	
	}

	ReadData_WIPI(*pLWIPIHASH);

	return 0;
}

void ReadData_WIPI(stHASHGINFO *pLWIPIHASH)
{
	U32		uiCount;
	int 	start = 0;
	FILE 	*fp;
	char 	buf[BUFSIZ];
	char 	*bufcmp;
	char 	parse[BUFSIZ];
	LWIPI_CONF	LWIPICONF;

	fp = fopen(FILE_PATH_FILTER, "r");
	if(fp == NULL) {
		log_print(LOGN_CRI, LH"OPEN ERROR[%s][%s]", 
			LT, FILE_PATH_FILTER, strerror(errno));
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

//				log_print(LOGN_INFO, "원문 %d -> %s", strlen(parse), parse);
				bufcmp = &buf[2];
				if( ! strncmp(bufcmp,"LWIPI",strlen("LWIPI")) ){
					if(pLWIPIHASH != NULL) {
						memset(&LWIPICONF, 0x00, LWIPI_CONF_SIZE);
						LWIPI_CONF_GRASP_LEX(parse, strlen(parse), (char *)&LWIPICONF);
						hashg_add(pLWIPIHASH, (U8 *)&LWIPICONF, (U8 *)&LWIPICONF);
						// hashg_print_all("AAA",pLWIPIHASH);
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

	uiCount = hashg_get_occupied_node_count(pLWIPIHASH);
	log_print(LOGN_CRI, "READDATA WIPI HASHTABLE CNT=%u MAX=%u USE=%.lf%%, NODE ALLOC=%u MAX=%u, AvgLinked=%.02lf",
		uiCount, pLWIPIHASH->uiHashSize, ((double)uiCount / (double)pLWIPIHASH->uiHashSize) * 100,
		pLWIPIHASH->uiLinkedCnt, pLWIPIHASH->uiHashSize * 10, (double)pLWIPIHASH->uiLinkedCnt / (double)uiCount);
}

/** SetUpSignal function.
 *
 *  SetUpSignal Function
 *
 *  @return			void
 *  @see			wipi_init.c wipi_main.c
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
 *  @see			wipi_init.c wipi_main.c
 *
 **/
void UserControlledSignal(S32 isign)
{
	giStopFlag = 0;
	giFinishSignal = isign;
	log_print(LOGN_CRI, "User Controlled Signal Req = %d", isign);
}

/** FinishProgram function.
 *
 *  FinishProgram Function
 *
 *  @return			void
 *  @see			wipi_init.c wipi_main.c
 *
 **/
void FinishProgram(void)
{
	log_print(LOGN_CRI, "PROGRAM IS NORMALLY TERMINATED, Cause = %d", giFinishSignal);
	exit(0);
}

/** IgnoreSignal function.
 *
 *  IgnoreSignal Function
 *
 *	@param	isign	:	signal
 *
 *  @return			void
 *  @see			wipi_init.c wipi_main.c
 *
 **/
void IgnoreSignal(S32 isign)
{
	if (isign != SIGALRM)
	{
		log_print(LOGN_CRI, "UNWANTED SIGNAL IS RECEIVED, signal = %d", isign);
	}

	signal(isign, IgnoreSignal);
}

/*
 * $Log: wipi_init.c,v $
 * Revision 1.3  2011/09/06 12:46:41  hhbaek
 * *** empty log message ***
 *
 * Revision 1.2  2011/09/05 01:35:33  uamyd
 * modified to runnable source
 *
 * Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * NEW OAM SYSTEM
 *
 * Revision 1.4  2011/08/21 09:07:53  hhbaek
 * Commit TAF/SRC/ *
 *
 * Revision 1.3  2011/08/17 07:27:42  dcham
 * *** empty log message ***
 *
 * Revision 1.2  2011/08/05 09:04:50  uamyd
 * modified
 *
 * Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 * init DQMS2
 *
 * Revision 1.4  2011/05/09 15:24:41  jsyoon
 * *** empty log message ***
 *
 * Revision 1.3  2011/01/11 04:09:11  uamyd
 * modified
 *
 * Revision 1.1.1.1  2010/08/23 01:12:58  uamyd
 * DQMS With TOTMON, 2nd-import
 *
 * Revision 1.2  2009/06/29 13:27:52  dark264sh
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2009/05/26 02:14:27  dqms
 * Init TAF_RPPI
 *
 * Revision 1.1.1.1  2008/06/09 08:17:17  jsyoon
 * WATAS3 PROJECT START
 *
 * Revision 1.3  2007/08/29 12:44:55  dark264sh
 * *** empty log message ***
 *
 * Revision 1.2  2007/08/29 09:25:14  dark264sh
 * *** empty log message ***
 *
 * Revision 1.1  2007/08/21 12:55:11  dark264sh
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
 * Revision 1.6  2006/11/08 07:29:43  shlee
 * CONF관련 hasho -> hashg로 변경 및 CONF_CNT 101 CONF_PREA_CNT 811로 변경
 *
 * Revision 1.5  2006/11/07 08:51:11  shlee
 * hasho_init function
 *
 * Revision 1.4  2006/10/20 10:03:39  dark264sh
 * *** empty log message ***
 *
 * Revision 1.3  2006/10/18 08:53:31  dark264sh
 * nifo debug 코드 추가
 *
 * Revision 1.2  2006/10/13 07:03:51  dark264sh
 * filter PATH define으로 변경
 *
 * Revision 1.1  2006/10/12 14:31:13  shlee
 * INIT
 *
 */
