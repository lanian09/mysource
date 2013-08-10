#ifndef __CALL_FUNC_H__
#define __CALL_FUNC_H__

/*
 * CALL STATUS 정의 
 */
#define DEF_CALLSTATE_INIT	0
#define DEF_CALLSTATE_FIN	1
#define DEF_CALLSTATE_DORM	2
#define DEF_CALLSTATE_RECV	3	/* DOrMANT 상태에서 START_PI_DATA_RECALL_NUM을 받았을 때 */
#define DEF_CALLSTATE_DFIN	4	/* DORMANT 상태에서 PI_WAIT_TIMEOUT */

#define DEF_CALL_NORMAL		0
#define DEF_CALL_RECALL		1
#define DEF_CALL_RECALL_1	2

typedef struct _st_STOP_CALL_KEY {
	UINT		uiClientIP;
	UINT		uiReserved;
} st_CALL_KEY, *pst_CALL_KEY;
#define STOP_CALL_KEY_SIZE		sizeof(st_CALL_KEY)

typedef struct _st_call_timer_arg {
	IP4			ClientIP;
	STIME		CallTime;
} CALL_TIMER_ARG;

#define DEF_CALLTIMER_SIZE		sizeof(CALL_TIMER_ARG)

extern OFFSET Create_Call_Session(stMEMSINFO *pMEMSINFO, stHASHOINFO *pHASHOINFO, stTIMERNINFO *pTIMERNINFO,LOG_COMMON *pLOG_COMMON, int type, int len, char *data);
extern int Init_Call_Session(CALL_SESSION_HASH_DATA *pCALL_SESSION_HASH_DATA, stMEMSINFO *pMEMSINFO, stHASHOINFO *pHASHOINFO, stTIMERNINFO *pTIMERNINFO, LOG_COMMON *pLOG_COMMON, int type, int len, char *data);
extern void invoke_del_CALL(void *p);
extern void Call_Session_Process( int type, int len, char *data);
extern U16 Make_Page_UserError(LOG_PAGE_TRANS *pLOG_PAGE_TRANS);
extern int Page_Process( LOG_HTTP_TRANS *pLOG_HTTP_TRANS, BODY *pBODY);
extern char *getSigString(int type);
extern void ETC_Process(Capture_Header_Msg *pCAPHEAD, INFO_ETH *pINFOETH);
extern void INET_Process(LOG_INET *pINETSESS);


#endif	/* __CALL_FUNC_H__ */
