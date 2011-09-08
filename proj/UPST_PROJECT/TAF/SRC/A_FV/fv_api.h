#ifndef _FV_API_H_
#define _FV_API_H_

#include <common_stg.h>
#include <nifo.h>
#include <logutil.h>
#include <hasho.h>
#include <hashg.h>
#include <timerN.h>
#include <fv.h>

/* http_init.c */
extern S32 dInitFV(stMEMSINFO **pMEMSINFO);
extern void SetUpSignal(void);
extern void UserControlledSignal(S32 isign);
extern void FinishProgram(void);
extern void IgnoreSignal(S32 isign);

extern S32 test_func(S32 type, S32 len, U8 *data, S32 memflag, void *out);

extern S32 dSend_FV_Data(stMEMSINFO *pMEMSINFO, S32 dSndMsgQ, U8 *pNode);
#endif
