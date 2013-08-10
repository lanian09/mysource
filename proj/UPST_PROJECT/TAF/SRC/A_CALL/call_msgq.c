/**     @file   call_msgq.c
 *      - Call Session�� ���� �ϴ� ���μ���
 *
 *      Copyright (c) 2006~ by Upresto Inc. Korea
 *      All rights reserved
 *
 *       $Id: call_msgq.c,v 1.2 2011/09/04 08:04:25 dhkim Exp $
 *
 *      @Author     $Author: dhkim $
 *      @version    $Revision: 1.2 $
 *      @date       $Date: 2011/09/04 08:04:25 $
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
#include <sys/time.h>

// LIB
#include "typedef.h"
#include "loglib.h"
#include "mems.h"
#include "gifo.h"
#include "cifo.h"

#include "Analyze_Ext_Abs.h"

// PROJECT
#include "procid.h"
#include "capdef.h"
#include "common_stg.h"

// .
#include "call_msgq.h"
#include "call_func.h"
#include "call_utils.h"

extern stCIFO *gpCIFO;
extern stMEMSINFO *pMEMSINFO;

extern S32 	   guiSeqProcID;

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
S32 dSend_CALL_Data(stMEMSINFO *pMEMSINFO, S32 dSeqProcID, U8 *pNode)
{

#ifdef MEM_TEST
	nifo_node_delete(pMEMSINFO, pNode);
#else
	S32 	dRet;
    int     type;


	if(pNode != NULL) {
		type = ((TLV *)(pNode + NIFO_SIZE))->type;
		log_print(LOGN_INFO, "!!!!!!!!!!!!! MSG TO SEQ_PROC_ID[%d] TYPE[%d][%s] LEN[%d]",
				dSeqProcID, type,
				((type==START_CALL_NUM || type==STOP_CALL_NUM || type==RADIUS_START_NUM || type==LOG_PISIGNAL_DEF_NUM) ? PRINT_TAG_DEF_ALL_CALL_INPUT(type) : PRINT_DEF_NUM_table_log(type)),
				((TLV *)(pNode + NIFO_SIZE))->len);
	} else {
		log_print(LOGN_INFO, "************* pNode is NULL[%p] SEQ_PROC_ID[%d] pMEM[%p]", pNode, dSeqProcID, pMEMSINFO);
	}
		
	if( gifo_write(pMEMSINFO, gpCIFO, guiSeqProcID, dSeqProcID, nifo_offset(pMEMSINFO,pNode)) < 0 ){
		log_print(LOGN_CRI, "[%s][%s.%d] gifo_write to Target=%d, dRet[%d][%s]", 
			__FILE__, __FUNCTION__, __LINE__, dSeqProcID, dRet, strerror(-dRet));
		return -1;
	}
#endif
	return 0;
}


S32 Send_Call_Session_LOG(CALL_SESSION_HASH_DATA *pCALL_SESSION_HASH_DATA)
{
	S32				dRet;

	LOG_COMMON		*pLOG_COMMON;
	LOG_CALL_TRANS	*pLOG_CALL_TRANS;
	struct timeval  stNowTime;
	U8				*pLOGCALLNODE;

	pLOG_COMMON = &pCALL_SESSION_HASH_DATA->aLOG_COMMON;
	pLOG_CALL_TRANS = pCALL_SESSION_HASH_DATA->pLOG_CALL_TRANS;
	if(!pLOG_CALL_TRANS){ /* ???  LOGN_CRI �߰� */ return 0; }

	log_print(LOGN_DEBUG, "@@@ Send_Call CIP[%d.%d.%d.%d]", HIPADDR(pLOG_COMMON->uiClientIP) );

	memcpy((char *) pLOG_CALL_TRANS , (char *) pLOG_COMMON , LOG_COMMON_SIZE);

	/* pLOG_CALL_TRANS�� ������ �����ϴ� �κ� */
	gettimeofday(&stNowTime, NULL);
	pLOG_CALL_TRANS->OpEndTime = stNowTime.tv_sec;
	pLOG_CALL_TRANS->OpEndMTime = stNowTime.tv_usec;
	ASSOCIATION_OTHERS_LOG_CALL_TRANS_Once_At_Last(pLOG_CALL_TRANS);


