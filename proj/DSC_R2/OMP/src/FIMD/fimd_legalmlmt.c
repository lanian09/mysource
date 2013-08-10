#include "fimd_proto.h"

/* hjjung fimd_legalmlmt.c 추가 */
//extern SFM_LEG       *g_pstLEGInfo;
extern SFM_CALL       *g_pstCALLInfo; // added by dcham 20110525

extern char resBuf[4096], resHead[1024]; 


void fimd_mmc_makeLegAlmClsOutputMsg(IxpcQMsgType *rxIxpcMsg, int sysIndex, char *seqNo, int all);
int fimd_mmc_leg_set_alm_lmt(IxpcQMsgType *rxIxpcMsg, char argSys[32], char argType[32], 
							char argLevel[32], int limit, int durat, int sysIndex );
int checkValidForLeg(char argSys[32],char argType[32],char argLevel[32],int limit,int durat );
int fimd_mmc_updateLegAlmClsValue (int type, int level, int limit, int durat, int sysIndex);
int checkValidForCall(char argSys[32],char argType[32],char argLevel[32],int limit,int durat ); // added by dcham 20110525
int fimd_mmc_updateTPSAlmClsValue (int type, int level, int limit, int durat, int sysIndex); // added by dcham 20110525

void fimd_mmc_makeLegAlmClsOutputMsg(
			IxpcQMsgType *rxIxpcMsg,
			int sysIndex,
			char *seqNo, int all
			)
{
	char	tmpBuf[512], trcBuf[25];

	// 시스템 공통 정보
	//
	if(all == 1){
		if(sysIndex == 0) 
			sprintf(trcBuf, "    SYSTEM = [SCMA]\n");
		else 
			sprintf(trcBuf, "    SYSTEM = [SCMB]\n");
		strcat(resBuf, trcBuf);
	}

	memset(tmpBuf,0x00,sizeof(tmpBuf));
#if 0
	sprintf(tmpBuf,"    SESSION         :  %-6d %-6d %-6d   %-8d %-8d %-8d\n",
			g_pstCALLInfo->legInfo[sysIndex].minLimit,
			g_pstCALLInfo->legInfo[sysIndex].majLimit,
			g_pstCALLInfo->legInfo[sysIndex].criLimit,
			g_pstCALLInfo->legInfo[sysIndex].minDurat, 
			g_pstCALLInfo->legInfo[sysIndex].majDurat,
			g_pstCALLInfo->legInfo[sysIndex].criDurat);
#else
	sprintf(tmpBuf,"    RLEG         :  %-6d %-6d %-6d   %-8d %-8d %-8d\n",
			g_pstCALLInfo->legInfo[sysIndex].minLimit,
			g_pstCALLInfo->legInfo[sysIndex].majLimit,
			g_pstCALLInfo->legInfo[sysIndex].criLimit,
			g_pstCALLInfo->legInfo[sysIndex].minDurat,
			g_pstCALLInfo->legInfo[sysIndex].majDurat,
			g_pstCALLInfo->legInfo[sysIndex].criDurat);
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

/* added by dcham 20110525 */
void fimd_mmc_makeCallAlmClsOutputMsg(
			IxpcQMsgType *rxIxpcMsg,
			int sysIndex,
			char *seqNo, int all
			)
{
	char	tmpBuf[512], trcBuf[25];

	// 시스템 공통 정보
	//
	if(all == 1){
		if(sysIndex == 0) 
			sprintf(trcBuf, "    SYSTEM = [SCMA]\n");
		else 
			sprintf(trcBuf, "    SYSTEM = [SCMB]\n");
		strcat(resBuf, trcBuf);
	}

	memset(tmpBuf,0x00,sizeof(tmpBuf));

	sprintf(tmpBuf,"    TPS          :  %-6d %-6d %-6d   %-8d %-8d %-8d\n",
			g_pstCALLInfo->tpsInfo[sysIndex].minLimit,
			g_pstCALLInfo->tpsInfo[sysIndex].majLimit,
			g_pstCALLInfo->tpsInfo[sysIndex].criLimit,
			g_pstCALLInfo->tpsInfo[sysIndex].minDurat,
			g_pstCALLInfo->tpsInfo[sysIndex].majDurat,
			g_pstCALLInfo->tpsInfo[sysIndex].criDurat);
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

int fimd_mmc_leg_set_alm_lmt(
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
	if( checkValidForLeg( argSys,argType,argLevel,limit,durat ) < 0 ) {
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

    if (!strcasecmp(argType, "RANA"))
        type = SFM_ALM_TYPE_SESSION_USAGE;

    if (!strcasecmp(argLevel, "MINOR"))
        level = SFM_ALM_MINOR;
    else if (!strcasecmp(argLevel, "MAJOR")) 
        level = SFM_ALM_MAJOR;
    else if (!strcasecmp(argLevel, "CRITICAL")) 
        level = SFM_ALM_CRITICAL;

	switch (type)
	{
		case SFM_ALM_TYPE_SESSION_USAGE:
			fimd_mmc_updateLegAlmClsValue (type, level, limit, durat, sysIndex);
			break;
	}

	fimd_txMMLResult (rxIxpcMsg, resBuf, 0, 0, 0, 0, 1);

	fimd_updateLegAlmInfo();
	fimd_broadcastAlmEvent2Client();
	return 1;

}

int fimd_mmc_call_set_alm_lmt(
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
	if( checkValidForCall( argSys,argType,argLevel,limit,durat ) < 0 ) {
		sprintf(resBuf,"    RESULT = FAIL\n    REASON = INVALID PARAMETER\n");
		fimd_txMMLResult (rxIxpcMsg, resBuf, -1, 0, 0, 0, 1);
		return -1;
	}


    sprintf(resBuf,"    RESULT = SUCCESS\n");
        sprintf(tmpBuf,"    SYSTEM = %s\n", argSys);
        strcat (resBuf,tmpBuf);
        //sprintf(tmpBuf,"    TYPE   = %s\n", argType);
        //strcat (resBuf,tmpBuf);
        sprintf(tmpBuf,"    LEVEL  = %s\n", argLevel);
        strcat (resBuf,tmpBuf);
        sprintf(tmpBuf,"    LIMIT  = %d\n", limit);
        strcat (resBuf,tmpBuf);
        if (durat>0) {
            sprintf(tmpBuf,"    DURAT  = %d\n", durat);
            strcat (resBuf,tmpBuf);
        }

		//if (!strcasecmp(argType, "RANA")) 
		type = SFM_ALM_TYPE_TPS; 

    if (!strcasecmp(argLevel, "MINOR"))
        level = SFM_ALM_MINOR;
    else if (!strcasecmp(argLevel, "MAJOR")) 
        level = SFM_ALM_MAJOR;
    else if (!strcasecmp(argLevel, "CRITICAL")) 
        level = SFM_ALM_CRITICAL;

	switch (type)
	{
		case SFM_ALM_TYPE_TPS:
			fimd_mmc_updateTPSAlmClsValue (type, level, limit, durat, sysIndex);
			break;
	}

	fimd_txMMLResult (rxIxpcMsg, resBuf, 0, 0, 0, 0, 1);

	fimd_updateCallAlmInfo();
	fimd_broadcastAlmEvent2Client();
	return 1;

}

int checkValidForLeg( char argSys[32],char argType[32],char argLevel[32],int limit,int durat )
{
    int result = -1;
    if ( !strcasecmp( argSys, "SCMA" ) || !strcasecmp( argSys, "SCMB" ) )
        if( !strcasecmp( argType, "RANA") )
            if( !strcasecmp( argLevel, "MINOR") || !strcasecmp( argLevel, "MAJOR" ) || !strcasecmp( argLevel, "CRITICAL" ) )
                if( limit>=1 && limit<=999999 )
                    if( durat>=0 && durat<=255 )
                        return 1;

    return result;
}

/* added  by dcham 20110525 */
int checkValidForCall( char argSys[32],char argType[32],char argLevel[32],int limit,int durat )
{
    int result = -1;
    if ( !strcasecmp( argSys, "SCMA" ) || !strcasecmp( argSys, "SCMB" ) )
      //  if( !strcasecmp( argType, "RANA") )
            if( !strcasecmp( argLevel, "MINOR") || !strcasecmp( argLevel, "MAJOR" ) || !strcasecmp( argLevel, "CRITICAL" ) )
                if( limit>=1 && limit<=999999 )
                    if( durat>=0 && durat<=255 )
                        return 1;

    return result;
}
int fimd_mmc_updateLegAlmClsValue (int type, int level, int limit, int durat, int sysIndex)
{
	LEG_SESS_NUM_INFO *pLegInfo = NULL;
	if(sysIndex>=0 && sysIndex<2)
		pLegInfo = (LEG_SESS_NUM_INFO*)&g_pstCALLInfo->legInfo[sysIndex];
	else {
		sprintf(resBuf,"    RESULT = FAIL\n    REASON = INVALID SYSTEM TYPE : %d))\n\n", sysIndex);
	 	return -1;
	}
	
	switch(type) {

		/*SESSION INFO*/
		case SFM_ALM_TYPE_SESSION_USAGE:

			switch(level){
			case SFM_ALM_MINOR:
				if(limit >=  pLegInfo->majLimit){
						sprintf(resBuf,"    RESULT = FAIL\n    REASON = INVALID LIMIT VALUE(MORE THAN MAJOR(%d))\n\n", pLegInfo->majLimit);
								return -1;
				}
				pLegInfo->minLimit = limit;
				if(durat) pLegInfo->minDurat = (unsigned char)durat;
				break;

			case SFM_ALM_MAJOR:
				if(limit <=  pLegInfo->minLimit){
						sprintf(resBuf,"    RESULT = FAIL\n    REASON = INVALID LIMIT VALUE(LESS THAN MINOR(%d))\n\n", pLegInfo->minLimit);
								return -1;
				}
				if (limit >= pLegInfo->criLimit) {
						sprintf(resBuf,"    RESULT = FAIL\n    REASON = INVALID LIMIT VALUE(MORE THAN CRITICAL(%d))\n\n", pLegInfo->criLimit);
								return -1;
				}
				pLegInfo->majLimit = limit;
				if(durat) pLegInfo->majDurat = (unsigned char)durat;
				break;

			case SFM_ALM_CRITICAL:
				if(limit <=  pLegInfo->majLimit){
					sprintf(resBuf,"    RESULT = FAIL\n    REASON = INVALID LIMIT VALUE(LESS THAN MAJOR(%d))\n", pLegInfo->majLimit);
							return -1;
				}
				pLegInfo->criLimit = limit;
				if(durat) pLegInfo->criDurat = (unsigned char)durat;
				break;	
			}
			break;

	}/*End of switch*/

	alm_lmt_leg_input();	

	return 0;
}

int fimd_mmc_updateTPSAlmClsValue (int type, int level, int limit, int durat, int sysIndex)
{
	TPS_INFO *pTpsInfo = NULL;
	if(sysIndex>=0 && sysIndex<2)
		pTpsInfo = (TPS_INFO*)&g_pstCALLInfo->tpsInfo[sysIndex];
	else {
		sprintf(resBuf,"    RESULT = FAIL\n    REASON = INVALID SYSTEM TYPE : %d))\n\n", sysIndex);
	 	return -1;
	}
	
	switch(type) {

		/*CALL INFO*/
		case SFM_ALM_TYPE_TPS:

			switch(level){
			case SFM_ALM_MINOR:
				if(limit >=  pTpsInfo->majLimit){
						sprintf(resBuf,"    RESULT = FAIL\n    REASON = INVALID LIMIT VALUE(MORE THAN MAJOR(%d))\n\n", pTpsInfo->majLimit);
								return -1;
				}
				pTpsInfo->minLimit = limit;
				if(durat) pTpsInfo->minDurat = (unsigned char)durat;
				break;

			case SFM_ALM_MAJOR:
				if(limit <=  pTpsInfo->minLimit){
						sprintf(resBuf,"    RESULT = FAIL\n    REASON = INVALID LIMIT VALUE(LESS THAN MINOR(%d))\n\n", pTpsInfo->minLimit);
								return -1;
				}
				if (limit >= pTpsInfo->criLimit) {
						sprintf(resBuf,"    RESULT = FAIL\n    REASON = INVALID LIMIT VALUE(MORE THAN CRITICAL(%d))\n\n", pTpsInfo->criLimit);
								return -1;
				}
				pTpsInfo->majLimit = limit;
				if(durat) pTpsInfo->majDurat = (unsigned char)durat;
				break;

			case SFM_ALM_CRITICAL:
				if(limit <=  pTpsInfo->majLimit){
					sprintf(resBuf,"    RESULT = FAIL\n    REASON = INVALID LIMIT VALUE(LESS THAN MAJOR(%d))\n", pTpsInfo->majLimit);
							return -1;
				}
				pTpsInfo->criLimit = limit;
				if(durat) pTpsInfo->criDurat = (unsigned char)durat;
				break;	
			}
			break;

	}/*End of switch*/

	alm_lmt_tps_input();	

	return 0;
}
