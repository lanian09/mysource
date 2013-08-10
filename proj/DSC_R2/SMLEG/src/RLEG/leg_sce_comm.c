#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/msg.h>
#include <pthread.h>
#include "utillib.h"
#include "leg.h"
#include "leg_sm_sess.h"
#include "ipaf_sem.h"

/* Function Time Check */
#ifdef FUNC_TIME_CHECK 
#include "func_time_check.h"
st_FuncTimeCheckList    	stFuncTimeCheckList;
st_FuncTimeCheckList    	*pFUNC = &stFuncTimeCheckList;
#endif
/* definition extern global variable */
extern char		 			mySysName[COMM_MAX_NAME_LEN];
extern LEG_DATA_SUM        *gpstCallInfo[DEF_STAT_SET_CNT];
extern LEG_TOT_STAT_t      *gpstTotStat[DEF_STAT_SET_CNT];
extern unsigned int			gCIdx;
extern unsigned int			gSIdx;
extern unsigned int			gMyIdx;

/* definition global variable */
int	giPrevSMConn;

/* definition function prototype*/
int mmc_sce_log_out (SUBS_INFO *psi, int succfail, int dErrCode);
int dSendCallBack(int argHandle, int dResult, int dErrCode, char *msg);
int dProcHandle(HANDLE_t *pHandle);

void connectionIsDown (void)
{
	/* disconnect callback function 은 Handle 이 없기 때문에 callback type을 넣어서 전송.. */
	dSendCallBack(SM_CB_HANDLE_DISCON, SM_CB_HANDLE_DISCON, 0, NULL);
	return;
}


//prints every error that occurs
void handleError (Uint32 argHandle, ReturnCode* argReturnCode)
{
	int				dErrCode = 0;

	dErrCode = argReturnCode->u.errorCode->type;
	dSendCallBack(argHandle, SM_CB_HANDLE_FAIL, dErrCode, NULL);

#ifdef DEBUG
	printReturnCode(argReturnCode);
#endif
	freeReturnCode(argReturnCode);
	return;
}


//prints a success result every 100 results
void handleSuccess (Uint32 argHandle, ReturnCode* argReturnCode)
{
	dSendCallBack(argHandle, SM_CB_HANDLE_SUCC, 0, NULL);

#ifdef DEBUG
	printReturnCode(argReturnCode);
#endif
	freeReturnCode(argReturnCode);
	return;
}

void checkTheArguments (int argc, char* argv[])
{
	if (argc != 4)
	{
		dAppLog (LOG_DEBUG, "usage: LoginLogout <SM-address> <domain> <num-susbcribers>");
		exit(1);
	}
}


int checkConnectSCE (void)
{
	SMNB_HANDLE     *pgSCE_nbapi = &gSMConn.hdl_nbSceApi;
	int rtn=0;

	if((rtn=SMNB_isConnected(*pgSCE_nbapi)) == 0) {
		if (giPrevSMConn == CONNECTED) {
			dAppLog (LOG_CRI, "CHK-CON][SM CON OFF");
			loc_sadb->smConn[gMyIdx].dConn = DISCONNECTED;
			giPrevSMConn = DISCONNECTED;
		}
#if 0
	int isConn=0;
		isConn = connect_sm (DO_CONN_ON_NB);
		if (isConn == 0) {
			//if (giPrevSMConn == CONNECTED) {
			dAppLog (LOG_CRI, "CHK-CON] retry failed ");
			loc_sadb->smConn[gMyIdx].dConn = DISCONNECTED;
			giPrevSMConn = DISCONNECTED;
			//}
		}
		else {
			//if (giPrevSMConn == DISCONNECTED) {
			dAppLog (LOG_CRI, "[CHK-CON] retry succ ");
			loc_sadb->smConn[gMyIdx].dConn = CONNECTED;
			giPrevSMConn = CONNECTED;
			rtn = 1;
			//}
		}
#endif
	}
	else {
		if (giPrevSMConn == DISCONNECTED) {
			dAppLog (LOG_CRI, "CHK-CON][SM CON ON");
			loc_sadb->smConn[gMyIdx].dConn = CONNECTED;
			giPrevSMConn = CONNECTED;
		}
	}

	return rtn;
}