//	LOG_CALL_TRANS_Prt("Send CALL LOG", pLOG_CALL_TRANS);
	/* pLOGCALLNODE = LOG CALL TRANS node�� ã�´�.  */
	pLOGCALLNODE = nifo_ptr(pMEMSINFO, nifo_get_offset_node(pMEMSINFO, (U8 *)pLOG_CALL_TRANS));

	if(pLOG_CALL_TRANS->FirstPlatformType == 0) { for_debugging(); }

	if((dRet = dSend_CALL_Data(pMEMSINFO, SEQ_PROC_CI_LOG, pLOGCALLNODE)) < 0) {
		log_print(LOGN_CRI, "[%s][%s.%d] MSGQ WRITE FAILE[%d][%s]", __FILE__, __FUNCTION__, __LINE__, dRet, strerror(-dRet));
		return -5;
	}
	pCALL_SESSION_HASH_DATA->pLOG_CALL_TRANS = NULL;
	pCALL_SESSION_HASH_DATA->offset_CALL = 0;


	return 0;
}


S32 Send_Page_Session_LOG(void *p)
{
	S32				dRet;
	LOG_COMMON		*pLOG_COMMON;
	LOG_PAGE_TRANS	*pLOG_PAGE_TRANS;
	struct timeval  stNowTime;
	U8				*pLOGPAGENODE;
	CALL_SESSION_HASH_DATA *pCALL_SESSION_HASH_DATA;

	pCALL_SESSION_HASH_DATA = p;

	pLOG_COMMON = &pCALL_SESSION_HASH_DATA->aLOG_COMMON;
	pLOG_PAGE_TRANS = pCALL_SESSION_HASH_DATA->pLOG_PAGE_TRANS;
	if(!pLOG_PAGE_TRANS){ /* ???  LOGN_CRI �߰� */ return 0; }

	log_print(LOGN_DEBUG, "@@@ Send_Page TIMEOUT CIP[%d.%d.%d.%d]", HIPADDR(pLOG_COMMON->uiClientIP) );

//	memcpy((char *) pLOG_PAGE_TRANS , (char *) pLOG_COMMON , LOG_COMMON_SIZE);

	/* pLOG_PAGE_TRANS�� ������ �����ϴ� �κ� */
	pLOG_PAGE_TRANS->LastUserErrorCode = Make_Page_UserError(pLOG_PAGE_TRANS);
	gettimeofday(&stNowTime, NULL);
	pLOG_PAGE_TRANS->OpEndTime = stNowTime.tv_sec;
	pLOG_PAGE_TRANS->OpEndMTime = stNowTime.tv_usec;
	ASSOCIATION_OTHERS_LOG_PAGE_TRANS_Once_At_Last(pLOG_PAGE_TRANS);

	if(pCALL_SESSION_HASH_DATA->aPAGE_DATA.pBODY != NULL){
		//      pTHIS->aPAGE_DATA.pBODY �޸� ����
		U8 *__pNODE;
		__pNODE = nifo_ptr(pMEMSINFO, nifo_get_offset_node(pMEMSINFO, (U8 *) pCALL_SESSION_HASH_DATA->aPAGE_DATA.pBODY));
//		nifo_node_unlink_nont(pMEMSINFO, __pNODE);
//		nifo_cont_delete(pMEMSINFO, __pNODE);
		nifo_node_delete(pMEMSINFO, __pNODE);
		pCALL_SESSION_HASH_DATA->aPAGE_DATA.pBODY = NULL;
		pCALL_SESSION_HASH_DATA->aPAGE_DATA.offset_BODY = 0;
	}

#if 0		/* �����ִ°� ���� ������ ������.
pInBODY�� ������ BODY pointer�̹Ƿ� ,
Send�� ���� �Ͱ��� ���� �����ϸ� ,
���� ��� NEW_PAGE�� ������ state�鿡�� pInBODY�� pBODY�� �ٿ��δ� ������ ��ĥ�� �ִ�.
free�� state�� ���������� �׻� �ϰ�, session�� ������ ��ü�� free���ֹǷ� ������ �߻����� �ʴ´�
*/
	if(pCALL_SESSION_HASH_DATA->aPAGE_DATA.pInBODY != NULL){
		//      pTHIS->aPAGE_DATA.pInBODY �޸� ����
		U8 *__pNODE;
		__pNODE = nifo_ptr(pMEMSINFO, nifo_get_offset_node(pMEMSINFO, (U8 *) pCALL_SESSION_HASH_DATA->aPAGE_DATA.pInBODY));
		nifo_node_unlink_nont(pMEMSINFO, __pNODE);
		nifo_cont_delete(pMEMSINFO, __pNODE);
		pCALL_SESSION_HASH_DATA->aPAGE_DATA.pInBODY = NULL;
	}
#endif

//	LOG_PAGE_TRANS_Prt("Send PAGE LOG", pLOG_PAGE_TRANS);
	/* pLOGPAGENODE = LOG PAGE TRANS node�� ã�´�.  */
	pLOGPAGENODE = nifo_ptr(pMEMSINFO, nifo_get_offset_node(pMEMSINFO, (U8 *)pLOG_PAGE_TRANS));
	if((dRet = dSend_CALL_Data(pMEMSINFO, SEQ_PROC_CI_LOG, pLOGPAGENODE)) < 0) {
		log_print(LOGN_CRI, "[%s][%s.%d] MSGQ WRITE FAILE[%d][%s]", __FILE__, __FUNCTION__, __LINE__, dRet, strerror(-dRet));
		return -5;
	}

	pCALL_SESSION_HASH_DATA->pLOG_PAGE_TRANS = NULL;
	pCALL_SESSION_HASH_DATA->offset_PAGE = 0;
	//  ������ LOG_PAGE_TRANS�� �޾��ش�. ������ ������ ���� �ٷ� �̿��Ҽ� �ְ� ���ֱ� ���ؼ��̴�. 
	// ??? Invoke_del_CALL()������ ���� ���� �۾��� �ɼ� �ִ�. (���� ��) 
	if(pCALL_SESSION_HASH_DATA->pLOG_PAGE_TRANS == NULL){
		struct timeval  stNowTime;

		log_print(LOGN_INFO, "%s : alloc LOG_PAGE_TRANS",(char *) __FUNCTION__);
		/* PAGE NODE �Ҵ� �� LOG_PAGE_TRANS TLV �Ҵ� */
		if((pLOGPAGENODE = nifo_node_alloc(pMEMSINFO)) == NULL) {
			log_print(LOGN_CRI, "ERROR : [%s][%s.%d] pLOG_PAGE_TRANS nifo_node_alloc NULL", __FILE__, __FUNCTION__, __LINE__);
			return -6;
		}
		if((pCALL_SESSION_HASH_DATA->pLOG_PAGE_TRANS = (LOG_PAGE_TRANS *) nifo_tlv_alloc(pMEMSINFO, pLOGPAGENODE, LOG_PAGE_TRANS_DEF_NUM, LOG_PAGE_TRANS_SIZE, DEF_MEMSET_ON)) == NULL) {
			log_print(LOGN_CRI, "ERROR : [%s][%s.%d] pLOG_PAGE_TRANS nifo_tlv_alloc NULL", __FILE__, __FUNCTION__, __LINE__);
			nifo_node_unlink_nont(pMEMSINFO, pLOGPAGENODE);
			nifo_cont_delete(pMEMSINFO, pLOGPAGENODE);
			return -7;
		}
		pCALL_SESSION_HASH_DATA->offset_PAGE = nifo_offset(pMEMSINFO, pLOGPAGENODE);
		/* PAGE_DATA �ʱ�ȭ */
		pCALL_SESSION_HASH_DATA->aPAGE_DATA.szLastURLLen = 0;
		pCALL_SESSION_HASH_DATA->aPAGE_DATA.szLastURL[0] = 0x00;
		pCALL_SESSION_HASH_DATA->aPAGE_DATA.LocationLen = 0;
		pCALL_SESSION_HASH_DATA->aPAGE_DATA.Location[0] = 0x00;

		/* pLOG_PAGE_TRANS ������ �ʱ�ȭ */
		gettimeofday(&stNowTime, NULL);
		pCALL_SESSION_HASH_DATA->pLOG_PAGE_TRANS->OpStartTime = stNowTime.tv_sec;
		pCALL_SESSION_HASH_DATA->pLOG_PAGE_TRANS->OpStartMTime = stNowTime.tv_usec;

		// Page�� state�� �ʱ�ȭ(INIT)    ==> �� �κ��� �Լ��� ��������� ��.
		// �Լ� ���� ���ڸ� STS_NEW_PAGE�� ������ ���⼭���� ����ϸ� �ڵ����� PAGE_STATE�� ���� ��.
		pCALL_SESSION_HASH_DATA->PAGE_OLD_STATE = 0;
		pCALL_SESSION_HASH_DATA->PAGE_STATE = STS_NEW_PAGE;

		// send �Ҷ� 
		pCALL_SESSION_HASH_DATA->pLOG_PAGE_TRANS->PageID = pCALL_SESSION_HASH_DATA->PAGE_ID;
		pCALL_SESSION_HASH_DATA->PAGE_ID ++;

		memcpy((char *)pCALL_SESSION_HASH_DATA->pLOG_PAGE_TRANS, &pCALL_SESSION_HASH_DATA->aLOG_COMMON, LOG_COMMON_SIZE);
	} 

	return 0;
}

