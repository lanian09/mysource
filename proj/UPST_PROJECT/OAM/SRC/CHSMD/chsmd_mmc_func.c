/**A.1 * File Include ************************************/

/* SYS HEADER */
#include <stdio.h>
#include <sys/types.h>	/* MSGSND(2) */
#include <sys/ipc.h>	/* MSGSND(2) */
#include <sys/msg.h>	/* MSGSND(2) */
#include <signal.h>		/* PTHREAD_KILL(P) */
#include <stdlib.h>		/* ATOI(3) */
#include <pthread.h>	/* pthread_mutex_lock() */
/* LIB HEADER */
#include "clisto.h"		/* U8 */
#include "filedb.h"
#include "loglib.h"
#include "verlib.h"
#include "utillib.h"
/* PRO HEADER */
#include "msgdef.h"
#include "mmcdef.h"
#include "sockio.h"
#include "procid.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
#include "almstat.h"
/* LOC HEADER */
#include "chsmd_hw.h"
#include "chsmd_sw.h"
#include "chsmd_func.h"	/* dMsgsnd(), dGetNode() */
#include "chsmd_mmc.h"

/**B.1*  Definition of New Constants *********************/
/**B.2*  Definition of New Type  **************************/
/**C.1*  Declaration of Variables  ************************/
int SEQ_PROC_IDX[MAX_SW_COUNT];
/**C.2*  Declaration of Variables  ************************/
extern pst_NTAM       fidb;
extern pst_DIRECT_MNG director;
extern pst_SWITCH_MNG swch;

extern st_subsys_mng    stSubSysMng;
extern st_PthrdDirect	stPthrdDir[MAX_DIRECT_COUNT];
extern st_PthrdDirect	stPthrdSW[MAX_SWITCH_COUNT];

extern char STR_TSW_COM[MAX_SW_COUNT][30];
extern int  dCurrBlockCnt;

/**D.1*  Definition of Functions  *************************/
/**D.2*  Definition of Functions  *************************/
int dDis_prc(mml_msg *ml)
{
    int         i,j,dLoopCnt;
    dbm_msg_t   smsg;
    char        szBuf[MSG_DATA_LEN];
    char        szStatus[16];

	if( (dCurrBlockCnt % PRINT_UNIT) == 0)
		dLoopCnt = (dCurrBlockCnt / PRINT_UNIT);
	else
		dLoopCnt = (dCurrBlockCnt / PRINT_UNIT) + 1;

	for(j = 0; j < dLoopCnt; j++) {

		sprintf(smsg.data, "--------------------------------------------------------");
		sprintf(szBuf, "\n%-16s (%7s)\t%-8s \t%-8s","PROCESS","  PID  ","STATUS", "VERSION");
		strcat(smsg.data, szBuf);
		sprintf(szBuf, "\n--------------------------------------------------------");
		strcat(smsg.data, szBuf);

		for(i = j*PRINT_UNIT; ((i < ((j+1)*PRINT_UNIT)) && (i < dCurrBlockCnt)); i++) {

			if(fidb->mpsw[i] == NORMAL)
				sprintf(szStatus, "ALIVE");
			else if(fidb->mpsw[i] == CRITICAL)
				sprintf(szStatus, "DEAD");
			else if(fidb->mpsw[i] == STOP)
				sprintf(szStatus, "STOP");
			else if (fidb->mpsw[i] == NOT_EQUIP)
				sprintf(szStatus, "NOT EQUIP");
			else
				continue;

			sprintf(szBuf, "\n%-16s (%7lld)\t%-8s \t%-8s",
				STR_TSW_COM[i], fidb->mpswinfo[i].pid, szStatus, g_stVersion->szVersion[SEQ_PROC_IDX[i]]);

			if(strlen(smsg.data) + strlen(szBuf) > MSG_DATA_LEN)
				break;
			strcat(smsg.data, szBuf);
		}

		sprintf(szBuf, "\n--------------------------------------------------------");
		strcat(smsg.data, szBuf);

		//dSlen = strlen(smsg.data) + 1;

		smsg.head.msg_len   = strlen(smsg.data) + 1;
		smsg.common.mml_err = DBM_SUCCESS;

		if(j != (dLoopCnt-1)) smsg.common.cont_flag = DBM_CONTINUE;
		else                  smsg.common.cont_flag = DBM_END;

		log_print(LOGN_INFO, "dLoopCnt=%d, j+1=%d, dCurrBlockCnt=%d", dLoopCnt, (j+1), dCurrBlockCnt);
		if( dSendMess(ml, &smsg, dLoopCnt, j+1) < 0 ){
			log_print(LOGN_CRI, "dDis_prc : Failed in dSendMess");
			return -1;
		}
	}

	log_print(LOGN_INFO,"MMCD]SUCCESS SEND DIS_PRC RESULT");
    return 1;

}/* end of dDis_prc */

int dDis_SysInfo(mml_msg *ml)
{
	int			i, dRet;
	dbm_msg_t	smsg;
	char		szBuf[MSG_DATA_LEN], sTemp[BUF_SIZE];

	sprintf(smsg.data, "-----------------------------------------------------");
	sprintf(szBuf, "\n%-10s  %7s  %15s  %15s","TYPE", "LOAD(%)", "MAX", "CUR");
	strcat(smsg.data, szBuf);
	sprintf(szBuf, "\n-----------------------------------------------------");
	strcat(smsg.data, szBuf);

	sprintf(szBuf, "\n%-10s  %7.1f  %15lld  %15lld", "CPU",
		(fidb->cpusts.lMax!=0ll)?(((float)fidb->cpusts.llCur/(float)fidb->cpusts.lMax)*100):0,
		fidb->cpusts.lMax, fidb->cpusts.llCur);
	strcat(smsg.data, szBuf);

	sprintf(szBuf, "\n%-10s  %7.1f  %15lld  %15lld", "MEM",
		(fidb->memsts.lMax!=0ll)?(((float)fidb->memsts.llCur/(float)fidb->memsts.lMax)*100):0,
		fidb->memsts.lMax, fidb->memsts.llCur);
	strcat(smsg.data, szBuf);

	sprintf(szBuf, "\n%-10s  %7.1f  %15lld  %15lld", "QUEUE",
		(fidb->queuests.lMax!=0ll)?(((float)fidb->queuests.llCur/(float)fidb->queuests.lMax)*100):0,
		fidb->queuests.lMax, fidb->queuests.llCur);
	strcat(smsg.data, szBuf);

	sprintf(szBuf, "\n%-10s  %7.1f  %15lld  %15lld", "NIFO",
		(fidb->nifosts.lMax!=0ll)?(((float)fidb->nifosts.llCur/(float)fidb->nifosts.lMax)*100):0,
		fidb->nifosts.lMax, fidb->nifosts.llCur);
	strcat(smsg.data, szBuf);

#if 0
/** 수집 장비 TRAFFIC */
	sprintf(szBuf, "\n%-10s  %7.1f  %15lld  %15lld", "TRAFFIC",
        (fidb->bytests.lMax!=0ll)?(((float)fidb->bytests.llCur/(float)fidb->bytests.lMax)*100):0,
        fidb->bytests.lMax, fidb->bytests.llCur);
    strcat(smsg.data, szBuf);
#endif

	for(i = 0 ; i < MAX_DISK_COUNT; i++)
	{
		sprintf(sTemp, "%s%d", "Disk", i+1);
		sprintf(szBuf, "\n%-10s  %7.1f  %15lld  %15lld", sTemp,
			(fidb->disksts[i].lMax!=0ll)?(((float)fidb->disksts[i].llCur/(float)fidb->disksts[i].lMax)*100):0,
			fidb->disksts[i].lMax, fidb->disksts[i].llCur);
		strcat(smsg.data, szBuf);
	}

	sprintf(szBuf, "\n-----------------------------------------------------");
	strcat(smsg.data, szBuf);
	sprintf(szBuf, "\n%-10s  %-7s","TYPE", "STATUS");
	strcat(smsg.data, szBuf);
	sprintf(szBuf, "\n-----------------------------------------------------");
	strcat(smsg.data, szBuf);

	for(i = 0; i < MAX_LINK_COUNT; i++)
	{
		sprintf(sTemp, "%s%d", "LAN", i+1);
		if(fidb->link[i] == NORMAL)
			sprintf(szBuf, "\n%-10s  %-7s", sTemp, "NORMAL");
		else if(fidb->link[i] == CRITICAL)
			sprintf(szBuf, "\n%-10s  %-7s", sTemp, "CRITICAL");
		else if(fidb->link[i] == NOT_EQUIP)
			sprintf(szBuf, "\n%-10s  %-7s", sTemp, "NOT_EQUIP");
		else
			continue;

		strcat(smsg.data, szBuf);
	}

	sprintf(szBuf, "\n-----------------------------------------------------");
	strcat(smsg.data, szBuf);

	//dSlen = strlen(smsg.data) + 1;
	smsg.head.msg_len     = strlen(smsg.data) +1;
	smsg.common.mml_err   = DBM_SUCCESS;
	smsg.common.cont_flag = DBM_END;

	if( (dRet = dSendMess(ml, &smsg, 1, 1)) < 0) {
		log_print(LOGN_CRI, LH"FAILED IN dSendMess() dRet[%d]", LT, dRet);
		return -1;
	}

	log_print(LOGN_INFO,"MMCD]SUCCESS SEND DIS_SYSINFO RESULT");
	return 1;
}

