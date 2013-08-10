/**		@file	online_msgq.c
 * 		- ONLINE TCP Session�� ���� �ϴ� ���μ���
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: online_msgq.c,v 1.2 2011/09/05 08:20:24 dhkim Exp $
 *
 * 		@Author		$Author: dhkim $
 * 		@version	$Revision: 1.2 $
 * 		@date		$Date: 2011/09/05 08:20:24 $
 * 		@ref		online_msgq.c
 * 		@todo		library�� ��������� ���� ����, library ���� ���� �Լ� ��ġ
 *
 * 		@section	Intro(�Ұ�)
 * 		- ONLINE TCP Session�� ���� �ϴ� ���μ���
 *
 * 		@section	Requirement
 * 		 @li library ���� ���� �Լ� ��ġ
 *
 **/

// LIB
#include "typedef.h"
#include "loglib.h"
#include "mems.h"
#include "cifo.h"
#include "gifo.h"
#include "nifo.h"

// DQMS
#include "procid.h"

// .
#include "online_msgq.h"


/**
 * Declare variables
 */
extern stCIFO *gpCIFO;

/** dSend_ONLINE_Data function.
 *
 *  dSend_ONLINE_Data Function
 *
 *  @param	*pMEMSINFO : New Interface ���� ����ü
 *  @param	*pNode : �����ϰ��� �ϴ� Node
 *  @param	dSeqProcID : send the sequence id to the next process
 *
 *  @return			S32	 SUCCESS: 0, FAIL: -1(NIFO NODE ���� ����) -2(TLV NODE ���� ����) -3(�޽��� ���� ����)
 *  @see			online_msgq.c
 *
 **/
S32 dSend_ONLINE_Data(stMEMSINFO *pMEMSINFO, S32 dSeqProcID, U8 *pNode)
{

#ifdef MEM_TEST
	nifo_node_delete(pMEMSINFO, pNode);
#else
	S32				dRet;
	if(dSeqProcID > 0) {
        if((dRet = gifo_write(pMEMSINFO, gpCIFO, SEQ_PROC_A_ONLINE, dSeqProcID, nifo_offset(pMEMSINFO,pNode))) < 0) {
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
 *  $Log: online_msgq.c,v $
 *  Revision 1.2  2011/09/05 08:20:24  dhkim
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *  NEW OAM SYSTEM
 *
 *  Revision 1.3  2011/08/17 13:06:40  hhbaek
 *  A_ONLINE
 *
 *  Revision 1.2  2011/08/09 08:17:40  uamyd
 *  add blocks
 *
 *  Revision 1.1.1.1  2011/08/05 00:27:18  uamyd
 *  init DQMS2
 *
 *  Revision 1.2  2011/01/11 04:09:09  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2010/08/23 01:12:58  uamyd
 *  DQMS With TOTMON, 2nd-import
 *
 *  Revision 1.1.1.1  2009/05/26 02:14:42  dqms
 *  Init TAF_RPPI
 *
 *  Revision 1.1.1.1  2008/06/09 08:17:16  jsyoon
 *  WATAS3 PROJECT START
 *
 *  Revision 1.1  2007/08/21 12:53:54  dark264sh
 *  no message
 *
 *  Revision 1.2  2006/11/28 12:58:27  cjlee
 *  doxygen
 *
 *  Revision 1.1  2006/10/27 12:35:51  dark264sh
 *  *** empty log message ***
 *
 */
