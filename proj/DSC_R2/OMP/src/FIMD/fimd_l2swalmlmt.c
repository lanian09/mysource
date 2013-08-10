#include "fimd_proto.h"

extern SFM_L2Dev   *g_pstL2Dev;;
extern char resBuf[4096], resHead[1024];

int checkValidForl2sw( char argSys[32],char argType[32],char argLevel[32],int limit,int durat );
int fimd_mmc_updateL2swAlmClsValue (int type, int level, int limit, int durat, void *ptr);
/*
	by sjjeon
	L2 switch set alarm limit
*/

int fimd_mmc_l2sw_set_alm_lmt( IxpcQMsgType *rxIxpcMsg, char argSys[32],char argType[32],
		                                char argLevel[32],int limit,int durat, int sysIndex )
{
	char tmpBuf[256];
	int type, level;
	if( checkValidForl2sw( argSys,argType,argLevel,limit,durat ) < 0 ) { 
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


	if (!strcasecmp(argLevel, "MINOR")) 
		level = SFM_ALM_MINOR;
	else if (!strcasecmp(argLevel, "MAJOR")) 
		level = SFM_ALM_MAJOR;
	else if (!strcasecmp(argLevel, "CRITICAL"))
		level = SFM_ALM_CRITICAL;


	switch (type)
	{
		case SFM_ALM_TYPE_CPU_USAGE:
			fimd_mmc_updateL2swAlmClsValue (type, level, limit, durat,(void*)&g_pstL2Dev->l2Info[sysIndex].cpuInfo);
			break;

		case SFM_ALM_TYPE_MEMORY_USAGE:
			fimd_mmc_updateL2swAlmClsValue (type, level, limit, durat,(void*)&g_pstL2Dev->l2Info[sysIndex].memInfo);
			break;
	}

	fimd_txMMLResult (rxIxpcMsg, resBuf, 0, 0, 0, 0, 1);

	fimd_updateL2SWlmInfo(sysIndex);
	fimd_broadcastAlmEvent2Client();
	return 1;

}

/*

*/
int checkValidForl2sw( char argSys[32],char argType[32],char argLevel[32],int limit,int durat )
{
	int result = -1;
	if ( !strcasecmp( argSys, "L2SWA" ) || !strcasecmp( argSys, "L2SWB" ) )
		if( !strcasecmp( argType, "CPU") || !strcasecmp( argType, "MEM" ) )
			if( !strcasecmp( argLevel, "MINOR") || !strcasecmp( argLevel, "MAJOR" ) || 
					!strcasecmp( argLevel, "CRITICAL" ) )
				if( limit>=1 && limit<=99 )
					if( durat>=0 && durat<=255 )
						return 1;

	return result;

}

/*
	by sjjeon
	alarm limit 설정체크 함수.(min, maj,cri 체크) / limit_l2sw_file update
	
*/
int fimd_mmc_updateL2swAlmClsValue (int type, int level, int limit, int durat, void *ptr)
{

	SFM_PDCpuInfo	*cpuInfo;
	SFM_PDMemInfo	*memInfo;

	switch (type)
	{
		case SFM_ALM_TYPE_CPU_USAGE:
			cpuInfo = (SFM_PDCpuInfo*)ptr;

			switch (level) {
				case SFM_ALM_MINOR:
					if ((unsigned char)limit >= cpuInfo->majLimit) {
						sprintf(resBuf,"    RESULT = FAIL\n    REASON = INVALID LIMIT VALUE(MORE THAN MAJOR(%d))\n\n", cpuInfo->majLimit);
						return -1;
					}
					cpuInfo->minLimit = (unsigned char)limit;
					if (durat) cpuInfo->minDurat = (unsigned char)durat;
					break;

				case SFM_ALM_MAJOR:
					if ((unsigned char)limit <= cpuInfo->minLimit) {
						sprintf(resBuf,"    RESULT = FAIL\n    REASON = INVALID LIMIT VALUE(LESS THAN MINOR(%d))\n\n", cpuInfo->minLimit);
						return -1;
					}
					if ((unsigned char)limit >= cpuInfo->criLimit) {
						sprintf(resBuf,"    RESULT = FAIL\n    REASON = INVALID LIMIT VALUE(MORE THAN CRITICAL(%d))\n\n", cpuInfo->criLimit);
						return -1;
					}
					cpuInfo->majLimit = (unsigned char)limit;
					if (durat) cpuInfo->majDurat = (unsigned char)durat;
					break;

				case SFM_ALM_CRITICAL:
					if ((unsigned char)limit <= cpuInfo->majLimit) {
						sprintf(resBuf,"    RESULT = FAIL\n    REASON = INVALID LIMIT VALUE(LESS THAN MAJOR(%d))\n", cpuInfo->majLimit);
						return -1;
					}
					cpuInfo->criLimit = (unsigned char)limit;
					if (durat) cpuInfo->criDurat = (unsigned char)durat;
					break;
			}
			break;

		case SFM_ALM_TYPE_MEMORY_USAGE:
			memInfo = (SFM_PDMemInfo*)ptr;
			switch (level) {
				case SFM_ALM_MINOR:
					if ((unsigned char)limit >= memInfo->majLimit) {
						sprintf(resBuf,"    RESULT = FAIL\n    REASON = INVALID LIMIT VALUE(MORE THAN MAJOR(%d))\n\n", memInfo->majLimit);
						return -1;
					}
					memInfo->minLimit = (unsigned char)limit;
					if (durat) memInfo->minDurat = (unsigned char)durat;
					break;
				case SFM_ALM_MAJOR:
					if ((unsigned char)limit <= memInfo->minLimit) {
						sprintf(resBuf,"    RESULT = FAIL\n    REASON = INVALID LIMIT VALUE(LESS THAN MINOR(%d))\n\n", memInfo->minLimit);
						return -1;
					}
					if ((unsigned char)limit >= memInfo->criLimit) {
						sprintf(resBuf,"    RESULT = FAIL\n    REASON = INVALID LIMIT VALUE(MORE THAN CRITICAL(%d))\n\n", memInfo->criLimit);
						return -1;
					}
					memInfo->majLimit = (unsigned char)limit;
					if (durat) memInfo->majDurat = (unsigned char)durat;
					break;
				case SFM_ALM_CRITICAL:
					if ((unsigned char)limit <= memInfo->majLimit) {
						sprintf(resBuf,"    RESULT = FAIL\n    REASON = INVALID LIMIT VALUE(LESS THAN MAJOR(%d))\n", memInfo->majLimit);
						return -1;
					}
					memInfo->criLimit = (unsigned char)limit;
					if (durat) memInfo->criDurat = (unsigned char)durat;
					break;
			}
			break;

	}

	alm_lmt_l2sw_input(); // by sjjeon 2009.06
	return 1;

}

/*
	by sjjeon
	L2 switch alarm limit infomation 
*/
void fimd_mmc_makeL2swAlmClsOutputMsg (
            IxpcQMsgType *rxIxpcMsg,
            int sysIndex,
            char *seqNo, int all 
            )   
{

    char    tmpBuf[256], trcBuf[25];

    // 시스템 공통 정보
    //  
    if(all == 1){ 
        if(sysIndex == 0)  
            sprintf(trcBuf, "    SYSTEM = [L2SWA]\n");
        else 
            sprintf(trcBuf, "    SYSTEM = [L2SWB]\n");
        strcat(resBuf, trcBuf);
    }   

    sprintf(tmpBuf,"    CPU          :  %-6d %-6d %-6d   %-8d %-8d %-8d\n",
            g_pstL2Dev->l2Info[sysIndex].cpuInfo.minLimit,
            g_pstL2Dev->l2Info[sysIndex].cpuInfo.majLimit,
            g_pstL2Dev->l2Info[sysIndex].cpuInfo.criLimit,
            g_pstL2Dev->l2Info[sysIndex].cpuInfo.minDurat,
            g_pstL2Dev->l2Info[sysIndex].cpuInfo.majDurat,
            g_pstL2Dev->l2Info[sysIndex].cpuInfo.criDurat);
    strcat (resBuf,tmpBuf);

    sprintf(tmpBuf,"    MEM          :  %-6d %-6d %-6d   %-8d %-8d %-8d\n",
            g_pstL2Dev->l2Info[sysIndex].memInfo.minLimit,
            g_pstL2Dev->l2Info[sysIndex].memInfo.majLimit,
            g_pstL2Dev->l2Info[sysIndex].memInfo.criLimit,
            g_pstL2Dev->l2Info[sysIndex].memInfo.minDurat,
            g_pstL2Dev->l2Info[sysIndex].memInfo.majDurat,
            g_pstL2Dev->l2Info[sysIndex].memInfo.criDurat);
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

}/* End of fimd_mmc_makeL2swAlmClsOutputMsg */
