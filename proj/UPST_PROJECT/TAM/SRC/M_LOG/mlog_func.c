/**		@file	mlog_log.c
 * 		- M_LOG에서 LOG 포맷을 변경처리하는 소스
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: mlog_func.c,v 1.1 2011/08/31 13:13:41 dcham Exp $
 *
 * 		@Author		$Author: dcham $
 * 		@version	$Revision: 1.1 $
 * 		@date		$Date: 2011/08/31 13:13:41 $
 * 		@ref		mlog_init.c mlog_maic.c
 *
 * 		@section	Intro(소개)
 * 		- M_LOG에서 LOG 포맷을 변경처리하는 소스
 *
 * 		@section	Requirement
 *
 **/

#include <stdio.h>
#include <errno.h>
#include <signal.h>

#include "loglib.h"
#include "mlog_func.h"

#ifdef IMSI2MIN
int dImsi2Min(char *inimsi, char *outmin);
#endif

/** dConvertHTTPtoRTSP function.
 *
 *  dConvertHTTPtoRTSP Function
 *
 *  @return			S32
 *  @see			cilog_init.c cilog_main.c
 *
 **/

extern stHASHOINFO   *pDEFECTINFO;

S32 dConvertHTTPtoRTSP(LOG_HTTP_TRANS *pHTTP, LOG_RTSP_TRANS *pRTSP)
{
	pRTSP->uiCallTime 			= pHTTP->uiCallTime;
	pRTSP->uiCallMTime 			= pHTTP->uiCallMTime;
	pRTSP->uiAccStartTime		= pHTTP->uiAccStartTime;
	pRTSP->uiAccStartMTime		= pHTTP->uiAccStartMTime;
	memcpy(pRTSP->szModel, pHTTP->szModel, MAX_MODEL_SIZE); 
	pRTSP->uiNASName 			= pHTTP->uiNASName;
	pRTSP->ucFA_ID				= pHTTP->ucFA_ID;
	pRTSP->ucSECTOR				= pHTTP->ucSECTOR;
	pRTSP->ucSYSID				= pHTTP->ucSYSID;
	pRTSP->ucBSCID				= pHTTP->ucBSCID;
	pRTSP->ucBTSID				= pHTTP->ucBTSID;
	memcpy(pRTSP->szBSMSC, pHTTP->szBSMSC, DEF_BSMSD_LENGTH);
	memcpy(pRTSP->szMIN, pHTTP->szMIN, MAX_MIN_SIZE);
	memcpy(pRTSP->szNetOption, pHTTP->szNetOption, MAX_SVCOPTION_SIZE);
	memcpy(pRTSP->szBrowserInfo, pHTTP->szBrowserInfo, MAX_BROWSERINFO_SIZE);
	pRTSP->uiClientIP 			= pHTTP->uiClientIP;
	memcpy(pRTSP->szIMSI, pHTTP->szIMSI, MAX_MIN_SIZE);
	pRTSP->usServiceType 		= pHTTP->usServiceType; 
	memcpy(pRTSP->szHostName, pHTTP->szHostName, MAX_HOSTNAME_SIZE);
	pRTSP->uiPCFIP 				= pHTTP->uiPCFIP; 

	pRTSP->usClientPort 		= pHTTP->usClientPort;
	pRTSP->uiServerIP 			= pHTTP->uiServerIP;
	pRTSP->usServerPort 		= pHTTP->usServerPort;
	pRTSP->uiTcpSynTime 		= pHTTP->uiTcpSynTime;
	pRTSP->uiTcpSynMTime 		= pHTTP->uiTcpSynMTime;
	pRTSP->usTransID 			= pHTTP->usTransID;
	pRTSP->uiPageID 			= pHTTP->uiPageID;
	pRTSP->usPlatformType 		= pHTTP->usPlatformType;
    pRTSP->usSvcL4Type 			= pHTTP->usSvcL4Type;
	pRTSP->usSvcL7Type 			= pHTTP->usSvcL7Type;
	pRTSP->ucSubSysNo 			= pHTTP->ucSubSysNo;        
	pRTSP->usContentsType 		= pHTTP->usContentsType;   
	memcpy(pRTSP->szContentsType, pHTTP->szContentsType, MAX_CONTENTSTYPE_SIZE);   
	pRTSP->ucMethod 			= pHTTP->ucMethod;       
	pRTSP->uiReqStartTime 		= pHTTP->uiReqStartTime; 
	pRTSP->uiReqStartMTime 		= pHTTP->uiReqStartMTime;
	pRTSP->uiReqEndTime 		= pHTTP->uiReqEndTime;  
	pRTSP->uiReqEndMTime 		= pHTTP->uiReqEndMTime;
	pRTSP->uiReqAckTime 		= pHTTP->uiReqAckTime;      
	pRTSP->uiReqAckMTime 		= pHTTP->uiReqAckMTime;    
	pRTSP->uiResStartTime 		= pHTTP->uiResStartTime;  
	pRTSP->uiResStartMTime 		= pHTTP->uiResStartMTime;
	pRTSP->uiResEndTime 		= pHTTP->uiResEndTime;  
	pRTSP->uiResEndMTime 		= pHTTP->uiResEndMTime;
	pRTSP->uiMNAckTime 			= pHTTP->uiMNAckTime; 
	pRTSP->uiMNAckMTime 		= pHTTP->uiMNAckMTime;           
	pRTSP->uiLastPktTime 		= pHTTP->uiLastPktTime;         
	pRTSP->uiLastPktMTime 		= pHTTP->uiLastPktMTime;        
	pRTSP->llTransGapTime 		= pHTTP->llTransGapTime;        
	pRTSP->usResCode 			= pHTTP->usResCode;             
	pRTSP->ucTcpClientStatus 	= pHTTP->ucTcpClientStatus;     
	pRTSP->ucTcpServerStatus 	= pHTTP->ucTcpServerStatus;     
	pRTSP->ucStatus 			= pHTTP->ucStatus;   		
	pRTSP->usUserErrorCode 		= pHTTP->usUserErrorCode;    
	pRTSP->usL4FailCode 		= pHTTP->usL4FailCode;   	
	pRTSP->usL7FailCode 		= pHTTP->usL7FailCode;       
	pRTSP->usURLSize 			= pHTTP->usLOGURLSize;          
	memcpy(pRTSP->szURL, pHTTP->szLOGURL, MAX_LOGURL_SIZE);
	pRTSP->uiIPDataUpPktCnt 	= pHTTP->uiIPDataUpPktCnt;   
	pRTSP->uiIPDataDnPktCnt 	= pHTTP->uiIPDataDnPktCnt;   
	pRTSP->uiIPTotUpPktCnt 		= pHTTP->uiIPTotUpPktCnt;    
	pRTSP->uiIPTotDnPktCnt 		= pHTTP->uiIPTotDnPktCnt;   
	pRTSP->uiIPDataUpRetransCnt = pHTTP->uiIPDataUpRetransCnt;   
	pRTSP->uiIPDataDnRetransCnt = pHTTP->uiIPDataDnRetransCnt;   
	pRTSP->uiIPTotUpRetransCnt 	= pHTTP->uiIPTotUpRetransCnt;    
	pRTSP->uiIPTotDnRetransCnt 	= pHTTP->uiIPTotDnRetransCnt;    
	pRTSP->uiIPDataUpPktSize 	= pHTTP->uiIPDataUpPktSize;      
	pRTSP->uiIPDataDnPktSize 	= pHTTP->uiIPDataDnPktSize;      
	pRTSP->uiIPTotUpPktSize 	= pHTTP->uiIPTotUpPktSize;       
	pRTSP->uiIPTotDnPktSize 	= pHTTP->uiIPTotDnPktSize;       
	pRTSP->uiIPDataUpRetransSize= pHTTP->uiIPDataUpRetransSize;  
	pRTSP->uiIPDataDnRetransSize= pHTTP->uiIPDataDnRetransSize;  
	pRTSP->uiIPTotUpRetransSize = pHTTP->uiIPTotUpRetransSize;   
	pRTSP->uiIPTotDnRetransSize = pHTTP->uiIPTotDnRetransSize;   
	pRTSP->uiTcpUpBodySize 		= pHTTP->uiTcpUpBodySize;        
	pRTSP->uiTcpDnBodySize 		= pHTTP->uiTcpDnBodySize;        
	pRTSP->uiUpHeaderSize 		= pHTTP->uiUpHeaderSize;        
	pRTSP->uiDnHeaderSize 		= pHTTP->uiDnHeaderSize;       
	pRTSP->uiUpBodySize 		= pHTTP->uiUpBodySize;         
	pRTSP->uiDnBodySize 		= pHTTP->uiDnBodySize;         
	memcpy(pRTSP->szMenuTitle, pHTTP->szMenuTitle, MAX_MENUTITLE_SIZE);  
	memcpy(pRTSP->CATID, pHTTP->CATID, MAX_CATID_SIZE);    
	pRTSP->uiOpStartTime 		= pHTTP->uiOpStartTime;         
	pRTSP->uiOpStartMTime 		= pHTTP->uiOpStartMTime;        
	pRTSP->uiOpEndTime 			= pHTTP->uiOpEndTime;           
	pRTSP->uiOpEndMTime 		= pHTTP->uiOpEndMTime;          
	return 0;
} 


