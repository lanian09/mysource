/* SYS HEADER */
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/* LIB HEADER */
#include "clisto.h"		/* U8 */
#include "filedb.h"		/* MAX_CH_COUNT */
#include "loglib.h"
#include "utillib.h"	/* util_makenid() */
#include "nsocklib.h"	/* st_ClientInfo, st_FDInfo */
/* PRO HEADER */
#include "path.h"
#include "msgdef.h"		/* st_MsgQ */
#include "mmcdef.h"		/* mml_msg */
#include "sockio.h"		/* NTAFT, MAGIC_NUMBER */
#include "procid.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
#include "almstat.h"
/* LOC HEADER */
#include "if_sock.h"	/* dSendPacket() */
#include "if_func.h"	/* dGetNode(), dMsgsnd() */
#include "if_msgq.h"

/** B.1* DEFINITION OF NEW CONSTANTS **********************/
/** C.1* DEFINITION OF NEW TYPES **************************/
/** D.1* DECLARATION OF VARIABLES *************************/
extern pst_NTAM      fidb;
extern st_subsys     stSubSys[MAX_CH_COUNT];
extern st_ClientInfo stSock[MAX_CH_COUNT];

extern int gdSysNo;
/** E.1* DEFINITION OF FUNCTIONS **************************/
/** E.2* DEFINITION OF FUNCTIONS **************************/

int dIsRcvedMessage( pst_MsgQ pstMsg )
{
    int 	dRet;

	if( (dRet = dMsgrcv( &pstMsg )) < 0 ){
		if( dRet != -1 ){
			log_print(LOGN_CRI, LH"FAILED IN dMsgrcv(SI_SVC)",LT);
		}
		return NO_MSG;
	}
	return 1;
}

int dHandleMsg(st_ClientInfo *stNet, st_MsgQ *pstMsgQ, st_FDInfo *stFD, int dIndex)
{
	unsigned short	usSvcID, usMsgID;
	int				dRet, dType, dSize;
	pst_MsgQSub		pstMsgQSub;
	mml_msg			*ml;

	dSize		= 0;
	pstMsgQSub	= (pst_MsgQSub)&pstMsgQ->llMType;
	dType		= pstMsgQSub->usType;
	usSvcID		= pstMsgQSub->usSvcID;
	usMsgID		= pstMsgQSub->usMsgID;

	switch(usSvcID)
	{
		case SID_MML:
			ml = (mml_msg*)&pstMsgQ->szBody[0];
			switch(ml->msg_id)
			{
				case MI_MASK_NTAF_CHN:
					dMask_Ntaf_Chn(ml, pstMsgQ->llNID, dIndex);
					break;
				case MI_UMASK_NTAF_CHN:
					dUMask_Ntaf_Chn(ml, pstMsgQ->llNID, stNet);
					break;
				case MI_MASK_INTERLOCK_CHN:
					dMask_Interlock_Chn(ml, pstMsgQ->llNID, dIndex);
					break;
				case MI_UMASK_INTERLOCK_CHN:
					dUMask_Interlock_Chn(ml, pstMsgQ->llNID);
					break;
				case MI_GET_NTAF_INFO:
					dGet_Sub_Info(ml, pstMsgQ->llNID, dIndex);
					break;
				case MI_GET_NTAM_INFO:
				default:
					if(usMsgID == MID_MML_REQ)
					{
						ml = (mml_msg*)&pstMsgQ->szBody[0];
						switch(ml->msg_id)
						{
							case MI_DIS_SUB_PRC:
							case MI_ACT_SUB_PRC:
							case MI_DACT_SUB_PRC:
							case MID_MML_RST:
							case MI_DIS_SUB_SESS:
							case MI_DIS_SUB_SYS_INFO:
								log_print(LOGN_INFO, LH"usSvcID[%hu] usMsgID[%hu] MSG[%hu] PROID[%hu] >>> TLEN[%d]", 
									LT, usSvcID, usMsgID, ml->msg_id, pstMsgQ->ucProID, dSize);
								if( (dRet = dSndNtafMsg(stNet, pstMsgQ, stFD)) < 0)
									log_print(LOGN_CRI, LH"ERROR IN dSndNtafMsg() dRet[%d]", LT, dRet);
								return 0;
							default:
								log_print(LOGN_CRI, LH"MML NOT SUPPORT SID=SID_MML msg_id[%hu] usMsgID[%hu]", LT, ml->msg_id, usMsgID);
								return -1;
						}
					}
					log_print(LOGN_CRI, LH"MML NOT SUPPORT SID=SID_MML msg_id[%hu] usMsgID[%hu]", LT, ml->msg_id, usMsgID);
					return -1;
			}
			log_print(LOGN_INFO, LH"MML SVCID=%d MSGID=%d ProID=%d MML=%d", LT, usSvcID, usMsgID, pstMsgQ->ucProID, ml->msg_id);
			break;
		case SID_FLT:
			log_print(LOGN_INFO, LH"SVC=%d MSG=%d MSG >>> TLEN=%d", LT, usSvcID, usMsgID, dSize);
			if( (dRet = dSndNtafMsg(stNet, pstMsgQ, stFD)) < 0)
			{
				log_print(LOGN_INFO, LH"SND SOCK usSvcID[%hu] usMsgID[%hu] ucProID[%hu]", LT, usSvcID, usMsgID, pstMsgQ->ucProID);
				return -1;
			}
			log_print(LOGN_INFO, LH"SND SOCK usSvcID[%hu] usMsgID[%hu] ucProID[%hu]", LT, usSvcID, usMsgID, pstMsgQ->ucProID);
			break;
		default:
			log_print(LOGN_CRI, LH"Not Support usSvcID[%hu]", LT, usSvcID);
			break;
	}

	return 0;
}

