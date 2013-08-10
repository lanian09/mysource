/**A.1*  FILE INCLUSION *******************************************************/
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <define.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <linux/limits.h>		/*	PATH_MAX	*/

//#include "lgt_nms.h"			/* LGT 관련 내용들, 주로 NMS 연관 자료 */
#include "define.h"				/* MAX_THRES */
#include "filter.h"
#include "db_define.h"
#include "db_struct.h"

#include "s_mng_func.h"
#include "s_mng_msg.h"
#include "s_mng_flt.h"

#include "common_stg.h"

#include "path.h"				/* DATA_PATH */
#include "procid.h"
#include "mems.h"
#include "cifo.h"
#include "nifo.h"
#include "gifo.h"
#include "hasho.h"

#include "db_api.h"
#include "mmcdef.h"				/* st_ConnInfo */
#include "msgdef.h"				/* st_MsgQ */
#include "sockio.h"				/* st_subsys_mng */

#include "loglib.h"
#include "utillib.h"			/* util_cvtipaddr() */
#include "nsocklib.h"			/* SIDB_DATE_SIZE */

/** B.1* DEFINITION OF NEW CONSTANTS **************************/

/** C.1* DEFINITION OF NEW TYPES **************************/

/** D.1* DECLARATION OF EXTERN VARIABLES **************************/
extern MYSQL			stMySQL;
extern st_ConnInfo		stConnInfo;
extern stMEMSINFO		*pMEMSINFO;
extern stCIFO			*gpCIFO;
extern stHASHOINFO		*pIRMINFO;
extern st_subsys_mng	*pstSubSys;

/** D.2* DECLARATION OF VARIABLES **************************/
st_TraceList			*trace_tbl;
st_Flt_Info				*flt_info;
st_Mmlsg				g_stMmlsg;
int						g_dIndex;


/** E.1* DEFINITION OF EXTERN FUNCTIONS **************************/
/*	START: MID_FLT_ALL의 Message를 보내온 TAF에게 SCTP filter 정보를 보내주기 위하여	*/
/*	END: Writer: HAN-JIN PARK, Date: 2009.05.11	*/

