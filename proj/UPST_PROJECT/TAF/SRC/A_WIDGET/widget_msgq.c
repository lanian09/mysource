/**     @file   widget_msgq.c
 *      - WIDGET Service Processing
 *
 *      Copyright (c) 2006~ by Upresto Inc. Korea
 *      All rights reserved
 *
 *       $Id: widget_msgq.c,v 1.2 2011/09/06 12:46:41 hhbaek Exp $
 *
 *      @Author     $Author: hhbaek $
 *      @version    $Revision: 1.2 $
 *      @date       $Date: 2011/09/06 12:46:41 $
 *      @ref        widget_msgq.c
 *      @todo       library가 만들어지지 않은 상태, library 생성 이후 함수 대치
 *
 *      @section    Intro(소개)
 *      - WIDGET Service Processing
 *
 *      @section    Requirement
 *       @li library 생성 이후 함수 대치
 *
 **/

/**
 * Include headers
 */
// LIB
#include "common_stg.h"
#include "procid.h"

// LIB
#include "gifo.h"
#include "nifo.h"
#include "loglib.h"

// .
#include "widget_msgq.h"

/**
 * Declare variables
 */
extern stCIFO *gpCIFO;

/**
 *	Implement func.
 */

/** dSend_WIDGET_Data function.
 *
 *  dSend_WIDGET_Data Function
 *
 *  @param  *pMWIDGETINFO : New Interface 관리 구조체
 *  @param  *pNode : 전송하고자 하는 Node
 *  @param  dSeqProcID : send the sequence id to the next process
 *
 *  @return         S32  SUCCESS: 0, FAIL: -1(NIFO NODE 생성 실패) -2(TLV NODE 생성 실패) -3(메시지 전송 실패)
 *  @see            widget_msgq.c
 *
 **/
S32 dSend_WIDGET_Data(stMEMSINFO *pMEMSINFO, S32 dSeqProcID, U8 *pNode)
{

#ifdef MEM_TEST
	nifo_node_delete(pMEMSINFO, pNode);
#else
	S32             dRet;
	if(dSeqProcID > 0) {
        if((dRet = gifo_write(pMEMSINFO, gpCIFO, SEQ_PROC_A_WIDGET, dSeqProcID, nifo_offset(pMEMSINFO,pNode))) < 0) {
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
 *  $Log: widget_msgq.c,v $
 *  Revision 1.2  2011/09/06 12:46:41  hhbaek
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *  NEW OAM SYSTEM
 *
 *  Revision 1.3  2011/08/17 13:16:31  hhbaek
 *  A_WIDGET
 *
 *  Revision 1.2  2011/08/09 08:17:42  uamyd
 *  add blocks
 *
 *  Revision 1.1.1.1  2011/08/05 00:27:18  uamyd
 *  init DQMS2
 *
 *  Revision 1.2  2011/01/11 04:09:11  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2010/08/23 01:13:01  uamyd
 *  DQMS With TOTMON, 2nd-import
 *
 *  Revision 1.1.1.1  2009/05/26 02:14:47  dqms
 *  Init TAF_RPPI
 *
 *  Revision 1.1  2008/11/25 12:45:57  dark264sh
 *  WIDGET 처리
 *
 */
          
