#ifndef __FLTMNG_FUNC_H__
#define __FLTMNG_FUNC_H__

#include <mysql/mysql.h>		/* MYSQL			*/
#include "msgdef.h"			/* st_MsgQ */
#include "mmcdef.h"			/* st_ConnInfo, mml_msg */
#include "filter.h"			/* st_AlmLevel_List, st_Flt_Common, TIMER_INFO, st_Flt_Info, st_SCTP_Shm, st_NAS_Shm, st_Conf */

#include "common_stg.h"		/* st_TraceList, SVC_CONF */

#define MAX_SQLQUERY_SIZE	20480
#define FLT_SHORT_CHECK		15
#define FLT_LONG_CHECK      300

extern int dIsRcvedMessage( pst_MsgQ *ppstMsg );
extern void Apply_Filter_SvcInfo(st_ConnInfo *pstConnInfo, unsigned short uhMsgID);
extern void Apply_Filter_Alm(st_AlmLevel_List *pstAlmLevelList);
extern void Apply_Filter_Common(st_Flt_Common *pstFltCommon);
extern void Apply_Filter_NAS(st_ConnInfo *pstConnInfo, unsigned short uhMsgID);
extern void Apply_Filter_SCTP(st_ConnInfo *pstConnInfo, unsigned short uhMsgID);
extern void Apply_Filter_TRC(st_TraceList *pstTraceList);
extern int Apply_Filter_TIMER(TIMER_INFO *pstData);
extern int Apply_Filter_SVC_ONOFF(SVC_CONF *pstData);
extern int dSendPREA(unsigned short uhMsgID);
extern int dSetFltInfoToFile(st_Flt_Info *pstFlt);
extern int dMakeSendMsg(st_MsgQ *pstMsg);
extern int dProc_Msg(st_MsgQ *pstMsg);
extern int dWriteTimerFile(TIMER_INFO *pstData);
extern int dGetNode(U8 **ppNODE, pst_MsgQ *ppstMsgQ);
extern int dMsgsnd(int procID, OFFSET offset);
extern int dReadTimerFile(TIMER_INFO *pstData);
extern int dGetGNTimer(char *szDelim);
extern int dWriteSCTPFile(st_SCTP_Shm *pstData);
extern int dSelectSCTP(MYSQL *pstMySQL, st_SCTP_Shm *pstData);
extern int dWriteMNIPFile(st_NAS_Shm *pstData);
extern int dSelectMNIP(MYSQL *pstMySQL, st_NAS_Shm *pstData);
extern int dConnectMySQL(MYSQL *pstMySQL, st_ConnInfo *pstConnInfo);
extern int dWriteSVRIPFile(st_SvcInfo_Shm *pstData);
extern int dSelectSVRIP(MYSQL *pstMySQL, st_SvcInfo_Shm *pstData);
extern void Apply_Filter_TCP(pst_Conf pstConf);
extern int dInitLogLvl();
extern int dLogWrite( int dCount, int dLogLvl);
extern void Apply_Filter_LOG(unsigned char *szBuf);
extern int dis_ntaf_timer(mml_msg *ml);
extern int dis_ntaf_sess(mml_msg *ml);
extern int Send_mmc(int dMID, UINT uIndex);
extern int dInitSysConfig();
extern void do_while();



#endif /* __FLTMNG_FUNC_H__ */
