#ifndef _SIPT_INIT_H_
#define _SIPT_INIT_H_

/**
 *	Include headers
 */
// LIB
#include "typedef.h"
#include "mems.h"
#include "hasho.h"
#include "timerN.h"

/**
 *	Define cons.
 */
#define SIP_INFO_CNT			100013

/**
 *  Declare structures
 */
typedef struct _st_STOP_CALL_KEY {
    UINT        uiClientIP;
    UINT        uiReserved;
} st_CALL_KEY, *pst_CALL_KEY;
#define STOP_CALL_KEY_SIZE      sizeof(st_CALL_KEY)

/**
 *	Declare functions
 */
int Init_SIPTKeyShm();
S32 dInitSIPT(stMEMSINFO **pMEMSINFO, stHASHOINFO **pSIPHASH, stTIMERNINFO **pTIMERNINFO);
void SetUpSignal(void);
void UserControlledSignal(S32 isign);
void FinishProgram(void);
void IgnoreSignal(S32 isign);
void vSIPTimerReConstruct(stHASHOINFO *pHASH, stTIMERNINFO *pTIMER);

#endif	/* _SIPT_INIT_H_ */
