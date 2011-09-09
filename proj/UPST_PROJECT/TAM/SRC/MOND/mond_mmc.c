/***** A.1 * File Include *******************************/
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/msg.h>
#include <errno.h>

#include "typedef.h"
#include "utillib.h"
#include "loglib.h"

#include "msgdef.h"
#include "procid.h"
#include "almstat.h"
#include "filedb.h"
#include "mmcdef.h"

#include "mond_func.h"
#include "mond_mmc.h"


/**B.1*  Definition of New Constants ******************************************/
/**B.2*  Definition of New Type  **********************************************/
/**C.1*  Declaration of Variables  ********************************************/
extern int gdIndex;

/**D.1*  Definition of Functions  *********************************************/
/**D.2*  Definition of Functions  *********************************************/

int SendMess(mml_msg *mmsg, dbm_msg_t *smsg)
{
	unsigned char   *pNODE;
    int             dMsgLen, dRet;
    pst_MsgQ        pstSndMsg;
    pst_MsgQSub     pstSndSub;

	if( (dRet = dGetNode(&pNODE, &pstSndMsg)) < 0 ){
		log_print(LOGN_CRI,LH"FAILED IN dGetNode(QMON), errno=%d:%s",
				LT,errno,strerror(errno));
		return -1;
	}

    smsg->head.src_proc     = SEQ_PROC_CHSMD;
    smsg->head.dst_func     = mmsg->src_proc;
    smsg->head.dst_proc     = mmsg->src_func;
    smsg->head.cmd_id        = mmsg->cmd_id;
    smsg->head.msg_id        = mmsg->msg_id;

    pstSndSub = (pst_MsgQSub)&pstSndMsg->llMType;
    pstSndSub->usType = DEF_SYS;
    pstSndSub->usSvcID = SID_MML;
    pstSndSub->usMsgID = MID_MML_RST;

    util_makenid( SEQ_PROC_CHSMD, &pstSndMsg->llNID );
	pstSndMsg->ucNTAFID = 0;
    pstSndMsg->ucProID = SEQ_PROC_CHSMD;
    pstSndMsg->llIndex = gdIndex;
    gdIndex++;

    pstSndMsg->dMsgQID = 0;
    pstSndMsg->usBodyLen = sizeof(dbm_msg_t)-MSG_DATA_LEN+smsg->head.msg_len;
    pstSndMsg->usRetCode = 0;

    memcpy( pstSndMsg->szBody, smsg, pstSndMsg->usBodyLen );

	dMsgLen = DEF_MSGHEAD_LEN + pstSndMsg->usBodyLen;

	if( (dRet = dMsgsnd(SEQ_PROC_MMCD, pNODE)) < 0 ){
		log_print(LOGN_CRI, "SEND FAIL  FOR MMCD IS NOT DELIVERED : %s",
				strerror(errno) );
		return -1;
	}
	else{
		log_print(LOGN_DEBUG, "SEND OK [%d] : [MSG LEN][%d]  [MMCD PROCID][%d] !!!",
				smsg->head.cmd_id, smsg->head.msg_len, SEQ_PROC_MMCD);
	}

    return 1;
}

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

