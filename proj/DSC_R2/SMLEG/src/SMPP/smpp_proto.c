/**********************************************************
   Author   : IN-Team
   Revision History : 
        2004. 01. 29    updated

   Description:

   Copyright (c) INFRAVALLEY, Inc.
***********************************************************/
#include <smpp.h>

extern  char    sysLabel[COMM_MAX_NAME_LEN], mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN];

extern 	SMPP_CID  	smpp_cid[MAX_SMPP_CID];
extern 	SMPP_FD_TBL client;
extern 	SMS_HIS     sms_his;

/* SUBMIT_SM Operation Format
** Header
	- command_length(4)		: set to overall length of PDU
	- command_id(4) 		: submit_sm
	- command_status(4) 	: no used set to NULL
	- sequence_number(4) 	: submit_sm_resp PDU 식별할 수 있는 ID

** Mandatory Parameters
	- service_type(variable. max 6)	: 
	- source_addr_ton(1) : type of number 
		# type of number value 
			0 - unknown
			1 - International
			2 - national
	- source_add_npi(1) : number plan indicator
	- source_addr(variable. max 21)
	- dest_add_ton(1)
	- dest_add_npi(1)
	- dest_addr(variable. max 21)
	- esm_class(1) : indicate msg type & mode
	- protocol_ID(1) : not used in CDMA
	- priority_flag(1) 
	- schedule_delivery_time(1 or 17) : set to NULL for immediate delivery
	- validity_period(1 or 17) : set to NULL to request SMSC default validity period
	- registered_delivery(1)
	- replace_if_present_flag(1)
	- data_coding(1) 
	- sm_default_msg_id(1)
	- sm_length(1)
	- short_message(variable 0~254)

** Optional Parameters - 각 parameter별로 TLV format
	- callback_num : Tag=0x0381
		Octet 1 : digit mode indicator
		Octet 2 : type of number
   		Octet 3 : number plan indicator
		Octet 4 ~ : callback number digits encoded as ASCII characters
	- route_dest_addr : Tag=0x1401
*/

int make_smpp_DELIVER(SMS_HIS *sms_hist, SMPP_MSG_INFO *msg_info,  SMPP_DELIVER *smpp)
{
	char 	tmpStr[SMPP_MSG_ADDR_LEN];

	//////////////////////////
	// SMPP BODY
	//////////////////////////
	/* TID */
	smpp->tid = htonl(msg_info->tid);
	/* ORGADDR */
	//strcpy(smpp->org_addr, msg_info->org_addr);
	sprintf(smpp->org_addr, "%s", msg_info->org_addr);
	/* DSTADDR */
	if(strlen(sms_hist->info.subsID) < SMPP_MSG_ADDR_LEN) {
		//tmpStr[0] = '0';
		strcpy(tmpStr, sms_hist->info.subsID+5);
#if 1
		sprintf(smpp->dst_addr, "0%s", tmpStr);
#endif
#if 0
		sprintf(smpp->dst_addr, "01080803122");
		//sprintf(smpp->dst_addr, "01024447290");
#endif
		//dAppLog (LOG_DEBUG, "DELIVER ADDR:%s", smpp->dst_addr);
		//strcpy(smpp->dst_addr, sms_hist->info.subsID);
	}
	else{
		dAppLog(LOG_CRI, "[make_smpp_DELIVER] subscriber ID is wrong");
		return -1;
	}

	/* CALLBACK */
	//strcpy(smpp->callback, msg_info->callback);
	sprintf(smpp->callback, "%s", msg_info->callback);
	/* TEXT */
	sprintf(smpp->text, "%s", sms_hist->info.smsMsg);
	dAppLog( LOG_DEBUG, " @@@ SMS Message:%s(size:%d)", smpp->text, strlen(smpp->text));
	//strcpy(smpp->text, sms_hist->info.smsMsg);
	//smpp->sn = htonl(sms_hist->cid);
	smpp->sn = sms_hist->cid;

	//////////////////////////
	// SMPP HEADER
	//////////////////////////
	smpp->header.type = SMPP_DELIVER_MSG;
	smpp->header.len = htonl(sizeof(SMPP_DELIVER)-sizeof(SMPP_MSG_H));

	dAppLog (LOG_DEBUG, " @@@ SEND  SMPP_DELIVER_MSG ");
	dAppLog (LOG_DEBUG, " HDR   : TYPE     = :%d", smpp->header.type);
	dAppLog (LOG_DEBUG, " HDR   : SIZE     = :%d", smpp->header.len);
	dAppLog (LOG_DEBUG, " BODY  : TID      = :%d", smpp->tid);
	dAppLog (LOG_DEBUG, " BODY  : ORG_ADDR = :%s", smpp->org_addr);
	dAppLog (LOG_DEBUG, " BODY  : DST_ADDR = :%s", smpp->dst_addr);
	dAppLog (LOG_DEBUG, " BODY  : CALLBACK = :%s", smpp->callback);
	dAppLog (LOG_DEBUG, " BODY  : TEXT     = :%s", smpp->text);
	dAppLog (LOG_DEBUG, " BODY  : SN       = :%d", smpp->sn);
	dAppLog (LOG_CRI, "SEND DELIVER = TID:%d ORG:%s DST:%s CALLBK:%s SN:%d MSG:%s"
		, smpp->tid, smpp->org_addr, smpp->dst_addr, smpp->callback, smpp->sn, smpp->text);
	return 0;	
}

