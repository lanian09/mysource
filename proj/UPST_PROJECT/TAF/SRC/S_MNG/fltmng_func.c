/** A.1 * File Include *************************************************************/
#include <fcntl.h>				/* O_CREAT, O_WRONLY, O_TRUNC ... */
#include <mysql/mysql.h>		/* MYSQL			*/
#include <dirent.h>				/* DIR */
#include <unistd.h>				/* usleep(3), CLOSE(2) */
#include <sys/stat.h>			/* mkdir(2) */

#include "loglib.h"				/* log_print(), LOGN_ level */
#include "utillib.h"			/* util_makenid() */
#include "filelib.h"			/* write_file() */
#include "dblib.h"

#include "filter.h"				/* st_Flt_Info */
#include "define.h"				/* MAX_MNIP_COUNT - 2000 */
#include "filedb.h"				/* st_keepalive */
#include "sockio.h"				/* NTAFT_HEADER_LEN */
#include "db_define.h"			/* ALM_CNT,  ... */

#include "path.h"				/* DATA_PATH */
#include "commdef.h"			/* FILE_... ext */
#include "procid.h"
#include "sshmid.h"
#include "msgdef.h"				/* st_Msg */
#include "mmcdef.h"				/* MID_*, SID_* */
#include "mems.h"				/* stMEMSINFO */
#include "cifo.h"				/* stCIFO */
#include "nifo.h"				/* nifo_XXX() */
#include "gifo.h"				/* gifo_XXX() */

#include <common_stg.h>

#include "fltmng_func.h"
#include "fltmng_file.h"		/* Search_Service_Conf() */
#include "fltmng_log.h"			/* print_FltCommon(), print_AlmLevel() */

/** B.1 * Definition of New Constants **********************************************/

/** B.1 * Definition of New Type ***************************************************/

/** C.1 * Declaration of Variables *************************************************/
pst_keepalive_taf 		keepalive;
st_Flt_Info				*flt_info;
st_TraceList			*trace_tbl;
OFFSET					gOffset;
int						gdStopFlag;

/** C.2 * Declaration of External Variables ****************************************/
extern stMEMSINFO	    *gpMEMSINFO;
extern stCIFO			*gpCIFO;

extern time_t			g_tUpdate;
extern time_t			g_tInterval;

/** D.1 * Definition of Functions **************************************************/
/***********************************************************************************/
int dGetNode(U8 **ppNODE, pst_MsgQ *ppstMsgQ)
{
	*ppNODE   = NULL;
	*ppstMsgQ = NULL;

	if( (*ppNODE = nifo_node_alloc(gpMEMSINFO)) == NULL ){
		log_print(LOGN_WARN, LH"FAILED IN nifo_node_alloc"EH, LT, ET);
		return -1;
	}

	if( (*ppstMsgQ = (pst_MsgQ)nifo_tlv_alloc(gpMEMSINFO, *ppNODE, DEF_MSGQ_NUM, DEF_MSGQ_SIZE, DEF_MEMSET_OFF)) == NULL ){
		log_print(LOGN_WARN, LH"FAILED IN nifo_tlv_alloc, return NULL", LT);
		nifo_node_delete(gpMEMSINFO, *ppNODE);
		return -2;
	}

	return 0;
}

int dMsgsnd(int procID, OFFSET offset)
{
	if( gifo_write( gpMEMSINFO, gpCIFO, SEQ_PROC_S_MNG, procID, offset ) < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN gifo_write(SEQ_PROC_S_MNG:%d > TARGET:%d), offset=%ld"EH,
			LT, SEQ_PROC_S_MNG, procID, offset, ET);
		nifo_node_delete(gpMEMSINFO, nifo_ptr( gpMEMSINFO, offset ));
		usleep(0);
		return -1;
	}

	g_tUpdate	= time(0);
	g_tInterval	= FLT_SHORT_CHECK;
	log_print(LOGN_INFO, LH"SND S_MNG:%d > TARGET:%d, offset=%ld", LT, SEQ_PROC_S_MNG, procID, offset);
	return 0;
}

void do_while(void)
{
	int			dRet, dMsgflag;
	time_t		tCurr;
	pst_MsgQ	pstMsg;

	dMsgflag	= 0;

	while(!gdStopFlag)
	{
		if( (dRet = dIsRcvedMessage(&pstMsg)) < 0)
			usleep(1);
		else if(dRet == 0)
		{
#ifdef REGENERATE_FLT	//	HJPARK(2008.07.01): If there is no filter information, msgrcv() return immediately.
			tCurr = time(NULL);
			if( (tCurr - g_tUpdate) > g_tInterval)	// g_tInterval = FLT_LONG_CHECK(300 seconds)
			{
				if(!flt_info->stSvcInfoShm.dCount || !flt_info->stNASShm.dCount)
				{
					log_print(LOGN_INFO, "%s: MAKE & SEND REQUEST. INTERVAL[%ld]", __FUNCTION__, g_tInterval);

					if(!flt_info->stSvcInfoShm.dCount)
						log_print(LOGN_INFO, "%s: flt_info->stSvcInfoShm.dCount[%d]", __FUNCTION__, flt_info->stSvcInfoShm.dCount);

					if(!flt_info->stNASShm.dCount)
						log_print(LOGN_INFO, "%s: flt_info->stNASShm.dCount[%d]", __FUNCTION__, flt_info->stNASShm.dCount);

					if( (dRet = dMakeSendMsg(pstMsg)) < 0)
						continue;

					dMsgsnd(SEQ_PROC_CI_SVC, gOffset);
				}
			}
#else
			if(dMsgflag == 0 )
			{
				/* 메시지 큐에 메시지가 없음 */
				tCurr = time(NULL);
				if( (tCurr-g_tUpdate) > g_tInterval)
				{
					log_print(LOGN_INFO, "%s: MAKE & SEND REQUEST. INTERVAL[%ld]", __FUNCTION__, g_tInterval);
					if( (dRet = dMakeSendMsg(pstMsg)) < 0)
						continue;

					dMsgsnd(SEQ_PROC_CI_SVC, gOffset);
				}
			}
#endif	// END: #ifdef REGENERATE_FLT
		}
		else if(dRet == (pstMsg->usBodyLen+DEF_MSGHEAD_LEN-sizeof(long)))
		{
			log_print(LOGN_INFO, "%s: MSG received.", __FUNCTION__);
			dProc_Msg(pstMsg);
			dMsgflag = 1;
			continue;
		}
		else
		{
			log_print(LOGN_CRI, "%s: FAIL[dRcvMQ] INVAILD msg size dRet[%d] usBodyLen+HEADLEN[%lu]", 
				__FUNCTION__, dRet, (pstMsg->usBodyLen+DEF_MSGHEAD_LEN));
			continue;
		}
#ifdef REGENERATE_FLT	// HJPARK(2008.07.01): If there is no filter information, msgrcv() return immediately.
		sleep(5);
#endif	// END: #ifdef REGENERATE_FLT
	}
}

int dIsRcvedMessage( pst_MsgQ *ppstMsg )
{
	gOffset = 0;

	if( (gOffset = gifo_read( gpMEMSINFO, gpCIFO, SEQ_PROC_S_MNG )) > 0 ){
		if( (*ppstMsg = (pst_MsgQ)nifo_get_value( gpMEMSINFO, DEF_MSGQ_NUM, gOffset )) != NULL ){
			return 0;
		} 

		log_print(LOGN_CRI, LH"FAILED IN nifo_get_value(st_MsgQ=%d), gOffset=%ld", LT, DEF_MSGQ_NUM, gOffset);
		return -1;
	}
	
	usleep(0);
	return -1;
}

int dProc_Msg(st_MsgQ *pstMsg)
{
	mml_msg 		*ml;
	pst_MsgQSub 	pstMsgQSub;
	int				dRet;

	pstMsgQSub = (pst_MsgQSub)&pstMsg->llMType;

	log_print(LOGN_INFO, "dProc_Msg(): ID S [%d, %d ,%d]", pstMsgQSub->usType,
							pstMsgQSub->usSvcID, pstMsgQSub->usMsgID );

	if( pstMsgQSub->usType == DEF_SYS )
	{
		if(pstMsgQSub->usSvcID == SID_FLT)
		{
			switch( pstMsgQSub->usMsgID )
			{
				case MID_FLT_TMF:
					break;

				case MID_FLT_SVC:
					log_print(LOGN_INFO, "dProc_Msg(): [FLT SVC] INFORMATION RECV FROM NTAM ");
					Apply_Filter_SvcInfo( (st_ConnInfo*)pstMsg->szBody, pstMsgQSub->usMsgID);
					break;

				case MID_FLT_ALM:
					Apply_Filter_Alm((st_AlmLevel_List *)pstMsg->szBody);

					log_print(LOGN_INFO, "dProc_Msg(): BEFORE MSG[Alarm Level]");
					print_AlmLevel((st_AlmLevel_List *)pstMsg->szBody);
					log_print(LOGN_INFO, "dProc_Msg(): AFTER MSG[Alarm Level]");
					break;

				case MID_FLT_COMMON:
					Apply_Filter_Common((st_Flt_Common *)pstMsg->szBody);

					log_print(LOGN_INFO, "dProc_Msg(): BEFORE MSG[FLT_COMMON]");
					print_FltCommon((st_Flt_Common *)pstMsg->szBody);
					log_print(LOGN_INFO, "dProc_Msg(): AFTER MSG[FLT_COMMON]");
					break;

				case MID_FLT_MNIP:
					log_print(LOGN_INFO, "dProc_Msg(): [IP POOL] INFORMATION RECV FROM NTAM ");
					Apply_Filter_NAS( (st_ConnInfo*)pstMsg->szBody, pstMsgQSub->usMsgID);
					break;

				case MID_FLT_SCTP:
					log_print(LOGN_INFO, "dProc_Msg(): [SCTP] INFORMATION RECV FROM NTAM ");
					Apply_Filter_SCTP( (st_ConnInfo*)pstMsg->szBody, pstMsgQSub->usMsgID);
					break;

				case MID_FLT_LOG:
					Apply_Filter_LOG((unsigned char*)pstMsg->szBody);
					break;

				case MID_FLT_TCP:
					Apply_Filter_TCP((st_Conf *)pstMsg->szBody);
					break;

				case MID_FLT_TRC:
					Apply_Filter_TRC((st_TraceList*)pstMsg->szBody);
					break;

				case MID_FLT_TIMER:
					if( (dRet = Apply_Filter_TIMER((TIMER_INFO*)pstMsg->szBody)) < 0)
						log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN Apply_Filter_TIMER() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
					break;

				case MID_FLT_ONOFF:
					log_print(LOGN_DEBUG, "[COMMANDLIST] GET MSG SVC ONOFF SET");

					if( (dRet = Apply_Filter_SVC_ONOFF((SVC_CONF *)pstMsg->szBody)) < 0)
					{
						log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN Apply_Filter_SVC_ONOFF() dRet[%d]", __FILE__, __FUNCTION__, __LINE__
									   , dRet);
					}
					else
					{
						/* SEND TO PRE_A */
						if( (dRet = dSendPREA(pstMsgQSub->usMsgID)) < 0)
						{
							log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN dSendPREA() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
							return -2;
						}
					}

					break;

				default:
					log_print(LOGN_CRI, "dProc_Msg(): FAIL[MsgID = %hu] INVALID MsgID.",
						pstMsgQSub->usMsgID);
					return -1;
			}
            dSetFltInfoToFile(flt_info);

			g_tUpdate = time(0);
			g_tInterval = FLT_LONG_CHECK;
			log_print(LOGN_INFO, "dProc_Msg(): SET INTERVAL = [%ld]", g_tInterval);
		}
		else if(pstMsgQSub->usSvcID == SID_CHKREQ)
		{
			switch( pstMsgQSub->usMsgID )
			{
				case MID_FLT_LOG:
				case MID_FLT_SYS:
				case MID_FLT_TCP:
				case MID_DIS_NTAF_CONF:
					Send_mmc(pstMsgQSub->usMsgID, pstMsg->llIndex);
					break;

				default:
					log_print(LOGN_CRI, "dProc_Msg(): FAIL[MsgID = %hu] INVALID MsgID.",
						pstMsgQSub->usMsgID);
					return -1;
			}
		}
		else if(pstMsgQSub->usSvcID == SID_MML)
		{
			ml = (mml_msg *)pstMsg->szBody;
            log_print(LOGN_INFO, "MML MSG GET MID[%d]", ml->msg_id);

			switch( ml->msg_id )
			{
				case MI_DIS_SUB_SESS:
                    dRet = dis_ntaf_sess(ml);
                    if(dRet < 0)
					{
                        log_print(LOGN_CRI,"MMC : Failed in dis_ntaf_sess");
                        return -1;
                    }
                    break;

                case MI_SUB_TIMER:
                    dRet = dis_ntaf_timer(ml);
                    if(dRet < 0)
					{
                        log_print(LOGN_CRI,"MMC : Failed in dis_ntaf_timer");
                        return -1;
                    }
                    break;
				default:
					log_print(LOGN_CRI, "dProc_Msg(): FAIL, INVALID MML MSG GET MID=%d", ml->msg_id);
                    return -1;
			}
		}
		else
		{
			log_print(LOGN_CRI, "dProc_Msg(): FAIL[ucSvcID=%u] INVALID ucSvcID.",
				pstMsgQSub->usSvcID);
			return -1;
		}
	}

	else
	{
		log_print(LOGN_CRI, "dProc_Msg(): FAIL[Type = %u] INVALID Type", pstMsgQSub->usType);
		return -1;
	}

	return 0;
}


