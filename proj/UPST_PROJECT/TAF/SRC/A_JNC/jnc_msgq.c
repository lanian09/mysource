/**		@file	jnet_msgq.c
 * 		- JAVA NETWORK CONTENT ���� �ϴ� ���μ���
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
 * 		@todo		library�� ��������� ���� ����, library ���� ���� �Լ� ��ġ
 *
 * 		@section	Intro(�Ұ�)
 * 		- JAVA NETWORK CONTENT ���� �ϴ� ���μ���
 *
 * 		@section	Requirement
 * 		 @li library ���� ���� �Լ� ��ġ
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
 *  @param	*pMEMSINFO : New Interface ���� ����ü
 *  @param	*pNode : �����ϰ��� �ϴ� Node
 *  @param	dSeqProcID : send the sequence id to the next process 
 *
 *  @return			S32	 SUCCESS: 0, FAIL: -1(NIFO NODE ���� ����) -2(TLV NODE ���� ����) -3(�޽��� ���� ����)
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

