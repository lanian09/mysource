
#include "stmd_proto.h"

extern  char    trcBuf[4096], trcTmp[1024];
extern  int     trcFlag, trcLogFlag, trcLogId;
extern  int     ixpcQid, condQid, nmsifQid, fimdQid;
extern  OnDemandList    onDEMAND[MAX_ONDEMAND_NUM];
extern  char    mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN];


int stmd_txMMLResult (
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

    // 모든 명령어 수행 결과 메시지를 MMCD를 거쳐 NMSIF를 통해 NMS로 전송해야 하는데,
    // on-demand 통계 결과와 dis-stat-his 결과 메시지는 주기적 통계처럼 단가와
    // 중장기로 나누어 구별해야하고, 나머지 일반 명령어는 MMC_RESPONSE로 구별할 수
    // 있어야 한다.
    // - 이를 위해 STMD에서 on-demand와 dis-stat-his로 5분짜리를 조회한 경우는
    //   단기통계로, dis-stat-his로 5분짜리 이외의 것을 조회한 경우는 중장기통계로,
    //   나머지 명령어 처리 결과는 MMC_RESPONSE로 구분해서 MMCD로 보낼때 mtype에 넣는다.
    //
    strcpy (cmdName, rxReqMsg->head.cmdName);
    if (!strncasecmp (cmdName, "STAT-", 5)) {
        if ( !strncasecmp (cmdName, "STAT-DIS", 8 ) ) 
            txGenQMsg.mtype = MTYPE_MMC_RESPONSE;
        else 
            txGenQMsg.mtype = MTYPE_STAT_REPORT_SHORT_TERM;
    } else if (!strcasecmp (cmdName,"dis-stat-his")) {
        if (!strcasecmp (rxReqMsg->head.para[1].paraName, "5MINUTELY"))
            txGenQMsg.mtype = MTYPE_STAT_REPORT_SHORT_TERM;
        else
            txGenQMsg.mtype = MTYPE_STAT_REPORT_LONG_TERM;
    } else {
        txGenQMsg.mtype = MTYPE_MMC_RESPONSE;
    }
    
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

    if (msgsnd(ixpcQid, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0) {
        sprintf(trcBuf, "msgsnd error=%s, cmd=%s, len=%d\n", strerror(errno), txResMsg->head.cmdName, txLen);
        trclib_writeErr(FL,trcBuf);
        return -1;
    }

    commlib_microSleep(100000);
    
    return 1;
}

int stmd_ondemand_txMMLResult (int list, char *resBuf, char resCode, char contFlag, unsigned short extendTime, char *cmdName, char segFlag, char seqNo)
{
    GeneralQMsgType txGenQMsg;
    IxpcQMsgType    *txIxpcMsg;
    MMLResMsgType   *txResMsg;
    int             txLen;

    txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
    txResMsg = (MMLResMsgType*)txIxpcMsg->body;

    // on-demand 통계 결과 메시지는 MMCD로 보내지만 MTYPE_STAT_REPORT_SHORT_TERM로 보낸다.
    // --> NMSIF가 MMCD로 통해 받아야 한다.
    //
    txGenQMsg.mtype = MTYPE_STAT_REPORT_SHORT_TERM;

    strcpy (txIxpcMsg->head.srcSysName, mySysName);
    strcpy (txIxpcMsg->head.srcAppName, myAppName);
    strcpy (txIxpcMsg->head.dstSysName, mySysName);
    strcpy (txIxpcMsg->head.dstAppName, "MMCD");
    txIxpcMsg->head.segFlag = segFlag;
    txIxpcMsg->head.seqNo = seqNo;

    txResMsg->head.mmcdJobNo = onDEMAND[list].cmdId;
    txResMsg->head.extendTime = extendTime;
    txResMsg->head.resCode = resCode;
    txResMsg->head.contFlag = contFlag;
    strcpy(txResMsg->head.cmdName, cmdName);

    strcpy(txResMsg->body, resBuf);

    txIxpcMsg->head.bodyLen = sizeof(txResMsg->head) + strlen(txResMsg->body);
    txLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;

    sprintf(trcBuf, "stmd_ondemand_txMMLResult] cmdName[%s], txLen=%d\n", cmdName, txLen);
    trclib_writeLog(FL, trcBuf);

    if (msgsnd(ixpcQid, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0) {
        sprintf(trcBuf, "msgsnd error = %s, cmd = %s\n", strerror(errno), txResMsg->head.cmdName);
        trclib_writeErr(FL,trcBuf);
#if 0
    } else {
        fprintf(stderr, "%s,%d\n", resBuf,txLen);
        fprintf(stderr,"extendTime = %d, contFlag = %d\n", extendTime, contFlag);
#endif
    }
    commlib_microSleep(100000);
	return txLen;
}

/*
* Logon 통계 대한 usage 값을 FIMD에 보내기 위한 함수
* added by uamyd 20110208
*/
int stmd_txMsg2FIMD(char *buff)
{
    int             txLen;
    GeneralQMsgType txGenQMsg;
    IxpcQMsgType    *txIxpcMsg;

    txGenQMsg.mtype = MTYPE_STATUS_REPORT;
    txIxpcMsg       = (IxpcQMsgType*)txGenQMsg.body;
    memset ((void*)&txIxpcMsg->head, 0, sizeof(txIxpcMsg->head));

    strcpy (txIxpcMsg->head.srcSysName, mySysName);
    strcpy (txIxpcMsg->head.srcAppName, myAppName);
    strcpy (txIxpcMsg->head.dstSysName, mySysName);
    strcpy (txIxpcMsg->head.dstAppName, "FIMD");

    txIxpcMsg->head.msgId   = MSGID_LOGON_STATISTICS_REPORT;
    txIxpcMsg->head.segFlag = 0;
    txIxpcMsg->head.seqNo   = 1;
    txIxpcMsg->head.bodyLen = sizeof(STAT_LOGON_RATE)*2; //LOGON, LOGOUT 2개를 동시에 보낸다.

    memcpy (txIxpcMsg->body, buff, sizeof(STAT_LOGON_RATE)*2);
    txLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;

    if (msgsnd(fimdQid, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0) {
        sprintf(trcBuf, "msgsnd fail to FIMD; err=%d(%s)\n", errno, strerror(errno));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    } 
    else 
    {
        if (trcFlag || trcLogFlag) 
        {
            sprintf(trcBuf,"send to FIMD\n%s", buff);
            trclib_writeLog (FL,trcBuf);
        }
    }
    commlib_microSleep(100000);
    return 1;
}

int stmd_txMsg2Cond(char *buff, int msgId, char segFlag, char seqNo)
{
    int             txLen;
    GeneralQMsgType txGenQMsg;
    IxpcQMsgType    *txIxpcMsg;
    // 주기적 통계(hourly, daily, weekly, monthly)는 중장기 통계로 설정해 보낸다.
    //
    txGenQMsg.mtype = MTYPE_STAT_REPORT_LONG_TERM;

    txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
    memset ((void*)&txIxpcMsg->head, 0, sizeof(txIxpcMsg->head));

    strcpy (txIxpcMsg->head.srcSysName, mySysName);
    strcpy (txIxpcMsg->head.srcAppName, myAppName);
    strcpy (txIxpcMsg->head.dstSysName, mySysName);
    strcpy (txIxpcMsg->head.dstAppName, "COND");

    txIxpcMsg->head.msgId = msgId;
    txIxpcMsg->head.segFlag = segFlag;
    txIxpcMsg->head.seqNo = seqNo;

    strcpy (txIxpcMsg->body, buff);
    txIxpcMsg->head.bodyLen = strlen(buff);
//	fprintf(stderr, "BODYLEN : %d\n", strlen(buff));
    txLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;

    if (msgsnd(condQid, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0) {
        sprintf(trcBuf, "msgsnd fail to COND; err=%d(%s)\n", errno, strerror(errno));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    } 
    else 
    {
        if (trcFlag || trcLogFlag) 
        {
            sprintf(trcBuf,"send to COND\n%s", buff);
            trclib_writeLog (FL,trcBuf);
        }
    }
    commlib_microSleep(100000);
    return 1;
}

int stmd_cron_txMsg2Cond(char *buff, int msgId, char segFlag, char seqNo)
{
    int             txLen;
    GeneralQMsgType txGenQMsg;
    IxpcQMsgType    *txIxpcMsg;
    // cron job 결과 메시지는 COND로 보내고, 단기통계로 설정해 보낸다.
    //
    txGenQMsg.mtype = MTYPE_STAT_REPORT_SHORT_TERM;

    txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
    memset ((void*)&txIxpcMsg->head, 0, sizeof(txIxpcMsg->head));

    strcpy (txIxpcMsg->head.srcSysName, mySysName);
    strcpy (txIxpcMsg->head.srcAppName, myAppName);
    strcpy (txIxpcMsg->head.dstSysName, mySysName);
    strcpy (txIxpcMsg->head.dstAppName, "COND");

    txIxpcMsg->head.msgId = msgId;
    txIxpcMsg->head.segFlag = segFlag;
    txIxpcMsg->head.seqNo = seqNo;

//    strcpy (txIxpcMsg->body, buff);
    txIxpcMsg->head.bodyLen = strlen(buff);
	if( txIxpcMsg->head.bodyLen > MAX_IXPC_QMSG_LEN )
		txIxpcMsg->head.bodyLen = MAX_IXPC_QMSG_LEN;

    strncpy (txIxpcMsg->body, buff, txIxpcMsg->head.bodyLen);
    txLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;

//printf("condQid = %d->%d\n", condQid, msgId);
    if (msgsnd(condQid, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0) 
    {
        sprintf(trcBuf, "msgsnd fail to COND(%d); err=%d(%s), msgId %d, len=%d\n", condQid,errno, strerror(errno), msgId, txLen);
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    } 
    else 
    {
        if (trcFlag || trcLogFlag) 
        {
            sprintf(trcBuf,"send to COND\n%s", buff);
            trclib_writeLog (FL,trcBuf);
        }
    }
    commlib_microSleep(100000);
//	fprintf(stderr, "return cond ():txLen:%d \n",txLen );
    return 1;
}

int stmd_txMsg2Nmsib(char *buff, int msgId, char segFlag, char seqNo)
{
#ifdef NMSIF
    int             txLen;
    GeneralQMsgType txGenQMsg;
    IxpcQMsgType    *txIxpcMsg;

    // 주기적 통계(hourly, daily, weekly, monthly)는 중장기 통계로 설정해 보낸다.
    //
    txGenQMsg.mtype = MTYPE_STAT_REPORT_LONG_TERM;

    txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
    memset ((void*)&txIxpcMsg->head, 0, sizeof(txIxpcMsg->head));

    strcpy (txIxpcMsg->head.srcSysName, mySysName);
    strcpy (txIxpcMsg->head.srcAppName, myAppName);
    strcpy (txIxpcMsg->head.dstSysName, mySysName);
    strcpy (txIxpcMsg->head.dstAppName, "NMSIF");

    txIxpcMsg->head.msgId = msgId;
    txIxpcMsg->head.segFlag = segFlag;
    txIxpcMsg->head.seqNo = seqNo;

    strcpy (txIxpcMsg->body, buff);
    txIxpcMsg->head.bodyLen = strlen(buff);
    txLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;

    if (msgsnd(nmsifQid, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0) {
        sprintf(trcBuf, "msgsnd fail to NMSIF; err=%d(%s)\n", errno, strerror(errno));
        trclib_writeLogErr (FL,trcBuf);
        return -1;
    } else {
        if (trcFlag || trcLogFlag) {
            sprintf(trcBuf,"send to NMSIF\n%s", buff);
            trclib_writeLog (FL,trcBuf);
        }
    }
    commlib_microSleep(100000);

#endif

    return 1;
}

/*
    #define ONDEMANDJOB     0
    #define CRONJOB         1
    #define PERIODIC        2
*/
int sendCondResultMsg(int type, char* msgBuf, int code)
{
/*  char sndBuf[4096],tmpBuf[1024];
    int i, linecnt=0, oldpos=0;
    int seq=0;
    
    for(i=0; i<strlen(msgBuf); i++)
    {
        if(msgBuf[i] == '\n')
        {
            //printf("line find=%d\n",linecnt++);
            linecnt++;
            if(linecnt == 60)
            {
                memset(sndBuf, 0, sizeof(sndBuf));
                strncpy(sndBuf, &msgBuf[oldpos], (i-oldpos)+1);
                //printf("%s\n",sndBuf); 
                if(type == CRONJOB)
                {
                    stmd_cron_txMsg2Cond (sndBuf, code, 1, seq++);
                }
                else //PERIODIC
                {
                    stmd_txMsg2Cond(sndBuf, code, 1, seq++);
                }
                linecnt = 0;
                oldpos = i+1;
            }
        }
        
        if( (i+1) == strlen(msgBuf))
        {
            memset(sndBuf, 0, sizeof(sndBuf));
            strncpy(sndBuf, &msgBuf[oldpos], (i-oldpos)+1);
            if(type == CRONJOB)
            {
                stmd_cron_txMsg2Cond (sndBuf, code, 0, seq++);
            }
            else //PERIODIC
            {
                stmd_txMsg2Cond (sndBuf, code, 0, seq++);
            }
            //printf("%s\n",sndBuf);
            //printf("*** Last Message\n");
        }
    }
*/
    return 1;
}

