/**     @file   capd_msgq.c
 *      - Send CAPD DATA 
 *
 *      Copyright (c) 2006~ by Upresto Inc. Korea
 *      All rights reserved
 *
 *       $Id: capd_msgq.c,v 1.3 2011/05/30 01:09:06 jjinri Exp $
 *
 *      @Author     $Author: jjinri $
 *      @version    $Revision: 1.3 $
 *      @date       $Date: 2011/05/30 01:09:06 $
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

#include "capd_def.h"
#include "utillib.h"
#include "mems.h"
#include "nifo.h"
#include "capd_bfr.h"

unsigned char 	*Send_Node_Head = NULL;
unsigned int 	Send_Node_Cnt=0;
unsigned int 	Collection_Cnt = COLLECTION_MIN;
unsigned int 	old_time = 0;   
unsigned int 	check_pkt=0;
                    
/** dSend_CAPD_Data function.
 *
 *  dSend_CAPD_Data Function : we collect the nodes for sending to the next process effectively.
 *      Compare the time and packet count.
 *      if the packets are received more than old , we increae the collection #.
 *      if the packets are received less than old , we decreae the collection #.
 *
 *  @param  *pstMEMSINFO : New Interface 관리 구조체
 *  @param  *pNode : 전송하고자 하는 Node
 *  @param  dSndMsgQ : send the msg to the next process
 *  @param  sec :  the time for comparing 
 *
 *  @return         S32  SUCCESS: 0, FAIL: -1(NIFO NODE 생성 실패) -2(TLV NODE 생성 실패) -3(메시지 전송 실패)
 *  @see            capd_msgq.c
 *
 **/
int dSend_CAPD_Data(stMEMSINFO *pstMEMSINFO, S32 dSndMsgQ, U8 *pNode, U32 sec)
{
#ifdef MEM_TEST
    nifo_node_delete(pstMEMSINFO, pNode);
#else
	/*
	 * SIGTERM, SIGUSR1 시그널을 받은 경우에는 
	 * 버퍼링하고 있는 패킷을 모두 전송 한 후에 종료 한다. 
	 */
	if(pNode == NULL) {
		Collection_Cnt = 0;
	} else {
		if(Send_Node_Head) {
			nifo_node_link_nont_prev(pstMEMSINFO, Send_Node_Head, pNode);
		} else {
			Send_Node_Head = pNode;
		}
		Send_Node_Cnt++;
		check_pkt++;
	}
#ifdef BUFFERING
    if(Send_Node_Cnt > Collection_Cnt) {
#endif	/* BUFFERING */
        INT		dRet;

        if((dRet = nifo_msg_write(pstMEMSINFO, dSndMsgQ, Send_Node_Head)) < 0) {
            dAppLog(LOG_CRI, "nifo_msg_write dRet[%d][%s]", dRet, strerror(-dRet));
            return -1;
        } else {
			if(pNode == NULL)
				dAppLog(LOG_INFO, "## Send Buffered Packet=%d", Send_Node_Cnt);

			dAppLog(LOG_DEBUG, "### SNC=%d CBC=%u", Send_Node_Cnt, Collection_Cnt);
            Send_Node_Cnt = 0;
            Send_Node_Head = NULL;
        }
#ifdef BUFFERING
    }
#endif 	/* BUFFERING */
	/*
	 * 패킷이 들어오는 속도에 따라 버퍼링의 개수를 조절한다. 
	 */
#ifdef BUFFERING
    if((old_time + COLLECTION_TIME) < sec) {
        if((check_pkt / COLLECTION_TIME) >  (Collection_Cnt * COLLECTION_MULTIPLY)) {
            Collection_Cnt *= COLLECTION_MULTIPLY;
            if (Collection_Cnt > COLLECTION_MAX) {
                Collection_Cnt = COLLECTION_MAX;
            }
        }
		else if((check_pkt / COLLECTION_TIME) < (Collection_Cnt / COLLECTION_MULTIPLY)) {
            Collection_Cnt /= COLLECTION_MULTIPLY;
            if (Collection_Cnt < COLLECTION_MIN) {
                Collection_Cnt = COLLECTION_MIN;
            }
        }
		dAppLog(LOG_DEBUG, "CHECK_PKT:%d SEND_NODE_CNT:%d COLLECTION_CNT:%d"
				, check_pkt, Send_Node_Cnt, Collection_Cnt);
        check_pkt = 0;
        old_time = sec;
    }
#endif /* BUFFERING */

#endif
    return 0;
}

/**
 *  $Log: capd_msgq.c,v $
 *  Revision 1.3  2011/05/30 01:09:06  jjinri
 *  buffering add
 *
 *  Revision 1.2  2011/04/28 05:05:57  jjinri
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2011/04/19 14:13:46  june
 *  성능 패키지
 *
 *  Revision 1.1.1.1  2011/01/20 12:18:52  june
 *  DSC CVS RECOVERY
 *
 *  Revision 1.3  2010/10/18 10:20:12  june
 *  capd 성능 향상
 *
 *  Revision 1.2  2009/06/15 15:58:46  dsc
 *  LOG LEVEL 정리
 *
 *  Revision 1.1  2009/05/27 19:55:42  dsc
 *  DLPI
 *
 *  Revision 1.3  2009/05/18 08:05:15  dsc
 *  *** empty log message ***
 *
 *  Revision 1.2  2009/05/16 18:36:29  dsc
 *  pcap lib read 방식 변경
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
 *  Revision 1.1.1.1  2008/12/30 02:33:51  upst_cvs
 *  BSD R3.0.0
 *
 *  Revision 1.1  2008/10/15 12:06:35  jsyoon
 *  CAPD, ANA MIF 구조로 변경
 *
 *  Revision 1.1.1.1  2008/06/09 08:17:17  jsyoon
 *  WATAS3 PROJECT START
 *
 *  Revision 1.2  2007/08/27 14:00:10  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.1  2007/08/21 12:55:29  dark264sh
 *  no message
 *
 *  Revision 1.4  2006/11/28 12:58:27  cjlee
 *  doxygen
 *
 *  Revision 1.3  2006/11/23 09:24:35  cjlee
 *  remove the log and bug fix
 *
 *  Revision 1.2  2006/11/22 07:20:54  cjlee
 *  multiple send
 *
 *  Revision 1.1  2006/10/20 10:04:07  dark264sh
 *  *** empty log message ***
 *
 */
