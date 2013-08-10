#include "leg.h"
#include "leg_mmc_hld.h"

extern char g_szProcName[32];

char *str_time(time_t t)
{
	static char mtime_str[81];

	strftime(mtime_str,80,"%Y-%m-%d %T",localtime(&t));
	return (char*)mtime_str;
}

void Trace_LOGIN (SUBS_INFO *pSub, int TrcType, int dErrCode)
{
	if (pSub->dTrcFlag < 0 ) return;
	if (pSub->szMIN[0] == '\0') { dAppLog(LOG_WARN, "[TRACE LOGIN] IMSI is NULL"); return; }
	if (pSub->szFramedIP[0] == '\0') { dAppLog(LOG_WARN, "[TRACE LOGIN] IP is NULL"); return; }

	switch (pSub->dTrcFlag)
	{
		case TRACE_METHOD_IMSI:
			Send_CondTrcMsg_RLEG(pSub, TrcType, dErrCode); 
			break;
		case TRACE_METHOD_IP:
			Send_CondTrcMsg_RLEG(pSub, TrcType, dErrCode);
			break;
		default:
			dAppLog(LOG_CRI, "[TRACE LOGIN] trace flag(%d) type(%d) is wrong(MIN:%s)", 
							pSub->dTrcFlag, TrcType, pSub->szMIN);
			break;
	}
}

void Trace_LOGOUT (SUBS_INFO *pSub, int TrcType, int dErrCode)
{
	if (pSub->dTrcFlag < 0) return;
	if (pSub->szMIN[0] == '\0') { dAppLog(LOG_WARN, "[TRACE LOGOUT] IMSI is NULL"); return; }
	if (pSub->szFramedIP[0] == '\0') { dAppLog(LOG_WARN, "[TRACE LOGOUT] IP is NULL"); return; }

	switch (pSub->dTrcFlag)
	{
		case TRACE_METHOD_IMSI:
			Send_CondTrcMsg_RLEG(pSub, TrcType, dErrCode);
			break;
		case TRACE_METHOD_IP:
			Send_CondTrcMsg_RLEG(pSub, TrcType, dErrCode);
			break;
		default:
			dAppLog(LOG_CRI, "[TRACE LOGOUT] trace flag(%d) type(%d) is wrong(MIN:%s)", 
							pSub->dTrcFlag, TrcType, pSub->szMIN);
			break;
	} 
}

int Send_CondTrcMsg_RLEG (SUBS_INFO *pSub, int TrcType, int dErrCode)
{
    GeneralQMsgType txGenQMsg;
    IxpcQMsgType    *txIxpcMsg = NULL;
    char            szTrcMsg[4096];
    int             dMsgLen= 0, dTxLen;
    struct timeval  stTmval;
    char            sztimebuf[128];
	char			szTypeStr[128];

    memset(&stTmval, 0x00, sizeof(struct timeval));
    memset(&sztimebuf, 0x00, sizeof(sztimebuf));
    memset(&txGenQMsg, 0x00, sizeof(txGenQMsg));
    memset(szTrcMsg, 0x00, 4096);

	switch (TrcType)
	{
	case TRACE_TYPE_LOGIN:
		sprintf (szTypeStr, "Login Request!\n\tRuleSet ID : P=%d, H=%d\n"
				, pSub->uiPBit, pSub->uiHBit);
		break;
	case TRACE_TYPE_LOGIN_FAIL:
		sprintf (szTypeStr, "Login FAIL!\n\tRuleSet ID : P=%d, H=%d ErrCode=%d\n"
				, pSub->uiPBit, pSub->uiHBit, dErrCode);
		break;
	case TRACE_TYPE_LOGIN_SUCCESS:
		sprintf (szTypeStr, "Login Success!\n\tRuleSet ID : P=%d, H=%d\n"
				, pSub->uiPBit, pSub->uiHBit);
		break;
	case TRACE_TYPE_LOGOUT:
		sprintf (szTypeStr, "Logout Request!\n\tRuleSet ID : P=%d, H=%d\n"
				, pSub->uiPBit, pSub->uiHBit);
		break;
	case TRACE_TYPE_LOGOUT_FAIL:
		sprintf (szTypeStr, "Logout FAIL!\n\tRuleSet ID : P=%d, H=%d ErrCode=%d\n"
				, pSub->uiPBit, pSub->uiHBit, dErrCode);
		break;
	case TRACE_TYPE_LOGOUT_SUCCESS:
		sprintf (szTypeStr, "Logout Success!\n\tRuleSet ID : P=%d, H=%d\n"
				, pSub->uiPBit, pSub->uiHBit);
		break;
#if 0
	case TRACE_TYPE_LOGOUT_TIMEOUT:
		sprintf (szTypeStr, "Session timeout, Logout OK!\n");
		break;
#endif
	default:
		sprintf (szTypeStr, "Unknown TrcType:%d \n", TrcType);
		break;
	}
    dAppLog(LOG_CRI, "[SUCC] SEND TO TRACE STR:%s", szTypeStr);

    txGenQMsg.mtype = MTYPE_TRC_CONSOLE;
    txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
    memset((void*)&txIxpcMsg->head, 0x00, sizeof(txIxpcMsg->head));

    strcpy(txIxpcMsg->head.srcSysName, mySysName);
    strcpy(txIxpcMsg->head.srcAppName, myAppName);
    strcpy(txIxpcMsg->head.dstSysName, "DSCM");
    strcpy(txIxpcMsg->head.dstAppName, "COND");

    gettimeofday(&stTmval, NULL );
    sprintf( sztimebuf,"[%s.%06ld]", str_time(stTmval.tv_sec), stTmval.tv_usec);
	if (pSub->dTrcFlag == TRACE_METHOD_IMSI) {
    	dMsgLen += snprintf(szTrcMsg + dMsgLen, sizeof(szTrcMsg)
							, "IMSI:%s S7000 CALL TRACE INFORMATION (%s)\n", pSub->szMIN, g_szProcName);
	}
	else if (pSub->dTrcFlag == TRACE_METHOD_IP) {
    	dMsgLen += snprintf(szTrcMsg + dMsgLen, sizeof(szTrcMsg)
							, "IP:%s S7000 CALL TRACE INFORMATION (%s)\n", pSub->szFramedIP, g_szProcName);
	}
	else {
		dAppLog(LOG_CRI, "trace type is wrong(%d)", pSub->dTrcFlag);
	}
	dMsgLen += snprintf(szTrcMsg + dMsgLen, sizeof(szTrcMsg), "%s\t%s\n", mySysName, sztimebuf);
    dMsgLen += snprintf(szTrcMsg + dMsgLen, sizeof(szTrcMsg)
						, "------------------------------------------------------------\n");
	dMsgLen += snprintf(szTrcMsg + dMsgLen, sizeof(szTrcMsg), "\n\t%s\n", szTypeStr);
    dMsgLen += snprintf(szTrcMsg + dMsgLen, sizeof(szTrcMsg)
						, "------------------------------------------------------------\n");

    txIxpcMsg->head.bodyLen = dMsgLen;
    memcpy(txIxpcMsg->body, szTrcMsg, dMsgLen);

    dTxLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;

    if( msgsnd(dIxpcQid, (void*)&txGenQMsg, dTxLen, IPC_NOWAIT) < 0 ) {
        dAppLog(LOG_CRI, "[FAIL] SEND TO TRACE ERROR %d(%s)\n", errno, strerror(errno));
        return -1;
    }
    else {
        dAppLog(LOG_CRI, "[SUCC] SEND TO TRACE SUCCESS");
    }
    return 0;
}


