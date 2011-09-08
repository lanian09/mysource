/**********************************************************
   Author   : 
   Revision : 

   Description:
   Copyright (c) , Inc.
***********************************************************/
#include <smpp.h>
#include "comm_msgtypes.h"


extern 	SMPP_CID  		smpp_cid[MAX_SMPP_CID];
extern 	SMPP_FD_TBL 	client;
extern 	SMS_HIS     	sms_his;
extern 	MYSQL			sql, *conn;
extern  SMPP_MSG_INFO 	smpp_msg_info;
extern  int     		smppQid;
extern MPTimer         	*gpCurMPTimer;
// TRACE_INFO.conf ����ü 
extern st_SESSInfo		*gpCurTrc; // CURRENT OPERATION pointer

st_SESSInfo       g_stTrcInfo;

int send_sms_toSmc(int smcid, SMS_HIS *sms_hist)  
{
	SMPP_DELIVER	smpp;
	int 		err, ret=0;
	time_t		now;
	SMS_INFO	*rcv_msg = &sms_hist->info;

	memset(&smpp, 0x00, sizeof(SMPP_DELIVER));

	if (strlen(rcv_msg->smsMsg) == 0) {
		dAppLog(LOG_CRI, "[send_smpp_sm] sm_data length zero imsi=%s", rcv_msg->subsID);
		return eP_DataErr;
	}
    if ((err = make_smpp_DELIVER (sms_hist, &smpp_msg_info,  &smpp)) < 0)
	{
		sms_hist->delivSts = SMPP_ERR_NORMAL;
		ret = smpp_insertDB(sms_hist);
		if( ret < 0 ) {
			dAppLog(LOG_CRI, "[send_sms_toSmc] sms syntax error!!, smpp_err insert query fail..");
			return -1;
		}
		return err;
	}

	err = write_sock_data(smcid, (char*)&smpp, sizeof(SMPP_DELIVER));

	/* ���ۿ� ������ ��츸 DB�� INNSERT �ϰ� SMPP LOG�� ����� */
	if (err >= 0) {
		Trace_DELIVER_MSG (sms_hist, &smpp);
		ret = smpp_insertDB(sms_hist);
		if( ret < 0 ) {
			dAppLog(LOG_CRI, "[send_sms_toSmc] write_sock_data success, but insert query fail..");
			return -1;
		}
#ifdef DEBUG
		dAppLog(LOG_DEBUG, " @@@ SEND DELIBER MSG(SN=%d)", smpp.sn);
#endif
#if 0 /* by june */
		smpp_log(SEND_TO_SMSC, "DELIVER", smcid, sms_hist->cid, sms_hist, &smpp_msg_info); 
#endif
	} else {
		sms_hist->delivSts = SMPP_ERR_WRITE;
		ret = smpp_insertDB(sms_hist);
		if( ret < 0 ) {
			dAppLog(LOG_CRI, "[send_sms_toSmc] write_sock_data success, but insert query fail..");
			return -1;
		}
		ClearCid(sms_hist->cid);
		return -1;	
	}

	smpp_cid[sms_hist->cid].cid = sms_hist->cid;
	smpp_cid[sms_hist->cid].conidx = smcid;
	smpp_cid[sms_hist->cid].prototype = 0;	/* ����� ������� ���� */
	smpp_cid[sms_hist->cid].starttime =  time(&now);
	
	return 0;
}

