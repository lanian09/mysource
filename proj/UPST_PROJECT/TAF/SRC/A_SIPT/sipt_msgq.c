/**     @file   sipt_msgq.c
 *      - SIPT Service Processing
 *
 *      Copyright (c) 2006~ by Upresto Inc. Korea
 *      All rights reserved
 *
 *       $Id: sipt_msgq.c,v 1.2 2011/09/06 12:46:39 hhbaek Exp $
 *
 *      @Author     $Author: hhbaek $
 *      @version    $Revision: 1.2 $
 *      @date       $Date: 2011/09/06 12:46:39 $
 *      @ref        sipt_msgq.c
 *      @todo       Nothing
 *
 *      @section    Intro(소개)
 *      - SIPT Service Processing
 *
 *      @section    Requirement
 *       @li Nothing
 *
 **/

/**
 * Include headers
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
#include "sipt_msgq.h"

/**
 * Declare variables
 */
extern stCIFO	*gpCIFO;
extern S64	    snd_cnt;

/**
 *	Impl. func.
 */

/** dSend_SIPT_Data function.
 *
 *  dSend_SIPT_Data Function
 *
 *  @param  *pMEMSINFO : New Interface 관리 구조체
 *  @param  *pNode : 전송하고자 하는 Node
 *  @param  dSeqProcID : send the sequence id to the next process
 *
 *  @return         S32  SUCCESS: 0, FAIL: -1(NIFO NODE 생성 실패) -2(TLV NODE 생성 실패) -3(메시지 전송 실패)
 *  @see            sipt_msgq.c
 *
 **/
S32 dSend_SIPT_Data(stMEMSINFO *pMEMSINFO, S32 dSeqProcID, U8 *pNode)
{

#ifdef MEM_TEST
	nifo_node_delete(pMEMSINFO, pNode);
#else
	S32             dRet;
	
	snd_cnt++;

	if(dSeqProcID > 0) {
        if((dRet = gifo_write(pMEMSINFO, gpCIFO, SEQ_PROC_A_SIPT, dSeqProcID, nifo_offset(pMEMSINFO,pNode))) < 0) {
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
 *  $Log: sipt_msgq.c,v $
 *  Revision 1.2  2011/09/06 12:46:39  hhbaek
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *  NEW OAM SYSTEM
 *
 *  Revision 1.3  2011/08/17 13:13:55  hhbaek
 *  A_SIPT
 *
 *  Revision 1.2  2011/08/09 05:31:09  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2011/08/05 00:27:18  uamyd
 *  init DQMS2
 *
 *  Revision 1.2  2011/01/11 04:09:10  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2010/08/23 01:12:59  uamyd
 *  DQMS With TOTMON, 2nd-import
 *
 *  Revision 1.1.1.1  2009/05/26 02:14:34  dqms
 *  Init TAF_RPPI
 *
 *  Revision 1.1  2008/09/18 07:21:33  dark264sh
 *  IM 서비스 추가 (SIP, XCAP, MSRP)
 *
 *  Revision 1.1.1.1  2007/12/27 08:17:40  uamyd
 *  import
 *
 *  Revision 1.1.1.1  2007/10/22 07:50:43  jsyoon
 *  AQUA3 Project = AQUA2 + IMS Protocol + Packet Dump
 *
 *  Revision 1.2  2007/03/24 07:49:48  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.1  2007/03/05 00:37:21  dark264sh
 *  *** empty log message ***
 *
 */
          