int dAct_prc(mml_msg *ml)
{
    int 		dRet, i, flag, dCmdLen, pid, dGetPid;
    dbm_msg_t 	smsg;
    char 		lstr[MMLCONTENTS];

    dCmdLen = strlen(ml->msg_body[0].para_cont);
    if(dCmdLen == 0) {
        smsg.common.mml_err   = eNeedProcName;
        smsg.common.cont_flag = DBM_END;
        smsg.head.msg_len     = 0;
        if( (dRet = dSendMess(ml,&smsg,0,0)) < 0 ){
            log_print(LOGN_CRI,"dAct_prc : Failed in dSendMess");
            return -1;
        }
        return 1;
    }

    memset(lstr, 0x00, MMLCONTENTS);
    for(i=0;i<dCmdLen;i++) {
        lstr[i] = toupper(ml->msg_body[0].para_cont[i]);
        if (lstr[i] == 0) break;
    }

    for(i=0,flag=1;i<dCurrBlockCnt;i++) {
        if (strcmp(lstr, STR_TSW_COM[i]) == 0) {
            flag = 0;
            break;
        }
    }

    memset(lstr,0x00,MMLCONTENTS);
    if (flag) {
        smsg.common.mml_err = eBlockNotRegistered;
        smsg.common.cont_flag = DBM_END;
        smsg.head.msg_len = 0;

        dRet = dSendMess(ml,&smsg,0,0);
        if(dRet < 0) {
            log_print(LOGN_CRI,"dAct_prc : Failed in dSendMess");
            return -1;
        }
        return 1;
    }

    if(fidb->mpsw[i] == NORMAL) {
        sprintf(smsg.data,
        "\n    BLOCK  = %s\n    STATUS = ALREADY ALIVE", STR_TSW_COM[i]);

        smsg.common.mml_err = eProcAliveError;
        smsg.common.cont_flag = DBM_END;
        smsg.head.msg_len = strlen(smsg.data) + 1;
        dRet = dSendMess(ml,&smsg,0,0);
        if(dRet < 0) {
            log_print(LOGN_CRI,"dAct_prc : Failed in dSendMess");
            return -1;
        }
        return 1;
    }
    else
    {
        if((pid = auto_restart_the_process(i)) > 0) {
            fidb->mpswinfo[i].pid = pid;
            fidb->mpswinfo[i].when = time(NULL);

            log_print(LOGN_CRI,"dAct_prc : START [%s]", STR_TSW_COM[i]);

            sprintf(smsg.data,
            "\n    BLOCK  = %s[PID=%d]\n    STATUS = ALIVE", STR_TSW_COM[i], pid);
            smsg.common.mml_err = DBM_SUCCESS;
            smsg.common.cont_flag = DBM_END;
            smsg.head.msg_len = strlen(smsg.data) + 1;

            dRet = dSendMess(ml,&smsg,0,0);
            if(dRet < 0) {
                log_print(LOGN_CRI,"dAct_prc : Failed in dSendMess");
                return -1;
            }

            for( i=1 ; i<dCurrBlockCnt; i++) {
                if(fidb->mpswinfo[i].pid != 0 && fidb->mpsw[i] == CRITICAL ) {
                    dGetPid = get_proc_id(STR_TSW_COM[i]);
                    if( dGetPid < 0 ) {
                        fidb->mpsw[i] = CRITICAL;
                    } else {
                        log_print(LOGN_DEBUG,"[START SUCCESS] [%s]", STR_TSW_COM[i]);
                        Send_CondMess( SYSTYPE_GTAM, LOCTYPE_PROCESS, INVTYPE_USERPROC, i, NORMAL, fidb->mpsw[i] );

                        fidb->mpsw[i] = NORMAL;
                    }
                }

            }

            return 1;
        } else {
            sprintf(smsg.data,
            "\n    BLOCK  = %s\n    STATUS = CANNOT BE ONLINE",
            STR_TSW_COM[i]);

            smsg.common.mml_err = eGeneralError;
            smsg.common.cont_flag = DBM_END;
            smsg.head.msg_len = strlen(smsg.data) + 1;

            dRet = dSendMess(ml,&smsg,0,0);
            if(dRet < 0) {
                log_print(LOGN_CRI,"dAct_prc : Failed in dSendMess");
                return -1;
            }
            return 1;
        }
    }
	log_print(LOGN_CRI,"MMCD]SUCCESS ACTIVATION PROCESS=%s", STR_TSW_COM[i]);
    return 0;
}


