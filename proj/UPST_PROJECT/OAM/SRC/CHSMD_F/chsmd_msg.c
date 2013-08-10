/**A.1*  File Inclusion *******************************************************/

/* SYS HEADER */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>
/* LIB HEADER */
#include "clisto.h"		/* U8 */
#include "filedb.h"		/* st_NTAF */
#include "commdef.h"
#include "loglib.h"
#include "utillib.h"	/* make_utilnid */
/* PRO HEADER */
#include "msgdef.h"		/* st_MsgQ */
#include "sockio.h"
#include "mmcdef.h"
#include "procid.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
#include "almstat.h"
/* LOC HEADER */
#include "chsmd_init.h"	/* write_FIDB() */
#include "chsmd_func.h"	/* dGetNode(), dMsgsnd() */
#include "chsmd_msg.h"

/**B.1*  Definition of New Constants ******************************************/
/**B.2*  Definition of New Type  **********************************************/
/**C.1*  Declaration of Variables  ********************************************/
char mtime_str[256];

/**C.2*  Declaration of Variables  ********************************************/
extern pst_NTAF 	fidb;

extern int			dNIDIndex;
extern int          gdSysNo;	/* SYSTEM NO */

/**D.1*  Definition of Functions  *************************/
/**D.2*  Definition of Functions  *************************/

/*******************************************************************************
 * CONVERT TIME TO STRING
*******************************************************************************/
char *mtime()
{
    time_t t;
    t = time(&t);
    strftime(mtime_str, 80, "%Y-%m-%d %T %a", localtime((time_t *)&t));
    mtime_str[21] = toupper(mtime_str[21]);
    mtime_str[22] = toupper(mtime_str[22]);
    return mtime_str;
}

void CalcLoad(char invtype, short invno, long long *llLoadVal )
{
	long long	llCurLoadVal;

	llCurLoadVal = 0ll;

	switch(invtype)
	{
		case INVTYPE_CPU:
			llCurLoadVal = ((float)fidb->cpusts.llCur)/10.0;
			break;
		case INVTYPE_MEMORY:
			llCurLoadVal = ((float)fidb->memsts.llCur/(float)fidb->memsts.lMax)*100.0;
			break;
		case INVTYPE_DISK:
			llCurLoadVal = ((float)fidb->disksts[invno].llCur/(float)fidb->disksts[invno].lMax)*100.0;
			break;
		case INVTYPE_QUEUE:
			llCurLoadVal = ((float)fidb->quests.llCur/(float)fidb->quests.lMax)*100.0;
			break;
		case INVTYPE_NIFO:
			llCurLoadVal = ((float)fidb->nifosts.llCur/(float)fidb->nifosts.lMax)*100.0;
			break;
		default:
			log_print(LOGN_DEBUG,"UNVALID INVTYPE[%d]",invno);
			break;
	}

	*llLoadVal = llCurLoadVal;
}

/*******************************************************************************
 * SEND ALARM MESSAGE TO COND
*******************************************************************************/
void Send_AlmMsg( char loctype, char invtype, short invno, char almstatus, char oldalm)
{
	st_almsts almsts;

	int				dRet;
	pst_MsgQ        pstSndMsg;
	pst_MsgQSub     pstSndSub;
	U8			   *pNODE;

	if( (dRet = dGetNode(&pNODE, &pstSndMsg)) < 0 ){
        log_print(LOGN_CRI,LH"FAILED IN dGetNode(CHSMD)",LT);
        return;
    }

	memset( &almsts, 0x00, DEF_ALMSTS_SIZE );

	almsts.ucSysType = SYSTYPE_TAF;
	almsts.ucSysNo = (unsigned char)gdSysNo;	/* NTAF */
	almsts.ucLocType = loctype;
	almsts.ucInvType = invtype;
	almsts.ucInvNo = invno;
	almsts.ucAlmLevel = almstatus;
	almsts.ucOldAlmLevel = oldalm;

	if( loctype == LOCTYPE_LOAD ){
		CalcLoad( invtype, invno, &almsts.llLoadVal );
	}
	else
		almsts.llLoadVal = 0;

	time(&almsts.tWhen);

	set_time(almsts.ucLocType, almsts.ucInvType, almsts.ucInvNo, almsts.tWhen);

#ifdef DEBUG
	log_print(LOGN_INFO,"LOCTYPE[%x], INVTYPE[%x], INVNO[%d] ALMSTATUS[%x]",
	loctype, invtype, invno, almstatus );
#endif

	/* Head 결정 */

	pstSndSub = (pst_MsgQSub)(unsigned long*)&pstSndMsg->llMType;
    pstSndSub->usType = DEF_SYS;
    pstSndSub->usSvcID = SID_STATUS;
    pstSndSub->usMsgID = MID_ALARM;

	util_makenid( SEQ_PROC_CHSMD, &pstSndMsg->llNID );
	pstSndMsg->ucNTAMID = 0;
	pstSndMsg->ucNTAFID = almsts.ucSysNo;
    pstSndMsg->ucProID = SEQ_PROC_CHSMD;
    pstSndMsg->llIndex = dNIDIndex;
    dNIDIndex++;

	pstSndMsg->dMsgQID = 0;

	/*
	* NTAFT_HEADER_LEN < Socket Header Size 만큼 +
	*/
    pstSndMsg->usBodyLen = DEF_ALMSTS_SIZE;
    pstSndMsg->usRetCode = 0;

	//memset( pstSndMsg->szBody, 0x00, MNG_PKT_HEAD_SIZE );
	memcpy( &pstSndMsg->szBody[0], &almsts, pstSndMsg->usBodyLen );

	/***************************************************************************
	 * ALMD로 메시지 전송
	***************************************************************************/
	if( (dRet = dMsgsnd(SEQ_PROC_ALMD, pNODE)) < 0 ){
        log_print(LOGN_CRI, LH"FAILED IN dMsgsnd(ALMD)"EH, LT, ET);
        return;
    }
    log_print(LOGN_DEBUG, LH"SUCCESS IN dMsgsnd(ALMD) MESSAGE PROCESSID=%d, BLEN=%d", LT, SEQ_PROC_ALMD, pstSndMsg->usBodyLen);


	/***************************************************************************
	 * COND로 전송하던 내용을 전송하는 부분 추가
	***************************************************************************/
	pstSndSub->usMsgID = MID_CONSOL;
	pstSndMsg->dMsgQID = 0;

	if( (dRet = dMsgsnd(SEQ_PROC_CI_SVC, pNODE)) < 0 ){
        log_print(LOGN_CRI, LH"FAILED IN dMsgsnd(CI_SVC)"EH, LT, ET);
        return;
    }
    log_print(LOGN_DEBUG, LH"SUCCESS IN dMsgsnd(CI_SVC) MESSAGE PROCESSID=%d, BLEN=%d, SID=%d, MID=%d", 
		LT, SEQ_PROC_CI_SVC, pstSndMsg->usBodyLen, pstSndSub->usSvcID, pstSndSub->usMsgID);

	/* UPDATE fidb.mem */
	write_FIDB();

	return;
}

