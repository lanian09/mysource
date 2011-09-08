#include "fimd_proto.h"

extern SFM_SCE      *g_pstSCEInfo;

extern char resBuf[4096], resHead[1024]; 


void fimd_mmc_makeSceAlmClsOutputMsg(IxpcQMsgType *rxIxpcMsg, int sysIndex, char *seqNo, int all);
int fimd_mmc_sce_set_alm_lmt(IxpcQMsgType *rxIxpcMsg, char argSys[32], char argType[32], 
							char argLevel[32], int limit, int durat, int sysIndex );
int checkValidForSce(char argSys[32],char argType[32],char argLevel[32],int limit,int durat );
int fimd_mmc_updateSceAlmClsValue (int type, int level, int limit, int durat, int sysIndex);


/* SCE Make MMC-INFO, sjjeon*/
void fimd_mmc_makeSceAlmClsOutputMsg(
			IxpcQMsgType *rxIxpcMsg,
			int sysIndex,
			char *seqNo, int all
			)
{
//	int		i;
	char	tmpBuf[512], trcBuf[25];

	// 시스템 공통 정보
	//
	if(all == 1){
		if(sysIndex == 0) 
			sprintf(trcBuf, "    SYSTEM = [SCEA]\n");
		else 
			sprintf(trcBuf, "    SYSTEM = [SCEB]\n");
		strcat(resBuf, trcBuf);
	}

	/* CPU 정보: 1개정보만 보여준다.*/
#if 0
	for(i=0; i<MAX_SCE_CPU_CNT; i++) {

		memset(tmpBuf,0x00,sizeof(tmpBuf));
		sprintf(tmpBuf,"    CPU-%d        :  %-6d %-6d %-6d   %-8d %-8d %-8d\n",
			i,
			g_pstSCEInfo->SCEDev[sysIndex].cpuInfo[i].minLimit,
			g_pstSCEInfo->SCEDev[sysIndex].cpuInfo[i].majLimit,
			g_pstSCEInfo->SCEDev[sysIndex].cpuInfo[i].criLimit,
			g_pstSCEInfo->SCEDev[sysIndex].cpuInfo[i].minDurat, 
			g_pstSCEInfo->SCEDev[sysIndex].cpuInfo[i].majDurat,
			g_pstSCEInfo->SCEDev[sysIndex].cpuInfo[i].criDurat);

		strcat (resBuf,tmpBuf);
	}
#else
	memset(tmpBuf,0x00,sizeof(tmpBuf));
	sprintf(tmpBuf,"    CPU          :  %-6d %-6d %-6d   %-8d %-8d %-8d\n",
		g_pstSCEInfo->SCEDev[sysIndex].cpuInfo[0].minLimit,
		g_pstSCEInfo->SCEDev[sysIndex].cpuInfo[0].majLimit,
		g_pstSCEInfo->SCEDev[sysIndex].cpuInfo[0].criLimit,
		g_pstSCEInfo->SCEDev[sysIndex].cpuInfo[0].minDurat, 
		g_pstSCEInfo->SCEDev[sysIndex].cpuInfo[0].majDurat,
		g_pstSCEInfo->SCEDev[sysIndex].cpuInfo[0].criDurat);
		
	strcat (resBuf,tmpBuf);
#endif

	/* MEM 정보: 1개정보만 보여준다. */
#if 0
	for(i=0; i<MAX_SCE_MEM_CNT; i++) {

		memset(tmpBuf,0x00,sizeof(tmpBuf));
		sprintf(tmpBuf,"    MEM-%d        :  %-6d %-6d %-6d   %-8d %-8d %-8d\n",
			i,
			g_pstSCEInfo->SCEDev[sysIndex].memInfo[i].minLimit,
			g_pstSCEInfo->SCEDev[sysIndex].memInfo[i].majLimit,
			g_pstSCEInfo->SCEDev[sysIndex].memInfo[i].criLimit,
			g_pstSCEInfo->SCEDev[sysIndex].memInfo[i].minDurat, 
			g_pstSCEInfo->SCEDev[sysIndex].memInfo[i].majDurat,
			g_pstSCEInfo->SCEDev[sysIndex].memInfo[i].criDurat);

		strcat (resBuf,tmpBuf);
	}
#else
	memset(tmpBuf,0x00,sizeof(tmpBuf));
	sprintf(tmpBuf,"    MEM          :  %-6d %-6d %-6d   %-8d %-8d %-8d\n",
		g_pstSCEInfo->SCEDev[sysIndex].memInfo[0].minLimit,
		g_pstSCEInfo->SCEDev[sysIndex].memInfo[0].majLimit,
		g_pstSCEInfo->SCEDev[sysIndex].memInfo[0].criLimit,
		g_pstSCEInfo->SCEDev[sysIndex].memInfo[0].minDurat, 
		g_pstSCEInfo->SCEDev[sysIndex].memInfo[0].majDurat,
		g_pstSCEInfo->SCEDev[sysIndex].memInfo[0].criDurat);

	strcat (resBuf,tmpBuf);
#endif 

	memset(tmpBuf,0x00,sizeof(tmpBuf));
#if 0
	sprintf(tmpBuf,"    DISK         :  %-6d %-6d %-6d   %-8d %-8d %-8d\n",
			g_pstSCEInfo->SCEDev[sysIndex].diskInfo.minLimit,
			g_pstSCEInfo->SCEDev[sysIndex].diskInfo.majLimit,
			g_pstSCEInfo->SCEDev[sysIndex].diskInfo.criLimit,
			g_pstSCEInfo->SCEDev[sysIndex].diskInfo.minDurat, 
			g_pstSCEInfo->SCEDev[sysIndex].diskInfo.majDurat,
			g_pstSCEInfo->SCEDev[sysIndex].diskInfo.criDurat);
#else
	sprintf(tmpBuf,"    DISK         :  %-6d %-6d %-6d\n",
			g_pstSCEInfo->SCEDev[sysIndex].diskInfo.minLimit,
			g_pstSCEInfo->SCEDev[sysIndex].diskInfo.majLimit,
			g_pstSCEInfo->SCEDev[sysIndex].diskInfo.criLimit);
#endif
	strcat (resBuf,tmpBuf);

	strcat (resBuf,"    ==================================================================\n");

	if (strlen(resBuf) > 3000) { // 3000 byte이상이면 mmcd로 결과메시지를 보낸다.
		strcat (resBuf,"\n");
		fimd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, 5, 1, (*seqNo)++);
		strcpy (resBuf, resHead);
		commlib_microSleep(50000); // 50 MS
	}
	memset(trcBuf, 0x00, sizeof(trcBuf));
	return;

}