int dSndNtafMsg(st_ClientInfo *stNet, st_MsgQ *pstMsgQ, st_FDInfo *stFD)
{
	unsigned short	usSvcID, usMsgID;
	int				dRet, dIdx, size;
	char			szBuf[1024 * 10];

	pst_NTAFTHeader	pstHeader;
	pst_MsgQSub		pstMsgQSub;

	pstMsgQSub = (pst_MsgQSub)&pstMsgQ->llMType;
	usSvcID	= pstMsgQSub->usSvcID;
	usMsgID	= pstMsgQSub->usMsgID;

	pstHeader	= (pst_NTAFTHeader)szBuf;
	size		= pstMsgQ->usBodyLen;
	pstHeader->llMagicNumber	= MAGIC_NUMBER;
	pstHeader->usTotlLen		= size;
	pstHeader->usBodyLen		= pstMsgQ->usBodyLen - NTAFT_HEADER_LEN;
	pstHeader->ucNTAFID			= pstMsgQ->ucNTAFID;
	pstHeader->ucSvcID			= usSvcID;
	pstHeader->ucMsgID			= usMsgID;
	pstHeader->llIndex			= pstMsgQ->llIndex;

	log_print(LOGN_INFO, "F=%s:%s.%d: CALLSTOP MSG >>> TLEN[%d] BodyLEN[%d]", __FILE__, __FUNCTION__, __LINE__,
		size, pstHeader->usBodyLen);

	if( (dIdx = dGetSubSfd(stNet, pstMsgQ->ucNTAFID)) < 0)
	{
		log_print(LOGN_WARN, "F=%s:%s.%d: Not Connect Client dIdx=%d SystemNo=%d ProID=%d MSGID=%d", __FILE__, __FUNCTION__, __LINE__,
			dIdx, pstMsgQ->ucNTAFID, pstMsgQ->ucProID, usMsgID);
		return -1;
	}

	memcpy(&szBuf[NTAFT_HEADER_LEN], &pstMsgQ->szBody[NTAFT_HEADER_LEN], (size-NTAFT_HEADER_LEN));
	if( (dRet = dSendPacket(stNet, dIdx, stFD, szBuf, size)) < 0)
	{
		log_print(LOGN_WARN, "F=%s:%s.%d: Not Connect Client dIdx[%d] SystemNo[%d] ProID[%d]", __FILE__, __FUNCTION__, __LINE__,
			dIdx, pstMsgQ->ucNTAFID, pstMsgQ->ucProID);
		return -2;
	}
	log_print(LOGN_INFO, "F=%s:%s.%d: SND SOCK SVCID[%d] MSGID[%d] ProID[%d]", __FILE__, __FUNCTION__, __LINE__, usSvcID, usMsgID, pstMsgQ->ucProID);

	return 0;
}

