/**A.1*  File Inclusion *******************************************************/

/* SYS HEADER */
#include <stdio.h>
#include <string.h>
#include <signal.h>		/* SIGTERM */
#include <errno.h>
/* LIB HEADER */
#include "clisto.h"		/* U8 */
#include "filedb.h"
#include "loglib.h"
#include "utillib.h"	/* util_makenid() */
#include "verlib.h"		/* st_VERSION */
/* PRO HEADER */
#include "path.h"
#include "msgdef.h"		/* st_MsgQ */
#include "mmcdef.h"
#include "sockio.h"		/* NTAFT_HEADER_LEN */
#include "procid.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
#include "almstat.h"
/* LOC HEADER */
#include "chsmd_msg.h"	/* Send_AlmMsg() */
#include "chsmd_sw.h"	/* get_proc_id() */
#include "chsmd_func.h"	/* dMsgsnd(), dGetNode() */
#include "chsmd_mmc.h"

/**B.1*  Definition of New Constants ******************************************/
/**B.2*  Definition of New Type  **********************************************/
/**C.1*  Declaration of Variables ( Local ) ***********************************/
st_Version	*version;
int 		 SEQ_PROC_IDX[MAX_NTAF_SW_BLOCK];

/**C.2*  Declaration of Variables ( External ) ********************************/
extern st_NTAF *fidb;

extern int  dCurrBlockCnt;
extern int  gdSysNo;

extern char STR_TSW_COM[MAX_NTAF_SW_BLOCK][30];

/**D.1*  Definition of Functions  *********************************************/
/**D.2*  Definition of Functions  *********************************************/

int dDis_prc(mml_msg *ml)
{
	int			i, dRet, dSlen, dVerCount, dLineLen;
	dbm_msg_t	smsg;
	char		szBuf[MSG_DATA_LEN], szBuf2[MSG_DATA_LEN], STR_VER_COM[MAX_NTAF_SW_BLOCK][30], szStatus[16], sVersion[16];

	memset(STR_VER_COM, 0x00, 30*MAX_NTAF_SW_BLOCK);
	if( (dVerCount = dGetVerBlock(FILE_MC_INIT, STR_VER_COM)) < 0)
		log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN dGetVerBlock(%s) dVerCount[%d]", __FILE__, __FUNCTION__, __LINE__, FILE_MC_INIT, dVerCount);

	sprintf(smsg.data, "---------------------------------------------------");
	dLineLen = strlen(smsg.data);
	sprintf(szBuf, "\n%-11s (%7s)\t%-8s\t%-s", "PROCESS", "  PID  ", "STATUS", "VERSION");
	strcat(smsg.data, szBuf);
	sprintf(szBuf, "\n---------------------------------------------------");
	strcat(smsg.data, szBuf);

	for(i = 0; i < dVerCount ;i++) {

		sprintf(sVersion, "-");
		switch(fidb->mpsw[i]){
			case NORMAL:    
				sprintf(szStatus, "ALIVE");
				sprintf(sVersion, "%s", version->szVersion[SEQ_PROC_IDX[i]]);
				break;
			case CRITICAL:  sprintf(szStatus, "DEAD"); break;
			case STOP:      sprintf(szStatus, "STOP"); break;
			case NOT_EQUIP: sprintf(szStatus, "NOT EQUIP"); break;
			default: continue;

		}

		sprintf(szBuf, "\n%-11s (%7lld)\t%-8s\t%-8s", STR_TSW_COM[i], fidb->mpswinfo[i].pid, szStatus, sVersion);

		if( (strlen(smsg.data)+strlen(szBuf))>(MSG_DATA_LEN-dLineLen)) {
			sprintf(szBuf2, "\n---------------------------------------------------");
			strcat(smsg.data, szBuf2);
			smsg.common.mml_err   = DBM_SUCCESS;
			smsg.common.cont_flag = DBM_CONTINUE;
			smsg.head.msg_len     = strlen(smsg.data)+1;
			if( (dRet = dSendToNtam(ml,&smsg)) < 0) {
				log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN dSendToNtam() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
				return -2;
			}
			sprintf(smsg.data, "---------------------------------------------------");
		}
		strcat(smsg.data,szBuf);
	}
    sprintf(szBuf, "\n---------------------------------------------------");
    strcat(smsg.data, szBuf);

    dSlen	= strlen(smsg.data) + 1;
    smsg.common.mml_err		= DBM_SUCCESS;
    smsg.common.cont_flag	= DBM_END;
    smsg.head.msg_len		= dSlen;
    if( (dRet = dSendToNtam(ml,&smsg)) < 0)
    {
        log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN dSendToNtam() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
        return -2;
    }

    return 1;

}/* end of dDis_prc */


