/**** ��ɾ� ó�� ���� �Լ� ****/

#include <smpp.h>


extern char smpp_info_file[256];
extern  int smppQid, ixpcQid;
extern int	numMmcHdlr;
extern MmcHdlrVector mmcHdlrVector[MML_MAX_MMC_HANDLER];
extern char	resBuf[4096];
extern char resHead[1024];
extern char	mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN];



int write_smpp_info()
{
	int		fd;
	char    *env;
	if ((env = getenv(IV_HOME)) == NULL) {
        dAppLog(LOG_CRI, "[write_smpp_info]  not found %s environment name\n", IV_HOME); 
        return -1;
    }

    sprintf (smpp_info_file, "%s/%s", env, SMPP_CONF_FILE);

	fd = open(smpp_info_file, O_WRONLY | O_CREAT, 0664);
	if (fd < 0) {
		dAppLog(LOG_CRI, "[write_smpp_info] smpp_info_file open fail: err=%d\n", errno);
		return -1;
	}
	write(fd, smc_tbl, sizeof(SMC_TABLE));
	close(fd);

	return 1;

}


//------------------------------------------------------------------------------
// 1. bsearch �Լ����� �񱳿��Ƿ� ����ϴ� �Լ�  
//------------------------------------------------------------------------------
int smpp_mmcHdlrVector_bsrchCmp (const void *a, const void *b)
{
	return (strcasecmp ((char*)a, ((MmcHdlrVector*)b)->cmdName));
} //----- End of fimd_mmcHdlrVector_bsrchCmp -----//


//------------------------------------------------------------------------------
// 1. qsort �Լ����� �񱳿����� ����ϴ� �Լ�
//------------------------------------------------------------------------------
int smpp_mmcHdlrVector_qsortCmp (const void *a, const void *b)
{
	return (strcasecmp (((MmcHdlrVector*)a)->cmdName, ((MmcHdlrVector*)b)->cmdName));
} //----- End of fimd_mmcHdlrVector_qsortCmp -----//


int recv_mmc (IxpcQMsgType *rxIxpcMsg)
{
	MMLReqMsgType		*mmlReqMsg;
	MmcHdlrVector	*mmcHdlr;

	mmlReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	if ((mmcHdlr = (MmcHdlrVector*) bsearch (
					mmlReqMsg->head.cmdName,
					mmcHdlrVector,
					numMmcHdlr,
					sizeof(MmcHdlrVector),
					smpp_mmcHdlrVector_bsrchCmp)) == NULL) {
		dAppLog(LOG_CRI, "[Access_SMPPDB] received unknown mml_cmd(%s)\n", mmlReqMsg->head.cmdName);
		return -1;
	}

	// ó�� function�� ȣ���Ѵ�.
	(int)(*(mmcHdlr->func)) (rxIxpcMsg);
	return 1;

} //----- End of fimd_exeMMCMsg -----//


int smpp_mmc_set_smc_info (IxpcQMsgType *rxIxpcMsg)
{
#if 0 /* by june */
	int		i;
	char	resCode=0;
	char	tmpBuf[256];
	int		smc_index;
	MMLReqMsgType	*rxMmlReqMsg;
	memset(resBuf,0,sizeof(resBuf));
	rxMmlReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
/*** EKYANG
	smc_index = atoi(rxMmlReqMsg->head.para[0])-1;
***/	
	smc_index = atoi(rxMmlReqMsg->head.para[0]);
	strcpy(smc_tbl->smc_info[smc_index].ip_addr,rxMmlReqMsg->head.para[1]);
	strcpy(smc_tbl->smc_info[smc_index].user_id,rxMmlReqMsg->head.para[2]);
	strcpy(smc_tbl->smc_info[smc_index].passwd,rxMmlReqMsg->head.para[3]);
	strcpy(smc_tbl->smc_info[smc_index].use_scm,rxMmlReqMsg->head.para[4]);
	smc_tbl->smc_info[smc_index].port_no = atoi(rxMmlReqMsg->head.para[5]);

	
  	sprintf(resBuf,"SMC_ID      = %d           SYSTEM_TYPE = %s\n"
		"SYSTEM_ID   = %s         PASSWD      = %s\n"
		"PORT_NO     = %d        IP_ADDR     = %s"
		,smc_index,smc_tbl->smc_info[smc_index].use_scm
		,smc_tbl->smc_info[smc_index].user_id, smc_tbl->smc_info[smc_index].passwd
		, smc_tbl->smc_info[smc_index].port_no, smc_tbl->smc_info[smc_index].ip_addr);

	smpp_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 0);
	write_smpp_info();
#endif
	return 1;
}


int smpp_mmc_dis_smc_info (IxpcQMsgType *rxIxpcMsg)
{
#if 0 /* by june */
	int		i;
	char	resCode=0;
	char	tmpBuf[256];
	int		smc_index;
	MMLReqMsgType	*rxMmlReqMsg;
	memset(resBuf,0,sizeof(resBuf));
	rxMmlReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
	smc_index = atoi(rxMmlReqMsg->head.para[0]);
	
	if(smc_tbl->smc_info[smc_index].port_no == 0){
		resCode = -1;
		sprintf(resBuf,"  REASON = DATA NOT FOUND");
	}
	else {
		sprintf(resBuf,"SMC_ID      = %d           SYSTEM_TYPE = %s\n"
			"SYSTEM_ID   = %s         PASSWD      = %s\n"
			"PORT_NO     = %d        IP_ADDR     = %s"
			,smc_index,smc_tbl->smc_info[smc_index].use_scm
			,smc_tbl->smc_info[smc_index].user_id, smc_tbl->smc_info[smc_index].passwd
			, smc_tbl->smc_info[smc_index].port_no, smc_tbl->smc_info[smc_index].ip_addr);
	}

	smpp_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 0);
