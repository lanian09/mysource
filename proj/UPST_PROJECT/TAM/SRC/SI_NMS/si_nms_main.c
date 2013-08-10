/*******************************************************************************
			DQMS Project

	Author   : Jae Seung Lee
	Section  : SI_NMS
	SCCS ID  : @(#)si_nms_main.c	1.1
	Date     : 01/21/05
	Revision History :
        '05. 01. 21		Initial
		'08. 01. 07		Update By LSH for review
		'08. 01. 14		Add By LSH for IUPS NTAM

	Description :

	Copyright (c) uPRESTO 2005
*******************************************************************************/

/***** A.1 * File Include *******************************/
#include <unistd.h>				/*	getpid(2)	*/
#include <stdlib.h>				/*	exit(3)		*/
#include <errno.h>				/*	errno(3)	*/
#include <mysql/mysql.h>		/*	MYSQL		*/

// LIB
#include "typedef.h"
#include "loglib.h"
#include "verlib.h"
#include "dblib.h"
#include "filelib.h"
#include "mems.h"
#include "cifo.h"

// OAM
#include "almstat.h"

// DQMS
#include "procid.h"
#include "sshmid.h"
#include "path.h"
#include "msgdef.h"
#include "timesec.h"

// .
#include "si_nms_comm.h"

/** B.1* DEFINITION OF NEW CONSTANTS **********************/
/** C.1* DEFINITION OF NEW TYPES **************************/
/** .1* DECLARATION OF VARIABLES *************************/
char				vERSION[7] = "R4.0.0";

int					gJiSTOPFlag;
time_t				tvLastCkTime;

st_NMSIPList		gstNMSIPList;
st_NMSPortInfo		gstNMSPortInfo;
st_NMSSFdInfo		gstNMSSFdInfo[MAX_CONN];

char        		szIP[16], szName[32], szPass[32], szAlias[32];
MYSQL				stMySQL;
st_ConnInfo			stConnInfo;

stMEMSINFO			*pMEMSINFO;
stCIFO				*pCIFO;


/** E.1* DEFINITION OF FUNCTIONS **************************/
/** E.2* DEFINITION OF FUNCTIONS **************************/
extern int dInitProc(stMEMSINFO **pMEMSINFO, stCIFO **pCIFO);												/*	SRC/SI_NMS/si_nms_init.c	*/
extern void FinishProgram(void);																			/*	SRC/SI_NMS/si_nms_init.c	*/
extern int dInitSockFd(st_SelectInfo *stFD, int dListenIdx);												/*	SRC/SI_NMS/si_nms_sock.c	*/
extern int Check_ClientEvent(st_NMSSFdInfo *stSock, st_SelectInfo *stFD);									/*	SRC/SI_NMS/si_nms_sock.c	*/
extern int dIsReceivedMessage(pst_MsgQ pstMsgQ);															/*	SRC/SI_NMS/si_nms_func.c	*/
extern int dHandleFSTATMsgQ(st_NMSSFdInfo *stSock, st_atQueryInfo *pstAtQueryInfo, st_SelectInfo *stFD);	/*	SRC/SI_NMS/si_nms_sock.c	*/
extern int dHandleCONDMsgQ(st_NMSSFdInfo *stSock, st_SelectInfo *stFD, char *psData, size_t szStrLen);		/*	SRC/SI_NMS/si_nms_sock.c	*/
extern int dHandleALMDMsgQ(st_NMSSFdInfo *stSock, st_SelectInfo *stFD, st_almsts *pstAlmStatus);			/*	SRC/SI_NMS/si_nms_sock.c	*/
extern int dInitFidb(void);			/* si_nms_init.c */