int dDact_prc(mml_msg *ml)
{
    int 		dRet;
    int 		i;
    int 		flag;
    int 		dCmdLen;
    dbm_msg_t 	smsg;
    char 		lstr[MMLCONTENTS];
    pid_t 		p;

    dCmdLen = strlen(ml->msg_body[0].para_cont);
    if(dCmdLen == 0) {
        smsg.common.mml_err = eNeedProcName;
        smsg.common.cont_flag = DBM_END;
        smsg.head.msg_len = 0;
        dRet = dSendMess(ml,&smsg,0,0);
        if(dRet < 0) {
            log_print(LOGN_CRI,"dDact_prc : Failed in dSendMess");
            return -1;
        }
        return 1;
    }

    memset(lstr,0x00,MMLCONTENTS);

    for(i=0;i<dCmdLen;i++) {
        lstr[i] = toupper(ml->msg_body[0].para_cont[i]);
        if (lstr[i] == 0) break;
    }


    for(i=0,flag=1;i<dCurrBlockCnt;i++) {
        if (strcmp(lstr, STR_TSW_COM[i]) == 0) {
            flag = 0;
            break;
        }
    }

    memset(lstr,0x00,MMLCONTENTS);
    if (flag) {
        smsg.common.mml_err = eBlockNotRegistered;
        smsg.common.cont_flag = DBM_END;
        smsg.head.msg_len = 0;

        dRet = dSendMess(ml,&smsg,0,0);
        if(dRet < 0) {
            log_print(LOGN_CRI,"dDact_prc : Failed in dSendMess");
            return -1;
        }
        return 1;
    }

    if((fidb->mpsw[i] == CRITICAL || fidb->mpsw[i] == STOP )) {
        sprintf(smsg.data,
        "\n    BLOCK  = %s\n    STATUS = ALREADY DEAD", STR_TSW_COM[i]);

        smsg.common.mml_err = eProcDeadError;
        smsg.common.cont_flag = DBM_END;
        smsg.head.msg_len = strlen(smsg.data) + 1;

        dRet = dSendMess(ml,&smsg,0,0);
        if(dRet < 0) {
            log_print(LOGN_CRI,"dDact_prc : Failed in dSendMess");
            return -1;
        }
        return 1;
    }
    else
    {
        if (i > 0 && i < dCurrBlockCnt) {
            if(fidb->mpswinfo[i].pid != 0 && fidb->mpsw[i] == NORMAL ) {
                p = get_proc_id(STR_TSW_COM[i]);
                if( p > 0 ) {
                    kill(p, SIGTERM);

                    Send_CondMess( SYSTYPE_GTAM, LOCTYPE_PROCESS, INVTYPE_USERPROC, i, STOP, fidb->mpsw[i] );

                    fidb->mpsw[i] = STOP;
                    fidb->mpswinfo[i].when = 0;
                    fidb->mpswinfo[i].pid = 0;

					log_print(LOGN_CRI,"dDact_prc : STOP [%s]", STR_TSW_COM[i]);

                    sprintf(smsg.data,
                    "\n    BLOCK  = %s [PID=%d]\n    STATUS = STOP", STR_TSW_COM[i], p);
                    smsg.common.mml_err = DBM_SUCCESS;
                    smsg.common.cont_flag = DBM_END;
                    smsg.head.msg_len = strlen(smsg.data) + 1;

                    dRet = dSendMess(ml,&smsg,0,0);
                    if(dRet < 0) {
                        log_print(LOGN_CRI,"dDact_prc : Failed in dSendMess");
                        return -1;
                    }
                    return 1;
                }
            } else {
                sprintf(smsg.data,
                "\n    BLOCK  = %s\n    STATUS = ALREADY DEAD", STR_TSW_COM[i]);
                smsg.common.mml_err = eProcDeadError;
                smsg.common.cont_flag = DBM_END;
                smsg.head.msg_len = strlen(smsg.data) + 1;

                dRet = dSendMess(ml,&smsg,0,0);
                if(dRet < 0) {
                    log_print(LOGN_CRI,"dDact_prc : Failed in dSendMess");
                    return -1;
                }
                return 1;
            }
        } else if (i == 0) {
            sprintf(smsg.data,
            "\n    BLOCK  = %s\n    STATUS = CANNOT BE KILLED\n    USE \"KillMC -b %s\" ON UNIX SHELL",
            STR_TSW_COM[i], STR_TSW_COM[i]);
            smsg.common.mml_err = eCHSMDNotDEAD;
            smsg.common.cont_flag = DBM_END;
            smsg.head.msg_len = strlen(smsg.data) + 1;

            dRet = dSendMess(ml,&smsg,0,0);
            if(dRet < 0) {
                log_print(LOGN_CRI,"dDact_prc : Failed in dSendMess");
                return -1;
            }
            return 1;
        } else {
            smsg.common.mml_err = eBlockNotRegistered;
            smsg.common.cont_flag = DBM_END;
            smsg.head.msg_len = strlen(smsg.data) + 1;
            dRet = dSendMess(ml,&smsg,0,0);
            if(dRet < 0) {
                log_print(LOGN_CRI,"dDact_prc : Failed in dSendMess");
                return -1;
            }
            return 1;
        }
    }
	log_print(LOGN_CRI,"MMCD]SUCCESS DEACTIVATION PROCESS=%s",STR_TSW_COM[i]);
	return 1;
}

int ntaf_proc_ctl(mml_msg *ml)
{
    int     	 dRet, dTAFID;
	dbm_msg_t 	 smsg;
    pst_MsgQ 	 pstMsgQ;
	pst_MsgQSub	 pstMsgQSub;
	U8		    *pNODE;

	dTAFID = atoi(ml->msg_body[0].para_cont);
	if( stSubSysMng.sys[dTAFID -1].usSysNo != dTAFID ) {

        smsg.common.mml_err   = eINVALID_SUBSYSTEM;
        smsg.common.cont_flag = DBM_END;
        smsg.head.msg_len     = 0;
        if(dSendMess(ml, &smsg,0,0) < 0) {
            log_print(LOGN_CRI, "ntaf_proc_ctl : Failed in dSendMMC");
            return -1;
        }
        return 1;
    }

	if( (dRet = dGetNode(&pNODE, &pstMsgQ)) < 0 ){
		log_print(LOGN_CRI,LH"FAILED IN dGetNode(CHSMD)",LT);
		return -2;
	}

	pstMsgQSub = (pst_MsgQSub)&pstMsgQ->llMType;

    pstMsgQSub->usType  = DEF_SYS;
    pstMsgQSub->usSvcID = SID_MML;
    pstMsgQSub->usMsgID = MID_MML_REQ;
    pstMsgQ->ucNTAFID   = dTAFID;

    log_print(LOGN_INFO, "[MMCD][SEND NTAF:%d] TYPE=%d SID=%d MID=%d MSG=%d NTAFID=%d",
		pstMsgQ->ucNTAFID,pstMsgQSub->usType, pstMsgQSub->usSvcID,
		pstMsgQSub->usMsgID, ml->msg_id, pstMsgQ->ucNTAFID);

    pstMsgQ->usBodyLen = DEF_MMLHEADER_SIZE;
    pstMsgQ->ucProID   = SEQ_PROC_CHSMD;
    memcpy(&pstMsgQ->szBody, ml, DEF_MMLHEADER_SIZE);
	if( dMsgsnd(SEQ_PROC_SI_SVC, pNODE) < 0 ){
        log_print(LOGN_CRI, LH"SEND FAIL for MMCD is not delivered to TAFID=%d"EH, LT, dTAFID, ET);
        return -3;
    }

    return 1;
}

