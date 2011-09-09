/**     @file   call_msgq.c
 *      - Call Session을 관리 하는 프로세스
 *
 *      Copyright (c) 2006~ by Upresto Inc. Korea
 *      All rights reserved
 *
 *       $Id: im_msgq.c,v 1.2 2011/09/04 12:16:51 hhbaek Exp $
 *
 *      @Author     $Author: hhbaek $
 *      @version    $Revision: 1.2 $
 *      @date       $Date: 2011/09/04 12:16:51 $
 *      @warning    .
 *      @ref        call_msgq.c
 *      @todo       library가 만들어지지 않은 상태, library 생성 이후 함수 대치
 *
 *      @section    Intro(소개)
 *      - CALL Session을 관리 하는 프로세스
 *
 *      @section    Requirement
 *       @li library 생성 이후 함수 대치
 *
 **/

/**
 *	Include headers
 */
// TOP
#include "common_stg.h"
#include "procid.h"

// LIB 
#include "loglib.h"
#include "cifo.h"
#include "gifo.h"

// .
#include "im_msgq.h"

/**
 *	Declare var.
 */
extern stCIFO		*gpCIFO;

/** dSend_IM_Data function.
 *
 *  dSend_IM_Data Function
 *
 *  @param  *pMEMSINFO : New Interface 관리 구조체
 *  @param  *pNode : 전송하고자 하는 Node
 *  @param  dSeqProcID : Send the Msg to the next process
 *
 *  @return         S32  SUCCESS: 0, FAIL: -1(NIFO NODE 생성 실패) -2(TLV NODE 생성 실패) -3(메시지 전송 실패)
 *  @see            call_msgq.c
 *
 **/
S32 dSend_IM_Data(stMEMSINFO *pMEMSINFO, S32 dSeqProcID, U8 *pNode)
{

#ifdef MEM_TEST
	nifo_node_delete(pMEMSINFO, pNode);
#else
	S32             dRet;
	if(dSeqProcID > 0)
	{

#if 0
		if((dRet = nifo_msg_write(pMEMSINFO, dSeqProcID, pNode)) < 0) {
			log_print(LOGN_CRI, "[%s][%s.%d] nifo_msg_write dRet[%d][%s]", __FILE__, __FUNCTION__, __LINE__, dRet, strerror(-dRet));
			return -1;

		}
#endif
		
		dRet = gifo_write(pMEMSINFO, gpCIFO, SEQ_PROC_A_IM, dSeqProcID,
							nifo_offset(pMEMSINFO, pNode));
		if(dRet < 0)
		{
			log_print(LOGN_CRI, "[%s][%s.%d] gifo_write dRet[%d][%s]",
						__FILE__, __FUNCTION__, __LINE__, dRet, strerror(-dRet));
			return -1;
		}

	}
	else
	{
		nifo_node_delete(pMEMSINFO, pNode);
		log_print(LOGN_CRI, "SEND DATA TO BLOCK[%d]", dSeqProcID);
	}
#endif
	return 0;
}

/**
 *  $Log: im_msgq.c,v $
 *  Revision 1.2  2011/09/04 12:16:51  hhbaek
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *  NEW OAM SYSTEM
 *
 *  Revision 1.4  2011/08/25 07:25:47  uamyd
 *  nifo_msg_write api or log changed to gifo_write
 *
 *  Revision 1.3  2011/08/11 08:06:55  hhbaek
 *  Commit A_IM
 *
 *  Revision 1.2  2011/08/09 09:08:20  hhbaek
 *  Commit A_IM
 *
 *  Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 *  init DQMS2
 *
 *  Revision 1.3  2011/05/09 11:51:13  dark264sh
 *  A_IM: A_CALL multi 처리
 *
 *  Revision 1.2  2011/01/11 04:09:07  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2010/08/23 01:13:02  uamyd
 *  DQMS With TOTMON, 2nd-import
 *
 *  Revision 1.1  2009/06/13 11:38:45  jsyoon
 *  *** empty log message ***
 *
 */
          