int main(void)
{
	int				i, dRet;
	time_t			tLastTime, tCurTime, tLastPingMySQL;
	st_atQueryInfo	stAtQueryInfo;
	st_almsts		stAlmStatus;
	st_MsgQ			stMsgQ;
	pst_MsgQSub		pstMsgQSub;
	st_SelectInfo	stFD;

	// Initialize log
	dRet = log_init(S_SSHM_LOG_LEVEL, getpid(), SEQ_PROC_SI_NMS, LOG_PATH"/SI_NMS", "SI_NMS");
	if(dRet < 0)
	{
		log_print(LOGN_WARN, LH"MAIN : Failed in Initialize LOGLIB Info [%d]", LT,  dRet);
		exit(-1);
	}

	// Set version
	if( (dRet = set_version(S_SSHM_VERSION, SEQ_PROC_SI_NMS, vERSION)) < 0)
	{
		log_print(LOGN_WARN, LH"MAIN : Failed in Initialize Version Info [%d]", LT,  dRet);
		exit(-2);
	}

	// Initialize FIDB 
	// TODO dInitFidb 안에서 공유메모리 초기화를 해야함
	if( (dRet = dInitFidb()) < 0)
    {
        log_print(LOGN_CRI, LH"ERROR IN dInitFidb() dRet[%d]", LT, dRet);
        exit(-3);
    }

	// MySQL conf
	if((dRet = get_db_conf(FILE_MYSQL_CONF, szIP, szName, szPass, szAlias)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN get_db_conf() dRet[%d]", LT, dRet);
		exit(-4);
	}

	// Creat connection for MySQL
	if((dRet = db_conn(&stMySQL, szIP, szName, szPass, szAlias)) < 0)
	{
			log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN db_conn() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		exit(-5);
	}

	// Initialize GIFO */
	if( (dRet = dInitProc(&pMEMSINFO, &pCIFO)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dInitProc(2) dRet[%d]", LT, dRet);
		exit(-6);
	}

	memset(&stFD, 0x00, sizeof(st_SelectInfo));

	for(i = 0; i < PORT_IDX_MAX; i++)
	{
		if( (dRet = dInitSockFd(&stFD, i)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dInitSockFd(%d[%d]) dRet[%d]", LT, gstNMSPortInfo.port[i], i, dRet);
			exit(-7);
		} /* end of if */
	}

	for(i = PORT_IDX_MAX; i< MAX_CONN; i++)
	{
		gstNMSSFdInfo[i].tLastTime			= 0;
		gstNMSSFdInfo[i].cMask				= 0;
		gstNMSSFdInfo[i].cLevel				= 0;
		gstNMSSFdInfo[i].dSfd				= 0;
		gstNMSSFdInfo[i].dListenPort		= 0;
		gstNMSSFdInfo[i].uIPAddr			= 0;
		gstNMSSFdInfo[i].dType				= FD_TYPE_DATA;
		gstNMSSFdInfo[i].dWriteStartPos		= 0;
		gstNMSSFdInfo[i].dWriteEndPos		= 0;
		gstNMSSFdInfo[i].dLastFlag			= 0;
	}

	log_print(LOGN_CRI, LH"SI_NMS PROCESS INITIAL SUCCESS", LT);

	/* PROCESS JOB START */
	tLastTime = time(NULL);
	while(gJiSTOPFlag)
	{
		memset(&stMsgQ, 0x00, DEF_MSGQ_SIZE);
		if( (dRet = dIsReceivedMessage(&stMsgQ)) < 0)
			sleep(0);
		else
		{
			pstMsgQSub	= (pst_MsgQSub)&stMsgQ.llMType;
			log_print(LOGN_DEBUG, LH"Type=%d SvcID=%d MSG=%d INDEX=%lld", LT,
				pstMsgQSub->usType, pstMsgQSub->usSvcID, pstMsgQSub->usMsgID, stMsgQ.llIndex);
			switch(pstMsgQSub->usType)
			{
				case DEF_SVC:
					switch(pstMsgQSub->usSvcID)
					{
						case SID_SVC:
							switch(pstMsgQSub->usMsgID)
							{
								case MID_LOG_STATISTICS:
									memcpy(&stAtQueryInfo, stMsgQ.szBody, DEF_ATQUERYINFO_SIZE);
									if( (dRet = dHandleFSTATMsgQ(gstNMSSFdInfo, &stAtQueryInfo, &stFD)) < 0)
										log_print(LOGN_CRI, LH"ERROR IN dHandleFSTATMsgQ() dRet[%d]", LT, dRet);
									else
									{
										log_print(LOGN_DEBUG, LH"SUCCESS IN dHandleFSTATMsgQ() cPeriod[%s] tStartTime[%lu] tEndTime[%lu]", LT,
											(stAtQueryInfo.cPeriod == STAT_PERIOD_5MIN)?"STAT_PERIOD_5MIN":"STAT_PERIOD_HOUR",
											stAtQueryInfo.tStartTime, stAtQueryInfo.tEndTime);
									}
									break;
								case MID_LOG_CONSOLE:
									if( (dRet = dHandleCONDMsgQ(gstNMSSFdInfo, &stFD, stMsgQ.szBody, stMsgQ.usBodyLen)) < 0)
										log_print(LOGN_CRI, LH"ERROR IN dHandleCONDMsgQ() dRet[%d]", LT, dRet);
									else
									{
										log_print(LOGN_DEBUG, LH"SUCCESS IN dHandleCONDMsgQ() ConsoleMsg[%d-%s]", LT,
											stMsgQ.usBodyLen, stMsgQ.szBody);
									}
									break;
								case MID_LOG_ALARM:
									memcpy(&stAlmStatus, stMsgQ.szBody, sizeof(st_almsts));
									if( (dRet = dHandleALMDMsgQ(gstNMSSFdInfo, &stFD, &stAlmStatus)) < 0)
										log_print(LOGN_CRI, LH"ERROR IN dHandleALMDMsgQ() dRet[%d]", LT, dRet);
									else
									{
										log_print(LOGN_DEBUG, LH"ucLocType[%hu] ucSysType[%hu] ucSysNo[%hu] ucInvType[%hu]"
											"ucInvNo[%hu] ucAlmLevel[%hu] ucOldAlmLevel[%hu] tWhen[%lu] "
											"uiIPAddr[%u] llLoadVal[%lld]", LT,
											stAlmStatus.ucLocType, stAlmStatus.ucSysType, stAlmStatus.ucSysNo, stAlmStatus.ucInvType,
											stAlmStatus.ucInvNo, stAlmStatus.ucAlmLevel, stAlmStatus.ucOldAlmLevel, stAlmStatus.tWhen,
											stAlmStatus.uiIPAddr, stAlmStatus.llLoadVal);
									}
									break;
								default:
									break;
							}
							break;
						default:
							log_print(LOGN_CRI, LH"NOT SUPPORT SVCID Type=%d SvcID=%d MSG=%d INDEX=%lld", LT,
								pstMsgQSub->usType, pstMsgQSub->usSvcID, pstMsgQSub->usMsgID, stMsgQ.llIndex);
							break;
					}
					break;
				default:
					log_print(LOGN_CRI, LH"NOT SUPPORT SYSTYPE Type=%d SvcID=%d MSG=%d INDEX=%lld", LT,
						pstMsgQSub->usType, pstMsgQSub->usSvcID, pstMsgQSub->usMsgID, stMsgQ.llIndex);
					break;
			}
		}

		if( (dRet = Check_ClientEvent(gstNMSSFdInfo, &stFD)) < 0)
			log_print(LOGN_CRI, LH"ERROR IN Check_ClientEvent() dRet[%d]", LT, dRet);

		if( (tCurTime - tLastPingMySQL) > SEC_OF_HOUR)
		{
			if((dRet = db_check_alive(&stMySQL)) < 0)
			{
				log_print(LOGN_CRI,LH"ERROR IN db_check_alive() dRet[%d]", LT, dRet);
				exit(-9);
			}
			tLastPingMySQL = tCurTime;
		}
	}
	FinishProgram();

	return 0;
}