#if 0 /* BACKUP by uamyd 20110531 */
int dDis_prc(mml_msg *ml)
{
	int			i, j, dRet, dSlen, dVerCount;
	dbm_msg_t	smsg;
	char		szBuf[MSG_DATA_LEN], STR_VER_COM[MAX_NTAF_SW_BLOCK][30], szStatus[16], sVersion[16];

	memset(STR_VER_COM, 0x00, 30*MAX_NTAF_SW_BLOCK);
	if( (dVerCount = dGetVerBlock(MC_INIT, STR_VER_COM)) < 0)
		log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN dGetVerBlock(%s) dVerCount[%d]", __FILE__, __FUNCTION__, __LINE__, MC_INIT, dVerCount);

	sprintf(smsg.data, "---------------------------------------------------");
	sprintf(szBuf, "\n%-11s (%7s)\t%-8s\t%-s", "PROCESS", "  PID  ", "STATUS", "VERSION");
	strcat(smsg.data, szBuf);
	sprintf(szBuf, "\n---------------------------------------------------");
	strcat(smsg.data, szBuf);

	for(i = 0; i < dCurrBlockCnt ;i++)
	{
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

		for(j = 0; j < dVerCount; j++)
		{
			if(strcmp(STR_TSW_COM[i], STR_VER_COM[j]) == 0){
				if( SEQ_PROC_IDX[j] < MAX_NTAF_SW_BLOCK )
				sprintf(sVersion, "%s", version->ver[SEQ_PROC_IDX[j]]);
				else
				sprintf(sVersion, "-");
			}
		}

		sprintf(szBuf, "\n%-11s (%7lld)\t%-8s\t%-8s", STR_TSW_COM[i], fidb->mpswinfo[i].pid, szStatus, sVersion);

		if( (strlen(smsg.data)+strlen(szBuf))>(MSG_DATA_LEN-strlen("\n---------------------------------------------------")))
		{
			log_print(LOGN_CRI, "F=%s:%s.%d: smsg.data[%lu] is longer than MSG_DATA_LEN[%d]", __FILE__, __FUNCTION__, __LINE__,
				(strlen(smsg.data)+strlen(szBuf)+strlen("\n---------------------------------------------------")), MSG_DATA_LEN);
			return -1;
		}
		strcat(smsg.data,szBuf);
	}
    sprintf(szBuf, "\n---------------------------------------------------");
    strcat(smsg.data, szBuf);

    dSlen	= strlen(smsg.data) + 1;
    smsg.common.mml_err		= DBM_SUCCESS;
    smsg.common.cont_flag	= DBM_END;
    smsg.head.msg_len		= dSlen;
    if( (dRet = dSendToNtam(ml,&smsg)) < 0)
    {
        log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN dSendToNtam() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
        return -2;
    }

    return 1;

}/* end of dDis_prc */
#endif

int dSysInfo(mml_msg *ml)
{
	int		   i, dRet;
	short	   hStrLen;
	dbm_msg_t  smsg;
	char       szBuf[MSG_DATA_LEN], sTemp[BUF_SIZE];

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
		(fidb->quests.lMax!=0ll)?(((float)fidb->quests.llCur/(float)fidb->quests.lMax)*100):0,
		fidb->quests.lMax, fidb->quests.llCur);
	strcat(smsg.data, szBuf);

	sprintf(szBuf, "\n%-10s  %7.1f  %15lld  %15lld", "NIFO",
		(fidb->nifosts.lMax!=0ll)?(((float)fidb->nifosts.llCur/(float)fidb->nifosts.lMax)*100):0,
		fidb->nifosts.lMax, fidb->nifosts.llCur);
	strcat(smsg.data, szBuf);

	sprintf(szBuf, "\n%-10s  %7.1f  %15lld  %15lld", "TRAFFIC",
		(fidb->bytests.lMax!=0ll)?(((float)fidb->bytests.llCur/(float)fidb->bytests.lMax)*100):0,
		fidb->bytests.lMax, fidb->bytests.llCur);
	strcat(smsg.data, szBuf);

	for(i = 0 ; i < MAX_NTAF_DISK_COUNT-2; i++)
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

	for(i = 0; i < MAX_NTAF_LINK; i++)
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

	hStrLen = (short)strlen(smsg.data) + 1;
	smsg.common.mml_err = DBM_SUCCESS;
	smsg.common.cont_flag = DBM_END;
	smsg.head.msg_len = hStrLen;

	if( (dRet = dSendToNtam(ml,&smsg)) < 0)
	{
		log_print(LOGN_CRI, "F=%s:%s.%d: ERROR IN dSendToNtam() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		return -1;
	}

	return 0;
}

