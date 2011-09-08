/**     @file   xcap_msgq.c
 *      - XCAP Service Processing
 *
 *      Copyright (c) 2006~ by Upresto Inc. Korea
 *      All rights reserved
 *
 *       $Id: xcap_msgq.c,v 1.2 2011/09/06 12:46:42 hhbaek Exp $
 *
 *      @Author     $Author: hhbaek $
 *      @version    $Revision: 1.2 $
 *      @date       $Date: 2011/09/06 12:46:42 $
 *      @ref        xcap_msgq.c
 *      @todo       Nothing
 *
 *      @section    Intro(소개)
 *      - XCAP Service Processing
 *
 *      @section    Requirement
 *       @li Nothing
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
#include "xcap_msgq.h"

/**
 *	Declare var.
 */
extern stCIFO *gpCIFO;

/**
 *	Implement func.
 */

/** dSend_XCAP_Data function.
 *
 *  dSend_XCAP_Data Function
 *
 *  @param  *pMEMSINFO : New Interface 관리 구조체
 *  @param  *pNode : 전송하고자 하는 Node
 *  @param  dSeqProcID : send the sequence id to the next process
 *
 *  @return         S32  SUCCESS: 0, FAIL: -1(NIFO NODE 생성 실패) -2(TLV NODE 생성 실패) -3(메시지 전송 실패)
 *  @see            xcap_msgq.c
 *
 **/
S32 dSend_XCAP_Data(stMEMSINFO *pMEMSINFO, S32 dSeqProcID, U8 *pNode)
{

#ifdef MEM_TEST
	nifo_node_delete(pMEMSINFO, pNode);
#else
	S32             dRet;
	if(dSeqProcID > 0) {
        if((dRet = gifo_write(pMEMSINFO, gpCIFO, SEQ_PROC_A_XCAP, dSeqProcID, nifo_offset(pMEMSINFO,pNode))) < 0) {
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
 *  $Log: xcap_msgq.c,v $
 *  Revision 1.2  2011/09/06 12:46:42  hhbaek
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *  NEW OAM SYSTEM
 *
 *  Revision 1.5  2011/08/25 07:25:47  uamyd
 *  nifo_msg_write api or log changed to gifo_write
 *
 *  Revision 1.4  2011/08/21 09:07:53  hhbaek
 *  Commit TAF/SRC/	*
 *
 *  Revision 1.3  2011/08/17 07:28:12  dcham
 *  *** empty log message ***
 *
 *  Revision 1.2  2011/08/08 11:05:44  uamyd
 *  modified block added
 *
 *  Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 *  init DQMS2
 *
 *  Revision 1.2  2011/01/11 04:09:11  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2010/08/23 01:13:01  uamyd
 *  DQMS With TOTMON, 2nd-import
 *
 *  Revision 1.1.1.1  2009/05/26 02:14:30  dqms
 *  Init TAF_RPPI
 *
 *  Revision 1.1  2008/09/18 07:24:37  dark264sh
 *  IM 서비스 추가 (SIP, XCAP, MSRP)
 *
 *  Revision 1.1.1.1  2007/12/27 08:17:38  uamyd
 *  import
 *
 *  Revision 1.1.1.1  2007/10/22 07:50:44  jsyoon
 *  AQUA3 Project = AQUA2 + IMS Protocol + Packet Dump
 *
 *  Revision 1.1  2007/03/07 10:33:41  dark264sh
 *  *** empty log message ***
 *
 *
 */
          
