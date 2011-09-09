#ifndef __IF_MSGQ_H__
#define __IF_MSGQ_H__

#include "nsocklib.h" 	/* st_ClientInfo, st_FDInfo */
#include "msgdef.h"		/* st_MsgQ */
#include "mmcdef.h"		/* mml_msg */

extern int dSendToProc(st_ClientInfo *stNet, int dIdx, char *szBuf, pst_NTAFTHeader pstHeader);
extern int dSndNtafMsg(st_ClientInfo *stNet, st_MsgQ *pstMsgQ, st_FDInfo *stFD);
extern int dGetSubSfd(st_ClientInfo *stNet, int dSysNo);
extern int get_subsys_info(char *szFileName, char *szData);
extern int dMask_Ntaf_Chn(mml_msg *ml, long long llNID, int dIndex);
extern int dUMask_Ntaf_Chn(mml_msg *ml, long long llNID, st_ClientInfo *stNet);
extern int dMask_Interlock_Chn(mml_msg *ml, long long llNID, int dIndex);
extern int dUMask_Interlock_Chn(mml_msg *ml, long long llNID);
extern int dGet_Sub_Info(mml_msg *ml, long long llNID, int dIndex);
extern int dGetSysNo(st_ClientInfo *stNet, int dIdx);
extern int dGetSysNoWithIP(unsigned int uiIP);
extern int dSendToMMCD(mml_msg *mmsg, dbm_msg_t *smsg, long long llNID);
extern int dSendMsg(int qid, pst_MsgQ pstMsg);
extern int dHandleMsgAll(st_ClientInfo *stNet, st_MsgQ *pstMsgQ, st_FDInfo *stFD, int dIndex);
extern int dHandleMsg(st_ClientInfo *stNet, st_MsgQ *pstMsgQ, st_FDInfo *stFD, int dIndex);
extern int dIsRcvedMessage( pst_MsgQ pstMsg );

#endif /* __IF_MSGQ_H__ */
