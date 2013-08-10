#include "rana.h"
#include "rana_mmc_hld.h"
#include "rana_session.h"

/* Declaration of Global Variable */
char    resBuf[4096], resHead[4096], resTmp[1024];

MMC_LOGOUT_INFO 	mmcLOGOUT[SM_HANDLE_NUM];

/* Declaration of Extern Variable */
extern char mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN];
extern int mmcdQid, 	dIxpcQid;
extern st_SESSInfo		*gpCurTrc;

extern int RouteRLEG(long long llIMSI, int tidx);
int CheckTrace (SUBS_INFO *pSubsInfo);

int mmc_sce_log_in (IxpcQMsgType *rxIxpcMsg)
{
	dAppLog(LOG_DEBUG, "[mmc_sce_log_in] SYSTEM = %s    APP = %s    mmc_sce_log_in recv.", mySysName, myAppName );
	return 1;
}

char *str_time(time_t t)
{
	static char mtime_str[81];

	strftime(mtime_str,80,"%Y-%m-%d %T",localtime(&t));
	return (char*)mtime_str;
}

int CheckTrace (SUBS_INFO *pSubsInfo)
{
	int trcIdx, dRet = -1;

	if (gpCurTrc->dTCount <= 0) return -1;
	if (pSubsInfo->szMIN[0] == '\0') { dAppLog(LOG_WARN, "[CheckTraace]  IMSI is NULL"); return -1; }
	if (pSubsInfo->szFramedIP == '\0') { dAppLog(LOG_WARN, "[CheckTrace] IP is NULL"); return -1; }

	for( trcIdx = 0; trcIdx < gpCurTrc->dTCount; trcIdx++ )
	{
		switch (gpCurTrc->stTrc[trcIdx].dType)
		{
		case TRACE_METHOD_IMSI:
			if(!strcmp(gpCurTrc->stTrc[trcIdx].szImsi, pSubsInfo->szMIN)) {
				dRet = TRACE_TYPE_TIMEOUT_LOGOUT;
			}
			break;
		case TRACE_METHOD_IP:
			if(!strcmp(gpCurTrc->stTrc[trcIdx].szImsi, pSubsInfo->szFramedIP)) {
				dRet = TRACE_TYPE_TIMEOUT_LOGOUT;
			}
			break;
		default:
			dAppLog(LOG_CRI, "[TRACE] trace type is wrong(%d:%d)", trcIdx, gpCurTrc->stTrc[trcIdx].dType);
			break;
		}
	} /* end of for */
	return dRet;
}

void Trace_LOGIN (st_RADInfo *pstRADInfo, int acc_type, ST_PKG_INFO *pstPKGInfo)
{
	int trcIdx;

	if (gpCurTrc->dTCount <= 0) return;
	if (pstRADInfo->szMIN == NULL) { dAppLog(LOG_WARN, "[TRACE] ACC IMSI is NULL"); return; }
	if (pstRADInfo->uiFramedIP==0) { dAppLog(LOG_WARN, "[TRACE] ACC IP is NULL"); return; }

	for( trcIdx = 0; trcIdx < gpCurTrc->dTCount; trcIdx++ )
	{
		switch (gpCurTrc->stTrc[trcIdx].dType)
		{
		case TRACE_METHOD_IMSI:
			if(!strcmp(gpCurTrc->stTrc[trcIdx].szImsi, pstRADInfo->szMIN)) {
				Send_CondTrcMsg_RLEG(pstRADInfo, acc_type, TRACE_METHOD_IMSI, pstPKGInfo->sPkgNo);
			}
			break;
		case TRACE_METHOD_IP:
			if(!strcmp(gpCurTrc->stTrc[trcIdx].szImsi, CVT_INT2STR_IP(pstRADInfo->uiFramedIP))) {
				Send_CondTrcMsg_RLEG(pstRADInfo, acc_type, TRACE_METHOD_IP, pstPKGInfo->sPkgNo);
			}
			break;
		default:
			dAppLog(LOG_CRI, "[TRACE] trace type is wrong(%d:%d)", trcIdx, gpCurTrc->stTrc[trcIdx].dType);
			break;
		}
	} /* end of for */
}