/*
 * hasho 에서 threshold값을 읽어오기 위한 함수.
 * created by uamyd 20100929
 */
U32 dGetThreshold()
{
    stHASHONODE     *pHASHONODE;
    //HData_DEFECT    *pstDefectHash;

	SERVICE_TYPE    SvcType=SERVICE_DIAMETER;

    if ( (pHASHONODE = hasho_find(pDEFECTINFO, (U8*)&SvcType)) )
    {
        //pstDefectHash = (HData_DEFECT*)nifo_ptr(pDEFECTINFO, pHASHONODE->offset_Data);
		//return pstDefectHash->uiResponseTime;
        return ((HData_DEFECT*)nifo_ptr(pDEFECTINFO, pHASHONODE->offset_Data)) -> uiResponseTime;
    }
    return 0;
}

/** dConvertSIGNALtoDIAMETER function.
 *
 *  dConvertSIGNALtoDIAMETER Function
 *
 *  @return			S32
 *  @see			cilog_init.c cilog_main.c
 *
 **/

S32 dConvertSIGNALtoDIAMETER(LOG_SIGNAL *pSig, DB_LOG_DIAMETER *pDia)
{
	int dErrorFlag=0, dSvcThreshold;

	/* common */
	pDia->uiCallTime 			= pSig->uiCallTime;
	pDia->uiCallMTime 			= pSig->uiCallMTime;
	pDia->uiAccStartTime		= pSig->uiAccStartTime;
	pDia->uiAccStartMTime		= pSig->uiAccStartMTime;
	memcpy(pDia->szModel, pSig->szModel, MAX_MODEL_SIZE); 
	pDia->uiNASName 			= pSig->uiNASName;
	pDia->ucFA_ID				= pSig->ucFA_ID;
	pDia->ucSECTOR				= pSig->ucSECTOR;
	pDia->ucSYSID				= pSig->ucSYSID;
	pDia->ucBSCID				= pSig->ucBSCID;
	pDia->ucBTSID				= pSig->ucBTSID;
	memcpy(pDia->szBSMSC, pSig->szBSMSC, DEF_BSMSD_LENGTH);
	memcpy(pDia->szMIN, pSig->szMIN, MAX_MIN_SIZE);
	memcpy(pDia->szNetOption, pSig->szNetOption, MAX_SVCOPTION_SIZE);
	memcpy(pDia->szBrowserInfo, pSig->szBrowserInfo, MAX_BROWSERINFO_SIZE);
	pDia->uiClientIP 			= pSig->uiClientIP;
	memcpy(pDia->szIMSI, pSig->szIMSI, MAX_MIN_SIZE);
	pDia->usServiceType 		= pSig->usServiceType; 
	memcpy(pDia->szHostName, pSig->szHostName, MAX_HOSTNAME_SIZE);
	pDia->uiPCFIP 				= pSig->uiPCFIP; 

	pDia->uiOpStartTime 		= pSig->uiSessStartTime;         
	pDia->uiOpStartMTime 		= pSig->uiSessStartMTime;        
	pDia->uiOpEndTime 			= pSig->uiSessEndTime;           
	pDia->uiOpEndMTime 			= pSig->uiSessEndMTime;          

	log_print (LOGN_INFO, "GET DIAMETER LOG IMSI[%s] PROTOTYPE[%d] MSGTYPE[%d]",
                    pDia->szIMSI, pSig->uiProtoType, pSig->uiMsgType);

	/* diameter */

	/* get Threshold value */
	dSvcThreshold = dGetThreshold();

	/* Error Check */
	if( pSig->uiLastUserErrCode == 3 ){
		pDia->uiLastFailReason = (DIAMETER_DEFECT + DIAMETER_CMD_DEFECT + pSig->uiResultCode);
		dErrorFlag = 1;
	}else if( pSig->uiLastUserErrCode != 0 ){
		pDia->uiLastFailReason = (DIAMETER_DEFECT + DIAMETER_CMD_DEFECT + pSig->uiLastUserErrCode);
		dErrorFlag = 1;
	}

	if( dSvcThreshold > 0 && pSig->uiSessDuration > dSvcThreshold*1000 ){
		pDia->uiLastFailReason = (DIAMETER_DEFECT + SERVICE_DELAY_DEFECT + RESPONSETIME);
		dErrorFlag = 1;
	}

	pDia->uiCSCFIP	= pSig->uiSrcIP;
    pDia->uiHSSIP	= pSig->uiDestIP;

	switch(pSig->uiMsgType){
		case USER_AUTHORIZATION_TRANS:
			log_print(LOGN_INFO,"MSGTYPE> USER_AUTHORIZATION_TRANS");
			pDia->usUARReqCnt 		= 1;
			if( dErrorFlag != 0 ) 	pDia->usUARSuccRepCnt = 0;
			else					pDia->usUARSuccRepCnt = 1;

			pDia->uiUARStartTime	= pSig->uiSessStartTime;
			pDia->uiUARStartMTime	= pSig->uiSessStartMTime;
			pDia->uiUAREndTime		= pSig->uiSessEndTime;
			pDia->uiUAREndMTime		= pSig->uiSessEndMTime;

			break;

		case SERVER_ASSIGNMENT_TRANS:
			log_print(LOGN_INFO,"MSGTYPE> SERVER_ASSIGNMENT_TRANS");
			pDia->usSARReqCnt 		= 1;
			if( dErrorFlag != 0 )	pDia->usSARSuccRepCnt = 0;
			else 					pDia->usSARSuccRepCnt = 1;

			pDia->uiSARStartTime	= pSig->uiSessStartTime;
			pDia->uiSARStartMTime	= pSig->uiSessStartMTime;
			pDia->uiSAREndTime		= pSig->uiSessEndTime;
			pDia->uiSAREndMTime		= pSig->uiSessEndMTime;

			break;

		case LOCATION_INFO_TRANS:
			log_print(LOGN_INFO,"MSGTYPE> LOCATION_INFO_TRANS");
			pDia->usLIRReqCnt 		= 1;
			if( dErrorFlag != 0 )	pDia->usLIRSuccRepCnt = 0;
			else					pDia->usLIRSuccRepCnt = 1;

			pDia->uiLIRStartTime	= pSig->uiSessStartTime;
			pDia->uiLIRStartMTime	= pSig->uiSessStartMTime;
			pDia->uiLIREndTime		= pSig->uiSessEndTime;
			pDia->uiLIREndMTime		= pSig->uiSessEndMTime;

			break;
		
		case MULTIMEDIA_AUTH_TRANS:
			log_print(LOGN_INFO,"MSGTYPE> MULTIMEDIA_AUTH_TRANS");
			pDia->usMARReqCnt 		= 1;
			if( dErrorFlag != 0 )	pDia->usMARSuccRepCnt = 0;
			else					pDia->usMARSuccRepCnt = 1;

			pDia->uiMARStartTime	= pSig->uiSessStartTime;
			pDia->uiMARStartMTime	= pSig->uiSessStartMTime;
			pDia->uiMAREndTime		= pSig->uiSessEndTime;
			pDia->uiMAREndMTime		= pSig->uiSessEndMTime;

			break;

		case REGISTRATION_TERMINATION_TRANS:
			log_print(LOGN_INFO,"MSGTYPE> REGISTRATION_TERMINATION_TRANS");
			pDia->usRTRReqCnt 		= 1;
			if( dErrorFlag != 0 ) 	pDia->usRTRSuccRepCnt = 0;
			else				 	pDia->usRTRSuccRepCnt = 1;

			pDia->uiRTRStartTime	= pSig->uiSessStartTime;
			pDia->uiRTRStartMTime	= pSig->uiSessStartMTime;
			pDia->uiRTREndTime		= pSig->uiSessEndTime;
			pDia->uiRTREndMTime		= pSig->uiSessEndMTime;

			break;
	
		case PUSH_PROFILE_TRANS:
			log_print(LOGN_INFO,"MSGTYPE> PUSH_PROFILE_TRANS");
			pDia->usPPRReqCnt 		= 1;
			if( dErrorFlag != 0 ) 	pDia->usPPRSuccRepCnt = 0;
			else 					pDia->usPPRSuccRepCnt = 1;

			pDia->uiPPRStartTime	= pSig->uiSessStartTime;
			pDia->uiPPRStartMTime	= pSig->uiSessStartMTime;
			pDia->uiPPREndTime		= pSig->uiSessEndTime;
			pDia->uiPPREndMTime		= pSig->uiSessEndMTime;

			break;

		case USER_DATA_TRANS:
		case PROFILE_UPDATE_TRANS:
		case SUBSCRIBE_NOTIFICATIONS_TRANS:
		case PUSH_NOTIFICATION_TRANS:
		case BOOSTRAPPING_INFO_TRANS:
		case MESSAGE_PROCES_TRANS:
		case ACCOUNTING_REQUEST_TRANS:
		case DEVICE_WATCHDOG_TRANS:
			log_print(LOGN_INFO,"DIAMETER MSGTYPE=%d",pSig->uiMsgType);
			pDia->usDiaReqCnt		= 1;
			if( dErrorFlag != 0 ) 	pDia->usDiaSuccRepCnt = 0;
			else 					pDia->usDiaSuccRepCnt = 1;

			STG_DiffTIME64(pSig->uiSessEndTime, pSig->uiSessEndMTime, 
						pSig->uiSessStartTime, pSig->uiSessStartMTime, &pDia->llDiaSuccSumTime );
			break;
		default:
			log_print(LOGN_WARN,"UNKNOWN DIAMETER MSGTYPE(%d)",pSig->uiMsgType);
			break;
	}

	return 0;
}
 
