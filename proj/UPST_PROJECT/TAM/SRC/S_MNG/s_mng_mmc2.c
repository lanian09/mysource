/**A.1*  FILE INCLUSION *******************************************************/
#include <linux/limits.h>	/*	PATH_MAX	*/
#include <errno.h>
#include <unistd.h>				/* USLEEP(3) */

#include "s_mng_mmc.h"
#include "s_mng_dft.h"
#include "s_mng_init.h"		/* dInit_WatchInfoDefectThreshold() */
#include "s_mng_msg.h"		/* dSendMMC() */
#include "s_mng_func.h"		/* dCopy(), dSendMsg_SI_SVCMON() */
#include "s_mng_util.h"		/* dGetTimeFromStr() */
#include "s_mng_flt.h"		/* st_Sub_GI_NTAF_List */
#include "s_mng_mon.h"

#include <watch_mon.h>		/*	SYSTEM_TYPE_PCF, SYSTEM_TYPE_PDSN	*/
#include "db_api.h"
#include "db_struct.h"
#include "db_define.h"
#include "sockio.h"

#include "commdef.h"
#include "procid.h"
#include "path.h"
#include "msgdef.h"
#include "sockio.h"			/* st_subsys_mng */	

#include "mems.h"
#include "cifo.h"
#include "nifo.h"
#include "gifo.h"

#include "loglib.h"
#include "utillib.h"

/** B.1* DEFINITION OF NEW CONSTANTS **********************/
/*	END: Writer: Han-jin Park, Date: 2009.05.07	*/

/** C.1* DEFINITION OF NEW TYPES **************************/
/** D.1* DECLARATION OF VARIABLES *************************/
extern stMEMSINFO				*pMEMSINFO;
extern stCIFO					*gpCIFO;
extern MYSQL					stMySQL;
extern st_MonTotal				*gMonTotal;
extern st_MonTotal_1Min			*gMonTotal1Min;
extern st_subsys_mng			*pstSubSys;

/** E.1* DEFINITION OF FUNCTIONS **************************/
/** E.2* DEFINITION OF FUNCTIONS **************************/
int dis_flt_sctp(mml_msg *ml, long long llNID)
{
	int				i, j, dRet, dLoopCnt, dCnt, dIdx;
	size_t			szWhereLen;
	unsigned int	uIP;
	unsigned short	huNetMask;
	unsigned char	cSysType, cDirection, cExistIP, cExistNetMask, cExistSysType, cExistDirection, cNeedComma;
	char			sWhere[BUFSIZ];
	dbm_msg_t		smsg;
	st_SCTP_DB		stSCTPDB[MAX_SCTP_COUNT];
	st_SCTP_MMC		stSCTPList;

	dCnt	= 0;
	dIdx	= 0;

	cExistIP		= 0;
	cExistNetMask	= 0;
	cExistSysType	= 0;
	cExistDirection	= 0;
	cNeedComma		= 0;

	memset(&smsg, 0x00, sizeof(dbm_msg_t));
	memset(&stSCTPDB, 0x00, sizeof(st_SCTP_DB)*MAX_SCTP_COUNT);
	memset(&stSCTPList, 0X00, sizeof(st_SCTP_MMC));

	for(i = 0; i < ml->num_of_para; i++)
	{
		switch(ml->msg_body[i].para_id)
		{
			case 22:
				if( (dRet = inet_pton(AF_INET, ml->msg_body[i].para_cont, &uIP)) <= 0)
				{
					log_print(LOGN_CRI, LH"FAILED IN inet_pton(%s) dRet[%d]", LT, ml->msg_body[0].para_cont, dRet);

					set_dbm_ret(&smsg, eINVALID_IP, DBM_END, 0);
					if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
					{
						log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
						return -1;
					}
					return -2;
				}
				uIP			= ntohl(uIP);
				cExistIP	= 1;
				break;
			case 65:
				huNetMask		= (unsigned short)atoi(ml->msg_body[i].para_cont);
				cExistNetMask	= 1;
				break;
			case 707:
				cSysType		= (unsigned char)atoi(ml->msg_body[i].para_cont);
				cExistSysType	= 1;
				break;
			case 701:
				cDirection		= (unsigned char)atoi(ml->msg_body[i].para_cont);
				cExistDirection	= 1;
				break;
			default:
				log_print(LOGN_WARN, LH"INVALID para_id[%hu] para_cont[%s]", LT,
					ml->msg_body[i].para_id, ml->msg_body[i].para_cont);
		}
	}

	if(ml->num_of_para)
	{
		sprintf(sWhere, " WHERE ");
		szWhereLen	= strlen(sWhere);

		if(cExistIP && cExistNetMask)
		{
			sprintf(&sWhere[szWhereLen], "((IP>>(32-%hu))<<(32-%hu))=((%u>>(32-%hu))<<(32-%hu))", huNetMask, huNetMask, uIP, huNetMask, huNetMask);
			cNeedComma	= 1;
		}
		else if(cExistIP)
		{
			sprintf(&sWhere[szWhereLen], "IP=%u", uIP);
			cNeedComma	= 1;
		}

		if(cExistSysType)
		{
			if(cNeedComma)
			{
				szWhereLen = strlen(sWhere);
				sprintf(&sWhere[szWhereLen], " AND ");
			}
			else
				cNeedComma	= 1;

			szWhereLen = strlen(sWhere);
			sprintf(&sWhere[szWhereLen], "SYSTYPE=%hu", cSysType);
		}

		if(cExistDirection)
		{
			if(cNeedComma)
			{
				szWhereLen = strlen(sWhere);
				sprintf(&sWhere[szWhereLen], " AND ");
			}
			else
				cNeedComma	= 1;

			szWhereLen = strlen(sWhere);
			sprintf(&sWhere[szWhereLen], "DIRECTION=%hu", cDirection);
		}

		szWhereLen = strlen(sWhere);
		sprintf(&sWhere[szWhereLen], " ");

		log_print(LOGN_DEBUG, LH"sWhere[%s]", LT, sWhere);
	}
	else
	{
		szWhereLen	= 0;
		sWhere[0]	= 0x00;
	}

	if( (dRet = dGetSCTPInfo(&stMySQL, stSCTPDB, &dCnt, sWhere)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dGetSCTPInfo() dRet[%d]", LT, dRet);
		smsg.common.mml_err		= eDBQUERYERROR;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= 0;
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -3;
		}
		return -4;
	}

	if (dCnt >= MAX_SCTP_COUNT)
	{
		sprintf(smsg.data, "OVER MAX FLT_SCTP COUNT[%d]", MAX_SCTP_COUNT);

		smsg.common.mml_err		= eOVERMAXROW;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= strlen(smsg.data)+1;
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -5;
		}
		return -6;
	}

	if( (dCnt % DEF_SCTP_MMC) == 0)
		dLoopCnt	= dCnt/DEF_SCTP_MMC;
	else
		dLoopCnt	= (dCnt/DEF_SCTP_MMC)+1;

	if(dCnt == 0)
	{
		smsg.common.mml_err		= DBM_SUCCESS;
		smsg.common.cont_flag	= DBM_END;

		memcpy(smsg.data, &stSCTPList, sizeof(st_SCTP_MMC));

		smsg.head.msg_len = sizeof(st_SCTP_MMC);
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -7;
		}
	}

	for(i = 0; i < dLoopCnt; i++)
	{
		for(j = 0; j < DEF_SCTP_MMC; j++)
		{
			stSCTPList.stSCTP[j].uSCTPIP			= stSCTPDB[dIdx].uSCTPIP;
			stSCTPList.stSCTP[j].cSysType			= stSCTPDB[dIdx].cSysType;
			stSCTPList.stSCTP[j].cDirection			= stSCTPDB[dIdx].cDirection;
			stSCTPList.stSCTP[j].huGroupID			= stSCTPDB[dIdx].huGroupID;

			strcpy(stSCTPList.stSCTP[j].szDesc, stSCTPDB[dIdx].szDesc);

			log_print(LOGN_DEBUG, "[SEND ALL SCTPINFO IDX[%d] uSCTPIP[%u] cSysType[%hu] cDirection[%hu] huGroupID[%hu] szDesc[%s]",
				dIdx+1, stSCTPList.stSCTP[j].uSCTPIP, stSCTPList.stSCTP[j].cSysType, stSCTPList.stSCTP[j].cDirection,
				stSCTPList.stSCTP[j].huGroupID, stSCTPList.stSCTP[j].szDesc);

			if( (dIdx+1) == dCnt)
				break;
			dIdx++;
		}
		smsg.common.TotPage	= dLoopCnt;
		smsg.common.CurPage	= i + 1;

		if(j == DEF_SVC_MMC)
			stSCTPList.dCount = DEF_SVC_MMC;
		else
			stSCTPList.dCount = j + 1;

		memcpy(smsg.data, &stSCTPList, sizeof(st_SvcInfoList));
		smsg.common.mml_err = DBM_SUCCESS;
		if(i == (dLoopCnt-1))
			smsg.common.cont_flag = DBM_END;
		else
			smsg.common.cont_flag = DBM_CONTINUE;

		smsg.head.msg_len = sizeof(st_SvcInfoList);
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -8;
		}
	}
	return 0;
}

int add_flt_sctp(mml_msg *ml, long long llNID)
{
	int				i, dRet, dSlen, dCount;
	size_t			szDescLen;
	unsigned int	uTmp;
	dbm_msg_t		smsg;
	st_SCTP_DB		stSCTPDB;

	smsg.data[0] = 0x00;
	memset(&stSCTPDB, 0x00, sizeof(st_SCTP_DB));

	for(i = 0; i < ml->num_of_para; i++)
	{
		switch(ml->msg_body[i].para_id)
		{
			case 22:
				if( (dRet = inet_pton(AF_INET, ml->msg_body[i].para_cont, &uTmp)) <= 0)
				{
					log_print(LOGN_CRI, LH"FAILED IN inet_pton(%s) dRet[%d]", LT, ml->msg_body[0].para_cont, dRet);
					set_dbm_ret(&smsg, eINVALID_IP, DBM_END, 0);
					if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
					{
						log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
						return -1;
					}
					return -2;
				}
				stSCTPDB.uSCTPIP	= ntohl(uTmp);
				break;
			case 707:
				stSCTPDB.cSysType	= (unsigned char)atoi(ml->msg_body[i].para_cont);
				break;
			case 701:
				stSCTPDB.cDirection	= (unsigned char)atoi(ml->msg_body[i].para_cont);
				break;
			case 580:
				stSCTPDB.huGroupID	= (unsigned short)atoi(ml->msg_body[i].para_cont);
				break;
			case 479:
				memset(stSCTPDB.szDesc, 0x00, MAX_SDESC);
				if( (szDescLen = strlen(ml->msg_body[i].para_cont)) > 0)
				{
					if(szDescLen > MAX_SDESC)
						strncpy(stSCTPDB.szDesc, ml->msg_body[i].para_cont, MAX_SDESC-1);
					else
						strcpy(stSCTPDB.szDesc, ml->msg_body[i].para_cont);
				}
				else
					sprintf(stSCTPDB.szDesc, " - ");
				break;
			default:
				log_print(LOGN_WARN, LH"INVALID para_id[%hu] para_cont[%s]", LT,
					ml->msg_body[i].para_id, ml->msg_body[i].para_cont);
		}
	}

	if( ((stSCTPDB.cDirection==1)&&(stSCTPDB.cSysType!=HSS_SYSTYPE)) || ((stSCTPDB.cDirection==2)&&(stSCTPDB.cSysType!=CSCF_SYSTYPE)))
	{
		log_print(LOGN_WARN, LH"INVALID cSysType[%hu] cDirection[%hu]", LT, stSCTPDB.cSysType, stSCTPDB.cDirection);
		set_dbm_ret(&smsg, eBadParameter, DBM_END, 0);
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -3;
		}
		return -4;
	}

	if( (dRet = dGetSCTPInfoCount(&stMySQL, &dCount)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dGetSCTPInfoCount() dRet[%d]", LT, dRet);
		set_dbm_ret(&smsg, eDBQUERYERROR, DBM_END, 0);
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -5;
		}
		return -6;
	}

	if(dCount >= MAX_SCTP_COUNT)
	{
		sprintf(smsg.data, "\n OVER MAX FLT_SCTP COUNT[%d]", MAX_SCTP_COUNT);

		dSlen = strlen(smsg.data) + 1;
		smsg.common.mml_err		= DBM_SUCCESS;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= dSlen;

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -7;
		}
		return -8;
	}

	if( (dRet = dInsertSCTPInfo(&stMySQL, &stSCTPDB)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dInsertSCTPInfo() dRet[%d]", LT, dRet);
		if(dRet == -1)
			set_dbm_ret(&smsg, eAlreadyRegisteredData, DBM_END, 0);
		else
			set_dbm_ret(&smsg, eDBQUERYERROR, DBM_END, 0);

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -9;
		}
		return -10;
	}
	else
	{
		dSlen					= strlen(smsg.data) + 1;
		smsg.common.mml_err		= DBM_SUCCESS;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= dSlen;

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -11;
		}

		if( (dRet = dInit_WatchFltEquip_Info()) < 0)
		{
			log_print(LOGN_CRI,LH"ERROR IN dInit_WatchFltEquip_Info() dRet[%d]", LT, dRet);
			return -12;
		}

		/* added shared memory by uamyd 2008.01.15 */
		if( (dRet = dInit_FltSCTP_Info()) < 0)
		{
			log_print(LOGN_CRI,LH"ERROR IN dInit_FltSCTP_Info() dRet[%d]", LT, dRet);
			return -13;
		}

		/* send to ntaf */
		for(i = 0; i < MAX_NTAF_NUM; i++)
		{
			if(pstSubSys->sys[i].uiIP > 0)
			{
				if( (dRet = dSend_FltSCTP_Data(pstSubSys->sys[i].usSysNo)) < 0)
				{
					log_print(LOGN_CRI, LH"ERROR IN dSend_FltSCTP_Data(usSysNo[%d][%d]) dRet[%d]", 
						LT, i, pstSubSys->sys[i].usSysNo, dRet);
					return -14;
				}
			}
		}/* end of for  */
	}

	return 0;
}

int del_flt_sctp(mml_msg *ml, long long llNID)
{
	int				i, dRet;
	unsigned int	uiIP;
	dbm_msg_t		smsg;

	smsg.data[0] = 0x00;

	for(i = 0; i < ml->num_of_para; i++)
	{
		switch(ml->msg_body[i].para_id)
		{
			case 22:
				if( (dRet = inet_pton(AF_INET, ml->msg_body[i].para_cont, &uiIP)) <= 0)
				{
					log_print(LOGN_CRI, LH"FAILED IN inet_pton(%s) dRet[%d]", LT, ml->msg_body[0].para_cont, dRet);
					set_dbm_ret(&smsg, eINVALID_IP, DBM_END, 0);
					if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
					{
						log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
						return -1;
					}
					return -2;
				}
				uiIP	= ntohl(uiIP);
				break;
			default:
				log_print(LOGN_WARN, LH"INVALID para_id[%hu] para_cont[%s]", LT,
					ml->msg_body[i].para_id, ml->msg_body[i].para_cont);
		}
	}

	if( (dRet = dGetCountSCTPIP(&stMySQL, uiIP)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dGetCountSCTPIP(uiIP[%u]) dRet[%d]", LT, uiIP, dRet);
		set_dbm_ret(&smsg, eDBQUERYERROR, DBM_END, 0);
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -3;
		}
		return -4;
	}
	else if(dRet == 0)
	{
		log_print(LOGN_CRI, LH"No uiIP[%u] in FLT_SCTP dRet[%d]", LT, uiIP, dRet);
		set_dbm_ret(&smsg, eNotFoundData, DBM_END, 0);
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -5;
		}
		return -6;
	}

    if( (dRet = dDeleteSCTP(&stMySQL, uiIP)) < 0)
    {
		log_print(LOGN_CRI, LH"ERROR IN dDeleteSCTP(uiIP[%u]) dRet[%d]", LT, uiIP, dRet);
		set_dbm_ret(&smsg, eDBQUERYERROR, DBM_END, 0);
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -7;
		}
		return -8;
    }
    else
    {
		set_dbm_ret(&smsg, DBM_SUCCESS, DBM_END, 0);
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
            return -9;
        }

		if( (dRet = dInit_WatchFltEquip_Info()) < 0)
		{
			log_print(LOGN_CRI,LH"ERROR IN dInit_WatchFltEquip_Info() dRet[%d]", LT, dRet);
			return -10;
		}

		/*	added shared memory by uamyd 2008.01.15	*/
		if( (dRet = dInit_FltSCTP_Info()) < 0)
		{
			log_print(LOGN_CRI,LH"ERROR IN dInit_FltSCTP_Info() dRet[%d]", LT, dRet);
			return -11;
		}

		/* send to ntaf */
		for(i = 0; i < MAX_NTAF_NUM; i++)
		{
			if(pstSubSys->sys[i].uiIP > 0)
			{
				if( (dRet = dSend_FltSCTP_Data(pstSubSys->sys[i].usSysNo)) < 0)
				{
					log_print(LOGN_CRI, LH"ERROR IN dSend_FltSCTP_Data(usSysNo[%d][%d]) dRet[%d]", 
						LT, i, pstSubSys->sys[i].usSysNo, dRet);
					return -12;
				}
			}
		}
	}

    return 0;
}

