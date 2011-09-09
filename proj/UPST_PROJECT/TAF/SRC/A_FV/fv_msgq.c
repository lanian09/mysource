/**     @file   fv_msgq.c
 *      - FV Service Processing
 *
 *      Copyright (c) 2006~ by Upresto Inc. Korea
 *      All rights reserved
 *
 *       $Id: fv_msgq.c,v 1.2 2011/09/04 12:16:50 hhbaek Exp $
 *
 *      @Author     $Author: hhbaek $
 *      @version    $Revision: 1.2 $
 *      @date       $Date: 2011/09/04 12:16:50 $
 *      @ref        fv_msgq.c
 *      @todo       library�� ��������� ���� ����, library ���� ���� �Լ� ��ġ
 *
 *      @section    Intro(�Ұ�)
 *      - FV Service Processing
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
#include "common_stg.h"

// LIB
#include "mems.h"
#include "cifo.h"
#include "gifo.h"
#include "nifo.h"
#include "loglib.h"

// .
#include "fv_msgq.h"

/**
 *	Declare var.
 */
extern stCIFO *gpCIFO;

/**
 *	Implement func.
 */

/** dSend_FV_Data function.
 *
 *  dSend_FV_Data Function
 *
 *  @param  *pMEMSINFO : New Interface ���� ����ü
 *  @param  *pNode : �����ϰ��� �ϴ� Node
 *  @param  dSeqProcID : send the sequence id to the next process 
 *
 *  @return         S32  SUCCESS: 0, FAIL: -1(NIFO NODE ���� ����) -2(TLV NODE ���� ����) -3(�޽��� ���� ����)
 *  @see            fv_msgq.c
 *
 **/
S32 dSend_FV_Data(stMEMSINFO *pMEMSINFO, S32 dSeqProcID, U8 *pNode)
{

#ifdef MEM_TEST
	nifo_node_delete(pMEMSINFO, pNode);
#else
	S32             dRet;
	if(dSeqProcID > 0) {
        if((dRet = gifo_write(pMEMSINFO, gpCIFO, SEQ_PROC_A_FV, dSeqProcID, nifo_offset(pMEMSINFO,pNode))) < 0) {
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
 *  $Log: fv_msgq.c,v $
 *  Revision 1.2  2011/09/04 12:16:50  hhbaek
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *  NEW OAM SYSTEM
 *
 *  Revision 1.4  2011/08/21 09:07:50  hhbaek
 *  Commit TAF/SRC/ *
 *
 *  Revision 1.3  2011/08/17 07:17:33  dcham
 *  *** empty log message ***
 *
 *  Revision 1.2  2011/08/08 11:05:41  uamyd
 *  modified block added
 *
 *  Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 *  init DQMS2
 *
 *  Revision 1.2  2011/01/11 04:09:06  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2010/08/23 01:13:02  uamyd
 *  DQMS With TOTMON, 2nd-import
 *
 *  Revision 1.1.1.1  2009/05/26 02:14:36  dqms
 *  Init TAF_RPPI
 *
 *  Revision 1.1  2008/06/17 12:17:04  dark264sh
 *  init
 *
 */
          
