/**     @file   capd_msgq.c
 *      - Send CAPD DATA 
 *
 *      Copyright (c) 2006~ by Upresto Inc. Korea
 *      All rights reserved
 *
 *       $Id: capd_msgq.c,v 1.1.1.1 2011/04/19 14:13:46 june Exp $
 *
 *      @Author     $Author: june $
 *      @version    $Revision: 1.1.1.1 $
 *      @date       $Date: 2011/04/19 14:13:46 $
 *      @warning    .
 *      @ref        capd_msgq.c
 *      @todo       library�� ��������� ���� ����, library ���� ���� �Լ� ��ġ
 *
 *      @section    Intro(�Ұ�)
 *      - TCP Session�� ���� �ϴ� ���μ���
 *
 *      @section    Requirement
 *       @li library ���� ���� �Լ� ��ġ
 *
 **/            
#include <time.h>

#include "capd_def.h"
#include "utillib.h"
#include "mems.h"
#include "nifo.h"
 
#ifdef BUFFERING
#define COLLECTION_MIN          50
#else 
#define COLLECTION_MIN          0
#endif 

#define COLLECTION_MAX          80
#define COLLECTION_TIME         5
#define COLLECTION_MULTIPLY     2
                    
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
 *  @param  *pstMEMSINFO : New Interface ���� ����ü
 *  @param  *pNode : �����ϰ��� �ϴ� Node
 *  @param  dSndMsgQ : send the msg to the next process
 *  @param  sec :  the time for comparing 
 *
 *  @return         S32  SUCCESS: 0, FAIL: -1(NIFO NODE ���� ����) -2(TLV NODE ���� ����) -3(�޽��� ���� ����)
 *  @see            capd_msgq.c
 *
 **/
int dSend_CAPD_Data(stMEMSINFO *pstMEMSINFO, S32 dSndMsgQ, U8 *pNode, U32 sec)
{
#ifdef MEM_TEST
    nifo_node_delete(pstMEMSINFO, pNode);
#else
	/*
	 * SIGTERM, SIGUSR1 �ñ׳��� ���� ��쿡�� 
	 * ���۸��ϰ� �ִ� ��Ŷ�� ��� ���� �� �Ŀ� ���� �Ѵ�. 
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

    if(Send_Node_Cnt > Collection_Cnt) {
        UINT             dRet;

        if((dRet = nifo_msg_write(pstMEMSINFO, dSndMsgQ, Send_Node_Head)) < 0) {
            dAppLog(LOG_CRI, "[%s][%s.%d] nifo_msg_write dRet[%d][%s]", __FILE__, __FUNCTION__, __LINE__, dRet, strerror(-dRet));
            return -1;
        } else {
			if(pNode == NULL)
				dAppLog(LOG_CRI, "### Send Buffered Packet[%d]", Send_Node_Cnt);

            Send_Node_Cnt = 0;
            Send_Node_Head = NULL;
        }
    }
	/*
	 * ��Ŷ�� ������ �ӵ��� ���� ���۸��� ������ �����Ѵ�. 
	 */
#ifdef BUFFERING
    if( (old_time + COLLECTION_TIME) < sec) {
        if( (check_pkt / COLLECTION_TIME) >  (Collection_Cnt * COLLECTION_MULTIPLY) ) {
            Collection_Cnt *= COLLECTION_MULTIPLY;
            if(Collection_Cnt > COLLECTION_MAX) {
                Collection_Cnt = COLLECTION_MAX;
            }
        } else if( (check_pkt / COLLECTION_TIME) < (Collection_Cnt / COLLECTION_MULTIPLY) ) {
            Collection_Cnt /= COLLECTION_MULTIPLY;
            if(Collection_Cnt < COLLECTION_MIN) {
                Collection_Cnt = COLLECTION_MIN;
            }
        }
		dAppLog(LOG_INFO, "CHECK_PKT:%d COLLECTION_CNT:%d", check_pkt, Collection_Cnt);
        check_pkt = 0;
        old_time = sec;
    }
#endif /* BUFFERING */

#endif
    return 0;
}

/**
 *  $Log: capd_msgq.c,v $
 *  Revision 1.1.1.1  2011/04/19 14:13:46  june
 *  ���� ��Ű��
 *
 *  Revision 1.1.1.1  2011/01/20 12:18:54  june
 *  DSC CVS RECOVERY
 *
 *  Revision 1.1  2009/07/19 06:25:00  jjinri
 *  RDRANA CSV PARSING VERSION
 *
 *  Revision 1.1  2009/04/25 12:15:16  june
 *  rdrana
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
 *  CAPD, ANA MIF ������ ����
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
