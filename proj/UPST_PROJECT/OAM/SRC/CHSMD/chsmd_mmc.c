/**A.1 * File Include ********************************************************/

/* SYS HEADER */
#include <stdio.h>
#include <sys/types.h>	/* MSGSND(2) */
#include <sys/ipc.h>	/* MSGSND(2) */
#include <sys/msg.h>	/* MSGSND(2) */
#include <signal.h>		/* PTHREAD_KILL(P) */
/* LIB HEADER */
#include "filedb.h"
#include "loglib.h"
#include "verlib.h"
#include "utillib.h"
/* PRO HEADER */
#include "msgdef.h"
#include "mmcdef.h"
#include "sockio.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */
#include "chsmd_hw.h"
#include "chsmd_sw.h"
#include "chsmd_mask.h"		/* dWriteMaskInfo() */
#include "chsmd_mmc.h"

/**B.1*  Definition of New Constants ******************************************/
/**B.2*  Definition of New Type  **********************************************/
/**C.1*  Declaration of Variables  ********************************************/
/**C.2*  Declaration of Variables  ********************************************/
/**D.1*  Definition of Functions  *********************************************/
/**D.2*  Definition of Functions  *********************************************/

/*******************************************************************************
 MMC COMMAND HANDLE
*******************************************************************************/
int MMC_Handle_Proc(mml_msg *mmsg, long mtype)
{
	int			dRet;
	dbm_msg_t	smsg;

	memset(&smsg, 0x00, DEF_DBMMSG_SIZE);
	smsg.common.TotPage	= 1;
	smsg.common.CurPage	= 1;

	log_print(LOGN_CRI, "[MMCD]->[CHSMD] [msg_id] = %d [cmd_id] = %d [LEN] = %d [Dummy] = %d",
		mmsg->msg_id, mmsg->cmd_id, mmsg->msg_len, mmsg->dummy);

	switch(mmsg->msg_id) {

		case MI_ACT_PRC:
			if( (dRet = dAct_prc(mmsg)) < 0){
				log_print(LOGN_CRI, LH"ERROR IN dAct_prc(), dRet=%d",LT,dRet);
			}
			break;

		case MI_DACT_PRC:
			if( (dRet = dDact_prc(mmsg)) < 0){
				log_print(LOGN_CRI, LH"ERROR IN dDact_prc(), dRet=%d",LT,dRet);
			}
			break;

		case MI_DIS_PRC:
			if( (dRet = dDis_prc(mmsg)) < 0){
				log_print(LOGN_CRI, LH"ERROR IN dDis_prc(), dRet=%d",LT,dRet);
			}
			break;

		case MI_DIS_SYS_INFO:
			if( (dRet = dDis_SysInfo(mmsg)) < 0){
				log_print(LOGN_CRI, LH"ERROR IN dDis_SysInfo(), dRet=%d",LT,dRet);
			}
			break;

		case MI_DIS_SUB_SYS_INFO:
			if( (dRet = taf_sys_info(mmsg)) < 0){
				log_print(LOGN_CRI, LH"ERROR IN taf_sys_info(), dRet=%d",LT,dRet);
			}
			break;

		case MI_DIS_SUB_PRC:
		case MI_ACT_SUB_PRC:
		case MI_DACT_SUB_PRC:
			if( (dRet = ntaf_proc_ctl(mmsg)) < 0){
				log_print(LOGN_CRI, LH"ERROR IN ntaf_proc_ctl(), dRet=%d",LT,dRet);
			}
			break;

		/*	DIRECT-PORT		*/
		case MI_MASK_DIRECTOR_PORT:
			if( (dRet = dMaskDirectPort(mmsg)) < 0){
				log_print(LOGN_CRI, LH"ERROR IN dMaskDirectPort(), dRet=%d",LT,dRet);
			}
			break;

		case MI_UMASK_DIRECTOR_PORT:
			if( (dRet = dUmaskDirectPort(mmsg)) < 0){
				log_print(LOGN_CRI, LH"ERROR IN dUmaskDirectPort(), dRet=%d",LT,dRet);
			}
			break;

		/*	SWITCH-PORT		*/
		case MI_MASK_SWITCH_PORT:
			if( (dRet = dMaskSwitchPort(mmsg)) < 0){
				log_print(LOGN_CRI, LH"ERROR IN dMaskSwitchPort(), dRet=%d",LT,dRet);
			}
			break;

		case MI_UMASK_SWITCH_PORT:
			if( (dRet = dUmaskSwitchPort(mmsg)) < 0){
				log_print(LOGN_CRI, LH"ERROR IN dUmaskSwitchPort(), dRet=%d",LT,dRet);
			}
			break;

		default:
			return 0;
	}
	log_print(LOGN_DEBUG, "SENDMESS END");

	switch(mmsg->msg_id)
	{
		/*	DIRECT-PORT		*/
		case MI_MASK_DIRECTOR_PORT:
		case MI_UMASK_DIRECTOR_PORT:
		case MI_MASK_SWITCH_PORT:
		case MI_UMASK_SWITCH_PORT:
			if( (dRet = dWriteMaskInfo(0)) < 0){
				log_print(LOGN_CRI, LH"ERROR IN dWriteMaskInfo(), dRet=%d",LT,dRet);
			}
			break;
		default:
			break;
	}

	return 1;
}

