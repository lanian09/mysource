#include "fimd_proto.h"

extern char trcBuf[4096];
extern int ixpcQid;
extern SFM_sfdb         *sfdb;

static time_t DupInfoRcvTime[2] = {0, 0};	// 0 -> SCMA, 1 -> SCMB

int fimd_exeDupStatusRpt(IxpcQMsgType *rxIxpcMsg)
{

	GeneralQMsgType txGQ;
	IxpcQMsgType	*txIQ;
	char		peerDupSts = 3; /* 3 means Unknown */
	dup_status_res *pDSr;
	int		txLen, i, sysindex = -1;
	char		logTemp[1024];
	time_t		lastRcvTime;


	for(i = 0; i < SYSCONF_MAX_ASSO_SYS_NUM; i++){
		if((!strncmp(sfdb->sys[i].commInfo.type, MP_SYSTEM_TYPE, COMM_MAX_NAME_LEN)) &&
			(!strncmp(sfdb->sys[i].commInfo.group, MP_SYSTEM_GROUP, COMM_MAX_NAME_LEN)) &&
			(strncmp(sfdb->sys[i].commInfo.name, rxIxpcMsg->head.srcSysName, COMM_MAX_NAME_LEN))){
			
			//index means that last character of "SCMA" or "SCAB", 'A' or 'B', minus 'A',
			//then we will get 0 or 1.
			lastRcvTime = DupInfoRcvTime[sfdb->sys[i].commInfo.name[3]-'A'];
			peerDupSts = sfdb->sys[i].commInfo.systemDup.myStatus;
			sysindex = i;
			break;
		}
	}

	txIQ = (IxpcQMsgType *)txGQ.body;
	txGQ.mtype = MTYPE_DUP_STATUS_RESPONSE;
	memset ((void*)&txIQ->head, 0, sizeof(txIQ->head));
	pDSr = (dup_status_res *)txIQ->body;

	strcpy (txIQ->head.srcSysName, rxIxpcMsg->head.dstSysName);
	strcpy (txIQ->head.srcAppName, rxIxpcMsg->head.dstAppName);
	strcpy (txIQ->head.dstSysName, rxIxpcMsg->head.srcSysName);
	strcpy (txIQ->head.dstAppName, rxIxpcMsg->head.srcAppName);
	
	txIQ->head.segFlag = 0;
	txIQ->head.seqNo = 1;

	if(sysindex < 0){	//System not found
		pDSr->response_result = 0;
	}else{			// System found
		pDSr->response_result = 1;
		pDSr->dup_status = peerDupSts;
		pDSr->dup_update_time = htonl(lastRcvTime);
	}

	txIQ->head.bodyLen = sizeof(txIQ->head) + sizeof(dup_status_res);
	txLen = sizeof(txIQ->head) + txIQ->head.bodyLen;
	
	if (msgsnd(ixpcQid, (void*)&txGQ, txLen, IPC_NOWAIT) < 0) {
		sprintf(logTemp, "[fimd_exeDupStatusRpt] msgsnd error = %s\n", strerror(errno));
		trclib_writeLogErr(FL,logTemp);
		return -1;
	} else {
		sprintf(logTemp, "[fimd_exeDupStatusRpt] msgsnd success\n");
		trclib_writeLogErr(FL,logTemp);
	}

	return 1;

} /* End of fimd_exeDupStatusRpt */

int fimd_exeDupUpdateNoti(IxpcQMsgType *rxIxpcMsg)
{
	dup_status_res *pDSr;
	int				i;

	pDSr = (dup_status_res *)rxIxpcMsg->body;

    for(i = 0; i < SYSCONF_MAX_ASSO_SYS_NUM; i++){
        if((!strncmp(sfdb->sys[i].commInfo.type, MP_SYSTEM_TYPE, COMM_MAX_NAME_LEN)) &&
           (!strncmp(sfdb->sys[i].commInfo.group, MP_SYSTEM_GROUP, COMM_MAX_NAME_LEN)) &&
		   (!strncmp(sfdb->sys[i].commInfo.name, rxIxpcMsg->head.srcSysName, COMM_MAX_NAME_LEN))){

			//index means that last character of "BSDA" or "BSDB", 'A' or 'B', minus 'A',
			//then we will get 0 or 1.
			DupInfoRcvTime[sfdb->sys[i].commInfo.name[3]-'A'] = time(NULL);
            sfdb->sys[i].commInfo.systemDup.myStatus = pDSr->dup_status;
			break;
        }
    }

	if(i >= SYSCONF_MAX_ASSO_SYS_NUM){
		sprintf(trcBuf,"[fimd_exeDupUpdateNoti] Unknown system name - %s\n", rxIxpcMsg->head.srcSysName);
		trclib_writeLogErr (FL,trcBuf);

	}

    return 1;

}
