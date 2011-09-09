/**A.1*  FILE INCLUSION *******************************************************/
#include <stdio.h>
#include <sys/time.h>
#include <linux/limits.h>	/*	PATH_MAX	*/
#include <sys/ipc.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <db_api.h>
#include <errno.h>

#include "s_mng_func.h"
#include "s_mng_flt.h"
#include "s_mng_init.h"
#include "s_mng_msg.h"			/* dIsReceivedMessage() */
#include "s_mng_mmc.h"			/* dHandleMMC() */

#include "path.h"
#include "procid.h"
#include "sshmid.h"
#include "msgdef.h"
#include "mems.h"
#include "cifo.h"
#include "nifo.h"
#include "gifo.h"

#include "sockio.h"
#include "filedb.h"				/* st_keepalive */

#include "loglib.h"
#include "verlib.h"
#include "ipclib.h"
#include "filelib.h"
#include "dblib.h"

/** B.1* DEFINITION OF NEW CONSTANTS **********************/
#define sVersion "R4.0.0"

/** C.1* DEFINITION OF NEW TYPES **************************/
/** D.1* DECLARATION OF VARIABLES *************************/
MYSQL						stMySQL;
st_ConnInfo					stConnInfo;
stMEMSINFO					*pMEMSINFO;
stCIFO						*gpCIFO;
st_MonTotal      			*gMonTotal;
st_MonTotal_1Min      		*gMonTotal1Min;
st_keepalive				*keepalive;
st_TraceList				*trace_tbl;
st_WatchFilter      		*gWatchFilter;

int							JiSTOPFlag;

extern st_subsys_mng		*pstSubSys;
extern st_Flt_Info			*flt_info;

extern int					gInfoAccessHour;
extern int					gInfoAccessMin;
extern int					gCTNInfoHour;
extern int					gCTNInfoMin;


/** E.1* DEFINITION OF FUNCTIONS **************************/


/** E.2* DEFINITION OF FUNCTIONS **************************/

