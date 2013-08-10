
#include <smpp.h>

extern st_SESSInfo			*gpCurTrc; // CURRENT OPERATION pointer
// TIMEOUT.conf ±¸Á¶Ã¼ 
extern MPTimer				*gpCurMPTimer;


char *str_time(time_t t)
{
	static char mtime_str[81];

	strftime(mtime_str,80,"%Y-%m-%d %T",localtime(&t));
	return (char*)mtime_str;
}

void Trace_BLKMSG (SMS_INFO *rcvBlkMsg)
{
	int trcIdx;

	if (gpCurTrc->dTCount <= 0) return;
	if (rcvBlkMsg->subsID == NULL) { dAppLog(LOG_WARN, "[TRACE] BLOCK INFO : IMSI is NULL"); return; }
	if (rcvBlkMsg->smsMsg == NULL) { dAppLog(LOG_WARN, "[TRACE] BLOCK INFO : SMS-MSG is NULL"); return; }

	for( trcIdx = 0; trcIdx < gpCurTrc->dTCount; trcIdx++ )
	{
		switch (gpCurTrc->stTrc[trcIdx].dType)
		{
		case TRACE_METHOD_IMSI:
			if(!strcmp(gpCurTrc->stTrc[trcIdx].szImsi, rcvBlkMsg->subsID)) {
				Send_TrcMsg_BlkMsg_SMPP2COND(rcvBlkMsg, TRACE_METHOD_IMSI);
			}
			break;
		case TRACE_METHOD_IP:
			if(!strcmp(gpCurTrc->stTrc[trcIdx].szImsi, rcvBlkMsg->subsIP)) {
				Send_TrcMsg_BlkMsg_SMPP2COND(rcvBlkMsg, TRACE_METHOD_IP);
			}
			break;
		default:
			dAppLog(LOG_CRI, "[TRACE] trace type is wrong(%d:%d)", trcIdx, gpCurTrc->stTrc[trcIdx].dType);
			break;
		}
	} /* end of for */
}

void Trace_DELIVER_MSG (SMS_HIS *sms_his, SMPP_DELIVER *deliver)
{
	int trcIdx;

	if (gpCurTrc->dTCount <= 0) return;
	if (sms_his->info.subsID == NULL) { dAppLog(LOG_WARN, "[TRACE] DELIVER MSG : IMSI is NULL"); return; }
	if (sms_his->info.subsIP == NULL) { dAppLog(LOG_WARN, "[TRACE] DELIVER MSG : IP is NULL"); return; }

	for( trcIdx = 0; trcIdx < gpCurTrc->dTCount; trcIdx++ )
	{
		switch (gpCurTrc->stTrc[trcIdx].dType)
		{
		case TRACE_METHOD_IMSI:
			if(!strcmp(gpCurTrc->stTrc[trcIdx].szImsi, sms_his->info.subsID)) {
				Send_TrcMsg_DelivMsg_SMPP2COND (sms_his, deliver, TRACE_METHOD_IMSI);
			}
			break;
		case TRACE_METHOD_IP:
			if(!strcmp(gpCurTrc->stTrc[trcIdx].szImsi, sms_his->info.subsIP)) {
				Send_TrcMsg_DelivMsg_SMPP2COND (sms_his, deliver, TRACE_METHOD_IP);
			}
			break;
		default:
			dAppLog(LOG_CRI, "[TRACE] trace type is wrong(%d:%d)", trcIdx, gpCurTrc->stTrc[trcIdx].dType);
			break;
		}
	} /* end of for */
}


int Send_TrcMsg_BlkMsg_SMPP2COND (SMS_INFO *rcvBlkMsg, int trace_type)
{
    GeneralQMsgType txGenQMsg;
    IxpcQMsgType    *txIxpcMsg = NULL;
    char            szTrcMsg[4096];
    int             dMsgLen= 0, dTxLen;
    struct timeval  stTmval;
    char            sztimebuf[128];
	char			szTypeStr[512];

    memset(&stTmval, 0x00, sizeof(struct timeval));
    memset(&sztimebuf, 0x00, sizeof(sztimebuf));
    memset(&txGenQMsg, 0x00, sizeof(txGenQMsg));
    memset(szTrcMsg, 0x00, 4096);

    txGenQMsg.mtype = MTYPE_TRC_CONSOLE;
    txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
    memset((void*)&txIxpcMsg->head, 0x00, sizeof(txIxpcMsg->head));

    strcpy(txIxpcMsg->head.srcSysName, mySysName);
    strcpy(txIxpcMsg->head.srcAppName, myAppName);
    strcpy(txIxpcMsg->head.dstSysName, "DSCM");
    strcpy(txIxpcMsg->head.dstAppName, "COND");

    gettimeofday(&stTmval, NULL );
    sprintf( sztimebuf,"[%s.%06ld]", str_time(stTmval.tv_sec), stTmval.tv_usec);
	sprintf (szTypeStr, "SMS : block info Message\nblock time:%d\nP/H:%02d %02d\nsms msg:%s "
				, rcvBlkMsg->blkTm, rcvBlkMsg->sPBit, rcvBlkMsg->sHBit, rcvBlkMsg->smsMsg);

	if (trace_type == TRACE_METHOD_IMSI) {
    	dMsgLen += snprintf(szTrcMsg + dMsgLen, sizeof(szTrcMsg), "IMSI:%s S7000 CALL TRACE INFORMATION (SMPP)\n", rcvBlkMsg->subsID);
	} else if (trace_type == TRACE_METHOD_IP) {
    	dMsgLen += snprintf(szTrcMsg + dMsgLen, sizeof(szTrcMsg), "IP:%s: S7000 CALL TRACE INFORMATION (SMPP)\n", rcvBlkMsg->subsIP);
	}
	else {
		dAppLog(LOG_CRI, "trace type is wrong(%d)", trace_type);
	}
	dMsgLen += snprintf(szTrcMsg + dMsgLen, sizeof(szTrcMsg), "%s\t%s \n", mySysName, sztimebuf);
    dMsgLen += snprintf(szTrcMsg + dMsgLen, sizeof(szTrcMsg), "------------------------------------------------------------\n");
	dMsgLen += snprintf(szTrcMsg + dMsgLen, sizeof(szTrcMsg), "Trace Log] %s \n", szTypeStr);

    txIxpcMsg->head.bodyLen = dMsgLen;
    memcpy(txIxpcMsg->body, szTrcMsg, dMsgLen);

    dTxLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;

    if( msgsnd(ixpcQid, (void*)&txGenQMsg, dTxLen, IPC_NOWAIT) < 0 ) {
        dAppLog(LOG_CRI, "[FAIL] SEND TO TRACE ERROR %d(%s)\n", errno, strerror(errno));
        return -1;
    }
    else {
        dAppLog(LOG_CRI, "[SUCC] SEND TO BLK-INFO TRACE, SUCCESS(%d)", dTxLen);
    }
    return 0;
}