/*
   unsigned short  sendFlag;
   unsigned short  pkgID;
   unsigned short  sPBit;
   unsigned short  sHBit;
   unsigned int    blkTm;
   unsigned char   subsID[SMS_MAX_SUBSID_LEN];
   unsigned char   smsMsg[SMPP_MSG_TEXT_LEN];
*/
void smpp_dumpSmppMsg(SMS_INFO *rcv_msg)
{
	dAppLog(LOG_DEBUG, " @@@ RECV Block Information @@@");
	dAppLog(LOG_DEBUG, " PKG_ID = %d  P/H:%02d %02d", rcv_msg->pkgID, rcv_msg->sPBit, rcv_msg->sHBit);
	dAppLog(LOG_DEBUG, " BLK_TM = %d", rcv_msg->blkTm);
	dAppLog(LOG_DEBUG, " SUBSID = %s", rcv_msg->subsID);
	dAppLog(LOG_DEBUG, " SMSMSG = %s(len %d)", rcv_msg->smsMsg, strlen(rcv_msg->smsMsg));
	dAppLog(LOG_DEBUG, "BLKINFO RECV] [MIN:%s][P/H:%02d %02d][BLK_TM:%d][MSG:%s][LEN:%d"
			, rcv_msg->subsID, rcv_msg->sPBit, rcv_msg->sHBit
			, rcv_msg->blkTm, rcv_msg->smsMsg, strlen(rcv_msg->smsMsg));
}


void recv_smpp_cmd(IxpcQMsgType *rxIxpcMsg)
{
	int		err, ret;
	int 	smcid;
	int	duration = 0;
	MYSQL_RES   *result;
	MYSQL_ROW   row;
	int cid=-1;
	SMS_INFO 	*rcv_msg;

	rcv_msg = (SMS_INFO *)rxIxpcMsg->body;

	smpp_dumpSmppMsg(rcv_msg);
#if 0
	Trace_BLKMSG (rcv_msg);
#endif
	memset(&sms_his, 0x00, sizeof(SMS_HIS));
	memcpy(&sms_his, rcv_msg, sizeof(SMS_INFO));
#if 0
	sms_his.delivTm   = 0;
	sms_his.delivSts  = 0;
	sms_his.reportSts = 0;
	memset(sms_his.reportTm, 0x00, sizeof(char)*SMPP_MSG_DELIVER_TIME_LEN);
#endif
	/* SMSC �� �ϳ��� TCP Session �� �����Ѵ�. ���Ŀ� �ټ��� SMSC���� ���� Ȯ���� ���� �����̴�. */
	smcid = ONE_SMC_INFO;
	if (client.condata[smcid].fd == 0 || client.condata[smcid].bind == 0)  {
		dAppLog(LOG_CRI, "[recv_smpp_cmd]  SMSC Status Invalid: MDN=%s SMSCID=%d FD=%d BIND=%d"
				, rcv_msg->subsID, smcid, client.condata[smcid].fd, client.condata[smcid].bind);
		return;
		/* ������ ���� socket���� ���¸� Ȯ���ؼ� ������ �ȵ� ��� ��� �ؾ� �ұ�.
		 * �ϴ���, �׳� ����
		 * �� ��� ����ڰ� ���� ���񽺸� ����ϸ�?? �ΰ��� �κ��̶� ���� ���ؾ� �� 
		 * ���: ����� Package ID�� ������ Rule Set���� Block �� ����Ÿ ���񽺴� 
		 *       ������� action�� ���ؼ� �¿�Ǵ°��� �ƴ϶� ������ ���ǰ� 
		 *       ����ڿ��Դ� notify ������ �ǹ̷� SMS ���� �� ������ ������ �׳� ������ �ȴ�. 
		 *       ������ ���� ������� action �� ���� ��û ����Ÿ ���񽺰� redirection �Ǹ� 
		 *       ��� ���� �� ����ؾ� �Ѵ� */
	}

	/* DB SELECT BY SUBSCRIBER ID and SMS-SEND-TIME */
	ret = smpp_selectDB(sms_his.info.subsID);
	if (ret < 0) {
		/* select query �� ����. �����ڰ� table�� �ִ��� ������ �𸣴� ���� */
		dAppLog(LOG_CRI, "[recv_smpp_cmd] smpp_selectDB query fail.");
		return;
	}

	result = mysql_store_result(conn);
	row = mysql_fetch_row(result);
	/* CASE NOT TO BE SUBSCRIBERS ********************************************************************/
	/* DB INSERT SMS HISTORY */
	if ( row == NULL ) {
		/* Create Call ID */
		cid = NewCid();
		if (cid <= 0) {
			dAppLog(LOG_CRI, "[recv_smpp_cmd] NewCid Fail: MDN=0%s", rcv_msg->subsID);
			return;
		}
		sms_his.cid 	= cid;
		sms_his.delivTm	= time(0);
#ifdef DEBUG
		dAppLog(LOG_DEBUG, "[recv_smpp_cmd] NEW SMS MSG : To send msg");
#endif
	}

	/* CASE TO BE SUBSCRIBER    ********************************************************************/
	else {
		/* row[0] : Select ���� ù��° Į�� -> SUBS_ID ���� ��� �ִ�. 
		** row[1] : Select ���� �ι�° Į�� -> ���� �ð� - DELIV_TIME �� �ð� ���̸� ���� �ִ�. 
		*/
		duration = atoi(row[1]);
		if (duration < 0) duration = 0;

		dAppLog(LOG_DEBUG, "[recv_smpp_cmd] DB DuTime:%d  CONF DuTime:%u", duration, gpCurMPTimer->sms_timeout);

		/* DURATION and SMS-SEND-TIME �� */
		if ( duration > gpCurMPTimer->sms_timeout) { 
			/* Create Call ID */
			cid = NewCid();
			if (cid <= 0) {
				dAppLog(LOG_CRI, "[recv_smpp_cmd] NewCid Fail: MDN=0%s", rcv_msg->subsID);
				return;
			}
			sms_his.cid 	= cid;
			sms_his.delivTm = time(0);
		}
		else {
			/* ���� ��� - �Ⱥ����� */
			dAppLog(LOG_CRI, "Does not send sms, reason: The duration did not pass.");
			return;
		}
	}

	/* SMS MSG SEND TO SMSC */
	err = send_sms_toSmc(smcid, &sms_his); 
	if(err < 0) {
		dAppLog(LOG_CRI, "[recv_smpp_cmd]  send_smpp_smc fail (%s) SMSC=%d", rcv_msg->subsID, smcid);
		return;
	}
	return;
}

