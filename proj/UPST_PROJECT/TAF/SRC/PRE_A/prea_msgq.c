/**     @file   pre_a_msgq.c
 *      - TCP Session을 관리 하는 프로세스
 *
 *      Copyright (c) 2006~ by Upresto Inc. Korea
 *      All rights reserved
 *
 *       $Id: prea_msgq.c,v 1.2 2011/09/06 12:46:43 hhbaek Exp $
 *
 *      @Author     $Author: hhbaek $
 *      @version    $Revision: 1.2 $
 *      @date       $Date: 2011/09/06 12:46:43 $
 *      @ref        pre_a_msgq.c
 *      @todo       library가 만들어지지 않은 상태, library 생성 이후 함수 대치
 *
 *      @section    Intro(소개)
 *      - TCP Session을 관리 하는 프로세스
 *
 *      @section    Requirement
 *       @li library 생성 이후 함수 대치
 *
 **/

/**
 *	Include header
 */
// TOP
#include "common_stg.h"
#include "commdef.h"
#include "procid.h"

// LIB
#include "cifo.h"
#include "gifo.h"
#include "loglib.h"

// .
#include "prea_msgq.h"

/**
 *	Declare var.
 */
extern stCIFO	*gpCIFO;

unsigned char   *Send_Node_Head[MAX_MP_NUM];
unsigned int    Send_Node_Cnt[MAX_MP_NUM];
unsigned int    Collection_Cnt[MAX_MP_NUM];
unsigned int    old_time[MAX_MP_NUM];
unsigned int    check_pkt[MAX_MP_NUM];

/**
 *	Implement func.
 */

/** dSend_PREA_Data function.
 *
 *  dSend_PREA_Data Function
 *
 *  @param  *pMEMSINFO : New Interface 관리 구조체
 *  @param  *pNode : 전송하고자 하는 Node
 *  @param  dSndMsgQ : send the msg to the next process
 *
 *  @return         S32  SUCCESS: 0, FAIL: -1(NIFO NODE 생성 실패) -2(TLV NODE 생성 실패) -3(메시지 전송 실패)
 *  @see            pre_a_msgq.c
 *
 **/
S32 dSend_PREA_Data(stMEMSINFO *pMEMSINFO, S32 dSndProcID, U8 *pNode)
{

#ifdef MEM_TEST
	nifo_node_delete(pMEMSINFO, pNode);
#else
	S32             dRet;
	if((dRet = gifo_write(pMEMSINFO, gpCIFO, SEQ_PROC_PRE_A, dSndProcID, nifo_offset(pMEMSINFO, pNode))) < 0) {
		log_print(LOGN_CRI, LH"gifo_write to=%d, dRet[%d][%s]", 
			LT, dSndProcID, dRet, strerror(-dRet));
		return -1;
	}
#endif
	return 0;
}

int dSend_PREABUFF_Data(stMEMSINFO *pstMEMSINFO, S32 dSndProcID, U8 *pNode, U32 sec, U32 index)
{
#ifdef MEM_TEST
	nifo_node_delete(pstMEMSINFO, pNode);
#else
	/*
	 * SIGTERM, SIGUSR1 시그널을 받은 경우에는 
	 * 버퍼링하고 있는 패킷을 모두 전송 한 후에 종료 한다. 
	 */
	if(pNode == NULL) {
		Collection_Cnt[index] = 0;
	} else {
		if(Send_Node_Head[index]) {
			nifo_node_link_nont_prev(pstMEMSINFO, Send_Node_Head[index], pNode);
		} else {
			Send_Node_Head[index] = pNode;
		}
		Send_Node_Cnt[index]++;
		check_pkt[index]++;
	}

	if(Send_Node_Cnt[index] > Collection_Cnt[index]) {
		UINT             dRet;

		dRet = gifo_write(pstMEMSINFO, gpCIFO, SEQ_PROC_PRE_A, dSndProcID, nifo_offset(pstMEMSINFO, Send_Node_Head[index]));
		if( dRet  < 0) {
			log_print(LOGN_CRI, LH"gifo_write dSndProcID[%d] dRet[%d][%s]", 
				LT, dSndProcID, dRet, strerror(-dRet));
			return -1;
		} else {
			if(pNode == NULL)
				log_print(LOGN_CRI, "### Send Buffered Packet[%d]", Send_Node_Cnt[index]);

			Send_Node_Cnt[index] = 0;
			Send_Node_Head[index] = NULL;
		}
	}
	/*
	 * 패킷이 들어오는 속도에 따라 버퍼링의 개수를 조절한다. 
	 */
#ifdef BUFFERING
	if( (old_time[index] + COLLECTION_TIME) < sec) {
		if( (check_pkt[index] / COLLECTION_TIME) >  (Collection_Cnt[index] * COLLECTION_MULTIPLY) ) {
			Collection_Cnt[index] *= COLLECTION_MULTIPLY;
			if(Collection_Cnt[index] > COLLECTION_MAX) {
				Collection_Cnt[index] = COLLECTION_MAX;
			}
		} else if( (check_pkt[index] / COLLECTION_TIME) < (Collection_Cnt[index] / COLLECTION_MULTIPLY) ) {
			Collection_Cnt[index] /= COLLECTION_MULTIPLY;
			if(Collection_Cnt[index] < COLLECTION_MIN) {
				Collection_Cnt[index] = COLLECTION_MIN;
			}
		}
		log_print(LOGN_INFO, "CHECK_PKT:%d COLLECTION_CNT:%d", check_pkt[index], Collection_Cnt[index]);
		check_pkt[index] = 0;
		old_time[index] = sec;
	}
#endif /* BUFFERING */

#endif
	return 0;
}
/**
 *  $Log: prea_msgq.c,v $
 *  Revision 1.2  2011/09/06 12:46:43  hhbaek
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *  NEW OAM SYSTEM
 *
 *  Revision 1.1  2011/08/21 14:04:35  uamyd
 *  added prea
 *
 *  Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 *  init DQMS2
 *
 *  Revision 1.3  2011/05/11 07:48:17  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.2  2011/01/11 04:09:12  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2010/08/23 01:12:59  uamyd
 *  DQMS With TOTMON, 2nd-import
 *
 *  Revision 1.1.1.1  2009/05/26 02:14:47  dqms
 *  Init TAF_RPPI
 *
 *  Revision 1.1.1.1  2008/06/09 08:17:16  jsyoon
 *  WATAS3 PROJECT START
 *
 *  Revision 1.1  2007/08/21 12:58:42  dark264sh
 *  no message
 *
 *  Revision 1.2  2006/11/28 12:58:27  cjlee
 *  doxygen
 *
 *  Revision 1.1  2006/10/20 10:04:44  dark264sh
 *  *** empty log message ***
 *
 */
          