int dMakeSendMsg(st_MsgQ *pstMsg)
{
	pst_MsgQSub	pstMsgQSub;

	util_makenid(SEQ_PROC_S_MNG, &pstMsg->llNID);

	pstMsgQSub			= (pst_MsgQSub)&pstMsg->llMType;
	pstMsgQSub->usType	= DEF_SYS;
	pstMsgQSub->usSvcID	= SID_FLT;
	pstMsgQSub->usMsgID	= MID_FLT_ALL;

	pstMsg->ucNTAFID	= flt_info->stTmfInfo.usTmfID;
	pstMsg->llIndex		= 0;
	pstMsg->usBodyLen	= NTAFT_HEADER_LEN;
	pstMsg->usRetCode	= 0;
	pstMsg->ucProID		= SEQ_PROC_S_MNG;
	pstMsg->dMsgQID		= 0;

	return 0;
}


void Apply_Filter_LOG(unsigned char *szBuf)
{
	int		dRet;
	st_Conf	*pstConf;

	pstConf = (st_Conf*)&szBuf[0];

	if( (dRet = dLogWrite(MAX_SW_COUNT, pstConf->usLogLevel)) < 0)
	{
		log_print(LOGN_DEBUG, "F=%s:%s.%d: ERROR IN dLogWrite() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		return;
	}

	if( (dRet = dInitLogLvl()) < 0)
		log_print(LOGN_DEBUG, "F=%s:%s.%d: ERROR IN dInitLogLvl() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
}

void Apply_Filter_TCP(pst_Conf pstConf)
{
	flt_info->stHdrLogShm.sHdrCapFlag = pstConf->usTcpHdr;
	log_print(LOGN_DEBUG, "F=%s:%s.%d: flt_info->stHdrLogShm.sHdrCapFlag [%d]", __FILE__, __FUNCTION__, __LINE__, flt_info->stHdrLogShm.sHdrCapFlag);
}

void Apply_Filter_Tmf(st_Tmf_Info *pstTmfInfo)
{
	if( flt_info->stTmfInfo.usTmfID != pstTmfInfo->usTmfID )
		flt_info->stTmfInfo.usTmfID = pstTmfInfo->usTmfID;
}

void Apply_Filter_SvcInfo(st_ConnInfo *pstConnInfo, unsigned short uhMsgID)
{
	int				dRet;
	MYSQL			stMySQL;
	st_SvcInfo_Shm	stSVRInfo;

	if( (dRet = dConnectMySQL(&stMySQL, pstConnInfo)) < 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN dConnectMySQL() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		return;
	}

	if( (dRet = dSelectSVRIP(&stMySQL, &stSVRInfo)) < 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN dSelectSVRIP() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		db_disconn(&stMySQL);
		return;
	}

	db_disconn(&stMySQL);

	if( (dRet = dWriteSVRIPFile(&stSVRInfo)) < 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN dWriteSVRIPFile() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		return;
	}

	if( (dRet = dSendPREA(uhMsgID)) < 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN dSendPREA() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		return;
	}

	memcpy(&flt_info->stSvcInfoShm, &stSVRInfo, sizeof(st_SvcInfo_Shm));
}


void Apply_Filter_Alm(st_AlmLevel_List *pstAlmLevelList)
{
	if(pstAlmLevelList->dCount != ALM_CNT)
	{
		log_print(LOGN_CRI, "WRONG ALARM COUNT RECEIVED, [COUNT]:[%d]", pstAlmLevelList->dCount);
		return ;
	}

	keepalive->cpu.minor		= pstAlmLevelList->stAlmLevel[CPU_LOC].sMinorLevel;
	keepalive->cpu.major		= pstAlmLevelList->stAlmLevel[CPU_LOC].sMajorLevel;
	keepalive->cpu.critical		= pstAlmLevelList->stAlmLevel[CPU_LOC].sCriticalLevel;

	keepalive->mem.minor		= pstAlmLevelList->stAlmLevel[MEM_LOC].sMinorLevel;
	keepalive->mem.major		= pstAlmLevelList->stAlmLevel[MEM_LOC].sMajorLevel;
	keepalive->mem.critical		= pstAlmLevelList->stAlmLevel[MEM_LOC].sCriticalLevel;

	keepalive->disk.minor		= pstAlmLevelList->stAlmLevel[DISK_LOC].sMinorLevel;
	keepalive->disk.major		= pstAlmLevelList->stAlmLevel[DISK_LOC].sMajorLevel;
	keepalive->disk.critical	= pstAlmLevelList->stAlmLevel[DISK_LOC].sCriticalLevel;

	keepalive->queue.minor		= pstAlmLevelList->stAlmLevel[QUE_LOC].sMinorLevel;
	keepalive->queue.major		= pstAlmLevelList->stAlmLevel[QUE_LOC].sMajorLevel;
	keepalive->queue.critical	= pstAlmLevelList->stAlmLevel[QUE_LOC].sCriticalLevel;

	keepalive->nifo.minor		= pstAlmLevelList->stAlmLevel[NIFO_LOC].sMinorLevel;
	keepalive->nifo.major		= pstAlmLevelList->stAlmLevel[NIFO_LOC].sMajorLevel;
	keepalive->nifo.critical	= pstAlmLevelList->stAlmLevel[NIFO_LOC].sCriticalLevel;

	keepalive->stBytes.minor	= pstAlmLevelList->stAlmLevel[TRAFFIC_LOC].sMinorLevel;
	keepalive->stBytes.major	= pstAlmLevelList->stAlmLevel[TRAFFIC_LOC].sMajorLevel;
	keepalive->stBytes.critical	= pstAlmLevelList->stAlmLevel[TRAFFIC_LOC].sCriticalLevel;
}

void Apply_Filter_Common(st_Flt_Common *pstFltCommon)
{
	flt_info->stFltCommon.usCheckInterval	= pstFltCommon->usCheckInterval;
	flt_info->stFltCommon.usRepeatCnt		= pstFltCommon->usRepeatCnt;
	flt_info->stFltCommon.usTCPLongLast		= pstFltCommon->usTCPLongLast;
	flt_info->stFltCommon.bOnOffState		= pstFltCommon->bOnOffState;
}


void Apply_Filter_NAS(st_ConnInfo *pstConnInfo, unsigned short uhMsgID)
{
	int			dRet;
	MYSQL		stMySQL;
	st_NAS_Shm	stNASInfo;

	if( (dRet = dConnectMySQL(&stMySQL, pstConnInfo)) < 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN dConnectMySQL() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		return;
	}

	if( (dRet = dSelectMNIP(&stMySQL, &stNASInfo)) < 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN dSelectMNIP() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		db_disconn(&stMySQL);
		return;
	}

	db_disconn(&stMySQL);

	if( (dRet = dWriteMNIPFile(&stNASInfo)) < 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN dWriteMNIPFile() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		return;
	}

	if( (dRet = dSendPREA(uhMsgID)) < 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN dSendPREA() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		return;
	}

	memcpy(&flt_info->stNASShm, &stNASInfo, sizeof(st_NAS_Shm));
}

void Apply_Filter_SCTP(st_ConnInfo *pstConnInfo, unsigned short uhMsgID)
{
	int			dRet;
	MYSQL		stMySQL;
	st_SCTP_Shm	stSCTPInfo;

	if( (dRet = dConnectMySQL(&stMySQL, pstConnInfo)) < 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN dConnectMySQL() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		return;
	}

	if( (dRet = dSelectSCTP(&stMySQL, &stSCTPInfo)) < 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN dSelectSCTP() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		db_disconn(&stMySQL);
		return;
	}

	db_disconn(&stMySQL);

	if( (dRet = dWriteSCTPFile(&stSCTPInfo)) < 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN dWriteSCTPFile() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		return;
	}

	if( (dRet = dSendPREA(uhMsgID)) < 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN dSendPREA() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		return;
	}

	memcpy(&flt_info->stSCTPShm, &stSCTPInfo, sizeof(st_SCTP_Shm));
}


int dSetFltInfoToFile(st_Flt_Info *pstFlt)
{
	if( write_file( FILE_FLT_INFO, (char*)pstFlt, sizeof(st_Flt_Info), 0 ) < 0 ){
		log_print(LOGN_CRI,LH"FAILED IN write_file(%s)",LT,FILE_FLT_INFO);
		return -1;
	}
	return 0;

}

int dInitSysConfig(void)
{
	FILE	*fp;
	char	szBuf[1024], szType[64], szTmp[64], szInfo[64];
	int		i = 0;

	if( (fp = fopen(FILE_SYS_CONFIG, "r")) == NULL)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN fopen(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, FILE_SYS_CONFIG, errno, strerror(errno));
		return -1;
	}

	i	= 0;
	while(fgets(szBuf, 1024, fp) != NULL)
	{
		if(szBuf[0] != '#')
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN FILE[%s] LINE[%d] FORMAT", __FILE__, __FUNCTION__, __LINE__, FILE_SYS_CONFIG, i);
			fclose(fp);
			return -2;
		}

		if(szBuf[1] == '#')
			continue;
		else if(szBuf[1] == 'E')
			break;
		else if(szBuf[1] == '@')
		{
			if(sscanf(&szBuf[2], "%s %s %s", szType, szTmp, szInfo) == 3)
			{
				if(strcmp(szType,"SYS") == 0)
				{
					if(strcmp(szTmp,"NO") == 0)
						flt_info->stTmfInfo.usTmfID = atoi(szInfo);
					else if(strcmp(szTmp, "TYPE") == 0)
					{
						memcpy(flt_info->stTmfInfo.szSysType, szInfo, strlen(szInfo));
						flt_info->stTmfInfo.szSysType[strlen(szInfo)]	= 0x00;
						log_print(LOGN_INFO, "GET SYSTEM TYPE[%s]", flt_info->stTmfInfo.szSysType);
					}
				}
			}
		}
		i++;
	}
	fclose(fp);

	return i;
} /* end of dInitSysConfig */