S32 Send_Dialup_Session_LOG(CALL_SESSION_HASH_DATA *pCALL_SESSION_HASH_DATA)
{
	S32					dRet;

	LOG_DIALUP_SESS		*pLOG_DIALUP_SESS;

	struct timeval		stNowTime;
	U8					*pLOGDIALUPNODE;

	pLOG_DIALUP_SESS = pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS;
	if(!pLOG_DIALUP_SESS){ /* LOGN_CRI �߰� */ return 0; }

	log_print(LOGN_DEBUG, "@@@ Send_Dialup CIP: %d.%d.%d.%d", HIPADDR(pLOG_DIALUP_SESS->uiClientIP) );

	/* pLOG_DIALUP_SESS �� ������ �����ϴ� �κ� */
	gettimeofday(&stNowTime, NULL);
	pLOG_DIALUP_SESS->SessEndTime = pLOG_DIALUP_SESS->LastPktTime;
	pLOG_DIALUP_SESS->SessEndMTime = pLOG_DIALUP_SESS->LastPktMTime;
	pLOG_DIALUP_SESS->OpEndTime = stNowTime.tv_sec;
	pLOG_DIALUP_SESS->OpEndMTime = stNowTime.tv_usec;

//	LOG_DIALUP_SESS_Prt("SEND DIALUP LOG", pLOG_DIALUP_SESS);
	pLOGDIALUPNODE = nifo_ptr(pMEMSINFO, nifo_get_offset_node(pMEMSINFO, (U8 *)pLOG_DIALUP_SESS));

	if((dRet = dSend_CALL_Data(pMEMSINFO, SEQ_PROC_CI_LOG, pLOGDIALUPNODE)) < 0) {
		log_print(LOGN_CRI, "[%s][%s.%d] MSGQ WRITE FAILE[%d][%s]", __FILE__, __FUNCTION__, __LINE__, dRet, strerror(-dRet));
		return -5;
	}
	pCALL_SESSION_HASH_DATA->pLOG_DIALUP_SESS = NULL;
	pCALL_SESSION_HASH_DATA->offset_DIALUP = 0;

	return 0;
}