int dHandleMsgAll(st_ClientInfo *stNet, st_MsgQ *pstMsgQ, st_FDInfo *stFD, int dIndex)
{
	unsigned short	usSvcID, usMsgID;
	int				i, dRet, dType, size;
	char			szBuf[10240];
	st_NTAFTHeader	*pstHeader;
	st_MsgQSub		*pstMsgQSub;

	pstMsgQSub	= (pst_MsgQSub)&pstMsgQ->llMType;
	dType		= pstMsgQSub->usType;
	usSvcID		= pstMsgQSub->usSvcID;
	usMsgID		= pstMsgQSub->usMsgID;
	pstHeader	= (pst_NTAFTHeader)szBuf;

	size		= pstMsgQ->usBodyLen;

	pstHeader->llMagicNumber	= MAGIC_NUMBER;
	pstHeader->usTotlLen		= size+NTAFT_HEADER_LEN;
	pstHeader->usBodyLen		= pstMsgQ->usBodyLen;
	pstHeader->ucNTAFID			= pstMsgQ->ucNTAFID;
	pstHeader->ucSvcID			= usSvcID;
	pstHeader->ucMsgID			= usMsgID;
	pstHeader->llIndex			= pstMsgQ->llIndex;

	log_print(LOGN_INFO, LH"Send All MSG >>> SVCID=%d, MSGID=%d, TLEN=%d, BLEN=%d", LT, usSvcID, usMsgID, size, pstHeader->usBodyLen);

	memcpy(&szBuf[NTAFT_HEADER_LEN], &pstMsgQ->szBody[0], pstHeader->usBodyLen);

	for(i = 0; i < MAX_CH_COUNT; i++)
	{
		if( (stNet[i].uiIP>0) && (stNet[i].dSfd>0) && (stNet[i].dSysNo<(MAX_CH_COUNT+1)))
		{
			if( (dRet = dSendPacket(stNet, i, stFD, szBuf, size)) < 0)
			{
				log_print(LOGN_CRI, LH"FAILED IN dHandleMsg dRet=%d", LT, dRet);
				continue;
			}
			log_print(LOGN_DEBUG, "MSG SEND SUCCESS, Probing SysNo=%d, SVCID=%d MSGID=%d", stNet[i].dSysNo, usSvcID, usMsgID);
		}
	}
	log_print(LOGN_INFO, LH"SND SOCK SVCID=%d, MSGID=%d, ProID=%d", LT, usSvcID, usMsgID, pstMsgQ->ucProID);

	return 0;
}