int dMaskDirectPort(mml_msg *ml)
{
	int		  i, dRet;
	int		  dDirectNo, dPortType, dPortNo;
	size_t	  szLen;
	dbm_msg_t smsg;

	dDirectNo	= 0;
	dPortType	= 0;
	dPortNo		= 0;
	for(i = 0; i < ml->num_of_para; i++) {

		switch(ml->msg_body[i].para_id) {

			case 711:
				dDirectNo	= (unsigned int)atoi(ml->msg_body[i].para_cont);
				break;
			case 712:
				dPortType	= (unsigned int)atoi(ml->msg_body[i].para_cont);
				break;
			case 713:
				dPortNo		= (unsigned int)atoi(ml->msg_body[i].para_cont);
				break;
			default:
				log_print(LOGN_WARN, LH"INVALID para_id=%hu para_cont=%s",
					LT,ml->msg_body[i].para_id, ml->msg_body[i].para_cont);
		}
	}

	switch(dPortType) {

		case TYPE_MONITOR:

			if(!dPortNo && (dPortNo > MAX_MONITOR_PORT_COUNT)) {
		        smsg.common.mml_err		= eBadParameter;
		        smsg.common.cont_flag	= DBM_END;
		        smsg.head.msg_len		= 0;
				if( (dRet = dSendMess(ml, &smsg, 0, 0)) < 0) {

					log_print(LOGN_CRI, LH"ERROR IN dSendMess() dRet=%d", LT, dRet);
					return -1;
				}

				log_print(LOGN_WARN,LH"MASK. MONITOR.PORT_NO OVER=%d", LT, dPortNo );
				return -2;
			}

			if( (dRet = pthread_kill(stPthrdDir[dDirectNo-1].PthrdDirID, SIGUSR2)) != 0) {

				log_print(LOGN_CRI, LH"THREAD=%d, FAILED IN pthread_kill1(PthrdDirID=%lu, SIGUSR2=%d)"EH,
					LT, dDirectNo-1, stPthrdDir[dDirectNo-1].PthrdDirID, SIGUSR2, ET);
			}

			if( (dRet = pthread_mutex_lock(&stPthrdDir[dDirectNo-1].PthrdMutex)) != 0) {

				log_print(LOGN_CRI, LH"THREAD=%d, FAILED IN pthread_mutex_lock1(PthrdMutex) dRet=%d"EH,
					LT, dDirectNo-1, dRet, ET);
			}

			if( director->stDIRECT[dDirectNo-1].cMonitorPort[dPortNo-1] != (unsigned char)NOT_EQUIP){
				director->stDIRECT[dDirectNo-1].cMonitorPort[dPortNo-1] |= (unsigned char)MASK;
			}

			if( (dRet = pthread_mutex_unlock(&stPthrdDir[dDirectNo-1].PthrdMutex)) != 0) {

				log_print(LOGN_CRI, LH"THREAD=%d, FAILED IN pthread_mutex_lock2(PthrdMutex) dRet=%d"EH,
					LT, dDirectNo-1, dRet, ET);
			}
			break;

		case TYPE_MIRROR:
			if(!dPortNo) {
		        smsg.common.mml_err		= eBadParameter;
		        smsg.common.cont_flag	= DBM_END;
		        smsg.head.msg_len		= 0;
				if( (dRet = dSendMess(ml, &smsg, 0, 0)) < 0) {
					log_print(LOGN_CRI, LH"ERROR IN dSendMess() dRet=%d", LT, dRet);
					return -3;
				}
				log_print(LOGN_WARN, LH"MASK. MIRROR.PORT_NO OVER=%d", LT, dPortNo);
				return -4;
			}

			if( (dRet = pthread_kill(stPthrdDir[dDirectNo-1].PthrdDirID, SIGUSR2)) != 0) {

				log_print(LOGN_CRI, LH"THREAD=%d, FAILED IN pthread_kill2(PthrdDirID=%lu, SIGUSR2=%d)"EH,
					LT, dDirectNo-1, stPthrdDir[dDirectNo-1].PthrdDirID, SIGUSR2, ET);
			}

			if( (dRet = pthread_mutex_lock(&stPthrdDir[dDirectNo-1].PthrdMutex)) != 0) {

				log_print(LOGN_CRI, LH"THREAD=%d, FAILED IN pthread_mutex_lock3(PthrdMutex) dRet=%d"EH,
					LT, dDirectNo-1, dRet, ET);
			}

			if( director->stDIRECT[dDirectNo-1].cMirrorPort[dPortNo-1] != (unsigned char)NOT_EQUIP){
				director->stDIRECT[dDirectNo-1].cMirrorPort[dPortNo-1] |= (unsigned char)MASK;
			}

			if( (dRet = pthread_mutex_unlock(&stPthrdDir[dDirectNo-1].PthrdMutex)) != 0) {

				log_print(LOGN_CRI, LH"THREAD=%d, FAILED IN pthread_mutex_lock4(PthrdMutex) dRet=%d"EH,
					LT, dDirectNo-1, dRet, ET);
			}
			break;

		case TYPE_ALL:
			if(dPortNo) {
		        smsg.common.mml_err		= eBadParameter;
		        smsg.common.cont_flag	= DBM_END;
		        smsg.head.msg_len		= 0;
				if( (dRet = dSendMess(ml, &smsg, 0, 0)) < 0) {

					log_print(LOGN_CRI, LH"ERROR IN dSendMess() dRet=%d", LT, dRet);
					return -5;
				}
				log_print(LOGN_WARN, LH"MASK. DON'T NEED PORT_NO",LT);
				return -6;
			}

			if( (dRet = pthread_kill(stPthrdDir[dDirectNo-1].PthrdDirID, SIGUSR2)) != 0) {

				log_print(LOGN_CRI, LH"THREAD=%d, FAILED IN pthread_kill3(PthrdDirID=%lu, SIGUSR2=%d)"EH,
					LT, dDirectNo-1, stPthrdDir[dDirectNo-1].PthrdDirID, SIGUSR2, ET);
			}

			if( (dRet = pthread_mutex_lock(&stPthrdDir[dDirectNo-1].PthrdMutex)) != 0) {

				log_print(LOGN_CRI, LH"THREAD=%d, FAILED IN pthread_mutex_lock5(PthrdMutex) dRet=%d"EH,
					LT, dDirectNo-1, dRet, ET);
			}

			for(i = 0; i < MAX_MONITOR_PORT_COUNT; i++) {

				if( director->stDIRECT[dDirectNo-1].cMonitorPort[i] != (unsigned char)NOT_EQUIP){
					director->stDIRECT[dDirectNo-1].cMonitorPort[i] |= (unsigned char)MASK;
				}
			}

			for(i = 0; i < MAX_MIRROR_PORT_COUNT; i++) {

				if( director->stDIRECT[dDirectNo-1].cMirrorPort[i] != (unsigned char)NOT_EQUIP){
					director->stDIRECT[dDirectNo-1].cMirrorPort[i] |= (unsigned char)MASK;
				}
			}

			if( (dRet = pthread_mutex_unlock(&stPthrdDir[dDirectNo-1].PthrdMutex)) != 0) {
				log_print(LOGN_CRI, LH"THREAD=%d, FAILED IN pthread_mutex_lock6(PthrdMutex) dRet=%d"EH,
					LT, dDirectNo-1, dRet, ET);
			}
			break;

		case TYPE_POWER:
			if(!dPortNo && (dPortNo > MAX_DIRECT_POWER_COUNT)) {
		        smsg.common.mml_err		= eBadParameter;
		        smsg.common.cont_flag	= DBM_END;
		        smsg.head.msg_len		= 0;

				if( (dRet = dSendMess(ml, &smsg, 0, 0)) < 0) {

					log_print(LOGN_CRI, LH"ERROR IN dSendMess() dRet=%d", LT, dRet);
					return -7;
				}
				log_print(LOGN_WARN, LH"MASK. POWER_NO OVER=%d", LT, dPortNo);
				return -8;
			}

			if( (dRet = pthread_kill(stPthrdDir[dDirectNo-1].PthrdDirID, SIGUSR2)) != 0) {

				log_print(LOGN_CRI, LH"THREAD=%d, FAILED IN pthread_kill4(PthrdDirID=%lu, SIGUSR2=%d)"EH,
					LT, dDirectNo-1, stPthrdDir[dDirectNo-1].PthrdDirID, SIGUSR2, ET);
			}

			if( (dRet = pthread_mutex_lock(&stPthrdDir[dDirectNo-1].PthrdMutex)) != 0) {

				log_print(LOGN_CRI, LH"THREAD=%d, FAILED IN pthread_mutex_lock7(PthrdMutex) dRet=%d"EH,
					LT, dDirectNo-1, dRet, ET);
			}

			if( director->stDIRECT[dDirectNo-1].cPower[dPortNo-1] != (unsigned char)NOT_EQUIP){
				director->stDIRECT[dDirectNo-1].cPower[dPortNo-1] |= (unsigned char)MASK;
			}

			if( (dRet = pthread_mutex_unlock(&stPthrdDir[dDirectNo-1].PthrdMutex)) != 0) {

				log_print(LOGN_CRI, LH"THREAD=%d, FAILED IN pthread_mutex_lock8(PthrdMutex) dRet=%d"EH,
					LT, dDirectNo-1, dRet, ET);
			}
			break;

		default:
			log_print(LOGN_WARN, LH"INVALID dPortType=%d", LT, dPortType);
	        smsg.common.mml_err		= eBadParameter;
	        smsg.common.cont_flag	= DBM_END;
	        smsg.head.msg_len		= 0;

			if( (dRet = dSendMess(ml, &smsg, 0, 0)) < 0) {

				log_print(LOGN_CRI, LH"ERROR IN dSendMess() dRet=%d", LT, dRet);
				return -9;
			}
			return -10;
	}

	smsg.data[0] = 0x00;
	szLen = 1;
	smsg.common.mml_err		= DBM_SUCCESS;
	smsg.common.cont_flag	= DBM_END;
	smsg.head.msg_len		= szLen;

	if( (dRet = dSendMess(ml, &smsg, 0, 0)) < 0) {
		log_print(LOGN_CRI, LH"ERROR IN dSendMess() dRet=%d", LT, dRet);
		return -11;
	}

	if( TYPE_ALL != dPortType ) log_print(LOGN_CRI,"MMCD]SUCCESS MASKED DIRECTOR.%s:%d", TYPE_MONITOR==dPortType?"MONITOR":TYPE_MIRROR==dPortType?"MIRROR":"POWER",dPortNo);
	else						log_print(LOGN_CRI,"MMCD]SUCCESS MASKED DIRECTOR.TYPE:ALL");

	return 0;
}

