/**     @file   mms_msgq.c
 *      - MMS Service Processing
 *
 *      Copyright (c) 2006~ by Upresto Inc. Korea
 *      All rights reserved
 *
 *       $Id: mms_msgq.c,v 1.2 2011/09/05 05:28:46 uamyd Exp $
 *
 *      @Author     $Author: uamyd $
 *      @version    $Revision: 1.2 $
 *      @date       $Date: 2011/09/05 05:28:46 $
 *      @ref        mms_msgq.c
 *      @todo       library가 만들어지지 않은 상태, library 생성 이후 함수 대치
 *
 *      @section    Intro(소개)
 *      - MMS Service Processing
 *
 *      @section    Requirement
 *       @li library 생성 이후 함수 대치
 *
 **/

#include "loglib.h"
#include "mems.h"
#include "cifo.h"
#include "gifo.h"
#include "nifo.h"

#include "procid.h"
#include "common_stg.h"

#include "mms_msgq.h"


extern stCIFO *gpCIFO;


/** dSend_MMS_Data function.
 *
 *  dSend_MMS_Data Function
 *
 *  @param  *pMEMSINFO : New Interface 관리 구조체
 *  @param  *pNode : 전송하고자 하는 Node
 *  @param  dSeqProcID : send the sequence id to the next process 
 *
 *  @return         S32  SUCCESS: 0, FAIL: -1(NIFO NODE 생성 실패) -2(TLV NODE 생성 실패) -3(메시지 전송 실패)
 *  @see            mms_msgq.c
 *
 **/
S32 dSend_MMS_Data(stMEMSINFO *pMEMSINFO, S32 dSeqProcID, U8 *pNode)
{

#ifdef MEM_TEST
	nifo_node_delete(pMEMSINFO, pNode);
#else
	S32             dRet;
	if(dSeqProcID > 0) {
        if((dRet = gifo_write(pMEMSINFO, gpCIFO, SEQ_PROC_A_MMS, dSeqProcID, nifo_offset(pMEMSINFO,pNode))) < 0) {
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
 *  $Log: mms_msgq.c,v $
 *  Revision 1.2  2011/09/05 05:28:46  uamyd
 *  A_MMS modified
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *  NEW OAM SYSTEM
 *
 *  Revision 1.4  2011/08/21 09:07:52  hhbaek
 *  Commit TAF/SRC
 *
 *  Revision 1.3  2011/08/17 07:23:55  dcham
 *  *** empty log message ***
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
 *  Revision 1.1.1.1  2009/05/26 02:14:40  dqms
 *  Init TAF_RPPI
 *
 *  Revision 1.1.1.1  2008/06/09 08:17:17  jsyoon
 *  WATAS3 PROJECT START
 *
 *  Revision 1.1  2007/09/03 08:32:26  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.1  2007/08/29 12:44:32  dark264sh
 *  *** empty log message ***
 *
 */
          