int ntam_trc_info(mml_msg *ml)
{
    pst_MsgQ    pstMsgQ;
    pst_MsgQSub pstMsgQSub;
	U8			*pNODE;

	pNODE = nifo_node_alloc(pMEMSINFO);
	if( pNODE == NULL ){
		log_print(LOGN_WARN, LH"FAILED IN nifo_node_alloc"EH, LT, ET);
		return -1;
	}
	
	pstMsgQ = (pst_MsgQ)nifo_tlv_alloc(pMEMSINFO, pNODE, DEF_MSGQ_NUM, DEF_MSGQ_SIZE, DEF_MEMSET_OFF);
	if( pstMsgQ == NULL ){
		log_print(LOGN_CRI, LH"FAILED IN nifo_tlv_alloc, return  NULL", LT);
		nifo_node_delete(pMEMSINFO, pNODE);
		return -2;
	}

    pstMsgQSub = (pst_MsgQSub)&pstMsgQ->llMType;

    pstMsgQSub->usType = DEF_SYS;
    pstMsgQSub->usSvcID = SID_MML;
    pstMsgQSub->usMsgID = MID_MML_REQ;

    pstMsgQ->ucNTAMID = atoi(ml->msg_body[0].para_cont);

    log_print(LOGN_INFO, "NTAM SYSNO [%d]", pstMsgQ->ucNTAMID);

    pstMsgQ->usBodyLen = sizeof(mml_msg)+ NTAFT_HEADER_LEN;
    pstMsgQ->ucProID = SEQ_PROC_S_MNG;


    memcpy(&pstMsgQ->szBody[NTAFT_HEADER_LEN], ml, sizeof(mml_msg));

	if( gifo_write(pMEMSINFO, gpCIFO, SEQ_PROC_S_MNG, SEQ_PROC_SI_SVC, nifo_offset(pMEMSINFO, pNODE)) < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN gifo_write(S_MNG:%d > SI_SVC:%d), offset=%ld"EH,
			LT, SEQ_PROC_S_MNG, SEQ_PROC_SI_SVC, nifo_offset(pMEMSINFO, pNODE), ET);
		nifo_node_delete(pMEMSINFO, pNODE);
		usleep(0);
		return -3;
	}

	log_print(LOGN_DEBUG,LH"SUCCESS SEND TO SI_SVC", LT);
    return 1;
}

int dis_flt_svr(mml_msg *ml, long long llNID)
{
	int				i, j, dRet, dLoopCnt, dCnt, dIdx;
	size_t			szWhereLen;
	char			sWhere[BUFSIZ];
	unsigned int	uIP;
	unsigned short	huNetMask, huApp, huL4, huL7;
	unsigned char	cFlag, cSysType, cExistIP, cExistNetMask, cExistFlag, cExistSysType, cExistApp, cExistL4, cExistL7, cNeedComma;
	dbm_msg_t		smsg;
	st_SvcMmc		stSvcMmc[MAX_SVR_CNT];
	st_SvcInfoList	stList;

	dCnt			= 0;
	dIdx			= 0;
	cExistIP		= 0;
	cExistNetMask	= 0;
	cExistFlag		= 0;
	cExistSysType	= 0;
	cExistApp		= 0;
	cExistL4		= 0;
	cExistL7		= 0;
	cNeedComma		= 0;

	memset(&smsg, 0x00, sizeof(dbm_msg_t));
	memset(&stList, 0x00, sizeof(st_SvcInfoList));

	for(i = 0; i < ml->num_of_para; i++)
	{
		switch(ml->msg_body[i].para_id)
		{
			case 22:
				if( (dRet = inet_pton(AF_INET, ml->msg_body[i].para_cont, &uIP)) <= 0)
				{
					log_print(LOGN_CRI, LH"FAILED IN inet_pton(%s) dRet[%d]", LT, ml->msg_body[0].para_cont, dRet);

					set_dbm_ret(&smsg, eINVALID_IP, DBM_END, 0);
					if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
					{
						log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
						return -1;
					}
					return -2;
				}
				uIP			= ntohl(uIP);
				cExistIP	= 1;
				break;
			case 65:
				huNetMask		= (unsigned short)atoi(ml->msg_body[i].para_cont);
				cExistNetMask	= 1;
				break;
			case 702:
				cFlag			= (unsigned char)atoi(ml->msg_body[i].para_cont);
				cExistFlag		= 1;
				break;
			case 706:
				cSysType		= (unsigned char)atoi(ml->msg_body[i].para_cont);
				cExistSysType	= 1;
				break;
			case 700:
				huApp			= (unsigned short)atoi(ml->msg_body[i].para_cont);
				cExistApp		= 1;
				break;
			case 608:
				huL4			= (unsigned short)atoi(ml->msg_body[i].para_cont);
				cExistL4		= 1;
				break;
			case 609:
				huL7			= (unsigned short)atoi(ml->msg_body[i].para_cont);
				cExistL7		= 1;
				break;
			default:
				log_print(LOGN_WARN, LH"INVALID para_id[%hu] para_cont[%s]", LT,
					ml->msg_body[i].para_id, ml->msg_body[i].para_cont);
		}
	}

	if(ml->num_of_para)
	{
		sprintf(sWhere, " WHERE ");
		szWhereLen	= strlen(sWhere);

		if(cExistIP && cExistNetMask)
		{
			sprintf(&sWhere[szWhereLen], "((IP>>(32-%hu))<<(32-%hu))=((%u>>(32-%hu))<<(32-%hu))", huNetMask, huNetMask, uIP, huNetMask, huNetMask);
			cNeedComma	= 1;
		}
		else if(cExistIP)
		{
			sprintf(&sWhere[szWhereLen], "IP=%u", uIP);
			cNeedComma	= 1;
		}

		if(cExistFlag)
		{
			if(cNeedComma)
			{
				szWhereLen = strlen(sWhere);
				sprintf(&sWhere[szWhereLen], " AND ");
			}
			else
				cNeedComma	= 1;

			szWhereLen = strlen(sWhere);
			sprintf(&sWhere[szWhereLen], "FLAG=%hu", cFlag);
		}

		if(cExistSysType)
		{
			if(cNeedComma)
			{
				szWhereLen = strlen(sWhere);
				sprintf(&sWhere[szWhereLen], " AND ");
			}
			else
				cNeedComma	= 1;

			szWhereLen = strlen(sWhere);
			sprintf(&sWhere[szWhereLen], "SYSTYPE=%hu", cSysType);
		}

		if(cExistApp)
		{
			if(cNeedComma)
			{
				szWhereLen = strlen(sWhere);
				sprintf(&sWhere[szWhereLen], " AND ");
			}
			else
				cNeedComma	= 1;

			szWhereLen = strlen(sWhere);
			sprintf(&sWhere[szWhereLen], "APPCODE=%hu", huApp);
		}

		if(cExistL4)
		{
			if(cNeedComma)
			{
				szWhereLen = strlen(sWhere);
				sprintf(&sWhere[szWhereLen], " AND ");
			}
			else
				cNeedComma	= 1;

			szWhereLen = strlen(sWhere);
			sprintf(&sWhere[szWhereLen], "L4=%hu", huL4);
		}

		if(cExistL7)
		{
			if(cNeedComma)
			{
				szWhereLen = strlen(sWhere);
				sprintf(&sWhere[szWhereLen], " AND ");
			}
			else
				cNeedComma	= 1;

			szWhereLen = strlen(sWhere);
			sprintf(&sWhere[szWhereLen], "L7=%hu", huL7);
		}

		szWhereLen = strlen(sWhere);
		sprintf(&sWhere[szWhereLen], " ");

		log_print(LOGN_DEBUG, LH"sWhere[%s]", LT, sWhere);
	}
	else
	{
		szWhereLen	= 0;
		sWhere[0]	= 0x00;
	}

	if( (dRet = dGetSvcInfo(&stMySQL, stSvcMmc, &dCnt, sWhere)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dGetSvcInfo() dRet[%d]", LT, dRet);
		smsg.common.mml_err		= eDBQUERYERROR;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= 0;
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -3;
		}
		return -4;
	}

	if( (dCnt % DEF_SVC_MMC) == 0)
		dLoopCnt	= dCnt/DEF_SVC_MMC;
	else
		dLoopCnt	= (dCnt/DEF_SVC_MMC)+1;

	if(dCnt == 0)
	{
		smsg.common.mml_err		= DBM_SUCCESS;
		smsg.common.cont_flag	= DBM_END;

		memcpy(smsg.data, &stList, sizeof(st_SvcInfoList));

		smsg.head.msg_len = sizeof(st_SvcInfoList);
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -5;
		}
	}

	for(i = 0; i < dLoopCnt; i++)
	{
		for(j = 0; j < DEF_SVC_MMC; j++)
		{
			stList.stSvcMmc[j].uSvcIP			= stSvcMmc[dIdx].uSvcIP;
			stList.stSvcMmc[j].huPort			= stSvcMmc[dIdx].huPort;
			stList.stSvcMmc[j].cFlag			= stSvcMmc[dIdx].cFlag;
			stList.stSvcMmc[j].cSysType			= stSvcMmc[dIdx].cSysType;

			stList.stSvcMmc[j].huL4Code			= stSvcMmc[dIdx].huL4Code;
			stList.stSvcMmc[j].huL7Code			= stSvcMmc[dIdx].huL7Code;
			stList.stSvcMmc[j].huAppCode		= stSvcMmc[dIdx].huAppCode;

			strcpy(stList.stSvcMmc[j].szDesc, stSvcMmc[dIdx].szDesc);

			log_print(LOGN_DEBUG,
				"[SEND ALL SVCINFO] IDX[%d] uSvcIP[%u] huPort[%hu] cFlag[%hu] cSysType[%hu] huL4Code[%hu] huL7Code[%hu] huAppCode[%hu] szDesc[%s]",
				dIdx+1, stList.stSvcMmc[j].uSvcIP, stList.stSvcMmc[j].huPort, stList.stSvcMmc[j].cFlag, stList.stSvcMmc[j].cSysType,
				stList.stSvcMmc[j].huL4Code, stList.stSvcMmc[j].huL7Code, stList.stSvcMmc[j].huAppCode, stList.stSvcMmc[j].szDesc);

			if( (dIdx+1) == dCnt)
				break;
			dIdx++;
		}
		smsg.common.TotPage	= dLoopCnt;
		smsg.common.CurPage	= i + 1;

		if(j == DEF_SVC_MMC)
			stList.dCount = DEF_SVC_MMC;
		else
			stList.dCount = j + 1;

		memcpy(smsg.data, &stList, sizeof(st_SvcInfoList));
		smsg.common.mml_err = DBM_SUCCESS;
		if(i == (dLoopCnt-1))
			smsg.common.cont_flag = DBM_END;
		else
			smsg.common.cont_flag = DBM_CONTINUE;

		smsg.head.msg_len = sizeof(st_SvcInfoList);
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -6;
		}
	}
	return 0;
}

int add_flt_svr(mml_msg *ml, long long llNID)
{
	int				i, dRet, dSlen, dCount;
	size_t			szDescLen;
	unsigned int	uTmp;
	dbm_msg_t		smsg;
	st_SvcInfo		stSvcInfo;

	smsg.data[0] = 0x00;
	memset(&stSvcInfo, 0x00, sizeof(st_SvcInfo));

	for(i = 0; i < ml->num_of_para; i++)
	{
		switch(ml->msg_body[i].para_id)
		{
			case 22:
				if( (dRet = inet_pton(AF_INET, ml->msg_body[i].para_cont, &uTmp)) <= 0)
				{
					log_print(LOGN_CRI, LH"FAILED IN inet_pton(%s) dRet[%d]", LT, ml->msg_body[0].para_cont, dRet);

					set_dbm_ret(&smsg, eINVALID_IP, DBM_END, 0);
					if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
					{
						log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
						return -1;
					}
					return -2;
				}
				stSvcInfo.uSvcIP	= ntohl(uTmp);
				break;
			case 66:
				stSvcInfo.huPort	= (unsigned short)atoi(ml->msg_body[i].para_cont);
				break;
			case 702:
				stSvcInfo.cFlag		= (unsigned char)atoi(ml->msg_body[i].para_cont);
				break;
			case 706:
				stSvcInfo.cSysType	= (unsigned char)atoi(ml->msg_body[i].para_cont);
				break;
			case 700:
				stSvcInfo.huAppCode	= (unsigned short)atoi(ml->msg_body[i].para_cont);
				break;
			case 608:
				stSvcInfo.huL4Code	= (unsigned short)atoi(ml->msg_body[i].para_cont);
				break;
			case 609:
				stSvcInfo.huL7Code	= (unsigned short)atoi(ml->msg_body[i].para_cont);
				break;
			case 479:
				memset(stSvcInfo.szDesc, 0x00, MAX_SDESC);
				if( (szDescLen = strlen(ml->msg_body[i].para_cont)) > 0)
				{
					if(szDescLen > MAX_SDESC)
						strncpy(stSvcInfo.szDesc, ml->msg_body[i].para_cont, MAX_SDESC-1);
					else
						strcpy(stSvcInfo.szDesc, ml->msg_body[i].para_cont);
				}
				else
					sprintf(stSvcInfo.szDesc, " - ");
				break;
			default:
				log_print(LOGN_WARN, LH"INVALID para_id[%hu] para_cont[%s]", LT,
					ml->msg_body[i].para_id, ml->msg_body[i].para_cont);
		}
	}

	if( ((stSvcInfo.cFlag==RP_FLAG_INDEX) && ( (stSvcInfo.cSysType==AAA_SYSTYPE) || (stSvcInfo.cSysType==SERVICE_SYSTYPE))) ||
		((stSvcInfo.cFlag==PI_FLAG_INDEX) && ( (stSvcInfo.cSysType==PDSN_SYSTYPE) || (stSvcInfo.cSysType==LNS_SYSTYPE))))
	{
		log_print(LOGN_WARN, LH"INVALID cFlag[%hu] cSysType[%hu]", LT, stSvcInfo.cFlag, stSvcInfo.cSysType);
		set_dbm_ret(&smsg, eBadParameter, DBM_END, 0);
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -3;
		}
		return -4;
	}

	if( (dRet = dGetSvcInfoCount(&stMySQL, &dCount)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dGetSvcInfoCount() dRet[%d]", LT, dRet);
		set_dbm_ret(&smsg, eDBQUERYERROR, DBM_END, 0);
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -5;
		}
		return -6;
	}

	if(dCount >= MAX_SVR_CNT)
	{
		sprintf(smsg.data, "dCount[%d] IS OVER MAX_SVR_CNT[%d] IN FLT_SVR", dCount, MAX_SVR_CNT);
		dSlen = strlen(smsg.data) + 1;

		smsg.common.mml_err		= DBM_SUCCESS;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= dSlen;

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -7;
		}
		return -8;
	}

	if( (dRet = dInsertSvcInfo(&stMySQL, &stSvcInfo)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dInsertSvcInfo() dRet[%d]", LT, dRet);
		if(dRet == -1)
			set_dbm_ret(&smsg, eAlreadyRegisteredData, DBM_END, 0);
		else
			set_dbm_ret(&smsg, eDBQUERYERROR, DBM_END, 0);

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -9;
		}
		return -10;
	}
	else
	{
		set_dbm_ret(&smsg, DBM_SUCCESS, DBM_END, 0);
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -11;
		}

		/* added shared memory by uamyd 2008.01.15 */
		if( (dRet = dInit_FltSvc_Info()) < 0)
		{
			log_print(LOGN_CRI,LH"ERROR IN dInit_FltSvc_Info() dRet[%d]", LT, dRet);
			return -12;
		}

		if( (dRet = dInit_WatchFltSVC_Info()) < 0)
		{
			log_print(LOGN_CRI,LH"ERROR IN dInit_WatchFltSVC_Info() dRet[%d]", LT, dRet);
			return -13;
		}

		/* 이 명령에 대해서는 O_SVCMON만 처리를 한다. */
		switch(stSvcInfo.huL4Code)
		{
			case L4_INET_TCP:
			case L4_INET_TCP_RECV:
			case L4_INET_TCP_USER:
			case L4_INET_HTTP:
			case L4_INET_HTTP_RECV:
			case L4_INET_HTTP_USER:
				return 0;
		}

		/* send to ntaf */
		for(i = 0; i < MAX_NTAF_NUM; i++)
		{
			if(pstSubSys->sys[i].uiIP > 0)
			{
				if( (dRet = dSend_FltSvc_Data(pstSubSys->sys[i].usSysNo)) < 0)
				{
					log_print(LOGN_CRI, LH"ERROR IN dSend_FltSvc_Data(usSysNo[%d][%d]) dRet[%d]", 
						LT, i, pstSubSys->sys[i].usSysNo, dRet);
					return -14;
				}
			}
		}/* end of for  */
	}

	return 0;
}

