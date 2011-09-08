/**		@file	msrp_msgq.c
 * 		- MSRP Session�� ���� �ϴ� ���μ���
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: msrpt_msgq.c,v 1.3 2011/09/05 12:26:40 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.3 $
 * 		@date		$Date: 2011/09/05 12:26:40 $
 * 		@ref		msrp_msgq.c
 * 		@todo		Nothing
 *
 * 		@section	Intro(�Ұ�)
 * 		- MSRP Session�� ���� �ϴ� ���μ���
 *
 * 		@section	Requirement
 * 		 @li		Nothing
 *
 **/

/**
 *	Include headers
 */
// TOP
#include "procid.h"
#include "common_stg.h"

// LIB
#include "mems.h"
#include "cifo.h"
#include "gifo.h"
#include "nifo.h"
#include "loglib.h"

// .
#include "msrpt_msgq.h"

/**
 *	Declare var.
 */
extern stCIFO *gpCIFO;

/**
 *	Implement func.
 */

/** dSend_MSRPT_Data function.
 *
 *  dSend_MSRPT_Data Function
 *
 *  @param	*pMEMSINFO : New Interface ���� ����ü
 *  @param	*pNode : �����ϰ��� �ϴ� Node
 *  @param	dSeqProcID : send the sequence id to the next process
 *
 *  @return			S32	 SUCCESS: 0, FAIL: -1(NIFO NODE ���� ����) -2(TLV NODE ���� ����) -3(�޽��� ���� ����)
 *  @see			http_msgq.c
 *
 **/
S32 dSend_MSRPT_Data(stMEMSINFO *pMEMSINFO, S32 dSeqProcID, U8 *pNode)
{

#ifdef MEM_TEST
	nifo_node_delete(pMEMSINFO, pNode);
#else
	S32				dRet;
	if(dSeqProcID > 0) {
        if((dRet = gifo_write(pMEMSINFO, gpCIFO, SEQ_PROC_A_MSRPT, dSeqProcID, nifo_offset(pMEMSINFO,pNode))) < 0) {
            log_print(LOGN_CRI, LH"gifo_write, to=%d dRet[%d][%s]", LT, dSeqProcID, dRet, strerror(-dRet));
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
 *  $Log: msrpt_msgq.c,v $
 *  Revision 1.3  2011/09/05 12:26:40  hhbaek
 *  *** empty log message ***
 *
 *  Revision 1.2  2011/09/05 01:35:33  uamyd
 *  modified to runnable source
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *  NEW OAM SYSTEM
 *
 *  Revision 1.3  2011/08/21 09:07:52  hhbaek
 *  Commit TAF/SRC/ *
 *
 *  Revision 1.2  2011/08/09 05:31:08  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2011/08/05 00:27:18  uamyd
 *  init DQMS2
 *
 *  Revision 1.2  2011/01/11 04:09:09  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2010/08/23 01:13:00  uamyd
 *  DQMS With TOTMON, 2nd-import
 *
 *  Revision 1.1.1.1  2009/05/26 02:14:14  dqms
 *  Init TAF_RPPI
 *
 *  Revision 1.1  2008/09/18 06:48:10  dark264sh
 *  IM ���� �߰� (SIP, XCAP, MSRP)
 *
 *  Revision 1.1.1.1  2007/12/27 08:15:16  uamyd
 *  import
 *
 *  Revision 1.1.1.1  2007/10/22 07:50:44  jsyoon
 *  AQUA3 Project = AQUA2 + IMS Protocol + Packet Dump
 *
 *  Revision 1.1  2007/05/07 01:48:09  dark264sh
 *  INIT
 *
 */
