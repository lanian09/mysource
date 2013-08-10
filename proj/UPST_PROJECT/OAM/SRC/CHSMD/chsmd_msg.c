/**A.1 * File Include *************************************/

/* SYS HEADER */
#include <stdio.h>
#include <sys/msg.h>
#include <errno.h>		/* errno */
/* LIB HEADER */
#include "clisto.h"		/* U8 */
#include "utillib.h"
#include "loglib.h"
#include "ipclib.h"
#include "filedb.h"
/* PRO HEADER */
#include "msgdef.h"
#include "sockio.h"
#include "procid.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
#include "almstat.h"
/* LOC HEADER */
#include "chsmd_func.h"	/* dMsgsnd(), dGetNode() */
#include "chsmd_msg.h"

/**B.1*  Definition of New Constants **********************/
/**B.2*  Definition of New Type  **************************/
/**C.1*  Declaration of Variables  ************************/
static  char    mtime_str[81];

extern pst_NTAM fidb;

extern int	gdIndex;
extern int  gdSysNo;

extern st_subsys_mng stSubSysMng;

extern pst_SWITCH_MNG swch;



/**D.1*  Definition of Functions  *************************/

/**D.2*  Definition of Functions  *************************/


char *cvt_time(time_t t)
{
    if (strftime(mtime_str, 80, "%Y-%m:%d", localtime((time_t *)&t)) <= 0)
        return "UNKNOWN";
    else
        return mtime_str;
}

char *cvt_time2(time_t t)
{
    if (strftime(mtime_str, 80, "%m/%d %H:%M", localtime((time_t *)&t)) <= 0)
        return "UNKNOWN";
    else
        return mtime_str;
}


char *mtime()
{
    time_t t;
    t = time(&t);
    strftime(mtime_str, 80, "%Y-%m-%d %T %a", localtime((time_t *)&t));
    mtime_str[21] = toupper(mtime_str[21]);
    mtime_str[22] = toupper(mtime_str[22]);
    return mtime_str;
}