int connectSCE (int conn_mode)
{
	SMNB_HANDLE     *pgSCE_nbapi = &gSMConn.hdl_nbSceApi;

	int             nPort;
	char            szIP[MAX_SM_IP_SIZE];
	int 			isConn=0;

	if(!(gSMConn.hdl_nbSceApi = SMNB_init(0, SM_API_BUF_SIZE, 10, 30))) {
		dAppLog (LOG_CRI, "CONNECT][SMNB_init failed");
		return -1;
	}

	if (!strncmp(mySysName, _SYS_NAME_MPA, 4)) {
		sprintf(szIP, "%s", gSCM[0].ip);
		nPort = gSCM[0].port;
	}
	else if (!strncmp(mySysName, _SYS_NAME_MPB, 4)) {
		sprintf(szIP, "%s", gSCM[1].ip);
		nPort = gSCM[1].port;
	}

	if (conn_mode == DO_CONN_ON_NB) {
		/* Non-Block API Connection */
		isConn = SMNB_connect (*pgSCE_nbapi, szIP, nPort);
#if 0
		if (isConn) {
			dAppLog (LOG_CRI, "CONNECT] STS=%d", isConn);
		}
#endif
		SMNB_setReplyFailCallBack (*pgSCE_nbapi, handleError);
		SMNB_setReplySuccessCallBack (*pgSCE_nbapi, handleSuccess);
		SMNB_setReconnectTimeout(*pgSCE_nbapi, 2);
		SMNB_setDisconnectListener (*pgSCE_nbapi, connectionIsDown);
	}

	return isConn;
}


int connect_sm (int conn_mode)
{
	SMNB_HANDLE     *pgSCE_nbapi = &gSMConn.hdl_nbSceApi;
	int 		isConn=0;
	int         nPort;
	char        szIP[MAX_SM_IP_SIZE];

	if (!strncmp(mySysName, _SYS_NAME_MPA, 4)) {
		sprintf(szIP, "%s", gSCM[0].ip);
		nPort = gSCM[0].port;
	}
	else if (!strncmp(mySysName, _SYS_NAME_MPB, 4)) {
		sprintf(szIP, "%s", gSCM[1].ip);
		nPort = gSCM[1].port;
	}

	isConn = SMNB_connect (*pgSCE_nbapi, szIP, nPort);
	dAppLog (LOG_CRI, "CONNECT] STS=%d", isConn);
	return isConn;
}


void disconnSCE (void)
{
	SMNB_HANDLE     *pgSCE_nbapi = &gSMConn.hdl_nbSceApi;

	/* non-block resource release */
	SMNB_disconnect (*pgSCE_nbapi);
    dAppLog(LOG_CRI, "### #1 disconnect");
	SMNB_release (*pgSCE_nbapi);
    dAppLog(LOG_CRI, "### #2 release");
}