int dAct_prc(mml_msg *ml)
{
    int 		dRet;
    int 		i;
    int 		flag;
    int 		dCmdLen;
    dbm_msg_t 	smsg;
    char 		lstr[MMLCONTENTS];
    int 		pid, dGetPid;


    dCmdLen = strlen(ml->msg_body[1].para_cont);
    if(dCmdLen == 0)
    {
        smsg.common.mml_err = eNeedProcName;
        smsg.common.cont_flag = DBM_END;
        smsg.head.msg_len = 0;
        dRet = dSendToNtam(ml,&smsg);
        if(dRet < 0)
        {
            log_print(LOGN_CRI,"dAct_prc : Failed in dSendToNtam");
            return -1;
        }
        return 1;
    }

    memset(lstr, 0x00, MMLCONTENTS);
    for(i=0;i<dCmdLen;i++)
    {
        lstr[i] = toupper(ml->msg_body[1].para_cont[i]);
        if (lstr[i] == 0) break;
    }

    for(i=0,flag=1;i<dCurrBlockCnt;i++)
    {
        if (strcmp(lstr, STR_TSW_COM[i]) == 0)
        {
            flag = 0;
            break;
        }
    }

    memset(lstr,0x00,MMLCONTENTS);
    if (flag)
    {
        smsg.common.mml_err = eBlockNotRegistered;
        smsg.common.cont_flag = DBM_END;
        smsg.head.msg_len = 0;
        dRet = dSendToNtam(ml,&smsg);
        if(dRet < 0)
        {
            log_print(LOGN_CRI,"dAct_prc : Failed in dSendToNtam");
            return -1;
        }
        return 1;
    }

    if(fidb->mpsw[i] == NORMAL)
    {
        sprintf(smsg.data,
        "\n    BLOCK  = %s\n    STATUS = ALREADY ALIVE", STR_TSW_COM[i]);

        smsg.common.mml_err = eProcAliveError;
        smsg.common.cont_flag = DBM_END;
        smsg.head.msg_len = strlen(smsg.data) + 1;
        dRet = dSendToNtam(ml,&smsg);
        if(dRet < 0)
        {
            log_print(LOGN_CRI,"dAct_prc : Failed in dSendToNtam");
            return -1;
        }
        return 1;
    }
    else
    {
        if((pid = auto_restart_the_process(i)) > 0)
        {
            fidb->mpswinfo[i].pid = pid;
            fidb->mpswinfo[i].when = time(NULL);

            log_print(LOGN_CRI,"dAct_prc : START [%s]", STR_TSW_COM[i]);

            sprintf(smsg.data,
            "\n    BLOCK  = %s[PID=%d]\n    STATUS = ALIVE", STR_TSW_COM[i], pid);
            smsg.common.mml_err = DBM_SUCCESS;
            smsg.common.cont_flag = DBM_END;
            smsg.head.msg_len = strlen(smsg.data) + 1;
            dRet = dSendToNtam(ml,&smsg);
            if(dRet < 0)
            {
                log_print(LOGN_CRI,"dAct_prc : Failed in dSendToNtam");
                return -1;
            }

            for( i=1 ; i<dCurrBlockCnt; i++)
            {
                if(fidb->mpswinfo[i].pid != 0 && fidb->mpsw[i] == CRITICAL )
                {
                    dGetPid = get_proc_id(STR_TSW_COM[i]);
                    if( dGetPid < 0 )
                    {
                        fidb->mpsw[i] = CRITICAL;
                    }
                    else
                    {
                        log_print(LOGN_DEBUG,"[START SUCCESS] [%s]", STR_TSW_COM[i]);
						Send_AlmMsg( LOCTYPE_PROCESS, INVTYPE_USERPROC, i, NORMAL, fidb->mpsw[i] );

                        fidb->mpsw[i] = NORMAL;
                    }
                }

            }

            return 1;
        }
        else
        {
            sprintf(smsg.data,
            "\n    BLOCK  = %s\n    STATUS = CANNOT BE ONLINE",
            STR_TSW_COM[i]);

            smsg.common.mml_err = eGeneralError;
            smsg.common.cont_flag = DBM_END;
            smsg.head.msg_len = strlen(smsg.data) + 1;
            dRet = dSendToNtam(ml,&smsg);
            if(dRet < 0)
            {
                log_print(LOGN_CRI,"dAct_prc : Failed in dSendToNtam");
                return -1;
            }
            return 1;
        }
    }

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

    dCmdLen = strlen(ml->msg_body[1].para_cont);
    if(dCmdLen == 0)
    {
        smsg.common.mml_err = eNeedProcName;
        smsg.common.cont_flag = DBM_END;
        smsg.head.msg_len = 0;
        dRet = dSendToNtam(ml,&smsg);
        if(dRet < 0)
        {
            log_print(LOGN_CRI,"dDact_prc : Failed in dSendToNtam");
            return -1;
        }
        return 1;
    }

    memset(lstr,0x00,MMLCONTENTS);

    for(i=0;i<dCmdLen;i++)
    {
        lstr[i] = toupper(ml->msg_body[1].para_cont[i]);
        if (lstr[i] == 0) break;
    }

	log_print(LOGN_INFO, "MML : PROCNAME[%s]", lstr);


    for(i=0,flag=1;i<dCurrBlockCnt;i++)
    {
        if (strcmp(lstr, STR_TSW_COM[i]) == 0)
        {
            flag = 0;
            break;
        }
    }

    memset(lstr,0x00,MMLCONTENTS);
    if (flag)
    {
        smsg.common.mml_err = eBlockNotRegistered;
        smsg.common.cont_flag = DBM_END;
        smsg.head.msg_len = 0;
        dRet = dSendToNtam(ml,&smsg);
        if(dRet < 0)
        {
            log_print(LOGN_CRI,"dDact_prc : Failed in dSendToNtam");
            return -1;
        }
        return 1;
    }

    if((fidb->mpsw[i] == CRITICAL || fidb->mpsw[i] == STOP))
    {
        sprintf(smsg.data,
        "\n    BLOCK  = %s\n    STATUS = ALREADY DEAD", STR_TSW_COM[i]);

        smsg.common.mml_err = eProcDeadError;
        smsg.common.cont_flag = DBM_END;
        smsg.head.msg_len = strlen(smsg.data) + 1;
        dRet = dSendToNtam(ml,&smsg);
        if(dRet < 0)
        {
            log_print(LOGN_CRI,"dDact_prc : Failed in dSendToNtam");
            return -1;
        }
        return 1;
    }
    else
    {
        if (i > 0 && i < dCurrBlockCnt)
        {
            if(fidb->mpswinfo[i].pid != 0 && fidb->mpsw[i] == NORMAL )
            {
                p = get_proc_id(STR_TSW_COM[i]);
                if( p > 0 )
                {
                    kill(p, SIGTERM);
					Send_AlmMsg( LOCTYPE_PROCESS, INVTYPE_USERPROC, i, STOP, fidb->mpsw[i] );

                    fidb->mpsw[i] = STOP;
                    fidb->mpswinfo[i].when = 0;
                    fidb->mpswinfo[i].pid = 0;

					log_print(LOGN_CRI,"dDact_prc : STOP [%s]", STR_TSW_COM[i]);

                    sprintf(smsg.data,
                    "\n    BLOCK  = %s [PID=%d]\n    STATUS = STOP", STR_TSW_COM[i], p);
                    smsg.common.mml_err = DBM_SUCCESS;
                    smsg.common.cont_flag = DBM_END;
                    smsg.head.msg_len = strlen(smsg.data) + 1;
                    dRet = dSendToNtam(ml,&smsg);
                    if(dRet < 0)
                    {
                        log_print(LOGN_CRI,"dDact_prc : Failed in dSendToNtam");
                        return -1;
                    }
                    return 1;
                }
            }
            else
            {
                sprintf(smsg.data,
                "\n    BLOCK  = %s\n    STATUS = ALREADY DEAD", STR_TSW_COM[i]);
                smsg.common.mml_err = eProcDeadError;
                smsg.common.cont_flag = DBM_END;
                smsg.head.msg_len = strlen(smsg.data) + 1;
                dRet = dSendToNtam(ml,&smsg);
                if(dRet < 0)
                {
                    log_print(LOGN_CRI,"dDact_prc : Failed in dSendToNtam");
                    return -1;
                }
                return 1;
            }
        }
        else if (i == 0)
        {
            sprintf(smsg.data,
            "\n    BLOCK  = %s\n    STATUS = CANNOT BE KILLED\n    USE \"KillMC -b %s\" ON UNIX SHELL",
            STR_TSW_COM[i], STR_TSW_COM[i]);
            smsg.common.mml_err = eCHSMDNotDEAD;
            smsg.common.cont_flag = DBM_END;
            smsg.head.msg_len = strlen(smsg.data) + 1;
            dRet = dSendToNtam(ml,&smsg);
            if(dRet < 0)
            {
                log_print(LOGN_CRI,"dDact_prc : Failed in dSendToNtam");
                return -1;
            }
            return 1;
        }
        else
        {
            smsg.common.mml_err = eBlockNotRegistered;
            smsg.common.cont_flag = DBM_END;
            smsg.head.msg_len = strlen(smsg.data) + 1;
            dRet = dSendToNtam(ml,&smsg);
            if(dRet < 0)
            {
                log_print(LOGN_CRI,"dDact_prc : Failed in dSendToNtam");
                return -1;
            }
            return 1;
        }
    }

	return -2;
}

