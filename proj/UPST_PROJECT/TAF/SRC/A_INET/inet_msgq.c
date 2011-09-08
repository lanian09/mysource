/**		@file	inet_func.c
 * 		- Internet ��뷮�� ��� �ϴ� ���μ���
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: inet_msgq.c,v 1.2 2011/09/04 12:16:51 hhbaek Exp $
 *
 * 		@Author		$Author: hhbaek $
 * 		@version	$Revision: 1.2 $
 * 		@date		$Date: 2011/09/04 12:16:51 $
 * 		@ref		inet_func.c
 * 		@todo		Nothing
 *
 * 		@section	Intro(�Ұ�)
 * 		- Internet ��뷮�� ��� �ϴ� ���μ���
 *
 * 		@section	Requirement
 * 		 @li library ���� ���� �Լ� ��ġ
 *
 **/

/**
 * Include headers
 */
#include <stdio.h>

// TOP
#include "commdef.h"
#include "common_stg.h"			/* LOG_INET .. */

// LIB headers
#include "typedef.h"			/* UINT .. */
#include "mems.h"
#include "cifo.h"
#include "nifo.h"
#include "gifo.h"
#include "loglib.h"

// .
#include "inet_msgq.h"

/**
 * Declare variables
 */
extern stCIFO	*gpCIFO;
extern S32		guiSeqProcID;

unsigned char   *Send_Node_Head[MAX_MP_NUM];
unsigned int    Diff_Node_Cnt[MAX_MP_NUM];
unsigned int    Send_Node_Cnt[MAX_MP_NUM];
unsigned int    Collection_Cnt[MAX_MP_NUM];
unsigned int    old_time[MAX_MP_NUM];
unsigned int    check_pkt[MAX_MP_NUM];

time_t          curTime, oldTime=0, chkTime;

/**
 *	Implement func.
 */

/** dSendINETLog function.
 *
 *  dSendINETLog Function
 *
 *  @param  *pLOG : LOG_INET
 *
 *  @return         S32
 *  @see            inet_main.c
 *
 *  @exception      Nothing
 *  @note           Nothing
 **/
S32 dSendINETLog(stMEMSINFO *pMEMSINFO, LOG_INET *pLOG, S32 dSeqProcID)
{
//	LOG_INET_Prt("PRINT LOG_INET", pLOG);
	return dSend_INET_Data(pMEMSINFO, dSeqProcID, nifo_ptr(pMEMSINFO, nifo_get_offset_node(pMEMSINFO, (U8 *)pLOG)));
}

/** dSendINETLog function.
 *
 *  dSendINETLog Function
 *
 *  @param  *pLOG : LOG_INET
 *
 *  @return         S32
 *  @see            inet_main.c
 *
 *  @exception      Nothing
 *  @note           Nothing
 **/
S32 dSend_INET_Data(stMEMSINFO *pMEMSINFO, S32 dSeqProcID, U8 *pData)
{
	S32			dRet;

	if(dSeqProcID > 0) {
        log_print(LOGN_DEBUG, "SEND DATA OFFSET[%d] dSeqProcID[%d]", nifo_offset(pMEMSINFO, pData), dSeqProcID);
    
        if((dRet = gifo_write(pMEMSINFO, gpCIFO, guiSeqProcID, dSeqProcID, nifo_offset(pMEMSINFO, pData))) < 0) {
            log_print(LOGN_CRI, "[%s.%d] gifo_write. to =%d dRet[%d][%s]",
                __FUNCTION__, __LINE__, dSeqProcID, dRet, strerror(-dRet));
            return -1;
        }   
    } else {
        log_print(LOGN_DEBUG, "SEND DATA SeqProcID[%d]", dSeqProcID);
        nifo_node_delete(pMEMSINFO, pData);
    }

	return 0;
}

int dSend_INET2_Data(stMEMSINFO *pstMEMSINFO, S32 dSeqProcID, U8 *pNode, U32 sec, U32 index)
{
#ifdef MEM_TEST
	nifo_node_delete(pstMEMSINFO, pNode);
#else
	/*
	 * SIGTERM, SIGUSR1 �ñ׳��� ���� ��쿡�� 
	 * ���۸��ϰ� �ִ� ��Ŷ�� ��� ���� �� �Ŀ� ���� �Ѵ�. 
	 */

	if(dSeqProcID <= 0) {
		log_print(LOGN_DEBUG, "SEND DATA SeqProcID[%d]", dSeqProcID);
		nifo_node_delete(pstMEMSINFO, pNode);

		return -1;
	}

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
		dRet = gifo_write(pstMEMSINFO, gpCIFO, guiSeqProcID, dSeqProcID, nifo_offset(pstMEMSINFO, Send_Node_Head[index]));
		if( dRet  < 0) {
            log_print(LOGN_CRI, "[%s.%d] gifo_write. to =%d dRet[%d][%s]",
                __FUNCTION__, __LINE__, dSeqProcID, dRet, strerror(-dRet));
            return -1;
		} else {
			if(pNode == NULL)
				log_print(LOGN_CRI, "### Send Buffered Packet[%d]", Send_Node_Cnt[index]);

			Send_Node_Cnt[index] = 0;
			Send_Node_Head[index] = NULL;
		}
	}
	/*
	 * ��Ŷ�� ������ �ӵ��� ���� ���۸��� ������ �����Ѵ�. 
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
/*
 * 	$Log: inet_msgq.c,v $
 * 	Revision 1.2  2011/09/04 12:16:51  hhbaek
 * 	*** empty log message ***
 * 	
 * 	Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 * 	NEW OAM SYSTEM
 * 	
 * 	Revision 1.4  2011/08/21 14:04:33  uamyd
 * 	added prea
 * 	
 * 	Revision 1.3  2011/08/17 13:00:01  hhbaek
 * 	A_INET
 * 	
 * 	Revision 1.2  2011/08/10 09:57:43  uamyd
 * 	modified and block added
 * 	
 * 	Revision 1.1.1.1  2011/08/05 00:27:18  uamyd
 * 	init DQMS2
 * 	
 * 	Revision 1.5  2011/05/11 07:44:08  jsyoon
 * 	*** empty log message ***
 * 	
 * 	Revision 1.4  2011/05/09 09:36:15  dark264sh
 * 	A_INET: A_CALL multi ó��
 * 	
 * 	Revision 1.3  2011/04/21 14:24:06  dark264sh
 * 	*** empty log message ***
 * 	
 * 	Revision 1.2  2011/04/13 14:15:36  dark264sh
 * 	A_INET: dSendINETLog ó��
 * 	
 * 	Revision 1.1  2011/04/13 13:14:38  dark264sh
 * 	A_INET �߰�
 * 	
 */