int loginSCE (SUBS_INFO *si)
{
	char* ip = &(si->szFramedIP[0]);
	SMNB_HANDLE     *pgSCE_nbapi = &gSMConn.hdl_nbSceApi;
	char *prop_key[2] = { NULL,  NULL };
	char *prop_val[2] = { NULL,  NULL };
	int	argHandle=0;
	SM_SESS_KEY		stSmKey;
	SM_SESS_KEY		*pSmKey = &stSmKey;
	SM_SESS_BODY	*pSmBody;

	/* login operation */
	if (pgSCE_nbapi == NULL) {
		dAppLog (LOG_CRI, "LI:%d] %s %s %02d %02d %02d"
				, ERR_1001, si->szMIN, ip, si->uiCBit, si->uiPBit, si->uiHBit);
		return -1;
	}
	if (ip == NULL) {
		dAppLog (LOG_CRI, "LI] %s %02d %02d %02d"
				, ERR_1001, si->szMIN, si->uiCBit, si->uiPBit, si->uiHBit);
		return -1;
	}

	/* parameter malloc */
	if((prop_key[0]=(char*)malloc(10)))	sprintf(prop_key[0], "%s", PROPERTY_PACKAGE_ID);
	if((prop_val[0]=(char*)malloc(10)))	sprintf(prop_val[0], "%d", si->sPkgNo);

#ifdef FUNC_TIME_CHECK
	START_FUNC_TIME_CHECK(pFUNC, 0);
#endif

	/* logon statistic & cps count */
	gpstCallInfo[gCIdx]->cps[gMyIdx].uiLogOnSumCps++;
	gpstTotStat[gSIdx]->stLogon[gMyIdx][DEF_LOG_ON].uiSMIndex = gMyIdx;
	gpstTotStat[gSIdx]->stLogon[gMyIdx][DEF_LOG_ON].uiLogMode = DEF_LOG_ON;
	gpstTotStat[gSIdx]->stLogon[gMyIdx][DEF_LOG_ON].uiLogOn_Request++;
	gpstTotStat[gSIdx]->stLogon[gMyIdx][DEF_LOG_ON].uiLogOn_HBIT[si->uiHBit]++;

	/* login operation */
	argHandle = SMNB_login (*pgSCE_nbapi,
							si->szMIN, 			/* subscriber name */
							&ip, 				/* a single ip mapping */
							&si->type,			/* mapping type */
							1,					/* mapping size */
							prop_key,			/* property key */
							prop_val,			/* property value */
							1,					/* property count */
							si->szDomain, 		/* domain */
							0, 					/* mappings are not additive, 1:additive, 0:override */
							-1); 				/* disable auto-logout */

#ifdef FUNC_TIME_CHECK
	END_FUNC_TIME_CHECK(pFUNC, 0);
	PRINT_FUNC_TIME_CHECK(pFUNC);
#endif
	
	/* parameter delete */
	if (prop_key[0]) { free(prop_key[0]); prop_key[0] = NULL; }
	if (prop_val[0]) { free(prop_val[0]); prop_val[0] = NULL; }

	/* Return Check */
	if (argHandle < 0) {
		// LOGON 통계: 추가 필드 작업.
		gpstTotStat[gSIdx]->stLogon[gMyIdx][DEF_LOG_ON].uiLogOn_Fail++;
		gpstTotStat[gSIdx]->stLogon[gMyIdx][DEF_LOG_ON].uiLogOn_APIReqErr++;

		// TRACE: LOGON TRY FAIL
		Trace_LOGIN(si, TRACE_TYPE_LOGIN_FAIL, argHandle);

		dAppLog (LOG_CRI, "LI:%d] %010d %s %s %02d %02d %02d"
				, ERR_1003, argHandle, si->szMIN, ip, si->uiCBit, si->uiPBit, si->uiHBit);
		return -1;
	}

	pSmKey->uiSID = argHandle;
	if((pSmBody = get_sm_sess(pSmKey)) == NULL) {
		dAppLog (LOG_CRI, "LI:%d] %010d %s %s %02d %02d %02d"
				, ERR_1004, argHandle, si->szMIN, ip, si->uiCBit, si->uiPBit, si->uiHBit);
		return -2;
	}
	pSmBody->uiOperMode = DEF_LOG_ON;
	memcpy(&pSmBody->stSubs, si, sizeof(SUBS_INFO));
	
#ifdef PRT_LOGIN
	dAppLog (LOG_CRI, "LI] %010d %s %s %02d %02d %02d"
			, argHandle, si->szMIN, ip, si->uiCBit, si->uiPBit, si->uiHBit);
#endif
	return 0;
}