int dUmaskDirectPort(mml_msg *ml)
{
	int			i, dRet;
	size_t		szLen;
	dbm_msg_t	smsg;
	int			dDirectNo, dPortType, dPortNo;

	dDirectNo	= 0;
	dPortType	= 0;
	dPortNo		= 0;
	for(i = 0; i < ml->num_of_para; i++) {

		switch(ml->msg_body[i].para_id) {

			case 711:
				dDirectNo	= (unsigned int)atoi(ml->msg_body[i].para_cont);
				break;
			case 712:
				dPortType	= (unsigned int)atoi(ml->msg_body[i].para_cont);
				break;
			case 713:
				dPortNo		= (unsigned int)atoi(ml->msg_body[i].para_cont);
				break;
			default:
				log_print(LOGN_WARN, LH"INVALID para_id=%hu, para_cont=%s", 
					LT, ml->msg_body[i].para_id, ml->msg_body[i].para_cont);
		}
	}

	switch(dPortType) {

		case TYPE_MONITOR:
			if(!dPortNo && (dPortNo > MAX_MONITOR_PORT_COUNT)) {
		        smsg.common.mml_err		= eBadParameter;
		        smsg.common.cont_flag	= DBM_END;
		        smsg.head.msg_len		= 0;
				if( (dRet = dSendMess(ml, &smsg, 0, 0)) < 0) {

					log_print(LOGN_CRI, LH"ERROR IN dSendMess() dRet=%d", LT, dRet);
					return -1;
				}
				log_print(LOGN_WARN, LH"UMASK. MONITOR.PORT_NO OVER=%d", LT, dPortNo );
				return -2;
			}

			if( (dRet = pthread_kill(stPthrdDir[dDirectNo-1].PthrdDirID, SIGUSR2)) != 0) {

				log_print(LOGN_CRI, LH"THREAD=%d, FAILED IN pthread_kill1(PthrdDirID=%lu, SIGUSR2=%d)"EH,
					LT, dDirectNo-1, stPthrdDir[dDirectNo-1].PthrdDirID, SIGUSR2, ET);
			}

			if( (dRet = pthread_mutex_lock(&stPthrdDir[dDirectNo-1].PthrdMutex)) != 0) {

				log_print(LOGN_CRI, LH"THREAD=%d, FAILED IN pthread_mutex_lock1(PthrdMutex) dRet=%d"EH,
					LT, dDirectNo-1, dRet, ET);
			}

			if( director->stDIRECT[dDirectNo-1].cMonitorPort[dPortNo-1] != (unsigned char)NOT_EQUIP){
				director->stDIRECT[dDirectNo-1].cMonitorPort[dPortNo-1] ^= (unsigned char)MASK;
			}

			if( (dRet = pthread_mutex_unlock(&stPthrdDir[dDirectNo-1].PthrdMutex)) != 0) {

				log_print(LOGN_CRI, LH"THREAD=%d, FAILED IN pthread_mutex_lock2(PthrdMutex) dRet=%d"EH,
					LT, dDirectNo-1, dRet, ET);
			}
			break;

		case TYPE_MIRROR:
			if(!dPortNo) {
		        smsg.common.mml_err		= eBadParameter;
		        smsg.common.cont_flag	= DBM_END;
		        smsg.head.msg_len		= 0;

				if( (dRet = dSendMess(ml, &smsg, 0, 0)) < 0) {
					log_print(LOGN_CRI, LH"ERROR IN dSendMess() dRet=%d", LT, dRet);
					return -3;
				}
				log_print(LOGN_WARN, LH"UMASK. MIRROR.PORT_NO OVER=%d", LT, dPortNo );
				return -4;
			}

			if( (dRet = pthread_kill(stPthrdDir[dDirectNo-1].PthrdDirID, SIGUSR2)) != 0) {

				log_print(LOGN_CRI, LH"THREAD=%d, FAILED IN pthread_kill2(PthrdDirID=%lu, SIGUSR2=%d)"EH,
					LT, dDirectNo-1, stPthrdDir[dDirectNo-1].PthrdDirID, SIGUSR2, ET);
			}

			if( (dRet = pthread_mutex_lock(&stPthrdDir[dDirectNo-1].PthrdMutex)) != 0) {

				log_print(LOGN_CRI, LH"THREAD=%d, FAILED IN pthread_mutex_lock3(PthrdMutex) dRet=%d"EH,
					LT, dDirectNo-1, dRet, ET);
			}

			if( director->stDIRECT[dDirectNo-1].cMirrorPort[dPortNo-1] != (unsigned char)NOT_EQUIP){
				director->stDIRECT[dDirectNo-1].cMirrorPort[dPortNo-1] ^= (unsigned char)MASK;
			}

			if( (dRet = pthread_mutex_unlock(&stPthrdDir[dDirectNo-1].PthrdMutex)) != 0) {

				log_print(LOGN_CRI, LH"THREAD=%d, FAILED IN pthread_mutex_lock(PthrdMutex) dRet=%d"EH,
					LT, dDirectNo-1, dRet, ET);
			}
			break;

		case TYPE_ALL:
			if(dPortNo) {
		        smsg.common.mml_err		= eBadParameter;
		        smsg.common.cont_flag	= DBM_END;
		        smsg.head.msg_len		= 0;

				if( (dRet = dSendMess(ml, &smsg, 0, 0)) < 0) {
					log_print(LOGN_CRI, LH"ERROR IN dSendMess() dRet=%d", LT, dRet);
					return -5;
				}
				log_print(LOGN_WARN,LH"UMASK, DON'T NEED PORT_NO", LT);
				return -6;
			}

			if( (dRet = pthread_kill(stPthrdDir[dDirectNo-1].PthrdDirID, SIGUSR2)) != 0) {

				log_print(LOGN_CRI, LH"THREAD=%d, FAILED IN pthread_kill3(PthrdDirID=%lu, SIGUSR2=%d)"EH,
					LT, dDirectNo-1, stPthrdDir[dDirectNo-1].PthrdDirID, SIGUSR2, ET);
			}

			if( (dRet = pthread_mutex_lock(&stPthrdDir[dDirectNo-1].PthrdMutex)) != 0) {

				log_print(LOGN_CRI, LH"THREAD=%d, FAILED IN pthread_mutex_lock5(PthrdMutex) dRet=%d"EH,
					LT, dDirectNo-1, dRet, ET);
			}

			for(i = 0; i < MAX_MONITOR_PORT_COUNT; i++)
			{
				if( director->stDIRECT[dDirectNo-1].cMonitorPort[i] != (unsigned char)NOT_EQUIP){
					director->stDIRECT[dDirectNo-1].cMonitorPort[i] ^= (unsigned char)MASK;
				}
			}

			for(i = 0; i < MAX_MIRROR_PORT_COUNT; i++) {
				if( director->stDIRECT[dDirectNo-1].cMirrorPort[i] != (unsigned char)NOT_EQUIP){
					director->stDIRECT[dDirectNo-1].cMirrorPort[i] ^= (unsigned char)MASK;
				}
			}

			if( (dRet = pthread_mutex_unlock(&stPthrdDir[dDirectNo-1].PthrdMutex)) != 0) {

				log_print(LOGN_CRI, LH"THREAD[%d] FAILED IN pthread_mutex_lock6(PthrdMutex) dRet=%d"EH,
					LT, dDirectNo-1, dRet, ET);
			}
			break;

		case TYPE_POWER:
			if(!dPortNo && (dPortNo > MAX_DIRECT_POWER_COUNT)) {
		        smsg.common.mml_err		= eBadParameter;
		        smsg.common.cont_flag	= DBM_END;
		        smsg.head.msg_len		= 0;

				if( (dRet = dSendMess(ml, &smsg, 0, 0)) < 0) {

					log_print(LOGN_CRI, LH"ERROR IN dSendMess() dRet=%d", LT, dRet);
					return -7;
				}
				log_print(LOGN_WARN, LH"UMASK. POWER_NO OVER=%d", LT, dPortNo );
				return -8;
			}

			if( (dRet = pthread_kill(stPthrdDir[dDirectNo-1].PthrdDirID, SIGUSR2)) != 0) {

				log_print(LOGN_CRI, LH"THREAD=%d, FAILED IN pthread_kill(PthrdDirID=%lu, SIGUSR2=%d)"EH,
					LT, dDirectNo-1, stPthrdDir[dDirectNo-1].PthrdDirID, SIGUSR2, ET);
			}

			if( (dRet = pthread_mutex_lock(&stPthrdDir[dDirectNo-1].PthrdMutex)) != 0) {

				log_print(LOGN_CRI, LH"THREAD=%d, FAILED IN pthread_mutex_lock(PthrdMutex) dRet=%d"EH,
					LT, dDirectNo-1, dRet, ET);
			}

			if( director->stDIRECT[dDirectNo-1].cPower[dPortNo-1] != (unsigned char)NOT_EQUIP){
				director->stDIRECT[dDirectNo-1].cPower[dPortNo-1] ^= (unsigned char)MASK;
			}

			if( (dRet = pthread_mutex_unlock(&stPthrdDir[dDirectNo-1].PthrdMutex)) != 0) {

				log_print(LOGN_CRI, LH"THREAD=%d, FAILED IN pthread_mutex_lock(PthrdMutex) dRet=%d"EH,
					LT, dDirectNo-1, dRet, ET);
			}
			break;

		default:
			log_print(LOGN_WARN, LH"INVALID dPortType=%hu", LT, dPortType);
	        smsg.common.mml_err		= eBadParameter;
	        smsg.common.cont_flag	= DBM_END;
	        smsg.head.msg_len		= 0;

			if( (dRet = dSendMess(ml, &smsg, 0, 0)) < 0) {

				log_print(LOGN_CRI, LH"ERROR IN dSendMess() dRet=%d", LT, dRet);
				return -9;
			}
			return -10;
	}

	smsg.data[0] = 0x00;
	szLen = 1;
	smsg.common.mml_err		= DBM_SUCCESS;
	smsg.common.cont_flag	= DBM_END;
	smsg.head.msg_len		= szLen;

	if( (dRet = dSendMess(ml, &smsg, 0, 0)) < 0) {
		log_print(LOGN_CRI, LH"ERROR IN dSendMess() dRet=%d", LT, dRet);
		return -11;
	}

	if( TYPE_ALL != dPortType ) log_print(LOGN_CRI,"MMCD. SUCCESS MASKED DIRECTOR.%s:%d", TYPE_MONITOR==dPortType?"MONITOR":TYPE_MIRROR==dPortType?"MIRROR":"POWER",dPortNo);
	else						log_print(LOGN_CRI,"MMCD. SUCCESS MASKED DIRECTOR.TYPE:ALL");

	return 0;
}