void  make_smpp_BIND(int idx, SMPP_BIND *smpp)
{
	//////////////////////////
	// SMPP BODY
	//////////////////////////
    strcpy((char *)smpp->id, smc_tbl->smc_info[idx].user_id);
    strcpy((char *)smpp->pwd, smc_tbl->smc_info[idx].passwd);

	//////////////////////////
	// SMPP HEADER
	//////////////////////////
    smpp->header.type = SMPP_BIND_MSG;
    smpp->header.len  = htonl(sizeof(SMPP_BIND)-sizeof(SMPP_MSG_H)); 
	
	return;
}

void  make_smpp_REPORT_ACK(int code, SMPP_REPORT_ACK *smpp)
{
	//////////////////////////
	// SMPP BODY
	//////////////////////////
    smpp->result = code;

	//////////////////////////
	// SMPP HEADER
	//////////////////////////
    smpp->header.type = SMPP_REPORT_ACK_MSG;
    smpp->header.len  = htonl(sizeof(SMPP_REPORT_ACK)-sizeof(SMPP_MSG_H)); 
	
	return;
}

int protocol_bind_try(int idx)
{
	SMPP_BIND smpp;
	int 		err, cid;

	memset(&smpp, 0x00, sizeof(SMPP_BIND));
	make_smpp_BIND(idx, &smpp);
	//dAppLog(LOG_DEBUG, "########## user:%s pass:%s", smpp.id, smpp.pwd);

	err = write_sock_data(idx, (char *)&smpp, sizeof(SMPP_BIND));

	client.condata[idx].bindtry = SOCK_BIND_TRY_STS;

	if (err >= 0) {
		dAppLog(LOG_CRI, "[protocol_bind_try] Send Success: CID=%d SMSC=%d", cid, idx);
#if 0 /* by june */	
		smpp_log(SEND_TO_SMSC, "BIND", idx, cid, 0, 0, NULL, NULL);
#endif
	} else { 
		dAppLog(LOG_CRI, "[protocol_bind_try] bind send err: CID=%d SMSC=%d", cid, idx);
		ClearCid(cid);
	}	
	return err;
}

int read_sock_data_to_buffer(int idx)
{
    int rcvd_len, toread_len, msg_flag, body_len, rval;

	if (client.condata[idx].rbuf.rcv_len > 5300) {
		dAppLog(LOG_CRI, "RCV_lEN =%d\n", client.condata[idx].rbuf.rcv_len);
		return -1;
	}
    rcvd_len = client.condata[idx].rbuf.rcv_len;
    msg_flag = client.condata[idx].rbuf.msg_flag;
    body_len = client.condata[idx].rbuf.body_len;

	/* 읽을 data가 Head/Body 인지 결정. 읽을 Pointer 결정 */
    if (rcvd_len == 0) {
       	msg_flag = HEAD_READ_STS;
       	toread_len = SMPP_HEAD_LEN;
    } else {
        if (msg_flag == HEAD_READ_STS)  {
			toread_len = SMPP_HEAD_LEN - rcvd_len;
        } else if (msg_flag == BODY_READ_STS) {
            toread_len =  body_len - (rcvd_len - SMPP_HEAD_LEN);
        } else {
			dAppLog(LOG_CRI, "[read_sock_data_to_buffer] msg_flag Invalid: SMSC=%d MSG_FLAG=%d",
				idx, msg_flag);
            return -1;
        }
    }

    if (toread_len < 0) {
		dAppLog(LOG_CRI, "[read_sock_data_to_buffer] TO_Read Data Len Invalid: LEN=%d SMSC=%d",
			toread_len, idx);   
        return -1;
	}

	if (toread_len != 0) {
		rval = read_socket(idx, toread_len, &client.condata[idx].rbuf.buf[rcvd_len]);
		if (rval < 0) {
			return rval;
		} else if (rval > 1) {
			/* toread_len만큼 다 못 읽은 경우 */
			client.condata[idx].rbuf.rcv_len += (rval -2);
			return rval;
		} 
	}

	/* 2004-02-02. yrpark. toread_len이 0인 경우에도 수행함 -> to be corrected */
    client.condata[idx].rbuf.rcv_len += toread_len;
    if (msg_flag == HEAD_READ_STS) {
        client.condata[idx].rbuf.msg_flag = BODY_READ_STS;
    } else {
        client.condata[idx].rbuf.msg_flag = HEAD_READ_STS;
	}
    return 0;
}

