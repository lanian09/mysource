/**		@file	msrpm_msgq.c
 * 		- MSRP Session을 관리 하는 프로세스
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: msrpm_msgq.c,v 1.2 2011/09/05 05:43:37 uamyd Exp $
 *
 * 		@Author		$Author: uamyd $
 * 		@version	$Revision: 1.2 $
 * 		@date		$Date: 2011/09/05 05:43:37 $
 * 		@ref		msrpm_msgq.c
 * 		@todo		Nothing
 *
 * 		@section	Intro(소개)
 * 		- MSRP Session을 관리 하는 프로세스
 *
 * 		@section	Requirement
 * 		 @li		Nothing
 *
 **/

#include "loglib.h"
#include "mems.h"
#include "cifo.h"
#include "gifo.h"
#include "nifo.h"

#include "path.h"
#include "common_stg.h"
#include "procid.h"

#include "msrpm_msgq.h"

extern stCIFO *gpCIFO;

/** dSend_MSRPM_Data function.
 *
 *  dSend_MSRPM_Data Function
 *
 *  @param	*pMEMSINFO : New Interface 관리 구조체
 *  @param	*pNode : 전송하고자 하는 Node
 *  @param	dSeqProcID : send the sequence id to the next process
 *
 *  @return			S32	 SUCCESS: 0, FAIL: -1(NIFO NODE 생성 실패) -2(TLV NODE 생성 실패) -3(메시지 전송 실패)
 *  @see			http_msgq.c
 *
 **/
S32 dSend_MSRPM_Data(stMEMSINFO *pMEMSINFO, S32 dSeqProcID, U8 *pNode)
{

#ifdef MEM_TEST
	nifo_node_delete(pMEMSINFO, pNode);
#else
	S32				dRet;
	if(dSeqProcID > 0) {
        if((dRet = gifo_write(pMEMSINFO, gpCIFO, SEQ_PROC_A_MSRPM, dSeqProcID, nifo_offset(pMEMSINFO,pNode))) < 0) {
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
 *  $Log: msrpm_msgq.c,v $
 *  Revision 1.2  2011/09/05 05:43:37  uamyd
 *  MSRPM modified
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *  NEW OAM SYSTEM
 *
 *  Revision 1.5  2011/08/21 09:07:52  hhbaek
 *  Commit TAF/SRC
 *
 *  Revision 1.4  2011/08/17 12:12:18  dcham
 *  *** empty log message ***
 *
 *  Revision 1.3  2011/08/09 05:31:08  uamyd
 *  modified
 *
 *  Revision 1.2  2011/08/08 11:05:43  uamyd
 *  modified block added
 *
 *  Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 *  init DQMS2
 *
 *  Revision 1.2  2011/01/11 04:09:08  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2010/08/23 01:12:58  uamyd
 *  DQMS With TOTMON, 2nd-import
 *
 *  Revision 1.1.1.1  2009/05/26 02:14:39  dqms
 *  Init TAF_RPPI
 *
 *  Revision 1.1  2008/09/18 06:35:03  dark264sh
 *  IM 서비스 추가 (SIP, XCAP, MSRP)
 *
 *  Revision 1.1.1.1  2007/12/27 08:17:40  uamyd
 *  import
 *
 *  Revision 1.1.1.1  2007/10/22 07:50:42  jsyoon
 *  AQUA3 Project = AQUA2 + IMS Protocol + Packet Dump
 *
 *  Revision 1.1  2007/05/07 01:46:17  dark264sh
 *  INIT
 *
 */
