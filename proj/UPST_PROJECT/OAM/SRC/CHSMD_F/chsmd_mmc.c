/**A.1*  File Inclusion *******************************************************/

/* SYS HEADER */
#include <stdio.h>
#include <string.h>		/* memset */
/* LIB HEADER */
#include "loglib.h"
/* PRO HEADER */
#include "mmcdef.h"		/* mml_msg */
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */
#include "chsmd_mmc.h"

/**B.1*  Definition of New Constants ******************************************/
/**B.2*  Definition of New Type  **********************************************/
/**C.1*  Declaration of Variables  ********************************************/
/**D.1*  Definition of Functions  *********************************************/
/**D.2*  Definition of Functions  *********************************************/

/*******************************************************************************
 * MMC 명령어 처리
*******************************************************************************/
int MMC_Handle_Proc(mml_msg *mmsg, long mtype, long long  llIndex)
{
    dbm_msg_t   smsg;
	int			dRet;

    /*** 초기화 ***************************************************************/
    memset((void *)&smsg, 0, DEF_DBMMSG_SIZE);
	smsg.common.TotPage  = 1;
    smsg.common.StatFlag = 1;

    /***************************************************************************
	 * 메시지 id별 처리
	***************************************************************************/
    switch (mmsg->msg_id)
	{
		case MI_ACT_SUB_PRC:
            dRet = dAct_prc(mmsg);
            if( dRet < 0 ){
                log_print( LOGN_CRI, "MMC : Failed in dAct_prc dRet:%d", dRet );
                return -1;
            }
            break;

        case MI_DACT_SUB_PRC:
            dRet = dDact_prc(mmsg);
            if( dRet < 0 ){
                log_print( LOGN_CRI, "MMC : Failed in dDact_prc dRet:%d", dRet );
                return -2;
            }
            break;

        case MI_DIS_SUB_PRC:
            dRet = dDis_prc(mmsg);
            if( dRet < 0 ){
                log_print( LOGN_CRI, "MMC : Failed in dDis_prc dRet:%d", dRet );
                return -3;
            }
            break;

        case MI_DIS_SUB_SYS_INFO:
            if( (dRet = dSysInfo(mmsg)) < 0)
			{
                log_print( LOGN_CRI, "F=%s:%s.%d: ERROR IN dSysInfo() dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
                return -4;
            }
            break;

		default:
            return 0;
    }

    return 1;
}

