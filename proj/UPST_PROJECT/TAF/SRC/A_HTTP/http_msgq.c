/**		@file	http_msgq.c
 * 		- HTTP Session을 관리 하는 프로세스
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: http_msgq.c,v 1.2 2011/09/04 11:12:12 dhkim Exp $
 *
 * 		@Author		$Author: dhkim $
 * 		@version	$Revision: 1.2 $
 * 		@date		$Date: 2011/09/04 11:12:12 $
 * 		@ref		http_msgq.c
 * 		@todo		library가 만들어지지 않은 상태, library 생성 이후 함수 대치
 *
 * 		@section	Intro(소개)
 * 		- HTTP Session을 관리 하는 프로세스
 *
 * 		@section	Requirement
 * 		 @li library 생성 이후 함수 대치
 *
 **/

// LIB
#include "typedef.h"
#include "mems.h"
#include "cifo.h"
#include "nifo.h"
#include "gifo.h"
#include "loglib.h"

extern stCIFO 	*gpCIFO;
extern UINT 	guiSeqProcID;


/** dSend_HTTP_Data function.
 *
 *  dSend_HTTP_Data Function
 *
 *  @param	*pMEMSINFO : New Interface 관리 구조체
 *  @param	*pNode : 전송하고자 하는 Node
 *  @param	dSeqProcID : send the sequence id to the next process
 *
 *  @return			S32	 SUCCESS: 0, FAIL: -1(NIFO NODE 생성 실패) -2(TLV NODE 생성 실패) -3(메시지 전송 실패)
 *  @see			http_msgq.c
 *
 **/
S32 dSend_HTTP_Data(stMEMSINFO *pMEMSINFO, S32 dSeqProcID, U8 *pNode)
{

#ifdef MEM_TEST
	nifo_node_delete(pMEMSINFO, pNode);
#else
	S32				dRet;
	if(dSeqProcID > 0) {
		if((dRet = gifo_write(pMEMSINFO, gpCIFO, guiSeqProcID, dSeqProcID, nifo_offset(pMEMSINFO,pNode))) < 0) {
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
 *  $Log: http_msgq.c,v $
 *  Revision 1.2  2011/09/04 11:12:12  dhkim
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *  NEW OAM SYSTEM
 *
 *  Revision 1.2  2011/08/05 09:04:49  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 *  init DQMS2
 *
 *  Revision 1.2  2011/01/11 04:09:07  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2010/08/23 01:12:59  uamyd
 *  DQMS With TOTMON, 2nd-import
 *
 *  Revision 1.1.1.1  2009/05/26 02:14:19  dqms
 *  Init TAF_RPPI
 *
 *  Revision 1.1.1.1  2008/06/09 08:17:17  jsyoon
 *  WATAS3 PROJECT START
 *
 *  Revision 1.1  2007/08/21 12:53:00  dark264sh
 *  no message
 *
 *  Revision 1.5  2006/11/28 12:58:27  cjlee
 *  doxygen
 *
 *  Revision 1.4  2006/10/20 10:01:32  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.3  2006/10/18 08:53:31  dark264sh
 *  nifo debug 코드 추가
 *
 *  Revision 1.2  2006/10/11 11:52:33  dark264sh
 *  PRE_A, A_TCP, A_HTTP에 SVC filter 적용
 *
 *  Revision 1.1.1.1  2006/10/11 07:45:49  dark264sh
 *  no message
 *
 *  Revision 1.2  2006/09/13 04:30:25  dark264sh
 *  strerror 잘못 찍는 부분 수정
 *
 *  Revision 1.1  2006/08/25 07:15:08  dark264sh
 *  no message
 *
 */