/* hjjung */
void fimd_mmc_makeSceSessionAlmClsOutputMsg(
			IxpcQMsgType *rxIxpcMsg,
			int sysIndex,
			char *seqNo, int all
			)
{
//	int		i;
	char	tmpBuf[512], trcBuf[25];

	// 시스템 공통 정보
	//
	if(all == 1){
		if(sysIndex == 0) 
			sprintf(trcBuf, "    SYSTEM = [SCEA]\n");
		else 
			sprintf(trcBuf, "    SYSTEM = [SCEB]\n");
		strcat(resBuf, trcBuf);
	}

	/* hjjung_20100823 */
	memset(tmpBuf,0x00,sizeof(tmpBuf));
#if 0
	sprintf(tmpBuf,"    USER         :  %-6d %-6d %-6d   %-8d %-8d %-8d\n",
			g_pstSCEInfo->SCEDev[sysIndex].userInfo.minLimit,
			g_pstSCEInfo->SCEDev[sysIndex].userInfo.majLimit,
			g_pstSCEInfo->SCEDev[sysIndex].userInfo.criLimit,
			g_pstSCEInfo->SCEDev[sysIndex].userInfo.minDurat, 
			g_pstSCEInfo->SCEDev[sysIndex].userInfo.majDurat,
			g_pstSCEInfo->SCEDev[sysIndex].userInfo.criDurat);
#else
	sprintf(tmpBuf,"    USER         :  %-6d %-6d %-6d   %-8d %-8d %-8d\n",
			g_pstSCEInfo->SCEDev[sysIndex].userInfo.minLimit,
			g_pstSCEInfo->SCEDev[sysIndex].userInfo.majLimit,
			g_pstSCEInfo->SCEDev[sysIndex].userInfo.criLimit,
			g_pstSCEInfo->SCEDev[sysIndex].userInfo.minDurat,
			g_pstSCEInfo->SCEDev[sysIndex].userInfo.majDurat,
			g_pstSCEInfo->SCEDev[sysIndex].userInfo.criDurat);
#endif
	strcat (resBuf,tmpBuf);

	strcat (resBuf,"    ==================================================================\n");

	if (strlen(resBuf) > 3000) { // 3000 byte이상이면 mmcd로 결과메시지를 보낸다.
		strcat (resBuf,"\n");
		fimd_txMMLResult (rxIxpcMsg, resBuf, 0, 1, 5, 1, (*seqNo)++);
		strcpy (resBuf, resHead);
		commlib_microSleep(50000); // 50 MS
	}
	memset(trcBuf, 0x00, sizeof(trcBuf));
	return;

}