int comm_txMMLResult (
			int SendQid,			// 보내고자 하는 Message Q id
            char *resBuf,
            char resCode,
            char contFlag,
            unsigned short extendTime,
            char segFlag,
            char seqNo
            )
{
    GeneralQMsgType txGenQMsg;
    IxpcQMsgType    *txIxpcMsg;
    MMLResMsgType   *txResMsg;
    int             txLen;

    txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
    memset ((void*)&txIxpcMsg->head, 0, sizeof(txIxpcMsg->head));

    txResMsg = (MMLResMsgType*)txIxpcMsg->body;

	txGenQMsg.mtype = MTYPE_MMC_RESPONSE;

    strcpy (txIxpcMsg->head.srcSysName, mySysName);
    strcpy (txIxpcMsg->head.srcAppName, g_szProcName);
    strcpy (txIxpcMsg->head.dstSysName, "DSCM");
    strcpy (txIxpcMsg->head.dstAppName, "MMCD");
    
    txIxpcMsg->head.segFlag = segFlag;
    txIxpcMsg->head.seqNo   = seqNo;
    
    txResMsg->head.mmcdJobNo  = 0; // TODO JobNo 필요하면 MMCR 에서 전달해주어야 함. 
    txResMsg->head.extendTime = extendTime;
    txResMsg->head.resCode    = resCode;
    txResMsg->head.contFlag   = contFlag;
    strcpy(txResMsg->head.cmdName, "sess-log-out");
    
    strcpy(txResMsg->body, resBuf);

    txIxpcMsg->head.bodyLen = sizeof(txResMsg->head) + strlen(txResMsg->body);
    txLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;

	dAppLog(LOG_DEBUG, "[comm_txMMLResult] SRC(SYS:%s, APP:%s) DST(SYS:%s, APP:%s)", 
			txIxpcMsg->head.srcSysName, txIxpcMsg->head.srcAppName,
			txIxpcMsg->head.dstSysName, txIxpcMsg->head.dstAppName);

    if (msgsnd(SendQid, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0) {
		dAppLog(LOG_CRI, "[comm_txMMLResult] msgsnd error=%s, cmd=%s, len=%d"
				, strerror(errno), txResMsg->head.cmdName, txLen);
        return -1;
    } 

	return 1;
}


int mmc_sce_log_out (SUBS_INFO *psi, int succfail, int dErrCode)
{
	char    		mmlBuf[BUFSIZ];

	/* Parameter setting && make login feild */
	dAppLog(LOG_CRI, "MMC][SCE-LOG-OUT][IMSI:%s] RESULT = %d", succfail);
	/* mml response */

	if( succfail == MMC_LOGOUT_SUCC )
	{
        sprintf(mmlBuf, "\tSYSTEM = %s\n\tAPP = %s\n\tSUCCESSED LOGOUT IMSI:%s.\n"
				, mySysName, g_szProcName, psi->szMIN );
	}
	else
	{
        sprintf(mmlBuf, "\tSYSTEM = %s\n\tAPP = %s\n\tFAILED LOGOUT IMSI:%s ERROR NUM=%d\n"
				, mySysName, g_szProcName, psi->szMIN, dErrCode);
	}

	comm_txMMLResult(dIxpcQid, mmlBuf, 0, 0, 0, 0, 1);

	return 1;
}

