#include "fimd_proto.h"
#include "sfm_snmp.h"
#include "netOrderSysInfoChange.h"   /*add by sjjeon*/
#include "netOrderSceInfoChange.h"   /*add by sjjeon*/


extern int		ixpcQid, condQid, nmsifQid, eqSysCnt;
extern int		dataPortNum, eventPortNum;
extern char		mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN];
extern char		trcBuf[4096], trcTmp[1024], sysLabel[COMM_MAX_NAME_LEN];
extern SFM_sfdb		*sfdb;
extern SFM_L3PD		*l3pd;
extern SFM_SCE		*g_pstSCEInfo;
//extern SFM_LEG		*g_pstLEGInfo;
extern SFM_CALL		*g_pstCALLInfo; // added by dcham 20110525
extern SFM_L2Dev    *g_pstL2Dev;
extern SFM_LOGON    *g_pstLogonRate;
extern SFM_LOGON    g_stLogonRate[LOG_MOD_CNT][2];
extern int		trcLogId, trcErrLogId, trcFlag, trcLogFlag;
extern FimdClientContext	cliTbl[SOCKLIB_MAX_CLIENT_CNT];
/* 20040921-mnpark */
extern int 		dbSyncFailCnt[SYSCONF_MAX_ASSO_SYS_NUM];
extern FimdKeepAlive	fimdKeepAlive[MAX_KEEPALIVE];
extern int 		dbDeadCnt[SYSCONF_MAX_ASSO_SYS_NUM];
extern int		*sound_flag;
extern int		first_flag;
extern char    		*rsrcName[SFM_MAX_RSRC_LOAD_CNT];

	
FimdProcMonSMS 	procSysInfo[3]; 
int	cur_smsalm;

int fimd_getSysIndexByType (char *sysType);
int procGetState(char *status);

//------------------------------------------------------------------------------
// sfdb->sys에서 system name과 일치하는 index를 찾는다.
//------------------------------------------------------------------------------
int fimd_getSysIndexByName (char *sysName)
{
	int		i;
	for (i=0; i<SYSCONF_MAX_ASSO_SYS_NUM; i++) {
		if (!strcasecmp (sfdb->sys[i].commInfo.name, sysName)){

			return i;
		}

	}
	i = fimd_getSysIndexByType(sysName);
	if ( i >= 0 ) return i;

	sprintf(trcBuf,"[fimd_getSysIndexByName] not found sysName[%s]\n", sysName);
	trclib_writeLogErr (FL,trcBuf);

	return -1;

} //----- End of fimd_getSysIndexByName -----//

//------------------------------------------------------------------------------
// sfdb->sys에서 system name과 일치하는 index를 찾는다.
//------------------------------------------------------------------------------
int fimd_getSysIndexByType (char *sysType)
{
    int     i;

    for (i=0; i<SYSCONF_MAX_ASSO_SYS_NUM; i++) {
        if (!strcasecmp (sfdb->sys[i].commInfo.type, sysType))
            return i;
    }
    sprintf(trcBuf,"[fimd_getSysIndexByType] not found sysType[%s]\n", sysType);
    trclib_writeLogErr (FL,trcBuf);
    return -1;

} //----- End of fimd_getSysIndexByType -----//





//------------------------------------------------------------------------------
// sfdb->group에서 system group name과 일치하는 index를 찾는다.
//------------------------------------------------------------------------------
int fimd_getGroupIndexByName (char *groupName)
{
	int		i;

	for (i=0; i<SYSCONF_MAX_GROUP_NUM; i++) {
		if (!strcasecmp (sfdb->group[i].name, groupName))
			return i;
	}
	sprintf(trcBuf,"[fimd_getGroupIndexByName] not found groupName[%s]\n", groupName);
	trclib_writeLogErr (FL,trcBuf);
	return -1;

} //----- End of fimd_getGroupIndexByName -----//



//------------------------------------------------------------------------------
// sfdb->sys에서 해당 시스템내의 disk name과 일치하는 index를 찾는다.
//------------------------------------------------------------------------------
int fimd_getDiskIndexByName (int sysIndex, char *diskName)
{
	int		i;

	for (i=0; i<SFM_MAX_DISK_CNT; i++) {
		if (!strcasecmp (sfdb->sys[sysIndex].commInfo.diskInfo[i].name, diskName))
			return i;
	}
	sprintf(trcBuf,"[fimd_getDiskIndexByName] not found diskName[%s] at %s\n",
			diskName, sfdb->sys[sysIndex].commInfo.name);
	trclib_writeLogErr (FL,trcBuf);
	return -1;

} //----- End of fimd_getDiskIndexByName -----//



//------------------------------------------------------------------------------
// sfdb->sys에서 해당 시스템내의 lan name과 일치하는 index를 찾는다.
//------------------------------------------------------------------------------
int fimd_getLanIndexByName (int sysIndex, char *lanIp)
{
	int	i;

	for (i=0; i<SFM_MAX_LAN_CNT; i++) {
		/* modify by mnpark - 20040112*/
		/*if (!strcasecmp (sfdb->sys[sysIndex].commInfo.lanInfo[i].name, lanName))*/
		if (!strcasecmp (sfdb->sys[sysIndex].commInfo.lanInfo[i].targetIp, lanIp))
			return i;
	}
	sprintf(trcBuf,"[fimd_getLanIndexByName] not found lanIp[%s] at %s\n",
			lanIp, sfdb->sys[sysIndex].commInfo.name);
	trclib_writeLogErr (FL,trcBuf);
	return -1;

} //----- End of fimd_getLanIndexByName -----//

//-----------------------//
/* by helca */

int fimd_getRmtLanIndexByName (int sysIndex, char *rmtLanIp)
{
	int		i;

	for (i=0; i<SFM_MAX_RMT_LAN_CNT; i++) {
		if (!strcasecmp (sfdb->sys[sysIndex].commInfo.rmtLanInfo[i].targetIp, rmtLanIp))
			return i;
	}
	sprintf(trcBuf,"[fimd_getRmtLanIndexByName] not found rmtLanIp[%s] at %s\n",
			rmtLanIp, sfdb->sys[sysIndex].commInfo.name);
	trclib_writeLogErr (FL,trcBuf);
	return -1;

} //----- End of fimd_getRmtLanIndexByName -----//

int fimd_getPDSNIPIndexByName (int sysIndex, char *pdsnIp)
{
	int i;
	for (i=0; i<RADIUS_IP_CNT; i++) {
                if (!strcasecmp (sfdb->sys[sysIndex].succRateIpInfo.radius[i].ipAddr, pdsnIp))
                        return i;
        }
        sprintf(trcBuf,"[fimd_getPDSNIPIndexByName] not found pdsnIp[%s] at %s\n",
                        pdsnIp, sfdb->sys[sysIndex].commInfo.name);
        trclib_writeLogErr (FL,trcBuf);
        return -1;

}




int fimd_getOptLanIndexByName (int sysIndex, char *optLanIp)
{
	int		i;

	for (i=0; i<2; i++) {
		if (!strcasecmp (sfdb->sys[sysIndex].commInfo.optLanInfo[i].name, optLanIp))
			return i;
	}
	sprintf(trcBuf,"[fimd_getOptLanIndexByName] not found optLanIp[%s] at %s\n",
			optLanIp, sfdb->sys[sysIndex].commInfo.name);
	trclib_writeLogErr (FL,trcBuf);
	return -1;

} //----- End of fimd_getOptLanIndexByName -----// 

int fimd_getSuccRateIndexByName (int sysIndex, char *IndexName)
{
	int		i;
	for (i=0; i<SFM_REAL_SUCC_RATE_CNT; i++) {
		if (!strcasecmp ((char*)sfdb->sys[sysIndex].commInfo.succRate[i].name, IndexName))
			return i;
	}
	sprintf(trcBuf,"[fimd_getSuccRateIndexByName] not found IndexName[%s] at %s\n",
			IndexName, sfdb->sys[sysIndex].commInfo.name);
	trclib_writeLogErr (FL,trcBuf);
	return -1;

} //----- End of fimd_getSuccRateIndexByName -----// 

int fimd_getRmtDbLanIndexByName (int sysIndex, char *rmtDbIp)
{
	int		i;

	for (i=0; i<SFM_MAX_DB_CNT; i++) {
		if (!strcasecmp ((char*)sfdb->sys[sysIndex].commInfo.rmtDbSts[i].sIpAddress, rmtDbIp))
			return i;
	}
	sprintf(trcBuf,"[fimd_getRmtDbLanIndexByName] not found rmtDbIp[%s] at %s\n",
			rmtDbIp, sfdb->sys[sysIndex].commInfo.name);
	trclib_writeLogErr (FL,trcBuf);
	return -1;

} //----- End of fimd_getRmtDbLanIndexByName -----//
// --- end by helca --- //

int fimd_getRsrcIndexByName (int sysIndex, char *IndexName)
{
	int		i;

	for(i = 0; i < SFM_MAX_RSRC_LOAD_CNT; i++){
		if(rsrcName[i] && !strcasecmp(IndexName, rsrcName[i]))
			return i;
	}

	sprintf(trcBuf,"[fimd_getSuccRateIndexByName] not found IndexName[%s] at %s\n",
			IndexName, sfdb->sys[sysIndex].commInfo.name);
	trclib_writeLogErr (FL,trcBuf);
	return -1;

} //----- End of fimd_getRsrcIndexByName -----// 

int fimd_getNmsifIndexByName (int sysIndex, char *IndexName)
{

    if(sysIndex != 0)
        return -1;

    if(!strcasecmp(IndexName, "CONSOLE"))
        return 1;
    else if(!strcasecmp(IndexName, "ALARM"))
        return 0;
    else if(!strcasecmp(IndexName, "STATISTICS"))
        return 4;
    else if(!strcasecmp(IndexName, "CONFIG"))
        return 2;
    else if(!strcasecmp(IndexName, "MMC"))
        return 3;    
	else
		return -1;
	
/*
    sprintf(trcBuf,"[fimd_getNmsifIndexByName] not found IndexName[%s] at %s\n",
            IndexName, sfdb->sys[sysIndex].commInfo.name);
    trclib_writeLogErr (FL,trcBuf);
    return -1;
*/

} //----- End of fimd_getNmsifIndexByName -----//