/* SCE Make SET-INFO, sjjeon*/
int fimd_mmc_sce_set_alm_lmt(
			IxpcQMsgType *rxIxpcMsg, 
			char argSys[32],
			char argType[32],
			char argLevel[32],
			int limit,
			int durat,
			int sysIndex )
{

	char tmpBuf[256];
	int type, level;
	if( checkValidForSce( argSys,argType,argLevel,limit,durat ) < 0 ) {
		sprintf(resBuf,"    RESULT = FAIL\n    REASON = INVALID PARAMETER\n");
		fimd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, 1);
		return -1;
	}


    sprintf(resBuf,"    RESULT = SUCCESS\n");
        sprintf(tmpBuf,"    SYSTEM = %s\n", argSys);
        strcat (resBuf,tmpBuf);
        sprintf(tmpBuf,"    TYPE   = %s\n", argType);
        strcat (resBuf,tmpBuf);
        sprintf(tmpBuf,"    LEVEL  = %s\n", argLevel);
        strcat (resBuf,tmpBuf);
        sprintf(tmpBuf,"    LIMIT  = %d\n", limit);
        strcat (resBuf,tmpBuf);
        if (durat>0) {
            sprintf(tmpBuf,"    DURAT  = %d\n", durat);
            strcat (resBuf,tmpBuf);
        }


	if (!strcasecmp(argType, "CPU")) 
		type = SFM_ALM_TYPE_CPU_USAGE;
	else if (!strcasecmp(argType, "MEM"))
		type = SFM_ALM_TYPE_MEMORY_USAGE;
	else if (!strcasecmp(argType, "DISK"))
		type = SFM_ALM_TYPE_DISK_USAGE;
	/* hjjung */
	else if (!strcasecmp(argType, "USER"))
		type = SFM_ALM_TYPE_USER_USAGE;

    if (!strcasecmp(argLevel, "MINOR"))
        level = SFM_ALM_MINOR;
    else if (!strcasecmp(argLevel, "MAJOR")) 
        level = SFM_ALM_MAJOR;
    else if (!strcasecmp(argLevel, "CRITICAL")) 
        level = SFM_ALM_CRITICAL;

	switch (type)
	{
		case SFM_ALM_TYPE_CPU_USAGE:
			fimd_mmc_updateSceAlmClsValue (type, level, limit, durat, sysIndex);
			break;

		case SFM_ALM_TYPE_MEMORY_USAGE:
			fimd_mmc_updateSceAlmClsValue (type, level, limit, durat, sysIndex);
			break;
		case SFM_ALM_TYPE_DISK_USAGE:
			fimd_mmc_updateSceAlmClsValue (type, level, limit, durat, sysIndex);
			break;
		/* hjjung */
		case SFM_ALM_TYPE_USER_USAGE:
			fimd_mmc_updateSceAlmClsValue (type, level, limit, durat, sysIndex);
			break;
	}

	fimd_txMMLResult (rxIxpcMsg, resBuf, 0, 0, 0, 0, 1);

	fimd_updateSceAlmInfo();
	fimd_broadcastAlmEvent2Client();
	return 1;

}

/* sjjeon */
int checkValidForSce( char argSys[32],char argType[32],char argLevel[32],int limit,int durat )
{
    int result = -1;
    if ( !strcasecmp( argSys, "SCEA" ) || !strcasecmp( argSys, "SCEB" ) ) {
        //if( !strcasecmp( argType, "CPU") || !strcasecmp( argType, "MEM" ) )
        if( !strcasecmp( argType, "CPU") || !strcasecmp( argType, "MEM" ) || !strcasecmp( argType, "DISK") ){
            if( !strcasecmp( argLevel, "MINOR") || !strcasecmp( argLevel, "MAJOR" ) || !strcasecmp( argLevel, "CRITICAL" ) )
                if( limit>=1 && limit<=99 )
                    if( durat>=0 && durat<=255 )
                        return 1;
		/* hjjung add USER */
		}else if(!strcasecmp( argType, "USER")){
            if( !strcasecmp( argLevel, "MINOR") || !strcasecmp( argLevel, "MAJOR" ) || !strcasecmp( argLevel, "CRITICAL" ) )
                if( limit>=1 && limit<=999999 )
                    if( durat>=0 && durat<=255 )
                        return 1;
		}
	}

    return result;
}

