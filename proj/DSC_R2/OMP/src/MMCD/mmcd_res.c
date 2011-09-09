#include "mmcd_proto.h"

extern int		mmcdQid, ixpcQid, MML_NUM_CMD;
extern time_t	currentTime;
extern char		trcBuf[4096], trcTmp[1024];
extern char		mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN];
extern MMLCmdContext		*mmlCmdTbl;
extern MmcdJobTblContext	*mmcdJobTbl;
extern MmcdCliTblContext	*mmcdCliTbl;
extern int		trcFlag, trcLogFlag;


//------------------------------------------------------------------------------
// application으로부터 수신한 응답메시지를 처리한다.
// - job table과 client table을 검색하여 최종 결과메시지를 구성하여 client로 보낸다.
// - 마지막 응답메시지가 아니면 deadlineTime을 갱신하고, 마지막 응답메시지이면
//	job table을 해제한다.
//------------------------------------------------------------------------------
int mmcd_exeMmcResMsg (
		GeneralQMsgType *rxGenQMsg // application으로부터 수신한 응답메시지
		)
{
//  +------------------------+   ----------------------------+
//  | mtype                  |                               |
//  +------------------------+   --------------+             |
//  | +--------------------+ |                 |        GeneralQMsgType
//  | | ixpc header        | |                 |             |
//  | +--------------------+ |            IxpcQMsgType       |
//  | | +----------------+ | |   ----+         |             |
//  | | | mml res header | | |       |         |             |
//  | | +----------------+ | |  MMLResMsgType  |             |
//  | | | mml res body   | | |       |         |             |
//  | | |                | | |       |         |             |
//  | | |                | | |       |         |             |
//  | | +----------------+ | |   ----+         |             |
//  | +--------------------+ |   --------------+             |
//  +------------------------+   ----------------------------+ 
	int				jobNo;
	IxpcQMsgType	*rxIxpcMsg;
	MMLResMsgType	*rxResMsg;

	rxIxpcMsg = (IxpcQMsgType*)rxGenQMsg->body;
	rxResMsg = (MMLResMsgType*)rxIxpcMsg->body;

	jobNo = (int)rxResMsg->head.mmcdJobNo;

	// 이미 timeout 처리되어 해제된 후 나중에 메시지가 들어올 수도 있다.
	//
	if (!mmcdJobTbl[jobNo].tpInd ||
		strcasecmp (mmcdJobTbl[jobNo].cmdName, rxResMsg->head.cmdName)) {
		if (trcFlag || trcLogFlag) {
			sprintf(trcBuf,"[mmcd_exeMmcResMsg] not found job; jobNo=%d, cmdName=%s\n",
					jobNo, rxResMsg->head.cmdName);
			trclib_writeLog (FL,trcBuf);
		}
		return -1;
	}

	// client로 응답메시지를 보낸다.
	// - application에서 결과 메시지를 출력가능한 string을 보내오고 mmcd는 time_stamp와
	//	user_name, command_slogan을 앞에 붙이고, 끝에 COMPLETED or CONTINUE를 붙여서
	//	client로 보낼 최종 응답메시지를 구성한다.
	//
	if (mmcd_sendResult2Client (jobNo, rxGenQMsg, mmcdJobTbl[jobNo].clientType) < 0) {
		return -1;
	}

	// 마지막 응답메시지가 아니면 deadlineTime을 갱신하고, 마지막 응답메시지이면
	//	job table을 해제한다.
	//
	if (rxResMsg->head.contFlag) {
		mmcdJobTbl[jobNo].deadlineTime = currentTime + rxResMsg->head.extendTime;
	} else {
		memset ((void*)&mmcdJobTbl[jobNo], 0, sizeof(MmcdJobTblContext));
	}

	return 1;

} //----- End of mmcd_exeMmcResMsg -----//
