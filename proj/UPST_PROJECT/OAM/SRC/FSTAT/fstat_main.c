/**
	@file		fstat_main.c
	@author
	@version
	@date		2011-07-26
	@brief		메인 소스 파일은 헤더파일 없음
*/

/**
 *	INCLUDE HEADERS
 */

/* SYS HEADER */
#include <mysql/mysql.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>			/* getpid(void) */
/* LIB HEADER */
#include "commdef.h"
#include "filedb.h"
#include "nifo.h"
#include "gifo.h"
#include "cifo.h"
#include "mems.h"
#include "dblib.h"
#include "loglib.h"
#include "verlib.h"
#include "nsocklib.h"
#include "filelib.h"
/* PRO HEADER */
#include "procid.h"
#include "path.h"
#include "mmcdef.h"
#include "msgdef.h"
#include "sshmid.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
#include "almstat.h"
#include "filedb.h"
/* LOC HEADER */
#include "fstat_init.h"

/**
 *	DECLARE VARIABLES
 */
int JiSTOPFlag;
int FinishFlag;

int gdSHMLOADID;
int gdSHMFAULTID;
int gdSHMTRAFFICID;

stMEMSINFO			*gpRECVMEMS;
stCIFO				*pCIFO;

unsigned long long timerNID;

st_SubSysInfoList   gstSubList;
st_LOADLIST         *pstSHMLOADTbl;
st_FAULTLIST        *pstSHMFAULTTbl;
short               gdTimeFlag;
time_t              tLocalTime;

stTIMERNINFO        *pTIMER;

char        		szIP[16], szName[32], szPass[32], szAlias[32];
st_ConnInfo			stConnInfo;
MYSQL				stMySQL;

/**
 *	DECLARE FUNCTIONS
 */
extern void SetUpSignal(void);
extern int dInit_ipcs(void);
extern void InitProc(void);
extern int dIsReceivedMessage(pst_MsgQ *pstMsgQ);
extern int dCheckNTAFLoadList(time_t tLocalTime, st_NTAF *stNTAF, short usNTAFID);
extern int dCheckNTAMLoadList(time_t tLocalTime, st_NTAM * stNTAM);
extern int dCheckFaultList(time_t tLocalTime, st_almsts * stAlmStatus, short usNTAFID);
extern int dCheckTrafficList(time_t tLocalTime, st_NtafStatList *stTRAFFIC);
extern void FinishProgram(void);

/**
 *	IMPLEMENT FUNCTIONS
 */