int dMaskSwitchPort(mml_msg *ml)
{
	int			i, dRet;
	size_t		szLen;
	dbm_msg_t	smsg;
	int		dSwitchNo, dPortType, dPortNo;

	dSwitchNo	= 0;
	dPortType	= 0;
	dPortNo		= 0;
	for(i = 0; i < ml->num_of_para; i++) {

		switch(ml->msg_body[i].para_id) {

			case 712:
				dPortType	= (unsigned int)atoi(ml->msg_body[i].para_cont);
				break;
			case 713:
				dPortNo		= (unsigned int)atoi(ml->msg_body[i].para_cont);
				break;
			case 714:
				dSwitchNo	= (unsigned int)atoi(ml->msg_body[i].para_cont);
				break;
			default:
				log_print(LOGN_WARN, LH"INVALID para_id=%hu, para_cont=%s",
					LT, ml->msg_body[i].para_id, ml->msg_body[i].para_cont);
				break;
		}
	}

	switch(dPortType) {

		case TYPE_ONE:
			if(!dPortNo) {
		        smsg.common.mml_err		= eBadParameter;
		        smsg.common.cont_flag	= DBM_END;
		        smsg.head.msg_len		= 0;

				if( (dRet = dSendMess(ml, &smsg, 0, 0)) < 0) {
					log_print(LOGN_CRI, LH"ERROR IN dSendMess() dRet=%d", LT, dRet);
					return -1;
				}
				log_print(LOGN_WARN, LH"MASK. SWITCH.PORT_NO NEEDS", LT);
				return -2;
			}

			if( (dRet = pthread_kill(stPthrdSW[dSwitchNo-1].PthrdDirID, SIGUSR2)) != 0) {

				log_print(LOGN_CRI, LH"THREAD=%d, FAILED IN pthread_kill1(PthrdDirID=%lu, SIGUSR2=%d)"EH,
					LT, dSwitchNo-1, stPthrdSW[dSwitchNo-1].PthrdDirID, SIGUSR2, ET);
			}

			if( (dRet = pthread_mutex_lock(&stPthrdSW[dSwitchNo-1].PthrdMutex)) != 0) {
				log_print(LOGN_CRI, LH"THREAD=%d, FAILED IN pthread_mutex_lock1(PthrdMutex) dRet=%d"EH,
					LT, dSwitchNo-1, dRet, ET);
			}

			if( swch->stSwitch[dSwitchNo-1].cSwitchPort[dPortNo-1] != (unsigned char)NOT_EQUIP){
				swch->stSwitch[dSwitchNo-1].cSwitchPort[dPortNo-1] |= (unsigned char)MASK;
			}

			if( (dRet = pthread_mutex_unlock(&stPthrdSW[dSwitchNo-1].PthrdMutex)) != 0) {

				log_print(LOGN_CRI, LH"THREAD=%d, FAILED IN pthread_mutex_lock2(PthrdMutex) dRet=%d"EH,
					LT, dSwitchNo-1, dRet, ET);
			}
			break;

		case TYPE_ALL:
			if(dPortNo) {
		        smsg.common.mml_err		= eBadParameter;
		        smsg.common.cont_flag	= DBM_END;
		        smsg.head.msg_len		= 0;

				if( (dRet = dSendMess(ml, &smsg, 0, 0)) < 0) {

					log_print(LOGN_CRI, LH"ERROR IN dSendMess() dRet=%d", LT, dRet);
					return -3;
				}
				log_print(LOGN_WARN, LH"MASK. SWITCH.PORT_NO DON'T NEEDS", LT);
				return -4;
			}

			if( (dRet = pthread_kill(stPthrdSW[dSwitchNo-1].PthrdDirID, SIGUSR2)) != 0) {

				log_print(LOGN_CRI, LH"THREAD=%d, FAILED IN pthread_kill2(PthrdDirID=%lu, SIGUSR2=%d"EH,
					LT, dSwitchNo-1, stPthrdSW[dSwitchNo-1].PthrdDirID, SIGUSR2, ET);
			}

			if( (dRet = pthread_mutex_lock(&stPthrdSW[dSwitchNo-1].PthrdMutex)) != 0) {

				log_print(LOGN_CRI, LH"THREAD=%d, FAILED IN pthread_mutex_lock3(PthrdMutex) dRet=%d"EH,
					LT, dSwitchNo-1, dRet, ET);
			}

			for(i = 0; i < MAX_SWITCH_PORT_COUNT; i++) {
				if( swch->stSwitch[dSwitchNo-1].cSwitchPort[i] != (unsigned char)NOT_EQUIP){
					swch->stSwitch[dSwitchNo-1].cSwitchPort[i] |= (unsigned char)MASK;
				}
			}

			if( (dRet = pthread_mutex_unlock(&stPthrdSW[dSwitchNo-1].PthrdMutex)) != 0) {

				log_print(LOGN_CRI, LH"THREAD=%d, FAILED IN pthread_mutex_lock(PthrdMutex) dRet=%d"EH,
					LT, dSwitchNo-1, dRet, ET);
			}
			break;

		default:
			log_print(LOGN_WARN, LH"INVALID dPortType=%d", LT, dPortType);
	        smsg.common.mml_err		= eBadParameter;
	        smsg.common.cont_flag	= DBM_END;
	        smsg.head.msg_len		= 0;

			if( (dRet = dSendMess(ml, &smsg, 0, 0)) < 0) {

				log_print(LOGN_CRI, LH"ERROR IN dSendMess() dRet=%d", LT, dRet);
				return -5;
			}
			return -6;
	}

	smsg.data[0] = 0x00;
	szLen = 1;
	smsg.common.mml_err		= DBM_SUCCESS;
	smsg.common.cont_flag	= DBM_END;
	smsg.head.msg_len		= szLen;

	if( (dRet = dSendMess(ml, &smsg, 0, 0)) < 0) {
		log_print(LOGN_CRI, LH"ERROR IN dSendMess() dRet=%d", LT, dRet);
		return -6;
	}

	if( TYPE_ALL == dPortType ) log_print(LOGN_CRI,"MMCD]SUCCESS MASKED SWITCH=%d.PORT=ALL",dSwitchNo);
	else 						log_print(LOGN_CRI,"MMCD]SUCCESS MASKED SWITCH=%d.PORT=%d",dSwitchNo, dPortType);
	return 0;
}

