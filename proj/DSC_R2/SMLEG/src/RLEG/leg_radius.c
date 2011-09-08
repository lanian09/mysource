/* File Include */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "ipaf_svc.h"
#include "utillib.h"
#include "leg.h"


/* Declaration of Global Variable */
//extern char sysLable[COMM_MAX_NAME_LEN], mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN];
/* Declaration of Extern Global Variable */
extern unsigned int	gMyIdx;
SM_BUF_CLR			gMySmBuf;

extern void dSetCurTIMEOUT(NOTIFY_SIG *pNOTISIG);

void dump_sce_login_info (PSUBS_INFO pSI)
{
	dAppLog(LOG_INFO, "[dGetSubsInfo] GET SUBSCRIBER-NAME		   	:%s", pSI->szMIN);
	dAppLog(LOG_INFO, "[dGetSubsInfo] GET SUBSCRIBER-IP			 	:%s", pSI->szFramedIP);
#if 0
	dAppLog(LOG_INFO, "[dGetSubsInfo] GET SUBSCRIBER-DOMAIN		 	:%s", pSI->szDomain);
	dAppLog(LOG_INFO, "[dGetSubsInfo] GET SUBSCRIBER-PROPERTY-NAME1 :%s", pSI->prop_key[0]);
	dAppLog(LOG_INFO, "[dGetSubsInfo] GET SUBSCRIBER-PROPERTY-VAL2  :%s", pSI->prop_val[0]);
	dAppLog(LOG_INFO, "[dGetSubsInfo] GET SUBSCRIBER-PROPERTY-NAME2 :%s", pSI->prop_key[1]);
	dAppLog(LOG_INFO, "[dGetSubsInfo] GET SUBSCRIBER-PROPERTY-KEY2  :%s", pSI->prop_val[1]);
#endif
}

void dump_sm_sub_clr (SM_BUF_CLR *pSmBuf)
{
	dAppLog(LOG_INFO, "[SM_BUF_CLR] dUsed=%d", gMyIdx, pSmBuf->dUsed);
	dAppLog(LOG_INFO, "[SM_BUF_CLR] dPeriod=%d", gMyIdx, pSmBuf->dPeriod);
	dAppLog(LOG_INFO, "[SM_BUF_CLR] dHour=%d", gMyIdx, pSmBuf->dHour);
	dAppLog(LOG_INFO, "[SM_BUF_CLR] uiClrPeriod=%d", gMyIdx, pSmBuf->uiClrPeriod);
}

void period_smbuf_clear (int k, void *d)
{
	time_t      cur_time;
	struct tm*  pcur_tm;
	SMNB_HANDLE  *pgSCE_nbapi=NULL;
	int rst=0;

	cur_time = time ((time_t *)0);
	pcur_tm  = (struct tm*)localtime((time_t*)&cur_time);

	if (gMySmBuf.dUsed != CMD_USE_ON) return;

	if (loc_sadb->loc_system_dup.myLocalDupStatus == SYS_STATE_ACTIVE) {
		pgSCE_nbapi = &gSMConn.hdl_nbSceApi;
		if (pgSCE_nbapi==NULL) {
			dAppLog(LOG_CRI, "[period_smbuf_clear] Connection is disconnected");
		}
		rst = SMNB_clearClientBuffer(*pgSCE_nbapi);
		dAppLog(LOG_CRI, "[period_smbuf_clear] SM Buffer is Cleared(RST=%d)", rst);
	}

	if (gMySmBuf.uiClrPeriod>0) {
		set_cb_timeout(period_smbuf_clear, 4, NULL, gMySmBuf.uiClrPeriod);
	}
	else {
		dAppLog(LOG_CRI, "[period_smbuf_clear] SM Buffer Clear Period is invalid(period=%u)", gMySmBuf.uiClrPeriod);
	}
}