/** CILOG_LOG_WEB function.
 *
 *  Printing Function for CILOG
 *
 * @param *fp 			: file description
 * @param *pthis 		: LOG_WEB type's pointer
 *
 *  @return 	void
 *  @see    	common_stg.h
 *
 *  @exception  nothing
 *  @note 		This is used for logging to the file.
 **/
void CILOG_LOG_WEB_P1(FILE *fp, LOG_RPPI *pthis)
{
	{	/** 	STIME       uiCallTime;  : HIDDEN = NO */		/* index : 0 */

		S8 STG_PrintPre[1024]; S8 tmp[1024]; time_t ttCallTime = pthis->uiCallTime; if(pthis->uiCallTime){ strftime(STG_PrintPre, 1024, "%Y%m%d%H%M%S", localtime((time_t *)&ttCallTime)); sprintf(tmp, "\t%u", pthis->uiCallTime); strcat(STG_PrintPre, tmp); } else { sprintf(STG_PrintPre,"0\t0"); }
		FILEPRINT(fp,"%s	",  (STG_PrintPre) );

	}
	{	/** 	MTIME       uiCallMTime;  : HIDDEN = NO */		/* index : 1 */


		FILEPRINT(fp,"%d	", (S32) (pthis->uiCallMTime) );

	}
	{	/** 	STRING      szModel[MAX_MODEL_SIZE];  : HIDDEN = NO */		/* index : 2 */


		FILEPRINT(fp,"%s	",  (pthis->szModel) );

	}
	{	/** 	STRING      szBSMSC[DEF_BSMSD_LENGTH];  : HIDDEN = NO */		/* index : 3 */


		FILEPRINT(fp,"%s	",  (pthis->szBSMSC) );

	}
	{	/** 	STRING      szMIN[MAX_MIN_SIZE];  : HIDDEN = NO */		/* index : 4 */

#ifdef IMSI2MIN
		int		dRet;

		if(pthis->szMIN[0] == '\0')
		{
			dRet = dImsi2Min(pthis->szIMSI, pthis->szMIN);
			if(dRet > 0){
				FILEPRINT(fp,"%s	",  (pthis->szMIN) );
			}
		}
		else{
			FILEPRINT(fp,"%s	",  (pthis->szMIN) );
		}
#else
		/* TODO: MIN이 없을 경우 IMSI를 MIN으로 변환하여 저장한다. */
		FILEPRINT(fp,"%s	",  (pthis->szMIN) );
#endif

	}
	{	/** 	IP4         uiClientIP;  : HIDDEN = NO */		/* index : 5 */

		S8 STG_PrintPre[1024]; S8 tmp[1024]; sprintf(STG_PrintPre, "%d.%d.%d.%d", HIPADDR(pthis->uiClientIP)); sprintf(tmp, "\t%u", pthis->uiClientIP); strcat(STG_PrintPre, tmp);
		FILEPRINT(fp,"%s	",  (STG_PrintPre) );

	}
	{	/** 	STIME       uiReleaseTime;  : HIDDEN = NO */		/* index : 6 */

		S8 STG_PrintPre[1024]; S8 tmp[1024]; time_t ttCallTime = pthis->uiReleaseTime; if(pthis->uiReleaseTime){ strftime(STG_PrintPre, 1024, "%Y%m%d%H%M%S", localtime((time_t *)&ttCallTime)); sprintf(tmp, "\t%u", pthis->uiReleaseTime); strcat(STG_PrintPre, tmp); } else { sprintf(STG_PrintPre,"0\t0"); }
		FILEPRINT(fp,"%s	",  (STG_PrintPre) );

	}
	{	/** 	MTIME       uiReleaseMTime;  : HIDDEN = NO */		/* index : 7 */


		FILEPRINT(fp,"%d	", (S32) (pthis->uiReleaseMTime) );

	}
	{	/** 	STIME       uiOpEndTime;  : HIDDEN = NO */		/* index : 8 */

		S8 STG_PrintPre[1024]; S8 tmp[1024]; time_t ttCallTime = pthis->uiOpEndTime; if(pthis->uiOpEndTime){ strftime(STG_PrintPre, 1024, "%Y%m%d%H%M%S", localtime((time_t *)&ttCallTime)); sprintf(tmp, "\t%u", pthis->uiOpEndTime); strcat(STG_PrintPre, tmp); } else { sprintf(STG_PrintPre,"0\t0"); }
		FILEPRINT(fp,"%s	",  (STG_PrintPre) );

	}
	{	/** 	MTIME       uiOpEndMTime;  : HIDDEN = NO */		/* index : 9 */


		FILEPRINT(fp,"%d	", (S32) (pthis->uiOpEndMTime) );

	}

	FILEPRINT(fp, "\n");
}

