#include "mcdm_proto.h"

extern int	ixpcQid;
extern char	mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN];
extern char	trcBuf[4096], trcTmp[1024];
extern time_t	currentTime;
extern int	numDistrMmc;
extern int	trcFlag, trcLogFlag;
extern SFM_sfdb	*sfdb;
extern IxpcConSts *ixpcCON;	
extern McdmJobTblContext                mcdmJobTbl[MCDM_NUM_TP_JOB_TBL];
extern McdmDistribMmcTblContext 	mcdmDistrMmcTbl[MCDM_MAX_DISTRIB_MMC];
int fimd_getSysIndexByName (char *sysName);
int mcdm_send_local_fail_res (IxpcQMsgType *rxIxpcMsg, char *reason, char *reSysName, int contFlag);
int fimd_getProcIndexByName (int sysIndex, char *procName);
void mcdm_delSysName(int idx, MMLReqMsgType *mmlReqMsg);


//------------------------------------------------------------------------------
// mcdmDistrTbl을 확인하여 실제 명령어를 처리할 프로세스들로 메시지를 전달한다.
// 1. mcdmDistrMmcTbl에서 해당 명령어에 대한 정보를 찾는다.
// 2. mcdmDistrMmcTbl에 등록된 놈들로 메시지를 전달한다.
// 3. mcdmJobTbl에 전달한 내용을 저장한다.
//------------------------------------------------------------------------------
int mcdm_rxDistribMmcReq (GeneralQMsgType *rxGenQMsg)
{
	int	i, j,jobNo, txLen, sysIndex, procIndex;
	//char    ReferMsgId[256];	
	IxpcQMsgType			*rxIxpcMsg;
	IxpcQMsgType			rxIxpcMsgOrg;
	MMLReqMsgType			*mmlReqMsg;
	MMLReqMsgType			*mmlReqMsgOrg;
	McdmDistribMmcTblContext	*distrInfo;	

	int 	newParaCnt=0;
	int 	cannotSystem[3], cannotCnt=0;
	char 	cannotReason[3][1024];
	int sysId, prcId, flag =0;
	
	memset(cannotSystem, 0 , sizeof(int)*3);
	memset(cannotReason, 0 , 1024*3);

	rxIxpcMsg = (IxpcQMsgType*)rxGenQMsg->body;
	memcpy(&rxIxpcMsgOrg, rxGenQMsg->body , sizeof(IxpcQMsgType));
	mmlReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
	mmlReqMsgOrg = (MMLReqMsgType*) &rxIxpcMsgOrg.body;

	// 해당 명령어 정보를 찾는다.
	//
	if ((distrInfo = (McdmDistribMmcTblContext*) bsearch (
					mmlReqMsg->head.cmdName,
					mcdmDistrMmcTbl,
					numDistrMmc,
					sizeof(McdmDistribMmcTblContext),
					mcdm_distrMmcTbl_bsrchCmp)) == NULL)
	{
		sprintf(trcBuf,"[mcdm_rxDistribMmcReq] received unknown distrib_cmd(%s)\n", mmlReqMsg->head.cmdName);
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}
/*fprintf(stderr,"#[%s:%d] cmd : %s,\n dstAppName : %s,\n dstSysName : %s,\n sysCnt :%d,\n type :%d\n",
		__FUNCTION__, __LINE__,
		distrInfo->cmdName,distrInfo->dstAppName,distrInfo->dstSysName,distrInfo->sysCnt,distrInfo->type);
*/
	// 메시지를 전달한 후 전달한 내용을 저장할 jobNo를 할당한다.
	//
	if ((jobNo = mcdm_allocJobTbl ()) < 0) {
		sprintf(trcBuf,"[mcdm_rxDistribMmcReq] jobTbl alloc fail\n");
		trclib_writeLogErr (FL,trcBuf);
		return -1;
	}

	/* 먼저 mmcd jobid를 저장하고 뒤에 save에서 할당된 jobNO를 붙여 보낸다. */
	mcdmJobTbl[jobNo].mmcdJobNo    = mmlReqMsg->head.mmcdJobNo;

//printf("=====================jean jobNo=%d, %d\n", jobNo, mmlReqMsg->head.mmcdJobNo);

	rxIxpcMsg->head.byteOrderFlag = BYTE_ORDER_TAG;
	txLen = sizeof(rxIxpcMsg->head) + rxIxpcMsg->head.bodyLen;

	newParaCnt=0;
	if ( strncmp(distrInfo->dstAppName, "MMCR", strlen(distrInfo->dstAppName) ) ) {
    	memset(mmlReqMsg->head.para, 0x00, MML_MAX_PARA_CNT*sizeof(CommPara));
    	for (i=0; i<mmlReqMsgOrg->head.paraCnt; i++) {
        	if( !(strlen(mmlReqMsgOrg->head.para[i].paraVal))) continue;
       		
		strcpy (mmlReqMsg->head.para[newParaCnt].paraName, mmlReqMsgOrg->head.para[i].paraName);
        	strcpy (mmlReqMsg->head.para[newParaCnt].paraVal, mmlReqMsgOrg->head.para[i].paraVal);
        	newParaCnt++;
    	}
    		mmlReqMsg->head.paraCnt = newParaCnt;
	}



	//jean 각 분기문 안에 있던 공통 내용 밖으로 뺌.
	/* 명령어파라미터에 destination이 있으면 해당시스템으로만 전송한다.*/
	/* #1 */	
	for (i=0; i<MML_MAX_PARA_CNT; i++){
		if ( mmlReqMsg->head.para[i].paraVal[0] == NULL ) continue;

//fprintf(stderr, "parameter %d name %s, value %s\n", i, mmlReqMsg->head.para[i].paraName, mmlReqMsg->head.para[i].paraVal);

		for (j=0; j<distrInfo->sysCnt; j++){
			/* 같은 시스템이름이 있으면 */
			if ( !strcasecmp ( mmlReqMsg->head.para[i].paraVal, distrInfo->dstSysName[j] ) ){

				//jean add : check ixpc block
				if ( (sysIndex = fimd_getSysIndexByName(distrInfo->dstSysName[j])) < 0 ){
               		sprintf(trcBuf,"[mcdm_rxDistribMmcReq] cannot send distrib_cmd(%s) to %s-%s; PROC_DEAD\n",
                   			mmlReqMsg->head.cmdName, distrInfo->dstSysName[j], distrInfo->dstAppName);
               		trclib_writeLog (FL,trcBuf);

              		//mcdm_send_local_fail_res ( rxIxpcMsgOrg,"SYSTEM NOT EXIST", distrInfo->dstSysName[j], 0 );
					// sjjeon
              		mcdm_send_local_fail_res ((IxpcQMsgType*)&rxIxpcMsgOrg,"SYSTEM NOT EXIST", distrInfo->dstSysName[j], 0 );
              		return -1;
				}

//fprintf(stderr, "system para sysIndex=%d %s\n", sysIndex, distrInfo->dstAppName);
				
				if ( (procIndex = fimd_getProcIndexByName(sysIndex, "IXPC")) < 0 ){
					sprintf(trcBuf,"[mcdm_rxDistribMmcReq] cannot send distrib_cmd(%s) to %s-%s; PROC_DEAD\n",
									mmlReqMsg->head.cmdName, distrInfo->dstSysName[j], distrInfo->dstAppName);
                   	trclib_writeLog (FL,trcBuf);

                   	//mcdm_send_local_fail_res ( rxIxpcMsgOrg,"IXPC PROCESS NOT EXIST",distrInfo->dstSysName[j], 0);
					// sjjeon
                   	mcdm_send_local_fail_res ((IxpcQMsgType *)&rxIxpcMsgOrg,"IXPC PROCESS NOT EXIST",distrInfo->dstSysName[j], 0);
					return -1;
				}

				if (sfdb->sys[sysIndex].commInfo.procInfo[procIndex].status == SFM_STATUS_DEAD)
				{
					sprintf(trcBuf,"[mcdm_rxDistribMmcReq] cannot send distrib_cmd(%s) to %s-%s; PROC_DEAD\n",
							mmlReqMsg->head.cmdName, distrInfo->dstSysName[j], distrInfo->dstAppName);
					trclib_writeLog (FL,trcBuf);
					//sjjeon
					mcdm_send_local_fail_res ((IxpcQMsgType *)&rxIxpcMsgOrg,"APPLICATON DOWN OR IXPC DISCONNECTED",distrInfo->dstSysName[j], 0);
					return -1;
				}

				mcdm_saveDistrMmcInfo2JobTbl (jobNo, rxIxpcMsg, distrInfo, 1 );

				strcpy (rxIxpcMsg->head.dstSysName, distrInfo->dstSysName[j]);
				strcpy (mcdmJobTbl[jobNo].dstSysName[0], distrInfo->dstSysName[j]);
				mcdmJobTbl[jobNo].dstSysName[1][0] = NULL;

				/* system name은 빼고 준다. limsh. 2004/08/18 */
				mcdm_delSysName(i, mmlReqMsg);

				/* sjjeon:2009/04/24, 
      				dsca, dscb 시스템으로 던지는 mmc 명령어 paraCnt를 hton() 사용한다. */
				if(strcasecmp(rxIxpcMsg->head.dstSysName, "dscm"))
					mmlReqMsg->head.paraCnt = (unsigned short) htons(mmlReqMsg->head.paraCnt);

				/* {{ 20040426 */
				if ( distrInfo->type == DIST_SIN_BYPASS ){
					/*  src proc, job id를 원래 꺼로 붙인다. */
					mmlReqMsg->head.mmcdJobNo =  mcdmJobTbl[jobNo].mmcdJobNo;
					strcpy (rxIxpcMsg->head.srcAppName, mcdmJobTbl[jobNo].srcAppName );
				}
#ifdef CRYUN
				strcpy( rxIxpcMsg->head.srcAppName, distrInfo->dstAppName);
#endif
				/* }} 20040426 */
				if (msgsnd (ixpcQid, (void*)rxGenQMsg, txLen, IPC_NOWAIT) < 0) {
					sprintf(trcBuf,"[mcdm_rxDistribMmcReq] msgsnd fail distrib_cmd(%s) to %s-%s; err=%d(%s)\n",
							mmlReqMsg->head.cmdName, distrInfo->dstSysName[j], distrInfo->dstAppName, errno, strerror(errno));
					trclib_writeLogErr (FL,trcBuf);
					mcdm_deallocJobTbl (jobNo);
					return -1;
				}
			
				if (trcFlag || trcLogFlag) {
					sprintf(trcBuf,"[mcdm_rxDistribMmcReq] send distrib_cmd(%s) to %s-%s\n",
							mmlReqMsg->head.cmdName, distrInfo->dstSysName[j], distrInfo->dstAppName);
					trclib_writeLog (FL,trcBuf);
				}

				if ( distrInfo->type == DIST_SIN_BYPASS ){
					mcdm_deallocJobTbl (jobNo);
				}

				return 1;
			}  
		}
	} /* End of #1 */

	/* #2 */
	for (i=0; i<MML_MAX_PARA_CNT; i++){
		int jobNum=0;
		if ( mmlReqMsg->head.para[i].paraVal[0] == NULL ) continue;

//fprintf(stderr, "all %d name %s, value %s\n", i, mmlReqMsg->head.para[i].paraName, mmlReqMsg->head.para[i].paraVal);

		/* 같은 시스템이름이 있으면 */
		if ( !strcasecmp ( mmlReqMsg->head.para[i].paraVal, "ALL" ) ){
			for (j=0; j < distrInfo->sysCnt; j++) 	{
				if ( (sysIndex = fimd_getSysIndexByName(distrInfo->dstSysName[j])) < 0 ){
					cannotSystem[j]=1;
					strcpy(cannotReason[j],"SYSTEM NOT EXIST");
					cannotCnt++;
					continue;
				}
				if ( (procIndex = fimd_getProcIndexByName(sysIndex, "IXPC")) < 0 ){
					cannotSystem[j]=1;
					strcpy(cannotReason[j],"IXPC PROCESS NOT EXIST");
					cannotCnt++;
					continue;
				}
				if (sfdb->sys[sysIndex].commInfo.procInfo[procIndex].status == SFM_STATUS_DEAD)	{
					cannotSystem[j]=1;
					strcpy(cannotReason[j],"APPLICATON DOWN OR IXPC DISCONNECTED");
					cannotCnt++;
				}
			}

//fprintf(stderr, "jean cannotCnt =%d\n", cannotCnt);
				
			if (cannotCnt == distrInfo->sysCnt){
				for (j=0; j < distrInfo->sysCnt; j++) {
					sprintf(trcBuf,"[mcdm_rxDistribMmcReq] cannot send distrib_cmd(%s) to %s-%s; PROC_DEAD\n",
							mmlReqMsg->head.cmdName, distrInfo->dstSysName[j], distrInfo->dstAppName);
					trclib_writeLog (FL,trcBuf);

					if (j == distrInfo->sysCnt-1)	{
						//mcdm_send_local_fail_res (rxIxpcMsgOrg,cannotReason[j],distrInfo->dstSysName[j], 0);
						//sjjeon
						mcdm_send_local_fail_res ((IxpcQMsgType *)&rxIxpcMsgOrg,(char*)&cannotReason[j],distrInfo->dstSysName[j], 0);
					} else{
						//mcdm_send_local_fail_res (rxIxpcMsgOrg,cannotReason[j],distrInfo->dstSysName[j], 1);
						// sjjeon
						mcdm_send_local_fail_res ((IxpcQMsgType *)&rxIxpcMsgOrg,(char*)&cannotReason[j],distrInfo->dstSysName[j], 1);
					}

				}
				return -1;
			}

//fprintf(stderr, "%s:%d, cnt %d\n",__FUNCTION__, __LINE__, distrInfo->sysCnt-cannotCnt);			
			/* 그외에는 무조건 양쪽으로전송한다. */
			mcdm_saveDistrMmcInfo2JobTbl (jobNo, rxIxpcMsg, distrInfo, distrInfo->sysCnt-cannotCnt );

			/* ALL은 빼고 준다. limsh. 2004/08/18 */
			mcdm_delSysName(i, mmlReqMsg);
//fprintf(stderr, "%s:%d, cmdName %s\n",__FUNCTION__, __LINE__, mmlReqMsg->head.cmdName); 

			jobNum=0;
			
			for (j=0; j < distrInfo->sysCnt; j++) {
				if ( cannotSystem[j] == 1 ) {
					sprintf(trcBuf,"[mcdm_rxDistribMmcReq] cannot send distrib_cmd(%s) to %s-%s; PROC_DEAD\n",
							mmlReqMsg->head.cmdName, distrInfo->dstSysName[j], distrInfo->dstAppName);
					trclib_writeLog (FL,trcBuf);

					//mcdm_send_local_fail_res ( rxIxpcMsgOrg,cannotReason[j],distrInfo->dstSysName[j], 1);
					// sjjeon
					mcdm_send_local_fail_res ((IxpcQMsgType *)&rxIxpcMsgOrg,(char*)&cannotReason[j],distrInfo->dstSysName[j], 1);
					continue;
				}

				if ( distrInfo->type == DIST_BYPASS ) {
					
					/*  src proc, job id를 원래 꺼로 붙인다. */
					mmlReqMsg->head.mmcdJobNo =  mcdmJobTbl[jobNo].mmcdJobNo;
					strcpy (rxIxpcMsg->head.srcAppName, mcdmJobTbl[jobNo].srcAppName );
				}
				else {
					strcpy (rxIxpcMsg->head.dstSysName, distrInfo->dstSysName[j]);
					strcpy (mcdmJobTbl[jobNo].dstSysName[jobNum], distrInfo->dstSysName[j]);
				}
	
//fprintf(stderr, "#[%s:%d]############ %s %s\n",__FUNCTION__,__LINE__, rxIxpcMsg->head.dstSysName, rxIxpcMsg->head.dstAppName);

				if (msgsnd (ixpcQid, (void*)rxGenQMsg, txLen, IPC_NOWAIT) < 0) {
					sprintf(trcBuf,"[mcdm_rxDistribMmcReq] msgsnd fail distrib_cmd(%s) to %s-%s; err=%d(%s)\n",
							mmlReqMsg->head.cmdName, distrInfo->dstSysName[j], distrInfo->dstAppName, errno, strerror(errno));
					trclib_writeLogErr (FL,trcBuf);
					mcdm_deallocJobTbl (jobNo);
					return -1;
				}
				if (trcFlag || trcLogFlag) {
					sprintf(trcBuf,"[mcdm_rxDistribMmcReq] send distrib_cmd(%s) to %s-%s\n",
							mmlReqMsg->head.cmdName, distrInfo->dstSysName[j], distrInfo->dstAppName);
					trclib_writeLog (FL,trcBuf);
				}
				
				jobNum++;

				if (j < (distrInfo->sysCnt -1))
					commlib_microSleep(50000);
			}

			if ( distrInfo->type == DIST_BYPASS ){
				mcdm_deallocJobTbl (jobNo);
			}
				
			return 1;
		}  
	} /* End of #2 */

	/* #3 */
	if ( distrInfo->type == DIST_ACTIVE_OMP || distrInfo->type == DIST_ACTIVE ){
		
//fprintf(stderr, "#[%s:%d]############ type : DIST_ACTIVE_OMP & DIST_ACTIVE\n",__FUNCTION__,__LINE__);
		int jobNum=0;
	    sysId =0;
		prcId =0;
		flag =0;
		/* 전송 시스템만 count */
//		if(!strcasecmp(sfdb->active_sys_name, "ACTIVE") ){

			for (j=0; j < distrInfo->sysCnt; j++) {
				if ( (sysIndex = fimd_getSysIndexByName(distrInfo->dstSysName[j])) < 0 ){
					cannotSystem[j]=1;
					strcpy(cannotReason[j],"SYSTEM NOT EXIST");
					cannotCnt++;
					continue;
				}

				//printf ("sysIndex: %d, %s\n", sysIndex, distrInfo->dstSysName[j]);

				if ((sysIndex != 0) && (sfdb->sys[sysIndex].commInfo.systemDup.myStatus != 1)) {
					cannotSystem[j]=2;		// not active
					strcpy(cannotReason[j],"SYSTEM NOT ACTIVE");
					cannotCnt++;
					continue;
				}

				if ( (procIndex = fimd_getProcIndexByName(sysIndex, "IXPC")) < 0 ){
					cannotSystem[j]=1;
					strcpy(cannotReason[j],"IXPC PROCESS NOT EXIST");
					cannotCnt++;
					continue;
				}

				if (sfdb->sys[sysIndex].commInfo.procInfo[procIndex].status == SFM_STATUS_DEAD)	{
					cannotSystem[j]=1;
					strcpy(cannotReason[j],"APPLICATON DOWN OR IXPC DISCONNECTED");
					cannotCnt++;
				}
			}
//		}

#if 0
		else if ( !strcasecmp(sfdb->active_sys_name, "STANDBY") ||  !strcasecmp(sfdb->active_sys_name, "UNKNOWN") ){
			for (j=0; j < distrInfo->sysCnt; j++) {
				if ( (sysIndex = fimd_getSysIndexByName(distrInfo->dstSysName[j])) < 0 ){
					cannotSystem[j]=1;
					strcpy(cannotReason[j],"SYSTEM NOT EXIST");
					cannotCnt++;
					continue;
				}
				if ( (procIndex = fimd_getProcIndexByName(sysIndex, "IXPC")) < 0 ){
					cannotSystem[j]=1;
					strcpy(cannotReason[j],"IXPC PROCESS NOT EXIST");
					cannotCnt++;
					continue;
				}

				if (sfdb->sys[sysIndex].commInfo.procInfo[procIndex].status == SFM_STATUS_DEAD || 
					strcasecmp(distrInfo->dstSysName[j], "DSCM") )	{
					cannotSystem[j]=1;
					if(!strcasecmp(distrInfo->dstSysName[j], "DSCM")){
						strcpy(cannotReason[j],"APPLICATON DOWN OR IXPC DISCONNECTED");
					}else{
						printf ("sysname: %s\n", distrInfo->dstSysName[j]);
						strcpy(cannotReason[j],"ACTIVE_SYSTEM_NAME IS UNKNOWN");
					}
					cannotCnt++;
				}
			}

		}
		else {
			for (j=0; j < distrInfo->sysCnt; j++) {
				if ( (sysIndex = fimd_getSysIndexByName(sfdb->active_sys_name)) < 0 ){
					cannotSystem[j]=1;
					strcpy(cannotReason[j],"SYSTEM NOT EXIST");
					cannotCnt++;
					continue;
				}
				if ( (procIndex = fimd_getProcIndexByName(sysIndex, "IXPC")) < 0 ){
					cannotSystem[j]=1;
					strcpy(cannotReason[j],"IXPC PROCESS NOT EXIST");
					cannotCnt++;
					continue;
				}

				if (sfdb->sys[sysIndex].commInfo.procInfo[procIndex].status == SFM_STATUS_DEAD || 
					(strcasecmp(sfdb->active_sys_name, distrInfo->dstSysName[j]) && 
					 strcasecmp(distrInfo->dstSysName[j], "DSCM")) )	{

					 if ( strcasecmp(sfdb->active_sys_name, distrInfo->dstSysName[j]) && 
						  strcasecmp(distrInfo->dstSysName[j], "DSCM")){

						cannotSystem[j]=2;
						strcpy(cannotReason[j],"NOT ACTIVE SYSTEM");
						cannotCnt++;

					 } else{

						cannotSystem[j]=1;
						strcpy(cannotReason[j],"APPLICATON DOWN OR IXPC DISCONNECTED");
						cannotCnt++;

					}
				}

			}
		}
#endif

//printf("jean cannotCnt =%d %d\n", cannotCnt, distrInfo->sysCnt);
		
		if (cannotCnt == distrInfo->sysCnt){

//printf("jean cannotCnt =%d %d\n", cannotCnt, distrInfo->sysCnt);
			
			for (j=0; j < distrInfo->sysCnt; j++) {
				if ( cannotSystem[j] == 2 ) {
					continue;
				}

				sprintf(trcBuf,"[mcdm_rxDistribMmcReq] cannot send distrib_cmd(%s) to %s-%s; PROC_DEAD\n",
						mmlReqMsg->head.cmdName, distrInfo->dstSysName[j], distrInfo->dstAppName);
				trclib_writeLog (FL,trcBuf);

				if (j == distrInfo->sysCnt-1)	{
					//mcdm_send_local_fail_res ( rxIxpcMsgOrg,cannotReason[j],distrInfo->dstSysName[j], 0);
					//sjjeon
					mcdm_send_local_fail_res ((IxpcQMsgType *)&rxIxpcMsgOrg,(char*)&cannotReason[j],distrInfo->dstSysName[j], 0);
				} else{
					//mcdm_send_local_fail_res ( rxIxpcMsgOrg,cannotReason[j],distrInfo->dstSysName[j], 1);
					//sjjeon
					mcdm_send_local_fail_res ((IxpcQMsgType *)&rxIxpcMsgOrg,(char*)&cannotReason[j],distrInfo->dstSysName[j], 1);
				}

			}
			return -1;
		}

//printf("++++++++++++++++jean cnt %d\n", distrInfo->sysCnt-cannotCnt);			
		
		/* 그외에는 무조건 양쪽으로전송한다. */
		mcdm_saveDistrMmcInfo2JobTbl (jobNo, rxIxpcMsg, distrInfo, distrInfo->sysCnt-cannotCnt );

		jobNum=0;
		for (i=0; i < distrInfo->sysCnt; i++) {

			if ( cannotSystem[i] == 1 ){
				sprintf(trcBuf,"[mcdm_rxDistribMmcReq] cannot send distrib_cmd(%s) to %s-%s; PROC_DEAD\n",
						mmlReqMsg->head.cmdName, distrInfo->dstSysName[i], distrInfo->dstAppName);
				trclib_writeLog (FL,trcBuf);

//printf("jean************ 222 %s can not system [%s]\n", distrInfo->dstSysName[i], cannotReason[i]);
				
				//mcdm_send_local_fail_res ( rxIxpcMsgOrg,cannotReason[i],distrInfo->dstSysName[i], 1);
				//sjjeon
				mcdm_send_local_fail_res ((IxpcQMsgType *)& rxIxpcMsgOrg,(char*)&cannotReason[i],distrInfo->dstSysName[i], 1);
				continue;
			} else if ( cannotSystem[i] == 2 ) {

//printf("jean************ 222 %s not active system [%s]\n", distrInfo->dstSysName[i], cannotReason[i]);
				
				continue;
			}

			strcpy (rxIxpcMsg->head.dstSysName, distrInfo->dstSysName[i]);
			strcpy (mcdmJobTbl[jobNo].dstSysName[jobNum], distrInfo->dstSysName[i]);

//fprintf(stderr, "#[%s:%d]############ Send dstSysName : %s \n",__FUNCTION__,__LINE__, rxIxpcMsg->head.dstSysName);
			if (msgsnd (ixpcQid, (void*)rxGenQMsg, txLen, IPC_NOWAIT) < 0) {
				sprintf(trcBuf,"[mcdm_rxDistribMmcReq] msgsnd fail distrib_cmd(%s) to %s-%s; err=%d(%s)\n",
						mmlReqMsg->head.cmdName, distrInfo->dstSysName[i], distrInfo->dstAppName, errno, strerror(errno));
				trclib_writeLogErr (FL,trcBuf);
				mcdm_deallocJobTbl (jobNo);
				return -1;
			}
			if (trcFlag || trcLogFlag) {
				sprintf(trcBuf,"[mcdm_rxDistribMmcReq] send distrib_cmd(%s) to %s-%s\n",
						mmlReqMsg->head.cmdName, distrInfo->dstSysName[i], distrInfo->dstAppName);
				trclib_writeLog (FL,trcBuf);
			}
			jobNum++;

			if (i < (distrInfo->sysCnt -1))
				commlib_microSleep(50000);
		}

		return 1;

	} else if ( distrInfo->type == DIST_SINGLE || distrInfo->type == DIST_SIN_BYPASS ) { //일단 처리 안함.
		
		sysId = 0;
	    prcId = 0;
		flag = 0;

		/* application상태를 확인하여 한쪽으로 전송한다. */
		for (j=0; j<distrInfo->sysCnt; j++){
			
			/* 전송 시스템만 count */
			mcdm_saveDistrMmcInfo2JobTbl (jobNo, rxIxpcMsg, distrInfo, 1 );

			strcpy (rxIxpcMsg->head.dstSysName, distrInfo->dstSysName[j]);
			strcpy (mcdmJobTbl[jobNo].dstSysName[0], distrInfo->dstSysName[j]);

			mcdmJobTbl[jobNo].dstSysName[1][0] = NULL;

			if ( distrInfo->type == DIST_SIN_BYPASS ){
				/*  src proc, job id를 원래 꺼로 붙인다. */
				mmlReqMsg->head.mmcdJobNo =  mcdmJobTbl[jobNo].mmcdJobNo;
				strcpy (rxIxpcMsg->head.srcAppName, mcdmJobTbl[jobNo].srcAppName );
			}

			if (msgsnd (ixpcQid, (void*)rxGenQMsg, txLen, IPC_NOWAIT) < 0) {
				sprintf(trcBuf,"[mcdm_rxDistribMmcReq] send distrib_cmd(%s) to %s-%s\n",
						mmlReqMsg->head.cmdName, distrInfo->dstSysName[j], distrInfo->dstAppName);
				trclib_writeLogErr (FL,trcBuf);
				mcdm_deallocJobTbl (jobNo);
				return -1;
			}

			if (trcFlag || trcLogFlag) {
				sprintf(trcBuf,"[mcdm_rxDistribMmcReq] send distrib_cmd(%s) to %s-%s\n",
						mmlReqMsg->head.cmdName, rxIxpcMsg->head.dstSysName, rxIxpcMsg->head.dstAppName);
				trclib_writeLog (FL,trcBuf);
			}

			if ( distrInfo->type == DIST_SIN_BYPASS ){
				mcdm_deallocJobTbl (jobNo);
			}
			return 1;
		}

		if ( flag == 0 ){
			sprintf(trcBuf,"[mcdm_rxDistribMmcReq] msgsnd fail distrib_cmd(%s) to %s-%s; TYPE=SINGLE,PROC_DEAD,err=%d(%s)\n",
					mmlReqMsg->head.cmdName, distrInfo->dstSysName[j], distrInfo->dstAppName, errno, strerror(errno));
			trclib_writeLogErr (FL,trcBuf);
		//	mcdm_send_local_fail_res ( rxIxpcMsgOrg,"APPLICATON DOWN OR IXPC DISCONNECTED",distrInfo->dstSysName, 0);
			// sjjeon
			mcdm_send_local_fail_res ((IxpcQMsgType *)& rxIxpcMsgOrg,"APPLICATON DOWN OR IXPC DISCONNECTED",distrInfo->dstSysName[j], 0);
			return -1;
		}

	} else {
		
		int jobNum=0;

		for (j=0; j < distrInfo->sysCnt; j++) {
			if ( (sysIndex = fimd_getSysIndexByName(distrInfo->dstSysName[j])) < 0 ){
				cannotSystem[j]=1;
				strcpy(cannotReason[j],"SYSTEM NOT EXIST");
				cannotCnt++;
				continue;
			}
			if ( (procIndex = fimd_getProcIndexByName(sysIndex, "IXPC")) < 0 ){
				cannotSystem[j]=1;
				strcpy(cannotReason[j],"IXPC PROCESS NOT EXIST");
				cannotCnt++;
				continue;
			}

			if (sfdb->sys[sysIndex].commInfo.procInfo[procIndex].status == SFM_STATUS_DEAD)	{
				cannotSystem[j]=1;
				strcpy(cannotReason[j],"APPLICATON DOWN OR IXPC DISCONNECTED");
				cannotCnt++;
			}
		}

//printf("jean cannotCnt =%d\n", cannotCnt);
		
		if (cannotCnt == distrInfo->sysCnt){
			for (j=0; j < distrInfo->sysCnt; j++) {
				sprintf(trcBuf,"[mcdm_rxDistribMmcReq] cannot send distrib_cmd(%s) to %s-%s; PROC_DEAD\n",
						mmlReqMsg->head.cmdName, distrInfo->dstSysName[j], distrInfo->dstAppName);
				trclib_writeLog (FL,trcBuf);

				if (j == distrInfo->sysCnt-1)	{
					//mcdm_send_local_fail_res ( rxIxpcMsgOrg,cannotReason[j],distrInfo->dstSysName[j], 0);
					//sjjeon
					mcdm_send_local_fail_res ((IxpcQMsgType *)& rxIxpcMsgOrg,(char*)&cannotReason[j],distrInfo->dstSysName[j], 0);
				} else{
					//mcdm_send_local_fail_res ( rxIxpcMsgOrg,cannotReason[j],distrInfo->dstSysName[j], 1);
					//sjjeon
					mcdm_send_local_fail_res ((IxpcQMsgType *)& rxIxpcMsgOrg,(char*)&cannotReason[j],distrInfo->dstSysName[j], 1);
				}

			}
			return -1;
		}

printf("++++++++++++++++ cnt %d\n", distrInfo->sysCnt-cannotCnt);			
		
		/* 그외에는 무조건 양쪽으로전송한다. */
		mcdm_saveDistrMmcInfo2JobTbl (jobNo, rxIxpcMsg, distrInfo, distrInfo->sysCnt-cannotCnt );

		jobNum=0;
		for (i=0; i < distrInfo->sysCnt; i++) {

			if ( cannotSystem[i] == 1 ){
				sprintf(trcBuf,"[mcdm_rxDistribMmcReq] cannot send distrib_cmd(%s) to %s-%s; PROC_DEAD\n",
						mmlReqMsg->head.cmdName, distrInfo->dstSysName[i], distrInfo->dstAppName);
				trclib_writeLog (FL,trcBuf);

				//yhshin 
				//mcdm_send_local_fail_res (rxIxpcMsgOrg,cannotReason[i],(char*)&distrInfo->dstSysName[i], 1);
				mcdm_send_local_fail_res ((IxpcQMsgType *)& rxIxpcMsgOrg,(char*)&cannotReason[i],distrInfo->dstSysName[i], 1);
				continue;
			}

			if ( distrInfo->type == DIST_BYPASS ){
				/*  src proc, job id를 원래 꺼로 붙인다. */
				mmlReqMsg->head.mmcdJobNo =  mcdmJobTbl[jobNo].mmcdJobNo;
				strcpy (rxIxpcMsg->head.srcAppName, mcdmJobTbl[jobNo].srcAppName );
			}
			else {
				strcpy (rxIxpcMsg->head.dstSysName, distrInfo->dstSysName[i]);
				strcpy (mcdmJobTbl[jobNo].dstSysName[jobNum], distrInfo->dstSysName[i]);
			}
	
//printf("#[%s:%d]############ %s %s\n",__FUNCTION__, __LINE__, rxIxpcMsg->head.dstSysName, rxIxpcMsg->head.dstAppName);

			if (msgsnd (ixpcQid, (void*)rxGenQMsg, txLen, IPC_NOWAIT) < 0) {
				sprintf(trcBuf,"[mcdm_rxDistribMmcReq] msgsnd fail distrib_cmd(%s) to %s-%s; err=%d(%s)\n",
						mmlReqMsg->head.cmdName, distrInfo->dstSysName[i], distrInfo->dstAppName, errno, strerror(errno));
				trclib_writeLogErr (FL,trcBuf);
				mcdm_deallocJobTbl (jobNo);
				return -1;
			}
			if (trcFlag || trcLogFlag) {
				sprintf(trcBuf,"[mcdm_rxDistribMmcReq] send distrib_cmd(%s) to %s-%s\n",
						mmlReqMsg->head.cmdName, distrInfo->dstSysName[i], distrInfo->dstAppName);
				trclib_writeLog (FL,trcBuf);
			}
			jobNum++;

			if (i < (distrInfo->sysCnt -1))
				commlib_microSleep(50000);
		}

		if ( distrInfo->type == DIST_BYPASS ){
			mcdm_deallocJobTbl (jobNo);
		}
			
	} /* End of #3 */

	return 1;

} //----- End of mcdm_rxDistribMmcReq -----//