int fimd_gethwNtpIndexByName (int sysIndex, char *IndexName)
{
	int		i;
	char	ntpName[2][10];

	strcpy(ntpName[0], "DAEMON");
	strcpy(ntpName[1], "CHANNEL");
	for (i=0; i<MAX_HW_NTP; i++) {
		if (!strcasecmp (ntpName[i], IndexName))
			return i;
	}
	sprintf(trcBuf,"[fimd_gethwNtpIndexByName] not found IndexName[%s] at %s\n",
			IndexName, sfdb->sys[sysIndex].commInfo.name);
	trclib_writeLogErr (FL,trcBuf);
	return -1;

} //----- End of fimd_gethwNtpIndexByName -----//

//------------------------------------------------------------------------------
// sfdb->sys에서 해당 시스템내의 proc name과 일치하는 index를 찾는다.
//------------------------------------------------------------------------------
int fimd_getProcIndexByName (int sysIndex, char *procName)
{
	int		i;

	for (i=0; i<SFM_MAX_PROC_CNT; i++) {
		if (!strcasecmp (sfdb->sys[sysIndex].commInfo.procInfo[i].name, procName))
			return i;
	}
	sprintf(trcBuf,"[fimd_getProcIndexByName] not found procName[%s] at %s\n",
			procName, sfdb->sys[sysIndex].commInfo.name);
	trclib_writeLogErr (FL,trcBuf);
	return -1;

} //----- End of fimd_getProcIndexByName -----//

//------------------------------------------------------------------------------
// sfdb->sys에서 해당 시스템내의 HW name과 일치하는 index를 찾는다.
//------------------------------------------------------------------------------
#if 1 // by helca
int fimd_getHwIndexByName (int sysIndex, char *hwName)
{
	int		i;
	for (i=0; i<SFM_MAX_HPUX_HW_COM; i++) {
		if (!strcasecmp (sfdb->sys[sysIndex].specInfo.u.sms.hpuxHWInfo.hwcom[i].name, hwName))
			return i;
	}

	sprintf(trcBuf,"[fimd_getHwIndexByName] not found hwName[%s] at %s\n",
			hwName, sfdb->sys[sysIndex].commInfo.name);
	trclib_writeLogErr (FL,trcBuf);
	return -1;

} //----- End of fimd_getHwIndexByName -----//
#endif

//------------------------------------------------------------------------------
// client table을 검색하여 주어진 fd와 일치하는 놈을 찾아 그 index를 return한다.
//------------------------------------------------------------------------------
int fimd_getCliIndex (int fd)
{
    int     cliIndex;

    for (cliIndex=0; cliIndex < SOCKLIB_MAX_CLIENT_CNT; cliIndex++) {
        if (cliTbl[cliIndex].sockFd == fd)
            return cliIndex;
    }
    return -1;

} //----- End of fimd_getCliIndex -----//



