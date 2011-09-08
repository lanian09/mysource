#ifndef __O_SVCMON_MSGQ_H__
#define __O_SVCMON_MSGQ_H__

extern int dIsReceivedMessage(st_MsgQ *pstMsgQ);
extern int dSendMsg(int qid, st_MsgQ *pstMsgQ);
extern void Send_CondMess(int sysno, int loctype, char invtype, short invno, char almstatus, char oldalm, U8 tmpl4type, UINT ip, long long loadval);

#endif /* __O_SVCMON_MSGQ_H__ */