#if 0
void CalcLoad(char invtype, short invno, long long *llLoadVal )
{
    long long   llCurLoadVal;

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
#endif

void Send_CondMess(int sysno, int loctype, char invtype, short invno, char almstatus, char oldalm)
{
	st_almsts	almsts;

	int			dRet;

	pst_MsgQ	pstSndMsg;
	pst_MsgQSub	pstSndSub;
	U8		   *pNODE;

	if( (dRet = dGetNode(&pNODE, &pstSndMsg)) < 0 ){
		log_print(LOGN_CRI,LH"FAILED IN dGetNode(CHSMD)",LT);
        return;
	}


	memset(&almsts,      0x00, DEF_ALMSTS_SIZE);

	almsts.ucSysType     = SYSTYPE_TAM;
    almsts.ucSysNo       = (unsigned char)gdSysNo;
    almsts.ucLocType     = loctype;
    almsts.ucInvType     = invtype;
    almsts.ucInvNo       = invno;
    almsts.ucAlmLevel    = almstatus;
	almsts.ucOldAlmLevel = oldalm;

	if( LOCTYPE_CHNL == loctype && INVTYPE_LINK == invtype ){
		almsts.uiIPAddr = stSubSysMng.sys[invno].uiIP;
		log_print(LOGN_DEBUG,"[CHECK CHANNEL] IDX=%d, IP=%s(%u)",
			almsts.ucInvNo, util_cvtipaddr(NULL,almsts.uiIPAddr), almsts.uiIPAddr);
	}
#if 0
	switch( loctype ){
		case LOCTYPE_LOAD:
			CalcLoad( invtype, invno, &almsts.llLoadVal );
			break;
		case LOCTYPE_CHNL:
			if( INVTYPE_LINK == invtype ){
				almsts.uiIPAddr = stSubSysMng.sys[invno].uiIP;
				log_print(LOGN_DEBUG,"[CHECK CHANNEL] IDX=%d, IP=%s(%u)",
					almsts.ucInvNo, util_cvtipaddr(NULL,almsts.uiIPAddr), almsts.uiIPAddr);
			}
			break;
		default:
			almsts.llLoadVal = 0;
	}
#endif

    time(&almsts.tWhen);

	/* set_time
	set_time(almsts.ucLocType, almsts.ucInvType, almsts.ucInvNo, almsts,tWhen);
	*/

	pstSndSub = (pst_MsgQSub)&pstSndMsg->llMType;
    pstSndSub->usType = DEF_SYS;
    pstSndSub->usSvcID = SID_STATUS;
    pstSndSub->usMsgID = MID_CONSOL;

    util_makenid( SEQ_PROC_CHSMD, &pstSndMsg->llNID );
	pstSndMsg->ucNTAFID = 0;
    pstSndMsg->ucProID = SEQ_PROC_CHSMD;
    pstSndMsg->llIndex = gdIndex;
    gdIndex++;

    pstSndMsg->dMsgQID   = 0;
    pstSndMsg->usBodyLen = DEF_ALMSTS_SIZE;
    pstSndMsg->usRetCode = 0;

    memcpy( pstSndMsg->szBody, &almsts, pstSndMsg->usBodyLen );

	/****************************************************************************************/
	/* COND로 메세지 전송                                                                   */
	/****************************************************************************************/
	if( (dRet = dMsgsnd(SEQ_PROC_COND, pNODE)) < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN dMsgsnd(COND)"EH, LT, ET);
		return;
	}
	log_print(LOGN_DEBUG, LH"SUCCESS IN dMsgsnd(COND) MESSAGE PROCESSID=%d", LT, SEQ_PROC_COND);

	pstSndSub->usMsgID = MID_ALARM;
	pstSndMsg->dMsgQID   = 0;

	/****************************************************************************************/
	/* ALMD로 메세지 전송                                                                   */
	/****************************************************************************************/
	if( (dRet = dMsgsnd(SEQ_PROC_ALMD, pNODE)) < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN dMsgsnd(ALMD)"EH, LT, ET);
		return;
	}
	log_print(LOGN_DEBUG, LH"SUCCESS IN dMsgsnd(ALMD) MESSAGE PROCESSID=%d", LT, SEQ_PROC_ALMD);

	/* UPDATE fidb.mem */
	write_FIDB();
}

void Send_CondDirSWMess(unsigned char cSysNo, char cInvType, unsigned char cInvNo, char cCurAlmStat, char cOldAlmStat)
{
	st_almsts	almsts;
	int			dRet;

	pst_MsgQ	pstSndMsg;
	pst_MsgQSub pstSndSub;
	U8		   *pNODE;


	if( (dRet = dGetNode(&pNODE, &pstSndMsg)) < 0 ){
		log_print(LOGN_CRI,LH"FAILED IN dGetNode(CHSMD)",LT);
        return;
	}

	memset(&almsts, 0x00, DEF_ALMSTS_SIZE);

	switch(cInvType)
	{
		case INVTYPE_PORT_SWITCH:
			almsts.ucLocType	= LOCTYPE_PHSC;
			almsts.ucSysType	= SYSTYPE_SWITCH;
			break;
		case INVTYPE_CPU_SWITCH:
			almsts.ucLocType	= LOCTYPE_LOAD;
			almsts.ucSysType	= SYSTYPE_SWITCH;
			almsts.llLoadVal	= swch->stSwitch[cSysNo].uSwitchCPU[cInvNo];
			break;
		case INVTYPE_MEMORY_SWITCH:
			almsts.ucLocType	= LOCTYPE_LOAD;
			almsts.ucSysType	= SYSTYPE_SWITCH;
			almsts.llLoadVal	= ((float)swch->stSwitch[cSysNo].uSwitchMEM[0]/(float)(swch->stSwitch[cSysNo].uSwitchMEM[0]+swch->stSwitch[cSysNo].uSwitchMEM[1]))*100;
			break;
		default:
			almsts.ucLocType	= LOCTYPE_PHSC;
			almsts.ucSysType	= SYSTYPE_DIRECT;
			break;
	}

	almsts.ucInvType		= cInvType;
	almsts.ucSysNo			= cSysNo+1;
	almsts.ucInvNo			= cInvNo;
	almsts.ucAlmLevel		= cCurAlmStat;
	almsts.ucOldAlmLevel	= cOldAlmStat;

	log_print(LOGN_DEBUG, "F=%s:%s:%d] ucSysNo[%hu] ucInvType[%hu] ucInvNo[%hu] ucAlmLevel[0x%02X] ucOldAlmLevel[0x%02X] llLoadVal[%lld]", __FILE__, __FUNCTION__, __LINE__,
		almsts.ucSysNo, almsts.ucInvType, almsts.ucInvNo, almsts.ucAlmLevel, almsts.ucOldAlmLevel, almsts.llLoadVal);

	time(&almsts.tWhen);

	pstSndSub			= (pst_MsgQSub)&pstSndMsg->llMType;
	pstSndSub->usType	= DEF_SYS;
	pstSndSub->usSvcID	= SID_STATUS;
	pstSndSub->usMsgID	= MID_CONSOL;

	util_makenid(SEQ_PROC_CHSMD, &pstSndMsg->llNID);
	pstSndMsg->ucNTAFID	= 0;
	pstSndMsg->ucProID	= SEQ_PROC_CHSMD;
	pstSndMsg->llIndex	= gdIndex;
	gdIndex++;

	pstSndMsg->dMsgQID	= 0;
	pstSndMsg->usBodyLen = DEF_ALMSTS_SIZE;
	pstSndMsg->usRetCode = 0;

	memcpy(pstSndMsg->szBody, &almsts, pstSndMsg->usBodyLen);


	/****************************************************************************************/
	/* COND로 메세지 전송                                                                   */
	/****************************************************************************************/
	if( (dRet = dMsgsnd(SEQ_PROC_COND, pNODE)) < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN dMsgsnd(COND)"EH, LT, ET);
		return;
	}
	log_print(LOGN_DEBUG, LH"SUCCESS IN dMsgsnd(COND) MESSAGE PROCESSID=%d", LT, SEQ_PROC_COND);

	if(almsts.ucSysType == SYSTYPE_DIRECT)
		pstSndSub->usSvcID	= SID_STATUS_DIRECT;
	else
		pstSndSub->usSvcID	= SID_STATUS_SWITCH;

	pstSndSub->usMsgID	= MID_ALARM;
	pstSndMsg->dMsgQID	= 0;

	/****************************************************************************************/
	/* ALMD로 메세지 전송                                                                   */
	/****************************************************************************************/
	if( (dRet = dMsgsnd(SEQ_PROC_ALMD, pNODE)) < 0 ){
		log_print(LOGN_CRI, LH"FAILED IN dMsgsnd(ALMD)"EH, LT, ET);
		return;
	}
	log_print(LOGN_DEBUG, LH"SUCCESS IN dMsgsnd(ALMD) MESSAGE PROCESSID=%d", LT, SEQ_PROC_ALMD);

	/* UPDATE fidb.mem */
	write_FIDB();
}


void SetFIDBValue( unsigned char *ucFIDB, unsigned char ucNEW )
{
	unsigned char ucTmp;

	ucTmp = *ucFIDB;

	// 이미 mask 처리가 되어 있는 경우에는 값만 udpate 를 하고, mask 처리를 유지시킴.
    if( ucTmp & MASK ) {
        ucTmp = ucNEW;
        ucTmp |= MASK;

    } else {
        ucTmp = ucNEW;
    }

	*ucFIDB = ucTmp;
}