int Trace_LOGOUT (st_RADInfo *pstRADInfo, int acc_type)
{
	int trcIdx, dRet = -1;

	if (gpCurTrc->dTCount <= 0) return -1;
	if (pstRADInfo->szMIN == NULL) { dAppLog(LOG_WARN, "[TRACE] ACC IMSI is NULL"); return -1; }
	if (pstRADInfo->uiFramedIP==0) { dAppLog(LOG_WARN, "[TRACE] ACC IP is NULL"); return -1; }

	for( trcIdx = 0; trcIdx < gpCurTrc->dTCount; trcIdx++ )
	{
		switch (gpCurTrc->stTrc[trcIdx].dType)
		{
		case TRACE_METHOD_IMSI:
			if(!strcmp(gpCurTrc->stTrc[trcIdx].szImsi, pstRADInfo->szMIN)) {
				Send_CondTrcMsg_RLEG(pstRADInfo, acc_type, TRACE_METHOD_IMSI, -1);
				dRet = TRACE_METHOD_IMSI;
			}
			break;
		case TRACE_METHOD_IP:
			if(!strcmp(gpCurTrc->stTrc[trcIdx].szImsi, CVT_INT2STR_IP(pstRADInfo->uiFramedIP))) {
				Send_CondTrcMsg_RLEG(pstRADInfo, acc_type, TRACE_METHOD_IP, -1);
				dRet = TRACE_METHOD_IP;
			}
			break;
		default:
			dAppLog(LOG_CRI, "[TRACE] type is wrong(%d:%d)", trcIdx, gpCurTrc->stTrc[trcIdx].dType);
			break;
		}
	} /* end of for */
	return dRet;
}

int Send_CondTrcMsg_RLEG (pst_RADInfo pstRADInfo, int acc_type, int trace_type, int pkgNo)
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
	switch (acc_type)
	{
	case TRACE_TYPE_ACC_START:	
		sprintf (szTypeStr, "RADIUS Accouting Request(Start)\n\tRuleSet ID : P=%d, H=%d\n"
				, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT);
		break;
	case TRACE_TYPE_ACC_INTERIM:	
		sprintf (szTypeStr, "RADIUS Accouting Request(Interim)\n\tRuleSet ID : P=%d, H=%d\n"
				, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT);
		break;
	case TRACE_TYPE_ACC_END:
		sprintf (szTypeStr, "RADIUS Accouting Request(Stop)\n\tRuleSet ID : P=%d, H=%d\n"
				, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT);
		break;
	case TRACE_TYPE_DISCONN_REQ:
		sprintf (szTypeStr, "RADIUS Disconnect Request\n\tRuleSet ID : P=%d, H=%d\n"
				, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT);
		break;
	case TRACE_TYPE_LOGIN:
		sprintf (szTypeStr, "Login OK!\n\tRuleSet ID : P=%d, H=%d\n"
				, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT);
		break;
	case TRACE_TYPE_LOGIN_OK:
		sprintf (szTypeStr, "Login OK!\n\tRuleSet ID : P=%d, H=%d\n"
				, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT);
		break;
	case TRACE_TYPE_LOGIN_FAIL:
		sprintf (szTypeStr, "Login Fail!\n\tRuleSet ID : P=%d, H=%d\n"
				, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT);
		break;
	case TRACE_TYPE_NOT_LOGIN:
		sprintf (szTypeStr, "Don't Login Call!\n\tRuleSet ID : P=%d, H=%d\n"
				, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT);
		break;
	case TRACE_TYPE_LOGOUT:
		sprintf (szTypeStr, "Logout OK!\n\tRuleSet ID : P=%d, H=%d\n"
				, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT);
		break;
	case TRACE_TYPE_LOGOUT_OK:
		sprintf (szTypeStr, "Logout OK!\n\tRuleSet ID : P=%d, H=%d\n"
				, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT);
		break;
	case TRACE_TYPE_LOGOUT_FAIL:
		sprintf (szTypeStr, "Logout Fail!\n\tRuleSet ID : P=%d, H=%d\n"
				, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT);
		break;
	case TRACE_TYPE_NOT_LOGOUT:
		sprintf (szTypeStr, "Don't Logout!\n\tRuleSet ID : P=%d, H=%d\n"
				, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT);
		break;
	case TRACE_TYPE_TIMEOUT:
		sprintf (szTypeStr, "timeout!\n\tRuleSet ID : P=%d, H=%d\n"
				, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT);
		break;
	case TRACE_TYPE_TIMEOUT_LOGOUT:
		sprintf (szTypeStr, "timeout, Logout OK!\n\tRuleSet ID : P=%d, H=%d\n"
				, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT);
		break;
	case TRACE_TYPE_TIMEOUT_NOT_LOGOUT:
		sprintf (szTypeStr, "timeout, Don't Logout!\n\tRuleSet ID : P=%d, H=%d\n"
				, pstRADInfo->uiPBIT, pstRADInfo->uiHBIT);
		break;
	default:
		sprintf (szTypeStr, "UNKNOWN MSG TYPE:%d \n", acc_type);
		break;
	}
    dAppLog(LOG_CRI, "[SUCC] send to trace str:%s", szTypeStr);

    txGenQMsg.mtype = MTYPE_TRC_CONSOLE;
    txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
    memset((void*)&txIxpcMsg->head, 0x00, sizeof(txIxpcMsg->head));

    strcpy(txIxpcMsg->head.srcSysName, mySysName);
    strcpy(txIxpcMsg->head.srcAppName, myAppName);
    strcpy(txIxpcMsg->head.dstSysName, "DSCM");
    strcpy(txIxpcMsg->head.dstAppName, "COND");

    gettimeofday(&stTmval, NULL );
    sprintf( sztimebuf,"[%s.%06ld]", str_time(stTmval.tv_sec), stTmval.tv_usec);
	if (trace_type == TRACE_METHOD_IMSI) {
    	dMsgLen += snprintf(szTrcMsg + dMsgLen, sizeof(szTrcMsg), "IMSI:%s S7000 CALL TRACE INFORMATION (RANA)\n", pstRADInfo->szMIN);
	}
	else if (trace_type == TRACE_METHOD_IP) {
    	dMsgLen += snprintf(szTrcMsg + dMsgLen, sizeof(szTrcMsg), "IP:%s S7000 CALL TRACE INFORMATION (RANA)\n", CVT_INT2STR_IP(pstRADInfo->uiFramedIP));
	}
	else {
		dAppLog(LOG_CRI, "[FAIL] TRACE TYPE is WRONG(%d)", trace_type);
	}
	dMsgLen += snprintf(szTrcMsg + dMsgLen, sizeof(szTrcMsg), "%s\t%s\n", mySysName, sztimebuf);
    dMsgLen += snprintf(szTrcMsg + dMsgLen, sizeof(szTrcMsg), "------------------------------------------------------------\n");
	dMsgLen += snprintf(szTrcMsg + dMsgLen, sizeof(szTrcMsg), "\n\t%s\n", szTypeStr);
    dMsgLen += snprintf(szTrcMsg + dMsgLen, sizeof(szTrcMsg), "------------------------------------------------------------\n");

    txIxpcMsg->head.bodyLen = dMsgLen;
    memcpy(txIxpcMsg->body, szTrcMsg, dMsgLen);

    dTxLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;

    if( msgsnd(dIxpcQid, (void*)&txGenQMsg, dTxLen, IPC_NOWAIT) < 0 ) {
        dAppLog(LOG_CRI, "[FAIL] SEND TO TRACE ERROR %d(%s)\n", errno, strerror(errno));
        return -1;
    }
    else {
        dAppLog(LOG_CRI, "[SUCC] send to trace success");
    }
    return 0;
}

