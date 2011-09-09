/**		@file	jnet_msgq.c
 * 		- JAVA NETWORK CONTENT 관리 하는 프로세스
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: jnc_msgq.c,v 1.2 2011/09/05 05:21:22 uamyd Exp $
 *
 * 		@Author		$Author: uamyd $
 * 		@version	$Revision: 1.2 $
 * 		@date		$Date: 2011/09/05 05:21:22 $
 * 		@ref		jnet_msgq.c
 * 		@todo		library가 만들어지지 않은 상태, library 생성 이후 함수 대치
 *
 * 		@section	Intro(소개)
 * 		- JAVA NETWORK CONTENT 관리 하는 프로세스
 *
 * 		@section	Requirement
 * 		 @li library 생성 이후 함수 대치
 *
 **/

#include "loglib.h"

#include "mems.h"
#include "cifo.h"
#include "gifo.h"
#include "nifo.h"

#include "common_stg.h"
#include "procid.h"

#include "jnc_msgq.h"


extern stCIFO *gpCIFO;

/** dSend_JNC_Data function.
 *
 *  dSend_JNC_Data Function
 *
 *  @param	*pMEMSINFO : New Interface 관리 구조체
 *  @param	*pNode : 전송하고자 하는 Node
 *  @param	dSeqProcID : send the sequence id to the next process 
 *
 *  @return			S32	 SUCCESS: 0, FAIL: -1(NIFO NODE 생성 실패) -2(TLV NODE 생성 실패) -3(메시지 전송 실패)
 *  @see			http_msgq.c
 *
 **/
S32 dSend_JNC_Data(stMEMSINFO *pMEMSINFO, S32 dSeqProcID, U8 *pNode)
{
#ifdef MEM_TEST
	nifo_node_delete(pMEMSINFO, pNode);
#else
	S32				dRet;
	if(dSeqProcID > 0) {
        if((dRet = gifo_write(pMEMSINFO, gpCIFO, SEQ_PROC_A_JNC, dSeqProcID, nifo_offset(pMEMSINFO,pNode))) < 0) {
            log_print(LOGN_CRI, "[%s][%s.%d] gifo_write, to=%d dRet[%d][%s]",
                __FILE__, __FUNCTION__, __LINE__, dSeqProcID, dRet, strerror(-dRet));
            return -1;
        }
    } else {
        nifo_node_delete(pMEMSINFO, pNode);
        log_print(LOGN_CRI, "SEND DATA TO BLOCK[%d]", dSeqProcID);
    }
#endif	
	return 0;
}

