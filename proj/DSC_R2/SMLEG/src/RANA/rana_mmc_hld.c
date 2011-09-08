#include "rana.h"
#include "rana_mmc_hld.h"


#define FL  __FILE__, __LINE__
extern char    trcBuf[], trcTmp[];
extern  int  trcFlag, trcLogFlag, trcLogId;


COMM_MMCHdlrVector   mmcHdlrVector[CDELAY_MAX_MMC_HANDLER] =
{
//  {"sync-rule-file",    	mmc_sync_rule_file},
//  {"sess-log-out",    	mmc_sce_log_out},
//  {"dis-rule-use",    	mmc_dis_rule_use},
//  {"set-rule-use",    	mmc_set_rule_use},
//  {"del-rule-use",    	mmc_del_rule_use},
//  {"set-ovld-ctrl",    	mmc_set_ovld_ctrl},
//  {"dis-ovld-ctrl",    	mmc_dis_ovld_ctrl},
//	{"dis-rleg-sess", 		mmc_dis_rleg_sess},
//	{"set-smbuf-clr", 		mmc_set_smbuf_clr},		// BY JUNE, 2011-03-16
//	{"del-smbuf-clr", 		mmc_del_smbuf_clr},		// BY JUNE, 2011-03-16
//	{"dis-smbuf-clr", 		mmc_dis_smbuf_clr}		// BY JUNE, 2011-03-16
};

int     y_numMmcHdlr  = 10;


int stmd_mmcHdlrVector_bsrchCmp (const void *a, const void *b)
{   
    return (strcasecmp ((char*)a, ((COMM_MMCHdlrVector*)b)->cmdName));
}



int comm_mmcHdlrVector_qsortCmp (const void *a, const void *b)                                                                 
{                                                                                                                              
    return (strcasecmp (((COMM_MMCHdlrVector *)a)->cmdName, ((COMM_MMCHdlrVector *)b)->cmdName));                              
} 


void mmc_yh_init () {                                                                                                          
	qsort ( (void*)mmcHdlrVector,                                                                                              
			y_numMmcHdlr,                                                                                                      
			sizeof(COMM_MMCHdlrVector),                                                                                        
			comm_mmcHdlrVector_qsortCmp );                                                                                     
} 


//------------------------------------------------------------------------------
// MMCD에서 명령어를 수신한 경우
//------------------------------------------------------------------------------
int mmcd_exeMMCMsg (IxpcQMsgType *rxIxpcMsg)
{

    MMLReqMsgType       *mmlReqMsg;
    COMM_MMCHdlrVector   *mmcHdlr;

    mmlReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
    if ((mmcHdlr = (COMM_MMCHdlrVector*) bsearch (
            mmlReqMsg->head.cmdName,
            mmcHdlrVector,
            y_numMmcHdlr,
            sizeof(COMM_MMCHdlrVector),
            stmd_mmcHdlrVector_bsrchCmp)) == NULL)
    {
		dAppLog(LOG_CRI, "[exeMMCMsg] RECV UNKNOWN MML_CMD(:%s:)\n", mmlReqMsg->head.cmdName);
        return -1;
    }
	dAppLog(LOG_DEBUG, "[exeMMCMsg] RECV MML_CMD(:%s:)\n", mmlReqMsg->head.cmdName);

    // Call MML Handler
    (int)(*(mmcHdlr->func)) (rxIxpcMsg);
    
    return 1;
}


int mmcd_exeRxQMsg (GeneralQMsgType *rxGenQMsg)
{
    int     ret = 1;

    switch (rxGenQMsg->mtype) {
        case MTYPE_SETPRINT:
            ret = trclib_exeSetPrintMsg ((TrcLibSetPrintMsgType*)rxGenQMsg);
            break;
        case MTYPE_MMC_REQUEST:
            ret = mmcd_exeMMCMsg ((IxpcQMsgType*)rxGenQMsg->body);
            break;
        default:
			dAppLog(LOG_CRI, "[exeRxQMsg] unexpected mtype(:%ld:)\n", rxGenQMsg->mtype);
            return -1;
    }

    return 1;
}


int comm_txMMLResult (
			int SendQid,			// 보내고자 하는 Message Q id
            IxpcQMsgType *rxIxpcMsg,
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

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
    txResMsg = (MMLResMsgType*)txIxpcMsg->body;

	txGenQMsg.mtype = MTYPE_MMC_RESPONSE;
	strcpy (cmdName, rxReqMsg->head.cmdName);

    strcpy (txIxpcMsg->head.srcSysName, rxIxpcMsg->head.dstSysName);
    strcpy (txIxpcMsg->head.srcAppName, rxIxpcMsg->head.dstAppName);
    strcpy (txIxpcMsg->head.dstSysName, rxIxpcMsg->head.srcSysName);
    strcpy (txIxpcMsg->head.dstAppName, rxIxpcMsg->head.srcAppName);
    
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