#if 0
void recv_smpp_cmd(SMS_INFO *rcv_msg) 
{
	int		i, err, ret;
	int 	root, smcid, slen;
	int	duration = 0;
	MYSQL_RES   *result;
	MYSQL_ROW   row;
	int cid=-1;

	smpp_dumpSmppMsg(rcv_msg);
	memset(&sms_his, 0x00, sizeof(SMS_HIS));
	memcpy(&sms_his, rcv_msg, sizeof(SMS_INFO));
#if 0
	sms_his.delivTm   = 0;
	sms_his.delivSts  = 0;
	sms_his.reportSts = 0;
	memset(sms_his.reportTm, 0x00, sizeof(char)*SMPP_MSG_DELIVER_TIME_LEN);
#endif
	/* SMSC �� �ϳ��� TCP Session �� �����Ѵ�. ���Ŀ� �ټ��� SMSC���� ���� Ȯ���� ���� �����̴�. */
	smcid = ONE_SMC_INFO;
	if (client.condata[smcid].fd == 0 || client.condata[smcid].bind == 0)  {
		dAppLog(LOG_CRI, "[recv_smpp_cmd]  SMSC Status Invalid: MDN=%s SMSCID=%d FD=%d BIND=%d"
				, rcv_msg->subsID, smcid, client.condata[smcid].fd, client.condata[smcid].bind);
		return;
		/* ������ ���� socket���� ���¸� Ȯ���ؼ� ������ �ȵ� ��� ��� �ؾ� �ұ�.
		 * �ϴ���, �׳� ����
		 * �� ��� ����ڰ� ���� ���񽺸� ����ϸ�?? �ΰ��� �κ��̶� ���� ���ؾ� �� 
		 * ���: ����� Package ID�� ������ Rule Set���� Block �� ����Ÿ ���񽺴� 
		 *       ������� action�� ���ؼ� �¿�Ǵ°��� �ƴ϶� ������ ���ǰ� 
		 *       ����ڿ��Դ� notify ������ �ǹ̷� SMS ���� �� ������ ������ �׳� ������ �ȴ�. 
		 *       ������ ���� ������� action �� ���� ��û ����Ÿ ���񽺰� redirection �Ǹ� 
		 *       ��� ���� �� ����ؾ� �Ѵ� */
	}

	/* DB SELECT BY SUBSCRIBER ID and SMS-SEND-TIME */
	ret = smpp_selectDB(sms_his.info.subsID);
	if (ret < 0) {
		/* select query �� ����. �����ڰ� table�� �ִ��� ������ �𸣴� ���� */
		dAppLog(LOG_CRI, "[recv_smpp_cmd] smpp_selectDB query fail.");
		return;
	}

	result = mysql_store_result(conn);
	row = mysql_fetch_row(result);
	/* CASE NOT TO BE SUBSCRIBERS ********************************************************************/
	/* DB INSERT SMS HISTORY */
	if ( row == NULL ) {
		/* Create Call ID */
		cid = NewCid();
		if (cid <= 0) {
			dAppLog(LOG_CRI, "[recv_smpp_cmd] NewCid Fail: MDN=0%s", rcv_msg->subsID);
			return;
		}
		sms_his.cid 	= cid;
		sms_his.delivTm	= time(0);
#ifdef DEBUG
		dAppLog(LOG_DEBUG, "[recv_smpp_cmd] NEW SMS MSG : To send msg");
#endif
	}

	/* CASE TO BE SUBSCRIBER    ********************************************************************/
	else {
		/* row[0] : Select ���� ù��° Į�� -> SUBS_ID ���� ��� �ִ�. 
		** row[1] : Select ���� �ι�° Į�� -> ���� �ð� - DELIV_TIME �� �ð� ���̸� ���� �ִ�. 
		*/
		duration = atoi(row[1]);
		if (duration < 0) duration = 0;

		if (gpMPTimer == NULL) {
			dAppLog(LOG_CRI, "[recv_smpp_cmd] timeout shm trouble"); return;
		}
		dAppLog(LOG_DEBUG, "[recv_smpp_cmd] DB DuTime:%d  CONF DuTime:%d", duration, gpMPTimer->sms_timeout);

		/* DURATION and SMS-SEND-TIME �� */
		if ( duration > gpMPTimer->sms_timeout) { 
			/* Create Call ID */
			cid = NewCid();
			if (cid <= 0) {
				dAppLog(LOG_CRI, "[recv_smpp_cmd] NewCid Fail: MDN=0%s", rcv_msg->subsID);
				return;
			}
			sms_his.cid 	= cid;
			sms_his.delivTm = time(0);
		}
		else {
			/* ���� ��� - �Ⱥ����� */
			dAppLog(LOG_CRI, "Does not send sms, reason: The duration did not pass.");
			return;
		}
	}

	/* SMS MSG SEND TO SMSC */
	err = send_sms_toSmc(smcid, &sms_his); 
	if(err < 0) {
		dAppLog(LOG_CRI, "[recv_smpp_cmd]  send_smpp_smc fail (%s) SMSC=%d", rcv_msg->subsID, smcid);
		return;
	}
	return;
}
#endif