int dSendToProc(st_ClientInfo *stNet, int dIdx, char *szBuf, pst_NTAFTHeader pstHeader)
{
	int                 dRet, dSeqProcID;
	unsigned short      usSvcID, usMsgID;

	pst_MsgQ            pstMsgQ;
	pst_MsgQSub			pstMsgQSub;
	dbm_msg_t 		   *smsg;
	U8				   *pNODE;

	if(pstHeader->llMagicNumber != MAGIC_NUMBER)
	{
		log_print(LOGN_CRI, "dSendToProc : MAGIC NUMBER ERROR, Will be Reset[%lld][%lld]",
			MAGIC_NUMBER, pstHeader->llMagicNumber);
		return -1;
	} /* end of if */

	usSvcID		= pstHeader->ucSvcID;
	usMsgID		= pstHeader->ucMsgID;

	if( dGetNode( &pNODE, &pstMsgQ ) < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN dGetNode(SI_SVC)",LT);
		return -2;
	}

	pstMsgQSub	= (pst_MsgQSub)&pstMsgQ->llMType;
	pstMsgQSub->usType	= DEF_SYS;
	pstMsgQSub->usSvcID	= usSvcID;
	pstMsgQSub->usMsgID	= usMsgID;

	pstMsgQ->llIndex	= pstHeader->llIndex;
	pstMsgQ->ucNTAFID	= pstHeader->ucNTAFID;
	pstMsgQ->usRetCode	= 0;
	pstMsgQ->ucProID	= SEQ_PROC_SI_SVC;
	pstMsgQ->dMsgQID	= 0;
	pstMsgQ->usBodyLen	= pstHeader->usBodyLen;

	log_print(LOGN_INFO, "RECV TAF=%d TOT=%d BLEN=%d SVCID=%d MSGID=%d",
		pstHeader->ucNTAFID, pstHeader->usTotlLen, pstHeader->usBodyLen, usSvcID, usMsgID);

	util_makenid(SEQ_PROC_SI_SVC, &pstMsgQ->llNID);

	if( (pstHeader->ucNTAFID <= 0) || (pstHeader->ucNTAFID > MAX_CH_COUNT))
	{
		log_print(LOGN_CRI, "RECV ERR TAF NO=%d", pstHeader->ucNTAFID);
		return 0;
	}

	switch(usSvcID)
	{
		case SID_STATUS:
			if( (usMsgID == MID_ALARM) || (usMsgID == MID_TRAFFIC))
				dSeqProcID = SEQ_PROC_ALMD;
			else if(usMsgID == MID_CONSOL)
				dSeqProcID = SEQ_PROC_COND;
			else
			{
				log_print(LOGN_CRI, "NOT SUPPORT MSGID SVCID=%d MSGID=%d TAFNO=%d BODYLEN=%d",
					usSvcID, usMsgID, pstMsgQ->ucNTAFID, pstMsgQ->usBodyLen);
				return 0;
			}

			memcpy(pstMsgQ->szBody, &szBuf[NTAFT_HEADER_LEN], pstHeader->usBodyLen);
			if( (dRet = dMsgsnd(dSeqProcID, pNODE)) < 0){
				log_print(LOGN_CRI, LH"SENDQ ERR=%d SVCID=%d MSGID=%d TAFNO=%d BODYLEN=%d", 
					LT, dRet, usSvcID, usMsgID, pstMsgQ->ucNTAFID, pstMsgQ->usBodyLen);
			} else {
				log_print(LOGN_INFO, "SENDQ SVCID=%d MSGID=%d TAFNO=%d BODYLEN=%d", 
					usSvcID, usMsgID, pstMsgQ->ucNTAFID, pstMsgQ->usBodyLen);
			}
			break;

		case SID_MML:
			dSeqProcID = SEQ_PROC_MMCD;
			memcpy(pstMsgQ->szBody, szBuf+NTAFT_HEADER_LEN, pstHeader->usBodyLen);
			smsg	= (dbm_msg_t*)pstMsgQ->szBody;
			if( (dRet = dMsgsnd(dSeqProcID, pNODE)) < 0) {
				log_print(LOGN_CRI, LH"FAIL SND MMCD=%d SVCID=%d MSGID=%d BODYLEN=%d CID=%d MID=%d", 
					LT, dSeqProcID, usSvcID, usMsgID, pstMsgQ->usBodyLen, smsg->head.cmd_id, smsg->head.msg_id);
			} else {
				log_print(LOGN_INFO, "SND MMCD=%d SVCID=%d MSGID=%d BODYLEN=%d CID=%d MID=%d", 
					dSeqProcID, usSvcID, usMsgID, pstMsgQ->usBodyLen, smsg->head.cmd_id, smsg->head.msg_id);
			}
			break;

		case SID_FLT:
		case SID_CHKRES:

			if( (pstMsgQ->ucNTAFID = dGetSysNo(stNet, dIdx)) <= 0)
			{
				log_print(LOGN_WARN, LH"dSendToProc : NOT REGISTER NTAF=%d IP=%u", LT, pstMsgQ->ucNTAFID, stNet[dIdx].uiIP);
				return 0;
			}

			dSeqProcID = SEQ_PROC_S_MNG;
			memcpy(pstMsgQ->szBody, szBuf + NTAFT_HEADER_LEN, pstHeader->usBodyLen);
			if( (dRet = dMsgsnd(dSeqProcID, pNODE)) < 0){
				log_print(LOGN_CRI, LH"FAIL=%d SND S_MNG=%d SVCID=%d MSGID=%d BODYLEN=%d", 
					LT, dRet, dSeqProcID, usSvcID, usMsgID, pstMsgQ->usBodyLen);
			} else {
				log_print(LOGN_INFO, "SND S_MNG=%d SVCID=%d MSGID=%d BODYLEN=%d", dSeqProcID, usSvcID, usMsgID, pstMsgQ->usBodyLen);
			}
			break;

		default:
			log_print(LOGN_CRI, "dSendToProc : NOT SUPPORT SVCID=%d MSGID=%d", usSvcID, usMsgID);
			return 0;
	} /* end of switch */

	return 0;
} /* end of dSendToProc */