int dInitLogLvl(void)
{
	FILE	*fp;
	char	szBuf[1024], szType1[64];
	int		i, dCount, dLogLvl;

	if( (fp = fopen(FILE_LOG_LEVEL, "r")) == NULL)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN fopen(%s) [errno:%d-%s]", 
			__FILE__, __FUNCTION__, __LINE__, FILE_LOG_LEVEL, errno, strerror(errno));
		return -1;
	}

	i	= 0;
	while(fgets(szBuf, 1024, fp) != NULL)
	{
		if(szBuf[0] != '#')
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN FILE[%s] LINE[%d] FORMAT", 
				__FILE__, __FUNCTION__, __LINE__, FILE_LOG_LEVEL, i);
			fclose(fp);
			return -2;
		}

		if(szBuf[1] == '#')
			continue;
		else if(szBuf[1] == 'E')
			break;
		else if(szBuf[1] == '@')
		{
			if(sscanf(&szBuf[2],"%s %d %d",szType1, &dCount, &dLogLvl) == 3)
			{
				if(strcmp(szType1,"LOGLVL") == 0)
				{
					log_print(LOGN_DEBUG, "%s: Read Data[%s]", __FUNCTION__, &szBuf[2]);
					if( (dCount>MAX_SW_COUNT)||(dCount<0))
					{
						log_print(LOGN_CRI, "F=%s:%s.%d INVALID COUNT=%d", __FILE__, __FUNCTION__, __LINE__, dCount);
						dCount = MAX_SW_COUNT;
					}

					flt_info->stLogLevelList.dCount	= dCount;
					for(i = 0; i < flt_info->stLogLevelList.dCount; i++)
					{
						flt_info->stLogLevelList.stLogLevel.usLogLevel[i]  = dLogLvl;
						log_print(LOGN_INFO,"%s: Read LogLevel count[%d] usLogLevel[%d]",
							__FUNCTION__, flt_info->stLogLevelList.dCount, flt_info->stLogLevelList.stLogLevel.usLogLevel[i]);
					}
				}
			}
		}
	}
	fclose(fp);

	return i;
}


int dSendPREA(unsigned short uhMsgID)
{
	int				dRet;
	U8				*pNode;
	NOTIFY_SIG		*pNOTIFY;

	if( (pNode = nifo_node_alloc(gpMEMSINFO)) == NULL)
	{
		log_print(LOGN_CRI, LH"FAILED IN nifo_node_alloc()"EH, LT,ET);
		return -1;
	}

	pNOTIFY = (NOTIFY_SIG*)nifo_tlv_alloc(gpMEMSINFO, pNode, NOTIFY_SIG_DEF_NUM, sizeof(NOTIFY_SIG), DEF_MEMSET_ON);
	if( pNOTIFY  == NULL) {
		log_print(LOGN_CRI, LH"FAILED IN nifo_tlv_alloc()", LT);
		return -2;
	}

	pNOTIFY->uiEventTime	= time(NULL);
	pNOTIFY->uiType			= (unsigned int)uhMsgID;

	if( (dRet = dMsgsnd(SEQ_PROC_PRE_A, nifo_offset(gpMEMSINFO, pNode))) < 0 ){
		log_print(LOGN_CRI, LH"ERROR IN dMsgsnd() dRet=%d", LT, dRet);
		return -3;
	}

	return 0;
}

int dLogWrite(int dCount, int dLogLvl)
{
	int		i, dRet;
	char	sDataPath[PATH_MAX], sFullFileName[PATH_MAX];
	FILE	*fp;
	DIR		*dirp;

	memset(sDataPath, 0x00, PATH_MAX);
	memset(sFullFileName, 0x00, PATH_MAX);

	sprintf(sDataPath, "%s", DATA_PATH);
	log_print(LOGN_INFO, "F=%s:%s.%d: dLogWrite sDataPath[%s]", __FILE__, __FUNCTION__, __LINE__, sDataPath);

	sprintf(sFullFileName, "%s", FILE_LOG_LEVEL);
	log_print(LOGN_INFO, "F=%s:%s.%d: dLogWrite sFullFileName[%s]", __FILE__, __FUNCTION__, __LINE__, sFullFileName);

	if( (dirp = opendir(sDataPath)) == (DIR*)NULL)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN opendir(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, sDataPath, errno, strerror(errno));
		mkdir(sDataPath, 0777);
	}
	else
		closedir(dirp);

	if( (fp = fopen(sFullFileName, "w")) == NULL)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN fopen(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, sFullFileName, errno, strerror(errno));
		return -1;
	}

	fprintf(fp, "## LOGLVL ## Process Count ## LEVEL \n#@ LOGLVL %d %d \n##\n#E \n", dCount, dLogLvl);

	/*	fflush가 실패할 경우, 최소 100회 반복 처리	*/
	for(i = 0; ((i<100) && ((dRet = fflush(fp))!=0)); i++)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN fflush(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, sFullFileName, errno, strerror(errno));
		return -2;
	}

	if( (dRet = fclose(fp)) != 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN fclose(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, sFullFileName, errno, strerror(errno));
		return -3;
	}

	return 0;
}

int dis_ntaf_sess(mml_msg *ml)
{
	pst_MsgQ 	pstSndMsg;
	pst_MsgQSub	pstMsgQSub;
    dbm_msg_t   smsg;
    char    	szBuf[MSG_DATA_LEN];
	U8			*pNODE;

	if( dGetNode(&pNODE, &pstSndMsg) < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN dGetNode(S_MNG)", LT);
		return -1;
	}

	memset(szBuf, 0x00, MSG_DATA_LEN);
	smsg.data[0] = 0x00;

	if(strcmp(flt_info->stTmfInfo.szSysType, "GN") == 0)
	{
    	sprintf(szBuf, "\n GTP_SESS\t =  %5d CNT(MAX : %d)\n", flt_info->stSessCntInfo.dGtpSessCnt, 100000);
	}
	else if(strcmp(flt_info->stTmfInfo.szSysType, "GI") == 0)
	{
		sprintf(smsg.data, "\n TCP_SESS\t =  %5d CNT(MAX : %d)", flt_info->stSessCntInfo.dTcpSessCnt, 100000);
		sprintf(szBuf, "\n HTTP_SESS\t =  %5d CNT(MAX : %d)", flt_info->stSessCntInfo.dHttpSessCnt, 100000);
    	strcat(smsg.data,szBuf);

    	sprintf(szBuf, "\n VOD_SESS\t =  %5d CNT(MAX : %d)", flt_info->stSessCntInfo.dVodSessCnt, 100000);
    	strcat(smsg.data,szBuf);

    	sprintf(szBuf, "\n MEDIA_SESS\t =  %5d CNT(MAX : %d)", flt_info->stSessCntInfo.dVodMedSessCnt, 100000);
    	strcat(smsg.data,szBuf);

    	sprintf(szBuf, "\n PAGE_SESS\t =  %5d CNT(MAX : %d)\n", flt_info->stSessCntInfo.dPageSessCnt, 100000);
	}

    strcat(smsg.data,szBuf);


    smsg.common.mml_err = DBM_SUCCESS;
    smsg.common.cont_flag = DBM_END;
    smsg.head.msg_len = strlen(smsg.data) +1;

	smsg.head.src_proc = SEQ_PROC_CHSMD;
	smsg.head.dst_func = ml->src_func;
	smsg.head.dst_proc = ml->src_proc;
	smsg.head.cmd_id = ml->cmd_id;
	smsg.head.msg_id = ml->msg_id;

	pstMsgQSub = (pst_MsgQSub)&pstSndMsg->llMType;
	pstMsgQSub->usType = DEF_SYS;
	pstMsgQSub->usSvcID = SID_MML;
	pstMsgQSub->usMsgID = MID_MML_RST;

	pstSndMsg->llNID = SEQ_PROC_CHSMD;
	pstSndMsg->llIndex = 1;
	pstSndMsg->usRetCode = 0;
	pstSndMsg->ucNTAFID = flt_info->stTmfInfo.usTmfID;

	pstSndMsg->usBodyLen = sizeof(dbm_msg_t)-MSG_DATA_LEN+smsg.head.msg_len + NTAFT_HEADER_LEN;
	memcpy( &pstSndMsg->szBody[0], &smsg, pstSndMsg->usBodyLen );

	if( dMsgsnd(SEQ_PROC_CI_SVC, nifo_offset(gpMEMSINFO, pNODE)) < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN dMsgsnd(to=CI_SVC:%d)"EH,LT,SEQ_PROC_CI_SVC,ET);
		return -2;
	}

	log_print(LOGN_DEBUG, LH" * SUCCESS SEND TO CI_SVC=%d, MSG [%s]",LT, SEQ_PROC_CI_SVC, smsg.data);
    return 1;
}

