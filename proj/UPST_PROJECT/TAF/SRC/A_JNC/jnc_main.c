/**		@file	jnet_main.c
 * 		- java network content�� ���� �ϴ� ���μ���
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: jnc_main.c,v 1.2 2011/09/05 05:21:21 uamyd Exp $
 *
 * 		@Author		$Author: uamyd $
 * 		@version	$Revision: 1.2 $
 * 		@date		$Date: 2011/09/05 05:21:21 $
 * 		@warning	.
 * 		@ref		jnet_main.c l4.h jnet_init.c jnet_func.c
 * 		@todo		
 * 		@section	Intro(�Ұ�)
 * 		- java network content�� ���� �ϴ� ���μ���
 *
 * 		@section	Requirement
 * 		 @li library ���� ���� �Լ� ��ġ
 *
 **/

#include <stdio.h>
#include <unistd.h>

#include "loglib.h"
#include "verlib.h"
#include "mems.h"
#include "cifo.h"
#include "gifo.h"
#include "nifo.h"

#include "common_stg.h"
#include "path.h"
#include "commdef.h"
#include "procid.h"
#include "sshmid.h"

#include "jnc_func.h"
#include "jnc_init.h"


S32				giFinishSignal;		/**< Finish Signal */
S32				giStopFlag;			/**< main loop Flag 0: Stop, 1: Loop */

stMEMSINFO		*pMEMSINFO;			/**< new interface ���� ����ü */
stCIFO			*gpCIFO;

static stHASHOINFO	*pJNCHASH;			/**< JNC Hash Table ���� ����ü */
struct timeval      stNowTime;

/* FOR DEBUG */
S64				curSessCnt;		/* Transaction ���� */
S64				sessCnt;

/** main function.
 *
 *  man Function
 *
 *  @param	argc	:	�Ķ���� ����
 *  @param	*argv[]	:	�Ķ����
 *
 *  @return			S32
 *  @see			jnc_main.c jnc_init.c jnc_func.c
 *
 *  @exception		.
 *  @note			.
 **/
S32 main(S32 argc, S8 *argv[])
{
	S32				dRet;		/**< �Լ� Return �� */
	OFFSET			offset;
	U8				*pNode;
	U8				*pDATA;
	TCP_INFO		*pTCPINFO;
#if DEBUG_MODE
	time_t			oldTime, nowTime;
#endif

    char    vERSION[7] = "R3.0.0";


	/* log_print �ʱ�ȭ */
	log_init(S_SSHM_LOG_LEVEL, getpid(), SEQ_PROC_A_JNC, LOG_PATH"/A_JNC", "A_JNC");

	/* A_JNC �ʱ�ȭ */
	if((dRet = dInitJNC(&pMEMSINFO, &pJNCHASH)) < 0)
	{
		log_print(LOGN_CRI, "[%s.%d] dInitJNC dRet[%d]", __FUNCTION__, __LINE__, dRet);
		exit(0);
	}
	else
		log_print(LOGN_CRI, "dInitJNC Success!! [%d]", dRet);

    if((dRet = set_version(S_SSHM_VERSION, SEQ_PROC_A_JNC, vERSION)) < 0 ) {
        log_print(LOGN_CRI, "SET_VERSION ERROR(RET=%d,IDX=%d,VER=%s)", dRet, SEQ_PROC_A_JNC, vERSION);
    }
	log_print(LOGN_CRI, "START JNC(%s) JNC_SESS_CNT[%d]", vERSION, TCP_SESS_CNT);

	/* MAIN LOOP */
	while(giStopFlag)
	{
		if((offset = gifo_read(pMEMSINFO, gpCIFO, SEQ_PROC_A_JNC)) > 0) {
			pNode = nifo_ptr(pMEMSINFO, offset);
			pTCPINFO = (TCP_INFO *)nifo_get_value(pMEMSINFO, TCP_INFO_DEF_NUM, offset);
			pDATA = (U8 *)nifo_get_value(pMEMSINFO, ETH_DATA_NUM, offset);
			if(pTCPINFO == NULL) {
				log_print(LOGN_CRI, "[%s][%s.%d] TCP_INFO NULL", __FILE__, __FUNCTION__, __LINE__);
				nifo_node_delete(pMEMSINFO, pNode);
			} 
			else {
				if((dRet = dJNCSvcProcess(pMEMSINFO, pJNCHASH, pTCPINFO, pNode, pDATA)) < 0) {
					log_print(LOGN_CRI, "[%s.%d] dJNCSvcProcess dRet[%d]", __FUNCTION__, __LINE__,dRet);
					nifo_node_delete(pMEMSINFO, pNode);
				}	
			}

		} else {
			usleep(0);
		}
#if DEBUG_MODE
		nowTime = time(NULL);
		if(nowTime >= oldTime + 60) {
			oldTime = nowTime;
			log_print(LOGN_CRI, "CUR_SESS_CNT[%lld] SESS_CNT[%lld]",curSessCnt, sessCnt);
			sessCnt = 0;
		}
#endif
	}

	FinishProgram();

	return 0;
}

