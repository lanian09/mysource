/**		@file	http_main.c
 * 		- HTTP Transaction을 관리 하는 프로세스
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: ihttp_main.c,v 1.2 2011/09/04 11:40:36 dhkim Exp $
 *
 * 		@Author		$Author: dhkim $
 * 		@version	$Revision: 1.2 $
 * 		@date		$Date: 2011/09/04 11:40:36 $
 * 		@warning	.
 * 		@ref		http_main.c l4.h http_init.c http_func.c
 * 		@todo		library가 만들어지지 않은 상태, library 생성 이후 함수 대치
 *
 * 		@section	Intro(소개)
 * 		- HTTP Transaction을 관리 하는 프로세스
 *
 * 		@section	Requirement
 * 		 @li library 생성 이후 함수 대치
 *
 **/

#include <unistd.h>

// LIB
#include "typedef.h"
#include "loglib.h"
#include "verlib.h"
#include "mems.h"
#include "nifo.h"
#include "gifo.h"
#include "cifo.h"
#include "hasho.h"

// PROJECT
#include "path.h"
#include "sshmid.h"
#include "procid.h"
#include "common_stg.h"

// .
#include "ihttp_init.h"
#include "ihttp_func.h"


/**
 * Declare variables
 */
S32             giFinishSignal;     /**< Finish Signal */
S32             giStopFlag;         /**< main loop Flag 0: Stop, 1: Loop */

stMEMSINFO      *pMEMSINFO;         /**< new interface  */
stCIFO          *gpCIFO;
stHASHOINFO     *pTCPHASH;          /**< TCP Hash Table */
stHASHOINFO     *pHTTPHASH;         /**< HTTP Hash Table */

S32             gACALLCnt = 0;

/* FOR DEBUG */
S64             curSessCnt;     /* Transaction */
S64             sessCnt;
S64             rcvNodeCnt;     
S64             diffSeqCnt;     /* DIFF SEQ*/

UINT            guiSeqProcID;

/* FOR MULTIPLE PROCESS */
char           gszMyProc[32];

/** main function.
 *
 *  man Function
 *
 *  @param	argc	:	파라미터 개수
 *  @param	*argv[]	:	파라미터
 *
 *  @return			S32
 *  @see			http_main.c l4.h http_init.c http_func.c
 *
 *  @exception		.
 *  @note			.
 **/

S32 main(S32 argc, S8 *argv[])
{
	S32				dRet;		/**< 함수 Return 값 */
	OFFSET			offset;
	U8				*pNode;
	U8				*pDATA;
	TCP_INFO		*pTCPINFO;
	time_t			oldTime, nowTime;
	int 			dLen, PROCNO;
    UINT 			uiSeqProcKey;

	char 			szLOGPATH[128];

    char    		vERSION[7] = "R3.0.0";


	memcpy(&gszMyProc[0], argv[0], strlen(argv[0]));

	dLen = strlen(gszMyProc);
	PROCNO = atoi(&gszMyProc[dLen-1]);
	guiSeqProcID = SEQ_PROC_A_IHTTP0 + PROCNO;

	/* log_print 초기화 */
	sprintf(szLOGPATH, LOG_PATH"/%s", gszMyProc);
	log_init(S_SSHM_LOG_LEVEL,getpid(), guiSeqProcID, szLOGPATH, gszMyProc);

	/* A_HTTP 초기화 */
	if((dRet = dInitHttp(&pMEMSINFO, &pTCPHASH, &pHTTPHASH)) < 0)
	{
		log_print(LOGN_CRI, "[%s.%d] dInitHttp dRet[%d]", __FUNCTION__, __LINE__, dRet);
		exit(0);
	}

    if((dRet = set_version(S_SSHM_VERSION, guiSeqProcID, vERSION)) < 0 ) {
        log_print(LOGN_CRI, "SET_VERSION ERROR(RET=%d,IDX=%d,VER=%s)", dRet, uiSeqProcKey, vERSION);
    }
	log_print(LOGN_CRI, "START IHTTP[%d](%s) ITCP_SESS_CNT[%d] IHTTP_TRANS_CNT[%d]", PROCNO, vERSION, ITCP_SESS_CNT, IHTTP_TRANS_CNT);

	/* MAIN LOOP */
	while(giStopFlag)
	{
		if((offset = gifo_read(pMEMSINFO, gpCIFO, guiSeqProcID)) > 0) {
			pNode = nifo_ptr(pMEMSINFO, offset);
			pTCPINFO = (TCP_INFO *)nifo_get_value(pMEMSINFO, TCP_INFO_DEF_NUM, offset);
			pDATA = (U8 *)nifo_get_value(pMEMSINFO, ETH_DATA_NUM, offset);

			if(dProcHttpTrans(pMEMSINFO, pTCPHASH, pHTTPHASH, pTCPINFO, pNode, pDATA) < 0) {
				nifo_node_delete(pMEMSINFO, pNode);
			}	
		} else {
			usleep(0);
		}

		nowTime = time(NULL);
		if(nowTime >= oldTime + 60) {
			oldTime = nowTime;
			log_print(LOGN_CRI, "CUR_SESS_CNT[%lld] SESS_CNT[%lld] RCVNODE_CNT[%lld] DIFFSEQ_CNT[%lld]", 
				curSessCnt, sessCnt, rcvNodeCnt, diffSeqCnt);
			rcvNodeCnt = 0;
			diffSeqCnt = 0;
			sessCnt = 0;
		}
	}

	FinishProgram();

	return 0;
}

/*
 *  $Log: ihttp_main.c,v $
 *  Revision 1.2  2011/09/04 11:40:36  dhkim
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *  NEW OAM SYSTEM
 *
 *  Revision 1.3  2011/08/17 12:57:49  hhbaek
 *  A_IHTTP
 *
 *  Revision 1.2  2011/08/10 09:57:43  uamyd
 *  modified and block added
 *
 *  Revision 1.1.1.1  2011/08/05 00:27:18  uamyd
 *  init DQMS2
 *
 *  Revision 1.2  2011/05/09 10:29:36  dark264sh
 *  A_IHTTP: A_CALL multi 처리
 *
 *  Revision 1.1  2011/04/11 12:06:34  dark264sh
 *  A_IHTTP 추가
 *
 */