/* update info - sjjeon*/
int fimd_mmc_updateSceAlmClsValue (int type, int level, int limit, int durat, int sysIndex)
{
	int i=0;
	SFM_SCEDev *pSceInfo = NULL;
	if(sysIndex>=0 && sysIndex<2)
		pSceInfo = (SFM_SCEDev*)&g_pstSCEInfo->SCEDev[sysIndex];  // SCEA=0 & SCEB=1 info
	else {
		sprintf(resBuf,"    RESULT = FAIL\n    REASON = INVALID SYSTEM TYPE : %d))\n\n", sysIndex);
	 	return -1;
	}
/*sce 각 장비별 cpu에 모두 동일한 값 설정 (sce 1대 3cpu)*/
	switch(type) {

		/*CPU INFO*/
		case SFM_ALM_TYPE_CPU_USAGE:

			switch(level){
			case SFM_ALM_MINOR:
				for(i=0;i<MAX_SCE_CPU_CNT;i++){
					if((unsigned char)limit >=  pSceInfo->cpuInfo[i].majLimit){
						sprintf(resBuf,"    RESULT = FAIL\n    REASON = INVALID LIMIT VALUE(MORE THAN MAJOR(%d))\n\n", pSceInfo->cpuInfo[i].majLimit);
								return -1;
					}
					pSceInfo->cpuInfo[i].minLimit = (unsigned char)limit;
					if(durat) pSceInfo->cpuInfo[i].minDurat = (unsigned char)durat;
				}
				break;

			case SFM_ALM_MAJOR:
				for(i=0;i<MAX_SCE_CPU_CNT;i++){
					if((unsigned char)limit <=  pSceInfo->cpuInfo[i].minLimit){
						sprintf(resBuf,"    RESULT = FAIL\n    REASON = INVALID LIMIT VALUE(LESS THAN MINOR(%d))\n\n", pSceInfo->cpuInfo[i].minLimit);
								return -1;
					}
					if ((unsigned char)limit >= pSceInfo->cpuInfo[i].criLimit) {
						sprintf(resBuf,"    RESULT = FAIL\n    REASON = INVALID LIMIT VALUE(MORE THAN CRITICAL(%d))\n\n", pSceInfo->cpuInfo[i].criLimit);
								return -1;
					}
					pSceInfo->cpuInfo[i].majLimit = (unsigned char)limit;
					if(durat) pSceInfo->cpuInfo[i].majDurat = (unsigned char)durat;
				}
				break;

			case SFM_ALM_CRITICAL:
				for(i=0;i<MAX_SCE_CPU_CNT;i++){
					if((unsigned char)limit <=  pSceInfo->cpuInfo[i].majLimit){
						sprintf(resBuf,"    RESULT = FAIL\n    REASON = INVALID LIMIT VALUE(LESS THAN MAJOR(%d))\n", pSceInfo->cpuInfo[i].majLimit);
								return -1;
					}
					pSceInfo->cpuInfo[i].criLimit = (unsigned char)limit;
					if(durat) pSceInfo->cpuInfo[i].criDurat= (unsigned char)durat;
				}
				break;	
			}
			break;


		/*MEM INFO*/
		case SFM_ALM_TYPE_MEMORY_USAGE:

			switch(level){
			case SFM_ALM_MINOR:
				for(i=0;i<MAX_SCE_MEM_CNT;i++){
					if((unsigned char)limit >=  pSceInfo->memInfo[i].majLimit){
						sprintf(resBuf,"    RESULT = FAIL\n    REASON = INVALID LIMIT VALUE(MORE THAN MAJOR(%d))\n\n", pSceInfo->memInfo[i].majLimit);
								return -1;
					}
					pSceInfo->memInfo[i].minLimit = (unsigned char)limit;
					if(durat) pSceInfo->memInfo[i].minDurat = (unsigned char)durat;
				}
				break;

			case SFM_ALM_MAJOR:
				for(i=0;i<MAX_SCE_MEM_CNT;i++){
					if((unsigned char)limit <=  pSceInfo->memInfo[i].minLimit){
						sprintf(resBuf,"    RESULT = FAIL\n    REASON = INVALID LIMIT VALUE(LESS THAN MINOR(%d))\n\n", pSceInfo->memInfo[i].minLimit);
								return -1;
					}
					if ((unsigned char)limit >= pSceInfo->memInfo[i].criLimit) {
						sprintf(resBuf,"    RESULT = FAIL\n    REASON = INVALID LIMIT VALUE(MORE THAN CRITICAL(%d))\n\n", pSceInfo->memInfo[i].criLimit);
								return -1;
					}
					pSceInfo->memInfo[i].majLimit = (unsigned char)limit;
					if(durat) pSceInfo->memInfo[i].majDurat = (unsigned char)durat;
				}
				break;

			case SFM_ALM_CRITICAL:
				for(i=0;i<MAX_SCE_MEM_CNT;i++){
					if((unsigned char)limit <=  pSceInfo->memInfo[i].majLimit){
						sprintf(resBuf,"    RESULT = FAIL\n    REASON = INVALID LIMIT VALUE(LESS THAN MAJOR(%d))\n", pSceInfo->memInfo[i].majLimit);
								return -1;
					}
					pSceInfo->memInfo[i].criLimit = (unsigned char)limit;
					if(durat) pSceInfo->memInfo[i].criDurat = (unsigned char)durat;
				}
				break;	
			}
			break;

		/*DISK INFO*/
		case SFM_ALM_TYPE_DISK_USAGE:

			switch(level){
			case SFM_ALM_MINOR:
				if((unsigned char)limit >=  pSceInfo->diskInfo.majLimit){
						sprintf(resBuf,"    RESULT = FAIL\n    REASON = INVALID LIMIT VALUE(MORE THAN MAJOR(%d))\n\n", pSceInfo->diskInfo.majLimit);
								return -1;
				}
				pSceInfo->diskInfo.minLimit = (unsigned char)limit;
				if(durat) pSceInfo->diskInfo.minDurat = (unsigned char)durat;
				break;

			case SFM_ALM_MAJOR:
				if((unsigned char)limit <=  pSceInfo->diskInfo.minLimit){
						sprintf(resBuf,"    RESULT = FAIL\n    REASON = INVALID LIMIT VALUE(LESS THAN MINOR(%d))\n\n", pSceInfo->diskInfo.minLimit);
								return -1;
				}
				if ((unsigned char)limit >= pSceInfo->diskInfo.criLimit) {
						sprintf(resBuf,"    RESULT = FAIL\n    REASON = INVALID LIMIT VALUE(MORE THAN CRITICAL(%d))\n\n", pSceInfo->diskInfo.criLimit);
								return -1;
				}
				pSceInfo->diskInfo.majLimit = (unsigned char)limit;
				if(durat) pSceInfo->diskInfo.majDurat = (unsigned char)durat;
				break;

			case SFM_ALM_CRITICAL:
				if((unsigned char)limit <=  pSceInfo->diskInfo.majLimit){
					sprintf(resBuf,"    RESULT = FAIL\n    REASON = INVALID LIMIT VALUE(LESS THAN MAJOR(%d))\n", pSceInfo->diskInfo.majLimit);
							return -1;
				}
				pSceInfo->diskInfo.criLimit = (unsigned char)limit;
				if(durat) pSceInfo->diskInfo.criDurat = (unsigned char)durat;
				break;	
			}
			break;
		/* hjjung */
		/*USER INFO*/
		case SFM_ALM_TYPE_USER_USAGE:

			switch(level){
			case SFM_ALM_MINOR:
				if(limit >=  pSceInfo->userInfo.majLimit){
						sprintf(resBuf,"    RESULT = FAIL\n    REASON = INVALID LIMIT VALUE(MORE THAN MAJOR(%d))\n\n", pSceInfo->userInfo.majLimit);
								return -1;
				}
				pSceInfo->userInfo.minLimit = limit;
				if(durat) pSceInfo->userInfo.minDurat = (unsigned char)durat;
				break;

			case SFM_ALM_MAJOR:
				if(limit <=  pSceInfo->userInfo.minLimit){
						sprintf(resBuf,"    RESULT = FAIL\n    REASON = INVALID LIMIT VALUE(LESS THAN MINOR(%d))\n\n", pSceInfo->userInfo.minLimit);
								return -1;
				}
				if (limit >= pSceInfo->userInfo.criLimit) {
						sprintf(resBuf,"    RESULT = FAIL\n    REASON = INVALID LIMIT VALUE(MORE THAN CRITICAL(%d))\n\n", pSceInfo->userInfo.criLimit);
								return -1;
				}
				pSceInfo->userInfo.majLimit = limit;
				if(durat) pSceInfo->userInfo.majDurat = (unsigned char)durat;
				break;

			case SFM_ALM_CRITICAL:
				if(limit <=  pSceInfo->userInfo.majLimit){
					sprintf(resBuf,"    RESULT = FAIL\n    REASON = INVALID LIMIT VALUE(LESS THAN MAJOR(%d))\n", pSceInfo->userInfo.majLimit);
							return -1;
				}
				pSceInfo->userInfo.criLimit = limit;
				printf("limit2 >>> %d\n", pSceInfo->userInfo.criLimit);
				if(durat) pSceInfo->userInfo.criDurat = (unsigned char)durat;
				break;	
			}
			break;


	}/*End of switch*/

	alm_lmt_sce_input();	

	return 0;
}





