/**		@file	pciv_msgq.c
 * 		- A_IV ���μ����� �ʱ�ȭ �ϴ� �Լ���
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
 * 		@section	Intro(�Ұ�)
 * 		- A_IV ���μ����� �ʱ�ȭ �ϴ� �Լ���
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
 *  @param  *pMEMSINFO : New Interface ���� ����ü
 *  @param  *pNode : �����ϰ��� �ϴ� Node
 *  @param  dSeqProcID : send the sequence id to the next process 
 *
 *  @return         S32  SUCCESS: 0, FAIL: -1(NIFO NODE ���� ����) -2(TLV NODE ���� ����) -3(�޽��� ���� ����)
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


