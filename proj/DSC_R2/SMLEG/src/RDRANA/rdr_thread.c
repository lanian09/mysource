#include <sys/msg.h>
#include <shmutil.h>
#include "comm_msgtypes.h"
#include "ipam_sesssvc.h"
#include "sysconf.h"
#include "comm_util.h"
#include "utillib.h"

extern int dGetTraceData(pst_SESSInfo pstInfo);
extern int dMyQid;
extern st_SESSInfo	g_stTrcInfo;

extern int keepalivelib_increase(void);


void RealTimeTrace(void *arg)
{
	int ret;
	GeneralQMsgType rxGenQMsg;

	memset(&rxGenQMsg, 0, sizeof(GeneralQMsgType));

	while(1)
	{
		keepalivelib_increase();

		while ((ret = msgrcv(dMyQid, &rxGenQMsg, sizeof(GeneralQMsgType), 0, IPC_NOWAIT)) > 0 )
		{
			switch (rxGenQMsg.mtype)
			{
                case MTYPE_TRACE_INFO:
                    // TODO TrcInfo.conf ¨¡AAI reloading.
                    memset(&g_stTrcInfo, 0x00, sizeof(st_SESSInfo));
                    dGetTraceData(&g_stTrcInfo);
                    dAppLog(LOG_CRI, "MMCR Msg Get....");
                    break;
				default:
					break;

			}

			keepalivelib_increase();
			memset(&rxGenQMsg, 0, sizeof(GeneralQMsgType));

		}
		commlib_microSleep(1000);
	}
}
#if 0
int traceMMCMsg( IxpcQMsgType *rxIxpcMsg )
{
	MMLReqMsgType       *mmlReqMsg;
	RDRMmcHdlrVector	*mmcHdlrVector;

	mmlReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
	if ((mmcHdlr = (RDRMmcHdlrVector*) bsearch (
			mmlReqMsg->head.cmdName,
			mmcHdlrVector,
			numMmcHdlr,
			sizeof(RDRMmcHdlrVector),
			rdrana_mmcHdlrVector_bsrchCmp)) == NULL)
	{
		dAppLog(LOG_CRI,"[traceMMCMsg] received unknown mml_cmd(:%s:)", mmlReqMsg->head.cmdName);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	(int)(*(mmcHdlr->func)) (rxIxpcMsg);

	return 1;
}

int rdrana_mmc_dis_call_trc (IxpcQMsgType *rxIxpcMsg)
{

    return 0;
}

int rdrana_mmc_canc_call_trc (IxpcQMsgType *rxIxpcMsg)
{

    return 0;
}

int rdrana_mmc_reg_call_trc (IxpcQMsgType *rxIxpcMsg)
{

    return 0;
}

int rdrana_mmc_chg_call_trc (IxpcQMsgType *rxIxpcMsg)
{

    return 0;
}
   
#endif