#ifdef IMSI2MIN
int dImsi2Min(char *inimsi, char *outmin)
{
	int     len;

	/* 데이터 체크 */
	if( (inimsi[0] == '\0') || ((len = strlen(inimsi)) < 15))
		return -1;

	/* 국번 4자리 */
	if(inimsi[5] == '1'){
		outmin[0] = '0';
		memcpy(&outmin[1], &inimsi[5], 2);
		outmin[3] = '0';
		memcpy(&outmin[4], &inimsi[7], 8);
	}
	/* 국번 3자리 */
	else if(inimsi[5] == '0'){
		memcpy(&outmin[0], &inimsi[5], 3);
		outmin[3] = outmin[4] = '0';
		memcpy(&outmin[5], &inimsi[8], 7);
	}
	else{
		return -2;
	}

	outmin[12] = '\0';

	return 1;
}

#endif



/*
 * $Log: mlog_func.c,v $
 * Revision 1.1  2011/08/31 13:13:41  dcham
 * M_LOG added
 *
 * Revision 1.8  2011/05/27 12:26:57  jsyoon
 * *** empty log message ***
 *
 * Revision 1.7  2011/04/30 15:28:07  innaei
 * *** empty log message ***
 *
 * Revision 1.6  2011/04/20 11:04:23  dark264sh
 * M_LOG: complie 오류 수정
 *
 * Revision 1.5  2011/04/19 12:37:20  jsyoon
 * *** empty log message ***
 *
 * Revision 1.4  2011/02/15 02:54:55  jsyoon
 * *** empty log message ***
 *
 * Revision 1.3  2011/01/11 04:09:17  uamyd
 * modified
 *
 * Revision 1.5  2010/11/14 10:22:44  jwkim96
 * STP 작업 내용 반영.
 *
 * Revision 1.4  2010/09/29 06:50:05  uamyd
 * added lastFailReason to LOG_DIAMETER
 *
 * Revision 1.3  2010/09/29 02:25:50  uamyd
 * variable name changed
 *
 * Revision 1.2  2010/09/29 00:53:55  uamyd
 * added convertLogSignalToLogDiameter
 *
 * Revision 1.1.1.1  2010/08/23 01:13:11  uamyd
 * DQMS With TOTMON, 2nd-import
 *
 * Revision 1.2  2009/06/13 11:19:31  dark264sh
 * M_LOG LOG_VT_SESS 처리
 *
 * Revision 1.1  2009/06/10 23:52:59  dark264sh
 * *** empty log message ***
 *
 */