int logoutSCE (SUBS_INFO *si, unsigned short usSvcID)
{
	char* ip = &(si->szFramedIP[0]);
	SMNB_HANDLE     *pgSCE_nbapi = &gSMConn.hdl_nbSceApi;
	SM_SESS_KEY		stSmKey;
	SM_SESS_KEY		*pSmKey = &stSmKey;
	SM_SESS_BODY	*pSmBody;
	int argHandle=0;

	if (pgSCE_nbapi == NULL) {
		dAppLog (LOG_CRI, "LO:%d] %010d %s %s %02d %02d %02d"
				, ERR_2001, argHandle, si->szMIN, ip, si->uiCBit, si->uiPBit, si->uiHBit);
		if (usSvcID == SID_LOG_OUT_MMC)
			mmc_sce_log_out(si, MMC_LOGOUT_FAIL, -1);
		return -1;
	}
	if (ip == NULL) {
		dAppLog (LOG_CRI, "LO:%d] %010d %s %02d %02d %02d"
				, ERR_2001, argHandle, si->szMIN, si->uiCBit, si->uiPBit, si->uiHBit);
		if (usSvcID == SID_LOG_OUT_MMC)
			mmc_sce_log_out(si, MMC_LOGOUT_FAIL, -2);
		return -2;
	} 

	/* logon statistic & cps count */
	gpstCallInfo[gCIdx]->cps[gMyIdx].uiLogOutSumCps++;
	gpstTotStat[gSIdx]->stLogon[gMyIdx][DEF_LOG_OUT].uiSMIndex = gMyIdx;
	gpstTotStat[gSIdx]->stLogon[gMyIdx][DEF_LOG_OUT].uiLogMode = DEF_LOG_OUT;
	gpstTotStat[gSIdx]->stLogon[gMyIdx][DEF_LOG_OUT].uiLogOn_Request++;
	gpstTotStat[gSIdx]->stLogon[gMyIdx][DEF_LOG_OUT].uiLogOn_HBIT[si->uiHBit]++;

#ifdef FUNC_TIME_CHECK
	START_FUNC_TIME_CHECK(pFUNC, 0);
#endif
	argHandle = SMNB_logoutByMapping (*pgSCE_nbapi,
									 ip,
									 si->type,
									 si->szDomain);
#ifdef FUNC_TIME_CHECK
	END_FUNC_TIME_CHECK(pFUNC, 0);
	PRINT_FUNC_TIME_CHECK(pFUNC);
#endif

	if (argHandle < 0) {
		// LOGON 통계: 추가 필드 작업.
		gpstTotStat[gSIdx]->stLogon[gMyIdx][DEF_LOG_OUT].uiLogOn_Fail++;
		gpstTotStat[gSIdx]->stLogon[gMyIdx][DEF_LOG_OUT].uiLogOn_APIReqErr++;

		// TRACE: LOGOUT TRY FAIL
		Trace_LOGOUT(si, TRACE_TYPE_LOGOUT_FAIL, argHandle);
		dAppLog (LOG_CRI, "LO:%d] %010d %s %s %02d %02d %02d"
				, ERR_2003, argHandle, si->szMIN, ip, si->uiCBit, si->uiPBit, si->uiHBit);
		if( usSvcID == SID_LOG_OUT_MMC )
			mmc_sce_log_out(si, MMC_LOGOUT_FAIL, -3);
		return -3;
	}

	pSmKey->uiSID = argHandle;
	if((pSmBody = get_sm_sess(pSmKey)) == NULL) {
		dAppLog (LOG_CRI, "LO:%d] %010d %s %s %02d %02d %02d"
				, ERR_2004, argHandle, si->szMIN, ip, si->uiCBit, si->uiPBit, si->uiHBit);
		return -4;
	}
	pSmBody->uiOperMode = DEF_LOG_OUT;
	memcpy(&pSmBody->stSubs, si, sizeof(SUBS_INFO));

#ifdef PRT_LOGOUT
	dAppLog (LOG_CRI, "LO] %010d %s %s %02d %02d %02d"
					, argHandle, si->szMIN, ip, si->uiCBit, si->uiPBit, si->uiHBit);
#endif
	return argHandle;
}

