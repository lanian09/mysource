/*
 * im_init.h
 *
 */

#ifndef __IM_INIT_H__
#define __IM_INIT_H__

/**
 *	Include headers
 */
// LIB
#include "typedef.h"
#include "mems.h"
#include "hasho.h"
#include "timerN.h"

/**
 *	Define structures
 */
typedef struct _st_call_timer_arg {
	IP4				ClientIP;
} st_CALLTimer;

typedef struct _st_SIPSessKey {
    unsigned int    uiClientIP;
    unsigned int    uiReserved;
} st_IMSessKey, *pst_IMSessKey;
#define ST_CALLTIMER_SIZE 	sizeof(st_CALLTimer)

/**
 *	Declare func.
 */
extern S32 dInitIM(stMEMSINFO **pMEMSINFO, stHASHOINFO **pHASHOINFO, stTIMERNINFO **pTIMERNINFO);
extern void SetUpSignal(void);
extern void UserControlledSignal(S32 isign);
extern void FinishProgram(void);
extern void IgnoreSignal(S32 isign);
extern void vIMSESSTimerReConstruct(stHASHOINFO *pHASH, stTIMERNINFO *pTIMER);

#endif /* __IM_INIT_H__ */
