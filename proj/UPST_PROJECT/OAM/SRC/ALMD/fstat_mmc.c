/*******************************************************************************
			DQMS Project

	Author   : Park Si Woo
	Section  : ALMD
	SCCS ID  : @(#)fstat_mmc.c	1.1
	Date     : 08/11/03
	Revision History :
        '01.  7. 21     Initial
		'03.  1. 15		Initial

	Description :

	Copyright (c) uPRESTO 2005
*******************************************************************************/

/***** A.1 * File Include *******************************/

/* SYS HEADER */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/msg.h>
/* LIB HEADER */
#include "typedef.h"
#include "loglib.h"
#include "utillib.h"
/* PRO HEADER */
#include "msgdef.h"
#include "procid.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
#include "almstat.h"
/* LOC HEADER */
#include "almd_func.h"
#include "almd_sock.h"
#include "fstat_mmc.h"

/**B.1*  Definition of New Constants ******************************************/
#define DEF_MASK 171


/**B.2*  Definition of New Type  **********************************************/

/**C.1*  Declaration of Variables  ********************************************/

extern st_WNTAM      *fidb;

extern st_TOT_MASKNTAF stMASKNTAF;


int SetNTPSTS( int dSysNo, int dType, unsigned char ucCur, unsigned char ucOld  );

/**D.1*  Definition of Functions  *********************************************/

/**D.2*  Definition of Functions  *********************************************/


int mask_ntp_alm(mml_msg *mmsg, dbm_msg_t *smsg)
{
	int		i;

	short int	sSysType, sSysNo, sInvNo, sMask;

	sSysType	= 0;
	sInvNo		= 0;
	sMask		= 0;

	log_print(LOGN_DEBUG, "[F=%s:%s.%d] MASK_NTP_ALM NUM_OF_PARA[%d]", __FILE__, __FUNCTION__, __LINE__, mmsg->num_of_para);
	for(i = 0; i < mmsg->num_of_para; i++)
	{
		log_print(LOGN_DEBUG, "[F=%s:%s.%d] IDX[%d] PARA_ID[%d] PARA_TYPE[%d] PARA_CONT[%s]",
			__FILE__, __FUNCTION__, __LINE__, i, mmsg->msg_body[i].para_id, mmsg->msg_body[i].para_type, mmsg->msg_body[i].para_cont);
		/* 20040430,poopee */
		switch (mmsg->msg_body[i].para_id)
		{
			case 69:	/* SUBSYS */
				sSysType = atoi(mmsg->msg_body[i].para_cont);
				break;
			case 56:	/* SYSNO */
				sSysNo = atoi(mmsg->msg_body[i].para_cont);
				break;
			case 174:	/* NTP_SYS */
				sInvNo = atoi(mmsg->msg_body[i].para_cont);
				break;
			case 178:	/* MASK_STATUS */
				sMask = atoi(mmsg->msg_body[i].para_cont);
				break;
		}
	}
	sSysNo -= 1;
	sInvNo -= 164;

	if(sSysType == 51)
	{
		log_print(LOGN_DEBUG, "[F=%s:%s.%d] INVNO[%d] MASK[%d]", __FILE__, __FUNCTION__, __LINE__, sInvNo, sMask);

		if( (sInvNo == 0) || (sInvNo == 1))
		{
			smsg->common.mml_err	= eBadParameter;
			smsg->common.cont_flag	= DBM_END;
			smsg->head.msg_len		= 0;
			smsg->common.total_cnt	= 1;
			smsg->common.curr_cnt	= 1;
			smsg->common.TotPage	= 1;
			smsg->common.CurPage	= 1;
			smsg->common.StatFlag	= 0;
			SendMess(mmsg, smsg);
		}
	}
	else
	{
		smsg->common.mml_err	= eBadParameter;
		smsg->common.cont_flag	= DBM_END;
		smsg->head.msg_len		= 0;
		smsg->common.total_cnt	= 1;
		smsg->common.curr_cnt	= 1;
		smsg->common.TotPage	= 1;
		smsg->common.CurPage	= 1;
		smsg->common.StatFlag	= 0;
		SendMess(mmsg, smsg);
	}

	return 0;
}


int SetNTPSTS( int dSysNo, int dType, unsigned char ucCur, unsigned char ucOld  )
{
    time_t nowtime;
    pst_MsgQ pstMsg;
    st_MsgQSub *stSubMq;
    st_almsts stAlm;
    int dRet;
	unsigned char *pNODE; 

	if( (dRet = dGetNode(&pNODE, &pstMsg)) < 0 ){
		log_print(LOGN_CRI,"%s.%d:%s FAILED IN dGetNode(QMON), errno=%d:%s",
				__FILE__,__LINE__,__FUNCTION__,errno,strerror(errno));
		return -1;
	}

    stAlm.ucSysType = SYSTYPE_TAF;
    stAlm.ucSysNo = dSysNo+1;
    stAlm.ucLocType = LOCTYPE_NTP;
    stAlm.ucInvType = INVTYPE_TIMESYNC;
    stAlm.ucInvNo = 0;
    stAlm.ucAlmLevel = ucCur;
    stAlm.ucOldAlmLevel = ucOld;
    time(&nowtime);
    stAlm.tWhen = nowtime;
    stAlm.uiIPAddr = 0;


	log_print(LOGN_DEBUG, "SYSNO[%d] SETNTPSTS RET", dSysNo+1 );

    stSubMq = (st_MsgQSub*)&pstMsg->llMType;

    stSubMq->usType = DEF_SYS;
    stSubMq->usSvcID =  SID_STATUS;
    stSubMq->usMsgID = MID_ALARM;

    util_makenid( SEQ_PROC_ALMD, &pstMsg->llNID );
    pstMsg->ucProID = SEQ_PROC_ALMD;
    pstMsg->llIndex = 1;

    pstMsg->dMsgQID = 0;
    pstMsg->usRetCode = 0;
	pstMsg->ucNTAFID = dSysNo +1;

    pstMsg->usBodyLen = sizeof(st_almsts);

    memcpy( &pstMsg->szBody[0], &stAlm, sizeof(st_almsts) );

	if( (dRet = dMsgsnd(SEQ_PROC_FSTAT, pNODE)) < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN dMsgsnd(FSTAT)", LT);
		return -1;
	}

    log_print(LOGN_DEBUG,"[SUCC] FSTATD MSGSND NTAF_TIMESYNC SYSTEMNO[%d] INVTYPE[%d] INVNO[%d] STATUS[%d] ",
        stAlm.ucSysNo, stAlm.ucInvType, stAlm.ucInvNo, stAlm.ucAlmLevel );

    return 0;
}

