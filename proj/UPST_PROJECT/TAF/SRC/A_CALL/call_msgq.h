#ifndef __CALL_MSGQ_H__
#define __CALL_MSGQ_H__

extern S32 dSend_CALL_Data(stMEMSINFO *pMEMSINFO, S32 dSeqProcID, U8 *pNode);
extern S32 Send_Call_Session_LOG(CALL_SESSION_HASH_DATA *pCALL_SESSION_HASH_DATA);
extern S32 Send_Page_Session_LOG(void *p);
extern S32 Send_Dialup_Session_LOG(CALL_SESSION_HASH_DATA *pCALL_SESSION_HASH_DATA);
extern void dSend_Service_Start_Signal(CALL_SESSION_HASH_DATA *pCALLSESSHASHDATA, STIME SvcStartTime, MTIME SvcStartMTime);
extern void dSend_INET_Signal(CALL_SESSION_HASH_DATA *pCALLSESSHASHDATA, STIME SvcStartTime, MTIME SvcStartMTime, int type);
extern void Send_Clear_Msg(UINT dSeqProcID, UINT uiClientIP, INT LastPktTime);

#endif	/* __CALL_MSGQ_H__ */
