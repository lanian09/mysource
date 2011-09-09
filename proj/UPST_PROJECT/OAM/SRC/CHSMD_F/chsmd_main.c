/**A.1*  File Inclusion *******************************************************/

/* SYS HEADER */
#include <stdio.h>
#include <errno.h>
#include <string.h>			/* MEMCPY() */
#include <stdlib.h>			/* EXIT(3) */
#include <unistd.h>			/* GETPID(2) */
#include <sys/stat.h>		/* stat */
/* LIB HEADER */
#include "filedb.h"			/* st_NTAF, st_keepalive_taf */
#include "loglib.h"
#include "verlib.h"			/* set_version() */
#include "ipclib.h"			/* st_Qentry */
/* PRO HEADER */
#include "path.h"
#include "sshmid.h"
#include "procid.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
#include "almstat.h"
/* LOC HEADER */
#include "chsmd_load.h"		/* st_linkdev */
#include "chsmd_hw.h"		/* FILE_MESSAGE */
#include "chsmd_disk.h"		/* st_SoldfList, dGetDF() */
#include "chsmd_init.h"		/* dGetSYSCFG(), init_CHSMD(), init_prcmd(), FinishProgram() */
#include "chsmd_msg.h"		/* Send_ALmMsg() */
#include "chsmd_mmc.h"		/* MMC_Handle_Proc() */
#include "chsmd_ntpd.h"		/* CheckNTPStS() */
#include "chsmd_sw.h"		/* check_software() */
#include "chsmd_func.h"		/* dMsgrcv() */

/**B.1*  Definition of New Constants ******************************************/
/**B.2*  Definition of New Type  **********************************************/
/**C.1*  Declaration of Variables  ********************************************/
pst_keepalive_taf keepalive;

int dCurrBlockCnt;
int StopFlag = 1;
int	gdMsgOldSize;
int killmcflag =0;
int gdSysNo = -1;
int g_dMirror_Term;

char STR_TSW_COM[MAX_NTAF_SW_BLOCK][30];
char vERSION[7] = "R3.0.0";

/**C.2*  Declaration of Variables  ********************************************/
extern pst_NTAF	  fidb;

/* 프로세스 관리와 CPU, MEMORY, 알람정보 처리 */
extern char disk_oldstat[MAX_NTAF_DISK_COUNT];

/**D.1*  Definition of Functions  *********************************************/
/**D.2*  Definition of Functions  *********************************************/