int dGetSubSfd(st_ClientInfo *stNet, int dSysNo)
{
	int		i;

	for(i = 0; i < MAX_CH_COUNT; i++)
	{
		if((stNet[i].dSysNo == dSysNo) && (stNet[i].dSfd > 0))
			return i;
	}

	return -1;
}

int dGetSysNo(st_ClientInfo *stNet, int dIdx)
{
    unsigned int i, uiIP;

    uiIP = stNet[dIdx].uiIP;

    for(i = 0; i < MAX_CH_COUNT; i++){/* 20050426  ÀÌÀç½Â */

        if(stSubSys[i].uiIP == uiIP){
			return  stSubSys[i].usSysNo;
		}
    }

    return -1;
}

int dGetSysNoWithIP(unsigned int uiIP)
{
    unsigned int i;

    for(i = 0; i < MAX_CH_COUNT; i++){

        if(stSubSys[i].uiIP == uiIP){
			return  stSubSys[i].usSysNo;
		}
    }

    return -1;
}

int dMask_Ntaf_Chn(mml_msg *ml, long long llNID, int dIndex)
{
	int 		dSysNo, i;
	dbm_msg_t 	smsg;

	smsg.data[0] = 0x00;
	dSysNo = atoi(ml->msg_body[0].para_cont);
	for(i = 0 ; i < MAX_CH_COUNT; i++)
	{
		if(stSubSys[i].usSysNo == dSysNo) {
			if(fidb->NTAFChnl[i] == MASK) {
				log_print(LOGN_DEBUG, "NTAF=%d POS=%d CHN=%d ALREADY MASKED",
					dSysNo, i, fidb->NTAFChnl[i]);
				smsg.common.mml_err = eAlreadyMaskChn;
			} else {
				log_print(LOGN_DEBUG, "NTAF=%d POS=%d CHN MASKED : B=%d C=%d ",
					dSysNo, i, fidb->NTAFChnl[i], MASK);
				fidb->NTAFChnl[i] = MASK;
				smsg.common.mml_err = DBM_SUCCESS;
			}
			break;
		}
	}

	if(i == MAX_CH_COUNT) {
		smsg.common.mml_err = eNotFindSysNo;
		log_print(LOGN_CRI, "NTAF=%d NOT MATCH=%d", dSysNo, i);
	}

	smsg.common.cont_flag = DBM_END;
	smsg.head.msg_len = 0;

	dSendToMMCD(ml, &smsg, llNID);

	return 0;
}

