#define __MCDM_TYPE__
#include "mcdm_proto.h"

int			mcdmQid, ixpcQid, eqSysCnt=0;
char		mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN];
char		trcBuf[4096], trcTmp[1024];
char		resBuf[4096], resHead[1024];
time_t		currentTime;
int			lastAllocJobNo=0;
int			numDistrMmc;
McdmJobTblContext			mcdmJobTbl[MCDM_NUM_TP_JOB_TBL];
McdmDistribMmcTblContext	mcdmDistrMmcTbl[MCDM_MAX_DISTRIB_MMC];
SFM_sfdb	*sfdb;
IxpcConSts 	*ixpcCON;	

extern int	trcFlag, trcLogFlag;

McdmOwnMmcHdlrVector	ownMmcHdlrVector[MCDM_MAX_OWN_MMC_HANDLER] = 
{
};
int		numOwnMmcHdlr=0;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main (int ac, char *av[])
{
	int		ret, rxCnt, loopCnt=0;
	int		tickPerSec=100; // 1초당 loop를 몇번 도는지 횟수; cpu 성능에 따라 달라진다.
	GeneralQMsgType	rxGenQMsg;

//	if((check_Index = check_my_run_status("MCDM")) < 0)
//		exit(0);

	if (mcdm_initial() < 0) {
		fprintf(stderr,">>>>>> mcdm_initial fail\n");
		return -1;
	}

	// clear previous messages
	while (msgrcv(mcdmQid, &rxGenQMsg, sizeof(rxGenQMsg), 0, IPC_NOWAIT) > 0);

	//trcLogFlag = 1;

	memset(&rxGenQMsg,0,sizeof(GeneralQMsgType));
	while (1)
	{
		rxCnt = 0;
		while ((ret = msgrcv(mcdmQid, &rxGenQMsg, sizeof(rxGenQMsg), 0, IPC_NOWAIT)) > 0) {
			rxCnt++;
			mcdm_exeRxQMsg (&rxGenQMsg);
			memset(&rxGenQMsg,0,sizeof(GeneralQMsgType));
		}
		if ((ret < 0) && (errno == EINVAL || errno == EFAULT)) {
			sprintf(trcBuf,"[mcdm_main] >>> msgrcv fail; err=%d(%s)\n", errno, strerror(errno));
			trclib_writeLogErr (FL,trcBuf);
			return -1;
		}
		if (!rxCnt) commlib_microSleep(5000);

		loopCnt++;

		// 0.5초마다 jobTbl을 확인하여 timeout처리된 놈을 처리한다.
		//
		if ((loopCnt % (tickPerSec/2)) == 0) {
			currentTime = time(0);
			keepalivelib_increase ();
			mcdm_checkJobTbl ();
		}

	} //-- end of while(1) --//

	return 0;
	
} //----- End of main -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int mcdm_exeRxQMsg (GeneralQMsgType *rxGenQMsg)
{
	int	ret=1;

	switch (rxGenQMsg->mtype) {
		case MTYPE_SETPRINT:
			ret = trclib_exeSetPrintMsg ((TrcLibSetPrintMsgType*)rxGenQMsg);
			break;

		case MTYPE_MMC_REQUEST: //3
//			printf("MTYPE_MMC_REQUEST\n");
			ret = mcdm_rxMMCReqMsg (rxGenQMsg);
			break;

		case MTYPE_MMC_RESPONSE: //4
//			printf("MTYPE_MMC_RESPONSE\n");
			ret = mcdm_rxDistribMmcRes (rxGenQMsg);
			break;

		default:
			sprintf(trcBuf,"[mcdm_exeRxQMsg] received unknown mtype(%ld)\n", rxGenQMsg->mtype);
			trclib_writeLogErr (FL,trcBuf);
			return -1;
	}

	return 1;

} //----- End of mcdm_exeRxQMsg -----//



//------------------------------------------------------------------------------
// MMCD에서 명령어를 수신한 경우
//------------------------------------------------------------------------------
int mcdm_rxMMCReqMsg (GeneralQMsgType *rxGenQMsg)
{
	IxpcQMsgType		*rxIxpcMsg;
	MMLReqMsgType		*mmlReqMsg;
	McdmOwnMmcHdlrVector	*ownMmcHdlr;

	rxIxpcMsg = (IxpcQMsgType*)rxGenQMsg->body;
	mmlReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	if ((ownMmcHdlr = (McdmOwnMmcHdlrVector*) bsearch (
					mmlReqMsg->head.cmdName,
					ownMmcHdlrVector,
					numOwnMmcHdlr,
					sizeof(McdmOwnMmcHdlrVector),
					mcdm_ownMmcHdlrVector_bsrchCmp)) != NULL)
	{
		// 자신이 직접 처리하는 명령어인 경우 처리 function을 호출한다.
		//
		(int)(*(ownMmcHdlr->func)) (rxIxpcMsg);
	}
	else {
		// 다른 놈으로 분배해야 하는 명령어인 경우
		//
		if ( mcdm_rxDistribMmcReq (rxGenQMsg) < 0 ){
			;
		}
	}

	return 1;

} //----- End of mcdm_rxMMCReqMsg -----//