int del_flt_svr(mml_msg *ml, long long llNID)
{
	int				i, dRet;
	unsigned short	usPort, huFlag;
	unsigned int	uiIP;
	dbm_msg_t		smsg;

	smsg.data[0] = 0x00;

	for(i = 0; i < ml->num_of_para; i++)
	{
		switch(ml->msg_body[i].para_id)
		{
			case 22:
				if( (dRet = inet_pton(AF_INET, ml->msg_body[i].para_cont, &uiIP)) <= 0)
				{
					log_print(LOGN_CRI, LH"FAILED IN inet_pton(%s) dRet[%d]", LT, ml->msg_body[0].para_cont, dRet);

					set_dbm_ret(&smsg, eINVALID_IP, DBM_END, 0);
					if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
					{
						log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
						return -1;
					}
					return -2;
				}
				uiIP	= ntohl(uiIP);
				break;
			case 66:
				usPort	= (unsigned short)atoi(ml->msg_body[i].para_cont);
				break;
			case 702:
				huFlag	= (unsigned short)atoi(ml->msg_body[i].para_cont);
				break;
			default:
				log_print(LOGN_WARN, LH"INVALID para_id[%hu] para_cont[%s]", LT,
					ml->msg_body[i].para_id, ml->msg_body[i].para_cont);
		}
	}

    if( (dRet = dGetCountSVC(&stMySQL, uiIP, usPort, huFlag)) < 0)
    {
		log_print(LOGN_CRI, LH"ERROR IN dGetCountSVC(uiIP[%u], usPort[%hu], huFlag[%hu]) dRet[%d]", LT, uiIP, usPort, huFlag, dRet);
		set_dbm_ret(&smsg, eDBQUERYERROR, DBM_END, 0);
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -3;
		}
		return -4;
    }
	else if(dRet == 0)
	{
		log_print(LOGN_CRI, LH"No uiIP[%u], usPort[%hu], huFlag[%hu] in FLT_SVC dRet[%d]", LT, uiIP, usPort, huFlag, dRet);
		set_dbm_ret(&smsg, eNotFoundData, DBM_END, 0);
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -5;
		}
		return -6;
	}

    if( (dRet = dDeleteSvc(&stMySQL, uiIP, usPort, huFlag)) < 0)
    {
		log_print(LOGN_CRI, LH"ERROR IN dDeleteSvc(uiIP[%u], usPort[%hu], huFlag[%hu]) dRet[%d]", LT, uiIP, usPort, huFlag, dRet);
		set_dbm_ret(&smsg, eDBQUERYERROR, DBM_END, 0);
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -7;
		}
		return -8;
    }
    else
    {
		set_dbm_ret(&smsg, DBM_SUCCESS, DBM_END, 0);
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
            return -9;
        }

		if( (dRet = dInit_WatchFltSVC_Info()) < 0)
		{
			log_print(LOGN_CRI,LH"ERROR IN dInit_WatchFltSVC_Info() dRet[%d]", LT, dRet);
			return -10;
		}

		/*	added shared memory by uamyd 2008.01.15	*/
		if( (dRet = dInit_FltSvc_Info()) < 0)
		{
			log_print(LOGN_CRI,LH"ERROR IN dInit_FltSvc_Info() dRet[%d]", LT, dRet);
			return -11;
		}

		/* send to ntaf */
		for(i = 0; i < MAX_NTAF_NUM; i++)
		{
			if(pstSubSys->sys[i].uiIP > 0)
			{
				if( (dRet = dSend_FltSvc_Data(pstSubSys->sys[i].usSysNo)) < 0)
				{
					log_print(LOGN_CRI, LH"ERROR IN dSend_FltSvc_Data(usSysNo[%d][%d]) dRet[%d]", 
						LT, i, pstSubSys->sys[i].usSysNo, dRet);
					return -12;
				}
			}
		}
	}

    return 0;
}

int dis_flt_clt(mml_msg *ml, long long llNID)
{
	int				dRet, dLoopCnt, i, j, dCnt = 0, dIdx = 0;
	unsigned int	uIP;
	unsigned short	huNetMask;
	unsigned char	cFlag, cSysType, cExistIP, cExistNetMask, cExistFlag, cExistSysType, cNeedComma, newTypeEnable=0;
	size_t			szWhereLen;
	char			sWhere[BUFSIZ];
	dbm_msg_t		smsg;
	st_NAS_db		stNASMmc[MAX_MNIP_COUNT];
	st_NAS_MMC		stList;


	cExistIP		= 0;
	cExistNetMask	= 0;
	cExistFlag		= 0;
	cExistSysType	= 0;
	cNeedComma		= 0;

	memset(&stList, 0x00, sizeof(st_NAS_MMC));
	memset(&smsg,   0x00, sizeof(dbm_msg_t));


	for(i = 0; i < ml->num_of_para; i++){
		switch(ml->msg_body[i].para_id) {
			case 22:
				if( (dRet = inet_pton(AF_INET, ml->msg_body[i].para_cont, &uIP)) <= 0){
					log_print(LOGN_CRI, LH"FAILED IN inet_pton(%s) dRet[%d]", LT, ml->msg_body[0].para_cont, dRet);

					set_dbm_ret(&smsg, eINVALID_IP, DBM_END, 0);
					if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0){
						log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
						return -1;
					}
					return -2;
				}
				uIP			= ntohl(uIP);
				cExistIP	= 1;
				break;
			case 65:
				huNetMask		= (unsigned short)atoi(ml->msg_body[i].para_cont);
				cExistNetMask	= 1;
				break;
			case 702:
				cFlag			= (unsigned char)atoi(ml->msg_body[i].para_cont);
				cExistFlag		= 1;
				break;
			case 705:
				cSysType		= (unsigned char)atoi(ml->msg_body[i].para_cont);
				cExistSysType	= 1;
				if( cSysType >= BSC_SYSTYPE && cSysType <= BTS_SYSTYPE )
					newTypeEnable = 1;
				break;
			default:
				log_print(LOGN_WARN, LH"INVALID para_id[%hu] para_cont[%s]", LT,
					ml->msg_body[i].para_id, ml->msg_body[i].para_cont);
		}
	}

	if(ml->num_of_para){
		sprintf(sWhere, " WHERE ");
		szWhereLen	= strlen(sWhere);

		if(cExistIP && cExistNetMask){
			sprintf(&sWhere[szWhereLen], "((IP>>(32-%hu))<<(32-%hu))=((%u>>(32-%hu))<<(32-%hu))", huNetMask, huNetMask, uIP, huNetMask, huNetMask);
			cNeedComma	= 1;

		}else if(cExistIP){
			sprintf(&sWhere[szWhereLen], "IP=%u", uIP);
			cNeedComma	= 1;

		}else if(cExistNetMask){
			sprintf(&sWhere[szWhereLen], "NETMASK=%hu", huNetMask);
			cNeedComma	= 1;
		}

		szWhereLen = strlen(sWhere);

		if(cExistFlag){
			if(cNeedComma){
				szWhereLen = strlen(sWhere);
				sprintf(&sWhere[szWhereLen], " AND ");

			}else
				cNeedComma	= 1;

			szWhereLen = strlen(sWhere);
			sprintf(&sWhere[szWhereLen], "FLAG=%hu", cFlag);
		}

		if(cExistSysType){
			if(cNeedComma){
				szWhereLen = strlen(sWhere);
				sprintf(&sWhere[szWhereLen], " AND ");

			}else
				cNeedComma	= 1;

			szWhereLen = strlen(sWhere);
			sprintf(&sWhere[szWhereLen], "SYSTYPE=%hu", cSysType);
		}

		szWhereLen = strlen(sWhere);
		sprintf(&sWhere[szWhereLen], " ");

		log_print(LOGN_DEBUG, "Where[%s]", sWhere);

	}else{
		szWhereLen	= 0;
		sWhere[0]	= 0x00;
	}

	dRet = dGetNAS(&stMySQL, stNASMmc, &dCnt, sWhere);
	if( dRet < 0 ){
		log_print(LOGN_CRI, LH"ERROR IN dGetNAS() dRet[%d]", LT, dRet);

		set_dbm_ret(&smsg, eDBQUERYERROR, DBM_END, 0);
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0){
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -3;
		}
		return -4;
	}

	if( (dCnt % DEF_MNIP_MMC) == 0)
		dLoopCnt = (dCnt / DEF_MNIP_MMC);
	else
		dLoopCnt = (dCnt / DEF_MNIP_MMC)+1;

	if(dCnt == 0){
		set_dbm_ret(&smsg, DBM_SUCCESS, DBM_END, 0);
		memset(smsg.data, 0X00, MSG_DATA_LEN);

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0){
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -5;
		}
	}

	for(i = 0; i < dLoopCnt; i++)
	{
		for(j = 0; j < DEF_MNIP_MMC; j++)
		{
			stList.stNAS[j].dIdx		= dIdx+1;
			stList.stNAS[j].uMNIP		= stNASMmc[dIdx].uMNIP;
			stList.stNAS[j].usNetMask	= stNASMmc[dIdx].usNetMask;
			stList.stNAS[j].cFlag		= stNASMmc[dIdx].cFlag;
			stList.stNAS[j].cSysType	= stNASMmc[dIdx].cSysType;

			sprintf(stList.stNAS[j].szDesc, "%s", stNASMmc[dIdx].szDesc);

			log_print(LOGN_DEBUG,"idx[%d] [SEND ALL FLT_CLT] MNIP[%u] NETMASK[%hu] FLAG[%hu] SYSTYPE[%hu] DESC[%s]",
				dIdx, stList.stNAS[j].uMNIP, stList.stNAS[j].usNetMask, stList.stNAS[j].cFlag, stList.stNAS[j].cSysType, stList.stNAS[j].szDesc);

			if( (dIdx+1) == dCnt)
				break;
			dIdx++;
		}
		smsg.common.TotPage	= dLoopCnt;
		smsg.common.CurPage	= i + 1;


		smsg.common.mml_err = DBM_SUCCESS;
		if(i == dLoopCnt - 1)
			smsg.common.cont_flag = DBM_END;
		else
			smsg.common.cont_flag = DBM_CONTINUE;

		if(j == DEF_MNIP_MMC) 	stList.dCount = DEF_MNIP_MMC;
		else 					stList.dCount = j + 1;

		stList.ucType = cSysType;

		memcpy(smsg.data, &stList, sizeof(st_NAS_MMC));
		smsg.head.msg_len = sizeof(st_NAS_MMC);

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -6;
		}
	}

	return 0;
}

int add_flt_clt(mml_msg *ml, long long llNID)
{
	int				i, dRet, dSlen, uTmp;
	size_t			szDescLen;
	dbm_msg_t		smsg;
	st_NAS_db		stNasDB;

	smsg.data[0]	= 0x00;
	memset(&stNasDB, 0x00, sizeof(st_NAS_db));

	smsg.common.TotPage	= 0;
	smsg.common.CurPage	= 0;

	for(i = 0; i < ml->num_of_para; i++)
	{
		switch(ml->msg_body[i].para_id)
		{
			case 22:
				if( (dRet = inet_pton(AF_INET, ml->msg_body[i].para_cont, &uTmp)) <= 0)
				{
					log_print(LOGN_CRI, LH"FAILED IN inet_pton(%s) dRet[%d]", LT, ml->msg_body[0].para_cont, dRet);

					set_dbm_ret(&smsg, eINVALID_IP, DBM_END, 0);
					if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
					{
						log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
						return -1;
					}
					return -2;
				}
				stNasDB.uMNIP		= ntohl(uTmp);
				break;
			case 65:
				stNasDB.usNetMask	= (unsigned short)atoi(ml->msg_body[i].para_cont);
				break;
			case 702:
				stNasDB.cFlag		= (unsigned char)atoi(ml->msg_body[i].para_cont);
				break;
			case 705:
				stNasDB.cSysType	= (unsigned char)atoi(ml->msg_body[i].para_cont);
				break;
			case 479:
				memset(stNasDB.szDesc, 0x00, MAX_SDESC);
				if( (szDescLen = strlen(ml->msg_body[i].para_cont)) > 0)
				{
					if(szDescLen > MAX_SDESC)
						strncpy(stNasDB.szDesc, ml->msg_body[i].para_cont, MAX_SDESC-1);
					else
						strcpy(stNasDB.szDesc, ml->msg_body[i].para_cont);
				}
				else
					sprintf(stNasDB.szDesc, " - ");
				break;
			default:
				log_print(LOGN_WARN, LH"INVALID para_id[%hu] para_cont[%s]", LT,
					ml->msg_body[i].para_id, ml->msg_body[i].para_cont);
		}
	}

	if( ((stNasDB.cFlag==PI_FLAG_INDEX) && ( (stNasDB.cSysType==PCF_SYSTYPE)||(stNasDB.cSysType==LAC_SYSTYPE))) ||
		((stNasDB.cFlag==RP_FLAG_INDEX) &&
			( (stNasDB.cSysType==PDSN_SYSTYPE)||(stNasDB.cSysType==MNIP_SYSTYPE)||
			(stNasDB.cSysType==LNS_SYSTYPE)||(stNasDB.cSysType==AAA_SYSTYPE)||(stNasDB.cSysType==CRX_SYSTYPE))))
	{
		log_print(LOGN_WARN, LH"INVALID cFlag[%hu] cSysType[%hu]", LT, stNasDB.cFlag, stNasDB.cSysType);
		set_dbm_ret(&smsg, eBadParameter, DBM_END, 0);
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -3;
		}
		return -4;
	}

	/*	COUNT MNIP NUMBER IN DB	*/
	uTmp	= 0;
	if( (dRet = dGetMNIPCount(&stMySQL, &uTmp)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dGetMNIPCount() dRet[%d]", LT, dRet);
		set_dbm_ret(&smsg, eDBQUERYERROR, DBM_END, 0);

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -5;
		}
		return -6;
	}

	if(uTmp >= MAX_MNIP_COUNT)
	{
		sprintf(smsg.data, "COUNT[%d] IS OVER MAX[%d] IN FLT_CLT", uTmp, MAX_MNIP_COUNT);
		dSlen = strlen(smsg.data) + 1;

		smsg.common.mml_err		= DBM_SUCCESS;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= dSlen;

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -7;
		}
		return -8;
	}

	if( (dRet = dInsertMNIP(&stMySQL, &stNasDB)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dInsertMNIP() dRet[%d]", LT, dRet);
		if(dRet == -1)
			set_dbm_ret(&smsg, eAlreadyRegisteredData, DBM_END, 0);
		else
			set_dbm_ret(&smsg, eDBQUERYERROR, DBM_END, 0);

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -9;
		}
		return -10;
	}
	else
	{
		if( (dRet = dInit_FltIPPool_Info()) < 0)
		{
			log_print(LOGN_CRI,LH"ERROR IN dInit_FltIPPool_Info() dRet[%d]", LT, dRet);
			return -13;
		}

		/* send to ntaf */
		for(i = 0; i < MAX_NTAF_NUM; i++)
		{
			if(pstSubSys->sys[i].uiIP > 0)
			{
				if( (dRet = dSend_FltIPPool_Data(pstSubSys->sys[i].usSysNo)) < 0)
				{
					log_print(LOGN_CRI, LH"ERROR IN dSend_FltIPPool_Data() dRet[%d]", LT, dRet);
					return -14;
				}
			}
		}

		smsg.common.mml_err		= DBM_SUCCESS;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= 0;

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -11;
		}
	}

    return 0;
}

int del_flt_clt(mml_msg *ml, long long llNID)
{
	int				i, dRet;
	unsigned int	uMNIP, uTmp;
	unsigned short	huNetmask, huFlag;
	dbm_msg_t		smsg;

	smsg.data[0]		= 0x00;
	smsg.common.TotPage	= 0;
	smsg.common.CurPage	= 0;

	for(i = 0; i < ml->num_of_para; i++)
	{
		switch(ml->msg_body[i].para_id)
		{
			case 22:
				if( (dRet = inet_pton(AF_INET, ml->msg_body[i].para_cont, &uTmp)) <= 0)
				{
					log_print(LOGN_CRI, LH"FAILED IN inet_pton(%s) dRet[%d]", LT, ml->msg_body[0].para_cont, dRet);

					set_dbm_ret(&smsg, eINVALID_IP, DBM_END, 0);
					if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
					{
						log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
						return -1;
					}
					return -2;
				}
				uMNIP		= ntohl(uTmp);
				break;
			case 65:
				huNetmask	= (unsigned short)atoi(ml->msg_body[i].para_cont);
				break;
			case 702:
				huFlag		= (unsigned short)atoi(ml->msg_body[i].para_cont);
				break;
			default:
				log_print(LOGN_WARN, LH"INVALID para_id[%hu] para_cont[%s]", LT,
					ml->msg_body[i].para_id, ml->msg_body[i].para_cont);
		}
	}

    if( (dRet = dGetCountMNIP(&stMySQL, uMNIP, huNetmask, huFlag)) < 0)
    {
		log_print(LOGN_CRI, LH"ERROR IN dGetCountMNIP(uMNIP[%u], huNetmask[%hu], huFlag[%hu]) dRet[%d]", LT, uMNIP, huNetmask, huFlag, dRet);
		set_dbm_ret(&smsg, eDBQUERYERROR, DBM_END, 0);
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -3;
		}
		return -4;
    }
	else if(dRet == 0)
	{
		log_print(LOGN_CRI, LH"No uMNIP[%u], huNetmask[%hu], huFlag[%hu] in FLT_MNIP dRet[%d]", LT, uMNIP, huNetmask, huFlag, dRet);
		set_dbm_ret(&smsg, eNotFoundData, DBM_END, 0);
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -5;
		}
		return -6;
	}

	/* DELETE MNIP FROM DB */
	if( (dRet = dDeleteMNIP(&stMySQL, uMNIP, huNetmask, huFlag)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dDeleteMNIP() dRet[%d]", LT, dRet);
		set_dbm_ret(&smsg, eDBQUERYERROR, DBM_END, 0);
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -7;
		}
		return -8;
	}
	else
	{
		set_dbm_ret(&smsg, DBM_SUCCESS, DBM_END, 0);
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -9;
		}

		if( (dRet = dInit_FltIPPool_Info()) < 0)
		{
			log_print(LOGN_CRI,LH"ERROR IN dInit_FltIPPool_Info() dRet[%d]", LT, dRet);
			return -10;
		}

		/* send to ntaf */
		for(i = 0; i < MAX_NTAF_NUM; i++)
		{
			if(pstSubSys->sys[i].uiIP > 0)
			{
				if( (dRet = dSend_FltIPPool_Data(pstSubSys->sys[i].usSysNo)) < 0)
				{
					log_print(LOGN_CRI, LH"FAILED IN dSend_FltIPPool_Data(SysNo[%d]) dRet[%d]", 
						LT, pstSubSys->sys[i].usSysNo, dRet);
					return -11;
				}
			}
		}
	}

	return 0;
}