int dUMask_Ntaf_Chn(mml_msg *ml, long long llNID, st_ClientInfo *stNet)
{
	int			dSysNo, i;
	dbm_msg_t	smsg;

	smsg.data[0] = 0x00;
	dSysNo = atoi(ml->msg_body[0].para_cont);

	log_print(LOGN_DEBUG, "UMASK :: SYSNO=%d", dSysNo);
	if(fidb->NTAFChnl[dSysNo-1] != MASK)
	{
		smsg.common.mml_err = eAlreadyUMaskChn;
		log_print(LOGN_DEBUG, "UMASK:: NTAF=%d CHN=%d ALREADY UNMASKED", dSysNo, fidb->NTAFChnl[dSysNo-1]);
	}
	else
	{
		for(i = 0; i < MAX_CH_COUNT; i++)
		{
			if(stSock[i].dSysNo == dSysNo && stSock[i].uiIP == stSubSys[dSysNo-1].uiIP)
			{
				if(stSock[i].dLastFlag == SOCKET_OPEN)
					fidb->NTAFChnl[dSysNo-1] = NORMAL;
				else
					fidb->NTAFChnl[dSysNo-1] = CRITICAL;
				smsg.common.mml_err = DBM_SUCCESS;
				log_print(LOGN_WARN, "UMASK :: SET CURRENT stSock[i=%d].SysNo=%d:%d SubSys.uiIP=%u EXECT FLAG=%d STATUS=%d",
					i, stSock[i].dSysNo, dSysNo, stSubSys[dSysNo-1].uiIP, stSock[i].dLastFlag, fidb->NTAFChnl[dSysNo-1]);
				break;
			}
			else if(stSock[i].dSysNo == dSysNo && stSock[i].uiIP != stSubSys[dSysNo-1].uiIP)
			{
				log_print(LOGN_CRI, "UMASK :: stSock[i=%d].SysNo=%d:InSysN=%d Memory differ SubSys[dSysNo=%d -1].uiIP=%u",
					i, stSock[i].dSysNo, dSysNo, dSysNo, stSubSys[dSysNo-1].uiIP);
				exit(0);
			}
		}

		if(i == MAX_CH_COUNT)
		{
			if(stSubSys[dSysNo-1].usSysNo != dSysNo)
			{
				smsg.common.mml_err = eNotFindSysNo;
				log_print(LOGN_WARN, "UMASK :: NTAF=%d NOT MATCH=%d", dSysNo, i);
			}
			else
			{
				fidb->NTAFChnl[dSysNo-1] = CRITICAL;
				smsg.common.mml_err = DBM_SUCCESS;
				log_print(LOGN_WARN, "UMASK :: NTAF=%d POS=%d->MAX_CH_COUNT=%d CHN=%d UNMASKED TO CRITICAL", dSysNo, i, MAX_CH_COUNT, fidb->NTAFChnl[i]);
			}
		}
	}
	smsg.common.cont_flag = DBM_END;
	smsg.head.msg_len = 0;
	dSendToMMCD(ml, &smsg, llNID);

	return 0;
}

