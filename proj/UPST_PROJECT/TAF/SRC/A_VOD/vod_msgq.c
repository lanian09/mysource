/******************************************************************************* 
 *		@file   vod_msgq.c
 *      - TCP Session을 관리 하는 프로세스
 *
 *      Copyright (c) 2006~ by Upresto Inc. Korea
 *      All rights reserved
 *
 *       $Id: vod_msgq.c,v 1.2 2011/09/06 12:46:39 hhbaek Exp $
 *
 *      @Author     $Author: hhbaek $
 *      @version    $Revision: 1.2 $
 *      @date       $Date: 2011/09/06 12:46:39 $
 *      @warning    .
 *      @ref        vod_msgq.c
 *      @todo       library가 만들어지지 않은 상태, library 생성 이후 함수 대치
 *
 *      @section    Intro(소개)
 *      - TCP Session을 관리 하는 프로세스
 *
 *      @section    Requirement
 *       @li library 생성 이후 함수 대치
 *
*******************************************************************************/

/**
 *	Include headers
 */
// TOP
#include "common_stg.h"
#include "procid.h"

// LIB
#include "mems.h"
#include "gifo.h"
#include "nifo.h" 
#include "cifo.h"
#include "loglib.h"

// .
#include "vod_msgq.h"

/**
 *	Declare var.
 */
extern stCIFO *gpCIFO;

/**
 *	Implement func.
 */

/******************************************************************************* 
 *	dSend_VOD_Data function.
 *  @param  *pMEMSINFO : New Interface 관리 구조체
 *  @param  *pNode : 전송하고자 하는 Node
 *  @param  dSeqProcID : send the sequence id to the next process
 *  @return         S32  SUCCESS: 0, FAIL: -1(NIFO NODE 생성 실패) -2(TLV NODE 생성 실패) -3(메시지 전송 실패)
 *  @see            vod_msgq.c
*******************************************************************************/
S32 dSend_VOD_Data(stMEMSINFO *pMEMSINFO, S32 dSeqProcID, U8 *pNode)
{

#ifdef MEM_TEST
	nifo_node_delete(pMEMSINFO, pNode);
#else
	S32             dRet;
	if(dSeqProcID > 0) {
        if((dRet = gifo_write(pMEMSINFO, gpCIFO, SEQ_PROC_A_VOD, dSeqProcID, nifo_offset(pMEMSINFO,pNode))) < 0) {
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
 *  $Log: vod_msgq.c,v $
 *  Revision 1.2  2011/09/06 12:46:39  hhbaek
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *  NEW OAM SYSTEM
 *
 *  Revision 1.4  2011/08/21 09:07:53  hhbaek
 *  Commit TAF/SRC/ *
 *
 *  Revision 1.3  2011/08/17 07:26:30  dcham
 *  *** empty log message ***
 *
 *  Revision 1.2  2011/08/08 11:05:43  uamyd
 *  modified block added
 *
 *  Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 *  init DQMS2
 *
 *  Revision 1.2  2011/01/11 04:09:10  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2010/08/23 01:12:59  uamyd
 *  DQMS With TOTMON, 2nd-import
 *
 *  Revision 1.1.1.1  2009/05/26 02:14:29  dqms
 *  Init TAF_RPPI
 *
 *  Revision 1.1.1.1  2008/06/09 08:17:16  jsyoon
 *  WATAS3 PROJECT START
 *
 *  Revision 1.2  2007/09/03 09:35:21  watas
 *  *** empty log message ***
 *
 *  Revision 1.1  2007/08/21 12:54:39  dark264sh
 *  no message
 *
 *  Revision 1.2  2006/11/28 12:58:27  cjlee
 *  doxygen
 *
 *  Revision 1.1  2006/10/20 10:03:10  dark264sh
 *  *** empty log message ***
 *
 */