int main(void)
{
	int				i, dRet, dIsCheckAccess, dIsCTNAccess;
	pst_MsgQ		pstMsgQ;
	pst_MsgQSub		pstMsgQSub;
	char			dbip[32], username[32], passwd[32], alias[32];
	/****************************************************************************************************************
		1분 간격으로 현재 trace_tbl의 내용중에 tExpiredTime이 지난 정보가 있는지 확인할 때 필요한 변수(I, tEachMin) 선언 추가
		 - Writer: Han-jin Park
		 - DAte: 2008.09.19
	****************************************************************************************************************/
	time_t			tCurTime, tEachMin, tLastPingMySQL;
	struct tm		stTmInfoAccess;

	/* SETTING TIMER */
	tCurTime	= time(NULL);

	log_init(S_SSHM_LOG_LEVEL, getpid(), SEQ_PROC_S_MNG, LOG_PATH"/S_MNG", "S_MNG");

	SetUpSignal();

	if((pMEMSINFO = nifo_init_zone((U8*)"S_MNG", SEQ_PROC_S_MNG, FILE_NIFO_ZONE)) == NULL ){
		log_print(LOGN_CRI, LH"nifo_init_zone NULL", LT);
		exit(-1);
	}

	if((gpCIFO = gifo_init_group(FILE_CIFO_CONF, FILE_GIFO_CONF)) == NULL ){
        log_print(LOGN_CRI, LH"FAILED IN gifo_init_group, cifo=%s, gifo=%s",
            LT, FILE_CIFO_CONF, FILE_GIFO_CONF);
		exit(-2);
    }

	if( shm_init(S_SSHM_FLT_INFO, sizeof(st_Flt_Info), (void**)&flt_info) < 0 ){
		log_print(LOGN_CRI,LH"ERROR IN shm_init(FLT_INFO=0x%x)"EH, LT, S_SSHM_FLT_INFO, ET);
		exit(-3);
	}

	if( shm_init(S_SSHM_SUBSYS, sizeof(st_subsys_mng), (void**)&pstSubSys) < 0 ){
		log_print(LOGN_CRI,LH"ERROR IN shm_init(SUBSYS=0x%x)"EH, LT, S_SSHM_SUBSYS, ET);
		exit(-4);
	}

	if( shm_init(S_SSHM_WATCH_FILTER, sizeof(st_WatchFilter), (void**)&gWatchFilter) < 0 ){
		log_print(LOGN_CRI,LH"ERROR IN shm_init(WATCH_FILTER=0x%x)"EH, LT, S_SSHM_WATCH_FILTER, ET);
		exit(-5);
	}

	gWatchFilter->stModelInfoList.dActiveStatus = 1;

	if( (dRet = dReadWatchFilter()) < 0)
		log_print(LOGN_CRI,LH"ERROR IN dReadWatchFilter() dRet[%d]", LT, dRet);

	if( shm_init(S_SSHM_MON_TOTAL, sizeof(st_MonTotal), (void**)&gMonTotal) < 0 ){
		log_print(LOGN_CRI,LH"ERROR IN shm_init(MON_TOTAL=0x%x)"EH, LT, S_SSHM_MON_TOTAL, ET);
		exit(-6);
	}

	if( shm_init(S_SSHM_MON_TOTAL_1MIN, sizeof(st_MonTotal_1Min), (void**)&gMonTotal1Min) < 0 ){
		log_print(LOGN_CRI,LH"ERROR IN shm_init(MON_TOTAL_1Min=0x%x)"EH, LT, S_SSHM_MON_TOTAL_1MIN, ET);
		exit(-7);
	}

	if( shm_init(S_SSHM_KEEPALIVE, sizeof(st_keepalive), (void**)&keepalive) < 0 ){
		log_print(LOGN_CRI,LH"ERROR IN shm_init(KEEPALIVE=0x%x)"EH, LT, S_SSHM_KEEPALIVE, ET);
		exit(-8);
	}

    if( (dRet = set_version(S_SSHM_VERSION, SEQ_PROC_S_MNG, sVersion)) < 0)
	{
        log_print(LOGN_WARN, LH"ERROR IN set_version() dRet[%d]", LT, dRet);
        exit(-10);
    }

	if( (dRet = get_db_conf(FILE_MYSQL_CONF, dbip, username, passwd, alias)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dInitialMysqlEnvironment() dRet[%d]", LT, dRet);
		exit(-11);
	}

	if( (dRet = db_conn(&stMySQL, dbip, username, passwd, alias)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dConnectMySQL() dRet[%d]", LT, dRet);
		exit(-12);
	}

	if( (dRet = dInit_IPC()) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dInit_IPC() dRet[%d]", LT, dRet);
		exit(-13);
	}

	if( (dRet = dMakeIRMHash()) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dMakeIRMHash() dRet[%d]", LT, dRet);
		exit(-14);
	}

	/*	파일에서 SYSNO, DB확인	*/
	if( (dRet = dInitSysConfig()) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dInitSysConfig() dRet[%d]", LT, dRet);
		exit(-15);
	}

	if( (dRet = dInit_Info()) < 0)
		log_print(LOGN_CRI, LH"ERROR IN dInit_Info() dRet=%d", LT, dRet);

	if( (dRet = dWriteWatchFilter(FILE_WATCHFILTER)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dWriteWatchFilter() dRet[%d]", LT, dRet);
		if( (dRet = remove(FILE_WATCHFILTER)) == -1)
		{
			log_print(LOGN_CRI, LH"FAILED IN remove(%s) errno[%d-%s]", LT, FILE_WATCHFILTER, errno, strerror(errno));
			exit(-16);
		}
		exit(-17);
	}

	log_print(LOGN_CRI, LH"S_MNG(%s) PROCESS START!!", LT, sVersion);

	tEachMin = ((time(NULL) / SEC_OF_MIN) + 1) * SEC_OF_MIN;
	while(JiSTOPFlag)
 	{
		/****************************************************************************************************************
			1분 간격으로 현재 trace_tbl의 내용중에 tExpiredTime이 지난 정보가 있으면, trace를 종료하도록 처리
				- Writer: Han-jin Park
				- DAte: 2008.09.19
		****************************************************************************************************************/
		if( (tCurTime = time(NULL)) >= tEachMin)
		{
			for(i = 0; i < trace_tbl->count; i++)
			{
				log_print(LOGN_DEBUG, LH"trace_tbl->stTraceInfo[%02d].tExpiredTime[%d]", LT,
					i, trace_tbl->stTraceInfo[i].tExpiredTime);
				if(trace_tbl->stTraceInfo[i].tExpiredTime <= tEachMin)
					stop_trc_info(trace_tbl, i);
			}
			tEachMin = ( (tCurTime / SEC_OF_MIN) + 1) * SEC_OF_MIN;
		}

		if( (tCurTime - tLastPingMySQL) > SEC_OF_HOUR)
		{
			if( (dRet = db_check_alive(&stMySQL)) < 0)
			{
				log_print(LOGN_CRI,LH"ERROR IN dPingMySQL() dRet[%d]", LT, dRet);
				return -18;
			}
			tLastPingMySQL = tCurTime;
		}

		/*	하루 1회 SYS_CFG의 INFOACCESS 의 구성 정보의 시간과 분이 일치하는 시간에 TB_MACCESSC 테이블에서 Filter 정보를 갱신한다.	*/
		localtime_r(&tCurTime, &stTmInfoAccess);
		if( (dIsCheckAccess==0) && ((stTmInfoAccess.tm_hour==gInfoAccessHour)&&(stTmInfoAccess.tm_min==gInfoAccessMin)))
		{
			if( (dRet = dInit_WatchInfoAccess()) < 0)
			{
				log_print(LOGN_CRI,LH"ERROR IN dInit_WatchInfoAccess() dRet[%d]", LT, dRet);
				return -19;
			}
			else
				dIsCheckAccess	= 1;
		}
		else if( (stTmInfoAccess.tm_hour!=gInfoAccessHour)&&(stTmInfoAccess.tm_min!=gInfoAccessMin))
			dIsCheckAccess	= 0;

		if( (dIsCTNAccess==0) && ((stTmInfoAccess.tm_hour==gCTNInfoHour)&&(stTmInfoAccess.tm_min==gCTNInfoMin)))
		{
			if( (dRet = dInit_CTNInfo()) < 0)
				log_print(LOGN_CRI,LH"ERROR IN dInit_CTNInfo() dRet[%d]", LT, dRet);
			else
				dIsCTNAccess	= 1;
		}
		else if( (stTmInfoAccess.tm_hour!=gCTNInfoHour)&&(stTmInfoAccess.tm_min!=gCTNInfoMin))
			dIsCTNAccess	= 0;

		if( (dRet = dIsReceivedMessage(&pstMsgQ)) < 0)
		{
			log_print(LOGN_CRI, LH"FAIL IN dIsReceivedMessage() dRet[%d]", LT, dRet);
			exit(-20);
		}
		else if(dRet == 100)
			continue;
		else
		{
			pstMsgQSub	= (pst_MsgQSub)&pstMsgQ->llMType;
			log_print(LOGN_DEBUG, LH"Type=%d SvcID=%d MSG=%d INDEX=%lld", LT,
				pstMsgQSub->usType, pstMsgQSub->usSvcID, pstMsgQSub->usMsgID, pstMsgQ->llIndex);
			switch(pstMsgQSub->usType)
			{
				case DEF_SYS:
					switch(pstMsgQSub->usSvcID)
					{
						case SID_MML:
							dRet = dHandleMMC(pstMsgQ);
							break;
						case SID_FLT:/* FLT_NTAF 공유메모리를 읽는다. */
							dRet = dReadFltSHM(pstMsgQ);
							break;
						case SID_GFLT:/* FLT_GDGS  공유메모리에 쓴다. */
							dRet = dWriteFltSHM(pstMsgQ);
							break;
						case SID_CHKRES: /* NTAF에서 올라온것*/
							dRet = dResChkInfo(pstMsgQ );
							break;
						default:
							log_print(LOGN_CRI, LH"NOT SUPPORT SVCID Type=%d SvcID=%d MSG=%d INDEX=%lld", LT,
								pstMsgQSub->usType, pstMsgQSub->usSvcID, pstMsgQSub->usMsgID, pstMsgQ->llIndex);
							break;
					}
					break;
				default:
					log_print(LOGN_CRI, LH"NOT SUPPORT SYSTYPE Type=%d SvcID=%d MSG=%d INDEX=%lld", LT,
						pstMsgQSub->usType, pstMsgQSub->usSvcID, pstMsgQSub->usMsgID, pstMsgQ->llIndex);
					break;
			} /* Switch-end */
		}
	} /* while-loop end */
	FinishProgram(&stMySQL);

	return 0;
} /* end of main */