int dis_all_user_info(mml_msg *ml, long long llNID)
{
	int				i, j, dRet, dCnt, dIdx, dLoopCnt;
	dbm_msg_t		smsg;

	st_User_Add_List stList;
	st_UserAdd		stUserAdd[MAX_USER];

	dCnt	= 0;
	dIdx	= 0;
	memset(&smsg, 0x00, sizeof(dbm_msg_t));
	memset(&stList, 0x00, sizeof(st_User_Add_List));

	if( (dRet = dGetUserInfo(&stMySQL, stUserAdd, &dCnt)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dGetUserInfo() dRet[%d]", LT, dRet);

		smsg.common.mml_err		= eDBQUERYERROR;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= 0;
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -1;
		}
		return -2;
	}

	if(dCnt >= MAX_USER)
	{
		sprintf(smsg.data, "OVER MAX SYS_USER_INFO COUNT[%d]", MAX_USER);

		smsg.common.mml_err		= eOVERMAXROW;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= strlen(smsg.data)+1;
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -3;
		}
		return -4;
	}

	if( (dCnt%(DEF_USER_MMC-2)) == 0)
		dLoopCnt = dCnt / (DEF_USER_MMC - 2);
	else
		dLoopCnt = (dCnt / (DEF_USER_MMC - 2)) + 1;

	if(dCnt == 0)
	{
		smsg.common.mml_err		= DBM_SUCCESS;
		smsg.common.cont_flag	= DBM_END;

		memcpy(smsg.data, &stList, sizeof(st_User_Add_List));

		smsg.head.msg_len		= sizeof(st_User_Add_List);
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -1;
		}
	}

	for(i = 0; i < dLoopCnt; i++)
	{
		for(j = 0; j < (DEF_USER_MMC-2); j++)
		{
			stList.stUserAdd[j].sSLevel         = stUserAdd[dIdx].sSLevel;
			stList.stUserAdd[j].usLogin         = stUserAdd[dIdx].usLogin;
			stList.stUserAdd[j].uLastLoginTime  = stUserAdd[dIdx].uLastLoginTime;
			stList.stUserAdd[j].uConnectIP      = stUserAdd[dIdx].uConnectIP;

			stList.stUserAdd[j].uCreateTime		= stUserAdd[dIdx].uCreateTime;

			sprintf(stList.stUserAdd[j].szLocalName, "%s", stUserAdd[dIdx].szLocalName);
			sprintf(stList.stUserAdd[j].szUserName, "%s", stUserAdd[dIdx].szUserName);
			sprintf(stList.stUserAdd[j].szContact, "%s", stUserAdd[dIdx].szContact);

			log_print(LOGN_DEBUG,"idx[%d] [SEND ALL USERINFO] LEVEL[%hd] LOGIN[%hu] LAST_LOGIN_TIME[%u]"
				" CONNCET_IP[%u] USER_NAME[%s] CONTACT[%s] CREATE_TIME[%u]",
				dIdx, stList.stUserAdd[j].sSLevel, stList.stUserAdd[j].usLogin, stList.stUserAdd[j].uLastLoginTime, stList.stUserAdd[j].uConnectIP,
				stList.stUserAdd[j].szUserName, stList.stUserAdd[j].szContact, stList.stUserAdd[j].uCreateTime);

			if( (dIdx+1) == dCnt)
				break;
			dIdx++;
		}
		smsg.common.TotPage	= dLoopCnt;
		smsg.common.CurPage	= i + 1;

		if (j == (DEF_USER_MMC-2))
			stList.dCount	= (DEF_USER_MMC-2);
		else
			stList.dCount	= j +1;

		memcpy(smsg.data, &stList, sizeof(st_User_Add_List));
		smsg.common.mml_err = DBM_SUCCESS;
		if(i == (dLoopCnt-1))
			smsg.common.cont_flag = DBM_END;
		else
			smsg.common.cont_flag = DBM_CONTINUE;

		smsg.head.msg_len = sizeof(st_User_Add_List);
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -1;
		}
	}

	return 0;
}

int add_admin_info(mml_msg *ml, long long llNID)/*ADD*/
{
	int				dRet, dCnt, dSlen;
	dbm_msg_t		smsg;
	st_User_Add		stUserAdd;

	smsg.data[0] = 0x00;

	if(strlen(ml->msg_body[0].para_cont) > 0)
	{
		sprintf(stUserAdd.szUserName, "%s", ml->msg_body[0].para_cont);
		log_print(LOGN_INFO, LH"stUserAdd.UserName[%s]", LT, stUserAdd.szUserName);
	}

	if(strlen(ml->msg_body[1].para_cont) > 0)
	{
		sprintf(stUserAdd.szPassword, "%s", ml->msg_body[1].para_cont);
		log_print(LOGN_INFO, LH"stUserAdd.szUserPass[%s]", LT, stUserAdd.szPassword);
	}

	stUserAdd.sSLevel	= atoi(ml->msg_body[2].para_cont);

	if(strlen(ml->msg_body[3].para_cont) > 0)
	{
		sprintf(stUserAdd.szLocalName, "%s", ml->msg_body[3].para_cont);
		log_print(LOGN_INFO, LH"stUserAdd.szLocalName[%s]", LT, stUserAdd.szLocalName);
	}

	if(strlen(ml->msg_body[4].para_cont) > 0)
	{
		sprintf(stUserAdd.szContact, "%s", ml->msg_body[4].para_cont);
		log_print(LOGN_INFO, LH"stUserAdd.szContact[%s]", LT, stUserAdd.szContact);
	}

	if(ml->num_of_para >= 6)
	{
		if( (dRet = inet_pton(AF_INET, ml->msg_body[5].para_cont, &stUserAdd.uConnectIP)) <= 0)
		{
			log_print(LOGN_CRI, LH"FAILED IN inet_pton(%s) dRet[%d]", LT, ml->msg_body[5].para_cont, dRet);

			dSlen					= strlen(smsg.data) + 1;
			smsg.common.mml_err		= eINVALID_IP;
			smsg.common.cont_flag	= DBM_END;
			smsg.head.msg_len		= dSlen;

			if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
			{
				log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
				return -1;
			}
			return -2;
		}
		else
			stUserAdd.uConnectIP = ntohl(stUserAdd.uConnectIP);
	}
	else
		stUserAdd.uConnectIP = 0;

	stUserAdd.uCreateTime	= time(NULL);

	if( (dRet = dGetMMCUserCount(&stMySQL, &dCnt)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dGetMMCUserCount() dRet[%d]", LT, dRet);

		smsg.common.mml_err		= eDBQUERYERROR;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= 0;
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -5;
		}
		return -6;
	}

	if(dCnt >= MAX_USER)
	{
		sprintf(smsg.data, "OVER MAX SYS_USER_INFO COUNT[%d]", MAX_USER);
		dSlen = strlen(smsg.data) + 1;

		smsg.common.mml_err		= eOVERMAXROW;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= dSlen;

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -7;
		}
		return -8;
	}

    if( (dRet = dAddAdminInfo(&stMySQL, &stUserAdd)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dAddAdminInfo() dRet[%d]", LT, dRet);
		if(dRet == -1)
		{
			dSlen					= strlen(smsg.data) + 1;
			smsg.common.mml_err		= eAlreadyRegisteredData;
			smsg.common.cont_flag	= DBM_END;
			smsg.head.msg_len		= dSlen;
		}
		else
		{
			dSlen					= strlen(smsg.data) + 1;
			smsg.common.mml_err		= eDBQUERYERROR;
			smsg.common.cont_flag	= DBM_END;
			smsg.head.msg_len		= dSlen;
		}

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -9;
		}
		return -10;
    }
	else
	{
		dSlen					= strlen(smsg.data) + 1;
		smsg.common.mml_err		= DBM_SUCCESS;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= dSlen;

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -11;
		}
	}

    return 0;
}

int del_admin_info(mml_msg *ml, long long llNID)/*DEL*/
{
	int				dRet, dSlen;
	dbm_msg_t		smsg;
	st_User_Add		stUserAdd;

	smsg.data[0]	= 0x00;

	if(strlen(ml->msg_body[0].para_cont) > 0)
	{
		sprintf(stUserAdd.szUserName, "%s", ml->msg_body[0].para_cont);
		log_print(LOGN_INFO, LH"stUserAdd.UserName[%s]", LT, stUserAdd.szUserName);
	}
	else
	{
		log_print(LOGN_CRI, LH"stUserAdd.UserName[%s]", LT, stUserAdd.szUserName);
		return -1;
	}

	if( (dRet = dSelectAdminInfo(&stMySQL, &stUserAdd)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dSelectAdminInfo() dRet[%d]", LT, dRet);
		dSlen					= strlen(smsg.data) + 1;
		smsg.common.mml_err		= eDBQUERYERROR;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= dSlen;

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -2;
		}
		return -3;
	}
	else if(dRet == 0)
	{
		dSlen					= strlen(smsg.data) + 1;
		smsg.common.mml_err		= eAdminInfoNotRegistered;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= dSlen;

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -4;
		}
		return -5;
	}

	if( (dRet = dDeleteAdminInfo(&stMySQL, &stUserAdd)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dDeleteAdminInfo() dRet[%d]", LT, dRet);
		dSlen					= strlen(smsg.data) + 1;
		smsg.common.mml_err		= eDBQUERYERROR;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= dSlen;

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -6;
		}
		return -7;
	}
	else
	{
		dSlen					= strlen(smsg.data) + 1;
		smsg.common.mml_err		= DBM_SUCCESS;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= dSlen;

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -8;
		}
	}

    return 0;
}

int set_admin_info(mml_msg *ml, long long llNID)/* CHG*/
{
	int				dRet, dSlen;
	dbm_msg_t		smsg;
	st_User_Add		stUserAdd;

	smsg.data[0] = 0x00;

	if(strlen(ml->msg_body[0].para_cont) > 0)
	{
		sprintf(stUserAdd.szUserName, "%s", ml->msg_body[0].para_cont);
		log_print(LOGN_INFO, LH"stUserAdd.UserName[%s]", LT, stUserAdd.szUserName);
	}
	else
	{
		log_print(LOGN_CRI, LH"stUserAdd.UserName[%s]", LT, stUserAdd.szUserName);
		return -1;
	}

	if(strlen(ml->msg_body[1].para_cont) > 0)
	{
		sprintf(stUserAdd.szPassword, "%s", ml->msg_body[1].para_cont);
		log_print(LOGN_INFO, LH"stUserAdd.szUserPass[%s]", LT, stUserAdd.szPassword);
	}
	else
	{
		log_print(LOGN_CRI, LH"stUserAdd.szUserPass[%s]", LT, stUserAdd.szPassword);
		return -2;
	}

	if( ((stUserAdd.sSLevel = atoi(ml->msg_body[2].para_cont)) > -1) && (stUserAdd.sSLevel < 3))
		log_print(LOGN_INFO, LH"stUserAdd.sSLevel[%d]", LT, stUserAdd.sSLevel);
	else
	{
		log_print(LOGN_CRI, LH"stUserAdd.sSLevel[%d]", LT, stUserAdd.sSLevel);
		return -3;
	}

	if( (dRet = dSelectAdminInfo(&stMySQL, &stUserAdd)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dSelectAdminInfo() dRet[%d]", LT, dRet);
		dSlen					= strlen(smsg.data) + 1;
		smsg.common.mml_err		= eDBQUERYERROR;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= dSlen;

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -4;
		}
		return -5;
	}
	else if(dRet == 0)
	{
		dSlen					= strlen(smsg.data) + 1;
		smsg.common.mml_err		= eAdminInfoNotRegistered;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= dSlen;

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -6;
		}
		return -7;
	}

	if( (dRet = dUpdateAdminInfo(&stMySQL, &stUserAdd)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dUpdateAdminInfo(szUserName[%s]) dRet[%d]", LT, stUserAdd.szUserName, dRet);

		dSlen					= strlen(smsg.data) + 1;
		smsg.common.mml_err		= eDBQUERYERROR;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= dSlen;

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -8;
		}
		return -9;
	}
	else
	{
		dSlen					= strlen(smsg.data) + 1;
		smsg.common.mml_err		= DBM_SUCCESS;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= dSlen;

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -10;
		}
	}

	return 0;
}

int dis_flt_his(mml_msg *ml, INT64 llNID)
{
    int                 dRet, dLoopCnt, i, j;
	unsigned int		tStartTime, tEndTime;
    dbm_msg_t           smsg;
    int                 dIdx = 0, dCnt = 0;

	st_SysCONDMsg		stCOND_DB[MAX_CONDMSG_CNT];
	st_CondMsg_List		stList;

	memset(&smsg, 0x00, sizeof(dbm_msg_t));
	memset(&stList, 0x00, sizeof(st_CondMsg_List));

    smsg.common.total_cnt = 0;
    smsg.common.TotPage = 0;
    smsg.common.CurPage = 0;

    memset(&stList, 0x00, sizeof(st_CondMsg_List));

	tStartTime	= tGetTimeFromStr(ml->msg_body[0].para_cont);
    tEndTime	= tGetTimeFromStr(ml->msg_body[1].para_cont);

	log_print(LOGN_INFO, LH"STIME[%u] ETIME[%u]", LT, tStartTime, tEndTime);
    if( (((int)tStartTime-(int)tEndTime) > 0) || ((tEndTime-tStartTime) > (86400*7)))
    {
        smsg.common.mml_err		= eINVALIDE_SEARCH_TIME;
        smsg.common.cont_flag	= DBM_END;
        smsg.head.msg_len		= 0;
        if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
        {
            log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
            return -1;
        }
        return -2;
    }

    if( (dRet = dGet_CONDMsg(&stMySQL, stCOND_DB, tStartTime, tEndTime, &dCnt)) < 0)
    {
        log_print(LOGN_CRI, LH"ERROR IN dGet_CONDMsg() dRet[%d]", LT, dRet);
        smsg.common.mml_err		= eDBQUERYERROR;
        smsg.common.cont_flag	= DBM_END;
        smsg.head.msg_len		= 0;
        if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
        {
            log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
            return -3;
        }
        return -4;
    }

	if( (dCnt%MAX_CONDMSG_MMC) == 0) /* MAX_CONDMSG_MMC : 6 */
		dLoopCnt = dCnt / MAX_CONDMSG_MMC;
	else
		dLoopCnt = (dCnt/MAX_CONDMSG_MMC)+1;
	log_print(LOGN_INFO, LH"COND MSG GET COUNT [%d] LOOP[%d]", LT, dCnt, dLoopCnt);

    if(dCnt == 0)
    {
		smsg.common.mml_err		= DBM_SUCCESS;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= 0;
        if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
        {
            log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
            return -5;
        }
        return 0;
    }

    for(i = 0; i < dLoopCnt; i++)
    {
        for(j = 0; j < MAX_CONDMSG_MMC; j++)
        {
            stList.stCONDMsg[j].uiTime = stCOND_DB[dIdx].uiTime;

            sprintf(stList.stCONDMsg[j].szMessage, "%s", stCOND_DB[dIdx].szMessage);

            if(dIdx + 1 == dCnt)
                break;

            dIdx++;
        }

		smsg.common.TotPage = dLoopCnt;
        smsg.common.CurPage = i + 1;

        if(j == MAX_CONDMSG_MMC)
            stList.dCount = MAX_CONDMSG_MMC;
        else
            stList.dCount = j + 1;

        memcpy(smsg.data, &stList, sizeof(st_CondMsg_List));
        smsg.common.mml_err = DBM_SUCCESS;
        if(i == dLoopCnt - 1)
            smsg.common.cont_flag = DBM_END;
        else
            smsg.common.cont_flag = DBM_CONTINUE;

        smsg.head.msg_len = sizeof(st_CondMsg_List) + 1;
        if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
        {
            log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
            return -6;
        }
    }

    return 0;
}