int dis_ntaf_timer(mml_msg *ml)
{
    char    	szBuf[MSG_DATA_LEN];
	pst_MsgQ 	pstSndMsg;
	pst_MsgQSub pstMsgQSub;
    dbm_msg_t   smsg;
	U8			*pNODE;

	if( dGetNode(&pNODE, &pstSndMsg) < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN dGetNode(S_MNG)", LT);
		return -1;
	}

	memset(szBuf, 0x00, MSG_DATA_LEN);
	smsg.data[0] = 0x00;

	log_print(LOGN_INFO, "FLT SYS TYPE[%s]", flt_info->stTmfInfo.szSysType);

	if(strcmp(flt_info->stTmfInfo.szSysType, "GN") == 0)
	{
    	sprintf(smsg.data, "\n %-10s = %3d SEC", "GN TIME", dGetGNTimer("GN"));
		sprintf(szBuf, "\n %-10s = %3d SEC\n", "FRAG", dGetGNTimer("FRAG"));
	}
	else if(strcmp(flt_info->stTmfInfo.szSysType, "GI") == 0)
	{
    	sprintf(szBuf, "\n NO DATA\n");
	}

	strcat(smsg.data,szBuf);

    smsg.common.mml_err = DBM_SUCCESS;
    smsg.common.cont_flag = DBM_END;
    smsg.head.msg_len = strlen(smsg.data) +1;

	smsg.head.src_proc = SEQ_PROC_CHSMD;
    smsg.head.dst_func = ml->src_func;
    smsg.head.dst_proc = ml->src_proc;
    smsg.head.cmd_id = ml->cmd_id;
    smsg.head.msg_id = ml->msg_id;

	pstMsgQSub = (pst_MsgQSub)&pstSndMsg->llMType;

    pstMsgQSub->usType = DEF_SYS;
	pstMsgQSub->usSvcID = SID_MML;
	pstMsgQSub->usMsgID = MID_MML_RST;

	pstSndMsg->llNID = SEQ_PROC_CHSMD;
	pstSndMsg->llIndex = 1;
	pstSndMsg->usRetCode = 0;
	pstSndMsg->ucNTAFID = flt_info->stTmfInfo.usTmfID;

	//pstSndMsg->usBodyLen = sizeof(dbm_msg_t)-MSG_DATA_LEN+smsg.head.msg_len + NTAFT_HEADER_LEN;
	pstSndMsg->usBodyLen = sizeof(dbm_msg_t)-MSG_DATA_LEN+smsg.head.msg_len;

	log_print(LOGN_INFO, "sizeof(smsg)[%lu], BODY LEN [%d]", sizeof(smsg), pstSndMsg->usBodyLen);

	memcpy( &pstSndMsg->szBody[0], &smsg, pstSndMsg->usBodyLen );

	if( dMsgsnd(SEQ_PROC_CI_SVC, nifo_offset(gpMEMSINFO, pNODE)) < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN dMsgsnd(to=CI_SVC:%d)"EH,LT,SEQ_PROC_CI_SVC,ET);
		return -2;
	}

	log_print(LOGN_DEBUG, LH" * SUCCESS SEND TO CI_SVC=%d, MSG [%s]",LT, SEQ_PROC_CI_SVC, smsg.data);
	return 1;
}

int dGetGNTimer(char *szDelim)
{
    FILE    *fa;
    char    szBuf[1024];
    char    szGubun[36];
    int     dValue;

    if( (fa = fopen(FILE_GN_TIMEOUT, "r")) == NULL )
    {
        log_print( LOGN_WARN, "[FAIL:%d] OPEN FILE[%s]", errno, FILE_GN_TIMEOUT );
        return -1;
    }

	while(fgets(szBuf, 1024, fa) != NULL)
    {
        if(szBuf[0] != '#')
        {
            log_print(LOGN_DEBUG, "%s FILE FORMAT ERROR", FILE_GN_TIMEOUT);
            fclose(fa);
            return -1;
        }

        if(szBuf[1] == '#')
            continue;
        else if(szBuf[1] == 'E')
            break;

        if( sscanf(&szBuf[2], "%s %d", szGubun, &dValue) == 2)
        {
            if(strcmp(szGubun, szDelim) == 0)
            {
                fclose(fa);
                return dValue;
            }
        }
    } /* while-loop end*/

    fclose(fa);
    return 0;
}

void Apply_Filter_TRC(st_TraceList *pstTraceList)
{
	int		i;

	trace_tbl->count = pstTraceList->count;
	if(pstTraceList->count == 0)
	{
		memset(trace_tbl, 0x00, sizeof(st_TraceList));
		return ;
	}

	log_print(LOGN_DEBUG, "<<<<<<< * TRACE INFOMATION * >>>>>>>");
	for(i = 0; i < pstTraceList->count; i++)
	{
		if(pstTraceList->stTraceInfo[i].dType == TRC_TYPE_IP)
		{
			trace_tbl->stTraceInfo[i].stTraceID.uIP = pstTraceList->stTraceInfo[i].stTraceID.uIP;
			log_print(LOGN_DEBUG, "IP[%u]", trace_tbl->stTraceInfo[i].stTraceID.uIP);
		}
		else if(pstTraceList->stTraceInfo[i].dType == TRC_TYPE_IMSI)
		{
			trace_tbl->stTraceInfo[i].stTraceID.llIMSI = atoll((char*)pstTraceList->stTraceInfo[i].stTraceID.szMIN);
			sprintf((char*)trace_tbl->stTraceInfo[i].stTraceID.szMIN, "%s", pstTraceList->stTraceInfo[i].stTraceID.szMIN);
			trace_tbl->stTraceInfo[i].dType = TRC_TYPE_IMSI;
			log_print(LOGN_DEBUG, "IMSI[%lld]", trace_tbl->stTraceInfo[i].stTraceID.llIMSI);
		}
		else if(pstTraceList->stTraceInfo[i].dType == TRC_TYPE_MDN)
		{
			trace_tbl->stTraceInfo[i].stTraceID.llIMSI = atoll((char*)pstTraceList->stTraceInfo[i].stTraceID.szMIN);
			sprintf((char*)trace_tbl->stTraceInfo[i].stTraceID.szMIN, "%s", pstTraceList->stTraceInfo[i].stTraceID.szMIN);
			trace_tbl->stTraceInfo[i].dType = TRC_TYPE_MDN;
			log_print(LOGN_DEBUG, "CTN[%s]", trace_tbl->stTraceInfo[i].stTraceID.szMIN);
		}
		else if(pstTraceList->stTraceInfo[i].dType == TRC_TYPE_ROAM_IMSI)
		{
			trace_tbl->stTraceInfo[i].stTraceID.llIMSI = atoll((char*)pstTraceList->stTraceInfo[i].stTraceID.szMIN);
			sprintf((char*)trace_tbl->stTraceInfo[i].stTraceID.szMIN, "%s", pstTraceList->stTraceInfo[i].stTraceID.szMIN);
			trace_tbl->stTraceInfo[i].dType = TRC_TYPE_ROAM_IMSI;
			log_print(LOGN_DEBUG, "ROAM_IMSI[%010lld]", trace_tbl->stTraceInfo[i].stTraceID.llIMSI);
		}
		else if(pstTraceList->stTraceInfo[i].dType == TRC_TYPE_ROAM_MDN)
		{
			trace_tbl->stTraceInfo[i].stTraceID.llIMSI = atoll((char*)pstTraceList->stTraceInfo[i].stTraceID.szMIN);
			sprintf((char*)trace_tbl->stTraceInfo[i].stTraceID.szMIN, "%s", pstTraceList->stTraceInfo[i].stTraceID.szMIN);
			trace_tbl->stTraceInfo[i].dType = TRC_TYPE_ROAM_MDN;
			log_print(LOGN_DEBUG, "ROAM_CTN[%s]", trace_tbl->stTraceInfo[i].stTraceID.szMIN);
		}
		else
			log_print(LOGN_DEBUG, "MDN[%s]", pstTraceList->stTraceInfo[i].stTraceID.szMIN);
	}
}

int Apply_Filter_TIMER(TIMER_INFO *pstData)
{
	int		dRet;

	log_print(LOGN_DEBUG, "<<<<<<< * TIMER INFOMATION * >>>>>>>");
	memcpy(&flt_info->stTimerInfo, pstData, sizeof(TIMER_INFO));

	if( (dRet = dWriteTimerFile(&flt_info->stTimerInfo)) < 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN dWriteTimerInfo() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		return -1;
	}

	return 0;
}

int Apply_Filter_SVC_ONOFF(SVC_CONF *pstData)
{
	int dRet;

	log_print(LOGN_DEBUG, "<<<<<<< * SVC ONOFF * >>>>>>>");

	/* FILE Write */
	dRet = Serch_Service_Conf(pstData->dSvcID, pstData->dOnOff);
	if(dRet < 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN Serch_Service_Conf() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		return -1;
	}
	
	return 0;
}

int dWriteTimerFile(TIMER_INFO *pstData)
{
	char	sFileName[PATH_MAX];
	FILE	*fp;

	sprintf(sFileName, "%s", FILE_TIMER);
	if( (fp = fopen(sFileName, "w")) == NULL)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN fopen(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, sFileName, errno, strerror(errno));
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

int dConnectMySQL(MYSQL *pstMySQL, st_ConnInfo *pstConnInfo)
{
	int dRet;
	dRet = db_conn( pstMySQL, pstConnInfo->szIP, pstConnInfo->szName, pstConnInfo->szPass, pstConnInfo->szDBName);
	if( dRet < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN dRet=%d, db error=%d:%s", LT, dRet, mysql_errno(pstMySQL), mysql_error(pstMySQL));
		return -1;
	}
	return 0;
}

int dSelectMNIP(MYSQL *pstMySQL, st_NAS_Shm *pstData)
{
	int			i;
	char		szBuf[MAX_SQLQUERY_SIZE];
	MYSQL_ROW	stRow;
	MYSQL_RES	*pstRst;

	sprintf(szBuf,
		"SELECT "
			"IP, NETMASK, FLAG, IFNULL(SYSTYPE,'-'), IFNULL(`DESC`,'-') "
        "FROM "
        	"FLT_CLT");

	if(mysql_query(pstMySQL, szBuf) != 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_query(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			szBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
		return -1;
	}

	if( (pstRst = mysql_store_result(pstMySQL)) == NULL)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_store_result(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			szBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
		return -2;
	}

	i		= 0;
	while( (stRow = mysql_fetch_row(pstRst)) != NULL)
	{
		if(i >= MAX_MNIP_COUNT)
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: FLT_CLT MAX_MNIP_COUNT[%d] OVER[%d] [%s]", __FILE__, __FUNCTION__, __LINE__,
				MAX_MNIP_COUNT, i, szBuf);
			break;
		}

		pstData->stNAS[i].dIdx			= i+1;
		pstData->stNAS[i].uMNIP			= (unsigned int)atoi(stRow[0]);
		pstData->stNAS[i].usNetMask		= (unsigned short)atoi(stRow[1]);
		pstData->stNAS[i].cFlag			= (unsigned char)atoi(stRow[2]);
		pstData->stNAS[i].cSysType		= (unsigned char)atoi(stRow[3]);

		if(strlen(stRow[4]) < MAX_SDESC)
			strncpy(pstData->stNAS[i].szDesc, stRow[4], MAX_SDESC);
		else
		{
			strncpy(pstData->stNAS[i].szDesc, stRow[4], MAX_SDESC-1);
			pstData->stNAS[i].szDesc[MAX_SDESC-1] = 0x00;
		}
		i++;
	}
	mysql_free_result(pstRst);
	pstData->dCount = i;

	return 0;
}