int Trace_LOGIN_Req (st_RADInfo *pstRADInfo, int acc_type)
{
	int trcIdx, dRet = -1;

	if (gpCurTrc->dTCount <= 0) return -1;
	if (pstRADInfo->szMIN == NULL) { dAppLog(LOG_WARN, "[TRACE] ACC IMSI is NULL"); return -1; }
	if (pstRADInfo->uiFramedIP==0) { dAppLog(LOG_WARN, "[TRACE] ACC IP is NULL"); return -1; }

	for( trcIdx = 0; trcIdx < gpCurTrc->dTCount; trcIdx++ )
	{
		switch (gpCurTrc->stTrc[trcIdx].dType)
		{
		case TRACE_METHOD_IMSI:
			if(!strcmp(gpCurTrc->stTrc[trcIdx].szImsi, pstRADInfo->szMIN)) {
				Send_CondTrcMsg_RLEG(pstRADInfo, acc_type, TRACE_METHOD_IMSI, -1);
				dRet = TRACE_METHOD_IMSI;
			}
			break;
		case TRACE_METHOD_IP:
			if(!strcmp(gpCurTrc->stTrc[trcIdx].szImsi, CVT_INT2STR_IP(pstRADInfo->uiFramedIP))) {
				Send_CondTrcMsg_RLEG(pstRADInfo, acc_type, TRACE_METHOD_IP, -1);
				dRet = TRACE_METHOD_IP;
			}
			break;
		default:
			dAppLog(LOG_CRI, "[TRACE] trace type is wrong(%d:%d)", trcIdx, gpCurTrc->stTrc[trcIdx].dType);
			break;
		}
	} /* end of for */
	return dRet;
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
    char            cmdName[32];
    MMLReqMsgType   *rxReqMsg;

    txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
    memset ((void*)&txIxpcMsg->head, 0, sizeof(txIxpcMsg->head));

    txResMsg = (MMLResMsgType*)txIxpcMsg->body;

	txGenQMsg.mtype = MTYPE_MMC_RESPONSE;
	strcpy (cmdName, rxReqMsg->head.cmdName);

    strcpy (txIxpcMsg->head.srcSysName, mySysName);
    strcpy (txIxpcMsg->head.srcAppName, myAppName);
    strcpy (txIxpcMsg->head.dstSysName, "DSCM");
    strcpy (txIxpcMsg->head.dstAppName, "MMCD");
    
    txIxpcMsg->head.segFlag = segFlag;
    txIxpcMsg->head.seqNo   = seqNo;
    
    txResMsg->head.mmcdJobNo  = rxReqMsg->head.mmcdJobNo;
    txResMsg->head.extendTime = extendTime;
    txResMsg->head.resCode    = resCode;
    txResMsg->head.contFlag   = contFlag;
    strcpy(txResMsg->head.cmdName, cmdName);
    
    
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