int dis_defect_info(mml_msg *ml, long long llNID)
{
	int							i, dRet, dLoopCnt;
	unsigned int				uTmp;
	size_t						dSlen;
	dbm_msg_t					smsg;
	st_TrendInfo				stTrendInfo;
	st_Defect_24Hours_List		*stDefectList;
	st_Defect_24Hours_List_1Min	*stDefectList1Min;
	int monflag = 0;

    smsg.data[0]			= 0x00;

    smsg.common.total_cnt	= 0;
    smsg.common.TotPage		= 0;
    smsg.common.CurPage		= 0;

	stTrendInfo.ucOfficeID	= (unsigned char)atoi(ml->msg_body[0].para_cont);
	stTrendInfo.ucSysType	= (unsigned char)atoi(ml->msg_body[1].para_cont);
	stTrendInfo.ucSubType	= (unsigned char)atoi(ml->msg_body[2].para_cont);
	stTrendInfo.usL4Code	= (unsigned short)atoi(ml->msg_body[3].para_cont);

	if( (dRet = inet_pton(AF_INET, ml->msg_body[4].para_cont, &uTmp)) <= 0)
	{
		log_print(LOGN_CRI, LH"FAILED IN inet_pton(%s) dRet[%d]", LT, ml->msg_body[0].para_cont, dRet);

		dSlen					= strlen(smsg.data) + 1;
		smsg.common.mml_err		= eINVALID_IP;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= dSlen;

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -1;
		}
		return -2;
	}
	stTrendInfo.uiIP	= ntohl(uTmp);

	stTrendInfo.ucSYSID		= (unsigned char)atoi(ml->msg_body[5].para_cont);
	stTrendInfo.ucBSCID		= (unsigned char)atoi(ml->msg_body[6].para_cont);
	stTrendInfo.usBTSID		= (unsigned char)atoi(ml->msg_body[7].para_cont);
	stTrendInfo.ucFAID		= (unsigned char)atoi(ml->msg_body[8].para_cont);
	stTrendInfo.ucSECID		= (unsigned char)atoi(ml->msg_body[9].para_cont);
	stTrendInfo.ucDefectCode	= (unsigned char)atoi(ml->msg_body[10].para_cont);
	monflag	= (unsigned char)atoi(ml->msg_body[11].para_cont);

	if( monflag == 0 )
	{
		if(gMonTotal->dUsedCnt%12 == 0)
			dLoopCnt	= (gMonTotal->dUsedCnt/12);
		else
			dLoopCnt	= (gMonTotal->dUsedCnt/12)+1;
		for(i = dLoopCnt-1; i >= 0; i--)
		{
#if 1 /* INYOUNG */
			if( (dRet = dGetDefTrendInfo(i, &stTrendInfo, (st_Defect_24Hours_List*)&smsg.data[0])) < 0)
#else
			if( (dRet = dGetDefTrendInfo(i, &stTrendInfo, (st_Defect_24Hours_List*)&smsg.data[0])) == 0)
#endif
			{
				log_print(LOGN_CRI, LH"INFORM IN dGetDefTrendInfo() dRet[%d]", LT, dRet);

				smsg.common.mml_err		= eNotFoundData;
				smsg.common.cont_flag	= DBM_END;
				smsg.head.msg_len		= 0;
				if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
				{
					log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
					return -3;
				}
				return -4;
			}
#if 1 /* INYOUNG */
			else if((i > 0) && (dRet == 0)){
				continue;
			}
#endif
			else
			{
				smsg.common.mml_err = DBM_SUCCESS;
				stDefectList	= (st_Defect_24Hours_List*)&smsg.data[0];
				if(i == 0)
				{
					stDefectList->cFlag		= MSG_END;
					smsg.common.cont_flag	= DBM_END;
				}
				else
				{
					stDefectList->cFlag		= MSG_CONTINUE;
					smsg.common.cont_flag	= DBM_CONTINUE;
				}

				smsg.head.msg_len = sizeof(st_Defect_24Hours_List) + 1;
				if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
				{
					log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
					return -6;
				}
			}
		}
	}
	else
	{
		if(gMonTotal1Min->dUsedCnt%60 == 0)
			dLoopCnt	= (gMonTotal1Min->dUsedCnt/60);
		else
			dLoopCnt	= (gMonTotal1Min->dUsedCnt/60)+1;
		for(i = dLoopCnt-1; i >= 0; i--)
		{
#if 1 /* INYOUNG */
			if( (dRet = dGetDefTrendInfo1Min(i, &stTrendInfo, (st_Defect_24Hours_List_1Min *)&smsg.data[0])) < 0)
#else
			if( (dRet = dGetDefTrendInfo1Min(i, &stTrendInfo, (st_Defect_24Hours_List_1Min *)&smsg.data[0])) == 0)
#endif
			
			{
				log_print(LOGN_CRI, LH"INFORM IN dGetDefTrendInfo() dRet[%d]", LT, dRet);

				smsg.common.mml_err		= eNotFoundData;
				smsg.common.cont_flag	= DBM_END;
				smsg.head.msg_len		= 0;
				if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0) {
					log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
					return -3;
				}
				return -4;
			}
#if 1 /* INYOUNG */
			else if( (i > 0) && (dRet == 0)) {
				continue;
			}
#endif
			else
			{
				smsg.common.mml_err = DBM_SUCCESS;
				stDefectList1Min	= (st_Defect_24Hours_List_1Min *)&smsg.data[0];
				if(i == 0) {
					stDefectList1Min->cFlag	= MSG_END;
					smsg.common.cont_flag	= DBM_END;
				}
				else {
					stDefectList1Min->cFlag	= MSG_CONTINUE;
					smsg.common.cont_flag	= DBM_CONTINUE;
				}

				smsg.head.msg_len = sizeof(st_Defect_24Hours_List_1Min) + 1;
				if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0) {
					log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
					return -6;
				}
			}
		}
	}

    return 0;
}

int dis_trend_info(mml_msg *ml, INT64 llNID)
{
	int							i, dRet, dLoopCnt;
	unsigned int				uTmp;
	size_t						dSlen;
	dbm_msg_t					smsg;
	st_TrendInfo				stTrendInfo;
	st_PtStatus_24Hours_List	*stPtStatus;
	st_PtStatus_24Hours_List_1Min	*stPtStatus1Min;
	int monflag = 0;

	smsg.data[0]			= 0x00;

	smsg.common.total_cnt	= 0;
	smsg.common.TotPage		= 0;
	smsg.common.CurPage		= 0;

	stTrendInfo.ucOfficeID	= (unsigned char)atoi(ml->msg_body[0].para_cont);
	stTrendInfo.ucSysType	= (unsigned char)atoi(ml->msg_body[1].para_cont);
	stTrendInfo.ucSubType	= (unsigned char)atoi(ml->msg_body[2].para_cont);
	stTrendInfo.usL4Code	= (unsigned short)atoi(ml->msg_body[3].para_cont);

	if( (dRet = inet_pton(AF_INET, ml->msg_body[4].para_cont, &uTmp)) <= 0)
	{
		log_print(LOGN_CRI, LH"FAILED IN inet_pton(%s) dRet[%d]", LT, ml->msg_body[0].para_cont, dRet);

		dSlen					= strlen(smsg.data) + 1;
		smsg.common.mml_err		= eINVALID_IP;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= dSlen;

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -1;
		}
		return -2;
	}
	stTrendInfo.uiIP	= ntohl(uTmp);

	stTrendInfo.ucSYSID		= (unsigned char)atoi(ml->msg_body[5].para_cont);
	stTrendInfo.ucBSCID		= (unsigned char)atoi(ml->msg_body[6].para_cont);
	stTrendInfo.usBTSID		= (unsigned char)atoi(ml->msg_body[7].para_cont);
	stTrendInfo.ucFAID		= (unsigned char)atoi(ml->msg_body[8].para_cont);
	stTrendInfo.ucSECID		= (unsigned char)atoi(ml->msg_body[9].para_cont);
	monflag	= (unsigned char)atoi(ml->msg_body[10].para_cont);

	if( monflag == 0 )
	{
		if(gMonTotal->dUsedCnt % FIVE_MIN_PER_HOUR_COUNT == 0)
			dLoopCnt	= (gMonTotal->dUsedCnt / FIVE_MIN_PER_HOUR_COUNT);
		else
			dLoopCnt	= (gMonTotal->dUsedCnt / FIVE_MIN_PER_HOUR_COUNT)+1;
		for(i = dLoopCnt-1; i >= 0; i--)
		{
			dRet = dGetTrendInfo(i, &stTrendInfo, (st_PtStatus_24Hours_List *)&smsg.data[0]);
#if 1 /* INYOUNG */
			if( dRet < 0) {
#else
			if( dRet == 0) {
#endif
				log_print(LOGN_CRI, LH"INFORM IN dGetTrendInfo() dRet[%d]", LT, dRet);

				smsg.common.mml_err		= eNotFoundData;
				smsg.common.cont_flag	= DBM_END;
				smsg.head.msg_len		= 0;
				if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0) {
					log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
					return -3;
				}
				return -4;
			}
#if 1 /* INYOUNG */
			else if((i > 0) && (dRet == 0)){
				continue;
			}	
#endif
			else
			{
				smsg.common.mml_err = DBM_SUCCESS;
				stPtStatus	= (st_PtStatus_24Hours_List*)&smsg.data[0];
				if(i == 0) {
					stPtStatus->cFlag		= MSG_END;
					smsg.common.cont_flag	= DBM_END;
				}
				else {
					stPtStatus->cFlag		= MSG_CONTINUE;
					smsg.common.cont_flag	= DBM_CONTINUE;
				}

				smsg.head.msg_len = sizeof(st_PtStatus_24Hours_List) + 1;
				if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0) {
					log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
					return -6;
				}
			}
		}
	}
	else
	{
		if(gMonTotal1Min->dUsedCnt % ONE_MIN_PER_HOUR_COUNT == 0)
			dLoopCnt	= (gMonTotal1Min->dUsedCnt / ONE_MIN_PER_HOUR_COUNT);
		else
			dLoopCnt	= (gMonTotal1Min->dUsedCnt / ONE_MIN_PER_HOUR_COUNT)+1;
		for(i = dLoopCnt-1; i >= 0; i--)
		{
			dRet = dGetTrendInfo1Min(i, &stTrendInfo, (st_PtStatus_24Hours_List_1Min *)&smsg.data[0]);
#if 1 /* INYOUNG */
			if( dRet < 0)
#else
			if( dRet == 0)
#endif
			{
				log_print(LOGN_CRI, LH"INFORM IN dGetTrendInfo() dRet[%d]", LT, dRet);

				smsg.common.mml_err		= eNotFoundData;
				smsg.common.cont_flag	= DBM_END;
				smsg.head.msg_len		= 0;
				if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0) {
					log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
					return -3;
				}
				return -4;
			}
#if 1 /* INYOUNG */
			else if((i > 0) && (dRet == 0)){
				continue;
			}
#endif
			else
			{
				smsg.common.mml_err = DBM_SUCCESS;
				stPtStatus1Min	= (st_PtStatus_24Hours_List_1Min *)&smsg.data[0];
				if(i == 0) {
					stPtStatus1Min->cFlag		= MSG_END;
					smsg.common.cont_flag	= DBM_END;
				}
				else {
					stPtStatus1Min->cFlag		= MSG_CONTINUE;
					smsg.common.cont_flag	= DBM_CONTINUE;
				}

				smsg.head.msg_len = sizeof(st_PtStatus_24Hours_List_1Min) + 1;
				if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
				{
					log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
					return -6;
				}
			}
		}
	}

	return 0;
}

int dReloadInfo(mml_msg *ml, INT64 llNID)
{
	int				i, dRet;
	unsigned int	uType;
	dbm_msg_t		smsg;

    smsg.data[0]			= 0x00;

	for(i = 0; i < ml->num_of_para; i++)
	{
		switch(ml->msg_body[i].para_id)
		{
			case 710:
				uType	= (unsigned int)atoi(ml->msg_body[i].para_cont);
				break;
			default:
				log_print(LOGN_WARN, LH"INVALID para_id[%hu] para_cont[%s]", LT,
					ml->msg_body[i].para_id, ml->msg_body[i].para_cont);
		}
	}

	switch(uType)
	{
		case 0:		/*	CONF_MN		*/
			if( (dRet = dInit_FltIPPool_Info()) < 0)
			{
				log_print(LOGN_CRI,LH"ERROR IN dInit_FltIPPool_Info() dRet[%d]", LT, dRet);
				set_dbm_ret(&smsg, eNotCompleteProcess, DBM_END, 0);
				if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
				{
					log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
					return -1;
				}
				return -2;
			}

			for(i = 0; i < MAX_NTAF_NUM; i++)
			{
				if(pstSubSys->sys[i].uiIP > 0)
				{
					if( (dRet = dSend_FltIPPool_Data(pstSubSys->sys[i].usSysNo)) < 0)
					{
						log_print(LOGN_CRI, LH"FAILED IN dSend_FltIPPool_Data(SysNo[%d]) dRet[%d]", 
							LT, pstSubSys->sys[i].usSysNo, dRet);
						return -3;
					}
				}
			}
			break;
		case 1:		/*	CONF_SVR	*/
			if( (dRet = dInit_WatchFltSVC_Info()) < 0)
			{
				log_print(LOGN_CRI,LH"ERROR IN dInit_WatchFltSVC_Info() dRet[%d]", LT, dRet);
				set_dbm_ret(&smsg, eNotCompleteProcess, DBM_END, 0);
				if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
				{
					log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
					return -4;
				}
				return -5;
			}

			/*	added shared memory by uamyd 2008.01.15	*/
			if( (dRet = dInit_FltSvc_Info()) < 0)
			{
				log_print(LOGN_CRI,LH"ERROR IN dInit_FltSvc_Info() dRet[%d]", LT, dRet);
				set_dbm_ret(&smsg, eNotCompleteProcess, DBM_END, 0);
				if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
				{
					log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
					return -6;
				}
				return -7;
			}

			/* send to ntaf */
			for(i = 0; i < MAX_NTAF_NUM; i++)
			{
				if(pstSubSys->sys[i].uiIP > 0)
				{
					if( (dRet = dSend_FltSvc_Data(pstSubSys->sys[i].usSysNo)) < 0)
					{
						log_print(LOGN_CRI, LH"ERROR IN dSend_FltSvc_Data(usSysNo[%d][%d]) dRet[%d]", 
							LT, i, pstSubSys->sys[i].usSysNo, dRet);
						return -8;
					}
				}
			}
			break;
		case 2:		/*	CONF_SCTP	*/
			if( (dRet = dInit_WatchFltEquip_Info()) < 0)
			{
				log_print(LOGN_CRI,LH"ERROR IN dInit_WatchFltEquip_Info() dRet[%d]", LT, dRet);
				set_dbm_ret(&smsg, eNotCompleteProcess, DBM_END, 0);
				if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
				{
					log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
					return -9;
				}
				return -10;
			}

			/*	added shared memory by uamyd 2008.01.15	*/
			if( (dRet = dInit_FltSCTP_Info()) < 0)
			{
				log_print(LOGN_CRI,LH"ERROR IN dInit_FltSCTP_Info() dRet[%d]", LT, dRet);
				set_dbm_ret(&smsg, eNotCompleteProcess, DBM_END, 0);
				if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
				{
					log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
					return -11;
				}
				return -12;
			}

			/* send to ntaf */
			for(i = 0; i < MAX_NTAF_NUM; i++)
			{
				if(pstSubSys->sys[i].uiIP > 0)
				{
					if( (dRet = dSend_FltSCTP_Data(pstSubSys->sys[i].usSysNo)) < 0)
					{
						log_print(LOGN_CRI, LH"ERROR IN dSend_FltSCTP_Data(usSysNo[%d][%d]) dRet[%d]", 
							LT, i, pstSubSys->sys[i].usSysNo, dRet);
						return -13;
					}
				}
			}
			break;
		case 3:		/*	CONF_ACCESS	*/
			if( (dRet = dInit_WatchInfoAccess()) < 0)
			{
				log_print(LOGN_CRI,LH"ERROR IN dInit_WatchInfoAccess() dRet[%d]", LT, dRet);
				set_dbm_ret(&smsg, eNotCompleteProcess, DBM_END, 0);
				if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
				{
					log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
					return -14;
				}
				return -15;
			}
			break;
		case 4:		/*	CONF_EQUIP	*/
			if( (dRet = dInit_WatchFltEquip_Info()) < 0)
			{
				log_print(LOGN_CRI,LH"ERROR IN dInit_WatchFltEquip_Info() dRet[%d]", LT, dRet);
				set_dbm_ret(&smsg, eNotCompleteProcess, DBM_END, 0);
				if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
				{
					log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
					return -16;
				}
				return -17;
			}
			break;
		case 5:		/*	CONF_MODEL	*/
			if( (dRet = dInit_CTNInfo()) < 0)
			{
				log_print(LOGN_CRI,LH"ERROR IN dInit_CTNInfo() dRet[%d]", LT, dRet);
				set_dbm_ret(&smsg, eNotCompleteProcess, DBM_END, 0);
				if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
				{
					log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
					return -18;
				}
				return -19;
			}
			break;
		case 6:		/*	CONF_IRM	*/
			if( (dRet = dInit_IRMInfo()) < 0)
			{
				log_print(LOGN_CRI,LH"ERROR IN dInit_IRMInfo() dRet[%d]", LT, dRet);
				set_dbm_ret(&smsg, eNotCompleteProcess, DBM_END, 0);
				if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
				{
					log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
					return -20;
				}
				return -21;
			}
			break;
		case 7:		/*	CONF_DEFECT	*/
			if( (dRet = dMake_DefectInfoFile()) < 0)
			{
				log_print(LOGN_CRI,LH"ERROR IN dMake_DefectInfoFile() dRet[%d]", LT, dRet);
				set_dbm_ret(&smsg, eNotCompleteProcess, DBM_END, 0);
				if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
				{
					log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
					return -20;
				}
				return -21;
			}
			break;

		default:
			log_print(LOGN_WARN, LH"INVALID CONFTYPE[%hu]", LT, uType);
	}

	set_dbm_ret(&smsg, DBM_SUCCESS, DBM_END, 0);
	if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
		return -22;
	}

	return 0;
}

