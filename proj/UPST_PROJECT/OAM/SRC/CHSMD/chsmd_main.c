/**A.1 * File Include *************************************/

/* SYS HEADER */
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>		/* EXIT(3) */
#include <pthread.h>
#include <unistd.h>		/* SLEEP(3), USLEEP(3) */
#include <string.h>		/* memset(), strerror(), memcpy() */
/* LIB HEADER */
#include "filedb.h"
#include "loglib.h"
#include "ipclib.h"
#include "verlib.h"
/* PRO HEADER */
#include "path.h"
#include "sshmid.h"
#include "msgdef.h"
#include "mmcdef.h"		/* mml_msg */
#include "sockio.h"
#include "procid.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
#include "almstat.h"
/* LOC HEADER */
#include "chsmd_init.h"	/* FinishProgram(), dGetSYSCFG() */
#include "chsmd_cpu.h"	/* st_cpu_state */
#include "chsmd_mem.h"	/* st_mem_state, mem_compute() */
#include "chsmd_disk.h"	/* st_df_total, df_compute() */
#include "chsmd_link.h"	/* st_linkdev, Link_Check(), dChnl_Chk() */
#include "chsmd_ntp.h"	/* CheckNTPStS() */
#include "chsmd_hw.h"	/* st_PthrdDirect, dCheckHW() */
#include "chsmd_sw.h"	/* dCheckMySQLD(), check_software() */
#include "chsmd_mmc.h"	/* MMC_Handle_Proc() */
#include "chsmd_msg.h"	/* st_qentry( defined st_Qentry in ipclib.h ) */
#include "chsmd_mask.h"	/* dWriteMaskInfo() */
#include "chsmd_func.h"	/* dMsgrcv() */

/**B.1*  Definition of New Constants **********************/
/**B.2*  Definition of New Type  **************************/
/**C.1*  Declaration of Variables  ************************/
st_subsys_mng 	stSubSysMng;
char 			STR_TSW_COM[MAX_SW_COUNT][30];
int forkcnt, old_forkcnt;
int StopFlag	= 1;
int gdSysNo		= 0;
int dCurrBlockCnt;

/*** 각각의 값들을 각각의 module handler 로 옮겨야 함 */
st_cpu_state gcpu;
st_mem_state gmem;
st_df_total  gdf;
st_linkdev   stLinkDev[MAX_LINK_COUNT];

st_PthrdDirect	stPthrdDir[MAX_DIRECT_COUNT];
st_PthrdDirect	stPthrdSW[MAX_SWITCH_COUNT];

char vERSION[7] = "R4.0.0";

/**C.2*  Declaration of External Variables ************/
pst_NTAM			fidb;
pst_DIRECT_MNG		director;
pst_SWITCH_MNG		swch;
pst_keepalive       keepalive;