int dSendToNtam(mml_msg *ml, dbm_msg_t *smsg)
{
    pst_MsgQ 	pstSndMsg;
	pst_MsgQSub pstMsgQSub;
    int     	dRet;
	U8		   *pNODE;

	if( (dRet = dGetNode(&pNODE, &pstSndMsg)) < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN dGetNode(CHSMD)", LT);
		return -1;
	}

    smsg->head.src_proc = SEQ_PROC_CHSMD;
    smsg->head.dst_func = ml->src_func;
    smsg->head.dst_proc = ml->src_proc;
    smsg->head.cmd_id = ml->cmd_id;
    smsg->head.msg_id = ml->msg_id;

	log_print(LOGN_INFO, "MML SEND CID[%d] MID[%d]", smsg->head.cmd_id, smsg->head.msg_id);
	pstMsgQSub = (pst_MsgQSub)(unsigned long)&pstSndMsg->llMType;

    pstMsgQSub->usType = DEF_SYS;
    pstMsgQSub->usSvcID = SID_MML;
    pstMsgQSub->usMsgID = MID_MML_RST;

	pstSndMsg->ucProID = SEQ_PROC_CHSMD;
	util_makenid(SEQ_PROC_CHSMD,&pstSndMsg->llNID);
    pstSndMsg->llIndex = 1;
    pstSndMsg->usRetCode = 0;

	pstSndMsg->ucNTAFID = gdSysNo;

    /* NOTE : +1 in CI_SVC */
    pstSndMsg->usBodyLen = DEF_DBMMSG_SIZE - MSG_DATA_LEN + smsg->head.msg_len;

    memcpy( &pstSndMsg->szBody[0], smsg, pstSndMsg->usBodyLen );

	dRet = dMsgsnd(SEQ_PROC_CI_SVC, pNODE);
    if(dRet < 0) {
		log_print(LOGN_CRI,LH"FAILED IN dMsgsnd, to CI_SVC=%d",LT, dRet);
        return -2;
    }

	log_print(LOGN_DEBUG," * SUCCESS SEND TO PROCESSID=%d, PROID=%d, SID=%d, MID=%d, BLEN[%d]\n%s",
		SEQ_PROC_CI_SVC, pstSndMsg->ucProID, pstMsgQSub->usSvcID, pstMsgQSub->usMsgID,
		pstSndMsg->usBodyLen, smsg->data);

    return 0;
}