int dSndDNMSInfo(mml_msg *ml, INT64 llNID)
{
	int				i, dRet;
	char			sSourceFile[PATH_MAX], sTargetPath[PATH_MAX], sTargetFile[PATH_MAX], sTargetFullPath[PATH_MAX], sDate[13];
	unsigned int	uType;
	dbm_msg_t		smsg;
	time_t			tCurrent;
	struct tm		stCurrent;

    smsg.data[0] = 0x00;

	for(i = 0; i < ml->num_of_para; i++)
	{
		switch(ml->msg_body[i].para_id)
		{
			case 710:
				uType	= (unsigned int)atoi(ml->msg_body[i].para_cont);
				break;
			default:
				log_print(LOGN_WARN, LH"INVALID para_id[%hu] para_cont[%s]", LT,
					ml->msg_body[i].para_id, ml->msg_body[i].para_cont);
		}
	}

	if( (tCurrent = time(NULL)) == -1)
	{
		log_print(LOGN_CRI, LH"FAILED IN time(NULL) errno[%d-%s]", LT,
			errno, strerror(errno));
		set_dbm_ret(&smsg, eNotCompleteProcess, DBM_END, 0);
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -1;
		}
		return -2;
	}

	if(localtime_r( (const time_t*)&tCurrent, &stCurrent) == NULL)
	{
		log_print(LOGN_CRI, LH"FAILED IN localtime_t(tCurrent[%lu]) errno[%d-%s]", LT,
				tCurrent, errno, strerror(errno));
		set_dbm_ret(&smsg, eNotCompleteProcess, DBM_END, 0);
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -3;
		}
		return -4;
	}

	if( (dRet = strftime(sDate, 13, "%Y%m%d%H%M", &stCurrent)) != 12)
	{
		log_print(LOGN_CRI, LH"FAILED IN strftime(sDate[%s]) errno[%d-%s]\n", LT,
				sDate, errno, strerror(errno));
		set_dbm_ret(&smsg, eNotCompleteProcess, DBM_END, 0);
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -5;
		}
		return -6;
	}

	sprintf(sTargetPath, "%s/%s", START_PATH, "SVCMON");
	switch(uType)
	{
		case 0:		/*	DFTINF	*/
			sprintf(sSourceFile, "%s%s", DATA_PATH, "DFTINF.DAT");
			sprintf(sTargetFile, "DQM01_DFTINF_M_%s.DAT", sDate);
			break;
		case 1:		/*	SVCINF	*/
			sprintf(sSourceFile, "%s%s", DATA_PATH, "SVCINF.DAT");
			sprintf(sTargetFile, "DQM01_SVCINF_M_%s.DAT", sDate);
			break;
		case 2:		/*	L4INF	*/
			sprintf(sSourceFile, "%s%s", DATA_PATH, "L4INF.DAT");
			sprintf(sTargetFile, "DQM01_L4INF_M_%s.DAT", sDate);
			break;
		case 3:		/*	OFFINF	*/
			sprintf(sSourceFile, "%s%s", DATA_PATH, "OFFINF.DAT");
			sprintf(sTargetFile, "DQM01_OFFINF_M_%s.DAT", sDate);
			break;
		case 4:		/*	IPINF	*/
			sprintf(sSourceFile, "%s%s", DATA_PATH, "IPINF.DAT");
			sprintf(sTargetFile, "DQM01_IPINF_M_%s.DAT", sDate);
			break;
		default:
			log_print(LOGN_WARN, LH"INVALID CONFTYPE[%hu]", LT, uType);
			set_dbm_ret(&smsg, eBadParameter, DBM_END, 0);
			if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
			{
				log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
				return -7;
			}

			return -8;
	}

	sprintf(sTargetFullPath, "%s/%s", sTargetPath, sTargetFile);
	if( (dRet = dCopy(sSourceFile, sTargetFullPath)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dCopy(sSourceFile[%s], sTargetFullPath[%s]) dRet[%d]", LT,
			sSourceFile, sTargetFullPath, dRet);
		set_dbm_ret(&smsg, eNotCompleteProcess, DBM_END, 0);
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -9;
		}
		return -10;
	}

	if( (dRet = dSendMsg_SI_SVCMON(sDate, (unsigned char*)sTargetPath, (unsigned char*)sTargetFile)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dSendMsg_SI_SVCMON(sDate[%s], sTargetPath[%s], sTargetFile[%s]) dRet[%d]", LT,
			sDate, sTargetPath, sTargetFile, dRet);
		set_dbm_ret(&smsg, eNotCompleteProcess, DBM_END, 0);
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -11;
		}
		return -12;
	}

	set_dbm_ret(&smsg, DBM_SUCCESS, DBM_END, 0);
	if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
		return -13;
	}

	return 0;
}

int dis_monthrs_info(mml_msg *ml, long long llNID)
{
	int					i, j, dRet, dLoopCnt, dCnt, dIdx;
	unsigned char		cSvcType, cAlarmType, cExistSvcType, cExistAlarmType, cExistBranchID, cNeedComma;
	unsigned short		huBranchID;
	unsigned int        dSvcIP; 
	unsigned char          cExistSvcIPType=0;//20110616 dcham
	size_t				szWhereLen;
	char				sWhere[BUFSIZ];
        size_t                  dSlen;
	dbm_msg_t			smsg;
	st_MON_Thres_List	stList;
	st_MON_ThresMMC		stMONThresMMC[MAX_MON_THRESHOLD_COUNT];

	dCnt			= 0;
	dIdx			= 0;
	cExistBranchID	= 0;
	cExistSvcType	= 0;
	cExistAlarmType	= 0;
	cNeedComma		= 0;

	memset(&smsg, 0x00, sizeof(dbm_msg_t));
	memset(&stMONThresMMC, 0x00, sizeof(st_MON_ThresMMC)*MAX_MON_THRESHOLD_COUNT);

	for(i = 0; i < ml->num_of_para; i++)
	{
		switch(ml->msg_body[i].para_id)
		{
			case 605:
				huBranchID	= (unsigned short)atoi(ml->msg_body[i].para_cont);
				cExistBranchID	= 1;
				break;
			case 606:
				cSvcType	= (unsigned char)atoi(ml->msg_body[i].para_cont);
				cExistSvcType	= 1;
				break;
			case 607:
				cAlarmType	= (unsigned char)atoi(ml->msg_body[i].para_cont);
				cExistAlarmType	= 1;
				break;
				//20110616 dcham
			case 622:
				if( (dRet = inet_pton(AF_INET, ml->msg_body[i].para_cont, &dSvcIP)) <= 0)
				{
					log_print(LOGN_CRI, LH"FAILED IN inet_pton(%s) dRet[%d]",LT,  ml->msg_body[i].para_cont, dRet);

					dSlen                   = strlen(smsg.data) + 1;
					smsg.common.mml_err             = eINVALID_IP;
					smsg.common.cont_flag   = DBM_END;
					smsg.head.msg_len               = dSlen;

					if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
					{
						log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
						return -1;
					}
					return -2;
				}
				else
			           dSvcIP = ntohl(dSvcIP);
				cExistSvcIPType	= 1;
				break;
		}
	}

	if(ml->num_of_para)
	{
		sprintf(sWhere, " WHERE ");
		szWhereLen	= strlen(sWhere);

		if(cExistBranchID)
		{
			if(cNeedComma)
			{
				szWhereLen = strlen(sWhere);
				sprintf(&sWhere[szWhereLen], " AND ");
			}
			else
				cNeedComma	= 1;

			szWhereLen = strlen(sWhere);
			sprintf(&sWhere[szWhereLen], "BranchID=%hu", huBranchID);
		}

		if(cExistSvcType)
		{
			if(cNeedComma)
			{
				szWhereLen = strlen(sWhere);
				sprintf(&sWhere[szWhereLen], " AND ");
			}
			else
				cNeedComma	= 1;

			szWhereLen = strlen(sWhere);
			sprintf(&sWhere[szWhereLen], "SvcType=%hu", cSvcType);
		}

		if(cExistAlarmType)
		{
			if(cNeedComma)
			{
				szWhereLen = strlen(sWhere);
				sprintf(&sWhere[szWhereLen], " AND ");
			}
			else
				cNeedComma	= 1;

			szWhereLen = strlen(sWhere);
			sprintf(&sWhere[szWhereLen], "AlarmType=%hu", cAlarmType);
		}
		//dcham 20110616
		if(cExistSvcIPType)
		{
			if(cNeedComma)
			{
				szWhereLen = strlen(sWhere);
				sprintf(&sWhere[szWhereLen], " AND ");
			}
			else
				cNeedComma	= 1;

			szWhereLen = strlen(sWhere);
			sprintf(&sWhere[szWhereLen], "SvcIP=%u", dSvcIP);
		}

		szWhereLen = strlen(sWhere);
		sprintf(&sWhere[szWhereLen], " ");

		log_print(LOGN_DEBUG, LH"sWhere[%s]", LT, sWhere);
	}
	else
	{
		szWhereLen	= 0;
		sWhere[0]	= 0x00;
	}

	if( (dRet = dGetMONThres(&stMySQL, stMONThresMMC, &dCnt, sWhere)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dGetMONThres() dRet[%d]", LT, dRet);

		smsg.common.mml_err		= eDBQUERYERROR;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= 0;
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -1;
		}
		return -2;
	}

	if(dCnt >= MAX_MON_THRESHOLD_COUNT)
	{
		sprintf(smsg.data, "OVER MAX INFO_MON_THRESHOLD COUNT[%d]", MAX_MON_THRESHOLD_COUNT);

		smsg.common.mml_err		= eOVERMAXROW;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= strlen(smsg.data)+1;
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -3;
		}
		return -4;
	}

	if( (dCnt%DEF_MON_THRESHOLD_COUNT) == 0)
		dLoopCnt = dCnt / DEF_MON_THRESHOLD_COUNT;
	else
		dLoopCnt = (dCnt/DEF_MON_THRESHOLD_COUNT)+1;

	if(dCnt == 0)
	{
		smsg.common.mml_err		= eNotFoundData;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= strlen(smsg.data)+1;

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -5;
		}
	}

	for(i = 0; i < dLoopCnt; i++)
	{
		for(j = 0; j < DEF_MON_THRESHOLD_COUNT; j++)
		{
			stList.stMonThreshold[j].huBranchID		= stMONThresMMC[dIdx].huBranchID;
			stList.stMonThreshold[j].cSvcType		= stMONThresMMC[dIdx].cSvcType;
			stList.stMonThreshold[j].cAlarmType		= stMONThresMMC[dIdx].cAlarmType;
			stList.stMonThreshold[j].dSvcIP 		= stMONThresMMC[dIdx].dSvcIP; // dcham 20110616
			stList.stMonThreshold[j].cStartHour		= stMONThresMMC[dIdx].cStartHour;
			stList.stMonThreshold[j].cDayRange		= stMONThresMMC[dIdx].cDayRange;

			stList.stMonThreshold[j].huDayRate		= stMONThresMMC[dIdx].huDayRate;
			stList.stMonThreshold[j].huNightRate	= stMONThresMMC[dIdx].huNightRate;
			stList.stMonThreshold[j].uDayMinTrial	= stMONThresMMC[dIdx].uDayMinTrial;
			stList.stMonThreshold[j].uNigthMinTrial	= stMONThresMMC[dIdx].uNigthMinTrial;
			stList.stMonThreshold[j].uPeakTrial		= stMONThresMMC[dIdx].uPeakTrial;

			sprintf(stList.stMonThreshold[j].szDesc, "%s", stMONThresMMC[dIdx].szDesc);

			log_print(LOGN_DEBUG, "idx[%d] [SEND ALL MON THRS INFO] "
				"huBranchID[%hu] cSvcType[%hu] cAlarmType[%hu] dSvcIP[%s] cStartHour[%hu] cDayRange[%hu] huDayRate[%hu] "
				"huNightRate[%hu] uDayMinTrial[%hu] uNigthMinTrial[%u] uPeakTrial[%hu] szDesc[%s]",
				dIdx, stList.stMonThreshold[j].huBranchID, stList.stMonThreshold[j].cSvcType, stList.stMonThreshold[j].cAlarmType,
				util_cvtipaddr(NULL, stList.stMonThreshold[j].dSvcIP), // dcham 20110616
				stList.stMonThreshold[j].cStartHour, stList.stMonThreshold[j].cDayRange, stList.stMonThreshold[j].huDayRate,
				stList.stMonThreshold[j].huNightRate, stList.stMonThreshold[j].uDayMinTrial, stList.stMonThreshold[j].uNigthMinTrial,
				stList.stMonThreshold[j].uPeakTrial, stList.stMonThreshold[j].szDesc);

			if( (dIdx+1) == dCnt)
				break;
			dIdx++;
		}
		smsg.common.TotPage	= dLoopCnt;
		smsg.common.CurPage	= i+1;

		if(j == DEF_MON_THRESHOLD_COUNT)
			stList.dCount = DEF_MON_THRESHOLD_COUNT;
		else
			stList.dCount = j+1;

		memcpy(smsg.data, &stList, sizeof(st_MON_Thres_List));
		smsg.common.mml_err = DBM_SUCCESS;
		if(i == (dLoopCnt-1))
			smsg.common.cont_flag = DBM_END;
		else
			smsg.common.cont_flag = DBM_CONTINUE;

		smsg.head.msg_len = sizeof(st_MON_Thres_List);
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -6;
		}
	}

	return 0;
}

int set_monthrs_info(mml_msg *ml, long long llNID)
{
	int				i, dRet, dSuccess;
	unsigned int	dSvcIP; 
	size_t			szLength;
	unsigned char	cEndHour;
	st_MON_ThresMMC	stMONThres;
	dbm_msg_t		smsg;

	smsg.data[0]	= 0x00;
	memset(&stMONThres, 0x00, sizeof(st_MON_ThresMMC));
	for(i = 0; i < ml->num_of_para; i++)
	{
		switch(ml->msg_body[i].para_id)
		{
			case 605:
				stMONThres.huBranchID	= (unsigned short)atoi(ml->msg_body[i].para_cont);
				break;
			case 606:
				stMONThres.cSvcType		= (unsigned char)atoi(ml->msg_body[i].para_cont);
				break;
			case 607:
				stMONThres.cAlarmType	= (unsigned char)atoi(ml->msg_body[i].para_cont);
				break;
				// dcham 20110616
			case 622:
				if( (dRet = inet_pton(AF_INET, ml->msg_body[i].para_cont, &dSvcIP)) <= 0)
				{
					log_print(LOGN_CRI, LH"FAILED IN inet_pton(%s) dRet[%d]",LT, ml->msg_body[i].para_cont, dRet);

					szLength = strlen(smsg.data) + 1;
					smsg.common.mml_err = eINVALID_IP;
					smsg.common.cont_flag = DBM_END;
					smsg.head.msg_len = szLength;

					if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
					{
						log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
						return -1;
					}
					return -2;
				}
			           
				stMONThres.dSvcIP = ntohl(dSvcIP);
				break;
		}
	}

	if(stMONThres.huBranchID != 11)
	{
		if( (dRet = dSelectMONThres(&stMySQL, &stMONThres)) < 0)
	    {
			log_print(LOGN_CRI, LH"ERROR IN dSelectMONThres() dRet[%d]", LT, dRet);

			szLength = strlen(smsg.data) + 1;
			if(dRet == -3)
				smsg.common.mml_err	= eDuplicateEntry;
			else if(dRet == -4)
				smsg.common.mml_err = eNotFoundData;
			else
				smsg.common.mml_err	= eDBQUERYERROR;

			smsg.common.cont_flag	= DBM_END;
			smsg.head.msg_len		= szLength;

			if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
			{
				log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
				return -1;
			}
			return -2;
	    }
		else
			cEndHour = stMONThres.cDayRange;

		for(i = 0; i < ml->num_of_para; i++)
		{
			switch(ml->msg_body[i].para_id)
			{
				case 605:
				case 606:
				case 607:
					break;
				case 703:
					stMONThres.cStartHour		= (unsigned char)atoi(ml->msg_body[i].para_cont);
					break;
				case 704:
					cEndHour					= (unsigned int)atoi(ml->msg_body[i].para_cont);
					break;
				case 426:
					stMONThres.huDayRate		= (unsigned short)atoi(ml->msg_body[i].para_cont);
					break;
				case 427:
					stMONThres.huNightRate		= (unsigned short)atoi(ml->msg_body[i].para_cont);
					break;
				case 428:
					stMONThres.uDayMinTrial		= (unsigned int)atoi(ml->msg_body[i].para_cont);
					break;
				case 429:
					stMONThres.uNigthMinTrial	= (unsigned int)atoi(ml->msg_body[i].para_cont);
					break;
				case 430:
					stMONThres.uPeakTrial		= (unsigned int)atoi(ml->msg_body[i].para_cont);
					break;
				case 479:
					if( (szLength = strlen(ml->msg_body[i].para_cont)) > 0)
					{
						if(szLength > MAX_SDESC)
							strncpy(stMONThres.szDesc, ml->msg_body[i].para_cont, MAX_SDESC-1);
						else
							strcpy(stMONThres.szDesc, ml->msg_body[i].para_cont);
					}
					else
						sprintf(stMONThres.szDesc, "-");
					break;
				default:
					log_print(LOGN_CRI, LH"ERROR IN Parameter para_id[%hu]", LT, ml->msg_body[i].para_id);
			}
		}

		if( (stMONThres.huBranchID>MAX_MON_OFFICE_IDX) || (stMONThres.cSvcType>MAX_SYSTEM_TYPE_IDX+1) || (stMONThres.cAlarmType>MAX_ALARMTYPE_IDX))
		{
			szLength				= strlen(smsg.data) + 1;
			smsg.common.mml_err		= eBadParameter;
			smsg.common.cont_flag	= DBM_END;
			smsg.head.msg_len		= szLength;

			if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
			{
				log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
				return -3;
			}
			return -4;
		}

		if(stMONThres.cStartHour > cEndHour)
		{
			szLength				= strlen(smsg.data) + 1;
			smsg.common.mml_err		= eBadParameter;
			smsg.common.cont_flag	= DBM_END;
			smsg.head.msg_len		= szLength;

			if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
			{
				log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
				return -5;
			}
			return -6;
		}
		else
			stMONThres.cDayRange = cEndHour - stMONThres.cStartHour;

	    if( (dRet = dChgMONThres(&stMySQL, &stMONThres)) < 0)
	    {
			log_print(LOGN_CRI, LH"ERROR IN dChgMONThres() dRet[%d]", LT, dRet);

			szLength				= strlen(smsg.data) + 1;
			smsg.common.mml_err		= eDBQUERYERROR;
			smsg.common.cont_flag	= DBM_END;
			smsg.head.msg_len		= szLength;

			if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
			{
				log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
				return -7;
			}
			return -8;
	    }
		else
		{
			if( (dRet = dInit_WatchInfoMonThreshold()) < 0)
				log_print(LOGN_CRI, LH"ERROR IN dInit_WatchInfoMonThreshold() dRet[%d]", LT, dRet);

		    szLength				= strlen(smsg.data) + 1;
		    smsg.common.mml_err		= DBM_SUCCESS;
		    smsg.common.cont_flag	= DBM_END;
		    smsg.head.msg_len		= szLength;

			if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
			{
				log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
				return -9;
			}
		}
	}
	else
	{
		if(stMONThres.cSvcType >= SYSTEM_TYPE_PDSN)
		{
			szLength				= strlen(smsg.data) + 1;
			smsg.common.mml_err		= eBadParameter;
			smsg.common.cont_flag	= DBM_END;
			smsg.head.msg_len		= szLength;

			if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
			{
				log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
				return -9;
			}
			return -10;
		}

		for(dSuccess = 0, stMONThres.huBranchID = 0; stMONThres.huBranchID <= OFFICE_IDX_DJ; stMONThres.huBranchID++)
		{
			if( (dRet = dSelectMONThres(&stMySQL, &stMONThres)) < 0)
		    {
				log_print(LOGN_CRI, LH"ERROR IN dSelectMONThres() dRet[%d]", LT, dRet);
				continue;
		    }
			else
				cEndHour = stMONThres.cDayRange;

			for(i = 0; i < ml->num_of_para; i++)
			{
				switch(ml->msg_body[i].para_id)
				{
					case 605:
					case 606:
					case 607:
						break;
					case 703:
						stMONThres.cStartHour		= (unsigned char)atoi(ml->msg_body[i].para_cont);
						break;
					case 704:
						cEndHour					= (unsigned int)atoi(ml->msg_body[i].para_cont);
						break;
					case 426:
						stMONThres.huDayRate		= (unsigned short)atoi(ml->msg_body[i].para_cont);
						break;
					case 427:
						stMONThres.huNightRate		= (unsigned short)atoi(ml->msg_body[i].para_cont);
						break;
					case 428:
						stMONThres.uDayMinTrial		= (unsigned int)atoi(ml->msg_body[i].para_cont);
						break;
					case 429:
						stMONThres.uNigthMinTrial	= (unsigned int)atoi(ml->msg_body[i].para_cont);
						break;
					case 430:
						stMONThres.uPeakTrial		= (unsigned int)atoi(ml->msg_body[i].para_cont);
						break;
					case 479:
						if( (szLength = strlen(ml->msg_body[i].para_cont)) > 0)
						{
							if(szLength > MAX_SDESC)
								strncpy(stMONThres.szDesc, ml->msg_body[i].para_cont, MAX_SDESC-1);
							else
								strcpy(stMONThres.szDesc, ml->msg_body[i].para_cont);
						}
						else
							sprintf(stMONThres.szDesc, "-");
						break;
					default:
						log_print(LOGN_CRI, LH"ERROR IN Parameter para_id[%hu]", LT, ml->msg_body[i].para_id);
				}
			}

			if( (stMONThres.huBranchID>MAX_MON_OFFICE_IDX) || (stMONThres.cSvcType>MAX_SYSTEM_TYPE_IDX+1) || (stMONThres.cAlarmType>MAX_ALARMTYPE_IDX))
			{
				log_print(LOGN_CRI, LH"huBranchID[%hu] cSvcType[%hu] cAlarmType[%hu]", LT,
					stMONThres.huBranchID, stMONThres.cSvcType, stMONThres.cAlarmType);
				continue;
			}

			if(stMONThres.cStartHour > cEndHour)
			{
				log_print(LOGN_CRI, LH"cStartHour[%hu] > cEndHour[%hu]", LT,
					stMONThres.cStartHour, cEndHour);
				continue;
			}
			else
				stMONThres.cDayRange = cEndHour - stMONThres.cStartHour;

		    if( (dRet = dChgMONThres(&stMySQL, &stMONThres)) < 0)
		    {
				log_print(LOGN_CRI, LH"ERROR IN dChgMONThres() dRet[%d]", LT, dRet);
				continue;
		    }
			else
				dSuccess++;
		}

		if(dSuccess)
		{
			log_print(LOGN_DEBUG, LH"dSuccess[%d]", LT, dSuccess);

			if( (dRet = dInit_WatchInfoMonThreshold()) < 0)
				log_print(LOGN_CRI, LH"ERROR IN dInit_WatchInfoMonThreshold() dRet[%d]", LT, dRet);
		}

	    szLength				= strlen(smsg.data) + 1;
	    smsg.common.mml_err		= DBM_SUCCESS;
	    smsg.common.cont_flag	= DBM_END;
	    smsg.head.msg_len		= szLength;

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -9;
		}
	}

    return 0;
}