int main(void)
{
	int				dRet;
	pst_MsgQ		pstMsgQ;
	st_MsgQSub		*pstMsgQSub;
	st_NTAF			*pstNTAF;
	st_NTAM			*pstNTAM;
	//T_Alm_Status	*stFAULT;
	st_almsts		*stFAULT;
	st_NtafStatList	*stTRAFFIC;
	time_t			tCurTime, tLastPingMySQL;

	log_init(S_SSHM_LOG_LEVEL, getpid(), SEQ_PROC_FSTAT, LOG_PATH"/FSTAT", "FSTAT");
	/*
	AppLogInit(getpid(), SEQ_PROC_FSTAT, FSTAT_LOG, "FSTAT");

	if( (dRet = dInitLogShm()) <0)
	{
		log_print(LOGN_CRI, LH" ERROR IN dInitLogShm() dRet[%d]", LT, dRet);
		exit(-1);
	}

	if( (dRet = Init_shm_common()) <0)
	{
		log_print(LOGN_CRI, LH" ERROR IN Init_shm_common() dRet[%d]", LT, dRet);
		exit(-2);
	}
	*/

	SetUpSignal();

    if( (dRet = set_version(S_SSHM_VERSION, SEQ_PROC_FSTAT, "R4.0.0")) < 0)
	{
		log_print(LOGN_CRI, LH" ERROR IN set_version dRet[%d]",LT, dRet);
        return -1;
    }

	if((dRet = get_db_conf(FILE_MYSQL_CONF, szIP, szName, szPass, szAlias)) < 0)
	{
		log_print(LOGN_CRI, LH" ERROR IN get_db_conf() dRet[%d]", LT, dRet);
		exit(-5);
	}

	if((dRet = db_conn(&stMySQL, szIP, szName, szPass, szAlias)) < 0)
	{
			log_print(LOGN_CRI, LH" ERROR IN db_conn() dRet[%d]", LT, dRet);
		exit(-6);
	}

#if 0	
	if( (dRet = dInitialMysqlEnvironment(&stConnInfo)) < 0)
	{
		log_print(LOGN_CRI, LH" ERROR IN dInitialMysqlEnvironment() dRet[%d]", LT, dRet);
		exit(-5);
	}
	
	if( (dRet = dConnectMySQL(&stMySQL, &stConnInfo)) < 0)
	{
		log_print(LOGN_CRI, LH" ERROR IN dConnectMySQL() dRet[%d]", LT, dRet);
		exit(-6);
	}
#endif

    dRet = dInit_ipcs();
    if(dRet < 0){
        log_print(LOGN_CRI, "MAIN : FAILED IN dInit_IPC dRet[%d] ", dRet);
        return -3;
    }

	InitProc();

    log_print(LOGN_CRI, "MAIN : FSTAT PROCESS START ");

	while(JiSTOPFlag)
	{
		tLocalTime	= time(NULL)/300*300;
		timerN_invoke(pTIMER);
		if( (dRet = dIsReceivedMessage(&pstMsgQ)) < 0)
		{
			usleep(0);
			continue;
		}
		else
		{
			pstMsgQSub = (st_MsgQSub*)&pstMsgQ->llMType;
			log_print(LOGN_INFO,"MESSAGE RECV TYPE:[%d] SID:[%d] MID:[%d]", pstMsgQSub->usType, pstMsgQSub->usSvcID, pstMsgQSub->usMsgID);
			switch(pstMsgQSub->usType)
			{
				case DEF_SVC:
					switch(pstMsgQSub->usSvcID)
					{
						case SID_SVC:
							switch(pstMsgQSub->usMsgID)
							{
								case MID_STAT_LOAD:
									if(pstMsgQ->ucNTAFID > 0)
									{
										pstNTAF = (st_NTAF *)&pstMsgQ->szBody[0];
										dCheckNTAFLoadList(tLocalTime, pstNTAF, pstMsgQ->ucNTAFID);
									}
									else if(pstMsgQ->ucNTAFID == 0 && pstMsgQ->ucNTAMID == 0)
									{
										pstNTAM = (st_NTAM *)&pstMsgQ->szBody[0];
										dCheckNTAMLoadList(tLocalTime, pstNTAM);
									}
									else
										log_print(LOGN_WARN," FAIL SYSTYPE LOAD DATA");
									break;

								case MID_STAT_FAULT:
	 								//stFAULT = (T_Alm_Status *)&stMsgQ.szBody[0];
									stFAULT = (st_almsts *)&pstMsgQ->szBody[0];
									dCheckFaultList(tLocalTime, stFAULT, pstMsgQ->ucNTAFID);
									break;

								case MID_STAT_TRAFFIC:
	 								stTRAFFIC = (st_NtafStatList*)&pstMsgQ->szBody[0];
									dCheckTrafficList(tLocalTime, stTRAFFIC);
									break;

								default:
									log_print(LOGN_WARN, "FAIL NOT DEFINE MSGID[%d]",pstMsgQSub->usMsgID);
							}
							break;
						default:
							log_print(LOGN_WARN, "FAIL NOT DEFINE SVCID[%d]",pstMsgQSub->usSvcID);
					}
					break;
				default:
					log_print(LOGN_WARN, "FAIL NOT DEFINE TYPE[%d]",pstMsgQSub->usType);
			}
		}

		if( ((tCurTime=time(NULL)) - tLastPingMySQL) > SEC_OF_HOUR)
		{
			//if( (dRet = dPingMySQL(&stMySQL)) < 0)
			if((dRet = db_check_alive(&stMySQL)) < 0)
			{
				log_print(LOGN_CRI,LH" ERROR IN dPingMySQL() dRet[%d]", LT, dRet);
				return -16;
			}
			tLastPingMySQL = tCurTime;
		}
	}
    FinishProgram();

    return 0;
}