/*******************************************************************************
 * 상태 변화가 있음을 ALMD로 전송
*******************************************************************************/
int Send_ALMD(void)
{
	int			dRet;

	pst_MsgQ	pstSndMsg;
	pst_MsgQSub	pstSndSub;
	U8		   *pNODE;

	if( (dRet = dGetNode(&pNODE, &pstSndMsg)) < 0 ){
        log_print(LOGN_CRI,LH"FAILED IN dGetNode(CHSMD)",LT);
        return -1;
    }

	pstSndSub = (pst_MsgQSub)(unsigned long*)&pstSndMsg->llMType;
	pstSndSub->usType = DEF_SYS;
    pstSndSub->usSvcID = SID_STATUS;
    pstSndSub->usMsgID = MID_ALARM;

	util_makenid( SEQ_PROC_CHSMD, &pstSndMsg->llNID );
	pstSndMsg->ucNTAMID = 0;
	pstSndMsg->ucNTAFID = (unsigned char)gdSysNo;
	pstSndMsg->ucProID = SEQ_PROC_CHSMD;
    pstSndMsg->llIndex = dNIDIndex;
    dNIDIndex++;

	pstSndMsg->dMsgQID   = 0;
    pstSndMsg->usBodyLen = 0;
    pstSndMsg->usRetCode = 0;
	memset( &pstSndMsg->szBody, 0x00, MAX_MSGBODY_SIZE );

	dRet = dMsgsnd(SEQ_PROC_ALMD, pNODE);
    if(dRet < 0) {
        log_print(LOGN_CRI,"SEND FAIL for ALMD is not delivered : %s", strerror(errno));
        return -3;
    }

	log_print(LOGN_DEBUG,"MESSAGE SEND PROCESSID=%d, BLEN=%d, PROC=%d, SID=%d, MID=%d, NTAF=%d,",
		SEQ_PROC_ALMD, pstSndMsg->usBodyLen, pstSndMsg->ucProID,
		pstSndSub->usSvcID, pstSndSub->usMsgID, gdSysNo);
    return 1;
}

void SetFIDBValue( unsigned char *ucFIDB, unsigned char ucNEW )
{
	unsigned char	usTmp;

	usTmp = *ucFIDB;

    if( usTmp & MASK ) {
        usTmp = ucNEW | MASK;

    } else {
        usTmp = ucNEW;
    }

	*ucFIDB = usTmp;
}

void set_time(unsigned char loctype, unsigned char invtype, unsigned char invno, time_t tWhen )
{
	int		index;

	index	= -1;
	switch(loctype)
	{
		case LOCTYPE_PHSC:
			switch(invtype)
			{
				case INVTYPE_LINK:
					index = IDX_LINK + invno;
					break;
				case INVTYPE_POWER:
					index = IDX_PWR + invno;
					break;
				case INVTYPE_FAN:
					index = IDX_FAN + invno;
					break;
				case INVTYPE_DISKARRAY:
					index = IDX_DISKAR + invno;
					break;
				case INVTYPE_PORT:
					index = IDX_PORT + invno;
			}
			break;
		case LOCTYPE_PROCESS:
			switch(invtype)
			{
				case INVTYPE_USERPROC:
					index = 0;
			}
		case LOCTYPE_LOAD:
			switch(invtype)
			{
				case INVTYPE_CPU:
					index = IDX_CPU;
					break;
				case INVTYPE_MEMORY:
					index = IDX_MEM;
					break;
				case INVTYPE_DISK:
					index = IDX_DISK+invno;
					break;
				case INVTYPE_QUEUE:
					index = IDX_QUE;
					break;
				case INVTYPE_NIFO:
					index = IDX_NIFO;
					break;
			}
			break;
	}

	if(index < 0)
	{
		log_print(LOGN_WARN,"UNVALID REFERENCE : LOCTYPE[%d] INVTYPE[%d] INVNO[%d] TIME[%ld]", loctype, invtype, invno, tWhen );
		return;
	}
	else if(index == 0)
	{
		log_print(LOGN_INFO,"INVOKED PROCESS IDX[%d]",invno);
		return;
	}

	fidb->tEventUpTime[index] = tWhen;
	log_print(LOGN_INFO, "STORED EVENTTIME[%ld] INDEX[%d]", tWhen, index);
}

