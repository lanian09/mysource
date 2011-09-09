/**     @file   call_msgq.c
 *      - Call Session�� ���� �ϴ� ���μ���
 *
 *      Copyright (c) 2006~ by Upresto Inc. Korea
 *      All rights reserved
 *
 *       $Id: vt_msgq.c,v 1.2 2011/09/06 12:46:40 hhbaek Exp $
 *
 *      @Author     $Author: hhbaek $
 *      @version    $Revision: 1.2 $
 *      @date       $Date: 2011/09/06 12:46:40 $
 *      @warning    .
 *      @ref        call_msgq.c
 *      @todo       library�� ��������� ���� ����, library ���� ���� �Լ� ��ġ
 *
 *      @section    Intro(�Ұ�)
 *      - CALL Session�� ���� �ϴ� ���μ���
 *
 *      @section    Requirement
 *       @li library ���� ���� �Լ� ��ġ
 *
 **/

/**
 *	Include headers
 */

// TOP
#include "procid.h"

// LIB
#include "cifo.h"
#include "nifo.h"
#include "gifo.h"
#include "loglib.h"

// .
#include "vt_msgq.h"

/**
 *	Declare var.
 */
extern stCIFO *gpCIFO;

/**
 *	Implement func.
 */

/** dSend_CALL_Data function.
 *
 *  dSend_CALL_Data Function
 *
 *  @param  *pMEMSINFO : New Interface ���� ����ü
 *  @param  *pNode : �����ϰ��� �ϴ� Node
 *  @param  dSndMsgQ : Send the Msg to the next process
 *
 *  @return         S32  SUCCESS: 0, FAIL: -1(NIFO NODE ���� ����) -2(TLV NODE ���� ����) -3(�޽��� ���� ����)
 *  @see            call_msgq.c
 *
 **/
S32 dSend_VT_Data(stMEMSINFO *pMEMSINFO, S32 dSndMsgQ, U8 *pNode)
{

#ifdef MEM_TEST
	nifo_node_delete(pMEMSINFO, pNode);
#else
	S32             dRet;
	if((dRet = gifo_write(pMEMSINFO, gpCIFO, SEQ_PROC_A_VT, dSndMsgQ, nifo_offset(pMEMSINFO,pNode))) < 0) {
		log_print(LOGN_CRI, "[%s][%s.%d] gifo_write dRet[%d][%s]", __FILE__, __FUNCTION__, __LINE__, dRet, strerror(-dRet));
		return -1;
	}
#endif
	return 0;
}

/**
 *  $Log: vt_msgq.c,v $
 *  Revision 1.2  2011/09/06 12:46:40  hhbaek
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *  NEW OAM SYSTEM
 *
 *  Revision 1.5  2011/08/25 07:25:47  uamyd
 *  nifo_msg_write api or log changed to gifo_write
 *
 *  Revision 1.4  2011/08/11 08:09:11  hhbaek
 *  Commit A_VT
 *
 *  Revision 1.3  2011/08/09 09:06:42  hhbaek
 *  Commit A_VT
 *
 *  Revision 1.2  2011/08/09 05:20:01  hhbaek
 *  Commit A_VT service block
 *
 *  Revision 1.1.1.1  2011/08/05 00:27:18  uamyd
 *  init DQMS2
 *
 *  Revision 1.2  2011/01/11 04:09:11  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2010/08/23 01:13:01  uamyd
 *  DQMS With TOTMON, 2nd-import
 *
 *  Revision 1.1  2009/06/10 21:46:09  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2009/05/26 02:14:22  dqms
 *  Init TAF_RPPI
 *
 *  Revision 1.1.1.1  2008/06/09 08:17:16  jsyoon
 *  WATAS3 PROJECT START
 *
 *  Revision 1.1  2007/08/21 12:52:37  dark264sh
 *  no message
 *
 *  Revision 1.2  2006/11/28 12:58:27  cjlee
 *  doxygen
 *
 *  Revision 1.1  2006/10/20 10:00:53  dark264sh
 *  *** empty log message ***
 *
 */
          