void dSend_Service_Start_Signal(CALL_SESSION_HASH_DATA *pCALLSESSHASHDATA, STIME SvcStartTime, MTIME SvcStartMTime)
{
	int 			dRet;
	LOG_COMMON 		*pSTARTSERVICE;
	UCHAR 			*pSTARTNODE;

	if( (pSTARTNODE = nifo_node_alloc(pMEMSINFO)) == NULL) {
		log_print(LOGN_CRI, "[%s.%d] nifo_node_alloc is NULL", __FUNCTION__, __LINE__);
	} else {
		if( (pSTARTSERVICE = (LOG_COMMON *)nifo_tlv_alloc(
						pMEMSINFO, pSTARTNODE, START_SERVICE_DEF_NUM, LOG_COMMON_SIZE, DEF_MEMSET_OFF)) == NULL ) {
			log_print(LOGN_CRI, "[%s.%d] nifo_tlv_alloc is NULL", __FUNCTION__, __LINE__);
			nifo_node_delete(pMEMSINFO, pSTARTNODE);
		} else {
			memcpy(pSTARTSERVICE, &pCALLSESSHASHDATA->aLOG_COMMON, LOG_COMMON_SIZE);
			pSTARTSERVICE->uiCallTime = SvcStartTime;
			pSTARTSERVICE->uiCallMTime = SvcStartMTime;
//			LOG_COMMON_Prt("START SERVICE SIGNAL", pSTARTSERVICE);
			if((dRet = dSend_CALL_Data(pMEMSINFO, SEQ_PROC_CI_LOG, pSTARTNODE)) < 0) {
				log_print(LOGN_CRI, "[%s] ERROR [%s.%d] MSGQ WRITE FAILE[%d][%s]", __FILE__, __FUNCTION__, __LINE__, 
						dRet, strerror(-dRet));
				nifo_node_delete(pMEMSINFO, pSTARTNODE);
			} else {
				log_print(LOGN_DEBUG, "	SEND START SERVICE SIGNAL IP[%d.%d.%d.%d] IMSI[%s] TIME[%u.%u]", 
						HIPADDR(pSTARTSERVICE->uiClientIP), pSTARTSERVICE->szIMSI, pSTARTSERVICE->uiCallTime, pSTARTSERVICE->uiCallMTime);
			}
		}
	}
}

