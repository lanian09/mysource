#ifndef _TCP_INIT_H_
#define _TCP_INIT_H_

#include "typedef.h"

typedef struct _st_STOP_CALL_KEY {
	UINT		uiClientIP;
	UINT		uiReserved;
} st_CALL_KEY, *pst_CALL_KEY;
#define STOP_CALL_KEY_SIZE		sizeof(st_CALL_KEY)

extern void SetUpSignal(void);
extern void vTCPTimerReConstruct(stHASHOINFO *pHASH, stTIMERNINFO *pTIMER);
extern void UserControlledSignal(S32 isign);
extern void IgnoreSignal(S32 isign);
extern S32 dInitTcp(stMEMSINFO **pMEMSINFO, stHASHOINFO **pHASHOINFO, stTIMERNINFO **pTIMERNINFO);
extern void FinishProgram(void);


#endif /* _TCP_INIT_H_ */