int dWriteMNIPFile(st_NAS_Shm *pstData)
{
	char			sFullPath[PATH_MAX], sLineBuf[BUF_SIZE];
	int				i, fd;
	ssize_t			sszWrLen;
	size_t			szLineLen;
	struct in_addr	stAddr;


	sprintf(sFullPath, "%s", FILE_FLT_MNIP);
	if( (fd = open(sFullPath, O_CREAT|O_WRONLY|O_TRUNC, 0644)) == -1)
	{
		log_print(LOGN_CRI, LH"FAILED IN open(%s)"EH,LT, FILE_FLT_MNIP, ET);
		return -1;
	}

	for(i = 0; i < pstData->dCount; i++)
	{
		memset(sLineBuf, 0x00, BUF_SIZE);
		sprintf(sLineBuf, "\n( LPREA\n");
		szLineLen = strlen(sLineBuf);

		stAddr.s_addr = htonl(pstData->stNAS[i].uMNIP);
		sprintf(&sLineBuf[szLineLen], "<k>ServerIP=%s\n", inet_ntoa(stAddr));
		szLineLen = strlen(sLineBuf);

		sprintf(&sLineBuf[szLineLen], "<k>Mask=%hu\n", pstData->stNAS[i].usNetMask);
		szLineLen = strlen(sLineBuf);

		switch(pstData->stNAS[i].cFlag)
		{
			case RP_FLAG_INDEX:
				sprintf(&sLineBuf[szLineLen], "<k>RpPiFlag=%s\n", "RP_FLAG");
				break;
			case PI_FLAG_INDEX:
				sprintf(&sLineBuf[szLineLen], "<k>RpPiFlag=%s\n", "PI_FLAG");
				break;
			default:
				log_print(LOGN_CRI, LH"Unknown Flag[%hu], Check FLT_CLT table",LT, pstData->stNAS[i].cFlag);
				if(close(fd) == -1)
				{
					log_print(LOGN_CRI, LH"FAILED IN close()"EH,LT,ET);
					return -2;
				}
				return -3;
		}
		szLineLen = strlen(sLineBuf);

		switch(pstData->stNAS[i].cSysType)
		{
			case PCF_SYSTYPE:
				sprintf(&sLineBuf[szLineLen], "SysType=%s\n", "TYPE_PCF");
				break;
			case PDSN_SYSTYPE:
				sprintf(&sLineBuf[szLineLen], "SysType=%s\n", "TYPE_PDSN");
				break;
			case AAA_SYSTYPE:
				sprintf(&sLineBuf[szLineLen], "SysType=%s\n", "TYPE_AAA");
				break;
			case LNS_SYSTYPE:
				sprintf(&sLineBuf[szLineLen], "SysType=%s\n", "TYPE_LNS");
				break;
			case MNIP_SYSTYPE:
				sprintf(&sLineBuf[szLineLen], "SysType=%s\n", "TYPE_MNIP");
				break;
			case LAC_SYSTYPE:
				sprintf(&sLineBuf[szLineLen], "SysType=%s\n", "TYPE_LAC");
				break;
			case CRX_SYSTYPE:
				sprintf(&sLineBuf[szLineLen], "SysType=%s\n", "TYPE_CRX");
				break;
			case PDIF_SYSTYPE:
				sprintf(&sLineBuf[szLineLen], "SysType=%s\n", "TYPE_PDIF");
				break;
			default:
				log_print(LOGN_CRI, LH"Unknown SysType[%hu], Check FLT_CLT table",LT, pstData->stNAS[i].cSysType);
				if(close(fd) == -1)
				{
					log_print(LOGN_CRI, LH"FAILED IN close()"EH,LT,ET);
					return -4;
				}
				return -5;
		}
		szLineLen = strlen(sLineBuf);

		sprintf(&sLineBuf[szLineLen], ") LPREA\n");
		szLineLen = strlen(sLineBuf);

		if( (sszWrLen = write(fd, sLineBuf, szLineLen)) == -1)
		{
			log_print(LOGN_CRI, LH"FAILED IN write()"EH,LT,ET);
			if(close(fd) == -1)
			{
				log_print(LOGN_CRI, LH"FAILED IN close()"EH,LT,ET);
				return -6;
			}
			return -7;
		}

		if(sszWrLen != szLineLen)
		{
			log_print(LOGN_CRI, LH"Didn't complete to write a file[%s] sszWrLen[%lu] szLineLen[%lu]",
				LT, FILE_FLT_MNIP, sszWrLen, szLineLen);
			if(close(fd) == -1)
			{
				log_print(LOGN_CRI, LH"FAILED IN close()"EH,LT,ET);
				return -8;
			}
			return -9;
		}
	}

	if(close(fd) == -1)
	{
		log_print(LOGN_CRI, LH"FAILED IN close()"EH,LT,ET);
		return -10;
	}

	return 0;
}

int Send_mmc(int dMID, UINT uIndex)
{
	int				dRet;
	pst_MsgQ		pstMsgQ;
	pst_MsgQSub		pstMsgQSub;
	st_Conf			stConf;
	U8 				*pNODE;

	if( dGetNode(&pNODE, &pstMsgQ) < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN dGetNode(S_MNG)", LT);
		return -1;
	}

	/* MAKE MSGQ CHK */
	pstMsgQSub = (pst_MsgQSub)&pstMsgQ->llMType;

	pstMsgQSub->usSvcID	= SID_CHKRES;
	pstMsgQSub->usType	= DEF_SYS;
	pstMsgQSub->usMsgID	= dMID;

	pstMsgQ->ucNTAFID	= flt_info->stTmfInfo.usTmfID;
	pstMsgQ->ucProID	= SEQ_PROC_S_MNG;
	pstMsgQ->dMsgQID	= 0;
	pstMsgQ->llIndex	= uIndex;

	log_print(LOGN_INFO, "F=%s:%s.%d: MID[%d] uIndex[%d]", __FILE__, __FUNCTION__, __LINE__, dMID, uIndex);

	memset(&stConf,0x00,sizeof(st_Conf));
	if(dMID == MID_FLT_ALM)
	{
		pstMsgQ->usBodyLen = sizeof(st_AlmLevel_List)+NTAFT_HEADER_LEN;
		memcpy(&pstMsgQ->szBody[NTAFT_HEADER_LEN], &flt_info->stAlmLevelList, sizeof(st_AlmLevel_List));
	}
	else if(dMID == MID_DIS_NTAF_CONF)
	{
		stConf.usLogLevel	= flt_info->stLogLevelList.stLogLevel.usLogLevel[0]  ;
		stConf.usSysNo		= flt_info->stTmfInfo.usTmfID;
		stConf.usTcpHdr		= flt_info->stHdrLogShm.sHdrCapFlag;
		pstMsgQ->usBodyLen	= sizeof(st_Conf)+NTAFT_HEADER_LEN;
		memcpy(&pstMsgQ->szBody[NTAFT_HEADER_LEN], &stConf, sizeof(st_Conf));
	}

	if( (dRet = dMsgsnd(SEQ_PROC_CI_SVC, nifo_offset( gpMEMSINFO, pNODE ))) < 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN dMsgsnd() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		return 0;
	}
	log_print(LOGN_INFO, "F=%s:%s.%d: CHK_SVC SND OK CI_SVC [%d] BodyLen[%d] uIndex[%d]", 
		__FILE__, __FUNCTION__, __LINE__, SEQ_PROC_CI_SVC,pstMsgQ->usBodyLen ,uIndex);

	return 0;
}

int dSelectSVRIP(MYSQL *pstMySQL, st_SvcInfo_Shm *pstData)
{
	int			i;
	char		szBuf[MAX_SQLQUERY_SIZE];
	MYSQL_ROW	stRow;
	MYSQL_RES	*pstRst;

	sprintf(szBuf,
		"SELECT "
			"IP,PORT,FLAG,IFNULL(SYSTYPE,0),IFNULL(L4,0),IFNULL(L7,0),IFNULL(APPCODE,0), IFNULL(`DESC`,'-') "
		"FROM "
			"FLT_SVR "
		"ORDER BY "
			"IP,PORT,FLAG");

	if(mysql_query(pstMySQL, szBuf) != 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_query(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			szBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
		return -1;
	}

	if( (pstRst = mysql_store_result(pstMySQL)) == NULL)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_store_result(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			szBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
		return -2;
	}

	i		= 0;
	while( (stRow = mysql_fetch_row(pstRst)) != NULL)
	{
		if(i >= MAX_SVC_CNT)
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: FLT_SVR MAX_SVC_CNT[%d] OVER[%d] [%s]", __FILE__, __FUNCTION__, __LINE__,
				MAX_SVC_CNT, i, szBuf);
			break;
		}

		pstData->stSvcInfo[i].dIdx		= i+1;
		pstData->stSvcInfo[i].uSvcIP	= (unsigned int)atoi(stRow[0]);
		pstData->stSvcInfo[i].huPort	= (unsigned short)atoi(stRow[1]);
		pstData->stSvcInfo[i].cFlag		= (unsigned short)atoi(stRow[2]);
		pstData->stSvcInfo[i].cSysType	= (unsigned short)atoi(stRow[3]);

		pstData->stSvcInfo[i].huL4Code	= (unsigned short)atoi(stRow[4]);
		pstData->stSvcInfo[i].huL7Code	= (unsigned short)atoi(stRow[5]);
		pstData->stSvcInfo[i].huAppCode	= (unsigned short)atoi(stRow[6]);

		if(strlen(stRow[7]) < MAX_SDESC)
			strncpy(pstData->stSvcInfo[i].szDesc, stRow[7], MAX_SDESC);
		else
		{
			strncpy(pstData->stSvcInfo[i].szDesc, stRow[7], MAX_SDESC-1);
			pstData->stSvcInfo[i].szDesc[MAX_SDESC-1] = 0x00;
		}
		i++;
	}
	mysql_free_result(pstRst);
	pstData->dCount = i;

	return 0;

}