void dSend_INET_Signal(CALL_SESSION_HASH_DATA *pCALLSESSHASHDATA, STIME SvcStartTime, MTIME SvcStartMTime, int type)
{
	int 			dRet;
	LOG_SIGNAL 		*pSTARTSERVICE;
	UCHAR 			*pSTARTNODE;

	if( (pSTARTNODE = nifo_node_alloc(pMEMSINFO)) == NULL) {
		log_print(LOGN_CRI, "[%s.%d] nifo_node_alloc is NULL", __FUNCTION__, __LINE__);
	} else {
		if( (pSTARTSERVICE = (LOG_SIGNAL *)nifo_tlv_alloc(
						pMEMSINFO, pSTARTNODE, type, LOG_SIGNAL_SIZE, DEF_MEMSET_OFF)) == NULL ) {
			log_print(LOGN_CRI, "[%s.%d] nifo_tlv_alloc is NULL", __FUNCTION__, __LINE__);
			nifo_node_delete(pMEMSINFO, pSTARTNODE);
		} else {
			memcpy(pSTARTSERVICE, &pCALLSESSHASHDATA->aLOG_COMMON, LOG_COMMON_SIZE);
			pSTARTSERVICE->uiCallTime = SvcStartTime;
			pSTARTSERVICE->uiCallMTime = SvcStartMTime;
			pSTARTSERVICE->uiProtoType = type;
			pSTARTSERVICE->uiNasIP = pCALLSESSHASHDATA->uiPDSNIP;

//			LOG_COMMON_Prt("START_PI_DATA_RECALL_NUM", pSTARTSERVICE);
			if((dRet = dSend_CALL_Data(pMEMSINFO, SEQ_PROC_CI_LOG, pSTARTNODE)) < 0) {
				log_print(LOGN_CRI, "[%s] ERROR [%s.%d] MSGQ WRITE FAILE[%d][%s]", __FILE__, __FUNCTION__, __LINE__, dRet, strerror(-dRet));
				nifo_node_delete(pMEMSINFO, pSTARTNODE);
			} else {
				log_print(LOGN_DEBUG, "	SEND %s IP[%d.%d.%d.%d] IMSI[%s] TIME[%u.%u]", 
						getSigString(type), HIPADDR(pSTARTSERVICE->uiClientIP), pSTARTSERVICE->szIMSI, pSTARTSERVICE->uiCallTime, pSTARTSERVICE->uiCallMTime);
			}
		}
	}
}