int Send_TrcMsg_DelivMsg_SMPP2COND (SMS_HIS *sms_his, SMPP_DELIVER *deliver, int trace_type)
{
#if 1
    GeneralQMsgType txGenQMsg;
    IxpcQMsgType    *txIxpcMsg = NULL;
    char            szTrcMsg[4096];
    int             dMsgLen= 0, dTxLen;
    struct timeval  stTmval;
    char            sztimebuf[128];
	char			szTypeStr[512];
	char			strTime[32];

    memset(&stTmval, 0x00, sizeof(struct timeval));
    memset(&sztimebuf, 0x00, sizeof(sztimebuf));
    memset(&txGenQMsg, 0x00, sizeof(txGenQMsg));
    memset(szTrcMsg, 0x00, 4096);
	memset(strTime, 0x00, sizeof(strTime));

    txGenQMsg.mtype = MTYPE_TRC_CONSOLE;
    txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
    memset((void*)&txIxpcMsg->head, 0x00, sizeof(txIxpcMsg->head));

    strcpy(txIxpcMsg->head.srcSysName, mySysName);
    strcpy(txIxpcMsg->head.srcAppName, myAppName);
    strcpy(txIxpcMsg->head.dstSysName, "DSCM");
    strcpy(txIxpcMsg->head.dstAppName, "COND");

    gettimeofday(&stTmval, NULL );
    sprintf( sztimebuf,"[%s.%06ld]", str_time(stTmval.tv_sec), stTmval.tv_usec);
	sprintf (szTypeStr, "\tSMS : Send DELIVER Message\n" 
						"\tSend Time : %s\n"
						"\tS/N : %02d\n"
						"\tTID : %02d\n"
						"\tOrg Addr : %s\n"
						"\tDst Addr : %s\n"
						"\tSMS MSG  : %s "
				, str_time((time_t)sms_his->delivTm)
				, deliver->sn, deliver->tid, deliver->org_addr, deliver->dst_addr, deliver->text);

	if (trace_type == TRACE_METHOD_IMSI) {
    	dMsgLen += snprintf(szTrcMsg + dMsgLen, sizeof(szTrcMsg), "IMSI:%s S7000 CALL TRACE INFORMATION (SMPP)\n", sms_his->info.subsID);
	} else if (trace_type == TRACE_METHOD_IP) {
    	dMsgLen += snprintf(szTrcMsg + dMsgLen, sizeof(szTrcMsg), "IP:%s: S7000 CALL TRACE INFORMATION (SMPP)\n", sms_his->info.subsIP);
	}
	else {
		dAppLog(LOG_CRI, "trace type is wrong(%d)", trace_type);
	}
	dMsgLen += snprintf(szTrcMsg + dMsgLen, sizeof(szTrcMsg), "%s\t%s \n", mySysName, sztimebuf);
    dMsgLen += snprintf(szTrcMsg + dMsgLen, sizeof(szTrcMsg), "------------------------------------------------------------\n");
	dMsgLen += snprintf(szTrcMsg + dMsgLen, sizeof(szTrcMsg), "%s \n", szTypeStr);
    dMsgLen += snprintf(szTrcMsg + dMsgLen, sizeof(szTrcMsg), "------------------------------------------------------------\n");

    txIxpcMsg->head.bodyLen = dMsgLen;
    memcpy(txIxpcMsg->body, szTrcMsg, dMsgLen);

    dTxLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;

    if( msgsnd(ixpcQid, (void*)&txGenQMsg, dTxLen, IPC_NOWAIT) < 0 ) {
        dAppLog(LOG_CRI, "[FAIL] SEND TO TRACE ERROR %d(%s)\n", errno, strerror(errno));
        return -1;
    }
    else {
        dAppLog(LOG_CRI, "[SUCC] SEND TO DELIVER TRACE, SUCCESS(%d)", dTxLen);
    }
#endif
    return 0;
}