int dis_thrs_info(mml_msg *ml, long long llNID)
{
	int				i, j, dRet, dLoopCnt, dCnt, dIdx;
	unsigned char	cSvcType, cExistSvcType;
	size_t			szWhereLen;
	char			sWhere[BUFSIZ];
	dbm_msg_t		smsg;
	st_Thres_MMC	stList;
	st_ThresMMC		stThresMMC[MAX_DEFECT_THRES];

	dCnt			= 0;
	dIdx			= 0;
	cExistSvcType	= 0;

	memset(&smsg, 0x00, sizeof(dbm_msg_t));
	memset(&stThresMMC, 0x00, sizeof(st_ThresMMC)*MAX_DEFECT_THRES);

	for(i = 0; i < ml->num_of_para; i++)
	{
		switch(ml->msg_body[i].para_id)
		{
			case 610:
				cSvcType		= (unsigned char)atoi(ml->msg_body[i].para_cont);
				cExistSvcType	= 1;
				break;
			default:
				log_print(LOGN_WARN, LH"INVALID para_id[%hu] para_cont[%s]", LT,
					ml->msg_body[i].para_id, ml->msg_body[i].para_cont);
		}
	}

	if(ml->num_of_para)
	{
		sprintf(sWhere, " WHERE ");
		szWhereLen	= strlen(sWhere);

		if(cExistSvcType)
		{
			szWhereLen = strlen(sWhere);
			sprintf(&sWhere[szWhereLen], "SvcType=%hu", cSvcType);
		}

		szWhereLen = strlen(sWhere);
		sprintf(&sWhere[szWhereLen], " ");

		log_print(LOGN_DEBUG, LH"sWhere[%s]", LT, sWhere);
	}
	else
	{
		szWhereLen	= 0;
		sWhere[0]	= 0x00;
	}

	if( (dRet = dGetThres(&stMySQL, stThresMMC, &dCnt, sWhere)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dGetThres() dRet[%d]", LT, dRet);

		smsg.common.mml_err		= eDBQUERYERROR;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= 0;
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -3;
		}
		return -4;
	}

	if(dCnt >= MAX_DEFECT_THRES)
	{
		sprintf(smsg.data, "OVER MAX INFO_DEFFECT_THRESHOLD MAX_DEFECT_THRES[%d]", MAX_DEFECT_THRES);

		smsg.common.mml_err		= eOVERMAXROW;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= strlen(smsg.data)+1;
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -5;
		}
		return -6;
	}

	if( (dCnt%DEF_THRES_MMC) == 0)
		dLoopCnt = dCnt / DEF_THRES_MMC;
	else
		dLoopCnt = (dCnt/DEF_THRES_MMC)+1;

	if(dCnt == 0)
	{
		smsg.common.mml_err		= eNotFoundData;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= strlen(smsg.data)+1;

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -7;
		}
	}

	for(i = 0; i < dLoopCnt; i++)
	{
		for(j = 0; j < DEF_THRES_MMC; j++)
		{
			stList.stThres[j].cSvcType			= stThresMMC[dIdx].cSvcType;
			stList.stThres[j].uTCPSetupTime		= stThresMMC[dIdx].uTCPSetupTime;
			stList.stThres[j].uResponseTime		= stThresMMC[dIdx].uResponseTime;
			stList.stThres[j].uUpThroughput		= stThresMMC[dIdx].uUpThroughput;
			stList.stThres[j].uDnThroughput		= stThresMMC[dIdx].uDnThroughput;
			stList.stThres[j].uUpRetransCount	= stThresMMC[dIdx].uUpRetransCount;
			stList.stThres[j].uDnRetransCount	= stThresMMC[dIdx].uDnRetransCount;
			stList.stThres[j].uUpJitter			= stThresMMC[dIdx].uUpJitter;
			stList.stThres[j].uDnJitter			= stThresMMC[dIdx].uDnJitter;
			stList.stThres[j].uUpPacketLoss		= stThresMMC[dIdx].uUpPacketLoss;
			stList.stThres[j].uDnPacketLoss		= stThresMMC[dIdx].uDnPacketLoss;

			sprintf(stList.stThres[j].szDesc, "%s", stThresMMC[dIdx].szDesc);

			log_print(LOGN_DEBUG, "idx[%d] [SEND ALL DEFECT THRS INFO] "
				"cSvcType[%hu] uTCPSetupTime[%u] uResponseTime[%u] uUpThroughput[%u] uDnThroughput[%u] uUpRetransCount[%hu] "
				"uDnRetransCount[%u] uUpJitter[%u] uDnJitter[%u] uUpPacketLoss[%u] uDnPacketLoss[%u] szDesc[%s]",
				dIdx, stList.stThres[j].cSvcType, stList.stThres[j].uTCPSetupTime, stList.stThres[j].uResponseTime, stList.stThres[j].uUpThroughput,
				stList.stThres[j].uDnThroughput, stList.stThres[j].uUpRetransCount, stList.stThres[j].uDnRetransCount, stList.stThres[j].uUpJitter,
				stList.stThres[j].uDnJitter, stList.stThres[j].uUpPacketLoss, stList.stThres[j].uDnPacketLoss, stList.stThres[j].szDesc);

			if( (dIdx+1) == dCnt)
				break;
			dIdx++;
		}
		smsg.common.TotPage	= dLoopCnt;
		smsg.common.CurPage	= i+1;

		if(j == DEF_THRES_MMC)
			stList.dCount = DEF_THRES_MMC;
		else
			stList.dCount = j+1;

		memcpy(smsg.data, &stList, sizeof(st_Thres_MMC));
		smsg.common.mml_err = DBM_SUCCESS;
		if(i == (dLoopCnt-1))
			smsg.common.cont_flag = DBM_END;
		else
			smsg.common.cont_flag = DBM_CONTINUE;

		smsg.head.msg_len = sizeof(st_Thres_MMC);
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -8;
		}
	}

	return 0;
}

int set_thrs_info(mml_msg *ml, long long llNID)
{
	int				i, dRet;
	size_t			dSlen;
	st_ThresMMC		stThres;
	dbm_msg_t		smsg;

	smsg.data[0]		= 0x00;
	memset(&stThres, 0x00, sizeof(st_ThresMMC));
	for(i = 0; i < ml->num_of_para; i++)
	{
		if(ml->msg_body[i].para_id == 610)
		{
			stThres.cSvcType = (unsigned char)atoi(ml->msg_body[i].para_cont);
			break;
		}
	}

	if(stThres.cSvcType == 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN Parameter cSvcType[%hu]", LT, stThres.cSvcType);

		dSlen					= strlen(smsg.data) + 1;
		smsg.common.mml_err		= eBadParameter;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= dSlen;

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -1;
		}
		return -2;
	}

	if( (dRet = dSelectThres(&stMySQL, &stThres)) < 0)
    {
		log_print(LOGN_CRI, LH"ERROR IN dSelectThres() dRet[%d]", LT, dRet);

		dSlen = strlen(smsg.data) + 1;
		if(dRet == -3)
			smsg.common.mml_err	= eDuplicateEntry;
		else if(dRet == -4)
			smsg.common.mml_err = eNotFoundData;
		else
			smsg.common.mml_err	= eDBQUERYERROR;

		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= dSlen;

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -3;
		}
		return -4;
    }

	for(i = 0; i < ml->num_of_para; i++)
	{
		switch(ml->msg_body[i].para_id)
		{
			case 610:
				break;
			case 611:
				stThres.uTCPSetupTime	= (unsigned int)atoi(ml->msg_body[i].para_cont);
				break;
			case 612:
				stThres.uResponseTime	= (unsigned int)atoi(ml->msg_body[i].para_cont);
				break;
			case 613:
				stThres.uUpThroughput	= (unsigned int)atoi(ml->msg_body[i].para_cont);
				break;
			case 614:
				stThres.uDnThroughput	= (unsigned int)atoi(ml->msg_body[i].para_cont);
				break;
			case 615:
				stThres.uUpRetransCount	= (unsigned int)atoi(ml->msg_body[i].para_cont);
				break;
			case 616:
				stThres.uDnRetransCount	= (unsigned int)atoi(ml->msg_body[i].para_cont);
				break;
			case 617:
				stThres.uUpJitter	= (unsigned int)atoi(ml->msg_body[i].para_cont);
				break;
			case 618:
				stThres.uDnJitter	= (unsigned int)atoi(ml->msg_body[i].para_cont);
				break;
			case 619:
				stThres.uUpPacketLoss	= (unsigned int)atoi(ml->msg_body[i].para_cont);
				break;
			case 620:
				stThres.uDnPacketLoss	= (unsigned int)atoi(ml->msg_body[i].para_cont);
				break;
			case 479:
				memset(stThres.szDesc, 0x00, MAX_SDESC);
				if( (dSlen = strlen(ml->msg_body[i].para_cont)) > 0)
				{
					if(dSlen > MAX_SDESC)
						strncpy(stThres.szDesc, ml->msg_body[i].para_cont, MAX_SDESC-1);
					else
						strcpy(stThres.szDesc, ml->msg_body[i].para_cont);
				}
				else
					sprintf(stThres.szDesc, "-");
				break;
			default:
				log_print(LOGN_CRI, LH"ERROR IN Parameter para_id[%hu]", LT, ml->msg_body[i].para_id);
		}
	}

	log_print(LOGN_DEBUG, LH"cSvcType[%hu] uTCPSetupTime[%u] uResponseTime[%u] uUpThroughput[%u] uDnThroughput[%u]"
		"uUpRetransCount[%u] uDnRetransCount[%u] uUpJitter[%u] uDnJitter[%u] uUpPacketLoss[%u] uDnPacketLoss[%u] szDesc[%s]", LT,
		stThres.cSvcType, stThres.uTCPSetupTime, stThres.uResponseTime, stThres.uUpThroughput, stThres.uDnThroughput, stThres.uUpRetransCount,
		stThres.uDnRetransCount, stThres.uUpJitter, stThres.uDnJitter, stThres.uUpPacketLoss, stThres.uDnPacketLoss, stThres.szDesc);

	if( (dRet = dChgThres(&stMySQL, &stThres)) < 0)
    {
		log_print(LOGN_CRI, LH"ERROR IN dChgThres() dRet[%d]", LT, dRet);

		dSlen					= strlen(smsg.data) + 1;
		smsg.common.mml_err		= eDBQUERYERROR;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= dSlen;

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -5;
		}
		return -6;
    }
    else
    {
		if( (dRet = dInit_WatchInfoDefectThreshold()) < 0)
			log_print(LOGN_CRI, LH"ERROR IN dInit_WatchInfoDefectThreshold() dRet[%d]", LT, dRet);

        dSlen					= strlen(smsg.data) + 1;
        smsg.common.mml_err		= DBM_SUCCESS;
        smsg.common.cont_flag	= DBM_END;
        smsg.head.msg_len		= dSlen;

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -7;
		}
	}

    return 0;
}

int dis_equip_info(mml_msg *ml, long long llNID)
{
	int					i, j, dRet, dCnt, dIdx, dLoopCnt;
	dbm_msg_t			smsg;
	st_Info_Equip_MMC	stList;
	st_InfoEquip_MMC	stInfoEquip[MAX_EQUIP_INFO];

	dCnt	= 0;
	dIdx	= 0;
	memset(&stList, 0x00, sizeof(st_Info_Equip_MMC));
	memset(&smsg, 0x00, sizeof(dbm_msg_t));

	if( (dRet = dGetEquipInfo(&stMySQL, stInfoEquip, &dCnt)) < 0){

		log_print(LOGN_CRI, LH"ERROR IN dGetEquipInfo() dRet[%d]", LT, dRet);
		set_dbm_ret(&smsg, eDBQUERYERROR, DBM_END, 0);
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0){
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -1;
		}
		return -2;
	}

	if(dCnt >= MAX_EQUIP_INFO){

		sprintf(smsg.data, "OVER MAX TB_MEQUIPMC COUNT[%d]", MAX_EQUIP_INFO);
		set_dbm_ret(&smsg, eOVERMAXROW, DBM_END, strlen(smsg.data)+1);
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0){
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -3;
		}
		return -4;
	}

	if( (dCnt%DEF_EQUIP_MMC) == 0) dLoopCnt	= dCnt/DEF_EQUIP_MMC;
	else						   dLoopCnt	= (dCnt/DEF_EQUIP_MMC)+1;

	if(dCnt == 0){
		memcpy(smsg.data, &stList, sizeof(st_Info_Equip_MMC));
		set_dbm_ret(&smsg, DBM_SUCCESS, DBM_END, sizeof(st_Info_Equip_MMC) );
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0){
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -5;
		}
	}

	for(i = 0; i < dLoopCnt; i++){
		for(j = 0; j < DEF_EQUIP_MMC; j++){
			stList.stInfoEquip[j].uEquipIP      = stInfoEquip[dIdx].uEquipIP;
			stList.stInfoEquip[j].uNetmask		= stInfoEquip[dIdx].uNetmask;
			stList.stInfoEquip[j].cEquipType    = stInfoEquip[dIdx].cEquipType;
			stList.stInfoEquip[j].cMon1MinFlag  = stInfoEquip[dIdx].cMon1MinFlag;

			strcpy(stList.stInfoEquip[j].szEquipTypeName, stInfoEquip[dIdx].szEquipTypeName);
			strcpy(stList.stInfoEquip[j].szEquipName, stInfoEquip[dIdx].szEquipName);
			strcpy(stList.stInfoEquip[j].szDesc, stInfoEquip[dIdx].szDesc);

			log_print(LOGN_DEBUG, "dIdx[%d] uEquipIP[%u] uNetmask[%u] cEquipType[%hu] szEquipTypeName[%s] szEquipName[%s] c1MinFlag[%d] szDesc[%s]",
				dIdx, stList.stInfoEquip[j].uEquipIP, stList.stInfoEquip[i].uNetmask, stList.stInfoEquip[j].cEquipType,
				stList.stInfoEquip[j].szEquipTypeName, stList.stInfoEquip[j].szEquipName, stList.stInfoEquip[j].cMon1MinFlag, 
				stList.stInfoEquip[j].szDesc);
			if(dIdx + 1 == dCnt)
				break;
			dIdx++;
		}
		smsg.common.TotPage = dLoopCnt;
		smsg.common.CurPage = i + 1;

		if(j == DEF_EQUIP_MMC) stList.dCount = DEF_EQUIP_MMC;
		else				   stList.dCount = j + 1;

		memcpy(smsg.data, &stList, sizeof(st_Info_Equip_MMC));

		smsg.common.mml_err	= DBM_SUCCESS;
		if(i == (dLoopCnt-1))
			smsg.common.cont_flag = DBM_END;
		else
			smsg.common.cont_flag = DBM_CONTINUE;

		smsg.head.msg_len = sizeof(st_Info_Equip_MMC);
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -6;
		}
	}

	return 0;
}