int dGetVerBlock(char *fn, char (*p)[30])
{
	int     ln, rdcnt, scan_cnt, ldcnt;
	char    buf[256], Bname[128], sCommand[256], sOnOff[256];
	FILE    *fp;

	if( (fp = fopen(fn, "r")) == NULL)
		return -1;  /* fopen error */

	ln		= 0;
	rdcnt	= 0;
	ldcnt   = 0;
	while(fgets(buf, 256, fp) != NULL)
	{
		ln++;
		/*
		* from Source to Target : sscanf
		*/
		if(buf[0] != '#')
		{
			printf("SYNTAX ERROR FILE:%s, LINE:%d - FIRST CHARACTER IS MUST '#'!\n", fn, ln);
			return -1;
		}
		else if(buf[1] == '#')
		{
			if( ((scan_cnt= sscanf(&buf[2], "%s %s %s", Bname, sCommand, sOnOff))==3) && (strcasecmp(sOnOff, "ON") == 0))
			{
				sprintf(*(p+rdcnt), "%s", Bname);
				rdcnt++;
			}
		}
		else if(buf[1] == 'E')
			break;
		else if(buf[1] == '@')
		{
			if( (scan_cnt= sscanf(&buf[2], "%s %*s", Bname)) != 1)
				sprintf(Bname, " - ");

			sprintf(*(p+rdcnt), "%s", Bname);
			rdcnt++;
			ldcnt++;
		}
		else
		{
			printf("SYNTAX ERROR FILE:%s, LINK:%d\n",fn, ln);
			return -2;
		}
	}
	fclose(fp);

	//return rdcnt;
	return ldcnt;
}