#if 0
int findConnectSCE (void)
{   
	int i, rst=0;

	for (i=0 ; i < MAX_SM_CONN_COUNT ; i++)
	{
		if ((rst = checkConnectSCE (i)) == 0) return i;
	}
	return -1;
}
#endif

int dSendCallBack(int argHandle, int dCBType, int dErrCode, char *msg)
{
	int 	dRet;
	int		dSize;
	GeneralQMsgType txGenQMsg;
	pst_MsgQ        pstMsgQ;
	pst_MsgQSub     pstMsgQSub;
	HANDLE_t		stHandle;

	txGenQMsg.mtype = MTYPE_CALL_BACK;

	pstMsgQ = (st_MsgQ *)&txGenQMsg.body;
	pstMsgQ->llMType = 0; 
	pstMsgQSub = (st_MsgQSub *)&pstMsgQ->llMType;
	pstMsgQ->usBodyLen = sizeof(HANDLE_t);

	stHandle.argHandle = argHandle;
	stHandle.dCBType   = dCBType;
	stHandle.dErrCode  = dErrCode;
	// stHandle.szMsg = 현재 사용 안함.
	memcpy(&pstMsgQ->szBody[0], &stHandle, sizeof(HANDLE_t));

	/* send subs info msg */
	dSize = sizeof(st_MsgQ) - MAX_MSGBODY_SIZE + pstMsgQ->usBodyLen;
	if((dRet = msgsnd(dMyQid, &txGenQMsg, dSize, 0)) < 0) {
		dAppLog( LOG_CRI, "SEND_CB] [Qid=%d] SEND ERROR: %d:%d", dMyQid, argHandle, dCBType);
		return -1;
	}

	return 0;
}


int dHandleSucess (HANDLE_t *pHandle)
{
	SM_SESS_KEY		stSmKey;
	SM_SESS_KEY		*pSmKey = &stSmKey;
	SM_SESS_BODY	*pSmBody;

	pSmKey->uiSID = pHandle->argHandle;
	pSmBody = find_sm_sess(pSmKey);
	if (pSmBody == NULL) {
		dAppLog(LOG_CRI, "SUC_CB:%d] (%u)", ERR_3001, pHandle->argHandle);
		return -1;
	}

	if (pSmBody->uiOperMode > DEF_LOG_OUT) {
		dAppLog(LOG_CRI, "SUC_CB] (%u)", ERR_3002, pHandle->argHandle);
		del_sm_sess(pSmKey, pSmBody);
		return -1;
	}

	if (pSmBody->uiOperMode == DEF_LOG_ON) {
		gpstTotStat[gSIdx]->stLogon[gMyIdx][pSmBody->uiOperMode].uiLogOn_Success++;
		Trace_LOGIN(&pSmBody->stSubs, TRACE_TYPE_LOGIN_SUCCESS, 0);
		dAppLog (LOG_DEBUG, "SUC_CB] LI(%010u)", pHandle->argHandle);
	}
	else if(pSmBody->uiOperMode == DEF_LOG_OUT) {
		gpstTotStat[gSIdx]->stLogon[gMyIdx][pSmBody->uiOperMode].uiLogOn_Success++;
		Trace_LOGIN(&pSmBody->stSubs, TRACE_TYPE_LOGOUT_SUCCESS, 0);
		if (pSmBody->stSubs.usSvcID == SID_LOG_OUT_MMC)
			mmc_sce_log_out(&pSmBody->stSubs, MMC_LOGOUT_SUCC, 0);
		dAppLog (LOG_DEBUG, "SUC_CB] LO(%010u)", pHandle->argHandle);
	}

	del_sm_sess(pSmKey, pSmBody);
	return 0;
}