int add_equip_info(mml_msg *ml, long long llNID)
{
	int				i, dRet, dCnt;
	unsigned int	uTmp;
	size_t			szDescLen;
	dbm_msg_t		smsg;
	st_Info_Equip	stInfoEquip;

	smsg.data[0]		= 0x00;
	memset(&stInfoEquip, 0x00, sizeof(st_Info_Equip));
	dCnt				= 0;

	for(i = 0; i < ml->num_of_para; i++){
		switch(ml->msg_body[i].para_id){
			case 22:
				if( (dRet = inet_pton(AF_INET, ml->msg_body[i].para_cont, &uTmp)) <= 0){
					log_print(LOGN_CRI, LH"FAILED IN inet_pton(%s) dRet[%d]", LT, ml->msg_body[0].para_cont, dRet);

					set_dbm_ret(&smsg, eINVALID_IP, DBM_END, 0);
					if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0){
						log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
						return -1;
					}
					return -2;
				}
				stInfoEquip.uEquipIP	= ntohl(uTmp);
				break;
			case 65:
				stInfoEquip.uNetmask	= (unsigned int)atoi(ml->msg_body[i].para_cont);
				break;
			case 566:
				stInfoEquip.cEquipType	= (unsigned char)atoi(ml->msg_body[i].para_cont);
				break;
			case 570:
				if( (szDescLen = strlen(ml->msg_body[i].para_cont)) > 0){
					if(szDescLen > DEF_EQUIPNAME)
						strncpy(stInfoEquip.szEquipTypeName, ml->msg_body[i].para_cont, DEF_EQUIPNAME-1);
					else
						strncpy(stInfoEquip.szEquipTypeName, ml->msg_body[i].para_cont, szDescLen);
				}
				break;
			case 571:
				if( (szDescLen = strlen(ml->msg_body[i].para_cont)) > 0){
					if(szDescLen > DEF_EQUIPNAME)
						strncpy(stInfoEquip.szEquipName, ml->msg_body[i].para_cont, DEF_EQUIPNAME-1);
					else
						strncpy(stInfoEquip.szEquipName, ml->msg_body[i].para_cont, szDescLen);
				}
				else
					sprintf(stInfoEquip.szEquipName, "-");
				break;
			case 479:
				if( (szDescLen = strlen(ml->msg_body[i].para_cont)) > 0){
					if(szDescLen > MAX_SDESC)
						strncpy(stInfoEquip.szDesc, ml->msg_body[i].para_cont, MAX_SDESC-1);
					else
						strncpy(stInfoEquip.szDesc, ml->msg_body[i].para_cont, szDescLen);
				}
				else
					sprintf(stInfoEquip.szDesc, " - ");
				break;
			case 800:
				stInfoEquip.cMon1MinFlag = (unsigned char)atoi(ml->msg_body[i].para_cont);
				log_print(LOGN_INFO,"CHECK Monflag=%d", stInfoEquip.cMon1MinFlag);
				break;
			default:
				log_print(LOGN_WARN, LH"INVALID para_id[%hu] para_cont[%s]", LT,
					ml->msg_body[i].para_id, ml->msg_body[i].para_cont);
				break;
		}
	}

	if(!strlen(stInfoEquip.szEquipTypeName)){
		switch(stInfoEquip.cEquipType){
			case SYSTEM_TYPE_PCF:
				sprintf(stInfoEquip.szEquipTypeName, "PCF");
				if(stInfoEquip.uNetmask == 0)
					stInfoEquip.uNetmask = 32;
				break;
			case SYSTEM_TYPE_PDSN:
				sprintf(stInfoEquip.szEquipTypeName, "PDSN");
				if(stInfoEquip.uNetmask == 0)
					stInfoEquip.uNetmask = 32;
				break;
			case SYSTEM_TYPE_AAA:
				sprintf(stInfoEquip.szEquipTypeName, "AAA");
				if(stInfoEquip.uNetmask == 0)
					stInfoEquip.uNetmask = 32;
				break;
			case SYSTEM_TYPE_HSS:
				sprintf(stInfoEquip.szEquipTypeName, "HSS");
				if(stInfoEquip.uNetmask == 0)
					stInfoEquip.uNetmask = 32;
				break;
			case SYSTEM_TYPE_LNS:
				sprintf(stInfoEquip.szEquipTypeName, "LNS");
				break;
			case SYSTEM_TYPE_SERVICE:
				sprintf(stInfoEquip.szEquipTypeName, "CSCF");
				if(stInfoEquip.uNetmask == 0)
					stInfoEquip.uNetmask = 32;
				break;
			case SYSTEM_TYPE_ROAMAAA:
				sprintf(stInfoEquip.szEquipTypeName, "ROAM_AAA");
				break;
			case LAC_SYSTYPE:
				sprintf(stInfoEquip.szEquipTypeName, "LAC");
				break;
			default:
				if(stInfoEquip.cEquipType >= ROAM_JAPAN)
					sprintf(stInfoEquip.szEquipTypeName, "ROAM_NASIP");
				else
					sprintf(stInfoEquip.szEquipTypeName, "-");
				break;
		}
	}

	if(stInfoEquip.uNetmask == 0)
		stInfoEquip.uNetmask = 32;

	log_print(LOGN_CRI, LH"uEquipIP[%u] uNetmask[%u] cEquipType[%d] szEquipTypeName[%s] szEquipName[%s] uc1MonFlag[%d] szDesc[%s]", LT,
		stInfoEquip.uEquipIP, stInfoEquip.uNetmask, stInfoEquip.cEquipType, stInfoEquip.szEquipTypeName, stInfoEquip.szEquipName, stInfoEquip.cMon1MinFlag, stInfoEquip.szDesc);

	if( (dRet = dGetInfoEquipCount(&stMySQL, &dCnt)) < 0){
		log_print(LOGN_CRI, LH"ERROR IN dGetInfoEquipCount() dRet[%d]", LT, dRet);
		set_dbm_ret(&smsg, eDBQUERYERROR, DBM_END, strlen(smsg.data) + 1);
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0){
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -5;
		}
		return -6;
	}

	if(dCnt >= MAX_EQUIP_INFO){
		sprintf(smsg.data, "\n TB_MEQUIPMC COUNT[%d] is OVER MAX_EQUIP_INFO[%d]", dCnt, MAX_EQUIP_INFO);
		set_dbm_ret(&smsg, DBM_SUCCESS, DBM_END, strlen(smsg.data) + 1);
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0){
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -7;
		}
		return -8;
	}

	if( (dRet = dInsertEquip(&stMySQL, &stInfoEquip)) < 0){

		log_print(LOGN_CRI, LH"ERROR IN dInsertEquip() dRet[%d]", LT, dRet);
		if(dRet == -1) set_dbm_ret(&smsg, eAlreadyRegisteredData, DBM_END, 0);
		else           set_dbm_ret(&smsg, eDBQUERYERROR, DBM_END, 0);

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0){
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -9;
		}
		return -10;
	}else{
		set_dbm_ret(&smsg, DBM_SUCCESS, DBM_END, 0 );
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0){
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -11;
		}

		if( (dRet = dInit_WatchFltEquip_Info()) < 0){
			log_print(LOGN_CRI,LH"ERROR IN dInit_WatchFltEquip_Info() dRet[%d]", LT, dRet);
			return -12;
		}
	}

	return 0;
}

int del_equip_info(mml_msg *ml, long long llNID)
{
	int				dRet, dSlen;
	unsigned int	uiIP;
	dbm_msg_t		smsg;

	smsg.data[0]	= 0x00;

	/* CHECK IP */
	if( (dRet = inet_pton(AF_INET, ml->msg_body[0].para_cont, &uiIP)) <= 0)
	{
		log_print(LOGN_CRI, LH"FAILED IN inet_pton(%s)", LT, ml->msg_body[0].para_cont);

		dSlen					= strlen(smsg.data) + 1;
		smsg.common.mml_err		= eINVALID_IP;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= dSlen;
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -1;
		}
		return -2;
	}

	if( (dRet =  dDeleteEquip(&stMySQL, ntohl(uiIP))) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dDeleteEquip(uiIP[%u]) dRet[%d]", LT, uiIP, dRet);
		dSlen					= strlen(smsg.data) + 1;
		smsg.common.mml_err		= eDBQUERYERROR;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= dSlen;
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -3;
		}
		return -4;
	}
	else
	{
		smsg.common.mml_err		= DBM_SUCCESS;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= 0;
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -5;
		}

		if( (dRet = dInit_WatchFltEquip_Info()) < 0)
		{
			log_print(LOGN_CRI,LH"ERROR IN dInit_WatchFltEquip_Info() dRet[%d]", LT, dRet);
			return -6;
		}
	}

	return 0;
}

int dMakeHashCode( char *pszURLBuf, int dLength, char *pszHash )
{
    int         i, dIndex;
    UCHAR       szResultHash[8] = {0,0,0,0,0,0,0,0};

    for( i=0; i<16; i++ )
        pszHash[i] = 0;

    for( dIndex=0; dIndex<(dLength%8==0?dLength:dLength+(8-dLength%8)); dIndex++) {
        if( dIndex>(dLength-1))
            break;
        szResultHash[dIndex%8] += pszURLBuf[dIndex];
    }

    for( dIndex=0; dIndex<8; dIndex++ ) {
        if( dIndex > (dLength-1) ) {
            sprintf( pszHash+dIndex*2, "00");
            continue;
        }
        if( szResultHash[dIndex]<=0xf )
            sprintf( pszHash+dIndex*2, "0%x", szResultHash[dIndex] );
        else
            sprintf( pszHash+dIndex*2, "%2x", szResultHash[dIndex] );
    }

    return 1;
}

int dis_his_cmd(mml_msg *ml, INT64 llNID)
{
	int			dRet, dCnt, dLoopCnt, dIsAdmin, i;
	time_t		tStartTime, tEndTime;
	dbm_msg_t	smsg;
	st_CmdList	stList;

	smsg.data[0]			= 0x00;
	smsg.common.total_cnt	= 1;
	memset(&stList, 0x00, sizeof(st_CmdList));
	if(ml->num_of_para == 0)
	{
		tEndTime	= time(NULL);
		tStartTime	= tEndTime - 3600;
	}
	else
	{
		tStartTime	= tGetTimeFromStr(ml->msg_body[0].para_cont);
		tEndTime	= tGetTimeFromStr(ml->msg_body[1].para_cont);
	}

	if( ((dIsAdmin = dIsAdminID(&stMySQL, ml->adminid)) < 0) || (dIsAdmin > 1))
	{
		smsg.common.mml_err		= eDBQUERYERROR;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= 0;

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -1;
		}
		return -2;
	}

	if( (dRet = dGetCmdHistoryCount(&stMySQL, ml->adminid, tStartTime, tEndTime, dIsAdmin, &dCnt)) < 0)
	{
		smsg.common.mml_err		= eDBQUERYERROR;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= 0;

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -3;
		}
		return -4;
	}

	if(dCnt == 0)
	{
		smsg.common.mml_err		= DBM_SUCCESS;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= 0;
		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -5;
		}
		return -6;
	}
	else
	{
		dLoopCnt = (dCnt/MAX_CMD_CNT)+1;
		if( (dCnt%MAX_CMD_CNT) == 0)
			dLoopCnt--;
	}

	for(i = 0; i < dLoopCnt; i++)
	{
		if( (dRet = dGetCommandList(&stMySQL, ml->adminid, &stList.stCmd[0], tStartTime, tEndTime, dIsAdmin, (i+1))) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dGetCommandList(sUserName[%s], tStartTime[%ld], tEndTime[%ld]) dRet[%d]", LT,
				ml->adminid, tStartTime, tEndTime, dRet);

			smsg.common.mml_err		= eDBQUERYERROR;
			smsg.common.cont_flag	= DBM_END;
			smsg.head.msg_len		= 0;
			if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
			{
				log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
				return -7;
			}
			return -8;
		}
		else
		{
			stList.dCount = dRet;
			if(dLoopCnt == 1)
				smsg.common.TotPage		= dLoopCnt-1;
			else
				smsg.common.TotPage		= dLoopCnt;

			smsg.common.CurPage		= i+1;

			memcpy(smsg.data, &stList, sizeof(st_CmdList));
			smsg.common.mml_err = DBM_SUCCESS;
			if(i == dLoopCnt - 1)
				smsg.common.cont_flag = DBM_END;
			else
				smsg.common.cont_flag = DBM_CONTINUE;

			smsg.head.msg_len = sizeof(st_CmdList);
			if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
			{
				log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
				return -9;
			}
		}
	}

	return 0;
}

int add_monthrs_info(mml_msg *ml, long long llNID)
{
	int				i, dRet;
	size_t			szLength;
	unsigned char	cEndHour;
	st_MON_ThresMMC	stMONThres;
	dbm_msg_t		smsg;

	smsg.data[0]	= 0x00;
	memset(&stMONThres, 0x00, sizeof(st_MON_ThresMMC));
	for(i = 0; i < ml->num_of_para; i++)
	{
		switch(ml->msg_body[i].para_id)
		{
			case 605:
				stMONThres.huBranchID	= (unsigned short)atoi(ml->msg_body[i].para_cont);
				break;
			case 606:
				stMONThres.cSvcType		= (unsigned char)atoi(ml->msg_body[i].para_cont);
				break;
			case 607:
				stMONThres.cAlarmType	= (unsigned char)atoi(ml->msg_body[i].para_cont);
				break;
				// dcham 20110616
			case 622:
				if( (dRet = inet_pton(AF_INET, ml->msg_body[i].para_cont, &stMONThres.dSvcIP)) <= 0)
				{
					log_print(LOGN_CRI, LH"FAILED IN inet_pton(%s) dRet[%d]", LT,ml->msg_body[i].para_cont, dRet);
					return eINVALID_IP;
				}
				stMONThres.dSvcIP       = ntohl(stMONThres.dSvcIP);
				break;
			case 703:
				stMONThres.cStartHour       = (unsigned char)atoi(ml->msg_body[i].para_cont);
				break;  
			case 704:
				cEndHour                    = (unsigned int)atoi(ml->msg_body[i].para_cont);
				break;  
			case 426:
				stMONThres.huDayRate        = (unsigned short)atoi(ml->msg_body[i].para_cont);
				break;  
			case 427:
				stMONThres.huNightRate      = (unsigned short)atoi(ml->msg_body[i].para_cont);
				break;  
			case 428:
				stMONThres.uDayMinTrial     = (unsigned int)atoi(ml->msg_body[i].para_cont);
				break;  
			case 429:
				stMONThres.uNigthMinTrial   = (unsigned int)atoi(ml->msg_body[i].para_cont);
				break;  
			case 430:
				stMONThres.uPeakTrial       = (unsigned int)atoi(ml->msg_body[i].para_cont);
				break;  
			case 479:
				if( (szLength = strlen(ml->msg_body[i].para_cont)) > 0)
				{       
					if(szLength > MAX_SDESC)
						strncpy(stMONThres.szDesc, ml->msg_body[i].para_cont, MAX_SDESC-1);
					else    
						strcpy(stMONThres.szDesc, ml->msg_body[i].para_cont);
				}       
				else    
					sprintf(stMONThres.szDesc, "-");
				break;  
			default:
				log_print(LOGN_CRI, LH"ERROR IN Parameter para_id[%hu]", LT, ml->msg_body[i].para_id);
		}
	}

	if( (stMONThres.huBranchID>MAX_MON_OFFICE_IDX) || (stMONThres.cSvcType>MAX_SYSTEM_TYPE_IDX+1) || (stMONThres.cAlarmType>MAX_ALARMTYPE_IDX))
	{
		szLength				= strlen(smsg.data) + 1;
		smsg.common.mml_err		= eBadParameter;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= szLength;

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -3;
		}
		return -4;
	}

	if(stMONThres.cStartHour > cEndHour)
	{
		szLength				= strlen(smsg.data) + 1;
		smsg.common.mml_err		= eBadParameter;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= szLength;

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -5;
		}
		return -6;
	}
	else
		stMONThres.cDayRange = cEndHour - stMONThres.cStartHour;

	if( (dRet = dInsertMONThres(&stMySQL, &stMONThres)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dInsertMONThres() dRet[%d]", LT, dRet);

		szLength				= strlen(smsg.data) + 1;
		smsg.common.mml_err		= eDBQUERYERROR;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= szLength;

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -7;
		}
		return -8;
	}
	else
	{
		if( (dRet = dInit_WatchInfoMonThreshold()) < 0)
			log_print(LOGN_CRI, LH"ERROR IN dInit_WatchInfoMonThreshold() dRet[%d]", LT, dRet);

		szLength				= strlen(smsg.data) + 1;
		smsg.common.mml_err		= DBM_SUCCESS;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= szLength;

		if( (dRet = dSendMMC(ml, &smsg, llNID)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMMC() dRet[%d]", LT, dRet);
			return -9;
		}
	}
	return 0;
}
