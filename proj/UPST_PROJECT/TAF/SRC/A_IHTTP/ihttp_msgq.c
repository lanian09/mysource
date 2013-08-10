/**		@file	http_msgq.c
 * 		- HTTP Session�� ���� �ϴ� ���μ���
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: ihttp_msgq.c,v 1.2 2011/09/04 11:40:36 dhkim Exp $
 *
 * 		@Author		$Author: dhkim $
 * 		@version	$Revision: 1.2 $
 * 		@date		$Date: 2011/09/04 11:40:36 $
 * 		@ref		http_msgq.c
 * 		@todo		library�� ��������� ���� ����, library ���� ���� �Լ� ��ġ
 *
 * 		@section	Intro(�Ұ�)
 * 		- HTTP Session�� ���� �ϴ� ���μ���
 *
 * 		@section	Requirement
 * 		 @li library ���� ���� �Լ� ��ġ
 *
 **/

// LIB
#include "typedef.h"
#include "loglib.h"
#include "mems.h"
#include "nifo.h"
#include "gifo.h"
#include "cifo.h"

/**
 * Declare variables
 */
extern stCIFO	*gpCIFO;
extern UINT		guiSeqProcID;

/** dSend_HTTP_Data function.
 *
 *  dSend_HTTP_Data Function
 *
 *  @param	*pMEMSINFO : New Interface ���� ����ü
 *  @param	*pNode : �����ϰ��� �ϴ� Node
 *  @param	dSeqProcID : send the sequence id to the next process
 *
 *  @return			S32	 SUCCESS: 0, FAIL: -1(NIFO NODE ���� ����) -2(TLV NODE ���� ����) -3(�޽��� ���� ����)
 *  @see			http_msgq.c
 *
 **/
S32 dSend_HTTP_Data(stMEMSINFO *pMEMSINFO, S32 dSeqProcID, U8 *pNode)
{

#ifdef MEM_TEST
	nifo_node_delete(pMEMSINFO, pNode);
#else
	S32				dRet;
	if(dSeqProcID > 0) {
        if((dRet = gifo_write(pMEMSINFO, gpCIFO, guiSeqProcID, dSeqProcID, nifo_offset(pMEMSINFO,pNode))) < 0) {
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

/**
 *  $Log: ihttp_msgq.c,v $
 *  Revision 1.2  2011/09/04 11:40:36  dhkim
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *  NEW OAM SYSTEM
 *
 *  Revision 1.3  2011/08/17 12:57:50  hhbaek
 *  A_IHTTP
 *
 *  Revision 1.2  2011/08/10 09:57:43  uamyd
 *  modified and block added
 *
 *  Revision 1.1.1.1  2011/08/05 00:27:18  uamyd
 *  init DQMS2
 *
 *  Revision 1.1  2011/04/11 12:06:34  dark264sh
 *  A_IHTTP �߰�
 *
 */
