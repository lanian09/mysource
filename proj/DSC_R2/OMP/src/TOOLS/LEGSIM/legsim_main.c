#include "legsim.h"
#include "sfm_msgtypes.h"

#include <time.h>

int     stmdQid, fimdQid;
char    mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN];

int send2FIMD(char *buff)
{
    int             txLen;
    GeneralQMsgType txGenQMsg;
    IxpcQMsgType    *txIxpcMsg;

    txGenQMsg.mtype = MTYPE_STATUS_REPORT;
    txIxpcMsg       = (IxpcQMsgType*)txGenQMsg.body;
    memset ((void*)&txIxpcMsg->head, 0, sizeof(txIxpcMsg->head));
        
    strcpy (txIxpcMsg->head.srcSysName, "SCMA");
    strcpy (txIxpcMsg->head.srcAppName, "RLEG");
    strcpy (txIxpcMsg->head.dstSysName, "DSCM");
    strcpy (txIxpcMsg->head.dstAppName, "FIMD");
        
    txIxpcMsg->head.msgId   = MSGID_CPS_REPORT;
    txIxpcMsg->head.bodyLen = sizeof(LEG_DATA);
        
    memcpy (txIxpcMsg->body, buff, sizeof(LEG_DATA));
    txLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;
    
    if (msgsnd(fimdQid, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0) {
		fprintf(stderr,"FAILED IN msgsnd(FIMD:%d:0x%x), err=%d:%s\n",
				fimdQid, fimdQid,errno, strerror(errno));
        return -1;
    }
	fprintf(stdout, "SUCCESS IN msgsnd(FIMD:%d:0x%x)\n",fimdQid,fimdQid);
    return 1;
}

int send2STMD(char *buff)
{
    int             txLen;
    GeneralQMsgType txGenQMsg;
    IxpcQMsgType    *txIxpcMsg;

    txGenQMsg.mtype = MTYPE_STATUS_REPORT;
    txIxpcMsg       = (IxpcQMsgType*)txGenQMsg.body;
    memset ((void*)&txIxpcMsg->head, 0, sizeof(txIxpcMsg->head));
        
    strcpy (txIxpcMsg->head.srcSysName, "SCMA");
    strcpy (txIxpcMsg->head.srcAppName, "RLEG");
    strcpy (txIxpcMsg->head.dstSysName, "SCMA");
    strcpy (txIxpcMsg->head.dstAppName, "RLEG");
        
    txIxpcMsg->head.msgId   = MSGID_LEG_STATISTICS_REPORT;
    txIxpcMsg->head.bodyLen = sizeof(LEG_TOT_STAT);
        
    memcpy (txIxpcMsg->body, buff, sizeof(LEG_TOT_STAT));
    txLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;
    
    if (msgsnd(stmdQid, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0) {
		fprintf(stderr,"FAILED IN msgsnd(STMD:%d:0x%x), err=%d:%s\n",
				stmdQid,stmdQid,errno, strerror(errno));
        return -1;
    }
	fprintf(stdout, "SUCCESS IN msgsnd(STMD:%d:0x%x)\n",stmdQid,stmdQid);
    return 1;
}  

int makeLeg(PLEG_STAT pLEG)
{
	int i;
	LEG_PDSN_STAT *pPDSN;
	char *szIPList[] = { "10.160.250.132", "10.160.250.140" };
	int  uiIPList[] = { 178322052, 178322060 };

	pLEG->uiCount = sizeof(szIPList)/sizeof(char*);

	if( pLEG->uiCount > DEF_PDSN_CNT ) pLEG->uiCount = DEF_PDSN_CNT;
	fprintf(stdout,"PDSN Count = %d\n",pLEG->uiCount);

	for( i = 0; i< pLEG->uiCount; i++ ){
		pPDSN = &pLEG->stPDSNStat[i];
		pPDSN->uiPDSN_IP         = uiIPList[i];//inet_addr(szIP);
		pPDSN->uiPDSN_RecvCnt    = 1;     
		pPDSN->uiPDSN_StartCnt   = 2;    
		pPDSN->uiPDSN_InterimCnt = 3;  
		pPDSN->uiPDSN_StopCnt    = 4;     
		pPDSN->uiPDSN_DiscReqCnt = 5;  
		pPDSN->uiLogOn_StartCnt  = 6;   
		pPDSN->uiLogOn_InterimCnt= 7; 
		pPDSN->uiLogOn_StopCnt   = 8;    
		pPDSN->uiLogOn_DiscReqCnt= 9; 
		fprintf(stdout,"IP=%u(%s), Recv=%d Start=%d Interim=%d Stop=%d DiscReq=%d\n"
					   "\t Logon, Start=%d Interim=%d Stop=%d DiscReq=%d\n",
		pPDSN->uiPDSN_IP, szIPList[i],
		pPDSN->uiPDSN_RecvCnt, pPDSN->uiPDSN_StartCnt, pPDSN->uiPDSN_InterimCnt, pPDSN->uiPDSN_StopCnt, pPDSN->uiPDSN_DiscReqCnt,
		pPDSN->uiLogOn_StartCnt, pPDSN->uiLogOn_InterimCnt, pPDSN->uiLogOn_StopCnt, pPDSN->uiLogOn_DiscReqCnt);
	}
	
	return 0;
}

int makeLogon(PLOGON_STAT pLOGON,int i, int j)
{
	pLOGON->uiSMIndex       = i;
	pLOGON->uiLogMode       = j;          
	pLOGON->uiLogOn_Request = 1;    
	pLOGON->uiLogOn_Success = 2;    
	pLOGON->uiLogOn_Fail    = 3;       
	pLOGON->uiLogOn_Reason1 = 4;    
	pLOGON->uiLogOn_Reason2 = 5;    
	pLOGON->uiLogOn_Reason3 = 6;    
	pLOGON->uiLogOn_Reason4 = 7;    
	pLOGON->uiLogOn_APIReqErr = 8;  
	pLOGON->uiLogOn_APITimeout= 9; 
	
	for( i=0;i<DEF_HBIT_CNT;i++){
		pLOGON->uiLogOn_HBIT[i] =i+1;
	}

	fprintf(stdout,"LOGON MOD=%d SMIndex=%d\n" 
			"Req=%d Success=%d Fail=%d R1=%d R2=%d R3=%d R4=%d APIReq=%d APITimeout=%d\n"
			"BIT[1]=%d ~ BIT=[%d]=%d\n",
			pLOGON->uiSMIndex, pLOGON->uiLogMode,
			pLOGON->uiLogOn_Request, pLOGON->uiLogOn_Success, pLOGON->uiLogOn_Fail,
			pLOGON->uiLogOn_Reason1, pLOGON->uiLogOn_Reason2, pLOGON->uiLogOn_Reason3, pLOGON->uiLogOn_Reason4,
			pLOGON->uiLogOn_APIReqErr, pLOGON->uiLogOn_APITimeout,
			pLOGON->uiLogOn_HBIT[0],i-1,pLOGON->uiLogOn_HBIT[i-1]);
	return 0;
}
void CVT_LOGON_H2N(PLOGON_STAT pLOGON)
{
	int i;
	pLOGON->uiSMIndex = htonl(pLOGON->uiSMIndex);
	pLOGON->uiLogMode = htonl(pLOGON->uiLogMode);
	pLOGON->uiLogOn_Request = htonl(pLOGON->uiLogOn_Request);
	pLOGON->uiLogOn_Success = htonl(pLOGON->uiLogOn_Success);
	pLOGON->uiLogOn_Fail    = htonl(pLOGON->uiLogOn_Fail);
	pLOGON->uiLogOn_Reason1 = htonl(pLOGON->uiLogOn_Reason1);
	pLOGON->uiLogOn_Reason2 = htonl(pLOGON->uiLogOn_Reason2);
	pLOGON->uiLogOn_Reason3 = htonl(pLOGON->uiLogOn_Reason3);
	pLOGON->uiLogOn_Reason4 = htonl(pLOGON->uiLogOn_Reason4);
	pLOGON->uiLogOn_APIReqErr = htonl(pLOGON->uiLogOn_APIReqErr);
	pLOGON->uiLogOn_APITimeout= htonl(pLOGON->uiLogOn_APITimeout);
	for( i = 0; i< DEF_HBIT_CNT; i++ ){
		pLOGON->uiLogOn_HBIT[i] = htonl(pLOGON->uiLogOn_HBIT[i]);
	}
	
}

void CVT_PDSN_H2N(LEG_PDSN_STAT *pPDSN)
{
	pPDSN->uiPDSN_IP          = htonl(pPDSN->uiPDSN_IP);
	pPDSN->uiPDSN_RecvCnt     = htonl(pPDSN->uiPDSN_RecvCnt);
	pPDSN->uiPDSN_StartCnt    = htonl(pPDSN->uiPDSN_StartCnt);
	pPDSN->uiPDSN_InterimCnt  = htonl(pPDSN->uiPDSN_InterimCnt);
	pPDSN->uiPDSN_StopCnt     = htonl(pPDSN->uiPDSN_StopCnt);
	pPDSN->uiPDSN_DiscReqCnt  = htonl(pPDSN->uiPDSN_DiscReqCnt);
	pPDSN->uiLogOn_StartCnt   = htonl(pPDSN->uiLogOn_StartCnt);
	pPDSN->uiLogOn_InterimCnt = htonl(pPDSN->uiLogOn_InterimCnt);
	pPDSN->uiLogOn_StopCnt    = htonl(pPDSN->uiLogOn_StopCnt);
	pPDSN->uiLogOn_DiscReqCnt = htonl(pPDSN->uiLogOn_DiscReqCnt);
}

void CVT_LEG_H2N(LEG_STAT *pLEG)
{
	int i;
	for( i = 0; i < pLEG->uiCount; i++ ){
		CVT_PDSN_H2N(&pLEG->stPDSNStat[i]);
	}
	pLEG->uiCount = htonl(pLEG->uiCount);
}

void CVT_H2N(LEG_TOT_STAT *pLEGTOT)
{
	int i,j;
	CVT_LEG_H2N(&pLEGTOT->stAcct);
	for( i = 0; i < MAX_RLEG_CNT; i++ ){
		for( j = 0; j < LOG_MOD_CNT; j++ ){
			CVT_LOGON_H2N(&pLEGTOT->stLogon[i][j]);
		}
	}
}

int makeLegTotal()
{
	int i,j;
	LEG_TOT_STAT   leg_tot;

	sprintf(leg_tot.szSysName,"SCMA");
	fprintf(stdout,"System Name = %s\n", leg_tot.szSysName);
	makeLeg(&leg_tot.stAcct);

	for( i = 0 ; i< MAX_RLEG_CNT; i++ ){
		for( j = 0; j< LOG_MOD_CNT; j++ ){
			makeLogon(&leg_tot.stLogon[i][j],i,j);
		}
	}
	CVT_H2N(&leg_tot);
	send2STMD((char*)&leg_tot);
	return 0;
}

int makeLegData()
{
	int    i;
	LEG_DATA ld;
	LEG_SUM_CPS   *pcps;
	LEG_SESS_DATA *psess;

	memset(&ld, 0x00, sizeof(ld));

	pcps = &ld.cps;
	psess= &ld.sess;

	for(i = 0; i < 100; i++ ){
		//cps
		pcps->uiLogOnSumCps = 100;
		pcps->uiLogOutSumCps = 80;

		//sess
		psess->amount = 500;

		//tps
		ld.tps = 230;

		//CONVERT	
		pcps->uiLogOnSumCps = htonl(pcps->uiLogOnSumCps);
		pcps->uiLogOutSumCps = htonl(pcps->uiLogOutSumCps);

		psess->amount = htonl(psess->amount);
		
		ld.tps = htonl(ld.tps);

		send2FIMD((char*)&ld);
		fprintf(stdout,"send value=CPS[%d:%d], SESS[%d], TPS[%d]. time=%ld\n",
				ntohl(pcps->uiLogOnSumCps), ntohl(pcps->uiLogOutSumCps),
				ntohl(psess->amount), ntohl(ld.tps), time(NULL));
		sleep(5);
		
	}
	
	return 0;
}

int main(void)
{

    if ( InitSys() < 0 )
        exit(1);

	//makeLegTotal();
	makeLegData();
	return 0;
}

