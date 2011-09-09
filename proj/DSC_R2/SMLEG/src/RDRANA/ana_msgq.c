/**     @file   ana_msgq.c
 *      - Send PANA DATA 
 *
 *      Copyright (c) 2006~ by Upresto Inc. Korea
 *      All rights reserved
 *
 *       $Id: ana_msgq.c,v 1.1.1.1 2011/04/19 14:13:46 june Exp $
 *
 *      @Author     $Author: june $
 *      @version    $Revision: 1.1.1.1 $
 *      @date       $Date: 2011/04/19 14:13:46 $
 *      @warning    .
 *      @ref        capd_msgq.c
 *      @todo       library가 만들어지지 않은 상태, library 생성 이후 함수 대치
 *
 *      @section    Intro(소개)
 *      - TCP Session을 관리 하는 프로세스
 *
 *      @section    Requirement
 *       @li library 생성 이후 함수 대치
 *
 **/            
#include <time.h>

#include "comm_typedef.h"
#include "define.h"
#include "utillib.h"
#include "mems.h"
#include "nifo.h"
 
#ifdef RDR_BUFFERING
#define COLLECTION_MIN          50
#else 
#define COLLECTION_MIN          0
#endif /* RDR_BUFFERING */

#define COLLECTION_MAX          160
#define COLLECTION_TIME         5
#define COLLECTION_MULTIPLY     2
                    
unsigned char 	*Send_Node_Head_t = NULL;
unsigned int 	Send_Node_Cnt_t=0;
unsigned int 	Collection_Cnt_t = COLLECTION_MIN;
unsigned int 	old_time_t = 0;   
unsigned int 	check_pkt_t=0;
                    
/** dSend_PANA_Data function.
 *
 *  dSend_PANA_Data Function : we collect the nodes for sending to the next process effectively.
 *      Compare the time and packet count.
 *      if the packets are received more than old , we increae the collection #.
 *      if the packets are received less than old , we decreae the collection #.
 *
 *  @param  *pstMEMSINFO : 	New Interface 관리 구조체
 *  @param  *pNode : 		전송하고자 하는 Node
 *  @param  dSndMsgQ : 		send the msg to the next process
 *  @param  sec :  			the time for comparing 
 *
 *  @return         S32  SUCCESS: 0, FAIL: -1(NIFO NODE 생성 실패) -2(TLV NODE 생성 실패) -3(메시지 전송 실패)
 *  @see            capd_msgq.c
 *
 **/
int dSend_PANA_Data(stMEMSINFO *pstMEMSINFO, S32 dSndMsgQ, U8 *pNode, U32 sec)
{
	/*
	 * SIGTERM, SIGUSR1 시그널을 받은 경우에는 
	 * 버퍼링하고 있는 패킷을 모두 전송 한 후에 종료 한다. 
	 */
	if(pNode == NULL) {
		Collection_Cnt_t = 0;
	} else {
		if(Send_Node_Head_t) {
			nifo_node_link_nont_prev(pstMEMSINFO, Send_Node_Head_t, pNode);
		} else {
			Send_Node_Head_t = pNode;
		}
		Send_Node_Cnt_t++;
		check_pkt_t++;
	}

    if(Send_Node_Cnt_t > Collection_Cnt_t) {
        UINT             dRet;

        if((dRet = nifo_msg_write(pstMEMSINFO, dSndMsgQ, Send_Node_Head_t)) < 0) {
            dAppLog(LOG_CRI, "[%s][%s.%d] nifo_msg_write dRet[%d][%s]", __FILE__, __FUNCTION__, __LINE__, dRet, strerror(-dRet));
            return -1;
        } else {
			if(pNode == NULL)
				dAppLog(LOG_CRI, "### Send Buffered Packet[%d]", Send_Node_Cnt_t);

            Send_Node_Cnt_t = 0;
            Send_Node_Head_t = NULL;
        }
    }
	/*
	 * 패킷이 들어오는 속도에 따라 버퍼링의 개수를 조절한다. 
	 */
#ifdef RDR_BUFFERING
    if( (old_time_t + COLLECTION_TIME) < sec) {
        if( (check_pkt_t / COLLECTION_TIME) >  (Collection_Cnt_t * COLLECTION_MULTIPLY) ) {
            Collection_Cnt_t *= COLLECTION_MULTIPLY;
            if(Collection_Cnt_t > COLLECTION_MAX) {
                Collection_Cnt_t = COLLECTION_MAX;
            }
        } else if( (check_pkt_t / COLLECTION_TIME) < (Collection_Cnt_t / COLLECTION_MULTIPLY) ) {
            Collection_Cnt_t /= COLLECTION_MULTIPLY;
            if(Collection_Cnt_t < COLLECTION_MIN) {
                Collection_Cnt_t = COLLECTION_MIN;
            }
        }
		dAppLog(LOG_INFO, "CHECK_PKT:%d COLLECTION_CNT:%d", check_pkt_t, Collection_Cnt_t);
        check_pkt_t = 0;
        old_time_t = sec;
    }
#endif /* RDR_BUFFERING */

    return 0;
}

/**
 *  $Log: ana_msgq.c,v $
 *  Revision 1.1.1.1  2011/04/19 14:13:46  june
 *  성능 패키지
 *
 *  Revision 1.1.1.1  2011/01/20 12:18:54  june
 *  DSC CVS RECOVERY
 *
 *  Revision 1.1  2009/07/19 06:25:00  jjinri
 *  RDRANA CSV PARSING VERSION
 *
 *  Revision 1.1  2009/05/05 18:48:30  june
 *  RDRANA Trace 관련 작업 msgq처리 : MMCR , COND , SEND_Cond_SUBS
 *
 *  Revision 1.1.1.1  2009/04/06 13:02:06  june
 *  LGT DSC project init
 *
 *  Revision 1.1.1.1  2009/04/06 09:10:25  june
 *  LGT DSC project start
 *
 *  Revision 1.2  2009/03/19 07:43:34  jsyoon
 *  Add compile option for buffering
 *
 *  Revision 1.1.1.1  2008/12/30 02:32:42  upst_cvs
 *  BSD R3.0.0
 *
 */