void proc_msgQ_data(void)
{
	GeneralQMsgType     rxGenQMsg;
	IxpcQMsgType    	*rxIxpcMsg;
	NOTIFY_SIG			*pNOTIFY;
	SmsMsgType			rxSmsMsg;
	int     			msgq_size=0;

	msgq_size = sizeof(GeneralQMsgType) - sizeof(long);
	if((msgrcv(smppQid, &rxGenQMsg, msgq_size, 0, IPC_NOWAIT)) < 0) { // MSG_NOERROR IPC_NOWAIT
		if (errno != ENOMSG) {
			dAppLog(LOG_CRI, "[proc_msgQ_data] QID=%d MSGRCV ERRER:%d (%s)"
					, smppQid, errno, strerror(errno));
		}
		return;
	}
    	
    switch (rxGenQMsg.mtype) 
	{
	/* SMS processing */
    case MTYPE_STATUS_REPORT:
		dAppLog(LOG_CRI, "MTYPE_STATUS_REPORT Rcv");
		rxIxpcMsg = (IxpcQMsgType *)rxGenQMsg.body;
        recv_smpp_cmd ((IxpcQMsgType*)rxIxpcMsg);
        break;
	
	/* MMCD processing */
	case MTYPE_MMC_REQUEST:
		dAppLog(LOG_CRI, "MTYPE_MMC_REQUEST Rcv");
		rxIxpcMsg = (IxpcQMsgType *)rxGenQMsg.body;
    	recv_mmc ((IxpcQMsgType*)rxIxpcMsg);
    	break;

	/* TRACE processing
	 * - without data and just nofification
	 */
	case MTYPE_TRC_CONFIG:
		dAppLog(LOG_CRI, "MTYPE_TRC_CONFIG Rcv");
		rxIxpcMsg = (IxpcQMsgType *)rxGenQMsg.body;
		pNOTIFY = (NOTIFY_SIG *)rxIxpcMsg->body;
		dSetCurTrace(pNOTIFY);
		//dLoad_TrcConf(&g_stTrcInfo);
		break;

	case MTYPE_TIMER_CONFIG:
		dAppLog(LOG_CRI, "MTYPE_TRC_CONFIG Rcv");
		rxIxpcMsg = (IxpcQMsgType *)rxGenQMsg.body;
		pNOTIFY = (NOTIFY_SIG *)rxIxpcMsg->body;
		dSetCurTIMEOUT(pNOTIFY);
		break;

	default:
		dAppLog(LOG_CRI, "[proc_msgQ_data] Unknown message is received: mtype=%d"
				, rxSmsMsg.mtype); 
        break;
    }
    return;
}