/**D.1*  Definition of Functions  *********************/
/**D.2*  Definition of External Functions  ************/
/*******************************************************************************
 MAIN FUNCTION
*******************************************************************************/
int main(void)
{
	time_t			now, sw_set, dog_set, fork_set, db_set, hw_set;
	st_qentry		recvmesg;

	long			*pmsg;
	int				i, dRet, dSysType, *dResult, dPortIndex, dOldInterLock[MAX_ICH_COUNT], maskflag;
#if 0
	int             oldMirrorSts[MAX_PORT_COUNT], oldMirrorActsts[MAX_PORT_COUNT]; //TAF ONLY - ENDACE CARD PORT
#endif

	mml_msg			*ml;
	pst_MsgQ		pstMsg;
	st_MsgQSub		stMsgQSub;
	st_SoldfList	stSoldfList;

	db_set			= 0;
	hw_set			= 0;
	dSysType		= 0;

	log_init(S_SSHM_LOG_LEVEL, getpid(), SEQ_PROC_CHSMD, LOG_PATH"/CHSMD", "CHSMD");

	signal_handling();

	if( (dRet = dGetBlocks(FILE_MC_INIT, STR_TSW_COM)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dGetBlocks(%s) dRet[%d]", LT, FILE_MC_INIT, dRet);
		exit(-4);
	}
	else
		dCurrBlockCnt = dRet;

	if( (dRet = set_version(S_SSHM_VERSION, SEQ_PROC_CHSMD, vERSION)) < 0){
		log_print(LOGN_CRI, LH"ERROR IN set_version(prc_idx[%d],ver[%s]) dRet[%d]", LT, SEQ_PROC_CHSMD, vERSION, dRet);
		exit(-17);
	}

	forkcnt		= 0;
	old_forkcnt	= 0;

	if( (dRet = dInitProcess()) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dInitProcess() dRet[%d]", LT, dRet);
		exit(-5);
	}

	memset(&stSoldfList, 0x00, sizeof(st_SoldfList));
	if( (dRet = dGetSYSCFG(&stSoldfList)) < 0)
	{
		log_print(LOGN_CRI, LH"ERROR IN dGetSYSCFG() dRet[%d]", LT, dRet);
		exit(-6);
	}
	else
		gdSysNo = dRet;

	log_print(LOGN_DEBUG, LH"SUCCESS INITIALIZE CHSMD [%d] [%d]\n", LT, dRet, getpid());

	now = time(&now);

	fidb->mpsw[SEQ_PROC_CHSMD]      = STOP;
	fidb->mpswinfo[SEQ_PROC_CHSMD].pid = 0;

	log_print(LOGN_CRI, LH"CHAMD START VER=%s FIRST STATUS[%d] PID[%lld] WHEN[%lld]", LT,
		vERSION,
		fidb->mpsw[SEQ_PROC_CHSMD],
		fidb->mpswinfo[SEQ_PROC_CHSMD].pid,
		fidb->mpswinfo[SEQ_PROC_CHSMD].when);

	for(i = 0; i < MAX_DIRECT_COUNT; i++)
	{
		pthread_mutex_init(&stPthrdDir[i].PthrdMutex, NULL);
		stPthrdDir[i].cStop			= 0;
		stPthrdDir[i].dArg			= i;

		if( (dRet = pthread_create(&stPthrdDir[i].PthrdDirID, NULL, CheckDirector_init, (void*)&stPthrdDir[i].dArg)) != 0)
		{
			log_print(LOGN_CRI, LH"THREAD[%d] FAILED IN pthread_create(PthrdDirID)"EH, LT, i, ET);
			exit(-7);
		}
		else
			log_print(LOGN_DEBUG, LH"THREAD[%d] SUCCESS IN pthread_create(PthrdDirID[%lu])",
				LT, i, stPthrdDir[i].PthrdDirID);

		stPthrdDir[i].tLast	= now;
	}

	for(i = 0; i < MAX_SWITCH_COUNT; i++)
	{
		pthread_mutex_init(&stPthrdSW[i].PthrdMutex, NULL);
		stPthrdSW[i].cStop			= 0;
		stPthrdSW[i].dArg			= i;

		if( (dRet = pthread_create(&stPthrdSW[i].PthrdDirID, NULL, CheckSwitch_init, (void*)&stPthrdSW[i].dArg)) != 0)
		{
			log_print(LOGN_CRI, LH"THREAD[%d] FAILED IN pthread_create(PthrdDirID)"EH, LT, i, ET);
			exit(-8);
		}
		else
			log_print(LOGN_DEBUG, LH"THREAD[%d] SUCCESS IN pthread_create(PthrdDirID[%lu])",
				LT, i, stPthrdSW[i].PthrdDirID);

		stPthrdSW[i].tLast	= now;
	}

	fork_set	= now;
	dog_set		= now;
	sw_set		= now;

	/***************************************************************************
	 MAIN WHILE LOOP START
	***************************************************************************/
	while (StopFlag)
	{
		if( (dRet = IsRcvedMessage(&pstMsg)) > 0)
		{
			memcpy(&stMsgQSub, &pstMsg->llMType, sizeof(stMsgQSub));
			log_print(LOGN_DEBUG, LH"[MTYPE] [0x%02x]", LT, (unsigned int)pstMsg->llMType);
			recvmesg.mtype = pstMsg->llMType;
			if( (recvmesg.mtype != KillMC) && (recvmesg.mtype != StartMC))
				memcpy(recvmesg.mtext, &pstMsg->szBody[0], sizeof(mml_msg));
			else
			{
				memcpy(recvmesg.mtext, &pstMsg->szBody[sizeof(long)], sizeof(long)*3);
				pmsg = (long*)&recvmesg;
				log_print(LOGN_DEBUG, LH"[TIFB] [%ld] [%ld] [%ld] [%ld]", LT, recvmesg.mtype, pmsg[1], pmsg[2], pmsg[3]);
			}
		}
		else{
                        recvmesg.mtype = NO_MSG;
                        usleep(0);
                }

		switch(recvmesg.mtype)
		{
			case KillMC:
				pmsg = (long*)&recvmesg;
				if( (pmsg[1] > 0) && (pmsg[1] < dCurrBlockCnt))
				{
					if( (fidb->mpsw[pmsg[1]] == NORMAL) && (fidb->mpswinfo[pmsg[1]].pid != 0))
						Send_CondMess(SYSTYPE_TAM, LOCTYPE_PROCESS, INVTYPE_USERPROC, pmsg[1], STOP, fidb->mpsw[pmsg[1]]);

					if(fidb->mpsw[pmsg[1]] &= MASK)
						fidb->mpsw[pmsg[1]] = MASK;
					else
						fidb->mpsw[pmsg[1]] = STOP;

					fidb->mpswinfo[pmsg[1]].pid	= 0;
					fidb->mpswinfo[pmsg[1]].when	= now;
				}
				log_print(LOGN_CRI, LH"KillMC -b %s(%ld) : pid=%ld when=%ld", LT, STR_TSW_COM[pmsg[1]], pmsg[1], pmsg[2], pmsg[3]);
				write_FIDB();
				break;

			case StartMC:
				pmsg = (long *)&recvmesg;
				log_print(LOGN_DEBUG, LH"ID[%ld] STATUS[0x%02x]>[0x%02x] PID[%ld]", LT, pmsg[1], fidb->mpsw[pmsg[1]], NORMAL, pmsg[2]);
				if( (pmsg[1] >= 0) && (pmsg[1] < dCurrBlockCnt))
				{
					if( ((fidb->mpsw[pmsg[1]] == STOP) || (fidb->mpsw[pmsg[1]] == CRITICAL)) && (fidb->mpswinfo[pmsg[1]].pid == 0))
						Send_CondMess(SYSTYPE_TAM, LOCTYPE_PROCESS, INVTYPE_USERPROC, pmsg[1], NORMAL, fidb->mpsw[pmsg[1]]);

					if(fidb->mpsw[pmsg[1]] &= MASK)
						fidb->mpsw[pmsg[1]] = MASK;
					else if(fidb->mpsw[pmsg[1]] == NOT_EQUIP)
					{
/*** TAM /TAF 통합시 고려되어야 함 ****/
						Send_CondMess(SYSTYPE_TAM, LOCTYPE_PROCESS, INVTYPE_USERPROC, pmsg[1], NORMAL, CRITICAL);
						fidb->mpsw[pmsg[1]] = NORMAL;
					}
					else
						fidb->mpsw[pmsg[1]] = NORMAL;

					fidb->mpswinfo[pmsg[1]].pid	= pmsg[2];
					fidb->mpswinfo[pmsg[1]].when	= now;
				}
				log_print(LOGN_CRI, LH"StartMC -b %s(%ld) : pid=%ld when=%ld", LT, STR_TSW_COM[pmsg[1]], pmsg[1], pmsg[2], pmsg[3]);
				write_FIDB();
				break;

			case NO_MSG:
				break;

			default:
				log_print(LOGN_DEBUG, LH"MMC_Handle [%ld]", LT, recvmesg.mtype);
				ml = (mml_msg*)&recvmesg.mtext[0];
				log_print(LOGN_DEBUG, LH"[stMag BODY][%d]", LT, pstMsg->usBodyLen);
#if 0 /* 아무리 봐도 이상함..... para_id == 69 인 값을 찾을 수가 없음 , 있긴 하지만...엄한 IMSI 를 의미함...ㅜㅡ */
				for(i = 0; i < ml->num_of_para; i++)
				{
					log_print(LOGN_DEBUG, "PARA [%d]:[%s]", ml->msg_body[i].para_id, ml->msg_body[i].para_cont);
					if(ml->msg_body[i].para_id == 69)	/* SUBSYS */
						dSysType = atoi(ml->msg_body[i].para_cont);
				}

				if( (ml->msg_id == MI_ACT_PRC) || (ml->msg_id == MI_DACT_PRC) || (ml->msg_id == MI_MASK_PRC) ||
					(ml->msg_id == MI_UMASK_PRC) || (ml->msg_id == MI_DIS_PRC_STS))
				{
					if(dSysType == 50)
						MMC_Handle_Proc( (mml_msg*)recvmesg.mtext, recvmesg.mtype);
				}
				else
					MMC_Handle_Proc( (mml_msg*)recvmesg.mtext, recvmesg.mtype);
#endif /**********************************************************************************************************/
				MMC_Handle_Proc( (mml_msg*)recvmesg.mtext, recvmesg.mtype);
				write_FIDB();
				break;
		}

		now = time(&now);
		/***********************************************************************
	 	CHECK SOFTWARE PER 1 SECOND
		***********************************************************************/
		if(abs(now - sw_set) > PROC_CHECK_LIMIT)
		{
			check_software();
			signal_handling();

			/*******************************************************************
			CHECK HARDWARE STATUS
			*******************************************************************/
			cpu_compute(&gcpu);
			mem_compute(&gmem);
			df_compute(&gdf, &stSoldfList);

			/* FOR DEBUG ***************************************************************************/
			log_print(LOGN_INFO, "[CPU   ]: [%8lld] [%10lld] [%5.1f]", fidb->cpusts.llCur, fidb->cpusts.lMax, ((float)fidb->cpusts.llCur/(float)fidb->cpusts.lMax)*100.0);
			log_print(LOGN_INFO, "[MEMORY]: [%8lld] [%10lld] [%5.1f]", fidb->memsts.llCur, fidb->memsts.lMax, ((float)fidb->memsts.llCur/(float)fidb->memsts.lMax)*100.0);
			log_print(LOGN_INFO, "[QUEUE ]: [%8lld] [%10lld] [%5.1f]", fidb->queuests.llCur, fidb->queuests.lMax, ((float)fidb->queuests.llCur/(float)fidb->queuests.lMax)*100.0);
			log_print(LOGN_INFO, "[NIFO  ]: [%8lld] [%10lld] [%5.1f]", fidb->nifosts.llCur, fidb->nifosts.lMax, ((float)fidb->nifosts.llCur/(float)fidb->nifosts.lMax)*100.0);

			for(i = 0; i < MAX_DISK_COUNT; i++)
			{
				if(fidb->disksts[i].lMax != 0)
				{
					log_print(LOGN_INFO,"[DISK(%d)] : [%8lld] [%10lld] [%5.1f] [%-7s]",
						i, fidb->disksts[i].llCur, fidb->disksts[i].lMax,
						((float)fidb->disksts[i].llCur/(float)fidb->disksts[i].lMax)*100.0, gdf.stData[i].szName);
				}
			}
			/*****************************************************************************************/
			signal_handling();
			sw_set = now ;
		}

		/*
		*	HARD WARE
		*/
		if(abs(now-hw_set) > HW_CHECK_LIMIT)
		{
			if( (dRet = Link_Check()) < 0)
				log_print( LOGN_CRI, LH"ERROR IN Link_Check() dRet=%d", LT, dRet);

			CheckNTPStS();
			dChnl_Chk();	/* TAF 에서는 TAM Channel 관리 */
			dCheckHW();
			signal_handling();
			hw_set	= now;
		}

		/*	Check DB Channel	*/
		if(abs(now-db_set) > DB_CHECK_LIMIT) {
			dCheckMySQLD();
			signal_handling();
			db_set	= now;
		}

// FOR TAF
#if 0
		log_print(LOGN_DEBUG,"MIRROR PORT CHECK");
		for( i = 0; i < MAX_PORT_COUNT; i++ ){

			if( oldMirrorSts[i] && oldMirrorSts[i] != fidb->mirrorsts[i] ){
				Send_CondMess(SYSTYPE_TAF, LOCTYPE_PHSC, INVTYPE_MIRROR_STS, i+1, fidb->mirrorsts[i], oldMirrorSts[i]);
			}
			oldMirrorSts[i] = fidb->mirrorsts[i];

			if( oldMirrorActsts[i] && oldMirrorActsts[i] != fidb->mirroractsts[i] ){
				Send_CondMess(SYSTYPE_TAF, LOCTYPE_PHSC, INVTYPE_MIRROR_ACT, i+1, fidb->mirroractsts[i], oldMirrorActsts[i]);
			}
			oldMirrorActsts[i] = fidb->mirroractsts[i];
		}
#endif
		


		if(abs(now-fork_set) > FORK_CHECK_LIMIT) {
			if(old_forkcnt == forkcnt)
				forkcnt = 1;
			else
				old_forkcnt = forkcnt;

			fork_set = now;
		}

		maskflag = 0;
		for(i = 0; i < MAX_ICH_COUNT; i++) {

			if(dOldInterLock[i] && (fidb->cInterlock[i] != dOldInterLock[i])) {

				log_print(LOGN_DEBUG, LH"dOldInterLock[%d]:[%u] fidb->cInterlock[%d]:[%hu]", 
					LT, i, dOldInterLock[i], i, fidb->cInterlock[i]);
				Send_CondMess(SYSTYPE_TAM, LOCTYPE_CHNL, INVTYPE_LINK, i, fidb->cInterlock[i], dOldInterLock[i]);
			}

			if( (dOldInterLock[i] & MASK) != (fidb->cInterlock[i] & MASK) ){
				maskflag++;
			}

			dOldInterLock[i] = fidb->cInterlock[i];
		}

		if( maskflag ){
			dWriteMaskInfo(0);
		}

		for(i = 0; i < MAX_DIRECT_COUNT; i++)
		{
			if( (now - stPthrdDir[i].tLast) > THREAD_CHECK_INTERVAL)
			{
				log_print(LOGN_DEBUG, LH"THREAD[%d] Check(PthrdDirID[%lu] now[%lu] tLast[%lu] dRunCount[%u] dLastCount[%u]", 
					LT, i, stPthrdDir[i].PthrdDirID, now, stPthrdDir[i].tLast, stPthrdDir[i].dRunCount, stPthrdDir[i].dLastCount);
				if(stPthrdDir[i].dRunCount == stPthrdDir[i].dLastCount)
				{
					log_print(LOGN_CRI, LH"THREAD[%d] STOPPED dRunCount[%u] dLastCount[%u]", 
						LT, i, stPthrdDir[i].dRunCount, stPthrdDir[i].dLastCount);

					if( (dRet = pthread_kill(stPthrdDir[i].PthrdDirID, SIGUSR1)) != 0)
					{
						log_print(LOGN_CRI, LH"THREAD[%d] FAILED IN pthread_kill(PthrdDirID[%lu], SIGUSR1[%d]) errno[%d-%s]", 
							LT, i, stPthrdDir[i].PthrdDirID, SIGUSR1, errno, strerror(errno));
					}

					for(dPortIndex = 0; dPortIndex < MAX_MONITOR_PORT_COUNT; dPortIndex++)
					{
						if( (director->stDIRECT[i].cMonitorPort[dPortIndex] != NOT_EQUIP) && (director->stDIRECT[i].cMonitorPort[dPortIndex] != MASK))
						{
							Send_CondDirSWMess(i, INVTYPE_PORT_MONITOR, dPortIndex, CRITICAL, director->stDIRECT[i].cMonitorPort[dPortIndex]);
							pthread_mutex_lock(&stPthrdDir[i].PthrdMutex);
							director->stDIRECT[i].cMonitorPort[dPortIndex] = CRITICAL;
							pthread_mutex_unlock(&stPthrdDir[i].PthrdMutex);
							log_print(LOGN_DEBUG, LH"cMonitorPort[%02d][%02d][0x%02X]",
								LT, i, dPortIndex, director->stDIRECT[i].cMonitorPort[dPortIndex]);
						}
					}

					for(dPortIndex = 0; dPortIndex < MAX_MIRROR_PORT_COUNT; dPortIndex++)
					{
						if( (director->stDIRECT[i].cMirrorPort[dPortIndex] != NOT_EQUIP) && (director->stDIRECT[i].cMirrorPort[dPortIndex] != MASK))
						{
							Send_CondDirSWMess(i, INVTYPE_PORT_MIRROR, dPortIndex, CRITICAL, director->stDIRECT[i].cMirrorPort[dPortIndex]);
							pthread_mutex_lock(&stPthrdDir[i].PthrdMutex);
							director->stDIRECT[i].cMirrorPort[dPortIndex] = CRITICAL;
							pthread_mutex_unlock(&stPthrdDir[i].PthrdMutex);
							log_print(LOGN_DEBUG, LH"cMirrorPort[%02d][%02d][0x%02X]",
								LT, i, dPortIndex, director->stDIRECT[i].cMirrorPort[dPortIndex]);
						}
					}

					/* added by uamyd 20100928 for Director Power Check routine */
					for(dPortIndex = 0; dPortIndex < MAX_DIRECT_POWER_COUNT; dPortIndex++)
					{
						if( (director->stDIRECT[i].cPower[dPortIndex] != NOT_EQUIP) && (director->stDIRECT[i].cPower[dPortIndex] != MASK))
						{
							Send_CondDirSWMess(i, INVTYPE_POWER, dPortIndex, CRITICAL, director->stDIRECT[i].cPower[dPortIndex]);
							pthread_mutex_lock(&stPthrdDir[i].PthrdMutex);
							director->stDIRECT[i].cPower[dPortIndex] = CRITICAL;
							pthread_mutex_unlock(&stPthrdDir[i].PthrdMutex);
							log_print(LOGN_DEBUG, LH"cPower[%02d][%02d][0x%02X]",
								LT, i, dPortIndex, director->stDIRECT[i].cPower[dPortIndex]);
						}
					}
					pthread_mutex_lock(&stPthrdDir[i].PthrdMutex);
					director->stDIRECT[i].tEachUpTime	= now;
					pthread_mutex_unlock(&stPthrdDir[i].PthrdMutex);
					director->tUpTime	= now;

					if( (dRet = pthread_join(stPthrdDir[i].PthrdDirID, &(stPthrdDir[i].pRetVal))) != 0)
					{
						log_print(LOGN_CRI, LH"THREAD[%d] FAILED IN pthread_join(PthrdDirID[%lu])"EH, LT,i,
							stPthrdDir[i].PthrdDirID, ET);
						exit(-9);
					}
					else
					{
						if(pclose(stPthrdDir[i].fPipe) == -1)
						{
							log_print(LOGN_CRI, LH"THREAD[%d] FAILED IN pclose()"EH,LT,i,ET);
						}

						dResult = (int*)stPthrdDir[i].pRetVal;
						log_print(LOGN_DEBUG, LH"THREAD[%d] SUCCESS IN pthread_join(PthrdDirID[%lu], RetVal[%d])", 
							LT, i, stPthrdDir[i].PthrdDirID, *dResult);
						stPthrdDir[i].cStop		= 0;
						stPthrdDir[i].dArg		= i;
					}

					if( (dRet = pthread_create(&stPthrdDir[i].PthrdDirID, NULL, CheckDirector_init, (void*)&stPthrdDir[i].dArg)) != 0)
					{
						log_print(LOGN_CRI, LH"THREAD[%d] FAILED IN pthread_create(PthrdDirID)"EH, LT,i, ET);
						exit(-10);
					}
					else
						log_print(LOGN_DEBUG, LH"THREAD[%d] SUCCESS IN pthread_create(PthrdDirID[%lu])", LT, i, stPthrdDir[i].PthrdDirID);
				}
				log_print(LOGN_DEBUG, LH"THREAD[%d] dRunCount[%u] dLastCount[%u]", LT, i, stPthrdDir[i].dRunCount, stPthrdDir[i].dLastCount);
				stPthrdDir[i].dLastCount = stPthrdDir[i].dRunCount;
			}
			else
			{
				if(stPthrdDir[i].tLastRenew != stPthrdDir[i].tLast)
				{
					log_print(LOGN_DEBUG, LH"THREAD[%d] PthrdDirID[%lu] tLastRenew[%lu] tLast[%lu]", 
						LT, i, stPthrdDir[i].PthrdDirID, stPthrdDir[i].tLastRenew, stPthrdDir[i].tLast);
					for(dPortIndex = 0; dPortIndex < MAX_MONITOR_PORT_COUNT; dPortIndex++)
						log_print(LOGN_DEBUG, LH"cMonitorPort[%02d][%02d][0x%02X]", LT, i, dPortIndex, director->stDIRECT[i].cMonitorPort[dPortIndex]);

					for(dPortIndex = 0; dPortIndex < MAX_MIRROR_PORT_COUNT; dPortIndex++)
						log_print(LOGN_DEBUG, LH"cMirrorPort[%02d][%02d][0x%02X]", LT, i, dPortIndex, director->stDIRECT[i].cMirrorPort[dPortIndex]);

					/* added by uamyd 20101118 */
					for(dPortIndex = 0; dPortIndex < MAX_DIRECT_POWER_COUNT; dPortIndex++)
						log_print(LOGN_DEBUG, LH"cDirectPower[%02d][%02d][0x%02X]", LT, i, dPortIndex, director->stDIRECT[i].cPower[dPortIndex]);

					stPthrdDir[i].tLastRenew	= stPthrdDir[i].tLast;
				}
			}
		}

		for(i = 0; i < MAX_SWITCH_COUNT; i++)
		{
			if( (now - stPthrdSW[i].tLast) > THREAD_CHECK_INTERVAL)
			{
				log_print(LOGN_DEBUG, LH"THREAD[%d] Check(PthrdDirID[%lu] now[%lu] tLast[%lu] dRunCount[%u] dLastCount[%u]", 
					LT,i,stPthrdSW[i].PthrdDirID, now, stPthrdSW[i].tLast, stPthrdSW[i].dRunCount, stPthrdSW[i].dLastCount);
				if(stPthrdSW[i].dRunCount == stPthrdSW[i].dLastCount)
				{
					log_print(LOGN_CRI, LH"THREAD[%d] STOPPED dRunCount[%u] dLastCount[%u]",
						LT, i, stPthrdSW[i].dRunCount, stPthrdSW[i].dLastCount);

					if( (dRet = pthread_kill(stPthrdSW[i].PthrdDirID, SIGUSR1)) != 0)
					{
						log_print(LOGN_CRI, LH"THREAD[%d] FAILED IN pthread_kill(PthrdDirID[%lu], SIGUSR1[%d])"EH,
							LT,i, stPthrdSW[i].PthrdDirID, SIGUSR1, ET);
					}

					for(dPortIndex = 0; dPortIndex < MAX_SWITCH_PORT_COUNT; dPortIndex++)
					{
						if( (swch->stSwitch[i].cSwitchPort[dPortIndex] != NOT_EQUIP) && (swch->stSwitch[i].cSwitchPort[dPortIndex] != MASK))
						{
							Send_CondDirSWMess(i, INVTYPE_PORT_SWITCH, dPortIndex, CRITICAL, swch->stSwitch[i].cSwitchPort[dPortIndex]);
							pthread_mutex_lock(&stPthrdSW[i].PthrdMutex);
							swch->stSwitch[i].cSwitchPort[dPortIndex] = CRITICAL;
							pthread_mutex_unlock(&stPthrdSW[i].PthrdMutex);
							log_print(LOGN_DEBUG, LH"cSwitchPort[%02d][%02d][0x%02X]", 
								LT, i, dPortIndex, swch->stSwitch[i].cSwitchPort[dPortIndex]);
						}
					}

					for(dPortIndex = 0; dPortIndex < MAX_SWITCH_CPU_COUNT; dPortIndex++)
					{
						pthread_mutex_lock(&stPthrdSW[i].PthrdMutex);
						swch->stSwitch[i].uSwitchCPU[dPortIndex] = 0;
						pthread_mutex_unlock(&stPthrdSW[i].PthrdMutex);
						log_print(LOGN_DEBUG, LH"uSwitchCPU[%02d][%02d][0x%02X]",
							LT, i, dPortIndex, swch->stSwitch[i].uSwitchCPU[dPortIndex]);
					}

					for(dPortIndex = 0; dPortIndex < MAX_SWITCH_MEM_COUNT; dPortIndex++)
					{
						pthread_mutex_lock(&stPthrdSW[i].PthrdMutex);
						swch->stSwitch[i].uSwitchMEM[dPortIndex] = 0;
						pthread_mutex_unlock(&stPthrdSW[i].PthrdMutex);
						log_print(LOGN_DEBUG, LH"uSwitchMEM[%02d][%02d][0x%02X]",
							LT, i, dPortIndex, swch->stSwitch[i].uSwitchMEM[dPortIndex]);
					}

					pthread_mutex_lock(&stPthrdSW[i].PthrdMutex);
					swch->stSwitch[i].tEachUpTime	= now;
					pthread_mutex_unlock(&stPthrdSW[i].PthrdMutex);
					swch->tUpTime	= now;

					if( (dRet = pthread_join(stPthrdSW[i].PthrdDirID, &(stPthrdSW[i].pRetVal))) != 0)
					{
						log_print(LOGN_CRI, LH"THREAD[%d] FAILED IN pthread_join(PthrdDirID[%lu])"EH,
							LT, i, stPthrdSW[i].PthrdDirID, ET);
						exit(-11);
					}
					else
					{
						if(pclose(stPthrdSW[i].fPipe) == -1)
						{
							log_print(LOGN_CRI, LH"THREAD[%d] FAILED IN pclose()"EH, LT, i, ET);
						}

						dResult = (int*)stPthrdSW[i].pRetVal;
						log_print(LOGN_DEBUG, LH"THREAD[%d] SUCCESS IN pthread_join(PthrdDirID[%lu], RetVal[%d])", 
							LT, i, stPthrdSW[i].PthrdDirID, *dResult);
						stPthrdSW[i].cStop		= 0;
						stPthrdSW[i].dArg		= i;
					}

					if( (dRet = pthread_create(&stPthrdSW[i].PthrdDirID, NULL, CheckSwitch_init, (void*)&stPthrdSW[i].dArg)) != 0)
					{
						log_print(LOGN_CRI, LH"THREAD[%d] FAILED IN pthread_create(PthrdDirID)"EH, LT, i, ET);
						exit(-12);
					}
					else
						log_print(LOGN_DEBUG, LH"THREAD[%d] SUCCESS IN pthread_create(PthrdDirID[%lu])", LT, i, stPthrdSW[i].PthrdDirID);
				}
				log_print(LOGN_DEBUG, LH"THREAD[%d] dRunCount[%u] dLastCount[%u]", LT, i, stPthrdSW[i].dRunCount, stPthrdSW[i].dLastCount);
				stPthrdSW[i].dLastCount = stPthrdSW[i].dRunCount;
			}
			else
			{
				if(stPthrdSW[i].tLastRenew != stPthrdSW[i].tLast)
				{
					log_print(LOGN_DEBUG, LH"THREAD[%d] PthrdDirID[%lu] tLastRenew[%lu] tLast[%lu]", 
						LT, i, stPthrdSW[i].PthrdDirID, stPthrdSW[i].tLastRenew, stPthrdSW[i].tLast);

					for(dPortIndex = 0; dPortIndex < MAX_SWITCH_PORT_COUNT; dPortIndex++)
					{
						log_print(LOGN_DEBUG, LH"cSwitchPort[%02d][%02d][0x%02X]", 
							LT, i, dPortIndex, swch->stSwitch[i].cSwitchPort[dPortIndex]);
					}

					for(dPortIndex = 0; dPortIndex < MAX_SWITCH_CPU_COUNT; dPortIndex++)
					{
						log_print(LOGN_DEBUG, LH"uSwitchCPU[%02d][%02d][%u]", 
							LT, i, dPortIndex, swch->stSwitch[i].uSwitchCPU[dPortIndex]);
					}

					for(dPortIndex = 0; dPortIndex < MAX_SWITCH_MEM_COUNT; dPortIndex++)
					{
						log_print(LOGN_DEBUG, LH"uSwitchMEM[%02d][%02d][%u]", 
							LT, i, dPortIndex, swch->stSwitch[i].uSwitchMEM[dPortIndex]);
					}

					stPthrdSW[i].tLastRenew	= stPthrdSW[i].tLast;
				}
			}
		}
	}

	/***************************************************************************
	 MAIN WHILE LOOP END
	***************************************************************************/

    if( fidb->mpsw[SEQ_PROC_CHSMD] == NORMAL && fidb->mpswinfo[SEQ_PROC_CHSMD].pid != 0 )
    	Send_CondMess( SYSTYPE_TAM, LOCTYPE_PROCESS, INVTYPE_USERPROC, SEQ_PROC_CHSMD, STOP, fidb->mpsw[SEQ_PROC_CHSMD] );

	fidb->mpsw[SEQ_PROC_CHSMD]       = STOP;
	fidb->mpswinfo[SEQ_PROC_CHSMD].pid  = 0;
	fidb->mpswinfo[SEQ_PROC_CHSMD].when = time(&now);

	log_print(LOGN_CRI, LH"PROCESS END!!",LT);
	FinishProgram();

	return 0;
}


/*******************************************************************************
 CHECK MESSAGE QUEUE
*******************************************************************************/
int IsRcvedMessage( pst_MsgQ *pstMsg )
{
	int		dRet;

	keepalive->cnt[SEQ_PROC_CHSMD]++;
	if( (dRet = dMsgrcv(pstMsg)) < 0 ){
		if( dRet != -1 ){
			log_print(LOGN_CRI, LH"FAILED IN dMsgrcv(CHSMD)",LT);
		}
		return NO_MSG;
	}
    return 1;
}

void *CheckDirector_init(void *arg)
{
	int					dMyID, *dRetVal;
	struct sigaction	saPthrd, saPthrdWait;

	dRetVal	= (int*)arg;
	dMyID	= *dRetVal;

	*dRetVal			= 0;
	saPthrd.sa_handler	= sigHandler_DirPthrd;
	if(sigemptyset(&saPthrd.sa_mask) == -1)
	{
		*dRetVal	= -1;
		pthread_exit(arg);
	}
	saPthrd.sa_flags = 0;

	if(sigaction(SIGUSR1, &saPthrd, NULL) == -1)
	{
		*dRetVal	= -2;
		pthread_exit(arg);
	}

	saPthrdWait.sa_handler	= sigWaitHandler_DirPthrd;
	if(sigemptyset(&saPthrdWait.sa_mask) == -1)
	{
		*dRetVal	= -3;
		pthread_exit(arg);
	}
	saPthrdWait.sa_flags = 0;

	if(sigaction(SIGUSR2, &saPthrdWait, NULL) == -1)
	{
		*dRetVal	= -4;
		pthread_exit(arg);
	}

	while(StopFlag && !stPthrdDir[dMyID].cStop)
	{
		if( ((stPthrdDir[dMyID].tStart = time(NULL))-stPthrdDir[dMyID].tLast) > DIRECT_CHECK_LIMIT)
		{
			stPthrdDir[dMyID].tLast = stPthrdDir[dMyID].tStart;
			CheckDirector(dMyID);
			stPthrdDir[dMyID].dRunCount++;
		}
	}

	return arg;
}

void sigHandler_DirPthrd(int signum)
{
	int			i;
	pthread_t	PthrdID;

	PthrdID = pthread_self();
	for(i = 0; i < MAX_DIRECT_COUNT; i++)
	{
		if(stPthrdDir[i].PthrdDirID == PthrdID)
			break;
	}

	if(i >= MAX_DIRECT_COUNT)
		exit(-13);
	else
		stPthrdDir[i].dArg	= -5;

	pthread_mutex_unlock(&stPthrdDir[i].PthrdMutex);
	pthread_exit(&stPthrdDir[i].dArg);
}

void sigWaitHandler_DirPthrd(int signum)
{
	int			i;
	pthread_t	PthrdID;

	PthrdID = pthread_self();
	for(i = 0; i < MAX_DIRECT_COUNT; i++)
	{
		if(stPthrdDir[i].PthrdDirID == PthrdID)
			break;
	}

	if(i >= MAX_DIRECT_COUNT)
		exit(-14);

	pthread_mutex_unlock(&stPthrdDir[i].PthrdMutex);
	sleep(2);
}

void *CheckSwitch_init(void *arg)
{
	int					dMyID, *dRetVal;
	struct sigaction	saPthrd, saPthrdWait;

	dRetVal	= (int*)arg;
	dMyID	= *dRetVal;

	*dRetVal			= 0;
	saPthrd.sa_handler	= sigHandler_SWPthrd;
	if(sigemptyset(&saPthrd.sa_mask) == -1)
	{
		*dRetVal	= -1;
		pthread_exit(arg);
	}
	saPthrd.sa_flags = 0;

	if(sigaction(SIGUSR1, &saPthrd, NULL) == -1)
	{
		*dRetVal	= -2;
		pthread_exit(arg);
	}

	saPthrdWait.sa_handler	= sigWaitHandler_SWPthrd;
	if(sigemptyset(&saPthrdWait.sa_mask) == -1)
	{
		*dRetVal	= -3;
		pthread_exit(arg);
	}
	saPthrdWait.sa_flags = 0;

	if(sigaction(SIGUSR2, &saPthrdWait, NULL) == -1)
	{
		*dRetVal	= -4;
		pthread_exit(arg);
	}

	while(StopFlag && !stPthrdSW[dMyID].cStop)
	{
		if( ((stPthrdSW[dMyID].tStart = time(NULL))-stPthrdSW[dMyID].tLast) > DIRECT_CHECK_LIMIT)
		{
			stPthrdSW[dMyID].tLast = stPthrdSW[dMyID].tStart;
			CheckSwitch(dMyID);
			stPthrdSW[dMyID].dRunCount++;
		}
	}

	return arg;
}

void sigHandler_SWPthrd(int signum)
{
	int			i;
	pthread_t	PthrdID;

	PthrdID = pthread_self();
	for(i = 0; i < MAX_SWITCH_COUNT; i++)
	{
		if(stPthrdSW[i].PthrdDirID == PthrdID)
			break;
	}

	if(i >= MAX_SWITCH_COUNT)
		exit(-15);
	else
		stPthrdSW[i].dArg	= -5;

	pthread_mutex_unlock(&stPthrdSW[i].PthrdMutex);
	pthread_exit(&stPthrdSW[i].dArg);
}

void sigWaitHandler_SWPthrd(int signum)
{
	int			i;
	pthread_t	PthrdID;

	PthrdID = pthread_self();
	for(i = 0; i < MAX_SWITCH_COUNT; i++)
	{
		if(stPthrdSW[i].PthrdDirID == PthrdID)
			break;
	}

	if(i >= MAX_SWITCH_COUNT)
		exit(-16);

	pthread_mutex_unlock(&stPthrdSW[i].PthrdMutex);
	sleep(2);
}