/*******************************************************************************
 * MAIN FUNCTION
*******************************************************************************/
int main()
{
	pst_MsgQ	pstMsg;
    st_MsgQSub	stMsgQSub;
	st_Qentry   recvmesg;
#ifdef ENABLE_ENCLOSURE
	struct stat stStat;
#endif

	time_t	now, sw_set, dog_set, load_set, load_df;
	long   *pmsg;
	int		i, dRet, EndFlag, dfcnt, dMirrorCnt;
	char	szLoad[6], cOldMirrorStatus[MAX_ENDACE_COUNT], cOldMirrorActiveStatus[MAX_ENDACE_COUNT];

	st_SoldfList   dflist;

	EndFlag			= 1;

	for(i = 0; i < MAX_ENDACE_COUNT; i++)
	{
		cOldMirrorStatus[i]			= 0x00;
		cOldMirrorActiveStatus[i]	= 0x00;
	}

	/*** LOG INITIALIZATION****************************************************/
	log_init(S_SSHM_LOG_LEVEL, getpid(), SEQ_PROC_CHSMD, LOG_PATH"/CHSMD", "CHSMD");

	/*** BLOCK COUNT SETTING - CAUTION! TAF와 TAM의 개수가 다르고, System마다 다름을 기억하자. */
	if( (dRet = dGetBlocks(FILE_MC_INIT,STR_TSW_COM)) < 0 ){

		log_print(LOGN_CRI,LH"ERROR GET McInit INF ",LT);
		exit(0);
	}

	dCurrBlockCnt = dRet;


	/* CHSMD 공유메모리 메시지 큐 초기화 */
	/***************************************************************************
	 * SHARED MEMORY INITIALIZATION
	***************************************************************************/
	if( (dRet = init_CHSMD()) < 0 ){

		log_print(LOGN_CRI,LH"FAILED IN init_CHSMD()",LT);
		exit(0);
	}

	if( (dRet = set_version(S_SSHM_VERSION, SEQ_PROC_CHSMD, vERSION)) < 0 ){
		log_print(LOGN_DEBUG,LH"set_version error(ret=%d,idx=%d,ver=%s)\n",LT, dRet,SEQ_PROC_CHSMD,vERSION);
		exit(0);
	}

	/*** READ SYSTEM CONFIG FILE **********************************************/
	memset(&dflist, 0x00, sizeof(st_SoldfList));
	if( (dRet = dGetSYSCFG(&dflist)) < 0)
	{
		log_print(LOGN_WARN, LH"FAILED IN dGetSYSCFG() dRet=%d", LT, dRet);
		exit(0);
	}
	gdSysNo = dRet;

	/*** CHECK DAG CARD TYPE **************************************************/
	if( (dRet = dCheckDagType()) < 0 ){

		log_print(LOGN_CRI,LH"FAILED IN dCheckDagType()=%d",LT,dRet );
		exit(0);
	}

	log_print(LOGN_CRI,LH"CHSMD %s Started",LT,vERSION);


	log_print(LOGN_DEBUG,LH"NTPCNT[%d] STATUSD[%d] STATUSC[%d]", LT,fidb->hwntpcnt, fidb->hwntp[0], fidb->hwntp[1]);

	/***************************************************************************
	* signal and ipc - msgq
	***************************************************************************/
	dRet = init_prcmd();
	if( dRet < 0 )
	{
		log_print(LOGN_DEBUG, LH"[FAIL] INIT_CHSMD() [%d]", LT,dRet );
		exit(1);
	}

	/*** g_dMirror_Term 은 사용되지 않음...ㅜㅡ 활용할 수는 있을 것 같음, 그래서 내비둠. */
	dRet = dInit_Mirror_Timer(&g_dMirror_Term);
	if( dRet < 0 ) {
		log_print( LOGN_CRI, LH"FAIL IN dInit_Mirror_Timer dRet: %d", LT,dRet );
		//exit(0);	//사용하지 않기 때문에 주석처리함. 만약 사용하게 된다면 exit(0)를 활성화시켜야 함.
	}

#ifdef ENABLE_ENCLOSURE
	dRet = Init_Enclosure();
	if( dRet < 0 )
	{
		log_print(LOGN_DEBUG,LH"[ERROR] ENCLOSURE SERIAL FILE OPEN",LT);
		exit(0);
	}

	stat( FILE_MESSAGE, &stStat );
	gdMsgOldSize = stStat.st_size;
#endif

	/***************************************************************************
	* INITIAL CPU OLD VALUE SETTING
	***************************************************************************/
	for(i=0; i<2; i++) {
		cpu_compute();
		sleep(1);
	}

	now = time(&now);

	/* INITIALIZE CHSMD FIDB STATUS *******************************************/

	fidb->mpsw[SEQ_PROC_CHSMD] = STOP;
	fidb->mpswinfo[SEQ_PROC_CHSMD].pid = 0;

	while(EndFlag)
	{
		/***********************************************************************
		* CHECK MESSAGE QUEUE
		***********************************************************************/
		if( (dRet = IsRcvedMessage(&pstMsg)) > 0)
		{
			memcpy(&stMsgQSub, &pstMsg->llMType, sizeof(stMsgQSub));

			recvmesg.mtype = pstMsg->llMType;
			if( (recvmesg.mtype != KillMC) && (recvmesg.mtype != StartMC))
				memcpy(recvmesg.mtext, &pstMsg->szBody[0], sizeof(mml_msg));
			else
			{
				memcpy(recvmesg.mtext, &pstMsg->szBody[sizeof(long)], sizeof(long)*3);
				pmsg = (long *)&recvmesg;
				log_print(LOGN_DEBUG, LH"[TIFB] [%ld] [%ld] [%ld] ", LT,recvmesg.mtype, pmsg[1], pmsg[2]);
			}
		}
		else
			recvmesg.mtype = NO_MSG;

		switch(recvmesg.mtype)
		{
			case KillMC:	/*	KillMC로 부터 받은 메시지 **********************/
				pmsg = (long*)&recvmesg;
				/* 첫번째 첨자는 등록 블럭 ID */
				if( (pmsg[1] > 0) && (pmsg[1] < dCurrBlockCnt))
				{
					if( (fidb->mpsw[pmsg[1]] == NORMAL) && (fidb->mpswinfo[pmsg[1]].pid != 0))
						Send_AlmMsg(LOCTYPE_PROCESS, INVTYPE_USERPROC, pmsg[1], STOP, fidb->mpsw[pmsg[1]]);

					if(fidb->mpsw[pmsg[1]] != MASK)
						fidb->mpsw[pmsg[1]] = STOP;

					fidb->mpswinfo[pmsg[1]].pid		= 0;
					fidb->mpswinfo[pmsg[1]].when	= now;
				}

				if(pmsg[1] == 0)
				{
					if( (fidb->mpsw[pmsg[1]] == NORMAL) && (fidb->mpswinfo[pmsg[1]].pid != 0))
						Send_AlmMsg(LOCTYPE_PROCESS, INVTYPE_USERPROC, pmsg[1], STOP, fidb->mpsw[pmsg[1]]);

					killmcflag = 1;

					if(fidb->mpsw[pmsg[1]] != MASK)
						fidb->mpsw[pmsg[1]] = STOP;

					fidb->mpswinfo[pmsg[1]].pid		= 0;

					/*********************************************************
					pmsg[3] 은 T_Alm_Status의 type 이 time_t를 갖틑 tWhen 값
					단, long type으로 바뀌었을 경우에 한함
					*********************************************************/
					fidb->mpswinfo[pmsg[1]].when	= pmsg[3];
				}
				log_print(LOGN_DEBUG, LH"CHECK STATUS : [%ld] [0x%02x]", LT,pmsg[1], fidb->mpsw[pmsg[1]]);

				/*** pmsg[2] IS PROCESS ID ************************************/
				log_print(LOGN_CRI, LH"KillMC -b %s(%ld) : PID:%ld STATUS:0x%02x", LT,STR_TSW_COM[pmsg[1]], pmsg[1],  pmsg[2], fidb->mpsw[pmsg[1]]);
				write_FIDB();
				break;

			case StartMC:	/*	StartMC로 부터 받은 메시지 *********************/
				pmsg = (long*)&recvmesg;
				/*	첫번째 첨자는 등록 블럭 ID	*/
				if( (pmsg[1] >= 0) && (pmsg[1] < dCurrBlockCnt))
				{
					if( ((fidb->mpsw[pmsg[1]] == STOP) ||( fidb->mpsw[pmsg[1]] == CRITICAL)) && (fidb->mpswinfo[pmsg[1]].pid == 0))
						Send_AlmMsg(LOCTYPE_PROCESS, INVTYPE_USERPROC, pmsg[1], NORMAL, fidb->mpsw[pmsg[1]]);

					if(pmsg[1] == 0)
						log_print(LOGN_DEBUG, LH"CHSMD INFORMATION : [%ld] [0x%02x]", LT,pmsg[1], fidb->mpsw[pmsg[1]]);

					if(fidb->mpsw[pmsg[1]] == MASK)
						fidb->mpsw[pmsg[1]] = MASK;
					else if(fidb->mpsw[pmsg[1]] == NOT_EQUIP)
					{
						Send_AlmMsg(LOCTYPE_PROCESS, INVTYPE_USERPROC, pmsg[1], NORMAL, CRITICAL);
						fidb->mpsw[pmsg[1]] = NORMAL;
					}
					else
						fidb->mpsw[pmsg[1]] = NORMAL;

					/* 두번째 값은 pid이다 */
					fidb->mpswinfo[pmsg[1]].pid		= pmsg[2];
					fidb->mpswinfo[pmsg[1]].when	= now;
				}
				log_print(LOGN_CRI, LH"StartMC -b %s(%ld) : PID:%ld STATUS:0x%02x", LT, STR_TSW_COM[pmsg[1]], pmsg[1], pmsg[2], fidb->mpsw[pmsg[1]]);
				write_FIDB();
				/***************************************************************
				* BECAUSE FORK TIME PROBLEM
				***************************************************************/
				break;

			case NO_MSG:
				log_print(LOGN_DEBUG, LH"SKIP WHILE LOOP END",LT);
				EndFlag = 0;
				break;

			default:
				break;
		}
	}
	load_df = load_set = dog_set = sw_set = now;

	/***************************************************************************
	* MAIN WHILE LOOP START
	***************************************************************************/
	while(StopFlag)
	{
		/***********************************************************************
		* CHECK MESSAGE QUEUE
		***********************************************************************/
        if( (dRet = IsRcvedMessage(&pstMsg)) > 0)
		{
			memcpy(&stMsgQSub, &pstMsg->llMType, sizeof(stMsgQSub));

			recvmesg.mtype = pstMsg->llMType;
			if( (recvmesg.mtype != KillMC) && (recvmesg.mtype != StartMC))
			{
				memcpy(recvmesg.mtext, &pstMsg->szBody[0], sizeof(mml_msg));
				log_print(LOGN_DEBUG, LH"[MMC] RLEN[%d] BLEN[%d] PROID[%d] ", LT, dRet, pstMsg->usBodyLen, pstMsg->ucProID);
			}
			else
			{
				memcpy(recvmesg.mtext, &pstMsg->szBody[sizeof(long)], sizeof(long)*3);
				pmsg = (long*)&recvmesg;
				log_print(LOGN_DEBUG, LH"[TIFB] [%ld] [%ld] [%ld] [%ld] ", LT,recvmesg.mtype, pmsg[1], pmsg[2], pmsg[3]);
			}
		}
		else
			recvmesg.mtype = NO_MSG;

		switch(recvmesg.mtype)
		{
			case KillMC:	/* KillMC로 부터 받은 메시지 **********************/
				pmsg = (long*)&recvmesg;
				/* 첫번째 첨자는 등록 블럭 ID */
				if( (pmsg[1] > 0) && (pmsg[1] < dCurrBlockCnt))
				{
					if( (fidb->mpsw[pmsg[1]] == NORMAL) && (fidb->mpswinfo[pmsg[1]].pid != 0))
						Send_AlmMsg(LOCTYPE_PROCESS, INVTYPE_USERPROC, pmsg[1], STOP, fidb->mpsw[pmsg[1]]);

					if(fidb->mpsw[pmsg[1]] != MASK)
						fidb->mpsw[pmsg[1]] = STOP;

					fidb->mpswinfo[pmsg[1]].pid		= 0;
					fidb->mpswinfo[pmsg[1]].when	= now;
				}

				if(pmsg[1] == 0)
				{
					if( (fidb->mpsw[pmsg[1]] == NORMAL) && (fidb->mpswinfo[pmsg[1]].pid != 0))
						Send_AlmMsg(LOCTYPE_PROCESS, INVTYPE_USERPROC, pmsg[1], STOP, fidb->mpsw[pmsg[1]]);

					killmcflag = 1;

					if(fidb->mpsw[pmsg[1]] != MASK)
						fidb->mpsw[pmsg[1]] = STOP;

					fidb->mpswinfo[pmsg[1]].pid		= 0;
					fidb->mpswinfo[pmsg[1]].when	= pmsg[3];
				}
				Send_ALMD();

				/*** pmsg[2] IS PROCESS ID ************************************/
				log_print(LOGN_CRI, LH"KillMC -b %s(%ld) : PID:%ld STATUS:0x%02x", LT,STR_TSW_COM[pmsg[1]], pmsg[1], pmsg[2], fidb->mpsw[pmsg[1]]);
				write_FIDB();
				break;

			case StartMC:	/*	StartMC로 부터 받은 메시지 *********************/
				pmsg = (long*)&recvmesg;
				/* 첫번째 첨자는 등록 블럭 ID */
				if( (pmsg[1] >= 0) && (pmsg[1] < dCurrBlockCnt))
				{
					if( ((fidb->mpsw[pmsg[1]] == STOP) || (fidb->mpsw[pmsg[1]] == CRITICAL)) && (fidb->mpswinfo[pmsg[1]].pid == 0))
						Send_AlmMsg(LOCTYPE_PROCESS, INVTYPE_USERPROC, pmsg[1], NORMAL, fidb->mpsw[pmsg[1]]);

					if(fidb->mpsw[pmsg[1]] == MASK)
						fidb->mpsw[pmsg[1]] = MASK;
					else if(fidb->mpsw[pmsg[1]] == NOT_EQUIP)
					{
						Send_AlmMsg(LOCTYPE_PROCESS, INVTYPE_USERPROC, pmsg[1], NORMAL, CRITICAL);
						fidb->mpsw[pmsg[1]] = NORMAL;
					}
					else
						fidb->mpsw[pmsg[1]] = NORMAL;

					fidb->mpswinfo[pmsg[1]].pid		= pmsg[2];
					fidb->mpswinfo[pmsg[1]].when	= now;
				}
				Send_ALMD();

				/*	두번째 값은 pid이다	*/
				log_print(LOGN_CRI, LH"StartMC -b %s(%ld) : PID:%ld STATUS:0x%02x", LT,STR_TSW_COM[pmsg[1]], pmsg[1],  pmsg[2], fidb->mpsw[pmsg[1]]);
				write_FIDB();
				/***************************************************************
				* BECAUSE FORK TIME PROBLEM
				***************************************************************/
				sleep(2);
				break;

			case NO_MSG:
				break;

			default:
				if( (dRet = MMC_Handle_Proc((mml_msg*)recvmesg.mtext, recvmesg.mtype, pstMsg->llIndex)) < 0)
					log_print(LOGN_WARN, LH"[MAIN]FAILED IN MMC_Handle_Proc[%d]", LT,dRet);
				write_FIDB();
				break;
		}
		now = time(&now);

		/***********************************************************************
		* CHECK PROCESS STATUS PER 2 SECOND
		***********************************************************************/
		if( abs(now - sw_set ) > PROC_CHECK_LIMIT)
		{
			log_print(LOGN_INFO,LH"[CHECK SOFTWARE]",LT);
			check_software();
			sw_set = now ;
		}

		/***********************************************************************
		* CHECK CPU, MEMORY LOAD PER 1 SECOND
		***********************************************************************/
		if(abs(now-load_set) > HW_CHECK_LIMIT)
		{
			if( (dRet = dLinkCheck()) < 0)
				log_print( LOGN_CRI, LH"ERROR IN dLinkCheck() dRet=%d", LT, dRet);

			cpu_compute();
			mem_compute();
			queue_compute();
			nifo_compute();
			dCheckHW();
			CheckNTPStS();

			load_set = now ;
		}

		/***********************************************************************
		* CHECK DISK LOAD PER 60 SECOND
		***********************************************************************/
		if(abs(now-load_df) > DISK_CHECK_LIMIT)
		{
			log_print(LOGN_DEBUG, LH"[CHECK DISK LOAD]",LT);
			dGetDF(&dflist);
			for(dfcnt = 0; dfcnt < dflist.dCount; dfcnt++)
			{
				sprintf(szLoad, "%5.2f", (double)(100.0 - dflist.stSoldf[dfcnt].dPercent));
				log_print(LOGN_INFO, LH" DF USAGE =%s", LT,szLoad);

				fidb->disksts[dfcnt].llCur	= dflist.stSoldf[dfcnt].llUsed;
				fidb->disksts[dfcnt].lMax	= dflist.stSoldf[dfcnt].llTotal;

				dCheckLoad(INVTYPE_DISK, dfcnt, szLoad);
				log_print(LOGN_DEBUG, LH"DCHECKLOAD  MPDISK = [%s] OLDSTAT = [0x%x]", LT,szLoad, (char)disk_oldstat[dfcnt]);
			}
			load_df = now ;
		}

		for(dMirrorCnt = 0; dMirrorCnt < MAX_ENDACE_COUNT; dMirrorCnt++)
		{
			if(cOldMirrorStatus[dMirrorCnt] == 0x00)
				cOldMirrorStatus[dMirrorCnt]	= fidb->mirrorsts[dMirrorCnt];
			else
			{
				if(cOldMirrorStatus[dMirrorCnt] != fidb->mirrorsts[dMirrorCnt])
				{
					Send_AlmMsg(LOCTYPE_PHSC, INVTYPE_MIRROR_STS, dMirrorCnt+1, fidb->mirrorsts[dMirrorCnt], cOldMirrorStatus[dMirrorCnt]);
					cOldMirrorStatus[dMirrorCnt]	= fidb->mirrorsts[dMirrorCnt];
				}
			}

			if(cOldMirrorActiveStatus[dMirrorCnt] == 0x00)
				cOldMirrorActiveStatus[dMirrorCnt]	= fidb->mirrorActsts[dMirrorCnt];
			else
			{
				if(cOldMirrorActiveStatus[dMirrorCnt] != fidb->mirrorActsts[dMirrorCnt])
				{
					Send_AlmMsg(LOCTYPE_PHSC, INVTYPE_MIRROR_ACT, dMirrorCnt+1, fidb->mirrorActsts[dMirrorCnt], cOldMirrorActiveStatus[dMirrorCnt]);
					cOldMirrorActiveStatus[dMirrorCnt]	= fidb->mirrorActsts[dMirrorCnt];
				}
			}
		}
	}

	/***************************************************************************
	* WHEN CHSMD IS DEAD
	***************************************************************************/
	if(killmcflag != 1)
	{
		if( (fidb->mpsw[SEQ_PROC_CHSMD] == NORMAL) && (fidb->mpswinfo[SEQ_PROC_CHSMD].pid != 0))
			Send_AlmMsg(LOCTYPE_PROCESS, INVTYPE_USERPROC, SEQ_PROC_CHSMD, STOP, fidb->mpsw[SEQ_PROC_CHSMD]);

		fidb->mpsw[SEQ_PROC_CHSMD]	= STOP;
		fidb->mpswinfo[SEQ_PROC_CHSMD].pid	= 0;
		fidb->mpswinfo[SEQ_PROC_CHSMD].when	= -1;
	}
	write_FIDB();
	FinishProgram();

	return 0;
}

/*******************************************************************************
 * CHECK MESSAGE QUEUE FUNCTION
*******************************************************************************/
int IsRcvedMessage(pst_MsgQ *pstMsg)
{
	int     dRet;

	keepalive->cnt[SEQ_PROC_CHSMD]++;

	alarm(1);
	dRet = dMsgrcv(pstMsg);
	alarm(0);
	if( dRet < 0 ){
	
        if( dRet != -1 ){
            log_print(LOGN_CRI, LH"FAILED IN dMsgrcv(CHSMD)",LT);
        }
        return NO_MSG;
    }

	return 1;
}