int dWriteSVRIPFile(st_SvcInfo_Shm *pstData)
{
	char			sFullPath[PATH_MAX], sLineBuf[BUF_SIZE];
	int				i, fd;
	ssize_t			sszWrLen;
	size_t			szLineLen;
	struct in_addr	stAddr;


	sprintf(sFullPath, "%s", FILE_FLT_SVR);
	if( (fd = open(sFullPath, O_CREAT|O_WRONLY|O_TRUNC, 0644)) == -1)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN open(%s) [errno:%d-%s]", 
			__FILE__, __FUNCTION__, __LINE__, FILE_FLT_SVR, errno, strerror(errno));
		return -1;
	}

	for(i = 0; i < pstData->dCount; i++)
	{

#if 1 /* INYOUNG */
		switch(pstData->stSvcInfo[i].huL4Code)
		{
			case L4_INET_TCP_USER:
			case L4_INET_HTTP_USER:
			case L4_INET_TCP:
			case L4_INET_TCP_RECV:
			case L4_INET_HTTP:
			case L4_INET_HTTP_RECV:
				continue;
		}
#endif

		memset(sLineBuf, 0x00, BUF_SIZE);
		sprintf(sLineBuf, "\n( LPREA\n");
		szLineLen = strlen(sLineBuf);

		stAddr.s_addr = htonl(pstData->stSvcInfo[i].uSvcIP);
		sprintf(&sLineBuf[szLineLen], "<k>ServerIP=%s\n", inet_ntoa(stAddr));
		szLineLen = strlen(sLineBuf);

		sprintf(&sLineBuf[szLineLen], "<k>ServerPort=%hu\n", pstData->stSvcInfo[i].huPort);
		szLineLen = strlen(sLineBuf);

		switch(pstData->stSvcInfo[i].cFlag)
		{
			case RP_FLAG_INDEX:
				sprintf(&sLineBuf[szLineLen], "<k>RpPiFlag=%s\n", "RP_FLAG");
				break;
			case PI_FLAG_INDEX:
				sprintf(&sLineBuf[szLineLen], "<k>RpPiFlag=%s\n", "PI_FLAG");
				break;
			default:
				log_print(LOGN_CRI, "F=%s:%s.%d: Unknown Flag[%hu], Check FLT_SVR table", __FILE__, __FUNCTION__, __LINE__, pstData->stSvcInfo[i].cFlag);
				if(close(fd) == -1)
				{
					log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN close() [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, errno, strerror(errno));
					return -2;
				}
				return -3;
		}
		szLineLen = strlen(sLineBuf);

		switch(pstData->stSvcInfo[i].cSysType)
		{
			case PDSN_SYSTYPE:
				sprintf(&sLineBuf[szLineLen], "SysType=%s\n", "TYPE_PDSN");
				break;
			case AAA_SYSTYPE:
				sprintf(&sLineBuf[szLineLen], "SysType=%s\n", "TYPE_AAA");
				break;
			case LNS_SYSTYPE:
				sprintf(&sLineBuf[szLineLen], "SysType=%s\n", "TYPE_LNS");
				break;
			case SERVICE_SYSTYPE:
				sprintf(&sLineBuf[szLineLen], "SysType=%s\n", "TYPE_SVC");
				break;
			case PDIF_SYSTYPE:
				sprintf(&sLineBuf[szLineLen], "SysType=%s\n", "TYPE_PDIF");
				break;
			default:
				log_print(LOGN_CRI, "F=%s:%s.%d: Unknown SysType[%hu], Check FLT_SVR table", __FILE__, __FUNCTION__, __LINE__, pstData->stSvcInfo[i].cSysType);
				if(close(fd) == -1)
				{
					log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN close() [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, errno, strerror(errno));
					return -4;
				}
				return -5;
		}
		szLineLen = strlen(sLineBuf);

		switch(pstData->stSvcInfo[i].huL4Code)
		{
			case L4_WAP20:
				sprintf(&sLineBuf[szLineLen], "L4Code=%s\n", "L4_WAP20");
				break;
			case L4_TODAY:
				sprintf(&sLineBuf[szLineLen], "L4Code=%s\n", "L4_TODAY");
				break;
			case L4_WIPI:
				sprintf(&sLineBuf[szLineLen], "L4Code=%s\n", "L4_WIPI");
				break;
			case L4_WIPI_ONLINE:
				sprintf(&sLineBuf[szLineLen], "L4Code=%s\n", "L4_WIPI_ONLINE");
				break;
			case L4_DN_2G:
				sprintf(&sLineBuf[szLineLen], "L4Code=%s\n", "L4_DN_2G");
				break;
			case L4_DN_2G_NODN:
				sprintf(&sLineBuf[szLineLen], "L4Code=%s\n", "L4_DN_2G_NODN");
				break;
			case L4_DN_JAVA:
				sprintf(&sLineBuf[szLineLen], "L4Code=%s\n", "L4_DN_JAVA");
				break;
			case L4_DN_VOD:
				sprintf(&sLineBuf[szLineLen], "L4Code=%s\n", "L4_DN_VOD");
				break;
			case L4_DN_VOD_NODN:
				sprintf(&sLineBuf[szLineLen], "L4Code=%s\n", "L4_DN_VOD_NODN");
				break;
			case L4_OMA_DN:
				sprintf(&sLineBuf[szLineLen], "L4Code=%s\n", "L4_OMA_DN");
				break;
			case L4_OMA_DN_2G:
				sprintf(&sLineBuf[szLineLen], "L4Code=%s\n", "L4_OMA_DN_2G");
				break;
			case L4_OMA_DN_VOD:
				sprintf(&sLineBuf[szLineLen], "L4Code=%s\n", "L4_OMA_DN_VOD");
				break;
			case L4_OMA_DN_WIPI:
				sprintf(&sLineBuf[szLineLen], "L4Code=%s\n", "L4_OMA_DN_WIPI");
				break;
			case L4_VOD_STREAM:
				sprintf(&sLineBuf[szLineLen], "L4Code=%s\n", "L4_VOD_STREAM");
				break;
			case L4_RTS_FB:
				sprintf(&sLineBuf[szLineLen], "L4Code=%s\n", "L4_RTS_FB");
				break;
			case L4_RTS_WB:
				sprintf(&sLineBuf[szLineLen], "L4Code=%s\n", "L4_RTS_WB");
				break;
			case L4_MMS_UP:
				sprintf(&sLineBuf[szLineLen], "L4Code=%s\n", "L4_MMS_UP");
				break;
			case L4_MMS_UP_NODN:
				sprintf(&sLineBuf[szLineLen], "L4Code=%s\n", "L4_MMS_UP_NODN");
				break;
			case L4_MMS_DN:
				sprintf(&sLineBuf[szLineLen], "L4Code=%s\n", "L4_MMS_DN");
				break;
			case L4_MMS_DN_NODN:
				sprintf(&sLineBuf[szLineLen], "L4Code=%s\n", "L4_MMS_DN_NODN");
				break;
			case L4_MMS_NEW:
				sprintf(&sLineBuf[szLineLen], "L4Code=%s\n", "L4_MMS_NEW");
				break;
			case L4_JNC:
				sprintf(&sLineBuf[szLineLen], "L4Code=%s\n", "L4_JNC");
				break;
			case L4_FB:
				sprintf(&sLineBuf[szLineLen], "L4Code=%s\n", "L4_FB");
				break;
			case L4_IV:
				sprintf(&sLineBuf[szLineLen], "L4Code=%s\n", "L4_IV");
				break;
			case L4_EMS:
				sprintf(&sLineBuf[szLineLen], "L4Code=%s\n", "L4_EMS");
				break;
			case L4_P_EMS:
				sprintf(&sLineBuf[szLineLen], "L4Code=%s\n", "L4_P_EMS");
				break;
			case L4_EMS_NO:
				sprintf(&sLineBuf[szLineLen], "L4Code=%s\n", "L4_EMS_NO");
				break;
			case L4_FV_FB:
				sprintf(&sLineBuf[szLineLen], "L4Code=%s\n", "L4_FV_FB");
				break;
			case L4_FV_EMS:
				sprintf(&sLineBuf[szLineLen], "L4Code=%s\n", "L4_FV_EMS");
				break;
			case L4_FV_IV:
				sprintf(&sLineBuf[szLineLen], "L4Code=%s\n", "L4_FV_IV");
				break;
			case L4_SIP_MS:
				sprintf(&sLineBuf[szLineLen], "L4Code=%s\n", "L4_SIP_MS");
				break;
			case L4_SIP_VENDOR:
				sprintf(&sLineBuf[szLineLen], "L4Code=%s\n", "L4_SIP_VENDOR");
				break;
			case L4_SIP_CSCF:
				sprintf(&sLineBuf[szLineLen], "L4Code=%s\n", "L4_SIP_CSCF");
				break;
			case L4_MSRP_MS:
				sprintf(&sLineBuf[szLineLen], "L4Code=%s\n", "L4_MSRP_MS");
				break;
			case L4_MSRP_VENDOR:
				sprintf(&sLineBuf[szLineLen], "L4Code=%s\n", "L4_MSRP_VENDOR");
				break;
			case L4_XCAP:
				sprintf(&sLineBuf[szLineLen], "L4Code=%s\n", "L4_XCAP");
				break;
			case L4_MBOX:
				sprintf(&sLineBuf[szLineLen], "L4Code=%s\n", "L4_MBOX");
				break;
			case L4_BANKON:
				sprintf(&sLineBuf[szLineLen], "L4Code=%s\n", "L4_BANKON");
				break;
			case L4_VMBANK:
				sprintf(&sLineBuf[szLineLen], "L4Code=%s\n", "L4_VMBANK");
				break;
			case L4_WIDGET:
				sprintf(&sLineBuf[szLineLen], "L4Code=%s\n", "L4_WIDGET");
				break;
			case L4_WIDGET_NODN:
				sprintf(&sLineBuf[szLineLen], "L4Code=%s\n", "L4_WIDGET_NODN");
				break;
			case L4_VT:
				sprintf(&sLineBuf[szLineLen], "L4Code=%s\n", "L4_VT");
				break;
			case L4_PHONE:
				sprintf(&sLineBuf[szLineLen], "L4Code=%s\n", "L4_PHONE");
				break;
			case L4_PHONE_ETC:
				sprintf(&sLineBuf[szLineLen], "L4Code=%s\n", "L4_PHONE_ETC");
				break;
			case L4_FTP:
				sprintf(&sLineBuf[szLineLen], "L4Code=%s\n", "L4_FTP");
				break;
			case L4_DNS:
				sprintf(&sLineBuf[szLineLen], "L4Code=%s\n", "L4_DNS");
				break;
			case L4_CORP:
				sprintf(&sLineBuf[szLineLen], "L4Code=%s\n", "L4_CORP");
				break;
			case L4_UNKNOWN:
				sprintf(&sLineBuf[szLineLen], "L4Code=%s\n", "L4_UNKNOWN");
				break;
			case L4_INET_TCP:
			case L4_INET_TCP_RECV:
			case L4_INET_HTTP:
			case L4_INET_HTTP_RECV:
			case L4_INET_TCP_USER:
			case L4_INET_HTTP_USER:
				break;
			default:
				log_print(LOGN_CRI, "F=%s:%s.%d: Unknown L4Code[%hu], Check FLT_SVR table", __FILE__, __FUNCTION__, __LINE__, pstData->stSvcInfo[i].huL4Code);
				if(close(fd) == -1)
				{
					log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN close() [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, errno, strerror(errno));
					return -6;
				}
				return -7;
		}
		szLineLen = strlen(sLineBuf);

		switch(pstData->stSvcInfo[i].huL7Code)
		{
			case APP_UNKNOWN:
				sprintf(&sLineBuf[szLineLen], "L7Code=%s\n", "APP_UNKNOWN");
				break;
			case APP_MENU:
				sprintf(&sLineBuf[szLineLen], "L7Code=%s\n", "APP_MENU");
				break;
			case APP_DOWN:
				sprintf(&sLineBuf[szLineLen], "L7Code=%s\n", "APP_DOWN");
				break;
			case APP_STREAM:
				sprintf(&sLineBuf[szLineLen], "L7Code=%s\n", "APP_STREAM");
				break;
			case APP_MMS:
				sprintf(&sLineBuf[szLineLen], "L7Code=%s\n", "APP_MMS");
				break;
			case APP_JNC:
				sprintf(&sLineBuf[szLineLen], "L7Code=%s\n", "APP_JNC");
				break;
			case APP_ONLINE:
				sprintf(&sLineBuf[szLineLen], "L7Code=%s\n", "APP_ONLINE");
				break;
			case APP_ETC:
				sprintf(&sLineBuf[szLineLen], "L7Code=%s\n", "APP_ETC");
				break;
			case APP_FV_DOC:
				sprintf(&sLineBuf[szLineLen], "L7Code=%s\n", "APP_FV_DOC");
				break;
			case APP_FV_PAGE:
				sprintf(&sLineBuf[szLineLen], "L7Code=%s\n", "APP_FV_PAGE");
				break;
			case APP_FV_SAVE:
				sprintf(&sLineBuf[szLineLen], "L7Code=%s\n", "APP_FV_SAVE");
				break;
			case APP_EMS_SRVADD:
				sprintf(&sLineBuf[szLineLen], "L7Code=%s\n", "APP_EMS_SRVADD");
				break;
			case APP_EMS_SRVMOD:
				sprintf(&sLineBuf[szLineLen], "L7Code=%s\n", "APP_EMS_SRVMOD");
				break;
			case APP_EMS_SRVDEL:
				sprintf(&sLineBuf[szLineLen], "L7Code=%s\n", "APP_EMS_SRVDEL");
				break;
			case APP_EMS_SPAMADD:
				sprintf(&sLineBuf[szLineLen], "L7Code=%s\n", "APP_EMS_SPAMADD");
				break;
			case APP_EMS_SPAMDEL:
				sprintf(&sLineBuf[szLineLen], "L7Code=%s\n", "APP_EMS_SPAMDEL");
				break;
			case APP_EMS_RCVTIME:
				sprintf(&sLineBuf[szLineLen], "L7Code=%s\n", "APP_EMS_RCVTIME");
				break;
			case APP_EMS_NEWMAIL:
				sprintf(&sLineBuf[szLineLen], "L7Code=%s\n", "APP_EMS_NEWMAIL");
				break;
			case APP_EMS_BODY:
				sprintf(&sLineBuf[szLineLen], "L7Code=%s\n", "APP_EMS_BODY");
				break;
			case APP_EMS_SEND:
				sprintf(&sLineBuf[szLineLen], "L7Code=%s\n", "APP_EMS_SEND");
				break;
			case APP_EMS_ACK:
				sprintf(&sLineBuf[szLineLen], "L7Code=%s\n", "APP_EMS_ACK");
				break;
			case APP_EMS_SYNC:
				sprintf(&sLineBuf[szLineLen], "L7Code=%s\n", "APP_EMS_SYNC");
				break;
			case APP_IM_UP:
				sprintf(&sLineBuf[szLineLen], "L7Code=%s\n", "APP_IM_UP");
				break;
			case APP_IM_DN:
				sprintf(&sLineBuf[szLineLen], "L7Code=%s\n", "APP_IM_DN");
				break;
			case APP_WIDGET_UTI:
				sprintf(&sLineBuf[szLineLen], "L7Code=%s\n", "APP_WIDGET_UTI");
				break;
			case APP_WIDGET_SSD:
				sprintf(&sLineBuf[szLineLen], "L7Code=%s\n", "APP_WIDGET_SSD");
				break;
			case APP_WIDGET_PID:
				sprintf(&sLineBuf[szLineLen], "L7Code=%s\n", "APP_WIDGET_PID");
				break;
			case APP_WIDGET_SWD:
				sprintf(&sLineBuf[szLineLen], "L7Code=%s\n", "APP_WIDGET_SWD");
				break;
			case APP_WIDGET_DWD:
				sprintf(&sLineBuf[szLineLen], "L7Code=%s\n", "APP_WIDGET_DWD");
				break;
			case APP_WIDGET_WSC:
				sprintf(&sLineBuf[szLineLen], "L7Code=%s\n", "APP_WIDGET_WSC");
				break;
			case APP_WIDGET_WAC:
				sprintf(&sLineBuf[szLineLen], "L7Code=%s\n", "APP_WIDGET_WAC");
				break;
			case APP_WIDGET_WMU:
				sprintf(&sLineBuf[szLineLen], "L7Code=%s\n", "APP_WIDGET_WMU");
				break;
			case APP_WIDGET_WCD:
				sprintf(&sLineBuf[szLineLen], "L7Code=%s\n", "APP_WIDGET_WCD");
				break;
			case APP_WIDGET_WDL:
				sprintf(&sLineBuf[szLineLen], "L7Code=%s\n", "APP_WIDGET_WDL");
				break;
			case APP_WIDGET_WIN:
				sprintf(&sLineBuf[szLineLen], "L7Code=%s\n", "APP_WIDGET_WIN");
				break;
			case APP_PHONE:
				sprintf(&sLineBuf[szLineLen], "L7Code=%s\n", "APP_PHONE");
				break;
			case APP_FTP:
				sprintf(&sLineBuf[szLineLen], "L7Code=%s\n", "APP_FTP");
				break;
			case APP_DNS:
				sprintf(&sLineBuf[szLineLen], "L7Code=%s\n", "APP_DNS");
				break;
			default:
				log_print(LOGN_CRI, "F=%s:%s.%d: Unknown L7Code[%hu], Check FLT_SVR table", __FILE__, __FUNCTION__, __LINE__, pstData->stSvcInfo[i].huL7Code);
				if(close(fd) == -1)
				{
					log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN close() [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, errno, strerror(errno));
					return -8;
				}
				return -9;
		}
		szLineLen = strlen(sLineBuf);

		switch(pstData->stSvcInfo[i].huAppCode)
		{
			case 0:
				sprintf(&sLineBuf[szLineLen], "AppCode=%s\n", "SEQ_PROC_ETC");
				break;
			case SEQ_PROC_A_UDP:
				sprintf(&sLineBuf[szLineLen], "AppCode=%s\n", "SEQ_PROC_A_UDP");
				break;
			case SEQ_PROC_A_HTTP0:
				sprintf(&sLineBuf[szLineLen], "AppCode=%s\n", "SEQ_PROC_A_HTTP0");
				break;
			case SEQ_PROC_A_HTTP1:
				sprintf(&sLineBuf[szLineLen], "AppCode=%s\n", "SEQ_PROC_A_HTTP1");
				break;
			case SEQ_PROC_A_HTTP2:
				sprintf(&sLineBuf[szLineLen], "AppCode=%s\n", "SEQ_PROC_A_HTTP2");
				break;
			case SEQ_PROC_A_HTTP3:
				sprintf(&sLineBuf[szLineLen], "AppCode=%s\n", "SEQ_PROC_A_HTTP3");
				break;
			case SEQ_PROC_A_HTTP4:
				sprintf(&sLineBuf[szLineLen], "AppCode=%s\n", "SEQ_PROC_A_HTTP4");
				break;
			case SEQ_PROC_A_WAP20:
				sprintf(&sLineBuf[szLineLen], "AppCode=%s\n", "SEQ_PROC_A_WAP20");
				break;
			case SEQ_PROC_A_WIPI:
				sprintf(&sLineBuf[szLineLen], "AppCode=%s\n", "SEQ_PROC_A_WIPI");
				break;
			case SEQ_PROC_A_2G:
				sprintf(&sLineBuf[szLineLen], "AppCode=%s\n", "SEQ_PROC_A_2G");
				break;
			case SEQ_PROC_A_JAVA:
				sprintf(&sLineBuf[szLineLen], "AppCode=%s\n", "SEQ_PROC_A_JAVA");
				break;
			case SEQ_PROC_A_VOD:
				sprintf(&sLineBuf[szLineLen], "AppCode=%s\n", "SEQ_PROC_A_VOD");
				break;
			case SEQ_PROC_A_MMS:
				sprintf(&sLineBuf[szLineLen], "AppCode=%s\n", "SEQ_PROC_A_MMS");
				break;
			case SEQ_PROC_A_JNC:
				sprintf(&sLineBuf[szLineLen], "AppCode=%s\n", "SEQ_PROC_A_JNC");
				break;
			case SEQ_PROC_A_CALL0:
				sprintf(&sLineBuf[szLineLen], "AppCode=%s\n", "SEQ_PROC_A_CALL0");
				break;
			case SEQ_PROC_A_CALL1:
				sprintf(&sLineBuf[szLineLen], "AppCode=%s\n", "SEQ_PROC_A_CALL1");
				break;
			case SEQ_PROC_A_CALL2:
				sprintf(&sLineBuf[szLineLen], "AppCode=%s\n", "SEQ_PROC_A_CALL2");
				break;
			case SEQ_PROC_A_FV:
				sprintf(&sLineBuf[szLineLen], "AppCode=%s\n", "SEQ_PROC_A_FV");
				break;
			case SEQ_PROC_A_EMS:
				sprintf(&sLineBuf[szLineLen], "AppCode=%s\n", "SEQ_PROC_A_EMS");
				break;
			case SEQ_PROC_A_IV:
				sprintf(&sLineBuf[szLineLen], "AppCode=%s\n", "SEQ_PROC_A_IV");
				break;
			case SEQ_PROC_A_FB:
				sprintf(&sLineBuf[szLineLen], "AppCode=%s\n", "SEQ_PROC_A_FB");
				break;
			case SEQ_PROC_A_SIPM:
				sprintf(&sLineBuf[szLineLen], "AppCode=%s\n", "SEQ_PROC_A_SIPM");
				break;
			case SEQ_PROC_A_SIPT:
				sprintf(&sLineBuf[szLineLen], "AppCode=%s\n", "SEQ_PROC_A_SIPT");
				break;
			case SEQ_PROC_A_SIP:
				sprintf(&sLineBuf[szLineLen], "AppCode=%s\n", "SEQ_PROC_A_SIP");
				break;
			case SEQ_PROC_A_MSRPM:
				sprintf(&sLineBuf[szLineLen], "AppCode=%s\n", "SEQ_PROC_A_MSRPM");
				break;
			case SEQ_PROC_A_MSRPT:
				sprintf(&sLineBuf[szLineLen], "AppCode=%s\n", "SEQ_PROC_A_MSRPT");
				break;
			case SEQ_PROC_A_XCAP:
				sprintf(&sLineBuf[szLineLen], "AppCode=%s\n", "SEQ_PROC_A_XCAP");
				break;
			case SEQ_PROC_A_ONLINE:
				sprintf(&sLineBuf[szLineLen], "AppCode=%s\n", "SEQ_PROC_A_ONLINE");
				break;
			case SEQ_PROC_A_WIDGET:
				sprintf(&sLineBuf[szLineLen], "AppCode=%s\n", "SEQ_PROC_A_WIDGET");
				break;
			case SEQ_PROC_A_SCTP:
				sprintf(&sLineBuf[szLineLen], "AppCode=%s\n", "SEQ_PROC_A_SCTP");
				break;
			case SEQ_PROC_A_DIAMETER:
				sprintf(&sLineBuf[szLineLen], "AppCode=%s\n", "SEQ_PROC_A_DIAMETER");
				break;
			case SEQ_PROC_A_RADIUS:
				sprintf(&sLineBuf[szLineLen], "AppCode=%s\n", "SEQ_PROC_A_RADIUS");
				break;
			case SEQ_PROC_A_VT:
				sprintf(&sLineBuf[szLineLen], "AppCode=%s\n", "SEQ_PROC_A_VT");
				break;
			case SEQ_PROC_A_IM:
				sprintf(&sLineBuf[szLineLen], "AppCode=%s\n", "SEQ_PROC_A_IM");
				break;
			case SEQ_PROC_A_RP0:
				sprintf(&sLineBuf[szLineLen], "AppCode=%s\n", "SEQ_PROC_A_RP0");
				break;
			case SEQ_PROC_A_GRE0:
				sprintf(&sLineBuf[szLineLen], "AppCode=%s\n", "SEQ_PROC_A_GRE0");
				break;
			case SEQ_PROC_A_FTP:
				sprintf(&sLineBuf[szLineLen], "AppCode=%s\n", "SEQ_PROC_A_FTP");
				break;
			case SEQ_PROC_A_DNS:
				sprintf(&sLineBuf[szLineLen], "AppCode=%s\n", "SEQ_PROC_A_DNS");
				break;
			case SEQ_PROC_A_L2TP:
				sprintf(&sLineBuf[szLineLen], "AppCode=%s\n", "SEQ_PROC_A_L2TP");
				break;
			default:
				log_print(LOGN_CRI, "F=%s:%s.%d: Unknown AppCode[%hu], Check FLT_SVR table", __FILE__, __FUNCTION__, __LINE__, pstData->stSvcInfo[i].huAppCode);
				if(close(fd) == -1)
				{
					log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN close() [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, errno, strerror(errno));
					return -10;
				}
				return -11;
		}
		szLineLen = strlen(sLineBuf);

		sprintf(&sLineBuf[szLineLen], ") LPREA\n");
		szLineLen = strlen(sLineBuf);

		if( (sszWrLen = write(fd, sLineBuf, szLineLen)) == -1)
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN write() [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, errno, strerror(errno));
			if(close(fd) == -1)
			{
				log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN close() [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, errno, strerror(errno));
				return -12;
			}
			return -13;
		}

		if(sszWrLen != szLineLen)
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: Didn't complete to write a file[%s] sszWrLen[%lu] szLineLen[%lu]", __FILE__, __FUNCTION__, __LINE__,
				"FLT_MNIP.dat", sszWrLen, szLineLen);

			if(close(fd) == -1)
			{
				log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN close() [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, errno, strerror(errno));
				return -14;
			}
			return -15;
		}
	}

	if(close(fd) == -1)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN close() [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, errno, strerror(errno));
		return -16;
	}

	return 0;
}