int dUmaskSwitchPort(mml_msg *ml)
{
	int			i, dRet;
	size_t		szLen;
	dbm_msg_t	smsg;
	int			dSwitchNo, dPortType, dPortNo;

	dSwitchNo	= 0;
	dPortType	= 0;
	dPortNo		= 0;
	for(i = 0; i < ml->num_of_para; i++) {

		switch(ml->msg_body[i].para_id) {

			case 712:
				dPortType	= (unsigned int)atoi(ml->msg_body[i].para_cont);
				break;
			case 713:
				dPortNo		= (unsigned int)atoi(ml->msg_body[i].para_cont);
				break;
			case 714:
				dSwitchNo	= (unsigned int)atoi(ml->msg_body[i].para_cont);
				break;
			default:
				log_print(LOGN_WARN, LH"INVALID para_id=%hu, para_cont=%s",
					LT, ml->msg_body[i].para_id, ml->msg_body[i].para_cont);
				break;
		}
	}

	switch(dPortType) {

		case TYPE_ONE:
			if(!dPortNo) {
		        smsg.common.mml_err		= eBadParameter;
		        smsg.common.cont_flag	= DBM_END;
		        smsg.head.msg_len		= 0;

				if( (dRet = dSendMess(ml, &smsg, 0, 0)) < 0) {
					log_print(LOGN_CRI, LH"ERROR IN dSendMess() dRet=%d", LT, dRet);
					return -1;
				}
				log_print(LOGN_WARN, LH"UMASK. SWITCH.PORT_NO NEEDS", LT);
				return -2;
			}

			if( (dRet = pthread_kill(stPthrdSW[dSwitchNo-1].PthrdDirID, SIGUSR2)) != 0) {

				log_print(LOGN_CRI, LH"THREAD=%d, FAILED IN pthread_kill1(PthrdDirID=%lu, SIGUSR2=%d)"EH,
					LT, dSwitchNo-1, stPthrdSW[dSwitchNo-1].PthrdDirID, SIGUSR2, ET);
			}

			if( (dRet = pthread_mutex_lock(&stPthrdSW[dSwitchNo-1].PthrdMutex)) != 0) {

				log_print(LOGN_CRI, LH"THREAD=%d, FAILED IN pthread_mutex_lock1(PthrdMutex) dRet=%d"EH,
					LT, dSwitchNo-1, dRet, ET);
			}

			if( swch->stSwitch[dSwitchNo-1].cSwitchPort[dPortNo-1] != (unsigned char)NOT_EQUIP){
				swch->stSwitch[dSwitchNo-1].cSwitchPort[dPortNo-1] ^= (unsigned char)MASK;
			}

			if( (dRet = pthread_mutex_unlock(&stPthrdSW[dSwitchNo-1].PthrdMutex)) != 0) {

				log_print(LOGN_CRI, LH"THREAD=%d, FAILED IN pthread_mutex_lock(PthrdMutex) dRet=%d"EH,
					LT, dSwitchNo-1, dRet, ET);
			}
			break;

		case TYPE_ALL:

			if(dPortNo) {
		        smsg.common.mml_err		= eBadParameter;
		        smsg.common.cont_flag	= DBM_END;
		        smsg.head.msg_len		= 0;

				if( (dRet = dSendMess(ml, &smsg, 0, 0)) < 0) {
					log_print(LOGN_CRI, LH"ERROR IN dSendMess() dRet=%d", LT, dRet);
					return -3;
				}
				log_print(LOGN_WARN, LH"UMASK. SWITCH.PORT_NO DON'T NEEDS", LT);
				return -4;
			}

			if( (dRet = pthread_kill(stPthrdSW[dSwitchNo-1].PthrdDirID, SIGUSR2)) != 0)
			{
				log_print(LOGN_CRI, LH"THREAD=%d, FAILED IN pthread_kill2(PthrdDirID=%lu, SIGUSR2=%d)"EH,
					LT, dSwitchNo-1, stPthrdSW[dSwitchNo-1].PthrdDirID, SIGUSR2, ET);
			}

			if( (dRet = pthread_mutex_lock(&stPthrdSW[dSwitchNo-1].PthrdMutex)) != 0) {
				log_print(LOGN_CRI, LH"THREAD=%d, FAILED IN pthread_mutex_lock3(PthrdMutex) dRet=%d"EH,
					LT, dSwitchNo-1, dRet, ET);
			}

			for(i = 0; i < MAX_SWITCH_PORT_COUNT; i++) {
				if( swch->stSwitch[dSwitchNo-1].cSwitchPort[i] != (unsigned char)NOT_EQUIP){
					swch->stSwitch[dSwitchNo-1].cSwitchPort[i] ^= (unsigned char)MASK;
				}
			}

			if( (dRet = pthread_mutex_unlock(&stPthrdSW[dSwitchNo-1].PthrdMutex)) != 0) {

				log_print(LOGN_CRI, LH"THREAD=%d, FAILED IN pthread_mutex_lock4(PthrdMutex) dRet=%d"EH,
					LT, dSwitchNo-1, dRet, ET);
			}
			break;

		default:
			log_print(LOGN_WARN, LH"INVALID dPortType=%hu", LT, dPortType);
	        smsg.common.mml_err		= eBadParameter;
	        smsg.common.cont_flag	= DBM_END;
	        smsg.head.msg_len		= 0;

			if( (dRet = dSendMess(ml, &smsg, 0, 0)) < 0) {

				log_print(LOGN_CRI, LH"ERROR IN dSendMess() dRet=%d", LT, dRet);
				return -5;
			}
			return -6;
	}

	smsg.data[0] = 0x00;
	szLen = 1;
	smsg.common.mml_err		= DBM_SUCCESS;
	smsg.common.cont_flag	= DBM_END;
	smsg.head.msg_len		= szLen;
	if( (dRet = dSendMess(ml, &smsg, 0, 0)) < 0) {
		log_print(LOGN_CRI, LH"ERROR IN dSendMess() dRet=%d", LT, dRet);
		return -6;
	}

	if( TYPE_ALL == dPortType ) log_print(LOGN_CRI,"MMCD]SUCCESS UNMASKED SWITCH=%d.PORT=ALL",dSwitchNo);
	else 						log_print(LOGN_CRI,"MMCD]SUCCESS UNMASKED SWITCH=%d.PORT=%d",dSwitchNo, dPortType);
	return 0;
}