int check_smpp_head_msg(int idx) 
{
	SMPP_MSG_H	msg_h;

	memcpy(&msg_h, client.condata[idx].rbuf.buf, SMPP_HEAD_LEN);

	if ((msg_h.type<SMPP_BIND_MSG)||(msg_h.type>SMPP_REPORT_ACK_MSG)){
		dAppLog(LOG_CRI, "[check_smpp_head_msg] smpp msg type Invalid: SMSC=%d, type=%d"
				, idx, msg_h.type); 
		return -1;
	}

	/* msg_h.cmd_len이 Head + Body Len을 나타냄 */
	client.condata[idx].rbuf.body_len = ntohl(msg_h.len);
	return 0;
}


int read_complete_msg(int idx)
{
    int 	rval, body_len;

READ_SOCK:
	rval = read_sock_data_to_buffer(idx);
	if (rval < 0) {
		client.condata[idx].rbuf.rcv_len = 0;
		client.condata[idx].rbuf.body_len = 0;
		client.condata[idx].rbuf.msg_flag = HEAD_READ_STS;
		return rval;
	}
	else if (rval > 1) return rval;

	if (client.condata[idx].rbuf.msg_flag == BODY_READ_STS) {
		/* HEAD 부분을 읽은 경우 */
		if (check_smpp_head_msg(idx) < 0) {
			return eP_Sock_Disconnect;
		} else  {
			/* Body를 읽어오기 위해 retry read */
			goto READ_SOCK;
		}
	} else {
		/* BODY까지 다 읽은 경우 break */
		//fprintf(stderr, "read body complete!!\n");
		// break;
	}

    body_len = client.condata[idx].rbuf.body_len;
   	client.condata[idx].rbuf.buf[body_len+SMPP_HEAD_LEN] = 0;

	/* 다음 socket_data 읽기 위한 초기화 */
    client.condata[idx].rbuf.rcv_len = 0;
    client.condata[idx].rbuf.msg_flag = HEAD_READ_STS;
    return 0;
}