int branchMessage (GeneralQMsgType *prxGenQMsg)
{
	USHORT			usSvcID;
	pst_MsgQ		pstMsgQ;
	pst_MsgQSub		pstMsgQSub;
	IxpcQMsgType	*rxIxpcMsg;
	SUBS_INFO		*pSubsInfo;
	NOTIFY_SIG		*pNOTIFY;
	HANDLE_t		stHandle;

	switch(prxGenQMsg->mtype)
	{
		case MTYPE_RADIUS_TRANSMIT:

			// SM CONNECT CHECK: MANUAL CONNECTION CASE
			if (loc_sadb->smConn[gMyIdx].dConn != CONNECTED) {
				return -1;
			}
			pstMsgQ		= (pst_MsgQ)&prxGenQMsg->body;
			pstMsgQSub 	= (pst_MsgQSub)pstMsgQ;
			usSvcID 	= pstMsgQSub->usSvcID;

			switch(usSvcID)
			{
			case SID_LOG_ON:		/* LOG-ON */

				pSubsInfo = (SUBS_INFO *)&pstMsgQ->szBody[0];
				Trace_LOGIN (pSubsInfo, TRACE_TYPE_LOGIN, 0);
				/////////////////////
				loginSCE (pSubsInfo);
				/////////////////////
				break;

			case SID_LOG_OUT:		/* LOG-OUT */
				pSubsInfo = (SUBS_INFO *)&pstMsgQ->szBody[0];
				Trace_LOGOUT (pSubsInfo, TRACE_TYPE_LOGOUT, 0);
				//////////////////////
				logoutSCE (pSubsInfo, SID_LOG_OUT);
				//////////////////////
				break;

			case SID_LOG_OUT_MMC:		/* LOG-OUT */
				pSubsInfo = (SUBS_INFO *)&pstMsgQ->szBody[0];
				Trace_LOGOUT (pSubsInfo, TRACE_TYPE_LOGOUT, 0);
				//////////////////////
				logoutSCE (pSubsInfo, SID_LOG_OUT_MMC);
				//////////////////////
				break;
					
			default:
				dAppLog(LOG_DEBUG, "ERROR NOT-MATCHED SVCID][*SID:%d", usSvcID);
				break;
			}
			break;
		case MTYPE_SM_BUF_CLR:
			pstMsgQ		= (pst_MsgQ)&prxGenQMsg->body;
			pstMsgQSub 	= (pst_MsgQSub)pstMsgQ;
			usSvcID 	= pstMsgQSub->usSvcID;

			switch(usSvcID)
			{
			case SID_SET_SM_BUF_CLR:	/* RADIUS MESSAGE */

				memcpy(&gMySmBuf, &pstMsgQ->szBody[0], sizeof(SM_BUF_CLR));
				dump_sm_sub_clr(&gMySmBuf);
				set_cb_timeout(period_smbuf_clear, 4, NULL, gMySmBuf.uiClrPeriod);
				break;

			case SID_DEL_SM_BUF_CLR:		/* LOG-OUT */
				memcpy(&gMySmBuf, &pstMsgQ->szBody[0], sizeof(SM_BUF_CLR));
				dump_sm_sub_clr(&gMySmBuf);
				gMySmBuf.dUsed			= CMD_USE_OFF;
				gMySmBuf.dPeriod 		= 0;
				gMySmBuf.dHour			= 0;
				gMySmBuf.uiClrPeriod	= 0;
				break;
					
			default:
				dAppLog(LOG_DEBUG, "ERROR NOT-MATCHED SVCID][*SID:%d", usSvcID);
				break;
			}
			break;
#if 1
		case MTYPE_TIMER_CONFIG : // MMCR -> IxpcQMsgType 으로 전달. 
			rxIxpcMsg	= (IxpcQMsgType *)&prxGenQMsg->body;
			pNOTIFY = (NOTIFY_SIG *)&rxIxpcMsg->body[0];
			dSetCurTIMEOUT(pNOTIFY);
			break;
#endif
		case MTYPE_CALL_BACK:
			pstMsgQ = (pst_MsgQ)&prxGenQMsg->body;
			memcpy(&stHandle, &pstMsgQ->szBody[0], sizeof(HANDLE_t));
			dProcHandle(&stHandle);
			break;
		default:
			dAppLog(LOG_CRI, "ERROR NOT-MATCHED GeneralQMsgType][MTYPE:%d"
					, prxGenQMsg->mtype);
			break;
	}

	return 0;
}