//------------------------------------------------------------------------------
// 실제 명령어를 처리할 놈들로 보낸 정보를 mcdmJobTbl에 저장한다.
//------------------------------------------------------------------------------
int mcdm_saveDistrMmcInfo2JobTbl (
		int jobNo,
		IxpcQMsgType *rxIxpcMsg,
		McdmDistribMmcTblContext *distrInfo,
		int reqCnt
		)
{
	//int		i;
	MMLReqMsgType	*mmlReqMsg;

	mmlReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	mcdmJobTbl[jobNo].tpInd        = 1;
	mcdmJobTbl[jobNo].reqSysCnt    = reqCnt;
	mcdmJobTbl[jobNo].deadlineTime = currentTime + MCDM_WAIT_RESPONSE_TIMER;
	strcpy (mcdmJobTbl[jobNo].cmdName, mmlReqMsg->head.cmdName);
	strcpy (mcdmJobTbl[jobNo].srcSysName, rxIxpcMsg->head.srcSysName);
	strcpy (mcdmJobTbl[jobNo].srcAppName, rxIxpcMsg->head.srcAppName);

	strcpy (rxIxpcMsg->head.srcSysName, mySysName);
	strcpy (rxIxpcMsg->head.srcAppName, myAppName);
	strcpy (rxIxpcMsg->head.dstAppName, distrInfo->dstAppName);
	mmlReqMsg->head.mmcdJobNo = jobNo;

	if (trcFlag || trcLogFlag) {
		sprintf(trcBuf,"[mcdm_saveDistrMmcInfo2JobTbl] jobNo=%d,reqSysCnt=%d\n",
				jobNo, mcdmJobTbl[jobNo].reqSysCnt );
		trclib_writeLog (FL,trcBuf);
	}

	return 1;

} //----- End of mcdm_saveDistrMmcInfo2JobTbl -----//