int dMask_Interlock_Chn(mml_msg *ml, long long llNID, int dIndex)
{
	char		sInterlock[BUF_SIZE];
	int			dInterlock, dRet;
	dbm_msg_t	smsg;

	smsg.data[0] = 0x00;
	if(strlen(ml->msg_body[0].para_cont)<BUF_SIZE)
		strncpy(sInterlock, ml->msg_body[0].para_cont, BUF_SIZE);
	else
	{
		strncpy(sInterlock, ml->msg_body[0].para_cont, BUF_SIZE-1);
		sInterlock[BUF_SIZE-1] = 0x00;
	}

	if(strcasecmp(sInterlock, "TAM_DB") == 0)
		dInterlock = SI_DB_INTERLOCK;
	else if(strcasecmp(sInterlock, "D_NMS") == 0)
		dInterlock = SI_DNMS_INTERLOCK;
	else if(strcasecmp(sInterlock, "NMS") == 0)
		dInterlock = SI_NMS_INTERLOCK;
	else
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: Unknown Parameter[%s]", __FILE__, __FUNCTION__, __LINE__, sInterlock);

		smsg.common.mml_err		= eBadParameter;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= 0;
		if( (dRet = dSendToMMCD(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN dSendToMMCD() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
			return -1;
		}
		return -2;
	}

	if(fidb->cInterlock[dInterlock] == MASK)
	{
		log_print(LOGN_DEBUG, "F=%s:%s.%d: CInterlock[%d-%s] IS ALREADY MASKED[0x%02X]", __FILE__, __FUNCTION__, __LINE__,
			dInterlock, sInterlock, fidb->cInterlock[dInterlock]);
		smsg.common.mml_err = eAlreadyMaskChn;
	}
	else
	{
		log_print(LOGN_DEBUG, "F=%s:%s.%d: CInterlock[%d-%s] IS MASKED[0x%02X->0x%02X]", __FILE__, __FUNCTION__, __LINE__,
			dInterlock, sInterlock, fidb->cInterlock[dInterlock], MASK);
		fidb->cInterlock[dInterlock] = MASK;
		smsg.common.mml_err = DBM_SUCCESS;
	}
	smsg.common.cont_flag	= DBM_END;
	smsg.head.msg_len		= 0;
	if( (dRet = dSendToMMCD(ml, &smsg, llNID)) < 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN dSendToMMCD() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		return -3;
	}

	return 0;
}

int dUMask_Interlock_Chn(mml_msg *ml, long long llNID)
{
	char		sInterlock[BUF_SIZE];
	int			dInterlock, dRet;
	dbm_msg_t	smsg;

	smsg.data[0] = 0x00;
	if(strlen(ml->msg_body[0].para_cont)<BUF_SIZE)
		strncpy(sInterlock, ml->msg_body[0].para_cont, BUF_SIZE);
	else
	{
		strncpy(sInterlock, ml->msg_body[0].para_cont, BUF_SIZE-1);
		sInterlock[BUF_SIZE-1] = 0x00;
	}

	if(strcasecmp(sInterlock, "TAM_DB") == 0)
		dInterlock = SI_DB_INTERLOCK;
	else if(strcasecmp(sInterlock, "D_NMS") == 0)
		dInterlock = SI_DNMS_INTERLOCK;
	else if(strcasecmp(sInterlock, "NMS") == 0)
		dInterlock = SI_NMS_INTERLOCK;
	else
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: Unknown Parameter[%s]", __FILE__, __FUNCTION__, __LINE__, sInterlock);

		smsg.common.mml_err		= eBadParameter;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= 0;
		if( (dRet = dSendToMMCD(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN dSendToMMCD() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
			return -1;
		}
		return -2;
	}

	log_print(LOGN_DEBUG, "F=%s:%s.%d: sInterlock[%s] dInterlock[%d]", __FILE__, __FUNCTION__, __LINE__, sInterlock, dInterlock);
	if(fidb->cInterlock[dInterlock] != MASK)
	{
		smsg.common.mml_err = eAlreadyUMaskChn;
		log_print(LOGN_DEBUG, "F=%s:%s.%d: CInterlock[%d-%s] IS ALREADY UNMASKED[0x%02X]", __FILE__, __FUNCTION__, __LINE__,
			dInterlock, sInterlock, fidb->cInterlock[dInterlock]);
	}
	else
	{
		fidb->cInterlock[dInterlock] = NORMAL;
		smsg.common.mml_err = DBM_SUCCESS;
	}

	smsg.common.cont_flag	= DBM_END;
	smsg.head.msg_len		= 0;
	if( (dRet = dSendToMMCD(ml, &smsg, llNID)) < 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN dSendToMMCD() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		return -3;
	}

	return 0;
}

int dSendToMMCD(mml_msg *mmsg, dbm_msg_t *smsg, long long llNID)
{
    pst_MsgQ        pstSndMsg;
	pst_MsgQSub		pstMsgQSub;
    dbm_msg_t       testmsg;
	U8			   *pNODE;

	if( dGetNode( &pNODE, &pstSndMsg ) < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN dGetNode(SI_SVC)", LT);
		return -1;
	}

    smsg->head.src_proc     = SEQ_PROC_SI_SVC;
    smsg->head.dst_func     = mmsg->src_proc;
    smsg->head.dst_proc     = mmsg->src_func;
    smsg->head.cmd_id       = mmsg->cmd_id;
    smsg->head.msg_id       = mmsg->msg_id;

	pstMsgQSub = (pst_MsgQSub)&pstSndMsg->llMType;

    pstMsgQSub->usType  = DEF_SYS;
    pstMsgQSub->usSvcID = SID_MML;
    pstMsgQSub->usMsgID = MID_MML_RST;

	pstSndMsg->llNID     = llNID;
    pstSndMsg->ucProID   = SEQ_PROC_SI_SVC;
    pstSndMsg->llIndex   = 1;
    pstSndMsg->dMsgQID   = 0;
    pstSndMsg->usBodyLen = DEF_DBMMSG_SIZE- MSG_DATA_LEN + smsg->head.msg_len;
    pstSndMsg->usRetCode = 0;

    memcpy(pstSndMsg->szBody, smsg, pstSndMsg->usBodyLen);

	if( dMsgsnd( SEQ_PROC_MMCD, pNODE ) < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN dMsgsnd(to MMCD)"EH,LT,ET);
        return -2;
	}

	memcpy(&testmsg, &pstSndMsg->szBody[0], DEF_DBMMSG_SIZE);//FOR TEST? DEBUG?
	log_print(LOGN_DEBUG,
		"dSendToMMCD=%d][SEND OK [%d] : MSGLEN[%d] BODYLEN[%d]\n%s",
		SEQ_PROC_MMCD,
		smsg->head.cmd_id, smsg->head.msg_len, pstSndMsg->usBodyLen, testmsg.data);

    return 0;
}

int dGet_Sub_Info(mml_msg *ml, long long llNID, int dIndex)
{
    int			dSlen;
	char		szBuf[MSG_DATA_LEN];
	dbm_msg_t 	smsg;

    smsg.data[0] = 0x00;
	memset(szBuf, 0x00, MSG_DATA_LEN);
	smsg.common.mml_err = get_subsys_info(FILE_SUB_SYS, smsg.data);
	dSlen = strlen(smsg.data) + 1;
    smsg.common.cont_flag = DBM_END;
    smsg.head.msg_len = dSlen;

    if(dSendToMMCD(ml, &smsg, llNID) < 0) {
		log_print(LOGN_CRI, "dGet_Sub_Info : Failed in dSendMMC");
        return -1;
	}

    return 0;
}

int	get_subsys_info(char *szFileName, char *szData)
{
	FILE 	*fa;
	int		dCnt, dRet;
    char 	szBuf[1024], szInfo_1[64], szInfo_2[64], szInfo_3[64], szInfo_4[64];
    char 	szInfo_5[64], szInfo_6[64],	szStr[1024], szTemp[100];

	if( (fa = fopen(szFileName, "r")) == NULL)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN fopen(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			szFileName, errno, strerror(errno));
        return eDataReadError;
	}
	else
		log_print(LOGN_DEBUG, "F=%s:%s.%d: SUCCESS IN fopen(%s)", __FILE__, __FUNCTION__, __LINE__, szFileName);

    sprintf(szData,
		"-------------------------------------------\n"
		" NTAM NO = %d\n"
		"-------------------------------------------\n"
		" SYSNO IPADDR          ALIAS\n"
		"-------------------------------------------\n",
		gdSysNo);

	dCnt = 0;
    while(fgets(szBuf,1024,fa) != NULL)
    {
        if(szBuf[0] != '#')
		{
            log_print(LOGN_CRI, "F=%s:%s.%d: File(%s) row format error", __FILE__, __FUNCTION__, __LINE__, szFileName);
            fclose(fa);
            return eDataReadError;
        }

        if(szBuf[1] == '#')
            continue;
        else if(szBuf[1] == 'E')
            break;
        else if(szBuf[1] == '@')
		{
			if( (dRet = sscanf(&szBuf[2], "%s %s %s %s %s %s", szInfo_1,szInfo_2,szInfo_3,szInfo_4,szInfo_5, szInfo_6)) == 6)
			{
				if(strcmp(szInfo_1, "TMF") == 0)
				{
					sprintf(szStr, " %5d %-15s %-10s\n", atoi(szInfo_3), szInfo_4, szInfo_6);
					strcat(szData, szStr);
					dCnt++;
				}
			}
			else if(dRet == 5)
			{
				log_print(LOGN_DEBUG, "F=%s:%s.%d: Failed To Get Alias[%s]", __FILE__, __FUNCTION__, __LINE__, szBuf);
				if(strcmp(szInfo_1, "TMF") == 0)
				{
					sprintf(szStr, " %5d %-15s %-10s\n", atoi(szInfo_3), szInfo_4, szInfo_6);
					strcat(szData, szStr);
					dCnt++;
				}
			}
        }
    }
	sprintf(szTemp, "-------------------------------------------\n TOTAL = %d", dCnt);
	strcat(szData, szTemp);
    fclose(fa);

    return DBM_SUCCESS;
}