#if 0
void proc_msgQ_data(void)
{
	GeneralQMsgType     rxGenQMsg;
	SmsMsgType			rxSmsMsg;
	int     			msgq_size=0;

#if 0		
    if (msgrcv(smppQid, &rxSmsMsg, sizeof(SmsMsgType), 0, IPC_NOWAIT) < 0) {
        if (errno != ENOMSG) {
			dAppLog(LOG_CRI, "[proc_msgQ_data] Receive MSGQ Fail : errno=%d(cause:%s)"
					, errno, strerror(errno)); 
        }
        return;
	}
#endif

	msgq_size = sizeof(GeneralQMsgType) - sizeof(long);
	if((msgrcv(smppQid, &rxGenQMsg, msgq_size, 0, IPC_NOWAIT)) < 0) { // MSG_NOERROR IPC_NOWAIT
		if (errno != ENOMSG) {
			dAppLog(LOG_DEBUG, "[proc_msgQ_data] QID=%d MSGRCV ERRER:%d[%s], "
					, smppQid, errno, strerror(errno));
		}
		return;
	}
    	
    switch (rxSmsMsg.mtype) {
	/* MMCD ��ɾ� ó�� */
	case MTYPE_MMC_REQUEST:
    	//recv_mmc ((IxpcQMsgType*)&rxSmsMsg.sms);
    	break;

	/* BLOCK RDR �� ���� SMS ó�� */
    case MTYPE_STATUS_REPORT:
        recv_smpp_cmd ((SMS_INFO *)&rxSmsMsg.sms);
        break;
    
	default:
		dAppLog(LOG_CRI, "[proc_msgQ_data] Unknown message is received: mtype=%d"
				, rxSmsMsg.mtype); 
        break;
    }
    	
    return;
}
#endif
