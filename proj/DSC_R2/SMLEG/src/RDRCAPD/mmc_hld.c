
#include "mmc_hld.h"


#define FL  __FILE__, __LINE__
extern char    trcBuf[], trcTmp[];
extern  int  trcFlag, trcLogFlag, trcLogId;


COMM_MMCHdlrVector   mmcHdlrVector[CDELAY_MAX_MMC_HANDLER] =
{
    {"dis-mmc-sample1",    	mmc_prc_sample1}
};

int     y_numMmcHdlr  = 12;


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
        sprintf(trcBuf,"[mmcd_exeMMCMsg] received unknown mml_cmd(:%s:)\n", mmlReqMsg->head.cmdName);
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    }
//fprintf(stderr, "stmd_exeMMCMsg::cmdName = %s\n", mmcHdlr->cmdName);


    if (trcLogFlag) {
        sprintf(trcBuf, "cmdName = %s\n", mmcHdlr->cmdName);
        trclib_writeLog(FL, trcBuf);
    }
    // 처리 function을 호출한다.
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
            sprintf(trcBuf, "unexpected mtype[%ld]\n", rxGenQMsg->mtype);
            trclib_writeErr(FL, trcBuf);
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
//	txGenQMsg.mtype = MTYPE_ALARM_REPORT;


	strcpy (cmdName, rxReqMsg->head.cmdName);

    strcpy (txIxpcMsg->head.srcSysName, rxIxpcMsg->head.dstSysName);
    strcpy (txIxpcMsg->head.srcAppName, rxIxpcMsg->head.dstAppName);
    strcpy (txIxpcMsg->head.dstSysName, rxIxpcMsg->head.srcSysName);
    strcpy (txIxpcMsg->head.dstAppName, rxIxpcMsg->head.srcAppName);
//    strcpy (txIxpcMsg->head.dstAppName, "COND");
    
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

#if 0
   	fprintf (stderr, "comm_tx, sys:%s, app:%s, dest:%s, desapp:%s\n", 
			txIxpcMsg->head.srcSysName, txIxpcMsg->head.srcAppName,
			txIxpcMsg->head.dstSysName, txIxpcMsg->head.dstAppName);
#endif
	
	sprintf (trcBuf, "comm_tx, sys:%s, app:%s, dest:%s, desapp:%s\n", 
			txIxpcMsg->head.srcSysName, txIxpcMsg->head.srcAppName,
			txIxpcMsg->head.dstSysName, txIxpcMsg->head.dstAppName);

	trclib_writeLog(FL, trcBuf);

    if (msgsnd(SendQid, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0) {
        sprintf(trcBuf, "msgsnd error=%s, cmd=%s, len=%d\n", strerror(errno), txResMsg->head.cmdName, txLen);
        trclib_writeErr(FL,trcBuf);
        return -1;
    } 

	return 1;
}