int dSelectSCTP(MYSQL *pstMySQL, st_SCTP_Shm *pstData)
{
	int			i;
	char		szBuf[MAX_SQLQUERY_SIZE];
	MYSQL_ROW	stRow;
	MYSQL_RES	*pstRst;

	sprintf(szBuf,
		"SELECT "
			"IP,IFNULL(SYSTYPE,0),IFNULL(DIRECTION,0),IFNULL(GROUPID,0),IFNULL(`DESC`,'-') "
		"FROM "
			"FLT_SCTP "
		"ORDER BY "
			"IP");

	if(mysql_query(pstMySQL, szBuf) != 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_query(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			szBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
		return -1;
	}

	if( (pstRst = mysql_store_result(pstMySQL)) == NULL)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN mysql_store_result(%s) [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__,
			szBuf, mysql_errno(pstMySQL), mysql_error(pstMySQL));
		return -2;
	}

	i		= 0;
	while( (stRow = mysql_fetch_row(pstRst)) != NULL)
	{
		if(i >= MAX_SCTP_COUNT)
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: FLT_CLT MAX_MNIP_COUNT[%d] OVER[%d] [%s]", __FILE__, __FUNCTION__, __LINE__,
				MAX_SCTP_COUNT, i, szBuf);
			break;
		}

		pstData->stSCTP[i].dIdx			= i+1;
		pstData->stSCTP[i].uSCTPIP		= (unsigned int)atoi(stRow[0]);
		pstData->stSCTP[i].cSysType		= (unsigned char)atoi(stRow[1]);
		pstData->stSCTP[i].cDirection	= (unsigned char)atoi(stRow[2]);
		pstData->stSCTP[i].huGroupID	= (unsigned short)atoi(stRow[3]);

		if(strlen(stRow[4]) < MAX_SDESC)
			strncpy(pstData->stSCTP[i].szDesc, stRow[4], MAX_SDESC);
		else
		{
			strncpy(pstData->stSCTP[i].szDesc, stRow[4], MAX_SDESC-1);
			pstData->stSCTP[i].szDesc[MAX_SDESC-1] = 0x00;
		}
		i++;
	}
	mysql_free_result(pstRst);
	pstData->dCount = i;

	return 0;
}

