/**		@file	pciv_msgq.c
 * 		- A_IV 프로세스를 초기화 하는 함수들
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: pciv_msgq.c,v 1.1 2011/09/05 05:05:57 dhkim Exp $
 *
 * 		@Author		$Author: dhkim $
 * 		@version	$Revision: 1.1 $
 * 		@date		$Date: 2011/09/05 05:05:57 $
 * 		@ref		pciv_msgq.c 
 *
 * 		@section	Intro(소개)
 * 		- A_IV 프로세스를 초기화 하는 함수들
 *
 * 		@section	Requirement
 *
 **/

#include <stdio.h>
#include <errno.h>
#include <signal.h>

// LIB
#include "typedef.h"
#include "loglib.h"
#include "mems.h"
#include "cifo.h"
#include "gifo.h"
#include "nifo.h"

// PROJECT
#include "procid.h"

#include "pciv_msgq.h"


extern stCIFO		*gpCIFO;


/** dSend_PCIV_Data function.
 *
 *  dSend_PCIV_Data Function
 *
 *  @param  *pMEMSINFO : New Interface 관리 구조체
 *  @param  *pNode : 전송하고자 하는 Node
 *  @param  dSeqProcID : send the sequence id to the next process 
 *
 *  @return         S32  SUCCESS: 0, FAIL: -1(NIFO NODE 생성 실패) -2(TLV NODE 생성 실패) -3(메시지 전송 실패)
 *  @see            pciv_msgq.c
 *
 **/
S32 dSend_Data(stMEMSINFO *pMEMSINFO, S32 dSeqProcID, U8 *pNode)
{
#ifdef MEM_TEST
	nifo_node_delete(pMEMSINFO, pNode);
#else
	S32				dRet;
    if(dSeqProcID > 0) {
        if((dRet = gifo_write(pMEMSINFO, gpCIFO, SEQ_PROC_A_IV, dSeqProcID, nifo_offset(pMEMSINFO,pNode))) < 0) {
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

/*
 * $Log: pciv_msgq.c,v $
 * Revision 1.1  2011/09/05 05:05:57  dhkim
 * *** empty log message ***
 *
 *
 */