void Send_Clear_Msg(UINT dSeqProcID, UINT uiClientIP, INT LastPktTime)
{
	int 			dRet;
	st_CALL_KEY 	*pCALLKEY;
	UCHAR 			*pCALLNODE;


	if( (pCALLNODE = nifo_node_alloc(pMEMSINFO)) == NULL) {
		log_print(LOGN_CRI, "[%s.%d] nifo_node_alloc is NULL", __FUNCTION__, __LINE__);
	} else {
		if( (pCALLKEY = (st_CALL_KEY *)nifo_tlv_alloc(
						pMEMSINFO, pCALLNODE, CLEAR_CALL_NUM, STOP_CALL_KEY_SIZE, DEF_MEMSET_OFF)) == NULL ) {
			log_print(LOGN_CRI, "[%s.%d] nifo_tlv_alloc is NULL", __FUNCTION__, __LINE__);
			nifo_node_delete(pMEMSINFO, pCALLNODE);
		} else {
			pCALLKEY->uiClientIP = uiClientIP;
			pCALLKEY->uiReserved = LastPktTime;

			if((dRet = dSend_CALL_Data(pMEMSINFO, dSeqProcID, pCALLNODE)) < 0) {
				log_print(LOGN_CRI, "[%s] ERROR [%s.%d] MSGQ WRITE FAILE[%d][%s]",
						__FILE__, __FUNCTION__, __LINE__, dRet, strerror(-dRet));
				nifo_node_delete(pMEMSINFO, pCALLNODE);
			} else {
				log_print(LOGN_INFO, "SEND CALL CLEAR MESSAGE TO MSGQ:%u IP:%d.%d.%d.%d LastPktTime: %d", 
						dSeqProcID, HIPADDR(pCALLKEY->uiClientIP), pCALLKEY->uiReserved);
			}
		}
	}
}



/**
 *  $Log: call_msgq.c,v $
 *  Revision 1.2  2011/09/04 08:04:25  dhkim
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2011/08/29 05:56:42  dcham
 *  NEW OAM SYSTEM
 *
 *  Revision 1.3  2011/08/18 04:18:31  hhbaek
 *  *** empty log message ***
 *
 *  Revision 1.2  2011/08/17 07:15:03  dcham
 *  *** empty log message ***
 *
 *  Revision 1.1.1.1  2011/08/05 00:27:17  uamyd
 *  init DQMS2
 *
 *  Revision 1.3  2011/01/11 04:09:06  uamyd
 *  modified
 *
 *  Revision 1.1.1.1  2010/08/23 01:13:01  uamyd
 *  DQMS With TOTMON, 2nd-import
 *
 *  Revision 1.2  2009/08/15 21:04:57  pkg
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