/** E.2* DEFINITION OF FUNCTIONS **************************/
int dReq_Flt_Thrs_Msg(int dSid, int dSysNo)
{
	int				i, j, dRet, dLoopCnt, dIdx, dCount;
	st_MsgQ			stMsgQ;
	st_ThresMMC		stThresMMC[MAX_THRES];
	st_Thres_List	stThresList;

	log_print(LOGN_DEBUG, "\n**** FLT DEFECT THRESHOLD INFORMATION ****");
	memset(stThresMMC, 0x00, sizeof(st_Thres_MMC)*MAX_THRES);
	if( (dRet = dGetThres(&stMySQL, stThresMMC, &dCount, NULL)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dGetThres() dRet[%d]", LT, dRet);
		return -1;
	}

	stThresList.dCount = dCount;
	if( (stThresList.dCount % DEF_THRES_MMC ) == 0)
		dLoopCnt = stThresList.dCount / DEF_THRES_MMC;
	else
		dLoopCnt = (stThresList.dCount/DEF_THRES_MMC)+1;

	for(i = 0; i < dLoopCnt; i++)
	{
		for(j = 0; j < DEF_THRES_MMC; j++)
		{
			stThresList.stThres[j].cSvcType			= stThresMMC[dIdx].cSvcType;
			stThresList.stThres[j].uTCPSetupTime	= stThresMMC[dIdx].uTCPSetupTime;
			stThresList.stThres[j].uResponseTime	= stThresMMC[dIdx].uResponseTime;
			stThresList.stThres[j].uUpThroughput	= stThresMMC[dIdx].uUpThroughput;
			stThresList.stThres[j].uDnThroughput	= stThresMMC[dIdx].uDnThroughput;
			stThresList.stThres[j].uUpRetransCount	= stThresMMC[dIdx].uUpRetransCount;
			stThresList.stThres[j].uDnRetransCount	= stThresMMC[dIdx].uDnRetransCount;
			stThresList.stThres[j].uUpJitter		= stThresMMC[dIdx].uUpJitter;
			stThresList.stThres[j].uDnJitter		= stThresMMC[dIdx].uDnJitter;
			stThresList.stThres[j].uUpPacketLoss	= stThresMMC[dIdx].uUpPacketLoss;
			stThresList.stThres[j].uDnPacketLoss	= stThresMMC[dIdx].uDnPacketLoss;
			sprintf(stThresList.stThres[j].szDesc, "%s", stThresMMC[dIdx].szDesc);

			stThresList.stThres[j].dIdx			= dIdx;

			log_print(LOGN_DEBUG, "idx[%d] [SEND ALL DEFECT THRS INFO] "
				"cSvcType[%hu] uTCPSetupTime[%u] uResponseTime[%u] uUpThroughput[%u] uDnThroughput[%u] uUpRetransCount[%hu] "
				"uDnRetransCount[%u] uUpJitter[%u] uDnJitter[%u] uUpPacketLoss[%u] uDnPacketLoss[%u] szDesc[%s]",
				stThresList.stThres[j].dIdx, stThresList.stThres[j].cSvcType, stThresList.stThres[j].uTCPSetupTime, stThresList.stThres[j].uResponseTime, stThresList.stThres[j].uUpThroughput,
				stThresList.stThres[j].uDnThroughput, stThresList.stThres[j].uUpRetransCount, stThresList.stThres[j].uDnRetransCount, stThresList.stThres[j].uUpJitter,
				stThresList.stThres[j].uDnJitter, stThresList.stThres[j].uUpPacketLoss, stThresList.stThres[j].uDnPacketLoss, stThresList.stThres[j].szDesc);

			if( (dIdx+1) == stThresList.dCount)
				break;

			dIdx++;
		}

		stMsgQ.usBodyLen = sizeof(st_Thres_List) + NTAFT_HEADER_LEN;
		memcpy(&stMsgQ.szBody[NTAFT_HEADER_LEN], &stThresList, sizeof(st_Thres_List));

		if( (dRet = dSend_FltMsg_To_SubSystem(&stMsgQ, MID_FLT_THRES, dSid, dSysNo)) < 0)
			return dRet;

		log_print(LOGN_DEBUG, "FLT THRESHOLD MSG SEND SUCCESS. [NTAMNO]:[%d] [TOT][%d] [CUR]:[%d]", dSysNo, stThresList.dCount, (dIdx+1));
	}

	return 0;
}

int dSend_FltMsg_To_SubSystem(pst_MsgQ pstMsgQ, int dMid, int dSid, int dSysNo)
{
	int			dRet;
	pst_MsgQSub	pstMsgQSub;

	pstMsgQ->ucNTAMID	= dSysNo;
	pstMsgQSub			= (pst_MsgQSub)&pstMsgQ->llMType;

	pstMsgQSub->usSvcID		= dSid;
	pstMsgQSub->usMsgID		= dMid;
	/**/pstMsgQ->llIndex	= g_dIndex;
	pstMsgQSub->usType		= DEF_SYS;

	pstMsgQ->ucProID		= SEQ_PROC_S_MNG;
	pstMsgQ->dMsgQID		= 0;

	if( (dRet = dSendMsg(pstMsgQ, SEQ_PROC_SI_SVC)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dSendMsg(SI_SVC[%d]) dRet[%d]", 
			LT, SEQ_PROC_SI_SVC, dRet);
		return -1;
	}

	return 0;
}

int dSend_NTAF_AlarmLevel(int dSysNo)
{
	int					dRet;
	st_AlmLevel_List	stAlm;
	st_MsgQ				stMsgQ;
	pst_MsgQSub			pstMsgQSub;

	if( (dRet = dRead_FLT_AlmLvl_NTAF(&stAlm, dSysNo)) < 0)
	{
		log_print(LOGN_CRI,LH"ERROR IN dRead_FLT_AlmLvl_NTAF(dSysNo[%d]) dRet[%d]", LT, dSysNo, dRet);
		return -1;
	}

	/* FLT ALMLVL NTAF */
	stAlm.dCount = ALM_CNT-1;

	pstMsgQSub = (pst_MsgQSub)&stMsgQ.llMType;

	pstMsgQSub->usType 	= DEF_SYS;
	pstMsgQSub->usSvcID = SID_FLT;
	pstMsgQSub->usMsgID = MID_FLT_ALM;

	stMsgQ.ucNTAFID 	= dSysNo;
	stMsgQ.usBodyLen 	= sizeof(st_AlmLevel_List) + NTAFT_HEADER_LEN;
	stMsgQ.ucProID 		= SEQ_PROC_S_MNG;
	stMsgQ.dMsgQID 		= 0;

	memcpy(&stMsgQ.szBody[NTAFT_HEADER_LEN], &stAlm, sizeof(st_AlmLevel_List));

	if( (dRet = dSendMsg(&stMsgQ, SEQ_PROC_SI_SVC)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dSendMsg(SI_SVC[%d]) dRet[%d]", 
			LT, SEQ_PROC_SI_SVC, dRet);
		return -2;
	}
	log_print(LOGN_DEBUG, LH"NTAF ALARM LEVEL SEND SUCCESS [NTAFNO]:[%d]", LT, dSysNo);

	return 0;
}

int dSendMsg_O_SVCMON(unsigned short huMsgID)
{
	int				dRet;
	st_MsgQ			stMsgQ;
	pst_MsgQSub		pstMsgQSub;

	pstMsgQSub			= (pst_MsgQSub)&stMsgQ.llMType;
	pstMsgQSub->usType	= DEF_SYS;
	pstMsgQSub->usSvcID	= SID_FLT;
	pstMsgQSub->usMsgID	= huMsgID;

	stMsgQ.usBodyLen	= 0;
	stMsgQ.ucProID		= SEQ_PROC_S_MNG;
	stMsgQ.dMsgQID		= 0;

	if( (dRet = dSendMsg(&stMsgQ, SEQ_PROC_O_SVCMON)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dSendMsg(O_SVCMON[%d]) dRet[%d]", 
			LT, SEQ_PROC_O_SVCMON, dRet);
		return -1;
	}

	return 0;
}

int dSendMsg_SIDB(char *sDate, char *sFullPath, char *sFileName)
{
	int				dRet;
	st_MsgQ			stMsgQ;
	pst_MsgQSub		pstMsgQSub;
	st_SI_DB		stSIDB;
	char			sFullFileName[PATH_MAX];
	size_t			szStrLen;

	pstMsgQSub			= (pst_MsgQSub)&stMsgQ.llMType;
	pstMsgQSub->usType	= DEF_SVC;
	pstMsgQSub->usSvcID	= SID_LOG;
	pstMsgQSub->usMsgID	= 0;

	stMsgQ.usBodyLen	= sizeof(st_SI_DB);
	stMsgQ.ucProID		= SEQ_PROC_S_MNG;
	stMsgQ.dMsgQID		= 0;

	if(strlen(sDate) > SIDB_DATE_SIZE)
	{
		strncpy(stSIDB.date, sDate, (SIDB_DATE_SIZE-1));
		stSIDB.date[SIDB_DATE_SIZE-1] = 0x00;
	}
	else
		strcpy(stSIDB.date, sDate);

	if( (szStrLen = strlen(sFileName)) > RNES_PKT_SIZE)
	{
		log_print(LOGN_CRI, LH"FileName length[%lu] is longer than RNES_PKT_SIZE[%d]", LT,
			szStrLen, RNES_PKT_SIZE);
		return -1;
	}
	else
		strcpy(stSIDB.filename, sFileName);

	memset(sFullFileName, 0x00, PATH_MAX);
	sprintf(sFullFileName, "%s/%s", sFullPath, sFileName);
	if( (szStrLen = strlen(sFullFileName)) > RNES_PKT_SIZE)
	{
		log_print(LOGN_CRI, LH"File Path length[%lu] is longer than RNES_PKT_SIZE[%d]", LT,
			szStrLen, RNES_PKT_SIZE);
		return -2;
	}
	else
		strcpy(stSIDB.name, sFullFileName);

	memcpy(stMsgQ.szBody, &stSIDB, sizeof(st_SI_DB));

	if( (dRet = dSendMsg(&stMsgQ, SEQ_PROC_SI_DB)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dSendMsg(SI_DB[%d]) dRet[%d]", 
			LT, SEQ_PROC_SI_DB, dRet);
		return -1;
	}
	log_print(LOGN_INFO, LH"SUCCESS dSendMsg(SI_DB[%d]) dRet[%d]", 
		LT, SEQ_PROC_SI_DB, dRet);

	return 0;
}

int dSendMsg_SI_SVCMON(char *sDate, unsigned char *sFullPath, unsigned char *sFileName)
{
	int				dRet;
	st_MsgQ			stMsgQ;
	pst_MsgQSub		pstMsgQSub;
	st_SI_DB		stSIDB;
	char			sFullFileName[PATH_MAX];
	size_t			szStrLen;

	pstMsgQSub			= (pst_MsgQSub)&stMsgQ.llMType;
	pstMsgQSub->usType	= DEF_SYS;
	pstMsgQSub->usSvcID	= SID_STATUS;
	pstMsgQSub->usMsgID	= MID_SVC_MONITOR_INF;

	stMsgQ.usBodyLen	= sizeof(st_SI_DB);
	stMsgQ.ucProID		= SEQ_PROC_S_MNG;
	stMsgQ.dMsgQID		= 0;

	if(strlen(sDate) > SIDB_DATE_SIZE)
	{
		strncpy(stSIDB.date, sDate, (SIDB_DATE_SIZE-1));
		stSIDB.date[SIDB_DATE_SIZE-1] = 0x00;
	}
	else
		strcpy(stSIDB.date, sDate);

	if( (szStrLen = strlen((char*)sFileName)) > RNES_PKT_SIZE)
	{
		log_print(LOGN_CRI, LH"FileName length[%lu] is longer than RNES_PKT_SIZE[%d]", LT,
			szStrLen, RNES_PKT_SIZE);
		return -1;
	}
	else
		strcpy(stSIDB.filename, (char*)sFileName);

	memset(sFullFileName, 0x00, PATH_MAX);
	sprintf(sFullFileName, "%s/%s", sFullPath, sFileName);
	if( (szStrLen = strlen(sFullFileName)) > RNES_PKT_SIZE)
	{
		log_print(LOGN_CRI, LH"File Path length[%lu] is longer than RNES_PKT_SIZE[%d]", LT,
			szStrLen, RNES_PKT_SIZE);
		return -2;
	}
	else
		strcpy(stSIDB.name, sFullFileName);

	memcpy(stMsgQ.szBody, &stSIDB, sizeof(st_SI_DB));

	if( (dRet = dSendMsg(&stMsgQ, SEQ_PROC_SI_SVCMON)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dSendMsg(SI_SVCMON[%d]) dRet[%d]", 
			LT, SEQ_PROC_SI_SVCMON, dRet);
		return -1;
	}
	log_print(LOGN_INFO, LH"SUCCESS dSendMsg(SI_SVCMON[%d]) dRet[%d]", 
		LT, SEQ_PROC_SI_SVCMON, dRet);

	return 0;
}

int dSend_FltSvc_Data(int dSysNo)
{
	int				dRet;
	st_MsgQ			stMsgQ;
	pst_MsgQSub		pstMsgQSub;

	pstMsgQSub			= (pst_MsgQSub)&stMsgQ.llMType;
	pstMsgQSub->usType	= DEF_SYS;
	pstMsgQSub->usSvcID	= SID_FLT;
	pstMsgQSub->usMsgID	= MID_FLT_SVC;

	stMsgQ.ucNTAFID		= dSysNo;
	stMsgQ.usBodyLen	= sizeof(st_ConnInfo) + NTAFT_HEADER_LEN;
	stMsgQ.ucProID		= SEQ_PROC_S_MNG;
	stMsgQ.dMsgQID		= 0;

	memcpy(&stMsgQ.szBody[NTAFT_HEADER_LEN], &stConnInfo, sizeof(st_ConnInfo));

	if( (dRet = dSendMsg(&stMsgQ, SEQ_PROC_SI_SVC)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dSendMsg(SI_SVC[%d]) dRet[%d]", 
			LT, SEQ_PROC_SI_SVC, dRet);
		return -1;
	}
	log_print(LOGN_DEBUG, "FLT SERVER IP INFORMATION SEND SUCCESS TO NTAF [NTAFNO]:[%d]", dSysNo);

	return 0;
}

int dSend_FltSCTP_Data(int dSysNo)
{
	int				dRet;
	st_MsgQ			stMsgQ;
	pst_MsgQSub		pstMsgQSub;

	pstMsgQSub			= (pst_MsgQSub)&stMsgQ.llMType;
	pstMsgQSub->usType	= DEF_SYS;
	pstMsgQSub->usSvcID	= SID_FLT;
	pstMsgQSub->usMsgID	= MID_FLT_SCTP;

	stMsgQ.ucNTAFID		= dSysNo;
	stMsgQ.usBodyLen	= sizeof(st_ConnInfo) + NTAFT_HEADER_LEN;
	stMsgQ.ucProID		= SEQ_PROC_S_MNG;
	stMsgQ.dMsgQID		= 0;

	memcpy(&stMsgQ.szBody[NTAFT_HEADER_LEN], &stConnInfo, sizeof(st_ConnInfo));

	if( (dRet = dSendMsg(&stMsgQ, SEQ_PROC_SI_SVC)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dSendMsg(SI_SVC[%d]) dRet[%d]", 
			LT, SEQ_PROC_SI_SVC, dRet);
		return -1;
	}
	log_print(LOGN_DEBUG, "FLT SERVER IP INFORMATION SEND SUCCESS TO NTAF [NTAFNO]:[%d]", dSysNo);

	return 0;
}

int dSend_FltIPPool_Data(int dSysNo)
{
	int				dRet;
	st_MsgQ			stMsgQ;
	pst_MsgQSub		pstMsgQSub;

	pstMsgQSub			= (pst_MsgQSub)&stMsgQ.llMType;
	pstMsgQSub->usType	= DEF_SYS;
	pstMsgQSub->usSvcID	= SID_FLT;
	pstMsgQSub->usMsgID	= MID_FLT_MNIP;

	stMsgQ.ucNTAFID		= dSysNo;
	stMsgQ.usBodyLen	= sizeof(st_ConnInfo) + NTAFT_HEADER_LEN;
	stMsgQ.ucProID		= SEQ_PROC_S_MNG;
	stMsgQ.dMsgQID		= 0;

	memcpy(&stMsgQ.szBody[NTAFT_HEADER_LEN], &stConnInfo, sizeof(st_ConnInfo));

	if( (dRet = dSendMsg(&stMsgQ, SEQ_PROC_SI_SVC)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dSendMsg(SI_SVC[%d]) dRet[%d]", 
			LT, SEQ_PROC_SI_SVC, dRet);
		return -1;
	}
	log_print(LOGN_DEBUG, "FLT MN IP INFORMATION SEND SUCCESS TO NTAF [NTAFNO]:[%d]", dSysNo);

	return 0;
}

int dReadFltSHM(st_MsgQ *pstMsgQ)
{
	pst_MsgQSub		pstMsgQSub;
	int				dRet;

	pstMsgQSub = (pst_MsgQSub)&pstMsgQ->llMType;

	if(pstMsgQSub->usMsgID != MID_FLT_ALL)
	{
		log_print(LOGN_WARN, LH"Invalid MsgID[%hu]", LT, pstMsgQSub->usMsgID);
		return -1;
	}

	if( (dRet = dSend_NTAF_AlarmLevel(pstMsgQ->ucNTAFID)) < 0)
		log_print(LOGN_WARN, LH"ERROR IN dSend_NTAF_AlarmLevel() dRet[%d]", LT, dRet);

	if( (dRet = dSend_FltSvc_Data(pstMsgQ->ucNTAFID)) < 0)
		log_print(LOGN_WARN, LH"ERROR IN dSend_FltSvc_Data() dRet[%d]", LT, dRet);

	if( (dRet = dSend_FltIPPool_Data(pstMsgQ->ucNTAFID)) < 0)
		log_print(LOGN_WARN, LH"ERROR IN dSend_FltIPPool_Data() dRet[%d]", LT, dRet);

	if( (dRet = dSend_FltSCTP_Data(pstMsgQ->ucNTAFID)) < 0)
		log_print(LOGN_WARN, LH"ERROR IN dSend_FltSCTP_Data() dRet[%d]", LT, dRet);

	if( (dRet = dSendTrcToNtaf(pstMsgQ->ucNTAFID)) < 0)
		log_print(LOGN_WARN, LH"ERROR IN dSendTrcToNtaf() dRet[%d]", LT, dRet);

	if( (dRet = dSendTimerToNtaf(pstMsgQ->ucNTAFID)) < 0)
		log_print(LOGN_WARN, LH"ERROR IN dSendTrcToNtaf() dRet[%d]", LT, dRet);

	return 0;
}

int dSendTimerToNtaf(int dSysNo)
{
    int         dRet;
    int         size;
    st_MsgQ     stMsgQ;
    pst_MsgQSub pstMsgQSub;

    pstMsgQSub = (pst_MsgQSub)&stMsgQ.llMType;
    pstMsgQSub->usType  = DEF_SYS;
    pstMsgQSub->usSvcID = SID_FLT;
    pstMsgQSub->usMsgID = MID_FLT_TIMER;

    stMsgQ.llIndex = 1;
    stMsgQ.ucProID = SEQ_PROC_S_MNG;
    stMsgQ.dMsgQID = 0;

    stMsgQ.usBodyLen= sizeof(TIMER_INFO) + NTAFT_HEADER_LEN;
    size = stMsgQ.usBodyLen - NTAFT_HEADER_LEN;

    memcpy(&stMsgQ.szBody[NTAFT_HEADER_LEN], &flt_info->stTimerInfo, size);

	stMsgQ.ucNTAFID = dSysNo;
	if( (dRet = dSendMsg(&stMsgQ, SEQ_PROC_SI_SVC)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dSendMsg() dRet[%d]", LT, dRet);
		return -1;
	}

    return 0;
}

int dSendTrcToNtaf(int dSysNo)
{
    int         	dRet;
    int         	size;
    st_MsgQ			stMsgQ;
    pst_MsgQSub		pstMsgQSub;
	st_TraceList	stTraceList;

    pstMsgQSub = (pst_MsgQSub)&stMsgQ.llMType;
    pstMsgQSub->usType  = DEF_SYS;
    pstMsgQSub->usSvcID = SID_FLT;
    pstMsgQSub->usMsgID = MID_FLT_TRC;

    stMsgQ.llIndex = 1;
    stMsgQ.ucProID = SEQ_PROC_S_MNG;
    stMsgQ.dMsgQID = 0;

    stMsgQ.usBodyLen= sizeof(st_TraceList) + NTAFT_HEADER_LEN;
    size = stMsgQ.usBodyLen - NTAFT_HEADER_LEN;

	if( (dRet = dConvertIMSItoIRM(&stTraceList)) < 0)
	{
		log_print(LOGN_CRI, LH"FAILED IN dConvertIMSItoIRM() dRet[%d]", LT, dRet);
		return -1;
	}
    memcpy(&stMsgQ.szBody[NTAFT_HEADER_LEN], &stTraceList, size);

	stMsgQ.ucNTAFID = dSysNo;
	if( (dRet = dSendMsg(&stMsgQ, SEQ_PROC_SI_SVC)) < 0)
	{
		log_print(LOGN_CRI, LH"FAILED IN dSendTrcToNtaf dRet[%d]", LT, dRet);
		return -2;
	}

    return 0;
}

int dWriteFltSHM(st_MsgQ *pstMsgQ)
{
    pst_MsgQSub pstMsgQSub;


    pstMsgQSub = (pst_MsgQSub)&pstMsgQ->llMType;
    log_print(LOGN_INFO, "WriteFltSHM [%d] LEN[%d] ", pstMsgQSub->usMsgID, pstMsgQ->usBodyLen);

	switch(pstMsgQSub->usMsgID)
	{
		case MID_FLT_TMF:
			Apply_Filter_Tmf((st_Tmf_Info *)pstMsgQ->szBody);
			pstMsgQSub->usSvcID = SID_FLT;
			Send_Chk(pstMsgQ);
			break;

		case MID_FLT_SVC:
			Apply_Filter_SvcInfo((st_SvcInfo_List *)pstMsgQ->szBody);
			pstMsgQSub->usSvcID = SID_FLT;
			Send_Chk(pstMsgQ);
			break;

		case MID_FLT_COMMON:
			break;

		case MID_FLT_MNIP:
			Apply_Filter_NAS((st_NAS_List *)pstMsgQ->szBody);
			pstMsgQSub->usSvcID = SID_FLT;
			Send_Chk(pstMsgQ);
			break;

		case MID_FLT_SCTP:
			Apply_Filter_SCTP((st_SCTP_MMC*)pstMsgQ->szBody);
			pstMsgQSub->usSvcID = SID_FLT;
			Send_Chk(pstMsgQ);
			break;

		case MID_FLT_USER:
			 Apply_Sys_User_Info((st_User_Add_List *)pstMsgQ->szBody);
			 break;

		case MID_FLT_THRES:
			 Apply_Filter_Thres((st_Thres_List *)pstMsgQ->szBody);
			 break;

		case MID_FLT_MSC:
			break;

		case MID_FLT_EQUIP:
			Apply_Info_Equip((st_Info_Equip_List *)pstMsgQ->szBody);
			break;

		case MID_FLT_MS:
			break;

		case MID_FLT_SNA:
			break;

		default:
			log_print(LOGN_CRI, "dProc_Msg(): UNKNOWN MSGID RECEIVED [MSGID]:[%d].",
                    pstMsgQSub->usMsgID);
			return -1;
	}

    return 1;
}

int Apply_Filter_Tmf(st_Tmf_Info *pstTmfInfo)
{
    log_print(LOGN_DEBUG, "<<<<<<<<<* FILTER TMF APPLY TO SHARED MEMORY *>>>>>>>>>>");

	/* NTAM 시스템 NO 파일로 저장해야함 */
    if( flt_info->stTmfInfo.usTmfID != pstTmfInfo->usTmfID )
        flt_info->stTmfInfo.usTmfID = pstTmfInfo->usTmfID;
    log_print(LOGN_INFO, "NTAM SYSNO[%d]", pstTmfInfo->usTmfID);

    return 0;
}


int Apply_Filter_SvcInfo(st_SvcInfo_List *pstSvcInfoList)
{
	int			i, j, dRet;
	st_SvcInfo	stSvcInfo;

	flt_info->stSvcInfoShm.dCount = pstSvcInfoList->dCount;
	for(i = 0; i < DEF_SVC_CNT; i++)
	{
		flt_info->stSvcInfoShm.stSvcInfo[pstSvcInfoList->stSvcInfo[i].dIdx].uSvcIP		= pstSvcInfoList->stSvcInfo[i].uSvcIP;
		flt_info->stSvcInfoShm.stSvcInfo[pstSvcInfoList->stSvcInfo[i].dIdx].huPort		= pstSvcInfoList->stSvcInfo[i].huPort;
		flt_info->stSvcInfoShm.stSvcInfo[pstSvcInfoList->stSvcInfo[i].dIdx].cFlag		= pstSvcInfoList->stSvcInfo[i].cFlag;
		flt_info->stSvcInfoShm.stSvcInfo[pstSvcInfoList->stSvcInfo[i].dIdx].cSysType	= pstSvcInfoList->stSvcInfo[i].cSysType;

		flt_info->stSvcInfoShm.stSvcInfo[pstSvcInfoList->stSvcInfo[i].dIdx].huL4Code	= pstSvcInfoList->stSvcInfo[i].huL4Code;
		flt_info->stSvcInfoShm.stSvcInfo[pstSvcInfoList->stSvcInfo[i].dIdx].huL7Code	= pstSvcInfoList->stSvcInfo[i].huL7Code;
		flt_info->stSvcInfoShm.stSvcInfo[pstSvcInfoList->stSvcInfo[i].dIdx].huAppCode	= pstSvcInfoList->stSvcInfo[i].huAppCode;

		strcpy(flt_info->stSvcInfoShm.stSvcInfo[pstSvcInfoList->stSvcInfo[i].dIdx].szDesc, pstSvcInfoList->stSvcInfo[i].szDesc);

		if(pstSvcInfoList->dCount == (pstSvcInfoList->stSvcInfo[i].dIdx+1))
		{
			log_print(LOGN_DEBUG, "<<<<<<<<<* FILTER SVC APPLY TO SHARED MEMORY *>>>>>>>>>>");
			if( (dRet = dTruncateTbl(&stMySQL, "FLT_SVR")) < 0)
			{
				log_print(LOGN_CRI, LH"ERROR IN dTruncateTbl(\"FLT_SVC\") dRet[%d]", LT, dRet);
				return -1;
			}

			for(j = 0 ; j < pstSvcInfoList->dCount ; j++)
			{
				stSvcInfo.uSvcIP 		= flt_info->stSvcInfoShm.stSvcInfo[j].uSvcIP;
				stSvcInfo.huPort 		= flt_info->stSvcInfoShm.stSvcInfo[j].huPort;
				stSvcInfo.cFlag 		= flt_info->stSvcInfoShm.stSvcInfo[j].cFlag;
				stSvcInfo.cSysType 		= flt_info->stSvcInfoShm.stSvcInfo[j].cSysType;

				stSvcInfo.huL4Code 		= flt_info->stSvcInfoShm.stSvcInfo[j].huL4Code;
				stSvcInfo.huL7Code 		= flt_info->stSvcInfoShm.stSvcInfo[j].huL7Code;
				stSvcInfo.huAppCode 	= flt_info->stSvcInfoShm.stSvcInfo[j].huAppCode;

				strcpy(stSvcInfo.szDesc, flt_info->stSvcInfoShm.stSvcInfo[j].szDesc);

				log_print(LOGN_DEBUG, "dIdx[%d] uSvcIP[%u} huPort[%hu] cFlag[%hu] cSysType[%hu] huL4Code[%hu] huL7Code[%hu] huAppCode[%hu] Desc[%s]",
					j, stSvcInfo.uSvcIP, stSvcInfo.huPort , stSvcInfo.cFlag, stSvcInfo.cSysType,
					stSvcInfo.huL4Code, stSvcInfo.huL7Code, stSvcInfo.huAppCode, stSvcInfo.szDesc);

				if( (dRet = dInsertSvcInfo(&stMySQL, &stSvcInfo)) < 0)
				{
					log_print(LOGN_CRI, LH"ERROR IN dInsertSvcInfo() dRet[%d]", LT, dRet);
					return -2;
				}
			}
			break;
		}
	}

	return 0;
}

int Apply_Filter_NAS(st_NAS_List *pstNASList)
{
	int			i, j, dRet;
	st_NAS_db	stNAS;

	flt_info->stNASShm.dCount = pstNASList->dCount;

	for(i = 0; i < DEF_MNIP_COUNT; i++)
	{
		flt_info->stNASShm.stNAS[pstNASList->stNAS[i].dIdx].uMNIP		= pstNASList->stNAS[i].uMNIP;
		flt_info->stNASShm.stNAS[pstNASList->stNAS[i].dIdx].usNetMask	= pstNASList->stNAS[i].usNetMask;
		flt_info->stNASShm.stNAS[pstNASList->stNAS[i].dIdx].cFlag		= pstNASList->stNAS[i].cFlag;
		flt_info->stNASShm.stNAS[pstNASList->stNAS[i].dIdx].cSysType	= pstNASList->stNAS[i].cSysType;

		strcpy(flt_info->stNASShm.stNAS[pstNASList->stNAS[i].dIdx].szDesc, pstNASList->stNAS[i].szDesc);

		if(pstNASList->dCount == (pstNASList->stNAS[i].dIdx+1))
		{
			log_print(LOGN_DEBUG, "<<<<<<<<<* FILTER IPPOOL APPLY TO SHARED MEMORY *>>>>>>>>>>");
			if( (dRet = dTruncateTbl(&stMySQL, "FLT_IPPOOL")) < 0)
			{
				log_print(LOGN_CRI, LH"ERROR IN dTruncateTbl(\"FLT_IPPOOL\") dRet[%d]", LT, dRet);
				return -1;
			}

			for(j = 0; j < pstNASList->dCount; j++)
			{
				stNAS.uMNIP		= flt_info->stNASShm.stNAS[j].uMNIP;
				stNAS.usNetMask	= flt_info->stNASShm.stNAS[j].usNetMask;
				stNAS.cFlag		= flt_info->stNASShm.stNAS[j].cFlag;
				stNAS.cSysType	= flt_info->stNASShm.stNAS[j].cSysType;

				strcpy(stNAS.szDesc, flt_info->stNASShm.stNAS[j].szDesc);

				log_print(LOGN_DEBUG, "IDX[%d] uMNIP[%u] usNetMask[%hu] cFlag[%hu] cSysType[%hu]",
					j, stNAS.uMNIP, stNAS.usNetMask, stNAS.cFlag, stNAS.cSysType);

				if( (dRet = dInsertMNIP(&stMySQL, &stNAS)) < 0)
				{
					log_print(LOGN_CRI, LH"ERROR IN dInsertMNIP() dRet[%d]", LT, dRet);
					return -2;
				}
			}
			break;
		}
	}

	return 0;
}

int Apply_Filter_SCTP(st_SCTP_MMC *pstSCTPList)
{
	int			i, j, dRet;
	st_SCTP_DB	stSCTPDB;

	flt_info->stSCTPShm.dCount = pstSCTPList->dCount;

	for(i = 0; i < DEF_MNIP_COUNT; i++)
	{
		flt_info->stSCTPShm.stSCTP[pstSCTPList->stSCTP[i].dIdx].uSCTPIP		= pstSCTPList->stSCTP[i].uSCTPIP;
		flt_info->stSCTPShm.stSCTP[pstSCTPList->stSCTP[i].dIdx].cSysType	= pstSCTPList->stSCTP[i].cSysType;
		flt_info->stSCTPShm.stSCTP[pstSCTPList->stSCTP[i].dIdx].cDirection	= pstSCTPList->stSCTP[i].cDirection;
		flt_info->stSCTPShm.stSCTP[pstSCTPList->stSCTP[i].dIdx].huGroupID	= pstSCTPList->stSCTP[i].huGroupID;

		strcpy(flt_info->stSCTPShm.stSCTP[pstSCTPList->stSCTP[i].dIdx].szDesc, pstSCTPList->stSCTP[i].szDesc);

		if(pstSCTPList->dCount == (pstSCTPList->stSCTP[i].dIdx+1))
		{
			log_print(LOGN_DEBUG, "<<<<<<<<<* FILTER IPPOOL APPLY TO SHARED MEMORY *>>>>>>>>>>");
			if( (dRet = dTruncateTbl(&stMySQL, "FLT_SCTP")) < 0)
			{
				log_print(LOGN_CRI, LH"ERROR IN dTruncateTbl(\"FLT_SCTP\") dRet[%d]", LT, dRet);
				return -1;
			}

			for(j = 0; j < pstSCTPList->dCount; j++)
			{
				stSCTPDB.uSCTPIP		= flt_info->stSCTPShm.stSCTP[j].uSCTPIP;
				stSCTPDB.cSysType		= flt_info->stSCTPShm.stSCTP[j].cSysType;
				stSCTPDB.cDirection		= flt_info->stSCTPShm.stSCTP[j].cDirection;
				stSCTPDB.huGroupID		= flt_info->stSCTPShm.stSCTP[j].huGroupID;

				strcpy(stSCTPDB.szDesc, flt_info->stSCTPShm.stSCTP[j].szDesc);

				log_print(LOGN_DEBUG, "IDX[%d] uSCTPIP[%u] cSysType[%hu] cDirection[%hu] huGroupID[%hu]",
					j, stSCTPDB.uSCTPIP, stSCTPDB.cSysType, stSCTPDB.cDirection, stSCTPDB.huGroupID);

				if( (dRet = dInsertSCTPInfo(&stMySQL, &stSCTPDB)) < 0)
				{
					log_print(LOGN_CRI, LH"ERROR IN dInsertSCTPInfo() dRet[%d]", LT, dRet);
					return -2;
				}
			}
			break;
		}
	}

	return 0;
}

int Apply_Filter_Thres(st_Thres_List *pstThresList)
{
	int			i, j, dRet;
	st_Thres	stThres;

	flt_info->stThresShm.dCount = pstThresList->dCount;

	for(i = 0; i < DEF_THRES_MMC; i++)
	{
		flt_info->stThresShm.stThres[pstThresList->stThres[i].dIdx].cSvcType			= pstThresList->stThres[i].cSvcType;
		flt_info->stThresShm.stThres[pstThresList->stThres[i].dIdx].uTCPSetupTime		= pstThresList->stThres[i].uTCPSetupTime;
		flt_info->stThresShm.stThres[pstThresList->stThres[i].dIdx].uResponseTime		= pstThresList->stThres[i].uResponseTime;
		flt_info->stThresShm.stThres[pstThresList->stThres[i].dIdx].uUpThroughput		= pstThresList->stThres[i].uUpThroughput;
		flt_info->stThresShm.stThres[pstThresList->stThres[i].dIdx].uDnThroughput		= pstThresList->stThres[i].uDnThroughput;
		flt_info->stThresShm.stThres[pstThresList->stThres[i].dIdx].uUpRetransCount		= pstThresList->stThres[i].uUpRetransCount;
		flt_info->stThresShm.stThres[pstThresList->stThres[i].dIdx].uDnRetransCount		= pstThresList->stThres[i].uDnRetransCount;
		flt_info->stThresShm.stThres[pstThresList->stThres[i].dIdx].uUpJitter			= pstThresList->stThres[i].uUpJitter;
		flt_info->stThresShm.stThres[pstThresList->stThres[i].dIdx].uDnJitter			= pstThresList->stThres[i].uDnJitter;
		flt_info->stThresShm.stThres[pstThresList->stThres[i].dIdx].uUpPacketLoss		= pstThresList->stThres[i].uUpPacketLoss;
		flt_info->stThresShm.stThres[pstThresList->stThres[i].dIdx].uDnPacketLoss		= pstThresList->stThres[i].uDnPacketLoss;

		strcpy(flt_info->stThresShm.stThres[pstThresList->stThres[i].dIdx].szDesc, pstThresList->stThres[i].szDesc);

		if(pstThresList->dCount == (pstThresList->stThres[i].dIdx+1))
		{
			log_print(LOGN_DEBUG, "<<<<<<<<<* FILTER THRESHOLD APPLY TO SHARED MEMORY *>>>>>>>>>>");
			if( (dRet = dTruncateTbl(&stMySQL, "INFO_DEFECT_THRESHOLD")) < 0)
			{
				log_print(LOGN_CRI, LH"ERROR IN dTruncateTbl(\"INFO_DEFECT_THRESHOLD\") dRet[%d]", LT, dRet);
				return -1;
			}
			/*End of dTruncateTbl*/

			for(j = 0; j < pstThresList->dCount; j++)
			{
				stThres.dIdx			= flt_info->stThresShm.stThres[j].dIdx;
				stThres.cSvcType		= flt_info->stThresShm.stThres[j].cSvcType;
				stThres.uTCPSetupTime	= flt_info->stThresShm.stThres[j].uTCPSetupTime;
				stThres.uResponseTime	= flt_info->stThresShm.stThres[j].uResponseTime;
				stThres.uUpThroughput	= flt_info->stThresShm.stThres[j].uUpThroughput;
				stThres.uDnThroughput	= flt_info->stThresShm.stThres[j].uDnThroughput;
				stThres.uUpRetransCount	= flt_info->stThresShm.stThres[j].uUpRetransCount;
				stThres.uDnRetransCount	= flt_info->stThresShm.stThres[j].uDnRetransCount;
				stThres.uUpJitter		= flt_info->stThresShm.stThres[j].uUpJitter;
				stThres.uDnJitter		= flt_info->stThresShm.stThres[j].uDnJitter;
				stThres.uUpPacketLoss	= flt_info->stThresShm.stThres[j].uUpPacketLoss;
				stThres.uDnPacketLoss	= flt_info->stThresShm.stThres[j].uDnPacketLoss;
				strcpy(stThres.szDesc, flt_info->stThresShm.stThres[j].szDesc);

				log_print(LOGN_CRI, "dIdx[%d] cSvcType[%u] uTCPSetupTime[%u] uResponseTime[%u] uUpThroughput[%u] uDnThroughput[%u] "
					"uUpRetransCount[%u] uDnRetransCount[%u] uUpJitter[%u] uDnJitter[%u] uUpPacketLoss[%u] uDnPacketLoss[%u] szDesc[%s]",
					stThres.dIdx, stThres.cSvcType, stThres.uTCPSetupTime, stThres.uResponseTime,
					stThres.uUpThroughput, stThres.uDnThroughput, stThres.uUpRetransCount, stThres.uDnRetransCount,
					stThres.uUpJitter, stThres.uDnJitter, stThres.uUpPacketLoss, stThres.uDnPacketLoss, stThres.szDesc);

				if( (dRet = dInsertThres(&stMySQL, &stThres)) < 0)
				{
					log_print(LOGN_CRI, LH"ERROR IN dInsertThres() dRet[%d]", LT, dRet);
					return -2;
				}
			}
			break;
		}
	}

	return 0;
}/* end of Apply_INFO_THRESHOLD*/

int Apply_Info_Equip(st_Info_Equip_List *pstInfoEquipList)
{
    int 			i,j,dRet;
    st_Info_Equip   stInfoEquip;

	flt_info->stInfoEquipShm.dCount = pstInfoEquipList->dCount;

	log_print(LOGN_INFO, "INFO EQUIP MSG GET");

    for(i = 0; i < DEF_EQUIP; i++)
    {
		flt_info->stInfoEquipShm.stInfoEquip[pstInfoEquipList->stInfoEquip[i].dIdx].dIdx		= pstInfoEquipList->stInfoEquip[i].dIdx;
		flt_info->stInfoEquipShm.stInfoEquip[pstInfoEquipList->stInfoEquip[i].dIdx].uEquipIP	= pstInfoEquipList->stInfoEquip[i].uEquipIP;
		flt_info->stInfoEquipShm.stInfoEquip[pstInfoEquipList->stInfoEquip[i].dIdx].cEquipType	= pstInfoEquipList->stInfoEquip[i].cEquipType;

		strcpy(flt_info->stInfoEquipShm.stInfoEquip[pstInfoEquipList->stInfoEquip[i].dIdx].szEquipTypeName, pstInfoEquipList->stInfoEquip[i].szEquipTypeName);
		strcpy(flt_info->stInfoEquipShm.stInfoEquip[pstInfoEquipList->stInfoEquip[i].dIdx].szEquipName, pstInfoEquipList->stInfoEquip[i].szEquipName);
		strcpy(flt_info->stInfoEquipShm.stInfoEquip[pstInfoEquipList->stInfoEquip[i].dIdx].szDesc, pstInfoEquipList->stInfoEquip[i].szDesc);

        if(pstInfoEquipList->dCount == (pstInfoEquipList->stInfoEquip[i].dIdx+1))
		{
			log_print(LOGN_DEBUG, "<<<<<<<<<* FILTER EQUIP APPLY TO SHARED MEMORY *>>>>>>>>>>");
			if( (dRet = dTruncateTbl(&stMySQL, "TB_MEQUIPMC")) < 0)
			{
				log_print(LOGN_CRI, LH"ERROR IN dTruncateTbl(\"TB_MEQUIPMC\") dRet[%d]", LT, dRet);
				return -1;
			}

			for(j = 0; j < pstInfoEquipList->dCount; j++)
			{
				stInfoEquip.uEquipIP 	=  flt_info->stInfoEquipShm.stInfoEquip[j].uEquipIP;
				stInfoEquip.cEquipType 	=  flt_info->stInfoEquipShm.stInfoEquip[j].cEquipType;

				strcpy(stInfoEquip.szEquipTypeName, flt_info->stInfoEquipShm.stInfoEquip[j].szEquipTypeName);
				strcpy(stInfoEquip.szEquipName, flt_info->stInfoEquipShm.stInfoEquip[j].szEquipName);
				strcpy(stInfoEquip.szDesc, flt_info->stInfoEquipShm.stInfoEquip[j].szDesc);

				log_print(LOGN_DEBUG, "Index[%d] uEquipIP[%u] cEquipType[%hu] szEquipTypeName[%s] szEquipName[%s] szDesc[%s]",
					j, stInfoEquip.uEquipIP, stInfoEquip.cEquipType, stInfoEquip.szEquipTypeName, stInfoEquip.szEquipName, stInfoEquip.szDesc);

                if( (dRet = dInsertEquip(&stMySQL, &stInfoEquip)) < 0)
                {
                    log_print(LOGN_CRI, LH"ERROR IN dInsertEquip() dRet[%d]", LT, dRet);
                    return -2;
                }
            }
			break;
        }
    }

    return 0;
}

int Send_Chk(st_MsgQ *pstMsgQ)
{
	int			i, dRet, size;
	st_MsgQ		stTempMsgQ;

	pstMsgQ->usBodyLen	= pstMsgQ->usBodyLen + NTAFT_HEADER_LEN;
	size				= pstMsgQ->usBodyLen - NTAFT_HEADER_LEN;

	memcpy(&stTempMsgQ, pstMsgQ, DEF_MSGHEAD_LEN);
	memcpy(&stTempMsgQ.szBody[NTAFT_HEADER_LEN], pstMsgQ->szBody, size);

	for(i = 0; i < MAX_NTAF_NUM; i++)
	{
		if(pstSubSys->sys[i].uiIP > 0)
		{
			stTempMsgQ.ucNTAFID = pstSubSys->sys[i].usSysNo;
			if( (dRet = dSendMsg(&stTempMsgQ, SEQ_PROC_SI_SVC)) < 0)
			{
				log_print(LOGN_CRI, LH"ERROR IN dSendMsg(SI_SVC[%d]) dRet[%d]", 
					LT, SEQ_PROC_SI_SVC, dRet);
				return -1;
			}
			log_print(LOGN_INFO, LH"SUCCESS IN Send_Chk(NTAF_ID[%d], size[%d])", 
				LT, pstMsgQ->ucNTAFID, size);
		}
	}

	return 0;
}

int Apply_Sys_User_Info(st_User_Add_List *pstUserList)
{
	int				i, j, dRet;
	st_User_Add		stUserAdd;

	flt_info->stUserAddShm.dCount = pstUserList->dCount;

	for(i = 0; i < DEF_USER; i++)
	{
		flt_info->stUserAddShm.stUserAdd[pstUserList->stUserAdd[i].dIdx].sSLevel		= pstUserList->stUserAdd[i].sSLevel;
		flt_info->stUserAddShm.stUserAdd[pstUserList->stUserAdd[i].dIdx].usLogin		= pstUserList->stUserAdd[i].usLogin;
		flt_info->stUserAddShm.stUserAdd[pstUserList->stUserAdd[i].dIdx].uLastLoginTime	= pstUserList->stUserAdd[i].uLastLoginTime;
		flt_info->stUserAddShm.stUserAdd[pstUserList->stUserAdd[i].dIdx].uConnectIP		= pstUserList->stUserAdd[i].uConnectIP;
		flt_info->stUserAddShm.stUserAdd[pstUserList->stUserAdd[i].dIdx].uCreateTime	= pstUserList->stUserAdd[i].uCreateTime;

		strcpy(flt_info->stUserAddShm.stUserAdd[pstUserList->stUserAdd[i].dIdx].szUserName, pstUserList->stUserAdd[i].szUserName);
		strcpy(flt_info->stUserAddShm.stUserAdd[pstUserList->stUserAdd[i].dIdx].szPassword, pstUserList->stUserAdd[i].szPassword);
		strcpy(flt_info->stUserAddShm.stUserAdd[pstUserList->stUserAdd[i].dIdx].szLocalName, pstUserList->stUserAdd[i].szLocalName);
		strcpy(flt_info->stUserAddShm.stUserAdd[pstUserList->stUserAdd[i].dIdx].szContact, pstUserList->stUserAdd[i].szContact);

		log_print(LOGN_INFO, LH"szUserName[%s] uConnectIP[%u:%s] uCreateTime[%u]", LT,
			pstUserList->stUserAdd[i].szUserName, pstUserList->stUserAdd[i].uConnectIP, util_cvtipaddr(NULL,pstUserList->stUserAdd[i].uConnectIP), pstUserList->stUserAdd[i].uCreateTime);

		if(pstUserList->dCount == (pstUserList->stUserAdd[i].dIdx+1))
		{
			if( (dRet = dTruncateTbl(&stMySQL, "SYS_USER_INFO")) < 0)
			{
				log_print(LOGN_CRI, LH"ERROR IN dTruncateTbl(\"SYS_USER_INFO\") dRet[%d]", LT, dRet);
				return -1;
			}

			for(j = 0; j < pstUserList->dCount; j++)
			{
				stUserAdd.sSLevel			= flt_info->stUserAddShm.stUserAdd[j].sSLevel;
				stUserAdd.usLogin			= flt_info->stUserAddShm.stUserAdd[j].usLogin;
				stUserAdd.uLastLoginTime	= flt_info->stUserAddShm.stUserAdd[j].uLastLoginTime;
				stUserAdd.uConnectIP		= flt_info->stUserAddShm.stUserAdd[j].uConnectIP;

				stUserAdd.uCreateTime		= flt_info->stUserAddShm.stUserAdd[j].uCreateTime;

				strcpy(stUserAdd.szUserName, flt_info->stUserAddShm.stUserAdd[j].szUserName);
				strcpy(stUserAdd.szPassword, flt_info->stUserAddShm.stUserAdd[j].szPassword);
				strcpy(stUserAdd.szLocalName, flt_info->stUserAddShm.stUserAdd[j].szLocalName);
				strcpy(stUserAdd.szContact, flt_info->stUserAddShm.stUserAdd[j].szContact);

				log_print(LOGN_DEBUG, "IDX[%d] sSLevel[%hd] szUserName[%s] szLocalName[%s] uCreateTime[%u]",
					j, stUserAdd.sSLevel,stUserAdd.szUserName, stUserAdd.szLocalName, stUserAdd.uCreateTime);

				if( (dRet = dAddAdminInfo(&stMySQL, &stUserAdd)) < 0)
				{
					log_print(LOGN_CRI, LH"ERROR IN dAddAdminInfo() dRet[%d]", LT, dRet);
					return -2;
				}
			}
			break;
		}
	}

	return 0;
}

int dResChkInfo(pst_MsgQ pstMsgQ)
{
	pst_MsgQSub	pstMsgQSub;

	pstMsgQSub = (pst_MsgQSub)&pstMsgQ->llMType;

	log_print(LOGN_INFO, LH"usSvcID[%hu] usMsgID[%hu] llIndex[%lld]", LT,
		pstMsgQSub->usSvcID, pstMsgQSub->usMsgID, pstMsgQ->llIndex);
	switch(pstMsgQSub->usMsgID)
	{
		case MID_FLT_ALM:
		case MID_FLT_LOG:
		case MID_FLT_SYS:
		case MID_FLT_TCP:
		case MID_DIS_NTAF_CONF :
			dRcv_Alm(pstMsgQ, pstMsgQSub->usMsgID);
			break;

		default:
			log_print(LOGN_CRI, LH"INVALID usMsgID[%hu]", LT, pstMsgQSub->usMsgID);
			return -1;
	}

	return 0;
}

int dRcv_Alm(pst_MsgQ pstMsgQ, USHORT usMID)
{
	int					dRet;
	char				szMsg[MSG_DATA_LEN];
	pst_AlmLevel_List	pstAlmLevelList;
	pst_Conf			pstConf;
	dbm_msg_t			smsg;
	mml_msg				ml;

	memset(szMsg, 0x00, MSG_DATA_LEN);

	if(usMID == MID_FLT_ALM)
	{
		memcpy(smsg.data, pstMsgQ->szBody, sizeof(st_AlmLevel_List));
		pstAlmLevelList = (st_AlmLevel_List *)smsg.data;
		smsg.head.msg_len = sizeof(st_AlmLevel_List);
	}
	else
	{
		memcpy(smsg.data, pstMsgQ->szBody, sizeof(st_Conf));
		pstConf = (st_Conf *)smsg.data;
		smsg.head.msg_len = sizeof(st_Conf);
	}

	smsg.common.mml_err     = DBM_SUCCESS;
	smsg.common.cont_flag   = DBM_END;

	ml.src_func = g_stMmlsg.src_func[pstMsgQ->llIndex] ;
	ml.src_proc = g_stMmlsg.src_proc[pstMsgQ->llIndex] ;
	ml.cmd_id   = g_stMmlsg.cmd_id[pstMsgQ->llIndex] ;
	ml.msg_id   = g_stMmlsg.msg_id[pstMsgQ->llIndex] ;

	if( (dRet = dSendMMC(&ml, &smsg, pstMsgQ->llNID)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
		return -1;
	}

	return 0;
}

int dSendRPPI(unsigned short uhMsgID)
{
	int				dRet;
	unsigned char	*pNode;
	NOTIFY_SIG		*pNOTIFY;

	if( (pNode = nifo_node_alloc(pMEMSINFO)) == NULL)
	{
		log_print(LOGN_CRI, LH"ERROR IN nifo_node_alloc()", LT);
		return -1;
	}

	if( (pNOTIFY = (NOTIFY_SIG*)nifo_tlv_alloc(pMEMSINFO, pNode, NOTIFY_SIG_DEF_NUM, sizeof(NOTIFY_SIG), DEF_MEMSET_ON)) == NULL)
	{
		log_print(LOGN_CRI, LH"ERROR IN nifo_tlv_alloc()", LT);
		return -2;
	}
	else
	{
		pNOTIFY->uiEventTime	= time(NULL);
		pNOTIFY->uiType			= (unsigned int)uhMsgID;
	}

	if( (dRet = gifo_write(pMEMSINFO, gpCIFO, SEQ_PROC_S_MNG, SEQ_PROC_A_RPPI, nifo_offset(pMEMSINFO, pNode))) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN gifo_write() [errno:%d-%s]", LT, -dRet, strerror(-dRet));
		return -3;
	}

	return 0;
}

int dSendROAM(unsigned short uhMsgID)
{
	int				dRet;
	unsigned char	*pNode;
	NOTIFY_SIG		*pNOTIFY;

	if( (pNode = nifo_node_alloc(pMEMSINFO)) == NULL)
	{
		log_print(LOGN_CRI, LH"ERROR IN nifo_node_alloc()", LT);
		return -1;
	}

	if( (pNOTIFY = (NOTIFY_SIG*)nifo_tlv_alloc(pMEMSINFO, pNode, NOTIFY_SIG_DEF_NUM, sizeof(NOTIFY_SIG), DEF_MEMSET_ON)) == NULL)
	{
		log_print(LOGN_CRI, LH"ERROR IN nifo_tlv_alloc()", LT);
		return -2;
	}
	else
	{
		pNOTIFY->uiEventTime	= time(NULL);
		pNOTIFY->uiType			= (unsigned int)uhMsgID;
	}

	if( (dRet = gifo_write(pMEMSINFO, gpCIFO, SEQ_PROC_S_MNG, SEQ_PROC_A_ROAM, nifo_offset(pMEMSINFO, pNode))) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN gifo_write() [errno:%d-%s]", LT, -dRet, strerror(-dRet));
		return -3;
	}

	return 0;
}

int dSendMTRACE(unsigned short uhMsgID)
{
	int				dRet;
	unsigned char	*pNode;
	NOTIFY_SIG		*pNOTIFY;

	if( (pNode = nifo_node_alloc(pMEMSINFO)) == NULL)
	{
		log_print(LOGN_CRI, LH"ERROR IN nifo_node_alloc()", LT);
		return -1;
	}

	if( (pNOTIFY = (NOTIFY_SIG*)nifo_tlv_alloc(pMEMSINFO, pNode, NOTIFY_SIG_DEF_NUM, sizeof(NOTIFY_SIG), DEF_MEMSET_ON)) == NULL)
	{
		log_print(LOGN_CRI, LH"ERROR IN nifo_tlv_alloc()", LT);
		return -2;
	}
	else
	{
		pNOTIFY->uiEventTime	= time(NULL);
		pNOTIFY->uiType			= (unsigned int)uhMsgID;
	}

	if( (dRet = gifo_write(pMEMSINFO, gpCIFO, SEQ_PROC_S_MNG, SEQ_PROC_M_TRACE, nifo_offset(pMEMSINFO, pNode))) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN gifo_wrtie() [errno:%d-%s]", LT, -dRet, strerror(-dRet));
		return -3;
	}

	return 0;
}


int dReadTimerFile(TIMER_INFO *pstData)
{
	FILE			*fp;
	size_t			szLen;
	char			sFileName[PATH_MAX], sBuf[512];
	int				i;

	sprintf(sFileName, "%s%s", DATA_PATH, "Timer.dat");
	if( (fp = fopen(sFileName, "r")) == NULL)
	{
		if( (fp = fopen(sFileName, "w")) == NULL)
		{
			log_print(LOGN_CRI, LH"FAILED IN fopen(%s) [errno:%d-%s]", LT, sFileName, errno, strerror(errno));
		}
		else
			fclose(fp);

		return -1;
	}

	i		= 0;
	sBuf[0]	= 0x00;
	while(fgets(sBuf, 512, fp) != NULL)
	{
		szLen = strlen(sBuf);
		while(isspace(sBuf[szLen-1]))
			sBuf[--szLen] = 0x00;

		if(sBuf[0] != '#')
		{
			log_print(LOGN_CRI, LH"ERROR IN FILE(%s) LINE(%d) FORMAT", LT, sFileName, i);
			fclose(fp);
			return -2;
		}

		if(sBuf[1] == '#')
		{
			sBuf[0] = 0x00;
			continue;
		}
		else if(sBuf[1] == 'E')
			break;
		else
		{
			if(sscanf(&sBuf[2], "%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u",
				&pstData->usTimerInfo[RPPI_CALL_TIMEOUT], &pstData->usTimerInfo[RPPI_WAIT_TIMEOUT], 
				&pstData->usTimerInfo[PI_VT_TIMEOUT], &pstData->usTimerInfo[PI_IM_TIMEOUT], 
				&pstData->usTimerInfo[PI_TCP_RSTWAIT], &pstData->usTimerInfo[PI_TCP_TIMEOUT],
				&pstData->usTimerInfo[PI_DNS_TIMEOUT], &pstData->usTimerInfo[PI_SIP_TIMEOUT], 
				&pstData->usTimerInfo[PI_MSRP_TIMEOUT], &pstData->usTimerInfo[PI_RAD_TIMEOUT], 
				&pstData->usTimerInfo[PI_DIA_TIMEOUT], &pstData->usTimerInfo[PI_CALL_TIMEOUT],
				&pstData->usTimerInfo[PI_WAIT_TIMEOUT], &pstData->usTimerInfo[PI_DORM_TIMEOUT], 
				&pstData->usTimerInfo[RP_CALL_TIMEOUT], &pstData->usTimerInfo[RP_DORM_TIMEOUT], 
				&pstData->usTimerInfo[PI_INET_TIMEOUT], &pstData->usTimerInfo[PI_RCALL_TIMEOUT], 
				&pstData->usTimerInfo[RP_RCALL_TIMEOUT], &pstData->usTimerInfo[PI_RCALL_SIGWAIT],
				&pstData->usTimerInfo[RP_RCALL_SIGWAIT]) == 21)
			{
				if(i >= 1)
				{
					log_print(LOGN_CRI, LH"File[%s]'count is over maximum(%d).", 
							LT, sFileName, 1);
					fclose(fp);
					return -3;
				}

				log_print(LOGN_INFO, "TIMERINFO: RPPI_CALL_TIMEOUT[%u] RPPI_WAIT_TIMEOUT[%u] PI_VT_TIMEOUT[%u] "
						"PI_IM_TIMEOUT[%u] PI_TCP_RSTWAIT[%u] PI_TCP_TIMEOUT[%u] PI_DNS_TIMEOUT[%u] "
						"PI_SIP_TIMEOUT[%u] PI_MSRP_TIMEOUT[%u] PI_RAD_TIMEOUT[%u] PI_DIA_TIMEOUT[%u] "
						"PI_CALL_TIMEOUT[%u] PI_WAIT_TIMEOUT[%u] PI_DORM_TIMEOUT[%u] RP_CALL_TIMEOUT[%u] "
						"RP_DORM_TIMEOUT[%u] PI_INET_TIMEOUT[%u] PI_RCALL_TIMEOUT[%u] RP_RCALL_TIMEOUT[%u]" 
						"PI_RCALL_SIGWAIT[%u] RP_RCALL_SIGWAIT[%u",
					pstData->usTimerInfo[RPPI_CALL_TIMEOUT], pstData->usTimerInfo[RPPI_WAIT_TIMEOUT], 
					pstData->usTimerInfo[PI_VT_TIMEOUT], pstData->usTimerInfo[PI_IM_TIMEOUT],
					pstData->usTimerInfo[PI_TCP_RSTWAIT], pstData->usTimerInfo[PI_TCP_TIMEOUT], 
					pstData->usTimerInfo[PI_DNS_TIMEOUT], pstData->usTimerInfo[PI_SIP_TIMEOUT],
					pstData->usTimerInfo[PI_MSRP_TIMEOUT], pstData->usTimerInfo[PI_RAD_TIMEOUT], 
					pstData->usTimerInfo[PI_DIA_TIMEOUT], pstData->usTimerInfo[PI_CALL_TIMEOUT],
					pstData->usTimerInfo[PI_WAIT_TIMEOUT], pstData->usTimerInfo[PI_DORM_TIMEOUT], 
					pstData->usTimerInfo[RP_CALL_TIMEOUT], pstData->usTimerInfo[RP_DORM_TIMEOUT],
					pstData->usTimerInfo[PI_INET_TIMEOUT], pstData->usTimerInfo[PI_RCALL_TIMEOUT],
					pstData->usTimerInfo[RP_RCALL_TIMEOUT], pstData->usTimerInfo[PI_RCALL_SIGWAIT],
					pstData->usTimerInfo[RP_RCALL_SIGWAIT]);

				i++;
			}
			sBuf[0] = 0x00;
		}
	} // while-loop end
	fclose(fp);

	return 0;
}

int dWriteTimerFile(TIMER_INFO *pstData)
{
	char	sFileName[PATH_MAX];
	FILE	*fp;

	sprintf(sFileName, "%s%s", DATA_PATH, "Timer.dat");
	if( (fp = fopen(sFileName, "w")) == NULL)
	{
		log_print(LOGN_CRI, LH"FAILED IN fopen(%s) [errno:%d-%s]", LT, sFileName, errno, strerror(errno));
		return -1;
	}

	fprintf(fp, "## Timer FILE\n");

	fprintf(fp, "#@ %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u\n",
		pstData->usTimerInfo[RPPI_CALL_TIMEOUT], pstData->usTimerInfo[RPPI_WAIT_TIMEOUT], 
		pstData->usTimerInfo[PI_VT_TIMEOUT], pstData->usTimerInfo[PI_IM_TIMEOUT],
		pstData->usTimerInfo[PI_TCP_RSTWAIT], pstData->usTimerInfo[PI_TCP_TIMEOUT], 
		pstData->usTimerInfo[PI_DNS_TIMEOUT], pstData->usTimerInfo[PI_SIP_TIMEOUT],
		pstData->usTimerInfo[PI_MSRP_TIMEOUT], pstData->usTimerInfo[PI_RAD_TIMEOUT], 
		pstData->usTimerInfo[PI_DIA_TIMEOUT], pstData->usTimerInfo[PI_CALL_TIMEOUT],
		pstData->usTimerInfo[PI_WAIT_TIMEOUT], pstData->usTimerInfo[PI_DORM_TIMEOUT], 
		pstData->usTimerInfo[RP_CALL_TIMEOUT], pstData->usTimerInfo[RP_DORM_TIMEOUT],
		pstData->usTimerInfo[PI_INET_TIMEOUT], pstData->usTimerInfo[PI_RCALL_TIMEOUT],
		pstData->usTimerInfo[RP_RCALL_TIMEOUT], pstData->usTimerInfo[PI_RCALL_SIGWAIT],
		pstData->usTimerInfo[RP_RCALL_SIGWAIT]);

	fprintf(fp, "#E END\n");

	fclose(fp);

	return 0;
}

int dCopy(char *sSourcePath, char *sDestinationPath)
{
	int			fdSrc, fdDest;
	ssize_t		sszRdLen, sszWrLen;
	char		sBuf[BUFSIZ];
	struct stat	stSrcStat;

	if( (fdSrc = open(sSourcePath, O_RDONLY)) == -1)
	{
		log_print(LOGN_CRI, LH"ERROR IN open(sSourcePath[%s]) errno[%d-%s]", LT,
			sSourcePath, errno, strerror(errno));
		return -1;
	}

	if(fstat(fdSrc, &stSrcStat) == -1)
	{
		fprintf(stderr, LH"FAILED IN fstat(fdSrc[%d-%s]) errno[%d-%s]\n", LT,
			fdSrc, sSourcePath, errno, strerror(errno));

		if(close(fdSrc) == -1)
		{
			log_print(LOGN_CRI, LH"FAILED IN close(fdSrc[%d-%s]) errno[%d-%s]\n", LT,
				fdSrc, sSourcePath, errno, strerror(errno));
		}
		return -2;
	}

	if( (fdDest = open(sDestinationPath, O_WRONLY | O_CREAT, stSrcStat.st_mode)) == -1)
	{
		log_print(LOGN_CRI, LH"ERROR IN open(sDestinationPath[%s]) errno[%d-%s]", LT,
			sDestinationPath, errno, strerror(errno));
		if(close(fdSrc) == -1)
		{
			log_print(LOGN_CRI, LH"FAILED IN close(fdSrc[%d-%s]) errno[%d-%s]\n", LT,
				fdSrc, sSourcePath, errno, strerror(errno));
		}
		return -3;
	}

	while( (sszRdLen = read(fdSrc, sBuf, BUFSIZ)) != -1)
	{
		if(!sszRdLen)
			break;

		if( (sszWrLen = write(fdDest, sBuf, (size_t)sszRdLen)) == -1)
		{
			log_print(LOGN_CRI, LH"ERROR IN write(fdDest[%d-%s]) errno[%d-%s]", LT,
				fdDest, sDestinationPath, errno, strerror(errno));

			if(close(fdSrc) == -1)
			{
				log_print(LOGN_CRI, LH"FAILED IN close(fdSrc[%d-%s]) errno[%d-%s]", LT,
					fdSrc, sSourcePath, errno, strerror(errno));
			}

			if(close(fdDest) == -1)
			{
				log_print(LOGN_CRI, LH"FAILED IN close(fdDest[%d-%s]) errno[%d-%s]", LT,
					fdDest, sDestinationPath, errno, strerror(errno));
			}
			return -4;
		}

		if(sszWrLen != sszRdLen)
		{
			log_print(LOGN_CRI, LH"sszWrLen[%lu] is not equal sszRdLen[%lu]", LT,
				sszWrLen, sszRdLen);

			if(close(fdSrc) == -1)
			{
				log_print(LOGN_CRI, LH"FAILED IN close(fdSrc[%d-%s]) errno[%d-%s]", LT,
					fdSrc, sSourcePath, errno, strerror(errno));
			}

			if(close(fdDest) == -1)
			{
				log_print(LOGN_CRI, LH"FAILED IN close(fdDest[%d-%s]) errno[%d-%s]", LT,
					fdDest, sDestinationPath, errno, strerror(errno));
			}
			return -5;
		}
	}

	if(sszRdLen == -1)
	{
		log_print(LOGN_CRI, LH"FAILED IN read(fdSrc[%d-%s]) errno[%d-%s]", LT,
			fdSrc, sSourcePath, errno, strerror(errno));

		if(close(fdSrc) == -1)
		{
			log_print(LOGN_CRI, LH"FAILED IN close(fdSrc[%d-%s]) errno[%d-%s]", LT,
				fdSrc, sSourcePath, errno, strerror(errno));
		}

		if(close(fdDest) == -1)
		{
			log_print(LOGN_CRI, LH"FAILED IN close(fdDest[%d-%s]) errno[%d-%s]", LT,
				fdDest, sDestinationPath, errno, strerror(errno));
		}
		return -6;
	}

	if(close(fdSrc) == -1)
	{
		log_print(LOGN_CRI, LH"FAILED IN close(fdSrc[%d-%s]) errno[%d-%s]", LT,
			fdSrc, sSourcePath, errno, strerror(errno));
	}

	if(close(fdDest) == -1)
	{
		log_print(LOGN_CRI, LH"FAILED IN close(fdDest[%d-%s]) errno[%d-%s]", LT,
			fdDest, sDestinationPath, errno, strerror(errno));
	}

	return 0;
}

int dMakeIRMHash(void)
{
	int		dLine;
	char	szBuf[1024], szInfo_1[64], szInfo_2[64], szInfo_3[64];
	FILE	*fa;

	st_IMSIHash_Key		stIRMHashKey;
	st_IMSIHash_Key		*pKey = &stIRMHashKey;
	st_IMSIHash_Data	stIRMHashData;
	st_IMSIHash_Data	*pData = &stIRMHashData;
	stHASHONODE			*pHASHNODE;

	/* reset hash */
	hasho_reset(pIRMINFO);

	if( (fa = fopen(FILE_IRM, "r")) == NULL)
	{
		log_print(LOGN_CRI, LH"ERROR IN fopen(%s)"EH, LT, FILE_IRM, ET);
		return -1;
	}

	dLine = 0;
	while(fgets(szBuf,1024,fa) != NULL)
	{
		if(szBuf[0] != '#')
		{
			log_print(LOGN_CRI, LH"File=%s:%d ROW FORMAT ERR", LT, FILE_IRM, dLine+1);
			fclose(fa);
			return -2;
		}

		if(szBuf[1] == '#')
			continue;
		else if(szBuf[1] == 'E')
			break;
		else if(szBuf[1] == '@')
		{
			if(sscanf(&szBuf[2],"%s %s %s", szInfo_1, szInfo_2, szInfo_3) == 3)
			{
				log_print(LOGN_CRI, "READ IRM DATA i=%d INFO1=%s INFO2=%s INFO3=%s", dLine, szInfo_1, szInfo_2, szInfo_3);

				memset(pKey, 0x00, DEF_IMSIHASH_KEY_SIZE);
				memset(pData, 0x00, DEF_IMSIHASH_DATA_SIZE);

				if(strlen(szInfo_1) >= DEF_IMSI_PATTERN_SIZE)
				{
					log_print(LOGN_CRI, LH"szInfo_1[%s] is over DEF_IMSI_PATTERN_SIZE[%d]",  LT,
						szInfo_1, DEF_IMSI_PATTERN_SIZE);
					continue;
				}
				else
					strncpy((char*)pKey->sIMSI, szInfo_1, DEF_IMSI_PATTERN_SIZE);

				if(strlen(szInfo_2) >= DEF_IMSI_PATTERN_SIZE)
				{
					log_print(LOGN_CRI, LH"szInfo_2[%s] is over DEF_IMSI_PATTERN_SIZE[%d]",  LT,
						szInfo_2, DEF_IMSI_PATTERN_SIZE);
					continue;
				}
				else
					strncpy((char*)pData->sIRM, szInfo_2, DEF_IMSI_PATTERN_SIZE);

				if(strlen(szInfo_3) >= DEF_IMSI_PREFIX_SIZE)
				{
					log_print(LOGN_CRI, LH"szInfo_3[%s] is over DEF_IMSI_PREFIX_SIZE[%d]",  LT,
						szInfo_3, DEF_IMSI_PREFIX_SIZE);
					continue;
				}
				else
					strncpy((char*)pData->sPrefix, szInfo_3, DEF_IMSI_PREFIX_SIZE);

				if( (pHASHNODE = hasho_add(pIRMINFO, (U8 *)pKey, (U8 *)pData)) != NULL)
					log_print(LOGN_CRI, "HASH ADD IRM i=%d IMSI=%s IRM=%s PREFIX=%s", dLine, pKey->sIMSI, pData->sIRM, pData->sPrefix);
				else
					log_print(LOGN_CRI, "HASH ADD FAIL IRM i=%d IMSI=%s IRM=%s PREFIX=%s", dLine, pKey->sIMSI, pData->sIRM, pData->sPrefix);
			}
			else
				log_print(LOGN_CRI, LH"ERROR IN sscanf(%s) errno[%d-%s]", LT, &szBuf[2], errno, strerror(errno));
		}
		dLine++;
	}
	fclose(fa);

	return 0;
}

int dConvertIMSItoIRM(st_TraceList *pstTraceList)
{
	int					i, offset;
	U8					sResult[MAX_MIN_SIZE];
	size_t				szLen;

	st_IMSIHash_Key		stIRMHashKey;
	st_IMSIHash_Key		*pKey = &stIRMHashKey;
	st_IMSIHash_Data	*pData;
	stHASHONODE			*pHASHNODE;

	memcpy(pstTraceList, trace_tbl, sizeof(st_TraceList));

	for(i = 0; i < pstTraceList->count; i++)
	{
		switch(pstTraceList->stTraceInfo[i].dType)
		{
			case TRC_TYPE_ROAM_IMSI:
			case TRC_TYPE_ROAM_MDN:
				szLen	= strlen((char*)pstTraceList->stTraceInfo[i].stTraceID.szMIN);
				if( (szLen=(szLen>MAX_MIN_LEN)?MAX_MIN_LEN:szLen) >= DEF_IMSI_MIN_LEN)
				{
					memset(pKey, 0x00, DEF_IMSIHASH_KEY_SIZE);
					offset = szLen - DEF_IMSI_MIN_LEN;
					memcpy(pKey->sIMSI, &pstTraceList->stTraceInfo[i].stTraceID.szMIN[offset], DEF_IMSI_PATTERN_LEN);
					pKey->sIMSI[DEF_IMSI_PATTERN_LEN] = 0x00;
					offset = offset + DEF_IMSI_PATTERN_LEN;

					if((pHASHNODE = hasho_find(pIRMINFO, (U8 *)pKey)) != NULL)
					{
						pData = (st_IMSIHash_Data*)nifo_ptr(pIRMINFO, pHASHNODE->offset_Data);
						sprintf((char*)sResult, "%s%s", pData->sIRM, &pstTraceList->stTraceInfo[i].stTraceID.szMIN[offset]);
						log_print(LOGN_INFO, LH"szMIN[%s] sResult[%s]", LT,
							pstTraceList->stTraceInfo[i].stTraceID.szMIN, sResult);

						strncpy((char*)pstTraceList->stTraceInfo[i].stTraceID.szMIN, (char*)sResult, MAX_MIN_LEN);
					}
					else
						log_print(LOGN_CRI, LH"FAILED IN hasho_find(sIMSI[%s])", LT, pKey->sIMSI);
				}
				else
				{
					log_print(LOGN_INFO, LH"SHORT szLen[%lu] szMIN[%s]", LT,
						szLen, pstTraceList->stTraceInfo[i].stTraceID.szMIN);
				}
				break;
			default:
				break;
		}
	}

	return 0;
}

/*
 * dRecordModelCustCnt()
 * - 상관분석에서 단말모델에 대한 가입자별 비율을 표기하기 위한 함수
 * - model명을 입력 받고, 해당 model명이 있는 경우, counting,
 *                                       없는 경우, 새로 입력 후 counting

	typedef struct _st_Model{
		U8  szModel[MAX_MODEL_SIZE];
		U32 uiCount;
	} st_Model;

	typedef struct _st_Model_Stat{
		st_Model stModel[1000];
		U32 uiModelCnt;
	} st_Model_Stat;

*/
int dRecordModelCustCnt(st_Model_Stat *pstMS, char *pszModel, int len)
{
    int 	 		i, isMatched;
	unsigned int 	*pdCnt;
    st_Model 		*pstModel;

    log_print(LOGN_INFO,"insert Model=[%s:%d]", pszModel,len);

    pstModel = &pstMS->stModel[0];
    pdCnt    = &pstMS->uiModelCnt;

	if( *pdCnt+1 > MAX_MODEL_COUNT){
		log_print(LOGN_CRI,LH"FAILED insert MAX (%d) OVER", LT, MAX_MODEL_COUNT);
		return -1;
	}

    for( i = 0, isMatched = 0; i < *pdCnt; i++ ){
        if( strcmp( (char*)((pstModel+i)->szModel), pszModel ) == 0 ){
            (pstModel+i)->uiCount++;
            isMatched = 1;
            log_print(LOGN_INFO,"matched model=%s, cnt=%d", pszModel, (pstModel+i)->uiCount);
        }
    }

    if( isMatched != 1 ){
            memcpy(&(pstModel+*pdCnt)->szModel[0], pszModel, len);
            (pstModel+*pdCnt)->szModel[len] = 0x00;
            (pstModel+*pdCnt)->uiCount++;
            (*pdCnt)++;
            log_print(LOGN_INFO,"added model count %d -> %d", *pdCnt-1, *pdCnt);
    }
    return 0;
}

int dGetYYYYMMDD(char *pszDate)
{
	int       dRet;
	time_t 	  tCurr;
	struct tm stCurr;
	
	tCurr = time(NULL);

	if(localtime_r( (const time_t*)&tCurr, &stCurr) == NULL)
    {
        log_print(LOGN_CRI, LH"FAILED IN localtime_r(tCurr[%lu]) errno[%d-%s]", LT,
                tCurr, errno, strerror(errno));
        return -1;
    }

    if( (dRet = strftime(pszDate, 9, "%Y%m%d", &stCurr)) != 8)
    {
        log_print(LOGN_CRI, LH"FAILED IN strftime(szYYYYMMDD[%s]) errno[%d-%s]", LT,
                pszDate, errno, strerror(errno));
        return -2;
    }

	stCurr.tm_hour = 0;
    stCurr.tm_min  = 0;
    stCurr.tm_sec  = 0;
 
    return mktime( &stCurr);
}

int dCreateTBSql_ModelCustCnt(st_Model_Stat *pstMS)
{
	FILE *fp;
	int  i, dRet, dTime;
	/** DB table name 수정해야 함 */
	/** OUTPUT file name 도 수정해야 함 */
	/** szYYYYMMDD 를 구하는 공식 추가해야 함 */
	char szTable[] = "TB_MMDLCNTD", szFilePath[PATH_MAX], szFileName[PATH_MAX], szFullPath[PATH_MAX], szYYYYMMDD[9];
	st_Model *pstModel;

	if( (dTime = dGetYYYYMMDD(&szYYYYMMDD[0])) < 0 ){
		log_print(LOGN_CRI,"FAILED IN dGetYYYYMMDD(dRet=%d)",dTime);
		return -1;
	} 

	memset(szFilePath, 0x00, PATH_MAX);
	sprintf(szFilePath, "%s/TAMDB_LOG", START_PATH);
	sprintf(szFileName, "DQMS_FSQL_ID0001_T%s000000.SQL", szYYYYMMDD);
	sprintf(szFullPath, "%s/%s", szFilePath, szFileName);
	

	if( (fp= fopen(szFullPath, "w")) == NULL ){
        log_print(LOGN_CRI,LH"FAILED IN fopen, fn=%s",LT, szFullPath);
		/** errno 추가 */
		/** EEXIST 인 경우에 대한 처리? */
        return -2;
    }

	/* 이미 있는 table 제거 */
    fprintf(fp,"DELETE FROM %s;\n", szTable);

	/* create insert statement per each model */
    for( i = 0; i< pstMS->uiModelCnt; i++ ){
        pstModel = &pstMS->stModel[i];
        fprintf(fp,"INSERT INTO %s VALUES ('%s000000', %d, '%s', %d);\n", szTable, szYYYYMMDD, dTime, pstModel->szModel, pstModel->uiCount);
		/** error 처리가 필요할까... */

    }

	/* final commit statement */
    fprintf(fp,"COMMIT;\n");

    if( fp != NULL )
        fclose(fp);

	if( (dRet=dSendMsg_SIDB(szYYYYMMDD, szFilePath, szFileName) ) < 0 ){
		log_print(LOGN_CRI, LH"ERROR IN dSendMsg_SIDB() Fn=%s, dRet[%d]", LT, szFullPath, dRet);
        return -3;
	}

    return 0;

}