//------------------------------------------------------------------------------
// 장애,상태 메시지를 cond로 보낸다.
//------------------------------------------------------------------------------
int fimd_txMsg2Cond (char *buff, long mtype, int msgId)
{
	int				txLen;
	GeneralQMsgType	txGenQMsg;
	IxpcQMsgType	*txIxpcMsg;

	txGenQMsg.mtype = mtype; // cond에서 mtype에 따라 장애메시지, 상태메시지를 다른 곳에 logging한다.

	txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
	memset ((void*)&txIxpcMsg->head, 0, sizeof(txIxpcMsg->head));

	strcpy (txIxpcMsg->head.srcSysName, mySysName);
	strcpy (txIxpcMsg->head.srcAppName, myAppName);
	strcpy (txIxpcMsg->head.dstSysName, mySysName);
	strcpy (txIxpcMsg->head.dstAppName, "COND");
	txIxpcMsg->head.msgId   = msgId; // NMSIB에서만 임시로 사용하는 값이다. COND에서는 사용하지 않는다.
	txIxpcMsg->head.segFlag = 0;
	txIxpcMsg->head.seqNo   = 1;

	strcpy (txIxpcMsg->body, buff);
	txIxpcMsg->head.bodyLen = strlen(buff);
	txLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;

	if (msgsnd(condQid, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0) {
		sprintf(trcBuf,"[fimd_txMsg2Cond] msgsnd fail to COND; err=%d(%s)\n%s",
				errno, strerror(errno), buff);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	} else {
		if (trcFlag || trcLogFlag) {
			sprintf(trcBuf,"[fimd_txMsg2Cond] send to COND\n%s", buff);
			trclib_writeLog (FL,trcBuf);
		}
	}
	return 1;

} //----- End of fimd_txMsg2Cond -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int fimd_txMMLResult (
			IxpcQMsgType *rxIxpcMsg,
			char *buff,
			char resCode,
			char contFlag,
			unsigned short extendTime,
			char segFlag,
			char seqNo
			)
{
	int	txLen;
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

	txIxpcMsg->head.bodyLen = sizeof(txMmlResMsg->head) + strlen(buff);
	txLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;

	if (msgsnd(ixpcQid, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0) {
		sprintf(trcBuf,"[fimd_txMMLResult] msgsnd fail to MMCD; err=%d(%s)\n%s",
				errno, strerror(errno), buff);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	} else {
		if (trcFlag || trcLogFlag) {
			sprintf(trcBuf,"[fimd_txMMLResult] send to MMCD\n%s", buff);
			trclib_writeLog (FL,trcBuf);
		}
	}
	return 1;

} //----- End of fimd_txMMLResult -----//



//------------------------------------------------------------------------------
// sfdb->sys에 있는 해당 시스템 정보를 접속되어 있는 모든 GUI client로 보낸다.
// - data port로 접속되고 clinet의 state가 ACTIVE인 놈들로만 보낸다.
//------------------------------------------------------------------------------
int fimd_broadcastSfdb2Client (int sysIndex)
{
	int		i;
	for (i=0; i<SOCKLIB_MAX_CLIENT_CNT; i++) {

		if (cliTbl[i].bindPortNum != dataPortNum ||
			cliTbl[i].state != FIMD_CLIENT_STATE_ACTIVE){
#if 0 /* by june */
			fprintf(stderr, "sys:%d, bind port:%d, data port:%d, client state:%d\n"
					, sysIndex
					, cliTbl[i].bindPortNum
					, dataPortNum
					, cliTbl[i].state);
#endif
			continue;
		}
		fimd_sendSfdb2Client (sysIndex, i);
		//fimd_broadcastMessageInfo(sysIndex, i);
	}

	return 1;

} //----- End of fimd_broadcastSfdb2Client -----//

void dumpSockLibHeadType(SockLibHeadType *hdr)
{
#if 1
	fprintf(stderr, "### Header Dump ###\n");
	fprintf(stderr, " @ map type: %d\n", hdr->mapType);
	fprintf(stderr, " @ seg flag: %d\n", hdr->segFlag);
	fprintf(stderr, " @ seq no  : %d\n", hdr->seqNo);
	fprintf(stderr, " @ body len: %d\n", hdr->bodyLen);
#endif
}

/**
* LOGON 성공율 감시를 위한 Network Ordering Converting 함수
* added by uamyd 20110210
*/
void SFM_LOGON_SUCC_RATE_H2N(SFM_LOGON  *logonInf)
{
//    unsigned char  mask;
//    unsigned short rate;
    logonInf->rate     = (unsigned short)htons(logonInf->rate);
//    unsigned char  level;
    logonInf->minLimit = (unsigned int)htonl(logonInf->minLimit);
    logonInf->majLimit = (unsigned int)htonl(logonInf->majLimit);
    logonInf->criLimit = (unsigned int)htonl(logonInf->criLimit);
//    unsigned char  minFlag;     
//    unsigned char  majFlag;
//    unsigned char  criFlag;

}

//------------------------------------------------------------------------------
// - sfdb에 있는 상태/장애 정보를 infoType에 따라  특정 client로 보낸다.
//	 . infoType->: 1:SFM_SysInfo
// - spCnt를 이용  신호점 서브시스템의 경우 실제 실장된 정보만 GUI로 전송.
//   . spCnt가 128이상이면 1st 128개의 신호점/서브시스템 정보를 먼저 전송하고
//     나머지를 2nd에 전송한다.
//------------------------------------------------------------------------------
int fimd_sendSfdb2Client  (int sysIndex, int cliIndex)
{
	int		segFlag = 1, txLen, bodyLen, infoType = 0;
	void	*srcP;
	SFM_NMSInfo		*nmsinfo;
	SFM_L3PD		*pL3pd;

	void  *pSys;
	SFM_SysCommInfo *psyscomm;
	SockLibMsgType	txSockMsg;
	int i, systype;

		
	do {
		infoType++;

		switch(infoType) {
			case GUI_SYS_INFO_TYPE	: 
   				bodyLen = sizeof(SFM_SysAlmInfo) + sizeof(SFM_SysCommInfo);
    			srcP = &sfdb->sys[sysIndex];
				break;
			case 2 : /* GUI_HW_INFO */
 				bodyLen = sizeof(SFM_SysInfo) - (sizeof(SFM_SysAlmInfo) + sizeof(SFM_SysCommInfo) + sizeof(SFM_SysSuccRateIpInfo));
				srcP = &sfdb->sys[sysIndex].specInfo;
 				break;
			case 3 : /* L3PD_NMS_INFO */
				bodyLen = sizeof(SFM_L3PD)+sizeof(SFM_NMSInfo);
				srcP = l3pd;
				break;
			case 4 : /* snmp 로 가져온 SCE Device Information */
				bodyLen = sizeof(SFM_SCE);
				srcP = g_pstSCEInfo;
				break;
			case 5: /* L2Switch */
				bodyLen = sizeof(SFM_L2Dev);
				srcP = g_pstL2Dev;
				break;
			/* hjjung */
			case 6:
				bodyLen = sizeof(SFM_CALL);
				//srcP = &sfdb->legData;
				srcP = g_pstCALLInfo;
			    sprintf(trcBuf,"[fimd_proc] TPS1:%d(%d), TPS2:%d(%d), SESS1:%d, SESS2:%d, CPS_ON:%d, CPS_OFF:%d\n",
						g_pstCALLInfo->tpsInfo[0].num,g_pstCALLInfo->tpsInfo[0].level,
						g_pstCALLInfo->tpsInfo[1].num,g_pstCALLInfo->tpsInfo[1].level,
						g_pstCALLInfo->legInfo[0].num,g_pstCALLInfo->legInfo[1].num, 
						g_pstCALLInfo->cps.uiLogOnSumCps, g_pstCALLInfo->cps.uiLogOutSumCps);
			     trclib_writeLogErr (FL,trcBuf);
/* DEBUG: by june, 2010-09-10
 *		- RLEG CPS 트버블 슈팅.
 */
#if 0
sprintf(trcBuf,"[fimd_sendSfdb2Client] >>>>>> CPS logon=%d logout=%d\n"
		, g_pstLEGInfo->cps.uiLogOnSumCps
		, g_pstLEGInfo->cps.uiLogOutSumCps);
trclib_writeLogErr (FL,trcBuf);
#endif
				break;

			case 7: //LOGON SUCCESS RATE
				bodyLen = sizeof(SFM_LOGON)*2;
				srcP    = &g_stLogonRate[0][0];
				break;
			case 8: //LOGOUT SUCCESS RATE
				bodyLen = sizeof(SFM_LOGON)*2;
				srcP    = &g_stLogonRate[1][0];
				break;
#if 0
			case 9:
				bodyLen = sizeof(SFM_CALL);
				srcP = g_pstCALLInfo;
                                break;
#endif
			default :
				return 1;
				break;
		}
		
		if(infoType == 9) segFlag = 0;
		else segFlag = 1;
		if((infoType < 1) || (infoType > 8)) {
			sprintf(trcBuf,"[fimd_sendSfdb2Client] infoType Err id=%d\n",infoType);
			trclib_writeLogErr (FL,trcBuf);
			return 0;
		}
		// yhshin
		txSockMsg.head.mapType = sysIndex;
		txSockMsg.head.segFlag = segFlag;
		txSockMsg.head.seqNo   = infoType;
		txSockMsg.head.bodyLen = bodyLen;

		/* network order change : gui로 전송전에 network ordering을 변경한다. - sjjeon */
		switch(infoType){
			case 1:
   				//bodyLen = sizeof(SFM_SysAlmInfo) + sizeof(SFM_SysCommInfo);
				memcpy ((void*)txSockMsg.body, (void*)srcP, bodyLen);

				psyscomm = (SFM_SysCommInfo *)(txSockMsg.body + sizeof(SFM_SysAlmInfo));
//				printf ("------ psyscomm->procCnt %x.\n", psyscomm->procCnt);
//				for (i=0; i < SFM_MAX_PROC_CNT-15; i++)
//					printf ("------ psyscomm->procInfo.pid %d.\n", (SFM_SysCommInfo *)psyscomm->procInfo[i].pid);
				SFM_SysCommInfo_H2N (psyscomm);
				break;

			case 2:
 				//bodyLen = sizeof(SFM_SysInfo) - (sizeof(SFM_SysAlmInfo) + sizeof(SFM_SysCommInfo) + sizeof(SFM_SysSuccRateIpInfo));
				//srcP = &sfdb->sys[sysIndex].specInfo;

				memcpy ((void*)txSockMsg.body, (void*)srcP, bodyLen);

				pSys = (SFM_SysSpecInfo *)&txSockMsg.body;
				SFM_SysSpecInfo_H2N (pSys);
				SFM_SysSuccRateIpInfo_H2N ((pSys + sizeof(SFM_SysSpecInfo)));
				break;

			case 3:
				//bodyLen = sizeof(SFM_L3PD)+sizeof(SFM_NMSInfo);
				//srcP = l3pd;

				memcpy((void*)txSockMsg.body, (void*)srcP, sizeof(SFM_L3PD));
				pL3pd   = (SFM_L3PD *)&txSockMsg.body;
				SFM_L3PD_H2N((SFM_L3PD*)pL3pd);


				// in case, NMSIF is not running, exception handling
				for(i = 0; i < SYSCONF_MAX_ASSO_SYS_NUM; i++){
					if(!strcasecmp(sfdb->sys[i].commInfo.type, "OMP"))
						break;
				}
				systype = i;

				for(i = 0; i < sfdb->sys[systype].commInfo.procCnt; i++){
					if(!strcasecmp(sfdb->sys[systype].commInfo.procInfo[i].name, "NMSIF"))
						break;
				}


				nmsinfo = (SFM_NMSInfo*)&txSockMsg.body[sizeof(SFM_L3PD)];

				if(i <  sfdb->sys[systype].commInfo.procCnt && 
					sfdb->sys[systype].commInfo.procInfo[i].status == SFM_STATUS_ALIVE)
				{
//					memcpy(nmsinfo, &(sfdb->nmsInfo), sizeof(SFM_NMSInfo));

					for(i = 0; i < MAX_NMS_CON; i++){ nmsinfo->mask[i]        = (sfdb->nmsInfo.mask[i]);
				    	nmsinfo->level[i]       = (sfdb->nmsInfo.level[i]);
						nmsinfo->fd[i]          = htonl(sfdb->nmsInfo.fd[i]);
						nmsinfo->port[i]        = htonl(sfdb->nmsInfo.port[i]);
						nmsinfo->ipaddr[i]      = htonl(sfdb->nmsInfo.ipaddr[i]);
						nmsinfo->ptype[i]       = htonl(sfdb->nmsInfo.ptype[i]);
						nmsinfo->prev_ptype[i]  = htonl(sfdb->nmsInfo.prev_ptype[i]);
						nmsinfo->rxTime[i]      = htonl(sfdb->nmsInfo.rxTime[i]);
#if 0
//if(sfdb->nmsInfo.port[i]>0)
printf("a[%d], mask: %d, level: %d, fd: %d, port: %d, ip: %d, type : %d, prev_type: %d, time: %d\n", 
		i, sfdb->nmsInfo.mask[i],sfdb->nmsInfo.level[i],sfdb->nmsInfo.fd[i],sfdb->nmsInfo.port[i],
		sfdb->nmsInfo.ipaddr[i],sfdb->nmsInfo.ptype[i],sfdb->nmsInfo.prev_ptype[i],sfdb->nmsInfo.rxTime[i]);
#endif
//printf("a-2. idx: %d, port: %d, level : %d, type : %d\n", i, sfdb->nmsInfo.port[i], sfdb->nmsInfo.level[i], sfdb->nmsInfo.ptype[i]);
					}
				}else{
				//  by helca 11.1 NMSIF의 status가 SFM_STATUS_DEAD일때 mask정보만 유지한다.
				//  NMSIF의 Alarm_level은 SFM_ALM_MINOR뿐
					for(i = 0; i < MAX_NMS_CON; i++){
						sfdb->nmsInfo.level[i] = SFM_ALM_MINOR;
/* when NMSIF updates these fields and simultanously these fields are updated here,
 * values of these fields are totally mess. */
/*						sfdb->nmsInfo.fd[i] = 0;
						sfdb->nmsInfo.port[i] = 0;
						sfdb->nmsInfo.ipaddr[i] = 0;
				    	sfdb->nmsInfo.ptype[i] = 0;
						sfdb->nmsInfo.prev_ptype[i] = 0;
						sfdb->nmsInfo.rxTime[i] = 0;                                                                     
*/	
						
//if(sfdb->nmsInfo.port[i]>0)
//printf("b. idx: %d, port: %d, level : %d, type : %d\n", i, sfdb->nmsInfo.port[i], sfdb->nmsInfo.level[i], sfdb->nmsInfo.ptype[i]);
						nmsinfo->mask[i]        = sfdb->nmsInfo.mask[i];
						nmsinfo->level[i]       = sfdb->nmsInfo.level[i];
						nmsinfo->fd[i]          = htonl(sfdb->nmsInfo.fd[i]);
						nmsinfo->port[i]        = htonl(sfdb->nmsInfo.port[i]);
						nmsinfo->ipaddr[i]      = htonl(sfdb->nmsInfo.ipaddr[i]);
						nmsinfo->ptype[i]       = htonl(sfdb->nmsInfo.ptype[i]);
						nmsinfo->prev_ptype[i]  = htonl(sfdb->nmsInfo.prev_ptype[i]);
						nmsinfo->rxTime[i]      = htonl(sfdb->nmsInfo.rxTime[i]);
						
//printf("b-2. idx: %d, port: %d, level : %d, type : %d\n", i, sfdb->nmsInfo.port[i], sfdb->nmsInfo.level[i], sfdb->nmsInfo.ptype[i]);
					}
				}

				break;

			case 4:
				//bodyLen = sizeof(SFM_SCE);
				//srcP = g_pstSCEInfo;
#if 0 /* by june */
				dumpSockLibHeadType(&txSockMsg.head);
#endif
				memcpy((void*)txSockMsg.body, (void*)srcP, sizeof(SFM_SCE));
				SFM_SCE_H2N((SFM_SCE*)&txSockMsg.body);
				break;
			case 5:				/* L2 Switch information reserved, by june */

				memcpy((void*)txSockMsg.body, (void*)srcP, sizeof(SFM_L2Dev));
				SFM_L2DEV_H2N((SFM_L2Dev*)&txSockMsg.body);

				break;
			case 6:				/* leg cps information reserved, by june */
				memcpy((void*)txSockMsg.body, (void*)srcP, bodyLen);
				/* hjjung_20100823 */
				SFM_LEG_CPS_H2N((SFM_CALL *)&txSockMsg.body);
				break;
			case 7:				/* added by uamyd 20110209, LOGON 성공율 통계 */
			case 8:             /* for LOGOUT SUCCESS RATE, added by uamyd 20110424 */
				/* LOGON, LOGOUT ㅐ� 모양이 동일함 */
				memcpy((void*)txSockMsg.body, (void*)srcP, bodyLen);
				SFM_LOGON_SUCC_RATE_H2N((SFM_LOGON *)&txSockMsg.body); //SCMA
				SFM_LOGON_SUCC_RATE_H2N((SFM_LOGON *)&txSockMsg.body[sizeof(SFM_LOGON)]); //SCMB
				break;
#if 0
			case 9:				/* tps call information reserved, by dcham 20110525 */
				memcpy((void*)txSockMsg.body, (void*)srcP, bodyLen);
				SFM_TPS_CALL_H2N((SFM_CALL *)&txSockMsg.body);
				break;
#endif
		}
		txLen = sizeof(txSockMsg.head) + bodyLen; //yhshin txSockMsg.head.bodyLen;

//		if (socklib_sndMsg (cliTbl[cliIndex].sockFd, (char*)&txSockMsg, txLen) < 0) {

		if (socklib_sndMsg_hdr_chg (cliTbl[cliIndex].sockFd, (char*)&txSockMsg, txLen) < 0) {
			sprintf(trcBuf,"[fimd_sendSfdb2Client ] socklib_sndMsg fail to %s(fd=%d) (infoType:%d, bodyLen: %d)\n"
					"                        Error: %s\n",	
					cliTbl[cliIndex].cliAddr, cliTbl[cliIndex].sockFd, txSockMsg.head.seqNo, bodyLen, //txSockMsg.head.bodyLen,
					strerror(errno));
			trclib_writeLogErr (FL,trcBuf);
			// client table에서 삭제한다.
			memset ((void*)&cliTbl[cliIndex], 0, sizeof(FimdClientContext));
			return -1;
		}
	} while(segFlag != 0);
	
	return 1;
}




//------------------------------------------------------------------------------
// 장애 발생 또는 해지를 감지하여 장애 처리를 모두 수행한 후 호출되어,
// - 접속되어 있는 모든 GUI client로 event port를 통해 이를 통보한다.
//	-> GUI에서는 이를 수신하면 alarm_history DB를 다시 읽어 current alarm list table을
//		update한다.
//------------------------------------------------------------------------------
int fimd_broadcastAlmEvent2Client (void)
{
	int		i;
	for (i=0; i<SOCKLIB_MAX_CLIENT_CNT; i++) {
		if (cliTbl[i].bindPortNum != eventPortNum)
			continue;
		fimd_sendAlmEvent2Client (i);
	}

	return 1;

} //----- End of fimd_broadcastAlmEvent2Client -----//

//------------------------------------------------------------------------------
// 장애 발생 또는 해지를 감지하여 장애 처리를 모두 수행한 후 호출되어,
// - 접속되어 있는 모든 GUI client로 event port를 통해 이를 통보한다.
//  -> GUI에서는 이를 수신하면 alarm_history DB를 다시 읽어 current alarm list table을
//      update한다.
//------------------------------------------------------------------------------
int fimd_broadcastStatEvent2Client (char *tStamp)
{
    int     i;

    for (i=0; i<SOCKLIB_MAX_CLIENT_CNT; i++) {
        if (cliTbl[i].bindPortNum != eventPortNum)
            continue;
        fimd_sendStatEvent2Client (i,tStamp);
    }

    return 1;

} //----- End of fimd_broadcastStatEvent2Client -----//

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int fimd_sendStatEvent2Client (int cliIndex, char *tStamp)
{
    int     txLen;
    SockLibMsgType  txSockMsg;

    memset(txSockMsg.body, 0x00, sizeof(txSockMsg.body));
    strcpy (txSockMsg.body, tStamp);

	// yhshin
	txSockMsg.head.mapType = MAPTYPE_NOTIFICATION_ALARM;// all alm
	txSockMsg.head.segFlag = 0;
	txSockMsg.head.seqNo = 1;
	txSockMsg.head.bodyLen = strlen(txSockMsg.body);

    txLen = sizeof(txSockMsg.head) + strlen(txSockMsg.body); //txSockMsg.head.bodyLen;

//    if (socklib_sndMsg (cliTbl[cliIndex].sockFd, (char*)&txSockMsg, txLen) < 0) {
    if (socklib_sndMsg_hdr_chg (cliTbl[cliIndex].sockFd, (char*)&txSockMsg, txLen) < 0) {
        sprintf(trcBuf,"[fimd_sendStatEvent2Client] socklib_sndMsg fail to %s(fd=%d) MsgBody: %s \n"
               		"                            Error: %s\n",  
		cliTbl[cliIndex].cliAddr, cliTbl[cliIndex].sockFd, txSockMsg.body, strerror(errno));
        // client table에서 삭제한다.
        memset ((void*)&cliTbl[cliIndex], 0, sizeof(FimdClientContext));
        return -1;
    }

    return 1;

} //----- End of fimd_sendStatEvent2Client -----//


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// sjjeon - error 이 함수가 지속적으로 보낸다. client로 ....
int fimd_sendAlmEvent2Client (int cliIndex)
{
	int		txLen;
	SockLibMsgType	txSockMsg;

	memset(txSockMsg.body, 0x00, sizeof(txSockMsg.body));
	strcpy (txSockMsg.body, "ALARM_EVENT_NOTIFICATION");
	//strcpy (txSockMsg.body, "SEND ALARM_EVENT_NOTIFICATION");
	txSockMsg.head.mapType = MAPTYPE_NOTIFICATION_ALARM;// all alm
	txSockMsg.head.segFlag = 0;
	txSockMsg.head.seqNo = 1;
	txSockMsg.head.bodyLen = strlen(txSockMsg.body);
	txLen = sizeof(txSockMsg.head) + strlen(txSockMsg.body); //txSockMsg.head.bodyLen;

	if (socklib_sndMsg_hdr_chg (cliTbl[cliIndex].sockFd, (char*)&txSockMsg, txLen) < 0) {
		sprintf(trcBuf,"[fimd_sendAlmEvent2Client] socklib_sndMsg fail to %s(fd=%d) MsgBody: %s\n"
				"                           Error: %s\n",
				cliTbl[cliIndex].cliAddr, cliTbl[cliIndex].sockFd, txSockMsg.body, strerror(errno));
		trclib_writeLogErr (FL,trcBuf);
		// client table에서 삭제한다.
		memset ((void*)&cliTbl[cliIndex], 0, sizeof(FimdClientContext));
		return -1;
	}

	return 1;

} //----- End of fimd_sendAlmEvent2Client -----//



//------------------------------------------------------------------------------
// GUI가 처음 기동되어 각종 객체들을 생성하는 등 initial 절차에 필요한 정보를
//	요청한 경우 호출되어,
// - 각 시스템에 대한 정보를 일정시간 간격을 두고 한번만 전송한다.
// - 시간 간격을 두고 전송해야 하므로 thread를 생성하여 수행한다.
//------------------------------------------------------------------------------
int tmp_thr_arg_sockFd;
int fimd_rxConfigRequest (int sockFd)
{
	pthread_attr_t	thrAttr;
	pthread_t		thrId;
	int				ret;

	tmp_thr_arg_sockFd = sockFd;

	pthread_attr_init (&thrAttr);
	pthread_attr_setscope (&thrAttr, PTHREAD_SCOPE_SYSTEM);
	pthread_attr_setdetachstate (&thrAttr, PTHREAD_CREATE_DETACHED);
	if ((ret = pthread_create (&thrId, &thrAttr, fimd_rxConfReqThread, (void*)&tmp_thr_arg_sockFd)) != 0) {
		sprintf(trcBuf,"[fimd_rxConfigRequest] pthread_create fail(fimd_rxConfReqThread)\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}
	return 1;

} //----- End of fimd_rxConfigRequest -----//

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void *fimd_rxConfReqThread (void *arg)
{
	int		i, sockFd, cliIndex, txLen, *count;
	SockLibMsgType	txSockMsg;

	memcpy ((void*)&sockFd, arg, sizeof(sockFd));

	// 메시지를 보낸놈의 index를 구한다.
	//
	if ((cliIndex = fimd_getCliIndex (sockFd)) < 0) {
		sprintf(trcBuf,"[fimd_rxConfReqThread] not found fd(%d) in cliTbl\n", sockFd);
		trclib_writeLogErr (FL,trcBuf);
		return NULL;
	}

	//fimd_SysActiveAlmEvent2Client();

	// update client state
	//
	cliTbl[cliIndex].state = FIMD_CLIENT_STATE_INITIAL;


	//
	// 처음에 현재 실장되어 있는 시스템의 갯수만을 먼저 보내고,
	// sfdb의 정보를 시스템 단위로 나누어 보낸다.
	//
	
	// 실장된 시스템 갯수를 먼저 보낸다.
	count = (int*)txSockMsg.body;
	*count = htonl(eqSysCnt);
	txSockMsg.head.segFlag = 0;
	txSockMsg.head.seqNo = 1;
	txSockMsg.head.bodyLen = sizeof(int);  // yhshin
	txLen = sizeof(txSockMsg.head) + sizeof(int); //txSockMsg.head.bodyLen;

	// 보내는 순간에 접속이 끊어질 수 있는데,
	// - 끊어지면 main thread에서 cliTbl을 clear하므로 sockFd를 확인하고 보낸다.
	//
	if (!cliTbl[cliIndex].sockFd)
		return NULL;
//	if (socklib_sndMsg (cliTbl[cliIndex].sockFd, (char*)&txSockMsg, txLen) < 0) {
	if (socklib_sndMsg_hdr_chg (cliTbl[cliIndex].sockFd, (char*)&txSockMsg, txLen) < 0) {
		sprintf(trcBuf,"[fimd_rxConfReqThread] socklib_sndMsg fail to %s(fd=%d)\n"
				"                       Error: %s\n",
				cliTbl[cliIndex].cliAddr, cliTbl[cliIndex].sockFd, strerror(errno));
		trclib_writeLogErr (FL,trcBuf);
		memset ((void*)&cliTbl[cliIndex], 0, sizeof(FimdClientContext));
		return NULL;
	} else {
		sprintf(trcBuf,"[fimd_rxConfReqThread] send to client; cliAddr=%s,fd=%d,eqSysCnt=%d\n",
				cliTbl[cliIndex].cliAddr, cliTbl[cliIndex].sockFd, eqSysCnt);
		trclib_writeLogErr (FL,trcBuf);
	}

	// 각 시스템의 정보를 시간 간격을 두고 보낸다.
	for (i=0; i<eqSysCnt; i++) {
		// 보내는 순간에 접속이 끊어질 수 있는데,
		// - 끊어지면 main thread에서 cliTbl을 clear하므로 sockFd를 확인하고 보낸다.
		//
		if (!cliTbl[cliIndex].sockFd)
			return NULL;
		fimd_sendSfdb2Client (i, cliIndex);
		sprintf(trcBuf,"[fimd_rxConfReqThread] send (%s-%s-%s); cliAddr=%s,fd=%d\n",
				sfdb->sys[i].commInfo.type,
				sfdb->sys[i].commInfo.group,
				sfdb->sys[i].commInfo.name,
				cliTbl[cliIndex].cliAddr, cliTbl[cliIndex].sockFd);
		trclib_writeLogErr (FL,trcBuf);
		commlib_microSleep(200000);
	}

	return NULL;

} //----- End of fimd_rxConfReqThread -----//



//------------------------------------------------------------------------------
// GUI에서 initial이 완료되어 INIT_COMPLETE를 보내면 client의 state를 ACTIVE로 변경하여
//	주기적으로 각 시스템에 대한 sfdb 정보를 전송될 수 있도록 한다.
//------------------------------------------------------------------------------
int fimd_rxInitComplete (int sockFd)
{
	int		cliIndex;

	// 메시지를 보낸놈의 index를 구한다.
	//
	if ((cliIndex = fimd_getCliIndex (sockFd)) < 0) {
		sprintf(trcBuf,"[fimd_rxInitComplete] not found fd(%d) in cliTbl\n", sockFd);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}
	sprintf(trcBuf,"[fimd_rxInitComplete] %d.receive INIT_COMPLETE from %s(fd=%d), b:%d\n", cliIndex,
			cliTbl[cliIndex].cliAddr, cliTbl[cliIndex].sockFd, cliTbl[cliIndex].bindPortNum);
	trclib_writeLogErr(FL,trcBuf);

	// update client state
	//
	cliTbl[cliIndex].state = FIMD_CLIENT_STATE_ACTIVE;

	return 1;

} //----- End of fimd_rxInitComplete -----//

int fimd_SmsAlmEvent2Client(int almLevel)
{
	int     i, txLen;
	SockLibMsgType  txSockMsg;

	if(almLevel > 3) {
		sprintf(trcBuf,"[fimd_SmsAlmEvent2Client] Curren DSC Alm = %d\n", almLevel);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	for (i=0; i<SOCKLIB_MAX_CLIENT_CNT; i++) {
		if (cliTbl[i].bindPortNum != eventPortNum)
			continue;

		memset(txSockMsg.body, 0x00, sizeof(txSockMsg.body));
		sprintf(txSockMsg.body, "%d", almLevel);

		//yhshin
		txSockMsg.head.mapType = MAPTYPE_SMS_ALARM;
		txSockMsg.head.segFlag = 0;
		txSockMsg.head.seqNo = 1;
		txSockMsg.head.bodyLen = strlen(txSockMsg.body);
		txLen = sizeof(txSockMsg.head)+strlen(txSockMsg.body); // txSockMsg.head.bodyLen;

//		if (socklib_sndMsg (cliTbl[i].sockFd, (char*)&txSockMsg, txLen) < 0) {
		if (socklib_sndMsg_hdr_chg (cliTbl[i].sockFd, (char*)&txSockMsg, txLen) < 0) {
			sprintf(trcBuf,"[fimd_SmsAlmEvent2Client] socklib_sndMsg fail to %s(fd=%d)\n"
					"                          Error: %s\n", 
				cliTbl[i].cliAddr, cliTbl[i].sockFd, strerror(errno));
			trclib_writeLogErr (FL,trcBuf);
			memset ((void*)&cliTbl[i], 0, sizeof(FimdClientContext));
			return -1;
		}
	}
	return 1;
}

//------------------------------------------------------------------------------
//		active side report
//------------------------------------------------------------------------------
int fimd_SysActiveAlmEvent2Client (void)
{
    int     i, txLen;
	SockLibMsgType  txSockMsg;


	for (i=0; i<SOCKLIB_MAX_CLIENT_CNT; i++) {
		if (cliTbl[i].bindPortNum != eventPortNum)
			continue;

		memset(txSockMsg.body, 0x00, sizeof(txSockMsg.body));
		strcpy (txSockMsg.body, sfdb->active_sys_name);

		//yhshin
		txSockMsg.head.mapType = (MAPTYPE_ACTIVE_ALARM);
		txSockMsg.head.segFlag = 0;
		txSockMsg.head.seqNo = 1;
		txSockMsg.head.bodyLen = (strlen(txSockMsg.body));
		txLen = sizeof(txSockMsg.head) + strlen(txSockMsg.body); //txSockMsg.head.bodyLen;
//		if (socklib_sndMsg (cliTbl[i].sockFd, (char*)&txSockMsg, txLen) < 0) {
		if (socklib_sndMsg_hdr_chg (cliTbl[i].sockFd, (char*)&txSockMsg, txLen) < 0) {
			sprintf(trcBuf,"[fimd_SysActiveAlmEvent2Client] socklib_sndMsg fail to %s(fd=%d)\n"
					"                                Error: %s\n", 
				cliTbl[i].cliAddr, cliTbl[i].sockFd, strerror(errno));
			trclib_writeLogErr (FL,trcBuf);
			// client table에서 삭제한다.
			memset ((void*)&cliTbl[i], 0, sizeof(FimdClientContext));
			return -1;
		}
	}

	return 1;
}

//------------------------------------------------------------------------------
//		stop audio alm일 경우 
//------------------------------------------------------------------------------
int fimd_AudioAlmEvent2Client (void)
{
	int		i, txLen;
	SockLibMsgType	txSockMsg;


	for (i=0; i<SOCKLIB_MAX_CLIENT_CNT; i++) {
		if (cliTbl[i].bindPortNum != eventPortNum)
			continue;

		//yhshin
		memset(txSockMsg.body, 0x00, sizeof(txSockMsg.body));
		strcpy (txSockMsg.body, "ALARM_EVENT_NOTIFICATION");
		//strcpy (txSockMsg.body, "AUDIO ALARM_EVENT_NOTIFICATION");
		txSockMsg.head.mapType = (MAPTYPE_AUDIO_ALARM);// audio stop(1), event alarm(0)
		txSockMsg.head.segFlag = 0;
		txSockMsg.head.seqNo = 1;
		txSockMsg.head.bodyLen = (strlen(txSockMsg.body));
		txLen = sizeof(txSockMsg.head) + strlen(txSockMsg.body); //txSockMsg.head.bodyLen;

		sprintf(trcBuf,"[fimd_AudioAlmEvent2Client] txSockMsg.head.mapType : %d, txLen : %d\n", txSockMsg.head.mapType, txLen);
		trclib_writeLogErr (FL,trcBuf);


//		if (socklib_sndMsg (cliTbl[i].sockFd, (char*)&txSockMsg, txLen) < 0) {
		if (socklib_sndMsg_hdr_chg (cliTbl[i].sockFd, (char*)&txSockMsg, txLen) < 0) {
			sprintf(trcBuf,"[fimd_AudioAlmEvent2Client] socklib_sndMsg fail to %s(fd=%d)\n"
					"                            Error: %s\n", 
					cliTbl[i].cliAddr, cliTbl[i].sockFd, strerror(errno));
			trclib_writeLogErr (FL,trcBuf);
			// client table에서 삭제한다.
			memset ((void*)&cliTbl[i], 0, sizeof(FimdClientContext));
			return -1;
		}


	}

	//fprintf(stderr,"fimd_AudioAlmEvent2Client.....\n");
	return 1;

} //----- End of fimd_AudioAlmEvent2Client -----//
#if 1
int fimd_DuplicationStsEvent2Client (void)
{
	int		i, txLen;
	SockLibMsgType	txSockMsg;


	for (i=0; i<SOCKLIB_MAX_CLIENT_CNT; i++) {
		if (cliTbl[i].bindPortNum != eventPortNum)
			continue;

		//yhshin
		memset(txSockMsg.body, 0x00, sizeof(txSockMsg.body));
		strcpy (txSockMsg.body, "ALARM_EVENT_NOTIFICATION");
		//strcpy (txSockMsg.body, "DUPLICATION ALARM_EVENT_NOTIFICATION");
		txSockMsg.head.mapType = (MAPTYPE_DUPLICATION_STS);
		txSockMsg.head.segFlag = 0;
		txSockMsg.head.seqNo = 1;
		txSockMsg.head.bodyLen = (strlen(txSockMsg.body));
		txLen = sizeof(txSockMsg.head) + strlen(txSockMsg.body); //txSockMsg.head.bodyLen;

//		if (socklib_sndMsg (cliTbl[i].sockFd, (char*)&txSockMsg, txLen) < 0) {
		if (socklib_sndMsg_hdr_chg (cliTbl[i].sockFd, (char*)&txSockMsg, txLen) < 0) {
			sprintf(trcBuf,"[fimd_DuplicationStsEvent2Client] socklib_sndMsg fail to %s(fd=%d)\n"
					"                                  Error: %s\n", 
					cliTbl[i].cliAddr, cliTbl[i].sockFd, strerror(errno));
			trclib_writeLogErr (FL,trcBuf);
			memset ((void*)&cliTbl[i], 0, sizeof(FimdClientContext));
			return -1;
		}


	}

	return 1;

} //----- End of fimd_DuplicationStsEvent2Client -----//

#endif


//------------------------------------------------------------------------------
//      stop audio alm일 경우 
//------------------------------------------------------------------------------
int fimd_DactAlmEvent2Client (void)
{
    int     i, txLen;
    SockLibMsgType  txSockMsg;


    for (i=0; i<SOCKLIB_MAX_CLIENT_CNT; i++) {
        if (cliTbl[i].bindPortNum != eventPortNum)
            continue;

		// yhshin
        memset(txSockMsg.body, 0x00, sizeof(txSockMsg.body));
        strcpy (txSockMsg.body, "DACT PPD");
        txSockMsg.head.mapType = (MAPTYPE_AUDIO_DACT);
        txSockMsg.head.segFlag = 0;
        txSockMsg.head.seqNo = 1;
        txSockMsg.head.bodyLen = (strlen(txSockMsg.body));
        txLen = sizeof(txSockMsg.head) + strlen(txSockMsg.body); //txSockMsg.head.bodyLen;

//        if (socklib_sndMsg (cliTbl[i].sockFd, (char*)&txSockMsg, txLen) < 0) {
        if (socklib_sndMsg_hdr_chg (cliTbl[i].sockFd, (char*)&txSockMsg, txLen) < 0) {
            sprintf(trcBuf,"[fimd_DactAlmEvent2Client] socklib_sndMsg fail to %s(fd=%d)\n"
			    "                           Error: %s\n", 
                    cliTbl[i].cliAddr, cliTbl[i].sockFd, strerror(errno));
            trclib_writeLogErr (FL,trcBuf);
            // client table에서 삭제한다.
            memset ((void*)&cliTbl[i], 0, sizeof(FimdClientContext));
            return -1;
        }


    }

    //fprintf(stderr,"fimd_AudioAlmEvent2Client.....\n");
    return 1;

} //----- End of fimd_AudioAlmEvent2Client -----//

//------------------------------------------------------------------------------
//      stop audio alm일 경우 
//------------------------------------------------------------------------------
int fimd_ActAlmEvent2Client (void)
{
    int     i, txLen;
    SockLibMsgType  txSockMsg;


    for (i=0; i<SOCKLIB_MAX_CLIENT_CNT; i++) {
        if (cliTbl[i].bindPortNum != eventPortNum)
            continue;

		// yhshin
        memset(txSockMsg.body, 0x00, sizeof(txSockMsg.body));
        strcpy (txSockMsg.body, "ACT PPD");
        txSockMsg.head.mapType = (MAPTYPE_AUDIO_ACT);
        txSockMsg.head.segFlag = 0;
        txSockMsg.head.seqNo = 1;
        txSockMsg.head.bodyLen = (strlen(txSockMsg.body));
        txLen = sizeof(txSockMsg.head) + strlen(txSockMsg.body); //txSockMsg.head.bodyLen;

 //       if (socklib_sndMsg (cliTbl[i].sockFd, (char*)&txSockMsg, txLen) < 0) {
        if (socklib_sndMsg_hdr_chg (cliTbl[i].sockFd, (char*)&txSockMsg, txLen) < 0) {
            sprintf(trcBuf,"[fimd_ActAlmEvent2Client] socklib_sndMsg fail to %s(fd=%d)\n"
			    "                          Error: %s\n",
                    cliTbl[i].cliAddr, cliTbl[i].sockFd, strerror(errno));
            trclib_writeLogErr (FL,trcBuf);
            // client table에서 삭제한다.
            memset ((void*)&cliTbl[i], 0, sizeof(FimdClientContext));
            return -1;
        }


    }

    //fprintf(stderr,"fimd_AudioAlmEvent2Client.....\n");
    return 1;

} //----- End of fimd_AudioAlmEvent2Client -----//

int fimd_almProcMP(int sysIndex)
{
	int i,j;
	int changeFlag;
	int ompIndex;
	char findName[256];



	strcpy(findName,sfdb->sys[sysIndex].commInfo.name);

	ompIndex = fimd_getSysIndexByName (SYSCONF_SYSTYPE_OMP);

	if(ompIndex != sysIndex) {
	    for (i=0; i<SFM_MAX_LAN_CNT; i++) {
           if (!strncasecmp (sfdb->sys[ompIndex].commInfo.lanInfo[i].name, findName, strlen(findName) )) {
               if(sfdb->sys[ompIndex].commInfo.lanInfo[i].status == SFM_LAN_CONNECTED) {
                   break;
               }
           }
    	}

    	if ( i>=SFM_MAX_LAN_CNT ){
			
			sprintf (trcBuf, "fimd_almProcMP network connection fail; err=%d:%s\n", errno, strerror(errno)); 
			trclib_writeLogErr (FL,trcBuf);
			sprintf (trcBuf, "fimd terminate.\n"); trclib_writeLogErr (FL, trcBuf);
        	exit(0); // network fail
    	}
	}
	
	for (i=0; i<sfdb->sys[sysIndex].commInfo.procCnt; i++) {
		SFM_ProcInfo *procInfo=&sfdb->sys[sysIndex].commInfo.procInfo[i];
		if (procInfo->mask == SFM_ALM_MASKED) // mask된 놈은 상태관리에서 제외한다.
			continue;
		for (j=0; j<procSysInfo[sysIndex].appCnt; j++){
			if ( !strcmp( procSysInfo[sysIndex].procInfo[j].name, "") ) continue;
			if ( !strcasecmp ( procSysInfo[sysIndex].procInfo[j].name, procInfo->name ) ) break;
		}
		if ( j>SFM_MAX_PROC_CNT ) continue; 
		
		//strcpy (procInfo->name, procSysInfo[sysIndex].procInfo[j].name);
		//왜?? 아래처럼 처리 했을까?? 주석처리..sjjeon
		//strcpy(procInfo->name, procInfo->name);
		procInfo->level 		= procInfo->level;
		procInfo->prevStatus 	= procInfo->status;
		procInfo->status		= procSysInfo[sysIndex].procInfo[j].status;

		// 이전상태와 다르면 장애 처리
		if (procInfo->prevStatus != procInfo->status) {

			fimd_hdlProcAlm (sysIndex, i, 0);
			changeFlag = 1;
		}
	}

	if(sysIndex == fimd_getSysIndexByName (SYSCONF_SYSTYPE_OMP)) {
		if (changeFlag) {
			fimd_updateSysAlmInfo (sysIndex);
			fimd_broadcastAlmEvent2Client ();
		}

		return 1;
	}

	if (changeFlag) {
		fimd_updateSysAlmInfo (sysIndex);
		fimd_broadcastAlmEvent2Client ();
	}

	return 1;
}


int fimd_getProcStatOMP (int sysIndex )
{
	char	command[256], fname[256], lineBuf[1024];
	char	token[8][32];
	FILE	*fp;
	int		ret, appCnt=0;

	sprintf (fname, "/tmp/disprc_omp");
	unlink ( fname );
	sprintf (command, "disprc 2> %s", fname);
	system (command);


	if ((fp = fopen (fname, "r")) == NULL) {
		sprintf (trcBuf, "[disprc] fopen fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno)); 
		trclib_writeLogErr (FL,trcBuf);
		return 0;
	}

	while (fgets (lineBuf, sizeof(lineBuf), fp) != NULL) {

		if ( lineBuf[0]=='=' || lineBuf[0]=='-' || lineBuf[0]=='#' || lineBuf[0]=='\n' ) /* comment line or empty */
			continue;

		memset ( token, 0, sizeof(token) );
		ret = sscanf (lineBuf, "%s %s %s %s %s %s %s %s", token[0], token[1], token[2], token[3],
											 		token[4], token[5], token[6], token[7]);

		if ( !strcasecmp ( token[0],"Process" ) || !strncmp ( token[0],"TOTAL",5 ))
			continue;

		if(ret > 7) {
			strcpy(procSysInfo[sysIndex].procInfo[appCnt].name, token[0]);
			procSysInfo[sysIndex].procInfo[appCnt++].status = procGetState(token[2]);
		
			if( token[3][0] == '-' ) {
					strcpy(procSysInfo[sysIndex].procInfo[appCnt].name, token[4]);
					procSysInfo[sysIndex].procInfo[appCnt++].status = procGetState(token[6]);
			} else {	
					strcpy(procSysInfo[sysIndex].procInfo[appCnt].name, token[5]);
					procSysInfo[sysIndex].procInfo[appCnt++].status = procGetState(token[7]);
			}
		} 
	}

	procSysInfo[sysIndex].appCnt = appCnt;

	if(fp)fclose(fp);

	return 1;
}


int fimd_getProcStatMP (int sysIndex )
{
	char	target[32], command[256], fname[256], lineBuf[1024], findName[256];
	char	token[8][32], targetIp[32];
	FILE	*fp;
	int		i, ret,  appCnt=0,ompIndex;

	strcpy(findName,sfdb->sys[sysIndex].commInfo.name);

	ompIndex = fimd_getSysIndexByName (SYSCONF_SYSTYPE_OMP);

	for (i=0; i<SFM_MAX_LAN_CNT; i++) {
		if (!strncasecmp (sfdb->sys[ompIndex].commInfo.lanInfo[i].name, findName, strlen(findName) )) {
		    if(sfdb->sys[ompIndex].commInfo.lanInfo[i].status == SFM_LAN_CONNECTED)	{
				break;
			}
		}
	}

	if ( i>=SFM_MAX_LAN_CNT ){
		sprintf (trcBuf, " %s network is disconnected\n", findName );
		trclib_writeLogErr (FL,trcBuf);
		return -1; // network fail
	}

	strcpy (target, sfdb->sys[ompIndex].commInfo.lanInfo[i].name);
	strcpy (targetIp, sfdb->sys[ompIndex].commInfo.lanInfo[i].targetIp);
	sprintf (fname, "/tmp/disprc_%s", target);
	unlink ( fname );
	// target을 호스트명으로 찾음.
	sprintf (command, "rsh %s -l root disprc 2> %s", targetIp, fname);
	alarm(1);
	system (command);
	alarm(0);

	if ((fp = fopen (fname, "r")) == NULL) {
		sprintf (trcBuf, "[disprc] fopen fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno)); 
		trclib_writeLogErr (FL,trcBuf);
		return 0;
	}

	while (fgets (lineBuf, sizeof(lineBuf), fp) != NULL) {

		if ( lineBuf[0]=='=' || lineBuf[0]=='-' || lineBuf[0]=='#' || lineBuf[0]=='\n' ) /* comment line or empty */
			continue;

		memset ( token, 0, sizeof(token) );
		ret = sscanf (lineBuf, "%s %s %s %s %s %s %s %s", token[0], token[1], token[2], token[3],
											 		token[4], token[5], token[6], token[7]);

		if ( !strcasecmp ( token[0],"Process" ) || !strncmp ( token[0],"TOTAL",5 ))
			continue;

		if(ret > 7) {
			strcpy(procSysInfo[sysIndex].procInfo[appCnt].name, token[0]);
			procSysInfo[sysIndex].procInfo[appCnt++].status = procGetState(token[2]);
		
			if( token[3][0] == '-' ) {
					strcpy(procSysInfo[sysIndex].procInfo[appCnt].name, token[4]);
					procSysInfo[sysIndex].procInfo[appCnt++].status = procGetState(token[6]);
			} else {	
					strcpy(procSysInfo[sysIndex].procInfo[appCnt].name, token[5]);
					procSysInfo[sysIndex].procInfo[appCnt++].status = procGetState(token[7]);
			}
		} else  if ( (token[1][0] == '-') && strcasecmp(token[1],"-")) {
				// altibase는 altibase -a로 전송됨으로 확인해서 -a는 삭제한다.
				if ( !strcasecmp ( token[0], "altibase" ) ){
					sprintf(procSysInfo[sysIndex].procInfo[appCnt].name,"%s", token[0] );
				} else {
					sprintf(procSysInfo[sysIndex].procInfo[appCnt].name,"%s %s", token[0], token[1]);
				}
				procSysInfo[sysIndex].procInfo[appCnt++].status = procGetState(token[3]);
		}
		else {
				sprintf(procSysInfo[sysIndex].procInfo[appCnt].name,"%s", token[0]);
				procSysInfo[sysIndex].procInfo[appCnt++].status = procGetState(token[2]);
		}
	}
	procSysInfo[sysIndex].appCnt = appCnt;

	if(fp)fclose(fp);

	if(appCnt == 0) {
		logPrint(trcErrLogId, FL, "sysIndex=[%d] is unreachable!\n", sysIndex);
		return -1;
	}

	return 1;
}

int procGetState(char *status)
{
	if(!strncasecmp(status,"ALIVE",5))
       return SFM_STATUS_ALIVE;
	else
       return SFM_STATUS_DEAD;
}

int fimd_smsDisPrc(int sysIdx)
{
	int i,j;

	if(sysIdx == 0) return 1;

	logPrint(trcLogId, FL,"---------------(%s-%d)---------------\n",sfdb->sys[sysIdx].commInfo.name,procSysInfo[sysIdx].appCnt);
	for(i=0;i<procSysInfo[sysIdx].appCnt;j++){
		logPrint(trcLogId,FL,"[%d]%s , %d\n",i,
				 procSysInfo[sysIdx].procInfo[i].name, procSysInfo[sysIdx].procInfo[i].status);
	}

	return 1;
}

int fimd_checkProcState(int sysIndex, char *procName, int procIndex)
{
	int i, result;

	result = SFM_STATUS_DEAD;

	if(procSysInfo[sysIndex].appCnt == 0) {
//		logPrint(trcErrLogId, FL, "sysIndex=[%d] is unreachable!\n", sysIndex);
		result = sfdb->sys[sysIndex].commInfo.procInfo[procIndex].status;
	}

	for(i=0;i<procSysInfo[sysIndex].appCnt;i++) {

		if(!strcasecmp(procName,procSysInfo[sysIndex].procInfo[i].name)) {
			result = procSysInfo[sysIndex].procInfo[i].status;
			logPrint(trcErrLogId,FL,"checkState na=%s, st=%d\n",
					procSysInfo[sysIndex].procInfo[i].name, result);
			break;
		}
	}

	return result;
}

int fimd_smsOpenCallMon(int sysIndex, int count)
{
	int pid, ret=0;
	int loop = 0, i, ompIndex;

	memset(procSysInfo,0,sizeof(procSysInfo));
    ompIndex = fimd_getSysIndexByName (SYSCONF_SYSTYPE_OMP);

	if ((pid = fork()) != 0)
		return 0;

	while(1)
	{
		if(sysIndex == ompIndex) {
			ret = fimd_getProcStatOMP ( sysIndex);
		} else {
			ret = fimd_getProcStatMP ( sysIndex);
		}
		loop++;

		if((loop >= count) || ret )
		  break;

	}

	if(sysIndex == ompIndex) { //omp keepalive
		if(ret) fimd_almProcMP(sysIndex);
		logPrint(trcErrLogId, FL, "[%d]alarm Process Check by rsh command \n", sysIndex);
		exit(0);
	}

    if(ret==-1){ // network fail
        for (i=0; i < sfdb->sys[sysIndex].commInfo.procCnt; i++) {
            // mask된 놈은 상태관리에서 제외한다.
            if (sfdb->sys[sysIndex].commInfo.procInfo[i].mask   == SFM_ALM_MASKED) continue;
            sfdb->sys[sysIndex].commInfo.procInfo[i].prevStatus = sfdb->sys[sysIndex].commInfo.procInfo[i].status;
            sfdb->sys[sysIndex].commInfo.procInfo[i].status     = SFM_STATUS_DEAD;
            if (sfdb->sys[sysIndex].commInfo.procInfo[i].prevStatus != sfdb->sys[sysIndex].commInfo.procInfo[i].status) {
                fimd_hdlProcAlm(sysIndex, i, 1);
                fimd_updateSysAlmInfo (sysIndex);
                fimd_broadcastAlmEvent2Client ();
            }
        }

        for (i=0; i<SFM_MAX_LAN_CNT; i++) {
            if (!strncasecmp (sfdb->sys[ompIndex].commInfo.lanInfo[i].name, SYSCONF_SYSTYPE_BSD, 3)) {
                if(sfdb->sys[ompIndex].commInfo.lanInfo[i].status == SFM_LAN_CONNECTED) {
                    break;
                }
            }
        }

        if ( i>=SFM_MAX_LAN_CNT ){
			logPrint(trcErrLogId, FL, "All Networks are disconnected with MPs\n");
        }

        exit(0);
    }

	if(ret==1) {
		logPrint(trcErrLogId, FL, "[%d]alarm Process Check by rsh command \n", sysIndex);
		fimd_almProcMP( sysIndex );
	} 

	exit(0);	
}

/*
 * 20040921-mnpark
 * DB Replication이 지속적으로 실패하면
 * MP의 ATIF,WISEIB process를 remote로 kill시킨다. 
 * 가능한 모든 조건이 만족한 상태에서만 killprc하도록 한다. 
 * check 조건 
 * 	- SAMD로부터 상태 Message를 정상적으로 받아야 한다. 
 *	- REPLICATION은 연속적으로 정해진 횟수 이상으로 실패해야한다. 
 *	- remote로 MP Process 상태 확인 성공해야 한다. 
 *  - 양 SIDE MP Process가 모두 ALIVE해야 한다.  
 */
// sjjeon : 사용하지 않는 함수 인것 같다...
int	fimd_doKillprc(int almType, int sysIndex, char *procName)
{
	char	targetIp[SFM_MAX_TARGET_IP_LEN], command[256], reason[128], tmpFile[128];
	char	*destSysName;
	int		ompIndex, altibaseIdx, procIdx, connectedLanIdx;
	pid_t	pid;

	//#define	TMP_FILE	"/tmp/killprcFromFimd"

    /* 20041122 */
    if ((pid = fork()) != 0){
        return 0;
	}
/*debug*/
//fprintf(stderr,"sysIndex(%d), procName(%s)\n", sysIndex, procName);

	memset(tmpFile, 0x00, sizeof(tmpFile));
	sprintf(tmpFile, "/tmp/%sKillFromFimd", procName);

	destSysName 	= sfdb->sys[sysIndex].commInfo.name;
	ompIndex 		= fimd_getSysIndexByName (SYSCONF_SYSTYPE_OMP);

	if( (altibaseIdx = fimd_getProcIndexByName(sysIndex, PROC_NAME_DB)) < 0 ) {
		sprintf(trcBuf,"[fimd_doKillprc] proc  not found sysName[%s] procName[%s]\n", destSysName, PROC_NAME_DB);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	if( (procIdx = fimd_getProcIndexByName(sysIndex, procName)) < 0 ) {
		sprintf(trcBuf,"[fimd_doKillprc] proc  not found sysName[%s] procName[%s]\n", destSysName, procName);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	if( (connectedLanIdx = fimd_getConnectedLanIdx(sysIndex)) < 0 ) {
		sprintf (trcBuf, "[fimd_doKillPrc]fimd_doKillprc break; reason=%s disconnected all\n", destSysName );
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	memset(targetIp, 0x00, sizeof(targetIp));
	memset(command, 0x00, sizeof(command));

	strcpy(targetIp, sfdb->sys[ompIndex].commInfo.lanInfo[connectedLanIdx].targetIp);
	unlink(tmpFile);
	sprintf (command, "rsh %s -l root killprc -b %s 2> %s", targetIp, procName, tmpFile);
	alarm(1);

	memset(reason, 0x00, sizeof(reason));
	switch(almType)
	{
		case ALMCODE_SFM_PROCESS:
			sprintf(reason, "DB is DEAD");
			break;
		case ALMCODE_SFM_DB_REP:
			sprintf(reason, "DB Replication Fail");
			break;
	}

	sprintf	(trcBuf, " ");
	sprintf (trcBuf, "[fimd_doKillPrc]try killprc destSysName=%s, procName=%s, reason=%s\n", destSysName, procName, reason);
	logPrint(trcLogId,FL,trcBuf);

	if(!system (command)) {
		sprintf (trcBuf, "[fimd_doKillPrc]!SUCCESS(%s)\n", command);
		logPrint(trcLogId,FL,trcBuf);

		/* send alm msg */
		fimd_makeKillprcAlmMsg(almType, sysIndex, procIdx, altibaseIdx);
	}
	else {
		sprintf (trcBuf, "[fimd_doKillPrc]!FAIL(%s)\n", command);
		logPrint(trcLogId,FL,trcBuf);
	}
	alarm(0);
	exit(0);	/* 20041122 */
}

/*
 * 바로 Return Message를 받기 위해서 
 */
int fimd_checkMPProc(int sysIndex, int count)
{
	int ret=0;
	int loop = 0;

	memset(procSysInfo,0,sizeof(procSysInfo));

	while(1)
	{
		ret = fimd_getProcStatMP ( sysIndex);
		loop++;

		if((loop >= count) || ret )
		  break;

	}

	if(ret) {
		fimd_almProcMP( sysIndex );
	} 

	return ret;
}


/*
 * @return 	STATUS_REPORT의 KEEPALIVE_RETRY Count
 */
int fimd_getKeepAliveStsRty(int	sysIndex)
{
	int	i;

	for (i=0; i<MAX_KEEPALIVE; i++) {
		if (!fimdKeepAlive[i].category)	continue;

		if(fimdKeepAlive[i].category == MSGID_SYS_COMM_STATUS_REPORT
			&& fimdKeepAlive[i].sysIdx == sysIndex)
			return fimdKeepAlive[i].retry;
	}		
	return -1;
}

/*
 * ToDo
 * @return	MP SysIndex of Standby 
 */
int fimd_getStandbySysIndex()
{
	int	sysIndex, ompIndex;
	
	ompIndex = fimd_getSysIndexByName(SYSCONF_SYSTYPE_OMP);
	for(sysIndex=0; sysIndex<eqSysCnt; sysIndex++) {
		if(sysIndex == ompIndex) continue;
		if(!strcasecmp(sfdb->sys[sysIndex].commInfo.name , sfdb->active_sys_name))
			break;
	}
	if(sysIndex == eqSysCnt)	return -1;

	return sysIndex;
}

int fimd_getMPProcAllAlive(char *procName)
{
	SFM_ProcInfo 	*procInfo	= NULL;
	int				sysIndex, ompIndex, procIndex, procAliveCnt;

	ompIndex = fimd_getSysIndexByName(SYSCONF_SYSTYPE_OMP);

	procAliveCnt = 0;
	for(sysIndex=0; sysIndex<eqSysCnt; sysIndex++) {
		if(sysIndex == ompIndex) continue;

		procIndex 	= fimd_getProcIndexByName(sysIndex, procName);
		procInfo	= &sfdb->sys[sysIndex].commInfo.procInfo[procIndex];
		if(procInfo->status == SFM_STATUS_ALIVE) 
			procAliveCnt++;
	}

	if(procAliveCnt == eqSysCnt-1)
		return 1;
	else
		return 0;
}

/*
 * 20041129.mnpark
 * Process 상태를 return한다. 
 */
int fimd_getProcStatus(int sysIndex, char *procName) 
{
	SFM_ProcInfo 	*procInfo	= NULL;
	int				procIndex;

	procIndex 	= fimd_getProcIndexByName(sysIndex, procName);
	if(procIndex < 0) 	
		return -1;

	procInfo	= &sfdb->sys[sysIndex].commInfo.procInfo[procIndex];
	return procInfo->status;
}

/*
 * 20041130.mnpark
 * OMP와 target System과 연결된 LAN이 있는지 check한다. 
 * @return	0 : 연결된 LAN이 없음
 *			1 : 연결된 LAN이 있음 
 */
int fimd_getConnectedLanIdx(int targetSysIndex)
{
	char	*destSysName;
	int		i, ompIndex;	

	destSysName 	= sfdb->sys[targetSysIndex].commInfo.name;

	ompIndex 		= fimd_getSysIndexByName (SYSCONF_SYSTYPE_OMP);

	for (i=0; i<SFM_MAX_LAN_CNT; i++) {
		if (!strncasecmp (sfdb->sys[ompIndex].commInfo.lanInfo[i].name, destSysName, strlen(destSysName) )) {
		    if(sfdb->sys[ompIndex].commInfo.lanInfo[i].status == SFM_LAN_CONNECTED)	{
				return i;
			}
		}
	}
	if ( i>=SFM_MAX_LAN_CNT ){
		sprintf (trcBuf, "[fime_getConnectedLanIdx]ERR %s disconnected all\n", destSysName );
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	return -1;
}

int fimd_txMsg2Nmsif (char *system, int type, int class, char *info, int time)
{
	int		txLen;
	GeneralQMsgType	txGenQMsg;
	NmsAlmInfo	*alminfo;

	memset (&txGenQMsg, 0, sizeof (GeneralQMsgType));

	alminfo = (NmsAlmInfo *)txGenQMsg.body;

	txGenQMsg.mtype = MTYPE_ALARM_REPORT; // cond에서 mtype에 따라 장애메시지, 상태메시지를 다른 곳에 logging한다.

	strcpy (alminfo->sysname, system);
	alminfo->atype	= type;
	alminfo->aclass	= class;
	strcpy (alminfo->desc, info);
	alminfo->time	= time;
	
	txLen = sizeof(NmsAlmInfo) + 8;
#ifdef DEBUG
	sprintf(trcBuf,"[fimd_txMsg2Nmsif] txGenQMsg.mtype %ld, system=%s type=%d class=%d time=%d message=%s\n",
			txGenQMsg.mtype,system, type, class, time, info);
	trclib_writeLogErr(FL,trcBuf);
#endif

	if (msgsnd(nmsifQid, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0) {
		sprintf(trcBuf,"[fimd_txMsg2nmsif] msgsnd fail to nmsif; err=%d(%s)\n",
				errno, strerror(errno));
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	} else {
		if (trcFlag || trcLogFlag) {
			sprintf(trcBuf,"[fimd_txMsg2nmsif] send to nmsif\n");
			trclib_writeLog (FL,trcBuf);
		}
	}
	return 1;

} //----- End of fimd_txMsg2nmsif -----//


#define SUCCRATE_IP_NUM		3
#define SUCC_RATE_ANAAA_ITR	6
int fimd_SuccessRate_AAA_UAWAP(int sysIndex)
{
	return 1;
}

int  evaluate_successRate(int sysIndex, SFM_SysSuccRate *succRate, SuccRateIpInfo *orgRate,
						 SuccRateIpInfo *newRate, int entrysize)
{
	int	i, j, found;
	int	changeFlag = 0;
	char	almLevel = 0;

	if(succRate->mask == SFM_ALM_MASKED){
		return 0;
	}

	// ACTIVE side일 경우 cnt, rate가 해당 조건에 만족할때. 
	// cnt가 0인경우 조건을 제외한다..by helca 7002.07.04	
	if(!strcasecmp(sfdb->sys[sysIndex].commInfo.name, sfdb->active_sys_name)) {	

		for( i = 0;(i < entrysize) && newRate[i].ipAddr; i++){
			if(((newRate[i].count > succRate->cnt) &&
						(newRate[i].rate < succRate->rate)) ){
				newRate[i].level = SFM_ALM_CRITICAL;
			}else{
				newRate[i].level = SFM_ALM_NORMAL;
			}
		}	
	}
	
	// Original Success Rate => New Success Rate
	for(i = 0;(i < entrysize) && orgRate[i].ipAddr; i++){
		found = 0;
		for(j = 0;(j < entrysize) && newRate[j].ipAddr; j++){
			if(orgRate[i].ipAddr == newRate[j].ipAddr){
				found = 1;
				break;
			}
		}

		if(found){
			// current level is different from previous one.
			// alarm occured or cleared.
#if 0
{
struct in_addr addr;
addr.s_addr = newRate[j].ipAddr;
fprintf(stderr, "FOUND[%d] %s -LEVEL %d:%d\n",i, inet_ntoa(addr),
orgRate[i].level, newRate[j].level);
}
#endif
			if(orgRate[i].level != newRate[j].level){
				changeFlag = 1;
				if(newRate[j].level == SFM_ALM_CRITICAL){
					fimd_hdlSuccRateIpAlm(sysIndex, newRate[i], succRate, 1);
				}else{
					fimd_hdlSuccRateIpAlm(sysIndex, newRate[i], succRate, 0);
				}
			}
		}else{  // IP is not used any more.
#if 0
{
struct in_addr addr;
addr.s_addr = newRate[j].ipAddr;
fprintf(stderr, "NOT FOUND[%d] %s -LEVEL %d\n",i, inet_ntoa(addr),
orgRate[i].level);
}
#endif
			// When alarm level is CRITICAL, clear the alarm.
			if(orgRate[i].level == SFM_ALM_CRITICAL){
				changeFlag = 1;
				fimd_hdlSuccRateIpAlm(sysIndex, orgRate[i], succRate, 0);
			}
		}
	}

	// New Success Rate => Original Success Rate
	for(i = 0;(i < entrysize) && newRate[i].ipAddr; i++){
		found = 0;
		for(j = 0;(j < entrysize) && orgRate[j].ipAddr; j++){
			if(newRate[i].ipAddr == orgRate[j].ipAddr){
				found = 1;
				break;
			}
		}

		if(!found){  // New IP Address
			// When alarm level is CRITICAL, occur the alarm.
			if(newRate[i].level == SFM_ALM_CRITICAL){
				changeFlag = 1;
				fimd_hdlSuccRateIpAlm(sysIndex, newRate[i], succRate, 1);
			}
		}
	}

	// Alarm level setting
	for(i = 0;(i < entrysize) && newRate[i].ipAddr; i++){
		almLevel |= newRate[i].level;
	}
	succRate->level = almLevel;


	return changeFlag;

}

void fimd_check_NMS_status(int sysIndex)
{
    int i, j;
    char    beforeLevel[PORT_IDX_LAST+1];

    if(strcasecmp(sfdb->sys[sysIndex].commInfo.name, "DSCM"))
        return;

    for(i = 0; i < PORT_IDX_LAST; i++){
		beforeLevel[i] = sfdb->nmsInfo.level[i];
		if(first_flag == 0) beforeLevel[i] = 0;	
		if(sfdb->nmsInfo.mask[i] != SFM_ALM_MASKED){ 
			sfdb->nmsInfo.level[i] = SFM_ALM_MINOR;
		}		
    }

    for(i = 0; i < MAX_NMS_CON; i++){
        if(sfdb->nmsInfo.ptype[i] != FD_TYPE_DATA)
            continue;
        for(j = 0; j < PORT_IDX_LAST; j++){
            if(sfdb->nmsInfo.port[j] == sfdb->nmsInfo.port[i]){
                sfdb->nmsInfo.level[j] = SFM_ALM_NORMAL;
            }
        }
    }

    for(i = 0; i < PORT_IDX_LAST; i++){
        if(sfdb->nmsInfo.mask[i] == SFM_ALM_MASKED){
            continue;
		}

        if(sfdb->nmsInfo.level[i] != beforeLevel[i]){
            if(beforeLevel[i] == SFM_ALM_NORMAL){ // alarm occured
                fimd_hdlNmsifStsAlm(sysIndex, i, SFM_ALM_MINOR, 1);
           		fimd_broadcastAlmEvent2Client (); 
            }else{ // alarm clear
                fimd_hdlNmsifStsAlm(sysIndex, i, SFM_ALM_MINOR, 0);
	           	fimd_broadcastAlmEvent2Client (); 
            }
        }
    }
    first_flag = 1;
}

int fimd_broadcastDualAlmEvent2Client (int dualActStdFlag)
{
        int             i;
        for (i=0; i<SOCKLIB_MAX_CLIENT_CNT; i++) {
                if (cliTbl[i].bindPortNum != eventPortNum)
                        continue;
                fimd_sendDualAlmEvent2Client (i, dualActStdFlag);
        }

        return 1;

} //----- End of fimd_broadcastDualAlmEvent2Client -----//

int fimd_sendDualAlmEvent2Client (int cliIndex, int dualActStdFlag)
{
        int txLen;
        SockLibMsgType  txSockMsg;

        memset(txSockMsg.body, 0x00, sizeof(txSockMsg.body));
        strcpy (txSockMsg.body, "ALARM_EVENT_NOTIFICATION");
        //strcpy (txSockMsg.body, "DUAL ALARM_EVENT_NOTIFICATION");
        
        if (dualActStdFlag == 1)
        	txSockMsg.head.mapType = (MAPTYPE_DUAL_STATUS_ACT);
        else if (dualActStdFlag == 2)
        	txSockMsg.head.mapType = (MAPTYPE_DUAL_STATUS_STD);
        	
        txSockMsg.head.segFlag = 0;
        txSockMsg.head.seqNo = 1;
        txSockMsg.head.bodyLen = (strlen(txSockMsg.body));
        txLen = sizeof(txSockMsg.head) + strlen(txSockMsg.body); //txSockMsg.head.bodyLen;

  //      if (socklib_sndMsg (cliTbl[cliIndex].sockFd, (char*)&txSockMsg, txLen) < 0) {
        if (socklib_sndMsg_hdr_chg (cliTbl[cliIndex].sockFd, (char*)&txSockMsg, txLen) < 0) {
                sprintf(trcBuf,"[fimd_sendDualAlmEvent2Client] socklib_sndMsg fail to %s(fd=%d) MsgBody: %s\n"
                                "                           Error: %s\n",
                                cliTbl[cliIndex].cliAddr, cliTbl[cliIndex].sockFd, txSockMsg.body, strerror(errno));
                trclib_writeLogErr (FL,trcBuf);
                // client table에서 삭제한다.
                memset ((void*)&cliTbl[cliIndex], 0, sizeof(FimdClientContext));
                return -1;
        }

        return 1;

} //----- End of fimd_sendDualAlmEvent2Client -----//


