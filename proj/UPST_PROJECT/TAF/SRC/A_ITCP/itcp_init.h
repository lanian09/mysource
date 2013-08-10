#ifndef _ITCP_INIT_H_
#define _ITCP_INIT_H_

/**
 * Include headers
 */
// LIB
#include "typedef.h"
#include "mems.h"
#include "hasho.h"
#include "timerN.h"

/**
 *	Declare structures
 */
typedef struct _st_STOP_CALL_KEY {
    UINT        uiClientIP;
    UINT        uiReserved;
} st_CALL_KEY, *pst_CALL_KEY;
#define STOP_CALL_KEY_SIZE      sizeof(st_CALL_KEY)

/**
 * Declare functions
 */
int Init_TCPKeyShm( int ProcNum );
S32 dInitTcp(stMEMSINFO **pMEMSINFO, stHASHOINFO **pHASHOINFO, stTIMERNINFO **pTIMERNINFO);
void SetUpSignal(void);
void UserControlledSignal(S32 isign);
void FinishProgram(void);
void IgnoreSignal(S32 isign);
void vTCPTimerReConstruct(stHASHOINFO *pHASH, stTIMERNINFO *pTIMER);

#endif	/* _ITCP_INIT_H_ */