#endif
	return 1;

}


int smpp_mmc_del_smc_info (IxpcQMsgType *rxIxpcMsg)
{
#if 0 /* by june */
	int		i, tup_cnt=0;
	char	resCode=0;
	char	tmpBuf[256];
	int		smc_index;
	MMLReqMsgType	*rxMmlReqMsg;
	memset(resBuf,0,sizeof(resBuf));
	rxMmlReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
	smc_index = atoi(rxMmlReqMsg->head.para[0]);
	
	if(strlen(smc_tbl->smc_info[smc_index].ip_addr) == 0){
		strcpy(resBuf, "  REASON = DATA NOT FOUND");
		resCode = -1;
	}

	for(i=0; i< PREFIX_MAX; i++){
		if(smc_tbl->mc_entry[i] == smc_index)
			tup_cnt++;	
	}

	if(tup_cnt > 0){
		strcpy(resBuf, "  REASON = PREFIX IS EXIST");
		resCode = -1;	
		smpp_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 0);
		return -1;
	}

	memset(&smc_tbl->smc_info[smc_index],0,sizeof(SMC_INF));
	for(i =0 ;i< PREFIX_MAX; i++){
		if(smc_tbl->mc_entry[i]== smc_index)
			smc_tbl->mc_entry[i]= 0;
	}
	smpp_txMMLResult (rxIxpcMsg, resBuf, resCode, 0, 0, 0, 0);
	write_smpp_info();
#endif
	return 1;
	
}


//-------------------------------------------------------------------------------
//	MML Command �� ���� ���μ����� �Ϸ�� �� result message �� MMLResMsgType ��
//	�޽��� �������� ����� MMCD�� ixpcQid�� �̿��� msgsnd �Ѵ�.(�޽���ť ���)
//	@param  rxIxpcMsg  	: request ���� �޽��� (mmc command message)
//	@param	buff		: MML Command ��� �޽��� ���� ����
//	@param  resCode		: ��ɾ� ó�� ��� -> success(0), fail(-1)
//	@param  contFlag	: ������ �޽��� ���� ǥ�� -> last(0), not_last(1)
//	@param  extendTime 	: not_last�� ��� ���� �޽������� timer ����ð�(��) 
//		-> mmcd���� extendTime �ð���ŭ timer�� �����Ų��.
//	@param  segFlag		: �� �޽����� �ʹ� ��� �ѹ��� ������ ���� �� ����
//	@param  seqNo		: sequence number (segment�� ��� �Ϸù�ȣ) 
//
//	@return  1	 ����
//	@return -1	 ����
//--------------------------------------------------------------------------------
int smpp_txMMLResult (IxpcQMsgType *rxIxpcMsg, char *buff, char resCode,
			char contFlag, unsigned short extendTime, char segFlag, char seqNo)
{
	int		txLen;
	GeneralQMsgType	txGenQMsg;
	IxpcQMsgType	*txIxpcMsg;
	MMLResMsgType	*txMmlResMsg;
	MMLReqMsgType	*rxMmlReqMsg;

	txGenQMsg.mtype = MTYPE_MMC_RESPONSE;

	txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
	memset ((void*)&txIxpcMsg->head, 0, sizeof(txIxpcMsg->head));

	txMmlResMsg = (MMLResMsgType*)txIxpcMsg->body;
	rxMmlReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	// ixpc routing header
	strcpy (txIxpcMsg->head.srcSysName, mySysName);
	strcpy (txIxpcMsg->head.srcAppName, myAppName);
	strcpy (txIxpcMsg->head.dstSysName, rxIxpcMsg->head.srcSysName);
	strcpy (txIxpcMsg->head.dstAppName, rxIxpcMsg->head.srcAppName);
	txIxpcMsg->head.segFlag = segFlag;
	txIxpcMsg->head.seqNo   = seqNo;

	// mml result header
	txMmlResMsg->head.mmcdJobNo  = rxMmlReqMsg->head.mmcdJobNo;
	txMmlResMsg->head.extendTime = extendTime;
	txMmlResMsg->head.resCode    = resCode;
	txMmlResMsg->head.contFlag   = contFlag;
	strcpy (txMmlResMsg->head.cmdName, rxMmlReqMsg->head.cmdName);

	// result message
	strcpy (txMmlResMsg->body, buff);

	txIxpcMsg->head.bodyLen = sizeof(txMmlResMsg->head) + strlen(buff) + 1;
	txLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;

	if (msgsnd(ixpcQid, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0) {
		dAppLog(LOG_CRI, "[fimd_txMMLResult] msgsnd fail to MMCD; err=%d(%s)\n%s",
				errno, strerror(errno), buff);
		return -1;
	} else {
			dAppLog(LOG_CRI, "[fimd_txMMLResult] send to MMCD\n%s", buff);
	}
	return 1;

} //----- End of fimd_txMMLResult -----//