int dWriteSCTPFile(st_SCTP_Shm *pstData)
{
	char			sFullPath[PATH_MAX], sLineBuf[BUF_SIZE];
	int				i, fd;
	ssize_t			sszWrLen;
	size_t			szLineLen;
	struct in_addr	stAddr;


	sprintf(sFullPath, "%s", FILE_FLT_SCTP);
	if( (fd = open(sFullPath, O_CREAT|O_WRONLY|O_TRUNC, 0644)) == -1)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN open(%s) [errno:%d-%s]", 
			__FILE__, __FUNCTION__, __LINE__, FILE_FLT_SCTP, errno, strerror(errno));
		return -1;
	}

	for(i = 0; i < pstData->dCount; i++)
	{
		memset(sLineBuf, 0x00, BUF_SIZE);
		sprintf(sLineBuf, "\n( LPREA\n");
		szLineLen = strlen(sLineBuf);

		stAddr.s_addr = htonl(pstData->stSCTP[i].uSCTPIP);
		sprintf(&sLineBuf[szLineLen], "<k>ServerIP=%s\n", inet_ntoa(stAddr));
		szLineLen = strlen(sLineBuf);

		switch(pstData->stSCTP[i].cSysType)
		{
			case HSS_SYSTYPE:
				sprintf(&sLineBuf[szLineLen], "SysType=%s\n", "TYPE_HSS");
				break;
			case CSCF_SYSTYPE:
				sprintf(&sLineBuf[szLineLen], "SysType=%s\n", "TYPE_CSCF");
				break;
			default:
				log_print(LOGN_CRI, "F=%s:%s.%d: UNAVAILABLE SysType[%hu], Check FLT_SVR table", __FILE__, __FUNCTION__, __LINE__, pstData->stSCTP[i].cSysType);
				if(close(fd) == -1)
				{
					log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN close() [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, errno, strerror(errno));
					return -2;
				}
				return -3;
		}
		szLineLen = strlen(sLineBuf);

		switch(pstData->stSCTP[i].cDirection)
		{
			case 1:		/*	UP		*/
				sprintf(&sLineBuf[szLineLen], "Direction=%s\n", "DIR_UP");
				break;
			case 2:		/*	DOWN	*/
				sprintf(&sLineBuf[szLineLen], "Direction=%s\n", "DIR_DOWN");
				break;
			default:
				log_print(LOGN_CRI, "F=%s:%s.%d: UNAVAILABLE SysType[%hu], Check FLT_SVR table", __FILE__, __FUNCTION__, __LINE__, pstData->stSCTP[i].cDirection);
				if(close(fd) == -1)
				{
					log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN close() [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, errno, strerror(errno));
					return -4;
				}
				return -5;
		}
		szLineLen = strlen(sLineBuf);

		sprintf(&sLineBuf[szLineLen], "GroupID=%hu\n", pstData->stSCTP[i].huGroupID);
		szLineLen = strlen(sLineBuf);

		sprintf(&sLineBuf[szLineLen], ") LPREA\n");
		szLineLen = strlen(sLineBuf);

		if( (sszWrLen = write(fd, sLineBuf, szLineLen)) == -1)
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN write() [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, errno, strerror(errno));
			if(close(fd) == -1)
			{
				log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN close() [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, errno, strerror(errno));
				return -6;
			}
			return -7;
		}

		if(sszWrLen != szLineLen)
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: Didn't complete to write a file[%s] sszWrLen[%lu] szLineLen[%lu]", __FILE__, __FUNCTION__, __LINE__,
				"FLT_MNIP.dat", sszWrLen, szLineLen);
			if(close(fd) == -1)
			{
				log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN close() [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, errno, strerror(errno));
				return -8;
			}
			return -9;
		}
	}

	if(close(fd) == -1)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN close() [errno:%d-%s]", __FILE__, __FUNCTION__, __LINE__, errno, strerror(errno));
		return -10;
	}

	return 0;
}

int dReadTimerFile(TIMER_INFO *pstData)
{
	FILE			*fp;
	size_t			szLen;
	char			sFileName[PATH_MAX], sBuf[512];
	int				i;

	sprintf(sFileName, "%s", FILE_TIMER);
	if( (fp = fopen(sFileName, "r")) == NULL)
	{
		if( (fp = fopen(sFileName, "w")) == NULL)
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN fopen(%s) [errno:%d-%s]", 
				__FILE__, __FUNCTION__, __LINE__, sFileName, errno, strerror(errno));
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
			log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN FILE(%s) LINE(%d) FORMAT", __FILE__, __FUNCTION__, __LINE__, sFileName, i);
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
					log_print(LOGN_CRI, "F=%s:%s.%d: File[%s]'count is over maximum(%d).", __FILE__, __FUNCTION__, __LINE__, sFileName, 1);
					fclose(fp);
					return -3;
				}

				log_print(LOGN_INFO, "TIMERINFO: RPPI_CALL_TIMEOUT[%u] RPPI_WAIT_TIMEOUT[%u] PI_VT_TIMEOUT[%u] "
						"PI_IM_TIMEOUT[%u] PI_TCP_RSTWAIT[%u] PI_TCP_TIMEOUT[%u] PI_DNS_TIMEOUT[%u] "
						"PI_SIP_TIMEOUT[%u] PI_MSRP_TIMEOUT[%u] PI_RAD_TIMEOUT[%u] PI_DIA_TIMEOUT[%u] "
						"PI_CALL_TIMEOUT[%u] PI_WAIT_TIMEOUT[%u] PI_DORM_TIMEOUT[%u] RP_CALL_TIMEOUT[%u] "
						"RP_DORM_TIMEOUT[%u] PI_INET_TIMEOUT[%u] PI_RCALL_TIMEOUT[%u] RP_RCALL_TIMEOUT[%u]"
						"PI_RCALL_SIGWAIT[%u] RP_RCALL_SIGWAIT[%u]",
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