int check_smpp_ack(int idx)
{
	int			cid, ret;
	SMPP_MSG_H	msg_h;

	memcpy(&msg_h, client.condata[idx].rbuf.buf, SMPP_HEAD_LEN);

	switch (msg_h.type)
	{
	case SMPP_BIND_ACK_MSG:
		{
			SMPP_BIND_ACK ack;

			memcpy(&ack, &(client.condata[idx].rbuf.buf), SMPP_HEAD_LEN+msg_h.len); /*msg body copy */

			if (client.condata[idx].bind != 1) {
				client.condata[idx].bind = 1;
				dAppLog(LOG_CRI, "[check_smpp_ack] BIND State Change: SMSC=%d", idx); 
			}
			dAppLog(LOG_DEBUG, "RECV SMPP_BIND_ACK_MSG");
		}
		break;
	case SMPP_DELIVER_ACK_MSG:
		{
			SMPP_DELIVER_ACK ack;

			dAppLog(LOG_DEBUG, "RECV SMPP_DELIVER_ACK_MSG");
			memcpy(&ack, &(client.condata[idx].rbuf.buf), SMPP_HEAD_LEN+msg_h.len); /*msg body copy */

			dAppLog(LOG_DEBUG, " HEADER : TYPE     = %d", ack.header.type);
			dAppLog(LOG_DEBUG, " HEADER : LEN      = %d", ack.header.len);
			dAppLog(LOG_DEBUG, " BODY   : RESULT   = %d", ack.result);
			dAppLog(LOG_DEBUG, " BODY   : ORG_ADDR = %s", ack.org_addr);
			dAppLog(LOG_DEBUG, " BODY   : DST_ADDR = %s", ack.dst_addr);
			dAppLog(LOG_DEBUG, " BODY   : SN       = %d", ack.sn);

			dAppLog (LOG_CRI, "RECV DELIVER ACK = RST:%d ORG:%s DST:%s SN:%d "
					, ack.result, ack.org_addr, ack.dst_addr, ack.sn);
#if 0
			cid = ntohl(ack.sn);
			if (cid < 0 || cid >= MAX_SMPP_CID) {
				dAppLog(LOG_CRI, "[check_smpp_ack] CID Invalid: CID=%d SMSC=%d\n", cid, idx); 
				return -1;
			}

			if (smpp_cid[cid].cid != cid) {
				dAppLog(LOG_CRI, "[check_smpp_ack] Return CID Mismatch: RCV_CID=%d SEND_CID=%d SMSC=%d"
						, cid, smpp_cid[cid].cid, idx);
				return eP_CID;
			}
#endif
			sms_his.delivSts = ack.result;
			sms_his.cid = ack.sn;

			/* result code 별 처리해야 한다. */
			switch(sms_his.delivSts)
			{
			case DELIVER_E_OK:
			case DELIVER_E_SENT:
				break;
			case DELIVER_E_SYSFAIL:
			case DELIVER_E_AUTH_FAIL:
			case DELIVER_E_FORMAT_ERR:
			case DELIVER_E_NOT_BOUND:
			case DELIVER_E_NO_DESTIN:
			case DELIVER_E_EXPIRED:
			case DELIVER_E_INVALID_TERM:
			case DELIVER_E_OVERFLOW:
				ClearCid(cid);
				break;
			default:
				dAppLog(LOG_CRI, "[check_smpp_ack] SMPP_DELIVER_ACK_MSG Unkown result code(%d)"
						, sms_his.delivSts);
			}

			/* DELIVER ACK, result update */
			ret = smpp_updateDB(&sms_his, SMPP_DELIVER_ACK_MSG);
			if ( ret < 0 ) {
				dAppLog(LOG_CRI, "[check_smpp_ack] SMPP_DELIVER_ACK_MSG Update fail");
			} else {
				dAppLog(LOG_DEBUG, "[check_smpp_ack] SMPP_DELIVER_ACK_MSG Update success");
			}
		}
		break;

	case SMPP_REPORT_MSG:
		{
			SMPP_REPORT 	recvMsg;
			SMPP_REPORT_ACK sendMsg;

			memset(&recvMsg, 0x00, sizeof(SMPP_REPORT));
			memset(&sendMsg, 0x00, sizeof(SMPP_REPORT_ACK));
			dAppLog(LOG_DEBUG, "RECV SMPP_REPORT_MSG");
			memcpy(&recvMsg, &(client.condata[idx].rbuf.buf), SMPP_HEAD_LEN+msg_h.len);
			
			dAppLog(LOG_DEBUG, " HEADER : TYPE      = %d", recvMsg.header.type);
			dAppLog(LOG_DEBUG, " HEADER : SIZE      = %d", recvMsg.header.len);
			dAppLog(LOG_DEBUG, " BODY   : RESULT    = %d", recvMsg.result);
			dAppLog(LOG_DEBUG, " BODY   : ORG_ADDR  = %s", recvMsg.org_addr);
			dAppLog(LOG_DEBUG, " BODY   : DST_ADDR  = %s", recvMsg.dst_addr);
			dAppLog(LOG_DEBUG, " BODY   : SN        = %d", recvMsg.sn);
			dAppLog(LOG_DEBUG, " BODY   : DELIVER T = %s", recvMsg.deliver_t);
			dAppLog(LOG_DEBUG, " BODY   : DEST_CODE = %s", recvMsg.dest_code);

			dAppLog (LOG_CRI, "RECV REPORT = RST CODE:%d ORG:%s DST:%s SN:%d DLV_TM:%s DST CODE:%s"
					, recvMsg.result, recvMsg.org_addr, recvMsg.dst_addr, recvMsg.sn, recvMsg.deliver_t, recvMsg.dest_code);

			strcpy(sms_his.reportTm, recvMsg.deliver_t);
			sms_his.reportSts = recvMsg.result;
			sms_his.cid = recvMsg.sn;
			/* result code 별 처리해야 한다. */
			switch(sms_his.reportSts)
			{
			case REPORT_E_SEND:

				break;
			case REPORT_E_INVALIDDST:
			case REPORT_E_POWEROFF:
			case REPORT_E_HIDDEN:
			case REPORT_E_TERMFUL:
			case REPORT_E_ETC:
			case REPORT_E_PORTED_OUT:
			case REPORT_E_ROAMING_FAIL:

				break;
			default:
				dAppLog(LOG_CRI, "[check_smpp_ack] SMPP_REPORT_MSG Unkown result code(%d)"
						, recvMsg.result);
				break;
			}

			/* REPORT, result update */
			ret = smpp_updateDB(&sms_his, SMPP_REPORT_MSG);
			if ( ret < 0 ) {
				dAppLog(LOG_CRI, "[check_smpp_ack] SMPP_REPORT_MSG Update fail");
			}
			else{
				dAppLog(LOG_DEBUG, "[check_smpp_ack] SMPP_REPORT_MSG Update success");
			}

			make_smpp_REPORT_ACK(REPORT_E_OK, &sendMsg);
			ret =write_sock_data(idx, (char *)&sendMsg, sizeof(SMPP_REPORT_ACK));
			if (ret < 0) { return -1; }

			ClearCid(cid);
		}
		break;
	}

	//smpp_log(RCV_FROM_SMSC, htonl(smpp.cmd_id), idx, cid, 0, 0, NULL, NULL);

	return 1;
}

