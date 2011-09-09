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
// application���κ��� ������ ����޽����� ó���Ѵ�.
// - job table�� client table�� �˻��Ͽ� ���� ����޽����� �����Ͽ� client�� ������.
// - ������ ����޽����� �ƴϸ� deadlineTime�� �����ϰ�, ������ ����޽����̸�
//	job table�� �����Ѵ�.
//------------------------------------------------------------------------------
int mmcd_exeMmcResMsg (
		GeneralQMsgType *rxGenQMsg // application���κ��� ������ ����޽���
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

	// �̹� timeout ó���Ǿ� ������ �� ���߿� �޽����� ���� ���� �ִ�.
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

	// client�� ����޽����� ������.
	// - application���� ��� �޽����� ��°����� string�� �������� mmcd�� time_stamp��
	//	user_name, command_slogan�� �տ� ���̰�, ���� COMPLETED or CONTINUE�� �ٿ���
	//	client�� ���� ���� ����޽����� �����Ѵ�.
	//
	if (mmcd_sendResult2Client (jobNo, rxGenQMsg, mmcdJobTbl[jobNo].clientType) < 0) {
		return -1;
	}

	// ������ ����޽����� �ƴϸ� deadlineTime�� �����ϰ�, ������ ����޽����̸�
	//	job table�� �����Ѵ�.
	//
	if (rxResMsg->head.contFlag) {
		mmcdJobTbl[jobNo].deadlineTime = currentTime + rxResMsg->head.extendTime;
	} else {
		memset ((void*)&mmcdJobTbl[jobNo], 0, sizeof(MmcdJobTblContext));
	}

	return 1;

} //----- End of mmcd_exeMmcResMsg -----//