int taf_sys_info(mml_msg *ml)
{
	int			dRet, dTAFID;
	dbm_msg_t	smsg;
	pst_MsgQ	pstMsgQ;
	pst_MsgQSub	pstMsgQSub;
	U8		   *pNODE;

	dTAFID = (unsigned char)atoi(ml->msg_body[0].para_cont);
	if(stSubSysMng.sys[dTAFID -1].usSysNo != dTAFID)
	{
		smsg.common.mml_err		= eINVALID_SUBSYSTEM;
		smsg.common.cont_flag	= DBM_END;
		smsg.head.msg_len		= 0;
		if( (dRet = dSendMess(ml, &smsg, 0, 0)) < 0)
		{
			log_print(LOGN_CRI, LH"ERROR IN dSendMess() dRet[%d]", LT, dRet);
			return -1;
		}
		return -2;
	}

	if( dGetNode(&pNODE, &pstMsgQ) < 0 ){
		log_print(LOGN_CRI,LH"FAILED IN dGetNode(CHSMD)",LT);
        return -3;
	}

	pstMsgQSub = (pst_MsgQSub)&pstMsgQ->llMType;

	pstMsgQSub->usType	= DEF_SYS;
	pstMsgQSub->usSvcID	= SID_MML;
	pstMsgQSub->usMsgID	= MID_MML_REQ;
	pstMsgQ->ucNTAFID   = dTAFID;

	log_print(LOGN_INFO, LH"[SEND NTAF:%hu] TYPE[%u] SID[%hu] MID[%hu] MSG[%hu] NTAFID[%hu]", LT,
		pstMsgQ->ucNTAFID, pstMsgQSub->usType, pstMsgQSub->usSvcID, pstMsgQSub->usMsgID, ml->msg_id, pstMsgQ->ucNTAFID);

	pstMsgQ->usBodyLen	= DEF_MMLHEADER_SIZE;
	pstMsgQ->ucProID	= SEQ_PROC_CHSMD;

	memcpy(&pstMsgQ->szBody[0], ml, DEF_MMLHEADER_SIZE);
	if( dMsgsnd(SEQ_PROC_SI_SVC, pNODE) < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN dMsgsnd(SI_SVC)"EH, LT, ET);
		return -4;
	}

	log_print(LOGN_DEBUG, LH"SUCCESS IN dMsgsnd(SI_SVC) MESSAGE PROCESSID=%d", LT, SEQ_PROC_SI_SVC);
	return 0;
}

int dSendMess(mml_msg *ml,dbm_msg_t *smsg, int dTotPage, int dCurPage )
{
    pst_MsgQ    pstSndMsg;
    pst_MsgQSub pstMsgQSub;
	U8		   *pNODE;

	if( dGetNode(&pNODE, &pstSndMsg) < 0 ){
		log_print(LOGN_CRI,LH"FAILED IN dGetNode(CHSMD)",LT);
        return -1;
	}

    smsg->head.src_proc = SEQ_PROC_CHSMD;
    smsg->head.dst_func = ml->src_func;
    smsg->head.dst_proc = ml->src_proc;
    smsg->head.cmd_id = ml->cmd_id;
    smsg->head.msg_id = ml->msg_id;

    smsg->common.TotPage = dTotPage;
    smsg->common.CurPage = dCurPage;

    pstMsgQSub = (pst_MsgQSub)&pstSndMsg->llMType;

    pstMsgQSub->usType = DEF_SYS;
    pstMsgQSub->usSvcID = SID_MML;
    pstMsgQSub->usMsgID = MID_MML_RST;

    util_makenid(SEQ_PROC_CHSMD,&pstSndMsg->llNID);
    pstSndMsg->llIndex   = 1;
    pstSndMsg->usRetCode = 0;

    pstSndMsg->usBodyLen = sizeof(dbm_msg_t)-MSG_DATA_LEN+smsg->head.msg_len;
    memcpy( pstSndMsg->szBody, smsg, pstSndMsg->usBodyLen );

	if( dMsgsnd(SEQ_PROC_MMCD, pNODE) < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN dMsgsnd(MMCD)"EH, LT, ET);
		return -2;
	}
	log_print(LOGN_DEBUG, LH"SUCCESS IN dMsgsnd(MMCD) MESSAGE PROCESSID=%d", LT, SEQ_PROC_MMCD);

    return 1;
}