int dHandleFail (HANDLE_t *pHandle)
{
	SM_SESS_KEY		stSmKey;
	SM_SESS_KEY		*pSmKey = &stSmKey;
	SM_SESS_BODY	*pSmBody;

	pSmKey->uiSID = pHandle->argHandle;
	pSmBody = find_sm_sess(pSmKey);
	if (pSmBody == NULL) {
		dAppLog(LOG_CRI, "ERR_CB:%d] (%u)", ERR_4001, pHandle->argHandle);
		return -1;
	}

	if (pSmBody->uiOperMode > DEF_LOG_OUT) {
		dAppLog(LOG_CRI, "ERR_CB%d] (%u)", ERR_4002, pHandle->argHandle);
		del_sm_sess(pSmKey, pSmBody);
		return -1;
	}

	if (pSmBody->uiOperMode == DEF_LOG_ON) {
		Trace_LOGIN(&pSmBody->stSubs, TRACE_TYPE_LOGIN_FAIL, pHandle->dErrCode);
		dAppLog (LOG_CRI, "ERR_CB] LI (%010u:%d)", pHandle->argHandle, pHandle->dErrCode);
	}
	else if( pSmBody->uiOperMode == DEF_LOG_OUT )
	{
		Trace_LOGOUT(&pSmBody->stSubs, TRACE_TYPE_LOGOUT_FAIL, pHandle->dErrCode);
		dAppLog (LOG_CRI, "ERR_CB] LO (%010u:%d)", pHandle->argHandle, pHandle->dErrCode);
		if (pSmBody->stSubs.usSvcID == SID_LOG_OUT_MMC)
			mmc_sce_log_out(&pSmBody->stSubs, MMC_LOGOUT_FAIL, pHandle->dErrCode);
	}

	switch( pHandle->dErrCode )
	{
		case 0:
		case 1:
		case 2:
		case 30000:
		case 30002:
		case 30003:
		case 30004:
		case 30005:
		case 100002:
		case 8000000:
		case 8000001:
			gpstTotStat[gSIdx]->stLogon[gMyIdx][pSmBody->uiOperMode].uiLogOn_Reason1++;
			break;

		case 33006:
		case 33008:
		case 100000:
		case 100001:
		case 100003:
		case 100004:
		case 100005:
		case 100006:
		case 100007:
		case 100008:
			gpstTotStat[gSIdx]->stLogon[gMyIdx][pSmBody->uiOperMode].uiLogOn_Reason2++;
			break;

		case 8000002:
			gpstTotStat[gSIdx]->stLogon[gMyIdx][pSmBody->uiOperMode].uiLogOn_Reason3++;
			break;

		default:
			gpstTotStat[gSIdx]->stLogon[gMyIdx][pSmBody->uiOperMode].uiLogOn_Reason4++;
			break;
	}

	del_sm_sess(pSmKey, pSmBody);
	return 0;
}


int dHandleDisconnect (HANDLE_t *pHandle)
{
	dAppLog (LOG_CRI, "[DISCONN_CB] connection is down");
	//disconnSCE ();
#if 0
	int isConn=0;

	isConn = connect_sm(DO_CONN_ON_NB);
	if (isConn == 0) {
		dAppLog (LOG_CRI, "[DISCONN_CB] retry failed ");
		loc_sadb->smConn[gMyIdx].dConn = DISCONNECTED;
	}
	else {
		dAppLog (LOG_CRI, "[DISCONN_UP] retry succ ");
		loc_sadb->smConn[gMyIdx].dConn = CONNECTED;
	}
#endif
	return 0;
}

int dProcHandle(HANDLE_t *pHandle)
{
	int dRet=0;

	if(pHandle->dCBType == SM_CB_HANDLE_SUCC) {
		dRet = dHandleSucess (pHandle);
	}
	else if( pHandle->dCBType == SM_CB_HANDLE_FAIL) {
		dRet = dHandleFail (pHandle);
	}
	else if( pHandle->dCBType == SM_CB_HANDLE_DISCON) {
		dRet = dHandleDisconnect (pHandle);
	}

	return dRet;
}

