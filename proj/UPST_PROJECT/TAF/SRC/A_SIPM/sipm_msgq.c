/**		@file	sipm_msgq.c
 * 		- SIP Session을 관리 하는 프로세스
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: sipm_msgq.c,v 1.2 2011/09/05 12:26:42 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.2 $
 * 		@date		$Date: 2011/09/05 12:26:42 $
 * 		@ref		sipm_msgq.c
 * 		@todo		Nothing
 *
 * 		@section	Intro(소개)
 * 		- SIP Session을 관리 하는 프로세스
 *
 * 		@section	Requirement
 * 		 @li		Nothing
 *
 **/

#include "sipm_msgq.h"

/** dSend_SIPM_Data function.
 *
 *  dSend_SIPM_Data Function
 *
 *  @param	*pMEMSINFO : New Interface 관리 구조체
 *  @param	*pNode : 전송하고자 하는 Node
 *  @param	dSeqProcID : send the sequence id to the next process
 *
 *  @return			S32	 SUCCESS: 0, FAIL: -1(NIFO NODE 생성 실패) -2(TLV NODE 생성 실패) -3(메시지 전송 실패)
 *  @see			http_msgq.c
 *
 **/
S32 dSend_SIPM_Data(stMEMSINFO *pMEMSINFO, S32 dSeqProcID, U8 *pNode)
{

#ifdef MEM_TEST
	nifo_node_delete(pMEMSINFO, pNode);
#else
	S32				dRet;
	if(dSeqProcID > 0) {
        if((dRet = gifo_write(pMEMSINFO, gpCIFO, SEQ_PROC_A_SIPM, dSeqProcID, nifo_offset(pMEMSINFO,pNode))) < 0) {
            log_print(LOGN_CRI, LH"gifo_write, to=%d dRet[%d][%s]",
                LT, dSeqProcID, dRet, strerror(-dRet));
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
 *  $Log: sipm_msgq.c,v $
 *  Revision 1.2  2011/09/05 12:26:42  hhbaek
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *  NEW OAM SYSTEM
 *
 *  Revision 1.4  2011/08/18 01:38:46  hhbaek
 *  A_SIPM
 *
 *  Revision 1.3  2011/08/09 08:17:41  uamyd
 *  add blocks
 *
 *  Revision 1.2  2011/08/09 05:31:08  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 *  init DQMS2
 *
 *  Revision 1.2  2011/01/11 04:09:10  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2010/08/23 01:13:02  uamyd
 *  DQMS With TOTMON, 2nd-import
 *
 *  Revision 1.1.1.1  2009/05/26 02:14:35  dqms
 *  Init TAF_RPPI
 *
 *  Revision 1.1  2008/09/18 07:19:52  dark264sh
 *  IM 서비스 추가 (SIP, XCAP, MSRP)
 *
 *  Revision 1.1.1.1  2007/12/27 08:17:38  uamyd
 *  import
 *
 *  Revision 1.1.1.1  2007/10/22 07:50:44  jsyoon
 *  AQUA3 Project = AQUA2 + IMS Protocol + Packet Dump
 *
 *  Revision 1.1  2007/05/10 02:57:30  dark264sh
 *  A_SIPM (TCP Merge) 추가
 *
 */