//------------------------------------------------------------------------------
// sfdb->sys에서 system name과 일치하는 index를 찾는다.
//------------------------------------------------------------------------------
int fimd_getSysIndexByName (char *sysName)
{
    int     i;

    for (i=0; i<SYSCONF_MAX_ASSO_SYS_NUM; i++) {
#if 0
    sprintf(trcBuf,"[mcdm_getSysIndexByName] sfdb->sys[%d].commInfo.name:%s,sysName:%s \n",i,sfdb->sys[i].commInfo.name, sysName);
    trclib_writeLogErr (FL,trcBuf);
#endif

        if (!strcasecmp (sfdb->sys[i].commInfo.name, sysName))
            return i;
    }
    sprintf(trcBuf,"[mcdm_getSysIndexByName] not found sysName[%s]\n", sysName);
    trclib_writeLogErr (FL,trcBuf);
    return -1;

} //----- End of fimd_getSysIndexByName -----//

//------------------------------------------------------------------------------
// sfdb->sys에서 해당 시스템내의 proc name과 일치하는 index를 찾는다.
//------------------------------------------------------------------------------
int fimd_getProcIndexByName (int sysIndex, char *procName)
{
    int     i;

    for (i=0; i<SFM_MAX_PROC_CNT; i++) {
        if (!strcasecmp (sfdb->sys[sysIndex].commInfo.procInfo[i].name, procName))
            return i;
    }
    sprintf(trcBuf,"[fimd_getProcIndexByName] not found procName[%s] at %s\n",
            procName, sfdb->sys[sysIndex].commInfo.name);
    trclib_writeLogErr (FL,trcBuf);
    return -1;

} //----- End of fimd_getProcIndexByName -----//


void mcdm_delSysName(int idx, MMLReqMsgType *mmlReqMsg)
{

	int nextCnt;
	CommPara	tmpPara[MML_MAX_PARA_CNT];

	nextCnt = mmlReqMsg->head.paraCnt-idx-1;	
	if( nextCnt<=0 ) {
		memset(&mmlReqMsg->head.para[idx], 0x00, sizeof(CommPara));
	}
	else {
		memcpy(tmpPara, &mmlReqMsg->head.para[idx+1], nextCnt*sizeof(CommPara));
		memset(&mmlReqMsg->head.para[idx], 0x00, (nextCnt + 1)*sizeof(CommPara));
		memcpy(&mmlReqMsg->head.para[idx], tmpPara, nextCnt*sizeof(CommPara));
	}
	mmlReqMsg->head.paraCnt -= 1;

}



