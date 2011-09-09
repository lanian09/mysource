/******************************************************************************* 
        @file   sctp_main.c
 *      - A_SCTP 프로세스를 초기화 하는 함수들
 *
 *      Copyright (c) 2006~ by Upresto Inc. Korea
 *      All rights reserved
 *
 *      $Id: sctp_main.c,v 1.2 2011/09/06 02:07:44 dcham Exp $
 *
 *      @Author     $Author: dcham $
 *      @version    $Revision: 1.2 $
 *      @date       $Date: 2011/09/06 02:07:44 $
 *      @ref        sctp_main.c
 *
 *      @section    Intro(소개)
 *      - A_SCTP 메인 프로세스
 *
 *      @section    Requirement
 *
*******************************************************************************/

/* INCLUDE ********************************************************************/

/* SYS HEADER */
/* LIB HEADER */
#include "commdef.h"
#include "mems.h"
#include "cifo.h"
#include "gifo.h"
#include "nifo.h"
#include "loglib.h"
#include "verlib.h"
#include "Analyze_Ext_Abs.h"
/* PRO HEADER */
#include "common_stg.h"
#include "procid.h"
#include "sshmid.h"
#include "capdef.h"
#include "path.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */
#include "sctpstack.h"
#include "sctp_serv.h"
#include "sctp_init.h"

/* VARIABLES ******************************************************************/
int				gdStopFlag;
OFFSET			gdOffset;

stMEMSINFO		*gpMEMSINFO;
stCIFO			*gpCIFO;

/* FUNCTION *******************************************************************/

/*******************************************************************************

*******************************************************************************/
char		vERSION[7] = "R3.0.0";

int main()
{
	int			dRet, i;
	time_t		tCurrentTime, tCheckTime;

	UCHAR				*pNODE;
	Capture_Header_Msg	*pCAPHEAD;
	INFO_ETH			*pINFOETH;


	/* INITIALIZE log_print INFORAMTION */
	log_init(S_SSHM_LOG_LEVEL, getpid(), SEQ_PROC_A_SCTP, LOG_PATH"/A_SCTP", "A_SCTP");

	/* INITIALIZE S_SCTP INIT */
	dRet = dInitSCTP();
	if( dRet < 0 ) {
		log_print( LOGN_CRI, "[ERROR] FAIL IN dInitSCTP() dRet:%d", dRet );
		exit(0);
	}

	/* SET TIME INFO */
	time( &tCurrentTime );
	tCheckTime = tCurrentTime;

	if((dRet = set_version(S_SSHM_VERSION, SEQ_PROC_A_SCTP, vERSION)) < 0 ) {
		log_print(LOGN_CRI, "SET_VERSION ERROR(RET=%d,IDX=%d,VER=%s)", dRet, SEQ_PROC_A_SCTP, vERSION);
	}
	log_print( LOGN_CRI, "## A_SCTP PROCESS START (%s)0##", vERSION);

	/* MAIN WHILE LOOP */
	while( gdStopFlag )
	{
		time( &tCurrentTime );

		for( i=0; i<1000; i++ ) {
			gdOffset = gifo_read(gpMEMSINFO, gpCIFO, SEQ_PROC_A_SCTP);
			if (gdOffset > 0) {

				pCAPHEAD = (Capture_Header_Msg *)nifo_get_value(gpMEMSINFO, CAP_HEADER_NUM, gdOffset);
				pINFOETH = (INFO_ETH *)nifo_get_value(gpMEMSINFO, INFO_ETH_NUM, gdOffset);
				pNODE = nifo_ptr(gpMEMSINFO, gdOffset);
				
				if( pINFOETH == NULL ) {
					log_print( LOGN_CRI, "[%s][%s.%d] ERROR : Received node is NULL ", __FILE__, __FUNCTION__, __LINE__);
					nifo_node_delete(gpMEMSINFO, pNODE);
				} else {
					dRet = dAnalyzeSCTP(pCAPHEAD, pINFOETH, pNODE);
					if( dRet < 0 ) {
						log_print( LOGN_INFO, "ERROR IN dAnalyzeSCTP dRet:%d", dRet );
						nifo_node_delete(gpMEMSINFO, pNODE);
					}
				}
			}
			else {
				usleep(0);
				break;
			}
		}

		/* CHECK FOR TIMEOUT */
		if( tCurrentTime != tCheckTime ) {
		if( tCurrentTime%5 == 0 )
			log_print( LOGN_INFO, "ASSO_CNT:%u, STACK_CNT:%u", 
						   	   pstASSOSTACKTbl->stAssoTbl.uiAssoCount,
						   	   pstASSOSTACKTbl->stStackTbl.uiCurrCount );
			tCheckTime = tCurrentTime;
		}
	}

	FinishProgram();

	return 0;
}

/*
* $Log: sctp_main.c,v $
* Revision 1.2  2011/09/06 02:07:44  dcham
* *** empty log message ***
*
* Revision 1.1.1.1  2011/08/29 05:56:42  dcham
* NEW OAM SYSTEM
*
* Revision 1.4  2011/08/21 09:07:52  hhbaek
* Commit TAF/SRC/ *
*
* Revision 1.3  2011/08/17 07:24:32  dcham
* *** empty log message ***
*
* Revision 1.2  2011/08/05 02:38:56  uamyd
* A_SCTP modified
*
* Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
* init DQMS2
*
* Revision 1.4  2011/01/11 04:09:09  uamyd
* modified
*
* Revision 1.1.1.1  2010/08/23 01:12:59  uamyd
* DQMS With TOTMON, 2nd-import
*
* Revision 1.3  2009/07/15 17:10:56  dqms
* set_version 위치 및 Plastform Type 변경
*
* Revision 1.2  2009/05/27 14:24:48  dqms
* *** empty log message ***
*
* Revision 1.1  2009/05/27 07:38:13  dqms
* *** empty log message ***
*
* Revision 1.1  2009/05/13 11:38:41  upst_cvs
* NEW
*
* Revision 1.3  2008/03/21 13:36:21  uamyd
* *** empty log message ***
*
* Revision 1.2  2008/01/15 14:50:31  dark264sh
* add version infomation
*
* Revision 1.1  2008/01/11 12:09:08  pkg
* import two-step by uamyd
*
* Revision 1.4  2007/06/01 11:07:08  doit1972
* MODIFY LOG INFO
*
* Revision 1.3  2007/05/30 02:49:26  doit1972
* MODIFY NIFO NODE DELETE POSITION
*
* Revision 1.2  2007/05/10 02:21:50  doit1972
* *** empty log message ***
*
* Revision 1.1  2007/05/04 00:43:18  doit1972
* NEW FILE
*
*/
